// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/test.h>
#include <sysinfo.h>
#include <test/test.h>
#include <test/ut.h>

#include "../../drivers/sysinfo/sandbox.h"

static int dm_test_sysinfo(struct unit_test_state *uts)
{
	struct udevice *sysinfo;
	bool called_detect = false;
	char str[64];
	int i;

	ut_assertok(sysinfo_get(&sysinfo));
	ut_assert(sysinfo);

	ut_asserteq(-EPERM, sysinfo_get_bool(sysinfo, BOOL_CALLED_DETECT,
					     &called_detect));
	ut_assert(!called_detect);

	sysinfo_detect(sysinfo);

	ut_assertok(sysinfo_get_bool(sysinfo, BOOL_CALLED_DETECT,
				     &called_detect));
	ut_assert(called_detect);

	ut_assertok(sysinfo_get_str(sysinfo, STR_VACATIONSPOT, sizeof(str),
				    str));
	ut_assertok(strcmp(str, "R'lyeh"));

	ut_assertok(sysinfo_get_int(sysinfo, INT_TEST1, &i));
	ut_asserteq(0, i);

	ut_assertok(sysinfo_get_int(sysinfo, INT_TEST2, &i));
	ut_asserteq(100, i);

	ut_assertok(sysinfo_get_str(sysinfo, STR_VACATIONSPOT, sizeof(str),
				    str));
	ut_assertok(strcmp(str, "Carcosa"));

	ut_assertok(sysinfo_get_int(sysinfo, INT_TEST1, &i));
	ut_asserteq(1, i);

	ut_assertok(sysinfo_get_int(sysinfo, INT_TEST2, &i));
	ut_asserteq(99, i);

	ut_assertok(sysinfo_get_str(sysinfo, STR_VACATIONSPOT, sizeof(str),
				    str));
	ut_assertok(strcmp(str, "Yuggoth"));

	return 0;
}

DM_TEST(dm_test_sysinfo, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
