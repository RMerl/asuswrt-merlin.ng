# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Toradex, Inc.

# Common for all Toradex modules
ifeq ($(CONFIG_SPL_BUILD),y)
# Necessary to create built-in.o
obj- := __dummy__.o
else
obj-$(CONFIG_TDX_CFG_BLOCK) += tdx-cfg-block.o
obj-y += tdx-common.o
endif
