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
#ifndef _GPIO_OMAP4_H
#define _GPIO_OMAP4_H

#include <asm/omap_gpio.h>

#define OMAP_MAX_GPIO			192

#define OMAP44XX_GPIO1_BASE		0x4A310000
#define OMAP44XX_GPIO2_BASE		0x48055000
#define OMAP44XX_GPIO3_BASE		0x48057000
#define OMAP44XX_GPIO4_BASE		0x48059000
#define OMAP44XX_GPIO5_BASE		0x4805B000
#define OMAP44XX_GPIO6_BASE		0x4805D000

#endif /* _GPIO_OMAP4_H */
