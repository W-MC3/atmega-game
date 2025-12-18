//
// Created by mikai on 12/18/2025.
//

#include <stdbool.h>
#include <stdint.h>
#include "proto.h"

void proto_init() {
    // TODO: impl
}

void proto_recv_byte(unsigned char byte) {
    // TODO: impl
}

bool proto_has_packet() {
    // TODO: impl
    return false;
}

proto_packet_t proto_get_packet() {
    // TODO: impl
    return { 0 };
}

void proto_emit(uint8_t op) {
    // TODO: impl
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