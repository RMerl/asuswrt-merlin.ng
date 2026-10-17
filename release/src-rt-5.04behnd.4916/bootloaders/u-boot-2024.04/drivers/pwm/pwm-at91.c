// SPDX-License-Identifier: GPL-2.0+
/*
 * PWM support for Microchip AT91 architectures.
 *
 * Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Dan Sneddon <daniel.sneddon@microchip.com>
 *
 * Based on drivers/pwm/pwm-atmel.c from Linux.
 */
#include <clk.h>
#include <common.h>
#include <div64.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/time.h>
#include <pwm.h>

#define PERIOD_BITS 16
#define PWM_MAX_PRES 10

#define PWM_ENA 0x04
#define PWM_CHANNEL_OFFSET 0x20
#define PWM_CMR 0x200
#define PWM_CMR_CPRE_MSK GENMASK(3, 0)
#define PWM_CMR_CPOL BIT(9)
#define PWM_CDTY 0x204
#define PWM_CPRD 0x20C

struct at91_pwm_priv {
	void __iomem *base;
	struct clk pclk;
	u32 clkrate;
};

static int at91_pwm_calculate_cprd_and_pres(struct udevice *dev,
					    unsigned long clkrate,
					    uint period_ns, uint duty_ns,
					    unsigned long *cprd, u32 *pres)
{
	u64 cycles = period_ns;
	int shift;

	/* Calculate the period cycles and prescale value */
	cycles *= clkrate;
	do_div(cycles, NSEC_PER_SEC);

	/*
	 * The register for the period length is period_bits bits wide.
	 * So for each bit the number of clock cycles is wider divide the input
	 * clock frequency by two using pres and shift cprd accordingly.
	 */
	shift = fls(cycles) - PERIOD_BITS;

	if (shift > PWM_MAX_PRES) {
		return -EINVAL;
	} else if (shift > 0) {
		*pres = shift;
		cycles >>= *pres;
	} else {
		*pres = 0;
	}

	*cprd = cycles;

	return 0;
}

static void at91_pwm_calculate_cdty(uint period_ns, uint duty_ns,
				    unsigned long clkrate, unsigned long cprd,
				     u32 pres, unsigned long *cdty)
{
	u64 cycles = duty_ns;

	cycles *= clkrate;
	do_div(cycles, NSEC_PER_SEC);
	cycles >>= pres;
	*cdty = cprd - cycles;
}

/**
 * Returns: channel status after set operation
 */
static bool at91_pwm_set(void __iomem *base, uint channel, bool enable)
{
	u32 val, cur_status;

	val = ioread32(base + PWM_ENA);
	cur_status = !!(val & BIT(channel));

	/* if channel is already in that state, do nothing */
	if (!(enable ^ cur_status))
		return cur_status;

	if (enable)
		val |= BIT(channel);
	else
		val &= ~(BIT(channel));

	iowrite32(val, base + PWM_ENA);

	return cur_status;
}

static int at91_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct at91_pwm_priv *priv = dev_get_priv(dev);

	at91_pwm_set(priv->base, channel, enable);

	return 0;
}

static int at91_pwm_set_config(struct udevice *dev, uint channel,
			       uint period_ns, uint duty_ns)
{
	struct at91_pwm_priv *priv = dev_get_priv(dev);
	unsigned long cprd, cdty;
	u32 pres, val;
	int channel_enabled;
	int ret;

	ret = at91_pwm_calculate_cprd_and_pres(dev, priv->clkrate, period_ns,
					       duty_ns, &cprd, &pres);
	if (ret)
		return ret;

	at91_pwm_calculate_cdty(period_ns, duty_ns, priv->clkrate, cprd, pres, &cdty);

	/* disable the channel */
	channel_enabled = at91_pwm_set(priv->base, channel, false);

	/* It is necessary to preserve CPOL, inside CMR */
	val = ioread32(priv->base + (channel * PWM_CHANNEL_OFFSET) + PWM_CMR);
	val = (val & ~PWM_CMR_CPRE_MSK) | (pres & PWM_CMR_CPRE_MSK);
	iowrite32(val, priv->base + (channel * PWM_CHANNEL_OFFSET) + PWM_CMR);

	iowrite32(cprd, priv->base + (channel * PWM_CHANNEL_OFFSET) + PWM_CPRD);

	iowrite32(cdty, priv->base + (channel * PWM_CHANNEL_OFFSET) + PWM_CDTY);

	/* renable the channel if needed */
	if (channel_enabled)
		at91_pwm_set(priv->base, channel, true);

	return 0;
}

static int at91_pwm_set_invert(struct udevice *dev, uint channel,
			       bool polarity)
{
	struct at91_pwm_priv *priv = dev_get_priv(dev);
	u32 val;

	val = ioread32(priv->base + (channel * PWM_CHANNEL_OFFSET) + PWM_CMR);
	if (polarity)
		val |= PWM_CMR_CPOL;
	else
		val &= ~PWM_CMR_CPOL;
	iowrite32(val, priv->base + (channel * PWM_CHANNEL_OFFSET) + PWM_CMR);

	return 0;
}

static int at91_pwm_probe(struct udevice *dev)
{
	struct at91_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->pclk);
	if (ret)
		return ret;

	/* clocks aren't ref-counted so just enabled them once here */
	ret = clk_enable(&priv->pclk);
	if (ret)
		return ret;

	priv->clkrate = clk_get_rate(&priv->pclk);

	return ret;
}

static const struct pwm_ops at91_pwm_ops = {
	.set_config = at91_pwm_set_config,
	.set_enable = at91_pwm_set_enable,
	.set_invert = at91_pwm_set_invert,
};

static const struct udevice_id at91_pwm_of_match[] = {
	{ .compatible = "atmel,sama5d2-pwm" },
	{ }
};

U_BOOT_DRIVER(at91_pwm) = {
	.name = "at91_pwm",
	.id = UCLASS_PWM,
	.of_match = at91_pwm_of_match,
	.probe = at91_pwm_probe,
	.priv_auto = sizeof(struct at91_pwm_priv),
	.ops = &at91_pwm_ops,
};
