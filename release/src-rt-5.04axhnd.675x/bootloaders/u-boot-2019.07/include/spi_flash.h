/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Common SPI flash Interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 */

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include <dm.h>	/* Because we dereference struct udevice here */
#include <linux/types.h>
#include <linux/mtd/spi-nor.h>

/* by default ENV use the same parameters than SF command */
#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS	CONFIG_SF_DEFAULT_BUS
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS	CONFIG_SF_DEFAULT_CS
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED
#endif
#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE	CONFIG_SF_DEFAULT_MODE
#endif

struct spi_slave;

struct dm_spi_flash_ops {
	int (*read)(struct udevice *dev, u32 offset, size_t len, void *buf);
	int (*write)(struct udevice *dev, u32 offset, size_t len,
		     const void *buf);
	int (*erase)(struct udevice *dev, u32 offset, size_t len);
	/**
	 * get_sw_write_prot() - Check state of software write-protect feature
	 *
	 * SPI flash chips can lock a region of the flash defined by a
	 * 'protected area'. This function checks if this protected area is
	 * defined.
	 *
	 * @dev:	SPI flash device
	 * @return 0 if no region is write-protected, 1 if a region is
	 *	write-protected, -ENOSYS if the driver does not implement this,
	 *	other -ve value on error
	 */
	int (*get_sw_write_prot)(struct udevice *dev);
};

/* Access the serial operations for a device */
#define sf_get_ops(dev) ((struct dm_spi_flash_ops *)(dev)->driver->ops)

#ifdef CONFIG_DM_SPI_FLASH
/**
 * spi_flash_read_dm() - Read data from SPI flash
 *
 * @dev:	SPI flash device
 * @offset:	Offset into device in bytes to read from
 * @len:	Number of bytes to read
 * @buf:	Buffer to put the data that is read
 * @return 0 if OK, -ve on error
 */
int spi_flash_read_dm(struct udevice *dev, u32 offset, size_t len, void *buf);

/**
 * spi_flash_write_dm() - Write data to SPI flash
 *
 * @dev:	SPI flash device
 * @offset:	Offset into device in bytes to write to
 * @len:	Number of bytes to write
 * @buf:	Buffer containing bytes to write
 * @return 0 if OK, -ve on error
 */
int spi_flash_write_dm(struct udevice *dev, u32 offset, size_t len,
		       const void *buf);

/**
 * spi_flash_erase_dm() - Erase blocks of the SPI flash
 *
 * Note that @len must be a muiltiple of the flash sector size.
 *
 * @dev:	SPI flash device
 * @offset:	Offset into device in bytes to start erasing
 * @len:	Number of bytes to erase
 * @return 0 if OK, -ve on error
 */
int spi_flash_erase_dm(struct udevice *dev, u32 offset, size_t len);

/**
 * spl_flash_get_sw_write_prot() - Check state of software write-protect feature
 *
 * SPI flash chips can lock a region of the flash defined by a
 * 'protected area'. This function checks if this protected area is
 * defined.
 *
 * @dev:	SPI flash device
 * @return 0 if no region is write-protected, 1 if a region is
 *	write-protected, -ENOSYS if the driver does not implement this,
 *	other -ve value on error
 */
int spl_flash_get_sw_write_prot(struct udevice *dev);

int spi_flash_probe_bus_cs(unsigned int busnum, unsigned int cs,
			   unsigned int max_hz, unsigned int spi_mode,
			   struct udevice **devp);

/* Compatibility function - this is the old U-Boot API */
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int spi_mode);

/* Compatibility function - this is the old U-Boot API */
void spi_flash_free(struct spi_flash *flash);

static inline int spi_flash_read(struct spi_flash *flash, u32 offset,
				 size_t len, void *buf)
{
	return spi_flash_read_dm(flash->dev, offset, len, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
				  size_t len, const void *buf)
{
	return spi_flash_write_dm(flash->dev, offset, len, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
				  size_t len)
{
	return spi_flash_erase_dm(flash->dev, offset, len);
}

struct sandbox_state;

int sandbox_sf_bind_emul(struct sandbox_state *state, int busnum, int cs,
			 struct udevice *bus, ofnode node, const char *spec);

void sandbox_sf_unbind_emul(struct sandbox_state *state, int busnum, int cs);

#else
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);

void spi_flash_free(struct spi_flash *flash);

static inline int spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf)
{
	struct mtd_info *mtd = &flash->mtd;
	size_t retlen;

	return mtd->_read(mtd, offset, len, &retlen, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	struct mtd_info *mtd = &flash->mtd;
	size_t retlen;

	return mtd->_write(mtd, offset, len, &retlen, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
		size_t len)
{
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
#endif

static inline int spi_flash_protect(struct spi_flash *flash, u32 ofs, u32 len,
					bool prot)
{
	if (!flash->flash_lock || !flash->flash_unlock)
		return -EOPNOTSUPP;

	if (prot)
		return flash->flash_lock(flash, ofs, len);
	else
		return flash->flash_unlock(flash, ofs, len);
}

#endif /* _SPI_FLASH_H_ */
