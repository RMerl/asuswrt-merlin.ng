// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the driver model ADC API
 *
 * Copyright (c) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <adc.h>
#include <dm.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/test.h>
#include <errno.h>
#include <fdtdec.h>
#include <power/regulator.h>
#include <power/sandbox_pmic.h>
#include <sandbox-adc.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_adc_bind(struct unit_test_state *uts)
{
	struct udevice *dev;
	unsigned int channel_mask;

	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));
	ut_asserteq_str(SANDBOX_ADC_DEVNAME, dev->name);

	ut_assertok(adc_channel_mask(dev, &channel_mask));
	ut_asserteq((1 << SANDBOX_ADC_CHANNELS) - 1, channel_mask);

	return 0;
}
DM_TEST(dm_test_adc_bind, UT_TESTF_SCAN_FDT);

static int dm_test_adc_wrong_channel_selection(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));
	ut_asserteq(-EINVAL, adc_start_channel(dev, SANDBOX_ADC_CHANNELS));

	return 0;
}
DM_TEST(dm_test_adc_wrong_channel_selection, UT_TESTF_SCAN_FDT);

static int dm_test_adc_supply(struct unit_test_state *uts)
{
	struct udevice *supply;
	struct udevice *dev;
	int uV;

	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));

	/* Test Vss value - predefined 0 uV */
	ut_assertok(adc_vss_value(dev, &uV));
	ut_asserteq(SANDBOX_ADC_VSS_VALUE, uV);

	/* Test Vdd initial value - buck2 */
	ut_assertok(adc_vdd_value(dev, &uV));
	ut_asserteq(SANDBOX_BUCK2_INITIAL_EXPECTED_UV, uV);

	/* Change Vdd value - buck2 manual preset */
	ut_assertok(regulator_get_by_devname(SANDBOX_BUCK2_DEVNAME, &supply));
	ut_assertok(regulator_set_value(supply, SANDBOX_BUCK2_SET_UV));
	ut_asserteq(SANDBOX_BUCK2_SET_UV, regulator_get_value(supply));

	/* Update ADC plat and get new Vdd value */
	ut_assertok(adc_vdd_value(dev, &uV));
	ut_asserteq(SANDBOX_BUCK2_SET_UV, uV);

	/* Disable buck2 and test ADC supply enable function */
	ut_assertok(regulator_set_enable(supply, false));
	ut_asserteq(false, regulator_get_enable(supply));
	/* adc_start_channel() should enable the supply regulator */
	ut_assertok(adc_start_channel(dev, 0));
	ut_asserteq(true, regulator_get_enable(supply));

	return 0;
}
DM_TEST(dm_test_adc_supply, UT_TESTF_SCAN_FDT);

struct adc_channel adc_channel_test_data[] = {
	{ 0, SANDBOX_ADC_CHANNEL0_DATA },
	{ 1, SANDBOX_ADC_CHANNEL1_DATA },
	{ 2, SANDBOX_ADC_CHANNEL2_DATA },
	{ 3, SANDBOX_ADC_CHANNEL3_DATA },
};

static int dm_test_adc_single_channel_conversion(struct unit_test_state *uts)
{
	struct adc_channel *tdata = adc_channel_test_data;
	unsigned int i, data;
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));
	/* Test each ADC channel's value */
	for (i = 0; i < SANDBOX_ADC_CHANNELS; i++, tdata++) {
		ut_assertok(adc_start_channel(dev, tdata->id));
		ut_assertok(adc_channel_data(dev, tdata->id, &data));
		ut_asserteq(tdata->data, data);
	}

	return 0;
}
DM_TEST(dm_test_adc_single_channel_conversion, UT_TESTF_SCAN_FDT);

static int dm_test_adc_multi_channel_conversion(struct unit_test_state *uts)
{
	struct adc_channel channels[SANDBOX_ADC_CHANNELS];
	struct udevice *dev;
	struct adc_channel *tdata = adc_channel_test_data;
	unsigned int i, channel_mask;

	channel_mask = ADC_CHANNEL(0) | ADC_CHANNEL(1) |
		       ADC_CHANNEL(2) | ADC_CHANNEL(3);

	/* Start multi channel conversion */
	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));
	ut_assertok(adc_start_channels(dev, channel_mask));
	ut_assertok(adc_channels_data(dev, channel_mask, channels));

	/* Compare the expected and returned conversion data. */
	for (i = 0; i < SANDBOX_ADC_CHANNELS; i++, tdata++)
		ut_asserteq(tdata->data, channels[i].data);

	return 0;
}
DM_TEST(dm_test_adc_multi_channel_conversion, UT_TESTF_SCAN_FDT);

static int dm_test_adc_single_channel_shot(struct unit_test_state *uts)
{
	struct adc_channel *tdata = adc_channel_test_data;
	unsigned int i, data;

	for (i = 0; i < SANDBOX_ADC_CHANNELS; i++, tdata++) {
		/* Start single channel conversion */
		ut_assertok(adc_channel_single_shot("adc@0", tdata->id, &data));
		/* Compare the expected and returned conversion data. */
		ut_asserteq(tdata->data, data);
	}

	return 0;
}
DM_TEST(dm_test_adc_single_channel_shot, UT_TESTF_SCAN_FDT);

static int dm_test_adc_multi_channel_shot(struct unit_test_state *uts)
{
	struct adc_channel channels[SANDBOX_ADC_CHANNELS];
	struct adc_channel *tdata = adc_channel_test_data;
	unsigned int i, channel_mask;

	channel_mask = ADC_CHANNEL(0) | ADC_CHANNEL(1) |
		       ADC_CHANNEL(2) | ADC_CHANNEL(3);

	/* Start single call and multi channel conversion */
	ut_assertok(adc_channels_single_shot("adc@0", channel_mask, channels));

	/* Compare the expected and returned conversion data. */
	for (i = 0; i < SANDBOX_ADC_CHANNELS; i++, tdata++)
		ut_asserteq(tdata->data, channels[i].data);

	return 0;
}
DM_TEST(dm_test_adc_multi_channel_shot, UT_TESTF_SCAN_FDT);

static const int dm_test_adc_uV_data[SANDBOX_ADC_CHANNELS] = {
	((u64)SANDBOX_ADC_CHANNEL0_DATA * SANDBOX_BUCK2_INITIAL_EXPECTED_UV) /
		SANDBOX_ADC_DATA_MASK,
	((u64)SANDBOX_ADC_CHANNEL1_DATA * SANDBOX_BUCK2_INITIAL_EXPECTED_UV) /
		SANDBOX_ADC_DATA_MASK,
	((u64)SANDBOX_ADC_CHANNEL2_DATA * SANDBOX_BUCK2_INITIAL_EXPECTED_UV) /
		SANDBOX_ADC_DATA_MASK,
	((u64)SANDBOX_ADC_CHANNEL3_DATA * SANDBOX_BUCK2_INITIAL_EXPECTED_UV) /
		SANDBOX_ADC_DATA_MASK,
};

static int dm_test_adc_raw_to_uV(struct unit_test_state *uts)
{
	struct adc_channel *tdata = adc_channel_test_data;
	unsigned int i, data;
	struct udevice *dev;
	int uV;

	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));
	/* Test each ADC channel's value in microvolts */
	for (i = 0; i < SANDBOX_ADC_CHANNELS; i++, tdata++) {
		ut_assertok(adc_start_channel(dev, tdata->id));
		ut_assertok(adc_channel_data(dev, tdata->id, &data));
		ut_assertok(adc_raw_to_uV(dev, data, &uV));
		ut_asserteq(dm_test_adc_uV_data[i], uV);
	}

	return 0;
}
DM_TEST(dm_test_adc_raw_to_uV, UT_TESTF_SCAN_FDT);
