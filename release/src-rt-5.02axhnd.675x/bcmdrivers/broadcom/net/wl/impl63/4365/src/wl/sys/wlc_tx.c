/*
 * wlc_tx.c
 *
 * Common transmit datapath components
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_tx.c 776140 2019-06-19 09:21:36Z $
 *
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlc_types.h>
#include <siutils.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <bcmwifi_channels.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/vlan.h>
#include <d11.h>
#include <wlioctl.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_pub.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_ampdu.h>
#include <wlc_scb_ratesel.h>
#include <wlc_key.h>
#include <wlc_antsel.h>
#include <wlc_stf.h>
#include <wlc_ht.h>
#include <wlc_prot.h>
#include <wlc_prot_g.h>
#define _inc_wlc_prot_n_preamble_	/* include static INLINE uint8 wlc_prot_n_preamble() */
#include <wlc_prot_n.h>
#include <wlc_apps.h>
#include <wlc_bmac.h>
#if defined(WL_PROT_OBSS) && !defined(WL_PROT_OBSS_DISABLED)
#include <wlc_prot_obss.h>
#endif // endif
#include <wlc_cac.h>
#include <wlc_vht.h>
#include <wlc_txbf.h>
#include <wlc_txc.h>
#include <wlc_keymgmt.h>
#include <wlc_led.h>
#include <wlc_ap.h>
#include <bcmwpa.h>
#include <wlc_btcx.h>
#include <wlc_p2p.h>
#include <wlc_bsscfg.h>

#include <wl_export.h>
#include <wlc_nar.h>
#include <wlc_ulb.h>
#ifdef WLTOEHW
#include <wlc_tso.h>
#endif /* WLTOEHW */
#ifdef WL_LPC
#include <wlc_scb_powersel.h>
#endif /* WL_LPC */
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif /* WLTDLS */
#ifdef WL_RELMCAST
#include "wlc_relmcast.h"
#endif /* WL_RELMCAST */
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#include <wlc_bta.h>
#endif /* WLBTAMP */
#if defined(BCMCCX) || defined(EXT_STA) || defined(STA)
#include <proto/eapol.h>
#endif /* BCMCCX || EXT_STA || STA */
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif /* WLMCNX */
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif /* WLMCHAN */
#ifdef WLAMSDU_TX
#include <wlc_amsdu.h>
#endif /* WLAMSDU_TX */
#ifdef L2_FILTER
#include <wlc_l2_filter.h>
#endif /* L2_FILTER */
#ifdef WET
#include <wlc_wet.h>
#endif /* WET */
#ifdef WET_TUNNEL
#include <wlc_wet_tunnel.h>
#endif /* WET_TUNNEL */
#ifdef WMF
#include <wlc_wmf.h>
#endif /* WMF */
#ifdef PSTA
#include <wlc_psta.h>
#endif /* PSTA */
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif /* WLTDLS */
#ifdef BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#ifdef PKTC
#include <proto/ethernet.h>
#endif /* PKTC */
#if defined(PKTC) || defined(PKTC_TX_DONGLE)
#include <proto/802.3.h>
#endif /* PKTC || PKTC_TX_DONGLE */
#ifdef STA
#include <wlc_wpa.h>
#include <wlc_pm.h>
#endif /* STA */
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif /* PROP_TXSTATUS */
#if defined(BCMWAPI_WPI) || defined(BCMWAPI_WAI)
#include <wlc_wapi.h>
#include <bcmcrypto/sms4.h>
#endif /* BCMWAPI_WPI || BCMWAPI_WAI */
#ifdef WLTEST
#include <bcmnvram.h>
#include <bcmotp.h>
#endif /* WLTEST */
#ifdef WLWNM
#include <wlc_wnm.h>
#endif // endif
#ifdef WL_PROXDETECT
#include <wlc_pdsvc.h>
#endif // endif
#ifdef WLC_HIGH_ONLY
#include <bcm_rpc_tp.h>
#include <wlc_rpctx.h>
#endif // endif
#include <wlc_txtime.h>
#include <wlc_airtime.h>
#include <wlc_11u.h>
#include <wlc_tx.h>
#include <wlc_mbss.h>

#ifdef WL11K
#include <wlc_rrm.h>
#endif // endif

#ifdef WL_MODESW
#include <wlc_modesw.h>
#endif // endif

#ifdef WLAWDL
#include <wlc_awdl.h>
#endif // endif

#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif // endif
#include <event_trace.h>

#if defined(WL_OBSS_DYNBW) && !defined(WL_OBSS_DYNBW_DISABLED)
#include <wlc_obss_dynbw.h>
#endif /* defined(WL_OBSS_DYNBW) && !defined(WL_OBSS_DYNBW_DISABLED) */

#ifdef WL_MU_TX
#include <wlc_mutx.h>
#endif // endif
#ifdef WDS
#include <wlc_wds.h>
#endif /* WDS */

#ifdef NEW_TXQ
struct txq_fifo_state {
	int flags;	/* queue state */
	int highwater;	/* target fill capacity, in usec */
	int lowwater;	/* low-water mark to re-enable flow, in usec */
	int buffered;	/* current buffered estimate, in usec */
};

struct txq_fifo_cnt {
	uint32 pkts;
	uint32 time;
	uint32 q_stops;
	uint32 q_service;
	uint32 hw_stops;
	uint32 hw_service;
};

/* Define TXQ_LOG for extra logging info with a 'dump' function "txqlog" */
#ifdef TXQ_LOG

#ifndef TXQ_LOG_LEN
#define TXQ_LOG_LEN 1024
#endif // endif

/* txq_log_entry.type values */
enum {
	TXQ_LOG_NULL = 0,
	TXQ_LOG_REQ = 1,
	TXQ_LOG_PKT,
	TXQ_LOG_DMA
};

/* sample format for a wlc_txq_fill() request */
typedef struct fill_req {
	uint32 time;    /* usec time of request */
	uint32 reqtime; /* usecs requested for fill */
	uint8 fifo;     /* AC fifo for the req */
} fill_req_t;

/* sample format for a pkt supplied to low TxQ for a fill req */
typedef struct pkt_sample {
	uint8 prio;     /* packet precidence */
	uint16 bytes;   /* MPDU bytes */
	uint32 time;    /* estimated txtime of pkt */
} pkt_sample_t;

/* sample format for a low TxQ fill of DMA ring */
typedef struct dma_fill {
	uint32 time;        /* usec time of request */
	uint16 pktq_before; /* pktq length before fill */
	uint16 pktq_after;  /* pktq length after fill */
	uint16 desc_before; /* DMA desc in use before fill */
	uint16 desc_after;  /* DMA desc in use after fill */
} dma_fill_t;

/* general sample format for txq_log */
typedef struct txq_log_entry {
	uint8 type;     /* the type of the sample union */
	union {
		fill_req_t req;
		pkt_sample_t pkt;
		dma_fill_t dma_fill;
	} u;
} txq_log_entry_t;

static void wlc_txq_log_req(txq_info_t *txqi, uint fifo, uint reqtime);
static void wlc_txq_log_pkts(txq_info_t *txqi, struct spktq* queue);
static int wlc_wme_wmmac_check_fixup(wlc_info_t *wlc, struct scb *scb, void *sdu);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_txq_log_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

#define TXQ_LOG_REQ(txqi, fifo, reqtime)	wlc_txq_log_req(txqi, fifo, reqtime)
#define TXQ_LOG_PKTS(txqi, queue)		wlc_txq_log_pkts(txqi, queue)

#else /* !TXQ_LOG */

#define TXQ_LOG_REQ(txqi, fifo, reqtime)	do {} while (0)
#define TXQ_LOG_PKTS(txqi, queue)		do {} while (0)

#endif /* TXQ_LOG */

#define TXQ_MAXFIFO 4

/* status during txfifo sync */
#define TXQ_PKT_DEL	0x01
#define HEAD_PKT_FLUSHED 0xFF

struct txq {
	txq_t *next;
	txq_supply_fn_t supply;
	void *supply_ctx;
	struct swpktq *swq;
	struct pktq unused_q;
	struct txq_fifo_state unused[NFIFO];
#ifdef WLCNT
	struct txq_fifo_cnt fifo_cnt[NFIFO_EXT];
#endif // endif
	struct txq_fifo_state fifo_state[NFIFO_EXT];
	uint nswq;
};

struct txq_info {
	wlc_info_t *wlc;
	wlc_pub_t *pub;
	osl_t *osh;
	txq_t *txq_list;
	struct spktq *delq;     /* delete queue holding pkts-to-delete temporarily */
#ifdef TXQ_LOG
	uint16 log_len;
	uint16 log_idx;
	txq_log_entry_t *log;
#endif // endif
};

/** txq_fifo_state.flags: Flow control at top of low TxQ for queue buffered time */
#define TXQ_STOPPED     0x01

/** txq_fifo_state.flags: Mask of any HW (DMA/rpctx) flow control flags */
#define TXQ_HW_MASK     0x06

/** txq_fifo_state.flags: HW (DMA/rpctx) flow control for packet count limit */
#define TXQ_HW_STOPPED  0x02

/** txq_fifo_state.flags: HW (DMA/rpctx) flow control for outside reason */
#define TXQ_HW_HOLD     0x04

#endif /* NEW_TXQ */

#if defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED))
static const uint32 txbw2rspecbw[] = {
	RSPEC_BW_20MHZ, /* WL_TXBW20L   */
	RSPEC_BW_20MHZ, /* WL_TXBW20U   */
	RSPEC_BW_40MHZ, /* WL_TXBW40    */
	RSPEC_BW_40MHZ, /* WL_TXBW40DUP */
	RSPEC_BW_20MHZ, /* WL_TXBW20LL */
	RSPEC_BW_20MHZ, /* WL_TXBW20LU */
	RSPEC_BW_20MHZ, /* WL_TXBW20UL */
	RSPEC_BW_20MHZ, /* WL_TXBW20UU */
	RSPEC_BW_40MHZ, /* WL_TXBW40L */
	RSPEC_BW_40MHZ, /* WL_TXBW40U */
	RSPEC_BW_80MHZ /* WL_TXBW80 */
};

static const uint16 txbw2phyctl0bw[] = {
	PHY_TXC1_BW_20MHZ,
	PHY_TXC1_BW_20MHZ_UP,
	PHY_TXC1_BW_40MHZ,
	PHY_TXC1_BW_40MHZ_DUP,
};

static const uint16 txbw2acphyctl0bw[] = {
	D11AC_PHY_TXC_BW_20MHZ,
	D11AC_PHY_TXC_BW_20MHZ,
	D11AC_PHY_TXC_BW_40MHZ,
	D11AC_PHY_TXC_BW_40MHZ,
	D11AC_PHY_TXC_BW_20MHZ,
	D11AC_PHY_TXC_BW_20MHZ,
	D11AC_PHY_TXC_BW_20MHZ,
	D11AC_PHY_TXC_BW_20MHZ,
	D11AC_PHY_TXC_BW_40MHZ,
	D11AC_PHY_TXC_BW_40MHZ,
	D11AC_PHY_TXC_BW_80MHZ
};

static const uint16 txbw2acphyctl1bw[] = {
	(WL_CHANSPEC_CTL_SB_LOWER >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_UPPER >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_LOWER >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_LOWER >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_LL >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_LU >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_UL >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_UU >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_LOWER >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_UPPER >> WL_CHANSPEC_CTL_SB_SHIFT),
	(WL_CHANSPEC_CTL_SB_LOWER >> WL_CHANSPEC_CTL_SB_SHIFT)
};

/* Override BW bit in phyctl */
#define WLC_PHYCTLBW_OVERRIDE(pctl, m, pctlo) \
		if (~pctlo & 0xffff)  \
			pctl = (pctl & ~(m)) | (pctlo & (m))

#else /*  defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED)) */

#define WLC_PHYCTLBW_OVERRIDE(pctl, m, pctlo)

#ifdef NEW_TXQ
static void txq_init(txq_t *txq, uint nswq,
	txq_supply_fn_t supply_fn, void *supply_ctx, int high, int low);
static void txq_hw_fill(txq_info_t *txqi, txq_t *txq, uint fifo_idx);

static int txq_space(txq_t *txq, uint fifo_idx);
static int txq_inc(txq_t *txq, uint fifo_idx, int inc_time);
static int txq_dec(txq_t *txq, uint fifo_idx, int dec_time);
static void txq_stop_set(txq_t *txq, uint fifo_idx);
static void txq_stop_clr(txq_t *txq, uint fifo_idx);
static int txq_stopped(txq_t *txq, uint fifo_idx);
static void txq_hw_stop_set(txq_t *txq, uint fifo_idx);
static void txq_hw_stop_clr(txq_t *txq, uint fifo_idx);
#ifdef WL_MULTIQUEUE
static void txq_hw_hold_set(txq_t *txq, uint fifo_idx);
static void txq_hw_hold_clr(txq_t *txq, uint fifo_idx);
#endif /* WL_MULTIQUEUE */
static int txq_hw_stopped(txq_t *txq, uint fifo_idx);

static void wlc_txq_watchdog(void *ctx);
static int wlc_txq_down(void *ctx);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_txq_module_dump(void *ctx, struct bcmstrbuf *b);
static int wlc_txq_dump(txq_info_t *txqi, txq_t *txq, struct bcmstrbuf *b);
#endif // endif

#ifdef TXQ_MUX
static uint wlc_txq_immediate_output(void *ctx, uint ac, uint request_time, struct spktq *output_q);
#endif /* TXQ_MUX */

#endif /* NEW_TXQ */

#endif /* defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED))) */

#ifdef BCMDBG_ERR
static const char *fifo_names[] = { "AC_BK", "AC_BE", "AC_VI", "AC_VO", "BCMC", "ATIM" };
#endif // endif

static void wlc_pdu_txhdr(wlc_info_t *wlc, void *p, struct scb *scb, wlc_txh_info_t *txh_info);
static int wlc_txfast(wlc_info_t *wlc, struct scb *scb, void *sdu, uint pktlen,
	wlc_key_t *key, const wlc_key_info_t *key_info);
static void wlc_update_txpktsuccess_stats(wlc_info_t *wlc, struct scb *scb, uint pkt_len,
	uint8 prio);
static void wlc_update_txpktfail_stats(wlc_info_t *wlc, uint pkt_len, uint8 prio);
static void wlc_dofrag(wlc_info_t *wlc, void *p, uint frag, uint nfrags,
	uint next_payload_len, struct scb *scb, bool is8021x,
	uint fifo, wlc_key_t *key, const wlc_key_info_t *key_info,
	uint8 prio, uint frag_length);
static struct dot11_header *wlc_80211hdr(wlc_info_t *wlc, void *p,
	struct scb *scb, bool MoreFrag, const wlc_key_info_t *key_info,
	uint8 prio, uint16 *pushlen, uint fifo);
static void* wlc_allocfrag(osl_t *osh, void *sdu, uint offset, uint headroom, uint frag_length,
	uint tailroom);
#if defined(BCMPCIEDEV) && defined(BCMFRAGPOOL)
static void *
wlc_allocfrag_txfrag(osl_t *osh, void *sdu, uint offset, uint frag_length, bool lastfrag);
#endif // endif
static INLINE uint wlc_frag(wlc_info_t *wlc, struct scb *scb, uint8 ac, uint plen, uint *flen);
static uint16 wlc_d11n_hdrs(wlc_info_t *wlc, void *p, struct scb *scb,
	uint txparams_flags, uint frag, uint nfrags, uint queue, uint next_frag_len,
	const wlc_key_info_t *key_info, ratespec_t rspec_override);
static INLINE ratespec_t wlc_d11ac_hdrs_determine_mimo_txbw(wlc_info_t *wlc, struct scb *scb,
	wlc_bsscfg_t *bsscfg, ratespec_t rspec);
static INLINE void wlc_d11ac_hdrs_rts_cts(struct scb *scb /* [in] */,
	wlc_bsscfg_t *bsscfg /* [in] */, ratespec_t rspec, bool use_rts, bool use_cts,
	ratespec_t *rts_rspec /* [in/out] */, d11actxh_rate_t *rate_hdr /* [in/out] */,
	uint8 *rts_preamble_type /* [out] */, uint16 *mcl /* [in/out] */);
static uint16 wlc_d11ac_hdrs(wlc_info_t *wlc, void *p, struct scb *scb,
	uint txparams_flags, uint frag, uint nfrags, uint queue, uint next_frag_len,
	const wlc_key_info_t *key_info, ratespec_t rspec_override);
static uint wlc_calc_frame_len(wlc_info_t *wlc, ratespec_t rate, uint8 preamble_type, uint dur);
static void wlc_txh_info_set_aging(wlc_info_t* wlc, wlc_txh_info_t* txh_info, bool enable);

static void wlc_txq_freed_pkt_time(wlc_info_t *wlc, void *pkt, uint16 *time_adj);
#ifdef WL_MULTIQUEUE
static void wlc_detach_queue(wlc_info_t *wlc, wlc_txq_info_t *qi);
static void wlc_attach_queue(wlc_info_t *wlc, wlc_txq_info_t *qi);
static void wlc_txq_free_pkt(wlc_info_t *wlc, void *pkt, uint16 *time_adj);
static void wlc_low_txq_account(wlc_info_t *wlc, txq_t *low_txq, uint fifo_idx,
	uint8 flag, uint16 *time_adj);
#endif /* WL_MULTIQUEUE */

#ifdef WLAMPDU_MAC
static void wlc_tx_fifo_epoch_save(wlc_info_t *wlc, wlc_txq_info_t *qi, uint fifo_idx);
#endif /* WLAMPDU_MAC */

#ifdef STA
static void wlc_pm_tx_upd(wlc_info_t *wlc, struct scb *scb, void *pkt, bool ps0ok, uint fifo);
#endif /* STA */

#ifdef WLAMPDU
static uint16 wlc_compute_ampdu_mpdu_dur(wlc_info_t *wlc, ratespec_t rate);
#endif /* WLAMPDU */

#ifdef EXT_STA
static void wlc_ethaddr_from_d11hdr(struct dot11_header *d11hdr, struct ether_header *ethdr);
static void *wlc_hdr_proc_safemode(wlc_info_t *wlc, void *sdu);
#endif /* EXT_STA */

int wlc_scb_peek_txfifo(wlc_info_t *wlc, struct scb *scb, void *sdu, uint *fifo);
int wlc_scb_txfifo(wlc_info_t *wlc, struct scb *scb, void *sdu, uint *fifo);

#ifdef HOST_HDR_FETCH
static void wlc_txhdr_push_prepare(wlc_info_t *wlc, void* p, wlc_txh_info_t * txh_info);
#endif // endif

/* enqueue the packet to delete queue */
static void
wlc_txq_delq_enq(void *ctx, void *pkt)
{
	txq_info_t *txqi = ctx;
	pktenq(txqi->delq, pkt);
}

/* flush the delete queue/free all the packet */
static void
wlc_txq_delq_flush(void *ctx)
{
	txq_info_t *txqi = ctx;
	pktqflush(txqi->osh, txqi->delq);
}

/** * pktq filter function to delete pkts associated with an SCB */
static pktq_filter_result_t
wlc_txq_scb_free_filter(void* ctx, void* pkt)
{
	struct scb *scb = (struct scb *)ctx;
	return (WLPKTTAGSCBGET(pkt) == scb) ? PKT_FILTER_DELETE: PKT_FILTER_NOACTION;
}

/** free all pkts asscoated with the given scb on a pktq for a prec */
void
wlc_txq_pktq_scb_pfilter(wlc_info_t *wlc, int prec, struct pktq *pq, struct scb *scb)
{
	pktq_pfilter(pq, prec, wlc_txq_scb_free_filter, scb,
		wlc_txq_delq_enq, wlc->txqi, wlc_txq_delq_flush, wlc->txqi);
}

/** free all pkts asscoated with the given scb on a pktq for given precedences */
void
wlc_txq_pktq_scb_filter(wlc_info_t *wlc, uint prec_bmp, struct pktq *pq, struct scb *scb)
{
	uint prec;
	int prec_cnt = PKTQ_MAX_PREC-1; // PKTQ_MAX_PREC is 16.

	/* Loop over all precedences set in the bitmap */
	while (prec_cnt >= 0) {
		prec = (prec_bmp & (1 << prec_cnt));
		if ((prec) && (!pktq_pempty(pq, prec_cnt))) {
			WL_PRINT(("wl%d: filter %d packets of prec=%d for scb:0x%p\n",
				wlc->pub->unit, pktq_plen(pq, prec_cnt), prec_cnt, scb));
			wlc_txq_pktq_scb_pfilter(wlc, prec_cnt, pq, scb);
		}
		prec_cnt--;
	}
}

/*
 * Clean up and fixups from wlc_txfifo()
 * This routine is called for each tx dequeue,
 * the #ifdefs make sure we have the shortest possble code path
 */
static void BCMFASTPATH
txq_hw_hdr_fixup(wlc_info_t *wlc, void *p, wlc_txh_info_t *txh_info, uint fifo)
{
	wlc_pkttag_t *pkttag = WLPKTTAG(p);

#ifdef STA
	struct dot11_header *h;
	uint16 fc;
#endif /* STA */

#if defined(STA) || defined(BCMCCX)
	struct scb *scb = WLPKTTAGSCBGET(p);
	wlc_bsscfg_t *cfg;
#endif // endif

	ASSERT(pkttag);

#if defined(STA) || defined(BCMCCX)
	ASSERT(scb);
	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);
	BCM_REFERENCE(cfg);
#endif // endif

#ifdef STA
	h = txh_info->d11HdrPtr;

	/* If the assert below fires this packet is missing the d11 header */
	ASSERT(h);
	fc = ltoh16(h->fc);
	if ((cfg) && (cfg->pm) && !(pkttag->flags3 & WLF3_NO_PMCHANGE) &&
		!(FC_TYPE(fc) == FC_TYPE_CTL && FC_SUBTYPE_ANY_PSPOLL(FC_SUBTYPE(fc))) &&
		!(FC_TYPE(fc) == FC_TYPE_DATA && FC_SUBTYPE_ANY_NULL(FC_SUBTYPE(fc)) &&
		(cfg->pm->PM == PM_MAX ? !FC_SUBTYPE_ANY_QOS(fc) : TRUE))) {
		wlc_pm_tx_upd(wlc, scb, p, TRUE, fifo);
		wlc_adv_pspoll_upd(wlc, scb, p, FALSE, fifo);
	}
#endif /* STA */

	PKTDBG_TRACE(wlc->osh, p, PKTLIST_TXFIFO);
	if ((pkttag->flags & WLF_EXPTIME)) {
		/* turn on lifetime */
		txh_info->Tstamp = pkttag->u.exptime;
		wlc_txh_info_set_aging(wlc, txh_info, TRUE);
		wlc_set_txh_info(wlc, p, txh_info);
	}

#if defined(WLPKTDLYSTAT) || defined(WL11K)
	/* Save the packet enqueue time (for latency calculations) */
	if (pkttag->shared.enqtime == 0) {
		pkttag->shared.enqtime = WLC_GET_CURR_TIME(wlc);
	}
#endif // endif

#ifdef BCMCCX
	/* timestamp the packet if AC queue is empty.  This timestamp is
	 * used to calculate the media delay, since the previous
	 * tx complete timestamp is not usable (expired).
	 */
	if (CAC_ENAB(wlc->pub) &&
	    (TXPKTPENDGET(wlc, fifo) == 0) &&
	    ((cfg)->current_bss->ccx_version >= 4))
		wlc_ccx_tsm_mediadelay(wlc->cac, fifo, p, scb);
#endif	/* BCMCCX */

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		WLFC_COUNTER_TXSTATUS_TOD11(wlc);
	}
#endif // endif
}

/* Place holder for tx datapath functions
 * Refer to RB http://wlan-rb.sj.broadcom.com/r/18439/ and
 * TWIKI http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/WlDriverTxQueuingUpdate2013
 * for details on datapath restructuring work
 * These functions will be moved in stages from wlc.[ch]
 */

#ifdef NEW_TXQ
static void BCMFASTPATH
txq_stop_clr(txq_t *txq, uint fifo_idx)
{
	txq->fifo_state[fifo_idx].flags &= ~TXQ_STOPPED;
}

static int BCMFASTPATH
txq_stopped(txq_t *txq, uint fifo_idx)
{
	return (txq->fifo_state[fifo_idx].flags & TXQ_STOPPED) != 0;
}

uint8
txq_stopped_map(txq_t *txq)
{
	uint fifo_idx;
	uint8 stop_map = 0;

	for (fifo_idx = 0; fifo_idx < txq->nswq; fifo_idx++) {
		if (txq_stopped(txq, fifo_idx)) {
			stop_map |= 1 << fifo_idx;
		}
	}

	return stop_map;
}

struct swpktq* BCMFASTPATH
wlc_low_txq(txq_t *txq)
{
	ASSERT(txq);
	return txq->swq;
}

int
wlc_txq_buffered_time(txq_t *txq, uint fifo_idx)
{
	return txq->fifo_state[fifo_idx].buffered;
}

static int BCMFASTPATH
txq_space(txq_t *txq, uint fifo_idx)
{
	return txq->fifo_state[fifo_idx].highwater - txq->fifo_state[fifo_idx].buffered;
}

static int BCMFASTPATH
txq_inc(txq_t *txq, uint fifo_idx, int inc_time)
{
	int rem;

	rem = (txq->fifo_state[fifo_idx].buffered += inc_time);

	if (rem < 0) {
		WL_ERROR(("%s rem: %d fifo:%d\n", __FUNCTION__, rem, fifo_idx));
	}

	ASSERT(rem >= 0);

	return rem;
}

static int BCMFASTPATH
txq_dec(txq_t *txq, uint fifo_idx, int dec_time)
{
	int rem;

	rem = (txq->fifo_state[fifo_idx].buffered -= dec_time);

	if (rem < 0) {
		WL_ERROR(("%s txq 0x%p rem: %d fifo:%d dec_time:%d \n",
			__FUNCTION__, txq, rem, fifo_idx, dec_time));
	}
	ASSERT(rem >= 0);

	return rem;
}

extern int
wlc_low_txq_buffered_inc(txq_t *txq, uint fifo_idx, int inc_time)
{
	return txq_inc(txq, fifo_idx, inc_time);
}

extern int
wlc_low_txq_buffered_dec(txq_t *txq, uint fifo_idx, int dec_time)
{
	return txq_dec(txq, fifo_idx, dec_time);
}

static void BCMFASTPATH
txq_stop_set(txq_t *txq, uint fifo_idx)
{
	txq->fifo_state[fifo_idx].flags |= TXQ_STOPPED;
}

static void BCMFASTPATH
txq_hw_stop_set(txq_t *txq, uint fifo_idx)
{
	txq->fifo_state[fifo_idx].flags |= TXQ_HW_STOPPED;
}

static void BCMFASTPATH
txq_hw_stop_clr(txq_t *txq, uint fifo_idx)
{
	txq->fifo_state[fifo_idx].flags &= ~TXQ_HW_STOPPED;
}

#ifdef WL_MULTIQUEUE

static void BCMFASTPATH
txq_hw_hold_set(txq_t *txq, uint fifo_idx)
{
	txq->fifo_state[fifo_idx].flags |= TXQ_HW_HOLD;
}

static void BCMFASTPATH
txq_hw_hold_clr(txq_t *txq, uint fifo_idx)
{
	txq->fifo_state[fifo_idx].flags &= ~TXQ_HW_HOLD;
}

#endif /* WL_MULTIQUEUE */

static int BCMFASTPATH
txq_hw_stopped(txq_t *txq, uint fifo_idx)
{
	return (txq->fifo_state[fifo_idx].flags & TXQ_HW_MASK) != 0;
}

#if defined(BCM_DMA_CT) && defined(WLC_LOW)
void
wlc_tx_dma_timer(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc_bmac_sched_aqm_fifo(wlc->hw);
}
#endif /* defined(BCM_DMA_CT) && defined(WLC_LOW) */

static void BCMFASTPATH
txq_hw_fill(txq_info_t *txqi, txq_t *txq, uint fifo_idx)
{
	void *p;
	struct swpktq *swq;
	osl_t *osh;
	wlc_info_t *wlc = txqi->wlc;
	wlc_pub_t *pub = txqi->pub;
	wlc_txh_info_t txh_info;
	bool commit = 1;
#if defined(BCM_DMA_CT) && defined(WLC_LOW)
	bool firstentry = TRUE;
#endif /* BCM_DMA_CT && WLC_LOW */
#ifdef WLAMPDU
	bool sw_ampdu;
#endif /* WLAMPDU */
#ifdef WLC_HIGH_ONLY
	uint8 txpktpend = 1; /* to be removed */
#endif // endif
	wlc_bsscfg_t *bsscfg;
#ifdef WLCNT
	struct txq_fifo_cnt *cnt;
#endif // endif
#ifdef HOST_HDR_FETCH
	bool pushtxhdr_enab = HOST_HDR_FETCH_ENAB();
#endif /* HOST_HDR_FETCH */
#ifdef WLAMPDU
	sw_ampdu = (AMPDU_ENAB(pub) && AMPDU_HOST_ENAB(pub));
#endif /* WLAMPDU */

	swq = txq->swq;
	osh = txqi->osh;

	BCM_REFERENCE(osh);
	BCM_REFERENCE(pub);
	BCM_REFERENCE(bsscfg);

#ifdef WLC_HIGH_ONLY
	/* Start aggregating rpc calls as we pass down the individual pkts in this AMPDU */
	if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU) {
		bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU, TRUE);
	}
#endif // endif

	if (!PIO_ENAB(wlc->pub)) {
		uint queue;
#if defined(BCM_DMA_CT) && defined(WLC_LOW)
		uint prev_queue;
		bool bulk_commit = FALSE;
#ifdef HOST_HDR_FETCH
		bool pushtxdr_bulk_commit = FALSE;
#endif /* HOST_HDR_FETCH */

		queue = prev_queue = WLC_HW_NFIFO_INUSE(wlc);
		if (BCM_DMA_CT_ENAB(wlc)) {
			/* in CutThru dma do not update dma for every pkt. */
			commit = FALSE;
		}
#endif /* BCM_DMA_CT && WLC_LOW */

		while ((p = pktq_swdeq(swq, fifo_idx))) {
			uint ndesc;

#ifdef WLC_LOW
			struct scb *scb = WLPKTTAGSCBGET(p);
#endif // endif

			/* calc the number of descriptors needed to queue this frame */

			/* for unfragmented packets, count the number of packet buffer
			 * segments, each being a contiguous virtual address range.
			 */

			bsscfg = wlc->bsscfg[WLPKTTAGBSSCFGGET(p)];
			ASSERT(bsscfg != NULL);

			ndesc = pktsegcnt(osh, p);

			if (wlc->dma_avoidance_war)
				ndesc *= 2;

			wlc_get_txh_info(wlc, p, &txh_info);

			queue = WLC_TXFID_GET_QUEUE(ltoh16(txh_info.TxFrameID));

#if defined(BCM_DMA_CT) && defined(WLC_LOW)
			if (BCM_DMA_CT_ENAB(wlc)) {
				/* take care of queue change */
				if ((queue != prev_queue) &&
					(prev_queue != WLC_HW_NFIFO_INUSE(wlc))) {
#ifdef HOST_HDR_FETCH
						if (pushtxhdr_enab) {
							if (pushtxdr_bulk_commit) {
								wl_txhdr_commit(wlc->wl);
								pushtxdr_bulk_commit = FALSE;
							}
						} else
#endif // endif
						{
							dma_commit(WLC_HW_DI(wlc, prev_queue));
							if (wlc->cpbusy_war) {
								wlc_bmac_sched_aqm_fifo(wlc->hw);
							} else {
								dma_commit(WLC_HW_AQM_DI(wlc,
									prev_queue));
							}
							bulk_commit = FALSE;
						}
				}
			}
#endif /* BCM_DMA_CT && WLC_LOW */
			if (txq_hw_stopped(txq, queue)) {
				/* return the packet to the queue */
				pktq_swenq_head(swq, fifo_idx, p);
				break;
			}
#ifdef WLCNT
			cnt = &txq->fifo_cnt[queue];
#endif // endif
			WLCNTINCR(cnt->hw_service);

			/* check for sufficient dma resources */
			if ((!HOST_HDR_FETCH_ENAB() && TXAVAIL(wlc, queue) <= ndesc) ||
#ifdef HOST_HDR_FETCH
				(HOST_HDR_FETCH_ENAB() && ((__TXAVAIL(wlc, queue) <= ndesc) ||
				(BCM_DMA_CT_ENAB(wlc) && !__AQMTXAVAIL(wlc, queue)))) ||
#endif // endif
				0) {
				/* this pkt will not fit on the dma ring */

				/* mark this hw fifo as stopped */
				txq_hw_stop_set(txq, queue);
				WLCNTINCR(cnt->hw_stops);

				/* return the packet to the queue */
				pktq_swenq_head(swq, fifo_idx, p);
				break;
			}

			/* We are done with WLF_FIFOPKT regardless */
			WLPKTTAG(p)->flags &= ~WLF_FIFOPKT;

			/* We are done with WLF_TXCMISS regardless */
			WLPKTTAG(p)->flags &= ~WLF_TXCMISS;

			/* Apply misc fixups for state and powersave */
			txq_hw_hdr_fixup(wlc, p, &txh_info, queue);
#ifdef HOST_HDR_FETCH
			if (pushtxhdr_enab)
				goto push_txhdr;
#endif // endif

#ifdef WLC_LOW
			/* When a BC/MC frame is being committed to the BCMC
			 * fifo via DMA (NOT PIO), update
			 * ucode or BSS info as appropriate.
			 */
			if (queue == TX_BCMC_FIFO) {
				uint16 frameid = ltoh16(txh_info.TxFrameID);

#if defined(MBSS)
				/* For MBSS mode, keep track of the
				 * last bcmc FID in the bsscfg info.
				 * A snapshot of the FIDs for each BSS
				 * will be committed to shared memory at DTIM.
				 */
				if (MBSS_ENAB(pub)) {
					bsscfg->bcmc_fid = frameid;
#if defined(DONGLEBUILD)
					if (!BCMDHDHDR_ENAB())
						PKTSETFRAMEID(p, frameid);
#endif // endif
					wlc_mbss_txq_update_bcmc_counters(wlc, bsscfg, p);
				} else
#endif /* MBSS */
				{
					/*
					 * Commit BCMC sequence number in
					 * the SHM frame ID location
					*/
					wlc_bmac_write_shm(wlc->hw, M_BCMC_FID, frameid);
				}
			}

			/* XXX:TXQ WES: Hope this can be removed.
			 * Reference JIRA:SWWLAN-40902
			 */

			if (WLC_WAR16165(wlc)) {
				wlc_war16165(wlc, TRUE);
			}

#ifdef BCM_DMA_CT
			/* XXX:CRWLDOT11M-2187, 2432 WAR, avoid AQM hang when PM !=0.
			 * A 10us delay is needed for the 1st entry of posting
			 * if the override is already ON for WAR this issue and
			 * not suffer much penalty.
			 */
			if (BCM_DMA_CT_ENAB(wlc) && firstentry &&
				wlc->cfg->pm->PM != PM_OFF && (D11REV_IS(wlc->pub->corerev, 65))) {
				if (!(wlc->hw->maccontrol & MCTL_WAKE) &&
				    !(wlc->hw->wake_override & WLC_WAKE_OVERRIDE_CLKCTL)) {
				  wlc_ucode_wake_override_set(wlc->hw, WLC_WAKE_OVERRIDE_CLKCTL);
				} else {
					OSL_DELAY(10);
				}
				firstentry = FALSE;
			} else
#endif /* BCM_DMA_CT */
			{
			/*
			 * PR 113378: Checking for the PM wake override bit before calling the
			 * override.
			 * PR 107865: DRQ output should be latched before being written to DDQ.
			 */
			if (((D11REV_IS(wlc->pub->corerev, 41)) ||
			     (D11REV_IS(wlc->pub->corerev, 44))) &&
			    (!(wlc->hw->maccontrol & MCTL_WAKE) &&
			     !(wlc->hw->wake_override & WLC_WAKE_OVERRIDE_4335_PMWAR)))
				wlc_ucode_wake_override_set(wlc->hw, WLC_WAKE_OVERRIDE_4335_PMWAR);
			}

#ifdef WLAMPDU
			/* Toggle commit bit on last AMPDU when using software/host aggregation */
			if (sw_ampdu) {
				uint16 mpdu_type =
				        (((ltoh16(txh_info.MacTxControlLow)) &
				          TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT);
				commit = ((mpdu_type == TXC_AMPDU_LAST) ||
				          (mpdu_type == TXC_AMPDU_NONE));
			}
#endif /* WLAMPDU */
#ifdef HOST_HDR_FETCH
push_txhdr:
			if (pushtxhdr_enab) {
				wlc_txhdr_push_prepare(wlc, p, &txh_info);
				/* Hand off the packet to pciedev layer */
				if (wl_txhdr_push(wlc->wl, p, queue, commit) != BCME_OK) {
					/* return the packet to the queue */
					WLPKTTAGSCBSET(p, scb);
					pktq_swenq_head(swq, fifo_idx, p);

					break;
				}

#if defined(BCM_DMA_CT) && defined(WLC_LOW)
				pushtxdr_bulk_commit = TRUE;
				prev_queue = queue;
#endif /* BCM_DMA_CT && WLC_LOW */
				/* Update pending count */
				TXDMAPEND_INCR(wlc, queue, ndesc);
				AQMDMA_PENDINCR(wlc, queue, 1);
			} else
#endif /* HOST_HDR_FETCH */
			{
				if (wlc_bmac_dma_txfast(wlc, queue, p, commit) < 0) {
					/* the dma did not have enough room to take the pkt */
					/* mark this hw fifo as stopped */
					txq_hw_stop_set(txq, queue);
					WLCNTINCR(cnt->hw_stops);

					/* return the packet to the queue */
					WLPKTTAGSCBSET(p, scb);
					pktq_swenq_head(swq, fifo_idx, p);
					break;
				}
#if defined(BCM_DMA_CT) && defined(WLC_LOW)
				if (BCM_DMA_CT_ENAB(wlc)) {
					prev_queue = queue;
					bulk_commit = TRUE;
				}
#endif // endif
			}
			PKTDBG_TRACE(osh, p, PKTLIST_DMAQ);

			/* WES: Check if this is still needed. HW folks should be in on this. */
			/* XXX: WAR for in case pkt does not wake up chip, read maccontrol reg
			 * tracked by PR94097
			 */
			if ((BCM4331_CHIP_ID == CHIPID(wlc->pub->sih->chip) ||
				BCM4350_CHIP(wlc->pub->sih->chip) ||
				BCM4352_CHIP_ID == CHIPID(wlc->pub->sih->chip) ||
				BCM43602_CHIP_ID == CHIPID(wlc->pub->sih->chip) ||
				BCM4360_CHIP_ID == CHIPID(wlc->pub->sih->chip)) &&
				(bsscfg->pm->PM))
			{
					(void)R_REG(wlc->osh, &wlc->regs->maccontrol);
			}

#else
			if (RPCTX_ENAB(pub)) {
				uint16 frameid;

				/* frameid for BCMCFID shm. Not used for MBSS */
				if (queue == TX_BCMC_FIFO &&
				    !MBSS_ENAB(pub)) {
					frameid = ltoh16(txh_info.TxFrameID);
				} else {
					frameid = INVALIDFID;
				}

#ifdef WLAMPDU
				/* For software/host mode ampdu commit only if it is the last
				 * MPDU in the AMPDU, singleton AMPDU or regular MPDU
				 */
				if (sw_ampdu) {
					uint16 mpdu_type =
						(((ltoh16(txh_info.MacTxControlLow)) &
						TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT);
					if ((mpdu_type == TXC_AMPDU_FIRST) ||
					    (mpdu_type == TXC_AMPDU_MIDDLE)) {
						/* non-last MPDUs in an AMPDU do not increase
						 * the a pkt count or cause a commit
						 */
						txpktpend = 0;
						commit = 0;
					} else if (mpdu_type == TXC_AMPDU_LAST) {
						/* the entire AMPDU is given a weighted pkt count */
						txpktpend =
						        wlc_ampdu_get_txpkt_weight(wlc->ampdu_tx);
						commit = 1;
					} else {
						txpktpend = 1;
						commit = 1;
					}
				}
#endif /* WLAMPDU */

				WL_PRHDRS(wlc, __FUNCTION__, txh_info.d11HdrPtr,
					txh_info.hdrPtr, NULL,
					PKTLEN(osh, p) - (int)((int8*)txh_info.d11HdrPtr -
					(int8*)PKTDATA(osh, p)));

				if (wlc_rpctx_tx(wlc->rpctx,
					queue, p, commit, frameid, txpktpend)) {
					/* The RPC queue did not have enough room to take the pkt */

					/*
					 * Mark this hw fifo as stopped
					 * If the error is due to congestion the txcomplete will
					 * restart the queue, if not the datapath stops.
					 * This is done to prevent additional new packets from
					 * stomping over the contents of dongle state and memory
					 * and allow for subsequent investigation/debug
					 */
					txq_hw_stop_set(txq, queue);
					WLCNTINCR(cnt->hw_stops);

					/* Return the packet to the queue */
					pktq_swenq_head(swq, fifo_idx, p);

					WL_TRACE(("%s RPC Requeue Frame:0x%x FIFO:%d\n",
						__FUNCTION__, frameid, queue));
					break;
				}
			}
#endif /* WLC_LOW */
		}

		/* WES: Here would be a good spot to do the 'commit' work---DMA register write
		 * of Ptr to signal the addition of all the newly posted descriptors.
		 */
#if defined(BCM_DMA_CT) && defined(WLC_LOW)
#ifdef HOST_HDR_FETCH
		/* Done with the loop: do the bulk commit now */
		if (BCM_DMA_CT_ENAB(wlc) && pushtxdr_bulk_commit) {
			wl_txhdr_commit(wlc->wl);
		}
#endif /* HOST_HDR_FETCH */

		if (BCM_DMA_CT_ENAB(wlc) && bulk_commit) {
			ASSERT(queue < WLC_HW_NFIFO_INUSE(wlc));
			dma_commit(WLC_HW_DI(wlc, queue));
			if (wlc->cpbusy_war) {
				wlc_bmac_sched_aqm_fifo(wlc->hw);
			} else {
				dma_commit(WLC_HW_AQM_DI(wlc, queue));
			}
		}
#endif /* BCM_DMA_CT && WLC_LOW */
	} else {
#ifdef WLCNT
		cnt = &txq->fifo_cnt[fifo_idx];
#endif // endif
		while ((p = pktq_swdeq(swq, fifo_idx))) {
			uint nbytes = pkttotlen(osh, p);
			BCM_REFERENCE(nbytes);

			/* return if insufficient pio resources */
			if (!wlc_pio_txavailable(WLC_HW_PIO(wlc, fifo_idx), nbytes, 1)) {
				/* mark this hw fifo as stopped */
				txq_hw_stop_set(txq, fifo_idx);
				WLCNTINCR(cnt->hw_stops);
				break;
			}

			/* Following code based on original wlc_txfifo() */
			wlc_pio_tx(WLC_HW_PIO(wlc, fifo_idx), p);
		}
	}

#ifdef WLC_HIGH_ONLY
	/* Stop the rpc packet aggregation and release all queued rpc packets */
	if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU) {
		bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU, FALSE);
	}
#endif // endif
}

static void
wlc_txq_watchdog(void *ctx)
{
	txq_info_t *txqi = (txq_info_t*)ctx;

	/* Check txfifo complete to open datafifo */
	wlc_tx_open_datafifo(txqi->wlc);

	return;
}

static int
wlc_txq_down(void *ctx)
{
	txq_info_t *txqi = (txq_info_t*)ctx;
	txq_t *txq;

	for (txq = txqi->txq_list; txq != NULL; txq = txq->next) {
		wlc_low_txq_flush(txqi, txq);
	}

	return 0;
}

static void
wlc_low_txq_set_watermark(txq_t *txq, int highwater, int lowwater)
{
	uint i;

	for (i = 0; i < txq->nswq; i++) {
		if (highwater >= 0) {
			txq->fifo_state[i].highwater = highwater;
		}
		if (lowwater >= 0) {
			txq->fifo_state[i].lowwater = lowwater;
		}
	}
}

static void
txq_init(txq_t *txq, uint nswq, txq_supply_fn_t supply_fn, void *supply_ctx, int high, int low)
{
	txq->next = NULL;
	txq->supply = supply_fn;
	txq->supply_ctx = supply_ctx;
	txq->nswq = nswq;

	pktq_sw_init(txq->swq, txq->nswq, -1);

	/*
	 * If TXQ_MUX is not defined the watermarks will be reset to
	 * to packet values based on txmaxpkts when
	 * the AMPDU module finishes initialization
	 */
	wlc_low_txq_set_watermark(txq, high, low);
}

txq_t*
wlc_low_txq_alloc(txq_info_t *txqi, txq_supply_fn_t supply_fn, void *supply_ctx,
	uint nswq, int high, int low)
{
	txq_t *txq;

	/* The nswq managed by the low txq can never exceed the count of hw fifos */
	ASSERT(nswq <= NFIFO_EXT);

	/* Allocate private state struct */
	if ((txq = MALLOCZ(txqi->osh, sizeof(txq_t))) == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          txqi->pub->unit, __FUNCTION__, MALLOCED(txqi->osh)));
		return NULL;
	}

	/* Allocate software queue */
	if ((txq->swq = MALLOCZ(txqi->osh, sizeof(struct swpktq))) == NULL) {
		MFREE(txqi->osh, txq, sizeof(txq_t));
		WL_ERROR(("wl%d: %s: MALLOC software queue failed, malloced %d bytes\n",
		          txqi->pub->unit, __FUNCTION__, MALLOCED(txqi->osh)));
		return NULL;
	}

	txq_init(txq, nswq, supply_fn, supply_ctx, high, low);

	/* add new queue to the list */
	txq->next = txqi->txq_list;
	txqi->txq_list = txq;

	return txq;
}

void
wlc_low_txq_free(txq_info_t *txqi, txq_t* txq)
{
	txq_t *p;

	if (txq == NULL) {
		return;
	}

	wlc_low_txq_flush(txqi,  txq);

	/* remove the queue from the linked list */
	p = txqi->txq_list;
	if (p == txq)
		txqi->txq_list = txq->next;
	else {
		while (p != NULL && p->next != txq)
			p = p->next;

		if (p != NULL) {
			p->next = txq->next;
		} else {
			/* assert that we found txq before getting to the end of the list */
			WL_ERROR(("%s: did not find txq %p\n", __FUNCTION__, txq));
			ASSERT(p != NULL);
		}
	}
	txq->next = NULL;

	MFREE(txqi->osh, txq->swq, sizeof(*txq->swq));
	MFREE(txqi->osh, txq, sizeof(*txq));
}

/* Cleanup function for the low_txq.
* Frees all packets belonging to the given scb 'remove' in the low_txq
*/
#ifdef NEW_TXQ
static void *wlc_peek_lastpkt(wlc_info_t *wlc, uint fifo)
{
	void **dmapkt_list = NULL;
	hnddma_t* di = NULL;
	int size;
	int n = TXPKTPENDGET(wlc, fifo);
	void *pkt = NULL;

	if (n == 0)
		return NULL;

	size = n * sizeof(void *);
	dmapkt_list = MALLOCZ(wlc->osh, size);
	if (!dmapkt_list) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		return NULL;
	}

	di = WLC_HW_DI(wlc, fifo);
	if (di && (dma_peekntxp(di, &n, dmapkt_list, HNDDMA_RANGE_ALL) == BCME_OK)) {
		if (n > 0)
			pkt = dmapkt_list[n-1];
	}
	MFREE(wlc->osh, (void *)dmapkt_list, size);
	return (pkt);
}

void
wlc_low_txq_scb_flush(wlc_info_t *wlc, wlc_txq_info_t *qi, struct scb *remove)
{
	uint swq;
	void *head_pkt, *pkt;
	txq_t *low_txq = qi->low_txq;
	struct swpktq *free_swq = wlc_low_txq(low_txq);
#if defined(WLAMPDU_MAC)
	uint8 flipEpoch;
	uint8 lastEpoch;
	wlc_txh_info_t txh_info;
	uint8 *tsoHdrPtr;
#endif /* defined(WLAMPDU_MAC) */

	for (swq = 0; swq < low_txq->nswq; swq++) {
#if defined(WLAMPDU_MAC)
		flipEpoch = 0;
		lastEpoch = HEAD_PKT_FLUSHED;
		pkt = wlc_peek_lastpkt(wlc, swq);
		if (pkt) {
#ifdef HOST_HDR_FETCH
			if (__PKTISHDRINHOST(wlc->osh, pkt)) {
				lastEpoch = PKTFRAGEPOCH(wlc->osh, pkt);
			} else
#endif // endif
			{
				wlc_get_txh_info(wlc, pkt, &txh_info);
				tsoHdrPtr = txh_info.tsoHdrPtr;
				lastEpoch = (tsoHdrPtr[2] & TOE_F2_EPOCH);
			}
		}
#endif /* WLAMPDU_MAC */
		head_pkt = NULL;
		while (pktq_ppeek(free_swq, swq) != head_pkt) {
			pkt = pktq_swdeq(free_swq, swq);
			if (WLPKTTAGSCBGET(pkt) != remove) {
				if (!head_pkt)
					head_pkt = pkt;
#ifdef WLAMPDU_MAC
				wlc_epoch_upd(wlc, pkt, &flipEpoch, &lastEpoch);
				/* clear pkt delete condition */
				flipEpoch &= ~TXQ_PKT_DEL;
#endif /* WLAMPDU_MAC */
				pktq_swenq(free_swq, swq, pkt);
			} else {
				uint fifo_idx;
				int buffered, lowwater;
				uint16 fid = wlc_get_txh_frameid(wlc, pkt);
				fifo_idx = WLC_TXFID_GET_QUEUE(fid);

				if (wlc->active_queue->low_txq == low_txq)
					TXPKTPENDDEC(wlc, fifo_idx, 1);

				/* The packet belongs to the SCB being removed.
				 * Delete the packet and update the bookkeeping for
				 * the low_txq buffered time.
				 */
				wlc_low_txq_buffered_dec(low_txq, fifo_idx, 1);
				PKTFREE(wlc->osh, pkt, TRUE);
#if defined(WLAMPDU_MAC)
				flipEpoch |= TXQ_PKT_DEL;
#endif /* defined(WLAMPDU_MAC) */

				buffered = low_txq->fifo_state[fifo_idx].buffered;
				lowwater = low_txq->fifo_state[fifo_idx].lowwater;
				if (buffered <= lowwater)
					txq_stop_clr(low_txq, fifo_idx);
			}
		}
#ifdef WLAMPDU_MAC
		if (AMPDU_AQM_ENAB(wlc->pub)) {
			wlc_tx_fifo_epoch_save(wlc, qi, swq);
		}
#endif /* WLAMPDU_MAC */
	}
}

typedef struct {
	int time;
	uint16 pkt_cnt;
	map_pkts_cb_fn orig_cb;
	void *orig_ctx;
	wlc_info_t *wlc;
} low_txq_map_t;

static bool
wlc_low_txq_pkt_cb(void *ctx, void *pkt)
{
	bool ret = FALSE;
	uint16 *buffered;
	low_txq_map_t *low_txq_map = (low_txq_map_t *)ctx;

	ret = low_txq_map->orig_cb(low_txq_map->orig_ctx, pkt);

	if (ret == TRUE) {
#if defined(TXQ_MUX)
		buffered = &low_txq_map->time;
#else
		buffered = &low_txq_map->pkt_cnt;
#endif // endif
		wlc_txq_freed_pkt_time(low_txq_map->wlc, pkt, buffered);
	}
	return ret;
}

static void
wlc_low_tx_map_pkts(wlc_info_t *wlc, struct swpktq *swq, int fifo, map_pkts_cb_fn cb, void *ctx)
{
	void *head_pkt, *pkt;
	bool pkt_free;

	head_pkt = NULL;
	while (pktq_ppeek(swq, fifo) != head_pkt) {
		pkt = pktq_swdeq(swq, fifo);
		pkt_free = cb(ctx, pkt);
		if (!pkt_free) {
			if (!head_pkt)
				head_pkt = pkt;
			pktq_swenq(swq, fifo, pkt);
		} else
			PKTFREE(wlc->osh, pkt, TRUE);
	}
}

void
wlc_low_txq_map_pkts(wlc_info_t *wlc, wlc_txq_info_t *qi, map_pkts_cb_fn cb, void *ctx)
{
	uint swq;
	txq_t *low_txq;
	struct swpktq *free_swq;
	low_txq_map_t low_txq_map;

	low_txq = qi->low_txq;
	free_swq = wlc_low_txq(low_txq);

	low_txq_map.orig_cb = cb;
	low_txq_map.orig_ctx = ctx;
	low_txq_map.wlc = wlc;
	for (swq = 0; swq < low_txq->nswq; swq++) {
		low_txq_map.pkt_cnt  = 0;
		low_txq_map.time  = 0;
		wlc_low_tx_map_pkts(wlc, free_swq, swq, wlc_low_txq_pkt_cb, &low_txq_map);
#if defined(TXQ_MUX)
		wlc_low_txq_buffered_dec(low_txq, swq, low_txq_map.time);
#else
		wlc_low_txq_buffered_dec(low_txq, swq, low_txq_map.pkt_cnt);
#endif // endif
	}
}
#endif /* NEW_TXQ */

void
wlc_tx_map_pkts(wlc_info_t *wlc, struct pktq *q, int prec, map_pkts_cb_fn cb, void *ctx)
{
	void *head_pkt, *pkt;
	bool pkt_free;

	head_pkt = NULL;
	while (pktq_ppeek(q, prec) != head_pkt) {
		pkt = pktq_pdeq(q, prec);
		pkt_free = cb(ctx, pkt);
		if (!pkt_free) {
			if (!head_pkt)
				head_pkt = pkt;
			pktq_penq(q, prec, pkt);
		} else
			PKTFREE(wlc->osh, pkt, TRUE);
	}
}

void
wlc_txq_map_pkts(wlc_info_t *wlc, wlc_txq_info_t *qi, map_pkts_cb_fn cb, void *ctx)
{
	int prec;
	struct pktq *q;
	q =  WLC_GET_TXQ(qi);

	PKTQ_PREC_ITER(q, prec) {
		wlc_tx_map_pkts(wlc, q, prec, cb, ctx);
	}
}

#ifdef AP
void
wlc_scb_psq_map_pkts(wlc_info_t *wlc, struct pktq *q, map_pkts_cb_fn cb, void *ctx)
{
	int prec;

	PKTQ_PREC_ITER(q, prec) {
		wlc_tx_map_pkts(wlc, q, prec, cb, ctx);
	}
}
#endif // endif

void
wlc_bsscfg_psq_map_pkts(wlc_info_t *wlc, struct pktq *q, map_pkts_cb_fn cb, void *ctx)
{
	int prec;

	PKTQ_PREC_ITER(q, prec) {
		wlc_tx_map_pkts(wlc, q, prec, cb, ctx);
	}
}

void
wlc_low_txq_flush(txq_info_t *txqi, txq_t* txq)
{
	uint fifo_idx;
	uint pktcnt;
	wlc_info_t *wlc = txqi->wlc;
	BCM_REFERENCE(wlc);

	for (fifo_idx = 0; fifo_idx < txq->nswq; fifo_idx++) {
		/* flush any pkts in the software queues */
		pktcnt = pktq_plen(txq->swq, fifo_idx);
		if (pktcnt) {
			pktq_swflush(txqi->osh, txq->swq, fifo_idx, TRUE, NULL, 0);
			if (wlc->active_queue->low_txq == txq)
				TXPKTPENDDEC(wlc, fifo_idx, pktcnt);
			else {
				WL_ERROR(("%s: skip TXPKTENDDEC for inactive(%s) low_txq"
					" fifo %d pkts %d\n",
					(txq == wlc->primary_queue->low_txq)?"primary":"excursion",
					__FUNCTION__, fifo_idx, pktcnt));
			}

			wlc_low_txq_buffered_dec(txq, fifo_idx, pktcnt);

			WL_ERROR(("%s: flush fifo %d pkts %d\n", __FUNCTION__, fifo_idx, pktcnt));
		}

#ifdef WL_MULTIQUEUE
		if (txq->fifo_state[fifo_idx].buffered != 0) {
			WL_ERROR(("wl%d: %s: txq 0x%p primary 0x%p excursion 0x%p"
				" txq->fifo_state[%u].buffered=%d\n",
				wlc->pub->unit, __FUNCTION__,
				txq, wlc->primary_queue->low_txq,
				wlc->excursion_queue->low_txq,
				fifo_idx, txq->fifo_state[fifo_idx].buffered));
		}
#endif /* WL_MULTIQUEUE */
	}
}

#ifdef TXQ_MUX
/**
 * @brief Tx path MUX output function
 *
 * Tx path MUX output function. This function is registerd via a call to wlc_mux_add_member() by
 * wlc_txq_alloc(). This function will supply the packets to the caller from packets held
 * in the TxQ's immediate packet path.
 */
static uint BCMFASTPATH
wlc_txq_immediate_output(void *ctx, uint ac, uint request_time, struct spktq *output_q)
{
	wlc_txq_info_t *qi = (wlc_txq_info_t *)ctx;
	struct pktq *q = WLC_GET_TXQ(qi);
	wlc_info_t *wlc = qi->wlc;
	struct scb *scb;
	ratespec_t rspec;
	wlc_txh_info_t txh_info;
	wlc_pkttag_t *pkttag;
	uint16 prec_map;
	int prec;
	void *pkt[DOT11_MAXNUMFRAGS];
	int i, count;
	uint pkttime;
	uint supplied_time = 0;
#ifdef WLAMPDU_MAC
	bool check_epoch = TRUE;
#endif /* WLAMPDU_MAC */

	ASSERT(ac < AC_COUNT);

	/* use a prec_map that matches the AC fifo parameter */
	prec_map = wlc->fifo2prec_map[ac];

	while (supplied_time < request_time && (pkt[0] = pktq_mdeq(q, prec_map, &prec))) {
		pkttag = WLPKTTAG(pkt[0]);
		scb = WLPKTTAGSCBGET(pkt[0]);
		ASSERT(scb != NULL);

		/* Drop the frame if it was targeted to an scb not on the current band */
		/* XXX WES: should we "clean" the immediate queue of pkts if we change
		 * channels so that this could be an ASSERT?
		 */
		if (scb->bandunit != wlc->band->bandunit) {
			PKTFREE(wlc->osh, pkt[0], TRUE);
			WLCNTINCR(wlc->pub->_cnt->txchanrej);
			continue;
		}

		if (pkttag->flags & WLF_MPDU) {
			uint fifo; /* is this param needed anymore ? */

			/*
			 * Drop packets that have been inserted here due to mulitqueue reclaim
			 * Need multiqueue support to push pkts to low_txq instead of qi->q
			 */
			if (pkttag->flags & WLF_TXHDR) {
				PKTFREE(wlc->osh, pkt[0], TRUE);
				WLCNTINCR(wlc->pub->_cnt->txchanrej);
				continue;
			}

			/* fetch the rspec saved in tx_prams at the head of the pkt
			 * before tx_params are removed by wlc_prep_pdu()
			 */
			rspec = wlc_pdu_txparams_rspec(wlc->osh, pkt[0]);

			count = 1;
			/* not looking at error return */
			(void)wlc_prep_pdu(wlc, scb, pkt[0], &fifo);

			wlc_get_txh_info(wlc, pkt[0], &txh_info);

			/* calculate and store the estimated pkt tx time */
			pkttime = wlc_tx_mpdu_time(wlc, scb, rspec, ac, txh_info.d11FrameSize);
			WLPKTTAG(pkt[0])->pktinfo.atf.pkt_time = (uint16)pkttime;
		} else {
			uint fifo; /* is this param needed anymore ? */
			int rc;
			/* WES:XXX Should the error be checked for memory error in the frag case?
			 * wlc_prep_sdu() is currently returning BCME_BUSY when it cannot allocate
			 * frags.
			 */
			rc = wlc_prep_sdu(wlc, scb, pkt, &count, &fifo);
			if (rc) {
				WL_ERROR(("%s(): SCB%s:0x%p q:0x%p "
				"FIFO:%u AC:%u rc:%d prec_map:0x%x len:%u\n",
				__FUNCTION__, SCB_INTERNAL(scb) ?  "(Internal)" : "",
				scb, q, fifo, ac, rc, prec_map, pktq_mlen(q, prec_map)));
			}
			/* the immediate queue should not have SDUs, right? */
			ASSERT(0);

			rspec = wlc_tx_current_ucast_rate(wlc, scb, ac);
			wlc_get_txh_info(wlc, pkt[0], &txh_info);

			/* calculate and store the estimated pkt tx time */
			pkttime = wlc_tx_mpdu_time(wlc, scb, rspec, ac, txh_info.d11FrameSize);
			WLPKTTAG(pkt[0])->pktinfo.atf.pkt_time = (uint16)pkttime;
		}

		supplied_time += pkttime;

		for (i = 0; i < count; i++) {
#ifdef WL11N
			uint16 frameid;

			wlc_get_txh_info(wlc, pkt[i], &txh_info);

#ifdef WLAMPDU_MAC
			/* For AQM AMPDU Aggregation:
			 * If there is a transition from A-MPDU aggregation frames to a
			 * non-aggregation frame, the epoch needs to change. Otherwise the
			 * non-agg frames may get included in an A-MPDU.
			 */
			if (check_epoch && AMPDU_AQM_ENAB(wlc->pub)) {
				/* Once we check the condition, we don't need to check again since
				 * we are enqueuing an non_ampdu frame so wlc_ampdu_was_ampdu() will
				 * be false.
				 */
				check_epoch = FALSE;
				/* if the previous frame in the fifo was an ampdu mpdu,
				 * change the epoch
				 */
				if (wlc_ampdu_was_ampdu(wlc->ampdu_tx, ac)) {
					bool epoch =
					        wlc_ampdu_chgnsav_epoch(wlc->ampdu_tx,
					                                ac,
					                                AMU_EPOCH_CHG_MPDU,
					                                scb,
					                                (uint8)PKTPRIO(pkt[i]),
					                                &txh_info);
					wlc_txh_set_epoch(wlc, txh_info.tsoHdrPtr, epoch);
				}
			}
#endif /* WLAMPDU_MAC */

			frameid = ltoh16(txh_info.TxFrameID);
			if (frameid & TXFID_RATE_PROBE_MASK) {
				wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid, FALSE, 0, ac);
			}
#endif /* WL11N */
			/* add this pkt to the output queue */
			pktenq(output_q, pkt[i]);
		}
	}

#ifdef WLAMPDU_MAC
	/* For Ucode/HW AMPDU Aggregation:
	 * If there are non-aggreation packets added to a fifo, make sure the epoch will
	 * change the next time entries are made to the aggregation info side-channel.
	 * Otherwise, the agg logic may include the non-aggreation packets into an A-AMPDU.
	 */
	if (pktq_len(output_q) > 0 && AMPDU_MAC_ENAB(wlc->pub) && !AMPDU_AQM_ENAB(wlc->pub)) {
		wlc_ampdu_change_epoch(wlc->ampdu_tx, ac, AMU_EPOCH_CHG_MPDU);
	}
#endif /* WLAMPDU_MAC */

	return supplied_time;
}

/**
 * @brief Tx path MUX enqueue function
 *
 * Tx path MUX enqueue function. This funciton is used to send a packet immediately in the
 * context of a TxQ, but not in any SCB flow.
 */
int BCMFASTPATH
wlc_txq_immediate_enqueue(wlc_info_t *wlc, wlc_txq_info_t *qi, void *pkt, uint prec)
{
	struct scb *scb = WLPKTTAGSCBGET(pkt);

	BCM_REFERENCE(scb);
	ASSERT(WLPKTTAG(pkt)->flags & WLF_MPDU);

	if (wlc_prec_enq(wlc, WLC_GET_TXQ(qi), pkt, prec)) {
		/* convert prec to ac fifo number, 4 precs per ac fifo */
		int ac_idx = prec / (WLC_PREC_COUNT / AC_COUNT);
		wlc_mux_source_wake(qi->ac_mux[ac_idx], qi->mux_hdl[ac_idx]);

		/* kick the low txq */
		wlc_send_q(wlc, qi);

		return TRUE;
	}

	return FALSE;
}

/*
 * Rate and time utils
 */

ratespec_t
wlc_tx_current_ucast_rate(wlc_info_t *wlc, struct scb *scb, uint ac)
{
	/* XXX Need to look up the rate for the current AC, but this fn
	 * fills in BE unless a pkt is supplied.
	 */

	return wlc_scb_ratesel_get_primary(wlc, scb, NULL);
}

/* Retrieve the rspec from the tx_params structure at the head of an WLF_MPDU packet */
ratespec_t
wlc_pdu_txparams_rspec(osl_t *osh, void *p)
{
	ratespec_t rspec;
	int8 *rspec_ptr;

	/* pkt should be an MPDU, but not TXHDR, a packet with ucode txhdr
	 * The pkts with WLF_MPDU start with the wlc_pdu_tx_params_t, but pkts
	 * with WLF_TXHDR format flag indicate that PKTDATA starts with a ucode
	 * txhdr.
	 */
	ASSERT((WLPKTTAG(p)->flags & (WLF_MPDU | WLF_TXHDR)) == WLF_MPDU);

	/* copy the rspec from the tx_params
	 * Note that the tx_params address may not be 32bit aligned, so
	 * doing a copy instead of casting the PKTDATA pointer to a tx_params
	 * and doing a direct dereferenc for the rspec
	 */
	rspec_ptr = (int8*)PKTDATA(osh, p) +
		OFFSETOF(wlc_pdu_tx_params_t, rspec_override);

	memcpy(&rspec, rspec_ptr, sizeof(rspec));

	return rspec;
}

uint
wlc_tx_mpdu_frame_seq_overhead(ratespec_t rspec,
	wlc_bsscfg_t *bsscfg, wlcband_t *band, uint ac)
{
	ratespec_t ack_rspec;
	ratespec_t ctl_rspec;
	uint flags = 0;

	ack_rspec = band->basic_rate[RSPEC_REFERENCE_RATE(rspec)];

	ctl_rspec = ack_rspec;

	/*
	 * Short slot for ERP STAs
	 * The AP bsscfg will keep track of all sta's shortslot/longslot cap,
	 * and keep current_bss->capability up to date.
	 */
	if (BAND_5G(band->bandtype) ||
		bsscfg->current_bss->capability & DOT11_CAP_SHORTSLOT)
			flags |= (WLC_AIRTIME_SHORTSLOT);

	return wlc_airtime_pkt_overhead_us(flags, ctl_rspec, ack_rspec,
	                                   bsscfg, wme_fifo2ac[ac]);
}

uint
wlc_tx_mpdu_time(wlc_info_t *wlc, struct scb* scb,
	ratespec_t rspec, uint ac, uint mpdu_len)
{
	wlcband_t *band = wlc_scbband(scb);
	int bandtype = band->bandtype;
	wlc_bsscfg_t *bsscfg;
	uint fixed_overhead_us;
	int short_preamble;
	bool is2g;

	ASSERT(scb->bsscfg);
	bsscfg = scb->bsscfg;

	fixed_overhead_us = wlc_tx_mpdu_frame_seq_overhead(rspec,
		bsscfg, band, ac);

	is2g = BAND_2G(bandtype);

	/* calculate the preample type */
	if (RSPEC_ISLEGACY(rspec)) {
		/* For legacy reates calc the short/long preamble.
		 * Only applicable for 2, 5.5, and 11.
		 * Check the bss config and other overrides.
		 */

		uint mac_rate = (rspec & RATE_MASK);

		if ((mac_rate == WLC_RATE_2M ||
		     mac_rate == WLC_RATE_5M5 ||
		     mac_rate == WLC_RATE_11M) &&
		    WLC_PROT_CFG_SHORTPREAMBLE(wlc->prot, bsscfg) &&
		    (scb->flags & SCB_SHORTPREAMBLE) &&
		    (bsscfg->PLCPHdr_override != WLC_PLCP_LONG)) {
			short_preamble = 1;
		} else {
			short_preamble = 0;
		}
	} else {
		/* For VHT, always MM, for HT, assume MM and
		 * don't bother with Greenfield
		 */
		short_preamble = 0;
	}

	return (fixed_overhead_us +
	        wlc_txtime(rspec, is2g, short_preamble, mpdu_len));
}

ratespec_t
wlc_lowest_scb_rspec(struct scb *scb)
{
	ASSERT(scb != NULL);

	return LEGACY_RSPEC(scb->rateset.rates[0]);
}

ratespec_t
wlc_lowest_band_rspec(wlcband_t *band)
{
	ASSERT(band != NULL);

	return LEGACY_RSPEC(band->hw_rateset.rates[0]);
}

#if defined(TXQ_LOG)
static void
wlc_txq_log_req(txq_info_t *txqi, uint fifo, uint reqtime)
{
	txq_log_entry_t *e;
	uint16 idx = txqi->log_idx;
	uint32 time = 0;

	if (txqi->log_len == 0) {
		return;
	}

#ifdef MACOSX
	{
		clock_usec_t usecs;
		clock_sec_t secs;

		clock_get_system_microtime(&secs, &usecs);
		time = (uint32)(usecs + secs * 1000000);
	}
#elif defined(linux)
	{
		time = (1000000 / HZ) * jiffies;
	}
#elif defined(NDIS)
	{
		LARGE_INTEGER system_time;
		NdisGetCurrentSystemTime(&system_time);
		time = system_time.LowPart;
	}
#elif defined(_RTE_)
	{
		time = 1000 * hnd_time();
	}
#endif /* MACOSX */

	/* get the current entry */
	e = &txqi->log[idx];

	/* save the info */
	e->type = TXQ_LOG_REQ;
	e->u.req.time = time;
	e->u.req.reqtime = (uint32)reqtime;
	e->u.req.fifo = (uint8)fifo;

	txqi->log_idx = (idx + 1) % txqi->log_len;
}

static void
wlc_txq_log_pkts(txq_info_t *txqi, struct spktq* queue)
{
	txq_log_entry_t *e;
	uint16 idx;
	uint16 log_len = txqi->log_len;
	void* p;

	if (log_len == 0) {
		return;
	}

	/* get the current entry */
	idx = txqi->log_idx;
	e = &txqi->log[idx];

	p = queue->q[0].head;
	while (p) {
		/* save the info */
		e->type = TXQ_LOG_PKT;
		e->u.pkt.prio = (uint8)PKTPRIO(p);
		e->u.pkt.bytes = (uint16)pkttotlen(txqi->osh, p);
		e->u.pkt.time = (uint32)WLPKTTIME(p);

		p = PKTLINK(p);
		idx++;
		if (idx == log_len) {
			idx = 0;
			e = txqi->log;
		} else {
			e++;
		}
	}

	txqi->log_idx = idx;
}

static int
wlc_txq_log_dump(void *ctx, struct bcmstrbuf *b)
{
	txq_info_t *txqi = (txq_info_t *)ctx;
	int idx;
	uint16 log_len = txqi->log_len;
	uint16 log_idx = txqi->log_idx;
	txq_log_entry_t *e;
	uint32 base_time = 0;
	uint32 last_time = 0;
	uint pkt_count = 0;
	uint pkt_bytes_tot = 0;
	uint pkt_time_tot = 0;

	idx = log_idx;
	e = &txqi->log[idx];

	do {
		/* summary after a block of pkt samples */
		if (e->type != TXQ_LOG_PKT &&
		    pkt_count > 0) {
			/* only print a summary line if there is more than 1 pkt */
			if (pkt_count > 1) {
				bcm_bprintf(b, "PktTot %u B%u %u us\n",
				            pkt_count, pkt_bytes_tot, pkt_time_tot);
			}

			pkt_count = 0;
			pkt_bytes_tot = 0;
			pkt_time_tot = 0;
		}

		/* individual samples */
		if (e->type == TXQ_LOG_REQ) {
			if (base_time == 0) {
				base_time = e->u.req.time;
				last_time = base_time;
			}

			bcm_bprintf(b, "FillReq %u(+%u) F%u %u us\n",
			            e->u.req.time - base_time,
			            e->u.req.time - last_time,
			            e->u.req.fifo,
			            e->u.req.reqtime);

			last_time = e->u.req.time;
		} else if (e->type == TXQ_LOG_PKT) {
			uint bytes = e->u.pkt.bytes;
			uint time = e->u.pkt.time;

			bcm_bprintf(b, "Pkt P%d B%u %u us\n",
			            wlc_prio2prec_map[e->u.pkt.prio], bytes, time);

			pkt_count++;
			pkt_bytes_tot += bytes;
			pkt_time_tot += time;
		}

		/* advance to the next entry */
		idx++;
		if (idx == log_len) {
			idx = 0;
			e = txqi->log;
		} else {
			e++;
		}

	} while (idx != log_idx);

	return BCME_OK;
}

#endif /* TXQ_LOG */

void BCMFASTPATH
wlc_txq_fill(txq_info_t *txqi, txq_t *txq)
{
	int rerun = FALSE;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	int rerun_count = 0;
#endif // endif
	uint fifo_idx;
	int fill_space;
	int added_time;
	struct spktq fill_q;
	struct swpktq *swq;
	wlc_info_t *wlc = txqi->wlc;
#ifdef WLCNT
	struct txq_fifo_cnt *cnt;
#endif // endif
	if (wlc->in_send_q) {
		rerun = TRUE;
		WL_ERROR(("wl%d: in_send_q, txq=%p\n", wlc->pub->unit, txq));
		return;
	}

	wlc->in_send_q = TRUE;

	/* init the round robin fill_q */
	pktqinit(&fill_q, -1);

	swq = txq->swq;

	/* Loop thru all the fifos and dequeue the packets
	 * BCMC packets will be added to the BCMC fifo if any of the nodes enter PS on the AP.
	 */
	for (fifo_idx = 0; fifo_idx < txq->nswq; fifo_idx++) {
		/* skip fill from above if this fifo is stopped */
		if (!txq_stopped(txq, fifo_idx)) {
#ifdef WLCNT
			cnt = &txq->fifo_cnt[fifo_idx];
#endif // endif
			WLCNTINCR(cnt->q_service);

			/* find out how much room (measured in time) in this fifo of txq */
			fill_space = txq_space(txq, fifo_idx);

			/* Ask feed for pkts to fill available time */
			TXQ_LOG_REQ(txqi, fifo_idx, fill_space);

			/* CPU INTENSIVE */
			added_time = txq->supply(txq->supply_ctx, fifo_idx, fill_space, &fill_q);

			/* skip work if no new data */
			if (added_time > 0) {

				TXQ_LOG_PKTS(txqi, &fill_q);

				/* total accumulation counters update */
				WLCNTADD(cnt->pkts, fill_q.len);
				WLCNTADD(cnt->time, added_time);

				/* track pending packet count per fifo */
				TXPKTPENDINC(wlc, fifo_idx, fill_q.len);
				WL_TRACE(("wl:%s: pktpend inc %d to %d\n", __FUNCTION__,
				          fill_q.len, TXPKTPENDGET(wlc, fifo_idx)));

				/* XXX:TXQ
				 * WES: Need to track the time that will remain *after* the next
				 * txstatus. This just means discounting the pkt at the head of the
				 * hw fifo for single mpdus, but for AMPDU, packet count for tx
				 * status is:
				 * AQM-> variable up to the BA window or epoch bit change
				 * SW -> end of AMPDU
				 */
				/* track added time per fifo */
				txq_inc(txq, fifo_idx, added_time);

				/* append the provided packets to the txq */
				pktq_sw_append(swq, fifo_idx, &fill_q);

				/* check for flow control from new added time */
				if (added_time >= fill_space) {
					txq_stop_set(txq, fifo_idx);
					WLCNTINCR(cnt->q_stops);
				}
			}
		}

		/* post to HW fifo if HW fifo is not stopped and there is data to send */
		if (!txq_hw_stopped(txq, fifo_idx) && pktq_plen(swq, fifo_idx) > 0) {
			txq_hw_fill(txqi, txq, fifo_idx);
		}

		/* if there was a reenterant call to this fn, rerun the loop */
		if (rerun && (fifo_idx == txq->nswq - 1)) {
			rerun = FALSE;
			fifo_idx = (uint)-1; /* set index back so loop will re-run */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
			rerun_count++;
			WL_ERROR(("wl:%s: rerun %d triggered\n", __FUNCTION__, rerun_count));
#endif // endif
		}
	}

	wlc->in_send_q = FALSE;
}

#else
void BCMFASTPATH
wlc_txq_fill(txq_info_t *txqi, txq_t *txq)
{
	uint fifo_idx, nfifo, ac;
	int fill_space;
	int added_time;
	struct swpktq *swq;
	wlc_info_t *wlc = txqi->wlc;
	struct spktq output_q;
#ifdef WLCNT
	struct txq_fifo_cnt *cnt;
#endif // endif

	if (wlc->in_send_q) {
		return;
	}
	wlc->in_send_q = TRUE;

	/* init the round robin output_q */
	pktqinit(&output_q, -1);

	swq = txq->swq;

	for (ac = AC_COUNT; ac > 0; ac--) {
		added_time = txq->supply(txq->supply_ctx, ac-1, -1, &output_q, &fifo_idx);

		if (added_time == 0) {
			continue;
		}
		ASSERT(fifo_idx < WLC_HW_NFIFO_INUSE(wlc));

#ifdef WLCNT
		cnt = &txq->fifo_cnt[fifo_idx];
#endif // endif
		WLCNTINCR(cnt->q_service);

		/* find out how much room (measured in time) in this fifo of txq */
		fill_space = txq_space(txq, fifo_idx);

		/* total accumulation counters update */
		WLCNTADD(cnt->pkts, output_q.len);
		WLCNTADD(cnt->time, added_time);

		/* track weighted pending packet count per fifo */
		TXPKTPENDINC(wlc, fifo_idx, (int16)added_time);
		WL_TRACE(("wl:%s: fifo:%d pktpend inc %d to %d\n", __FUNCTION__,
			fifo_idx, added_time, TXPKTPENDGET(wlc, fifo_idx)));

		/* XXX:TXQ
		 * WES: Need to track the time that will remain *after* the next txstatus.
		 * This just means discounting the pkt at the head of the
		 * hw fifo for single mpdus, but for AMPDU, packet count for tx status is:
		 * AQM-> variable up to the BA window or epoch bit change
		 * SW -> end of AMPDU
		 */
		/* track added time per fifo */
		txq_inc(txq, fifo_idx, added_time);

		/* append the provided packets to the txq */
		pktq_sw_append(swq, fifo_idx, &output_q);

		/* check for flow control from new added time */
		if (added_time >= fill_space) {
			txq_stop_set(txq, fifo_idx);
			WLCNTINCR(cnt->q_stops);
		}
	}

	nfifo = PIO_ENAB(wlc->pub) ? NFIFO : txq->nswq;
	for (fifo_idx = 0; fifo_idx < nfifo; fifo_idx++) {
		txq_hw_fill(txqi, txq, fifo_idx);
	}

	wlc->in_send_q = FALSE;
}
#endif /* TXQ_MUX */

void BCMFASTPATH
wlc_txq_complete(txq_info_t *txqi, txq_t *txq, uint fifo_idx, int complete_pkts, int complete_time)
{
	int remaining_time;

	remaining_time = txq_dec(txq, fifo_idx, complete_time);

	/* XXX:TXQ
	 * WES: Consider making the a hi/low water mark on the dma ring just like the sw.
	 * On a large ring, it could save some work.
	 * On a small ring, we may want to keep every last bit full
	 * at the cost of often hitting the stop condition
	 */
	/* open up the hw fifo as soon as any pkts free */
	txq_hw_stop_clr(txq, fifo_idx);

	/* check for flow control release */
	if (remaining_time <= txq->fifo_state[fifo_idx].lowwater) {
		txq_stop_clr(txq, fifo_idx);
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_txq_dump(txq_info_t *txqi, txq_t *txq, struct bcmstrbuf *b)
{
	uint i;

	if (MQUEUE_ENAB(txqi->wlc->pub)) {
		wlc_info_t *wlc = txqi->wlc;
		const char *name;

		if (wlc->active_queue->low_txq == txq) {
			name = "Active Q";
		} else if (wlc->primary_queue->low_txq == txq) {
			name = "Primary Q";
		} else if (wlc->excursion_queue->low_txq == txq) {
			name = "Excursion Q";
		} else {
			name = "";
		}

		bcm_bprintf(b, "txq %p: %s\n", txq, name);
	} else {
		bcm_bprintf(b, "txq %p:\n", txq);
	}

	for (i = 0; i < txq->nswq; i++) {
		struct txq_fifo_state *f = &txq->fifo_state[i];
		uint qlen = 0;
#ifdef WLCNT
		struct txq_fifo_cnt *cnt = &txq->fifo_cnt[i];
#endif // endif

		bcm_bprintf(b, "fifo %u:\n", i);

		bcm_bprintf(b, "qpkts %u flags 0x%x hw %d lw %d buffered %d\n",
		            qlen, f->flags, f->highwater, f->lowwater, f->buffered);

#ifdef WLCNT
		bcm_bprintf(b,
			"Total: pkts %u time %u Q stop %u service %u HW stop %u service %u\n",
			cnt->pkts, cnt->time, cnt->q_stops,
			cnt->q_service, cnt->hw_stops, cnt->hw_service);
#endif // endif
	}

	return BCME_OK;
}
static int
wlc_txq_module_dump(void *ctx, struct bcmstrbuf *b)
{
	txq_info_t *txqi = (txq_info_t *)ctx;
	txq_t *txq;

	txq = txqi->txq_list;

	while (txq != NULL) {
		wlc_txq_dump(txqi, txq, b);
		txq = txq->next;
	}

	return BCME_OK;
}

#endif /* BCMDBG || BCMDBG_DUMP */

#if defined(TXQ_MUX)
uint BCMFASTPATH
wlc_pull_q(void *ctx, uint fifo_idx, int requested_time, struct spktq *output_q)
{
	wlc_txq_info_t *txq = (wlc_txq_info_t*)ctx;

	return wlc_mux_output(txq->ac_mux[fifo_idx], requested_time, output_q);
}
#else /* TXQ_MUX */
void BCMFASTPATH
wlc_low_txq_enq(txq_info_t *txqi, txq_t *txq, uint fifo_idx, void *pkt, int pkt_time)
{
	wlc_info_t *wlc = txqi->wlc;
#ifdef WLCNT
	struct txq_fifo_cnt *cnt = &txq->fifo_cnt[fifo_idx];
#endif // endif

	ASSERT(fifo_idx < NFIFO);

	/* enqueue the packet to the specified fifo queue */
	pktq_swenq(txq->swq, fifo_idx, pkt);

	/* total accumulation counters update */
	WLCNTADD(cnt->pkts, 1);
	WLCNTADD(cnt->time, pkt_time);

#ifdef TXQ_MUX
	WLPKTTAG(p)->pktinfo.atf.pkt_time = (uint16)pkt_time;
#endif // endif

	/* track pending packet count per fifo */
	TXPKTPENDINC(wlc, fifo_idx, 1);
	WL_TRACE(("wl:%s: pktpend inc 1 to %d\n", __FUNCTION__,
	          TXPKTPENDGET(wlc, fifo_idx)));

	/* track added time per fifo */
	txq_inc(txq, fifo_idx, pkt_time);

	/* check for flow control from new added time */
	if (!txq_stopped(txq, fifo_idx) && txq_space(txq, fifo_idx) <= 0) {
		txq_stop_set(txq, fifo_idx);
		WLCNTINCR(cnt->q_stops);
	}
}

int BCMFASTPATH
wlc_scb_peek_txfifo(wlc_info_t *wlc, struct scb *scb, void *pkt, uint *pfifo)
{
	int bcmerror = BCME_OK;
	uint32 flags;
	uint16 txframeid;
	wlc_bsscfg_t *cfg;
	wlc_pdu_tx_params_t *tx_params;

	ASSERT(pkt);

	/* check if dest fifo is already decided */
	flags = WLPKTTAG(pkt)->flags;
	if (flags & WLF_MPDU) {
		if (flags & WLF_TXHDR) {
			cfg = SCB_BSSCFG(scb);

			ASSERT(cfg != NULL);
			if ((BSSCFG_AP(cfg) || BSSCFG_IBSS(cfg)) &&
				SCB_ISMULTI(scb) && WLC_BCMC_PSMODE(wlc, cfg)) {
				/* The wlc_apps_ps_prep_mpdu will change the txfifo when
				 * scb ISMULTI and PSMODE is true, so get tx fifo from
				 * wlc_scb_txfifo.
				 */
				bcmerror = wlc_scb_txfifo(wlc, scb, pkt, pfifo);
			} else {
				txframeid = wlc_get_txh_frameid(wlc, pkt);
				*pfifo = WLC_TXFID_GET_QUEUE(txframeid);
			}
		}
		else {
			tx_params = (wlc_pdu_tx_params_t *)PKTDATA(wlc->osh, pkt);
			*pfifo = load32_ua(&tx_params->fifo);
		}

		if (!MU_TX_ENAB(wlc) && (*pfifo >= NFIFO)) {
			WL_ERROR(("%s: fifo %u >= NFIFO %u when MUTX is disabled\n", __FUNCTION__,
				*pfifo, NFIFO));
			bcmerror = wlc_scb_txfifo(wlc, scb, pkt, pfifo);
			WL_ERROR(("%s: changed to fifo %u txhdr %s\n", __FUNCTION__, *pfifo,
				(flags & WLF_TXHDR) ? "true":"false"));
			if (flags & WLF_TXHDR) {
				wlc_txh_info_t txh_info;

				cfg = SCB_BSSCFG(scb);
				wlc_get_txh_info(wlc, pkt, &txh_info);
				/* setup frameid, also possibly change seq */
				if (*pfifo == TX_BCMC_FIFO) {
					txframeid = bcmc_fid_generate(wlc, cfg,
						txh_info.TxFrameID);
				} else {
					txframeid = (((wlc->counter++) << TXFID_SEQ_SHIFT) &
						TXFID_SEQ_MASK) | WLC_TXFID_SET_QUEUE(*pfifo);
				}
				txh_info.TxFrameID = htol16(txframeid);
				wlc_set_txh_info(wlc, pkt, &txh_info);
			} else {
				int8 *fifo_ptr = (int8*)PKTDATA(wlc->osh, pkt) +
					OFFSETOF(wlc_pdu_tx_params_t, fifo);
				memcpy(fifo_ptr, pfifo, sizeof(*pfifo));
			}
		}

		return bcmerror;
	}

	bcmerror = wlc_scb_txfifo(wlc, scb, pkt, pfifo);

	return bcmerror;
}

int BCMFASTPATH
wlc_scb_txfifo(wlc_info_t *wlc, struct scb *scb, void *pkt, uint *pfifo)
{
	uint8 prio;
	wlc_bsscfg_t *cfg;

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);
	BCM_REFERENCE(cfg);

	/*
	 * On the AP, bcast/mcast always goes through the bcmc fifo if bcmc_scb is in PS Mode.
	 * bcmc_scb is in PS Mode if at least one ucast STA is in PS.  SDUs are data
	 * packets and always go in BE_FIFO unless WME is enabled.
	 *
	 * Default prio has to be 0 for TKIP MIC calculation when sending packet on a non-WME
	 * link, as the receiving end uses prio of 0 for MIC calculation when QoS header
	 * is absent from the packet.
	 */

	prio = 0;
	if (SCB_QOS(scb)) {
		prio = (uint8)PKTPRIO(pkt);
		ASSERT(prio <= MAXPRIO);
	}

	*pfifo = TX_AC_BE_FIFO;

	if ((BSSCFG_AP(cfg) || BSSCFG_IBSS(cfg)) && SCB_ISMULTI(scb) &&
		WLC_BCMC_PSMODE(wlc, cfg)) {
		*pfifo = TX_BCMC_FIFO;
#ifdef WLBTAMP
	} else if (SCB_11E(scb)) {
		*pfifo = prio2fifo[prio];
#endif /* WLBTAMP */
	} else if (SCB_WME(scb)) {
		*pfifo = prio2fifo[prio];

#ifdef WL_MU_TX
		if (BSSCFG_AP(cfg) && MU_TX_ENAB(wlc)) {
			wlc_mutx_sta_txfifo(wlc->mutx, scb, pfifo);
		}
#endif // endif
	}

	return BCME_OK;
}

uint BCMFASTPATH
wlc_pull_q(void *ctx, uint ac, int requested_time, struct spktq *output_q, uint *fifo_idx)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	void *pkt[DOT11_MAXNUMFRAGS];
	wlc_txq_info_t *qi;
	int prec;
	uint16 prec_map;
	int err, i, count;
	int supplied_time = 0;
	wlc_pkttag_t *pkttag;
	struct scb *scb;
	struct pktq *q;
	wlc_bsscfg_t *cfg;
	osl_t *osh;
	uint fifo = NFIFO_EXT, prev_fifo = NFIFO_EXT;
#ifdef WL_BSSCFG_TX_SUPR
#ifdef PROP_TXSTATUS
	int suppress_to_host = 0;
#endif /* PROP_TXSTATUS */
#endif /* WL_BSSCFG_TX_SUPR */

#ifdef WL_BSSCFG_TX_SUPR
	/* Waiting to drain the FIFO so don't pull any packet */
	/* XXX Add other conditions that want to drain all the FIFOs as
	 * current block_datafifo mechanism blocks only SDUs
	 */
	if (wlc->block_datafifo & DATA_BLOCK_TX_SUPR) {
		WL_INFORM(("wl%d: wlc->block_datafifo & DATA_BLOCK_TX_SUPR\n", wlc->pub->unit));
		return supplied_time;
	}
#endif /* WL_BSSCFG_TX_SUPR */

/* WES: Make sure wlc_send_q is not called when detach pending */
	/* Detaching queue is still pending, don't queue packets to FIFO */
	if (wlc->txfifo_detach_pending) {
		WL_INFORM(("wl%d: wlc->txfifo_detach_pending %d\n",
		           wlc->pub->unit, wlc->txfifo_detach_pending));
		return supplied_time;
	}

	/* only do work for the active queue */
	qi = wlc->active_queue;

	osh = wlc->osh;
	BCM_REFERENCE(osh);

	ASSERT(qi);
	q = WLC_GET_TXQ(qi);

	/* Use a prec_map that matches the AC fifo parameter */
	prec_map = wlc->fifo2prec_map[ac];

	pkt[0] = pktq_mpeek(q, prec_map, &prec);

	if (pkt[0] == NULL) {
		return supplied_time;
	}

	scb = WLPKTTAGSCBGET(pkt[0]);
	ASSERT(scb != NULL);

	wlc_scb_peek_txfifo(wlc, scb, pkt[0], &fifo);
	ASSERT(fifo < WLC_HW_NFIFO_INUSE(wlc));

	if (txq_stopped(qi->low_txq, fifo)) {
		return supplied_time;
	}

	/* find out how much room (measured in time) in this fifo of txq */
	requested_time = txq_space(qi->low_txq, fifo);

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	if (BCMDHDHDR_ENAB()) {
		requested_time = MIN(requested_time, lfbufpool_avail(D11_LFRAG_BUF_POOL));
	}
#endif // endif

	/* Send all the enq'd pkts that we can.
	 * Dequeue packets with precedence with empty HW fifo only
	 */
	while ((supplied_time < requested_time) && prec_map &&
		(pkt[0] = pktq_mdeq(q, prec_map, &prec))) {

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
		if (BCMDHDHDR_ENAB()) {
			ASSERT(lfbufpool_avail(D11_LFRAG_BUF_POOL));
		}
#endif // endif

		/* Send AMPDU using wlc_sendampdu (calls wlc_txfifo() also),
		 * SDU using wlc_prep_sdu and PDU using wlc_prep_pdu followed by
		 * wlc_txfifo() for each of the fragments
		 */
		pkttag = WLPKTTAG(pkt[0]);

		scb = WLPKTTAGSCBGET(pkt[0]);
		ASSERT(scb != NULL);

		cfg = SCB_BSSCFG(scb);
		ASSERT(cfg != NULL);
		BCM_REFERENCE(cfg);

		wlc_scb_peek_txfifo(wlc, scb, pkt[0], &fifo);

		if ((prev_fifo != NFIFO_EXT) && (prev_fifo != fifo)) {
			/* if dest fifo changed in the loop then exit with
			 * prev_fifo; caller of wlc_pull_q cannot handle fifo
			 * changes.
			 */
			pktq_penq_head(q, prec, pkt[0]);
			break;
		}
		ASSERT(fifo < WLC_HW_NFIFO_INUSE(wlc));

		*fifo_idx = fifo;
		prev_fifo = fifo;

		/* XXX:TXQ
		 * WES:BSS absence suppression can change into
		 * stopping the flow from SCB queues that
		 * belong to BSSCFG that are BSS_TX_SUP(). No need to support
		 * wlc_bsscfg_tx_psq_enq() since the pkts are already held on a queue.
		 */
#ifdef WL_BSSCFG_TX_SUPR
		if (BSS_TX_SUPR(cfg)) {

			ASSERT(!(wlc->block_datafifo & DATA_BLOCK_TX_SUPR));
#ifdef PROP_TXSTATUS
			if (PROP_TXSTATUS_ENAB(wlc->pub)) {
				if (suppress_to_host) {
					/*
					XXX: If any packet was suppressed, check rest
					that are in queue
					*/
					if (wlc_suppress_sync_fsm(wlc, scb, pkt[0], FALSE)) {
						wlc_process_wlhdr_txstatus(wlc,
						        WLFC_CTL_PKTFLAG_WLSUPPRESS,
						        pkt[0], FALSE);
						PKTFREE(osh, pkt[0], TRUE);
					}
					else {
						suppress_to_host = 0;
					}
				}
				if (!suppress_to_host) {
					if (wlc_bsscfg_tx_psq_enq(wlc, scb->bsscfg, pkt[0], prec)) {
						suppress_to_host = 1;
						wlc_suppress_sync_fsm(wlc, scb, pkt[0], TRUE);
						/*
						release this packet and move on to the next
						element in q,
						host will resend this later.
						*/
						wlc_process_wlhdr_txstatus(wlc,
						        WLFC_CTL_PKTFLAG_WLSUPPRESS,
						        pkt[0], FALSE);
						PKTFREE(osh, pkt[0], TRUE);
					}
				}
			} else
#endif /* PROP_TXSTATUS */
			if (wlc_bsscfg_tx_psq_enq(wlc, cfg, pkt[0], prec)) {
				WL_ERROR(("%s: FAILED TO ENQUEUE PKT\n", __FUNCTION__));
				PKTFREE(osh, pkt[0], TRUE);
			}
			continue;
		}
#ifdef BCMDBG
		if (cfg->psq != NULL && pktq_len(cfg->psq) > 0) {
			WL_ERROR(("%s: %u SUPR PKTS LEAKED IN PSQ\n",
			          __FUNCTION__, pktq_len(cfg->psq)));
		}
#endif // endif
#endif /* WL_BSSCFG_TX_SUPR */

#ifdef WLAMPDU
		if (WLPKTFLAG_AMPDU(pkttag)) {
			err = wlc_sendampdu(wlc->ampdu_tx,
				qi, pkt, prec, output_q, &supplied_time);
		}
		else
#endif /* WLAMPDU */
		{
			if (pkttag->flags & WLF_MPDU) {
				count = 1;
				err = wlc_prep_pdu(wlc, scb,  pkt[0], &fifo);
			} else {
				err = wlc_prep_sdu(wlc, scb, pkt, &count, &fifo);
			}

			/* transmit if no error */
			if (!err) {

#ifdef WLAMPDU_MAC
				if (fifo < WLC_HW_NFIFO_INUSE(wlc) &&
				    AMPDU_MAC_ENAB(wlc->pub) && !AMPDU_AQM_ENAB(wlc->pub)) {
					wlc_ampdu_change_epoch(wlc->ampdu_tx, fifo,
						AMU_EPOCH_CHG_MPDU);
				}
#endif /* WLAMPDU_MAC */

				for (i = 0; i < count; i++) {
					wlc_txh_info_t txh_info;

					uint16 frameid;

					wlc_get_txh_info(wlc, pkt[i], &txh_info);
					frameid = ltoh16(txh_info.TxFrameID);
#ifdef WLAMPDU_MAC
					if ((i == 0) && fifo < WLC_HW_NFIFO_INUSE(wlc) &&
					    AMPDU_AQM_ENAB(wlc->pub)) {
						if (wlc_ampdu_was_ampdu(wlc->ampdu_tx, fifo)) {
							bool epoch =
							wlc_ampdu_chgnsav_epoch(wlc->ampdu_tx,
							        fifo,
								AMU_EPOCH_CHG_MPDU,
								scb,
								(uint8)PKTPRIO(pkt[i]),
								&txh_info);
							wlc_txh_set_epoch(wlc, txh_info.tsoHdrPtr,
							                  epoch);
						}
					}
#endif /* WLAMPDU_MAC */

#ifdef WL11N
					if (frameid & TXFID_RATE_PROBE_MASK) {
						wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid,
							FALSE, 0, WME_PRIO2AC(PKTPRIO(pkt[i])));
					}
#endif /* WL11N */
					/* fifo is already looked ahead in
					 * wlc_scb_(peek)_txfifo() so frameid
					 * queue != fifo should not happen.
					 * Add a assert check here to catch
					 * any error cases. one such case was
					 * fifo downgrade.
					 */
					if ((WLC_TXFID_GET_QUEUE(frameid)) != fifo) {
						WL_ERROR(("%s %s IDX(%d) FIFO(%d) FID(%d) "
							"mismatch\n",
							__FUNCTION__, (pkttag->flags & WLF_MPDU) ?
							"PDU" :"SDU", *fifo_idx, fifo,
							WLC_TXFID_GET_QUEUE(frameid)));
						ASSERT((WLC_TXFID_GET_QUEUE(frameid)) == fifo);
					}

					/* add this pkt to the output queue */
					pktenq(output_q, pkt[i]);
					/* fake a time of 1 for each pkt */
					supplied_time += 1;
				}
			}
		}

		if (err == BCME_BUSY) {
			PKTDBG_TRACE(osh, pkt[0], PKTLIST_PRECREQ);
			pktq_penq_head(q, prec, pkt[0]);

			/* Remove this prec from the prec map when the pkt at the head
			 * causes a BCME_BUSY. The next lower prec may have pkts that
			 * are not blocked.
			 * When prec_map is zero, the top of the loop will bail out
			 * because it will not dequeue any pkts.
			 */
			prec_map &= ~(1 << prec);
		}
	}

	/* Check if flow control needs to be turned off after sending the packet */
	if (!EDCF_ENAB(wlc->pub) || (wlc->pub->wlfeatureflag & WL_SWFL_FLOWCONTROL)) {
		if (wlc_txflowcontrol_prio_isset(wlc, qi, ALLPRIO) &&
		    (pktq_len(q) < wlc->pub->tunables->datahiwat / 2)) {
			wlc_txflowcontrol(wlc, qi, OFF, ALLPRIO);
		}
	} else if (wlc->pub->_priofc) {
		int prio;

		/* XXX WES: consider keeping a bitmask of prios that were sent above
		 * so that the code below would only check the those prios.
		 * A common case would be only 1 prio (best effort) being sent for
		 * the majority of calls to this fn.
		 */
		for (prio = MAXPRIO; prio >= 0; prio--) {
			if (wlc_txflowcontrol_prio_isset(wlc, qi, prio) &&
			    (pktq_plen(q, wlc_prio2prec_map[prio]) <
			     wlc->pub->tunables->datahiwat/2)) {
				wlc_txflowcontrol(wlc, qi, OFF, prio);
			}
		}
	}

	return supplied_time;
}
#endif /* TXQ_MUX */

/* module entries */
txq_info_t *
BCMATTACHFN(wlc_txq_attach)(wlc_info_t *wlc)
{
	txq_info_t *txqi;
	int err;

	/* Allocate private states struct. */
	if ((txqi = MALLOCZ(wlc->osh, sizeof(txq_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		goto fail;
	}

	txqi->wlc = wlc;
	txqi->pub = wlc->pub;
	txqi->osh = wlc->osh;

	if ((txqi->delq = MALLOC(wlc->osh, sizeof(*(txqi->delq)))) == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		goto fail;
	}
	pktqinit(txqi->delq, PKTQ_LEN_MAX);

	/* Register module entries. */
	err = wlc_module_register(wlc->pub,
	                          NULL /* txq_iovars */,
	                          "txq", txqi,
	                          NULL /* wlc_txq_doiovar */,
	                          wlc_txq_watchdog /* wlc_txq_watchdog */,
	                          NULL,
	                          wlc_txq_down);

	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          txqi->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(txqi->pub, "txq", wlc_txq_module_dump, (void *)txqi);
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef TXQ_LOG
	/* allocate the debugging txq log */
	txqi->log_len = TXQ_LOG_LEN;
	txqi->log = MALLOCZ(txqi->osh, sizeof(*txqi->log) * TXQ_LOG_LEN);
	if (txqi->log == NULL) {
		WL_ERROR(("wl%d: %s: MALLOC failed for TXQ log, malloced %d bytes\n",
		          txqi->pub->unit, __FUNCTION__, MALLOCED(txqi->osh)));
		txqi->log_len = 0;
	}
	/* register the dump fn for the txq log */
	wlc_dump_register(txqi->pub, "txqlog", wlc_txq_log_dump, (void *)txqi);
#endif /* TXQ_LOG */

	return txqi;

fail:
	wlc_txq_detach(txqi);
	return NULL;
}

void
BCMATTACHFN(wlc_txq_detach)(txq_info_t *txqi)
{
	osl_t *osh;
	txq_t *txq, *next;

	if (txqi == NULL)
		return;

	osh = txqi->osh;

	for (txq = txqi->txq_list; txq != NULL; txq = next) {
		next = txq->next;
		wlc_low_txq_free(txqi, txq);
	}

	wlc_module_unregister(txqi->pub, "txq", txqi);

#ifdef TXQ_LOG
	/* free the debugging txq log */
	if (txqi->log != NULL) {
		MFREE(osh, txqi->log, sizeof(*txqi->log) * txqi->log_len);
		txqi->log = NULL;
	}
#endif /* TXQ_LOG */

	if (txqi->delq != NULL) {
		pktqdeinit(txqi->delq);
		MFREE(osh, txqi->delq, sizeof(*(txqi->delq)));
	}

	MFREE(osh, txqi, sizeof(txq_info_t));
}

int
wlc_txq_fc_verify(txq_info_t *txqi, txq_t *txq)
{
	uint fifo_idx;

	for (fifo_idx = 0; fifo_idx < txq->nswq; fifo_idx++) {
		if (txq_stopped(txq, fifo_idx) && TXPKTPENDGET(txqi->wlc, fifo_idx) == 0) {
			WL_ERROR(("%s FIFO %d NOT stopped\n", __FUNCTION__,  fifo_idx));
			return FALSE;
		}
	}

	return TRUE;
}
#endif /* NEW_TXQ */

void
wlc_block_datafifo(wlc_info_t *wlc, uint32 mask, uint32 val)
{
	uint8 block = wlc->block_datafifo;
	uint8 new_block;
#if defined(TXQ_MUX)
	uint8 scb_block_mask;
#endif  /* defined(NEW_TXQ) && defined(NEW_SCB_TXQ) */

	/* the mask should specify some flag bits */
	ASSERT((mask & 0xFF) != 0);
	/* the value should specify only valid flag bits */
	ASSERT((val & 0xFFFFFF00) == 0);
	/* value should only have bits in the mask */
	ASSERT((val & ~mask) == 0);

	new_block = (block & ~mask) | val;

	/* just return if no change */
	if (block == new_block) {
		return;
	}
	wlc->block_datafifo = new_block;

#if defined(TXQ_MUX)
	/* mask for all block_datafifo reasons to stop scb tx */
	scb_block_mask = (DATA_BLOCK_SCAN | DATA_BLOCK_QUIET | DATA_BLOCK_JOIN |
	                  DATA_BLOCK_TXCHAIN | DATA_BLOCK_SPATIAL | DATA_BLOCK_MUTX);

	/* if scbs were blocked, but not now, start scb tx */
	if ((block & scb_block_mask) != 0 &&
	    (new_block & scb_block_mask) == 0) {
		wlc_scbq_global_stop_flag_clear(wlc->tx_scbq, SCBQ_FC_BLOCK_DATA);
#ifdef AP
		wlc_bcmc_global_start_mux_sources(wlc);
#endif // endif
	}
	/* if scbs were not blocked, but are now, stop scb tx */
	if ((block & scb_block_mask) == 0 &&
	    (new_block & scb_block_mask) != 0) {
		wlc_scbq_global_stop_flag_set(wlc->tx_scbq, SCBQ_FC_BLOCK_DATA);
#ifdef AP
		wlc_bcmc_global_stop_mux_sources(wlc);
#endif // endif
	}
#endif  /* defined(NEW_TXQ) && defined(NEW_SCB_TXQ) */

}

/* This function will check if the packet has to be fragmeted or not
 * and based on this for TKIp, pktdfetch will be done
 */

bool
wlc_is_packet_fragmented(wlc_info_t *wlc, struct scb *scb,
		wlc_bsscfg_t *bsscfg, void *lb)
{
	osl_t *osh = wlc->osh;
	int btc_mode;
	uint thresh, pkt_length, bt_thresh = 0;
	uint8 prio = 0;

	if (SCB_QOS(scb))
		prio = (uint8)PKTPRIO((void *)lb);
	thresh = wlc->fragthresh[WME_PRIO2AC(prio)];

	btc_mode = wlc_btc_mode_get(wlc);
	if (IS_BTCX_FULLTDM(btc_mode))
		bt_thresh = wlc_btc_frag_threshold(wlc, scb);
	if (bt_thresh)
		thresh = thresh > bt_thresh ? bt_thresh : thresh;

	pkt_length = pkttotlen(osh, (void *)lb);
	pkt_length -= ETHER_HDR_LEN;
	if (pkt_length < (thresh - (DOT11_A4_HDR_LEN + DOT11_QOS_LEN +
			DOT11_FCS_LEN + ETHER_ADDR_LEN + TKIP_MIC_SIZE))) {
		return (0);
	}

	return (1);
}

#ifdef NEW_TXQ
void BCMFASTPATH
wlc_send_q(wlc_info_t *wlc, wlc_txq_info_t *qi)
{
	/* Don't send the packets while flushing DMA and low tx queues */
	if (!wlc->pub->up || wlc->hw->reinit) {
		return;
	}

	if ((qi == wlc->active_queue) &&
		!wlc->txfifo_detach_pending) {
		wlc_txq_fill(wlc->txqi, qi->low_txq);
	}
}
#else
void BCMFASTPATH
wlc_send_q(wlc_info_t *wlc, wlc_txq_info_t *qi)
{
	void *pkt[DOT11_MAXNUMFRAGS];
	int prec;
	uint16 prec_map;
	int err, i, count;
	uint fifo;
	wlc_pkttag_t *pkttag;
	struct scb *scb;
	struct pktq *q;
	wlc_bsscfg_t *cfg;
	osl_t *osh;
#ifdef WL_BSSCFG_TX_SUPR
	bool retry = TRUE;
#endif // endif

	ASSERT(wlc);
	ASSERT(wlc->pub);
	ASSERT(qi);

#ifdef WL_BSSCFG_TX_SUPR
	/* Waiting to drain the FIFO so don't pull any packet */
	/* XXX Add other conditions that want to drain all the FIFOs as
	 * current block_datafifo mechanism blocks only SDUs
	 */
	if (wlc->block_datafifo & DATA_BLOCK_TX_SUPR) {
		WL_INFORM(("wl%d: wlc->block_datafifo & DATA_BLOCK_TX_SUPR\n", wlc->pub->unit));
		return;
	}
#endif // endif

	/* Detaching queue is still pending, don't queue packets to FIFO */
	if (wlc->txfifo_detach_pending) {
		WL_INFORM(("wl%d: wlc->txfifo_detach_pending %d\n",
		           wlc->pub->unit, wlc->txfifo_detach_pending));
		return;
	}

	/* only do work for the active queue */
	if (qi != wlc->active_queue) {
		WL_INFORM(("wl%d: qi %p != wlc->active_queue %p\n", wlc->pub->unit,
		        qi, wlc->active_queue));
		return;
	}

	if (wlc->in_send_q) {
		WL_INFORM(("wl%d: in_send_q, qi=%p\n", wlc->pub->unit, qi));
		return;
	}

	wlc->in_send_q = TRUE;

	osh = wlc->osh;

	q = WLC_GET_TXQ(qi);

	prec_map = wlc->tx_prec_map;

	WLDURATION_ENTER(wlc, DUR_SENDQ);

	/* Send all the enq'd pkts that we can.
	 * Dequeue packets with precedence with empty HW fifo only
	 */
	while (prec_map && (pkt[0] = pktq_mdeq(q, prec_map, &prec))) {
		/* Send AMPDU using wlc_sendampdu (calls wlc_txfifo() also),
		 * SDU using wlc_prep_sdu and PDU using wlc_prep_pdu followed by
		 * wlc_txfifo() for each of the fragments
		 */
		pkttag = WLPKTTAG(pkt[0]);

		scb = WLPKTTAGSCBGET(pkt[0]);
		ASSERT(scb != NULL);

		cfg = SCB_BSSCFG(scb);
		ASSERT(cfg != NULL);
		BCM_REFERENCE(cfg);

#ifdef WL_BSSCFG_TX_SUPR
		if (BSS_TX_SUPR(cfg) &&
			!(BSSCFG_IS_AIBSS_PS_ENAB(cfg) && (pkttag->flags & WLF_PSDONTQ))) {

			ASSERT(!(wlc->block_datafifo & DATA_BLOCK_TX_SUPR));

#ifdef PROP_TXSTATUS
			if (PROP_TXSTATUS_ENAB(wlc->pub)) {
				retry = wlc_should_retry_suppressed_pkt(wlc, pkt[0],
				                                        TX_STATUS_SUPR_NACK_ABS);
				retry |= SCB_ISMULTI(scb);
				if (retry == FALSE) {
					if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub))
						wlc_suppress_sync_fsm(wlc, scb, pkt[0], TRUE);
					wlc_process_wlhdr_txstatus(wlc,
					                           WLFC_CTL_PKTFLAG_WLSUPPRESS,
					                           pkt[0], FALSE);
					PKTFREE(osh, pkt[0], TRUE);
				}
			} /* if PROP_TXSTATUS_ENAB */
#endif /* PROP_TXSTATUS */
			if (retry && wlc_bsscfg_tx_psq_enq(wlc, cfg, pkt[0], prec)) {
				WL_ERROR(("%s: FAILED TO ENQUEUE PKT\n", __FUNCTION__));
				PKTFREE(osh, pkt[0], TRUE);
			}
			continue;
		}
#endif /* WL_BSSCFG_TX_SUPR */
#if defined(WLAWDL)
		if (BSSCFG_AWDL(wlc, cfg)) {
			/* AMPDU frames are checked at source (txeval) */
#ifdef AWDL_FAMILY
			if (WLPKTTAG(pkt[0])->flags & WLF_AMPDU_MPDU) {
					;
			}
			else
#endif // endif
			if (wlc_awdl_check_xmit_pkt(wlc, scb, pkt[0]) == BCME_TXFAIL) {
				PKTFREE(wlc->pub->osh, pkt[0], TRUE);
				continue;
			}
			/* In case band is changed, change awdl peer's scb->bandunit */
			if (!SCB_ISMULTI(scb) &&
				(scb->bandunit != CHSPEC_WLCBANDUNIT(WLC_BAND_PI_RADIO_CHANSPEC))) {
					wlc_awdl_update_peer_info(wlc, &scb->ea);
			}
		}
#if defined(PROP_TXSTATUS)
		else if (AWDL_ENAB(wlc->pub) && PROP_TXSTATUS_ENAB(wlc->pub)) {
			if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) &&
				wlc_suppress_sync_fsm(wlc, scb, pkt[0], FALSE)) {
				wlc_process_wlhdr_txstatus(wlc,
				                           WLFC_CTL_PKTFLAG_WLSUPPRESS,
				                           pkt[0], FALSE);
				PKTFREE(wlc->pub->osh, pkt[0], TRUE);
				continue;
			}
			/* Non-AWDL traffic must not be sent on the wrong channel */
			if ((WLPKTTAG(pkt[0])->flags & WLF_DATA) &&
			    !AP_ACTIVE(wlc) && wlc->cfg->current_bss &&
			    wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC) !=
			    wf_chspec_ctlchan(wlc->cfg->current_bss->chanspec)) {
			    if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub))
					wlc_suppress_sync_fsm(wlc, scb, pkt[0], TRUE);
				wlc_process_wlhdr_txstatus(wlc,
				                           WLFC_CTL_PKTFLAG_WLSUPPRESS,
				                           pkt[0], FALSE);
				PKTFREE(wlc->pub->osh, pkt[0], TRUE);
				continue;
			}
		}
#endif /* PROP_TXSTATUS */
#endif /* WLAWDL */
#ifdef WLAMPDU
		if (WLPKTFLAG_AMPDU(pkttag)) {
			err = wlc_sendampdu(wlc->ampdu_tx, qi, pkt, prec);
		}
		else
#endif // endif
		{
			if (pkttag->flags & WLF_MPDU) {
				count = 1;
				err = wlc_prep_pdu(wlc, scb, pkt[0], &fifo);
			} else {
				err = wlc_prep_sdu(wlc, scb, pkt, &count, &fifo);
			}

			/* transmit if no error */
			if (!err) {

#ifdef WLAMPDU_MAC
				if (fifo < AC_COUNT &&
				    AMPDU_MAC_ENAB(wlc->pub) && !AMPDU_AQM_ENAB(wlc->pub)) {
					wlc_ampdu_change_epoch(wlc->ampdu_tx, fifo,
						AMU_EPOCH_CHG_MPDU);
				}
#endif // endif

				for (i = 0; i < count; i++) {
					wlc_txh_info_t txh_info;
#ifdef WL11N
					uint16 frameid;
#endif // endif
					wlc_get_txh_info(wlc, pkt[i], &txh_info);
#ifdef WLAMPDU_MAC
					if ((i == 0) && fifo < AC_COUNT &&
					    AMPDU_AQM_ENAB(wlc->pub)) {
						if (wlc_ampdu_was_ampdu(wlc->ampdu_tx, fifo)) {
							bool epoch =
							wlc_ampdu_chgnsav_epoch(wlc->ampdu_tx,
							        fifo,
								AMU_EPOCH_CHG_MPDU,
								WLPKTTAGSCBGET(pkt[i]),
								(uint8)PKTPRIO(pkt[i]),
								&txh_info);
							wlc_txh_set_epoch(wlc, &txh_info, epoch);
						}
					}
#endif // endif

#ifdef WL11N
					frameid = ltoh16(txh_info.TxFrameID);
					if (frameid & TXFID_RATE_PROBE_MASK)
						wlc_scb_ratesel_probe_ready(wlc->wrsi,
							WLPKTTAGSCBGET(pkt[i]),	frameid, FALSE, 0,
							WME_PRIO2AC(PKTPRIO(pkt[i])));
#endif // endif
#if defined(WLAWDL_LATENCY_SUPPORT) && defined(WLAWDL) && defined(MACOSX)
					if (is_awdl && wlc_awdl_latency_dbg(wlc) &&
						pkttag->ori_pkt_buf &&
						(pkttag->flags3 & WLF3_AWDL_TX))
						pkttag->ts[1] = OSL_SYSUPTIME();
#endif /* WLAWDL_LATENCY_SUPPORT && WLAWDL && MACOSX */

					wlc_txfifo(wlc, fifo, pkt[i], &txh_info, TRUE, 1);
				}
			}
		}

		if (err == BCME_BUSY) {
			PKTDBG_TRACE(osh, pkt[0], PKTLIST_PRECREQ);
			pktq_penq_head(q, prec, pkt[0]);
			if (wlc->block_datafifo) {
				/* WAR for PR85951.  If current highest priority hit
				 * block_datafifo, lower-prio non-SDU may be allowed;
				 * fake a prec_map change and allow it to continue.
				 */
				prec_map &= wlc->tx_prec_map;
				prec_map &= ~(1 << prec);
			} else {
				/* If send failed due to any other reason than a change in
				 * HW FIFO condition, quit. Otherwise, read the new prec_map!
				 */
				if (prec_map == wlc->tx_prec_map)
					break;
				prec_map = wlc->tx_prec_map;
			}
		}
	}

	/* Check if flow control needs to be turned off after sending the packet */
	wlc_check_txq_fc(wlc, qi);
	WLDURATION_EXIT(wlc, DUR_SENDQ);
	wlc->in_send_q = FALSE;
}
#endif /* NEW_TXQ */

#ifndef SCANOL
void
wlc_send_active_q(wlc_info_t *wlc)
{
	if (WLC_TXQ_OCCUPIED(wlc)) {
		wlc_send_q(wlc, wlc->active_queue);
	}
}
#endif /* SCANOL */

#if defined(BCMPCIEDEV)
static void
wlc_upd_flr_weight(wlc_info_t *wlc, struct scb* scb, void *p)
{
	uint8 fl;
	ratespec_t cur_rspec = 0;
	uint16 frmbytes = 0;

	if (BCMPCIEDEV_ENAB()) {
		frmbytes = pkttotlen(wlc->osh, p) +
			wlc_scb_dot11hdrsize(scb);
		fl = RAVG_PRIO2FLR(PRIOMAP(wlc), PKTPRIO(p));
		cur_rspec = wlc_ravg_get_scb_cur_rspec(wlc, scb);
		/* adding pktlen into the corresponding moving average buffer */
		RAVG_ADD(TXPKTLEN_RAVG(scb, fl), TXPKTLEN_RBUF(scb, fl),
			frmbytes, RAVG_EXP_PKT);
		/* adding weight into the corresponding moving average buffer */
		wlc_ravg_add_weight(wlc, scb, fl, cur_rspec);
	}
}
#endif /* BCMPCIEDEV */

static int
wlc_wme_wmmac_check_fixup(wlc_info_t *wlc, struct scb *scb, void *sdu)
{
	int bcmerror;
	uint fifo_prio;
	uint fifo_ret;
	uint8 prio = (uint8)PKTPRIO(sdu);

	fifo_ret = fifo_prio = prio2fifo[prio];
	bcmerror = wlc_wme_downgrade_fifo(wlc, &fifo_ret, scb);
	if (bcmerror == BCME_OK) {
		/* Change packet priority if fifo changed */
		if (fifo_prio != fifo_ret) {
			prio = fifo2prio[fifo_ret];
			PKTSETPRIO(sdu, prio);
		}
	}

	return bcmerror;
}

/*
 * Enqueues a packet on txq, then sends as many packets as possible.
 * Packets are enqueued to a maximum depth of WLC_DATAHIWAT*2.
 * tx flow control is turned on at WLC_DATAHIWAT, and off at WLC_DATAHIWAT/2.
 *
 * NOTE: it's important that the txq be able to accept at least
 * one packet after tx flow control is turned on, as this ensures
 * NDIS does not drop a packet.
 *
 * Returns TRUE if packet discarded and freed because txq was full, FALSE otherwise.
 */
bool BCMFASTPATH
wlc_sendpkt(wlc_info_t *wlc, void *sdu, struct wlc_if *wlcif)
{
	struct scb *scb = NULL;
	osl_t *osh;
	struct ether_header *eh;
	struct ether_addr *dst;
	wlc_bsscfg_t *bsscfg;
	struct ether_addr *wds = NULL;
	bool discarded = FALSE;
#ifdef WLTDLS
	struct scb *tdls_scb = NULL;
#endif // endif
	void *pkt, *n;
	int8 bsscfgidx = -1;
	uint32 lifetime = 0;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_INFORM */
#ifdef WL_RELMCAST
	uint8 flag = FALSE;
#endif // endif
	wlc_bss_info_t *current_bss;
#ifdef EXT_STA
	struct ether_header ethdr;
#endif // endif
#if defined(PKTC) || defined(PKTC_TX_DONGLE)
	uint prec, next_fid;
#endif // endif
	uint bandunit;
	wlc_key_info_t scb_key_info;
	wlc_key_info_t bss_key_info;
#ifdef	WLCAC
uint8 ret_cac = 0;
#endif // endif
#if defined(BCMPCIEDEV)
	ratespec_t cur_rspec = 0;
	uint8 fl = 0;
	BCM_REFERENCE(fl);
	BCM_REFERENCE(cur_rspec);
#endif /* BCMPCIEDEV */

	WL_TRACE(("wlc%d: wlc_sendpkt\n", wlc->pub->unit));

	/* sanity */
	ASSERT(sdu != NULL);
	ASSERT(PKTLEN(wlc->osh, sdu) >= ETHER_HDR_LEN);

	if (PKTLEN(wlc->osh, sdu) < ETHER_HDR_LEN) {
		PKTCFREE(wlc->osh, sdu, TRUE);
		return TRUE;
	}

	osh = wlc->osh;

	/* figure out the bsscfg for this packet */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	ASSERT(WLPKTTAG(sdu) != NULL);

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub) && BSSCFG_SAFEMODE(bsscfg)) {
		/* only dst/src addr are filled in ethdr */
		wlc_ethaddr_from_d11hdr((struct dot11_header *)PKTDATA(osh, sdu), &ethdr);
		eh = &ethdr;
	}
	else
#endif // endif
	{

		eh = (struct ether_header*) PKTDATA(osh, sdu);

#ifdef WLBTAMP
		/* Process BT-SIG packets and fixup missing/inconsistent info including wlcif. */
		/* XXX JQL:
		 * When HCI ACL data frame comes in here from the dongle host it doesn't
		 * have the DA therefore fixing it up here.
		 * Also the wlcif is correct when this function is called for packets generated
		 * within wl driver but it is wrong when it is called for packets sent in from
		 * the dongle host therefore fixing it up here too.
		 */
		if (BTA_ENAB(wlc->pub) && (ntoh16(eh->ether_type) < ETHER_TYPE_MIN) &&
		    (PKTLEN(osh, sdu) >= RFC1042_HDR_LEN) &&
		    bcmp(&eh[1], BT_SIG_SNAP_MPROT, DOT11_LLC_SNAP_HDR_LEN - 2) == 0) {
			/* a non-zero return value indicates no further processing is needed! */
			if (BTA_ENAB(wlc->pub) && wlc_bta_send_proc(wlc->bta, sdu, &wlcif))
				goto toss;
			/* update eh again since we may have changed the headers location! */
			eh = (struct ether_header*)PKTDATA(osh, sdu);
		}
#endif /* WLBTAMP */

		/* figure out the bsscfg for this packet */
		bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
		ASSERT(bsscfg != NULL);
	}

	/* fixup wlcif in case it is NULL */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	WL_APSTA_TX(("wl%d: wlc_sendpkt() pkt %p, len %u, wlcif %p type %d\n",
	             wlc->pub->unit, sdu, pkttotlen(osh, sdu), wlcif, wlcif->type));

	/* QoS map set */
	if (WL11U_ENAB(wlc)) {
		wlc_11u_set_pkt_prio(wlc->m11u, bsscfg, sdu);
	}

#ifdef WET
	/* Apply WET only on the STA interface */
	if (wlc->wet && BSSCFG_STA(bsscfg) &&
	    wlc_wet_send_proc(wlc->weth, sdu, &sdu) < 0)
		goto toss;
#endif /* WET */

	if (wlcif->type == WLC_IFTYPE_WDS) {
		scb = wlcif->u.scb;
		wds = &scb->ea;
	}

	/* discard if we're not up or not yet part of a BSS */
	if (!wlc->pub->up ||
	(!wds && (!bsscfg->up || (BSSCFG_STA(bsscfg) && ETHER_ISNULLADDR(&bsscfg->BSSID))))) {
		WL_INFORM(("wl%d: %s: bsscfg %p is not up\n", wlc->pub->unit,
			__FUNCTION__, bsscfg));
		WLCNTINCR(wlc->pub->_cnt->txnoassoc);
		goto toss;
	}

#ifdef L2_FILTER
	if (L2_FILTER_ENAB(wlc->pub)) {
		if (wlc_l2_filter_send_data_frame(wlc, bsscfg, sdu) == 0)
			goto toss;
	}
#endif /* L2_FILTER */

#ifdef WL_MCAST_FILTER_NOSTA
	if (ETHER_ISMULTI(eh->ether_dhost) && bsscfg->mcast_filter_nosta && !wds &&
			(wlc_bss_assocscb_getcnt(wlc, bsscfg) == 0)) {
		goto toss;
	}
#endif // endif

	if (!BSSCFG_SAFEMODE(bsscfg)) {
#ifdef WMF
		/* Do the WMF processing for multicast packets */
		if (!wds && WMF_ENAB(bsscfg) && (ETHER_ISMULTI(eh->ether_dhost) ||
			(bsscfg->wmf_ucast_igmp && is_igmp(eh)))) {
			/* We only need to process packets coming from the stack/ds */
			switch (wlc_wmf_packets_handle(bsscfg, NULL, sdu, 0)) {
			case WMF_TAKEN:
				/* The packet is taken by WMF return */
				return (FALSE);
			case WMF_DROP:
				/* Packet DROP decision by WMF. Toss it */
				goto toss;
			default:
				/* Continue the transmit path */
				break;
			}
		}
#endif /* WMF */

#ifdef MAC_SPOOF
		/* in MAC Clone/Spoof mode, change our MAC address to be that of the original
		 * sender of the packet.  This will allow full layer2 bridging for that client.
		 * Note:  This is to be used in STA mode, and is not to be used with WET.
		 */
		if (wlc->mac_spoof &&
		    bcmp(&eh->ether_shost, &wlc->pub->cur_etheraddr, ETHER_ADDR_LEN)) {
			if (wlc->wet)
				WL_ERROR(("Configuration Error,"
					"MAC spoofing not supported in WET mode"));
			else {
				bcopy(&eh->ether_shost, &wlc->pub->cur_etheraddr, ETHER_ADDR_LEN);
				bcopy(&eh->ether_shost, &bsscfg->cur_etheraddr, ETHER_ADDR_LEN);
				WL_INFORM(("%s:  Setting WL device MAC address to %s",
					__FUNCTION__,
					bcm_ether_ntoa(&bsscfg->cur_etheraddr, eabuf)));
				wlc_set_mac(bsscfg);
			}
		}
#endif /* MAC_SPOOF */
	}

#ifdef WLWNM_AP
	/* Do the WNM processing */
	/* if packet was handled by DMS/FMS already, bypass it */
	if (BSSCFG_AP(bsscfg) && WLWNM_ENAB(wlc->pub) && WLPKTTAGSCBGET(sdu) == NULL) {
		/* try to process wnm related functions before sending packet */
		int ret = wlc_wnm_packets_handle(bsscfg, sdu, 1);
		switch (ret) {
		case WNM_TAKEN:
			/* The packet is taken by WNM return */
			return FALSE;
		case WNM_DROP:
			/* The packet drop decision by WNM free and return */
			goto toss;
		default:
			/* Continue the forwarding path */
			break;
		}
	}
#endif /* WLWNM_AP */

#ifdef WET_TUNNEL
	if (BSSCFG_AP(bsscfg) && WET_TUNNEL_ENAB(wlc->pub)) {
		wlc_wet_tunnel_send_proc(wlc->wetth, sdu);
	}
#endif /* WET_TUNNEL */

#ifdef PSTA
	if (PSTA_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
		/* If a connection for the STAs or the wired clients
		 * doesn't exist create it. If connection already
		 * exists send the frames on it.
		 */
		if (wlc_psta_send_proc(wlc->psta, &sdu, &bsscfg) != BCME_OK) {
			WL_INFORM(("wl%d: tossing frame\n", wlc->pub->unit));
			goto toss;
		}
	}
#endif /* PSTA */

	if (wds)
		dst = wds;
	else if (BSSCFG_AP(bsscfg)) {
#ifdef WLWNM_AP
		/* Do the WNM processing */
		if (WLWNM_ENAB(wlc->pub) &&
		    wlc_wnm_dms_amsdu_on(wlc, bsscfg) &&
		    WLPKTTAGSCBGET(sdu) != NULL) {
			dst = &(WLPKTTAGSCBGET(sdu)->ea);
		}
		else
#endif /* WLWNM_AP */
		dst = (struct ether_addr*)eh->ether_dhost;
	}
	else {
		dst = bsscfg->BSS ? &bsscfg->BSSID : (struct ether_addr*)eh->ether_dhost;
#ifdef WL_RELMCAST
		/* increment rmc tx frame counter only when RMC is enabled */
		if (RMC_ENAB(wlc->pub) && ETHER_ISMULTI(dst))
			flag = TRUE;
#endif // endif
#ifdef WLTDLS
		if (TDLS_ENAB(wlc->pub)) {
			tdls_scb = wlc_tdls_query(wlc->tdls, bsscfg,
				sdu, (struct ether_addr*)eh->ether_dhost);
			if (tdls_scb) {
				dst = (struct ether_addr*)eh->ether_dhost;
				bsscfg = tdls_scb->bsscfg;
				WL_TMP(("wl%d:%s(): to dst %s, use TDLS, bsscfg=0x%p\n",
					wlc->pub->unit, __FUNCTION__,
					bcm_ether_ntoa(dst, eabuf), bsscfg));
				ASSERT(bsscfg != NULL);
			}
		}
#endif // endif

	}

	current_bss = bsscfg->current_bss;

	bandunit = CHSPEC_WLCBANDUNIT(current_bss->chanspec);

	/* check IAPP L2 update frame */
	if (!wds && BSSCFG_AP(bsscfg) && ETHER_ISMULTI(dst)) {

		if ((ntoh16(eh->ether_type) == ETHER_TYPE_IAPP_L2_UPDATE) &&
			((WLPKTTAG(sdu)->flags & WLF_HOST_PKT) ||
#ifdef PROP_TXSTATUS
			(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(sdu)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST) ||
#endif /* PROP_TXSTATUS */
			0)) {
			struct ether_addr *src;

			src = (struct ether_addr*)eh->ether_shost;

			/* cleanup the scb */
			if ((scb = wlc_scbfindband(wlc, bsscfg, src, bandunit)) != NULL) {
				WL_INFORM(("wl%d: %s: non-associated station %s\n", wlc->pub->unit,
					__FUNCTION__, bcm_ether_ntoa(src, eabuf)));
				wlc_bss_mac_event(wlc, bsscfg, WLC_E_DISASSOC_IND, &scb->ea,
					WLC_E_STATUS_SUCCESS, DOT11_RC_DISASSOC_LEAVING, 0, 0, 0);
				wlc_scbfree(wlc, scb);
			}
		}
	}
	/* Done with WLF_HOST_PKT flag; clear it now. This flag should not be
	 * used beyond this point as it is overloaded on WLF_FIFOPKT. It is
	 * set when pkt leaves the per port layer indicating it is coming from
	 * host or bridge.
	 */
	WLPKTTAG(sdu)->flags &= ~WLF_HOST_PKT;

	/* toss if station is not associated to the correct bsscfg. Make sure to use
	 * the band while looking up as we could be scanning on different band
	 */
	/* Class 3 (BSS) frame */
	if (!wds && bsscfg->BSS && !ETHER_ISMULTI(dst)) {
		if ((scb = wlc_scbfindband(wlc, bsscfg, dst, bandunit)) == NULL) {
			WL_INFORM(("wl%d: %s: invalid class 3 frame to "
				"non-associated station %s\n", wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(dst, eabuf)));
			WLCNTINCR(wlc->pub->_cnt->txnoassoc);
			goto toss;
		}
	}
	/* Class 1 (IBSS/TDLS) or 4 (WDS) frame */
	else {
#if defined(WLTDLS)
		if ((TDLS_SUPPORT(wlc->pub)) && (tdls_scb != NULL))
			scb = tdls_scb;
		else
#endif /* defined(WLTDLS) */
#ifdef WLAWDL
		if (bsscfg && BSSCFG_AWDL(wlc, bsscfg)) {
			if (!AWDL_ENAB(wlc->pub))
				goto toss;
			if ((!wlc_awdl_tx_outside_aw_allowed(wlc) && !wlc_awdl_in_aw(wlc)) ||
				!wlc_awdl_valid_channel(wlc) || wlc_awdl_in_flow_ctrl(wlc)) {
				wlc_incr_awdl_counter(wlc, AWDL_DATA_TX_SUPR);
				if (ETHER_ISMULTI(dst))
					scb = WLC_BCMCSCB_GET(wlc, bsscfg);
				else
					scb = wlc_scbfind_dualband(wlc, bsscfg, dst);
				if (!scb)
					goto toss;
#ifdef PROP_TXSTATUS
				if (PROP_TXSTATUS_ENAB(wlc->pub)) {
					if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) &&
						!wlc_suppress_sync_fsm(wlc, scb, sdu, FALSE))
						wlc_suppress_sync_fsm(wlc, scb, sdu, TRUE);
					wlc_process_wlhdr_txstatus(wlc, WLFC_CTL_PKTFLAG_WLSUPPRESS,
						sdu, FALSE);
				}
#endif /* PROP_TXSTATUS */
				FOREACH_CHAINED_PKT(sdu, n) {
					PKTCLRCHAINED(osh, sdu);
					PKTFREE(wlc->pub->osh, sdu, TRUE);
				}
#ifdef MACOSX
				return TRUE;
#else
				return FALSE;
#endif // endif
					}
					if (!ETHER_ISMULTI(dst)) {
						if (!(scb = wlc_awdl_update_peer_info(wlc, dst))) {
							WLCNTINCR(wlc->pub->_cnt->txnoassoc);
							goto toss;
						}
					}
					wlc_incr_awdl_counter(wlc, AWDL_DATA_TX);
				}
#endif /* WLAWDL */

		if (ETHER_ISMULTI(dst)) {
			scb = WLC_BCMCSCB_GET(wlc, bsscfg);
			if (scb == NULL) {
				WL_ERROR(("wl%d: %s: invalid multicast frame\n",
				          wlc->pub->unit, __FUNCTION__));
				WLCNTINCR(wlc->pub->_cnt->txnoassoc);
				goto toss;
			}
		}
		else if (scb == NULL &&
		         (scb = wlc_scblookupband(wlc, bsscfg, dst, bandunit)) == NULL) {
			WL_ERROR(("wl%d: %s: out of scbs\n", wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
			/* Increment interface stats */
			if (wlcif) {
				WLCNTINCR(wlcif->_cnt.txnobuf);
			}
			goto toss;
		}
#ifdef WLBTAMP
		else if (BSS_BTA_ENAB(wlc, bsscfg) &&
		         scb != NULL &&
		         (SCB_BSSCFG(scb) != bsscfg || !SCB_ASSOCIATED(scb))) {
			WL_INFORM(("wl%d: %s: invalid class 4 frame to "
			           "non-associated BT-AMP peer %s\n", wlc->pub->unit, __FUNCTION__,
			           bcm_ether_ntoa(dst, eabuf)));
			WLCNTINCR(wlc->pub->_cnt->txnoassoc);
			goto toss;
		}
#endif // endif
	}

	/* per-port code must keep track of WDS cookies */
	ASSERT(!wds || SCB_WDS(scb));

	if (SCB_LEGACY_WDS(scb)) {
		/* Discard frame if wds link is down */
		if (!(scb->flags & SCB_WDS_LINKUP)) {
			WLCNTINCR(wlc->pub->_cnt->txnoassoc);
			goto toss;
		}

		/* Discard 802_1X frame if it belongs to another WDS interface
		 * Frame might be received here due to bridge flooding.
		 */
		if (eh->ether_type == hton16(ETHER_TYPE_802_1X)) {
			struct scb *scb_da = NULL;
			wlc_bsscfg_t *cfg;
			scb_da = wlc_scbapfind(wlc, (struct ether_addr *)(&eh->ether_dhost), &cfg);

			if (scb_da && (scb != scb_da)) {
				goto toss;
			}
		}
	}

#ifdef DWDS
	/* For MultiAP, Maintaining a list of clients attached to the DWDS STA.
	 * Using this mac address table to avoid handling BCMC packets
	 * that get looped back from rootAP.
	 */
	if (MAP_ENAB(bsscfg) && BSSCFG_STA(bsscfg) && bsscfg->dwds_loopback_filter &&
		SCB_DWDS(scb) && ETHER_ISMULTI(eh->ether_dhost)) {
		uint8 *sa = eh->ether_shost;
		if (wlc_dwds_findsa(wlc, bsscfg, sa) == NULL) {
			wlc_dwds_addsa(wlc, bsscfg, sa);
		}
	}
#endif /* DWDS */

#if defined(WLTDLS)
	if (TDLS_SUPPORT(wlc->pub))
		ASSERT(!tdls_scb || BSSCFG_IS_TDLS(bsscfg));
#endif /* defined(WLTDLS) */

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
#ifdef WLTDLS
		bool tdls_action = FALSE;
		if (eh->ether_type == hton16(ETHER_TYPE_89_0D))
			tdls_action = TRUE;
#endif // endif
#ifdef WLMCHAN
		if (MCHAN_ACTIVE(wlc->pub)) {
			/* Free up packets to non active queue */
			if (wlc->primary_queue) {
				wlc_bsscfg_t *other_cfg = wlc_mchan_get_other_cfg_frm_q(wlc,
					wlc->primary_queue);
				if ((other_cfg == scb->bsscfg) && !SCB_ISMULTI(scb)) {
					FOREACH_CHAINED_PKT(sdu, n) {
						PKTCLRCHAINED(osh, sdu);
						if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub))
							wlc_suppress_sync_fsm(wlc, scb, sdu, TRUE);
						wlc_process_wlhdr_txstatus(wlc,
							WLFC_CTL_PKTFLAG_D11SUPPRESS, sdu, FALSE);
						PKTFREE(wlc->pub->osh, sdu, TRUE);
					}
					return FALSE;
				}
			}
		}
#endif /* WLMCHAN */
		if (!SCB_ISMULTI(scb) &&
#ifdef WLTDLS
			!tdls_action &&
#endif // endif
		1) {
			void *head = NULL, *tail = NULL;
			void *pktc_head = sdu;
			uint32 pktc_head_flags = WLPKTTAG(sdu)->flags;

			FOREACH_CHAINED_PKT(sdu, n) {
				if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) &&
					wlc_suppress_sync_fsm(wlc, scb, sdu, FALSE)) {
					PKTCLRCHAINED(osh, sdu);
					/* wlc_suppress? */
					wlc_process_wlhdr_txstatus(wlc, WLFC_CTL_PKTFLAG_WLSUPPRESS,
						sdu, FALSE);
					PKTFREE(wlc->pub->osh, sdu, TRUE);
				} else {
					PKTCENQTAIL(head, tail, sdu);
				}
			}

			/* return if all packets are suppressed */
			if (head == NULL)
				return FALSE;
			sdu = head;

			/* XXX: head packet in the chain was suppressed. So, need to copy
				saved flags from the dropped head pkt to new head pkt.
			*/
			if (pktc_head != sdu)
				WLPKTTAG(sdu)->flags = pktc_head_flags;
		}
	}
#endif /* PROP_TXSTATUS */

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub) && BSSCFG_SAFEMODE(bsscfg))
		pkt = wlc_hdr_proc_safemode(wlc, sdu);
	else
#endif // endif
		pkt = wlc_hdr_proc(wlc, sdu, scb);

	/* Header conversion failed */
	if (pkt == NULL)
		goto toss;

	sdu = pkt;
#ifdef EVENT_LOG_COMPILE /* ifdef prevents ROM abandoning */
	if (WLPKTTAG(pkt)->flags & WLF_8021X)
		WL_EVENT_LOG(EVENT_LOG_TAG_TRACE_WL_INFO, TRACE_FW_EAPOL_FRAME_TRANSMIT_START);
#endif /* EVENT_LOG_COMPILE */
	/* early discard of non-8021x frames if keys are not plumbed */
	wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
		WLC_KEY_FLAG_NONE, &scb_key_info);
	wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE, &bss_key_info);
	if (!BSSCFG_SAFEMODE(bsscfg) &&
	    (WSEC_ENABLED(bsscfg->wsec) && !WSEC_SES_OW_ENABLED(bsscfg->wsec)) &&
	    !(WLPKTTAG(pkt)->flags & WLF_8021X) &&
#ifdef BCMWAPI_WAI
	    !(WLPKTTAG(pkt)->flags & WLF_WAI) &&
#endif // endif
		(scb_key_info.algo == CRYPTO_ALGO_OFF) &&
#ifdef EXT_STA
		/* pass non-8021x tx frame if frame has exemption */
		!(WLEXTSTA_ENAB(wlc->pub) && BSSCFG_HAS_NATIVEIF(bsscfg) &&
		(WLPKTFLAG_EXEMPT_GET(WLPKTTAG(pkt)) == WSEC_EXEMPT_ALWAYS ||
		(WLPKTFLAG_EXEMPT_GET(WLPKTTAG(pkt)) == WSEC_EXEMPT_NO_PAIRWISE &&
		!ETHER_ISMULTI(dst)))) &&
#endif /* EXT_STA */
		(bss_key_info.algo == CRYPTO_ALGO_OFF)) {
		WL_WSEC(("wl%d: %s: tossing unencryptable frame, flags=0x%x\n",
			wlc->pub->unit, __FUNCTION__, WLPKTTAG(pkt)->flags));
		goto toss;
	}

	bsscfgidx = WLC_BSSCFG_IDX(bsscfg);
	ASSERT(BSSCFGIDX_ISVALID(bsscfgidx));
	WLPKTTAGBSSCFGSET(sdu, bsscfgidx);
	WLPKTTAGSCBSET(sdu, scb);

	/* wlc_wme_wmmac_check_fixup is called by wlc_ampdu_agg() and wlc_scb_txfifo().
	 * Now we do it here, the top level fn wlc_sendpkt().
	 * This way we're done for good no matter how many forks
	 * (AMPDU, AMPDU+PKTC, non-AMPDU, etc.).
	 * Becasue ap mode didn't implement the TSPEC now,
	 * so ap mode skip WMMAC downgrade check.
	 */
	if (SCB_WME(scb) && BSSCFG_STA(bsscfg)) {
		if (wlc_wme_wmmac_check_fixup(wlc, scb, sdu) != BCME_OK) {
			/* let the main prep code drop the frame and account it properly */
			WL_CAC(("%s: Pkt dropped. WMMAC downgrade check failed.\n", __FUNCTION__));
			goto toss;
		}
	}

#ifdef	WLCAC
	if (CAC_ENAB(wlc->pub)) {

		ret_cac = wlc_cac_is_traffic_admitted(wlc->cac, WME_PRIO2AC(PKTPRIO(sdu)), scb);
		if ((ret_cac == WLC_CAC_NOT_ADMITTED) || (ret_cac == WLC_CAC_ALLOWED_TXOP_ISOVER)) {
			WL_CAC(("%s: Pkt dropped. Admission not granted for ac %d pktprio %d\n",
				__FUNCTION__, WME_PRIO2AC(PKTPRIO(sdu)), PKTPRIO(sdu)));
			goto toss;
		}

		if (BSSCFG_AP(bsscfg) && !SCB_ISMULTI(scb)) {
			wlc_cac_reset_inactivity_interval(wlc->cac, WME_PRIO2AC(PKTPRIO(sdu)), scb);
		}
#ifdef BCMCCX
		if (current_bss->ccx_version >= 4)
			wlc_ccx_tsm_pktcnt(wlc->cac, WME_PRIO2AC(PKTPRIO(sdu)), scb);
#endif /* BCMCCX */
	}
#endif /* WLCAC */

	/* Set packet lifetime if configured */
	if (!(WLPKTTAG(sdu)->flags & WLF_EXPTIME) &&
	    (lifetime = wlc->lifetime[(SCB_WME(scb) ? WME_PRIO2AC(PKTPRIO(sdu)) : AC_BE)]))
		wlc_lifetime_set(wlc, sdu, lifetime);

	WLPKTTAG(sdu)->flags |= WLF_DATA;
#ifdef WLAWDL
	if (BSSCFG_AWDL(wlc, bsscfg))
		wlc_awdl_pkt_inc(wlc->awdl_info, scb, FALSE, ETHER_ISMULTI(dst)? TRUE : FALSE);
#endif /* WLAWDL */
#ifdef STA
	/* Restart the Dynamic Fast Return To Sleep sleep return timer if needed */
	if (bsscfg->pm->dfrts_logic != WL_DFRTS_LOGIC_OFF)
		wlc_update_sleep_ret(bsscfg, FALSE, TRUE, 0, pkttotlen(wlc->osh, sdu));
#ifdef WL11K
	if (WL11K_ENAB(wlc->pub)) {
		wlc_rrm_upd_data_activity_ts(wlc->rrm_info);
	}
#endif /* WL11K */
#endif /* STA */

#if defined(AP) && defined(TXQ_MUX)
if (SCB_ISMULTI(scb))
{
		/* BCMC enqueue function path calls wlc_send_q()  */
		wlc_bcmc_enqueue(wlc, bsscfg, sdu, WLC_PRIO_TO_PREC(PKTPRIO(sdu)));
} else {
#endif /* defined(AP) && defined(TXQ_MUX) */

	if (BSSCFG_IBSS(bsscfg) && AIBSS_ENAB(wlc->pub)) {
		/* Enable packet callback */
		WLF2_PCB1_REG(sdu, WLF2_PCB1_AIBSS_DATA);
	}

#if defined(PKTC) || defined(PKTC_TX_DONGLE)
	prec = WLC_PRIO_TO_PREC(PKTPRIO(sdu));
	next_fid = SCB_TXMOD_NEXT_FID(scb, TXMOD_START);

#ifdef WLAMSDU_TX
	if (AMSDU_TX_ENAB(wlc->pub) &&
			PKTC_ENAB(wlc->pub) && (next_fid == TXMOD_AMSDU) &&
			(SCB_TXMOD_NEXT_FID(scb, TXMOD_AMSDU) == TXMOD_AMPDU)) {
		/* When chaining and amsdu tx are enabled try doing AMSDU agg
		 * while queing the frames to per scb queues.
		 */
		WL_TRACE(("%s: skipping amsdu mod for %p chained %d\n",
		          __FUNCTION__, sdu, PKTISCHAINED(sdu)));
		SCB_TX_NEXT(TXMOD_AMSDU, scb, sdu, prec);
	} else
#endif // endif
	if ((next_fid == TXMOD_AMPDU) || (next_fid == TXMOD_NAR)) {
		WL_TRACE(("%s: ampdu mod for sdu %p chained %d\n",
		          __FUNCTION__, sdu, PKTISCHAINED(sdu)));

#if defined(BCMPCIEDEV)
		if (BCMPCIEDEV_ENAB()) {
			wlc_upd_flr_weight(wlc, scb, sdu);
		}
#endif /* BCMPCIEDEV */
		SCB_TX_NEXT(TXMOD_START, scb, sdu, prec);
	} else {
#if defined(BCMPCIEDEV)
		if (BCMPCIEDEV_ENAB()) {
			fl = RAVG_PRIO2FLR(PRIOMAP(wlc), PKTPRIO(sdu));
			cur_rspec = wlc_ravg_get_scb_cur_rspec(wlc, scb);
		}
#endif /* BCMPCIEDEV */

		/* Modules other than ampdu and NAR are not aware of chaining */
		FOREACH_CHAINED_PKT(sdu, n) {
			PKTCLRCHAINED(osh, sdu);
			if (n != NULL)
				wlc_pktc_sdu_prep(wlc, scb, sdu, n, lifetime);
#if defined(BCMPCIEDEV)
			if (BCMPCIEDEV_ENAB()) {
				uint16 frmbytes = pkttotlen(wlc->osh, sdu) +
					wlc_scb_dot11hdrsize(scb);
				/* adding pktlen into the moving average buffer */
				RAVG_ADD(TXPKTLEN_RAVG(scb, fl), TXPKTLEN_RBUF(scb, fl),
					frmbytes, RAVG_EXP_PKT);
			}
#endif /* BCMPCIEDEV */
			SCB_TX_NEXT(TXMOD_START, scb, sdu, prec);
		}

#if defined(BCMPCIEDEV)
		if (BCMPCIEDEV_ENAB())
			/* adding weight into the moving average buffer */
			wlc_ravg_add_weight(wlc, scb, fl, cur_rspec);
#endif /* BCMPCIEDEV */
	}
#else /* PKTC */
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		wlc_upd_flr_weight(wlc, scb, sdu);
	}
#endif /* BCMPCIEDEV */
	SCB_TX_NEXT(TXMOD_START, scb, sdu, WLC_PRIO_TO_PREC(PKTPRIO(sdu)));
#endif /* PKTC */
#if defined(AP) && defined(TXQ_MUX)
		wlc_send_q(wlc, wlcif->qi);
}
#endif /* defined(AP) && defined(TXQ_MUX) */

#ifdef WL_RELMCAST
	if (RMC_ENAB(wlc->pub) && flag) {
		wlc_rmc_tx_frame_inc(wlc);
	}
#endif // endif
#ifndef TXQ_MUX
	wlc_send_q(wlc, wlcif->qi);
#endif // endif

#ifdef WL_EXCESS_PMWAKE
	wlc->excess_pmwake->ca_txrxpkts++;
#endif /* WL_EXCESS_PMWAKE */
	return (FALSE);

toss:
	FOREACH_CHAINED_PKT(sdu, n) {
		PKTCLRCHAINED(osh, sdu);

		/* Increment wme stats */
		if (WME_ENAB(wlc->pub)) {
			WLCNTINCR(wlc->pub->_wme_cnt->tx_failed[WME_PRIO2AC(PKTPRIO(sdu))].packets);
			WLCNTADD(wlc->pub->_wme_cnt->tx_failed[WME_PRIO2AC(PKTPRIO(sdu))].bytes,
			         pkttotlen(osh, sdu));
		}

#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			wlc_process_wlhdr_txstatus(wlc, WLFC_CTL_PKTFLAG_TOSSED_BYWLC, sdu, FALSE);
			WLFC_COUNTER_TXSTATUS_WLCTOSS(wlc);
		}
#endif /* PROP_TXSTATUS */

		WL_APSTA_TX(("wl%d: %s: tossing pkt %p\n", wlc->pub->unit, __FUNCTION__, sdu));
#if defined(MACOSX) && defined(WLAWDL)
		if (WLPKTTAG(sdu)->flags3 & WLF3_AWDL_TX) {
			if (!WLPKTTAGSCBGET(sdu))
				discarded = TRUE;
		}
#endif // endif
		PKTFREE(osh, sdu, TRUE);
	}
	return (discarded);
}

#if defined(MBSS) && !defined(TXQ_MUX)
/*
 * Return true if packet got enqueued in BCMC PS packet queue.
 * This happens when the BSS is in transition from ON to OFF.
 * Called in prep_pdu and prep_sdu.
 */
static bool
bcmc_pkt_q_check(wlc_info_t *wlc, struct scb *bcmc_scb, wlc_pkt_t pkt)
{
	if (!MBSS_ENAB(wlc->pub) || !SCB_PS(bcmc_scb) ||
		!(bcmc_scb->bsscfg->flags & WLC_BSSCFG_PS_OFF_TRANS)) {
		/* No need to enqueue pkt to PS queue */
		return FALSE;
	}

	/* BSS is in PS transition from ON to OFF; Enqueue frame on SCB's PSQ */
	if (wlc_apps_bcmc_ps_enqueue(wlc, bcmc_scb, pkt) < 0) {
		WL_PS(("wl%d: Failed to enqueue BC/MC pkt for BSS %d\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bcmc_scb->bsscfg)));
		PKTFREE(wlc->osh, pkt, TRUE);
	}
	/* Force caller to give up packet and not tx */
	return TRUE;
}
#else
#define bcmc_pkt_q_check(wlc, bcmc_scb, pkt) FALSE
#endif /* MBSS */

/*
 * Common transmit packet routine
 * Return 0 when a packet is accepted and should not be referenced again
 * by the caller -- wlc will free it.
 * Return an error code when we're flow-controlled and the caller should try again later..
 */

int BCMFASTPATH
wlc_prep_sdu(wlc_info_t *wlc, struct scb *scb, void **pkts, int *npkts, uint *fifop)
{
#if !defined(NEW_TXQ)
	uint ndesc;
#endif /* !NEW_TXQ */
	uint i, j, nfrags, frag_length = 0, next_payload_len, fifo;
	void *sdu;
	uint offset;
	osl_t *osh;
	uint headroom, want_tailroom;
	uint pkt_length;
	uint8 prio = 0;
	struct ether_header *eh;
	bool is_8021x;
#ifdef BCMWAPI_WAI
	uint32 is_wai;
#endif /* BCMWAPI_WAI */
	bool fast_path;
	wlc_bsscfg_t *bsscfg;
	wlc_pkttag_t *pkttag;
#if defined(BCMDBG_ERR) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	bool is_tkip = FALSE;
	bool key_prep_sdu = FALSE;
#ifdef EXT_STA
	struct ether_header ethdr;
#endif // endif
#ifdef STA
	bool is_4way_m4 = FALSE;
#endif /* STA */
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;

	/* Make sure that we are passed in an SDU */
	sdu = pkts[0];
	ASSERT(sdu != NULL);
	pkttag = WLPKTTAG(sdu);
	ASSERT(pkttag != NULL);

	ASSERT((pkttag->flags & WLF_MPDU) == 0);

	osh = wlc->osh;

	ASSERT(scb != NULL);

	if (!wlc->pub->up) {
		WL_INFORM(("wl%d: wlc_prep_sdu: wl is not up\n", wlc->pub->unit));
		WLCNTINCR(wlc->pub->_cnt->txnoassoc);
		goto toss;
	}

	/* Something is blocking data packets */
	if (wlc->block_datafifo) {
		WL_INFORM(("wl%d: %s: block_datafifo 0x%x\n",
		           wlc->pub->unit, __FUNCTION__, wlc->block_datafifo));
		return BCME_BUSY;
	}

	is_8021x = (WLPKTTAG(sdu)->flags & WLF_8021X);
	if (is_8021x) {
		WL_ASSOC_LT(("tx:prep:802.1x\n"));
	}
#ifdef BCMWAPI_WAI
	is_wai = (WLPKTTAG(sdu)->flags & WLF_WAI);
#endif /* BCMWAPI_WAI */

#ifndef TXQ_MUX
	/* The MBSS BCMC PS work is checks to see if
	 * bcmc pkts need to go on a bcmc BSSCFG PS queue,
	 * what is being added here *is* the bcmc bsscfg queue with TXQ_MUX.
	 */
	if (SCB_ISMULTI(scb) && bcmc_pkt_q_check(wlc, scb, sdu)) {
		/* Does BCMC pkt need to go to BSS's PS queue? */
		/* Force caller to give up packet and not tx */
		return BCME_NOTREADY;
	}
#endif // endif

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	WL_APSTA_TX(("wl%d.%d: wlc_prep_sdu: pkt %p dst %s\n", wlc->pub->unit,
	             WLC_BSSCFG_IDX(bsscfg), sdu,
	             bcm_ether_ntoa(&scb->ea, eabuf)));

	/* toss if station is not associated to the correct bsscfg */
	if (bsscfg->BSS && !SCB_LEGACY_WDS(scb) && !SCB_ISMULTI(scb)) {
		/* Class 3 (BSS) frame */
		if (!SCB_ASSOCIATED(scb)) {
			WL_INFORM(("wl%d.%d: wlc_prep_sdu: invalid class 3 frame to "
				   "non-associated station %s\n", wlc->pub->unit,
				    WLC_BSSCFG_IDX(bsscfg), bcm_ether_ntoa(&scb->ea, eabuf)));
			WLCNTINCR(wlc->pub->_cnt->txnoassoc);
			goto toss;
		}
	}
#ifdef WLBTAMP
	else if (BSS_BTA_ENAB(wlc, bsscfg)) {
		/* Class 4 (BT-AMP) frame */
		if (!SCB_ASSOCIATED(scb)) {
			WL_INFORM(("wl%d.%d: wlc_prep_sdu: invalid frame to "
				   "non-associated BT-AMP peer %s\n", wlc->pub->unit,
				    WLC_BSSCFG_IDX(bsscfg), bcm_ether_ntoa(&scb->ea, eabuf)));
			WLCNTINCR(wlc->pub->_cnt->txnoassoc);
			goto toss;
		}
	}
#endif /* WLBTAMP */

/* XXX:TXQ
 * WES:
 * This check & toss should be outside this fn, in the SCB context of
 * the tx path,in the scbq output fn, or in the immedate txq output fn
 */
#if defined(TXQ_MUX)
	ASSERT(scb->bandunit == wlc->band->bandunit);
#else
	/* Toss the frame if scb's band does not match our current band
	 * this is possible if the STA has roamed while the packet was on txq
	 */
	if ((scb->bandunit != wlc->band->bandunit) &&
		!BSS_TDLS_ENAB(wlc, SCB_BSSCFG(scb))) {
		/* different band */
		WL_INFORM(("wl%d.%d: frame destined to %s sent on incorrect band %d\n",
		           wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), bcm_ether_ntoa(&scb->ea, eabuf),
		           scb->bandunit));
		WLCNTINCR(wlc->pub->_cnt->txnoassoc);
		goto toss;
	}
#endif /* TXQ_MUX */

	if (wlc_scb_txfifo(wlc, scb, sdu, &fifo) != BCME_OK) {
		goto toss;
	}
	/* prio could be different now */
	prio = (uint8)PKTPRIO(sdu);
#if defined(MACOSX) && !defined(NEW_TXQ)
	/* change the FIFO to VO if the packet is EAPOL */
	if (is_8021x) {
		fifo = TX_CTL_FIFO;
	}
#endif // endif

	*fifop = fifo;
#if !defined(NEW_TXQ)
	/* XXX: TXQ
	 * WES: will not need this check for time-based txq
	 * Instead the queue fill call askes for the pkt-time to fill
	 */
	/*
	 * Apply FIFO admission control to reduce "rate lag"
	 * by limiting the amount of data queued to the hardware.
	 */
	if ((TXPKTPENDGET(wlc, fifo) >= wlc_get_txmaxpkts(wlc)) && (fifo <= TX_AC_VO_FIFO)) {
		/* Mark precedences related to this FIFO, unsendable */
		WLC_TX_FIFO_CLEAR(wlc, fifo);
		return BCME_BUSY;
	}
#endif /* !NEW_TXQ */

	/* XXX:
	 * WES: Note, the runt check is wrong. Min length is
	 * ETHER_HDR_LEN (DA/SA/TypeLen) plus 3 bytes LLC
	 * If LLC is AA-AA-03, then 8 bytes req.
	 * SDU runt check should be moved up the stack to wlc_hdr_proc
	 */

	/* toss runt frames */
	pkt_length = pkttotlen(osh, sdu);
	if (pkt_length < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN) {
		WLCNTINCR(wlc->pub->_cnt->txrunt);
		WLCIFCNTINCR(scb, txrunt);
		WLCNTSCBINCR(scb->scb_stats.tx_failures);
#ifdef WL11K
		wlc_rrm_stat_qos_counter(scb, prio, OFFSETOF(rrm_stat_group_qos_t, txfail));
#endif // endif
		goto toss;
	}

	/* check enough headroom has been reserved */
	/* uncomment after fixing vx port
	 * ASSERT((uint)PKTHEADROOM(osh, sdu) < D11_TXH_LEN_EX(wlc));
	 */
#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub) && BSSCFG_SAFEMODE(bsscfg)) {
		wlc_ethaddr_from_d11hdr((struct dot11_header*)PKTDATA(osh, sdu), &ethdr);
		eh = &ethdr;
	}
	else
#endif // endif
	eh = (struct ether_header*) PKTDATA(osh, sdu);

	if (BSSCFG_AP(bsscfg) && !SCB_LEGACY_WDS(scb)) {
		/* Check if this is a packet indicating a STA roam and remove assoc state if so.
		 * Only check if we are getting the send on a non-wds link so that we do not
		 * process our own roam/assoc-indication packets
		 */
		if (ETHER_ISMULTI(eh->ether_dhost))
			if (wlc_roam_check(wlc->ap, bsscfg, eh, pkt_length))
				goto toss;
	}

	/* toss if station not yet authorized to receive non-802.1X frames */
	if (bsscfg->eap_restrict && !SCB_ISMULTI(scb) && !SCB_AUTHORIZED(scb)) {
		if (!is_8021x) {
			WL_ERROR(("wl%d.%d: non-802.1X frame to unauthorized station %s (%s)\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
			        bcm_ether_ntoa(&scb->ea, eabuf),
				SCB_LEGACY_WDS(scb) ? "WDS" : "STA"));
			goto toss;
		}
	}

#ifdef BCMWAPI_WAI
	/* toss if station not yet authorized to receive non-WAI frames */
	if (WAPI_WAI_RESTRICT(wlc, bsscfg) && !SCB_ISMULTI(scb) && !SCB_AUTHORIZED(scb)) {
		if (!is_wai) {
			WL_ERROR(("wl%d.%d: non-WAI frame len %d to unauthorized station %s\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), pkt_length,
				bcm_ether_ntoa(&scb->ea, eabuf)));
			goto toss;
		}
	}
#endif /* BCMWAPI_WAI */

	WL_PRUSR("tx", (uchar*)eh, ETHER_HDR_LEN);

	if (WSEC_ENABLED(bsscfg->wsec) && !BSSCFG_SAFEMODE(bsscfg)) {
#ifdef EXT_STA
	    if (WLEXTSTA_ENAB(wlc->pub) && BSSCFG_HAS_NATIVEIF(bsscfg)) {
		uint exempt;

		exempt = WLPKTFLAG_EXEMPT_GET(WLPKTTAG(sdu));
		ASSERT((exempt == WSEC_EXEMPT_NO)||
		       (exempt == WSEC_EXEMPT_ALWAYS) ||
		       (exempt == WSEC_EXEMPT_NO_PAIRWISE));
		if (exempt == WSEC_EXEMPT_ALWAYS) {
			/* Do not encrypt...packet marked exempt */
			WL_WSEC(("wl%d: wlc_send: not encrypting exempt frame\n",
			            WLCWLUNIT(wlc)));
		} else if (exempt == WSEC_EXEMPT_NO_PAIRWISE) {
			/* send unencrypted if no pairwise key exists */
			key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
				WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
			if (key_info.algo != CRYPTO_ALGO_OFF) {
				WL_WSEC(("wl%d: wlc_send: encrypting exempt frame using "
					 "per-path key\n", WLCWLUNIT(wlc)));
			} else {
				WL_WSEC(("wl%d: wlc_send: not encrypting exempt frame, "
					 "no per-path key available\n", WLCWLUNIT(wlc)));
			}
		}
		else {
			/* Use a paired key or primary group key if present, toss otherwise */
			key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
				WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
			if (key_info.algo != CRYPTO_ALGO_OFF) {
				WL_WSEC(("wl%d.%d: wlc_prep_sdu: using per-path key algo %d\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), key_info.algo));
			} else if (is_8021x && WSEC_SES_OW_ENABLED(bsscfg->wsec)) {
				WL_WSEC(("wl%d.%d: wlc_prep_sdu: not encrypting 802.1x frame "
				        "during OW\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
			} else {
				key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE,
					&key_info);
				if (key_info.algo == CRYPTO_ALGO_OFF) {
					WL_WSEC(("wl%d.%d: wlc_prep_sdu: "
						"ext_sta tossing unencryptable frame\n",
						wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
					goto toss;
				}
			}
		}
	    } else
#endif /* EXT_STA */
	    {
		if (is_8021x) {
			/*
			* use per scb WPA_auth to handle 802.1x frames differently
			* in WPA/802.1x mixed mode when having a pairwise key:
			*  - 802.1x frames are unencrypted in plain 802.1x mode
			*  - 802.1x frames are encrypted in WPA mode
			*/
			uint32 WPA_auth = SCB_LEGACY_WDS(scb) ? bsscfg->WPA_auth : scb->WPA_auth;

			key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
				WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
			if ((WPA_auth != WPA_AUTH_DISABLED) &&
			    (key_info.algo != CRYPTO_ALGO_OFF)) {
				WL_WSEC(("wl%d.%d: wlc_prep_sdu: encrypting 802.1X frame using "
					"per-path key\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
			} else {
				/* Do not encrypt 802.1X packets with group keys */
				WL_WSEC(("wl%d.%d: wlc_prep_sdu: not encrypting 802.1x frame\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
			}
		}
#ifdef BCMWAPI_WAI
		else if (is_wai) {
			/* Do not encrypt WAI packets with group keys */
			WL_WSEC(("wl%d.%d: wlc_prep_sdu: not encrypting WAI frame\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		}
#endif /* BCMWAPI_WAI */
		else {
			/* Use a paired key or primary group key if present, toss otherwise */
			key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
				WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
			if (key_info.algo != CRYPTO_ALGO_OFF) {
				WL_WSEC(("wl%d.%d: wlc_prep_sdu: using pairwise key\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
			} else if ((key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt,
				bsscfg, FALSE, &key_info)) != NULL &&
				key_info.algo != CRYPTO_ALGO_OFF) {
				WL_WSEC(("wl%d.%d: wlc_prep_sdu: using default tx key\n",
				         wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
			} else {
				WL_WSEC(("wl%d.%d: wlc_prep_sdu: tossing unencryptable frame\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
				goto toss;
			}
		}
	    }
	} else {	/* Do not encrypt packet */
		WL_WSEC(("wl%d.%d: wlc_prep_sdu: not encrypting frame, encryption disabled\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		WL_WSEC(("wl%d.%d: wlc_prep_sdu: wsec 0x%x \n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg), bsscfg->wsec));
	}

	/* ensure we have a valid (potentially null, with ALGO_OFF) key and key_info */
	if (key == NULL)
		key = wlc_keymgmt_get_bss_key(wlc->keymgmt, bsscfg, WLC_KEY_ID_INVALID, &key_info);
	ASSERT(key != NULL);

#ifdef STA
	if (is_8021x && !BSSCFG_SAFEMODE(bsscfg) &&
		wlc_keymgmt_b4m4_enabled(wlc->keymgmt, bsscfg)) {
		if (wlc_is_4way_msg(wlc, sdu, ETHER_HDR_LEN, PMSG4)) {
			WL_WSEC(("wl%d:%s(): Tx 4-way M4 pkt...\n", wlc->pub->unit, __FUNCTION__));
			is_4way_m4 = TRUE;
		}
	}
#endif /* STA */

	if (key_info.algo != CRYPTO_ALGO_OFF) {
#ifdef BCMCCX
		/* CKIP: header conversion and MIC encapsulation calculation */
		if (WSEC_CKIP_MIC_ENABLED(bsscfg->wsec)) {
			int err;
			ASSERT((key_info.algo == CRYPTO_ALGO_WEP1) ||
			        (key_info.algo == CRYPTO_ALGO_WEP128));

			key_prep_sdu = TRUE;
			err = wlc_key_prep_tx_msdu(key, sdu, 0, prio);
			if (err != BCME_OK) {
				WL_WSEC(("wl%d.%d: %s: Error %d adding CKIP MIC to SDU\n",
					WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err));
				goto toss;
			}

			/* convert 8023hdr to ckip hdr, update pkt_length and eh */
			pkt_length = PKTLEN(osh, sdu);
			eh = (struct ether_header*) PKTDATA(osh, sdu);
		}
#endif /* BCMCCX */

		/* TKIP MIC space reservation */
		if (key_info.algo == CRYPTO_ALGO_TKIP) {
			is_tkip = TRUE;
			pkt_length += TKIP_MIC_SIZE;
		}
	}

	/* calculate how many frags needed
	 *  ETHER_HDR - (CKIP_LLC_MIC) - LLC/SNAP-ETHER_TYPE - payload - TKIP_MIC
	 */
	if (!WLPKTFLAG_AMSDU(pkttag) && !WLPKTFLAG_AMPDU(pkttag) && !BSSCFG_SAFEMODE(bsscfg) &&
		!is_8021x)
		nfrags = wlc_frag(wlc, scb, WME_PRIO2AC(prio), pkt_length, &frag_length);
	else {
		frag_length = pkt_length - ETHER_HDR_LEN;
		nfrags = 1;
	}

#if !defined(NEW_TXQ)
	/* WES: low txq now handles the direct queuing to the DMA ring or PIO. */
	if (!PIO_ENAB(wlc->pub)) {
		/* calc the number of descriptors needed to queue this frame */
		if (nfrags == 1) {
			/* for unfragmented packets, count the number of packet buffer
			 * segments, each being a contiguous virtual address range.
			 */
			ndesc = pktsegcnt(osh, sdu);
		} else {
			/* for fragmented packets, each frag will be created from
			 * an newly allocated packet buffer.
			 */
			ndesc = nfrags;
		}

		/*
		 * WES: This does not seem correct.
		 * This code should only be in the nfrags > 1 case if at all.
		 */
#ifdef DMATXRC
		/* early tx reclaim uses additional descriptor for pkt header */
		if (DMATXRC_ENAB(wlc->pub) && (pkttag->flags & WLF_PHDR))
			ndesc++;
#endif // endif
		/* DMA avoidance WAR may split ranges into two descriptors */
		if (wlc->dma_avoidance_war)
			ndesc *= 2;

		/* bail if there is insufficient room in the ring to hold all fragments */
		if (TXAVAIL(wlc, fifo) <= ndesc) {
			/* Mark precedences related to this FIFO, unsendable */
			WLC_TX_FIFO_CLEAR(wlc, fifo);
			return BCME_BUSY;
		}
	} else {
		uint sz;

		/* calculate required "flow-control" amount of all fragments */

		/* per frag header overhead */
		sz = D11_TXH_LEN_EX(wlc) + DOT11_A3_HDR_LEN;

		if (!WLCISACPHY(wlc->band))
			sz += D11_PHY_HDR_LEN;
		/* account for A4 */
		if (SCB_A4_DATA(scb))
			sz += ETHER_ADDR_LEN;
		/* SCB_QOS: account for QoS Control Field */
		if (SCB_QOS(scb))
			sz += DOT11_QOS_LEN;
		/* per frag encrypt overhead */
		sz += key_info.iv_len;
		if (WLC_KEY_SW_ONLY(&key_info) || WLC_KEY_IS_LINUX_CRYPTO(&key_info))
			sz += key_info.icv_len;

		/* total frag overhead */
		sz = sz * nfrags;
		sz += pkt_length - ETHER_HDR_LEN;
		/* bail if there is insufficient room in the fifo to hold all fragments */
		if (!wlc_pio_txavailable(WLC_HW_PIO(wlc, fifo), sz, nfrags)) {
			/* Mark precedences related to this FIFO, unsendable */
			WLC_TX_FIFO_CLEAR(wlc, fifo);
			return BCME_BUSY;
		}
	}
#endif /* !NEW_TXQ */
	fast_path = (nfrags == 1);

	/* prepare sdu (non ckip, which already added the header mic) */
	if (key_prep_sdu != TRUE) {
		(void)wlc_key_prep_tx_msdu(key, sdu, ((nfrags > 1) ? frag_length : 0), prio);
	}

	/*
	 * Prealloc all fragment buffers for this frame.
	 * If any of the allocs fail, free them all and bail.
	 * p->data points to ETHER_HEADER, original ether_type is passed separately
	 */
	offset = ETHER_HDR_LEN;
	for (i = 0; i < nfrags; i++) {
		headroom = TXOFF;
		want_tailroom = 0;

		if (key_info.algo != CRYPTO_ALGO_OFF) {
			if (WLC_KEY_SW_ONLY(&key_info) || WLC_KEY_IS_LINUX_CRYPTO(&key_info)) {
				headroom += key_info.iv_len;
				want_tailroom += key_info.icv_len;
				fast_path = FALSE;		/* no fast path for SW encryption */
			}

			/* In TKIP, MIC field can span over two fragments or can be the only payload
			 * of the last frag. Add tailroom for TKIP MIC for the last two
			 * fragments.
			 *   use slow_path for pkt with >=3 buffers(header + data)
			 *   since wlc_dofrag_tkip only works for pkt with <=2 buffers
			 * WPA FIXME: algo checking is done inconsistently
			 */
			if (is_tkip) {
				void *p;
				p = PKTNEXT(osh, sdu);
				if ((i + 2) >= nfrags) {
					WL_WSEC(("wl%d.%d: wlc_prep_sdu: checking "
						"space for TKIP tailroom\n",
						wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
					want_tailroom += TKIP_MIC_SIZE;
					if (p != NULL) {
						if (PKTNEXT(osh, p))
							fast_path = FALSE;
						else if ((uint)PKTTAILROOM(osh, p) < want_tailroom)
							fast_path = FALSE;
					} else if ((uint)PKTTAILROOM(osh, sdu) < want_tailroom)
						fast_path = FALSE;
				}

				/* Cloned TKIP packets to go via the slow path so that
				 * each interface gets a private copy of data to add MIC
				 */
				if (PKTSHARED(sdu) || (p && PKTSHARED(p)))
					fast_path = FALSE;
			}
		}

		if (fast_path) {
			/* fast path for non fragmentation/non copy of pkts
			 * header buffer has been allocated in wlc_sendpkt()
			 * don't change ether_header at the front
			 */
			/* tx header cache hit */
			if (WLC_TXC_ENAB(wlc) &&
			    !BSSCFG_SAFEMODE(bsscfg)) {
				wlc_txc_info_t *txc = wlc->txc;

				if (TXC_CACHE_ENAB(txc) &&
				    /* wlc_prep_sdu_fast may set WLF_TXCMISS... */
				    (pkttag->flags & WLF_TXCMISS) == 0 &&
				    (pkttag->flags & WLF_BYPASS_TXC) == 0) {
					if (wlc_txc_hit(txc, scb, sdu,
						((is_tkip && WLC_KEY_MIC_IN_HW(&key_info)) ?
						(pkt_length - TKIP_MIC_SIZE) : pkt_length),
						fifo, prio)) {
						int err;

						WLCNTINCR(wlc->pub->_cnt->txchit);
						err = wlc_txfast(wlc, scb, sdu, pkt_length,
							key, &key_info);
						if (err != BCME_OK)
							return err;

						*npkts = 1;
						goto done;
					}
					pkttag->flags |= WLF_TXCMISS;
					WLCNTINCR(wlc->pub->_cnt->txcmiss);
				}
			}
			ASSERT(i == 0);

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
			/* It's time to swap to use D11_BUFFER to construct mpdu */
			if (BCMDHDHDR_ENAB() && PKTISTXFRAG(osh, sdu) &&
				PKTSWAPD11BUF(osh, sdu) != BCME_OK)
				return BCME_BUSY;
#endif // endif
			pkts[0] = sdu;
		} else {
			/* before fragmentation make sure the frame contents are valid */
			PKTCTFMAP(osh, sdu);

			/* fragmentation:
			 * realloc new buffer for each frag, copy over data,
			 * append original ether_header at the front
			 */
#if defined(BCMPCIEDEV) && defined(BCMFRAGPOOL)
			if (BCMPCIEDEV_ENAB() && BCMLFRAG_ENAB() && PKTISTXFRAG(osh, sdu) &&
			    (PKTFRAGTOTLEN(osh, sdu) > 0))
				pkts[i] = wlc_allocfrag_txfrag(osh, sdu, offset, frag_length,
					(i == (nfrags-1)));
			else
#endif // endif
				pkts[i] = wlc_allocfrag(osh, sdu, offset, headroom, frag_length,
					want_tailroom);

			if (pkts[i] == NULL) {
				for (j = 0; j < i; j++)
					PKTFREE(osh, pkts[j], TRUE);
				pkts[0] = sdu;    /* restore pointer to sdu */
				WL_ERROR(("wl%d.%d: %s: allocfrag failed\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
					__FUNCTION__));
				WLCNTINCR(wlc->pub->_cnt->txnobuf);
				WLCIFCNTINCR(scb, txnobuf);
				WLCNTSCBINCR(scb->scb_stats.tx_failures);
#ifdef WL11K
				wlc_rrm_stat_qos_counter(scb, prio,
					OFFSETOF(rrm_stat_group_qos_t, txfail));
#endif // endif
				return BCME_BUSY;
			} else {
#ifdef BCMWAPI_WPI
				if ((key_info.algo == CRYPTO_ALGO_SMS4) &&
				    WLC_KEY_SW_ONLY(&key_info)) {
					int cur_len = PKTLEN(osh, pkts[i]);
					PKTSETLEN(osh, pkts[i],
						cur_len + SMS4_WPI_CBC_MAC_LEN);
				}
#endif /* BCMWAPI_WPI */

				/* Copy the ETH in sdu to each pkts[i] fragment */
				PKTPUSH(osh, pkts[i], ETHER_HDR_LEN);
				bcopy((char*)eh, (char*)PKTDATA(osh, pkts[i]), ETHER_HDR_LEN);

				/* Slow path, when BCM_DHDHDR is enabled,
				 * each pkts[] has ether header 14B in dongle and host data addr
				 * points to llc snap 8B in DHDHDR for first pkts[].
				 * The others host data addr point to data partion in different
				 * offest.
				 */

				/* Transfer SDU's pkttag info to the last fragment */
				if (i == (nfrags - 1)) {
					wlc_pkttag_info_move(wlc, sdu, pkts[i]);

					/* Reset original sdu's metadata, so that PKTFREE of
					 * original sdu does not send tx status prematurely
					 */
#ifdef BCMPCIEDEV
					if (BCMPCIEDEV_ENAB() && BCMLFRAG_ENAB() &&
					    PKTISTXFRAG(osh, sdu) && (PKTFRAGTOTLEN(osh, sdu) > 0))
						PKTRESETHASMETADATA(osh, sdu);
#endif // endif
				}
			}

			/* set fragment priority */
			PKTSETPRIO(pkts[i], PKTPRIO(sdu));

			/* leading frag buf must be aligned 0-mod-2 */
			ASSERT(ISALIGNED(PKTDATA(osh, pkts[i]), 2));
			offset += frag_length;
		}
	}

	/* build and transmit each fragment */
	for (i = 0; i < nfrags; i++) {
		if (i < nfrags - 1) {
			next_payload_len = pkttotlen(osh, pkts[i + 1]);
			ASSERT(next_payload_len >= ETHER_HDR_LEN);
			next_payload_len -= ETHER_HDR_LEN;
		}
		else
			next_payload_len = 0;

		WLPKTTAGSCBSET(pkts[i], scb);

		/* ether header is on front of each frag to provide the dhost and shost
		 *  ether_type may not be original, should not be used
		 */
		wlc_dofrag(wlc, pkts[i], i, nfrags, next_payload_len, scb,
			is_8021x, fifo, key, &key_info, prio, frag_length);

	}

	/* Free the original SDU if not shared */
	if (pkts[nfrags - 1] != sdu) {
		PKTFREE(osh, sdu, TRUE);
		sdu = pkts[nfrags - 1];
		BCM_REFERENCE(sdu);
	}

	*npkts = nfrags;

done:
#ifdef STA
	if (is_4way_m4)
		wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_M4_TX, bsscfg, scb, key, sdu);
#endif /* STA */

	WLCNTSCBINCR(scb->scb_stats.tx_pkts);
#ifdef WL11K
	wlc_rrm_stat_bw_counter(wlc, scb, TRUE);
	wlc_rrm_stat_qos_counter(scb, prio, OFFSETOF(rrm_stat_group_qos_t, txframe));
#endif // endif
	WLCNTSCBADD(scb->scb_stats.tx_ucast_bytes, pkt_length);
	wlc_update_txpktsuccess_stats(wlc, scb, pkt_length, prio);
	return 0;
toss:
	*npkts = 0;
	wlc_update_txpktfail_stats(wlc, pkttotlen(osh, sdu), prio);
	PKTFREE(osh, sdu, TRUE);
	return BCME_ERROR;
}

/* driver fast path to send sdu using cached tx d11-phy-mac header (before llc/snap)
 *  doesn't support wsec and eap_restrict for now
 */
static int BCMFASTPATH
wlc_txfast(wlc_info_t *wlc, struct scb *scb, void *sdu, uint pktlen,
	wlc_key_t *key, const wlc_key_info_t *key_info)
{
	osl_t *osh = wlc->osh;
	struct ether_header *eh;
	uint8 *txh;
	uint16 seq = 0, frameid;
	struct dot11_header *h;
	struct ether_addr da, sa;
	uint fifo = 0;
	wlc_pkttag_t *pkttag;
	wlc_bsscfg_t *bsscfg;
	uint flags = 0;
	uint16 txc_hwseq, TxFrameID_off;
	uint d11TxdLen;

	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	ASSERT(ISALIGNED(PKTDATA(osh, sdu), 2));

	pkttag = WLPKTTAG(sdu);

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	/* It's time to swap to use D11_BUFFER to construct mpdu */
	if (BCMDHDHDR_ENAB() && PKTISTXFRAG(wlc->osh, sdu) &&
		PKTSWAPD11BUF(wlc->osh, sdu) != BCME_OK)
		return BCME_BUSY;
#endif // endif

	/* headroom has been allocated, may have llc/snap header already */
	/* uncomment after fixing osl_vx port
	 * ASSERT(PKTHEADROOM(osh, sdu) >= (TXOFF - DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN));
	 */

	/* eh is going to be overwritten, save DA and SA for later fixup */
	eh = (struct ether_header*) PKTDATA(osh, sdu);
	eacopy((char*)eh->ether_dhost, &da);
	eacopy((char*)eh->ether_shost, &sa);

	/* strip off ether header, copy cached tx header onto the front of the frame */
	PKTPULL(osh, sdu, ETHER_HDR_LEN);

	ASSERT(WLC_TXC_ENAB(wlc));

	txh = (uint8*)wlc_txc_cp(wlc->txc, scb, sdu, &flags);
#if defined(WLC_UCODE_CACHE)
	if (D11REV_GE(wlc->pub->corerev, 42))
		d11TxdLen = D11_TXH_SHORT_LEN(wlc);
	else
#endif /* WLC_UCODE_CACHE */
		d11TxdLen = D11_TXH_LEN_EX(wlc);

	/* txc_hit doesn't check DA,SA, fixup is needed for different configs */
	h = (struct dot11_header*) (((char*)txh) + d11TxdLen);

	if (WLCISACPHY(wlc->band)) {
		TxFrameID_off = OFFSETOF(d11actxh_t, PktInfo.TxFrameID);
		txc_hwseq = (ltoh16(((d11actxh_t *)txh)->PktInfo.MacTxControlLow)) & D11AC_TXC_ASEQ;
	} else {
		TxFrameID_off = OFFSETOF(d11txh_t, TxFrameID);
		txc_hwseq = (ltoh16(((d11txh_t *)txh)->MacTxControlLow)) & TXC_HWSEQ;
	}

	if (!bsscfg->BSS) {
		/* IBSS, no need to fixup BSSID */
	} else if (SCB_A4_DATA(scb)) {
		/* wds: fixup a3 with DA, a4 with SA */
		ASSERT((ltoh16(h->fc) & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
		eacopy((char*)&da, (char*)&h->a3);
		eacopy((char*)&sa, (char*)&h->a4);
	} else if (BSSCFG_STA(bsscfg)) {
		/* normal STA to AP, fixup a3 with DA */
		eacopy((char*)&da, (char*)&h->a3);
	} else {
		/* normal AP to normal STA, fixup a3 with SA */
		eacopy((char*)&sa, (char*)&h->a3);
	}

#ifdef WLAMSDU_TX
	/* fixup qos control field to indicate it is an AMSDU frame */
	if (AMSDU_TX_ENAB(wlc->pub) &&	AMSDU_TX_AC_ENAB(wlc->ami, PKTPRIO(sdu)) &&
			SCB_QOS(scb)) {
		uint16 *qos;
		qos = (uint16 *)((uint8 *)h + ((SCB_WDS(scb) || SCB_DWDS(scb)) ? DOT11_A4_HDR_LEN :
				DOT11_A3_HDR_LEN));
		/* set or clear the A-MSDU bit */
		if (WLPKTFLAG_AMSDU(pkttag))
			*qos |= htol16(QOS_AMSDU_MASK);
		else
			*qos &= ~htol16(QOS_AMSDU_MASK);
	}
#endif // endif

	/* fixup counter in TxFrameID */
	fifo = WLC_TXFID_GET_QUEUE(ltoh16(*(uint16 *)(txh + TxFrameID_off)));
	if (fifo == TX_BCMC_FIFO) {
		frameid = bcmc_fid_generate(wlc, bsscfg, *(uint16 *)(txh + TxFrameID_off));
	} else {
		frameid = ltoh16(*(uint16 *)(txh + TxFrameID_off)) & ~TXFID_SEQ_MASK;
		seq = (wlc->counter++) << TXFID_SEQ_SHIFT;
		frameid |= (seq & TXFID_SEQ_MASK);
	}
	*(uint16 *)(txh + TxFrameID_off) = htol16(frameid);

	/* fix up the seqnum in the hdr */
#ifdef PROP_TXSTATUS
	if (WL_SEQ_GET_FROMDRV(pkttag->seq)) {
		seq = WL_SEQ_GET_NUM(pkttag->seq) << SEQNUM_SHIFT;
		h->seq = htol16(seq);
	} else if (WLPKTFLAG_AMPDU(pkttag)) {
		seq = WL_SEQ_GET_NUM(pkttag->seq) << SEQNUM_SHIFT;
		h->seq = htol16(seq);
	}
#else
	if (WLPKTFLAG_AMPDU(pkttag)) {
		seq = pkttag->seq << SEQNUM_SHIFT;
		h->seq = htol16(seq);
	}
#endif /* PROP_TXSTATUS */

	else if (!txc_hwseq) {
		seq = SCB_SEQNUM(scb, PKTPRIO(sdu)) << SEQNUM_SHIFT;
		SCB_SEQNUM(scb, PKTPRIO(sdu))++;
		h->seq = htol16(seq);
	}

#ifdef PROP_TXSTATUS
	if (WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
		WL_SEQ_SET_NUM(pkttag->seq, ltoh16(h->seq) >> SEQNUM_SHIFT);
		WL_SEQ_SET_FROMFW(pkttag->seq, 1);
	}
#endif /* PROP_TXSTATUS */

	/* TDLS U-APSD Buffer STA: save Seq and TID for PIT */
#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, SCB_BSSCFG(scb)) && wlc_tdls_buffer_sta_enable(wlc->tdls)) {
		uint16 fc;
		uint16 type;
		uint8 tid = 0;
		bool a4;

		fc = ltoh16(h->fc);
		type = FC_TYPE(fc);
		a4 = ((fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
		if ((type == FC_TYPE_DATA) && FC_SUBTYPE_ANY_QOS(FC_SUBTYPE(fc))) {
			uint16 qc;
			int offset = a4 ? DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN;

			qc = (*(uint16 *)((uchar *)h + offset));
			tid = (QOS_TID(qc) & 0x0f);
		}
		wlc_tdls_update_tid_seq(wlc->tdls, scb, tid, seq);
	}
#endif /* WLTDLS */

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		uint tsoHdrSize = 0;

		/* Update timestamp */
		if ((pkttag->flags & WLF_EXPTIME)) {
			((d11actxh_t *)txh)->PktInfo.Tstamp =
			                htol16((pkttag->u.exptime >> D11AC_TSTAMP_SHIFT) & 0xffff);
			((d11actxh_t *)txh)->PktInfo.MacTxControlLow |= htol16(D11AC_TXC_AGING);
		}
		((d11actxh_t *)txh)->PktInfo.Seq = h->seq;

		/* Get the packet length after adding d11 header. */
		pktlen = pkttotlen(osh, sdu);
		/* Update frame len (fc to fcs) */
#ifdef WLTOEHW
		tsoHdrSize = (wlc->toe_bypass ?
			0 : wlc_tso_hdr_length((d11ac_tso_t*)PKTDATA(wlc->osh, sdu)));
#endif // endif
		((d11actxh_t *)txh)->PktInfo.FrameLen =
			htol16((uint16)(pktlen +  DOT11_FCS_LEN - d11TxdLen - tsoHdrSize));
	} else {
		/* Update timestamp */
		if ((pkttag->flags & WLF_EXPTIME)) {
			((d11txh_t *)txh)->TstampLow = htol16(pkttag->u.exptime & 0xffff);
			((d11txh_t *)txh)->TstampHigh = htol16((pkttag->u.exptime >> 16) & 0xffff);
			((d11txh_t *)txh)->MacTxControlLow |= htol16(TXC_LIFETIME);
		}
	}

	if (CAC_ENAB(wlc->pub) && (fifo != TX_BCMC_FIFO)) {
		/* Request the usage of cached value for duration in CAC
		 * update cac used time with cached value.
		 */
		if (wlc_cac_use_dur_cache(wlc->cac, WME_PRIO2AC(PKTPRIO(sdu)), PKTPRIO(sdu), scb,
				(ltoh16(((d11actxh_t *)txh)->PktInfo.FrameLen))))
			WL_ERROR(("wl%d: ac %d: txop exceeded allocated TS time\n",
			          wlc->pub->unit, WME_PRIO2AC(PKTPRIO(sdu))));
	}

	/* update packet tag with the saved flags */
	pkttag->flags |= flags;

	wlc_txc_prep_sdu(wlc->txc, scb, key, key_info, sdu);

	if ((BSSCFG_AP(bsscfg) || BSS_TDLS_BUFFER_STA(bsscfg)) &&
		SCB_PS(scb) && (fifo != TX_BCMC_FIFO)) {
		if (AC_BITMAP_TST(scb->apsd.ac_delv, WME_PRIO2AC(PKTPRIO(sdu))))
			wlc_apps_apsd_prepare(wlc, scb, sdu, h, TRUE);
		else
			wlc_apps_pspoll_resp_prepare(wlc, scb, sdu, h, TRUE);
	}

	/* (bsscfg specific): Add one more entry of the current rate to keep an accurate histogram.
	 * If the rate changed, we wouldn't be in the fastpath
	*/
	if (!bsscfg->txrspecidx) {
		bsscfg->txrspec[bsscfg->txrspecidx][0] = bsscfg->txrspec[NTXRATE-1][0]; /* rspec */
		bsscfg->txrspec[bsscfg->txrspecidx][1] = bsscfg->txrspec[NTXRATE-1][1]; /* nfrags */
	} else {
		bsscfg->txrspec[bsscfg->txrspecidx][0] = bsscfg->txrspec[bsscfg->txrspecidx - 1][0];
		bsscfg->txrspec[bsscfg->txrspecidx][1] = bsscfg->txrspec[bsscfg->txrspecidx - 1][1];
	}

	bsscfg->txrspecidx = (bsscfg->txrspecidx+1) % NTXRATE;

#if defined(PROP_TXSTATUS) && defined(WLFCTS)
#ifdef WLCNTSCB
	if (WLFCTS_ENAB(wlc->pub))
		pkttag->rspec = scb->scb_stats.tx_rate;
#endif // endif
#endif /* PROP_TXSTATUS & WLFCTS */

	return BCME_OK;
}

int BCMFASTPATH
wlc_prep_sdu_fast(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb, void *sdu, uint *fifop,
	wlc_key_t *key, wlc_key_info_t *key_info, uint8* txc_hit)
{
	uint pktlen;
#if !defined(NEW_TXQ)
	uint ndesc;
#endif /* NEW_TXQ */
	uint32 fifo;
	uint8 prio = 0;
	int err = BCME_OK;

	ASSERT(SCB_AMPDU(scb));
	ASSERT(!SCB_ISMULTI(scb));

	/* On the AP, bcast/mcast always goes through the bcmc fifo if bcmc_scb
	 * is in PS Mode. bcmc_scb is in PS Mode if at least one ucast STA is in PS.
	 * SDUs are data packets and always go in BE_FIFO unless WME is enabled.
	 */
	prio = SCB_QOS(scb) ? (uint8)PKTPRIO(sdu) : 0;
	ASSERT(prio <= MAXPRIO);

	if (SCB_WME(scb))
		fifo = prio2fifo[prio];
	else
		fifo = TX_AC_BE_FIFO;
#ifdef WL_MU_TX
	if (BSSCFG_AP(bsscfg) && MU_TX_ENAB(wlc))
		wlc_mutx_sta_txfifo(wlc->mutx, scb, &fifo);
#endif // endif

	*fifop = fifo;

#if !defined(NEW_TXQ)
	/*
	 * WES: will not need this check for time-based txq
	 * Instead the queue fill call asks for the pkt-time to fill
	 */
	/*
	 * Apply FIFO admission control to reduce "rate lag"
	 * by limiting the amount of data queued to the hardware.
	 */
	if ((TXPKTPENDGET(wlc, fifo) >= wlc_get_txmaxpkts(wlc)) && (fifo <= TX_AC_VO_FIFO)) {
		/* Mark precedences related to this FIFO, unsendable */
		WLC_TX_FIFO_CLEAR(wlc, fifo);
		return BCME_BUSY;
	}
#endif /* !NEW_TXQ */

	/* 1x or fragmented frames will force slow prep */
	pktlen = pkttotlen(wlc->osh, sdu);
	if ((WLPKTTAG(sdu)->flags &
	    (WLF_8021X
#ifdef BCMWAPI_WAI
	     | WLF_WAI
#endif // endif
	     | 0)) || (((WLPKTTAG(sdu)->flags & WLF_AMSDU) == 0) &&
	      ((uint16)pktlen >= (wlc->fragthresh[WME_PRIO2AC(prio)] -
	      (uint16)(DOT11_A4_HDR_LEN + DOT11_QOS_LEN + DOT11_FCS_LEN + ETHER_ADDR_LEN))))) {
		return BCME_UNSUPPORTED;
	}

#if !defined(NEW_TXQ)
	/* WES: low txq now handles the direct queuing to the DMA ring or PIO. */
	/* calc the number of descriptors needed to queue this frame */
	ndesc = pktsegcnt(wlc->osh, sdu);

#ifdef DMATXRC
	/* early tx reclaim uses additional descriptor for pkt header */
	if (DMATXRC_ENAB(wlc->pub) && (WLPKTTAG(sdu)->flags & WLF_PHDR))
		ndesc++;
#endif // endif

	/* DMA avoidance WAR may split ranges into two descriptors */
	if (wlc->dma_avoidance_war)
		ndesc *= 2;

	/* bail if there is insufficient room in the ring to hold all fragments */
	if (TXAVAIL(wlc, fifo) <= ndesc) {
		/* Mark precedences related to this FIFO, unsendable */
		WLC_TX_FIFO_CLEAR(wlc, fifo);
		return BCME_BUSY;
	}
#endif /* !NEW_TXQ */

	/* TX header cache hit */
	if (WLC_TXC_ENAB(wlc)) {
		wlc_txc_info_t *txc = wlc->txc;

		if (TXC_CACHE_ENAB(txc) &&
		    (WLPKTTAG(sdu)->flags & WLF_BYPASS_TXC) == 0) {
			ASSERT((WLPKTTAG(sdu)->flags & WLF_TXCMISS) == 0);
			if ((*txc_hit == 1) || (wlc_txc_hit(txc, scb, sdu, pktlen, fifo, prio))) {
				WLCNTINCR(wlc->pub->_cnt->txchit);
				err = wlc_txfast(wlc, scb, sdu, pktlen, key, key_info);
				if (err != BCME_OK)
					return err;
				*txc_hit = 1;
				goto done;
			}
			WLPKTTAG(sdu)->flags |= WLF_TXCMISS;
			WLCNTINCR(wlc->pub->_cnt->txcmiss);
		}
	}

	return BCME_UNSUPPORTED;

done:
	WLCNTSCBINCR(scb->scb_stats.tx_pkts);
#ifdef WL11K
	wlc_rrm_stat_bw_counter(wlc, scb, TRUE);
	wlc_rrm_stat_qos_counter(scb, prio, OFFSETOF(rrm_stat_group_qos_t, txframe));
#endif // endif
	WLCNTSCBADD(scb->scb_stats.tx_ucast_bytes, pktlen);
	wlc_update_txpktsuccess_stats(wlc, scb, pktlen, prio);
	return BCME_OK;
}

void BCMFASTPATH
wlc_tx_open_datafifo(wlc_info_t *wlc)
{
#ifdef STA
	wlc_bsscfg_t *cfg;
#endif /* STA */

	/* Don't opend datafifo critical session */
	if (!wlc->pub->up || wlc->hw->reinit || wlc->txfifo_detach_pending) {
		return;
	}

	if (TXPKTPENDTOT(wlc) == 0) {
		if (wlc->block_datafifo & DATA_BLOCK_TX_SUPR)
			wlc_bsscfg_tx_check(wlc);

		if (wlc->block_datafifo & DATA_BLOCK_PS)
			wlc_apps_process_pend_ps(wlc);

#ifdef WL_MU_TX
		if ((wlc->block_datafifo & DATA_BLOCK_MUTX)) {
			wlc_mutx_txfifo_complete(wlc);
			wlc_block_datafifo(wlc, DATA_BLOCK_MUTX, 0);
		}
#endif /* WL_MU_TX */

#ifdef WL11N
		if ((wlc->block_datafifo & DATA_BLOCK_TXCHAIN)) {
			if (wlc->stf->txchain_pending != 0) {
				wlc_stf_txchain_set_complete(wlc);
				wlc_block_datafifo(wlc, DATA_BLOCK_TXCHAIN, 0);
			}
#ifdef WL_MODESW
			if (wlc->stf->pending_opstreams) {
				wlc_stf_op_txrxstreams_complete(wlc);
				wlc_block_datafifo(wlc, DATA_BLOCK_TXCHAIN, 0);
				WL_MODE_SWITCH(("wl%d: %s: UNBLOCKED DATAFIFO \n", WLCWLUNIT(wlc),
					__FUNCTION__));
			}

			if (WLC_MODESW_ENAB(wlc->pub))
				wlc_modesw_bsscfg_complete_downgrade(wlc->modesw);
#endif /* WL_MODESW */
		}
		if ((wlc->block_datafifo & DATA_BLOCK_SPATIAL)) {
			wlc_stf_spatialpolicy_set_complete(wlc);
			wlc_block_datafifo(wlc, DATA_BLOCK_SPATIAL, 0);
		}
#endif /* WL11N */

#ifdef WLC_LOW
#ifdef BCM_DMA_CT
	if (BCM_DMA_CT_ENAB(wlc) && wlc->cfg->pm->PM != PM_OFF) {
		if ((D11REV_IS(wlc->pub->corerev, 65)) &&
			(pktq_empty(WLC_GET_TXQ(wlc->cfg->wlcif->qi)) &&
			(wlc->hw->wake_override & WLC_WAKE_OVERRIDE_CLKCTL)))
			wlc_ucode_wake_override_clear(wlc->hw, WLC_WAKE_OVERRIDE_CLKCTL);
	} else
#endif /* BCM_DMA_CT */
	{
		/*
		 * PR 113378: Checking for the PM wake override bit before calling the override
		 * PR 107865: DRQ output should be latched before being written to DDQ.
		 * If awake, try to sleep.
		 * XXX: 1) need to check whether the pkts in other queue's are empty.
		 * XXX: 2) currently only for primary STA interface
		 */
		if (((D11REV_IS(wlc->pub->corerev, 41)) || (D11REV_IS(wlc->pub->corerev, 44))) &&
		    (pktq_empty(WLC_GET_TXQ(wlc->cfg->wlcif->qi)) &&
			(wlc->hw->wake_override & WLC_WAKE_OVERRIDE_4335_PMWAR)))
			wlc_ucode_wake_override_clear(wlc->hw, WLC_WAKE_OVERRIDE_4335_PMWAR);
	}
#endif /* WLC_LOW */
	}

	/* Clear MHF2_TXBCMC_NOW flag if BCMC fifo has drained */
	if (AP_ENAB(wlc->pub) &&
	    wlc->bcmcfifo_drain && TXPKTPENDGET(wlc, TX_BCMC_FIFO) == 0) {
		wlc->bcmcfifo_drain = FALSE;
		wlc_mhf(wlc, MHF2, MHF2_TXBCMC_NOW, 0, WLC_BAND_AUTO);
	}

#ifdef STA
	/* figure out which bsscfg is being worked on... */
	/* XXX mSTA: get the bsscfg that is currently associating...we need to
	 * revisit how we find the bsscfg that is doing TX_DRAIN when simultaneous
	 * association is supported.
	 */

	cfg = wlc->as_info->assoc_req[0];
	if (cfg == NULL)
		return;

	if (cfg->assoc->state == AS_WAIT_TX_DRAIN &&
	    pktq_empty(WLC_GET_TXQ(cfg->wlcif->qi))&&
	    TXPKTPENDTOT(wlc) == 0) {
		wlc_join_BSS(cfg,
			wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last]);
	}
#endif	/* STA */
}

#if defined(TXQ_MUX)
void BCMFASTPATH
wlc_txfifo_complete(wlc_info_t *wlc, uint fifo, uint16 txpktpend, int txpkttime)
#else
void BCMFASTPATH
wlc_txfifo_complete(wlc_info_t *wlc, uint fifo, uint16 txpktpend)
#endif /* TXQ_MUX */
{
#ifdef NEW_TXQ
	txq_t *txq;
#endif /* NEW_TXQ */

	TXPKTPENDDEC(wlc, fifo, txpktpend);
	WL_TRACE(("wlc_txfifo_complete, pktpend dec %d to %d\n", txpktpend,
		TXPKTPENDGET(wlc, fifo)));

#ifdef NEW_TXQ
	/* track time per fifo */
	txq = (wlc->txfifo_detach_transition_queue != NULL) ?
		wlc->txfifo_detach_transition_queue->low_txq : wlc->active_queue->low_txq;

#if defined(TXQ_MUX)
	wlc_txq_complete(wlc->txqi, txq, fifo, txpktpend, txpkttime);
#else
	/* XXX:
	 * Fake TX time for low TXQ only (no MUX) implementation
	 */
	wlc_txq_complete(wlc->txqi, txq, fifo, txpktpend, txpktpend);
#endif // endif

#else /* NEW_TXQ */
	/* There is more room; mark precedences related to this FIFO sendable */

	/* WES: feedback goes to low txq. No need to update tx_prec_map with WLC_TX_FIFO_ENAB() */
	WLC_TX_FIFO_ENAB(wlc, fifo);
#endif /* NEW_TXQ */

	ASSERT(TXPKTPENDGET(wlc, fifo) >= 0);

	/* Check the change to open datafifo */
	wlc_tx_open_datafifo(wlc);
}

/* create the d11 hardware txhdr for an MPDU packet that does not have a txhdr.
 * The packet begins with a tx_params structure used to supply some
 * parameters to wlc_d11hdrs()
 */
static void
wlc_pdu_txhdr(wlc_info_t *wlc, void *p, struct scb *scb, wlc_txh_info_t *txh_info)
{
	wlc_pdu_tx_params_t tx_params;
	wlc_key_info_t key_info;

	/* pull off the saved tx params */
	memcpy(&tx_params, PKTDATA(wlc->osh, p), sizeof(wlc_pdu_tx_params_t));
	PKTPULL(wlc->osh, p, sizeof(wlc_pdu_tx_params_t));

	/* add headers */
	wlc_key_get_info(tx_params.key, &key_info);
	wlc_d11hdrs(wlc, p, scb, tx_params.flags, 0, 1, tx_params.fifo, 0,
	            &key_info, tx_params.rspec_override, NULL);

	wlc_get_txh_info(wlc, p, txh_info);

	/* management frame protection */
	if (key_info.algo != CRYPTO_ALGO_OFF) {
		uint d11h_off;
		int err;

		d11h_off = (uint)((uint8 *)txh_info->d11HdrPtr - (uint8 *)PKTDATA(wlc->osh, p));

		PKTPULL(wlc->osh, p, d11h_off);
		err  = wlc_key_prep_tx_mpdu(tx_params.key, p, txh_info->hdrPtr);
		if (err != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_key_prep_tx_mpdu failed with status %d\n",
				wlc->pub->unit, __FUNCTION__, err));
		}
		PKTPUSH(wlc->osh, p, d11h_off);
	}
}

/* prepares pdu for transmission. returns BCM error codes */
int
wlc_prep_pdu(wlc_info_t *wlc, struct scb *scb, void *pdu, uint *fifop)
{
#if !defined(NEW_TXQ)
	uint nbytes;
#endif /* NEW_TXQ */
	uint fifo;
	wlc_txh_info_t txh_info;
	wlc_bsscfg_t *cfg;
	wlc_pkttag_t *pkttag;

	WL_TRACE(("wl%d: %s()\n", wlc->pub->unit, __FUNCTION__));

	pkttag = WLPKTTAG(pdu);
	ASSERT(pkttag != NULL);

	/* Make sure that it's a PDU */
	ASSERT(pkttag->flags & WLF_MPDU);

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	BCM_REFERENCE(cfg);
	ASSERT(cfg != NULL);

#ifndef TXQ_MUX
	/* The MBSS BCMC PS work checks to see if
	 * bcmc pkts need to go on a bcmc BSSCFG PS queue,
	 * what is being added here *is* the bcmc bsscfg queue with TXQ_MUX.
	 */
	if (SCB_ISMULTI(scb) && bcmc_pkt_q_check(wlc, scb, pdu)) {
		/* Does BCMC pkt need to go to BSS's PS queue? */
		/* Force caller to give up packet and not tx */
		return BCME_NOTREADY;
	}
#endif // endif

	/* Drop the frame if it's not on the current band */
	/* Note: We drop it instead of returning as this can result in head-of-line
	 * blocking for Probe request frames during the scan
	 */
	if (scb->bandunit != CHSPEC_WLCBANDUNIT(WLC_BAND_PI_RADIO_CHANSPEC)) {
		PKTFREE(wlc->osh, pdu, TRUE);
		WLCNTINCR(wlc->pub->_cnt->txchanrej);
		return BCME_BADBAND;
	}

	/* Something is blocking data packets */
	if (wlc->block_datafifo & ~(DATA_BLOCK_JOIN | DATA_BLOCK_SCAN)) {
		if (wlc->block_datafifo & (DATA_BLOCK_TX_SUPR |
			DATA_BLOCK_TXCHAIN | DATA_BLOCK_SPATIAL | DATA_BLOCK_MUTX)) {
			WL_ERROR(("wl%d: %s: block_datafifo 0x%x\n",
				wlc->pub->unit, __FUNCTION__, wlc->block_datafifo));
		}
		return BCME_BUSY;
	}

	/* add the txhdr if not present */
	if ((pkttag->flags & WLF_TXHDR) == 0)
		wlc_pdu_txhdr(wlc, pdu, scb, &txh_info);
	else
		wlc_get_txh_info(wlc, pdu, &txh_info);

	/*
	 * If the STA is in PS mode, this must be a PS Poll response or APSD delivery frame;
	 * fix the MPDU accordingly.
	 */
	if ((BSSCFG_AP(cfg) || BSS_TDLS_ENAB(wlc, cfg)) && SCB_PS(scb)) {
		/* TxFrameID can be updated for multi-cast packets */
		wlc_apps_ps_prep_mpdu(wlc, pdu, &txh_info);
	}

#if defined(WLAWDL) && defined(AWDL_FAMILY)
	if (pkttag->flags & WLF_FIFOPKT) {
		WL_AMPDU(("%s(): fix pkt seq 0x%04x txhdr chanspec to 0x%04x\n",
			__FUNCTION__, pkttag->seq, WLC_BAND_PI_RADIO_CHANSPEC));
		wlc_txh_set_chanspec(wlc, &txh_info, WLC_BAND_PI_RADIO_CHANSPEC);
	}
#endif // endif

	/* get the pkt queue info. This was put at wlc_sendctl or wlc_send for PDU */
	fifo = WLC_TXFID_GET_QUEUE(ltoh16(txh_info.TxFrameID));
	*fifop = fifo;

#if !defined(NEW_TXQ)
/* WES: low txq now handles the direct queuing to the DMA ring or PIO */
	if (!PIO_ENAB(wlc->pub)) {
		uint ndesc;
		/* calc the number of descriptors needed to queue this frame */

		/* for unfragmented packets, count the number of packet buffer
		 * segments, each being a contiguous virtual address range.
		 */
		ndesc = pktsegcnt(wlc->osh, pdu);

		if (wlc->dma_avoidance_war)
			ndesc *= 2;
		/* return if insufficient dma resources */
		if ((uint)TXAVAIL(wlc, fifo) <= ndesc) {
			/* Mark precedences related to this FIFO, unsendable */
			WLC_TX_FIFO_CLEAR(wlc, fifo);
			return BCME_BUSY;
		}
	} else {
		nbytes = pkttotlen(wlc->osh, pdu);
		BCM_REFERENCE(nbytes);

		/* return if insufficient pio resources */
		if (!wlc_pio_txavailable(WLC_HW_PIO(wlc, fifo), nbytes, 1)) {
			/* Mark precedences related to this FIFO, unsendable */
			WLC_TX_FIFO_CLEAR(wlc, fifo);
			return BCME_BUSY;
		}
	}
#endif /* !NEW_TXQ */

	if (ltoh16(((struct dot11_header *)(txh_info.d11HdrPtr))->fc) != FC_TYPE_DATA)
		WLCNTINCR(wlc->pub->_cnt->txctl);

	return 0;
}

static void
wlc_update_txpktfail_stats(wlc_info_t *wlc, uint32 pkt_len, uint8 prio)
{
	if (WME_ENAB(wlc->pub)) {
		WLCNTINCR(wlc->pub->_wme_cnt->tx_failed[WME_PRIO2AC(prio)].packets);
		WLCNTADD(wlc->pub->_wme_cnt->tx_failed[WME_PRIO2AC(prio)].bytes, pkt_len);
	}
}

static void BCMFASTPATH
wlc_update_txpktsuccess_stats(wlc_info_t *wlc, struct scb *scb, uint32 pkt_len, uint8 prio)
{
	/* update stat counters */
	WLCNTINCR(wlc->pub->_cnt->txframe);
	WLCNTADD(wlc->pub->_cnt->txbyte, pkt_len);
	WLPWRSAVETXFINCR(wlc);
#ifdef WLLED
	wlc_led_start_activity_timer(wlc->ledh);
#endif // endif

	/* update interface stat counters */
	WLCNTINCR(SCB_WLCIFP(scb)->_cnt.txframe);
	WLCNTADD(SCB_WLCIFP(scb)->_cnt.txbyte, pkt_len);

	if (WME_ENAB(wlc->pub)) {
		WLCNTINCR(wlc->pub->_wme_cnt->tx[WME_PRIO2AC(prio)].packets);
		WLCNTADD(wlc->pub->_wme_cnt->tx[WME_PRIO2AC(prio)].bytes, pkt_len);
	}
}

/*
 * Convert 802.3 MAC header to 802.11 MAC header (data only)
 * and add WEP IV information if enabled.
 */
static struct dot11_header * BCMFASTPATH
wlc_80211hdr(wlc_info_t *wlc, void *p, struct scb *scb,
	bool MoreFrag, const wlc_key_info_t *key_info, uint8 prio, uint16 *pushlen, uint fifo)
{
	struct ether_header *eh;
	struct dot11_header *h;
	struct ether_addr tmpaddr;
	uint16 offset;
	struct ether_addr *ra;
	wlc_pkttag_t *pkttag;
	bool a4;
	osl_t *osh = wlc->osh;
	wlc_bsscfg_t *bsscfg;
	uint16 fc = 0;

	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	osh = wlc->osh;
	BCM_REFERENCE(osh);
	pkttag = WLPKTTAG(p);
	eh = (struct ether_header *) PKTDATA(osh, p);
	/* Only doing A3 frame if 802.3 frame's destination address
	 * matches this SCB's address.
	 */
	if (pkttag->flags & WLF_8021X)
		a4 = SCB_DWDS(scb) ?
		(bcmp(&eh->ether_dhost, &scb->ea, ETHER_ADDR_LEN) != 0) : (SCB_A4_8021X(scb));
	else
		a4 = (SCB_A4_DATA(scb) != 0);
	ra = (a4 ? &scb->ea : NULL);

	/* convert 802.3 header to 802.11 header */
	/* Make room for 802.11 header, add additional bytes if WEP enabled for IV */
	offset = DOT11_A3_HDR_LEN - ETHER_HDR_LEN;
	if (a4)
		offset += ETHER_ADDR_LEN;
	if (SCB_QOS(scb))
		offset += DOT11_QOS_LEN;

	ASSERT(key_info != NULL);

	if (!WLC_KEY_IS_LINUX_CRYPTO(key_info))
		offset += key_info->iv_len;
	h = (struct dot11_header *) PKTPUSH(osh, p, offset);
	bzero((char*)h, offset);

	*pushlen = offset + ETHER_HDR_LEN;

	if (a4) {
		ASSERT(ra != NULL);
		/* WDS: a1 = RA, a2 = TA, a3 = DA, a4 = SA, ToDS = 0, FromDS = 1 */
		bcopy((char*)ra, (char*)&h->a1, ETHER_ADDR_LEN);
		bcopy((char*)&bsscfg->cur_etheraddr, (char*)&h->a2, ETHER_ADDR_LEN);
		/* eh->ether_dhost and h->a3 may overlap */
		if (key_info->algo != CRYPTO_ALGO_OFF || SCB_QOS(scb)) {
			/* In WEP case, &h->a3 + 4 = &eh->ether_dhost due to IV offset, thus
			 * need to bcopy
			 */
			bcopy((char*)&eh->ether_dhost, (char*)&tmpaddr, ETHER_ADDR_LEN);
			bcopy((char*)&tmpaddr, (char*)&h->a3, ETHER_ADDR_LEN);
		}
		/* eh->ether_shost and h->a4 may overlap */
		bcopy((char*)&eh->ether_shost, (char*)&tmpaddr, ETHER_ADDR_LEN);
		bcopy((char*)&tmpaddr, (char*)&h->a4, ETHER_ADDR_LEN);
		fc |= FC_TODS | FC_FROMDS;

		/* A-MSDU: use BSSID for A3 and A4, only need to fix up A3 */
		if (WLPKTFLAG_AMSDU(pkttag))
			bcopy((char*)&bsscfg->BSSID, (char*)&h->a3, ETHER_ADDR_LEN);
	}
	else if (BSSCFG_AP(bsscfg)) {
		ASSERT(ra == NULL);
		/* AP: a1 = DA, a2 = BSSID, a3 = SA, ToDS = 0, FromDS = 1 */
		bcopy(eh->ether_dhost, h->a1.octet, ETHER_ADDR_LEN);
		bcopy(bsscfg->BSSID.octet, h->a2.octet, ETHER_ADDR_LEN);
		/* eh->ether_shost and h->a3 may overlap */
		if (key_info->algo != CRYPTO_ALGO_OFF || SCB_QOS(scb)) {
			/* In WEP case, &h->a3 + 4 = &eh->ether_shost due to IV offset,
			 * thus need to bcopy
			 */
			bcopy((char*)&eh->ether_shost, (char*)&tmpaddr, ETHER_ADDR_LEN);
			bcopy((char*)&tmpaddr, (char*)&h->a3, ETHER_ADDR_LEN);
		}
		fc |= FC_FROMDS;
	} else {
		ASSERT(ra == NULL);
		if (bsscfg->BSS) {
			/* BSS STA: a1 = BSSID, a2 = SA, a3 = DA, ToDS = 1, FromDS = 0 */
			bcopy((char*)&bsscfg->BSSID, (char*)&h->a1, ETHER_ADDR_LEN);
			bcopy((char*)&eh->ether_dhost, (char*)&tmpaddr, ETHER_ADDR_LEN);
			bcopy((char*)&eh->ether_shost, (char*)&h->a2, ETHER_ADDR_LEN);
			bcopy((char*)&tmpaddr, (char*)&h->a3, ETHER_ADDR_LEN);
			fc |= FC_TODS;
		} else {
			/* IBSS/TDLS STA: a1 = DA, a2 = SA, a3 = BSSID, ToDS = 0, FromDS = 0 */
			bcopy((char*)&eh->ether_dhost, (char*)&h->a1, ETHER_ADDR_LEN);
			bcopy((char*)&eh->ether_shost, (char*)&h->a2, ETHER_ADDR_LEN);
			bcopy((char*)&bsscfg->BSSID, (char*)&h->a3, ETHER_ADDR_LEN);
		}
	}

	/* SCB_QOS: Fill QoS Control Field */
	if (SCB_QOS(scb)) {
		uint16 qos, *pqos;
		wlc_wme_t *wme = bsscfg->wme;

		/* Set fragment priority */
		qos = (prio << QOS_PRIO_SHIFT) & QOS_PRIO_MASK;

		/* Set the ack policy; AMPDU overrides wme_noack */
		if (WLPKTFLAG_AMPDU(pkttag))
			qos |= (QOS_ACK_NORMAL_ACK << QOS_ACK_SHIFT) & QOS_ACK_MASK;
		else if (wme->wme_noack == QOS_ACK_NO_ACK) {
			WLPKTTAG(p)->flags |= WLF_WME_NOACK;
			qos |= (QOS_ACK_NO_ACK << QOS_ACK_SHIFT) & QOS_ACK_MASK;
		}
		else
			qos |= (wme->wme_noack << QOS_ACK_SHIFT) & QOS_ACK_MASK;

		/* Set the A-MSDU bit for AMSDU packet */
		if (WLPKTFLAG_AMSDU(pkttag))
			qos |= (1 << QOS_AMSDU_SHIFT) & QOS_AMSDU_MASK;

		pqos = (uint16 *)((uchar *)h + (a4 ? DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN));
		ASSERT(ISALIGNED(pqos, sizeof(*pqos)));

		*pqos = htol16(qos);

		/* Set subtype to QoS Data */
		fc |= (FC_SUBTYPE_QOS_DATA << FC_SUBTYPE_SHIFT);
	}

	fc |= (FC_TYPE_DATA << FC_TYPE_SHIFT);

	/* Set MoreFrag, WEP, and Order fc fields */
	if (MoreFrag)
		fc |= FC_MOREFRAG;
	if (key_info->algo != CRYPTO_ALGO_OFF)
		fc |= FC_WEP;

	h->fc = htol16(fc);

	return h;
}

/* alloc fragment buf and fill with data from msdu input packet */
static void *
wlc_allocfrag(osl_t *osh, void *sdu, uint offset, uint headroom, uint frag_length, uint tailroom)
{
	void *p1;
	uint plen;
	uint totlen = pkttotlen(osh, sdu);

	/* In TKIP, (offset >= pkttotlen(sdu)) is possible due to MIC. */
	if (offset >= totlen) {
		plen = 0; /* all sdu has been consumed */
	} else {
		plen = MIN(frag_length, totlen - offset);
	}
	/* one-copy: alloc fragment buffers and fill with data from msdu input pkt */

	/* XXX: this must be a copy until we fix up our PKTXXX macros to handle
	 * non contiguous pkts.
	 */

	/* alloc a new pktbuf */
	if ((p1 = PKTGET(osh, headroom + tailroom + plen, TRUE)) == NULL)
		return (NULL);
	PKTPULL(osh, p1, headroom);

	/* copy frags worth of data at offset into pktbuf */
	pktcopy(osh, sdu, offset, plen, (uchar*)PKTDATA(osh, p1));
	PKTSETLEN(osh, p1, plen);

	/* Currently our PKTXXX macros only handle contiguous pkts. */
	ASSERT(!PKTNEXT(osh, p1));

	return (p1);
}

#if defined(BCMPCIEDEV) && defined(BCMFRAGPOOL)
static void *
wlc_allocfrag_txfrag(osl_t *osh, void *sdu, uint offset, uint frag_length,
	bool lastfrag)
{
	void *p1;
	uint plen;
	/* Len remaining to fill in the fragments */
	uint lenToFill = pkttotlen(osh, sdu) - offset;

	/* Currently TKIP is not handled */
	plen = MIN(frag_length, lenToFill);

	/* Cut out ETHER_HDR_LEN as frag_data does not account for that */
	offset -= ETHER_HDR_LEN;

	/* Need 202 bytes of headroom for TXOFF, 22 bytes for amsdu path */
	/* TXOFF + amsdu headroom */
#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	if (BCMDHDHDR_ENAB()) {
		p1 = pktpool_lfrag_get(SHARED_FRAG_POOL, D11_LFRAG_BUF_POOL);
	} else
#endif /* BCM_DHDHDR && DONGLEBUILD */
	{
		p1 = pktpool_lfrag_get(SHARED_FRAG_POOL, NULL);
	}
	if (p1 == NULL)
		return (NULL);

	PKTPULL(osh, p1, 224);
	PKTSETLEN(osh, p1, 0);

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	/* The sdu packet it has ETH 14B in DNG and 8B LSH in BCM_DHDHDR.
	 * The first fragment pkts[0], it must have 8B LSH in BCM_DHDHDR as well
	 * we don't generate it in DNG, so pkt[0] has same data address as sdu.
	 * So don't PKTPUSH DOT11_LLC_SNAP_HDR_LEN in DNG and don't subtract
	 * DOT11_LLC_SNAP_HDR_LEN from plen.
	 * So for BCM_DHDHDR we get new txlfrag with D11_BUFFER and setup HOST
	 * data_address len .etc for each fragment pkt.  Now in this function each pkts[]'s
	 * length is 0, the caller will push ETH 14B and copy it from sdu to each pkts[].
	 */
#endif /* BCM_DHDHDR && DONGLEBUILD */
	if (!BCMDHDHDR_ENAB()) {
		/* If first fragment, copy LLC/SNAP header
		 * Host fragment length in this case, becomes plen - DOT11_LLC_SNAP hdr len
		 */
		if (offset == 0) {
			PKTPUSH(osh, p1, DOT11_LLC_SNAP_HDR_LEN);
			bcopy((uint8*)(PKTDATA(osh, sdu) + ETHER_HDR_LEN),
				PKTDATA(osh, p1), DOT11_LLC_SNAP_HDR_LEN);
			plen -= DOT11_LLC_SNAP_HDR_LEN;
		} else {
			/* For fragments other than the first fragment, deduct DOT11_LLC_SNAP len,
			 * as that len should not be offset from Host data low address
			 */
			offset -= DOT11_LLC_SNAP_HDR_LEN;
		}
	}

	/* Set calculated address offsets for Host data */
	PKTSETFRAGDATA_HI(osh, p1, 1, PKTFRAGDATA_HI(osh, sdu, 1));
	PKTSETFRAGDATA_LO(osh, p1, 1, PKTFRAGDATA_LO(osh, sdu, 1) + offset);
	PKTSETFRAGTOTNUM(osh, p1, 1);
	PKTSETFRAGLEN(osh, p1, 1, plen);
	PKTSETFRAGTOTLEN(osh, p1, plen);
	PKTSETIFINDEX(osh, p1, PKTIFINDEX(osh, sdu));

#ifdef PROP_TXSTATUS
	/* If incoming sdu is from host, set appropriate flags for new frags too */
	if (WL_TXSTATUS_GET_FLAGS(WLPKTTAG(sdu)->wl_hdr_information) &
		WLFC_PKTFLAG_PKTFROMHOST) {

		WL_TXSTATUS_SET_FLAGS(WLPKTTAG(p1)->wl_hdr_information,
			WLFC_PKTFLAG_PKTFROMHOST);
	}
#endif /* PROP_TXSTATUS */

	/* Only last fragment should have metadata
	 * and valid PKTID. Reset metadata and set invalid PKTID
	 * for other fragments
	 */
	if (lastfrag) {
		PKTSETFRAGPKTID(osh, p1, PKTFRAGPKTID(osh, sdu));
		PKTSETFRAGFLOWRINGID(osh, p1, PKTFRAGFLOWRINGID(osh, sdu));
		PKTFRAGSETRINGINDEX(osh, p1, PKTFRAGRINGINDEX(osh, sdu));
		PKTSETFRAGMETADATA_HI(osh, p1, PKTFRAGMETADATA_HI(osh, sdu));
		PKTSETFRAGMETADATA_LO(osh, p1, PKTFRAGMETADATA_LO(osh, sdu));
		PKTSETFRAGMETADATALEN(osh, p1,  PKTFRAGMETADATALEN(osh, sdu));
		PKTSETHASMETADATA(osh, p1);
	} else {
		PKTRESETHASMETADATA(osh, p1);
		PKTSETFRAGPKTID(osh, p1, 0xdeadbeaf);
	}

	return (p1);
}
#endif	/* BCMPCIEDEV */

/* construct one fragment */
static void BCMFASTPATH
wlc_dofrag(wlc_info_t *wlc, void *p, uint frag, uint nfrags,
	uint next_payload_len, struct scb *scb, bool is8021x, uint fifo,
	wlc_key_t *key, const wlc_key_info_t *key_info,
	uint8 prio, uint frag_length)
{
	struct dot11_header *h = NULL;
	uint next_frag_len;
	uint16 frameid, txc_hdr_len = 0;
	uint16 txh_off = 0;
	wlc_bsscfg_t *cfg;
	uint16 d11hdr_len = 0;
	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

#ifdef BCMDBG
	{
	char eabuf[ETHER_ADDR_STR_LEN];
	WL_APSTA_TX(("wl%d.%d: wlc_dofrag: send %p to %s, cfg %p, fifo %d, frag %d, nfrags %d\n",
	             wlc->pub->unit, WLC_BSSCFG_IDX(cfg), p,
	             bcm_ether_ntoa(&scb->ea, eabuf), cfg, fifo, frag, nfrags));
	}
#endif /* BCMDBG */

	/*
	 * 802.3 (header length = 22):
	 *                     (LLC includes ether_type in last 2 bytes):
	 * ----------------------------------------------------------------------------------------
	 * |                                      |   DA   |   SA   | L | LLC/SNAP | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *                                             6        6     2       6      2
	 *
	 * NON-WDS
	 *
	 * Conversion to 802.11 (header length = 32):
	 * ----------------------------------------------------------------------------------------
	 * |              | FC | D |   A1   |   A2   |   A3   | S | QoS | LLC/SNAP | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *                   2   2      6        6        6     2    2        6      2
	 *
	 * Conversion to 802.11 (WEP, QoS):
	 * ----------------------------------------------------------------------------------------
	 * |         | FC | D |   A1   |   A2   |   A3   | S | QoS | IV | LLC/SNAP | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *             2   2      6        6        6     2    2     4        6      2
	 *
	 * WDS
	 *
	 * Conversion to 802.11 (header length = 38):
	 * ----------------------------------------------------------------------------------------
	 * |     | FC | D |   A1   |   A2   |   A3   | S |   A4   | QoS | LLC/SNAP | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *         2   2      6        6        6     2      6      2         6      2
	 *
	 * Conversion to 802.11 (WEP, QoS):
	 * ----------------------------------------------------------------------------------------
	 * | FC | D |   A1   |   A2   |   A3   | S |   A4   |  QoS | IV | LLC/SNAP | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *    2   2      6        6        6     2      6       2    4        6      2
	 *
	 */

	WL_WSEC(("wl%d.%d: %s: tx sdu, len %d(with 802.3 hdr)\n",
	         wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pkttotlen(wlc->osh, p)));

	ASSERT(key != NULL && key_info != NULL);

	/*
	 * Convert 802.3 MAC header to 802.11 MAC header (data only)
	 * and add WEP IV information if enabled.
	 *  requires valid DA and SA, does not care ether_type
	 */
	if (!BSSCFG_SAFEMODE(cfg))
		h = wlc_80211hdr(wlc, p, scb, (bool)(frag != (nfrags - 1)),
			key_info, prio, &d11hdr_len, fifo);

	WL_WSEC(("wl%d.%d: %s: tx sdu, len %d(w 80211hdr and iv, no d11hdr, icv and fcs)\n",
	         wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, pkttotlen(wlc->osh, p)));

	/* determine total MPDU length of next frag */
	next_frag_len = next_payload_len;
	if (next_frag_len > 0) {	/* there is a following frag */
		next_frag_len += DOT11_A3_HDR_LEN + DOT11_FCS_LEN;
		/* A4 header */
		if (SCB_A4_DATA(scb))
			next_frag_len += ETHER_ADDR_LEN;
		/* SCB_QOS: account for QoS Control Field */
		if (SCB_QOS(scb))
			next_frag_len += DOT11_QOS_LEN;
		if (!WLC_KEY_IS_LINUX_CRYPTO(key_info))
			next_frag_len += key_info->iv_len + key_info->icv_len;
	}

	/* add d11 headers */
	frameid = wlc_d11hdrs(wlc, p, scb,
		(WLC_PROT_CFG_SHORTPREAMBLE(wlc->prot, cfg) &&
	         (scb->flags & SCB_SHORTPREAMBLE) != 0),
		frag, nfrags, fifo, next_frag_len, key_info, 0, &txh_off);
	BCM_REFERENCE(frameid);

	txc_hdr_len = d11hdr_len + (txh_off + D11_TXH_LEN_EX(wlc));

#ifdef STA
	/*
	 * TKIP countermeasures: register pkt callback for last frag of
	 * MIC failure notification
	 *
	 * XXX - This only checks for the next 802.1x frame; how can we make
	 * sure this is the MIC failure notification?
	 */
	if (BSSCFG_STA(cfg) && next_frag_len == 0 && is8021x) {
		bool tkip_cm;
		tkip_cm = wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, cfg);
		if (tkip_cm) {
			WL_WSEC(("wl%d.%d: %s: TKIP countermeasures: sending MIC failure"
				" report...\n", WLCWLUNIT(wlc),
				WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			if (frameid == 0xffff) {
				WL_ERROR(("wl%d.%d: %s: could not register MIC failure packet"
					" callback\n", WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg),
					__FUNCTION__));
			}
			else {
				/* register packet callback */
				WLF2_PCB1_REG(p, WLF2_PCB1_TKIP_CM);
			}
		}
	}
#endif /* STA */

	/* 802.3 header needs to be included in WEP-encrypted portion of frame, and
	 * we need the IV info in the frame, so we're forced to do WEP here
	 */
	{
	uint d11h_off = txh_off + D11_TXH_LEN_EX(wlc);
	wlc_txh_info_t txh_info;

	wlc_get_txh_info(wlc, p, &txh_info);
	PKTPULL(wlc->osh, p, d11h_off);
	if (wlc_key_prep_tx_mpdu(key, p, (wlc_txd_t *)txh_info.hdrPtr) != BCME_OK)
		WL_WSEC(("wl%d: %s: wlc_key_prep_tx_mpdu error\n",
		         wlc->pub->unit, __FUNCTION__));
	PKTPUSH(wlc->osh, p, d11h_off);
	}

	if (BSSCFG_AP(cfg) ||
	    BSS_TDLS_BUFFER_STA(cfg)) {
		/* Processing to set MoreData when needed */
		if (fifo != TX_BCMC_FIFO) {
			/* Check if packet is being sent to STA in PS mode */
			if (SCB_PS(scb)) {
				bool last_frag;

				last_frag = (frag == nfrags - 1);
				/* Make preparations for APSD delivery frame or PS-Poll response */
				if (AC_BITMAP_TST(scb->apsd.ac_delv, WME_PRIO2AC(PKTPRIO(p))))
					wlc_apps_apsd_prepare(wlc, scb, p, h, last_frag);
				else
					wlc_apps_pspoll_resp_prepare(wlc, scb, p, h, last_frag);
				BCM_REFERENCE(last_frag);
			}
		} else {
			/* The uCode clears the MoreData field of the last bcmc pkt per dtim period
			 */
			/* Suppress setting MoreData if we have to support legacy AES.
			 * This should probably also be conditional on at least one legacy
			 * STA associated
			 */
			/*
			* Note: We currently use global WPA_auth for WDS and per scb WPA_auth
			* for others, but fortunately there is no bcmc frames over WDS link
			* therefore we don't need to worry about WDS and it is safe to use per
			* scb WPA_auth only here!
			*/
			/* Also look at wlc_apps_ps_prep_mpdu if following condition ever changes */
			wlc_key_info_t bss_key_info;
			wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, cfg, FALSE, &bss_key_info);
			if (!bcmwpa_is_wpa_auth(cfg->WPA_auth) ||
				(bss_key_info.algo != CRYPTO_ALGO_AES_CCM)) {
				h->fc |= htol16(FC_MOREDATA);
			}
		}
	}

	/* dont cache 8021x frames in case of DWDS */
	if (is8021x && SCB_A4_DATA(scb) && !SCB_A4_8021X(scb))
		return;

	/* install new header into tx header cache: starting from the bytes after DA-SA-L  */
	if (WLC_TXC_ENAB(wlc)) {
		wlc_txc_info_t *txc = wlc->txc;

		if (TXC_CACHE_ENAB(txc) && (nfrags == 1) &&
		    !(WLPKTTAG(p)->flags & WLF_BYPASS_TXC) &&
		    !BSSCFG_SAFEMODE(cfg)) {
			wlc_cac_update_dur_cache(wlc->cac, WME_PRIO2AC(PKTPRIO(p)), PKTPRIO(p),
					scb, 0, 0, WLC_CAC_DUR_CACHE_REFRESH);
			wlc_txc_add(txc, scb, p, txc_hdr_len, fifo, prio, txh_off, d11hdr_len);
		}
	}
}

#ifdef EXT_STA
static void
wlc_ethaddr_from_d11hdr(struct dot11_header *d11hdr, struct ether_header *ethdr)
{
	uint16 fc;
	struct ether_addr a1, a2, a3;

	fc = ltoh16(d11hdr->fc);

	ASSERT(FC_TYPE(fc) == FC_TYPE_DATA);

	bcopy((char *)&d11hdr->a1, (char *)&a1, ETHER_ADDR_LEN);
	bcopy((char *)&d11hdr->a2, (char *)&a2, ETHER_ADDR_LEN);
	bcopy((char *)&d11hdr->a3, (char *)&a3, ETHER_ADDR_LEN);

	if ((fc & FC_TODS) && !(fc & FC_FROMDS)) {
		/* infrastructure */
		bcopy((char*)&a3, (char*)&ethdr->ether_dhost, ETHER_ADDR_LEN);
		bcopy((char*)&a2, (char*)&ethdr->ether_shost, ETHER_ADDR_LEN);
	} else if (!(fc & FC_TODS) && (fc & FC_FROMDS)) {
		/* AP pkt */
		bcopy((char*)&a1, (char*)&ethdr->ether_dhost, ETHER_ADDR_LEN);
		bcopy((char*)&a3, (char*)&ethdr->ether_shost, ETHER_ADDR_LEN);
	} else {
		/* adhoc */
		ASSERT(!(fc & FC_TODS) && !(fc & FC_FROMDS));
		bcopy((char *)&a1, (char *)&ethdr->ether_dhost, ETHER_ADDR_LEN);
		bcopy((char*)&a2, (char*)&ethdr->ether_shost, ETHER_ADDR_LEN);
	}

	return;
}
static void *
wlc_hdr_proc_safemode(wlc_info_t *wlc, void *sdu)
{
	osl_t *osh;
	void *pkt;
	int prio;
	void *pkt_h;

	osh = wlc->osh;

	/* allocate enough room once for all cases */
	prio = PKTPRIO(sdu);
	if ((uint)PKTHEADROOM(osh, sdu) < TXOFF || PKTSHARED(sdu)) {
		if ((pkt = PKTGET(osh, TXOFF, TRUE)) == NULL) {
			WL_ERROR(("wl%d: wlc_hdr_proc, PKTGET headroom %d failed\n",
				wlc->pub->unit, TXOFF));
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
			return NULL;
		}
		PKTPULL(osh, pkt, TXOFF);

		wlc_pkttag_info_move(wlc, sdu, pkt);
		/* Transfer priority */
		PKTSETPRIO(pkt, prio);

		/* move 802.11 hdr from data buffer to header buffer */
		pkt_h = PKTDATA(osh, sdu);
		PKTPULL(osh, sdu, DOT11_A3_HDR_LEN);
		PKTPUSH(osh, pkt, DOT11_A3_HDR_LEN);
		bcopy((char*)pkt_h, (char*)PKTDATA(osh, pkt), DOT11_A3_HDR_LEN);

		/* chain original sdu onto newly allocated header */
		PKTSETNEXT(osh, pkt, sdu);

		/* copy exempt field for new header */
		if (WLEXTSTA_ENAB(wlc->pub))
			WLPKTFLAG_EXEMPT_SET(WLPKTTAG(pkt), WLPKTFLAG_EXEMPT_GET(WLPKTTAG(sdu)));
		sdu = pkt;
	}
	return sdu;
}
#endif /* EXT_STA */

/* allocate headroom buffer if necessary and convert ether frame to 8023 frame
 * !! WARNING: HND WL driver only supports data frame of type ethernet, 802.1x or 802.3
 */
void * BCMFASTPATH
wlc_hdr_proc(wlc_info_t *wlc, void *sdu, struct scb *scb)
{
	osl_t *osh;
	struct ether_header *eh;
	void *pkt, *phdr;
	int prio, use_phdr;
	uint16 ether_type;
	uint headroom = TXOFF;

	osh = wlc->osh;

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	/* When BCM_DHDHDR enabled dongle uses D3_BUFFER by default,
	* it doesn't have extra headroom to construct 802.3 or 802.11 header.
	* The DHD host driver will prepare the 802.3 header for dongle.
	*/
	if (BCMDHDHDR_ENAB() && PKTISTXFRAG(osh, sdu))
		headroom = 0;
#endif // endif

	/* allocate enough room once for all cases */
	prio = PKTPRIO(sdu);
	phdr = NULL;
	use_phdr = FALSE;
#ifdef WLC_LOW
#ifdef DMATXRC
	use_phdr = DMATXRC_ENAB(wlc->pub) ? TRUE : FALSE;
	if (use_phdr)
		phdr = wlc_phdr_get(wlc);
#endif /* DMATXRC */

#endif /* WLC_LOW */

	if ((uint)PKTHEADROOM(osh, sdu) < headroom || PKTSHARED(sdu) || (use_phdr && phdr)) {
		if (use_phdr && phdr)
			pkt = phdr;
		else
			pkt = PKTGET(osh, TXOFF, TRUE);

		if (pkt == NULL) {
			WL_ERROR(("wl%d: %s, PKTGET headroom %d failed\n",
				wlc->pub->unit, __FUNCTION__, (int)TXOFF));
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
			/* increment interface stats */
			WLCIFCNTINCR(scb, txnobuf);
			WLCNTSCBINCR(scb->scb_stats.tx_failures);
#ifdef WL11K
			wlc_rrm_stat_qos_counter(scb, prio, OFFSETOF(rrm_stat_group_qos_t, txfail));
#endif // endif
			return NULL;
		}

		if (phdr == NULL)
			PKTPULL(osh, pkt, TXOFF);

		wlc_pkttag_info_move(wlc, sdu, pkt);

#ifdef DMATXRC
		if (DMATXRC_ENAB(wlc->pub) && use_phdr && phdr) {
			/* Over-written by wlc_pkttag_info_move */
			WLPKTTAG(pkt)->flags |= WLF_PHDR;
		}
#endif /* DMATXRC */
		/* Transfer priority */
		PKTSETPRIO(pkt, prio);

		/* move ether_hdr from data buffer to header buffer */
		eh = (struct ether_header*) PKTDATA(osh, sdu);
		PKTPULL(osh, sdu, ETHER_HDR_LEN);
		PKTPUSH(osh, pkt, ETHER_HDR_LEN);
		bcopy((char*)eh, (char*)PKTDATA(osh, pkt), ETHER_HDR_LEN);

		/* chain original sdu onto newly allocated header */
		PKTSETNEXT(osh, pkt, sdu);
#ifdef EXT_STA
		/* copy exempt field for new header */
		if (WLEXTSTA_ENAB(wlc->pub))
			WLPKTFLAG_EXEMPT_SET(WLPKTTAG(pkt), WLPKTFLAG_EXEMPT_GET(WLPKTTAG(sdu)));
#endif /* EXT_STA */
#if defined(DMATXRC) && defined(PKTC_TX_DONGLE)
		if (DMATXRC_ENAB(wlc->pub) && PKTC_ENAB(wlc->pub) && PKTLINK(sdu)) {
			PKTSETLINK(pkt, PKTLINK(sdu));
			PKTSETLINK(sdu, NULL);
			PKTSETCHAINED(wlc->osh, pkt);
		}
#endif // endif
		sdu = pkt;
	}

#if defined(EXT_STA)
	/* N.B.: VLAN tag managed by nwifi.sys */
	if (!WLEXTSTA_ENAB(wlc->pub))
#endif /* EXT_STA */
	{
	/*
	 * Optionally add an 802.1Q VLAN tag to convey non-zero priority for
	 * non-WMM associations.
	 */
	eh = (struct ether_header *)PKTDATA(osh, sdu);

	if (prio && !SCB_QOS(scb)) {
		if (headroom != 0 && (wlc->vlan_mode != OFF) &&
			(ntoh16(eh->ether_type) != ETHER_TYPE_8021Q)) {
			struct ethervlan_header *vh;
			struct ether_header da_sa;

			bcopy(eh, &da_sa, VLAN_TAG_OFFSET);
			vh = (struct ethervlan_header *)PKTPUSH(osh, sdu, VLAN_TAG_LEN);
			bcopy(&da_sa, vh, VLAN_TAG_OFFSET);

			vh->vlan_type = hton16(ETHER_TYPE_8021Q);
			vh->vlan_tag = hton16(prio << VLAN_PRI_SHIFT);	/* Priority-only Tag */
		}

	}
	}

	/*
	 * Original Ethernet (header length = 14):
	 * ----------------------------------------------------------------------------------------
	 * |                                                     |   DA   |   SA   | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *                                                            6        6     2
	 *
	 * Conversion to 802.3 (header length = 22):
	 *                     (LLC includes ether_type in last 2 bytes):
	 * ----------------------------------------------------------------------------------------
	 * |                                      |   DA   |   SA   | L | LLC/SNAP | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *                                             6        6     2       6      2
	 */

	eh = (struct ether_header *)PKTDATA(osh, sdu);
	ether_type = ntoh16(eh->ether_type);
	if (ether_type > ETHER_MAX_DATA) {
		if (ether_type == ETHER_TYPE_802_1X) {
			WLPKTTAG(sdu)->flags |= WLF_8021X;
			WLPKTTAG(sdu)->flags |= WLF_BYPASS_TXC;
			/*
			 * send 8021x packets at higher priority than best efforts.
			 * use _VI, as _VO prioroty creates "txop exceeded" error
			 */
			prio = PRIO_8021D_VI;
			PKTSETPRIO(sdu, prio);
		}
#ifdef BCMWAPI_WAI
		else if (ether_type == ETHER_TYPE_WAI) {
			WLPKTTAG(sdu)->flags |= WLF_WAI;
			WLPKTTAG(sdu)->flags |= WLF_BYPASS_TXC;
		}
#endif /* BCMWAPI_WAI */

		/* save original type in pkt tag */
		WLPKTTAG(sdu)->flags |= WLF_NON8023;

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
		/* When BCM_DHDHDR enabled, dongle just need to adjust the host data addr to
		 * include llc snal 8B in host.
		 */
		if (BCMDHDHDR_ENAB() && headroom == 0) {
			/* Don't adjust host data addr if sdu was pktfetched, because llc snap is
			 * already part of pktfetched data.
			 */
			if (PKTFRAGTOTNUM(wlc->osh, sdu)) {
				PKTSETFRAGDATA_LO(wlc->osh, sdu, 1,
					PKTFRAGDATA_LO(wlc->osh, sdu, 1) - DOT11_LLC_SNAP_HDR_LEN);
				PKTSETFRAGLEN(wlc->osh, sdu, 1,
					PKTFRAGLEN(wlc->osh, sdu, 1) + DOT11_LLC_SNAP_HDR_LEN);
				PKTSETFRAGTOTLEN(wlc->osh, sdu,
					PKTFRAGTOTLEN(wlc->osh, sdu) + DOT11_LLC_SNAP_HDR_LEN);
			} else {
				uint16 plen = (uint16)pkttotlen(osh, sdu) - ETHER_HDR_LEN;
				eh->ether_type = hton16(plen);
			}
		} else
#endif /* BCM_DHDHDR && DONGLEBUILD */
		{
			wlc_ether_8023hdr(wlc, osh, eh, sdu);
		}
	}

	return sdu;
}

static INLINE uint
wlc_frag(wlc_info_t *wlc, struct scb *scb, uint8 ac, uint plen, uint *flen)
{
	uint payload, thresh, nfrags, bt_thresh = 0;
	int btc_mode;

	plen -= ETHER_HDR_LEN;

	ASSERT(ac < AC_COUNT);

	thresh = wlc->fragthresh[ac];

	btc_mode = wlc_btc_mode_get(wlc);

	if (IS_BTCX_FULLTDM(btc_mode))
		bt_thresh = wlc_btc_frag_threshold(wlc, scb);

	if (bt_thresh)
		thresh = thresh > bt_thresh ? bt_thresh : thresh;

	/* optimize for non-fragmented case */
	if (plen < (thresh - (DOT11_A4_HDR_LEN + DOT11_QOS_LEN +
		DOT11_FCS_LEN + ETHER_ADDR_LEN))) {
		*flen = plen;
		return (1);
	}

	/* account for 802.11 MAC header */
	thresh -= DOT11_A3_HDR_LEN + DOT11_FCS_LEN;

	/* account for A4 */
	if (SCB_A4_DATA(scb))
		thresh -= ETHER_ADDR_LEN;

	/* SCB_QOS: account for QoS Control Field */
	if (SCB_QOS(scb))
		thresh -= DOT11_QOS_LEN;

	/*
	 * Spec says broadcast and multicast frames are not fragmented.
	 * LLC/SNAP considered part of packet payload.
	 * Fragment length must be even per 9.4 .
	 */
	if ((plen > thresh) && !SCB_ISMULTI(scb)) {
		*flen = payload = thresh & ~1;
		nfrags = (plen + payload - 1) / payload;
	} else {
		*flen = plen;
		nfrags = 1;
	}

	ASSERT(nfrags <= DOT11_MAXNUMFRAGS);

	return (nfrags);
}

/*
 * Add d11txh_t, cck_phy_hdr_t for pre-11ac phy.
 *
 * 'p' data must start with 802.11 MAC header
 * 'p' must allow enough bytes of local headers to be "pushed" onto the packet
 *
 * headroom == D11_PHY_HDR_LEN + D11_TXH_LEN (D11_TXH_LEN is now 104 bytes)
 *
 */
uint16
wlc_d11n_hdrs(wlc_info_t *wlc, void *p, struct scb *scb, uint txparams_flags, uint frag,
	uint nfrags, uint queue, uint next_frag_len,
	const wlc_key_info_t *key_info, ratespec_t rspec_override)
{
	struct dot11_header *h;
	d11txh_t *txh;
	uint8 *plcp, plcp_fallback[D11_PHY_HDR_LEN];
	osl_t *osh;
	int len, phylen, rts_phylen;
	uint16 fc, type, frameid, mch, phyctl, xfts, mainrates, rate_flag;
	uint16 seq = 0, mcl = 0, status = 0;
	bool use_rts = FALSE;
	bool use_cts = FALSE, rspec_history = FALSE;
	bool use_rifs = FALSE;
	bool short_preamble;
	uint8 preamble_type = WLC_LONG_PREAMBLE, fbr_preamble_type = WLC_LONG_PREAMBLE;
	uint8 rts_preamble_type = WLC_LONG_PREAMBLE, rts_fbr_preamble_type = WLC_LONG_PREAMBLE;
	uint8 *rts_plcp, rts_plcp_fallback[D11_PHY_HDR_LEN];
	struct dot11_rts_frame *rts = NULL;
	ratespec_t rts_rspec = 0, rts_rspec_fallback = 0;
	bool qos, a4;
	uint8 ac;
	uint rate;
	wlc_pkttag_t *pkttag;
	ratespec_t rspec, rspec_fallback;
	ratesel_txparams_t cur_rate;
#if defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED))
	uint16 phyctl_bwo = -1;
#endif /* (BCMDBG) || (WLTEST) */
#if WL_HT_TXBW_OVERRIDE_ENAB
	int8 txbw_override_idx;
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */
#ifdef WL11N
#define ANTCFG_NONE 0xFF
	uint8 antcfg = ANTCFG_NONE;
	uint8 fbantcfg = ANTCFG_NONE;
	uint16 mimoantsel;
	bool use_mimops_rts = FALSE;
	uint phyctl1_stf = 0;
#endif /* WL11N */
	uint16 durid = 0;
	wlc_bsscfg_t *bsscfg;
	bool g_prot;
	int8 n_prot;
	wlc_wme_t *wme;
#ifdef WL_LPC
	uint8 lpc_offset = 0;
#endif // endif
#ifdef WL11N
	uint8 sgi_tx;
	wlc_ht_info_t *hti = wlc->hti;
#endif /* WL11N */
	uint keyinfo_len = 0;

	ASSERT(scb != NULL);
	ASSERT(queue < NFIFO);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	short_preamble = (txparams_flags & WLC_TX_PARAMS_SHORTPRE) != 0;

	g_prot = WLC_PROT_G_CFG_G(wlc->prot_g, bsscfg);
	n_prot = WLC_PROT_N_CFG_N(wlc->prot_n, bsscfg);

	wme = bsscfg->wme;

	osh = wlc->osh;

	/* locate 802.11 MAC header */
	h = (struct dot11_header*) PKTDATA(osh, p);
	pkttag = WLPKTTAG(p);
	fc = ltoh16(h->fc);
	type = FC_TYPE(fc);
	qos = (type == FC_TYPE_DATA && FC_SUBTYPE_ANY_QOS(FC_SUBTYPE(fc)));
	a4 = (fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS);

	/* compute length of frame in bytes for use in PLCP computations
	 * phylen =  packet length + ICV_LEN + FCS_LEN
	 */
	len = pkttotlen(osh, p);
	phylen = len + DOT11_FCS_LEN;

	/* Add room in phylen for the additional bytes for icv, if required.
	 * this is true for both h/w and s/w encryption. The latter only
	 * modifies the pkt length
	 */
	if (key_info != NULL) {
		if (WLC_KEY_IS_MGMT_GROUP(key_info))
			keyinfo_len = WLC_KEY_MMIC_IE_LEN(key_info);
		else
			keyinfo_len = key_info->icv_len;

		/* external crypto adds iv to the pkt, include it in phylen */
		if (WLC_KEY_IS_LINUX_CRYPTO(key_info))
			keyinfo_len += key_info->iv_len;

		if (WLC_KEY_FRAG_HAS_TKIP_MIC(p, key_info, frag, nfrags))
			keyinfo_len += TKIP_MIC_SIZE;
	}
	phylen += keyinfo_len;

	WL_NONE(("wl%d: %s: len %d, phylen %d\n", WLCWLUNIT(wlc), __FUNCTION__, len, phylen));

	/* add PLCP */
	plcp = PKTPUSH(osh, p, D11_PHY_HDR_LEN);

	/* add Broadcom tx descriptor header */
	txh = (d11txh_t*)PKTPUSH(osh, p, D11_TXH_LEN);
	bzero((char*)txh, D11_TXH_LEN);

	/* use preassigned or software seqnum */
#ifdef PROP_TXSTATUS
	if (WL_SEQ_GET_FROMDRV(pkttag->seq)) {
		seq = WL_SEQ_GET_NUM(pkttag->seq);
	} else if (WLPKTFLAG_AMPDU(pkttag)) {
		seq = WL_SEQ_GET_NUM(pkttag->seq);
	}
#else
	if (WLPKTFLAG_AMPDU(pkttag))
		seq = pkttag->seq;
#endif /* PROP_TXSTATUS */
	else if (SCB_QOS(scb) && ((fc & FC_KIND_MASK) == FC_QOS_DATA) && !ETHER_ISMULTI(&h->a1)) {
		seq = SCB_SEQNUM(scb, PKTPRIO(p));
		/* Increment the sequence number only after the last fragment */
		if (frag == (nfrags - 1))
			SCB_SEQNUM(scb, PKTPRIO(p))++;
	}
	/* use h/w seqnum */
	else if (type != FC_TYPE_CTL)
		mcl |= TXC_HWSEQ;

	if (type != FC_TYPE_CTL) {
		seq = (seq << SEQNUM_SHIFT) | (frag & FRAGNUM_MASK);
		h->seq = htol16(seq);
	}

#ifdef PROP_TXSTATUS
	if (WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
		WL_SEQ_SET_NUM(pkttag->seq, ltoh16(h->seq) >> SEQNUM_SHIFT);
		WL_SEQ_SET_FROMFW(pkttag->seq, 1);
	}
#endif /* PROP_TXSTATUS */
	/* setup frameid */
	if (queue == TX_BCMC_FIFO) {
		frameid = bcmc_fid_generate(wlc, bsscfg, txh->TxFrameID);
	} else {
		frameid = (((wlc->counter++) << TXFID_SEQ_SHIFT) & TXFID_SEQ_MASK) |
			WLC_TXFID_SET_QUEUE(queue);
	}

	/* set the ignpmq bit for all pkts tx'd in PS mode and for beacons and for anything
	 * going out from a STA interface.
	 */
	if (SCB_PS(scb) || ((fc & FC_KIND_MASK) == FC_BEACON) || BSSCFG_STA(bsscfg))
		mcl |= TXC_IGNOREPMQ;

	ac = WME_PRIO2AC(PKTPRIO(p));
	/* (1) RATE: determine and validate primary rate and fallback rates */

	if (RSPEC_ACTIVE(rspec_override)) {
		rspec = rspec_fallback = rspec_override;
#ifdef WL11N
		if (WLANTSEL_ENAB(wlc) && !ETHER_ISMULTI(&h->a1)) {
			/* set tx antenna config */
			wlc_antsel_antcfg_get(wlc->asi, FALSE, FALSE, 0, 0,
				&antcfg, &fbantcfg);
		}
#endif /* WL11N */
	}
	/* XXX: set both regular and fb rate for 802.1x as lowest basic
	 * Setting only the fb rate as basic fails for 4321
	 */
	else if ((type == FC_TYPE_MNG) ||
		(type == FC_TYPE_CTL) ||
		(pkttag->flags & WLF_8021X) ||
#ifdef BCMWAPI_WAI
		(pkttag->flags & WLF_WAI) ||
#endif // endif
		(pkttag->flags3 & WLF3_DATA_WOWL_PKT)) {
		if ((fc & FC_KIND_MASK) == FC_PROBE_RESP) {
			rspec = rspec_fallback =
				scb->rateset.rates[scb->basic_rate_indx] & RATE_MASK;
		} else {
			rspec = rspec_fallback = scb->rateset.rates[0] & RATE_MASK;
		}
	}
	else if (RSPEC_ACTIVE(wlc->band->mrspec_override) && ETHER_ISMULTI(&h->a1))
		rspec = rspec_fallback = wlc->band->mrspec_override;
	else if (RSPEC_ACTIVE(wlc->band->rspec_override) && !ETHER_ISMULTI(&h->a1)) {
		rspec = rspec_fallback = wlc->band->rspec_override;
#ifdef WL11N
		if (WLANTSEL_ENAB(wlc)) {
			/* set tx antenna config */
			wlc_antsel_antcfg_get(wlc->asi, FALSE, FALSE, 0, 0,
			&antcfg, &fbantcfg);
		}
#endif /* WL11N */
	} else if (ETHER_ISMULTI(&h->a1) || SCB_INTERNAL(scb))
		rspec = rspec_fallback = scb->rateset.rates[0] & RATE_MASK;
	else {
		/* run rate algorithm for data frame only, a cookie will be deposited in frameid */
		cur_rate.num = 2; /* only need up to 2 for non-11ac */
		cur_rate.ac = ac;
		wlc_scb_ratesel_gettxrate(wlc->wrsi, scb, &frameid, &cur_rate, &rate_flag);
		rspec = cur_rate.rspec[0];
		rspec_fallback = cur_rate.rspec[1];

		if (((scb->flags & SCB_BRCM) == 0) &&
		    (((fc & FC_KIND_MASK) == FC_NULL_DATA) ||
		     ((fc & FC_KIND_MASK) == FC_QOS_NULL))) {
			/* Use RTS/CTS rate for NULL frame */
			rspec = wlc_rspec_to_rts_rspec(bsscfg, rspec, FALSE);
			rspec_fallback = scb->rateset.rates[0] & RATE_MASK;
		} else {
			pkttag->flags |= WLF_RATE_AUTO;

			/* The rate histo is updated only for packets on auto rate. */
			/* perform rate history after txbw has been selected */
			if (frag == 0)
				rspec_history = TRUE;
		}
#ifdef WL11N
		if (rate_flag & RATESEL_VRATE_PROBE)
			WLPKTTAG(p)->flags |= WLF_VRATE_PROBE;

		if (WLANTSEL_ENAB(wlc)) {
			wlc_antsel_antcfg_get(wlc->asi, FALSE, TRUE, cur_rate.antselid[0],
				cur_rate.antselid[1], &antcfg, &fbantcfg);
		}
#endif /* WL11N */

#ifdef WL_LPC
		if (LPC_ENAB(wlc)) {
			/* Query the power offset to be used from LPC */
			lpc_offset = wlc_scb_lpc_getcurrpwr(wlc->wlpci, scb, ac);
		} else {
			/* No Link Power Control. Transmit at nominal power. */
		}
#endif // endif
	}

#ifdef WL11N
	/*
	 * At this point the rspec may not include a valid txbw. Apply HT mimo use rules.
	 *
	 * XXX: The stf mode also needs to be selected. It is unknown at this date (12/10/05)
	 * whether or not mimo auto rate will include mode selection. For now forced rates can
	 * include an stf mode, if none is selected the default is SISO.
	 */
	phyctl1_stf = wlc->stf->ss_opmode;

	if (N_ENAB(wlc->pub)) {
		uint32 mimo_txbw;
		uint8 mimo_preamble_type;

		bool stbc_tx_forced = WLC_IS_STBC_TX_FORCED(wlc);
		bool stbc_ht_scb_auto = WLC_STF_SS_STBC_HT_AUTO(wlc, scb);

		/* apply siso/cdd to single stream mcs's or ofdm if rspec is auto selected */
		if (((IS_MCS(rspec) && IS_SINGLE_STREAM(rspec & RSPEC_RATE_MASK)) ||
		     IS_OFDM(rspec)) &&
		    !(rspec & RSPEC_OVERRIDE_MODE)) {
			rspec &= ~(RSPEC_TXEXP_MASK | RSPEC_STBC);

			/* For SISO MCS use STBC if possible */
			if (IS_MCS(rspec) && (stbc_tx_forced ||
				(RSPEC_ISHT(rspec) && stbc_ht_scb_auto))) {
				ASSERT(WLC_STBC_CAP_PHY(wlc));
				rspec |= RSPEC_STBC;
			} else if (phyctl1_stf == PHY_TXC1_MODE_CDD) {
				rspec |= (1 << RSPEC_TXEXP_SHIFT);
			}
		}

		if (((IS_MCS(rspec_fallback) &&
		      IS_SINGLE_STREAM(rspec_fallback & RSPEC_RATE_MASK)) ||
		     IS_OFDM(rspec_fallback)) &&
		    !(rspec_fallback & RSPEC_OVERRIDE_MODE)) {
			rspec_fallback &= ~(RSPEC_TXEXP_MASK | RSPEC_STBC);

			/* For SISO MCS use STBC if possible */
			if (IS_MCS(rspec_fallback) && (stbc_tx_forced ||
				(RSPEC_ISHT(rspec_fallback) && stbc_ht_scb_auto))) {
				ASSERT(WLC_STBC_CAP_PHY(wlc));
				rspec_fallback |= RSPEC_STBC;
			} else if (phyctl1_stf == PHY_TXC1_MODE_CDD) {
				rspec_fallback |= (1 << RSPEC_TXEXP_SHIFT);
			}
		}

		/* Is the phy configured to use 40MHZ frames? If so then pick the desired txbw */
		if (CHSPEC_IS40(wlc->chanspec)) {
			/* default txbw is 20in40 SB */
			mimo_txbw = RSPEC_BW_20MHZ;

			if (RSPEC_BW(rspec) != RSPEC_BW_UNSPECIFIED) {
				/* If the ratespec override has a bw greater than
				 * 20MHz, specify th txbw value here.
				 * Otherwise, the default setup above is already 20MHz.
				 */
				if (!RSPEC_IS20MHZ(rspec)) {
					/* rspec bw > 20MHz is not allowed for CCK/DSSS
					 * so we can only have OFDM or HT here
					 * mcs 32 and legacy OFDM must be 40b/w DUP, other HT
					 * is just plain 40MHz
					 */
					mimo_txbw = RSPEC_BW(rspec);
				}
			} else if (IS_MCS(rspec)) {
				if ((scb->flags & SCB_IS40) &&
#ifdef WLMCHAN
					 /* if mchan enabled and bsscfg is AP, then must
					  * check the bsscfg chanspec to make sure our AP
					  * is operating on 40MHz channel.
					  */
					 (!MCHAN_ENAB(wlc->pub) || !BSSCFG_AP(bsscfg) ||
					  CHSPEC_IS40(bsscfg->current_bss->chanspec)) &&
#endif /* WLMCHAN */
					 TRUE) {
					mimo_txbw = RSPEC_BW_40MHZ;
				}
			}
#if WL_HT_TXBW_OVERRIDE_ENAB
			WL_HT_TXBW_OVERRIDE_IDX(hti, rspec, txbw_override_idx);

			if (txbw_override_idx >= 0) {
				mimo_txbw = txbw2rspecbw[txbw_override_idx];
				phyctl_bwo = txbw2phyctl0bw[txbw_override_idx];
			}
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */
			/* mcs 32 must be 40b/w DUP */
			if (IS_MCS(rspec) && ((rspec & RSPEC_RATE_MASK) == 32))
				mimo_txbw = RSPEC_BW_40MHZ;
		} else  {
			/* mcs32 is 40 b/w only.
			 * This is possible for probe packets on a STA during SCAN
			 */
			if ((rspec & RSPEC_RATE_MASK) == 32) {
				WL_INFORM(("wl%d: wlc_d11hdrs mcs 32 invalid in 20MHz mode, using"
					"mcs 0 instead\n", wlc->pub->unit));

				rspec = HT_RSPEC(0);	/* mcs 0 */
			}

			/* Fix fallback as well */
			if ((rspec_fallback & RSPEC_RATE_MASK) == 32)
				rspec_fallback = HT_RSPEC(0);	/* mcs 0 */

			mimo_txbw = RSPEC_BW_20MHZ;
		}

		rspec &= ~RSPEC_BW_MASK;
		rspec |= mimo_txbw;

		rspec_fallback &= ~RSPEC_BW_MASK;
		if (IS_MCS(rspec_fallback))
			rspec_fallback |= mimo_txbw;
		else
			rspec_fallback |= RSPEC_BW_20MHZ;
		sgi_tx = WLC_HT_GET_SGI_TX(hti);
		if (!RSPEC_ACTIVE(wlc->band->rspec_override)) {
			if (IS_MCS(rspec) && (sgi_tx == ON))
				rspec |= RSPEC_SHORT_GI;
			else if (sgi_tx == OFF)
				rspec &= ~RSPEC_SHORT_GI;

			if (IS_MCS(rspec_fallback) && (sgi_tx == ON))
				rspec_fallback |= RSPEC_SHORT_GI;
			else if (sgi_tx == OFF)
				rspec_fallback &= ~RSPEC_SHORT_GI;
		}

		if (!RSPEC_ACTIVE(wlc->band->rspec_override)) {
			ASSERT(!(rspec & RSPEC_LDPC_CODING));
			ASSERT(!(rspec_fallback & RSPEC_LDPC_CODING));
			rspec &= ~RSPEC_LDPC_CODING;
			rspec_fallback &= ~RSPEC_LDPC_CODING;
			if (wlc->stf->ldpc_tx == ON ||
			    (SCB_LDPC_CAP(scb) && wlc->stf->ldpc_tx == AUTO)) {
				if (IS_MCS(rspec))
					rspec |= RSPEC_LDPC_CODING;
				if (IS_MCS(rspec_fallback))
					rspec_fallback |= RSPEC_LDPC_CODING;
			}
		}

		mimo_preamble_type = wlc_prot_n_preamble(wlc, scb);
		if (IS_MCS(rspec)) {
			preamble_type = mimo_preamble_type;
			if (n_prot == WLC_N_PROTECTION_20IN40 &&
			    RSPEC_IS40MHZ(rspec))
				use_cts = TRUE;

			/* if the mcs is multi stream check if it needs an rts */
			if (!IS_SINGLE_STREAM(rspec & RSPEC_RATE_MASK)) {
				if (WLC_HT_SCB_RTS_ENAB(hti, scb)) {
					use_rts = use_mimops_rts = TRUE;
				}
			}

			/* if SGI is selected, then forced mm for single stream */
			if ((rspec & RSPEC_SHORT_GI) && IS_SINGLE_STREAM(rspec & RSPEC_RATE_MASK)) {
				ASSERT(IS_MCS(rspec));
				preamble_type = WLC_MM_PREAMBLE;
			}
		}

		if (IS_MCS(rspec_fallback)) {
			fbr_preamble_type = mimo_preamble_type;

			/* if SGI is selected, then forced mm for single stream */
			if ((rspec_fallback & RSPEC_SHORT_GI) &&
			    IS_SINGLE_STREAM(rspec_fallback & RSPEC_RATE_MASK))
				fbr_preamble_type = WLC_MM_PREAMBLE;
		}
	} else
#endif /* WL11N */
	{
		/* Set ctrlchbw as 20Mhz */
		ASSERT(!IS_MCS(rspec));
		ASSERT(!IS_MCS(rspec_fallback));
		rspec &= ~RSPEC_BW_MASK;
		rspec_fallback &= ~RSPEC_BW_MASK;
		rspec |= RSPEC_BW_20MHZ;
		rspec_fallback |= RSPEC_BW_20MHZ;

#ifdef WL11N
#if NCONF
		/* for nphy, stf of ofdm frames must follow policies */
		if ((WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) && IS_OFDM(rspec)) {
			rspec &= ~RSPEC_TXEXP_MASK;
			if (phyctl1_stf == PHY_TXC1_MODE_CDD) {
				rspec |= (1 << RSPEC_TXEXP_SHIFT);
			}
		}
		if ((WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) && IS_OFDM(rspec_fallback)) {
			rspec_fallback &= ~RSPEC_TXEXP_MASK;
			if (phyctl1_stf == PHY_TXC1_MODE_CDD) {
				rspec_fallback |= (1 << RSPEC_TXEXP_SHIFT);
			}
		}
#endif /* NCONF */
#endif /* WL11N */
	}

	/* record rate history after the txbw is valid */
	if (rspec_history) {
#ifdef WL11N
		/* store current tx ant config ratesel (ignore probes) */
		if (WLANTSEL_ENAB(wlc) && N_ENAB(wlc->pub) &&
			((frameid & TXFID_RATE_PROBE_MASK) == 0)) {
			wlc_antsel_set_unicast(wlc->asi, antcfg);
		}
#endif // endif

		/* update per bsscfg tx rate */
		bsscfg->txrspec[bsscfg->txrspecidx][0] = rspec;
		bsscfg->txrspec[bsscfg->txrspecidx][1] = (uint8) nfrags;
		bsscfg->txrspecidx = (bsscfg->txrspecidx+1) % NTXRATE;

		WLCNTSCBSET(scb->scb_stats.tx_rate_fallback, rspec_fallback);
	}

	/* mimo bw field MUST now be valid in the rspec (it affects duration calculations) */
	rate = RSPEC2RATE(rspec);

	/* (2) PROTECTION, may change rspec */
	if (((type == FC_TYPE_DATA) || (type == FC_TYPE_MNG)) &&
	    ((phylen > wlc->RTSThresh) || (pkttag->flags & WLF_USERTS)) &&
	    !ETHER_ISMULTI(&h->a1))
		use_rts = TRUE;

	if ((wlc->band->gmode && g_prot && IS_OFDM(rspec)) ||
	    (N_ENAB(wlc->pub) && IS_MCS(rspec) && n_prot)) {
		if (nfrags > 1) {
			/* For a frag burst use the lower modulation rates for the entire frag burst
			 * instead of protection mechanisms.
			 * As per spec, if protection mechanism is being used, a fragment sequence
			 * may only employ ERP-OFDM modulation for the final fragment and control
			 * response. (802.11g Sec 9.10) For ease we send the *whole* sequence at the
			 * lower modulation instead of using a higher modulation for the last frag.
			 */

			/* downgrade the rate to CCK or OFDM */
			if (g_prot) {
				/* Use 11 Mbps as the rate and fallback. We should make sure that if
				 * we are downgrading an OFDM rate to CCK, we should pick a more
				 * robust rate.  6 and 9 Mbps are not usually selected by rate
				 * selection, but even if the OFDM rate we are downgrading is 6 or 9
				 * Mbps, 11 Mbps is more robust.
				 */
				rspec = rspec_fallback = CCK_RSPEC(WLC_RATE_11M);
			} else {
				/* Use 24 Mbps as the rate and fallback for what would have been
				 * a MIMO rate. 24 Mbps is the highest phy mandatory rate for OFDM.
				 */
				rspec = rspec_fallback = OFDM_RSPEC(WLC_RATE_24M);
			}
			pkttag->flags &= ~WLF_RATE_AUTO;
		} else {
			/* Use protection mechanisms on unfragmented frames */
			/* If this is a 11g protection, then use CTS-to-self */
			if (wlc->band->gmode && g_prot && !IS_CCK(rspec))
				use_cts = TRUE;
		}
	}

	/* calculate minimum rate */
	ASSERT(RSPEC2KBPS(rspec_fallback) <= RSPEC2KBPS(rspec));
	ASSERT(VALID_RATE_DBG(wlc, rspec));
	ASSERT(VALID_RATE_DBG(wlc, rspec_fallback));

	/* fix the preamble type for non-MCS rspec/rspec-fallback
	 * If SCB is short preamble capable and and shortpreamble is enabled
	 * and rate is NOT 1M or Override is NOT set to LONG preamble then use SHORT preamble
	 * else use LONG preamble
	 * OFDM should bypass below preamble set, but old chips are OK since they ignore that bit
	 * XXX before 4331A0 PR79339 is fixed, restrict to CCK, which is actually correct
	 */
	if (IS_CCK(rspec)) {
		if (short_preamble &&
		    !((RSPEC2RATE(rspec) == WLC_RATE_1M) ||
		      (scb->bsscfg->PLCPHdr_override == WLC_PLCP_LONG)))
			preamble_type = WLC_SHORT_PREAMBLE;
		else
			preamble_type = WLC_LONG_PREAMBLE;
	}

	if (IS_CCK(rspec_fallback)) {
		if (short_preamble &&
		    !((RSPEC2RATE(rspec_fallback) == WLC_RATE_1M) ||
		      (scb->bsscfg->PLCPHdr_override == WLC_PLCP_LONG)))
			fbr_preamble_type = WLC_SHORT_PREAMBLE;
		else
			fbr_preamble_type = WLC_LONG_PREAMBLE;
	}

	ASSERT(!IS_MCS(rspec) || WLC_IS_MIMO_PREAMBLE(preamble_type));
	ASSERT(!IS_MCS(rspec_fallback) || WLC_IS_MIMO_PREAMBLE(fbr_preamble_type));

	WLCNTSCB_COND_SET(((type == FC_TYPE_DATA) &&
		((FC_SUBTYPE(fc) != FC_SUBTYPE_NULL) &&
			(FC_SUBTYPE(fc) != FC_SUBTYPE_QOS_NULL))),
			scb->scb_stats.tx_rate, rspec);
#ifdef WLSCB_HISTO
	/* test if required */
	WLSCB_HISTO_TX_DATA_IF_LEGACY(scb, rspec, fc, 1 << WLPKTTAG_AMSDU(p));
#endif /* WLSCB_HISTO */
#ifdef WLFCTS
	if (WLFCTS_ENAB(wlc->pub))
		WLPKTTAG(p)->rspec = rspec;
#endif // endif

	/* RIFS(testing only): based on frameburst, non-CCK frames only */
	if (SCB_HT_CAP(scb) && WLC_HT_GET_FRAMEBURST(hti) &&
		WLC_HT_GET_RIFS(hti) &&
		n_prot != WLC_N_PROTECTION_MIXEDMODE && !IS_CCK(rspec) &&
	    !ETHER_ISMULTI(&h->a1) && ((fc & FC_KIND_MASK) == FC_QOS_DATA)) {
		uint16 qos_field, *pqos;

		WLPKTTAG(p)->flags |= WLF_RIFS;
		mcl |= (TXC_FRAMEBURST | TXC_USERIFS);
		use_rifs = TRUE;

		/* RIFS implies QoS frame with no-ack policy, hack the QoS field */
		pqos = (uint16 *)((uchar *)h + (a4 ?
			DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN));
		qos_field = ltoh16(*pqos) & ~QOS_ACK_MASK;
		qos_field |= (QOS_ACK_NO_ACK << QOS_ACK_SHIFT) & QOS_ACK_MASK;
		*pqos = htol16(qos_field);
	}

	/* (3) PLCP: determine PLCP header and MAC duration, fill d11txh_t */
	wlc_compute_plcp(wlc, rspec, phylen, fc, plcp);
	wlc_compute_plcp(wlc, rspec_fallback, phylen, fc, plcp_fallback);
	bcopy(plcp_fallback, (char*)&txh->FragPLCPFallback, sizeof(txh->FragPLCPFallback));

	/* Length field now put in CCK FBR CRC field for AES */
	if (IS_CCK(rspec_fallback)) {
		txh->FragPLCPFallback[4] = phylen & 0xff;
		txh->FragPLCPFallback[5] = (phylen & 0xff00) >> 8;
	}

#ifdef WLAMPDU
	/* mark pkt as aggregable if it is */
	if (WLPKTFLAG_AMPDU(pkttag) && IS_MCS(rspec)) {
		if (WLC_KEY_ALLOWS_AMPDU(key_info)) {
			WLPKTTAG(p)->flags |= WLF_MIMO;
			if (WLC_HT_GET_AMPDU_RTS(wlc->hti))
				use_rts = TRUE;
		}
	}
#endif /* WLAMPDU */

	/* MIMO-RATE: need validation ?? */
	mainrates = IS_OFDM(rspec) ? D11A_PHY_HDR_GRATE((ofdm_phy_hdr_t *)plcp) : plcp[0];

	/* DUR field for main rate */
	if (((fc & FC_KIND_MASK) != FC_PS_POLL) && !ETHER_ISMULTI(&h->a1) &&
			!use_rifs) {
#ifdef WLAMPDU
		if (WLPKTFLAG_AMPDU(pkttag))
			durid = wlc_compute_ampdu_mpdu_dur(wlc, rspec);
		else
#endif /* WLAMPDU */
		durid = wlc_compute_frame_dur(wlc, rspec, preamble_type, next_frag_len);
		h->durid = htol16(durid);
	}
	else if (use_rifs) {
		/* NAV protect to end of next max packet size */
		durid = (uint16)wlc_calc_frame_time(wlc, rspec, preamble_type, DOT11_MAX_FRAG_LEN);
		durid += RIFS_11N_TIME;
		h->durid = htol16(durid);
	}

	/* DUR field for fallback rate */
	if (fc == FC_PS_POLL)
		txh->FragDurFallback = h->durid;
	else if (ETHER_ISMULTI(&h->a1) || use_rifs)
		txh->FragDurFallback = 0;
	else {
#ifdef WLAMPDU
		if (WLPKTFLAG_AMPDU(pkttag))
			durid = wlc_compute_ampdu_mpdu_dur(wlc, rspec_fallback);
		else
#endif /* WLAMPDU */
		durid = wlc_compute_frame_dur(wlc, rspec_fallback,
		                              fbr_preamble_type, next_frag_len);
		txh->FragDurFallback = htol16(durid);
	}

	/* Timestamp */
	if ((pkttag->flags & WLF_EXPTIME)) {
		txh->TstampLow = htol16(pkttag->u.exptime & 0xffff);
		txh->TstampHigh = htol16((pkttag->u.exptime >> 16) & 0xffff);

		mcl |= TXC_LIFETIME;	/* Enable timestamp for the packet */
	}

	/* (4) MAC-HDR: MacTxControlLow */
	if (frag == 0)
		mcl |= TXC_STARTMSDU;

	if (!ETHER_ISMULTI(&h->a1) && !WLPKTFLAG_RIFS(pkttag) &&
	    !(wlc->_amsdu_noack && WLPKTFLAG_AMSDU(pkttag)) &&
	    !(!WLPKTFLAG_AMPDU(pkttag) && qos && wme->wme_noack))
		mcl |= TXC_IMMEDACK;

	if (type == FC_TYPE_DATA) {
		if ((WLC_HT_GET_FRAMEBURST(hti) && (rate > WLC_FRAMEBURST_MIN_RATE) &&
#ifdef WLAMPDU
		     AMPDU_ENAB(wlc->pub) && SCB_AMPDU(scb) &&
		     (wlc_ampdu_frameburst_override(wlc->ampdu_tx) == FALSE) &&
#endif // endif
		     (!wme->edcf_txop[ac] || WLPKTFLAG_AMPDU(pkttag))) ||
#ifdef WLBTAMP
		     (BTA_ENAB(wlc->pub) &&
		     wlc_bta_frameburst_active(wlc->bta, pkttag, rate)) ||
#endif // endif
		     FALSE)
#ifdef WL11N
			/* don't allow bursting if rts is required around each mimo frame */
			if (use_mimops_rts == FALSE)
#endif // endif
				mcl |= TXC_FRAMEBURST;
	}

	if (BAND_5G(wlc->band->bandtype))
		mcl |= TXC_FREQBAND_5G;

	if (CHSPEC_IS40(WLC_BAND_PI_RADIO_CHANSPEC))
		mcl |= TXC_BW_40;

	txh->MacTxControlLow = htol16(mcl);

	/* MacTxControlHigh */
	mch = 0;

	/* note: mch is updated for h/w enc as part of tx_prep_mpdu */
#ifdef WLBTAMP
	/* use short range mode txpwr at 4dbm */
	if (pkttag->flags & WLF_BTA_SRM)
		mch |= TXC_ALT_TXPWR;
#endif // endif
	/* Set fallback rate preamble type */
	if ((fbr_preamble_type == WLC_SHORT_PREAMBLE) ||
	    (fbr_preamble_type == WLC_GF_PREAMBLE)) {
		ASSERT((fbr_preamble_type == WLC_GF_PREAMBLE) ||
		       (!IS_MCS(rspec_fallback)));
		mch |= TXC_PREAMBLE_DATA_FB_SHORT;
	}

	/* MacFrameControl */
	bcopy((char*)&h->fc, (char*)&txh->MacFrameControl, sizeof(uint16));

	txh->TxFesTimeNormal = htol16(0);

	txh->TxFesTimeFallback = htol16(0);

	/* TxFrameRA */
	bcopy((char*)&h->a1, (char*)&txh->TxFrameRA, ETHER_ADDR_LEN);

	/* TxFrameID */
	txh->TxFrameID = htol16(frameid);

#ifdef WL11N
	/* Set tx antenna configuration for all transmissions */
	if (WLANTSEL_ENAB(wlc)) {
		if (antcfg == ANTCFG_NONE) {
			/* use tx antcfg default */
			wlc_antsel_antcfg_get(wlc->asi, TRUE, FALSE, 0, 0, &antcfg, &fbantcfg);
		}
		mimoantsel = wlc_antsel_buildtxh(wlc->asi, antcfg, fbantcfg);
		txh->ABI_MimoAntSel = htol16(mimoantsel);
	}
#endif /* WL11N */

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		uint16 bss = (uint16)wlc_mcnx_BSS_idx(wlc->mcnx, bsscfg);
		ASSERT(bss < M_P2P_BSS_MAX);
		txh->ABI_MimoAntSel |= htol16(bss << ABI_MAS_ADDR_BMP_IDX_SHIFT);
	}
#endif // endif

	/* TxStatus, Note the case of recreating the first frag of a suppressed frame
	 * then we may need to reset the retry cnt's via the status reg
	 */
	txh->TxStatus = htol16(status);

	if (D11REV_GE(wlc->pub->corerev, 16)) {
		/* extra fields for ucode AMPDU aggregation, the new fields are added to
		 * the END of previous structure so that it's compatible in driver.
		 * In old rev ucode, these fields should be ignored
		 */
		txh->MaxNMpdus = htol16(0);
		txh->u1.MaxAggDur = htol16(0);
		txh->u2.MaxAggLen_FBR = htol16(0);
		txh->MinMBytes = htol16(0);
	}

	/* (5) RTS/CTS: determine RTS/CTS PLCP header and MAC duration, furnish d11txh_t */

	/* RTS PLCP header and RTS frame */
	if (use_rts || use_cts) {
		if (use_rts && use_cts)
			use_cts = FALSE;

#ifdef NOT_YET
		/* XXX Optimization: if use rts-cts or cts-to-self,
		 * mixedmode preamble may not be necessary if the dest
		 * support GF. Clear hdr_mcs_mixedmode and reset
		 * short_preamble, recalculating duration fields are
		 * required.
		 */
		if (SCB_ISGF_CAP(scb)) {
			hdr_mcs_mixedmode = FALSE;
			hdr_cckofdm_shortpreamble = TRUE;
		}
#endif /* NOT_YET */

		rts_rspec = wlc_rspec_to_rts_rspec(bsscfg, rspec, FALSE);
		rts_rspec_fallback = wlc_rspec_to_rts_rspec(bsscfg, rspec_fallback, FALSE);

		/* XXX
		 * OFDM should bypass below preamble set, but old chips are OK since they ignore
		 * that bit. Before 4331A0 PR79339 is fixed, restrict to CCK, which is actually
		 * correct
		 */
		if (!IS_OFDM(rts_rspec) &&
		    !((RSPEC2RATE(rts_rspec) == WLC_RATE_1M) ||
		      (scb->bsscfg->PLCPHdr_override == WLC_PLCP_LONG))) {
			rts_preamble_type = WLC_SHORT_PREAMBLE;
			mch |= TXC_PREAMBLE_RTS_MAIN_SHORT;
		}

		if (!IS_OFDM(rts_rspec_fallback) &&
		    !((RSPEC2RATE(rts_rspec_fallback) == WLC_RATE_1M) ||
		      (scb->bsscfg->PLCPHdr_override == WLC_PLCP_LONG))) {
			rts_fbr_preamble_type = WLC_SHORT_PREAMBLE;
			mch |= TXC_PREAMBLE_RTS_FB_SHORT;
		}

		/* RTS/CTS additions to MacTxControlLow */
		if (use_cts) {
			txh->MacTxControlLow |= htol16(TXC_SENDCTS);
		} else {
			txh->MacTxControlLow |= htol16(TXC_SENDRTS);
			txh->MacTxControlLow |= htol16(TXC_LONGFRAME);
		}

		/* RTS PLCP header */
		ASSERT(ISALIGNED(txh->RTSPhyHeader, sizeof(uint16)));
		rts_plcp = txh->RTSPhyHeader;
		if (use_cts)
			rts_phylen = DOT11_CTS_LEN + DOT11_FCS_LEN;
		else
			rts_phylen = DOT11_RTS_LEN + DOT11_FCS_LEN;

		/* dot11n headers */
		wlc_compute_plcp(wlc, rts_rspec, rts_phylen, fc, rts_plcp);

		/* fallback rate version of RTS PLCP header */
		wlc_compute_plcp(wlc, rts_rspec_fallback, rts_phylen, fc, rts_plcp_fallback);

		bcopy(rts_plcp_fallback, (char*)&txh->RTSPLCPFallback,
			sizeof(txh->RTSPLCPFallback));

		/* RTS frame fields... */
		rts = (struct dot11_rts_frame*)&txh->rts_frame;

		durid = wlc_compute_rtscts_dur(wlc, use_cts, rts_rspec,
		        rspec, rts_preamble_type, preamble_type, phylen, FALSE);
		rts->durid = htol16(durid);

		/* fallback rate version of RTS DUR field */
		durid = wlc_compute_rtscts_dur(wlc, use_cts,
		        rts_rspec_fallback, rspec_fallback, rts_fbr_preamble_type,
		        fbr_preamble_type, phylen, FALSE);
		txh->RTSDurFallback = htol16(durid);

		if (use_cts) {
			rts->fc = htol16(FC_CTS);
			bcopy((char*)&h->a2, (char*)&rts->ra, ETHER_ADDR_LEN);
		} else {
			rts->fc = htol16((uint16)FC_RTS);
			bcopy((char*)&h->a1, (char*)&rts->ra, ETHER_ADDR_LEN);
			bcopy((char*)&h->a2, (char*)&rts->ta, ETHER_ADDR_LEN);
		}

		/* mainrate
		 *    low 8 bits: main frag rate/mcs,
		 *    high 8 bits: rts/cts rate/mcs
		 */
		mainrates |= (IS_OFDM(rts_rspec) ?
			D11A_PHY_HDR_GRATE((ofdm_phy_hdr_t *)rts_plcp) : rts_plcp[0]) << 8;
	} else {
		bzero((char*)txh->RTSPhyHeader, D11_PHY_HDR_LEN);
		bzero((char*)&txh->rts_frame, sizeof(struct dot11_rts_frame));
		bzero((char*)txh->RTSPLCPFallback, sizeof(txh->RTSPLCPFallback));
		txh->RTSDurFallback = 0;
	}

#ifdef WLAMPDU
	/* add null delimiter count */
	if (WLPKTFLAG_AMPDU(pkttag) && IS_MCS(rspec)) {
		uint16 minbytes = 0;
		txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM] =
			wlc_ampdu_null_delim_cnt(wlc->ampdu_tx, scb, rspec, phylen, &minbytes);
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub))
			txh->MinMBytes = htol16(minbytes);
#endif // endif
	}
#endif	/* WLAMPDU */

	/* Now that RTS/RTS FB preamble types are updated, write the final value */
	txh->MacTxControlHigh = htol16(mch);

	/* MainRates (both the rts and frag plcp rates have been calculated now) */
	txh->MainRates = htol16(mainrates);

	/* XtraFrameTypes */
	xfts = PHY_TXC_FRAMETYPE(rspec_fallback);
	xfts |= (PHY_TXC_FRAMETYPE(rts_rspec) << XFTS_RTS_FT_SHIFT);
	xfts |= (PHY_TXC_FRAMETYPE(rts_rspec_fallback) << XFTS_FBRRTS_FT_SHIFT);
	xfts |= CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC) << XFTS_CHANNEL_SHIFT;
	txh->XtraFrameTypes = htol16(xfts);

	/* PhyTxControlWord */
	phyctl = PHY_TXC_FRAMETYPE(rspec);
	if ((preamble_type == WLC_SHORT_PREAMBLE) ||
	    (preamble_type == WLC_GF_PREAMBLE)) {
		ASSERT((preamble_type == WLC_GF_PREAMBLE) || !IS_MCS(rspec));
		phyctl |= PHY_TXC_SHORT_HDR;
		WLCNTINCR(wlc->pub->_cnt->txprshort);
	}

	/* phytxant is properly bit shifted */
	phyctl |= wlc_stf_d11hdrs_phyctl_txant(wlc, rspec);
	if (WLCISNPHY(wlc->band) && (wlc->pub->sromrev >= 9)) {
		uint8 rate_offset;
		uint16 minbytes;
		rate_offset = wlc_stf_get_pwrperrate(wlc, rspec, 0);
		phyctl |= (rate_offset <<  PHY_TXC_PWR_SHIFT);
		rate_offset = wlc_stf_get_pwrperrate(wlc, rspec_fallback, 0);
		minbytes = ltoh16(txh->MinMBytes);
		minbytes |= (rate_offset << MINMBYTES_FBRATE_PWROFFSET_SHIFT);
		txh->MinMBytes = htol16(minbytes);
	}

#ifdef WL_LPC
	if ((pkttag->flags & WLF_RATE_AUTO) && LPC_ENAB(wlc)) {
			uint16 ratepwr_offset = 0;

			/* Note the per rate power offset for this rate */
			ratepwr_offset = wlc_phy_lpc_get_txcpwrval(WLC_PI(wlc), phyctl);

			/* Update the Power offset bits */
			/* lpc_offset = lpc_offset + ratepwr_offset;
			*  XXX Investigate how to combine offsets
			*/
			if (ratepwr_offset) {
				/* for the LPC enabled 11n chips ratepwr_offset should be 0 */
				ASSERT(FALSE);
			}
			wlc_phy_lpc_set_txcpwrval(WLC_PI(wlc), &phyctl, lpc_offset);
	}
#endif /* WL_LPC */
	txh->PhyTxControlWord = htol16(phyctl);

	/* PhyTxControlWord_1 */
	if (WLC_PHY_11N_CAP(wlc->band) || WLCISLPPHY(wlc->band)) {
		uint16 phyctl1 = 0;

		phyctl1 = wlc_phytxctl1_calc(wlc, rspec, wlc->chanspec);
		WLC_PHYCTLBW_OVERRIDE(phyctl1, PHY_TXC1_BW_MASK, phyctl_bwo);
		txh->PhyTxControlWord_1 = htol16(phyctl1);

		phyctl1 = wlc_phytxctl1_calc(wlc, rspec_fallback, wlc->chanspec);
		if (IS_MCS(rspec_fallback)) {
			WLC_PHYCTLBW_OVERRIDE(phyctl1, PHY_TXC1_BW_MASK, phyctl_bwo);
		}
		txh->PhyTxControlWord_1_Fbr = htol16(phyctl1);

		if (use_rts || use_cts) {
			phyctl1 = wlc_phytxctl1_calc(wlc, rts_rspec, wlc->chanspec);
			txh->PhyTxControlWord_1_Rts = htol16(phyctl1);
			phyctl1 = wlc_phytxctl1_calc(wlc, rts_rspec_fallback, wlc->chanspec);
			txh->PhyTxControlWord_1_FbrRts = htol16(phyctl1);
		}

		/*
		 * For mcs frames, if mixedmode(overloaded with long preamble) is going to be set,
		 * fill in non-zero MModeLen and/or MModeFbrLen
		 *  it will be unnecessary if they are separated
		 */
		if (IS_MCS(rspec) && (preamble_type == WLC_MM_PREAMBLE)) {
			uint16 mmodelen = wlc_calc_lsig_len(wlc, rspec, phylen);
			txh->MModeLen = htol16(mmodelen);
		}

		if (IS_MCS(rspec_fallback) && (fbr_preamble_type == WLC_MM_PREAMBLE)) {
			uint16 mmodefbrlen = wlc_calc_lsig_len(wlc, rspec_fallback, phylen);
			txh->MModeFbrLen = htol16(mmodefbrlen);
		}
	}

	/* XXX Make sure that either rate is NOT MCS or if preamble type is mixedmode
	 * then length is not 0
	 */
	ASSERT(!IS_MCS(rspec) ||
	       ((preamble_type == WLC_MM_PREAMBLE) == (txh->MModeLen != 0)));
	ASSERT(!IS_MCS(rspec_fallback) ||
	       ((fbr_preamble_type == WLC_MM_PREAMBLE) == (txh->MModeFbrLen != 0)));

	if (SCB_WME(scb) && qos && wme->edcf_txop[ac]) {
		uint frag_dur, dur, dur_fallback;

		ASSERT(!ETHER_ISMULTI(&h->a1));

		/* WME: Update TXOP threshold */
		/*
		 * XXX should this calculation be done for null frames also?
		 * What else is wrong with these calculations?
		 */
		if ((!WLPKTFLAG_AMPDU(pkttag)) && (frag == 0)) {
			int16 delta;

			frag_dur = wlc_calc_frame_time(wlc, rspec, preamble_type, phylen);

			if (rts) {
				/* 1 RTS or CTS-to-self frame */
				dur = wlc_calc_cts_time(wlc, rts_rspec, rts_preamble_type);
				dur_fallback = wlc_calc_cts_time(wlc, rts_rspec_fallback,
				                                 rts_fbr_preamble_type);
				/* (SIFS + CTS) + SIFS + frame + SIFS + ACK */
				dur += ltoh16(rts->durid);
				dur_fallback += ltoh16(txh->RTSDurFallback);
			} else if (use_rifs) {
				dur = frag_dur;
				dur_fallback = 0;
			} else {
				/* frame + SIFS + ACK */
				dur = frag_dur;
				dur += wlc_compute_frame_dur(wlc, rspec, preamble_type, 0);

				dur_fallback = wlc_calc_frame_time(wlc, rspec_fallback,
				               fbr_preamble_type, phylen);
				dur_fallback += wlc_compute_frame_dur(wlc, rspec_fallback,
				                                      fbr_preamble_type, 0);
			}
			/* NEED to set TxFesTimeNormal (hard) */
			txh->TxFesTimeNormal = htol16((uint16)dur);
			/* NEED to set fallback rate version of TxFesTimeNormal (hard) */
			txh->TxFesTimeFallback = htol16((uint16)dur_fallback);

			/* update txop byte threshold (txop minus intraframe overhead) */
			delta = (int16)(wme->edcf_txop[ac] - (dur - frag_dur));
			if (delta >= 0) {
#ifdef WLAMSDU_TX
				if (AMSDU_TX_ENAB(wlc->pub) &&
				    WLPKTFLAG_AMSDU(pkttag) && (queue == TX_AC_BE_FIFO)) {
					WL_ERROR(("edcf_txop changed, update AMSDU\n"));
					wlc_amsdu_txop_upd(wlc->ami);
				} else
#endif // endif
				{
				uint newfragthresh =
				        wlc_calc_frame_len(wlc, rspec,
				                           preamble_type, (uint16)delta);
				/* range bound the fragthreshold */
				newfragthresh = MAX(newfragthresh, DOT11_MIN_FRAG_LEN);
				newfragthresh = MIN(newfragthresh, wlc->usr_fragthresh);
				/* update the fragthresh and do txc update */
				if (wlc->fragthresh[ac] != (uint16)newfragthresh)
					wlc_fragthresh_set(wlc, ac, (uint16)newfragthresh);
				}
			} else
				WL_ERROR(("wl%d: %s txop invalid for rate %d\n",
					wlc->pub->unit, fifo_names[queue], RSPEC2RATE(rspec)));

			if (CAC_ENAB(wlc->pub) &&
				queue <= TX_AC_VO_FIFO) {
				wlc_cac_update_dur_cache(wlc->cac, ac, PKTPRIO(p), scb, dur,
					(phylen - keyinfo_len), WLC_CAC_DUR_CACHE_PREP);
				/* update cac used time */
				if (wlc_cac_update_used_time(wlc->cac, ac, dur, scb))
					WL_ERROR(("wl%d: ac %d: txop exceeded allocated TS time\n",
						wlc->pub->unit, ac));
			}

			/*
			 * FIXME: The MPDUs of the next transmitted MSDU after
			 * rate drops or RTS/CTS kicks in may exceed
			 * TXOP. Without tearing apart the transmit
			 * path---either push rate and RTS/CTS decisions much
			 * earlier (hard), allocate fragments just in time
			 * (harder), or support late refragmentation (even
			 * harder)---it's too difficult to fix this now.
			 */
			if (dur > wme->edcf_txop[ac])
				WL_ERROR(("wl%d: %s txop exceeded phylen %d/%d dur %d/%d\n",
					wlc->pub->unit, fifo_names[queue], phylen,
					wlc->fragthresh[ac], dur, wme->edcf_txop[ac]));
		}
	} else if (SCB_WME(scb) && qos && CAC_ENAB(wlc->pub) && queue <= TX_AC_VO_FIFO) {
		uint dur;
		if (rts) {
			/* 1 RTS or CTS-to-self frame */
			dur = wlc_calc_cts_time(wlc, rts_rspec, rts_preamble_type);
			/* (SIFS + CTS) + SIFS + frame + SIFS + ACK */
			dur += ltoh16(rts->durid);
		} else {
			/* frame + SIFS + ACK */
			dur = wlc_calc_frame_time(wlc, rspec, preamble_type, phylen);
			dur += wlc_compute_frame_dur(wlc, rspec, preamble_type, 0);
		}

		/* update cac used time */
		wlc_cac_update_dur_cache(wlc->cac, ac, PKTPRIO(p), scb, dur,
				(phylen - keyinfo_len), WLC_CAC_DUR_CACHE_PREP);
		if (wlc_cac_update_used_time(wlc->cac, ac, dur, scb))
			WL_ERROR(("wl%d: ac %d: txop exceeded allocated TS time\n",
				wlc->pub->unit, ac));
	}

#ifdef WLAWDL
	if (txparams_flags & WLC_TX_PARAMS_AWDL_AF_TS) {
		WL_AWDL(("%s(): set ABI_MAS_AWDL_TS_INSERT.\n", __FUNCTION__));
		txh->ABI_MimoAntSel |= htol16(ABI_MAS_AWDL_TS_INSERT); /* bit 12 */
	}
#endif	/* WLAWDL */

	/* With d11 hdrs on, mark the packet as MPDU with txhdr */
	WLPKTTAG(p)->flags |= (WLF_MPDU | WLF_TXHDR);

	/* TDLS U-APSD buffer STA: if peer is in PS, save the last MPDU's seq and tid for PTI */
#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, SCB_BSSCFG(scb)) &&
		SCB_PS(scb) &&
		wlc_tdls_buffer_sta_enable(wlc->tdls)) {
		uint8 tid = 0;
		if (qos) {
			uint16 qc;
			qc = (*(uint16 *)((uchar *)h + (a4 ?
				DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN)));
			tid = (QOS_TID(qc) & 0x0f);
		}
		wlc_tdls_update_tid_seq(wlc->tdls, scb, tid, seq);
	}
#endif // endif

	return (frameid);
}

#ifdef WL11N
/** At this point the rspec may not include a valid txbw. Pick a transmit bandwidth. */
static INLINE ratespec_t
wlc_d11ac_hdrs_determine_mimo_txbw(wlc_info_t *wlc, struct scb *scb, wlc_bsscfg_t *bsscfg,
                                   ratespec_t rspec)
{
	uint32 mimo_txbw;
	/* XXX 4360: wlc_d11ac_hdrs(): this txbw section and the same one in the
	 *11n_hdrs fn can be removed and ratesel can keep track of the configured
	 * bw for a STA.
	 */

	if (RSPEC_BW(rspec) != RSPEC_BW_UNSPECIFIED) {
		mimo_txbw = RSPEC_BW(rspec);

		/* If the ratespec override has a bw greater than
		 * the channel bandwidth, limit here.
		 */
		if (CHSPEC_IS20(wlc->chanspec)) {
			mimo_txbw = RSPEC_BW_20MHZ;
		} else if (CHSPEC_IS40(wlc->chanspec)) {
			mimo_txbw = MIN(mimo_txbw, RSPEC_BW_40MHZ);
		}
		else if (CHSPEC_IS80(wlc->chanspec)) {
			mimo_txbw = MIN(mimo_txbw, RSPEC_BW_80MHZ);
		} else if (CHSPEC_IS160(wlc->chanspec) || CHSPEC_IS8080(wlc->chanspec)) {
			mimo_txbw = MIN(mimo_txbw, RSPEC_BW_160MHZ);
		}
	}
	/* Is the phy configured to use > 20MHZ frames? If so then pick the
	 * desired txbw
	 */
	else if (CHSPEC_IS8080(wlc->chanspec) &&
		RSPEC_ISVHT(rspec) && (scb->flags3 & SCB3_IS_80_80)) {
		mimo_txbw = RSPEC_BW_160MHZ;
	} else if (CHSPEC_IS160(wlc->chanspec) &&
		RSPEC_ISVHT(rspec) && (scb->flags3 & SCB3_IS_160)) {
		mimo_txbw = RSPEC_BW_160MHZ;
	} else if (CHSPEC_IS80(wlc->chanspec) && RSPEC_ISVHT(rspec)) {
		mimo_txbw = RSPEC_BW_80MHZ;
	} else if (CHSPEC_BW_GE(wlc->chanspec, WL_CHANSPEC_BW_40)) {
		/* default txbw is 20in40 */
		mimo_txbw = RSPEC_BW_20MHZ;

		if (RSPEC_ISHT(rspec) || RSPEC_ISVHT(rspec)) {
			if (scb->flags & SCB_IS40) {
				mimo_txbw = RSPEC_BW_40MHZ;
#ifdef WLMCHAN
				/* XXX 4360: why would scb->flags indicate is40
				 * if the sta is associated at a 20MHz? Do we
				 *	need different flags for capability (is40)
				 * from operational for the current state,
				 * which would be is20?  This same problem needs
				 * to be fixed
				 * for the 80MHz case.
				 */
				/* PR95044: if mchan enabled and bsscfg is AP,
				 * then must check the bsscfg chanspec to make
				 * sure our AP is operating on 40MHz channel.
				 */
				if (MCHAN_ENAB(wlc->pub) && BSSCFG_AP(bsscfg) &&
					CHSPEC_IS20(
					bsscfg->current_bss->chanspec)) {
					mimo_txbw = RSPEC_BW_20MHZ;
				}
#endif /* WLMCHAN */
			}
		}

		if (RSPEC_ISHT(rspec) && ((rspec & RSPEC_RATE_MASK) == 32))
			mimo_txbw = RSPEC_BW_40MHZ;
	} else	{
		mimo_txbw = RSPEC_BW_20MHZ;
	}

	return mimo_txbw;
} /* wlc_d11ac_hdrs_determine_mimo_txbw */
#endif /* WL11N */

/**
 * If the caller decided that an rts or cts frame needs to be transmitted, the transmit properties
 * of the rts/cts have to be determined and communicated to the transmit hardware. RTS/CTS rates are
 * always legacy rates (HR/DSSS, ERP, or OFDM).
 */
static INLINE void
wlc_d11ac_hdrs_rts_cts(struct scb *scb /* [in] */, wlc_bsscfg_t *bsscfg /* [in] */,
	ratespec_t rspec, bool use_rts, bool use_cts,
	ratespec_t *rts_rspec /* [out] */, d11actxh_rate_t *rate_hdr /* [in/out] */,
	uint8 *rts_preamble_type /* [out] */, uint16 *mcl /* [in/out] MacControlLow */)
{
	*rts_preamble_type = WLC_LONG_PREAMBLE;
	/* RTS PLCP header and RTS frame */
	if (use_rts || use_cts) {
		uint16 phy_rate;
		uint8 rts_rate;

		if (use_rts && use_cts)
			use_cts = FALSE;

#ifdef NOT_YET
		/* XXX Optimization: if use rts-cts or cts-to-self,
		 * mixedmode preamble may not be necessary if the dest
		 * support GF. Clear hdr_mcs_mixedmode and reset
		 * short_preamble, recalculating duration fields are
		 * required.
		 */
		if (SCB_ISGF_CAP(scb)) {
			hdr_mcs_mixedmode = FALSE;
			hdr_cckofdm_shortpreamble = TRUE;
		}
#endif /* NOT_YET */

		*rts_rspec = wlc_rspec_to_rts_rspec(bsscfg, rspec, FALSE);
		ASSERT(RSPEC_ISLEGACY(*rts_rspec));

		/* extract the MAC rate in 0.5Mbps units */
		rts_rate = (*rts_rspec & RSPEC_RATE_MASK);

		rate_hdr->RtsCtsControl = IS_CCK(*rts_rspec) ?
			htol16(D11AC_RTSCTS_FRM_TYPE_11B) :
			htol16(D11AC_RTSCTS_FRM_TYPE_11AG);

		/* XXX
		 * OFDM should bypass below preamble set, but old chips are OK
		 * since they ignore that bit. Before 4331A0 PR79339 is fixed,
		 * restrict to CCK, which is actually correct
		 */
		if (!IS_OFDM(*rts_rspec) &&
			!((rts_rate == WLC_RATE_1M) ||
			(scb->bsscfg->PLCPHdr_override == WLC_PLCP_LONG))) {
			*rts_preamble_type = WLC_SHORT_PREAMBLE;
			rate_hdr->RtsCtsControl |= htol16(D11AC_RTSCTS_SHORT_PREAMBLE);
		}

		/* set RTS/CTS flag */
		if (use_cts) {
			rate_hdr->RtsCtsControl |= htol16(D11AC_RTSCTS_USE_CTS);
		} else {
			rate_hdr->RtsCtsControl |= htol16(D11AC_RTSCTS_USE_RTS);
			*mcl |= D11AC_TXC_LFRM;
		}

		/* RTS/CTS Rate index - Bits 3-0 of plcp byte0	*/
		phy_rate = rate_info[rts_rate] & 0xf;
		rate_hdr->RtsCtsControl |= htol16((phy_rate << D11AC_RTSCTS_RATE_SHIFT));
	} else {
		rate_hdr->RtsCtsControl = 0;
	}
} /* wlc_d11ac_hdrs_rts_cts */

/*
 * Add d11actxh_t for 11ac phy
 *
 * 'p' data must start with 802.11 MAC header
 * 'p' must allow enough bytes of local headers to be "pushed" onto the packet
 *
 * headroom == D11AC_TXH_LEN (D11AC_TXH_LEN is now 124 bytes which include PHY PLCP header)
 *
 */
static uint16
wlc_d11ac_hdrs(wlc_info_t *wlc, void *p, struct scb *scb, uint txparams_flags, uint frag,
	uint nfrags, uint queue, uint next_frag_len,
	const wlc_key_info_t *key_info, ratespec_t rspec_override)
{
	struct dot11_header *h;
	d11actxh_t *txh;
	osl_t *osh;
	int len, phylen;
	uint16 fc, type, frameid, mch, phyctl, rate_flag;
	uint16 seq = 0, mcl = 0, status = 0;
	bool use_rts = FALSE;
	bool use_cts = FALSE, rspec_history = FALSE;
	bool use_rifs = FALSE;
	bool short_preamble;
	uint8 preamble_type = WLC_LONG_PREAMBLE;
	uint8 rts_preamble_type;
	uint8 IV_offset = 0;
	struct dot11_rts_frame *rts = NULL;
	ratespec_t rts_rspec = 0;
	bool qos, a4;
	uint8 ac;
	uint txrate;
	wlc_pkttag_t *pkttag;
	ratespec_t rspec;
	ratesel_txparams_t cur_rate;
#if defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED))
	uint16 phyctl_bwo = -1;
	uint16 phyctl_sbwo = -1;
#endif /* defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED)) */
#if WL_HT_TXBW_OVERRIDE_ENAB
	int8 txbw_override_idx;
#endif // endif
#ifdef WL11N
	wlc_ht_info_t *hti = wlc->hti;
	uint8 sgi_tx;
	bool use_mimops_rts = FALSE;
	int rspec_legacy = -1; /* is initialized to prevent compiler warning */
#endif /* WL11N */
	uint16 durid = 0;
	wlc_bsscfg_t *bsscfg;
	bool g_prot;
	int8 n_prot;
	wlc_wme_t *wme;
	uint corerev = wlc->pub->corerev;
	d11actxh_rate_t *rate_blk;
	d11actxh_rate_t *rate_hdr;
	uint8 *plcp;
#ifdef WL_LPC
	uint8 lpc_offset = 0;
#endif // endif
#if defined(WL_BEAMFORMING)
	uint8 bf_shm_index = BF_SHM_IDX_INV, bf_shmx_idx = BF_SHM_IDX_INV;
	bool bfen = FALSE, fbw_bfen = FALSE;
#endif /* WL_BEAMFORMING */
	uint8 fbw = FBW_BW_INVALID; /* fallback bw */
	txpwr204080_t txpwrs;
	int txpwr_bfsel;
	uint8 txpwr;
	int k;
	bool mutx_pkteng_on = FALSE;
	wl_tx_chains_t txbf_chains = 0;
	uint8	bfe_sts_cap = 0, txbf_uidx = 0;
	uint keyinfo_len = 0;
#if defined(WL_PROT_OBSS) && !defined(WL_PROT_OBSS_DISABLED)
	ratespec_t phybw = (CHSPEC_IS8080(wlc->chanspec) || CHSPEC_IS160(wlc->chanspec)) ?
		RSPEC_BW_160MHZ :
		(CHSPEC_IS80(wlc->chanspec) ? RSPEC_BW_80MHZ :
		(CHSPEC_IS40(wlc->chanspec) ? RSPEC_BW_40MHZ : RSPEC_BW_20MHZ));
#endif // endif

	(void)(fbw);

	BCM_REFERENCE(bfe_sts_cap);
	BCM_REFERENCE(txbf_chains);
	BCM_REFERENCE(mutx_pkteng_on);
	ASSERT(scb != NULL);
	ASSERT(queue < WLC_HW_NFIFO_INUSE(wlc));

#ifdef WL_MUPKTENG
	mutx_pkteng_on = wlc_mutx_pkteng_on(wlc->mutx);
#endif // endif
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	short_preamble = (txparams_flags & WLC_TX_PARAMS_SHORTPRE) != 0;

	g_prot = WLC_PROT_G_CFG_G(wlc->prot_g, bsscfg);
	n_prot = WLC_PROT_N_CFG_N(wlc->prot_n, bsscfg);

	wme = bsscfg->wme;

	osh = wlc->osh;

	/* locate 802.11 MAC header */
	h = (struct dot11_header*) PKTDATA(osh, p);
	pkttag = WLPKTTAG(p);
	fc = ltoh16(h->fc);
	type = FC_TYPE(fc);
	qos = (type == FC_TYPE_DATA && FC_SUBTYPE_ANY_QOS(FC_SUBTYPE(fc)));
	a4 = (fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS);

	/* compute length of frame in bytes for use in PLCP computations */
	len = pkttotlen(osh, p);
	phylen = len + DOT11_FCS_LEN;

	/* Add room in phylen for the additional bytes for icv, if required.
	 * this is true for both h/w and s/w encryption. The latter only
	 * modifies the pkt length
	 */
	if (key_info != NULL) {
		if (WLC_KEY_IS_MGMT_GROUP(key_info))
			keyinfo_len = WLC_KEY_MMIC_IE_LEN(key_info);
		else
			keyinfo_len = key_info->icv_len;

		/* external crypto adds iv to the pkt, include it in phylen */
		if (WLC_KEY_IS_LINUX_CRYPTO(key_info))
			keyinfo_len += key_info->iv_len;

		if (WLC_KEY_FRAG_HAS_TKIP_MIC(p, key_info, frag, nfrags))
			keyinfo_len += TKIP_MIC_SIZE;
	}
	phylen += keyinfo_len;

	WL_NONE(("wl%d: %s: len %d, phylen %d\n", WLCWLUNIT(wlc), __FUNCTION__, len, phylen));

	/* add Broadcom tx descriptor header */
	txh = (d11actxh_t*)PKTPUSH(osh, p, D11AC_TXH_LEN);
	bzero((char*)txh, D11AC_TXH_LEN);

	/* MAC-HDR: MacTxControlLow */

	/* set the ignpmq bit for all pkts tx'd in PS mode and for beacons and for anything
	 * going out from a STA interface.
	 */
	if (SCB_PS(scb) || ((fc & FC_KIND_MASK) == FC_BEACON) || BSSCFG_STA(bsscfg))
		mcl |= D11AC_TXC_IPMQ;

#ifdef WL_MUPKTENG
	if (mutx_pkteng_on)
		mcl |= D11AC_TXC_IPMQ;
#endif // endif

	if (frag == 0)
		mcl |= D11AC_TXC_STMSDU;

	/* MacTxControlHigh */
	/* start from fix rate. clear it if auto */
	mch = D11AC_TXC_FIX_RATE;

	/* Chanspec - channel info for packet suppression */
	txh->PktInfo.Chanspec = htol16(wlc->chanspec);

	/* IV Offset, i.e. start of the 802.11 header */
	IV_offset = DOT11_A3_HDR_LEN;
	if (type == FC_TYPE_DATA) {
		if (a4)
			IV_offset += ETHER_ADDR_LEN;
		if (qos)
			IV_offset += DOT11_QOS_LEN;
	} else if (type == FC_TYPE_CTL) {
		/* Subtract one address and SeqNum */
		IV_offset -= ETHER_ADDR_LEN + 2;
	}
	txh->PktInfo.IVOffset = IV_offset;

	/* FrameLen */
	txh->PktInfo.FrameLen = htol16((uint16)phylen);

#ifdef PSPRETEND
	/* If D11AC_TXC_PPS is set, ucode is monitoring for failed TX status. In that case,
	 * it will add new PMQ entry so start the process of draining the TX fifo.
	 * Of course, PS pretend should be in enabled state and we guard against the "ignore PMQ"
	 * because this is used to force a packet to air.
	 */
	/* Do not allow IBSS bsscfg to use normal pspretend yet,
	 * as PMQ process is not supported for them now.
	 */
	if (SCB_PS_PRETEND_ENABLED(scb) && !BSSCFG_IBSS(bsscfg) &&
		!(mcl & D11AC_TXC_IPMQ) && (type == FC_TYPE_DATA)) {
		mch |= D11AC_TXC_PPS;
	}
#endif // endif

	/* use preassigned or software seqnum */
#ifdef PROP_TXSTATUS
	if (WL_SEQ_GET_FROMDRV(pkttag->seq)) {
		seq = WL_SEQ_GET_NUM(pkttag->seq);
	} else if (WLPKTFLAG_AMPDU(pkttag)) {
		seq = WL_SEQ_GET_NUM(pkttag->seq);
	}
#else
	if (WLPKTFLAG_AMPDU(pkttag)) {
		seq = pkttag->seq;
	}
#endif /* PROP_TXSTATUS */
	else if (SCB_QOS(scb) && ((fc & FC_KIND_MASK) == FC_QOS_DATA) &&
		!ETHER_ISMULTI(&h->a1)) {
		seq = SCB_SEQNUM(scb, PKTPRIO(p));
		/* Increment the sequence number only after the last fragment */
		if (frag == (nfrags - 1))
			SCB_SEQNUM(scb, PKTPRIO(p))++;
	} else if (type != FC_TYPE_CTL)
		mcl |= D11AC_TXC_ASEQ;

	if (type != FC_TYPE_CTL) {
		seq = (seq << SEQNUM_SHIFT) | (frag & FRAGNUM_MASK);
		h->seq = htol16(seq);
	}

#ifdef PROP_TXSTATUS
	if (WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
		WL_SEQ_SET_NUM(pkttag->seq, ltoh16(h->seq) >> SEQNUM_SHIFT);
		WL_SEQ_SET_FROMFW(pkttag->seq, 1);
	}
#endif /* PROP_TXSTATUS */
	/* setup frameid, also possibly change seq */
	if (queue == TX_BCMC_FIFO) {
		frameid = bcmc_fid_generate(wlc, bsscfg, txh->PktInfo.TxFrameID);
	} else {
		frameid = (((wlc->counter++) << TXFID_SEQ_SHIFT) & TXFID_SEQ_MASK) |
			WLC_TXFID_SET_QUEUE(queue);
	}

	/* TDLS U-APSD buffer STA: if peer is in PS, save the last MPDU's seq and tid for PTI */
#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, SCB_BSSCFG(scb)) &&
		SCB_PS(scb) &&
		wlc_tdls_buffer_sta_enable(wlc->tdls)) {
		uint8 tid = 0;
		if (qos) {
			uint16 qc;
			qc = (*(uint16 *)((uchar *)h + (a4 ?
				DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN)));
			tid = (QOS_TID(qc) & 0x0f);
		}
		wlc_tdls_update_tid_seq(wlc->tdls, scb, tid, seq);
	}
#endif // endif

	/* Sequence number final write */
	txh->PktInfo.Seq = h->seq;

	/* Timestamp */
	if ((pkttag->flags & WLF_EXPTIME)) {
		txh->PktInfo.Tstamp = htol16((pkttag->u.exptime >> D11AC_TSTAMP_SHIFT) & 0xffff);
		mcl |= D11AC_TXC_AGING; /* Enable timestamp for the packet */
	}

	/* TxStatus, Note the case of recreating the first frag of a suppressed frame
	 * then we may need to reset the retry cnt's via the status reg
	 */
	txh->PktInfo.TxStatus = htol16(status);

	ac = WME_PRIO2AC(PKTPRIO(p));

	/* (1) RATE: determine and validate primary rate and fallback rates */

	cur_rate.num = 1; /* default to 1 */

	if (RSPEC_ACTIVE(rspec_override)) {
		cur_rate.rspec[0] = rspec_override;
	}
	else if ((type == FC_TYPE_MNG) ||
		(type == FC_TYPE_CTL) ||
		(pkttag->flags & WLF_8021X) ||
#ifdef BCMWAPI_WAI
		(pkttag->flags & WLF_WAI) ||
#endif // endif
		FALSE) {
		if ((fc & FC_KIND_MASK) == FC_PROBE_RESP) {
			cur_rate.rspec[0] = scb->rateset.rates[scb->basic_rate_indx] & RATE_MASK;
		} else {
			cur_rate.rspec[0] = scb->rateset.rates[0] & RATE_MASK;
		}
	} else if (RSPEC_ACTIVE(wlc->band->mrspec_override) && ETHER_ISMULTI(&h->a1)) {
		cur_rate.rspec[0] = wlc->band->mrspec_override;
	} else if (RSPEC_ACTIVE(wlc->band->rspec_override) && !ETHER_ISMULTI(&h->a1)) {
		cur_rate.rspec[0] = wlc->band->rspec_override;
	} else if (ETHER_ISMULTI(&h->a1) || SCB_INTERNAL(scb)) {
		cur_rate.rspec[0] = scb->rateset.rates[0] & RATE_MASK;
	} else {
		/* run rate algorithm for data frame only, a cookie will be deposited in frameid */
		cur_rate.num = 4; /* enable multi fallback rate */
		cur_rate.ac = ac;
		wlc_scb_ratesel_gettxrate(wlc->wrsi, scb, &frameid, &cur_rate, &rate_flag);

		if (((scb->flags & SCB_BRCM) == 0) &&
		    (((fc & FC_KIND_MASK) == FC_NULL_DATA) ||
		     ((fc & FC_KIND_MASK) == FC_QOS_NULL))) {
			/* Use RTS/CTS rate for NULL frame */
			cur_rate.num = 2;
			cur_rate.rspec[0] =
				wlc_rspec_to_rts_rspec(bsscfg, cur_rate.rspec[0], FALSE);
			cur_rate.rspec[1] = scb->rateset.rates[0] & RATE_MASK;
		} else {
			pkttag->flags |= WLF_RATE_AUTO;

			/* The rate histo is updated only for packets on auto rate. */
			/* perform rate history after txbw has been selected */
			if (frag == 0)
				rspec_history = TRUE;
		}

		mch &= ~D11AC_TXC_FIX_RATE;
#ifdef WL11N
		if (rate_flag & RATESEL_VRATE_PROBE)
			WLPKTTAG(p)->flags |= WLF_VRATE_PROBE;
#endif /* WL11N */

#ifdef WL_LPC
		if (LPC_ENAB(wlc)) {
			/* Query the power offset to be used from LPC */
			lpc_offset = wlc_scb_lpc_getcurrpwr(wlc->wlpci, scb, ac);
		} else {
			/* No Link Power Control. Transmit at nominal power. */
		}
#endif // endif
	}

#ifdef WL_RELMCAST
	if (RMC_ENAB(wlc->pub))
		wlc_rmc_process(wlc->rmc, type, p, h, &mcl, &cur_rate.rspec[0]);
#endif // endif

#ifdef WL_MU_TX
	bf_shmx_idx = wlc_txbf_get_mubfi_idx(wlc->txbf, scb);
	if (bf_shmx_idx != BF_SHM_IDX_INV) {
		/* link is capable of using mu sounding enabled in shmx bfi interface  */
		mch |= D11AC_TXC_BFIX;
		if (SCB_MU(scb) && WLPKTFLAG_AMPDU(pkttag) && RSPEC_ISVHT(cur_rate.rspec[0]) &&
#if defined(BCMDBG) || defined(BCMDBG_MU)
		wlc_mutx_on(wlc->mutx) &&
#endif // endif
		queue >= TX_FIFO_MU_START) {
			/* link is capable of mutx and has been enabled to do mutx */
			mch |= D11AC_TXC_MU;
#ifdef WL_MUPKTENG
			if (mutx_pkteng_on) {
				mcl |=  D11AC_TXC_AMPDU;
				mch &= ~D11AC_TXC_SVHT;
			}
#endif // endif
		}
	}
#endif /* WL_MU_TX */

	rate_blk = WLC_TXD_RATE_INFO_GET(txh, corerev);
	for (k = 0; k < cur_rate.num; k++) {
		/* init primary and fallback rate pointers */
		rspec = cur_rate.rspec[k];
		rate_hdr = &rate_blk[k];

		plcp = rate_hdr->plcp;
		rate_hdr->RtsCtsControl = 0;

#ifdef WL11N
		if (N_ENAB(wlc->pub)) {
			uint32 mimo_txbw;

			rspec_legacy = RSPEC_ISLEGACY(rspec);

			mimo_txbw = wlc_d11ac_hdrs_determine_mimo_txbw(wlc, scb, bsscfg, rspec);
#if WL_HT_TXBW_OVERRIDE_ENAB
			if (CHSPEC_IS40(wlc->chanspec) || CHSPEC_IS80(wlc->chanspec)) {
				WL_HT_TXBW_OVERRIDE_IDX(hti, rspec, txbw_override_idx);

				if (txbw_override_idx >= 0) {
					mimo_txbw = txbw2rspecbw[txbw_override_idx];
					phyctl_bwo = txbw2acphyctl0bw[txbw_override_idx];
					phyctl_sbwo = txbw2acphyctl1bw[txbw_override_idx];
				}
			}
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */

#ifdef WL_OBSS_DYNBW
			if (WLC_OBSS_DYNBW_ENAB(wlc->pub)) {
				wlc_obss_dynbw_tx_bw_override(wlc->obss_dynbw, bsscfg,
					&mimo_txbw);
			}
#endif /* WL_OBSS_DYNBW */

			rspec &= ~RSPEC_BW_MASK;
			rspec |= mimo_txbw;
		} else /* N_ENAB */
#endif /* WL11N */
		{
			/* Set ctrlchbw as 20Mhz */
			ASSERT(RSPEC_ISLEGACY(rspec));
			rspec &= ~RSPEC_BW_MASK;
			rspec |= RSPEC_BW_20MHZ;
		}

#ifdef WL11N
		if (N_ENAB(wlc->pub)) {
			uint8 mimo_preamble_type;

			sgi_tx = WLC_HT_GET_SGI_TX(hti);

			/* XXX 4360: consider having SGI always come from the rspec
			 * instead of calculating here
			 */
			if (!RSPEC_ACTIVE(wlc->band->rspec_override)) {
				bool _scb_stbc_on = FALSE;
				bool stbc_tx_forced = WLC_IS_STBC_TX_FORCED(wlc);
				bool stbc_ht_scb_auto = WLC_STF_SS_STBC_HT_AUTO(wlc, scb);
				bool stbc_vht_scb_auto = WLC_STF_SS_STBC_VHT_AUTO(wlc, scb);

				if (sgi_tx == OFF) {
					rspec &= ~RSPEC_SHORT_GI;
				} else if (sgi_tx == ON) {
					if (RSPEC_ISHT(rspec) || RSPEC_ISVHT(rspec)) {
						rspec |= RSPEC_SHORT_GI;
					}
				}

				/* XXX move this LDPC decision outside, i.e. turn off when power
				 * is a concern ?
				 */
				/* XXX 4360: consider having LDPC always come from the rspec
				 * instead of calculating here
				 */

				ASSERT(!(rspec & RSPEC_LDPC_CODING));
				rspec &= ~RSPEC_LDPC_CODING;
				rspec &= ~RSPEC_STBC;

				/* LDPC */
				if (wlc->stf->ldpc_tx == ON ||
				    (((RSPEC_ISVHT(rspec) && SCB_VHT_LDPC_CAP(wlc->vhti, scb)) ||
				      (RSPEC_ISHT(rspec) && SCB_LDPC_CAP(scb))) &&
				     wlc->stf->ldpc_tx == AUTO)) {
					if (!rspec_legacy)
						rspec |= RSPEC_LDPC_CODING;
				}

				/* STBC */
				if ((wlc_ratespec_nsts(rspec) == 1)) {
					if (stbc_tx_forced ||
					  ((RSPEC_ISHT(rspec) && stbc_ht_scb_auto) ||
					   (RSPEC_ISVHT(rspec) && stbc_vht_scb_auto))) {
						_scb_stbc_on = TRUE;
					}

					/* XXX: CRDOT11ACPHY-1826 : Disable STBC when mcs > 7
					 * if rspec is 160Mhz for 80p80phy.
					 */
					if (WLC_PHY_AS_80P80(wlc, wlc->chanspec) &&
						RSPEC_ISVHT(rspec) && RSPEC_IS160MHZ(rspec)) {
						uint mcs = (rspec & RSPEC_VHT_MCS_MASK);
						if (mcs > 7)
							_scb_stbc_on = FALSE;
					}

					if (_scb_stbc_on && !rspec_legacy)
						rspec |= RSPEC_STBC;
				}
			}

#ifdef DYN160
			/* When DYN160 enabled, set the NSS in rspec as per op_txstreams */
			if (DYN160_ACTIVE(wlc->pub) && RSPEC_ISVHT(rspec)) {
				uint8 nss = (rspec & RSPEC_VHT_NSS_MASK) >> RSPEC_VHT_NSS_SHIFT;
				if (nss > wlc->stf->op_txstreams) {
					rspec &= ~RSPEC_VHT_NSS_MASK;
					rspec |= ((wlc->stf->op_txstreams) << RSPEC_VHT_NSS_SHIFT);
				}
			}
#endif /* DYN160 */

			/* Determine the HT frame format, HT_MM or HT_GF */
			if (RSPEC_ISHT(rspec)) {
				int nsts;

				mimo_preamble_type = wlc_prot_n_preamble(wlc, scb);
				if (RSPEC_ISHT(rspec)) {
					nsts = wlc_ratespec_nsts(rspec);
					preamble_type = mimo_preamble_type;
					/* XXX 4360: might need the same 20in40 check
					 * and protection for VHT case
					 */
					if (n_prot == WLC_N_PROTECTION_20IN40 &&
						RSPEC_IS40MHZ(rspec))
						use_cts = TRUE;

					/* XXX 4360: might need the same mimops check
					 * for VHT case
					 */
					/* if the mcs is multi stream check if it needs
					 * an rts
					 */
					if (nsts > 1) {
						if (WLC_HT_GET_SCB_MIMOPS_ENAB(hti, scb) &&
							WLC_HT_GET_SCB_MIMOPS_RTS_ENAB(hti, scb))
							use_rts = use_mimops_rts = TRUE;
					}

					/* if SGI is selected, then forced mm for single
					 * stream
					 * (spec section 9.16, Short GI operation)
					 */
					if (RSPEC_ISSGI(rspec) && nsts == 1) {
						preamble_type = WLC_MM_PREAMBLE;
					}
				}
			} else {
				/* VHT always uses MM preamble */
				if (RSPEC_ISVHT(rspec)) {
					preamble_type = WLC_MM_PREAMBLE;
				}
			}
		}
#endif /* WL11N */

		if (wlc->pub->_dynbw == TRUE) {
#ifdef WL11N
			if (RSPEC_BW(rspec) == RSPEC_BW_80MHZ)
				fbw = BW_40MHZ;
			else if (RSPEC_BW(rspec) == RSPEC_BW_40MHZ) {
#ifdef WL11AC
				if (RSPEC_ISVHT(rspec)) {
					uint8 mcs, nss, prop_mcs = VHT_PROP_MCS_MAP_NONE;
					bool ldpc;
					uint16 mcsmap = 0;
					uint8 vht_ratemask =
						wlc_vht_get_scb_ratemask(wlc->vhti, scb);
					mcs = rspec & RSPEC_VHT_MCS_MASK;
					nss = (rspec & RSPEC_VHT_NSS_MASK) >> RSPEC_VHT_NSS_SHIFT;
					ldpc = (rspec & RSPEC_LDPC_CODING) ? TRUE : FALSE;
					mcs = VHT_MCS_MAP_GET_MCS_PER_SS(nss, vht_ratemask);
					if (vht_ratemask & WL_VHT_FEATURES_1024QAM)
						prop_mcs = VHT_PROP_MCS_MAP_10_11;
					mcsmap = wlc_get_valid_vht_mcsmap(mcs, prop_mcs, BW_20MHZ,
						ldpc, nss, vht_ratemask);
					if (mcsmap & (1 << mcs))
						fbw = BW_20MHZ;
				} else
#endif /* WL11AC */
					fbw = BW_20MHZ;
			}
#endif /* WL11N */
		}

		/* - BW_20MHZ to make it 0-index based */
		rate_hdr->FbwInfo = (((fbw - BW_20MHZ) & FBW_BW_MASK) << FBW_BW_SHIFT);

		/* (2) PROTECTION, may change rspec */
		if (((type == FC_TYPE_DATA) || (type == FC_TYPE_MNG)) &&
			((phylen > wlc->RTSThresh) || (pkttag->flags & WLF_USERTS)) &&
			!ETHER_ISMULTI(&h->a1))
			use_rts = TRUE;

		if ((wlc->band->gmode && g_prot && IS_OFDM(rspec)) ||
			((RSPEC_ISHT(rspec) || RSPEC_ISVHT(rspec)) && n_prot)) {
			if (nfrags > 1) {
				/* For a frag burst use the lower modulation rates for the
				 * entire frag burst instead of protection mechanisms.
				 * As per spec, if protection mechanism is being used, a
				 * fragment sequence may only employ ERP-OFDM modulation for
				 * the final fragment and control response. (802.11g Sec 9.10)
				 * For ease we send the *whole* sequence at the
				 * lower modulation instead of using a higher modulation for the
				 * last frag.
				 */

				/* downgrade the rate to CCK or OFDM */
				if (g_prot) {
					/* Use 11 Mbps as the rate and fallback. We should make
					 * sure that if we are downgrading an OFDM rate to CCK,
					 * we should pick a more robust rate.  6 and 9 Mbps are not
					 * usually selected by rate selection, but even if the OFDM
					 * rate we are downgrading is 6 or 9 Mbps, 11 Mbps is more
					 * robust.
					 */
					rspec = CCK_RSPEC(WLC_RATE_11M);
				} else {
					/* Use 24 Mbps as the rate and fallback for what would have
					 * been a MIMO rate. 24 Mbps is the highest phy mandatory
					 * rate for OFDM.
					 */
					rspec = OFDM_RSPEC(WLC_RATE_24M);
				}
				pkttag->flags &= ~WLF_RATE_AUTO;
			} else {
				/* Use protection mechanisms on unfragmented frames */
				/* If this is a 11g protection, then use CTS-to-self */
				if (wlc->band->gmode && g_prot && !IS_CCK(rspec))
					use_cts = TRUE;
			}
		}

		/* calculate minimum rate */
		ASSERT(VALID_RATE_DBG(wlc, rspec));

		/* fix the preamble type for non-MCS rspec/rspec-fallback
		 * If SCB is short preamble capable and and shortpreamble is enabled
		 * and rate is NOT 1M or Override is NOT set to LONG preamble then use SHORT
		 * preamble else use LONG preamble
		 * OFDM should bypass below preamble set, but old chips are OK since they ignore
		 * that bit XXX before 4331A0 PR79339 is fixed, restrict to CCK, which is actually
		 * correct
		 */
		if (IS_CCK(rspec)) {
			if (short_preamble &&
				!((RSPEC2RATE(rspec) == WLC_RATE_1M) ||
				(scb->bsscfg->PLCPHdr_override == WLC_PLCP_LONG)))
				preamble_type = WLC_SHORT_PREAMBLE;
			else
				preamble_type = WLC_LONG_PREAMBLE;
		}

		ASSERT(RSPEC_ISLEGACY(rspec) || WLC_IS_MIMO_PREAMBLE(preamble_type));

#if defined(WL_BEAMFORMING)
		if (TXBF_ENAB(wlc->pub)) {
			/* bfe_sts_cap = 3: foursteams, 2: three streams, 1: two streams */
			bfe_sts_cap  = wlc_txbf_get_bfe_sts_cap(wlc->txbf, scb);
			if (bfe_sts_cap && (D11REV_LE(wlc->pub->corerev, 64) ||
				!wlc_txbf_bfrspexp_enable(wlc->txbf))) {
				/* Explicit TxBF: Number of txbf chains
				 * is min(#active txchains, #bfe sts + 1)
				 */
				txbf_chains = MIN((uint8)WLC_BITSCNT(wlc->stf->txchain),
						(bfe_sts_cap + 1));
			} else {
				/* bfe_sts_cap=0 indicates there is no Explicit TxBf link to this
				 * peer, and driver will probably use Implicit TxBF. Ignore the
				 * spatial_expension policy, and always use all currently enabled
				 * txcores
				 */
				txbf_chains = (uint8)WLC_BITSCNT(wlc->stf->txchain);
			}
		}
#endif /* WL_BEAMFORMING */
		/* get txpwr for bw204080 and txbf on/off */
		if (wlc_stf_get_204080_pwrs(wlc, rspec, &txpwrs, txbf_chains) != BCME_OK) {
			ASSERT(!"phyctl1 ppr returns error!");
		}

		txpwr_bfsel = 0;
#if defined(WL_BEAMFORMING)
		if (TXBF_ENAB(wlc->pub) &&
			(wlc->allow_txbf) &&
			(preamble_type != WLC_GF_PREAMBLE) &&
			!SCB_ISMULTI(scb) &&
			(type == FC_TYPE_DATA) &&
			!WLC_PHY_AS_80P80(wlc, wlc->chanspec)) {
			ratespec_t fbw_rspec;
			uint16 txpwr_mask, stbc_val;
			fbw_rspec = rspec;
			bfen = wlc_txbf_sel(wlc->txbf, rspec, scb, &bf_shm_index, &txpwrs);

			if (bfen) {
				/* BFM0: Provide alternative tx info if phyctl has bit 3
				 * (BFM) bit
				 * set but ucode has to tx with BFM cleared.
				 */
				if (D11REV_GE(corerev, 64)) {
					/* acphy2 mac/phy interface */
					txpwr_mask = D11AC2_BFM0_TXPWR_MASK;
					stbc_val = D11AC2_BFM0_STBC;
				} else {
					txpwr_mask = BFM0_TXPWR_MASK;
					stbc_val = BFM0_STBC;
				}
				rate_hdr->Bfm0 =
					((uint16)(txpwrs.pbw[((rspec & RSPEC_BW_MASK) >>
					RSPEC_BW_SHIFT) - BW_20MHZ][TXBF_OFF_IDX])) & txpwr_mask;
				rate_hdr->Bfm0 |=
					(uint16)(RSPEC_ISSTBC(rspec) ? stbc_val : 0);
			}

			txpwr_bfsel = bfen ? 1 : 0;

			if (fbw != FBW_BW_INVALID) {
				fbw_rspec = (fbw_rspec & ~RSPEC_BW_MASK) |
				((fbw == BW_40MHZ) ?  RSPEC_BW_40MHZ : RSPEC_BW_20MHZ);
				if (!RSPEC_ISSTBC(fbw_rspec))
					fbw_bfen = wlc_txbf_sel(wlc->txbf, fbw_rspec, scb,
						&bf_shm_index, &txpwrs);
				else
					fbw_bfen = bfen;

				rate_hdr->FbwInfo |= (uint16)(fbw_bfen ? FBW_TXBF : 0);
			}

			if (RSPEC_ACTIVE(wlc->band->rspec_override)) {
				wlc_txbf_applied2ovr_upd(wlc->txbf, bfen);
			}
		}
#endif /* WL_BEAMFORMING */

		/* (3) PLCP: determine PLCP header and MAC duration, fill d11txh_t */
		wlc_compute_plcp(wlc, rspec, phylen, fc, plcp);

		/* RateInfo.TxRate */
		txrate = wlc_rate_rspec2rate(rspec);
		rate_hdr->TxRate = htol16(txrate/500);

		/* RIFS(testing only): based on frameburst, non-CCK frames only */
		if (SCB_HT_CAP(scb) && WLC_HT_GET_FRAMEBURST(hti) &&
			WLC_HT_GET_RIFS(hti) &&
			n_prot != WLC_N_PROTECTION_MIXEDMODE && !IS_CCK(rspec) &&
			!ETHER_ISMULTI(&h->a1) && ((fc & FC_KIND_MASK) == FC_QOS_DATA)) {
			uint16 qos_field, *pqos;

			WLPKTTAG(p)->flags |= WLF_RIFS;
			mcl |= (D11AC_TXC_MBURST | D11AC_TXC_URIFS);
			use_rifs = TRUE;

			/* RIFS implies QoS frame with no-ack policy, hack the QoS field */
			pqos = (uint16 *)((uchar *)h + (a4 ?
				DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN));
			qos_field = ltoh16(*pqos) & ~QOS_ACK_MASK;
			qos_field |= (QOS_ACK_NO_ACK << QOS_ACK_SHIFT) & QOS_ACK_MASK;
			*pqos = htol16(qos_field);
		}

#ifdef WLAMPDU
		/* mark pkt as aggregable if it is */
		if ((k == 0) && WLPKTFLAG_AMPDU(pkttag) && !RSPEC_ISLEGACY(rspec)) {
			if (WLC_KEY_ALLOWS_AMPDU(key_info)) {
				WLPKTTAG(p)->flags |= WLF_MIMO;
				if (WLC_HT_GET_AMPDU_RTS(wlc->hti))
					use_rts = TRUE;
			}
		}
#endif /* WLAMPDU */

		/* DUR field for main rate */
		if (((fc & FC_KIND_MASK) != FC_PS_POLL) && !ETHER_ISMULTI(&h->a1) &&
			!use_rifs) {
			durid = wlc_compute_frame_dur(wlc, rspec, preamble_type, next_frag_len);
			h->durid = htol16(durid);
		}
		else if (use_rifs) {
			/* NAV protect to end of next max packet size */
			durid = (uint16)wlc_calc_frame_time(wlc, rspec, preamble_type,
				DOT11_MAX_FRAG_LEN);
			durid += RIFS_11N_TIME;
			h->durid = htol16(durid);
		}

		if (!ETHER_ISMULTI(&h->a1) && !WLPKTFLAG_RIFS(pkttag) &&
			!(wlc->_amsdu_noack && WLPKTFLAG_AMSDU(pkttag)) &&
			!(!WLPKTFLAG_AMPDU(pkttag) && qos && wme->wme_noack))
			mcl |= D11AC_TXC_IACK;

		if (type == FC_TYPE_DATA) {
			if ((WLC_HT_GET_FRAMEBURST(hti) &&
				(txrate > WLC_FRAMEBURST_MIN_RATE) &&
#ifdef WLAMPDU
				AMPDU_ENAB(wlc->pub) && SCB_AMPDU(scb) &&
				(wlc_ampdu_frameburst_override(wlc->ampdu_tx) == FALSE) &&
#endif // endif
				(!wme->edcf_txop[ac] || WLPKTFLAG_AMPDU(pkttag))) ||
#ifdef WLBTAMP
				(BTA_ENAB(wlc->pub) &&
				wlc_bta_frameburst_active(wlc->bta, pkttag, txrate)) ||
#endif // endif
				FALSE)
#ifdef WL11N
				/* dont allow bursting if rts is required around each mimo frame */
				if (use_mimops_rts == FALSE)
#endif // endif
					mcl |= D11AC_TXC_MBURST;
		}

		/* XXX 4360: VHT rates are always in an AMPDU, setup the TxD fields here
		 * By default use Single-VHT mpdu ampdu mode.
		 */
		if (RSPEC_ISVHT(rspec)) {
			mch |= D11AC_TXC_SVHT;
		}

#if defined(WL_PROT_OBSS) && !defined(WL_PROT_OBSS_DISABLED)
		/* If OBSS protection is enabled, set CTS/RTS accordingly. */
		if (WLC_PROT_OBSS_PROTECTION(wlc->prot_obss) && !use_rts && !use_cts) {
			if (ETHER_ISMULTI(&h->a1)) {
				/* Multicast and broadcast pkts need CTS protection */
				use_cts = TRUE;
			} else {
				/* Unicast pkts < 80 bw need RTS protection */
				if (RSPEC_BW(rspec) < phybw) {
					use_rts = TRUE;
				}
			}
		}
#endif /* WL_PROT_OBSS && !WL_PROT_OBSS_DISABLED */
#ifdef WLAWDL
		if (txparams_flags & WLC_TX_PARAMS_AWDL_AF_TS) {
			mch |= D11AC_TXC_AWDL_PHYTT;
			WL_AWDL(("%s(): set D11AC_TXC_AWDL_PHYTT mch = %x\n",
				__FUNCTION__, mch));
		}
#endif /* WLAWDL */

		/* (5) RTS/CTS: determine RTS/CTS PLCP header and MAC duration, furnish d11txh_t */
		wlc_d11ac_hdrs_rts_cts(scb, bsscfg, rspec, use_rts, use_cts, &rts_rspec, rate_hdr,
		                       &rts_preamble_type, &mcl);
#ifdef WL_BEAMFORMING
		if (TXBF_ENAB(wlc->pub)) {
			bool mutx_on = ((k == 0) && (mch & D11AC_TXC_MU));
			if (bfen || mutx_on) {
				if (bf_shm_index != BF_SHM_IDX_INV) {
					rate_hdr->RtsCtsControl |= htol16((bf_shm_index <<
						D11AC_RTSCTS_BF_IDX_SHIFT));
				}
				else if (mutx_on) {
					/* Allow MU txbf even if SU explicit txbf is not allowed.
					 * If still implicit SU txbf capable and if we end up
					 * doing it, we won't have its index in the header
					 * but it is okay as ucode correctly calculates it.
					 */
					rate_hdr->RtsCtsControl |= htol16((bf_shmx_idx <<
						D11AC_RTSCTS_BF_IDX_SHIFT));
					WL_TXBF(("wl:%d %s MU txbf capable with %s SU txbf\n",
						wlc->pub->unit, __FUNCTION__,
						(bfen ? "IMPLICIT" : "NO")));
				} else {
					rate_hdr->RtsCtsControl |= htol16(D11AC_RTSCTS_IMBF);
				}
				/* Move wlc_txbf_fix_rspec_plcp() here to remove RSPEC_STBC
				 * from rspec before wlc_acphy_txctl0_calc_ex().
				 * rate_hdr->PhyTxControlWord_0 will not set D11AC2_PHY_TXC_STBC
				 * if this rate enable txbf.
				 */
				wlc_txbf_fix_rspec_plcp(wlc->txbf, &rspec, plcp, txbf_chains);
			}
		}
#endif /* WL_BEAMFORMING */
		/* PhyTxControlWord_0 */
		phyctl = wlc_acphy_txctl0_calc_ex(wlc, rspec, preamble_type);

		if (WLC_PHY_AS_80P80(wlc, wlc->chanspec) && !RSPEC_IS160MHZ(rspec) &&
			!IS_OFDM(rspec)) {
			phyctl = wlc_stf_d11hdrs_phyctl_txcore_80p80phy(wlc, phyctl);
		}

		WLC_PHYCTLBW_OVERRIDE(phyctl, D11AC_PHY_TXC_BW_MASK, phyctl_bwo);
#if defined(WL_PROXDETECT) && !defined(WL_PROXDETECT_DISABLED)
		if (PROXD_ENAB(wlc->pub) && FC_SUBTYPE(fc) == FC_SUBTYPE_ACTION &&
			type == FC_TYPE_MNG) {
			/* TOF measurement pkts use only primary antenna to tx */
			wlc_proxd_tx_conf(wlc, &phyctl, &mch, pkttag);
		}
#endif // endif
#ifdef WL_BEAMFORMING
		if (bfen) {
			phyctl |= (D11AC_PHY_TXC_BFM);
		} else {
			if (wlc_txbf_bfmspexp_enable(wlc->txbf) &&
			(preamble_type != WLC_GF_PREAMBLE) &&
			(!RSPEC_ISSTBC(rspec)) && IS_MCS(rspec) &&
			!WLC_PHY_AS_80P80(wlc, wlc->chanspec)) {
				uint8 nss = (uint8) wlc_ratespec_nss(rspec);
				uint8 ntx = (uint8) wlc_stf_txchain_get(wlc, rspec);

				if (nss == 3 && ntx == 4) {
					txbf_uidx = 96;
				} else if (nss == 2 && ntx == 4) {
					txbf_uidx = 97;
				} else if (nss == 2 && ntx == 3) {
					txbf_uidx = 98;
				}
				/* no need to set BFM (phytxctl.B3) as ucode will do it */
			}
		}
#endif /* WL_BEAMFORMING */

		rate_hdr->PhyTxControlWord_0 = htol16(phyctl);

		/* PhyTxControlWord_1 */
		txpwr = txpwrs.pbw[((rspec & RSPEC_BW_MASK) >> RSPEC_BW_SHIFT) -
			BW_20MHZ][txpwr_bfsel];
		if (D11REV_GE(corerev, 40)
				&& (FC_TYPE(fc) == FC_TYPE_MNG) && (FC_SUBTYPE(fc) == FC_SUBTYPE_PROBE_RESP)) {
			txpwr = wlc_get_bcnprs_txpwr_offset(wlc, txpwr);
		}
#ifdef WL_LPC
		/* Apply the power index */
		if ((pkttag->flags & WLF_RATE_AUTO) && LPC_ENAB(wlc)) {
			phyctl = wlc_acphy_txctl1_calc_ex(wlc, rspec, lpc_offset, txpwr, FALSE);
			/* Preserve the PhyCtl for later (dotxstatus) */
			wlc_scb_lpc_store_pcw(wlc->wlpci, scb, ac, phyctl);
		} else
#endif // endif
		{
			phyctl = wlc_acphy_txctl1_calc_ex(wlc, rspec, 0, txpwr, FALSE);
		}

		/* for fallback bw */
		if (fbw != FBW_BW_INVALID) {
			txpwr = txpwrs.pbw[fbw - BW_20MHZ][TXBF_OFF_IDX];
			rate_hdr->FbwInfo |= ((uint16)txpwr << FBW_BFM0_TXPWR_SHIFT);
#ifdef WL_BEAMFORMING
			if (fbw_bfen) {
				txpwr = txpwrs.pbw[fbw - BW_20MHZ][TXBF_ON_IDX];
				rate_hdr->FbwInfo |= ((uint16)txpwr << FBW_BFM_TXPWR_SHIFT);
			}
#endif // endif
			rate_hdr->FbwInfo = htol16(rate_hdr->FbwInfo);
		}

		WLC_PHYCTLBW_OVERRIDE(phyctl, D11AC_PHY_TXC_PRIM_SUBBAND_MASK, phyctl_sbwo);
		rate_hdr->PhyTxControlWord_1 = htol16(phyctl);

		/* PhyTxControlWord_2 */
		phyctl = wlc_acphy_txctl2_calc_ex(wlc, rspec, txbf_uidx);
		rate_hdr->PhyTxControlWord_2 = htol16(phyctl);

		/* Avoid changing TXOP threshold based on multicast packets */
		if ((k == 0) && SCB_WME(scb) && qos && wme->edcf_txop[ac] &&
			!ETHER_ISMULTI(&h->a1)) {
			uint frag_dur, dur;

			/* WME: Update TXOP threshold */
			/*
			 * XXX should this calculation be done for null frames also?
			 * What else is wrong with these calculations?
			 */
			if ((!WLPKTFLAG_AMPDU(pkttag)) && (frag == 0)) {
				int16 delta;

				frag_dur = wlc_calc_frame_time(wlc, rspec, preamble_type, phylen);

				if (rts) {
					/* 1 RTS or CTS-to-self frame */
					dur = wlc_calc_cts_time(wlc, rts_rspec, rts_preamble_type);
					/* (SIFS + CTS) + SIFS + frame + SIFS + ACK */
					dur += ltoh16(rts->durid);
				} else if (use_rifs) {
					dur = frag_dur;
				} else {
					/* frame + SIFS + ACK */
					dur = frag_dur;
					dur += wlc_compute_frame_dur(wlc, rspec, preamble_type, 0);

				}

				/* update txop byte threshold (txop minus intraframe overhead) */
				delta = (int16)(wme->edcf_txop[ac] - (dur - frag_dur));
				if (delta >= 0) {
#ifdef WLAMSDU_TX
					if (WLPKTFLAG_AMSDU(pkttag) && (queue == TX_AC_BE_FIFO)) {
						WL_ERROR(("edcf_txop changed, update AMSDU\n"));
						wlc_amsdu_txop_upd(wlc->ami);
					} else
#endif // endif
					{
						uint newfragthresh =
							wlc_calc_frame_len(wlc, rspec,
							preamble_type, (uint16)delta);
						/* range bound the fragthreshold */
						newfragthresh =
							MAX(newfragthresh, DOT11_MIN_FRAG_LEN);
						newfragthresh =
							MIN(newfragthresh, wlc->usr_fragthresh);
						/* update the fragthresh and do txc update */
						if (wlc->fragthresh[ac] != (uint16)newfragthresh)
							wlc_fragthresh_set(wlc, ac,
							   (uint16)newfragthresh);
					}
				} else
					WL_ERROR(("wl%d: %s txop invalid for rate %d\n",
					wlc->pub->unit, fifo_names[queue], RSPEC2RATE(rspec)));

				if (CAC_ENAB(wlc->pub) &&
					queue <= TX_AC_VO_FIFO) {
					wlc_cac_update_dur_cache(wlc->cac, ac, PKTPRIO(p), scb, dur,
						(phylen - keyinfo_len), WLC_CAC_DUR_CACHE_PREP);
					/* update cac used time */
					if (wlc_cac_update_used_time(wlc->cac, ac, dur, scb))
						WL_ERROR(("wl%d: ac %d: txop exceeded allocated TS"
							"time\n", wlc->pub->unit, ac));
				}

				/*
				 * FIXME: The MPDUs of the next transmitted MSDU after
				 * rate drops or RTS/CTS kicks in may exceed
				 * TXOP. Without tearing apart the transmit
				 * path---either push rate and RTS/CTS decisions much
				 * earlier (hard), allocate fragments just in time
				 * (harder), or support late refragmentation (even
				 * harder)---it's too difficult to fix this now.
				 */
				if (dur > wme->edcf_txop[ac])
					WL_ERROR(("wl%d: %s txop exceeded phylen %d/%d dur %d/%d\n",
						wlc->pub->unit, fifo_names[queue], phylen,
						wlc->fragthresh[ac], dur, wme->edcf_txop[ac]));
			}
		} else if ((k == 0) && SCB_WME(scb) && qos && CAC_ENAB(wlc->pub) &&
			queue <= TX_AC_VO_FIFO) {
			uint dur;
			if (rts) {
				/* 1 RTS or CTS-to-self frame */
				dur = wlc_calc_cts_time(wlc, rts_rspec, rts_preamble_type);
				/* (SIFS + CTS) + SIFS + frame + SIFS + ACK */
				dur += ltoh16(rts->durid);
			} else {
				/* frame + SIFS + ACK */
				dur = wlc_calc_frame_time(wlc, rspec, preamble_type, phylen);
				dur += wlc_compute_frame_dur(wlc, rspec, preamble_type, 0);
			}
			wlc_cac_update_dur_cache(wlc->cac, ac, PKTPRIO(p), scb, dur,
				(phylen - keyinfo_len), WLC_CAC_DUR_CACHE_PREP);
			/* update cac used time */
			if (wlc_cac_update_used_time(wlc->cac, ac, dur, scb))
				WL_ERROR(("wl%d: ac %d: txop exceeded allocated TS time\n",
					wlc->pub->unit, ac));
		}
		/* Store the final rspec back into the cur_rate */
		cur_rate.rspec[k] = rspec;
	} /* rate loop ends */

	/* Mark last rate */
	rate_blk[cur_rate.num-1].RtsCtsControl |= htol16(D11AC_RTSCTS_LAST_RATE);
	txh->PktInfo.MacTxControlLow = htol16(mcl);
	txh->PktInfo.MacTxControlHigh = htol16(mch);

	/* TxFrameID (gettxrate may have updated it) */
	txh->PktInfo.TxFrameID = htol16(frameid);

	/* record rate history here (pick final primary rspec) */
	rspec = cur_rate.rspec[0];
	WLCNTSCB_COND_SET(((type == FC_TYPE_DATA) &&
		((FC_SUBTYPE(fc) != FC_SUBTYPE_NULL) &&
			(FC_SUBTYPE(fc) != FC_SUBTYPE_QOS_NULL))),
			scb->scb_stats.tx_rate, rspec);

	/* record rate history after the txbw is valid */
	if (rspec_history) {
		/* update per bsscfg tx rate */
		bsscfg->txrspec[bsscfg->txrspecidx][0] = rspec;
		bsscfg->txrspec[bsscfg->txrspecidx][1] = (uint8) nfrags;
		bsscfg->txrspecidx = (bsscfg->txrspecidx+1) % NTXRATE;

		WLCNTSCBSET(scb->scb_stats.tx_rate_fallback, cur_rate.rspec[cur_rate.num - 1]);
	}

#ifdef WLFCTS
	if (WLFCTS_ENAB(wlc->pub))
		WLPKTTAG(p)->rspec = rspec;
#endif // endif

#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
		uint16 bss = (uint16)wlc_mcnx_BSS_idx(wlc->mcnx, bsscfg);
		ASSERT(bss < M_P2P_BSS_MAX);
		if (D11REV_GE(corerev, 64)) {
			txh->u.rev64.CacheInfo.BssIdEncAlg |= (bss << D11AC_BSSID_SHIFT);
		} else {
			txh->u.rev40.CacheInfo.BssIdEncAlg |= (bss << D11AC_BSSID_SHIFT);
		}
	}
#endif // endif

#ifdef WLAMPDU_MAC
	if (WLPKTFLAG_AMPDU(pkttag) && D11REV_GE(wlc->pub->corerev, 40)) {
#ifdef WL_MUPKTENG
		if (mutx_pkteng_on)
			wlc_ampdu_mupkteng_fill_percache_info(wlc->ampdu_tx, scb,
				(uint8)PKTPRIO(p), txh);
		else
#endif // endif
			wlc_ampdu_fill_percache_info(wlc->ampdu_tx, scb, (uint8)PKTPRIO(p), txh);
	}
#endif /* WLAMPDU_MAC */

	/* With d11 hdrs on, mark the packet as MPDU with txhdr */
	WLPKTTAG(p)->flags |= (WLF_MPDU | WLF_TXHDR);

	return (frameid);
}

/*
 * Add 802.11 headers
 *
 * 'p' data must start with 802.11 MAC header
 * 'p' must allow enough bytes of local headers to be "pushed" onto the packet
 *
 * For pre-11ac PHY: 	headroom == D11_PHY_HDR_LEN + D11_TXH_LEN (D11_TXH_LEN is now 104 bytes)
 * For 11ac PHY: 	headroom == D11AC_TXH_LEN (D11AC_TXH_LEN is now 124 bytes
 *                                  which include PHY PLCP header)
 *
 */
uint16
wlc_d11hdrs(wlc_info_t *wlc, void *p, struct scb *scb, uint txparams_flags,
	uint frag, uint nfrags, uint queue, uint next_frag_len,
	const wlc_key_info_t *key_info, ratespec_t rspec_override, uint16 *txh_off)
{
	uint16 retval;
#if defined(BCMDBG) || defined(WLMSG_PRHDRS) || defined(WLMSG_PRPKT)
	uint8 *txh;
#endif // endif
#if defined(BCMDBG) || defined(WLMSG_PRHDRS)
	uint8 *h = PKTDATA(wlc->osh, p);
#endif // endif

	if (WLCISACPHY(wlc->band)) {
		retval = wlc_d11ac_hdrs(wlc, p, scb,  txparams_flags, frag,
		                        nfrags, queue, next_frag_len, key_info, rspec_override);
#ifdef WLTOEHW
		/* add tso-oe header, if capable and toe module is not bypassed */
		if (wlc->toe_capable && !wlc->toe_bypass)
			wlc_toe_add_hdr(wlc, p, scb, key_info, nfrags, txh_off);
#endif // endif
	} else {
		retval = wlc_d11n_hdrs(wlc, p, scb,  txparams_flags, frag,
		                       nfrags, queue, next_frag_len, key_info, rspec_override);
	}

#if defined(BCMDBG) || defined(WLMSG_PRHDRS) || defined(WLMSG_PRPKT)
	txh = PKTDATA(wlc->osh, p);
#endif // endif

	WL_PRHDRS(wlc, "txpkt hdr (MPDU)", h, (wlc_txd_t*)txh, NULL, PKTLEN(wlc->osh, p));
	WL_PRPKT("txpkt body (MPDU)", txh, PKTLEN(wlc->osh, p));

	return retval;

}

/* The opposite of wlc_calc_frame_time */
static uint
wlc_calc_frame_len(wlc_info_t *wlc, ratespec_t ratespec, uint8 preamble_type, uint dur)
{
	uint nsyms, mac_len, Ndps;
	uint rate = RSPEC2RATE(ratespec);

	WL_TRACE(("wl%d: %s: rspec 0x%x, preamble_type %d, dur %d\n",
	          wlc->pub->unit, __FUNCTION__, ratespec, preamble_type, dur));

	if (RSPEC_ISVHT(ratespec)) {
		mac_len = (dur * rate) / 8000;
	} else if (RSPEC_ISHT(ratespec)) {
		mac_len = wlc_ht_calc_frame_len(wlc->hti, ratespec, preamble_type, dur);
	} else if (RSPEC_ISOFDM(ratespec)) {
		dur -= APHY_PREAMBLE_TIME;
		dur -= APHY_SIGNAL_TIME;
		if (WLCISGPHY(wlc->band))
			dur -= DOT11_OFDM_SIGNAL_EXTENSION;
		/* Ndbps = Mbps * 4 = rate(500Kbps) * 2 */
		Ndps = rate*2;
		nsyms = dur / APHY_SYMBOL_TIME;
		mac_len = ((nsyms * Ndps) - (APHY_SERVICE_NBITS + APHY_TAIL_NBITS)) / 8;
	} else {
		if (preamble_type & WLC_SHORT_PREAMBLE)
			dur -= BPHY_PLCP_SHORT_TIME;
		else
			dur -= BPHY_PLCP_TIME;
		mac_len = dur * rate;
		/* divide out factor of 2 in rate (1/2 mbps) */
		mac_len = mac_len / 8 / 2;
	}
	return mac_len;
}

#ifndef NEW_TXQ
void BCMFASTPATH
wlc_txfifo(wlc_info_t *wlc, uint fifo, void *p, wlc_txh_info_t* txh_info,
	bool commit, int8 txpktpend)
{
	uint16 frameid = INVALIDFID;
	struct scb *scb;
	wlc_bsscfg_t *cfg;

	WL_TRACE(("wlc%d: %s(fifo=%d commit=%d txpktpend=%d fid=0x%x)\n",
	          wlc->pub->unit, __FUNCTION__, fifo, commit, txpktpend,
	          ltoh16(txh_info->TxFrameID)));

	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);
	BCM_REFERENCE(cfg);

	/* We are done with WLF_FIFOPKT regardless */
	WLPKTTAG(p)->flags &= ~WLF_FIFOPKT;

	/* We are done with WLF_TXCMISS regardless */
	WLPKTTAG(p)->flags &= ~WLF_TXCMISS;

	/* Apply misc fixups for state and powersave */
	txq_hw_hdr_fixup(wlc, p, txh_info, fifo);

	/* When a BC/MC frame is being committed to the BCMC fifo via DMA (NOT PIO), update
	 * ucode or BSS info as appropriate.
	 */
	if (fifo == TX_BCMC_FIFO) {
		frameid = ltoh16(txh_info->TxFrameID);

#if defined(MBSS)
		/* For MBSS mode, keep track of the last bcmc FID in the bsscfg info.
		 * A snapshot of the FIDs for each BSS will be committed to shared memory
		 * at DTIM.
		 */
		if (MBSS_ENAB(wlc->pub)) {
			wlc_bsscfg_t *bsscfg;
			bsscfg = wlc->bsscfg[WLPKTTAGBSSCFGGET(p)];
			ASSERT(bsscfg != NULL);
			bsscfg->bcmc_fid = frameid;
#if defined(DONGLEBUILD)
			if (!BCMDHDHDR_ENAB())
				PKTSETFRAMEID(p, frameid);
#endif // endif
			wlc_mbss_txq_update_bcmc_counters(wlc, bsscfg, p);

			/* invalidate frameid so wlc_bmac_txfifo() does not commit to
			 * the non-MBSS BCMCFID location
			 */
			frameid = INVALIDFID;
		}
#endif /* MBSS */
	}

	if (WLC_WAR16165(wlc))
		wlc_war16165(wlc, TRUE);

#ifdef WLC_HIGH_ONLY
	if (RPCTX_ENAB(wlc->pub)) {
		WL_PRHDRS(wlc, __FUNCTION__, txh_info->d11HdrPtr,
		          txh_info->hdrPtr, NULL,
		          PKTLEN(wlc->osh, p) -
		          (int)((int8*)txh_info->d11HdrPtr - (int8*)PKTDATA(wlc->osh, p)));

		/* Bump up pending count for committed packets */
		if (commit) {
			TXPKTPENDINC(wlc, fifo, txpktpend);
			WL_TRACE(("%s: pktpend inc %d to %d\n", __FUNCTION__,
			          txpktpend, TXPKTPENDGET(wlc, fifo)));
		}

		if (wlc_rpctx_tx(wlc->rpctx, fifo, p, commit, frameid, txpktpend)) {
			PKTFREE(wlc->osh, p, TRUE);

			if (commit) {
				TXPKTPENDDEC(wlc, fifo, txpktpend);
				WL_TRACE(("%s: rpctx fail, pktpend dec %d to %d\n", __FUNCTION__,
				          txpktpend, TXPKTPENDGET(wlc, fifo)));
			}
		}

		return;
	}
#else /* !WLC_HIGHT_ONLY */
#ifdef BCMDBG_POOL
	if (WLPKTTAG(p)->flags & WLF_PHDR) {
		void *pdata;

		pdata = PKTNEXT(wlc->pub.osh, p);
		ASSERT(pdata);
		ASSERT(WLPKTTAG(pdata)->flags & WLF_DATA);

		PKTPOOLSETSTATE(pdata, POOL_TXD11);
	}
	else {
		PKTPOOLSETSTATE(p, POOL_TXD11);
	}
#endif // endif

#ifdef DWDS_HW_TID_WAR
	/* XXX: Customer specific WAR to address a interopabilty issue with
	 * older products. Code is enabled only in customer base.
	 */
	if (SCB_DWDS(scb) && (scb->key != NULL) && (scb->key->algo == CRYPTO_ALGO_AES_CCM)) {
		wlc_dwds_hw_tid_war(wlc, cfg, scb->key,
			(struct dot11_header *)(txh_info->d11HdrPtr), PKTPRIO(p));
	}
#endif /* DWDS_HW_TID_WAR */

	/* Bump up pending count for committed packets */
	if (commit) {
		TXPKTPENDINC(wlc, fifo, txpktpend);
		WL_TRACE(("%s: pktpend inc %d to %d\n", __FUNCTION__,
		          txpktpend, TXPKTPENDGET(wlc, fifo)));
	}
	/* Commit BCMC sequence number in the SHM frame ID location */
	if (frameid != INVALIDFID)
		BCMCFID(wlc, frameid);

#ifdef WLC_LOW
	/*
	* PR 113378: Checking for the PM wake override bit before calling the override
	* PR 107865: DRQ output should be latched before being written to DDQ.
	*/
	if (((D11REV_IS(wlc->pub->corerev, 41)) || (D11REV_IS(wlc->pub->corerev, 44))) &&
		(!(wlc->hw->maccontrol & MCTL_WAKE) &&
		!(wlc->hw->wake_override & WLC_WAKE_OVERRIDE_4335_PMWAR)))
		wlc_ucode_wake_override_set(wlc->hw, WLC_WAKE_OVERRIDE_4335_PMWAR);
#endif /* WLC_LOW */

	if (!PIO_ENAB(wlc->pub)) {
		if (wlc_bmac_dma_txfast(wlc, fifo, p, commit) < 0) {
			PKTFREE(wlc->osh, p, TRUE);
			WL_ERROR(("%s: fatal, toss frames !!!\n", __FUNCTION__));
			if (commit) {
				TXPKTPENDDEC(wlc, fifo, txpktpend);
				WL_TRACE(("%s: dma fail, pktpend dec %d to %d\n", __FUNCTION__,
				          txpktpend, TXPKTPENDGET(wlc, fifo)));
			}
		} else {
			PKTDBG_TRACE(wlc->osh, p, PKTLIST_DMAQ);
		}
	} else
		wlc_pio_tx(WLC_HW_PIO(wlc, fifo), p);

	/* XXX: WAR for in case pkt does not wake up chip, read maccontrol reg
	 * tracked by PR94097
	 */

	if ((BCM4331_CHIP_ID == CHIPID(wlc->pub->sih->chip) ||
		BCM4350_CHIP(wlc->pub->sih->chip) ||
		BCM4352_CHIP_ID == CHIPID(wlc->pub->sih->chip) ||
		BCM43602_CHIP_ID == CHIPID(wlc->pub->sih->chip) ||
		BCM4360_CHIP_ID == CHIPID(wlc->pub->sih->chip)) &&
		(cfg->pm->PM)) {
		(void)R_REG(wlc->osh, &wlc->regs->maccontrol);
	}
#endif /* WLC_HIGH_ONLY */
}
#endif /* !NEW_TXQ */
/**
 * Fetch the TxD (DMA hardware hdr), vhtHdr (rev 40+ ucode hdr),
 * and 'd11_hdr' (802.11 frame header), from the given Tx prepared pkt.
 * This function only works for d11 core rev >= 40 since it is extracting
 * rev 40+ specific header information.
 *
 * A Tx Prepared packet is one that has been prepared for the hw fifo and starts with the
 * hw/ucode header.
 *
 * @param wlc           wlc_info_t pointer
 * @param p             pointer to pkt from wich to extract interesting header pointers
 * @param ptxd          on return will be set to a pointer to the TxD hardware header,
 *                      also know as @ref d11ac_tso_t
 * @param ptxh          on return will be set to a pointer to the ucode TxH header @ref d11actxh_t
 * @param pd11hdr       on return will be set to a pointer to the 802.11 header bytes,
 *                      starting with Frame Control
 */
void BCMFASTPATH
wlc_txprep_pkt_get_hdrs(wlc_info_t* wlc, void* p,
                        uint8** ptxd, d11actxh_t** ptxh, struct dot11_header** pd11_hdr)
{
	uint8* txd;
	d11actxh_t* txh;
	uint txd_len;
	uint txh_len;

	/* this fn is only for new VHT (11ac) capable ucode headers */
	ASSERT(D11REV_GE(wlc->pub->corerev, 40));

	txd = PKTDATA(wlc->osh, p);
	txd_len = (txd[0] & TOE_F0_HDRSIZ_NORMAL) ?
	        TSO_HEADER_LENGTH : TSO_HEADER_PASSTHROUGH_LENGTH;

	txh = (d11actxh_t*)(txd + txd_len);

#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
	if (vhtHdr->PktInfo.MacTxControlLow & htol16(D11AC_TXC_HDR_FMT_SHORT)) {
		txh_len = D11AC_TXH_SHORT_LEN;
	} else
#endif /* defined(WLC_UCODE_CACHE) && D11CONF_GE(42) */
	{
		txh_len = D11AC_TXH_LEN;
	}

	*ptxd = txd;
	*ptxh = txh;
	*pd11_hdr = (struct dot11_header*)((uint8*)txh + txh_len);
}

/*
 * have to extract SGI/STBC differently pending on frame type being 11n or 11vht
 */
bool BCMFASTPATH
wlc_txh_get_isSGI(const wlc_txh_info_t* txh_info)
{
	uint16 frametype;

	ASSERT(txh_info);

	frametype = ltoh16(txh_info->PhyTxControlWord0) & PHY_TXC_FT_MASK;
	if (frametype == FT_HT)
		return (PLCP3_ISSGI(txh_info->plcpPtr[3]));
	else if (frametype == FT_VHT)
		return (VHT_PLCP3_ISSGI(txh_info->plcpPtr[3]));

	return FALSE;
}

bool BCMFASTPATH
wlc_txh_get_isSTBC(const wlc_txh_info_t* txh_info)
{
	uint8 frametype;

	ASSERT(txh_info);
	ASSERT(txh_info->plcpPtr);

	frametype = ltoh16(txh_info->PhyTxControlWord0) & PHY_TXC_FT_MASK;
	if (frametype == FT_HT)
		return (PLCP3_ISSTBC(txh_info->plcpPtr[3]));
	else if (frametype == FT_VHT)
		return ((txh_info->plcpPtr[0] & VHT_SIGA1_STBC) != 0);

	return FALSE;
}

chanspec_t
wlc_txh_get_chanspec(wlc_info_t* wlc, wlc_txh_info_t* tx_info)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = (d11txh_t*)(tx_info->hdrPtr);
		return (ltoh16(nonVHTHdr->XtraFrameTypes) >> XFTS_CHANNEL_SHIFT);
	} else {
		d11actxh_t* vhtHdr = (d11actxh_t*)(tx_info->hdrPtr);
		return ltoh16(vhtHdr->PktInfo.Chanspec);
	}
}

#ifdef WL_CS_PKTRETRY
void
wlc_txh_set_chanspec(wlc_info_t* wlc, wlc_txh_info_t* tx_info, chanspec_t new_chanspec)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = (d11txh_t*)(tx_info->hdrPtr);
		uint16 xtra_frame_types = ltoh16(nonVHTHdr->XtraFrameTypes);

		xtra_frame_types &= ~(CHSPEC_CHANNEL(0xFFFF) << XFTS_CHANNEL_SHIFT);
		xtra_frame_types |= CHSPEC_CHANNEL(new_chanspec) << XFTS_CHANNEL_SHIFT;

		nonVHTHdr->XtraFrameTypes = htol16(xtra_frame_types);
	} else {
		d11actxh_t* vhtHdr = (d11actxh_t*)(tx_info->hdrPtr);
		vhtHdr->PktInfo.Chanspec = htol16(new_chanspec);
	}
}
#endif /* WL_CS_PKTRETRY */

uint16 BCMFASTPATH
wlc_get_txh_frameid(wlc_info_t* wlc, void* p)
{
	uint8 *pkt_data = NULL;

	ASSERT(p);
	pkt_data = PKTDATA(wlc->osh, p);
	ASSERT(pkt_data != NULL);
#ifdef HOST_HDR_FETCH
	ASSERT(__PKTISHDRINHOST(wlc->osh, p) == 0);
#endif // endif
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = (d11txh_t *)pkt_data;

		return ltoh16(nonVHTHdr->TxFrameID);
	} else {
		uint tsoHdrSize = 0;
		d11actxh_t* vhtHdr;
		d11actxh_pkt_t *ppkt_info;

#ifdef WLTOEHW
		tsoHdrSize = (wlc->toe_bypass ?
			0 : wlc_tso_hdr_length((d11ac_tso_t*)pkt_data));
#endif /* WLTOEHW */

		ASSERT(PKTLEN(wlc->osh, p) >= D11AC_TXH_SHORT_LEN + tsoHdrSize);

		vhtHdr = (d11actxh_t*)(pkt_data + tsoHdrSize);

		ppkt_info = &(vhtHdr->PktInfo);

		return ltoh16(ppkt_info->TxFrameID);
	}
}

/* parse info from a tx header and return in a common format --
* needed due to differences between ac and non-ac tx hdrs
*/
void BCMFASTPATH
wlc_get_txh_info(wlc_info_t* wlc, void* p, wlc_txh_info_t* tx_info)
{
	uint8 *pkt_data = NULL;
#ifdef HOST_HDR_FETCH
	ASSERT(PKTLEN(wlc->osh, p));
	ASSERT(__PKTISHDRINHOST(wlc->osh, p) == 0);
#endif // endif

	if (p == NULL || tx_info == NULL ||
		((pkt_data = PKTDATA(wlc->osh, p)) == NULL)) {
		WL_ERROR(("%s: null P or tx_info or pktdata.\n", __FUNCTION__));
		ASSERT(p != NULL);
		ASSERT(tx_info != NULL);
		ASSERT(pkt_data != NULL);
		return;
	}

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = (d11txh_t *)pkt_data;

		tx_info->tsoHdrPtr = NULL;
		tx_info->tsoHdrSize = 0;

		tx_info->TxFrameID = nonVHTHdr->TxFrameID;
		tx_info->Tstamp = (((uint32)ltoh16(nonVHTHdr->TstampHigh) << 16) |
		                   ltoh16(nonVHTHdr->TstampLow));
		tx_info->MacTxControlLow = nonVHTHdr->MacTxControlLow;
		tx_info->MacTxControlHigh = nonVHTHdr->MacTxControlHigh;
		tx_info->hdrSize = sizeof(d11txh_t);
		tx_info->hdrPtr = (wlc_txd_t*)nonVHTHdr;
		tx_info->d11HdrPtr = (void*)((uint8*)(nonVHTHdr + 1) + D11_PHY_HDR_LEN);
		tx_info->TxFrameRA = nonVHTHdr->TxFrameRA;
		tx_info->plcpPtr = (uint8 *)(nonVHTHdr + 1);
		tx_info->PhyTxControlWord0 = nonVHTHdr->PhyTxControlWord;
		tx_info->PhyTxControlWord1 = nonVHTHdr->PhyTxControlWord_1;
		tx_info->seq = ltoh16(((struct dot11_header*)tx_info->d11HdrPtr)->seq) >>
			SEQNUM_SHIFT;

		tx_info->d11FrameSize = (uint16)(pkttotlen(wlc->osh, p) -
			(sizeof(d11txh_t) + D11_PHY_HDR_LEN));

		/* unused */
		tx_info->PhyTxControlWord2 = 0;
		tx_info->w.ABI_MimoAntSel = (uint16)(nonVHTHdr->ABI_MimoAntSel);
	} else {
		uint8* pktHdr;
		uint tsoHdrSize = 0;
		d11actxh_t* vhtHdr;
		d11actxh_rate_t *rateInfo;
		d11actxh_pkt_t *ppkt_info;

		pktHdr = (uint8*)pkt_data;

#ifdef WLTOEHW
		tsoHdrSize = (wlc->toe_bypass ?
			0 : wlc_tso_hdr_length((d11ac_tso_t*)pktHdr));
		tx_info->tsoHdrPtr = (void*)((tsoHdrSize != 0) ? pktHdr : NULL);
		tx_info->tsoHdrSize = tsoHdrSize;
#else
		tx_info->tsoHdrPtr = NULL;
		tx_info->tsoHdrSize = 0;
#endif /* WLTOEHW */

		ASSERT(PKTLEN(wlc->osh, p) >= D11AC_TXH_SHORT_LEN + tsoHdrSize);

		vhtHdr = (d11actxh_t*)(pktHdr + tsoHdrSize);

		ppkt_info = &(vhtHdr->PktInfo);

		tx_info->Tstamp = ((uint32)ltoh16(ppkt_info->Tstamp)) << D11AC_TSTAMP_SHIFT;
		tx_info->TxFrameID = ppkt_info->TxFrameID;
		tx_info->MacTxControlLow = ppkt_info->MacTxControlLow;
		tx_info->MacTxControlHigh = ppkt_info->MacTxControlHigh;

#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
		if (tx_info->MacTxControlLow & htol16(D11AC_TXC_HDR_FMT_SHORT)) {
			int cache_idx = (ltoh16(tx_info->MacTxControlLow) &
				D11AC_TXC_CACHE_IDX_MASK) >> D11AC_TXC_CACHE_IDX_SHIFT;
			rateInfo = (d11actxh_rate_t *)wlc_txc_get_rate_info_shdr(wlc->txc,
				cache_idx);
			tx_info->hdrSize = D11AC_TXH_SHORT_LEN;
		} else
#endif /* defined(WLC_UCODE_CACHE) && D11CONF_GE(42) */
		{
			rateInfo = WLC_TXD_RATE_INFO_GET(vhtHdr, wlc->pub->corerev);
			tx_info->hdrSize = D11AC_TXH_LEN;
		}

		tx_info->hdrPtr = (wlc_txd_t*)vhtHdr;
		tx_info->d11HdrPtr = (void*)((uint8*)vhtHdr + tx_info->hdrSize);
		tx_info->d11FrameSize =
		        pkttotlen(wlc->osh, p) - (tsoHdrSize + tx_info->hdrSize);

		/* a1 holds RA when RA is used; otherwise this field can/should be ignored */
		tx_info->TxFrameRA = (uint8*)(((struct dot11_header *)
								(tx_info->d11HdrPtr))->a1.octet);

		tx_info->plcpPtr = (uint8*)(rateInfo[0].plcp);
		/* usu this will work, as long as primary rate is the only one concerned with */
		tx_info->PhyTxControlWord0 = rateInfo[0].PhyTxControlWord_0;
		tx_info->PhyTxControlWord1 = rateInfo[0].PhyTxControlWord_1;
		tx_info->PhyTxControlWord2 = rateInfo[0].PhyTxControlWord_2;
		tx_info->w.FbwInfo = ltoh16(rateInfo[0].FbwInfo);

		tx_info->seq = ltoh16(ppkt_info->Seq) >> SEQNUM_SHIFT;
	}
}

/* set info in a tx header from a common format --
* needed due to differences between ac and non-ac tx hdrs --
* ptr fields are NOT settable and this function will ASSERT if such op is tried
*/
void
wlc_set_txh_info(wlc_info_t* wlc, void* p, wlc_txh_info_t* tx_info)
{
	ASSERT(p);
	ASSERT(tx_info);

	if (p == NULL) return;

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* nonVHTHdr = &tx_info->hdrPtr->d11txh;

		nonVHTHdr->TxFrameID = tx_info->TxFrameID;
		nonVHTHdr->TstampLow = htol16((uint16)tx_info->Tstamp);
		nonVHTHdr->TstampHigh = htol16((uint16)(tx_info->Tstamp >> 16));
		nonVHTHdr->PhyTxControlWord = tx_info->PhyTxControlWord0;
		nonVHTHdr->MacTxControlLow = tx_info->MacTxControlLow;
		nonVHTHdr->MacTxControlHigh = tx_info->MacTxControlHigh;

		nonVHTHdr->ABI_MimoAntSel = tx_info->w.ABI_MimoAntSel;

		/* these values should not be modified */
		ASSERT(tx_info->hdrSize == sizeof(d11txh_t));
		ASSERT(tx_info->d11HdrPtr == (void*)((uint8*)(nonVHTHdr + 1) + D11_PHY_HDR_LEN));
		ASSERT(tx_info->TxFrameRA == nonVHTHdr->TxFrameRA);
	} else {
		d11actxh_t* vhtHdr = &tx_info->hdrPtr->txd;
		d11actxh_rate_t* rate_info;

		vhtHdr->PktInfo.TxFrameID = tx_info->TxFrameID;
		/* vht tx hdr has only one time stamp field */
		vhtHdr->PktInfo.Tstamp = htol16((uint16)(tx_info->Tstamp >> D11AC_TSTAMP_SHIFT));
		vhtHdr->PktInfo.MacTxControlLow = tx_info->MacTxControlLow;
		vhtHdr->PktInfo.MacTxControlHigh = tx_info->MacTxControlHigh;

		rate_info = WLC_TXD_RATE_INFO_GET(vhtHdr, wlc->pub->corerev);

		/* XXX 4360: need to take into acct multiple rates perhaps
		 * via pass rspec into this func
		 */
		rate_info[0].PhyTxControlWord_0 = tx_info->PhyTxControlWord0;
		rate_info[0].PhyTxControlWord_1 = tx_info->PhyTxControlWord1;
		rate_info[0].PhyTxControlWord_2 = tx_info->PhyTxControlWord2;

		/* a1 holds RA when RA is used; otherwise this field should be ignored */
		ASSERT(tx_info->TxFrameRA == (uint8*)
				((struct dot11_header *)tx_info->d11HdrPtr)->a1.octet);
		ASSERT(tx_info->plcpPtr == (uint8*)(rate_info[0].plcp));
	}
}

#ifdef WL11N
/*
 *	For HT framem, return mcs index
 *	For VHT frame:
 *		bit 0-3 mcs index
 *		bit 6-4 nsts for VHT
 * 		bit 7:	 1 for VHT
 */
uint8
wlc_txh_info_get_mcs(wlc_info_t* wlc, wlc_txh_info_t* txh_info)
{
	uint8 mcs, frametype;
	uint8 *plcp;
	ASSERT(txh_info);

	frametype = ltoh16(txh_info->PhyTxControlWord0) & PHY_TXC_FT_MASK;
	ASSERT((frametype == FT_HT) || (frametype == FT_VHT));

	plcp = (uint8 *)(txh_info->plcpPtr);
	if (frametype == FT_HT) {
		mcs = (plcp[0] & MIMO_PLCP_MCS_MASK);
	} else {
		mcs = wlc_vht_get_rate_from_plcp(plcp);
	}
	return mcs;
}
#endif /* WL11N */

bool BCMFASTPATH
wlc_txh_info_is5GHz(wlc_info_t* wlc, wlc_txh_info_t* txh_info)
{
	uint16 mcl;
	bool is5GHz = FALSE;

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		ASSERT(txh_info);
		mcl = ltoh16(txh_info->MacTxControlLow);
		is5GHz = ((mcl & TXC_FREQBAND_5G) == TXC_FREQBAND_5G);
	} else {
		d11actxh_t* vhtHdr = &(txh_info->hdrPtr->txd);
		uint16 cs = (uint16)ltoh16(vhtHdr->PktInfo.Chanspec);
		is5GHz = CHSPEC_IS5G(cs);
	}
	return is5GHz;
}

bool
wlc_txh_has_rts(wlc_info_t* wlc, wlc_txh_info_t* txh_info)
{
	uint16 flag;

	ASSERT(txh_info);
	if (D11REV_LT(wlc->pub->corerev, 40))
		flag = htol16(txh_info->MacTxControlLow) & TXC_SENDRTS;
	else if (D11REV_LT(wlc->pub->corerev, 64))
		flag = htol16(txh_info->hdrPtr->txd.u.rev40.RateInfo[0].RtsCtsControl)
		        & D11AC_RTSCTS_USE_RTS;
	else
		flag = htol16(txh_info->hdrPtr->txd.u.rev64.RateInfo[0].RtsCtsControl)
		        & D11AC_RTSCTS_USE_RTS;

	return (flag ? TRUE : FALSE);
}

void BCMFASTPATH
wlc_txh_set_aging(wlc_info_t *wlc, void *hdr, bool enable)
{
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		d11txh_t* txh = (d11txh_t*)hdr;
		if (!enable)
			txh->MacTxControlLow &= htol16(~TXC_LIFETIME);
		else
			txh->MacTxControlLow |= htol16(TXC_LIFETIME);
	} else {
		d11actxh_t* txh_ac = (d11actxh_t*)hdr;
		if (!enable)
			txh_ac->PktInfo.MacTxControlLow &= htol16(~D11AC_TXC_AGING);
		else
			txh_ac->PktInfo.MacTxControlLow |= htol16(D11AC_TXC_AGING);
	}
}

static void BCMFASTPATH
wlc_txh_info_set_aging(wlc_info_t* wlc, wlc_txh_info_t* txh_info, bool enable)
{
	ASSERT(txh_info);
	wlc_txh_set_aging(wlc, txh_info->hdrPtr, enable);

	if (D11REV_LT(wlc->pub->corerev, 40)) {
		txh_info->MacTxControlLow = txh_info->hdrPtr->d11txh.MacTxControlLow;
	} else {
		txh_info->MacTxControlLow = txh_info->hdrPtr->txd.PktInfo.MacTxControlLow;
	}
}

static void
wlc_txq_freed_pkt_time(wlc_info_t *wlc, void *pkt, uint16 *time_adj)
{
	ASSERT(time_adj != NULL);
#if defined(TXQ_MUX)
	/* Account for the buffered time we are eliminating */
	*time_adj += WLPKTTIME(pkt);
#else
#ifdef WLAMPDU
	/* For SW AMPDU, only the last MPDU in an AMPDU counts
	 * as txpkt_weight.
	 * Otherwise, 1 for each packet.
	 *
	 * SW AMPDU is only relavant for d11 rev < 40, non-AQM.
	 */
	if (WLPKTFLAG_AMPDU(WLPKTTAG(pkt)) &&
		AMPDU_HOST_ENAB(wlc->pub)) {
		uint16 count;
		wlc_txh_info_t txh_info;
		uint16 mpdu_type;

		wlc_get_txh_info(wlc, pkt, &txh_info);
		mpdu_type = (((ltoh16(txh_info.MacTxControlLow)) &
			TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT);

		if (mpdu_type == TXC_AMPDU_NONE) {
			/* regular MPDUs just count as 1 */
			count = 1;
		} else if (mpdu_type != TXC_AMPDU_LAST) {
			/* non-last MPDUs are counted as zero */
			count = 0;
		} else {
			/* the last MPDU in an AMPDU has a count that
			 * equals the txpkt_weight
			 */
			count = wlc_ampdu_get_txpkt_weight(wlc->ampdu_tx);
		}
		*time_adj += count;
	}
	else
#endif /* WLAMPDU */
		*time_adj += 1;
#endif /* TXQ_MUX */
}

#ifdef AP
/**
 * This function is used to 'reset' bcmc administration for all applicable bsscfgs to prevent bcmc
 * fifo lockup.
 * Should only be called when there is no more bcmc traffic pending due to flush.
 */
void
wlc_tx_fifo_sync_bcmc_reset(wlc_info_t *wlc)
{
	uint i;

	if (MBSS_ENAB(wlc->pub)) {
		wlc_bsscfg_t *cfg = NULL;
		FOREACH_AP(wlc, i, cfg) {
			if (cfg->bcmc_fid_shm != INVALIDFID) {
				WL_INFORM(("wl%d.%d: %s: cfg(%p) bcmc_fid = 0x%x bcmc_fid_shm ="
					" 0x%x, resetting bcmc_fids mc_pkts %d\n",
					WLCWLUNIT(wlc), WLC_BSSCFG_IDX(cfg), __FUNCTION__,
					cfg, cfg->bcmc_fid, cfg->bcmc_fid_shm,
					TXPKTPENDGET(wlc, TX_BCMC_FIFO)));
			}
			/* Let's reset the FIDs since we have completed flush */
			wlc_mbss_bcmc_reset(wlc, cfg);
		}
	} else if (wlc->cfg != NULL) {
		struct scb *bcmc_scb = WLC_BCMCSCB_GET(wlc, wlc->cfg);
		if (bcmc_scb != NULL) {
			BCMCFID(wlc, INVALIDFID);
			if ((SCB_PS(bcmc_scb) == TRUE) &&
				(!BSSCFG_IBSS(wlc->cfg) || !AIBSS_ENAB(wlc->pub))) {
				bcmc_scb->PS = FALSE;
			}
		}
	}
}
#endif /* AP */

#ifdef WL_MULTIQUEUE
static void
wlc_attach_queue(wlc_info_t *wlc, wlc_txq_info_t *qi)
{
	wlc_txq_info_t *cur_active_queue = wlc->active_queue;

	ASSERT(wlc->active_queue != qi);
	ASSERT(MQUEUE_ENAB(wlc->pub));

	WL_MQ(("MQ: %s: qi %p\n", __FUNCTION__, qi));

	wlc->active_queue = qi;

#ifdef NEW_TXQ
	/* set a flag indicating that wlc_tx_fifo_attach_complete() needs to be called */
	wlc->txfifo_attach_pending = TRUE;

	if (cur_active_queue != NULL) {
		wlc_detach_queue(wlc, cur_active_queue);
	}

	/* If the detach is is pending at this point, wlc_tx_fifo_attach_complete() will
	 * be called when the detach completes in wlc_tx_fifo_sync_complete().
	 */

	/* If the detach is not pending (done or call was skipped just above), then see if
	 * wlc_tx_fifo_attach_complete() was called. If wlc_tx_fifo_attach_complete() was not
	 * called (wlc->txfifo_attach_pending), then call it now.
	 */
	if (!wlc->txfifo_detach_pending &&
	    wlc->txfifo_attach_pending) {
		wlc_tx_fifo_attach_complete(wlc);
	}
#else
	if (cur_active_queue != NULL) {
		wlc_detach_queue(wlc, cur_active_queue);
	}
#endif /* NEW_TXQ */
}

static void
wlc_detach_queue(wlc_info_t *wlc, wlc_txq_info_t *qi)
{
	uint fifo_bitmap;
	uint fbmp;
	uint txpktpendtot = 0;
	uint i;
	ASSERT(wlc->active_queue != NULL);
	ASSERT(MQUEUE_ENAB(wlc->pub));

	WL_MQ(("MQ: %s: qi %p\n", __FUNCTION__, qi));

	/* no hardware to sync if we are down */
	if (!wlc->pub->up) {
		WL_MQ(("MQ: %s: !up, so bailing early. TXPEND = %d\n",
		__FUNCTION__, TXPKTPENDTOT(wlc)));
		ASSERT(TXPKTPENDTOT(wlc) == 0);
		return;
	}

	fifo_bitmap = BITMAP_SYNC_ALL_EXT_TX_FIFOS;

	/* If there are pending packets on the fifo, then stop the fifo
	 * processing and re-enqueue packets
	 */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;
		txpktpendtot += TXPKTPENDGET(wlc, i);
	}
	if ((txpktpendtot > 0) && (!wlc->txfifo_detach_pending)) {

		/* Need this for split driver */
		wlc->txfifo_detach_pending = TRUE;

		/* save the fifo that is being detached as the destination to re-enqueue
		 * packets from the txfifos if no previous detach pending
		 */
		wlc->txfifo_detach_transition_queue = qi;

		/* Do not allow any new packets to flow to the fifo from the new active_queue
		 * while we are synchronizing the fifo for the detached queue.
		 * The wlc_bmac_tx_fifo_sync() process started below will trigger tx status
		 * processing that could trigger new pkt enqueues to the fifo.
		 * The 'hold' call will flow control the fifo.
		 */
		for (i = 0; i < NFIFO_EXT; i++) {
			txq_hw_hold_set(wlc->active_queue->low_txq, i);
		}

		/* flush the fifos and process txstatus from packets that
		 * were sent before the flush
		 * wlc_tx_fifo_sync_complete() will be called when all transmitted
		 * packets' txstatus have been processed. The call may be done
		 * before wlc_bmac_tx_fifo_sync() returns, or after in a split driver.
		 */
		wlc_bmac_tx_fifo_sync(wlc->hw, fifo_bitmap, SYNCFIFO);
	}
#ifdef NEW_TXQ
	else {
		if (TXPKTPENDTOT(wlc) == 0) {
			WL_MQ(("MQ: %s: skipping FIFO SYNC since TXPEND = 0\n", __FUNCTION__));
		} else {
			WL_MQ(("MQ: %s: skipping FIFO SYNC since detach already pending\n",
			__FUNCTION__));
		}

		WL_MQ(("MQ: %s: DMA pend 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d \n", __FUNCTION__,
		       WLC_HW_DI(wlc, 0) ? dma_txpending(WLC_HW_DI(wlc, 0)) : 0,
		       WLC_HW_DI(wlc, 1) ? dma_txpending(WLC_HW_DI(wlc, 1)) : 0,
		       WLC_HW_DI(wlc, 2) ? dma_txpending(WLC_HW_DI(wlc, 2)) : 0,
		       WLC_HW_DI(wlc, 3) ? dma_txpending(WLC_HW_DI(wlc, 3)) : 0,
		       WLC_HW_DI(wlc, 4) ? dma_txpending(WLC_HW_DI(wlc, 4)) : 0,
		       WLC_HW_DI(wlc, 5) ? dma_txpending(WLC_HW_DI(wlc, 5)) : 0));
	}
#endif /* NEW_TXQ */
}

/* Start or continue an excursion from the currently active
 * tx queue context.
 * Switch to the dedicated excursion queue
 */
void
wlc_excursion_start(wlc_info_t *wlc)
{
	if (!MQUEUE_ENAB(wlc->pub)) {
		WL_MQ(("MQ: %s: feature not enabled, exiting\n", __FUNCTION__));
		return;
	}

	if (wlc->excursion_active) {
		WL_MQ(("MQ: %s: already active, exiting\n", __FUNCTION__));
		return;
	}
#ifdef WLMCHAN
	if (MCHAN_ENAB(wlc->pub) && wlc->excursion_queue == wlc->active_queue) {
		WL_MCHAN(("MCHAN: %s: same queue\n", __FUNCTION__));
		return;
	}
#endif // endif
	WL_MQ(("MQ: %s:\n", __FUNCTION__));

	wlc->excursion_active = TRUE;

	/* if we are not in an excursion, the active_queue should be the primary_queue */
	ASSERT(wlc->primary_queue == wlc->active_queue);

	wlc_attach_queue(wlc, wlc->excursion_queue);
}

/* Terminate the excursion from the active tx queue context.
 * Switch from the excursion queue back to the current primary_queue
 */
void
wlc_excursion_end(wlc_info_t *wlc)
{
	wlc_txq_info_t *excursion_queue;

	if (!MQUEUE_ENAB(wlc->pub)) {
		WL_MQ(("MQ: %s: feature not enabled, exiting\n", __FUNCTION__));
		return;
	}

	if (!wlc->excursion_active) {
		WL_MQ(("MQ: %s: not in active excursion, exiting\n", __FUNCTION__));
		return;
	}

	WL_MQ(("MQ: %s:\n", __FUNCTION__));

	wlc->excursion_active = FALSE;

	excursion_queue = wlc->active_queue;
	wlc_attach_queue(wlc, wlc->primary_queue);
	if (excursion_queue != wlc->excursion_queue)
		return;

	/* Packets being sent during an excursion should only be valid
	 * for the duration of that excursion.  If any packets are still
	 * in the queue at the end of excursion should be flushed.
	 * Flush both the high and low txq.
	 */
	pktq_flush(wlc->osh, WLC_GET_TXQ(excursion_queue), TRUE, NULL, 0);
	wlc_low_txq_flush(wlc->txqi, excursion_queue->low_txq);
}

/* Switch from the excursion queue to the given queue during excursion. */
/* FIXME: This is a hack to allow a different txq to be used
 * during excursion. Need to figure out a way to switch the
 * excursion queue off and the given queue on without
 * terminating the excursion.
 */
void
wlc_active_queue_set(wlc_info_t *wlc, wlc_txq_info_t *new_active_queue)
{
	wlc_txq_info_t *old_active_queue = wlc->active_queue;

	if (!MQUEUE_ENAB(wlc->pub)) {
		WL_MQ(("MQ: %s: feature not enabled, exiting\n", __FUNCTION__));
		return;
	}

	WL_MQ(("MQ: %s: qi %p\n", __FUNCTION__, new_active_queue));

	ASSERT(new_active_queue != NULL);
	if (new_active_queue == old_active_queue)
		return;

	wlc_attach_queue(wlc, new_active_queue);
	if (old_active_queue != wlc->excursion_queue)
		return;

	/* Packets being sent during an excursion should only be valid
	 * for the duration of that excursion.  If any packets are still
	 * in the queue at the end of excursion should be flushed.
	 */
	pktq_flush(wlc->osh, WLC_GET_TXQ(old_active_queue), TRUE, NULL, 0);
}

/* Use the given queue as the new primary queue
 * wlc->primary_queue is updated, detaching the former
 * primary_queue from the fifos if necessary
 */
void
wlc_primary_queue_set(wlc_info_t *wlc, wlc_txq_info_t *new_primary)
{
	if (!MQUEUE_ENAB(wlc->pub)) {
		WL_MQ(("MQ: %s: feature not enabled, exiting\n", __FUNCTION__));
		return;
	}

	WL_MQ(("MQ: %s: qi %p\n", __FUNCTION__, new_primary));

	wlc->primary_queue = new_primary;

	/* if an excursion is active the active_queue should remain on the
	 * excursion_queue. At the end of the excursion, the new primary_queue
	 * will be made active.
	 */
	if (wlc->excursion_active)
		return;

	wlc_attach_queue(wlc, new_primary);
}

#ifdef WL_MU_TX
int
wlc_tx_fifo_hold_set(wlc_info_t *wlc, uint fifo_bitmap)
{
	uint i, fbmp;
	uint flush_fbmp = 0;

	if ((wlc->txfifo_detach_pending) || (wlc->excursion_active)) {
		return BCME_NOTREADY;
	}

	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;
		if (TXPKTPENDGET(wlc, i) > 0) {
			/* flush only fifos with pending pkts */
			flush_fbmp |= (1 << i);
		}

		/* Do not allow any new packets to flow to the fifo from the active_queue
		 * while we are synchronizing the fifo for the active_queue.
		 * The wlc_bmac_tx_fifo_sync() process started below will trigger tx status
		 * processing that could trigger new pkt enqueues to the fifo.
		 * The 'hold' call will flow control the fifo.
		 */
		txq_hw_hold_set(wlc->active_queue->low_txq, i);
	}

	/* flush the fifos and process txstatus from packets that
	 * were sent before the flush
	 * wlc_tx_fifo_sync_complete() will be called when all transmitted
	 * packets' txstatus have been processed. The call may be done
	 * before wlc_bmac_tx_fifo_sync() returns, or after in a split driver.
	 */
	if (flush_fbmp) {
		wlc->txfifo_detach_pending = TRUE;
		wlc->txfifo_detach_transition_queue = wlc->active_queue;
		wlc_bmac_tx_fifo_sync(wlc->hw, flush_fbmp, FLUSHFIFO);
	}

	return BCME_OK;
}

void
wlc_tx_fifo_hold_clr(wlc_info_t *wlc, uint fifo_bitmap)
{
	uint i, fbmp;

	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0)
			continue;
		/* Clear the hw fifo hold since we are done with the MU Client scheduler. */
		txq_hw_hold_clr(wlc->active_queue->low_txq, i);
	}
}
#endif /* WL_MU_TX */

static void
wlc_txq_free_pkt(wlc_info_t *wlc, void *pkt, uint16 *time_adj)
{
#ifdef HOST_HDR_FETCH
	/* reset HDR in HOST flags now */
	PKTRESETHDRINHOST(wlc->osh, pkt);
#endif // endif

	wlc_txq_freed_pkt_time(wlc, pkt, time_adj);
	PKTFREE(wlc->osh, pkt, TRUE);
}

static struct scb*
wlc_recover_pkt_scb(wlc_info_t *wlc, void *pkt, wlc_txh_info_t *txh_info)
{
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;

#ifdef HOST_HDR_FETCH
	ASSERT(__PKTISHDRINHOST(wlc->osh, pkt) == 0);
#endif // endif

	scb = WLPKTTAG(pkt)->_scb;

	/* check to see if the SCB is just one of the permanent hwrs_scbs */
	if (scb == wlc->band->hwrs_scb ||
	    (NBANDS(wlc) > 1 && scb == wlc->bandstate[OTHERBANDUNIT(wlc)]->hwrs_scb)) {
		WL_MQ(("MQ: %s: recovering %p hwrs_scb\n",
		       __FUNCTION__, pkt));
		return scb;
	}

	/*  use the bsscfg index in the packet tag to find out which
	 *  bsscfg this packet belongs to
	 */
	bsscfg = wlc->bsscfg[WLPKTTAGBSSCFGGET(pkt)];

#ifndef WLAWDL
	/* unicast pkts have been seen with null bsscfg here */
	ASSERT(bsscfg || !ETHER_ISMULTI(txh_info->TxFrameRA));
#endif // endif
	if (bsscfg == NULL) {
		WL_MQ(("MQ: %s: pkt cfg idx %d no longer exists\n",
			__FUNCTION__, WLPKTTAGBSSCFGGET(pkt)));
		scb = NULL;
	} else if (ETHER_ISMULTI(txh_info->TxFrameRA)) {
		scb = WLC_BCMCSCB_GET(wlc, bsscfg);
		ASSERT(scb != NULL);
		WL_MQ(("MQ: %s: recovering %p bcmc scb\n",
			__FUNCTION__, pkt));
	} else {
		uint bandindex = (wlc_txh_info_is5GHz(wlc, txh_info))?
		                 BAND_5G_INDEX:BAND_2G_INDEX;
		scb = wlc_scbfindband(wlc, bsscfg, (struct ether_addr*)txh_info->TxFrameRA,
		                      bandindex);

#if defined(BCMDBG)
		if (scb != NULL) {
			char eabuf[ETHER_ADDR_STR_LEN];
			bcm_ether_ntoa(&scb->ea, eabuf);
			WL_MQ(("MQ: %s: recovering %p scb %p:%s\n", __FUNCTION__,
			       pkt, scb, eabuf));
		} else {
			WL_MQ(("MQ: %s: failed recovery scb for pkt %p\n", __FUNCTION__, pkt));
		}
#endif // endif
	}

	WLPKTTAGSCBSET(pkt, scb);

	return scb;
}

#ifdef NEW_TXQ
#ifdef PROP_TXSTATUS
/* Additional processing for full dongle proptxstatus
 * Packets  are freed unless they are retried
 */
static void *
wlc_proptxstatus_process_pkt(wlc_info_t *wlc, void *pkt, uint16 *buffered)
{
	if (PROP_TXSTATUS_ENAB(wlc->pub) &&
		(HOST_HDR_FETCH_ENAB() || (wlc->excursion_active == FALSE))) {
		struct scb *scb_pkt = WLPKTTAGSCBGET(pkt);

		if (!(scb_pkt && SCB_ISMULTI(scb_pkt))) {
			wlc_suppress_sync_fsm(wlc, scb_pkt, pkt, TRUE);
			wlc_process_wlhdr_txstatus(wlc,
				WLFC_CTL_PKTFLAG_WLSUPPRESS, pkt, FALSE);
		}
		if (wlc_should_retry_suppressed_pkt(wlc, pkt, TX_STATUS_SUPR_PMQ) ||
			(scb_pkt && SCB_ISMULTI(scb_pkt))) {
			/* These are dongle generated pkts
			 * don't free and attempt requeue to interface queue
			 */
			return pkt;
		} else {
			wlc_txq_free_pkt(wlc, pkt, buffered);
			return NULL;
		}
	} else {
		return pkt;
	}
}
#endif /* PROP_TXSTATUS */

#ifdef BCMDBG
/** WL_MQ() debugging output helper function */
static void
wlc_txpend_counts(wlc_info_t *wlc, const char* fn)
{
	int i;
#ifdef WLC_LOW
	struct {
		uint txp;
		uint pend;
	} counts[NFIFO] = {{0, 0}};

	if (!WL_MQ_ON()) {
		return;
	}

	for  (i = 0; i < NFIFO; i++) {
		hnddma_t *di = WLC_HW_DI(wlc, i);

		if (di != NULL) {
			counts[i].txp = dma_txp(di);
			counts[i].pend = TXPKTPENDGET(wlc, i);
		}
	}

	WL_MQ(("MQ: %s: TXPKTPEND/DMA "
	       "[0]:(%d,%d) [1]:(%d,%d) [2]:(%d,%d) [3]:(%d,%d) [4]:(%d,%d) [5]:(%d,%d)\n",
	       fn,
	       counts[0].pend, counts[0].txp,
	       counts[1].pend, counts[1].txp,
	       counts[2].pend, counts[2].txp,
	       counts[3].pend, counts[3].txp,
	       counts[4].pend, counts[4].txp,
	       counts[5].pend, counts[5].txp));

#else
	for  (i = 0; i < NFIFO; i++) {
		WL_MQ(("MQ: %s: TXPEND[%d] = %d\n", fn,
		       i, TXPKTPENDGET(wlc, i)));
	}
#endif /* WLC_LOW */
}
#else
#define wlc_txpend_counts(w, fn)
#endif /* BCMDBG */

static void
wlc_low_txq_account(wlc_info_t *wlc, txq_t *low_txq, uint fifo_idx,
	uint8 flag, uint16* time_adj)
{
	void *pkt;
	struct swpktq *swq;
	struct spktq pkt_list;
#if defined(WLAMPDU_MAC)
	uint8 flipEpoch = 0;
	uint8 lastEpoch = HEAD_PKT_FLUSHED;
#endif /* defined(WLAMPDU_MAC) */

	pktqinit(&pkt_list, -1);
	swq = low_txq->swq;

	while ((pkt = pktq_swdeq(swq, fifo_idx))) {
#ifdef HOST_HDR_FETCH
		if (!wlc_recover_pkt_scb_hdr_inhost(wlc, pkt)) {
#else
		wlc_txh_info_t txh_info;
		wlc_get_txh_info(wlc, pkt, &txh_info);

		if (!wlc_recover_pkt_scb(wlc, pkt, &txh_info)) {
#endif /* HOST_HDR_FETCH */
#if defined(WLAMPDU_MAC)
			flipEpoch |= TXQ_PKT_DEL;
#endif /* defined(WLAMPDU_MAC) */
			wlc_txq_free_pkt(wlc, pkt, time_adj);
			continue;
		}

		/* SWWLAN-39516: Removing WLF3_TXQ_SHORT_LIFETIME optimization.
		 * Could be addressed by pkt lifetime?
		 */
		if (WLPKTTAG(pkt)->flags3 & WLF3_TXQ_SHORT_LIFETIME) {
			WL_INFORM(("MQ: %s: cancel TxQ short-lived pkt %p"
				" during chsw...\n",
				__FUNCTION__, pkt));
#if defined(WLAMPDU_MAC)
			flipEpoch |= TXQ_PKT_DEL;
#endif /* defined(WLAMPDU_MAC) */

			wlc_txq_free_pkt(wlc, pkt, time_adj);
			continue;
		}

		/* For the following cases, pkt is freed here:
		 * 1. flag is FLUSHFIFO and all packets should be freed.
		 * 2. flag is FLUSHFIFO_FLUSHID and the flowring ID matches.
		 * 3. flag is FLUSHFIFO_FLUSHSCB and the scb pointer matches.
		 */
		if ((flag == FLUSHFIFO) ||
#ifdef PROP_TXSTATUS
			((flag == FLUSHFIFO_FLUSHID) &&
				(wlc->fifoflush_id == PKTFRAGFLOWRINGID(wlc->osh, pkt))) ||
#endif /* PROP_TXSTATUS */
			((flag == FLUSHFIFO_FLUSHSCB) &&
				(wlc->fifoflush_scb == WLPKTTAGSCBGET(pkt)))) {
#if defined(WLAMPDU_MAC)
			flipEpoch |= TXQ_PKT_DEL;
#endif /* defined(WLAMPDU_MAC) */
			wlc_txq_free_pkt(wlc, pkt, time_adj);
			continue;
		}

		/* For the following cases, pkt needs to be queued back:
		 * 1. flag is SYNCFIFO and packets are supposed to be queued back.
		 * 2. flag is FLUSHFIFO_FLUSHID but the flowring ID doesn't match.
		 * 3. flag is FLUSHFIFO_FLUSHSCB but the scb pointer doesn't match.
		 */
#ifdef PROP_TXSTATUS
		/* Go through the proptxstatus process before queuing back */
		if (wlc_proptxstatus_process_pkt(wlc, pkt, time_adj) == NULL) {
#ifdef WLAMPDU_MAC
			flipEpoch |= TXQ_PKT_DEL;
#endif /* WLAMPDU_MAC */
			continue;
		}
#endif /* PROP_TXSTATUS */

		/* not a host packet or pktfree not allowed
		 * enqueue it back
		 */
#ifdef WLAMPDU_MAC
		wlc_epoch_upd(wlc, pkt, &flipEpoch, &lastEpoch);
		/* clear pkt delete condition */
		flipEpoch &= ~TXQ_PKT_DEL;
#endif /* WLAMPDU_MAC */
		pktenq(&pkt_list, pkt);
	}
	pktq_sw_prepend(swq, fifo_idx, &pkt_list);
}

/**
 * called by BMAC, related to new ucode method of suspend & flushing d11 core tx
 * fifos.
 */
void
wlc_tx_fifo_sync_complete(wlc_info_t *wlc, uint fifo_bitmap, uint8 flag)
{
	struct spktq pkt_list;
	void *pkt;
	txq_t *low_txq;
	uint i;
	uint fbmp;
	uint16 time_adj;
#ifdef HOST_HDR_FETCH
	struct spktq bus_pkt_list;
#endif // endif

	/* check for up first? If we down the driver, maybe we will
	 * get and ignore this call since we flush all txfifo pkts,
	 * and wlc->txfifo_detach_pending would be false.
	 */
	ASSERT(wlc->txfifo_detach_pending);
	ASSERT(wlc->txfifo_detach_transition_queue != NULL);

	WL_MQ(("MQ: %s: TXPENDTOT = %d\n", __FUNCTION__, TXPKTPENDTOT(wlc)));
	wlc_txpend_counts(wlc, __FUNCTION__);

	/* init a local pkt queue to shuffle pkts in this fn */
	pktqinit(&pkt_list, -1);

	/* get the destination queue */
	low_txq = wlc->txfifo_detach_transition_queue->low_txq;

	/* pull all the packets that were queued to HW or RPC layer,
	 * and push them on the low software TxQ
	 */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0) /* not the right fifo to process */
			continue;
		time_adj = 0;
#ifdef HOST_HDR_FETCH
		if (HOST_HDR_FETCH_ENAB()) {
			pktqinit(&bus_pkt_list, -1);

			/* Reclaim pkts from pciedev layer */
			wl_reclaim_bus_txpkts(wlc->wl, &bus_pkt_list, i, FALSE);
			TXDMAPEND_CLR(wlc, i);
			AQMDMA_PENDCLR(wlc, i);

			/* enqueue the collected pkts from DMA ring
			* at the head of the low software TxQ
			*/
			if (pktq_len(&bus_pkt_list) > 0) {
				pktq_sw_prepend(low_txq->swq, i, &bus_pkt_list);
			}
		}
#endif /* HOST_HDR_FETCH */
		if (!PIO_ENAB((wlc)->pub)) {
			hnddma_t *di = WLC_HW_DI(wlc, i);

			if (di == NULL)
				continue;

			while (NULL != (pkt = wlc_bmac_dma_getnexttxp(wlc, i,
					HNDDMA_RANGE_ALL))) {
				pktenq(&pkt_list, pkt);
			}

			/* The DMA should have been cleared of all packets by the
			 * wlc_bmac_dma_getnexttxp() loop above.
			 */
			ASSERT(dma_txactive(di) == 0);
		} else { /* if !PIO */
			pio_t *pio = WLC_HW_PIO(wlc, i);

			if (pio == NULL)
				continue;

			while (NULL != (pkt = wlc_pio_getnexttxp(pio))) {
				pktenq(&pkt_list, pkt);
			}
		} /* if !PIO */

		WL_MQ(("MQ: %s: fifo %d: collected %d pkts\n", __FUNCTION__,
		       i, pktq_len(&pkt_list)));

		/* enqueue the collected pkts from DMA ring
		* at the head of the low software TxQ
		*/
		if (pktq_len(&pkt_list) > 0) {
			pktq_sw_prepend(low_txq->swq, i, &pkt_list);
		}

		/*
		* Account for the collective packets in low_txq at this stage.
		* Need to suppress or flush all packets currently in low_txq
		* depending on the argument 'flag'
		*/
		wlc_low_txq_account(wlc, low_txq, i, flag, &time_adj);

		if ((TXPKTPENDGET(wlc, i) > 0) && (time_adj > 0)) {
			WLC_TXFIFO_COMPLETE(wlc, i, time_adj,
				wlc_txq_buffered_time(low_txq, i));
		}

#ifdef AP
		/**
		 * Rather than checking fids on a per-packet basis using wlc_mbss_dotxstatus_mcmx
		 * for each freed bcmc packet in wlc_low_txq_account, simply 'reset' the bcmc
		 * administration. (a speed optimization)
		 */
		if ((i == TX_BCMC_FIFO) && (TXPKTPENDGET(wlc, i) == 0) &&
			(wlc->txfifo_detach_transition_queue == wlc->primary_queue)) {
			wlc_tx_fifo_sync_bcmc_reset(wlc);
		}
#endif /* AP */

#ifdef WLAMPDU_MAC
		/*
		 * Make a backup of epoch bit settings before completing queue switch.
		 * Packets on the dma ring have already been accounted for at this point and
		 * low_q packet state is finalized. Now find and save appropriate epoch bit
		 * setting which shall be restored when switching back to this queue.
		 *
		 * For multi-destination traffic case, each destination traffic packet stream
		 * would be differentiated from each other by an epoch flip which would have
		 * already been well handled at this point. We are only concerned about the
		 * final packet in order to determine the appropriate epoch state of the FIFO,
		 * for use from the next pkt on wards.
		 *
		 * Note : It would not be handling the selective TID
		 * flush cases (FLUSHFIFO_FLUSHID)
		 * TBD :
		 * 1. Epoch save and restore for NON -AQM cases (SWAMPDU and HWAMPDU)
		 * 2. Handle selective TID flushes (FLUSHFIFO_FLUSHID)
		*/
		if (AMPDU_AQM_ENAB(wlc->pub)) {
			wlc_tx_fifo_epoch_save(wlc, wlc->txfifo_detach_transition_queue, i);
		}
#endif /* WLAMPDU_MAC */

		/* clear any HW 'stopped' flags since the hw fifo is now empty */
		txq_hw_stop_clr(low_txq, i);
	} /* end for */

	wlc->txfifo_detach_transition_queue = NULL;
	wlc->txfifo_detach_pending = FALSE;

} /* wlc_tx_fifo_sync_complete */

void
wlc_tx_fifo_attach_complete(wlc_info_t *wlc)
{
	int i;
	int new_count;
	txq_t *low_q;
#ifdef BCMDBG
	struct {
		uint pend;
		uint pkt;
		uint txp;
	} counts [NFIFO_EXT] = {{0, 0, 0}};
#ifdef WLC_LOW
	hnddma_t *di;
#endif // endif
#endif /* BCMDBG */

	ASSERT(MQUEUE_ENAB(wlc->pub));
	wlc->txfifo_attach_pending = FALSE;

	/* bail out if no new active_queue */
	if (wlc->active_queue == NULL) {
		return;
	}

	low_q = wlc->active_queue->low_txq;

	/* for each HW fifo, set the new pending count when we are attaching a new queue */
	for  (i = 0; i < NFIFO_EXT; i++) {
		TXPKTPENDCLR(wlc, i);

		/* for initial NEW_TXQ implementation, the buffered time is the
		 * weighted TXPKTPEND count.
		 */
		new_count = wlc_txq_buffered_time(low_q, i);

		TXPKTPENDINC(wlc, i, (uint16)new_count);

		/* Clear the hw fifo hold since we are done with the fifo_detach */
		txq_hw_hold_clr(low_q, i);

#ifdef BCMDBG
		counts[i].pend = new_count;
		counts[i].pkt = pktq_plen(low_q->swq, i);
#ifdef WLC_LOW
		/* At the point we are attaching a new queue, the DMA should have been
		 * cleared of all packets
		 */
		if ((di = WLC_HW_DI(wlc, i)) != NULL) {
			ASSERT(dma_txactive(di) == 0);
			counts[i].txp = dma_txp(di);
		}
#endif /* WLC_LOW */
#endif /* BCMDBG */
	}

#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(wlc->pub)) {
		for  (i = 0; i < NFIFO_EXT; i++) {
			/* restore the epoch bit setting for this queue fifo */
			wlc_ampdu_set_epoch(wlc->ampdu_tx, i, wlc->active_queue->epoch[i]);
		}
	}
#endif /* WLAMPDU_MAC */

	WL_MQ(("MQ: %s: New TXPKTPEND/pkt/DMA "
	       "[0]:(%d,%d,%d) [1]:(%d,%d,%d) [2]:(%d,%d,%d) "
	       "[3]:(%d,%d,%d) [4]:(%d,%d,%d) [5]:(%d,%d,%d)\n",
	       __FUNCTION__,
	       counts[0].pend, counts[0].pkt, counts[0].txp,
	       counts[1].pend, counts[1].pkt, counts[1].txp,
	       counts[2].pend, counts[2].pkt, counts[2].txp,
	       counts[3].pend, counts[3].pkt, counts[3].txp,
	       counts[4].pend, counts[4].pkt, counts[4].txp,
	       counts[5].pend, counts[5].pkt, counts[5].txp));
}

void
wlc_tx_fifo_scb_flush(wlc_info_t *wlc, struct scb *remove)
{
	wlc_txq_info_t *active_queue;
	uint i, fbmp, fifo_bitmap = 0x0F;

	/* Flush packets in HW FIFO. */
	if (!wlc->txfifo_detach_pending) {
		if (wlc->excursion_active == FALSE)
			active_queue = wlc->active_queue;
		else
			active_queue = wlc->excursion_queue;

#ifdef WL_MU_TX
		if (wlc_mutx_sta_client_index(wlc->mutx, remove) != MU_CLIENT_INDEX_NONE) {
			fifo_bitmap = 0;
			wlc_mutx_sta_fifo_bitmap(wlc->mutx, remove, &fifo_bitmap);
		}
#endif // endif
		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0)
				continue;
			if (TXPKTPENDGET(wlc, i) <= 0)
				fifo_bitmap &= ~(1 << i);
			else
				txq_hw_hold_set(active_queue->low_txq, i);
		}

		if (fifo_bitmap != 0) {
			wlc->txfifo_detach_transition_queue = active_queue;
			wlc->txfifo_detach_pending = TRUE;
			wlc->fifoflush_scb = remove;
			wlc_bmac_tx_fifo_sync(wlc->hw, fifo_bitmap, FLUSHFIFO_FLUSHSCB);
		}

		for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
			if ((fbmp & 0x01) == 0)
				continue;
			txq_hw_hold_clr(active_queue->low_txq, i);
		}
	}
}
#else /* NEW_TXQ */
void
wlc_tx_fifo_sync_complete(wlc_info_t *wlc, uint fifo_bitmap, uint8 flag)
{
	void *pkt_list;
	void *pkt;
	struct pktq *q;
	uint i;
	uint prec;
	uint fbmp;
	struct scb *scb_pkt = NULL;

#if defined(PROP_TXSTATUS)
	void *pkt_list_sorted = NULL;
#endif // endif

	/* check for up first? If we down the driver, maybe we will
	 * get and ignore this call since we flush all txfifo pkts,
	 * and wlc->txfifo_detach_pending would be false.
	 */
	ASSERT(wlc->txfifo_detach_pending);

	WL_MQ(("MQ: %s: TXPEND = %d\n", __FUNCTION__, TXPKTPENDTOT(wlc)));

	pkt_list = NULL;

#ifdef WLC_HIGH_ONLY
	/* split driver:
	 *    loop fifo,rpctxq instead of DMA and PIO getnexttxp
	 */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0) /* not the right fifo to process */
			continue;

		if (!wlc_rpctx_fifo_enabled(wlc->rpctx, i))
			continue;

		while (NULL != (pkt = GETNEXTTXP(wlc, i))) {
			if (flag == FLUSHFIFO) {
				PKTFREE(wlc->osh, pkt, TRUE);
			} else if ((flag == FLUSHFIFO_FLUSHID) &&
				(wlc->fifoflush_id == PKTFRAGFLOWRINGID(wlc->osh, pkt))) {
					PKTFREE(wlc->osh, pkt, TRUE);
			} else { /* SYNCFIFO */
				PKTSETLINK(pkt, pkt_list);
				pkt_list = pkt;
			}
		}
	}
#else
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0) /* not the right fifo to process */
			continue;

		if (!PIO_ENAB((wlc)->pub)) {
			hnddma_t *di = WLC_HW_DI(wlc, i);

			if (di == NULL)
				continue;

			while (NULL != (pkt = wlc_bmac_dma_getnexttxp(wlc, i, HNDDMA_RANGE_ALL))) {

				if (flag == FLUSHFIFO) {
					PKTFREE(wlc->osh, pkt, TRUE);
				} else if ((flag == FLUSHFIFO_FLUSHID) &&
					(wlc->fifoflush_id == PKTFRAGFLOWRINGID(wlc->osh, pkt))) {
						PKTFREE(wlc->osh, pkt, TRUE);
				} else { /* SYNCFIFO */
					PKTSETLINK(pkt, pkt_list);
					pkt_list = pkt;
					WL_MQ(("MQ: %s: recovering %p prio %d prec %d\n",
					       __FUNCTION__, pkt,
					       PKTPRIO(pkt), WLC_PRIO_TO_PREC(PKTPRIO(pkt))));
				}
			} /* end while(wlc_bmac_dma_getnexttxp) */
		} else { /* if !PIO */
			pio_t *pio = WLC_HW_PIO(wlc, i);

			if (pio == NULL)
				continue;

			while (NULL != (pkt = wlc_pio_getnexttxp(pio))) {
				if (flag == FLUSHFIFO) {
					PKTFREE(wlc->osh, pkt, TRUE);
				} else if ((flag == FLUSHFIFO_FLUSHID) &&
					(wlc->fifoflush_id == PKTFRAGFLOWRINGID(wlc->osh, pkt))) {
					PKTFREE(wlc->osh, pkt, TRUE);
				} else { /* SYNCFIFO */
					PKTSETLINK(pkt, pkt_list);
					pkt_list = pkt;
				}
			}
		} /* if !PIO */
	} /* end for */
#endif /* WLC_HIGH_ONLY */

	q = WLC_GET_TXQ(wlc->txfifo_detach_transition_queue);
	while (pkt_list != NULL) {
		wlc_txh_info_t txh_info;
		pkt = pkt_list;
		pkt_list = PKTLINK(pkt);
		PKTSETLINK(pkt, NULL);

		/* PERF: An inline function with only needed fields can help perf */
		wlc_get_txh_info(wlc, pkt, &txh_info);

		WL_MQ(("MQ: %s: working on %p FID 0x%x\n",
		       __FUNCTION__, pkt, htol16(txh_info.TxFrameID)));

		/* restore the scb for the packet
		 * if there is no scb, the packet should be tossed
		 */
		if (!(scb_pkt = wlc_recover_pkt_scb(wlc, pkt, &txh_info))) {
			/* tossed packet counter??? */
			WL_MQ(("MQ: %s: failed SCB recovery for %p\n",
			       __FUNCTION__, pkt));
			PKTFREE(wlc->osh, pkt, TRUE);
			continue;
		}

#if defined(PROP_TXSTATUS)
		if (PROP_TXSTATUS_ENAB(wlc->pub) &&
			((wlc->excursion_active == FALSE) && (flag != FLUSHFIFO_FLUSHID))) {
			PKTSETLINK(pkt, pkt_list_sorted);
			pkt_list_sorted = pkt;
			continue;
		}
#endif /* defined(PROP_TXSTATUS) */

		/* SWWLAN-39516: Removing WLF3_TXQ_SHORT_LIFETIME optimization.
		 * Could be addressed by pkt lifetime?
		 */
		if (WLPKTTAG(pkt)->flags3 & WLF3_TXQ_SHORT_LIFETIME) {
			WL_TDLS(("MQ: %s: cancel TxQ short-lived pkt %p FID= %d during chsw...\n",
			       __FUNCTION__, pkt, htol16(txh_info.TxFrameID)));
			PKTFREE(wlc->osh, pkt, TRUE);
			continue;
		}

		prec = WLC_PRIO_TO_PREC(PKTPRIO(pkt));

		/* SWWLAN-39513: remove use of FIFOPKT */
		/* Mark as retrieved from HW FIFO */
		WLPKTTAG(pkt)->flags |= WLF_FIFOPKT;

#ifdef PSPRETEND
		if (!SCB_ISMULTI(scb_pkt) && SCB_PS(scb_pkt) &&
		       !(WLPKTTAG(pkt)->flags & WLF_PSDONTQ)) {
			/* consider flushed packet as suppressed */
			if (wlc_apps_scb_supr_enq(wlc, scb_pkt, pkt)) {
				PKTFREE(wlc->osh, pkt, TRUE);
				WLCNTINCR(wlc->pub->_cnt->txnobuf);
				WLCIFCNTINCR(scb, txnobuf);
				WLCNTSCBINCR(scb->scb_stats.tx_failures);
				BCM_REFERENCE(scb);
			}
		}
		else
#endif /* PSPRETEND */
		if (pktq_pfull(q, prec)) {
			struct scb *scb = WLPKTTAGSCBGET(pkt);
			WL_MQ(("MQ: %s: PREC %d full! freeing pkt %p\n", __FUNCTION__,
			       prec, pkt));
			PKTFREE(wlc->osh, pkt, TRUE);
			WLCNTINCR(wlc->pub->_cnt->txnobuf);
			WLCIFCNTINCR(scb, txnobuf);
			WLCNTSCBINCR(scb->scb_stats.tx_failures);
			BCM_REFERENCE(scb);
		} else {
			WL_MQ(("MQ: %s: PREC %d Re-Enqueue pkt %p\n", __FUNCTION__,
			       prec, pkt));
			pktq_penq_head(q, prec, pkt);
		}
	} /* end while (pkt_list != NULL) */
#if defined(PROP_TXSTATUS)
	if (PROP_TXSTATUS_ENAB(wlc->pub) && (flag != FLUSHFIFO_FLUSHID) &&
		(wlc->excursion_active == FALSE)) {
		while (pkt_list_sorted != NULL) {
			bool free_pkt = TRUE;
			pkt = pkt_list_sorted;
			pkt_list_sorted = PKTLINK(pkt);
			PKTSETLINK(pkt, NULL);
			scb_pkt = WLPKTTAGSCBGET(pkt);
			if (!(scb_pkt && SCB_ISMULTI(scb_pkt))) {
				wlc_suppress_sync_fsm(wlc, scb_pkt, pkt, TRUE);
				wlc_process_wlhdr_txstatus(wlc,
					WLFC_CTL_PKTFLAG_WLSUPPRESS, pkt, FALSE);
			}
			if (wlc_should_retry_suppressed_pkt(wlc, pkt, TX_STATUS_SUPR_PMQ) ||
				(scb_pkt && SCB_ISMULTI(scb_pkt))) {
				/* These are dongle generated pkts
				 * dont free and attempt requeue to interface queue
				 */
				/* XXX This enqueues in the wrong order,
				 * should be 'pktq_penq_head()' as above
				*/
				wlc_txq_info_t *qi = SCB_WLCIFP(scb_pkt)->qi;
				struct pktq *q = WLC_GET_TXQ(qi);
				free_pkt = !wlc_prec_enq(wlc, q, pkt,
					WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt)));
			}
			if (free_pkt)
				PKTFREE(wlc->pub->osh, pkt, TRUE);
		}
	}
#endif /* defined(PROP_TXSTATUS) */

#ifdef WLC_HIGH_ONLY
	/* process completions from selected fifos */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0) /* not the right fifo to process */
			continue;

		if (!wlc_rpctx_fifo_enabled(wlc->rpctx, i))
			continue;

		WL_MQ(("MQ: %s: FIFO %d completing %d\n", __FUNCTION__,
			i, TXPKTPENDGET(wlc, i)));

		if (TXPKTPENDGET(wlc, i) > 0) {
			WLC_TXFIFO_COMPLETE(wlc, i, (uint16)TXPKTPENDGET(wlc, i), 0);
		}
	}
#else
	/* Process completions from selected fifos
	 * Don't need txtime for the old datapath, use zero to allow
	 * for consistent use of the macro instead of the real function
	 */
	for (i = 0, fbmp = fifo_bitmap; fbmp; i++, fbmp = fbmp >> 1) {
		if ((fbmp & 0x01) == 0) /* not the right fifo to process */
			continue;

		if ((!PIO_ENAB(wlc->pub) && WLC_HW_DI(wlc, i) == NULL) ||
		    (PIO_ENAB(wlc->pub) && WLC_HW_PIO(wlc, i) == NULL)) {
			continue;
		}

		WL_MQ(("MQ: %s: FIFO %d completing %d\n", __FUNCTION__,
		       i, TXPKTPENDGET(wlc, i)));

		if (TXPKTPENDGET(wlc, i) > 0) {
			WLC_TXFIFO_COMPLETE(wlc, i, (uint16)TXPKTPENDGET(wlc, i), 0);
		}
	}
#endif /* WLC_HIGH_ONLY */

#ifdef WLC_HIGH_ONLY
	/* do deferred txq deletion now */
	if (wlc->txfifo_detach_transition_queue_del) {
		wlc->txfifo_detach_transition_queue_del = FALSE;
		wlc_txq_free(wlc, wlc->osh, wlc->txfifo_detach_transition_queue);
	}
#endif // endif

	wlc->txfifo_detach_transition_queue = NULL;
	wlc->txfifo_detach_pending = FALSE;

#ifdef WLC_HIGH_ONLY
	/* Restart send after switching the queue for split driver */
	if (wlc->active_queue != NULL && WLC_TXQ_OCCUPIED(wlc)) {
		wlc_send_q(wlc, wlc->active_queue);
	}
#endif /* WLC_HIGH_ONLY */
}
#endif /* NEW_TXQ */
#endif /* WL_MULTIQUEUE */

#if defined(WLAMPDU_MAC)
/* Update and save epoch value of the last packet in the queue.
 * Epoch value shall be saved for the queue that is getting dettached,
 * Same shall be restored upon "re-attach" of the given queue in wlc_tx_fifo_attach_complete.
 */
static void
wlc_tx_fifo_epoch_save(wlc_info_t *wlc, wlc_txq_info_t *qi, uint fifo_idx)
{
	txq_t *low_txq;
	struct swpktq *swq;
	void *pkt;
	wlc_txh_info_t txh_info;

	ASSERT(qi != NULL);
	low_txq = qi->low_txq;
	ASSERT(low_txq != NULL);
	swq = low_txq->swq;
	ASSERT(swq != NULL);

	if (fifo_idx < NFIFO_EXT) {
		if ((pkt = swq->q[fifo_idx].tail) != NULL) {
			wlc_get_txh_info(wlc, pkt, &txh_info);
			qi->epoch[fifo_idx] = wlc_txh_get_epoch(wlc, &txh_info);
			/* restore the epoch bit setting for this queue fifo */
			if (qi == wlc->active_queue) {
				wlc_ampdu_set_epoch(wlc->ampdu_tx, fifo_idx, qi->epoch[fifo_idx]);
			}
		}
	}
}

/* After deletion of packet to flip epoch bit of next enque packets in low_txq if last save epoch
 * value equal to next enque packet epoch value otherwise don't flip epoch value of enqueue
 * packets.
 * flipEpoch 0th bit is used to keep track of last packet state i.e deleted or enq.
 * flipEpoch 1st bit is uesed to keep track of flip epoch state.
*/

void
wlc_epoch_upd(wlc_info_t *wlc, void *pkt, uint8 *flipEpoch, uint8 *lastEpoch)
{
	wlc_txh_info_t txh_info;
	uint8 *tsoHdrPtr;
#ifdef HOST_HDR_FETCH
	ASSERT(__PKTISHDRINHOST(wlc->osh, pkt) == 0);
#endif // endif
	wlc_get_txh_info(wlc, pkt, &txh_info);
	tsoHdrPtr = txh_info.tsoHdrPtr;

	/* if last packet was deleted then storing the epoch flip state in 1st bit of flipEpoch */
	if ((*flipEpoch & TXQ_PKT_DEL) && (*lastEpoch != HEAD_PKT_FLUSHED)) {
		if ((*lastEpoch & TOE_F2_EPOCH) == (tsoHdrPtr[2] & TOE_F2_EPOCH))
			*flipEpoch |= TOE_F2_EPOCH;
		else
			*flipEpoch &= ~TOE_F2_EPOCH;
	}

	/* flipping epoch bit of packet if flip epoch state bit (1st bit) is set */
	if (*flipEpoch & TOE_F2_EPOCH) {
		tsoHdrPtr[2] ^= TOE_F2_EPOCH;
	}

	/* storing current epoch bit */
	*lastEpoch = (tsoHdrPtr[2] & TOE_F2_EPOCH);
}
#endif /* WLAMPDU_MAC */

#ifdef WLAMPDU
bool BCMFASTPATH
wlc_txh_get_isAMPDU(wlc_info_t* wlc, const wlc_txh_info_t* txh_info)
{
	bool isAMPDU = FALSE;
	uint16 mcl;
	ASSERT(wlc);
	ASSERT(txh_info);

	mcl = ltoh16(txh_info->MacTxControlLow);
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		isAMPDU = ((mcl & D11AC_TXC_AMPDU) != 0);
	} else {
		isAMPDU = ((mcl & TXC_AMPDU_MASK) != TXC_AMPDU_NONE);
	}

	return isAMPDU;
}

/**
 * Set the epoch value in the given packet data.
 * This function is only for AQM hardware (d11 core rev >= 40) headers.
 *
 * @param wlc           wlc_info_t pointer
 * @param pdata	        pointer to the beginning of the packet data which
 *                      should be the TxD hardware header.
 * @param epoch         the epoch value (0/1) to set
 */
void BCMFASTPATH
wlc_txh_set_epoch(wlc_info_t* wlc, uint8* pdata, uint8 epoch)
{
	uint8* tsoHdrPtr = pdata;

	ASSERT(D11REV_GE(wlc->pub->corerev, 40));
	ASSERT(pdata != NULL);

	if (epoch) {
		tsoHdrPtr[2] |= TOE_F2_EPOCH;
	} else {
		tsoHdrPtr[2] &= ~TOE_F2_EPOCH;
	}

	/* JIRA:CRWLDOT11M-978:
	 * PR105383 WAR: add soft-epoch in tx-descriptor to work-around
	 * the problem of aggregating across epoch.
	 */
	if (D11REV_LT(wlc->pub->corerev, 42)) {
		uint tsoHdrSize;
		d11actxh_t* vhtHdr;

		tsoHdrSize = ((tsoHdrPtr[0] & TOE_F0_HDRSIZ_NORMAL) ?
		              TSO_HEADER_LENGTH : TSO_HEADER_PASSTHROUGH_LENGTH);

		vhtHdr = (d11actxh_t*)(tsoHdrPtr + tsoHdrSize);

		vhtHdr->PktInfo.TSOInfo = htol16(tsoHdrPtr[2] << 8);
	}
}

/**
 * Get the epoch value from the given packet data.
 * This function is only for AQM hardware (d11 core rev >= 40) headers.
 *
 * @param wlc           wlc_info_t pointer
 * @param pdata	        pointer to the beginning of the packet data which
 *                      should be the TxD hardware header.
 * @return              The fucntion returns the epoch value (0/1) from the frame
 */
uint8
wlc_txh_get_epoch(wlc_info_t* wlc, wlc_txh_info_t* txh_info)
{
	uint8* tsoHdrPtr = txh_info->tsoHdrPtr;

	ASSERT(D11REV_GE(wlc->pub->corerev, 40));
	ASSERT(txh_info->tsoHdrPtr != NULL);

	return ((tsoHdrPtr[2] & TOE_F2_EPOCH)? 1 : 0);
}

/* wlc_compute_ampdu_mpdu_dur()
 *
 * Calculate the 802.11 MAC header DUR field for MPDU in an A-AMPDU
 * DUR for MPDU = 1 SIFS + 1 BA
 *
 * rate			MPDU rate in unit of 500kbps
 */
static uint16
wlc_compute_ampdu_mpdu_dur(wlc_info_t *wlc, ratespec_t rate)
{
	uint16 dur = SIFS(wlc->band);

	dur += (uint16)wlc_calc_ba_time(wlc->hti, rate, WLC_SHORT_PREAMBLE);

	return (dur);
}
#endif /* WLAMPDU */

#ifdef STA
static void
wlc_pm_tx_upd(wlc_info_t *wlc, struct scb *scb, void *pkt, bool ps0ok, uint fifo)
{
	wlc_bsscfg_t *cfg;
	wlc_pm_st_t *pm;

	WL_RTDC(wlc, "wlc_pm_tx_upd", 0, 0);

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	pm = cfg->pm;

	WL_NONE(("%s: wme=%d, qos=%d, pm_ena=%d, pm_pend=%d\n", __FUNCTION__,
		WME_ENAB(wlc->pub), SCB_QOS(scb), pm->PMenabled, pm->PMpending));

	if (!pm->PMenabled ||
#ifdef WLTDLS
	    (BSS_TDLS_ENAB(wlc, cfg) && !cfg->tdls->tdls_PMEnable) ||
#endif // endif
	    (!cfg->BSS && !BSS_TDLS_ENAB(wlc, cfg) && !BSSCFG_IS_AIBSS_PS_ENAB(cfg)))
		return;

	/* Turn off PM mode */
	/* Do not leave PS mode on a tx if the Receive Throttle
	 * feature is  enabled. Leaving PS mode during the OFF
	 * part of the receive throttle duty cycle
	 * for any reason will defeat the whole purpose
	 * of the OFF part of the duty cycle - heat reduction.
	 */
	if (!PM2_RCV_DUR_ENAB(cfg)) {
		/* Leave PS mode if in fast PM mode */
		if (pm->PM == PM_FAST &&
		    ps0ok &&
		    !pm->PMblocked) {
			/* XXX JQL: we need to wait until PM is indicated to the BSS
			 * (PMpending is FALSE) when turning on PM mode but in case
			 * transmitting another back to back frame which is a trigger to
			 * turn off the PM mode we use a boolean pm2_ps0_allowed to
			 * control if turning off PM mode is allowed when turning on PM mode
			 * is in progress...
			 */
			if (!pm->PMpending || pm->pm2_ps0_allowed) {
				WL_RTDC(wlc, "wlc_pm_tx_upd: exit PS", 0, 0);
				wlc_set_pmstate(cfg, FALSE);
#ifdef WL_PWRSTATS
				if (PWRSTATS_ENAB(wlc->pub)) {
					wlc_pwrstats_set_frts_data(wlc->pwrstats, TRUE);
				}
#endif /* WL_PWRSTATS */
				wlc_pm2_sleep_ret_timer_start(cfg);
			}
			else if (pm->PMpending) {
				pm->pm2_ps0_allowed = TRUE;
			}
		}
	}

#ifdef WME
	/* Start an APSD USP */
	ASSERT(WLPKTTAG(pkt)->flags & WLF_TXHDR);

	/* If sending APSD trigger frame, stay awake until EOSP */
	if (WME_ENAB(wlc->pub) &&
	    SCB_QOS(scb) &&
	    /* APSD trigger is only meaningful in PS mode */
	    pm->PMenabled &&
	    /* APSD trigger must not be any PM transitional frame */
	    !pm->PMpending) {
		struct dot11_header *h;
		wlc_txh_info_t txh_info;
		uint16 kind;
		bool qos;
#ifdef WLTDLS
		uint16 qosctrl;
#endif // endif

		wlc_get_txh_info(wlc, pkt, &txh_info);
		h = txh_info.d11HdrPtr;
		kind = (ltoh16(h->fc) & FC_KIND_MASK);
		qos = (kind  == FC_QOS_DATA || kind == FC_QOS_NULL);

#ifdef WLTDLS
		qosctrl = qos ? ltoh16(*(uint16 *)((uint8 *)h + DOT11_A3_HDR_LEN)) : 0;
#endif // endif
		WL_NONE(("%s: qos=%d, fifo=%d, trig=%d, sta_usp=%d\n", __FUNCTION__,
			qos, fifo, AC_BITMAP_TST(scb->apsd.ac_trig,
			WME_PRIO2AC(PKTPRIO(pkt))), pm->apsd_sta_usp));

		if (qos &&
		    fifo != TX_BCMC_FIFO &&
		    AC_BITMAP_TST(scb->apsd.ac_trig, WME_PRIO2AC(PKTPRIO(pkt))) &&
#ifdef WLTDLS
			(!BSS_TDLS_ENAB(wlc, cfg) || kind != FC_QOS_NULL ||
			!QOS_EOSP(qosctrl)) &&
#endif // endif
			TRUE) {
#ifdef WLTDLS
			if (BSS_TDLS_ENAB(wlc, cfg))
				cfg->tdls->apsd_sta_settime = OSL_SYSUPTIME();
#endif // endif
			if (!pm->apsd_sta_usp) {
				WL_PS(("wl%d.%d: APSD wake\n",
				       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
				wlc_set_apsd_stausp(cfg, TRUE);
#ifdef WLP2P
				/* APSD trigger frame is being sent, and re-trigger frame
				 * is expected when the next presence period starts unless
				 * U-APSD EOSP is received between now and then.
				 */
				if (BSS_P2P_ENAB(wlc, cfg))
					wlc_p2p_apsd_retrigger_upd(wlc->p2p, cfg, TRUE);
#endif // endif
			}
			scb->flags |= SCB_SENT_APSD_TRIG;
		}
	}
#endif /* WME */
}
/* Given an ethernet packet, prepare the packet as if for transmission. The driver needs to program
 * the frame in the template and also provide d11txh equivalent information to it
 */
void *
wlc_sdu_to_pdu(wlc_info_t *wlc, void *sdu, struct scb *scb, bool is_8021x)
{
	void *pkt;
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
	uint8 prio = 0;
	uint frag_length = 0;
	uint pkt_length, nfrags;
	wlc_bsscfg_t *bsscfg;

	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	/* wlc_hdr_proc --> Ether to 802.3 */
	pkt = wlc_hdr_proc(wlc, sdu, scb);

	if (pkt == NULL) {
		PKTFREE(wlc->osh, sdu, TRUE);
		return NULL;
	}

	ASSERT(sdu == pkt);

	prio = 0;
	if (SCB_QOS(scb)) {
		prio = (uint8)PKTPRIO(sdu);
		ASSERT(prio <= MAXPRIO);
	}

	if (WSEC_ENABLED(bsscfg->wsec) && !BSSCFG_SAFEMODE(bsscfg)) {
		/* Use a paired key or primary group key if present */
		key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
			WLC_KEY_FLAG_NONE, &key_info);
		if (key_info.algo != CRYPTO_ALGO_OFF) {
			WL_WSEC(("wl%d.%d: %s: using pairwise key\n",
				WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		} else if (is_8021x && WSEC_SES_OW_ENABLED(bsscfg->wsec)) {
			WL_WSEC(("wl%d.%d: wlc_sdu_to_pdu: not encrypting 802.1x frame "
					"during OW\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		} else {
			key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE, &key_info);
			if (key_info.algo != CRYPTO_ALGO_OFF) {
				WL_WSEC(("wl%d.%d: %s: using group key",
					WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
			} else {
				WL_ERROR(("wl%d.%d: %s: no key for encryption\n",
					WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
			}
		}
	} else {
		key = wlc_keymgmt_get_bss_key(wlc->keymgmt, bsscfg, WLC_KEY_ID_INVALID, &key_info);
		ASSERT(key != NULL);
	}

	pkt_length = pkttotlen(wlc->osh, pkt);

	/* TKIP MIC space reservation */
	if (key_info.algo == CRYPTO_ALGO_TKIP) {
		pkt_length += TKIP_MIC_SIZE;
		PKTSETLEN(wlc->osh, pkt, pkt_length);
	}

	nfrags = wlc_frag(wlc, scb, wme_fifo2ac[TX_AC_BE_FIFO], pkt_length, &frag_length);
	BCM_REFERENCE(nfrags);
	ASSERT(nfrags == 1);

	/* This header should not be installed to or taken from Header Cache */
	WLPKTTAG(pkt)->flags |= WLF_BYPASS_TXC;

	/* wlc_dofrag --> Get the d11hdr put on it with TKIP MIC at the tail for TKIP */
	wlc_dofrag(wlc, pkt, 0, 1, 0, scb, is_8021x, TX_AC_BE_FIFO, key,
		&key_info, prio, frag_length);

	return pkt;
}

bool
wlc_is_4way_msg(wlc_info_t *wlc, void *pkt, int offset, wpa_msg_t msg)
{
	struct dot11_llc_snap_header * lsh;
	eapol_hdr_t *eapol = NULL;
	eapol_wpa_key_header_t *ek;
	unsigned short us_tmp;
	int pair, ack, mic, kerr, req, sec, install;
	bool is_4way_msg = FALSE;
	osl_t *osh = wlc->osh;
	uint body_len = pkttotlen(osh, pkt) - offset;
	uchar *pbody = (uchar*)(PKTDATA(osh, pkt) + offset);
	uint len1;

	if (body_len <
	    (DOT11_LLC_SNAP_HDR_LEN +
	     EAPOL_HDR_LEN +
	     EAPOL_WPA_KEY_LEN))
		return FALSE;

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	/* For packet is txfrag, the LLC SNAP header is pktfetched with eapol
	 * when BCM_DHDHDR is enabled.
	 */
	if (BCMDHDHDR_ENAB() && PKTISTXFRAG(osh, pkt)) {
		len1 = PKTLEN(osh, pkt);
		if (len1 < ETHER_HDR_LEN)
			return FALSE;

		/* LLC SNAP header is at start of next packet */
		pkt = PKTNEXT(osh, pkt);
		if (!pkt)
			return FALSE;

		offset = 0;
		pbody = PKTDATA(osh, pkt);
	}
#endif /* BCM_DHDHDR && DONGLEBUILD */
	{
		len1 = PKTLEN(osh, pkt) - offset;
		if (len1 >= DOT11_LLC_SNAP_HDR_LEN)
			lsh = (struct dot11_llc_snap_header *)(pbody);
		else
			return FALSE;
		len1 -= DOT11_LLC_SNAP_HDR_LEN;
		pbody += DOT11_LLC_SNAP_HDR_LEN;

		if (!(lsh->dsap == 0xaa &&
		      lsh->ssap == 0xaa &&
		      lsh->ctl == 0x03 &&
		      lsh->oui[0] == 0 &&
		      lsh->oui[1] == 0 &&
		      lsh->oui[2] == 0 &&
		      ntoh16(lsh->type) == ETHER_TYPE_802_1X)) {
			return FALSE;
		}
	}

	/* assuming the eapol msg at the start of the pkt->next */
	if (len1 < EAPOL_HDR_LEN)
		pbody = PKTDATA(osh, PKTNEXT(osh, pkt));

	eapol = (eapol_hdr_t*)pbody;

	/* make sure it is an EAPOL-Key frame big enough for an 802.11 Key desc */
	if ((eapol->version != WPA_EAPOL_VERSION && eapol->version != WPA2_EAPOL_VERSION) ||
	    (eapol->type != EAPOL_KEY) ||
	    (ntoh16(eapol->length) < EAPOL_WPA_KEY_LEN))
		return FALSE;

	ek = (eapol_wpa_key_header_t*)((int8*)eapol + EAPOL_HDR_LEN);
	/* m1 & m4 signatures on wpa are same on wpa2 */
	if (ek->type == EAPOL_WPA2_KEY || ek->type == EAPOL_WPA_KEY)
	{
		us_tmp = ntoh16_ua(&ek->key_info);

		pair = 	0 != (us_tmp & WPA_KEY_PAIRWISE);
		ack = 0	 != (us_tmp & WPA_KEY_ACK);
		mic = 0	 != (us_tmp & WPA_KEY_MIC);
		kerr = 	0 != (us_tmp & WPA_KEY_ERROR);
		req = 0	 != (us_tmp & WPA_KEY_REQ);
		sec = 0	 != (us_tmp & WPA_KEY_SECURE);
		install	 = 0 != (us_tmp & WPA_KEY_INSTALL);
		switch (msg) {
			case PMSG4:
		/* check if it matches 4-Way M4 signature */
				if (pair && !req && mic && !ack && !kerr && ek->data_len == 0)
					is_4way_msg = TRUE;
				break;
			case PMSG1:
				/* check if it matches 4-way M1 signature */
				if (!sec && !mic && ack && !install && pair && !kerr && !req)
					is_4way_msg = TRUE;

			break;
			default:
			break;
		}
	}

	return is_4way_msg;
}

#endif /* STA */

wlc_txq_info_t*
wlc_txq_alloc(wlc_info_t *wlc, osl_t *osh)
{
	wlc_txq_info_t *qi, *p;
#ifndef DONGLEBUILD
	int i;
#endif // endif
#if defined(TXQ_MUX)
	uint ac;
#endif /* TXQ_MUX */
	uint no_hw_fifos = NFIFO;

	qi = (wlc_txq_info_t*)MALLOCZ(osh, sizeof(wlc_txq_info_t));
	if (qi == NULL) {
		return NULL;
	}

#ifdef DONGLEBUILD
	/* JIRA:SWWLAN-32956: keep rte queue eviction code as-is until it does not rely on
	 * packet eviciton for pkt buffer management
	 */

	/* Have enough room for control packets along with HI watermark */
	/* Also, add room to txq for total psq packets if all the SCBs leave PS mode */
	/* The watermark for flowcontrol to OS packets will remain the same */
	pktq_init(WLC_GET_TXQ(qi), WLC_PREC_COUNT,
	          (2 * wlc->pub->tunables->datahiwat) + PKTQ_LEN_DEFAULT +
	          wlc->pub->psq_pkts_total);
#else
	/* Set the overall queue packet limit to the max, just rely on per-prec limits */
	pktq_init(WLC_GET_TXQ(qi), WLC_PREC_COUNT, PKTQ_LEN_MAX);

	/* Have enough room for control packets along with HI watermark */
	/* Also, add room to txq for total psq packets if all the SCBs leave PS mode */
	/* The watermark for flowcontrol to OS packets will remain the same */
	for (i = 0; i < WLC_PREC_COUNT; i++) {
		pktq_set_max_plen(WLC_GET_TXQ(qi), i,
		                  (2 * wlc->pub->tunables->datahiwat) + PKTQ_LEN_DEFAULT +
		                  wlc->pub->psq_pkts_total);
	}
#endif /* DONGLEBUILD */

	/* add this queue to the the global list */
	p = wlc->tx_queues;
	if (p == NULL) {
		wlc->tx_queues = qi;
	} else {
		while (p->next != NULL)
			p = p->next;
		p->next = qi;
	}

	WL_MQ(("MQ: %s: qi %p\n", __FUNCTION__, qi));

#ifdef NEW_TXQ
	/* Note call to wlc_low_txq_alloc() uses txq as supply context if mux is enabled,
	 * wlc otherwise
	 */

#ifdef TXQ_MUX
	/*
	 * Allocate the low txq to accompany this queue context
	 * Allocated for all physical queues in the device
	 *
	 */
	qi->wlc = wlc;

	/* Allocate queues for the 4 data, bcmc and atim FIFOs */
	qi->low_txq = wlc_low_txq_alloc(wlc->txqi, wlc_pull_q, qi, NFIFO,
	                                wlc->pub->tunables->txq_highwater,
	                                wlc->pub->tunables->txq_lowwater);

	if (qi->low_txq == NULL) {
		WL_ERROR(("wl%d: %s: wlc_low_txq_alloc failed\n", wlc->pub->unit, __FUNCTION__));
		wlc_txq_free(wlc, osh, qi);
		return NULL;
	}

	/* Allocate a mux for each FIFO and create our immediate path MUX input
	 * Allocate an immediate queue handler only for the 4 data FIFOs
	 */
	for (ac = 0; ac < NFIFO; ac++) {
		wlc_mux_t *mux = wlc_mux_alloc(wlc->tx_mux, ac, WLC_TX_MUX_QUANTA);
		int err = 0;

		qi->ac_mux[ac] = mux;

		if ((mux != NULL) && (ac < AC_COUNT)) {
			err = wlc_mux_add_source(mux,
			                         qi,
			                         wlc_txq_immediate_output,
			                         &qi->mux_hdl[ac]);

			if (err) {
				WL_ERROR(("wl%d:%s wlc_mux_add_member() failed err = %d\n",
				          wlc->pub->unit, __FUNCTION__, err));
			}
		}

		if (mux == NULL || err) {
			wlc_txq_free(wlc, osh, qi);
			qi = NULL;
			break;
		}
	}
#else
#if defined(BCM_DMA_CT) && !defined(BCM_DMA_CT_DISABLED)
	no_hw_fifos = NFIFO_EXT;
#endif // endif
	/*
	 * Allocate the low txq to accompany this queue context
	 * Allocated for all physical queues in the device
	 */
	qi->low_txq = wlc_low_txq_alloc(wlc->txqi, wlc_pull_q, wlc, no_hw_fifos,
		wlc_get_txmaxpkts(wlc), wlc_get_txmaxpkts(wlc)/2);

	if (qi->low_txq == NULL) {
		WL_ERROR(("wl%d: %s: wlc_low_txq_alloc failed\n", wlc->pub->unit, __FUNCTION__));
		wlc_txq_free(wlc, osh, qi);
		return NULL;
	}
#endif /* TXQ_MUX */

	WL_MQ(("MQ: %s: qi %p\n", __FUNCTION__, qi));

#endif /* NEW_TXQ */

	return qi;
}

void
wlc_txq_free(wlc_info_t *wlc, osl_t *osh, wlc_txq_info_t *qi)
{
	wlc_txq_info_t *p;
#if defined(TXQ_MUX)
	uint ac;
#endif /* TXQ_MUX */

	if (qi == NULL)
		return;

#ifdef WLC_HIGH_ONLY
	/* defer txq deletion until detach transition is done.
	 * (i.e. wlc_tx_fifo_sync_complete() is finished,
	 * which is asynchronous in BMAC architecture...)
	 */
	if (wlc->txfifo_detach_pending &&
	    wlc->txfifo_detach_transition_queue == qi) {
		WL_MQ(("MQ: %s: defer qi %p\n", __FUNCTION__, qi));
		wlc->txfifo_detach_transition_queue_del = TRUE;
		return;
	}
#endif /* WLC_HIGH_ONLY */

	WL_MQ(("MQ: %s: qi %p\n", __FUNCTION__, qi));

	/* remove the queue from the linked list */
	p = wlc->tx_queues;
	if (p == qi)
		wlc->tx_queues = p->next;
	else {
		while (p != NULL && p->next != qi)
			p = p->next;
		if (p == NULL)
			WL_ERROR(("%s: null ptr2", __FUNCTION__));

		/* assert that we found qi before getting to the end of the list */
		ASSERT(p != NULL);

		if (p != NULL) {
			p->next = p->next->next;
		}
	}

#ifdef NEW_TXQ
#ifdef TXQ_MUX

	/* free the immediate queue per-AC MUX sources that connect to the MUXs */
	for (ac = 0; ac < ARRAYSIZE(qi->mux_hdl); ac++) {
		mux_source_handle_t mux_src = qi->mux_hdl[ac];

		/* the mux source pointer may be NULL if there was a
		 * malloc failure during wlc_txq_alloc()
		 */
		if (mux_src != NULL) {
			wlc_mux_del_source(qi->ac_mux[ac], mux_src);

			/* clear the pointer to tidy up */
			qi->mux_hdl[ac] = NULL;
		}
	}

	/* free the per-FIFO MUXs */
	for (ac = 0; ac < ARRAYSIZE(qi->ac_mux); ac++) {
		wlc_mux_t *mux = qi->ac_mux[ac];

		/* the mux pointer may be NULL if there was a
		 * malloc failure during wlc_txq_alloc()
		 */
		if (mux != NULL) {
			wlc_mux_free(mux);

			/* clear the pointer to tidy up */
			qi->ac_mux[ac] = NULL;
		}
	}

#endif /* TXQ_MUX */

	/* Free the low_txq accompanying this txq context */
	wlc_low_txq_free(wlc->txqi, qi->low_txq);

#endif /* NEW_TXQ */

#ifdef PKTQ_LOG
	wlc_pktq_stats_free(wlc, WLC_GET_TXQ(qi));
#endif // endif
	ASSERT(pktq_empty(WLC_GET_TXQ(qi)));
	MFREE(osh, qi, sizeof(wlc_txq_info_t));
}

static void
wlc_txflowcontrol_signal(wlc_info_t *wlc, wlc_txq_info_t *qi, bool on, int prio)
{
	wlc_if_t *wlcif;
	uint curr_qi_stopped = qi->stopped;

	for (wlcif = wlc->wlcif_list; wlcif != NULL; wlcif = wlcif->next) {
		if (curr_qi_stopped != qi->stopped) {
			/* This tells us that while performing wl_txflowcontrol(),
			 * the qi->stopped state changed.
			 * This can happen when turning wl flowcontrol OFF since in
			 * the process of draining packets from wl layer, flow control
			 * can get turned back on.
			 */
			WL_ERROR(("wl%d: qi(%p) stopped changed from 0x%x to 0x%x, exit %s\n",
			          wlc->pub->unit, qi, curr_qi_stopped, qi->stopped, __FUNCTION__));
			break;
		}
		if (wlcif->qi == qi &&
		    wlcif->if_flags & WLC_IF_LINKED)
			wl_txflowcontrol(wlc->wl, wlcif->wlif, on, prio);
	}
}

void
wlc_txflowcontrol_reset(wlc_info_t *wlc)
{
	wlc_txq_info_t *qi;

	for (qi = wlc->tx_queues; qi != NULL; qi = qi->next) {
		wlc_txflowcontrol_reset_qi(wlc, qi);
	}
}

/* check for the particular priority flow control bit being set */
bool
wlc_txflowcontrol_prio_isset(wlc_info_t *wlc, wlc_txq_info_t *q, int prio)
{
	uint prio_mask;

	if (prio == ALLPRIO) {
		prio_mask = TXQ_STOP_FOR_PRIOFC_MASK;
	} else {
		ASSERT(prio >= 0 && prio <= MAXPRIO);
		prio_mask = NBITVAL(prio);
	}

	return (q->stopped & prio_mask) == prio_mask;
}

/* check if a particular override is set for a queue */
bool
wlc_txflowcontrol_override_isset(wlc_info_t *wlc, wlc_txq_info_t *qi, uint override)
{
	/* override should have some bit on */
	ASSERT(override != 0);
	/* override should not have a prio bit on */
	ASSERT((override & TXQ_STOP_FOR_PRIOFC_MASK) == 0);

	return ((qi->stopped & override) != 0);
}

/* propagate the flow control to all interfaces using the given tx queue */
void
wlc_txflowcontrol(wlc_info_t *wlc, wlc_txq_info_t *qi, bool on, int prio)
{
	uint prio_bits;
	uint cur_bits;

	if (prio == ALLPRIO) {
		prio_bits = TXQ_STOP_FOR_PRIOFC_MASK;
	} else {
		ASSERT(prio >= 0 && prio <= MAXPRIO);
		prio_bits = NBITVAL(prio);
	}

	cur_bits = qi->stopped & prio_bits;

	/* Check for the case of no change and return early
	 * Otherwise update the bit and continue
	 */
	if (on) {
		if (cur_bits == prio_bits) {
			return;
		}
		mboolset(qi->stopped, prio_bits);
	} else {
		if (cur_bits == 0) {
			return;
		}
		mboolclr(qi->stopped, prio_bits);
	}

	/* If there is a flow control override we will not change the external
	 * flow control state.
	 */
	if (qi->stopped & ~TXQ_STOP_FOR_PRIOFC_MASK) {
		return;
	}

	wlc_txflowcontrol_signal(wlc, qi, on, prio);
}

void
wlc_txflowcontrol_override(wlc_info_t *wlc, wlc_txq_info_t *qi, bool on, uint override)
{
	uint prev_override;

	ASSERT(override != 0);
	ASSERT((override & TXQ_STOP_FOR_PRIOFC_MASK) == 0);
	ASSERT(qi != NULL);

	prev_override = (qi->stopped & ~TXQ_STOP_FOR_PRIOFC_MASK);

	/* Update the flow control bits and do an early return if there is
	 * no change in the external flow control state.
	 */
	if (on) {
		mboolset(qi->stopped, override);
		/* if there was a previous override bit on, then setting this
		 * makes no difference.
		 */
		if (prev_override) {
			return;
		}

		wlc_txflowcontrol_signal(wlc, qi, ON, ALLPRIO);
	} else {
		mboolclr(qi->stopped, override);
		/* clearing an override bit will only make a difference for
		 * flow control if it was the only bit set. For any other
		 * override setting, just return
		 */
		if (prev_override != override) {
			return;
		}

		if (qi->stopped == 0) {
			wlc_txflowcontrol_signal(wlc, qi, OFF, ALLPRIO);
		} else {
			int prio;

			for (prio = MAXPRIO; prio >= 0; prio--) {
				if (!mboolisset(qi->stopped, NBITVAL(prio)))
					wlc_txflowcontrol_signal(wlc, qi, OFF, prio);
			}
		}
	}
}

void
wlc_txflowcontrol_reset_qi(wlc_info_t *wlc, wlc_txq_info_t *qi)
{
	ASSERT(qi != NULL);

	if (qi->stopped) {
		wlc_txflowcontrol_signal(wlc, qi, OFF, ALLPRIO);
		qi->stopped = 0;
	}
}

void BCMFASTPATH
wlc_txq_enq(void *ctx, struct scb *scb, void *sdu, uint prec)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_txq_info_t *qi = SCB_WLCIFP(scb)->qi;
	struct pktq *q = WLC_GET_TXQ(qi);
	int prio;
	int datahiwat;

#ifdef TXQ_MUX
	WL_ERROR(("%s() scb:0x%p sdu:0x%p prec:0x%x\n", __FUNCTION__, scb, sdu, prec));
#endif // endif
	ASSERT(!PKTISCHAINED(sdu));

	prio = PKTPRIO(sdu);
	datahiwat = (WLPKTTAG(sdu)->flags & WLF_AMPDU_MPDU)
		  ? wlc->pub->tunables->ampdudatahiwat
		  : wlc->pub->tunables->datahiwat;

	ASSERT(pktq_max(q) >= datahiwat);
#ifdef BCMDBG_POOL
	if (WLPKTTAG(sdu)->flags & WLF_PHDR) {
		void *pdata;

		pdata = PKTNEXT(wlc->pub.osh, sdu);
		ASSERT(pdata);
		ASSERT(WLPKTTAG(pdata)->flags & WLF_DATA);
		PKTPOOLSETSTATE(pdata, POOL_TXENQ);
	}
	else {
		PKTPOOLSETSTATE(sdu, POOL_TXENQ);
	}
#endif // endif

#ifdef TXQ_MUX
{
	void * tmp_pkt = sdu;

	while (tmp_pkt) {
		prhex(__FUNCTION__, PKTDATA(wlc->osh, tmp_pkt), PKTLEN(wlc->osh, tmp_pkt));
		tmp_pkt = PKTNEXT(wlc->osh, tmp_pkt);
	}
}
#endif // endif

#if defined(MACOSX) && !defined(NEW_TXQ)
	/* Change the precedence and priority to VO if
	* is EAPOL packet
	*/
	if ((WLPKTTAG(sdu)->flags & WLF_8021X)) {
		prio = PRIO_8021D_VO;
		prec = prio;
	}
#endif // endif
	if (!wlc_prec_enq(wlc, q, sdu, prec)) {
		if (!EDCF_ENAB(wlc->pub) || (wlc->pub->wlfeatureflag & WL_SWFL_FLOWCONTROL))
			WL_ERROR(("wl%d: %s: txq overflow\n", wlc->pub->unit, __FUNCTION__));
		else
			WL_INFORM(("wl%d: %s: txq overflow\n", wlc->pub->unit, __FUNCTION__));

		PKTDBG_TRACE(wlc->osh, sdu, PKTLIST_FAIL_PRECQ);
		PKTFREE(wlc->osh, sdu, TRUE);
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		WLCIFCNTINCR(scb, txnobuf);
		WLCNTSCB_COND_INCR(scb, scb->scb_stats.tx_failures);
#ifdef WL11K
		wlc_rrm_stat_qos_counter(scb, prio, OFFSETOF(rrm_stat_group_qos_t, txfail));
#endif // endif
	}

	/* Check if flow control needs to be turned on after enqueuing the packet
	 *   Don't turn on flow control if EDCF is enabled. Driver would make the decision on what
	 *   to drop instead of relying on stack to make the right decision
	 */
	if (!EDCF_ENAB(wlc->pub) || (wlc->pub->wlfeatureflag & WL_SWFL_FLOWCONTROL)) {
		if (pktq_len(q) >= datahiwat) {
			wlc_txflowcontrol(wlc, qi, ON, ALLPRIO);
		}
	} else if (wlc->pub->_priofc) {
		if (pktq_plen(q, wlc_prio2prec_map[prio]) >= datahiwat) {
			wlc_txflowcontrol(wlc, qi, ON, prio);
		}
	}
}

uint BCMFASTPATH
wlc_txq_txpktcnt(void *ctx)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_txq_info_t *qi = wlc->active_queue;
	uint16 q_len = pktq_len(WLC_GET_TXQ(qi));

#ifdef NEW_TXQ
	q_len += pktq_len(qi->low_txq->swq);
#endif // endif
	return (uint)q_len;
}

/*
 * bcmc_fid_generate:
 * Generate frame ID for a BCMC packet.  The frag field is not used
 * for MC frames so is used as part of the sequence number.
 */
uint16
bcmc_fid_generate(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint16 txFrameID)
{
	uint16 frameid;

	frameid = ltoh16(txFrameID) & ~(TXFID_SEQ_MASK | TXFID_QUEUE_MASK);
	frameid |= (((wlc->mc_fid_counter++) << TXFID_SEQ_SHIFT) & TXFID_SEQ_MASK) |
		TX_BCMC_FIFO;

	return frameid;
}

/* Get the number of fifos configured for the specified txq */
uint
wlc_txq_nfifo(txq_t *txq)
{
	return (txq->nswq);
}

/* get the value of the txmaxpkts */
uint16
wlc_get_txmaxpkts(wlc_info_t *wlc)
{
	return wlc->pub->txmaxpkts;
}

/* Set the value of the txmaxpkts */
void
wlc_set_txmaxpkts(wlc_info_t *wlc, uint16 txmaxpkts)
{
#if defined(NEW_TXQ) && !defined(TXQ_MUX)
/* XXX
 * when TXQ_MUX is defined the watermarks are timebased
 * otherwise it is packet based
 */
	wlc_txq_info_t *qi;
	for (qi = wlc->tx_queues; qi != NULL; qi = qi->next) {
		wlc_low_txq_set_watermark(qi->low_txq, txmaxpkts, txmaxpkts - 1);
	}

#endif /* NEW_TXQ && !TXQ_MUX */
	wlc->pub->txmaxpkts = txmaxpkts;
}

/* Reset txmaxpkts to their defaults */
void
wlc_set_default_txmaxpkts(wlc_info_t *wlc)
{
	uint16 txmaxpkts = MAXTXPKTS;

#ifdef WLAMPDU_MAC
	if (wlc->pub->_ampdumac == AMPDU_AGG_AQM)
		txmaxpkts = MAXTXPKTS_AMPDUAQM_DFT;
	else if (AMPDU_UCODE_ENAB(wlc->pub) || AMPDU_HW_ENAB(wlc->pub))
		txmaxpkts = MAXTXPKTS_AMPDUMAC;
#endif // endif
	wlc_set_txmaxpkts(wlc, txmaxpkts);
}

#ifdef PROP_TXSTATUS
/* De-queue the packets from all txq and low_txq, and free them for the flow ring pkts */
void
wlc_txq_flush_flowid_pkts(wlc_info_t *wlc, uint16 flowid)
{
	struct pktq tmp_q;
	void *pkt;
	int prec;
	wlc_txq_info_t *tx_q;
#ifdef NEW_TXQ
	txq_t* low_txq;
	struct swpktq tmp_swq;
	wlc_txh_info_t txh_info;
	struct swpktq *free_swq;
	uint swq;
	void *head_pkt;
	uint32 pkt_adj;
	uint16 time_adj;
#if defined(WLAMPDU_MAC)
	uint8 flipEpoch;
	uint8 lastEpoch;
	uint8 *tsoHdrPtr;
#endif /* WLAMPDU_MAC */
#endif /* NEW_TXQ */

	pktq_init(&tmp_q, WLC_PREC_COUNT, PKTQ_LEN_MAX);
#ifdef NEW_TXQ
	pktq_sw_init(&tmp_swq, PKTQ_MAX_SW, PKTQ_LEN_MAX);
#endif // endif

	/* De-queue the packets from all txq and free them for the flow ring pkts */
	for (tx_q = wlc->tx_queues; tx_q != NULL; tx_q = tx_q->next) {
		/* txq */
		while ((pkt = pktq_deq(&tx_q->txq, &prec))) {
			if (PKTISTXFRAG(wlc->osh, pkt) &&
				(flowid == PKTFRAGFLOWRINGID(wlc->osh, pkt))) {
				PKTFREE(wlc->pub->osh, pkt, TRUE);
				continue;
			}
			pktq_penq(&tmp_q, prec, pkt);
		}
		/* Enqueue back rest of the packets */
		while ((pkt = pktq_deq(&tmp_q, &prec))) {
			pktq_penq(&tx_q->txq, prec, pkt);
		}
#ifdef NEW_TXQ
		/* De-queue the packets from all low_txq and free them for the flow ring pkts */
		low_txq = tx_q->low_txq;
		free_swq = wlc_low_txq(low_txq);
		for (swq = 0; swq < low_txq->nswq; swq++) {
#if defined(WLAMPDU_MAC)
			flipEpoch = 0;
			lastEpoch = HEAD_PKT_FLUSHED;
			pkt = wlc_peek_lastpkt(wlc, swq);
			if (pkt) {
#ifdef HOST_HDR_FETCH
				if (__PKTISHDRINHOST(wlc->osh, pkt)) {
					lastEpoch = PKTFRAGEPOCH(wlc->osh, pkt);
				} else
#endif // endif
				{
					wlc_get_txh_info(wlc, pkt, &txh_info);
					tsoHdrPtr = txh_info.tsoHdrPtr;
					lastEpoch = (tsoHdrPtr[2] & TOE_F2_EPOCH);
				}
			}
#endif /* WLAMPDU_MAC */
			head_pkt = NULL;
			pkt_adj = 0;
			time_adj = 0;
			while (pktq_ppeek(free_swq, swq) != head_pkt) {
				pkt = pktq_swdeq(free_swq, swq);
				if (PKTISTXFRAG(wlc->osh, pkt) &&
					(flowid == PKTFRAGFLOWRINGID(wlc->osh, pkt))) {
					wlc_get_txh_info(wlc, pkt, &txh_info);
					ASSERT(swq ==
						WLC_TXFID_GET_QUEUE(ltoh16(txh_info.TxFrameID)));
					pkt_adj++;
					wlc_txq_freed_pkt_time(wlc, pkt, &time_adj);
					PKTFREE(wlc->pub->osh, pkt, TRUE);
#if defined(WLAMPDU_MAC)
					flipEpoch |= TXQ_PKT_DEL;
#endif /* WLAMPDU_MAC */
				} else {
					if (!head_pkt)
						head_pkt = pkt;
#ifdef WLAMPDU_MAC
					wlc_epoch_upd(wlc, pkt, &flipEpoch, &lastEpoch);
					/* clear pkt delete condition */
					flipEpoch &= ~TXQ_PKT_DEL;
#endif /* WLAMPDU_MAC */
					pktq_swenq(free_swq, swq, pkt);
				}
			}
			if (pkt_adj) {
				if ((tx_q != wlc->active_queue) &&
					(tx_q != wlc->txfifo_detach_transition_queue)) {
#if defined(TXQ_MUX)
					/* txpendpkt=0, no need to run full WLC_TXFIFO_COMPLETE */
					wlc_txq_complete(wlc->txqi,
						tx_q->low_txq, swq, pkt_adj, time_adj);
#else
					/* XXX:
					 * Fake TX time for low TXQ only (no MUX) implementation
					 */
					wlc_txq_complete(wlc->txqi,
						tx_q->low_txq, swq, time_adj, time_adj);
#endif /* TXQ_MUX */
				}
				else {
#if defined(TXQ_MUX)
					/* Complete the pkts/time removed in the loop above */
					WLC_TXFIFO_COMPLETE(wlc,
						swq, (uint16)pkt_adj, time_adj);
#else
					/* The time value parameter  is ignored for !TXQ_MUX */
					WLC_TXFIFO_COMPLETE(wlc, swq, time_adj, 0);
#endif /* TXQ_MUX */
				}
			}
#ifdef WLAMPDU_MAC
			if (AMPDU_AQM_ENAB(wlc->pub)) {
				wlc_tx_fifo_epoch_save(wlc, tx_q, swq);
			}
#endif /* WLAMPDU_MAC */
		}
#endif /* NEW_TXQ */
	}
}
#endif /* PROP_TXSTATUS */

#ifdef HOST_HDR_FETCH
/* Exported function called by pciedev to submit pkt to TX FIFO */
void
wlc_bmac_dma_submit(wlc_info_t *wlc, void *p,  uint queue, bool commit, bool firstentry)
{
	uint ndesc;
	struct scb *scb;
	wlc_txh_info_t txh_info;

	ASSERT(p);
	ASSERT(queue < WLC_HW_NFIFO_INUSE(wlc));

	/* Initialize */
	scb = WLPKTTAGSCBGET(p);

	/* No of TX Fifo descriptors required */
	ndesc = pktsegcnt(wlc->osh, p);
	if (wlc->dma_avoidance_war)
		ndesc *= 2;

#if defined(BCMDBG) || defined(WLTEST)
	if (queue != TX_BCMC_FIFO)
		wlc_get_txh_info(wlc, p, &txh_info);
#endif // endif

	if (queue == TX_BCMC_FIFO) {
		uint16 frameid;
#if defined(MBSS)
		wlc_bsscfg_t *bsscfg;
#endif // endif
		wlc_get_txh_info(wlc, p, &txh_info);
		frameid = ltoh16(txh_info.TxFrameID);

#if defined(MBSS)
		bsscfg = wlc->bsscfg[WLPKTTAGBSSCFGGET(p)];
		ASSERT(bsscfg != NULL);
		/* For MBSS mode, keep track of the
		 * last bcmc FID in the bsscfg info.
		 * A snapshot of the FIDs for each BSS
		 * will be committed to shared memory at DTIM.
		 */
		if (MBSS_ENAB(wlc->pub)) {
			bsscfg->bcmc_fid = frameid;
			wlc_mbss_txq_update_bcmc_counters(wlc, bsscfg, p);
		} else
#endif /* MBSS */
		{
			/*
			 * Commit BCMC sequence number in
			 * the SHM frame ID location
			*/
			wlc_bmac_write_shm(wlc->hw, M_BCMC_FID, frameid);
		}
	}

	/* XXX:TXQ WES: Hope this can be removed.
	 * Reference JIRA:SWWLAN-40902
	 */

	if (WLC_WAR16165(wlc)) {
		wlc_war16165(wlc, TRUE);
	}

#ifdef BCM_DMA_CT
	/* XXX:CRWLDOT11M-2187, 2432 WAR, avoid AQM hang when PM !=0.
	 * A 10us delay is needed for the 1st entry of posting
	 * if the override is already ON for WAR this issue and
	 * not suffer much penalty.
	 */
	if (BCM_DMA_CT_ENAB(wlc) && firstentry &&
		wlc->cfg->pm->PM != PM_OFF && (D11REV_IS(wlc->pub->corerev, 65))) {
		if (!(wlc->hw->maccontrol & MCTL_WAKE) &&
		    !(wlc->hw->wake_override & WLC_WAKE_OVERRIDE_CLKCTL)) {
		  wlc_ucode_wake_override_set(wlc->hw, WLC_WAKE_OVERRIDE_CLKCTL);
		} else {
			OSL_DELAY(10);
		}
	} else
#endif /* BCM_DMA_CT */
	{
	/*
	 * PR 113378: Checking for the PM wake override bit before calling the
	 * override.
	 * PR 107865: DRQ output should be latched before being written to DDQ.
	 */
		if (((D11REV_IS(wlc->pub->corerev, 41)) ||
		     (D11REV_IS(wlc->pub->corerev, 44))) &&
		    (!(wlc->hw->maccontrol & MCTL_WAKE) &&
		     !(wlc->hw->wake_override & WLC_WAKE_OVERRIDE_4335_PMWAR)))
			wlc_ucode_wake_override_set(wlc->hw, WLC_WAKE_OVERRIDE_4335_PMWAR);
	}

	/* Set hdr in host flag only if
	 * 1. Is a TX Frag &
	 * 2. not a multicact frame
	 */
	/* BCMC: Multicast frames retain their d11 buffer.
	 * Might have to be re enqued back into low txq
	 * So ahve all the headers locally
	 */

	/* NONTXFRAG : Non txfgrags are also pushed through this path
	 * to maintain the order of the packets. But its a dummy transaction.
	 * Hdrs are still kept in local memory
	 */
	if (!SCB_ISMULTI(scb))
		PKTSETHDRINHOST(wlc->osh, p);

#if defined(BCMDBG) || defined(WLTEST)
	/* Debug for PSMx WD for fifo number unmatch */
	ASSERT(queue == WLC_TXFID_GET_QUEUE(ltoh16(txh_info.TxFrameID)));
#endif // endif

	/* MAC fifo posting */
	if (wlc_bmac_dma_txfast(wlc, queue, p, commit) < 0) {
		WL_ERROR(("%s: desc not available : txavail %d dma pend %d \n",
			__FUNCTION__, TXAVAIL(wlc, queue), TXDMAPEND(wlc, queue)));
		/* this submission here has to succeed */
		/* No backup queues to enqueue it back */
		/* What about non assert builds  ? */
		ASSERT(0);
	}

	/* release d11 buffer now */
	if (__PKTISHDRINHOST(wlc->osh, p)) {
		void *d11buf = PKTHEAD(wlc->osh, p);
		PKTSETBUF(wlc->osh, p, ((uchar *)p + LBUFFRAGSZ),
			PKTLEN(wlc->osh, p));
		lfbufpool_free(d11buf);
	}

	/* DMA accounting */
	TXDMAPEND_DECR(wlc, queue, ndesc);
	AQMDMA_PENDDECR(wlc, queue, 1);
}
void
wlc_bmac_dma_commit(wlc_info_t *wlc, uint queue)
{
#if defined(BCM_DMA_CT) && defined(WLC_LOW)
	if (BCM_DMA_CT_ENAB(wlc)) {
		ASSERT(queue < WLC_HW_NFIFO_INUSE(wlc));
		dma_commit(WLC_HW_DI(wlc, queue));
		if (wlc->cpbusy_war) {
			wlc_bmac_sched_aqm_fifo(wlc->hw);
		} else {
			dma_commit(WLC_HW_AQM_DI(wlc, queue));
		}
	}
#endif /* BCM_DMA_CT && WLC_LOW */
}
static void
wlc_txhdr_push_prepare(wlc_info_t *wlc, void* p, wlc_txh_info_t * txh_info)
{
	uint8 *tsoHdrPtr;
	uint8 epoch;

	if (PKTISTXFRAG(wlc->osh, p)) {
		tsoHdrPtr = txh_info->tsoHdrPtr;
		epoch = (tsoHdrPtr[2] & TOE_F2_EPOCH) >> 1;
		PKTSETFRAGEPOCH(wlc->osh, p, epoch);

		PKTFRAGSETRA(wlc->osh, p, txh_info->TxFrameRA);

		PKTFRAGSETBAND(wlc->osh, p, wlc->band->bandunit);
	}
}
struct scb*
wlc_recover_pkt_scb_hdr_inhost(wlc_info_t *wlc, void *pkt)
{
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;

	/* Check for HDR really in host */
	if (!__PKTISHDRINHOST(wlc->osh, pkt)) {
		wlc_txh_info_t txh_info;
		wlc_get_txh_info(wlc, pkt, &txh_info);
		return wlc_recover_pkt_scb(wlc, pkt, &txh_info);
	}

	scb = WLPKTTAG(pkt)->_scb;

	/* check to see if the SCB is just one of the permanent hwrs_scbs */
	if (scb == wlc->band->hwrs_scb ||
	    (NBANDS(wlc) > 1 && scb == wlc->bandstate[OTHERBANDUNIT(wlc)]->hwrs_scb)) {
		WL_MQ(("MQ: %s: recovering %p hwrs_scb\n",
		       __FUNCTION__, pkt));
		return scb;
	}

	/*  use the bsscfg index in the packet tag to find out which
	 *  bsscfg this packet belongs to
	 */
	bsscfg = wlc->bsscfg[WLPKTTAGBSSCFGGET(pkt)];

#ifndef WLAWDL
	/* unicast pkts have been seen with null bsscfg here */
	ASSERT(bsscfg || !ETHER_ISMULTI(PKTFRAGRA(wlc->osh, pkt)));
#endif // endif
	if (bsscfg == NULL) {
		WL_MQ(("MQ: %s: pkt cfg idx %d no longer exists\n",
			__FUNCTION__, WLPKTTAGBSSCFGGET(pkt)));
		scb = NULL;
	} else if (ETHER_ISMULTI(PKTFRAGRA(wlc->osh, pkt))) {
		scb = WLC_BCMCSCB_GET(wlc, bsscfg);
		ASSERT(scb != NULL);
		WL_MQ(("MQ: %s: recovering %p bcmc scb\n",
			__FUNCTION__, pkt));
	} else {
		scb = wlc_scbfindband(wlc, bsscfg, (struct ether_addr*)PKTFRAGRA(wlc->osh, pkt),
		                      PKTFRAGBAND(wlc->osh, pkt));

#if defined(BCMDBG)
		if (scb != NULL) {
			char eabuf[ETHER_ADDR_STR_LEN];
			bcm_ether_ntoa(&scb->ea, eabuf);
			WL_MQ(("MQ: %s: recovering %p scb %p:%s\n", __FUNCTION__,
			       pkt, scb, eabuf));
		} else {
			WL_MQ(("MQ: %s: failed recovery scb for pkt %p\n", __FUNCTION__, pkt));
		}
#endif // endif
	}

	WLPKTTAGSCBSET(pkt, scb);

	return scb;
}
#endif /* HOST_HDR_FETCH */
void BCMFASTPATH
wlc_txq_enq_spq(wlc_info_t *wlc, struct scb *scb, struct spktq *spq, uint prec)
{
	wlc_txq_info_t *qi = SCB_WLCIFP(scb)->qi;
	struct pktq *q = WLC_GET_TXQ(qi);
	int prio;
	int datahiwat;
	void *sdu = pktqpeek(spq);

#ifdef TXQ_MUX
	WL_ERROR(("%s() scb:0x%p sdu:0x%p prec:0x%x\n",
		__FUNCTION__, OSL_OBFUSCATE_BUF(scb),
		OSL_OBFUSCATE_BUF(sdu), prec));
	{
		void *next;
		void *p = sdu;
		while (p) {
			void *tmp_pkt = p;
			next = PKTLINK(p);
			while (tmp_pkt) {
				prhex(__FUNCTION__, PKTDATA(wlc->osh, tmp_pkt),
					PKTLEN(wlc->osh, tmp_pkt));
				tmp_pkt = PKTNEXT(wlc->osh, tmp_pkt);
			}
			p = next;
		}
	}
#endif /* TXQ_MUX */
	ASSERT(!PKTISCHAINED(sdu));

	prio = PKTPRIO(sdu);
	datahiwat = (WLPKTTAG(sdu)->flags & WLF_AMPDU_MPDU)
		  ? wlc->pub->tunables->ampdudatahiwat
		  : wlc->pub->tunables->datahiwat;

	ASSERT(pktq_max(q) >= datahiwat);
#ifdef BCMDBG_POOL
	{
		void *next;
		void *p = sdu;
		while (p) {
			next = PKTLINK(p);
			if (WLPKTTAG(p)->flags & WLF_PHDR) {
				void *pdata = PKTNEXT(wlc->pub.osh, p);
				ASSERT(pdata);
				ASSERT(WLPKTTAG(pdata)->flags & WLF_DATA);
				PKTPOOLSETSTATE(pdata, POOL_TXENQ);
			} else {
				PKTPOOLSETSTATE(p, POOL_TXENQ);
			}
			p = next;
		}
	}
#endif /* BCMDBG_POOL */

	wlc_prec_enq_spq(wlc, scb, q, spq, prec);

	/* Check if flow control needs to be turned on after enqueuing the packet
	 *   Don't turn on flow control if EDCF is enabled. Driver would make the decision on what
	 *   to drop instead of relying on stack to make the right decision
	 */
	if (!EDCF_ENAB(wlc->pub) || (wlc->pub->wlfeatureflag & WL_SWFL_FLOWCONTROL)) {
		if (pktq_len(q) >= datahiwat) {
			wlc_txflowcontrol(wlc, qi, ON, ALLPRIO);
		}
	} else if (wlc->pub->_priofc) {
		if (pktq_plen(q, wlc_prio2prec_map[prio]) >= datahiwat) {
			wlc_txflowcontrol(wlc, qi, ON, prio);
		}
	}
} /* wlc_txq_enq_spq */
