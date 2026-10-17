# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#
# (C) Copyright 2008
# Graeme Russ, graeme.russ@gmail.com.
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002
# Daniel Engström, Omicron Ceti AB, daniel@omicron.se.

ifndef CONFIG_SPL
obj-y += car.o
endif
ifdef CONFIG_SPL_BUILD
obj-y += coreboot_spl.o
else
obj-y += sdram.o
endif
obj-y += coreboot.o
obj-y += timestamp.o
