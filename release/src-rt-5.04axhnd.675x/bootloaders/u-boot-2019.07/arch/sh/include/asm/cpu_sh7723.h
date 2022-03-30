/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Renesas Solutions Corp.
 *
 * SH7723 Internal I/O register
 */

#ifndef _ASM_CPU_SH7723_H_
#define _ASM_CPU_SH7723_H_

#define CACHE_OC_NUM_WAYS	4
#define CCR_CACHE_INIT	0x0000090d

/* EXP */
#define TRA		0xFF000020
#define EXPEVT	0xFF000024
#define INTEVT	0xFF000028

/* MMU */
#define PTEH	0xFF000000
#define PTEL	0xFF000004
#define TTB		0xFF000008
#define TEA		0xFF00000C
#define MMUCR	0xFF000010
#define PASCR	0xFF000070
#define IRMCR	0xFF000078

/* CACHE */
#define CCR		0xFF00001C
#define RAMCR	0xFF000074

/* INTC */

/* BSC */
#define CMNCR		0xFEC10000
#define	CS0BCR		0xFEC10004
#define CS2BCR		0xFEC10008
#define CS4BCR		0xFEC10010
#define CS5ABCR		0xFEC10014
#define CS5BBCR		0xFEC10018
#define CS6ABCR		0xFEC1001C
#define CS6BBCR		0xFEC10020
#define CS0WCR		0xFEC10024
#define CS2WCR		0xFEC10028
#define CS4WCR		0xFEC10030
#define CS5AWCR		0xFEC10034
#define CS5BWCR		0xFEC10038
#define CS6AWCR		0xFEC1003C
#define CS6BWCR		0xFEC10040
#define RBWTCNT		0xFEC10054

/* SBSC */
#define SBSC_SDCR	0xFE400008
#define SBSC_SDWCR	0xFE40000C
#define SBSC_SDPCR	0xFE400010
#define SBSC_RTCSR	0xFE400014
#define SBSC_RTCNT	0xFE400018
#define SBSC_RTCOR	0xFE40001C
#define SBSC_RFCR	0xFE400020

/* DMAC */

/* CPG */
#define FRQCR       0xA4150000
#define VCLKCR      0xA4150004
#define SCLKACR     0xA4150008
#define SCLKBCR     0xA415000C
#define IRDACLKCR   0xA4150018
#define PLLCR       0xA4150024
#define DLLFRQ      0xA4150050

/* LOW POWER MODE */
#define STBCR       0xA4150020
#define MSTPCR0     0xA4150030
#define MSTPCR1     0xA4150034
#define MSTPCR2     0xA4150038

/* RWDT */
#define RWTCNT      0xA4520000
#define RWTCSR      0xA4520004
#define WTCNT		RWTCNT

/* TMU */
#define TMU_BASE	0xFFD80000

/* TPU */

/* CMT */
#define CMSTR       0xA44A0000
#define CMCSR       0xA44A0060
#define CMCNT       0xA44A0064
#define CMCOR       0xA44A0068

/* MSIOF */

/* SCIF */
#define SCIF0_BASE  0xFFE00000
#define SCIF1_BASE  0xFFE10000
#define SCIF2_BASE  0xFFE20000
#define SCIF3_BASE  0xa4e30000
#define SCIF4_BASE  0xa4e40000
#define SCIF5_BASE  0xa4e50000

/* RTC */
/* IrDA */
/* KEYSC */
/* USB */
/* IIC */
/* FLCTL */
/* VPU */
/* VIO(CEU) */
/* VIO(VEU) */
/* VIO(BEU) */
/* 2DG */
/* LCDC */
/* VOU */
/* TSIF */
/* SIU */
/* ATAPI */

/* PFC */
#define PACR        0xA4050100
#define PBCR        0xA4050102
#define PCCR        0xA4050104
#define PDCR        0xA4050106
#define PECR        0xA4050108
#define PFCR        0xA405010A
#define PGCR        0xA405010C
#define PHCR        0xA405010E
#define PJCR        0xA4050110
#define PKCR        0xA4050112
#define PLCR        0xA4050114
#define PMCR        0xA4050116
#define PNCR        0xA4050118
#define PQCR        0xA405011A
#define PRCR        0xA405011C
#define PSCR        0xA405011E
#define PTCR        0xA4050140
#define PUCR        0xA4050142
#define PVCR        0xA4050144
#define PWCR        0xA4050146
#define PXCR        0xA4050148
#define PYCR        0xA405014A
#define PZCR        0xA405014C
#define PSELA       0xA405014E
#define PSELB       0xA4050150
#define PSELC       0xA4050152
#define PSELD       0xA4050154
#define HIZCRA      0xA4050158
#define HIZCRB      0xA405015A
#define HIZCRC      0xA405015C
#define HIZCRD      0xA405015E
#define MSELCRA     0xA4050180
#define MSELCRB     0xA4050182
#define PULCR       0xA4050184
#define DRVCRA      0xA405018A
#define DRVCRB      0xA405018C

/* I/O Port */
#define PADR        0xA4050120
#define PBDR        0xA4050122
#define PCDR        0xA4050124
#define PDDR        0xA4050126
#define PEDR        0xA4050128
#define PFDR        0xA405012A
#define PGDR        0xA405012C
#define PHDR        0xA405012E
#define PJDR        0xA4050130
#define PKDR        0xA4050132
#define PLDR        0xA4050134
#define PMDR        0xA4050136
#define PNDR        0xA4050138
#define PQDR        0xA405013A
#define PRDR        0xA405013C
#define PSDR        0xA405013E
#define PTDR        0xA4050160
#define PUDR        0xA4050162
#define PVDR        0xA4050164
#define PWDR        0xA4050166
#define PXDR        0xA4050168
#define PYDR        0xA405016A
#define PZDR        0xA405016C

/* UBC */
/* H-UDI */

#endif /* _ASM_CPU_SH7723_H_ */
