# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2012 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>

obj-$(CONFIG_USB_GADGET_DOWNLOAD) += gadget.o
obj-$(CONFIG_MISC_COMMON) += misc.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_BOARD_COMMON)	+= board.o
ifdef CONFIG_EXYNOS5_DT
obj-y += exynos5-dt.o
obj-$(CONFIG_BOARD_TYPES) += exynos5-dt-types.o
endif
endif
