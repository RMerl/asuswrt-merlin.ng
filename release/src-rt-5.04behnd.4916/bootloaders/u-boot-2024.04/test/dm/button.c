// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Based on led.c
 */

#include <common.h>
#include <dm.h>
#include <adc.h>
#include <button.h>
#include <power/regulator.h>
#include <power/sandbox_pmic.h>
#include <asm/gpio.h>
#include <dm/test.h>
#include <dt-bindings/input/input.h>
#include <test/ut.h>

/* Base test of the button uclass */
static int dm_test_button_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Get the top-level gpio buttons device */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 0, &dev));
	/* Get the 2 gpio buttons */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 2, &dev));

	/* Get the top-level adc buttons device */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 3, &dev));
	/* Get the 3 adc buttons */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 4, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 5, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 6, &dev));

	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_BUTTON, 7, &dev));

	return 0;
}
DM_TEST(dm_test_button_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test of the button uclass using the button_gpio driver */
static int dm_test_button_gpio(struct unit_test_state *uts)
{
	const int offset = 3;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate a BUTTON. BUTTON 1 is connected to GPIO
	 * bank gpio_a, offset 3.
	 */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));

	ut_asserteq(0, sandbox_gpio_set_value(gpio, offset, 0));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(BUTTON_OFF, button_get_state(dev));

	ut_asserteq(0, sandbox_gpio_set_value(gpio, offset, 1));
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(BUTTON_ON, button_get_state(dev));

	return 0;
}
DM_TEST(dm_test_button_gpio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test obtaining a BUTTON by label */
static int dm_test_button_label(struct unit_test_state *uts)
{
	struct udevice *dev, *cmp;

	ut_assertok(button_get_by_label("button1", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_assertok(button_get_by_label("button2", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 2, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_asserteq(-ENODEV, button_get_by_label("nobutton", &dev));

	return 0;
}
DM_TEST(dm_test_button_label, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test button has linux,code */
static int dm_test_button_linux_code(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_asserteq(BTN_1, button_get_code(dev));

	return 0;
}
DM_TEST(dm_test_button_linux_code, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test adc-keys driver */
static int dm_test_button_keys_adc(struct unit_test_state *uts)
{
	struct udevice *supply;
	struct udevice *dev;
	int uV;

	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));

	ut_assertok(regulator_get_by_devname(SANDBOX_BUCK2_DEVNAME, &supply));
	ut_assertok(regulator_set_value(supply, SANDBOX_BUCK2_SET_UV));
	ut_asserteq(SANDBOX_BUCK2_SET_UV, regulator_get_value(supply));
	/* Update ADC plat and get new Vdd value */
	ut_assertok(adc_vdd_value(dev, &uV));
	ut_asserteq(SANDBOX_BUCK2_SET_UV, uV);

	/*
	 * sandbox-adc returns constant value on channel 3, is used by adc-keys:
	 * SANDBOX_ADC_CHANNEL3_DATA * SANDBOX_BUCK2_SET_UV / SANDBOX_ADC_DATA_MASK =
	 * 0x3000 * 3300000 / 0xffff = 618759uV
	 * This means that button3 and button4 are released and button5
	 * is pressed.
	 */
	ut_assertok(button_get_by_label("button3", &dev));
	ut_asserteq(BUTTON_OFF, button_get_state(dev));
	ut_assertok(button_get_by_label("button4", &dev));
	ut_asserteq(BUTTON_OFF, button_get_state(dev));
	ut_assertok(button_get_by_label("button5", &dev));
	ut_asserteq(BUTTON_ON, button_get_state(dev));

	return 0;
}
DM_TEST(dm_test_button_keys_adc, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
