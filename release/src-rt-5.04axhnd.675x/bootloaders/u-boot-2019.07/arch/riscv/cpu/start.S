/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Startup Code for RISC-V Core
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Copyright (c) 2017 Padmarao Begari <Padmarao.Begari@microsemi.com>
 *
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <common.h>
#include <elf.h>
#include <asm/csr.h>
#include <asm/encoding.h>
#include <generated/asm-offsets.h>

#ifdef CONFIG_32BIT
#define LREG			lw
#define SREG			sw
#define REGBYTES		4
#define RELOC_TYPE		R_RISCV_32
#define SYM_INDEX		0x8
#define SYM_SIZE		0x10
#else
#define LREG			ld
#define SREG			sd
#define REGBYTES		8
#define RELOC_TYPE		R_RISCV_64
#define SYM_INDEX		0x20
#define SYM_SIZE		0x18
#endif

.section .data
secondary_harts_relocation_error:
	.ascii "Relocation of secondary harts has failed, error %d\n"

.section .text
.globl _start
_start:
#ifdef CONFIG_RISCV_MMODE
	csrr	a0, mhartid
#endif

	/* save hart id and dtb pointer */
	mv	tp, a0
	mv	s1, a1

	la	t0, trap_entry
	csrw	MODE_PREFIX(tvec), t0

	/* mask all interrupts */
	csrw	MODE_PREFIX(ie), zero

#ifdef CONFIG_SMP
	/* check if hart is within range */
	/* tp: hart id */
	li	t0, CONFIG_NR_CPUS
	bge	tp, t0, hart_out_of_bounds_loop
#endif

#ifdef CONFIG_SMP
	/* set xSIE bit to receive IPIs */
#ifdef CONFIG_RISCV_MMODE
	li	t0, MIE_MSIE
#else
	li	t0, SIE_SSIE
#endif
	csrs	MODE_PREFIX(ie), t0
#endif

/*
 * Set stackpointer in internal/ex RAM to call board_init_f
 */
call_board_init_f:
	li	t0, -16
	li	t1, CONFIG_SYS_INIT_SP_ADDR
	and	sp, t1, t0		/* force 16 byte alignment */

call_board_init_f_0:
	mv	a0, sp
	jal	board_init_f_alloc_reserve

	/*
	 * Set global data pointer here for all harts, uninitialized at this
	 * point.
	 */
	mv	gp, a0

	/* setup stack */
#ifdef CONFIG_SMP
	/* tp: hart id */
	slli	t0, tp, CONFIG_STACK_SIZE_SHIFT
	sub	sp, a0, t0
#else
	mv	sp, a0
#endif

#ifndef CONFIG_XIP
	/*
	 * Pick hart to initialize global data and run U-Boot. The other harts
	 * wait for initialization to complete.
	 */
	la	t0, hart_lottery
	li	s2, 1
	amoswap.w s2, t1, 0(t0)
	bnez	s2, wait_for_gd_init
#else
	bnez	tp, secondary_hart_loop
#endif

#ifdef CONFIG_OF_PRIOR_STAGE
	la	t0, prior_stage_fdt_address
	SREG	s1, 0(t0)
#endif

	jal	board_init_f_init_reserve

	/* save the boot hart id to global_data */
	SREG	tp, GD_BOOT_HART(gp)

#ifndef CONFIG_XIP
	la	t0, available_harts_lock
	fence	rw, w
	amoswap.w zero, zero, 0(t0)

wait_for_gd_init:
	la	t0, available_harts_lock
	li	t1, 1
1:	amoswap.w t1, t1, 0(t0)
	fence	r, rw
	bnez	t1, 1b

	/* register available harts in the available_harts mask */
	li	t1, 1
	sll	t1, t1, tp
	LREG	t2, GD_AVAILABLE_HARTS(gp)
	or	t2, t2, t1
	SREG	t2, GD_AVAILABLE_HARTS(gp)

	fence	rw, w
	amoswap.w zero, zero, 0(t0)

	/*
	 * Continue on hart lottery winner, others branch to
	 * secondary_hart_loop.
	 */
	bnez	s2, secondary_hart_loop
#endif

	/* Enable cache */
	jal	icache_enable
	jal	dcache_enable

#ifdef CONFIG_DEBUG_UART
	jal	debug_uart_init
#endif

	mv	a0, zero		/* a0 <-- boot_flags = 0 */
	la	t5, board_init_f
	jr	t5			/* jump to board_init_f() */

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 */
.globl relocate_code
relocate_code:
	mv	s2, a0			/* save addr_sp */
	mv	s3, a1			/* save addr of gd */
	mv	s4, a2			/* save addr of destination */

/*
 *Set up the stack
 */
stack_setup:
#ifdef CONFIG_SMP
	/* tp: hart id */
	slli	t0, tp, CONFIG_STACK_SIZE_SHIFT
	sub	sp, s2, t0
#else
	mv	sp, s2
#endif

	la	t0, _start
	sub	t6, s4, t0		/* t6 <- relocation offset */
	beq	t0, s4, clear_bss	/* skip relocation */

	mv	t1, s4			/* t1 <- scratch for copy_loop */
	la	t3, __bss_start
	sub	t3, t3, t0		/* t3 <- __bss_start_ofs */
	add	t2, t0, t3		/* t2 <- source end address */

copy_loop:
	LREG	t5, 0(t0)
	addi	t0, t0, REGBYTES
	SREG	t5, 0(t1)
	addi	t1, t1, REGBYTES
	blt	t0, t2, copy_loop

/*
 * Update dynamic relocations after board_init_f
 */
fix_rela_dyn:
	la	t1, __rel_dyn_start
	la	t2, __rel_dyn_end
	beq	t1, t2, clear_bss
	add	t1, t1, t6		/* t1 <- rela_dyn_start in RAM */
	add	t2, t2, t6		/* t2 <- rela_dyn_end in RAM */

/*
 * skip first reserved entry: address, type, addend
 */
	bne	t1, t2, 7f

6:
	LREG	t5, -(REGBYTES*2)(t1)	/* t5 <-- relocation info:type */
	li	t3, R_RISCV_RELATIVE	/* reloc type R_RISCV_RELATIVE */
	bne	t5, t3, 8f		/* skip non-RISCV_RELOC entries */
	LREG	t3, -(REGBYTES*3)(t1)
	LREG	t5, -(REGBYTES)(t1)	/* t5 <-- addend */
	add	t5, t5, t6		/* t5 <-- location to fix up in RAM */
	add	t3, t3, t6		/* t3 <-- location to fix up in RAM */
	SREG	t5, 0(t3)
7:
	addi	t1, t1, (REGBYTES*3)
	ble	t1, t2, 6b

8:
	la	t4, __dyn_sym_start
	add	t4, t4, t6

9:
	LREG	t5, -(REGBYTES*2)(t1)	/* t5 <-- relocation info:type */
	srli	t0, t5, SYM_INDEX	/* t0 <--- sym table index */
	andi	t5, t5, 0xFF		/* t5 <--- relocation type */
	li	t3, RELOC_TYPE
	bne	t5, t3, 10f		/* skip non-addned entries */

	LREG	t3, -(REGBYTES*3)(t1)
	li	t5, SYM_SIZE
	mul	t0, t0, t5
	add	s5, t4, t0
	LREG	t5, REGBYTES(s5)
	add	t5, t5, t6		/* t5 <-- location to fix up in RAM */
	add	t3, t3, t6		/* t3 <-- location to fix up in RAM */
	SREG	t5, 0(t3)
10:
	addi	t1, t1, (REGBYTES*3)
	ble	t1, t2, 9b

/*
 * trap update
*/
	la	t0, trap_entry
	add	t0, t0, t6
	csrw	MODE_PREFIX(tvec), t0

clear_bss:
	la	t0, __bss_start		/* t0 <- rel __bss_start in FLASH */
	add	t0, t0, t6		/* t0 <- rel __bss_start in RAM */
	la	t1, __bss_end		/* t1 <- rel __bss_end in FLASH */
	add	t1, t1, t6		/* t1 <- rel __bss_end in RAM */
	beq	t0, t1, relocate_secondary_harts

clbss_l:
	SREG	zero, 0(t0)		/* clear loop... */
	addi	t0, t0, REGBYTES
	bne	t0, t1, clbss_l

relocate_secondary_harts:
#ifdef CONFIG_SMP
	/* send relocation IPI */
	la	t0, secondary_hart_relocate
	add	a0, t0, t6

	/* store relocation offset */
	mv	s5, t6

	mv	a1, s2
	mv	a2, s3
	jal	smp_call_function

	/* hang if relocation of secondary harts has failed */
	beqz	a0, 1f
	mv	a1, a0
	la	a0, secondary_harts_relocation_error
	jal	printf
	jal	hang

	/* restore relocation offset */
1:	mv	t6, s5
#endif

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
call_board_init_r:
	jal	invalidate_icache_all
	jal	flush_dcache_all
	la	t0, board_init_r
	mv	t4, t0			/* offset of board_init_r() */
	add	t4, t4, t6		/* real address of board_init_r() */
/*
 * setup parameters for board_init_r
 */
	mv	a0, s3			/* gd_t */
	mv	a1, s4			/* dest_addr */

/*
 * jump to it ...
 */
	jr	t4			/* jump to board_init_r() */

#ifdef CONFIG_SMP
hart_out_of_bounds_loop:
	/* Harts in this loop are out of bounds, increase CONFIG_NR_CPUS. */
	wfi
	j	hart_out_of_bounds_loop
#endif

#ifdef CONFIG_SMP
/* SMP relocation entry */
secondary_hart_relocate:
	/* a1: new sp */
	/* a2: new gd */
	/* tp: hart id */

	/* setup stack */
	slli	t0, tp, CONFIG_STACK_SIZE_SHIFT
	sub	sp, a1, t0

	/* update global data pointer */
	mv	gp, a2
#endif

secondary_hart_loop:
	wfi

#ifdef CONFIG_SMP
	csrr	t0, MODE_PREFIX(ip)
#ifdef CONFIG_RISCV_MMODE
	andi	t0, t0, MIE_MSIE
#else
	andi	t0, t0, SIE_SSIE
#endif
	beqz	t0, secondary_hart_loop

	mv	a0, tp
	jal	handle_ipi
#endif

	j	secondary_hart_loop
