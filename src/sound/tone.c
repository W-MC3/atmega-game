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
#include "../hardware/uart/uart.h"
#include "../../lib/system.h"
#include "../hardware/Timers/timer0/timer0.h"
#include "../hardware/Timers/timer2/timer2.h"
#include "../../lib/scheduler/delay.h"

static volatile bool buzzerEnabled = false;

volatile uint32_t toneStartTime = 0;
volatile uint16_t toneDuration = 0;
static void (* volatile toneDoneCallback)(void *arg) = NULL;
void *argument;

volatile e_TIM0_ClockSource timer0_stored_prescaler = (e_TIM0_ClockSource)0;

void timer0CompareCallback(void) {
    buzzerEnabled = !buzzerEnabled;
    if (buzzerEnabled) {
        // Disable PWM output from Timer2 (mute) and disable it's clock source
        timer0_stored_prescaler = (e_TIM0_ClockSource)(TCCR0B & 0x07);
        setCompareOutputModeBTimer2(TIM2_DisconnectedOC2BCompareMatch);
    } else {
        // Enable PWM output from Timer2 (sound)
        setTimer0ClockSource(timer0_stored_prescaler);
        setCompareOutputModeBTimer2(TIM2_ClearOC2BCompareMatch);
    }

    if (toneDuration == 0) {
        return;
    }

    if (scheduler_millis() - toneStartTime >= toneDuration && toneDoneCallback != NULL) {
        toneDuration = 0;
        toneDoneCallback(argument);
    }
}

void setVolume(uint8_t volume) {
    setOCR2B(volume);
}

void playTone(uint16_t frequency, uint16_t duration, void (*toneCallback)(void *), void *arg) {
    toneDoneCallback = toneCallback;
    toneStartTime = scheduler_millis();
    toneDuration = duration;
    argument = arg;

    if (frequency == 0) {
        return;
    }

    const uint32_t prescalers[] = {1, 8, 64, 256, 1024};
    const e_TIM0_ClockSource prescalerCodes[] = {
        TIM0_CLOCK_DEFAULT,
        TIM0_CLOCK_PRESCALER_8,
        TIM0_CLOCK_PRESCALER_64,
        TIM0_CLOCK_PRESCALER_256,
        TIM0_CLOCK_PRESCALER_1024
    };

    uint8_t prescalerIndex = 0;
    uint16_t top = 0;
    for (uint8_t i = 0; i < 5; i++) {
        top = (FREQ_CPU / (2 * prescalers[i] * frequency)) - 1;
        prescalerIndex = i;
        if (top < 150) {
            break;
        }
    }

    timer0_stored_prescaler = prescalerCodes[prescalerIndex];
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
