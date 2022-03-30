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

.pushsection .text.__ashldi3, "ax"
ENTRY(__ashldi3)
ENTRY(__aeabi_llsl)

	subs	r3, r2, #32
	rsb	ip, r2, #32
	movmi	ah, ah, lsl r2
	movpl	ah, al, lsl r3
 ARM(	orrmi	ah, ah, al, lsr ip	)
 THUMB(	lsrmi	r3, al, ip		)
 THUMB(	orrmi	ah, ah, r3		)
	mov	al, al, lsl r2
	ret	lr

ENDPROC(__ashldi3)
ENDPROC(__aeabi_llsl)
.popsection
