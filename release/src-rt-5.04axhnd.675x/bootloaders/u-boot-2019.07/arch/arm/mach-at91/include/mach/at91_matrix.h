/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 */

#ifndef AT91_MATRIX_H
#define AT91_MATRIX_H

#ifdef __ASSEMBLY__

#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20)
#define AT91_ASM_MATRIX_CSA0	(ATMEL_BASE_MATRIX + 0x11C)
#elif defined(CONFIG_AT91SAM9261)
#define AT91_ASM_MATRIX_CSA0	(ATMEL_BASE_MATRIX + 0x30)
#elif defined(CONFIG_AT91SAM9263)
#define AT91_ASM_MATRIX_CSA0	(ATMEL_BASE_MATRIX + 0x120)
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#define AT91_ASM_MATRIX_CSA0	(ATMEL_BASE_MATRIX + 0x128)
#else
#error AT91_ASM_MATRIX_CSA0 is not definied for current CPU
#endif

#define AT91_ASM_MATRIX_MCFG	ATMEL_BASE_MATRIX

#else
#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9G20)
#define AT91_MATRIX_MASTERS	6
#define AT91_MATRIX_SLAVES	5
#elif defined(CONFIG_AT91SAM9261)
#define AT91_MATRIX_MASTERS	1
#define AT91_MATRIX_SLAVES	5
#elif defined(CONFIG_AT91SAM9263)
#define AT91_MATRIX_MASTERS	9
#define AT91_MATRIX_SLAVES	7
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#define AT91_MATRIX_MASTERS	11
#define AT91_MATRIX_SLAVES	8
#else
#error CPU not supported. Please update at91_matrix.h
#endif

typedef struct at91_priority {
	u32	a;
	u32	b;
} at91_priority_t;

typedef struct at91_matrix {
	u32		mcfg[AT91_MATRIX_MASTERS];
#if defined(CONFIG_AT91SAM9261)
	u32		scfg[AT91_MATRIX_SLAVES];
	u32		res61_1[3];
	u32		tcr;
	u32		res61_2[2];
	u32		csa;
	u32		pucr;
	u32		res61_3[114];
#else
	u32		reserve1[16 - AT91_MATRIX_MASTERS];
	u32		scfg[AT91_MATRIX_SLAVES];
	u32		reserve2[16 - AT91_MATRIX_SLAVES];
	at91_priority_t	pr[AT91_MATRIX_SLAVES];
	u32		reserve3[32 - (2 * AT91_MATRIX_SLAVES)];
	u32		mrcr;		/* 0x100 Master Remap Control */
	u32		reserve4[3];
#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
	u32		ccr[52];	/* 0x110 - 0x1E0 Chip Configuration */
	u32		womr;		/* 0x1E4 Write Protect Mode  */
	u32		wpsr;		/* 0x1E8 Write Protect Status */
	u32		resg45_1[10];
#elif defined(CONFIG_AT91SAM9260)  || defined(CONFIG_AT91SAM9G20)
	u32		res60_1[3];
	u32		csa;
	u32		res60_2[56];
#elif defined(CONFIG_AT91SAM9263)
	u32		res63_1;
	u32		tcmr;
	u32		res63_2[2];
	u32		csa[2];
	u32		res63_3[54];
#else
	u32		reserve5[60];
#endif
#endif
} at91_matrix_t;

#endif /* __ASSEMBLY__ */

#define AT91_MATRIX_CSA_DBPUC		0x00000100
#define AT91_MATRIX_CSA_VDDIOMSEL_1_8V	0x00000000
#define AT91_MATRIX_CSA_VDDIOMSEL_3_3V	0x00010000

#define AT91_MATRIX_CSA_EBI_CS1A	0x00000002
#define AT91_MATRIX_CSA_EBI_CS3A	0x00000008
#define AT91_MATRIX_CSA_EBI_CS4A	0x00000010
#define AT91_MATRIX_CSA_EBI_CS5A	0x00000020

#define AT91_MATRIX_CSA_EBI1_CS2A	0x00000008

#if defined CONFIG_AT91SAM9261
/* Remap Command for AHB Master 0 (ARM926EJ-S Instruction Master) */
#define	AT91_MATRIX_MCFG_RCB0	(1 << 0)
/* Remap Command for AHB Master 1 (ARM926EJ-S Data Master) */
#define	AT91_MATRIX_MCFG_RCB1	(1 << 1)
#endif

/* Undefined Length Burst Type */
#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9263) || \
	defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#define AT91_MATRIX_MCFG_ULBT_INFINITE	0x00000000
#define AT91_MATRIX_MCFG_ULBT_SINGLE	0x00000001
#define AT91_MATRIX_MCFG_ULBT_FOUR	0x00000002
#define AT91_MATRIX_MCFG_ULBT_EIGHT	0x00000003
#define AT91_MATRIX_MCFG_ULBT_SIXTEEN	0x00000004
#endif
#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#define AT91_MATRIX_MCFG_ULBT_THIRTYTWO	0x00000005
#define AT91_MATRIX_MCFG_ULBT_SIXTYFOUR	0x00000006
#define AT91_MATRIX_MCFG_ULBT_128	0x00000007
#endif

/* Default Master Type */
#define AT91_MATRIX_SCFG_DEFMSTR_TYPE_NONE	0x00000000
#define AT91_MATRIX_SCFG_DEFMSTR_TYPE_LAST	0x00010000
#define AT91_MATRIX_SCFG_DEFMSTR_TYPE_FIXED	0x00020000

/* Fixed Index of Default Master */
#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9263) || \
	defined(CONFIG_AT91SAM9M10G45)
#define	AT91_MATRIX_SCFG_FIXED_DEFMSTR(x)	((x & 0xf) << 18)
#elif defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9260)
#define	AT91_MATRIX_SCFG_FIXED_DEFMSTR(x)	((x & 7) << 18)
#endif

/* Maximum Number of Allowed Cycles for a Burst */
#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#define	AT91_MATRIX_SCFG_SLOT_CYCLE(x)	((x & 0x1ff) << 0)
#elif defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9261) || \
	defined(CONFIG_AT91SAM9263)
#define	AT91_MATRIX_SCFG_SLOT_CYCLE(x)	((x & 0xff) << 0)
#endif

/* Arbitration Type */
#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9263)
#define	AT91_MATRIX_SCFG_ARBT_ROUND_ROBIN	0x00000000
#define	AT91_MATRIX_SCFG_ARBT_FIXED_PRIORITY	0x01000000
#endif

/* Master Remap Control Register */
#if defined(CONFIG_AT91SAM9260) || defined(CONFIG_AT91SAM9263) || \
	defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
/* Remap Command for AHB Master 0 (ARM926EJ-S Instruction Master) */
#define	AT91_MATRIX_MRCR_RCB0	(1 << 0)
/* Remap Command for AHB Master 1 (ARM926EJ-S Data Master) */
#define	AT91_MATRIX_MRCR_RCB1	(1 << 1)
#endif
#if defined(CONFIG_AT91SAM9263) || defined(CONFIG_AT91SAM9G45) || \
	defined(CONFIG_AT91SAM9M10G45)
#define	AT91_MATRIX_MRCR_RCB2	0x00000004
#define	AT91_MATRIX_MRCR_RCB3	0x00000008
#define	AT91_MATRIX_MRCR_RCB4	0x00000010
#define	AT91_MATRIX_MRCR_RCB5	0x00000020
#define	AT91_MATRIX_MRCR_RCB6	0x00000040
#define	AT91_MATRIX_MRCR_RCB7	0x00000080
#define	AT91_MATRIX_MRCR_RCB8	0x00000100
#endif
#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
#define	AT91_MATRIX_MRCR_RCB9	0x00000200
#define	AT91_MATRIX_MRCR_RCB10	0x00000400
#define	AT91_MATRIX_MRCR_RCB11	0x00000800
#endif

/* TCM Configuration Register */
#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
/* Size of ITCM enabled memory block */
#define	AT91_MATRIX_TCMR_ITCM_0		0x00000000
#define	AT91_MATRIX_TCMR_ITCM_32	0x00000040
/* Size of DTCM enabled memory block */
#define	AT91_MATRIX_TCMR_DTCM_0		0x00000000
#define	AT91_MATRIX_TCMR_DTCM_32	0x00000060
#define	AT91_MATRIX_TCMR_DTCM_64	0x00000070
/* Wait state TCM register */
#define	AT91_MATRIX_TCMR_TCM_NO_WS	0x00000000
#define	AT91_MATRIX_TCMR_TCM_ONE_WS	0x00000800
#endif
#if defined(CONFIG_AT91SAM9263)
/* Size of ITCM enabled memory block */
#define	AT91_MATRIX_TCMR_ITCM_0		0x00000000
#define	AT91_MATRIX_TCMR_ITCM_16	0x00000005
#define	AT91_MATRIX_TCMR_ITCM_32	0x00000006
/* Size of DTCM enabled memory block */
#define	AT91_MATRIX_TCMR_DTCM_0		0x00000000
#define	AT91_MATRIX_TCMR_DTCM_16	0x00000050
#define	AT91_MATRIX_TCMR_DTCM_32	0x00000060
#endif
#if defined(CONFIG_AT91SAM9261)
/* Size of ITCM enabled memory block */
#define	AT91_MATRIX_TCMR_ITCM_0		0x00000000
#define	AT91_MATRIX_TCMR_ITCM_16	0x00000005
#define	AT91_MATRIX_TCMR_ITCM_32	0x00000006
#define	AT91_MATRIX_TCMR_ITCM_64	0x00000007
/* Size of DTCM enabled memory block */
#define	AT91_MATRIX_TCMR_DTCM_0		0x00000000
#define	AT91_MATRIX_TCMR_DTCM_16	0x00000050
#define	AT91_MATRIX_TCMR_DTCM_32	0x00000060
#define	AT91_MATRIX_TCMR_DTCM_64	0x00000070
#endif

#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
/* Video Mode Configuration Register */
#define	AT91C_MATRIX_VDEC_SEL_OFF	0x00000000
#define	AT91C_MATRIX_VDEC_SEL_ON	0x00000001
/* Write Protect Mode Register */
#define	AT91_MATRIX_WPMR_WP_WPDIS	0x00000000
#define	AT91_MATRIX_WPMR_WP_WPEN	0x00000001
#define	AT91_MATRIX_WPMR_WPKEY		0xFFFFFF00	/* Write Protect KEY */
/* Write Protect Status Register */
#define	AT91_MATRIX_WPSR_NO_WPV		0x00000000
#define	AT91_MATRIX_WPSR_WPV		0x00000001
#define	AT91_MATRIX_WPSR_WPVSRC		0x00FFFF00	/* Write Protect Violation Source */
#endif

/* USB Pad Pull-Up Control Register */
#if defined(CONFIG_AT91SAM9261)
#define	AT91_MATRIX_USBPUCR_PUON	0x40000000
#endif

#define AT91_MATRIX_PRA_M0(x)	((x & 3) << 0)	/* Master 0 Priority Reg. A*/
#define AT91_MATRIX_PRA_M1(x)	((x & 3) << 4)	/* Master 1 Priority Reg. A*/
#define AT91_MATRIX_PRA_M2(x)	((x & 3) << 8)	/* Master 2 Priority Reg. A*/
#define AT91_MATRIX_PRA_M3(x)	((x & 3) << 12)	/* Master 3 Priority Reg. A*/
#define AT91_MATRIX_PRA_M4(x)	((x & 3) << 16)	/* Master 4 Priority Reg. A*/
#define AT91_MATRIX_PRA_M5(x)	((x & 3) << 20)	/* Master 5 Priority Reg. A*/
#define AT91_MATRIX_PRA_M6(x)	((x & 3) << 24)	/* Master 6 Priority Reg. A*/
#define AT91_MATRIX_PRA_M7(x)	((x & 3) << 28)	/* Master 7 Priority Reg. A*/
#define AT91_MATRIX_PRB_M8(x)	((x & 3) << 0)	/* Master 8 Priority Reg. B) */
#define AT91_MATRIX_PRB_M9(x)	((x & 3) << 4)	/* Master 9 Priority Reg. B) */
#define AT91_MATRIX_PRB_M10(x)	((x & 3) << 8)	/* Master 10 Priority Reg. B) */

#endif
