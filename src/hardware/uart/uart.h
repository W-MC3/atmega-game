/****************************************************************************************
* File:         uart.h
* Author:       Michiel Dirks
* Created on:   09-09-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#ifndef CODE_UART_H
#define CODE_UART_H

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief UART parity configuration options.
 */
typedef enum {
    UART_PARITY_NONE = 0,
    UART_PARITY_EVEN = 1 << UPM01,
    UART_PARITY_ODD  = 1 << UPM01 | 1 << UPM00
} uart_parity_t;

/**
 * @brief UART stop bits configuration options.
 */
typedef enum {
    UART_STOP_1BIT = 0,
    UART_STOPtimer1_config_2BITS = 1 << USBS0
} uart_stop_bits_t;

/**
 * @brief UART character size configuration options.
 */
typedef enum {
    UART_5BITS = 0,
    UART_CS_6BITS = 1 << UCSZ00,
    UART_CS_7BITS = 1 << UCSZ01,
    UART_CS_8BITS = 1 << UCSZ01 | 1 << UCSZ00
} uart_char_size_t;

/**
 * @brief UART initialization parameters.
 */
typedef struct {
    uint32_t baudRate;
    uart_parity_t parity;
    uart_stop_bits_t stopBits;
    uart_char_size_t charSize;
} uart_config_t;

/**
 * @brief Indicates whether the transmitter is idle.
 * @return true if no transmit buffer is active, else false.
 */
bool txAvailable();

/**
 * @brief Initializes the UART peripheral.
 * @param config UART configuration parameters.
 */
void initUart(uart_config_t config);

/**
 * @brief Sends a buffer via UART using interrupt-driven transmission.
 * @param data Pointer to the buffer to send.
 * @param dataLen Number of bytes to send.
 */
void sendUartData(void* data, uint8_t dataLen);

/**
 * @brief Checks if a received byte is available.
 * @return true if RX buffer contains at least one byte.
 */
bool uartDataAvailable();

/**
 * @brief Reads one byte from the RX buffer.
 * @return The byte read, or 0 if buffer is empty.
 */
uint8_t readUartByte();

#ifdef __cplusplus
}
#endif

#endif //CODE_UART_H