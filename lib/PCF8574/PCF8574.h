/****************************************************************************************
* File:         PCF8574.h
 * Authors:       Michiel Dirks
* Created on:   06-01-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef ATMEGA_GAME_PCF8574_H
#define ATMEGA_GAME_PCF8574_H

#include <stdint.h>

void pcf8574Init(uint8_t address);

void pcf8574Write(uint8_t port_val);

#endif //ATMEGA_GAME_PCF8574_H