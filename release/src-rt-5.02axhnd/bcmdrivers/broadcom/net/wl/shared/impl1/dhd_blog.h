/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

#ifndef __DHD_BLOG_H__
#define __DHD_BLOG_H__

#if defined(BCM_BLOG)

#include <linux/blog.h>
#include <dngl_stats.h>
#include <dhd.h>

#define DHD_BLOG_SKIP(skb, reason) blog_skip((skb), (reason));

struct dhd_pub;

extern struct dhd_pub *g_dhd_info[];

int dhd_handle_blog_sinit(struct dhd_pub *dhdp, int ifidx, struct sk_buff *skb);

int dhd_handle_blog_emit(struct dhd_pub *dhdp, struct net_device *net, int ifidx,
	void *pktbuf, int b_wmf_unicast);

extern int (*fdb_check_expired_dhd_hook)(unsigned char *addr);
int fdb_check_expired_dhd(uint8 *addr);

#if defined(BCM_WFD)
extern int
dhd_handle_wfd_blog(struct dhd_pub *dhdp, struct net_device *net, int ifidx,
	void *pktbuf, int b_wmf_unicast);
#endif /* BCM_WFD */

void dhd_handle_blog_disconnect_event(struct dhd_pub *dhdp, wl_event_msg_t *event);
extern void dhd_if_inc_rxpkt_cnt(dhd_pub_t *dhdp, int ifidx, uint32 pktlen);

extern int
dhd_blog_flush_flowring(struct dhd_pub *dhdp, uint16 flowid);

#endif /* BCM_BLOG */

#ifndef BCM_COUNTER_EXTSTATS
void net_dev_clear_stats64(struct net_device *dev_p);
struct rtnl_link_stats64 *net_dev_collect_stats64(struct net_device *dev_p,struct rtnl_link_stats64 *stats);
#endif

void dhd_if_inc_txpkt_mcnt(dhd_pub_t *dhdp, int ifidx, int pktlen);
#endif /* __DHD_BLOG_H__ */
