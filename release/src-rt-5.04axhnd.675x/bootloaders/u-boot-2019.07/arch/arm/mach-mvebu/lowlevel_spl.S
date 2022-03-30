/* SPDX-License-Identifier: GPL-2.0+ */

#include <config.h>
#include <linux/linkage.h>

ENTRY(save_boot_params)
	stmfd	sp!, {r0 - r12, lr}	/* @ save registers on stack */
	ldr	r12, =CONFIG_SPL_BOOTROM_SAVE
	str	sp, [r12]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

ENTRY(return_to_bootrom)
	ldr	r12, =CONFIG_SPL_BOOTROM_SAVE
	ldr	sp, [r12]
	mov	r0, #0x0		/* @ return value: 0x0 NO_ERR */
	ldmfd	sp!, {r0 - r12, pc}	/* @ restore regs and return */
ENDPROC(return_to_bootrom)

/*
 * cache_inv - invalidate Cache line
 * r0 - dest
 */
	.global cache_inv
	.type  cache_inv, %function
	cache_inv:

	stmfd   sp!, {r1-r12}

	mcr     p15, 0, r0, c7, c6, 1

	ldmfd   sp!, {r1-r12}
	bx      lr


/*
 * flush_l1_v6 - l1 cache clean invalidate
 * r0 - dest
 */
	.global flush_l1_v6
	.type	flush_l1_v6, %function
	flush_l1_v6:

	stmfd   sp!, {r1-r12}

	mcr     p15, 0, r0, c7, c10, 5	/* @ data memory barrier */
	mcr     p15, 0, r0, c7, c14, 1	/* @ clean & invalidate D line */
	mcr     p15, 0, r0, c7, c10, 4	/* @ data sync barrier */

	ldmfd   sp!, {r1-r12}
	bx      lr


/*
 * flush_l1_v7 - l1 cache clean invalidate
 * r0 - dest
 */
	.global flush_l1_v7
	.type	flush_l1_v7, %function
	flush_l1_v7:

	stmfd   sp!, {r1-r12}

	dmb				/* @data memory barrier */
	mcr     p15, 0, r0, c7, c14, 1	/* @ clean & invalidate D line */
	dsb				/* @data sync barrier */

	ldmfd   sp!, {r1-r12}
	bx      lr
