/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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

#ifndef _BCM_MCAST_WHITELIST_H_
#define _BCM_MCAST_WHITELIST_H_

typedef struct bcm_mcast_whitelist_info
{
   int                is_ssm;
   bcm_mcast_ipaddr_t grp;
   bcm_mcast_ipaddr_t src;
   uint16_t           outer_vlanid; 
} bcm_mcast_whitelist_info_t;

typedef uint32_t whitelist_key_t;

typedef int (*bcm_mcast_whitelist_add_hook_t)(bcm_mcast_whitelist_info_t *pWhitelistInfo,
                                              whitelist_key_t            *whitelist_hdl);
typedef int (*bcm_mcast_whitelist_delete_hook_t)(whitelist_key_t whitelist_hdl);

extern bcm_mcast_whitelist_add_hook_t bcm_mcast_whitelist_add_fn;
extern bcm_mcast_whitelist_delete_hook_t bcm_mcast_whitelist_delete_fn;


int bcm_mcast_whitelist_add(bcm_mcast_whitelist_info_t *pWhitelistInfo,
                            whitelist_key_t            *whitelist_hdl);
int bcm_mcast_whitelist_delete(whitelist_key_t whitelist_hdl);
int bcm_mcast_whitelist_init(void);
void bcm_mcast_whitelist_exit(void);

#endif
