// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Youngbok, Park <park@nexell.co.kr>
 *
 * (C) Copyright 2019 Stefan Bosch <stefan_b@posteo.net>
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <dwmmc.h>
#include <log.h>
#include <syscon.h>
#include <asm/arch/reset.h>
#include <asm/arch/clk.h>

#define DWMCI_CLKSEL			0x09C
#define DWMCI_SHIFT_0			0x0
#define DWMCI_SHIFT_1			0x1
#define DWMCI_SHIFT_2			0x2
#define DWMCI_SHIFT_3			0x3
#define DWMCI_SET_SAMPLE_CLK(x)	(x)
#define DWMCI_SET_DRV_CLK(x)	((x) << 16)
#define DWMCI_SET_DIV_RATIO(x)	((x) << 24)
#define DWMCI_CLKCTRL			0x114
#define NX_MMC_CLK_DELAY(x, y, a, b)	((((x) & 0xFF) << 0) |\
					(((y) & 0x03) << 16) |\
					(((a) & 0xFF) << 8)  |\
					(((b) & 0x03) << 24))

struct nexell_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct nexell_dwmmc_priv {
	struct clk *clk;
	struct dwmci_host host;
	int fifo_size;
	bool fifo_mode;
	int frequency;
	u32 min_freq;
	u32 max_freq;
	int d_delay;
	int d_shift;
	int s_delay;
	int s_shift;
	bool mmcboost;
};

struct clk *clk_get(const char *id);

static int nx_dw_mmc_clksel(struct dwmci_host *host)
{
	/* host->priv is pointer to "struct udevice" */
	struct nexell_dwmmc_priv *priv = dev_get_priv(host->priv);
	u32 val;

	if (priv->mmcboost)
		val = DWMCI_SET_SAMPLE_CLK(DWMCI_SHIFT_0) |
		      DWMCI_SET_DRV_CLK(DWMCI_SHIFT_0) | DWMCI_SET_DIV_RATIO(1);
	else
		val = DWMCI_SET_SAMPLE_CLK(DWMCI_SHIFT_0) |
		      DWMCI_SET_DRV_CLK(DWMCI_SHIFT_0) | DWMCI_SET_DIV_RATIO(3);

	dwmci_writel(host, DWMCI_CLKSEL, val);

	return 0;
}

static void nx_dw_mmc_reset(int ch)
{
	int rst_id = RESET_ID_SDMMC0 + ch;

	nx_rstcon_setrst(rst_id, 0);
	nx_rstcon_setrst(rst_id, 1);
}

static void nx_dw_mmc_clk_delay(struct udevice *dev)
{
	unsigned int delay;
	struct nexell_dwmmc_priv *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;

	delay = NX_MMC_CLK_DELAY(priv->d_delay,
				 priv->d_shift, priv->s_delay, priv->s_shift);

	writel(delay, (host->ioaddr + DWMCI_CLKCTRL));
	debug("%s: Values set: d_delay==%d, d_shift==%d, s_delay==%d, "
	      "s_shift==%d\n", __func__, priv->d_delay, priv->d_shift,
	      priv->s_delay, priv->s_shift);
}

static unsigned int nx_dw_mmc_get_clk(struct dwmci_host *host, uint freq)
{
	struct clk *clk;
	struct udevice *dev = host->priv;
	struct nexell_dwmmc_priv *priv = dev_get_priv(dev);

	int index = host->dev_index;
	char name[50] = { 0, };

	clk = priv->clk;
	if (!clk) {
		sprintf(name, "%s.%d", DEV_NAME_SDHC, index);
		clk = clk_get((const char *)name);
		if (!clk)
			return 0;
		priv->clk = clk;
	}

	return clk_get_rate(clk) / 2;
}

static unsigned long nx_dw_mmc_set_clk(struct dwmci_host *host,
				       unsigned int rate)
{
	struct clk *clk;
	char name[50] = { 0, };
	struct udevice *dev = host->priv;
	struct nexell_dwmmc_priv *priv = dev_get_priv(dev);

	int index = host->dev_index;

	clk = priv->clk;
	if (!clk) {
		sprintf(name, "%s.%d", DEV_NAME_SDHC, index);
		clk = clk_get((const char *)name);
		if (!clk) {
			debug("%s: clk_get(\"%s\") failed!\n", __func__, name);
			return 0;
		}
		priv->clk = clk;
	}

	clk_disable(clk);
	rate = clk_set_rate(clk, rate);
	clk_enable(clk);

	return rate;
}

static int nexell_dwmmc_of_to_plat(struct udevice *dev)
{
	struct nexell_dwmmc_priv *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	int val = -1;

	debug("%s\n", __func__);

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);
	host->buswidth = dev_read_u32_default(dev, "bus-width", 4);
	host->get_mmc_clk = nx_dw_mmc_get_clk;
	host->clksel = nx_dw_mmc_clksel;
	host->priv = dev;

	val = dev_read_u32_default(dev, "index", -1);
	if (val < 0 || val > 2) {
		debug("  'index' missing/invalid!\n");
		return -EINVAL;
	}
	host->dev_index = val;

	priv->fifo_size = dev_read_u32_default(dev, "fifo-size", 0x20);
	priv->fifo_mode = dev_read_bool(dev, "fifo-mode");
	priv->frequency = dev_read_u32_default(dev, "frequency", 50000000);
	priv->max_freq = dev_read_u32_default(dev, "max-frequency", 50000000);
	priv->min_freq = 400000;  /* 400 kHz */
	priv->d_delay = dev_read_u32_default(dev, "drive_dly", 0);
	priv->d_shift = dev_read_u32_default(dev, "drive_shift", 3);
	priv->s_delay = dev_read_u32_default(dev, "sample_dly", 0);
	priv->s_shift = dev_read_u32_default(dev, "sample_shift", 2);
	priv->mmcboost = dev_read_u32_default(dev, "mmcboost", 0);

	debug("  index==%d, name==%s, ioaddr==0x%08x\n",
	      host->dev_index, host->name, (u32)host->ioaddr);
	return 0;
}

static int nexell_dwmmc_probe(struct udevice *dev)
{
	struct nexell_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct nexell_dwmmc_priv *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	struct udevice *pwr_dev __maybe_unused;

	host->fifoth_val = MSIZE(0x2) |
		RX_WMARK(priv->fifo_size / 2 - 1) |
		TX_WMARK(priv->fifo_size / 2);

	host->fifo_mode = priv->fifo_mode;

	dwmci_setup_cfg(&plat->cfg, host, priv->max_freq, priv->min_freq);
	host->mmc = &plat->mmc;
	host->mmc->priv = &priv->host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	if (nx_dw_mmc_set_clk(host, priv->frequency * 4) !=
	    priv->frequency * 4) {
		debug("%s: nx_dw_mmc_set_clk(host, %d) failed!\n",
		      __func__, priv->frequency * 4);
		return -EIO;
	}
	debug("%s: nx_dw_mmc_set_clk(host, %d) OK\n",
	      __func__, priv->frequency * 4);

	nx_dw_mmc_reset(host->dev_index);
	nx_dw_mmc_clk_delay(dev);

	return dwmci_probe(dev);
}

static int nexell_dwmmc_bind(struct udevice *dev)
{
	struct nexell_mmc_plat *plat = dev_get_plat(dev);

	return dwmci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id nexell_dwmmc_ids[] = {
	{ .compatible = "nexell,nexell-dwmmc" },
	{ }
};

U_BOOT_DRIVER(nexell_dwmmc_drv) = {
	.name		= "nexell_dwmmc",
	.id		= UCLASS_MMC,
	.of_match	= nexell_dwmmc_ids,
	.of_to_plat = nexell_dwmmc_of_to_plat,
	.ops		= &dm_dwmci_ops,
	.bind		= nexell_dwmmc_bind,
	.probe		= nexell_dwmmc_probe,
	.priv_auto	= sizeof(struct nexell_dwmmc_priv),
	.plat_auto	= sizeof(struct nexell_mmc_plat),
};
