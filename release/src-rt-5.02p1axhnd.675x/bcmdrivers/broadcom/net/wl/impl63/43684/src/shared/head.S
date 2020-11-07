/*
 * RTE Run-time initialization
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: head.S 654158 2016-08-11 09:30:01Z $
 */

#include <mipsinc.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbsocram.h>
#include <bcmdevs.h>

#ifdef	CONFIG_XIP
#error	Use boot.S+sbsdram.S for XIP
#endif // endif

/* Debug macro - write a number to a pair of chipc regs - use it with caution,
 *  the registers being used only exist in chip rev >= 22, meaning NOT in 5354
 *  and previous chips. Also,  it changes k0 and k1 registers.
 *  Value can be read from epidiag -j using "pci r 0x180000d0 4"
 */
#ifdef	BCMDBG
#if	defined(IL_BIGENDIAN) && defined(BCMHND74K)
#define	BPADDR_OFF	4
#define	BPDATA_OFF	12
#else
#define	BPADDR_OFF	0
#define	BPDATA_OFF	8
#endif // endif

#define TRACEINIT(x) \
	li	k0,KSEG1ADDR(0x180000d0); \
	li	k1,x; \
	sw	k1,BPADDR_OFF(k0)

#define TRACE(x) \
	li	k1,x; \
	sw	k1,BPADDR_OFF(k0)

#define TRACE2(x) \
	li	k1,x; \
	sw	k1,BPDATA_OFF(k0)

#else
#define TRACEINIT(x)
#define TRACE(x)
#define TRACE2(x)
#endif	/* BCMDBG */

	.text
	LEAF(startup)
	.set	noreorder

	/* Check that we booted from SDRAM */
	TRACEINIT(0x484400)
	bal	1f
	nop
1:	li	t0,PHYSADDR_MASK
	and	s0,t0,ra			# s0: Physaddr of our load address
	li	t1,SI_FLASH1
	blt	s0,t1,checksi
	nop

	/* We don't do flash */
panic:
	TRACE2(0x48440f1a)
	b	panic
	nop

	/* Figure out if we have an SB or AI chip */
checksi:
	TRACE(0x484401)
	li	s2,KSEG1			# s2 = KSEG1 Memory base
	li	a0,KSEG1ADDR(SI_ENUM_BASE_DEFAULT)
	li	t0,CID_TYPE_MASK
	lw	t1,CC_CHIPID(a0)
	and	t1,t0,t1
	srl	s7,t1,CID_TYPE_SHIFT		# s7 = ChipType (0 for SB, = 1 for AI)
	bne	s7,zero,aichip
	nop

sbchip:	TRACE(0x484402)
	lw	t1,CC_CHIPID(a0)		# ChipID register
	li	t2,CID_ID_MASK			# chip id is bit 0-15
	li	a3,BCM5354_CHIP_ID		# 5354 ChipID
	and	t2,t1,t2
	bne	t2,a3,ramcheck			# if not 5354 then skip
	nop

	li	t2,0x1				# Need define
	sw	t2,PMU_REG_CONTROL_ADDR(a0)
	li 	t3,0x6800000			# Should only affect the switch bits
	sw	t3,PMU_REG_CONTROL_DATA(a0)

	/*
	 * Trim the output voltage of the 1.2V BB switcher and 2.5V
	 * regulator to the correct value.
	 */
	li	t2,0x0
	sw	t2,PMU_REG_CONTROL_ADDR(a0)
	li 	t3,0x2000			# Reduce the output voltage of
	sw	t3,PMU_REG_CONTROL_DATA(a0)	# BB switcher to get 1.2V
	li	t2,0x3
	sw	t2,PMU_REG_CONTROL_ADDR(a0)
	li 	t3,0x02000000			# Increase the output voltage
	sw	t3,PMU_REG_CONTROL_DATA(a0)	# of VDDP LDO to get 2.5V

	lw	t2,PMU_CTL(a0)			# Check if PLL has been programmed
	andi	t2,t2,PCTL_XTALFREQ_MASK
	bnez	t2,3f				# Yup, leave it alone
	nop
	li	t2,0x7ffff			# Should only turn off the PLL bit
	sw	t2,PMU_MIN_RES_MASK(a0)		# Disable base band PLL
	sw	t2,PMU_MAX_RES_MASK(a0)
	nop

	/* Init code for FF4 space without TLB, enabling RAC */
3:	li 	t0,0x1fa0000c			# Set up CBR to 0x1fax_xxxx
	mtc0 	t0,$22,6
	li 	t1,0x1fa00000
	lw 	t2,0x14(t1)
	or 	t3,t2,0xc0000000		# enable ffxx_xxxx space # without programming TLB
	sw 	t3,0x14(t1)
	li 	t0,0xff40000c			# change CBR to ff4x_xxxx
	mtc0 	t0,$22,6

	/* Check for socram */
	/* XXX:	 This is assuming that if there is an socram core that's the only
	 * memory on the chip. Although that is true for all existing SB chips
	 * it could be false at some point. If that happens we should do something
	 * like the ai code below that checks to see if we are running on socram.
	 */
ramcheck:
	TRACE(0x484403)
	lw	v1,(SBCONFIGOFF + SBIDHIGH)(a0)
	and	a1,v1,SBIDH_CC_MASK
	srl	a1,a1,SBIDH_CC_SHIFT
	beq	a1,SOCRAM_CORE_ID,issocram
	nop
	beq	a1,MEMC_CORE_ID,szmem
	nop
	beq	a1,SDRAM_CORE_ID,szmem
	nop
	addu	a0,SI_CORE_SIZE
	bne	a1,(SBIDH_CC_MASK >> SBIDH_CC_SHIFT),ramcheck
	nop
	b	panic				# No mem core
	nop

aichip:
	TRACE(0x484404)
	/* XXX:	Should search the EROM, find the socram core and figure out
	 * its address, instead of using SI_FASTRAM
	 */
	li	t0,SI_FASTRAM			# Socram address in chips with both
	blt	s0,t0,szmem
	nop

	or	s2,s2,s0			# Adjust KSEG1 Mem base (for cache init)
	and	s2,s2,0xff000000

	/* Should branch to issocram (after finding the reg address from the
	 * erom above) to figure out its size
	 */
	li	v0,(128 * 1024)
	b	setsp
	nop

	/* Some SDRAM controller, size the memory */
szmem:
	li	t2,0xbeefaa55
	sw	zero,0(s2)
	li	v0,(1 << 20)
	li	v1,(128 << 20)

4:	or	t0,v0,s2
	sw	t2,0(t0)
	lw	t1,0(s2)
	beq	t1,t2,setsp
	nop

	sll	v0,v0,1
	bne	v0,v1,4b
	nop
	/* Didn't find an alias, must be 128MB */

	b	setsp
	nop

	/* The socram core tells us how much memory there is */
issocram:
	TRACE(0x484405)
	lw	t0,SR_COREINFO(a0)
	and	t0,t0,SRCI_MS0_MASK
	add	t0,t0,SR_MS0_BASE
	li	v0,1
	sll	v0,v0,t0

	/* v0 now contains memory size in bytes */

setsp:
	TRACE(0x484406)
	/* Record the memory size */
	la	t0,_memsize
	sw	v0,0(t0)

	/* Set up stack pointer in KSEG0 */
	or	v0,s2
	li	t0,PHYSADDR_MASK
	and	v0,v0,t0
	or	v0,KSEG0
	sub	sp,v0,4

#if defined(BCMSDIODEV) || defined(BCMHOSTVARS)
	/*
	 * Check if the host wrote nvram vars at the end of memory.  If so,
	 * record their location/size and start the stack pointer below them.
	 * If valid, the vars length is in words (4 bytes) a multiple of 4 encoded in the last
	 * word of memory as (~len << 16 | len).  The vars precede the length.
	 */
	lw	t0,0(sp)
	srl	t1,t0,0x10
	sll	t2,t0,0x10
	or	t1,t1,t2
	nor	t1,zero,t1
	bne	t1,t0,clearbss
	and	t0,t0,0xffff
	sll	t0,t0,2
	sub	sp,sp,t0
	la	t1,_varsz
	sw	t0,0(t1)
	la	t1,_vars
	sw	sp,0(t1)
	sub	sp,sp,4
#endif /* BCMSDIODEV || BCMHOSTVARS */

	/* Clear BSS */
clearbss:
	TRACE(0x484407)
	la	a0,bss_start
	la	a1,bss_end
1:	sw	zero,0(a0)
	addi	a0,a0,4
	blt	a0,a1,1b
	nop

	/* Setup trap handlers */
settrap:
	TRACE(0x484408)
	la	t0,trap_init
	jalr	t0
	nop
	TRACE(0x484409)

	/* Jump to C */
gomain:
	la	t0,c_main
	jalr	t0
	move	a0,ra

	/* If c_main returns, stop */
theend:	TRACE2(0x4844dead)
	nop
	nop
	.set	mips32
	wait
	.set	mips0
	nop
	nop
	b	theend
	nop

	.set reorder
	END(startup)

	.data
	.globl	_memsize
_memsize:
	.word	0
	.globl	__watermark
__watermark:
	.word	0

#if defined(BCMSDIODEV) || defined(BCMHOSTVARS)
	.globl	_varsz
_varsz:
	.word	0
	.globl	_vars
_vars:
	.word	0
#endif /* BCMSDIODEV || BCMHOSTVARS */
