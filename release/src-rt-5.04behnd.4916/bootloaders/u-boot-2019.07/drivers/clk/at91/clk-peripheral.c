// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/io.h>
#include <mach/at91_pmc.h>
#include "pmc.h"

#define PERIPHERAL_ID_MIN	2
#define PERIPHERAL_ID_MAX	31
#define PERIPHERAL_MASK(id)	(1 << ((id) & PERIPHERAL_ID_MAX))

enum periph_clk_type {
	CLK_PERIPH_AT91RM9200 = 0,
	CLK_PERIPH_AT91SAM9X5,
};
/**
 * sam9x5_periph_clk_bind() - for the periph clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
static int sam9x5_periph_clk_bind(struct udevice *dev)
{
	return at91_clk_sub_device_bind(dev, "periph-clk");
}

static const struct udevice_id sam9x5_periph_clk_match[] = {
	{
		.compatible = "atmel,at91rm9200-clk-peripheral",
		.data = CLK_PERIPH_AT91RM9200,
	},
	{
		.compatible = "atmel,at91sam9x5-clk-peripheral",
		.data = CLK_PERIPH_AT91SAM9X5,
	},
	{}
};

U_BOOT_DRIVER(sam9x5_periph_clk) = {
	.name = "sam9x5-periph-clk",
	.id = UCLASS_MISC,
	.of_match = sam9x5_periph_clk_match,
	.bind = sam9x5_periph_clk_bind,
};

/*---------------------------------------------------------*/

static int periph_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	enum periph_clk_type clk_type;
	void *addr;

	if (clk->id < PERIPHERAL_ID_MIN)
		return -1;

	clk_type = dev_get_driver_data(dev_get_parent(clk->dev));
	if (clk_type == CLK_PERIPH_AT91RM9200) {
		addr = &pmc->pcer;
		if (clk->id > PERIPHERAL_ID_MAX)
			addr = &pmc->pcer1;

		setbits_le32(addr, PERIPHERAL_MASK(clk->id));
	} else {
		writel(clk->id & AT91_PMC_PCR_PID_MASK, &pmc->pcr);
		setbits_le32(&pmc->pcr,
			     AT91_PMC_PCR_CMD_WRITE | AT91_PMC_PCR_EN);
	}

	return 0;
}

static ulong periph_get_rate(struct clk *clk)
{
	struct udevice *dev;
	struct clk clk_dev;
	ulong clk_rate;
	int ret;

	dev = dev_get_parent(clk->dev);

	ret = clk_get_by_index(dev, 0, &clk_dev);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&clk_dev);

	clk_free(&clk_dev);

	return clk_rate;
}

static struct clk_ops periph_clk_ops = {
	.of_xlate = at91_clk_of_xlate,
	.enable = periph_clk_enable,
	.get_rate = periph_get_rate,
};

U_BOOT_DRIVER(clk_periph) = {
	.name	= "periph-clk",
	.id	= UCLASS_CLK,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.probe = at91_clk_probe,
	.ops	= &periph_clk_ops,
};
