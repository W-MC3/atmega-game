//
// Created by mikai on 11/26/2025.
//

#define SDCARD_CS_PIN 4
#define	ILI9341_CS_PIN 10			// <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus)
#define	ILI9341_DC_PIN 9			// <= DC pin (1=data or 0=command indicator line) also called RS

#include <gfx/gfx.h>
#include <SdFat_Adafruit_Fork.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdbool.h>

gfx_scene_t* active_scene;
gfx_rect_t dirty_rects[GFX_TILEMAP_MAX_DIRTY_PER_FRAME];
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
    if (active_scene == NULL) {
        return;
    }

    if ((active_scene->tilemap->flags & GFX_DIRTY_BIT) != 0) {
        tft.fillScreen(ILI9341_BLUE);
        for (int16_t tx = 0; tx < GFX_TILEMAP_WIDTH; tx++) {
            for (int16_t ty = 0; ty < GFX_TILEMAP_HEIGHT; ty++) {
                const int idx = ty * GFX_TILEMAP_WIDTH + tx;
                const int tile = active_scene->tilemap->tiles[idx];

                gfx_draw_tile({ tx, ty }, active_scene->tilemap->kinds[tile], GFX_FULLSCREEN);
            }
        }

        active_scene->tilemap->flags &= ~GFX_DIRTY_BIT;
    }

    for (uint8_t i = 0; i < dirty_rects_count; i++) {
        gfx_rect_t* rect = &dirty_rects[i];
        gfx_vec2_t top_left = gfx_screen_to_world((gfx_vec2_t){ (int16_t)rect->x, (int16_t)rect->y });
        gfx_vec2_t bottom_right = gfx_screen_to_world((gfx_vec2_t){ (int16_t)(rect->x + rect->width), (int16_t)(rect->y + rect->height) });

        for (int16_t tx = top_left.x; tx <= bottom_right.x; tx++) {
            for (int16_t ty = top_left.y; ty <= bottom_right.y; ty++) {
                if (tx < 0 || ty < 0 || tx >= GFX_TILEMAP_WIDTH || ty >= GFX_TILEMAP_HEIGHT) {
                    continue;
                }

                const int idx = ty * GFX_TILEMAP_WIDTH + tx;
                const int tile = active_scene->tilemap->tiles[idx];
                gfx_draw_tile({ tx, ty }, active_scene->tilemap->kinds[tile], *rect);
            }
        }
    }

    for (uint8_t i = 0; i < active_scene->sprite_count; i++) {
        gfx_sprite_t* sprite = active_scene->sprites[i];
        if ((sprite->flags & GFX_DIRTY_BIT) != 0) {
            tft.fillCircle(sprite->position.x, sprite->position.y, 2, ILI9341_RED);
            sprite->flags &= ~GFX_DIRTY_BIT;
        }
    }

    dirty_rects_count = 0;
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
    for (uint8_t i = 0; i < active_scene->sprite_count; i++) {
        if (active_scene->sprites[i] == sprite) {
            for (uint8_t j = i; j < active_scene->sprite_count - 1; j++) {
                active_scene->sprites[j] = active_scene->sprites[j + 1];
            }
            active_scene->sprite_count--;
            gfx_invalidate_sprite(sprite);
            return;
        }
    }
}

void gfx_move_sprite(gfx_sprite_t* sprite, const int16_t x, const int16_t y) {
    gfx_invalidate_sprite(sprite);
    sprite->position.x = x;
    sprite->position.y = y;
}

void gfx_draw_tile(gfx_vec2_t position, gfx_bitmap_t* bitmap, gfx_rect_t rect) {
    File32 f = SD.open(bitmap->filename);
    if (!f) return;

    uint8_t row[bitmap->row_size];

    const gfx_vec2_t screen_pos = gfx_world_to_screen({ position.x, position.y });

    for (int16_t y = GFX_TILEMAP_TILE_HEIGHT - 1; y >= 0; y--) {
        uint32_t rowOffset = bitmap->offset + (uint32_t)(GFX_TILEMAP_TILE_HEIGHT - 1 - y) * bitmap->row_size;
        f.seek(rowOffset);
        f.read(row, bitmap->row_size);

        tft.startWrite();
        for (int16_t x = 0; x < GFX_TILEMAP_TILE_WIDTH; x++) {
            const uint8_t b = row[(x * 3) + 0];
            const uint8_t g = row[(x * 3) + 1];
            const uint8_t r = row[(x * 3) + 2];

            if ((r | g | b) == 0) continue;

            const int16_t sx = screen_pos.x + x - GFX_TILEMP_TILE_HALF_WIDTH;
            const int16_t sy = screen_pos.y + y;

            if (sx < rect.x || sy < rect.y ||
                sx >= rect.x + rect.width ||
                sy >= rect.y + rect.height) {
                continue;
            }

            const uint16_t color = tft.color565(r, g, b);

            tft.writePixel(sx, sy, color);
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
}

void gfx_invalidate_tilemap(gfx_tilemap_t* map) {
    map->flags |= GFX_DIRTY_BIT;

    for (uint8_t i = 0; i < active_scene->sprite_count; i++) {
        gfx_invalidate_sprite(active_scene->sprites[i]);
    }
}

void gfx_push_dirty_rect(const int16_t x, const int16_t y, const int16_t width, const int16_t height) {
    dirty_rects[dirty_rects_count++] = (gfx_rect_t){
        .x = x,
        .y = y,
        .width = width,
        .height = height
    };
}

void gfx_invalidate_tile(gfx_tilemap_t* map, const int16_t tx, const int16_t ty) {
    if (map != active_scene->tilemap) {
        return;
    }

    gfx_vec2_t screen_pos = gfx_world_to_screen((gfx_vec2_t){ tx, ty });
    gfx_push_dirty_rect(
        screen_pos.x - GFX_TILEMP_TILE_HALF_WIDTH,
        screen_pos.y,
        GFX_TILEMAP_TILE_WIDTH,
        GFX_TILEMAP_TILE_HEIGHT
    );
}

void gfx_invalidate_sprite(gfx_sprite_t* sprite) {
    sprite->flags |= GFX_DIRTY_BIT;
    gfx_push_dirty_rect(sprite->position.x - 5, sprite->position.y - 5, 10, 10);
}

gfx_vec2_t gfx_world_to_screen(const gfx_vec2_t vec) {
    const int16_t x = (vec.x - vec.y) * GFX_TILEMP_TILE_HALF_WIDTH;
    const int16_t y = (vec.x + vec.y) * GFX_TILEMP_TILE_HALF_HEIGHT;

    return {
        static_cast<int16_t>(x + tft.width() / 2),
        static_cast<int16_t>(y + tft.height() / 2)
    };
}

gfx_vec2_t gfx_screen_to_world(const gfx_vec2_t vec) {
    const int16_t centered_x = vec.x - (tft.width() / 2);
    const int16_t centered_y = vec.y - (tft.height() / 2);

    const float dvx = (float)centered_x / GFX_TILEMP_TILE_HALF_WIDTH;
    const float dvy = (float)centered_y / GFX_TILEMP_TILE_HALF_HEIGHT;

    const float fx = (dvx + dvy) / 2.0f;
    const float fy = (dvy - dvx) / 2.0f;

    return { (int16_t)fx, (int16_t)fy };
}