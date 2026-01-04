//
// Created by mikai on 1/4/2026.
//

#ifndef ATMEGA_GAME_NPC_H
#define ATMEGA_GAME_NPC_H

#include "gfx/gfx.h"
#include "game/player.h"

typedef struct game_npc {
    gfx_bitmap_t  tex_north;
    gfx_bitmap_t  tex_east;
    gfx_bitmap_t  tex_south;
    gfx_bitmap_t  tex_west;
    gfx_sprite_t  sprite;
} game_npc_t;

void init_npc(game_npc_t* npc);
void move_npc(game_npc_t *npc, e_DIRECTION heading, int16_t x, int16_t y);

#endif //ATMEGA_GAME_NPC_H