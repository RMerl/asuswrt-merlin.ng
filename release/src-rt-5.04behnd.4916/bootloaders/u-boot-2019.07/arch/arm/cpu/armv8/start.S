/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/armv8/mmu.h>

/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 *************************************************************************/

.globl	_start
_start:
#if defined(CONFIG_LINUX_KERNEL_IMAGE_HEADER)
#include <asm/boot0-linux-kernel-header.h>
#elif defined(CONFIG_ENABLE_ARM_SOC_BOOT0_HOOK)
/*
 * Various SoCs need something special and SoC-specific up front in
 * order to boot, allow them to set that in their boot0.h file and then
 * use it here.
 */
#include <asm/arch/boot0.h>
#else
	b	reset
#endif

	.align 3

.globl	_TEXT_BASE
_TEXT_BASE:
	.quad	CONFIG_SYS_TEXT_BASE

/*
 * These are defined in the linker script.
 */
.globl	_end_ofs
_end_ofs:
	.quad	_end - _start

.globl	_bss_start_ofs
_bss_start_ofs:
	.quad	__bss_start - _start

.globl	_bss_end_ofs
_bss_end_ofs:
	.quad	__bss_end - _start

reset:
	/* Allow the board to save important registers */
	b	save_boot_params
.globl	save_boot_params_ret
save_boot_params_ret:

#if CONFIG_POSITION_INDEPENDENT
	/*
	 * Fix .rela.dyn relocations. This allows U-Boot to be loaded to and
	 * executed at a different address than it was linked at.
	 */
pie_fixup:
	adr	x0, _start		/* x0 <- Runtime value of _start */
	ldr	x1, _TEXT_BASE		/* x1 <- Linked value of _start */
	sub	x9, x0, x1		/* x9 <- Run-vs-link offset */
	adr	x2, __rel_dyn_start	/* x2 <- Runtime &__rel_dyn_start */
	adr	x3, __rel_dyn_end	/* x3 <- Runtime &__rel_dyn_end */
pie_fix_loop:
	ldp	x0, x1, [x2], #16	/* (x0, x1) <- (Link location, fixup) */
	ldr	x4, [x2], #8		/* x4 <- addend */
	cmp	w1, #1027		/* relative fixup? */
	bne	pie_skip_reloc
	/* relative fix: store addend plus offset at dest location */
	add	x0, x0, x9
	add	x4, x4, x9
	str	x4, [x0]
pie_skip_reloc:
	cmp	x2, x3
	b.lo	pie_fix_loop
pie_fixup_done:
#endif

#ifdef CONFIG_SYS_RESET_SCTRL
	bl reset_sctrl
#endif

#if defined(CONFIG_ARMV8_SPL_EXCEPTION_VECTORS) || !defined(CONFIG_SPL_BUILD)
.macro	set_vbar, regname, reg
	msr	\regname, \reg
.endm
	adr	x0, vectors
#else
.macro	set_vbar, regname, reg
.endm
#endif
	/*
	 * Could be EL3/EL2/EL1, Initial State:
	 * Little Endian, MMU Disabled, i/dCache Disabled
	 */
	switch_el x1, 3f, 2f, 1f
3:	set_vbar vbar_el3, x0
	mrs	x0, scr_el3
	orr	x0, x0, #0xf			/* SCR_EL3.NS|IRQ|FIQ|EA */
	msr	scr_el3, x0
	msr	cptr_el3, xzr			/* Enable FP/SIMD */
#ifdef COUNTER_FREQUENCY
	ldr	x0, =COUNTER_FREQUENCY
	msr	cntfrq_el0, x0			/* Initialize CNTFRQ */
#endif
	b	0f
2:	set_vbar	vbar_el2, x0
	mov	x0, #0x33ff
	msr	cptr_el2, x0			/* Enable FP/SIMD */
	b	0f
1:	set_vbar	vbar_el1, x0
	mov	x0, #3 << 20
	msr	cpacr_el1, x0			/* Enable FP/SIMD */
0:

	/*
	 * Enable SMPEN bit for coherency.
	 * This register is not architectural but at the moment
	 * this bit should be set for A53/A57/A72.
	 */
#ifdef CONFIG_ARMV8_SET_SMPEN
	switch_el x1, 3f, 1f, 1f
3:
	mrs     x0, S3_1_c15_c2_1               /* cpuectlr_el1 */
	orr     x0, x0, #0x40
	msr     S3_1_c15_c2_1, x0
1:
#endif

	/* Apply ARM core specific erratas */
	bl	apply_core_errata

	/*
	 * Cache/BPB/TLB Invalidate
	 * i-cache is invalidated before enabled in icache_enable()
	 * tlb is invalidated before mmu is enabled in dcache_enable()
	 * d-cache is invalidated before enabled in dcache_enable()
	 */

	/* Processor specific initialization */
	bl	lowlevel_init

#if defined(CONFIG_ARMV8_SPIN_TABLE) && !defined(CONFIG_SPL_BUILD)
	branch_if_master x0, x1, master_cpu
	b	spin_table_secondary_jump
	/* never return */
#elif defined(CONFIG_ARMV8_MULTIENTRY)
	branch_if_master x0, x1, master_cpu

	/*
	 * Slave CPUs
	 */
slave_cpu:
	wfe
	ldr	x1, =CPU_RELEASE_ADDR
	ldr	x0, [x1]
	cbz	x0, slave_cpu
	br	x0			/* branch to the given address */
#endif /* CONFIG_ARMV8_MULTIENTRY */
master_cpu:
	bl	_main

#ifdef CONFIG_SYS_RESET_SCTRL
reset_sctrl:
	switch_el x1, 3f, 2f, 1f
3:
	mrs	x0, sctlr_el3
	b	0f
2:
	mrs	x0, sctlr_el2
	b	0f
1:
	mrs	x0, sctlr_el1

0:
	ldr	x1, =0xfdfffffa
	and	x0, x0, x1

	switch_el x1, 6f, 5f, 4f
6:
	msr	sctlr_el3, x0
	b	7f
5:
	msr	sctlr_el2, x0
	b	7f
4:
	msr	sctlr_el1, x0

7:
	dsb	sy
	isb
	b	__asm_invalidate_tlb_all
	ret
#endif

/*-----------------------------------------------------------------------*/

WEAK(apply_core_errata)

	mov	x29, lr			/* Save LR */
	/* For now, we support Cortex-A53, Cortex-A57 specific errata */

	/* Check if we are running on a Cortex-A53 core */
	branch_if_a53_core x0, apply_a53_core_errata

	/* Check if we are running on a Cortex-A57 core */
	branch_if_a57_core x0, apply_a57_core_errata
0:
	mov	lr, x29			/* Restore LR */
	ret

apply_a53_core_errata:

#ifdef CONFIG_ARM_ERRATA_855873
	mrs	x0, midr_el1
	tst	x0, #(0xf << 20)
	b.ne	0b

	mrs	x0, midr_el1
	and	x0, x0, #0xf
	cmp	x0, #3
	b.lt	0b

	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Enable data cache clean as data cache clean/invalidate */
	orr	x0, x0, #1 << 44
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
#endif
	b 0b

apply_a57_core_errata:

#ifdef CONFIG_ARM_ERRATA_828024
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable non-allocate hint of w-b-n-a memory type */
	orr	x0, x0, #1 << 49
	/* Disable write streaming no L1-allocate threshold */
	orr	x0, x0, #3 << 25
	/* Disable write streaming no-allocate threshold */
	orr	x0, x0, #3 << 27
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
#endif

#ifdef CONFIG_ARM_ERRATA_826974
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable speculative load execution ahead of a DMB */
	orr	x0, x0, #1 << 59
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
#endif

#ifdef CONFIG_ARM_ERRATA_833471
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* FPSCR write flush.
	 * Note that in some cases where a flush is unnecessary this
	    could impact performance. */
	orr	x0, x0, #1 << 38
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
#endif

#ifdef CONFIG_ARM_ERRATA_829520
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable Indirect Predictor bit will prevent this erratum
	    from occurring
	 * Note that in some cases where a flush is unnecessary this
	    could impact performance. */
	orr	x0, x0, #1 << 4
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
#endif

#ifdef CONFIG_ARM_ERRATA_833069
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable Enable Invalidates of BTB bit */
	and	x0, x0, #0xE
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
#endif
	b 0b
ENDPROC(apply_core_errata)

/*-----------------------------------------------------------------------*/

WEAK(lowlevel_init)
	mov	x29, lr			/* Save LR */

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

#ifdef CONFIG_ARMV8_MULTIENTRY
	branch_if_master x0, x1, 2f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

	/*
	 * All slaves will enter EL2 and optionally EL1.
	 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2

lowlevel_in_el2:
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	adr	x4, lowlevel_in_el1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el1

lowlevel_in_el1:
#endif

#endif /* CONFIG_ARMV8_MULTIENTRY */

2:
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)

WEAK(smp_kick_all_cpus)
	/* Kick secondary cpus up by SGI 0 interrupt */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	ldr	x0, =GICD_BASE
	b	gic_kick_secondary_cpus
#endif
	ret
ENDPROC(smp_kick_all_cpus)

/*-----------------------------------------------------------------------*/

ENTRY(c_runtime_cpu_setup)
#if defined(CONFIG_ARMV8_SPL_EXCEPTION_VECTORS) || !defined(CONFIG_SPL_BUILD)
	/* Relocate vBAR */
	adr	x0, vectors
	switch_el x1, 3f, 2f, 1f
3:	msr	vbar_el3, x0
	b	0f
2:	msr	vbar_el2, x0
	b	0f
1:	msr	vbar_el1, x0
0:
#endif

	ret
ENDPROC(c_runtime_cpu_setup)

WEAK(save_boot_params)
	b	save_boot_params_ret	/* back to my caller */
ENDPROC(save_boot_params)
