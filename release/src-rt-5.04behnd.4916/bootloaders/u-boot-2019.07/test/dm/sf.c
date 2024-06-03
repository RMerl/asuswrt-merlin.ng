// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <mapmem.h>
#include <os.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/test.h>
#include <dm/util.h>
#include <test/ut.h>

/* Simple test of sandbox SPI flash */
static int dm_test_spi_flash(struct unit_test_state *uts)
{
	struct udevice *dev, *emul;
	int full_size = 0x200000;
	int size = 0x10000;
	u8 *src, *dst;
	int i;

	src = map_sysmem(0x20000, full_size);
	ut_assertok(os_write_file("spi.bin", src, full_size));
	ut_assertok(uclass_first_device_err(UCLASS_SPI_FLASH, &dev));

	dst = map_sysmem(0x20000 + full_size, full_size);
	ut_assertok(spi_flash_read_dm(dev, 0, size, dst));
	ut_assertok(memcmp(src, dst, size));

	/* Erase */
	ut_assertok(spi_flash_erase_dm(dev, 0, size));
	ut_assertok(spi_flash_read_dm(dev, 0, size, dst));
	for (i = 0; i < size; i++)
		ut_asserteq(dst[i], 0xff);

	/* Write some new data */
	for (i = 0; i < size; i++)
		src[i] = i;
	ut_assertok(spi_flash_write_dm(dev, 0, size, src));
	ut_assertok(spi_flash_read_dm(dev, 0, size, dst));
	ut_assertok(memcmp(src, dst, size));

	/* Try the write-protect stuff */
	ut_assertok(uclass_first_device_err(UCLASS_SPI_EMUL, &emul));
	ut_asserteq(0, spl_flash_get_sw_write_prot(dev));
	sandbox_sf_set_block_protect(emul, 1);
	ut_asserteq(1, spl_flash_get_sw_write_prot(dev));
	sandbox_sf_set_block_protect(emul, 0);
	ut_asserteq(0, spl_flash_get_sw_write_prot(dev));

	/*
	 * Since we are about to destroy all devices, we must tell sandbox
	 * to forget the emulation device
	 */
	sandbox_sf_unbind_emul(state_get_current(), 0, 0);

	return 0;
}
DM_TEST(dm_test_spi_flash, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Functional test that sandbox SPI flash works correctly */
static int dm_test_spi_flash_func(struct unit_test_state *uts)
{
	/*
	 * Create an empty test file and run the SPI flash tests. This is a
	 * long way from being a unit test, but it does test SPI device and
	 * emulator binding, probing, the SPI flash emulator including
	 * device tree decoding, plus the file-based backing store of SPI.
	 *
	 * More targeted tests could be created to perform the above steps
	 * one at a time. This might not increase test coverage much, but
	 * it would make bugs easier to find. It's not clear whether the
	 * benefit is worth the extra complexity.
	 */
	ut_asserteq(0, run_command_list(
		"host save hostfs - 0 spi.bin 200000;"
		"sf probe;"
		"sf test 0 10000", -1,  0));
	/*
	 * Since we are about to destroy all devices, we must tell sandbox
	 * to forget the emulation device
	 */
	sandbox_sf_unbind_emul(state_get_current(), 0, 0);

	return 0;
}
DM_TEST(dm_test_spi_flash_func, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
