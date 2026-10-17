/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Pinctrl / GPIO driver for StarFive SoC
 *
 * Copyright (C) 2022 Shanghai StarFive Technology Co., Ltd.
 *   Author: Lee Kuan Lim <kuanlim.lee@starfivetech.com>
 *   Author: Jianlong Huang <jianlong.huang@starfivetech.com>
 */

#define STARFIVE_PINCTRL(a, b) { .number = a, .name = b }

extern const struct pinctrl_ops starfive_pinctrl_ops;

struct starfive_pinctrl_pin {
	unsigned int number;
	const char *name;
};

struct starfive_pinctrl_soc_info {
	/* pinctrl */
	int (*set_one_pinmux)(struct udevice *dev, unsigned int pin,
			      unsigned int din, u32 dout, u32 doen, u32 func);
	int (*get_padcfg_base)(struct udevice *dev,
			       unsigned int pin);

	/* gpio dout/doen/din/gpioinput register */
	unsigned int dout_reg_base;
	unsigned int dout_mask;
	unsigned int doen_reg_base;
	unsigned int doen_mask;
	unsigned int gpi_reg_base;
	unsigned int gpi_mask;
	unsigned int gpioin_reg_base;

	/* gpio */
	const char *gpio_bank_name;
	int ngpios;
	void (*gpio_init_hw)(struct udevice *dev);
};

/*
 * struct starfive_pinctrl_priv - private data for Starfive pinctrl driver
 *
 * @padctl_base: base address of the pinctrl device
 * @info: SoC specific data & function
 */
struct starfive_pinctrl_priv {
	void __iomem *base;
	struct starfive_pinctrl_soc_info *info;
};

void starfive_set_gpiomux(struct udevice *dev, unsigned int pin,
			  unsigned int din, u32 dout, u32 doen);
int starfive_pinctrl_probe(struct udevice *dev,
			   const struct starfive_pinctrl_soc_info *info);
