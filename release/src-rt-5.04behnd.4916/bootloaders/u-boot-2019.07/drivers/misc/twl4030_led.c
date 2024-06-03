// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix at windriver.com>
 *
 * twl4030_led_init is from cpu/omap3/common.c, power_init_r
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05 at gmail.com>
 *	Shashi Ranjan <shashiranjanmca05 at gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2 at ti.com>
 *	Syed Mohammed Khasim <khasim at ti.com>
 */

#include <twl4030.h>

void twl4030_led_init(unsigned char ledon_mask)
{
	/* LEDs need to have corresponding PWMs enabled */
	if (ledon_mask & TWL4030_LED_LEDEN_LEDAON)
		ledon_mask |= TWL4030_LED_LEDEN_LEDAPWM;
	if (ledon_mask & TWL4030_LED_LEDEN_LEDBON)
		ledon_mask |= TWL4030_LED_LEDEN_LEDBPWM;

	twl4030_i2c_write_u8(TWL4030_CHIP_LED, TWL4030_LED_LEDEN,
			     ledon_mask);

}
