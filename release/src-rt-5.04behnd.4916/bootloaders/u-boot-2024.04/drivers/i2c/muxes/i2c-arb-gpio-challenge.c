// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

struct i2c_arbitrator_priv {
	struct gpio_desc ap_claim;
	struct gpio_desc ec_claim;
	uint slew_delay_us;
	uint wait_retry_ms;
	uint wait_free_ms;
};

int i2c_arbitrator_deselect(struct udevice *mux, struct udevice *bus,
			    uint channel)
{
	struct i2c_arbitrator_priv *priv = dev_get_priv(mux);
	int ret;

	debug("%s: %s\n", __func__, mux->name);
	ret = dm_gpio_set_value(&priv->ap_claim, 0);
	udelay(priv->slew_delay_us);

	return ret;
}

int i2c_arbitrator_select(struct udevice *mux, struct udevice *bus,
			  uint channel)
{
	struct i2c_arbitrator_priv *priv = dev_get_priv(mux);
	unsigned start;
	int ret;

	debug("%s: %s\n", __func__, mux->name);
	/* Start a round of trying to claim the bus */
	start = get_timer(0);
	do {
		unsigned start_retry;
		int waiting = 0;

		/* Indicate that we want to claim the bus */
		ret = dm_gpio_set_value(&priv->ap_claim, 1);
		if (ret)
			goto err;
		udelay(priv->slew_delay_us);

		/* Wait for the EC to release it */
		start_retry = get_timer(0);
		while (get_timer(start_retry) < priv->wait_retry_ms) {
			ret = dm_gpio_get_value(&priv->ec_claim);
			if (ret < 0) {
				goto err;
			} else if (!ret) {
				/* We got it, so return */
				return 0;
			}

			if (!waiting)
				waiting = 1;
		}

		/* It didn't release, so give up, wait, and try again */
		ret = dm_gpio_set_value(&priv->ap_claim, 0);
		if (ret)
			goto err;

		mdelay(priv->wait_retry_ms);
	} while (get_timer(start) < priv->wait_free_ms);

	/* Give up, release our claim */
	printf("I2C: Could not claim bus, timeout %lu\n", get_timer(start));
	ret = -ETIMEDOUT;
	ret = 0;
err:
	return ret;
}

static int i2c_arbitrator_probe(struct udevice *dev)
{
	struct i2c_arbitrator_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	debug("%s: %s\n", __func__, dev->name);
	priv->slew_delay_us = fdtdec_get_int(blob, node, "slew-delay-us", 0);
	priv->wait_retry_ms = fdtdec_get_int(blob, node, "wait-retry-us", 0) /
		1000;
	priv->wait_free_ms = fdtdec_get_int(blob, node, "wait-free-us", 0) /
		1000;
	ret = gpio_request_by_name(dev, "our-claim-gpio", 0, &priv->ap_claim,
				   GPIOD_IS_OUT);
	if (ret)
		goto err;
	ret = gpio_request_by_name(dev, "their-claim-gpios", 0, &priv->ec_claim,
				   GPIOD_IS_IN);
	if (ret)
		goto err_ec_gpio;

	return 0;

err_ec_gpio:
	dm_gpio_free(dev, &priv->ap_claim);
err:
	debug("%s: ret=%d\n", __func__, ret);
	return ret;
}

static int i2c_arbitrator_remove(struct udevice *dev)
{
	struct i2c_arbitrator_priv *priv = dev_get_priv(dev);

	dm_gpio_free(dev, &priv->ap_claim);
	dm_gpio_free(dev, &priv->ec_claim);

	return 0;
}

static const struct i2c_mux_ops i2c_arbitrator_ops = {
	.select		= i2c_arbitrator_select,
	.deselect	= i2c_arbitrator_deselect,
};

static const struct udevice_id i2c_arbitrator_ids[] = {
	{ .compatible = "i2c-arb-gpio-challenge" },
	{ }
};

U_BOOT_DRIVER(i2c_arbitrator) = {
	.name = "i2c_arbitrator",
	.id = UCLASS_I2C_MUX,
	.of_match = i2c_arbitrator_ids,
	.probe = i2c_arbitrator_probe,
	.remove = i2c_arbitrator_remove,
	.ops = &i2c_arbitrator_ops,
	.priv_auto	= sizeof(struct i2c_arbitrator_priv),
};
