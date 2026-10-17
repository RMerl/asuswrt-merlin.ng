// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2019
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <i2c.h>
#include <log.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/gpio.h>
#include "max77620_init.h"

void pin_mux_mmc(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX77620 LDO2 to 3.3V for SD card power */
	debug("%s: Set LDO2 for VDDIO_SDMMC_AP power to 3.3V\n", __func__);
	ret = i2c_get_chip_for_busnum(0, MAX77620_I2C_ADDR_7BIT, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX77620 I2C chip\n", __func__);
		return;
	}
	/* 0xF2 for 3.3v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	val = 0xF2;
	ret = dm_i2c_write(dev, MAX77620_CNFG1_L2_REG, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x27 failed: %d\n", ret);
}

/*
 * Routine: start_cpu_fan
 * Description: Enable/start PWM CPU fan on P2571
 */
void start_cpu_fan(void)
{
	/* GPIO_PE4 is PS_VDD_FAN_ENABLE */
	gpio_request(TEGRA_GPIO(E, 4), "FAN_VDD");
	gpio_direction_output(TEGRA_GPIO(E, 4), 1);
}
