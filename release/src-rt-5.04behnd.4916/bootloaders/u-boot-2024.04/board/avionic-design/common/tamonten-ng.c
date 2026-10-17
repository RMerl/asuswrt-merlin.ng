// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Avionic Design GmbH <www.avionic-design.de>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include "pinmux-config-tamonten-ng.h"
#include <i2c.h>

#define PMU_I2C_ADDRESS		0x2D

#define PMU_REG_LDO5		0x32

#define PMU_REG_LDO_HIGH_POWER	1

/* Voltage selection for the LDOs with 100mV resolution */
#define PMU_REG_LDO_SEL_100(mV)	((((mV - 1000) / 100) + 2) << 2)

#define PMU_REG_LDO_100(st, mV)	(PMU_REG_LDO_##st | PMU_REG_LDO_SEL_100(mV))

#define PMU_LDO5(st, mV)	PMU_REG_LDO_100(st, mV)

void pinmux_init(void)
{
	pinmux_config_pingrp_table(tamonten_ng_pinmux_common,
		ARRAY_SIZE(tamonten_ng_pinmux_common));
	pinmux_config_pingrp_table(unused_pins_lowpower,
		ARRAY_SIZE(unused_pins_lowpower));

	/* Initialize any non-default pad configs (APB_MISC_GP regs) */
	pinmux_config_drvgrp_table(tamonten_ng_padctrl,
		ARRAY_SIZE(tamonten_ng_padctrl));
}

void gpio_early_init(void)
{
	/* Turn on the alive signal */
	gpio_request(TEGRA_GPIO(V, 2), "ALIVE");
	gpio_direction_output(TEGRA_GPIO(V, 2), 1);

	/* Remove the reset on the external periph */
	gpio_request(TEGRA_GPIO(I, 4), "nRST_PERIPH");
	gpio_direction_output(TEGRA_GPIO(I, 4), 1);
}

void pmu_write(uchar reg, uchar data)
{
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(4, PMU_I2C_ADDRESS, 1, &dev);
	if (ret) {
		debug("%s: Cannot find PMIC I2C chip\n", __func__);
		return;
	}
	dm_i2c_write(dev, reg, &data, 1);
}

/*
 * Do I2C/PMU writes to bring up SD card bus power
 *
 */
void board_sdmmc_voltage_init(void)
{
	/* Enable LDO5 with 3.3v for SDMMC3 */
	pmu_write(PMU_REG_LDO5, PMU_LDO5(HIGH_POWER, 3300));

	/* Switch the power on */
	gpio_request(TEGRA_GPIO(J, 2), "EN_3V3_EMMC");
	gpio_direction_output(TEGRA_GPIO(J, 2), 1);
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

	/* Bring up the SDIO1 power rail */
	board_sdmmc_voltage_init();
}
