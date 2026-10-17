// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <sysinfo.h>
#include <i2c.h>
#include <log.h>
#include <asm/gpio.h>

#include "gazerbeam.h"

/* Sequence number of I2C bus that holds the GPIO expanders */
static const int I2C_BUS_SEQ_NO = 1;

/* I2C address of SC/MC2 expander */
static const int MC2_EXPANDER_ADDR = 0x20;
/* I2C address of MC4 expander */
static const int MC4_EXPANDER_ADDR = 0x22;

/* Number of the GPIO to read the SC data from */
static const int SC_GPIO_NO;
/* Number of the GPIO to read the CON data from */
static const int CON_GPIO_NO = 1;

/**
 * struct sysinfo_gazerbeam_priv - Private data structure for the gazerbeam
 *	sysinfo driver
 * @reset_gpios:  GPIOs for the sysinfo's reset GPIOs.
 * @var_gpios:	  GPIOs for the sysinfo's hardware variant GPIOs
 * @ver_gpios:	  GPIOs for the sysinfo's hardware version GPIOs
 * @variant:	  Container for the sysinfo's hardware variant (CON/CPU)
 * @multichannel: Container for the sysinfo's multichannel variant (MC4/MC2/SC)
 * @hwversion:	  Container for the sysinfo's hardware version
 */
struct sysinfo_gazerbeam_priv {
	struct gpio_desc reset_gpios[2];
	struct gpio_desc var_gpios[2];
	struct gpio_desc ver_gpios[4];
	int variant;
	int multichannel;
	int hwversion;
};

/**
 * _read_sysinfo_variant_data() - Read variant information from the hardware.
 * @dev: The sysinfo device for which to determine the multichannel and device
 *	 type information.
 *
 * The data read from the sysinfo's hardware (mostly hard-wired GPIOs) is stored
 * in the private data structure of the driver to be used by other driver
 * methods.
 *
 * Return: 0 if OK, -ve on error.
 */
static int _read_sysinfo_variant_data(struct udevice *dev)
{
	struct sysinfo_gazerbeam_priv *priv = dev_get_priv(dev);
	struct udevice *i2c_bus;
	struct udevice *dummy;
	char *listname;
	int mc4, mc2, sc, mc2_sc, con;
	int gpio_num;
	int res;

	res = uclass_get_device_by_seq(UCLASS_I2C, I2C_BUS_SEQ_NO, &i2c_bus);
	if (res) {
		debug("%s: Could not get I2C bus %d (err = %d)\n",
		      dev->name, I2C_BUS_SEQ_NO, res);
		return res;
	}

	if (!i2c_bus) {
		debug("%s: Could not get I2C bus %d\n",
		      dev->name, I2C_BUS_SEQ_NO);
		return -EIO;
	}

	mc2_sc = !dm_i2c_probe(i2c_bus, MC2_EXPANDER_ADDR, 0, &dummy);
	mc4 = !dm_i2c_probe(i2c_bus, MC4_EXPANDER_ADDR, 0, &dummy);

	if (mc2_sc && mc4) {
		debug("%s: Board hardware configuration inconsistent.\n",
		      dev->name);
		return -EINVAL;
	}

	listname = mc2_sc ? "var-gpios-mc2" : "var-gpios-mc4";

	gpio_num = gpio_request_list_by_name(dev, listname, priv->var_gpios,
					     ARRAY_SIZE(priv->var_gpios),
					     GPIOD_IS_IN);
	if (gpio_num < 0) {
		debug("%s: Requesting gpio list %s failed (err = %d).\n",
		      dev->name, listname, gpio_num);
		return gpio_num;
	}

	sc = dm_gpio_get_value(&priv->var_gpios[SC_GPIO_NO]);
	if (sc < 0) {
		debug("%s: Error while reading 'sc' GPIO (err = %d)",
		      dev->name, sc);
		return sc;
	}

	mc2 = mc2_sc ? (sc ? 0 : 1) : 0;

	if ((sc && mc2) || (sc && mc4) || (!sc && !mc2 && !mc4)) {
		debug("%s: Board hardware configuration inconsistent.\n",
		      dev->name);
		return -EINVAL;
	}

	con = dm_gpio_get_value(&priv->var_gpios[CON_GPIO_NO]);
	if (con < 0) {
		debug("%s: Error while reading 'con' GPIO (err = %d)",
		      dev->name, con);
		return con;
	}

	priv->variant = con ? VAR_CON : VAR_CPU;

	priv->multichannel = mc4 ? 4 : (mc2 ? 2 : (sc ? 1 : 0));

	return 0;
}

/**
 * _read_hwversion() - Read the hardware version from the sysinfo.
 * @dev: The sysinfo device for which to read the hardware version.
 *
 * The hardware version read from the sysinfo (from hard-wired GPIOs) is stored
 * in the private data structure of the driver to be used by other driver
 * methods.
 *
 * Return: 0 if OK, -ve on error.
 */
static int _read_hwversion(struct udevice *dev)
{
	struct sysinfo_gazerbeam_priv *priv = dev_get_priv(dev);
	int res;

	res = gpio_request_list_by_name(dev, "ver-gpios", priv->ver_gpios,
					ARRAY_SIZE(priv->ver_gpios),
					GPIOD_IS_IN);
	if (res < 0) {
		debug("%s: Error getting GPIO list 'ver-gpios' (err = %d)\n",
		      dev->name, res);
		return -ENODEV;
	}

	res = dm_gpio_get_values_as_int(priv->ver_gpios,
					ARRAY_SIZE(priv->ver_gpios));
	if (res < 0) {
		debug("%s: Error reading HW version from expander (err = %d)\n",
		      dev->name, res);
		return res;
	}

	priv->hwversion = res;

	res = gpio_free_list(dev, priv->ver_gpios, ARRAY_SIZE(priv->ver_gpios));
	if (res < 0) {
		debug("%s: Error freeing HW version GPIO list (err = %d)\n",
		      dev->name, res);
		return res;
	}

	return 0;
}

static int sysinfo_gazerbeam_detect(struct udevice *dev)
{
	int res;

	res = _read_sysinfo_variant_data(dev);
	if (res) {
		debug("%s: Error reading multichannel variant (err = %d)\n",
		      dev->name, res);
		return res;
	}

	res = _read_hwversion(dev);
	if (res) {
		debug("%s: Error reading hardware version (err = %d)\n",
		      dev->name, res);
		return res;
	}

	return 0;
}

static int sysinfo_gazerbeam_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_gazerbeam_priv *priv = dev_get_priv(dev);

	switch (id) {
	case BOARD_MULTICHANNEL:
		*val = priv->multichannel;
		break;
	case BOARD_VARIANT:
		*val = priv->variant;
		break;
	case BOARD_HWVERSION:
		*val = priv->hwversion;
		break;
	default:
		debug("%s: Integer value %d unknown\n", dev->name, id);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id sysinfo_gazerbeam_ids[] = {
	{ .compatible = "gdsys,sysinfo-gazerbeam" },
	{ /* sentinel */ }
};

static const struct sysinfo_ops sysinfo_gazerbeam_ops = {
	.detect = sysinfo_gazerbeam_detect,
	.get_int = sysinfo_gazerbeam_get_int,
};

static int sysinfo_gazerbeam_probe(struct udevice *dev)
{
	struct sysinfo_gazerbeam_priv *priv = dev_get_priv(dev);
	int gpio_num, i;

	gpio_num = gpio_request_list_by_name(dev, "reset-gpios",
					     priv->reset_gpios,
					     ARRAY_SIZE(priv->reset_gpios),
					     GPIOD_IS_OUT);

	if (gpio_num < 0) {
		debug("%s: Error getting GPIO list 'reset-gpios' (err = %d)\n",
		      dev->name, gpio_num);
		return gpio_num;
	}

	/* Set startup-finished GPIOs */
	for (i = 0; i < ARRAY_SIZE(priv->reset_gpios); i++) {
		int res = dm_gpio_set_value(&priv->reset_gpios[i], 0);

		if (res) {
			debug("%s: Error while setting GPIO %d (err = %d)\n",
			      dev->name, i, res);
			return res;
		}
	}

	return 0;
}

U_BOOT_DRIVER(sysinfo_gazerbeam) = {
	.name           = "sysinfo_gazerbeam",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_gazerbeam_ids,
	.ops		= &sysinfo_gazerbeam_ops,
	.priv_auto	= sizeof(struct sysinfo_gazerbeam_priv),
	.probe          = sysinfo_gazerbeam_probe,
};
