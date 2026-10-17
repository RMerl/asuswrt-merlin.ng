/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015  Angelo Dureghello <angelo@sysam.it>
 * Based on code from Bernhard Kuhn <bkuhn@metrowerks.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/cache.h>

#define _START	_start
#define _FAULT	_fault


.macro  SAVE_ALL
	move.w	#0x2700,%sr;		/* disable intrs */
	subl	#60,%sp;		/* space for 15 regs */
	moveml	%d0-%d7/%a0-%a6,%sp@
.endm

.macro  RESTORE_ALL
	moveml	%sp@,%d0-%d7/%a0-%a6;
	addl	#60,%sp;		/* space for 15 regs */
	rte
.endm

/* If we come from a pre-loader we don't need an initial exception
 * table.
 */
#if !defined(CONFIG_MONITOR_IS_IN_RAM)

.text

/*
 * Vector table. This is used for initial platform startup.
 * These vectors are to catch any un-intended traps.
 */
_vectors:
/* Flash offset is 0 until we setup CS0 */
.long	0x00000000
#if defined(CONFIG_M5307) && \
	   (CONFIG_TEXT_BASE == CFG_SYS_INT_FLASH_BASE)
.long	_start - CONFIG_TEXT_BASE
#else
.long	_START
#endif

.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

#endif

.text

.globl _start
_start:
	nop
	nop
	move.w	#0x2700,%sr

	/* set MBAR address + valid flag */
	move.l	#(CFG_SYS_MBAR + 1), %d0
	move.c	%d0, %MBAR

	move.l	#(CFG_SYS_INIT_RAM_ADDR + 1), %d0
	move.c	%d0, %RAMBAR

	/* DS 4.8.2 (Cache Organization) invalidate and disable cache */
        move.l	#CF_CACR_CINVA, %d0
        movec	%d0, %CACR
        move.l	#0, %d0
        movec	%d0, %ACR0
        movec	%d0, %ACR1

	/*
	 * if we come from a pre-loader we have no exception table and
	 * therefore no VBR to set
	 */
#if !defined(CONFIG_MONITOR_IS_IN_RAM)
	move.l	#CFG_SYS_FLASH_BASE, %d0
	movec	%d0, %VBR
#endif

	/* initialize general use internal ram */
	move.l	#0, %d0
	move.l	#(ICACHE_STATUS), %a1	/* icache */
	move.l	#(DCACHE_STATUS), %a2	/* dcache */
	move.l	%d0, (%a1)
	move.l	%d0, (%a2)

	/* put relocation table address to a5 */
	move.l	#__got_start, %a5

	/* setup stack initially on top of internal static ram  */
	move.l	#(CFG_SYS_INIT_RAM_ADDR + CFG_SYS_INIT_RAM_SIZE), %sp

	/*
	 * if configured, malloc_f arena will be reserved first,
	 * then (and always) gd struct space will be reserved
	 */
	move.l	%sp, -(%sp)
	move.l	#board_init_f_alloc_reserve, %a1
	jsr	(%a1)

	/* update stack and frame-pointers */
	move.l	%d0, %sp
	move.l	%sp, %fp

	/* initialize reserved area */
	move.l	%d0, -(%sp)
	move.l	#board_init_f_init_reserve, %a1
	jsr	(%a1)

	/* run low-level CPU init code (from flash) */
	move.l	#cpu_init_f, %a1
	jsr	(%a1)

	/* run low-level board init code (from flash) */
	clr.l	%sp@-
	move.l	#board_init_f, %a1
	jsr	(%a1)

	/* board_init_f() does not return */

/******************************************************************************/

/*
 * void relocate_code(addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 */
.globl relocate_code
relocate_code:
	link.w	%a6,#0
	move.l	8(%a6), %sp	/* set new stack pointer */
	move.l	12(%a6), %d0	/* Save copy of Global Data pointer */
	move.l	16(%a6), %a0	/* Save copy of Destination Address */

	move.l	#CONFIG_SYS_MONITOR_BASE, %a1
	move.l	#__init_end, %a2
	move.l	%a0, %a3
	/* copy the code to RAM */
1:
	move.l	(%a1)+, (%a3)+
	cmp.l	%a1,%a2
	bgt.s	1b

#define R_68K_32	1
#define R_68K_RELATIVE	22

	move.l #(__rel_dyn_start), %a1
	move.l #(__rel_dyn_end), %a2

fixloop:
	move.l	(%a1)+, %d1	/* Elf32_Rela r_offset */
	move.l	(%a1)+, %d2	/* Elf32_Rela r_info */
	move.l	(%a1)+, %d3	/* Elf32_Rela r_addend */

	andi.l	#0xff, %d2
	cmp.l	#R_68K_32, %d2
	beq.s	fixup
	cmp.l	#R_68K_RELATIVE, %d2
	beq.s	fixup

	bra	fixnext

fixup:
	/* relative fix: store addend plus offset at dest location */
	move.l	%a0, %a3
	add.l	%d1, %a3
	sub.l   #CONFIG_SYS_MONITOR_BASE, %a3
	move.l	(%a3), %d4
	add.l	%a0, %d4
	sub.l   #CONFIG_SYS_MONITOR_BASE, %d4
	move.l	%d4, (%a3)

fixnext:
	cmp.l	%a1, %a2
	bge.s	fixloop

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
	move.l	%a0, %a1
	add.l	#(in_ram - CONFIG_SYS_MONITOR_BASE), %a1
	jmp	(%a1)

in_ram:

clear_bss:
	/*
	 * Now clear BSS segment
	 */
	move.l	#(_sbss), %a1
	move.l	#(_ebss), %d1
6:
	clr.l	(%a1)+
	cmp.l	%a1,%d1
	bgt.s	6b

	/*
	 * fix got table in RAM
	 */
	move.l	#(__got_start), %a5	/* fix got pointer register a5 */

	/* calculate relative jump to board_init_r in ram */
	move.l	#(board_init_r), %a1

	/* set parameters for board_init_r */
	move.l	%a0,-(%sp)	/* dest_addr */
	move.l	%d0,-(%sp)	/* gd */
	jsr	(%a1)

/******************************************************************************/

/* exception code */
.globl _fault
_fault:
	bra	_fault

.globl _exc_handler
_exc_handler:
	SAVE_ALL
	movel	%sp,%sp@-
	move.l	#exc_handler, %a1
	jsr	(%a1)
	addql	#4,%sp
	RESTORE_ALL

.globl _int_handler
_int_handler:
	SAVE_ALL
	movel	%sp,%sp@-
	move.l	#int_handler, %a1
	jsr	(%a1)
	addql	#4,%sp
	RESTORE_ALL

/******************************************************************************/

.align	4
