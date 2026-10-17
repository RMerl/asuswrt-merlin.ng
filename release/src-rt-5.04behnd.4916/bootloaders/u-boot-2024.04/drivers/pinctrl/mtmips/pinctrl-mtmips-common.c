// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <dm/pinctrl.h>
#include <linux/io.h>

#include "pinctrl-mtmips-common.h"

void mtmips_pinctrl_reg_set(struct mtmips_pinctrl_priv *priv,
			    u32 reg, u32 shift, u32 mask, u32 value)
{
	u32 val;

	val = readl(priv->base + reg);
	val &= ~(mask << shift);
	val |= value << shift;
	writel(val, priv->base + reg);
}

int mtmips_get_functions_count(struct udevice *dev)
{
	struct mtmips_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->nfuncs;
}

const char *mtmips_get_function_name(struct udevice *dev, unsigned int selector)
{
	struct mtmips_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->funcs[selector]->name;
}

int mtmips_pinmux_group_set(struct udevice *dev, unsigned int group_selector,
			    unsigned int func_selector)
{
	struct mtmips_pinctrl_priv *priv = dev_get_priv(dev);
	const struct mtmips_pmx_group *grp = &priv->groups[group_selector];
	const struct mtmips_pmx_func *func = priv->funcs[func_selector];
	int i;

	if (!grp->nfuncs)
		return 0;

	for (i = 0; i < grp->nfuncs; i++) {
		if (!strcmp(grp->funcs[i].name, func->name)) {
			mtmips_pinctrl_reg_set(priv, grp->reg, grp->shift,
					       grp->mask, grp->funcs[i].value);
			return 0;
		}
	}

	return -EINVAL;
}

int mtmips_pinctrl_probe(struct mtmips_pinctrl_priv *priv, u32 ngroups,
			 const struct mtmips_pmx_group *groups)
{
	int i, j, n;

	priv->ngroups = ngroups;
	priv->groups = groups;

	priv->nfuncs = 0;

	for (i = 0; i < ngroups; i++)
		priv->nfuncs += groups[i].nfuncs;

	priv->funcs = malloc(priv->nfuncs * sizeof(*priv->funcs));
	if (!priv->funcs)
		return -ENOMEM;

	n = 0;

	for (i = 0; i < ngroups; i++) {
		for (j = 0; j < groups[i].nfuncs; j++)
			priv->funcs[n++] = &groups[i].funcs[j];
	}

	return 0;
}
