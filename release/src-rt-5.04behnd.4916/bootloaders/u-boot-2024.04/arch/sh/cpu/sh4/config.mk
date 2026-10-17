# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
PLATFORM_CPPFLAGS += $(call cc-option,-m4-nofpu,-m4)
