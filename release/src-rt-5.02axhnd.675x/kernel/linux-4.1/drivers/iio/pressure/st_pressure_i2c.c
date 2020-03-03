/*
 * STMicroelectronics pressures driver
 *
 * Copyright 2013 STMicroelectronics Inc.
 *
 * Denis Ciocca <denis.ciocca@st.com>
 *
 * Licensed under the GPL-2.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/iio/iio.h>

#include <linux/iio/common/st_sensors.h>
#include <linux/iio/common/st_sensors_i2c.h>
#include "st_pressure.h"

#ifdef CONFIG_OF
static const struct of_device_id st_press_of_match[] = {
	{
		.compatible = "st,lps001wp-press",
		.data = LPS001WP_PRESS_DEV_NAME,
	},
	{
		.compatible = "st,lps25h-press",
		.data = LPS25H_PRESS_DEV_NAME,
	},
	{
		.compatible = "st,lps331ap-press",
		.data = LPS331AP_PRESS_DEV_NAME,
	},
	{},
};
MODULE_DEVICE_TABLE(of, st_press_of_match);
#else
#define st_press_of_match NULL
#endif

static int st_press_i2c_probe(struct i2c_client *client,
						const struct i2c_device_id *id)
{
	struct iio_dev *indio_dev;
	struct st_sensor_data *press_data;
	int err;

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*press_data));
	if (!indio_dev)
		return -ENOMEM;

	press_data = iio_priv(indio_dev);
	st_sensors_of_i2c_probe(client, st_press_of_match);

	st_sensors_i2c_configure(indio_dev, client, press_data);

	err = st_press_common_probe(indio_dev);
	if (err < 0)
		return err;

	return 0;
}

static int st_press_i2c_remove(struct i2c_client *client)
{
	st_press_common_remove(i2c_get_clientdata(client));

	return 0;
}

static const struct i2c_device_id st_press_id_table[] = {
	{ LPS001WP_PRESS_DEV_NAME },
	{ LPS25H_PRESS_DEV_NAME },
	{ LPS331AP_PRESS_DEV_NAME },
	{},
};
MODULE_DEVICE_TABLE(i2c, st_press_id_table);

static struct i2c_driver st_press_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "st-press-i2c",
		.of_match_table = of_match_ptr(st_press_of_match),
	},
	.probe = st_press_i2c_probe,
	.remove = st_press_i2c_remove,
	.id_table = st_press_id_table,
};
module_i2c_driver(st_press_driver);

MODULE_AUTHOR("Denis Ciocca <denis.ciocca@st.com>");
MODULE_DESCRIPTION("STMicroelectronics pressures i2c driver");
MODULE_LICENSE("GPL v2");
