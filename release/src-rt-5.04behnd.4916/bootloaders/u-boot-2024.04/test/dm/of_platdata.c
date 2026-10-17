// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dt-structs.h>
#include <irq.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>
#include <asm-generic/gpio.h>
#include <asm/global_data.h>

/* Test that we can find a device using of-platdata */
static int dm_test_of_plat_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_SERIAL, &dev));
	ut_asserteq_str("sandbox_serial", dev->name);

	return 0;
}
DM_TEST(dm_test_of_plat_base, UT_TESTF_SCAN_PDATA);

/* Test that we can read properties from a device */
static int dm_test_of_plat_props(struct unit_test_state *uts)
{
	struct dtd_sandbox_spl_test *plat;
	struct udevice *dev;
	int i;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "sandbox_spl_test",
					      &dev));

	plat = dev_get_plat(dev);
	ut_assert(plat->boolval);
	ut_asserteq(1, plat->intval);
	ut_asserteq(3, ARRAY_SIZE(plat->intarray));
	ut_asserteq(2, plat->intarray[0]);
	ut_asserteq(3, plat->intarray[1]);
	ut_asserteq(4, plat->intarray[2]);
	ut_asserteq(5, plat->byteval);
	ut_asserteq(1, ARRAY_SIZE(plat->maybe_empty_int));
	ut_asserteq(0, plat->maybe_empty_int[0]);
	ut_asserteq(3, ARRAY_SIZE(plat->bytearray));
	ut_asserteq(6, plat->bytearray[0]);
	ut_asserteq(0, plat->bytearray[1]);
	ut_asserteq(0, plat->bytearray[2]);
	ut_asserteq(9, ARRAY_SIZE(plat->longbytearray));
	for (i = 0; i < ARRAY_SIZE(plat->longbytearray); i++)
		ut_asserteq(9 + i, plat->longbytearray[i]);
	ut_asserteq_str("message", plat->stringval);
	ut_asserteq(3, ARRAY_SIZE(plat->stringarray));
	ut_asserteq_str("multi-word", plat->stringarray[0]);
	ut_asserteq_str("message", plat->stringarray[1]);
	ut_asserteq_str("", plat->stringarray[2]);

	ut_assertok(uclass_next_device_err(&dev));
	plat = dev_get_plat(dev);
	ut_assert(!plat->boolval);
	ut_asserteq(3, plat->intval);
	ut_asserteq(5, plat->intarray[0]);
	ut_asserteq(0, plat->intarray[1]);
	ut_asserteq(0, plat->intarray[2]);
	ut_asserteq(8, plat->byteval);
	ut_asserteq(3, ARRAY_SIZE(plat->bytearray));
	ut_asserteq(1, plat->bytearray[0]);
	ut_asserteq(0x23, plat->bytearray[1]);
	ut_asserteq(0x34, plat->bytearray[2]);
	for (i = 0; i < ARRAY_SIZE(plat->longbytearray); i++)
		ut_asserteq(i < 4 ? 9 + i : 0, plat->longbytearray[i]);
	ut_asserteq_str("message2", plat->stringval);
	ut_asserteq_str("another", plat->stringarray[0]);
	ut_asserteq_str("multi-word", plat->stringarray[1]);
	ut_asserteq_str("message", plat->stringarray[2]);

	ut_assertok(uclass_next_device_err(&dev));
	plat = dev_get_plat(dev);
	ut_assert(!plat->boolval);
	ut_asserteq_str("one", plat->stringarray[0]);
	ut_asserteq_str("", plat->stringarray[1]);
	ut_asserteq_str("", plat->stringarray[2]);
	ut_asserteq(1, plat->maybe_empty_int[0]);

	ut_assertok(uclass_next_device_err(&dev));
	plat = dev_get_plat(dev);
	ut_assert(!plat->boolval);
	ut_asserteq_str("spl", plat->stringarray[0]);

	ut_asserteq(-ENODEV, uclass_next_device_err(&dev));

	return 0;
}
DM_TEST(dm_test_of_plat_props, UT_TESTF_SCAN_PDATA);

/*
 * find_driver_info - recursively find the driver_info for a device
 *
 * This sets found[idx] to true when it finds the driver_info record for a
 * device, where idx is the index in the driver_info linker list.
 *
 * @uts: Test state
 * @parent: Parent to search
 * @found: bool array to update
 * Return: 0 if OK, non-zero on error
 */
static int find_driver_info(struct unit_test_state *uts, struct udevice *parent,
			    bool found[])
{
	struct udevice *dev;

	/* If not the root device, find the entry that caused it to be bound */
	if (parent->parent) {
		const int n_ents =
			ll_entry_count(struct driver_info, driver_info);
		int idx = -1;
		int i;

		for (i = 0; i < n_ents; i++) {
			const struct driver_rt *drt = gd_dm_driver_rt() + i;

			if (drt->dev == parent) {
				idx = i;
				found[idx] = true;
				break;
			}
		}

		ut_assert(idx != -1);
	}

	device_foreach_child(dev, parent) {
		int ret;

		ret = find_driver_info(uts, dev, found);
		if (ret < 0)
			return ret;
	}

	return 0;
}

/* Check that every device is recorded in its driver_info struct */
static int dm_test_of_plat_dev(struct unit_test_state *uts)
{
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	bool found[n_ents];
	uint i;

	/* Skip this test if there is no platform data */
	if (!CONFIG_IS_ENABLED(OF_PLATDATA_DRIVER_RT))
		return -EAGAIN;

	/* Record the indexes that are found */
	memset(found, '\0', sizeof(found));
	ut_assertok(find_driver_info(uts, gd->dm_root, found));

	/* Make sure that the driver entries without devices have no ->dev */
	for (i = 0; i < n_ents; i++) {
		const struct driver_rt *drt = gd_dm_driver_rt() + i;
		struct udevice *dev;

		if (found[i]) {
			/* Make sure we can find it */
			ut_assertnonnull(drt->dev);
			ut_assertok(device_get_by_ofplat_idx(i, &dev));
			ut_asserteq_ptr(dev, drt->dev);
		} else {
			ut_assertnull(drt->dev);
			ut_asserteq(-ENOENT, device_get_by_ofplat_idx(i, &dev));
		}
	}

	return 0;
}
DM_TEST(dm_test_of_plat_dev, UT_TESTF_SCAN_PDATA);

/* Test handling of phandles that point to other devices */
static int dm_test_of_plat_phandle(struct unit_test_state *uts)
{
	struct dtd_sandbox_clk_test *plat;
	struct udevice *dev, *clk;

	ut_assertok(uclass_first_device_err(UCLASS_MISC, &dev));
	ut_asserteq_str("sandbox_clk_test", dev->name);
	plat = dev_get_plat(dev);

	ut_assertok(device_get_by_ofplat_idx(plat->clocks[0].idx, &clk));
	ut_asserteq_str("sandbox_fixed_clock", clk->name);

	ut_assertok(device_get_by_ofplat_idx(plat->clocks[1].idx, &clk));
	ut_asserteq_str("sandbox_clk", clk->name);
	ut_asserteq(1, plat->clocks[1].arg[0]);

	ut_assertok(device_get_by_ofplat_idx(plat->clocks[2].idx, &clk));
	ut_asserteq_str("sandbox_clk", clk->name);
	ut_asserteq(0, plat->clocks[2].arg[0]);

	ut_assertok(device_get_by_ofplat_idx(plat->clocks[3].idx, &clk));
	ut_asserteq_str("sandbox_clk", clk->name);
	ut_asserteq(3, plat->clocks[3].arg[0]);

	ut_assertok(device_get_by_ofplat_idx(plat->clocks[4].idx, &clk));
	ut_asserteq_str("sandbox_clk", clk->name);
	ut_asserteq(2, plat->clocks[4].arg[0]);

	return 0;
}
DM_TEST(dm_test_of_plat_phandle, UT_TESTF_SCAN_PDATA);

#if CONFIG_IS_ENABLED(OF_PLATDATA_PARENT)
/* Test that device parents are correctly set up */
static int dm_test_of_plat_parent(struct unit_test_state *uts)
{
	struct udevice *rtc, *i2c;

	ut_assertok(uclass_first_device_err(UCLASS_RTC, &rtc));
	ut_assertok(uclass_first_device_err(UCLASS_I2C, &i2c));
	ut_asserteq_ptr(i2c, dev_get_parent(rtc));

	return 0;
}
DM_TEST(dm_test_of_plat_parent, UT_TESTF_SCAN_PDATA);
#endif

/* Test clocks with of-platdata */
static int dm_test_of_plat_clk(struct unit_test_state *uts)
{
	struct dtd_sandbox_clk_test *plat;
	struct udevice *dev;
	struct clk clk;

	ut_assertok(uclass_first_device_err(UCLASS_MISC, &dev));
	ut_asserteq_str("sandbox_clk_test", dev->name);
	plat = dev_get_plat(dev);

	ut_assertok(clk_get_by_phandle(dev, &plat->clocks[0], &clk));
	ut_asserteq_str("sandbox_fixed_clock", clk.dev->name);

	return 0;
}
DM_TEST(dm_test_of_plat_clk, UT_TESTF_SCAN_PDATA);

/* Test irqs with of-platdata */
static int dm_test_of_plat_irq(struct unit_test_state *uts)
{
	struct dtd_sandbox_irq_test *plat;
	struct udevice *dev;
	struct irq irq;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "sandbox_irq_test",
					      &dev));
	plat = dev_get_plat(dev);

	ut_assertok(irq_get_by_phandle(dev, &plat->interrupts_extended[0],
				       &irq));
	ut_asserteq_str("sandbox_irq", irq.dev->name);

	return 0;
}
DM_TEST(dm_test_of_plat_irq, UT_TESTF_SCAN_PDATA);

/* Test GPIOs with of-platdata */
static int dm_test_of_plat_gpio(struct unit_test_state *uts)
{
	struct dtd_sandbox_gpio_test *plat;
	struct udevice *dev;
	struct gpio_desc desc;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "sandbox_gpio_test",
					      &dev));
	plat = dev_get_plat(dev);

	ut_assertok(gpio_request_by_phandle(dev, &plat->test_gpios[0], &desc,
					    GPIOD_IS_OUT));
	ut_asserteq_str("sandbox_gpio", desc.dev->name);

	return 0;
}
DM_TEST(dm_test_of_plat_gpio, UT_TESTF_SCAN_PDATA);
