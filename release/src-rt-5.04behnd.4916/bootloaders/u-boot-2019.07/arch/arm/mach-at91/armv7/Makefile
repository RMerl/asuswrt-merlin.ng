# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2013
# Bo Shen <voice.shen@atmel.com>

obj-$(CONFIG_SAMA5D2)	+= sama5d2_devices.o
obj-$(CONFIG_SAMA5D3)	+= sama5d3_devices.o
obj-$(CONFIG_SAMA5D4)	+= sama5d4_devices.o
obj-y += clock.o
obj-y += cpu.o
obj-y += reset.o
ifeq ($(CONFIG_ATMEL_PIT_TIMER),)
obj-y += timer.o
endif
