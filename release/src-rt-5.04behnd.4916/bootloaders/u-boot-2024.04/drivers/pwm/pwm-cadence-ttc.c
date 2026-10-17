// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021 Xilinx, Inc. Michal Simek
 */

#define LOG_CATEGORY UCLASS_PWM

#include <clk.h>
#include <common.h>
#include <div64.h>
#include <dm.h>
#include <log.h>
#include <pwm.h>
#include <asm/io.h>
#include <log.h>
#include <div64.h>
#include <linux/bitfield.h>
#include <linux/math64.h>
#include <linux/log2.h>
#include <linux/time.h>
#include <dm/device_compat.h>

#define CLOCK_CONTROL		0
#define COUNTER_CONTROL		0xc
#define INTERVAL_COUNTER	0x24
#define MATCH_1_COUNTER		0x30

#define CLK_FALLING_EDGE	BIT(6)
#define CLK_SRC_EXTERNAL	BIT(5)
#define CLK_PRESCALE_MASK	GENMASK(4, 1)
#define CLK_PRESCALE_ENABLE	BIT(0)

#define COUNTER_WAVE_POL		BIT(6)
#define COUNTER_WAVE_DISABLE		BIT(5)
#define COUNTER_RESET			BIT(4)
#define COUNTER_MATCH_ENABLE		BIT(3)
#define COUNTER_DECREMENT_ENABLE	BIT(2)
#define COUNTER_INTERVAL_ENABLE		BIT(1)
#define COUNTER_COUNTING_DISABLE	BIT(0)

#define TTC_REG(reg, channel) ((reg) + (channel) * sizeof(u32))
#define TTC_CLOCK_CONTROL(reg, channel) \
	TTC_REG((reg) + CLOCK_CONTROL, (channel))
#define TTC_COUNTER_CONTROL(reg, channel) \
	TTC_REG((reg) + COUNTER_CONTROL, (channel))
#define TTC_INTERVAL_COUNTER(reg, channel) \
	TTC_REG((reg) + INTERVAL_COUNTER, (channel))
#define TTC_MATCH_1_COUNTER(reg, channel) \
	TTC_REG((reg) + MATCH_1_COUNTER, (channel))

struct cadence_ttc_pwm_plat {
	u8 *regs;
	u32 timer_width;
};

struct cadence_ttc_pwm_priv {
	u8 *regs;
	u32 timer_width;
	u32 timer_mask;
	unsigned long frequency;
	bool invert[2];
};

static int cadence_ttc_pwm_set_invert(struct udevice *dev, uint channel,
				      bool polarity)
{
	struct cadence_ttc_pwm_priv *priv = dev_get_priv(dev);

	if (channel > 2) {
		dev_err(dev, "Unsupported channel number %d(max 2)\n", channel);
		return -EINVAL;
	}

	priv->invert[channel] = polarity;

	dev_dbg(dev, "polarity=%u. Please config PWM again\n", polarity);

	return 0;
}

static int cadence_ttc_pwm_set_config(struct udevice *dev, uint channel,
				      uint period_ns, uint duty_ns)
{
	struct cadence_ttc_pwm_priv *priv = dev_get_priv(dev);
	u32 counter_ctrl, clock_ctrl;
	int period_clocks, duty_clocks, prescaler;

	dev_dbg(dev, "channel %d, duty %d/period %d ns\n", channel,
		duty_ns, period_ns);

	if (channel > 2) {
		dev_err(dev, "Unsupported channel number %d(max 2)\n", channel);
		return -EINVAL;
	}

	/* Make sure counter is stopped */
	counter_ctrl = readl(TTC_COUNTER_CONTROL(priv->regs, channel));
	setbits_le32(TTC_COUNTER_CONTROL(priv->regs, channel),
		     COUNTER_COUNTING_DISABLE | COUNTER_WAVE_DISABLE);

	/* Calculate period, prescaler and set clock control register */
	period_clocks = div64_u64(((int64_t)period_ns * priv->frequency),
				  NSEC_PER_SEC);

	prescaler = ilog2(period_clocks) + 1 - priv->timer_width;
	if (prescaler < 0)
		prescaler = 0;

	clock_ctrl = readl(TTC_CLOCK_CONTROL(priv->regs, channel));

	if (!prescaler) {
		clock_ctrl &= ~(CLK_PRESCALE_ENABLE | CLK_PRESCALE_MASK);
	} else {
		clock_ctrl &= ~CLK_PRESCALE_MASK;
		clock_ctrl |= CLK_PRESCALE_ENABLE;
		clock_ctrl |= FIELD_PREP(CLK_PRESCALE_MASK, prescaler - 1);
	};

	/* External source is not handled by this driver now */
	clock_ctrl &= ~CLK_SRC_EXTERNAL;

	writel(clock_ctrl, TTC_CLOCK_CONTROL(priv->regs, channel));

	/* Calculate interval and set counter control value */
	duty_clocks = div64_u64(((int64_t)duty_ns * priv->frequency),
				NSEC_PER_SEC);

	writel((period_clocks >> prescaler) & priv->timer_mask,
	       TTC_INTERVAL_COUNTER(priv->regs, channel));
	writel((duty_clocks >> prescaler) & priv->timer_mask,
	       TTC_MATCH_1_COUNTER(priv->regs, channel));

	/* Restore/reset counter */
	counter_ctrl &= ~COUNTER_DECREMENT_ENABLE;
	counter_ctrl |= COUNTER_INTERVAL_ENABLE |
			COUNTER_RESET |
			COUNTER_MATCH_ENABLE;

	if (priv->invert[channel])
		counter_ctrl |= COUNTER_WAVE_POL;
	else
		counter_ctrl &= ~COUNTER_WAVE_POL;

	writel(counter_ctrl, TTC_COUNTER_CONTROL(priv->regs, channel));

	dev_dbg(dev, "%d/%d clocks, prescaler 2^%d\n", duty_clocks,
		period_clocks, prescaler);

	return 0;
};

static int cadence_ttc_pwm_set_enable(struct udevice *dev, uint channel,
				      bool enable)
{
	struct cadence_ttc_pwm_priv *priv = dev_get_priv(dev);

	if (channel > 2) {
		dev_err(dev, "Unsupported channel number %d(max 2)\n", channel);
		return -EINVAL;
	}

	dev_dbg(dev, "Enable: %d, channel %d\n", enable, channel);

	if (enable) {
		clrbits_le32(TTC_COUNTER_CONTROL(priv->regs, channel),
			     COUNTER_COUNTING_DISABLE |
			     COUNTER_WAVE_DISABLE);
		setbits_le32(TTC_COUNTER_CONTROL(priv->regs, channel),
			     COUNTER_RESET);
	} else {
		setbits_le32(TTC_COUNTER_CONTROL(priv->regs, channel),
			     COUNTER_COUNTING_DISABLE |
			     COUNTER_WAVE_DISABLE);
	}

	return 0;
};

static int cadence_ttc_pwm_probe(struct udevice *dev)
{
	struct cadence_ttc_pwm_priv *priv = dev_get_priv(dev);
	struct cadence_ttc_pwm_plat *plat = dev_get_plat(dev);
	struct clk clk;
	int ret;

	priv->regs = plat->regs;
	priv->timer_width = plat->timer_width;
	priv->timer_mask = GENMASK(priv->timer_width - 1, 0);

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	priv->frequency = clk_get_rate(&clk);
	if (IS_ERR_VALUE(priv->frequency)) {
		dev_err(dev, "failed to get rate\n");
		return priv->frequency;
	}
	dev_dbg(dev, "Clk frequency: %ld\n", priv->frequency);

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	return 0;
}

static int cadence_ttc_pwm_of_to_plat(struct udevice *dev)
{
	struct cadence_ttc_pwm_plat *plat = dev_get_plat(dev);
	const char *cells;

	cells = dev_read_prop(dev, "#pwm-cells", NULL);
	if (!cells)
		return -EINVAL;

	plat->regs = dev_read_addr_ptr(dev);

	plat->timer_width = dev_read_u32_default(dev, "timer-width", 16);

	return 0;
}

static int cadence_ttc_pwm_bind(struct udevice *dev)
{
	const char *cells;

	cells = dev_read_prop(dev, "#pwm-cells", NULL);
	if (!cells)
		return -ENODEV;

	return 0;
}

static const struct pwm_ops cadence_ttc_pwm_ops = {
	.set_invert = cadence_ttc_pwm_set_invert,
	.set_config = cadence_ttc_pwm_set_config,
	.set_enable = cadence_ttc_pwm_set_enable,
};

static const struct udevice_id cadence_ttc_pwm_ids[] = {
	{ .compatible = "cdns,ttc" },
	{ }
};

U_BOOT_DRIVER(cadence_ttc_pwm) = {
	.name = "cadence_ttc_pwm",
	.id = UCLASS_PWM,
	.of_match = cadence_ttc_pwm_ids,
	.ops = &cadence_ttc_pwm_ops,
	.bind = cadence_ttc_pwm_bind,
	.of_to_plat = cadence_ttc_pwm_of_to_plat,
	.probe = cadence_ttc_pwm_probe,
	.priv_auto = sizeof(struct cadence_ttc_pwm_priv),
	.plat_auto = sizeof(struct cadence_ttc_pwm_plat),
};
