/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Philippe Reynes <tremyfr@yahoo.fr>
 */

#include <config.h>
#include <generated/asm-offsets.h>
#include <asm/macro.h>
#include <asm/arch/imx-regs.h>
#include "apf27.h"

	.macro init_aipi
	/*
	 * setup AIPI1 and AIPI2
	 */
	write32 AIPI1_PSR0, ACFG_AIPI1_PSR0_VAL
	write32 AIPI1_PSR1, ACFG_AIPI1_PSR1_VAL
	write32 AIPI2_PSR0, ACFG_AIPI2_PSR0_VAL
	write32 AIPI2_PSR1, ACFG_AIPI2_PSR1_VAL

	/* Change SDRAM signal strengh */
	ldr r0, =GPCR
	ldr r1, =ACFG_GPCR_VAL
	ldr r5, [r0]
	orr r5, r5, r1
	str r5, [r0]

	.endm /* init_aipi */

	.macro init_clock
	ldr r0, =CSCR
	/* disable MPLL/SPLL first */
	ldr r1, [r0]
	bic r1, r1, #(CSCR_MPEN|CSCR_SPEN)
	str r1, [r0]

 	/*
	 * pll clock initialization predefined in apf27.h
	 */
	write32 MPCTL0, ACFG_MPCTL0_VAL
	write32 SPCTL0, ACFG_SPCTL0_VAL

	write32 CSCR, ACFG_CSCR_VAL|CSCR_MPLL_RESTART|CSCR_SPLL_RESTART

	/*
	 * add some delay here
	 */
	mov r1, #0x1000
	1:  subs r1, r1, #0x1
	bne 1b

	/* peripheral clock divider */
	write32 PCDR0, ACFG_PCDR0_VAL
	write32 PCDR1, ACFG_PCDR1_VAL

	/* Configure PCCR0 and PCCR1 */
	write32 PCCR0, ACFG_PCCR0_VAL
	write32 PCCR1, ACFG_PCCR1_VAL

	.endm /* init_clock */

	.macro init_ddr
	/* wait for SDRAM/LPDDR ready (SDRAMRDY) */
	ldr		r0, =IMX_ESD_BASE
	ldr		r4, =ESDMISC_SDRAM_RDY
2:	ldr		r1, [r0, #ESDMISC_ROF]
	ands		r1, r1, r4
	bpl		2b

	/* LPDDR Soft Reset Mobile/Low Power DDR SDRAM. */
	ldr		r0, =IMX_ESD_BASE
	ldr		r4, =ACFG_ESDMISC_VAL
	orr		r1, r4, #ESDMISC_MDDR_DL_RST
	str		r1, [r0, #ESDMISC_ROF]

	/* Hold for more than 200ns */
	ldr		r1, =0x10000
1:	subs		r1, r1, #0x1
	bne		1b

	str		r4, [r0]

	ldr		r0, =IMX_ESD_BASE
	ldr		r1, =ACFG_SDRAM_ESDCFG_REGISTER_VAL
	str		r1, [r0, #ESDCFG0_ROF]

	ldr		r0, =IMX_ESD_BASE
	ldr		r1, =ACFG_PRECHARGE_CMD
	str		r1, [r0, #ESDCTL0_ROF]

	/* write8(0xA0001000, any value) */
	ldr		r1, =PHYS_SDRAM_1+ACFG_SDRAM_PRECHARGE_ALL_VAL
	strb		r2, [r1]

	ldr		r1, =ACFG_AUTOREFRESH_CMD
	str		r1, [r0, #ESDCTL0_ROF]

	ldr 		r4, =PHYS_SDRAM_1	/* CSD0 base address	*/

	ldr 		r6,=0x7		/* load loop counter	*/
1:	str 		r5,[r4]		/* run auto-refresh cycle to array 0 */
	subs 		r6,r6,#1
	bne 1b

	ldr		r1, =ACFG_SET_MODE_REG_CMD
	str		r1, [r0, #ESDCTL0_ROF]

	/* set standard mode register */
	ldr		r4, = PHYS_SDRAM_1+ACFG_SDRAM_MODE_REGISTER_VAL
	strb		r2, [r4]

	/* set extended mode register */
	ldr		r4, =PHYS_SDRAM_1+ACFG_SDRAM_EXT_MODE_REGISTER_VAL
	strb		r5, [r4]

	ldr		r1, =ACFG_NORMAL_RW_CMD
	str		r1, [r0, #ESDCTL0_ROF]

	/* 2nd sdram */
	ldr		r0, =IMX_ESD_BASE
	ldr		r1, =ACFG_SDRAM_ESDCFG_REGISTER_VAL
	str		r1, [r0, #ESDCFG1_ROF]

	ldr		r0, =IMX_ESD_BASE
	ldr		r1, =ACFG_PRECHARGE_CMD
	str		r1, [r0, #ESDCTL1_ROF]

	/* write8(0xB0001000, any value) */
	ldr		r1, =PHYS_SDRAM_2+ACFG_SDRAM_PRECHARGE_ALL_VAL
	strb		r2, [r1]

	ldr		r1, =ACFG_AUTOREFRESH_CMD
	str		r1, [r0, #ESDCTL1_ROF]

	ldr 		r4, =PHYS_SDRAM_2	/* CSD1 base address */

	ldr 		r6,=0x7		/* load loop counter */
1:	str 		r5,[r4]		/* run auto-refresh cycle to array 0 */
	subs 		r6,r6,#1
	bne 1b

	ldr		r1, =ACFG_SET_MODE_REG_CMD
	str		r1, [r0, #ESDCTL1_ROF]

	/* set standard mode register */
	ldr		r4, =PHYS_SDRAM_2+ACFG_SDRAM_MODE_REGISTER_VAL
	strb		r2, [r4]

	/* set extended mode register */
	ldr		r4, =PHYS_SDRAM_2+ACFG_SDRAM_EXT_MODE_REGISTER_VAL
	strb		r2, [r4]

	ldr		r1, =ACFG_NORMAL_RW_CMD
	str		r1, [r0, #ESDCTL1_ROF]
	.endm /* init_ddr */

.globl lowlevel_init
lowlevel_init:

	init_aipi
	init_clock
#ifdef CONFIG_SPL_BUILD
	init_ddr
#endif

	mov	pc, lr
