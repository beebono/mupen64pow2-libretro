LOCAL_PATH := $(call my-dir)

ROOT_DIR     := $(LOCAL_PATH)/../..
LIBRETRO_DIR := $(ROOT_DIR)/libretro
INCFLAGS     :=

# Reset flags that the common makefile doesn't properly handle
platform     := android
COREFLAGS    :=
COREASMFLAGS :=
CORELDLIBS   :=
DYNAFLAGS    :=
HAVE_NEON    :=
SOURCES_C    :=
SOURCES_CXX  :=
SOURCES_ASM  :=
SOURCES_NASM :=
ANDROID      := 1
AWK          ?= awk
STRINGS      ?= $(NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strings
TR           ?= tr
LOCAL_SHORT_COMMANDS := true
WITH_DYNAREC :=

# foir and arMLEt support aarch64 and x86_64 only (see Application.mk)
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
  WITH_DYNAREC := aarch64
else ifeq ($(TARGET_ARCH_ABI),x86_64)
  WITH_DYNAREC := x86_64
endif

include $(ROOT_DIR)/Makefile.common

AWK_DEST_DIR    := $(NDK_APP_OUT)/local/$(TARGET_ARCH_ABI)/asm_defines
ASM_DEFINES_NASM := $(AWK_DEST_DIR)/asm_defines_nasm.h
ASM_DEFINES_GAS  := $(AWK_DEST_DIR)/asm_defines_gas.h

COREFLAGS += -D__LIBRETRO__ -DOS_ANDROID -DUSE_FILE32API -DM64P_PLUGIN_API -DM64P_CORE_PROTOTYPES -D_ENDUSER_RELEASE -DSINC_LOWER_QUALITY -DMUPENPLUSAPI $(INCFLAGS) $(DYNAFLAGS) -DANDROID -DHAVE_POSIX_MEMALIGN=1

GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
  COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

# The pow2 plugins are built as their own modules: ndk-build has no per-file
# flags, and they need the flags of their own builds (Makefile.common)
POW2_PLUGIN_SOURCES_C := $(FOIR_SOURCES_C) $(ARMLET_SOURCES_C) $(ARMLET_HLE_SOURCES_C) $(ARMLET_LIGHTNING_SOURCES_C)
POW2_ANDROID_CFLAGS   := -O3 -DNDEBUG -fvisibility=hidden

include $(CLEAR_VARS)
LOCAL_MODULE     := pow2_foir
LOCAL_SRC_FILES  := $(FOIR_SOURCES_C)
LOCAL_CFLAGS     := $(POW2_ANDROID_CFLAGS) $(FOIR_CFLAGS)
LOCAL_ARM_MODE   := arm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE     := pow2_armlet
LOCAL_SRC_FILES  := $(ARMLET_SOURCES_C)
LOCAL_CFLAGS     := $(POW2_ANDROID_CFLAGS) $(ARMLET_CFLAGS)
LOCAL_ARM_MODE   := arm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE     := pow2_armlet_hle
LOCAL_SRC_FILES  := $(ARMLET_HLE_SOURCES_C)
LOCAL_CFLAGS     := $(POW2_ANDROID_CFLAGS) $(ARMLET_CFLAGS) -ffast-math
LOCAL_ARM_MODE   := arm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE     := pow2_lightning
LOCAL_SRC_FILES  := $(ARMLET_LIGHTNING_SOURCES_C)
LOCAL_CFLAGS     := $(POW2_ANDROID_CFLAGS) -std=gnu11 -DHAVE_MMAP=1 $(ARMLET_LIGHTNING_INC)
LOCAL_ARM_MODE   := arm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE       := pow2_armlet_cxx
LOCAL_SRC_FILES    := $(filter-out %/vu_inline.cpp,$(ARMLET_SOURCES_CXX))
LOCAL_CPPFLAGS     := $(POW2_ANDROID_CFLAGS) $(ARMLET_CXXFLAGS)
LOCAL_CPP_FEATURES := exceptions
LOCAL_ARM_MODE     := arm
include $(BUILD_STATIC_LIBRARY)

# vu_inline.cpp holds the VU op bodies the aarch64 JIT copies into its code:
# they must leave every register the JIT keeps live alone, and stay out of LTO
include $(CLEAR_VARS)
LOCAL_MODULE       := pow2_armlet_vu
LOCAL_SRC_FILES    := $(filter %/vu_inline.cpp,$(ARMLET_SOURCES_CXX))
LOCAL_CPPFLAGS     := $(POW2_ANDROID_CFLAGS) $(ARMLET_CXXFLAGS)
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
  LOCAL_CPPFLAGS   += -fno-lto -ffixed-x9 -ffixed-x10 -ffixed-x11 -ffixed-x12 -ffixed-x13 -ffixed-x14 -ffixed-x15 \
                      -ffixed-x18 -ffixed-x20 -ffixed-x21 -ffixed-x22 -ffixed-x23 -ffixed-x24 -ffixed-x25 -ffixed-x26 \
                      -ffixed-x27 -ffixed-x28
endif
LOCAL_CPP_FEATURES := exceptions
LOCAL_ARM_MODE     := arm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := retro
LOCAL_SRC_FILES        := $(filter-out $(ARMLET_SOURCES_CXX),$(SOURCES_CXX)) $(filter-out $(POW2_PLUGIN_SOURCES_C),$(SOURCES_C)) $(SOURCES_ASM) $(SOURCES_NASM)
LOCAL_C_INCLUDES       := $(AWK_DEST_DIR)
LOCAL_ASMFLAGS         := $(COREASMFLAGS) -I$(AWK_DEST_DIR)/
LOCAL_CPPFLAGS         := -std=gnu++11 $(CXXFLAGS) $(COREFLAGS)
LOCAL_CFLAGS           := $(CFLAGS) $(COREFLAGS)
LOCAL_LDFLAGS          := -Wl,-version-script=$(LIBRETRO_DIR)/link.T
LOCAL_LDLIBS           := -llog -ldl -lm $(CORELDLIBS)
LOCAL_WHOLE_STATIC_LIBRARIES := pow2_foir pow2_armlet pow2_armlet_hle pow2_lightning pow2_armlet_cxx pow2_armlet_vu
LOCAL_CPP_FEATURES     := exceptions
LOCAL_ARM_MODE         := arm
LOCAL_ARM_NEON         := true
LOCAL_CONLYFLAGS       := -std=gnu11

include $(BUILD_SHARED_LIBRARY)

# Use the core's NDK-compiled object so offsets match its ABI and flags.
ASM_DEFINES_OBJ := $(LOCAL_OBJS_DIR)/$(call get-object-name,$(CORE_DIR)/src/asm_defines/asm_defines.c)

$(ASM_DEFINES_NASM): $(ASM_DEFINES_OBJ) $(CORE_DIR)/tools/gen_asm_defines.awk
	@mkdir -p $(dir $@)
	$(STRINGS) "$<" | $(TR) -d '\r' | $(AWK) \
		-v dest_dir="$(dir $@)" \
		-f $(CORE_DIR)/tools/gen_asm_defines.awk

$(ASM_DEFINES_GAS): $(ASM_DEFINES_NASM)

$(foreach src,$(SOURCES_ASM),$(eval $(LOCAL_OBJS_DIR)/$(call get-object-name,$(src)) : $(ASM_DEFINES_GAS)))
$(foreach src,$(SOURCES_NASM),$(eval $(LOCAL_OBJS_DIR)/$(call get-object-name,$(src)) : $(ASM_DEFINES_NASM)))
