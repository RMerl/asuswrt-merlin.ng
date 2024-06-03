# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y += mmc.o
obj-$(CONFIG_$(SPL_)DM_MMC) += mmc-uclass.o
obj-$(CONFIG_$(SPL_)MMC_WRITE) += mmc_write.o

ifndef CONFIG_$(SPL_)BLK
obj-y += mmc_legacy.o
endif

obj-$(CONFIG_SUPPORT_EMMC_BOOT) += mmc_boot.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_MMC_BOOT) += fsl_esdhc_spl.o
endif

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_MMC_SDHCI_BCMBCA) += bcmbca_sdhci_spl.o
endif

obj-$(CONFIG_ARM_PL180_MMCI) += arm_pl180_mmci.o
obj-$(CONFIG_MMC_DAVINCI)		+= davinci_mmc.o
obj-$(CONFIG_MMC_DW)			+= dw_mmc.o
obj-$(CONFIG_MMC_DW_EXYNOS)		+= exynos_dw_mmc.o
obj-$(CONFIG_MMC_DW_K3)			+= hi6220_dw_mmc.o
obj-$(CONFIG_MMC_DW_ROCKCHIP)		+= rockchip_dw_mmc.o
obj-$(CONFIG_MMC_DW_SOCFPGA)		+= socfpga_dw_mmc.o
obj-$(CONFIG_MMC_DW_SNPS)		+= snps_dw_mmc.o
obj-$(CONFIG_FSL_ESDHC) += fsl_esdhc.o
obj-$(CONFIG_FTSDC010) += ftsdc010_mci.o
obj-$(CONFIG_GENERIC_ATMEL_MCI) += gen_atmel_mci.o
obj-$(CONFIG_MMC_MESON_GX) += meson_gx_mmc.o
obj-$(CONFIG_MMC_SPI) += mmc_spi.o
obj-$(CONFIG_MVEBU_MMC) += mvebu_mmc.o
obj-$(CONFIG_MMC_OMAP_HS)		+= omap_hsmmc.o
obj-$(CONFIG_MMC_MXC)			+= mxcmmc.o
obj-$(CONFIG_MMC_MXS)			+= mxsmmc.o
obj-$(CONFIG_MMC_PCI)			+= pci_mmc.o
obj-$(CONFIG_PXA_MMC_GENERIC) += pxa_mmc_gen.o
obj-$(CONFIG_SUPPORT_EMMC_RPMB) += rpmb.o
obj-$(CONFIG_MMC_SANDBOX)		+= sandbox_mmc.o
obj-$(CONFIG_SH_MMCIF) += sh_mmcif.o
obj-$(CONFIG_SH_SDHI) += sh_sdhi.o
obj-$(CONFIG_STM32_SDMMC2) += stm32_sdmmc2.o
obj-$(CONFIG_JZ47XX_MMC) += jz_mmc.o

# SDHCI
obj-$(CONFIG_MMC_SDHCI)			+= sdhci.o
obj-$(CONFIG_MMC_SDHCI_ATMEL)		+= atmel_sdhci.o
obj-$(CONFIG_MMC_SDHCI_BCM2835)		+= bcm2835_sdhci.o
obj-$(CONFIG_MMC_SDHCI_BCMSTB)		+= bcmstb_sdhci.o
obj-$(CONFIG_MMC_SDHCI_BCMBCA)		+= bcmbca_sdhci.o
obj-$(CONFIG_MMC_SDHCI_CADENCE)		+= sdhci-cadence.o
obj-$(CONFIG_MMC_SDHCI_K3_ARASAN)	+= k3_arsan_sdhci.o
obj-$(CONFIG_MMC_SDHCI_KONA)		+= kona_sdhci.o
obj-$(CONFIG_MMC_SDHCI_MSM)		+= msm_sdhci.o
obj-$(CONFIG_MMC_SDHCI_MV)		+= mv_sdhci.o
obj-$(CONFIG_MMC_SDHCI_PIC32)		+= pic32_sdhci.o
obj-$(CONFIG_MMC_SDHCI_ROCKCHIP)	+= rockchip_sdhci.o
obj-$(CONFIG_MMC_SDHCI_S5P)		+= s5p_sdhci.o
obj-$(CONFIG_MMC_SDHCI_SPEAR)		+= spear_sdhci.o
obj-$(CONFIG_MMC_SDHCI_STI) 		+= sti_sdhci.o
obj-$(CONFIG_MMC_SDHCI_TANGIER)		+= tangier_sdhci.o
obj-$(CONFIG_MMC_SDHCI_TEGRA)		+= tegra_mmc.o
obj-$(CONFIG_MMC_SDHCI_XENON)		+= xenon_sdhci.o
obj-$(CONFIG_MMC_SDHCI_ZYNQ)		+= zynq_sdhci.o

obj-$(CONFIG_MMC_SUNXI)			+= sunxi_mmc.o
obj-$(CONFIG_MMC_UNIPHIER)		+= tmio-common.o uniphier-sd.o
obj-$(CONFIG_RENESAS_SDHI)		+= tmio-common.o renesas-sdhi.o
obj-$(CONFIG_MMC_BCM2835)		+= bcm2835_sdhost.o
obj-$(CONFIG_MMC_MTK)			+= mtk-sd.o
