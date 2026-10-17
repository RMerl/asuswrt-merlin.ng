/*
 * Utility functions for FEL mode, when running SPL in AArch64.
 *
 * Copyright (c) 2017 Arm Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/system.h>
#include <linux/linkage.h>
#include <asm/arch/cpu.h>

/*
 * We don't overwrite save_boot_params() here, to save the FEL state upon
 * entry, since this would run *after* the RMR reset, which clobbers that
 * state.
 * Instead we store the state _very_ early in the boot0 hook, *before*
 * resetting to AArch64.
 */

/*
 * The FEL routines in BROM run in AArch32.
 * Reset back into 32-bit mode here and restore the saved FEL state
 * afterwards.
 * Resetting back into AArch32/EL3 using the RMR always enters the BROM,
 * but we can use the CPU hotplug mechanism to branch back to our code
 * immediately.
 */
ENTRY(return_to_fel)
	/*
	 * the RMR reset will clear all registers, so save the arguments
	 * (LR and SP) in the fel_stash structure, which we read anyways later
	 */
	adr	x2, fel_stash
	str	w0, [x2]
	str	w1, [x2, #4]

	adr	x1, fel_stash_addr	// to find the fel_stash address in AA32
	str	w2, [x1]

	ldr	w0, =0xfa50392f		// CPU hotplug magic
#ifdef CONFIG_MACH_SUN50I_H616
	ldr	w2, =(SUNXI_R_CPUCFG_BASE + 0x1c0)
	str	w0, [x2], #0x4
#elif CONFIG_MACH_SUN50I_H6
	ldr	w2, =(SUNXI_RTC_BASE + 0x1b8)	// BOOT_CPU_HP_FLAG_REG
	str	w0, [x2], #0x4
#else
	ldr	w2, =(SUNXI_CPUCFG_BASE + 0x1a4) // offset for CPU hotplug base
	str	w0, [x2, #0x8]
#endif
	adr	x0, back_in_32
	str	w0, [x2]

	dsb	sy
	isb	sy
	mov	x0, #2			// RMR reset into AArch32
	dsb	sy
	msr	RMR_EL3, x0
	isb	sy
1:	wfi
	b	1b

/* AArch32 code to restore the state from fel_stash and return back to FEL. */
back_in_32:
	.word	0xe59f0028	// ldr	r0, [pc, #40]	; load fel_stash address
	.word	0xe5901008	// ldr	r1, [r0, #8]
	.word	0xe129f001	// msr	CPSR_fc, r1
	.word	0xf57ff06f	// isb
	.word	0xe590d000	// ldr	sp, [r0]
	.word	0xe590e004	// ldr	lr, [r0, #4]
	.word	0xe5901010	// ldr	r1, [r0, #16]
	.word	0xee0c1f10	// mcr	15, 0, r1, cr12, cr0, {0} ; VBAR
	.word	0xe590100c	// ldr	r1, [r0, #12]
	.word	0xee011f10	// mcr	15, 0, r1, cr1, cr0, {0}  ; SCTLR
	.word	0xf57ff06f	// isb
	.word	0xe12fff1e	// bx	lr		; return to FEL
fel_stash_addr:
	.word   0x00000000	// receives fel_stash addr, by AA64 code above
ENDPROC(return_to_fel)
