/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * BCM4710 address space map and definitions
 *
 * Think twice before adding to this file, this is not the kitchen sink
 * These definitions are not guaranteed for all 47xx chips, only the 4710
 *
 * Copyright (C) 2000 Broadcom Corporation
 * $Id: bcm4710.h,v 1.1 2001/10/31 18:49:25 mpl Exp $
 */

#ifndef _bcm4710_h_
#define _bcm4710_h_

/* Address map */
#define BCM4710_SDRAM		0x00000000	/* Physical SDRAM */
#define BCM4710_PCI_MEM		0x08000000	/* Host Mode PCI memory access space (64 MB) */
#define BCM4710_PCI_CFG		0x0c000000	/* Host Mode PCI configuration space (64 MB) */
#define BCM4710_PCI_DMA		0x40000000	/* Client Mode PCI memory access space (1 GB) */
#define	BCM4710_SDRAM_SWAPPED	0x10000000	/* Byteswapped Physical SDRAM */
#define BCM4710_ENUM		0x18000000	/* Beginning of core enumeration space */

/* Core register space */
#define BCM4710_REG_SDRAM	0x18000000	/* SDRAM core registers */
#define BCM4710_REG_ILINE20	0x18001000	/* InsideLine20 core registers */
#define BCM4710_REG_EMAC0	0x18002000	/* Ethernet MAC 0 core registers */
#define BCM4710_REG_CODEC	0x18003000	/* Codec core registers */
#define BCM4710_REG_USB		0x18004000	/* USB core registers */
#define BCM4710_REG_PCI		0x18005000	/* PCI core registers */
#define BCM4710_REG_MIPS	0x18006000	/* MIPS core registers */
#define BCM4710_REG_EXTIF	0x18007000	/* External Interface core registers */
#define BCM4710_REG_EMAC1	0x18008000	/* Ethernet MAC 1 core registers */

#define	BCM4710_EXTIF		0x1f000000	/* External Interface base address */
#define	BCM4710_EJTAG		0xff200000	/* MIPS EJTAG space (2M) */

#define	BCM4710_UART		(BCM4710_REG_EXTIF + 0x00000300)

#define	BCM4710_EUART		(BCM4710_EXTIF + 0x00800000)
#define	BCM4710_LED		(BCM4710_EXTIF + 0x00900000)

#ifdef	CONFIG_VSIM
#define	BCM4710_TRACE(trval)        do { *((int *)0xa0002ff8) = (trval); } while (0)
#else
#define	BCM4710_TRACE(trval)        do { *((unsigned char *)KSEG1ADDR(BCM4710_LED)) = (trval); \
					 *((int *)0xa0002ff8) = (trval); } while (0)
#endif


#endif /* _bcm4710_h_ */
