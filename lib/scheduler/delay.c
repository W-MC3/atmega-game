/****************************************************************************************
* File:         delay.c
* Author:       Michiel Dirks
* Created on:   26-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <stddef.h>
#include "delay.h"
#include "../../src/hardware/Timers/timer1/timer1.h"

static volatile uint32_t _millis = 0;

static void _millisUpdater(void) {
    _millis++;
}

uint32_t scheduler_millis(void) {
    uint32_t ms;
    // Safely copy _millis before returning to ensure millis does not get updated while returning
    uint8_t bak = SREG;
    cli();
    ms = _millis;
    SREG = bak;
    return ms;
}

void init_system_timer(void) {
    initTimer1(&(s_TIM1_CONFIG) {
        .compareOutputModeA = 0,
        .compareOutputModeB = 0,
        .waveformGenerationMode = MODE_4,    // CTC with OCR1A
        .inputCaptureEdgeSelect = Falling,
        .clockSource = CLOCK_PRESCALER_64,
        .inputCaptureEnabledInterruptCallback = NULL,
        .CompBMatchInterruptCallback = NULL,
        .CompAMatchInterruptCallback = _millisUpdater,
        .TimerOverflowInterruptCallback = NULL
    });

    setOCR1A(249);
}