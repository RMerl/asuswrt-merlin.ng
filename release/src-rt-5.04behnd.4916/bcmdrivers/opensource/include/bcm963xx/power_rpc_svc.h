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
 * Power RPC Driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#ifndef _POWER_RPC_SVC_H_
#define _POWER_RPC_SVC_H_

#include <itc_rpc.h>

#define PWR_DOMAIN_NAME_MAX_LEN         (8)
#define RPC_SERVICE_VER_PWR_DOMAIN_ID           (0)
#define RPC_SERVICE_VER_PWR_DOMAIN_NAME         (0)
#define RPC_SERVICE_VER_PWR_GET_DOMAIN_STATE    (0)
#define RPC_SERVICE_VER_PWR_SET_DOMAIN_STATE    (0)

typedef enum  
{
   PWR_DOM_STATE_OFF       = 0,
   PWR_DOM_STATE_ON        = 0xf,
   PWR_DOM_STATE_UNCHANGED = 0xfe,
   PWR_DOM_STATE_UNKNOWN   = 0xff
}pwr_dom_state;

typedef enum  
{
   PWR_DOM_RESET_DEASSERT  = 0,
   PWR_DOM_RESET_ASSERT    = 1,
   PWR_DOM_RESET_PULSE     = 2,
   PWR_DOM_RESET_MAX
}pwr_dom_reset;


int bcm_rpc_pwr_set_domain_state(char *name, uint8_t name_size, pwr_dom_state state, pwr_dom_reset reset);
int bcm_rpc_pwr_get_domain_state(char *name, uint8_t name_size, pwr_dom_state *state, pwr_dom_reset *reset);

#endif
