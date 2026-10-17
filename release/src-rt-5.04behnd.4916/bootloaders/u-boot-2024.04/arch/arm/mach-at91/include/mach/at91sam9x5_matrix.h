/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Matrix-centric header file for the AT91SAM9X5 family
 *
 *  Copyright (C) 2012-2013 Atmel Corporation.
 *
 * Memory Controllers (MATRIX, EBI) - System peripherals registers.
 * Based on AT91SAM9X5 & AT91SAM9N12 preliminary datasheet.
 */

#ifndef __AT91SAM9X5_MATRIX_H__
#define __AT91SAM9X5_MATRIX_H__

#ifndef __ASSEMBLY__

/* AT91SAM9N12 Matrix definition is a subset of AT91SAM9X5. */
struct at91_matrix {
	u32	mcfg[16];
	u32	scfg[16];
	u32	pras[16][2];
	u32	mrcr;           /* 0x100 Master Remap Control */
	u32	filler[5];
#ifdef CONFIG_AT91SAM9X5
	u32	filler1[2];
#endif
	/* EBI Chip Select Assignment Register
	 * 0x118: AT91SAM9N12
	 * 0x120: AT91SAM9X5
	 */
	u32	ebicsa;
	u32	filler4[47];
#ifdef CONFIG_AT91SAM9N12
	u32	filler5[2];
#endif
	u32	wpmr;
	u32	wpsr;
};

#endif /* __ASSEMBLY__ */

#define AT91_MATRIX_ULBT_INFINITE	(0 << 0)
#define AT91_MATRIX_ULBT_SINGLE		(1 << 0)
#define AT91_MATRIX_ULBT_FOUR		(2 << 0)
#define AT91_MATRIX_ULBT_EIGHT		(3 << 0)
#define AT91_MATRIX_ULBT_SIXTEEN	(4 << 0)
#define AT91_MATRIX_ULBT_THIRTYTWO	(5 << 0)
#define AT91_MATRIX_ULBT_SIXTYFOUR	(6 << 0)
#define AT91_MATRIX_ULBT_128		(7 << 0)

#define AT91_MATRIX_DEFMSTR_TYPE_NONE	(0 << 16)
#define AT91_MATRIX_DEFMSTR_TYPE_LAST	(1 << 16)
#define AT91_MATRIX_DEFMSTR_TYPE_FIXED	(2 << 16)
#define AT91_MATRIX_FIXED_DEFMSTR_SHIFT 18

#define AT91_MATRIX_M0PR_SHIFT          0
#define AT91_MATRIX_M1PR_SHIFT          4
#define AT91_MATRIX_M2PR_SHIFT          8
#define AT91_MATRIX_M3PR_SHIFT          12
#define AT91_MATRIX_M4PR_SHIFT          16
#define AT91_MATRIX_M5PR_SHIFT          20
#define AT91_MATRIX_M6PR_SHIFT          24
#define AT91_MATRIX_M7PR_SHIFT          28

#define AT91_MATRIX_M8PR_SHIFT          0  /* register B */
#define AT91_MATRIX_M9PR_SHIFT          4  /* register B */
#define AT91_MATRIX_M10PR_SHIFT         8  /* register B */
#define AT91_MATRIX_M11PR_SHIFT         12 /* register B */

#define AT91_MATRIX_RCB0                (1 << 0)
#define AT91_MATRIX_RCB1                (1 << 1)
#define AT91_MATRIX_RCB2                (1 << 2)
#define AT91_MATRIX_RCB3                (1 << 3)
#define AT91_MATRIX_RCB4                (1 << 4)
#define AT91_MATRIX_RCB5                (1 << 5)
#define AT91_MATRIX_RCB6                (1 << 6)
#define AT91_MATRIX_RCB7                (1 << 7)
#define AT91_MATRIX_RCB8                (1 << 8)
#define AT91_MATRIX_RCB9                (1 << 9)
#define AT91_MATRIX_RCB10               (1 << 10)

#define AT91_MATRIX_EBI_CS1A_SMC                (0 << 1)
#define AT91_MATRIX_EBI_CS1A_SDRAMC             (1 << 1)
#define AT91_MATRIX_EBI_CS3A_SMC                (0 << 3)
#define AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA     (1 << 3)
#define AT91_MATRIX_EBI_DBPU_ON                 (0 << 8)
#define AT91_MATRIX_EBI_DBPU_OFF                (1 << 8)
#define AT91_MATRIX_EBI_DBPD_ON                 (0 << 9)
#define AT91_MATRIX_EBI_DBPD_OFF                (1 << 9)
#define AT91_MATRIX_EBI_VDDIOMSEL_1_8V          (0 << 16)
#define AT91_MATRIX_EBI_VDDIOMSEL_3_3V          (1 << 16)
#define AT91_MATRIX_EBI_EBI_IOSR_REDUCED        (0 << 17)
#define AT91_MATRIX_EBI_EBI_IOSR_NORMAL         (1 << 17)
#define AT91_MATRIX_NFD0_ON_D0                  (0 << 24)
#define AT91_MATRIX_NFD0_ON_D16                 (1 << 24)
#define AT91_MATRIX_MP_OFF                      (0 << 25)
#define AT91_MATRIX_MP_ON                       (1 << 25)

#endif
