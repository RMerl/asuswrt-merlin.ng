/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include <configs/rk3036_common.h>

#ifndef CONFIG_SPL_BUILD

/* Store env in emmc */
#define CONFIG_SYS_MMC_ENV_DEV		0 /* emmc */
#define CONFIG_SYS_MMC_ENV_PART		0 /* user area */

#endif

#endif
