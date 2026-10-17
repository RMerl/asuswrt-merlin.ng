# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y += mmc.o
obj-$(CONFIG_$(SPL_TPL_)DM_MMC) += mmc-uclass.o

ifdef CONFIG_$(SPL_TPL_)DM_MMC
obj-$(CONFIG_$(SPL_TPL_)BOOTSTD) += mmc_bootdev.o
endif

obj-$(CONFIG_$(SPL_TPL_)MMC_WRITE) += mmc_write.o
obj-$(CONFIG_MMC_PWRSEQ) += mmc-pwrseq.o
obj-$(CONFIG_MMC_SDHCI_ADMA_HELPERS) += sdhci-adma.o

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
obj-$(CONFIG_MMC_DW_CORTINA)		+= ca_dw_mmc.o
obj-$(CONFIG_MMC_DW_EXYNOS)		+= exynos_dw_mmc.o
obj-$(CONFIG_MMC_DW_K3)			+= hi6220_dw_mmc.o
obj-$(CONFIG_MMC_DW_ROCKCHIP)		+= rockchip_dw_mmc.o
obj-$(CONFIG_MMC_DW_SOCFPGA)		+= socfpga_dw_mmc.o
obj-$(CONFIG_MMC_DW_SNPS)		+= snps_dw_mmc.o
obj-$(CONFIG_FSL_ESDHC) += fsl_esdhc.o
obj-$(CONFIG_FSL_ESDHC_IMX) += fsl_esdhc_imx.o
obj-$(CONFIG_FTSDC010) += ftsdc010_mci.o
obj-$(CONFIG_GENERIC_ATMEL_MCI) += gen_atmel_mci.o
obj-$(CONFIG_MMC_MESON_GX) += meson_gx_mmc.o
obj-$(CONFIG_MMC_SPI) += mmc_spi.o
obj-$(CONFIG_MVEBU_MMC) += mvebu_mmc.o
obj-$(CONFIG_MMC_OMAP_HS)		+= omap_hsmmc.o
obj-$(CONFIG_MMC_MXC)			+= mxcmmc.o
obj-$(CONFIG_MMC_MXS)			+= mxsmmc.o
obj-$(CONFIG_MMC_OCTEONTX)		+= octeontx_hsmmc.o
obj-$(CONFIG_MMC_OWL)			+= owl_mmc.o
obj-$(CONFIG_MMC_PCI)			+= pci_mmc.o
obj-$(CONFIG_SUPPORT_EMMC_RPMB) += rpmb.o
obj-$(CONFIG_MMC_SANDBOX)		+= sandbox_mmc.o
obj-$(CONFIG_SH_MMCIF) += sh_mmcif.o
obj-$(CONFIG_STM32_SDMMC2) += stm32_sdmmc2.o
obj-$(CONFIG_JZ47XX_MMC) += jz_mmc.o
obj-$(CONFIG_NEXELL_DWMMC) += nexell_dw_mmc.o

# SDHCI
obj-$(CONFIG_MMC_SDHCI)			+= sdhci.o
obj-$(CONFIG_MMC_SDHCI_ASPEED)		+= aspeed_sdhci.o
obj-$(CONFIG_MMC_SDHCI_ATMEL)		+= atmel_sdhci.o
obj-$(CONFIG_MMC_SDHCI_BCM2835)		+= bcm2835_sdhci.o
obj-$(CONFIG_MMC_SDHCI_BCMSTB)		+= bcmstb_sdhci.o
obj-$(CONFIG_MMC_SDHCI_BCMBCA)		+= bcmbca_sdhci.o
obj-$(CONFIG_MMC_SDHCI_CADENCE)		+= sdhci-cadence.o
obj-$(CONFIG_MMC_SDHCI_AM654)		+= am654_sdhci.o
obj-$(CONFIG_MMC_SDHCI_IPROC)		+= iproc_sdhci.o
obj-$(CONFIG_MMC_SDHCI_KONA)		+= kona_sdhci.o
obj-$(CONFIG_MMC_SDHCI_MSM)		+= msm_sdhci.o
obj-$(CONFIG_MMC_SDHCI_MV)		+= mv_sdhci.o
obj-$(CONFIG_MMC_SDHCI_NPCM)            += npcm_sdhci.o
obj-$(CONFIG_MMC_SDHCI_PIC32)		+= pic32_sdhci.o
obj-$(CONFIG_MMC_SDHCI_ROCKCHIP)	+= rockchip_sdhci.o
obj-$(CONFIG_MMC_SDHCI_S5P)		+= s5p_sdhci.o
obj-$(CONFIG_MMC_SDHCI_STI)		+= sti_sdhci.o
obj-$(CONFIG_MMC_SDHCI_TANGIER)		+= tangier_sdhci.o
obj-$(CONFIG_MMC_SDHCI_TEGRA)		+= tegra_mmc.o
obj-$(CONFIG_MMC_SDHCI_XENON)		+= xenon_sdhci.o
obj-$(CONFIG_MMC_SDHCI_ZYNQ)		+= zynq_sdhci.o

obj-$(CONFIG_MMC_SUNXI)			+= sunxi_mmc.o
obj-$(CONFIG_MMC_PITON)			+= piton_mmc.o
obj-$(CONFIG_MMC_UNIPHIER)		+= tmio-common.o uniphier-sd.o
obj-$(CONFIG_RENESAS_SDHI)		+= tmio-common.o renesas-sdhi.o
obj-$(CONFIG_MMC_BCM2835)		+= bcm2835_sdhost.o
obj-$(CONFIG_MMC_MTK)			+= mtk-sd.o
obj-$(CONFIG_MMC_SDHCI_F_SDH30)		+= f_sdh30.o
