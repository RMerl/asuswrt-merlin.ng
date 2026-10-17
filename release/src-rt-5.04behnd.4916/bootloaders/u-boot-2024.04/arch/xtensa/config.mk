# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2007 - 2013 Tensilica, Inc.
# (C) Copyright 2014 - 2016 Cadence Design Systems Inc.

PLATFORM_CPPFLAGS += -D__XTENSA__ -mlongcalls -mforce-no-pic \
		     -ffunction-sections -fdata-sections

LDFLAGS_FINAL += --gc-sections
