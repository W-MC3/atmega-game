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
        .BaudRate = UART_BAUDRATE,
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

        // ---- USE VALUES HERE ----
        // joyX = 0..255
        // joyY = 0..255

        // Example: print to UART, or control something
        while (!txAvailable());
        print("X=%u Y=%u\r\n", joyX, joyY);
        while (!txAvailable());
        print("Z=%u, c=%u\r\n", z, c);
    }

    _delay_ms(20);   // Nunchuk needs ~5–10ms between reads
}

int main() {
    start();

    for (;;) {
        loop();
    }
}