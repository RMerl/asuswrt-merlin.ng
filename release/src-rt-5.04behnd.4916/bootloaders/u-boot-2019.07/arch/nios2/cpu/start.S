/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <version.h>

/*
 * icache and dcache configuration used only for start.S.
 * the values are chosen so that it will work for all configuration.
 */
#define ICACHE_LINE_SIZE	32 /* fixed 32 */
#define ICACHE_SIZE_MAX		0x10000 /* 64k max */
#define DCACHE_LINE_SIZE_MIN	4 /* 4, 16, 32 */
#define DCACHE_SIZE_MAX		0x10000 /* 64k max */

	/* RESTART */
	.text
	.global _start, _except_start, _except_end

_start:
	wrctl	status, r0		/* Disable interrupts */
	/*
	 * ICACHE INIT -- only the icache line at the reset address
	 * is invalidated at reset. So the init must stay within
	 * the cache line size (8 words). If GERMS is used, we'll
	 * just be invalidating the cache a second time. If cache
	 * is not implemented initi behaves as nop.
	 */
	ori	r4, r0, %lo(ICACHE_LINE_SIZE)
	movhi	r5, %hi(ICACHE_SIZE_MAX)
	ori	r5, r5, %lo(ICACHE_SIZE_MAX)
0:	initi	r5
	sub	r5, r5, r4
	bgt	r5, r0, 0b
	br	_except_end	/* Skip the tramp */

	/*
	 * EXCEPTION TRAMPOLINE -- the following gets copied
	 * to the exception address (below), but is otherwise at the
	 * default exception vector offset (0x0020).
	 */
_except_start:
	movhi	et, %hi(_exception)
	ori	et, et, %lo(_exception)
	jmp	et
_except_end:

	/*
	 * INTERRUPTS -- for now, all interrupts masked and globally
	 * disabled.
	 */
	wrctl	ienable, r0		/* All disabled	*/

	/*
	 * DCACHE INIT -- if dcache not implemented, initd behaves as
	 * nop.
	 */
	ori	r4, r0, %lo(DCACHE_LINE_SIZE_MIN)
	movhi	r5, %hi(DCACHE_SIZE_MAX)
	ori	r5, r5, %lo(DCACHE_SIZE_MAX)
	mov	r6, r0
1:	initd	0(r6)
	add	r6, r6, r4
	bltu	r6, r5, 1b

	/*
	 * RELOCATE CODE, DATA & COMMAND TABLE -- the following code
	 * assumes code, data and the command table are all
	 * contiguous. This lets us relocate everything as a single
	 * block. Make sure the linker script matches this ;-)
	 */
	nextpc	r4
_cur:	movhi	r5, %hi(_cur - _start)
	ori	r5, r5, %lo(_cur - _start)
	sub	r4, r4, r5		/* r4 <- cur _start */
	mov	r8, r4
	movhi	r5, %hi(_start)
	ori	r5, r5, %lo(_start)	/* r5 <- linked _start */
	mov	sp, r5		/* initial stack below u-boot code */
	beq	r4, r5, 3f

	movhi	r6, %hi(CONFIG_SYS_MONITOR_LEN)
	ori	r6, r6, %lo(CONFIG_SYS_MONITOR_LEN)
	add	r6, r6, r5
2:	ldwio	r7, 0(r4)
	addi	r4, r4, 4
	stwio	r7, 0(r5)
	addi	r5, r5, 4
	bne	r5, r6, 2b
3:

	/* JUMP TO RELOC ADDR */
	movhi	r4, %hi(_reloc)
	ori	r4, r4, %lo(_reloc)
	jmp	r4
_reloc:

	/* STACK INIT -- zero top two words for call back chain. */
	addi	sp, sp, -8
	stw	r0, 0(sp)
	stw	r0, 4(sp)
	mov	fp, sp

#ifdef CONFIG_DEBUG_UART
	/* Set up the debug UART */
	movhi	r2, %hi(debug_uart_init@h)
	ori	r2, r2, %lo(debug_uart_init@h)
	callr	r2
#endif

	/* Allocate and initialize reserved area, update SP */
	mov	r4, sp
	movhi	r2, %hi(board_init_f_alloc_reserve@h)
	ori	r2, r2, %lo(board_init_f_alloc_reserve@h)
	callr	r2
	mov	sp, r2
	mov	r4, sp
	movhi	r2, %hi(board_init_f_init_reserve@h)
	ori	r2, r2, %lo(board_init_f_init_reserve@h)
	callr	r2

	/* Update frame-pointer */
	mov	fp, sp

	/* Call board_init_f -- never returns */
	mov	r4, r0
	movhi	r2, %hi(board_init_f@h)
	ori	r2, r2, %lo(board_init_f@h)
	callr	r2

	/*
	 * NEVER RETURNS -- but branch to the _start just
	 * in case ;-)
	 */
	br	_start

	/*
	 * relocate_code -- Nios2 handles the relocation above. But
	 * the generic board code monkeys with the heap, stack, etc.
	 * (it makes some assumptions that may not be appropriate
	 * for Nios). Nevertheless, we capitulate here.
	 *
	 * We'll call the board_init_r from here since this isn't
	 * supposed to return.
	 *
	 * void relocate_code (ulong sp, gd_t *global_data,
	 *			ulong reloc_addr)
	 *			__attribute__ ((noreturn));
	 */
	.text
	.global relocate_code

relocate_code:
	mov	sp, r4		/* Set the new sp */
	mov	r4, r5

	/*
	 * ZERO BSS/SBSS -- bss and sbss are assumed to be adjacent
	 * and between __bss_start and __bss_end.
	 */
	movhi	r5, %hi(__bss_start)
	ori	r5, r5, %lo(__bss_start)
	movhi	r6, %hi(__bss_end)
	ori	r6, r6, %lo(__bss_end)
	beq	r5, r6, 5f

4:	stw	r0, 0(r5)
	addi	r5, r5, 4
	bne	r5, r6, 4b
5:

	movhi	r8, %hi(board_init_r@h)
	ori	r8, r8, %lo(board_init_r@h)
	callr	r8
	ret
