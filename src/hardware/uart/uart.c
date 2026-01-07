/****************************************************************************************
* File:         uart.c
* Author:       Michiel Dirks
* Created on:   09-09-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include "uart.h"

#define CLOCK_PRESCALER 16

/**
 * @brief Initializes UART0 with the provided configuration (blocking mode, no interrupts).
 */
void initUart(uart_config_t config) {
    const uint16_t ubrr = F_CPU / CLOCK_PRESCALER / config.baudRate - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);  // Shift by 8 to get the high byte
    UBRR0L = (uint8_t)ubrr;

    UCSR0A = 0;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Enable RX and TX, but no interrupts

    UCSR0C = config.parity | config.stopBits | config.charSize;
}

/**
 * @brief Sends a buffer via UART using blocking transmission.
 * @param data Pointer to the buffer to send.
 * @param dataLen Number of bytes to send.
 * 
 * This function blocks until all bytes have been transmitted.
 */
void sendUartData(void* data, uint8_t dataLen) {
    const uint8_t *buf = (const uint8_t*)data;
    
    for (uint8_t i = 0; i < dataLen; i++) {
        // Wait until the transmit buffer is empty
        while (!(UCSR0A & (1 << UDRE0)));
        
        // Send the byte
        UDR0 = buf[i];
    }
    
    // Wait until transmission is complete
    while (!(UCSR0A & (1 << TXC0)));
    
    // Clear the TX complete flag
    UCSR0A |= (1 << TXC0);
}

/**
 * @brief Reports whether the transmitter is ready (always true in blocking mode).
 * @return Always returns true since blocking mode doesn't queue data.
 */
bool txAvailable() {
    // In blocking mode, transmitter is always available
    return true;
}

/**
 * @brief Checks if a received byte is available (non-blocking check).
 * @return true if RX buffer contains at least one byte.
 */
bool uartDataAvailable() {
    return (UCSR0A & (1 << RXC0)) != 0;
}

/**
 * @brief Reads one byte from UART (blocking).
 * @return The byte read.
 * 
 * This function blocks until a byte is received.
 */
uint8_t readUartByte() {
    // Wait until data is received
    while (!(UCSR0A & (1 << RXC0)));
    
    // Read and return the byte
    return UDR0;
}
