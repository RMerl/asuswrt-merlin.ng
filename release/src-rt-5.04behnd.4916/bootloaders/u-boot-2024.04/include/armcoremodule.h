/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2005
 * ARM Ltd.
 * Peter Pearse, <Peter.Pearse@arm.com>
 * Configuration for ARM Core Modules.
 * No standalonw port yet available
 * - this file is included by both integratorap.h & integratorcp.h
 */

#ifndef __ARMCOREMODULE_H
#define __ARMCOREMODULE_H

#define CM_BASE			0x10000000

/* CM registers common to all CMs */
/* Note that observed values after reboot into the ARM Boot Monitor
   have been used as defaults, rather than the POR values */
#define OS_CTRL			0x0000000C
#define CMMASK_REMAP		0x00000005	/* set remap & led           */
#define CMMASK_RESET		0x00000008
#define OS_LOCK			0x00000014
#define CMVAL_LOCK1		0x0000A000	/* locking value             */
#define CMVAL_LOCK2		0x0000005F	/* locking value             */
#define CMVAL_UNLOCK		0x00000000	/* any value != CM_LOCKVAL   */
#define OS_SDRAM		0x00000020
#define OS_INIT			0x00000024
#define CMMASK_MAP_SIMPLE	0xFFFDFFFF	/* simple mapping */
#define CMMASK_TCRAM_DISABLE	0xFFFEFFFF	/* TCRAM disabled */
#define CMMASK_LOWVEC		0x00000000	/* vectors @ 0x00000000 */
#define CMMASK_LE		0xFFFFFFF7	/* little endian */
#define CMMASK_CMxx6_COMMON	0x00000013      /* Common value for CMxx6 */
						/* - observed reset value of */
						/*   CM926EJ-S */
						/*   CM1136-EJ-S */

#define OS_SPD		0x00000100	/* The SDRAM SPD data is copied here */

#endif /* __ARMCOREMODULE_H */
