# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

obj-y += cpu.o
obj-y += mmu_table.o
ifndef CONFIG_TPL_BUILD
# SPL
obj-$(CONFIG_SPL_BUILD)	+= pmc.o
endif

ifndef CONFIG_SPL_BUILD
# u-boot proper
obj-$(CONFIG_BCMBCA_RDP) += rdp_fw/
endif