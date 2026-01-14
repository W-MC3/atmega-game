#include "gravur.h"
#include <stdint.h>
#include <stdlib.h>

static const uint8_t font[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
    {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
    {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
    {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
    {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
    {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
    {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
    {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
    {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
    {0b111, 0b101, 0b111, 0b001, 0b111}, // 9
};

void gravur_write_integer(uint16_t x, uint16_t y, uint16_t scale, bool mirrored, int num) {
    char buffer[10];
    itoa(num, buffer, 10);

    uint16_t cursor_x = x;

    for (char* p = buffer; *p != '\0'; p++) {
        char c = *p;
        if (c < '0' || c > '9') {
            continue;
        }

        uint8_t digit = c - '0';

        gfx_begin_batch(cursor_x, y, 3 * scale, 5 * scale);
        for (uint8_t row = 0; row < 5; row++) {
            uint8_t actual_row = mirrored ? (4 - row) : row;
            for (uint8_t sy = 0; sy < scale; sy++) {
                for (uint8_t col = 0; col < 3; col++) {
                    uint16_t color;
                    if (font[digit][actual_row] & (1 << (2 - col))) {
                        color = 0xFFFF;
                    } else {
                        color = 0x0000;
                    }
                    for (uint8_t sx = 0; sx < scale; sx++) {
                        gfx_push_pixel(color);
                    }
                }
            }
        }
        gfx_end_batch();

        cursor_x += (3 + 1) * scale;
    }
}