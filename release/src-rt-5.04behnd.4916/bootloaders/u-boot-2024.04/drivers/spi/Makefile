# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

# There are many options which enable SPI, so make this library available
ifdef CONFIG_$(SPL_TPL_)DM_SPI
obj-y += spi-uclass.o
obj-$(CONFIG_CADENCE_QSPI) += cadence_qspi.o cadence_qspi_apb.o
obj-$(CONFIG_CADENCE_OSPI_VERSAL) += cadence_ospi_versal.o
obj-$(CONFIG_SANDBOX) += spi-emul-uclass.o
obj-$(CONFIG_SOFT_SPI) += soft_spi.o
obj-$(CONFIG_SPI_ASPEED_SMC) += spi-aspeed-smc.o
obj-$(CONFIG_SPI_MEM) += spi-mem.o
obj-$(CONFIG_TI_QSPI) += ti_qspi.o
obj-$(CONFIG_FSL_QSPI) += fsl_qspi.o
else
obj-y += spi.o
obj-$(CONFIG_SPI_MEM) += spi-mem-nodm.o
endif

obj-$(CONFIG_ALTERA_SPI) += altera_spi.o
obj-$(CONFIG_APPLE_SPI) += apple_spi.o
obj-$(CONFIG_ATH79_SPI) += ath79_spi.o
obj-$(CONFIG_ATMEL_QSPI) += atmel-quadspi.o
obj-$(CONFIG_ATMEL_SPI) += atmel_spi.o
obj-$(CONFIG_BCM63XX_HSSPI) += bcm63xx_hsspi.o
obj-$(CONFIG_BCMBCA_HSSPI) += bcmbca_hsspi.o
obj-$(CONFIG_BCM63XX_SPI) += bcm63xx_spi.o
obj-$(CONFIG_BCMSTB_SPI) += bcmstb_spi.o
obj-$(CONFIG_CF_SPI) += cf_spi.o
obj-$(CONFIG_CORTINA_SFLASH) += ca_sflash.o
obj-$(CONFIG_DAVINCI_SPI) += davinci_spi.o
obj-$(CONFIG_DESIGNWARE_SPI) += designware_spi.o
obj-$(CONFIG_EXYNOS_SPI) += exynos_spi.o
obj-$(CONFIG_FSL_DSPI) += fsl_dspi.o
obj-$(CONFIG_FSL_ESPI) += fsl_espi.o
obj-$(CONFIG_SYNQUACER_SPI) += spi-synquacer.o
obj-$(CONFIG_GXP_SPI) += gxp_spi.o
obj-$(CONFIG_ICH_SPI) +=  ich.o
obj-$(CONFIG_IPROC_QSPI) += iproc_qspi.o
obj-$(CONFIG_KIRKWOOD_SPI) += kirkwood_spi.o
obj-$(CONFIG_MESON_SPIFC) += meson_spifc.o
obj-$(CONFIG_MESON_SPIFC_A1) += meson_spifc_a1.o
obj-$(CONFIG_MICROCHIP_COREQSPI) += microchip_coreqspi.o
obj-$(CONFIG_MPC8XX_SPI) += mpc8xx_spi.o
obj-$(CONFIG_MPC8XXX_SPI) += mpc8xxx_spi.o
obj-$(CONFIG_MTK_SNFI_SPI) += mtk_snfi_spi.o
obj-$(CONFIG_MTK_SNOR) += mtk_snor.o
obj-$(CONFIG_MTK_SPIM) += mtk_spim.o
obj-$(CONFIG_MT7620_SPI) += mt7620_spi.o
obj-$(CONFIG_MT7621_SPI) += mt7621_spi.o
obj-$(CONFIG_MSCC_BB_SPI) += mscc_bb_spi.o
obj-$(CONFIG_MVEBU_A3700_SPI) += mvebu_a3700_spi.o
obj-$(CONFIG_MXC_SPI) += mxc_spi.o
obj-$(CONFIG_MXS_SPI) += mxs_spi.o
obj-$(CONFIG_NPCM_FIU_SPI) += npcm_fiu_spi.o
obj-$(CONFIG_NPCM_PSPI) += npcm_pspi.o
obj-$(CONFIG_NXP_FSPI) += nxp_fspi.o
obj-$(CONFIG_ATCSPI200_SPI) += atcspi200_spi.o
obj-$(CONFIG_OCTEON_SPI) += octeon_spi.o
obj-$(CONFIG_OMAP3_SPI) += omap3_spi.o
obj-$(CONFIG_PIC32_SPI) += pic32_spi.o
obj-$(CONFIG_PL022_SPI) += pl022_spi.o
obj-$(CONFIG_SPI_QUP) += spi-qup.o
obj-$(CONFIG_SPI_MXIC) += spi-mxic.o
obj-$(CONFIG_RENESAS_RPC_SPI) += renesas_rpc_spi.o
obj-$(CONFIG_ROCKCHIP_SFC) += rockchip_sfc.o
obj-$(CONFIG_ROCKCHIP_SPI) += rk_spi.o
obj-$(CONFIG_SANDBOX_SPI) += sandbox_spi.o
obj-$(CONFIG_SPI_SIFIVE) += spi-sifive.o
obj-$(CONFIG_SPI_SN_F_OSPI) += spi-sn-f-ospi.o
obj-$(CONFIG_SPI_SUNXI) += spi-sunxi.o
obj-$(CONFIG_SH_QSPI) += sh_qspi.o
obj-$(CONFIG_STM32_QSPI) += stm32_qspi.o
obj-$(CONFIG_STM32_SPI) += stm32_spi.o
obj-$(CONFIG_TEGRA114_SPI) += tegra114_spi.o
obj-$(CONFIG_TEGRA20_SFLASH) += tegra20_sflash.o
obj-$(CONFIG_TEGRA20_SLINK) += tegra20_slink.o
obj-$(CONFIG_TEGRA210_QSPI) += tegra210_qspi.o
obj-$(CONFIG_UNIPHIER_SPI) += uniphier_spi.o
obj-$(CONFIG_XILINX_SPI) += xilinx_spi.o
obj-$(CONFIG_ZYNQ_SPI) += zynq_spi.o
obj-$(CONFIG_ZYNQ_QSPI) += zynq_qspi.o
obj-$(CONFIG_ZYNQMP_GQSPI) += zynqmp_gqspi.o
