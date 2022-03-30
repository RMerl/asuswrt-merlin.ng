# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018 Synopsys, Inc. All rights reserved.

bsp-generate: u-boot u-boot.bin
	$(Q)python3 $(srctree)/board/$(BOARDDIR)/headerize-hsdk.py \
		--arc-id 0x52 --image $(srctree)/u-boot.bin \
		--elf $(srctree)/u-boot
	$(Q)tools/mkimage -T script -C none -n 'uboot update script' \
		-d $(srctree)/u-boot-update.txt \
		$(srctree)/u-boot-update.scr &> /dev/null
