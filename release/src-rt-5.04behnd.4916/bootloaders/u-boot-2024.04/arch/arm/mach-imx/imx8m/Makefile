# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2017 NXP

obj-y += lowlevel_init.o
obj-y += clock_slice.o soc.o
obj-$(CONFIG_ARMV8_PSCI) += psci.o
obj-$(CONFIG_IMX8MQ) += clock_imx8mq.o
obj-$(CONFIG_IMX8MM)$(CONFIG_IMX8MN)$(CONFIG_IMX8MP) += clock_imx8mm.o
