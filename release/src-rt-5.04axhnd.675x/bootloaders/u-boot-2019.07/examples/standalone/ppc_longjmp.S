/* SPDX-License-Identifier: LGPL-2.1+ */
/* longjmp for PowerPC.
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
#define FP(x...) x

.globl      ppc_longjmp;

ppc_longjmp:
	lwz r1,(JB_GPR1*4)(r3)
	lwz r2,(JB_GPR2*4)(r3)
	lwz r0,(JB_LR*4)(r3)
	lwz r14,((JB_GPRS+0)*4)(r3)
FP(	lfd 14,((JB_FPRS+0*2)*4)(r3))
	lwz r15,((JB_GPRS+1)*4)(r3)
FP(	lfd 15,((JB_FPRS+1*2)*4)(r3))
	lwz r16,((JB_GPRS+2)*4)(r3)
FP(	lfd 16,((JB_FPRS+2*2)*4)(r3))
	lwz r17,((JB_GPRS+3)*4)(r3)
FP(	lfd 17,((JB_FPRS+3*2)*4)(r3))
	lwz r18,((JB_GPRS+4)*4)(r3)
FP(	lfd 18,((JB_FPRS+4*2)*4)(r3))
	lwz r19,((JB_GPRS+5)*4)(r3)
FP(	lfd 19,((JB_FPRS+5*2)*4)(r3))
	lwz r20,((JB_GPRS+6)*4)(r3)
FP(	lfd 20,((JB_FPRS+6*2)*4)(r3))
	mtlr r0
	lwz r21,((JB_GPRS+7)*4)(r3)
FP(	lfd 21,((JB_FPRS+7*2)*4)(r3))
	lwz r22,((JB_GPRS+8)*4)(r3)
FP(	lfd 22,((JB_FPRS+8*2)*4)(r3))
	lwz r0,(JB_CR*4)(r3)
	lwz r23,((JB_GPRS+9)*4)(r3)
FP(	lfd 23,((JB_FPRS+9*2)*4)(r3))
	lwz r24,((JB_GPRS+10)*4)(r3)
FP(	lfd 24,((JB_FPRS+10*2)*4)(r3))
	lwz r25,((JB_GPRS+11)*4)(r3)
FP(	lfd 25,((JB_FPRS+11*2)*4)(r3))
	mtcrf 0xFF,r0
	lwz r26,((JB_GPRS+12)*4)(r3)
FP(	lfd 26,((JB_FPRS+12*2)*4)(r3))
	lwz r27,((JB_GPRS+13)*4)(r3)
FP(	lfd 27,((JB_FPRS+13*2)*4)(r3))
	lwz r28,((JB_GPRS+14)*4)(r3)
FP(	lfd 28,((JB_FPRS+14*2)*4)(r3))
	lwz r29,((JB_GPRS+15)*4)(r3)
FP(	lfd 29,((JB_FPRS+15*2)*4)(r3))
	lwz r30,((JB_GPRS+16)*4)(r3)
FP(	lfd 30,((JB_FPRS+16*2)*4)(r3))
	lwz r31,((JB_GPRS+17)*4)(r3)
FP(	lfd 31,((JB_FPRS+17*2)*4)(r3))
	mr r3,r4
	blr
