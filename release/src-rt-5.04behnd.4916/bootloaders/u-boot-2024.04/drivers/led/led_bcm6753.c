// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * based on:
 * drivers/led/led_bcm6858.c
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <log.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <linux/bitops.h>

#define LEDS_MAX		32
#define LEDS_WAIT		100

/* LED Mode register */
#define LED_MODE_REG		0x0
#define LED_MODE_OFF		0
#define LED_MODE_ON		1
#define LED_MODE_MASK		1

/* LED Controller Global settings register */
#define CLED_CTRL_REG			0x00
#define CLED_CTRL_SERIAL_LED_DATA_PPOL	BIT(1)
#define CLED_CTRL_SERIAL_LED_CLK_POL	BIT(2)
#define CLED_CTRL_SERIAL_LED_EN_POL	BIT(3)
#define CLED_CTRL_SERIAL_LED_MSB_FIRST	BIT(4)
#define CLED_CTRL_MASK			0x1E
/* LED Controller IP LED source select register */
#define CLED_HW_LED_EN_REG		0x04
/* Hardware LED Polarity register */
#define CLED_HW_LED_IP_PPOL_REG		0x0c
/* Soft LED Set Register */
#define CLED_SW_LED_IP_SET_REG		0x10
/* Parallel LED Output Polarity Register */
#define CLED_PLED_OP_PPOL_REG		0x18
/* LED Channel activate register */
#define CLED_LED_CH_ACTIVATE_REG	0x1c
/* LED 0 Config 0 reg */
#define CLED_LED_0_CONFIG_0		0x20
/* Soft LED Clear Register */
#define CLED_SW_LED_IP_CLEAR_REG	0x444
/* Soft LED Status Register */
#define CLED_SW_LED_IP_STATUS_REG	0x448

/* Size of all registers used for the config of one LED */
#define CLED_CONFIG_SIZE		(4 * sizeof(u32))

#define CLED_CONFIG0_MODE		0
#define CLED_CONFIG0_MODE_MASK		(BIT(0) | BIT(1))
#define CLED_CONFIG0_MODE_STEADY	0
#define CLED_CONFIG0_MODE_FADING	1
#define CLED_CONFIG0_MODE_PULSATING	2

#define CLED_CONFIG0_FLASH_CTRL_SHIFT	3
#define CLED_CONFIG0_FLASH_CTRL_MASK	(BIT(3) | BIT(4) | BIT(5))

struct bcm6753_led_priv {
	void __iomem *regs;
	u8 pin;
};

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
static const int bcm6753_flash_rate[8] = {
	0, 40, 80, 160, 320, 640, 1280, 2560
};

static u32 bcm6753_flash_rate_value(int period_ms)
{
	unsigned long value = 7;
	int i;

	for (i = 0; i < ARRAY_SIZE(bcm6753_flash_rate); i++) {
		if (period_ms <= bcm6753_flash_rate[i]) {
			value = i;
			break;
		}
	}

	return value;
}

static int bcm6753_led_set_period(struct udevice *dev, int period_ms)
{
	struct bcm6753_led_priv *priv = dev_get_priv(dev);
	u32 offset, shift, value;

	offset = CLED_LED_0_CONFIG_0 + (CLED_CONFIG_SIZE * priv->pin);
	value  = bcm6753_flash_rate_value(period_ms);
	shift  = CLED_CONFIG0_FLASH_CTRL_SHIFT;

	/* set mode steady */
	clrbits_32(priv->regs + offset, CLED_CONFIG0_MODE_MASK);
	setbits_32(priv->regs + offset, CLED_CONFIG0_MODE_STEADY);

	/* set flash rate */
	clrbits_32(priv->regs + offset, CLED_CONFIG0_FLASH_CTRL_MASK);
	setbits_32(priv->regs + offset, value << shift);

	/* enable config */
	setbits_32(priv->regs + CLED_LED_CH_ACTIVATE_REG, 1 << priv->pin);

	return 0;
}

static enum led_state_t bcm6753_led_get_state(struct udevice *dev)
{
	struct bcm6753_led_priv *priv = dev_get_priv(dev);
	enum led_state_t state = LEDST_OFF;
	u32 sw_led_ip_status;

	sw_led_ip_status = readl(priv->regs + CLED_SW_LED_IP_STATUS_REG);
	if (sw_led_ip_status & (1 << priv->pin))
		state = LEDST_ON;

	return state;
}

static int bcm6753_led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct bcm6753_led_priv *priv = dev_get_priv(dev);

	switch (state) {
	case LEDST_OFF:
		setbits_32(priv->regs + CLED_SW_LED_IP_CLEAR_REG, (1 << priv->pin));
		if (IS_ENABLED(CONFIG_LED_BLINK))
			bcm6753_led_set_period(dev, 0);
		break;
	case LEDST_ON:
		setbits_32(priv->regs + CLED_SW_LED_IP_SET_REG, (1 << priv->pin));
		if (IS_ENABLED(CONFIG_LED_BLINK))
			bcm6753_led_set_period(dev, 0);
		break;
	case LEDST_TOGGLE:
		if (bcm6753_led_get_state(dev) == LEDST_OFF)
			return bcm6753_led_set_state(dev, LEDST_ON);
		else
			return bcm6753_led_set_state(dev, LEDST_OFF);
		break;
#ifdef CONFIG_LED_BLINK
	case LEDST_BLINK:
		setbits_32(priv->regs + CLED_SW_LED_IP_SET_REG, (1 << priv->pin));
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct led_ops bcm6753_led_ops = {
	.get_state = bcm6753_led_get_state,
	.set_state = bcm6753_led_set_state,
#ifdef CONFIG_LED_BLINK
	.set_period = bcm6753_led_set_period,
#endif
};

static int bcm6753_led_probe(struct udevice *dev)
{
	struct bcm6753_led_priv *priv = dev_get_priv(dev);
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
	clrbits_32(regs + CLED_HW_LED_EN_REG, 1 << pin);

	/* configure the polarity */
	if (dev_read_bool(dev, "active-low"))
		clrbits_32(regs + CLED_PLED_OP_PPOL_REG, 1 << pin);
	else
		setbits_32(regs + CLED_PLED_OP_PPOL_REG, 1 << pin);

	return 0;
}

U_BOOT_DRIVER(bcm6753_led) = {
	.name = "bcm6753-led",
	.id = UCLASS_LED,
	.probe = bcm6753_led_probe,
	.priv_auto = sizeof(struct bcm6753_led_priv),
	.ops = &bcm6753_led_ops,
};

static int bcm6753_led_wrap_probe(struct udevice *dev)
{
	void __iomem *regs;
	u32 set_bits = 0;

	regs = dev_remap_addr(dev);
	if (!regs)
		return -EINVAL;

	if (dev_read_bool(dev, "brcm,serial-led-msb-first"))
		set_bits |= CLED_CTRL_SERIAL_LED_MSB_FIRST;
	if (dev_read_bool(dev, "brcm,serial-led-en-pol"))
		set_bits |= CLED_CTRL_SERIAL_LED_EN_POL;
	if (dev_read_bool(dev, "brcm,serial-led-clk-pol"))
		set_bits |= CLED_CTRL_SERIAL_LED_CLK_POL;
	if (dev_read_bool(dev, "brcm,serial-led-data-ppol"))
		set_bits |= CLED_CTRL_SERIAL_LED_DATA_PPOL;

	clrsetbits_32(regs + CLED_CTRL_REG, CLED_CTRL_MASK, set_bits);

	return 0;
}

static int bcm6753_led_wrap_bind(struct udevice *parent)
{
	ofnode node;

	dev_for_each_subnode(node, parent) {
		struct udevice *dev;
		int ret;

		ret = device_bind_driver_to_node(parent, "bcm6753-led",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id bcm6753_led_ids[] = {
	{ .compatible = "brcm,bcm6753-leds" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6753_led_wrap) = {
	.name	= "bcm6753_led_wrap",
	.id	= UCLASS_NOP,
	.of_match = bcm6753_led_ids,
	.probe = bcm6753_led_wrap_probe,
	.bind = bcm6753_led_wrap_bind,
};
