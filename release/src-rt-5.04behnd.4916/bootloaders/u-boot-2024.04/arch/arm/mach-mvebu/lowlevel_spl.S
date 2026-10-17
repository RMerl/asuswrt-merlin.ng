/* SPDX-License-Identifier: GPL-2.0+ */

#include <config.h>
#include <linux/linkage.h>

/*
 * BootROM loads the header part of kwbimage into L2 cache. BIN header usually
 * contains U-Boot SPL, optionally it can also contain additional arguments.
 * The number of these arguments is in r0, pointer to the argument array in r1.
 * BootROM expects executable BIN header code to return to address stored in lr.
 * Other registers (r2 - r12) must be preserved. We save all registers to the
 * address of CONFIG_SPL_STACK + 4. BIN header arguments (passed via r0 and r1)
 * are currently not used by U-Boot SPL binary.
 */
ENTRY(save_boot_params)
	stmfd	sp!, {r0 - r12, lr}	/* @ save registers on stack */
	ldr	r12, =(CONFIG_SPL_STACK + 4)
	str	sp, [r12]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

ENTRY(return_to_bootrom)
	ldr	r12, =(CONFIG_SPL_STACK + 4)
	ldr	sp, [r12]
	ldmfd	sp!, {r0 - r12, lr}	/* @ restore registers from stack */
	mov	r0, #0x0		/* @ return value: 0x0 NO_ERR */
	bx	lr			/* @ return to bootrom */
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
