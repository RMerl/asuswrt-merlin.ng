# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

# If clock.c is compiled for Thumb2, then it fails on OMAP3530
CFLAGS_clock.o += -marm

obj-y	:= lowlevel_init.o

obj-y	+= board.o
obj-y	+= boot.o
obj-y	+= clock.o
obj-y	+= sys_info.o
obj-y	+= prcm-regs.o
obj-y	+= hw_data.o
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_OMAP3_ID_NAND)	+= spl_id_nand.o
endif

obj-$(CONFIG_DRIVER_TI_EMAC)	+= emac.o
obj-$(CONFIG_EMIF4)	+= emif4.o
obj-$(CONFIG_SDRC)	+= sdrc.o
obj-$(CONFIG_USB_MUSB_AM35X)	+= am35x_musb.o
