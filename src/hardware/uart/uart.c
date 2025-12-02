/****************************************************************************************
* File:         uart.c
* Author:       Michiel Dirks
* Created on:   09-09-2025
* Company:      Windesheim
* Website:      https://www.windesheim.nl/opleidingen/voltijd/bachelor/ict-zwolle
****************************************************************************************/

#include <stdlib.h>
#include <avr/interrupt.h>
#include "uart.h"

#define RX_BUFFER_SIZE 120

#define CLOCK_PRESCALER 16

//Circular FIFO buffer
typedef struct {
    uint8_t buffer[RX_BUFFER_SIZE];
    uint8_t head; // next byte to read
    uint8_t tail; // next free position to write
    bool bufferOverrun;
} receiveBuffer_s;

volatile receiveBuffer_s rxBuffer = { {0}, 0, 0, false };

typedef struct {
    volatile void* bufferStart;
    volatile uint16_t bufferSize;
    volatile uint16_t currentIndex;
} sendBuffer_s;

volatile sendBuffer_s dataToSend = { NULL, 0, 0 };

/**
 * @brief Initializes UART0 with the provided configuration and enables interrupts.
 */
void initUart(uart_config_t config) {
    const uint16_t ubrr = F_CPU / CLOCK_PRESCALER / config.baudRate - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);  // Shift by 8 to get the high byte
    UBRR0L = (uint8_t)ubrr;

    UCSR0A = 0;
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0); // RX-complete interrupt enabled, TX-complete interrupt enabled

    UCSR0C = config.parity | config.stopBits | config.charSize;

    sei();
}

/**
 * @brief UART data register empty handler to feed bytes from the TX buffer.
 */
void sendNextFromBuffer() {
    if (dataToSend.currentIndex < dataToSend.bufferSize) {
        const uint8_t *buf = (uint8_t*)dataToSend.bufferStart;
        UDR0 = buf[dataToSend.currentIndex++];
    }
    else {
        dataToSend.bufferStart = NULL;
        dataToSend.bufferSize = 0;
        dataToSend.currentIndex = 0;
        UCSR0B &= ~(1 << UDRIE0);
    }
}

/**
 * @brief Queues a buffer for transmission via UART using UDRE interrupts.
 *
 * The user MUST check whether the buffer is ready to send new data by calling
 * txAvailable() before invoking this function; otherwise, the previous message
 * may not be sent completely.
 */
void sendUartData(void* data, uint8_t dataLen) {
    uint8_t sreg = SREG;
    cli();
    dataToSend.bufferStart = data;
    dataToSend.bufferSize = dataLen;
    dataToSend.currentIndex = 0;
    UCSR0B |= (1 << UDRIE0);
    SREG = sreg;
}

/**
 * @brief Reports whether a TX transfer is active.
 */
bool txAvailable() {
    return dataToSend.bufferSize == 0;
}

/**
 * @brief Indicates whether RX buffer contains at least one byte.
 */
bool uartDataAvailable() {
    return rxBuffer.head != rxBuffer.tail;
}

/**
 * @brief Reads next byte from RX circular buffer, or 0 if empty.
 */
uint8_t readUartByte() {
    if (rxBuffer.head == rxBuffer.tail) {
        return 0;
    }
    uint8_t byte = rxBuffer.buffer[rxBuffer.head];
    rxBuffer.head = (rxBuffer.head + 1) % RX_BUFFER_SIZE;
    return byte;
}


/**
 * @brief Returns and clears the RX overrun flag.
 */
bool receiveOverrun() {
    const bool overrun = rxBuffer.bufferOverrun;
    rxBuffer.bufferOverrun = false;
    return overrun;
}

ISR(USART_RX_vect) {
    const uint8_t receivedByte = UDR0;
    const uint8_t nextTail = (rxBuffer.tail + 1) % RX_BUFFER_SIZE;

    if (nextTail == rxBuffer.head) {
        // Overwrite and set flag when full
        rxBuffer.head = (rxBuffer.head + 1) % RX_BUFFER_SIZE;
        rxBuffer.bufferOverrun = true;
    }

    rxBuffer.buffer[rxBuffer.tail] = receivedByte;
    rxBuffer.tail = nextTail;
}

ISR(USART_TX_vect) {
    (void)UCSR0A;
}

ISR(USART_UDRE_vect) {
    sendNextFromBuffer();
}
