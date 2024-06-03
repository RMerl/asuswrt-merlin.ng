# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2004 Freescale Semiconductor, Inc.

PLATFORM_CPPFLAGS += -DCONFIG_E300 -msoft-float
PLATFORM_RELFLAGS += -msingle-pic-base -fno-jump-tables
