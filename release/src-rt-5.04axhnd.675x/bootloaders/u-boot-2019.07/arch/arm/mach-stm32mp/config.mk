# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

ifndef CONFIG_SPL
ALL-y += u-boot.stm32
else
ifdef CONFIG_SPL_BUILD
ALL-y += u-boot-spl.stm32
endif
endif

MKIMAGEFLAGS_u-boot.stm32 = -T stm32image -a $(CONFIG_SYS_TEXT_BASE) -e $(CONFIG_SYS_TEXT_BASE)

u-boot.stm32: MKIMAGEOUTPUT = u-boot.stm32.log

u-boot.stm32: u-boot.bin FORCE
	$(call if_changed,mkimage)

MKIMAGEFLAGS_u-boot-spl.stm32 = -T stm32image -a $(CONFIG_SPL_TEXT_BASE) -e $(CONFIG_SPL_TEXT_BASE)

spl/u-boot-spl.stm32: MKIMAGEOUTPUT = spl/u-boot-spl.stm32.log

spl/u-boot-spl.stm32: spl/u-boot-spl.bin FORCE
	$(call if_changed,mkimage)

u-boot-spl.stm32 : spl/u-boot-spl.stm32
	$(call if_changed,copy)
