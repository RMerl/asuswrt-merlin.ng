#if defined(CONFIG_BCM_KF_MIPS_BCM963XX)
/***********************************************************
 *
 * Copyright (c) 2009 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2009:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ************************************************************/
#include <linux/clocksource.h>
#include <linux/init.h>

#include <asm/time.h>
#include <bcm_map_part.h>

static cycle_t timer2_hpt_read(struct clocksource *cs)
{
    return (TIMER->TimerCnt2);
}

static struct clocksource clocksource_mips = {
	.name		= "MIPS",
	.read		= timer2_hpt_read,
	.mask		= CLOCKSOURCE_MASK(30),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

int __init init_r4k_clocksource(void)
{
	if (!cpu_has_counter || !mips_hpt_frequency)
		return -ENXIO;

	/* Calculate a somewhat reasonable rating value */
	clocksource_mips.rating = 300;

	clocksource_register_hz(&clocksource_mips, 50000000);

	return 0;
}
#endif
