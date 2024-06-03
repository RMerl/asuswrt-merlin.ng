// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2008-2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <fsl_ddr_sdram.h>
#include <asm/processor.h>

#if (CONFIG_CHIP_SELECTS_PER_CTRL > 4)
#error Invalid setting for CONFIG_CHIP_SELECTS_PER_CTRL
#endif

/*
 * regs has the to-be-set values for DDR controller registers
 * ctrl_num is the DDR controller number
 * step: 0 goes through the initialization in one pass
 *       1 sets registers and returns before enabling controller
 *       2 resumes from step 1 and continues to initialize
 * Dividing the initialization to two steps to deassert DDR reset signal
 * to comply with JEDEC specs for RDIMMs.
 */
void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
			     unsigned int ctrl_num, int step)
{
	unsigned int i, bus_width;
	struct ccsr_ddr __iomem *ddr;
	u32 temp_sdram_cfg;
	u32 total_gb_size_per_controller;
	int timeout;
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR111_DDR134
	int timeout_save;
	volatile ccsr_local_ecm_t *ecm = (void *)CONFIG_SYS_MPC85xx_ECM_ADDR;
	unsigned int csn_bnds_backup = 0, cs_sa, cs_ea, *csn_bnds_t;
	int csn = -1;
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR_A003
	u32 save1, save2;
#endif

	switch (ctrl_num) {
	case 0:
		ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;
		break;
#if defined(CONFIG_SYS_FSL_DDR2_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 1)
	case 1:
		ddr = (void *)CONFIG_SYS_FSL_DDR2_ADDR;
		break;
#endif
#if defined(CONFIG_SYS_FSL_DDR3_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 2)
	case 2:
		ddr = (void *)CONFIG_SYS_FSL_DDR3_ADDR;
		break;
#endif
#if defined(CONFIG_SYS_FSL_DDR4_ADDR) && (CONFIG_SYS_NUM_DDR_CTLRS > 3)
	case 3:
		ddr = (void *)CONFIG_SYS_FSL_DDR4_ADDR;
		break;
#endif
	default:
		printf("%s unexpected ctrl_num = %u\n", __FUNCTION__, ctrl_num);
		return;
	}

	if (step == 2)
		goto step2;

	if (regs->ddr_eor)
		out_be32(&ddr->eor, regs->ddr_eor);
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR111_DDR134
	debug("Workaround for ERRATUM_DDR111_DDR134\n");
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		cs_sa = (regs->cs[i].bnds >> 16) & 0xfff;
		cs_ea = regs->cs[i].bnds & 0xfff;
		if ((cs_sa <= 0xff) && (cs_ea >= 0xff)) {
			csn = i;
			csn_bnds_backup = regs->cs[i].bnds;
			csn_bnds_t = (unsigned int *) &regs->cs[i].bnds;
			if (cs_ea > 0xeff)
				*csn_bnds_t = regs->cs[i].bnds + 0x01000000;
			else
				*csn_bnds_t = regs->cs[i].bnds + 0x01000100;
			debug("Found cs%d_bns (0x%08x) covering 0xff000000, "
				"change it to 0x%x\n",
				csn, csn_bnds_backup, regs->cs[i].bnds);
			break;
		}
	}
#endif
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i == 0) {
			out_be32(&ddr->cs0_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs0_config, regs->cs[i].config);
			out_be32(&ddr->cs0_config_2, regs->cs[i].config_2);

		} else if (i == 1) {
			out_be32(&ddr->cs1_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs1_config, regs->cs[i].config);
			out_be32(&ddr->cs1_config_2, regs->cs[i].config_2);

		} else if (i == 2) {
			out_be32(&ddr->cs2_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs2_config, regs->cs[i].config);
			out_be32(&ddr->cs2_config_2, regs->cs[i].config_2);

		} else if (i == 3) {
			out_be32(&ddr->cs3_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs3_config, regs->cs[i].config);
			out_be32(&ddr->cs3_config_2, regs->cs[i].config_2);
		}
	}

	out_be32(&ddr->timing_cfg_3, regs->timing_cfg_3);
	out_be32(&ddr->timing_cfg_0, regs->timing_cfg_0);
	out_be32(&ddr->timing_cfg_1, regs->timing_cfg_1);
	out_be32(&ddr->timing_cfg_2, regs->timing_cfg_2);
	out_be32(&ddr->sdram_mode, regs->ddr_sdram_mode);
	out_be32(&ddr->sdram_mode_2, regs->ddr_sdram_mode_2);
	out_be32(&ddr->sdram_mode_3, regs->ddr_sdram_mode_3);
	out_be32(&ddr->sdram_mode_4, regs->ddr_sdram_mode_4);
	out_be32(&ddr->sdram_mode_5, regs->ddr_sdram_mode_5);
	out_be32(&ddr->sdram_mode_6, regs->ddr_sdram_mode_6);
	out_be32(&ddr->sdram_mode_7, regs->ddr_sdram_mode_7);
	out_be32(&ddr->sdram_mode_8, regs->ddr_sdram_mode_8);
	out_be32(&ddr->sdram_md_cntl, regs->ddr_sdram_md_cntl);
	out_be32(&ddr->sdram_interval, regs->ddr_sdram_interval);
	out_be32(&ddr->sdram_data_init, regs->ddr_data_init);
	out_be32(&ddr->sdram_clk_cntl, regs->ddr_sdram_clk_cntl);
	out_be32(&ddr->timing_cfg_4, regs->timing_cfg_4);
	out_be32(&ddr->timing_cfg_5, regs->timing_cfg_5);
	out_be32(&ddr->ddr_zq_cntl, regs->ddr_zq_cntl);
	out_be32(&ddr->ddr_wrlvl_cntl, regs->ddr_wrlvl_cntl);
#ifndef CONFIG_SYS_FSL_DDR_EMU
	/*
	 * Skip these two registers if running on emulator
	 * because emulator doesn't have skew between bytes.
	 */

	if (regs->ddr_wrlvl_cntl_2)
		out_be32(&ddr->ddr_wrlvl_cntl_2, regs->ddr_wrlvl_cntl_2);
	if (regs->ddr_wrlvl_cntl_3)
		out_be32(&ddr->ddr_wrlvl_cntl_3, regs->ddr_wrlvl_cntl_3);
#endif

	out_be32(&ddr->ddr_sr_cntr, regs->ddr_sr_cntr);
	out_be32(&ddr->ddr_sdram_rcw_1, regs->ddr_sdram_rcw_1);
	out_be32(&ddr->ddr_sdram_rcw_2, regs->ddr_sdram_rcw_2);
	out_be32(&ddr->ddr_cdr1, regs->ddr_cdr1);
#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		out_be32(&ddr->sdram_cfg_2,
			 regs->ddr_sdram_cfg_2 & ~SDRAM_CFG2_D_INIT);
		out_be32(&ddr->init_addr, CONFIG_SYS_SDRAM_BASE);
		out_be32(&ddr->init_ext_addr, DDR_INIT_ADDR_EXT_UIA);

		/* DRAM VRef will not be trained */
		out_be32(&ddr->ddr_cdr2,
			 regs->ddr_cdr2 & ~DDR_CDR2_VREF_TRAIN_EN);
	} else
#endif
	{
		out_be32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2);
		out_be32(&ddr->init_addr, regs->ddr_init_addr);
		out_be32(&ddr->init_ext_addr, regs->ddr_init_ext_addr);
		out_be32(&ddr->ddr_cdr2, regs->ddr_cdr2);
	}
	out_be32(&ddr->err_disable, regs->err_disable);
	out_be32(&ddr->err_int_en, regs->err_int_en);
	for (i = 0; i < 32; i++) {
		if (regs->debug[i]) {
			debug("Write to debug_%d as %08x\n", i+1, regs->debug[i]);
			out_be32(&ddr->debug[i], regs->debug[i]);
		}
	}

#ifdef CONFIG_SYS_FSL_ERRATUM_DDR_A003474
	out_be32(&ddr->debug[12], 0x00000015);
	out_be32(&ddr->debug[21], 0x24000000);
#endif /* CONFIG_SYS_FSL_ERRATUM_DDR_A003474 */

	/*
	 * For RDIMMs, JEDEC spec requires clocks to be stable before reset is
	 * deasserted. Clocks start when any chip select is enabled and clock
	 * control register is set. Because all DDR components are connected to
	 * one reset signal, this needs to be done in two steps. Step 1 is to
	 * get the clocks started. Step 2 resumes after reset signal is
	 * deasserted.
	 */
	if (step == 1) {
		udelay(200);
		return;
	}

step2:
	/* Set, but do not enable the memory */
	temp_sdram_cfg = regs->ddr_sdram_cfg;
	temp_sdram_cfg &= ~(SDRAM_CFG_MEM_EN);
	out_be32(&ddr->sdram_cfg, temp_sdram_cfg);
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR_A003
	debug("Workaround for ERRATUM_DDR_A003\n");
	if (regs->ddr_sdram_rcw_2 & 0x00f00000) {
		out_be32(&ddr->timing_cfg_2, regs->timing_cfg_2 & 0xf07fffff);
		out_be32(&ddr->debug[2], 0x00000400);
		out_be32(&ddr->ddr_zq_cntl, regs->ddr_zq_cntl & 0x7fffffff);
		out_be32(&ddr->ddr_wrlvl_cntl, regs->ddr_wrlvl_cntl & 0x7fffffff);
		out_be32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2 & 0xffffffeb);
		out_be32(&ddr->mtcr, 0);
		save1 = in_be32(&ddr->debug[12]);
		save2 = in_be32(&ddr->debug[21]);
		out_be32(&ddr->debug[12], 0x00000015);
		out_be32(&ddr->debug[21], 0x24000000);
		out_be32(&ddr->sdram_interval, regs->ddr_sdram_interval & 0xffff);
		out_be32(&ddr->sdram_cfg, temp_sdram_cfg | SDRAM_CFG_BI | SDRAM_CFG_MEM_EN);

		asm volatile("sync;isync");
		while (!(in_be32(&ddr->debug[1]) & 0x2))
			;

		switch (regs->ddr_sdram_rcw_2 & 0x00f00000) {
		case 0x00000000:
			out_be32(&ddr->sdram_md_cntl,
				MD_CNTL_MD_EN		|
				MD_CNTL_CS_SEL_CS0_CS1	|
				0x04000000		|
				MD_CNTL_WRCW		|
				MD_CNTL_MD_VALUE(0x02));
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if (!(regs->cs[2].config & SDRAM_CS_CONFIG_EN))
				break;
			while (in_be32(&ddr->sdram_md_cntl) & MD_CNTL_MD_EN)
				;
			out_be32(&ddr->sdram_md_cntl,
				 MD_CNTL_MD_EN		|
				 MD_CNTL_CS_SEL_CS2_CS3	|
				 0x04000000		|
				 MD_CNTL_WRCW		|
				 MD_CNTL_MD_VALUE(0x02));
#endif
			break;
		case 0x00100000:
			out_be32(&ddr->sdram_md_cntl,
				MD_CNTL_MD_EN		|
				MD_CNTL_CS_SEL_CS0_CS1	|
				0x04000000		|
				MD_CNTL_WRCW		|
				MD_CNTL_MD_VALUE(0x0a));
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if (!(regs->cs[2].config & SDRAM_CS_CONFIG_EN))
				break;
			while (in_be32(&ddr->sdram_md_cntl) & MD_CNTL_MD_EN)
				;
			out_be32(&ddr->sdram_md_cntl,
				 MD_CNTL_MD_EN		|
				 MD_CNTL_CS_SEL_CS2_CS3	|
				 0x04000000		|
				 MD_CNTL_WRCW		|
				 MD_CNTL_MD_VALUE(0x0a));
#endif
			break;
		case 0x00200000:
			out_be32(&ddr->sdram_md_cntl,
				MD_CNTL_MD_EN		|
				MD_CNTL_CS_SEL_CS0_CS1	|
				0x04000000		|
				MD_CNTL_WRCW		|
				MD_CNTL_MD_VALUE(0x12));
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if (!(regs->cs[2].config & SDRAM_CS_CONFIG_EN))
				break;
			while (in_be32(&ddr->sdram_md_cntl) & MD_CNTL_MD_EN)
				;
			out_be32(&ddr->sdram_md_cntl,
				 MD_CNTL_MD_EN		|
				 MD_CNTL_CS_SEL_CS2_CS3	|
				 0x04000000		|
				 MD_CNTL_WRCW		|
				 MD_CNTL_MD_VALUE(0x12));
#endif
			break;
		case 0x00300000:
			out_be32(&ddr->sdram_md_cntl,
				MD_CNTL_MD_EN		|
				MD_CNTL_CS_SEL_CS0_CS1	|
				0x04000000		|
				MD_CNTL_WRCW		|
				MD_CNTL_MD_VALUE(0x1a));
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if (!(regs->cs[2].config & SDRAM_CS_CONFIG_EN))
				break;
			while (in_be32(&ddr->sdram_md_cntl) & MD_CNTL_MD_EN)
				;
			out_be32(&ddr->sdram_md_cntl,
				 MD_CNTL_MD_EN		|
				 MD_CNTL_CS_SEL_CS2_CS3	|
				 0x04000000		|
				 MD_CNTL_WRCW		|
				 MD_CNTL_MD_VALUE(0x1a));
#endif
			break;
		default:
			out_be32(&ddr->sdram_md_cntl,
				MD_CNTL_MD_EN		|
				MD_CNTL_CS_SEL_CS0_CS1	|
				0x04000000		|
				MD_CNTL_WRCW		|
				MD_CNTL_MD_VALUE(0x02));
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if (!(regs->cs[2].config & SDRAM_CS_CONFIG_EN))
				break;
			while (in_be32(&ddr->sdram_md_cntl) & MD_CNTL_MD_EN)
				;
			out_be32(&ddr->sdram_md_cntl,
				 MD_CNTL_MD_EN		|
				 MD_CNTL_CS_SEL_CS2_CS3	|
				 0x04000000		|
				 MD_CNTL_WRCW		|
				 MD_CNTL_MD_VALUE(0x02));
#endif
			printf("Unsupported RC10\n");
			break;
		}

		while (in_be32(&ddr->sdram_md_cntl) & 0x80000000)
			;
		udelay(6);
		out_be32(&ddr->sdram_cfg, temp_sdram_cfg);
		out_be32(&ddr->timing_cfg_2, regs->timing_cfg_2);
		out_be32(&ddr->debug[2], 0x0);
		out_be32(&ddr->ddr_zq_cntl, regs->ddr_zq_cntl);
		out_be32(&ddr->ddr_wrlvl_cntl, regs->ddr_wrlvl_cntl);
		out_be32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2);
		out_be32(&ddr->debug[12], save1);
		out_be32(&ddr->debug[21], save2);
		out_be32(&ddr->sdram_interval, regs->ddr_sdram_interval);

	}
#endif
	/*
	 * For 8572 DDR1 erratum - DDR controller may enter illegal state
	 * when operatiing in 32-bit bus mode with 4-beat bursts,
	 * This erratum does not affect DDR3 mode, only for DDR2 mode.
	 */
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR_115
	debug("Workaround for ERRATUM_DDR_115\n");
	if ((((in_be32(&ddr->sdram_cfg) >> 24) & 0x7) == SDRAM_TYPE_DDR2)
	    && in_be32(&ddr->sdram_cfg) & 0x80000) {
		/* set DEBUG_1[31] */
		setbits_be32(&ddr->debug[0], 1);
	}
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR111_DDR134
	debug("Workaround for ERRATUM_DDR111_DDR134\n");
	/*
	 * This is the combined workaround for DDR111 and DDR134
	 * following the published errata for MPC8572
	 */

	/* 1. Set EEBACR[3] */
	setbits_be32(&ecm->eebacr, 0x10000000);
	debug("Setting EEBACR[3] to 0x%08x\n", in_be32(&ecm->eebacr));

	/* 2. Set DINIT in SDRAM_CFG_2*/
	setbits_be32(&ddr->sdram_cfg_2, SDRAM_CFG2_D_INIT);
	debug("Setting sdram_cfg_2[D_INIT] to 0x%08x\n",
		in_be32(&ddr->sdram_cfg_2));

	/* 3. Set DEBUG_3[21] */
	setbits_be32(&ddr->debug[2], 0x400);
	debug("Setting DEBUG_3[21] to 0x%08x\n", in_be32(&ddr->debug[2]));

#endif	/* part 1 of the workaound */

	/*
	 * 500 painful micro-seconds must elapse between
	 * the DDR clock setup and the DDR config enable.
	 * DDR2 need 200 us, and DDR3 need 500 us from spec,
	 * we choose the max, that is 500 us for all of case.
	 */
	udelay(500);
	asm volatile("sync;isync");

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* enter self-refresh */
		setbits_be32(&ddr->sdram_cfg_2, SDRAM_CFG2_FRC_SR);
		/* do board specific memory setup */
		board_mem_sleep_setup();
		temp_sdram_cfg = (in_be32(&ddr->sdram_cfg) | SDRAM_CFG_BI);
	} else
#endif
		temp_sdram_cfg = (in_be32(&ddr->sdram_cfg) & ~SDRAM_CFG_BI);

	/* Let the controller go */
	out_be32(&ddr->sdram_cfg, temp_sdram_cfg | SDRAM_CFG_MEM_EN);
	asm volatile("sync;isync");

	total_gb_size_per_controller = 0;
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (!(regs->cs[i].config & 0x80000000))
			continue;
		total_gb_size_per_controller += 1 << (
			((regs->cs[i].config >> 14) & 0x3) + 2 +
			((regs->cs[i].config >> 8) & 0x7) + 12 +
			((regs->cs[i].config >> 0) & 0x7) + 8 +
			3 - ((regs->ddr_sdram_cfg >> 19) & 0x3) -
			26);			/* minus 26 (count of 64M) */
	}
	if (fsl_ddr_get_intl3r() & 0x80000000)	/* 3-way interleaving */
		total_gb_size_per_controller *= 3;
	else if (regs->cs[0].config & 0x20000000) /* 2-way interleaving */
		total_gb_size_per_controller <<= 1;
	/*
	 * total memory / bus width = transactions needed
	 * transactions needed / data rate = seconds
	 * to add plenty of buffer, double the time
	 * For example, 2GB on 666MT/s 64-bit bus takes about 402ms
	 * Let's wait for 800ms
	 */
	bus_width = 3 - ((ddr->sdram_cfg & SDRAM_CFG_DBW_MASK)
			>> SDRAM_CFG_DBW_SHIFT);
	timeout = ((total_gb_size_per_controller << (6 - bus_width)) * 100 /
		(get_ddr_freq(ctrl_num) >> 20)) << 1;
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR111_DDR134
	timeout_save = timeout;
#endif
	total_gb_size_per_controller >>= 4;	/* shift down to gb size */
	debug("total %d GB\n", total_gb_size_per_controller);
	debug("Need to wait up to %d * 10ms\n", timeout);

	/* Poll DDR_SDRAM_CFG_2[D_INIT] bit until auto-data init is done.  */
	while ((in_be32(&ddr->sdram_cfg_2) & SDRAM_CFG2_D_INIT) &&
		(timeout >= 0)) {
		udelay(10000);		/* throttle polling rate */
		timeout--;
	}

	if (timeout <= 0)
		printf("Waiting for D_INIT timeout. Memory may not work.\n");

#ifdef CONFIG_SYS_FSL_ERRATUM_DDR111_DDR134
	/* continue this workaround */

	/* 4. Clear DEBUG3[21] */
	clrbits_be32(&ddr->debug[2], 0x400);
	debug("Clearing D3[21] to 0x%08x\n", in_be32(&ddr->debug[2]));

	/* DDR134 workaround starts */
	/* A: Clear sdram_cfg_2[odt_cfg] */
	clrbits_be32(&ddr->sdram_cfg_2, SDRAM_CFG2_ODT_CFG_MASK);
	debug("Clearing SDRAM_CFG2[ODT_CFG] to 0x%08x\n",
		in_be32(&ddr->sdram_cfg_2));

	/* B: Set DEBUG1[15] */
	setbits_be32(&ddr->debug[0], 0x10000);
	debug("Setting D1[15] to 0x%08x\n", in_be32(&ddr->debug[0]));

	/* C: Set timing_cfg_2[cpo] to 0b11111 */
	setbits_be32(&ddr->timing_cfg_2, TIMING_CFG_2_CPO_MASK);
	debug("Setting TMING_CFG_2[CPO] to 0x%08x\n",
		in_be32(&ddr->timing_cfg_2));

	/* D: Set D6 to 0x9f9f9f9f */
	out_be32(&ddr->debug[5], 0x9f9f9f9f);
	debug("Setting D6 to 0x%08x\n", in_be32(&ddr->debug[5]));

	/* E: Set D7 to 0x9f9f9f9f */
	out_be32(&ddr->debug[6], 0x9f9f9f9f);
	debug("Setting D7 to 0x%08x\n", in_be32(&ddr->debug[6]));

	/* F: Set D2[20] */
	setbits_be32(&ddr->debug[1], 0x800);
	debug("Setting D2[20] to 0x%08x\n", in_be32(&ddr->debug[1]));

	/* G: Poll on D2[20] until cleared */
	while (in_be32(&ddr->debug[1]) & 0x800)
		udelay(10000);          /* throttle polling rate */

	/* H: Clear D1[15] */
	clrbits_be32(&ddr->debug[0], 0x10000);
	debug("Setting D1[15] to 0x%08x\n", in_be32(&ddr->debug[0]));

	/* I: Set sdram_cfg_2[odt_cfg] */
	setbits_be32(&ddr->sdram_cfg_2,
		regs->ddr_sdram_cfg_2 & SDRAM_CFG2_ODT_CFG_MASK);
	debug("Setting sdram_cfg_2 to 0x%08x\n", in_be32(&ddr->sdram_cfg_2));

	/* Continuing with the DDR111 workaround */
	/* 5. Set D2[21] */
	setbits_be32(&ddr->debug[1], 0x400);
	debug("Setting D2[21] to 0x%08x\n", in_be32(&ddr->debug[1]));

	/* 6. Poll D2[21] until its cleared */
	while (in_be32(&ddr->debug[1]) & 0x400)
		udelay(10000);          /* throttle polling rate */

	/* 7. Wait for state machine 2nd run, roughly 400ms/GB */
	debug("Wait for %d * 10ms\n", timeout_save);
	udelay(timeout_save * 10000);

	/* 8. Set sdram_cfg_2[dinit] if options requires */
	setbits_be32(&ddr->sdram_cfg_2,
		regs->ddr_sdram_cfg_2 & SDRAM_CFG2_D_INIT);
	debug("Setting sdram_cfg_2 to 0x%08x\n", in_be32(&ddr->sdram_cfg_2));

	/* 9. Poll until dinit is cleared */
	timeout = timeout_save;
	debug("Need to wait up to %d * 10ms\n", timeout);
	while ((in_be32(&ddr->sdram_cfg_2) & SDRAM_CFG2_D_INIT) &&
		(timeout >= 0)) {
		udelay(10000);		/* throttle polling rate */
		timeout--;
	}

	if (timeout <= 0)
		printf("Waiting for D_INIT timeout. Memory may not work.\n");

	/* 10. Clear EEBACR[3] */
	clrbits_be32(&ecm->eebacr, 10000000);
	debug("Clearing EEBACR[3] to 0x%08x\n", in_be32(&ecm->eebacr));

	if (csn != -1) {
		csn_bnds_t = (unsigned int *) &regs->cs[csn].bnds;
		*csn_bnds_t = csn_bnds_backup;
		debug("Change cs%d_bnds back to 0x%08x\n",
			csn, regs->cs[csn].bnds);
		setbits_be32(&ddr->sdram_cfg, 0x2);	/* MEM_HALT */
		switch (csn) {
		case 0:
			out_be32(&ddr->cs0_bnds, regs->cs[csn].bnds);
			break;
		case 1:
			out_be32(&ddr->cs1_bnds, regs->cs[csn].bnds);
			break;
#if CONFIG_CHIP_SELECTS_PER_CTRL > 2
		case 2:
			out_be32(&ddr->cs2_bnds, regs->cs[csn].bnds);
			break;
		case 3:
			out_be32(&ddr->cs3_bnds, regs->cs[csn].bnds);
			break;
#endif
		}
		clrbits_be32(&ddr->sdram_cfg, 0x2);
	}
#endif /* CONFIG_SYS_FSL_ERRATUM_DDR111_DDR134 */
#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot())
		/* exit self-refresh */
		clrbits_be32(&ddr->sdram_cfg_2, SDRAM_CFG2_FRC_SR);
#endif
}
