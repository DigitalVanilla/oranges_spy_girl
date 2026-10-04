################################################################################
# GCC-only Amiga build
################################################################################

GCC_ROOT ?= D:/SDK/amiga-gcc
GCC_TARGET = $(GCC_ROOT)/m68k-amigaos

CC = "$(GCC_ROOT)/bin/m68k-amigaos-gcc.exe"
CXX = "$(GCC_ROOT)/bin/m68k-amigaos-g++.exe"
AR = "$(GCC_ROOT)/bin/m68k-amigaos-ar.exe"
AS = "$(GCC_ROOT)/bin/vasmm68k_mot.exe"
STRIP = "$(GCC_ROOT)/bin/m68k-amigaos-strip.exe"
MKDIR_P = mkdir -p

export PATH := $(GCC_ROOT)/bin;$(PATH)

################################################################################
# Feature switches
################################################################################

# The demo currently uses MaggieLibrary directly through maggie.library.
# SAGE 3D and the older Maggie3D static renderer are not linked unless enabled.
SAGE_ENABLE_3D ?= 0

################################################################################
# Directories
################################################################################

SRC_DIR = src
ASSETS_DIR = assets
SAGE_DIR = include/SAGE
SAGE_SRC_DIR = $(SAGE_DIR)/src
MAGGIE3D_DIR = include/Maggie3D/static
MAGGIE3D_SRC_DIR = $(MAGGIE3D_DIR)/src
MAGGIELIB_DIR = include/MaggieLibrary
MAGGIELIB_INC_DIR = include/MaggieLibrary/include
OBJ_DIR = build/obj
MAGGIELIB_OBJ_DIR = build/maggie_library
MAGGIELIB_BUILD_LIBRARY = $(MAGGIELIB_OBJ_DIR)/maggie.library
MAGGIELIB_BUILD_STATIC = $(MAGGIELIB_OBJ_DIR)/libmaggie.a
DESKTOP_DIR ?= $(subst \,/,$(USERPROFILE))/Desktop
PACKAGE_ROOT ?= $(DESKTOP_DIR)
PACKAGE_NAME ?= spy_girl
PACKAGE_DIR ?= $(PACKAGE_ROOT)/$(PACKAGE_NAME)
BIN_DIR ?= build/bin

TARGET ?= $(BIN_DIR)/$(PACKAGE_NAME)

################################################################################
# Sources
################################################################################

APP_CSRCS = \
	$(SRC_DIR)/sage_optional_modules.c \
	$(SRC_DIR)/spy_anim.c \
	$(SRC_DIR)/spy_audio.c \
	$(SRC_DIR)/spy_camera_timeline.c \
	$(SRC_DIR)/spy_debug.c \
	$(SRC_DIR)/spy_input.c \
	$(SRC_DIR)/spy_log.c \
	$(SRC_DIR)/spy_logo.c \
	$(SRC_DIR)/spy_maggie.c \
	$(SRC_DIR)/spy_maggie_stream.c \
	$(SRC_DIR)/spy_preload.c \
	$(SRC_DIR)/spy_text.c \
	$(SRC_DIR)/spy_tunnel.c \
	$(SRC_DIR)/spy_tunnel_renderer.c \
	$(SRC_DIR)/spy_tunnel_effects.c \
	$(SRC_DIR)/spy_tunnel_pattern_effects.c \
	$(SRC_DIR)/spy_tunnel_reveal.c \
	$(SRC_DIR)/spy_typewriter.c \
	$(SRC_DIR)/spy_girl.c

APP_ASMSRCS = \
	$(SRC_DIR)/spy_tunnel_060.asm \
	$(SRC_DIR)/spy_tunnel_2x_060.asm

SAGE_CSRCS = $(addprefix $(SAGE_SRC_DIR)/, \
	sage.c \
	sage_logger.c \
	sage_error.c \
	sage_memory.c \
	sage_thread.c \
	sage_maths.c \
	sage_configfile.c \
	sage_vampire.c \
	sage_video.c \
	sage_bitmap.c \
	sage_picture.c \
	sage_event.c \
	sage_screen.c \
	sage_draw.c \
	sage_layer.c \
	sage_sprite.c \
	sage_tile.c \
	sage_tilemap.c \
	sage_input.c \
	sage_keyboard.c \
	sage_joyport.c \
	sage_audio.c \
	sage_loadwave.c \
	sage_load8svx.c \
	sage_sound.c \
	sage_loadtracker.c \
	sage_loadaiff.c \
	sage_music.c \
	sage_timer.c \
	sage_interrupt.c)

SAGE_3D_CSRCS = $(addprefix $(SAGE_SRC_DIR)/, \
	sage_3d.c \
	sage_3dtexture.c \
	sage_3drender.c \
	sage_3dtexmap.c \
	sage_3dengine.c \
	sage_3dentity.c \
	sage_3dcamera.c \
	sage_3dmaterial.c \
	sage_3dskybox.c \
	sage_3dterrain.c \
	sage_loadlwo.c \
	sage_loadobj.c)

SAGE_ASMSRCS = $(addprefix $(SAGE_SRC_DIR)/, \
	sage_blitter.asm \
	sage_ammxblit.asm \
	sage_fastdraw.asm \
	sage_vblint.asm \
	sage_itserver.asm)

SAGE_3D_ASMSRCS = $(addprefix $(SAGE_SRC_DIR)/, \
	sage_3dfastmap.asm)

SAGE_PREBUILT_OBJS = \
	$(SAGE_SRC_DIR)/ext/PT-AHIPlay.o

MAGGIE3D_CSRCS = $(addprefix $(MAGGIE3D_SRC_DIR)/, \
	maggie.c \
	memory.c \
	texture.c \
	zbuffer.c \
	draw.c \
	flattmap.c \
	gouraudtmap.c \
	flatshade.c \
	gouraudshade.c \
	convert.c \
	loader.c)

MAGGIE3D_ASMSRCS = \
	$(MAGGIE3D_SRC_DIR)/fast.asm

ifeq ($(SAGE_ENABLE_3D),1)
SAGE_CSRCS += $(SAGE_3D_CSRCS)
SAGE_ASMSRCS += $(SAGE_3D_ASMSRCS)
else
# Maggie3D is only needed by SAGE's optional 3D module, not by the current
# direct MaggieLibrary path in src/spy_maggie.c.
MAGGIE3D_CSRCS =
MAGGIE3D_ASMSRCS =
endif

APP_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(APP_CSRCS))
APP_ASMOBJS = $(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.o,$(APP_ASMSRCS))
SAGE_COBJS = $(patsubst $(SAGE_SRC_DIR)/%.c,$(OBJ_DIR)/sage_%.o,$(SAGE_CSRCS))
SAGE_ASMOBJS = $(patsubst $(SAGE_SRC_DIR)/%.asm,$(OBJ_DIR)/sage_%.o,$(SAGE_ASMSRCS))
MAGGIE3D_COBJS = $(patsubst $(MAGGIE3D_SRC_DIR)/%.c,$(OBJ_DIR)/maggie3d_%.o,$(MAGGIE3D_CSRCS))
MAGGIE3D_ASMOBJS = $(patsubst $(MAGGIE3D_SRC_DIR)/%.asm,$(OBJ_DIR)/maggie3d_%.o,$(MAGGIE3D_ASMSRCS))

OBJS = $(APP_OBJS) $(APP_ASMOBJS) $(SAGE_COBJS) $(SAGE_ASMOBJS) $(MAGGIE3D_COBJS) $(MAGGIE3D_ASMOBJS) $(SAGE_PREBUILT_OBJS)

################################################################################
# Options
################################################################################

SAGE_DEBUG ?= 0
SAGE_SAFE ?= 0

CPUFLAGS = -m68060 -m68881
MAGGIELIB_CFLAGS = -std=c11 -fexcess-precision=fast -noixemul -Ofast -fno-unsafe-math-optimizations -fomit-frame-pointer -m68060 -m68881 -mregparm -I include -Wdouble-promotion -MMD -MP
MAGGIELIB_ASFLAGS = -m68080 -m68882 -quiet -Fhunk -I $(GCC_TARGET)/ndk-include

CFLAGS = $(CPUFLAGS) \
	-O2 \
	-DPI=3.14159265358979323846 \
	-D_SAGE_DEBUG_MODE_=$(SAGE_DEBUG) \
	-D_SAGE_SAFE_MODE_=$(SAGE_SAFE) \
	-I$(SAGE_DIR)/include \
	-I$(MAGGIELIB_INC_DIR) \
	-I$(GCC_TARGET)/ndk-include \
	-idirafter include/LibInclude

# GCC 6.5.0b crashes internally on sage_screen.c at -O2.
SCREEN_CFLAGS = $(CPUFLAGS) \
	-O0 \
	-DPI=3.14159265358979323846 \
	-D_SAGE_DEBUG_MODE_=$(SAGE_DEBUG) \
	-D_SAGE_SAFE_MODE_=$(SAGE_SAFE) \
	-I$(SAGE_DIR)/include \
	-I$(MAGGIELIB_INC_DIR) \
	-I$(GCC_TARGET)/ndk-include \
	-idirafter include/LibInclude

MAGGIE3D_CFLAGS = $(CPUFLAGS) \
	-O2 \
	-DPI=3.14159265358979323846 \
	-D_USE_MAGGIE_=1 \
	-D_USE_FASTASM_=1 \
	-D_ACTIVATE_DEBUG_=0 \
	-I$(MAGGIE3D_SRC_DIR) \
	-I$(GCC_TARGET)/ndk-include \
	-idirafter include/LibInclude

ASFLAGS = -Fhunk -m68060
AMMX_ASFLAGS = -Fhunk -m68080

LDFLAGS = $(CPUFLAGS) -noixemul
LDLIBS = -lm -lamiga

################################################################################

.PHONY: all clean rebuild package maggie-library

all: package

$(OBJ_DIR) $(BIN_DIR) $(MAGGIELIB_OBJ_DIR) $(PACKAGE_ROOT):
	$(MKDIR_P) $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/spy_girl.o: $(SRC_DIR)/spy_config.h $(SRC_DIR)/spy_camera_timeline.h

$(OBJ_DIR)/spy_tunnel.o: $(SRC_DIR)/spy_config.h $(SRC_DIR)/spy_tunnel_renderer.h

$(OBJ_DIR)/spy_tunnel_renderer.o: $(SRC_DIR)/spy_config.h $(SRC_DIR)/spy_tunnel_renderer.h

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/sage_sage_screen.o: $(SAGE_SRC_DIR)/sage_screen.c | $(OBJ_DIR)
	$(CC) $(SCREEN_CFLAGS) -c $< -o $@

$(OBJ_DIR)/sage_%.o: $(SAGE_SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/sage_sage_ammxblit.o: $(SAGE_SRC_DIR)/sage_ammxblit.asm | $(OBJ_DIR)
	$(AS) $(AMMX_ASFLAGS) $< -o $@

$(OBJ_DIR)/sage_%.o: $(SAGE_SRC_DIR)/%.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/maggie3d_%.o: $(MAGGIE3D_SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(MAGGIE3D_CFLAGS) -c $< -o $@

$(OBJ_DIR)/maggie3d_%.o: $(MAGGIE3D_SRC_DIR)/%.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# SAGE ships this Devpac-style object prebuilt. Rebuilding the .s file would
# require legacy ahi/*.i assembler includes that are not part of this GCC SDK.
$(SAGE_SRC_DIR)/ext/PT-AHIPlay.o:
	@:

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

maggie-library: | $(MAGGIELIB_OBJ_DIR)
	$(MAKE) -C $(MAGGIELIB_DIR) \
		TARGET=../../$(MAGGIELIB_BUILD_LIBRARY) \
		STATIC_TARGET=../../$(MAGGIELIB_BUILD_STATIC) \
		BUILD_DIR=../../$(MAGGIELIB_OBJ_DIR) \
		GCC_ROOT=$(GCC_ROOT) \
		GCC_TARGET=$(GCC_TARGET) \
		CC=$(CC) \
		CXX=$(CXX) \
		AR=$(AR) \
		AS=$(AS) \
		STRIP=$(STRIP) \
		MKDIR_P="$(MKDIR_P)" \
		CFLAGS="$(MAGGIELIB_CFLAGS)" \
		ASFLAGS="$(MAGGIELIB_ASFLAGS)"
	$(STRIP) $(MAGGIELIB_BUILD_LIBRARY)

package: $(TARGET) maggie-library | $(PACKAGE_ROOT)
	rm -rf $(PACKAGE_DIR)
	$(MKDIR_P) $(PACKAGE_DIR)
	cp -f $(TARGET) $(PACKAGE_DIR)/
	cp -f $(MAGGIELIB_BUILD_LIBRARY) $(PACKAGE_DIR)/
	cp -f file_id.diz $(PACKAGE_DIR)/
	cp -f READ.ME $(PACKAGE_DIR)/read.me
	cp -Rf $(ASSETS_DIR) $(PACKAGE_DIR)/

clean:
	rm -f $(OBJ_DIR)/* $(TARGET)

rebuild: clean
	$(MAKE) all
