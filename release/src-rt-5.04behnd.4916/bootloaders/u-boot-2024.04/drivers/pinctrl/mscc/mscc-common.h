/* SPDX-License-Identifier: (GPL-2.0 OR MIT) */
/*
 * Microsemi SoCs pinctrl driver
 *
 * Author: <alexandre.belloni@free-electrons.com>
 * License: Dual MIT/GPL
 * Copyright (c) 2017 Microsemi Corporation
 */

#define MSCC_FUNC_PER_PIN	4

enum mscc_regs_gpio {
	MSCC_GPIO_OUT_SET,
	MSCC_GPIO_OUT_CLR,
	MSCC_GPIO_OUT,
	MSCC_GPIO_IN,
	MSCC_GPIO_OE,
	MSCC_GPIO_INTR,
	MSCC_GPIO_INTR_ENA,
	MSCC_GPIO_INTR_IDENT,
	MSCC_GPIO_ALT0,
	MSCC_GPIO_ALT1,
};

struct mscc_pin_caps {
	unsigned int pin;
	unsigned char functions[MSCC_FUNC_PER_PIN];
};

struct mscc_pin_data {
	const char *name;
	struct mscc_pin_caps *drv_data;
};

#define MSCC_P(p, f0, f1, f2)						\
static struct mscc_pin_caps mscc_pin_##p = {				\
	.pin = p,							\
	.functions = {							\
			FUNC_GPIO, FUNC_##f0, FUNC_##f1, FUNC_##f2,	\
	},								\
}

struct mscc_pmx_func {
	const char **groups;
	unsigned int ngroups;
};

struct mscc_pinctrl {
	struct udevice *dev;
	struct pinctrl_dev *pctl;
	void __iomem *regs;
	struct mscc_pmx_func *func;
	int num_func;
	const struct mscc_pin_data *mscc_pins;
	int num_pins;
	char * const *function_names;
	const unsigned long *mscc_gpios;
};

int mscc_pinctrl_probe(struct udevice *dev, int num_func,
		       const struct mscc_pin_data *mscc_pins, int num_pins,
		       char * const *function_names,
		       const unsigned long *mscc_gpios);
extern const struct pinctrl_ops mscc_pinctrl_ops;

extern const struct dm_gpio_ops mscc_gpio_ops;
