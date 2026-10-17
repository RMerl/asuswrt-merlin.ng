// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 * (C) Copyright 2019  Synamedia
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 */

#include <common.h>
#include <dm.h>
#include <mach/sdhci.h>
#include <malloc.h>
#include <sdhci.h>

/*
 * The BCMSTB SDHCI has a quirk in that its actual maximum frequency
 * capability is 100 MHz.  The divisor that is eventually written to
 * SDHCI_CLOCK_CONTROL is calculated based on what the MMC device
 * reports, and relative to this maximum frequency.
 *
 * This define used to be set to 52000000 (52 MHz), the desired
 * maximum frequency, but that would result in the communication
 * actually running at 100 MHz (seemingly without issue), which is
 * out-of-spec.
 *
 * Now, by setting this to 0 (auto-detect), 100 MHz will be read from
 * the capabilities register, and the resulting divisor will be
 * doubled, meaning that the clock control register will be set to the
 * in-spec 52 MHz value.
 */
#define BCMSTB_SDHCI_MAXIMUM_CLOCK_FREQUENCY	0
/*
 * When the minimum clock frequency is set to 0 (auto-detect), U-Boot
 * sets it to 100 MHz divided by SDHCI_MAX_DIV_SPEC_300, or 48,875 Hz,
 * which results in the controller timing out when trying to
 * communicate with the MMC device.  Hard-code this value to 400000
 * (400 kHz) to prevent this.
 */
#define BCMSTB_SDHCI_MINIMUM_CLOCK_FREQUENCY	400000

#define SDIO_CFG_CTRL				0x0
#define  SDIO_CFG_CTRL_SDCD_N_TEST_EN		BIT(31)
#define  SDIO_CFG_CTRL_SDCD_N_TEST_LEV		BIT(30)

#define SDIO_CFG_SD_PIN_SEL			0x44
#define  SDIO_CFG_SD_PIN_SEL_MASK		0x3
#define  SDIO_CFG_SD_PIN_SEL_CARD		BIT(1)

struct sdhci_bcmstb_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct sdhci_brcmstb_dev_priv {
	int (*init)(struct udevice *dev);
};

static int sdhci_brcmstb_init_2712(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);
	void *cfg_regs;
	u32 reg;

	/* Map in the non-standard CFG registers */
	cfg_regs = dev_remap_addr_name(dev, "cfg");
	if (!cfg_regs)
		return -ENOENT;

	if ((host->mmc->host_caps & MMC_CAP_NONREMOVABLE) ||
	    (host->mmc->host_caps & MMC_CAP_NEEDS_POLL)) {
		/* Force presence */
		reg = readl(cfg_regs + SDIO_CFG_CTRL);
		reg &= ~SDIO_CFG_CTRL_SDCD_N_TEST_LEV;
		reg |= SDIO_CFG_CTRL_SDCD_N_TEST_EN;
		writel(reg, cfg_regs + SDIO_CFG_CTRL);
	} else {
		/* Enable card detection line */
		reg = readl(cfg_regs + SDIO_CFG_SD_PIN_SEL);
		reg &= ~SDIO_CFG_SD_PIN_SEL_MASK;
		reg |= SDIO_CFG_SD_PIN_SEL_CARD;
		writel(reg, cfg_regs + SDIO_CFG_SD_PIN_SEL);
	}

	return 0;
}

static int sdhci_bcmstb_bind(struct udevice *dev)
{
	struct sdhci_bcmstb_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

/* No specific SDHCI operations are required */
static const struct sdhci_ops bcmstb_sdhci_ops = { 0 };

static int sdhci_bcmstb_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_bcmstb_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	struct sdhci_brcmstb_dev_priv *dev_priv;
	fdt_addr_t base;
	int ret;

	dev_priv = (struct sdhci_brcmstb_dev_priv *)dev_get_driver_data(dev);

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	host->name = dev->name;
	host->ioaddr = (void *)base;

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->ops = &bcmstb_sdhci_ops;

	ret = sdhci_setup_cfg(&plat->cfg, host,
			      BCMSTB_SDHCI_MAXIMUM_CLOCK_FREQUENCY,
			      BCMSTB_SDHCI_MINIMUM_CLOCK_FREQUENCY);
	if (ret)
		return ret;

	upriv->mmc = &plat->mmc;
	host->mmc->priv = host;

	if (dev_priv && dev_priv->init) {
		ret = dev_priv->init(dev);
		if (ret)
			return ret;
	}

	return sdhci_probe(dev);
}

static const struct sdhci_brcmstb_dev_priv match_priv_2712 = {
	.init = sdhci_brcmstb_init_2712,
};

static const struct udevice_id sdhci_bcmstb_match[] = {
	{ .compatible = "brcm,bcm2712-sdhci", .data = (ulong)&match_priv_2712 },
	{ .compatible = "brcm,bcm7425-sdhci" },
	{ .compatible = "brcm,sdhci-brcmstb" },
	{ }
};

U_BOOT_DRIVER(sdhci_bcmstb) = {
	.name = "sdhci-bcmstb",
	.id = UCLASS_MMC,
	.of_match = sdhci_bcmstb_match,
	.ops = &sdhci_ops,
	.bind = sdhci_bcmstb_bind,
	.probe = sdhci_bcmstb_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct sdhci_bcmstb_plat),
};
