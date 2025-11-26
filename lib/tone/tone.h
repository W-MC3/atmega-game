//
// Created by michiel on 11/26/25.
//

#ifndef TONE_H
#define TONE_H

#include <stdint.h>

void timer0CompareCallback(void);

void setVolume(uint8_t volume);

void playTone(uint16_t frequency);

void initTone(void);

#endif //TONE_H