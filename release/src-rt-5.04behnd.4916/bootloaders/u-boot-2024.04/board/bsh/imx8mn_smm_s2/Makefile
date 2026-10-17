#
# Copyright 2021 Collabora Ltd.
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mn_smm_s2.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_BSH_SMM_S2_DDR3L_256) += ddr3l_timing_256m.o
obj-$(CONFIG_BSH_SMM_S2_DDR3L_512) += ddr3l_timing_512m.o
endif
