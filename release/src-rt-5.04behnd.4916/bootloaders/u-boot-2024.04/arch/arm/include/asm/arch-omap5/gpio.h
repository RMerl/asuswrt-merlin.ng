/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This work is derived from the linux 2.6.27 kernel source
 * To fetch, use the kernel repository
 * git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
 * Use the v2.6.27 tag.
 *
 * Below is the original's header including its copyright
 *
 *  linux/arch/arm/plat-omap/gpio.c
 *
 * Support functions for OMAP GPIO
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 * Written by Juha Yrjölä <juha.yrjola@nokia.com>
 */
#ifndef _GPIO_OMAP5_H
#define _GPIO_OMAP5_H

#include <asm/omap_gpio.h>

#define OMAP_MAX_GPIO			256

#define OMAP54XX_GPIO1_BASE		0x4Ae10000
#define OMAP54XX_GPIO2_BASE		0x48055000
#define OMAP54XX_GPIO3_BASE		0x48057000
#define OMAP54XX_GPIO4_BASE		0x48059000
#define OMAP54XX_GPIO5_BASE		0x4805B000
#define OMAP54XX_GPIO6_BASE		0x4805D000
#define OMAP54XX_GPIO7_BASE		0x48051000
#define OMAP54XX_GPIO8_BASE		0x48053000


/* Get the GPIO index from the given bank number and bank gpio */
#define GPIO_TO_PIN(bank, bank_gpio)	(32 * (bank - 1) + (bank_gpio))

#endif /* _GPIO_OMAP5_H */
