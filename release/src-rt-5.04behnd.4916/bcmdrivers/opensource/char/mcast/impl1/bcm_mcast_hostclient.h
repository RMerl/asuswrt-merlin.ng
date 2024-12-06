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
#ifndef _BCM_MCAST_HOSTCLIENT_H_
#define _BCM_MCAST_HOSTCLIENT_H_

int bcm_mcast_hostclient_init(void);
void bcm_mcast_hostclient_exit(void);
void bcm_mcast_nf_hook_processing(bcm_mcast_ipaddr_t *ipaddr,
                                  struct sk_buff *skb);

#if defined(CC_MCAST_HOST_CLIENT_SUPPORT)
typedef struct bcm_mcast_hc_wl_info
{
  unsigned int ifidx_cnt;
  unsigned int ifidx[BCM_MCAST_MAX_SRC_IF];
  bcm_mcast_whitelist_info_t whitelist_info;
} t_hostclient_wl_info;
typedef struct bcm_mcast_hostclient_entry 
{
  struct hlist_node  hlist;
  bcm_mcast_ipaddr_t grp;
  bcm_mcast_ipaddr_t src;
  int                is_ssm;
  int                refcnt;
#if defined(CC_MCAST_WHITELIST_SUPPORT)
  t_hostclient_wl_info hc_wl_info;
#endif
} t_hostclient_entry;
int bcm_mcast_hostclient_entry_add(bcm_mcast_ipaddr_t *grp,
                                   bcm_mcast_ipaddr_t *src,
                                   t_BCM_MCAST_WAN_INFO *waninfo_p);
int bcm_mcast_hostclient_entry_remove(bcm_mcast_ipaddr_t *grp,
                                      bcm_mcast_ipaddr_t *src);
t_hostclient_entry *bcm_mcast_hostclient_entry_lookup(bcm_mcast_ipaddr_t *grp, 
                                                      bcm_mcast_ipaddr_t *src);
void bcm_mcast_hostclient_entries_dump(void);
void bcm_mcast_hostclient_lkup_and_update_flow_igmp(struct in_addr *grp,
                                                    struct in_addr *src);
void bcm_mcast_hostclient_lkup_and_update_flow_mld(struct in6_addr *grp,
                                                   struct in6_addr *src);
#endif
#endif
