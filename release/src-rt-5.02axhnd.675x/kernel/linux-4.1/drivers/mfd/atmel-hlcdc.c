/*
 * Copyright (C) 2014 Free Electrons
 * Copyright (C) 2014 Atmel
 *
 * Author: Boris BREZILLON <boris.brezillon@free-electrons.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/clk.h>
#include <linux/mfd/atmel-hlcdc.h>
#include <linux/mfd/core.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#define ATMEL_HLCDC_REG_MAX		(0x4000 - 0x4)

static const struct mfd_cell atmel_hlcdc_cells[] = {
	{
		.name = "atmel-hlcdc-pwm",
		.of_compatible = "atmel,hlcdc-pwm",
	},
	{
		.name = "atmel-hlcdc-dc",
		.of_compatible = "atmel,hlcdc-display-controller",
	},
};

static const struct regmap_config atmel_hlcdc_regmap_config = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.max_register = ATMEL_HLCDC_REG_MAX,
};

static int atmel_hlcdc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct atmel_hlcdc *hlcdc;
	struct resource *res;
	void __iomem *regs;

	hlcdc = devm_kzalloc(dev, sizeof(*hlcdc), GFP_KERNEL);
	if (!hlcdc)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	hlcdc->irq = platform_get_irq(pdev, 0);
	if (hlcdc->irq < 0)
		return hlcdc->irq;

	hlcdc->periph_clk = devm_clk_get(dev, "periph_clk");
	if (IS_ERR(hlcdc->periph_clk)) {
		dev_err(dev, "failed to get peripheral clock\n");
		return PTR_ERR(hlcdc->periph_clk);
	}

	hlcdc->sys_clk = devm_clk_get(dev, "sys_clk");
	if (IS_ERR(hlcdc->sys_clk)) {
		dev_err(dev, "failed to get system clock\n");
		return PTR_ERR(hlcdc->sys_clk);
	}

	hlcdc->slow_clk = devm_clk_get(dev, "slow_clk");
	if (IS_ERR(hlcdc->slow_clk)) {
		dev_err(dev, "failed to get slow clock\n");
		return PTR_ERR(hlcdc->slow_clk);
	}

	hlcdc->regmap = devm_regmap_init_mmio(dev, regs,
					      &atmel_hlcdc_regmap_config);
	if (IS_ERR(hlcdc->regmap))
		return PTR_ERR(hlcdc->regmap);

	dev_set_drvdata(dev, hlcdc);

	return mfd_add_devices(dev, -1, atmel_hlcdc_cells,
			       ARRAY_SIZE(atmel_hlcdc_cells),
			       NULL, 0, NULL);
}

static int atmel_hlcdc_remove(struct platform_device *pdev)
{
	mfd_remove_devices(&pdev->dev);

	return 0;
}

static const struct of_device_id atmel_hlcdc_match[] = {
	{ .compatible = "atmel,sama5d3-hlcdc" },
	{ /* sentinel */ },
};

static struct platform_driver atmel_hlcdc_driver = {
	.probe = atmel_hlcdc_probe,
	.remove = atmel_hlcdc_remove,
	.driver = {
		.name = "atmel-hlcdc",
		.of_match_table = atmel_hlcdc_match,
	},
};
module_platform_driver(atmel_hlcdc_driver);

MODULE_ALIAS("platform:atmel-hlcdc");
MODULE_AUTHOR("Boris Brezillon <boris.brezillon@free-electrons.com>");
MODULE_DESCRIPTION("Atmel HLCDC driver");
MODULE_LICENSE("GPL v2");
