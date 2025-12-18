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

void init_player();

void update_player();

void player_start_game(e_GAME_TYPE role);

#endif //ATMEGA_GAME_PLAYER_H