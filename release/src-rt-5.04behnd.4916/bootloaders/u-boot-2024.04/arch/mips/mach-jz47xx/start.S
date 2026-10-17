// SPDX-License-Identifier: GPL-2.0+
/*
 *  Startup Code for MIPS32 XBURST CPU-core
 *
 *  Copyright (c) 2010 Xiangfu Liu <xiangfu@sharism.cc>
 */

#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>
#include <asm/cache.h>
#include <mach/jz4780.h>

	.set noreorder

	.globl _start
	.text
_start:
#ifdef CONFIG_SPL_BUILD

	/* magic value ("MSPL") */
	.word 0x4d53504c

	/* Invalidate BTB */
	mfc0	t0, CP0_CONFIG, 7
	nop
	ori	t0, 2
	mtc0	t0, CP0_CONFIG, 7
	nop

	/*
	 * CU0=UM=EXL=IE=0, BEV=ERL=1, IP2~7=1
	 */
	li	t0, 0x0040FC04
	mtc0	t0, CP0_STATUS

	/* CAUSE register */
	/* IV=1, use the specical interrupt vector (0x200) */
	li	t1, 0x00800000
	mtc0	t1, CP0_CAUSE

#ifdef CONFIG_SOC_JZ4780
	/* enable bridge radical mode */
	la	t0, CPM_BASE
	lw	t1, 0x24(t0)
	ori	t1, t1, 0x22
	sw	t1, 0x24(t0)
#endif

	/* Set up stack */
	li	sp, CONFIG_SPL_STACK

	b		board_init_f
	 nop

#ifdef CONFIG_SOC_JZ4780

	.globl enable_caches
	.ent enable_caches
enable_caches:
	mtc0	zero, CP0_TAGLO
	mtc0	zero, CP0_TAGHI

	li	t0, KSEG0
	addu	t1, t0, CONFIG_SYS_DCACHE_SIZE
1:
	cache	INDEX_STORE_TAG_D, 0(t0)
	bne	t0, t1, 1b
	addiu	t0, t0, CONFIG_SYS_CACHELINE_SIZE

	li	t0, KSEG0
	addu	t1, t0, CONFIG_SYS_ICACHE_SIZE
2:
	cache	INDEX_STORE_TAG_I, 0(t0)
	bne	t0, t1, 2b
	addiu	t0, t0, CONFIG_SYS_CACHELINE_SIZE

	/* Invalidate BTB */
	mfc0	t0, CP0_CONFIG, 7
	nop
	ori	t0, 2
	mtc0	t0, CP0_CONFIG, 7
	nop

	/* Enable caches */
	li	t0, CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0, CP0_CONFIG
	nop

	jr	ra
	 nop

	.end enable_caches

#endif /* CONFIG_SOC_JZ4780 */
#endif /* !CONFIG_SPL_BUILD */
