// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/uclass.h>
#include <clk.h>
#include "clk.h"
#include <linux/err.h>

#define UBOOT_DM_CLK_IMX_PLLV3_GENERIC	"imx_clk_pllv3_generic"
#define UBOOT_DM_CLK_IMX_PLLV3_SYS	"imx_clk_pllv3_sys"
#define UBOOT_DM_CLK_IMX_PLLV3_USB	"imx_clk_pllv3_usb"
#define UBOOT_DM_CLK_IMX_PLLV3_AV	"imx_clk_pllv3_av"
#define UBOOT_DM_CLK_IMX_PLLV3_ENET     "imx_clk_pllv3_enet"
#define UBOOT_DM_CLK_IMX_PLLV3_GENV2	"imx_clk_pllv3_genericv2"

#define PLL_NUM_OFFSET		0x10
#define PLL_DENOM_OFFSET	0x20

#define BM_PLL_POWER		(0x1 << 12)
#define BM_PLL_POWER_V2		(0x1 << 21)
#define BM_PLL_ENABLE		(0x1 << 13)
#define BM_PLL_LOCK		(0x1 << 31)
#define BM_PLL_LOCK_V2		(0x1 << 29)

struct clk_pllv3 {
	struct clk	clk;
	void __iomem	*base;
	u32		power_bit;
	bool		powerup_set;
	u32		lock_bit;
	u32		enable_bit;
	u32		div_mask;
	u32		div_shift;
	unsigned long   ref_clock;
};

#define to_clk_pllv3(_clk) container_of(_clk, struct clk_pllv3, clk)

static ulong clk_pllv3_genericv2_get_rate(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(dev_get_clk_ptr(clk->dev));
	unsigned long parent_rate = clk_get_parent_rate(clk);

	u32 div = (readl(pll->base) >> pll->div_shift) & pll->div_mask;

	return (div == 0) ? parent_rate * 22 : parent_rate * 20;
}

static ulong clk_pllv3_genericv2_set_rate(struct clk *clk, ulong rate)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);

	u32 div = (readl(pll->base) >> pll->div_shift) & pll->div_mask;
	u32 val = (div == 0) ? parent_rate * 22 : parent_rate * 20;

	if (rate == val)
		return 0;

	return -EINVAL;
}

static ulong clk_pllv3_generic_get_rate(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(dev_get_clk_ptr(clk->dev));
	unsigned long parent_rate = clk_get_parent_rate(clk);

	u32 div = (readl(pll->base) >> pll->div_shift) & pll->div_mask;

	return (div == 1) ? parent_rate * 22 : parent_rate * 20;
}

static ulong clk_pllv3_generic_set_rate(struct clk *clk, ulong rate)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	u32 val, div;

	if (rate == parent_rate * 22)
		div = 1;
	else if (rate == parent_rate * 20)
		div = 0;
	else
		return -EINVAL;

	val = readl(pll->base);
	val &= ~(pll->div_mask << pll->div_shift);
	val |= (div << pll->div_shift);
	writel(val, pll->base);

	/* Wait for PLL to lock */
	while (!(readl(pll->base) & pll->lock_bit))
		;

	return 0;
}

static int clk_pllv3_generic_enable(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	u32 val;

	val = readl(pll->base);
	if (pll->powerup_set)
		val |= pll->power_bit;
	else
		val &= ~pll->power_bit;

	val |= pll->enable_bit;

	writel(val, pll->base);

	return 0;
}

static int clk_pllv3_generic_disable(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	u32 val;

	val = readl(pll->base);
	if (pll->powerup_set)
		val &= ~pll->power_bit;
	else
		val |= pll->power_bit;

	val &= ~pll->enable_bit;

	writel(val, pll->base);

	return 0;
}

static const struct clk_ops clk_pllv3_generic_ops = {
	.get_rate	= clk_pllv3_generic_get_rate,
	.enable		= clk_pllv3_generic_enable,
	.disable	= clk_pllv3_generic_disable,
	.set_rate	= clk_pllv3_generic_set_rate,
};

static const struct clk_ops clk_pllv3_genericv2_ops = {
	.get_rate	= clk_pllv3_genericv2_get_rate,
	.enable		= clk_pllv3_generic_enable,
	.disable	= clk_pllv3_generic_disable,
	.set_rate	= clk_pllv3_genericv2_set_rate,
};

static ulong clk_pllv3_sys_get_rate(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	u32 div = readl(pll->base) & pll->div_mask;

	return parent_rate * div / 2;
}

static ulong clk_pllv3_sys_set_rate(struct clk *clk, ulong rate)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	unsigned long min_rate;
	unsigned long max_rate;
	u32 val, div;

	if (parent_rate == 0)
		return -EINVAL;

	min_rate = parent_rate * 54 / 2;
	max_rate = parent_rate * 108 / 2;

	if (rate < min_rate || rate > max_rate)
		return -EINVAL;

	div = rate * 2 / parent_rate;
	val = readl(pll->base);
	val &= ~pll->div_mask;
	val |= div;
	writel(val, pll->base);

	/* Wait for PLL to lock */
	while (!(readl(pll->base) & pll->lock_bit))
		;

	return 0;
}

static const struct clk_ops clk_pllv3_sys_ops = {
	.enable		= clk_pllv3_generic_enable,
	.disable	= clk_pllv3_generic_disable,
	.get_rate	= clk_pllv3_sys_get_rate,
	.set_rate	= clk_pllv3_sys_set_rate,
};

static ulong clk_pllv3_av_get_rate(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	u32 mfn = readl(pll->base + PLL_NUM_OFFSET);
	u32 mfd = readl(pll->base + PLL_DENOM_OFFSET);
	u32 div = readl(pll->base) & pll->div_mask;
	u64 temp64 = (u64)parent_rate;

	if (mfd == 0)
		return -EIO;

	temp64 *= mfn;
	do_div(temp64, mfd);

	return parent_rate * div + (unsigned long)temp64;
}

static ulong clk_pllv3_av_set_rate(struct clk *clk, ulong rate)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);
	unsigned long parent_rate = clk_get_parent_rate(clk);
	unsigned long min_rate;
	unsigned long max_rate;
	u32 val, div;
	u32 mfn, mfd = 1000000;
	u32 max_mfd = 0x3FFFFFFF;
	u64 temp64;

	if (parent_rate == 0)
		return -EINVAL;

	min_rate = parent_rate * 27;
	max_rate = parent_rate * 54;

	if (rate < min_rate || rate > max_rate)
		return -EINVAL;

	if (parent_rate <= max_mfd)
		mfd = parent_rate;

	div = rate / parent_rate;
	temp64 = (u64)(rate - div * parent_rate);
	temp64 *= mfd;
	do_div(temp64, parent_rate);
	mfn = temp64;

	val = readl(pll->base);
	val &= ~pll->div_mask;
	val |= div;
	writel(val, pll->base);
	writel(mfn, pll->base + PLL_NUM_OFFSET);
	writel(mfd, pll->base + PLL_DENOM_OFFSET);

	/* Wait for PLL to lock */
	while (!(readl(pll->base) & pll->lock_bit))
		;

	return 0;
}

static const struct clk_ops clk_pllv3_av_ops = {
	.enable		= clk_pllv3_generic_enable,
	.disable	= clk_pllv3_generic_disable,
	.get_rate	= clk_pllv3_av_get_rate,
	.set_rate	= clk_pllv3_av_set_rate,
};

static ulong clk_pllv3_enet_get_rate(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(clk);

	return pll->ref_clock;
}

static const struct clk_ops clk_pllv3_enet_ops = {
	.enable	= clk_pllv3_generic_enable,
	.disable	= clk_pllv3_generic_disable,
	.get_rate	= clk_pllv3_enet_get_rate,
};

struct clk *imx_clk_pllv3(enum imx_pllv3_type type, const char *name,
			  const char *parent_name, void __iomem *base,
			  u32 div_mask)
{
	struct clk_pllv3 *pll;
	struct clk *clk;
	char *drv_name;
	int ret;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->power_bit = BM_PLL_POWER;
	pll->enable_bit = BM_PLL_ENABLE;
	pll->lock_bit = BM_PLL_LOCK;

	switch (type) {
	case IMX_PLLV3_GENERIC:
		drv_name = UBOOT_DM_CLK_IMX_PLLV3_GENERIC;
		pll->div_shift = 0;
		pll->powerup_set = false;
		break;
	case IMX_PLLV3_GENERICV2:
		pll->power_bit = BM_PLL_POWER_V2;
		pll->lock_bit = BM_PLL_LOCK_V2;
		drv_name = UBOOT_DM_CLK_IMX_PLLV3_GENV2;
		pll->div_shift = 0;
		pll->powerup_set = false;
		break;
	case IMX_PLLV3_SYS:
		drv_name = UBOOT_DM_CLK_IMX_PLLV3_SYS;
		pll->div_shift = 0;
		pll->powerup_set = false;
		break;
	case IMX_PLLV3_USB:
		drv_name = UBOOT_DM_CLK_IMX_PLLV3_USB;
		pll->div_shift = 1;
		pll->powerup_set = true;
		break;
	case IMX_PLLV3_AV:
		drv_name = UBOOT_DM_CLK_IMX_PLLV3_AV;
		pll->div_shift = 0;
		pll->powerup_set = false;
		break;
	case IMX_PLLV3_ENET:
		drv_name = UBOOT_DM_CLK_IMX_PLLV3_ENET;
		pll->ref_clock = 500000000;
		break;
	default:
		kfree(pll);
		return ERR_PTR(-EINVAL);
	}

	pll->base = base;
	pll->div_mask = div_mask;
	clk = &pll->clk;

	ret = clk_register(clk, drv_name, name, parent_name);
	if (ret) {
		kfree(pll);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_pllv3_generic) = {
	.name	= UBOOT_DM_CLK_IMX_PLLV3_GENERIC,
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_generic_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(clk_pllv3_genericv2) = {
	.name	= UBOOT_DM_CLK_IMX_PLLV3_GENV2,
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_genericv2_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(clk_pllv3_sys) = {
	.name	= UBOOT_DM_CLK_IMX_PLLV3_SYS,
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_sys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(clk_pllv3_usb) = {
	.name	= UBOOT_DM_CLK_IMX_PLLV3_USB,
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_generic_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(clk_pllv3_av) = {
	.name	= UBOOT_DM_CLK_IMX_PLLV3_AV,
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_av_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(clk_pllv3_enet) = {
	.name	= UBOOT_DM_CLK_IMX_PLLV3_ENET,
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_enet_ops,
};
