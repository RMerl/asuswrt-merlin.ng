/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  U-Boot - x86 Startup Code
 *
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

#include <config.h>
#include <asm/global_data.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <generated/generic-asm-offsets.h>
#include <generated/asm-offsets.h>
#include <linux/linkage.h>

.section .text.start
.code32
.globl _start
.type _start, @function
.globl _x86boot_start
_x86boot_start:
	/*
	 * This is the fail-safe 32-bit bootstrap entry point.
	 *
	 * This code is used when booting from another boot loader like
	 * coreboot or EFI. So we repeat some of the same init found in
	 * start16.
	 */
	cli
	cld

	/* Turn off cache (this might require a 486-class CPU) */
	movl	%cr0, %eax
	orl	$(X86_CR0_NW | X86_CR0_CD), %eax
	movl	%eax, %cr0
	wbinvd

	/* Tell 32-bit code it is being entered from an in-RAM copy */
	movl	$GD_FLG_WARM_BOOT, %ebx

	/*
	 * Zero the BIST (Built-In Self Test) value since we don't have it.
	 * It must be 0 or the previous loader would have reported an error.
	 */
	movl	$0, %ebp

	jmp	1f

	/* Add a way for tools to discover the _start entry point */
	.align	4
	.long	0x12345678
_start:
	/*
	 * This is the 32-bit cold-reset entry point, coming from start16.
	 * Set %ebx to GD_FLG_COLD_BOOT to indicate this.
	 */
	movl	$GD_FLG_COLD_BOOT, %ebx

	/* Save BIST */
	movl	%eax, %ebp
1:

	/* Save table pointer */
	movl	%ecx, %esi

#ifdef CONFIG_X86_LOAD_FROM_32_BIT
	lgdt	gdt_ptr2
#endif

	/* Load the segement registers to match the GDT loaded in start16.S */
	movl	$(X86_GDT_ENTRY_32BIT_DS * X86_GDT_ENTRY_SIZE), %eax
	movw	%ax, %fs
	movw	%ax, %ds
	movw	%ax, %gs
	movw	%ax, %es
	movw	%ax, %ss

	/* Clear the interrupt vectors */
	lidt	blank_idt_ptr

	/*
	 * Critical early platform init - generally not used, we prefer init
	 * to happen later when we have a console, in case something goes
	 * wrong.
	 */
	jmp	early_board_init
.globl early_board_init_ret
early_board_init_ret:
	post_code(POST_START)

	/* Initialise Cache-As-RAM */
	jmp	car_init
.globl car_init_ret
car_init_ret:
#ifndef CONFIG_HAVE_FSP
	/*
	 * We now have CONFIG_SYS_CAR_SIZE bytes of Cache-As-RAM (or SRAM,
	 * or fully initialised SDRAM - we really don't care which)
	 * starting at CONFIG_SYS_CAR_ADDR to be used as a temporary stack
	 * and early malloc() area. The MRC requires some space at the top.
	 *
	 * Stack grows down from top of CAR. We have:
	 *
	 * top-> CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE
	 *	MRC area
	 *	global_data with x86 global descriptor table
	 *	early malloc area
	 *	stack
	 * bottom-> CONFIG_SYS_CAR_ADDR
	 */
	movl	$(CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE - 4), %esp
#ifdef CONFIG_DCACHE_RAM_MRC_VAR_SIZE
	subl	$CONFIG_DCACHE_RAM_MRC_VAR_SIZE, %esp
#endif
#else
	/*
	 * U-Boot enters here twice. For the first time it comes from
	 * car_init_done() with esp points to a temporary stack and esi
	 * set to zero. For the second time it comes from fsp_init_done()
	 * with esi holding the HOB list address returned by the FSP.
	 */
#endif
	/* Set up global data */
	mov	%esp, %eax
	call	board_init_f_alloc_reserve
	mov	%eax, %esp
	call	board_init_f_init_reserve

#ifdef CONFIG_DEBUG_UART
	call	debug_uart_init
#endif

	/* Get address of global_data */
	mov	%fs:0, %edx
#ifdef CONFIG_HAVE_FSP
	/* Store the HOB list if we have one */
	test	%esi, %esi
	jz	skip_hob
	movl	%esi, GD_HOB_LIST(%edx)

	/*
	 * After fsp_init() returns, the stack has already been switched to a
	 * place within system memory as defined by CONFIG_FSP_TEMP_RAM_ADDR.
	 * Enlarge the size of malloc() pool before relocation since we have
	 * plenty of memory now.
	 */
	subl	$CONFIG_FSP_SYS_MALLOC_F_LEN, %esp
	movl	%esp, GD_MALLOC_BASE(%edx)
skip_hob:
#else
	/* Store table pointer */
	movl	%esi, GD_TABLE(%edx)
#endif
	/* Store BIST */
	movl	%ebp, GD_BIST(%edx)

	/* Set parameter to board_init_f() to boot flags */
	post_code(POST_START_DONE)
	xorl	%eax, %eax

	/* Enter, U-Boot! */
	call	board_init_f

	/* indicate (lack of) progress */
	movw	$0x85, %ax
	jmp	die

.globl board_init_f_r_trampoline
.type board_init_f_r_trampoline, @function
board_init_f_r_trampoline:
	/*
	 * SDRAM has been initialised, U-Boot code has been copied into
	 * RAM, BSS has been cleared and relocation adjustments have been
	 * made. It is now time to jump into the in-RAM copy of U-Boot
	 *
	 * %eax = Address of top of new stack
	 */

	/* Stack grows down from top of SDRAM */
	movl	%eax, %esp

	/* See if we need to disable CAR */
	call	car_uninit

	/* Re-enter U-Boot by calling board_init_f_r() */
	call	board_init_f_r

#ifdef CONFIG_TPL
.globl jump_to_spl
.type jump_to_spl, @function
jump_to_spl:
	/* Reset stack to the top of CAR space */
	movl	$(CONFIG_SYS_CAR_ADDR + CONFIG_SYS_CAR_SIZE - 4), %esp
#ifdef CONFIG_DCACHE_RAM_MRC_VAR_SIZE
	subl	$CONFIG_DCACHE_RAM_MRC_VAR_SIZE, %esp
#endif

	jmp	*%eax
#endif

die:
	hlt
	jmp	die
	hlt

WEAK(car_uninit)
	ret
ENDPROC(car_uninit)

blank_idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */

	.p2align	2	/* force 4-byte alignment */

	/* Add a multiboot header so U-Boot can be loaded by GRUB2 */
multiboot_header:
	/* magic */
	.long	0x1badb002
	/* flags */
	.long	(1 << 16)
	/* checksum */
	.long	-0x1BADB002 - (1 << 16)
	/* header addr */
	.long	multiboot_header - _x86boot_start + CONFIG_SYS_TEXT_BASE
	/* load addr */
	.long	CONFIG_SYS_TEXT_BASE
	/* load end addr */
	.long	0
	/* bss end addr */
	.long	0
	/* entry addr */
	.long	CONFIG_SYS_TEXT_BASE

#ifdef CONFIG_X86_LOAD_FROM_32_BIT
	/*
	 * The following Global Descriptor Table is just enough to get us into
	 * 'Flat Protected Mode' - It will be discarded as soon as the final
	 * GDT is setup in a safe location in RAM
	 */
gdt_ptr2:
	.word	0x1f		/* limit (31 bytes = 4 GDT entries - 1) */
	.long	gdt_rom2	/* base */

	/* Some CPUs are picky about GDT alignment... */
	.align	16
.globl gdt_rom2
gdt_rom2:
	/*
	 * The GDT table ...
	 *
	 *	 Selector	Type
	 *	 0x00		NULL
	 *	 0x08		Unused
	 *	 0x10		32bit code
	 *	 0x18		32bit data/stack
	 */
	/* The NULL Desciptor - Mandatory */
	.word	0x0000		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x00		/* access */
	.byte	0x00		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/* Unused Desciptor - (matches Linux) */
	.word	0x0000		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x00		/* access */
	.byte	0x00		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/*
	 * The Code Segment Descriptor:
	 * - Base   = 0x00000000
	 * - Size   = 4GB
	 * - Access = Present, Ring 0, Exec (Code), Readable
	 * - Flags  = 4kB Granularity, 32-bit
	 */
	.word	0xffff		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x9b		/* access */
	.byte	0xcf		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/*
	 * The Data Segment Descriptor:
	 * - Base   = 0x00000000
	 * - Size   = 4GB
	 * - Access = Present, Ring 0, Non-Exec (Data), Writable
	 * - Flags  = 4kB Granularity, 32-bit
	 */
	.word	0xffff		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x93		/* access */
	.byte	0xcf		/* flags + limit_high */
	.byte	0x00		/* base_high */
#endif
