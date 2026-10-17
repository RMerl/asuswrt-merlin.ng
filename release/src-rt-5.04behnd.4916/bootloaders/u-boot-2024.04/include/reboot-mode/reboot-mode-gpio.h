/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) Vaisala Oyj.
 */

#ifndef REBOOT_MODE_REBOOT_MODE_GPIO_H_
#define REBOOT_MODE_REBOOT_MODE_GPIO_H_

#include <asm/gpio.h>

/*
 * In case of initializing the driver statically (using U_BOOT_DEVICE macro),
 * we can use this struct to declare the pins used.
 */

#if !CONFIG_IS_ENABLED(OF_CONTROL)
struct reboot_mode_gpio_config {
	int gpio_dev_offset;
	int gpio_offset;
	int flags;
};
#endif

struct reboot_mode_gpio_platdata {
	struct gpio_desc *gpio_desc;
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	struct reboot_mode_gpio_config *gpios_config;
#endif
	int gpio_count;
};

#endif /* REBOOT_MODE_REBOOT_MODE_GPIO_H_ */
