/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_LS102XA_DEVDIS_H_
#define __FSL_LS102XA_DEVDIS_H_

#include <fsl_devdis.h>

const struct devdis_table devdis_tbl[] = {
	{ "pbl", 0x0, 0x80000000 },	/* PBL	*/
	{ "esdhc", 0x0, 0x20000000 },	/* eSDHC	*/
	{ "qdma", 0x0, 0x800000 },	/* qDMA		*/
	{ "edma", 0x0, 0x400000 },	/* eDMA		*/
	{ "usb3", 0x0, 0x84000 },	/* USB3.0 controller and PHY*/
	{ "usb2", 0x0, 0x40000 },	/* USB2.0 controller	*/
	{ "sata", 0x0, 0x8000 },	/* SATA		*/
	{ "sec", 0x0, 0x200 },		/* SEC		*/
	{ "dcu", 0x0, 0x2 },		/* Display controller Unit	*/
	{ "qe", 0x0, 0x1 },		/* QUICC Engine	*/
	{ "etsec1", 0x1, 0x80000000 },	/* eTSEC1 controller	*/
	{ "etesc2", 0x1, 0x40000000 },	/* eTSEC2 controller	*/
	{ "etsec3", 0x1, 0x20000000 },	/* eTSEC3 controller	*/
	{ "pex1", 0x2, 0x80000000 },	/* PCIE controller 1	*/
	{ "pex2", 0x2, 0x40000000 },	/* PCIE controller 2	*/
	{ "duart1", 0x3, 0x20000000 },	/* DUART1	*/
	{ "duart2", 0x3, 0x10000000 },	/* DUART2	*/
	{ "qspi", 0x3, 0x8000000 },	/* QSPI		*/
	{ "ddr", 0x4, 0x80000000 },	/* DDR		*/
	{ "ocram1", 0x4, 0x8000000 },	/* OCRAM1	*/
	{ "ifc", 0x4, 0x800000 },	/* IFC		*/
	{ "gpio", 0x4, 0x400000 },	/* GPIO		*/
	{ "dbg", 0x4, 0x200000 },	/* DBG		*/
	{ "can1", 0x4, 0x80000 },	/* FlexCAN1	*/
	{ "can2_4", 0x4, 0x40000 },	/* FlexCAN2_3_4	*/
	{ "ftm2_8", 0x4, 0x20000 },	/* FlexTimer2_3_4_5_6_7_8	*/
	{ "secmon", 0x4, 0x4000 },	/* Security Monitor	*/
	{ "wdog1_2", 0x4, 0x400 },	/* WatchDog1_2	*/
	{ "i2c2_3", 0x4, 0x200 },	/* I2C2_3	*/
	{ "sai1_4", 0x4, 0x100 },	/* SAI1_2_3_4	*/
	{ "lpuart2_6", 0x4, 0x80 },	/* LPUART2_3_4_5_6	*/
	{ "dspi1_2", 0x4, 0x40 },	/* DSPI1_2	*/
	{ "asrc", 0x4, 0x20 },		/* ASRC		*/
	{ "spdif", 0x4, 0x10 },		/* SPDIF	*/
	{ "i2c1", 0x4, 0x4 },		/* I2C1		*/
	{ "lpuart1", 0x4, 0x2 },	/* LPUART1	*/
	{ "ftm1", 0x4, 0x1 },		/* FlexTimer1	*/
};

#endif
