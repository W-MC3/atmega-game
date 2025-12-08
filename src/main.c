/****************************************************************************************
* File:         uart.c
* Author:       Michiel Dirks, Mikai Bolding
* Created on:   20-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <util/delay.h>
#include <gfx/gfx.h>
#include <Arduino.h>
#include "hardware/i2c/twi.h"
#include "hardware/ADC/ADC.h"
#include "hardware/uart/uart.h"
#include "../lib/print/print.h"
#include "../lib/nunchuk/nunchuk.h"
#include "../lib/tone/tone.h"
#include "math.h"
#include "../lib/scheduler/delay.h"

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

typedef struct {
    uint16_t frequency;  // Hz
    uint16_t duration;   // ms
} tone;

#define _ 0  // for rests

tone tetrisTheme[] = {
    {659, 406},
    {494, 203},
    {523, 203},
    {587, 406},
    {523, 203},
    {494, 203},
    {440, 406},
{1, 30},
    {440, 203},
    {523, 203},
    {659, 406},
    {587, 203},
    {523, 203},
    {494, 609},
    {523, 203},
    {587, 406},
    {659, 406},
    {523, 406},
    {440, 406},
{1, 30},
    {440, 203},
{1, 30},
    {440, 203},
    {494, 203},
    {523, 203},
    {587, 609},
    {698, 203},
    {880, 406},
    {784, 203},
    {698, 203},
    {659, 609},
    {523, 203},
    {659, 406},
    {587, 203},
    {523, 203},
    {494, 406},
{1, 30},
    {494, 203},
    {523, 203},
    {587, 406},
    {659, 406},
    {523, 406},
    {440, 406},
{1, 30},
    {440, 406},
};


uint8_t toneIndex = 0;
uint8_t toneCount = sizeof(tetrisTheme) / sizeof(tetrisTheme[0]);

void play_next_tone() {
    if (toneIndex >= toneCount) {
        toneIndex = 0;
    }

    uint16_t f = tetrisTheme[toneIndex].frequency;
    uint16_t d = tetrisTheme[toneIndex].duration;
    toneIndex++;

    playTone(f, d, play_next_tone);
}

void start(void) {
    init();

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
    playTone(100, 10, play_next_tone);
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

void test_gfx() {
    gfx_bitmap_t grass = {
        .filename = "GRASS.BMP"
    };

    gfx_bitmap_t water = {
        .filename = "WATER.BMP"
    };

    gfx_bitmap_t tile = {
        .filename = "TILE.BMP"
    };

    gfx_bitmap_t player_bmp = {
        .filename = "PLAYER.BMP"
    };

    gfx_tilemap_t tilemap = {
        .kinds = { &grass, &water, &tile },
        .tiles = {
            2, 2, 2, 2, 2,
            2, 0, 0, 0, 2,
            2, 0, 1, 0, 2,
            2, 0, 0, 0, 2,
            2, 2, 2, 2, 2
        }
    };

    gfx_sprite_t player = {
        .position = { 2, 2 },
        .size = { GFX_TILEMAP_TILE_WIDTH, GFX_TILEMAP_TILE_HEIGHT },
        .bitmap = &player_bmp
    };

    gfx_scene_t scene = {
        .tilemap = &tilemap,
        .sprites = { &player },
        .sprite_count = 1
    };

    gfx_init();
    gfx_init_bitmap(&grass);
    gfx_init_bitmap(&water);
    gfx_init_bitmap(&tile);
    gfx_init_bitmap(&player_bmp);
    gfx_set_scene(&scene);

    float angle = 0.0;

    const gfx_vec2_t center = gfx_world_to_screen((gfx_vec2_t){ 2, 2 });

    for (;;) {
        angle += 0.1f;
        gfx_move_sprite(&player,
            center.x + (int16_t)(30.0f * cosf(angle)),
            center.y + (int16_t)(30.0f * sinf(angle))
        );

        gfx_frame();
    }
}

int main() {
    start();
    test_gfx();

    for (;;) {
        loop();
    }
}