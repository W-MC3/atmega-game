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

#endif //ATMEGA_GAME_PLAYER_H