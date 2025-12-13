//
// Created by michiel on 12/13/25.
//

#ifndef ATMEGA_GAME_GAME_STATE_H
#define ATMEGA_GAME_GAME_STATE_H

#include <stdint.h>
#include "gfx/gfx.h"


#define TILE_DEADLY_FLAG 1
#define TILE_INACCESSIBLE_FLAG 2

uint8_t tile_flags[GFX_TILEMAP_WIDTH * GFX_TILEMAP_HEIGHT];

void game_over(uint16_t score);

#endif //ATMEGA_GAME_GAME_STATE_H