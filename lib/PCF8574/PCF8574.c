/****************************************************************************************
* File:         PCF8574.c
 * Authors:       Michiel Dirks
* Created on:   06-01-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include "PCF8574.h"
#include "../../src/hardware/i2c/twi.h"

uint8_t device_address;
uint8_t written_val;

void pcf8574Init(uint8_t address) {
 device_address = address;
}

void pcf8574Write(uint8_t port_val) {
 if (port_val == written_val) {
  return;
 }
 uint8_t inv = ~port_val; // invert all bits

 TWI_MT_Start();
 TWI_Transmit_SLAW(device_address);
 TWI_Transmit_Byte(inv);
 TWI_Stop();

 written_val = port_val;
}
