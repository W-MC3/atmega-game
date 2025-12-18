#include "world.h"
#include "resources.h"
#include "game/game_state.h"

/* =========================================================
   CONFIGURATION
   ========================================================= */

#define WATER_CHANCE 25
#define STONE_CHANCE 35
#define TRAP_ROW_1 4
#define TRAP_ROW_2 7

/* =========================================================
   BITMAPS
   ========================================================= */
static gfx_bitmap_t bmp_grass = {GRASS_TILE};
static gfx_bitmap_t bmp_water = {WATER_TILE};
static gfx_bitmap_t bmp_tile = {CHECKERBOARD_TILE};
static gfx_bitmap_t bmp_rock = {STONE_TILE};

static gfx_tilemap_t world_map = {
    GFX_DIRTY_BIT,
    {&bmp_grass, &bmp_water, &bmp_tile, &bmp_rock},
    {0}
};

/* =========================================================
   PRNG
   ========================================================= */
static uint8_t rng_counter = 0;

uint8_t get_rng_value(uint8_t idx) {
    uint16_t val = (uint16_t)(idx + 55);
    return (val * 13) >> 1;
}

int get_fixed_random(int min, int max) {
    uint8_t raw_val = get_rng_value(rng_counter++);
    int range = max - min;
    if (range <= 0)
        return min;
    return (raw_val % range) + min;
}

void world_set_seed(uint32_t seed) {
    rng_counter = 0;
}

/* =========================================================
   WORLD GENERATION LOGIC
   ========================================================= */

void world_generate_new(void) {
    int path_x = get_fixed_random(1, GFX_TILEMAP_WIDTH - 2); // Pad iets minder aan rand

    for (int y = GFX_TILEMAP_HEIGHT - 1; y >= 0; y--)
    {
        /* Pad loopt door */
        int r_dir = get_fixed_random(0, 3);
        path_x += (r_dir - 1);

        if (path_x < 1)
            path_x = 1;
        if (path_x > GFX_TILEMAP_WIDTH - 3)
            path_x = GFX_TILEMAP_WIDTH - 3; // Iets meer ruimte rechts

        for (int x = 0; x < GFX_TILEMAP_WIDTH; x++)
        {
            uint8_t tile = TILE_GRASS;

            bool safe_zone_bottom = (y < 3);
            bool safe_zone_top = (y >= GFX_TILEMAP_HEIGHT - 3);
            bool is_trap_row = (y == TRAP_ROW_1 || y == TRAP_ROW_2);

            if (safe_zone_bottom || safe_zone_top)
            {
                tile = TILE_GRASS;
            }
            else if (is_trap_row)
            {
                /* Muur van traps */
                tile = TILE_TILE;
            }
            else
            {
                /* === VEILIG PAD LOGICA === */
                /* We maken het pad nu 2 tegels breed (x en x+1) */
                /* Of we checken gewoon of x dichtbij path_x is */
                if (x == path_x)
                {
                    tile = TILE_GRASS; /* Altijd begaanbaar */
                }
                else
                {
                    // Clear tile flags
                    tile_flags[y * GFX_TILEMAP_WIDTH + x] = 0;

                    /* Random obstakels */
                    int r = get_fixed_random(0, 100);
                    if (r < WATER_CHANCE) {
                        tile = TILE_WATER;
                        tile_flags[y * GFX_TILEMAP_WIDTH + x] |= TILE_DEADLY_FLAG;
                    }
                    else if (r < STONE_CHANCE) {
                        tile = TILE_STONE;
                        tile_flags[y * GFX_TILEMAP_WIDTH + x] |= TILE_INACCESSIBLE_FLAG;
                    }
                    else {
                        tile = TILE_GRASS;
                    }
                }
            }

            world_map.tiles[y * GFX_TILEMAP_WIDTH + x] = tile;
        }
    }

    world_map.flags |= GFX_DIRTY_BIT;
}

void world_next_level(void) {
    world_generate_new();
}

void world_init(void) {
    gfx_init_bitmap(&bmp_grass);
    gfx_init_bitmap(&bmp_water);
    gfx_init_bitmap(&bmp_tile);
    gfx_init_bitmap(&bmp_rock);

    world_set_seed(0);
    world_generate_new();
}

gfx_tilemap_t *world_get_tilemap(void) {
    return &world_map;
}