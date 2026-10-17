# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifeq ($(CPU),sh2)
LDFLAGS_STANDALONE += -EB
endif

PLATFORM_CPPFLAGS += -DCONFIG_SH -D__SH__
PLATFORM_RELFLAGS += -fpic -ffunction-sections -fdata-sections
LDFLAGS_FINAL = --gc-sections
PLATFORM_RELFLAGS += -ffixed-r13
