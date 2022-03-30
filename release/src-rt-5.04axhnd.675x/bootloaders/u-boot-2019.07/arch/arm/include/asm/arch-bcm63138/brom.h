/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63138_BROM_H
#define _63138_BROM_H
#define BROM_SEC_BASE 0xfffeb614	   	 

/*
 * BROM_GEN Register Definition .
 */
typedef struct BromSec_ {
#define	BROM_GEN_JTAG_SPI_SLV_UNLOCK_MASK	0x1
#define	BROM_GEN_JTAG_SPI_SLV_UNLOCK_SHIFT	0x1
		uint32_t secBootCfg;
} BromSec;

#define BROM_GEN ((volatile BromSec * const) BROM_SEC_BASE)
#define BROM_GEN_JTAG_UNLOCK do {BROM_GEN->secBootCfg |= (BROM_GEN_JTAGE_SPI_SLV_UNLOCK_MASK<<BROM_GEN_JTAG_SPI_SLV_UNLOCK_SHIFT) ; }while(0) 

#endif
