# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y += common.o env.o

ifndef CONFIG_SPL_BUILD
obj-y += attr.o
obj-y += callback.o
obj-y += flags.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += eeprom.o
extra-$(CONFIG_ENV_IS_EMBEDDED) += embedded.o
obj-$(CONFIG_ENV_IS_IN_EEPROM) += embedded.o
extra-$(CONFIG_ENV_IS_IN_FLASH) += embedded.o
obj-$(CONFIG_ENV_IS_IN_NVRAM) += embedded.o
obj-$(CONFIG_ENV_IS_IN_NVRAM) += nvram.o
obj-$(CONFIG_ENV_IS_IN_ONENAND) += onenand.o
obj-$(CONFIG_ENV_IS_IN_SATA) += sata.o
obj-$(CONFIG_ENV_IS_IN_REMOTE) += remote.o
obj-$(CONFIG_ENV_IS_IN_UBI) += ubi.o
obj-$(CONFIG_ENV_IS_IN_BOOT_MAGIC) += boot_magic.o
else
obj-$(CONFIG_$(SPL_TPL_)ENV_SUPPORT) += attr.o
obj-$(CONFIG_$(SPL_TPL_)ENV_SUPPORT) += flags.o
obj-$(CONFIG_$(SPL_TPL_)ENV_SUPPORT) += callback.o
endif

obj-$(CONFIG_$(SPL_TPL_)ENV_IS_NOWHERE) += nowhere.o
obj-$(CONFIG_$(SPL_TPL_)ENV_IS_IN_MMC) += mmc.o
obj-$(CONFIG_$(SPL_TPL_)ENV_IS_IN_FAT) += fat.o
obj-$(CONFIG_$(SPL_TPL_)ENV_IS_IN_EXT4) += ext4.o
obj-$(CONFIG_$(SPL_TPL_)ENV_IS_IN_NAND) += nand.o
obj-$(CONFIG_$(SPL_TPL_)ENV_IS_IN_SPI_FLASH) += sf.o
obj-$(CONFIG_$(SPL_TPL_)ENV_IS_IN_FLASH) += flash.o

CFLAGS_embedded.o := -Wa,--no-warn -DENV_CRC=$(shell tools/envcrc 2>/dev/null)
