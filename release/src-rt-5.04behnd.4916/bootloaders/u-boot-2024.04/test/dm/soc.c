// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for the SOC uclass
 *
 * (C) Copyright 2020 - Texas Instruments Incorporated - https://www.ti.com/
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <soc.h>
#include <test/ut.h>

struct sb_soc_data {
	unsigned long param;
};

static int dm_test_soc(struct unit_test_state *uts)
{
	struct udevice *dev;
	char text[128];
	const struct soc_attr *soc_data;
	const struct sb_soc_data *match_data;

	static const struct sb_soc_data soc_sandbox1_sr10_data = { 0x91919191 };
	static const struct sb_soc_data soc_sandbox123_data    = { 0x84848484 };

	static const struct soc_attr sb_soc_devices_full[] = {
		{
			.family = "SANDBOX0xx",
			.machine = "SANDBOX012",
			.revision = "1.0",
			.data = NULL,
		},
		{
			.family = "SANDBOX1xx",
			.machine = "SANDBOX107",
			.revision = "1.0",
			.data = NULL,
		},
		{
			.family = "SANDBOX1xx",
			.machine = "SANDBOX123",
			.revision = "1.0",
			.data = &soc_sandbox123_data,
		},
		{
			.family = "SANDBOX1xx",
			.machine = "SANDBOX131",
			.revision = "2.0",
			.data = NULL,
		},
		{ /* sentinel */ }
	};

	static const struct soc_attr sb_soc_devices_partial[] = {
		{
			.family = "SANDBOX0xx",
			.revision = "1.0",
			.data = NULL,
		},
		{
			.family = "SANDBOX1xx",
			.revision = "1.0",
			.data = &soc_sandbox1_sr10_data,
		},
		{
			.family = "SANDBOX1xx",
			.revision = "2.0",
			.data = NULL,
		},
		{ /* sentinel */ }
	};

	static const struct soc_attr sb_soc_devices_nomatch[] = {
		{
			.family = "SANDBOX0xx",
			.revision = "1.0",
			.data = NULL,
		},
		{
			.family = "SANDBOX1xx",
			.revision = "2.0",
			.data = NULL,
		},
		{ /* sentinel */ }
	};

	ut_assertok(soc_get(&dev));

	ut_assertok(soc_get_machine(dev, text, sizeof(text)));
	ut_assertok(strcmp(text, "SANDBOX123"));

	ut_assertok(soc_get_family(dev, text, sizeof(text)));
	ut_assertok(strcmp(text, "SANDBOX1xx"));

	ut_assertok(soc_get_revision(dev, text, sizeof(text)));
	ut_asserteq_str(text, "1.0");

	soc_data = soc_device_match(sb_soc_devices_full);
	ut_assert(soc_data);

	match_data = soc_data->data;
	ut_asserteq(match_data->param, 0x84848484);

	soc_data = soc_device_match(sb_soc_devices_partial);
	ut_assert(soc_data);

	match_data = soc_data->data;
	ut_asserteq(match_data->param, 0x91919191);

	soc_data = soc_device_match(sb_soc_devices_nomatch);
	ut_asserteq_ptr(soc_data, NULL);

	return 0;
}

DM_TEST(dm_test_soc, UT_TESTF_SCAN_FDT);
