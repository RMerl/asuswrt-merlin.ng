// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/arch/clk.h>
#include <asm/global_data.h>

#define ATMEL_SDHC_MIN_FREQ	400000
#define ATMEL_SDHC_GCK_RATE	240000000

#define ATMEL_SDHC_MC1R 0x204
#define ATMEL_SDHC_MC1R_FCD	0x80

#ifndef CONFIG_DM_MMC
int atmel_sdhci_init(void *regbase, u32 id)
{
	struct sdhci_host *host;
	u32 max_clk, min_clk = ATMEL_SDHC_MIN_FREQ;

	host = (struct sdhci_host *)calloc(1, sizeof(struct sdhci_host));
	if (!host) {
		printf("%s: sdhci_host calloc failed\n", __func__);
		return -ENOMEM;
	}

	host->name = "atmel_sdhci";
	host->ioaddr = regbase;
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;
	max_clk = at91_get_periph_generated_clk(id);
	if (!max_clk) {
		printf("%s: Failed to get the proper clock\n", __func__);
		free(host);
		return -ENODEV;
	}
	host->max_clk = max_clk;

	add_sdhci(host, 0, min_clk);

	return 0;
}

#else

DECLARE_GLOBAL_DATA_PTR;

struct atmel_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static void atmel_sdhci_config_fcd(struct sdhci_host *host)
{
	u8 mc1r;

	/* If nonremovable, assume that the card is always present.
	 *
	 * WA: SAMA5D2 doesn't drive CMD if using CD GPIO line.
	 */
	if ((host->mmc->cfg->host_caps & MMC_CAP_NONREMOVABLE)
#if CONFIG_IS_ENABLED(DM_GPIO)
		|| dm_gpio_get_value(&host->cd_gpio) >= 0
#endif
	   ) {
		sdhci_readb(host, ATMEL_SDHC_MC1R);
		mc1r |= ATMEL_SDHC_MC1R_FCD;
		sdhci_writeb(host, mc1r, ATMEL_SDHC_MC1R);
	}
}

static int atmel_sdhci_deferred_probe(struct sdhci_host *host)
{
	struct udevice *dev = host->mmc->dev;
	int ret;

	ret = sdhci_probe(dev);
	if (ret)
		return ret;

	atmel_sdhci_config_fcd(host);

	return 0;
}

static const struct sdhci_ops atmel_sdhci_ops = {
	.deferred_probe	= atmel_sdhci_deferred_probe,
};

static int atmel_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct atmel_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	u32 max_clk;
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;
	host->bus_width	= fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					 "bus-width", 4);

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return ret;

	clk_set_rate(&clk, ATMEL_SDHC_GCK_RATE);

	max_clk = clk_get_rate(&clk);
	if (!max_clk)
		return -EINVAL;

	ret = clk_enable(&clk);
	/* return error only if the clock really has a clock enable func */
	if (ret && ret != -ENOSYS)
		return ret;

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	host->max_clk = max_clk;
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, ATMEL_SDHC_MIN_FREQ);
	if (ret)
		return ret;

	host->mmc->priv = host;
	host->ops = &atmel_sdhci_ops;
	upriv->mmc = host->mmc;

	ret = sdhci_probe(dev);
	if (ret)
		return ret;

	atmel_sdhci_config_fcd(host);

	return 0;
}

static int atmel_sdhci_bind(struct udevice *dev)
{
	struct atmel_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id atmel_sdhci_ids[] = {
	{ .compatible = "atmel,sama5d2-sdhci" },
	{ .compatible = "microchip,sam9x60-sdhci" },
	{ .compatible = "microchip,sama7g5-sdhci" },
	{ }
};

U_BOOT_DRIVER(atmel_sdhci_drv) = {
	.name		= "atmel_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= atmel_sdhci_ids,
	.ops		= &sdhci_ops,
	.bind		= atmel_sdhci_bind,
	.probe		= atmel_sdhci_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct atmel_sdhci_plat),
};
#endif
