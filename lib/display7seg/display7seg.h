/****************************************************************************************
* File:         display7seg.h
 * Authors:     Michiel Dirks
* Created on:   06-01-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/


#ifndef ATMEGA_GAME_DISPLAY7SEG_H
#define ATMEGA_GAME_DISPLAY7SEG_H

#include <stdint.h>

/*
 * Display syntax
 *
 *    a
 *  f   b
 *    g
 *  e   c
 *    d   dp
 */

#define SEG_A (1 << 0)
#define SEG_B (1 << 1)
#define SEG_C (1 << 2)
#define SEG_D (1 << 3)
#define SEG_E (1 << 4)
#define SEG_F (1 << 5)
#define SEG_G (1 << 6)
#define SEG_DP (1 << 7)

#define ZERO (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define ONE (SEG_B | SEG_C)
#define TWO (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)
#define THREE (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G)
#define FOUR (SEG_B | SEG_C | SEG_F | SEG_G)
#define FIVE (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define SIX (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define SEVEN (SEG_A | SEG_B | SEG_C)
#define EIGHT (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define NINE (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)

void update_7_display(const uint8_t num);

#endif //ATMEGA_GAME_DISPLAY7SEG_H