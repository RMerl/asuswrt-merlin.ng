// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <pwrseq.h>
#include <asm/gpio.h>
#include <linux/delay.h>

int mmc_pwrseq_get_power(struct udevice *dev, struct mmc_config *cfg)
{
	/* Enable power if needed */
	return uclass_get_device_by_phandle(UCLASS_PWRSEQ, dev, "mmc-pwrseq",
					   &cfg->pwr_dev);
}

static int mmc_pwrseq_set_power(struct udevice *dev, bool enable)
{
	struct gpio_desc reset;
	int ret;

	ret = gpio_request_by_name(dev, "reset-gpios", 0, &reset, GPIOD_IS_OUT);
	if (ret)
		return ret;
	dm_gpio_set_value(&reset, 1);
	udelay(1);
	dm_gpio_set_value(&reset, 0);
	udelay(200);

	return 0;
}

static const struct pwrseq_ops mmc_pwrseq_ops = {
	.set_power	= mmc_pwrseq_set_power,
};

static const struct udevice_id mmc_pwrseq_ids[] = {
	{ .compatible = "mmc-pwrseq-emmc" },
	{ }
};

U_BOOT_DRIVER(mmc_pwrseq_drv) = {
	.name		= "mmc_pwrseq_emmc",
	.id		= UCLASS_PWRSEQ,
	.of_match	= mmc_pwrseq_ids,
	.ops		= &mmc_pwrseq_ops,
};
