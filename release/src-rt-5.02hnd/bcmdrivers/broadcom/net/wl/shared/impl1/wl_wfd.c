/*
 * Linux-specific portion of
 * Broadcom 802.11 Networking Device Driver
 * - WFD: wifi forwarding driver
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
 * $Id: wl_wfd.c $
 */

#if defined(BCM_WFD)

#include <linux/nbuff.h>
#include <wlioctl.h>
#include <wl_dbg.h>

#include <wlc_cfg.h>
#include <wlc_channel.h>

#include <wlc_pub.h>
#include <wl_linux.h>
#include <wl_pktc.h>
#include <wlc_bsscfg.h>

#include <wl_wfd.h>


#if (defined(DSLCPE_WL_IQ) || defined(DSLCPE_TX_PRIO))
#include <wlc_iq.h>
#else
#define DSLCPE_IQ_PRIO(a)	(0)
#endif /* DSLCPE_WL_IQ */


#ifdef DSLCPE_CACHE_SMARTFLUSH
extern struct wlc_bsscfg *wl_bsscfg_find(wl_if_t *wlif);
#endif

#ifdef PKTC_TBL
extern void wl_txchainhandler(struct sk_buff *skb, unsigned long pktc_tbl_ptr,
	uint32 chainIdx, int wl_radio_idx);
extern void wl_txchainhandler_complete(int wl_radio_idx);
#endif
#ifdef DSLCPE_TX_PRIO
extern bool wl_pkt_drop_on_wmark(void *wl_ptr, struct sk_buff *skb, bool is_pktc);
#endif

void wl_wfd_txhandler(unsigned int rx_pktcnt, void **rx_pkts, int wl_radio_idx, uint32_t dummy)
{
	uint16_t chainIdx, prevChainIdx = PKTC_INVALID_CHAIN_IDX;
	unsigned long pktc_tbl_ptr = 0;
	struct sk_buff *skb_p;
	int loopcnt = 0;

	for (loopcnt = 0; loopcnt < rx_pktcnt; loopcnt++) {
		skb_p = (struct sk_buff *)rx_pkts[loopcnt];

		chainIdx = skb_p->wl.ucast.nic.wl_chainidx;

		if (chainIdx != prevChainIdx) {
			pktc_tbl_ptr = wl_pktc_req(PKTC_TBL_GET_BY_IDX, chainIdx, 0, 0);
			if (pktc_tbl_ptr == 0) {
				printk("Invalid ChainIdx %d received\n", chainIdx);
				prevChainIdx = PKTC_INVALID_CHAIN_IDX;
				nbuff_free(skb_p);
				continue;
			}
			prevChainIdx = chainIdx;
		}
		wl_txchainhandler(skb_p, pktc_tbl_ptr, chainIdx, wl_radio_idx);
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
	wl =  (wl_info_t*)(wlif->wl);

#if 1 /* checking part, may to remove later */
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
	prio = PKTPRIO(skb)&0x7;
	lvl = DSLCPE_IQ_PRIO(skb->mark);
	cur_prio = prio*PKT_PRIO_LVL+lvl;

	if ((wl_txq_thresh > 0) && (wl->txq_cnt[cur_prio] >= wl_txq_thresh)) {
		PKTFRMNATIVE(wl->osh, skb);
		PKTCFREE(wl->osh, skb, TRUE);
		TXQ_UNLOCK(wl);
		return;
	}

#ifdef DSLCPE_PREALLOC_SKB
	if (wl_pkt_drop_on_wmark(wl, skb, FALSE)) {
#else
	if (0) {
#endif
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
			for (cnt = 0; cnt < cur_prio; cnt ++) {
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
		wake_up_interruptible(&wl->kthread_wqh);
#else
		err = (int32)(schedule_work(&wl->txq_task.work) == 0);
#endif

		if (!err) {
			atomic_inc(&wl->callbacks);
			wl->txq_dispatched = TRUE;
		} else
			WL_ERROR(("wl%d: wl_start/schedule_work failed\n", wl->pub->unit));
	}

	TXQ_UNLOCK(wl);

	return;
}

int wl_wfd_bind(struct net_device *net, unsigned int unit)
{
	int wfd_idx = -1;

	wfd_idx = wfd_bind(net, WFD_WL_FWD_HOOKTYPE_SKB, true, (HOOK4PARM)wl_wfd_txhandler,
		0, (HOOK3PARM)wl_wfd_mcasthandler, unit);

	if (wfd_idx < 0) {
		WL_ERROR(("%s: wl%d: Error in binding WFD.\n", __FUNCTION__, unit));
	} else {
		WL_ERROR(("%s: wl%d: bound WFD, wfd_idx=%d\n", __FUNCTION__, unit, wfd_idx));

#ifdef PKTC_TBL
		if (wl_pktc_req(PKTC_TBL_UPDATE_WFD_IDX_BY_DEV, (unsigned long)net,
			(unsigned long)wfd_idx, 0) != 0) {
			WL_ERROR(("%s: wl%d: Unable to update WFD handle entry ",
				 __FUNCTION__, unit));
			WL_ERROR(("for dev 0x%p wfd_idx %d\n", net, wfd_idx));
			wfd_idx = -1;
		}
#endif
	}

	return wfd_idx;
}

void wl_wfd_unbind(int wfd_idx)
{
	wfd_unbind(wfd_idx, WFD_WL_FWD_HOOKTYPE_SKB);
	WL_ERROR(("%s: Unbound WFD, wfd_idx=%d\n", __FUNCTION__, wfd_idx));
}

int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev)
{
	int ret = 0;
	int wfd_if_idx;

	wfd_if_idx = netdev_path_get_hw_port(dev);
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
	wfd_if_idx -= wfd_idx * WL_NUM_OF_SSID_PER_UNIT;
#endif

	ret = wfd_registerdevice(wfd_idx, wfd_if_idx, dev);
	return ret;
}

int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev)
{
	int ret = 0;
	int wfd_if_idx;

	wfd_if_idx = netdev_path_get_hw_port(dev);
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
	wfd_if_idx -= wfd_idx * WL_NUM_OF_SSID_PER_UNIT;
#endif

	ret = wfd_unregisterdevice(wfd_idx, wfd_if_idx);
	if (ret != 0)
		WL_ERROR(("wl_wfd_unregisterdevice failed wfd_idx %d\n", wfd_idx));

	return ret;
}

#endif /* BCM_WFD */
