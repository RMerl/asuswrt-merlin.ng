# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2019-2020 PHYTEC Messtechnik GmbH
# Author: Teresa Remmet <t.remmet@phytec.de>

obj-y += phycore-imx8mm.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
endif
