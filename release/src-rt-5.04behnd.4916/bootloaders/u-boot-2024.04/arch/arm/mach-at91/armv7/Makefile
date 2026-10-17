# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2013
# Bo Shen <voice.shen@atmel.com>

obj-$(CONFIG_SAMA5D2)	+= sama5d2_devices.o clock.o
obj-$(CONFIG_SAMA5D3)	+= sama5d3_devices.o clock.o
obj-$(CONFIG_SAMA5D4)	+= sama5d4_devices.o clock.o
obj-$(CONFIG_SAMA7G5)	+= sama7g5_devices.o
obj-y += cpu.o
ifneq ($(CONFIG_ATMEL_TCB_TIMER),y)
ifneq ($(CONFIG_ATMEL_PIT_TIMER),y)
ifneq ($(CONFIG_MCHP_PIT64B_TIMER),y)
# old non-DM timer driver
obj-y += timer.o
endif
endif
endif
