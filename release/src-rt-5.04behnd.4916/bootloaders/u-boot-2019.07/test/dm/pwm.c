// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <pwm.h>
#include <dm/test.h>
#include <test/ut.h>

/* Basic test of the pwm uclass */
static int dm_test_pwm_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_PWM, 0, &dev));
	ut_assertnonnull(dev);
	ut_assertok(pwm_set_config(dev, 0, 100, 50));
	ut_assertok(pwm_set_enable(dev, 0, true));
	ut_assertok(pwm_set_enable(dev, 1, true));
	ut_assertok(pwm_set_enable(dev, 2, true));
	ut_asserteq(-ENOSPC, pwm_set_enable(dev, 3, true));
	ut_assertok(pwm_set_invert(dev, 0, true));

	ut_assertok(uclass_get_device(UCLASS_PWM, 1, &dev));
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_PWM, 2, &dev));

	return 0;
}
DM_TEST(dm_test_pwm_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
