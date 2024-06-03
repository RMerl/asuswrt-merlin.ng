/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  relocate - i.MX31-specific vector relocation
 *
 *  Copyright (c) 2013  Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

#include <linux/linkage.h>

/*
 * The i.MX31 SoC is very specific with respect to exceptions: it
 * does not provide RAM at the high vectors address (0xFFFF0000),
 * thus only the low address (0x00000000) is useable; but that is
 * in ROM, so let's avoid relocating the vectors.
 */
	.section	.text.relocate_vectors,"ax",%progbits

ENTRY(relocate_vectors)

	bx	lr

ENDPROC(relocate_vectors)
