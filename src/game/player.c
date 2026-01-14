/****************************************************************************************
* File:         player.c
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <Arduino.h>
#include <stdint.h>
#include "player.h"
#include "delay.h"
#include "../lib/PCF8574/PCF8574.h"
#include "../system.h"
#include "../lib/nunchuk/nunchuk.h"
#include "gfx/gfx.h"
#include "resources.h"
#include "../../lib/display7seg/display7seg.h"
#include "world_generation/world.h"
#include "net/proto.h"
#include "sound/sound.h"

#define TIME_BETWEEN_HOPS_MS 100
#define FULL_PLAYTIME (7 * 1000)  // The player starts with 7 seconds of playtime

uint32_t last_hop_time = 0;

int16_t playtime_left_ms = 0; // Max 7000, we also need negative numbers so that the timer won't wrap arounds
uint16_t score;

gfx_vec2_t playerPosition;
uint16_t maxY;

// GFX //
gfx_bitmap_t player_BL;
gfx_bitmap_t player_BR;
gfx_bitmap_t player_TL;
gfx_bitmap_t player_TR;

gfx_bitmap_t tile_selector;

gfx_sprite_t player;

// END GFX //

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
        .filename = PLAYER_BOTTOM_LEFT
    };
    player_BR = (gfx_bitmap_t){
        .filename = PLAYER_BOTTOM_RIGHT
    };
    player_TL = (gfx_bitmap_t){
        .filename = PLAYER_TOP_LEFT
    };
    player_TR = (gfx_bitmap_t){
        .filename = PLAYER_TOP_RIGHT
    };

    tile_selector = (gfx_bitmap_t){
        .filename = SELECTOR
    };

    player = (gfx_sprite_t){
        .position = { 500, 500 },
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

    player_reset_position();
    playtime_left_ms = FULL_PLAYTIME;
    last_hop_time = scheduler_millis();
    score = 0;
    maxY = 0;
}

e_GAME_TYPE player_get_role() {
    return current_game_type;
}

void add_score() {
    score += 1;

    playtime_left_ms += 1000;
    playtime_left_ms = min(playtime_left_ms, FULL_PLAYTIME);
}

void reset_playtime() {
    playtime_left_ms = FULL_PLAYTIME;
}

void move_player(uint8_t x_stick_val, uint8_t y_stick_val)
{
    int x = (int)x_stick_val - 128;
    int y = (int)y_stick_val - 128;

    const int DEADZONE = 70;

    if (abs(x) < DEADZONE && abs(y) < DEADZONE) {
        return;
    }

    if (current_game_type == RUNNER) {
        // play_sound(HOP, 0);
    }

    gfx_vec2_t last_position = playerPosition;

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
    else if (playerPosition.x > GFX_TILEMAP_WIDTH - 1) {
        playerPosition.x = GFX_TILEMAP_WIDTH - 1;
    }

    if (playerPosition.y < 0) {
        playerPosition.y = 0;
    }
    else if (playerPosition.y > GFX_TILEMAP_HEIGHT - 1) {
        playerPosition.y = GFX_TILEMAP_HEIGHT - 1;
    }

    uint16_t tilemap_index = (playerPosition.y) * GFX_TILEMAP_WIDTH + playerPosition.x;
    if ((tile_flags[tilemap_index] & TILE_INACCESSIBLE_FLAG) > 0 && current_game_type == RUNNER) {
        playerPosition = last_position;
    }

    const gfx_bitmap_t* sprite = player_sprite_lut[current_game_type][dir];

    if (sprite != NULL) {
        gfx_set_bitmap_sprite(&player, (gfx_bitmap_t *)sprite);
    }

    // TODO: only allow advancement to increment score.
    if (playerPosition.y > maxY)
    {
        maxY = playerPosition.y;
        add_score();
    }

    gfx_vec2_t player_screen_pos = gfx_world_to_screen(playerPosition);
    gfx_move_sprite(&player, player_screen_pos.x, player_screen_pos.y);

    uint8_t data[4] = { dir, (uint8_t)(playerPosition.x), (uint8_t)(playerPosition.y), 0 };
    proto_emit(CMD_MOVE, data);
}


void update_game_state() {
    // Death can't die
    if (current_game_type == RUNNER) {
        uint16_t tilemap_index = (playerPosition.y) * GFX_TILEMAP_WIDTH + playerPosition.x;
        if ((tile_flags[tilemap_index] & TILE_DEADLY_FLAG) > 0) {
            game_over(score);
        }
        if (playtime_left_ms == 0) {
            game_over(score);
        }
    }
}

void mark_tile_trap(gfx_vec2_t world_pos) {
    uint8_t idx = world_pos.y * GFX_TILEMAP_WIDTH + world_pos.x;
    tile_flags[idx] |= TILE_DEADLY_FLAG;
}

void unmark_tile_trap(gfx_vec2_t world_pos) {
    uint8_t idx = world_pos.y * GFX_TILEMAP_WIDTH + world_pos.x;
    tile_flags[idx] &= ~TILE_DEADLY_FLAG;
}

void update_player() {
    if (scheduler_millis() - last_hop_time > TIME_BETWEEN_HOPS_MS) {
        playtime_left_ms -= (uint16_t)(scheduler_millis() - last_hop_time);
        if (playtime_left_ms < 0) {
            playtime_left_ms = 0;
        }

        if (player_get_role() == RUNNER)
        {
            update_7_display(playtime_left_ms / 1000);
        } else {
            update_7_display(0);
        }

        if (nunchuk_get_state(NUNCHUK_ADDR)) {
            last_hop_time = scheduler_millis();

            uint8_t joyX = state.joy_x_axis;
            uint8_t joyY = state.joy_y_axis;

            move_player(joyX, joyY);
        }
    }


    update_game_state();
}

gfx_vec2_t player_get_screen_position() {
    return gfx_world_to_screen(playerPosition);
}

gfx_vec2_t player_get_world_position() {
    return playerPosition;
}

void player_reset_position() {
    playerPosition.x = GFX_TILEMAP_WIDTH / 2 - 1;
    playerPosition.y = 0;
    maxY = 0;

    gfx_vec2_t player_screen_pos = gfx_world_to_screen(playerPosition);
    gfx_move_sprite(&player, player_screen_pos.x, player_screen_pos.y);
}

uint16_t player_get_score() {
    return score;
}