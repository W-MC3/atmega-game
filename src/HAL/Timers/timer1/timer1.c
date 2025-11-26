/****************************************************************************************
* File:         timer1.c
* Author:       Michiel Dirks
* Created on:   12-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include "timer1.h"

static void (*inputCaptureCallback)(void) = NULL;
static void (*CompBMatchCallback)(void) = NULL;
static void (*CompAMatchCallback)(void) = NULL;
static void (*TimerOverflowCallback)(void) = NULL;

void initTimer1(const s_TIM1_CONFIG *config) {
    TCCR1A = (config->waveformGenerationMode & (1 << WGM11 | 1 << WGM10))
             | (config->compareOutputModeA
                | config->compareOutputModeB);

    TCCR1B = config->inputCaptureEdgeSelect | (config->waveformGenerationMode & (1 << WGM13 | 1 << WGM12)) | config->clockSource;
    inputCaptureCallback = config->inputCaptureEnabledInterruptCallback;
    CompBMatchCallback = config->CompBMatchInterruptCallback;
    CompAMatchCallback = config->CompAMatchInterruptCallback;
    TimerOverflowCallback = config->TimerOverflowInterruptCallback;

    if (inputCaptureCallback != NULL) {
        TIMSK1 |= (1 << ICIE1);
    }
    if (CompBMatchCallback != NULL) {
        TIMSK1 |= (1 << OCIE1B);
    }
    if (CompAMatchCallback != NULL) {
        TIMSK1 |= (1 << OCIE1A);
    }
    if (TimerOverflowCallback != NULL) {
        TIMSK1 |= (1 << TOIE1);
    }
}

void setOCR1A(uint8_t value) {
    OCR1A = value;
}

ISR(TIMER1_CAPT_vect) {
    if (inputCaptureCallback) {
        inputCaptureCallback();
    }
}

ISR(TIMER1_COMPA_vect) {
    if (CompAMatchCallback) {
        CompAMatchCallback();
    }
}

ISR(TIMER1_COMPB_vect) {
    if (CompBMatchCallback) {
        CompBMatchCallback();
    }
}

ISR(TIMER1_OVF_vect) {
    if (TimerOverflowCallback) {
        TimerOverflowCallback();
    }
}
