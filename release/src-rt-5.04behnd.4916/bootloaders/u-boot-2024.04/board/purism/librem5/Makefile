#
# Copyright 2017 NXP
# Copyright 2019 Purism
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += librem5.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o lpddr4_timing_b0.o
endif
