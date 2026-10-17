/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _PINCTRL_MTMIPS_COMMON_H_
#define _PINCTRL_MTMIPS_COMMON_H_

struct mtmips_pmx_func {
	const char *name;
	int value;
};

struct mtmips_pmx_group {
	const char *name;

	u32 reg;
	u32 shift;
	char mask;

	int pconf_avail;
	u32 pconf_reg;
	u32 pconf_shift;

	int nfuncs;
	const struct mtmips_pmx_func *funcs;
};

struct mtmips_pinctrl_priv {
	void __iomem *base;

	u32 ngroups;
	const struct mtmips_pmx_group *groups;

	u32 nfuncs;
	const struct mtmips_pmx_func **funcs;
};

#define FUNC(name, value)	{ name, value }

#define GRP(_name, _funcs, _reg, _shift, _mask) \
	{ .name = (_name), .reg = (_reg), .shift = (_shift), .mask = (_mask), \
	  .funcs = (_funcs), .nfuncs = ARRAY_SIZE(_funcs) }

#define GRP_PCONF(_name, _funcs, _reg, _shift, _mask, _pconf_reg, _pconf_shift) \
	{ .name = (_name), .reg = (_reg), .shift = (_shift), .mask = (_mask), \
	  .funcs = (_funcs), .nfuncs = ARRAY_SIZE(_funcs), .pconf_avail = 1, \
	  .pconf_reg = (_pconf_reg), .pconf_shift = (_pconf_shift) }

void mtmips_pinctrl_reg_set(struct mtmips_pinctrl_priv *priv,
			    u32 reg, u32 shift, u32 mask, u32 value);

int mtmips_get_functions_count(struct udevice *dev);
const char *mtmips_get_function_name(struct udevice *dev,
				     unsigned int selector);
int mtmips_pinmux_group_set(struct udevice *dev, unsigned int group_selector,
			    unsigned int func_selector);
int mtmips_pinctrl_probe(struct mtmips_pinctrl_priv *priv, u32 ngroups,
			 const struct mtmips_pmx_group *groups);

#endif /* _PINCTRL_MTMIPS_COMMON_H_ */
