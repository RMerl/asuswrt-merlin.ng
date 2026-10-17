// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>
#include <asm/test.h>

/* Basic test of the audio codec uclass */
static int dm_test_audio(struct unit_test_state *uts)
{
	int interface, rate, mclk_freq, bits_per_sample;
	struct udevice *dev;
	uint channels;

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_AUDIO_CODEC, &dev));
	ut_assertok(audio_codec_set_params(dev, 1, 2, 3, 4, 5));
	sandbox_get_codec_params(dev, &interface, &rate, &mclk_freq,
				 &bits_per_sample, &channels);
	ut_asserteq(1, interface);
	ut_asserteq(2, rate);
	ut_asserteq(3, mclk_freq);
	ut_asserteq(4, bits_per_sample);
	ut_asserteq(5, channels);

	return 0;
}
DM_TEST(dm_test_audio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
