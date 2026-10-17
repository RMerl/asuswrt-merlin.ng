// SPDX-License-Identifier: GPL-2.0+
/*
 * OMAP clock controller support
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <clk-uclass.h>
#include <asm/arch-am33xx/clock.h>

struct clk_ti_ctrl_offs {
	fdt_addr_t start;
	fdt_size_t end;
};

struct clk_ti_ctrl_priv {
	int offs_num;
	struct clk_ti_ctrl_offs *offs;
};

static int clk_ti_ctrl_check_offs(struct clk *clk, fdt_addr_t offs)
{
	struct clk_ti_ctrl_priv *priv = dev_get_priv(clk->dev);
	int i;

	for (i = 0; i < priv->offs_num; i++) {
		if (offs >= priv->offs[i].start && offs <= priv->offs[i].end)
			return 0;
	}

	return -EFAULT;
}

static int clk_ti_ctrl_disable(struct clk *clk)
{
	struct clk_ti_ctrl_priv *priv = dev_get_priv(clk->dev);
	u32 *clk_modules[2] = { };
	fdt_addr_t offs;
	int err;

	offs = priv->offs[0].start + clk->id;
	err = clk_ti_ctrl_check_offs(clk, offs);
	if (err) {
		dev_err(clk->dev, "invalid offset: 0x%llx\n", (fdt64_t)offs);
		return err;
	}

	clk_modules[0] = (u32 *)(offs);
	dev_dbg(clk->dev, "disable module @ %p\n", clk_modules[0]);
	do_disable_clocks(NULL, clk_modules, 1);
	return 0;
}

static int clk_ti_ctrl_enable(struct clk *clk)
{
	struct clk_ti_ctrl_priv *priv = dev_get_priv(clk->dev);
	u32 *clk_modules[2] = { };
	fdt_addr_t offs;
	int err;

	offs = priv->offs[0].start + clk->id;
	err = clk_ti_ctrl_check_offs(clk, offs);
	if (err) {
		dev_err(clk->dev, "invalid offset: 0x%llx\n", (fdt64_t)offs);
		return err;
	}

	clk_modules[0] = (u32 *)(offs);
	dev_dbg(clk->dev, "enable module @ %p\n", clk_modules[0]);
	do_enable_clocks(NULL, clk_modules, 1);
	return 0;
}

static ulong clk_ti_ctrl_get_rate(struct clk *clk)
{
	return 0;
}

static int clk_ti_ctrl_of_xlate(struct clk *clk,
				struct ofnode_phandle_args *args)
{
	if (args->args_count != 2) {
		dev_err(clk->dev, "invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = args->args[0];
	else
		clk->id = 0;

	dev_dbg(clk->dev, "name=%s, id=%ld\n", clk->dev->name, clk->id);
	return 0;
}

static int clk_ti_ctrl_of_to_plat(struct udevice *dev)
{
	struct clk_ti_ctrl_priv *priv = dev_get_priv(dev);
	fdt_size_t fdt_size;
	int i, size;

	size = dev_read_size(dev, "reg");
	if (size < 0) {
		dev_err(dev, "failed to get 'reg' size\n");
		return size;
	}

	priv->offs_num = size / 2 / sizeof(u32);
	dev_dbg(dev, "size=%d, regs_num=%d\n", size, priv->offs_num);

	priv->offs = kmalloc_array(priv->offs_num, sizeof(*priv->offs),
				   GFP_KERNEL);
	if (!priv->offs)
		return -ENOMEM;

	for (i = 0; i < priv->offs_num; i++) {
		priv->offs[i].start =
			dev_read_addr_size_index(dev, i, &fdt_size);
		if (priv->offs[i].start == FDT_ADDR_T_NONE) {
			dev_err(dev, "failed to get offset %d\n", i);
			return -EINVAL;
		}

		priv->offs[i].end = priv->offs[i].start + fdt_size;
		dev_dbg(dev, "start=0x%016llx, end=0x%016llx\n",
			(fdt64_t)priv->offs[i].start,
			(fdt64_t)priv->offs[i].end);
	}

	return 0;
}

static struct clk_ops clk_ti_ctrl_ops = {
	.of_xlate = clk_ti_ctrl_of_xlate,
	.enable = clk_ti_ctrl_enable,
	.disable = clk_ti_ctrl_disable,
	.get_rate = clk_ti_ctrl_get_rate,
};

static const struct udevice_id clk_ti_ctrl_ids[] = {
	{.compatible = "ti,clkctrl"},
	{},
};

U_BOOT_DRIVER(clk_ti_ctrl) = {
	.name = "ti_ctrl_clk",
	.id = UCLASS_CLK,
	.of_match = clk_ti_ctrl_ids,
	.of_to_plat = clk_ti_ctrl_of_to_plat,
	.ops = &clk_ti_ctrl_ops,
	.priv_auto = sizeof(struct clk_ti_ctrl_priv),
};
