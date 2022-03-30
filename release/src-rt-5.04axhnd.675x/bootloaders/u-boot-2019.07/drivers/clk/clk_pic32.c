// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <div64.h>
#include <wait_bit.h>
#include <dm/lists.h>
#include <asm/io.h>
#include <mach/pic32.h>
#include <dt-bindings/clock/microchip,clock.h>

DECLARE_GLOBAL_DATA_PTR;

/* Primary oscillator */
#define SYS_POSC_CLK_HZ	24000000

/* FRC clk rate */
#define SYS_FRC_CLK_HZ	8000000

/* Clock Registers */
#define OSCCON		0x0000
#define OSCTUNE		0x0010
#define SPLLCON		0x0020
#define REFO1CON	0x0080
#define REFO1TRIM	0x0090
#define PB1DIV		0x0140

/* SPLL */
#define ICLK_MASK	0x00000080
#define PLLIDIV_MASK	0x00000007
#define PLLODIV_MASK	0x00000007
#define CUROSC_MASK	0x00000007
#define PLLMUL_MASK	0x0000007F
#define FRCDIV_MASK	0x00000007

/* PBCLK */
#define PBDIV_MASK	0x00000007

/* SYSCLK MUX */
#define SCLK_SRC_FRC1	0
#define SCLK_SRC_SPLL	1
#define SCLK_SRC_POSC	2
#define SCLK_SRC_FRC2	7

/* Reference Oscillator Control Reg fields */
#define REFO_SEL_MASK	0x0f
#define REFO_SEL_SHIFT	0
#define REFO_ACTIVE	BIT(8)
#define REFO_DIVSW_EN	BIT(9)
#define REFO_OE		BIT(12)
#define REFO_ON		BIT(15)
#define REFO_DIV_SHIFT	16
#define REFO_DIV_MASK	0x7fff

/* Reference Oscillator Trim Register Fields */
#define REFO_TRIM_REG	0x10
#define REFO_TRIM_MASK	0x1ff
#define REFO_TRIM_SHIFT	23
#define REFO_TRIM_MAX	511

#define ROCLK_SRC_SCLK		0x0
#define ROCLK_SRC_SPLL		0x7
#define ROCLK_SRC_ROCLKI	0x8

/* Memory PLL */
#define MPLL_IDIV		0x3f
#define MPLL_MULT		0xff
#define MPLL_ODIV1		0x7
#define MPLL_ODIV2		0x7
#define MPLL_VREG_RDY		BIT(23)
#define MPLL_RDY		BIT(31)
#define MPLL_IDIV_SHIFT		0
#define MPLL_MULT_SHIFT		8
#define MPLL_ODIV1_SHIFT	24
#define MPLL_ODIV2_SHIFT	27
#define MPLL_IDIV_INIT		0x03
#define MPLL_MULT_INIT		0x32
#define MPLL_ODIV1_INIT		0x02
#define MPLL_ODIV2_INIT		0x01

struct pic32_clk_priv {
	void __iomem *iobase;
	void __iomem *syscfg_base;
};

static ulong pic32_get_pll_rate(struct pic32_clk_priv *priv)
{
	u32 iclk, idiv, odiv, mult;
	ulong plliclk, v;

	v = readl(priv->iobase + SPLLCON);
	iclk = (v & ICLK_MASK);
	idiv = ((v >> 8) & PLLIDIV_MASK) + 1;
	odiv = ((v >> 24) & PLLODIV_MASK);
	mult = ((v >> 16) & PLLMUL_MASK) + 1;

	plliclk = iclk ? SYS_FRC_CLK_HZ : SYS_POSC_CLK_HZ;

	if (odiv < 2)
		odiv = 2;
	else if (odiv < 5)
		odiv = (1 << odiv);
	else
		odiv = 32;

	return ((plliclk / idiv) * mult) / odiv;
}

static ulong pic32_get_sysclk(struct pic32_clk_priv *priv)
{
	ulong v;
	ulong hz;
	ulong div, frcdiv;
	ulong curr_osc;

	/* get clk source */
	v = readl(priv->iobase + OSCCON);
	curr_osc = (v >> 12) & CUROSC_MASK;
	switch (curr_osc) {
	case SCLK_SRC_FRC1:
	case SCLK_SRC_FRC2:
		frcdiv = ((v >> 24) & FRCDIV_MASK);
		div = ((1 << frcdiv) + 1) + (128 * (frcdiv == 7));
		hz = SYS_FRC_CLK_HZ / div;
		break;

	case SCLK_SRC_SPLL:
		hz = pic32_get_pll_rate(priv);
		break;

	case SCLK_SRC_POSC:
		hz = SYS_POSC_CLK_HZ;
		break;

	default:
		hz = 0;
		printf("clk: unknown sclk_src.\n");
		break;
	}

	return hz;
}

static ulong pic32_get_pbclk(struct pic32_clk_priv *priv, int periph)
{
	void __iomem *reg;
	ulong div, clk_freq;

	WARN_ON((periph < PB1CLK) || (periph > PB7CLK));

	clk_freq = pic32_get_sysclk(priv);

	reg = priv->iobase + PB1DIV + (periph - PB1CLK) * 0x10;
	div = (readl(reg) & PBDIV_MASK) + 1;

	return clk_freq / div;
}

static ulong pic32_get_cpuclk(struct pic32_clk_priv *priv)
{
	return pic32_get_pbclk(priv, PB7CLK);
}

static ulong pic32_set_refclk(struct pic32_clk_priv *priv, int periph,
			      int parent_rate, int rate, int parent_id)
{
	void __iomem *reg;
	u32 div, trim, v;
	u64 frac;

	WARN_ON((periph < REF1CLK) || (periph > REF5CLK));

	/* calculate dividers,
	 *   rate = parent_rate / [2 * (div + (trim / 512))]
	 */
	if (parent_rate <= rate) {
		div = 0;
		trim = 0;
	} else {
		div = parent_rate / (rate << 1);
		frac = parent_rate;
		frac <<= 8;
		do_div(frac, rate);
		frac -= (u64)(div << 9);
		trim = (frac >= REFO_TRIM_MAX) ? REFO_TRIM_MAX : (u32)frac;
	}

	reg = priv->iobase + REFO1CON + (periph - REF1CLK) * 0x20;

	/* disable clk */
	writel(REFO_ON | REFO_OE, reg + _CLR_OFFSET);

	/* wait till previous src change is active */
	wait_for_bit_le32(reg, REFO_DIVSW_EN | REFO_ACTIVE,
			  false, CONFIG_SYS_HZ, false);

	/* parent_id */
	v = readl(reg);
	v &= ~(REFO_SEL_MASK << REFO_SEL_SHIFT);
	v |= (parent_id << REFO_SEL_SHIFT);

	/* apply rodiv */
	v &= ~(REFO_DIV_MASK << REFO_DIV_SHIFT);
	v |= (div << REFO_DIV_SHIFT);
	writel(v, reg);

	/* apply trim */
	v = readl(reg + REFO_TRIM_REG);
	v &= ~(REFO_TRIM_MASK << REFO_TRIM_SHIFT);
	v |= (trim << REFO_TRIM_SHIFT);
	writel(v, reg + REFO_TRIM_REG);

	/* enable clk */
	writel(REFO_ON | REFO_OE, reg + _SET_OFFSET);

	/* switch divider */
	writel(REFO_DIVSW_EN, reg + _SET_OFFSET);

	/* wait for divider switching to complete */
	return wait_for_bit_le32(reg, REFO_DIVSW_EN, false,
				 CONFIG_SYS_HZ, false);
}

static ulong pic32_get_refclk(struct pic32_clk_priv *priv, int periph)
{
	u32 rodiv, rotrim, rosel, v, parent_rate;
	void __iomem *reg;
	u64 rate64;

	WARN_ON((periph < REF1CLK) || (periph > REF5CLK));

	reg = priv->iobase + REFO1CON + (periph - REF1CLK) * 0x20;
	v = readl(reg);
	/* get rosel */
	rosel = (v >> REFO_SEL_SHIFT) & REFO_SEL_MASK;
	/* get div */
	rodiv = (v >> REFO_DIV_SHIFT) & REFO_DIV_MASK;

	/* get trim */
	v = readl(reg + REFO_TRIM_REG);
	rotrim = (v >> REFO_TRIM_SHIFT) & REFO_TRIM_MASK;

	if (!rodiv)
		return 0;

	/* get parent rate */
	switch (rosel) {
	case ROCLK_SRC_SCLK:
		parent_rate = pic32_get_cpuclk(priv);
		break;
	case ROCLK_SRC_SPLL:
		parent_rate = pic32_get_pll_rate(priv);
		break;
	default:
		parent_rate = 0;
		break;
	}

	/* Calculation
	 * rate = parent_rate / [2 * (div + (trim / 512))]
	 */
	if (rotrim) {
		rodiv <<= 9;
		rodiv += rotrim;
		rate64 = parent_rate;
		rate64 <<= 8;
		do_div(rate64, rodiv);
		v = (u32)rate64;
	} else {
		v = parent_rate / (rodiv << 1);
	}
	return v;
}

static ulong pic32_get_mpll_rate(struct pic32_clk_priv *priv)
{
	u32 v, idiv, mul;
	u32 odiv1, odiv2;
	u64 rate;

	v = readl(priv->syscfg_base + CFGMPLL);
	idiv = v & MPLL_IDIV;
	mul = (v >> MPLL_MULT_SHIFT) & MPLL_MULT;
	odiv1 = (v >> MPLL_ODIV1_SHIFT) & MPLL_ODIV1;
	odiv2 = (v >> MPLL_ODIV2_SHIFT) & MPLL_ODIV2;

	rate = (SYS_POSC_CLK_HZ / idiv) * mul;
	do_div(rate, odiv1);
	do_div(rate, odiv2);

	return (ulong)rate;
}

static int pic32_mpll_init(struct pic32_clk_priv *priv)
{
	u32 v, mask;

	/* initialize */
	v = (MPLL_IDIV_INIT << MPLL_IDIV_SHIFT) |
	    (MPLL_MULT_INIT << MPLL_MULT_SHIFT) |
	    (MPLL_ODIV1_INIT << MPLL_ODIV1_SHIFT) |
	    (MPLL_ODIV2_INIT << MPLL_ODIV2_SHIFT);

	writel(v, priv->syscfg_base + CFGMPLL);

	/* Wait for ready */
	mask = MPLL_RDY | MPLL_VREG_RDY;
	return wait_for_bit_le32(priv->syscfg_base + CFGMPLL, mask,
				 true, get_tbclk(), false);
}

static void pic32_clk_init(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	struct pic32_clk_priv *priv;
	ulong rate, pll_hz;
	char propname[50];
	int i;

	priv = dev_get_priv(dev);
	pll_hz = pic32_get_pll_rate(priv);

	/* Initialize REFOs as not initialized and enabled on reset. */
	for (i = REF1CLK; i <= REF5CLK; i++) {
		snprintf(propname, sizeof(propname),
			 "microchip,refo%d-frequency", i - REF1CLK + 1);
		rate = fdtdec_get_int(blob, dev_of_offset(dev), propname, 0);
		if (rate)
			pic32_set_refclk(priv, i, pll_hz, rate, ROCLK_SRC_SPLL);
	}

	/* Memory PLL */
	pic32_mpll_init(priv);
}

static ulong pic32_get_rate(struct clk *clk)
{
	struct pic32_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate;

	switch (clk->id) {
	case PB1CLK ... PB7CLK:
		rate = pic32_get_pbclk(priv, clk->id);
		break;
	case REF1CLK ... REF5CLK:
		rate = pic32_get_refclk(priv, clk->id);
		break;
	case PLLCLK:
		rate = pic32_get_pll_rate(priv);
		break;
	case MPLL:
		rate = pic32_get_mpll_rate(priv);
		break;
	default:
		rate = 0;
		break;
	}

	return rate;
}

static ulong pic32_set_rate(struct clk *clk, ulong rate)
{
	struct pic32_clk_priv *priv = dev_get_priv(clk->dev);
	ulong pll_hz;

	switch (clk->id) {
	case REF1CLK ... REF5CLK:
		pll_hz = pic32_get_pll_rate(priv);
		pic32_set_refclk(priv, clk->id, pll_hz, rate, ROCLK_SRC_SPLL);
		break;
	default:
		break;
	}

	return rate;
}

static struct clk_ops pic32_pic32_clk_ops = {
	.set_rate = pic32_set_rate,
	.get_rate = pic32_get_rate,
};

static int pic32_clk_probe(struct udevice *dev)
{
	struct pic32_clk_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	fdt_size_t size;

	addr = fdtdec_get_addr_size(gd->fdt_blob, dev_of_offset(dev), "reg",
				    &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->iobase = ioremap(addr, size);
	if (!priv->iobase)
		return -EINVAL;

	priv->syscfg_base = pic32_get_syscfg_base();

	/* initialize clocks */
	pic32_clk_init(dev);

	return 0;
}

static const struct udevice_id pic32_clk_ids[] = {
	{ .compatible = "microchip,pic32mzda-clk"},
	{}
};

U_BOOT_DRIVER(pic32_clk) = {
	.name		= "pic32_clk",
	.id		= UCLASS_CLK,
	.of_match	= pic32_clk_ids,
	.ops		= &pic32_pic32_clk_ops,
	.probe		= pic32_clk_probe,
	.priv_auto_alloc_size = sizeof(struct pic32_clk_priv),
};
