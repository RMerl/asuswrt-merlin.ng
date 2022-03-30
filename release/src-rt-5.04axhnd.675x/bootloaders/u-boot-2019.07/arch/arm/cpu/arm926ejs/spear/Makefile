# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	:= cpu.o \
	   reset.o \
	   timer.o

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
obj-$(CONFIG_SPEAR600) += spear600.o
obj-$(CONFIG_DDR_MT47H64M16) += spr600_mt47h64m16_3_333_cl5_psync.o
obj-$(CONFIG_DDR_MT47H32M16) += spr600_mt47h32m16_333_cl5_psync.o
obj-$(CONFIG_DDR_MT47H32M16) += spr600_mt47h32m16_37e_166_cl4_sync.o
obj-$(CONFIG_DDR_MT47H128M8) += spr600_mt47h128m8_3_266_cl5_async.o
else
obj-y += spr_misc.o spr_lowlevel_init.o
endif

extra-$(CONFIG_SPL_BUILD) := start.o
