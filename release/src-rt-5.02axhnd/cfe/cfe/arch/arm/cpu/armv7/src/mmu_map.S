/*  *********************************************************************
    *
    <:copyright-BRCM:2012:proprietary:standard
    
       Copyright (c) 2012 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :> 
    ********************************************************************* */
	
#include <arm.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "bcm_map.h"
#include "mmu_map.h"

/* CPUCFG_MMU_TABLE_SIZE for an MMU table size*/

/* VA to PA page addr calculation (4K small pages) 
*    L2TBL_PA = L1TBL_PA[31:12] + VA[31:20]*4 
*     PAGE_PA = L2TBL_PA[31:10] + VA[19:12] ]
*/

/* 
*	r0 - table paddr
*	r1 - table size (bytes)
*	min size 16 bytes	
*/
FUNC(mmu_init_table)
        add	r1, r1, r0
        mov	r2, #0
1:
	str	r2,[r0]
	add	r0,r0,#4
        cmp     r0,r1
        blt     1b

	mov	pc, lr
END(mmu_init_table)

FUNC(mmu_set_ttbr)
        // Set the access control to client so AP is checked with tlb entry	
        ldr     r1, =0x55555555
        mcr     p15, 0, r1, c3, c0, 0 // Write Domain Access Control Register

        // Set Translation Table base address. r0 must be preserved
        mcr      p15, 0, r0, c2, c0, 0 // Write Translation Table Base Register 0
        isb
	dsb
	mov	pc,lr
END(mmu_set_ttbr)

FUNC(mmu_invalidate_tlb)
	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	isb
	dsb
	mov	pc,lr
END(mmu_invalidate_tlb)
	
#if defined(BTRM_USE_PAGEMAP)

/*
	
       mmu_set_page uses L2 1K Page. Sets number (PG_RNG) entries in L1 table
	updates corresponding to VA L1 entry which points to L2 table 
	r0 - L1 table paddr
	r1 - L2 table paddr
        r2 - page virtual address start
        r3 - number of pages 
        r4 - page memory attributes
	r5 - page physical address start
	r6 - page table attributes
	Registers trashed: r0-r8
	Note - there are no calls to nested subroutinies. LR is not preserved 
*/
#define L1TBL		r0
#define L2TBL		r1
#define PG_VA		r2
#define PG_RNG		r3
#define PG_ATTR		r4
#define PG_PA		r5
#define PG_TBL_ATTR	r6
#define L2TBL_LEN	r8
/*
*/
FUNC(mmu_set_page)

        /* Prepare  L1 table entry address  */
        add	L1TBL, L1TBL, PG_VA, lsr #18 /*  va[31-20] 0xfff00000 ->  03ffc*/
	bfc	L1TBL, #0, #2

	/* Clear 20 msb of the page table attribute */
	bfc	PG_ATTR, #12, #20


	/* Converting va[19:12] to the page table index*/
	lsr	PG_VA, PG_VA, #12
	bfc	PG_VA, #8, #24
	/* adjusting VA range to the max table index not exceeding 0xff*/
	add	PG_RNG, PG_VA, PG_RNG
	cmp	PG_RNG, L2TBL_LEN /* VA[19:12] 256*4*/
	blt	1f
	mov	PG_RNG, L2TBL_LEN
1:
        /* set L2 entries */
	/*4K Page offset [31:12]*/
	lsr	PG_PA, PG_PA, #12
2:
	/*Set page attributes and page offset*/
	orr	r7, PG_ATTR, PG_PA, lsl #12
	/* set to page table PA at VA[19:12] index*/
	str     r7, [L2TBL, PG_VA, lsl #2]
	/* Increment to consecutive va index 
		and pa page addresses */
	add	PG_PA, PG_PA, #1
	add	PG_VA, PG_VA, #1
        cmp     PG_VA, PG_RNG 
        bne     2b

	/* set L1 entry */
	/* clear 10 lsb of the L2 PA table address */
        bfc	L2TBL, #0, #10
	/* Clear 22 msb of the page table attributes*/
	bfc	PG_TBL_ATTR, #10, #22
	/* set */
	orr	L2TBL, L2TBL, PG_TBL_ATTR 
	str	L2TBL, [L1TBL] /* here is adjusting to word aligned table index (lsl (20-2)) */
	mov	pc,lr
END(mmu_set_page)

#endif /*BTRM_USE_PAGEMAP*/

/*
	map_set_scn - sets number (SCN_RNG) of L1 table entries. Operates on 1 MB sections
	r0 - L1 table paddr
        r1 - virtual address start
        r2 - address range in Megabytes 
        r3 - Sections' attributes
	r4 - mapped physical start address
	Table max size 16K (4K*4) 
*/

#define	WORD_SIZE	4  /* bytes */
#define	WORD_BITLEN	32 /*bitlen or shift*/
#define	WORD_ALGN	2  /* word alignment shift*/
#define	SCN_BITLEN       20 /*bitlen or shift */

#define L1TBL_PA	r0 /* */
#define SCN_VA		r1
#define SCN_RNG		r2
#define SCN_ATTR	r3
#define SCN_PA		r4
#define SCN_ENTRY	r5
#define L1TBL_LEN	r6
/* NN (not nested)  - meaning there are no calls to subroutine. LR is not preserved 
	registers r0-r6 are not preserved and must not be relied upon
*/
FUNC(mmu_set_scn)
	mov	SCN_ENTRY,#0
	/* adjust virt/phys addresses for the loop increment */
	lsr	SCN_VA, SCN_VA, #(SCN_BITLEN)
	lsr	SCN_PA, SCN_PA, #(SCN_BITLEN)
	/* clear 12 msb of the section attribute */
	bfc	SCN_ATTR, #SCN_BITLEN, #(WORD_BITLEN-SCN_BITLEN)
	/* set max  range not exceeding 0xfff */
	add	SCN_RNG, SCN_VA, SCN_RNG
	cmp	SCN_RNG, L1TBL_LEN 
	blt	1f
	mov	SCN_RNG, L1TBL_LEN
1:
	orr	SCN_ENTRY, SCN_ATTR, SCN_PA, lsl #(SCN_BITLEN)
	str     SCN_ENTRY, [L1TBL_PA, SCN_VA, lsl #(WORD_ALGN)]
	add	SCN_PA, SCN_PA, #1
	add	SCN_VA, SCN_VA, #1
        cmp     SCN_VA, SCN_RNG
        bne     1b

	mov	pc,lr
END(mmu_set_scn)



