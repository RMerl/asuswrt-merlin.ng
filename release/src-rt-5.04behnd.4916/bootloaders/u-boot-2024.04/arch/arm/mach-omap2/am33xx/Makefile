# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/

obj-$(CONFIG_AM33XX)	+= clock_am33xx.o
obj-$(CONFIG_AM43XX)	+= clock_am43xx.o

ifneq ($(CONFIG_AM43XX)$(CONFIG_AM33XX),)
obj-y	+= clock.o
endif

obj-y	+= sys_info.o
obj-y	+= ddr.o
ifeq ($(CONFIG_$(SPL_)SKIP_LOWLEVEL_INIT),)
obj-y	+= emif4.o
endif
obj-y	+= board.o
obj-y	+= mux.o
obj-y	+= prcm-regs.o
obj-y	+= hw_data.o
obj-y	+= fdt.o

obj-$(CONFIG_CLOCK_SYNTHESIZER)	+= clk_synthesizer.o

obj-$(CONFIG_AM33XX_CHILISOM) += chilisom.o
