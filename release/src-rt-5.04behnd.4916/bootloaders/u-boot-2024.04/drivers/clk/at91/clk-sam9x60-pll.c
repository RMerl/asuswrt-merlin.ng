// SPDX-License-Identifier: GPL-2.0+
/*
 * SAM9X60's PLL clock support.
 *
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on drivers/clk/at91/clk-sam9x60-pll.c from Linux.
 *
 */

#include <asm/processor.h>
#include <common.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <linux/clk-provider.h>
#include <linux/clk/at91_pmc.h>
#include <linux/delay.h>

#include "pmc.h"

#define UBOOT_DM_CLK_AT91_SAM9X60_DIV_PLL	"at91-sam9x60-div-pll-clk"
#define UBOOT_DM_CLK_AT91_SAM9X60_FRAC_PLL	"at91-sam9x60-frac-pll-clk"

#define	PMC_PLL_CTRL0_DIV_MSK	GENMASK(7, 0)
#define	PMC_PLL_CTRL1_MUL_MSK	GENMASK(31, 24)
#define PMC_PLL_CTRL1_FRACR_MSK	GENMASK(21, 0)

#define PLL_DIV_MAX		(FIELD_GET(PMC_PLL_CTRL0_DIV_MSK, UINT_MAX) + 1)
#define UPLL_DIV		2
#define PLL_MUL_MAX		(FIELD_GET(PMC_PLL_CTRL1_MUL_MSK, UINT_MAX) + 1)

#define FCORE_MIN		(600000000)
#define FCORE_MAX		(1200000000)

#define PLL_MAX_ID		7

struct sam9x60_pll {
	void __iomem *base;
	const struct clk_pll_characteristics *characteristics;
	const struct clk_pll_layout *layout;
	struct clk clk;
	u8 id;
};

#define to_sam9x60_pll(_clk)	container_of(_clk, struct sam9x60_pll, clk)

static inline bool sam9x60_pll_ready(void __iomem *base, int id)
{
	unsigned int status;

	pmc_read(base, AT91_PMC_PLL_ISR0, &status);

	return !!(status & BIT(id));
}

static long sam9x60_frac_pll_compute_mul_frac(u32 *mul, u32 *frac, ulong rate,
					      ulong parent_rate)
{
	unsigned long tmprate, remainder;
	unsigned long nmul = 0;
	unsigned long nfrac = 0;

	if (rate < FCORE_MIN || rate > FCORE_MAX)
		return -ERANGE;

	/*
	 * Calculate the multiplier associated with the current
	 * divider that provide the closest rate to the requested one.
	 */
	nmul = mult_frac(rate, 1, parent_rate);
	tmprate = mult_frac(parent_rate, nmul, 1);
	remainder = rate - tmprate;

	if (remainder) {
		nfrac = DIV_ROUND_CLOSEST_ULL((u64)remainder * (1 << 22),
					      parent_rate);

		tmprate += DIV_ROUND_CLOSEST_ULL((u64)nfrac * parent_rate,
						 (1 << 22));
	}

	/* Check if resulted rate is valid.  */
	if (tmprate < FCORE_MIN || tmprate > FCORE_MAX)
		return -ERANGE;

	*mul = nmul - 1;
	*frac = nfrac;

	return tmprate;
}

static ulong sam9x60_frac_pll_set_rate(struct clk *clk, ulong rate)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 nmul, cmul, nfrac, cfrac, val;
	bool ready = sam9x60_pll_ready(base, pll->id);
	long ret;

	if (!parent_rate)
		return 0;

	ret = sam9x60_frac_pll_compute_mul_frac(&nmul, &nfrac, rate,
						parent_rate);
	if (ret < 0)
		return 0;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);
	pmc_read(base, AT91_PMC_PLL_CTRL1, &val);
	cmul = (val & pll->layout->mul_mask) >> pll->layout->mul_shift;
	cfrac = (val & pll->layout->frac_mask) >> pll->layout->frac_shift;

	/* Check against current values. */
	if (sam9x60_pll_ready(base, pll->id) &&
	    nmul == cmul && nfrac == cfrac)
		return 0;

	/* Update it to hardware. */
	pmc_write(base, AT91_PMC_PLL_CTRL1,
		  (nmul << pll->layout->mul_shift) |
		  (nfrac << pll->layout->frac_shift));

	pmc_update_bits(base, AT91_PMC_PLL_UPDT,
			AT91_PMC_PLL_UPDT_UPDATE | AT91_PMC_PLL_UPDT_ID_MSK,
			AT91_PMC_PLL_UPDT_UPDATE | pll->id);

	while (ready && !sam9x60_pll_ready(base, pll->id)) {
		debug("waiting for pll %u...\n", pll->id);
		cpu_relax();
	}

	return parent_rate * (nmul + 1) + ((u64)parent_rate * nfrac >> 22);
}

static ulong sam9x60_frac_pll_get_rate(struct clk *clk)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 mul, frac, val;

	if (!parent_rate)
		return 0;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);
	pmc_read(base, AT91_PMC_PLL_CTRL1, &val);
	mul = (val & pll->layout->mul_mask) >> pll->layout->mul_shift;
	frac = (val & pll->layout->frac_mask) >> pll->layout->frac_shift;

	return (parent_rate * (mul + 1) + ((u64)parent_rate * frac >> 22));
}

static int sam9x60_frac_pll_enable(struct clk *clk)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;
	unsigned int val;
	ulong crate;

	crate = sam9x60_frac_pll_get_rate(clk);
	if (crate < FCORE_MIN || crate > FCORE_MAX)
		return -ERANGE;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);
	pmc_read(base, AT91_PMC_PLL_CTRL1, &val);

	if (sam9x60_pll_ready(base, pll->id))
		return 0;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT,
			AT91_PMC_PMM_UPDT_STUPTIM_MSK |
			AT91_PMC_PLL_UPDT_ID_MSK,
			AT91_PMC_PLL_UPDT_STUPTIM(0x3f) | pll->id);

	/* Recommended value for AT91_PMC_PLL_ACR */
	if (pll->characteristics->upll)
		val = AT91_PMC_PLL_ACR_DEFAULT_UPLL;
	else
		val = AT91_PMC_PLL_ACR_DEFAULT_PLLA;
	pmc_write(base, AT91_PMC_PLL_ACR, val);

	if (pll->characteristics->upll) {
		/* Enable the UTMI internal bandgap */
		val |= AT91_PMC_PLL_ACR_UTMIBG;
		pmc_write(base, AT91_PMC_PLL_ACR, val);

		udelay(10);

		/* Enable the UTMI internal regulator */
		val |= AT91_PMC_PLL_ACR_UTMIVR;
		pmc_write(base, AT91_PMC_PLL_ACR, val);

		udelay(10);

		pmc_update_bits(base, AT91_PMC_PLL_UPDT,
				AT91_PMC_PLL_UPDT_UPDATE |
				AT91_PMC_PLL_UPDT_ID_MSK,
				AT91_PMC_PLL_UPDT_UPDATE | pll->id);
	}

	pmc_update_bits(base, AT91_PMC_PLL_CTRL0,
			AT91_PMC_PLL_CTRL0_ENLOCK | AT91_PMC_PLL_CTRL0_ENPLL,
			AT91_PMC_PLL_CTRL0_ENLOCK | AT91_PMC_PLL_CTRL0_ENPLL);

	pmc_update_bits(base, AT91_PMC_PLL_UPDT,
			AT91_PMC_PLL_UPDT_UPDATE | AT91_PMC_PLL_UPDT_ID_MSK,
			AT91_PMC_PLL_UPDT_UPDATE | pll->id);

	while (!sam9x60_pll_ready(base, pll->id)) {
		debug("waiting for pll %u...\n", pll->id);
		cpu_relax();
	}

	return 0;
}

static int sam9x60_frac_pll_disable(struct clk *clk)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);

	pmc_update_bits(base, AT91_PMC_PLL_CTRL0,
			AT91_PMC_PLL_CTRL0_ENPLL, 0);

	if (pll->characteristics->upll)
		pmc_update_bits(base, AT91_PMC_PLL_ACR,
				AT91_PMC_PLL_ACR_UTMIBG |
				AT91_PMC_PLL_ACR_UTMIVR, 0);

	pmc_update_bits(base, AT91_PMC_PLL_UPDT,
			AT91_PMC_PLL_UPDT_UPDATE | AT91_PMC_PLL_UPDT_ID_MSK,
			AT91_PMC_PLL_UPDT_UPDATE | pll->id);

	return 0;
}

static const struct clk_ops sam9x60_frac_pll_ops = {
	.enable = sam9x60_frac_pll_enable,
	.disable = sam9x60_frac_pll_disable,
	.set_rate = sam9x60_frac_pll_set_rate,
	.get_rate = sam9x60_frac_pll_get_rate,
};

static int sam9x60_div_pll_enable(struct clk *clk)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;
	unsigned int val;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);
	pmc_read(base, AT91_PMC_PLL_CTRL0, &val);

	/* Stop if enabled. */
	if (val & pll->layout->endiv_mask)
		return 0;

	pmc_update_bits(base, AT91_PMC_PLL_CTRL0,
			pll->layout->endiv_mask,
			(1 << pll->layout->endiv_shift));

	pmc_update_bits(base, AT91_PMC_PLL_UPDT,
			AT91_PMC_PLL_UPDT_UPDATE | AT91_PMC_PLL_UPDT_ID_MSK,
			AT91_PMC_PLL_UPDT_UPDATE | pll->id);

	while (!sam9x60_pll_ready(base, pll->id)) {
		debug("waiting for pll %u...\n", pll->id);
		cpu_relax();
	}

	return 0;
}

static int sam9x60_div_pll_disable(struct clk *clk)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);

	pmc_update_bits(base, AT91_PMC_PLL_CTRL0,
			pll->layout->endiv_mask, 0);

	pmc_update_bits(base, AT91_PMC_PLL_UPDT,
			AT91_PMC_PLL_UPDT_UPDATE | AT91_PMC_PLL_UPDT_ID_MSK,
			AT91_PMC_PLL_UPDT_UPDATE | pll->id);

	return 0;
}

static ulong sam9x60_div_pll_set_rate(struct clk *clk, ulong rate)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;
	const struct clk_pll_characteristics *characteristics =
							pll->characteristics;
	ulong parent_rate = clk_get_parent_rate(clk);
	u8 div = DIV_ROUND_CLOSEST_ULL(parent_rate, rate) - 1;
	ulong req_rate = parent_rate / (div + 1);
	bool ready = sam9x60_pll_ready(base, pll->id);
	u32 val;

	if (!parent_rate || div > pll->layout->div_mask ||
	    req_rate < characteristics->output[0].min ||
	    req_rate > characteristics->output[0].max)
		return 0;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);
	pmc_read(base, AT91_PMC_PLL_CTRL0, &val);
	/* Compare against current value. */
	if (div == ((val & pll->layout->div_mask) >> pll->layout->div_shift))
		return 0;

	/* Update it to hardware. */
	pmc_update_bits(base, AT91_PMC_PLL_CTRL0,
			pll->layout->div_mask,
			div << pll->layout->div_shift);

	pmc_update_bits(base, AT91_PMC_PLL_UPDT,
			AT91_PMC_PLL_UPDT_UPDATE | AT91_PMC_PLL_UPDT_ID_MSK,
			AT91_PMC_PLL_UPDT_UPDATE | pll->id);

	while (ready && !sam9x60_pll_ready(base, pll->id)) {
		debug("waiting for pll %u...\n", pll->id);
		cpu_relax();
	}

	return req_rate;
}

static ulong sam9x60_div_pll_get_rate(struct clk *clk)
{
	struct sam9x60_pll *pll = to_sam9x60_pll(clk);
	void __iomem *base = pll->base;
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 val;
	u8 div;

	if (!parent_rate)
		return 0;

	pmc_update_bits(base, AT91_PMC_PLL_UPDT, AT91_PMC_PLL_UPDT_ID_MSK,
			pll->id);

	pmc_read(base, AT91_PMC_PLL_CTRL0, &val);

	div = (val & pll->layout->div_mask) >> pll->layout->div_shift;

	return parent_rate / (div + 1);
}

static const struct clk_ops sam9x60_div_pll_ops = {
	.enable = sam9x60_div_pll_enable,
	.disable = sam9x60_div_pll_disable,
	.set_rate = sam9x60_div_pll_set_rate,
	.get_rate = sam9x60_div_pll_get_rate,
};

static struct clk *
sam9x60_clk_register_pll(void __iomem *base, const char *type,
			 const char *name, const char *parent_name, u8 id,
			 const struct clk_pll_characteristics *characteristics,
			 const struct clk_pll_layout *layout, u32 flags)
{
	struct sam9x60_pll *pll;
	struct clk *clk;
	int ret;

	if (!base || !type || !name || !parent_name || !characteristics ||
	    !layout || id > PLL_MAX_ID)
		return ERR_PTR(-EINVAL);

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->id = id;
	pll->characteristics = characteristics;
	pll->layout = layout;
	pll->base = base;
	clk = &pll->clk;
	clk->flags = flags;

	ret = clk_register(clk, type, name, parent_name);
	if (ret) {
		kfree(pll);
		clk = ERR_PTR(ret);
	}

	return clk;
}

struct clk *
sam9x60_clk_register_div_pll(void __iomem *base, const char *name,
			     const char *parent_name, u8 id,
			     const struct clk_pll_characteristics *characteristics,
			     const struct clk_pll_layout *layout, bool critical)
{
	return sam9x60_clk_register_pll(base,
		UBOOT_DM_CLK_AT91_SAM9X60_DIV_PLL, name, parent_name, id,
		characteristics, layout,
		CLK_GET_RATE_NOCACHE | (critical ? CLK_IS_CRITICAL : 0));
}

struct clk *
sam9x60_clk_register_frac_pll(void __iomem *base, const char *name,
			      const char *parent_name, u8 id,
			      const struct clk_pll_characteristics *characteristics,
			      const struct clk_pll_layout *layout, bool critical)
{
	return sam9x60_clk_register_pll(base,
		UBOOT_DM_CLK_AT91_SAM9X60_FRAC_PLL, name, parent_name, id,
		characteristics, layout,
		CLK_GET_RATE_NOCACHE | (critical ? CLK_IS_CRITICAL : 0));
}

U_BOOT_DRIVER(at91_sam9x60_div_pll_clk) = {
	.name = UBOOT_DM_CLK_AT91_SAM9X60_DIV_PLL,
	.id = UCLASS_CLK,
	.ops = &sam9x60_div_pll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(at91_sam9x60_frac_pll_clk) = {
	.name = UBOOT_DM_CLK_AT91_SAM9X60_FRAC_PLL,
	.id = UCLASS_CLK,
	.ops = &sam9x60_frac_pll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
