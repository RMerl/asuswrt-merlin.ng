/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include <asm_macros.S>
#include <platform_def.h>
#include <cpu_macros.S>

	.globl	a9_mmu_set_ttbr
	.globl	a9_mmu_invalidate_tlb
	.globl	a9_mmu_set_scn
	.globl	a9_mmu_enable
	.globl	a9_l1cache_enable_d
	.globl	a9_l1cache_enable_i
	.globl	a9_gic_secure_init
	.globl	a9_l1cache_inval_d

func a9_mmu_set_ttbr
	push	{r0-r12,r14}
	// Set the access control to client so AP is checked with tlb entry	
	ldr	r1, =0x55555555
	mcr	p15, 0, r1, c3, c0, 0 // Write Domain Access Control Register

	// Set Translation Table base address. r0 must be preserved
	mcr	p15, 0, r0, c2, c0, 0 // Write Translation Table Base Register 0
	isb
	dsb
	pop	{r0-r12,r14}
	mov	pc,lr
endfunc a9_mmu_set_ttbr

func a9_mmu_invalidate_tlb
	push	{r0-r12,r14}
	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	isb
	dsb
	pop	{r0-r12,r14}
	mov	pc,lr
endfunc a9_mmu_invalidate_tlb
	


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
#define	SCN_BITLEN	20 /*bitlen or shift */

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
func a9_mmu_set_scn
	push	{r0-r12,r14}
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
	str	SCN_ENTRY, [L1TBL_PA, SCN_VA, lsl #(WORD_ALGN)]
	add	SCN_PA, SCN_PA, #1
	add	SCN_VA, SCN_VA, #1
	cmp	SCN_VA, SCN_RNG
	bne	1b
	pop	{r0-r12,r14}
	mov	pc,lr
endfunc a9_mmu_set_scn

/*
 * CR1 bits (CP#15 CR1)
 */
#define CR_M	(1 << 0)	/* MMU enable				*/
#define CR_C	(1 << 2)	/* Dcache enable			*/
#define CR_Z	(1 << 11)	/* Program Flow Prediction		*/
#define CR_I	(1 << 12)	/* Icache enable			*/


func a9_mmu_enable
	push	{r0-r12,r14}
	mrc	p15, 0, r0, c1, c0, 0     // Read CP15 System Control register
	orr	r0, r0, #CR_M             // Set M bit 0 to enable MMU
	mcr	p15, 0, r0, c1, c0, 0     // Write CP15 System Control register
	isb
	pop	{r0-r12,r14}
	mov	pc, lr
endfunc a9_mmu_enable


func a9_l1cache_enable_i
	push	{r0-r12,r14}
	mrc	p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
	orr	r0, r0, #CR_I         // Enable I Cache
	orr	r0, r0, #CR_Z         // Enable Prediction
	mcr	p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
	isb
	pop	{r0-r12,r14}
	mov	pc, lr
endfunc a9_l1cache_enable_i


func a9_l1cache_enable_d
	push	{r0-r12,r14}
	mrc	p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
	orr	r0, r0, #CR_C         // Enable D Cache
	mcr	p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
	isb
	pop	{r0-r12,r14}
	mov	pc, lr
endfunc a9_l1cache_enable_d

func a9_gic_secure_init
	push	{r0-r12,r14}
	/* setup the GIC for non secure world. group 0 secure state, group 1 non secure state */
	/* enable interrupt for both groups */
	ldr	r0, =GICD_BASE
	ldr	r1, =0x3
	str	r1, [r0, #GICD_CTLR_OFFSET]
	
	/* assign all the interrupt to group 1 */
	ldr	r2, [r0, #GICD_TYPER_OFFSET]
	and	r2, r2, #0x1f
	add	r2, r2, #1
	ldr	r1, =0xffffffff
	add	r0, r0, #GICD_IGROUPR0_OFFSET	
setgroup:
	str	r1, [r0]
	add	r0, #4
	subs	r2, r2, #1
	bne	setgroup

	/* enable both group interrupt in the cpu interface */
	ldr	r0, =GICC_BASE
	ldr	r1, [r0, #GICC_CTLR_OFFSET]
	orr	r1, #0x3
	str	r1, [r0]

	/* set cpu priority mask view for nonsecure */
	mov	r1, #0x80
	str	r1, [r0, #GICC_PMR_OFFSET]
	pop	{r0-r12,r14}
	mov	pc, lr
endfunc a9_gic_secure_init


func a9_l1cache_inval_d
	push	{r0-r12,r14}
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	ands	r3, r0, #0x7000000		@ extract loc from clidr
	mov	r3, r3, lsr #23			@ left align loc bit field
	beq	invfinished			@ if loc is 0, then no need to clean
	mov	r10, #0				@ start clean at cache level 0
invloop1:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	invskip				@ skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb
						@ but we compile with armv5
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
invloop2:
	mov	r9, r4				@ create working copy of max way size
invloop3:
	orr	r6, r10, r9, lsl r5		@ factor way and cache number into r6
	orr	r6, r6, r7, lsl r2		@ factor index number into r6
	mcr	p15, 0, r6, c7, c6, 2		@ invalidate by set/way
	subs	r9, r9, #1			@ decrement the way
	bge	invloop3
	subs	r7, r7, #1			@ decrement the index
	bge	invloop2
invskip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	invloop1
invfinished:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb
	pop	{r0-r12,r14}
	mov	pc, lr
endfunc a9_l1cache_inval_d
