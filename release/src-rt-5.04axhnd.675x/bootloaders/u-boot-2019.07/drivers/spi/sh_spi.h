/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SH SPI driver
 *
 * Copyright (C) 2011 Renesas Solutions Corp.
 */

#ifndef __SH_SPI_H__
#define __SH_SPI_H__

#include <spi.h>

struct sh_spi_regs {
	unsigned long tbr_rbr;
	unsigned long resv1;
	unsigned long cr1;
	unsigned long resv2;
	unsigned long cr2;
	unsigned long resv3;
	unsigned long cr3;
	unsigned long resv4;
	unsigned long cr4;
};

/* CR1 */
#define SH_SPI_TBE	0x80
#define SH_SPI_TBF	0x40
#define SH_SPI_RBE	0x20
#define SH_SPI_RBF	0x10
#define SH_SPI_PFONRD	0x08
#define SH_SPI_SSDB	0x04
#define SH_SPI_SSD	0x02
#define SH_SPI_SSA	0x01

/* CR2 */
#define SH_SPI_RSTF	0x80
#define SH_SPI_LOOPBK	0x40
#define SH_SPI_CPOL	0x20
#define SH_SPI_CPHA	0x10
#define SH_SPI_L1M0	0x08

/* CR3 */
#define SH_SPI_MAX_BYTE	0xFF

/* CR4 */
#define SH_SPI_TBEI	0x80
#define SH_SPI_TBFI	0x40
#define SH_SPI_RBEI	0x20
#define SH_SPI_RBFI	0x10
#define SH_SPI_SSS1	0x08
#define SH_SPI_WPABRT	0x04
#define SH_SPI_SSS0	0x01

#define SH_SPI_FIFO_SIZE	32
#define SH_SPI_NUM_CS		4

struct sh_spi {
	struct spi_slave	slave;
	struct sh_spi_regs	*regs;
};

static inline struct sh_spi *to_sh_spi(struct spi_slave *slave)
{
	return container_of(slave, struct sh_spi, slave);
}

#endif
