/****************************************************************************************
* File:         game_state.c
* Author:       Michiel Dirks
* Created on:   12-12-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/
#define HIGHSCORE_ADDR 0x00

#include <stdint.h>
#include "game_state.h"

#include <stddef.h>

#include "player.h"
#include "print.h"
#include "gfx/gfx.h"
#include "resources.h"
#include "gfx/gravur.h"
#include "net/proto.h"
#include "world_generation/world.h"
#include "../eeprom/eeprom.h"

enum Game_State game_state = GAME_IDLE;

void show_fullscreen(const char* filename) {
    gfx_bitmap_t start_game_bitmap = {
        .filename = filename
    };

    gfx_sprite_t start_game_sprite = (gfx_sprite_t){
        .position = { 120, 0 },
        .size = { 240, 320 },
        .bitmap = &start_game_bitmap,
    };

    gfx_init_bitmap(&start_game_bitmap);
    gfx_draw_sprite(&start_game_sprite);
}

uint16_t save_high_score(uint16_t score) {
    uint16_t current = eeprom_read_uint16(HIGHSCORE_ADDR);
    if (score > current) {
        eeprom_write_uint16(HIGHSCORE_ADDR, score);
        current = score;
    }

    return current;
}

void start_game(e_GAME_TYPE type) {
    player_start_game(type);
    game_state = GAME_RUNNING;
}

void game_over(uint16_t score) {
    game_state = GAME_OVER;
    uint16_t high_score = save_high_score(score);

    uint8_t data[4] = { 0 };
    proto_emit(CMD_GAME_OVER, data);

    show_fullscreen(GAMEOVER_SCREEN);
    gravur_write_integer(142, 123, 2, false, score);
    gravur_write_integer(142, 139, 2, false, high_score);
}

enum Game_State get_game_state() {
    return game_state;
}

void set_game_state(enum Game_State state) {
    game_state = state;
}