// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Google, Inc
 */

#include <common.h>
#include <fdtdec.h>
#include <dm.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <asm/gpio.h>
#include <test/ut.h>

/* Test that sandbox GPIOs work correctly */
static int dm_test_gpio(struct unit_test_state *uts)
{
	unsigned int offset, gpio;
	struct dm_gpio_ops *ops;
	struct udevice *dev;
	const char *name;
	int offset_count;
	char buf[80];

	/*
	 * We expect to get 3 banks. One is anonymous (just numbered) and
	 * comes from platdata. The other two are named a (20 gpios)
	 * and b (10 gpios) and come from the device tree. See
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
	ut_asserteq(0, sandbox_gpio_get_open_drain(dev, offset));
	ut_assertok(ops->set_open_drain(dev, offset, 1));
	ut_asserteq(1, sandbox_gpio_get_open_drain(dev, offset));
	ut_assertok(ops->set_open_drain(dev, offset, 0));
	ut_asserteq(0, sandbox_gpio_get_open_drain(dev, offset));

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

	return 0;
}
DM_TEST(dm_test_gpio, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that sandbox anonymous GPIOs work correctly */
static int dm_test_gpio_anon(struct unit_test_state *uts)
{
	unsigned int offset, gpio;
	struct udevice *dev;
	const char *name;
	int offset_count;

	/* And the anonymous bank */
	ut_assertok(gpio_lookup_name("14", &dev, &offset, &gpio));
	ut_asserteq_str(dev->name, "gpio_sandbox");
	ut_asserteq(14, offset);
	ut_asserteq(14, gpio);

	name = gpio_get_bank_info(dev, &offset_count);
	ut_asserteq_ptr(NULL, name);
	ut_asserteq(CONFIG_SANDBOX_GPIO_COUNT, offset_count);

	return 0;
}
DM_TEST(dm_test_gpio_anon, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_gpio_requestf, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_gpio_copy, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we don't leak memory with GPIOs */
static int dm_test_gpio_leak(struct unit_test_state *uts)
{
	ut_assertok(dm_test_gpio(uts));
	ut_assertok(dm_test_gpio_anon(uts));
	ut_assertok(dm_test_gpio_requestf(uts));
	ut_assertok(dm_leak_check_end(uts));

	return 0;
}
DM_TEST(dm_test_gpio_leak, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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

	/* Last GPIO is ignord as it comes after <0> */
	ut_asserteq(3, gpio_request_list_by_name(dev, "test-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));
	ut_asserteq(-EBUSY, gpio_request_list_by_name(dev, "test-gpios",
						      desc_list2,
						      ARRAY_SIZE(desc_list2),
						      0));
	ut_assertok(gpio_free_list(dev, desc_list, 3));
	ut_asserteq(3, gpio_request_list_by_name(dev,  "test-gpios", desc_list,
						 ARRAY_SIZE(desc_list),
						 GPIOD_IS_OUT |
						 GPIOD_IS_OUT_ACTIVE));
	ut_asserteq_ptr(gpio_a, desc_list[0].dev);
	ut_asserteq(1, desc_list[0].offset);
	ut_asserteq_ptr(gpio_a, desc_list[1].dev);
	ut_asserteq(4, desc_list[1].offset);
	ut_asserteq_ptr(gpio_b, desc_list[2].dev);
	ut_asserteq(5, desc_list[2].offset);
	ut_asserteq(1, dm_gpio_get_value(desc_list));
	ut_assertok(gpio_free_list(dev, desc_list, 3));

	ut_asserteq(6, gpio_request_list_by_name(dev, "test2-gpios", desc_list,
						 ARRAY_SIZE(desc_list), 0));
	/* This was set to output previously, so still will be */
	ut_asserteq(GPIOF_OUTPUT, gpio_get_function(gpio_a, 1, NULL));

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
DM_TEST(dm_test_gpio_phandles, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
