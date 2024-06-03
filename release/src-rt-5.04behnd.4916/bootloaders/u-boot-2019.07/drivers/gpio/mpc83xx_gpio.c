// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale MPC83xx GPIO handling.
 */

#include <common.h>
#include <mpc83xx.h>
#include <asm/gpio.h>
#include <asm/io.h>

#ifndef CONFIG_MPC83XX_GPIO_0_INIT_DIRECTION
#define CONFIG_MPC83XX_GPIO_0_INIT_DIRECTION 0
#endif
#ifndef CONFIG_MPC83XX_GPIO_1_INIT_DIRECTION
#define CONFIG_MPC83XX_GPIO_1_INIT_DIRECTION 0
#endif
#ifndef CONFIG_MPC83XX_GPIO_0_INIT_OPEN_DRAIN
#define CONFIG_MPC83XX_GPIO_0_INIT_OPEN_DRAIN 0
#endif
#ifndef CONFIG_MPC83XX_GPIO_1_INIT_OPEN_DRAIN
#define CONFIG_MPC83XX_GPIO_1_INIT_OPEN_DRAIN 0
#endif
#ifndef CONFIG_MPC83XX_GPIO_0_INIT_VALUE
#define CONFIG_MPC83XX_GPIO_0_INIT_VALUE 0
#endif
#ifndef CONFIG_MPC83XX_GPIO_1_INIT_VALUE
#define CONFIG_MPC83XX_GPIO_1_INIT_VALUE 0
#endif

static unsigned int gpio_output_value[MPC83XX_GPIO_CTRLRS];

/*
 * Generic_GPIO primitives.
 */

int gpio_request(unsigned gpio, const char *label)
{
	if (gpio >= MAX_NUM_GPIOS)
		return -1;

	return 0;
}

int gpio_free(unsigned gpio)
{
	/* Do not set to input */
	return 0;
}

/* set GPIO pin 'gpio' as an input */
int gpio_direction_input(unsigned gpio)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	unsigned int ctrlr;
	unsigned int line;
	unsigned int line_mask;

	/* 32-bits per controller */
	ctrlr = gpio >> 5;
	line = gpio & (0x1F);

	/* Big endian */
	line_mask = 1 << (31 - line);

	clrbits_be32(&im->gpio[ctrlr].dir, line_mask);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
int gpio_direction_output(unsigned gpio, int value)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	unsigned int ctrlr;
	unsigned int line;
	unsigned int line_mask;

	if (value != 0 && value != 1) {
		printf("Error: Value parameter must be 0 or 1.\n");
		return -1;
	}

	gpio_set_value(gpio, value);

	/* 32-bits per controller */
	ctrlr = gpio >> 5;
	line = gpio & (0x1F);

	/* Big endian */
	line_mask = 1 << (31 - line);

	/* Make the line output */
	setbits_be32(&im->gpio[ctrlr].dir, line_mask);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
int gpio_get_value(unsigned gpio)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	unsigned int ctrlr;
	unsigned int line;
	unsigned int line_mask;

	/* 32-bits per controller */
	ctrlr = gpio >> 5;
	line = gpio & (0x1F);

	/* Big endian */
	line_mask = 1 << (31 - line);

	/* Read the value and mask off the bit */
	return (in_be32(&im->gpio[ctrlr].dat) & line_mask) != 0;
}

/* write GPIO OUT value to pin 'gpio' */
int gpio_set_value(unsigned gpio, int value)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	unsigned int ctrlr;
	unsigned int line;
	unsigned int line_mask;

	if (value != 0 && value != 1) {
		printf("Error: Value parameter must be 0 or 1.\n");
		return -1;
	}

	/* 32-bits per controller */
	ctrlr = gpio >> 5;
	line = gpio & (0x1F);

	/* Big endian */
	line_mask = 1 << (31 - line);

	/* Update the local output buffer soft copy */
	gpio_output_value[ctrlr] =
		(gpio_output_value[ctrlr] & ~line_mask) | \
			(value ? line_mask : 0);

	/* Write the output */
	out_be32(&im->gpio[ctrlr].dat, gpio_output_value[ctrlr]);

	return 0;
}

/* Configure GPIO registers early */
void mpc83xx_gpio_init_f(void)
{
	immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

#if MPC83XX_GPIO_CTRLRS >= 1
	out_be32(&im->gpio[0].dir, CONFIG_MPC83XX_GPIO_0_INIT_DIRECTION);
	out_be32(&im->gpio[0].odr, CONFIG_MPC83XX_GPIO_0_INIT_OPEN_DRAIN);
	out_be32(&im->gpio[0].dat, CONFIG_MPC83XX_GPIO_0_INIT_VALUE);
	out_be32(&im->gpio[0].ier, 0xFFFFFFFF); /* Clear all events */
	out_be32(&im->gpio[0].imr, 0);
	out_be32(&im->gpio[0].icr, 0);
#endif

#if MPC83XX_GPIO_CTRLRS >= 2
	out_be32(&im->gpio[1].dir, CONFIG_MPC83XX_GPIO_1_INIT_DIRECTION);
	out_be32(&im->gpio[1].odr, CONFIG_MPC83XX_GPIO_1_INIT_OPEN_DRAIN);
	out_be32(&im->gpio[1].dat, CONFIG_MPC83XX_GPIO_1_INIT_VALUE);
	out_be32(&im->gpio[1].ier, 0xFFFFFFFF); /* Clear all events */
	out_be32(&im->gpio[1].imr, 0);
	out_be32(&im->gpio[1].icr, 0);
#endif
}

/* Initialize GPIO soft-copies */
void mpc83xx_gpio_init_r(void)
{
#if MPC83XX_GPIO_CTRLRS >= 1
	gpio_output_value[0] = CONFIG_MPC83XX_GPIO_0_INIT_VALUE;
#endif

#if MPC83XX_GPIO_CTRLRS >= 2
	gpio_output_value[1] = CONFIG_MPC83XX_GPIO_1_INIT_VALUE;
#endif
}
