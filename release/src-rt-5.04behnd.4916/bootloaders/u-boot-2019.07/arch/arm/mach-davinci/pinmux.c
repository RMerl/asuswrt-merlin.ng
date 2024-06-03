// SPDX-License-Identifier: GPL-2.0+
/*
 * DaVinci pinmux functions.
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc Ltd, <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 * Copyright (C) 2004 Texas Instruments.
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>

/*
 * Change the setting of a pin multiplexer field.
 *
 * Takes an array of pinmux settings similar to:
 *
 * struct pinmux_config uart_pins[] = {
 *	{ &davinci_syscfg_regs->pinmux[8], 2, 7 },
 *	{ &davinci_syscfg_regs->pinmux[9], 2, 0 }
 * };
 *
 * Stepping through the array, each pinmux[n] register has the given value
 * set in the pin mux field specified.
 *
 * The number of pins in the array must be passed (ARRAY_SIZE can provide
 * this value conveniently).
 *
 * Returns 0 if all field numbers and values are in the correct range,
 * else returns -1.
 */
int davinci_configure_pin_mux(const struct pinmux_config *pins,
			      const int n_pins)
{
	int i;

	/* check for invalid pinmux values */
	for (i = 0; i < n_pins; i++) {
		if (pins[i].field >= PIN_MUX_NUM_FIELDS ||
		    (pins[i].value & ~PIN_MUX_FIELD_MASK) != 0)
			return -1;
	}

	/* configure the pinmuxes */
	for (i = 0; i < n_pins; i++) {
		const int offset = pins[i].field * PIN_MUX_FIELD_SIZE;
		const unsigned int value = pins[i].value << offset;
		const unsigned int mask = PIN_MUX_FIELD_MASK << offset;
		const dv_reg *mux = pins[i].mux;

		writel(value | (readl(mux) & (~mask)), mux);
	}

	return 0;
}

/*
 * Configure multiple pinmux resources.
 *
 * Takes an pinmux_resource array of pinmux_config and pin counts:
 *
 * const struct pinmux_resource pinmuxes[] = {
 *	PINMUX_ITEM(uart_pins),
 *	PINMUX_ITEM(i2c_pins),
 * };
 *
 * The number of items in the array must be passed (ARRAY_SIZE can provide
 * this value conveniently).
 *
 * Each item entry is configured in the defined order. If configuration
 * of any item fails, -1 is returned and none of the following items are
 * configured. On success, 0 is returned.
 */
int davinci_configure_pin_mux_items(const struct pinmux_resource *item,
				    const int n_items)
{
	int i;

	for (i = 0; i < n_items; i++) {
		if (davinci_configure_pin_mux(item[i].pins,
					      item[i].n_pins) != 0)
			return -1;
	}

	return 0;
}
