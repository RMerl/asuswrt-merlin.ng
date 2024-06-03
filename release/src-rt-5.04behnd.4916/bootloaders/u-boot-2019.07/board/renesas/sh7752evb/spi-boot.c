/*
 * Copyright (C) 2012  Renesas Solutions Corp.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 */

#include <common.h>

#define CONFIG_RAM_BOOT_PHYS	CONFIG_SYS_TEXT_BASE
#define CONFIG_SPI_ADDR		0x00000000
#define CONFIG_SPI_LENGTH	CONFIG_SYS_MONITOR_LEN
#define CONFIG_RAM_BOOT		CONFIG_SYS_TEXT_BASE

#define SPIWDMADR	0xFE001018
#define SPIWDMCNTR	0xFE001020
#define SPIDMCOR	0xFE001028
#define SPIDMINTSR	0xFE001188
#define SPIDMINTMR	0xFE001190

#define SPIDMINTSR_DMEND	0x00000004

#define TBR	0xFE002000
#define RBR	0xFE002000

#define CR1	0xFE002008
#define CR2	0xFE002010
#define CR3	0xFE002018
#define CR4	0xFE002020

/* CR1 */
#define SPI_TBE		0x80
#define SPI_TBF		0x40
#define SPI_RBE		0x20
#define SPI_RBF		0x10
#define SPI_PFONRD	0x08
#define SPI_SSDB	0x04
#define SPI_SSD		0x02
#define SPI_SSA		0x01

/* CR2 */
#define SPI_RSTF	0x80
#define SPI_LOOPBK	0x40
#define SPI_CPOL	0x20
#define SPI_CPHA	0x10
#define SPI_L1M0	0x08

/* CR4 */
#define SPI_TBEI	0x80
#define SPI_TBFI	0x40
#define SPI_RBEI	0x20
#define SPI_RBFI	0x10
#define SPI_SpiS0	0x02
#define SPI_SSS		0x01

#define spi_write(val, addr)	(*(volatile unsigned long *)(addr)) = val
#define spi_read(addr)		(*(volatile unsigned long *)(addr))

/* M25P80 */
#define M25_READ	0x03

#define __uses_spiboot2	__attribute__((section(".spiboot2.text")))
static void __uses_spiboot2 spi_reset(void)
{
	int timeout = 0x00100000;

	/* Make sure the last transaction is finalized */
	spi_write(0x00, CR3);
	spi_write(0x02, CR1);
	while (!(spi_read(CR4) & SPI_SpiS0)) {
		if (timeout-- < 0)
			break;
	}
	spi_write(0x00, CR1);

	spi_write(spi_read(CR2) | SPI_RSTF, CR2);	/* fifo reset */
	spi_write(spi_read(CR2) & ~SPI_RSTF, CR2);

	spi_write(0, SPIDMCOR);
}

static void __uses_spiboot2 spi_read_flash(void *buf, unsigned long addr,
					   unsigned long len)
{
	spi_write(M25_READ, TBR);
	spi_write((addr >> 16) & 0xFF, TBR);
	spi_write((addr >> 8) & 0xFF, TBR);
	spi_write(addr & 0xFF, TBR);

	spi_write(SPIDMINTSR_DMEND, SPIDMINTSR);
	spi_write((unsigned long)buf, SPIWDMADR);
	spi_write(len & 0xFFFFFFE0, SPIWDMCNTR);
	spi_write(1, SPIDMCOR);

	spi_write(0xff, CR3);
	spi_write(spi_read(CR1) | SPI_SSDB, CR1);
	spi_write(spi_read(CR1) | SPI_SSA, CR1);

	while (!(spi_read(SPIDMINTSR) & SPIDMINTSR_DMEND))
		;

	/* Nagate SP0-SS0 */
	spi_write(0, CR1);
}

void __uses_spiboot2 spiboot_main(void)
{
	void (*_start)(void) = (void *)CONFIG_SYS_TEXT_BASE;

	spi_reset();
	spi_read_flash((void *)CONFIG_RAM_BOOT_PHYS, CONFIG_SPI_ADDR,
			CONFIG_SPI_LENGTH);

	_start();
}
