// SPDX-License-Identifier: GPL-2.0+
/*
 * Support of SDHCI for Microchip PIC32 SoC.
 *
 * Copyright (C) 2015 Microchip Technology Inc.
 * Andrei Pistirica <andrei.pistirica@microchip.com>
 */

#include <dm.h>
#include <sdhci.h>
#include <clk.h>
#include <linux/errno.h>
#include <mach/pic32.h>

struct pic32_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int pic32_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct pic32_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);

	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_name(dev, "base_clk", &clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&clk);

	if (IS_ERR_VALUE(clk_rate))
		return clk_rate;

	host->ioaddr = dev_remap_addr(dev);

	if (!host->ioaddr)
		return -EINVAL;

	host->name	= dev->name;
	host->quirks	= SDHCI_QUIRK_NO_HISPD_BIT;
	host->bus_width	= dev_read_u32_default(dev, "bus-width", 4);
	host->max_clk   = clk_rate;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;

	host->mmc->priv = host;
	upriv->mmc = host->mmc;

	ret = sdhci_probe(dev);
	if (ret)
		return ret;

	if (!dev_read_bool(dev, "microchip,use-sdcd")) {
		// Use workaround 1 for erratum #15 by default
		u8 ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
		ctrl = (ctrl & ~SDHCI_CTRL_CD_TEST_INS) | SDHCI_CTRL_CD_TEST;
		sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
	}

	return 0;
}

static int pic32_sdhci_bind(struct udevice *dev)
{
	struct pic32_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id pic32_sdhci_ids[] = {
	{ .compatible = "microchip,pic32mzda-sdhci" },
	{ }
};

U_BOOT_DRIVER(pic32_sdhci_drv) = {
	.name			= "pic32_sdhci",
	.id			= UCLASS_MMC,
	.of_match		= pic32_sdhci_ids,
	.ops			= &sdhci_ops,
	.bind			= pic32_sdhci_bind,
	.probe			= pic32_sdhci_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct pic32_sdhci_plat)
};
