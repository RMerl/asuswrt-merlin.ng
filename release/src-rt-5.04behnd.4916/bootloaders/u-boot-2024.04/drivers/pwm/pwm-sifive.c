// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 SiFive, Inc
 * For SiFive's PWM IP block documentation please refer Chapter 14 of
 * Reference Manual : https://static.dev.sifive.com/FU540-C000-v1.0.pdf
 *
 * Limitations:
 * - When changing both duty cycle and period, we cannot prevent in
 *   software that the output might produce a period with mixed
 *   settings (new period length and old duty cycle).
 * - The hardware cannot generate a 100% duty cycle.
 * - The hardware generates only inverted output.
 */

#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <regmap.h>
#include <asm/global_data.h>
#include <linux/io.h>
#include <linux/log2.h>
#include <linux/bitfield.h>

/* PWMCFG fields */
#define PWM_SIFIVE_PWMCFG_SCALE         GENMASK(3, 0)
#define PWM_SIFIVE_PWMCFG_STICKY        BIT(8)
#define PWM_SIFIVE_PWMCFG_ZERO_CMP      BIT(9)
#define PWM_SIFIVE_PWMCFG_DEGLITCH      BIT(10)
#define PWM_SIFIVE_PWMCFG_EN_ALWAYS     BIT(12)
#define PWM_SIFIVE_PWMCFG_EN_ONCE       BIT(13)
#define PWM_SIFIVE_PWMCFG_CENTER        BIT(16)
#define PWM_SIFIVE_PWMCFG_GANG          BIT(24)
#define PWM_SIFIVE_PWMCFG_IP            BIT(28)

/* PWM_SIFIVE_SIZE_PWMCMP is used to calculate offset for pwmcmpX registers */
#define PWM_SIFIVE_SIZE_PWMCMP          4
#define PWM_SIFIVE_CMPWIDTH             16

#define PWM_SIFIVE_CHANNEL_ENABLE_VAL   0
#define PWM_SIFIVE_CHANNEL_DISABLE_VAL  0xffff

DECLARE_GLOBAL_DATA_PTR;

struct pwm_sifive_regs {
	unsigned long cfg;
	unsigned long cnt;
	unsigned long pwms;
	unsigned long cmp0;
};

struct pwm_sifive_data {
	struct pwm_sifive_regs regs;
};

struct pwm_sifive_priv {
	void __iomem *base;
	ulong freq;
	const struct pwm_sifive_data *data;
};

static int pwm_sifive_set_config(struct udevice *dev, uint channel,
				 uint period_ns, uint duty_ns)
{
	struct pwm_sifive_priv *priv = dev_get_priv(dev);
	const struct pwm_sifive_regs *regs = &priv->data->regs;
	unsigned long scale_pow;
	unsigned long long num;
	u32 scale, val = 0, frac;

	debug("%s: period_ns=%u, duty_ns=%u\n", __func__, period_ns, duty_ns);

	/*
	 * The PWM unit is used with pwmzerocmp=0, so the only way to modify the
	 * period length is using pwmscale which provides the number of bits the
	 * counter is shifted before being feed to the comparators. A period
	 * lasts (1 << (PWM_SIFIVE_CMPWIDTH + pwmscale)) clock ticks.
	 * (1 << (PWM_SIFIVE_CMPWIDTH + scale)) * 10^9/rate = period
	 */
	scale_pow = lldiv((uint64_t)priv->freq * period_ns, 1000000000);
	scale = clamp(ilog2(scale_pow) - PWM_SIFIVE_CMPWIDTH, 0, 0xf);
	val |= (FIELD_PREP(PWM_SIFIVE_PWMCFG_SCALE, scale) | PWM_SIFIVE_PWMCFG_EN_ALWAYS);

	/*
	 * The problem of output producing mixed setting as mentioned at top,
	 * occurs here. To minimize the window for this problem, we are
	 * calculating the register values first and then writing them
	 * consecutively
	 */
	num = (u64)duty_ns * (1U << PWM_SIFIVE_CMPWIDTH);
	frac = DIV_ROUND_CLOSEST_ULL(num, period_ns);
	frac = min(frac, (1U << PWM_SIFIVE_CMPWIDTH) - 1);
	frac = (1U << PWM_SIFIVE_CMPWIDTH) - 1 - frac;

	writel(val, priv->base + regs->cfg);
	writel(frac, priv->base + regs->cmp0 + channel *
	       PWM_SIFIVE_SIZE_PWMCMP);

	return 0;
}

static int pwm_sifive_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct pwm_sifive_priv *priv = dev_get_priv(dev);
	const struct pwm_sifive_regs *regs = &priv->data->regs;

	debug("%s: Enable '%s'\n", __func__, dev->name);

	if (enable)
		writel(PWM_SIFIVE_CHANNEL_ENABLE_VAL, priv->base +
		       regs->cmp0 + channel * PWM_SIFIVE_SIZE_PWMCMP);
	else
		writel(PWM_SIFIVE_CHANNEL_DISABLE_VAL, priv->base +
		       regs->cmp0 + channel * PWM_SIFIVE_SIZE_PWMCMP);

	return 0;
}

static int pwm_sifive_of_to_plat(struct udevice *dev)
{
	struct pwm_sifive_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);

	return 0;
}

static int pwm_sifive_probe(struct udevice *dev)
{
	struct pwm_sifive_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret = 0;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		debug("%s get clock fail!\n", __func__);
		return -EINVAL;
	}

	priv->freq = clk_get_rate(&clk);
	priv->data = (struct pwm_sifive_data *)dev_get_driver_data(dev);

	return 0;
}

static const struct pwm_ops pwm_sifive_ops = {
	.set_config	= pwm_sifive_set_config,
	.set_enable	= pwm_sifive_set_enable,
};

static const struct pwm_sifive_data pwm_data = {
	.regs = {
		.cfg = 0x00,
		.cnt = 0x08,
		.pwms = 0x10,
		.cmp0 = 0x20,
	},
};

static const struct udevice_id pwm_sifive_ids[] = {
	{ .compatible = "sifive,pwm0", .data = (ulong)&pwm_data},
	{ }
};

U_BOOT_DRIVER(pwm_sifive) = {
	.name	= "pwm_sifive",
	.id	= UCLASS_PWM,
	.of_match = pwm_sifive_ids,
	.ops	= &pwm_sifive_ops,
	.of_to_plat     = pwm_sifive_of_to_plat,
	.probe		= pwm_sifive_probe,
	.priv_auto	= sizeof(struct pwm_sifive_priv),
};
