# TARGET #


TARGET := 3DS
LIBRARY := 0

ifeq ($(TARGET),$(filter $(TARGET),3DS WIIU))
    ifeq ($(strip $(DEVKITPRO)),)
        $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
    endif
endif

# COMMON CONFIGURATION #


LAUNCHER_M = 1

ifeq ($(LAUNCHER_M), 1)
	NAME := MK7DX
else 
	NAME := MK7DX_Installer
endif

BUILD_DIR := build
OUTPUT_DIR := output
INCLUDE_DIRS := $(SOURCE_DIRS) include
SOURCE_DIRS := source source/json source/allocator

EXTRA_OUTPUT_FILES :=

LIBRARY_DIRS := $(PORTLIBS) $(CTRULIB) $(DEVKITPRO)/libcwav $(DEVKITPRO)/libncsnd
LIBRARIES := citro3d ctru png z m curl mbedtls mbedx509 mbedcrypto cwav ncsnd

ifeq ($(LAUNCHER_M), 1)
	VERSION_MAJOR := 1
	VERSION_MINOR := 3
	VERSION_MICRO := 2
else
	VERSION_MAJOR := 1
	VERSION_MINOR := 0
	VERSION_MICRO := 1
endif

BUILD_FLAGS := -march=armv6k -mtune=mpcore -mfloat-abi=hard
BUILD_FLAGS_CC := -g -Wall -Wno-strict-aliasing -O3 -mword-relocations \
					-fomit-frame-pointer -ffast-math $(ARCH) $(INCLUDE) -DARM11 -D_3DS $(BUILD_FLAGS) \
					-DAPP_VERSION_MAJOR=${VERSION_MAJOR} \
					-DAPP_VERSION_MINOR=${VERSION_MINOR} \
					-DAPP_VERSION_MICRO=${VERSION_MICRO} \
					-DLAUNCHER_MODE=${LAUNCHER_M}

BUILD_FLAGS_CXX := $(COMMON_FLAGS) -std=gnu++11
RUN_FLAGS :=





# 3DS/Wii U CONFIGURATION #

ifeq ($(TARGET),$(filter $(TARGET),3DS WIIU))
	TITLE := MK7DX
	ifeq ($(LAUNCHER_M), 1)
		DESCRIPTION := Launcher & Updater
	else 
		DESCRIPTION := Installer
	endif
    AUTHOR := dshack.org
endif

# 3DS CONFIGURATION #

ifeq ($(TARGET),3DS)
    LIBRARY_DIRS += $(DEVKITPRO)/libctru $(DEVKITPRO)/portlibs/3ds/
    LIBRARIES += citro3d ctru png z m curl mbedtls mbedx509 mbedcrypto

    PRODUCT_CODE := CTR-P-MK7DX
    UNIQUE_ID := 0x3070C

    CATEGORY := Application
    USE_ON_SD := true

    MEMORY_TYPE := Application
    SYSTEM_MODE := 64MB
    SYSTEM_MODE_EXT := Legacy
    CPU_SPEED := 268MHz
    ENABLE_L2_CACHE := true

    ICON_FLAGS := --flags visible,recordusage

	ifeq ($(LAUNCHER_M), 1)
		ROMFS_DIR := romfs
	else 
		ROMFS_DIR := romfs_installer
	endif

    BANNER_AUDIO := resources/audio.cwav
    
    BANNER_IMAGE := resources/banner.cgfx
    
	ICON := resources/icon.png

	LOGO := resources/logo.bcma.lz
endif

# INTERNAL #

include buildtools/make_base

cleanupdater:
	@rm -f $(BUILD_DIR)/3ds-arm/source/updater.d $(BUILD_DIR)/3ds-arm/source/updater.o

re : clean all
.PHONY: re