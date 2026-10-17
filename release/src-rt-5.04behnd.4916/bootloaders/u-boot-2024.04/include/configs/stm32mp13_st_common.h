/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STMicroelectronics STM32MP13x boards
 */

#ifndef __CONFIG_STM32MP13_ST_COMMON_H__
#define __CONFIG_STM32MP13_ST_COMMON_H__

#define STM32MP_BOARD_EXTRA_ENV \
	"usb_pgood_delay=2000\0" \
	"console=ttySTM0\0"

#include <configs/stm32mp13_common.h>

/* uart with on-board st-link */
#define CFG_SYS_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200, \
					 230400, 460800, 921600, \
					 1000000, 2000000, 4000000}

#endif
