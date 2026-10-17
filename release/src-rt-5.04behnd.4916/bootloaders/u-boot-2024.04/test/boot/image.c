// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for vbe-simple bootmeth. All start with 'vbe_simple'
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <image.h>
#include <test/suites.h>
#include <test/ut.h>
#include "bootstd_common.h"

/* Test of image phase */
static int test_image_phase(struct unit_test_state *uts)
{
	int val;

	ut_asserteq_str("U-Boot phase", genimg_get_phase_name(IH_PHASE_U_BOOT));
	ut_asserteq_str("SPL Phase", genimg_get_phase_name(IH_PHASE_SPL));
	ut_asserteq_str("any", genimg_get_phase_name(IH_PHASE_NONE));
	ut_asserteq_str("Unknown Phase", genimg_get_phase_name(-1));

	ut_asserteq(IH_PHASE_U_BOOT, genimg_get_phase_id("u-boot"));
	ut_asserteq(IH_PHASE_SPL, genimg_get_phase_id("spl"));
	ut_asserteq(IH_PHASE_NONE, genimg_get_phase_id("none"));
	ut_asserteq(-1, genimg_get_phase_id("fred"));

	val = image_ph(IH_PHASE_SPL, IH_TYPE_FIRMWARE);
	ut_asserteq(IH_PHASE_SPL, image_ph_phase(val));
	ut_asserteq(IH_TYPE_FIRMWARE, image_ph_type(val));

	return 0;
}
BOOTSTD_TEST(test_image_phase, 0);
