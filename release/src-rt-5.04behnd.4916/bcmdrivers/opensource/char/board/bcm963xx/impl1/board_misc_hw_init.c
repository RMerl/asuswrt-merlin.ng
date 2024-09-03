/*
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
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
 */

// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg


#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
#include <linux/dma-map-ops.h>
#endif
#include <linux/delay.h>
#include "pmc_ssb_access.h"
#include "bcm_misc_hw_init.h"

int bcm_misc_hw_init(void)
{
    /* Set 1.0V, 1.5V and 1.8V  digital voltage switching regulator's gain setting  based on 
       JIRA SWBCACPE-18708 and SWBCACPE-18709 */
#if defined(CONFIG_BCM963148)
    write_ssbm_reg(0x0, 0x840, 1);  /* Set 1.0V rail to the gain of 16*/
    write_ssbm_reg(0x20, 0x830, 1); /* Set 1.8V rail to the gain of 4*/
    write_ssbm_reg(0x40, 0x800, 1); /* Set 1.5V rail to the gain of 8*/
#endif
#if defined(CONFIG_BCM963138)
    write_ssbm_reg(0x0, 0x800, 1);  /* Set 1.0V rail to the gain of 8*/
    write_ssbm_reg(0x20, 0x830, 1); /* Set 1.8V rail to the gain of 4*/
    write_ssbm_reg(0x40, 0x800, 1); /* Set 1.5V rail to the gain of 8*/
    /* 63138 over current watchdog enabling. It must be done after all switch regulator register is set 
     * This is only needed if we use internal voltage regulator. Read 1.5V rail, if it is zero, the board
     * use external voltage regulator and do NOT need to enable the watchdog */
    if( read_ssbm_reg(0x40) != 0 ) {
        udelay(1000);
        enable_over_current_watchdog();
    }
#endif

    return 0;
}

arch_initcall(bcm_misc_hw_init);
