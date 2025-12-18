//
// Created by mikai on 12/18/2025.
//

#ifndef ATMEGA_GAME_PROTO_H
#define ATMEGA_GAME_PROTO_H

#include <stdint.h>
#include <stdbool.h>

#define PROTO_PACKET_MAX_DATA_SIZE 4

#define CMD_NOOP          0xFF // NO-OP
// note: can be added if needed later on, realistically, it may not make sense... ~mikaib
// #define CMD_NACK          0x00 // Not ACKnowledge packet
// #define CMD_ACK           0x01 // ACKnowledge packet
#define CMD_PING          0x02 // Ping packet (no data)
#define CMD_SEED          0x03 // RNG seed (1x uint32_t)
#define CMD_MOVE          0x04 // Player pos (2x uint8_t)
#define CMD_HEALTH        0x05 // Health update (1x uint8_t)
#define CMD_READY         0x06 // Ready status (1x uint8_t)
#define CMD_START         0x07 // Start game (no data)
#define CMD_ACTIVATE_TRAP 0x08 // Activate trap (1x uint8_t)

typedef struct proto_packet {
    uint8_t opcode;
    uint8_t id;
    uint8_t crc;
    uint8_t data[PROTO_PACKET_MAX_DATA_SIZE];
} proto_packet_t;

// Initializes the networking subsystem
void proto_init();

// The function to call to push a byte into the networking subsystem
void proto_recv_byte(uint8_t byte);

// Check if the protocol has a completed packet
bool proto_has_packet();

// Get the last packet that has been received.
proto_packet_t proto_get_packet();

// Emit a packet
void proto_emit(uint8_t op, uint8_t data[PROTO_PACKET_MAX_DATA_SIZE]);

// Get uint32_t from packet at position of idx
uint32_t proto_get_uint32(proto_packet_t* packet, uint8_t idx);

// Get uint8_t from packet at position of idx
uint8_t proto_get_uint8(proto_packet_t* packet, uint8_t idx);

#endif //ATMEGA_GAME_PROTO_H