/* Memory sub-system initialization code */

#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>

	.text
	.set noreorder
	.set mips32

	.globl	lowlevel_init
lowlevel_init:

	/*
	 * Step 2) Establish Status Register
	 * (set BEV, clear ERL, clear EXL, clear IE)
	 */
	li	t1, 0x00400000
	mtc0	t1, CP0_STATUS

	/*
	 * Step 3) Establish CP0 Config0
	 * (set K0=3)
	 */
	li	t1, 0x00000003
	mtc0	t1, CP0_CONFIG

	/*
	 * Step 7) Establish Cause
	 * (set IV bit)
	 */
	li	t1, 0x00800000
	mtc0	t1, CP0_CAUSE

	/* Establish Wired (and Random) */
	mtc0	zero, CP0_WIRED
	nop

	jr	ra
	nop
