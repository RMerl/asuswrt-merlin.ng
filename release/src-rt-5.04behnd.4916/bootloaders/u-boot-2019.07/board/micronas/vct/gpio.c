// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 */

#include <common.h>
#include <asm/io.h>
#include "vct.h"

/*
 * Find out to which of the 2 gpio modules the pin specified in the
 * argument belongs:
 * GPIO_MODULE yields 0 for pins  0 to 31,
 *                    1 for pins 32 to 63
 */
#define GPIO_MODULE(pin)	((pin) >> 5)

/*
 * Bit position within a 32-bit peripheral register (where every
 * bit is one bitslice)
 */
#define MASK(pin)		(1 << ((pin) & 0x1F))
#define BASE_ADDR(mod)		module_base[mod]

/*
 * Lookup table for transforming gpio module number 0 to 2 to
 * address offsets
 */
static u32 module_base[] = {
	GPIO1_BASE,
	GPIO2_BASE
};

static void clrsetbits(u32 addr, u32 and_mask, u32 or_mask)
{
	reg_write(addr, (reg_read(addr) & ~and_mask) | or_mask);
}

int vct_gpio_dir(int pin, int dir)
{
	u32 gpio_base;

	gpio_base = BASE_ADDR(GPIO_MODULE(pin));

	if (dir == 0)
		clrsetbits(GPIO_SWPORTA_DDR(gpio_base), MASK(pin), 0);
	else
		clrsetbits(GPIO_SWPORTA_DDR(gpio_base), 0, MASK(pin));

	return 0;
}

void vct_gpio_set(int pin, int val)
{
	u32 gpio_base;

	gpio_base = BASE_ADDR(GPIO_MODULE(pin));

	if (val == 0)
		clrsetbits(GPIO_SWPORTA_DR(gpio_base), MASK(pin), 0);
	else
		clrsetbits(GPIO_SWPORTA_DR(gpio_base), 0, MASK(pin));
}

int vct_gpio_get(int pin)
{
	u32 gpio_base;
	u32 value;

	gpio_base = BASE_ADDR(GPIO_MODULE(pin));
	value = reg_read(GPIO_EXT_PORTA(gpio_base));

	return ((value & MASK(pin)) ? 1 : 0);
}
