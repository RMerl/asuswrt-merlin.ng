// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for panel uclass
 *
 * Copyright (c) 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <video.h>
#include <asm/gpio.h>
#include <asm/test.h>
#include <dm/test.h>
#include <power/regulator.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of the panel uclass */
static int dm_test_panel(struct unit_test_state *uts)
{
	struct udevice *dev, *pwm, *gpio, *reg;
	uint period_ns;
	uint duty_ns;
	bool enable;
	bool polarity;

	ut_assertok(uclass_first_device_err(UCLASS_PANEL, &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_PWM, "pwm", &pwm));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_assertok(regulator_get_by_platname("VDD_EMMC_1.8V", &reg));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(false, enable);
	ut_asserteq(false, regulator_get_enable(reg));

	ut_assertok(panel_enable_backlight(dev));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(1000, period_ns);
	ut_asserteq(170 * 1000 / 255, duty_ns);
	ut_asserteq(true, enable);
	ut_asserteq(false, polarity);
	ut_asserteq(1, sandbox_gpio_get_value(gpio, 1));
	ut_asserteq(true, regulator_get_enable(reg));

	ut_assertok(panel_set_backlight(dev, 40));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(64 * 1000 / 255, duty_ns);

	ut_assertok(panel_set_backlight(dev, BACKLIGHT_MAX));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(255 * 1000 / 255, duty_ns);

	ut_assertok(panel_set_backlight(dev, BACKLIGHT_MIN));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(0 * 1000 / 255, duty_ns);
	ut_asserteq(1, sandbox_gpio_get_value(gpio, 1));

	ut_assertok(panel_set_backlight(dev, BACKLIGHT_DEFAULT));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(true, enable);
	ut_asserteq(170 * 1000 / 255, duty_ns);

	ut_assertok(panel_set_backlight(dev, BACKLIGHT_OFF));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(0 * 1000 / 255, duty_ns);
	ut_asserteq(0, sandbox_gpio_get_value(gpio, 1));
	ut_asserteq(false, regulator_get_enable(reg));

	return 0;
}
DM_TEST(dm_test_panel, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
