/****************************************************************************************
* File:         EEPROM.h
* Author:       Michiel Dirks
* Created on:   16-12-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef ATMEGA_GAME_EEPROM_H
#define ATMEGA_GAME_EEPROM_H

#include <stdint.h>

void eeprom_write_uint16(uint16_t address, uint16_t value);
uint16_t eeprom_read_uint16(uint16_t address);

#endif //ATMEGA_GAME_EEPROM_H