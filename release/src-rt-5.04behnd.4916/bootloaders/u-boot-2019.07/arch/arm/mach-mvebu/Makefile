# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2014-2016 Stefan Roese <sr@denx.de>

ifdef CONFIG_ARM64

obj-$(CONFIG_ARMADA_3700) += armada3700/
obj-$(CONFIG_ARMADA_8K) += armada8k/
obj-y += arm64-common.o

else # CONFIG_ARM64

ifdef CONFIG_KIRKWOOD

obj-y	= dram.o
obj-y	+= gpio.o
obj-y	+= mbus.o
obj-y	+= timer.o

else # CONFIG_KIRKWOOD

obj-y	= cpu.o
obj-y	+= dram.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMADA_375) += ../../../drivers/ddr/marvell/axp/xor.o
obj-$(CONFIG_ARMADA_38X) += ../../../drivers/ddr/marvell/a38x/xor.o
obj-$(CONFIG_ARMADA_XP) += ../../../drivers/ddr/marvell/axp/xor.o
obj-$(CONFIG_ARMADA_MSYS) += ../../../drivers/ddr/marvell/axp/xor.o
obj-$(CONFIG_MVEBU_EFUSE) += efuse.o

extra-y += kwbimage.cfg

KWB_REPLACE += BOOT_FROM
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_SPI),)
	KWB_CFG_BOOT_FROM=spi
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_MMC),)
	KWB_CFG_BOOT_FROM=sdio
endif
ifneq ($(CONFIG_MVEBU_SPL_BOOT_DEVICE_UART),)
	KWB_CFG_BOOT_FROM=uart
endif

ifneq ($(CONFIG_SECURED_MODE_IMAGE),)
KWB_REPLACE += CSK_INDEX
KWB_CFG_CSK_INDEX = $(CONFIG_SECURED_MODE_CSK_INDEX)

KWB_REPLACE += SEC_BOOT_DEV
KWB_CFG_SEC_BOOT_DEV=$(patsubst "%",%, \
	$(if $(findstring BOOT_SPI_NOR_FLASH,$(CONFIG_SPL_BOOT_DEVICE)),0x34) \
	$(if $(findstring BOOT_SDIO_MMC_CARD,$(CONFIG_SPL_BOOT_DEVICE)),0x31) \
	)

KWB_REPLACE += SEC_FUSE_DUMP
KWB_CFG_SEC_FUSE_DUMP = a38x
endif

$(src)/kwbimage.cfg: $(src)/kwbimage.cfg.in include/autoconf.mk \
		include/config/auto.conf
	$(Q)sed -ne '$(foreach V,$(KWB_REPLACE),s/^#@$(V)/$(V) $(KWB_CFG_$(V))/;)p' \
	<$< >$(dir $<)$(@F)

endif # CONFIG_SPL_BUILD
obj-y	+= gpio.o
obj-y	+= mbus.o
obj-y	+= timer.o
obj-$(CONFIG_SPL_BUILD) += spl.o
obj-$(CONFIG_SPL_BUILD) += lowlevel_spl.o

obj-$(CONFIG_ARMADA_38X) += serdes/a38x/
obj-$(CONFIG_ARMADA_XP) += serdes/axp/

endif # CONFIG_KIRKWOOD
endif # CONFIG_ARM64
