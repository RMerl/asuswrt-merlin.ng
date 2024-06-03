// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Alexander Graf <agraf@suse.de>
 *
 * Based on drivers/pinctrl/mvebu/pinctrl-mvebu.c and
 *          drivers/gpio/bcm2835_gpio.c
 *
 * This driver gets instantiated by the GPIO driver, because both devices
 * share the same device node.
 * https://spdx.org/licenses
 */

#include <common.h>
#include <config.h>
#include <errno.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/gpio.h>

struct bcm283x_pinctrl_priv {
	u32 *base_reg;
};

#define MAX_PINS_PER_BANK 16

static void bcm2835_gpio_set_func_id(struct udevice *dev, unsigned int gpio,
				     int func)
{
	struct bcm283x_pinctrl_priv *priv = dev_get_priv(dev);
	int reg_offset;
	int field_offset;

	reg_offset = BCM2835_GPIO_FSEL_BANK(gpio);
	field_offset = BCM2835_GPIO_FSEL_SHIFT(gpio);

	clrsetbits_le32(&priv->base_reg[reg_offset],
			BCM2835_GPIO_FSEL_MASK << field_offset,
			(func & BCM2835_GPIO_FSEL_MASK) << field_offset);
}

static int bcm2835_gpio_get_func_id(struct udevice *dev, unsigned int gpio)
{
	struct bcm283x_pinctrl_priv *priv = dev_get_priv(dev);
	u32 val;

	val = readl(&priv->base_reg[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return (val >> BCM2835_GPIO_FSEL_SHIFT(gpio) & BCM2835_GPIO_FSEL_MASK);
}

/*
 * bcm283x_pinctrl_set_state: configure pin functions.
 * @dev: the pinctrl device to be configured.
 * @config: the state to be configured.
 * @return: 0 in success
 */
int bcm283x_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	u32 pin_arr[MAX_PINS_PER_BANK];
	u32 function;
	int i, len, pin_count = 0;

	if (!dev_read_prop(config, "brcm,pins", &len) || !len ||
	    len & 0x3 || dev_read_u32_array(config, "brcm,pins", pin_arr,
						  len / sizeof(u32))) {
		debug("Failed reading pins array for pinconfig %s (%d)\n",
		      config->name, len);
		return -EINVAL;
	}

	pin_count = len / sizeof(u32);

	function = dev_read_u32_default(config, "brcm,function", -1);
	if (function < 0) {
		debug("Failed reading function for pinconfig %s (%d)\n",
		      config->name, function);
		return -EINVAL;
	}

	for (i = 0; i < pin_count; i++)
		bcm2835_gpio_set_func_id(dev, pin_arr[i], function);

	return 0;
}

static int bcm283x_pinctrl_get_gpio_mux(struct udevice *dev, int banknum,
					int index)
{
	if (banknum != 0)
		return -EINVAL;

	return bcm2835_gpio_get_func_id(dev, index);
}

static const struct udevice_id bcm2835_pinctrl_id[] = {
	{.compatible = "brcm,bcm2835-gpio"},
	{}
};

int bcm283x_pinctl_probe(struct udevice *dev)
{
	struct bcm283x_pinctrl_priv *priv;
	int ret;
	struct udevice *pdev;

	priv = dev_get_priv(dev);
	if (!priv) {
		debug("%s: Failed to get private\n", __func__);
		return -EINVAL;
	}

	priv->base_reg = dev_read_addr_ptr(dev);
	if (priv->base_reg == (void *)FDT_ADDR_T_NONE) {
		debug("%s: Failed to get base address\n", __func__);
		return -EINVAL;
	}

	/* Create GPIO device as well */
	ret = device_bind(dev, lists_driver_lookup_name("gpio_bcm2835"),
			  "gpio_bcm2835", NULL, dev_of_offset(dev), &pdev);
	if (ret) {
		/*
		 * While we really want the pinctrl driver to work to make
		 * devices go where they should go, the GPIO controller is
		 * not quite as crucial as it's only rarely used, so don't
		 * fail here.
		 */
		printf("Failed to bind GPIO driver\n");
	}

	return 0;
}

static struct pinctrl_ops bcm283x_pinctrl_ops = {
	.set_state	= bcm283x_pinctrl_set_state,
	.get_gpio_mux	= bcm283x_pinctrl_get_gpio_mux,
};

U_BOOT_DRIVER(pinctrl_bcm283x) = {
	.name		= "bcm283x_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= of_match_ptr(bcm2835_pinctrl_id),
	.priv_auto_alloc_size = sizeof(struct bcm283x_pinctrl_priv),
	.ops		= &bcm283x_pinctrl_ops,
	.probe		= bcm283x_pinctl_probe,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags		= DM_FLAG_PRE_RELOC,
#endif
};
