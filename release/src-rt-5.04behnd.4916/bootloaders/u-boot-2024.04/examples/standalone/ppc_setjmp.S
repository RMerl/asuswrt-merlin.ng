/* SPDX-License-Identifier: LGPL-2.1+ */
/* setjmp for PowerPC.
   Copyright (C) 1995, 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
 */

#include <ppc_asm.tmpl>

# define JB_GPR1   0  /* Also known as the stack pointer */
# define JB_GPR2   1
# define JB_LR     2  /* The address we will return to */
# define JB_GPRS   3  /* GPRs 14 through 31 are saved, 18 in total */
# define JB_CR     21 /* Condition code registers. */
# define JB_FPRS   22 /* FPRs 14 through 31 are saved, 18*2 words total */
# define JB_SIZE   (58*4)

#define FP(x...) x

.globl      setctxsp;
setctxsp:
	mr  r1, r3
	blr

.globl      ppc_setjmp;
ppc_setjmp:
	stw  r1,(JB_GPR1*4)(3)
	mflr r0
	stw  r2,(JB_GPR2*4)(3)
	stw  r14,((JB_GPRS+0)*4)(3)
FP(	stfd 14,((JB_FPRS+0*2)*4)(3))
	stw  r0,(JB_LR*4)(3)
	stw  r15,((JB_GPRS+1)*4)(3)
FP(	stfd 15,((JB_FPRS+1*2)*4)(3))
	mfcr r0
	stw  r16,((JB_GPRS+2)*4)(3)
FP(	stfd 16,((JB_FPRS+2*2)*4)(3))
	stw  r0,(JB_CR*4)(3)
	stw  r17,((JB_GPRS+3)*4)(3)
FP(	stfd 17,((JB_FPRS+3*2)*4)(3))
	stw  r18,((JB_GPRS+4)*4)(3)
FP(	stfd 18,((JB_FPRS+4*2)*4)(3))
	stw  r19,((JB_GPRS+5)*4)(3)
FP(	stfd 19,((JB_FPRS+5*2)*4)(3))
	stw  r20,((JB_GPRS+6)*4)(3)
FP(	stfd 20,((JB_FPRS+6*2)*4)(3))
	stw  r21,((JB_GPRS+7)*4)(3)
FP(	stfd 21,((JB_FPRS+7*2)*4)(3))
	stw  r22,((JB_GPRS+8)*4)(3)
FP(	stfd 22,((JB_FPRS+8*2)*4)(3))
	stw  r23,((JB_GPRS+9)*4)(3)
FP(	stfd 23,((JB_FPRS+9*2)*4)(3))
	stw  r24,((JB_GPRS+10)*4)(3)
FP(	stfd 24,((JB_FPRS+10*2)*4)(3))
	stw  r25,((JB_GPRS+11)*4)(3)
FP(	stfd 25,((JB_FPRS+11*2)*4)(3))
	stw  r26,((JB_GPRS+12)*4)(3)
FP(	stfd 26,((JB_FPRS+12*2)*4)(3))
	stw  r27,((JB_GPRS+13)*4)(3)
FP(	stfd 27,((JB_FPRS+13*2)*4)(3))
	stw  r28,((JB_GPRS+14)*4)(3)
FP(	stfd 28,((JB_FPRS+14*2)*4)(3))
	stw  r29,((JB_GPRS+15)*4)(3)
FP(	stfd 29,((JB_FPRS+15*2)*4)(3))
	stw  r30,((JB_GPRS+16)*4)(3)
FP(	stfd 30,((JB_FPRS+16*2)*4)(3))
	stw  r31,((JB_GPRS+17)*4)(3)
FP(	stfd 31,((JB_FPRS+17*2)*4)(3))

	li 3, 0
	blr
