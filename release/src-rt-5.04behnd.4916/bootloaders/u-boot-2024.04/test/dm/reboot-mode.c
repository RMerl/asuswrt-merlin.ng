// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2018 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <reboot-mode/reboot-mode.h>
#include <env.h>
#include <log.h>
#include <asm/gpio.h>
#include <asm/rtc.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>
#include <rtc.h>
#include <linux/byteorder/generic.h>

static int dm_test_reboot_mode_gpio(struct unit_test_state *uts)
{
	struct udevice *gpio_dev;
	struct udevice *rm_dev;
	int gpio0_offset = 0;
	int gpio1_offset = 1;

	uclass_get_device_by_name(UCLASS_GPIO, "pinmux-gpios", &gpio_dev);

	/* Prepare the GPIOs for "download" mode */
	sandbox_gpio_set_direction(gpio_dev, gpio0_offset, 0);
	sandbox_gpio_set_direction(gpio_dev, gpio1_offset, 0);
	sandbox_gpio_set_value(gpio_dev, gpio0_offset, 1);
	sandbox_gpio_set_value(gpio_dev, gpio1_offset, 1);

	ut_assertok(uclass_get_device_by_name(UCLASS_REBOOT_MODE,
					      "reboot-mode0", &rm_dev));
	ut_assertok(dm_reboot_mode_update(rm_dev));

	ut_asserteq_str("download", env_get("bootstatus"));

	return 0;
}

DM_TEST(dm_test_reboot_mode_gpio,
	UT_TESTF_PROBE_TEST | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

static int dm_test_reboot_mode_rtc(struct unit_test_state *uts)
{
	struct udevice *rtc_dev;
	struct udevice *rm_dev;
	u32 read_val;
	u32 test_magic_val = cpu_to_be32(0x21969147);

	uclass_get_device_by_name(UCLASS_RTC, "rtc@43",
				  &rtc_dev);
	dm_rtc_write(rtc_dev, REG_AUX0, (u8 *)&test_magic_val, 4);

	ut_assertok(uclass_get_device_by_name(UCLASS_REBOOT_MODE,
					      "reboot-mode@14", &rm_dev));
	ut_assertok(dm_reboot_mode_update(rm_dev));

	ut_asserteq_str("test", env_get("bootstatus"));

	dm_rtc_read(rtc_dev, REG_AUX0, (u8 *)&read_val, 4);
	ut_asserteq(read_val, 0);

	return 0;
}

DM_TEST(dm_test_reboot_mode_rtc,
	UT_TESTF_PROBE_TEST | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);
