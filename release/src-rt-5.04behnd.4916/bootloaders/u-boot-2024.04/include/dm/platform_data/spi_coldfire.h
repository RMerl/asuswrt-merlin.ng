/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2018  Angelo Dureghello <angelo@sysam.it>
 */

#ifndef __spi_coldfire_h
#define __spi_coldfire_h

#define MAX_CTAR_REGS		8
#define MAX_CTAR_FIELDS		8

/*
 * struct coldfire_spi_plat - information about a coldfire spi module
 *
 * @regs_addr: base address for module registers
 * @speed_hz: default SCK frequency
 * @mode: default SPI mode
 * @num_cs: number of DSPI chipselect signals
 */
struct coldfire_spi_plat {
	fdt_addr_t regs_addr;
	uint speed_hz;
	uint mode;
	uint num_cs;
	uint ctar[MAX_CTAR_REGS][MAX_CTAR_FIELDS];
};

#endif /* __spi_coldfire_h */
