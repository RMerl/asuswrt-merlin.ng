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
#include "pmc_drv.h"
#include "pmc_cpu.h"
#include "pmc_drv.h"

#define CPU_NUM_MAX  (4)
char cpu_domain_name[CPU_NUM_MAX][PWR_DOMAIN_NAME_MAX_LEN+1] = { "a55core0", "a55core1", "a55core2", "a55core3"};


int pmc_cpu_init(unsigned int cpu_id)
{
    int ret = 0;

    if (cpu_id >= CPU_NUM_MAX)
    {
        printk("%s: ERROR: cpu_id[%d] > CPU_NUM_MAX[4]\n",__FUNCTION__, cpu_id);
        return -1;
    }

   ret = bcm_rpc_pwr_set_domain_state(cpu_domain_name[cpu_id], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
   
   return ret;
}

int pmc_cpu_shutdown(unsigned int cpu_id)
{
    int ret = 0;

    if (cpu_id > CPU_NUM_MAX)
    {
        printk("%s: ERROR: cpu_id[%d] > CPU_NUM_MAX[4]\n",__FUNCTION__, cpu_id);
        return -1;
    }

   ret = bcm_rpc_pwr_set_domain_state(cpu_domain_name[cpu_id], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
   return ret;
}

