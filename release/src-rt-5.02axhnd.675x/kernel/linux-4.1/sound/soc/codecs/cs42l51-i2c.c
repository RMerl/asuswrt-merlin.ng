/*
 * cs42l56.c -- CS42L51 ALSA SoC I2C audio driver
 *
 * Copyright 2014 CirrusLogic, Inc.
 *
 * Author: Brian Austin <brian.austin@cirrus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <sound/soc.h>

#include "cs42l51.h"

static struct i2c_device_id cs42l51_i2c_id[] = {
	{"cs42l51", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, cs42l51_i2c_id);

static int cs42l51_i2c_probe(struct i2c_client *i2c,
			     const struct i2c_device_id *id)
{
	struct regmap_config config;

	config = cs42l51_regmap;
	config.val_bits = 8;
	config.reg_bits = 8;

	return cs42l51_probe(&i2c->dev, devm_regmap_init_i2c(i2c, &config));
}

static int cs42l51_i2c_remove(struct i2c_client *i2c)
{
	snd_soc_unregister_codec(&i2c->dev);

	return 0;
}

static struct i2c_driver cs42l51_i2c_driver = {
	.driver = {
		.name = "cs42l51",
		.owner = THIS_MODULE,
		.of_match_table = cs42l51_of_match,
	},
	.probe = cs42l51_i2c_probe,
	.remove = cs42l51_i2c_remove,
	.id_table = cs42l51_i2c_id,
};

module_i2c_driver(cs42l51_i2c_driver);

MODULE_DESCRIPTION("ASoC CS42L51 I2C Driver");
MODULE_AUTHOR("Brian Austin, Cirrus Logic Inc, <brian.austin@cirrus.com>");
MODULE_LICENSE("GPL");
