# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
# Copyright (C) 2014-2021 Altera Corporation <www.altera.com>

ifdef CONFIG_$(SPL_)ALTERA_SDRAM
obj-$(CONFIG_TARGET_SOCFPGA_GEN5) += sdram_gen5.o sequencer.o
obj-$(CONFIG_TARGET_SOCFPGA_ARRIA10) += sdram_arria10.o
obj-$(CONFIG_TARGET_SOCFPGA_STRATIX10) += sdram_soc64.o sdram_s10.o
obj-$(CONFIG_TARGET_SOCFPGA_AGILEX) += sdram_soc64.o sdram_agilex.o
obj-$(CONFIG_TARGET_SOCFPGA_N5X) += sdram_soc64.o sdram_n5x.o
endif
