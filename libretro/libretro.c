/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - libretro.c                                         *
 *   Copyright (C) 2020 M4xw <m4x@m4xw.net>                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <compat/strl.h>

#include "libretro.h"
#include "libretro_private.h"
#include "libretro_core_options.h"

#include "mupen64plus-next_common.h"
#include "pow2/pow2_host.h"

#include <libco.h>

#ifdef HAVE_LIBNX
#include <switch.h>
#endif

#include "api/m64p_frontend.h"
#include "api/m64p_types.h"
#include "device/r4300/r4300_core.h"
#include "device/memory/memory.h"
#include "main/main.h"
#include "api/callbacks.h"
#include "main/cheat.h"
#include "main/version.h"
#include "main/util.h"
#include "main/savestates.h"
#include "main/mupen64plus.ini.h"
#include "api/m64p_config.h"
#include <file/file_path.h>
#include "main/rom.h"
#include "plugin/plugin.h"
#include "device/rcp/pi/pi_controller.h"
#include "device/pif/pif.h"
#include "libretro_memory.h"

#include "audio_plugin.h"

#define PATH_SIZE 2048

#define ISHEXDEC ((codeLine[cursor]>='0') && (codeLine[cursor]<='9')) || ((codeLine[cursor]>='a') && (codeLine[cursor]<='f')) || ((codeLine[cursor]>='A') && (codeLine[cursor]<='F'))

/* Forward declarations */
void inputGetKeys_default_descriptor(void);

struct retro_perf_callback perf_cb;
retro_get_cpu_features_t perf_get_cpu_features_cb = NULL;

retro_log_printf_t log_cb = NULL;
retro_video_refresh_t video_cb = NULL;
retro_input_poll_t poll_cb = NULL;
retro_input_state_t input_cb = NULL;
retro_audio_sample_batch_t audio_batch_cb = NULL;
retro_environment_t environ_cb = NULL;

struct retro_rumble_interface rumble;

save_memory_data saved_memory;

static cothread_t game_thread;
cothread_t retro_thread;

int astick_deadzone;
int astick_sensitivity;
int r_cbutton;
int l_cbutton;
int d_cbutton;
int u_cbutton;
bool alternate_mapping;

static uint8_t* game_data = NULL;
static uint32_t game_size = 0;

static bool     emu_initialized     = false;
static unsigned audio_buffer_size   = 2048;

static bool     initializing         = true;
static bool     load_game_successful = false;

bool libretro_swap_buffer;

/* geometry last reported to the frontend */
static unsigned video_width  = 320;
static unsigned video_height = 240;
static float    video_aspect = 4.0f / 3.0f;
static bool     foir_widescreen = false;

// Savestate globals
bool retro_savestate_complete = false;
int  retro_savestate_result = 0;

// 64DD globals
char* retro_dd_path_img = NULL;
char* retro_dd_path_rom = NULL;

// Other Subsystems
char* retro_transferpak_rom_path = NULL;
char* retro_transferpak_ram_path = NULL;

uint32_t EnableFullspeed = 0;
uint32_t CountPerOp = 0;
uint32_t CountPerOpDenomPot = 0;
uint32_t CountPerScanlineOverride = 0;
uint32_t ForceDisableExtraMem = 0;
uint32_t IgnoreTLBExceptions = 0;

extern struct device g_dev;
extern unsigned int r4300_emumode;
extern struct cheat_ctx g_cheat_ctx;

// after the controller's CONTROL* member has been assigned we can update
// them straight from here...
extern struct
{
    CONTROL *control;
    BUTTONS buttons;
} controller[4];
// ...but it won't be at least the first time we're called, in that case set
// these instead for input_plugin to read.
int pad_pak_types[4];
int pad_present[4] = {1, 1, 1, 1};

static void n64DebugCallback(void* aContext, int aLevel, const char* aMessage)
{
    char buffer[1024];
    snprintf(buffer, 1024, CORE_NAME ": %s\n", aMessage);
    if (log_cb)
        log_cb(RETRO_LOG_INFO, buffer);
}

extern m64p_rom_header ROM_HEADER;

static void setup_variables(void)
{
    bool categoriesSupported = false;

    static const struct retro_controller_description port[] = {
        { "Controller", RETRO_DEVICE_JOYPAD },
        { "RetroPad", RETRO_DEVICE_JOYPAD },
    };

    static const struct retro_controller_info ports[] = {
        { port, 2 },
        { port, 2 },
        { port, 2 },
        { port, 2 },
        { 0, 0 }
    };

    libretro_set_core_options(environ_cb, &categoriesSupported);
    environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

static void cleanup_global_paths()
{
    // Ensure potential leftovers are cleaned up
    if(retro_dd_path_img)
    {
        free(retro_dd_path_img);
        retro_dd_path_img = NULL;
    }

    if(retro_dd_path_rom)
    {
        free(retro_dd_path_rom);
        retro_dd_path_rom = NULL;
    }

    if(retro_transferpak_rom_path)
    {
        free(retro_transferpak_rom_path);
        retro_transferpak_rom_path = NULL;
    }

    if(retro_transferpak_ram_path)
    {
        free(retro_transferpak_ram_path);
        retro_transferpak_ram_path = NULL;
    }
}

static void n64StateCallback(void *Context, m64p_core_param param_type, int new_value)
{
    if(param_type == M64CORE_STATE_LOADCOMPLETE || param_type == M64CORE_STATE_SAVECOMPLETE)
    {
        retro_savestate_complete = true;
        retro_savestate_result = new_value;
    }
}

static bool emu_step_load_data()
{
    log_cb(RETRO_LOG_DEBUG, CORE_NAME ": [EmuThread] M64CMD_ROM_OPEN\n");

    if(CoreDoCommand(M64CMD_ROM_OPEN, game_size, (void*)game_data))
    {
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, CORE_NAME ": failed to load ROM\n");
        goto load_fail;
    }

    free(game_data);
    game_data = NULL;

    log_cb(RETRO_LOG_DEBUG, CORE_NAME ": [EmuThread] M64CMD_ROM_GET_HEADER\n");

    if(CoreDoCommand(M64CMD_ROM_GET_HEADER, sizeof(ROM_HEADER), &ROM_HEADER))
    {
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, CORE_NAME ": failed to query ROM header information\n");
        goto load_fail;
    }

    return true;

load_fail:
    free(game_data);
    game_data = NULL;
    //stop = 1;

    return false;
}

static void emu_step_initialize(void)
{
    if (emu_initialized)
        return;

    emu_initialized = true;

    plugin_connect_all();
}

static void* EmuThreadFunction(void* param)
{
    uint32_t netplay_port = 0;
    uint16_t netplay_player = 1;

    initializing = false;

#if 0
    if (netplay_port)
    {
        uint32_t version;
        if (CoreDoCommand(M64CMD_NETPLAY_GET_VERSION, 0x010000, &version) == M64ERR_SUCCESS)
        {
            log_cb(RETRO_LOG_INFO, "Netplay: using core version %u\n", version);

            if (CoreDoCommand(M64CMD_NETPLAY_INIT, netplay_port, "") == M64ERR_SUCCESS)
                log_cb(RETRO_LOG_INFO, "Netplay: init success\n");

            uint32_t reg_id = 0;
            while (reg_id == 0)
            {
#ifdef __MINGW32__
                rand_s(&reg_id);
#else
                reg_id = rand();
#endif
            }
            reg_id += netplay_player;

            if (CoreDoCommand(M64CMD_NETPLAY_CONTROL_PLAYER, netplay_player, &reg_id) == M64ERR_SUCCESS)
                log_cb(RETRO_LOG_INFO, "Netplay: registered for player %d\n", netplay_player);
        }
    }
#endif

    log_cb(RETRO_LOG_DEBUG, CORE_NAME ": [EmuThread] M64CMD_EXECUTE\n");

    // Runs until CMD_STOP
    CoreDoCommand(M64CMD_EXECUTE, 0, NULL);

    return NULL;
}

const char* retro_get_system_directory(void)
{
    const char* dir;
    environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir);

    return dir ? dir : ".";
}


void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb)   { }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_cb = cb; }

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
    size_t outSize = 0;
    bool result = false;
    void* gameBuffer = NULL;

    cleanup_global_paths();

    switch(game_type)
    {
        case RETRO_GAME_TYPE_DD:
            if(num_info == 1)
            {
                retro_dd_path_img = strdup(info[0].path);
            }
            else if(num_info == 2)
            {
                retro_dd_path_img = strdup(info[0].path);
                retro_dd_path_rom = strdup(info[1].path);
            } else {
                return false;
            }
            
            log_cb(RETRO_LOG_INFO, "Loading %s...\n", info[0].path);
            
            result = load_file(info[1].path, &gameBuffer, &outSize) == file_ok;
            if(result)
            {
               memcpy(&info[1].data, &gameBuffer, sizeof(void*));
               memcpy(&info[1].size, &outSize, sizeof(size_t));
               result = result && retro_load_game(&info[1]);
               
               if(gameBuffer)
               {
                  free(gameBuffer);
                  gameBuffer = NULL;
                  // To prevent potential double free // TODO: revisit later
                  memcpy(&info[1].data, &gameBuffer, sizeof(void*));
               }
            }
            break;
        case RETRO_GAME_TYPE_TRANSFERPAK:
            if(num_info == 3)
            {
                retro_transferpak_ram_path = strdup(info[0].path);
                retro_transferpak_rom_path = strdup(info[1].path);
            } else {
                return false;
            }
            
            log_cb(RETRO_LOG_INFO, "Loading %s...\n", info[0].path);
            log_cb(RETRO_LOG_INFO, "Loading %s...\n", info[1].path);
            log_cb(RETRO_LOG_INFO, "Loading %s...\n", info[2].path);
            result = load_file(info[2].path, &gameBuffer, &outSize) == file_ok;
            if(result)
            {
               memcpy(&info[2].data, &gameBuffer, sizeof(void*));
               memcpy(&info[2].size, &outSize, sizeof(size_t));
               result = result && retro_load_game(&info[2]);
               if(gameBuffer)
               {
                  free(gameBuffer);
                  gameBuffer = NULL;
                  // To prevent potential double free // TODO: revisit later
                  memcpy(&info[2].data, &gameBuffer, sizeof(void*));
               }
            }
            break;
        default:
            return false;
    }

	 return result;
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    static const struct retro_subsystem_memory_info memory_info_dd[] = {
        { "srm", RETRO_MEMORY_DD },
        {}
    };

    static const struct retro_subsystem_memory_info memory_info_transferpak[] = {
        { "srm", RETRO_MEMORY_TRANSFERPAK },
        {}
    };

    static const struct retro_subsystem_rom_info dd_roms[] = {
        { "Disk", "ndd", true, false, true, memory_info_dd, 1 },
        { "Cartridge", "n64|v64|z64|bin|u1", true, false, true, NULL, 0 },
        {}
    };

    static const struct retro_subsystem_rom_info transferpak_roms[] = {
        { "Gameboy RAM", "ram|sav", true, false, true, NULL, 0 },
        { "Gameboy ROM", "rom|gb|gbc", true, false, true, NULL, 0 },
        { "Cartridge", "n64|v64|z64|bin|u1", true, false, true, memory_info_transferpak, 1 },
        {}
    };

    static const struct retro_subsystem_info subsystems[] = {
        { "N64 Disk Drive", "ndd", dd_roms, 2, RETRO_GAME_TYPE_DD },
        { "N64 Transferpak", "gb", transferpak_roms, 3, RETRO_GAME_TYPE_TRANSFERPAK },
        {}
    };

    environ_cb(RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO, (void*)subsystems);
    
    setup_variables();
}

void retro_get_system_info(struct retro_system_info *info)
{
    info->library_name = "Mupen64Plus-pow2";
    info->library_version = "3.0" FLAVOUR_VERSION GIT_VERSION;
    info->valid_extensions = "n64|v64|z64|bin|u1";
    info->need_fullpath = false;
    info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->geometry.base_width   = video_width;
    info->geometry.base_height  = video_height;
    info->geometry.max_width    = POW2_VIDEO_MAX_WIDTH;
    info->geometry.max_height   = POW2_VIDEO_MAX_HEIGHT;
    info->geometry.aspect_ratio = video_aspect;
    info->timing.fps = vi_expected_refresh_rate_from_tv_standard(ROM_PARAMS.systemtype);
    info->timing.sample_rate = 44100.0;
}

unsigned retro_get_region (void)
{
    return ((ROM_PARAMS.systemtype == SYSTEM_PAL) ? RETRO_REGION_PAL : RETRO_REGION_NTSC);
}

void copy_file(char * ininame, char * fileName)
{
    const char* filename = ConfigGetSharedDataFilepath(fileName);
    FILE *fp = fopen(filename, "w");
    if (fp != NULL)    {
        fputs(ininame, fp);
        fclose(fp);
    }
}

void retro_init(void)
{
    char* sys_pathname;
    environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sys_pathname);
    char pathname[PATH_SIZE];
    strncpy(pathname, sys_pathname, PATH_SIZE);
    if (pathname[(strlen(pathname)-1)] != '/' && pathname[(strlen(pathname)-1)] != '\\')
        strcat(pathname, "/");
    strcat(pathname, "Mupen64plus/");
    if (!path_is_directory(pathname))
        path_mkdir(pathname);
    copy_file(inifile, "mupen64plus.ini");

    struct retro_log_callback log;
    unsigned colorMode = RETRO_PIXEL_FORMAT_XRGB8888;

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
        log_cb = log.log;
    else
        log_cb = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf_cb))
        perf_get_cpu_features_cb = perf_cb.get_cpu_features;
    else
        perf_get_cpu_features_cb = NULL;

    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &colorMode);
    environ_cb(RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE, &rumble);
    initializing = true;

    retro_thread = co_active();
    game_thread = co_create(65536 * sizeof(void*) * 16, (void (*)(void))EmuThreadFunction);

    m64p_error ret = CoreStartup(FRONTEND_API_VERSION, ".", ".", NULL, n64DebugCallback, 0, n64StateCallback);
    if(ret && log_cb)
        log_cb(RETRO_LOG_ERROR, CORE_NAME ": failed to initialize core (err=%i)\n", ret);

    // foir and arMLEt keep their files (arMLEt's ucode cache) next to mupen64plus.ini
    if (!ret && !pow2_plugins_startup(pathname) && log_cb)
        log_cb(RETRO_LOG_ERROR, CORE_NAME ": failed to start the plugins\n");
}

void retro_deinit(void)
{
    // Prevent yield to game_thread on unsuccessful load
    if(load_game_successful)
    {
        CoreDoCommand(M64CMD_STOP, 0, NULL);
        co_switch(game_thread); /* Let the core thread finish */
        CoreDoCommand(M64CMD_ROM_CLOSE, 0, NULL);
    }

    pow2_plugins_shutdown();
    CoreShutdown();
    deinit_audio_libretro();

    if (perf_cb.perf_log)
        perf_cb.perf_log();
}

void update_controllers()
{
    struct retro_variable var;

    var.key = CORE_NAME "-pak1";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        int p1_pak = PLUGIN_NONE;
        if (!strcmp(var.value, "rumble"))
            p1_pak = PLUGIN_RAW;
        else if (!strcmp(var.value, "memory"))
            p1_pak = PLUGIN_MEMPAK;
        else if (!strcmp(var.value, "transfer"))
            p1_pak = PLUGIN_TRANSFER_PAK;

        // If controller struct is not initialised yet, set pad_pak_types instead
        // which will be looked at when initialising the controllers.
        if (controller[0].control)
            controller[0].control->Plugin = p1_pak;
        else
            pad_pak_types[0] = p1_pak;
    }

    var.key = CORE_NAME "-pak2";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        int p2_pak = PLUGIN_NONE;
        if (!strcmp(var.value, "rumble"))
            p2_pak = PLUGIN_RAW;
        else if (!strcmp(var.value, "memory"))
            p2_pak = PLUGIN_MEMPAK;
        else if (!strcmp(var.value, "transfer"))
            p2_pak = PLUGIN_TRANSFER_PAK;

        if (controller[1].control)
            controller[1].control->Plugin = p2_pak;
        else
            pad_pak_types[1] = p2_pak;
    }

    var.key = CORE_NAME "-pak3";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        int p3_pak = PLUGIN_NONE;
        if (!strcmp(var.value, "rumble"))
            p3_pak = PLUGIN_RAW;
        else if (!strcmp(var.value, "memory"))
            p3_pak = PLUGIN_MEMPAK;
        else if (!strcmp(var.value, "transfer"))
            p3_pak = PLUGIN_TRANSFER_PAK;

        if (controller[2].control)
            controller[2].control->Plugin = p3_pak;
        else
            pad_pak_types[2] = p3_pak;
    }

    var.key = CORE_NAME "-pak4";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        int p4_pak = PLUGIN_NONE;
        if (!strcmp(var.value, "rumble"))
            p4_pak = PLUGIN_RAW;
        else if (!strcmp(var.value, "memory"))
            p4_pak = PLUGIN_MEMPAK;
        else if (!strcmp(var.value, "transfer"))
            p4_pak = PLUGIN_TRANSFER_PAK;

        if (controller[3].control)
            controller[3].control->Plugin = p4_pak;
        else
            pad_pak_types[3] = p4_pak;
    }

    var.key = CORE_NAME "-astick-deadzone";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        astick_deadzone = (int)(atoi(var.value) * 0.01f * 0x8000);

    var.key = CORE_NAME "-astick-sensitivity";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
        astick_sensitivity = atoi(var.value);

    var.key = CORE_NAME "-r-cbutton";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            r_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }

    var.key = CORE_NAME "-l-cbutton";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            l_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }

    var.key = CORE_NAME "-d-cbutton";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            d_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }

    var.key = CORE_NAME "-u-cbutton";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (!strcmp(var.value, "C1"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_A;
        else if (!strcmp(var.value, "C2"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_Y;
        else if (!strcmp(var.value, "C3"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_B;
        else if (!strcmp(var.value, "C4"))
            u_cbutton = RETRO_DEVICE_ID_JOYPAD_X;
    }

    var.key = CORE_NAME "-alt-map";
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        bool alternate_mapping_prev = alternate_mapping;
        alternate_mapping = !strcmp(var.value, "False") ? 0 : 1;

        if (alternate_mapping != alternate_mapping_prev)
            inputGetKeys_default_descriptor();
    }
}

static const char *get_variable(const char *key)
{
    struct retro_variable var;

    var.key = key;
    var.value = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        return var.value;
    return NULL;
}

/* True/False core options and the foir option each one sets */
static const struct
{
    const char *key;
    const char *foir_key;
} foir_bool_options[] = {
    { CORE_NAME "-foir-widescreen",         "Widescreen" },
    { CORE_NAME "-foir-interlace-hack",     "InterlaceHack" },
    { CORE_NAME "-foir-crop-overscan-h",    "CropOverscanH" },
    { CORE_NAME "-foir-crop-overscan-v",    "CropOverscanV" },
    { CORE_NAME "-foir-vi-antialiasing",    "VIAntiAliasing" },
    { CORE_NAME "-foir-vi-divot",           "VIDivot" },
    { CORE_NAME "-foir-vi-dedither",        "VIDedither" },
    { CORE_NAME "-foir-vi-gamma",           "VIGamma" },
    { CORE_NAME "-foir-fast-combiner",      "FastCombiner" },
    { CORE_NAME "-foir-relaxed-precision",  "RelaxedPrecision" },
    { CORE_NAME "-foir-fast-perspective",   "FastPerspective" },
    { CORE_NAME "-foir-specialize",         "Specialize" },
};

/* arMLEt reads these when the ROM is opened */
static const struct
{
    const char *key;
    const char *armlet_key;
} armlet_options[] = {
    { CORE_NAME "-armlet-gfx-mode",     "GfxMode" },
    { CORE_NAME "-armlet-gfx-probe",    "GfxProbe" },
    { CORE_NAME "-armlet-rdp-coalesce", "RdpCoalesce" },
};

static void update_plugin_variables(void)
{
    const char *value;
    size_t i;

    for (i = 0; i < sizeof(foir_bool_options) / sizeof(foir_bool_options[0]); i++)
    {
        value = get_variable(foir_bool_options[i].key);
        if (value)
            pow2_foir_set(foir_bool_options[i].foir_key, !strcmp(value, "True"));
    }

    value = get_variable(CORE_NAME "-foir-widescreen");
    foir_widescreen = value && !strcmp(value, "True");

    // foir's FrameSkip: 0..3 = fixed skip level, 4 = auto
    value = get_variable(CORE_NAME "-foir-frameskip");
    if (value)
        pow2_foir_set("FrameSkip", !strcmp(value, "auto") ? 4 : atoi(value));

    // foir's Threads: 0 = none (render on the emulation thread), 1 = auto, N + 1 = N workers
    value = get_variable(CORE_NAME "-foir-threads");
    if (value)
        pow2_foir_set("Threads", !strcmp(value, "auto") ? 1 : atoi(value) + 1);

    for (i = 0; i < sizeof(armlet_options) / sizeof(armlet_options[0]); i++)
    {
        value = get_variable(armlet_options[i].key);
        if (value)
            pow2_armlet_set(armlet_options[i].armlet_key, value);
    }
}

static void update_variables(bool startup)
{
    const char *value;

    if (startup)
    {
       value = get_variable(CORE_NAME "-Framerate");
       if (value)
          EnableFullspeed = !strcmp(value, "Original") ? 0 : 1;

       value = get_variable(CORE_NAME "-virefresh");
       if (value)
          CountPerScanlineOverride = !strcmp(value, "Auto") ? 0 : atoi(value);

       value = get_variable(CORE_NAME "-cpucore");
       if (value)
       {
          if (!strcmp(value, "pure_interpreter"))
             r4300_emumode = EMUMODE_PURE_INTERPRETER;
          else if (!strcmp(value, "cached_interpreter"))
             r4300_emumode = EMUMODE_INTERPRETER;
          else if (!strcmp(value, "dynamic_recompiler"))
             r4300_emumode = EMUMODE_DYNAREC;
       }

       value = get_variable(CORE_NAME "-CountPerOp");
       if (value)
          CountPerOp = atoi(value);

       value = get_variable(CORE_NAME "-CountPerOpDenomPot");
       if (value)
          CountPerOpDenomPot = atoi(value);

       if (EnableFullspeed)
          CountPerOp = 1; // Force CountPerOp == 1

       value = get_variable(CORE_NAME "-ForceDisableExtraMem");
       if (value)
          ForceDisableExtraMem = !strcmp(value, "False") ? 0 : 1;

       value = get_variable(CORE_NAME "-IgnoreTLBExceptions");
       if (value)
       {
          if (!strcmp(value, "False"))
             IgnoreTLBExceptions = 0;
          else if (!strcmp(value, "OnlyNotEnabled"))
             IgnoreTLBExceptions = 1;
          else if (!strcmp(value, "AlwaysIgnoreTLB"))
             IgnoreTLBExceptions = 2;
       }
    }

    update_plugin_variables();
    update_controllers();
}

static void format_saved_memory(void)
{
    format_sram(saved_memory.sram);
    format_eeprom(saved_memory.eeprom, EEPROM_MAX_SIZE);
    format_flashram(saved_memory.flashram);

    for (int i = 0; i < GAME_CONTROLLERS_COUNT; ++i)
    {
      // Generate a random serial ID
      uint32_t serial[6];
      int k;
      for (k = 0; k < 6; ++k)
      {
         serial[k] = xoshiro256pp_next(&l_mpk_idgen);
      }

    format_mempak(saved_memory.mempack + i * MEMPAK_SIZE,
        serial,
        DEFAULT_MEMPAK_DEVICEID,
        DEFAULT_MEMPAK_BANKS,
        DEFAULT_MEMPAK_VERSION);
    }
}

bool retro_load_game(const struct retro_game_info *game)
{
    char* gamePath;
    char* newPath;

    // Workaround for broken subsystem on static platforms
    // Note: game->path can be NULL if loading from a archive
    // Current impl. uses mupen internals so that wouldn't work either way for dd/tpak
    // So we just sanity check
    if(!retro_dd_path_img && game->path)
    {
        gamePath = (char*)game->path;
        newPath = (char*)calloc(1, strlen(gamePath)+5);
        strcpy(newPath, gamePath);
        strcat(newPath, ".ndd");
        FILE* fileTest = fopen(newPath, "r");
        if(!fileTest)
        {
            free(newPath);
        } else {
            fclose(fileTest);
            // Free'd later in Mupen Core
            retro_dd_path_img = newPath;
        }
    }
    
    if (!retro_transferpak_rom_path && game->path)
    {
       gamePath = (char *)game->path;
       newPath = (char *)calloc(1, strlen(gamePath) + 4);
       strcpy(newPath, gamePath);
       strcat(newPath, ".gb");
       FILE *fileTest = fopen(newPath, "r");
       if (!fileTest)
       {
          free(newPath);
       }
       else
       {
          fclose(fileTest);
          // Free'd later in Mupen Core
          retro_transferpak_rom_path = newPath;
 
          // We have a gb rom!
          if (!retro_transferpak_ram_path)
          {
             gamePath = (char *)game->path;
             newPath = (char *)calloc(1, strlen(gamePath) + 5);
             strcpy(newPath, gamePath);
             strcat(newPath, ".sav");
             FILE *fileTest = fopen(newPath, "r");
             if (!fileTest)
             {
                free(newPath);
             }
             else
             {
                fclose(fileTest);
                // Free'd later in Mupen Core
                retro_transferpak_ram_path = newPath;
             }
          }
       }
    }
 
    // Init default vals
    retro_savestate_complete = true;
    load_game_successful = false;

    format_saved_memory();

    update_variables(true);
    video_aspect = foir_widescreen ? 16.0f / 9.0f : 4.0f / 3.0f;

    init_audio_libretro(audio_buffer_size);

    game_data = malloc(game->size);
    memcpy(game_data, game->data, game->size);
    game_size = game->size;

    if (!emu_step_load_data())
        return false;

    emu_step_initialize();

    load_game_successful = true;

    return true;
}

void retro_unload_game(void)
{
    cleanup_global_paths();

    emu_initialized = false;

    // Reset savestate job var
    retro_savestate_complete = false;
}

static void present_frame(void)
{
    const struct pow2_frame *frame = pow2_video_frame();

    if (!frame->pixels)
    {
        video_cb(NULL, video_width, video_height, 0);
        return;
    }

    if (frame->width != video_width || frame->height != video_height || frame->aspect != video_aspect)
    {
        struct retro_game_geometry geometry;

        video_width  = frame->width;
        video_height = frame->height;
        video_aspect = frame->aspect;

        geometry.base_width   = video_width;
        geometry.base_height  = video_height;
        geometry.max_width    = POW2_VIDEO_MAX_WIDTH;
        geometry.max_height   = POW2_VIDEO_MAX_HEIGHT;
        geometry.aspect_ratio = video_aspect;
        environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
    }

    video_cb(frame->pixels, frame->width, frame->height, frame->pitch);
}

void retro_run (void)
{
    static bool updated = false;

    libretro_swap_buffer = false;

    // Between frames, on the emulation thread's OS thread: foir applies its live options here
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
       update_variables(false);

    co_switch(game_thread);

    if (libretro_swap_buffer)
        present_frame();
    else
        video_cb(NULL, video_width, video_height, 0); // dupe: foir held the last frame
}

void retro_reset (void)
{
    CoreDoCommand(M64CMD_RESET, 0, (void*)0);
}

void *retro_get_memory_data(unsigned type)
{
    switch (type)
    {
        case RETRO_MEMORY_SYSTEM_RAM: return g_dev.rdram.dram;
        case RETRO_MEMORY_TRANSFERPAK:
        case RETRO_MEMORY_DD:
        case RETRO_MEMORY_SAVE_RAM:   return &saved_memory;
    }
    return NULL;
}

size_t retro_get_memory_size(unsigned type)
{
    switch (type)
    {
        case RETRO_MEMORY_SYSTEM_RAM:  return RDRAM_MAX_SIZE;
        case RETRO_MEMORY_TRANSFERPAK:
        case RETRO_MEMORY_DD:
        case RETRO_MEMORY_SAVE_RAM:    return sizeof(saved_memory);
    }

    return 0;
}

size_t retro_serialize_size (void)
{
    return 16788288 + 1024 + 4 + 4096;
}

bool retro_serialize(void *data, size_t size)
{
   if (initializing)
      return false;

   retro_savestate_complete = false;
   retro_savestate_result = 0;

   savestates_set_job(savestates_job_save, savestates_type_m64p, data);

   while (!retro_savestate_complete)
   {
      co_switch(game_thread);
   }

   return !!retro_savestate_result;
}

bool retro_unserialize(const void *data, size_t size)
{
   if (initializing)
      return false;

   retro_savestate_complete = false;
   retro_savestate_result = 0;

   savestates_set_job(savestates_job_load, savestates_type_m64p, data);

   while (!retro_savestate_complete)
   {
      co_switch(game_thread);
   }

   return true;
}

//Needed to be able to detach controllers for Lylat Wars multiplayer
//Only sets if controller struct is initialised as addon paks do.
void retro_set_controller_port_device(unsigned in_port, unsigned device) {
    if (in_port < 4){
        switch(device)
        {
            case RETRO_DEVICE_NONE:
               if (controller[in_port].control){
                   controller[in_port].control->Present = 0;
                   break;
               } else {
                   pad_present[in_port] = 0;
                   break;
               }

            case RETRO_DEVICE_JOYPAD:
            default:
               if (controller[in_port].control){
                   controller[in_port].control->Present = 1;
                   break;
               } else {
                   pad_present[in_port] = 1;
                   break;
               }
        }
    }
}

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_cheat_reset(void)
{
    cheat_delete_all(&g_cheat_ctx);
}

void retro_cheat_set(unsigned index, bool enabled, const char* codeLine)
{
    char name[256];
    m64p_cheat_code mupenCode[256];
    int matchLength=0,partCount=0;
    uint32_t codeParts[256];
    int cursor;

    //Generate a name
    sprintf(name, "cheat_%u",index);

    //Break the code into Parts
    for (cursor=0;;cursor++)
    {
        if (ISHEXDEC){
            matchLength++;
        } else {
            if (matchLength){
               char codePartS[matchLength];
               strncpy(codePartS,codeLine+cursor-matchLength,matchLength);
               codePartS[matchLength]=0;
               codeParts[partCount++]=strtoul(codePartS,NULL,16);
               matchLength=0;
            }
        }
        if (!codeLine[cursor]){
            break;
        }
    }

    //Assign the parts to mupenCode
    for (cursor=0;2*cursor+1<partCount;cursor++){
        mupenCode[cursor].address=codeParts[2*cursor];
        mupenCode[cursor].value=codeParts[2*cursor+1];
    }

    //Assign to mupenCode
    cheat_add_new(&g_cheat_ctx, name, mupenCode, partCount / 2);
    cheat_set_enabled(&g_cheat_ctx, name, enabled);
}

void retro_return(void)
{
    co_switch(retro_thread);
}

static int GamesharkActive = 0;

int event_gameshark_active(void)
{
    return GamesharkActive;
}

void event_set_gameshark(int active)
{
    // if boolean value doesn't change then just return
    if (!active == !GamesharkActive)
        return;

    // set the button state
    GamesharkActive = (active ? 1 : 0);

    // notify front-end application that gameshark button state has changed
    StateChanged(M64CORE_INPUT_GAMESHARK, GamesharkActive);
}
