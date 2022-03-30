/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Quentin Schulz, Bootlin, quentin.schulz@bootlin.com
 *
 * Structure for use with U_BOOT_DEVICE for pl022 SPI devices or to use
 * in ofdata_to_platdata.
 */

#ifndef __spi_pl022_h
#define __spi_pl022_h

#include <fdtdec.h>

struct pl022_spi_pdata {
	fdt_addr_t addr;
	fdt_size_t size;
	unsigned int freq;
};

#endif /* __spi_pl022_h */
