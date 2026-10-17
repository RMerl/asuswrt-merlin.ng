#
# Copyright 2019 NXP
# Copyright 2022 Linaro
#
# SPDX-License-Identifier:      GPL-2.0+
#

ifdef CONFIG_TARGET_IMX8MP_RSB3720A1_6G
obj-y += imx8mp_rsb3720a1.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_rsb3720a1_6G.o
endif
endif

ifdef CONFIG_TARGET_IMX8MP_RSB3720A1_4G
obj-y += imx8mp_rsb3720a1.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_rsb3720a1_4G.o
endif
endif
