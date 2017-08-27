
/*
 * Linux-specific portion of
 * Broadcom 802.11 Networking Device Driver
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
 * $Id: wl_pktc.c $
 */

#if defined(PKTC_TBL)

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>

#include <wlc_cfg.h>
#include <wlioctl.h>
#include <wlc_key.h>

#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>

#if (defined(DSLCPE_WL_IQ) || defined(DSLCPE_TX_PRIO))
#include <wlc_iq.h>
#else
#define DSLCPE_IQ_PRIO(a)	(0)
#endif /* DSLCPE_WL_IQ || DSLCPE_TX_PRIO */

#include <wl_dbg.h>
#include <wl_linux.h>

#include <wl_pktc.h>

extern int wl_start_int(wl_info_t *wl, wl_if_t *wlif, struct sk_buff *skb);
#ifdef DSLCPE_CACHE_SMARTFLUSH
extern struct wlc_bsscfg *wl_bsscfg_find(wl_if_t *wlif);
#endif
extern bool wl_pkt_drop_on_wmark(void *wl_ptr, struct sk_buff *skb, bool is_pktc);

#ifdef DSLCPE
extern void osl_set_wlunit(osl_t *osh, uint8 unit);
#endif

typedef int (*HardStartXmitFuncP)(struct sk_buff *skb, struct net_device *dev);

void (*bcm63xx_wlan_txchainhandler_hook)(struct sk_buff *skb, uint32_t pktc_tbl_ptr,
	uint8 wlTxChainIdx) = 0;
void (*bcm63xx_wlan_txchainhandler_complete_hook)(void) = 0;

int wl_dump_pktc(wl_info_t *wl, struct bcmstrbuf *b);

wl_pktc_tbl_t pktc_tbl[CHAIN_ENTRY_NUM];
pktc_handle_t pktc_wldev[WLAN_DEVICE_MAX];
int pktc_tx_enabled = 1;

const unsigned int bitmap[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
const unsigned int bitpos[] = {1, 2, 4, 8, 16};

inline int get_32bitmap_pos(uint32 v)
{
	int i;

	register unsigned int r = 0; /* result of log2(v) will go here */

	if (!v)
		return -1;

	for (i = 4; i >= 0; i--) { /* unroll for speed... */
		if (v & bitmap[i]) {
			v >>= bitpos[i];
			r |= bitpos[i];
		}
	}
	return r;
}

inline int get_16bitmap_pos(uint16 v)
{
	int i;

	register unsigned int r = 0; /* result of log2(v) will go here */

	if (!v)
		return -1;

	for (i = 3; i >= 0; i--) { /* unroll for speed... */
		if (v & bitmap[i]) {
			v >>= bitpos[i];
			r |= bitpos[i];
		}
	}
	return r;
}

void wl_callbacks_inc(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;

	atomic_inc(&wl->callbacks);
}

void wl_callbacks_dec(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;

	atomic_dec(&wl->callbacks);
}

void wl_set_txq_dispatched(pktc_info_t *pktci, bool flag)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;

	wl->txq_txchain_dispatched = flag;  /* global flag for wl thread */
	wlif->_txq_txchain_dispatched = flag; /* flag per interface */
}

bool wl_get_txq_dispatched(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;

	return wlif->_txq_txchain_dispatched; /* get flag per interface */
}

int wl_schedule_work(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;

	wake_up_interruptible(&wl->kthread_wqh);

	return 1; /* success */
}

void wl_write_to_chain_table(pktc_info_t *pktci, uint16 chainIdx, struct sk_buff *chead,
	struct sk_buff *ctail, uint8 fifo)
{
	c_entry_t *pPktc;

	wl_txchain_lock(pktci);
	pPktc = &(pktci->pktc_table[0]);
	PKTCENQCHAINTAIL(pPktc[fifo].chain[chainIdx].chead, pPktc[fifo].chain[chainIdx].ctail,
		chead, ctail);
	pPktc[fifo].chainidx_bitmap |= (1 << chainIdx);
	pktci->prio_bitmap |= (1 << fifo);
	wl_txchain_unlock(pktci);

	return;
}

void BCMFASTPATH
wl_start_txchain_txqwork(struct wl_task *task)
{
	pktc_info_t *pktci = (pktc_info_t *)task->context;
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;
	struct sk_buff *chead_tmp, *skb;
	int chainIdx, cnt, fifo;
	uint32 tmpChainIdxBitmap = 0;
	c_entry_t *pPktc;

	wl_txchain_lock(pktci);
	pPktc = &(pktci->pktc_table[0]);
	while ((fifo = get_16bitmap_pos(pktci->prio_bitmap)) != -1) {
		tmpChainIdxBitmap = pPktc[fifo].chainidx_bitmap;
		while ((chainIdx = get_32bitmap_pos(tmpChainIdxBitmap)) != -1) {
			cnt = 0;
			chead_tmp = skb = pPktc[fifo].chain[chainIdx].chead;
			while (skb != NULL) {
				cnt++;
				if (cnt >= wl->pub->tunables->txsbnd) {
					/* point to the next skb */
					pPktc[fifo].chain[chainIdx].chead = PKTCLINK(skb);
					if (pPktc[fifo].chain[chainIdx].chead == NULL)
						pPktc[fifo].chain[chainIdx].ctail = NULL;
					PKTSETCLINK(skb, NULL); /* break the chain */
					break;
				}
				skb = PKTCLINK(skb);
			}
			if ((cnt >= 0) && (cnt < wl->pub->tunables->txsbnd)) {
				/* less than threshold */
				pPktc[fifo].chain[chainIdx].chead =
				pPktc[fifo].chain[chainIdx].ctail = NULL;
				pPktc[fifo].chainidx_bitmap &= ~(1<<chainIdx);
			}

			wl_txchain_unlock(pktci);

			if (chead_tmp != NULL) {
				WLCNTSET(pktci->stats.txcurrchainsz, cnt);
				if (pktci->stats.txcurrchainsz >
					pktci->stats.txmaxchainsz)
					WLCNTSET(pktci->stats.txmaxchainsz,
						pktci->stats.txcurrchainsz);

				PKTCSETCNT(chead_tmp, cnt);
				wl_start_pktc(pktci, chead_tmp->dev, chead_tmp);
			}

			tmpChainIdxBitmap &= ~(1<<chainIdx);
			wl_txchain_lock(pktci);
		} /* inner while */

		if (!(pPktc[fifo].chainidx_bitmap))
			pktci->prio_bitmap &= ~(1<<fifo);
	} /* outer while */
	wl_set_txq_dispatched(pktci, FALSE);
	wl_txchain_unlock(pktci);

	wl_callbacks_dec(pktci);
	return;
}

/* this is for transmit path */
static uint32 chainidx_bitmap[WLAN_DEVICE_MAX]={0}; /* per phy interface */

inline
void wl_txchainhandler(struct sk_buff *skb, unsigned long pktc_tbl_ptr,
	uint32 chainIdx, int wl_radio_idx)
{
	pktc_info_t *pktci = NULL;
	wl_pktc_tbl_t *pt = (wl_pktc_tbl_t*)pktc_tbl_ptr;
	int prio = 0, lvl = 0, cur_prio = 0;
	uint8 fifo;
	bool need_insert = TRUE;

	/* pktc_tbl_ptr is validated before passing to this function and wl_handle as well */
	pktci = (pktc_info_t *)(pt->wl_handle);

	skb->dev = pt->tx_dev;
	PKTSETCLINK(skb, NULL);
	skb->prev = skb->next = NULL;

#ifdef DSLCPE_CACHE_SMARTFLUSH
{
	wlc_bsscfg_t *cfg;
	cfg = wl_bsscfg_find(pktci->wlif);

	/* to avoid MIC failure in TKIP */
	if ((dsl_tx_pkt_flush_len == 0) || (cfg->wsec & TKIP_ENABLED))
	        PKTSETDIRTYP(pktci->osh, skb, NULL);

	if (PKTDIRTYPISVALID(pktci->osh, skb)) {
		uint8_t *dirty_p = PKTGETDIRTYP(pktci->osh, skb);
		uint8_t *deepest = PKTDATA(NULL, skb) + dsl_tx_pkt_flush_len;
		if (dirty_p > deepest)
			deepest = dirty_p;
		if (deepest > skb_tail_pointer(skb))
			deepest = skb_tail_pointer(skb);
		PKTSETDIRTYP(pktci->osh, skb, deepest);
	}
}
#endif /* DSLCPE_CACHE_SMARTFLUSH */

#ifdef DSLCPE_PREALLOC_SKB
	PKT_PREALLOCINC(pktci->osh, skb, 1);
#endif

	/* find skb priority */
	prio = PKTPRIO(skb)&0x7;
	lvl = DSLCPE_IQ_PRIO(skb->mark);
	cur_prio = prio << 1 | lvl;
#ifdef DSLCPE_TX_PRIO
	fifo = priolvl2fifo[cur_prio];
#else
	fifo = 4; /* AC_BE */
#endif
	pktci->prio = prio;

#ifdef DSLCPE_PREALLOC_SKB
	if (wl_pkt_drop_on_wmark(NULL, skb, TRUE)) {
#else
	if (0) {
#endif
		/* low prio AC_BK (1), discard */
		if (likely((prio == 1) && (lvl == 0))) {
			PKTFREE(pktci->osh, skb, TRUE);
			need_insert = FALSE;
		}
		else
		{
			/* find low prio to discard */
			struct sk_buff *skb_drop = NULL;
			int cnt;
			need_insert = FALSE;

			/* need to find lowest to drop, fifo 0 keeps the lowest priority */
			for (cnt = 0; cnt < cur_prio; cnt ++) {
				if (pt->chain[cnt].chead != NULL) {
					skb_drop = pktc_tbl->chain[cnt].chead;
					pt->chain[cnt].chead = PKTCLINK(skb_drop);
					PKTSETCLINK(skb_drop, NULL);
					if (pt->chain[cnt].chead == NULL) {
						pt->chain[cnt].ctail = NULL;
						/* reset prio bit for this lower queue
						 * since it's empty.
						 */
						pt->prio_bitmap &= ~(1<<cnt);
					}
					/* drop this pkt */
					PKTFREE(pktci->osh, skb_drop, TRUE);
					need_insert = TRUE;
					break;
				}
			}
			/* No low prio pkt: Drop this pkt */
			if (!need_insert)
				PKTFREE(pktci->osh, skb, TRUE);
		}
	}

	if (likely(need_insert)) {
		/* insert to proper priority queue based on current priority,
		 * so low txq holds low priority pkt, say prio 2 to fifo 0.
		 */
		chainidx_bitmap[wl_radio_idx] |= (1<<chainIdx);
		pt->prio_bitmap |= (1 << fifo);

		PKTSETCHAINED(pktci->osh, skb);

		/* chain SKBs based on priority */
		PKTCENQTAIL(pt->chain[fifo].chead, pt->chain[fifo].ctail, skb);

		pktci->stats.total_pkts++;
		pt->hits ++;
	} else {
		WLCNTINCR(pktci->stats.txdrop);
	}
	return;
}

inline
void wl_txchainhandler_complete(int wl_radio_idx)
{
	pktc_info_t *pktci = NULL;
	wl_pktc_tbl_t *pt;
	c_pair_t  *pChain;
	int chainIdx, fifo;

	while ((chainIdx = get_32bitmap_pos(chainidx_bitmap[wl_radio_idx])) != -1) {
		pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_BY_IDX, chainIdx, 0, 0);
		if (pt) {
			pktci = (pktc_info_t *)(pt->wl_handle);
			if (!pktci) {
				printk("Error - pktci is NULL!\n");
				return;
			}
			while ((fifo = get_16bitmap_pos(pt->prio_bitmap)) != -1) {
				pChain = &(pt->chain[fifo]);
				wl_write_to_chain_table(pktci, pt->idx, pChain->chead,
					pChain->ctail, fifo);
				pChain->chead = pChain->ctail = NULL;
				pt->prio_bitmap &= ~(1<<fifo);
			}

			wl_txchain_lock(pktci);
			if (wl_get_txq_dispatched(pktci) == FALSE) {
				wl_set_txq_dispatched(pktci, TRUE);
				if (wl_schedule_work(pktci)) {
					wl_callbacks_inc(pktci);
				}
			}
			wl_txchain_unlock(pktci);
		} else {
			printk("%s: ERR: wl_radio_idx=%d, chainIdx=%d, bitMap=0x%08x\n",
				__FUNCTION__, wl_radio_idx, chainIdx,
				chainidx_bitmap[wl_radio_idx]);
			/* What about the loss of SKBs/BDs : TBD ??? */
		}
		chainidx_bitmap[wl_radio_idx] &= ~(1<<chainIdx);
	}

	return;
}

/* this is for receive path */
int32 wl_rxchainhandler(wl_info_t *wl, struct sk_buff *skb)
{
	wl_pktc_tbl_t *pt;
	unsigned long dev_xmit;

#if defined(PKTC_TBL) && defined(BCM_WFD) && \
	(defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858))
        priv_link_t *priv_link = netdev_priv(skb->dev);
	if ((inject_to_fastpath) && (WL_IFTYPE_WDS != priv_link->wlif->if_type)) {
		/* call to registered fastpath callback */
		send_packet_to_upper_layer(skb);
		return BCME_OK;
	} else
#else
	if (PKTISCHAINED(skb))
#endif
	{
		struct ether_header *eh = (struct ether_header *)PKTDATA(wl->osh, skb);
		pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_BY_DA,
			(unsigned long)(eh->ether_dhost), 0, 0);
		if (pt && pt->tx_dev != NULL) {
			if (pt->tx_dev->netdev_ops == NULL)
				return (BCME_ERROR);

			dev_xmit = (unsigned long)(pt->tx_dev->netdev_ops->ndo_start_xmit);
			if (dev_xmit) {
				pt->hits ++;
				/* call enet xmit directly */
				((HardStartXmitFuncP)dev_xmit)(skb, pt->tx_dev);
				return (BCME_OK);
			}
		}
	}
	return (BCME_ERROR);
}

wl_pktc_tbl_t *wl_pktc_attach(struct wl_info *wl, struct wl_if *wlif)
{
	wl_pktc_tbl_t *pktc_tbl;

	/* init tx work queue for processing chained packets  */
	wl->txq_txchain_dispatched = FALSE;

	wlc_dump_register(wl->pub, "pktc", (dump_fn_t)wl_dump_pktc, (void *)wl);
	fdb_check_expired_wl_hook = wl_check_fdb_expired;
	wl_pktc_req_hook = wl_pktc_req;
	wl_pktc_del_hook = wl_pktc_del;

	pktc_tbl = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_START_ADDRESS, 0, 0, 0);

#ifdef DSLCPE
	osl_set_wlunit(wl->osh, wl->unit);
#endif

#if defined(BCM_BLOG)
	wl->pub->fcache = 1; /* enable fcache by default */
#endif
	return pktc_tbl;
}

void wl_pktc_detach(struct wl_info *wl)
{
	fdb_check_expired_wl_hook = NULL;
}

int wl_pktc_init(wl_if_t *wlif, struct net_device *dev)
{
	struct pktc_info *pktci;
	wl_info_t *wl = wlif->wl;

	/* allocate pktc private info struct */
	pktci = (struct pktc_info *) MALLOC(wl->osh, sizeof(struct pktc_info));
	if (!pktci) {
		WL_ERROR(("wl%d: %s: malloc of pktci failed\n",
			(wl->pub) ? wl->pub->unit:wlif->subunit, __FUNCTION__));
		return (-1);
	}
	bzero(pktci, sizeof(struct pktc_info));

	spin_lock_init(&wlif->txchain_lock);

	wlif->pktci = pktci;
	pktci->osh = wl->osh;
	pktci->wlif = (void *)wlif;

	wl_pktc_req(PKTC_TBL_UPDATE_WLAN_HANDLE, (unsigned long)pktci, (unsigned long)dev, 0);
#ifdef BCM_WFD
	wl_pktc_req(PKTC_TBL_UPDATE_WFD_IDX_BY_DEV, (unsigned long)dev,
		(unsigned long)wl->wfd_idx, 0);
#endif

	wlif->txq_txchain_task.context = pktci;
	wlif->_txq_txchain_dispatched = FALSE;

	return 0;
}

void wl_pktc_free(wl_if_t *wlif)
{
	wl_info_t *wl = wlif->wl;

	if (wlif->pktci) {
		wl_pktc_req(PKTC_TBL_DELETE_WLAN_HANDLE, (unsigned long)wlif->pktci, 0, 0);
		MFREE(wl->osh, wlif->pktci, sizeof(pktc_info_t));
	}
}

int wl_check_fdb_expired(unsigned char *addr)
{
	wl_pktc_tbl_t *pt;

	pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_BY_DA, (unsigned long)addr, 0, 0);
	if (pt && pt->hits) {
		pt->hits = 0;
		return 0; /* packet is going through, not expired */
	}

	return 1; /* expired */
}
void wl_txchain_lock(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	spin_lock_bh(&(wlif->txchain_lock));
}

void wl_txchain_unlock(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	spin_unlock_bh(&(wlif->txchain_lock));
}

void BCMFASTPATH wl_start_pktc(pktc_info_t *pktci, struct net_device *dev,
	struct sk_buff *skb)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;

	wl_start_int(wl, WL_DEV_IF(dev), skb);
}


/* for packet chaining */
unsigned long wl_pktc_req(int req_id, unsigned long param0, unsigned long param1,
	unsigned long param2)
{
	int i;
	wl_pktc_tbl_t *pt;

	switch (req_id) {
	case PKTC_TBL_GET_BY_DA:
		/* param0 is DA */
		return (PKTC_TBL_FN_LOOKUP(pktc_tbl, (uint8_t*)param0));

	case PKTC_TBL_GET_START_ADDRESS:
		return (unsigned long)(&pktc_tbl[0]);

	case PKTC_TBL_GET_BY_IDX:
		/* param0 is pktc chain table index */
		if (param0 >= CHAIN_ENTRY_NUM) {
			printk("chain idx is out of range! (%ld)\n", param0);
			return 0;
		}
		if (!(pktc_tbl[param0].in_use) || !(pktc_tbl[param0].wl_handle)) {
			printk("Error : chain idx %ld is not in use or invalid handle 0x%lx\n",
				param0, pktc_tbl[param0].wl_handle);
			return 0;
		}
		return (unsigned long)(&pktc_tbl[param0]);

	case PKTC_TBL_UPDATE_WLAN_HANDLE:
		for (i = 0; i < WLAN_DEVICE_MAX; i++) {
			if (pktc_wldev[i].handle == 0)
				break; /* found the empty entry */
		}
		if (i == WLAN_DEVICE_MAX) {
			printk("wlan device number is out of range! (%d)\n", i);
			return -1;
		}

		/* param0 is wl info pointer, param1 is dev */
		pktc_wldev[i].handle = param0;
		pktc_wldev[i].dev = param1;
		return 0;

#if defined(BCM_WFD)
	case PKTC_TBL_UPDATE_WFD_IDX_BY_DEV:
		for (i = 0; i < WLAN_DEVICE_MAX; i++) {
			if (pktc_wldev[i].dev == param0) {
				pktc_wldev[i].wfd_idx = param1;
#if 0
				printk("Found dev 0x%x updating wfd_idx %d i %d\n",
					pktc_wldev[i].dev, pktc_wldev[i].wfd_idx, i);
#endif
				return 0; /* found the matching wl dev entry */
			}
		}
		if (i == WLAN_DEVICE_MAX) {
			printk("ERROR, Matching WLAN device entry not found for dev(0x%lx)\n",
				param0);
			return -1;
		}
		break;
#endif /* BCM_WFD */

	case PKTC_TBL_SET_TX_MODE:
		/* param0 is enable: 1 or disable: 0 */
		pktc_tx_enabled = param0;
		break;

	case PKTC_TBL_GET_TX_MODE:
		/* enable: 1 or disable: 0 */
		return pktc_tx_enabled;

	case PKTC_TBL_UPDATE:
		param2 = (unsigned long)NULL;

		/* param1 is tx device */
		for (i = 0; i < WLAN_DEVICE_MAX; i++) {
			if (pktc_wldev[i].dev == param1) {
				param2 = (unsigned long)&pktc_wldev[i];
				break;
			}
		}
		/* param0 is addr, param1 is dev, param2 is wl handle if any */
		pt = (wl_pktc_tbl_t *)PKTC_TBL_FN_UPDATE(pktc_tbl, (uint8_t *)param0,
			(struct net_device *)param1, (pktc_handle_t *)param2);
		if ((pt == NULL) || (pt->idx >= CHAIN_ENTRY_NUM))
			return PKTC_INVALID_CHAIN_IDX;

		/* if wl_handle is NULL and tx dev is 'wl', which means pkt is going to dhd drv,
		 * we should not create the chain entry for it, hence pkt won't be chained and sent
		 * to tx_dev directly but fcache.
		 */
		if ((pt->tx_dev == NULL) || ((param2 == (unsigned long)NULL) &&
			(!strncmp(pt->tx_dev->name, "wl", 2)))) {
			/* remove this chain entry */
			PKTC_TBL_FN_CLEAR(pktc_tbl, (uint8 *)param0);
			return PKTC_INVALID_CHAIN_IDX;
		}

#if defined(BCM_WFD)
{
		uint16 wfd_idx = pt->wfd_idx;
#if 0
		printk("pt->wfd_idx 0x%x pt->idx 0x%x ret 0x%x wfd_idx after rotate 0x%x\n",
			pt->wfd_idx, pt->idx,
			((pt->idx & (~PKTC_WFD_IDX_BITMASK)) | (wfd_idx << PKTC_WFD_IDX_BITPOS)),
			(wfd_idx << PKTC_WFD_IDX_BITPOS));
#endif
		return ((pt->idx & (~PKTC_WFD_IDX_BITMASK)) | (wfd_idx << PKTC_WFD_IDX_BITPOS));
}
#else
		return pt->idx; /* return chain index */
#endif /* BCM_WFD */

	case PKTC_TBL_DELETE:
		PKTC_TBL_FN_CLEAR(pktc_tbl, (uint8 *)param0);
		return 0;

	case PKTC_TBL_DELETE_WLAN_HANDLE:
		/* param0 is handle pointer */
		for (i = 0; i < WLAN_DEVICE_MAX; i++) {
			if (pktc_wldev[i].handle == param0) {
				pktc_wldev[i].dev = 0;
				pktc_wldev[i].handle = 0;
			}
		}
		return 0;

	case PKTC_TBL_FLUSH:
		printk("pktc_tbl flush!\n");
		for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
			if (pktc_tbl[i].in_use)
				memset(&pktc_tbl[i], 0, sizeof(wl_pktc_tbl_t));
		}
		return 0;

	default:
		return 0;
	}
	return 0;
}

void wl_pktc_del(unsigned long addr)
{
	wl_pktc_req(PKTC_TBL_DELETE, addr, 0, 0);
}

void pktc_tbl_clear_fn(wl_pktc_tbl_t *tbl, uint8_t *da)
{
	uint16_t pktc_tbl_hash_idx = 0;
	wl_pktc_tbl_t *pt = NULL;
	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		memset(pt, 0, sizeof(wl_pktc_tbl_t));
		return;
	}
	/* Secondary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		memset(pt, 0, sizeof(wl_pktc_tbl_t));
		return;
	}
	return;
}

unsigned long pktc_tbl_update_fn(wl_pktc_tbl_t *tbl, uint8_t *da, struct net_device *dev,
	pktc_handle_t *handle_p)
{
	uint16_t pktc_tbl_hash_idx = 0, pktc_tbl_hash_idx2 = 0;
	wl_pktc_tbl_t *pt, *pt2;

	if (!dev) { /* device is a mandatory parameter */
		goto invalid_param_out;
	}
	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pktc_tbl_hash_idx2 = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	pt2 = &tbl[pktc_tbl_hash_idx2];
	if (!pt->in_use && !pt2->in_use) {
		/* Both Primary & Secondary hash index not in-use; Grab the first one */
		goto add_primary;
	}
	else if (!pt->in_use) { /* Primary not in-use */
		if (_eacmp(pt2->ea.octet, (da)) == 0) { /* Exist in secondary */
			goto add_secondary_exist;
		} else { /* Secondary is occupied - use unsed Primary */
			goto add_primary;
		}
	}
	else if (!pt2->in_use) { /* Secondary not in-use */
		if (_eacmp(pt->ea.octet, (da)) == 0) { /* Exist in Primary */
			goto add_primary_exist;
		} else { /* Primary is occupied - use unsed Secondary */
			goto add_secondary;
		}
	} else { /* Both are in-use */
		if (_eacmp(pt->ea.octet, (da)) == 0) { /* Exist in Primary */
			goto add_primary_exist;
		}
		else if (_eacmp(pt2->ea.octet, (da)) == 0)  { /* Exist in Secondary */
			goto add_secondary_exist;
		}
	}
invalid_param_out:
	/* Reaching here means - both are occupied with different MAC : Unavailable */
#if 0
	printk("Hash collision : Entry %d occupied [%02x:%02x:%02x:%02x:%02x:%02x]\n",
		pktc_tbl_hash_idx, pt->ea.octet[0], pt->ea.octet[1], pt->ea.octet[2],
		pt->ea.octet[3], pt->ea.octet[4], pt->ea.octet[5]);
#endif
	return 0;

add_primary:
	pt->ea = *(struct _mac_address *)(da);
	pt->idx = pktc_tbl_hash_idx;
	pt->in_use = 1;
add_primary_exist:
	pt->tx_dev = dev;
	if (handle_p) {
		pt->wl_handle = handle_p->handle;
#if defined(BCM_WFD)
		pt->wfd_idx = handle_p->wfd_idx;
#endif
	}
	return (unsigned long)pt;
add_secondary:
	pt2->ea = *(struct _mac_address *)(da);
	pt2->idx = pktc_tbl_hash_idx2;
	pt2->in_use = 1;
add_secondary_exist:
	pt2->tx_dev = dev;
	if (handle_p) {
		pt2->wl_handle = handle_p->handle;
#if defined(BCM_WFD)
		pt2->wfd_idx = handle_p->wfd_idx;
#endif
	}
	return (unsigned long)pt2;
}

unsigned long pktc_tbl_lookup_fn(wl_pktc_tbl_t *tbl, uint8_t *da)
{
	uint16_t pktc_tbl_hash_idx = 0;
	wl_pktc_tbl_t *pt = NULL;
	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		return (unsigned long)pt;
	}
	/* Secondary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0) {
		return (unsigned long)pt;
	}
	return 0;
}

int wl_dump_pktc(wl_info_t *wl, struct bcmstrbuf *b)
{
	int i;
	pktc_info_t *pktci;

	printk("FROM WLAN: \n");
	printk("idx  dest_MAC           dest_dev   hits\n");
	printk("----------------------------------------\n");
	for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
		if ((pktc_tbl[i].in_use) && (pktc_tbl[i].wl_handle == 0)) {
			printk("%02d  %02x:%02x:%02x:%02x:%02x:%02x   %s       %d\n",
				pktc_tbl[i].idx,
				pktc_tbl[i].ea.octet[0],
				pktc_tbl[i].ea.octet[1],
				pktc_tbl[i].ea.octet[2],
				pktc_tbl[i].ea.octet[3],
				pktc_tbl[i].ea.octet[4],
				pktc_tbl[i].ea.octet[5],
				(pktc_tbl[i].tx_dev == NULL) ? "NULL" : pktc_tbl[i].tx_dev->name,
				pktc_tbl[i].hits);
		}
	}

	printk("\nTO WLAN: \n");
#if defined(BCM_WFD)
	printk("idx  prio  dest_MAC         dest_dev   wfd_idx   chainsz (cur/max)   pkts   drops\n");
#else
	printk("idx  prio  dest_MAC         dest_dev   chainsz (cur/max)   pkts   drops\n");
#endif
	printk("------------------------------------------------------------------------------------\n");
	for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
		if ((pktc_tbl[i].in_use) && (pktc_tbl[i].wl_handle)) {
				pktci = (pktc_info_t *)(pktc_tbl[i].wl_handle);
				printk("%02d    %d  %02x:%02x:%02x:%02x:%02x:%02x    ", pktc_tbl[i].idx,
				pktci->prio,
				pktc_tbl[i].ea.octet[0],
				pktc_tbl[i].ea.octet[1],
				pktc_tbl[i].ea.octet[2],
				pktc_tbl[i].ea.octet[3],
				pktc_tbl[i].ea.octet[4],
				pktc_tbl[i].ea.octet[5]);
			printk("%s        ", (pktc_tbl[i].tx_dev == NULL) ?
				"NULL" : pktc_tbl[i].tx_dev->name);
#if defined(BCM_WFD)
			printk("%d          ", pktc_tbl[i].wfd_idx);
#endif
			printk("%d / %d        ",
				pktci->stats.txcurrchainsz, pktci->stats.txmaxchainsz);
			printk("%lu    %d\n", pktci->stats.total_pkts, pktc_tbl[i].pktci->stats.txdrop);
		}
	}

	return 0;
}

#endif /* PKTC_TBL */
