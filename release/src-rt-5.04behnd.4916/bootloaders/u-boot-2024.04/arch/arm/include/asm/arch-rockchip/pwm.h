/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Google, Inc
 * (C) Copyright 2008-2014 Rockchip Electronics
 */

#ifndef _ASM_ARCH_PWM_H
#define _ASM_ARCH_PWM_H

struct rockchip_pwm_regs {
	unsigned long duty;
	unsigned long period;
	unsigned long cntr;
	unsigned long ctrl;
};

#define PWM_CTRL_TIMER_EN		(1 << 0)
#define PWM_CTRL_OUTPUT_EN		(1 << 3)

#define RK_PWM_DISABLE                  (0 << 0)
#define RK_PWM_ENABLE                   (1 << 0)

#define PWM_ONE_SHOT                    (0 << 1)
#define PWM_CONTINUOUS                  (1 << 1)
#define RK_PWM_CAPTURE                  (1 << 2)

#define PWM_DUTY_POSTIVE                (1 << 3)
#define PWM_DUTY_NEGATIVE               (0 << 3)
#define PWM_DUTY_MASK			(1 << 3)

#define PWM_INACTIVE_POSTIVE            (1 << 4)
#define PWM_INACTIVE_NEGATIVE           (0 << 4)
#define PWM_INACTIVE_MASK		(1 << 4)

#define PWM_OUTPUT_LEFT                 (0 << 5)
#define PWM_OUTPUT_CENTER               (1 << 5)

#define PWM_LOCK			(1 << 6)
#define PWM_UNLOCK			(0 << 6)

#define PWM_LP_ENABLE                   (1 << 8)
#define PWM_LP_DISABLE                  (0 << 8)

#define PWM_SEL_SCALE_CLK		(1 << 9)
#define PWM_SEL_SRC_CLK			(0 << 9)

#endif
