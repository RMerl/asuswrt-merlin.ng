/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Renesas GRPEACH board
 *
 * Copyright (C) 2017-2019 Renesas Electronics
 */

#ifndef __GRPEACH_H
#define __GRPEACH_H

/* Board Clock , P1 clock frequency (XTAL=13.33MHz) */

/* Miscellaneous */

/* Internal RAM Size (RZ/A1=3M, RZ/A1M=5M, RZ/A1H=10M) */
#define CFG_SYS_SDRAM_BASE		0x20000000
#define CFG_SYS_SDRAM_SIZE		(10 * 1024 * 1024)

/* Network interface */
#define CFG_SH_ETHER_USE_PORT	0
#define CFG_SH_ETHER_PHY_ADDR	0
#define CFG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_MII
#define CFG_SH_ETHER_CACHE_WRITEBACK
#define CFG_SH_ETHER_CACHE_INVALIDATE
#define CFG_SH_ETHER_ALIGNE_SIZE	64

#endif	/* __GRPEACH_H */
