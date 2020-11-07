/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
#include "MipsAsm.h"
	
/*
Public	void
BlockByteFill (int size, uchar pattern, uchar* dstPtr)
	*****************************************************************************
	*	This function fills uchar type array with the pattern					*
	*	a0 -- size																*
	*	a1 -- pattern
	*	a2 -- dstPtr
	*****************************************************************************
Public	void
BlockByteClear (int size, short* dstPtr)
	*****************************************************************************
	*	This function fills uchar type array with the 0							*
	*	a0 -- size																*
	*	a1 -- pattern
	*	a2 -- dstPtr
	*****************************************************************************
*/
#if defined(bcm47xx) && defined(USE_FAST_TEXT)
	.section .fast_text,"ax",@progbits
#else
	.text
#endif
	.align	2
	.globl  BlockByteClear
	.globl	BlockByteFill
	.ent	BlockByteClear	2
	.livereg	0x3000FF0F 0
	
BlockByteClear:
	.set noreorder
	move	a2, a1
	move	a1, zero
BlockByteFill:
	move	v1, ra

	/* if len <= 4 do straight byte move */
	srl		t0, a0, 2
	beqz	t0, ByteFillLoopStart
	move	v0, a0

	neg		t0, a2
	andi 	t0, t0, 3
	beq		t0, zero, ByteFillAligned
	addu	t1, a2, t0
FillByteLoop:
	addu	a2, 1
	bne		a2, t1, FillByteLoop
	sb		a1, -1(a2)

	sub		a0, t0
ByteFillAligned:
	andi	v0, a0, 3
	sll		t0, a1, 8
	or		t0, t0, a1
	sll		a1, t0, 16
	or		a1, a1, t0
	jal		BlockLongFill0
	sub		a0, a0, v0

ByteFillLoopStart:
	beq		v0, zero, EndByteFillLoop
	addu	t1, a2, v0

ByteFillLoop:
		addu	a2,	1
		bne		a2, t1, ByteFillLoop
		sb		a1, -1(a2)

EndByteFillLoop:
	move	ra, v1
	j	ra
	nop
	.set reorder
	.end	BlockByteClear

	
