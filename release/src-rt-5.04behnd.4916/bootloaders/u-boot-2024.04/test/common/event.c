// SPDX-License-Identifier: GPL-2.0+
/*
 * Unit tests for event handling
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <event.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>

struct test_state {
	struct udevice *dev;
	int val;
};

static bool called;

static int h_adder(void *ctx, struct event *event)
{
	struct event_data_test *data = &event->data.test;
	struct test_state *test_state = ctx;

	test_state->val += data->signal;

	return 0;
}

static int h_adder_simple(void)
{
	called = true;

	return 0;
}
EVENT_SPY_SIMPLE(EVT_TEST, h_adder_simple);

static int test_event_base(struct unit_test_state *uts)
{
	struct test_state state;
	int signal;

	state.val = 12;
	ut_assertok(event_register("wibble", EVT_TEST, h_adder, &state));

	signal = 17;

	/* Check that the handler is called */
	ut_assertok(event_notify(EVT_TEST, &signal, sizeof(signal)));
	ut_asserteq(12 + 17, state.val);

	return 0;
}
COMMON_TEST(test_event_base, 0);

static int test_event_simple(struct unit_test_state *uts)
{
	called = false;

	/* Check that the handler is called */
	ut_assertok(event_notify_null(EVT_TEST));
	ut_assert(called);

	return 0;
}
COMMON_TEST(test_event_simple, 0);

static int h_probe(void *ctx, struct event *event)
{
	struct test_state *test_state = ctx;

	test_state->dev = event->data.dm.dev;
	switch (event->type) {
	case EVT_DM_PRE_PROBE:
		test_state->val |= 1;
		break;
	case EVT_DM_POST_PROBE:
		test_state->val |= 2;
		break;
	default:
		break;
	}

	return 0;
}

static int test_event_probe(struct unit_test_state *uts)
{
	struct test_state state;
	struct udevice *dev;

	if (!IS_ENABLED(SANDBOX))
		return -EAGAIN;

	state.val = 0;
	ut_assertok(event_register("pre", EVT_DM_PRE_PROBE, h_probe, &state));
	ut_assertok(event_register("post", EVT_DM_POST_PROBE, h_probe, &state));

	/* Probe a device */
	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));

	/* Check that the handler is called */
	ut_asserteq(3, state.val);

	return 0;
}
COMMON_TEST(test_event_probe, UT_TESTF_DM | UT_TESTF_SCAN_FDT);
