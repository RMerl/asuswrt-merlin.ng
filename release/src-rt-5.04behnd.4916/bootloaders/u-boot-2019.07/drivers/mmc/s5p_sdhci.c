// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <asm/gpio.h>
#include <asm/arch/mmc.h>
#include <asm/arch/clk.h>
#include <errno.h>
#include <asm/arch/pinmux.h>

#ifdef CONFIG_DM_MMC
struct s5p_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

DECLARE_GLOBAL_DATA_PTR;
#endif

static char *S5P_NAME = "SAMSUNG SDHCI";
static void s5p_sdhci_set_control_reg(struct sdhci_host *host)
{
	unsigned long val, ctrl;
	/*
	 * SELCLKPADDS[17:16]
	 * 00 = 2mA
	 * 01 = 4mA
	 * 10 = 7mA
	 * 11 = 9mA
	 */
	sdhci_writel(host, SDHCI_CTRL4_DRIVE_MASK(0x3), SDHCI_CONTROL4);

	val = sdhci_readl(host, SDHCI_CONTROL2);
	val &= SDHCI_CTRL2_SELBASECLK_MASK(3);

	val |=	SDHCI_CTRL2_ENSTAASYNCCLR |
		SDHCI_CTRL2_ENCMDCNFMSK |
		SDHCI_CTRL2_ENFBCLKRX |
		SDHCI_CTRL2_ENCLKOUTHOLD;

	sdhci_writel(host, val, SDHCI_CONTROL2);

	/*
	 * FCSEL3[31] FCSEL2[23] FCSEL1[15] FCSEL0[7]
	 * FCSel[1:0] : Rx Feedback Clock Delay Control
	 *	Inverter delay means10ns delay if SDCLK 50MHz setting
	 *	01 = Delay1 (basic delay)
	 *	11 = Delay2 (basic delay + 2ns)
	 *	00 = Delay3 (inverter delay)
	 *	10 = Delay4 (inverter delay + 2ns)
	 */
	val = SDHCI_CTRL3_FCSEL0 | SDHCI_CTRL3_FCSEL1;
	sdhci_writel(host, val, SDHCI_CONTROL3);

	/*
	 * SELBASECLK[5:4]
	 * 00/01 = HCLK
	 * 10 = EPLL
	 * 11 = XTI or XEXTCLK
	 */
	ctrl = sdhci_readl(host, SDHCI_CONTROL2);
	ctrl &= ~SDHCI_CTRL2_SELBASECLK_MASK(0x3);
	ctrl |= SDHCI_CTRL2_SELBASECLK_MASK(0x2);
	sdhci_writel(host, ctrl, SDHCI_CONTROL2);
}

static void s5p_set_clock(struct sdhci_host *host, u32 div)
{
	/* ToDo : Use the Clock Framework */
	set_mmc_clk(host->index, div);
}

static const struct sdhci_ops s5p_sdhci_ops = {
	.set_clock	= &s5p_set_clock,
	.set_control_reg = &s5p_sdhci_set_control_reg,
};

static int s5p_sdhci_core_init(struct sdhci_host *host)
{
	host->name = S5P_NAME;

	host->quirks = SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_BROKEN_VOLTAGE |
		SDHCI_QUIRK_32BIT_DMA_ADDR |
		SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_USE_WIDE8;
	host->max_clk = 52000000;
	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	host->ops = &s5p_sdhci_ops;

	if (host->bus_width == 8)
		host->host_caps |= MMC_MODE_8BIT;

#ifndef CONFIG_BLK
	return add_sdhci(host, 0, 400000);
#else
	return 0;
#endif
}

int s5p_sdhci_init(u32 regbase, int index, int bus_width)
{
	struct sdhci_host *host = calloc(1, sizeof(struct sdhci_host));
	if (!host) {
		printf("sdhci__host allocation fail!\n");
		return -ENOMEM;
	}
	host->ioaddr = (void *)regbase;
	host->index = index;
	host->bus_width = bus_width;

	return s5p_sdhci_core_init(host);
}

static int do_sdhci_init(struct sdhci_host *host)
{
	int dev_id, flag, ret;

	flag = host->bus_width == 8 ? PINMUX_FLAG_8BIT_MODE : PINMUX_FLAG_NONE;
	dev_id = host->index + PERIPH_ID_SDMMC0;

	ret = exynos_pinmux_config(dev_id, flag);
	if (ret) {
		printf("external SD not configured\n");
		return ret;
	}

	if (dm_gpio_is_valid(&host->pwr_gpio)) {
		dm_gpio_set_value(&host->pwr_gpio, 1);
		ret = exynos_pinmux_config(dev_id, flag);
		if (ret) {
			debug("MMC not configured\n");
			return ret;
		}
	}

	if (dm_gpio_is_valid(&host->cd_gpio)) {
		ret = dm_gpio_get_value(&host->cd_gpio);
		if (ret) {
			debug("no SD card detected (%d)\n", ret);
			return -ENODEV;
		}
	}

	return s5p_sdhci_core_init(host);
}

static int sdhci_get_config(const void *blob, int node, struct sdhci_host *host)
{
	int bus_width, dev_id;
	unsigned int base;

	/* Get device id */
	dev_id = pinmux_decode_periph_id(blob, node);
	if (dev_id < PERIPH_ID_SDMMC0 || dev_id > PERIPH_ID_SDMMC3) {
		debug("MMC: Can't get device id\n");
		return -EINVAL;
	}
	host->index = dev_id - PERIPH_ID_SDMMC0;

	/* Get bus width */
	bus_width = fdtdec_get_int(blob, node, "samsung,bus-width", 0);
	if (bus_width <= 0) {
		debug("MMC: Can't get bus-width\n");
		return -EINVAL;
	}
	host->bus_width = bus_width;

	/* Get the base address from the device node */
	base = fdtdec_get_addr(blob, node, "reg");
	if (!base) {
		debug("MMC: Can't get base address\n");
		return -EINVAL;
	}
	host->ioaddr = (void *)base;

	gpio_request_by_name_nodev(offset_to_ofnode(node), "pwr-gpios", 0,
				   &host->pwr_gpio, GPIOD_IS_OUT);
	gpio_request_by_name_nodev(offset_to_ofnode(node), "cd-gpios", 0,
				   &host->cd_gpio, GPIOD_IS_IN);

	return 0;
}

#ifdef CONFIG_DM_MMC
static int s5p_sdhci_probe(struct udevice *dev)
{
	struct s5p_sdhci_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	ret = sdhci_get_config(gd->fdt_blob, dev_of_offset(dev), host);
	if (ret)
		return ret;

	ret = do_sdhci_init(host);
	if (ret)
		return ret;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 400000);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int s5p_sdhci_bind(struct udevice *dev)
{
	struct s5p_sdhci_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = sdhci_bind(dev, &plat->mmc, &plat->cfg);
	if (ret)
		return ret;

	return 0;
}

static const struct udevice_id s5p_sdhci_ids[] = {
	{ .compatible = "samsung,exynos4412-sdhci"},
	{ }
};

U_BOOT_DRIVER(s5p_sdhci_drv) = {
	.name		= "s5p_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= s5p_sdhci_ids,
	.bind		= s5p_sdhci_bind,
	.ops		= &sdhci_ops,
	.probe		= s5p_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct s5p_sdhci_plat),
};
#endif /* CONFIG_DM_MMC */
