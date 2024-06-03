/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 */

#ifndef __MACRO_H__
#define __MACRO_H__
#ifdef __ASSEMBLY__

.macro	write32, addr, data
	mov.l \addr ,r1
	mov.l \data ,r0
	mov.l r0, @r1
.endm

.macro	write16, addr, data
	mov.l \addr ,r1
	mov.w \data ,r0
	mov.w r0, @r1
.endm

.macro	write8, addr, data
	mov.l \addr ,r1
	mov.l \data ,r0
	mov.b r0, @r1
.endm

.macro	wait_timer, time
	mov.l	\time ,r3
1:
	nop
	tst	r3, r3
	bf/s	1b
	dt	r3
.endm

#endif /* __ASSEMBLY__ */
#endif /* __MACRO_H__ */
