/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012  Renesas Solutions Corp.
 */

#include <config.h>
#include <asm/processor.h>
#include <asm/macro.h>

.macro	or32, addr, data
	mov.l \addr, r1
	mov.l \data, r0
	mov.l @r1, r2
	or    r2, r0
	mov.l r0, @r1
.endm

.macro	wait_DBCMD
	mov.l	DBWAIT_A, r0
	mov.l	@r0, r1
.endm

	.global lowlevel_init
	.section	.spiboot1.text
	.align  2

lowlevel_init:
	/*------- GPIO -------*/
	write16 PDCR_A,	PDCR_D		! SPI0
	write16 PGCR_A,	PGCR_D		! SPI0, GETHER MDIO gate(PTG1)
	write16 PJCR_A,	PJCR_D		! SCIF4
	write16 PTCR_A,	PTCR_D		! STATUS
	write16 PSEL1_A, PSEL1_D	! SPI0
	write16 PSEL2_A, PSEL2_D	! SPI0
	write16 PSEL5_A, PSEL5_D	! STATUS

	bra	exit_gpio
	nop

	.align	2

/*------- GPIO -------*/
PDCR_A:		.long	0xffec0006
PGCR_A:		.long	0xffec000c
PJCR_A:		.long	0xffec0012
PTCR_A:		.long	0xffec0026
PSEL1_A:	.long	0xffec0072
PSEL2_A:	.long	0xffec0074
PSEL5_A:	.long	0xffec007a

PDCR_D:		.long	0x0000
PGCR_D:		.long	0x0004
PJCR_D:		.long	0x0000
PTCR_D:		.long	0x0000
PSEL1_D:	.long	0x0000
PSEL2_D:	.long	0x3000
PSEL5_D:	.long	0x0ffc

	.align	2

exit_gpio:
	mov	#0, r14
	mova	2f, r0
	mov.l	PC_MASK, r1
	tst	r0, r1
	bf	2f

	bra	exit_pmb
	nop

	.align	2

/* If CPU runs on SDRAM (PC=0x5???????) or not. */
PC_MASK:	.long	0x20000000

2:
	mov	#1, r14

	mov.l	EXPEVT_A, r0
	mov.l	@r0, r0
	mov.l	EXPEVT_POWER_ON_RESET, r1
	cmp/eq	r0, r1
	bt	1f

	/*
	 * If EXPEVT value is manual reset or tlb multipul-hit,
	 * initialization of DDR3IF is not necessary.
	 */
	bra	exit_ddr
	nop

1:
	/*------- Reset -------*/
	write32 MRSTCR0_A, MRSTCR0_D
	write32 MRSTCR1_A, MRSTCR1_D

	/* For Core Reset */
	mov.l	DBACEN_A, r0
	mov.l	@r0, r0
	cmp/eq	#0, r0
	bt	3f

	/*
	 * If DBACEN == 1(DBSC was already enabled), we have to avoid the
	 * initialization of DDR3-SDRAM.
	 */
	bra	exit_ddr
	nop

3:
	/*------- DDR3IF -------*/
	/* oscillation stabilization time */
	wait_timer	WAIT_OSC_TIME

	/* step 3 */
	write32 DBCMD_A, DBCMD_RSTL_VAL
	wait_timer	WAIT_30US

	/* step 4 */
	write32 DBCMD_A, DBCMD_PDEN_VAL

	/* step 5 */
	write32 DBKIND_A, DBKIND_D

	/* step 6 */
	write32 DBCONF_A, DBCONF_D
	write32 DBTR0_A, DBTR0_D
	write32 DBTR1_A, DBTR1_D
	write32 DBTR2_A, DBTR2_D
	write32 DBTR3_A, DBTR3_D
	write32 DBTR4_A, DBTR4_D
	write32 DBTR5_A, DBTR5_D
	write32 DBTR6_A, DBTR6_D
	write32 DBTR7_A, DBTR7_D
	write32 DBTR8_A, DBTR8_D
	write32 DBTR9_A, DBTR9_D
	write32 DBTR10_A, DBTR10_D
	write32 DBTR11_A, DBTR11_D
	write32 DBTR12_A, DBTR12_D
	write32 DBTR13_A, DBTR13_D
	write32 DBTR14_A, DBTR14_D
	write32 DBTR15_A, DBTR15_D
	write32 DBTR16_A, DBTR16_D
	write32 DBTR17_A, DBTR17_D
	write32 DBTR18_A, DBTR18_D
	write32 DBTR19_A, DBTR19_D
	write32 DBRNK0_A, DBRNK0_D

	/* step 7 */
	write32 DBPDCNT3_A, DBPDCNT3_D

	/* step 8 */
	write32 DBPDCNT1_A, DBPDCNT1_D
	write32 DBPDCNT2_A, DBPDCNT2_D
	write32 DBPDLCK_A, DBPDLCK_D
	write32 DBPDRGA_A, DBPDRGA_D
	write32 DBPDRGD_A, DBPDRGD_D

	/* step 9 */
	wait_timer	WAIT_30US

	/* step 10 */
	write32 DBPDCNT0_A, DBPDCNT0_D

	/* step 11 */
	wait_timer	WAIT_30US
	wait_timer	WAIT_30US

	/* step 12 */
	write32 DBCMD_A, DBCMD_WAIT_VAL
	wait_DBCMD

	/* step 13 */
	write32 DBCMD_A, DBCMD_RSTH_VAL
	wait_DBCMD

	/* step 14 */
	write32 DBCMD_A, DBCMD_WAIT_VAL
	write32 DBCMD_A, DBCMD_WAIT_VAL
	write32 DBCMD_A, DBCMD_WAIT_VAL
	write32 DBCMD_A, DBCMD_WAIT_VAL

	/* step 15 */
	write32 DBCMD_A, DBCMD_PDXT_VAL

	/* step 16 */
	write32 DBCMD_A, DBCMD_MRS2_VAL

	/* step 17 */
	write32 DBCMD_A, DBCMD_MRS3_VAL

	/* step 18 */
	write32 DBCMD_A, DBCMD_MRS1_VAL

	/* step 19 */
	write32 DBCMD_A, DBCMD_MRS0_VAL

	/* step 20 */
	write32 DBCMD_A, DBCMD_ZQCL_VAL

	write32 DBCMD_A, DBCMD_REF_VAL
	write32 DBCMD_A, DBCMD_REF_VAL
	wait_DBCMD

	/* step 21 */
	write32 DBADJ0_A, DBADJ0_D
	write32 DBADJ1_A, DBADJ1_D
	write32 DBADJ2_A, DBADJ2_D

	/* step 22 */
	write32 DBRFCNF0_A, DBRFCNF0_D
	write32 DBRFCNF1_A, DBRFCNF1_D
	write32 DBRFCNF2_A, DBRFCNF2_D

	/* step 23 */
	write32 DBCALCNF_A, DBCALCNF_D

	/* step 24 */
	write32 DBRFEN_A, DBRFEN_D
	write32 DBCMD_A, DBCMD_SRXT_VAL

	/* step 25 */
	write32 DBACEN_A, DBACEN_D

	/* step 26 */
	wait_DBCMD

	bra	exit_ddr
	nop

	.align 2

EXPEVT_A:		.long	0xff000024
EXPEVT_POWER_ON_RESET:	.long	0x00000000

/*------- Reset -------*/
MRSTCR0_A:	.long	0xffd50030
MRSTCR0_D:	.long	0xfe1ffe7f
MRSTCR1_A:	.long	0xffd50034
MRSTCR1_D:	.long	0xfff3ffff

/*------- DDR3IF -------*/
DBCMD_A:	.long	0xfe800018
DBKIND_A:	.long	0xfe800020
DBCONF_A:	.long	0xfe800024
DBTR0_A:	.long	0xfe800040
DBTR1_A:	.long	0xfe800044
DBTR2_A:	.long	0xfe800048
DBTR3_A:	.long	0xfe800050
DBTR4_A:	.long	0xfe800054
DBTR5_A:	.long	0xfe800058
DBTR6_A:	.long	0xfe80005c
DBTR7_A:	.long	0xfe800060
DBTR8_A:	.long	0xfe800064
DBTR9_A:	.long	0xfe800068
DBTR10_A:	.long	0xfe80006c
DBTR11_A:	.long	0xfe800070
DBTR12_A:	.long	0xfe800074
DBTR13_A:	.long	0xfe800078
DBTR14_A:	.long	0xfe80007c
DBTR15_A:	.long	0xfe800080
DBTR16_A:	.long	0xfe800084
DBTR17_A:	.long	0xfe800088
DBTR18_A:	.long	0xfe80008c
DBTR19_A:	.long	0xfe800090
DBRNK0_A:	.long	0xfe800100
DBPDCNT0_A:	.long	0xfe800200
DBPDCNT1_A:	.long	0xfe800204
DBPDCNT2_A:	.long	0xfe800208
DBPDCNT3_A:	.long	0xfe80020c
DBPDLCK_A:	.long	0xfe800280
DBPDRGA_A:	.long	0xfe800290
DBPDRGD_A:	.long	0xfe8002a0
DBADJ0_A:	.long	0xfe8000c0
DBADJ1_A:	.long	0xfe8000c4
DBADJ2_A:	.long	0xfe8000c8
DBRFCNF0_A:	.long	0xfe8000e0
DBRFCNF1_A:	.long	0xfe8000e4
DBRFCNF2_A:	.long	0xfe8000e8
DBCALCNF_A:	.long	0xfe8000f4
DBRFEN_A:	.long	0xfe800014
DBACEN_A:	.long	0xfe800010
DBWAIT_A:	.long	0xfe80001c

WAIT_OSC_TIME:	.long	6000
WAIT_30US:	.long	13333

DBCMD_RSTL_VAL:	.long	0x20000000
DBCMD_PDEN_VAL:	.long	0x1000d73c
DBCMD_WAIT_VAL:	.long	0x0000d73c
DBCMD_RSTH_VAL:	.long	0x2100d73c
DBCMD_PDXT_VAL:	.long	0x110000c8
DBCMD_MRS0_VAL:	.long	0x28000930
DBCMD_MRS1_VAL:	.long	0x29000004
DBCMD_MRS2_VAL:	.long	0x2a000008
DBCMD_MRS3_VAL:	.long	0x2b000000
DBCMD_ZQCL_VAL:	.long	0x03000200
DBCMD_REF_VAL:	.long	0x0c000000
DBCMD_SRXT_VAL:	.long	0x19000000
DBKIND_D:	.long	0x00000007
DBCONF_D:	.long	0x0f030a01
DBTR0_D:	.long	0x00000007
DBTR1_D:	.long	0x00000006
DBTR2_D:	.long	0x00000000
DBTR3_D:	.long	0x00000007
DBTR4_D:	.long	0x00070007
DBTR5_D:	.long	0x0000001b
DBTR6_D:	.long	0x00000014
DBTR7_D:	.long	0x00000005
DBTR8_D:	.long	0x00000015
DBTR9_D:	.long	0x00000006
DBTR10_D:	.long	0x00000008
DBTR11_D:	.long	0x00000007
DBTR12_D:	.long	0x0000000e
DBTR13_D:	.long	0x00000056
DBTR14_D:	.long	0x00000006
DBTR15_D:	.long	0x00000004
DBTR16_D:	.long	0x00150002
DBTR17_D:	.long	0x000c0017
DBTR18_D:	.long	0x00000200
DBTR19_D:	.long	0x00000040
DBRNK0_D:	.long	0x00000001
DBPDCNT0_D:	.long	0x00000001
DBPDCNT1_D:	.long	0x00000001
DBPDCNT2_D:	.long	0x00000000
DBPDCNT3_D:	.long	0x00004010
DBPDLCK_D:	.long	0x0000a55a
DBPDRGA_D:	.long	0x00000028
DBPDRGD_D:	.long	0x00017100

DBADJ0_D:	.long	0x00000000
DBADJ1_D:	.long	0x00000000
DBADJ2_D:	.long	0x18061806
DBRFCNF0_D:	.long	0x000001ff
DBRFCNF1_D:	.long	0x08001000
DBRFCNF2_D:	.long	0x00000000
DBCALCNF_D:	.long	0x0000ffff
DBRFEN_D:	.long	0x00000001
DBACEN_D:	.long	0x00000001

	.align 2
exit_ddr:
#if defined(CONFIG_SH_32BIT)
	/*------- set PMB -------*/
	write32	PASCR_A,	PASCR_29BIT_D
	write32	MMUCR_A,	MMUCR_D

	/*****************************************************************
	 * ent	virt		phys		v	sz	c	wt
	 * 0	0xa0000000	0x00000000	1	128M	0	1
	 * 1	0xa8000000	0x48000000	1	128M	0	1
	 * 5	0x88000000	0x48000000	1	128M	1	1
	 */
	write32	PMB_ADDR_SPIBOOT_A,	PMB_ADDR_SPIBOOT_D
	write32	PMB_DATA_SPIBOOT_A,	PMB_DATA_SPIBOOT_D
	write32	PMB_ADDR_DDR_C1_A,	PMB_ADDR_DDR_C1_D
	write32	PMB_DATA_DDR_C1_A,	PMB_DATA_DDR_C1_D
	write32	PMB_ADDR_DDR_N1_A,	PMB_ADDR_DDR_N1_D
	write32	PMB_DATA_DDR_N1_A,	PMB_DATA_DDR_N1_D

	write32	PMB_ADDR_ENTRY2,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY3,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY4,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY6,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY7,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY8,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY9,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY10,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY11,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY12,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY13,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY14,	PMB_ADDR_NOT_USE_D
	write32	PMB_ADDR_ENTRY15,	PMB_ADDR_NOT_USE_D

	write32	PASCR_A,	PASCR_INIT
	mov.l	DUMMY_ADDR, r0
	icbi	@r0
#endif	/* if defined(CONFIG_SH_32BIT) */

exit_pmb:
	/* CPU is running on ILRAM? */
	mov	r14, r0
	tst	#1, r0
	bt	1f

	mov.l	_stack_ilram, r15
	mov.l	_spiboot_main, r0
100:	bsrf	r0
	nop

	.align	2
_spiboot_main:	.long	(spiboot_main - (100b + 4))
_stack_ilram:	.long	0xe5204000

1:
	write32	CCR_A,	CCR_D

	rts
	 nop

	.align 2

#if defined(CONFIG_SH_32BIT)
/*------- set PMB -------*/
PMB_ADDR_SPIBOOT_A:	.long	PMB_ADDR_BASE(0)
PMB_ADDR_DDR_N1_A:	.long	PMB_ADDR_BASE(1)
PMB_ADDR_DDR_C1_A:	.long	PMB_ADDR_BASE(5)
PMB_ADDR_ENTRY2:	.long	PMB_ADDR_BASE(2)
PMB_ADDR_ENTRY3:	.long	PMB_ADDR_BASE(3)
PMB_ADDR_ENTRY4:	.long	PMB_ADDR_BASE(4)
PMB_ADDR_ENTRY6:	.long	PMB_ADDR_BASE(6)
PMB_ADDR_ENTRY7:	.long	PMB_ADDR_BASE(7)
PMB_ADDR_ENTRY8:	.long	PMB_ADDR_BASE(8)
PMB_ADDR_ENTRY9:	.long	PMB_ADDR_BASE(9)
PMB_ADDR_ENTRY10:	.long	PMB_ADDR_BASE(10)
PMB_ADDR_ENTRY11:	.long	PMB_ADDR_BASE(11)
PMB_ADDR_ENTRY12:	.long	PMB_ADDR_BASE(12)
PMB_ADDR_ENTRY13:	.long	PMB_ADDR_BASE(13)
PMB_ADDR_ENTRY14:	.long	PMB_ADDR_BASE(14)
PMB_ADDR_ENTRY15:	.long	PMB_ADDR_BASE(15)

PMB_ADDR_SPIBOOT_D:	.long	mk_pmb_addr_val(0xa0)
PMB_ADDR_DDR_C1_D:	.long	mk_pmb_addr_val(0x88)
PMB_ADDR_DDR_N1_D:	.long	mk_pmb_addr_val(0xa8)
PMB_ADDR_NOT_USE_D:	.long	0x00000000

PMB_DATA_SPIBOOT_A:	.long	PMB_DATA_BASE(0)
PMB_DATA_DDR_N1_A:	.long	PMB_DATA_BASE(1)
PMB_DATA_DDR_C1_A:	.long	PMB_DATA_BASE(5)

/*						ppn   ub v s1 s0  c  wt */
PMB_DATA_SPIBOOT_D:	.long	mk_pmb_data_val(0x00, 0, 1, 1, 0, 0, 1)
PMB_DATA_DDR_C1_D:	.long	mk_pmb_data_val(0x48, 0, 1, 1, 0, 1, 1)
PMB_DATA_DDR_N1_D:	.long	mk_pmb_data_val(0x48, 1, 1, 1, 0, 0, 1)

PASCR_A:		.long	0xff000070
DUMMY_ADDR:		.long	0xa0000000
PASCR_29BIT_D:		.long	0x00000000
PASCR_INIT:		.long	0x80000080
MMUCR_A:		.long	0xff000010
MMUCR_D:		.long	0x00000004	/* clear ITLB */
#endif	/* CONFIG_SH_32BIT */

CCR_A:		.long	CCR
CCR_D:		.long	CCR_CACHE_INIT
