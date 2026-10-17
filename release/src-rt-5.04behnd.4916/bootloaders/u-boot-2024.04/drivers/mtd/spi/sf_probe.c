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
#include <linux/mtd/spi-nor.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <spi-mem.h>

#include "sf_internal.h"

static int spi_nor_create_read_dirmap(struct spi_nor *nor)
{
	struct spi_mem_dirmap_info info = {
		.op_tmpl = SPI_MEM_OP(SPI_MEM_OP_CMD(nor->read_opcode, 0),
				      SPI_MEM_OP_ADDR(nor->addr_width, 0, 0),
				      SPI_MEM_OP_DUMMY(nor->read_dummy, 0),
				      SPI_MEM_OP_DATA_IN(0, NULL, 0)),
		.offset = 0,
		.length = nor->mtd.size,
	};
	struct spi_mem_op *op = &info.op_tmpl;

	/* get transfer protocols. */
	spi_nor_setup_op(nor, op, nor->read_proto);
	op->data.buswidth = spi_nor_get_protocol_data_nbits(nor->read_proto);

	/* convert the dummy cycles to the number of bytes */
	op->dummy.nbytes = (nor->read_dummy * op->dummy.buswidth) / 8;
	if (spi_nor_protocol_is_dtr(nor->read_proto))
		op->dummy.nbytes *= 2;

	nor->dirmap.rdesc = spi_mem_dirmap_create(nor->spi, &info);
	if (IS_ERR(nor->dirmap.rdesc))
		return PTR_ERR(nor->dirmap.rdesc);

	return 0;
}

static int spi_nor_create_write_dirmap(struct spi_nor *nor)
{
	struct spi_mem_dirmap_info info = {
		.op_tmpl = SPI_MEM_OP(SPI_MEM_OP_CMD(nor->program_opcode, 0),
				      SPI_MEM_OP_ADDR(nor->addr_width, 0, 0),
				      SPI_MEM_OP_NO_DUMMY,
				      SPI_MEM_OP_DATA_OUT(0, NULL, 0)),
		.offset = 0,
		.length = nor->mtd.size,
	};
	struct spi_mem_op *op = &info.op_tmpl;

	/* get transfer protocols. */
	spi_nor_setup_op(nor, op, nor->write_proto);
	op->data.buswidth = spi_nor_get_protocol_data_nbits(nor->write_proto);

	if (nor->program_opcode == SPINOR_OP_AAI_WP && nor->sst_write_second)
		op->addr.nbytes = 0;

	nor->dirmap.wdesc = spi_mem_dirmap_create(nor->spi, &info);
	if (IS_ERR(nor->dirmap.wdesc))
		return PTR_ERR(nor->dirmap.wdesc);

	return 0;
}

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

	if (CONFIG_IS_ENABLED(SPI_DIRMAP)) {
		ret = spi_nor_create_read_dirmap(flash);
		if (ret)
			return ret;

		ret = spi_nor_create_write_dirmap(flash);
		if (ret)
			return ret;
	}

	if (CONFIG_IS_ENABLED(SPI_FLASH_MTD))
		ret = spi_flash_mtd_register(flash);

err_read_id:
	spi_release_bus(spi);
	return ret;
}

#if !CONFIG_IS_ENABLED(DM_SPI_FLASH)
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
	if (CONFIG_IS_ENABLED(SPI_DIRMAP)) {
		spi_mem_dirmap_destroy(flash->dirmap.wdesc);
		spi_mem_dirmap_destroy(flash->dirmap.rdesc);
	}

	if (CONFIG_IS_ENABLED(SPI_FLASH_MTD))
		spi_flash_mtd_unregister(flash);

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

	if (!mtd->erasesize ||
	    (offset % mtd->erasesize || len % mtd->erasesize)) {
		debug("SF: Erase offset/length not multiple of erase size\n");
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

int spi_flash_std_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct spi_flash *flash;

	flash = dev_get_uclass_priv(dev);
	flash->dev = dev;
	flash->spi = slave;
	return spi_flash_probe_slave(flash);
}

static int spi_flash_std_remove(struct udevice *dev)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	int ret;

	if (CONFIG_IS_ENABLED(SPI_DIRMAP)) {
		spi_mem_dirmap_destroy(flash->dirmap.wdesc);
		spi_mem_dirmap_destroy(flash->dirmap.rdesc);
	}

	ret = spi_nor_remove(flash);
	if (ret)
		return ret;

	if (CONFIG_IS_ENABLED(SPI_FLASH_MTD))
		spi_flash_mtd_unregister(flash);

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

U_BOOT_DRIVER(jedec_spi_nor) = {
	.name		= "jedec_spi_nor",
	.id		= UCLASS_SPI_FLASH,
	.of_match	= spi_flash_std_ids,
	.probe		= spi_flash_std_probe,
	.remove		= spi_flash_std_remove,
	.priv_auto	= sizeof(struct spi_nor),
	.ops		= &spi_flash_std_ops,
	.flags		= DM_FLAG_OS_PREPARE,
};

DM_DRIVER_ALIAS(jedec_spi_nor, spansion_m25p16)

#endif /* CONFIG_DM_SPI_FLASH */
