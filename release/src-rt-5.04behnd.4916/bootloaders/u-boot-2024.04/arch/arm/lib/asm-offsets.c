// SPDX-License-Identifier: GPL-2.0+
/*
 * Adapted from Linux v2.6.36 kernel: arch/powerpc/kernel/asm-offsets.c
 *
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 * We use the technique used in the OSF Mach kernel code:
 * generate asm statements containing #defines,
 * compile this file to assembler, and then extract the
 * #defines from the assembly-language output.
 *
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <linux/kbuild.h>
#include <linux/arm-smccc.h>

#if defined(CONFIG_MX51) || defined(CONFIG_MX53)
#include <asm/arch/imx-regs.h>
#endif

int main(void)
{
	/*
	 * TODO : Check if each entry in this file is really necessary.
	 *   - struct esdramc_regs
	 *   - struct max_regs
	 *   - struct aips_regs
	 *   - struct aipi_regs
	 *   - struct clkctl
	 *   - struct dpll
	 * are used only for generating asm-offsets.h.
	 * It means their offset addresses are referenced only from assembly
	 * code. Is it better to define the macros directly in headers?
	 */

#if defined(CONFIG_MX51) || defined(CONFIG_MX53)
	/* Round up to make sure size gives nice stack alignment */
	DEFINE(CLKCTL_CCMR, offsetof(struct clkctl, ccr));
	DEFINE(CLKCTL_CCDR, offsetof(struct clkctl, ccdr));
	DEFINE(CLKCTL_CSR, offsetof(struct clkctl, csr));
	DEFINE(CLKCTL_CCSR, offsetof(struct clkctl, ccsr));
	DEFINE(CLKCTL_CACRR, offsetof(struct clkctl, cacrr));
	DEFINE(CLKCTL_CBCDR, offsetof(struct clkctl, cbcdr));
	DEFINE(CLKCTL_CBCMR, offsetof(struct clkctl, cbcmr));
	DEFINE(CLKCTL_CSCMR1, offsetof(struct clkctl, cscmr1));
	DEFINE(CLKCTL_CSCMR2, offsetof(struct clkctl, cscmr2));
	DEFINE(CLKCTL_CSCDR1, offsetof(struct clkctl, cscdr1));
	DEFINE(CLKCTL_CS1CDR, offsetof(struct clkctl, cs1cdr));
	DEFINE(CLKCTL_CS2CDR, offsetof(struct clkctl, cs2cdr));
	DEFINE(CLKCTL_CDCDR, offsetof(struct clkctl, cdcdr));
	DEFINE(CLKCTL_CHSCCDR, offsetof(struct clkctl, chsccdr));
	DEFINE(CLKCTL_CSCDR2, offsetof(struct clkctl, cscdr2));
	DEFINE(CLKCTL_CSCDR3, offsetof(struct clkctl, cscdr3));
	DEFINE(CLKCTL_CSCDR4, offsetof(struct clkctl, cscdr4));
	DEFINE(CLKCTL_CWDR, offsetof(struct clkctl, cwdr));
	DEFINE(CLKCTL_CDHIPR, offsetof(struct clkctl, cdhipr));
	DEFINE(CLKCTL_CDCR, offsetof(struct clkctl, cdcr));
	DEFINE(CLKCTL_CTOR, offsetof(struct clkctl, ctor));
	DEFINE(CLKCTL_CLPCR, offsetof(struct clkctl, clpcr));
	DEFINE(CLKCTL_CISR, offsetof(struct clkctl, cisr));
	DEFINE(CLKCTL_CIMR, offsetof(struct clkctl, cimr));
	DEFINE(CLKCTL_CCOSR, offsetof(struct clkctl, ccosr));
	DEFINE(CLKCTL_CGPR, offsetof(struct clkctl, cgpr));
	DEFINE(CLKCTL_CCGR0, offsetof(struct clkctl, ccgr0));
	DEFINE(CLKCTL_CCGR1, offsetof(struct clkctl, ccgr1));
	DEFINE(CLKCTL_CCGR2, offsetof(struct clkctl, ccgr2));
	DEFINE(CLKCTL_CCGR3, offsetof(struct clkctl, ccgr3));
	DEFINE(CLKCTL_CCGR4, offsetof(struct clkctl, ccgr4));
	DEFINE(CLKCTL_CCGR5, offsetof(struct clkctl, ccgr5));
	DEFINE(CLKCTL_CCGR6, offsetof(struct clkctl, ccgr6));
	DEFINE(CLKCTL_CMEOR, offsetof(struct clkctl, cmeor));
#if defined(CONFIG_MX53)
	DEFINE(CLKCTL_CCGR7, offsetof(struct clkctl, ccgr7));
#endif

	/* DPLL */
	DEFINE(PLL_DP_CTL, offsetof(struct dpll, dp_ctl));
	DEFINE(PLL_DP_CONFIG, offsetof(struct dpll, dp_config));
	DEFINE(PLL_DP_OP, offsetof(struct dpll, dp_op));
	DEFINE(PLL_DP_MFD, offsetof(struct dpll, dp_mfd));
	DEFINE(PLL_DP_MFN, offsetof(struct dpll, dp_mfn));
	DEFINE(PLL_DP_HFS_OP, offsetof(struct dpll, dp_hfs_op));
	DEFINE(PLL_DP_HFS_MFD, offsetof(struct dpll, dp_hfs_mfd));
	DEFINE(PLL_DP_HFS_MFN, offsetof(struct dpll, dp_hfs_mfn));
#endif

#ifdef CONFIG_ARM_SMCCC
	DEFINE(ARM_SMCCC_RES_X0_OFFS, offsetof(struct arm_smccc_res, a0));
	DEFINE(ARM_SMCCC_RES_X2_OFFS, offsetof(struct arm_smccc_res, a2));
	DEFINE(ARM_SMCCC_QUIRK_ID_OFFS, offsetof(struct arm_smccc_quirk, id));
	DEFINE(ARM_SMCCC_QUIRK_STATE_OFFS, offsetof(struct arm_smccc_quirk, state));
#ifdef CONFIG_ARM64
	DEFINE(ARM_SMCCC_1_2_REGS_X0_OFFS,	offsetof(struct arm_smccc_1_2_regs, a0));
	DEFINE(ARM_SMCCC_1_2_REGS_X2_OFFS,	offsetof(struct arm_smccc_1_2_regs, a2));
	DEFINE(ARM_SMCCC_1_2_REGS_X4_OFFS,	offsetof(struct arm_smccc_1_2_regs, a4));
	DEFINE(ARM_SMCCC_1_2_REGS_X6_OFFS,	offsetof(struct arm_smccc_1_2_regs, a6));
	DEFINE(ARM_SMCCC_1_2_REGS_X8_OFFS,	offsetof(struct arm_smccc_1_2_regs, a8));
	DEFINE(ARM_SMCCC_1_2_REGS_X10_OFFS,	offsetof(struct arm_smccc_1_2_regs, a10));
	DEFINE(ARM_SMCCC_1_2_REGS_X12_OFFS,	offsetof(struct arm_smccc_1_2_regs, a12));
	DEFINE(ARM_SMCCC_1_2_REGS_X14_OFFS,	offsetof(struct arm_smccc_1_2_regs, a14));
	DEFINE(ARM_SMCCC_1_2_REGS_X16_OFFS,	offsetof(struct arm_smccc_1_2_regs, a16));
#endif
#endif

	return 0;
}
