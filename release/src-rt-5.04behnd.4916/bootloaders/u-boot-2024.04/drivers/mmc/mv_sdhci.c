// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell SD Host Controller Interface
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/global_data.h>
#include <linux/mbus.h>

#define MVSDH_NAME "mv_sdh"

#define SDHCI_WINDOW_CTRL(win)		(0x4080 + ((win) << 4))
#define SDHCI_WINDOW_BASE(win)		(0x4084 + ((win) << 4))

DECLARE_GLOBAL_DATA_PTR;

struct mv_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static void sdhci_mvebu_mbus_config(void __iomem *base)
{
	const struct mbus_dram_target_info *dram;
	int i;

	dram = mvebu_mbus_dram_info();

	for (i = 0; i < 4; i++) {
		writel(0, base + SDHCI_WINDOW_CTRL(i));
		writel(0, base + SDHCI_WINDOW_BASE(i));
	}

	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		/* Write size, attributes and target id to control register */
		writel(((cs->size - 1) & 0xffff0000) | (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       base + SDHCI_WINDOW_CTRL(i));

		/* Write base address to base register */
		writel(cs->base, base + SDHCI_WINDOW_BASE(i));
	}
}

static int mv_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mv_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	host->name = MVSDH_NAME;
	host->ioaddr = dev_read_addr_ptr(dev);
	host->quirks = SDHCI_QUIRK_32BIT_DMA_ADDR | SDHCI_QUIRK_WAIT_SEND_CMD;
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;

	/* Configure SDHCI MBUS mbus bridge windows */
	sdhci_mvebu_mbus_config(host->ioaddr);

	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int mv_sdhci_bind(struct udevice *dev)
{
	struct mv_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id mv_sdhci_ids[] = {
	{ .compatible = "marvell,armada-380-sdhci" },
	{ }
};

U_BOOT_DRIVER(mv_sdhci_drv) = {
	.name		= MVSDH_NAME,
	.id		= UCLASS_MMC,
	.of_match	= mv_sdhci_ids,
	.bind		= mv_sdhci_bind,
	.probe		= mv_sdhci_probe,
	.ops		= &sdhci_ops,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct mv_sdhci_plat),
};
