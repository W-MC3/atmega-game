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
static proto_packet_t current_packet;

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

    recv_buffer[recv_index++] = byte;

    if (recv_index >= PROTO_PACKET_SIZE) {
        current_packet.opcode = recv_buffer[0];
        current_packet.id = recv_buffer[1];
        current_packet.crc = recv_buffer[2]; // TODO: validate CRC

        for (uint8_t i = 0; i < PROTO_PACKET_MAX_DATA_SIZE; i++) {
            current_packet.data[i] = recv_buffer[3 + i];
        }

        packet_ready = true;
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
    static uint8_t packet_id = 0;

    uint8_t crc = op ^ packet_id;
    for (uint8_t i = 0; i < PROTO_PACKET_MAX_DATA_SIZE; i++) {
        crc ^= data[i];
    }

    uint8_t buf[PROTO_PACKET_SIZE] = { op, packet_id++, crc };
    memcpy(&buf[3], data, PROTO_PACKET_MAX_DATA_SIZE); // copies in the remaining data

    while (!txAvailable()) {} // perhaps we should use a callback or interrupt?
    sendUartData(buf, PROTO_PACKET_SIZE);
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