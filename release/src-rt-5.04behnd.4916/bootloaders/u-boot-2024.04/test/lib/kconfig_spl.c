// SPDX-License-Identifier: GPL-2.0+
/*
 * Test of linux/kconfig.h macros for SPL
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static int lib_test_spl_is_enabled(struct unit_test_state *uts)
{
	ulong val;

	ut_asserteq(0, CONFIG_IS_ENABLED(CMDLINE));
	ut_asserteq(1, CONFIG_IS_ENABLED(OF_PLATDATA));
	ut_asserteq(0, CONFIG_IS_ENABLED(_UNDEFINED));

	/*
	 * This fails if CONFIG_TEST_KCONFIG_ENABLE is not enabled, since the
	 * value is used.
	 */
	if (IS_ENABLED(CONFIG_TEST_KCONFIG)) {
		val = IF_ENABLED_INT(CONFIG_TEST_KCONFIG_ENABLE,
				     CONFIG_TEST_KCONFIG_VALUE);
		printf("value %ld\n", val);
	}

	/*
	 * This fails if CONFIG_TEST_KCONFIG_ENABLE is not enabled, since the
	 * value is used.
	 */
	if (CONFIG_IS_ENABLED(TEST_KCONFIG)) {
		val = CONFIG_IF_ENABLED_INT(TEST_KCONFIG_ENABLE,
					    TEST_KCONFIG_VALUE);
		printf("value2 %ld\n", val);
	}

	return 0;
}
LIB_TEST(lib_test_spl_is_enabled, 0);
