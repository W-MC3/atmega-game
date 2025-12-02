//
// Created by mikai on 11/26/2025.
//

#define SDCARD_CS_PIN   4
#define	ILI9341_CS_PIN		10			// <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus)
#define	ILI9341_DC_PIN		9			// <= DC pin (1=data or 0=command indicator line) also called RS

#include <gfx/gfx.h>
#include <SdFat_Adafruit_Fork.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdbool.h>

gfx_scene_t* active_scene;
gfx_vec2_t dirty_rects[GFX_TILEMAP_MAX_DIRTY_PER_FRAME];
uint8_t dirty_rects_count = 0;
SdFat32 SD;

Adafruit_ILI9341 tft = Adafruit_ILI9341(ILI9341_CS_PIN, ILI9341_DC_PIN);

void gfx_init() {
    if(!SD.begin(SDCARD_CS_PIN, SD_SCK_MHZ(25))) {
        for(;;);
    }

    tft.begin();

    _delay_ms(200);
    gfx_reset();
}

int gfx_init_bitmap(gfx_bitmap_t* bitmap) {
    File32 f = SD.open(bitmap->filename);
    if (!f) return SD.sdErrorCode();

    uint8_t h[54];
    f.read(h, 54);
    bitmap->offset = *(uint32_t*)(h + 10);
    bitmap->row_size = ((GFX_TILEMAP_TILE_WIDTH * 3) + 3) & ~3;
    f.close();

    return 0;
}

void gfx_frame() {
    dirty_rects_count = 0;

    if (active_scene == NULL) {
        return;
    }

    if ((active_scene->tilemap->flags & GFX_DIRTY_BIT) != 0) {
        tft.fillScreen(ILI9341_BLUE);
        for (int16_t tx = 0; tx < GFX_TILEMAP_WIDTH; tx++) {
            for (int16_t ty = 0; ty < GFX_TILEMAP_HEIGHT; ty++) {
                const int idx = ty * GFX_TILEMAP_WIDTH + tx;
                const int tile = active_scene->tilemap->tiles[idx];

                gfx_draw_tile({ tx, ty }, { GFX_TILEMAP_TILE_WIDTH, GFX_TILEMAP_TILE_HEIGHT }, active_scene->tilemap->kinds[tile], { 0, 0, });
            }
        }
        active_scene->tilemap->flags &= ~GFX_DIRTY_BIT;
        return;
    }
}

void gfx_reset() {
    active_scene = NULL;
    dirty_rects_count = 0;
    tft.fillScreen(ILI9341_BLUE);
}

void gfx_add_sprite(gfx_sprite_t* sprite) {
    if (active_scene == NULL) {
        return;
    }

    if (active_scene->sprite_count >= GFX_SCENE_MAX_SPRITES) {
        return;
    }

    active_scene->sprites[active_scene->sprite_count++] = sprite;
    gfx_invalidate_sprite(sprite);
}

void gfx_remove_sprite(gfx_sprite_t* sprite) {
    // TODO: impl
}

void gfx_move_sprite(gfx_sprite_t* sprite, const int16_t x, const int16_t y) {
    sprite->position.x = x;
    sprite->position.y = y;
    gfx_invalidate_sprite(sprite);
}

void gfx_draw_tiles(gfx_vec2_t* positions, gfx_bitmap_t* bitmap, uint8_t count) {
    // TODO: impl
}

void gfx_draw_tile(gfx_vec2_t position, gfx_vec2_t size, gfx_bitmap_t* bitmap, gfx_vec2_t tex_offset) {
    File32 f = SD.open(bitmap->filename);
    if (!f) return;

    uint8_t row[bitmap->row_size];
    int16_t cX = tft.width() / 2 - GFX_TILEMP_TILE_HALF_WIDTH;

    for (int16_t y = size.y - 1; y >= 0; y--) {
        uint32_t rowOffset = bitmap->offset + (uint32_t)(size.y - 1 - y) * bitmap->row_size;
        f. seek(rowOffset);
        f.read(row, bitmap->row_size);

        tft.startWrite();
        for (int16_t x = 0; x < size.x; x++) {
            const uint8_t b = row[(x * 3) + 0];
            const uint8_t g = row[(x * 3) + 1];
            const uint8_t r = row[(x * 3) + 2];

            if ((r | g | b) == 0) continue;

            const uint16_t color = tft.color565(r, g, b);
            const gfx_vec2_t screen_pos = gfx_world_to_screen({ position.x, position.y });

            tft.writePixel(cX + screen_pos. x + x, screen_pos.y + y + (tft.height() / 2), color);
        }
        tft.endWrite();
    }

    f.close();
}

void gfx_set_tilemap(gfx_tilemap_t* map) {
    if (active_scene == NULL) {
        return;
    }

    active_scene->tilemap = map;
    gfx_invalidate_tilemap(map);
}

void gfx_set_scene(gfx_scene_t* scene) {
    active_scene = scene;

    gfx_invalidate_tilemap(scene->tilemap);
    for (uint8_t i = 0; i < scene->sprite_count; i++) {
        gfx_invalidate_sprite(scene->sprites[i]);
    }
}

void gfx_invalidate_tilemap(gfx_tilemap_t* map) {
    map->flags |= GFX_DIRTY_BIT;
}

void gfx_push_dirty_tile(const int16_t tx, const int16_t ty) {
    if (dirty_rects_count >= GFX_TILEMAP_MAX_DIRTY_PER_FRAME) {
        return;
    }

    if (tx > GFX_TILEMAP_WIDTH || ty > GFX_TILEMAP_HEIGHT) {
        return;
    }

    if (tx < 0 || ty < 0) {
        return;
    }

    dirty_rects[dirty_rects_count++] = { tx, ty };
}

void gfx_push_dirty_rect(const int16_t x, const int16_t y, const int16_t width, const int16_t height) {
    const int16_t min_x = x;
    const int16_t min_y = y;
    const int16_t max_x = x + width;
    const int16_t max_y = y + height;

    const gfx_vec2_t min_world = gfx_screen_to_world({ min_x, min_y });
    const gfx_vec2_t max_world = gfx_screen_to_world({ max_x, max_y });

    for (int16_t ty = min_world.y; ty <= max_world.y; ty++) {
        for (int16_t tx = min_world.x; tx <= max_world.x; tx++) {
            gfx_push_dirty_tile(tx, ty);
        }
    }
}

void gfx_invalidate_tile(gfx_tilemap_t* map, const int16_t tx, const int16_t ty) {
    if (map != active_scene->tilemap) {
        map->flags |= GFX_DIRTY_BIT;
        return;
    }

    gfx_push_dirty_tile(tx, ty);
}

void gfx_invalidate_sprite(gfx_sprite_t* sprite) {
    sprite->flags |= GFX_DIRTY_BIT;
}

gfx_vec2_t gfx_world_to_screen(const gfx_vec2_t vec) {
    const int16_t x = (vec.x - vec.y) * GFX_TILEMP_TILE_HALF_WIDTH;
    const int16_t y = (vec.x + vec.y) * GFX_TILEMP_TILE_HALF_HEIGHT;

    return { x, y };
}

gfx_vec2_t gfx_screen_to_world(const gfx_vec2_t vec) {
    const int16_t dvx = vec.x / GFX_TILEMP_TILE_HALF_WIDTH;
    const int16_t dvy = vec.y / GFX_TILEMP_TILE_HALF_HEIGHT;

    const int16_t x = (dvx + dvy) / 2;
    const int16_t y = (dvy - dvx) / 2;

    return { x, y };
}