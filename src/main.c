/****************************************************************************************
* File:         uart.c
* Author:       Michiel Dirks
* Created on:   20-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <util/delay.h>
#include "HAL/I2C/twi.h"
#include "HAL/uart/uart.h"
#include "HAL/print/print.h"
#include "../resources/Nunchuk/src/Nunchuk.h"

#define NUNCHUK_ADDR 0x52
#define UART_BAUDRATE 9600

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
}

void loop(void) {
    if (nunchuk_get_state(NUNCHUK_ADDR)) {

        uint8_t joyX = state.joy_x_axis;
        uint8_t joyY = state.joy_y_axis;
        uint8_t z = state.z_button;
        uint8_t c = state.c_button;

        while (!txAvailable());
        print("X=%u Y=%u\n", joyX, joyY);
        while (!txAvailable());
        print("Z=%u, c=%u\n", z, c);
    }

    _delay_ms(20);
}

int main() {
    start();

    for (;;) {
        loop();
    }
}