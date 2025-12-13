/****************************************************************************************
* File:         game_state.c
* Author:       Michiel Dirks
* Created on:   12-12-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/


#include <stdint.h>
#include "player.h"
#include "print.h"
#include "../hardware/uart/uart.h"
#include "gfx/gfx.h"

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

void save_high_score(uint16_t score) {
    // TODO: Check highscore and compare it with the current score
}

void start_game(enum game_type type) {
    if (type == RUNNER) {
        player_start_game();
    }
    game_state = GAME_RUNNING;
}

void game_over(uint16_t score) {
    game_state = GAME_OVER;
    save_high_score(score);
    gfx_frame();
    while (true) {}
}
/*
void update_game_state() {
    if (digitalRead(GAME_START_BUTTON_PIN)) {
        start_game(RUNNER);
    }
}
*/
