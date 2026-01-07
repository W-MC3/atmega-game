//
// Created by mikai on 12/18/2025.
//

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "./../hardware/uart/uart.h"
#include "proto.h"

#define PROTO_PACKET_SIZE (3 + PROTO_PACKET_MAX_DATA_SIZE)

static uint8_t recv_buffer[PROTO_PACKET_SIZE];
static uint8_t recv_index;
static bool packet_ready;
static bool packet_recv_start;
static proto_packet_t current_packet;
static uint8_t packet_id = 0;

void proto_init() {
    recv_index = 0;
    packet_ready = false;
    memset(recv_buffer, 0, sizeof(recv_buffer));
    memset(&current_packet, 0, sizeof(current_packet));
}

void proto_recv_byte(unsigned char byte) {
    if (packet_ready) {
        return;
    }

    if (!packet_recv_start) {
        if (byte == 0xFF) {
            packet_recv_start = true;
        }

        return;
    }

    recv_buffer[recv_index++] = byte;

    if (recv_index >= PROTO_PACKET_SIZE) {
        current_packet.opcode = recv_buffer[0];
        current_packet.id = recv_buffer[1];
        current_packet.crc = recv_buffer[2]; // TODO: validate CRC

        for (uint8_t i = 0; i < PROTO_PACKET_MAX_DATA_SIZE; i++) {
            current_packet.data[i] = recv_buffer[3 + i];
        }

        packet_ready = true;
        packet_recv_start = false;
        recv_index = 0;
    }
}

bool proto_has_packet() {
    return packet_ready;
}

proto_packet_t proto_get_packet() {
    packet_ready = false;
    return current_packet;
}

void proto_emit(uint8_t op, uint8_t data[PROTO_PACKET_MAX_DATA_SIZE]) {
    uint8_t crc = op ^ packet_id;
    for (uint8_t i = 0; i < PROTO_PACKET_MAX_DATA_SIZE; i++) {
        crc ^= data[i];
    }

    uint8_t buf[PROTO_PACKET_SIZE + 1] = { 0 }; // 0xFF = start bit
    buf[0] = 0xFF;
    buf[1] = op;
    buf[2] = packet_id++;
    buf[3] = crc;

    memcpy(&buf[4], data, PROTO_PACKET_MAX_DATA_SIZE); // copies in the remaining data

    while (!txAvailable()) {} // perhaps we should use a callback or interrupt?
    sendUartData(buf, PROTO_PACKET_SIZE + 1);
    while (!txAvailable()) {} // otherwise buf[4] gets evicted from the stack... aaahh
}

uint32_t proto_get_uint32(proto_packet_t* packet, uint8_t idx) {
    const uint8_t* d = packet->data; // idx is ignored but is still present for consistency.
    return d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
}

uint8_t proto_get_uint8(proto_packet_t* packet, uint8_t idx) {
    if (idx >= PROTO_PACKET_MAX_DATA_SIZE || idx < 0) { // packet data format is uint8_t, so size is 1:1
        return 0xFF;
    }

    return packet->data[idx];
}