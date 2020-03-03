#if defined(CONFIG_BCM_KF_MIPS_BCM963XX)
/***********************************************************
 *
 * Copyright (c) 2009 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2009:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license 
 * agreement governing use of this software, this software is licensed 
 * to you under the terms of the GNU General Public License version 2 
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, 
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give 
 *    you permission to link this software with independent modules, and 
 *    to copy and distribute the resulting executable under terms of your 
 *    choice, provided that you also meet, for each linked independent 
 *    module, the terms and conditions of the license of that module. 
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications 
 *    of the software.  
 * 
 * Not withstanding the above, under no circumstances may you combine 
 * this software in any way with any other Broadcom software provided 
 * under a license other than the GPL, without Broadcom's express prior 
 * written consent. 
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
