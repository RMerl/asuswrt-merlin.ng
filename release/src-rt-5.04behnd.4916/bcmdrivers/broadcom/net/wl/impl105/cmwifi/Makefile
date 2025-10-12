#
# Toploevel makefile to build hnd dongle
#

ifeq ($(CM_BUILDROOT),)
CURR_DIR := $(CURDIR)
CURR_NAME := $(notdir $(CURDIR))
PARENT_DIR := $(word 1, $(subst /$(CURR_NAME), /$(CURR_NAME),$(CURR_DIR)))

export ALLSRCBASE := $(PARENT_DIR)
export WLSRCBASE= $(ALLSRCBASE)/main/src
export WLCFGDIR = $(ALLSRCBASE)/main/src/wl/config
export WLMAIN = main/src

export HNDDRIVER_BASE = main/components/router
export BCA_HNDROUTER=y
export PLT = arm
export LINUX_VERSION = 4_1_0
export KERNELDIR=${BUILD_DIR}/kernel
export LINUXDIR = $(KERNELDIR)/linux
export KBUILD_VERBOSE := 1
SUBMAKE_SETTINGS = SRCBASE=$(SRCBASE) BASEDIR=$(BASEDIR)

include $(BUILD_DIR)/make.common

include $(CURR_DIR)/cmwifi.mk

export USR_CFLAGS
export USR_LDFLAGS

-include $(KERNEL_DIR)/.config
# EXT_CPU_ARCH_NAME := $(subst \",,$(CONFIG_BCM_CPU_ARCH_NAME))
# export EXT_CPU_ARCH_NAME
# prebuilt_files = $(shell find . -name *$(EXT_CPU_ARCH_NAME) -print)
endif
