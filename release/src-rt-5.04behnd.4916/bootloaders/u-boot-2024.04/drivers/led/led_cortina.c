// SPDX-License-Identifier: GPL-2.0+

/*
 * Copyright (C) 2020 Cortina-Access
 * Author: Jway Lin <jway.lin@cortina-access.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <log.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <linux/bitops.h>

#define LED_MAX_HW_BLINK		127
#define LED_MAX_COUNT			16

/* LED_CONTROL fields */
#define LED_BLINK_RATE1_SHIFT	0
#define LED_BLINK_RATE1_MASK	0xff
#define LED_BLINK_RATE2_SHIFT	8
#define LED_BLINK_RATE2_MASK	0xff
#define LED_CLK_TEST			BIT(16)
#define LED_CLK_POLARITY		BIT(17)
#define LED_CLK_TEST_MODE		BIT(16)
#define LED_CLK_TEST_RX_TEST	BIT(30)
#define LED_CLK_TEST_TX_TEST	BIT(31)

/* LED_CONFIG fields */
#define LED_EVENT_ON_SHIFT		0
#define LED_EVENT_ON_MASK		0x7
#define LED_EVENT_BLINK_SHIFT	3
#define LED_EVENT_BLINK_MASK	0x7
#define LED_EVENT_OFF_SHIFT	6
#define LED_EVENT_OFF_MASK		0x7
#define LED_OFF_ON_SHIFT		9
#define LED_OFF_ON_MASK			0x3
#define LED_PORT_SHIFT			11
#define LED_PORT_MASK			0x7
#define LED_OFF_VAL				BIT(14)
#define LED_SW_EVENT			BIT(15)
#define LED_BLINK_SEL			BIT(16)

/* LED_CONFIG structures */
struct cortina_led_cfg {
	void __iomem *regs;
	u32 pin;			/* LED pin nubmer */
	bool active_low;	/*Active-High or Active-Low*/
	u32 off_event;		/* set led off event (RX,TX,SW)*/
	u32 blink_event;	/* set led blink event (RX,TX,SW)*/
	u32 on_event;	/* set led on event (RX,TX,SW)*/
	u32 port;		/* corresponding ethernet port */
	int blink_sel;		/* select blink-rate1 or blink-rate2  */
};

/* LED_control structures */
struct cortina_led_plat {
	void __iomem *ctrl_regs;
	u16 rate1;	/* blink rate setting 0 */
	u16 rate2;	/* blink rate setting 1 */
};

enum ca_led_state_t {
	CA_EVENT_MODE = 0,
	CA_LED_ON = 1,
	CA_LED_OFF,
};

static void cortina_led_write(void __iomem *reg, unsigned long data)
{
	writel(data, reg);
}

static unsigned long cortina_led_read(void __iomem *reg)
{
	return readl(reg);
}

static enum led_state_t cortina_led_get_state(struct udevice *dev)
{
	struct cortina_led_cfg *priv = dev_get_priv(dev);
	enum led_state_t state = LEDST_OFF;
	u32 val;

	val = readl(priv->regs);

	if (val & LED_SW_EVENT)
		state = LEDST_ON;

	return state;
}

static int cortina_led_set_state(struct udevice *dev, enum led_state_t state)
{
	u32 val;
	struct cortina_led_cfg *priv = dev_get_priv(dev);

	val = readl(priv->regs);
	val &= ~(LED_OFF_ON_MASK << LED_OFF_ON_SHIFT);

	switch (state) {
	case LEDST_OFF:
		val &= ~LED_SW_EVENT;
		val |= CA_LED_OFF << LED_OFF_ON_SHIFT;
		cortina_led_write(priv->regs, val);
		break;
	case LEDST_ON:
		val |= LED_SW_EVENT;
		val |= CA_LED_ON << LED_OFF_ON_SHIFT;
		cortina_led_write(priv->regs, val);
		break;
	case LEDST_TOGGLE:
		if (cortina_led_get_state(dev) == LEDST_OFF)
			return cortina_led_set_state(dev, LEDST_ON);
		else
			return cortina_led_set_state(dev, LEDST_OFF);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct led_ops cortina_led_ops = {
	.get_state = cortina_led_get_state,
	.set_state = cortina_led_set_state,
};

static int ca_led_of_to_plat(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	/* Top-level LED node */
	if (!uc_plat->label) {
		struct cortina_led_plat *plt = dev_get_plat(dev);

		plt->rate1 =
			dev_read_u32_default(dev, "Cortina,blink-rate1", 256);
		plt->rate2 =
			dev_read_u32_default(dev, "Cortina,blink-rate2", 512);
		plt->ctrl_regs = dev_remap_addr(dev);
	} else {
		struct cortina_led_cfg *priv = dev_get_priv(dev);

		priv->regs = dev_remap_addr(dev_get_parent(dev));
		priv->pin = dev_read_u32_default(dev, "pin", LED_MAX_COUNT);
		priv->blink_sel = dev_read_u32_default(dev, "blink-sel", 0);
		priv->off_event = dev_read_u32_default(dev, "off-event", 0);
		priv->blink_event = dev_read_u32_default(dev, "blink-event", 0);
		priv->on_event = dev_read_u32_default(dev, "on-event", 0);
		priv->port = dev_read_u32_default(dev, "port", 0);

		if (dev_read_bool(dev, "active-low"))
			priv->active_low = true;
		else
			priv->active_low = false;
	}

	return 0;
}

static int cortina_led_probe(struct udevice *dev)
{
	struct led_uc_plat *uc_plat = dev_get_uclass_plat(dev);

	/* Top-level LED node */
	if (!uc_plat->label) {
		struct cortina_led_plat *plat = dev_get_plat(dev);
		u32 reg_value, val;
		u16 rate1, rate2;

		if (!plat->ctrl_regs)
			return -EINVAL;

		reg_value = 0;
		reg_value |= LED_CLK_POLARITY;

		rate1 = plat->rate1;
		rate2 = plat->rate2;

		val = rate1 / 16 - 1;
		rate1 = val > LED_MAX_HW_BLINK ?
					LED_MAX_HW_BLINK : val;
		reg_value |= (rate1 & LED_BLINK_RATE1_MASK) <<
					LED_BLINK_RATE1_SHIFT;

		val = rate2 / 16 - 1;
		rate2 = val > LED_MAX_HW_BLINK ?
					LED_MAX_HW_BLINK : val;
		reg_value |= (rate2 & LED_BLINK_RATE2_MASK) <<
					LED_BLINK_RATE2_SHIFT;

		cortina_led_write(plat->ctrl_regs, reg_value);

	} else {
		struct cortina_led_cfg *priv = dev_get_priv(dev);
		void __iomem *regs;
		u32 val, port, off_event, blink_event, on_event;

		regs = priv->regs;
		if (!regs)
			return -EINVAL;

		if (priv->pin >= LED_MAX_COUNT)
			return -EINVAL;

		priv->regs = regs + 4 + priv->pin * 4;

		val = cortina_led_read(priv->regs);

		if (priv->active_low)
			val |= LED_OFF_VAL;
		else
			val &= ~LED_OFF_VAL;

		if (priv->blink_sel == 0)
			val &= ~LED_BLINK_SEL;
		else if (priv->blink_sel == 1)
			val |= LED_BLINK_SEL;

		off_event = priv->off_event;
		val &= ~(LED_EVENT_OFF_MASK << LED_EVENT_OFF_SHIFT);
		if (off_event != 0)
			val |= BIT(off_event) << LED_EVENT_OFF_SHIFT;

		blink_event =  priv->blink_event;
		val &= ~(LED_EVENT_BLINK_MASK << LED_EVENT_BLINK_SHIFT);
		if (blink_event != 0)
			val |= BIT(blink_event) << LED_EVENT_BLINK_SHIFT;

		on_event = priv->on_event;
		val &= ~(LED_EVENT_ON_MASK << LED_EVENT_ON_SHIFT);
		if (on_event != 0)
			val |= BIT(on_event) << LED_EVENT_ON_SHIFT;

		port = priv->port;
		val &= ~(LED_PORT_MASK << LED_PORT_SHIFT);
		val |= port << LED_PORT_SHIFT;

		/* force off */
		val &= ~(LED_OFF_ON_MASK << LED_OFF_ON_SHIFT);
		val |= CA_LED_OFF << LED_OFF_ON_SHIFT;

		cortina_led_write(priv->regs, val);
	}

	return 0;
}

static int cortina_led_bind(struct udevice *parent)
{
	ofnode node;

	dev_for_each_subnode(node, parent) {
		struct udevice *dev;
		int ret;

		ret = device_bind_driver_to_node(parent, "ca-leds",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
	}

	return 0;
}

static const struct udevice_id ca_led_ids[] = {
	{ .compatible = "cortina,ca-leds" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(cortina_led) = {
	.name = "ca-leds",
	.id = UCLASS_LED,
	.of_match = ca_led_ids,
	.of_to_plat = ca_led_of_to_plat,
	.bind = cortina_led_bind,
	.probe = cortina_led_probe,
	.plat_auto	= sizeof(struct cortina_led_plat),
	.priv_auto	= sizeof(struct cortina_led_cfg),
	.ops = &cortina_led_ops,
};
