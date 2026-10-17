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
#include <asm/test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that we can find buses and chip-selects */
static int dm_test_spi_find(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();
	struct spi_slave *slave;
	struct udevice *bus, *dev;
	const int busnum = 0, cs = 0, mode = 0, speed = 1000000, cs_b = 2;
	struct spi_cs_info info;
	ofnode node;

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
	ut_asserteq(-ENODEV, spi_get_bus_and_cs(busnum, cs, &bus, &slave));

	/*
	 * This forces the device to be re-added, but there is no emulation
	 * connected so the probe will fail. We require that bus is left
	 * alone on failure, and that the _spi_get_bus_and_cs() does not add
	 * a 'partially-inited' device.
	 */
	ut_asserteq(-ENODEV, spi_find_bus_and_cs(busnum, cs, &bus, &dev));
	ut_asserteq(-ENOENT, _spi_get_bus_and_cs(busnum, cs, speed, mode,
						 "jedec_spi_nor", "name", &bus,
						 &slave));
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs);
	ut_assertok(spi_cs_info(bus, cs, &info));
	ut_asserteq_ptr(NULL, info.dev);

	/* Add the emulation and try again */
	ut_assertok(sandbox_sf_bind_emul(state, busnum, cs, bus, node,
					 "name"));
	ut_assertok(spi_find_bus_and_cs(busnum, cs, &bus, &dev));
	ut_assertok(_spi_get_bus_and_cs(busnum, cs, speed, mode,
					"jedec_spi_nor", "name", &bus, &slave));

	ut_assertok(spi_cs_info(bus, cs, &info));
	ut_asserteq_ptr(info.dev, slave->dev);

	/* We should be able to add something to another chip select */
	ut_assertok(sandbox_sf_bind_emul(state, busnum, cs_b, bus, node,
					 "name"));
	ut_asserteq(-EINVAL, _spi_get_bus_and_cs(busnum, cs_b, speed, mode,
						 "jedec_spi_nor", "name", &bus,
						 &slave));
	ut_asserteq(-EINVAL, spi_cs_info(bus, cs_b, &info));
	ut_asserteq_ptr(NULL, info.dev);

	/*
	 * Since we are about to destroy all devices, we must tell sandbox
	 * to forget the emulation device
	 */
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs);
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs_b);

	return 0;
}
DM_TEST(dm_test_spi_find, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* dm_test_spi_switch_slaves - Helper function to check whether spi_claim_bus
 *                             operates correctly with two spi slaves.
 *
 * Check that switching back and forth between two slaves claiming the bus
 * will update dm_spi_bus->speed and sandbox_spi bus speed/mode correctly.
 *
 * @uts - unit test state
 * @slave_a - first spi slave used for testing
 * @slave_b - second spi slave used for testing
 */
static int dm_test_spi_switch_slaves(struct unit_test_state *uts,
				     struct spi_slave *slave_a,
				     struct spi_slave *slave_b)
{
	struct udevice *bus;
	struct dm_spi_bus *bus_data;

	/* Check that slaves are on the same bus */
	ut_asserteq_ptr(dev_get_parent(slave_a->dev),
			dev_get_parent(slave_b->dev));

	bus = dev_get_parent(slave_a->dev);
	bus_data = dev_get_uclass_priv(bus);

	ut_assertok(spi_claim_bus(slave_a));
	ut_asserteq(slave_a->max_hz, bus_data->speed);
	ut_asserteq(slave_a->max_hz, sandbox_spi_get_speed(bus));
	ut_asserteq(slave_a->mode, sandbox_spi_get_mode(bus));
	spi_release_bus(slave_a);

	ut_assertok(spi_claim_bus(slave_b));
	ut_asserteq(slave_b->max_hz, bus_data->speed);
	ut_asserteq(slave_b->max_hz, sandbox_spi_get_speed(bus));
	ut_asserteq(slave_b->mode, sandbox_spi_get_mode(bus));
	spi_release_bus(slave_b);

	ut_assertok(spi_claim_bus(slave_a));
	ut_asserteq(slave_a->max_hz, bus_data->speed);
	ut_asserteq(slave_a->max_hz, sandbox_spi_get_speed(bus));
	ut_asserteq(slave_a->mode, sandbox_spi_get_mode(bus));
	spi_release_bus(slave_a);

	return 0;
}

static int dm_test_spi_claim_bus(struct unit_test_state *uts)
{
	struct udevice *bus;
	struct spi_slave *slave_a, *slave_b;
	struct dm_spi_slave_plat *slave_plat;
	const int busnum = 0, cs_a = 0, cs_b = 1;

	/* Get spi slave on CS0 */
	ut_assertok(spi_get_bus_and_cs(busnum, cs_a, &bus, &slave_a));
	/* Get spi slave on CS1 */
	ut_assertok(spi_get_bus_and_cs(busnum, cs_b, &bus, &slave_b));

	/* Different max_hz, different mode. */
	ut_assert(slave_a->max_hz != slave_b->max_hz);
	ut_assert(slave_a->mode != slave_b->mode);
	dm_test_spi_switch_slaves(uts, slave_a, slave_b);

	/* Different max_hz, same mode. */
	slave_a->mode = slave_b->mode;
	dm_test_spi_switch_slaves(uts, slave_a, slave_b);

	/*
	 * Same max_hz, different mode.
	 * Restore original mode for slave_a, from platdata.
	 */
	slave_plat = dev_get_parent_plat(slave_a->dev);
	slave_a->mode = slave_plat->mode;
	slave_a->max_hz = slave_b->max_hz;
	dm_test_spi_switch_slaves(uts, slave_a, slave_b);

	return 0;
}
DM_TEST(dm_test_spi_claim_bus, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that sandbox SPI works correctly */
static int dm_test_spi_xfer(struct unit_test_state *uts)
{
	struct spi_slave *slave;
	struct udevice *bus;
	const int busnum = 0, cs = 0;
	const char dout[5] = {0x9f};
	unsigned char din[5];

	ut_assertok(spi_get_bus_and_cs(busnum, cs, &bus, &slave));
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
#if CONFIG_IS_ENABLED(DM_SPI_FLASH)
	sandbox_sf_unbind_emul(state_get_current(), busnum, cs);
#endif

	return 0;
}
DM_TEST(dm_test_spi_xfer, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
