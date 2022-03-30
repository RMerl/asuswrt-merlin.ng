# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2014, Barco (www.barco.com)

obj-y := platinum.o
obj-$(CONFIG_TARGET_PLATINUM_PICON)     += platinum_picon.o
obj-$(CONFIG_TARGET_PLATINUM_TITANIUM)  += platinum_titanium.o

ifneq ($(CONFIG_SPL_BUILD),)
obj-$(CONFIG_TARGET_PLATINUM_PICON) += spl_picon.o
obj-$(CONFIG_TARGET_PLATINUM_TITANIUM) += spl_titanium.o
endif
