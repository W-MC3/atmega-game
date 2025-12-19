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
#include "../hardware/uart/uart.h"
#include "gfx/gfx.h"
#include "delay.h"
#include "world_manager.h"

//start button
#define BUTTON_BIT     PC1
#define DEBOUNCE_MS    30

enum Game_State {
    GAME_START,
    GAME_RUNNING,
    GAME_OVER,
};

enum Game_State game_state = GAME_START;

void save_high_score(uint16_t score) {
    // TODO: Check highscore and compare it with the current score
}

uint8_t start_button_pressed_event(void)
{
    static uint8_t stable_state = 1;     // HIGH (pull-up)
    static uint8_t last_reading = 1;
    static uint32_t last_debounce = 0;

    uint8_t reading = (PINC & (1 << BUTTON_BIT)) ? 1 : 0;
    uint32_t now = scheduler_millis();

    if (reading != last_reading) {
        last_debounce = now;
        last_reading = reading;
    }

    if ((now - last_debounce) >= DEBOUNCE_MS) {
        if (reading != stable_state) {
            stable_state = reading;

            // Button pressed (LOW)
            if (stable_state == 0) {
                return 1;
            }
        }
    }

    return 0;
}

void game_over(uint16_t score) {
    game_state = GAME_OVER;
    save_high_score(score);
    load_scene(SCENE_GAMEOVER);
    gfx_frame();
}

void show_homescreen() {
    load_scene(SCENE_HOME);
}


void game_start() {
    load_scene(SCENE_GAME);
    init_player();
}

void game_init() {
    world_manager_init();
    show_homescreen();
}

void game_loop() {
    if (game_state == GAME_START) {
        if (start_button_pressed_event()) {
            game_state = GAME_RUNNING;
            game_start();
            player_start_game(RUNNER);

        }
    }else {

        update_player();
    }

}

