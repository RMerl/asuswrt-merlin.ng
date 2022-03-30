# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2012-2017 Altera Corporation <www.altera.com>

obj-y	+= board.o
obj-y	+= clock_manager.o
obj-y	+= misc.o
obj-y	+= reset_manager.o

ifdef CONFIG_TARGET_SOCFPGA_GEN5
obj-y	+= clock_manager_gen5.o
obj-y	+= misc_gen5.o
obj-y	+= reset_manager_gen5.o
obj-y	+= scan_manager.o
obj-y	+= system_manager_gen5.o
obj-y	+= timer.o
obj-y	+= wrap_pll_config.o
obj-y	+= fpga_manager.o
endif

ifdef CONFIG_TARGET_SOCFPGA_ARRIA10
obj-y	+= clock_manager_arria10.o
obj-y	+= misc_arria10.o
obj-y	+= pinmux_arria10.o
obj-y	+= reset_manager_arria10.o
endif

ifdef CONFIG_TARGET_SOCFPGA_STRATIX10
obj-y	+= clock_manager_s10.o
obj-y	+= mailbox_s10.o
obj-y	+= misc_s10.o
obj-y	+= mmu-arm64_s10.o
obj-y	+= reset_manager_s10.o
obj-y	+= system_manager_s10.o
obj-y	+= timer_s10.o
obj-y	+= wrap_pinmux_config_s10.o
obj-y	+= wrap_pll_config_s10.o
endif

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_TARGET_SOCFPGA_GEN5
obj-y	+= spl_gen5.o
obj-y	+= freeze_controller.o
obj-y	+= wrap_iocsr_config.o
obj-y	+= wrap_pinmux_config.o
obj-y	+= wrap_sdram_config.o
endif
ifdef CONFIG_TARGET_SOCFPGA_ARRIA10
obj-y	+= spl_a10.o
endif
ifdef CONFIG_TARGET_SOCFPGA_STRATIX10
obj-y	+= spl_s10.o
endif
endif

ifdef CONFIG_TARGET_SOCFPGA_GEN5
# QTS-generated config file wrappers
CFLAGS_wrap_iocsr_config.o	+= -I$(srctree)/board/$(BOARDDIR)
CFLAGS_wrap_pinmux_config.o	+= -I$(srctree)/board/$(BOARDDIR)
CFLAGS_wrap_pll_config.o	+= -I$(srctree)/board/$(BOARDDIR)
CFLAGS_wrap_sdram_config.o	+= -I$(srctree)/board/$(BOARDDIR)
endif
