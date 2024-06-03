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

.pushsection .text.__lshldi3, "ax"
ENTRY(__lshrdi3)
ENTRY(__aeabi_llsr)

	subs	r3, r2, #32
	rsb	ip, r2, #32
	movmi	al, al, lsr r2
	movpl	al, ah, lsr r3
 ARM(	orrmi	al, al, ah, lsl ip	)
 THUMB(	lslmi	r3, ah, ip		)
 THUMB(	orrmi	al, al, r3		)
	mov	ah, ah, lsr r2
	ret	lr

ENDPROC(__lshrdi3)
ENDPROC(__aeabi_llsr)
.popsection
