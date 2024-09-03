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
