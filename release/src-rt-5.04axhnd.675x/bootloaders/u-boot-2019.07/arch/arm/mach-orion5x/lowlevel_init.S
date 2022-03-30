/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <config.h>
#include "asm/arch/orion5x.h"

/*
 * Configuration values for SDRAM access setup
 */

#define SDRAM_CONFIG			0x3148400
#define SDRAM_MODE			0x62
#define SDRAM_CONTROL			0x4041000
#define SDRAM_TIME_CTRL_LOW		0x11602220
#define SDRAM_TIME_CTRL_HI		0x40c
#define SDRAM_OPEN_PAGE_EN		0x0
/* DDR 1 2x 32M NANYA NT5DS16M16CS-6K ==> 64MB */
#define SDRAM_BANK0_SIZE		0x3ff0001
#define SDRAM_ADDR_CTRL			0x10

#define SDRAM_OP_NOP			0x05
#define SDRAM_OP_SETMODE		0x03

#define SDRAM_PAD_CTRL_WR_EN		0x80000000
#define SDRAM_PAD_CTRL_TUNE_EN		0x00010000
#define SDRAM_PAD_CTRL_DRVN_MASK	0x0000003f
#define SDRAM_PAD_CTRL_DRVP_MASK	0x00000fc0

/*
 * For Guideline MEM-3 - Drive Strength value
 */

#define DDR1_PAD_STRENGTH_DEFAULT	0x00001000
#define SDRAM_PAD_CTRL_DRV_STR_MASK	0x00003000

/*
 * For Guideline MEM-4 - DQS Reference Delay Tuning
 */

#define MSAR_ARMDDRCLCK_MASK		0x000000f0
#define MSAR_ARMDDRCLCK_H_MASK		0x00000100

#define MSAR_ARMDDRCLCK_333_167		0x00000000
#define MSAR_ARMDDRCLCK_500_167		0x00000030
#define MSAR_ARMDDRCLCK_667_167		0x00000060
#define MSAR_ARMDDRCLCK_400_200_1	0x000001E0
#define MSAR_ARMDDRCLCK_400_200		0x00000010
#define MSAR_ARMDDRCLCK_600_200		0x00000050
#define MSAR_ARMDDRCLCK_800_200		0x00000070

#define FTDLL_DDR1_166MHZ		0x0047F001

#define FTDLL_DDR1_200MHZ		0x0044D001

/*
 * Low-level init happens right after start.S has switched to SVC32,
 * flushed and disabled caches and disabled MMU. We're still running
 * from the boot chip select, so the first thing SPL should do is to
 * set up the RAM to copy U-Boot into.
 */

.globl lowlevel_init

lowlevel_init:

#ifdef CONFIG_SPL_BUILD

	/* Use 'r2 as the base for internal register accesses */
	ldr	r2, =ORION5X_REGS_PHY_BASE

	/* move internal registers from the default 0xD0000000
	 * to their intended location, defined by SoC */
	ldr	r3, =0xD0000000
	add	r3, r3, #0x20000
	str	r2, [r3, #0x80]

	/* Use R3 as the base for DRAM registers */
	add	r3, r2, #0x01000

	/*DDR SDRAM Initialization Control */
	ldr	r0, =0x00000001
	str	r0, [r3, #0x480]

	/* Use R3 as the base for PCI registers */
	add	r3, r2, #0x31000

	/* Disable arbiter */
	ldr	r0, =0x00000030
	str	r0, [r3, #0xd00]

	/* Use R3 as the base for DRAM registers */
	add	r3, r2, #0x01000

	/* set all dram windows to 0 */
	mov	r0, #0
	str	r0, [r3, #0x504]
	str	r0, [r3, #0x50C]
	str	r0, [r3, #0x514]
	str	r0, [r3, #0x51C]

	/* 1) Configure SDRAM  */
	ldr	r0, =SDRAM_CONFIG
	str	r0, [r3, #0x400]

	/* 2) Set SDRAM Control reg */
	ldr	r0, =SDRAM_CONTROL
	str	r0, [r3, #0x404]

	/* 3) Write SDRAM address control register */
	ldr	r0, =SDRAM_ADDR_CTRL
	str	r0, [r3, #0x410]

	/* 4) Write SDRAM bank 0 size register */
	ldr	r0, =SDRAM_BANK0_SIZE
	str	r0, [r3, #0x504]
	/* keep other banks disabled */

	/* 5) Write SDRAM open pages control register */
	ldr	r0, =SDRAM_OPEN_PAGE_EN
	str	r0, [r3, #0x414]

	/* 6) Write SDRAM timing Low register */
	ldr	r0, =SDRAM_TIME_CTRL_LOW
	str	r0, [r3, #0x408]

	/* 7) Write SDRAM timing High register */
	ldr	r0, =SDRAM_TIME_CTRL_HI
	str	r0, [r3, #0x40C]

	/* 8) Write SDRAM mode register */
	/* The CPU must not attempt to change the SDRAM Mode register setting */
	/* prior to DRAM controller completion of the DRAM initialization     */
	/* sequence. To guarantee this restriction, it is recommended that    */
	/* the CPU sets the SDRAM Operation register to NOP command, performs */
	/* read polling until the register is back in Normal operation value, */
	/* and then sets SDRAM Mode register to its new value.		      */

	/* 8.1 write 'nop' to SDRAM operation */
	ldr	r0, =SDRAM_OP_NOP
	str	r0, [r3, #0x418]

	/* 8.2 poll SDRAM operation until back in 'normal' mode.  */
1:
	ldr	r0, [r3, #0x418]
	cmp	r0, #0
	bne	1b

	/* 8.3 Now its safe to write new value to SDRAM Mode register	      */
	ldr	r0, =SDRAM_MODE
	str	r0, [r3, #0x41C]

	/* 8.4 Set new mode */
	ldr	r0, =SDRAM_OP_SETMODE
	str	r0, [r3, #0x418]

	/* 8.5 poll SDRAM operation until back in 'normal' mode.  */
2:
	ldr	r0, [r3, #0x418]
	cmp	r0, #0
	bne	2b

	/* DDR SDRAM Address/Control Pads Calibration */
	ldr	r0, [r3, #0x4C0]

	/* Set Bit [31] to make the register writable			*/
	orr	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	str	r0, [r3, #0x4C0]

	bic	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	bic	r0, r0, #SDRAM_PAD_CTRL_TUNE_EN
	bic	r0, r0, #SDRAM_PAD_CTRL_DRVN_MASK
	bic	r0, r0, #SDRAM_PAD_CTRL_DRVP_MASK

	/* Get the final N locked value of driving strength [22:17]	*/
	mov	r1, r0
	mov	r1, r1, LSL #9
	mov	r1, r1, LSR #26	 /* r1[5:0]<DrvN>  = r3[22:17]<LockN>	*/
	orr	r1, r1, r1, LSL #6 /* r1[11:6]<DrvP> = r1[5:0]<DrvN>	*/

	/* Write to both <DrvN> bits [5:0] and <DrvP> bits [11:6]	*/
	orr	r0, r0, r1
	str	r0, [r3, #0x4C0]

	/* DDR SDRAM Data Pads Calibration				*/
	ldr	r0, [r3, #0x4C4]

	/* Set Bit [31] to make the register writable			*/
	orr	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	str	r0, [r3, #0x4C4]

	bic	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	bic	r0, r0, #SDRAM_PAD_CTRL_TUNE_EN
	bic	r0, r0, #SDRAM_PAD_CTRL_DRVN_MASK
	bic	r0, r0, #SDRAM_PAD_CTRL_DRVP_MASK

	/* Get the final N locked value of driving strength [22:17]	*/
	mov	r1, r0
	mov	r1, r1, LSL #9
	mov	r1, r1, LSR #26
	orr	r1, r1, r1, LSL #6 /* r1[5:0] = r3[22:17]<LockN>	*/

	/* Write to both <DrvN> bits [5:0] and <DrvP> bits [11:6]	*/
	orr	r0, r0, r1

	str	r0, [r3, #0x4C4]

	/* Implement Guideline (GL# MEM-3) Drive Strength Value		*/
	/* Relevant for: 88F5181-A1/B0/B1 and 88F5281-A0/B0		*/

	ldr	r1, =DDR1_PAD_STRENGTH_DEFAULT

	/* Enable writes to DDR SDRAM Addr/Ctrl Pads Calibration register */
	ldr	r0, [r3, #0x4C0]
	orr	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	str	r0, [r3, #0x4C0]

	/* Correct strength and disable writes again */
	bic	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	bic	r0, r0, #SDRAM_PAD_CTRL_DRV_STR_MASK
	orr	r0, r0, r1
	str	r0, [r3, #0x4C0]

	/* Enable writes to DDR SDRAM Data Pads Calibration register */
	ldr	r0, [r3, #0x4C4]
	orr	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	str	r0, [r3, #0x4C4]

	/* Correct strength and disable writes again */
	bic	r0, r0, #SDRAM_PAD_CTRL_DRV_STR_MASK
	bic	r0, r0, #SDRAM_PAD_CTRL_WR_EN
	orr	r0, r0, r1
	str	r0, [r3, #0x4C4]

	/* Implement Guideline (GL# MEM-4) DQS Reference Delay Tuning	*/
	/* Relevant for: 88F5181-A1/B0/B1 and 88F5281-A0/B0		*/

	/* Get the "sample on reset" register for the DDR frequancy	*/
	ldr	r3, =0x10000
	ldr	r0, [r3, #0x010]
	ldr	r1, =MSAR_ARMDDRCLCK_MASK
	and	r1, r0, r1

	ldr	r0, =FTDLL_DDR1_166MHZ
	cmp	r1, #MSAR_ARMDDRCLCK_333_167
	beq	3f
	cmp	r1, #MSAR_ARMDDRCLCK_500_167
	beq	3f
	cmp	r1, #MSAR_ARMDDRCLCK_667_167
	beq	3f

	ldr	r0, =FTDLL_DDR1_200MHZ
	cmp	r1, #MSAR_ARMDDRCLCK_400_200_1
	beq	3f
	cmp	r1, #MSAR_ARMDDRCLCK_400_200
	beq	3f
	cmp	r1, #MSAR_ARMDDRCLCK_600_200
	beq	3f
	cmp	r1, #MSAR_ARMDDRCLCK_800_200
	beq	3f

	ldr	r0, =0

3:
	/* Use R3 as the base for DRAM registers */
	add	r3, r2, #0x01000

	ldr	r2, [r3, #0x484]
	orr	r2, r2, r0
	str	r2, [r3, #0x484]

	/* enable for 2 GB DDR; detection should find out real amount */
	sub	r0, r0, r0
	str	r0, [r3, #0x500]
	ldr	r0, =0x7fff0001
	str	r0, [r3, #0x504]

#endif /* CONFIG_SPL_BUILD */

	/* Return to U-Boot via saved link register */
	mov	pc, lr
