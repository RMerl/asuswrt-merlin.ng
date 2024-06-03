// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include "../pinctrl/renesas/sh_pfc.h"

#define GPIO_IOINTSEL	0x00	/* General IO/Interrupt Switching Register */
#define GPIO_INOUTSEL	0x04	/* General Input/Output Switching Register */
#define GPIO_OUTDT	0x08	/* General Output Register */
#define GPIO_INDT	0x0c	/* General Input Register */
#define GPIO_INTDT	0x10	/* Interrupt Display Register */
#define GPIO_INTCLR	0x14	/* Interrupt Clear Register */
#define GPIO_INTMSK	0x18	/* Interrupt Mask Register */
#define GPIO_MSKCLR	0x1c	/* Interrupt Mask Clear Register */
#define GPIO_POSNEG	0x20	/* Positive/Negative Logic Select Register */
#define GPIO_EDGLEVEL	0x24	/* Edge/level Select Register */
#define GPIO_FILONOFF	0x28	/* Chattering Prevention On/Off Register */
#define GPIO_BOTHEDGE	0x4c	/* One Edge/Both Edge Select Register */

#define RCAR_MAX_GPIO_PER_BANK		32

DECLARE_GLOBAL_DATA_PTR;

struct rcar_gpio_priv {
	void __iomem		*regs;
	int			pfc_offset;
};

static int rcar_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct rcar_gpio_priv *priv = dev_get_priv(dev);
	const u32 bit = BIT(offset);

	/*
	 * Testing on r8a7790 shows that INDT does not show correct pin state
	 * when configured as output, so use OUTDT in case of output pins.
	 */
	if (readl(priv->regs + GPIO_INOUTSEL) & bit)
		return !!(readl(priv->regs + GPIO_OUTDT) & bit);
	else
		return !!(readl(priv->regs + GPIO_INDT) & bit);
}

static int rcar_gpio_set_value(struct udevice *dev, unsigned offset,
			       int value)
{
	struct rcar_gpio_priv *priv = dev_get_priv(dev);

	if (value)
		setbits_le32(priv->regs + GPIO_OUTDT, BIT(offset));
	else
		clrbits_le32(priv->regs + GPIO_OUTDT, BIT(offset));

	return 0;
}

static void rcar_gpio_set_direction(void __iomem *regs, unsigned offset,
				    bool output)
{
	/*
	 * follow steps in the GPIO documentation for
	 * "Setting General Output Mode" and
	 * "Setting General Input Mode"
	 */

	/* Configure postive logic in POSNEG */
	clrbits_le32(regs + GPIO_POSNEG, BIT(offset));

	/* Select "General Input/Output Mode" in IOINTSEL */
	clrbits_le32(regs + GPIO_IOINTSEL, BIT(offset));

	/* Select Input Mode or Output Mode in INOUTSEL */
	if (output)
		setbits_le32(regs + GPIO_INOUTSEL, BIT(offset));
	else
		clrbits_le32(regs + GPIO_INOUTSEL, BIT(offset));
}

static int rcar_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct rcar_gpio_priv *priv = dev_get_priv(dev);

	rcar_gpio_set_direction(priv->regs, offset, false);

	return 0;
}

static int rcar_gpio_direction_output(struct udevice *dev, unsigned offset,
				      int value)
{
	struct rcar_gpio_priv *priv = dev_get_priv(dev);

	/* write GPIO value to output before selecting output mode of pin */
	rcar_gpio_set_value(dev, offset, value);
	rcar_gpio_set_direction(priv->regs, offset, true);

	return 0;
}

static int rcar_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct rcar_gpio_priv *priv = dev_get_priv(dev);

	if (readl(priv->regs + GPIO_INOUTSEL) & BIT(offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static int rcar_gpio_request(struct udevice *dev, unsigned offset,
			     const char *label)
{
	return pinctrl_gpio_request(dev, offset);
}

static int rcar_gpio_free(struct udevice *dev, unsigned offset)
{
	return pinctrl_gpio_free(dev, offset);
}

static const struct dm_gpio_ops rcar_gpio_ops = {
	.request		= rcar_gpio_request,
	.free			= rcar_gpio_free,
	.direction_input	= rcar_gpio_direction_input,
	.direction_output	= rcar_gpio_direction_output,
	.get_value		= rcar_gpio_get_value,
	.set_value		= rcar_gpio_set_value,
	.get_function		= rcar_gpio_get_function,
};

static int rcar_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rcar_gpio_priv *priv = dev_get_priv(dev);
	struct fdtdec_phandle_args args;
	struct clk clk;
	int node = dev_of_offset(dev);
	int ret;

	priv->regs = (void __iomem *)devfdt_get_addr(dev);
	uc_priv->bank_name = dev->name;

	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, node, "gpio-ranges",
					     NULL, 3, 0, &args);
	priv->pfc_offset = ret == 0 ? args.args[1] : -1;
	uc_priv->gpio_count = ret == 0 ? args.args[2] : RCAR_MAX_GPIO_PER_BANK;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "Failed to get GPIO bank clock\n");
		return ret;
	}

	ret = clk_enable(&clk);
	clk_free(&clk);
	if (ret) {
		dev_err(dev, "Failed to enable GPIO bank clock\n");
		return ret;
	}

	return 0;
}

static const struct udevice_id rcar_gpio_ids[] = {
	{ .compatible = "renesas,gpio-r8a7795" },
	{ .compatible = "renesas,gpio-r8a7796" },
	{ .compatible = "renesas,gpio-r8a77965" },
	{ .compatible = "renesas,gpio-r8a77970" },
	{ .compatible = "renesas,gpio-r8a77990" },
	{ .compatible = "renesas,gpio-r8a77995" },
	{ .compatible = "renesas,rcar-gen2-gpio" },
	{ .compatible = "renesas,rcar-gen3-gpio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(rcar_gpio) = {
	.name	= "rcar-gpio",
	.id	= UCLASS_GPIO,
	.of_match = rcar_gpio_ids,
	.ops	= &rcar_gpio_ops,
	.priv_auto_alloc_size = sizeof(struct rcar_gpio_priv),
	.probe	= rcar_gpio_probe,
};
