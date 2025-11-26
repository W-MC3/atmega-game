/****************************************************************************************
* File:         tone.c
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include "tone.h"
#include "../lib/system.h"
#include "HAL/Timers/timer0/timer0.h"
#include "HAL/Timers/timer2/timer2.h"

static bool buzzerEnabled = false;

void timer0CompareCallback(void) {
    buzzerEnabled = !buzzerEnabled;
    if (buzzerEnabled) {
        // Disable PWM output from Timer2 (mute)
        setCompareOutputModeBTimer2(TIM2_DisconnectedOC2BCompareMatch);
    } else {
        // Enable PWM output from Timer2 (sound)
        setCompareOutputModeBTimer2(TIM2_ClearOC2BCompareMatch);
    }
}

void setVolume(uint8_t volume) {
    setOCR2B(volume);
}

void playTone(uint16_t frequency) {
    const uint32_t prescalers[] = {1, 8, 64, 256, 1024};
    const e_TIM0_ClockSource prescalerCodes[] = {
        TIM0_CLOCK_DEFAULT,
        TIM0_CLOCK_PRESCALER_8,
        TIM0_CLOCK_PRESCALER_64,
        TIM0_CLOCK_PRESCALER_256,
        TIM0_CLOCK_PRESCALER_1024
    };

    uint8_t prescalerIndex = 4;
    uint16_t top = 0;
    for (uint8_t i = 4; i > 0; i--) {
        top = (FREQ_CPU / (2 * prescalers[i] * frequency)) - 1;
        if (top > 255) {
            break;
        } prescalerIndex = i;
    }

    setTimer0ClockSource(prescalerCodes[prescalerIndex]);

    setOCR0A(top);
}

void initTone(void) {
    initTimer0(&(s_TIM0_CONFIG) {
        .compareOutputModeA = TIM0_DisconnectedOC1ACompareMatch,
        .compareOutputModeB = TIM0_DisconnectedOC1BCompareMatch,
        .waveformGenerationMode = TIM0_MODE_2,
        .clockSource = TIM0_CLOCK_PRESCALER_256,
        .CompBMatchInterruptCallback = NULL,
        .CompAMatchInterruptCallback = timer0CompareCallback,
        .TimerOverflowInterruptCallback = NULL,
    });

    // Set PWM generation timer
    initTimer2(&(s_TIM2_CONFIG) {
        .compareOutputModeA = TIM2_DisconnectedOC2ACompareMatch,
        .compareOutputModeB = TIM2_ClearOC2BCompareMatch,
        .waveformGenerationMode = TIM2_MODE_3,
        .clockSource = TIM2_CLOCK_DEFAULT,
        .CompBMatchInterruptCallback = NULL,
        .CompAMatchInterruptCallback = NULL,
        .TimerOverflowInterruptCallback = NULL,
    });

    DDRD |= (1 << DDD3);
}
