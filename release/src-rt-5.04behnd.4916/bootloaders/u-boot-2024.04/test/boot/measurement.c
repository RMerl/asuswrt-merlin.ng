// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for measured boot functions
 *
 * Copyright 2023 IBM Corp.
 * Written by Eddie James <eajames@linux.ibm.com>
 */

#include <common.h>
#include <bootm.h>
#include <malloc.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>
#include <asm/io.h>

#define MEASUREMENT_TEST(_name, _flags)	\
	UNIT_TEST(_name, _flags, measurement_test)

static int measure(struct unit_test_state *uts)
{
	struct bootm_headers images;
	const size_t size = 1024;
	u8 *kernel;
	u8 *initrd;
	size_t i;

	kernel = malloc(size);
	initrd = malloc(size);

	images.os.image_start = map_to_sysmem(kernel);
	images.os.image_len = size;

	images.rd_start = map_to_sysmem(initrd);
	images.rd_end = images.rd_start + size;

	images.ft_addr = malloc(size);
	images.ft_len = size;

	env_set("bootargs", "measurement testing");

	for (i = 0; i < size; ++i) {
		kernel[i] = 0xf0 | (i & 0xf);
		initrd[i] = (i & 0xf0) | 0xf;
		images.ft_addr[i] = i & 0xff;
	}

	ut_assertok(bootm_measure(&images));

	free(images.ft_addr);
	free(initrd);
	free(kernel);

	return 0;
}
MEASUREMENT_TEST(measure, 0);

int do_ut_measurement(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(measurement_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(measurement_test);

	return cmd_ut_category("measurement", "measurement_test_", tests,
			       n_ents, argc, argv);
}
