// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinctrl driver for Nexell SoCs
 * (C) Copyright 2016 Nexell
 * Bongyu, KOO <freestyle@nexell.co.kr>
 *
 * (C) Copyright 2019 Stefan Bosch <stefan_b@posteo.net>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include "pinctrl-nexell.h"
#include "pinctrl-s5pxx18.h"

DECLARE_GLOBAL_DATA_PTR;

static void nx_gpio_set_bit(u32 *value, u32 bit, int enable)
{
	register u32 newvalue;

	newvalue = *value;
	newvalue &= ~(1ul << bit);
	newvalue |= (u32)enable << bit;
	writel(newvalue, value);
}

static void nx_gpio_set_bit2(u32 *value, u32 bit, u32 bit_value)
{
	register u32 newvalue = *value;

	newvalue = (u32)(newvalue & ~(3ul << (bit * 2)));
	newvalue = (u32)(newvalue | (bit_value << (bit * 2)));

	writel(newvalue, value);
}

static int nx_gpio_open_module(void *base)
{
	writel(0xFFFFFFFF, base + GPIOX_SLEW_DISABLE_DEFAULT);
	writel(0xFFFFFFFF, base + GPIOX_DRV1_DISABLE_DEFAULT);
	writel(0xFFFFFFFF, base + GPIOX_DRV0_DISABLE_DEFAULT);
	writel(0xFFFFFFFF, base + GPIOX_PULLSEL_DISABLE_DEFAULT);
	writel(0xFFFFFFFF, base + GPIOX_PULLENB_DISABLE_DEFAULT);
	return true;
}

static void nx_gpio_set_pad_function(void *base, u32 pin, u32 padfunc)
{
	u32 reg = (pin / 16) ? GPIOX_ALTFN1 : GPIOX_ALTFN0;

	nx_gpio_set_bit2(base + reg, pin % 16, padfunc);
}

static void nx_gpio_set_drive_strength(void *base, u32 pin, u32 drv)
{
	nx_gpio_set_bit(base + GPIOX_DRV1, pin, (int)(((u32)drv >> 0) & 0x1));
	nx_gpio_set_bit(base + GPIOX_DRV0, pin, (int)(((u32)drv >> 1) & 0x1));
}

static void nx_gpio_set_pull_mode(void *base, u32 pin, u32 mode)
{
	if (mode == nx_gpio_pull_off) {
		nx_gpio_set_bit(base + GPIOX_PULLENB, pin, false);
		nx_gpio_set_bit(base + GPIOX_PULLSEL, pin, false);
	} else {
		nx_gpio_set_bit(base + GPIOX_PULLSEL,
				pin, (mode & 1 ? true : false));
		nx_gpio_set_bit(base + GPIOX_PULLENB, pin, true);
	}
}

static void nx_alive_set_pullup(void *base, u32 pin, bool enable)
{
	u32 PULLUP_MASK;

	PULLUP_MASK = (1UL << pin);
	if (enable)
		writel(PULLUP_MASK, base + ALIVE_PADPULLUPSET);
	else
		writel(PULLUP_MASK, base + ALIVE_PADPULLUPRST);
}

static int s5pxx18_pinctrl_gpio_init(struct udevice *dev)
{
	struct nexell_pinctrl_priv *priv = dev_get_priv(dev);
	const struct nexell_pin_ctrl *ctrl = priv->pin_ctrl;
	unsigned long reg = priv->base;
	int i;

	for (i = 0; i < ctrl->nr_banks - 1; i++) /* except alive bank */
		nx_gpio_open_module((void *)(reg + ctrl->pin_banks[i].offset));

	return 0;
}

static int s5pxx18_pinctrl_alive_init(struct udevice *dev)
{
	struct nexell_pinctrl_priv *priv = dev_get_priv(dev);
	const struct nexell_pin_ctrl *ctrl = priv->pin_ctrl;
	unsigned long reg = priv->base;

	reg += ctrl->pin_banks[ctrl->nr_banks - 1].offset;

	writel(1, reg + ALIVE_PWRGATE);
	return 0;
}

int s5pxx18_pinctrl_init(struct udevice *dev)
{
	s5pxx18_pinctrl_gpio_init(dev);
	s5pxx18_pinctrl_alive_init(dev);

	return 0;
}

static int is_pin_alive(const char *name)
{
	return !strncmp(name, "alive", 5);
}

/**
 * s5pxx18_pinctrl_set_state: configure a pin state.
 * dev: the pinctrl device to be configured.
 * config: the state to be configured.
 */
static int s5pxx18_pinctrl_set_state(struct udevice *dev,
				     struct udevice *config)
{
	unsigned int count, idx, pin;
	unsigned int pinfunc, pinpud, pindrv;
	unsigned long reg;
	const char *name;
	int ret;

	/*
	 * refer to the following document for the pinctrl bindings
	 * doc/device-tree-bindings/pinctrl/nexell,s5pxx18-pinctrl.txt
	 */
	count = dev_read_string_count(config, "pins");

	if (count <= 0)
		return -EINVAL;

	pinfunc = dev_read_s32_default(config, "pin-function", -1);
	pinpud = dev_read_s32_default(config, "pin-pull", -1);
	pindrv = dev_read_s32_default(config, "pin-strength", -1);

	for (idx = 0; idx < count; idx++) {
		ret = dev_read_string_index(config, "pins", idx, &name);
		if (ret)
			return ret;
		if (!name)
			continue;
		reg = pin_to_bank_base(dev, name, &pin);

		if (is_pin_alive(name)) {
			/* pin pull up/down */
			if (pinpud != -1)
				nx_alive_set_pullup((void *)reg, pin,
						    pinpud & 1);
			continue;
		}

		/* pin function */
		if (pinfunc != -1)
			nx_gpio_set_pad_function((void *)reg, pin, pinfunc);

		/* pin pull up/down/off */
		if (pinpud != -1)
			nx_gpio_set_pull_mode((void *)reg, pin, pinpud);

		/* pin drive strength */
		if (pindrv != -1)
			nx_gpio_set_drive_strength((void *)reg, pin, pindrv);
	}

	return 0;
}

static struct pinctrl_ops s5pxx18_pinctrl_ops = {
	.set_state	= s5pxx18_pinctrl_set_state,
};

/* pin banks of s5pxx18 pin-controller */
static const struct nexell_pin_bank_data s5pxx18_pin_banks[] = {
	NEXELL_PIN_BANK(32, 0xA000, "gpioa"),
	NEXELL_PIN_BANK(32, 0xB000, "gpiob"),
	NEXELL_PIN_BANK(32, 0xC000, "gpioc"),
	NEXELL_PIN_BANK(32, 0xD000, "gpiod"),
	NEXELL_PIN_BANK(32, 0xE000, "gpioe"),
	NEXELL_PIN_BANK(6, 0x0800, "alive"),
};

const struct nexell_pin_ctrl s5pxx18_pin_ctrl[] = {
	{
		/* pin-controller data */
		.pin_banks	= s5pxx18_pin_banks,
		.nr_banks	= ARRAY_SIZE(s5pxx18_pin_banks),
	},
};

static const struct udevice_id s5pxx18_pinctrl_ids[] = {
	{ .compatible = "nexell,s5pxx18-pinctrl",
		.data = (ulong)s5pxx18_pin_ctrl },
	{ }
};

U_BOOT_DRIVER(pinctrl_s5pxx18) = {
	.name		= "pinctrl_s5pxx18",
	.id		= UCLASS_PINCTRL,
	.of_match	= s5pxx18_pinctrl_ids,
	.priv_auto	= sizeof(struct nexell_pinctrl_priv),
	.ops		= &s5pxx18_pinctrl_ops,
	.probe		= nexell_pinctrl_probe,
	.flags		= DM_FLAG_PRE_RELOC
};
