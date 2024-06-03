/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#ifndef __PINCTRL_MESON_H__
#define __PINCTRL_MESON_H__

#include <linux/types.h>

struct meson_pmx_group {
	const char *name;
	const unsigned int *pins;
	unsigned int num_pins;
	const void *data;
};

struct meson_pmx_func {
	const char *name;
	const char * const *groups;
	unsigned int num_groups;
};

struct meson_pinctrl_data {
	const char *name;
	struct meson_pmx_group *groups;
	struct meson_pmx_func *funcs;
	struct meson_bank *banks;
	unsigned int pin_base;
	unsigned int num_pins;
	unsigned int num_groups;
	unsigned int num_funcs;
	unsigned int num_banks;
	const struct driver *gpio_driver;
	void *pmx_data;
};

struct meson_pinctrl {
	struct meson_pinctrl_data *data;
	void __iomem *reg_mux;
	void __iomem *reg_gpio;
	void __iomem *reg_pull;
	void __iomem *reg_pullen;
	void __iomem *reg_ds;
};

/**
 * struct meson_reg_desc - a register descriptor
 *
 * @reg:	register offset in the regmap
 * @bit:	bit index in register
 *
 * The structure describes the information needed to control pull,
 * pull-enable, direction, etc. for a single pin
 */
struct meson_reg_desc {
	unsigned int reg;
	unsigned int bit;
};

/**
 * enum meson_pinconf_drv - value of drive-strength supported
 */
enum meson_pinconf_drv {
	MESON_PINCONF_DRV_500UA,
	MESON_PINCONF_DRV_2500UA,
	MESON_PINCONF_DRV_3000UA,
	MESON_PINCONF_DRV_4000UA,
};

/**
 * enum meson_reg_type - type of registers encoded in @meson_reg_desc
 */
enum meson_reg_type {
	REG_PULLEN,
	REG_PULL,
	REG_DIR,
	REG_OUT,
	REG_IN,
	REG_DS,
	NUM_REG,
};

/**
 * struct meson bank
 *
 * @name:	bank name
 * @first:	first pin of the bank
 * @last:	last pin of the bank
 * @regs:	array of register descriptors
 *
 * A bank represents a set of pins controlled by a contiguous set of
 * bits in the domain registers. The structure specifies which bits in
 * the regmap control the different functionalities. Each member of
 * the @regs array refers to the first pin of the bank.
 */
struct meson_bank {
	const char *name;
	unsigned int first;
	unsigned int last;
	struct meson_reg_desc regs[NUM_REG];
};

#define PIN(x, b)	(b + x)

#define FUNCTION(fn)							\
	{								\
		.name = #fn,						\
		.groups = fn ## _groups,				\
		.num_groups = ARRAY_SIZE(fn ## _groups),		\
	}

#define BANK_DS(n, f, l, per, peb, pr, pb, dr, db, or, ob, ir, ib, \
		dsr, dsb)                                                  \
	{                                                                  \
		.name = n,                                                 \
		.first = f,                                                \
		.last = l,                                                 \
		.regs = {                                                  \
		    [REG_PULLEN] = {per, peb},                             \
		    [REG_PULL] = {pr, pb},                                 \
		    [REG_DIR] = {dr, db},                                  \
		    [REG_OUT] = { or, ob},                                 \
		    [REG_IN] = {ir, ib},                                   \
		    [REG_DS] = {dsr, dsb},                                 \
		},                                                         \
	}

#define BANK(n, f, l, per, peb, pr, pb, dr, db, or, ob, ir, ib) \
	BANK_DS(n, f, l, per, peb, pr, pb, dr, db, or, ob, ir, ib, 0, 0)

#define MESON_PIN(x, b) PINCTRL_PIN(PIN(x, b), #x)

extern const struct pinctrl_ops meson_pinctrl_ops;

int meson_pinctrl_get_groups_count(struct udevice *dev);
const char *meson_pinctrl_get_group_name(struct udevice *dev,
					 unsigned int selector);
int meson_pinctrl_get_pins_count(struct udevice *dev);
const char *meson_pinctrl_get_pin_name(struct udevice *dev,
				       unsigned int selector);
int meson_pinmux_get_functions_count(struct udevice *dev);
const char *meson_pinmux_get_function_name(struct udevice *dev,
					   unsigned int selector);
int meson_pinctrl_probe(struct udevice *dev);

int meson_gpio_get(struct udevice *dev, unsigned int offset);
int meson_gpio_set(struct udevice *dev, unsigned int offset, int value);
int meson_gpio_get_direction(struct udevice *dev, unsigned int offset);
int meson_gpio_direction_input(struct udevice *dev, unsigned int offset);
int meson_gpio_direction_output(struct udevice *dev, unsigned int offset,
				int value);
int meson_gpio_probe(struct udevice *dev);

int meson_pinconf_set(struct udevice *dev, unsigned int pin,
		      unsigned int param, unsigned int arg);
int meson_pinconf_group_set(struct udevice *dev,
			    unsigned int group_selector,
			    unsigned int param, unsigned int arg);

#endif /* __PINCTRL_MESON_H__ */
