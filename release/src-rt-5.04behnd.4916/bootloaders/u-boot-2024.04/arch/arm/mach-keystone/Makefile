#
# (C) Copyright 2012-2014
#     Texas Instruments Incorporated, <www.ti.com>
#
# SPDX-License-Identifier:     GPL-2.0+
#

obj-y	+= init.o
obj-y	+= psc.o
obj-y	+= clock.o
obj-y	+= mon.o
CFLAGS_REMOVE_mon.o := $(LTO_CFLAGS)
ifndef CONFIG_SPL_BUILD
obj-y	+= cmd_clock.o
obj-y	+= cmd_mon.o
obj-y	+= cmd_poweroff.o
endif
obj-y	+= msmc.o
obj-y	+= ddr3.o
obj-y	+= keystone.o
obj-$(CONFIG_TARGET_K2E_EVM) += ddr3_spd.o
obj-$(CONFIG_TARGET_K2HK_EVM) += ddr3_spd.o
