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

#ifndef _wl_pktc_h_
#define _wl_pktc_h_

#include <typedefs.h>
#include <osl.h>

#include <dhd_nic_common.h>

#if !defined(BCM_PKTFWD)

#include <linux/netdevice.h>
#include <linux/skbuff.h>

struct pktc_info;
struct wl_task;
struct wl_info;
struct wl_if;

extern int wl_txq_thresh;

typedef int (*HardStartXmitFuncP)(struct sk_buff *skb, struct net_device *dev);

#define PKTC_MAX_SIZE	128 	/* max amount of chained packets to be sent at a time */

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
#ifndef PKT_PRIO_BASE_CNT
#define PKT_PRIO_BASE_CNT	8
#endif
#ifndef PKT_PRIO_LVL
#define PKT_PRIO_LVL		2
#endif
#define PKT_PRIO_LVL_CNT	PKT_PRIO_BASE_CNT * PKT_PRIO_LVL

/*
 * implement long bitmap
 */
#ifdef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
#define BITMAP_CHUNK_NUM	64   /* 2048 bits */
#else
#define BITMAP_CHUNK_NUM	8   /* 256 bits */
#endif
#define BITMAP_CHUNK_BITS	(sizeof(uint32) * 8)
#define BITMAP_CHUNK_BITSHIFT	5   /* 32 bits each chunk */
#define BITMAP_BITS_TOTAL	(BITMAP_CHUNK_NUM * BITMAP_CHUNK_BITS)
typedef struct bitmap_s {
	uint32 bit_chunk[BITMAP_CHUNK_NUM];
} pktc_bitmap_t;

#define CHAIN_ENTRY_NUM   BITMAP_BITS_TOTAL
struct chain_pair {
	struct sk_buff	*chead;
	struct sk_buff	*ctail;
};
typedef struct chain_pair c_pair_t;

struct pktc_entry {
	c_pair_t	chain[CHAIN_ENTRY_NUM];
	pktc_bitmap_t	chainidx_bitmap;
};
typedef struct pktc_entry c_entry_t;

struct pktc_stats {
	unsigned long	total_pkts[PKT_PRIO_BASE_CNT];
	uint32		txcurrchainsz;  /* current tx chain size via txchain logic */
	uint32		txmaxchainsz;   /* max tx chain size so far via txchain logic */
	uint32		txdrop[PKT_PRIO_BASE_CNT];
	uint32		tx_slowpath_skb;        /* tx skb via tx slow path */
	uint32		tx_slowpath_fkb;        /* tx fkb via tx slow path */
	uint32		rx_slowpath_skb;        /* tx skb via rx slow path */
	uint8		total_stas;	/* total associated STAs */
	uint32		n_references;  /* used to kfree() this object */
};
typedef struct pktc_stats c_stats_t;

/*
 * pktc info to be passed between threads
 */
struct pktc_info {
	c_pair_t	pktc_table[PKT_PRIO_LVL_CNT]; /* chaining table for tx */
	uint16		prio_bitmap;
	c_stats_t	stats;		/* pktc stats */
	osl_t		*osh;		/* pointer to os handler */
	void		*wlif;
	bool		_txq_txchain_dispatched;        /* dispatched flag per interface */
	spinlock_t	txchain_lock;
};
typedef struct pktc_info pktc_info_t;

/*
 * each entry of pktc table
 */
struct wl_pktc_tbl {
	struct _mac_address    ea;     /* Dest mac addr */
	uint16                 hits;    /* hit count, either to LAN or WLAN  */
	struct net_device     *tx_dev;    /* Dev to be sent */
	c_pair_t               chain[PKT_PRIO_LVL_CNT];
	uint16                 prio_bitmap;
	union {
		unsigned long  wl_handle;
		pktc_info_t    *pktci;
	};
	struct
	{
		BE_DECL(
			uint32	idx:16;		/* table idx */
			uint32	wfd_idx:2;
			uint32	sta_assoc:1;
			uint32  in_use:1;	/* Is this entry in-use */
			uint32	reserved:12; )
		LE_DECL(
			uint32	reserved:12;
			uint32	in_use:1;	/* Is this entry in-use */
			uint32	sta_assoc:1;
			uint32	wfd_idx:2;
			uint32	idx:16; )	/* table idx */
	};
	c_stats_t *g_stats; /* saved in table [0] */
};
typedef struct wl_pktc_tbl wl_pktc_tbl_t;

#define WLPKTCTBL(p)	((wl_pktc_tbl_t *)(p))

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

#ifdef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
uint16 pktc_tbl_hash(uint8 *da);
uint16 pktc_tbl_hash2(uint8 *da);
#else
uint8 pktc_tbl_hash(uint8 *da);
uint8 pktc_tbl_hash2(uint8 *da);
#endif
#define PKTC_TBL_HASH(da)	pktc_tbl_hash(da)
#define PKTC_TBL_HASH_2(da)	pktc_tbl_hash2(da)

#define PKTC_TBL_FN_CMP(brc, da, rxifp) \
    pktc_tbl_cmp_fn((wl_pktc_tbl_t *)brc, da, rxifp)
#define PKTC_TBL_FN_LOOKUP(brc, da) \
    pktc_tbl_lookup_fn((wl_pktc_tbl_t*)brc, da)
#define PKTC_TBL_FN_UPDATE(brc, da, dev, handle_p) \
    pktc_tbl_update_fn((wl_pktc_tbl_t *)brc, da, dev, handle_p)
#define PKTC_TBL_FN_CLEAR(brc, da) \
    pktc_tbl_clear_fn((wl_pktc_tbl_t *)brc, da)

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
#define PKTC_TBL_SET_STA_ASSOC		14   /* To set STA state in the pktc_tbl entry */

extern unsigned long wl_pktc_req(int rid, unsigned long p0, unsigned long p1, unsigned long p2);
extern unsigned long wl_pktc_req_with_lock(int rid, unsigned long p0, unsigned long p1, unsigned long p2);

#ifdef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT

#define PKTC_CHAIN_IDX_MASK 0xFFFF /* 16b chain idx */
#define PKTC_INVALID_CHAIN_IDX	0xFFFF
#define PKTC_WFD_IDX_BITMASK	(0x30000)
#define PKTC_WFD_IDX_BITPOS	16

#else

#define PKTC_CHAIN_IDX_MASK 0xFF /* 8b chain idx */
#define PKTC_INVALID_CHAIN_IDX	0x3FFE
#define PKTC_WFD_IDX_BITMASK	(0xC000)
#define PKTC_WFD_IDX_BITPOS	14

#endif

extern void wl_txchain_lock(pktc_info_t *pktci);
extern void wl_txchain_unlock(pktc_info_t *pktci);
extern void BCMFASTPATH wl_start_pktc(pktc_info_t *pktci, struct net_device *dev,
	struct sk_buff *skb);

extern wl_pktc_tbl_t *wl_pktc_attach(struct wl_info *wl, struct wl_if *wlif);
extern void wl_pktc_detach(struct wl_info *wl);
extern void BCMFASTPATH wl_start_txchain_txqwork(pktc_info_t *pktci);

extern int wl_pktc_init(struct wl_if *wlif, struct net_device *dev);
extern void wl_pktc_free(struct wl_if *wlif);
extern int32 wl_rxchainhandler(struct wl_info *wl, struct sk_buff *skb);

extern int wl_check_fdb_expired(unsigned char *addr,
                                struct net_device * net_device);
extern void wl_pktc_del(unsigned long addr);
extern void wl_pktc_del_ex(unsigned long addr, struct net_device * net_device);

#if defined(DSLCPE_PREALLOC_SKB)
extern void osl_pktpreallocinc(osl_t *osh, void *skb, int cnt);
extern void osl_pktpreallocdec(osl_t *osh, void *skb);

extern void wl_pktpreallocinc(uint8 unit, struct sk_buff *skb, int cnt);
extern void wl_pktpreallocdec(uint8 unit, struct sk_buff *skb);
extern bool wl_pkt_drop_on_wmark(osl_t *osh, bool is_pktc);

#define PKT_PREALLOCINC(osh, skb, c) osl_pktpreallocinc((osh), (skb), c)
#define CTF_ENAB(wl)	(!(wl)->prealloc_skb_mode)
#endif

/* DHD part */

#if defined(BCM_ROUTER_DHD)
#if defined(PKTC_TBL)
extern BCMFASTPATH unsigned long dhd_pktc_req(int req_id, unsigned long param0, unsigned long param1, unsigned long param2);
extern wl_pktc_tbl_t *dhd_pktc_attach(void *dhdp);
extern void dhd_pktc_detach(void *dhdp);
extern void dhd_pktc_dump(void *dhdp, void *buf);
extern BCMFASTPATH int32 dhd_rxchainhandler(void *dhdp, struct sk_buff *skb);
extern unsigned long (*dhd_pktc_req_hook)(int req_id, unsigned long param0, unsigned long param1, unsigned long param2);
extern void dhd_pktc_del(unsigned long addr, struct net_device * net_device);
extern void (*dhd_pktc_del_hook)(unsigned long addr,
                                 struct net_device * net_device);
#endif /* PKTC_TBL */

#if !defined(HNDCTF)
#define CTF_ENAB(x)	FALSE
#endif
#endif /* BCM_ROUTER_DHD */


#else  /* BCM_PKTFWD -------------------------------------------------------- */

/**
 * Transpose PKTC identifiers to wl_pktfwd.h
 */
#include <wl_pktfwd.h>

/** Use in bridge layer: return value of wl_pktc_req_hook(PKTC_TBL_UPDATE) is
 * tested whether valid or not.
 * If valid,
 *   wfd_idx is extracted using PKTC_WFD_IDX_BITMASK, PKTC_WFD_IDX_BITPOS.
 *   pktfwd key is passed to network processor via blog.
 */
#ifndef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
#error "BCM_PKTFWD requires 16bit key"
#endif

#define PKTC_CHAIN_IDX_MASK         0xFFFF /* 16b chain idx */
#define PKTC_INVALID_CHAIN_IDX	    WL_PKTFWD_KEY_INVALID
#define PKTC_WFD_IDX_BITMASK	    (0x30000)
#define PKTC_WFD_IDX_BITPOS	    16

/** Given a per WFD chain index, convert to global WFD_CHAIN index */
#define PKTC_WFD_CHAIN_IDX(wfd_idx, chain_idx) \
    ( (((wfd_idx) << PKTC_WFD_IDX_BITPOS) & PKTC_WFD_IDX_BITMASK) \
    | ((chain_idx) & PKTC_CHAIN_IDX_MASK) )


/** Invoked in wlc driver in rx ischainable: wlc_rx.c, wlc_amsdu.c */
#define PKTC_TBL_FN_CMP(tbl, sym, dev)  wl_pktfwd_match((sym), (dev))

/** wl_pktc_req requests redirected to wl_pktfwd_request */
#define PKTC_TBL_SET_TX_MODE        wl_pktfwd_req_set_txmode_e /*         wlc */
#define PKTC_TBL_GET_TX_MODE        wl_pktfwd_req_get_txmode_e /* bridge, wlc */
#define PKTC_TBL_UPDATE			    wl_pktfwd_req_ins_symbol_e /* bridge      */
#define PKTC_TBL_FLUSH			    wl_pktfwd_req_flush_full_e /*         wl  */
#define PKTC_TBL_SET_STA_ASSOC      wl_pktfwd_req_assoc_sta_e  /*         wls */

#define wl_pktc_req(rq, p0, p1, p2) wl_pktfwd_request((rq), (p0), (p1), (p2))

/** Redirect wlif init/free and radio attach/detach to wl_pktfwd ins/del */ 
#define wl_pktc_init(wlif, dev)     wl_pktfwd_wlif_ins(wlif)
#define wl_pktc_free(wlif)          wl_pktfwd_wlif_del(wlif)

#define wl_pktc_attach(wl, wlif)    wl_pktfwd_radio_ins(wl)
#define wl_pktc_detach(wl)          wl_pktfwd_radio_del(wl)

#define wl_rxchainhandler(wl, skb)  wl_pktfwd_upstream((wl), (skb))

#endif /* BCM_PKTFWD -------------------------------------------------------- */

/** Hook registered with Linux network stack bridge layer */
extern int (*fdb_check_expired_wl_hook)(unsigned char *addr,
                                        struct net_device * net_device);

extern int (*send_packet_to_upper_layer)(struct sk_buff *skb);
extern int inject_to_fastpath;

#if defined(PKTC)
extern unsigned long(* wl_pktc_req_hook)(int request, /* with lock */
            unsigned long param0, unsigned long param1, unsigned long param2);
#endif

#if defined(BCM_BLOG)
/** Hook registered with blog shim */
extern void (*wl_pktc_del_hook)(unsigned long addr,
                                struct net_device * net_device);
#endif

#endif /* _wl_pktc_h_ */
