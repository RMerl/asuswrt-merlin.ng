/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 1995, 1997, 2007-2013 Free Software Foundation, Inc.
 */

 /* ANSI concatenation macros.  */

 #define CONCAT1(a, b) CONCAT2(a, b)
 #define CONCAT2(a, b) a ## b

 /* Use the right prefix for global labels.  */

 #define SYM(x) CONCAT1 (__USER_LABEL_PREFIX__, x)

#ifndef WORKING_ASSEMBLER
#define abs_l abs
#define asl_l asl
#define mov_l mov
#endif

#define FUNC(X)         .type SYM(X),@function
#define HIDDEN_FUNC(X)	FUNC(X)` .hidden X
#define ENDFUNC0(X)     .Lfe_##X: .size X,.Lfe_##X-X
#define ENDFUNC(X)      ENDFUNC0(X)

	.section .text
	.align 4
	.global SYM(__st_r13_to_r15)
	.global SYM(__st_r13_to_r16)
	.global SYM(__st_r13_to_r17)
	.global SYM(__st_r13_to_r18)
	.global SYM(__st_r13_to_r19)
	.global SYM(__st_r13_to_r20)
	.global SYM(__st_r13_to_r21)
	.global SYM(__st_r13_to_r22)
	.global SYM(__st_r13_to_r23)
	.global SYM(__st_r13_to_r24)
	.global SYM(__st_r13_to_r25)
	HIDDEN_FUNC(__st_r13_to_r15)
	HIDDEN_FUNC(__st_r13_to_r16)
	HIDDEN_FUNC(__st_r13_to_r17)
	HIDDEN_FUNC(__st_r13_to_r18)
	HIDDEN_FUNC(__st_r13_to_r19)
	HIDDEN_FUNC(__st_r13_to_r20)
	HIDDEN_FUNC(__st_r13_to_r21)
	HIDDEN_FUNC(__st_r13_to_r22)
	HIDDEN_FUNC(__st_r13_to_r23)
	HIDDEN_FUNC(__st_r13_to_r24)
	HIDDEN_FUNC(__st_r13_to_r25)
	.align 4
SYM(__st_r13_to_r25):
	st r25, [sp,48]
SYM(__st_r13_to_r24):
	st r24, [sp,44]
SYM(__st_r13_to_r23):
	st r23, [sp,40]
SYM(__st_r13_to_r22):
	st r22, [sp,36]
SYM(__st_r13_to_r21):
	st r21, [sp,32]
SYM(__st_r13_to_r20):
	st r20, [sp,28]
SYM(__st_r13_to_r19):
	st r19, [sp,24]
SYM(__st_r13_to_r18):
	st r18, [sp,20]
SYM(__st_r13_to_r17):
	st r17, [sp,16]
SYM(__st_r13_to_r16):
	st r16, [sp,12]
SYM(__st_r13_to_r15):
#ifdef __ARC700__
	st r15, [sp,8] ; minimum function size to avoid stall: 6 bytes.
#else
	st_s r15, [sp,8]
#endif
	st_s r14, [sp,4]
	j_s.d [%blink]
	st_s r13, [sp,0]
	ENDFUNC(__st_r13_to_r15)
	ENDFUNC(__st_r13_to_r16)
	ENDFUNC(__st_r13_to_r17)
	ENDFUNC(__st_r13_to_r18)
	ENDFUNC(__st_r13_to_r19)
	ENDFUNC(__st_r13_to_r20)
	ENDFUNC(__st_r13_to_r21)
	ENDFUNC(__st_r13_to_r22)
	ENDFUNC(__st_r13_to_r23)
	ENDFUNC(__st_r13_to_r24)
	ENDFUNC(__st_r13_to_r25)

	.section .text
	.align 4
;	==================================
;	the loads

	.global SYM(__ld_r13_to_r15)
	.global SYM(__ld_r13_to_r16)
	.global SYM(__ld_r13_to_r17)
	.global SYM(__ld_r13_to_r18)
	.global SYM(__ld_r13_to_r19)
	.global SYM(__ld_r13_to_r20)
	.global SYM(__ld_r13_to_r21)
	.global SYM(__ld_r13_to_r22)
	.global SYM(__ld_r13_to_r23)
	.global SYM(__ld_r13_to_r24)
	.global SYM(__ld_r13_to_r25)
	HIDDEN_FUNC(__ld_r13_to_r15)
	HIDDEN_FUNC(__ld_r13_to_r16)
	HIDDEN_FUNC(__ld_r13_to_r17)
	HIDDEN_FUNC(__ld_r13_to_r18)
	HIDDEN_FUNC(__ld_r13_to_r19)
	HIDDEN_FUNC(__ld_r13_to_r20)
	HIDDEN_FUNC(__ld_r13_to_r21)
	HIDDEN_FUNC(__ld_r13_to_r22)
	HIDDEN_FUNC(__ld_r13_to_r23)
	HIDDEN_FUNC(__ld_r13_to_r24)
	HIDDEN_FUNC(__ld_r13_to_r25)
SYM(__ld_r13_to_r25):
	ld r25, [sp,48]
SYM(__ld_r13_to_r24):
	ld r24, [sp,44]
SYM(__ld_r13_to_r23):
	ld r23, [sp,40]
SYM(__ld_r13_to_r22):
	ld r22, [sp,36]
SYM(__ld_r13_to_r21):
	ld r21, [sp,32]
SYM(__ld_r13_to_r20):
	ld r20, [sp,28]
SYM(__ld_r13_to_r19):
	ld r19, [sp,24]
SYM(__ld_r13_to_r18):
	ld r18, [sp,20]
SYM(__ld_r13_to_r17):
	ld r17, [sp,16]
SYM(__ld_r13_to_r16):
	ld r16, [sp,12]
SYM(__ld_r13_to_r15):
#ifdef __ARC700__
	ld r15, [sp,8] ; minimum function size to avoid stall: 6 bytes.
#else
	ld_s r15, [sp,8]
#endif
	ld_s r14, [sp,4]
	j_s.d [%blink]
	ld_s r13, [sp,0]
	ENDFUNC(__ld_r13_to_r15)
	ENDFUNC(__ld_r13_to_r16)
	ENDFUNC(__ld_r13_to_r17)
	ENDFUNC(__ld_r13_to_r18)
	ENDFUNC(__ld_r13_to_r19)
	ENDFUNC(__ld_r13_to_r20)
	ENDFUNC(__ld_r13_to_r21)
	ENDFUNC(__ld_r13_to_r22)
	ENDFUNC(__ld_r13_to_r23)
	ENDFUNC(__ld_r13_to_r24)
	ENDFUNC(__ld_r13_to_r25)

	.global SYM(__ld_r13_to_r14_ret)
	.global SYM(__ld_r13_to_r15_ret)
	.global SYM(__ld_r13_to_r16_ret)
	.global SYM(__ld_r13_to_r17_ret)
	.global SYM(__ld_r13_to_r18_ret)
	.global SYM(__ld_r13_to_r19_ret)
	.global SYM(__ld_r13_to_r20_ret)
	.global SYM(__ld_r13_to_r21_ret)
	.global SYM(__ld_r13_to_r22_ret)
	.global SYM(__ld_r13_to_r23_ret)
	.global SYM(__ld_r13_to_r24_ret)
	.global SYM(__ld_r13_to_r25_ret)
	HIDDEN_FUNC(__ld_r13_to_r14_ret)
	HIDDEN_FUNC(__ld_r13_to_r15_ret)
	HIDDEN_FUNC(__ld_r13_to_r16_ret)
	HIDDEN_FUNC(__ld_r13_to_r17_ret)
	HIDDEN_FUNC(__ld_r13_to_r18_ret)
	HIDDEN_FUNC(__ld_r13_to_r19_ret)
	HIDDEN_FUNC(__ld_r13_to_r20_ret)
	HIDDEN_FUNC(__ld_r13_to_r21_ret)
	HIDDEN_FUNC(__ld_r13_to_r22_ret)
	HIDDEN_FUNC(__ld_r13_to_r23_ret)
	HIDDEN_FUNC(__ld_r13_to_r24_ret)
	HIDDEN_FUNC(__ld_r13_to_r25_ret)
	.section .text
	.align 4
SYM(__ld_r13_to_r25_ret):
	ld r25, [sp,48]
SYM(__ld_r13_to_r24_ret):
	ld r24, [sp,44]
SYM(__ld_r13_to_r23_ret):
	ld r23, [sp,40]
SYM(__ld_r13_to_r22_ret):
	ld r22, [sp,36]
SYM(__ld_r13_to_r21_ret):
	ld r21, [sp,32]
SYM(__ld_r13_to_r20_ret):
	ld r20, [sp,28]
SYM(__ld_r13_to_r19_ret):
	ld r19, [sp,24]
SYM(__ld_r13_to_r18_ret):
	ld r18, [sp,20]
SYM(__ld_r13_to_r17_ret):
	ld r17, [sp,16]
SYM(__ld_r13_to_r16_ret):
	ld r16, [sp,12]
SYM(__ld_r13_to_r15_ret):
	ld r15, [sp,8]
SYM(__ld_r13_to_r14_ret):
	ld blink,[sp,r12]
	ld_s r14, [sp,4]
	ld.ab r13, [sp,r12]
	j_s.d [%blink]
	add_s sp,sp,4
	ENDFUNC(__ld_r13_to_r14_ret)
	ENDFUNC(__ld_r13_to_r15_ret)
	ENDFUNC(__ld_r13_to_r16_ret)
	ENDFUNC(__ld_r13_to_r17_ret)
	ENDFUNC(__ld_r13_to_r18_ret)
	ENDFUNC(__ld_r13_to_r19_ret)
	ENDFUNC(__ld_r13_to_r20_ret)
	ENDFUNC(__ld_r13_to_r21_ret)
	ENDFUNC(__ld_r13_to_r22_ret)
	ENDFUNC(__ld_r13_to_r23_ret)
	ENDFUNC(__ld_r13_to_r24_ret)
	ENDFUNC(__ld_r13_to_r25_ret)
