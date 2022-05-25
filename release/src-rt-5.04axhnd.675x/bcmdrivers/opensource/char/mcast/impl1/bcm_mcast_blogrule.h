/*
* <:copyright-BRCM:2020:DUAL/GPL:standard
* 
*    Copyright (c) 2020 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
