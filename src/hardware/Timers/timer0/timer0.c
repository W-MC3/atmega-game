/****************************************************************************************
* File:         timer0.c
* Author:       Michiel Dirks
* Created on:   13-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <avr/interrupt.h>
#include <stdlib.h>
#include "timer0.h"

volatile static void (*CompBMatchCallback)(void) = NULL;
volatile static void (*CompAMatchCallback)(void) = NULL;
volatile static void (*TimerOverflowCallback)(void) = NULL;

void initTimer0(const s_TIM0_CONFIG *config) {
    TCCR0A = (config->waveformGenerationMode & (1 << WGM01 | 1 << WGM00))
             | (config->compareOutputModeA
             | config->compareOutputModeB);

    TCCR0B = (config->waveformGenerationMode & (1 << WGM02)) | config->clockSource;
    CompBMatchCallback = config->CompBMatchInterruptCallback;
    CompAMatchCallback = config->CompAMatchInterruptCallback;
    TimerOverflowCallback = config->TimerOverflowInterruptCallback;

    if (CompBMatchCallback != NULL) {
        TIMSK0 |= (1 << OCIE0B);
    }
    if (CompAMatchCallback != NULL) {
        TIMSK0 |= (1 << OCIE0A);
    }
    if (TimerOverflowCallback != NULL) {
        TIMSK0 |= (1 << TOIE0);
    }
}

void setTimer0ClockSource(e_TIM0_ClockSource clockSource)
{
    // Clear existing CS02:CS00 bits, then set new ones
    TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));
    TCCR0B |= (clockSource & ((1 << CS02) | (1 << CS01) | (1 << CS00)));
}


void setOCR0B(uint8_t value) {
    OCR0B = value;
}

uint8_t getOCR0B(void) {
    return OCR0B;
}

void setOCR0A(uint8_t value) {
    OCR0A = value;
}

uint8_t getOCR0A(void) {
    return OCR0A;
}

ISR(TIMER0_COMPA_vect) {
    if (CompAMatchCallback) {
        CompAMatchCallback();
    }
}

ISR(TIMER0_COMPB_vect) {
    if (CompBMatchCallback) {
        CompBMatchCallback();
    }
}

ISR(TIMER0_OVF_vect) {
    if (TimerOverflowCallback) {
        TimerOverflowCallback();
    }
}
