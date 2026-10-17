# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.

ifdef CONFIG_SYS_LITTLE_ENDIAN
KBUILD_LDFLAGS += -EL
PLATFORM_CPPFLAGS += -mlittle-endian
endif

ifdef CONFIG_SYS_BIG_ENDIAN
KBUILD_LDFLAGS += -EB
PLATFORM_CPPFLAGS += -mbig-endian
endif

PLATFORM_CPPFLAGS += -ffixed-r25 -D__ARC__ -gdwarf-2 -mno-sdata
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections -fno-common

# Needed for relocation
LDFLAGS_FINAL += -pie --gc-sections
