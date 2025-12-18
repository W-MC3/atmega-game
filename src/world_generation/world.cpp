#include "world.h"
#include <Arduino.h>

/* =========================================================
   CONFIGURATION
   ========================================================= */

#define WATER_CHANCE 30 /* Percentage kans op water tegel */
#define STONE_CHANCE 40 /* Percentage kans op steen tegel */
#define TRAP_ROW_1 4    /* Eerste rij met valstrikken */
#define TRAP_ROW_2 7    /* Tweede rij met valstrikken */

#define TILE_GRASS 0
#define TILE_WATER 1
#define TILE_TILE 2
#define TILE_STONE 3

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
   PSEUDO RANDOM NUMBER GENERATION (PRNG)
   ========================================================= */
static uint8_t rng_counter = 0;

/*
   PRNG Functie zoals gevraagd:
   Gebruikt index + offset, vermenigvuldigt met 13 en shift naar rechts.
   Dit vervangt de vaste array.
*/
uint8_t get_rng_value(uint8_t idx)
{
    /* We voegen 55 toe als 'seed' offset om variatie te garanderen */
    uint16_t val = (uint16_t)(idx + 55);
    return (val * 13) >> 1;
}

int get_fixed_random(int min, int max)
{
    uint8_t raw_val = get_rng_value(rng_counter++);

    int range = max - min;
    if (range <= 0)
        return min;

    return (raw_val % range) + min;
}

void world_set_seed(uint32_t seed)
{
    rng_counter = 0;
}

uint32_t world_get_seed(void) { return 0; }

/* =========================================================
   WORLD GENERATION LOGIC
   ========================================================= */

void world_generate_new(void)
{
    int path_x = get_fixed_random(1, GFX_TILEMAP_WIDTH - 1);

    for (int y = GFX_TILEMAP_HEIGHT - 1; y >= 0; y--)
    {
        /* Pad berekening (loopt altijd door voor continuïteit) */
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

            if (safe_zone_bottom || safe_zone_top)
            {
                tile = TILE_GRASS;
            }
            else if (is_trap_row)
            {
                /* === MASSIEVE MUUR === */
                /* Hier skippen we het veilige pad: ALLES wordt trap */
                tile = TILE_TILE;
            }
            else
            {
                /* Normale rijen */
                if (x == path_x)
                {
                    tile = TILE_GRASS; /* Veilig pad */
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

    world_set_seed(0);
    world_generate_new();
}

gfx_tilemap_t *world_get_tilemap(void)
{
    return &world_map;
}