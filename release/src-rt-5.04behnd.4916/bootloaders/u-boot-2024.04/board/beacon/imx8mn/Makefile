#
# Copyright 2020 Compass Electronics Group, LLC
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mn_beacon.o
obj-y += ../../freescale/common/
ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX8MN_BEACON_2GB_LPDDR
obj-y += lpddr4_2g_timing.o
else
obj-y += lpddr4_timing.o
endif
endif
