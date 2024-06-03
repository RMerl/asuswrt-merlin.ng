/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2014 Google Inc.
 *
 * This file is from coreboot soc/intel/broadwell/include/soc/spi.h
 */

#ifndef _BROADWELL_SPI_H_
#define _BROADWELL_SPI_H_

/*
 * SPI Opcode Menu setup for SPIBAR lockdown
 * should support most common flash chips.
 */

#define SPIBAR_OFFSET		0x3800
#define SPI_REG(x)		(RCB_REG(SPIBAR_OFFSET + (x)))

/* Reigsters within the SPIBAR */
#define SPIBAR_SSFC		0x91
#define SPIBAR_FDOC		0xb0
#define SPIBAR_FDOD		0xb4

#define SPIBAR_PREOP		0x94
#define SPIBAR_OPTYPE		0x96
#define SPIBAR_OPMENU_LOWER	0x98
#define SPIBAR_OPMENU_UPPER	0x9c

#define SPI_OPMENU_0 0x01 /* WRSR: Write Status Register */
#define SPI_OPTYPE_0 0x01 /* Write, no address */

#define SPI_OPMENU_1 0x02 /* BYPR: Byte Program */
#define SPI_OPTYPE_1 0x03 /* Write, address required */

#define SPI_OPMENU_2 0x03 /* READ: Read Data */
#define SPI_OPTYPE_2 0x02 /* Read, address required */

#define SPI_OPMENU_3 0x05 /* RDSR: Read Status Register */
#define SPI_OPTYPE_3 0x00 /* Read, no address */

#define SPI_OPMENU_4 0x20 /* SE20: Sector Erase 0x20 */
#define SPI_OPTYPE_4 0x03 /* Write, address required */

#define SPI_OPMENU_5 0x9f /* RDID: Read ID */
#define SPI_OPTYPE_5 0x00 /* Read, no address */

#define SPI_OPMENU_6 0xd8 /* BED8: Block Erase 0xd8 */
#define SPI_OPTYPE_6 0x03 /* Write, address required */

#define SPI_OPMENU_7 0x0b /* FAST: Fast Read */
#define SPI_OPTYPE_7 0x02 /* Read, address required */

#define SPI_OPMENU_UPPER ((SPI_OPMENU_7 << 24) | (SPI_OPMENU_6 << 16) | \
			  (SPI_OPMENU_5 << 8) | SPI_OPMENU_4)
#define SPI_OPMENU_LOWER ((SPI_OPMENU_3 << 24) | (SPI_OPMENU_2 << 16) | \
			  (SPI_OPMENU_1 << 8) | SPI_OPMENU_0)

#define SPI_OPTYPE ((SPI_OPTYPE_7 << 14) | (SPI_OPTYPE_6 << 12) | \
		    (SPI_OPTYPE_5 << 10) | (SPI_OPTYPE_4 << 8) |  \
		    (SPI_OPTYPE_3 << 6) | (SPI_OPTYPE_2 << 4) |	  \
		    (SPI_OPTYPE_1 << 2) | (SPI_OPTYPE_0))

#define SPI_OPPREFIX ((0x50 << 8) | 0x06) /* EWSR and WREN */

#define SPIBAR_HSFS                 0x04   /* SPI hardware sequence status */
#define  SPIBAR_HSFS_FLOCKDN        (1 << 15)/* Flash Configuration Lock-Down */
#define  SPIBAR_HSFS_SCIP           (1 << 5) /* SPI Cycle In Progress */
#define  SPIBAR_HSFS_AEL            (1 << 2) /* SPI Access Error Log */
#define  SPIBAR_HSFS_FCERR          (1 << 1) /* SPI Flash Cycle Error */
#define  SPIBAR_HSFS_FDONE          (1 << 0) /* SPI Flash Cycle Done */
#define SPIBAR_HSFC                 0x06   /* SPI hardware sequence control */
#define  SPIBAR_HSFC_BYTE_COUNT(c)  (((c - 1) & 0x3f) << 8)
#define  SPIBAR_HSFC_CYCLE_READ     (0 << 1) /* Read cycle */
#define  SPIBAR_HSFC_CYCLE_WRITE    (2 << 1) /* Write cycle */
#define  SPIBAR_HSFC_CYCLE_ERASE    (3 << 1) /* Erase cycle */
#define  SPIBAR_HSFC_GO             (1 << 0) /* GO: start SPI transaction */
#define SPIBAR_FADDR                0x08   /* SPI flash address */
#define SPIBAR_FDATA(n)             (0x10 + (4 * n)) /* SPI flash data */
#define SPIBAR_SSFS                 0x90
#define  SPIBAR_SSFS_ERROR          (1 << 3)
#define  SPIBAR_SSFS_DONE           (1 << 2)
#define SPIBAR_SSFC                 0x91
#define  SPIBAR_SSFC_DATA           (1 << 14)
#define  SPIBAR_SSFC_GO             (1 << 1)

#endif
