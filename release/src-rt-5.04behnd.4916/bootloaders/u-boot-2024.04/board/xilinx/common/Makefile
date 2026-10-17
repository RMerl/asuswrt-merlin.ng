# SPDX-License-Identifier: GPL-2.0
#
# (C) Copyright 2020 Xilinx, Inc.
# Michal Simek <michal.simek@amd.com>
#

obj-y	+= board.o
ifndef CONFIG_ARCH_ZYNQ
obj-$(CONFIG_DISPLAY_CPUINFO) += cpu-info.o
endif
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_FRU) += fru.o fru_ops.o
endif
