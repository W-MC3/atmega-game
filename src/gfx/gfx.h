//
// Created by mikai on 11/26/2025.
//

#ifndef ATMEGA_GAME_GFX_H
#define ATMEGA_GAME_GFX_H

#define GFX_DIRTY_BIT 1

#include <stdint.h>

// Represents the perspective of the tilemap
enum gfx_perspective {
    GFX_PERSPECTIVE_2D,
    GFX_PERSPECTIVE_ISO
};

// Represents a bitmap (SD Card handles)
typedef struct gfx_bitmap {
    const char* filename;
} gfx_bitmap_t;

// Represents a tilemap (The world, traps, etc)
typedef struct gfx_tilemap {
    uint8_t flags;
    gfx_perspective perspective;
} gfx_tilemap_t;

// Represents an entity (UI Elements, the player, etc)
typedef struct gfx_sprite {
    uint16_t x;
    uint16_t y;
    uint8_t flags;
    gfx_bitmap_t bitmap;
} gfx_sprite_t;

// Initializes the graphics framework
void gfx_init();

// Renders out the frame
void gfx_frame();

// Resets the currently active scene
void gfx_reset();

// Adds a sprite to the current scene
void gfx_add_sprite(gfx_sprite_t* sprite);

// Removes a sprite from the current scene
void gfx_remove_sprite(gfx_sprite_t* sprite);

// Sets the active tilemap of the scene
void gfx_set_tilemap(gfx_tilemap_t* map);

// Assumes the tilemap as dirty
void gfx_invalidate_tilemap(gfx_bitmap_t* map);

// Assumes a single tile as dirty, must be done if you changed that tile
void gfx_invalidate_tile(gfx_tilemap_t* map, uint16_t tx, uint16_t ty);

// Assumes a sprite as dirty
void gfx_invalidate_sprite(gfx_sprite_t* sprite);

#endif //ATMEGA_GAME_GFX_H