/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * code for switching cores into non-secure state and into HYP mode
 *
 * Copyright (c) 2013	Andre Przywara <andre.przywara@linaro.org>
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/gic.h>
#include <asm/armv7.h>
#include <asm/proc-armv/ptrace.h>

.arch_extension sec
.arch_extension virt

	.pushsection ._secure.text, "ax"

	.align	5
/* the vector table for secure state and HYP mode */
_monitor_vectors:
	.word 0	/* reset */
	.word 0 /* undef */
	adr pc, _secure_monitor
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0

.macro is_cpu_virt_capable	tmp
	mrc	p15, 0, \tmp, c0, c1, 1		@ read ID_PFR1
	and	\tmp, \tmp, #CPUID_ARM_VIRT_MASK	@ mask virtualization bits
	cmp	\tmp, #(1 << CPUID_ARM_VIRT_SHIFT)
.endm

/*
 * secure monitor handler
 * U-Boot calls this "software interrupt" in start.S
 * This is executed on a "smc" instruction, we use a "smc #0" to switch
 * to non-secure state.
 * r0, r1, r2: passed to the callee
 * ip: target PC
 */
_secure_monitor:
#ifdef CONFIG_ARMV7_PSCI
	ldr	r5, =_psci_vectors		@ Switch to the next monitor
	mcr	p15, 0, r5, c12, c0, 1
	isb

	@ Obtain a secure stack
	bl	psci_stack_setup

	@ Configure the PSCI backend
	push	{r0, r1, r2, ip}
	bl	psci_arch_init
	pop	{r0, r1, r2, ip}
#endif

#ifdef CONFIG_ARM_ERRATA_773022
	mrc	p15, 0, r5, c1, c0, 1
	orr	r5, r5, #(1 << 1)
	mcr	p15, 0, r5, c1, c0, 1
	isb
#endif

#ifdef CONFIG_ARM_ERRATA_774769
	mrc	p15, 0, r5, c1, c0, 1
	orr	r5, r5, #(1 << 25)
	mcr	p15, 0, r5, c1, c0, 1
	isb
#endif

	mrc	p15, 0, r5, c1, c1, 0		@ read SCR
	bic	r5, r5, #0x4a			@ clear IRQ, EA, nET bits
	orr	r5, r5, #0x31			@ enable NS, AW, FW bits
						@ FIQ preserved for secure mode
	mov	r6, #SVC_MODE			@ default mode is SVC
	is_cpu_virt_capable r4
#ifdef CONFIG_ARMV7_VIRT
	orreq	r5, r5, #0x100			@ allow HVC instruction
	moveq	r6, #HYP_MODE			@ Enter the kernel as HYP
	mrseq	r3, sp_svc
	msreq	sp_hyp, r3			@ migrate SP
#endif

	mcr	p15, 0, r5, c1, c1, 0		@ write SCR (with NS bit set)
	isb

	bne	1f

	@ Reset CNTVOFF to 0 before leaving monitor mode
	mrc	p15, 0, r4, c0, c1, 1		@ read ID_PFR1
	ands	r4, r4, #CPUID_ARM_GENTIMER_MASK	@ test arch timer bits
	movne	r4, #0
	mcrrne	p15, 4, r4, r4, c14		@ Reset CNTVOFF to zero
1:
	mov	lr, ip
	mov	ip, #(F_BIT | I_BIT | A_BIT)	@ Set A, I and F
	tst	lr, #1				@ Check for Thumb PC
	orrne	ip, ip, #T_BIT			@ Set T if Thumb
	orr	ip, ip, r6			@ Slot target mode in
	msr	spsr_cxfs, ip			@ Set full SPSR
	movs	pc, lr				@ ERET to non-secure

ENTRY(_do_nonsec_entry)
	mov	ip, r0
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	smc	#0
ENDPROC(_do_nonsec_entry)

.macro get_cbar_addr	addr
#ifdef CONFIG_ARM_GIC_BASE_ADDRESS
	ldr	\addr, =CONFIG_ARM_GIC_BASE_ADDRESS
#else
	mrc	p15, 4, \addr, c15, c0, 0	@ read CBAR
	bfc	\addr, #0, #15			@ clear reserved bits
#endif
.endm

.macro get_gicd_addr	addr
	get_cbar_addr	\addr
	add	\addr, \addr, #GIC_DIST_OFFSET	@ GIC dist i/f offset
.endm

.macro get_gicc_addr	addr, tmp
	get_cbar_addr	\addr
	is_cpu_virt_capable \tmp
	movne	\tmp, #GIC_CPU_OFFSET_A9	@ GIC CPU offset for A9
	moveq	\tmp, #GIC_CPU_OFFSET_A15	@ GIC CPU offset for A15/A7
	add	\addr, \addr, \tmp
.endm

#ifndef CONFIG_ARMV7_PSCI
/*
 * Secondary CPUs start here and call the code for the core specific parts
 * of the non-secure and HYP mode transition. The GIC distributor specific
 * code has already been executed by a C function before.
 * Then they go back to wfi and wait to be woken up by the kernel again.
 */
ENTRY(_smp_pen)
	cpsid	i
	cpsid	f

	bl	_nonsec_init

	adr	r0, _smp_pen			@ do not use this address again
	b	smp_waitloop			@ wait for IPIs, board specific
ENDPROC(_smp_pen)
#endif

/*
 * Switch a core to non-secure state.
 *
 *  1. initialize the GIC per-core interface
 *  2. allow coprocessor access in non-secure modes
 *
 * Called from smp_pen by secondary cores and directly by the BSP.
 * Do not assume that the stack is available and only use registers
 * r0-r3 and r12.
 *
 * PERIPHBASE is used to get the GIC address. This could be 40 bits long,
 * though, but we check this in C before calling this function.
 */
ENTRY(_nonsec_init)
	get_gicd_addr	r3

	mvn	r1, #0				@ all bits to 1
	str	r1, [r3, #GICD_IGROUPRn]	@ allow private interrupts

	get_gicc_addr	r3, r1

	mov	r1, #3				@ Enable both groups
	str	r1, [r3, #GICC_CTLR]		@ and clear all other bits
	mov	r1, #0xff
	str	r1, [r3, #GICC_PMR]		@ set priority mask register

	mrc	p15, 0, r0, c1, c1, 2
	movw	r1, #0x3fff
	movt	r1, #0x0004
	orr	r0, r0, r1
	mcr	p15, 0, r0, c1, c1, 2		@ NSACR = all copros to non-sec

/* The CNTFRQ register of the generic timer needs to be
 * programmed in secure state. Some primary bootloaders / firmware
 * omit this, so if the frequency is provided in the configuration,
 * we do this here instead.
 * But first check if we have the generic timer.
 */
#ifdef COUNTER_FREQUENCY
	mrc	p15, 0, r0, c0, c1, 1		@ read ID_PFR1
	and	r0, r0, #CPUID_ARM_GENTIMER_MASK	@ mask arch timer bits
	cmp	r0, #(1 << CPUID_ARM_GENTIMER_SHIFT)
	ldreq	r1, =COUNTER_FREQUENCY
	mcreq	p15, 0, r1, c14, c0, 0		@ write CNTFRQ
#endif

	adr	r1, _monitor_vectors
	mcr	p15, 0, r1, c12, c0, 1		@ set MVBAR to secure vectors
	isb

	mov	r0, r3				@ return GICC address
	bx	lr
ENDPROC(_nonsec_init)

#ifdef CONFIG_SMP_PEN_ADDR
/* void __weak smp_waitloop(unsigned previous_address); */
ENTRY(smp_waitloop)
	wfi
	ldr	r1, =CONFIG_SMP_PEN_ADDR	@ load start address
	ldr	r1, [r1]
#ifdef CONFIG_PEN_ADDR_BIG_ENDIAN
	rev	r1, r1
#endif
	cmp	r0, r1			@ make sure we dont execute this code
	beq	smp_waitloop		@ again (due to a spurious wakeup)
	mov	r0, r1
	b	_do_nonsec_entry
ENDPROC(smp_waitloop)
.weak smp_waitloop
#endif

	.popsection
