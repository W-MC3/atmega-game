/****************************************************************************************
* File:         timer1.h
* Author:       Michiel Dirks
* Created on:   12-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef TIMER1_H
#define TIMER1_H

#include <avr/interrupt.h>

typedef enum {
    ToggleOC1ACompareMatch = (1 << COM1A0),
    ClearOC1ACompareMatch = (1 << COM1A1),
    SetOC1ACompareMatch = (1 << COM1A0) | (1 << COM1A1),
} e_TIM1_CompareOutputModeChanA;

typedef enum {
    ToggleOC1BCompareMatch = (1 << COM1B0),
    ClearOC1BCompareMatch = (1 << COM1B1),
    SetOC1BCompareMatch = (1 << COM1B0) | (1 << COM1B1),
} e_TIM1_CompareOutputModeChanB;

typedef enum {
    MODE_0 = 0,
    MODE_1 = (1 << WGM10),
    MODE_2 = (1 << WGM11),
    MODE_3 = (1 << WGM11) | (1 << WGM10),
    MODE_4 = (1 << WGM12),
    MODE_5 = (1 << WGM12) | (1 << WGM10),
    MODE_6 = (1 << WGM12) | (1 << WGM11),
    MODE_7 = (1 << WGM12) | (1 << WGM11) | (1 << WGM10),
    MODE_8 = (1 << WGM13),
    MODE_9 = (1 << WGM13) | (1 << WGM10),
    MODE_10 = (1 << WGM13) | (1 << WGM11),
    MODE_11 = (1 << WGM13) | (1 << WGM11) | (1 << WGM10),
    MODE_12 = (1 << WGM13) | (1 << WGM12),
    MODE_13 = (1 << WGM13) | (1 << WGM12) | (1 << WGM10),
    MODE_14 = (1 << WGM13) | (1 << WGM12) | (1 << WGM11),
    MODE_15 = (1 << WGM13) | (1 << WGM12) | (1 << WGM11) | (1 << WGM10),
} e_TIM1_WaveformGenerationMode;

typedef enum {
    Falling = 0,
    Rising = (1 << ICES1),
} e_TIM1_EdgeSelect;

typedef enum {
    CLOCK_DEFAULT = (1 << CS10),
    CLOCK_PRESCALER_8 = (1 << CS11),
    CLOCK_PRESCALER_64 = (1 << CS11) | (1 << CS10),
    CLOCK_PRESCALER_256 = (1 << CS12),
    CLOCK_PRESCALER_1024 = (1 << CS12) | (1 << CS10),
    PinT1FallingEdge = (1 << CS12) | (1 << CS11),
    PinTRisingEdge = (1 << CS12) | (1 << CS11) | (1 << CS10),
} e_TIM1_ClockSource;

typedef void (*TimerCallback)(void);

typedef struct {
    e_TIM1_CompareOutputModeChanA compareOutputModeA;
    e_TIM1_CompareOutputModeChanB compareOutputModeB;
    e_TIM1_WaveformGenerationMode waveformGenerationMode;
    e_TIM1_EdgeSelect inputCaptureEdgeSelect;
    e_TIM1_ClockSource clockSource;
    TimerCallback inputCaptureEnabledInterruptCallback;
    TimerCallback CompBMatchInterruptCallback;
    TimerCallback CompAMatchInterruptCallback;
    TimerCallback TimerOverflowInterruptCallback;
} s_TIM1_CONFIG;

void initTimer1(const s_TIM1_CONFIG *config);

void setOCR1A(uint16_t value);

#endif //TIMER1_H