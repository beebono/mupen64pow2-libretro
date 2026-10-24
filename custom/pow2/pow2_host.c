/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - pow2_host.c                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Startup, core API and options of the pow2 plugins (see pow2_host.h) */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "libretro_private.h"
#include "mupen64plus-next_common.h"
#include "api/m64p_common.h"
#include "api/m64p_config.h"
#include "api/m64p_types.h"
#include "main/version.h"
#include "../../mupen64plus-rdp-foir/src/foir_suxty.h"
#include "pow2_host.h"

/* config.c: the in-memory config is off until this is called */
m64p_error ConfigInit(const char *ConfigDirOverride, const char *DataDirOverride);

/* the plugins' API, under the prefixes of rename-foir.h and rename-armlet.h */
typedef void (*pow2_debug_cb)(void *, int, const char *);
m64p_error foir_PluginStartup(m64p_dynlib_handle core, void *context, pow2_debug_cb debug_cb);
m64p_error foir_PluginShutdown(void);
m64p_error armlet_PluginStartup(m64p_dynlib_handle core, void *context, pow2_debug_cb debug_cb);
m64p_error armlet_PluginShutdown(void);

#define ARMLET_SECTION "Rsp-Armlet"

static int l_core_token;
#define POW2_CORE_HANDLE ((void *)&l_core_token)

static char l_user_dir[2048];
static bool l_started;

/* ConfigSaveSection would write mupen64plus.cfg into the frontend's working
 * directory: the settings come from the core options instead */
static m64p_error CALL pow2_ConfigSaveSection(const char *SectionName)
{
    (void)SectionName;
    return M64ERR_SUCCESS;
}

static const char * CALL pow2_ConfigGetUserConfigPath(void)
{
    return l_user_dir;
}

static const struct
{
    const char *name;
    void *fn;
} l_core_api[] = {
    { "CoreGetAPIVersions",      (void *)CoreGetAPIVersions },
    { "PluginGetVersion",        (void *)PluginGetVersion },
    { "ConfigOpenSection",       (void *)ConfigOpenSection },
    { "ConfigDeleteSection",     (void *)ConfigDeleteSection },
    { "ConfigSaveSection",       (void *)pow2_ConfigSaveSection },
    { "ConfigSetParameter",      (void *)ConfigSetParameter },
    { "ConfigGetParameter",      (void *)ConfigGetParameter },
    { "ConfigSetDefaultInt",     (void *)ConfigSetDefaultInt },
    { "ConfigSetDefaultFloat",   (void *)ConfigSetDefaultFloat },
    { "ConfigSetDefaultBool",    (void *)ConfigSetDefaultBool },
    { "ConfigSetDefaultString",  (void *)ConfigSetDefaultString },
    { "ConfigGetParamInt",       (void *)ConfigGetParamInt },
    { "ConfigGetParamFloat",     (void *)ConfigGetParamFloat },
    { "ConfigGetParamBool",      (void *)ConfigGetParamBool },
    { "ConfigGetParamString",    (void *)ConfigGetParamString },
    { "ConfigGetUserConfigPath", (void *)pow2_ConfigGetUserConfigPath },
};

/* The plugins are compiled with -Ddlsym=pow2_dlsym */
void *pow2_dlsym(void *handle, const char *name)
{
    size_t i;

    if (handle != POW2_CORE_HANDLE)
        return dlsym(handle, name);

    for (i = 0; i < sizeof(l_core_api) / sizeof(l_core_api[0]); i++)
        if (!strcmp(l_core_api[i].name, name))
            return l_core_api[i].fn;

    if (log_cb)
        log_cb(RETRO_LOG_WARN, CORE_NAME ": a plugin looked up %s, which the core does not provide\n", name);
    return NULL;
}

static void plugin_log(void *context, int level, const char *message)
{
    enum retro_log_level retro_level;

    if (!log_cb)
        return;

    switch (level)
    {
        case M64MSG_ERROR:   retro_level = RETRO_LOG_ERROR; break;
        case M64MSG_WARNING: retro_level = RETRO_LOG_WARN;  break;
        /* foir reports frame statistics at INFO every 120 frames */
        default:             retro_level = RETRO_LOG_DEBUG; break;
    }
    log_cb(retro_level, CORE_NAME ": [%s] %s\n", (const char *)context, message);
}

bool pow2_plugins_startup(const char *user_dir)
{
    m64p_error err;
    size_t len;

    if (l_started)
        return true;

    snprintf(l_user_dir, sizeof(l_user_dir), "%s", user_dir);
    len = strlen(l_user_dir);
    if (len && l_user_dir[len - 1] != '/' && l_user_dir[len - 1] != '\\' && len + 1 < sizeof(l_user_dir))
        strcat(l_user_dir, "/");

    ConfigInit(NULL, NULL);

    err = foir_PluginStartup(POW2_CORE_HANDLE, "foir", plugin_log);
    if (err != M64ERR_SUCCESS)
    {
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, CORE_NAME ": foir failed to start (err=%i)\n", err);
        return false;
    }

    err = armlet_PluginStartup(POW2_CORE_HANDLE, "arMLEt", plugin_log);
    if (err != M64ERR_SUCCESS)
    {
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, CORE_NAME ": arMLEt failed to start (err=%i)\n", err);
        foir_PluginShutdown();
        return false;
    }

    l_started = true;
    return true;
}

void pow2_plugins_shutdown(void)
{
    if (!l_started)
        return;

    armlet_PluginShutdown();
    foir_PluginShutdown();
    l_started = false;
}

bool pow2_foir_set(const char *key, int value)
{
    const struct foir_suxty_interface *foir = FoirSuxtyGetInterface();
    unsigned i;

    if (!l_started || !foir || foir->abi != FOIR_SUXTY_ABI)
        return false;

    for (i = 0; i < foir->option_count; i++)
    {
        if (strcmp(foir->options[i].key, key))
            continue;
        if (foir->set(i, value) >= 0)
            return true;
        if (log_cb)
            log_cb(RETRO_LOG_WARN, CORE_NAME ": foir refused %s = %i\n", key, value);
        return false;
    }

    if (log_cb)
        log_cb(RETRO_LOG_WARN, CORE_NAME ": foir has no option %s\n", key);
    return false;
}

bool pow2_armlet_set(const char *key, const char *value)
{
    m64p_handle section;

    if (!l_started || ConfigOpenSection(ARMLET_SECTION, &section) != M64ERR_SUCCESS)
        return false;

    return ConfigSetParameter(section, key, M64TYPE_STRING, value) == M64ERR_SUCCESS;
}
