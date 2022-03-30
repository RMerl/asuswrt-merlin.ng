/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#include <config.h>

/*
 * platform specific initializations are already done in Xloader
 * Initializations already done include
 * DDR, PLLs, IP's clock enable and reset release etc
 */
.globl lowlevel_init
lowlevel_init:
	mov	pc, lr

/* void setfreq(unsigned int device, unsigned int frequency) */
.global setfreq
setfreq:
	stmfd 	sp!,{r14}
	stmfd 	sp!,{r0-r12}

	mov  	r8,sp
	ldr 	sp,SRAM_STACK_V

	/* Saving the function arguements for later use */
	mov  	r4,r0
	mov  	r5,r1

	/* Putting DDR into self refresh */
	ldr 	r0,DDR_07_V
	ldr	r1,[r0]
	ldr	r2,DDR_ACTIVE_V
	bic	r1, r1, r2
	str	r1,[r0]
	ldr 	r0,DDR_57_V
	ldr	r1,[r0]
	ldr	r2,CYCLES_MASK_V
	bic	r1, r1, r2
	ldr	r2,REFRESH_CYCLES_V
	orr	r1, r1, r2, lsl #16
	str	r1,[r0]
	ldr 	r0,DDR_07_V
	ldr	r1,[r0]
	ldr	r2,SREFRESH_MASK_V
	orr	r1, r1, r2
	str	r1,[r0]

	/* flush pipeline */
	b	flush
	.align 5
flush:
	/* Delay to ensure self refresh mode */
	ldr	r0,SREFRESH_DELAY_V
delay:
	sub	r0,r0,#1
	cmp	r0,#0
	bne	delay

	/* Putting system in slow mode */
	ldr	r0,SCCTRL_V
	mov	r1,#2
	str	r1,[r0]

	/* Changing PLL(1/2) frequency */
	mov	r0,r4
	mov	r1,r5

	cmp	r4,#0
	beq	pll1_freq

	/* Change PLL2 (DDR frequency) */
	ldr	r6,PLL2_FREQ_V
	ldr	r7,PLL2_CNTL_V
	b	pll2_freq

pll1_freq:
	/* Change PLL1 (CPU frequency) */
	ldr	r6,PLL1_FREQ_V
	ldr	r7,PLL1_CNTL_V

pll2_freq:
	mov	r0,r6
	ldr	r1,[r0]
	ldr	r2,PLLFREQ_MASK_V
	bic	r1,r1,r2
	mov	r2,r5,lsr#1
	orr	r1,r1,r2,lsl#24
	str	r1,[r0]

	mov	r0,r7
	ldr	r1,P1C0A_V
	str	r1,[r0]
	ldr	r1,P1C0E_V
	str	r1,[r0]
	ldr	r1,P1C06_V
	str	r1,[r0]
	ldr	r1,P1C0E_V
	str	r1,[r0]

lock:
	ldr	r1,[r0]
	and	r1,r1,#1
	cmp	r1,#0
	beq	lock

	/* Putting system back to normal mode */
	ldr	r0,SCCTRL_V
	mov	r1,#4
	str	r1,[r0]

	/* Putting DDR back to normal */
	ldr	r0,DDR_07_V
	ldr	r1,[R0]
	ldr	r2,SREFRESH_MASK_V
	bic	r1, r1, r2
	str	r1,[r0]
	ldr	r2,DDR_ACTIVE_V
	orr	r1, r1, r2
	str	r1,[r0]

	/* Delay to ensure self refresh mode */
	ldr	r0,SREFRESH_DELAY_V
1:
	sub	r0,r0,#1
	cmp	r0,#0
	bne	1b

	mov	sp,r8
	/* Resuming back to code */
	ldmia	sp!,{r0-r12}
	ldmia	sp!,{pc}

SCCTRL_V:
	.word 0xfca00000
PLL1_FREQ_V:
	.word 0xfca8000C
PLL1_CNTL_V:
	.word 0xfca80008
PLL2_FREQ_V:
	.word 0xfca80018
PLL2_CNTL_V:
	.word 0xfca80014
PLLFREQ_MASK_V:
	.word 0xff000000
P1C0A_V:
	.word 0x1C0A
P1C0E_V:
	.word 0x1C0E
P1C06_V:
	.word 0x1C06

SREFRESH_DELAY_V:
	.word 0x9999
SRAM_STACK_V:
	.word 0xD2800600
DDR_07_V:
	.word 0xfc60001c
DDR_ACTIVE_V:
	.word 0x01000000
DDR_57_V:
	.word 0xfc6000e4
CYCLES_MASK_V:
	.word 0xffff0000
REFRESH_CYCLES_V:
	.word 0xf0f0
SREFRESH_MASK_V:
	.word 0x00010000

.global setfreq_sz
setfreq_sz:
	.word setfreq_sz - setfreq
