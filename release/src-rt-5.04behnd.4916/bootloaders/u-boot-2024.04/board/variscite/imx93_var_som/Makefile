#
# Copyright 2022 NXP
# Copyright 2023 Variscite Ltd.
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx93_var_som.o
obj-$(CONFIG_TARGET_IMX93_VAR_SOM) += ../common/imx9_eeprom.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_TARGET_IMX93_VAR_SOM) += lpddr4x_timing.o
else
obj-y += ../common/eth.o
obj-y += ../common/mmc.o
endif
