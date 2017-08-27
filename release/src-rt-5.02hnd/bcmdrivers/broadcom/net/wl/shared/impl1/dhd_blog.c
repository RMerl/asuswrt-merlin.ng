/*
 * DHD interfaces to blog system
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
 * $Id: dhd_blog.c 634379 2016-04-27 20:21:50Z $
 */

#if defined(BCM_BLOG)

#include <osl.h>

#include <linux/nbuff.h>
#include <linux/blog.h>

#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_dbg.h>

#include <dhd_blog.h>
#include <bcm_mcast.h>

#ifdef BCM_WFD
#include <dhd_flowring.h>
#endif

#if defined(DSLCPE) && defined(PKTC_TBL)
#include <wl_pktc.h>
#endif

int
dhd_handle_blog_sinit(struct dhd_pub *dhdp, int ifidx, struct sk_buff *skb)
{
	BlogAction_t blog_ret;
	unsigned int pktlen = PKTLEN(dhdp->osh, skb);

	DHD_PERIM_UNLOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));
	blog_ret = blog_sinit(skb, skb->dev, TYPE_ETH, 0, BLOG_WLANPHY);
	
#if defined(BCM_DHD_RUNNER)
	if (PKT_BLOG == blog_ret) {
		blog_lock();
		blog_link(IF_DEVICE, blog_ptr(skb), (void*)skb->dev, DIR_RX, skb->len);
		blog_unlock();
	}
#endif /* BCM_DHD_RUNNER */

	DHD_PERIM_LOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));
	if (PKT_DONE == blog_ret) {
		/* Doesnot need go to IP stack */
#ifdef DSLCPE
		dhdp->rx_fcache_cnt++;
#endif

		dhdp->dstats.rx_bytes += pktlen;
		dhdp->rx_packets ++;
		dhd_if_inc_rxpkt_cnt(dhdp, ifidx, pktlen);

		return PKT_DONE;
	}

#if defined(BCM_DHD_RUNNER)
	/* When DHD_RUNNER is enabled; All RX is done through Runner
	 * Sometimes for testing purpose, we could direct all RX through DHD using BCM_DHD_RUNNER
	 */
	if ((PKT_BLOG == blog_ret) && (skb->blog_p) && DHD_RNR_OFFL_RXCMPL(dhdp)) {
		skb->blog_p->rnr.is_rx_hw_acc_en = 1;
	}
#endif /* BCM_DHD_RUNNER */

#ifdef DSLCPE
	dhdp->rx_linux_cnt++;
#endif
	return !PKT_DONE;
}

int
dhd_handle_blog_emit(dhd_pub_t *dhdp, struct net_device *net, int ifidx,
		void *pktbuf, int b_wmf_unicast)
{
	struct sk_buff *skb = NULL;
	struct blog_t *blog_p = NULL;
	uint prio = 0, flowid = 0;

	if (IS_SKBUFF_PTR(pktbuf)) {
		skb = PNBUFF_2_SKBUFF(pktbuf);
		blog_p = skb->blog_p;
		prio = PKTPRIO(pktbuf);
		flowid = PKTFLOWID(pktbuf);
		/* save as the format of prio + flowid */
		PKTSETPRIO(pktbuf, prio);

		if (!b_wmf_unicast && (blog_p != NULL))
		{
#if defined(BCM_DHD_RUNNER)
			flow_ring_node_t *flow_ring_node;
			flow_ring_node = &(((flow_ring_node_t *)(dhdp->flow_ring_table))[flowid]);

			if (DHD_FLOWRING_RNR_OFFL(flow_ring_node))
			{
				blog_p->rnr.is_tx_hw_acc_en = 1;
				blog_p->rnr.is_wfd = 0;
				blog_p->rnr.flowring_idx = flowid;
				blog_p->rnr.ssid = ifidx;
				blog_p->rnr.priority = prio;
				blog_p->rnr.radio_idx = dhdp->unit;
			}
			else
#endif /* BCM_DHD_RUNNER */
#if defined(BCM_WFD)
			{
				blog_p->wfd.dhd_ucast.is_tx_hw_acc_en = 1;
				blog_p->wfd.dhd_ucast.is_wfd = 1;
				blog_p->wfd.dhd_ucast.is_chain = 0;
				blog_p->wfd.dhd_ucast.wfd_idx = dhdp->wfd_idx;
				blog_p->wfd.dhd_ucast.flowring_idx = flowid;
				blog_p->wfd.dhd_ucast.priority = prio;
				blog_p->wfd.dhd_ucast.ssid = ifidx;
			}
#else /* !BCM_WFD */
			{
				blog_p->wfd.dhd_ucast.is_tx_hw_acc_en = 0;
			}
#endif
			DHD_PERIM_UNLOCK(dhdp);
			blog_emit(pktbuf, dhd_idx2net(dhdp, ifidx), TYPE_ETH, 0, BLOG_WLANPHY);
			DHD_PERIM_LOCK(dhdp);
		}
	}

	return 0;
}

int fdb_check_expired_dhd(unsigned char *addr)
{
	DHD_INFO(("%s: check addr [%02x:%02x:%02x:%02x:%02x:%02x]\n", __FUNCTION__,
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]));

#if defined(DSLCPE) && defined(PKTC_TBL)
{
	wl_pktc_tbl_t *pt;
	
	pt = (wl_pktc_tbl_t *)dhd_pktc_req(PKTC_TBL_GET_BY_DA, (unsigned long)addr, 0, 0);
	if (pt && pt->hits) {
		pt->hits = 0;
		return 0; /* packet is going through, not expired */
	}
}
#endif

#if defined(BCM_WFD)
{
	int i;
	dhd_pub_t *dhdp;
	uint16 flowid;
	flow_ring_node_t *flow_ring_node;
	struct ether_addr ea;
	char eabuf[ETHER_ADDR_STR_LEN];

	bcopy(addr, ea.octet, ETHER_ADDR_LEN);

	for (i = 0; i < FWDER_MAX_RADIO; i++) {
		if (g_dhd_info[i] == NULL)
			continue;
		dhdp = g_dhd_info[i];

		for (flowid = 0; flowid < dhdp->num_flow_rings; flowid++) {
			flow_ring_node = &(((flow_ring_node_t *)(dhdp->flow_ring_table))[flowid]);
			/* only check the DA flow itself, other ea should be intact! */
			if (flow_ring_node &&
				(!eacmp(flow_ring_node->flow_info.da, addr))) {
				if (flow_ring_node->active) {
					DHD_INFO(("%s: flowid %d da=%s is still active !\n",
						__FUNCTION__, flowid, bcm_ether_ntoa(&ea, eabuf)));
					return 0; /* the flow is still active */
				} else {
					DHD_INFO(("%s: flowid %d da=%s is expired !\n",
						__FUNCTION__, flowid, bcm_ether_ntoa(&ea, eabuf)));
					return 1; /* the flow is expired */
				}
			}
		}
	}
	DHD_INFO(("%s: flow da=%s not found in the flowrings !\n",
		__FUNCTION__, bcm_ether_ntoa(&ea, eabuf)));
}
#endif /* BCM_WFD */
	return 1; /* return as expired */
}


void
dhd_handle_blog_disconnect_event(struct dhd_pub *dhdp, wl_event_msg_t *event)
{
	struct net_device *dev;

	dev = dhd_idx2net(dhdp, dhd_ifname2idx(dhdp->info, event->ifname));
	if (dev == NULL) {
		DHD_ERROR(("%s: dev not found! event ifname=%s\n",
			__FUNCTION__, event->ifname));
		return;
	}

	/* before calling system function to make action, first release prim lock,
	 * as tx direction will require this lock which on that cpu, the lock in the
	 * action handler may be holded
	 */
	DHD_PERIM_UNLOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));

	bcm_mcast_wlan_client_disconnect_notifier(dev, event->addr.octet);

	/* also destroy all fcache flows */
	blog_lock();
	blog_notify(DESTROY_NETDEVICE, dev, 0, 0);
	blog_unlock();

	DHD_PERIM_LOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));

	return;
}

#endif /* BCM_BLOG */
