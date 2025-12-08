//
// Created by mikai on 11/26/2025.
//

#ifndef ATMEGA_GAME_GFX_H
#define ATMEGA_GAME_GFX_H

#ifndef GFX_CONFIG_BACKGROUND_COLOUR
#define GFX_CONFIG_BACKGROUND_COLOUR 0x0000
#endif // GFX_CONFIG_BACKGROUND_COLOUR

#ifndef GFX_SCENE_MAX_SPRITES
#define GFX_SCENE_MAX_SPRITES 4
#endif // GFX_SCENE_MAX_SPRITES

#ifndef GFX_TILEMAP_MAX_KINDS
#define GFX_TILEMAP_MAX_KINDS 8
#endif // GFX_TILEMAP_MAX_KINDS

#ifndef GFX_TILEMAP_MAX_DIRTY_PER_FRAME
#define GFX_TILEMAP_MAX_DIRTY_PER_FRAME 8
#endif // GFX_TILEMAP_MAX_KINDS

#ifndef GFX_TILEMAP_WIDTH
#define GFX_TILEMAP_WIDTH 5
#endif // GFX_TILEMAP_WIDTH

#ifndef GFX_TILEMAP_HEIGHT
#define GFX_TILEMAP_HEIGHT 5
#endif // GFX_TILEMAP_HEIGHT

#ifndef GFX_TILEMAP_TILE_WIDTH
#define GFX_TILEMAP_TILE_WIDTH 64
#endif // GFX_TILEMAP_TILE_WIDTH

#ifndef GFX_TILEMAP_TILE_HEIGHT
#define GFX_TILEMAP_TILE_HEIGHT 32
#endif // GFX_TILEMAP_TILE_HEIGHT

#ifndef GFX_TILEMP_TILE_HALF_WIDTH
#define GFX_TILEMP_TILE_HALF_WIDTH (GFX_TILEMAP_TILE_WIDTH / 2)
#endif // GFX_TILEMP_TILE_HALF_WIDTH

#ifndef GFX_TILEMP_TILE_HALF_HEIGHT
#define GFX_TILEMP_TILE_HALF_HEIGHT (GFX_TILEMAP_TILE_HEIGHT / 2)
#endif // GFX_TILEMP_TILE_HALF_HEIGHT

#ifdef __cplusplus
#define GFX_EXTERN_C extern "C"
#else
#define GFX_EXTERN_C
#endif

#define GFX_DIRTY_BIT 1

#include <stdint.h>
#include <stdbool.h>

// Represents a bitmap (SD Card handles)
typedef struct gfx_bitmap {
    const char* filename;
    int32_t offset;
    int32_t row_size;
} gfx_bitmap_t;

// Represents a tilemap (The world, traps, etc), note that every tilemap is of equal size.
typedef struct gfx_tilemap {
    uint8_t flags;
    gfx_bitmap_t* kinds[GFX_TILEMAP_MAX_KINDS];
    uint8_t tiles[GFX_TILEMAP_WIDTH * GFX_TILEMAP_HEIGHT];
} gfx_tilemap_t;

// Represents a 2D vector
typedef struct gfx_vec2 {
    int16_t x;
    int16_t y;
} gfx_vec2_t;

// Represents a rectangle
typedef struct gfx_rect {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} gfx_rect_t;

// Represents an entity (UI Elements, the player, etc)
typedef struct gfx_sprite {
    uint8_t flags;
    gfx_vec2_t position;
    gfx_vec2_t size;
    gfx_bitmap_t* bitmap;
} gfx_sprite_t;

// Represents the scene
typedef struct gfx_scene {
    gfx_tilemap_t* tilemap;
    gfx_sprite_t* sprites[GFX_SCENE_MAX_SPRITES];
    uint8_t sprite_count;
} gfx_scene_t;

// Initializes the graphics framework
GFX_EXTERN_C void gfx_init();

// Renders out the frame
GFX_EXTERN_C void gfx_frame();

// Resets the currently active scene
GFX_EXTERN_C void gfx_reset();

// Loads metadata for a bitmap from the SD card
GFX_EXTERN_C int gfx_init_bitmap(gfx_bitmap_t* bitmap);

// Adds a sprite to the active scene
GFX_EXTERN_C void gfx_add_sprite(gfx_sprite_t* sprite);

// Removes a sprite from the active scene
GFX_EXTERN_C void gfx_remove_sprite(gfx_sprite_t* sprite);

// Moves a sprite to the given position (and invalidates it)
GFX_EXTERN_C void gfx_move_sprite(gfx_sprite_t* sprite, int16_t x, int16_t y);

// Sets the active tilemap of the active scene
GFX_EXTERN_C void gfx_set_tilemap(gfx_tilemap_t* map);

// Sets the current active scene
GFX_EXTERN_C void gfx_set_scene(gfx_scene_t* scene);

// Assumes the tilemap as dirty
GFX_EXTERN_C void gfx_invalidate_tilemap(gfx_tilemap_t* map);

// Assumes a single tile as dirty, must be done if you changed that tile
GFX_EXTERN_C void gfx_invalidate_tile(gfx_tilemap_t* map, int16_t tx, int16_t ty);

// Assumes a sprite as dirty
GFX_EXTERN_C void gfx_invalidate_sprite(gfx_sprite_t* sprite);

// Unsafely pushes a dirty-rect to the graphics driver
GFX_EXTERN_C void gfx_push_dirty_rect(int16_t x, int16_t y, int16_t width, int16_t height);

// Partially updates a single tile (internals)
GFX_EXTERN_C void gfx_draw_tile(gfx_vec2_t position, gfx_bitmap_t* bitmap, gfx_rect_t rect);

// Fully updates a sprite
GFX_EXTERN_C void gfx_draw_sprite(gfx_sprite_t* sprite);

// TODO: batch API (gfx_draw_tiles)?
// Transform a world to screen coordinate
GFX_EXTERN_C gfx_vec2_t gfx_world_to_screen(gfx_vec2_t vec);

// Transform a screen to world coordinate
GFX_EXTERN_C gfx_vec2_t gfx_screen_to_world(gfx_vec2_t vec);

#ifndef GFX_FULLSCREEN
#define GFX_FULLSCREEN ((gfx_rect_t){ 0, 0, 240, 320 })
#endif

#endif //ATMEGA_GAME_GFX_H