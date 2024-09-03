/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :>
 
*/

#include "bcm_mcast_priv.h"

int bcm_mcast_blogrule_process(bcm_mcast_ifdata  *pif,
                               void              *mc_fdb,
                               int                proto,
                               int                blogProto,
                               blogRule_t       **list_rule_p,
                               struct hlist_head *headMcHash,
                               void              *arg_p,
                               int                lan2lan,
                               struct net_device *wan_vlan_dev_p,
                               struct net_device *lan_vlan_dev_p,
                               uintptr_t          flowhdl);

void bcm_mcast_blogrule_get_vlan_info(blogRule_t *rule_p, 
                                      uint8_t    *numtags, 
                                      uint32_t   *vlan0, 
                                      uint32_t   *vlan1);

uint16_t bcm_mcast_blogrule_fetch_lan_tci(blogRule_t *rule_p);

int bcm_mcast_blogrule_get_whitelist_vlan_info(struct net_device *from_dev,
                                               bcm_mcast_whitelist_info_t *whitelist_info_p);
