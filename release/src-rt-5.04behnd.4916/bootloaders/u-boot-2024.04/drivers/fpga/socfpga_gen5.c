// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 * All rights reserved.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>

#define FPGA_TIMEOUT_CNT	0x1000000

static struct socfpga_fpga_manager *fpgamgr_regs =
	(struct socfpga_fpga_manager *)SOCFPGA_FPGAMGRREGS_ADDRESS;

/* Set CD ratio */
static void fpgamgr_set_cd_ratio(unsigned long ratio)
{
	clrsetbits_le32(&fpgamgr_regs->ctrl,
			0x3 << FPGAMGRREGS_CTRL_CDRATIO_LSB,
			(ratio & 0x3) << FPGAMGRREGS_CTRL_CDRATIO_LSB);
}

/* Start the FPGA programming by initialize the FPGA Manager */
static int fpgamgr_program_init(void)
{
	unsigned long msel, i;

	/* Get the MSEL value */
	msel = readl(&fpgamgr_regs->stat);
	msel &= FPGAMGRREGS_STAT_MSEL_MASK;
	msel >>= FPGAMGRREGS_STAT_MSEL_LSB;

	/*
	 * Set the cfg width
	 * If MSEL[3] = 1, cfg width = 32 bit
	 */
	if (msel & 0x8) {
		setbits_le32(&fpgamgr_regs->ctrl,
			     FPGAMGRREGS_CTRL_CFGWDTH_MASK);

		/* To determine the CD ratio */
		/* MSEL[1:0] = 0, CD Ratio = 1 */
		if ((msel & 0x3) == 0x0)
			fpgamgr_set_cd_ratio(CDRATIO_x1);
		/* MSEL[1:0] = 1, CD Ratio = 4 */
		else if ((msel & 0x3) == 0x1)
			fpgamgr_set_cd_ratio(CDRATIO_x4);
		/* MSEL[1:0] = 2, CD Ratio = 8 */
		else if ((msel & 0x3) == 0x2)
			fpgamgr_set_cd_ratio(CDRATIO_x8);

	} else {	/* MSEL[3] = 0 */
		clrbits_le32(&fpgamgr_regs->ctrl,
			     FPGAMGRREGS_CTRL_CFGWDTH_MASK);

		/* To determine the CD ratio */
		/* MSEL[1:0] = 0, CD Ratio = 1 */
		if ((msel & 0x3) == 0x0)
			fpgamgr_set_cd_ratio(CDRATIO_x1);
		/* MSEL[1:0] = 1, CD Ratio = 2 */
		else if ((msel & 0x3) == 0x1)
			fpgamgr_set_cd_ratio(CDRATIO_x2);
		/* MSEL[1:0] = 2, CD Ratio = 4 */
		else if ((msel & 0x3) == 0x2)
			fpgamgr_set_cd_ratio(CDRATIO_x4);
	}

	/* To enable FPGA Manager configuration */
	clrbits_le32(&fpgamgr_regs->ctrl, FPGAMGRREGS_CTRL_NCE_MASK);

	/* To enable FPGA Manager drive over configuration line */
	setbits_le32(&fpgamgr_regs->ctrl, FPGAMGRREGS_CTRL_EN_MASK);

	/* Put FPGA into reset phase */
	setbits_le32(&fpgamgr_regs->ctrl, FPGAMGRREGS_CTRL_NCONFIGPULL_MASK);

	/* (1) wait until FPGA enter reset phase */
	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		if (fpgamgr_get_mode() == FPGAMGRREGS_MODE_RESETPHASE)
			break;
	}

	/* If not in reset state, return error */
	if (fpgamgr_get_mode() != FPGAMGRREGS_MODE_RESETPHASE) {
		puts("FPGA: Could not reset\n");
		return -1;
	}

	/* Release FPGA from reset phase */
	clrbits_le32(&fpgamgr_regs->ctrl, FPGAMGRREGS_CTRL_NCONFIGPULL_MASK);

	/* (2) wait until FPGA enter configuration phase */
	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		if (fpgamgr_get_mode() == FPGAMGRREGS_MODE_CFGPHASE)
			break;
	}

	/* If not in configuration state, return error */
	if (fpgamgr_get_mode() != FPGAMGRREGS_MODE_CFGPHASE) {
		puts("FPGA: Could not configure\n");
		return -2;
	}

	/* Clear all interrupts in CB Monitor */
	writel(0xFFF, &fpgamgr_regs->gpio_porta_eoi);

	/* Enable AXI configuration */
	setbits_le32(&fpgamgr_regs->ctrl, FPGAMGRREGS_CTRL_AXICFGEN_MASK);

	return 0;
}

/* Ensure the FPGA entering config done */
static int fpgamgr_program_poll_cd(void)
{
	const uint32_t mask = FPGAMGRREGS_MON_GPIO_EXT_PORTA_NS_MASK |
			      FPGAMGRREGS_MON_GPIO_EXT_PORTA_CD_MASK;
	unsigned long reg, i;

	/* (3) wait until full config done */
	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		reg = readl(&fpgamgr_regs->gpio_ext_porta);

		/* Config error */
		if (!(reg & mask)) {
			printf("FPGA: Configuration error.\n");
			return -3;
		}

		/* Config done without error */
		if (reg & mask)
			break;
	}

	/* Timeout happened, return error */
	if (i == FPGA_TIMEOUT_CNT) {
		printf("FPGA: Timeout waiting for program.\n");
		return -4;
	}

	/* Disable AXI configuration */
	clrbits_le32(&fpgamgr_regs->ctrl, FPGAMGRREGS_CTRL_AXICFGEN_MASK);

	return 0;
}

/* Ensure the FPGA entering init phase */
static int fpgamgr_program_poll_initphase(void)
{
	unsigned long i;

	/* Additional clocks for the CB to enter initialization phase */
	if (fpgamgr_dclkcnt_set(0x4))
		return -5;

	/* (4) wait until FPGA enter init phase or user mode */
	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		if (fpgamgr_get_mode() == FPGAMGRREGS_MODE_INITPHASE)
			break;
		if (fpgamgr_get_mode() == FPGAMGRREGS_MODE_USERMODE)
			break;
	}

	/* If not in configuration state, return error */
	if (i == FPGA_TIMEOUT_CNT)
		return -6;

	return 0;
}

/* Ensure the FPGA entering user mode */
static int fpgamgr_program_poll_usermode(void)
{
	unsigned long i;

	/* Additional clocks for the CB to exit initialization phase */
	if (fpgamgr_dclkcnt_set(0x5000))
		return -7;

	/* (5) wait until FPGA enter user mode */
	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		if (fpgamgr_get_mode() == FPGAMGRREGS_MODE_USERMODE)
			break;
	}
	/* If not in configuration state, return error */
	if (i == FPGA_TIMEOUT_CNT)
		return -8;

	/* To release FPGA Manager drive over configuration line */
	clrbits_le32(&fpgamgr_regs->ctrl, FPGAMGRREGS_CTRL_EN_MASK);

	return 0;
}

/*
 * FPGA Manager to program the FPGA. This is the interface used by FPGA driver.
 * Return 0 for sucess, non-zero for error.
 */
int socfpga_load(Altera_desc *desc, const void *rbf_data, size_t rbf_size)
{
	int status;

	if ((uint32_t)rbf_data & 0x3) {
		puts("FPGA: Unaligned data, realign to 32bit boundary.\n");
		return -EINVAL;
	}

	/* Prior programming the FPGA, all bridges need to be shut off */

	/* Disable all signals from hps peripheral controller to fpga */
	writel(0, socfpga_get_sysmgr_addr() + SYSMGR_GEN5_FPGAINFGRP_MODULE);

	/* Disable all signals from FPGA to HPS SDRAM */
#define SDR_CTRLGRP_FPGAPORTRST_ADDRESS	0x5080
	writel(0, SOCFPGA_SDR_ADDRESS + SDR_CTRLGRP_FPGAPORTRST_ADDRESS);

	/* Disable all axi bridge (hps2fpga, lwhps2fpga & fpga2hps) */
	socfpga_bridges_reset(1);

	/* Unmap the bridges from NIC-301 */
	writel(0x1, SOCFPGA_L3REGS_ADDRESS);

	/* Initialize the FPGA Manager */
	status = fpgamgr_program_init();
	if (status)
		return status;

	/* Write the RBF data to FPGA Manager */
	fpgamgr_program_write(rbf_data, rbf_size);

	/* Ensure the FPGA entering config done */
	status = fpgamgr_program_poll_cd();
	if (status)
		return status;

	/* Ensure the FPGA entering init phase */
	status = fpgamgr_program_poll_initphase();
	if (status)
		return status;

	/* Ensure the FPGA entering user mode */
	return fpgamgr_program_poll_usermode();
}
