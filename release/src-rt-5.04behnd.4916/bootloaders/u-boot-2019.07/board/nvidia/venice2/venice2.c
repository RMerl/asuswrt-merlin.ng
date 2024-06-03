// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include "pinmux-config-venice2.h"

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_set_tristate_input_clamping();

	gpio_config_table(venice2_gpio_inits,
			  ARRAY_SIZE(venice2_gpio_inits));

	pinmux_config_pingrp_table(venice2_pingrps,
				   ARRAY_SIZE(venice2_pingrps));

	pinmux_config_drvgrp_table(venice2_drvgrps,
				   ARRAY_SIZE(venice2_drvgrps));
}
