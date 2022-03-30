// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <altera.h>
#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/misc.h>
#include <asm/pl310.h>
#include <linux/libfdt.h>
#include <asm/arch/mailbox_s10.h>

#include <dt-bindings/reset/altr,rst-mgr-s10.h>

DECLARE_GLOBAL_DATA_PTR;

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

/*
 * FPGA programming support for SoC FPGA Stratix 10
 */
static Altera_desc altera_fpga[] = {
	{
		/* Family */
		Intel_FPGA_Stratix10,
		/* Interface type */
		secure_device_manager_mailbox,
		/* No limitation as additional data will be ignored */
		-1,
		/* No device function table */
		NULL,
		/* Base interface address specified in driver */
		NULL,
		/* No cookie implementation */
		0
	},
};

/*
 * DesignWare Ethernet initialization
 */
#ifdef CONFIG_ETH_DESIGNWARE

static u32 socfpga_phymode_setup(u32 gmac_index, const char *phymode)
{
	u32 modereg;

	if (!phymode)
		return -EINVAL;

	if (!strcmp(phymode, "mii") || !strcmp(phymode, "gmii") ||
	    !strcmp(phymode, "sgmii"))
		modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII;
	else if (!strcmp(phymode, "rgmii"))
		modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII;
	else if (!strcmp(phymode, "rmii"))
		modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII;
	else
		return -EINVAL;

	clrsetbits_le32(&sysmgr_regs->emac0 + gmac_index,
			SYSMGR_EMACGRP_CTRL_PHYSEL_MASK,
			modereg);

	return 0;
}

static int socfpga_set_phymode(void)
{
	const void *fdt = gd->fdt_blob;
	struct fdtdec_phandle_args args;
	const char *phy_mode;
	u32 gmac_index;
	int nodes[3];	/* Max. 3 GMACs */
	int ret, count;
	int i, node;

	count = fdtdec_find_aliases_for_id(fdt, "ethernet",
					   COMPAT_ALTERA_SOCFPGA_DWMAC,
					   nodes, ARRAY_SIZE(nodes));
	for (i = 0; i < count; i++) {
		node = nodes[i];
		if (node <= 0)
			continue;

		ret = fdtdec_parse_phandle_with_args(fdt, node, "resets",
						     "#reset-cells", 1, 0,
						     &args);
		if (ret || args.args_count != 1) {
			debug("GMAC%i: Failed to parse DT 'resets'!\n", i);
			continue;
		}

		gmac_index = args.args[0] - EMAC0_RESET;

		phy_mode = fdt_getprop(fdt, node, "phy-mode", NULL);
		ret = socfpga_phymode_setup(gmac_index, phy_mode);
		if (ret) {
			debug("GMAC%i: Failed to parse DT 'phy-mode'!\n", i);
			continue;
		}
	}

	return 0;
}
#else
static int socfpga_set_phymode(void)
{
	return 0;
};
#endif

/*
 * Print CPU information
 */
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	puts("CPU:   Intel FPGA SoCFPGA Platform (ARMv8 64bit Cortex-A53)\n");

	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	char qspi_string[13];

	sprintf(qspi_string, "<0x%08x>", cm_get_qspi_controller_clk_hz());
	env_set("qspi_clock", qspi_string);

	socfpga_set_phymode();
	return 0;
}
#endif

int arch_early_init_r(void)
{
	socfpga_fpga_add(&altera_fpga[0]);

	return 0;
}

void do_bridge_reset(int enable, unsigned int mask)
{
	/* Check FPGA status before bridge enable */
	if (enable) {
		int ret = mbox_get_fpga_config_status(MBOX_RECONFIG_STATUS);

		if (ret && ret != MBOX_CFGSTAT_STATE_CONFIG)
			ret = mbox_get_fpga_config_status(MBOX_CONFIG_STATUS);

		if (ret)
			return;
	}

	socfpga_bridges_reset(enable);
}
