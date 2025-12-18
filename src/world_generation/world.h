#ifndef WORLD_H
#define WORLD_H

#include <gfx/gfx.h>
#include <stdint.h>
#include <stdbool.h>

/* =========================================================
   TILE TYPES
   ========================================================= */
#define TILE_GRASS 0
#define TILE_WATER 1
#define TILE_TILE 2
#define TILE_STONE 3

/* World module API */
void world_generate_new(void);
void world_init(void);
gfx_tilemap_t *world_get_tilemap(void);

/* Seed control for reproducible generation */
void world_set_seed(uint32_t seed);
uint32_t world_get_seed(void);

/* Spawn & regenerate helpers */
gfx_vec2_t world_get_spawn_tile(void);
void world_regenerate_and_respawn(gfx_sprite_t *player);
bool world_is_regenerating(void);

void world_next_level(void);

#endif /* WORLD_H */