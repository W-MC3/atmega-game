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
    // TODO: impl
    return 0;
}

uint8_t proto_get_uint8(proto_packet_t* packet, uint8_t idx) {
    // TODO: impl
    return 0;
}