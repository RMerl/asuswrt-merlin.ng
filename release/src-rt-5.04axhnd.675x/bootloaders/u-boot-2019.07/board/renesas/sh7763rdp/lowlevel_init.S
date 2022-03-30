/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 * Copyright (C) 2007 Kenati Technologies, Inc.
 *
 * board/sh7763rdp/lowlevel_init.S
 */

#include <config.h>

#include <asm/processor.h>
#include <asm/macro.h>

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	write32	WDTCSR_A, WDTCSR_D	/* Watchdog Control / Status Register */

	write32	WDTST_A, WDTST_D	/* Watchdog Stop Time Register */

	write32	WDTBST_A, WDTBST_D	/*
					 * 0xFFCC0008
					 * Watchdog Base Stop Time Register
					 */

	write32	CCR_A, CCR_CACHE_ICI_D	/* Address of Cache Control Register */
					/* Instruction Cache Invalidate */

	write32	MMUCR_A, MMU_CONTROL_TI_D	/* MMU Control Register */
						/* TI == TLB Invalidate bit */

	write32	MSTPCR0_A, MSTPCR0_D	/* Address of Power Control Register 0 */

	write32	MSTPCR1_A, MSTPCR1_D	/* Address of Power Control Register 1 */

	write32	RAMCR_A, RAMCR_D

	mov.l	MMSELR_A, r1
	mov.l	MMSELR_D, r0
	synco
	mov.l	r0, @r1

	mov.l	@r1, r2		/* execute two reads after setting MMSELR */
	mov.l	@r1, r2
	synco

	/* issue memory read */
	mov.l	DDRSD_START_A, r1	/* memory address to read*/
	mov.l	@r1, r0
	synco

	write32	MIM8_A, MIM8_D

	write32	MIMC_A, MIMC_D1

	write32	STRC_A, STRC_D

	write32	SDR4_A, SDR4_D

	write32	MIMC_A, MIMC_D2

	nop
	nop
	nop

	write32	SCR4_A, SCR4_D3

	write32	SCR4_A, SCR4_D2

	write32	SDMR02000_A, SDMR02000_D

	write32	SDMR00B08_A, SDMR00B08_D

	write32	SCR4_A, SCR4_D2

	write32	SCR4_A, SCR4_D4

	nop
	nop
	nop
	nop

	write32	SCR4_A, SCR4_D4

	nop
	nop
	nop
	nop

	write32	SDMR00308_A, SDMR00308_D

	write32	MIMC_A, MIMC_D3

	mov.l	SCR4_A, r1
	mov.l	SCR4_D1, r0
	mov.l	DELAY60_D, r3

delay_loop_60:
	mov.l	r0, @r1
	dt	r3
	bf	delay_loop_60
	nop

	write32	CCR_A, CCR_CACHE_D_2	/* Address of Cache Control Register */

bsc_init:
	write32	BCR_A, BCR_D

	write32	CS0BCR_A, CS0BCR_D

	write32	CS1BCR_A, CS1BCR_D

	write32	CS2BCR_A, CS2BCR_D

	write32	CS4BCR_A, CS4BCR_D

	write32	CS5BCR_A, CS5BCR_D

	write32	CS6BCR_A, CS6BCR_D

	write32	CS0WCR_A, CS0WCR_D

	write32	CS1WCR_A, CS1WCR_D

	write32	CS2WCR_A, CS2WCR_D

	write32	CS4WCR_A, CS4WCR_D

	write32	CS5WCR_A, CS5WCR_D

	write32	CS6WCR_A, CS6WCR_D

	write32	CS5PCR_A, CS5PCR_D

	write32	CS6PCR_A, CS6PCR_D

	mov.l	DELAY200_D, r3

delay_loop_200:
	dt	r3
	bf	delay_loop_200
	nop

	write16	PSEL0_A, PSEL0_D

	write16	PSEL1_A, PSEL1_D

	write32	ICR0_A, ICR0_D

	stc sr, r0	/* BL bit off(init=ON) */
	mov.l	SR_MASK_D, r1
	and r1, r0
	ldc r0, sr

	rts
	nop

	.align	2

DELAY60_D:	.long	60
DELAY200_D:	.long	17800

CCR_A:		.long	0xFF00001C
MMUCR_A:	.long	0xFF000010
RAMCR_A:	.long	0xFF000074

/* Low power mode control */
MSTPCR0_A:	.long	0xFFC80030
MSTPCR1_A:	.long	0xFFC80038

/* RWBT */
WDTST_A:	.long	0xFFCC0000
WDTCSR_A:	.long	0xFFCC0004
WDTBST_A:	.long	0xFFCC0008

/* BSC */
MMSELR_A:	.long	0xFE600020
BCR_A:		.long	0xFF801000
CS0BCR_A:	.long	0xFF802000
CS1BCR_A:	.long	0xFF802010
CS2BCR_A:	.long	0xFF802020
CS4BCR_A:	.long	0xFF802040
CS5BCR_A:	.long	0xFF802050
CS6BCR_A:	.long	0xFF802060
CS0WCR_A:	.long	0xFF802008
CS1WCR_A:	.long	0xFF802018
CS2WCR_A:	.long	0xFF802028
CS4WCR_A:	.long	0xFF802048
CS5WCR_A:	.long	0xFF802058
CS6WCR_A:	.long	0xFF802068
CS5PCR_A:	.long	0xFF802070
CS6PCR_A:	.long	0xFF802080
DDRSD_START_A:	.long	0xAC000000

/* INTC */
ICR0_A:		.long	0xFFD00000

/* DDR I/F */
MIM8_A:		.long	0xFE800008
MIMC_A:		.long	0xFE80000C
SCR4_A:		.long	0xFE800014
STRC_A:		.long	0xFE80001C
SDR4_A:		.long	0xFE800034
SDMR00308_A:	.long	0xFE900308
SDMR00B08_A:	.long	0xFE900B08
SDMR02000_A:	.long	0xFE902000

/* GPIO */
PSEL0_A:	.long	0xFFEF0070
PSEL1_A:	.long	0xFFEF0072

CCR_CACHE_ICI_D:.long	0x00000800
CCR_CACHE_D_2:	.long	0x00000103
MMU_CONTROL_TI_D:.long	0x00000004
RAMCR_D:	.long	0x00000200
MSTPCR0_D:	.long	0x00000000
MSTPCR1_D:	.long	0x00000000

MMSELR_D:	.long	0xa5a50000
BCR_D:		.long	0x00000000
CS0BCR_D:	.long	0x77777770
CS1BCR_D:	.long	0x77777670
CS2BCR_D:	.long	0x77777670
CS4BCR_D:	.long	0x77777670
CS5BCR_D:	.long	0x77777670
CS6BCR_D:	.long	0x77777670
CS0WCR_D:	.long	0x7777770F
CS1WCR_D:	.long	0x22000002
CS2WCR_D:	.long	0x7777770F
CS4WCR_D:	.long	0x7777770F
CS5WCR_D:	.long	0x7777770F
CS6WCR_D:	.long	0x7777770F
CS5PCR_D:	.long	0x77000000
CS6PCR_D:	.long	0x77000000
ICR0_D:		.long	0x00E00000
MIM8_D:		.long	0x00000000
MIMC_D1:	.long	0x01d10008
MIMC_D2:	.long	0x01d10009
MIMC_D3:	.long	0x01d10209
SCR4_D1:	.long	0x00000001
SCR4_D2:	.long	0x00000002
SCR4_D3:	.long	0x00000003
SCR4_D4:	.long	0x00000004
STRC_D:		.long	0x000f3980
SDR4_D:		.long	0x00000300
SDMR00308_D:	.long	0x00000000
SDMR00B08_D:	.long	0x00000000
SDMR02000_D:	.long	0x00000000
PSEL0_D:	.word	0x00000001
PSEL1_D:	.word	0x00000244
SR_MASK_D:	.long	0xEFFFFF0F
WDTST_D:	.long	0x5A000FFF
WDTCSR_D:	.long	0xA5000000
WDTBST_D:	.long	0x55000000
