/****************************************************************************************
* File:         player.c
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include "stdint.h"
#include "delay.h"
#include "../system.h"
#include "../lib/nunchuk/nunchuk.h"
#include "gfx/gfx.h"

#define TIME_BETWEEN_HOPS_MS 100

uint32_t last_hop_time = 0;

uint32_t playtime_left_ms = 0;
uint16_t score;

gfx_vec2_t playerPosition;

enum game_type {
    RUNNER,
    DEATH,
};

enum DIRECTION {
    NORTH,
    EAST,
    SOUTH,
    WEST,
};

gfx_bitmap_t player_front;
gfx_bitmap_t player_right;
gfx_bitmap_t player_back;
gfx_bitmap_t player_left;

gfx_sprite_t player;

void init_player() {
    gfx_bitmap_t player_front = {
        .filename = "player_front.BMP"
    };
    gfx_bitmap_t player_right = {
        .filename = "player_right.BMP"
    };
    gfx_bitmap_t player_back = {
        .filename = "player_back.BMP"
    };
    gfx_bitmap_t player_left = {
        .filename = "player_left.BMP"
    };

    player = (gfx_sprite_t){
        .position = { 0, 0 },
        .size = { GFX_TILEMAP_TILE_WIDTH, GFX_TILEMAP_TILE_HEIGHT },
        .bitmap = &player_front
    };

    gfx_init_bitmap(&player_front);
    gfx_init_bitmap(&player_right);
    gfx_init_bitmap(&player_back);
    gfx_init_bitmap(&player_left);

}

void start_game() {
    playtime_left_ms = 7 * 1000; // The player starts with 7 seconds of playtime
}

void move_player(uint8_t x_stick_val, uint8_t y_stick_val) {
    if (x_stick_val > 128 + 50) {
        playerPosition.x += 1;
        gfx_set_bitmap_sprite(&player, &player_front);
    }
    else if (x_stick_val < 128 - 50) {
        playerPosition.x -= 1;
        gfx_set_bitmap_sprite(&player, &player_right);
    }

    if (y_stick_val > 128 + 50) {
        playerPosition.y += 1;
        gfx_set_bitmap_sprite(&player, &player_back);
    }
    else if (y_stick_val < 128 - 50) {
        playerPosition.y -= 1;
        gfx_set_bitmap_sprite(&player, &player_left);
    }

    gfx_vec2_t player_screen_pos = gfx_world_to_screen(playerPosition);

    gfx_move_sprite(&player,
            player_screen_pos.x,
            player_screen_pos.y
        );

}

void update_player() {

    if (scheduler_millis() - last_hop_time < TIME_BETWEEN_HOPS_MS) {
        if (nunchuk_get_state(NUNCHUK_ADDR)) {
            last_hop_time = scheduler_millis();

            uint8_t joyX = state.joy_x_axis;
            uint8_t joyY = state.joy_y_axis;

            move_player(joyX, joyY);
        }
    }
}