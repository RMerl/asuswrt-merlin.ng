// SPDX-License-Identifier: GPL-2.0+
/*
 *  T30 LGE X3 SPL stage configuration
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

#define MAX77663_I2C_ADDR		(0x1C << 1)

#define MAX77663_REG_SD0		0x16
#define MAX77663_REG_SD0_DATA		(0x2100 | MAX77663_REG_SD0)
#define MAX77663_REG_SD1		0x17
#define MAX77663_REG_SD1_DATA		(0x3000 | MAX77663_REG_SD1)
#define MAX77663_REG_LDO4		0x2B
#define MAX77663_REG_LDO4_DATA		(0xE000 | MAX77663_REG_LDO4)

#define MAX77663_REG_GPIO1		0x37
#define MAX77663_REG_GPIO1_DATA		(0x0800 | MAX77663_REG_GPIO1)
#define MAX77663_REG_GPIO4		0x3A
#define MAX77663_REG_GPIO4_DATA		(0x0100 | MAX77663_REG_GPIO4)

void pmic_enable_cpu_vdd(void)
{
	/* Set VDD_CORE to 1.200V. */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_SD1_DATA);

	udelay(1000);

	/* Bring up VDD_CPU to 1.0125V. */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_SD0_DATA);
	udelay(1000);

	/* Bring up VDD_RTC to 1.200V. */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_LDO4_DATA);
	udelay(10 * 1000);

	/* Set GPIO4 and GPIO1 states */
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_GPIO4_DATA);
	tegra_i2c_ll_write(MAX77663_I2C_ADDR, MAX77663_REG_GPIO1_DATA);
}
