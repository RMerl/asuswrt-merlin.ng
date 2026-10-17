// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <pwm.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_cros_ec_pwm(struct unit_test_state *uts)
{
	struct udevice *pwm;
	struct udevice *ec;
	uint duty;

	ut_assertok(uclass_get_device_by_name(UCLASS_PWM, "cros-ec-pwm", &pwm));
	ut_assertnonnull(pwm);
	ec = dev_get_parent(pwm);
	ut_assertnonnull(ec);

	ut_assertok(pwm_set_config(pwm, 0, 100, 50));
	ut_assertok(pwm_set_enable(pwm, 0, true));
	ut_assertok(sandbox_cros_ec_get_pwm_duty(ec, 0, &duty));
	ut_asserteq(50 * EC_PWM_MAX_DUTY / 100, duty);

	ut_assertok(pwm_set_config(pwm, 0, 15721, 2719));
	ut_assertok(pwm_set_enable(pwm, 0, true));
	ut_assertok(sandbox_cros_ec_get_pwm_duty(ec, 0, &duty));
	ut_asserteq(2719 * EC_PWM_MAX_DUTY / 15721, duty);

	ut_assertok(pwm_set_enable(pwm, 0, false));
	ut_assertok(sandbox_cros_ec_get_pwm_duty(ec, 0, &duty));
	ut_asserteq(0, duty);

	ut_assertok(pwm_set_enable(pwm, 0, true));
	ut_assertok(sandbox_cros_ec_get_pwm_duty(ec, 0, &duty));
	ut_asserteq(2719 * EC_PWM_MAX_DUTY / 15721, duty);

	ut_assertok(pwm_set_config(pwm, 1, 1000, 0));
	ut_assertok(pwm_set_enable(pwm, 1, true));
	ut_assertok(sandbox_cros_ec_get_pwm_duty(ec, 1, &duty));
	ut_asserteq(0, duty);

	ut_assertok(pwm_set_config(pwm, 2, 1000, 1024));
	ut_assertok(pwm_set_enable(pwm, 2, true));
	ut_assertok(sandbox_cros_ec_get_pwm_duty(ec, 2, &duty));
	ut_asserteq(EC_PWM_MAX_DUTY, duty);

	ut_assertok(pwm_set_config(pwm, 3, EC_PWM_MAX_DUTY, 0xABCD));
	ut_assertok(pwm_set_enable(pwm, 3, true));
	ut_assertok(sandbox_cros_ec_get_pwm_duty(ec, 3, &duty));
	ut_asserteq(0xABCD, duty);

	ut_asserteq(-EINVAL, pwm_set_enable(pwm, 4, true));

	return 0;
}
DM_TEST(dm_test_cros_ec_pwm, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
