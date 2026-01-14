/****************************************************************************************
* File:         display7seg.c
 * Authors:     Michiel Dirks
* Created on:   06-01-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include "display7seg.h"
#include "../PCF8574/PCF8574.h"

uint8_t numToPins[] = {
 ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE
};

void update_7_display(const uint8_t num) {
 if (num < 0 || num > 9) {
  return;
 }
 pcf8574_write(numToPins[num]);
}
