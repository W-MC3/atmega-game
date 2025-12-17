/****************************************************************************************
 * File:         main.c
 * Author:       Michiel Dirks, Mikai Bolding
 * Description:  Hardware init + game loop (Deterministic)
 ****************************************************************************************/

#include <Arduino.h>
#include <util/delay.h>
#include <gfx/gfx.h>
#include "hardware/i2c/twi.h"
#include "hardware/ADC/ADC.h"
#include "hardware/uart/uart.h"
#include "../lib/print/print.h"
#include "../lib/nunchuk/nunchuk.h"
#include "../lib/scheduler/delay.h"
#include "sound/tone.h"
#include "sound/sound.h"
#include <world_generation/world.h>

#define UART_BAUDRATE 9600

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
    initUart((uart_config_t){.baudRate = UART_BAUDRATE, .parity = UART_PARITY_ODD, .stopBits = UART_STOP_1BIT, .charSize = UART_CS_8BITS});
    print_init(sendUartData, uartDataAvailable, readUartByte);
    nunchuk_begin(NUNCHUK_ADDR);

    /*
       GEEN analogRead seed meer!
       We gebruiken nu de vaste tabel in world.c zodat beide Arduino's gelijk zijn.
    */
    world_set_seed(0); // Reset de tabel naar index 0

    init_system_timer();
    startAdc();
    // initTone();
    gfx_init();
    world_init();

    game_scene.tilemap = world_get_tilemap();
    game_scene.sprite_count = 0;
    gfx_set_scene(&game_scene);

    init_scene();

    init_player();

    main_theme = register_sound("tetris.sfd");
    // play_sound(&main_theme);

    gfx_frame();
}

void loop(void)
{
    setVolume(adc_value);
    if (nunchuk_get_state(NUNCHUK_ADDR))
    {
        if (state.z_button)
        {
            world_next_level();
            _delay_ms(200);
        }
    }
    world_update();
    gfx_frame();
    _delay_ms(20);
}

int main(void)
{
    start();
    for (;;)
        loop();
}