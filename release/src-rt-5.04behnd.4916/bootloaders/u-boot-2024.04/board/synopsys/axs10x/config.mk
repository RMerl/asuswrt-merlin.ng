# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018 Synopsys, Inc. All rights reserved.

ifdef CONFIG_TARGET_AXS103
  PLATFORM_CPPFLAGS += -mcpu=archs
else
  PLATFORM_CPPFLAGS += -mcpu=arc700 -mlock -mswape
endif

bsp-generate: u-boot u-boot.bin
ifdef CONFIG_ISA_ARCV2
	$(Q)python3 $(srctree)/board/$(BOARDDIR)/headerize-axs.py \
		--header-type v2 \
		--arc-id 0x53 \
		--spi-flash-offset 0x200000 \
		--image $(srctree)/u-boot.bin \
		--elf $(srctree)/u-boot
else
	$(Q)python3 $(srctree)/board/$(BOARDDIR)/headerize-axs.py \
		--header-type v1 \
		--arc-id 0x434 \
		--spi-flash-offset 0x0 \
		--image $(srctree)/u-boot.bin \
		--elf $(srctree)/u-boot
endif
	$(Q)tools/mkimage -T script -C none -n 'uboot update script' \
		-d $(srctree)/u-boot-update.txt \
		$(srctree)/u-boot-update.img &> /dev/null
