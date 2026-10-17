// SPDX-License-Identifier: GPL-2.0+
/*
 * System clock support for AT91 architectures.
 *
 * Copyright (C) Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-system.c from Linux.
 */
#include <asm/processor.h>
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_SYSTEM		"at91-system-clk"

#define SYSTEM_MAX_ID		31

struct clk_system {
	void __iomem *base;
	struct clk clk;
	u8 id;
};

#define to_clk_system(_c) container_of(_c, struct clk_system, clk)

static inline int is_pck(int id)
{
	return (id >= 8) && (id <= 15);
}

static inline bool clk_system_ready(void __iomem *base, int id)
{
	unsigned int status;

	pmc_read(base, AT91_PMC_SR, &status);

	return !!(status & (1 << id));
}

static int clk_system_enable(struct clk *clk)
{
	struct clk_system *sys = to_clk_system(clk);

	pmc_write(sys->base, AT91_PMC_SCER, 1 << sys->id);

	if (!is_pck(sys->id))
		return 0;

	while (!clk_system_ready(sys->base, sys->id)) {
		debug("waiting for pck%u\n", sys->id);
		cpu_relax();
	}

	return 0;
}

static int clk_system_disable(struct clk *clk)
{
	struct clk_system *sys = to_clk_system(clk);

	pmc_write(sys->base, AT91_PMC_SCDR, 1 << sys->id);

	return 0;
}

static const struct clk_ops system_ops = {
	.enable = clk_system_enable,
	.disable = clk_system_disable,
	.get_rate = clk_generic_get_rate,
};

struct clk *at91_clk_register_system(void __iomem *base, const char *name,
				     const char *parent_name, u8 id)
{
	struct clk_system *sys;
	struct clk *clk;
	int ret;

	if (!base || !name || !parent_name || id > SYSTEM_MAX_ID)
		return ERR_PTR(-EINVAL);

	sys = kzalloc(sizeof(*sys), GFP_KERNEL);
	if (!sys)
		return ERR_PTR(-ENOMEM);

	sys->id = id;
	sys->base = base;

	clk = &sys->clk;
	clk->flags = CLK_GET_RATE_NOCACHE;
	ret = clk_register(clk, UBOOT_DM_CLK_AT91_SYSTEM, name, parent_name);
	if (ret) {
		kfree(sys);
		clk = ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(at91_system_clk) = {
	.name = UBOOT_DM_CLK_AT91_SYSTEM,
	.id = UCLASS_CLK,
	.ops = &system_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
