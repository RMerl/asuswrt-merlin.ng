/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

#include <asm-offsets.h>
#include <config.h>

	.text
	.global _start
_start:
	/*
	 * reserve registers:
	 * r10: Stores little/big endian offset for vectors
	 * r2: Stores imm opcode
	 * r3: Stores brai opcode
	 */

	mts	rmsr, r0	/* disable cache */

	addi	r8, r0, __end
	mts	rslr, r8
	/* TODO: Redo this code to call board_init_f_*() */
#if defined(CONFIG_SPL_BUILD)
	addi	r1, r0, CONFIG_SPL_STACK_ADDR
	mts	rshr, r1
	addi	r1, r1, -4	/* Decrement SP to top of memory */
#else
#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	addi	r1, r0, CONFIG_SYS_INIT_SP_OFFSET - CONFIG_VAL(SYS_MALLOC_F_LEN)
#else
	addi	r1, r0, CONFIG_SYS_INIT_SP_OFFSET
#endif
	mts	rshr, r1
	addi	r1, r1, -4	/* Decrement SP to top of memory */

	/* Find-out if u-boot is running on BIG/LITTLE endian platform
	 * There are some steps which is necessary to keep in mind:
	 * 1. Setup offset value to r6
	 * 2. Store word offset value to address 0x0
	 * 3. Load just byte from address 0x0
	 * 4a) LITTLE endian - r10 contains 0x2 because it is the smallest
	 *     value that's why is on address 0x0
	 * 4b) BIG endian - r10 contains 0x0 because 0x2 offset is on addr 0x3
	 */
	addik	r6, r0, 0x2 /* BIG/LITTLE endian offset */
	lwi	r7, r0, 0x28
	swi	r6, r0, 0x28 /* used first unused MB vector */
	lbui	r10, r0, 0x28 /* used first unused MB vector */
	swi	r7, r0, 0x28

	/* add opcode instruction for 32bit jump - 2 instruction imm & brai */
	addi	r2, r0, 0xb0000000	/* hex b000 opcode imm */
	addi	r3, r0, 0xb8080000	/* hew b808 opcode brai */

#ifdef CONFIG_SYS_RESET_ADDRESS
	/* reset address */
	swi	r2, r0, 0x0	/* reset address - imm opcode */
	swi	r3, r0, 0x4	/* reset address - brai opcode */

	addik	r6, r0, CONFIG_SYS_RESET_ADDRESS
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x2
	sh	r7, r0, r8
	rsubi	r8, r10, 0x6
	sh	r6, r0, r8
#endif

#ifdef CONFIG_SYS_USR_EXCEP
	/* user_vector_exception */
	swi	r2, r0, 0x8	/* user vector exception - imm opcode */
	swi	r3, r0, 0xC	/* user vector exception - brai opcode */

	addik	r6, r0, _exception_handler
	sw	r6, r1, r0
	/*
	 * BIG ENDIAN memory map for user exception
	 * 0x8: 0xB000XXXX
	 * 0xC: 0xB808XXXX
	 *
	 * then it is necessary to count address for storing the most significant
	 * 16bits from _exception_handler address and copy it to
	 * 0xa address. Big endian use offset in r10=0 that's why is it just
	 * 0xa address. The same is done for the least significant 16 bits
	 * for 0xe address.
	 *
	 * LITTLE ENDIAN memory map for user exception
	 * 0x8: 0xXXXX00B0
	 * 0xC: 0xXXXX08B8
	 *
	 * Offset is for little endian setup to 0x2. rsubi instruction decrease
	 * address value to ensure that points to proper place which is
	 * 0x8 for the most significant 16 bits and
	 * 0xC for the least significant 16 bits
	 */
	lhu	r7, r1, r10
	rsubi	r8, r10, 0xa
	sh	r7, r0, r8
	rsubi	r8, r10, 0xe
	sh	r6, r0, r8
#endif

	/* interrupt_handler */
	swi	r2, r0, 0x10	/* interrupt - imm opcode */
	swi	r3, r0, 0x14	/* interrupt - brai opcode */

	addik	r6, r0, _interrupt_handler
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x12
	sh	r7, r0, r8
	rsubi	r8, r10, 0x16
	sh	r6, r0, r8

	/* hardware exception */
	swi	r2, r0, 0x20	/* hardware exception - imm opcode */
	swi	r3, r0, 0x24	/* hardware exception - brai opcode */

	addik	r6, r0, _hw_exception_handler
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x22
	sh	r7, r0, r8
	rsubi	r8, r10, 0x26
	sh	r6, r0, r8
#endif /* CONFIG_SPL_BUILD */

	/* Flush cache before enable cache */
	addik	r5, r0, 0
	addik	r6, r0, XILINX_DCACHE_BYTE_SIZE
	bralid r15, flush_cache
	nop

	/* enable instruction and data cache */
	mfs	r12, rmsr
	ori	r12, r12, 0x1a0
	mts	rmsr, r12

	/* TODO: Redo this code to call board_init_f_*() */
clear_bss:
	/* clear BSS segments */
	addi	r5, r0, __bss_start
	addi	r4, r0, __bss_end
	cmp	r6, r5, r4
	beqi	r6, 3f
2:
	swi     r0, r5, 0 /* write zero to loc */
	addi    r5, r5, 4 /* increment to next loc */
	cmp     r6, r5, r4 /* check if we have reach the end */
	bnei    r6, 2b
3:	/* jumping to board_init */
#ifdef CONFIG_DEBUG_UART
	bralid	r15, debug_uart_init
	nop
#endif
#ifndef CONFIG_SPL_BUILD
	or	r5, r0, r0	/* flags - empty */
	addi    r31, r0, _gd
#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	addi	r6, r0, CONFIG_SYS_INIT_SP_OFFSET
	swi	r6, r31, GD_MALLOC_BASE
#endif
	brai	board_init_f
#else
	addi	r31, r0, _gd
#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	addi	r6, r0, CONFIG_SPL_STACK_ADDR
	swi	r6, r31, GD_MALLOC_BASE
#endif
	brai	board_init_r
#endif
1:	bri	1b

 .section .bss
.align 4
_gd:
         .space  GENERATED_GBL_DATA_SIZE

#ifndef CONFIG_SPL_BUILD
/*
 * Read 16bit little endian
 */
	.text
	.global	in16
	.ent	in16
	.align	2
in16:	lhu	r3, r0, r5
	bslli	r4, r3, 8
	bsrli	r3, r3, 8
	andi	r4, r4, 0xffff
	or	r3, r3, r4
	rtsd	r15, 8
	sext16	r3, r3
	.end	in16

/*
 * Write 16bit little endian
 * first parameter(r5) - address, second(r6) - short value
 */
	.text
	.global	out16
	.ent	out16
	.align	2
out16:	bslli	r3, r6, 8
	bsrli	r6, r6, 8
	andi	r3, r3, 0xffff
	or	r3, r3, r6
	sh	r3, r0, r5
	rtsd	r15, 8
	or	r0, r0, r0
	.end	out16

/*
 * Relocate u-boot
 */
	.text
	.global	relocate_code
	.ent	relocate_code
	.align	2
relocate_code:
	/*
	 * r5 - start_addr_sp
	 * r6 - new_gd
	 * r7 - reloc_addr
	 */
	addi	r1, r5, 0 /* Start to use new SP */
	addi	r31, r6, 0 /* Start to use new GD */

	add	r23, r0, r7 /* Move reloc addr to r23 */
	/* Relocate text and data - r12 temp value */
	addi	r21, r0, _start
	addi	r22, r0, __end - 4 /* Include BSS too */

	rsub	r6, r21, r22
	or	r5, r0, r0
1:	lw	r12, r21, r5 /* Load u-boot data */
	sw	r12, r23, r5 /* Write zero to loc */
	cmp	r12, r5, r6 /* Check if we have reach the end */
	bneid	r12, 1b
	addi	r5, r5, 4 /* Increment to next loc - relocate code */

       /* R23 points to the base address. */
	add	r23, r0, r7 /* Move reloc addr to r23 */
	addi	r24, r0, CONFIG_SYS_TEXT_BASE /* Get reloc offset */
	rsub	r23, r24, r23 /* keep - this is already here gd->reloc_off */

	addik	r6, r0, 0x2 /* BIG/LITTLE endian offset */
	lwi	r7, r0, 0x28
	swi	r6, r0, 0x28 /* used first unused MB vector */
	lbui	r10, r0, 0x28 /* used first unused MB vector */
	swi	r7, r0, 0x28

#ifdef CONFIG_SYS_USR_EXCEP
	addik	r6, r0, _exception_handler
	addk	r6, r6, r23 /* add offset */
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0xa
	sh	r7, r0, r8
	rsubi	r8, r10, 0xe
	sh	r6, r0, r8
#endif
	addik	r6, r0, _hw_exception_handler
	addk	r6, r6, r23 /* add offset */
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x22
	sh	r7, r0, r8
	rsubi	r8, r10, 0x26
	sh	r6, r0, r8

	addik	r6, r0, _interrupt_handler
	addk	r6, r6, r23 /* add offset */
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x12
	sh	r7, r0, r8
	rsubi	r8, r10, 0x16
	sh	r6, r0, r8

	/* Check if GOT exist */
	addik	r21, r23, _got_start
	addik	r22, r23, _got_end
	cmpu	r12, r21, r22
	beqi	r12, 2f /* No GOT table - jump over */

	/* Skip last 3 entries plus 1 because of loop boundary below */
	addik	r22, r22, -0x10

        /* Relocate the GOT. */
3:	lw	r12, r21, r0 /* Load entry */
	addk	r12, r12, r23 /* Add reloc offset */
	sw	r12, r21, r0 /* Save entry back */

	cmpu	r12, r21, r22 /* Check if this cross boundary */
	bneid	r12, 3b
	addik	r21. r21, 4

	/* Update pointer to GOT */
	mfs	r20, rpc
	addik	r20, r20, _GLOBAL_OFFSET_TABLE_ + 8
	addk	r20, r20, r23

	/* Flush caches to ensure consistency */
	addik	r5, r0, 0
	addik	r6, r0, XILINX_DCACHE_BYTE_SIZE
	bralid	r15, flush_cache
	nop

2:	addi	r5, r31, 0 /* gd is initialized in board_r.c */
	addi	r6, r0, CONFIG_SYS_TEXT_BASE
	addi	r12, r23, board_init_r
	bra	r12 /* Jump to relocated code */

	.end	relocate_code
#endif
