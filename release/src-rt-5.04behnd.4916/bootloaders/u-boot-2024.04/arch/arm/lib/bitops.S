/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Sean Anderson <sean.anderson@seco.com>
 *
 * ARM bitops to call when using THUMB1, which doesn't have these instructions.
 */
#include <linux/linkage.h>
#include <asm/assembler.h>

.pushsection .text.__fls
ENTRY(__fls)
	clz	r0, r0
	rsb	r0, r0, #31
	ret	lr
ENDPROC(__fls)
.popsection

.pushsection .text.__ffs
ENTRY(__ffs)
	rsb	r3, r0, #0
	and	r0, r0, r3
	clz	r0, r0
	rsb	r0, r0, #31
	ret	lr
ENDPROC(__ffs)
.popsection

.pushsection .text.fls
ENTRY(fls)
	cmp	r0, #0
	clzne	r0, r0
	rsbne	r0, r0, #32
	ret	lr
ENDPROC(fls)
.popsection

.pushsection .text.ffs
ENTRY(ffs)
	rsb	r3, r0, #0
	and	r0, r0, r3
	clz	r0, r0
	rsb	r0, r0, #32
	ret	lr
ENDPROC(ffs)
.popsection
