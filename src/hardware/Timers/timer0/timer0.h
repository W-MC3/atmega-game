/****************************************************************************************
* File:         timer0.h
* Author:       Michiel Dirks
* Created on:   13-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef TIMER0_H
#define TIMER0_H


#include <avr/interrupt.h>

typedef void (*TimerCallback)(void);

typedef enum {
    TIM0_DisconnectedOC1ACompareMatch = 0,
    TIM0_ToggleOC1ACompareMatch = (1 << COM0A0),
    TIM0_ClearOC1ACompareMatch = (1 << COM0A1),
    TIM0_SetOC1ACompareMatch = (1 << COM0A0) | (1 << COM0A1),
} e_TIM0_CompareOutputModeChanA;

typedef enum {
    TIM0_DisconnectedOC1BCompareMatch = 0,
    TIM0_ToggleOC1BCompareMatch = (1 << COM0B0),
    TIM0_ClearOC1BCompareMatch = (1 << COM0B1),
    TIM0_SetOC1BCompareMatch = (1 << COM0B0) | (1 << COM0B1),
} e_TIM0_CompareOutputModeChanB;

typedef enum {
    TIM0_MODE_0 = 0,
    TIM0_MODE_1 = (1 << WGM00),
    TIM0_MODE_2 = (1 << WGM01),
    TIM0_MODE_3 = (1 << WGM01) | (1 << WGM00),
    TIM0_MODE_4 = (1 << WGM02),
    TIM0_MODE_5 = (1 << WGM02) | (1 << WGM00),
    TIM0_MODE_6 = (1 << WGM02) | (1 << WGM01),
    TIM0_MODE_7 = (1 << WGM02) | (1 << WGM01) | (1 << WGM00),
} e_TIM0_WaveformGenerationMode;

typedef enum {
    TIM0_CLOCK_DEFAULT = (1 << CS00),
    TIM0_CLOCK_PRESCALER_8 = (1 << CS01),
    TIM0_CLOCK_PRESCALER_64 = (1 << CS01) | (1 << CS00),
    TIM0_CLOCK_PRESCALER_256 = (1 << CS02),
    TIM0_CLOCK_PRESCALER_1024 = (1 << CS02) | (1 << CS00),
    TIM0_PinT1FallingEdge = (1 << CS02) | (1 << CS01),
    TIM0_PinTRisingEdge = (1 << CS02) | (1 << CS01) | (1 << CS00),
} e_TIM0_ClockSource;

typedef struct {
    e_TIM0_CompareOutputModeChanA compareOutputModeA;
    e_TIM0_CompareOutputModeChanB compareOutputModeB;
    e_TIM0_WaveformGenerationMode waveformGenerationMode;
    e_TIM0_ClockSource clockSource;
    TimerCallback CompBMatchInterruptCallback;
    TimerCallback CompAMatchInterruptCallback;
    TimerCallback TimerOverflowInterruptCallback;
} s_TIM0_CONFIG;

void initTimer0(const s_TIM0_CONFIG *config);

void setTimer0ClockSource(e_TIM0_ClockSource clockSource);

void setOCR0B(uint8_t value);

uint8_t getOCR0B(void);

void setOCR0A(uint8_t value);

uint8_t getOCR0A(void);

#endif //TIMER0_H