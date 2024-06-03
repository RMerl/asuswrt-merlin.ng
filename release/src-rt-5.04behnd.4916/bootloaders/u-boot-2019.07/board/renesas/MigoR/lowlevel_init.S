/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2008
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * Copyright (C) 2007
 * Kenati Technologies, Inc.
 *
 * board/MigoR/lowlevel_init.S
 */

#include <config.h>

#include <asm/processor.h>
#include <asm/macro.h>

/*
 * Board specific low level init code, called _very_ early in the
 * startup sequence. Relocation to SDRAM has not happened yet, no
 * stack is available, bss section has not been initialised, etc.
 *
 * (Note: As no stack is available, no subroutines can be called...).
 */

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:
	write32	CCR_A, CCR_D		! Address of Cache Control Register
					! Instruction Cache Invalidate

	write32	MMUCR_A, MMUCR_D	! Address of MMU Control Register
					! TI == TLB Invalidate bit

	write32	MSTPCR0_A, MSTPCR0_D	! Address of Power Control Register 0

	write32	MSTPCR2_A, MSTPCR2_D	! Address of Power Control Register 2

	write16	PFC_PULCR_A, PFC_PULCR_D

	write16	PFC_DRVCR_A, PFC_DRVCR_D

	write16	SBSCR_A, SBSCR_D

	write16	PSCR_A, PSCR_D

	write16	RWTCSR_A, RWTCSR_D_1	! 0xA4520004 (Watchdog Control / Status Register)
					! 0xA507 -> timer_STOP / WDT_CLK = max

	write16	RWTCNT_A, RWTCNT_D	! 0xA4520000 (Watchdog Count Register)
					! 0x5A00 -> Clear

	write16	RWTCSR_A, RWTCSR_D_2	! 0xA4520004 (Watchdog Control / Status Register)
					! 0xA504 -> timer_STOP / CLK = 500ms

	write32	DLLFRQ_A, DLLFRQ_D	! 20080115
					! 20080115

	write32	FRQCR_A, FRQCR_D	! 0xA4150000 Frequency control register
					! 20080115

	write32	CCR_A, CCR_D_2		! Address of Cache Control Register
					! ??

bsc_init:
	write32	CMNCR_A, CMNCR_D

	write32	CS0BCR_A, CS0BCR_D

	write32	CS4BCR_A, CS4BCR_D

	write32	CS5ABCR_A, CS5ABCR_D

	write32	CS5BBCR_A, CS5BBCR_D

	write32	CS6ABCR_A, CS6ABCR_D

	write32	CS0WCR_A, CS0WCR_D

	write32	CS4WCR_A, CS4WCR_D

	write32	CS5AWCR_A, CS5AWCR_D

	write32	CS5BWCR_A, CS5BWCR_D

	write32	CS6AWCR_A, CS6AWCR_D

	! SDRAM initialization
	write32	SDCR_A, SDCR_D

	write32	SDWCR_A, SDWCR_D

	write32	SDPCR_A, SDPCR_D

	write32	RTCOR_A, RTCOR_D

	write32	RTCNT_A, RTCNT_D

	write32	RTCSR_A, RTCSR_D

	write32	RFCR_A, RFCR_D

	write8	SDMR3_A, SDMR3_D

	! BL bit off (init = ON) (?!?)

	stc	sr, r0				! BL bit off(init=ON)
	mov.l	SR_MASK_D, r1
	and	r1, r0
	ldc	r0, sr

	rts
	mov	#0, r0

	.align	4

CCR_A:		.long	CCR
MMUCR_A:	.long	MMUCR
MSTPCR0_A:	.long	MSTPCR0
MSTPCR2_A:	.long	MSTPCR2
PFC_PULCR_A:	.long	PULCR
PFC_DRVCR_A:	.long	DRVCR
SBSCR_A:	.long	SBSCR
PSCR_A:		.long	PSCR
RWTCSR_A:	.long	RWTCSR
RWTCNT_A:	.long	RWTCNT
FRQCR_A:	.long	FRQCR
PLLCR_A:	.long	PLLCR
DLLFRQ_A:	.long	DLLFRQ

CCR_D:		.long	0x00000800
CCR_D_2:	.long	0x00000103
MMUCR_D:	.long	0x00000004
MSTPCR0_D:	.long	0x00001001
MSTPCR2_D:	.long	0xffffffff
PFC_PULCR_D:	.long	0x6000
PFC_DRVCR_D:	.long	0x0464
FRQCR_D:	.long	0x07033639
PLLCR_D:	.long	0x00005000
DLLFRQ_D:	.long	0x000004F6

CMNCR_A:	.long	CMNCR
CMNCR_D:	.long	0x0000001B
CS0BCR_A:	.long	CS0BCR
CS0BCR_D:	.long	0x24920400
CS4BCR_A:	.long	CS4BCR
CS4BCR_D:	.long	0x00003400
CS5ABCR_A:	.long	CS5ABCR
CS5ABCR_D:	.long	0x24920400
CS5BBCR_A:	.long	CS5BBCR
CS5BBCR_D:	.long	0x24920400
CS6ABCR_A:	.long	CS6ABCR
CS6ABCR_D:	.long	0x24920400

CS0WCR_A:	.long	CS0WCR
CS0WCR_D:	.long	0x00000380
CS4WCR_A:	.long	CS4WCR
CS4WCR_D:	.long	0x00110080
CS5AWCR_A:	.long	CS5AWCR
CS5AWCR_D:	.long	0x00000300
CS5BWCR_A:	.long	CS5BWCR
CS5BWCR_D:	.long	0x00000300
CS6AWCR_A:	.long	CS6AWCR
CS6AWCR_D:	.long	0x00000300

SDCR_A:		.long	SBSC_SDCR
SDCR_D:		.long	0x80160809
SDWCR_A:	.long	SBSC_SDWCR
SDWCR_D:	.long	0x0014450C
SDPCR_A:	.long	SBSC_SDPCR
SDPCR_D:	.long	0x00000087
RTCOR_A:	.long	SBSC_RTCOR
RTCNT_A:	.long	SBSC_RTCNT
RTCNT_D:	.long	0xA55A0012
RTCOR_D:	.long	0xA55A001C
RTCSR_A:	.long	SBSC_RTCSR
RFCR_A:		.long	SBSC_RFCR
RFCR_D:		.long	0xA55A0221
RTCSR_D:	.long	0xA55A009a
SDMR3_A:	.long	0xFE581180
SDMR3_D:	.long	0x0

SR_MASK_D:	.long	0xEFFFFF0F

	.align	2

SBSCR_D:	.word	0x0044
PSCR_D:		.word	0x0000
RWTCSR_D_1:	.word	0xA507
RWTCSR_D_2:	.word	0xA504
RWTCNT_D:	.word	0x5A00
