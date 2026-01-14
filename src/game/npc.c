//
// Created by mikai on 1/4/2026.
//

#include <gfx/gfx.h>
#include "npc.h"

void init_npc(game_npc_t* npc) {
    gfx_init_bitmap(&(npc->tex_north));
    gfx_init_bitmap(&(npc->tex_east));
    gfx_init_bitmap(&(npc->tex_south));
    gfx_init_bitmap(&(npc->tex_west));
    move_npc(npc, NORTH, 500, 500);
}

void move_npc(game_npc_t *npc, e_DIRECTION heading, int16_t x, int16_t y) {
    gfx_sprite_t* sprite = &(npc->sprite);
    gfx_move_sprite(sprite, x, y);

    switch (heading) {
        case NORTH: gfx_set_bitmap_sprite(sprite, &(npc->tex_north)); break;
        case EAST: gfx_set_bitmap_sprite(sprite, &(npc->tex_east)); break;
        case SOUTH: gfx_set_bitmap_sprite(sprite, &(npc->tex_south)); break;
        case WEST: gfx_set_bitmap_sprite(sprite, &(npc->tex_west)); break;
        default: break;
    }
}