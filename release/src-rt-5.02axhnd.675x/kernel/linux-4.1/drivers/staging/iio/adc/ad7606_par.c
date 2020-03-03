/*
 * AD7606 Parallel Interface ADC driver
 *
 * Copyright 2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/io.h>

#include <linux/iio/iio.h>
#include "ad7606.h"

static int ad7606_par16_read_block(struct device *dev,
				 int count, void *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct ad7606_state *st = iio_priv(indio_dev);

	insw((unsigned long) st->base_address, buf, count);

	return 0;
}

static const struct ad7606_bus_ops ad7606_par16_bops = {
	.read_block	= ad7606_par16_read_block,
};

static int ad7606_par8_read_block(struct device *dev,
				 int count, void *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct ad7606_state *st = iio_priv(indio_dev);

	insb((unsigned long) st->base_address, buf, count * 2);

	return 0;
}

static const struct ad7606_bus_ops ad7606_par8_bops = {
	.read_block	= ad7606_par8_read_block,
};

static int ad7606_par_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct iio_dev *indio_dev;
	void __iomem *addr;
	resource_size_t remap_size;
	int irq;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq\n");
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	addr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(addr))
		return PTR_ERR(addr);

	remap_size = resource_size(res);

	indio_dev = ad7606_probe(&pdev->dev, irq, addr,
			  platform_get_device_id(pdev)->driver_data,
			  remap_size > 1 ? &ad7606_par16_bops :
			  &ad7606_par8_bops);

	if (IS_ERR(indio_dev))
		return PTR_ERR(indio_dev);

	platform_set_drvdata(pdev, indio_dev);

	return 0;
}

static int ad7606_par_remove(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);

	ad7606_remove(indio_dev, platform_get_irq(pdev, 0));

	return 0;
}

#ifdef CONFIG_PM
static int ad7606_par_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);

	ad7606_suspend(indio_dev);

	return 0;
}

static int ad7606_par_resume(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);

	ad7606_resume(indio_dev);

	return 0;
}

static const struct dev_pm_ops ad7606_pm_ops = {
	.suspend = ad7606_par_suspend,
	.resume  = ad7606_par_resume,
};
#define AD7606_PAR_PM_OPS (&ad7606_pm_ops)

#else
#define AD7606_PAR_PM_OPS NULL
#endif  /* CONFIG_PM */

static struct platform_device_id ad7606_driver_ids[] = {
	{
		.name		= "ad7606-8",
		.driver_data	= ID_AD7606_8,
	}, {
		.name		= "ad7606-6",
		.driver_data	= ID_AD7606_6,
	}, {
		.name		= "ad7606-4",
		.driver_data	= ID_AD7606_4,
	},
	{ }
};

MODULE_DEVICE_TABLE(platform, ad7606_driver_ids);

static struct platform_driver ad7606_driver = {
	.probe = ad7606_par_probe,
	.remove	= ad7606_par_remove,
	.id_table = ad7606_driver_ids,
	.driver = {
		.name	 = "ad7606",
		.pm    = AD7606_PAR_PM_OPS,
	},
};

module_platform_driver(ad7606_driver);

MODULE_AUTHOR("Michael Hennerich <hennerich@blackfin.uclinux.org>");
MODULE_DESCRIPTION("Analog Devices AD7606 ADC");
MODULE_LICENSE("GPL v2");
