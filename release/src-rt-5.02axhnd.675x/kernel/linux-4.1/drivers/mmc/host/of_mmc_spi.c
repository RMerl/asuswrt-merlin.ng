/*
 * OpenFirmware bindings for the MMC-over-SPI driver
 *
 * Copyright (c) MontaVista Software, Inc. 2008.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/spi/spi.h>
#include <linux/spi/mmc_spi.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>

/* For archs that don't support NO_IRQ (such as mips), provide a dummy value */
#ifndef NO_IRQ
#define NO_IRQ 0
#endif

MODULE_LICENSE("GPL");

enum {
	CD_GPIO = 0,
	WP_GPIO,
	NUM_GPIOS,
};

struct of_mmc_spi {
	int gpios[NUM_GPIOS];
	bool alow_gpios[NUM_GPIOS];
	int detect_irq;
	struct mmc_spi_platform_data pdata;
};

static struct of_mmc_spi *to_of_mmc_spi(struct device *dev)
{
	return container_of(dev->platform_data, struct of_mmc_spi, pdata);
}

static int of_mmc_spi_init(struct device *dev,
			   irqreturn_t (*irqhandler)(int, void *), void *mmc)
{
	struct of_mmc_spi *oms = to_of_mmc_spi(dev);

	return request_threaded_irq(oms->detect_irq, NULL, irqhandler, 0,
				    dev_name(dev), mmc);
}

static void of_mmc_spi_exit(struct device *dev, void *mmc)
{
	struct of_mmc_spi *oms = to_of_mmc_spi(dev);

	free_irq(oms->detect_irq, mmc);
}

struct mmc_spi_platform_data *mmc_spi_get_pdata(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct device_node *np = dev->of_node;
	struct of_mmc_spi *oms;
	const u32 *voltage_ranges;
	int num_ranges;
	int i;
	int ret = -EINVAL;

	if (dev->platform_data || !np)
		return dev->platform_data;

	oms = kzalloc(sizeof(*oms), GFP_KERNEL);
	if (!oms)
		return NULL;

	voltage_ranges = of_get_property(np, "voltage-ranges", &num_ranges);
	num_ranges = num_ranges / sizeof(*voltage_ranges) / 2;
	if (!voltage_ranges || !num_ranges) {
		dev_err(dev, "OF: voltage-ranges unspecified\n");
		goto err_ocr;
	}

	for (i = 0; i < num_ranges; i++) {
		const int j = i * 2;
		u32 mask;

		mask = mmc_vddrange_to_ocrmask(be32_to_cpu(voltage_ranges[j]),
					       be32_to_cpu(voltage_ranges[j + 1]));
		if (!mask) {
			ret = -EINVAL;
			dev_err(dev, "OF: voltage-range #%d is invalid\n", i);
			goto err_ocr;
		}
		oms->pdata.ocr_mask |= mask;
	}

	for (i = 0; i < ARRAY_SIZE(oms->gpios); i++) {
		enum of_gpio_flags gpio_flags;

		oms->gpios[i] = of_get_gpio_flags(np, i, &gpio_flags);
		if (!gpio_is_valid(oms->gpios[i]))
			continue;

		if (gpio_flags & OF_GPIO_ACTIVE_LOW)
			oms->alow_gpios[i] = true;
	}

	if (gpio_is_valid(oms->gpios[CD_GPIO])) {
		oms->pdata.cd_gpio = oms->gpios[CD_GPIO];
		oms->pdata.flags |= MMC_SPI_USE_CD_GPIO;
		if (!oms->alow_gpios[CD_GPIO])
			oms->pdata.caps2 |= MMC_CAP2_CD_ACTIVE_HIGH;
	}
	if (gpio_is_valid(oms->gpios[WP_GPIO])) {
		oms->pdata.ro_gpio = oms->gpios[WP_GPIO];
		oms->pdata.flags |= MMC_SPI_USE_RO_GPIO;
		if (!oms->alow_gpios[WP_GPIO])
			oms->pdata.caps2 |= MMC_CAP2_RO_ACTIVE_HIGH;
	}

	oms->detect_irq = irq_of_parse_and_map(np, 0);
	if (oms->detect_irq != 0) {
		oms->pdata.init = of_mmc_spi_init;
		oms->pdata.exit = of_mmc_spi_exit;
	} else {
		oms->pdata.caps |= MMC_CAP_NEEDS_POLL;
	}

	dev->platform_data = &oms->pdata;
	return dev->platform_data;
err_ocr:
	kfree(oms);
	return NULL;
}
EXPORT_SYMBOL(mmc_spi_get_pdata);

void mmc_spi_put_pdata(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct device_node *np = dev->of_node;
	struct of_mmc_spi *oms = to_of_mmc_spi(dev);

	if (!dev->platform_data || !np)
		return;

	kfree(oms);
	dev->platform_data = NULL;
}
EXPORT_SYMBOL(mmc_spi_put_pdata);
