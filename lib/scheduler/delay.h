/****************************************************************************************
* File:         timer1.h
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef ATMEGA_GAME_DELAY_H
#define ATMEGA_GAME_DELAY_H

#include <stdint.h>

void init_system_timer(void);
uint32_t millis(void);

#endif //ATMEGA_GAME_DELAY_H