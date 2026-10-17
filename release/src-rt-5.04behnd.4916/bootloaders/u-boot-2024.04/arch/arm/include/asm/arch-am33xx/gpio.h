/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _GPIO_AM33xx_H
#define _GPIO_AM33xx_H

#include <asm/omap_gpio.h>

#ifdef CONFIG_AM43XX
#define OMAP_MAX_GPIO		192
#else
#define OMAP_MAX_GPIO		128
#endif

#define AM33XX_GPIO0_BASE       0x44E07000
#define AM33XX_GPIO1_BASE       0x4804C000
#define AM33XX_GPIO2_BASE       0x481AC000
#define AM33XX_GPIO3_BASE       0x481AE000
#define AM33XX_GPIO4_BASE	0x48320000
#define AM33XX_GPIO5_BASE	0x48322000

/* GPIO CTRL register */
#define GPIO_CTRL_DISABLEMODULE_SHIFT	0
#define GPIO_CTRL_DISABLEMODULE_MASK	(1 << 0)
#define GPIO_CTRL_ENABLEMODULE		GPIO_CTRL_DISABLEMODULE_MASK

/* GPIO OUTPUT ENABLE register */
#define GPIO_OE_ENABLE(x)		(1 << x)

/* GPIO SETDATAOUT register */
#define GPIO_SETDATAOUT(x)		(1 << x)
#endif /* _GPIO_AM33xx_H */
