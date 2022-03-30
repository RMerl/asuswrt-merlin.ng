// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Gumstix Pepper and AM335x-based boards
 *
 * Copyright (C) 2014, Gumstix, Incorporated - http://www.gumstix.com/
 * Based on board/ti/am335x/board.c from Texas Instruments, Inc.
 */

#include <common.h>
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
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include <environment.h>
#include <watchdog.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
#define OSC	(V_OSCK/1000000)

static const struct ddr_data ddr3_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
};

const struct dpll_params dpll_ddr3 = {400, OSC-1, 1, -1, -1, -1, -1};

const struct ctrl_ioregs ioregs_ddr3 = {
	.cm0ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl               = MT41K256M16HA125E_IOCTRL_VALUE,
};

static const struct ddr_data ddr2_data = {
	.datardsratio0 = MT47H128M16RT25E_RD_DQS,
	.datafwsratio0 = MT47H128M16RT25E_PHY_FIFO_WE,
	.datawrsratio0 = MT47H128M16RT25E_PHY_WR_DATA,
};

static const struct cmd_control ddr2_cmd_ctrl_data = {
	.cmd0csratio = MT47H128M16RT25E_RATIO,

	.cmd1csratio = MT47H128M16RT25E_RATIO,

	.cmd2csratio = MT47H128M16RT25E_RATIO,
};

static const struct emif_regs ddr2_emif_reg_data = {
	.sdram_config = MT47H128M16RT25E_EMIF_SDCFG,
	.ref_ctrl = MT47H128M16RT25E_EMIF_SDREF,
	.sdram_tim1 = MT47H128M16RT25E_EMIF_TIM1,
	.sdram_tim2 = MT47H128M16RT25E_EMIF_TIM2,
	.sdram_tim3 = MT47H128M16RT25E_EMIF_TIM3,
	.emif_ddr_phy_ctlr_1 = MT47H128M16RT25E_EMIF_READ_LATENCY,
};

const struct dpll_params dpll_ddr2 = {266, OSC-1, 1, -1, -1, -1, -1};

const struct ctrl_ioregs ioregs_ddr2 = {
	.cm0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm2ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
};

static int read_eeprom(struct pepper_board_id *header)
{
	if (i2c_probe(CONFIG_SYS_I2C_EEPROM_ADDR)) {
		return -ENODEV;
	}

	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0, 1, (uchar *)header,
		sizeof(struct pepper_board_id))) {
		return -EIO;
	}

	return 0;
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	struct pepper_board_id header;

	enable_i2c0_pin_mux();
	i2c_set_bus_num(0);

	if (read_eeprom(&header) < 0)
		return &dpll_ddr3;

	switch (header.device_vendor) {
	case GUMSTIX_PEPPER:
		return &dpll_ddr2;
	case GUMSTIX_PEPPER_DVI:
		return &dpll_ddr3;
	default:
		return &dpll_ddr3;
	}
}

void sdram_init(void)
{
	const struct dpll_params *dpll = get_dpll_ddr_params();

	/*
	 * Here we are assuming PLL clock reveals the type of RAM.
	 * DDR2 = 266
	 * DDR3 = 400
	 * Note that DDR3 is the default.
	 */
	if (dpll->m == 266) {
		config_ddr(dpll->m, &ioregs_ddr2, &ddr2_data,
			&ddr2_cmd_ctrl_data, &ddr2_emif_reg_data, 0);
	}
	else if (dpll->m == 400) {
		config_ddr(dpll->m, &ioregs_ddr3, &ddr3_data,
			&ddr3_cmd_ctrl_data, &ddr3_emif_reg_data, 0);
	}
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	return serial_tstc() && serial_getc() == 'c';
}
#endif

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}


#endif

int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gpmc_init();

	return 0;
}

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
		.phy_if		= PHY_INTERFACE_MODE_RGMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int board_eth_init(bd_t *bis)
{
	int rv, n = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	const char *devname;

	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
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
	}

	writel((RGMII_MODE_ENABLE | RGMII_INT_DELAY), &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;

	/*
	 *
	 * CPSW RGMII Internal Delay Mode is not supported in all PVT
	 * operating points.  So we must set the TX clock delay feature
	 * in the KSZ9021 PHY.  Since we only support a single ethernet
	 * device in U-Boot, we only do this for the current instance.
	 */
	devname = miiphy_get_current_dev();
	/* max rx/tx clock delay, min rx/tx control delay */
	miiphy_write(devname, 0x0, 0x0b, 0x8104);
	miiphy_write(devname, 0x0, 0xc, 0xa0a0);

	/* min rx data delay */
	miiphy_write(devname, 0x0, 0x0b, 0x8105);
	miiphy_write(devname, 0x0, 0x0c, 0x0000);

	/* min tx data delay */
	miiphy_write(devname, 0x0, 0x0b, 0x8106);
	miiphy_write(devname, 0x0, 0x0c, 0x0000);

	return n;
}
#endif
