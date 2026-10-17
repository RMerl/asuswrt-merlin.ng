// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Disruptive Technologies Research AS
 * Sven Schwermer <sven.svenschwermer@disruptive-technologies.com>
 */

#ifndef _REGULATOR_COMMON_H
#define _REGULATOR_COMMON_H

#include <asm/gpio.h>

struct regulator_common_plat {
	struct gpio_desc gpio; /* GPIO for regulator enable control */
	unsigned int startup_delay_us;
	unsigned int off_on_delay_us;
	unsigned int enable_count;
};

int regulator_common_of_to_plat(struct udevice *dev,
				struct regulator_common_plat *plat, const
				char *enable_gpio_name);
int regulator_common_get_enable(const struct udevice *dev,
	struct regulator_common_plat *plat);
/*
 * Enable or Disable a regulator
 *
 * This is a reentrant function and subsequent calls that enable will
 * increase an internal counter, and disable calls will decrease the counter.
 * The actual resource will be enabled when the counter gets to 1 coming from 0,
 * and disabled when it reaches 0 coming from 1.
 *
 * @dev: regulator device
 * @plat: Platform data
 * @enable: bool indicating whether to enable or disable the regulator
 * @return:
 * 0 on Success
 * -EBUSY if the regulator cannot be disabled because it's requested by
 *        another device
 * -EALREADY if the regulator has already been enabled or has already been
 *        disabled
 * -EACCES if there is no possibility to enable/disable the regulator
 * -ve on different error situation
 */
int regulator_common_set_enable(const struct udevice *dev,
	struct regulator_common_plat *plat, bool enable);

#endif /* _REGULATOR_COMMON_H */
