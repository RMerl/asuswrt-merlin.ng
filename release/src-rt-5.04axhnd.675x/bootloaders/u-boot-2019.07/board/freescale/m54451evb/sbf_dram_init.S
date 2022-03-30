/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board-specific sbf ddr/sdram init.
 *
 * (C) Copyright 2017 Angelo Dureghello <angelo@sysam.it>
 */

 #include <config.h>

.global sbf_dram_init
.text

sbf_dram_init:
	/* Dram Initialization a1, a2, and d0 */
	/* mscr sdram */
	move.l	#0xFC0A4074, %a1
	move.b	#(CONFIG_SYS_SDRAM_DRV_STRENGTH), (%a1)
	nop

	/* SDRAM Chip 0 and 1 */
	move.l	#0xFC0B8110, %a1
	move.l	#0xFC0B8114, %a2

	/* calculate the size */
	move.l	#0x13, %d1
	move.l	#(CONFIG_SYS_SDRAM_SIZE), %d2
#ifdef CONFIG_SYS_SDRAM_BASE1
	lsr.l	#1, %d2
#endif

dramsz_loop:
	lsr.l	#1, %d2
	add.l	#1, %d1
	cmp.l	#1, %d2
	bne	dramsz_loop
#ifdef CONFIG_SYS_NAND_BOOT
	beq	asm_nand_chk_status
#endif
	/* SDRAM Chip 0 and 1 */
	move.l	#(CONFIG_SYS_SDRAM_BASE), (%a1)
	or.l	%d1, (%a1)
#ifdef CONFIG_SYS_SDRAM_BASE1
	move.l	#(CONFIG_SYS_SDRAM_BASE1), (%a2)
	or.l	%d1, (%a2)
#endif
	nop

	/* dram cfg1 and cfg2 */
	move.l	#0xFC0B8008, %a1
	move.l	#(CONFIG_SYS_SDRAM_CFG1), (%a1)
	nop
	move.l	#0xFC0B800C, %a2
	move.l	#(CONFIG_SYS_SDRAM_CFG2), (%a2)
	nop

	move.l	#0xFC0B8000, %a1	/* Mode */
	move.l	#0xFC0B8004, %a2	/* Ctrl */

	/* Issue PALL */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 2), (%a2)
	nop

	move.l	#1000, %d1
	bsr	asm_delay

	/* Issue PALL */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 2), (%a2)
	nop

	/* Perform two refresh cycles */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 4), %d0
	nop
	move.l	%d0, (%a2)
	move.l	%d0, (%a2)
	nop

	/* Issue LEMR */
	move.l	#(CONFIG_SYS_SDRAM_MODE), (%a1)
	nop
	move.l	#(CONFIG_SYS_SDRAM_EMOD), (%a1)

	move.l	#500, %d1
	bsr	asm_delay

	move.l	#(CONFIG_SYS_SDRAM_CTRL), %d1
	and.l	#0x7FFFFFFF, %d1

	or.l	#0x10000C00, %d1

	move.l	%d1, (%a2)
	nop

	move.l	#2000, %d1
	bsr	asm_delay

	rts
