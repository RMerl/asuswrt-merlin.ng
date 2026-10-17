// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Google, Inc
 */

#include <common.h>
#include <fdtdec.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <acpi/acpi_device.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that sandbox GPIOs work correctly */
static int dm_test_gpio(struct unit_test_state *uts)
{
	unsigned int offset, gpio;
	struct dm_gpio_ops *ops;
	struct udevice *dev;
	struct gpio_desc *desc;
	const char *name;
	int offset_count;
	char buf[80];

	/*
	 * We expect to get 4 banks. One is anonymous (just numbered) and
	 * comes from plat. The other are named a (20 gpios),
	 * b (10 gpios) and c (10 gpios) and come from the device tree. See
	 * test/dm/test.dts.
	 */
	ut_assertok(gpio_lookup_name("b4", &dev, &offset, &gpio));
	ut_asserteq_str(dev->name, "extra-gpios");
	ut_asserteq(4, offset);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT + 20 + 4, gpio);

	name = gpio_get_bank_info(dev, &offset_count);
	ut_asserteq_str("b", name);
	ut_asserteq(10, offset_count);

	/* Get the operations for this device */
	ops = gpio_get_ops(dev);
	ut_assert(ops->get_function);

	/* Cannot get a value until it is reserved */
	ut_asserteq(-EBUSY, gpio_get_value(gpio + 1));
	/*
	 * Now some tests that use the 'sandbox' back door. All GPIOs
	 * should default to input, include b4 that we are using here.
	 */
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 0 [ ]", buf);

	/* Change it to an output */
	sandbox_gpio_set_direction(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 0 [ ]", buf);

	sandbox_gpio_set_value(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 1 [ ]", buf);

	ut_assertok(gpio_request(gpio, "testing"));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 1 [x] testing", buf);

	/* Change the value a bit */
	ut_asserteq(1, ops->get_value(dev, offset));
	ut_assertok(ops->set_value(dev, offset, 0));
	ut_asserteq(0, ops->get_value(dev, offset));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: output: 0 [x] testing", buf);
	ut_assertok(ops->set_value(dev, offset, 1));
	ut_asserteq(1, ops->get_value(dev, offset));

	/* Make it an open drain output, and reset it */
	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_flags(dev, offset));
	ut_assertok(ops->set_flags(dev, offset,
				   GPIOD_IS_OUT | GPIOD_OPEN_DRAIN));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN,
		    sandbox_gpio_get_flags(dev, offset));
	ut_assertok(ops->set_flags(dev, offset,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_flags(dev, offset));

	/* Make it an input */
	ut_assertok(ops->direction_input(dev, offset));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 1 [x] testing", buf);
	sandbox_gpio_set_value(dev, offset, 0);
	ut_asserteq(0, sandbox_gpio_get_value(dev, offset));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 0 [x] testing", buf);

	ut_assertok(gpio_free(gpio));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b4: input: 0 [ ]", buf);

	/* Check the 'a' bank also */
	ut_assertok(gpio_lookup_name("a15", &dev, &offset, &gpio));
	ut_asserteq_str(dev->name, "base-gpios");
	ut_asserteq(15, offset);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT + 15, gpio);

	name = gpio_get_bank_info(dev, &offset_count);
	ut_asserteq_str("a", name);
	ut_asserteq(20, offset_count);

	/* add gpio hog tests */
	ut_assertok(gpio_hog_lookup_name("hog_input_active_low", &desc));
	ut_asserteq(GPIOD_IS_IN | GPIOD_ACTIVE_LOW, desc->flags);
	ut_asserteq(10, desc->offset);
	ut_asserteq(1, dm_gpio_get_value(desc));
	ut_assertok(gpio_hog_lookup_name("hog_input_active_high", &desc));
	ut_asserteq(GPIOD_IS_IN, desc->flags);
	ut_asserteq(11, desc->offset);
	ut_asserteq(0, dm_gpio_get_value(desc));
	ut_assertok(gpio_hog_lookup_name("hog_output_low", &desc));
	ut_asserteq(GPIOD_IS_OUT, desc->flags);
	ut_asserteq(12, desc->offset);
	ut_asserteq(0, dm_gpio_get_value(desc));
	ut_assertok(dm_gpio_set_value(desc, 1));
	ut_asserteq(1, dm_gpio_get_value(desc));
	ut_assertok(gpio_hog_lookup_name("hog_output_high", &desc));
	ut_asserteq(GPIOD_IS_OUT, desc->flags);
	ut_asserteq(13, desc->offset);
	ut_asserteq(1, dm_gpio_get_value(desc));
	ut_assertok(dm_gpio_set_value(desc, 0));
	ut_asserteq(0, dm_gpio_get_value(desc));

	/* Check if lookup for labels work */
	ut_assertok(gpio_lookup_name("hog_input_active_low", &dev, &offset,
				     &gpio));
	ut_asserteq_str(dev->name, "base-gpios");
	ut_asserteq(10, offset);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT + 10, gpio);
	ut_assert(gpio_lookup_name("hog_not_exist", &dev, &offset,
				   &gpio));

	return 0;
}
DM_TEST(dm_test_gpio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that GPIO open-drain/open-source emulation works correctly */
static int dm_test_gpio_opendrain_opensource(struct unit_test_state *uts)
{
	struct gpio_desc desc_list[8];
	struct udevice *dev, *gpio_c;
	char buf[80];

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(uclass_get_device(UCLASS_GPIO, 3, &gpio_c));
	ut_asserteq_str("pinmux-gpios", gpio_c->name);

	ut_asserteq(8, gpio_request_list_by_name(dev, "test3-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0))

	ut_asserteq(true, !!device_active(gpio_c));
	ut_asserteq_ptr(gpio_c, desc_list[0].dev);
	ut_asserteq_ptr(gpio_c, desc_list[1].dev);
	ut_asserteq_ptr(gpio_c, desc_list[2].dev);
	ut_asserteq_ptr(gpio_c, desc_list[3].dev);
	ut_asserteq_ptr(gpio_c, desc_list[4].dev);
	ut_asserteq_ptr(gpio_c, desc_list[5].dev);
	ut_asserteq_ptr(gpio_c, desc_list[6].dev);
	ut_asserteq_ptr(gpio_c, desc_list[7].dev);

	/* GPIO 0 is (GPIO_OUT|GPIO_OPEN_DRAIN) */
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN,
		    sandbox_gpio_get_flags(gpio_c, 0));

	/* Set it as output high */
	ut_assertok(dm_gpio_set_value(&desc_list[0], 1));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN | GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_flags(gpio_c, 0));

	/* Set it as output low */
	ut_assertok(dm_gpio_set_value(&desc_list[0], 0));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN,
		    sandbox_gpio_get_flags(gpio_c, 0));

	/* GPIO 1 is (GPIO_OUT|GPIO_OPEN_SOURCE) */
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_SOURCE,
		    sandbox_gpio_get_flags(gpio_c, 1));

	/* Set it as output high, should become output high */
	ut_assertok(dm_gpio_set_value(&desc_list[1], 1));
	ut_assertok(gpio_get_status(gpio_c, 1, buf, sizeof(buf)));
	ut_asserteq_str("c1: output: 1 [x] a-test.test3-gpios1", buf);

	/* Set it as output low */
	ut_assertok(dm_gpio_set_value(&desc_list[1], 0));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_SOURCE,
		    sandbox_gpio_get_flags(gpio_c, 1));

	ut_assertok(gpio_get_status(gpio_c, 1, buf, sizeof(buf)));
	ut_asserteq_str("c1: output: 0 [x] a-test.test3-gpios1", buf);

	/*
	 * GPIO 6 is (GPIO_ACTIVE_LOW|GPIO_OUT|GPIO_OPEN_DRAIN). Looking at it
	 * directlt from the driver, we get GPIOD_IS_OUT_ACTIVE also, since it
	 * is active low
	 */
	ut_asserteq(GPIOD_ACTIVE_LOW | GPIOD_IS_OUT | GPIOD_OPEN_DRAIN |
		    GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_flags(gpio_c, 6));

	/* Set it as output high, should become output low */
	ut_assertok(dm_gpio_set_value(&desc_list[6], 1));
	ut_assertok(gpio_get_status(gpio_c, 6, buf, sizeof(buf)));
	ut_asserteq_str("c6: output: 0 [x] a-test.test3-gpios6", buf);

	/* Set it as output low */
	ut_assertok(dm_gpio_set_value(&desc_list[6], 0));
	ut_asserteq(GPIOD_ACTIVE_LOW | GPIOD_IS_OUT | GPIOD_OPEN_DRAIN |
		    GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_flags(gpio_c, 6));

	/* GPIO 7 is (GPIO_ACTIVE_LOW|GPIO_OUT|GPIO_OPEN_SOURCE) */
	ut_asserteq(GPIOD_ACTIVE_LOW | GPIOD_IS_OUT | GPIOD_OPEN_SOURCE |
		    GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_flags(gpio_c, 7));

	/* Set it as output high */
	ut_assertok(dm_gpio_set_value(&desc_list[7], 1));
	ut_asserteq(GPIOD_ACTIVE_LOW | GPIOD_IS_OUT | GPIOD_OPEN_SOURCE,
		    sandbox_gpio_get_flags(gpio_c, 7));

	/* Set it as output low, should become output high */
	ut_assertok(dm_gpio_set_value(&desc_list[7], 0));
	ut_assertok(gpio_get_status(gpio_c, 7, buf, sizeof(buf)));
	ut_asserteq_str("c7: output: 1 [x] a-test.test3-gpios7", buf);

	ut_assertok(gpio_free_list(dev, desc_list, 8));

	return 0;
}
DM_TEST(dm_test_gpio_opendrain_opensource,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that sandbox anonymous GPIOs work correctly */
static int dm_test_gpio_anon(struct unit_test_state *uts)
{
	unsigned int offset, gpio;
	struct udevice *dev;
	const char *name;
	int offset_count;

	/* And the anonymous bank */
	ut_assertok(gpio_lookup_name("14", &dev, &offset, &gpio));
	ut_asserteq_str(dev->name, "sandbox_gpio");
	ut_asserteq(14, offset);
	ut_asserteq(14, gpio);

	name = gpio_get_bank_info(dev, &offset_count);
	ut_asserteq_ptr(NULL, name);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT, offset_count);

	return 0;
}
DM_TEST(dm_test_gpio_anon, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that gpio_requestf() works as expected */
static int dm_test_gpio_requestf(struct unit_test_state *uts)
{
	unsigned int offset, gpio;
	struct udevice *dev;
	char buf[80];

	ut_assertok(gpio_lookup_name("b5", &dev, &offset, &gpio));
	ut_assertok(gpio_requestf(gpio, "testing %d %s", 1, "hi"));
	sandbox_gpio_set_direction(dev, offset, 1);
	sandbox_gpio_set_value(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b5: output: 1 [x] testing 1 hi", buf);

	return 0;
}
DM_TEST(dm_test_gpio_requestf, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that gpio_request() copies its string */
static int dm_test_gpio_copy(struct unit_test_state *uts)
{
	unsigned int offset, gpio;
	struct udevice *dev;
	char buf[80], name[10];

	ut_assertok(gpio_lookup_name("b6", &dev, &offset, &gpio));
	strcpy(name, "odd_name");
	ut_assertok(gpio_request(gpio, name));
	sandbox_gpio_set_direction(dev, offset, 1);
	sandbox_gpio_set_value(dev, offset, 1);
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b6: output: 1 [x] odd_name", buf);
	strcpy(name, "nothing");
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b6: output: 1 [x] odd_name", buf);

	return 0;
}
DM_TEST(dm_test_gpio_copy, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that we don't leak memory with GPIOs */
static int dm_test_gpio_leak(struct unit_test_state *uts)
{
	ut_assertok(dm_test_gpio(uts));
	ut_assertok(dm_test_gpio_anon(uts));
	ut_assertok(dm_test_gpio_requestf(uts));
	ut_assertok(dm_leak_check_end(uts));

	return 0;
}
DM_TEST(dm_test_gpio_leak, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that we can find GPIOs using phandles */
static int dm_test_gpio_phandles(struct unit_test_state *uts)
{
	struct gpio_desc desc, desc_list[8], desc_list2[8];
	struct udevice *dev, *gpio_a, *gpio_b;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(gpio_request_by_name(dev, "test-gpios", 1, &desc, 0));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio_a));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 2, &gpio_b));
	ut_asserteq_str("base-gpios", gpio_a->name);
	ut_asserteq(true, !!device_active(gpio_a));
	ut_asserteq_ptr(gpio_a, desc.dev);
	ut_asserteq(4, desc.offset);
	/* GPIOF_INPUT is the sandbox GPIO driver default */
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_a, 4, NULL));
	ut_assertok(dm_gpio_free(dev, &desc));

	ut_asserteq(-ENOENT, gpio_request_by_name(dev, "test-gpios", 3, &desc,
						  0));
	ut_asserteq_ptr(NULL, desc.dev);
	ut_asserteq(desc.offset, 0);
	ut_asserteq(-ENOENT, gpio_request_by_name(dev, "test-gpios", 5, &desc,
						  0));

	/* Last GPIO is ignored as it comes after <0> */
	ut_asserteq(3, gpio_request_list_by_name(dev, "test-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));
	ut_asserteq(-EBUSY, gpio_request_list_by_name(dev, "test-gpios",
						      desc_list2,
						      ARRAY_SIZE(desc_list2),
						      0));
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_a, 4, NULL));
	ut_assertok(gpio_free_list(dev, desc_list, 3));
	ut_asserteq(GPIOF_UNUSED, gpio_get_function(gpio_a, 4, NULL));
	ut_asserteq(3, gpio_request_list_by_name(dev,  "test-gpios", desc_list,
						 ARRAY_SIZE(desc_list),
						 GPIOD_IS_OUT |
						 GPIOD_IS_OUT_ACTIVE));
	ut_asserteq(GPIOF_OUTPUT, gpio_get_function(gpio_a, 4, NULL));
	ut_asserteq_ptr(gpio_a, desc_list[0].dev);
	ut_asserteq(1, desc_list[0].offset);
	ut_asserteq_ptr(gpio_a, desc_list[1].dev);
	ut_asserteq(4, desc_list[1].offset);
	ut_asserteq_ptr(gpio_b, desc_list[2].dev);
	ut_asserteq(5, desc_list[2].offset);
	ut_asserteq(1, dm_gpio_get_value(desc_list));
	ut_assertok(gpio_free_list(dev, desc_list, 3));

	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE,
		    sandbox_gpio_get_flags(gpio_a, 1));
	ut_asserteq(6, gpio_request_list_by_name(dev, "test2-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));

	/* This was set to output previously but flags reset to 0 = INPUT */
	ut_asserteq(0, sandbox_gpio_get_flags(gpio_a, 1));
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_a, 1, NULL));

	/* Active low should invert the input value */
	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_b, 6, NULL));
	ut_asserteq(1, dm_gpio_get_value(&desc_list[2]));

	ut_asserteq(GPIOF_INPUT, gpio_get_function(gpio_b, 7, NULL));
	ut_asserteq(GPIOF_OUTPUT, gpio_get_function(gpio_b, 8, NULL));
	ut_asserteq(0, dm_gpio_get_value(&desc_list[4]));
	ut_asserteq(GPIOF_OUTPUT, gpio_get_function(gpio_b, 9, NULL));
	ut_asserteq(1, dm_gpio_get_value(&desc_list[5]));

	return 0;
}
DM_TEST(dm_test_gpio_phandles, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Check the gpio pin configuration get from device tree information */
static int dm_test_gpio_get_dir_flags(struct unit_test_state *uts)
{
	struct gpio_desc desc_list[6];
	struct udevice *dev;
	ulong flags;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));

	ut_asserteq(6, gpio_request_list_by_name(dev, "test3-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));

	ut_assertok(dm_gpio_get_flags(&desc_list[0], &flags));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_DRAIN, flags);

	ut_assertok(dm_gpio_get_flags(&desc_list[1], &flags));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_OPEN_SOURCE, flags);

	ut_assertok(dm_gpio_get_flags(&desc_list[2], &flags));
	ut_asserteq(GPIOD_IS_OUT, flags);

	ut_assertok(dm_gpio_get_flags(&desc_list[3], &flags));
	ut_asserteq(GPIOD_IS_IN | GPIOD_PULL_UP, flags);

	ut_assertok(dm_gpio_get_flags(&desc_list[4], &flags));
	ut_asserteq(GPIOD_IS_IN | GPIOD_PULL_DOWN, flags);

	ut_assertok(dm_gpio_get_flags(&desc_list[5], &flags));
	ut_asserteq(GPIOD_IS_IN, flags);

	ut_assertok(gpio_free_list(dev, desc_list, 6));

	return 0;
}
DM_TEST(dm_test_gpio_get_dir_flags, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test of gpio_get_acpi() */
static int dm_test_gpio_get_acpi(struct unit_test_state *uts)
{
	struct acpi_gpio agpio;
	struct udevice *dev;
	struct gpio_desc desc;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test-gpios", 1, &desc, 0));

	/* See sb_gpio_get_acpi() */
	ut_assertok(gpio_get_acpi(&desc, &agpio));
	ut_asserteq(1, agpio.pin_count);
	ut_asserteq(4, agpio.pins[0]);
	ut_asserteq(ACPI_GPIO_TYPE_IO, agpio.type);
	ut_asserteq(ACPI_GPIO_PULL_UP, agpio.pull);
	ut_asserteq_str("\\_SB.PINC", agpio.resource);
	ut_asserteq(0, agpio.interrupt_debounce_timeout);
	ut_asserteq(0, agpio.irq.pin);
	ut_asserteq(1234, agpio.output_drive_strength);
	ut_asserteq(true, agpio.io_shared);
	ut_asserteq(ACPI_GPIO_IO_RESTRICT_INPUT, agpio.io_restrict);
	ut_asserteq(ACPI_GPIO_ACTIVE_HIGH, agpio.polarity);

	return 0;
}
DM_TEST(dm_test_gpio_get_acpi, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test of gpio_get_acpi() with an interrupt GPIO */
static int dm_test_gpio_get_acpi_irq(struct unit_test_state *uts)
{
	struct acpi_gpio agpio;
	struct udevice *dev;
	struct gpio_desc desc;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test2-gpios", 2, &desc, 0));

	/* See sb_gpio_get_acpi() */
	ut_assertok(gpio_get_acpi(&desc, &agpio));
	ut_asserteq(1, agpio.pin_count);
	ut_asserteq(6, agpio.pins[0]);
	ut_asserteq(ACPI_GPIO_TYPE_INTERRUPT, agpio.type);
	ut_asserteq(ACPI_GPIO_PULL_DOWN, agpio.pull);
	ut_asserteq_str("\\_SB.PINC", agpio.resource);
	ut_asserteq(4321, agpio.interrupt_debounce_timeout);
	ut_asserteq(6, agpio.irq.pin);
	ut_asserteq(ACPI_IRQ_ACTIVE_BOTH, agpio.irq.polarity);
	ut_asserteq(ACPI_IRQ_SHARED, agpio.irq.shared);
	ut_asserteq(true, agpio.irq.wake);
	ut_asserteq(0, agpio.output_drive_strength);
	ut_asserteq(false, agpio.io_shared);
	ut_asserteq(0, agpio.io_restrict);
	ut_asserteq(ACPI_GPIO_ACTIVE_LOW, agpio.polarity);

	return 0;
}
DM_TEST(dm_test_gpio_get_acpi_irq, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that we can get/release GPIOs using managed API */
static int dm_test_gpio_devm(struct unit_test_state *uts)
{
	static const u32 flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE;
	struct gpio_desc *desc1, *desc2, *desc3, *desc_err;
	struct udevice *dev;
	struct udevice *dev2;

	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "a-test",
					      &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "another-test",
					      &dev2));

	/* Get 3 GPIOs from 'a-test' dev */
	desc1 = devm_gpiod_get_index(dev, "test4", 0, flags);
	ut_assert(!IS_ERR(desc1));
	desc2 = devm_gpiod_get_index(dev, "test4", 1, flags);
	ut_assert(!IS_ERR(desc2));
	desc3 = devm_gpiod_get_index_optional(dev, "test5", 0, flags);
	ut_assert(!IS_ERR(desc3));
	ut_assert(desc3);

	/*
	 * Try get the same 3 GPIOs from 'a-test' and 'another-test' devices.
	 * check that it fails
	 */
	desc_err = devm_gpiod_get_index(dev, "test4", 0, flags);
	ut_asserteq(-EBUSY, PTR_ERR(desc_err));
	desc_err = devm_gpiod_get_index(dev2, "test4", 0, flags);
	ut_asserteq(-EBUSY, PTR_ERR(desc_err));
	desc_err = devm_gpiod_get_index(dev, "test4", 1, flags);
	ut_asserteq(-EBUSY, PTR_ERR(desc_err));
	desc_err = devm_gpiod_get_index(dev2, "test4", 1, flags);
	ut_asserteq(-EBUSY, PTR_ERR(desc_err));
	desc_err = devm_gpiod_get_index_optional(dev, "test5", 0, flags);
	ut_asserteq_ptr(NULL, desc_err);
	desc_err = devm_gpiod_get_index_optional(dev2, "test5", 0, flags);
	ut_asserteq_ptr(NULL, desc_err);

	/* Try get GPIOs outside of the list */
	desc_err = devm_gpiod_get_index(dev, "test4", 2, flags);
	ut_assert(IS_ERR(desc_err));
	desc_err = devm_gpiod_get_index_optional(dev, "test5", 1, flags);
	ut_asserteq_ptr(NULL, desc_err);

	/* Manipulate the GPIOs */
	ut_assertok(dm_gpio_set_value(desc1, 1));
	ut_asserteq(1, dm_gpio_get_value(desc1));
	ut_assertok(dm_gpio_set_value(desc1, 0));
	ut_asserteq(0, dm_gpio_get_value(desc1));

	ut_assertok(dm_gpio_set_value(desc2, 1));
	ut_asserteq(1, dm_gpio_get_value(desc2));
	ut_assertok(dm_gpio_set_value(desc2, 0));
	ut_asserteq(0, dm_gpio_get_value(desc2));

	ut_assertok(dm_gpio_set_value(desc3, 1));
	ut_asserteq(1, dm_gpio_get_value(desc3));
	ut_assertok(dm_gpio_set_value(desc3, 0));
	ut_asserteq(0, dm_gpio_get_value(desc3));

	/* Check that the GPIO cannot be owned by more than one device */
	desc_err = devm_gpiod_get_index(dev2, "test4", 0, flags);
	ut_asserteq(-EBUSY, PTR_ERR(desc_err));
	desc_err = devm_gpiod_get_index(dev2, "test4", 1, flags);
	ut_asserteq(-EBUSY, PTR_ERR(desc_err));
	desc_err = devm_gpiod_get_index_optional(dev2, "test5", 0, flags);
	ut_asserteq_ptr(NULL, desc_err);

	/*
	 * Release one GPIO and check that we can get it back using
	 * 'another-test' and then 'a-test'
	 */
	devm_gpiod_put(dev, desc2);
	desc2 = devm_gpiod_get_index(dev2, "test4", 1, flags);
	ut_assert(!IS_ERR(desc2));

	devm_gpiod_put(dev2, desc2);
	desc2 = devm_gpiod_get_index(dev, "test4", 1, flags);
	ut_assert(!IS_ERR(desc2));

	/* Release one GPIO before removing the 'a-test' dev. */
	devm_gpiod_put(dev, desc2);
	device_remove(dev, DM_REMOVE_NORMAL);

	/* All the GPIOs must have been freed. We should be able to claim
	 * them with the 'another-test' device.
	 */
	desc1 = devm_gpiod_get_index(dev2, "test4", 0, flags);
	ut_assert(!IS_ERR(desc1));
	desc2 = devm_gpiod_get_index(dev2, "test4", 1, flags);
	ut_assert(!IS_ERR(desc2));
	desc3 = devm_gpiod_get_index_optional(dev2, "test5", 0, flags);
	ut_assert(!IS_ERR(desc3));
	ut_assert(desc3);

	device_remove(dev2, DM_REMOVE_NORMAL);
	return 0;
}
DM_TEST(dm_test_gpio_devm, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_clrset_flags(struct unit_test_state *uts)
{
	struct gpio_desc desc;
	struct udevice *dev;
	ulong flags;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test-gpios", 1, &desc, 0));

	ut_assertok(dm_gpio_clrset_flags(&desc, GPIOD_MASK_DIR, GPIOD_IS_OUT));
	ut_assertok(dm_gpio_get_flags(&desc, &flags));
	ut_asserteq(GPIOD_IS_OUT, flags);
	ut_asserteq(GPIOD_IS_OUT, desc.flags);
	ut_asserteq(0, sandbox_gpio_get_value(desc.dev, desc.offset));

	ut_assertok(dm_gpio_clrset_flags(&desc, 0, GPIOD_IS_OUT_ACTIVE));
	ut_assertok(dm_gpio_get_flags(&desc, &flags));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE, flags);
	ut_asserteq(GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE, desc.flags);
	ut_asserteq(1, sandbox_gpio_get_value(desc.dev, desc.offset));
	ut_asserteq(1, dm_gpio_get_value(&desc));

	ut_assertok(dm_gpio_clrset_flags(&desc, GPIOD_MASK_DIR, GPIOD_IS_IN));
	ut_assertok(dm_gpio_get_flags(&desc, &flags));
	ut_asserteq(GPIOD_IS_IN, flags & GPIOD_MASK_DIR);
	ut_asserteq(GPIOD_IS_IN, desc.flags & GPIOD_MASK_DIR);

	ut_assertok(dm_gpio_clrset_flags(&desc, GPIOD_MASK_PULL,
					 GPIOD_PULL_UP));
	ut_assertok(dm_gpio_get_flags(&desc, &flags));
	ut_asserteq(GPIOD_IS_IN | GPIOD_PULL_UP, flags);
	ut_asserteq(GPIOD_IS_IN | GPIOD_PULL_UP, desc.flags);

	/* Check we cannot set both PULL_UP and PULL_DOWN */
	ut_asserteq(-EINVAL, dm_gpio_clrset_flags(&desc, 0, GPIOD_PULL_DOWN));

	return 0;
}
DM_TEST(dm_test_clrset_flags, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Check that an active-low GPIO works as expected */
static int dm_test_clrset_flags_invert(struct unit_test_state *uts)
{
	struct gpio_desc desc;
	struct udevice *dev;
	ulong flags;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(gpio_request_by_name(dev, "test-gpios", 1, &desc,
					 GPIOD_IS_OUT | GPIOD_ACTIVE_LOW));

	/*
	 * From this size we see it as 0 (active low), but the sandbox driver
	 * sees the pin value high
	 */
	ut_asserteq(0, dm_gpio_get_value(&desc));
	ut_asserteq(1, sandbox_gpio_get_value(desc.dev, desc.offset));

	ut_assertok(dm_gpio_set_value(&desc, 1));
	ut_asserteq(1, dm_gpio_get_value(&desc));
	ut_asserteq(0, sandbox_gpio_get_value(desc.dev, desc.offset));

	/* Do the same with dm_gpio_clrset_flags() */
	ut_assertok(dm_gpio_clrset_flags(&desc, GPIOD_IS_OUT_ACTIVE, 0));
	ut_asserteq(0, dm_gpio_get_value(&desc));
	ut_asserteq(1, sandbox_gpio_get_value(desc.dev, desc.offset));

	ut_assertok(dm_gpio_clrset_flags(&desc, 0, GPIOD_IS_OUT_ACTIVE));
	ut_asserteq(1, dm_gpio_get_value(&desc));
	ut_asserteq(0, sandbox_gpio_get_value(desc.dev, desc.offset));

	ut_assertok(dm_gpio_get_flags(&desc, &flags));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
		    flags);
	ut_asserteq(GPIOD_IS_OUT | GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE,
		    desc.flags);

	ut_assertok(dm_gpio_clrset_flags(&desc, GPIOD_IS_OUT_ACTIVE, 0));
	ut_assertok(dm_gpio_get_flags(&desc, &flags));
	ut_asserteq(GPIOD_IS_OUT | GPIOD_ACTIVE_LOW, flags);
	ut_asserteq(GPIOD_IS_OUT | GPIOD_ACTIVE_LOW, desc.flags);

	return 0;
}
DM_TEST(dm_test_clrset_flags_invert, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int set_gpios(struct unit_test_state *uts, struct gpio_desc *desc,
		     int count, uint value)
{
	int i;

	for (i = 0; i < count; i++) {
		const uint mask = 1 << i;

		ut_assertok(sandbox_gpio_set_value(desc[i].dev, desc[i].offset,
						   value & mask));
	}

	return 0;
}

/* Check that an active-low GPIO works as expected */
static int dm_test_gpio_get_values_as_int(struct unit_test_state *uts)
{
	const int gpio_count = 3;
	struct gpio_desc desc[gpio_count];
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_asserteq(3, gpio_request_list_by_name(dev, "test-gpios", desc,
						 gpio_count, GPIOD_IS_IN));
	ut_assertok(set_gpios(uts, desc, gpio_count, 0));
	ut_asserteq(0, dm_gpio_get_values_as_int(desc, gpio_count));

	ut_assertok(set_gpios(uts, desc, gpio_count, 5));
	ut_asserteq(5, dm_gpio_get_values_as_int(desc, gpio_count));

	ut_assertok(set_gpios(uts, desc, gpio_count, 7));
	ut_asserteq(7, dm_gpio_get_values_as_int(desc, gpio_count));

	return 0;
}
DM_TEST(dm_test_gpio_get_values_as_int,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Check that an active-low GPIO works as expected */
static int dm_test_gpio_get_values_as_int_base3(struct unit_test_state *uts)
{
	const int gpio_count = 3;
	struct gpio_desc desc[gpio_count];
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_asserteq(3, gpio_request_list_by_name(dev, "test-gpios", desc,
						 gpio_count, GPIOD_IS_IN));

	/*
	 * First test the sandbox GPIO driver works as expected. The external
	 * pull resistor should be stronger than the internal one.
	 */
	sandbox_gpio_set_flags(desc[0].dev, desc[0].offset,
			       GPIOD_IS_IN | GPIOD_EXT_PULL_UP | GPIOD_PULL_UP);
	ut_asserteq(1, dm_gpio_get_value(desc));

	sandbox_gpio_set_flags(desc[0].dev, desc[0].offset, GPIOD_IS_IN |
			       GPIOD_EXT_PULL_DOWN | GPIOD_PULL_UP);
	ut_asserteq(0, dm_gpio_get_value(desc));

	sandbox_gpio_set_flags(desc[0].dev, desc[0].offset,
			       GPIOD_IS_IN | GPIOD_PULL_UP);
	ut_asserteq(1, dm_gpio_get_value(desc));

	sandbox_gpio_set_flags(desc[0].dev, desc[0].offset, GPIOD_PULL_DOWN);
	ut_asserteq(0, dm_gpio_get_value(desc));

	/*
	 * Set up pins: pull-up (1), pull-down (0) and floating (2). This should
	 * result in digits 2 0 1, i.e. 2 * 9 + 1 * 3 = 19
	 */
	sandbox_gpio_set_flags(desc[0].dev, desc[0].offset, GPIOD_EXT_PULL_UP);
	sandbox_gpio_set_flags(desc[1].dev, desc[1].offset,
			       GPIOD_EXT_PULL_DOWN);
	sandbox_gpio_set_flags(desc[2].dev, desc[2].offset, 0);
	ut_asserteq(19, dm_gpio_get_values_as_int_base3(desc, gpio_count));

	/*
	 * Set up pins: floating (2), pull-up (1) and pull-down (0). This should
	 * result in digits 0 1 2, i.e. 1 * 3 + 2 = 5
	 */
	sandbox_gpio_set_flags(desc[0].dev, desc[0].offset, 0);
	sandbox_gpio_set_flags(desc[1].dev, desc[1].offset, GPIOD_EXT_PULL_UP);
	sandbox_gpio_set_flags(desc[2].dev, desc[2].offset,
			       GPIOD_EXT_PULL_DOWN);
	ut_asserteq(5, dm_gpio_get_values_as_int_base3(desc, gpio_count));

	return 0;
}
DM_TEST(dm_test_gpio_get_values_as_int_base3,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Check that gpio_get_status return the label of a GPIO configured as GPIOD_AF */
static int dm_test_gpio_function(struct unit_test_state *uts)
{
	struct gpio_desc desc;
	struct udevice *dev;
	ulong flags;
	unsigned int offset, gpio;
	char buf[80];

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	/* request gpio_b 5 */
	ut_assertok(gpio_request_by_name(dev, "test-gpios", 2, &desc, 0));
	/* update gpio_b 5 function to GPIO_AF */
	ut_assertok(dm_gpio_clrset_flags(&desc, GPIOD_IS_AF, GPIOD_IS_AF));
	ut_assertok(dm_gpio_get_flags(&desc, &flags));
	ut_asserteq(GPIOD_IS_AF, flags);
	/* check using gpio_get_status that label is displayed for a pin with GPIO_AF function */
	ut_assertok(gpio_lookup_name("b5", &dev, &offset, &gpio));
	ut_assertok(gpio_get_status(dev, offset, buf, sizeof(buf)));
	ut_asserteq_str("b5: func a-test.test-gpios2", buf);

	ut_assertok(dm_gpio_free(dev, &desc));

	return 0;
}
DM_TEST(dm_test_gpio_function,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
