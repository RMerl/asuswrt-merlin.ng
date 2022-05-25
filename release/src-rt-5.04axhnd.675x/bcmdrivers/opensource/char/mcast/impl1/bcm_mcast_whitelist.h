/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
