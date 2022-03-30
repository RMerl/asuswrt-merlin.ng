/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Jerome Brunet  <jbrunet@baylibre.com>
 * Copyright (C) 2017 Xingyu Chen <xingyu.chen@amlogic.com>
 */

#ifndef __PINCTRL_MESON_AXG_H__
#define __PINCTRL_MESON_AXG_H__

#include "pinctrl-meson.h"

struct meson_pmx_bank {
	const char *name;
	unsigned int first;
	unsigned int last;
	unsigned int reg;
	unsigned int offset;
};

struct meson_axg_pmx_data {
	struct meson_pmx_bank *pmx_banks;
	unsigned int num_pmx_banks;
};

#define BANK_PMX(n, f, l, r, o)				\
	{							\
		.name   = n,					\
		.first	= f,					\
		.last	= l,					\
		.reg	= r,					\
		.offset = o,					\
	}

struct meson_pmx_axg_data {
	unsigned int func;
};

#define PMX_DATA(f)							\
	{								\
		.func = f,						\
	}

#define GROUP(grp, f)							\
	{								\
		.name = #grp,						\
		.pins = grp ## _pins,                                   \
		.num_pins = ARRAY_SIZE(grp ## _pins),			\
		.data = (const struct meson_pmx_axg_data[]){		\
			PMX_DATA(f),					\
		},							\
	}

#define GPIO_GROUP(gpio, b)						\
	{								\
		.name = #gpio,						\
		.pins = (const unsigned int[]){ PIN(gpio, b) },		\
		.num_pins = 1,						\
		.data = (const struct meson_pmx_axg_data[]){		\
			PMX_DATA(0),					\
		},							\
	}

extern const struct pinctrl_ops meson_axg_pinctrl_ops;
extern const struct driver meson_axg_gpio_driver;

#endif /* __PINCTRL_MESON_AXG_H__ */
