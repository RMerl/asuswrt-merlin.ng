// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <tpm_api.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/*
 * get_tpm_version() - Get a TPM of the given version
 *
 * @version: Version to get
 * @devp: Returns the TPM device
 * Returns: 0 if OK, -ENODEV if not found
 */
static int get_tpm_version(enum tpm_version version, struct udevice **devp)
{
	struct udevice *dev;

	/*
	 * For now we have to probe each TPM, since the version is set up in
	 * of_to_plat(). We could require TPMs to declare their version when
	 * probed, to avoid this
	 */
	uclass_foreach_dev_probe(UCLASS_TPM, dev) {
		if (tpm_get_version(dev) == version) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

/* Basic test of initing a TPM */
static int test_tpm_init(struct unit_test_state *uts, enum tpm_version version)
{
	struct udevice *dev;

	/* check probe success */
	ut_assertok(get_tpm_version(version, &dev));

	ut_assertok(tpm_init(dev));

	return 0;
}

static int dm_test_tpm(struct unit_test_state *uts)
{
	ut_assertok(test_tpm_init(uts, TPM_V1));
	ut_assertok(test_tpm_init(uts, TPM_V2));

	return 0;
}
DM_TEST(dm_test_tpm, UT_TESTF_SCAN_FDT);

/* Test report_state */
static int dm_test_tpm_report_state(struct unit_test_state *uts)
{
	struct udevice *dev;
	char buf[50];

	/* check probe success */
	ut_assertok(get_tpm_version(TPM_V2, &dev));

	ut_assert(tpm_report_state(dev, buf, sizeof(buf)));
	ut_asserteq_str("init_done=0", buf);

	ut_assertok(tpm_auto_start(dev));

	ut_assert(tpm_report_state(dev, buf, sizeof(buf)));
	ut_asserteq_str("init_done=1", buf);

	return 0;
}
DM_TEST(dm_test_tpm_report_state, UT_TESTF_SCAN_FDT);

/**
 * test_tpm_autostart() - check the tpm_auto_start() call
 *
 * @uts: Unit test state
 * @version: TPM version to use
 * @reinit: true to call tpm_init() first
 * Returns 0 if OK, non-zero on failure
 */
static int test_tpm_autostart(struct unit_test_state *uts,
			      enum tpm_version version, bool reinit)
{
	struct udevice *dev;

	/* check probe success */
	ut_assertok(get_tpm_version(version, &dev));

	if (reinit)
		ut_assertok(tpm_init(dev));

	/*
	 * tpm_auto_start will rerun tpm_init() if reinit, but handles the
	 * -EBUSY return code internally.
	 */
	ut_assertok(tpm_auto_start(dev));

	return 0;
}

static int dm_test_tpm_autostart(struct unit_test_state *uts)
{
	ut_assertok(test_tpm_autostart(uts, TPM_V1, false));
	ut_assertok(test_tpm_autostart(uts, TPM_V2, false));

	return 0;
}
DM_TEST(dm_test_tpm_autostart, UT_TESTF_SCAN_FDT);

static int dm_test_tpm_autostart_reinit(struct unit_test_state *uts)
{
	ut_assertok(test_tpm_autostart(uts, TPM_V1, true));
	ut_assertok(test_tpm_autostart(uts, TPM_V2, true));

	return 0;
}
DM_TEST(dm_test_tpm_autostart_reinit, UT_TESTF_SCAN_FDT);
