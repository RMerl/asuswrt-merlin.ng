// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <asm/state.h>
#include <dm/test.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

struct unit_test_state global_dm_test_state;
static struct dm_test_state _global_priv_dm_test_state;

/* Get ready for testing */
static int dm_test_init(struct unit_test_state *uts, bool of_live)
{
	struct dm_test_state *dms = uts->priv;

	memset(dms, '\0', sizeof(*dms));
	gd->dm_root = NULL;
	memset(dm_testdrv_op_count, '\0', sizeof(dm_testdrv_op_count));
	state_reset_for_test(state_get_current());

#ifdef CONFIG_OF_LIVE
	/* Determine whether to make the live tree available */
	gd->of_root = of_live ? uts->of_root : NULL;
#endif
	ut_assertok(dm_init(of_live));
	dms->root = dm_root();

	return 0;
}

/* Ensure all the test devices are probed */
static int do_autoprobe(struct unit_test_state *uts)
{
	struct udevice *dev;
	int ret;

	/* Scanning the uclass is enough to probe all the devices */
	for (ret = uclass_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_next_device(&dev))
		;

	return ret;
}

static int dm_test_destroy(struct unit_test_state *uts)
{
	int id;

	for (id = 0; id < UCLASS_COUNT; id++) {
		struct uclass *uc;

		/*
		 * If the uclass doesn't exist we don't want to create it. So
		 * check that here before we call uclass_find_device()/
		 */
		uc = uclass_find(id);
		if (!uc)
			continue;
		ut_assertok(uclass_destroy(uc));
	}

	return 0;
}

static int dm_do_test(struct unit_test_state *uts, struct unit_test *test,
		      bool of_live)
{
	struct sandbox_state *state = state_get_current();
	const char *fname = strrchr(test->file, '/') + 1;

	printf("Test: %s: %s%s\n", test->name, fname,
	       !of_live ? " (flat tree)" : "");
	ut_assertok(dm_test_init(uts, of_live));

	uts->start = mallinfo();
	if (test->flags & DM_TESTF_SCAN_PDATA)
		ut_assertok(dm_scan_platdata(false));
	if (test->flags & DM_TESTF_PROBE_TEST)
		ut_assertok(do_autoprobe(uts));
	if (test->flags & DM_TESTF_SCAN_FDT)
		ut_assertok(dm_extended_scan_fdt(gd->fdt_blob, false));

	/*
	 * Silence the console and rely on console recording to get
	 * our output.
	 */
	console_record_reset();
	if (!state->show_test_output)
		gd->flags |= GD_FLG_SILENT;
	test->func(uts);
	gd->flags &= ~GD_FLG_SILENT;
	state_set_skip_delays(false);

	ut_assertok(dm_test_destroy(uts));

	return 0;
}

/**
 * dm_test_run_on_flattree() - Check if we should run a test with flat DT
 *
 * This skips long/slow tests where there is not much value in running a flat
 * DT test in addition to a live DT test.
 *
 * @return true to run the given test on the flat device tree
 */
static bool dm_test_run_on_flattree(struct unit_test *test)
{
	const char *fname = strrchr(test->file, '/') + 1;

	return !strstr(fname, "video") || strstr(test->name, "video_base");
}

static int dm_test_main(const char *test_name)
{
	struct unit_test *tests = ll_entry_start(struct unit_test, dm_test);
	const int n_ents = ll_entry_count(struct unit_test, dm_test);
	struct unit_test_state *uts = &global_dm_test_state;
	struct unit_test *test;
	int run_count;

	uts->priv = &_global_priv_dm_test_state;
	uts->fail_count = 0;

	/*
	 * If we have no device tree, or it only has a root node, then these
	 * tests clearly aren't going to work...
	 */
	if (!gd->fdt_blob || fdt_next_node(gd->fdt_blob, 0, NULL) < 0) {
		puts("Please run with test device tree:\n"
		     "    ./u-boot -d arch/sandbox/dts/test.dtb\n");
		ut_assert(gd->fdt_blob);
	}

	if (!test_name)
		printf("Running %d driver model tests\n", n_ents);

	run_count = 0;
#ifdef CONFIG_OF_LIVE
	uts->of_root = gd->of_root;
#endif
	for (test = tests; test < tests + n_ents; test++) {
		const char *name = test->name;
		int runs;

		/* All tests have this prefix */
		if (!strncmp(name, "dm_test_", 8))
			name += 8;
		if (test_name && strcmp(test_name, name))
			continue;

		/* Run with the live tree if possible */
		runs = 0;
		if (IS_ENABLED(CONFIG_OF_LIVE)) {
			if (!(test->flags & DM_TESTF_FLAT_TREE)) {
				ut_assertok(dm_do_test(uts, test, true));
				runs++;
			}
		}

		/*
		 * Run with the flat tree if we couldn't run it with live tree,
		 * or it is a core test.
		 */
		if (!(test->flags & DM_TESTF_LIVE_TREE) &&
		    (!runs || dm_test_run_on_flattree(test))) {
			ut_assertok(dm_do_test(uts, test, false));
			runs++;
		}
		run_count += runs;
	}

	if (test_name && !run_count)
		printf("Test '%s' not found\n", test_name);
	else
		printf("Failures: %d\n", uts->fail_count);

	gd->dm_root = NULL;
	ut_assertok(dm_init(false));
	dm_scan_platdata(false);
	dm_scan_fdt(gd->fdt_blob, false);

	return uts->fail_count ? CMD_RET_FAILURE : 0;
}

int do_ut_dm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *test_name = NULL;

	if (argc > 1)
		test_name = argv[1];

	return dm_test_main(test_name);
}
