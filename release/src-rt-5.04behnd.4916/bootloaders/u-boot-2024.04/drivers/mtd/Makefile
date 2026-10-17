# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

mtd-$(CONFIG_MTD) += mtdcore.o mtd_uboot.o
mtd-$(CONFIG_DM_MTD) += mtd-uclass.o
mtd-$(CONFIG_MTD_PARTITIONS) += mtdpart.o
mtd-$(CONFIG_MTD_CONCAT) += mtdconcat.o
mtd-$(CONFIG_ALTERA_QSPI) += altera_qspi.o
mtd-$(CONFIG_FLASH_CFI_DRIVER) += cfi_flash.o
mtd-$(CONFIG_FLASH_CFI_MTD) += cfi_mtd.o
mtd-$(CONFIG_FLASH_CFI_LEGACY) += jedec_flash.o
mtd-$(CONFIG_STM32_FLASH) += stm32_flash.o
mtd-$(CONFIG_RENESAS_RPC_HF) += renesas_rpc_hf.o
mtd-$(CONFIG_HBMC_AM654) += hbmc-am654.o

# U-Boot build
ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TPL_BUILD),)

ifneq ($(mtd-y),)
obj-y += mtd.o
endif
obj-y += nand/
obj-y += onenand/
obj-y += spi/
obj-$(CONFIG_MTD_UBI) += ubi/
obj-$(CONFIG_NVMXIP) += nvmxip/

#SPL/TPL build
else

ifneq ($(mtd-y),)
obj-$(CONFIG_SPL_MTD) += mtd.o
endif
obj-$(CONFIG_$(SPL_TPL_)NAND_SUPPORT) += nand/
obj-$(CONFIG_SPL_ONENAND_SUPPORT) += onenand/
obj-$(CONFIG_$(SPL_TPL_)SPI_FLASH_SUPPORT) += spi/
obj-$(CONFIG_$(SPL_TPL_)UBI) += ubispl/

endif
