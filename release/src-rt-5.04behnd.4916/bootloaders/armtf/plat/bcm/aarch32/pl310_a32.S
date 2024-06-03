/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <asm_macros.S>
#include <platform_def.h>
#include <cpu_macros.S>

#define PL310_LOCKDOWN_NBREGS	8
#define PL310_LOCKDOWN_SZREG	4

#define PL310_8WAYS_MASK	0x00FF
#define PL310_16WAYS_UPPERMASK	0xFF00

	.globl	arm_cl2_lockallways
	.globl	arm_cl2_cleaninvbyway
	.globl	arm_cl2_invbyway
	.globl	arm_cl2_cleanbyway
	.globl	arm_cl2_cleanbypa
	.globl	arm_cl2_invbypa
	.globl	arm_cl2_cleaninvbypa
/*
 * void arm_cl2_lockallways(vaddr_t base)
 *
 * lock all L2 caches ways for data and instruction
 */
func arm_cl2_lockallways
	add	r1, r0, #PL310_DCACHE_LOCKDOWN_BASE
	ldr	r2, [r0, #PL310_AUX_CTRL]
	tst	r2, #PL310_AUX_16WAY_BIT
	mov	r2, #PL310_8WAYS_MASK
	orrne	r2, #PL310_16WAYS_UPPERMASK
	mov	r0, #PL310_LOCKDOWN_NBREGS
1:	/* lock Dcache and Icache */
	str	r2, [r1], #PL310_LOCKDOWN_SZREG
	str	r2, [r1], #PL310_LOCKDOWN_SZREG
	subs	r0, r0, #1
	bne	1b

	mov	pc, lr
endfunc arm_cl2_lockallways

/*
 * Set sync operation mask according to ways associativity.
 * Preserve r0 = pl310 iomem base address
 */
.macro syncbyway_set_mask reg
	ldr	\reg, [r0, #PL310_AUX_CTRL]
	tst	\reg, #PL310_AUX_16WAY_BIT
	mov	\reg, #PL310_8WAYS_MASK
	orrne	\reg, \reg, #PL310_16WAYS_UPPERMASK
.endm

/*
 * void arm_cl2_cleaninvbyway(vaddr_t base)
 * clean & invalidate the whole L2 cache.
 */
func arm_cl2_cleaninvbyway

	syncbyway_set_mask r1
	str	r1, [r0, #PL310_FLUSH_BY_WAY]

	/* Wait for all cache ways to be cleaned and invalidated */
loop_cli_way_done:
	ldr	r2, [r0, #PL310_FLUSH_BY_WAY]
	and	r2, r2, r1
	cmp	r2, #0
	bne	loop_cli_way_done

	/* Cache Sync */

	/*
	 * Wait for writing cache sync
	 * To PL310, Cache sync is atomic opertion, no need to check
	 * the status. For PL220, this check is needed. Keeping the loop
	 * for PL310 is no harm for PL310.
	 */
loop_cli_sync:
	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	loop_cli_sync

	mov	r1, #0
	str	r1, [r0, #PL310_SYNC]

loop_cli_sync_done:
	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	loop_cli_sync_done

	mov	pc, lr
endfunc arm_cl2_cleaninvbyway

/* void arm_cl2_invbyway(vaddr_t base) */
func arm_cl2_invbyway

	syncbyway_set_mask r1
	str	r1, [r0, #PL310_INV_BY_WAY]

loop_inv_way_done:
	ldr	r2, [r0, #PL310_INV_BY_WAY]
	and	r2, r2, r1
	cmp	r2, #0
	bne	loop_inv_way_done

loop_inv_way_sync:
	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	loop_inv_way_sync

	mov	r1, #0
	str	r1, [r0, #PL310_SYNC]

loop_inv_way_sync_done:
	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	loop_inv_way_sync_done

	mov	pc, lr
endfunc arm_cl2_invbyway

/* void arm_cl2_cleanbyway(vaddr_t base) */
func arm_cl2_cleanbyway

	syncbyway_set_mask r1
	str	r1, [r0, #PL310_CLEAN_BY_WAY]

loop_cl_way_done:
	ldr	r2, [r0, #PL310_CLEAN_BY_WAY]
	and	r2, r2, r1
	cmp	r2, #0
	bne	loop_cl_way_done

loop_cl_way_sync:
	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	loop_cl_way_sync

	mov	r1, #0
	str	r1, [r0, #PL310_SYNC]

loop_cl_way_sync_done:
	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	loop_cl_way_sync_done

	mov	pc, lr
endfunc arm_cl2_cleanbyway

/*
 * void _arm_cl2_xxxbypa(vaddr_t pl310_base, paddr_t start, paddr_t end,
 *			 int pl310value);
 * pl310value is one of PL310_CLEAN_BY_PA, PL310_INV_BY_PA or PL310_FLUSH_BY_PA
 */
func _arm_cl2_xxxbypa
	/* Align start address on PL310 line size */
	and	r1, #(~(PL310_LINE_SIZE - 1))
#ifdef SCU_BASE
	/*
	 * ARM ERRATA #764369
	 * Undocummented SCU Diagnostic Control Register
	 */
	/*
	 * NOTE:
	 * We're assuming that if mmu is enabled PL310_BASE and SCU_BASE
	 * still have the same relative offsets from each other.
	 */
	sub	r0, r0, #(PL310_BASE - SCU_BASE)
	mov	r12, #1
	str	r12, [r0, #SCU_ERRATA744369]
	dsb
	add	r0, r0, #(PL310_BASE - SCU_BASE)
#endif
loop_cl2_xxxbypa:
	str	r1, [r0, r3]

loop_xxx_pa_done:
	ldr	r12, [r0, r3]
	and	r12, r12, r1
	cmp	r12, #0
	bne	loop_xxx_pa_done

	add	r1, r1, #PL310_LINE_SIZE
	cmp	r2, r1
	bpl	loop_cl2_xxxbypa

loop_xxx_pa_sync:
	ldr	r12, [r0, #PL310_SYNC]
	cmp	r12, #0
	bne	loop_xxx_pa_sync

	mov	r12, #0
	str	r12, [r0, #PL310_SYNC]

loop_xxx_pa_sync_done:
	ldr	r12, [r0, #PL310_SYNC]
	cmp	r12, #0
	bne	loop_xxx_pa_sync_done

	mov	pc, lr
endfunc _arm_cl2_xxxbypa

/*
 * void _arm_cl2_cleanbypa(vaddr_t pl310_base, paddr_t start, paddr_t end);
 * clean L2 cache by physical address range.
 */
func arm_cl2_cleanbypa
	mov	r3, #PL310_CLEAN_BY_PA
	b	_arm_cl2_xxxbypa
endfunc arm_cl2_cleanbypa

/*
 * void arm_cl2_invbypa(vaddr_t pl310_base, paddr_t start, paddr_t end);
 * invalidate L2 cache by physical address range.
 */
func arm_cl2_invbypa
	mov	r3, #PL310_INV_BY_PA
	b	_arm_cl2_xxxbypa
endfunc arm_cl2_invbypa

/*
 * void arm_cl2_cleaninvbypa(vaddr_t pl310_base, paddr_t start, paddr_t end);
 * clean and invalidate L2 cache by physical address range.
 */
func arm_cl2_cleaninvbypa
	mov	r3, #PL310_FLUSH_BY_PA
	b	_arm_cl2_xxxbypa
endfunc arm_cl2_cleaninvbypa

