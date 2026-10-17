#
# Menlosystems MX8Menlo
# Copyright (C) 2021-2022 Marek Vasut <marex@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	:= mx8menlo.o

obj-y += ../../toradex/verdin-imx8mm/verdin-imx8mm.o

ifdef CONFIG_SPL_BUILD
obj-y += ../../toradex/verdin-imx8mm/spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += ../../toradex/verdin-imx8mm/lpddr4_timing.o
endif

# Common for all Toradex modules
ifeq ($(CONFIG_SPL_BUILD),y)
# Necessary to create built-in.o
obj- := __dummy__.o
else
obj-$(CONFIG_TDX_CFG_BLOCK) += ../../toradex/common/tdx-cfg-block.o
obj-y += ../../toradex/common/tdx-common.o
obj-y += ../../toradex/common/tdx-eeprom.o
endif
