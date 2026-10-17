/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 *
 * (C) Copyright 2020
 * Robert Bosch Power Tools GmbH
 *
 * Author
 *		Moses Christopher <BollavarapuMoses.Christopher@in.bosch.com>
 *
 * Copied from:
 *		arch/arm/include/asm/arch-am33xx/mem.h
 *
 * Initial Code from:
 *		Mansoor Ahamed <mansoor.ahamed@ti.com>
 *		Richard Woodruff <r-woodruff2@ti.com>
 */

#ifndef _MEM_GUARDIAN_H_
#define _MEM_GUARDIAN_H_

/*
 * GPMC settings -
 * Definitions is as per the following format
 * #define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. M_NAND - Micron Nand Flash
 * x is GPMC config registers from 1 to 7 (there will be 7 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of
 * the same.
 *
 * The following values are optimized for improving the NAND Read speed
 * They are applicable and tested for Bosch Guardian Board.
 * Read Speeds rose from 1.5MiBs to over 7.6MiBs
 *
 * Currently valid part Names are (PART):
 * M_NAND - Micron NAND
 */
#define GPMC_SIZE_256M		0x0
#define GPMC_SIZE_128M		0x8
#define GPMC_SIZE_64M		0xC
#define GPMC_SIZE_32M		0xE
#define GPMC_SIZE_16M		0xF

#define M_NAND_GPMC_CONFIG1	0x00000800
#define M_NAND_GPMC_CONFIG2	0x00030300
#define M_NAND_GPMC_CONFIG3	0x00030300
#define M_NAND_GPMC_CONFIG4	0x02000201
#define M_NAND_GPMC_CONFIG5	0x00030303
#define M_NAND_GPMC_CONFIG6	0x000000C0
#define M_NAND_GPMC_CONFIG7	0x00000008

/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS		8
/* max number of GPMC regs */
#define GPMC_MAX_REG		7

#define DBG_MPDB		6

#endif /* endif _MEM_GUARDIAN_H_ */
