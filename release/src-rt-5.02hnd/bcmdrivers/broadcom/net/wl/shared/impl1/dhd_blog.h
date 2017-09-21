/*
 * DHD blog interface - header file
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_blog.h 634379 2016-04-27 20:21:50Z $
 */

#ifndef __DHD_BLOG_H__
#define __DHD_BLOG_H__

#if defined(BCM_BLOG)

#include <linux/blog.h>
#include <proto/ethernet.h>
#include <proto/bcmevent.h>

#include <dngl_stats.h>
#include <dhd.h>

#define DHD_BLOG_SKIP(skb, reason) blog_skip((skb));

struct dhd_pub;

#if defined(BCM_WFD)
extern struct dhd_pub *g_dhd_info[];
#endif /* BCM_WFD */

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

#endif /* BCM_BLOG */

#endif /* __DHD_BLOG_H__ */
