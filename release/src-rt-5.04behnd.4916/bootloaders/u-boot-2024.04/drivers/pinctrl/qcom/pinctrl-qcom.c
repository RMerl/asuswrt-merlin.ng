// SPDX-License-Identifier: GPL-2.0+
/*
 * TLMM driver for Qualcomm APQ8016, APQ8096
 *
 * (C) Copyright 2018 Ramon Fried <ramon.fried@gmail.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <mach/gpio.h>

#include "pinctrl-qcom.h"

struct msm_pinctrl_priv {
	phys_addr_t base;
	struct msm_pinctrl_data *data;
};

#define GPIO_CONFIG_REG(priv, x) \
	(qcom_pin_offset((priv)->data->pin_data.pin_offsets, x))

#define TLMM_GPIO_PULL_MASK GENMASK(1, 0)
#define TLMM_FUNC_SEL_MASK GENMASK(5, 2)
#define TLMM_DRV_STRENGTH_MASK GENMASK(8, 6)
#define TLMM_GPIO_DISABLE BIT(9)

static const struct pinconf_param msm_conf_params[] = {
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 2 },
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 3 },
};

static int msm_get_functions_count(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->functions_count;
}

static int msm_get_pins_count(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->pin_data.pin_count;
}

static const char *msm_get_function_name(struct udevice *dev,
					 unsigned int selector)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->get_function_name(dev, selector);
}

static int msm_pinctrl_probe(struct udevice *dev)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	priv->data = (struct msm_pinctrl_data *)dev_get_driver_data(dev);

	return priv->base == FDT_ADDR_T_NONE ? -EINVAL : 0;
}

static const char *msm_get_pin_name(struct udevice *dev, unsigned int selector)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->data->get_pin_name(dev, selector);
}

static int msm_pinmux_set(struct udevice *dev, unsigned int pin_selector,
			  unsigned int func_selector)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	clrsetbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
			TLMM_FUNC_SEL_MASK | TLMM_GPIO_DISABLE,
			priv->data->get_function_mux(func_selector) << 2);
	return 0;
}

static int msm_pinconf_set(struct udevice *dev, unsigned int pin_selector,
			   unsigned int param, unsigned int argument)
{
	struct msm_pinctrl_priv *priv = dev_get_priv(dev);

	switch (param) {
	case PIN_CONFIG_DRIVE_STRENGTH:
		argument = (argument / 2) - 1;
		clrsetbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
				TLMM_DRV_STRENGTH_MASK, argument << 6);
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		clrbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
			     TLMM_GPIO_PULL_MASK);
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		clrsetbits_le32(priv->base + GPIO_CONFIG_REG(priv, pin_selector),
				TLMM_GPIO_PULL_MASK, argument);
		break;
	default:
		return 0;
	}

	return 0;
}

struct pinctrl_ops msm_pinctrl_ops = {
	.get_pins_count = msm_get_pins_count,
	.get_pin_name = msm_get_pin_name,
	.set_state = pinctrl_generic_set_state,
	.pinmux_set = msm_pinmux_set,
	.pinconf_num_params = ARRAY_SIZE(msm_conf_params),
	.pinconf_params = msm_conf_params,
	.pinconf_set = msm_pinconf_set,
	.get_functions_count = msm_get_functions_count,
	.get_function_name = msm_get_function_name,
};

int msm_pinctrl_bind(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);
	struct msm_pinctrl_data *data = (struct msm_pinctrl_data *)dev_get_driver_data(dev);
	struct driver *drv;
	struct udevice *pinctrl_dev;
	const char *name;
	int ret;

	drv = lists_driver_lookup_name("pinctrl_qcom");
	if (!drv)
		return -ENOENT;

	ret = device_bind_with_driver_data(dev_get_parent(dev), drv, ofnode_get_name(node), (ulong)data,
					   dev_ofnode(dev), &pinctrl_dev);
	if (ret)
		return ret;

	ofnode_get_property(node, "gpio-controller", &ret);
	if (ret < 0)
		return 0;

	/* Get the name of gpio node */
	name = ofnode_get_name(node);
	if (!name)
		return -EINVAL;

	drv = lists_driver_lookup_name("gpio_msm");
	if (!drv) {
		printf("Can't find gpio_msm driver\n");
		return -ENODEV;
	}

	/* Bind gpio device as a child of the pinctrl device */
	ret = device_bind_with_driver_data(pinctrl_dev, drv,
					   name, (ulong)&data->pin_data, node, NULL);
	if (ret) {
		device_unbind(pinctrl_dev);
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(pinctrl_qcom) = {
	.name		= "pinctrl_qcom",
	.id		= UCLASS_PINCTRL,
	.priv_auto	= sizeof(struct msm_pinctrl_priv),
	.ops		= &msm_pinctrl_ops,
	.probe		= msm_pinctrl_probe,
};
