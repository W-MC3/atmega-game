/****************************************************************************************
* File:         ADC.h
* Author:       Michiel Dirks
* Created on:   01-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef ADC_ADC_H
#define ADC_ADC_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#define ADJUST_ADC_DATA_LEFT (1 << ADLAR)

typedef enum {
    AREF_INTERNAL = 0,
    AREF_EXT = (1 << REFS0),
    AREF_INTERNAL_1_1V = (1 << REFS1) | (1 << REFS0),
} e_ADC_VOLTAGE_REFERENCE;

typedef enum {
    ADC_0 = 0,
    ADC_1 = (1 << MUX0),
    ADC_2 = (1 << MUX1),
    ADC_3 = (1 << MUX1) | (1 << MUX0),
    ADC_4 = (1 << MUX2),
    ADC_5 = (1 << MUX2) | (1 << MUX0),
    ADC_6 = (1 << MUX2) | (1 << MUX1),
    ADC_7 = (1 << MUX2) | (1 << MUX1) | (1 << MUX0),
    TEMPERATURE = (1 << MUX3),
    REF_1_1V = (1 << MUX3) | (1 << MUX2) | (1 << MUX1),
    REF_0V =  (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0),
} e_ADC_INPUT;

typedef enum {
    FREE_RUNNING = 0,
    ANALOG_COMP = (1 << ADTS0),
    EXT_INTERRUPT_REQ_0 = (1 << ADTS1),
    TIMER0_COMP_MATCH_A = (1 << ADTS1) | (1 << ADTS0),
    TIMER0_OVERFLOW = (1 << ADTS2),
    TIMER1_COMP_MATCH_B = (1 << ADTS2) | (1 << ADTS0),
    TIMER1_OVERFLOW = (1 << ADTS2) | (1 << ADTS1),
    TIMER1_CAPTURE = (1 << ADTS2) | (1 << ADTS1) | (1 << ADTS0),
} e_ADC_AUTO_TRIGGER_SOURCE;

typedef enum {
    DIV_PRE_2 = 0,
    DIV_PRE_4 = (1 << ADPS1),
    DIV_PRE_8 = (1 << ADPS0) | (1 << ADPS1),
    DIV_PRE_16 = (1 << ADPS2),
    DIV_PRE_32 = (1 << ADPS2) | (1 << ADPS0),
    DIV_PRE_64 = (1 << ADPS2) | (1 << ADPS1),
    DIV_PRE_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0),
 } e_ADC_PRESCALER;

typedef struct {
    // ADMUX register settings
    e_ADC_VOLTAGE_REFERENCE reference;
    bool adjust_data_left;
    e_ADC_INPUT input_source;

    // ADCSRA
    e_ADC_PRESCALER clock_prescaler;
    bool auto_trigger;

    // ADCSRB
    e_ADC_AUTO_TRIGGER_SOURCE interrupt_source;

    void (*callback)(uint16_t result);
} ADC_config_t;

void configure_adc(const ADC_config_t *config);

void enable_adc();

void disable_adc();

void start_conversion();

void set_auto_triggers_enabled(bool enable);

void set_adc_conversion_complete_interrupt(bool enable, void (*callback)(uint16_t result));

uint16_t read_adc_result();

#endif //ADC_ADC_H