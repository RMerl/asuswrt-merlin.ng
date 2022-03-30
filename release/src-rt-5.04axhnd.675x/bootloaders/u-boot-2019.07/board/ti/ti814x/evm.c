// SPDX-License-Identifier: GPL-2.0+
/*
 * evm.c
 *
 * Board functions for TI814x EVM
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <cpsw.h>
#include <environment.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include "evm.h"

DECLARE_GLOBAL_DATA_PTR;

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

/* UART Defines */
#ifdef CONFIG_SPL_BUILD
static const struct cmd_control evm_ddr2_cctrl_data = {
	.cmd0csratio	= 0x80,
	.cmd0iclkout	= 0x00,

	.cmd1csratio	= 0x80,
	.cmd1iclkout	= 0x00,

	.cmd2csratio	= 0x80,
	.cmd2iclkout	= 0x00,
};

static const struct emif_regs evm_ddr2_emif0_regs = {
	.sdram_config			= 0x40801ab2,
	.ref_ctrl			= 0x10000c30,
	.sdram_tim1			= 0x0aaaf552,
	.sdram_tim2			= 0x043631d2,
	.sdram_tim3			= 0x00000327,
	.emif_ddr_phy_ctlr_1		= 0x00000007
};

static const struct emif_regs evm_ddr2_emif1_regs = {
	.sdram_config			= 0x40801ab2,
	.ref_ctrl			= 0x10000c30,
	.sdram_tim1			= 0x0aaaf552,
	.sdram_tim2			= 0x043631d2,
	.sdram_tim3			= 0x00000327,
	.emif_ddr_phy_ctlr_1		= 0x00000007
};

const struct dmm_lisa_map_regs evm_lisa_map_regs = {
	.dmm_lisa_map_0			= 0x00000000,
	.dmm_lisa_map_1			= 0x00000000,
	.dmm_lisa_map_2			= 0x806c0300,
	.dmm_lisa_map_3			= 0x806c0300,
};

static const struct ddr_data evm_ddr2_data = {
	.datardsratio0		= ((0x35<<10) | (0x35<<0)),
	.datawdsratio0		= ((0x20<<10) | (0x20<<0)),
	.datawiratio0		= ((0<<10) | (0<<0)),
	.datagiratio0		= ((0<<10) | (0<<0)),
	.datafwsratio0		= ((0x90<<10) | (0x90<<0)),
	.datawrsratio0		= ((0x50<<10) | (0x50<<0)),
};

void set_uart_mux_conf(void)
{
	/* Set UART pins */
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	/* Set MMC pins */
	enable_mmc1_pin_mux();

	/* Set Ethernet pins */
	enable_enet_pin_mux();
}

void sdram_init(void)
{
	config_dmm(&evm_lisa_map_regs);

	config_ddr(0, NULL, &evm_ddr2_data, &evm_ddr2_cctrl_data,
		   &evm_ddr2_emif0_regs, 0);
	config_ddr(0, NULL, &evm_ddr2_data, &evm_ddr2_cctrl_data,
		   &evm_ddr2_emif1_regs, 1);
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	return 0;
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(1, 0, 0, -1, -1);

	return 0;
}
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x50,
		.sliver_reg_ofs	= 0x700,
		.phy_addr	= 1,
	},
	{
		.slave_reg_ofs	= 0x90,
		.sliver_reg_ofs	= 0x740,
		.phy_addr	= 0,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x100,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0x600,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x28,
	.hw_stats_reg_ofs	= 0x400,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_1,
};
#endif

int board_eth_init(bd_t *bis)
{
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		printf("<ethaddr> not set. Reading from E-fuse\n");
		/* try reading mac address from efuse */
		mac_lo = readl(&cdev->macid0l);
		mac_hi = readl(&cdev->macid0h);
		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
		else
			printf("Unable to read MAC address. Set <ethaddr>\n");
	}

	return cpsw_register(&cpsw_data);
}
