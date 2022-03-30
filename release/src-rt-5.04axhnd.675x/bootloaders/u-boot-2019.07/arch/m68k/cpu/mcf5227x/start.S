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
	moveml	%d0-%d7/%a0-%a6,%sp@;

#define RESTORE_ALL						\
	moveml	%sp@,%d0-%d7/%a0-%a6;				\
	addl	#60,%sp;		/* space for 15 regs */ \
	rte;

#if defined(CONFIG_CF_SBF)
#define ASM_DRAMINIT	(asm_dram_init - CONFIG_SYS_TEXT_BASE + \
	CONFIG_SYS_INIT_RAM_ADDR)
#define ASM_SBF_IMG_HDR	(asm_sbf_img_hdr - CONFIG_SYS_TEXT_BASE + \
	CONFIG_SYS_INIT_RAM_ADDR)
#endif

.text

/*
 * Vector table. This is used for initial platform startup.
 * These vectors are to catch any un-intended traps.
 */
_vectors:
#if defined(CONFIG_CF_SBF)
INITSP:	.long	0			/* Initial SP	*/
INITPC:	.long	ASM_DRAMINIT		/* Initial PC 	*/
#else
INITSP:	.long	0			/* Initial SP	*/
INITPC:	.long	_START			/* Initial PC 	*/
#endif

vector02_0F:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

/* Reserved */
vector10_17:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector18_1F:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

#if !defined(CONFIG_CF_SBF)
/* TRAP #0 - #15 */
vector20_2F:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

/* Reserved	*/
vector30_3F:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector64_127:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector128_191:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector192_255:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
#endif

#if defined(CONFIG_CF_SBF)
	/* Image header: chksum 4 bytes, len 4 bytes, img dest 4 bytes */
asm_sbf_img_hdr:
	.long	0x00000000		/* checksum, not yet implemented */
	.long	0x00020000		/* image length */
	.long	CONFIG_SYS_TEXT_BASE	/* image to be relocated at */

asm_dram_init:
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_CTRL), %d0
	movec	%d0, %RAMBAR1		/* init Rambar */

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET), %sp
	clr.l	%sp@-

	/* Must disable global address */
	move.l	#0xFC008000, %a1
	move.l	#(CONFIG_SYS_CS0_BASE), (%a1)
	move.l	#0xFC008008, %a1
	move.l	#(CONFIG_SYS_CS0_CTRL), (%a1)
	move.l	#0xFC008004, %a1
	move.l	#(CONFIG_SYS_CS0_MASK), (%a1)

	/*
	 * Dram Initialization
	 * a1, a2, and d0
	 */
	move.l	#0xFC0A4074, %a1
	move.b	#(CONFIG_SYS_SDRAM_DRV_STRENGTH), (%a1)
	nop

	/* SDRAM Chip 0 and 1 */
	move.l	#0xFC0B8110, %a1
	move.l	#0xFC0B8114, %a2

	/* calculate the size */
	move.l	#0x13, %d1
	move.l	#(CONFIG_SYS_SDRAM_SIZE), %d2
#ifdef CONFIG_SYS_SDRAM_BASE1
	lsr.l	#1, %d2
#endif

dramsz_loop:
	lsr.l	#1, %d2
	add.l	#1, %d1
	cmp.l	#1, %d2
	bne	dramsz_loop

	/* SDRAM Chip 0 and 1 */
	move.l	#(CONFIG_SYS_SDRAM_BASE), (%a1)
	or.l	%d1, (%a1)
#ifdef CONFIG_SYS_SDRAM_BASE1
	move.l	#(CONFIG_SYS_SDRAM_BASE1), (%a2)
	or.l	%d1, (%a2)
#endif
	nop

	/* dram cfg1 and cfg2 */
	move.l	#0xFC0B8008, %a1
	move.l	#(CONFIG_SYS_SDRAM_CFG1), (%a1)
	nop
	move.l	#0xFC0B800C, %a2
	move.l	#(CONFIG_SYS_SDRAM_CFG2), (%a2)
	nop

	move.l	#0xFC0B8000, %a1	/* Mode */
	move.l	#0xFC0B8004, %a2	/* Ctrl */

	/* Issue PALL */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 2), (%a2)
	nop

	/* Issue LEMR */
	move.l	#(CONFIG_SYS_SDRAM_MODE), (%a1)
	nop
	move.l	#(CONFIG_SYS_SDRAM_EMOD), (%a1)
	nop

	move.l	#1000, %d0
wait1000:
	nop
	subq.l	#1, %d0
	bne	wait1000

	/* Issue PALL */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 2), (%a2)
	nop

	/* Perform two refresh cycles */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 4), %d0
	nop
	move.l	%d0, (%a2)
	move.l	%d0, (%a2)
	nop

	move.l	#(CONFIG_SYS_SDRAM_CTRL), %d0
	and.l	#0x7FFFFFFF, %d0
	or.l	#0x10000c00, %d0
	move.l	%d0, (%a2)
	nop

	/*
	 * DSPI Initialization
	 * a0 - general, sram - 0x80008000 - 32, see M52277EVB.h
	 * a1 - dspi status
	 * a2 - dtfr
	 * a3 - drfr
	 * a4 - Dst addr
	 */

	/* Enable pins for DSPI mode - chip-selects are enabled later */
	move.l	#0xFC0A4036, %a0
	move.b	#0x3F, %d0
	move.b	%d0, (%a0)

	/* DSPI CS */
#ifdef CONFIG_SYS_DSPI_CS0
	move.b	(%a0), %d0
	or.l	#0xC0, %d0
	move.b	%d0, (%a0)
#endif
#ifdef CONFIG_SYS_DSPI_CS2
	move.l	#0xFC0A4037, %a0
	move.b	(%a0), %d0
	or.l	#0x10, %d0
	move.b	%d0, (%a0)
#endif
	nop

	/* Configure DSPI module */
	move.l	#0xFC05C000, %a0
	move.l	#0x80FF0C00, (%a0)	/* Master, clear TX/RX FIFO */

	move.l	#0xFC05C00C, %a0
	move.l	#0x3E000011, (%a0)

	move.l	#0xFC05C034, %a2	/* dtfr */
	move.l	#0xFC05C03B, %a3	/* drfr */

	move.l	#(ASM_SBF_IMG_HDR + 4), %a1
	move.l	(%a1)+, %d5
	move.l	(%a1), %a4

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_SBFHDR_DATA_OFFSET), %a0
	move.l	#(CONFIG_SYS_SBFHDR_SIZE), %d4

	move.l	#0xFC05C02C, %a1	/* dspi status */

	/* Issue commands and address */
	move.l	#0x8004000B, %d2	/* Fast Read Cmd */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80040000, %d2	/* Address byte 2 */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80040000, %d2	/* Address byte 1 */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80040000, %d2	/* Address byte 0 */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80040000, %d2	/* Dummy Wr and Rd */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	/* Transfer serial boot header to sram */
asm_dspi_rd_loop1:
	move.l	#0x80040000, %d2
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.b	%d1, (%a0)		/* read, copy to dst */

	add.l	#1, %a0			/* inc dst by 1 */
	sub.l	#1, %d4			/* dec cnt by 1 */
	bne	asm_dspi_rd_loop1

	/* Transfer u-boot from serial flash to memory */
asm_dspi_rd_loop2:
	move.l	#0x80040000, %d2
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.b	%d1, (%a4)		/* read, copy to dst */

	add.l	#1, %a4			/* inc dst by 1 */
	sub.l	#1, %d5			/* dec cnt by 1 */
	bne	asm_dspi_rd_loop2

	move.l	#0x00040000, %d2	/* Terminate */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	/* jump to memory and execute */
	move.l	#(CONFIG_SYS_TEXT_BASE + 0x400), %a0
	move.l	%a0, (%a1)
	jmp	(%a0)

asm_dspi_wr_status:
	move.l	(%a1), %d0		/* status */
	and.l	#0x0000F000, %d0
	cmp.l	#0x00003000, %d0
	bgt	asm_dspi_wr_status

	move.l	%d2, (%a2)
	rts

asm_dspi_rd_status:
	move.l	(%a1), %d0		/* status */
	and.l	#0x000000F0, %d0
	lsr.l	#4, %d0
	cmp.l	#0, %d0
	beq	asm_dspi_rd_status

	move.b	(%a3), %d1
	rts
#endif /* CONFIG_CF_SBF */

.text
	. = 0x400
.globl _start
_start:
	nop
	nop
	move.w	#0x2700,%sr		/* Mask off Interrupt */

	/* Set vector base register at the beginning of the Flash */
#if defined(CONFIG_CF_SBF)
	move.l	#CONFIG_SYS_TEXT_BASE, %d0
	movec	%d0, %VBR
#else
	move.l	#CONFIG_SYS_FLASH_BASE, %d0
	movec	%d0, %VBR

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_CTRL), %d0
	movec	%d0, %RAMBAR1
#endif

	/* invalidate and disable cache */
	move.l	#CF_CACR_CINV, %d0	/* Invalidate cache cmd */
	movec	%d0, %CACR		/* Invalidate cache */
	move.l	#0, %d0
	movec	%d0, %ACR0
	movec	%d0, %ACR1

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
	bsr	board_init_f_alloc_reserve

	/* update stack and frame-pointers */
	move.l	%d0, %sp
	move.l	%sp, %fp

	/* initialize reserved area */
	move.l	%d0, -(%sp)
	bsr	board_init_f_init_reserve

	/* run low-level CPU init code (from flash) */
	bsr	cpu_init_f
	clr.l	%sp@-

	/* run low-level board init code (from flash) */
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
