/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author
 *		Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Initial Code from:
 *		Richard Woodruff <r-woodruff2@ti.com>
 */

#ifndef _MEM_H_
#define _MEM_H_

/*
 * GPMC settings -
 * Definitions is as per the following format
 * #define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. STNOR - Intel Strata Flash
 * x is GPMC config registers from 1 to 6 (there will be 6 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of
 * the same. if values are independent of the board, this definition will be
 * present in this file if values are dependent on the board, then this should
 * go into corresponding mem-boardName.h file
 *
 * Currently valid part Names are (PART):
 * M_NAND - Micron NAND
 * STNOR - STMicrolelctronics M29W128GL
 */
#define GPMC_SIZE_256M		0x0
#define GPMC_SIZE_128M		0x8
#define GPMC_SIZE_64M		0xC
#define GPMC_SIZE_32M		0xE
#define GPMC_SIZE_16M		0xF

#define M_NAND_GPMC_CONFIG1	0x00000800
#define M_NAND_GPMC_CONFIG2	0x001e1e00
#define M_NAND_GPMC_CONFIG3	0x001e1e00
#define M_NAND_GPMC_CONFIG4	0x16051807
#define M_NAND_GPMC_CONFIG5	0x00151e1e
#define M_NAND_GPMC_CONFIG6	0x16000f80
#define M_NAND_GPMC_CONFIG7	0x00000008

#define STNOR_GPMC_CONFIG1	0x00001200
#define STNOR_GPMC_CONFIG2	0x00101000
#define STNOR_GPMC_CONFIG3	0x00030301
#define STNOR_GPMC_CONFIG4	0x10041004
#define STNOR_GPMC_CONFIG5	0x000C1010
#define STNOR_GPMC_CONFIG6	0x08070280
#define STNOR_GPMC_CONFIG7	0x00000F48

/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS		8
/* max number of GPMC regs */
#define GPMC_MAX_REG		7

#define DBG_MPDB		6

#endif /* endif _MEM_H_ */
