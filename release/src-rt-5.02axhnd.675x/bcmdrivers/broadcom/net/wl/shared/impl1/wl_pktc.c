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

#if defined(PKTC_TBL)

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>
#include <bcmutils.h>

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
extern bool wl_pkt_drop_on_wmark(osl_t *osh, bool is_pktc);

#ifdef DSLCPE
extern void osl_set_wlunit(osl_t *osh, uint8 unit);
extern uint8 osl_get_wlunit(osl_t *osh);
#endif

typedef int (*HardStartXmitFuncP)(struct sk_buff *skb, struct net_device *dev);
int wl_dump_pktc(wl_info_t *wl, struct bcmstrbuf *b);

/*
 * Global variables
 */
wl_pktc_tbl_t *g_pktc_tbl = NULL;
uint8 g_pktc_tbl_ref_cnt = 0;     /* reference count for g_pktc_tbl */
pktc_handle_t pktc_wldev[WLAN_DEVICE_MAX] = {0};

#if defined(CONFIG_BCM963158) && !defined(BCM_WFD)
int pktc_tx_enabled = 0;
#else
int pktc_tx_enabled = 1;
#endif
spinlock_t pktctbl_lock;

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

/* get the highest bit pos in bitmap */
inline int get_bitmap_pos(pktc_bitmap_t *v)
{
	int i, bit_num = -1;

	for (i = BITMAP_CHUNK_NUM - 1; i >= 0; i--) {
		if ((bit_num = get_32bitmap_pos(v->bit_chunk[i])) >= 0)
			bit_num += (i<<BITMAP_CHUNK_BITSHIFT) ;
		if (bit_num >= 0)
			break;
	}
	return bit_num;
}

inline void set_bitmap_pos(pktc_bitmap_t *v, int pos)
{
	int which_chunk;

#if 0
	if (pos >= BITMAP_BITS_TOTAL) {
		printk("input pos (%d) is exceeding the max value (%d)!\n", pos, BITMAP_BITS_TOTAL);
		return;
	}
#endif

	which_chunk = pos >> BITMAP_CHUNK_BITSHIFT; /* 32bits */
	v->bit_chunk[which_chunk] |= 1 << (pos % BITMAP_CHUNK_BITS);
}

inline void unset_bitmap_pos(pktc_bitmap_t *v, int pos)
{
        int which_chunk;

#if 0
        if (pos >= BITMAP_BITS_TOTAL) {
                printk("input pos (%d) is exceeding the max value (%d)!\n", pos, BITMAP_BITS_TOTAL);
                return;
        }
#endif

        which_chunk = pos >> BITMAP_CHUNK_BITSHIFT; /* 32bits */
        v->bit_chunk[which_chunk] &= ~(1 << (pos % BITMAP_CHUNK_BITS));
}

#ifdef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
uint16 pktc_tbl_hash(uint8 *da)
{
	return (hndcrc16((uint8 *)da, 6, CRC16_INIT_VALUE) % (CHAIN_ENTRY_NUM/2));
}
#else
uint8 pktc_tbl_hash(uint8 *da)
{
	return (hndcrc8((uint8 *)da, 6, CRC8_INIT_VALUE) % (CHAIN_ENTRY_NUM/2));
}
#endif

#ifdef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
uint16 pktc_tbl_hash2(uint8 *da)
{
	return ((hndcrc16((uint8 *)da, 6, CRC16_INIT_VALUE) % (CHAIN_ENTRY_NUM/2)) + (CHAIN_ENTRY_NUM/2));
}
#else
uint8 pktc_tbl_hash2(uint8 *da)
{
	return ((hndcrc8((uint8 *)da, 6, CRC8_INIT_VALUE) % (CHAIN_ENTRY_NUM/2)) + (CHAIN_ENTRY_NUM/2));
}
#endif

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
	wlif->pktci->_txq_txchain_dispatched = flag; /* flag per interface */
}

bool wl_get_txq_dispatched(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;

	return wlif->pktci->_txq_txchain_dispatched; /* get flag per interface */
}

int wl_schedule_work(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;

	wake_up_interruptible(&wl->kthread_wqh);

	return 1; /* success */
}

/* re-assign chainIdx-based chains to priority-based chains */
void wl_write_to_chain_table(pktc_info_t *pktci, struct sk_buff *chead,
	struct sk_buff *ctail, uint8 fifo)
{
	c_pair_t *pPktc;

	wl_txchain_lock(pktci);
	pPktc = &(pktci->pktc_table[0]);
	PKTCENQCHAINTAIL(pPktc[fifo].chead, pPktc[fifo].ctail,
		chead, ctail);
	pktci->prio_bitmap |= (1 << fifo);
	wl_txchain_unlock(pktci);

	return;
}

void BCMFASTPATH
wl_start_txchain_txqwork(pktc_info_t *pktci)
{
	struct sk_buff *chead_tmp, *skb, *prev_skb;
	int cnt, fifo;
	c_pair_t *pPktc;
	uint8 *da = NULL, *prev_da = NULL;
	int mixed_chain = 0; /* is the chain mixed with different DAs */

	wl_txchain_lock(pktci);
	pPktc = &(pktci->pktc_table[0]);
	while ((fifo = get_16bitmap_pos(pktci->prio_bitmap)) != -1) {
		cnt = 0;
		prev_da = NULL;
		mixed_chain = 0;
		chead_tmp = skb = pPktc[fifo].chead;
		while (skb != NULL) {
			da = (uint8 *)PKTDATA(pktci->osh, skb);
			cnt++;
			if ((prev_da != NULL) && (memcmp(prev_da, da, ETHER_ADDR_LEN) != 0))
				mixed_chain = 1;
			if ((cnt >= PKTC_MAX_SIZE) || mixed_chain) {
				if (mixed_chain) {
					/* new chain head is current skb */
					pPktc[fifo].chead = skb;
					PKTSETCLINK(prev_skb, NULL); /* break the chain */
				} else {
					/* new chain head is next skb */
					pPktc[fifo].chead = PKTCLINK(skb);
					PKTSETCLINK(skb, NULL); /* break the chain */
				}
				if (pPktc[fifo].chead == NULL)
					pPktc[fifo].ctail = NULL;
				break;
			}
			prev_skb = skb;
			prev_da = da;
			skb = PKTCLINK(skb);
		}
		if ((cnt >= 0) && (cnt < PKTC_MAX_SIZE) && !mixed_chain) {
			/* less than threshold */
			pPktc[fifo].chead =
			pPktc[fifo].ctail = NULL;
			pktci->prio_bitmap &= ~(1<<fifo);
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

		wl_txchain_lock(pktci);

	}
	wl_set_txq_dispatched(pktci, FALSE);
	wl_txchain_unlock(pktci);

	wl_callbacks_dec(pktci);
	return;
}

/* this is for transmit path */
pktc_bitmap_t g_chainidx_bitmap[WLAN_DEVICE_MAX]={0}; /* per phy interface */

void wl_txchainhandler(struct sk_buff *skb, uint32 chainIdx, int wl_radio_idx)
{
	wl_if_t *wlif;
	wl_info_t *wl;
	pktc_info_t *pktci = NULL;
	wl_pktc_tbl_t *pt;
	int prio = 0, lvl = 0, cur_prio = 0;
	uint8 fifo;
	bool need_insert = TRUE;
	struct sk_buff *skb_free = NULL;

	spin_lock_bh(&pktctbl_lock);
	pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_BY_IDX, chainIdx, 0, 0);
	if (pt == NULL) {
		printk("%s: pktc_tbl_ptr is NULL!\n", __FUNCTION__);
		spin_unlock_bh(&pktctbl_lock);
		PKTFREE(NULL, skb, TRUE);
		return;
	}

	pktci = (pktc_info_t *)(pt->wl_handle);

	if (pktci == NULL) {
		spin_unlock_bh(&pktctbl_lock);
		PKTFREE(NULL, skb, TRUE);
		return;
	}
	skb->dev = pt->tx_dev;
	PKTSETCLINK(skb, NULL);
	skb->prev = skb->next = NULL;
	wlif = (wl_if_t *)pktci->wlif;
	wl = wlif->wl;
	if (!wl->pub->up) {
		spin_unlock_bh(&pktctbl_lock);
		PKTFREE(NULL, skb, TRUE);
		return;
	}

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

#ifdef DSLCPE_PREALLOC_SKB
	if (wl_pkt_drop_on_wmark(pktci->osh, TRUE)) {
#else
	if (0) {
#endif
		/* low prio AC_BK (1), discard */
		if (likely((prio == 1) && (lvl == 0))) {
			skb_free = skb;
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
					skb_drop = pt->chain[cnt].chead;
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
					skb_free = skb_drop;
					need_insert = TRUE;
					break;
				}
			}
			/* No low prio pkt: Drop this pkt */
			if (!need_insert)
				skb_free = skb;
		}
	}

	if (likely(need_insert)) {
		/* insert to proper priority queue based on current priority,
		 * so low txq holds low priority pkt, say prio 2 to fifo 0.
		 */
		set_bitmap_pos(&g_chainidx_bitmap[wl_radio_idx], chainIdx);
		pt->prio_bitmap |= (1 << fifo);

		PKTSETCHAINED(pktci->osh, skb);

		/* chain SKBs based on priority */
		PKTCENQTAIL(pt->chain[fifo].chead, pt->chain[fifo].ctail, skb);

		WLCNTINCR(pktci->stats.total_pkts[prio]);
		WLCNTINCR(pt->hits);
	} else {
		WLCNTINCR(pktci->stats.txdrop[prio]);
	}

	spin_unlock_bh(&pktctbl_lock);
	if (skb_free)
		PKTFREE(pktci->osh, skb_free, TRUE);
	return;
}

void wl_txchainhandler_complete(int wl_radio_idx)
{
	pktc_info_t *pktci = NULL;
	wl_pktc_tbl_t *pt;
	c_pair_t  *pChain;
	int chainIdx, fifo;

	while ((chainIdx = get_bitmap_pos(&g_chainidx_bitmap[wl_radio_idx])) != -1) {
		unset_bitmap_pos(&g_chainidx_bitmap[wl_radio_idx], chainIdx);
		spin_lock_bh(&pktctbl_lock);
		pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_BY_IDX, chainIdx, 0, 0);
		if (pt == NULL || ((pktci = (pktc_info_t *)(pt->wl_handle)) == NULL)) {
			spin_unlock_bh(&pktctbl_lock);
			return;
		}
		while ((fifo = get_16bitmap_pos(pt->prio_bitmap)) != -1) {
			pChain = &(pt->chain[fifo]);
			wl_write_to_chain_table(pktci, pChain->chead, pChain->ctail, fifo);
			pChain->chead = pChain->ctail = NULL;
			pt->prio_bitmap &= ~(1<<fifo);
		}
		spin_unlock_bh(&pktctbl_lock);

		wl_txchain_lock(pktci);
		if (wl_get_txq_dispatched(pktci) == FALSE) {
			wl_set_txq_dispatched(pktci, TRUE);
			if (wl_schedule_work(pktci)) {
				wl_callbacks_inc(pktci);
			}
		}
		wl_txchain_unlock(pktci);
	}

	return;
}

/* this is for receive path */
int32 wl_rxchainhandler(wl_info_t *wl, struct sk_buff *skb)
{
	wl_pktc_tbl_t *pt;
	unsigned long dev_xmit;
	struct net_device *tx_dev=NULL;

#if defined(PKTC_TBL) && defined(BCM_WFD) && defined(CONFIG_BCM_PON)
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
		spin_lock_bh(&pktctbl_lock);
		pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_BY_DA,
			(unsigned long)(eh->ether_dhost), 0, 0);
		if ((pt == NULL) || (pt->tx_dev == NULL) || (pt->tx_dev->netdev_ops == NULL)) {
			spin_unlock_bh(&pktctbl_lock);
			goto exit;
		}
		tx_dev = pt->tx_dev;
		
#if defined(CONFIG_BCM_FC_BASED_WFD)
		if(pt->tx_dev->priv_flags & (IFF_BCM_WLANDEV))
		{ /* not support handle CHAIN XMIT yet .. let fcache handle it*/
			spin_unlock_bh(&pktctbl_lock);
			goto exit;
		}
#endif		

		dev_xmit = (unsigned long)(tx_dev->netdev_ops->ndo_start_xmit);
		if (!dev_xmit) {
			spin_unlock_bh(&pktctbl_lock);
			goto exit;
		}
		
		pt->hits ++;
		spin_unlock_bh(&pktctbl_lock);
		/* call enet xmit directly */
		((HardStartXmitFuncP)dev_xmit)(skb, tx_dev);
		return (BCME_OK);
	}
exit:
	if (wl->pub->pktc_tbl && WLPKTCTBL(wl->pub->pktc_tbl)->g_stats)
		WLCNTINCR(WLPKTCTBL(wl->pub->pktc_tbl)->g_stats->rx_slowpath_skb);
	return (BCME_ERROR);
}

wl_pktc_tbl_t *wl_pktc_attach(struct wl_info *wl, struct wl_if *wlif)
{
	wl_pktc_tbl_t *pt;
	size_t size = sizeof(wl_pktc_tbl_t)*CHAIN_ENTRY_NUM;

	/* Allocate pktc table */
	if (g_pktc_tbl == NULL) {
		g_pktc_tbl = (wl_pktc_tbl_t *) kmalloc(size, GFP_KERNEL);
		if (!g_pktc_tbl) {
			WL_ERROR(("wl%d: %s: malloc of g_pktc_tbl failed\n",
				(wl->pub) ? wl->pub->unit:wlif->subunit, __FUNCTION__));
			return NULL;
		}
		bzero((void*)g_pktc_tbl, size);
	}
	/* Increment pktc table reference count */
	g_pktc_tbl_ref_cnt++;

	/* init tx work queue for processing chained packets  */
	wl->txq_txchain_dispatched = FALSE;

	wlc_dump_register(wl->pub, "pktc", (dump_fn_t)wl_dump_pktc, (void *)wl);
	fdb_check_expired_wl_hook = wl_check_fdb_expired;
	wl_pktc_req_hook = wl_pktc_req_with_lock;
	wl_pktc_del_hook = wl_pktc_del_ex;

	pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_START_ADDRESS, 0, 0, 0);

	if (!pt->g_stats) {
		pt->g_stats = (struct pktc_stats *) kmalloc(sizeof(struct pktc_stats), GFP_KERNEL);
		if (!pt->g_stats) {
			WL_ERROR(("wl%d: %s: malloc of pktc_tbl->g_stats failed\n",
				(wl->pub) ? wl->pub->unit:wlif->subunit, __FUNCTION__));
			return NULL;
		}
		bzero(pt->g_stats, sizeof(struct pktc_stats));
	}
	pt->g_stats->n_references++;

#ifdef DSLCPE
	osl_set_wlunit(wl->osh, wl->unit);
#endif

#if defined(BCM_BLOG)
	wl->pub->fcache = 1; /* enable fcache by default */
#endif

	spin_lock_init(&pktctbl_lock);

	return pt;
}

void wl_pktc_detach(struct wl_info *wl)
{
	wl_pktc_tbl_t *pt = WLPKTCTBL(wl->pub->pktc_tbl);

	if (pt->g_stats != NULL) {
		if (--(pt->g_stats->n_references) == 0) {
			kfree(pt->g_stats);
			pt->g_stats = NULL;
		}
	}

	/* Free pktc table if there are no more interface references to the data*/
	if (g_pktc_tbl) {
		/* Decrement the pktc table reference count */
		g_pktc_tbl_ref_cnt--;

		if (g_pktc_tbl_ref_cnt <= 0) {
			kfree(g_pktc_tbl);
			g_pktc_tbl = NULL;
		}
	}

	fdb_check_expired_wl_hook = NULL;
	wl_pktc_req_hook = NULL;
	wl_pktc_del_hook = NULL;
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

	spin_lock_init(&pktci->txchain_lock);

	pktci->osh = wl->osh;
	pktci->wlif = (void *)wlif;

#ifdef DSLCPE
	osl_set_wlunit(wl->osh, wl->unit);
#endif

	wl_pktc_req(PKTC_TBL_UPDATE_WLAN_HANDLE, (unsigned long)pktci, (unsigned long)dev, 0);
#ifdef BCM_WFD
	wl_pktc_req(PKTC_TBL_UPDATE_WFD_IDX_BY_DEV, (unsigned long)dev,
		(unsigned long)wl->wfd_idx, 0);
#endif

	pktci->_txq_txchain_dispatched = FALSE;
	wlif->pktci = pktci;

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

int wl_check_fdb_expired(unsigned char *addr, struct net_device * net_device)
{
	wl_pktc_tbl_t *pt;

	WL_INFORM(("%s: addr=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]));

	pt = (wl_pktc_tbl_t *)wl_pktc_req(PKTC_TBL_GET_BY_DA, (unsigned long)addr, 0, 0);
	/* PSTA interface has zero hits as DA is the PC mac behind PSTA. 
	   Return not expired to avoid flushing bridge fdb to cause dip. 
	*/
	if (pt && (pt->hits || pt->sta_assoc)) {
		pt->hits = 0;
		return 0; /* packet is going through, not expired */
	}
	wl_pktc_del((unsigned long)addr);

	return 1; /* expired */
}

void wl_txchain_lock(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	spin_lock_bh(&(wlif->pktci->txchain_lock));
}

void wl_txchain_unlock(pktc_info_t *pktci)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	spin_unlock_bh(&(wlif->pktci->txchain_lock));
}

void BCMFASTPATH wl_start_pktc(pktc_info_t *pktci, struct net_device *dev,
	struct sk_buff *skb)
{
	wl_if_t *wlif = (wl_if_t *)pktci->wlif;
	wl_info_t *wl = wlif->wl;

	wl_start_int(wl, WL_DEV_IF(dev), skb);
}

void wl_pktc_clear_entry(wl_pktc_tbl_t *pt)
{
	if (pt != NULL) {
		pt->in_use = 0;
		pt->hits = 0;
		pt->tx_dev = NULL;
		pt->prio_bitmap = 0;
		pt->idx = 0;
		pt->sta_assoc = 0;
		if (pt->pktci != NULL) {
			bzero(&pt->pktci->stats, sizeof(struct pktc_stats));
		}
		bzero(&pt->ea, sizeof(struct _mac_address));
		bzero(&pt->chain[0], sizeof(pt->chain));
	}
}

/* traverse the entire pktc table to get the number of associated stations */
int wl_pktc_get_sta_num(void)
{
	int i, sta_cnt = 0;

	for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
		if ((g_pktc_tbl[i].in_use) && (g_pktc_tbl[i].sta_assoc)) {
			sta_cnt ++;
		}
	}
	return sta_cnt;
}

/* for packet chaining */
unsigned long wl_pktc_req(int req_id, unsigned long param0, unsigned long param1,
	unsigned long param2)
{
	int i;
	wl_pktc_tbl_t *pt;

	switch (req_id) {
	case PKTC_TBL_GET_BY_DA:
		if (g_pktc_tbl == NULL)
			return 0;

		/* param0 is DA */
		return (PKTC_TBL_FN_LOOKUP(g_pktc_tbl, (uint8_t*)param0));

	case PKTC_TBL_GET_START_ADDRESS:
		if (g_pktc_tbl == NULL)
			return 0;

		return (unsigned long)(&g_pktc_tbl[0]);

	case PKTC_TBL_GET_BY_IDX:
		if (g_pktc_tbl == NULL)
			return 0;

		/* param0 is pktc chain table index */
		if (param0 >= CHAIN_ENTRY_NUM) {
			printk("chain idx is out of range! (%ld)\n", param0);
			return 0;
		}
		if (!(g_pktc_tbl[param0].in_use) || !(g_pktc_tbl[param0].wl_handle)) {
#if 0
			printk("Error : chain idx %ld is not in use or invalid handle 0x%lx\n",
				param0, g_pktc_tbl[param0].wl_handle);
#endif
			return 0;
		}
		return (unsigned long)(&g_pktc_tbl[param0]);

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
	{
		bool is_wlan;

		if (!g_pktc_tbl || !g_pktc_tbl[0].g_stats)
			return 0;

		param2 = (unsigned long)NULL;

		/* param1 is tx device */
		for (i = 0; i < WLAN_DEVICE_MAX; i++) {
			if (pktc_wldev[i].dev == param1) {
				param2 = (unsigned long)&pktc_wldev[i];
				break;
			}
		}
		/* param0 is addr, param1 is dev, param2 is wl handle if any */
		pt = (wl_pktc_tbl_t *)PKTC_TBL_FN_UPDATE(g_pktc_tbl, (uint8_t *)param0,
			(struct net_device *)param1, (pktc_handle_t *)param2);

		if ((pt == NULL) || (pt->idx >= CHAIN_ENTRY_NUM))
			return PKTC_INVALID_CHAIN_IDX;

		is_wlan = is_netdev_wlan((struct net_device *)param1);

#if defined(CONFIG_BCM_FC_BASED_WFD)
		/* For CONFIG_BCM_FC_BASED_WFD , TXCHAIN will go through FCACHE, so it's ok for enable CHAIN for wl to wl.
                   If wl_handle is NULL and device is_wlan, which means pkt is going to dhd drv ,create chain entry for RX Chain.
                   But report INVALID_CHAIN_IDX for avoid update blog that send to DHD
		*/
		if ((pt->tx_dev == NULL) || (((pktc_info_t *)pt->wl_handle == NULL || (pktc_handle_t *)param2 == NULL) &&
			(is_wlan || (!strncmp(pt->tx_dev->name, "wds", 3))))) {
			/* remove this chain entry only when tx_dev == NULL */
			if (pt->tx_dev == NULL) 
			      PKTC_TBL_FN_CLEAR(g_pktc_tbl, (uint8 *)param0);
			return PKTC_INVALID_CHAIN_IDX;
		}
#else
		/* if wl_handle is NULL and device is wlan, which means pkt is going to dhd drv,
		 * we should not create the chain entry for it, hence pkt won't be chained and sent
		 * to tx_dev directly but fcache. Same as wds.
		 */
		if ((pt->tx_dev == NULL) || (((pktc_info_t *)pt->wl_handle == NULL || (pktc_handle_t *)param2 == NULL) &&
			(is_wlan || (!strncmp(pt->tx_dev->name, "wds", 3))))) {
			/* remove this chain entry */
			PKTC_TBL_FN_CLEAR(g_pktc_tbl, (uint8 *)param0);
			return PKTC_INVALID_CHAIN_IDX;
		}
#endif

		/* update associated station numbers */
		g_pktc_tbl[0].g_stats->total_stas = wl_pktc_get_sta_num();

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
	}

	case PKTC_TBL_DELETE:
		if (!g_pktc_tbl || !g_pktc_tbl[0].g_stats)
			return 0;

		PKTC_TBL_FN_CLEAR(g_pktc_tbl, (uint8 *)param0);
		/* update associated station numbers */
		g_pktc_tbl[0].g_stats->total_stas = wl_pktc_get_sta_num();
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
		if (g_pktc_tbl == NULL)
			return 0;

		WL_INFORM(("%s: pktc_tbl flush!\n", __FUNCTION__));
		for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
			if (g_pktc_tbl[i].in_use)
				wl_pktc_clear_entry(&g_pktc_tbl[i]);
		}
		return 0;

	case PKTC_TBL_SET_STA_ASSOC:
		if (!g_pktc_tbl || !g_pktc_tbl[0].g_stats)
			return 0;

		/* param0 is STA addr, param1 is assoc status, 0: disassoc, 1: assoc, param2 is event type */
		pt = (wl_pktc_tbl_t *)PKTC_TBL_FN_LOOKUP(g_pktc_tbl, (uint8_t*)param0);
		if (pt != NULL) {
			pt->sta_assoc = param1;
		}
		/* update associated station numbers */
		g_pktc_tbl[0].g_stats->total_stas = wl_pktc_get_sta_num();
		return 0;

	default:
		return 0;
	}
	return 0;
}

void wl_pktc_del(unsigned long addr)
{
	spin_lock_bh(&pktctbl_lock);
	wl_pktc_req(PKTC_TBL_DELETE, addr, 0, 0);
	if (g_pktc_tbl && g_pktc_tbl[0].g_stats && (g_pktc_tbl[0].g_stats->total_stas == 0)) {
		/* if no more wifi station associated, flush entire pktc table */
		wl_pktc_req(PKTC_TBL_FLUSH, 0, 0, 0);
	}
	spin_unlock_bh(&pktctbl_lock);
}

void wl_pktc_del_ex(unsigned long addr, struct net_device * net_device)
{
    BCM_REFERENCE(net_device);
    wl_pktc_del(addr);
}

unsigned long wl_pktc_req_with_lock( int req_id, unsigned long param0, unsigned long param1, unsigned long param2 )
{
	unsigned long retval;
	spin_lock_bh(&pktctbl_lock); 
	retval = wl_pktc_req( req_id, param0, param1, param2 );
	spin_unlock_bh(&pktctbl_lock);
	return retval;
}

void pktc_tbl_clear_fn(wl_pktc_tbl_t *tbl, uint8_t *da)
{
	uint16_t pktc_tbl_hash_idx = 0;
	wl_pktc_tbl_t *pt = NULL;
	int i = 0;

	/* Primary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0)
		goto free_pkts;

	/* Secondary Hash */
	pktc_tbl_hash_idx = PKTC_TBL_HASH_2(da);
	pt = &tbl[pktc_tbl_hash_idx];
	if (pt->in_use && _eacmp(pt->ea.octet, (da)) == 0)
		goto free_pkts;

	/* return: do nothing */
	return;

free_pkts:
	for (i=0; i < PKT_PRIO_LVL_CNT; i++) {
		if (pt->chain[i].chead != NULL) {
			PKTCFREE(pt->pktci->osh, pt->chain[i].chead, TRUE);
		}
	}

	/* clean up entry */
	wl_pktc_clear_entry(pt);

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
	int i, j;
	pktc_info_t *pktci;

	if (!g_pktc_tbl || !WLPKTCTBL(wl->pub->pktc_tbl)->g_stats)
		return -1;

	printk("FROM WLAN: \n");
	printk("idx  dest_MAC           dest_dev   hits\n");
	printk("----------------------------------------\n");
	for (i = 0; i < CHAIN_ENTRY_NUM; i++) {
		if ((g_pktc_tbl[i].in_use) && (g_pktc_tbl[i].wl_handle == 0)) {
			printk("%02d  %02x:%02x:%02x:%02x:%02x:%02x   %s       %d\n",
				g_pktc_tbl[i].idx,
				g_pktc_tbl[i].ea.octet[0],
				g_pktc_tbl[i].ea.octet[1],
				g_pktc_tbl[i].ea.octet[2],
				g_pktc_tbl[i].ea.octet[3],
				g_pktc_tbl[i].ea.octet[4],
				g_pktc_tbl[i].ea.octet[5],
				(g_pktc_tbl[i].tx_dev == NULL) ? "NULL" : g_pktc_tbl[i].tx_dev->name,
				g_pktc_tbl[i].hits);
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
		if ((g_pktc_tbl[i].in_use) && (g_pktc_tbl[i].wl_handle)) {
			for (j=0; j < 8; j++) {
				pktci = (pktc_info_t *)(g_pktc_tbl[i].wl_handle);
				if (pktci->stats.total_pkts[j] != 0) {
					printk("%02d    %d  %02x:%02x:%02x:%02x:%02x:%02x    ", g_pktc_tbl[i].idx,
						j,
						g_pktc_tbl[i].ea.octet[0],
						g_pktc_tbl[i].ea.octet[1],
						g_pktc_tbl[i].ea.octet[2],
						g_pktc_tbl[i].ea.octet[3],
						g_pktc_tbl[i].ea.octet[4],
						g_pktc_tbl[i].ea.octet[5]);
					printk("%s        ", (g_pktc_tbl[i].tx_dev == NULL) ?
						"NULL" : g_pktc_tbl[i].tx_dev->name);
#if defined(BCM_WFD)
					printk("%d          ", g_pktc_tbl[i].wfd_idx);
#endif
					printk("%d / %d        ",
						pktci->stats.txcurrchainsz, pktci->stats.txmaxchainsz);
					printk("%lu    %d\n", pktci->stats.total_pkts[j], g_pktc_tbl[i].pktci->stats.txdrop[j]);
				}
			}
		}
	}

	printk("\n");
	printk("transmit skb from slow path: %d\n", WLPKTCTBL(wl->pub->pktc_tbl)->g_stats->tx_slowpath_skb);
	printk("transmit fkb from slow path: %d\n", WLPKTCTBL(wl->pub->pktc_tbl)->g_stats->tx_slowpath_fkb);
	printk("received skb to slow path (fc or system): %d\n", WLPKTCTBL(wl->pub->pktc_tbl)->g_stats->rx_slowpath_skb);
	printk("total associated STAs: %d\n", WLPKTCTBL(wl->pub->pktc_tbl)->g_stats->total_stas);

	return 0;
}

#endif /* PKTC_TBL */
