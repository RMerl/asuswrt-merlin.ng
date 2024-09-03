/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "pmc_drv.h"
#include "pmc_rccore.h"
#include <board.h>


char rccore_domain_name[PMC_RCCORE_MAX][PWR_DOMAIN_NAME_MAX_LEN+1] = 
{
						"rccore0", 
						"rccore1", 
						"rccore2", 
						"rccore3",
#if defined(CONFIG_BCM968880)
						"rccore4", 
						"rccore5", 
						"rccore6"
#endif
}; // One zone per each runner pair 

int pmc_rccore_power_up(unsigned int rccore_num)
{
    int ret = 0;

	if (rccore_num >= PMC_RCCORE_MAX)
	{    
        printk("[%s:%d] ERROR rccore_num[%d]\n",__FILE__, __LINE__, rccore_num);
        return -1;    
	}

    ret = bcm_rpc_pwr_set_domain_state(rccore_domain_name[rccore_num], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
    return ret;
}
EXPORT_SYMBOL(pmc_rccore_power_up);

int pmc_rccore_power_down(unsigned int rccore_num)
{
    int ret = 0;

	if (rccore_num >= PMC_RCCORE_MAX)
	{
		printk("[%s:%d] ERROR rccore_num[%d]\n",__FILE__, __LINE__, rccore_num);
		return -1;   
	}         

    ret = bcm_rpc_pwr_set_domain_state(rccore_domain_name[rccore_num], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
    return ret;
}
EXPORT_SYMBOL(pmc_rccore_power_down);