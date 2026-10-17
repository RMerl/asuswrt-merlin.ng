#
# Copyright 2018 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mn_evk.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX8MN_LOW_DRIVE_MODE
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_ld.o
obj-$(CONFIG_IMX8M_DDR4) += ddr4_timing_ld.o
else
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
obj-$(CONFIG_IMX8M_DDR4) += ddr4_timing.o
endif
endif
