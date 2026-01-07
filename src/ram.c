//
// Created by Michiel Dirks on 07/01/2026.
//

#include <stdio.h>
#include <string.h>

#include "hardware/uart/uart.h"
extern char __heap_start;
extern char *__brkval;

int freeRam(void)
{
    char top;
    return &top - (__brkval == 0 ? &__heap_start : __brkval);
}

void print_ram(void) {
    int free = freeRam();
    char msg[16];
    snprintf(msg, sizeof(msg), "RAM: %d\n", free);
    sendUartData(msg, strlen(msg));
}