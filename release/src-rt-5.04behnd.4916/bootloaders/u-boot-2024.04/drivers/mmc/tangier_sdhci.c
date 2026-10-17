// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 */
#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <mmc.h>
#include <sdhci.h>

#define SDHCI_TANGIER_FMAX	200000000
#define SDHCI_TANGIER_FMIN	400000

struct sdhci_tangier_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	void __iomem *ioaddr;
};

static int sdhci_tangier_bind(struct udevice *dev)
{
	struct sdhci_tangier_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static int sdhci_tangier_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_tangier_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	fdt_addr_t base;
	int ret;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->ioaddr = devm_ioremap(dev, base, SZ_1K);
	if (!plat->ioaddr)
		return -ENOMEM;

	host->name = dev->name;
	host->ioaddr = plat->ioaddr;
	host->quirks = SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_BROKEN_VOLTAGE |
		       SDHCI_QUIRK_32BIT_DMA_ADDR | SDHCI_QUIRK_WAIT_SEND_CMD;

	/* MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195 */
	host->voltages = MMC_VDD_165_195;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	ret = sdhci_setup_cfg(&plat->cfg, host, SDHCI_TANGIER_FMAX,
			SDHCI_TANGIER_FMIN);
	if (ret)
		return ret;

	upriv->mmc = &plat->mmc;
	host->mmc->priv = host;

	return sdhci_probe(dev);
}

static const struct udevice_id sdhci_tangier_match[] = {
	{ .compatible = "intel,sdhci-tangier" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sdhci_tangier) = {
	.name		= "sdhci-tangier",
	.id		= UCLASS_MMC,
	.of_match	= sdhci_tangier_match,
	.bind		= sdhci_tangier_bind,
	.probe		= sdhci_tangier_probe,
	.ops		= &sdhci_ops,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct sdhci_tangier_plat),
};
