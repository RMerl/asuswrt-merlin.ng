// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <i2c.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include "../p2571/max77620_init.h"
#include "pinmux-config-e2220-1170.h"

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
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_clear_tristate_input_clamping();

	gpio_config_table(e2220_1170_gpio_inits,
			  ARRAY_SIZE(e2220_1170_gpio_inits));

	pinmux_config_pingrp_table(e2220_1170_pingrps,
				   ARRAY_SIZE(e2220_1170_pingrps));

	pinmux_config_drvgrp_table(e2220_1170_drvgrps,
				   ARRAY_SIZE(e2220_1170_drvgrps));
}
