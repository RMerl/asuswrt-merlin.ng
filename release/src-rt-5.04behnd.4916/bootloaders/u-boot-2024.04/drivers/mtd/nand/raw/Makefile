# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifdef CONFIG_SPL_BUILD

ifdef CONFIG_SPL_NAND_DRIVERS
NORMAL_DRIVERS=y
endif

obj-$(CONFIG_SPL_NAND_AM33XX_BCH) += am335x_spl_bch.o
obj-$(CONFIG_SPL_NAND_DENALI) += denali_spl.o
obj-$(CONFIG_SPL_NAND_SIMPLE) += nand_spl_simple.o
obj-$(CONFIG_SPL_NAND_LOAD) += nand_spl_load.o
obj-$(CONFIG_SPL_NAND_ECC) += nand_ecc.o
obj-$(CONFIG_SPL_NAND_BASE) += nand_base.o nand_amd.o nand_hynix.o \
				nand_macronix.o nand_micron.o \
				nand_samsung.o nand_toshiba.o
obj-$(CONFIG_SPL_NAND_IDENT) += nand_ids.o nand_timings.o
obj-$(CONFIG_$(SPL_TPL_)NAND_INIT) += nand.o
ifeq ($(CONFIG_SPL_ENV_SUPPORT),y)
obj-$(CONFIG_ENV_IS_IN_NAND) += nand_util.o
endif

else # not spl

NORMAL_DRIVERS=y

obj-y += nand.o
obj-y += nand_bbt.o
obj-y += nand_ids.o
obj-y += nand_util.o
obj-y += nand_ecc.o
obj-y += nand_base.o
obj-y += nand_amd.o
obj-y += nand_hynix.o
obj-y += nand_macronix.o
obj-y += nand_micron.o
obj-y += nand_samsung.o
obj-y += nand_toshiba.o
obj-y += nand_timings.o

endif # not spl

ifdef NORMAL_DRIVERS

obj-$(CONFIG_NAND_ECC_BCH) += nand_bch.o

obj-$(CONFIG_NAND_ATMEL) += atmel_nand.o
obj-$(CONFIG_DM_NAND_ATMEL) += atmel/
obj-$(CONFIG_NAND_ARASAN) += arasan_nfc.o
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand/
obj-$(CONFIG_NAND_DAVINCI) += davinci_nand.o
obj-$(CONFIG_NAND_DENALI) += denali.o
obj-$(CONFIG_NAND_DENALI_DT) += denali_dt.o
obj-$(CONFIG_NAND_FSL_ELBC) += fsl_elbc_nand.o
obj-$(CONFIG_NAND_FSL_IFC) += fsl_ifc_nand.o
obj-$(CONFIG_NAND_KIRKWOOD) += kirkwood_nand.o
obj-$(CONFIG_NAND_KMETER1) += kmeter1_nand.o
obj-$(CONFIG_NAND_LPC32XX_MLC) += lpc32xx_nand_mlc.o
obj-$(CONFIG_NAND_LPC32XX_SLC) += lpc32xx_nand_slc.o
obj-$(CONFIG_NAND_VF610_NFC) += vf610_nfc.o
obj-$(CONFIG_NAND_MXC) += mxc_nand.o
obj-$(CONFIG_NAND_MXS) += mxs_nand.o
obj-$(CONFIG_NAND_MXS_DT) += mxs_nand_dt.o
obj-$(CONFIG_NAND_OCTEONTX) += octeontx_nand.o
obj-$(CONFIG_NAND_OCTEONTX_HW_ECC) += octeontx_bch.o
obj-$(CONFIG_NAND_PXA3XX) += pxa3xx_nand.o
obj-$(CONFIG_TEGRA_NAND) += tegra_nand.o
obj-$(CONFIG_NAND_OMAP_GPMC) += omap_gpmc.o
obj-$(CONFIG_NAND_OMAP_ELM) += omap_elm.o
obj-$(CONFIG_NAND_SANDBOX) += sand_nand.o
obj-$(CONFIG_NAND_SUNXI) += sunxi_nand.o
obj-$(CONFIG_NAND_MXIC) += mxic_nand.o
obj-$(CONFIG_NAND_ZYNQ) += zynq_nand.o
obj-$(CONFIG_NAND_STM32_FMC2) += stm32_fmc2_nand.o
obj-$(CONFIG_CORTINA_NAND) += cortina_nand.o
obj-$(CONFIG_ROCKCHIP_NAND) += rockchip_nfc.o
obj-$(CONFIG_NAND_MT7621) += mt7621_nand.o

else  # minimal SPL drivers

obj-$(CONFIG_NAND_FSL_ELBC) += fsl_elbc_spl.o
obj-$(CONFIG_NAND_FSL_IFC) += fsl_ifc_spl.o
obj-$(CONFIG_NAND_MXC) += mxc_nand_spl.o
obj-$(CONFIG_NAND_MXS) += mxs_nand_spl.o mxs_nand.o
obj-$(CONFIG_NAND_SUNXI) += sunxi_nand_spl.o
obj-$(CONFIG_NAND_MT7621) += mt7621_nand_spl.o mt7621_nand.o
obj-$(CONFIG_NAND_BRCMNAND) += brcmnand/

endif # drivers
