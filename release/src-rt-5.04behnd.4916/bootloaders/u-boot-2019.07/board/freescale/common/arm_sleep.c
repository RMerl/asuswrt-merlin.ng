// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#ifndef CONFIG_ARMV7_NONSEC
#error " Deep sleep needs non-secure mode support. "
#else
#include <asm/secure.h>
#endif
#include <asm/armv7.h>

#if defined(CONFIG_ARCH_LS1021A)
#include <asm/arch/immap_ls102xa.h>
#endif

#include "sleep.h"
#ifdef CONFIG_U_QE
#include <fsl_qe.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

void __weak board_mem_sleep_setup(void)
{
}

void __weak board_sleep_prepare(void)
{
}

bool is_warm_boot(void)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;

	if (in_be32(&gur->crstsr) & DCFG_CCSR_CRSTSR_WDRFR)
		return 1;

	return 0;
}

void fsl_dp_disable_console(void)
{
	gd->flags |= GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE;
}

/*
 * When wakeup from deep sleep, the first 128 bytes space
 * will be used to do DDR training which corrupts the data
 * in there. This function will restore them.
 */
static void dp_ddr_restore(void)
{
	u64 *src, *dst;
	int i;
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;

	/* get the address of ddr date from SPARECR3 */
	src = (u64 *)in_le32(&scfg->sparecr[2]);
	dst = (u64 *)CONFIG_SYS_SDRAM_BASE;

	for (i = 0; i < DDR_BUFF_LEN / 8; i++)
		*dst++ = *src++;
}

#if defined(CONFIG_ARMV7_PSCI) && defined(CONFIG_ARCH_LS1021A)
void ls1_psci_resume_fixup(void)
{
	u32 tmp;
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;

#ifdef QIXIS_BASE
	void *qixis_base = (void *)QIXIS_BASE;

	/* Pull on PCIe RST# */
	out_8(qixis_base + QIXIS_RST_FORCE_3, 0);

	/* disable deep sleep signals in FPGA */
	tmp = in_8(qixis_base + QIXIS_PWR_CTL2);
	tmp &= ~QIXIS_PWR_CTL2_PCTL;
	out_8(qixis_base + QIXIS_PWR_CTL2, tmp);
#endif

	/* Disable wakeup interrupt during deep sleep */
	out_be32(&scfg->pmcintecr, 0);
	/* Clear PMC interrupt status */
	out_be32(&scfg->pmcintsr, 0xffffffff);

	/* Disable Warm Device Reset */
	tmp = in_be32(&scfg->dpslpcr);
	tmp &= ~SCFG_DPSLPCR_WDRR_EN;
	out_be32(&scfg->dpslpcr, tmp);
}
#endif

static void dp_resume_prepare(void)
{
	dp_ddr_restore();
	board_sleep_prepare();
	armv7_init_nonsec();
#ifdef CONFIG_U_QE
	u_qe_resume();
#endif
#if defined(CONFIG_ARMV7_PSCI) && defined(CONFIG_ARCH_LS1021A)
	ls1_psci_resume_fixup();
#endif
}

int fsl_dp_resume(void)
{
	u32 start_addr;
	void (*kernel_resume)(void);
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;

	if (!is_warm_boot())
		return 0;

	dp_resume_prepare();

	/* Get the entry address and jump to kernel */
	start_addr = in_le32(&scfg->sparecr[3]);
	debug("Entry address is 0x%08x\n", start_addr);
	kernel_resume = (void (*)(void))start_addr;
	secure_ram_addr(_do_nonsec_entry)(kernel_resume, 0, 0, 0);

	return 0;
}
