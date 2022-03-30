// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018, STMicroelectronics
 */

#include <common.h>
#include <serial.h>
#include <dm.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_serial(struct unit_test_state *uts)
{
	struct serial_device_info info_serial = {0};
	struct udevice *dev_serial;
	uint value_serial;

	ut_assertok(uclass_get_device_by_name(UCLASS_SERIAL, "serial",
					      &dev_serial));

	ut_assertok(serial_tstc());
	/*
	 * test with default config which is the only one supported by
	 * sandbox_serial driver
	 */
	ut_assertok(serial_setconfig(dev_serial, SERIAL_DEFAULT_CONFIG));
	ut_assertok(serial_getconfig(dev_serial, &value_serial));
	ut_assert(value_serial == SERIAL_DEFAULT_CONFIG);
	ut_assertok(serial_getinfo(dev_serial, &info_serial));
	ut_assert(info_serial.type == SERIAL_CHIP_UNKNOWN);
	ut_assert(info_serial.addr == SERIAL_DEFAULT_ADDRESS);
	/*
	 * test with a parameter which is NULL pointer
	 */
	ut_asserteq(-EINVAL, serial_getconfig(dev_serial, NULL));
	ut_asserteq(-EINVAL, serial_getinfo(dev_serial, NULL));
	/*
	 * test with a serial config which is not supported by
	 * sandbox_serial driver: test with wrong parity
	 */
	ut_asserteq(-ENOTSUPP,
		    serial_setconfig(dev_serial,
				     SERIAL_CONFIG(SERIAL_PAR_ODD,
						   SERIAL_8_BITS,
						   SERIAL_ONE_STOP)));
	/*
	 * test with a serial config which is not supported by
	 * sandbox_serial driver: test with wrong bits number
	 */
	ut_asserteq(-ENOTSUPP,
		    serial_setconfig(dev_serial,
				     SERIAL_CONFIG(SERIAL_PAR_NONE,
						   SERIAL_6_BITS,
						   SERIAL_ONE_STOP)));

	/*
	 * test with a serial config which is not supported by
	 * sandbox_serial driver: test with wrong stop bits number
	 */
	ut_asserteq(-ENOTSUPP,
		    serial_setconfig(dev_serial,
				     SERIAL_CONFIG(SERIAL_PAR_NONE,
						   SERIAL_8_BITS,
						   SERIAL_TWO_STOP)));

	return 0;
}

DM_TEST(dm_test_serial, DM_TESTF_SCAN_FDT);
