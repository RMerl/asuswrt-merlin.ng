# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y += input.o
obj-$(CONFIG_$(SPL_TPL_)CROS_EC_KEYB) += cros_ec_keyb.o
obj-$(CONFIG_$(SPL_TPL_)OF_CONTROL) += key_matrix.o
obj-$(CONFIG_$(SPL_TPL_)DM_KEYBOARD) += keyboard-uclass.o

ifndef CONFIG_SPL_BUILD

obj-$(CONFIG_I8042_KEYB) += i8042.o
obj-$(CONFIG_TEGRA_KEYBOARD) += tegra-kbc.o
obj-$(CONFIG_TWL4030_INPUT) += twl4030.o
obj-$(CONFIG_TWL6030_INPUT) += twl6030.o
endif
