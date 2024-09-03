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
#include "pmc_spu.h"
#include "bcm_ubus4.h"

int pmc_spu_power_up(void)
{
   int ret = 0;

   ret = bcm_rpc_pwr_set_domain_state("spu", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
   return ret;
}

int pmc_spu_power_down(void)
{
   int ret = 0;

   ret = bcm_rpc_pwr_set_domain_state("spu", PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
   return ret;
}

EXPORT_SYMBOL(pmc_spu_power_up);
EXPORT_SYMBOL(pmc_spu_power_down);

