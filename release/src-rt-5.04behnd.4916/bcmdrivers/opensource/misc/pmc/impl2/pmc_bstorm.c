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
 * Author: Akiva Sadovski <akiva.sadovski@broadcom.com>
*****************************************************************************/
#include "pmc_drv.h"
#include "pmc_cpu.h"
#include "pmc_drv.h"
#include <linux/delay.h>


int pmc_ml_init(void)
{
    int ret = 0;

    ret = bcm_rpc_pwr_set_domain_state("bstm", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
    if (ret == -1)
        printk("%s:%d: ret[%d]\n",__FUNCTION__ ,__LINE__, ret);

    mdelay(1);
    return ret;
}
EXPORT_SYMBOL(pmc_ml_init);

int pmc_ml_shutdown(void)
{
    int ret = 0;

    ret = bcm_rpc_pwr_set_domain_state("bstm", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
    if (ret == -1)
        printk("%s:%d: ret[%d]\n",__FUNCTION__ ,__LINE__, ret);

    return ret;
}
EXPORT_SYMBOL(pmc_ml_shutdown);

#if defined(CONFIG_BCM968880)
int pmc_v3d_init(void)
{
     int ret = 0;

    ret = bcm_rpc_pwr_set_domain_state("v3d", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
    if (ret == -1)
        printk("%s:%d: ret[%d]\n",__FUNCTION__ ,__LINE__, ret);

    return ret;
}
EXPORT_SYMBOL(pmc_v3d_init);

int pmc_v3d_shutdown(void)
{
     int ret = 0;

    ret = bcm_rpc_pwr_set_domain_state("v3d", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
    if (ret == -1)
        printk("%s:%d: ret[%d]\n",__FUNCTION__ ,__LINE__, ret);

    return ret;
}
EXPORT_SYMBOL(pmc_v3d_shutdown);
#endif
