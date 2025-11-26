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

#define NUNCHUK_ADDR 0x52
#define UART_BAUDRATE 9600

volatile uint8_t adc_value = 0;

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

    startAdc();

    initTone();
    playTone(3000);
}

void loop(void) {
    setVolume(adc_value);
    if (nunchuk_get_state(NUNCHUK_ADDR)) {

        uint8_t joyX = state.joy_x_axis;
        uint8_t joyY = state.joy_y_axis;
        uint8_t z = state.z_button;
        uint8_t c = state.c_button;
    }

    _delay_ms(20);
}

int main() {
    start();

    for (;;) {
        loop();
    }
}