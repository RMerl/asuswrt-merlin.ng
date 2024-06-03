// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Weidmüller Interface GmbH & Co. KG
 * Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
 *
 * Copyright (C) 2013 Soren Brinkmann <soren.brinkmann@xilinx.com>
 * Copyright (C) 2013 Xilinx, Inc. All rights reserved.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/lists.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

/* Register bitfield defines */
#define PLLCTRL_FBDIV_MASK	0x7f000
#define PLLCTRL_FBDIV_SHIFT	12
#define PLLCTRL_BPFORCE_MASK	(1 << 4)
#define PLLCTRL_PWRDWN_MASK	2
#define PLLCTRL_PWRDWN_SHIFT	1
#define PLLCTRL_RESET_MASK	1
#define PLLCTRL_RESET_SHIFT	0

#define ZYNQ_CLK_MAXDIV		0x3f
#define CLK_CTRL_DIV1_SHIFT	20
#define CLK_CTRL_DIV1_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV1_SHIFT)
#define CLK_CTRL_DIV0_SHIFT	8
#define CLK_CTRL_DIV0_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV0_SHIFT)
#define CLK_CTRL_SRCSEL_SHIFT	4
#define CLK_CTRL_SRCSEL_MASK	(0x3 << CLK_CTRL_SRCSEL_SHIFT)

#define CLK_CTRL_DIV2X_SHIFT	26
#define CLK_CTRL_DIV2X_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV2X_SHIFT)
#define CLK_CTRL_DIV3X_SHIFT	20
#define CLK_CTRL_DIV3X_MASK	(ZYNQ_CLK_MAXDIV << CLK_CTRL_DIV3X_SHIFT)

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SPL_BUILD
enum zynq_clk_rclk {mio_clk, emio_clk};
#endif

struct zynq_clk_priv {
	ulong ps_clk_freq;
#ifndef CONFIG_SPL_BUILD
	struct clk gem_emio_clk[2];
#endif
};

static void *zynq_clk_get_register(enum zynq_clk id)
{
	switch (id) {
	case armpll_clk:
		return &slcr_base->arm_pll_ctrl;
	case ddrpll_clk:
		return &slcr_base->ddr_pll_ctrl;
	case iopll_clk:
		return &slcr_base->io_pll_ctrl;
	case lqspi_clk:
		return &slcr_base->lqspi_clk_ctrl;
	case smc_clk:
		return &slcr_base->smc_clk_ctrl;
	case pcap_clk:
		return &slcr_base->pcap_clk_ctrl;
	case sdio0_clk ... sdio1_clk:
		return &slcr_base->sdio_clk_ctrl;
	case uart0_clk ... uart1_clk:
		return &slcr_base->uart_clk_ctrl;
	case spi0_clk ... spi1_clk:
		return &slcr_base->spi_clk_ctrl;
#ifndef CONFIG_SPL_BUILD
	case dci_clk:
		return &slcr_base->dci_clk_ctrl;
	case gem0_clk:
		return &slcr_base->gem0_clk_ctrl;
	case gem1_clk:
		return &slcr_base->gem1_clk_ctrl;
	case fclk0_clk:
		return &slcr_base->fpga0_clk_ctrl;
	case fclk1_clk:
		return &slcr_base->fpga1_clk_ctrl;
	case fclk2_clk:
		return &slcr_base->fpga2_clk_ctrl;
	case fclk3_clk:
		return &slcr_base->fpga3_clk_ctrl;
	case can0_clk ... can1_clk:
		return &slcr_base->can_clk_ctrl;
	case dbg_trc_clk ... dbg_apb_clk:
		/* fall through */
#endif
	default:
		return &slcr_base->dbg_clk_ctrl;
	}
}

static enum zynq_clk zynq_clk_get_cpu_pll(u32 clk_ctrl)
{
	u32 srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >> CLK_CTRL_SRCSEL_SHIFT;

	switch (srcsel) {
	case 2:
		return ddrpll_clk;
	case 3:
		return iopll_clk;
	case 0 ... 1:
	default:
		return armpll_clk;
	}
}

static enum zynq_clk zynq_clk_get_peripheral_pll(u32 clk_ctrl)
{
	u32 srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >> CLK_CTRL_SRCSEL_SHIFT;

	switch (srcsel) {
	case 2:
		return armpll_clk;
	case 3:
		return ddrpll_clk;
	case 0 ... 1:
	default:
		return iopll_clk;
	}
}

static ulong zynq_clk_get_pll_rate(struct zynq_clk_priv *priv, enum zynq_clk id)
{
	u32 clk_ctrl, reset, pwrdwn, mul, bypass;

	clk_ctrl = readl(zynq_clk_get_register(id));

	reset = (clk_ctrl & PLLCTRL_RESET_MASK) >> PLLCTRL_RESET_SHIFT;
	pwrdwn = (clk_ctrl & PLLCTRL_PWRDWN_MASK) >> PLLCTRL_PWRDWN_SHIFT;
	if (reset || pwrdwn)
		return 0;

	bypass = clk_ctrl & PLLCTRL_BPFORCE_MASK;
	if (bypass)
		mul = 1;
	else
		mul = (clk_ctrl & PLLCTRL_FBDIV_MASK) >> PLLCTRL_FBDIV_SHIFT;

	return priv->ps_clk_freq * mul;
}

#ifndef CONFIG_SPL_BUILD
static enum zynq_clk_rclk zynq_clk_get_gem_rclk(enum zynq_clk id)
{
	u32 clk_ctrl, srcsel;

	if (id == gem0_clk)
		clk_ctrl = readl(&slcr_base->gem0_rclk_ctrl);
	else
		clk_ctrl = readl(&slcr_base->gem1_rclk_ctrl);

	srcsel = (clk_ctrl & CLK_CTRL_SRCSEL_MASK) >> CLK_CTRL_SRCSEL_SHIFT;
	if (srcsel)
		return emio_clk;
	else
		return mio_clk;
}
#endif

static ulong zynq_clk_get_cpu_rate(struct zynq_clk_priv *priv, enum zynq_clk id)
{
	u32 clk_621, clk_ctrl, div;
	enum zynq_clk pll;

	clk_ctrl = readl(&slcr_base->arm_clk_ctrl);

	div = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;

	switch (id) {
	case cpu_1x_clk:
		div *= 2;
		/* fall through */
	case cpu_2x_clk:
		clk_621 = readl(&slcr_base->clk_621_true) & 1;
		div *= 2 + clk_621;
		break;
	case cpu_3or2x_clk:
		div *= 2;
		/* fall through */
	case cpu_6or4x_clk:
		break;
	default:
		return 0;
	}

	pll = zynq_clk_get_cpu_pll(clk_ctrl);

	return DIV_ROUND_CLOSEST(zynq_clk_get_pll_rate(priv, pll), div);
}

#ifndef CONFIG_SPL_BUILD
static ulong zynq_clk_get_ddr2x_rate(struct zynq_clk_priv *priv)
{
	u32 clk_ctrl, div;

	clk_ctrl = readl(&slcr_base->ddr_clk_ctrl);

	div = (clk_ctrl & CLK_CTRL_DIV2X_MASK) >> CLK_CTRL_DIV2X_SHIFT;

	return DIV_ROUND_CLOSEST(zynq_clk_get_pll_rate(priv, ddrpll_clk), div);
}
#endif

static ulong zynq_clk_get_ddr3x_rate(struct zynq_clk_priv *priv)
{
	u32 clk_ctrl, div;

	clk_ctrl = readl(&slcr_base->ddr_clk_ctrl);

	div = (clk_ctrl & CLK_CTRL_DIV3X_MASK) >> CLK_CTRL_DIV3X_SHIFT;

	return DIV_ROUND_CLOSEST(zynq_clk_get_pll_rate(priv, ddrpll_clk), div);
}

#ifndef CONFIG_SPL_BUILD
static ulong zynq_clk_get_dci_rate(struct zynq_clk_priv *priv)
{
	u32 clk_ctrl, div0, div1;

	clk_ctrl = readl(&slcr_base->dci_clk_ctrl);

	div0 = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	div1 = (clk_ctrl & CLK_CTRL_DIV1_MASK) >> CLK_CTRL_DIV1_SHIFT;

	return DIV_ROUND_CLOSEST(DIV_ROUND_CLOSEST(
		zynq_clk_get_pll_rate(priv, ddrpll_clk), div0), div1);
}
#endif

static ulong zynq_clk_get_peripheral_rate(struct zynq_clk_priv *priv,
					  enum zynq_clk id, bool two_divs)
{
	enum zynq_clk pll;
	u32 clk_ctrl, div0;
	u32 div1 = 1;

	clk_ctrl = readl(zynq_clk_get_register(id));

	div0 = (clk_ctrl & CLK_CTRL_DIV0_MASK) >> CLK_CTRL_DIV0_SHIFT;
	if (!div0)
		div0 = 1;

#ifndef CONFIG_SPL_BUILD
	if (two_divs) {
		div1 = (clk_ctrl & CLK_CTRL_DIV1_MASK) >> CLK_CTRL_DIV1_SHIFT;
		if (!div1)
			div1 = 1;
	}
#endif

	pll = zynq_clk_get_peripheral_pll(clk_ctrl);

	return
		DIV_ROUND_CLOSEST(
			DIV_ROUND_CLOSEST(
				zynq_clk_get_pll_rate(priv, pll), div0),
			div1);
}

#ifndef CONFIG_SPL_BUILD
static ulong zynq_clk_get_gem_rate(struct zynq_clk_priv *priv, enum zynq_clk id)
{
	struct clk *parent;

	if (zynq_clk_get_gem_rclk(id) == mio_clk)
		return zynq_clk_get_peripheral_rate(priv, id, true);

	parent = &priv->gem_emio_clk[id - gem0_clk];
	if (parent->dev)
		return clk_get_rate(parent);

	debug("%s: gem%d emio rx clock source unknown\n", __func__,
	      id - gem0_clk);

	return -ENOSYS;
}

static unsigned long zynq_clk_calc_peripheral_two_divs(ulong rate,
						       ulong pll_rate,
						       u32 *div0, u32 *div1)
{
	long new_err, best_err = (long)(~0UL >> 1);
	ulong new_rate, best_rate = 0;
	u32 d0, d1;

	for (d0 = 1; d0 <= ZYNQ_CLK_MAXDIV; d0++) {
		for (d1 = 1; d1 <= ZYNQ_CLK_MAXDIV >> 1; d1++) {
			new_rate = DIV_ROUND_CLOSEST(
					DIV_ROUND_CLOSEST(pll_rate, d0), d1);
			new_err = abs(new_rate - rate);

			if (new_err < best_err) {
				*div0 = d0;
				*div1 = d1;
				best_err = new_err;
				best_rate = new_rate;
			}
		}
	}

	return best_rate;
}

static ulong zynq_clk_set_peripheral_rate(struct zynq_clk_priv *priv,
					  enum zynq_clk id, ulong rate,
					  bool two_divs)
{
	enum zynq_clk pll;
	u32 clk_ctrl, div0 = 0, div1 = 0;
	ulong pll_rate, new_rate;
	u32 *reg;

	reg = zynq_clk_get_register(id);
	clk_ctrl = readl(reg);

	pll = zynq_clk_get_peripheral_pll(clk_ctrl);
	pll_rate = zynq_clk_get_pll_rate(priv, pll);
	clk_ctrl &= ~CLK_CTRL_DIV0_MASK;
	if (two_divs) {
		clk_ctrl &= ~CLK_CTRL_DIV1_MASK;
		new_rate = zynq_clk_calc_peripheral_two_divs(rate, pll_rate,
				&div0, &div1);
		clk_ctrl |= div1 << CLK_CTRL_DIV1_SHIFT;
	} else {
		div0 = DIV_ROUND_CLOSEST(pll_rate, rate);
		if (div0 > ZYNQ_CLK_MAXDIV)
			div0 = ZYNQ_CLK_MAXDIV;
		new_rate = DIV_ROUND_CLOSEST(rate, div0);
	}
	clk_ctrl |= div0 << CLK_CTRL_DIV0_SHIFT;

	zynq_slcr_unlock();
	writel(clk_ctrl, reg);
	zynq_slcr_lock();

	return new_rate;
}

static ulong zynq_clk_set_gem_rate(struct zynq_clk_priv *priv, enum zynq_clk id,
				   ulong rate)
{
	struct clk *parent;

	if (zynq_clk_get_gem_rclk(id) == mio_clk)
		return zynq_clk_set_peripheral_rate(priv, id, rate, true);

	parent = &priv->gem_emio_clk[id - gem0_clk];
	if (parent->dev)
		return clk_set_rate(parent, rate);

	debug("%s: gem%d emio rx clock source unknown\n", __func__,
	      id - gem0_clk);

	return -ENOSYS;
}
#endif

#ifndef CONFIG_SPL_BUILD
static ulong zynq_clk_get_rate(struct clk *clk)
{
	struct zynq_clk_priv *priv = dev_get_priv(clk->dev);
	enum zynq_clk id = clk->id;
	bool two_divs = false;

	switch (id) {
	case armpll_clk ... iopll_clk:
		return zynq_clk_get_pll_rate(priv, id);
	case cpu_6or4x_clk ... cpu_1x_clk:
		return zynq_clk_get_cpu_rate(priv, id);
	case ddr2x_clk:
		return zynq_clk_get_ddr2x_rate(priv);
	case ddr3x_clk:
		return zynq_clk_get_ddr3x_rate(priv);
	case dci_clk:
		return zynq_clk_get_dci_rate(priv);
	case gem0_clk ... gem1_clk:
		return zynq_clk_get_gem_rate(priv, id);
	case fclk0_clk ... can1_clk:
		two_divs = true;
		/* fall through */
	case dbg_trc_clk ... dbg_apb_clk:
	case lqspi_clk ... pcap_clk:
	case sdio0_clk ... spi1_clk:
		return zynq_clk_get_peripheral_rate(priv, id, two_divs);
	case dma_clk:
		return zynq_clk_get_cpu_rate(priv, cpu_2x_clk);
	case usb0_aper_clk ... swdt_clk:
		return zynq_clk_get_cpu_rate(priv, cpu_1x_clk);
	default:
		return -ENXIO;
	}
}

static ulong zynq_clk_set_rate(struct clk *clk, ulong rate)
{
	struct zynq_clk_priv *priv = dev_get_priv(clk->dev);
	enum zynq_clk id = clk->id;
	bool two_divs = false;

	switch (id) {
	case gem0_clk ... gem1_clk:
		return zynq_clk_set_gem_rate(priv, id, rate);
	case fclk0_clk ... can1_clk:
		two_divs = true;
		/* fall through */
	case lqspi_clk ... pcap_clk:
	case sdio0_clk ... spi1_clk:
	case dbg_trc_clk ... dbg_apb_clk:
		return zynq_clk_set_peripheral_rate(priv, id, rate, two_divs);
	default:
		return -ENXIO;
	}
}
#else
static ulong zynq_clk_get_rate(struct clk *clk)
{
	struct zynq_clk_priv *priv = dev_get_priv(clk->dev);
	enum zynq_clk id = clk->id;

	switch (id) {
	case cpu_6or4x_clk ... cpu_1x_clk:
		return zynq_clk_get_cpu_rate(priv, id);
	case ddr3x_clk:
		return zynq_clk_get_ddr3x_rate(priv);
	case lqspi_clk ... pcap_clk:
	case sdio0_clk ... spi1_clk:
		return zynq_clk_get_peripheral_rate(priv, id, 0);
	case i2c0_aper_clk ... i2c1_aper_clk:
		return zynq_clk_get_cpu_rate(priv, cpu_1x_clk);
	default:
		return -ENXIO;
	}
}
#endif

static struct clk_ops zynq_clk_ops = {
	.get_rate = zynq_clk_get_rate,
#ifndef CONFIG_SPL_BUILD
	.set_rate = zynq_clk_set_rate,
#endif
};

static int zynq_clk_probe(struct udevice *dev)
{
	struct zynq_clk_priv *priv = dev_get_priv(dev);
#ifndef CONFIG_SPL_BUILD
	unsigned int i;
	char name[16];
	int ret;

	for (i = 0; i < 2; i++) {
		sprintf(name, "gem%d_emio_clk", i);
		ret = clk_get_by_name(dev, name, &priv->gem_emio_clk[i]);
		if (ret < 0 && ret != -ENODATA) {
			dev_err(dev, "failed to get %s clock\n", name);
			return ret;
		}
	}
#endif

	priv->ps_clk_freq = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
					    "ps-clk-frequency", 33333333UL);

	return 0;
}

static const struct udevice_id zynq_clk_ids[] = {
	{ .compatible = "xlnx,ps7-clkc"},
	{}
};

U_BOOT_DRIVER(zynq_clk) = {
	.name		= "zynq_clk",
	.id		= UCLASS_CLK,
	.of_match	= zynq_clk_ids,
	.ops		= &zynq_clk_ops,
	.priv_auto_alloc_size = sizeof(struct zynq_clk_priv),
	.probe		= zynq_clk_probe,
};
