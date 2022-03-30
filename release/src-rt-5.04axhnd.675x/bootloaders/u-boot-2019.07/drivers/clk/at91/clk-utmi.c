// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <syscon.h>
#include <linux/io.h>
#include <mach/at91_pmc.h>
#include <mach/sama5_sfr.h>
#include "pmc.h"

/*
 * The purpose of this clock is to generate a 480 MHz signal. A different
 * rate can't be configured.
 */
#define UTMI_RATE	480000000

static int utmi_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk clk_dev;
	ulong clk_rate;
	u32 utmi_ref_clk_freq;
	u32 tmp;
	int err;
	int timeout = 2000000;

	if (readl(&pmc->sr) & AT91_PMC_LOCKU)
		return 0;

	/*
	 * If mainck rate is different from 12 MHz, we have to configure the
	 * FREQ field of the SFR_UTMICKTRIM register to generate properly
	 * the utmi clock.
	 */
	err = clk_get_by_index(clk->dev, 0, &clk_dev);
	if (err)
		return -EINVAL;

	clk_rate = clk_get_rate(&clk_dev);
	switch (clk_rate) {
	case 12000000:
		utmi_ref_clk_freq = 0;
		break;
	case 16000000:
		utmi_ref_clk_freq = 1;
		break;
	case 24000000:
		utmi_ref_clk_freq = 2;
		break;
	/*
	 * Not supported on SAMA5D2 but it's not an issue since MAINCK
	 * maximum value is 24 MHz.
	 */
	case 48000000:
		utmi_ref_clk_freq = 3;
		break;
	default:
		printf("UTMICK: unsupported mainck rate\n");
		return -EINVAL;
	}

	if (plat->regmap_sfr) {
		err = regmap_read(plat->regmap_sfr, AT91_SFR_UTMICKTRIM, &tmp);
		if (err)
			return -EINVAL;

		tmp &= ~AT91_UTMICKTRIM_FREQ;
		tmp |= utmi_ref_clk_freq;
		err = regmap_write(plat->regmap_sfr, AT91_SFR_UTMICKTRIM, tmp);
		if (err)
			return -EINVAL;
	} else if (utmi_ref_clk_freq) {
		printf("UTMICK: sfr node required\n");
		return -EINVAL;
	}

	tmp = readl(&pmc->uckr);
	tmp |= AT91_PMC_UPLLEN |
	       AT91_PMC_UPLLCOUNT |
	       AT91_PMC_BIASEN;
	writel(tmp, &pmc->uckr);

	while ((--timeout) && !(readl(&pmc->sr) & AT91_PMC_LOCKU))
		;
	if (!timeout) {
		printf("UTMICK: timeout waiting for UPLL lock\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static ulong utmi_clk_get_rate(struct clk *clk)
{
	/* UTMI clk rate is fixed. */
	return UTMI_RATE;
}

static struct clk_ops utmi_clk_ops = {
	.enable = utmi_clk_enable,
	.get_rate = utmi_clk_get_rate,
};

static int utmi_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct pmc_platdata *plat = dev_get_platdata(dev);
	struct udevice *syscon;

	uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
				     "regmap-sfr", &syscon);

	if (syscon)
		plat->regmap_sfr = syscon_get_regmap(syscon);

	return 0;
}

static int utmi_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id utmi_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-utmi" },
	{}
};

U_BOOT_DRIVER(at91sam9x5_utmi_clk) = {
	.name = "at91sam9x5-utmi-clk",
	.id = UCLASS_CLK,
	.of_match = utmi_clk_match,
	.probe = utmi_clk_probe,
	.ofdata_to_platdata = utmi_clk_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &utmi_clk_ops,
};
