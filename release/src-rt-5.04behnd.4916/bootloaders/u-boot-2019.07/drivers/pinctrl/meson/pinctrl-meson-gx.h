/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Beniamino Galvani <b.galvani@gmail.com>
 * Copyright (C) 2017 Jerome Brunet  <jbrunet@baylibre.com>
 */

#ifndef __PINCTRL_MESON_GX_H__
#define __PINCTRL_MESON_GX_H__

#include "pinctrl-meson.h"

struct meson_gx_pmx_data {
	bool is_gpio;
	unsigned int reg;
	unsigned int bit;
};

#define PMX_DATA(r, b, g)						\
	{								\
		.reg = r,						\
		.bit = b,						\
		.is_gpio = g,						\
	}

#define GROUP(grp, r, b)						\
	{								\
		.name = #grp,						\
		.pins = grp ## _pins,					\
		.num_pins = ARRAY_SIZE(grp ## _pins),			\
			.data = (const struct meson_gx_pmx_data[]){	\
			PMX_DATA(r, b, false),				\
		},							\
	}

#define GPIO_GROUP(gpio, b)						\
	{								\
		.name = #gpio,						\
		.pins = (const unsigned int[]){ PIN(gpio, b) },		\
		.num_pins = 1,						\
		.data = (const struct meson_gx_pmx_data[]){		\
			PMX_DATA(0, 0, true),				\
		},							\
	}

extern const struct pinctrl_ops meson_gx_pinctrl_ops;
extern const struct driver meson_gx_gpio_driver;

#endif /* __PINCTRL_MESON_GX_H__ */
