/****************************************************************************************
* File:         timer2.h
* Author:       Michiel Dirks
* Created on:   13-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef TIMER2_H
#define TIMER2_H

#include <avr/interrupt.h>

typedef void (*TimerCallback)(void);

typedef enum {
    TIM2_DisconnectedOC2ACompareMatch = 0,
    TIM2_ToggleOC2ACompareMatch = (1 << COM2A0),
    TIM2_ClearOC2ACompareMatch = (1 << COM2A1),
    TIM2_SetOC2ACompareMatch = (1 << COM2A0) | (1 << COM2A1),
} e_TIM2_CompareOutputModeChanA;

typedef enum {
    TIM2_DisconnectedOC2BCompareMatch = 0,
    TIM2_ToggleOC2BCompareMatch = (1 << COM2B0),
    TIM2_ClearOC2BCompareMatch = (1 << COM2B1),
    TIM2_SetOC2BCompareMatch = (1 << COM2B0) | (1 << COM2B1),
} e_TIM2_CompareOutputModeChanB;

typedef enum {
    TIM2_MODE_0 = 0,
    TIM2_MODE_1 = (1 << WGM20),
    TIM2_MODE_2 = (1 << WGM21),
    TIM2_MODE_3 = (1 << WGM21) | (1 << WGM20),
    TIM2_MODE_4 = (1 << WGM22),
    TIM2_MODE_5 = (1 << WGM22) | (1 << WGM20),
    TIM2_MODE_6 = (1 << WGM22) | (1 << WGM21),
    TIM2_MODE_7 = (1 << WGM22) | (1 << WGM21) | (1 << WGM20),
} e_TIM2_WaveformGenerationMode;

typedef enum {
    TIM2_CLOCK_DEFAULT = (1 << CS20),
    TIM2_CLOCK_PRESCALER_8 = (1 << CS21),
    TIM2_CLOCK_PRESCALER_64 = (1 << CS21) | (1 << CS20),
    TIM2_CLOCK_PRESCALER_256 = (1 << CS22),
    TIM2_CLOCK_PRESCALER_1024 = (1 << CS22) | (1 << CS20),
    TIM2_PinT1FallingEdge = (1 << CS22) | (1 << CS21),
    TIM2_PinTRisingEdge = (1 << CS22) | (1 << CS21) | (1 << CS20),
} e_TIM2_ClockSource;

typedef struct {
    e_TIM2_CompareOutputModeChanA compareOutputModeA;
    e_TIM2_CompareOutputModeChanB compareOutputModeB;
    e_TIM2_WaveformGenerationMode waveformGenerationMode;
    e_TIM2_ClockSource clockSource;
    TimerCallback CompBMatchInterruptCallback;
    TimerCallback CompAMatchInterruptCallback;
    TimerCallback TimerOverflowInterruptCallback;
} s_TIM2_CONFIG;

void initTimer2(const s_TIM2_CONFIG *config);

void setCompareOutputModeBTimer2(e_TIM2_CompareOutputModeChanB mode);

void setOCR2B(uint8_t value);

#endif //TIMER2_H