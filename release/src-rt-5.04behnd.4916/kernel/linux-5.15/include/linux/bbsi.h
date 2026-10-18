/*
 * Copyright (C) 2017 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _BBSI_H
#define _BBSI_H

#include <linux/spi/spi.h>

/* Kernel API: */

int bbsi_read(struct spi_device *spi_device, u32 addr,
	      u32 *data, unsigned long nbits);

int bbsi_write(struct spi_device *spi_device, u32 addr,
	       u32 data, unsigned long nbits);

int bbsi_writebuf(struct spi_device *spi_device, u32 addr,
		  u32 *data, unsigned long len);

int bbsi_readbuf(struct spi_device *spi_device, u32 addr,
		 u32 *data, unsigned long len);

u32 bbsi_read32(struct spi_device *spi_device, u32 addr);

void bbsi_write32(struct spi_device *spi_device, u32 addr,
		  u32 data);


#endif
