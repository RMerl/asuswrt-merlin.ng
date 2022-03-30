/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/arcregs.h>

ENTRY(_start)
	/* Setup interrupt vector base that matches "__text_start" */
	sr	__ivt_start, [ARC_AUX_INTR_VEC_BASE]

	; Disable/enable I-cache according to configuration
	lr	r5, [ARC_BCR_IC_BUILD]
	breq	r5, 0, 1f		; I$ doesn't exist
	lr	r5, [ARC_AUX_IC_CTRL]
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	bclr	r5, r5, 0		; 0 - Enable, 1 is Disable
#else
	bset	r5, r5, 0		; I$ exists, but is not used
#endif
	sr	r5, [ARC_AUX_IC_CTRL]

	mov	r5, 1
	sr	r5, [ARC_AUX_IC_IVIC]
	; As per ARC HS databook (see chapter 5.3.3.2)
	; it is required to add 3 NOPs after each write to IC_IVIC.
	nop
	nop
	nop

1:
	; Disable/enable D-cache according to configuration
	lr	r5, [ARC_BCR_DC_BUILD]
	breq	r5, 0, 1f		; D$ doesn't exist
	lr	r5, [ARC_AUX_DC_CTRL]
	bclr	r5, r5, 6		; Invalidate (discard w/o wback)
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	bclr	r5, r5, 0		; Enable (+Inv)
#else
	bset	r5, r5, 0		; Disable (+Inv)
#endif
	sr	r5, [ARC_AUX_DC_CTRL]

	mov	r5, 1
	sr	r5, [ARC_AUX_DC_IVDC]


1:
#ifdef CONFIG_ISA_ARCV2
	; Disable System-Level Cache (SLC)
	lr	r5, [ARC_BCR_SLC]
	breq	r5, 0, 1f		; SLC doesn't exist
	lr	r5, [ARC_AUX_SLC_CTRL]
	bclr	r5, r5, 6		; Invalidate (discard w/o wback)
	bclr	r5, r5, 0		; Enable (+Inv)
	sr	r5, [ARC_AUX_SLC_CTRL]

1:
#endif

#ifdef __ARC_UNALIGNED__
	/*
	 * Enable handling of unaligned access in the CPU as by default
	 * this HW feature is disabled while GCC starting from 8.1.0
	 * unconditionally uses it for ARC HS cores.
	 */
	flag	1 << STATUS_AD_BIT
#endif

	/* Establish C runtime stack and frame */
	mov	%sp, CONFIG_SYS_INIT_SP_ADDR
	mov	%fp, %sp

	/* Allocate reserved area from current top of stack */
	mov	%r0, %sp
	bl	board_init_f_alloc_reserve
	/* Set stack below reserved area, adjust frame pointer accordingly */
	mov	%sp, %r0
	mov	%fp, %sp

	/* Initialize reserved area - note: r0 already contains address */
	bl	board_init_f_init_reserve

#ifdef CONFIG_DEBUG_UART
	/* Earliest point to set up early debug uart */
	bl	debug_uart_init
#endif

	/* Zero the one and only argument of "board_init_f" */
	mov_s	%r0, 0
	bl	board_init_f

	/* We only get here if relocation is disabled by GD_FLG_SKIP_RELOC */
	/* Make sure we don't lose GD overwritten by zero new GD */
	mov	%r0, %r25
	mov	%r1, 0
	bl	board_init_r
ENDPROC(_start)

/*
 * void board_init_f_r_trampoline(stack-pointer address)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r0 = new stack-pointer
 */
ENTRY(board_init_f_r_trampoline)
	/* Set up the stack- and frame-pointers */
	mov	%sp, %r0
	mov	%fp, %sp

	/* Update position of intterupt vector table */
	lr	%r0, [ARC_AUX_INTR_VEC_BASE]
	ld	%r1, [%r25, GD_RELOC_OFF]
	add	%r0, %r0, %r1
	sr	%r0, [ARC_AUX_INTR_VEC_BASE]

	/* Re-enter U-Boot by calling board_init_f_r */
	j	board_init_f_r
ENDPROC(board_init_f_r_trampoline)
