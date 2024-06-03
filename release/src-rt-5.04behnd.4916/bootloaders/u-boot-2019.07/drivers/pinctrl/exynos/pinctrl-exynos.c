// SPDX-License-Identifier: GPL-2.0+
/*
 * Exynos pinctrl driver common code.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include "pinctrl-exynos.h"

DECLARE_GLOBAL_DATA_PTR;

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

/* given a pin-name, return the address of pin config registers */
static unsigned long pin_to_bank_base(struct udevice *dev, const char *pin_name,
						u32 *pin)
{
	struct exynos_pinctrl_priv *priv = dev_get_priv(dev);
	const struct samsung_pin_ctrl *pin_ctrl = priv->pin_ctrl;
	const struct samsung_pin_bank_data *bank_data = pin_ctrl->pin_banks;
	u32 nr_banks = pin_ctrl->nr_banks, idx = 0;
	char bank[10];

	/*
	 * The format of the pin name is <bank name>-<pin_number>.
	 * Example: gpa0-4 (gpa0 is the bank name and 4 is the pin number.
	 */
	while (pin_name[idx] != '-') {
		bank[idx] = pin_name[idx];
		idx++;
	}
	bank[idx] = '\0';
	*pin = pin_name[++idx] - '0';

	/* lookup the pin bank data using the pin bank name */
	for (idx = 0; idx < nr_banks; idx++)
		if (!strcmp(bank, bank_data[idx].name))
			break;

	return priv->base + bank_data[idx].offset;
}

/**
 * exynos_pinctrl_set_state: configure a pin state.
 * dev: the pinctrl device to be configured.
 * config: the state to be configured.
 */
int exynos_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(config);
	unsigned int count, idx, pin_num;
	unsigned int pinfunc, pinpud, pindrv;
	unsigned long reg, value;
	const char *name;

	/*
	 * refer to the following document for the pinctrl bindings
	 * linux/Documentation/devicetree/bindings/pinctrl/samsung-pinctrl.txt
	 */
	count = fdt_stringlist_count(fdt, node, "samsung,pins");
	if (count <= 0)
		return -EINVAL;

	pinfunc = fdtdec_get_int(fdt, node, "samsung,pin-function", -1);
	pinpud = fdtdec_get_int(fdt, node, "samsung,pin-pud", -1);
	pindrv = fdtdec_get_int(fdt, node, "samsung,pin-drv", -1);

	for (idx = 0; idx < count; idx++) {
		name = fdt_stringlist_get(fdt, node, "samsung,pins", idx, NULL);
		if (!name)
			continue;
		reg = pin_to_bank_base(dev, name, &pin_num);

		if (pinfunc != -1) {
			value = readl(reg + PIN_CON);
			value &= ~(0xf << (pin_num << 2));
			value |= (pinfunc << (pin_num << 2));
			writel(value, reg + PIN_CON);
		}

		if (pinpud != -1) {
			value = readl(reg + PIN_PUD);
			value &= ~(0x3 << (pin_num << 1));
			value |= (pinpud << (pin_num << 1));
			writel(value, reg + PIN_PUD);
		}

		if (pindrv != -1) {
			value = readl(reg + PIN_DRV);
			value &= ~(0x3 << (pin_num << 1));
			value |= (pindrv << (pin_num << 1));
			writel(value, reg + PIN_DRV);
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

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = base;
	priv->pin_ctrl = (struct samsung_pin_ctrl *)dev_get_driver_data(dev) +
				dev->req_seq;

	return 0;
}
