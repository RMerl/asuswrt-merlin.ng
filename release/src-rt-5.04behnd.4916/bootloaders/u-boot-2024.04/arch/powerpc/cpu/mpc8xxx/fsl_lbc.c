// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <hang.h>
#include <init.h>
#include <asm/fsl_lbc.h>

#ifdef CONFIG_MPC83xx
#include "../mpc83xx/elbc/elbc.h"
#endif

#ifdef CONFIG_MPC85xx
/* Boards should provide their own version of this if they use lbc sdram */
static void __lbc_sdram_init(void)
{
	/* Do nothing */
}
void lbc_sdram_init(void) __attribute__((weak, alias("__lbc_sdram_init")));
#endif


void print_lbc_regs(void)
{
	int i;

	printf("\nLocal Bus Controller Registers\n");
	for (i = 0; i < 8; i++) {
		printf("BR%d\t0x%08X\tOR%d\t0x%08X\n",
		       i, get_lbc_br(i), i, get_lbc_or(i));
	}
	printf("LBCR\t0x%08X\tLCRR\t0x%08X\n",
		       get_lbc_lbcr(), get_lbc_lcrr());
}

void init_early_memctl_regs(void)
{
	uint init_br1 = 1;

#ifdef CONFIG_SYS_FSL_ERRATUM_ELBC_A001
	/* Set the local bus monitor timeout value to the maximum */
	clrsetbits_be32(&(LBC_BASE_ADDR)->lbcr, LBCR_BMT|LBCR_BMTPS, 0xf);
#endif

#ifdef CONFIG_MPC85xx
	/* if cs1 is already set via debugger, leave cs0/cs1 alone */
	if (get_lbc_br(1) & BR_V)
		init_br1 = 0;
#endif

	/*
	 * Map banks 0 (and maybe 1) to the FLASH banks 0 (and 1) at
	 * preliminary addresses - these have to be modified later
	 * when FLASH size has been determined
	 */
#if defined(CONFIG_SYS_OR0_REMAP)
	set_lbc_or(0, CONFIG_SYS_OR0_REMAP);
#endif
#if defined(CONFIG_SYS_OR1_REMAP)
	set_lbc_or(1, CONFIG_SYS_OR1_REMAP);
#endif
	/* now restrict to preliminary range */
	if (init_br1) {
#if defined(CONFIG_SYS_BR0_PRELIM) && defined(CONFIG_SYS_OR0_PRELIM)
		set_lbc_br(0, CONFIG_SYS_BR0_PRELIM);
		set_lbc_or(0, CONFIG_SYS_OR0_PRELIM);
#endif

#if defined(CONFIG_SYS_BR1_PRELIM) && defined(CONFIG_SYS_OR1_PRELIM)
		set_lbc_or(1, CONFIG_SYS_OR1_PRELIM);
		set_lbc_br(1, CONFIG_SYS_BR1_PRELIM);
#endif
	}

#if defined(CONFIG_SYS_BR2_PRELIM) && defined(CONFIG_SYS_OR2_PRELIM)
	set_lbc_or(2, CONFIG_SYS_OR2_PRELIM);
	set_lbc_br(2, CONFIG_SYS_BR2_PRELIM);
#endif

#if defined(CONFIG_SYS_BR3_PRELIM) && defined(CONFIG_SYS_OR3_PRELIM)
	set_lbc_or(3, CONFIG_SYS_OR3_PRELIM);
	set_lbc_br(3, CONFIG_SYS_BR3_PRELIM);
#endif

#if defined(CONFIG_SYS_BR4_PRELIM) && defined(CONFIG_SYS_OR4_PRELIM)
	set_lbc_or(4, CONFIG_SYS_OR4_PRELIM);
	set_lbc_br(4, CONFIG_SYS_BR4_PRELIM);
#endif

#if defined(CONFIG_SYS_BR5_PRELIM) && defined(CONFIG_SYS_OR5_PRELIM)
	set_lbc_or(5, CONFIG_SYS_OR5_PRELIM);
	set_lbc_br(5, CONFIG_SYS_BR5_PRELIM);
#endif

#if defined(CONFIG_SYS_BR6_PRELIM) && defined(CONFIG_SYS_OR6_PRELIM)
	set_lbc_or(6, CONFIG_SYS_OR6_PRELIM);
	set_lbc_br(6, CONFIG_SYS_BR6_PRELIM);
#endif

#if defined(CONFIG_SYS_BR7_PRELIM) && defined(CONFIG_SYS_OR7_PRELIM)
	set_lbc_or(7, CONFIG_SYS_OR7_PRELIM);
	set_lbc_br(7, CONFIG_SYS_BR7_PRELIM);
#endif
}

/*
 * Configures a UPM. The function requires the respective MxMR to be set
 * before calling this function. "size" is the number or entries, not a sizeof.
 */
void upmconfig(uint upm, uint *table, uint size)
{
	fsl_lbc_t *lbc = LBC_BASE_ADDR;
	int i, mad, old_mad = 0;
	u32 mask = (~MxMR_OP_MSK & ~MxMR_MAD_MSK);
	u32 msel = BR_UPMx_TO_MSEL(upm);
	u32 *mxmr = &lbc->mamr + upm;
	volatile u8 *dummy = NULL;

	if (upm < UPMA || upm > UPMC) {
		printf("Error: %s() Bad UPM index %d\n", __func__, upm);
		hang();
	}

	/*
	 * Find the address for the dummy write - scan all of the BRs until we
	 * find one matching the UPM and extract the base address bits from it.
	 */
	for (i = 0; i < 8; i++) {
		if ((get_lbc_br(i) & (BR_V | BR_MSEL)) == (BR_V | msel)) {
			dummy = (volatile u8 *)(get_lbc_br(i) & BR_BA);
			break;
		}
	}

	if (!dummy) {
		printf("Error: %s() No matching BR\n", __func__);
		hang();
	}

	/* Program UPM using steps outlined by the reference manual */
	for (i = 0; i < size; i++) {
		out_be32(mxmr, (in_be32(mxmr) & mask) | MxMR_OP_WARR | i);
		out_be32(&lbc->mdr, table[i]);
		(void)in_be32(&lbc->mdr);
		*dummy = 0;
		do {
			mad = in_be32(mxmr) & MxMR_MAD_MSK;
		} while (mad <= old_mad && !(!mad && i == (size-1)));
		old_mad = mad;
	}

	/* Return to normal operation */
	out_be32(mxmr, (in_be32(mxmr) & mask) | MxMR_OP_NORM);
}
