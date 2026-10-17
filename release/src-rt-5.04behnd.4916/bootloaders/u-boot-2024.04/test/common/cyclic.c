// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <cyclic.h>
#include <dm.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>
#include <watchdog.h>
#include <linux/delay.h>

/* Test that cyclic function is called */
static bool cyclic_active = false;

static void cyclic_test(void *ctx)
{
	cyclic_active = true;
}

static int dm_test_cyclic_running(struct unit_test_state *uts)
{
	cyclic_active = false;
	ut_assertnonnull(cyclic_register(cyclic_test, 10 * 1000, "cyclic_demo",
					 NULL));

	/* Execute all registered cyclic functions */
	schedule();
	ut_asserteq(true, cyclic_active);

	return 0;
}
COMMON_TEST(dm_test_cyclic_running, 0);
