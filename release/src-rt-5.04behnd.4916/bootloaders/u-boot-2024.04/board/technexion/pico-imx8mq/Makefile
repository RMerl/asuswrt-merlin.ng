#
# Copyright 2017 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += pico-imx8mq.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_1gb.o lpddr4_timing_2gb.o lpddr4_timing_3gb.o lpddr4_timing_4gb.o
endif
