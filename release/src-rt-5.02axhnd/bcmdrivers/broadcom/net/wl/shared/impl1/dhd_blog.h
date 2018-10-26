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
