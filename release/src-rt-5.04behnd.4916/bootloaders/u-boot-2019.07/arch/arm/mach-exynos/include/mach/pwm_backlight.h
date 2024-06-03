/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef _PWM_BACKLIGHT_H_
#define _PWM_BACKLIGHT_H_

struct pwm_backlight_data {
	int pwm_id;
	int period;
	int max_brightness;
	int brightness;
};

extern int exynos_pwm_backlight_init(struct pwm_backlight_data *pd);

#endif /* _PWM_BACKLIGHT_H_ */
