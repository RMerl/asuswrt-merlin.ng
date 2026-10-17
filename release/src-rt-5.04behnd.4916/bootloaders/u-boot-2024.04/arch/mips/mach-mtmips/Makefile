# SPDX-License-Identifier: GPL-2.0+

obj-y += cpu.o

ifneq ($(CONFIG_SOC_MT7621),y)
obj-y += ddr_init.o
obj-y += ddr_cal.o
obj-$(CONFIG_SPL_BUILD) += spl.o
endif

obj-$(CONFIG_SOC_MT7620) += mt7620/
obj-$(CONFIG_SOC_MT7621) += mt7621/
obj-$(CONFIG_SOC_MT7628) += mt7628/
