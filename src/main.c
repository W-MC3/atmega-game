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

static const int8_t trap_lookup[] = {
    [2] = 4
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
    play_sound(TETRIS, 0);

    proto_init();
}

void game_init() {
    stop_sound_playback();
    init_npc(&player_npc);

    if (player_get_role() == DEATH) {
        gfx_add_sprite(&(player_npc.sprite));
    }
}

bool is_trap_kind(uint8_t index) {
    size_t len = sizeof(trap_lookup) / sizeof(trap_lookup[0]);
    if (index >= len) return false;
    return trap_lookup[index] != 0;
}

uint8_t get_active_tile(uint8_t current_tile) {
    size_t len = sizeof(trap_lookup) / sizeof(trap_lookup[0]);
    if (current_tile < len && trap_lookup[current_tile] != 0) {
        return (uint8_t)trap_lookup[current_tile];
    }

    return current_tile;
}

uint8_t get_inactive_tile(uint8_t current_tile) {
    for (size_t i = 0; i < sizeof(trap_lookup) / sizeof(trap_lookup[0]); i++) {
        if (trap_lookup[i] == current_tile) {
            return (uint8_t)i;
        }
    }

    return current_tile;
}

void activate_trap(gfx_vec2_t world_pos) {
    gfx_tilemap_t* tilemap = world_get_tilemap();
    uint8_t current_tile = gfx_get_tile(tilemap, world_pos.x, world_pos.y);

    if (!is_trap_kind(current_tile)) {
        return;
    }

    uint8_t desired_tile = get_active_tile(current_tile);

    gfx_set_tile(tilemap, world_pos.x, world_pos.y, desired_tile);
    mark_deadly_tile(world_pos);

    restore_in[world_pos.y * GFX_TILEMAP_WIDTH + world_pos.x] = scheduler_millis() + 2000;
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

void update_sounds() {
    update_sound_chunks();
}

void loop(void) {
    setVolume(adc_value);
    update_sounds();

    while (uartDataAvailable()) {
        proto_recv_byte(readUartByte());
    }

    if (get_game_state() == GAME_RUNNING) {
        update_player();

        gfx_frame();
    }

    game_update();
}

int main(void)
{
    start();
    for (;;) {
        loop();
    }
}