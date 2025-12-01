/****************************************************************************************
* File:         uart.c
* Author:       Michiel Dirks
* Created on:   20-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <util/delay.h>
#include <gfx/gfx.h>
#include <Arduino.h>
#include "hardware/i2c/twi.h"
#include "hardware/uart/uart.h"
#include "../lib/print/print.h"
#include "../lib/nunchuk/nunchuk.h"

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

void test_gfx() {
    gfx_bitmap_t grass_bmp = {
        .filename = "grass.bmp"
    };

    gfx_bitmap_t water_bmp = {
        .filename = "water.bmp"
    };

    gfx_bitmap_t player_bmp = {
        .filename = "player.bmp"
    };

    gfx_tilemap_t tilemap = {
        .kinds = { &grass_bmp, &water_bmp },
        .tiles = {
            0, 0, 0,
            0, 1, 0,
            0, 0, 0
        }
    };

    gfx_sprite_t player = {
        .position = { 0, 0 },
        .bitmap = &player_bmp
    };

    gfx_scene_t scene = {
        .tilemap = &tilemap,
        .sprites = { &player }
    };

    gfx_init();
    gfx_set_scene(&scene);

    for (;;) {
        gfx_move_sprite(&player, player.position.x + 1, player.position.y);
        gfx_frame();

        _delay_ms(100);
    }
}

int main() {
    init();
    test_gfx();

    // start();
    //
    // for (;;) {
    //     loop();
    // }
}