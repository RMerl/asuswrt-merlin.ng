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
Public	int
BlockShortMove (int size, short* srcPtr, short* dstPtr)
	*****************************************************************************
	*	a0 -- size																*
	*	a1 -- srcPtr
	*	a2 -- dstPtr	
	*****************************************************************************
*/
#if defined(bcm47xx) && defined(USE_FAST_TEXT)
	.section .fast_text,"ax",@progbits
#else
	.text	
#endif
	.align	2
	.globl	BlockShortMove
	.ent	BlockShortMove	2
	.livereg	0x3000FF0F 0
	.extern	    BlockLongMoveStart
	
BlockShortMove:
	.set noreorder
	andi 	t1, a0, 0x1
	beq		t1, zero, ShortMoveEven

	/* move odd short */
	lh 		t0, 0(a1)
	addu	a1, 2
	sh		t0, 0(a2)
	addu 	a2, 2
	subu	a0, 1 

ShortMoveEven:
	beqz 	a0, EndShortMoveSlowLoop

	andi	t0, a0, 0x7
	sll		a0, 1
	bne		t0, zero, ShortMoveSlowLoop
	addu	v1, a1, a0

	or		t2, a1, a2
	andi	t2, 0x3
	bne		t2, zero, ShortMoveFastLoop

	lui		t1, %hi(BlockLongMoveStart)
	ori		t1, t1, %lo(BlockLongMoveStart)
	jr		t1

ShortMoveFastLoop:
		lh		t0, 0(a1)
		lh		t1, 2(a1)
		lh		t2, 4(a1)
		lh		t3, 6(a1)
		lh		t4, 8(a1)
		lh		t5, 10(a1)
		lh		t6, 12(a1)
		lh		t7, 14(a1)
		sh		t0,	0(a2)
		sh		t1,	2(a2)
		sh		t2,	4(a2)
		sh		t3,	6(a2)
		sh		t4,	8(a2)
		sh		t5,	10(a2)
		sh		t6,	12(a2)

		addu	a1,	16
		addu	a2,	16
		bne		a1, v1, ShortMoveFastLoop
		sh		t7,	-2(a2)
EndShortMoveFastLoop:
	j	ra

ShortMoveSlowLoop:
		lh		t0, 0(a1)
		lh		t1, 2(a1)
		sh		t0,	0(a2)

		addu	a1,	4
		addu	a2,	4
		bne		a1, v1, ShortMoveSlowLoop
		sh		t1,	-2(a2)
EndShortMoveSlowLoop:
	j	ra
	nop
	.set reorder
	.end	BlockShortMove
