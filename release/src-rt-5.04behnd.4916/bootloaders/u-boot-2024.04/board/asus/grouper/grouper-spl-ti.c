// SPDX-License-Identifier: GPL-2.0+
/*
 *  T30 Grouper TI SPL stage configuration
 *
 *  (C) Copyright 2010-2013
 *  NVIDIA Corporation <www.nvidia.com>
 *
 *  (C) Copyright 2022
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <asm/arch/tegra.h>
#include <asm/arch-tegra/tegra_i2c.h>
#include <linux/delay.h>

#define TPS65911_I2C_ADDR		(0x2D << 1)
#define TPS65911_VDDCTRL_OP_REG		0x28
#define TPS65911_VDDCTRL_SR_REG		0x27
#define TPS65911_VDDCTRL_OP_DATA	(0x2400 | TPS65911_VDDCTRL_OP_REG)
#define TPS65911_VDDCTRL_SR_DATA	(0x0100 | TPS65911_VDDCTRL_SR_REG)

#define TPS62361B_I2C_ADDR		(0x60 << 1)
#define TPS62361B_SET3_REG		0x03
#define TPS62361B_SET3_DATA		(0x4600 | TPS62361B_SET3_REG)

void pmic_enable_cpu_vdd(void)
{
	/* Set VDD_CORE to 1.200V. */
	tegra_i2c_ll_write(TPS62361B_I2C_ADDR, TPS62361B_SET3_DATA);

	udelay(1000);

	/*
	 * Bring up CPU VDD via the TPS65911x PMIC on the DVC I2C bus.
	 * First set VDD to 1.0125V, then enable the VDD regulator.
	 */
	tegra_i2c_ll_write(TPS65911_I2C_ADDR, TPS65911_VDDCTRL_OP_DATA);
	udelay(1000);
	tegra_i2c_ll_write(TPS65911_I2C_ADDR, TPS65911_VDDCTRL_SR_DATA);
	udelay(10 * 1000);
}
