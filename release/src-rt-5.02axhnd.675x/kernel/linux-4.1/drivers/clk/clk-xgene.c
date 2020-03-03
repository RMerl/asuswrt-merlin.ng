/*
 * clk-xgene.c - AppliedMicro X-Gene Clock Interface
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Loc Ho <lho@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <asm/setup.h>

/* Register SCU_PCPPLL bit fields */
#define N_DIV_RD(src)			(((src) & 0x000001ff))

/* Register SCU_SOCPLL bit fields */
#define CLKR_RD(src)			(((src) & 0x07000000)>>24)
#define CLKOD_RD(src)			(((src) & 0x00300000)>>20)
#define REGSPEC_RESET_F1_MASK		0x00010000
#define CLKF_RD(src)			(((src) & 0x000001ff))

#define XGENE_CLK_DRIVER_VER		"0.1"

static DEFINE_SPINLOCK(clk_lock);

static inline u32 xgene_clk_read(void *csr)
{
	return readl_relaxed(csr);
}

static inline void xgene_clk_write(u32 data, void *csr)
{
	return writel_relaxed(data, csr);
}

/* PLL Clock */
enum xgene_pll_type {
	PLL_TYPE_PCP = 0,
	PLL_TYPE_SOC = 1,
};

struct xgene_clk_pll {
	struct clk_hw	hw;
	const char	*name;
	void __iomem	*reg;
	spinlock_t	*lock;
	u32		pll_offset;
	enum xgene_pll_type	type;
};

#define to_xgene_clk_pll(_hw) container_of(_hw, struct xgene_clk_pll, hw)

static int xgene_clk_pll_is_enabled(struct clk_hw *hw)
{
	struct xgene_clk_pll *pllclk = to_xgene_clk_pll(hw);
	u32 data;

	data = xgene_clk_read(pllclk->reg + pllclk->pll_offset);
	pr_debug("%s pll %s\n", pllclk->name,
		data & REGSPEC_RESET_F1_MASK ? "disabled" : "enabled");

	return data & REGSPEC_RESET_F1_MASK ? 0 : 1;
}

static unsigned long xgene_clk_pll_recalc_rate(struct clk_hw *hw,
				unsigned long parent_rate)
{
	struct xgene_clk_pll *pllclk = to_xgene_clk_pll(hw);
	unsigned long fref;
	unsigned long fvco;
	u32 pll;
	u32 nref;
	u32 nout;
	u32 nfb;

	pll = xgene_clk_read(pllclk->reg + pllclk->pll_offset);

	if (pllclk->type == PLL_TYPE_PCP) {
		/*
		 * PLL VCO = Reference clock * NF
		 * PCP PLL = PLL_VCO / 2
		 */
		nout = 2;
		fvco = parent_rate * (N_DIV_RD(pll) + 4);
	} else {
		/*
		 * Fref = Reference Clock / NREF;
		 * Fvco = Fref * NFB;
		 * Fout = Fvco / NOUT;
		 */
		nref = CLKR_RD(pll) + 1;
		nout = CLKOD_RD(pll) + 1;
		nfb = CLKF_RD(pll);
		fref = parent_rate / nref;
		fvco = fref * nfb;
	}
	pr_debug("%s pll recalc rate %ld parent %ld\n", pllclk->name,
		fvco / nout, parent_rate);

	return fvco / nout;
}

const struct clk_ops xgene_clk_pll_ops = {
	.is_enabled = xgene_clk_pll_is_enabled,
	.recalc_rate = xgene_clk_pll_recalc_rate,
};

static struct clk *xgene_register_clk_pll(struct device *dev,
	const char *name, const char *parent_name,
	unsigned long flags, void __iomem *reg, u32 pll_offset,
	u32 type, spinlock_t *lock)
{
	struct xgene_clk_pll *apmclk;
	struct clk *clk;
	struct clk_init_data init;

	/* allocate the APM clock structure */
	apmclk = kzalloc(sizeof(*apmclk), GFP_KERNEL);
	if (!apmclk) {
		pr_err("%s: could not allocate APM clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.ops = &xgene_clk_pll_ops;
	init.flags = flags;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.num_parents = parent_name ? 1 : 0;

	apmclk->name = name;
	apmclk->reg = reg;
	apmclk->lock = lock;
	apmclk->pll_offset = pll_offset;
	apmclk->type = type;
	apmclk->hw.init = &init;

	/* Register the clock */
	clk = clk_register(dev, &apmclk->hw);
	if (IS_ERR(clk)) {
		pr_err("%s: could not register clk %s\n", __func__, name);
		kfree(apmclk);
		return NULL;
	}
	return clk;
}

static void xgene_pllclk_init(struct device_node *np, enum xgene_pll_type pll_type)
{
        const char *clk_name = np->full_name;
        struct clk *clk;
        void *reg;

        reg = of_iomap(np, 0);
        if (reg == NULL) {
                pr_err("Unable to map CSR register for %s\n", np->full_name);
                return;
        }
        of_property_read_string(np, "clock-output-names", &clk_name);
        clk = xgene_register_clk_pll(NULL,
                        clk_name, of_clk_get_parent_name(np, 0),
                        CLK_IS_ROOT, reg, 0, pll_type, &clk_lock);
        if (!IS_ERR(clk)) {
                of_clk_add_provider(np, of_clk_src_simple_get, clk);
                clk_register_clkdev(clk, clk_name, NULL);
                pr_debug("Add %s clock PLL\n", clk_name);
        }
}

static void xgene_socpllclk_init(struct device_node *np)
{
	xgene_pllclk_init(np, PLL_TYPE_SOC);
}

static void xgene_pcppllclk_init(struct device_node *np)
{
	xgene_pllclk_init(np, PLL_TYPE_PCP);
}

/* IP Clock */
struct xgene_dev_parameters {
	void __iomem *csr_reg;		/* CSR for IP clock */
	u32 reg_clk_offset;		/* Offset to clock enable CSR */
	u32 reg_clk_mask;		/* Mask bit for clock enable */
	u32 reg_csr_offset;		/* Offset to CSR reset */
	u32 reg_csr_mask;		/* Mask bit for disable CSR reset */
	void __iomem *divider_reg;	/* CSR for divider */
	u32 reg_divider_offset;		/* Offset to divider register */
	u32 reg_divider_shift;		/* Bit shift to divider field */
	u32 reg_divider_width;		/* Width of the bit to divider field */
};

struct xgene_clk {
	struct clk_hw	hw;
	const char	*name;
	spinlock_t	*lock;
	struct xgene_dev_parameters	param;
};

#define to_xgene_clk(_hw) container_of(_hw, struct xgene_clk, hw)

static int xgene_clk_enable(struct clk_hw *hw)
{
	struct xgene_clk *pclk = to_xgene_clk(hw);
	unsigned long flags = 0;
	u32 data;

	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);

	if (pclk->param.csr_reg != NULL) {
		pr_debug("%s clock enabled\n", pclk->name);
		/* First enable the clock */
		data = xgene_clk_read(pclk->param.csr_reg +
					pclk->param.reg_clk_offset);
		data |= pclk->param.reg_clk_mask;
		xgene_clk_write(data, pclk->param.csr_reg +
					pclk->param.reg_clk_offset);
		pr_debug("%s clock PADDR base 0x%016LX clk offset 0x%08X mask 0x%08X value 0x%08X\n",
			pclk->name, __pa(pclk->param.csr_reg),
			pclk->param.reg_clk_offset, pclk->param.reg_clk_mask,
			data);

		/* Second enable the CSR */
		data = xgene_clk_read(pclk->param.csr_reg +
					pclk->param.reg_csr_offset);
		data &= ~pclk->param.reg_csr_mask;
		xgene_clk_write(data, pclk->param.csr_reg +
					pclk->param.reg_csr_offset);
		pr_debug("%s CSR RESET PADDR base 0x%016LX csr offset 0x%08X mask 0x%08X value 0x%08X\n",
			pclk->name, __pa(pclk->param.csr_reg),
			pclk->param.reg_csr_offset, pclk->param.reg_csr_mask,
			data);
	}

	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);

	return 0;
}

static void xgene_clk_disable(struct clk_hw *hw)
{
	struct xgene_clk *pclk = to_xgene_clk(hw);
	unsigned long flags = 0;
	u32 data;

	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);

	if (pclk->param.csr_reg != NULL) {
		pr_debug("%s clock disabled\n", pclk->name);
		/* First put the CSR in reset */
		data = xgene_clk_read(pclk->param.csr_reg +
					pclk->param.reg_csr_offset);
		data |= pclk->param.reg_csr_mask;
		xgene_clk_write(data, pclk->param.csr_reg +
					pclk->param.reg_csr_offset);

		/* Second disable the clock */
		data = xgene_clk_read(pclk->param.csr_reg +
					pclk->param.reg_clk_offset);
		data &= ~pclk->param.reg_clk_mask;
		xgene_clk_write(data, pclk->param.csr_reg +
					pclk->param.reg_clk_offset);
	}

	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);
}

static int xgene_clk_is_enabled(struct clk_hw *hw)
{
	struct xgene_clk *pclk = to_xgene_clk(hw);
	u32 data = 0;

	if (pclk->param.csr_reg != NULL) {
		pr_debug("%s clock checking\n", pclk->name);
		data = xgene_clk_read(pclk->param.csr_reg +
					pclk->param.reg_clk_offset);
		pr_debug("%s clock is %s\n", pclk->name,
			data & pclk->param.reg_clk_mask ? "enabled" :
							"disabled");
	}

	if (pclk->param.csr_reg == NULL)
		return 1;
	return data & pclk->param.reg_clk_mask ? 1 : 0;
}

static unsigned long xgene_clk_recalc_rate(struct clk_hw *hw,
				unsigned long parent_rate)
{
	struct xgene_clk *pclk = to_xgene_clk(hw);
	u32 data;

	if (pclk->param.divider_reg) {
		data = xgene_clk_read(pclk->param.divider_reg +
					pclk->param.reg_divider_offset);
		data >>= pclk->param.reg_divider_shift;
		data &= (1 << pclk->param.reg_divider_width) - 1;

		pr_debug("%s clock recalc rate %ld parent %ld\n",
			pclk->name, parent_rate / data, parent_rate);
		return parent_rate / data;
	} else {
		pr_debug("%s clock recalc rate %ld parent %ld\n",
			pclk->name, parent_rate, parent_rate);
		return parent_rate;
	}
}

static int xgene_clk_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	struct xgene_clk *pclk = to_xgene_clk(hw);
	unsigned long flags = 0;
	u32 data;
	u32 divider;
	u32 divider_save;

	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);

	if (pclk->param.divider_reg) {
		/* Let's compute the divider */
		if (rate > parent_rate)
			rate = parent_rate;
		divider_save = divider = parent_rate / rate; /* Rounded down */
		divider &= (1 << pclk->param.reg_divider_width) - 1;
		divider <<= pclk->param.reg_divider_shift;

		/* Set new divider */
		data = xgene_clk_read(pclk->param.divider_reg +
				pclk->param.reg_divider_offset);
		data &= ~((1 << pclk->param.reg_divider_width) - 1);
		data |= divider;
		xgene_clk_write(data, pclk->param.divider_reg +
					pclk->param.reg_divider_offset);
		pr_debug("%s clock set rate %ld\n", pclk->name,
			parent_rate / divider_save);
	} else {
		divider_save = 1;
	}

	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);

	return parent_rate / divider_save;
}

static long xgene_clk_round_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long *prate)
{
	struct xgene_clk *pclk = to_xgene_clk(hw);
	unsigned long parent_rate = *prate;
	u32 divider;

	if (pclk->param.divider_reg) {
		/* Let's compute the divider */
		if (rate > parent_rate)
			rate = parent_rate;
		divider = parent_rate / rate;   /* Rounded down */
	} else {
		divider = 1;
	}

	return parent_rate / divider;
}

const struct clk_ops xgene_clk_ops = {
	.enable = xgene_clk_enable,
	.disable = xgene_clk_disable,
	.is_enabled = xgene_clk_is_enabled,
	.recalc_rate = xgene_clk_recalc_rate,
	.set_rate = xgene_clk_set_rate,
	.round_rate = xgene_clk_round_rate,
};

static struct clk *xgene_register_clk(struct device *dev,
		const char *name, const char *parent_name,
		struct xgene_dev_parameters *parameters, spinlock_t *lock)
{
	struct xgene_clk *apmclk;
	struct clk *clk;
	struct clk_init_data init;
	int rc;

	/* allocate the APM clock structure */
	apmclk = kzalloc(sizeof(*apmclk), GFP_KERNEL);
	if (!apmclk) {
		pr_err("%s: could not allocate APM clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.ops = &xgene_clk_ops;
	init.flags = 0;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.num_parents = parent_name ? 1 : 0;

	apmclk->name = name;
	apmclk->lock = lock;
	apmclk->hw.init = &init;
	apmclk->param = *parameters;

	/* Register the clock */
	clk = clk_register(dev, &apmclk->hw);
	if (IS_ERR(clk)) {
		pr_err("%s: could not register clk %s\n", __func__, name);
		kfree(apmclk);
		return clk;
	}

	/* Register the clock for lookup */
	rc = clk_register_clkdev(clk, name, NULL);
	if (rc != 0) {
		pr_err("%s: could not register lookup clk %s\n",
			__func__, name);
	}
	return clk;
}

static void __init xgene_devclk_init(struct device_node *np)
{
	const char *clk_name = np->full_name;
	struct clk *clk;
	struct resource res;
	int rc;
	struct xgene_dev_parameters parameters;
	int i;

	/* Check if the entry is disabled */
        if (!of_device_is_available(np))
                return;

	/* Parse the DTS register for resource */
	parameters.csr_reg = NULL;
	parameters.divider_reg = NULL;
	for (i = 0; i < 2; i++) {
		void *map_res;
		rc = of_address_to_resource(np, i, &res);
		if (rc != 0) {
			if (i == 0) {
				pr_err("no DTS register for %s\n", 
					np->full_name);
				return;
			}
			break;
		}
		map_res = of_iomap(np, i);
		if (map_res == NULL) {
			pr_err("Unable to map resource %d for %s\n",
				i, np->full_name);
			goto err;
		}
		if (strcmp(res.name, "div-reg") == 0)
			parameters.divider_reg = map_res;
		else /* if (strcmp(res->name, "csr-reg") == 0) */
			parameters.csr_reg = map_res;
	}
	if (of_property_read_u32(np, "csr-offset", &parameters.reg_csr_offset))
		parameters.reg_csr_offset = 0;
	if (of_property_read_u32(np, "csr-mask", &parameters.reg_csr_mask))
		parameters.reg_csr_mask = 0xF;
	if (of_property_read_u32(np, "enable-offset",
				&parameters.reg_clk_offset))
		parameters.reg_clk_offset = 0x8;
	if (of_property_read_u32(np, "enable-mask", &parameters.reg_clk_mask))
		parameters.reg_clk_mask = 0xF;
	if (of_property_read_u32(np, "divider-offset",
				&parameters.reg_divider_offset))
		parameters.reg_divider_offset = 0;
	if (of_property_read_u32(np, "divider-width",
				&parameters.reg_divider_width))
		parameters.reg_divider_width = 0;
	if (of_property_read_u32(np, "divider-shift",
				&parameters.reg_divider_shift))
		parameters.reg_divider_shift = 0;
	of_property_read_string(np, "clock-output-names", &clk_name);

	clk = xgene_register_clk(NULL, clk_name,
		of_clk_get_parent_name(np, 0), &parameters, &clk_lock);
	if (IS_ERR(clk))
		goto err;
	pr_debug("Add %s clock\n", clk_name);
	rc = of_clk_add_provider(np, of_clk_src_simple_get, clk);
	if (rc != 0)
		pr_err("%s: could register provider clk %s\n", __func__,
			np->full_name);

	return;

err:
	if (parameters.csr_reg)
		iounmap(parameters.csr_reg);
	if (parameters.divider_reg)
		iounmap(parameters.divider_reg);
}

CLK_OF_DECLARE(xgene_socpll_clock, "apm,xgene-socpll-clock", xgene_socpllclk_init);
CLK_OF_DECLARE(xgene_pcppll_clock, "apm,xgene-pcppll-clock", xgene_pcppllclk_init);
CLK_OF_DECLARE(xgene_dev_clock, "apm,xgene-device-clock", xgene_devclk_init);
