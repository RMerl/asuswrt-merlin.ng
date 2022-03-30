/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2003	Josef Baumgartner <josef.baumgartner@telex.de>
 * Based on code from Bernhard Kuhn <bkuhn@metrowerks.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include "version.h"
#include <asm/cache.h>

#define _START	_start
#define _FAULT	_fault


#define SAVE_ALL						\
	move.w	#0x2700,%sr;		/* disable intrs */	\
	subl	#60,%sp;		/* space for 15 regs */ \
	moveml	%d0-%d7/%a0-%a6,%sp@;				\

#define RESTORE_ALL						\
	moveml	%sp@,%d0-%d7/%a0-%a6;				\
	addl	#60,%sp;		/* space for 15 regs */ \
	rte

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
.long	0x00000000		/* Flash offset is 0 until we setup CS0 */
#if defined(CONFIG_M5282) && (CONFIG_SYS_TEXT_BASE == CONFIG_SYS_INT_FLASH_BASE)
.long	_start - CONFIG_SYS_TEXT_BASE
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

#if defined(CONFIG_SYS_INT_FLASH_BASE) && \
    (defined(CONFIG_M5282) || defined(CONFIG_M5281))
#if (CONFIG_SYS_TEXT_BASE == CONFIG_SYS_INT_FLASH_BASE)
.long	0x55AA55AA,0xAA55AA55		/* CFM Backdoorkey */
.long	0xFFFFFFFF			/* all sectors protected */
.long	0x00000000			/* supervisor/User restriction */
.long	0x00000000			/* programm/data space restriction */
.long	0x00000000			/* Flash security */
#endif
#endif

.globl _start
_start:
	nop
	nop
	move.w	#0x2700,%sr

#if defined(CONFIG_M5208)
	/* Initialize RAMBAR: locate SRAM and validate it */
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_CTRL), %d0
	movec	%d0, %RAMBAR1
#endif

#if defined(CONFIG_M5272) || defined(CONFIG_M5249) || defined(CONFIG_M5253)
	/* set MBAR address + valid flag */
	move.l	#(CONFIG_SYS_MBAR + 1), %d0
	move.c	%d0, %MBAR

	/*** The 5249 has MBAR2 as well ***/
#ifdef CONFIG_SYS_MBAR2
	/* Get MBAR2 address */
	move.l	#(CONFIG_SYS_MBAR2 + 1), %d0
	 /* Set MBAR2 */
	movec	%d0, #0xc0e
#endif
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + 1), %d0
	movec	%d0, %RAMBAR0
#endif /* CONFIG_M5272 || CONFIG_M5249 || CONFIG_M5253 */

#if defined(CONFIG_M5282) || defined(CONFIG_M5271)
	/* set MBAR address + valid flag */
	move.l	#(CONFIG_SYS_MBAR + 1), %d0
	move.l	%d0, 0x40000000

	/* Initialize RAMBAR1: locate SRAM and validate it */
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + 0x21), %d0
	movec	%d0, %RAMBAR1

#if defined(CONFIG_M5282)
#if (CONFIG_SYS_TEXT_BASE == CONFIG_SYS_INT_FLASH_BASE)
	/*
	 * Setup code in SRAM to initialize FLASHBAR,
	 * if start from internal Flash
	 */
	move.l	#(_flashbar_setup-CONFIG_SYS_INT_FLASH_BASE), %a0
	move.l	#(_flashbar_setup_end-CONFIG_SYS_INT_FLASH_BASE), %a1
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR), %a2
_copy_flash:
	move.l	(%a0)+, (%a2)+
	cmp.l	%a0, %a1
	bgt.s	_copy_flash
	jmp	CONFIG_SYS_INIT_RAM_ADDR

_flashbar_setup:
	/* Initialize FLASHBAR: locate internal Flash and validate it */
	move.l	#(CONFIG_SYS_INT_FLASH_BASE + CONFIG_SYS_INT_FLASH_ENABLE), %d0
	movec	%d0, %FLASHBAR
	jmp	_after_flashbar_copy.L	/* Force jump to absolute address */
_flashbar_setup_end:
	nop
_after_flashbar_copy:
#else
	/* Setup code to initialize FLASHBAR, if start from external Memory */
	move.l	#(CONFIG_SYS_INT_FLASH_BASE + CONFIG_SYS_INT_FLASH_ENABLE), %d0
	movec	%d0, %FLASHBAR
#endif /* (CONFIG_SYS_TEXT_BASE == CONFIG_SYS_INT_FLASH_BASE) */

#endif
#endif
	/*
	 * if we come from a pre-loader we have no exception table and
	 * therefore no VBR to set
	 */
#if !defined(CONFIG_MONITOR_IS_IN_RAM)
#if defined(CONFIG_M5282) && (CONFIG_SYS_TEXT_BASE == CONFIG_SYS_INT_FLASH_BASE)
	move.l	#CONFIG_SYS_INT_FLASH_BASE, %d0
#else
	move.l	#CONFIG_SYS_FLASH_BASE, %d0
#endif
	movec	%d0, %VBR
#endif

#ifdef CONFIG_M5275
	/* set MBAR address + valid flag */
	move.l	#(CONFIG_SYS_MBAR + 1), %d0
	move.l	%d0, 0x40000000
/*	movec	%d0, %MBAR */

	/* Initialize RAMBAR: locate SRAM and validate it */
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + 0x21), %d0
	movec	%d0, %RAMBAR1
#endif

	/* initialize general use internal ram */
	move.l	#0, %d0
	move.l	#(ICACHE_STATUS), %a1	/* icache */
	move.l	#(DCACHE_STATUS), %a2	/* icache */
	move.l	%d0, (%a1)
	move.l	%d0, (%a2)

	/* put relocation table address to a5 */
	move.l	#__got_start, %a5

	/* setup stack initially on top of internal static ram  */
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE), %sp

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
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r3 = dest
 * r4 = src
 * r5 = length in bytes
 * r6 = cachelinesize
 */
.globl relocate_code
relocate_code:
	link.w	%a6,#0
	move.l	8(%a6), %sp		/* set new stack pointer */

	move.l	12(%a6), %d0		/* Save copy of Global Data pointer */
	move.l	16(%a6), %a0		/* Save copy of Destination Address */

	move.l	#CONFIG_SYS_MONITOR_BASE, %a1
	move.l	#__init_end, %a2
	move.l	%a0, %a3
	/* copy the code to RAM */
1:
	move.l	(%a1)+, (%a3)+
	cmp.l	%a1,%a2
	bgt.s	1b

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
	move.l	%a0, %a1
	add.l	#(_sbss - CONFIG_SYS_MONITOR_BASE),%a1
	move.l	%a0, %d1
	add.l	#(_ebss - CONFIG_SYS_MONITOR_BASE),%d1
6:
	clr.l	(%a1)+
	cmp.l	%a1,%d1
	bgt.s	6b

	/*
	 * fix got table in RAM
	 */
	move.l	%a0, %a1
	add.l	#(__got_start - CONFIG_SYS_MONITOR_BASE),%a1
	move.l	%a1,%a5			/* fix got pointer register a5 */

	move.l	%a0, %a2
	add.l	#(__got_end - CONFIG_SYS_MONITOR_BASE),%a2

7:
	move.l	(%a1),%d1
	sub.l	#_start,%d1
	add.l	%a0,%d1
	move.l	%d1,(%a1)+
	cmp.l	%a2, %a1
	bne	7b

	/* calculate relative jump to board_init_r in ram */
	move.l	%a0, %a1
	add.l	#(board_init_r - CONFIG_SYS_MONITOR_BASE), %a1

	/* set parameters for board_init_r */
	move.l	%a0,-(%sp)		/* dest_addr */
	move.l	%d0,-(%sp)		/* gd */
#if defined(DEBUG) && (CONFIG_SYS_TEXT_BASE != CONFIG_SYS_INT_FLASH_BASE) && \
    defined(CONFIG_SYS_HALT_BEFOR_RAM_JUMP)
	halt
#endif
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
	bsr	exc_handler
	addql	#4,%sp
	RESTORE_ALL

.globl _int_handler
_int_handler:
	SAVE_ALL
	movel	%sp,%sp@-
	bsr	int_handler
	addql	#4,%sp
	RESTORE_ALL

/******************************************************************************/

.globl version_string
version_string:
.ascii U_BOOT_VERSION_STRING, "\0"
.align 4
