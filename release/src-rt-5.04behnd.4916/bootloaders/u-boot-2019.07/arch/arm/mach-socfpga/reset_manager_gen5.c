// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 */


#include <common.h>
#include <asm/io.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>

static const struct socfpga_reset_manager *reset_manager_base =
		(void *)SOCFPGA_RSTMGR_ADDRESS;
static const struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

/* Assert or de-assert SoCFPGA reset manager reset. */
void socfpga_per_reset(u32 reset, int set)
{
	const u32 *reg;
	u32 rstmgr_bank = RSTMGR_BANK(reset);

	switch (rstmgr_bank) {
	case 0:
		reg = &reset_manager_base->mpu_mod_reset;
		break;
	case 1:
		reg = &reset_manager_base->per_mod_reset;
		break;
	case 2:
		reg = &reset_manager_base->per2_mod_reset;
		break;
	case 3:
		reg = &reset_manager_base->brg_mod_reset;
		break;
	case 4:
		reg = &reset_manager_base->misc_mod_reset;
		break;

	default:
		return;
	}

	if (set)
		setbits_le32(reg, 1 << RSTMGR_RESET(reset));
	else
		clrbits_le32(reg, 1 << RSTMGR_RESET(reset));
}

/*
 * Assert reset on every peripheral but L4WD0.
 * Watchdog must be kept intact to prevent glitches
 * and/or hangs.
 */
void socfpga_per_reset_all(void)
{
	const u32 l4wd0 = 1 << RSTMGR_RESET(SOCFPGA_RESET(L4WD0));

	writel(~l4wd0, &reset_manager_base->per_mod_reset);
	writel(0xffffffff, &reset_manager_base->per2_mod_reset);
}

#define L3REGS_REMAP_LWHPS2FPGA_MASK	0x10
#define L3REGS_REMAP_HPS2FPGA_MASK	0x08
#define L3REGS_REMAP_OCRAM_MASK		0x01

void socfpga_bridges_set_handoff_regs(bool h2f, bool lwh2f, bool f2h)
{
	u32 brgmask = 0x0;
	u32 l3rmask = L3REGS_REMAP_OCRAM_MASK;

	if (h2f)
		brgmask |= BIT(0);
	else
		l3rmask |= L3REGS_REMAP_HPS2FPGA_MASK;

	if (lwh2f)
		brgmask |= BIT(1);
	else
		l3rmask |= L3REGS_REMAP_LWHPS2FPGA_MASK;

	if (f2h)
		brgmask |= BIT(2);

	writel(brgmask, &sysmgr_regs->iswgrp_handoff[0]);
	writel(l3rmask, &sysmgr_regs->iswgrp_handoff[1]);
}

void socfpga_bridges_reset(int enable)
{
	const u32 l3mask = L3REGS_REMAP_LWHPS2FPGA_MASK |
				L3REGS_REMAP_HPS2FPGA_MASK |
				L3REGS_REMAP_OCRAM_MASK;

	if (enable) {
		/* brdmodrst */
		writel(0x7, &reset_manager_base->brg_mod_reset);
		writel(L3REGS_REMAP_OCRAM_MASK, SOCFPGA_L3REGS_ADDRESS);
	} else {
		socfpga_bridges_set_handoff_regs(false, false, false);

		/* Check signal from FPGA. */
		if (!fpgamgr_test_fpga_ready()) {
			/* FPGA not ready, do nothing. We allow system to boot
			 * without FPGA ready. So, return 0 instead of error. */
			printf("%s: FPGA not ready, aborting.\n", __func__);
			return;
		}

		/* brdmodrst */
		writel(0, &reset_manager_base->brg_mod_reset);

		/* Remap the bridges into memory map */
		writel(l3mask, SOCFPGA_L3REGS_ADDRESS);
	}
	return;
}
