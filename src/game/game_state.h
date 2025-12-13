//
// Created by michiel on 12/13/25.
//

#ifndef ATMEGA_GAME_GAME_STATE_H
#define ATMEGA_GAME_GAME_STATE_H

#include <stdint.h>
#include "gfx/gfx.h"


#define TILE_DEADLY_FLAG 1
#define TILE_INACCESSIBLE_FLAG 2

typedef enum {
    RUNNER,
    DEATH,
    GAME_TYPE_COUNT // Last value to keep track of count
} e_GAME_TYPE;

uint8_t tile_flags[GFX_TILEMAP_WIDTH * GFX_TILEMAP_HEIGHT];

void start_game(e_GAME_TYPE type);

void game_over(uint16_t score);

#endif //ATMEGA_GAME_GAME_STATE_H