/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Marvell International Ltd.
 */

#include <asm/arch-armada8k/cache_llc.h>
#include <linux/linkage.h>

/*
 * int __asm_flush_l3_dcache
 *
 * flush Armada-8K last level cache.
 *
 */
ENTRY(__asm_flush_l3_dcache)
	/* flush cache */
	mov     x0, #LLC_BASE_ADDR
	add	x0, x0, #LLC_FLUSH_BY_WAY
	movk    x0, #MVEBU_A8K_REGS_BASE_MSB, lsl #16
	mov     w1, #LLC_WAY_MASK
	str     w1, [x0]
	/* sync cache */
	mov     x0, #LLC_BASE_ADDR
	add	x0, x0, #LLC_CACHE_SYNC
	movk    x0, #MVEBU_A8K_REGS_BASE_MSB, lsl #16
	str     wzr, [x0]
	/* check that cache sync completed */
	mov     x0, #LLC_BASE_ADDR
	add	x0, x0, #LLC_CACHE_SYNC_COMPLETE
	movk    x0, #MVEBU_A8K_REGS_BASE_MSB, lsl #16
1:	ldr	w1, [x0]
	and	w1, w1, #LLC_CACHE_SYNC_MASK
	cbnz	w1, 1b
	/* return success */
	mov	x0, #0
	ret
ENDPROC(__asm_flush_l3_dcache)
