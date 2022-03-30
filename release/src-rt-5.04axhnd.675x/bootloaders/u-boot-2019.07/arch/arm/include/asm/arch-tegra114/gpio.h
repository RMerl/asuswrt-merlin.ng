/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef _TEGRA114_GPIO_H_
#define _TEGRA114_GPIO_H_

/*
 * The Tegra114 GPIO controller has 246 GPIOS in 8 banks of 4 ports,
 * each with 8 GPIOs.
 */
#define TEGRA_GPIO_PORTS	4	/* number of ports per bank */
#define TEGRA_GPIO_BANKS	8	/* number of banks */

#include <asm/arch-tegra/gpio.h>
#include <asm/arch-tegra30/gpio.h>

#endif	/* _TEGRA114_GPIO_H_ */
