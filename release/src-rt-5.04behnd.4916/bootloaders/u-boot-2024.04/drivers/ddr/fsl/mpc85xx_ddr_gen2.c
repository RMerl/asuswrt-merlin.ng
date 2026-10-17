// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <fsl_ddr_sdram.h>
#include <linux/delay.h>

#if (CONFIG_CHIP_SELECTS_PER_CTRL > 4)
#error Invalid setting for CONFIG_CHIP_SELECTS_PER_CTRL
#endif

void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
			     unsigned int ctrl_num, int step)
{
	unsigned int i;
	struct ccsr_ddr __iomem *ddr =
		(struct ccsr_ddr __iomem *)CFG_SYS_FSL_DDR_ADDR;

#if defined(CONFIG_SYS_FSL_ERRATUM_NMG_DDR120) && defined(CONFIG_MPC85xx)
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	uint svr;
#endif

	if (ctrl_num) {
		printf("%s unexpected ctrl_num = %u\n", __FUNCTION__, ctrl_num);
		return;
	}

#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_DDR120
	/*
	 * Set the DDR IO receiver to an acceptable bias point.
	 * Fixed in Rev 2.1.
	 */
	svr = get_svr();
	if ((SVR_MAJ(svr) == 1) || IS_SVR_REV(svr, 2, 0)) {
		if ((regs->ddr_sdram_cfg & SDRAM_CFG_SDRAM_TYPE_MASK) ==
		   SDRAM_CFG_SDRAM_TYPE_DDR2)
			out_be32(&gur->ddrioovcr, 0x90000000);
		else
			out_be32(&gur->ddrioovcr, 0xA8000000);
	}
#endif

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i == 0) {
			out_be32(&ddr->cs0_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs0_config, regs->cs[i].config);

		} else if (i == 1) {
			out_be32(&ddr->cs1_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs1_config, regs->cs[i].config);

		} else if (i == 2) {
			out_be32(&ddr->cs2_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs2_config, regs->cs[i].config);

		} else if (i == 3) {
			out_be32(&ddr->cs3_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs3_config, regs->cs[i].config);
		}
	}

	out_be32(&ddr->timing_cfg_3, regs->timing_cfg_3);
	out_be32(&ddr->timing_cfg_0, regs->timing_cfg_0);
	out_be32(&ddr->timing_cfg_1, regs->timing_cfg_1);
	out_be32(&ddr->timing_cfg_2, regs->timing_cfg_2);
	out_be32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2);
	out_be32(&ddr->sdram_mode, regs->ddr_sdram_mode);
	out_be32(&ddr->sdram_mode_2, regs->ddr_sdram_mode_2);
	out_be32(&ddr->sdram_md_cntl, regs->ddr_sdram_md_cntl);
	out_be32(&ddr->sdram_interval, regs->ddr_sdram_interval);
	out_be32(&ddr->sdram_data_init, regs->ddr_data_init);
	out_be32(&ddr->sdram_clk_cntl, regs->ddr_sdram_clk_cntl);
	out_be32(&ddr->init_addr, regs->ddr_init_addr);
	out_be32(&ddr->init_ext_addr, regs->ddr_init_ext_addr);

	/*
	 * 200 painful micro-seconds must elapse between
	 * the DDR clock setup and the DDR config enable.
	 */
	udelay(200);
	asm volatile("sync;isync");

	out_be32(&ddr->sdram_cfg, regs->ddr_sdram_cfg);

	/* Poll DDR_SDRAM_CFG_2[D_INIT] bit until auto-data init is done.  */
	while (in_be32(&ddr->sdram_cfg_2) & 0x10) {
		udelay(10000);		/* throttle polling rate */
	}
}
