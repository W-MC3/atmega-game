/****************************************************************************************
* File:         timer2.c
* Author:       Michiel Dirks
* Created on:   13-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <stdlib.h>
#include "timer2.h"

static void (*CompBMatchCallback)(void) = NULL;
static void (*CompAMatchCallback)(void) = NULL;
static void (*TimerOverflowCallback)(void) = NULL;

void initTimer2(const s_TIM2_CONFIG *config) {
    TCCR2A = (config->waveformGenerationMode & (1 << WGM21 | 1 << WGM20))
             | (config->compareOutputModeA
             | config->compareOutputModeB);

    TCCR2B = (config->waveformGenerationMode & (1 << WGM02)) | config->clockSource;
    CompBMatchCallback = config->CompBMatchInterruptCallback;
    CompAMatchCallback = config->CompAMatchInterruptCallback;
    TimerOverflowCallback = config->TimerOverflowInterruptCallback;

    if (CompBMatchCallback != NULL) {
        TIMSK2 |= (1 << OCIE2B);
    }
    if (CompAMatchCallback != NULL) {
        TIMSK2 |= (1 << OCIE2A);
    }
    if (TimerOverflowCallback != NULL) {
        TIMSK2 |= (1 << TOIE2);
    }
}

void setCompareOutputModeBTimer2(e_TIM2_CompareOutputModeChanB mode)
{
    // Clear existing COM2B1:0 bits
    TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0));

    // Set new mode bits
    TCCR2A |= mode;
}

void setOCR2B(uint8_t value) {
    OCR2B = value;
}

ISR(TIMER2_COMPA_vect) {
    if (CompAMatchCallback) {
        CompAMatchCallback();
    }
}

ISR(TIMER2_COMPB_vect) {
    if (CompBMatchCallback) {
        CompBMatchCallback();
    }
}

ISR(TIMER2_OVF_vect) {
    if (TimerOverflowCallback) {
        TimerOverflowCallback();
    }
}
