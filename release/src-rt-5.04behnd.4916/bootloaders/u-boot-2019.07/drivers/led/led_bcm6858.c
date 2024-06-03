// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * based on:
 * drivers/led/led_bcm6328.c
 * drivers/led/led_bcm6358.c
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

/* LED Controller Global settings register */
#define LED_CTRL_REG			0x00
#define LED_CTRL_MASK			0x1f
#define LED_CTRL_LED_TEST_MODE		BIT(0)
#define LED_CTRL_SERIAL_LED_DATA_PPOL	BIT(1)
#define LED_CTRL_SERIAL_LED_CLK_POL	BIT(2)
#define LED_CTRL_SERIAL_LED_EN_POL	BIT(3)
#define LED_CTRL_SERIAL_LED_MSB_FIRST	BIT(4)

/* LED Controller IP LED source select register */
#define LED_HW_LED_EN_REG		0x08
/* LED Flash control register0 */
#define LED_FLASH_RATE_CONTROL_REG0	0x10
/* Soft LED input register */
#define LED_SW_LED_IP_REG		0xb8
/* Soft LED input polarity register */
#define LED_SW_LED_IP_PPOL_REG		0xbc

struct bcm6858_led_priv {
	void __iomem *regs;
	u8 pin;
};

#ifdef CONFIG_LED_BLINK
/*
 * The value for flash rate are:
 * 0 : no blinking
 * 1 : rate is 25 Hz => 40 ms (period)
 * 2 : rate is 12.5 Hz => 80 ms (period)
 * 3 : rate is 6.25 Hz => 160 ms (period)
 * 4 : rate is 3.125 Hz => 320 ms (period)
 * 5 : rate is 1.5625 Hz => 640 ms (period)
 * 6 : rate is 0.7815 Hz => 1280 ms (period)
 * 7 : rate is 0.390625 Hz => 2560 ms (period)
 */
static const int bcm6858_flash_rate[8] = {
	0, 40, 80, 160, 320, 640, 1280, 2560
};

static u32 bcm6858_flash_rate_value(int period_ms)
{
	unsigned long value = 7;
	int i;

	for (i = 0; i < ARRAY_SIZE(bcm6858_flash_rate); i++) {
		if (period_ms <= bcm6858_flash_rate[i]) {
			value = i;
			break;
		}
	}

	return value;
}

static int bcm6858_led_set_period(struct udevice *dev, int period_ms)
{
	struct bcm6858_led_priv *priv = dev_get_priv(dev);
	u32 offset, shift, mask, value;

	offset = (priv->pin / 8) * 4;
	shift  = (priv->pin % 8) * 4;
	mask   = 0x7 << shift;
	value  = bcm6858_flash_rate_value(period_ms) << shift;

	clrbits_32(priv->regs + LED_FLASH_RATE_CONTROL_REG0 + offset, mask);
	setbits_32(priv->regs + LED_FLASH_RATE_CONTROL_REG0 + offset, value);

	return 0;
}
#endif

static enum led_state_t bcm6858_led_get_state(struct udevice *dev)
{
	struct bcm6858_led_priv *priv = dev_get_priv(dev);
	enum led_state_t state = LEDST_OFF;
	u32 sw_led_ip;

	sw_led_ip = readl(priv->regs + LED_SW_LED_IP_REG);
	if (sw_led_ip & (1 << priv->pin))
		state = LEDST_ON;

	return state;
}

static int bcm6858_led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct bcm6858_led_priv *priv = dev_get_priv(dev);

	switch (state) {
	case LEDST_OFF:
		clrbits_32(priv->regs + LED_SW_LED_IP_REG, (1 << priv->pin));
#ifdef CONFIG_LED_BLINK
		bcm6858_led_set_period(dev, 0);
#endif
		break;
	case LEDST_ON:
		setbits_32(priv->regs + LED_SW_LED_IP_REG, (1 << priv->pin));
#ifdef CONFIG_LED_BLINK
		bcm6858_led_set_period(dev, 0);
#endif
		break;
	case LEDST_TOGGLE:
		if (bcm6858_led_get_state(dev) == LEDST_OFF)
			return bcm6858_led_set_state(dev, LEDST_ON);
		else
			return bcm6858_led_set_state(dev, LEDST_OFF);
		break;
#ifdef CONFIG_LED_BLINK
	case LEDST_BLINK:
		setbits_32(priv->regs + LED_SW_LED_IP_REG, (1 << priv->pin));
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct led_ops bcm6858_led_ops = {
	.get_state = bcm6858_led_get_state,
	.set_state = bcm6858_led_set_state,
#ifdef CONFIG_LED_BLINK
	.set_period = bcm6858_led_set_period,
#endif
};

static int bcm6858_led_probe(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_platdata(dev);

	/* Top-level LED node */
	if (!uc_plat->label) {
		void __iomem *regs;
		u32 set_bits = 0;

		regs = dev_remap_addr(dev);
		if (!regs)
			return -EINVAL;

		if (dev_read_bool(dev, "brcm,serial-led-msb-first"))
			set_bits |= LED_CTRL_SERIAL_LED_MSB_FIRST;
		if (dev_read_bool(dev, "brcm,serial-led-en-pol"))
			set_bits |= LED_CTRL_SERIAL_LED_EN_POL;
		if (dev_read_bool(dev, "brcm,serial-led-clk-pol"))
			set_bits |= LED_CTRL_SERIAL_LED_CLK_POL;
		if (dev_read_bool(dev, "brcm,serial-led-data-ppol"))
			set_bits |= LED_CTRL_SERIAL_LED_DATA_PPOL;
		if (dev_read_bool(dev, "brcm,led-test-mode"))
			set_bits |= LED_CTRL_LED_TEST_MODE;

		clrsetbits_32(regs + LED_CTRL_REG, ~0, set_bits);
	} else {
		struct bcm6858_led_priv *priv = dev_get_priv(dev);
		void __iomem *regs;
		unsigned int pin;

		regs = dev_remap_addr(dev_get_parent(dev));
		if (!regs)
			return -EINVAL;

		pin = dev_read_u32_default(dev, "reg", LEDS_MAX);
		if (pin >= LEDS_MAX)
			return -EINVAL;

		priv->regs = regs;
		priv->pin = pin;

		/* this led is managed by software */
		clrbits_32(regs + LED_HW_LED_EN_REG, 1 << pin);

		/* configure the polarity */
		if (dev_read_bool(dev, "active-low"))
			clrbits_32(regs + LED_SW_LED_IP_PPOL_REG, 1 << pin);
		else
			setbits_32(regs + LED_SW_LED_IP_PPOL_REG, 1 << pin);
	}

	return 0;
}

static int bcm6858_led_bind(struct udevice *parent)
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

		ret = device_bind_driver_to_node(parent, "bcm6858-led",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;

		uc_plat = dev_get_uclass_platdata(dev);
		uc_plat->label = label;
	}

	return 0;
}

static const struct udevice_id bcm6858_led_ids[] = {
	{ .compatible = "brcm,bcm6858-leds" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6858_led) = {
	.name = "bcm6858-led",
	.id = UCLASS_LED,
	.of_match = bcm6858_led_ids,
	.bind = bcm6858_led_bind,
	.probe = bcm6858_led_probe,
	.priv_auto_alloc_size = sizeof(struct bcm6858_led_priv),
	.ops = &bcm6858_led_ops,
};
