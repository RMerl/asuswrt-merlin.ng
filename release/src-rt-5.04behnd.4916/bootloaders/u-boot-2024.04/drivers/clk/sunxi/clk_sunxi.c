// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <reset.h>
#include <asm/io.h>
#include <clk/sunxi.h>
#include <dm/device-internal.h>
#include <linux/bitops.h>
#include <linux/log2.h>

extern U_BOOT_DRIVER(sunxi_reset);

static const struct ccu_clk_gate *plat_to_gate(struct ccu_plat *plat,
					       unsigned long id)
{
	if (id >= plat->desc->num_gates)
		return NULL;

	return &plat->desc->gates[id];
}

static int sunxi_set_gate(struct clk *clk, bool on)
{
	struct ccu_plat *plat = dev_get_plat(clk->dev);
	const struct ccu_clk_gate *gate = plat_to_gate(plat, clk->id);
	u32 reg;

	if (gate && (gate->flags & CCU_CLK_F_DUMMY_GATE))
		return 0;

	if (!gate || !(gate->flags & CCU_CLK_F_IS_VALID)) {
		printf("%s: (CLK#%ld) unhandled\n", __func__, clk->id);
		return 0;
	}

	debug("%s: (CLK#%ld) off#0x%x, BIT(%d)\n", __func__,
	      clk->id, gate->off, ilog2(gate->bit));

	reg = readl(plat->base + gate->off);
	if (on)
		reg |= gate->bit;
	else
		reg &= ~gate->bit;

	writel(reg, plat->base + gate->off);

	return 0;
}

static int sunxi_clk_enable(struct clk *clk)
{
	return sunxi_set_gate(clk, true);
}

static int sunxi_clk_disable(struct clk *clk)
{
	return sunxi_set_gate(clk, false);
}

struct clk_ops sunxi_clk_ops = {
	.enable = sunxi_clk_enable,
	.disable = sunxi_clk_disable,
};

static int sunxi_clk_bind(struct udevice *dev)
{
	/* Reuse the platform data for the reset driver. */
	return device_bind(dev, DM_DRIVER_REF(sunxi_reset), "reset",
			   dev_get_plat(dev), dev_ofnode(dev), NULL);
}

static int sunxi_clk_probe(struct udevice *dev)
{
	struct clk_bulk clk_bulk;
	struct reset_ctl_bulk rst_bulk;
	int ret;

	ret = clk_get_bulk(dev, &clk_bulk);
	if (!ret)
		clk_enable_bulk(&clk_bulk);

	ret = reset_get_bulk(dev, &rst_bulk);
	if (!ret)
		reset_deassert_bulk(&rst_bulk);

	return 0;
}

static int sunxi_clk_of_to_plat(struct udevice *dev)
{
	struct ccu_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base)
		return -ENOMEM;

	plat->desc = (const struct ccu_desc *)dev_get_driver_data(dev);
	if (!plat->desc)
		return -EINVAL;

	return 0;
}

extern const struct ccu_desc a10_ccu_desc;
extern const struct ccu_desc a10s_ccu_desc;
extern const struct ccu_desc a23_ccu_desc;
extern const struct ccu_desc a31_ccu_desc;
extern const struct ccu_desc a31_r_ccu_desc;
extern const struct ccu_desc a64_ccu_desc;
extern const struct ccu_desc a80_ccu_desc;
extern const struct ccu_desc a80_mmc_clk_desc;
extern const struct ccu_desc a83t_ccu_desc;
extern const struct ccu_desc d1_ccu_desc;
extern const struct ccu_desc f1c100s_ccu_desc;
extern const struct ccu_desc h3_ccu_desc;
extern const struct ccu_desc h6_ccu_desc;
extern const struct ccu_desc h616_ccu_desc;
extern const struct ccu_desc h6_r_ccu_desc;
extern const struct ccu_desc r40_ccu_desc;
extern const struct ccu_desc v3s_ccu_desc;

static const struct udevice_id sunxi_clk_ids[] = {
#ifdef CONFIG_CLK_SUN4I_A10
	{ .compatible = "allwinner,sun4i-a10-ccu",
	  .data = (ulong)&a10_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN5I_A10S
	{ .compatible = "allwinner,sun5i-a10s-ccu",
	  .data = (ulong)&a10s_ccu_desc },
	{ .compatible = "allwinner,sun5i-a13-ccu",
	  .data = (ulong)&a10s_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN6I_A31
	{ .compatible = "allwinner,sun6i-a31-ccu",
	  .data = (ulong)&a31_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN4I_A10
	{ .compatible = "allwinner,sun7i-a20-ccu",
	  .data = (ulong)&a10_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN8I_A23
	{ .compatible = "allwinner,sun8i-a23-ccu",
	  .data = (ulong)&a23_ccu_desc },
	{ .compatible = "allwinner,sun8i-a33-ccu",
	  .data = (ulong)&a23_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN8I_A83T
	{ .compatible = "allwinner,sun8i-a83t-ccu",
	  .data = (ulong)&a83t_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN6I_A31_R
	{ .compatible = "allwinner,sun8i-a83t-r-ccu",
	  .data = (ulong)&a31_r_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN8I_H3
	{ .compatible = "allwinner,sun8i-h3-ccu",
	  .data = (ulong)&h3_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN6I_A31_R
	{ .compatible = "allwinner,sun8i-h3-r-ccu",
	  .data = (ulong)&a31_r_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN8I_R40
	{ .compatible = "allwinner,sun8i-r40-ccu",
	  .data = (ulong)&r40_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN8I_V3S
	{ .compatible = "allwinner,sun8i-v3-ccu",
	  .data = (ulong)&v3s_ccu_desc },
	{ .compatible = "allwinner,sun8i-v3s-ccu",
	  .data = (ulong)&v3s_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN9I_A80
	{ .compatible = "allwinner,sun9i-a80-ccu",
	  .data = (ulong)&a80_ccu_desc },
	{ .compatible = "allwinner,sun9i-a80-mmc-config-clk",
	  .data = (ulong)&a80_mmc_clk_desc },
#endif
#ifdef CONFIG_CLK_SUN50I_A64
	{ .compatible = "allwinner,sun50i-a64-ccu",
	  .data = (ulong)&a64_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN6I_A31_R
	{ .compatible = "allwinner,sun50i-a64-r-ccu",
	  .data = (ulong)&a31_r_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN8I_H3
	{ .compatible = "allwinner,sun50i-h5-ccu",
	  .data = (ulong)&h3_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN20I_D1
	{ .compatible = "allwinner,sun20i-d1-ccu",
	  .data = (ulong)&d1_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN50I_H6
	{ .compatible = "allwinner,sun50i-h6-ccu",
	  .data = (ulong)&h6_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN50I_H6_R
	{ .compatible = "allwinner,sun50i-h6-r-ccu",
	  .data = (ulong)&h6_r_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN50I_H616
	{ .compatible = "allwinner,sun50i-h616-ccu",
	  .data = (ulong)&h616_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUN50I_H6_R
	{ .compatible = "allwinner,sun50i-h616-r-ccu",
	  .data = (ulong)&h6_r_ccu_desc },
#endif
#ifdef CONFIG_CLK_SUNIV_F1C100S
	{ .compatible = "allwinner,suniv-f1c100s-ccu",
	  .data = (ulong)&f1c100s_ccu_desc },
#endif
	{ }
};

U_BOOT_DRIVER(sunxi_clk) = {
	.name		= "sunxi_clk",
	.id		= UCLASS_CLK,
	.of_match	= sunxi_clk_ids,
	.bind		= sunxi_clk_bind,
	.probe		= sunxi_clk_probe,
	.of_to_plat	= sunxi_clk_of_to_plat,
	.plat_auto	= sizeof(struct ccu_plat),
	.ops		= &sunxi_clk_ops,
};
