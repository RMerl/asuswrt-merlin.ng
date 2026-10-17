/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 *
 */

#include <asm/macro.h>
#include <linux/linkage.h>

hnf_pstate_poll:
	/* x0 has the desired status, return 0 for success, 1 for timeout
	 * clobber x1, x2, x3, x4, x6, x7
	 */
	mov	x1, x0
	mov	x7, #0			/* flag for timeout */
	mrs	x3, cntpct_el0		/* read timer */
	mov	w0, #600
	mov	w6, #1000
	mul	w0, w0, w6
	add	x3, x3, x0		/* timeout after 100 microseconds */
	mov	x0, #0x18
	movk	x0, #0x6120, lsl #16	/* HNF0_PSTATE_STATUS */
	mov	w6, #4			/* HN-F node count */
1:
	ldr	x2, [x0]
	cmp	x2, x1			/* check status */
	b.eq	2f
	mrs	x4, cntpct_el0
	cmp	x4, x3
	b.ls	1b
	mov	x7, #1			/* timeout */
	b	3f
2:
	add	x0, x0, #0x10000	/* move to next node */
	subs	w6, w6, #1
	cbnz	w6, 1b
3:
	mov	x0, x7
	ret

hnf_set_pstate:
	/* x0 has the desired state, clobber x1, x2, x6 */
	mov	x1, x0
	/* power state to SFONLY */
	mov	w6, #4			/* HN-F node count */
	mov	x0, #0x10
	movk x0, #0x6120, lsl #16		/* HNF0_PSTATE_REQ */
1:	/* set pstate to sfonly */
	ldr	x2, [x0]
	and	x2, x2, #0xfffffffffffffffc	/* & HNFPSTAT_MASK */
	orr	x2, x2, x1
	str	x2, [x0]
	add	x0, x0, #0x10000	/* move to next node */
	subs	w6, w6, #1
	cbnz	w6, 1b

	ret

ENTRY(__asm_flush_l3_dcache)
	/*
	 * Return status in x0
	 *    success 0
	 *    timeout 1 for setting SFONLY, 2 for FAM, 3 for both
	 */
	mov	x29, lr
	mov	x8, #0

	dsb	sy
	mov	x0, #0x1		/* HNFPSTAT_SFONLY */
	bl	hnf_set_pstate

	mov	x0, #0x4		/* SFONLY status */
	bl	hnf_pstate_poll
	cbz	x0, 1f
	mov	x8, #1			/* timeout */
1:
	dsb	sy
	mov	x0, #0x3		/* HNFPSTAT_FAM */
	bl	hnf_set_pstate

	mov	x0, #0xc		/* FAM status */
	bl	hnf_pstate_poll
	cbz	x0, 1f
	add	x8, x8, #0x2
1:
	mov	x0, x8
	mov	lr, x29
	ret
ENDPROC(__asm_flush_l3_dcache)

ENTRY(save_boot_params)
/*
 * void set_boot_params(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3)
 */
	adr	x4, bl33_info
	str	x0, [x4]
	b	save_boot_params_ret
ENDPROC(save_boot_params)
