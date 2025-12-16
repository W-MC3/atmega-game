#include "world.h"
#include <Arduino.h>

/* =========================================================
   BITMAPS
   ========================================================= */
static gfx_bitmap_t bmp_grass = {"GRASS.BMP"};
static gfx_bitmap_t bmp_water = {"WATER.BMP"};
static gfx_bitmap_t bmp_tile = {"TILE.BMP"};

/* =========================================================
   TILEMAP
   ========================================================= */
static gfx_tilemap_t world_map = {
    GFX_DIRTY_BIT,
    {&bmp_grass, &bmp_water, &bmp_tile},
    {0}};

/* =========================================================
   DETERMINISTIC RNG (Vaste random getallen)
   ========================================================= */
/*
   Een vaste lijst van 30 willekeurige getallen (0-255).
   Omdat deze hardcoded is, doen beide Arduino's EXACT hetzelfde.
*/
static const uint8_t rng_pool[30] = {
    42, 189, 7, 201, 55, 99, 12, 230, 150, 76,
    88, 33, 111, 250, 19, 145, 67, 200, 10, 123,
    5, 212, 90, 178, 44, 160, 29, 81, 255, 14};

static int rng_index = 0;

/* Hulpfunctie die random() vervangt */
int get_fixed_random(int min, int max)
{
    /* Pak het volgende getal uit de lijst */
    uint8_t raw_val = rng_pool[rng_index];

    /* Schuif index op, en begin opnieuw als we bij 30 zijn (modulo) */
    rng_index = (rng_index + 1) % 30;

    /* Map het getal (0-255) naar de gevraagde range [min, max) */
    int range = max - min;
    if (range <= 0)
        return min;

    return (raw_val % range) + min;
}

/* De seed functies zijn nu eigenlijk overbodig,
   maar we laten ze staan om errors te voorkomen,
   we resetten alleen de index. */
void world_set_seed(uint32_t seed)
{
    // Reset de reeks naar het begin
    rng_index = 0;
}

uint32_t world_get_seed(void)
{
    return 0; // Niet meer relevant
}

/* =========================================================
   WORLD LOGIC
   ========================================================= */
static void generate_row(int y)
{
    // Vervang random() door get_fixed_random()
    int path_x = get_fixed_random(1, GFX_TILEMAP_WIDTH - 1);

    for (int x = 0; x < GFX_TILEMAP_WIDTH; x++)
    {
        uint8_t tile = TILE_GRASS;

        if (x == path_x)
        {
            tile = TILE_WATER;
        }
        else
        {
            // Gebruik onze vaste lijst
            int r = get_fixed_random(0, 10);
            if (r < 3)
                tile = TILE_WATER;
            else if (r < 6)
                tile = TILE_GRASS;
            else
                tile = TILE_TILE;
        }

        world_map.tiles[y * GFX_TILEMAP_WIDTH + x] = tile;
        gfx_invalidate_tile(&world_map, x, y);
    }
}

void world_generate_new(void)
{
    // Start altijd vooraan in de lijst voor een nieuw level?
    // Als je dat wilt, uncomment de volgende regel:
    // rng_index = 0;

    /* We bouwen het level op met de vaste getallen */
    int path_x = get_fixed_random(1, GFX_TILEMAP_WIDTH - 1);

    for (int y = GFX_TILEMAP_HEIGHT - 1; y >= 0; y--)
    {
        int r_dir = get_fixed_random(0, 3); // 0, 1, 2
        path_x += (r_dir - 1);

        if (path_x < 1)
            path_x = 1;
        if (path_x > GFX_TILEMAP_WIDTH - 2)
            path_x = GFX_TILEMAP_WIDTH - 2;

        for (int x = 0; x < GFX_TILEMAP_WIDTH; x++)
        {
            uint8_t tile = TILE_GRASS;

            if (x == path_x)
            {
                tile = TILE_TILE;
            }
            else
            {
                int r = get_fixed_random(0, 100);
                if (r < 20)
                    tile = TILE_WATER;
                else if (r < 40)
                    tile = TILE_TILE;
                else
                    tile = TILE_GRASS;
            }

            world_map.tiles[y * GFX_TILEMAP_WIDTH + x] = tile;
        }
    }
    world_map.flags |= GFX_DIRTY_BIT;
}

void world_next_level(void)
{
    world_generate_new();
}

void world_init(void)
{
    gfx_init_bitmap(&bmp_grass);
    gfx_init_bitmap(&bmp_water);
    gfx_init_bitmap(&bmp_tile);

    // Reset index bij start
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