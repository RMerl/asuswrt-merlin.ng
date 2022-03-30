/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  relocate - i.MX27-specific vector relocation
 *
 *  Copyright (c) 2013  Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>

/*
 * The i.MX27 SoC is very specific with respect to exceptions: it
 * does not provide RAM at the high vectors address (0xFFFF0000),
 * thus only the low address (0x00000000) is useable; but that is
 * in ROM. Therefore, vectors cannot be changed at all.
 *
 * However, these ROM-based vectors actually just perform indirect
 * calls through pointers located in RAM at SoC-specific addresses,
 * as follows:
 *
 * Offset      Exception              Use by ROM code
 * 0x00000000  reset                  indirect branch to [0x00000014]
 * 0x00000004  undefined instruction  indirect branch to [0xfffffef0]
 * 0x00000008  software interrupt     indirect branch to [0xfffffef4]
 * 0x0000000c  prefetch abort         indirect branch to [0xfffffef8]
 * 0x00000010  data abort             indirect branch to [0xfffffefc]
 * 0x00000014  (reserved in ARMv5)    vector to ROM reset: 0xc0000000
 * 0x00000018  IRQ                    indirect branch to [0xffffff00]
 * 0x0000001c  FIQ                    indirect branch to [0xffffff04]
 *
 * In order to initialize exceptions on i.MX27, we must copy U-Boot's
 * indirect (not exception!) vector table into 0xfffffef0..0xffffff04
 * taking care not to copy vectors number 5 (reserved exception).
 */

	.section	.text.relocate_vectors,"ax",%progbits

ENTRY(relocate_vectors)

	ldr	r0, [r9, #GD_RELOCADDR]	/* r0 = gd->relocaddr */
	ldr	r1, =32			/* size of vector table */
	add	r0, r0, r1		/* skip to indirect table */
	ldr	r1, =0xFFFFFEF0		/* i.MX27 indirect table */
	ldmia	r0!, {r2-r8}		/* load indirect vectors 1..7 */
	stmia	r1!, {r2-r5, r7,r8}	/* write all but vector 5 */

	bx	lr

ENDPROC(relocate_vectors)
