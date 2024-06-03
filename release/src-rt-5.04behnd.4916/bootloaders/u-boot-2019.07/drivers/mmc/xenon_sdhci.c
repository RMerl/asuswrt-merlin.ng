// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Marvell SOC Platform Group Xenon SDHC as a platform device
 *
 * Copyright (C) 2016 Marvell, All Rights Reserved.
 *
 * Author:	Victor Gu <xigu@marvell.com>
 * Date:	2016-8-24
 *
 * Included parts of the Linux driver version which was written by:
 * Hu Ziji <huziji@marvell.com>
 *
 * Ported to from Marvell 2015.01 to mainline U-Boot 2017.01:
 * Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <sdhci.h>

DECLARE_GLOBAL_DATA_PTR;

/* Register Offset of SD Host Controller SOCP self-defined register */
#define SDHC_SYS_CFG_INFO			0x0104
#define SLOT_TYPE_SDIO_SHIFT			24
#define SLOT_TYPE_EMMC_MASK			0xFF
#define SLOT_TYPE_EMMC_SHIFT			16
#define SLOT_TYPE_SD_SDIO_MMC_MASK		0xFF
#define SLOT_TYPE_SD_SDIO_MMC_SHIFT		8
#define NR_SUPPORTED_SLOT_MASK			0x7

#define SDHC_SYS_OP_CTRL			0x0108
#define AUTO_CLKGATE_DISABLE_MASK		BIT(20)
#define SDCLK_IDLEOFF_ENABLE_SHIFT		8
#define SLOT_ENABLE_SHIFT			0

#define SDHC_SYS_EXT_OP_CTRL			0x010C
#define MASK_CMD_CONFLICT_ERROR			BIT(8)

#define SDHC_SLOT_RETUNING_REQ_CTRL		0x0144
/* retuning compatible */
#define RETUNING_COMPATIBLE			0x1

/* Xenon specific Mode Select value */
#define XENON_SDHCI_CTRL_HS200			0x5
#define XENON_SDHCI_CTRL_HS400			0x6

#define EMMC_PHY_REG_BASE			0x170
#define EMMC_PHY_TIMING_ADJUST			EMMC_PHY_REG_BASE
#define OUTPUT_QSN_PHASE_SELECT			BIT(17)
#define SAMPL_INV_QSP_PHASE_SELECT		BIT(18)
#define SAMPL_INV_QSP_PHASE_SELECT_SHIFT	18
#define EMMC_PHY_SLOW_MODE			BIT(29)
#define PHY_INITIALIZAION			BIT(31)
#define WAIT_CYCLE_BEFORE_USING_MASK		0xf
#define WAIT_CYCLE_BEFORE_USING_SHIFT		12
#define FC_SYNC_EN_DURATION_MASK		0xf
#define FC_SYNC_EN_DURATION_SHIFT		8
#define FC_SYNC_RST_EN_DURATION_MASK		0xf
#define FC_SYNC_RST_EN_DURATION_SHIFT		4
#define FC_SYNC_RST_DURATION_MASK		0xf
#define FC_SYNC_RST_DURATION_SHIFT		0

#define EMMC_PHY_FUNC_CONTROL			(EMMC_PHY_REG_BASE + 0x4)
#define DQ_ASYNC_MODE				BIT(4)
#define DQ_DDR_MODE_SHIFT			8
#define DQ_DDR_MODE_MASK			0xff
#define CMD_DDR_MODE				BIT(16)

#define EMMC_PHY_PAD_CONTROL			(EMMC_PHY_REG_BASE + 0x8)
#define REC_EN_SHIFT				24
#define REC_EN_MASK				0xf
#define FC_DQ_RECEN				BIT(24)
#define FC_CMD_RECEN				BIT(25)
#define FC_QSP_RECEN				BIT(26)
#define FC_QSN_RECEN				BIT(27)
#define OEN_QSN					BIT(28)
#define AUTO_RECEN_CTRL				BIT(30)

#define EMMC_PHY_PAD_CONTROL1			(EMMC_PHY_REG_BASE + 0xc)
#define EMMC5_1_FC_QSP_PD			BIT(9)
#define EMMC5_1_FC_QSP_PU			BIT(25)
#define EMMC5_1_FC_CMD_PD			BIT(8)
#define EMMC5_1_FC_CMD_PU			BIT(24)
#define EMMC5_1_FC_DQ_PD			0xff
#define EMMC5_1_FC_DQ_PU			(0xff << 16)

#define SDHCI_RETUNE_EVT_INTSIG			0x00001000

/* Hyperion only have one slot 0 */
#define XENON_MMC_SLOT_ID_HYPERION		0

#define MMC_TIMING_LEGACY	0
#define MMC_TIMING_MMC_HS	1
#define MMC_TIMING_SD_HS	2
#define MMC_TIMING_UHS_SDR12	3
#define MMC_TIMING_UHS_SDR25	4
#define MMC_TIMING_UHS_SDR50	5
#define MMC_TIMING_UHS_SDR104	6
#define MMC_TIMING_UHS_DDR50	7
#define MMC_TIMING_MMC_DDR52	8
#define MMC_TIMING_MMC_HS200	9
#define MMC_TIMING_MMC_HS400	10

#define XENON_MMC_MAX_CLK	400000000

enum soc_pad_ctrl_type {
	SOC_PAD_SD,
	SOC_PAD_FIXED_1_8V,
};

struct xenon_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct xenon_sdhci_priv {
	struct sdhci_host host;

	u8 timing;

	unsigned int clock;

	void *pad_ctrl_reg;
	int pad_type;
};

static int xenon_mmc_phy_init(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	u32 clock = priv->clock;
	u32 time;
	u32 var;

	/* Enable QSP PHASE SELECT */
	var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
	var |= SAMPL_INV_QSP_PHASE_SELECT;
	if ((priv->timing == MMC_TIMING_UHS_SDR50) ||
	    (priv->timing == MMC_TIMING_UHS_SDR25) ||
	    (priv->timing == MMC_TIMING_UHS_SDR12) ||
	    (priv->timing == MMC_TIMING_SD_HS) ||
	    (priv->timing == MMC_TIMING_LEGACY))
		var |= EMMC_PHY_SLOW_MODE;
	sdhci_writel(host, var, EMMC_PHY_TIMING_ADJUST);

	/* Poll for host MMC PHY clock init to be stable */
	/* Wait up to 10ms */
	time = 100;
	while (time--) {
		var = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
		if (var & SDHCI_CLOCK_INT_STABLE)
			break;

		udelay(100);
	}

	if (time <= 0) {
		pr_err("Failed to enable MMC internal clock in time\n");
		return -ETIMEDOUT;
	}

	/* Init PHY */
	var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
	var |= PHY_INITIALIZAION;
	sdhci_writel(host, var, EMMC_PHY_TIMING_ADJUST);

	if (clock == 0) {
		/* Use the possibly slowest bus frequency value */
		clock = 100000;
	}

	/* Poll for host eMMC PHY init to complete */
	/* Wait up to 10ms */
	time = 100;
	while (time--) {
		var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
		var &= PHY_INITIALIZAION;
		if (!var)
			break;

		/* wait for host eMMC PHY init to complete */
		udelay(100);
	}

	if (time <= 0) {
		pr_err("Failed to init MMC PHY in time\n");
		return -ETIMEDOUT;
	}

	return 0;
}

#define ARMADA_3700_SOC_PAD_1_8V	0x1
#define ARMADA_3700_SOC_PAD_3_3V	0x0

static void armada_3700_soc_pad_voltage_set(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;

	if (priv->pad_type == SOC_PAD_FIXED_1_8V)
		writel(ARMADA_3700_SOC_PAD_1_8V, priv->pad_ctrl_reg);
	else if (priv->pad_type == SOC_PAD_SD)
		writel(ARMADA_3700_SOC_PAD_3_3V, priv->pad_ctrl_reg);
}

static void xenon_mmc_phy_set(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	u32 var;

	/* Setup pad, set bit[30], bit[28] and bits[26:24] */
	var = sdhci_readl(host, EMMC_PHY_PAD_CONTROL);
	var |= AUTO_RECEN_CTRL | OEN_QSN | FC_QSP_RECEN |
		FC_CMD_RECEN | FC_DQ_RECEN;
	sdhci_writel(host, var, EMMC_PHY_PAD_CONTROL);

	/* Set CMD and DQ Pull Up */
	var = sdhci_readl(host, EMMC_PHY_PAD_CONTROL1);
	var |= (EMMC5_1_FC_CMD_PU | EMMC5_1_FC_DQ_PU);
	var &= ~(EMMC5_1_FC_CMD_PD | EMMC5_1_FC_DQ_PD);
	sdhci_writel(host, var, EMMC_PHY_PAD_CONTROL1);

	/*
	 * If timing belongs to high speed, set bit[17] of
	 * EMMC_PHY_TIMING_ADJUST register
	 */
	if ((priv->timing == MMC_TIMING_MMC_HS400) ||
	    (priv->timing == MMC_TIMING_MMC_HS200) ||
	    (priv->timing == MMC_TIMING_UHS_SDR50) ||
	    (priv->timing == MMC_TIMING_UHS_SDR104) ||
	    (priv->timing == MMC_TIMING_UHS_DDR50) ||
	    (priv->timing == MMC_TIMING_UHS_SDR25) ||
	    (priv->timing == MMC_TIMING_MMC_DDR52)) {
		var = sdhci_readl(host, EMMC_PHY_TIMING_ADJUST);
		var |= OUTPUT_QSN_PHASE_SELECT;
		sdhci_writel(host, var, EMMC_PHY_TIMING_ADJUST);
	}

	/*
	 * When setting EMMC_PHY_FUNC_CONTROL register,
	 * SD clock should be disabled
	 */
	var = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	var &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, var, SDHCI_CLOCK_CONTROL);

	var = sdhci_readl(host, EMMC_PHY_FUNC_CONTROL);
	if (host->mmc->ddr_mode) {
		var |= (DQ_DDR_MODE_MASK << DQ_DDR_MODE_SHIFT) | CMD_DDR_MODE;
	} else {
		var &= ~((DQ_DDR_MODE_MASK << DQ_DDR_MODE_SHIFT) |
			 CMD_DDR_MODE);
	}
	sdhci_writel(host, var, EMMC_PHY_FUNC_CONTROL);

	/* Enable bus clock */
	var = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	var |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, var, SDHCI_CLOCK_CONTROL);

	xenon_mmc_phy_init(host);
}

/* Enable/Disable the Auto Clock Gating function of this slot */
static void xenon_mmc_set_acg(struct sdhci_host *host, bool enable)
{
	u32 var;

	var = sdhci_readl(host, SDHC_SYS_OP_CTRL);
	if (enable)
		var &= ~AUTO_CLKGATE_DISABLE_MASK;
	else
		var |= AUTO_CLKGATE_DISABLE_MASK;

	sdhci_writel(host, var, SDHC_SYS_OP_CTRL);
}

#define SLOT_MASK(slot)		BIT(slot)

/* Enable specific slot */
static void xenon_mmc_enable_slot(struct sdhci_host *host, u8 slot)
{
	u32 var;

	var = sdhci_readl(host, SDHC_SYS_OP_CTRL);
	var |= SLOT_MASK(slot) << SLOT_ENABLE_SHIFT;
	sdhci_writel(host, var, SDHC_SYS_OP_CTRL);
}

/* Enable Parallel Transfer Mode */
static void xenon_mmc_enable_parallel_tran(struct sdhci_host *host, u8 slot)
{
	u32 var;

	var = sdhci_readl(host, SDHC_SYS_EXT_OP_CTRL);
	var |= SLOT_MASK(slot);
	sdhci_writel(host, var, SDHC_SYS_EXT_OP_CTRL);
}

static void xenon_mmc_disable_tuning(struct sdhci_host *host, u8 slot)
{
	u32 var;

	/* Clear the Re-Tuning Request functionality */
	var = sdhci_readl(host, SDHC_SLOT_RETUNING_REQ_CTRL);
	var &= ~RETUNING_COMPATIBLE;
	sdhci_writel(host, var, SDHC_SLOT_RETUNING_REQ_CTRL);

	/* Clear the Re-tuning Event Signal Enable */
	var = sdhci_readl(host, SDHCI_SIGNAL_ENABLE);
	var &= ~SDHCI_RETUNE_EVT_INTSIG;
	sdhci_writel(host, var, SDHCI_SIGNAL_ENABLE);
}

/* Mask command conflict error */
static void xenon_mask_cmd_conflict_err(struct sdhci_host *host)
{
	u32  reg;

	reg = sdhci_readl(host, SDHC_SYS_EXT_OP_CTRL);
	reg |= MASK_CMD_CONFLICT_ERROR;
	sdhci_writel(host, reg, SDHC_SYS_EXT_OP_CTRL);
}

/* Platform specific function for post set_ios configuration */
static void xenon_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct xenon_sdhci_priv *priv = host->mmc->priv;
	uint speed = host->mmc->tran_speed;
	int pwr_18v = 0;

	if ((sdhci_readb(host, SDHCI_POWER_CONTROL) & ~SDHCI_POWER_ON) ==
	    SDHCI_POWER_180)
		pwr_18v = 1;

	/* Set timing variable according to the configured speed */
	if (IS_SD(host->mmc)) {
		/* SD/SDIO */
		if (pwr_18v) {
			if (host->mmc->ddr_mode)
				priv->timing = MMC_TIMING_UHS_DDR50;
			else if (speed <= 25000000)
				priv->timing = MMC_TIMING_UHS_SDR25;
			else
				priv->timing = MMC_TIMING_UHS_SDR50;
		} else {
			if (speed <= 25000000)
				priv->timing = MMC_TIMING_LEGACY;
			else
				priv->timing = MMC_TIMING_SD_HS;
		}
	} else {
		/* eMMC */
		if (host->mmc->ddr_mode)
			priv->timing = MMC_TIMING_MMC_DDR52;
		else if (speed <= 26000000)
			priv->timing = MMC_TIMING_LEGACY;
		else
			priv->timing = MMC_TIMING_MMC_HS;
	}

	/* Re-init the PHY */
	xenon_mmc_phy_set(host);
}

/* Install a driver specific handler for post set_ios configuration */
static const struct sdhci_ops xenon_sdhci_ops = {
	.set_ios_post = xenon_sdhci_set_ios_post
};

static int xenon_sdhci_probe(struct udevice *dev)
{
	struct xenon_sdhci_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct xenon_sdhci_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	/* Set quirks */
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_32BIT_DMA_ADDR;

	/* Set default timing */
	priv->timing = MMC_TIMING_LEGACY;

	/* Disable auto clock gating during init */
	xenon_mmc_set_acg(host, false);

	/* Enable slot */
	xenon_mmc_enable_slot(host, XENON_MMC_SLOT_ID_HYPERION);

	/*
	 * Set default power on SoC PHY PAD register (currently only
	 * available on the Armada 3700)
	 */
	if (priv->pad_ctrl_reg)
		armada_3700_soc_pad_voltage_set(host);

	host->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_DDR_52MHz;
	switch (fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "bus-width",
		1)) {
	case 8:
		host->host_caps |= MMC_MODE_8BIT;
		break;
	case 4:
		host->host_caps |= MMC_MODE_4BIT;
		break;
	case 1:
		break;
	default:
		printf("Invalid \"bus-width\" value\n");
		return -EINVAL;
	}

	host->ops = &xenon_sdhci_ops;

	host->max_clk = XENON_MMC_MAX_CLK;
	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;

	ret = sdhci_probe(dev);
	if (ret)
		return ret;

	/* Enable parallel transfer */
	xenon_mmc_enable_parallel_tran(host, XENON_MMC_SLOT_ID_HYPERION);

	/* Disable tuning functionality of this slot */
	xenon_mmc_disable_tuning(host, XENON_MMC_SLOT_ID_HYPERION);

	/* Enable auto clock gating after init */
	xenon_mmc_set_acg(host, true);

	xenon_mask_cmd_conflict_err(host);

	return ret;
}

static int xenon_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);
	struct xenon_sdhci_priv *priv = dev_get_priv(dev);
	const char *name;

	host->name = dev->name;
	host->ioaddr = (void *)devfdt_get_addr(dev);

	if (device_is_compatible(dev, "marvell,armada-3700-sdhci"))
		priv->pad_ctrl_reg = (void *)devfdt_get_addr_index(dev, 1);

	name = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "marvell,pad-type",
			   NULL);
	if (name) {
		if (0 == strncmp(name, "sd", 2)) {
			priv->pad_type = SOC_PAD_SD;
		} else if (0 == strncmp(name, "fixed-1-8v", 10)) {
			priv->pad_type = SOC_PAD_FIXED_1_8V;
		} else {
			printf("Unsupported SOC PHY PAD ctrl type %s\n", name);
			return -EINVAL;
		}
	}

	return 0;
}

static int xenon_sdhci_bind(struct udevice *dev)
{
	struct xenon_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id xenon_sdhci_ids[] = {
	{ .compatible = "marvell,armada-8k-sdhci",},
	{ .compatible = "marvell,armada-3700-sdhci",},
	{ }
};

U_BOOT_DRIVER(xenon_sdhci_drv) = {
	.name		= "xenon_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= xenon_sdhci_ids,
	.ofdata_to_platdata = xenon_sdhci_ofdata_to_platdata,
	.ops		= &sdhci_ops,
	.bind		= xenon_sdhci_bind,
	.probe		= xenon_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct xenon_sdhci_priv),
	.platdata_auto_alloc_size = sizeof(struct xenon_sdhci_plat),
};
