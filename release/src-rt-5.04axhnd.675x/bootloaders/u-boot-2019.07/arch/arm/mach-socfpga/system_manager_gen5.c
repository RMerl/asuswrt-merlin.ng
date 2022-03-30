// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/fpga_manager.h>

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

/*
 * Populate the value for SYSMGR.FPGAINTF.MODULE based on pinmux setting.
 * The value is not wrote to SYSMGR.FPGAINTF.MODULE but
 * CONFIG_SYSMGR_ISWGRP_HANDOFF.
 */
static void populate_sysmgr_fpgaintf_module(void)
{
	u32 handoff_val = 0;

	/* ISWGRP_HANDOFF_FPGAINTF */
	writel(0, &sysmgr_regs->iswgrp_handoff[2]);

	/* Enable the signal for those HPS peripherals that use FPGA. */
	if (readl(&sysmgr_regs->nandusefpga) == SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_NAND;
	if (readl(&sysmgr_regs->rgmii1usefpga) == SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_EMAC1;
	if (readl(&sysmgr_regs->sdmmcusefpga) == SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SDMMC;
	if (readl(&sysmgr_regs->rgmii0usefpga) == SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_EMAC0;
	if (readl(&sysmgr_regs->spim0usefpga) == SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SPIM0;
	if (readl(&sysmgr_regs->spim1usefpga) == SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SPIM1;

	/* populate (not writing) the value for SYSMGR.FPGAINTF.MODULE
	based on pinmux setting */
	setbits_le32(&sysmgr_regs->iswgrp_handoff[2], handoff_val);

	handoff_val = readl(&sysmgr_regs->iswgrp_handoff[2]);
	if (fpgamgr_test_fpga_ready()) {
		/* Enable the required signals only */
		writel(handoff_val, &sysmgr_regs->fpgaintfgrp_module);
	}
}

/*
 * Configure all the pin muxes
 */
void sysmgr_pinmux_init(void)
{
	u32 regs = (u32)&sysmgr_regs->emacio[0];
	const u8 *sys_mgr_init_table;
	unsigned int len;
	int i;

	sysmgr_get_pinmux_table(&sys_mgr_init_table, &len);

	for (i = 0; i < len; i++) {
		writel(sys_mgr_init_table[i], regs);
		regs += sizeof(regs);
	}

	populate_sysmgr_fpgaintf_module();
}

/*
 * This bit allows the bootrom to configure the IOs after a warm reset.
 */
void sysmgr_config_warmrstcfgio(int enable)
{
	if (enable)
		setbits_le32(&sysmgr_regs->romcodegrp_ctrl,
			     SYSMGR_ROMCODEGRP_CTRL_WARMRSTCFGIO);
	else
		clrbits_le32(&sysmgr_regs->romcodegrp_ctrl,
			     SYSMGR_ROMCODEGRP_CTRL_WARMRSTCFGIO);
}
