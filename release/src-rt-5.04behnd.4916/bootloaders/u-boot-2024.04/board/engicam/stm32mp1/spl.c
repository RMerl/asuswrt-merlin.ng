// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Copyright (C) 2020 Engicam S.r.l.
 * Copyright (C) 2020 Amarula Solutions(India)
 */

#include <common.h>

/* board early initialisation in board_f: need to use global variable */
static u32 opp_voltage_mv __section(".data");

void board_vddcore_init(u32 voltage_mv)
{
	if (IS_ENABLED(CONFIG_PMIC_STPMIC1) && CONFIG_IS_ENABLED(POWER))
		opp_voltage_mv = voltage_mv;
}

int board_early_init_f(void)
{
	return 0;
}

