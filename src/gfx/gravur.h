#ifndef ATMEGA_GAME_GRAVUR_H
#define ATMEGA_GAME_GRAVUR_H

#include <stdint.h>

#include "gfx.h"

typedef struct gravur {
    gfx_bitmap_t** chars;
    uint8_t char_offset; // used for numerical fonts
    uint8_t size;
} gravur_t;

void gravur_write_integer(uint16_t x, uint16_t y, uint16_t scale, bool mirrored, int num);

#endif //ATMEGA_GAME_GRAVUR_H