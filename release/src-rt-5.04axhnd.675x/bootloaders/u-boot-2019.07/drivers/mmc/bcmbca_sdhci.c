// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 * (C) Copyright 2019  Synamedia
 * (C) Copyright 2019  Broadcom Ltd
 *
 * Author: Farhan Ali <farhan.ali@broadcom.com>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>

/* 400KHz is max freq for card ID etc. Use that as min */
#define MIN_FREQ 400000
#define BCMBCA_MMC_BOOT_MAIN_CTL	0
#define BCMBCA_MMC_BOOT_STATUS		0x4
#define BCMBCA_MMC_BOOT_MODE_MASK	1

struct sdhci_bcmbca_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int sdhci_bcmbca_bind(struct udevice *dev)
{
	struct sdhci_bcmbca_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static int sdhci_bcmbca_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_bcmbca_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	struct resource res;
	void * boot_regs;
	int ret;

	host->name = dev->name;

	/* Get sdhci controller base address */
	ret = dev_read_resource_byname(dev, "sdhci-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs sdhci-base address(ret = %d)!\n", ret);
		return ret;
	}
							
	host->quirks |= SDHCI_QUIRK_WAIT_SEND_CMD;
	host->ioaddr = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(host->ioaddr))
		return PTR_ERR(host->ioaddr);

	/* Get sdhci boot controller base address */
	ret = dev_read_resource_byname(dev, "sdhci-boot", &res);
	if (ret) {
		dev_err(dev, "can't get regs sdhci-boot address(ret = %d)!\n", ret);
		return ret;
	}

	boot_regs = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(boot_regs))
		return PTR_ERR(boot_regs);

	/* Get us out of boot mode */
	while(readl(boot_regs + BCMBCA_MMC_BOOT_STATUS) & BCMBCA_MMC_BOOT_MODE_MASK)
	{
		writel(0, boot_regs + BCMBCA_MMC_BOOT_MAIN_CTL);
	}

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	/* Use default max frequency from caps register */
	ret = sdhci_setup_cfg(&plat->cfg, host,
			      0,
			      MIN_FREQ);
	if (ret)
		return ret;

	upriv->mmc = &plat->mmc;
	host->mmc = &plat->mmc;
	host->mmc->priv = host;

	return sdhci_probe(dev);
}

static const struct udevice_id sdhci_bcmbca_match[] = {
	{ .compatible = "brcm,bcm63xx-sdhci" },
	{ .compatible = "brcm,sdhci-brcmbca" },
	{ }
};

U_BOOT_DRIVER(sdhci_bcmbca) = {
	.name = "sdhci-bcmbca",
	.id = UCLASS_MMC,
	.of_match = sdhci_bcmbca_match,
	.ops = &sdhci_ops,
	.bind = sdhci_bcmbca_bind,
	.probe = sdhci_bcmbca_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct sdhci_bcmbca_plat),
};
