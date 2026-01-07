/****************************************************************************************
 * File:         main.c
 * Authors:       Michiel Dirks, Mikai Bolding, Daniël Kool
* Created on:   20-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <Arduino.h>
#include <gfx/gfx.h>
#include "hardware/i2c/twi.h"
#include "hardware/ADC/ADC.h"
#include "hardware/uart/uart.h"
#include "hardware/Timers/timer_common.h"
#include "../lib/nunchuk/nunchuk.h"
#include "../lib/scheduler/delay.h"
#include "../lib/PCF8574/PCF8574.h"
#include "sound/tone.h"
#include "sound/sound.h"
#include "world_generation/world.h"
#include "game/player.h"
#include "game/game_state.h"
#include "net/proto.h"
#include "resources.h"
#include "game/npc.h"

#define NUNCHUK_ADDR 0x52
#define UART_BAUDRATE 2400
#define PCF8574_ADDR 0x21

s_Sound main_theme;
volatile uint8_t adc_value = 0;
static gfx_scene_t game_scene;
static game_npc_t player_npc = {
    .tex_north = { .filename = PLAYER_BOTTOM_RIGHT },
    .tex_east = { .filename = PLAYER_TOP_LEFT  },
    .tex_south = { .filename = PLAYER_TOP_RIGHT },
    .tex_west = { .filename = PLAYER_BOTTOM_LEFT },
    .sprite = {
        .position = { 0, 0 },
        .size = { GFX_TILEMAP_TILE_WIDTH, GFX_TILEMAP_TILE_HEIGHT },
    }
};

void adcCallback(const uint16_t result) { adc_value = result >> 8; }

void startAdc(void)
{
    configure_adc(&(ADC_config_t){
        .reference = AREF_EXT,
        .adjust_data_left = true,
        .input_source = ADC_0,
        .clock_prescaler = DIV_PRE_128,
        .auto_trigger = true,
        .interrupt_source = FREE_RUNNING,
        .callback = adcCallback,
    });
    enable_adc();
    start_conversion();
}

void start(void)
{
    init();
    TWI_Init();
    pcf8574_init(PCF8574_ADDR);
    initUart((uart_config_t) {
        .baudRate = UART_BAUDRATE,
        .parity = UART_PARITY_ODD,
        .stopBits = UART_STOP_1BIT,
        .charSize = UART_CS_8BITS
    });

    nunchuk_begin(NUNCHUK_ADDR);

    world_set_seed(0); // Reset de tabel naar index 0

    init_system_timer();
    startAdc();
    initTone();
    gfx_init();
    world_init();

    game_scene.tilemap = world_get_tilemap();
    game_scene.sprite_count = 0;
    gfx_set_scene(&game_scene);

    init_player();
    main_theme = register_sound(TETRIS);
    play_sound(&main_theme);

    proto_init();
}

void game_init() {
    init_npc(&player_npc);

    if (player_get_role() == DEATH) {
        gfx_add_sprite(&(player_npc.sprite));
    }
}

uint16_t world_pos_to_idx(gfx_vec2_t world_pos) {
    return world_pos.x + world_pos.y * GFX_TILEMAP_TILE_WIDTH;
}

gfx_vec2_t idx_to_world_pos(uint16_t idx) {
    return (gfx_vec2_t){ .x = idx % GFX_TILEMAP_TILE_WIDTH, .y = (int16_t)(idx / GFX_TILEMAP_TILE_WIDTH) };
}

void activate_trap(gfx_vec2_t world_pos) {
    return; // TODO: impl
}

void game_update_net() {
    while (proto_has_packet()) {
        proto_packet_t p = proto_get_packet();

        switch (p.opcode) {
            case CMD_NEXT_SCENE: {
                world_next_level();
                break;
            }

            case CMD_START: {
                start_game(RUNNER);
                game_init();
                break;
            }

            case CMD_MOVE: {
                gfx_vec2_t player_screen_pos = gfx_world_to_screen((gfx_vec2_t){ (int16_t)(p.data[1]), (int16_t)(p.data[2]) });
                move_npc(&player_npc, p.data[0], player_screen_pos.x, player_screen_pos.y);
                break;
            }

            case CMD_ACTIVATE_TRAP: {
                gfx_vec2_t trap_world_pos = { (int16_t)(p.data[0]), (int16_t)(p.data[1]) };
                activate_trap(trap_world_pos);

                break;
            }

            default:
                break;
        }
    }
}

void game_update() {
    game_update_net();

    switch (get_game_state()) {
        case GAME_IDLE:
            if (nunchuk_get_state(NUNCHUK_ADDR) && state.z_button) {
                uint8_t data[4] = { 0 };
                proto_emit(CMD_START, data);

                start_game(DEATH);
                game_init();
            }

            break;

        case GAME_RUNNING:
            if (nunchuk_get_state(NUNCHUK_ADDR) && state.z_button && player_get_role() == DEATH) {
                gfx_vec2_t selected_pos = player_get_world_position();

                uint8_t data[4] = { (uint8_t)selected_pos.x, (uint8_t)selected_pos.y, 0, 0 };
                proto_emit(CMD_ACTIVATE_TRAP, data);

                activate_trap(selected_pos);
            }

            update_player();

            break;

        default:
            break;
    }
}

void loop(void) {
    setVolume(adc_value);

    while (uartDataAvailable()) {
        proto_recv_byte(readUartByte());
    }

    if (get_game_state() == GAME_RUNNING) {
        update_player();

        cli();
        gfx_frame();
        sei();
    }

    game_update();
}

int main(void)
{
    start();
    for (;;)
        loop();
}