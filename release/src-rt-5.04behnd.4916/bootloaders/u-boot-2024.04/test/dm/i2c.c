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
#include <hexdump.h>
#include <test/test.h>
#include <test/ut.h>

static const int busnum;
static const int chip = 0x2c;

/* Test that we can find buses and chips */
static int dm_test_i2c_find(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	const int no_chip = 0x10;

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
DM_TEST(dm_test_i2c_find, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_i2c_read_write(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	uint8_t buf[5];

	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));
	ut_assertok(i2c_get_chip(bus, chip, 1, &dev));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\0\0\0\0", sizeof(buf));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\0AB\0", sizeof(buf));

	return 0;
}
DM_TEST(dm_test_i2c_read_write, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_i2c_speed, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_i2c_offset_len, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

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
DM_TEST(dm_test_i2c_probe_empty, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_i2c_bytewise(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	struct udevice *eeprom;
	uint8_t buf[5];

	ut_assertok(uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus));
	ut_assertok(i2c_get_chip(bus, chip, 1, &dev));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\0\0\0\0", sizeof(buf));

	/* Tell the EEPROM to only read/write one register at a time */
	ut_assertok(uclass_first_device_err(UCLASS_I2C_EMUL, &eeprom));
	ut_assertnonnull(eeprom);
	sandbox_i2c_eeprom_set_test_mode(eeprom, SIE_TEST_MODE_SINGLE_BYTE);

	/* Now we only get the first byte - the rest will be 0xff */
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\xff\xff\xff\xff", sizeof(buf));

	/* If we do a separate transaction for each byte, it works */
	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\0\0\0\0", sizeof(buf));

	/* This will only write A */
	ut_assertok(i2c_set_chip_flags(dev, 0));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\xff\xff\xff\xff", sizeof(buf));

	/* Check that the B was ignored */
	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\0A\0\0\0", sizeof(buf));

	/* Now write it again with the new flags, it should work */
	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_WR_ADDRESS));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\xff\xff\xff\xff", sizeof(buf));

	ut_assertok(i2c_set_chip_flags(dev, DM_I2C_CHIP_WR_ADDRESS |
						DM_I2C_CHIP_RD_ADDRESS));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem(buf, "\0\0AB\0\0", sizeof(buf));

	/* Restore defaults */
	sandbox_i2c_eeprom_set_test_mode(eeprom, SIE_TEST_MODE_NONE);
	ut_assertok(i2c_set_chip_flags(dev, 0));

	return 0;
}
DM_TEST(dm_test_i2c_bytewise, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_i2c_offset(struct unit_test_state *uts)
{
	struct udevice *eeprom;
	struct udevice *dev;
	uint8_t buf[5];

	ut_assertok(i2c_get_chip_for_busnum(busnum, chip, 1, &dev));

	/* Do a transfer so we can find the emulator */
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(uclass_first_device_err(UCLASS_I2C_EMUL, &eeprom));

	/* Offset length 0 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 0);
	ut_assertok(i2c_set_chip_offset_len(dev, 0));
	ut_assertok(dm_i2c_write(dev, 10 /* ignored */, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem("AB\0\0\0\0", buf, sizeof(buf));
	ut_asserteq(0, sanbox_i2c_eeprom_get_prev_offset(eeprom));

	/* Offset length 1 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 1);
	ut_assertok(i2c_set_chip_offset_len(dev, 1));
	ut_assertok(dm_i2c_write(dev, 2, (uint8_t *)"AB", 2));
	ut_asserteq(2, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem("ABAB\0", buf, sizeof(buf));
	ut_asserteq(0, sanbox_i2c_eeprom_get_prev_offset(eeprom));

	/* Offset length 2 boundary - check model wrapping */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 2);
	ut_assertok(i2c_set_chip_offset_len(dev, 2));
	ut_assertok(dm_i2c_write(dev, 0xFF, (uint8_t *)"A", 1));
	ut_asserteq(0xFF, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_assertok(dm_i2c_write(dev, 0x100, (uint8_t *)"B", 1));
	ut_asserteq(0x100, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_assertok(dm_i2c_write(dev, 0x101, (uint8_t *)"C", 1));
	ut_asserteq(0x101, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_assertok(dm_i2c_read(dev, 0xFF, buf, 5));
	ut_asserteq_mem("ABCAB", buf, sizeof(buf));
	ut_asserteq(0xFF, sanbox_i2c_eeprom_get_prev_offset(eeprom));

	/* Offset length 2 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 2);
	ut_assertok(i2c_set_chip_offset_len(dev, 2));
	ut_assertok(dm_i2c_write(dev, 0x2020, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x2020, buf, 5));
	ut_asserteq_mem("AB\0\0\0", buf, sizeof(buf));
	ut_asserteq(0x2020, sanbox_i2c_eeprom_get_prev_offset(eeprom));

	/* Offset length 3 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 3);
	ut_assertok(i2c_set_chip_offset_len(dev, 3));
	ut_assertok(dm_i2c_write(dev, 0x303030, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x303030, buf, 5));
	ut_asserteq_mem("AB\0\0\0", buf, sizeof(buf));
	ut_asserteq(0x303030, sanbox_i2c_eeprom_get_prev_offset(eeprom));

	/* Offset length 4 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 4);
	ut_assertok(i2c_set_chip_offset_len(dev, 4));
	ut_assertok(dm_i2c_write(dev, 0x40404040, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x40404040, buf, 5));
	ut_asserteq_mem("AB\0\0\0", buf, sizeof(buf));
	ut_asserteq(0x40404040, sanbox_i2c_eeprom_get_prev_offset(eeprom));

	/* Restore defaults */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 1);

	return 0;
}
DM_TEST(dm_test_i2c_offset, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_i2c_addr_offset(struct unit_test_state *uts)
{
	struct udevice *eeprom;
	struct udevice *dev;
	u8 buf[5];

	ut_assertok(i2c_get_chip_for_busnum(busnum, chip, 1, &dev));

	/* Do a transfer so we can find the emulator */
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(uclass_first_device_err(UCLASS_I2C_EMUL, &eeprom));

	/* Offset length 0 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 0);
	sandbox_i2c_eeprom_set_chip_addr_offset_mask(eeprom, 0x3);
	ut_assertok(i2c_set_chip_offset_len(dev, 0));
	ut_assertok(i2c_set_chip_addr_offset_mask(dev, 0x3));
	ut_assertok(dm_i2c_write(dev, 0x3, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x3, buf, 5));
	ut_asserteq_mem("AB\0\0\0\0", buf, sizeof(buf));
	ut_asserteq(0x3, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_asserteq(chip | 0x3, sanbox_i2c_eeprom_get_prev_addr(eeprom));

	/* Offset length 1 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 1);
	sandbox_i2c_eeprom_set_chip_addr_offset_mask(eeprom, 0x3);
	ut_assertok(i2c_set_chip_offset_len(dev, 1));
	ut_assertok(i2c_set_chip_addr_offset_mask(dev, 0x3));
	ut_assertok(dm_i2c_write(dev, 0x310, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x310, buf, 5));
	ut_asserteq_mem("AB\0\0\0\0", buf, sizeof(buf));
	ut_asserteq(0x310, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_asserteq(chip | 0x3, sanbox_i2c_eeprom_get_prev_addr(eeprom));

	/* Offset length 2 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 2);
	sandbox_i2c_eeprom_set_chip_addr_offset_mask(eeprom, 0x3);
	ut_assertok(i2c_set_chip_offset_len(dev, 2));
	ut_assertok(i2c_set_chip_addr_offset_mask(dev, 0x3));
	ut_assertok(dm_i2c_write(dev, 0x32020, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x32020, buf, 5));
	ut_asserteq_mem("AB\0\0\0\0", buf, sizeof(buf));
	ut_asserteq(0x32020, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_asserteq(chip | 0x3, sanbox_i2c_eeprom_get_prev_addr(eeprom));

	/* Offset length 3 */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 3);
	sandbox_i2c_eeprom_set_chip_addr_offset_mask(eeprom, 0x3);
	ut_assertok(i2c_set_chip_offset_len(dev, 3));
	ut_assertok(i2c_set_chip_addr_offset_mask(dev, 0x3));
	ut_assertok(dm_i2c_write(dev, 0x3303030, (uint8_t *)"AB", 2));
	ut_assertok(dm_i2c_read(dev, 0x3303030, buf, 5));
	ut_asserteq_mem("AB\0\0\0\0", buf, sizeof(buf));
	ut_asserteq(0x3303030, sanbox_i2c_eeprom_get_prev_offset(eeprom));
	ut_asserteq(chip | 0x3, sanbox_i2c_eeprom_get_prev_addr(eeprom));

	/* Restore defaults */
	sandbox_i2c_eeprom_set_offset_len(eeprom, 1);
	sandbox_i2c_eeprom_set_chip_addr_offset_mask(eeprom, 0);

	return 0;
}

DM_TEST(dm_test_i2c_addr_offset, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_i2c_reg_clrset(struct unit_test_state *uts)
{
	struct udevice *eeprom;
	struct udevice *dev;
	u8 buf[5];

	ut_assertok(i2c_get_chip_for_busnum(busnum, chip, 1, &dev));

	/* Do a transfer so we can find the emulator */
	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_assertok(uclass_first_device_err(UCLASS_I2C_EMUL, &eeprom));

	/* Dummy data for the test */
	ut_assertok(dm_i2c_write(dev, 0, "\xff\x00\xff\x00\x10", 5));

	/* Do some clrset tests */
	ut_assertok(dm_i2c_reg_clrset(dev, 0, 0xff, 0x10));
	ut_assertok(dm_i2c_reg_clrset(dev, 1, 0x00, 0x11));
	ut_assertok(dm_i2c_reg_clrset(dev, 2, 0xed, 0x00));
	ut_assertok(dm_i2c_reg_clrset(dev, 3, 0xff, 0x13));
	ut_assertok(dm_i2c_reg_clrset(dev, 4, 0x00, 0x14));

	ut_assertok(dm_i2c_read(dev, 0, buf, 5));
	ut_asserteq_mem("\x10\x11\x12\x13\x14", buf, sizeof(buf));

	return 0;
}
DM_TEST(dm_test_i2c_reg_clrset, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
