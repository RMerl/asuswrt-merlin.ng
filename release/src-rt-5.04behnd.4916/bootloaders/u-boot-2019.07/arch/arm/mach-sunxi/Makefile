# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
#
# Based on some other Makefile
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	+= board.o
obj-y	+= clock.o
obj-y	+= cpu_info.o
obj-y	+= dram_helpers.o
obj-y	+= pinmux.o
obj-$(CONFIG_SUN6I_P2WI)	+= p2wi.o
obj-$(CONFIG_SUN6I_PRCM)	+= prcm.o
obj-$(CONFIG_AXP_PMIC_BUS)	+= pmic_bus.o
obj-$(CONFIG_SUN8I_RSB)		+= rsb.o
obj-$(CONFIG_MACH_SUN4I)	+= clock_sun4i.o
obj-$(CONFIG_MACH_SUN5I)	+= clock_sun4i.o
obj-$(CONFIG_MACH_SUN6I)	+= clock_sun6i.o
obj-$(CONFIG_MACH_SUN7I)	+= clock_sun4i.o
obj-$(CONFIG_MACH_SUN50I)	+= clock_sun6i.o
ifdef CONFIG_MACH_SUN8I_A83T
obj-y				+= clock_sun8i_a83t.o
else
obj-$(CONFIG_MACH_SUN8I)	+= clock_sun6i.o
endif
obj-$(CONFIG_MACH_SUN9I)	+= clock_sun9i.o gtbus_sun9i.o
obj-$(CONFIG_MACH_SUN50I_H6)	+= clock_sun50i_h6.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_DRAM_SUN4I)	+= dram_sun4i.o
obj-$(CONFIG_DRAM_SUN6I)	+= dram_sun6i.o
obj-$(CONFIG_DRAM_SUN8I_A23)	+= dram_sun8i_a23.o
obj-$(CONFIG_DRAM_SUN8I_A33)	+= dram_sun8i_a33.o
obj-$(CONFIG_DRAM_SUN8I_A83T)	+= dram_sun8i_a83t.o
obj-$(CONFIG_DRAM_SUN9I)	+= dram_sun9i.o
obj-$(CONFIG_SPL_SPI_SUNXI)	+= spl_spi_sunxi.o
obj-$(CONFIG_SUNXI_DRAM_DW)	+= dram_sunxi_dw.o
obj-$(CONFIG_SUNXI_DRAM_DW)	+= dram_timings/
obj-$(CONFIG_DRAM_SUN50I_H6)	+= dram_sun50i_h6.o
endif
