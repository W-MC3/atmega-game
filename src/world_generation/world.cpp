#include "world.h"
#include <Arduino.h>

/* =========================================================
   CONFIGURATION
   ========================================================= */

#define WATER_CHANCE 25 /* Percentage kans op water tegel */
#define STONE_CHANCE 35 /* Percentage kans op steen tegel */
#define TRAP_ROW_1 4    /* Eerste rij met valstrikken */
#define TRAP_ROW_2 7    /* Tweede rij met valstrikken */

/* =========================================================
   BITMAPS
   ========================================================= */
static gfx_bitmap_t bmp_grass = {"GRASS.BMP"};
static gfx_bitmap_t bmp_water = {"WATER.BMP"};
static gfx_bitmap_t bmp_tile = {"TILE.BMP"};
static gfx_bitmap_t bmp_rock = {"STONE.bmp"};

/* =========================================================
   TILEMAP
   ========================================================= */
static gfx_tilemap_t world_map = {
    GFX_DIRTY_BIT,
    {&bmp_grass, &bmp_water, &bmp_tile, &bmp_rock},
    {0}};

/* =========================================================
   DETERMINISTIC RNG
   ========================================================= */
static const uint8_t rng_pool[30] = {
    42, 189, 7, 201, 55, 99, 12, 230, 150, 76,
    88, 33, 111, 250, 19, 145, 67, 200, 10, 123,
    5, 212, 90, 178, 44, 160, 29, 81, 255, 14};

static int rng_index = 0;

int get_fixed_random(int min, int max)
{
    uint8_t raw_val = rng_pool[rng_index];
    rng_index = (rng_index + 1) % 30;

    int range = max - min;
    if (range <= 0)
        return min;

    return (raw_val % range) + min;
}

void world_set_seed(uint32_t seed) { rng_index = 0; }
uint32_t world_get_seed(void) { return 0; }

/* =========================================================
   WORLD GENERATION LOGIC
   ========================================================= */

void world_generate_new(void)
{
    int path_x = get_fixed_random(1, GFX_TILEMAP_WIDTH - 1);

    for (int y = GFX_TILEMAP_HEIGHT - 1; y >= 0; y--)
    {
        /* Pad berekening */
        int r_dir = get_fixed_random(0, 3);
        path_x += (r_dir - 1);

        if (path_x < 1)
            path_x = 1;
        if (path_x > GFX_TILEMAP_WIDTH - 2)
            path_x = GFX_TILEMAP_WIDTH - 2;

        for (int x = 0; x < GFX_TILEMAP_WIDTH; x++)
        {
            uint8_t tile = TILE_GRASS;

            /* ZONES */
            bool safe_zone_bottom = (y < 3);
            bool safe_zone_top = (y >= GFX_TILEMAP_HEIGHT - 3);

            /* TRAP RIJEN */
            bool is_trap_row = (y == TRAP_ROW_1 || y == TRAP_ROW_2);

            /* LOGICA */
            if (safe_zone_bottom || safe_zone_top)
            {
                tile = TILE_GRASS;
            }
            else if (is_trap_row)
            {
                /* === DE MASSIEVE MUUR === */
                /* GEEN uitzondering meer voor het pad! Alles is Trap. */
                tile = TILE_TILE;
            }
            else
            {
                /* Normale rijen */
                if (x == path_x)
                {
                    tile = TILE_GRASS;
                }
                else
                {
                    int r = get_fixed_random(0, 100);
                    if (r < WATER_CHANCE)
                        tile = TILE_WATER;
                    else if (r < STONE_CHANCE)
                        tile = TILE_STONE;

                    else
                        tile = TILE_GRASS;
                }
            }

            world_map.tiles[y * GFX_TILEMAP_WIDTH + x] = tile;
        }
    }

    world_map.flags |= GFX_DIRTY_BIT;
}

/* =========================================================
   API
   ========================================================= */

void world_next_level(void)
{
    world_generate_new();
}

void world_init(void)
{
    gfx_init_bitmap(&bmp_grass);
    gfx_init_bitmap(&bmp_water);
    gfx_init_bitmap(&bmp_tile);
    gfx_init_bitmap(&bmp_rock);

    rng_index = 0;
    world_generate_new();
}

void world_update(void)
{
    /* Statisch */
}

gfx_tilemap_t *world_get_tilemap(void)
{
    return &world_map;
}