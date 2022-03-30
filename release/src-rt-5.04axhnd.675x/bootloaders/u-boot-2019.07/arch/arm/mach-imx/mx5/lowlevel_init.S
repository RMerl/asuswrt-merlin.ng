/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <asm/arch/imx-regs.h>
#include <generated/asm-offsets.h>
#include <linux/linkage.h>

.section ".text.init", "x"

.macro init_arm_erratum
	/* ARM erratum ID #468414 */
	mrc 15, 0, r1, c1, c0, 1
	orr r1, r1, #(1 << 5)    /* enable L1NEON bit */
	mcr 15, 0, r1, c1, c0, 1
.endm

/*
 * L2CC Cache setup/invalidation/disable
 */
.macro init_l2cc
	/* explicitly disable L2 cache */
	mrc 15, 0, r0, c1, c0, 1
	bic r0, r0, #0x2
	mcr 15, 0, r0, c1, c0, 1

	/* reconfigure L2 cache aux control reg */
	ldr r0, =0xC0 |			/* tag RAM */ \
		 0x4 |			/* data RAM */ \
		 1 << 24 |		/* disable write allocate delay */ \
		 1 << 23 |		/* disable write allocate combine */ \
		 1 << 22		/* disable write allocate */

#if defined(CONFIG_MX51)
	ldr r3, [r4, #ROM_SI_REV]
	cmp r3, #0x10

	/* disable write combine for TO 2 and lower revs */
	orrls r0, r0, #1 << 25
#endif

	mcr 15, 1, r0, c9, c0, 2

	/* enable L2 cache */
	mrc 15, 0, r0, c1, c0, 1
	orr r0, r0, #2
	mcr 15, 0, r0, c1, c0, 1

.endm /* init_l2cc */

/* AIPS setup - Only setup MPROTx registers.
 * The PACR default values are good.*/
.macro init_aips
	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	ldr r0, =AIPS1_BASE_ADDR
	ldr r1, =0x77777777
	str r1, [r0, #0x0]
	str r1, [r0, #0x4]
	ldr r0, =AIPS2_BASE_ADDR
	str r1, [r0, #0x0]
	str r1, [r0, #0x4]
	/*
	 * Clear the on and off peripheral modules Supervisor Protect bit
	 * for SDMA to access them. Did not change the AIPS control registers
	 * (offset 0x20) access type
	 */
.endm /* init_aips */

/* M4IF setup */
.macro init_m4if
#ifdef CONFIG_MX51
	/* VPU and IPU given higher priority (0x4)
	 * IPU accesses with ID=0x1 given highest priority (=0xA)
	 */
	ldr r0, =M4IF_BASE_ADDR

	ldr r1, =0x00000203
	str r1, [r0, #0x40]

	str r4, [r0, #0x44]

	ldr r1, =0x00120125
	str r1, [r0, #0x9C]

	ldr r1, =0x001901A3
	str r1, [r0, #0x48]

#endif
.endm /* init_m4if */

.macro setup_pll pll, freq
	ldr r0, =\pll
	adr r2, W_DP_\freq
	bl setup_pll_func
.endm

#define W_DP_OP		0
#define W_DP_MFD	4
#define W_DP_MFN	8

setup_pll_func:
	ldr r1, =0x00001232
	str r1, [r0, #PLL_DP_CTL] /* Set DPLL ON (set UPEN bit): BRMO=1 */
	mov r1, #0x2
	str r1, [r0, #PLL_DP_CONFIG] /* Enable auto-restart AREN bit */

	ldr r1, [r2, #W_DP_OP]
	str r1, [r0, #PLL_DP_OP]
	str r1, [r0, #PLL_DP_HFS_OP]

	ldr r1, [r2, #W_DP_MFD]
	str r1, [r0, #PLL_DP_MFD]
	str r1, [r0, #PLL_DP_HFS_MFD]

	ldr r1, [r2, #W_DP_MFN]
	str r1, [r0, #PLL_DP_MFN]
	str r1, [r0, #PLL_DP_HFS_MFN]

	ldr r1, =0x00001232
	str r1, [r0, #PLL_DP_CTL]
1:	ldr r1, [r0, #PLL_DP_CTL]
	ands r1, r1, #0x1
	beq 1b

	/* r10 saved upper lr */
	mov pc, lr

.macro setup_pll_errata pll, freq
	ldr r2, =\pll
	str r4, [r2, #PLL_DP_CONFIG] /* Disable auto-restart AREN bit */
	ldr r1, =0x00001236
	str r1, [r2, #PLL_DP_CTL]    /* Restart PLL with PLM=1 */
1:	ldr r1, [r2, #PLL_DP_CTL]    /* Wait for lock */
	ands r1, r1, #0x1
	beq 1b

	ldr r5, \freq
	str r5, [r2, #PLL_DP_MFN]    /* Modify MFN value */
	str r5, [r2, #PLL_DP_HFS_MFN]

	mov r1, #0x1
	str r1, [r2, #PLL_DP_CONFIG] /* Reload MFN value */

2:	ldr r1, [r2, #PLL_DP_CONFIG]
	tst r1, #1
	bne 2b

	ldr r1, =100		     /* Wait at least 4 us */
3:	subs r1, r1, #1
	bge 3b

	mov r1, #0x2
	str r1, [r2, #PLL_DP_CONFIG] /* Enable auto-restart AREN bit */
.endm

.macro init_clock
#if defined (CONFIG_MX51)
	ldr r0, =CCM_BASE_ADDR

	/* Gate of clocks to the peripherals first */
	ldr r1, =0x3FFFFFFF
	str r1, [r0, #CLKCTL_CCGR0]
	str r4, [r0, #CLKCTL_CCGR1]
	str r4, [r0, #CLKCTL_CCGR2]
	str r4, [r0, #CLKCTL_CCGR3]

	ldr r1, =0x00030000
	str r1, [r0, #CLKCTL_CCGR4]
	ldr r1, =0x00FFF030
	str r1, [r0, #CLKCTL_CCGR5]
	ldr r1, =0x00000300
	str r1, [r0, #CLKCTL_CCGR6]

	/* Disable IPU and HSC dividers */
	mov r1, #0x60000
	str r1, [r0, #CLKCTL_CCDR]

	/* Make sure to switch the DDR away from PLL 1 */
	ldr r1, =0x19239145
	str r1, [r0, #CLKCTL_CBCDR]
	/* make sure divider effective */
1:	ldr r1, [r0, #CLKCTL_CDHIPR]
	cmp r1, #0x0
	bne 1b

	/* Switch ARM to step clock */
	mov r1, #0x4
	str r1, [r0, #CLKCTL_CCSR]

#if defined(CONFIG_MX51_PLL_ERRATA)
	setup_pll PLL1_BASE_ADDR, 864
	setup_pll_errata PLL1_BASE_ADDR, W_DP_MFN_800_DIT
#else
	setup_pll PLL1_BASE_ADDR, 800
#endif

	setup_pll PLL3_BASE_ADDR, 665

	/* Switch peripheral to PLL 3 */
	ldr r0, =CCM_BASE_ADDR
	ldr r1, =0x000010C0 | CONFIG_SYS_DDR_CLKSEL
	str r1, [r0, #CLKCTL_CBCMR]
	ldr r1, =0x13239145
	str r1, [r0, #CLKCTL_CBCDR]
	setup_pll PLL2_BASE_ADDR, 665

	/* Switch peripheral to PLL2 */
	ldr r0, =CCM_BASE_ADDR
	ldr r1, =0x19239145
	str r1, [r0, #CLKCTL_CBCDR]
	ldr r1, =0x000020C0 | CONFIG_SYS_DDR_CLKSEL
	str r1, [r0, #CLKCTL_CBCMR]

	setup_pll PLL3_BASE_ADDR, 216

	/* Set the platform clock dividers */
	ldr r0, =ARM_BASE_ADDR
	ldr r1, =0x00000725
	str r1, [r0, #0x14]

	ldr r0, =CCM_BASE_ADDR

	/* Run 3.0 at Full speed, for other TO's wait till we increase VDDGP */
	ldr r3, [r4, #ROM_SI_REV]
	cmp r3, #0x10
	movls r1, #0x1
	movhi r1, #0

	str r1, [r0, #CLKCTL_CACRR]

	/* Switch ARM back to PLL 1 */
	str r4, [r0, #CLKCTL_CCSR]

	/* setup the rest */
	/* Use lp_apm (24MHz) source for perclk */
	ldr r1, =0x000020C2 | CONFIG_SYS_DDR_CLKSEL
	str r1, [r0, #CLKCTL_CBCMR]
	/* ddr clock from PLL 1, all perclk dividers are 1 since using 24MHz */
	ldr r1, =CONFIG_SYS_CLKTL_CBCDR
	str r1, [r0, #CLKCTL_CBCDR]

	/* Restore the default values in the Gate registers */
	ldr r1, =0xFFFFFFFF
	str r1, [r0, #CLKCTL_CCGR0]
	str r1, [r0, #CLKCTL_CCGR1]
	str r1, [r0, #CLKCTL_CCGR2]
	str r1, [r0, #CLKCTL_CCGR3]
	str r1, [r0, #CLKCTL_CCGR4]
	str r1, [r0, #CLKCTL_CCGR5]
	str r1, [r0, #CLKCTL_CCGR6]

	/* Use PLL 2 for UART's, get 66.5MHz from it */
	ldr r1, =0xA5A2A020
	str r1, [r0, #CLKCTL_CSCMR1]
	ldr r1, =0x00C30321
	str r1, [r0, #CLKCTL_CSCDR1]
	/* make sure divider effective */
1:	ldr r1, [r0, #CLKCTL_CDHIPR]
	cmp r1, #0x0
	bne 1b

	str r4, [r0, #CLKCTL_CCDR]

	/* for cko - for ARM div by 8 */
	mov r1, #0x000A0000
	add r1, r1, #0x00000F0
	str r1, [r0, #CLKCTL_CCOSR]
#else	/* CONFIG_MX53 */
	ldr r0, =CCM_BASE_ADDR

	/* Gate of clocks to the peripherals first */
	ldr r1, =0x3FFFFFFF
	str r1, [r0, #CLKCTL_CCGR0]
	str r4, [r0, #CLKCTL_CCGR1]
	str r4, [r0, #CLKCTL_CCGR2]
	str r4, [r0, #CLKCTL_CCGR3]
	str r4, [r0, #CLKCTL_CCGR7]
	ldr r1, =0x00030000
	str r1, [r0, #CLKCTL_CCGR4]
	ldr r1, =0x00FFF030
	str r1, [r0, #CLKCTL_CCGR5]
	ldr r1, =0x0F00030F
	str r1, [r0, #CLKCTL_CCGR6]

	/* Switch ARM to step clock */
	mov r1, #0x4
	str r1, [r0, #CLKCTL_CCSR]

	setup_pll PLL1_BASE_ADDR, 800

	setup_pll PLL3_BASE_ADDR, 400

	/* Switch peripheral to PLL3 */
	ldr r0, =CCM_BASE_ADDR
	ldr r1, =0x00015154
	str r1, [r0, #CLKCTL_CBCMR]
	ldr r1, =0x02898945
	str r1, [r0, #CLKCTL_CBCDR]
	/* make sure change is effective */
1:      ldr r1, [r0, #CLKCTL_CDHIPR]
	cmp r1, #0x0
	bne 1b

	setup_pll PLL2_BASE_ADDR, 400

	/* Switch peripheral to PLL2 */
	ldr r0, =CCM_BASE_ADDR
	ldr r1, =0x00888945
	str r1, [r0, #CLKCTL_CBCDR]

	ldr r1, =0x00016154
	str r1, [r0, #CLKCTL_CBCMR]

	/*change uart clk parent to pll2*/
	ldr r1, [r0, #CLKCTL_CSCMR1]
	and r1, r1, #0xfcffffff
	orr r1, r1, #0x01000000
	str r1, [r0, #CLKCTL_CSCMR1]

	/* make sure change is effective */
1:      ldr r1, [r0, #CLKCTL_CDHIPR]
	cmp r1, #0x0
	bne 1b

	setup_pll PLL3_BASE_ADDR, 216

	setup_pll PLL4_BASE_ADDR, 455

	/* Set the platform clock dividers */
	ldr r0, =ARM_BASE_ADDR
	ldr r1, =0x00000124
	str r1, [r0, #0x14]

	ldr r0, =CCM_BASE_ADDR
	mov r1, #0
	str r1, [r0, #CLKCTL_CACRR]

	/* Switch ARM back to PLL 1. */
	mov r1, #0x0
	str r1, [r0, #CLKCTL_CCSR]

	/* make uart div=6 */
	ldr r1, [r0, #CLKCTL_CSCDR1]
	and r1, r1, #0xffffffc0
	orr r1, r1, #0x0a
	str r1, [r0, #CLKCTL_CSCDR1]

	/* Restore the default values in the Gate registers */
	ldr r1, =0xFFFFFFFF
	str r1, [r0, #CLKCTL_CCGR0]
	str r1, [r0, #CLKCTL_CCGR1]
	str r1, [r0, #CLKCTL_CCGR2]
	str r1, [r0, #CLKCTL_CCGR3]
	str r1, [r0, #CLKCTL_CCGR4]
	str r1, [r0, #CLKCTL_CCGR5]
	str r1, [r0, #CLKCTL_CCGR6]
	str r1, [r0, #CLKCTL_CCGR7]

	mov r1, #0x00000
	str r1, [r0, #CLKCTL_CCDR]

	/* for cko - for ARM div by 8 */
	mov r1, #0x000A0000
	add r1, r1, #0x00000F0
	str r1, [r0, #CLKCTL_CCOSR]

#endif	/* CONFIG_MX53 */
.endm

ENTRY(lowlevel_init)
	mov r10, lr
	mov r4, #0	/* Fix R4 to 0 */

#if defined(CONFIG_SYS_MAIN_PWR_ON)
	ldr r0, =GPIO1_BASE_ADDR
	ldr r1, [r0, #0x0]
	orr r1, r1, #1 << 23
	str r1, [r0, #0x0]
	ldr r1, [r0, #0x4]
	orr r1, r1, #1 << 23
	str r1, [r0, #0x4]
#endif

	init_arm_erratum

	init_l2cc

	init_aips

	init_m4if

	init_clock

	mov pc, r10
ENDPROC(lowlevel_init)

/* Board level setting value */
#if defined(CONFIG_MX51_PLL_ERRATA)
W_DP_864:		.word DP_OP_864
			.word DP_MFD_864
			.word DP_MFN_864
W_DP_MFN_800_DIT:	.word DP_MFN_800_DIT
#else
W_DP_800:		.word DP_OP_800
			.word DP_MFD_800
			.word DP_MFN_800
#endif
#if defined(CONFIG_MX51)
W_DP_665:		.word DP_OP_665
			.word DP_MFD_665
			.word DP_MFN_665
#endif
W_DP_216:		.word DP_OP_216
			.word DP_MFD_216
			.word DP_MFN_216
W_DP_400:               .word DP_OP_400
			.word DP_MFD_400
			.word DP_MFN_400
W_DP_455:               .word DP_OP_455
			.word DP_MFD_455
			.word DP_MFN_455
