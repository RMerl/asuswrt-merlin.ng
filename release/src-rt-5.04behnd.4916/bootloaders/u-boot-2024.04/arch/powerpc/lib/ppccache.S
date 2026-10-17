/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 1998  Dan Malek <dmalek@jlc.net>
 * Copyright (C) 1999  Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 * Copyright (C) 2000, 2001,2002 Wolfgang Denk <wd@denx.de>
 * Copyright Freescale Semiconductor, Inc. 2004, 2006.
 */

#include <config.h>
#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbf */
/* Description:	 Data Cache block flush */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbf
ppcDcbf:
	dcbf	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbi */
/* Description:	 Data Cache block Invalidate */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbi
ppcDcbi:
	dcbi	r0,r3
	blr

/*--------------------------------------------------------------------------
 * Function:	 ppcDcbz
 * Description:	 Data Cache block zero.
 * Input:	 r3 = effective address
 * Output:	 none.
 *-------------------------------------------------------------------------- */

	.globl	ppcDcbz
ppcDcbz:
	dcbz	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcSync */
/* Description:	 Processor Synchronize */
/* Input:	 none. */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcSync
ppcSync:
	sync
	blr

/*
 * Write any modified data cache blocks out to memory and invalidate them.
 * Does not invalidate the corresponding instruction cache blocks.
 *
 * flush_dcache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(flush_dcache_range)
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	li	r5,L1_CACHE_BYTES-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,L1_CACHE_SHIFT
	beqlr
	mtctr	r4

1:	dcbf	0,r3
	addi	r3,r3,L1_CACHE_BYTES
	bdnz	1b
	sync				/* wait for dcbst's to get to ram */
#endif
	blr

/*
 * Like above, but invalidate the D-cache.  This is used by the 8xx
 * to invalidate the cache so the PPC core doesn't get stale data
 * from the CPM (no cache snooping here :-).
 *
 * invalidate_dcache_range(unsigned long start, unsigned long stop)
 */
_GLOBAL(invalidate_dcache_range)
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	li	r5,L1_CACHE_BYTES-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,L1_CACHE_SHIFT
	beqlr
	mtctr	r4

	sync
1:	dcbi	0,r3
	addi	r3,r3,L1_CACHE_BYTES
	bdnz	1b
	sync				/* wait for dcbi's to get to ram */
#endif
	blr
