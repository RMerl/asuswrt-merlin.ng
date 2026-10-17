// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gp_padctrl.h>
#include "pinmux-config-dalmore.h"
#include <i2c.h>

#define BAT_I2C_ADDRESS		0x48	/* TPS65090 charger */
#define PMU_I2C_ADDRESS		0x58	/* TPS65913 PMU */

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_config_pingrp_table(tegra114_pinmux_set_nontristate,
		ARRAY_SIZE(tegra114_pinmux_set_nontristate));

	pinmux_config_pingrp_table(tegra114_pinmux_common,
		ARRAY_SIZE(tegra114_pinmux_common));

	pinmux_config_pingrp_table(unused_pins_lowpower,
		ARRAY_SIZE(unused_pins_lowpower));

	/* Initialize any non-default pad configs (APB_MISC_GP regs) */
	pinmux_config_drvgrp_table(dalmore_padctrl,
		ARRAY_SIZE(dalmore_padctrl));
}

#if defined(CONFIG_MMC_SDHCI_TEGRA)
/*
 * Do I2C/PMU writes to bring up SD card bus power
 *
 */
void board_sdmmc_voltage_init(void)
{
	struct udevice *dev;
	uchar reg, data_buffer[1];
	int ret;

	ret = i2c_get_chip_for_busnum(0, PMU_I2C_ADDRESS, 1, &dev);
	if (ret) {
		debug("%s: Cannot find PMIC I2C chip\n", __func__);
		return;
	}

	/* TPS65913: LDO9_VOLTAGE = 3.3V */
	data_buffer[0] = 0x31;
	reg = 0x61;

	ret = dm_i2c_write(dev, reg, data_buffer, 1);
	if (ret)
		printf("%s: PMU i2c_write %02X<-%02X returned %d\n",
			__func__, reg, data_buffer[0], ret);

	/* TPS65913: LDO9_CTRL = Active */
	data_buffer[0] = 0x01;
	reg = 0x60;

	ret = dm_i2c_write(dev, reg, data_buffer, 1);
	if (ret)
		printf("%s: PMU i2c_write %02X<-%02X returned %d\n",
			__func__, reg, data_buffer[0], ret);

	/* TPS65090: FET6_CTRL = enable output auto discharge, enable FET6 */
	data_buffer[0] = 0x03;
	reg = 0x14;

	ret = i2c_get_chip_for_busnum(0, BAT_I2C_ADDRESS, 1, &dev);
	if (ret) {
		debug("%s: Cannot find charger I2C chip\n", __func__);
		return;
	}
	ret = dm_i2c_write(dev, reg, data_buffer, 1);
	if (ret)
		printf("%s: BAT i2c_write %02X<-%02X returned %d\n",
			__func__, reg, data_buffer[0], ret);
}

/*
 * Routine: pin_mux_mmc
 * Description: setup the MMC muxes, power rails, etc.
 */
void pin_mux_mmc(void)
{
	/*
	 * NOTE: We don't do mmc-specific pin muxes here.
	 * They were done globally in pinmux_init().
	 */

	/* Bring up the SDIO3 power rail */
	board_sdmmc_voltage_init();
}
#endif /* MMC */
