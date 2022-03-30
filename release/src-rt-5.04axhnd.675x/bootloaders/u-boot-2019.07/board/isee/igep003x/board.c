// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for IGEP COM AQUILA and SMARC AM335x based boards
 *
 * Copyright (C) 2013-2017, ISEE 2007 SL - http://www.isee.biz/
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
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <fdt_support.h>
#include <mtd_node.h>
#include <jffs2/load_kernel.h>
#include <environment.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

/* GPIO0_27 and GPIO0_26 are used to read board revision from IGEP003x boards
 * and control IGEP0034 green and red LEDs.
 * U-boot configures these pins as input pullup to detect board revision:
 * IGEP0034-LITE = 0b00
 * IGEP0034 (FULL) = 0b01
 * IGEP0033 = 0b1X
 */
#define GPIO_GREEN_REVISION	27
#define GPIO_RED_REVISION	26

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

/*
 * Routine: get_board_revision
 * Description: Returns the board revision
 */
static int get_board_revision(void)
{
	int revision;

	gpio_request(GPIO_GREEN_REVISION, "green_revision");
	gpio_direction_input(GPIO_GREEN_REVISION);
	revision = 2 * gpio_get_value(GPIO_GREEN_REVISION);
	gpio_free(GPIO_GREEN_REVISION);

	gpio_request(GPIO_RED_REVISION, "red_revision");
	gpio_direction_input(GPIO_RED_REVISION);
	revision = revision + gpio_get_value(GPIO_RED_REVISION);
	gpio_free(GPIO_RED_REVISION);

	return revision;
}

#ifdef CONFIG_SPL_BUILD
/* PN H5TQ4G63AFR is equivalent to MT41K256M16HA125*/
static const struct ddr_data ddr3_igep0034_data = {
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
};

static const struct ddr_data ddr3_igep0034_lite_data = {
	.datardsratio0 = K4B2G1646EBIH9_RD_DQS,
	.datawdsratio0 = K4B2G1646EBIH9_WR_DQS,
	.datafwsratio0 = K4B2G1646EBIH9_PHY_FIFO_WE,
	.datawrsratio0 = K4B2G1646EBIH9_PHY_WR_DATA,
};

static const struct cmd_control ddr3_igep0034_cmd_ctrl_data = {
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
};

static const struct cmd_control ddr3_igep0034_lite_cmd_ctrl_data = {
	.cmd0csratio = K4B2G1646EBIH9_RATIO,
	.cmd0iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,

	.cmd1csratio = K4B2G1646EBIH9_RATIO,
	.cmd1iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,

	.cmd2csratio = K4B2G1646EBIH9_RATIO,
	.cmd2iclkout = K4B2G1646EBIH9_INVERT_CLKOUT,
};

static struct emif_regs ddr3_igep0034_emif_reg_data = {
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
};

static struct emif_regs ddr3_igep0034_lite_emif_reg_data = {
	.sdram_config = K4B2G1646EBIH9_EMIF_SDCFG,
	.ref_ctrl = K4B2G1646EBIH9_EMIF_SDREF,
	.sdram_tim1 = K4B2G1646EBIH9_EMIF_TIM1,
	.sdram_tim2 = K4B2G1646EBIH9_EMIF_TIM2,
	.sdram_tim3 = K4B2G1646EBIH9_EMIF_TIM3,
	.zq_config = K4B2G1646EBIH9_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = K4B2G1646EBIH9_EMIF_READ_LATENCY,
};

const struct ctrl_ioregs ioregs_igep0034 = {
	.cm0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_igep0034_lite = {
	.cm0ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.cm1ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.cm2ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.dt0ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
	.dt1ioctl		= K4B2G1646EBIH9_IOCTRL_VALUE,
};

#define OSC    (V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		400, OSC-1, 1, -1, -1, -1, -1};

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

void sdram_init(void)
{
	if (get_board_revision() == 1)
		config_ddr(400, &ioregs_igep0034, &ddr3_igep0034_data,
			&ddr3_igep0034_cmd_ctrl_data, &ddr3_igep0034_emif_reg_data, 0);
	else
		config_ddr(400, &ioregs_igep0034_lite, &ddr3_igep0034_lite_data,
			&ddr3_igep0034_lite_cmd_ctrl_data, &ddr3_igep0034_lite_emif_reg_data, 0);
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	return serial_tstc() && serial_getc() == 'c';
}
#endif
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	gpmc_init();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	switch (get_board_revision()) {
		case 0:
			env_set("board_name", "igep0034-lite");
			break;
		case 1:
			env_set("board_name", "igep0034");
			break;
		default:
			env_set("board_name", "igep0033");
			break;
	}
#endif
	return 0;
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	static const struct node_info nodes[] = {
		{ "ti,omap2-nand", MTD_DEV_TYPE_NAND, },
	};

	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
	return 0;
}
#endif

#if defined(CONFIG_DRIVER_TI_CPSW)
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
		.phy_if		= PHY_INTERFACE_MODE_RMII,
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
	int rv, ret = 0;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;

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

	writel((GMII1_SEL_RMII | RMII1_IO_CLK_EN),
	       &cdev->miisel);

	if (get_board_revision() == 1)
		cpsw_slaves[0].phy_addr = 1;

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		ret += rv;

	return ret;
}
#endif
