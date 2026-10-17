# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2019 Google LLC

obj-y += fsp_common.o
obj-y += fsp_dram.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_VIDEO_FSP) += fsp_graphics.o
endif
obj-y += fsp_support.o
