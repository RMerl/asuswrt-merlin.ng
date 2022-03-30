/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 1995, 1996, 1998, 1999, 2000, 2003, 2004, 2005
   Free Software Foundation, Inc.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

#ifdef __ARMEB__
#define al r1
#define ah r0
#else
#define al r0
#define ah r1
#endif

.pushsection .text.__ashrdi3, "ax"
ENTRY(__ashrdi3)
ENTRY(__aeabi_lasr)

	subs	r3, r2, #32
	rsb	ip, r2, #32
	movmi	al, al, lsr r2
	movpl	al, ah, asr r3
 ARM(	orrmi	al, al, ah, lsl ip	)
 THUMB(	lslmi	r3, ah, ip		)
 THUMB(	orrmi	al, al, r3		)
	mov	ah, ah, asr r2
	ret	lr

ENDPROC(__ashrdi3)
ENDPROC(__aeabi_lasr)
.popsection
