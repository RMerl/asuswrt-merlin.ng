# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2004
# Psyent Corporation <www.psyent.com>
# Scott McNutt <smcnutt@psyent.com>

PLATFORM_CPPFLAGS += -D__NIOS2__
PLATFORM_CPPFLAGS += -G0

LDFLAGS_FINAL += --gc-sections
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections
