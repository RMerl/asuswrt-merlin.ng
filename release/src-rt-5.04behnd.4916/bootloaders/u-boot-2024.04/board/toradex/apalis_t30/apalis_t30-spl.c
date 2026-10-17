// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2021
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <common.h>
#include <asm/arch-tegra/tegra_i2c.h>
#include <linux/delay.h>

/* I2C addr is in 8 bit */
#define TPS65911_I2C_ADDR		0x5A
#define TPS65911_VDDCTRL_OP_REG		0x28
#define TPS65911_VDDCTRL_SR_REG		0x27
#define TPS65911_VDDCTRL_OP_DATA	(0x2400 | TPS65911_VDDCTRL_OP_REG)
#define TPS65911_VDDCTRL_SR_DATA	(0x0100 | TPS65911_VDDCTRL_SR_REG)

void pmic_enable_cpu_vdd(void)
{
	/*
	 * Bring up CPU VDD via the TPS65911x PMIC on the DVC I2C bus.
	 * First set VDD to 1.0125V, then enable the VDD regulator.
	 */
	udelay(1000);
	tegra_i2c_ll_write(TPS65911_I2C_ADDR,
			   TPS65911_VDDCTRL_OP_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS65911_I2C_ADDR,
			   TPS65911_VDDCTRL_SR_DATA);
	udelay(10 * 1000);
}
