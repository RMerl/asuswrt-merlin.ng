// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Google, Inc
 *
 * Note: Test coverage does not include 10-bit addressing
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <test/ut.h>

static const int busnum;
static const int chip = 0x2c;

/* Test that we can find buses and chips */
static int dm_test_i2c_find(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	const int no_chip = 0x10;

	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_I2C, busnum,
						       false, &bus));

	/*
	 * The post_bind() method will bind devices to chip selects. Check
	 * this then remove the emulation and the slave device.
	 */
	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));
	ut_assertok(dm_i2c_probe(bus, chip, 0, &dev));
	ut_asserteq(-ENOENT, dm_i2c_probe(bus, no_chip, 0, &dev));
	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_I2C, 1, &bus));

	return 0;
}
DM_TEST(dm_test_i2c_find, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_i2c_read_write(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	uint8_t buf[5];

	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));
	ut_assertok(i2c_get_chip(bus, chip, 1, &dev));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\0\0\0\0", sizeof(buf)));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\0AB\0", sizeof(buf)));

	return 0;
}
DM_TEST(dm_test_i2c_read_write, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_i2c_speed(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	uint8_t buf[5];

	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));

	/* Use test mode so we create the required errors for invalid speeds */
	sandbox_i2c_set_test_mode(bus, true);
	ut_assertok(i2c_get_chip(bus, chip, 1, &dev));
	ut_assertok(dm_i2c_set_bus_speed(bus, 100000));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(dm_i2c_set_bus_speed(bus, 400000));
	ut_asserteq(400000, dm_i2c_get_bus_speed(bus));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq(-EINVAL, dm_i2c_write(dev, 0, buf, 5));
	sandbox_i2c_set_test_mode(bus, false);

	return 0;
}
DM_TEST(dm_test_i2c_speed, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_i2c_offset_len(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	uint8_t buf[5];

	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));
	ut_assertok(i2c_get_chip(bus, chip, 1, &dev));
	ut_assertok(i2c_set_chip_offset_len(dev, 1));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));

	/* This is not supported by the uclass */
	ut_asserteq(-EINVAL, i2c_set_chip_offset_len(dev, 5));

	return 0;
}
DM_TEST(dm_test_i2c_offset_len, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_i2c_probe_empty(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;

	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));

	/* Use test mode so that this chip address will always probe */
	sandbox_i2c_set_test_mode(bus, true);
	ut_assertok(dm_i2c_probe(bus, SANDBOX_I2C_TEST_ADDR, 0, &dev));
	sandbox_i2c_set_test_mode(bus, false);

	return 0;
}
DM_TEST(dm_test_i2c_probe_empty, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_i2c_bytewise(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	struct udevice *eeprom;
	uint8_t buf[5];

	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));
	ut_assertok(i2c_get_chip(bus, chip, 1, &dev));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\0\0\0\0", sizeof(buf)));

	/* Tell the EEPROM to only read/write one register at a time */
	ut_assertok(uclass_first_device(UCLASS_I2C_EMUL, &eeprom));
	ut_assertnonnull(eeprom);
	sandbox_i2c_eeprom_set_test_mode(eeprom, SIE_TEST_MODE_SINGLE_BYTE);

	/* Now we only get the first byte - the rest will be 0xff */
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\xff\xff\xff\xff", sizeof(buf)));

	/* If we do a separate transaction for each byte, it works */
	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\0\0\0\0", sizeof(buf)));

	/* This will only write A */
	ut_assertok(i2c_set_chip_flags(dev, 0));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\xff\xff\xff\xff", sizeof(buf)));

	/* Check that the B was ignored */
	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\0A\0\0\0", sizeof(buf)));

	/* Now write it again with the new flags, it should work */
	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_WR_ADDRESS));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\xff\xff\xff\xff", sizeof(buf)));

	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_WR_ADDRESS |
						DM_I2C_CHIP_RD_ADDRESS));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "\0\0AB\0\0", sizeof(buf)));

	/* Restore defaults */
	sandbox_i2c_eeprom_set_test_mode(eeprom, SIE_TEST_MODE_NONE);
	ut_assertok(i2c_set_chip_flags(dev, 0));

	return 0;
}
DM_TEST(dm_test_i2c_bytewise, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int dm_test_i2c_offset(struct unit_test_state *uts)
{
	struct udevice *eeprom;
	struct udevice *dev;
	uint8_t buf[5];

	ut_assertok(i2c_get_chip_for_busnum(busnum, chip, 1, &dev));

	/* Do a transfer so we can find the emulator */
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(uclass_first_device(UCLASS_I2C_EMUL, &eeprom));

	/* Offset length 0 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 0);
	ut_assertok(i2c_set_chip_offset_len(dev, 0));
	ut_assertok(dm_i2c_write(dev, 10 /* ignored */, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "AB\0\0\0\0", sizeof(buf)));

	/* Offset length 1 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 1);
	ut_assertok(i2c_set_chip_offset_len(dev, 1));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(memcmp(buf, "ABAB\0", sizeof(buf)));

	/* Offset length 2 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 2);
	ut_assertok(i2c_set_chip_offset_len(dev, 2));
	ut_assertok(dm_i2c_write(dev, 0x210, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x210, buf, 5));
	ut_assertok(memcmp(buf, "AB\0\0\0", sizeof(buf)));

	/* Offset length 3 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 2);
	ut_assertok(i2c_set_chip_offset_len(dev, 2));
	ut_assertok(dm_i2c_write(dev, 0x410, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x410, buf, 5));
	ut_assertok(memcmp(buf, "AB\0\0\0", sizeof(buf)));

	/* Offset length 4 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 2);
	ut_assertok(i2c_set_chip_offset_len(dev, 2));
	ut_assertok(dm_i2c_write(dev, 0x420, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x420, buf, 5));
	ut_assertok(memcmp(buf, "AB\0\0\0", sizeof(buf)));

	/* Restore defaults */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 1);

	return 0;
}
DM_TEST(dm_test_i2c_offset, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
