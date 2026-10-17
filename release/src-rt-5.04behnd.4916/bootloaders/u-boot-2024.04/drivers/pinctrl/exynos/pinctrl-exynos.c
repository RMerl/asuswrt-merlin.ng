// SPDX-License-Identifier: GPL-2.0+
/*
 * Exynos pinctrl driver common code.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <log.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include "pinctrl-exynos.h"

/* CON, DAT, PUD, DRV */
const struct samsung_pin_bank_type bank_type_alive = {
	.fld_width = { 4, 1, 2, 2, },
	.reg_offset = { 0x00, 0x04, 0x08, 0x0c, },
};

static const char * const exynos_pinctrl_props[PINCFG_TYPE_NUM] = {
	[PINCFG_TYPE_FUNC]	= "samsung,pin-function",
	[PINCFG_TYPE_DAT]	= "samsung,pin-val",
	[PINCFG_TYPE_PUD]	= "samsung,pin-pud",
	[PINCFG_TYPE_DRV]	= "samsung,pin-drv",
};

/**
 * exynos_pinctrl_setup_peri: setup pinctrl for a peripheral.
 * conf: soc specific pin configuration data array
 * num_conf: number of configurations in the conf array.
 * base: base address of the pin controller.
 */
void exynos_pinctrl_setup_peri(struct exynos_pinctrl_config_data *conf,
		unsigned int num_conf, unsigned long base)
{
	unsigned int idx, val;

	for (idx = 0; idx < num_conf; idx++) {
		val = readl(base + conf[idx].offset);
		val &= ~(conf[idx].mask);
		val |= conf[idx].value;
		writel(val, base + conf[idx].offset);
	}
}

static void parse_pin(const char *pin_name, u32 *pin, char *bank_name)
{
	u32 idx = 0;

	/*
	 * The format of the pin name is <bank_name name>-<pin_number>.
	 * Example: gpa0-4 (gpa0 is the bank_name name and 4 is the pin number.
	 */
	while (pin_name[idx] != '-') {
		bank_name[idx] = pin_name[idx];
		idx++;
	}
	bank_name[idx] = '\0';
	*pin = pin_name[++idx] - '0';
}

/* given a bank name, find out the pin bank structure */
static const struct samsung_pin_bank_data *get_bank(struct udevice *dev,
						    const char *bank_name)
{
	struct exynos_pinctrl_priv *priv = dev_get_priv(dev);
	const struct samsung_pin_ctrl *pin_ctrl_array = priv->pin_ctrl;
	const struct samsung_pin_bank_data *bank_data;
	u32 nr_banks, pin_ctrl_idx = 0, idx = 0;

	/* lookup the pin bank data using the pin bank name */
	while (true) {
		const struct samsung_pin_ctrl *pin_ctrl =
			&pin_ctrl_array[pin_ctrl_idx];

		nr_banks = pin_ctrl->nr_banks;
		if (!nr_banks)
			break;

		bank_data = pin_ctrl->pin_banks;
		for (idx = 0; idx < nr_banks; idx++) {
			debug("pinctrl[%d] bank_data[%d] name is: %s\n",
					pin_ctrl_idx, idx, bank_data[idx].name);
			if (!strcmp(bank_name, bank_data[idx].name))
				return &bank_data[idx];
		}
		pin_ctrl_idx++;
	}

	return NULL;
}

static void exynos_pinctrl_set_pincfg(unsigned long reg_base, u32 pin_num,
				      u32 val, enum pincfg_type pincfg,
				      const struct samsung_pin_bank_type *type)
{
	u32 width = type->fld_width[pincfg];
	u32 reg_offset = type->reg_offset[pincfg];
	u32 mask = (1 << width) - 1;
	u32 shift = pin_num * width;
	u32 data;

	data = readl(reg_base + reg_offset);
	data &= ~(mask << shift);
	data |= val << shift;
	writel(data, reg_base + reg_offset);
}

/**
 * exynos_pinctrl_set_state: configure a pin state.
 * dev: the pinctrl device to be configured.
 * config: the state to be configured.
 */
int exynos_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct exynos_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned int count, idx;
	unsigned int pinvals[PINCFG_TYPE_NUM];

	/*
	 * refer to the following document for the pinctrl bindings
	 * linux/Documentation/devicetree/bindings/pinctrl/samsung-pinctrl.txt
	 */
	count = dev_read_string_count(config, "samsung,pins");
	if (count <= 0)
		return -EINVAL;

	for (idx = 0; idx < PINCFG_TYPE_NUM; ++idx) {
		pinvals[idx] = dev_read_u32_default(config,
						exynos_pinctrl_props[idx], -1);
	}
	pinvals[PINCFG_TYPE_DAT] = -1; /* ignore GPIO data register */

	for (idx = 0; idx < count; idx++) {
		const struct samsung_pin_bank_data *bank;
		unsigned int pin_num;
		char bank_name[10];
		unsigned long reg;
		const char *name = NULL;
		int pincfg, err;

		err = dev_read_string_index(config, "samsung,pins", idx, &name);
		if (err || !name)
			continue;

		parse_pin(name, &pin_num, bank_name);
		bank = get_bank(dev, bank_name);
		reg = priv->base + bank->offset;

		for (pincfg = 0; pincfg < PINCFG_TYPE_NUM; ++pincfg) {
			unsigned int val = pinvals[pincfg];

			if (val != -1)
				exynos_pinctrl_set_pincfg(reg, pin_num, val,
							  pincfg, bank->type);
		}
	}

	return 0;
}

int exynos_pinctrl_probe(struct udevice *dev)
{
	struct exynos_pinctrl_priv *priv;
	fdt_addr_t base;

	priv = dev_get_priv(dev);
	if (!priv)
		return -EINVAL;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = base;
	priv->pin_ctrl = (struct samsung_pin_ctrl *)dev_get_driver_data(dev) +
				dev_seq(dev);

	return 0;
}
