// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/state.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <test/ut.h>

/* Test that we can find buses and chip-selects */
static int dm_test_spi_find(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();
	struct spi_slave *slave;
	struct udevice *bus, *dev;
	const int busnum = 0, cs = 0, mode = 0, speed = 1000000, cs_b = 1;
	struct spi_cs_info info;
	ofnode node;

	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_SPI, busnum,
						       false, &bus));

	/*
	 * The post_bind() method will bind devices to chip selects. Check
	 * this then remove the emulation and the slave device.
	 */
	ut_asserteq(0, uclass_get_device_by_seq(UCLASS_SPI, busnum, &bus));
	ut_assertok(spi_cs_info(bus, cs, &info));
	node = dev_ofnode(info.dev);
	device_remove(info.dev, DM_REMOVE_NORMAL);
	device_unbind(info.dev);

	/*
	 * Even though the device is gone, the sandbox SPI drivers always
	 * reports that CS 0 is present
	 */
	ut_assertok(spi_cs_info(bus, cs, &info));
	ut_asserteq_ptr(NULL, info.dev);

	/* This finds nothing because we removed the device */
	ut_asserteq(-ENODEV, spi_find_bus_and_cs(busnum, cs, &bus, &dev));
	ut_asserteq(-ENODEV, spi_get_bus_and_cs(busnum, cs, speed, mode,
						NULL, 0, &bus, &slave));

	/*
	 * This forces the device to be re-added, but there is no emulation
	 * connected so the probe will fail. We require that bus is left
	 * alone on failure, and that the spi_get_bus_and_cs() does not add
	 * a 'partially-inited' device.
	 */
	ut_asserteq(-ENODEV, spi_find_bus_and_cs(busnum, cs, &bus, &dev));
	ut_asserteq(-ENOENT, spi_get_bus_and_cs(busnum, cs, speed, mode,
						"spi_flash_std", "name", &bus,
						&slave));
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs);
	ut_assertok(spi_cs_info(bus, cs, &info));
	ut_asserteq_ptr(NULL, info.dev);

	/* Add the emulation and try again */
	ut_assertok(sandbox_sf_bind_emul(state, busnum, cs, bus, node,
					 "name"));
	ut_assertok(spi_find_bus_and_cs(busnum, cs, &bus, &dev));
	ut_assertok(spi_get_bus_and_cs(busnum, cs, speed, mode,
				       "spi_flash_std", "name", &bus, &slave));

	ut_assertok(spi_cs_info(bus, cs, &info));
	ut_asserteq_ptr(info.dev, slave->dev);

	/* We should be able to add something to another chip select */
	ut_assertok(sandbox_sf_bind_emul(state, busnum, cs_b, bus, node,
					 "name"));
	ut_assertok(spi_get_bus_and_cs(busnum, cs_b, speed, mode,
				       "spi_flash_std", "name", &bus, &slave));
	ut_assertok(spi_cs_info(bus, cs_b, &info));
	ut_asserteq_ptr(info.dev, slave->dev);

	/*
	 * Since we are about to destroy all devices, we must tell sandbox
	 * to forget the emulation device
	 */
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs);
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs_b);

	return 0;
}
DM_TEST(dm_test_spi_find, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that sandbox SPI works correctly */
static int dm_test_spi_xfer(struct unit_test_state *uts)
{
	struct spi_slave *slave;
	struct udevice *bus;
	const int busnum = 0, cs = 0, mode = 0;
	const char dout[5] = {0x9f};
	unsigned char din[5];

	ut_assertok(spi_get_bus_and_cs(busnum, cs, 1000000, mode, NULL, 0,
				       &bus, &slave));
	ut_assertok(spi_claim_bus(slave));
	ut_assertok(spi_xfer(slave, 40, dout, din,
			     SPI_XFER_BEGIN | SPI_XFER_END));
	ut_asserteq(0xff, din[0]);
	ut_asserteq(0x20, din[1]);
	ut_asserteq(0x20, din[2]);
	ut_asserteq(0x15, din[3]);
	spi_release_bus(slave);

	/*
	 * Since we are about to destroy all devices, we must tell sandbox
	 * to forget the emulation device
	 */
#ifdef CONFIG_DM_SPI_FLASH
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs);
#endif

	return 0;
}
DM_TEST(dm_test_spi_xfer, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
