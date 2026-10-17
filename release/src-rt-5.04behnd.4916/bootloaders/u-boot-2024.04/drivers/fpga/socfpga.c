// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 * All rights reserved.
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

int fpgamgr_dclkcnt_set(unsigned long cnt)
{
	unsigned long i;

	/* Clear any existing done status */
	if (readl(&fpgamgr_regs->dclkstat))
		writel(0x1, &fpgamgr_regs->dclkstat);

	/* Write the dclkcnt */
	writel(cnt, &fpgamgr_regs->dclkcnt);

	/* Wait till the dclkcnt done */
	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		if (!readl(&fpgamgr_regs->dclkstat))
			continue;

		writel(0x1, &fpgamgr_regs->dclkstat);
		return 0;
	}

	return -ETIMEDOUT;
}

/* Write the RBF data to FPGA Manager */
void fpgamgr_program_write(const void *rbf_data, size_t rbf_size)
{
	uint32_t src = (uint32_t)rbf_data;
	uint32_t dst = SOCFPGA_FPGAMGRDATA_ADDRESS;

	/* Number of loops for 32-byte long copying. */
	uint32_t loops32 = rbf_size / 32;
	/* Number of loops for 4-byte long copying + trailing bytes */
	uint32_t loops4 = DIV_ROUND_UP(rbf_size % 32, 4);

	asm volatile(
		"	cmp	%2,	#0\n"
		"	beq	2f\n"
		"1:	ldmia	%0!,	{r0-r7}\n"
		"	stmia	%1!,	{r0-r7}\n"
		"	sub	%1,	#32\n"
		"	subs	%2,	#1\n"
		"	bne	1b\n"
		"2:	cmp	%3,	#0\n"
		"	beq	4f\n"
		"3:	ldr	%2,	[%0],	#4\n"
		"	str	%2,	[%1]\n"
		"	subs	%3,	#1\n"
		"	bne	3b\n"
		"4:	nop\n"
		: "+r"(src), "+r"(dst), "+r"(loops32), "+r"(loops4) :
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "cc");
}
