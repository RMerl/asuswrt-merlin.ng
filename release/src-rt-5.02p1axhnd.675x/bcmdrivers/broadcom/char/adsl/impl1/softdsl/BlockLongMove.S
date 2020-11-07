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
BlockLongClear0(int size, long* dstPtr)
BlockLongFill0 (int size, long pattern, long* dstPtr)
	*****************************************************************************
	*	This function fills long type array with the pattern					*
	*	a0 -- size (in bytes)													*
	*	a1 -- pattern
	*	a2 -- dstPtr
	*****************************************************************************
*/

	.text
	.align	2
	.globl  BlockLongClear0
	.globl	BlockLongFill0
	.ent	BlockLongFill0	2
	.livereg	0x3000FF0F 0
	
BlockLongClear0:
	.set noreorder
	move	a2, a1
	move	a1, zero
BlockLongFill0:
	beqz	a0, EndLongFillLoop

	andi 	t0, a0, 0xF
	beq		t0, zero, LongFillLoopStart
	addu	t1, a2, t0
LongFill0Loop:
	addu	a2, 4
	bne		a2, t1, LongFill0Loop
	sw 		a1, -4(a2)

	sub		a0, a0, t0
	beqz	a0, EndLongFillLoop

	addu	a0, a2, a0
	b		LongFillLoop
	nop

	.set reorder
	.end	BlockLongFill0

/*
Public	void
BlockLongFill (int size, long pattern, long* dstPtr)
	*****************************************************************************
	*	This function fills long type array with the pattern					*
	*	a0 -- size																*
	*	a1 -- pattern
	*	a2 -- dstPtr
	*****************************************************************************
Public	void
BlockLongClear (int size, short* dstPtr)
	*****************************************************************************
	*	This function fills long type array with the 0							*
	*	a0 -- size																*
	*	a1 -- pattern
	*	a2 -- dstPtr
	*****************************************************************************
*/

	.text
	.align	2
	.globl  BlockLongClear
	.globl	BlockLongFill
	.ent	BlockLongFill	2
	.ent	BlockLongClear	2
	.livereg	0x3000FF0F 0
	
BlockLongClear:
	.set noreorder
	move	a2, a1
	move	a1, zero
BlockLongFill:
	blezl	a0, EndLongFillLoop
	ASSERT_INTR
	andi 	t0, a0, 0x3
	bnel	t0, zero, EndLongFillLoop
	ASSERT_INTR
	sll		a0, 2
LongFillLoopStart:
	addu	a0, a2, a0
LongFillLoop:
		sw		a1, 0(a2)
		sw		a1,	4(a2)
		sw		a1, 8(a2)
		addu	a2,	16
		bne		a2, a0, LongFillLoop
		sw		a1,	-4(a2)
EndLongFillLoop:
	j	ra
	nop
	.set reorder
	.end	BlockLongFill


/*
Public	void
BlockLongMove0 (int size, long* srcPtr, long* dstPtr)
	*****************************************************************************
	*	a0 -- size (in bytes)													*
	*	a1 -- srcPtr
	*	a2 -- dstPtr	
	*****************************************************************************
*/
	.text	
	.align	2
	.globl	BlockLongMove0
	.ent	BlockLongMove0	2
	.livereg	0x3000FF0F 0
	
BlockLongMove0:
	.set noreorder

	beqz	a0, EndLongMoveLoop
	andi 	t0, a0, 0xF
	beq		t0, zero, BlockLongMoveStart_loc
	addu	t1, a1, t0
LongMove0Loop:
	lw 		t2, 0(a1)
	addu	a1, 4
	addu	a2, 4
	bne		a1, t1, LongMove0Loop
	sw 		t2, -4(a2)

	sub		a0, a0, t0
	beqz	a0, EndLongMoveLoop

	addu	a0, a1, a0
	b		LongMoveLoop
	nop
	
	.set reorder
	.end	BlockLongMove0
	
/*
Public	void
BlockLongMove (int size, long* srcPtr, long* dstPtr)
	*****************************************************************************
	*	a0 -- size																*
	*	a1 -- srcPtr
	*	a2 -- dstPtr	
	*****************************************************************************
*/
	.text
	.align	2
	.globl	BlockLongMove
	.globl	BlockLongMoveStart
	.ent	BlockLongMove	2
	.ent	BlockLongMoveStart	2
	.livereg	0x3000FF0F 0
	
BlockLongMove:
	.set noreorder
	beqz	a0, EndLongMoveLoop
	andi 	t0, a0, 0x3
	bnel	t0, zero, EndLongMoveLoop
	ASSERT_INTR
	sll		a0, 2
BlockLongMoveStart:
BlockLongMoveStart_loc:
	addu	a0, a1, a0
LongMoveLoop:
		lw		t0, 0(a1)
		lw		t1, 4(a1)
		lw		t2, 8(a1)
		lw		t3, 12(a1)
		sw		t0,	0(a2)
		sw		t1,	4(a2)
		sw		t2,	8(a2)

		addu	a1,	16
		addu	a2,	16
		bne		a1, a0, LongMoveLoop
		sw		t3,	-4(a2)
EndLongMoveLoop:
	j	ra
	nop
	.set reorder
	.end	BlockLongMove
