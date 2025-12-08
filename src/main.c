/****************************************************************************************
* File:         uart.c
* Author:       Michiel Dirks
* Created on:   20-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <util/delay.h>
#include "HAL/I2C/twi.h"
#include "HAL/ADC/ADC.h"
#include "HAL/uart/uart.h"
#include "../lib/print/print.h"
#include "../lib/nunchuk/nunchuk.h"
#include "../lib/tone/tone.h"
#include "../lib/tone/Notes.h"
#include "../lib/tone/multi_tone.h"
#include "math.h"
#include "../lib/scheduler/delay.h"

#define NUNCHUK_ADDR 0x52
#define UART_BAUDRATE 9600

#define NOTE_COUNT_MUSIC 120

const uint16_t one_sixteenth = 63; // one sixteenth is 62.5 ms

volatile uint8_t adc_value = 0;

int16_t const melody[CHANNEL_COUNT][NOTE_COUNT_MUSIC][2] =
{
    {
        QUAVER(E5), QUAVER(E5), QUAVER(SILENCE), QUAVER(E5), QUAVER(SILENCE), QUAVER(C5),  QUARTER(E5),     // 1
        QUARTER(G5), QUARTER(SILENCE), QUARTER(G4), QUARTER(SILENCE),
        QUARTER(C4), QUAVER(SILENCE), QUARTER(G4), QUAVER(SILENCE), QUARTER(E4),
        QUAVER(SILENCE), QUARTER(A4), QUARTER(B4), QUAVER(As4), QUARTER(A4),
        QUAVER(G4), QUAVER(E5), QUARTER(G5), QUARTER(A5), QUAVER(F5), QUAVER(G5),
        QUAVER(SILENCE), QUARTER(E5), QUAVER(C5), QUAVER(D5), QUARTER(B4), QUAVER(SILENCE),
        QUARTER(SILENCE), QUAVER(G5), QUAVER(Fs5), QUAVER(F5), QUARTER(Ds5), QUAVER(E5),                    // 7
        QUAVER(SILENCE), QUAVER(Gs4), QUAVER(A4), QUAVER(C5), QUAVER(SILENCE), QUAVER(A4), QUAVER(C5), QUAVER(D5),
        QUARTER(SILENCE), QUAVER(G5), QUAVER(Fs5), QUAVER(F5), QUARTER(Ds5), QUAVER(E5),
        QUAVER(SILENCE), QUARTER(C6), QUAVER(C6), QUARTER(C6), QUARTER(SILENCE),
        QUARTER(SILENCE), QUAVER(G5), QUAVER(Fs5), QUAVER(F5), QUARTER(Ds5), QUAVER(E5),                    // 11
        QUAVER(SILENCE), QUAVER(Gs4), QUAVER(A4), QUAVER(C5), QUAVER(SILENCE), QUAVER(A4), QUAVER(C5), QUAVER(D5),
        QUARTER(SILENCE), QUARTER(Ds5), QUAVER(SILENCE), QUARTER(D5), QUAVER(SILENCE),
        QUARTER(C5), QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE),
        QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE)
    },
    {
        QUAVER(Fs4), QUAVER(Fs4), QUAVER(SILENCE), QUAVER(Fs4), QUAVER(SILENCE), QUAVER(Fs4), QUARTER(Fs4), // 1
        QUARTER(B4), QUARTER(SILENCE), QUARTER(B4), QUARTER(SILENCE),
        QUARTER(E4), QUAVER(SILENCE), QUARTER(C4), QUAVER(SILENCE), QUARTER(G3),
        QUAVER(SILENCE), QUARTER(C4), QUARTER(D4), QUAVER(Cs4), QUARTER(C4),
        QUAVER(C4), QUAVER(G4), QUARTER(B4), QUARTER(C5), QUAVER(A4), QUAVER(B4),
        QUAVER(SILENCE), QUARTER(A4), QUAVER(E4), QUAVER(F4), QUARTER(D4), QUAVER(SILENCE),
        QUARTER(SILENCE), QUAVER(E5), QUAVER(Ds5), QUAVER(D5), QUARTER(B4), QUAVER(C5),                     // 7
        QUAVER(SILENCE), QUAVER(E4), QUAVER(F4), QUAVER(G4), QUAVER(SILENCE), QUAVER(C4), QUAVER(E4), QUAVER(F4),
        QUARTER(SILENCE), QUAVER(E5), QUAVER(Ds5), QUAVER(D5), QUARTER(B4), QUAVER(C5),
        QUAVER(SILENCE), QUARTER(G5), QUAVER(G5), QUARTER(G5), QUARTER(SILENCE),
        QUARTER(SILENCE), QUAVER(E5), QUAVER(Ds5), QUAVER(D5), QUARTER(B4), QUAVER(C5),                     // 11
        QUAVER(SILENCE), QUAVER(E4), QUAVER(F4), QUAVER(G4), QUAVER(SILENCE), QUAVER(C4), QUAVER(E4), QUAVER(F4),
        QUARTER(SILENCE), QUARTER(Gs4), QUAVER(SILENCE), QUARTER(F4), QUAVER(SILENCE),
        QUARTER(E4), QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE),
        QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE)
    },

    {
        QUAVER(D3), QUAVER(D3), QUAVER(SILENCE), QUAVER(D3), QUAVER(SILENCE), QUAVER(D3), QUARTER(D3),  // 1
        QUARTER(G4), QUARTER(SILENCE), QUARTER(G3), QUARTER(SILENCE),
        QUARTER(G3), QUAVER(SILENCE), QUARTER(E3), QUAVER(SILENCE), QUARTER(C3),
        QUAVER(SILENCE), QUARTER(F3), QUARTER(G3), QUAVER(Fs3), QUARTER(G3),
        QUAVER(E3), QUAVER(C4), QUARTER(E4), QUARTER(F4), QUAVER(D4), QUAVER(E4),
        QUAVER(SILENCE), QUARTER(C4), QUAVER(A3), QUAVER(B3), QUARTER(G3), QUAVER(SILENCE),
        QUARTER(C3), QUAVER(SILENCE), QUAVER(G3), QUARTER(SILENCE), QUARTER(C4),                        // 7
        QUARTER(F3), QUAVER(SILENCE), QUAVER(C4), QUARTER(C4), QUARTER(F3),
        QUARTER(C3), QUAVER(SILENCE), QUAVER(G3), QUARTER(SILENCE), QUAVER(G3), QUAVER(C4),
        QUAVER(SILENCE), QUARTER(F5), QUAVER(F5), QUARTER(F5), QUARTER(G3),
        QUARTER(C3), QUAVER(SILENCE), QUAVER(G3), QUARTER(SILENCE), QUARTER(C4),                        // 11
        QUARTER(F3), QUAVER(SILENCE), QUAVER(C4), QUARTER(C4), QUARTER(F3),
        QUARTER(C3), QUARTER(Gs3), QUAVER(SILENCE), QUARTER(As3), QUAVER(SILENCE),
        QUARTER(C4), QUAVER(SILENCE), QUAVER(G3), QUARTER(G3), QUARTER(C3),
        QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE), QUARTER(SILENCE)
    }
};

void adcCallback(const uint16_t result) {
    adc_value = result >> 8; // 8 bits is enough
}

void startAdc() {
    configure_adc(&(ADC_config_t){
        .reference = AREF_EXT,
        .adjust_data_left = true,
        .input_source = ADC_0,
        .clock_prescaler = DIV_PRE_128,
        .auto_trigger = true,
        .interrupt_source = FREE_RUNNING,
        .callback = adcCallback,
    });

    enable_adc();
    start_conversion();
}

void start(void) {
    TWI_Init();

    initUart((uart_config_t) {
        .baudRate = UART_BAUDRATE,
        .parity = UART_PARITY_ODD,
        .stopBits = UART_STOP_1BIT,
        .charSize = UART_CS_8BITS
    });

    print_init(
        sendUartData,
        uartDataAvailable,
        readUartByte
    );

    nunchuk_begin(NUNCHUK_ADDR);

    init_system_timer();

    startAdc();

    initTone();

    init_channel(0, melody[0][0][0]);
    init_channel(1, melody[1][0][0]);
    init_channel(2, melody[2][0][0]);
}

uint8_t event_idx = 0;
uint8_t melody_idx = 0;

uint32_t last_note_update_time = 0;

void update_notes() {
    if (millis() - last_note_update_time < one_sixteenth) {
        return;
    }
    last_note_update_time = millis();
    if(event_idx == 2)
    {
        melody_idx++;
        melody_idx %= NOTES_COUNT;
        event_idx = 0;
    }

    for(uint8_t channel = 0; channel < CHANNEL_COUNT; ++channel)
    {
        if(!event_idx || melody[channel][melody_idx][1])
            set_note(channel, melody[channel][melody_idx][0]);
        else
            set_note(channel, SILENCE);
    }

    event_idx++;
}

void loop(void) {
    setVolume(adc_value);
    if (nunchuk_get_state(NUNCHUK_ADDR)) {

        uint8_t joyX = state.joy_x_axis;
        uint8_t joyY = state.joy_y_axis;
        uint8_t z = state.z_button;
        uint8_t c = state.c_button;
    }

    update_notes();

    _delay_ms(20);
}

int main() {
    start();

    for (;;) {
        loop();
    }
}