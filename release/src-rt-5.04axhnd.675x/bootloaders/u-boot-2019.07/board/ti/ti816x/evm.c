// SPDX-License-Identifier: GPL-2.0+
/*
 * evm.c
 *
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Antoine Tenart, <atenart@adeneo-embedded.com>
 */

#include <common.h>
#include <environment.h>
#include <spl.h>
#include <netdev.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
#if defined(CONFIG_NAND)
	gpmc_init();
#endif
	return 0;
}

int board_eth_init(bd_t *bis)
{
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

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

	return davinci_emac_initialize();
}

#ifdef CONFIG_SPL_BUILD
static struct module_pin_mux mmc_pin_mux[] = {
	{ OFFSET(pincntl157), PULLDOWN_EN | PULLUDDIS | MODE(0x0) },
	{ OFFSET(pincntl158), PULLDOWN_EN | PULLUDEN | MODE(0x0) },
	{ OFFSET(pincntl159), PULLUP_EN | PULLUDDIS | MODE(0x0) },
	{ OFFSET(pincntl160), PULLUP_EN | PULLUDDIS | MODE(0x0) },
	{ OFFSET(pincntl161), PULLUP_EN | PULLUDDIS | MODE(0x0) },
	{ OFFSET(pincntl162), PULLUP_EN | PULLUDDIS | MODE(0x0) },
	{ OFFSET(pincntl163), PULLUP_EN | PULLUDDIS | MODE(0x0) },
	{ -1 },
};

void set_uart_mux_conf(void) {}

void set_mux_conf_regs(void)
{
	configure_module_pin_mux(mmc_pin_mux);
}

/*
 * EMIF Paramters.  Refer the EMIF register documentation and the
 * memory datasheet for details.  This is for 796 MHz.
 */
#define EMIF_TIM1   0x1779C9FE
#define EMIF_TIM2   0x50608074
#define EMIF_TIM3   0x009F857F
#define EMIF_SDREF  0x10001841
#define EMIF_SDCFG  0x62A73832
#define EMIF_PHYCFG 0x00000110
static const struct emif_regs ddr3_emif_regs = {
	.sdram_config		= EMIF_SDCFG,
	.ref_ctrl		= EMIF_SDREF,
	.sdram_tim1		= EMIF_TIM1,
	.sdram_tim2		= EMIF_TIM2,
	.sdram_tim3		= EMIF_TIM3,
	.emif_ddr_phy_ctlr_1	= EMIF_PHYCFG,
};

static const struct cmd_control ddr3_ctrl = {
	.cmd0csratio	= 0x100,
	.cmd0iclkout	= 0x001,
	.cmd1csratio	= 0x100,
	.cmd1iclkout	= 0x001,
	.cmd2csratio	= 0x100,
	.cmd2iclkout	= 0x001,
};

/* These values are obtained from the CCS app */
#define RD_DQS_GATE	(0x1B3)
#define RD_DQS		(0x35)
#define WR_DQS		(0x93)
static struct ddr_data ddr3_data = {
	.datardsratio0		= ((RD_DQS<<10) | (RD_DQS<<0)),
	.datawdsratio0		= ((WR_DQS<<10) | (WR_DQS<<0)),
	.datawiratio0		= ((0x20<<10) | 0x20<<0),
	.datagiratio0		= ((0x20<<10) | 0x20<<0),
	.datafwsratio0		= ((RD_DQS_GATE<<10) | (RD_DQS_GATE<<0)),
	.datawrsratio0		= (((WR_DQS+0x40)<<10) | ((WR_DQS+0x40)<<0)),
};

static const struct dmm_lisa_map_regs evm_lisa_map_regs = {
	.dmm_lisa_map_0 = 0x00000000,
	.dmm_lisa_map_1 = 0x00000000,
	.dmm_lisa_map_2 = 0x80640300,
	.dmm_lisa_map_3 = 0xC0640320,
};

void sdram_init(void)
{
	/*
	 * Pass in our DDR3 config information and that we have 2 EMIFs to
	 * configure.
	 */
	config_ddr(&ddr3_data, &ddr3_ctrl, &ddr3_emif_regs,
			&evm_lisa_map_regs, 2);
}
#endif /* CONFIG_SPL_BUILD */
