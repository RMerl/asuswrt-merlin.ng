# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.

ifndef CONFIG_CPU_BIG_ENDIAN
CONFIG_SYS_LITTLE_ENDIAN = 1
else
CONFIG_SYS_BIG_ENDIAN = 1
endif

ifdef CONFIG_SYS_LITTLE_ENDIAN
PLATFORM_LDFLAGS += -EL
PLATFORM_CPPFLAGS += -mlittle-endian
endif

ifdef CONFIG_SYS_BIG_ENDIAN
PLATFORM_LDFLAGS += -EB
PLATFORM_CPPFLAGS += -mbig-endian
endif

ifdef CONFIG_ARC_MMU_VER
CONFIG_MMU = 1
endif

ifdef CONFIG_CPU_ARC750D
PLATFORM_CPPFLAGS += -mcpu=arc700
endif

ifdef CONFIG_CPU_ARC770D
PLATFORM_CPPFLAGS += -mcpu=arc700 -mlock -mswape
endif

ifdef CONFIG_CPU_ARCEM6
PLATFORM_CPPFLAGS += -mcpu=arcem
endif

ifdef CONFIG_CPU_ARCHS34
PLATFORM_CPPFLAGS += -mcpu=archs
endif

ifdef CONFIG_CPU_ARCHS38
PLATFORM_CPPFLAGS += -mcpu=archs
endif

PLATFORM_CPPFLAGS += -ffixed-r25 -D__ARC__ -gdwarf-2 -mno-sdata
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections -fno-common

# Needed for relocation
LDFLAGS_FINAL += -pie --gc-sections

# Load address for standalone apps
CONFIG_STANDALONE_LOAD_ADDR ?= 0x82000000
