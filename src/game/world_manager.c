//
// Created by Christian ten Brinke on 18/12/2025.
//

#include "world_manager.h"

#include <stddef.h>

#include "game_state.h"

// ----------- Bitmaps -----------
static gfx_bitmap_t home_bmp;
static gfx_bitmap_t gameover_bmp;

// ----------- Sprites -----------
static gfx_sprite_t home_sprite;
static gfx_sprite_t gameover_sprite;

// ----------- Scenes -----------
static gfx_scene_t home_scene;
static gfx_scene_t game_scene;
static gfx_scene_t gameover_scene;

// ----------- Game scene data (must be static) -----------
static gfx_bitmap_t grass;
static gfx_bitmap_t water;
static gfx_bitmap_t tile;
static gfx_tilemap_t tilemap;

// Current scene tracker
static scene_t current_scene;

// Helper to initialize a fullscreen sprite-based scene
static void init_fullscreen_scene(
    gfx_scene_t* scene,
    gfx_sprite_t* sprite,
    gfx_bitmap_t* bmp,
    const char* filename
) {
    *bmp = (gfx_bitmap_t){ .filename = filename };
    gfx_init_bitmap(bmp);

    *sprite = (gfx_sprite_t){
        .position = {0, 0},
        .size = {320, 240},
        .bitmap = bmp
    };

    *scene = (gfx_scene_t){
        .tilemap = NULL,
        .sprites = { sprite },
        .sprite_count = 1
    };
}

// Initialize the main game scene
void init_game_scene() {
    grass = (gfx_bitmap_t){ .filename = "GRASS.BMP" };
    water = (gfx_bitmap_t){ .filename = "WATER.BMP" };
    tile  = (gfx_bitmap_t){ .filename = "TILE.BMP" };

    gfx_init_bitmap(&grass);
    gfx_init_bitmap(&water);
    gfx_init_bitmap(&tile);

    tilemap = (gfx_tilemap_t){
        .kinds = { &grass, &water, &tile },
        .tiles = {
            2, 2, 2, 2, 2,
            2, 0, 0, 0, 2,
            2, 0, 0, 0, 2,
            2, 0, 1, 0, 2,
            2, 0, 0, 0, 2,
            2, 0, 0, 0, 2,
            2, 2, 2, 2, 2
        }
    };

    tile_flags[17] |= TILE_DEADLY_FLAG;

    game_scene = (gfx_scene_t){
        .tilemap = &tilemap,
        .sprites = { },
        .sprite_count = 0
    };
}

// Initialize all scenes
void world_manager_init(void) {
    current_scene = (scene_t)-1;

    init_fullscreen_scene(&home_scene, &home_sprite, &home_bmp, "HOME.BMP");
    init_fullscreen_scene(&gameover_scene, &gameover_sprite, &gameover_bmp, "GAMEOVER.BMP");

    init_game_scene();
}

// Switch between scenes
void load_scene(scene_t scene_id) {
    if (scene_id == current_scene) return;

    gfx_reset();

    switch (scene_id) {
        case SCENE_HOME:
            gfx_set_scene(&home_scene);
            break;

        case SCENE_GAME:
            gfx_set_scene(&game_scene);
            break;

        case SCENE_GAMEOVER:
            gfx_set_scene(&gameover_scene);
            break;
    }

    current_scene = scene_id;
    gfx_frame();
}