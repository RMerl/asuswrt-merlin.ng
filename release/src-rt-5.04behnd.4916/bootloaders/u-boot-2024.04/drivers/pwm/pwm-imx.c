// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Basic support for the pwm module on imx6.
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <log.h>
#include <pwm.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <clk.h>

int pwm_config_internal(struct pwm_regs *pwm, unsigned long period_cycles,
			unsigned long duty_cycles, unsigned long prescale)
{
	u32 cr;

	writel(0, &pwm->ir);
	cr = PWMCR_PRESCALER(prescale) |
		PWMCR_DOZEEN | PWMCR_WAITEN |
		PWMCR_DBGEN | PWMCR_CLKSRC_IPG_HIGH;

	writel(cr, &pwm->cr);
	/* set duty cycles */
	writel(duty_cycles, &pwm->sar);
	/* set period cycles */
	writel(period_cycles, &pwm->pr);
	return 0;
}

#ifndef CONFIG_DM_PWM
/* pwm_id from 0..7 */
struct pwm_regs *pwm_id_to_reg(int pwm_id)
{

	switch (pwm_id) {
	case 0:
		return (struct pwm_regs *)PWM1_BASE_ADDR;
	case 1:
		return (struct pwm_regs *)PWM2_BASE_ADDR;
#ifdef CONFIG_MX6
	case 2:
		return (struct pwm_regs *)PWM3_BASE_ADDR;
	case 3:
		return (struct pwm_regs *)PWM4_BASE_ADDR;
#endif
#ifdef CONFIG_MX6SX
	case 4:
		return (struct pwm_regs *)PWM5_BASE_ADDR;
	case 5:
		return (struct pwm_regs *)PWM6_BASE_ADDR;
	case 6:
		return (struct pwm_regs *)PWM7_BASE_ADDR;
	case 7:
		return (struct pwm_regs *)PWM8_BASE_ADDR;
#endif
	default:
		printf("unknown pwm_id: %d\n", pwm_id);
		break;
	}
	return NULL;
}

int pwm_imx_get_parms(int period_ns, int duty_ns, unsigned long *period_c,
		      unsigned long *duty_c, unsigned long *prescale)
{
	unsigned long long c;

	/*
	 * we have not yet a clock framework for imx6, so add the clock
	 * value here as a define. Replace it when we have the clock
	 * framework.
	 */
	c = CFG_IMX6_PWM_PER_CLK;
	c = c * period_ns;
	do_div(c, 1000000000);
	*period_c = c;

	*prescale = *period_c / 0x10000 + 1;

	*period_c /= *prescale;
	c = *period_c * (unsigned long long)duty_ns;
	do_div(c, period_ns);
	*duty_c = c;

	/*
	 * according to imx pwm RM, the real period value should be
	 * PERIOD value in PWMPR plus 2.
	 */
	if (*period_c > 2)
		*period_c -= 2;
	else
		*period_c = 0;

	return 0;
}

int pwm_init(int pwm_id, int div, int invert)
{
	struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);

	if (!pwm)
		return -1;

	writel(0, &pwm->ir);
	return 0;
}

int pwm_config(int pwm_id, int duty_ns, int period_ns)
{
	struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);
	unsigned long period_cycles, duty_cycles, prescale;

	if (!pwm)
		return -1;

	pwm_imx_get_parms(period_ns, duty_ns, &period_cycles, &duty_cycles,
			  &prescale);

	return pwm_config_internal(pwm, period_cycles, duty_cycles, prescale);
}

int pwm_enable(int pwm_id)
{
	struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);

	if (!pwm)
		return -1;

	setbits_le32(&pwm->cr, PWMCR_EN);
	return 0;
}

void pwm_disable(int pwm_id)
{
	struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);

	if (!pwm)
		return;

	clrbits_le32(&pwm->cr, PWMCR_EN);
}

#else
struct imx_pwm_priv {
	struct pwm_regs *regs;
	bool invert;
	struct clk per_clk;
	struct clk ipg_clk;
};

int pwm_dm_imx_get_parms(struct imx_pwm_priv *priv, int period_ns,
		      int duty_ns, unsigned long *period_c, unsigned long *duty_c,
		      unsigned long *prescale)
{
	unsigned long long c;

	c = clk_get_rate(&priv->per_clk);
	c = c * period_ns;
	do_div(c, 1000000000);
	*period_c = c;

	*prescale = *period_c / 0x10000 + 1;

	*period_c /= *prescale;
	c = *period_c * (unsigned long long)duty_ns;
	do_div(c, period_ns);
	*duty_c = c;

	/*
	 * according to imx pwm RM, the real period value should be
	 * PERIOD value in PWMPR plus 2.
	 */
	if (*period_c > 2)
		*period_c -= 2;
	else
		*period_c = 0;

	return 0;
}

static int imx_pwm_set_invert(struct udevice *dev, uint channel,
			      bool polarity)
{
	struct imx_pwm_priv *priv = dev_get_priv(dev);

	debug("%s: polarity=%u\n", __func__, polarity);
	priv->invert = polarity;

	return 0;
}

static int imx_pwm_set_config(struct udevice *dev, uint channel,
			      uint period_ns, uint duty_ns)
{
	struct imx_pwm_priv *priv = dev_get_priv(dev);
	struct pwm_regs *regs = priv->regs;
	unsigned long period_cycles, duty_cycles, prescale;

	debug("%s: Config '%s' channel: %d\n", __func__, dev->name, channel);

	pwm_dm_imx_get_parms(priv, period_ns, duty_ns, &period_cycles, &duty_cycles,
			  &prescale);

	return pwm_config_internal(regs, period_cycles, duty_cycles, prescale);
};

static int imx_pwm_set_enable(struct udevice *dev, uint channel, bool enable)
{
	struct imx_pwm_priv *priv = dev_get_priv(dev);
	struct pwm_regs *regs = priv->regs;

	debug("%s: Enable '%s' state: %d\n", __func__, dev->name, enable);

	if (enable)
		setbits_le32(&regs->cr, PWMCR_EN);
	else
		clrbits_le32(&regs->cr, PWMCR_EN);

	return 0;
};

static int imx_pwm_of_to_plat(struct udevice *dev)
{
	int ret;
	struct imx_pwm_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);

	ret = clk_get_by_name(dev, "per", &priv->per_clk);
	if (ret) {
		printf("Failed to get per_clk\n");
		return ret;
	}

	ret = clk_get_by_name(dev, "ipg", &priv->ipg_clk);
	if (ret) {
		printf("Failed to get ipg_clk\n");
		return ret;
	}

	return 0;
}

static int imx_pwm_probe(struct udevice *dev)
{
	int ret;
	struct imx_pwm_priv *priv = dev_get_priv(dev);

	ret = clk_enable(&priv->per_clk);
	if (ret) {
		printf("Failed to enable per_clk\n");
		return ret;
	}

	ret = clk_enable(&priv->ipg_clk);
	if (ret) {
		printf("Failed to enable ipg_clk\n");
		return ret;
	}

	return 0;
}

static const struct pwm_ops imx_pwm_ops = {
	.set_invert	= imx_pwm_set_invert,
	.set_config	= imx_pwm_set_config,
	.set_enable	= imx_pwm_set_enable,
};

static const struct udevice_id imx_pwm_ids[] = {
	{ .compatible = "fsl,imx27-pwm" },
	{ }
};

U_BOOT_DRIVER(imx_pwm) = {
	.name	= "imx_pwm",
	.id	= UCLASS_PWM,
	.of_match = imx_pwm_ids,
	.ops	= &imx_pwm_ops,
	.of_to_plat	= imx_pwm_of_to_plat,
	.probe		= imx_pwm_probe,
	.priv_auto	= sizeof(struct imx_pwm_priv),
};
#endif
