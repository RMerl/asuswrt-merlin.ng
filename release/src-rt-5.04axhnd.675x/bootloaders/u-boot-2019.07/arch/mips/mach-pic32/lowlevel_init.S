/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
*/

#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/asm.h>

LEAF(lowlevel_init)
	/*
	 * Establish Cause
	 * (set IV bit)
	 */
	li	t1, 0x00800000
	mtc0	t1, CP0_CAUSE

	/* Establish Wired (and Random) */
	mtc0	zero, CP0_WIRED
	 nop

	jr	ra
	 nop
	END(lowlevel_init)
