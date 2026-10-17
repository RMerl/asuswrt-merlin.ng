// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <pwm.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of the pwm uclass */
static int dm_test_pwm_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	uint period_ns;
	uint duty_ns;
	bool enable;
	bool polarity;

	ut_assertok(uclass_get_device_by_name(UCLASS_PWM, "pwm", &dev));
	ut_assertnonnull(dev);
	ut_assertok(pwm_set_config(dev, 0, 100, 50));
	ut_assertok(pwm_set_enable(dev, 0, true));
	ut_assertok(pwm_set_enable(dev, 1, true));
	ut_assertok(pwm_set_enable(dev, 2, true));
	ut_asserteq(-ENOSPC, pwm_set_enable(dev, 3, true));
	ut_assertok(pwm_set_invert(dev, 0, true));

	ut_assertok(pwm_set_config(dev, 2, 100, 50));
	ut_assertok(sandbox_pwm_get_config(dev, 2, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(period_ns, 4096);
	ut_asserteq(duty_ns, 50 * 4096 / 100);

	ut_assertok(uclass_get_device(UCLASS_PWM, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_PWM, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_PWM, 2, &dev));
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_PWM, 3, &dev));

	return 0;
}
DM_TEST(dm_test_pwm_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
