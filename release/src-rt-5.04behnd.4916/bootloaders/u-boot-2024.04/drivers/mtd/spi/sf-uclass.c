// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY UCLASS_SPI_FLASH

#include <common.h>
#include <bootdev.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <test/test.h>
#include "sf_internal.h"

DECLARE_GLOBAL_DATA_PTR;

int spi_flash_read_dm(struct udevice *dev, u32 offset, size_t len, void *buf)
{
	return log_ret(sf_get_ops(dev)->read(dev, offset, len, buf));
}

int spi_flash_write_dm(struct udevice *dev, u32 offset, size_t len,
		       const void *buf)
{
	return log_ret(sf_get_ops(dev)->write(dev, offset, len, buf));
}

int spi_flash_erase_dm(struct udevice *dev, u32 offset, size_t len)
{
	return log_ret(sf_get_ops(dev)->erase(dev, offset, len));
}

int spl_flash_get_sw_write_prot(struct udevice *dev)
{
	struct dm_spi_flash_ops *ops = sf_get_ops(dev);

	if (!ops->get_sw_write_prot)
		return -ENOSYS;
	return log_ret(ops->get_sw_write_prot(dev));
}

/*
 * TODO(sjg@chromium.org): This is an old-style function. We should remove
 * it when all SPI flash drivers use dm
 */
struct spi_flash *spi_flash_probe(unsigned int busnum, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *slave;
	struct udevice *bus;
	char *str;

#if defined(CONFIG_SPL_BUILD) && CONFIG_IS_ENABLED(USE_TINY_PRINTF)
	str = "spi_flash";
#else
	char name[30];

	snprintf(name, sizeof(name), "spi_flash@%d:%d", busnum, cs);
	str = strdup(name);
#endif

	if (_spi_get_bus_and_cs(busnum, cs, max_hz, spi_mode,
				"jedec_spi_nor", str, &bus, &slave))
		return NULL;

	return dev_get_uclass_priv(slave->dev);
}

int spi_flash_probe_bus_cs(unsigned int busnum, unsigned int cs,
			   struct udevice **devp)
{
	struct spi_slave *slave;
	struct udevice *bus;
	int ret;

	ret = spi_get_bus_and_cs(busnum, cs, &bus, &slave);
	if (ret)
		return ret;

	*devp = slave->dev;
	return 0;
}

static int spi_flash_post_bind(struct udevice *dev)
{
	int ret;

	if (CONFIG_IS_ENABLED(BOOTDEV_SPI_FLASH) && test_sf_bootdev_enabled()) {
		ret = bootdev_setup_for_dev(dev, "sf_bootdev");
		if (ret)
			return log_msg_ret("bd", ret);
	}

	return 0;
}

UCLASS_DRIVER(spi_flash) = {
	.id		= UCLASS_SPI_FLASH,
	.name		= "spi_flash",
	.post_bind	= spi_flash_post_bind,
	.per_device_auto	= sizeof(struct spi_nor),
};
