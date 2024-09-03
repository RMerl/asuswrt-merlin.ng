/*
*    Copyright (c) 2015 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef _BCM_MCAST_BLOG_H_
#define _BCM_MCAST_BLOG_H_

#if defined(CONFIG_BLOG)
int bcm_mcast_blog_get_wlan_reporter_info(struct net_device *repDev, unsigned char *repMac, uint32_t *info);
int bcm_mcast_blog_cmp_wlan_reporter_info(struct net_device *repDev, uint32_t info_1, uint32_t info_2);
int bcm_mcast_blog_fill_rx_info(struct net_device *from_dev,
                                Blog_t *blog_p,
                                struct net_device **root_dev);
#else
#define bcm_mcast_blog_cmp_wlan_reporter_info(a, b, c) 0
static inline int bcm_mcast_blog_get_wlan_reporter_info(struct net_device *repDev, unsigned char *repMac, uint32_t *info)
{
    *info = 0;
    return 0;
}
#endif
void bcm_mcast_blog_release(int proto, bcm_mcast_flowkey_t flow_key);
int bcm_mcast_blog_process(bcm_mcast_ifdata *pif, 
                           void *mc_fdb, 
                           int proto, 
                           struct hlist_head *headMcHash,
                           uintptr_t flowhdl);

__init int bcm_mcast_blog_init(void);
void bcm_mcast_blog_exit(void);

/* XXX: Later, the function prototype will be made independent of Blog */
#if defined(CONFIG_BLOG)
typedef int (*bcm_mcast_flow_add_hook_t)(Blog_t * blog_p, BlogTraffic_t traffic, bcm_mcast_flowkey_t *hdl_p);
typedef Blog_t* (*bcm_mcast_flow_delete_hook_t)(bcm_mcast_flowkey_t key, BlogTraffic_t traffic);
#else
typedef int (*bcm_mcast_flow_add_hook_t)(void * blog_p, int traffic, bcm_mcast_flowkey_t *hdl_p);
typedef void* (*bcm_mcast_flow_delete_hook_t)(bcm_mcast_flowkey_t key, int traffic);
#endif

extern bcm_mcast_flow_add_hook_t bcm_mcast_flow_add_hook;
extern bcm_mcast_flow_delete_hook_t bcm_mcast_flow_delete_hook;

#endif /* _BCM_MCAST_BLOG_H_ */