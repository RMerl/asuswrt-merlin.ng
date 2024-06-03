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
 */

#include <common.h>
#include <linux/kbuild.h>
#include <linux/arm-smccc.h>

#if defined(CONFIG_MX25) || defined(CONFIG_MX27) || defined(CONFIG_MX35) \
	|| defined(CONFIG_MX51) || defined(CONFIG_MX53)
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

#if defined(CONFIG_MX25)
	/* Clock Control Module */
	DEFINE(CCM_CCTL, offsetof(struct ccm_regs, cctl));
	DEFINE(CCM_CGCR0, offsetof(struct ccm_regs, cgr0));
	DEFINE(CCM_CGCR1, offsetof(struct ccm_regs, cgr1));
	DEFINE(CCM_CGCR2, offsetof(struct ccm_regs, cgr2));
	DEFINE(CCM_PCDR2, offsetof(struct ccm_regs, pcdr[2]));
	DEFINE(CCM_MCR, offsetof(struct ccm_regs, mcr));

	/* Enhanced SDRAM Controller */
	DEFINE(ESDRAMC_ESDCTL0, offsetof(struct esdramc_regs, ctl0));
	DEFINE(ESDRAMC_ESDCFG0, offsetof(struct esdramc_regs, cfg0));
	DEFINE(ESDRAMC_ESDMISC, offsetof(struct esdramc_regs, misc));

	/* Multi-Layer AHB Crossbar Switch */
	DEFINE(MAX_MPR0, offsetof(struct max_regs, mpr0));
	DEFINE(MAX_SGPCR0, offsetof(struct max_regs, sgpcr0));
	DEFINE(MAX_MPR1, offsetof(struct max_regs, mpr1));
	DEFINE(MAX_SGPCR1, offsetof(struct max_regs, sgpcr1));
	DEFINE(MAX_MPR2, offsetof(struct max_regs, mpr2));
	DEFINE(MAX_SGPCR2, offsetof(struct max_regs, sgpcr2));
	DEFINE(MAX_MPR3, offsetof(struct max_regs, mpr3));
	DEFINE(MAX_SGPCR3, offsetof(struct max_regs, sgpcr3));
	DEFINE(MAX_MPR4, offsetof(struct max_regs, mpr4));
	DEFINE(MAX_SGPCR4, offsetof(struct max_regs, sgpcr4));
	DEFINE(MAX_MGPCR0, offsetof(struct max_regs, mgpcr0));
	DEFINE(MAX_MGPCR1, offsetof(struct max_regs, mgpcr1));
	DEFINE(MAX_MGPCR2, offsetof(struct max_regs, mgpcr2));
	DEFINE(MAX_MGPCR3, offsetof(struct max_regs, mgpcr3));
	DEFINE(MAX_MGPCR4, offsetof(struct max_regs, mgpcr4));

	/* AHB <-> IP-Bus Interface */
	DEFINE(AIPS_MPR_0_7, offsetof(struct aips_regs, mpr_0_7));
	DEFINE(AIPS_MPR_8_15, offsetof(struct aips_regs, mpr_8_15));
#endif

#if defined(CONFIG_MX27)
	DEFINE(AIPI1_PSR0, IMX_AIPI1_BASE + offsetof(struct aipi_regs, psr0));
	DEFINE(AIPI1_PSR1, IMX_AIPI1_BASE + offsetof(struct aipi_regs, psr1));
	DEFINE(AIPI2_PSR0, IMX_AIPI2_BASE + offsetof(struct aipi_regs, psr0));
	DEFINE(AIPI2_PSR1, IMX_AIPI2_BASE + offsetof(struct aipi_regs, psr1));

	DEFINE(CSCR, IMX_PLL_BASE + offsetof(struct pll_regs, cscr));
	DEFINE(MPCTL0, IMX_PLL_BASE + offsetof(struct pll_regs, mpctl0));
	DEFINE(SPCTL0, IMX_PLL_BASE + offsetof(struct pll_regs, spctl0));
	DEFINE(PCDR0, IMX_PLL_BASE + offsetof(struct pll_regs, pcdr0));
	DEFINE(PCDR1, IMX_PLL_BASE + offsetof(struct pll_regs, pcdr1));
	DEFINE(PCCR0, IMX_PLL_BASE + offsetof(struct pll_regs, pccr0));
	DEFINE(PCCR1, IMX_PLL_BASE + offsetof(struct pll_regs, pccr1));

	DEFINE(ESDCTL0_ROF, offsetof(struct esdramc_regs, esdctl0));
	DEFINE(ESDCFG0_ROF, offsetof(struct esdramc_regs, esdcfg0));
	DEFINE(ESDCTL1_ROF, offsetof(struct esdramc_regs, esdctl1));
	DEFINE(ESDCFG1_ROF, offsetof(struct esdramc_regs, esdcfg1));
	DEFINE(ESDMISC_ROF, offsetof(struct esdramc_regs, esdmisc));

	DEFINE(GPCR, IMX_SYSTEM_CTL_BASE +
		offsetof(struct system_control_regs, gpcr));
	DEFINE(FMCR, IMX_SYSTEM_CTL_BASE +
		offsetof(struct system_control_regs, fmcr));
#endif

#if defined(CONFIG_MX35)
	/* Round up to make sure size gives nice stack alignment */
	DEFINE(CLKCTL_CCMR, offsetof(struct ccm_regs, ccmr));
	DEFINE(CLKCTL_PDR0, offsetof(struct ccm_regs, pdr0));
	DEFINE(CLKCTL_PDR1, offsetof(struct ccm_regs, pdr1));
	DEFINE(CLKCTL_PDR2, offsetof(struct ccm_regs, pdr2));
	DEFINE(CLKCTL_PDR3, offsetof(struct ccm_regs, pdr3));
	DEFINE(CLKCTL_PDR4, offsetof(struct ccm_regs, pdr4));
	DEFINE(CLKCTL_RCSR, offsetof(struct ccm_regs, rcsr));
	DEFINE(CLKCTL_MPCTL, offsetof(struct ccm_regs, mpctl));
	DEFINE(CLKCTL_PPCTL, offsetof(struct ccm_regs, ppctl));
	DEFINE(CLKCTL_ACMR, offsetof(struct ccm_regs, acmr));
	DEFINE(CLKCTL_COSR, offsetof(struct ccm_regs, cosr));
	DEFINE(CLKCTL_CGR0, offsetof(struct ccm_regs, cgr0));
	DEFINE(CLKCTL_CGR1, offsetof(struct ccm_regs, cgr1));
	DEFINE(CLKCTL_CGR2, offsetof(struct ccm_regs, cgr2));
	DEFINE(CLKCTL_CGR3, offsetof(struct ccm_regs, cgr3));

	/* Multi-Layer AHB Crossbar Switch */
	DEFINE(MAX_MPR0, offsetof(struct max_regs, mpr0));
	DEFINE(MAX_SGPCR0, offsetof(struct max_regs, sgpcr0));
	DEFINE(MAX_MPR1, offsetof(struct max_regs, mpr1));
	DEFINE(MAX_SGPCR1, offsetof(struct max_regs, sgpcr1));
	DEFINE(MAX_MPR2, offsetof(struct max_regs, mpr2));
	DEFINE(MAX_SGPCR2, offsetof(struct max_regs, sgpcr2));
	DEFINE(MAX_MPR3, offsetof(struct max_regs, mpr3));
	DEFINE(MAX_SGPCR3, offsetof(struct max_regs, sgpcr3));
	DEFINE(MAX_MPR4, offsetof(struct max_regs, mpr4));
	DEFINE(MAX_SGPCR4, offsetof(struct max_regs, sgpcr4));
	DEFINE(MAX_MGPCR0, offsetof(struct max_regs, mgpcr0));
	DEFINE(MAX_MGPCR1, offsetof(struct max_regs, mgpcr1));
	DEFINE(MAX_MGPCR2, offsetof(struct max_regs, mgpcr2));
	DEFINE(MAX_MGPCR3, offsetof(struct max_regs, mgpcr3));
	DEFINE(MAX_MGPCR4, offsetof(struct max_regs, mgpcr4));
	DEFINE(MAX_MGPCR5, offsetof(struct max_regs, mgpcr5));

	/* AHB <-> IP-Bus Interface */
	DEFINE(AIPS_MPR_0_7, offsetof(struct aips_regs, mpr_0_7));
	DEFINE(AIPS_MPR_8_15, offsetof(struct aips_regs, mpr_8_15));
	DEFINE(AIPS_PACR_0_7, offsetof(struct aips_regs, pacr_0_7));
	DEFINE(AIPS_PACR_8_15, offsetof(struct aips_regs, pacr_8_15));
	DEFINE(AIPS_PACR_16_23, offsetof(struct aips_regs, pacr_16_23));
	DEFINE(AIPS_PACR_24_31, offsetof(struct aips_regs, pacr_24_31));
	DEFINE(AIPS_OPACR_0_7, offsetof(struct aips_regs, opacr_0_7));
	DEFINE(AIPS_OPACR_8_15, offsetof(struct aips_regs, opacr_8_15));
	DEFINE(AIPS_OPACR_16_23, offsetof(struct aips_regs, opacr_16_23));
	DEFINE(AIPS_OPACR_24_31, offsetof(struct aips_regs, opacr_24_31));
	DEFINE(AIPS_OPACR_32_39, offsetof(struct aips_regs, opacr_32_39));
#endif

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
#endif

	return 0;
}
