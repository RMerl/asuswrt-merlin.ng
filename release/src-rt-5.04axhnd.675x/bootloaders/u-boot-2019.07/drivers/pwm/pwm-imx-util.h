/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Basic support for the pwm module on imx6.
 */

#ifndef _pwm_imx_util_h_
#define _pwm_imx_util_h_

struct pwm_regs *pwm_id_to_reg(int pwm_id);
int pwm_imx_get_parms(int period_ns, int duty_ns, unsigned long *period_c,
		      unsigned long *duty_c, unsigned long *prescale);
#endif
