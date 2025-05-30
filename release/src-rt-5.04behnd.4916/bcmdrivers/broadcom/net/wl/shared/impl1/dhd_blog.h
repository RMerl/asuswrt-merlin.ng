/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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

#ifndef __DHD_BLOG_H__
#define __DHD_BLOG_H__

#if defined(BCM_BLOG)

#include <linux/blog.h>
#include <dngl_stats.h>
#include <dhd.h>

/* Blog handling for station connect event */
#define DHD_BLOG_CONNECT_EVENT

#define DHD_BLOG_SKIP(skb, reason) blog_skip((skb), (reason));

struct dhd_pub;

int dhd_handle_blog_sinit(struct dhd_pub *dhdp, int ifidx, struct sk_buff *skb);
#ifdef BCM_NBUFF_PKT_BPM
int dhd_handle_blog_finit(struct dhd_pub *dhdp, int ifidx, struct fkbuff *fkb,
        struct net_device *dev);
#endif /* BCM_NBUFF_PKT_BPM */

int dhd_handle_blog_emit(struct dhd_pub *dhdp, struct net_device *net, int ifidx,
	void *pktbuf, int b_wmf_unicast);

#if defined(BCM_BLOG_UNKNOWN_UCAST)
int dhd_handle_blog_emit_drop(struct dhd_pub *dhdp, struct net_device *net,
	int ifidx, void *pktbuf);
#endif /* BCM_BLOG_UNKNOWN_UCAST */

extern int (*fdb_check_expired_dhd_hook)(unsigned char * addr,
                                         struct net_device * net_device);
int fdb_check_expired_dhd(uint8 * addr, struct net_device * net_device);

#if defined(BCM_WFD)
extern int
dhd_handle_wfd_blog(struct dhd_pub *dhdp, struct net_device *net, int ifidx,
	void *pktbuf, int b_wmf_unicast);
#endif /* BCM_WFD */

void dhd_handle_blog_connect_event(struct dhd_pub *dhdp, wl_event_msg_t *event);
void dhd_handle_blog_disconnect_event(struct dhd_pub *dhdp, wl_event_msg_t *event);
extern void dhd_if_inc_rxpkt_cnt(dhd_pub_t *dhdp, int ifidx, uint32 pktlen);

/* Defining the feature flag DHD_BLOG_FLUSH_DEV to handle SVN dependency */
#define DHD_BLOG_FLUSH_DEV

#if defined(DHD_BLOG_FLUSH_DEV)
extern int dhd_blog_flush_dev(struct dhd_pub *dhdp, uint ifindex);
#endif /* DHD_BLOG_FLUSH_DEV */

extern int
dhd_blog_flush_flowring(struct dhd_pub *dhdp, uint16 flowid);

#endif /* BCM_BLOG */

#ifndef BCM_COUNTER_EXTSTATS
void net_dev_clear_stats64(struct net_device *dev_p);
struct rtnl_link_stats64 *
net_dev_collect_stats64(struct net_device *dev_p, struct rtnl_link_stats64 *stats);
#endif

void dhd_if_inc_txpkt_mcnt(dhd_pub_t *dhdp, int ifidx, int pktlen);
#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
void dhd_mcast_fill_blog(void *pktbuf, uint16_t staidx, bool wmf_enabled);
#endif

#endif /* __DHD_BLOG_H__ */
