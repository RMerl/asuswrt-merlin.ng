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

#include "ba_svc.h"
#include <linux/types.h>
#include <linux/delay.h>
#include <api_public.h>
#include <string.h>
#include <stdio.h>
#include "pmc_ethtop.h"

#define XPORT_NUM_MAX  (3)

int pmc_xport_power_on(int xport_id)
{
    int ret = 0;

    if (xport_id > XPORT_NUM_MAX)
    {
      printf("%s: ERROR: xport_id[%d] > XPORT_NUM_MAX[3]\n",__FUNCTION__, xport_id);
      return -1;
    }

    ret |= pmc_ethtop_power_up(ETHTOP_XPORT0 + xport_id);
    ret |= ba_xport_set_state(xport_id, 1);

    return ret;
}
