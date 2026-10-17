/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>
#include <asm/addrspace.h>
#include <asm/mipsmtregs.h>
#include <asm/cm.h>
#include "../mt7621.h"
#include "dram.h"

#ifndef CFG_SYS_INIT_SP_ADDR
#define CFG_SYS_INIT_SP_ADDR	(CFG_SYS_SDRAM_BASE + \
				CFG_SYS_INIT_SP_OFFSET)
#endif

#define SP_ADDR_TEMP		0xbe10dff0

	.macro init_wr sel
	MTC0	zero, CP0_WATCHLO,\sel
	mtc0	t1, CP0_WATCHHI,\sel
	.endm

	.macro setup_stack_gd
	li	t0, -16
	PTR_LI	t1, CFG_SYS_INIT_SP_ADDR
	and	sp, t1, t0		# force 16 byte alignment
	PTR_SUBU \
		sp, sp, GD_SIZE		# reserve space for gd
	and	sp, sp, t0		# force 16 byte alignment
	move	k0, sp			# save gd pointer
#if CONFIG_IS_ENABLED(SYS_MALLOC_F) && \
    !CONFIG_IS_ENABLED(INIT_STACK_WITHOUT_MALLOC_F)
	li	t2, CONFIG_VAL(SYS_MALLOC_F_LEN)
	PTR_SUBU \
		sp, sp, t2		# reserve space for early malloc
	and	sp, sp, t0		# force 16 byte alignment
#endif
	move	fp, sp

	/* Clear gd */
	move	t0, k0
1:
	PTR_S	zero, 0(t0)
	PTR_ADDIU t0, PTRSIZE
	blt	t0, t1, 1b
	 nop

#if CONFIG_IS_ENABLED(SYS_MALLOC_F) && \
    !CONFIG_IS_ENABLED(INIT_STACK_WITHOUT_MALLOC_F)
	PTR_S	sp, GD_MALLOC_BASE(k0)	# gd->malloc_base offset
#endif
	.endm

	.set	noreorder

ENTRY(_start)
	b	1f
	 mtc0	zero, CP0_COUNT

	/* Stage header required by BootROM */
	.org	0x8
	.word	0		# ep, filled by mkimage
	.word	0		# stage_size, filled by mkimage
	.word	0		# has_stage2
	.word	0		# next_ep
	.word	0		# next_size
	.word	0		# next_offset

1:
	/* Init CP0 Status */
	mfc0	t0, CP0_STATUS
	and	t0, ST0_IMPL
	or	t0, ST0_BEV | ST0_ERL
	mtc0	t0, CP0_STATUS
	ehb

	/* Clear Watch Status bits and disable watch exceptions */
	li	t1, 0x7		# Clear I, R and W conditions
	init_wr	0
	init_wr	1
	init_wr	2
	init_wr	3

	/* Clear WP, IV and SW interrupts */
	mtc0	zero, CP0_CAUSE

	/* Clear timer interrupt (CP0_COUNT cleared on branch to 'reset') */
	mtc0	zero, CP0_COMPARE

	/* VPE1 goes to wait code directly */
	mfc0	t0, CP0_TCBIND
	andi	t0, TCBIND_CURVPE
	bnez	t0, launch_vpe_entry
	 nop

	/* Core1 goes to specific launch entry */
	PTR_LI	t0, KSEG1ADDR(CONFIG_MIPS_CM_BASE)
	lw	t1, GCR_Cx_ID(t0)
	bnez	t1, launch_core_entry
	 nop

	/* MT7530 reset */
	li	t0, KSEG1ADDR(SYSCTL_BASE)
	lw	t1, SYSCTL_RSTCTL_REG(t0)
	ori	t1, MCM_RST
	sw	t1, SYSCTL_RSTCTL_REG(t0)

	/* Disable DMA route for PSE SRAM set by BootROM */
	PTR_LI	t0, KSEG1ADDR(DMA_CFG_ARB_BASE)
	sw	zero, DMA_ROUTE_REG(t0)

	/* Set CPU clock to 500MHz (Required if boot from NAND) */
	li	t0, KSEG1ADDR(SYSCTL_BASE)
	lw	t1, SYSCTL_CLKCFG0_REG(t0)
	ins	t1, zero, 30, 2		# CPU_CLK_SEL
	sw	t1, SYSCTL_CLKCFG0_REG(t0)

	/* Set CPU clock divider to 1/1 */
	li	t0, KSEG1ADDR(RBUS_BASE)
	li	t1, 0x101
	sw	t1, RBUS_DYN_CFG0_REG(t0)

	/* (Re-)initialize the SRAM */
	bal	mips_sram_init
	 nop

	/* Set up temporary stack */
	li	sp, SP_ADDR_TEMP

	/* Setup full CPS */
	bal	mips_cm_map
	 nop

	bal	mt7621_cps_init
	 nop

	/* Prepare for CPU/DDR initialization binary blob */
	bal	prepare_stage_bin
	 nop

	/* Call CPU/DDR initialization binary blob */
	li	t9, STAGE_LOAD_ADDR
	jalr	t9
	 nop

	/* Switch CPU PLL source */
	li	t0, KSEG1ADDR(SYSCTL_BASE)
	lw	t1, SYSCTL_CLKCFG0_REG(t0)
	li	t2, 1
	ins	t1, t2, CPU_CLK_SEL_S, 2
	sw	t1, SYSCTL_CLKCFG0_REG(t0)

	/*
	 * Currently SPL is running on locked L2 cache (on KSEG0).
	 * To reset the entire cache, we have to writeback SPL to DRAM first.
	 * Cache flush won't work here. Use memcpy instead.
	 */

	la	a0, __text_start
	move	a1, a0
	la	a2, __image_copy_end
	sub	a2, a2, a1
	li	a3, 5
	ins	a0, a3, 29, 3	# convert to KSEG1

	bal	memcpy
	 nop

	/* Disable caches */
	bal	mips_cache_disable
	 nop

	/* Reset caches */
	bal	mips_cache_reset
	 nop

	/* Disable SRAM */
	li	t0, KSEG1ADDR(FE_BASE)
	li	t1, FE_PSE_RESET
	sw	t1, FE_RST_GLO_REG(t0)

	/* Clear the .bss section */
	la	a0, __bss_start
	la	a1, __bss_end
1:	sw	zero, 0(a0)
	addiu	a0, 4
	ble	a0, a1, 1b
	 nop

	/* Set up initial stack and global data */
	setup_stack_gd

#if CONFIG_IS_ENABLED(INIT_STACK_WITHOUT_MALLOC_F)
	/* Set malloc base */
	li	t0, (CFG_SYS_INIT_SP_ADDR + 15) & (~15)
	PTR_S	t0, GD_MALLOC_BASE(k0)	# gd->malloc_base offset
#endif

#if defined(CONFIG_DEBUG_UART) && defined(CONFIG_SPL_SERIAL)
	/* Earliest point to set up debug uart */
	bal	debug_uart_init
	 nop
#endif

	/* Setup timer */
	bal	set_timer_freq_simple
	 nop

	/* Bootup secondary CPUs */
	bal	secondary_cpu_init
	 nop

	move	a0, zero		# a0 <-- boot_flags = 0
	bal board_init_f
	 move	ra, zero

	END(_start)
