/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#ifndef __PINCTRL_UNIPHIER_H__
#define __PINCTRL_UNIPHIER_H__

#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/build_bug.h>
#include <linux/kernel.h>
#include <linux/types.h>

/* drive strength control register number */
#define UNIPHIER_PIN_DRVCTRL_SHIFT	0
#define UNIPHIER_PIN_DRVCTRL_BITS	9
#define UNIPHIER_PIN_DRVCTRL_MASK	((1U << (UNIPHIER_PIN_DRVCTRL_BITS)) \
					 - 1)

/* drive control type */
#define UNIPHIER_PIN_DRV_TYPE_SHIFT	((UNIPHIER_PIN_DRVCTRL_SHIFT) + \
					 (UNIPHIER_PIN_DRVCTRL_BITS))
#define UNIPHIER_PIN_DRV_TYPE_BITS	2
#define UNIPHIER_PIN_DRV_TYPE_MASK	((1U << (UNIPHIER_PIN_DRV_TYPE_BITS)) \
					 - 1)

/* drive control type */
enum uniphier_pin_drv_type {
	UNIPHIER_PIN_DRV_1BIT,		/* 2 level control: 4/8 mA */
	UNIPHIER_PIN_DRV_2BIT,		/* 4 level control: 8/12/16/20 mA */
	UNIPHIER_PIN_DRV_3BIT,		/* 8 level control: 4/5/7/9/11/12/14/16 mA */
};

#define UNIPHIER_PIN_DRVCTRL(x) \
	(((x) & (UNIPHIER_PIN_DRVCTRL_MASK)) << (UNIPHIER_PIN_DRVCTRL_SHIFT))
#define UNIPHIER_PIN_DRV_TYPE(x) \
	(((x) & (UNIPHIER_PIN_DRV_TYPE_MASK)) << (UNIPHIER_PIN_DRV_TYPE_SHIFT))

#define UNIPHIER_PIN_ATTR_PACKED(drvctrl, drv_type)	\
	UNIPHIER_PIN_DRVCTRL(drvctrl) |			\
	UNIPHIER_PIN_DRV_TYPE(drv_type)

static inline unsigned int uniphier_pin_get_drvctrl(unsigned int data)
{
	return (data >> UNIPHIER_PIN_DRVCTRL_SHIFT) & UNIPHIER_PIN_DRVCTRL_MASK;
}

static inline unsigned int uniphier_pin_get_drv_type(unsigned int data)
{
	return (data >> UNIPHIER_PIN_DRV_TYPE_SHIFT) &
						UNIPHIER_PIN_DRV_TYPE_MASK;
}

/**
 * struct uniphier_pinctrl_pin - pin data for UniPhier SoC
 *
 * @number: pin number
 * @data: additional per-pin data
 */
struct uniphier_pinctrl_pin {
	unsigned number;
	const char *name;
	unsigned int data;
};

/**
 * struct uniphier_pinctrl_group - pin group data for UniPhier SoC
 *
 * @name: pin group name
 * @pins: array of pins that belong to the group
 * @num_pins: number of pins in the group
 * @muxvals: array of values to be set to pinmux registers
 */
struct uniphier_pinctrl_group {
	const char *name;
	const unsigned *pins;
	unsigned num_pins;
	const int *muxvals;
};

/**
 * struct uniphier_pinctrl_socdata - SoC data for UniPhier pin controller
 *
 * @pins: array of pin data
 * @pins_count: number of pin data
 * @groups: array of pin group data
 * @groups_count: number of pin group data
 * @functions: array of pinmux function names
 * @functions_count: number of pinmux functions
 * @mux_bits: bit width of each pinmux register
 * @reg_stride: stride of pinmux register address
 * @caps: SoC-specific capability flag
 */
struct uniphier_pinctrl_socdata {
	const struct uniphier_pinctrl_pin *pins;
	int pins_count;
	const struct uniphier_pinctrl_group *groups;
	int groups_count;
	const char * const *functions;
	int functions_count;
	unsigned caps;
#define UNIPHIER_PINCTRL_CAPS_PUPD_SIMPLE	BIT(3)
#define UNIPHIER_PINCTRL_CAPS_PERPIN_IECTRL	BIT(2)
#define UNIPHIER_PINCTRL_CAPS_DBGMUX_SEPARATE	BIT(1)
#define UNIPHIER_PINCTRL_CAPS_MUX_4BIT		BIT(0)
};

#define UNIPHIER_PINCTRL_PIN(a, b, c, d)				\
{									\
	.number = a,							\
	.name = b,							\
	.data = UNIPHIER_PIN_ATTR_PACKED(c, d),				\
}

#define __UNIPHIER_PINCTRL_GROUP(grp)					\
	{								\
		.name = #grp,						\
		.pins = grp##_pins,					\
		.num_pins = ARRAY_SIZE(grp##_pins),			\
		.muxvals = grp##_muxvals +				\
			BUILD_BUG_ON_ZERO(ARRAY_SIZE(grp##_pins) !=	\
					  ARRAY_SIZE(grp##_muxvals)),	\
	}

#define __UNIPHIER_PINMUX_FUNCTION(func)	#func

#ifdef CONFIG_SPL_BUILD
	/*
	 * a tricky way to drop unneeded *_pins and *_muxvals arrays from SPL,
	 * suppressing "defined but not used" warnings.
	 */
#define UNIPHIER_PINCTRL_GROUP(grp)					\
	{ .num_pins = ARRAY_SIZE(grp##_pins) + ARRAY_SIZE(grp##_muxvals) }
#define UNIPHIER_PINMUX_FUNCTION(func)		NULL
#else
#define UNIPHIER_PINCTRL_GROUP(grp)		__UNIPHIER_PINCTRL_GROUP(grp)
#define UNIPHIER_PINMUX_FUNCTION(func)		__UNIPHIER_PINMUX_FUNCTION(func)
#endif

#define UNIPHIER_PINCTRL_GROUP_SPL(grp)		__UNIPHIER_PINCTRL_GROUP(grp)
#define UNIPHIER_PINMUX_FUNCTION_SPL(func)	__UNIPHIER_PINMUX_FUNCTION(func)

/**
 * struct uniphier_pinctrl_priv - private data for UniPhier pinctrl driver
 *
 * @base: base address of the pinctrl device
 * @socdata: SoC specific data
 */
struct uniphier_pinctrl_priv {
	void __iomem *base;
	struct uniphier_pinctrl_socdata *socdata;
};

extern const struct pinctrl_ops uniphier_pinctrl_ops;

int uniphier_pinctrl_probe(struct udevice *dev,
			   struct uniphier_pinctrl_socdata *socdata);

#endif /* __PINCTRL_UNIPHIER_H__ */
