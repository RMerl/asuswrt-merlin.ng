/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 * Portions Copyright 2011-2012 NVIDIA Corporation
 */

#ifndef _TEGRA20_GPIO_H_
#define _TEGRA20_GPIO_H_

/*
 * The Tegra 2x GPIO controller has 224 GPIOs arranged in 7 banks of 4 ports,
 * each with 8 GPIOs.
 */
#define TEGRA_GPIO_PORTS	4	/* number of ports per bank */
#define TEGRA_GPIO_BANKS	7	/* number of banks */

#include <asm/arch-tegra/gpio.h>

/* GPIO Controller registers for a single bank */
struct gpio_ctlr_bank {
	uint gpio_config[TEGRA_GPIO_PORTS];
	uint gpio_dir_out[TEGRA_GPIO_PORTS];
	uint gpio_out[TEGRA_GPIO_PORTS];
	uint gpio_in[TEGRA_GPIO_PORTS];
	uint gpio_int_status[TEGRA_GPIO_PORTS];
	uint gpio_int_enable[TEGRA_GPIO_PORTS];
	uint gpio_int_level[TEGRA_GPIO_PORTS];
	uint gpio_int_clear[TEGRA_GPIO_PORTS];
};

struct gpio_ctlr {
	struct gpio_ctlr_bank gpio_bank[TEGRA_GPIO_BANKS];
};

#endif	/* TEGRA20_GPIO_H_ */
