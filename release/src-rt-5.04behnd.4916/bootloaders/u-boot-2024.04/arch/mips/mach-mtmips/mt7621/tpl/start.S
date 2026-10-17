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
#include <asm/addrspace.h>
#include <asm/mipsregs.h>
#include <asm/cm.h>
#include "../mt7621.h"

#define SP_ADDR_TEMP		0xbe10dff0

	.set noreorder

	.macro init_wr sel
	MTC0	zero, CP0_WATCHLO,\sel
	mtc0	t1, CP0_WATCHHI,\sel
	.endm

	.macro uhi_mips_exception
	move	k0, t9		# preserve t9 in k0
	move	k1, a0		# preserve a0 in k1
	li	t9, 15		# UHI exception operation
	li	a0, 0		# Use hard register context
	sdbbp	1		# Invoke UHI operation
	.endm

ENTRY(_start)
	b	reset
	 mtc0	zero, CP0_COUNT

	/*
	 * Store TPL size here.
	 * This will be used by SPL to locate u-boot payload.
	 */
	.org	TPL_INFO_OFFSET
	.word	TPL_INFO_MAGIC
	.word	__image_copy_len

	/* Exception vector */
	.org 0x200
	/* TLB refill, 32 bit task */
	uhi_mips_exception

	.org 0x280
	/* XTLB refill, 64 bit task */
	uhi_mips_exception

	.org 0x300
	/* Cache error exception */
	uhi_mips_exception

	.org 0x380
	/* General exception */
	uhi_mips_exception

	.org 0x400
	/* Catch interrupt exceptions */
	uhi_mips_exception

	.org 0x480
	/* EJTAG debug exception */
1:	b	1b
	 nop

	.org 0x500

reset:
	/* Set KSEG0 to Uncached */
	mfc0	t0, CP0_CONFIG
	ins	t0, zero, 0, 3
	ori	t0, t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
	ehb

	/* Check for CPU number */
	mfc0	t0, CP0_EBASE
	and	t0, t0, MIPS_EBASE_CPUNUM
	beqz	t0, 1f
	 nop

	/* Secondary core goes to specified SPL entry address */
	li	t0, KSEG1ADDR(SYSCTL_BASE)
	lw	t0, BOOT_SRAM_BASE_REG(t0)
	jr	t0
	 nop

	/* Init CP0 Status */
1:	mfc0	t0, CP0_STATUS
	and	t0, ST0_IMPL
	or	t0, ST0_BEV | ST0_ERL
	mtc0	t0, CP0_STATUS
	 nop

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

	/* Setup basic CPS */
	bal	mips_cm_map
	 nop

	li	t0, KSEG1ADDR(CONFIG_MIPS_CM_BASE)
	li	t1, GCR_REG0_BASE_VALUE
	sw	t1, GCR_REG0_BASE(t0)

	li	t1, ((GCR_REG0_MASK_VALUE << GCR_REGn_MASK_ADDRMASK_SHIFT) | \
		    GCR_REGn_MASK_CMTGT_IOCU0)
	sw	t1, GCR_REG0_MASK(t0)

	lw	t1, GCR_BASE(t0)
	ins	t1, zero, 0, 2		# CM_DEFAULT_TARGET
	sw	t1, GCR_BASE(t0)

	lw	t1, GCR_CONTROL(t0)
	li	t2, GCR_CONTROL_SYNCCTL
	or	t1, t1, t2
	sw	t1, GCR_CONTROL(t0)

	/* Increase SPI frequency */
	li	t0, KSEG1ADDR(SPI_BASE)
	li	t1, 5
	sw	t1, SPI_SPACE_REG(t0)

	/* Set CPU clock to 500MHz */
	li	t0, KSEG1ADDR(SYSCTL_BASE)
	lw	t1, SYSCTL_CLKCFG0_REG(t0)
	ins	t1, zero, 30, 2		# CPU_CLK_SEL
	sw	t1, SYSCTL_CLKCFG0_REG(t0)

	/* Set CPU clock divider to 1/1 */
	li	t0, KSEG1ADDR(RBUS_BASE)
	li	t1, 0x101
	sw	t1, RBUS_DYN_CFG0_REG(t0)

	/* Initialize the SRAM */
	bal	mips_sram_init
	 nop

	/* Set up initial stack */
	li	sp, SP_ADDR_TEMP

	bal	tpl_main
	 nop

	END(_start)
