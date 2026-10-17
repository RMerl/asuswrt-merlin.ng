# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2007-2008 Michal Simek
# Michal SIMEK <monstr@monstr.eu>
#
# (C) Copyright 2004 Atmark Techno, Inc.
# Yasushi SHOJI <yashi@atmark-techno.com>

PLATFORM_CPPFLAGS += -ffixed-r31 -D__microblaze__
PLATFORM_CPPFLAGS += -fdata-sections -ffunction-sections

LDFLAGS_FINAL += --gc-sections

ifeq ($(CONFIG_SPL_BUILD),)
PLATFORM_CPPFLAGS += -fPIC
LDFLAGS_u-boot += -pic
endif

ifeq ($(CONFIG_SYS_LITTLE_ENDIAN),y)
PLATFORM_ELFFLAGS += -B microblaze $(OBJCOPYFLAGS) -O elf32-microblazeel
else
PLATFORM_ELFFLAGS += -B microblaze $(OBJCOPYFLAGS) -O elf32-microblaze
endif
