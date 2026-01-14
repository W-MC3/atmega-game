//
// Created by michiel on 11/26/25.
//

#ifndef TONE_H
#define TONE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    void timer0CompareCallback(void);

    void setVolume(uint8_t volume);

    void playTone(uint16_t frequency, uint16_t duration, void (*toneCallback)());

    void initTone(void);

#ifdef __cplusplus
}
#endif

#endif //TONE_H