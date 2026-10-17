/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2005, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * BOARD/CPU
 */

/*
 * SERIAL
 */

/*
 * CFI Flash
 */

/*
 * MEMORY ORGANIZATION
 * -Monitor at top of sdram.
 * -The heap is placed below the monitor
 * -The stack is placed below the heap (&grows down).
 */
#define CFG_SYS_SDRAM_BASE		0xD0000000
#define CFG_SYS_SDRAM_SIZE		0x08000000

#endif /* __CONFIG_H */
