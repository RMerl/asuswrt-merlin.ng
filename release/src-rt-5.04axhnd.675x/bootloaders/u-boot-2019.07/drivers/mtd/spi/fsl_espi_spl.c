// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <spi_flash.h>
#include <malloc.h>

#define ESPI_BOOT_IMAGE_SIZE	0x48
#define ESPI_BOOT_IMAGE_ADDR	0x50
#define CONFIG_CFG_DATA_SECTOR	0

void fsl_spi_spl_load_image(uint32_t offs, unsigned int size, void *vdst)
{
	struct spi_flash *flash;

	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (flash == NULL) {
		puts("\nspi_flash_probe failed");
		hang();
	}

	spi_flash_read(flash, offs, size, vdst);
}

/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
void fsl_spi_boot(void)
{
	void (*uboot)(void) __noreturn;
	u32 offset, code_len, copy_len = 0;
#ifndef CONFIG_FSL_CORENET
	unsigned char *buf = NULL;
#endif
	struct spi_flash *flash;

	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (flash == NULL) {
		puts("\nspi_flash_probe failed");
		hang();
	}

#ifdef CONFIG_FSL_CORENET
	offset = CONFIG_SYS_SPI_FLASH_U_BOOT_OFFS;
	code_len = CONFIG_SYS_SPI_FLASH_U_BOOT_SIZE;
#else
	/*
	* Load U-Boot image from SPI flash into RAM
	*/
	buf = malloc(flash->page_size);
	if (buf == NULL) {
		puts("\nmalloc failed");
		hang();
	}
	memset(buf, 0, flash->page_size);

	spi_flash_read(flash, CONFIG_CFG_DATA_SECTOR,
		       flash->page_size, (void *)buf);
	offset = *(u32 *)(buf + ESPI_BOOT_IMAGE_ADDR);
	/* Skip spl code */
	offset += CONFIG_SYS_SPI_FLASH_U_BOOT_OFFS;
	/* Get the code size from offset 0x48 */
	code_len = *(u32 *)(buf + ESPI_BOOT_IMAGE_SIZE);
	/* Skip spl code */
	code_len = code_len - CONFIG_SPL_MAX_SIZE;
#endif
	/* copy code to DDR */
	printf("Loading second stage boot loader ");
	while (copy_len <= code_len) {
		spi_flash_read(flash, offset + copy_len, 0x2000,
			       (void *)(CONFIG_SYS_SPI_FLASH_U_BOOT_DST
			       + copy_len));
		copy_len = copy_len + 0x2000;
		putc('.');
	}

	/*
	* Jump to U-Boot image
	*/
	flush_cache(CONFIG_SYS_SPI_FLASH_U_BOOT_DST, code_len);
	uboot = (void *)CONFIG_SYS_SPI_FLASH_U_BOOT_START;
	(*uboot)();
}
