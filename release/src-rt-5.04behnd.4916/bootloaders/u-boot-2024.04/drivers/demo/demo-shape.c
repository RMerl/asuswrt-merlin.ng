// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <dm-demo.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/* Shape size */
#define WIDTH	8
#define HEIGHT	6

struct shape_data {
	int num_chars;	/* Number of non-space characters output so far */
	struct gpio_desc gpio_desc[8];
	int gpio_count;
};

/* Crazy little function to draw shapes on the console */
static int shape_hello(struct udevice *dev, int ch)
{
	const struct dm_demo_pdata *pdata = dev_get_plat(dev);
	struct shape_data *data = dev_get_priv(dev);
	static const struct shape {
		int start;
		int end;
		int dstart;
		int dend;
	} shapes[3] = {
		{ 0, 1, 0, 1 },
		{ 0, WIDTH, 0, 0 },
		{ HEIGHT / 2 - 1, WIDTH - HEIGHT / 2 + 1, -1, 1},
	};
	struct shape shape;
	unsigned int index;
	int line, pos, inside;
	const char *colour = pdata->colour;
	int first = 0;

	if (!ch)
		ch = pdata->default_char;
	if (!ch)
		ch = '@';

	index = (pdata->sides / 2) - 1;
	if (index >= ARRAY_SIZE(shapes))
		return -EIO;
	shape = shapes[index];

	for (line = 0; line < HEIGHT; line++) {
		first = 1;
		for (pos = 0; pos < WIDTH; pos++) {
			inside = pos >= shape.start && pos < shape.end;
			if (inside) {
				putc(first ? *colour++ : ch);
				data->num_chars++;
				first = 0;
				if (!*colour)
					colour = pdata->colour;
			} else {
				putc(' ');
			}
		}
		putc('\n');
		shape.start += shape.dstart;
		shape.end += shape.dend;
		if (shape.start < 0) {
			shape.dstart = -shape.dstart;
			shape.dend = -shape.dend;
			shape.start += shape.dstart;
			shape.end += shape.dend;
		}
	}

	return 0;
}

static int shape_status(struct udevice *dev, int *status)
{
	struct shape_data *data = dev_get_priv(dev);

	*status = data->num_chars;
	return 0;
}

static int set_light(struct udevice *dev, int light)
{
	struct shape_data *priv = dev_get_priv(dev);
	struct gpio_desc *desc;
	int ret;
	int i;

	desc = priv->gpio_desc;
	for (i = 0; i < priv->gpio_count; i++, desc++) {
		uint mask = 1 << i;

		ret = dm_gpio_set_value(desc, light & mask);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int get_light(struct udevice *dev)
{
	struct shape_data *priv = dev_get_priv(dev);
	struct gpio_desc *desc;
	uint value = 0;
	int ret;
	int i;

	desc = priv->gpio_desc;
	for (i = 0; i < priv->gpio_count; i++, desc++) {
		uint mask = 1 << i;

		ret = dm_gpio_get_value(desc);
		if (ret < 0)
			return ret;
		if (ret)
			value |= mask;
	}

	return value;
}

static const struct demo_ops shape_ops = {
	.hello = shape_hello,
	.status = shape_status,
	.get_light = get_light,
	.set_light = set_light,
};

static int shape_of_to_plat(struct udevice *dev)
{
	struct dm_demo_pdata *pdata = dev_get_plat(dev);
	int ret;

	/* Parse the data that is common with all demo devices */
	ret = demo_parse_dt(dev);
	if (ret)
		return ret;

	/* Parse the data that only we need */
	pdata->default_char = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					     "character", '@');

	return 0;
}

static int dm_shape_probe(struct udevice *dev)
{
	struct shape_data *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_list_by_name(dev, "light-gpios", priv->gpio_desc,
					ARRAY_SIZE(priv->gpio_desc),
					GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret < 0)
		return ret;
	priv->gpio_count = ret;
	debug("%s: %d GPIOs\n", __func__, priv->gpio_count);

	return 0;
}

static int dm_shape_remove(struct udevice *dev)
{
	struct shape_data *priv = dev_get_priv(dev);

	return gpio_free_list(dev, priv->gpio_desc, priv->gpio_count);
}

static const struct udevice_id demo_shape_id[] = {
	{ "demo-shape", 0 },
	{ },
};

U_BOOT_DRIVER(demo_shape_drv) = {
	.name	= "demo_shape_drv",
	.of_match = demo_shape_id,
	.id	= UCLASS_DEMO,
	.of_to_plat = shape_of_to_plat,
	.ops	= &shape_ops,
	.probe = dm_shape_probe,
	.remove = dm_shape_remove,
	.priv_auto	= sizeof(struct shape_data),
	.plat_auto	= sizeof(struct dm_demo_pdata),
};
