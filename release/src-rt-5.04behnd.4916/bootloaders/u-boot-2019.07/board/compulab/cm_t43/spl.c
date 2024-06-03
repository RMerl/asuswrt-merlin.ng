// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Compulab, Ltd.
 */

#include <common.h>
#include <spl.h>
#include <i2c.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr_defs.h>
#include <asm/gpio.h>
#include <power/pmic.h>
#include <power/tps65218.h>
#include "board.h"

const struct dpll_params dpll_mpu  = { 800,  24, 1,  -1, -1, -1, -1 };
const struct dpll_params dpll_core = { 1000, 24, -1, -1, 10,  8,  4 };
const struct dpll_params dpll_per  = { 960,  24, 5,  -1, -1, -1, -1 };
const struct dpll_params dpll_ddr  = { 400,  23, 1,  -1,  1, -1, -1 };

const struct ctrl_ioregs ioregs_ddr3 = {
	.cm0ioctl		= DDR3_ADDRCTRL_IOCTRL_VALUE,
	.cm1ioctl		= DDR3_ADDRCTRL_WD0_IOCTRL_VALUE,
	.cm2ioctl		= DDR3_ADDRCTRL_WD1_IOCTRL_VALUE,
	.dt0ioctl		= DDR3_DATA0_IOCTRL_VALUE,
	.dt1ioctl		= DDR3_DATA0_IOCTRL_VALUE,
	.dt2ioctrl		= DDR3_DATA0_IOCTRL_VALUE,
	.dt3ioctrl		= DDR3_DATA0_IOCTRL_VALUE,
	.emif_sdram_config_ext	= 0x0143,
};

/* EMIF DDR3 Configurations are different for production AM43X GP EVMs */
struct emif_regs ddr3_emif_regs = {
	.sdram_config			= 0x638413B2,
	.ref_ctrl			= 0x00000C30,
	.sdram_tim1			= 0xEAAAD4DB,
	.sdram_tim2			= 0x266B7FDA,
	.sdram_tim3			= 0x107F8678,
	.read_idle_ctrl			= 0x00050000,
	.zq_config			= 0x50074BE4,
	.temp_alert_config		= 0x0,
	.emif_ddr_phy_ctlr_1		= 0x0E004008,
	.emif_ddr_ext_phy_ctrl_1	= 0x08020080,
	.emif_ddr_ext_phy_ctrl_2	= 0x00000066,
	.emif_ddr_ext_phy_ctrl_3	= 0x00000091,
	.emif_ddr_ext_phy_ctrl_4	= 0x000000B9,
	.emif_ddr_ext_phy_ctrl_5	= 0x000000E6,
	.emif_rd_wr_exec_thresh		= 0x80000405,
	.emif_prio_class_serv_map	= 0x80000001,
	.emif_connect_id_serv_1_map	= 0x80000094,
	.emif_connect_id_serv_2_map	= 0x00000000,
	.emif_cos_config		= 0x000FFFFF
};

const u32 ext_phy_ctrl_const_base_ddr3[] = {
	0x00000000,
	0x00000044,
	0x00000044,
	0x00000046,
	0x00000046,
	0x00000000,
	0x00000059,
	0x00000077,
	0x00000093,
	0x000000A8,
	0x00000000,
	0x00000019,
	0x00000037,
	0x00000053,
	0x00000068,
	0x00000000,
	0x0,
	0x0,
	0x40000000,
	0x08102040
};

void emif_get_ext_phy_ctrl_const_regs(const u32 **regs, u32 *size)
{
	*regs = ext_phy_ctrl_const_base_ddr3;
	*size = ARRAY_SIZE(ext_phy_ctrl_const_base_ddr3);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

const struct dpll_params *get_dpll_mpu_params(void)
{
	return &dpll_mpu;
}

const struct dpll_params *get_dpll_core_params(void)
{
	return &dpll_core;
}

const struct dpll_params *get_dpll_per_params(void)
{
	return &dpll_per;
}

void scale_vcores(void)
{
	set_i2c_pin_mux();
	i2c_init(CONFIG_SYS_OMAP24_I2C_SPEED, CONFIG_SYS_OMAP24_I2C_SLAVE);
	if (i2c_probe(TPS65218_CHIP_PM))
		return;

	tps65218_voltage_update(TPS65218_DCDC1, TPS65218_DCDC_VOLT_SEL_1100MV);
	tps65218_voltage_update(TPS65218_DCDC2, TPS65218_DCDC_VOLT_SEL_1100MV);
}

void sdram_init(void)
{
	unsigned long ram_size;

	config_ddr(0, &ioregs_ddr3, NULL, NULL, &ddr3_emif_regs, 0);
	ram_size = get_ram_size((long int *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (ram_size == 0x80000000 ||
	    ram_size == 0x40000000 ||
	    ram_size == 0x20000000)
		return;

	ddr3_emif_regs.sdram_config = 0x638453B2;
	config_ddr(0, &ioregs_ddr3, NULL, NULL, &ddr3_emif_regs, 0);
	ram_size = get_ram_size((long int *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (ram_size == 0x08000000)
		return;

	hang();
}

