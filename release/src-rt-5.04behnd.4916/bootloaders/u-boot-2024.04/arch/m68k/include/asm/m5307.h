/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014  Angelo Dureghello <angelo@sysam.it>
 *
 */

#ifndef	mcf5307_h
#define	mcf5307_h

/*
 * Size of internal RAM  (RAMBAR)
 */
#define INT_RAM_SIZE 4096

/* Bit definitions and macros for SYPCR */
#define SYPCR_SWTAVAL		0x02
#define SYPCR_SWTA		0x04
#define SYPCR_SWT(x)		((x&0x3)<<3)
#define SYPCR_SWP		0x20
#define SYPCR_SWRI		0x40
#define SYPCR_SWE		0x80

/* Bit definitions and macros for CSMR */
#define CSMR_V			0x01
#define CSMR_UD			0x02
#define CSMR_UC			0x04
#define CSMR_SD			0x08
#define CSMR_SC			0x10
#define CSMR_CI			0x20
#define CSMR_AM			0x40
#define CSMR_WP			0x100

/* Bit definitions and macros for DACR (SDRAM) */
#define DACR_PM_CONTINUOUS	0x04
#define DACR_IP_PRECHG_ALL	0x08
#define DACR_PORT_SZ_32		0
#define DACR_PORT_SZ_8		(1<<4)
#define DACR_PORT_SZ_16		(2<<4)
#define DACR_IMRS_INIT_CMD	(1<<6)
#define DACR_CMD_PIN(x)		((x&7)<<8)
#define DACR_CASL(x)		((x&3)<<12)
#define DACR_RE			(1<<15)

/* Bit definitions and macros for CSCR */
#define CSCR_BSTW		0x08
#define CSCR_BSTR		0x10
#define CSCR_BEM		0x20
#define CSCR_PS(x)		((x&0x3)<<6)
#define CSCR_AA			0x100
#define CSCR_WS			((x&0xf)<<10)

/* Bit definitions for the ICR family of registers */
#define	MCFSIM_ICR_AUTOVEC	0x80	/* Auto-vectored intr */
#define	MCFSIM_ICR_LEVEL0	0x00	/* Level 0 intr */
#define	MCFSIM_ICR_LEVEL1	0x04	/* Level 1 intr */
#define	MCFSIM_ICR_LEVEL2	0x08	/* Level 2 intr */
#define	MCFSIM_ICR_LEVEL3	0x0c	/* Level 3 intr */
#define	MCFSIM_ICR_LEVEL4	0x10	/* Level 4 intr */
#define	MCFSIM_ICR_LEVEL5	0x14	/* Level 5 intr */
#define	MCFSIM_ICR_LEVEL6	0x18	/* Level 6 intr */
#define	MCFSIM_ICR_LEVEL7	0x1c	/* Level 7 intr */

#define	MCFSIM_ICR_PRI0		0x00	/* Priority 0 intr */
#define	MCFSIM_ICR_PRI1		0x01	/* Priority 1 intr */
#define	MCFSIM_ICR_PRI2		0x02	/* Priority 2 intr */
#define	MCFSIM_ICR_PRI3		0x03	/* Priority 3 intr */

#endif	/* mcf5307_h */
