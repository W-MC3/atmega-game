//
// Created by mikai on 11/26/2025.
//

#define SDCARD_CS_PIN 4
#define	ILI9341_CS_PIN 10			// <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus)
#define	ILI9341_DC_PIN 9			// <= DC pin (1=data or 0=command indicator line) also called RS

#include <gfx/gfx.h>
#include <util/delay.h>
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

/*
refer to: https://en.wikipedia.org/wiki/BMP_file_format

Offset (bytes) 	Size (bytes) 	OS/2 1.x BITMAPCOREHEADER[3]
10 	4 	The offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found.
14 	4 	The size of this header (12 bytes)
18 	2 	The bitmap width in pixels (unsigned 16-bit)
20 	2 	The bitmap height in pixels (unsigned 16-bit)
22 	2 	The number of color planes, must be 1
24 	2 	The number of bits per pixel

Please note that we always assume the format to be BGR24, which is the native format of the TFT display.
The relevant ffmpeg command for conversion is: ``ffmpeg -i INPUT.PNG -pix_fmt bgr24 OUTPUT.BMP``
*/
int gfx_init_bitmap(gfx_bitmap_t* bitmap) {
    File32 f = SD.open(bitmap->filename);
    if (!f) return SD.sdErrorCode();

    uint8_t h[54];
    f.read(h, 54);
    bitmap->offset = *(uint32_t*)(h + 10);
    int32_t width = *(int32_t*)(h + 18);
    bitmap->row_size = ((width * 3) + 3) & ~3;
    f.close();

    return 0;
}

void gfx_frame() {
    // exit if no scene is defined
    if (active_scene == NULL) {
        return;
    }

    // whole tilemap is dirty, redraw everything
    if ((active_scene->tilemap->flags & GFX_DIRTY_BIT) != 0) {
        tft.fillScreen(GFX_CONFIG_BACKGROUND_COLOUR);

        // iterate over all tiles and draw them, clip is set to GFX_FULLSCREEN
        for (int16_t tx = 0; tx < GFX_TILEMAP_WIDTH; tx++) {
            for (int16_t ty = 0; ty < GFX_TILEMAP_HEIGHT; ty++) {
                const int idx = ty * GFX_TILEMAP_WIDTH + tx;
                const int tile = active_scene->tilemap->tiles[idx];

                gfx_draw_tile({ tx, ty }, active_scene->tilemap->kinds[tile], GFX_FULLSCREEN);
            }
        }

        // remove dirty bit
        active_scene->tilemap->flags &= ~GFX_DIRTY_BIT;
    }

    // iterate over dirty rects
    for (uint8_t i = 0; i < dirty_rects_count; i++) {
        gfx_rect_t* rect = &dirty_rects[i];

        // bottom-right corner
        int16_t x1 = rect->x + rect->width - 1;
        int16_t y1 = rect->y + rect->height - 1;

        // get the world position of *all* four corners.
        gfx_vec2_t c0 = gfx_screen_to_world((gfx_vec2_t){ rect->x, rect->y });
        gfx_vec2_t c1 = gfx_screen_to_world((gfx_vec2_t){ x1, rect->y });
        gfx_vec2_t c2 = gfx_screen_to_world((gfx_vec2_t){ rect->x, y1 });
        gfx_vec2_t c3 = gfx_screen_to_world((gfx_vec2_t){ x1, y1 });

        // get the min and max tile coordinates between all the four corners
        int16_t tx_min = min(min(c0.x, c1.x), min(c2.x, c3.x));
        int16_t tx_max = max(max(c0.x, c1.x), max(c2.x, c3.x));
        int16_t ty_min = min(min(c0.y, c1.y), min(c2.y, c3.y));
        int16_t ty_max = max(max(c0.y, c1.y), max(c2.y, c3.y));

        // we iterate over all tiles and redraw with the dirty rect as the clip area.
        for (int16_t tx = tx_min; tx <= tx_max; tx++) {
            for (int16_t ty = ty_min; ty <= ty_max; ty++) {
                if (tx < 0 || ty < 0 || tx >= GFX_TILEMAP_WIDTH || ty >= GFX_TILEMAP_HEIGHT) continue;
                const int idx = ty * GFX_TILEMAP_WIDTH + tx; // (x, y) -> idx
                const int tile = active_scene->tilemap->tiles[idx];
                gfx_draw_tile((gfx_vec2_t){ tx, ty }, active_scene->tilemap->kinds[tile], *rect);
            }
        }
    }

    // draw sprites
    for (uint8_t i = 0; i < active_scene->sprite_count; i++) {
        gfx_sprite_t* sprite = active_scene->sprites[i];

        // redraw if dirty
        if ((sprite->flags & GFX_DIRTY_BIT) != 0) {
            gfx_draw_sprite(sprite);

            // remove dirty bit
            sprite->flags &= ~GFX_DIRTY_BIT;
        }
    }

    // clear dirty rects
    dirty_rects_count = 0;
}

void gfx_reset() {
    active_scene = NULL;
    dirty_rects_count = 0;
    tft.fillScreen(GFX_CONFIG_BACKGROUND_COLOUR);
}

bool gfx_add_sprite(gfx_sprite_t* sprite) {
    if (active_scene == NULL) {
        return false;
    }

    if (active_scene->sprite_count >= GFX_SCENE_MAX_SPRITES) {
        return false;
    }

    active_scene->sprites[active_scene->sprite_count++] = sprite;
    gfx_invalidate_sprite(sprite);

    return true;
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

void gfx_set_bitmap_sprite(gfx_sprite_t* sprite, gfx_bitmap_t* bitmap) {
    gfx_invalidate_sprite(sprite);
    sprite->bitmap = bitmap;
}

void gfx_draw_tile(gfx_vec2_t position, gfx_bitmap_t* bitmap, gfx_rect_t rect) {
    File32 f = SD.open(bitmap->filename);
    if (!f) return;

    uint8_t row[bitmap->row_size];

    const gfx_vec2_t screen_pos = gfx_world_to_screen({ position.x, position.y });
    const int16_t tile_x = screen_pos.x - GFX_TILEMP_TILE_HALF_WIDTH;
    const int16_t tile_y = screen_pos.y;

    int16_t start_x = max(0, rect.x - tile_x);
    int16_t start_y = max(0, rect.y - tile_y);
    int16_t end_x = min((int16_t)GFX_TILEMAP_TILE_WIDTH, (int16_t)(rect.x + rect.width - tile_x));
    int16_t end_y = min((int16_t)GFX_TILEMAP_TILE_HEIGHT, (int16_t)(rect.y + rect.height - tile_y));

    start_x = max(start_x, (int16_t)(0 - tile_x));
    start_y = max(start_y, (int16_t)(0 - tile_y));
    end_x = min(end_x, (int16_t)(tft.width() - tile_x));
    end_y = min(end_y, (int16_t)(tft.height() - tile_y));

    if (start_x >= end_x || start_y >= end_y) {
        f.close();
        return;
    }

    for (int16_t y = GFX_TILEMAP_TILE_HEIGHT - 1; y >= 0; y--) {
        if (y < start_y || y >= end_y) continue;

        uint32_t rowOffset = bitmap->offset + (uint32_t)(GFX_TILEMAP_TILE_HEIGHT - 1 - y) * bitmap->row_size;
        f.seek(rowOffset);
        f.read(row, bitmap->row_size);

        int16_t span_start = -1;

        for (int16_t x = start_x; x <= end_x; x++) {
            bool is_transparent = false;

            if (x < end_x) {
                const uint8_t b = row[(x * 3) + 0];
                const uint8_t g = row[(x * 3) + 1];
                const uint8_t r = row[(x * 3) + 2];
                is_transparent = ((r | g | b) == 0);
            } else {
                is_transparent = true;
            }

            if (!is_transparent && span_start == -1) {
                span_start = x;
            } else if ((is_transparent || x == end_x) && span_start != -1) {
                int16_t span_width = x - span_start;
                int16_t draw_x = tile_x + span_start;
                int16_t draw_y = tile_y + y;

                tft.startWrite();
                tft.setAddrWindow(draw_x, draw_y, span_width, 1);

                for (int16_t sx = span_start; sx < x; sx++) {
                    const uint8_t b = row[(sx * 3) + 0];
                    const uint8_t g = row[(sx * 3) + 1];
                    const uint8_t r = row[(sx * 3) + 2];
                    const uint16_t color = tft.color565(r, g, b);
                    tft.pushColor(color);
                }

                tft.endWrite();
                span_start = -1;
            }
        }
    }

    f.close();
}

void gfx_draw_sprite(gfx_sprite_t* sprite) {
    File32 f = SD.open(sprite->bitmap->filename);
    if (!f) return;

    uint8_t row[sprite->bitmap->row_size];

    const int16_t sprite_x = sprite->position.x - (sprite->size.x / 2);
    const int16_t sprite_y = sprite->position.y;

    for (int16_t y = sprite->size.y - 1; y >= 0; y--) {
        uint32_t rowOffset = sprite->bitmap->offset + (uint32_t)(sprite->size.y - 1 - y) * sprite->bitmap->row_size;
        f.seek(rowOffset);
        f.read(row, sprite->bitmap->row_size);

        int16_t span_start = -1;

        for (int16_t x = 0; x <= sprite->size.x; x++) {
            bool is_transparent = false;

            if (x < sprite->size.x) {
                const uint8_t b = row[(x * 3) + 0];
                const uint8_t g = row[(x * 3) + 1];
                const uint8_t r = row[(x * 3) + 2];
                is_transparent = ((r | g | b) == 0);
            } else {
                is_transparent = true;
            }

            if (!is_transparent && span_start == -1) {
                span_start = x;
            } else if ((is_transparent || x == sprite->size.x) && span_start != -1) {
                int16_t span_width = x - span_start;

                tft.startWrite();
                tft.setAddrWindow(sprite_x + span_start, sprite_y + y, span_width, 1);

                for (int16_t sx = span_start; sx < x; sx++) {
                    const uint8_t b = row[(sx * 3) + 0];
                    const uint8_t g = row[(sx * 3) + 1];
                    const uint8_t r = row[(sx * 3) + 2];
                    const uint16_t color = tft.color565(r, g, b);
                    tft.pushColor(color);
                }

                tft.endWrite();
                span_start = -1;
            }
        }
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

void gfx_set_tile(gfx_tilemap_t* map, int16_t tx, int16_t ty, uint8_t kind) {
    map->tiles[ty * GFX_TILEMAP_WIDTH + tx] = kind;
    gfx_invalidate_tile(map, tx, ty);
}

void gfx_invalidate_sprite(gfx_sprite_t* sprite) {
    sprite->flags |= GFX_DIRTY_BIT;
    gfx_push_dirty_rect(
        sprite->position.x - (sprite->size.x / 2),
        sprite->position.y,
        sprite->size.x,
        sprite->size.y
    );
}

gfx_vec2_t gfx_world_to_screen(const gfx_vec2_t vec) {
    const int16_t x = (vec.x - vec.y) * GFX_TILEMP_TILE_HALF_WIDTH;
    const int16_t y = (vec.x + vec.y) * GFX_TILEMP_TILE_HALF_HEIGHT;

    return (gfx_vec2_t){
        .x = (short)(x + tft.width() / 2),
        .y = (short)(y + tft.height() / 2)
    };
}

gfx_vec2_t gfx_screen_to_world(const gfx_vec2_t vec) {
    const float centered_x = vec.x - (tft.width() / 2.0f);
    const float centered_y = vec.y - (tft.height() / 2.0f);

    const float fx = (centered_x / GFX_TILEMP_TILE_HALF_WIDTH  + centered_y / GFX_TILEMP_TILE_HALF_HEIGHT) * 0.5f;
    const float fy = (centered_y / GFX_TILEMP_TILE_HALF_HEIGHT - centered_x / GFX_TILEMP_TILE_HALF_WIDTH) * 0.5f;

    return (gfx_vec2_t){
        .x = (int16_t)floorf(fx),
        .y = (int16_t)floorf(fy)
    };
}
