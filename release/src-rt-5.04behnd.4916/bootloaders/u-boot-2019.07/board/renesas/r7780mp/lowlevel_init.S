/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007,2008 Nobuhiro Iwamatsu
 *
 * u-boot/board/r7780mp/lowlevel_init.S
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

	write32	CCR_A, CCR_D		/* Address of Cache Control Register */
					/* Instruction Cache Invalidate */

	write32	FRQCR_A, FRQCR_D	/* Frequency control register */

	/* pin_multi_setting */
	write32	BBG_PMMR_A, BBG_PMMR_D_PMSR1

	write32	BBG_PMSR1_A, BBG_PMSR1_D

	write32	BBG_PMMR_A, BBG_PMMR_D_PMSR2

	write32	BBG_PMSR2_A, BBG_PMSR2_D

	write32	BBG_PMMR_A, BBG_PMMR_D_PMSR3

	write32	BBG_PMSR3_A, BBG_PMSR3_D

	write32	BBG_PMMR_A, BBG_PMMR_D_PMSR4

	write32	BBG_PMSR4_A, BBG_PMSR4_D

	write32	BBG_PMMR_A, BBG_PMMR_D_PMSRG

	write32	BBG_PMSRG_A, BBG_PMSRG_D

	/* cpg_setting */
	write32	FRQCR_A, FRQCR_D

	write32	DLLCSR_A, DLLCSR_D

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	/* wait 200us */
	mov.l	REPEAT0_R3, r3
	mov	#0, r2
repeat0:
	add	#1, r2
	cmp/hs	r3, r2
	bf	repeat0
	nop

	/* bsc_setting */
	write32	MMSELR_A, MMSELR_D

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

	/* ddr_setting */
	/* wait 200us */
	mov.l	REPEAT0_R3, r3
	mov	#0, r2
repeat1:
	add	#1, r2
	cmp/hs	r3, r2
	bf	repeat1
	nop

	mov.l	MIM_U_A, r0
	mov.l	MIM_U_D, r1
	synco
	mov.l	r1, @r0
	synco

	mov.l	MIM_L_A, r0
	mov.l	MIM_L_D0, r1
	synco
	mov.l	r1, @r0
	synco

	mov.l	STR_L_A, r0
	mov.l	STR_L_D, r1
	synco
	mov.l	r1, @r0
	synco

	mov.l	SDR_L_A, r0
	mov.l	SDR_L_D, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop
	nop

	mov.l	SCR_L_A, r0
	mov.l	SCR_L_D0, r1
	synco
	mov.l	r1, @r0
	synco

	mov.l	SCR_L_A, r0
	mov.l	SCR_L_D1, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop

	mov.l	EMRS_A, r0
	mov.l	EMRS_D, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop

	mov.l	MRS1_A, r0
	mov.l	MRS1_D, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop

	mov.l	SCR_L_A, r0
	mov.l	SCR_L_D2, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop

	mov.l	SCR_L_A, r0
	mov.l	SCR_L_D3, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop

	mov.l	SCR_L_A, r0
	mov.l	SCR_L_D4, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop

	mov.l	MRS2_A, r0
	mov.l	MRS2_D, r1
	synco
	mov.l	r1, @r0
	synco

	nop
	nop
	nop

	mov.l	SCR_L_A, r0
	mov.l	SCR_L_D5, r1
	synco
	mov.l	r1, @r0
	synco

	/* wait 200us */
	mov.l	REPEAT0_R1, r3
	mov	#0, r2
repeat2:
	add	#1, r2
	cmp/hs	r3, r2
	bf	repeat2

	synco

	mov.l	MIM_L_A, r0
	mov.l	MIM_L_D1, r1
	synco
	mov.l	r1, @r0
	synco

	rts
	nop
	.align	4

RWTCSR_D_1:		.word	0xA507
RWTCSR_D_2:		.word	0xA507
RWTCNT_D:		.word	0x5A00
	.align	2

BBG_PMMR_A:		.long	0xFF800010
BBG_PMSR1_A:		.long	0xFF800014
BBG_PMSR2_A:		.long	0xFF800018
BBG_PMSR3_A:		.long	0xFF80001C
BBG_PMSR4_A:		.long	0xFF800020
BBG_PMSRG_A:		.long	0xFF800024

BBG_PMMR_D_PMSR1:	.long	0xffffbffd
BBG_PMSR1_D:		.long	0x00004002
BBG_PMMR_D_PMSR2:	.long	0xfc21a7ff
BBG_PMSR2_D:		.long	0x03de5800
BBG_PMMR_D_PMSR3:	.long	0xfffffff8
BBG_PMSR3_D:		.long	0x00000007
BBG_PMMR_D_PMSR4:	.long	0xdffdfff9
BBG_PMSR4_D:		.long	0x20020006
BBG_PMMR_D_PMSRG:	.long	0xffffffff
BBG_PMSRG_D:		.long	0x00000000

FRQCR_A:		.long	FRQCR
DLLCSR_A:		.long	0xffc40010
FRQCR_D:		.long	0x40233035
DLLCSR_D:		.long	0x00000000

/* for DDR-SDRAM */
MIM_U_A:		.long	MIM_1
MIM_L_A:		.long	MIM_2
SCR_U_A:		.long	SCR_1
SCR_L_A:		.long	SCR_2
STR_U_A:		.long	STR_1
STR_L_A:		.long	STR_2
SDR_U_A:		.long	SDR_1
SDR_L_A:		.long	SDR_2

EMRS_A:			.long	0xFEC02000
MRS1_A:			.long	0xFEC00B08
MRS2_A:			.long	0xFEC00308

MIM_U_D:		.long	0x00004000
MIM_L_D0:		.long	0x03e80009
MIM_L_D1:		.long	0x03e80209
SCR_L_D0:		.long	0x3
SCR_L_D1:		.long	0x2
SCR_L_D2:		.long	0x2
SCR_L_D3:		.long	0x4
SCR_L_D4:		.long	0x4
SCR_L_D5:		.long	0x0
STR_L_D:		.long	0x000f0000
SDR_L_D:		.long	0x00000400
EMRS_D:			.long	0x0
MRS1_D:			.long	0x0
MRS2_D:			.long	0x0

/* Cache Controller */
CCR_A:			.long	CCR
MMUCR_A:		.long	MMUCR
RWTCNT_A:		.long	WTCNT

CCR_D:			.long	0x0000090b
CCR_D_2:		.long	0x00000103
MMUCR_D:		.long	0x00000004
MSTPCR0_D:		.long	0x00001001
MSTPCR2_D:		.long	0xffffffff

/* local Bus State Controller */
MMSELR_A:		.long	MMSELR
BCR_A:			.long	BCR
CS0BCR_A:		.long	CS0BCR
CS1BCR_A:		.long	CS1BCR
CS2BCR_A:		.long	CS2BCR
CS4BCR_A:		.long	CS4BCR
CS5BCR_A:		.long	CS5BCR
CS6BCR_A:		.long	CS6BCR
CS0WCR_A:		.long	CS0WCR
CS1WCR_A:		.long	CS1WCR
CS2WCR_A:		.long	CS2WCR
CS4WCR_A:		.long	CS4WCR
CS5WCR_A:		.long	CS5WCR
CS6WCR_A:		.long	CS6WCR
CS5PCR_A:		.long	CS5PCR
CS6PCR_A:		.long	CS6PCR

MMSELR_D:		.long	0xA5A50003
BCR_D:			.long	0x00000000
CS0BCR_D:		.long	0x77777770
CS1BCR_D:		.long	0x77777670
CS2BCR_D:		.long	0x77777770
CS4BCR_D:		.long	0x77777770
CS5BCR_D:		.long	0x77777670
CS6BCR_D:		.long	0x77777770
CS0WCR_D:		.long	0x00020006
CS1WCR_D:		.long	0x00232304
CS2WCR_D:		.long	0x7777770F
CS4WCR_D:		.long	0x7777770F
CS5WCR_D:		.long	0x00101006
CS6WCR_D:		.long	0x77777703
CS5PCR_D:		.long	0x77000000
CS6PCR_D:		.long	0x77000000

REPEAT0_R3:		.long	0x00002000
REPEAT0_R1:		.long	0x0000200
