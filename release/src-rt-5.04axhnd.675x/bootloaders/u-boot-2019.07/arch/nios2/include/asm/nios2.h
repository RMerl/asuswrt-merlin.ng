/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#ifndef __ASM_NIOS2_H__
#define __ASM_NIOS2_H__

/*------------------------------------------------------------------------
 * Control registers -- use with wrctl() & rdctl()
 *----------------------------------------------------------------------*/
#define CTL_STATUS	0		/* Processor status reg		*/
#define CTL_ESTATUS	1		/* Exception status reg		*/
#define CTL_BSTATUS	2		/* Break status reg		*/
#define CTL_IENABLE	3		/* Interrut enable reg		*/
#define CTL_IPENDING	4		/* Interrut pending reg		*/

/*------------------------------------------------------------------------
 * Access to control regs
 *----------------------------------------------------------------------*/

#define rdctl(reg) __builtin_rdctl(reg)
#define wrctl(reg, val) __builtin_wrctl(reg, val)

/*------------------------------------------------------------------------
 * Control reg bit masks
 *----------------------------------------------------------------------*/
#define STATUS_IE	(1<<0)		/* Interrupt enable		*/
#define STATUS_U	(1<<1)		/* User-mode			*/

/*------------------------------------------------------------------------
 * Bit-31 Cache bypass -- only valid for data access. When data cache
 * is not implemented, bit 31 is ignored for compatibility.
 *----------------------------------------------------------------------*/
#define CACHE_BYPASS(a) ((a) | 0x80000000)
#define CACHE_NO_BYPASS(a) ((a) & ~0x80000000)

#endif /* __ASM_NIOS2_H__ */
