/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 * Akshay Saraswat <akshay.s@samsung.com>
 */

#include <config.h>
#include <asm/arch/cpu.h>

	.globl relocate_wait_code
relocate_wait_code:
	adr     r0, code_base		@ r0: source address (start)
	adr     r1, code_end		@ r1: source address (end)
	ldr     r2, =0x02073000		@ r2: target address
1:
	ldmia   r0!, {r3-r6}
	stmia   r2!, {r3-r6}
	cmp     r0, r1
	blt     1b
	b	code_end
	.ltorg
/*
 * Secondary core waits here until Primary wake it up.
 * Below code is copied to CONFIG_EXYNOS_RELOCATE_CODE_BASE.
 * This is a workaround code which is supposed to act as a
 * substitute/supplement to the iROM code.
 *
 * This workaround code is relocated to the address 0x02073000
 * because that comes out to be the last 4KB of the iRAM
 * (Base Address - 0x02020000, Limit Address - 0x020740000).
 *
 * U-Boot and kernel are aware of this code and flags by the simple
 * fact that we are implementing a workaround in the last 4KB
 * of the iRAM and we have already defined these flag and address
 * values in both kernel and U-Boot for our use.
 */
code_base:
	b	 1f
/*
 * These addresses are being used as flags in u-boot and kernel.
 *
 * Jump address for resume and flag to check for resume/reset:
 * Resume address - 0x2073008
 * Resume flag - 0x207300C
 *
 * Jump address for cluster switching:
 * Switch address - 0x2073018
 *
 * Jump address for core hotplug:
 * Hotplug address - 0x207301C
 *
 * Jump address for C2 state (Reserved for future not being used right now):
 * C2 address - 0x2073024
 *
 * Managed per core status for the active cluster:
 * CPU0 state - 0x2073028
 * CPU1 state - 0x207302C
 * CPU2 state - 0x2073030
 * CPU3 state - 0x2073034
 *
 * Managed per core GIC status for the active cluster:
 * CPU0 gic state - 0x2073038
 * CPU1 gic state - 0x207303C
 * CPU2 gic state - 0x2073040
 * CPU3 gic state - 0x2073044
 *
 * Logic of the code:
 * Step-1: Read current CPU status.
 * Step-2: If it's a resume then continue, else jump to step 4.
 * Step-3: Clear inform1 PMU register and jump to inform0 value.
 * Step-4: If it's a switch, C2 or reset, get the hotplug address.
 * Step-5: If address is not available, enter WFE.
 * Step-6: If address is available, jump to that address.
 */
	nop			     @ for backward compatibility
	.word   0x0		     @ REG0: RESUME_ADDR
	.word   0x0		     @ REG1: RESUME_FLAG
	.word   0x0		     @ REG2
	.word   0x0		     @ REG3
_switch_addr:
	.word   0x0		     @ REG4: SWITCH_ADDR
_hotplug_addr:
	.word   0x0		     @ REG5: CPU1_BOOT_REG
	.word   0x0		     @ REG6
_c2_addr:
	.word   0x0		     @ REG7: REG_C2_ADDR
_cpu_state:
	.word   0x1		     @ CPU0_STATE : RESET
	.word   0x2		     @ CPU1_STATE : SECONDARY RESET
	.word   0x2		     @ CPU2_STATE : SECONDARY RESET
	.word   0x2		     @ CPU3_STATE : SECONDARY RESET
_gic_state:
	.word   0x0		     @ CPU0 - GICD_IGROUPR0
	.word   0x0		     @ CPU1 - GICD_IGROUPR0
	.word   0x0		     @ CPU2 - GICD_IGROUPR0
	.word   0x0		     @ CPU3 - GICD_IGROUPR0
1:
	adr     r0, _cpu_state
	mrc     p15, 0, r7, c0, c0, 5   @ read MPIDR
	and     r7, r7, #0xf	    @ r7 = cpu id
/* Read the current cpu state */
	ldr     r10, [r0, r7, lsl #2]
svc_entry:
	tst     r10, #(1 << 4)
	adrne   r0, _switch_addr
	bne     wait_for_addr
/* Clear INFORM1 */
	ldr     r0, =(0x10040000 + 0x804)
	ldr     r1, [r0]
	cmp     r1, #0x0
	movne   r1, #0x0
	strne   r1, [r0]
/* Get INFORM0 */
	ldrne   r1, =(0x10040000 + 0x800)
	ldrne   pc, [r1]
	tst     r10, #(1 << 0)
	ldrne   pc, =0x23e00000
	adr     r0, _hotplug_addr
wait_for_addr:
	ldr     r1, [r0]
	cmp     r1, #0x0
	bxne    r1
	wfe
	b	 wait_for_addr
	.ltorg
code_end:
	mov	pc, lr
