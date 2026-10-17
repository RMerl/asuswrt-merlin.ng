// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Collabora
 * (C) Copyright 2019 GE
 */

#include <common.h>
#include <bootcount.h>
#include <dm.h>
#include <spi_flash.h>

static const u8 bootcount_magic = 0xbc;

struct bootcount_spi_flash_priv {
	struct udevice *spi_flash;
	u32 offset;
};

static int bootcount_spi_flash_update(struct udevice *dev, u32 offset, u32 len, const void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	u32 sector_size = flash->sector_size;
	u32 sector_offset = offset % sector_size;
	u32 sector = offset - sector_offset;
	int err = 0;

	/* code only supports updating a single sector */
	if (sector_offset + len > sector_size)
		return -ENOSYS;

	u8 *buffer = malloc(sector_size);
	if (!buffer)
		return -ENOMEM;

	err = spi_flash_read_dm(dev, sector, sector_size, buffer);
	if (err < 0)
		goto out;

	memcpy(buffer + sector_offset, buf, len);

	err = spi_flash_erase_dm(dev, sector, sector_size);
	if (err < 0)
		goto out;

	err = spi_flash_write_dm(dev, sector, sector_size, buffer);
	if (err < 0)
		goto out;

out:
	free(buffer);
	return err;
}

static int bootcount_spi_flash_set(struct udevice *dev, const u32 a)
{
	struct bootcount_spi_flash_priv *priv = dev_get_priv(dev);
	const u16 val = bootcount_magic << 8 | (a & 0xff);

	if (bootcount_spi_flash_update(priv->spi_flash, priv->offset, 2, &val) < 0) {
		debug("%s: write failed\n", __func__);
		return -EIO;
	}

	return 0;
}

static int bootcount_spi_flash_get(struct udevice *dev, u32 *a)
{
	struct bootcount_spi_flash_priv *priv = dev_get_priv(dev);
	u16 val;

	if (spi_flash_read_dm(priv->spi_flash, priv->offset, 2, &val) < 0) {
		debug("%s: read failed\n", __func__);
		return -EIO;
	}

	if (val >> 8 == bootcount_magic) {
		*a = val & 0xff;
		return 0;
	}

	debug("%s: bootcount magic does not match on %04x\n", __func__, val);
	return -EIO;
}

static int bootcount_spi_flash_probe(struct udevice *dev)
{
	struct ofnode_phandle_args phandle_args;
	struct bootcount_spi_flash_priv *priv = dev_get_priv(dev);
	struct udevice *spi_flash;

	if (dev_read_phandle_with_args(dev, "spi-flash", NULL, 0, 0, &phandle_args)) {
		debug("%s: spi-flash backing device not specified\n", dev->name);
		return -ENOENT;
	}

	if (uclass_get_device_by_ofnode(UCLASS_SPI_FLASH, phandle_args.node, &spi_flash)) {
		debug("%s: could not get backing device\n", dev->name);
		return -ENODEV;
	}

	priv->spi_flash = spi_flash;
	priv->offset = dev_read_u32_default(dev, "offset", 0);

	return 0;
}

static const struct bootcount_ops bootcount_spi_flash_ops = {
	.get = bootcount_spi_flash_get,
	.set = bootcount_spi_flash_set,
};

static const struct udevice_id bootcount_spi_flash_ids[] = {
	{ .compatible = "u-boot,bootcount-spi-flash" },
	{ }
};

U_BOOT_DRIVER(bootcount_spi_flash) = {
	.name	= "bootcount-spi-flash",
	.id	= UCLASS_BOOTCOUNT,
	.priv_auto	= sizeof(struct bootcount_spi_flash_priv),
	.probe	= bootcount_spi_flash_probe,
	.of_match = bootcount_spi_flash_ids,
	.ops	= &bootcount_spi_flash_ops,
};
