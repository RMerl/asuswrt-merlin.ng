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

#define SYSTEM_MAX_ID		31

/**
 * at91_system_clk_bind() - for the system clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
static int at91_system_clk_bind(struct udevice *dev)
{
	return at91_clk_sub_device_bind(dev, "system-clk");
}

static const struct udevice_id at91_system_clk_match[] = {
	{ .compatible = "atmel,at91rm9200-clk-system" },
	{}
};

U_BOOT_DRIVER(at91_system_clk) = {
	.name = "at91-system-clk",
	.id = UCLASS_MISC,
	.of_match = at91_system_clk_match,
	.bind = at91_system_clk_bind,
};

/*----------------------------------------------------------*/

static inline int is_pck(int id)
{
	return (id >= 8) && (id <= 15);
}

static ulong system_clk_get_rate(struct clk *clk)
{
	struct clk clk_dev;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &clk_dev);
	if (ret)
		return -EINVAL;

	return clk_get_rate(&clk_dev);
}

static ulong system_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk clk_dev;
	int ret;

	ret = clk_get_by_index(clk->dev, 0, &clk_dev);
	if (ret)
		return -EINVAL;

	return clk_set_rate(&clk_dev, rate);
}

static int system_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	u32 mask;

	if (clk->id > SYSTEM_MAX_ID)
		return -EINVAL;

	mask = BIT(clk->id);

	writel(mask, &pmc->scer);

	/**
	 * For the programmable clocks the Ready status in the PMC
	 * status register should be checked after enabling.
	 * For other clocks this is unnecessary.
	 */
	if (!is_pck(clk->id))
		return 0;

	while (!(readl(&pmc->sr) & mask))
		;

	return 0;
}

static struct clk_ops system_clk_ops = {
	.of_xlate = at91_clk_of_xlate,
	.get_rate = system_clk_get_rate,
	.set_rate = system_clk_set_rate,
	.enable = system_clk_enable,
};

U_BOOT_DRIVER(system_clk) = {
	.name = "system-clk",
	.id = UCLASS_CLK,
	.probe = at91_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &system_clk_ops,
};
