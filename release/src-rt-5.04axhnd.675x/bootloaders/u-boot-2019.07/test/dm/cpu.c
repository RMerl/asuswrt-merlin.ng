// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <cpu.h>
#include <test/ut.h>

static int dm_test_cpu(struct unit_test_state *uts)
{
	struct udevice *dev;
	char text[128];
	struct cpu_info info;

	ut_assertok(cpu_probe_all());

	/* Check that cpu_probe_all really activated all CPUs */
	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev))
		ut_assert(dev->flags & DM_FLAG_ACTIVATED);

	ut_assertok(uclass_get_device_by_name(UCLASS_CPU, "cpu-test1", &dev));

	ut_assertok(cpu_get_desc(dev, text, sizeof(text)));
	ut_assertok(strcmp(text, "LEG Inc. SuperMegaUltraTurbo CPU No. 1"));

	ut_assertok(cpu_get_info(dev, &info));
	ut_asserteq(info.cpu_freq, 42 * 42 * 42 * 42 * 42);
	ut_asserteq(info.features, 0x42424242);

	ut_asserteq(cpu_get_count(dev), 42);

	ut_assertok(cpu_get_vendor(dev, text, sizeof(text)));
	ut_assertok(strcmp(text, "Languid Example Garbage Inc."));

	return 0;
}

DM_TEST(dm_test_cpu, DM_TESTF_SCAN_FDT);
