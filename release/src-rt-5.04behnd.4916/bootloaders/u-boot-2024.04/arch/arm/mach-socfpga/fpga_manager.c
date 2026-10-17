// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (C) 2012 Altera Corporation <www.altera.com>
 * All rights reserved.
 *
 * This file contains only support functions used also by the SoCFPGA
 * platform code, the real meat is located in drivers/fpga/socfpga.c .
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>

/* Timeout count */
#define FPGA_TIMEOUT_CNT		0x1000000

static struct socfpga_fpga_manager *fpgamgr_regs =
	(struct socfpga_fpga_manager *)SOCFPGA_FPGAMGRREGS_ADDRESS;

/* Check whether FPGA Init_Done signal is high */
static int is_fpgamgr_initdone_high(void)
{
	unsigned long val;

	val = readl(&fpgamgr_regs->gpio_ext_porta);
	return val & FPGAMGRREGS_MON_GPIO_EXT_PORTA_ID_MASK;
}

/* Get the FPGA mode */
int fpgamgr_get_mode(void)
{
	unsigned long val;

	val = readl(&fpgamgr_regs->stat);
	return val & FPGAMGRREGS_STAT_MODE_MASK;
}

/* Check whether FPGA is ready to be accessed */
int fpgamgr_test_fpga_ready(void)
{
	/* Check for init done signal */
	if (!is_fpgamgr_initdone_high())
		return 0;

	/* Check again to avoid false glitches */
	if (!is_fpgamgr_initdone_high())
		return 0;

	if (fpgamgr_get_mode() != FPGAMGRREGS_MODE_USERMODE)
		return 0;

	return 1;
}

/* Poll until FPGA is ready to be accessed or timeout occurred */
int fpgamgr_poll_fpga_ready(void)
{
	unsigned long i;

	/* If FPGA is blank, wait till WD invoke warm reset */
	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		/* check for init done signal */
		if (!is_fpgamgr_initdone_high())
			continue;
		/* check again to avoid false glitches */
		if (!is_fpgamgr_initdone_high())
			continue;
		return 1;
	}

	return 0;
}
