// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Aspeed Technology Inc.
 *
 * PWM controller driver for Aspeed ast2600 SoCs.
 * This drivers doesn't support earlier version of the IP.
 *
 * The formula of pwm period duration:
 * period duration = ((DIV_L + 1) * (PERIOD + 1) << DIV_H) / input-clk
 *
 * The formula of pwm duty cycle duration:
 * duty cycle duration = period duration * DUTY_CYCLE_FALLING_POINT / (PERIOD + 1)
 * = ((DIV_L + 1) * DUTY_CYCLE_FALLING_POINT << DIV_H) / input-clk
 *
 * The software driver fixes the period to 255, which causes the high-frequency
 * precision of the PWM to be coarse, in exchange for the fineness of the duty cycle.
 *
 * Register usage:
 * PIN_ENABLE: When it is unset the pwm controller will always output low to the extern.
 * Use to determine whether the PWM channel is enabled or disabled
 * CLK_ENABLE: When it is unset the pwm controller will reset the duty counter to 0 and
 * output low to the PIN_ENABLE mux after that the driver can still change the pwm period
 * and duty and the value will apply when CLK_ENABLE be set again.
 * Use to determine whether duty_cycle bigger than 0.
 * PWM_ASPEED_CTRL_INVERSE: When it is toggled the output value will inverse immediately.
 * PWM_ASPEED_DUTY_CYCLE_FALLING_POINT/PWM_ASPEED_DUTY_CYCLE_RISING_POINT: When these two
 * values are equal it means the duty cycle = 100%.
 *
 * Limitations:
 * - When changing both duty cycle and period, we cannot prevent in
 *   software that the output might produce a period with mixed
 *   settings.
 * - Disabling the PWM doesn't complete the current period.
 *
 * Improvements:
 * - When only changing one of duty cycle or period, our pwm controller will not
 *   generate the glitch, the configure will change at next cycle of pwm.
 *   This improvement can disable/enable through PWM_ASPEED_CTRL_DUTY_SYNC_DISABLE.
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <clk.h>
#include <reset.h>
#include <regmap.h>
#include <syscon.h>
#include <dm/device_compat.h>
#include <linux/math64.h>
#include <linux/bitfield.h>
#include <linux/time.h>
#include <asm/io.h>

/* The channel number of Aspeed pwm controller */
#define PWM_ASPEED_NR_PWMS 16

/* PWM Control Register */
#define PWM_ASPEED_CTRL(ch) ((ch) * 0x10 + 0x00)
#define PWM_ASPEED_CTRL_LOAD_SEL_RISING_AS_WDT BIT(19)
#define PWM_ASPEED_CTRL_DUTY_LOAD_AS_WDT_ENABLE BIT(18)
#define PWM_ASPEED_CTRL_DUTY_SYNC_DISABLE BIT(17)
#define PWM_ASPEED_CTRL_CLK_ENABLE BIT(16)
#define PWM_ASPEED_CTRL_LEVEL_OUTPUT BIT(15)
#define PWM_ASPEED_CTRL_INVERSE BIT(14)
#define PWM_ASPEED_CTRL_OPEN_DRAIN_ENABLE BIT(13)
#define PWM_ASPEED_CTRL_PIN_ENABLE BIT(12)
#define PWM_ASPEED_CTRL_CLK_DIV_H GENMASK(11, 8)
#define PWM_ASPEED_CTRL_CLK_DIV_L GENMASK(7, 0)

/* PWM Duty Cycle Register */
#define PWM_ASPEED_DUTY_CYCLE(ch) ((ch) * 0x10 + 0x04)
#define PWM_ASPEED_DUTY_CYCLE_PERIOD GENMASK(31, 24)
#define PWM_ASPEED_DUTY_CYCLE_POINT_AS_WDT GENMASK(23, 16)
#define PWM_ASPEED_DUTY_CYCLE_FALLING_POINT GENMASK(15, 8)
#define PWM_ASPEED_DUTY_CYCLE_RISING_POINT GENMASK(7, 0)

/* PWM fixed value */
#define PWM_ASPEED_FIXED_PERIOD 0xff

struct aspeed_pwm_priv {
	struct clk clk;
	struct regmap *regmap;
	struct reset_ctl reset;
};

static int aspeed_pwm_set_invert(struct udevice *dev, uint channel, bool polarity)
{
	struct aspeed_pwm_priv *priv = dev_get_priv(dev);

	if (channel >= PWM_ASPEED_NR_PWMS)
		return -EINVAL;

	regmap_update_bits(priv->regmap, PWM_ASPEED_CTRL(channel),
			   PWM_ASPEED_CTRL_INVERSE,
			   FIELD_PREP(PWM_ASPEED_CTRL_INVERSE,
				      polarity));
	return 0;
}

static int aspeed_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct aspeed_pwm_priv *priv = dev_get_priv(dev);

	if (channel >= PWM_ASPEED_NR_PWMS)
		return -EINVAL;

	regmap_update_bits(priv->regmap, PWM_ASPEED_CTRL(channel),
			   PWM_ASPEED_CTRL_PIN_ENABLE,
			   enable ? PWM_ASPEED_CTRL_PIN_ENABLE : 0);
	return 0;
}

static int aspeed_pwm_set_config(struct udevice *dev, uint channel,
				 uint period_ns, uint duty_ns)
{
	struct aspeed_pwm_priv *priv = dev_get_priv(dev);
	u32 duty_pt;
	unsigned long rate;
	u64 div_h, div_l, divisor;
	bool clk_en;

	if (channel >= PWM_ASPEED_NR_PWMS)
		return -EINVAL;
	dev_dbg(dev, "expect period: %dns, duty_cycle: %dns\n", period_ns,
		duty_ns);

	rate = clk_get_rate(&priv->clk);
	/*
	 * Pick the smallest value for div_h so that div_l can be the biggest
	 * which results in a finer resolution near the target period value.
	 */
	divisor = (u64)NSEC_PER_SEC * (PWM_ASPEED_FIXED_PERIOD + 1) *
		  (PWM_ASPEED_CTRL_CLK_DIV_L + 1);
	div_h = order_base_2(div64_u64((u64)rate * period_ns + divisor - 1, divisor));
	if (div_h > 0xf)
		div_h = 0xf;

	divisor = ((u64)NSEC_PER_SEC * (PWM_ASPEED_FIXED_PERIOD + 1)) << div_h;
	div_l = div64_u64((u64)rate * period_ns, divisor);

	if (div_l == 0)
		return -ERANGE;

	div_l -= 1;

	if (div_l > 255)
		div_l = 255;

	dev_dbg(dev, "clk source: %ld div_h %lld, div_l : %lld\n", rate, div_h,
		div_l);
	/* duty_pt = duty_cycle * (PERIOD + 1) / period */
	duty_pt = div64_u64(duty_ns * (u64)rate,
			    (u64)NSEC_PER_SEC * (div_l + 1) << div_h);
	dev_dbg(dev, "duty_cycle = %d, duty_pt = %d\n", duty_ns,
		duty_pt);

	if (duty_pt == 0) {
		clk_en = 0;
	} else {
		clk_en = 1;
		if (duty_pt >= (PWM_ASPEED_FIXED_PERIOD + 1))
			duty_pt = 0;
		/*
		 * Fixed DUTY_CYCLE_PERIOD to its max value to get a
		 * fine-grained resolution for duty_cycle at the expense of a
		 * coarser period resolution.
		 */
		regmap_update_bits(priv->regmap, PWM_ASPEED_DUTY_CYCLE(channel),
				   PWM_ASPEED_DUTY_CYCLE_PERIOD |
				       PWM_ASPEED_DUTY_CYCLE_RISING_POINT |
				       PWM_ASPEED_DUTY_CYCLE_FALLING_POINT,
				   FIELD_PREP(PWM_ASPEED_DUTY_CYCLE_PERIOD,
					      PWM_ASPEED_FIXED_PERIOD) |
				       FIELD_PREP(PWM_ASPEED_DUTY_CYCLE_FALLING_POINT,
						  duty_pt));
	}

	regmap_update_bits(priv->regmap, PWM_ASPEED_CTRL(channel),
			   PWM_ASPEED_CTRL_CLK_DIV_H |
			       PWM_ASPEED_CTRL_CLK_DIV_L |
			       PWM_ASPEED_CTRL_CLK_ENABLE,
			   FIELD_PREP(PWM_ASPEED_CTRL_CLK_DIV_H, div_h) |
			       FIELD_PREP(PWM_ASPEED_CTRL_CLK_DIV_L, div_l) |
			       FIELD_PREP(PWM_ASPEED_CTRL_CLK_ENABLE, clk_en));
	return 0;
}

static int aspeed_pwm_probe(struct udevice *dev)
{
	int ret;
	struct aspeed_pwm_priv *priv = dev_get_priv(dev);
	struct udevice *parent_dev = dev_get_parent(dev);

	priv->regmap = syscon_node_to_regmap(dev_ofnode(dev->parent));
	if (IS_ERR(priv->regmap)) {
		dev_err(dev, "Couldn't get regmap\n");
		return PTR_ERR(priv->regmap);
	}

	ret = clk_get_by_index(parent_dev, 0, &priv->clk);
	if (ret < 0) {
		dev_err(dev, "get clock failed\n");
		return ret;
	}

	ret = reset_get_by_index(parent_dev, 0, &priv->reset);
	if (ret) {
		dev_err(dev, "get reset failed\n");
		return ret;
	}
	ret = reset_deassert(&priv->reset);
	if (ret) {
		dev_err(dev, "cannot deassert reset control: %pe\n",
			ERR_PTR(ret));
		return ret;
	}

	return 0;
}

static int aspeed_pwm_remove(struct udevice *dev)
{
	struct aspeed_pwm_priv *priv = dev_get_priv(dev);

	reset_assert(&priv->reset);

	return 0;
}

static const struct pwm_ops aspeed_pwm_ops = {
	.set_invert	= aspeed_pwm_set_invert,
	.set_config	= aspeed_pwm_set_config,
	.set_enable	= aspeed_pwm_set_enable,
};

static const struct udevice_id aspeed_pwm_ids[] = {
	{ .compatible = "aspeed,ast2600-pwm" },
	{ }
};

U_BOOT_DRIVER(aspeed_pwm) = {
	.name = "aspeed_pwm",
	.id = UCLASS_PWM,
	.of_match = aspeed_pwm_ids,
	.ops = &aspeed_pwm_ops,
	.probe = aspeed_pwm_probe,
	.remove = aspeed_pwm_remove,
	.priv_auto = sizeof(struct aspeed_pwm_priv),
};
