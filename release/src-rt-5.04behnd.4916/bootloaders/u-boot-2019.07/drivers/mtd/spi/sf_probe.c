// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI flash probing
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>

#include "sf_internal.h"

/**
 * spi_flash_probe_slave() - Probe for a SPI flash device on a bus
 *
 * @flashp: Pointer to place to put flash info, which may be NULL if the
 * space should be allocated
 */
static int spi_flash_probe_slave(struct spi_flash *flash)
{
	struct spi_slave *spi = flash->spi;
	int ret;

	/* Setup spi_slave */
	if (!spi) {
		printf("SF: Failed to set up slave\n");
		return -ENODEV;
	}

	/* Claim spi bus */
	ret = spi_claim_bus(spi);
	if (ret) {
		debug("SF: Failed to claim SPI bus: %d\n", ret);
		return ret;
	}

	ret = spi_nor_scan(flash);
	if (ret)
		goto err_read_id;

#ifdef CONFIG_SPI_FLASH_MTD
	ret = spi_flash_mtd_register(flash);
#endif

err_read_id:
	spi_release_bus(spi);
	return ret;
}

#ifndef CONFIG_DM_SPI_FLASH
struct spi_flash *spi_flash_probe(unsigned int busnum, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *bus;
	struct spi_flash *flash;

	bus = spi_setup_slave(busnum, cs, max_hz, spi_mode);
	if (!bus)
		return NULL;

	/* Allocate space if needed (not used by sf-uclass */
	flash = calloc(1, sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate spi_flash\n");
		return NULL;
	}

	flash->spi = bus;
	if (spi_flash_probe_slave(flash)) {
		spi_free_slave(bus);
		free(flash);
		return NULL;
	}

	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
#ifdef CONFIG_SPI_FLASH_MTD
	spi_flash_mtd_unregister();
#endif
	spi_free_slave(flash->spi);
	free(flash);
}

#else /* defined CONFIG_DM_SPI_FLASH */

static int spi_flash_std_read(struct udevice *dev, u32 offset, size_t len,
			      void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	struct mtd_info *mtd = &flash->mtd;
	size_t retlen;

	return log_ret(mtd->_read(mtd, offset, len, &retlen, buf));
}

static int spi_flash_std_write(struct udevice *dev, u32 offset, size_t len,
			       const void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	struct mtd_info *mtd = &flash->mtd;
	size_t retlen;

	return mtd->_write(mtd, offset, len, &retlen, buf);
}

static int spi_flash_std_erase(struct udevice *dev, u32 offset, size_t len)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	struct mtd_info *mtd = &flash->mtd;
	struct erase_info instr;

	if (offset % mtd->erasesize || len % mtd->erasesize) {
		printf("SF: Erase offset/length not multiple of erase size\n");
		return -EINVAL;
	}

	memset(&instr, 0, sizeof(instr));
	instr.addr = offset;
	instr.len = len;

	return mtd->_erase(mtd, &instr);
}

static int spi_flash_std_get_sw_write_prot(struct udevice *dev)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_cmd_get_sw_write_prot(flash);
}

static int spi_flash_std_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);
	struct spi_flash *flash;

	flash = dev_get_uclass_priv(dev);
	flash->dev = dev;
	flash->spi = slave;
	debug("%s: slave=%p, cs=%d\n", __func__, slave, plat->cs);
	return spi_flash_probe_slave(flash);
}

static int spi_flash_std_remove(struct udevice *dev)
{
#ifdef CONFIG_SPI_FLASH_MTD
	spi_flash_mtd_unregister();
#endif
	return 0;
}

static const struct dm_spi_flash_ops spi_flash_std_ops = {
	.read = spi_flash_std_read,
	.write = spi_flash_std_write,
	.erase = spi_flash_std_erase,
	.get_sw_write_prot = spi_flash_std_get_sw_write_prot,
};

static const struct udevice_id spi_flash_std_ids[] = {
	{ .compatible = "jedec,spi-nor" },
	{ }
};

U_BOOT_DRIVER(spi_flash_std) = {
	.name		= "spi_flash_std",
	.id		= UCLASS_SPI_FLASH,
	.of_match	= spi_flash_std_ids,
	.probe		= spi_flash_std_probe,
	.remove		= spi_flash_std_remove,
	.priv_auto_alloc_size = sizeof(struct spi_flash),
	.ops		= &spi_flash_std_ops,
};

#endif /* CONFIG_DM_SPI_FLASH */
