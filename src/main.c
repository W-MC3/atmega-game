/****************************************************************************************
 * File:         main.c
 * Authors:       Michiel Dirks, Mikai Bolding, Daniël Kool
* Created on:   20-11-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <Arduino.h>
#include <util/delay.h>
#include <gfx/gfx.h>
#include "hardware/i2c/twi.h"
#include "hardware/ADC/ADC.h"
#include "hardware/uart/uart.h"
#include "../lib/nunchuk/nunchuk.h"
#include "../lib/scheduler/delay.h"
#include "sound/tone.h"
#include "sound/sound.h"
#include "world_generation/world.h"
#include "game/player.h"
#include "game/game_state.h"
#include "net/proto.h"

#define UART_BAUDRATE 2400

s_Sound main_theme;
volatile uint8_t adc_value = 0;
static gfx_scene_t game_scene;

void adcCallback(const uint16_t result) { adc_value = result >> 8; }

void startAdc(void)
{
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

void start(void)
{
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


    world_set_seed(0); // Reset de tabel naar index 0

    init_system_timer();
    startAdc();
    initTone();
    gfx_init();
    world_init();

    game_scene.tilemap = world_get_tilemap();
    game_scene.sprite_count = 0;
    gfx_set_scene(&game_scene);

    init_player();
    main_theme = register_sound(ZELDA);
    //play_sound(&main_theme);

    DDRB |= (1 << DDB5);

    proto_init();
    // start_game(RUNNER);
}

void loop(void) {
    // update_player();
    // gfx_frame();
    // setVolume(adc_value);

    while (uartDataAvailable()) {
        proto_recv_byte(readUartByte());
    }

    while (proto_has_packet()) {
        proto_packet_t p = proto_get_packet();

        switch (p.opcode) {
            case CMD_PING:
                PORTB ^= (1 << PORTB5);
                break;

            default:
                break;
        }
    }

    _delay_ms(1000);

    uint8_t data[4] = { 0 };
    proto_emit(CMD_PING, data);
}

int main(void)
{
    start();
    for (;;)
        loop();
}