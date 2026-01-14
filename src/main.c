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
#include "gfx/gravur.h"

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

typedef struct trap_state {
    uint16_t tx;
    uint16_t ty;
    uint32_t deactive_at;
    uint32_t reusable_at;
} trap_state_t;

trap_state_t traps[8] = { 0 };
uint8_t traps_size = 0;

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

    show_fullscreen(HOMESCREEN);

    game_scene.tilemap = world_get_tilemap();
    game_scene.sprite_count = 0;

    gfx_set_scene(&game_scene);

    init_player();
    play_sound(TETRIS, 0);

    proto_init();
    init_npc(&player_npc);
}

void game_init() {
    if (player_get_role() == DEATH) {
        gfx_add_sprite(&(player_npc.sprite));
    }

    traps_size = 0;
    world_next_level();
    move_npc(&player_npc, 0, 500, 500);
}

uint8_t get_active_variant(uint8_t kind) {
    switch (kind) {
        case 2: // SPIKE TRAP
            return 4;
        default:
            return kind;
    }
}

uint8_t get_inactive_variant(uint8_t kind) {
    switch (kind) {
        case 4: // SPIKE TRAP
            return 2;
        default:
            return kind;
    }
}

void activate_trap(gfx_vec2_t world_pos) {
    gfx_tilemap_t* tilemap = world_get_tilemap();
    uint8_t current_tile = gfx_get_tile(tilemap, world_pos.x, world_pos.y);
    uint8_t desired_tile = get_active_variant(current_tile);

    for (uint8_t i = 0; i < traps_size; i++)
    {
        if (traps[i].tx == world_pos.x && traps[i].ty == world_pos.y) {
            return;
        }
    }

    if (current_tile != desired_tile) {
        gfx_set_tile(tilemap, world_pos.x, world_pos.y, desired_tile);
        mark_tile_trap(world_pos);

        if (player_get_role() == DEATH)
        {
            uint8_t data[4] = { (uint8_t)(world_pos.x), (uint8_t)(world_pos.y), 0, 0 };
            proto_emit(CMD_ACTIVATE_TRAP, data);
        }

        traps[traps_size++] = (trap_state_t){
            .tx = world_pos.x,
            .ty = world_pos.y,
            .deactive_at = scheduler_millis() + 2500,
            .reusable_at = scheduler_millis() + 3500
        };
    }
}

void update_traps() {
    uint32_t now = scheduler_millis();

    for (uint8_t i = 0; i < traps_size; i++)
    {
        trap_state_t* trap = &traps[i];
        gfx_tilemap_t* tilemap = world_get_tilemap();
        uint8_t current_tile = gfx_get_tile(tilemap, trap->tx, trap->ty);

        if (now >= trap->deactive_at) {
            uint8_t desired_tile = get_inactive_variant(current_tile);
            if (current_tile != desired_tile) {
                gfx_set_tile(tilemap, trap->tx, trap->ty, desired_tile);
                unmark_tile_trap((gfx_vec2_t){ trap->tx, trap->ty });
            }
        }

        if (now >= trap->reusable_at) {
            for (uint8_t j = i; j < traps_size - 1; j++) {
                traps[j] = traps[j + 1];
            }
            traps_size--;
            i--;
        }
    }
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
                stop_sound_playback();
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

            case CMD_GAME_OVER: {
                if (player_get_role() == DEATH) {
                    gfx_remove_sprite(&(player_npc.sprite));
                }

                show_fullscreen(HOMESCREEN);
                set_game_state(GAME_OVER);
                break; // death won, runner lost
            }

            default:
                break;
            }
    }
}

void game_update() {
    switch (get_game_state()) {
        case GAME_IDLE:
        case GAME_OVER:
            if (nunchuk_get_state(NUNCHUK_ADDR) && state.z_button) {
                stop_sound_playback();

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

            if (player_get_role() == RUNNER) {
                gfx_vec2_t pos = player_get_world_position();
                uint8_t overflow_y = pos.x + 8;

                gravur_write_integer(8, 8, 4, false, player_get_score());

                if (pos.y == overflow_y) {
                    uint8_t data[4] = { 0 };
                    proto_emit(CMD_NEXT_SCENE, data);

                    player_reset_position();
                    world_next_level();
                }
            }

            break;

        default:
            break;
    }
}

void loop(void) {
    setVolume(adc_value);
    update_sound_chunks();

    while (uartDataAvailable()) {
        proto_recv_byte(readUartByte());
    }

    game_update_net();

    if (get_game_state() == GAME_RUNNING) {
        update_player();
        update_traps();

        // cli();
        if (get_game_state() == GAME_RUNNING) // hack, but in certain situations the state may have been changed.
        {
            gfx_frame();
        }
        // sei();
    }

    game_update();
}


int main(void)
{
    start();
    for (;;)
        loop();
}