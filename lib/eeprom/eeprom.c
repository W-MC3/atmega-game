/****************************************************************************************
* File:         EEPROM.c
* Author:       Michiel Dirks
* Created on:   16-12-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "eeprom.h"

// Wait till previous transfer completes
static void eeprom_wait(void) {
    while (EECR & (1 << EEPE));
}

static void eeprom_write_byte(uint16_t address, uint8_t data) {
    eeprom_wait();

    EEAR = address;
    EEDR = data;

    cli();
    EECR |= (1 << EEMPE);
    EECR |= (1 << EEPE);
    sei();
}

static uint8_t eeprom_read_byte(uint16_t address) {
    eeprom_wait();

    EEAR = address;
    EECR |= (1 << EERE);

    return EEDR;
}

void eeprom_write_uint16(uint16_t address, uint16_t value) {
    eeprom_write_byte(address, (uint8_t)(value & 0xFF));
    eeprom_write_byte(address + 1, (uint8_t)(value >> 8));
}

uint16_t eeprom_read_uint16(uint16_t address) {
    uint8_t low  = eeprom_read_byte(address);
    uint8_t high = eeprom_read_byte(address + 1);

    return ((uint16_t)high << 8) | low;
}
