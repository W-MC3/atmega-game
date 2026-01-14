/****************************************************************************************
* File:         player.h
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/


#ifndef ATMEGA_GAME_PLAYER_H
#define ATMEGA_GAME_PLAYER_H

#include "game_state.h"

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    DIR_COUNT // Last value to keep track of enum count
} e_DIRECTION;

void init_player();

void update_player();

void player_start_game(e_GAME_TYPE role);

gfx_vec2_t player_get_world_position();

gfx_vec2_t player_get_screen_position();

e_GAME_TYPE player_get_role();

void player_reset_position();

void reset_playtime();

uint16_t player_get_score();

void mark_tile_trap(gfx_vec2_t world_pos);

void unmark_tile_trap(gfx_vec2_t world_pos);

#endif //ATMEGA_GAME_PLAYER_H