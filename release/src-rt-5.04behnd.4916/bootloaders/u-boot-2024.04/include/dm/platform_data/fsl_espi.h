/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef __fsl_espi_h
#define __fsl_espi_h

struct fsl_espi_plat {
	uint flags;
	uint speed_hz;
	uint num_chipselect;
	fdt_addr_t regs_addr;
};

#endif /* __fsl_espi_h */
