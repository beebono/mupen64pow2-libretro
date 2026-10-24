/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - vidext_libretro.c                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* foir presents its frames through retro_run() (custom/pow2/pow2_video.c),
 * so the core has no video extension for a front-end to override. */

#include "m64p_types.h"
#include "m64p_vidext.h"
#include "vidext.h"

m64p_error OverrideVideoFunctions(m64p_video_extension_functions *VideoFunctionStruct)
{
    (void)VideoFunctionStruct;
    return M64ERR_UNSUPPORTED;
}
