// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell SD Host Controller Interface
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/mbus.h>

#define MVSDH_NAME "mv_sdh"

#define SDHCI_WINDOW_CTRL(win)		(0x4080 + ((win) << 4))
#define SDHCI_WINDOW_BASE(win)		(0x4084 + ((win) << 4))

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

#ifndef CONFIG_DM_MMC

#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
static struct sdhci_ops mv_ops;

#if defined(CONFIG_SHEEVA_88SV331xV5)
#define SD_CE_ATA_2	0xEA
#define  MMC_CARD	0x1000
#define  MMC_WIDTH	0x0100
static inline void mv_sdhci_writeb(struct sdhci_host *host, u8 val, int reg)
{
	struct mmc *mmc = host->mmc;
	u32 ata = (unsigned long)host->ioaddr + SD_CE_ATA_2;

	if (!IS_SD(mmc) && reg == SDHCI_HOST_CONTROL) {
		if (mmc->bus_width == 8)
			writew(readw(ata) | (MMC_CARD | MMC_WIDTH), ata);
		else
			writew(readw(ata) & ~(MMC_CARD | MMC_WIDTH), ata);
	}

	writeb(val, host->ioaddr + reg);
}

#else
#define mv_sdhci_writeb	NULL
#endif /* CONFIG_SHEEVA_88SV331xV5 */
#endif /* CONFIG_MMC_SDHCI_IO_ACCESSORS */

int mv_sdh_init(unsigned long regbase, u32 max_clk, u32 min_clk, u32 quirks)
{
	struct sdhci_host *host = NULL;
	host = calloc(1, sizeof(*host));
	if (!host) {
		printf("sdh_host malloc fail!\n");
		return -ENOMEM;
	}

	host->name = MVSDH_NAME;
	host->ioaddr = (void *)regbase;
	host->quirks = quirks;
	host->max_clk = max_clk;
#ifdef CONFIG_MMC_SDHCI_IO_ACCESSORS
	memset(&mv_ops, 0, sizeof(struct sdhci_ops));
	mv_ops.write_b = mv_sdhci_writeb;
	host->ops = &mv_ops;
#endif

	if (CONFIG_IS_ENABLED(ARCH_MVEBU)) {
		/* Configure SDHCI MBUS mbus bridge windows */
		sdhci_mvebu_mbus_config((void __iomem *)regbase);
	}

	return add_sdhci(host, 0, min_clk);
}

#else

DECLARE_GLOBAL_DATA_PTR;

struct mv_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int mv_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mv_sdhci_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	host->name = MVSDH_NAME;
	host->ioaddr = (void *)devfdt_get_addr(dev);
	host->quirks = SDHCI_QUIRK_32BIT_DMA_ADDR | SDHCI_QUIRK_WAIT_SEND_CMD;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;

	if (CONFIG_IS_ENABLED(ARCH_MVEBU)) {
		/* Configure SDHCI MBUS mbus bridge windows */
		sdhci_mvebu_mbus_config(host->ioaddr);
	}

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->mmc->priv = host;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int mv_sdhci_bind(struct udevice *dev)
{
	struct mv_sdhci_plat *plat = dev_get_platdata(dev);

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
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct mv_sdhci_plat),
};
#endif /* CONFIG_DM_MMC */
