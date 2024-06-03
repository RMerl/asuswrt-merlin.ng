/*
 * <:copyright-BRCM:2015:DUAL/GPL:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
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
