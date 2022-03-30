// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Basic support for the pwm module on imx6.
 */

#include <common.h>
#include <div64.h>
#include <pwm.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include "pwm-imx-util.h"

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
	u32 cr;

	if (!pwm)
		return -1;

	pwm_imx_get_parms(period_ns, duty_ns, &period_cycles, &duty_cycles,
			  &prescale);

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
