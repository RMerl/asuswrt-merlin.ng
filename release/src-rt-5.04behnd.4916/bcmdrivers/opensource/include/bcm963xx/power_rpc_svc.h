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
