/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010 eXMeritus, A Boeing Company
 */

#ifndef POWERPC_ASM_MPC85XX_GPIO_H_
#define POWERPC_ASM_MPC85XX_GPIO_H_

# include <asm/immap_85xx.h>

/*
 * The following internal functions are an MPC85XX-specific GPIO API which
 * allows setting and querying multiple GPIOs in a single operation.
 *
 * All of these look relatively large, but the arguments are almost always
 * constants, so they compile down to just a few instructions and a
 * memory-mapped IO operation or two.
 */
static inline void mpc85xx_gpio_set(unsigned int mask,
		unsigned int dir, unsigned int val)
{
	ccsr_gpio_t *gpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);

	/* First mask off the unwanted parts of "dir" and "val" */
	dir &= mask;
	val &= mask;

	/* Now read in the values we're supposed to preserve */
	dir |= (in_be32(&gpio->gpdir) & ~mask);
	val |= (in_be32(&gpio->gpdat) & ~mask);

	/*
	 * Poke the new output values first, then set the direction.  This
	 * helps to avoid transient data when switching from input to output
	 * and vice versa.
	 */
	out_be32(&gpio->gpdat, val);
	out_be32(&gpio->gpdir, dir);
}

static inline void mpc85xx_gpio_set_in(unsigned int gpios)
{
	mpc85xx_gpio_set(gpios, 0x00000000, 0x00000000);
}

static inline void mpc85xx_gpio_set_low(unsigned int gpios)
{
	mpc85xx_gpio_set(gpios, 0xFFFFFFFF, 0x00000000);
}

static inline void mpc85xx_gpio_set_high(unsigned int gpios)
{
	mpc85xx_gpio_set(gpios, 0xFFFFFFFF, 0xFFFFFFFF);
}

static inline unsigned int mpc85xx_gpio_get(unsigned int mask)
{
	ccsr_gpio_t *gpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);

	/* Read the requested values */
	return in_be32(&gpio->gpdat) & mask;
}

/*
 * These implement the generic Linux GPIO API on top of the other functions
 * in this header.
 */
static inline int gpio_request(unsigned gpio, const char *label)
{
	/* Compatibility shim */
	return 0;
}

static inline int gpio_free(unsigned gpio)
{
	/* Compatibility shim */
	return 0;
}

static inline int gpio_direction_input(unsigned gpio)
{
	mpc85xx_gpio_set_in(1U << gpio);
	return 0;
}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	if (value)
		mpc85xx_gpio_set_high(1U << gpio);
	else
		mpc85xx_gpio_set_low(1U << gpio);
	return 0;
}

static inline int gpio_get_value(unsigned gpio)
{
	return !!mpc85xx_gpio_get(1U << gpio);
}

static inline int gpio_set_value(unsigned gpio, int value)
{
	if (value)
		mpc85xx_gpio_set_high(1U << gpio);
	else
		mpc85xx_gpio_set_low(1U << gpio);
	return 0;
}

static inline int gpio_is_valid(int gpio)
{
	return (gpio >= 0) && (gpio < 32);
}

#endif /* not POWERPC_ASM_MPC85XX_GPIO_H_ */
