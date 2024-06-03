// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Linaro
 * peter.griffin <peter.griffin@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <dwmmc.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

struct hi6220_dwmmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct hi6220_dwmmc_priv_data {
	struct dwmci_host host;
};

static int hi6220_dwmmc_ofdata_to_platdata(struct udevice *dev)
{
	struct hi6220_dwmmc_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;

	host->name = dev->name;
	host->ioaddr = (void *)devfdt_get_addr(dev);
	host->buswidth = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"bus-width", 4);

	/* use non-removable property for differentiating SD card and eMMC */
	if (dev_read_bool(dev, "non-removable"))
		host->dev_index = 0;
	else
		host->dev_index = 1;

	host->priv = priv;

	return 0;
}

static int hi6220_dwmmc_probe(struct udevice *dev)
{
	struct hi6220_dwmmc_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct hi6220_dwmmc_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;

	/* Use default bus speed due to absence of clk driver */
	host->bus_hz = 50000000;

	dwmci_setup_cfg(&plat->cfg, host, host->bus_hz, 400000);
	host->mmc = &plat->mmc;

	host->mmc->priv = &priv->host;
	upriv->mmc = host->mmc;
	host->mmc->dev = dev;

	return dwmci_probe(dev);
}

static int hi6220_dwmmc_bind(struct udevice *dev)
{
	struct hi6220_dwmmc_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = dwmci_bind(dev, &plat->mmc, &plat->cfg);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id hi6220_dwmmc_ids[] = {
	{ .compatible = "hisilicon,hi6220-dw-mshc" },
	{ .compatible = "hisilicon,hi3798cv200-dw-mshc" },
	{ }
};

U_BOOT_DRIVER(hi6220_dwmmc_drv) = {
	.name = "hi6220_dwmmc",
	.id = UCLASS_MMC,
	.of_match = hi6220_dwmmc_ids,
	.ofdata_to_platdata = hi6220_dwmmc_ofdata_to_platdata,
	.ops = &dm_dwmci_ops,
	.bind = hi6220_dwmmc_bind,
	.probe = hi6220_dwmmc_probe,
	.priv_auto_alloc_size = sizeof(struct hi6220_dwmmc_priv_data),
	.platdata_auto_alloc_size = sizeof(struct hi6220_dwmmc_plat),
};
