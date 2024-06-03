// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */
#include <common.h>
#include <command.h>
#include <dm.h>

#include <asm/io.h>
#include <asm/pl310.h>

static void l2c310_of_parse_and_init(struct udevice *dev)
{
	u32 tag[3] = { 0, 0, 0 };
	u32 saved_reg, prefetch;
	struct pl310_regs *regs = (struct pl310_regs *)dev_read_addr(dev);

	/* Disable the L2 Cache */
	clrbits_le32(&regs->pl310_ctrl, L2X0_CTRL_EN);

	saved_reg = readl(&regs->pl310_aux_ctrl);
	if (!dev_read_u32(dev, "prefetch-data", &prefetch)) {
		if (prefetch)
			saved_reg |= L310_AUX_CTRL_DATA_PREFETCH_MASK;
		else
			saved_reg &= ~L310_AUX_CTRL_DATA_PREFETCH_MASK;
	}

	if (!dev_read_u32(dev, "prefetch-instr", &prefetch)) {
		if (prefetch)
			saved_reg |= L310_AUX_CTRL_INST_PREFETCH_MASK;
		else
			saved_reg &= ~L310_AUX_CTRL_INST_PREFETCH_MASK;
	}

	saved_reg |= dev_read_bool(dev, "arm,shared-override");
	writel(saved_reg, &regs->pl310_aux_ctrl);

	saved_reg = readl(&regs->pl310_tag_latency_ctrl);
	if (!dev_read_u32_array(dev, "arm,tag-latency", tag, 3))
		saved_reg |= L310_LATENCY_CTRL_RD(tag[0] - 1) |
			     L310_LATENCY_CTRL_WR(tag[1] - 1) |
			     L310_LATENCY_CTRL_SETUP(tag[2] - 1);
	writel(saved_reg, &regs->pl310_tag_latency_ctrl);

	saved_reg = readl(&regs->pl310_data_latency_ctrl);
	if (!dev_read_u32_array(dev, "arm,data-latency", tag, 3))
		saved_reg |= L310_LATENCY_CTRL_RD(tag[0] - 1) |
			     L310_LATENCY_CTRL_WR(tag[1] - 1) |
			     L310_LATENCY_CTRL_SETUP(tag[2] - 1);
	writel(saved_reg, &regs->pl310_data_latency_ctrl);

	/* Enable the L2 cache */
	setbits_le32(&regs->pl310_ctrl, L2X0_CTRL_EN);
}

static int l2x0_probe(struct udevice *dev)
{
	l2c310_of_parse_and_init(dev);

	return 0;
}


static const struct udevice_id l2x0_ids[] = {
	{ .compatible = "arm,pl310-cache" },
	{}
};

U_BOOT_DRIVER(pl310_cache) = {
	.name   = "pl310_cache",
	.id     = UCLASS_CACHE,
	.of_match = l2x0_ids,
	.probe	= l2x0_probe,
	.flags  = DM_FLAG_PRE_RELOC,
};
