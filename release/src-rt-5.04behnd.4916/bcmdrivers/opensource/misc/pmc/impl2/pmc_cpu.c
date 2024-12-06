/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
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

