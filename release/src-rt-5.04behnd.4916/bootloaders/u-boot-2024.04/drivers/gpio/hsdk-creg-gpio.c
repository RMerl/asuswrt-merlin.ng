/*
 * Synopsys HSDK SDP Generic PLL clock driver
 *
 * Copyright (C) 2017 Synopsys
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <log.h>
#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/printk.h>

#define DRV_NAME	"gpio_creg"

struct hsdk_creg_gpio {
	u32	*regs;
	u8	shift;
	u8	activate;
	u8	deactivate;
	u8	bit_per_gpio;
};

static int hsdk_creg_gpio_set_value(struct udevice *dev, unsigned oft, int val)
{
	struct hsdk_creg_gpio *hcg = dev_get_priv(dev);
	u8 reg_shift = oft * hcg->bit_per_gpio + hcg->shift;
	u32 reg = readl(hcg->regs);

	reg &= ~(GENMASK(hcg->bit_per_gpio - 1, 0) << reg_shift);
	reg |=  ((val ? hcg->deactivate : hcg->activate) << reg_shift);

	writel(reg, hcg->regs);

	return 0;
}

static int hsdk_creg_gpio_direction_output(struct udevice *dev, unsigned oft,
					   int val)
{
	hsdk_creg_gpio_set_value(dev, oft, val);

	return 0;
}

static int hsdk_creg_gpio_direction_input(struct udevice *dev, unsigned oft)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	pr_err("%s can't be used as input!\n", uc_priv->bank_name);

	return -ENOTSUPP;
}

static int hsdk_creg_gpio_get_value(struct udevice *dev, unsigned int oft)
{
	struct hsdk_creg_gpio *hcg = dev_get_priv(dev);
	u32 val = readl(hcg->regs);

	val >>= oft * hcg->bit_per_gpio + hcg->shift;
	val &= GENMASK(hcg->bit_per_gpio - 1, 0);
	return (val == hcg->deactivate) ? 1 : 0;
}

static const struct dm_gpio_ops hsdk_creg_gpio_ops = {
	.direction_output	= hsdk_creg_gpio_direction_output,
	.direction_input	= hsdk_creg_gpio_direction_input,
	.set_value		= hsdk_creg_gpio_set_value,
	.get_value		= hsdk_creg_gpio_get_value,
};

static int hsdk_creg_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct hsdk_creg_gpio *hcg = dev_get_priv(dev);
	u32 shift, bit_per_gpio, activate, deactivate, gpio_count;
	const u8 *defaults;

	hcg->regs = dev_read_addr_ptr(dev);
	gpio_count = dev_read_u32_default(dev, "gpio-count", 1);
	shift = dev_read_u32_default(dev, "gpio-first-shift", 0);
	bit_per_gpio = dev_read_u32_default(dev, "gpio-bit-per-line", 1);
	activate = dev_read_u32_default(dev, "gpio-activate-val", 1);
	deactivate = dev_read_u32_default(dev, "gpio-deactivate-val", 0);
	defaults = dev_read_u8_array_ptr(dev, "gpio-default-val", gpio_count);

	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");
	if (!uc_priv->bank_name)
		uc_priv->bank_name = dev_read_name(dev);

	if (!bit_per_gpio) {
		pr_err("%s: 'gpio-bit-per-line' can't be 0\n",
		       uc_priv->bank_name);

		return -EINVAL;
	}

	if (!gpio_count) {
		pr_err("%s: 'gpio-count' can't be 0\n",
		       uc_priv->bank_name);

		return -EINVAL;
	}

	if ((gpio_count * bit_per_gpio + shift) > 32) {
		pr_err("%s: u32 io register overflow: try to use %u bits\n",
		       uc_priv->bank_name, gpio_count * bit_per_gpio + shift);

		return -EINVAL;
	}

	if (GENMASK(31, bit_per_gpio) & activate) {
		pr_err("%s: 'gpio-activate-val' can't be more than %lu\n",
		       uc_priv->bank_name, GENMASK(bit_per_gpio - 1, 0));

		return -EINVAL;
	}

	if (GENMASK(31, bit_per_gpio) & deactivate) {
		pr_err("%s: 'gpio-deactivate-val' can't be more than %lu\n",
		       uc_priv->bank_name, GENMASK(bit_per_gpio - 1, 0));

		return -EINVAL;
	}

	if (activate == deactivate) {
		pr_err("%s: 'gpio-deactivate-val' and 'gpio-activate-val' can't be equal\n",
		       uc_priv->bank_name);

		return -EINVAL;
	}

	hcg->shift = (u8)shift;
	hcg->bit_per_gpio = (u8)bit_per_gpio;
	hcg->activate = (u8)activate;
	hcg->deactivate = (u8)deactivate;
	uc_priv->gpio_count = gpio_count;

	/* Setup default GPIO value if we have "gpio-default-val" array */
	if (defaults)
		for (u8 i = 0; i < gpio_count; i++)
			hsdk_creg_gpio_set_value(dev, i, defaults[i]);

	pr_debug("%s GPIO [0x%p] controller with %d gpios probed\n",
		 uc_priv->bank_name, hcg->regs, uc_priv->gpio_count);

	return 0;
}

static const struct udevice_id hsdk_creg_gpio_ids[] = {
	{ .compatible = "snps,creg-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_hsdk_creg) = {
	.name	= DRV_NAME,
	.id	= UCLASS_GPIO,
	.ops	= &hsdk_creg_gpio_ops,
	.probe	= hsdk_creg_gpio_probe,
	.of_match = hsdk_creg_gpio_ids,
	.plat_auto	= sizeof(struct hsdk_creg_gpio),
};
