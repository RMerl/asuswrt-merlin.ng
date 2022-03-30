// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013  Renesas Solutions Corp.
 */

#include <common.h>

#define CONFIG_SPI_ADDR		0x00000000
#define PHYADDR(_addr)		((_addr & 0x1fffffff) | 0x40000000)
#define CONFIG_RAM_BOOT_PHYS	PHYADDR(CONFIG_SYS_TEXT_BASE)

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
#define CR7	0xFE002038
#define CR8	0xFE002040

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

/* CR7 */
#define CR7_IDX_OR12	0x12
#define OR12_ADDR32	0x00000001

#define spi_write(val, addr)	(*(volatile unsigned long *)(addr)) = val
#define spi_read(addr)		(*(volatile unsigned long *)(addr))

/* M25P80 */
#define M25_READ	0x03
#define M25_READ_4BYTE	0x13

extern void bss_start(void);

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
	spi_write(CR7_IDX_OR12, CR7);
	if (spi_read(CR8) & OR12_ADDR32) {
		/* 4-bytes address mode */
		spi_write(M25_READ_4BYTE, TBR);
		spi_write((addr >> 24) & 0xFF, TBR);	/* ADDR31-24 */
	} else {
		/* 3-bytes address mode */
		spi_write(M25_READ, TBR);
	}
	spi_write((addr >> 16) & 0xFF, TBR);	/* ADDR23-16 */
	spi_write((addr >> 8) & 0xFF, TBR);	/* ADDR15-8 */
	spi_write(addr & 0xFF, TBR);		/* ADDR7-0 */

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
	/*
	 * This code rounds len up for SPIWDMCNTR. We should set it to 0 in
	 * lower 5-bits.
	 */
	void (*_start)(void) = (void *)CONFIG_SYS_TEXT_BASE;
	volatile unsigned long len = (bss_start - _start + 31) & 0xffffffe0;

	spi_reset();
	spi_read_flash((void *)CONFIG_RAM_BOOT_PHYS, CONFIG_SPI_ADDR, len);

	_start();
}
