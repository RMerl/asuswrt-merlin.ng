# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2014-2016 Stefan Roese <sr@denx.de>

ifdef CONFIG_ARM64

obj-$(CONFIG_ARMADA_3700) += armada3700/
obj-$(CONFIG_ARMADA_8K) += armada8k/
obj-$(CONFIG_ALLEYCAT_5) += alleycat5/
obj-y += arm64-common.o

else # CONFIG_ARM64

ifdef CONFIG_ARCH_KIRKWOOD

obj-y	= dram.o
obj-y	+= gpio.o
obj-y	+= mbus.o

else # CONFIG_ARCH_KIRKWOOD

obj-y	= cpu.o
obj-y	+= dram.o
obj-y	+= lowlevel.o
obj-$(CONFIG_DM_RESET) += system-controller.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMADA_375) += ../../../drivers/ddr/marvell/axp/xor.o
obj-$(CONFIG_ARMADA_38X) += ../../../drivers/ddr/marvell/a38x/xor.o
obj-$(CONFIG_ARMADA_XP) += ../../../drivers/ddr/marvell/axp/xor.o
obj-$(CONFIG_ARMADA_MSYS) += ../../../drivers/ddr/marvell/axp/xor.o

ifdef CONFIG_ARMADA_38X
obj-$(CONFIG_MVEBU_EFUSE) += efuse.o
endif

extra-y += kwbimage.cfg

ifneq ($(CONFIG_ARMADA_XP),)
	KWB_REPLACE += CPU
	KWB_CFG_CPU = SHEEVA
else ifneq ($(CONFIG_ARMADA_375)$(CONFIG_ARMADA_38X),)
	KWB_REPLACE += CPU
	KWB_CFG_CPU = A9
endif

KWB_REPLACE += LOAD_ADDRESS
KWB_CFG_LOAD_ADDRESS = $(CONFIG_SPL_TEXT_BASE)

KWB_REPLACE += BOOT_FROM
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI),)
	KWB_CFG_BOOT_FROM=spi
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_NAND),)
	KWB_CFG_BOOT_FROM=nand
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC),)
	KWB_CFG_BOOT_FROM=sdio
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SATA),)
	KWB_CFG_BOOT_FROM=sata
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_PEX),)
	KWB_CFG_BOOT_FROM=pex
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_UART),)
	KWB_CFG_BOOT_FROM=uart
endif

ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_NAND),)
KWB_REPLACE += NAND_PAGE_SIZE NAND_BLKSZ NAND_BADBLK_LOCATION
KWB_CFG_NAND_PAGE_SIZE = $(CONFIG_SYS_NAND_PAGE_SIZE)
KWB_CFG_NAND_BLKSZ = $(CONFIG_SYS_NAND_BLOCK_SIZE)
KWB_CFG_NAND_BADBLK_LOCATION = $(CONFIG_MVEBU_SPL_NAND_BADBLK_LOCATION)
endif

ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SATA),)
KWB_REPLACE += SATA_BLKSZ
KWB_CFG_SATA_BLKSZ = $(CONFIG_MVEBU_SPL_SATA_BLKSZ)
endif

ifneq ($(CONFIG_SECURED_MODE_IMAGE),)
KWB_REPLACE += CSK_INDEX
KWB_CFG_CSK_INDEX = $(CONFIG_SECURED_MODE_CSK_INDEX)

KWB_REPLACE += SEC_BOOT_DEV
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI),)
	KWB_CFG_SEC_BOOT_DEV=0x34
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC),)
	KWB_CFG_SEC_BOOT_DEV=0x31
endif

KWB_REPLACE += SEC_FUSE_DUMP
KWB_CFG_SEC_FUSE_DUMP = a38x
endif

ifdef CONFIG_ARMADA_38X
# BootROM output is by default enabled on pre-A38x and disabled on A38x
# DEBUG flag on A38x for non-UART boot source only enable BootROM output and nothing more
KWB_REPLACE += DEBUG
KWB_CFG_DEBUG = 1
endif

quiet_cmd_kwbcfg = KWBCFG  $@
cmd_kwbcfg = sed -ne '$(foreach V,$(KWB_REPLACE),s/\#@$(V)/$(V) $(KWB_CFG_$(V))/;)p' \
	<$< >$(dir $@)$(@F)

$(obj)/kwbimage.cfg: $(src)/kwbimage.cfg.in include/autoconf.mk \
		include/config/auto.conf
	$(call cmd,kwbcfg)

endif # CONFIG_SPL_BUILD
obj-y	+= gpio.o
obj-y	+= mbus.o
obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_SPL_BUILD) += lowlevel_spl.o

obj-$(CONFIG_ARMADA_38X) += serdes/a38x/
obj-$(CONFIG_ARMADA_XP) += serdes/axp/

endif # CONFIG_ARCH_KIRKWOOD
endif # CONFIG_ARM64
