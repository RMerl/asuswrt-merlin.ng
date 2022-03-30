// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <power/pmic.h>
#include <spmi/spmi.h>
#include <asm/gpio.h>
#include <test/ut.h>

/* Test if bus childs got probed propperly*/
static int dm_test_spmi_probe(struct unit_test_state *uts)
{
	const char *name = "spmi@0";
	struct udevice *bus, *dev;

	ut_assertok(uclass_get_device(UCLASS_SPMI, 0, &bus));

	/* Check bus name */
	ut_asserteq_str(name, bus->name);

	/* Check that it has some devices */
	ut_asserteq(device_has_children(bus), true);

	ut_assertok(device_find_first_child(bus, &dev));

	/* There should be at least one child */
	ut_assertnonnull(dev);

	/* Check that only PMICs are connected to the bus */
	while (dev) {
		ut_asserteq(device_get_uclass_id(dev), UCLASS_PMIC);
		device_find_next_child(&dev);
	}

	return 0;
}
DM_TEST(dm_test_spmi_probe, DM_TESTF_SCAN_FDT);

/* Test if it's possible to read bus directly and indirectly */
static int dm_test_spmi_access(struct unit_test_state *uts)
{
	const char *pmic_name = "pm8916@0";
	struct udevice *bus, *pmic;

	ut_assertok(uclass_get_device(UCLASS_SPMI, 0, &bus));

	ut_assertok(device_get_child(bus, 0, &pmic));

	/* Sanity check if it's proper PMIC */
	ut_asserteq_str(pmic_name, pmic->name);

	/* Read PMIC ID reg using SPMI bus - it assumes it has slaveID == 0*/
	ut_asserteq(spmi_reg_read(bus, 0, 0xC0, 0x4), 0x10);
	ut_asserteq(spmi_reg_read(bus, 0, 0xC0, 0x5), 0x5);

	/* Read ID reg via pmic interface */
	ut_asserteq(pmic_reg_read(pmic, 0xC004), 0x10);
	ut_asserteq(pmic_reg_read(pmic, 0xC005), 0x5);

	return 0;
}
DM_TEST(dm_test_spmi_access, DM_TESTF_SCAN_FDT);


/* Test if it's possible to access GPIO that should be in pmic */
static int dm_test_spmi_access_peripheral(struct unit_test_state *uts)
{
	struct udevice *dev;
	unsigned int offset, gpio;
	const char *name;
	int offset_count;

	/* Get second pin of PMIC GPIO */
	ut_assertok(gpio_lookup_name("spmi1", &dev, &offset, &gpio));

	/* Check if PMIC is parent */
	ut_asserteq(device_get_uclass_id(dev->parent), UCLASS_PMIC);

	/* This should be second gpio */
	ut_asserteq(1, offset);

	name = gpio_get_bank_info(dev, &offset_count);

	/* Check bank name */
	ut_asserteq_str("spmi", name);
	/* Check pin count */
	ut_asserteq(4, offset_count);

	ut_assertok(gpio_request(gpio, "testing"));

	/* Try to set/clear gpio */
	ut_assertok(gpio_direction_output(gpio, 0));
	ut_asserteq(gpio_get_value(gpio), 0);
	ut_assertok(gpio_direction_output(gpio, 1));
	ut_asserteq(gpio_get_value(gpio), 1);
	ut_assertok(gpio_direction_input(gpio));
	ut_asserteq(gpio_get_value(gpio), 1);

	ut_assertok(gpio_free(gpio));

	return 0;
}
DM_TEST(dm_test_spmi_access_peripheral, DM_TESTF_SCAN_FDT);
