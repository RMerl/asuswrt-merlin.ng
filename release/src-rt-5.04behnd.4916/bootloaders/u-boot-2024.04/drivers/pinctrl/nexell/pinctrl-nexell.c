// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinctrl driver for Nexell SoCs
 * (C) Copyright 2016 Nexell
 * Bongyu, KOO <freestyle@nexell.co.kr>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include "pinctrl-nexell.h"
#include "pinctrl-s5pxx18.h"

DECLARE_GLOBAL_DATA_PTR;

/* given a pin-name, return the address of pin config registers */
unsigned long pin_to_bank_base(struct udevice *dev, const char *pin_name,
			       u32 *pin)
{
	struct nexell_pinctrl_priv *priv = dev_get_priv(dev);
	const struct nexell_pin_ctrl *pin_ctrl = priv->pin_ctrl;
	const struct nexell_pin_bank_data *bank_data = pin_ctrl->pin_banks;
	u32 nr_banks = pin_ctrl->nr_banks, idx = 0;
	char bank[10];

	/*
	 * The format of the pin name is <bank name>-<pin_number>.
	 * Example: gpioa-4 (gpioa is the bank name and 4 is the pin number)
	 */
	while (pin_name[idx] != '-') {
		bank[idx] = pin_name[idx];
		idx++;
	}
	bank[idx] = '\0';
	*pin = (u32)dectoul(&pin_name[++idx], NULL);

	/* lookup the pin bank data using the pin bank name */
	for (idx = 0; idx < nr_banks; idx++)
		if (!strcmp(bank, bank_data[idx].name))
			break;

	return priv->base + bank_data[idx].offset;
}

int nexell_pinctrl_probe(struct udevice *dev)
{
	struct nexell_pinctrl_priv *priv;
	fdt_addr_t base;

	priv = dev_get_priv(dev);
	if (!priv)
		return -EINVAL;

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = base;

	priv->pin_ctrl = (struct nexell_pin_ctrl *)dev_get_driver_data(dev);

	s5pxx18_pinctrl_init(dev);

	return 0;
}
