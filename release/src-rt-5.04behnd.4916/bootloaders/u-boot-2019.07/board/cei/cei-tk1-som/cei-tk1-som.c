// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <power/as3722.h>

#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>

#include "pinmux-config-cei-tk1-som.h"

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_clear_tristate_input_clamping();

	gpio_config_table(cei_tk1_som_gpio_inits,
			  ARRAY_SIZE(cei_tk1_som_gpio_inits));

	pinmux_config_pingrp_table(cei_tk1_som_pingrps,
				   ARRAY_SIZE(cei_tk1_som_pingrps));

	pinmux_config_drvgrp_table(cei_tk1_som_drvgrps,
				   ARRAY_SIZE(cei_tk1_som_drvgrps));

	pinmux_config_mipipadctrlgrp_table(cei_tk1_som_mipipadctrlgrps,
                                           ARRAY_SIZE(cei_tk1_som_mipipadctrlgrps));
}

#ifdef CONFIG_PCI_TEGRA
int tegra_pcie_board_init(void)
{
/* TODO: Convert to driver model
	struct udevice *pmic;
	int err;

	err = as3722_init(&pmic);
	if (err) {
		error("failed to initialize AS3722 PMIC: %d\n", err);
		return err;
	}

	err = as3722_sd_enable(pmic, 4);
	if (err < 0) {
		error("failed to enable SD4: %d\n", err);
		return err;
	}

	err = as3722_sd_set_voltage(pmic, 4, 0x24);
	if (err < 0) {
		error("failed to set SD4 voltage: %d\n", err);
		return err;
	}
*/

	return 0;
}
#endif /* PCI */
