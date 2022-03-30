/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *
 * (C) Copyright 2014 Freescale Semiconductor, Inc
 */

#ifndef _IMX_THERMAL_H_
#define _IMX_THERMAL_H_

/* CPU Temperature Grades */
#define TEMP_COMMERCIAL         0
#define TEMP_EXTCOMMERCIAL      1
#define TEMP_INDUSTRIAL         2
#define TEMP_AUTOMOTIVE         3

struct imx_thermal_plat {
	void *regs;
	int fuse_bank;
	int fuse_word;
};

#endif	/* _IMX_THERMAL_H_ */
