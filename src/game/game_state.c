/****************************************************************************************
* File:         game_state.c
* Author:       Michiel Dirks
* Created on:   12-12-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/


#include <stdint.h>
#include "game_state.h"

#include <stddef.h>

#include "player.h"
#include "gfx/gfx.h"
#include "resources.h"

enum Game_State game_state = GAME_IDLE;

void show_boot_screen() {

    gfx_bitmap_t start_game_bitmap = {
        .filename = HOMESCREEN
    };

    gfx_sprite_t start_game_sprite = (gfx_sprite_t){
        .position = { 120, 0 },
        .size = { 240, 320 },
        .bitmap = &start_game_bitmap,
    };

    gfx_scene_t UI_scene = (gfx_scene_t){
        .tilemap = NULL,
        .sprites = {&start_game_sprite},
        .sprite_count = 1,
    };

    gfx_init_bitmap(&start_game_bitmap);
    gfx_set_scene(&UI_scene);
    gfx_draw_sprite(&start_game_sprite);
    gfx_frame();
}

void save_high_score(uint16_t score) {
    // TODO: Check highscore and compare it with the current score
}

void start_game(e_GAME_TYPE type) {
    player_start_game(type);
    game_state = GAME_RUNNING;
}

void game_over(uint16_t score) {
    game_state = GAME_OVER;
    save_high_score(score);
}

enum Game_State get_game_state() {
    return game_state;
}
