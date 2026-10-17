// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY UCLASS_SPI

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>
#include <dm/device_compat.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <dm/lists.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

#define SPI_DEFAULT_SPEED_HZ 100000

static int spi_set_speed_mode(struct udevice *bus, int speed, int mode)
{
	struct dm_spi_ops *ops;
	int ret;

	ops = spi_get_ops(bus);
	if (ops->set_speed)
		ret = ops->set_speed(bus, speed);
	else
		ret = -EINVAL;
	if (ret) {
		dev_err(bus, "Cannot set speed (err=%d)\n", ret);
		return ret;
	}

	if (ops->set_mode)
		ret = ops->set_mode(bus, mode);
	else
		ret = -EINVAL;
	if (ret) {
		dev_err(bus, "Cannot set mode (err=%d)\n", ret);
		return ret;
	}

	return 0;
}

int dm_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_ops *ops = spi_get_ops(bus);
	struct dm_spi_bus *spi = dev_get_uclass_priv(bus);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	uint speed, mode;

	speed = slave->max_hz;
	mode = slave->mode;

	if (spi->max_hz) {
		if (speed)
			speed = min(speed, spi->max_hz);
		else
			speed = spi->max_hz;
	}
	if (!speed)
		speed = SPI_DEFAULT_SPEED_HZ;

	if (speed != spi->speed || mode != spi->mode) {
		int ret = spi_set_speed_mode(bus, speed, slave->mode);

		if (ret)
			return log_ret(ret);

		spi->speed = speed;
		spi->mode = mode;
	}

	return log_ret(ops->claim_bus ? ops->claim_bus(dev) : 0);
}

void dm_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_ops *ops = spi_get_ops(bus);

	if (ops->release_bus)
		ops->release_bus(dev);
}

int dm_spi_xfer(struct udevice *dev, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_ops *ops = spi_get_ops(bus);

	if (bus->uclass->uc_drv->id != UCLASS_SPI)
		return -EOPNOTSUPP;
	if (!ops->xfer)
		return -ENOSYS;

	return ops->xfer(dev, bitlen, dout, din, flags);
}

int dm_spi_get_mmap(struct udevice *dev, ulong *map_basep, uint *map_sizep,
		    uint *offsetp)
{
	struct udevice *bus = dev->parent;
	struct dm_spi_ops *ops = spi_get_ops(bus);

	if (bus->uclass->uc_drv->id != UCLASS_SPI)
		return -EOPNOTSUPP;
	if (!ops->get_mmap)
		return -ENOSYS;

	return ops->get_mmap(dev, map_basep, map_sizep, offsetp);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return log_ret(dm_spi_claim_bus(slave->dev));
}

void spi_release_bus(struct spi_slave *slave)
{
	dm_spi_release_bus(slave->dev);
}

int spi_set_speed(struct spi_slave *slave, uint hz)
{
	struct dm_spi_ops *ops;
	int ret;

	ops = spi_get_ops(slave->dev->parent);
	if (ops->set_speed)
		ret = ops->set_speed(slave->dev->parent, hz);
	else
		ret = -EINVAL;
	if (ret)
		dev_err(slave->dev, "Cannot set speed (err=%d)\n", ret);
	return ret;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	return dm_spi_xfer(slave->dev, bitlen, dout, din, flags);
}

int spi_write_then_read(struct spi_slave *slave, const u8 *opcode,
			size_t n_opcode, const u8 *txbuf, u8 *rxbuf,
			size_t n_buf)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if (n_buf == 0)
		flags |= SPI_XFER_END;

	ret = spi_xfer(slave, n_opcode * 8, opcode, NULL, flags);
	if (ret) {
		dev_dbg(slave->dev,
			"spi: failed to send command (%zu bytes): %d\n",
			n_opcode, ret);
	} else if (n_buf != 0) {
		ret = spi_xfer(slave, n_buf * 8, txbuf, rxbuf, SPI_XFER_END);
		if (ret)
			dev_dbg(slave->dev,
				"spi: failed to transfer %zu bytes of data: %d\n",
				n_buf, ret);
	}

	return ret;
}

#if CONFIG_IS_ENABLED(OF_REAL)
static int spi_child_post_bind(struct udevice *dev)
{
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);

	if (!dev_has_ofnode(dev))
		return 0;

	return spi_slave_of_to_plat(dev, plat);
}
#endif

static int spi_post_probe(struct udevice *bus)
{
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		struct dm_spi_bus *spi = dev_get_uclass_priv(bus);

		spi->max_hz = dev_read_u32_default(bus, "spi-max-frequency", 0);
	}

	return 0;
}

static int spi_child_pre_probe(struct udevice *dev)
{
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);

	/*
	 * This is needed because we pass struct spi_slave around the place
	 * instead slave->dev (a struct udevice). So we have to have some
	 * way to access the slave udevice given struct spi_slave. Once we
	 * change the SPI API to use udevice instead of spi_slave, we can
	 * drop this.
	 */
	slave->dev = dev;

	slave->max_hz = plat->max_hz;
	slave->mode = plat->mode;
	slave->wordlen = SPI_DEFAULT_WORDLEN;

	return 0;
}

int spi_chip_select(struct udevice *dev)
{
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);

	return plat ? plat->cs : -ENOENT;
}

int spi_find_chip_select(struct udevice *bus, int cs, struct udevice **devp)
{
	struct dm_spi_ops *ops;
	struct spi_cs_info info;
	struct udevice *dev;
	int ret;

	/*
	 * Ask the driver. For the moment we don't have CS info.
	 * When we do we could provide the driver with a helper function
	 * to figure out what chip selects are valid, or just handle the
	 * request.
	 */
	ops = spi_get_ops(bus);
	if (ops->cs_info) {
		ret = ops->cs_info(bus, cs, &info);
	} else {
		/*
		 * We could assume there is at least one valid chip select.
		 * The driver didn't care enough to tell us.
		 */
		ret = 0;
	}

	if (ret) {
		dev_err(bus, "Invalid cs %d (err=%d)\n", cs, ret);
		return ret;
	}

	for (device_find_first_child(bus, &dev); dev;
	     device_find_next_child(&dev)) {
		struct dm_spi_slave_plat *plat;

		plat = dev_get_parent_plat(dev);
		dev_dbg(bus, "%s: plat=%p, cs=%d\n", __func__, plat, plat->cs);
		if (plat->cs == cs) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int spi_cs_is_valid(unsigned int busnum, unsigned int cs)
{
	struct spi_cs_info info;
	struct udevice *bus;
	int ret;

	ret = uclass_find_device_by_seq(UCLASS_SPI, busnum, &bus);
	if (ret) {
		log_debug("%s: No bus %d\n", __func__, busnum);
		return ret;
	}

	return spi_cs_info(bus, cs, &info);
}

int spi_cs_info(struct udevice *bus, uint cs, struct spi_cs_info *info)
{
	struct spi_cs_info local_info;
	int ret;

	if (!info)
		info = &local_info;

	/* If there is a device attached, return it */
	info->dev = NULL;
	ret = spi_find_chip_select(bus, cs, &info->dev);
	return ret == -ENODEV ? 0 : ret;
}

int spi_find_bus_and_cs(int busnum, int cs, struct udevice **busp,
			struct udevice **devp)
{
	struct udevice *bus, *dev;
	int ret;

	ret = uclass_find_device_by_seq(UCLASS_SPI, busnum, &bus);
	if (ret) {
		log_debug("%s: No bus %d\n", __func__, busnum);
		return ret;
	}
	ret = spi_find_chip_select(bus, cs, &dev);
	if (ret) {
		dev_dbg(bus, "%s: No cs %d\n", __func__, cs);
		return ret;
	}
	*busp = bus;
	*devp = dev;

	return ret;
}

int spi_get_bus_and_cs(int busnum, int cs, struct udevice **busp,
		       struct spi_slave **devp)
{
	struct udevice *bus, *dev;
	struct dm_spi_bus *bus_data;
	struct spi_slave *slave;
	int ret;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	ret = uclass_first_device_err(UCLASS_SPI, &bus);
#else
	ret = uclass_get_device_by_seq(UCLASS_SPI, busnum, &bus);
#endif
	if (ret) {
		log_err("Invalid bus %d (err=%d)\n", busnum, ret);
		return ret;
	}
	ret = spi_find_chip_select(bus, cs, &dev);
	if (ret) {
		dev_err(bus, "Invalid chip select %d:%d (err=%d)\n", busnum, cs, ret);
		return ret;
	}

	if (!device_active(dev)) {
		struct spi_slave *slave;

		ret = device_probe(dev);
		if (ret)
			goto err;
		slave = dev_get_parent_priv(dev);
		slave->dev = dev;
	}

	slave = dev_get_parent_priv(dev);
	bus_data = dev_get_uclass_priv(bus);

	/*
	 * In case the operation speed is not yet established by
	 * dm_spi_claim_bus() ensure the bus is configured properly.
	 */
	if (!bus_data->speed) {
		ret = spi_claim_bus(slave);
		if (ret)
			goto err;
	}
	*busp = bus;
	*devp = slave;

	return 0;

err:
	log_debug("%s: Error path, device '%s'\n", __func__, dev->name);

	return ret;
}

int _spi_get_bus_and_cs(int busnum, int cs, int speed, int mode,
			const char *drv_name, const char *dev_name,
			struct udevice **busp, struct spi_slave **devp)
{
	struct udevice *bus, *dev;
	struct dm_spi_slave_plat *plat;
	struct dm_spi_bus *bus_data;
	struct spi_slave *slave;
	bool created = false;
	int ret;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	ret = uclass_first_device_err(UCLASS_SPI, &bus);
#else
	ret = uclass_get_device_by_seq(UCLASS_SPI, busnum, &bus);
#endif
	if (ret) {
		log_err("Invalid bus %d (err=%d)\n", busnum, ret);
		return ret;
	}
	ret = spi_find_chip_select(bus, cs, &dev);

	/*
	 * If there is no such device, create one automatically. This means
	 * that we don't need a device tree node or platform data for the
	 * SPI flash chip - we will bind to the correct driver.
	 */
	if (ret == -ENODEV && drv_name) {
		dev_dbg(bus, "%s: Binding new device '%s', busnum=%d, cs=%d, driver=%s\n",
			__func__, dev_name, busnum, cs, drv_name);
		ret = device_bind_driver(bus, drv_name, dev_name, &dev);
		if (ret) {
			dev_dbg(bus, "%s: Unable to bind driver (ret=%d)\n",
				__func__, ret);
			return ret;
		}
		plat = dev_get_parent_plat(dev);
		plat->cs = cs;
		if (speed) {
			plat->max_hz = speed;
		} else {
			dev_warn(bus,
				 "Warning: SPI speed fallback to %u kHz\n",
				 SPI_DEFAULT_SPEED_HZ / 1000);
			plat->max_hz = SPI_DEFAULT_SPEED_HZ;
		}
		plat->mode = mode;
		created = true;
	} else if (ret) {
		dev_err(bus, "Invalid chip select %d:%d (err=%d)\n", busnum, cs, ret);
		return ret;
	} else if (dev) {
		plat = dev_get_parent_plat(dev);
	}

	if (!device_active(dev)) {
		struct spi_slave *slave;

		ret = device_probe(dev);
		if (ret)
			goto err;
		slave = dev_get_parent_priv(dev);
		slave->dev = dev;
	}

	slave = dev_get_parent_priv(dev);
	bus_data = dev_get_uclass_priv(bus);

	/*
	 * In case the operation speed is not yet established by
	 * dm_spi_claim_bus() ensure the bus is configured properly.
	 */
	if (!bus_data->speed) {
		ret = spi_claim_bus(slave);
		if (ret)
			goto err;
	}

	/* In case bus frequency or mode changed, update it. */
	if ((speed && bus_data->speed && bus_data->speed != speed) ||
	    (plat && plat->mode != mode)) {
		ret = spi_set_speed_mode(bus, speed, mode);
		if (ret)
			goto err_speed_mode;
	}

	*busp = bus;
	*devp = slave;
	log_debug("%s: bus=%p, slave=%p\n", __func__, bus, *devp);

	return 0;

err_speed_mode:
	spi_release_bus(slave);
err:
	log_debug("%s: Error path, created=%d, device '%s'\n", __func__,
		  created, dev->name);
	if (created) {
		device_remove(dev, DM_REMOVE_NORMAL);
		device_unbind(dev);
	}

	return ret;
}

/* Compatibility function - to be removed */
struct spi_slave *spi_setup_slave(unsigned int busnum, unsigned int cs,
				  unsigned int speed, unsigned int mode)
{
	struct spi_slave *slave;
	struct udevice *dev;
	int ret;

	ret = _spi_get_bus_and_cs(busnum, cs, speed, mode, NULL, 0, &dev,
				  &slave);
	if (ret)
		return NULL;

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	device_remove(slave->dev, DM_REMOVE_NORMAL);
}

int spi_slave_of_to_plat(struct udevice *dev, struct dm_spi_slave_plat *plat)
{
	int mode = 0;
	int value;

	plat->cs = dev_read_u32_default(dev, "reg", -1);
	plat->max_hz = dev_read_u32_default(dev, "spi-max-frequency",
					    SPI_DEFAULT_SPEED_HZ);
	if (dev_read_bool(dev, "spi-cpol"))
		mode |= SPI_CPOL;
	if (dev_read_bool(dev, "spi-cpha"))
		mode |= SPI_CPHA;
	if (dev_read_bool(dev, "spi-cs-high"))
		mode |= SPI_CS_HIGH;
	if (dev_read_bool(dev, "spi-3wire"))
		mode |= SPI_3WIRE;
	if (dev_read_bool(dev, "spi-half-duplex"))
		mode |= SPI_PREAMBLE;

	/* Device DUAL/QUAD mode */
	value = dev_read_u32_default(dev, "spi-tx-bus-width", 1);
	switch (value) {
	case 1:
		break;
	case 2:
		mode |= SPI_TX_DUAL;
		break;
	case 4:
		mode |= SPI_TX_QUAD;
		break;
	case 8:
		mode |= SPI_TX_OCTAL;
		break;
	default:
		warn_non_spl("spi-tx-bus-width %d not supported\n", value);
		break;
	}

	value = dev_read_u32_default(dev, "spi-rx-bus-width", 1);
	switch (value) {
	case 1:
		break;
	case 2:
		mode |= SPI_RX_DUAL;
		break;
	case 4:
		mode |= SPI_RX_QUAD;
		break;
	case 8:
		mode |= SPI_RX_OCTAL;
		break;
	default:
		warn_non_spl("spi-rx-bus-width %d not supported\n", value);
		break;
	}

	plat->mode = mode;

	return 0;
}

UCLASS_DRIVER(spi) = {
	.id		= UCLASS_SPI,
	.name		= "spi",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
#if CONFIG_IS_ENABLED(OF_REAL)
	.post_bind	= dm_scan_fdt_dev,
#endif
	.post_probe	= spi_post_probe,
	.child_pre_probe = spi_child_pre_probe,
	.per_device_auto	= sizeof(struct dm_spi_bus),
	.per_child_auto	= sizeof(struct spi_slave),
	.per_child_plat_auto	= sizeof(struct dm_spi_slave_plat),
#if CONFIG_IS_ENABLED(OF_REAL)
	.child_post_bind = spi_child_post_bind,
#endif
};

UCLASS_DRIVER(spi_generic) = {
	.id		= UCLASS_SPI_GENERIC,
	.name		= "spi_generic",
};

U_BOOT_DRIVER(spi_generic_drv) = {
	.name		= "spi_generic_drv",
	.id		= UCLASS_SPI_GENERIC,
};
