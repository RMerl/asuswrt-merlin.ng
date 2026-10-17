// SPDX-License-Identifier: GPL-2.0+
/*
 * EHRPWM PWM driver
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 *
 * Based on Linux kernel drivers/pwm/pwm-tiehrpwm.c
 */

#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <pwm.h>
#include <asm/io.h>
#include <linux/time.h>

/* Time base module registers */
#define TI_EHRPWM_TBCTL				0x00
#define TI_EHRPWM_TBPRD				0x0A

#define TI_EHRPWM_TBCTL_PRDLD_MASK		BIT(3)
#define TI_EHRPWM_TBCTL_PRDLD_SHDW		0
#define TI_EHRPWM_TBCTL_PRDLD_IMDT		BIT(3)
#define TI_EHRPWM_TBCTL_CLKDIV_MASK		GENMASK(12, 7)
#define TI_EHRPWM_TBCTL_CTRMODE_MASK		GENMASK(1, 0)
#define TI_EHRPWM_TBCTL_CTRMODE_UP		0
#define TI_EHRPWM_TBCTL_CTRMODE_DOWN		BIT(0)
#define TI_EHRPWM_TBCTL_CTRMODE_UPDOWN		BIT(1)
#define TI_EHRPWM_TBCTL_CTRMODE_FREEZE		GENMASK(1, 0)

#define TI_EHRPWM_TBCTL_HSPCLKDIV_SHIFT		7
#define TI_EHRPWM_TBCTL_CLKDIV_SHIFT		10

#define TI_EHRPWM_CLKDIV_MAX			7
#define TI_EHRPWM_HSPCLKDIV_MAX			7
#define TI_EHRPWM_PERIOD_MAX			0xFFFF

/* Counter compare module registers */
#define TI_EHRPWM_CMPA				0x12
#define TI_EHRPWM_CMPB				0x14

/* Action qualifier module registers */
#define TI_EHRPWM_AQCTLA			0x16
#define TI_EHRPWM_AQCTLB			0x18
#define TI_EHRPWM_AQSFRC			0x1A
#define TI_EHRPWM_AQCSFRC			0x1C

#define TI_EHRPWM_AQCTL_CBU_MASK		GENMASK(9, 8)
#define TI_EHRPWM_AQCTL_CBU_FRCLOW		BIT(8)
#define TI_EHRPWM_AQCTL_CBU_FRCHIGH		BIT(9)
#define TI_EHRPWM_AQCTL_CBU_FRCTOGGLE		GENMASK(9, 8)
#define TI_EHRPWM_AQCTL_CAU_MASK		GENMASK(5, 4)
#define TI_EHRPWM_AQCTL_CAU_FRCLOW		BIT(4)
#define TI_EHRPWM_AQCTL_CAU_FRCHIGH		BIT(5)
#define TI_EHRPWM_AQCTL_CAU_FRCTOGGLE		GENMASK(5, 4)
#define TI_EHRPWM_AQCTL_PRD_MASK		GENMASK(3, 2)
#define TI_EHRPWM_AQCTL_PRD_FRCLOW		BIT(2)
#define TI_EHRPWM_AQCTL_PRD_FRCHIGH		BIT(3)
#define TI_EHRPWM_AQCTL_PRD_FRCTOGGLE		GENMASK(3, 2)
#define TI_EHRPWM_AQCTL_ZRO_MASK		GENMASK(1, 0)
#define TI_EHRPWM_AQCTL_ZRO_FRCLOW		BIT(0)
#define TI_EHRPWM_AQCTL_ZRO_FRCHIGH		BIT(1)
#define TI_EHRPWM_AQCTL_ZRO_FRCTOGGLE		GENMASK(1, 0)

#define TI_EHRPWM_AQCTL_CHANA_POLNORMAL		(TI_EHRPWM_AQCTL_CAU_FRCLOW | \
						 TI_EHRPWM_AQCTL_PRD_FRCHIGH | \
						 TI_EHRPWM_AQCTL_ZRO_FRCHIGH)
#define TI_EHRPWM_AQCTL_CHANA_POLINVERSED	(TI_EHRPWM_AQCTL_CAU_FRCHIGH | \
						 TI_EHRPWM_AQCTL_PRD_FRCLOW | \
						 TI_EHRPWM_AQCTL_ZRO_FRCLOW)
#define TI_EHRPWM_AQCTL_CHANB_POLNORMAL		(TI_EHRPWM_AQCTL_CBU_FRCLOW | \
						 TI_EHRPWM_AQCTL_PRD_FRCHIGH | \
						 TI_EHRPWM_AQCTL_ZRO_FRCHIGH)
#define TI_EHRPWM_AQCTL_CHANB_POLINVERSED	(TI_EHRPWM_AQCTL_CBU_FRCHIGH | \
						 TI_EHRPWM_AQCTL_PRD_FRCLOW | \
						 TI_EHRPWM_AQCTL_ZRO_FRCLOW)

#define TI_EHRPWM_AQSFRC_RLDCSF_MASK		GENMASK(7, 6)
#define TI_EHRPWM_AQSFRC_RLDCSF_ZRO		0
#define TI_EHRPWM_AQSFRC_RLDCSF_PRD		BIT(6)
#define TI_EHRPWM_AQSFRC_RLDCSF_ZROPRD		BIT(7)
#define TI_EHRPWM_AQSFRC_RLDCSF_IMDT		GENMASK(7, 6)

#define TI_EHRPWM_AQCSFRC_CSFB_MASK		GENMASK(3, 2)
#define TI_EHRPWM_AQCSFRC_CSFB_FRCDIS		0
#define TI_EHRPWM_AQCSFRC_CSFB_FRCLOW		BIT(2)
#define TI_EHRPWM_AQCSFRC_CSFB_FRCHIGH		BIT(3)
#define TI_EHRPWM_AQCSFRC_CSFB_DISSWFRC		GENMASK(3, 2)
#define TI_EHRPWM_AQCSFRC_CSFA_MASK		GENMASK(1, 0)
#define TI_EHRPWM_AQCSFRC_CSFA_FRCDIS		0
#define TI_EHRPWM_AQCSFRC_CSFA_FRCLOW		BIT(0)
#define TI_EHRPWM_AQCSFRC_CSFA_FRCHIGH		BIT(1)
#define TI_EHRPWM_AQCSFRC_CSFA_DISSWFRC		GENMASK(1, 0)

#define TI_EHRPWM_NUM_CHANNELS                  2

struct ti_ehrpwm_priv {
	fdt_addr_t regs;
	u32 clk_rate;
	struct clk tbclk;
	unsigned long period_cycles[TI_EHRPWM_NUM_CHANNELS];
	bool polarity_reversed[TI_EHRPWM_NUM_CHANNELS];
};

static void ti_ehrpwm_modify(u16 val, u16 mask, fdt_addr_t reg)
{
	unsigned short v;

	v = readw(reg);
	v &= ~mask;
	v |= val & mask;
	writew(v, reg);
}

static int ti_ehrpwm_set_invert(struct udevice *dev, uint channel,
				bool polarity)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);

	if (channel >= TI_EHRPWM_NUM_CHANNELS)
		return -ENOSPC;

	/* Configuration of polarity in hardware delayed, do at enable */
	priv->polarity_reversed[channel] = polarity;
	return 0;
}

/**
 * set_prescale_div -	Set up the prescaler divider function
 * @rqst_prescaler:	prescaler value min
 * @prescale_div:	prescaler value set
 * @tb_clk_div:		Time Base Control prescaler bits
 */
static int set_prescale_div(unsigned long rqst_prescaler, u16 *prescale_div,
			    u16 *tb_clk_div)
{
	unsigned int clkdiv, hspclkdiv;

	for (clkdiv = 0; clkdiv <= TI_EHRPWM_CLKDIV_MAX; clkdiv++) {
		for (hspclkdiv = 0; hspclkdiv <= TI_EHRPWM_HSPCLKDIV_MAX;
		     hspclkdiv++) {
			/*
			 * calculations for prescaler value :
			 * prescale_div = HSPCLKDIVIDER * CLKDIVIDER.
			 * HSPCLKDIVIDER =  2 ** hspclkdiv
			 * CLKDIVIDER = (1),            if clkdiv == 0 *OR*
			 *              (2 * clkdiv),   if clkdiv != 0
			 *
			 * Configure prescale_div value such that period
			 * register value is less than 65535.
			 */

			*prescale_div = (1 << clkdiv) *
				(hspclkdiv ? (hspclkdiv * 2) : 1);
			if (*prescale_div > rqst_prescaler) {
				*tb_clk_div =
				    (clkdiv << TI_EHRPWM_TBCTL_CLKDIV_SHIFT) |
				    (hspclkdiv <<
				     TI_EHRPWM_TBCTL_HSPCLKDIV_SHIFT);
				return 0;
			}
		}
	}

	return 1;
}

static void ti_ehrpwm_configure_polarity(struct udevice *dev, uint channel)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);
	u16 aqctl_val, aqctl_mask;
	unsigned int aqctl_reg;

	/*
	 * Configure PWM output to HIGH/LOW level on counter
	 * reaches compare register value and LOW/HIGH level
	 * on counter value reaches period register value and
	 * zero value on counter
	 */
	if (channel == 1) {
		aqctl_reg = TI_EHRPWM_AQCTLB;
		aqctl_mask = TI_EHRPWM_AQCTL_CBU_MASK;

		if (priv->polarity_reversed[channel])
			aqctl_val = TI_EHRPWM_AQCTL_CHANB_POLINVERSED;
		else
			aqctl_val = TI_EHRPWM_AQCTL_CHANB_POLNORMAL;
	} else {
		aqctl_reg = TI_EHRPWM_AQCTLA;
		aqctl_mask = TI_EHRPWM_AQCTL_CAU_MASK;

		if (priv->polarity_reversed[channel])
			aqctl_val = TI_EHRPWM_AQCTL_CHANA_POLINVERSED;
		else
			aqctl_val = TI_EHRPWM_AQCTL_CHANA_POLNORMAL;
	}

	aqctl_mask |= TI_EHRPWM_AQCTL_PRD_MASK | TI_EHRPWM_AQCTL_ZRO_MASK;
	ti_ehrpwm_modify(aqctl_val, aqctl_mask, priv->regs + aqctl_reg);
}

/*
 * period_ns = 10^9 * (ps_divval * period_cycles) / PWM_CLK_RATE
 * duty_ns   = 10^9 * (ps_divval * duty_cycles) / PWM_CLK_RATE
 */
static int ti_ehrpwm_set_config(struct udevice *dev, uint channel,
				uint period_ns, uint duty_ns)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);
	u32 period_cycles, duty_cycles;
	u16 ps_divval, tb_divval;
	unsigned int i, cmp_reg;
	unsigned long long c;

	if (channel >= TI_EHRPWM_NUM_CHANNELS)
		return -ENOSPC;

	if (period_ns > NSEC_PER_SEC)
		return -ERANGE;

	c = priv->clk_rate;
	c = c * period_ns;
	do_div(c, NSEC_PER_SEC);
	period_cycles = (unsigned long)c;

	if (period_cycles < 1) {
		period_cycles = 1;
		duty_cycles = 1;
	} else {
		c = priv->clk_rate;
		c = c * duty_ns;
		do_div(c, NSEC_PER_SEC);
		duty_cycles = (unsigned long)c;
	}

	dev_dbg(dev, "channel=%d, period_ns=%d, duty_ns=%d\n",
		channel, period_ns, duty_ns);

	/*
	 * Period values should be same for multiple PWM channels as IP uses
	 * same period register for multiple channels.
	 */
	for (i = 0; i < TI_EHRPWM_NUM_CHANNELS; i++) {
		if (priv->period_cycles[i] &&
		    priv->period_cycles[i] != period_cycles) {
			/*
			 * Allow channel to reconfigure period if no other
			 * channels being configured.
			 */
			if (i == channel)
				continue;

			dev_err(dev, "period value conflicts with channel %u\n",
				i);
			return -EINVAL;
		}
	}

	priv->period_cycles[channel] = period_cycles;

	/* Configure clock prescaler to support Low frequency PWM wave */
	if (set_prescale_div(period_cycles / TI_EHRPWM_PERIOD_MAX, &ps_divval,
			     &tb_divval)) {
		dev_err(dev, "unsupported values\n");
		return -EINVAL;
	}

	/* Update clock prescaler values */
	ti_ehrpwm_modify(tb_divval, TI_EHRPWM_TBCTL_CLKDIV_MASK,
			 priv->regs + TI_EHRPWM_TBCTL);

	/* Update period & duty cycle with presacler division */
	period_cycles = period_cycles / ps_divval;
	duty_cycles = duty_cycles / ps_divval;

	/* Configure shadow loading on Period register */
	ti_ehrpwm_modify(TI_EHRPWM_TBCTL_PRDLD_SHDW, TI_EHRPWM_TBCTL_PRDLD_MASK,
			 priv->regs + TI_EHRPWM_TBCTL);

	writew(period_cycles, priv->regs + TI_EHRPWM_TBPRD);

	/* Configure ehrpwm counter for up-count mode */
	ti_ehrpwm_modify(TI_EHRPWM_TBCTL_CTRMODE_UP,
			 TI_EHRPWM_TBCTL_CTRMODE_MASK,
			 priv->regs + TI_EHRPWM_TBCTL);

	if (channel == 1)
		/* Channel 1 configured with compare B register */
		cmp_reg = TI_EHRPWM_CMPB;
	else
		/* Channel 0 configured with compare A register */
		cmp_reg = TI_EHRPWM_CMPA;

	writew(duty_cycles, priv->regs + cmp_reg);
	return 0;
}

static int ti_ehrpwm_disable(struct udevice *dev, uint channel)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);
	u16 aqcsfrc_val, aqcsfrc_mask;
	int err;

	if (channel >= TI_EHRPWM_NUM_CHANNELS)
		return -ENOSPC;

	/* Action Qualifier puts PWM output low forcefully */
	if (channel) {
		aqcsfrc_val = TI_EHRPWM_AQCSFRC_CSFB_FRCLOW;
		aqcsfrc_mask = TI_EHRPWM_AQCSFRC_CSFB_MASK;
	} else {
		aqcsfrc_val = TI_EHRPWM_AQCSFRC_CSFA_FRCLOW;
		aqcsfrc_mask = TI_EHRPWM_AQCSFRC_CSFA_MASK;
	}

	/* Update shadow register first before modifying active register */
	ti_ehrpwm_modify(TI_EHRPWM_AQSFRC_RLDCSF_ZRO,
			 TI_EHRPWM_AQSFRC_RLDCSF_MASK,
			 priv->regs + TI_EHRPWM_AQSFRC);

	ti_ehrpwm_modify(aqcsfrc_val, aqcsfrc_mask,
			 priv->regs + TI_EHRPWM_AQCSFRC);

	/*
	 * Changes to immediate action on Action Qualifier. This puts
	 * Action Qualifier control on PWM output from next TBCLK
	 */
	ti_ehrpwm_modify(TI_EHRPWM_AQSFRC_RLDCSF_IMDT,
			 TI_EHRPWM_AQSFRC_RLDCSF_MASK,
			 priv->regs + TI_EHRPWM_AQSFRC);

	ti_ehrpwm_modify(aqcsfrc_val, aqcsfrc_mask,
			 priv->regs + TI_EHRPWM_AQCSFRC);

	/* Disabling TBCLK on PWM disable */
	err = clk_disable(&priv->tbclk);
	if (err) {
		dev_err(dev, "failed to disable tbclk\n");
		return err;
	}

	return 0;
}

static int ti_ehrpwm_enable(struct udevice *dev, uint channel)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);
	u16 aqcsfrc_val, aqcsfrc_mask;
	int err;

	if (channel >= TI_EHRPWM_NUM_CHANNELS)
		return -ENOSPC;

	/* Disabling Action Qualifier on PWM output */
	if (channel) {
		aqcsfrc_val = TI_EHRPWM_AQCSFRC_CSFB_FRCDIS;
		aqcsfrc_mask = TI_EHRPWM_AQCSFRC_CSFB_MASK;
	} else {
		aqcsfrc_val = TI_EHRPWM_AQCSFRC_CSFA_FRCDIS;
		aqcsfrc_mask = TI_EHRPWM_AQCSFRC_CSFA_MASK;
	}

	/* Changes to shadow mode */
	ti_ehrpwm_modify(TI_EHRPWM_AQSFRC_RLDCSF_ZRO,
			 TI_EHRPWM_AQSFRC_RLDCSF_MASK,
			 priv->regs + TI_EHRPWM_AQSFRC);

	ti_ehrpwm_modify(aqcsfrc_val, aqcsfrc_mask,
			 priv->regs + TI_EHRPWM_AQCSFRC);

	/* Channels polarity can be configured from action qualifier module */
	ti_ehrpwm_configure_polarity(dev, channel);

	err = clk_enable(&priv->tbclk);
	if (err) {
		dev_err(dev, "failed to enable tbclk\n");
		return err;
	}

	return 0;
}

static int ti_ehrpwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	if (enable)
		return ti_ehrpwm_enable(dev, channel);

	return ti_ehrpwm_disable(dev, channel);
}

static int ti_ehrpwm_of_to_plat(struct udevice *dev)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr(dev);
	if (priv->regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "invalid address\n");
		return -EINVAL;
	}

	dev_dbg(dev, "regs=0x%08lx\n", priv->regs);
	return 0;
}

static int ti_ehrpwm_remove(struct udevice *dev)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);

	clk_release_all(&priv->tbclk, 1);
	return 0;
}

static int ti_ehrpwm_probe(struct udevice *dev)
{
	struct ti_ehrpwm_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int err;

	err = clk_get_by_name(dev, "fck", &clk);
	if (err) {
		dev_err(dev, "failed to get clock\n");
		return err;
	}

	priv->clk_rate = clk_get_rate(&clk);
	if (IS_ERR_VALUE(priv->clk_rate) || !priv->clk_rate) {
		dev_err(dev, "failed to get clock rate\n");
		if (IS_ERR_VALUE(priv->clk_rate))
			return priv->clk_rate;

		return -EINVAL;
	}

	/* Acquire tbclk for Time Base EHRPWM submodule */
	err = clk_get_by_name(dev, "tbclk", &priv->tbclk);
	if (err) {
		dev_err(dev, "failed to get tbclk clock\n");
		return err;
	}

	return 0;
}

static const struct pwm_ops ti_ehrpwm_ops = {
	.set_config = ti_ehrpwm_set_config,
	.set_enable = ti_ehrpwm_set_enable,
	.set_invert = ti_ehrpwm_set_invert,
};

static const struct udevice_id ti_ehrpwm_ids[] = {
	{.compatible = "ti,am3352-ehrpwm"},
	{.compatible = "ti,am33xx-ehrpwm"},
	{}
};

U_BOOT_DRIVER(ti_ehrpwm) = {
	.name = "ti_ehrpwm",
	.id = UCLASS_PWM,
	.of_match = ti_ehrpwm_ids,
	.ops = &ti_ehrpwm_ops,
	.of_to_plat = ti_ehrpwm_of_to_plat,
	.probe = ti_ehrpwm_probe,
	.remove = ti_ehrpwm_remove,
	.priv_auto = sizeof(struct ti_ehrpwm_priv),
};
