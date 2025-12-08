/****************************************************************************************
* File:         ADC.c
* Author:       Michiel Dirks
* Created on:   01-10-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <avr/interrupt.h>
#include "ADC.h"
#include "stdlib.h"

static void (*adc_callback)(uint16_t result) = NULL;

void configure_adc(const ADC_config_t *config) {
    ADMUX = config->reference | config->input_source;
    ADCSRA = config->clock_prescaler;
    ADCSRB = config->interrupt_source;

    if (config->adjust_data_left) {
        ADMUX |= (1 << ADLAR);
    }

    set_auto_triggers_enabled(config->auto_trigger);

    if (config->callback != NULL) {
        set_adc_conversion_complete_interrupt(true, config->callback);
    }
}

void enable_adc(void) {
    ADCSRA |= (1 << ADEN);
}

void disable_adc(void) {
    ADCSRA &= ~(1 << ADEN);
}

void start_conversion(void) {
    ADCSRA |= (1 << ADSC);
}

void set_auto_triggers_enabled(bool enable) {
    if (enable) {
        ADCSRA |= (1 << ADATE);
    }
    else {
        ADCSRA &= ~(1 << ADATE);
    }
}

void set_adc_conversion_complete_interrupt(bool enable, void (*callback)(uint16_t result)) {
    adc_callback = callback;
    if (enable) {
        ADCSRA |= (1 << ADIE);
    }
    else {
        ADCSRA &= ~(1 << ADIE);
    }
}

uint16_t read_adc_result(void) {
    return ADCL | (ADCH << 8);
}

ISR(ADC_vect) {
    uint16_t result = read_adc_result();
    if (adc_callback != NULL) {
        adc_callback(result);
    }
}
