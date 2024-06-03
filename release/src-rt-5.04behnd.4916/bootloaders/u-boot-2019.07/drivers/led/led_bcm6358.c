// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <asm/io.h>
#include <dm/lists.h>

#define LEDS_MAX		32
#define LEDS_WAIT		100

/* LED Mode register */
#define LED_MODE_REG		0x0
#define LED_MODE_OFF		0
#define LED_MODE_ON		1
#define LED_MODE_MASK		1

/* LED Control register */
#define LED_CTRL_REG		0x4
#define LED_CTRL_CLK_MASK	0x3
#define LED_CTRL_CLK_1		0
#define LED_CTRL_CLK_2		1
#define LED_CTRL_CLK_4		2
#define LED_CTRL_CLK_8		3
#define LED_CTRL_POL_SHIFT	2
#define LED_CTRL_POL_MASK	(1 << LED_CTRL_POL_SHIFT)
#define LED_CTRL_BUSY_SHIFT	3
#define LED_CTRL_BUSY_MASK	(1 << LED_CTRL_BUSY_SHIFT)

struct bcm6358_led_priv {
	void __iomem *regs;
	uint8_t pin;
	bool active_low;
};

static void bcm6358_led_busy(void __iomem *regs)
{
	while (readl_be(regs + LED_CTRL_REG) & LED_CTRL_BUSY_MASK)
		udelay(LEDS_WAIT);
}

static unsigned long bcm6358_led_get_mode(struct bcm6358_led_priv *priv)
{
	bcm6358_led_busy(priv->regs);

	return (readl_be(priv->regs + LED_MODE_REG) >> priv->pin) &
	       LED_MODE_MASK;
}

static int bcm6358_led_set_mode(struct bcm6358_led_priv *priv, uint8_t mode)
{
	bcm6358_led_busy(priv->regs);

	clrsetbits_be32(priv->regs + LED_MODE_REG,
			(LED_MODE_MASK << priv->pin),
			(mode << priv->pin));

	return 0;
}

static enum led_state_t bcm6358_led_get_state(struct udevice *dev)
{
	struct bcm6358_led_priv *priv = dev_get_priv(dev);
	enum led_state_t state = LEDST_OFF;

	switch (bcm6358_led_get_mode(priv)) {
	case LED_MODE_OFF:
		state = (priv->active_low ? LEDST_ON : LEDST_OFF);
		break;
	case LED_MODE_ON:
		state = (priv->active_low ? LEDST_OFF : LEDST_ON);
		break;
	}

	return state;
}

static int bcm6358_led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct bcm6358_led_priv *priv = dev_get_priv(dev);
	unsigned long mode;

	switch (state) {
	case LEDST_OFF:
		mode = (priv->active_low ? LED_MODE_ON : LED_MODE_OFF);
		break;
	case LEDST_ON:
		mode = (priv->active_low ? LED_MODE_OFF : LED_MODE_ON);
		break;
	case LEDST_TOGGLE:
		if (bcm6358_led_get_state(dev) == LEDST_OFF)
			return bcm6358_led_set_state(dev, LEDST_ON);
		else
			return bcm6358_led_set_state(dev, LEDST_OFF);
		break;
	default:
		return -ENOSYS;
	}

	return bcm6358_led_set_mode(priv, mode);
}

static const struct led_ops bcm6358_led_ops = {
	.get_state = bcm6358_led_get_state,
	.set_state = bcm6358_led_set_state,
};

static int bcm6358_led_probe(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_platdata(dev);

	/* Top-level LED node */
	if (!uc_plat->label) {
		void __iomem *regs;
		unsigned int clk_div;
		u32 set_bits = 0;

		regs = dev_remap_addr(dev);
		if (!regs)
			return -EINVAL;

		if (dev_read_bool(dev, "brcm,clk-dat-low"))
			set_bits |= LED_CTRL_POL_MASK;
		clk_div = dev_read_u32_default(dev, "brcm,clk-div",
					       LED_CTRL_CLK_1);
		switch (clk_div) {
		case 8:
			set_bits |= LED_CTRL_CLK_8;
			break;
		case 4:
			set_bits |= LED_CTRL_CLK_4;
			break;
		case 2:
			set_bits |= LED_CTRL_CLK_2;
			break;
		default:
			set_bits |= LED_CTRL_CLK_1;
			break;
		}

		bcm6358_led_busy(regs);
		clrsetbits_be32(regs + LED_CTRL_REG,
				LED_CTRL_POL_MASK | LED_CTRL_CLK_MASK,
				set_bits);
	} else {
		struct bcm6358_led_priv *priv = dev_get_priv(dev);
		unsigned int pin;

		priv->regs = dev_remap_addr(dev);
		if (!priv->regs)
			return -EINVAL;

		pin = dev_read_u32_default(dev, "reg", LEDS_MAX);
		if (pin >= LEDS_MAX)
			return -EINVAL;

		priv->pin = pin;

		if (dev_read_bool(dev, "active-low"))
			priv->active_low = true;
	}

	return 0;
}

static int bcm6358_led_bind(struct udevice *parent)
{
	ofnode node;

	dev_for_each_subnode(node, parent) {
		struct led_uc_plat *uc_plat;
		struct udevice *dev;
		const char *label;
		int ret;

		label = ofnode_read_string(node, "label");
		if (!label) {
			debug("%s: node %s has no label\n", __func__,
			      ofnode_get_name(node));
			return -EINVAL;
		}

		ret = device_bind_driver_to_node(parent, "bcm6358-led",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;

		uc_plat = dev_get_uclass_platdata(dev);
		uc_plat->label = label;
	}

	return 0;
}

static const struct udevice_id bcm6358_led_ids[] = {
	{ .compatible = "brcm,bcm6358-leds" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6358_led) = {
	.name = "bcm6358-led",
	.id = UCLASS_LED,
	.of_match = bcm6358_led_ids,
	.bind = bcm6358_led_bind,
	.probe = bcm6358_led_probe,
	.priv_auto_alloc_size = sizeof(struct bcm6358_led_priv),
	.ops = &bcm6358_led_ops,
};
