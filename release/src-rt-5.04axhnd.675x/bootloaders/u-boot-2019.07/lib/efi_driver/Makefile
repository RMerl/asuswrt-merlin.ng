# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2017 Heinrich Schuchardt

# This file only gets included with CONFIG_EFI_LOADER set, so all
# object inclusion implicitly depends on it

obj-y += efi_uclass.o
ifeq ($(CONFIG_BLK)$(CONFIG_PARTITIONS),yy)
obj-y += efi_block_device.o
endif
