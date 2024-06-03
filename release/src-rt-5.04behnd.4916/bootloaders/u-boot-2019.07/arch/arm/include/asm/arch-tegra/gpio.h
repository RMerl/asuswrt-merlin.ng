/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011, Google Inc. All rights reserved.
 */

#ifndef _TEGRA_GPIO_H_
#define _TEGRA_GPIO_H_

#include <dt-bindings/gpio/tegra-gpio.h>

#define TEGRA_GPIOS_PER_PORT	8
#define TEGRA_PORTS_PER_BANK	4
#define MAX_NUM_GPIOS           (TEGRA_GPIO_PORTS * TEGRA_GPIO_BANKS * 8)
#define GPIO_NAME_SIZE		20	/* gpio_request max label len */

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_FULLPORT(x)	((x) >> 3)
#define GPIO_BIT(x)		((x) & 0x7)

enum tegra_gpio_init {
	TEGRA_GPIO_INIT_IN,
	TEGRA_GPIO_INIT_OUT0,
	TEGRA_GPIO_INIT_OUT1,
};

struct tegra_gpio_config {
	u32 gpio:16;
	u32 init:2;
};

/**
 * Configure a list of GPIOs
 *
 * @param config	List of GPIO configurations
 * @param len		Number of config items in list
 */
void gpio_config_table(const struct tegra_gpio_config *config, int len);

#endif	/* TEGRA_GPIO_H_ */
