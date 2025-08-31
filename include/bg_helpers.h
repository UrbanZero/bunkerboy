// bg_helpers.h
#pragma once
#include <gb/gb.h>
#include <gbdk/platform.h>
#include <stdint.h>

#ifndef FONT_TILES
#define FONT_TILES 96
#endif

// Row-copy with base offset so we don't need a big tmp buffer.
// Supports up to 32 tiles wide (fits GB tilemap row).
static void blit_map_based(uint8_t x, uint8_t y,
                           uint8_t w_tiles, uint8_t h_tiles,
                           const unsigned char *map, uint8_t base)
{
    uint8_t rowbuf[32];
    for (uint8_t r = 0; r < h_tiles; ++r)
    {
        const unsigned char *src = map + (uint16_t)r * w_tiles;
        for (uint8_t c = 0; c < w_tiles; ++c)
            rowbuf[c] = src[c] + base;
        set_bkg_tiles(x, (uint8_t)(y + r), w_tiles, 1, rowbuf);
    }
}

// Show a png2asset-generated background image (tiles + map) at (0,0),
// adding `base` to all tile indices so it won't collide with your font.
// width/height are the *pixel* defines that png2asset generates.
static void bg_show_image_with_base(const unsigned char *tiles, uint16_t tile_count,
                                    const unsigned char *map,
                                    uint16_t width_px, uint16_t height_px,
                                    uint8_t base,
                                    uint8_t hide_window)
{
    if (hide_window)
        HIDE_WIN; // prevent opaque window from covering the BG
    DISPLAY_OFF;

    // Load the tile graphics at the requested base
    set_bkg_data(base, tile_count, tiles);

    // Convert pixels -> tiles (png2asset width/height are in pixels)
    uint8_t w_tiles = (uint8_t)(width_px >> 3);
    uint8_t h_tiles = (uint8_t)(height_px >> 3);

    blit_map_based(0, 0, w_tiles, h_tiles, map, base);

    SHOW_BKG;
    SCX_REG = 0;
    SCY_REG = 0;
    DISPLAY_ON;
}

// Convenience macro for png2asset symbols:
// Example: BG_SHOW_ASSET(door, FONT_TILES, 1 /*hide window*/);
#define BG_SHOW_ASSET(PREFIX, BASE, HIDE_WIN_)                             \
    bg_show_image_with_base(PREFIX##_tiles, PREFIX##_TILE_COUNT,           \
                            PREFIX##_map, PREFIX##_WIDTH, PREFIX##_HEIGHT, \
                            (BASE), (HIDE_WIN_))
