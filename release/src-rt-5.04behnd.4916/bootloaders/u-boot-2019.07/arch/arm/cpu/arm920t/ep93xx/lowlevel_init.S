/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Low-level initialization for EP93xx
 *
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 * Copyright (C) 2013
 * Sergey Kostanabev <sergey.kostanbaev <at> fairwaves.ru>
 *
 * Copyright (C) 2006 Dominic Rath <Dominic.Rath@gmx.de>
 * Copyright (C) 2006 Cirrus Logic Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <config.h>
#include <asm/arch-ep93xx/ep93xx.h>

/*
/* Configure the SDRAM based on the supplied settings.
 *
 * Input:	r0 - SDRAM DEVCFG register
 *		r2 - configuration for SDRAM chips
 * Output:	none
 * Modifies:	r3, r4
 */
ep93xx_sdram_config:
	/* Program the SDRAM device configuration register. */
	ldr	r3, =SDRAM_BASE
#ifdef CONFIG_EDB93XX_SDCS0
	str	r0, [r3, #SDRAM_OFF_DEVCFG0]
#endif
#ifdef CONFIG_EDB93XX_SDCS1
	str	r0, [r3, #SDRAM_OFF_DEVCFG1]
#endif
#ifdef CONFIG_EDB93XX_SDCS2
	str	r0, [r3, #SDRAM_OFF_DEVCFG2]
#endif
#ifdef CONFIG_EDB93XX_SDCS3
	str	r0, [r3, #SDRAM_OFF_DEVCFG3]
#endif

	/* Set the Initialize and MRS bits (issue continuous NOP commands
	 * (INIT & MRS set))
	 */
	ldr	r4, =(EP93XX_SDRAMCTRL_GLOBALCFG_INIT | \
			EP93XX_SDRAMCTRL_GLOBALCFG_MRS | \
			EP93XX_SDRAMCTRL_GLOBALCFG_CKE)
	str	r4, [r3, #SDRAM_OFF_GLCONFIG]

	/* Delay for 200us. */
	mov	r4, #0x3000
delay1:
	subs	r4, r4, #1
	bne	delay1

	/* Clear the MRS bit to issue a precharge all. */
	ldr	r4, =(EP93XX_SDRAMCTRL_GLOBALCFG_INIT | \
			EP93XX_SDRAMCTRL_GLOBALCFG_CKE)
	str	r4, [r3, #SDRAM_OFF_GLCONFIG]

	/* Temporarily set the refresh timer to 0x10. Make it really low so
	 * that refresh cycles are generated.
	 */
	ldr	r4, =0x10
	str	r4, [r3, #SDRAM_OFF_REFRSHTIMR]

	/* Delay for at least 80 SDRAM clock cycles. */
	mov	r4, #80
delay2:
	subs	r4, r4, #1
	bne	delay2

	/* Set the refresh timer to the fastest required for any device
	 * that might be used. Set 9.6 ms refresh time.
	 */
	ldr	r4, =0x01e0
	str	r4, [r3, #SDRAM_OFF_REFRSHTIMR]

	/* Select mode register update mode. */
	ldr	r4, =(EP93XX_SDRAMCTRL_GLOBALCFG_CKE | \
			EP93XX_SDRAMCTRL_GLOBALCFG_MRS)
	str	r4, [r3, #SDRAM_OFF_GLCONFIG]

	/* Program the mode register on the SDRAM by performing fake read */
	ldr	r4, [r2]

	/* Select normal operating mode. */
	ldr	r4, =EP93XX_SDRAMCTRL_GLOBALCFG_CKE
	str	r4, [r3, #SDRAM_OFF_GLCONFIG]

	/* Return to the caller. */
	mov	pc, lr

/*
 * Test to see if the SDRAM has been configured in a usable mode.
 *
 * Input:	r0 - Test address of SDRAM
 * Output:	r0 - 0 -- Test OK, -1 -- Failed
 * Modifies:	r0-r5
 */
ep93xx_sdram_test:
	/* Load the test patterns to be written to SDRAM. */
	ldr	r1, =0xf00dface
	ldr	r2, =0xdeadbeef
	ldr	r3, =0x08675309
	ldr	r4, =0xdeafc0ed

	/* Store the test patterns to SDRAM. */
	stmia	r0, {r1-r4}

	/* Load the test patterns from SDRAM one at a time and compare them
	 * to the actual pattern.
	 */
	ldr	r5, [r0]
	cmp	r5, r1
	ldreq	r5, [r0, #0x0004]
	cmpeq	r5, r2
	ldreq	r5, [r0, #0x0008]
	cmpeq	r5, r3
	ldreq	r5, [r0, #0x000c]
	cmpeq	r5, r4

	/* Return -1 if a mismatch was encountered, 0 otherwise. */
	mvnne	r0, #0xffffffff
	moveq	r0, #0x00000000

	/* Return to the caller. */
	mov	pc, lr

/*
 * Determine the size of the SDRAM. Use data=address for the scan.
 *
 * Input:	r0 - Start SDRAM address
 * Return:	r0 - Single block size
 *		r1 - Valid block mask
 *		r2 - Total block count
 * Modifies:	r0-r5
 */
ep93xx_sdram_size:
	/* Store zero at offset zero. */
	str	r0, [r0]

	/* Start checking for an alias at 1MB into SDRAM. */
	ldr	r1, =0x00100000

	/* Store the offset at the current offset. */
check_block_size:
	str	r1, [r0, r1]

	/* Read back from zero. */
	ldr	r2, [r0]

	/* Stop searching of an alias was found. */
	cmp	r1, r2
	beq	found_block_size

	/* Advance to the next power of two boundary. */
	mov	r1, r1, lsl #1

	/* Loop back if the size has not reached 256MB. */
	cmp	r1, #0x10000000
	bne	check_block_size

	/* A full 256MB of memory was found, so return it now. */
	ldr	r0, =0x10000000
	ldr	r1, =0x00000000
	ldr	r2, =0x00000001
	mov	pc, lr

	/* An alias was found. See if the first block is 128MB in size. */
found_block_size:
	cmp	r1, #0x08000000

	/* The first block is 128MB, so there is no further memory. Return it
	 * now.
	 */
	ldreq	r0, =0x08000000
	ldreq	r1, =0x00000000
	ldreq	r2, =0x00000001
	moveq	pc, lr

	/* Save the block size, set the block address bits to zero, and
	 * initialize the block count to one.
	 */
	mov	r3, r1
	ldr	r4, =0x00000000
	ldr	r5, =0x00000001

	/* Look for additional blocks of memory by searching for non-aliases. */
find_blocks:
	/* Store zero back to address zero. It may be overwritten. */
	str	r0, [r0]

	/* Advance to the next power of two boundary. */
	mov	r1, r1, lsl #1

	/* Store the offset at the current offset. */
	str	r1, [r0, r1]

	/* Read back from zero. */
	ldr	r2, [r0]

	/* See if a non-alias was found. */
	cmp	r1, r2

	/* If a non-alias was found, then or in the block address bit and
	 * multiply the block count by two (since there are two unique
	 * blocks, one with this bit zero and one with it one).
	 */
	orrne	r4, r4, r1
	movne	r5, r5, lsl #1

	/* Continue searching if there are more address bits to check. */
	cmp	r1, #0x08000000
	bne	find_blocks

	/* Return the block size, address mask, and count. */
	mov	r0, r3
	mov	r1, r4
	mov	r2, r5

	/* Return to the caller. */
	mov	pc, lr


.globl lowlevel_init
lowlevel_init:

	mov	r6, lr

	/* Make sure caches are off and invalidated. */
	ldr	r0, =0x00000000
	mcr	p15, 0, r0, c1, c0, 0
	nop
	nop
	nop
	nop
	nop

	/* Turn off the green LED and turn on the red LED. If the red LED
	 * is left on for too long, the external reset circuit described
	 * by application note AN258 will cause the system to reset.
	 */
	ldr	r1, =EP93XX_LED_DATA
	ldr	r0, [r1]
	bic	r0, r0, #EP93XX_LED_GREEN_ON
	orr	r0, r0, #EP93XX_LED_RED_ON
	str	r0, [r1]

	/* Undo the silly static memory controller programming performed
	 * by the boot rom.
	 */
	ldr	r0, =SMC_BASE

	/* Set WST1 and WST2 to 31 HCLK cycles (slowest access) */
	ldr	r1, =0x0000fbe0

	/* Reset EP93XX_OFF_SMCBCR0 */
	ldr	r2, [r0]
	orr	r2, r2, r1
	str	r2, [r0]

	ldr	r2, [r0, #EP93XX_OFF_SMCBCR1]
	orr	r2, r2, r1
	str	r2, [r0, #EP93XX_OFF_SMCBCR1]

	ldr	r2, [r0, #EP93XX_OFF_SMCBCR2]
	orr	r2, r2, r1
	str	r2, [r0, #EP93XX_OFF_SMCBCR2]

	ldr	r2, [r0, #EP93XX_OFF_SMCBCR3]
	orr	r2, r2, r1
	str	r2, [r0, #EP93XX_OFF_SMCBCR3]

	ldr	r2, [r0, #EP93XX_OFF_SMCBCR6]
	orr	r2, r2, r1
	str	r2, [r0, #EP93XX_OFF_SMCBCR6]

	ldr	r2, [r0, #EP93XX_OFF_SMCBCR7]
	orr	r2, r2, r1
	str	r2, [r0, #EP93XX_OFF_SMCBCR7]

	/* Set the PLL1 and processor clock. */
	ldr	r0, =SYSCON_BASE
#ifdef CONFIG_EDB9301
	/* 332MHz, giving a 166MHz processor clock. */
	ldr	r1, = 0x02b49907
#else

#ifdef CONFIG_EDB93XX_INDUSTRIAL
	/* 384MHz, giving a 196MHz processor clock. */
	ldr	r1, =0x02a4bb38
#else
	/* 400MHz, giving a 200MHz processor clock. */
	ldr	r1, =0x02a4e39e
#endif
#endif
	str	r1, [r0, #SYSCON_OFF_CLKSET1]

	nop
	nop
	nop
	nop
	nop

	/* Need to make sure that SDRAM is configured correctly before
	 * coping the code into it.
	 */

#ifdef CONFIG_EDB93XX_SDCS0
	mov	r11, #SDRAM_DEVCFG0_BASE
#endif
#ifdef CONFIG_EDB93XX_SDCS1
	mov	r11, #SDRAM_DEVCFG1_BASE
#endif
#ifdef CONFIG_EDB93XX_SDCS2
	mov	r11, #SDRAM_DEVCFG2_BASE
#endif
#ifdef CONFIG_EDB93XX_SDCS3
	ldr	r0, =SYSCON_BASE
	ldr	r0, [r0, #SYSCON_OFF_SYSCFG]
	ands	r0, r0, #SYSCON_SYSCFG_LASDO
	moveq	r11, #SDRAM_DEVCFG3_ASD0_BASE
	movne	r11, #SDRAM_DEVCFG3_ASD1_BASE
#endif
	/* See Table 13-5 in EP93xx datasheet for more info about DRAM
	 * register mapping */

	/* Try a 32-bit wide configuration of SDRAM. */
	ldr	r0, =(EP93XX_SDRAMCTRL_DEVCFG_BANKCOUNT | \
			EP93XX_SDRAMCTRL_DEVCFG_SROMLL | \
			EP93XX_SDRAMCTRL_DEVCFG_CASLAT_2 | \
			EP93XX_SDRAMCTRL_DEVCFG_RASTOCAS_2)

	/* Set burst count: 4 and CAS: 2
	 * Burst mode [A11:A10]; CAS [A16:A14]
	 */
	orr	r2, r11, #0x00008800
	bl	ep93xx_sdram_config

	/* Test the SDRAM. */
	mov	r0, r11
	bl	ep93xx_sdram_test
	cmp	r0, #0x00000000
	beq	ep93xx_sdram_done

	/* Try a 16-bit wide configuration of SDRAM. */
	ldr	r0, =(EP93XX_SDRAMCTRL_DEVCFG_BANKCOUNT | \
			EP93XX_SDRAMCTRL_DEVCFG_SROMLL | \
			EP93XX_SDRAMCTRL_DEVCFG_CASLAT_2 | \
			EP93XX_SDRAMCTRL_DEVCFG_RASTOCAS_2 | \
			EP93XX_SDRAMCTRL_DEVCFG_EXTBUSWIDTH)

	/* Set burst count: 8, CAS: 2, sequential burst
	 * Accoring to Table 13-3 for 16bit operations mapping must be shifted.
	 * Burst mode [A10:A9]; CAS [A15:A13]
	 */
	orr	r2, r11, #0x00004600
	bl	ep93xx_sdram_config

	/* Test the SDRAM. */
	mov	r0, r11
	bl	ep93xx_sdram_test
	cmp	r0, #0x00000000
	beq	ep93xx_sdram_done

	/* Turn off the red LED. */
	ldr	r0, =EP93XX_LED_DATA
	ldr	r1, [r0]
	bic	r1, r1, #EP93XX_LED_RED_ON
	str	r1, [r0]

	/* There is no SDRAM so flash the green LED. */
flash_green:
	orr	r1, r1, #EP93XX_LED_GREEN_ON
	str	r1, [r0]
	ldr	r2, =0x00010000
flash_green_delay_1:
	subs	r2, r2, #1
	bne	flash_green_delay_1
	bic	r1, r1, #EP93XX_LED_GREEN_ON
	str	r1, [r0]
	ldr	r2, =0x00010000
flash_green_delay_2:
	subs	r2, r2, #1
	bne	flash_green_delay_2
	orr	r1, r1, #EP93XX_LED_GREEN_ON
	str	r1, [r0]
	ldr	r2, =0x00010000
flash_green_delay_3:
	subs	r2, r2, #1
	bne	flash_green_delay_3
	bic	r1, r1, #EP93XX_LED_GREEN_ON
	str	r1, [r0]
	ldr	r2, =0x00050000
flash_green_delay_4:
	subs	r2, r2, #1
	bne	flash_green_delay_4
	b	flash_green


ep93xx_sdram_done:
	ldr	r1, =EP93XX_LED_DATA
	ldr	r0, [r1]
	bic	r0, r0, #EP93XX_LED_RED_ON
	str	r0, [r1]

	/* Determine the size of the SDRAM. */
	mov	r0, r11
	bl	ep93xx_sdram_size

	/* Save the SDRAM characteristics. */
	mov	r8, r0
	mov	r9, r1
	mov	r10, r2

	/* Compute total memory size into r1 */
	mul	r1, r8, r10
#ifdef CONFIG_EDB93XX_SDCS0
	ldr	r2, [r0, #SDRAM_OFF_DEVCFG0]
#endif
#ifdef CONFIG_EDB93XX_SDCS1
	ldr	r2, [r0, #SDRAM_OFF_DEVCFG1]
#endif
#ifdef CONFIG_EDB93XX_SDCS2
	ldr	r2, [r0, #SDRAM_OFF_DEVCFG2]
#endif
#ifdef CONFIG_EDB93XX_SDCS3
	ldr	r2, [r0, #SDRAM_OFF_DEVCFG3]
#endif

	/* Consider small DRAM size as:
	 * < 32Mb for 32bit bus
	 * < 64Mb for 16bit bus
	 */
	tst	r2, #EP93XX_SDRAMCTRL_DEVCFG_EXTBUSWIDTH
	moveq	r1, r1, lsr #1
	cmp	r1, #0x02000000

#if defined(CONFIG_EDB9301)
	/* Set refresh counter to 20ms for small DRAM size, otherwise 9.6ms */
	movlt	r1, #0x03f0
	movge	r1, #0x01e0
#else
	/* Set refresh counter to 30.7ms for small DRAM size, otherwise 15ms */
	movlt	r1, #0x0600
	movge	r1, #0x2f0
#endif
	str	r1, [r0, #SDRAM_OFF_REFRSHTIMR]

	/* Save the memory configuration information. */
	orr	r0, r11, #UBOOT_MEMORYCNF_BANK_SIZE
	stmia	r0, {r8-r11}

	mov	lr, r6
	mov	pc, lr
