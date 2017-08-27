
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
 * $Id: wl_pktc.h $
 */

#ifndef _wl_pktc_h_
#define _wl_pktc_h_

#include <linux/netdevice.h>
#include <linux/skbuff.h>

struct pktc_info;
struct wl_task;
struct wl_info;
struct wl_if;

extern int wl_txq_thresh;

typedef int (*HardStartXmitFuncP)(struct sk_buff *skb, struct net_device *dev);

#ifdef DSLCPE_CACHE_SMARTFLUSH
extern uint dsl_tx_pkt_flush_len;
#endif
#ifdef DSLCPE_TX_PRIO
/* map prio/lvl to fifo index. fifo 0 to be lowest priority queue */
extern const uint8 priolvl2fifo[];
#endif

/* Copy an ethernet address in reverse order */
#define	ether_rcopy(s, d) \
do { \
	((uint16 *)(d))[2] = ((uint16 *)(s))[2]; \
	((uint16 *)(d))[1] = ((uint16 *)(s))[1]; \
	((uint16 *)(d))[0] = ((uint16 *)(s))[0]; \
} while (0)

struct _mac_address {
	uint8 octet[6];
} __attribute__ ((packed));

/* PKT_PRIO_LVL_CNT should never exceed 16.
 * If it does make sure the prio_bitmap in the wl_pktc_tbl structure
 * and priority encoding logic between FAP and Ethernet driver for TX WLAN flows are modified
 */
#define PKT_PRIO_LVL_CNT	16 	/* PKT_PRIO_BASE_CNT * PKT_PRIO_LVL */

#define CHAIN_ENTRY_NUM   32
struct chain_pair {
	struct sk_buff *chead;
	struct sk_buff *ctail;
};
typedef struct chain_pair c_pair_t;

struct pktc_entry {
	c_pair_t chain[CHAIN_ENTRY_NUM];
	uint32       chainidx_bitmap;
};
typedef struct pktc_entry c_entry_t;

struct pktc_stats {
	unsigned long total_pkts;
	uint32  txcurrchainsz;  /* current tx chain size via txchain logic */
	uint32  txmaxchainsz;   /* max tx chain size so far via txchain logic */
	uint32  txdrop;
};
typedef struct pktc_stats c_stats_t;

struct pktc_info {
	c_entry_t	pktc_table[PKT_PRIO_LVL_CNT]; /* chaining table for tx */
	uint16		prio_bitmap;
	c_stats_t	stats;		/* pktc stats */
	osl_t		*osh;		/* pointer to os handler */
	void		*wlif;
	uint8		prio;
};
typedef struct pktc_info pktc_info_t;

struct wl_pktc_tbl {
	struct _mac_address    ea;     /* Dest mac addr */
	uint16             hits;    /* hit count */
	struct net_device  *tx_dev;    /* Dev to be sent */
	c_pair_t       chain[PKT_PRIO_LVL_CNT];
	uint16             idx; /* table idx */
	uint16             prio_bitmap;
	union {
		unsigned long	wl_handle;
		pktc_info_t	*pktci;
	};
	uint8              in_use; /* Is this entry in-use */
	struct
	{
		BE_DECL(
			uint16	wfd_idx:2;
			uint16	reserved:14; )
		LE_DECL(
			uint16	reserved:14;
			uint16	wfd_idx:2; )
	};
};
typedef struct wl_pktc_tbl wl_pktc_tbl_t;

/* compare two ethernet addresses - assumes the pointers can be referenced as shorts */
#define _eacmp(a, b)	((((uint16 *)(a))[0] ^ ((uint16 *)(b))[0]) | \
			(((uint16 *)(a))[1] ^ ((uint16 *)(b))[1]) | \
			(((uint16 *)(a))[2] ^ ((uint16 *)(b))[2]))

#define WLAN_DEVICE_MAX 32  /* supported max wlan devices (including virtual devices) */

struct pktc_handle {
	unsigned long   handle;  /* wlan handle */
	unsigned long   dev;     /* dev associated with the wlan handle */
	struct
	{
		BE_DECL(
			uint16	wfd_idx:2;
			uint16	reserved:14; )
		LE_DECL(
			uint16	reserved:14;
			uint16	wfd_idx:2; )
	};
};
typedef struct pktc_handle pktc_handle_t;

/* Hash table is divided into two part to avoid collision -
 *   First half (Primary) and if collision then second half (Secondary).
 * We can't afford to have a linear search during collision as this will affect performance.
 * By dividing in two halfs, we reduce the chance by 50%.
 * Hash functions for Primary and Secondary are different -
 *   this helps in avoiding further hash collision in secondary hash.
 */
#define PKTC_TBL_HASH(da)	((((uint8 *)da)[1] ^ ((uint8 *)da)[3] ^ ((uint8 *)da)[5]) % \
					((CHAIN_ENTRY_NUM)/2))
#define PKTC_TBL_HASH_2(da)	(((((uint8 *)da)[0] ^ ((uint8 *)da)[2] ^ ((uint8 *)da)[4]) % \
					((CHAIN_ENTRY_NUM)/2)) + ((CHAIN_ENTRY_NUM)/2))

#define PKTC_TBL_FN_CMP(brc, da, rxifp) pktc_tbl_cmp_fn(brc, da, rxifp)
#define PKTC_TBL_FN_LOOKUP(brc, da)  pktc_tbl_lookup_fn(brc, da)
#define PKTC_TBL_FN_UPDATE(brc, da, dev, handle_p) pktc_tbl_update_fn(brc, da, dev, handle_p)
#define PKTC_TBL_FN_CLEAR(brc, da) pktc_tbl_clear_fn(brc, da)

static inline int pktc_tbl_cmp_fn(wl_pktc_tbl_t *tbl, uint8_t *da, struct net_device *rxifp)
{
	wl_pktc_tbl_t *pt = tbl + PKTC_TBL_HASH(da);
	wl_pktc_tbl_t *pt2 = tbl + PKTC_TBL_HASH_2(da);

	return (((_eacmp((pt)->ea.octet, (da)) == 0) && ((pt)->tx_dev != rxifp)) ||
		((_eacmp((pt2)->ea.octet, (da)) == 0) && ((pt2)->tx_dev != rxifp)));
}
void pktc_tbl_clear_fn(wl_pktc_tbl_t *brc, uint8 *da);
unsigned long pktc_tbl_update_fn(wl_pktc_tbl_t *pt, uint8 *da, struct net_device *dev,
	pktc_handle_t *handle_p);
unsigned long pktc_tbl_lookup_fn(wl_pktc_tbl_t *pt, uint8_t *da);

/* PKTC requests */
#define PKTC_TBL_INIT			1    /* Initialize pktc table */
#define PKTC_TBL_GET_BY_DA		2    /* Get pktc_tbl pointer for pkt chaining by dest addr */
#define PKTC_TBL_GET_BY_IDX		3    /* Get pktc_tbl pointer for pkt chaining by table idx */
#define PKTC_TBL_UPDATE			4    /* To update pktc_tbl entry */
#define PKTC_TBL_UPDATE_WLAN_HANDLE	5    /* To update wlan handle */
#define PKTC_TBL_SET_TX_MODE		6    /* To set pktc tx mode: enabled=0, disabled=1 */
#define PKTC_TBL_GET_TX_MODE		7    /* To get pktc tx mode: enabled=0, disabled=1 */
#define PKTC_TBL_DELETE			8    /* To delete pktc_tbl entry */
#define PKTC_TBL_DUMP			9    /* To dump pktc_tbl table */
#define PKTC_TBL_GET_START_ADDRESS	10   /* Get the address/top of pktc_tbl table */
#define PKTC_TBL_DELETE_WLAN_HANDLE	11   /* To delete wlan handle in wldev table */
#define PKTC_TBL_UPDATE_WFD_IDX_BY_DEV	12   /* To update WFD Index by WLAN Dev */
#define PKTC_TBL_FLUSH			13   /* To flush out entire pktc_tbl table */

extern unsigned long wl_pktc_req(int rid, unsigned long p0, unsigned long p1, unsigned long p2);
extern unsigned long(* wl_pktc_req_hook)(int rid, unsigned long p0, unsigned long p1,
	unsigned long p2);

#define PKTC_INVALID_CHAIN_IDX	0x3FFE

#define PKTC_WFD_IDX_BITMASK	(0xC000)
#define PKTC_WFD_IDX_BITPOS	14

extern void wl_txchain_lock(pktc_info_t *pktci);
extern void wl_txchain_unlock(pktc_info_t *pktci);
extern void BCMFASTPATH wl_start_pktc(pktc_info_t *pktci, struct net_device *dev,
	struct sk_buff *skb);

extern int (*send_packet_to_upper_layer)(struct sk_buff *skb);
extern int inject_to_fastpath;

extern wl_pktc_tbl_t *wl_pktc_attach(struct wl_info *wl, struct wl_if *wlif);
extern void wl_pktc_detach(struct wl_info *wl);
extern void BCMFASTPATH wl_start_txchain_txqwork(struct wl_task *task);

extern int wl_pktc_init(struct wl_if *wlif, struct net_device *dev);
extern void wl_pktc_free(struct wl_if *wlif);
extern int32 wl_rxchainhandler(struct wl_info *wl, struct sk_buff *skb);

extern int (*fdb_check_expired_wl_hook)(unsigned char *addr);
extern int wl_check_fdb_expired(unsigned char *addr);
extern void wl_pktc_del(unsigned long addr);
extern void (*wl_pktc_del_hook)(unsigned long addr);

#if defined(DSLCPE_PREALLOC_SKB)
extern void osl_pktpreallocinc(osl_t *osh, void *skb, int cnt);
extern void osl_pktpreallocdec(osl_t *osh, void *skb);

extern void wl_pktpreallocinc(uint8 unit, struct sk_buff *skb, int cnt);
extern void wl_pktpreallocdec(uint8 unit, struct sk_buff *skb);
extern bool wl_pkt_drop_on_wmark(void *wl_ptr, struct sk_buff *skb, bool is_pktc);

#define PKT_PREALLOCINC(osh, skb, c) osl_pktpreallocinc((osh), (skb), c)
#define CTF_ENAB(wl)	(!(wl)->prealloc_skb_mode)
#endif

/* DHD part */

#if defined(BCM_ROUTER_DHD)
#if defined(PKTC_TBL)
extern unsigned long dhd_pktc_req(int req_id, unsigned long param0, unsigned long param1, unsigned long param2);
extern wl_pktc_tbl_t *dhd_pktc_attach(void *dhdp);
extern void dhd_pktc_dump(void *dhdp, void *buf);
extern int32 dhd_rxchainhandler(void *dhdp, struct sk_buff *skb);
extern unsigned long (*dhd_pktc_req_hook)(int req_id, unsigned long param0, unsigned long param1, unsigned long param2);
extern void dhd_pktc_del(unsigned long addr);
extern void (*dhd_pktc_del_hook)(unsigned long addr);
#endif /* PKTC_TBL */

#if !defined(HNDCTF)
#define CTF_ENAB(x)	FALSE
#endif
#endif /* BCM_ROUTER_DHD */

#endif /* _wl_pktc_h_ */
