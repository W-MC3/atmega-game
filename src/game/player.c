/****************************************************************************************
* File:         player.c
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <Arduino.h>
#include "player.h"
#include "print.h"
#include "hardware/uart/uart.h"
#include "stdint.h"
#include "delay.h"
#include "../system.h"
#include "../lib/nunchuk/nunchuk.h"
#include "gfx/gfx.h"

#define TIME_BETWEEN_HOPS_MS 100
#define FULL_PLAYTIME (7 * 1000)  // The player starts with 7 seconds of playtime

uint32_t last_hop_time = 0;

int16_t playtime_left_ms = 0; // Max 7000, we also need negative numbers so that the timer won't wrap arounds
uint16_t score;

uint16_t current_y = 0;

gfx_vec2_t playerPosition;


// GFX //
gfx_bitmap_t player_BL;
gfx_bitmap_t player_BR;
gfx_bitmap_t player_TL;
gfx_bitmap_t player_TR;

gfx_bitmap_t tile_selector;

gfx_sprite_t player;

// END GFX //

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    DIR_COUNT // Last value to keep track of enum count
} e_DIRECTION;

const gfx_bitmap_t* player_sprite_lut[GAME_TYPE_COUNT][DIR_COUNT] = {
    // RUNNER
    {
        &player_BR,
        &player_TL,
        &player_TR,
        &player_BL
    },

    // DEATH
    {
        NULL, NULL, NULL, NULL
    }
};


const int8_t dx_lut[DIR_COUNT] = { +1, -1,  0,  0 };
const int8_t dy_lut[DIR_COUNT] = {  0,  0, -1, +1 };

e_GAME_TYPE current_game_type;

void init_player() {
    player_BL = (gfx_bitmap_t){
        .filename = "PLAYERBL.BMP"
    };
    player_BR = (gfx_bitmap_t){
        .filename = "PLAYERBR.BMP"
    };
    player_TL = (gfx_bitmap_t){
        .filename = "PLAYERTL.BMP"
    };
    player_TR = (gfx_bitmap_t){
        .filename = "PLAYERTR.BMP"
    };

    tile_selector = (gfx_bitmap_t){
        .filename = "SELECTOR.BMP"
    };

    player = (gfx_sprite_t){
        .position = { 0, 0 },
        .size = { GFX_TILEMAP_TILE_WIDTH, GFX_TILEMAP_TILE_HEIGHT },
        .bitmap = &player_BL
    };

    gfx_init_bitmap(&player_BL);
    gfx_init_bitmap(&player_BR);
    gfx_init_bitmap(&player_TL);
    gfx_init_bitmap(&player_TR);

    gfx_init_bitmap(&tile_selector);

    gfx_add_sprite(&player);
}

void player_start_game(e_GAME_TYPE role) {
    current_game_type = role;

    if (role == DEATH) {
        gfx_set_bitmap_sprite(&player, &tile_selector);
    }

    playtime_left_ms = FULL_PLAYTIME;
    current_y = 0;
}

void add_score() {
    score += 1;

    playtime_left_ms += 1000;
    playtime_left_ms = min(playtime_left_ms, FULL_PLAYTIME);
}

void move_player(uint8_t x_stick_val, uint8_t y_stick_val)
{
    int x = (int)x_stick_val - 128;
    int y = (int)y_stick_val - 128;

    const int DEADZONE = 70;

    if (abs(x) < DEADZONE && abs(y) < DEADZONE) {
        return;
    }

    e_DIRECTION dir;

    if (abs(x) > abs(y)) {
        dir = x > 0 ? NORTH : EAST;
    } else {
        dir = y > 0 ? SOUTH : WEST;
    }

    // Move
    playerPosition.x += dx_lut[dir];
    playerPosition.y += dy_lut[dir];

    if (playerPosition.x < 0) {
        playerPosition.x = 0;
    }
    else if (playerPosition.x > 4) {
        playerPosition.x = 4;
    }

    if (playerPosition.y < 0) {
        playerPosition.y = 0;
    }
    else if (playerPosition.y > 4) {
        playerPosition.y = 4;
    }

    gfx_bitmap_t* sprite = player_sprite_lut[current_game_type][dir];

    if (sprite) {
        gfx_set_bitmap_sprite(&player, sprite);
    }

    if (current_y > score) {
        add_score();
    }

    gfx_vec2_t player_screen_pos = gfx_world_to_screen(playerPosition);
    gfx_move_sprite(&player, player_screen_pos.x, player_screen_pos.y);
}


void update_game_state() {
    // Death can't die
    if (current_game_type == RUNNER) {
        uint16_t tilemap_index = (playerPosition.y) * GFX_TILEMAP_WIDTH + playerPosition.x;
        if ((tile_flags[tilemap_index] & TILE_DEADLY_FLAG) > 0) {
            game_over(0);
        }
        if (playtime_left_ms < 0) {
            game_over(0);
        }
    }
}

void update_player() {

    if (scheduler_millis() - last_hop_time > TIME_BETWEEN_HOPS_MS) {
        if (nunchuk_get_state(NUNCHUK_ADDR)) {
            last_hop_time = scheduler_millis();

            uint8_t joyX = state.joy_x_axis;
            uint8_t joyY = state.joy_y_axis;

            move_player(joyX, joyY);
        }
    }


    update_game_state();
}