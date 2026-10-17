/* SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright (c) 2015 Free Electrons
 * Copyright (c) 2015 NextThing Co
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <w1.h>
#include <linux/delay.h>

#include <asm/gpio.h>

#define W1_TIMING_A	6
#define W1_TIMING_B	64
#define W1_TIMING_C	60
#define W1_TIMING_D	10
#define W1_TIMING_E	9
#define W1_TIMING_F	55
#define W1_TIMING_G	0
#define W1_TIMING_H	480
#define W1_TIMING_I	70
#define W1_TIMING_J	410

struct w1_gpio_pdata {
	struct gpio_desc	gpio;
	u64			search_id;
};

static bool w1_gpio_read_bit(struct udevice *dev)
{
	struct w1_gpio_pdata *pdata = dev_get_plat(dev);
	int val;

	dm_gpio_set_dir_flags(&pdata->gpio, GPIOD_IS_OUT);
	udelay(W1_TIMING_A);

	dm_gpio_set_dir_flags(&pdata->gpio, GPIOD_IS_IN);
	udelay(W1_TIMING_E);

	val = dm_gpio_get_value(&pdata->gpio);
	if (val < 0)
		debug("error in retrieving GPIO value");
	udelay(W1_TIMING_F);

	return val;
}

static u8 w1_gpio_read_byte(struct udevice *dev)
{
	int i;
	u8 ret = 0;

	for (i = 0; i < 8; ++i)
		ret |= (w1_gpio_read_bit(dev) ? 1 : 0) << i;

	return ret;
}

static void w1_gpio_write_bit(struct udevice *dev, bool bit)
{
	struct w1_gpio_pdata *pdata = dev_get_plat(dev);

	dm_gpio_set_dir_flags(&pdata->gpio, GPIOD_IS_OUT);

	bit ? udelay(W1_TIMING_A) : udelay(W1_TIMING_C);

	dm_gpio_set_value(&pdata->gpio, 1);

	bit ? udelay(W1_TIMING_B) : udelay(W1_TIMING_D);
}

static void w1_gpio_write_byte(struct udevice *dev, u8 byte)
{
	int i;

	for (i = 0; i < 8; ++i)
		w1_gpio_write_bit(dev, (byte >> i) & 0x1);
}

static bool w1_gpio_reset(struct udevice *dev)
{
	struct w1_gpio_pdata *pdata = dev_get_plat(dev);
	int val;

	/* initiate the reset pulse. first we must pull the bus to low */
	dm_gpio_set_dir_flags(&pdata->gpio, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	udelay(W1_TIMING_G);

	dm_gpio_set_value(&pdata->gpio, 0);
	/* wait for the specified time with the bus kept low */
	udelay(W1_TIMING_H);

	/* now we must read the presence pulse */
	dm_gpio_set_dir_flags(&pdata->gpio, GPIOD_IS_IN);
	udelay(W1_TIMING_I);

	val = dm_gpio_get_value(&pdata->gpio);
	if (val < 0)
		debug("error in retrieving GPIO value");

	/* if nobody pulled the bus down , it means nobody is on the bus */
	if (val != 0)
		return 1;
	/* we have the bus pulled down, let's wait for the specified presence time */
	udelay(W1_TIMING_J);

	/* read again, the other end should leave the bus free */
	val = dm_gpio_get_value(&pdata->gpio);
	if (val < 0)
		debug("error in retrieving GPIO value");

	/* bus is not going up again, so we have an error */
	if (val != 1)
		return 1;

	/* all good, presence detected */
	return 0;
}

static u8 w1_gpio_triplet(struct udevice *dev, bool bdir)
{
	u8 id_bit   = w1_gpio_read_bit(dev);
	u8 comp_bit = w1_gpio_read_bit(dev);
	u8 retval;

	if (id_bit && comp_bit)
		return 0x03;  /* error */

	if (!id_bit && !comp_bit) {
		/* Both bits are valid, take the direction given */
		retval = bdir ? 0x04 : 0;
	} else {
		/* Only one bit is valid, take that direction */
		bdir = id_bit;
		retval = id_bit ? 0x05 : 0x02;
	}

	w1_gpio_write_bit(dev, bdir);
	return retval;
}

static const struct w1_ops w1_gpio_ops = {
	.read_byte	= w1_gpio_read_byte,
	.reset		= w1_gpio_reset,
	.triplet	= w1_gpio_triplet,
	.write_byte	= w1_gpio_write_byte,
};

static int w1_gpio_of_to_plat(struct udevice *dev)
{
	struct w1_gpio_pdata *pdata = dev_get_plat(dev);
	int ret;

	ret = gpio_request_by_name(dev, "gpios", 0, &pdata->gpio, GPIOD_IS_IN);
	if (ret < 0)
		printf("Error claiming GPIO %d\n", ret);

	return ret;
};

static const struct udevice_id w1_gpio_id[] = {
	{ "w1-gpio", 0 },
	{ },
};

U_BOOT_DRIVER(w1_gpio_drv) = {
	.id				= UCLASS_W1,
	.name				= "w1_gpio_drv",
	.of_match			= w1_gpio_id,
	.of_to_plat		= w1_gpio_of_to_plat,
	.ops				= &w1_gpio_ops,
	.plat_auto	= sizeof(struct w1_gpio_pdata),
};
