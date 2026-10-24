/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-Next - libretro_core_options.h                            *
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

#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#include <mupen64plus-next_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define HAVE_NO_LANGEXTRA
struct retro_core_option_v2_category option_cats_us[] = {
   {
      "input",
      "Pak/Controller Options",
      "Configure Core Pak/Controller Options."
   },
   {
      "foir",
      "Video (foir)",
      "Configure the foir software RDP."
   },
   {
      "armlet",
      "RSP (arMLEt)",
      "Configure the arMLEt RSP."
   },
   { NULL, NULL, NULL },
};

struct retro_core_option_v2_definition option_defs_us[] = {
    {
        CORE_NAME "-foir-widescreen",
        "Widescreen",
        NULL,
        "Report a 16:9 display aspect ratio instead of 4:3, for widescreen hacks.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-foir-frameskip",
        "Frame Skip",
        NULL,
        "Skip rendering frames to hold full speed. Auto adapts to the load.",
        NULL,
        "foir",
        {
            {"0", "Off"},
            {"1", "Every 2nd"},
            {"2", "2 of 3"},
            {"3", "3 of 4"},
            {"auto", "Auto"},
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-foir-threads",
        "Worker Threads",
        NULL,
        "RDP worker threads. Auto uses the number of cores minus one; 0 renders on the emulation thread. Applied at the next launch.",
        NULL,
        "foir",
        {
            {"auto", "Auto"},
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {"6", NULL},
            {"7", NULL},
            { NULL, NULL },
        },
        "auto"
    },
    {
        CORE_NAME "-foir-interlace-hack",
        "Interlace Hack",
        NULL,
        "Keep the fresh field of field-rendered frames and interpolate the other lines, instead of weaving in stale rows.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-foir-crop-overscan-h",
        "Crop Overscan (Horizontal)",
        NULL,
        "Crop 8 VI pixels off the left and right edges, to hide edge artefacts.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-foir-crop-overscan-v",
        "Crop Overscan (Vertical)",
        NULL,
        "Crop VI lines off the top and bottom edges, to hide edge artefacts.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-foir-vi-antialiasing",
        "VI Anti-Aliasing",
        NULL,
        "Edge anti-aliasing as the VI does it, where the game enables it. Costs emulation-thread time.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-foir-vi-divot",
        "VI Divot Filter",
        NULL,
        "The VI divot filter, where the game enables it. Costs emulation-thread time.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-foir-vi-dedither",
        "VI Dither Restore",
        NULL,
        "The VI dither restore filter, where the game enables it. Costs emulation-thread time.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-foir-vi-gamma",
        "VI Gamma",
        NULL,
        "The VI gamma correction, where the game enables it. Costs emulation-thread time.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-foir-fast-combiner",
        "Fast Combiner",
        NULL,
        "Fast paths for common combiner and blender modes. Applied at the next launch.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-foir-relaxed-precision",
        "Relaxed Precision",
        NULL,
        "Lower accuracy depth values and calculations, for speed. Applied at the next launch.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-foir-fast-perspective",
        "Fast Perspective",
        NULL,
        "Approximate the texture perspective divide, for speed (not exact). Applied at the next launch.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-foir-specialize",
        "Specialised Kernels",
        NULL,
        "Use the precompiled per-state pixel kernels. Applied at the next launch.",
        NULL,
        "foir",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "True"
    },
    {
        CORE_NAME "-armlet-gfx-mode",
        "Graphics Microcode",
        NULL,
        "Native: graphics tasks whose microcode has a native (C/NEON) handler run natively, the rest on the JIT. JIT: every graphics task runs on parallel-rsp's JIT. Applied at the next launch.",
        NULL,
        "armlet",
        {
            {"hle", "Native"},
            {"lle", "JIT"},
            { NULL, NULL },
        },
        "hle"
    },
    {
        CORE_NAME "-armlet-gfx-probe",
        "Probe Unknown Microcode",
        NULL,
        "Derive and validate a native handler for graphics microcode that has no table entry; Loose also accepts close derivations. Applied at the next launch.",
        NULL,
        "armlet",
        {
            {"on", "On"},
            {"off", "Off"},
            {"loose", "Loose"},
            { NULL, NULL },
        },
        "on"
    },
    {
        CORE_NAME "-armlet-rdp-coalesce",
        "RDP List Coalescing",
        NULL,
        "Hand foir the RDP command stream in fewer, larger lists (exact). Applied at the next launch.",
        NULL,
        "armlet",
        {
            {"auto", "Auto"},
            {"on", "On"},
            {"off", "Off"},
            { NULL, NULL },
        },
        "auto"
    },
    {
        CORE_NAME "-cpucore",
        "CPU Core",
        NULL,
        "Select the R4300 CPU Backend, use Interpreter for best compability",
        NULL,
        NULL,
        {
            {"pure_interpreter", "Pure Interpreter"},
            {"cached_interpreter", "Cached Interpreter"},
#ifdef DYNAREC
            {"dynamic_recompiler", "Dynarec"},
#endif
            { NULL, NULL },
        },
#ifdef DYNAREC
        "dynamic_recompiler"
#else
        "cached_interpreter"
#endif
    },
    {
        CORE_NAME "-Framerate",
        "Framerate",
        NULL,
        "Fullspeed will enforce Count per Op 1, this will break some games!",
        NULL,
        NULL,
        {
            {"Original", NULL},
            {"Fullspeed", NULL},
            { NULL, NULL },
        },
        "Original"
    },
    {
        CORE_NAME "-virefresh",
        "VI Refresh (Overclock)",
        NULL,
        "Select a VI Refresh clock, Auto does not impact behaviour, other values override CountPerScanline.",
        NULL,
        NULL,
        {
            {"Auto", NULL},
            {"1500", NULL},
            {"2200", NULL},
            { NULL, NULL },
        },
        "Auto"
    },
    {
        CORE_NAME "-ForceDisableExtraMem",
        "Disable Expansion Pak",
        NULL,
        "Force disable Expansion Pak (might improve performance for some games while reducing emulation accuracy, will break games that require it).",
        NULL,
        NULL,
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-IgnoreTLBExceptions",
        "Ignore emulated TLB Exceptions",
        NULL,
        "(HACK) Ignore emulated TLB Exceptions, this might fix some broken romhacks. This option might be removed in the future.",
        NULL,
        NULL,
        {
            {"False", "Don't Ignore"},
            {"OnlyNotEnabled", "Ignore TLB Exceptions if not using TLB"},
            {"AlwaysIgnoreTLB", "Always Ignore TLB Exceptions"},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-CountPerOp",
        "Count Per Op",
        NULL,
        "Count per Op is used to approximate the Counter reg, 0 will use the embedded Database (or default to 2). Changing this will break stuff!",
        NULL,
        NULL,
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-CountPerOpDenomPot",
        "Count Per Op Divider (Overclock)",
        NULL,
        "Denominator for Count per Op (allowing sub-1 Count per Op in practice). Changing this can break stuff!",
        NULL,
        NULL,
        {
            {"0", NULL},
            {"1", NULL},
            {"2", NULL},
            {"3", NULL},
            {"4", NULL},
            {"5", NULL},
            {"6", NULL},
            {"7", NULL},
            {"8", NULL},
            {"9", NULL},
            {"10", NULL},
            {"11", NULL},
            { NULL, NULL },
        },
        "0"
    },
    {
        CORE_NAME "-astick-deadzone",
        "Analog Deadzone (percent)",
        NULL,
        "Size of the non responsive area around an analog stick.",
        NULL,
        "input",
        {
            {"0", NULL},
            {"5", NULL},
            {"10", NULL},
            {"15", NULL},
            {"20", NULL},
            {"25", NULL},
            {"30", NULL},
            { NULL, NULL },
        },
        "15"
    },
    {
        CORE_NAME "-astick-sensitivity",
        "Analog Sensitivity (percent)",
        NULL,
        "Adjust how far the stick needs to be moved to reach its max value.",
        NULL,
        "input",
        {
            {"50", NULL},
            {"55", NULL},
            {"60", NULL},
            {"65", NULL},
            {"70", NULL},
            {"75", NULL},
            {"80", NULL},
            {"85", NULL},
            {"90", NULL},
            {"95", NULL},
            {"100", NULL},
            {"105", NULL},
            {"110", NULL},
            {"115", NULL},
            {"120", NULL},
            {"125", NULL},
            {"130", NULL},
            {"135", NULL},
            {"140", NULL},
            {"145", NULL},
            {"150", NULL},
            { NULL, NULL },
        },
        "100"
    },
    {
        CORE_NAME "-r-cbutton",
        "Right C Button",
        NULL,
        "Select Right C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C1"
    },
    {
        CORE_NAME "-l-cbutton",
        "Left C Button",
        NULL,
        "Select Left C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C2"
    },
    {
        CORE_NAME "-d-cbutton",
        "Down C Button",
        NULL,
        "Select Down C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C3"
    },
    {
        CORE_NAME "-u-cbutton",
        "Up C Button",
        NULL,
        "Select Up C Button mapping.",
        NULL,
        "input",
        {
            {"C1", NULL},
            {"C2", NULL},
            {"C3", NULL},
            {"C4", NULL},
            { NULL, NULL },
        },
        "C4"
    },
    {
        CORE_NAME "-alt-map",
        "Independent C-button Controls",
        NULL,
        "Use an alternate control scheme, useful for some 3rdparty controllers.",
        NULL,
        "input",
        {
            {"False", NULL},
            {"True", NULL},
            { NULL, NULL },
        },
        "False"
    },
    {
        CORE_NAME "-pak1",
        "Player 1 Pak",
        NULL,
        "Select Player 1 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "memory"
    },
    {
        CORE_NAME "-pak2",
        "Player 2 Pak",
        NULL,
        "Select Player 2 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "none"
    },
    {
        CORE_NAME "-pak3",
        "Player 3 Pak",
        NULL,
        "Select Player 3 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "none"
    },
    {
        CORE_NAME "-pak4",
        "Player 4 Pak",
        NULL,
        "Select Player 4 Controller Pak.",
        NULL,
        "input",
        {
            {"none", NULL},
            {"memory", NULL},
            {"rumble", NULL},
            {"transfer", NULL},
            { NULL, NULL },
        },
        "none"
    },
    { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};

struct retro_core_options_v2 options_us = {
   option_cats_us,
   option_defs_us
};

struct retro_core_options_v2 *option_defs_intl[RETRO_LANGUAGE_LAST] = {
   &options_us, /* RETRO_LANGUAGE_ENGLISH */
   NULL,            /* RETRO_LANGUAGE_JAPANESE */
   NULL,            /* RETRO_LANGUAGE_FRENCH */
   NULL,            /* RETRO_LANGUAGE_SPANISH */
   NULL,            /* RETRO_LANGUAGE_GERMAN */
   NULL,            /* RETRO_LANGUAGE_ITALIAN */
   NULL,            /* RETRO_LANGUAGE_DUTCH */
   NULL,            /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
   NULL,            /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
   NULL,            /* RETRO_LANGUAGE_RUSSIAN */
   NULL,            /* RETRO_LANGUAGE_KOREAN */
   NULL,            /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
   NULL,            /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
   NULL,            /* RETRO_LANGUAGE_ESPERANTO */
   NULL,            /* RETRO_LANGUAGE_POLISH */
   NULL,            /* RETRO_LANGUAGE_VIETNAMESE */
   NULL,            /* RETRO_LANGUAGE_ARABIC */
   NULL,            /* RETRO_LANGUAGE_GREEK */
   NULL,            /* RETRO_LANGUAGE_TURKISH */
   NULL,            /* RETRO_LANGUAGE_SLOVAK */
   NULL,            /* RETRO_LANGUAGE_PERSIAN */
   NULL,            /* RETRO_LANGUAGE_HEBREW */
   NULL,            /* RETRO_LANGUAGE_ASTURIAN */
   NULL,            /* RETRO_LANGUAGE_FINNISH */
};


static INLINE void libretro_set_core_options(retro_environment_t environ_cb,
      bool *categories_supported)
{
   unsigned version  = 0;
#ifndef HAVE_NO_LANGEXTRA
   unsigned language = 0;
#endif

   if (!environ_cb || !categories_supported)
      return;

   *categories_supported = false;

   if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
      version = 0;

   if (version >= 2)
   {
#ifndef HAVE_NO_LANGEXTRA
      struct retro_core_options_v2_intl core_options_intl;

      core_options_intl.us    = &options_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = options_intl[language];

      *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL,
            &core_options_intl);
#else
      *categories_supported = environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
            &options_us);
#endif
   }
   else
   {
      size_t i, j;
      size_t option_index              = 0;
      size_t num_options               = 0;
      struct retro_core_option_definition
            *option_v1_defs_us         = NULL;
#ifndef HAVE_NO_LANGEXTRA
      size_t num_options_intl          = 0;
      struct retro_core_option_v2_definition
            *option_defs_intl          = NULL;
      struct retro_core_option_definition
            *option_v1_defs_intl       = NULL;
      struct retro_core_options_intl
            core_options_v1_intl;
#endif
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine total number of options */
      while (true)
      {
         if (option_defs_us[num_options].key)
            num_options++;
         else
            break;
      }

      if (version >= 1)
      {
         /* Allocate US array */
         option_v1_defs_us = (struct retro_core_option_definition *)
               calloc(num_options + 1, sizeof(struct retro_core_option_definition));

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
            struct retro_core_option_value *option_values         = option_def_us->values;
            struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
            struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;

            option_v1_def_us->key           = option_def_us->key;
            option_v1_def_us->desc          = option_def_us->desc;
            option_v1_def_us->info          = option_def_us->info;
            option_v1_def_us->default_value = option_def_us->default_value;

            /* Values must be copied individually... */
            while (option_values->value)
            {
               option_v1_values->value = option_values->value;
               option_v1_values->label = option_values->label;

               option_values++;
               option_v1_values++;
            }
         }

#ifndef HAVE_NO_LANGEXTRA
         if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
             (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH) &&
             options_intl[language])
            option_defs_intl = options_intl[language]->definitions;

         if (option_defs_intl)
         {
            /* Determine number of intl options */
            while (true)
            {
               if (option_defs_intl[num_options_intl].key)
                  num_options_intl++;
               else
                  break;
            }

            /* Allocate intl array */
            option_v1_defs_intl = (struct retro_core_option_definition *)
                  calloc(num_options_intl + 1, sizeof(struct retro_core_option_definition));

            /* Copy parameters from option_defs_intl array */
            for (i = 0; i < num_options_intl; i++)
            {
               struct retro_core_option_v2_definition *option_def_intl = &option_defs_intl[i];
               struct retro_core_option_value *option_values           = option_def_intl->values;
               struct retro_core_option_definition *option_v1_def_intl = &option_v1_defs_intl[i];
               struct retro_core_option_value *option_v1_values        = option_v1_def_intl->values;

               option_v1_def_intl->key           = option_def_intl->key;
               option_v1_def_intl->desc          = option_def_intl->desc;
               option_v1_def_intl->info          = option_def_intl->info;
               option_v1_def_intl->default_value = option_def_intl->default_value;

               /* Values must be copied individually... */
               while (option_values->value)
               {
                  option_v1_values->value = option_values->value;
                  option_v1_values->label = option_values->label;

                  option_values++;
                  option_v1_values++;
               }
            }
         }

         core_options_v1_intl.us    = option_v1_defs_us;
         core_options_v1_intl.local = option_v1_defs_intl;

         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_v1_intl);
#else
         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
#endif
      }
      else
      {
         /* Allocate arrays */
         variables  = (struct retro_variable *)calloc(num_options + 1,
               sizeof(struct retro_variable));
         values_buf = (char **)calloc(num_options, sizeof(char *));

         if (!variables || !values_buf)
            goto error;

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            const char *key                        = option_defs_us[i].key;
            const char *desc                       = option_defs_us[i].desc;
            const char *default_value              = option_defs_us[i].default_value;
            struct retro_core_option_value *values = option_defs_us[i].values;
            size_t buf_len                         = 3;
            size_t default_index                   = 0;

            values_buf[i] = NULL;

            if (desc)
            {
               size_t num_values = 0;

               /* Determine number of values */
               while (true)
               {
                  if (values[num_values].value)
                  {
                     /* Check if this is the default value */
                     if (default_value)
                        if (strcmp(values[num_values].value, default_value) == 0)
                           default_index = num_values;

                     buf_len += strlen(values[num_values].value);
                     num_values++;
                  }
                  else
                     break;
               }

               /* Build values string */
               if (num_values > 0)
               {
                  buf_len += num_values - 1;
                  buf_len += strlen(desc);

                  values_buf[i] = (char *)calloc(buf_len, sizeof(char));
                  if (!values_buf[i])
                     goto error;

                  strcpy(values_buf[i], desc);
                  strcat(values_buf[i], "; ");

                  /* Default value goes first */
                  strcat(values_buf[i], values[default_index].value);

                  /* Add remaining values */
                  for (j = 0; j < num_values; j++)
                  {
                     if (j != default_index)
                     {
                        strcat(values_buf[i], "|");
                        strcat(values_buf[i], values[j].value);
                     }
                  }
               }
            }

            variables[option_index].key   = key;
            variables[option_index].value = values_buf[i];
            option_index++;
         }

         /* Set variables */
         environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
      }

error:
      /* Clean up */

      if (option_v1_defs_us)
      {
         free(option_v1_defs_us);
         option_v1_defs_us = NULL;
      }

#ifndef HAVE_NO_LANGEXTRA
      if (option_v1_defs_intl)
      {
         free(option_v1_defs_intl);
         option_v1_defs_intl = NULL;
      }
#endif

      if (values_buf)
      {
         for (i = 0; i < num_options; i++)
         {
            if (values_buf[i])
            {
               free(values_buf[i]);
               values_buf[i] = NULL;
            }
         }

         free(values_buf);
         values_buf = NULL;
      }

      if (variables)
      {
         free(variables);
         variables = NULL;
      }
   }
}

#ifdef __cplusplus
}
#endif

#endif
