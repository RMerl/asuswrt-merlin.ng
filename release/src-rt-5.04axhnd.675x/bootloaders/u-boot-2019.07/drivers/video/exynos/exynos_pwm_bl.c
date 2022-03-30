// SPDX-License-Identifier: GPL-2.0+
/*
 * PWM BACKLIGHT driver for Board based on EXYNOS.
 *
 * Author: Donghwa Lee  <dh09.lee@samsung.com>
 *
 * Derived from linux/drivers/video/backlight/pwm_backlight.c
 */

#include <common.h>
#include <pwm.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pwm.h>
#include <asm/arch/pwm_backlight.h>

static struct pwm_backlight_data *pwm;

static int exynos_pwm_backlight_update_status(void)
{
	int brightness = pwm->brightness;
	int max = pwm->max_brightness;

	if (brightness == 0) {
		pwm_config(pwm->pwm_id, 0, pwm->period);
		pwm_disable(pwm->pwm_id);
	} else {
		pwm_config(pwm->pwm_id,
			brightness * pwm->period / max, pwm->period);
		pwm_enable(pwm->pwm_id);
	}
	return 0;
}

int exynos_pwm_backlight_init(struct pwm_backlight_data *pd)
{
	pwm = pd;

	exynos_pwm_backlight_update_status();

	return 0;
}
