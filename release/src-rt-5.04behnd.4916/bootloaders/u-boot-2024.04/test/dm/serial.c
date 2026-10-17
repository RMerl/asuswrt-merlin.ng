// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018, STMicroelectronics
 */

#include <common.h>
#include <log.h>
#include <serial.h>
#include <dm.h>
#include <asm/serial.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static const char test_message[] =
	"This is a test message\n"
	"consisting of multiple lines\n";

static int dm_test_serial(struct unit_test_state *uts)
{
	int i;
	struct serial_device_info info_serial = {0};
	struct udevice *dev_serial;
	size_t start, putc_written;

	uint value_serial;

	ut_assertok(uclass_get_device_by_name(UCLASS_SERIAL, "serial",
					      &dev_serial));

	ut_assertok(serial_tstc());
	ut_asserteq(115200, fetch_baud_from_dtb());
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
	ut_assert(info_serial.clock == SERIAL_DEFAULT_CLOCK);
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

	/* Verify that putc and puts print the same number of characters */
	sandbox_serial_endisable(false);
	start = sandbox_serial_written();
	for (i = 0; i < sizeof(test_message) - 1; i++)
		serial_putc(test_message[i]);
	putc_written = sandbox_serial_written();
	serial_puts(test_message);
	sandbox_serial_endisable(true);
	ut_asserteq(putc_written - start,
		    sandbox_serial_written() - putc_written);

	return 0;
}

DM_TEST(dm_test_serial, UT_TESTF_SCAN_FDT);
