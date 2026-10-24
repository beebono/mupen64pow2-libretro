/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - pow2_video.c                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* foir's video output for libretro, in place of its gl_out.c: the frame foir
 * presents is converted to XRGB8888 and handed to the frontend by retro_run().
 * Scaling, filtering and vsync are the frontend's. */

#include <string.h>

#include "../../mupen64plus-rdp-foir/src/gl_out.h"
#include "pow2_host.h"

extern bool libretro_swap_buffer;

static uint32_t l_pixels[POW2_VIDEO_MAX_WIDTH * POW2_VIDEO_MAX_HEIGHT];
static struct pow2_frame l_frame;
static bool l_active, l_widescreen;

bool gl_out_init(void *core, const struct gl_out_params *p)
{
    (void)core;
    l_widescreen = p->widescreen;
    l_active = true;
    return true;
}

void gl_out_shutdown(void)
{
    l_active = false;
}

bool gl_out_active(void) { return l_active; }
void gl_out_set_filter(bool on) { (void)on; }
void gl_out_set_widescreen(bool on) { l_widescreen = on; }
void gl_out_resize(int w, int h) { (void)w; (void)h; }
void gl_out_toggle_fullscreen(void) { }

/* rgba: RGBA8888 (R in the low byte) or, with raw16, the VI's RGBA5551
 * halfwords. screen_w / screen_h: the part of the 4:3 (or 16:9) screen
 * the image covers, for a VI that does not scan out the whole screen. */
void gl_out_present(const void *rgba, bool raw16, int w, int h, float screen_w, float screen_h, void (*pre_swap_cb)(int))
{
    size_t i, n;

    if (!l_active)
        return;

    if (!rgba || w <= 0 || h <= 0)
    {
        /* no image (VI blanked): keep the size, show black */
        if (!l_frame.pixels)
        {
            w = 320;
            h = 240;
        }
        else
        {
            w = (int)l_frame.width;
            h = (int)l_frame.height;
        }
        memset(l_pixels, 0, (size_t)w * (size_t)h * sizeof(uint32_t));
        screen_w = screen_h = 1.0f;
    }
    else
    {
        if (w > POW2_VIDEO_MAX_WIDTH)
            w = POW2_VIDEO_MAX_WIDTH;
        if (h > POW2_VIDEO_MAX_HEIGHT)
            h = POW2_VIDEO_MAX_HEIGHT;
        n = (size_t)w * (size_t)h;

        if (raw16)
        {
            const uint16_t *src = (const uint16_t *)rgba;
            for (i = 0; i < n; i++)
            {
                uint32_t p = src[i];
                uint32_t r = (p >> 11) & 31, g = (p >> 6) & 31, b = (p >> 1) & 31;
                r = (r << 3) | (r >> 2);
                g = (g << 3) | (g >> 2);
                b = (b << 3) | (b >> 2);
                l_pixels[i] = (r << 16) | (g << 8) | b;
            }
        }
        else
        {
            const uint32_t *src = (const uint32_t *)rgba;
            for (i = 0; i < n; i++)
            {
                uint32_t p = src[i];
                l_pixels[i] = ((p & 0xff) << 16) | (p & 0xff00) | ((p >> 16) & 0xff);
            }
        }
    }

    l_frame.pixels = l_pixels;
    l_frame.width = (unsigned)w;
    l_frame.height = (unsigned)h;
    l_frame.pitch = (size_t)w * sizeof(uint32_t);
    l_frame.aspect = (l_widescreen ? 16.0f / 9.0f : 4.0f / 3.0f) * screen_w / screen_h;

    if (pre_swap_cb)
        pre_swap_cb(1);
    libretro_swap_buffer = true;
}

const struct pow2_frame *pow2_video_frame(void)
{
    return &l_frame;
}
