// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Google Inc.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pwm.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/pwm.h>

struct exynos_pwm_priv {
	struct s5p_timer *regs;
};

static int exynos_pwm_set_config(struct udevice *dev, uint channel,
				uint period_ns, uint duty_ns)
{
	struct exynos_pwm_priv *priv = dev_get_priv(dev);
	struct s5p_timer *regs = priv->regs;
	unsigned int offset, prescaler;
	uint div = 4, rate, rate_ns;
	u32 val;
	u32 tcnt, tcmp, tcon;

	if (channel >= 5)
		return -EINVAL;
	debug("%s: Configure '%s' channel %u, period_ns %u, duty_ns %u\n",
	      __func__, dev->name, channel, period_ns, duty_ns);

	val = readl(&regs->tcfg0);
	prescaler = (channel < 2 ? val : (val >> 8)) & 0xff;
	div = (readl(&regs->tcfg1) >> MUX_DIV_SHIFT(channel)) & 0xf;

	rate = get_pwm_clk() / ((prescaler + 1) * (1 << div));
	debug("%s: pwm_clk %lu, rate %u\n", __func__, get_pwm_clk(), rate);

	if (channel < 4) {
		rate_ns = 1000000000 / rate;
		tcnt = period_ns / rate_ns;
		tcmp = duty_ns / rate_ns;
		debug("%s: tcnt %u, tcmp %u\n", __func__, tcnt, tcmp);

		/* Ensure that the comparitor will actually hit the target */
		if (tcmp == tcnt)
			tcmp = tcnt - 1;
		offset = channel * 3;
		writel(tcnt, &regs->tcntb0 + offset);
		writel(tcmp, &regs->tcmpb0 + offset);
	}

	tcon = readl(&regs->tcon);
	tcon |= TCON_UPDATE(channel);
	if (channel < 4)
		tcon |= TCON_AUTO_RELOAD(channel);
	else
		tcon |= TCON4_AUTO_RELOAD;
	writel(tcon, &regs->tcon);

	tcon &= ~TCON_UPDATE(channel);
	writel(tcon, &regs->tcon);

	return 0;
}

static int exynos_pwm_set_enable(struct udevice *dev, uint channel,
				 bool enable)
{
	struct exynos_pwm_priv *priv = dev_get_priv(dev);
	struct s5p_timer *regs = priv->regs;
	u32 mask;

	if (channel >= 4)
		return -EINVAL;
	debug("%s: Enable '%s' channel %u\n", __func__, dev->name, channel);
	mask = TCON_START(channel);
	clrsetbits_le32(&regs->tcon, mask, enable ? mask : 0);

	return 0;
}

static int exynos_pwm_probe(struct udevice *dev)
{
	struct exynos_pwm_priv *priv = dev_get_priv(dev);
	struct s5p_timer *regs = priv->regs;

	writel(PRESCALER_0 | PRESCALER_1 << 8, &regs->tcfg0);

	return 0;
}

static int exynos_pwm_of_to_plat(struct udevice *dev)
{
	struct exynos_pwm_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);

	return 0;
}

static const struct pwm_ops exynos_pwm_ops = {
	.set_config	= exynos_pwm_set_config,
	.set_enable	= exynos_pwm_set_enable,
};

static const struct udevice_id exynos_channels[] = {
	{ .compatible = "samsung,exynos4210-pwm" },
	{ }
};

U_BOOT_DRIVER(exynos_pwm) = {
	.name	= "exynos_pwm",
	.id	= UCLASS_PWM,
	.of_match = exynos_channels,
	.ops	= &exynos_pwm_ops,
	.probe	= exynos_pwm_probe,
	.of_to_plat	= exynos_pwm_of_to_plat,
	.priv_auto	= sizeof(struct exynos_pwm_priv),
};
