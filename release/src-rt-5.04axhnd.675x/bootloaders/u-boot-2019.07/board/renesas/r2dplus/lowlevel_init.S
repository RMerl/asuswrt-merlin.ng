/*
 * modified from SH-IPL+g (init-r0p751rlc0011rl.S)
 * Initial Register Data for R0P751RLC0011RL (SH7751R 240MHz/120MHz/60MHz)
 * Coyright (c) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
*/

#include <config.h>

#include <asm/processor.h>
#include <asm/macro.h>

	.global lowlevel_init
	.text
	.align	2

lowlevel_init:

	write32	CCR_A, CCR_D_D

	write32	MMUCR_A, MMUCR_D

	write32	BCR1_A, BCR1_D

	write16	BCR2_A, BCR2_D

	write16	BCR3_A, BCR3_D

	write32	BCR4_A, BCR4_D

	write32	WCR1_A, WCR1_D

	write32	WCR2_A, WCR2_D

	write32	WCR3_A, WCR3_D

	write16	PCR_A, PCR_D

	write16	LED_A, LED_D

	write32	MCR_A, MCR_D1

	write16	RTCNT_A, RTCNT_D

	write16	RTCOR_A, RTCOR_D

	write16	RFCR_A, RFCR_D

	write16	RTCSR_A, RTCSR_D

	write8	SDMR3_A, SDMR3_D0

	/* Wait DRAM refresh 30 times */
	mov.l	RFCR_A, r1
	mov	#30, r3
1:
	mov.w	@r1, r0
	extu.w	r0, r2
	cmp/hi	r3, r2
	bf	1b

	write32	MCR_A, MCR_D2

	write8	SDMR3_A, SDMR3_D1

	write32	IRLMASK_A, IRLMASK_D

	write32	CCR_A, CCR_D_E

	rts
	nop

	.align	2
CCR_A:		.long	CCR		/* Cache Control Register */
CCR_D_D:	.long	0x0808		/* Flush the cache, disable */
CCR_D_E:	.long	0x8000090B

FRQCR_A:	.long	FRQCR		/* FRQCR Address */
FRQCR_D:	.long	0x00000e0a	/* 03/07/15 modify */
BCR1_A:		.long	BCR1		/* BCR1 Address */
BCR1_D:		.long	0x00180008
BCR2_A:		.long	BCR2		/* BCR2 Address */
BCR2_D:		.long	0xabe8
BCR3_A:		.long	BCR3		/* BCR3 Address */
BCR3_D:		.long	0x0000
BCR4_A:		.long	BCR4		/* BCR4 Address */
BCR4_D:		.long	0x00000010
WCR1_A:		.long	WCR1		/* WCR1 Address */
WCR1_D:		.long	0x33343333
WCR2_A:		.long	WCR2		/* WCR2 Address */
WCR2_D:		.long	0xcff86fbf
WCR3_A:		.long	WCR3		/* WCR3 Address */
WCR3_D:		.long	0x07777707
LED_A:		.long	0x04000036	/* LED Address */
LED_D:		.long	0xFF		/* LED Data */
RTCNT_A:	.long	RTCNT		/* RTCNT Address */
RTCNT_D:	.word	0xA500		/* RTCNT Write Code A5h Data 00h */
.align 2
RTCOR_A:	.long	RTCOR		/* RTCOR Address */
RTCOR_D:	.word	0xA534		/* RTCOR Write Code */
.align 2
RTCSR_A:	.long	RTCSR		/* RTCSR Address */
RTCSR_D:	.word	0xA510		/* RTCSR Write Code */
.align 2
SDMR3_A:	.long	0xFF9400CC	/* SDMR3 Address */
SDMR3_D0:	.long	0x55
SDMR3_D1:	.long	0x00
MCR_A:		.long	MCR		/* MCR Address */
MCR_D1:		.long	0x081901F4	/* MRSET:'0' */
MCR_D2:		.long	0x481901F4	/* MRSET:'1' */
RFCR_A:		.long	RFCR		/* RFCR Address */
RFCR_D:		.long	0xA400		/* RFCR Write Code A4h Data 00h */
PCR_A:		.long	PCR		/* PCR Address */
PCR_D:		.long	0x0000
MMUCR_A:	.long	MMUCR		/* MMUCCR Address */
MMUCR_D:	.long	0x00000000	/* MMUCCR Data */
IRLMASK_A:	.long	0xA4000000	/* IRLMASK Address */
IRLMASK_D:	.long	0x00000000	/* IRLMASK Data */
