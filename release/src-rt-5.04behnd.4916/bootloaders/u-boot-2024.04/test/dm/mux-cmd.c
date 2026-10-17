// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Texas Instruments Inc.
 * Pratyush Yadav <p.yadav@ti.com>
 */
#include <common.h>
#include <dm.h>
#include <mux.h>
#include <mux-internal.h>
#include <dt-bindings/mux/mux.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>
#include <console.h>
#include <rand.h>

#define BUF_SIZE		256

/* Test 'mux list' */
static int dm_test_cmd_mux_list(struct unit_test_state *uts)
{
	char str[BUF_SIZE], *tok;
	struct udevice *dev;
	struct mux_chip *chip;
	struct mux_control *mux;
	int i;
	unsigned long val;

	sandbox_set_enable_memio(true);

	ut_assertok(uclass_get_device_by_name(UCLASS_MUX, "a-mux-controller",
					      &dev));
	chip = dev_get_uclass_priv(dev);
	ut_assertnonnull(chip);

	run_command("mux list", 0);
	ut_assert_nextline("a-mux-controller:");

	/*
	 * Check the table header to make sure we are not out of sync with the
	 * code in the command. If we are, catch it early.
	 */
	console_record_readline(str, BUF_SIZE);
	tok = strtok(str, " ");
	ut_asserteq_str("ID", tok);

	tok = strtok(NULL, " ");
	ut_asserteq_str("Selected", tok);

	tok = strtok(NULL, " ");
	ut_asserteq_str("Current", tok);
	tok = strtok(NULL, " ");
	ut_asserteq_str("State", tok);

	tok = strtok(NULL, " ");
	ut_asserteq_str("Idle", tok);
	tok = strtok(NULL, " ");
	ut_asserteq_str("State", tok);

	tok = strtok(NULL, " ");
	ut_asserteq_str("Num", tok);
	tok = strtok(NULL, " ");
	ut_asserteq_str("States", tok);

	for (i = 0; i < chip->controllers; i++) {
		mux = &chip->mux[i];

		console_record_readline(str, BUF_SIZE);

		/*
		 * Check if the ID printed matches with the ID of the chip we
		 * have.
		 */
		tok = strtok(str, " ");
		ut_assertok(strict_strtoul(tok, 10, &val));
		ut_asserteq(i, val);

		/* Check if mux selection state matches. */
		tok = strtok(NULL, " ");
		if (mux->in_use) {
			ut_asserteq_str("yes", tok);
		} else {
			ut_asserteq_str("no", tok);
		}

		/* Check if the current state matches. */
		tok = strtok(NULL, " ");
		if (mux->cached_state == MUX_IDLE_AS_IS) {
			ut_asserteq_str("unknown", tok);
		} else {
			ut_assertok(strict_strtoul(tok, 16, &val));
			ut_asserteq(mux->cached_state, val);
		}

		/* Check if the idle state matches */
		tok = strtok(NULL, " ");
		if (mux->idle_state == MUX_IDLE_AS_IS) {
			ut_asserteq_str("as-is", tok);
		} else {
			ut_assertok(strict_strtoul(tok, 16, &val));
			ut_asserteq(mux->idle_state, val);
		}

		/* Check if the number of states matches */
		tok = strtok(NULL, " ");
		ut_assertok(strict_strtoul(tok, 16, &val));
		ut_asserteq(mux->states, val);
	}

	return 0;
}
DM_TEST(dm_test_cmd_mux_list, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_cmd_mux_select(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct mux_chip *chip;
	struct mux_control *mux;
	char cmd[BUF_SIZE];
	unsigned int i, state;

	sandbox_set_enable_memio(true);

	ut_assertok(uclass_get_device_by_name(UCLASS_MUX, "a-mux-controller",
					      &dev));
	chip = dev_get_uclass_priv(dev);
	ut_assertnonnull(chip);

	srand(get_ticks() + rand());
	for (i = 0; i < chip->controllers; i++) {
		mux = &chip->mux[i];

		state = rand() % mux->states;

		snprintf(cmd, BUF_SIZE, "mux select a-mux-controller %x %x", i,
			 state);
		run_command(cmd, 0);
		ut_asserteq(!!mux->in_use, true);
		ut_asserteq(state, mux->cached_state);

		ut_assertok(mux_control_deselect(mux));
	}

	return 0;
}
DM_TEST(dm_test_cmd_mux_select, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_cmd_mux_deselect(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct mux_chip *chip;
	struct mux_control *mux;
	char cmd[BUF_SIZE];
	unsigned int i, state;

	sandbox_set_enable_memio(true);

	ut_assertok(uclass_get_device_by_name(UCLASS_MUX, "a-mux-controller",
					      &dev));
	chip = dev_get_uclass_priv(dev);
	ut_assertnonnull(chip);

	srand(get_ticks() + rand());
	for (i = 0; i < chip->controllers; i++) {
		mux = &chip->mux[i];

		state = rand() % mux->states;
		ut_assertok(mux_control_select(mux, state));

		snprintf(cmd, BUF_SIZE, "mux deselect a-mux-controller %d", i);
		run_command(cmd, 0);
		ut_asserteq(!!mux->in_use, false);
	}

	return 0;
}
DM_TEST(dm_test_cmd_mux_deselect, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
