/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#include <linux/init.h>
#include <linux/kernel.h>
#include "power_rpc_svc.h"
#include "bcm_memc.h"

static int g_threshold = -1;

int enable_ddr_auto_self_refresh(unsigned int thre)
{
    int ret = 0;
	unsigned int mc_clk_mhz, thresh_new;

	if (bcm_memc_get_spd_mhz(&mc_clk_mhz))
	{
		printk("[%s:%d] Error!!",__FUNCTION__,__LINE__);
		return -1;
	}
	thresh_new = (thre * mc_clk_mhz) & 0x3fffffff;

	printk("Sending RPC thre = %d, mc_clk_mhz = %d\n", thre, mc_clk_mhz);   // FIXMET

    ret = bcm_rpc_pwr_en_ddr_auto_self_refresh(thresh_new);

    if (ret)
        printk("[%s:%d] Error!! ret[%d]",__FUNCTION__,__LINE__,ret);
    else
        g_threshold = thre;

    return ret;
}

int disable_ddr_auto_self_refresh(void)
{
    int ret = 0;

	printk("Disable auto self refresh\n");

    ret = bcm_rpc_pwr_dis_ddr_auto_self_refresh();
    if (ret)
        printk("[%s:%d] Error!! ret[%d]",__FUNCTION__,__LINE__,ret);
    else
        g_threshold = -1;

    return ret;
}

int ddr_auto_self_refresh_thres_get(void)
{
    return g_threshold;
}

