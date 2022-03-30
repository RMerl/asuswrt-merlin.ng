// SPDX-License-Identifier: GPL-2.0

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <dm/pinctrl.h>
#include "pinctrl.h"

#define MAX_PIN_COUNT   50

struct bcmbca_pinctrl_priv {
	volatile struct pinctrl_reg *regp;
    unsigned int gpio_mux;
};

int bcmbca_pinctrl_probe(struct udevice *dev)
{
	struct bcmbca_pinctrl_priv *priv = dev_get_priv(dev);
    int ret;

	priv->regp = (struct pinctrl_reg *)dev_remap_addr(dev);
    ret = ofnode_read_u32(dev->node, "gpio-mux", &priv->gpio_mux);

	return 0;
}

/*
 * bcmbca_pinctrl_set_state_simple: pinctrl for a peripheral device.
 * @dev: the pinctrl device
 * @periph: the device to be configured
 * @return: 0 in success
 */
static int bcmbca_pinctrl_set_state_simple(struct udevice *dev, struct udevice *periph)
{
	struct bcmbca_pinctrl_priv *priv = dev_get_priv(dev);

	int size, len, i, ret;
	uint32_t pin_arr[MAX_PIN_COUNT];
	uint32_t phandle, function;
	const fdt32_t *list;
	ofnode arg;

	/* check if the device specify what pins to configure */
	list = dev_read_prop(periph, "pinctrl-0", &size);

	if (list)
	{
		size /= sizeof (*list);

		while(size--)
		{
			phandle = fdt32_to_cpu(*list++);

			arg = ofnode_get_by_phandle(phandle);

			if (ofnode_valid(arg))
			{
				ofnode_get_property(arg, "pins", &len);
				len /= sizeof(u32);
				ret = ofnode_read_u32_array(arg, "pins", pin_arr, len);
				ret |= ofnode_read_u32(arg, "function", &function);
				if (!ret)
				{
					for (i=0; i < len; i++)
					{
						bcmbca_pinmux_set(priv->regp, pin_arr[i], function);
					}
				}
			}
		}
	}
	else
	{
		if(dev_read_prop(periph, "pins", &len))
		{
			len /= sizeof(u32);
			ret = dev_read_u32_array(periph, "pins", pin_arr, len);
			ret |= dev_read_u32(periph, "function", &function);
			if (!ret)
			{
				for (i=0; i < len; i++)
				{
					bcmbca_pinmux_set(priv->regp, pin_arr[i], function);
				}
			}
		}
	}
	
	return 0;

}

/*
 * bcmbca_gpio_request: set pinctrl for a pin to gpio function.
 * @dev: the pinctrl device
 * @periph: the device to be configured
 * @return: 0 in success
 */
static int bcmbca_gpio_request(struct udevice *dev, unsigned int selector)
{
	struct bcmbca_pinctrl_priv *priv = dev_get_priv(dev);
    return bcmbca_pinmux_set(priv->regp, selector, priv->gpio_mux);
}

const struct pinctrl_ops bcmbca_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.set_state_simple = bcmbca_pinctrl_set_state_simple,
    .gpio_request_enable = bcmbca_gpio_request,
};

static const struct udevice_id bcmbca_pinctrl_match[] = {
	{
		.compatible = "brcm,bcmbca-pinctrl",
	},
};

U_BOOT_DRIVER(bcmbca_pinctrl) = {
	.name = "bcmbca_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = bcmbca_pinctrl_match,
	.ops = &bcmbca_pinctrl_ops,
	.priv_auto_alloc_size = sizeof(struct bcmbca_pinctrl_priv),
	.probe = bcmbca_pinctrl_probe,
};

