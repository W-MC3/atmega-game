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

enum Game_State {
    GAME_RUNNING,
    GAME_OVER,
};

enum Game_State game_state = GAME_OVER;

gfx_bitmap_t grass;
gfx_bitmap_t water;
gfx_bitmap_t tile;


gfx_tilemap_t tilemap;

gfx_scene_t scene;

void save_high_score(uint16_t score) {
    // TODO: Check highscore and compare it with the current score
}

void init_scene() {
    grass = (gfx_bitmap_t){
        .filename = "GRASS.BMP"
    };

    water = (gfx_bitmap_t){
        .filename = "WATER.BMP"
    };

    tile = (gfx_bitmap_t){
        .filename = "TILE.BMP"
    };

    tilemap = (gfx_tilemap_t){
        .kinds = { &grass, &water, &tile },
        .tiles = {
            2, 2, 2, 2, 2,
            2, 0, 0, 0, 2,
            2, 0, 0, 0, 2,
            2, 0, 1, 0, 2,
            2, 0, 0, 0, 2,
            2, 0, 0, 0, 2,
            2, 2, 2, 2, 2
        }
    };

    tile_flags[17] |= TILE_DEADLY_FLAG;

    scene = (gfx_scene_t){
        .tilemap = &tilemap,
        .sprites = { },
        .sprite_count = 1
    };

    gfx_init_bitmap(&grass);
    gfx_init_bitmap(&water);
    gfx_init_bitmap(&tile);
    gfx_set_scene(&scene);
}

void start_game(e_GAME_TYPE type) {

    player_start_game(type);
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
