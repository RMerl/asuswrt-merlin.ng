/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <config.h>
#include <asm-offsets.h>
#include <asm/cacheops.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/asm.h>
#include <system-constants.h>
#include "mt7628.h"

#define CACHE_STACK_SIZE	0x4000
#define CACHE_STACK_BASE	(SYS_INIT_SP_ADDR - CACHE_STACK_SIZE)

#define DELAY_USEC(us)		((58 * (us)) / 3)

	.set noreorder

LEAF(mips_sram_init)
#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	/* Setup CPU PLL */
	li	t0, DELAY_USEC(1000000)
	li	t1, KSEG1ADDR(SYSCTL_BASE + SYSCTL_ROM_STATUS_REG)
	li	t2, KSEG1ADDR(SYSCTL_BASE + SYSCTL_CLKCFG0_REG)

_check_rom_status:
	lw	t3, 0(t1)
	andi	t3, t3, 1
	bnez	t3, _rom_normal
	subu	t0, t0, 1
	bnez	t0, _check_rom_status
	 nop

	lw	t3, 0(t2)
	ori	t3, (CPU_PLL_FROM_BBP | CPU_PLL_FROM_XTAL)
	xori	t3, CPU_PLL_FROM_BBP
	b	_cpu_pll_done
	 nop

_rom_normal:
	lw	t3, 0(t2)
	ori	t3, (CPU_PLL_FROM_BBP | CPU_PLL_FROM_XTAL | \
		    DIS_BBP_SLEEP | EN_BBP_CLK)
	xori	t3, (CPU_PLL_FROM_BBP | CPU_PLL_FROM_XTAL)

_cpu_pll_done:
	sw	t3, 0(t2)

	li	t2, KSEG1ADDR(RBUSCTL_BASE + RBUSCTL_DYN_CFG0_REG)
	lw	t3, 0(t2)
	ori	t3, t3, (CPU_FDIV_M | CPU_FFRAC_M)
	xori	t3, t3, (CPU_FDIV_M | CPU_FFRAC_M)
	ori	t3, t3, ((1 << CPU_FDIV_S) | (1 << CPU_FFRAC_S))
	sw	t3, 0(t2)

	/* Clear WST & SPR bits in ErrCtl */
	mfc0	t0, CP0_ECC
	ins	t0, zero, 30, 2
	mtc0	t0, CP0_ECC
	ehb

	/* Simply initialize I-Cache */
	li	a0, 0
	li	a1, CONFIG_SYS_ICACHE_SIZE

	mtc0	zero, CP0_TAGLO		/* Zero to DDataLo */

1:	cache	INDEX_STORE_TAG_I, 0(a0)
	addiu	a0, CONFIG_SYS_ICACHE_LINE_SIZE
	bne	a0, a1, 1b
	 nop

	/* Simply initialize D-Cache */
	li	a0, 0
	li	a1, CONFIG_SYS_DCACHE_SIZE

	mtc0	zero, CP0_TAGLO, 2

2:	cache	INDEX_STORE_TAG_D, 0(a0)
	addiu	a0, CONFIG_SYS_DCACHE_LINE_SIZE
	bne	a0, a1, 2b
	 nop

	/* Set KSEG0 Cachable */
	mfc0	t0, CP0_CONFIG
	and	t0, t0, MIPS_CONF_IMPL
	or	t0, t0, CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0, CP0_CONFIG
	ehb

	/* Lock D-Cache */
	PTR_LI	a0, CACHE_STACK_BASE		/* D-Cache lock base */
	li	a1, CACHE_STACK_SIZE		/* D-Cache lock size */
	li	a2, 0x1ffff800			/* Mask of DTagLo[PTagLo] */

3:
	/* Lock one cacheline */
	and	t0, a0, a2
	ori	t0, 0xe0			/* Valid & Dirty & Lock bits */
	mtc0	t0, CP0_TAGLO, 2		/* Write to DTagLo */
	ehb
	cache	INDEX_STORE_TAG_D, 0(a0)

	addiu	a0, CONFIG_SYS_DCACHE_LINE_SIZE
	sub	a1, CONFIG_SYS_DCACHE_LINE_SIZE
	bnez	a1, 3b
	 nop
#endif /* CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT) */

	jr	ra
	 nop
	END(mips_sram_init)

NESTED(lowlevel_init, 0, ra)
	/* Save ra and do real lowlevel initialization */
	move	s0, ra

	PTR_LA	t9, mt7628_init
	jalr	t9
	 nop

	move	ra, s0

#if CONFIG_IS_ENABLED(INIT_STACK_WITHOUT_MALLOC_F)
	/* Set malloc base */
	li	t0, (SYS_INIT_SP_ADDR + 15) & (~15)
	PTR_S	t0, GD_MALLOC_BASE(k0)	# gd->malloc_base offset
#endif

	/* Write back data in locked cache to DRAM */
	PTR_LI	a0, CACHE_STACK_BASE		/* D-Cache unlock base */
	li	a1, CACHE_STACK_SIZE		/* D-Cache unlock size */

1:
	cache	HIT_WRITEBACK_INV_D, 0(a0)
	addiu	a0, CONFIG_SYS_DCACHE_LINE_SIZE
	sub	a1, CONFIG_SYS_DCACHE_LINE_SIZE
	bnez	a1, 1b
	 nop

	/* Set KSEG0 Uncached */
	mfc0	t0, CP0_CONFIG
	and	t0, t0, MIPS_CONF_IMPL
	or	t0, t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
	ehb

	jr	ra
	 nop
	END(lowlevel_init)
