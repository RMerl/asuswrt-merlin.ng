/*
 * Adapted from Linux v2.6.36 kernel: arch/powerpc/kernel/asm-offsets.c
 *
 * Generate definitions needed by assembly language modules.
 * This code generates raw asm output which is post-processed to extract
 * and format the required data.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>

#include <linux/kbuild.h>

int main(void)
{
	/*
	 * TODO : Check if each entry in this file is really necessary.
	 *   - struct ftahbc02s
	 *   - struct ftsdmc021
	 *   - struct andes_pcu
	 *   - struct dwcddr21mctl
	 * are used only for generating asm-offsets.h.
	 * It means their offset addresses are referenced only from assembly
	 * code. Is it better to define the macros directly in headers?
	 */

#ifdef CONFIG_FTSMC020
	OFFSET(FTSMC020_BANK0_CR,	ftsmc020, bank[0].cr);
	OFFSET(FTSMC020_BANK0_TPR,	ftsmc020, bank[0].tpr);
#endif
	BLANK();
#ifdef CONFIG_FTAHBC020S
	OFFSET(FTAHBC020S_SLAVE_BSR_4,	ftahbc02s, s_bsr[4]);
	OFFSET(FTAHBC020S_SLAVE_BSR_6,	ftahbc02s, s_bsr[6]);
	OFFSET(FTAHBC020S_CR,		ftahbc02s, cr);
#endif
	BLANK();
#ifdef CONFIG_FTPMU010
	OFFSET(FTPMU010_PDLLCR0,	ftpmu010, PDLLCR0);
#endif
	BLANK();
#ifdef CONFIG_FTSDMC021
	OFFSET(FTSDMC021_TP1,		ftsdmc021, tp1);
	OFFSET(FTSDMC021_TP2,		ftsdmc021, tp2);
	OFFSET(FTSDMC021_CR1,		ftsdmc021, cr1);
	OFFSET(FTSDMC021_CR2,		ftsdmc021, cr2);
	OFFSET(FTSDMC021_BANK0_BSR,	ftsdmc021, bank0_bsr);
	OFFSET(FTSDMC021_BANK1_BSR,	ftsdmc021, bank1_bsr);
	OFFSET(FTSDMC021_BANK2_BSR,	ftsdmc021, bank2_bsr);
	OFFSET(FTSDMC021_BANK3_BSR,	ftsdmc021, bank3_bsr);
#endif
	BLANK();
#ifdef CONFIG_ANDES_PCU
	OFFSET(ANDES_PCU_PCS4,		andes_pcu, pcs4.parm);	/* 0x104 */
#endif
	BLANK();
#ifdef CONFIG_DWCDDR21MCTL
	OFFSET(DWCDDR21MCTL_CCR,	dwcddr21mctl, ccr);	/* 0x04 */
	OFFSET(DWCDDR21MCTL_DCR,	dwcddr21mctl, dcr);	/* 0x04 */
	OFFSET(DWCDDR21MCTL_IOCR,	dwcddr21mctl, iocr);	/* 0x08 */
	OFFSET(DWCDDR21MCTL_CSR,	dwcddr21mctl, csr);	/* 0x0c */
	OFFSET(DWCDDR21MCTL_DRR,	dwcddr21mctl, drr);	/* 0x10 */
	OFFSET(DWCDDR21MCTL_DLLCR0,	dwcddr21mctl, dllcr[0]); /* 0x24 */
	OFFSET(DWCDDR21MCTL_DLLCR1,	dwcddr21mctl, dllcr[1]); /* 0x28 */
	OFFSET(DWCDDR21MCTL_DLLCR2,	dwcddr21mctl, dllcr[2]); /* 0x2c */
	OFFSET(DWCDDR21MCTL_DLLCR3,	dwcddr21mctl, dllcr[3]); /* 0x30 */
	OFFSET(DWCDDR21MCTL_DLLCR4,	dwcddr21mctl, dllcr[4]); /* 0x34 */
	OFFSET(DWCDDR21MCTL_DLLCR5,	dwcddr21mctl, dllcr[5]); /* 0x38 */
	OFFSET(DWCDDR21MCTL_DLLCR6,	dwcddr21mctl, dllcr[6]); /* 0x3c */
	OFFSET(DWCDDR21MCTL_DLLCR7,	dwcddr21mctl, dllcr[7]); /* 0x40 */
	OFFSET(DWCDDR21MCTL_DLLCR8,	dwcddr21mctl, dllcr[8]); /* 0x44 */
	OFFSET(DWCDDR21MCTL_DLLCR9,	dwcddr21mctl, dllcr[9]); /* 0x48 */
	OFFSET(DWCDDR21MCTL_RSLR0,	dwcddr21mctl, rslr[0]);	/* 0x4c */
	OFFSET(DWCDDR21MCTL_RDGR0,	dwcddr21mctl, rdgr[0]);	/* 0x5c */
	OFFSET(DWCDDR21MCTL_DTAR,	dwcddr21mctl, dtar);	/* 0xa4 */
	OFFSET(DWCDDR21MCTL_MR,		dwcddr21mctl, mr);	/* 0x1f0 */
#endif

	return 0;
}
