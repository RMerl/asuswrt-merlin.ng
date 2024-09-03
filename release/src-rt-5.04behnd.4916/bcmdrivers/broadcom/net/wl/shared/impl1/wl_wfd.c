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

#if defined(BCM_WFD)

#if !defined(BCM_PKTFWD)
#include <linux/nbuff.h>
#include <wlioctl.h>
#include <wl_dbg.h>
#include <wlc_cfg.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wl_linux.h>
#include <wl_pktc.h>
#include <wl_wfd.h>
#include <wlc_bsscfg.h>
#if (defined(DSLCPE_WL_IQ) || defined(DSLCPE_TX_PRIO))
#include <wlc_iq.h>
#else
#define DSLCPE_IQ_PRIO(a)	(0)
#endif /* DSLCPE_WL_IQ */

#ifdef DSLCPE_CACHE_SMARTFLUSH
extern struct wlc_bsscfg *wl_bsscfg_find(wl_if_t * wlif);
#endif

#ifdef PKTC_TBL
extern void wl_txchainhandler(struct sk_buff *skb, uint32 chainIdx, int wl_radio_idx);
extern void wl_txchainhandler_complete(int wl_radio_idx);
#endif
#ifdef DSLCPE_TX_PRIO
extern bool wl_pkt_drop_on_wmark(osl_t * osh, bool is_pktc);
#endif

void wl_wfd_txhandler(unsigned int rx_pktcnt, void **rx_pkts, int wl_radio_idx, uint32_t dummy)
{
	uint16_t chainIdx, prevChainIdx = PKTC_INVALID_CHAIN_IDX;
	unsigned long pktc_tbl_ptr = 0;
	struct sk_buff *skb_p;
	int loopcnt = 0;

	for (loopcnt = 0; loopcnt < rx_pktcnt; loopcnt++) {
		skb_p = (struct sk_buff *)rx_pkts[loopcnt];

		chainIdx = skb_p->wl.ucast.nic.wl_chainidx & PKTC_CHAIN_IDX_MASK;

		if (chainIdx != prevChainIdx) {
			spin_lock_bh(&pktctbl_lock);
			pktc_tbl_ptr = wl_pktc_req(PKTC_TBL_GET_BY_IDX, chainIdx, 0, 0);
			if (pktc_tbl_ptr == NULL ||
					((wl_pktc_tbl_t *) pktc_tbl_ptr)->wl_handle == 0UL) {
				spin_unlock_bh(&pktctbl_lock);
				//printk("Invalid ChainIdx %d received\n", chainIdx);
				prevChainIdx = PKTC_INVALID_CHAIN_IDX;
				nbuff_free(skb_p);
				continue;
			}
			spin_unlock_bh(&pktctbl_lock);
			prevChainIdx = chainIdx;
		}
		wl_txchainhandler(skb_p, chainIdx, wl_radio_idx);
	}

	wl_txchainhandler_complete(wl_radio_idx);
}

void wl_wfd_mcasthandler(uint32_t wl_radio_idx, unsigned long skb_p, unsigned long dev_p)
{
	struct sk_buff *skb = (struct sk_buff *)skb_p;
	struct net_device *dev = (struct net_device *)dev_p;

	wl_if_t *wlif;
	wl_info_t *wl;

#ifdef DSLCPE_TX_PRIO
	int prio = 0, lvl = 0, cur_prio = 0;
	uint8 fifo;
	bool need_insert = TRUE;
#endif

	if (!dev)
		return;

	wlif = WL_DEV_IF(dev);
	wl = (wl_info_t *) (wlif->wl);

#if 1	/* checking part, may to remove later */
	if (PKTCLINK(skb)) {
		printk("pkt chain link is not NULL,need to correct \r\n");
		PKTSETCLINK(skb, NULL);
	}
#endif
	/* skb device is not set, setting first */
	skb->dev = dev;
	skb->prev = NULL;

#ifdef DSLCPE_CACHE_SMARTFLUSH
	{
		wlc_bsscfg_t *cfg;
		cfg = wl_bsscfg_find(wlif);

		/* to avoid MIC failure in TKIP */
		if ((dsl_tx_pkt_flush_len == 0) || (cfg->wsec & TKIP_ENABLED))
			PKTSETDIRTYP(wl->osh, skb, NULL);
		if (PKTDIRTYPISVALID(wl->osh, skb)) {
			uint8_t *dirty_p = PKTGETDIRTYP(wl->osh, skb);
			uint8_t *deepest = PKTDATA(NULL, skb) + dsl_tx_pkt_flush_len;
			if (dirty_p > deepest)
				deepest = dirty_p;
			if (deepest > skb_tail_pointer(skb))
				deepest = skb_tail_pointer(skb);
			PKTSETDIRTYP(wl->osh, skb, deepest);
		}
	}
#endif /* DSLCPE_CACHE_SMARTFLUSH */

	/* Fix the priority if WME is enabled */
	if (WME_ENAB(wl->pub) && (PKTPRIO(skb) == 0))
		pktsetprio(skb, FALSE);

#ifdef DSLCPE_PREALLOC_SKB
	PKT_PREALLOCINC(wl->osh, skb, 1);
#endif

	/* Lock the queue as tasklet could be running at this time */
	TXQ_LOCK(wl);

#ifdef DSLCPE_TX_PRIO

	/* find skb priority */
	prio = PKTPRIO(skb) & 0x7;
	lvl = DSLCPE_IQ_PRIO(skb->mark);
	cur_prio = prio * PKT_PRIO_LVL + lvl;

	if ((wl_txq_thresh > 0) && (wl->txq_cnt[cur_prio] >= wl_txq_thresh)) {
		PKTFRMNATIVE(wl->osh, skb);
		PKTCFREE(wl->osh, skb, TRUE);
		TXQ_UNLOCK(wl);
		return;
	}
#ifdef DSLCPE_PREALLOC_SKB
	if (wl_pkt_drop_on_wmark(wl->osh, FALSE))
#else
	if (0)
#endif
	{
		/* low prio AC_BK (1), discard */
		if (likely((prio == 1) && (lvl == 0))) {
			PKTFREE(wl->osh, skb, TRUE);
			need_insert = FALSE;
		} else {
			/* find low prio to discard */
			struct sk_buff *skb_drop = NULL;
			int cnt;
			need_insert = FALSE;

			/* need to find lowest to drop, fifo 0 keeps the lowest priority */
			for (cnt = 0; cnt < cur_prio; cnt++) {
				if (wl->txq_head[cnt] != NULL) {
					skb_drop = wl->txq_head[cnt];
					wl->txq_head[cnt] = skb_drop->prev;
					skb_drop->prev = NULL;
					if (wl->txq_head[cnt] == NULL)
						wl->txq_tail[cnt] = NULL;
					wl->txq_cnt[cnt]--;
					/* drop this pkt */
					PKTFREE(wl->osh, skb_drop, TRUE);
					need_insert = TRUE;
					break;
				}
			}
			/* No low prio pkt: Drop this pkt */
			if (!need_insert) {
				PKTFREE(wl->osh, skb, TRUE);
			}
		}
	}

	if (likely(need_insert)) {
		/* insert to proper priority queue based on current priority,
		 * so low txq holds low priority pkt, say prio 2 to fifo 0.
		 */
		fifo = priolvl2fifo[cur_prio];
		if (wl->txq_head[fifo] == NULL)
			wl->txq_head[fifo] = skb;
		else {
			wl->txq_tail[fifo]->prev = skb;
		}
		wl->txq_tail[fifo] = skb;
		wl->txq_cnt[fifo]++;
	} else {
		WLCNTINCR(wl->pub->_cnt->txnobuf);
	}
#else
	if ((wl_txq_thresh > 0) && (wl->txq_cnt >= wl_txq_thresh)) {
		PKTFRMNATIVE(wl->osh, skb);
		PKTCFREE(wl->osh, skb, TRUE);
		TXQ_UNLOCK(wl);
		return;
	}

	if (wl->txq_head == NULL)
		wl->txq_head = skb;
	else
		wl->txq_tail->prev = skb;
	wl->txq_tail = skb;
	wl->txq_cnt++;

#endif /* DSLCPE_TX_PRIO */

	if (!wl->txq_dispatched) {
		int32 err = 0;

		atomic_inc(&wl->callbacks);
		wl->txq_dispatched = TRUE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
		wake_up_interruptible(&wl->kthread_wqh);
#else
		err = (int32) (schedule_work(&wl->txq_task.work) == 0);
#endif
		if (err) {
			atomic_dec(&wl->callbacks);
			wl->txq_dispatched = FALSE;
			WL_ERROR(("wl%d: wl_start/schedule_work failed\n", wl->pub->unit));
		}
	}

	TXQ_UNLOCK(wl);

	return;
}

int wl_wfd_bind(struct net_device *net, unsigned int unit)
{
	int wfd_idx = -1;

	wfd_idx = wfd_bind(net, NULL, WFD_WL_FWD_HOOKTYPE_SKB, true,
	    (HOOK4PARM) wl_wfd_txhandler,
	    0, (HOOK3PARM) wl_wfd_mcasthandler, (HOOK3PARM) NULL, unit);

	if (wfd_idx == WFD_NOT_SUPPORTED) {
		/* Radio is not supported by WFD, lets disable WFD */
		wfd_idx = WLAN_WFD_DISABLE_IDX;
		WL_ERROR(("%s: wl%d WFD not supported, disabled\n", __FUNCTION__, unit));
	} else if (wfd_idx < 0) {
		wfd_idx = WLAN_WFD_INVALID_IDX;
		WL_ERROR(("%s: wl%d: Error in binding WFD.\n", __FUNCTION__, unit));
	} else {
		WL_ERROR(("%s: wl%d: bound WFD, wfd_idx=%d\n", __FUNCTION__, unit, wfd_idx));

#ifdef PKTC_TBL
		if (wl_pktc_req(PKTC_TBL_UPDATE_WFD_IDX_BY_DEV, (unsigned long)net,
			(unsigned long)wfd_idx, 0) != 0) {
			WL_ERROR(("%s: wl%d: Unable to update WFD handle entry ",
				__FUNCTION__, unit));
			WL_ERROR(("for dev 0x%px wfd_idx %d\n", net, wfd_idx));
			wfd_idx = -1;
		}
#endif
	}

	return wfd_idx;
}

void wl_wfd_unbind(int wfd_idx)
{
	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return;
	}
	wfd_unbind(wfd_idx, WFD_WL_FWD_HOOKTYPE_SKB);
	WL_ERROR(("%s: Unbound WFD, wfd_idx=%d\n", __FUNCTION__, wfd_idx));
	// wl_pktfwd_wfd_del(wl);
}

int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev)
{
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return 0;
	}

	ret = wfd_registerdevice(wfd_idx, ifidx, dev);
	if (ret != 0)
		WL_ERROR(("%s failed wfd_idx %d\n", __FUNCTION__, wfd_idx));
	return ret;
}

int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev)
{
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return 0;
	}

	ret = wfd_unregisterdevice(wfd_idx, ifidx);
	if (ret != 0)
		WL_ERROR(("%s failed wfd_idx %d\n", __FUNCTION__, wfd_idx));

	return ret;
}

#else /* BCM_PKTFWD */

#include <wl_dbg.h>
#include <wl_linux.h>
#include <wl_pktc.h>	/* wl_pktfwd.h */
#include <wl_wfd.h>

#if defined(BCM_AWL)
#include <bcm_archer.h>
#include <wl_awl.h>
#endif /* BCM_AWL */

#if defined(BCM_WLAN_FPI)
#include <fpi_wlan.h>
#endif /* BCM_WLAN_FPI */

#if !defined(BCM_PKTLIST)
#error "BCM_PKTFWD: BCM_PKTLIST is not defined"
#endif

#ifdef MC_RX_LOOPBACK_ENABLED
/* NIC->RUNNER->WFD->BACKHERE into this function */
/* intruct caller to continue when return 0 */
int wl_wfd_rx_mcasthandler(uint32_t wl_radio_idx, unsigned long skb_p, unsigned long dev_p)
{
	wl_if_t *wlif;
	int result;

	if (!dev_p)
		return 0;
	wlif = WL_DEV_IF((struct net_device *)dev_p);
	/* if runner can help with identify mc with info->is_ucast in wfd, no need to check here */
	if (is_multicast_ether_addr(PKTDATA(NULL, skb_p))) {

		WL_LOCK(wlif->wl);
		result = wl_pktfwd_rx_mcast_handler(wlif, (void *)skb_p);
		WL_UNLOCK(wlif->wl);

		return (result == BCME_OK) ? -1 : 0;
	} else {
		return 0;
	}
}
#endif /* MC_RX_LOOPBACK_ENABLED */

#if defined(BCM_WLAN_FPI)
/*
 * Get WiFi (NIC) metadata for flow provisioning interface driver
 *
 * Input:
 *  net:  net device,
 *  sa:   source mac address (not used),
 *  da:   destination mac address
 *  prio: priority (0-7)
 *  data: metadata pointer to fill
 *
 * Return:
 *   0:    Success, 'data' will be updated with valid metadata
 *  <0:    Failure, 'data' will be 0
 */
int wl_wfd_get_fpi_metadata(struct net_device *net, u8 *sa, u8 *da, u8 prio,
	u32 *data)
{
	fpi_wl_metadata_t metadata;
	int ifidx;
	wl_if_t *wlif = WL_DEV_IF(net);
	wl_info_t *wl;
	u32 chainIdx = PKTC_INVALID_CHAIN_IDX;


	metadata.wl = 0;
	*data = metadata.wl;

	/*
	 * Input parameter validation
	 */
	/* Check if priority parameter is valid */
	if (prio >= PKTFWD_PRIO_MAX) {
		WL_ERROR(("%s: Invalid prio (%d)\n", __FUNCTION__, prio));
		return BCME_ERROR;
	}

	/* Check if wl public context  is valid */
	wl = (wlif) ? wlif->wl : NULL;
	if (!wl || !wlif) {
		WL_ERROR(("%s: net device (0x%px) wl/wlif NULL\n", __FUNCTION__, net));
		return BCME_ERROR;
	}
	ifidx = wlif->subunit;

	/* Check if interface index is in range */
	if (ifidx >= WL_MAX_IFS) {
		WL_ERROR(("%s: Invalid net device (0x%px) invalid ifidx (%d)\n",
			__FUNCTION__, net, ifidx));
		return BCME_ERROR;
	}

	WL_INFORM(("wl[%d.%d]: %s(0x%px, "MACDBG", "MACDBG" , %d)\n", wl->unit,
		ifidx, __FUNCTION__, net, MAC2STRDBG(sa), MAC2STRDBG(da), prio));

	/* Broadcast/multicast DA is not supported */
	if (ETHER_ISMULTI(da)) {
		WL_ERROR(("%s: ["MACDBG"] is not unicast address\n",
			__FUNCTION__, MAC2STRDBG(da)));
		return BCME_UNSUPPORTED;
	}

	if ((WLAN_WFD_ENABLED(wl->wfd_idx)) && (wl_pktc_req_hook)) {
		/* Handle WAN as well as OVS case (including EAP case) */
		chainIdx = wl_pktc_req_hook(PKTC_TBL_UPDATE, (unsigned long)da,
			(unsigned long)net, 0);

		if (chainIdx != PKTC_INVALID_CHAIN_IDX) {
			uint8_t prio4bit = 0;

			metadata.wfd.nic_ucast.is_tx_hw_acc_en = 1;
			metadata.wfd.nic_ucast.is_wfd = 1;
			/*
			 * Convert 3bit prio to 4bit priority for nic blog
			 * ignore iq_prio for now as skb is not available
			 */
			prio4bit = SET_WLAN_PRIORITY(prio4bit, prio);
			metadata.wfd.nic_ucast.priority = prio4bit;
			metadata.wfd.nic_ucast.is_chain = 1;
			metadata.wfd.nic_ucast.wfd_idx =
				((chainIdx & PKTC_WFD_IDX_BITMASK) >> PKTC_WFD_IDX_BITPOS);
			metadata.wfd.nic_ucast.chain_idx = chainIdx;
		} else {
			WL_ERROR(("wl[%d.%d]: %s Unable to get da context\n",
					wl->unit, ifidx, __FUNCTION__));
			/* non-WFD/PKTFWD software acceleration is not supported */
			return BCME_ERROR;
		}
	} else {
		WL_ERROR(("wl[%d.%d]: %s nonWFD/PKTFWD mode not supported\n",
				wl->unit, ifidx, __FUNCTION__));
		/* non-WFD/PKTFWD software acceleration is not supported */
		return BCME_UNSUPPORTED;
	}

	*data = metadata.wl;

	WL_ERROR(("wl[%d.%d]: %s(0x%px, "MACDBG" , "MACDBG" , %d) = 0x%x\n",
		wl->unit, ifidx, __FUNCTION__, net, MAC2STRDBG(sa), MAC2STRDBG(da),
		prio, *data));

	return BCME_OK;
}
#endif /* BCM_WLAN_FPI */

int wl_wfd_bind(wl_info_t * wl)
{
	int unit = wl->unit;
	int wfd_idx = -1;
	pktlist_context_t *wl_pktlist_context;

	PKTFWD_TRACE("wl%d", unit);

	if (unit >= WL_PKTFWD_RADIOS) {
		WL_ERROR(("%s: dhd<%d> WFD not supported, disabled\n", __FUNCTION__, unit));
		wfd_idx = WLAN_WFD_DISABLE_IDX;
		goto wl_wfd_bind_failure;
	}

	wl_pktlist_context = (pktlist_context_t *)
	    wl_pktfwd_request(wl_pktfwd_req_pktlist_e, unit, 0, 0);

	if (wl_pktlist_context == PKTLIST_CONTEXT_NULL) {
		WL_ERROR(("%s: wl%d wl_pktlist_context NULL\n", __FUNCTION__, unit));
		goto wl_wfd_bind_failure;
	}
#if defined(BCM_AWL)
	/* On Archer based platforms, Archer driver directly provides WFD services */
	wfd_idx = archer_wlan_bind(wl->dev, wl_pktlist_context,
	    ARCHER_WLAN_RADIO_MODE_SKB, (HOOK32) wl_pktfwd_xfer_callback, unit);
#else /* !BCM_AWL */
	wfd_idx = wfd_bind(wl->dev, wl_pktlist_context,
	    WFD_WL_FWD_HOOKTYPE_SKB, false, (HOOK4PARM) NULL, (HOOK32) wl_pktfwd_xfer_callback,
#ifdef MC_RX_LOOPBACK_ENABLED
	    (HOOK3PARM) NULL, (HOOK3PARM) wl_wfd_rx_mcasthandler, unit);
#else
	    (HOOK3PARM) NULL, (HOOK3PARM) NULL, unit);
#endif

#endif /* !BCM_AWL */

#if defined(BCM_WLHDR) && defined(BCM_PKTFWD_LLC_SNAP)
	if (wl_pktlist_context->add_llcsnap_header) {
		wl->wlhdr_support = true;
	}
#endif /* BCM_WLHDR && BCM_PKTFWD_LLC_SNAP */

	if (wfd_idx == WFD_NOT_SUPPORTED) {
		/* Radio is not supported by WFD, lets disable WFD */
		wfd_idx = WLAN_WFD_DISABLE_IDX;
		PKTFWD_ERROR("%s: wl%d WFD not supported, disabled\n", __FUNCTION__, unit);
		goto wl_wfd_bind_failure;
	} else if (wfd_idx < 0) {
		wfd_idx = WLAN_WFD_INVALID_IDX;
		PKTFWD_ERROR("wl%d wfd_idx %d failure", unit, wfd_idx);
		goto wl_wfd_bind_failure;
	} else
		PKTFWD_PRINT("wl%d wfd_idx %d success", unit, wfd_idx);

	if (wfd_idx != unit) {
		PKTFWD_ERROR("wl%d wfd_idx %d mismatch", unit, wfd_idx);
		wfd_idx = WLAN_WFD_INVALID_IDX;
	} else {
		wfd_set_rx_wait_queue(wfd_idx, &wl->kthread_wqh);
		wl_pktfwd_wfd_ins(wl, wfd_idx);
#if defined(BCM_WLAN_FPI)
		fpi_register_wl_get_metadata(wl_wfd_get_fpi_metadata);
#endif /* BCM_WLAN_FPI */
	}

wl_wfd_bind_failure:
	PKTFWD_ASSERT(wfd_idx != WLAN_WFD_INVALID_IDX);

	return wfd_idx;
}

void wl_wfd_unbind(wl_info_t * wl)
{
	int wfd_idx = wl->wfd_idx;

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return;
	}

#if defined(BCM_WLAN_FPI)
	fpi_register_wl_get_metadata(NULL);
#endif /* BCM_WLAN_FPI */

#if defined(BCM_AWL)
	archer_wlan_unbind(wl->unit);
#else /* !BCM_AWL */
	wfd_unbind(wfd_idx, WFD_WL_FWD_HOOKTYPE_SKB);
#endif /* !BCM_AWL */
	WL_ERROR(("%s: Unbound WFD, wfd_idx=%d\n", __FUNCTION__, wfd_idx));

	wl_pktfwd_wfd_del(wl);
}

int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev)
{
#if !defined(BCM_AWL)
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return 0;
	}

	ret = wfd_registerdevice(wfd_idx, ifidx, dev);
	if (ret != 0)
		WL_ERROR(("%s failed wfd_idx %d\n", __FUNCTION__, wfd_idx));
	return ret;
#else /* BCM_AWL */
	return wl_awl_register_dev(dev);
#endif /* BCM_AWL */
}

int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev)
{
#if !defined(BCM_AWL)
	int ret = 0;
	int ifidx = WLAN_NETDEVPATH_SSID(netdev_path_get_hw_port(dev));

	if (!WLAN_WFD_ENABLED(wfd_idx)) {
		/* Nothing to do */
		return 0;
	}

	ret = wfd_unregisterdevice(wfd_idx, ifidx);
	if (ret != 0)
		WL_ERROR(("%s failed wfd_idx %d\n", __FUNCTION__, wfd_idx));

	return ret;
#else /* BCM_AWL */
	return wl_awl_unregister_dev(dev);
#endif /* BCM_AWL */
}

#endif /* BCM_PKTFWD */

#endif /* BCM_WFD */
