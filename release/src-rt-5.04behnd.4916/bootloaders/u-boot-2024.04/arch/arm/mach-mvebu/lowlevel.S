/* SPDX-License-Identifier: GPL-2.0+ */

#include <config.h>
#include <linux/linkage.h>
#include <asm/system.h>
#include <asm/pl310.h>

ENTRY(arch_very_early_init)
#ifdef CONFIG_ARMADA_38X
	/*
	 * Only with disabled MMU its possible to switch the base
	 * register address on Armada 38x. Without this the SDRAM
	 * located at >= 0x4000.0000 is also not accessible, as its
	 * still locked to cache.
	 *
	 * So to fully release / unlock this area from cache, we need
	 * to first flush all caches, then disable the MMU and
	 * disable the L2 cache.
	 */

	/* Invalidate L1 I/D */
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array
	mcr	p15, 0, r0, c7, c10, 4	@ DSB
	mcr	p15, 0, r0, c7, c5, 4	@ ISB

	/* Disable MMU */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #CR_M
	mcr	p15, 0, r0, c1, c0, 0

	/*
	 * Disable L2 cache
	 *
	 * NOTE: Internal registers are still at address INTREG_BASE_ADDR_REG
	 *       but CFG_SYS_PL310_BASE is already calculated from base
	 *       address SOC_REGS_PHY_BASE.
	 */
	ldr	r1, =(CFG_SYS_PL310_BASE - SOC_REGS_PHY_BASE + INTREG_BASE_ADDR_REG)
	ldr	r0, [r1, #L2X0_CTRL_OFF]
	bic	r0, #L2X0_CTRL_EN
	str	r0, [r1, #L2X0_CTRL_OFF]
#endif

	/* Move internal registers from INTREG_BASE_ADDR_REG to SOC_REGS_PHY_BASE */
	ldr	r0, =SOC_REGS_PHY_BASE
	ldr	r1, =INTREG_BASE_ADDR_REG
	str	r0, [r1]
	add	r0, r0, #0xC000
	mcr	p15, 4, r0, c15, c0

	bx lr
ENDPROC(arch_very_early_init)
