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
#include "pmc_xrdp.h"
#include "power_rpc_svc.h"
#include <stdio.h>
#include <string.h>

int pmc_xrdp_init(void)
{
    int ret = 0;

    ret = bcm_rpc_pwr_set_domain_state("ethtop", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
    if (ret)
    {
      printf("%s:%d : Error:  call pwr_set_domain_state(ethtop)\n",__FUNCTION__, __LINE__);
      return -1;
    }    

    ret = bcm_rpc_pwr_set_domain_state("xrdp", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
    return ret;
}

int pmc_xrdp_shutdown(void)
{
    int ret = 0;

    ret = bcm_rpc_pwr_set_domain_state("xrdp", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
    return ret;
}

int pmc_xrdp_reset(void)
{
    return 0;
}

