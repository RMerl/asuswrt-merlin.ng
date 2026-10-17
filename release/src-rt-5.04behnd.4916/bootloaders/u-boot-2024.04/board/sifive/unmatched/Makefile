# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2020-2021 SiFive, Inc

obj-$(CONFIG_ID_EEPROM) += hifive-platform-i2c-eeprom.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y += unmatched.o
endif
