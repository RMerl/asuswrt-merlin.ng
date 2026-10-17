# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018 Synopsys, Inc. All rights reserved.

ifdef CONFIG_BOARD_HSDK
PLATFORM_CPPFLAGS += -mcpu=hs38_linux -mlittle-endian -matomic -mll64 \
                     -mdiv-rem -mswap -mnorm -mmpy-option=9 -mbarrel-shifter \
                     -mfpu=fpud_all

bsp-generate: u-boot u-boot.bin
	$(Q)python3 $(srctree)/board/$(BOARDDIR)/headerize-hsdk.py \
		--arc-id 0x52 --image $(srctree)/u-boot.bin \
		--elf $(srctree)/u-boot
	$(Q)tools/mkimage -T script -C none -n 'uboot update script' \
		-d $(srctree)/u-boot-update.txt \
		$(srctree)/u-boot-update.scr &> /dev/null
endif

ifdef CONFIG_BOARD_HSDK_4XD
PLATFORM_CPPFLAGS += -mcpu=hs4x_rel31 -mlittle-endian -matomic -mll64 \
                     -mdiv-rem -mswap -mnorm -mmpy-option=9 -mbarrel-shifter \
                     -mfpu=fpud_all

bsp-generate: u-boot u-boot.bin
	$(Q)python3 $(srctree)/board/$(BOARDDIR)/headerize-hsdk.py \
		--arc-id 0x54 --image $(srctree)/u-boot.bin \
		--elf $(srctree)/u-boot
	$(Q)tools/mkimage -T script -C none -n 'uboot update script' \
		-d $(srctree)/u-boot-update.txt \
		$(srctree)/u-boot-update.scr &> /dev/null
endif
