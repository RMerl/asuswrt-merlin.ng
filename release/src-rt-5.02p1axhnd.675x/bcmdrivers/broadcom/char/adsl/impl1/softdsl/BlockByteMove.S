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
BlockByteMove (int size, Byte* srcPtr, Byte* dstPtr)
	*****************************************************************************
	*	a0 -- size																*
	*	a1 -- srcPtr
	*	a2 -- dstPtr	
	*****************************************************************************
*/
	.text
	.align	2
	.globl	BlockByteMove
	.ent	BlockByteMove	2
	.livereg	0x3000FF0F 0
	
BlockByteMove:
	.set noreorder

	/* if len <= 4 do straight byte move */
	srl		t0, a0, 2
	beqz	t0, ByteMoveLastLoopStart

	xor		v0, a1,a2
	andi	v0, v0,0x3
	beqz	v0, BlockByteMoveByLong_loc

	neg		t0, a2
	andi 	t0, t0, 3
	beqz	t0, ByteMoveDstAligned
	addu	t1, a1, t0
ByteMoveLoop:
	lb		t2, 0(a1)
	addu	a1, 1
	sb		t2, 0(a2)
	bne		a1, t1, ByteMoveLoop
	addu 	a2, 1 
	sub		a0, t0

ByteMoveDstAligned:
	andi	t0, a0, 3
	sub		t0, a0, t0
	beqz	t0, ByteMoveLastLoopStart

	addu	t1, a1, t0
ByteMoveFastLoop:
	lwl		t0, 0(a1)
	lwr		t0, 3(a1)
	addu	a1, 4
	sw		t0, 0(a2)
	bne		a1, t1, ByteMoveFastLoop
	addu	a2, 4

	andi 	a0, a0, 3

ByteMoveLastLoopStart:
	beqz	a0, EndByteMoveLoop
	addu	t1, a1, a0
ByteMoveLoop1:
	lb 		t2, 0(a1)
	addu	a1, 1
	sb		t2, 0(a2)
	bne		a1, t1, ByteMoveLoop1
	addu	a2, 1

EndByteMoveLoop:
	j	ra
	nop
	.set reorder
	.end	BlockByteMove

/*
Public	void
BlockByteMoveByLong (int size, long* srcPtr, long* dstPtr)
	*****************************************************************************
	*	a0 -- size (in bytes)													*
	*	a1 -- srcPtr
	*	a2 -- dstPtr	
	*****************************************************************************
*/
	.text	
	.align	2
	.globl	BlockByteMoveByLong
	.ent	BlockByteMoveByLong	2
	.livereg	0x3000FF0F 0
BlockByteMoveByLong:
	.set noreorder

BlockByteMoveByLong_loc:
	move	v1, ra

	neg		t0, a1
	andi 	t0, t0, 3
	beq		t0, zero, ByLongAligned
	addu	t1, a1, t0
ByLongByteLoop:
	lb		t2, 0(a1)
	addu	a1, 1
	sb		t2, 0(a2)
	bne		a1, t1, ByLongByteLoop
	addu	a2, 1

	sub		a0, t0
ByLongAligned:
	andi	v0, a0, 3
	jal		BlockLongMove0
	sub		a0, a0, v0

	beq		v0, zero, EndByLongMove
	addu	t1, a1, v0

ByLongByteLoop1:
	lb 		t2, 0(a1)
	addu	a1, 1
	sb		t2, 0(a2)
	bne		a1, t1, ByLongByteLoop1
	addu	a2, 1

EndByLongMove:
	move	ra, v1
	j	ra
	nop
	
	.set reorder
	.end	BlockByteMoveByLong

