/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIGS_PX5_EVB_H
#define __CONFIGS_PX5_EVB_H

#include <configs/rk3368_common.h>

#define CONFIG_SYS_MMC_ENV_DEV		0
#define KERNEL_LOAD_ADDR		0x280000
#define DTB_LOAD_ADDR			0x5600000
#define INITRD_LOAD_ADDR		0x5bf0000

#define CONFIG_CONSOLE_SCROLL_LINES	10

#endif
