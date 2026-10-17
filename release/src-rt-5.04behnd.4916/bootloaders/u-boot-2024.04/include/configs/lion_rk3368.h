/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 */

#ifndef __CONFIGS_LION_RK3368_H
#define __CONFIGS_LION_RK3368_H

#include <configs/rk3368_common.h>

#define KERNEL_LOAD_ADDR		0x280000
#define DTB_LOAD_ADDR			0x5600000
#define INITRD_LOAD_ADDR		0x5bf0000
/* PHY needs longer aneg time at 1G */
#define PHY_ANEG_TIMEOUT		8000

#endif
