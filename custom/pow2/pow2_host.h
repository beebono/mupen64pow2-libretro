/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - pow2_host.h                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Hosts the mupen64plus-pow2 plugins, foir (video) and arMLEt (RSP), in the
 * libretro core. They are built from their unmodified sources with the
 * symbol prefixes of rename-foir.h / rename-armlet.h, and with dlsym()
 * redirected to pow2_dlsym(), which resolves their core API lookups. */

#ifndef POW2_HOST_H
#define POW2_HOST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Plugin lifetime: after CoreStartup() and before CoreShutdown().
 * user_dir is where the plugins keep their files (arMLEt's ucode cache). */
bool pow2_plugins_startup(const char *user_dir);
void pow2_plugins_shutdown(void);

/* foir options, by their [Video-Foir] key ("Widescreen", "FrameSkip" ...).
 * The value is the option's int: 0/1 for a bool, the index for an enum.
 * Before the ROM is open it is used by the coming launch; once it runs, the
 * options foir marks live take effect at once and the others at the next
 * launch. Must run on the emulation thread between frames once the ROM is
 * open, which retro_run() is. Returns false for an unknown key or value. */
bool pow2_foir_set(const char *key, int value);

/* arMLEt options, by their [Rsp-Armlet] key ("GfxMode" ...); read when the
 * ROM is opened. */
bool pow2_armlet_set(const char *key, const char *value);

/* foir's last presented frame, in XRGB8888 (pow2_video.c) */
struct pow2_frame
{
   const uint32_t *pixels;   /* NULL when no frame was presented yet */
   unsigned width, height;
   size_t pitch;             /* bytes */
   float aspect;             /* display aspect ratio */
};

const struct pow2_frame *pow2_video_frame(void);
#define POW2_VIDEO_MAX_WIDTH  1024
#define POW2_VIDEO_MAX_HEIGHT 768

#ifdef __cplusplus
}
#endif

#endif
