// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for TI AM335X based pxm2 board
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * u-boot:/board/ti/am335x/board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
 */

#include <cpsw.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <nand.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include "pmic.h"
#include "../common/board_am335x.h"
#include "../common/eeprom.h"
#include "../common/factoryset.h"

#ifdef CONFIG_SPL_BUILD
void draco_init_ddr(void)
{
struct emif_regs pxm2_ddr3_emif_reg_data = {
	.sdram_config = 0x41805332,
	.sdram_tim1 = 0x666b3c9,
	.sdram_tim2 = 0x243631ca,
	.sdram_tim3 = 0x33f,
	.emif_ddr_phy_ctlr_1 = 0x100005,
	.zq_config = 0,
	.ref_ctrl = 0x81a,
};

struct ddr_data pxm2_ddr3_data = {
	.datardsratio0 = 0x81204812,
	.datawdsratio0 = 0,
	.datafwsratio0 = 0x8020080,
	.datawrsratio0 = 0x4010040,
};

struct cmd_control pxm2_ddr3_cmd_ctrl_data = {
	.cmd0csratio = 0x80,
	.cmd0iclkout = 0,
	.cmd1csratio = 0x80,
	.cmd1iclkout = 0,
	.cmd2csratio = 0x80,
	.cmd2iclkout = 0,
};

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= DDR_IOCTRL_VAL,
	.cm1ioctl		= DDR_IOCTRL_VAL,
	.cm2ioctl		= DDR_IOCTRL_VAL,
	.dt0ioctl		= DDR_IOCTRL_VAL,
	.dt1ioctl		= DDR_IOCTRL_VAL,
};

	config_ddr(DDR_PLL_FREQ, &ioregs, &pxm2_ddr3_data,
		   &pxm2_ddr3_cmd_ctrl_data, &pxm2_ddr3_emif_reg_data, 0);
}

/*
 * voltage switching for MPU frequency switching.
 * @module = mpu - 0, core - 1
 * @vddx_op_vol_sel = vdd voltage to set
 */

#define MPU	0
#define CORE	1

int voltage_update(unsigned int module, unsigned char vddx_op_vol_sel)
{
	uchar buf[4];
	unsigned int reg_offset;

	if (module == MPU)
		reg_offset = PMIC_VDD1_OP_REG;
	else
		reg_offset = PMIC_VDD2_OP_REG;

	/* Select VDDx OP   */
	if (i2c_read(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	buf[0] &= ~PMIC_OP_REG_CMD_MASK;

	if (i2c_write(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	/* Configure VDDx OP  Voltage */
	if (i2c_read(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	buf[0] &= ~PMIC_OP_REG_SEL_MASK;
	buf[0] |= vddx_op_vol_sel;

	if (i2c_write(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	if (i2c_read(PMIC_CTRL_I2C_ADDR, reg_offset, 1, buf, 1))
		return 1;

	if ((buf[0] & PMIC_OP_REG_SEL_MASK) != vddx_op_vol_sel)
		return 1;

	return 0;
}

#define OSC     (V_OSCK/1000000)

const struct dpll_params dpll_mpu_pxm2 = {
		720, OSC-1, 1, -1, -1, -1, -1};

void spl_draco_board_init(void)
{
	uchar buf[4];
	/*
	 * pxm2 PMIC code.  All boards currently want an MPU voltage
	 * of 1.2625V and CORE voltage of 1.1375V to operate at
	 * 720MHz.
	 */
	if (i2c_probe(PMIC_CTRL_I2C_ADDR))
		return;

	/* VDD1/2 voltage selection register access by control i/f */
	if (i2c_read(PMIC_CTRL_I2C_ADDR, PMIC_DEVCTRL_REG, 1, buf, 1))
		return;

	buf[0] |= PMIC_DEVCTRL_REG_SR_CTL_I2C_SEL_CTL_I2C;

	if (i2c_write(PMIC_CTRL_I2C_ADDR, PMIC_DEVCTRL_REG, 1, buf, 1))
		return;

	/* Frequency switching for OPP 120 */
	if (voltage_update(MPU, PMIC_OP_REG_SEL_1_2_6) ||
	    voltage_update(CORE, PMIC_OP_REG_SEL_1_1_3)) {
		printf("voltage update failed\n");
	}
}

int draco_read_eeprom(void)
{
	/* nothing ToDo here for this board */

	return 0;
}
#endif /* if def CONFIG_SPL_BUILD */

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH) && defined(CONFIG_SPL_BUILD))
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
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 1,
		.phy_if		= PHY_INTERFACE_MODE_RMII,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 4,
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
#endif /* #if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) */

#if defined(CONFIG_DRIVER_TI_CPSW) || \
	(defined(CONFIG_USB_ETHER) && defined(CONFIG_USB_MUSB_GADGET))
int board_eth_init(struct bd_info *bis)
{
	int n = 0;
#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH) && defined(CONFIG_SPL_BUILD))
	struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;
#ifdef CONFIG_FACTORYSET
	int rv;
	if (!is_valid_ethaddr(factory_dat.mac))
		printf("Error: no valid mac address\n");
	else
		eth_env_set_enetaddr("ethaddr", factory_dat.mac);
#endif /* #ifdef CONFIG_FACTORYSET */

	/* Set rgmii mode and enable rmii clock to be sourced from chip */
	writel(RGMII_MODE_ENABLE  | RGMII_INT_DELAY, &cdev->miisel);

	rv = cpsw_register(&cpsw_data);
	if (rv < 0)
		printf("Error %d registering CPSW switch\n", rv);
	else
		n += rv;
#endif
	return n;
}
#endif /* #if defined(CONFIG_DRIVER_TI_CPSW) */

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	int ret;

	omap_nand_switch_ecc(1, 8);

#ifdef CONFIG_FACTORYSET
	if (factory_dat.asn[0] != 0) {
		char tmp[2 * MAX_STRING_LENGTH + 2];

		if (strncmp((const char *)factory_dat.asn, "PXM50", 5) == 0)
			factory_dat.pxm50 = 1;
		else
			factory_dat.pxm50 = 0;
		sprintf(tmp, "%s_%s", factory_dat.asn,
			factory_dat.comp_version);
		ret = env_set("boardid", tmp);
		if (ret)
			printf("error setting board id\n");
	} else {
		factory_dat.pxm50 = 1;
		ret = env_set("boardid", "PXM50_1.0");
		if (ret)
			printf("error setting board id\n");
	}
	debug("PXM50: %d\n", factory_dat.pxm50);
#endif

	return 0;
}
#endif
