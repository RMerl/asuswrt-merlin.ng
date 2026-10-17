/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 1994, 1995, 1997, 1998, 1999, 2000, 2001, 2002, 2003,
   2004, 2005, 2006
   Free Software Foundation, Inc.
 */

!! libgcc routines for the Renesas / SuperH SH CPUs.
!! Contributed by Steve Chamberlain.
!! sac@cygnus.com

!! ashiftrt_r4_x, ___ashrsi3, ___ashlsi3, ___lshrsi3 routines
!! recoded in assembly by Toshiyasu Morita
!! tm@netcom.com

/* SH2 optimizations for ___ashrsi3, ___ashlsi3, ___lshrsi3 and
   ELF local label prefixes by J"orn Rennecke
   amylaar@cygnus.com  */

!
! GLOBAL(ashlsi3)
!
! Entry:
!
! r4: Value to shift
! r5: Shifts
!
! Exit:
!
! r0: Result
!
! Destroys:
!
! (none)
!
	.global	__ashlsi3
	.align	2
__ashlsi3:
	mov	#31,r0
	and	r0,r5
	mova	__ashlsi3_table,r0
	mov.b	@(r0,r5),r5
#ifdef __sh1__
	add	r5,r0
	jmp	@r0
#else
	braf	r5
#endif
	mov	r4,r0

	.align	2
__ashlsi3_table:
	.byte		__ashlsi3_0-__ashlsi3_table
	.byte		__ashlsi3_1-__ashlsi3_table
	.byte		__ashlsi3_2-__ashlsi3_table
	.byte		__ashlsi3_3-__ashlsi3_table
	.byte		__ashlsi3_4-__ashlsi3_table
	.byte		__ashlsi3_5-__ashlsi3_table
	.byte		__ashlsi3_6-__ashlsi3_table
	.byte		__ashlsi3_7-__ashlsi3_table
	.byte		__ashlsi3_8-__ashlsi3_table
	.byte		__ashlsi3_9-__ashlsi3_table
	.byte		__ashlsi3_10-__ashlsi3_table
	.byte		__ashlsi3_11-__ashlsi3_table
	.byte		__ashlsi3_12-__ashlsi3_table
	.byte		__ashlsi3_13-__ashlsi3_table
	.byte		__ashlsi3_14-__ashlsi3_table
	.byte		__ashlsi3_15-__ashlsi3_table
	.byte		__ashlsi3_16-__ashlsi3_table
	.byte		__ashlsi3_17-__ashlsi3_table
	.byte		__ashlsi3_18-__ashlsi3_table
	.byte		__ashlsi3_19-__ashlsi3_table
	.byte		__ashlsi3_20-__ashlsi3_table
	.byte		__ashlsi3_21-__ashlsi3_table
	.byte		__ashlsi3_22-__ashlsi3_table
	.byte		__ashlsi3_23-__ashlsi3_table
	.byte		__ashlsi3_24-__ashlsi3_table
	.byte		__ashlsi3_25-__ashlsi3_table
	.byte		__ashlsi3_26-__ashlsi3_table
	.byte		__ashlsi3_27-__ashlsi3_table
	.byte		__ashlsi3_28-__ashlsi3_table
	.byte		__ashlsi3_29-__ashlsi3_table
	.byte		__ashlsi3_30-__ashlsi3_table
	.byte		__ashlsi3_31-__ashlsi3_table

__ashlsi3_6:
	shll2	r0
__ashlsi3_4:
	shll2	r0
__ashlsi3_2:
	rts
	shll2	r0

__ashlsi3_7:
	shll2	r0
__ashlsi3_5:
	shll2	r0
__ashlsi3_3:
	shll2	r0
__ashlsi3_1:
	rts
	shll	r0

__ashlsi3_14:
	shll2	r0
__ashlsi3_12:
	shll2	r0
__ashlsi3_10:
	shll2	r0
__ashlsi3_8:
	rts
	shll8	r0

__ashlsi3_15:
	shll2	r0
__ashlsi3_13:
	shll2	r0
__ashlsi3_11:
	shll2	r0
__ashlsi3_9:
	shll8	r0
	rts
	shll	r0

__ashlsi3_22:
	shll2	r0
__ashlsi3_20:
	shll2	r0
__ashlsi3_18:
	shll2	r0
__ashlsi3_16:
	rts
	shll16	r0

__ashlsi3_23:
	shll2	r0
__ashlsi3_21:
	shll2	r0
__ashlsi3_19:
	shll2	r0
__ashlsi3_17:
	shll16	r0
	rts
	shll	r0

__ashlsi3_30:
	shll2	r0
__ashlsi3_28:
	shll2	r0
__ashlsi3_26:
	shll2	r0
__ashlsi3_24:
	shll16	r0
	rts
	shll8	r0

__ashlsi3_31:
	shll2	r0
__ashlsi3_29:
	shll2	r0
__ashlsi3_27:
	shll2	r0
__ashlsi3_25:
	shll16	r0
	shll8	r0
	rts
	shll	r0

__ashlsi3_0:
	rts
	nop
