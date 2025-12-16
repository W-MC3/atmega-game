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
#include "sound/tone.h"
#include "sound/sound.h"
#include "../lib/scheduler/delay.h"
#include "../system.h"
#include "game/player.h"
#include "game/game_state.h"

#define UART_BAUDRATE 9600

s_Sound main_theme;

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

    // gfx init must be called before the sound code to initialize the SD card
    gfx_init();

    init_scene();

    init_player();

    //main_theme = register_sound("tetris.sfd");
    //play_sound(&main_theme);

    start_game(RUNNER);
}

void loop(void) {

    update_player();

    gfx_frame();

    setVolume(adc_value);
}


int main() {
    start();

    for (;;) {
        loop();
    }
}