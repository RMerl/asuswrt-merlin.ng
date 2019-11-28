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

#include <dhd_flowring.h>

#if defined(PKTC_TBL)
#include <wl_pktc.h>
#endif

INLINE BCMFASTPATH int
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
#if defined(DSLCPE) || defined(PKTC_TBL)
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

#if defined(DSLCPE) || defined(PKTC_TBL)
	dhdp->rx_linux_cnt++;
#endif
	return !PKT_DONE;
}

INLINE BCMFASTPATH int
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
				blog_p->rnr.llcsnap_flag = DHDHDR_SUPPORT(dhdp) ? 1 : 0;
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
				blog_p->wfd.dhd_ucast.wfd_prio = blog_p->iq_prio;
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

#if defined(PKTC_TBL)
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

/*
 * Call back function to filter flowring id for metadata based blog_notify 
 *
 * Input:
 *  metadata_p: meta data pointer given to the blog_notify(FLUSH) function
 *  blog_p:     pointer to flow control (blog) block
 *
 * Return
 *  non-zero: flowring index in the flush data matches the blog data
 *  zero:     on mismatch
 */
static int
dhd_blog_flush_flowring_match(void *metadata_p, const Blog_t *const blog_p)
{
	if (!metadata_p  || !blog_p) {
		/* invalid pointers, can not match */
		DHD_ERROR(("%s Invalid pointers metadata_p [0x%p] or blog_p [0x%p]\r\n",
			__FUNCTION__, metadata_p, blog_p));
		return 0;
	}

	return ((*((uint16*)metadata_p)) == blog_p->rnr.flowring_idx);
}

/*
 * flush flow cache based on flowid, called when
 *  - Flow ring create fails
 *  - Flow ring delete success
 *  - Driver unload
 *
 * Input:
 *  dhdp:    pointr to dhd public control block
 *  flowid:  id of flowring whose flow need to be flushed
 *           This should be the same one used for blog_emit()
 *
 * Return
 *  zero:     on successful flush
 *  non-zero: on failure
 */
int
dhd_blog_flush_flowring(struct dhd_pub *dhdp, uint16 flowid)
{
	struct net_device *dev;
	BlogFlushParams_t params = {};
	flow_ring_node_t *flow_ring_node = DHD_FLOW_RING(dhdp, flowid);


	dev = dhd_idx2net(dhdp, flow_ring_node->flow_info.ifindex);
	if (dev == NULL) {
		DHD_ERROR(("%s: dev not found! event if=%d\n",
			__FUNCTION__, flow_ring_node->flow_info.ifindex));
		return BCME_ERROR;
	}

	/* Need to unlock perim lock before calling system function */
	DHD_PERIM_UNLOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));

	/* Fill parameters for device + metadata based flush mechanism */
	params.flush_dev = 1;
	params.flush_meta = 1;
	params.devid = dev->ifindex;
	params.metadata_p = (void*)&flowid;
	params.func_p = dhd_blog_flush_flowring_match;

	/* Notify blog to flush based on device + metadata */
	blog_lock();
	blog_notify(FLUSH, dev, (unsigned long)&params, 0);
	blog_unlock();

	/* Lock back */
	DHD_PERIM_LOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));

	return BCME_OK;
}


void
dhd_handle_blog_disconnect_event(struct dhd_pub *dhdp, wl_event_msg_t *event)
{
	struct net_device *dev;
	BlogFlushParams_t params = {};

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

#if defined(PKTC_TBL)
	/* destroy pktc entry */
	dhd_pktc_del((unsigned long)(event->addr.octet));
#endif

	bcm_mcast_wlan_client_disconnect_notifier(dev, event->addr.octet);

	/* also destroy the fcache flow */
	params.flush_dstmac = 1;
	params.flush_srcmac = 1;
	memcpy(&params.mac[0], &event->addr.octet[0], sizeof(event->addr.octet));
	blog_lock();
	blog_notify(FLUSH, dev, (unsigned long)&params, 0);
	blog_unlock();

	DHD_PERIM_LOCK_ALL((dhdp->fwder_unit % FWDER_MAX_UNIT));

	return;
}
#ifndef BCM_COUNTER_EXTSTATS

/* two functions to support 64bit counter */
struct rtnl_link_stats64 *net_dev_collect_stats64(struct net_device *dev_p,struct rtnl_link_stats64 *stats)
{
	BlogStats_t bStats;
	BlogStats_t *bStats_p;
	struct rtnl_link_stats64 *dStats_p;
	struct rtnl_link_stats64 *cStats_p;

	if (dev_p == NULL || dev_p->get_stats_pointer == NULL) {
		memset(stats,0,sizeof(struct rtnl_link_stats64));
		return NULL;
	}

	dStats_p = (struct rtnl_link_stats64 *)dev_p->get_stats_pointer(dev_p, 'd');
	cStats_p = (struct rtnl_link_stats64 *)dev_p->get_stats_pointer(dev_p, 'c');
	bStats_p = (BlogStats_t *)dev_p->get_stats_pointer(dev_p, 'b');

	if (dStats_p && cStats_p && bStats_p) {
		memset(&bStats, 0, sizeof(BlogStats_t));

		blog_get_dev_running_stats_wlan(dev_p, &bStats);
		memcpy(cStats_p, dStats_p, sizeof(struct rtnl_link_stats64));

#if defined(CONFIG_BCM_KF_EXTSTATS)
		/* Handle packet count statistics */
		cStats_p->rx_packets += (bStats.rx_packets + bStats_p->rx_packets);
		cStats_p->tx_packets += (bStats.tx_packets + bStats_p->tx_packets);
		cStats_p->multicast  += (bStats.multicast  + bStats_p->multicast);
		cStats_p->tx_multicast_packets += (bStats.tx_multicast_packets +
				bStats_p->tx_multicast_packets);
		/* NOTE: There are no broadcast packets in BlogStats_t since the
		 * 		 * flowcache doesn't accelerate broadcast.  Thus, they aren't
		 * 		 		 * added here */

		/* set byte counts to 0 if the bstat packet counts are non 0 and
		 * 		 * the octet counts are 0 */
		/* Handle RX byte counts */
		if (((bStats.rx_bytes + bStats_p->rx_bytes) == 0) &&
				((bStats.rx_packets + bStats_p->rx_packets) > 0))
			cStats_p->rx_bytes = 0;
		else
			cStats_p->rx_bytes += bStats.rx_bytes + bStats_p->rx_bytes;

		/* Handle TX byte counts */
		if (((bStats.tx_bytes + bStats_p->tx_bytes) == 0) &&
				((bStats.tx_packets + bStats_p->tx_packets) > 0))
			cStats_p->tx_bytes = 0;
		else
			cStats_p->tx_bytes += bStats.tx_bytes + bStats_p->tx_bytes;

		/* Handle RX multicast byte counts */
		if (((bStats.rx_multicast_bytes + bStats_p->rx_multicast_bytes) == 0) &&
				((bStats.multicast + bStats_p->multicast) > 0))
			cStats_p->rx_multicast_bytes = 0;
		else
			cStats_p->rx_multicast_bytes += bStats.rx_multicast_bytes +
				bStats_p->rx_multicast_bytes;

		/* Handle TX multicast byte counts */
		if (((bStats.tx_multicast_bytes + bStats_p->tx_multicast_bytes) == 0) &&
				((bStats.tx_multicast_packets + bStats_p->tx_multicast_packets) > 0))
			cStats_p->tx_multicast_bytes = 0;
		else
			cStats_p->tx_multicast_bytes += bStats.tx_multicast_bytes +
				bStats_p->tx_multicast_bytes;
#else
		cStats_p->rx_packets += bStats.rx_packets + bStats_p->rx_packets;
		cStats_p->tx_packets += bStats.tx_packets + bStats_p->tx_packets;

		/* set byte counts to 0 if the bstat packet counts are non 0 and the
		 * 		 * octet counts are 0 */
		if (((bStats.rx_bytes + bStats_p->rx_bytes) == 0) &&
				((bStats.rx_packets + bStats_p->rx_packets) > 0))
			cStats_p->rx_bytes = 0;
		else
			cStats_p->rx_bytes += bStats.rx_bytes + bStats_p->rx_bytes;

		if (((bStats.tx_bytes + bStats_p->tx_bytes) == 0) &&
				((bStats.tx_packets + bStats_p->tx_packets) > 0))
			cStats_p->tx_bytes = 0;
		else
			cStats_p->tx_bytes += bStats.tx_bytes + bStats_p->tx_bytes;

		cStats_p->multicast += bStats.multicast + bStats_p->multicast;
#endif

		memcpy(stats,cStats_p,sizeof(struct rtnl_link_stats64));
		return cStats_p;

	} else {
		printk("!!!!The device should have three stats, bcd, "
				"refer br_netdevice.c\r\n");
		return NULL;
	}
}

void net_dev_clear_stats64(struct net_device *dev_p)
{
	BlogStats_t *bStats_p;
	struct rtnl_link_stats64 *dStats_p;
	struct rtnl_link_stats64 *cStats_p;

	if (dev_p == NULL)
		return;

	dStats_p = (struct rtnl_link_stats64 *)dev_p->get_stats_pointer(dev_p, 'd');
	cStats_p = (struct rtnl_link_stats64 *)dev_p->get_stats_pointer(dev_p, 'c');
	bStats_p = (BlogStats_t *)dev_p->get_stats_pointer(dev_p, 'b');

	if (dStats_p && cStats_p && bStats_p) {

        blog_clr_dev_stats(dev_p);

		memset(bStats_p, 0, sizeof(BlogStats_t));
		memset(dStats_p, 0, sizeof(struct rtnl_link_stats64));
		memset(cStats_p, 0, sizeof(struct rtnl_link_stats64));
	}
}

#endif

#endif /* BCM_BLOG */
