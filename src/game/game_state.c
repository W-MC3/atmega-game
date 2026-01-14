/****************************************************************************************
* File:         game_state.c
* Author:       Michiel Dirks
* Created on:   12-12-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/


#include <stdint.h>
#include "game_state.h"
#include "player.h"
#include "print.h"
#include "gfx/gfx.h"
#include "net/proto.h"
#include "world_generation/world.h"
#include "nunchuk.h"

enum Game_State game_state = GAME_IDLE;

void save_high_score(uint16_t score) {
    // TODO: Check highscore and compare it with the current score
}

/*
void init_scene() {
    tile_flags[12] |= TILE_DEADLY_FLAG;
    tile_flags[27] |= TILE_DEADLY_FLAG;

    tile_flags[17] |= TILE_INACCESSIBLE_FLAG;
    tile_flags[16] |= TILE_INACCESSIBLE_FLAG;
}
*/

void start_game(e_GAME_TYPE type) {
    player_start_game(type);
    game_state = GAME_RUNNING;
}

void game_over(uint16_t score) {
    game_state = GAME_OVER;
    save_high_score(score);

    uint8_t data[4] = { 0 };
    proto_emit(CMD_GAME_OVER, data);
}

enum Game_State get_game_state() {
    return game_state;
}

/*
void update_game_state() {
    if (digitalRead(GAME_START_BUTTON_PIN)) {
        start_game(RUNNER);
    }
}
*/