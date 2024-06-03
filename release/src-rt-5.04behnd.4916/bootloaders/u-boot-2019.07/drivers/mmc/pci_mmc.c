// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Google, Inc
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <mapmem.h>
#include <sdhci.h>
#include <asm/pci.h>

struct pci_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct pci_mmc_priv {
	struct sdhci_host host;
	void *base;
};

static int pci_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct pci_mmc_plat *plat = dev_get_platdata(dev);
	struct pci_mmc_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host = &priv->host;
	int ret;

	host->ioaddr = (void *)dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0,
					      PCI_REGION_MEM);
	host->name = dev->name;
	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;
	host->mmc = &plat->mmc;
	host->mmc->priv = &priv->host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int pci_mmc_bind(struct udevice *dev)
{
	struct pci_mmc_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

U_BOOT_DRIVER(pci_mmc) = {
	.name	= "pci_mmc",
	.id	= UCLASS_MMC,
	.bind	= pci_mmc_bind,
	.probe	= pci_mmc_probe,
	.ops	= &sdhci_ops,
	.priv_auto_alloc_size = sizeof(struct pci_mmc_priv),
	.platdata_auto_alloc_size = sizeof(struct pci_mmc_plat),
};

static struct pci_device_id mmc_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_SYSTEM_SDHCI << 8, 0xffff00) },
	{},
};

U_BOOT_PCI_DEVICE(pci_mmc, mmc_supported);
