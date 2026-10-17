/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Allwinner A64 (sun50i) CPU
 */

#include <asm/arch/cpu.h>

#if defined(CONFIG_RESERVE_ALLWINNER_BOOT0_HEADER) && !defined(CONFIG_SPL_BUILD)
/* reserve space for BOOT0 header information */
	b	reset
	.space	1532
#elif defined(CONFIG_ARM_BOOT_HOOK_RMR)
/*
 * Switch into AArch64 if needed.
 * Refer to arch/arm/mach-sunxi/rmr_switch.S for the original source.
 */
	tst     x0, x0                  // this is "b #0x84" in ARM
	b       reset
	.space  0x7c

	.word	0xe28f0070	// add     r0, pc, #112	 // @(fel_stash - .)
	.word	0xe59f106c	// ldr     r1, [pc, #108] // fel_stash - .
	.word	0xe0800001	// add     r0, r0, r1
	.word	0xe580d000	// str     sp, [r0]
	.word	0xe580e004	// str     lr, [r0, #4]
	.word	0xe10fe000	// mrs     lr, CPSR
	.word	0xe580e008	// str     lr, [r0, #8]
	.word	0xee11ef10	// mrc     15, 0, lr, cr1, cr0, {0}
	.word	0xe580e00c	// str     lr, [r0, #12]
	.word	0xee1cef10	// mrc     15, 0, lr, cr12, cr0, {0}
	.word	0xe580e010	// str     lr, [r0, #16]

	.word	0xe59f1034	// ldr     r1, [pc, #52] ; RVBAR_ADDRESS
	.word	0xe59f0034	// ldr     r0, [pc, #52] ; SUNXI_SRAMC_BASE
	.word	0xe5900024	// ldr     r0, [r0, #36] ; SRAM_VER_REG
	.word	0xe21000ff	// ands    r0, r0, #255    ; 0xff
	.word	0x159f102c	// ldrne   r1, [pc, #44] ; RVBAR_ALTERNATIVE
	.word	0xe59f002c	// ldr     r0, [pc, #44] ; CONFIG_*TEXT_BASE
	.word	0xe5810000	// str     r0, [r1]
	.word	0xf57ff04f	// dsb     sy
	.word	0xf57ff06f	// isb     sy
	.word	0xee1c0f50	// mrc     15, 0, r0, cr12, cr0, {2} ; RMR
	.word	0xe3800003	// orr     r0, r0, #3
	.word	0xee0c0f50	// mcr     15, 0, r0, cr12, cr0, {2} ; RMR
	.word	0xf57ff06f	// isb     sy
	.word	0xe320f003	// wfi
	.word	0xeafffffd	// b       @wfi

	.word	CONFIG_SUNXI_RVBAR_ADDRESS	// writable RVBAR mapping addr
	.word	SUNXI_SRAMC_BASE
	.word	CONFIG_SUNXI_RVBAR_ALTERNATIVE	// address for die variant
#ifdef CONFIG_SPL_BUILD
	.word	CONFIG_SPL_TEXT_BASE
#else
	.word   CONFIG_TEXT_BASE
#endif
	.word	fel_stash - .
#else
/* normal execution */
	b	reset
#endif
