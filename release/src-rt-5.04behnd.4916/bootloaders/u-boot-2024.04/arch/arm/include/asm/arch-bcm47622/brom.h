/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _47622_BROM_H
#define _47622_BROM_H

#define BROM_GEN_BASE          0xff800600

/*
 * BROM_GEN Register Definition .
 */
typedef struct BromGen_ {
#define	BROM_GEN_SPI_SLV_UNLOCK_MASK	0x1
#define	BROM_GEN_SPI_SLV_UNLOCK_SHIFT	0x2
#define BROM_GEN_TAG_UNLOCK_MASK	0x1
#define BROM_GEN_TAG_UNLOCK_SHIFT	0x1
		uint32_t secBootCfg;
		uint32_t bromCrcRevIdLow;
		uint32_t bromCrcRevIdHigh;
} BromGen;

#define BROM_GEN ((volatile BromGen * const) BROM_GEN_BASE)
#define BROM_GEN_JTAG_UNLOCK do {BROM_GEN->secBootCfg |= ((BROM_GEN_SPI_SLV_UNLOCK_MASK<<BROM_GEN_SPI_SLV_UNLOCK_SHIFT) | \
						(BROM_GEN_TAG_UNLOCK_MASK<<BROM_GEN_TAG_UNLOCK_SHIFT)); }while(0) 

#endif
