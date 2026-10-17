// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 IBM Corp.
 * Eddie James <eajames@linux.ibm.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/err.h>
#include <dm/lists.h>

struct aspeed_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int aspeed_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct aspeed_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	u32 max_clk;
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		debug("%s: clock get failed %d\n", __func__, ret);
		return ret;
	}

	ret = clk_enable(&clk);
	if (ret) {
		debug("%s: clock enable failed %d\n", __func__, ret);
		return ret;
	}

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);

	max_clk = clk_get_rate(&clk);
	if (IS_ERR_VALUE(max_clk)) {
		ret = max_clk;
		debug("%s: clock rate get failed %d\n", __func__, ret);
		goto err;
	}

	host->max_clk = max_clk;
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;
	upriv->mmc = host->mmc;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		goto err;

	ret = sdhci_probe(dev);
	if (ret)
		goto err;

	return 0;

err:
	clk_disable(&clk);
	return ret;
}

static int aspeed_sdhci_bind(struct udevice *dev)
{
	struct aspeed_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id aspeed_sdhci_ids[] = {
	{ .compatible = "aspeed,ast2400-sdhci" },
	{ .compatible = "aspeed,ast2500-sdhci" },
	{ .compatible = "aspeed,ast2600-sdhci" },
	{ }
};

U_BOOT_DRIVER(aspeed_sdhci_drv) = {
	.name		= "aspeed_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= aspeed_sdhci_ids,
	.ops		= &sdhci_ops,
	.bind		= aspeed_sdhci_bind,
	.probe		= aspeed_sdhci_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct aspeed_sdhci_plat),
};


static int aspeed_sdc_probe(struct udevice *parent)
{
	struct clk clk;
	int ret;

	ret = clk_get_by_index(parent, 0, &clk);
	if (ret) {
		debug("%s: clock get failed %d\n", __func__, ret);
		return ret;
	}

	ret = clk_enable(&clk);
	if (ret) {
		debug("%s: clock enable failed %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id aspeed_sdc_ids[] = {
	{ .compatible = "aspeed,ast2400-sd-controller" },
	{ .compatible = "aspeed,ast2500-sd-controller" },
	{ .compatible = "aspeed,ast2600-sd-controller" },
	{ }
};

U_BOOT_DRIVER(aspeed_sdc_drv) = {
	.name		= "aspeed_sdc",
	.id		= UCLASS_MISC,
	.of_match	= aspeed_sdc_ids,
	.probe		= aspeed_sdc_probe,
};
