# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2017 NXP

obj-y += ls1088a.o
obj-y += ddr.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_TARGET_LS1088ARDB) += eth_ls1088ardb.o
obj-$(CONFIG_TARGET_LS1088AQDS) += eth_ls1088aqds.o
endif
