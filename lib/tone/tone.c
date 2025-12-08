/****************************************************************************************
* File:         tone.c
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <stddef.h>
#include "tone.h"
#include "multi_tone.h"
#include "../../src/HAL/uart/uart.h"
#include "../../src/HAL/Timers/timer2/timer2.h"

#define CHANNELS 2

struct sound {
    uint16_t freq;
    uint16_t duration;
    const uint8_t *waveform;
} channel[CHANNELS];

void timer2OverflowCallback(void) {
    static uint8_t counter = 0;

    if (counter++ & 1) {    // 62.5 kHz / 2 = 31.25 kHz audio
        return;
    }

    uint8_t out = generate_audio_sample();
    OCR2B = out;
}

void setVolume(uint8_t volume) {
    setOCR2B(volume);
}

void initTone(void) {
    // Set PWM generation timer
    initTimer2(&(s_TIM2_CONFIG) {
        .compareOutputModeA = TIM2_DisconnectedOC2ACompareMatch,
        .compareOutputModeB = TIM2_ClearOC2BCompareMatch,
        .waveformGenerationMode = TIM2_MODE_3,  // Fast PWM, TOP = 255
        .clockSource = TIM2_CLOCK_DEFAULT,      // No prescaler
        .CompBMatchInterruptCallback = NULL,
        .CompAMatchInterruptCallback = NULL,
        .TimerOverflowInterruptCallback = timer2OverflowCallback,
    });

    DDRD |= (1 << DDD3);
}
