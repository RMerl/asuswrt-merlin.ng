// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/fpga_manager.h>

/*
 * Populate the value for SYSMGR.FPGAINTF.MODULE based on pinmux setting.
 * The value is not wrote to SYSMGR.FPGAINTF.MODULE but
 * CONFIG_SYSMGR_ISWGRP_HANDOFF.
 */
static void populate_sysmgr_fpgaintf_module(void)
{
	u32 handoff_val = 0;

	/* ISWGRP_HANDOFF_FPGAINTF */
	writel(0, socfpga_get_sysmgr_addr() + SYSMGR_ISWGRP_HANDOFF_OFFSET(2));

	/* Enable the signal for those HPS peripherals that use FPGA. */
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_GEN5_NAND_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_NAND;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_GEN5_RGMII1_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_EMAC1;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_GEN5_SDMMC_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SDMMC;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_GEN5_RGMII0_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_EMAC0;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_GEN5_SPIM0_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SPIM0;
	if (readl(socfpga_get_sysmgr_addr() + SYSMGR_GEN5_SPIM1_USEFPGA) ==
	    SYSMGR_FPGAINTF_USEFPGA)
		handoff_val |= SYSMGR_FPGAINTF_SPIM1;

	/* populate (not writing) the value for SYSMGR.FPGAINTF.MODULE
	based on pinmux setting */
	setbits_le32(socfpga_get_sysmgr_addr() +
		     SYSMGR_ISWGRP_HANDOFF_OFFSET(2),
		     handoff_val);

	handoff_val = readl(socfpga_get_sysmgr_addr() +
			    SYSMGR_ISWGRP_HANDOFF_OFFSET(2));
	if (fpgamgr_test_fpga_ready()) {
		/* Enable the required signals only */
		writel(handoff_val,
		       socfpga_get_sysmgr_addr() +
		       SYSMGR_GEN5_FPGAINFGRP_MODULE);
	}
}

/*
 * Configure all the pin muxes
 */
void sysmgr_pinmux_init(void)
{
	u32 regs = (u32)socfpga_get_sysmgr_addr() + SYSMGR_GEN5_EMACIO;
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
		setbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_GEN5_ROMCODEGRP_CTRL,
			     SYSMGR_ROMCODEGRP_CTRL_WARMRSTCFGIO);
	else
		clrbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_GEN5_ROMCODEGRP_CTRL,
			     SYSMGR_ROMCODEGRP_CTRL_WARMRSTCFGIO);
}
