// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Derived from mpc85xx_ddr_gen3.c, removed all workarounds
 */

#include <common.h>
#include <asm/io.h>
#include <fsl_ddr_sdram.h>
#include <asm/processor.h>
#include <fsl_immap.h>
#include <fsl_ddr.h>
#include <asm/arch/clock.h>

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
		printf("%s unexpected ctrl_num = %u\n", __func__, ctrl_num);
		return;
	}

	if (step == 2)
		goto step2;

	if (regs->ddr_eor)
		ddr_out32(&ddr->eor, regs->ddr_eor);
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i == 0) {
			ddr_out32(&ddr->cs0_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs0_config, regs->cs[i].config);
			ddr_out32(&ddr->cs0_config_2, regs->cs[i].config_2);

		} else if (i == 1) {
			ddr_out32(&ddr->cs1_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs1_config, regs->cs[i].config);
			ddr_out32(&ddr->cs1_config_2, regs->cs[i].config_2);

		} else if (i == 2) {
			ddr_out32(&ddr->cs2_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs2_config, regs->cs[i].config);
			ddr_out32(&ddr->cs2_config_2, regs->cs[i].config_2);

		} else if (i == 3) {
			ddr_out32(&ddr->cs3_bnds, regs->cs[i].bnds);
			ddr_out32(&ddr->cs3_config, regs->cs[i].config);
			ddr_out32(&ddr->cs3_config_2, regs->cs[i].config_2);
		}
	}

	ddr_out32(&ddr->timing_cfg_3, regs->timing_cfg_3);
	ddr_out32(&ddr->timing_cfg_0, regs->timing_cfg_0);
	ddr_out32(&ddr->timing_cfg_1, regs->timing_cfg_1);
	ddr_out32(&ddr->timing_cfg_2, regs->timing_cfg_2);
	ddr_out32(&ddr->sdram_mode, regs->ddr_sdram_mode);
	ddr_out32(&ddr->sdram_mode_2, regs->ddr_sdram_mode_2);
	ddr_out32(&ddr->sdram_mode_3, regs->ddr_sdram_mode_3);
	ddr_out32(&ddr->sdram_mode_4, regs->ddr_sdram_mode_4);
	ddr_out32(&ddr->sdram_mode_5, regs->ddr_sdram_mode_5);
	ddr_out32(&ddr->sdram_mode_6, regs->ddr_sdram_mode_6);
	ddr_out32(&ddr->sdram_mode_7, regs->ddr_sdram_mode_7);
	ddr_out32(&ddr->sdram_mode_8, regs->ddr_sdram_mode_8);
	ddr_out32(&ddr->sdram_md_cntl, regs->ddr_sdram_md_cntl);
	ddr_out32(&ddr->sdram_interval, regs->ddr_sdram_interval);
	ddr_out32(&ddr->sdram_data_init, regs->ddr_data_init);
	ddr_out32(&ddr->sdram_clk_cntl, regs->ddr_sdram_clk_cntl);
	ddr_out32(&ddr->timing_cfg_4, regs->timing_cfg_4);
	ddr_out32(&ddr->timing_cfg_5, regs->timing_cfg_5);
	ddr_out32(&ddr->ddr_zq_cntl, regs->ddr_zq_cntl);
	ddr_out32(&ddr->ddr_wrlvl_cntl, regs->ddr_wrlvl_cntl);
#ifndef CONFIG_SYS_FSL_DDR_EMU
	/*
	 * Skip these two registers if running on emulator
	 * because emulator doesn't have skew between bytes.
	 */

	if (regs->ddr_wrlvl_cntl_2)
		ddr_out32(&ddr->ddr_wrlvl_cntl_2, regs->ddr_wrlvl_cntl_2);
	if (regs->ddr_wrlvl_cntl_3)
		ddr_out32(&ddr->ddr_wrlvl_cntl_3, regs->ddr_wrlvl_cntl_3);
#endif

	ddr_out32(&ddr->ddr_sr_cntr, regs->ddr_sr_cntr);
	ddr_out32(&ddr->ddr_sdram_rcw_1, regs->ddr_sdram_rcw_1);
	ddr_out32(&ddr->ddr_sdram_rcw_2, regs->ddr_sdram_rcw_2);
	ddr_out32(&ddr->ddr_cdr1, regs->ddr_cdr1);
#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		ddr_out32(&ddr->sdram_cfg_2,
			  regs->ddr_sdram_cfg_2 & ~SDRAM_CFG2_D_INIT);
		ddr_out32(&ddr->init_addr, CONFIG_SYS_SDRAM_BASE);
		ddr_out32(&ddr->init_ext_addr, DDR_INIT_ADDR_EXT_UIA);

		/* DRAM VRef will not be trained */
		ddr_out32(&ddr->ddr_cdr2,
			  regs->ddr_cdr2 & ~DDR_CDR2_VREF_TRAIN_EN);
	} else
#endif
	{
		ddr_out32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2);
		ddr_out32(&ddr->init_addr, regs->ddr_init_addr);
		ddr_out32(&ddr->init_ext_addr, regs->ddr_init_ext_addr);
		ddr_out32(&ddr->ddr_cdr2, regs->ddr_cdr2);
	}
	ddr_out32(&ddr->err_disable, regs->err_disable);
	ddr_out32(&ddr->err_int_en, regs->err_int_en);
	for (i = 0; i < 32; i++) {
		if (regs->debug[i]) {
			debug("Write to debug_%d as %08x\n", i + 1,
			      regs->debug[i]);
			ddr_out32(&ddr->debug[i], regs->debug[i]);
		}
	}

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
	ddr_out32(&ddr->sdram_cfg, temp_sdram_cfg);

	/*
	 * 500 painful micro-seconds must elapse between
	 * the DDR clock setup and the DDR config enable.
	 * DDR2 need 200 us, and DDR3 need 500 us from spec,
	 * we choose the max, that is 500 us for all of case.
	 */
	udelay(500);
	asm volatile("dsb sy;isb");

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* enter self-refresh */
		temp_sdram_cfg = ddr_in32(&ddr->sdram_cfg_2);
		temp_sdram_cfg |= SDRAM_CFG2_FRC_SR;
		ddr_out32(&ddr->sdram_cfg_2, temp_sdram_cfg);
		/* do board specific memory setup */
		board_mem_sleep_setup();

		temp_sdram_cfg = (ddr_in32(&ddr->sdram_cfg) | SDRAM_CFG_BI);
	} else
#endif
		temp_sdram_cfg = ddr_in32(&ddr->sdram_cfg) & ~SDRAM_CFG_BI;
	/* Let the controller go */
	ddr_out32(&ddr->sdram_cfg, temp_sdram_cfg | SDRAM_CFG_MEM_EN);
	asm volatile("dsb sy;isb");

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
	if (regs->cs[0].config & 0x20000000) {
		/* 2-way interleaving */
		total_gb_size_per_controller <<= 1;
	}
	/*
	 * total memory / bus width = transactions needed
	 * transactions needed / data rate = seconds
	 * to add plenty of buffer, double the time
	 * For example, 2GB on 666MT/s 64-bit bus takes about 402ms
	 * Let's wait for 800ms
	 */
	bus_width = 3 - ((ddr_in32(&ddr->sdram_cfg) & SDRAM_CFG_DBW_MASK)
			>> SDRAM_CFG_DBW_SHIFT);
	timeout = ((total_gb_size_per_controller << (6 - bus_width)) * 100 /
		(get_ddr_freq(ctrl_num) >> 20)) << 1;
	total_gb_size_per_controller >>= 4;	/* shift down to gb size */
	debug("total %d GB\n", total_gb_size_per_controller);
	debug("Need to wait up to %d * 10ms\n", timeout);

	/* Poll DDR_SDRAM_CFG_2[D_INIT] bit until auto-data init is done.  */
	while ((ddr_in32(&ddr->sdram_cfg_2) & SDRAM_CFG2_D_INIT) &&
		(timeout >= 0)) {
		udelay(10000);		/* throttle polling rate */
		timeout--;
	}

	if (timeout <= 0)
		printf("Waiting for D_INIT timeout. Memory may not work.\n");
#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* exit self-refresh */
		temp_sdram_cfg = ddr_in32(&ddr->sdram_cfg_2);
		temp_sdram_cfg &= ~SDRAM_CFG2_FRC_SR;
		ddr_out32(&ddr->sdram_cfg_2, temp_sdram_cfg);
	}
#endif
}
