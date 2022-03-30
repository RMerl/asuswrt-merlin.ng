/*
 * Copyright (C) 2013,2014 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/psci.h>

	.pushsection ._secure.text, "ax"

	.arch_extension	sec

	.align	5
	.globl _psci_vectors
_psci_vectors:
	b	default_psci_vector	@ reset
	b	default_psci_vector	@ undef
	b	_smc_psci		@ smc
	b	default_psci_vector	@ pabort
	b	default_psci_vector	@ dabort
	b	default_psci_vector	@ hyp
	b	default_psci_vector	@ irq
	b	psci_fiq_enter		@ fiq

ENTRY(psci_fiq_enter)
	movs	pc, lr
ENDPROC(psci_fiq_enter)
.weak psci_fiq_enter

ENTRY(default_psci_vector)
	movs	pc, lr
ENDPROC(default_psci_vector)
.weak default_psci_vector

ENTRY(psci_version)
ENTRY(psci_cpu_suspend)
ENTRY(psci_cpu_off)
ENTRY(psci_cpu_on)
ENTRY(psci_affinity_info)
ENTRY(psci_migrate)
ENTRY(psci_migrate_info_type)
ENTRY(psci_migrate_info_up_cpu)
ENTRY(psci_system_off)
ENTRY(psci_system_reset)
ENTRY(psci_features)
ENTRY(psci_cpu_freeze)
ENTRY(psci_cpu_default_suspend)
ENTRY(psci_node_hw_state)
ENTRY(psci_system_suspend)
ENTRY(psci_set_suspend_mode)
ENTRY(psi_stat_residency)
ENTRY(psci_stat_count)
	mov	r0, #ARM_PSCI_RET_NI	@ Return -1 (Not Implemented)
	mov	pc, lr
ENDPROC(psci_stat_count)
ENDPROC(psi_stat_residency)
ENDPROC(psci_set_suspend_mode)
ENDPROC(psci_system_suspend)
ENDPROC(psci_node_hw_state)
ENDPROC(psci_cpu_default_suspend)
ENDPROC(psci_cpu_freeze)
ENDPROC(psci_features)
ENDPROC(psci_system_reset)
ENDPROC(psci_system_off)
ENDPROC(psci_migrate_info_up_cpu)
ENDPROC(psci_migrate_info_type)
ENDPROC(psci_migrate)
ENDPROC(psci_affinity_info)
ENDPROC(psci_cpu_on)
ENDPROC(psci_cpu_off)
ENDPROC(psci_cpu_suspend)
ENDPROC(psci_version)
.weak psci_version
.weak psci_cpu_suspend
.weak psci_cpu_off
.weak psci_cpu_on
.weak psci_affinity_info
.weak psci_migrate
.weak psci_migrate_info_type
.weak psci_migrate_info_up_cpu
.weak psci_system_off
.weak psci_system_reset
.weak psci_features
.weak psci_cpu_freeze
.weak psci_cpu_default_suspend
.weak psci_node_hw_state
.weak psci_system_suspend
.weak psci_set_suspend_mode
.weak psi_stat_residency
.weak psci_stat_count

_psci_table:
	.word	ARM_PSCI_FN_CPU_SUSPEND
	.word	psci_cpu_suspend
	.word	ARM_PSCI_FN_CPU_OFF
	.word	psci_cpu_off
	.word	ARM_PSCI_FN_CPU_ON
	.word	psci_cpu_on
	.word	ARM_PSCI_FN_MIGRATE
	.word	psci_migrate
	.word	ARM_PSCI_0_2_FN_PSCI_VERSION
	.word	psci_version
	.word	ARM_PSCI_0_2_FN_CPU_SUSPEND
	.word	psci_cpu_suspend
	.word	ARM_PSCI_0_2_FN_CPU_OFF
	.word	psci_cpu_off
	.word	ARM_PSCI_0_2_FN_CPU_ON
	.word	psci_cpu_on
	.word	ARM_PSCI_0_2_FN_AFFINITY_INFO
	.word	psci_affinity_info
	.word	ARM_PSCI_0_2_FN_MIGRATE
	.word	psci_migrate
	.word	ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE
	.word	psci_migrate_info_type
	.word	ARM_PSCI_0_2_FN_MIGRATE_INFO_UP_CPU
	.word	psci_migrate_info_up_cpu
	.word	ARM_PSCI_0_2_FN_SYSTEM_OFF
	.word	psci_system_off
	.word	ARM_PSCI_0_2_FN_SYSTEM_RESET
	.word	psci_system_reset
	.word	ARM_PSCI_1_0_FN_PSCI_FEATURES
	.word	psci_features
	.word	ARM_PSCI_1_0_FN_CPU_FREEZE
	.word	psci_cpu_freeze
	.word	ARM_PSCI_1_0_FN_CPU_DEFAULT_SUSPEND
	.word	psci_cpu_default_suspend
	.word	ARM_PSCI_1_0_FN_NODE_HW_STATE
	.word	psci_node_hw_state
	.word	ARM_PSCI_1_0_FN_SYSTEM_SUSPEND
	.word	psci_system_suspend
	.word	ARM_PSCI_1_0_FN_SET_SUSPEND_MODE
	.word	psci_set_suspend_mode
	.word	ARM_PSCI_1_0_FN_STAT_RESIDENCY
	.word	psi_stat_residency
	.word	ARM_PSCI_1_0_FN_STAT_COUNT
	.word	psci_stat_count
	.word	0
	.word	0

_smc_psci:
	push	{r4-r7,lr}

	@ Switch to secure
	mrc	p15, 0, r7, c1, c1, 0
	bic	r4, r7, #1
	mcr	p15, 0, r4, c1, c1, 0
	isb

	adr	r4, _psci_table
1:	ldr	r5, [r4]		@ Load PSCI function ID
	ldr	r6, [r4, #4]		@ Load target PC
	cmp	r5, #0			@ If reach the end, bail out
	moveq	r0, #ARM_PSCI_RET_INVAL	@ Return -2 (Invalid)
	beq	2f
	cmp	r0, r5			@ If not matching, try next entry
	addne	r4, r4, #8
	bne	1b

	blx	r6			@ Execute PSCI function

	@ Switch back to non-secure
2:	mcr	p15, 0, r7, c1, c1, 0

	pop	{r4-r7, lr}
	movs	pc, lr			@ Return to the kernel

@ Requires dense and single-cluster CPU ID space
ENTRY(psci_get_cpu_id)
	mrc	p15, 0, r0, c0, c0, 5	/* read MPIDR */
	and	r0, r0, #0xff		/* return CPU ID in cluster */
	bx	lr
ENDPROC(psci_get_cpu_id)
.weak psci_get_cpu_id

/* Imported from Linux kernel */
ENTRY(psci_v7_flush_dcache_all)
	stmfd	sp!, {r4-r5, r7, r9-r11, lr}
	dmb					@ ensure ordering with previous memory accesses
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	ands	r3, r0, #0x7000000		@ extract loc from clidr
	mov	r3, r3, lsr #23			@ left align loc bit field
	beq	finished			@ if loc is 0, then no need to clean
	mov	r10, #0				@ start clean at cache level 0
flush_levels:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	skip				@ skip if no cache, or just i-cache
	mrs     r9, cpsr			@ make cssr&csidr read atomic
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb					@ isb to sych the new cssr&csidr
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	msr     cpsr_c, r9
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
loop1:
	mov	r9, r7				@ create working copy of max index
loop2:
	orr	r11, r10, r4, lsl r5		@ factor way and cache number into r11
	orr	r11, r11, r9, lsl r2		@ factor index number into r11
	mcr	p15, 0, r11, c7, c14, 2		@ clean & invalidate by set/way
	subs	r9, r9, #1			@ decrement the index
	bge	loop2
	subs	r4, r4, #1			@ decrement the way
	bge	loop1
skip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	flush_levels
finished:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	dsb	st
	isb
	ldmfd	sp!, {r4-r5, r7, r9-r11, lr}
	bx	lr
ENDPROC(psci_v7_flush_dcache_all)

ENTRY(psci_disable_smp)
	mrc	p15, 0, r0, c1, c0, 1		@ ACTLR
	bic	r0, r0, #(1 << 6)		@ Clear SMP bit
	mcr	p15, 0, r0, c1, c0, 1		@ ACTLR
	isb
	dsb
	bx	lr
ENDPROC(psci_disable_smp)
.weak psci_disable_smp

ENTRY(psci_enable_smp)
	mrc	p15, 0, r0, c1, c0, 1		@ ACTLR
	orr	r0, r0, #(1 << 6)		@ Set SMP bit
	mcr	p15, 0, r0, c1, c0, 1		@ ACTLR
	isb
	bx	lr
ENDPROC(psci_enable_smp)
.weak psci_enable_smp

ENTRY(psci_cpu_off_common)
	push	{lr}

	bl	psci_v7_flush_dcache_all

	clrex					@ Why???

	mrc	p15, 0, r0, c1, c0, 0		@ SCTLR
	bic	r0, r0, #(1 << 2)		@ Clear C bit
	mcr	p15, 0, r0, c1, c0, 0		@ SCTLR
	isb
	dsb

	bl	psci_v7_flush_dcache_all

	clrex					@ Why???

	bl	psci_disable_smp

	pop	{lr}
	bx	lr
ENDPROC(psci_cpu_off_common)

@ The stacks are allocated in reverse order, i.e.
@ the stack for CPU0 has the highest memory address.
@
@ --------------------  __secure_stack_end
@ |  CPU0 target PC  |
@ |------------------|
@ |                  |
@ |    CPU0 stack    |
@ |                  |
@ |------------------|  __secure_stack_end - 1KB
@ |        .         |
@ |        .         |
@ |        .         |
@ |        .         |
@ --------------------  __secure_stack_start
@
@ This expects CPU ID in r0 and returns stack top in r0
LENTRY(psci_get_cpu_stack_top)
	@ stack top = __secure_stack_end - (cpuid << ARM_PSCI_STACK_SHIFT)
	ldr	r3, =__secure_stack_end
	sub	r0, r3, r0, LSL #ARM_PSCI_STACK_SHIFT
	sub	r0, r0, #4		@ Save space for target PC
	bx	lr
ENDPROC(psci_get_cpu_stack_top)

@ {r0, r1, r2, ip} from _do_nonsec_entry(kernel_entry, 0, machid, r2) in
@ arch/arm/lib/bootm.c:boot_jump_linux() must remain unchanged across
@ this function.
ENTRY(psci_stack_setup)
	mov	r6, lr
	mov	r7, r0
	bl	psci_get_cpu_id		@ CPU ID => r0
	bl	psci_get_cpu_stack_top	@ stack top => r0
	mov	sp, r0
	mov	r0, r7
	bx	r6
ENDPROC(psci_stack_setup)

ENTRY(psci_arch_init)
	mov	pc, lr
ENDPROC(psci_arch_init)
.weak psci_arch_init

ENTRY(psci_arch_cpu_entry)
	mov	pc, lr
ENDPROC(psci_arch_cpu_entry)
.weak psci_arch_cpu_entry

ENTRY(psci_cpu_entry)
	bl	psci_enable_smp

	bl	_nonsec_init

	bl	psci_stack_setup

	bl	psci_arch_cpu_entry

	bl	psci_get_cpu_id			@ CPU ID => r0
	mov	r2, r0				@ CPU ID => r2
	bl	psci_get_context_id		@ context id => r0
	mov	r1, r0				@ context id => r1
	mov	r0, r2				@ CPU ID => r0
	bl	psci_get_target_pc		@ target PC => r0
	b	_do_nonsec_entry
ENDPROC(psci_cpu_entry)

	.popsection
