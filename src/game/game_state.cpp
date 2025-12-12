/****************************************************************************************
* File:         game_state.c
* Author:       Michiel Dirks
* Created on:   12-12-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/
/*
#include <stdint.h>
#include <pins.h>
#include "player.h"

#define GAME_START_BUTTON_PIN ???

enum Game_State {
    GAME_RUNNING,
    GAME_OVER,
};

enum Game_State game_state = GAME_OVER;

enum game_type {
    RUNNER,
    DEATH,
};

void save_high_score() {
    // TODO: Check highscore
}

void start_game(game_type type) {
    if (type == RUNNER) {
        player_start_game();
    }
    game_state = GAME_RUNNING;
}

void game_over(uint16_t score) {
    game_state = GAME_OVER;
}

void update_game_state() {
    if (digitalRead(GAME_START_BUTTON_PIN)) {
        start_game(RUNNER);
    }
}
*/