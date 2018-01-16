/*
 * DHD Bus Module Interfce for Wireless Forwarding Driver (WFD)
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
 * $Id: dhd_wfd.c 634379 2016-04-27 20:21:50Z $
 */

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#if defined(BCM_NBUFF)
#include <linux/nbuff.h>
#endif
#include <bcmutils.h>
#include <dngl_stats.h>
#include <dhd_dbg.h>
#include <dhd.h>
#include <dhd_linux.h>
#include <dhd_flowring.h>
#include <dhd_bus.h>
#include <dhd_wfd.h>
#include <dhd_wmf_linux.h>

#if defined(BCM_BLOG)
#include <linux/blog.h>
#include <dhd_blog.h>
#endif

int
dhd_handle_wfd_blog(dhd_pub_t *dhdp, struct net_device *net, int ifidx,
	void *pktbuf, int b_wmf_unicast)
{
	uint prio = 0, flowid = 0;
	int ret = 0;
	struct sk_buff *skb = NULL;
	bool b_skb_fc_unhandled = FALSE;

#if defined(BCM_NBUFF)
	if (IS_FKBUFF_PTR(pktbuf))
		flowid = PKTFLOWID(pktbuf);
	else
		skb = PNBUFF_2_SKBUFF(pktbuf);
#endif /* BCM_NBUFF */

	prio = PKTPRIO(pktbuf);

	b_skb_fc_unhandled = skb && !DHD_PKT_GET_SKB_FLOW_HANDLED(skb);

	/* when wlan_mark is not marked, do WFD and blog for skb
	 * or for FKB, do it when it is wmf unicast from flowcache.
	 */
	if (b_skb_fc_unhandled || b_wmf_unicast) {
		ret = dhd_flowid_update(dhdp, ifidx, dhdp->flow_prio_map[prio], pktbuf);
		if (ret != BCME_OK) {
			PKTFREE(dhdp->osh, pktbuf, FALSE);
			return ret;
		}
		flowid = PKTFLOWID(pktbuf); /* get flowid after updated */
		/* save as the format of prio + flowid */
		PKTSETPRIO(pktbuf, prio);
	}

#if defined(BCM_BLOG)
	if (!b_wmf_unicast && b_skb_fc_unhandled) {
		struct blog_t *blog_p = skb->blog_p;
		if (blog_p != NULL)
		{
#if defined(BCM_DHD_RUNNER)
			flow_ring_node_t *flow_ring_node;
			flow_ring_node = &(((flow_ring_node_t *)(dhdp->flow_ring_table))[flowid]);

			if (DHD_FLOWRING_RNR_OFFL(flow_ring_node))
			{
				blog_p->rnr.is_tx_hw_acc_en = 1;
				blog_p->rnr.is_wfd = 0;
				blog_p->rnr.flowring_idx = PKTFLOWID(pktbuf);
				blog_p->rnr.ssid = ifidx;
				blog_p->rnr.priority = prio;
				blog_p->rnr.radio_idx = dhdp->unit;
			}
			else
#endif /* BCM_DHD_RUNNER */
			{
				blog_p->wfd.dhd_ucast.is_tx_hw_acc_en = 1;
				blog_p->wfd.dhd_ucast.is_wfd = 1;
				blog_p->wfd.dhd_ucast.is_chain = 0;
				blog_p->wfd.dhd_ucast.wfd_idx = dhdp->wfd_idx;
				blog_p->wfd.dhd_ucast.flowring_idx = flowid;
				blog_p->wfd.dhd_ucast.priority = prio;
				blog_p->wfd.dhd_ucast.ssid = ifidx;
			}

			DHD_PERIM_UNLOCK(dhdp);
			blog_emit(pktbuf, dhd_idx2net(dhdp, ifidx), TYPE_ETH, 0, BLOG_WLANPHY);

#if defined(BCM_DHD_RUNNER)
			/* when RUNNER accelerate flow, stats will not be availabe until
			 * put_stats is called blog has to fill vir_dev in order to get
			 * put_stats called, so call this blog_link() to fill vir_dev
			 */
			blog_lock();
			blog_link(IF_DEVICE, blog_p, (void*)net, DIR_TX, skb->len);
			blog_unlock();
#endif /* BCM_DHD_RUNNER */
			DHD_PERIM_LOCK(dhdp);
		}
	}

	if (skb && DHD_PKT_GET_SKB_FLOW_HANDLED(skb))
		DHD_PKT_CLR_SKB_FLOW_HANDLED(skb);
#endif /* BCM_BLOG */

	return ret;
}


static int BCMFASTPATH
dhd_wfd_forward(unsigned int pkt_cnt, void **pkts, unsigned long wl_radio_idx, unsigned long dummy)
{
	int cnt;
	int ifidx;
	uint16 flowid;
	dhd_pub_t *dhdp;
	FkBuff_t *fkb_p;
	pNBuff_t pNBuf;
	int ret;
	flow_ring_node_t *flow_ring_node;

	DHD_PERIM_LOCK_ALL(wl_radio_idx % FWDER_MAX_UNIT);
	dhdp = g_dhd_info[wl_radio_idx];

	for (cnt = 0; cnt < pkt_cnt; cnt++) { /* Process the array of packets */

		fkb_p = (FkBuff_t *)pkts[cnt];
		pNBuf = FKBUFF_2_PNBUFF((FkBuff_t *)fkb_p);
		ifidx = fkb_p->wl.ucast.dhd.ssid;

#if defined(DSLCPE) && defined(BCM_NBUFF)
		if (PKTATTACHTAG(dhdp->osh, pNBuf))  {
			PKTFREE(dhdp->osh, fkb_p, FALSE);
			dhdp->tx_dropped++;
			dhdp->tx_packets_dropped_wfd++;
			dhd_if_inc_txpkt_drop_cnt(dhdp, ifidx);
			continue;
		}
#endif /* DSLCPE && BCM_NBUFF */

		flowid = fkb_p->wl.ucast.dhd.flowring_idx;

		/* tag this packet as coming from wfd */
		DHD_PKT_SET_WFD_BUF(pNBuf);

		/* Save the flowid and the dataoff in the skb's pkttag */
		DHD_PKT_SET_FLOWID(pNBuf, flowid);

		flow_ring_node = DHD_FLOW_RING(dhdp, flowid);
		if ((flow_ring_node->status != FLOW_RING_STATUS_PENDING) &&
		    (flow_ring_node->status != FLOW_RING_STATUS_OPEN)) {
			DHD_INFO(("%s: on flowid %d when flow ring status is %d\r\n",
				__FUNCTION__,flowid, flow_ring_node->status));
			ret = BCME_NOTREADY;
		} else
		ret = dhd_bus_txqueue_enqueue(dhdp->bus, pNBuf, flowid);
		if (!ret) {
			dhdp->tx_packets++;
			dhdp->tx_packets_wfd++;
#ifndef BCM_DHD_RUNNER
			dhd_if_inc_txpkt_cnt(dhdp, ifidx, pNBuf);
#endif
		} else {
			PKTFREE(dhdp->osh, pNBuf, FALSE);
			dhdp->tx_dropped++;
			dhdp->tx_packets_dropped_wfd++;
			dhd_if_inc_txpkt_drop_cnt(dhdp, ifidx);
		}

	} /* for cnt */

	/* Flush all pending tx queued packets in bus(s) managed on this CPU core */
	dhd_wfd_invoke_func(wl_radio_idx, dhd_bus_txqueue_flush);

	DHD_PERIM_UNLOCK_ALL(wl_radio_idx % FWDER_MAX_UNIT);

	return 0;
}


static void BCMFASTPATH
dhd_send_all(unsigned int dummy)
{
	/* dummy function for now */
}

static void BCMFASTPATH
dhd_wfd_mcasthandler(uint32_t wl_radio_idx, unsigned long fkb, unsigned long dev)
{
	pNBuff_t *pNBuf = FKBUFF_2_PNBUFF((FkBuff_t *)fkb);
	struct net_device *dev_p = (struct net_device *)dev;
	int ret = 0,ifidx;
	dhd_pub_t *dhdp;
#ifdef DHD_WMF
	dhd_wmf_t *wmf;
#endif

	DHD_PERIM_LOCK_ALL(wl_radio_idx % FWDER_MAX_UNIT);
	dhdp = g_dhd_info[wl_radio_idx];
	/*  we can assum ifidx will be right since it comes from fastpath. */
	ifidx = dhd_dev_get_ifidx(dev_p);
#ifdef DHD_WMF
	wmf = dhd_wmf_conf(dhdp, ifidx);
#endif
#if defined(DSLCPE) && defined(BCM_NBUFF)
	if (PKTATTACHTAG(dhdp->osh, (FkBuff_t *)fkb)) {
		DHD_ERROR(("%s : pcie is still in suspend state!!\n", __FUNCTION__));
		goto free_drop;
	}
#endif /* DSLCPE && BCM_NBUFF */

	DHD_PKT_SET_WFD_BUF(pNBuf);

	/* if pkt's priority is not set, retrive it from TOS/DSCP and set it,
	 * if it is still 0, then set it to VI by default, in runner offloading N+M case,
	 * N will use pre-defined priority and for M the packet should be marked by
	 * correct dscp matching to the pre-defined priority.
	 */

#if defined(DSLCPE) && defined(BCM_DHD_RUNNER)
	/*  when DHD_RUNNER offloading is enabled, the N and M station priority has to be the same
	 *  otherwize, there is a chance N station will recevied duplicated pkt.
	 */
	if (g_multicast_priority > 0) {
		PKTSETPRIO(pNBuf, g_multicast_priority);
	} else
#endif /* DSLCPE && BCM_DHD_RUNNER */
	if (!PKTPRIO(pNBuf)) {
		pktsetprio(pNBuf, FALSE);
		if (!PKTPRIO(pNBuf))
			PKTSETPRIO(pNBuf, PRIO_8021D_VI);
	}
#ifdef DHD_WMF
	if (wmf->wmf_enable) {
		/* set  WAN multicast indication before sending to EMF module */
		DHD_PKT_SET_WAN_MCAST(pNBuf);
		DHD_INFO(("%s: pakcet:%p from WMF MCAST and is WANMCAST:%d\n",
			__FUNCTION__, pNBuf, DHD_PKT_GET_WAN_MCAST(pNBuf)));
		ret = dhd_wmf_packets_handle(dhdp, pNBuf, NULL, ifidx, 0);
		if (ret == WMF_TAKEN) 
			goto succ_count;
		else if (ret == WMF_DROP) 
			goto free_drop;
	}
#endif /* DHD_WMF */
	ret = dhd_flowid_update(dhdp, ifidx, dhdp->flow_prio_map[(PKTPRIO(pNBuf))], pNBuf);
	if (ret == BCME_OK) {
		ret = dhd_bus_txdata(dhdp->bus, pNBuf, (uint8)ifidx);
		if (ret == BCME_OK)
		{
			dhdp->tx_multicast++;
#ifdef DSLCPE
			dhd_if_inc_txpkt_mcnt(dhdp, ifidx, pNBuf);
#endif
			goto succ_count;
		} else {
			/* for mutlicast, maxrate is about 1Mbps,drop is normal, so no
			 * need to print error*/
			DHD_INFO(("%s: txdata error, ret:%d\n", __FUNCTION__, ret));
		} 
	} else {
		DHD_ERROR(("%s: flowid_update error.\n", __FUNCTION__));
	}
free_drop:
	PKTFREE(dhdp->osh, pNBuf, FALSE);
	dhd_if_inc_txpkt_drop_cnt(dhdp, ifidx);
	dhdp->tx_dropped++;
	dhdp->tx_packets_dropped_wfd_mcast++;
	goto unlock;
succ_count:
	dhdp->tx_packets_wfd_mcast++;
unlock:
	DHD_PERIM_UNLOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));
	return;
}


int dhd_wfd_bind(struct net_device *net, unsigned int unit)
{
	int wfd_idx = -1;

	wfd_idx = wfd_bind(net, WFD_WL_FWD_HOOKTYPE_FKB, true, (HOOK4PARM)dhd_wfd_forward,
		(HOOK32)dhd_send_all, (HOOK3PARM)dhd_wfd_mcasthandler, unit);
	if (wfd_idx < 0)
		DHD_ERROR(("%s: Error in binding WFD.\n", __FUNCTION__));

	return wfd_idx;
}

void dhd_wfd_unbind(int wfd_idx)
{
	wfd_unbind(wfd_idx, WFD_WL_FWD_HOOKTYPE_FKB);
}

int dhd_wfd_registerdevice(int wfd_idx, struct net_device *dev)
{
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	ret = wfd_registerdevice(wfd_idx, ifidx, dev);
	if (ret != 0) {
		DHD_ERROR(("%s failed wfd_idx %d, ifidx %d\n",
			__FUNCTION__, wfd_idx, ifidx));
	}
	return ret;
}

int dhd_wfd_unregisterdevice(int wfd_idx, struct net_device *dev)
{
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	ret = wfd_unregisterdevice(wfd_idx, ifidx);
	if (ret != 0) {
		DHD_ERROR(("%s failed wfd_idx %d ifidx %d\n",
			__FUNCTION__, wfd_idx, ifidx));
}

return ret;
}

/* Add wfd dump output to a buffer */
void
dhd_wfd_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf)
{
	bcm_bprintf(strbuf, "\ntx_packets_wfd  %lu tx_packets_dropped_wfd %lu\n",
		dhdp->tx_packets_wfd, dhdp->tx_packets_dropped_wfd);
	bcm_bprintf(strbuf, "tx_packets_wfd_mcast  %lu tx_packets_dropped_wfd_mcast %lu\n",
		dhdp->tx_packets_wfd_mcast, dhdp->tx_packets_dropped_wfd_mcast);

	return;
}
