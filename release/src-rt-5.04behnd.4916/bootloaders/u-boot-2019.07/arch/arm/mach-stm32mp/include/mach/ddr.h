/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#ifndef __MACH_STM32MP_DDR_H_
#define __MACH_STM32MP_DDR_H_

/* DDR power initializations */
enum ddr_type {
	STM32MP_DDR3,
	STM32MP_LPDDR2,
	STM32MP_LPDDR3,
};

int board_ddr_power_init(enum ddr_type ddr_type);

#endif
