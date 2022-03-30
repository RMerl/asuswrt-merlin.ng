// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <dm.h>
#include <power/as3722.h>
#include <power/pmic.h>

#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>

#include "pinmux-config-jetson-tk1.h"

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_clear_tristate_input_clamping();

	gpio_config_table(jetson_tk1_gpio_inits,
			  ARRAY_SIZE(jetson_tk1_gpio_inits));

	pinmux_config_pingrp_table(jetson_tk1_pingrps,
				   ARRAY_SIZE(jetson_tk1_pingrps));

	pinmux_config_drvgrp_table(jetson_tk1_drvgrps,
				   ARRAY_SIZE(jetson_tk1_drvgrps));

	pinmux_config_mipipadctrlgrp_table(jetson_tk1_mipipadctrlgrps,
					ARRAY_SIZE(jetson_tk1_mipipadctrlgrps));
}

#ifdef CONFIG_PCI_TEGRA
/* TODO: Convert to driver model */
static int as3722_sd_enable(struct udevice *pmic, unsigned int sd)
{
	int err;

	if (sd > 6)
		return -EINVAL;

	err = pmic_clrsetbits(pmic, AS3722_SD_CONTROL, 0, 1 << sd);
	if (err) {
		pr_err("failed to update SD control register: %d", err);
		return err;
	}

	return 0;
}

int tegra_pcie_board_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(pmic_as3722), &dev);
	if (ret) {
		debug("%s: Failed to find PMIC\n", __func__);
		return ret;
	}

	ret = as3722_sd_enable(dev, 4);
	if (ret < 0) {
		pr_err("failed to enable SD4: %d\n", ret);
		return ret;
	}

	ret = as3722_sd_set_voltage(dev, 4, 0x24);
	if (ret < 0) {
		pr_err("failed to set SD4 voltage: %d\n", ret);
		return ret;
	}

	return 0;
}
#endif /* PCI */
