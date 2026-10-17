// SPDX-License-Identifier: GPL-2.0+
/*
 * Socionext F_SDH30 eMMC driver
 * Copyright 2021 Linaro Ltd.
 * Copyright 2021 Socionext, Inc.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>

#define F_SDH30_ESD_CONTROL		0x124
#define F_SDH30_CMD_DAT_DELAY		BIT(9)

#define F_SDH30_TEST			0x158
#define F_SDH30_FORCE_CARD_INSERT	BIT(6)

struct f_sdh30_data {
	void (*init)(struct udevice *dev);
	u32 quirks;
};

struct f_sdh30_plat {
	struct mmc_config cfg;
	struct mmc mmc;

	bool enable_cmd_dat_delay;
	const struct f_sdh30_data *data;
};

DECLARE_GLOBAL_DATA_PTR;

static void f_sdh30_e51_init(struct udevice *dev)
{
	struct f_sdh30_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	u32 val;

	val = sdhci_readl(host, F_SDH30_ESD_CONTROL);
	if (plat->enable_cmd_dat_delay)
		val |= F_SDH30_CMD_DAT_DELAY;
	else
		val &= ~F_SDH30_CMD_DAT_DELAY;
	sdhci_writel(host, val, F_SDH30_ESD_CONTROL);

	val = sdhci_readl(host, F_SDH30_TEST);
	if (plat->cfg.host_caps & MMC_CAP_NONREMOVABLE)
		val |= F_SDH30_FORCE_CARD_INSERT;
	else
		val &= ~F_SDH30_FORCE_CARD_INSERT;
	sdhci_writel(host, val, F_SDH30_TEST);
}

static int f_sdh30_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct f_sdh30_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	plat->data = (const struct f_sdh30_data *)dev_get_driver_data(dev);

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;

	if (plat->data && plat->data->quirks)
		host->quirks = plat->data->quirks;

	ret = sdhci_setup_cfg(&plat->cfg, host, 200000000, 400000);
	if (ret)
		return ret;

	upriv->mmc = host->mmc;

	mmc_set_clock(host->mmc, host->mmc->cfg->f_min, MMC_CLK_ENABLE);

	ret = sdhci_probe(dev);
	if (ret)
		return ret;

	if (plat->data && plat->data->init)
		plat->data->init(dev);

	return 0;
}

static int f_sdh30_of_to_plat(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);
	struct f_sdh30_plat *plat = dev_get_plat(dev);

	host->name = strdup(dev->name);
	host->ioaddr = dev_read_addr_ptr(dev);
	host->bus_width = dev_read_u32_default(dev, "bus-width", 4);
	host->index = dev_read_u32_default(dev, "index", 0);

	plat->enable_cmd_dat_delay =
		dev_read_bool(dev, "socionext,enable-cmd-dat-delay");

	return 0;
}

static int f_sdh30_bind(struct udevice *dev)
{
	struct f_sdh30_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct f_sdh30_data f_sdh30_e51_data = {
	.init = f_sdh30_e51_init,
	.quirks = SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_SUPPORT_SINGLE,
};

static const struct udevice_id f_sdh30_mmc_ids[] = {
	{
		.compatible = "fujitsu,mb86s70-sdhci-3.0",
	},
	{
		.compatible = "socionext,f-sdh30-e51-mmc",
		.data = (ulong)&f_sdh30_e51_data,
	},
	{ }
};

U_BOOT_DRIVER(f_sdh30_drv) = {
	.name		= "f_sdh30_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= f_sdh30_mmc_ids,
	.of_to_plat	= f_sdh30_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= f_sdh30_bind,
	.probe		= f_sdh30_sdhci_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct f_sdh30_plat),
};
