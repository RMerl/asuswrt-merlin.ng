// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Microhip / Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <linux/io.h>
#include <mach/at91_pmc.h>
#include "pmc.h"

DECLARE_GLOBAL_DATA_PTR;

#define AT91_USB_CLK_SOURCE_MAX	2
#define AT91_USB_CLK_MAX_DIV	15

struct at91_usb_clk_priv {
	u32 num_clksource;
};

static ulong at91_usb_clk_get_rate(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct clk source;
	u32 tmp, usbdiv;
	u8 source_index;
	int ret;

	tmp = readl(&pmc->pcr);
	source_index = (tmp >> AT91_PMC_USB_USBS_OFFSET) &
			AT91_PMC_USB_USBS_MASK;
	usbdiv = (tmp >> AT91_PMC_USB_DIV_OFFSET) & AT91_PMC_USB_DIV_MASK;

	ret = clk_get_by_index(clk->dev, source_index, &source);
	if (ret)
		return 0;

	return clk_get_rate(&source) / (usbdiv + 1);
}

static ulong at91_usb_clk_set_rate(struct clk *clk, ulong rate)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	struct at91_usb_clk_priv *priv = dev_get_priv(clk->dev);
	struct clk source, best_source;
	ulong tmp_rate, best_rate = rate, source_rate;
	int tmp_diff, best_diff = -1;
	u32 div, best_div = 0;
	u8 best_source_index = 0;
	u8 i;
	u32 tmp;
	int ret;

	for (i = 0; i < priv->num_clksource; i++) {
		ret = clk_get_by_index(clk->dev, i, &source);
		if (ret)
			return ret;

		source_rate = clk_get_rate(&source);
		if (IS_ERR_VALUE(source_rate))
			return source_rate;

		for (div = 1; div < AT91_USB_CLK_MAX_DIV + 2; div++) {
			tmp_rate = DIV_ROUND_CLOSEST(source_rate, div);
			tmp_diff = abs(rate - tmp_rate);

			if (best_diff < 0 || best_diff > tmp_diff) {
				best_rate = tmp_rate;
				best_diff = tmp_diff;

				best_div = div - 1;
				best_source = source;
				best_source_index = i;
			}

			if (!best_diff || tmp_rate < rate)
				break;
		}

		if (!best_diff)
			break;
	}

	debug("AT91 USB: best sourc: %s, best_rate = %ld, best_div = %d\n",
	      best_source.dev->name, best_rate, best_div);

	ret = clk_enable(&best_source);
	if (ret)
		return ret;

	tmp = AT91_PMC_USB_USBS_(best_source_index) |
	      AT91_PMC_USB_DIV_(best_div);
	writel(tmp, &pmc->usb);

	return 0;
}

static struct clk_ops at91_usb_clk_ops = {
	.get_rate = at91_usb_clk_get_rate,
	.set_rate = at91_usb_clk_set_rate,
};

static int at91_usb_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct at91_usb_clk_priv *priv = dev_get_priv(dev);
	u32 cells[AT91_USB_CLK_SOURCE_MAX];
	u32 num_clksource;

	num_clksource = fdtdec_get_int_array_count(gd->fdt_blob,
						   dev_of_offset(dev),
						   "clocks", cells,
						   AT91_USB_CLK_SOURCE_MAX);

	if (!num_clksource)
		return -1;

	priv->num_clksource = num_clksource;

	return 0;
}

static int at91_usb_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id at91_usb_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-usb" },
	{}
};

U_BOOT_DRIVER(at91_usb_clk) = {
	.name = "at91-usb-clk",
	.id = UCLASS_CLK,
	.of_match = at91_usb_clk_match,
	.probe = at91_usb_clk_probe,
	.ofdata_to_platdata = at91_usb_clk_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct at91_usb_clk_priv),
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &at91_usb_clk_ops,
};
