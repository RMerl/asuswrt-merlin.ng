// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <sysinfo.h>
#include <asm/gpio.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_sysinfo_gpio(struct unit_test_state *uts)
{
	char buf[64];
	int val;
	struct udevice *sysinfo, *gpio;

	ut_assertok(uclass_get_device_by_name(UCLASS_SYSINFO, "sysinfo-gpio",
					      &sysinfo));
	ut_assertok(uclass_get_device_by_name(UCLASS_GPIO, "base-gpios", &gpio));

	/*
	 * Set up pins: pull-up (1), pull-down (0) and floating (2). This should
	 * result in digits 2 0 1, i.e. 2 * 9 + 1 * 3 = 19
	 */
	sandbox_gpio_set_flags(gpio, 15, GPIOD_EXT_PULL_UP);
	sandbox_gpio_set_flags(gpio, 16, GPIOD_EXT_PULL_DOWN);
	sandbox_gpio_set_flags(gpio, 17, 0);
	ut_assertok(sysinfo_detect(sysinfo));
	ut_assertok(sysinfo_get_int(sysinfo, SYSINFO_ID_BOARD_MODEL, &val));
	ut_asserteq(19, val);
	ut_assertok(sysinfo_get_str(sysinfo, SYSINFO_ID_BOARD_MODEL, sizeof(buf),
				    buf));
	ut_asserteq_str("rev_a", buf);

	/*
	 * Set up pins: floating (2), pull-up (1) and pull-down (0). This should
	 * result in digits 0 1 2, i.e. 1 * 3 + 2 = 5
	 */
	sandbox_gpio_set_flags(gpio, 15, 0);
	sandbox_gpio_set_flags(gpio, 16, GPIOD_EXT_PULL_UP);
	sandbox_gpio_set_flags(gpio, 17, GPIOD_EXT_PULL_DOWN);
	ut_assertok(sysinfo_detect(sysinfo));
	ut_assertok(sysinfo_get_int(sysinfo, SYSINFO_ID_BOARD_MODEL, &val));
	ut_asserteq(5, val);
	ut_assertok(sysinfo_get_str(sysinfo, SYSINFO_ID_BOARD_MODEL, sizeof(buf),
				    buf));
	ut_asserteq_str("foo", buf);

	/*
	 * Set up pins: floating (2), pull-up (1) and pull-down (0). This should
	 * result in digits 1 2 0, i.e. 1 * 9 + 2 * 3 = 15
	 */
	sandbox_gpio_set_flags(gpio, 15, GPIOD_EXT_PULL_DOWN);
	sandbox_gpio_set_flags(gpio, 16, 0);
	sandbox_gpio_set_flags(gpio, 17, GPIOD_EXT_PULL_UP);
	ut_assertok(sysinfo_detect(sysinfo));
	ut_assertok(sysinfo_get_int(sysinfo, SYSINFO_ID_BOARD_MODEL, &val));
	ut_asserteq(15, val);
	ut_assertok(sysinfo_get_str(sysinfo, SYSINFO_ID_BOARD_MODEL, sizeof(buf),
				    buf));
	ut_asserteq_str("unknown", buf);

	return 0;
}
DM_TEST(dm_test_sysinfo_gpio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
