/*
 * Code fragment that stores the CPU state into the trap area
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: trap.S 596126 2015-10-29 19:53:48Z $
 */

#include <mipsinc.h>
#include <hnd_trap.h>

#define	_trap_hook	KSEG1ADDR(0x20)

	LEAF(hnd_set_trap)

	lui	a1,%hi(_trap_hook)
	lw	v0,%lo(_trap_hook)(a1)
	sw	a0,%lo(_trap_hook)(a1)
	jr	ra
	nop

	END(hnd_set_trap)

	LEAF(trap_init)
	.set	noreorder

	move	t2,ra

	/* Setup minimal trap handlers */
	bal	1f			/* Get the address of next insn */
	nop

	.set	noat
	la	k1,_trap_ent
	j	k1
	li	k0,0
	.set	at

1:	lw	a0,0(ra)		/* a0:	first half of la (high addr) */
	lw	a1,4(ra)		/* a1:	second half of la (low addr) */
	lw	a2,8(ra)		/* a2:	jump */
	lw	a3,12(ra)		/* a3:	load immediate */

	srl	ra,16			/* Get the high 16 bits of our address */
	or	a0,ra			/*  and add them to the instruction */

	li	t0,KSEG1
	li	t1,LAST_TRAP		/* Last trap location */
	addu	t1,t0

1:	sw	a0,0(t0)
	sw	a1,4(t0)
	sw	a2,8(t0)
	sw	a3,12(t0)
	addiu	a3,TRAP_STRIDE		/* Add to the load immediate */
	bne	t0,t1,1b
	addiu	t0,0x80

	lui	t0,%hi(_trap_hook)	/* Initialize trap hook */
	sw	zero,%lo(_trap_hook)(t0)

	/* Turn off BEV & ERL bits now that we have trap handlers */
	mfc0	t0,C0_STATUS
	li	t1,~(ST0_BEV | ST0_ERL)
	and	t0,t0,t1
	mtc0	t0,C0_STATUS

	jr	t2
	nop

	.set	noat

_trap_ent:
	sub	k1,sp,TRAP_T_SIZE
	sw	k0,TR_TYPE(k1)
	mfc0	k0,C0_STATUS
	sw	$0,TR_REGS+0x00(k1)
	sw	$1,TR_REGS+0x04(k1)
	sw	k0,TR_STATUS(k1)
	mfc0	k0,C0_CAUSE
	sw	$2,TR_REGS+0x08(k1)
	sw	$3,TR_REGS+0x0c(k1)
	sw	k0,TR_CAUSE(k1)
	mfc0	k0,C0_EPC
	sw	$4,TR_REGS+0x10(k1)
	sw	$5,TR_REGS+0x14(k1)
	sw	k0,TR_EPC(k1)
	mfhi	k0
	sw	$6,TR_REGS+0x18(k1)
	sw	$7,TR_REGS+0x1c(k1)
	sw	k0,TR_HI(k1)
	mflo	k0
	sw	$8,TR_REGS+0x20(k1)
	sw	$9,TR_REGS+0x24(k1)
	sw	k0,TR_LO(k1)
	mfc0	k0,C0_BADVADDR
	sw	$10,TR_REGS+0x28(k1)
	sw	$11,TR_REGS+0x2c(k1)
	sw	k0,TR_BVA(k1)
	mfc0	k0,C0_ERREPC
	sw	$12,TR_REGS+0x30(k1)
	sw	$13,TR_REGS+0x34(k1)
	sw	k0,TR_ERRPC(k1)
	sw	$14,TR_REGS+0x38(k1)
	sw	$15,TR_REGS+0x3c(k1)
	sw	$16,TR_REGS+0x40(k1)
	sw	$17,TR_REGS+0x44(k1)
	sw	$18,TR_REGS+0x48(k1)
	sw	$19,TR_REGS+0x4c(k1)
	sw	$20,TR_REGS+0x50(k1)
	sw	$21,TR_REGS+0x54(k1)
	sw	$22,TR_REGS+0x58(k1)
	sw	$23,TR_REGS+0x5c(k1)
	sw	$24,TR_REGS+0x60(k1)
	sw	$25,TR_REGS+0x64(k1)
	/* 26 & 27 (k0 & k1) not saved */
	sw	$28,TR_REGS+0x70(k1)
	sw	$29,TR_REGS+0x74(k1)
	sw	$30,TR_REGS+0x78(k1)
	sw	$31,TR_REGS+0x7c(k1)

	lui	k0,%hi(_trap_hook)
	lw	k0,%lo(_trap_hook)(k0)
	beqz	k0,_hnd_die
	nop

	la	ra,_rte
	move	sp,k1
	jr	k0
	move	a0,k1

	/* No trap hook, just die */
	.global	_hnd_die
_hnd_die:
	/* Block all interrupts and exceptions */
	mfc0	t0,C0_STATUS
	li	t1,ST0_EXL
	or	t0,t0,t1
	mtc0	t0,C0_STATUS
	/* Don't use wait instruction; it re-enables interrupts */
1:	b	1b
	nop

_rte:
	/* XXX:	Should we restore the status register from the frame?
	 * Linux restores the interrupt mask bits only.
	 */
	move	k1,sp
	lw	$31,TR_REGS+0x7c(k1)
	lw	$30,TR_REGS+0x78(k1)
	lw	k0,TR_LO(k1)
	lw	$29,TR_REGS+0x74(k1)
	lw	$28,TR_REGS+0x70(k1)
	mtlo	k0
	/* 26 & 27 (k0 & k1) not saved */
	lw	$25,TR_REGS+0x64(k1)
	lw	$24,TR_REGS+0x60(k1)
	lw	k0,TR_HI(k1)
	lw	$23,TR_REGS+0x5c(k1)
	lw	$22,TR_REGS+0x58(k1)
	mthi	k0
	lw	$21,TR_REGS+0x54(k1)
	lw	$20,TR_REGS+0x50(k1)
	lw	k0,TR_EPC(k1)
	lw	$19,TR_REGS+0x4c(k1)
	lw	$18,TR_REGS+0x48(k1)
	mtc0	k0,C0_EPC
	lw	$17,TR_REGS+0x44(k1)
	lw	$16,TR_REGS+0x40(k1)
	lw	$15,TR_REGS+0x3c(k1)
	lw	$14,TR_REGS+0x38(k1)
	lw	$13,TR_REGS+0x34(k1)
	lw	$12,TR_REGS+0x30(k1)
	lw	$11,TR_REGS+0x2c(k1)
	lw	$10,TR_REGS+0x28(k1)
	lw	$9,TR_REGS+0x24(k1)
	lw	$8,TR_REGS+0x20(k1)
	lw	$7,TR_REGS+0x1c(k1)
	lw	$6,TR_REGS+0x18(k1)
	lw	$5,TR_REGS+0x14(k1)
	lw	$4,TR_REGS+0x10(k1)
	lw	$3,TR_REGS+0x0c(k1)
	lw	$2,TR_REGS+0x08(k1)
	lw	$1,TR_REGS+0x04(k1)
	/* Don't "restore" zero */

	nop
	nop
	.set	mips32
	eret
	.set	mips0

	END(trap_init)
