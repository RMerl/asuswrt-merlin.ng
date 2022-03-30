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
#ifndef _GPIO_H
#define _GPIO_H

#include <asm/arch/cpu.h>

#ifdef CONFIG_DM_GPIO

/* Information about a GPIO bank */
struct omap_gpio_platdata {
	int bank_index;
	ulong base;	/* address of registers in physical memory */
	const char *port_name;
};

#else

struct gpio_bank {
	void *base;
};

extern const struct gpio_bank *const omap_gpio_bank;

/**
 * Check if gpio is valid.
 *
 * @param gpio	GPIO number
 * @return 1 if ok, 0 on error
 */
int gpio_is_valid(int gpio);
#endif

#endif /* _GPIO_H_ */
