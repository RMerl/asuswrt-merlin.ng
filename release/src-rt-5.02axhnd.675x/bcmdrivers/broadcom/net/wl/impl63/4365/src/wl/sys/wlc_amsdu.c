/*
 * MSDU aggregation protocol source file
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_amsdu.c 746925 2018-02-14 20:44:43Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#if !defined(WLAMSDU) && !defined(WLAMSDU_TX)
#error "Neither WLAMSDU nor WLAMSDU_TX is defined"
#endif // endif

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#ifdef BCMCCX
#include <proto/802.11_ccx.h>
#endif // endif
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_frmutil.h>
#include <wlc_pcb.h>
#include <wlc_scb_ratesel.h>
#include <wlc_rate_sel.h>
#include <wlc_amsdu.h>
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_export.h>
#endif // endif

#ifdef PKTC_TBL
#include <wl_pktc.h>
#endif // endif

#if defined(WLAMPDU) && defined(WLOVERTHRUSTER)
/* headers to enable TCP ACK bypass for Overthruster */
#include <wlc_ampdu.h>
#include <proto/ethernet.h>
#include <proto/bcmip.h>
#include <proto/bcmtcp.h>
#endif /* WLAMPDU && WLOVERTHRUSTER */
#ifdef PSTA
#include <wlc_psta.h>
#endif // endif
#ifdef WL11AC
#include <wlc_vht.h>
#endif /* WL11AC */
#include <wlc_ht.h>
#include <wlc_tx.h>
#include <wlc_rx.h>
#include <wlc_bmac.h>
/*
 * A-MSDU agg flow control
 * if txpend/scb/prio >= WLC_AMSDU_HIGH_WATERMARK, start aggregating
 * if txpend/scb/prio <= WLC_AMSDU_LOW_WATERMARK, stop aggregating
 */
#define WLC_AMSDU_LOW_WATERMARK		1
#define WLC_AMSDU_HIGH_WATERMARK	1

/* default values for tunables, iovars */

#define CCKOFDM_PLCP_MPDU_MAX_LENGTH	8096	/* max MPDU length: 13 bits len field in PLCP */
#define AMSDU_MAX_MSDU_PKTLEN		VHT_MAX_AMSDU	/* max pkt length to be aggregated */
#define AMSDU_VHT_USE_HT_AGG_LIMITS_ENAB	1	/* use ht agg limits for vht */

#define AMSDU_AGGBYTES_MIN		500	/* the lowest aggbytes allowed */
#define MAX_TX_SUBFRAMES_LIMIT		16	/* the highest aggsf allowed */

#ifdef WLC_HIGH_ONLY
#define MAX_TX_SUBFRAMES		10	/* max num of MSDUs in one tx A-MSDU */
#define	MAX_RX_SUBFRAMES		32	/* max A-MSDU rx size / smallest frame bytes */
#else
#define	MAX_RX_SUBFRAMES		100	/* max A-MSDU rx size / smallest frame bytes */
#ifndef BCMSIM
#define MAX_TX_SUBFRAMES		14	/* max num of MSDUs in one A-MSDU */
#else
#define MAX_TX_SUBFRAMES		5	/* for linuxsim testing */
#endif // endif
#endif	/* WLC_HIGH_ONLY */
#define AMSDU_RX_SUBFRAMES_BINS	5	/* number of counters for amsdu subframes */

#define MAX_TX_SUBFRAMES_ACPHY	2	/* max num of MSDUs in one A-MSDU */

/* statistics */
#ifdef WL11AC
#define AMSDU_LENGTH_BINS		12	/* number of length bins in length histogram */
#else
#define AMSDU_LENGTH_BINS		8	/* number of length bins in length histogram */
#endif // endif
#define AMSDU_LENGTH_BIN_BYTES		1024	/* number of bytes in length represented by each
						* bin in length histogram
						*/
/* sw rx private states */
#define WLC_AMSDU_DEAGG_IDLE 		0	/* idle */
#define WLC_AMSDU_DEAGG_FIRST		1	/* deagg first frame received */
#define WLC_AMSDU_DEAGG_LAST		3	/* deagg last frame received */

#ifdef WLCNT
#define	WLC_AMSDU_CNT_VERSION	2	/* current version of wlc_amsdu_cnt_t */

/* block ack related stats */
typedef struct wlc_amsdu_cnt {
	uint16	version;	/* WLC_AMSDU_CNT_VERSION */
	uint16	length;		/* length of entire structure */

	uint32	agg_openfail;		/* num of MSDU open failure */
	uint32	agg_passthrough;	/* num of MSDU pass through w/o A-MSDU agg */
	uint32	agg_block;		/* num of MSDU blocked in A-MSDU agg */
	uint32	agg_amsdu;		/* num of A-MSDU released */
	uint32	agg_msdu;		/* num of MSDU aggregated in A-MSDU */
	uint32	agg_stop_tailroom;	/* num of MSDU aggs stopped for lack of tailroom */
	uint32	agg_stop_sf;		/* num of MSDU aggs stopped for sub-frame count limit */
	uint32	agg_stop_len;		/* num of MSDU aggs stopped for byte length limit */
	uint32	agg_stop_lowwm;		/* num of MSDU aggs stopped for tx low water mark */
	uint32	agg_stop_passthrough;	/* num of MSDU aggs stopped for un-aggregated frame */
	uint32	deagg_msdu;		/* MSDU of deagged A-MSDU(in ucode) */
	uint32	deagg_amsdu;		/* valid A-MSDU deagged(exclude bad A-MSDU) */
	uint32	deagg_badfmt;		/* MPDU is bad */
	uint32	deagg_wrongseq;		/* MPDU of one A-MSDU doesn't follow sequence */
	uint32	deagg_badsflen;		/* MPDU of one A-MSDU has length mismatch */
	uint32	deagg_badsfalign;	/* MPDU of one A-MSDU is not aligned to 4 bytes boundary */
	uint32  deagg_badtotlen;	/* A-MSDU tot length doesn't match summation of all sfs */
	uint32	deagg_openfail;		/* A-MSDU deagg open failures */
	uint32	deagg_swdeagglong;	/* A-MSDU sw_deagg doesn't handle long pkt */
	uint32	deagg_flush;		/* A-MSDU deagg flush; deagg errors may result in this */
	uint32	tx_pkt_free_ignored; /* tx pkt free event ignored due to invalid scb or !amsdutx */
	uint32	tx_padding_in_tail;	/* 4Byte pad was placed in tail of packet */
	uint32	tx_padding_in_head;	/* 4Byte pad was placed in head of packet */
	uint32	tx_padding_no_pad;	/* 4Byte pad was not needed (4B aligned or last in agg) */
	uint32	agg_amsdu_bytes_l;	/* num of total msdu bytes successfully transmitted */
	uint32	agg_amsdu_bytes_h;
	uint32	deagg_amsdu_bytes_l;	/* AMSDU bytes deagg successfully */
	uint32	deagg_amsdu_bytes_h;
} wlc_amsdu_cnt_t;
#endif	/* WLCNT */

typedef struct {
	/* tx counters */
	uint32 tx_msdu_histogram[MAX_TX_SUBFRAMES_LIMIT]; /* mpdus per amsdu histogram */
	uint32 tx_length_histogram[AMSDU_LENGTH_BINS]; /* amsdu length histogram */
	/* rx counters */
	uint32 rx_msdu_histogram[AMSDU_RX_SUBFRAMES_BINS]; /* mpdu per amsdu rx */
	uint32 rx_length_histogram[AMSDU_LENGTH_BINS]; /* amsdu rx length */
} amsdu_dbg_t;

/* iovar table */
enum {
	IOV_AMSDU_SIM,
	IOV_AMSDU_HIWM,
	IOV_AMSDU_LOWM,
	IOV_AMSDU_AGGSF,	/* num of subframes in one A-MSDU */
	IOV_AMSDU_AGGBYTES,	/* num of bytes in one A-MSDU */
	IOV_AMSDU_RXMAX,	/* get/set HT_CAP_MAX_AMSDU in HT cap field */
	IOV_AMSDU_BLOCK,	/* block amsdu agg */
	IOV_AMSDU_FLUSH,	/* flush all amsdu agg queues */
	IOV_AMSDU_DEAGGDUMP,	/* dump deagg pkt */
	IOV_AMSDU_COUNTERS,
	IOV_AMSDU_CLEAR_COUNTERS,
	IOV_AMSDUNOACK,
	IOV_AMSDU,
	IOV_RX_AMSDU_IN_AMPDU
};

static const bcm_iovar_t amsdu_iovars[] = {
	{"amsdu", IOV_AMSDU, (IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_BOOL, 0},
	{"rx_amsdu_in_ampdu", IOV_RX_AMSDU_IN_AMPDU, (IOVF_RSDB_SET), IOVT_BOOL, 0},
	{"amsdu_noack", IOV_AMSDUNOACK, (IOVF_RSDB_SET), IOVT_BOOL, 0},
	{"amsdu_aggsf", IOV_AMSDU_AGGSF, (IOVF_RSDB_SET), IOVT_UINT16, 0},
	{"amsdu_aggbytes", IOV_AMSDU_AGGBYTES, (0), IOVT_UINT32, 0},
#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	{"amsdu_aggblock", IOV_AMSDU_BLOCK, (0), IOVT_BOOL, 0},
#ifdef WLCNT
	{"amsdu_counters", IOV_AMSDU_COUNTERS, (0), IOVT_BUFFER, sizeof(wlc_amsdu_cnt_t)},
	{"amsdu_clear_counters", IOV_AMSDU_CLEAR_COUNTERS, 0, IOVT_VOID, 0},
#endif /* WLCNT */
#endif /* BCMDBG */
	{NULL, 0, 0, 0, 0}
};

typedef struct amsdu_deagg {
	int	amsdu_deagg_state;		/* A-MSDU deagg statemachine per device */
	void	*amsdu_deagg_p;			/* pointer to first pkt buffer in A-MSDU chain */
	void	*amsdu_deagg_ptail;		/* pointer to last pkt buffer in A-MSDU chain */
	uint16	first_pad;			/* front padding bytes of A-MSDU first sub frame */
} amsdu_deagg_t;

/* principle amsdu module local structure per device instance */
struct amsdu_info {
	wlc_info_t	*wlc;			/* pointer to main wlc structure */
	wlc_pub_t	*pub;			/* public common code handler */
	int	scb_handle;			/* scb cubby handle to retrieve data from scb */

	uint16	mac_rcvfifo_limit;		/* max rx fifo in bytes */
	uint16	amsdu_rx_mtu;			/* amsdu MTU, depend on rx fifo limit */
	bool	amsdu_rxcap_big;		/* TRUE: rx big amsdu capable (HT_MAX_AMSDU) */

	uint16	fifo_lowm;			/* low watermark for tx queue precendence */
	uint16	fifo_hiwm;			/* high watermark for tx queue precendence */

	bool	amsdu_agg_block;		/* global override: disable amsdu tx */

	bool	amsdu_agg_allowprio[NUMPRIO];	/* A-MSDU agg. TRUE: allowed, FALSE: disalowed */
	uint	amsdu_agg_bytes_limit[NUMPRIO];	/* max AMSDU bytes per priority */
	uint	amsdu_agg_sframes_limit[NUMPRIO];	/* max number of subframes in one A-MSDU */

	/* rx: streams per device */
	amsdu_deagg_t *amsdu_deagg;			/* A-MSDU deagg */

#ifdef WLCNT
	wlc_amsdu_cnt_t *cnt;			/* counters/stats */
#endif /* WLCNT */
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMSDU)
	amsdu_dbg_t *amdbg;
#endif // endif
};

typedef struct {
	uint	amsdu_ht_agg_bytes_max;	/* max ht AMSDU bytes negotiated */
	uint	amsdu_vht_agg_bytes_max; /* max vht AMSDU bytes negotiated */
	uint	amsdu_agg_bytes;	/* A-MSDU byte count */
	uint	amsdu_agg_sframes;	/* A-MSDU subframe count */
	void	*amsdu_agg_p;		/* A-MSDU pkt pointer to first MSDU */
	void	*amsdu_agg_ptail;	/* A-MSDU pkt pointer to last MSDU */
	uint	amsdu_agg_padlast;	/* pad bytes in the agg tail buffer */
	uint	amsdu_agg_txpending;
	bool	amsdu_agg_allowtid;	/* TRUE: agg is allowed, FALSE: agg is disallowed */
	uint8	headroom_pad_need;	/* # of bytes (0-3) need fr headrroom for pad prev pkt */
} amsdu_scb_prio;

/* per scb cubby info */
typedef struct scb_amsduinfo {
	amsdu_scb_prio prio[NUMPRIO];
} scb_amsdu_t;

#define SCB_AMSDU_CUBBY(ami, scb) (scb_amsdu_t *)SCB_CUBBY((scb), (ami)->scb_handle)

/* A-MSDU general */
static int wlc_amsdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *arg, int alen, int val_size, struct wlc_if *wlcif);

static void wlc_amsdu_mtu_init(amsdu_info_t *ami);
static int wlc_amsdu_down(void *hdl);
static int wlc_amsdu_up(void *hdl);

#ifdef WLAMSDU_TX
#ifndef WLAMSDU_TX_DISABLED
static int wlc_amsdu_scb_init(void *cubby, struct scb *scb);
static void wlc_amsdu_scb_deinit(void *cubby, struct scb *scb);
#endif // endif
/* A-MSDU aggregation */
static void  wlc_amsdu_agg(void *ctx, struct scb *scb, void *p, uint prec);
static void* wlc_amsdu_agg_open(amsdu_info_t *ami, wlc_bsscfg_t *bsscfg,
	struct ether_addr *ea, void *p);
static bool  wlc_amsdu_agg_append(amsdu_info_t *ami, struct scb *scb, void *p,
	uint tid, ratespec_t rspec);
static void  wlc_amsdu_agg_close(amsdu_info_t *ami, struct scb *scb, uint tid);

#ifdef WLOVERTHRUSTER
static bool wlc_amsdu_is_tcp_ack(amsdu_info_t *ami, void *p);
#endif // endif
static void  wlc_amsdu_scb_deactive(void *ctx, struct scb *scb);

static uint  wlc_amsdu_txpktcnt(void *ctx);

#if defined(WLAMSDU_TX) && !defined(WLAMSDU_TX_DISABLED) && (defined(BCMDBG) || \
	defined(BCMDBG_DUMP) || defined(BCMDBG_AMSDU))
static void wlc_amsdu_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#endif // endif

static txmod_fns_t BCMATTACHDATA(amsdu_txmod_fns) = {
	wlc_amsdu_agg,
	wlc_amsdu_txpktcnt,
	wlc_amsdu_scb_deactive,
	NULL
};
#endif /* WLAMSDU_TX */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMSDU)
static int wlc_amsdu_dump(amsdu_info_t *ami, struct bcmstrbuf *b);
#endif // endif

/* A-MSDU deaggregation */
static bool wlc_amsdu_deagg_open(amsdu_info_t *ami, int fifo, void *p,
	struct dot11_header *h, uint32 pktlen);
static bool wlc_amsdu_deagg_verify(amsdu_info_t *ami, uint16 fc, void *h);
static void wlc_amsdu_deagg_flush(amsdu_info_t *ami, int fifo);
static int wlc_amsdu_tx_attach(amsdu_info_t *ami, wlc_info_t *wlc);

#if (defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMSDU)) && \
	defined(WLCNT)
void wlc_amsdu_dump_cnt(amsdu_info_t *ami, struct bcmstrbuf *b);
#endif	/* defined(BCMDBG) && defined(WLCNT) */

#ifdef WL11K_ALL_MEAS
void wlc_amsdu_get_stats(wlc_info_t *wlc, rrm_stat_group_11_t *g11)
{
	ASSERT(wlc);
	ASSERT(g11);

	g11->txamsdu = wlc->ami->cnt->agg_amsdu;
	g11->amsdufail = wlc->ami->cnt->agg_openfail + wlc->ami->cnt->deagg_openfail;
	g11->amsduretry = 0; /* Not supported */
	g11->amsduretries = 0; /* Not supported */
	g11->txamsdubyte_h = wlc->ami->cnt->agg_amsdu_bytes_h;
	g11->txamsdubyte_l = wlc->ami->cnt->agg_amsdu_bytes_l;
	g11->amsduackfail = wlc->_amsdu_noack;
	g11->rxamsdu = wlc->ami->cnt->deagg_amsdu;
	g11->rxamsdubyte_h = wlc->ami->cnt->deagg_amsdu_bytes_h;
	g11->rxamsdubyte_l = wlc->ami->cnt->deagg_amsdu_bytes_l;
}
#endif /* WL11K_ALL_MEAS */

#ifdef WLAMSDU_TX
static void wlc_amsdu_dotxstatus(amsdu_info_t *ami, struct scb *scb, void *pkt);

/* handle callbacks when pkts either tx-ed or freed */
static void BCMFASTPATH
wlc_amsdu_pkt_freed(wlc_info_t *wlc, void *pkt, uint txs)
{
	if (AMSDU_TX_ENAB(wlc->pub)) {
		int err;
		struct scb *scb = NULL;
		wlc_bsscfg_t *bsscfg = wlc_bsscfg_find(wlc, WLPKTTAGBSSCFGGET(pkt), &err);
		/* if bsscfg or scb are stale or bad, then ignore this pkt for acctg purposes */
		if (!err && bsscfg) {
			scb = WLPKTTAGSCBGET(pkt);
			if (scb && SCB_AMSDU(scb)) {
				struct scb *newscb = (scb->bandunit < MAXBANDS)?
					wlc_scbfindband(wlc, bsscfg, &scb->ea, scb->bandunit): NULL;
				if (newscb == scb) {
					wlc_amsdu_dotxstatus(wlc->ami, scb, pkt);
				} else {
					WL_AMSDU(("wl%d:%s: not count scb(%p) pkts\n",
						wlc->pub->unit, __FUNCTION__, scb));
#ifdef WLCNT
					WLCNTINCR(wlc->ami->cnt->tx_pkt_free_ignored);
#endif /* WLCNT */
				}
			} else {
				WL_AMSDU(("wl%d:%s: not count scb(%p) pkts\n",
					wlc->pub->unit, __FUNCTION__, scb));
#ifdef WLCNT
				WLCNTINCR(wlc->ami->cnt->tx_pkt_free_ignored);
#endif /* WLCNT */
			}
		} else {
			WL_AMSDU(("wl%d:%s: not count bsscfg (%p) pkts\n",
				wlc->pub->unit, __FUNCTION__, bsscfg));
#ifdef WLCNT
			WLCNTINCR(wlc->ami->cnt->tx_pkt_free_ignored);
#endif /* WLCNT */
		}
	}
	/* callback is cleared by default by calling function */
}
#endif /* WLAMSDU_TX */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

amsdu_info_t *
BCMATTACHFN(wlc_amsdu_attach)(wlc_info_t *wlc)
{
	amsdu_info_t *ami;

	if (!(ami = (amsdu_info_t *)MALLOCZ(wlc->osh, sizeof(amsdu_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	ami->wlc = wlc;
	ami->pub = wlc->pub;

	if (!(ami->amsdu_deagg = (amsdu_deagg_t *)MALLOCZ(wlc->osh,
		sizeof(amsdu_deagg_t) * RX_FIFO_NUMBER))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

#ifdef WLCNT
	if (!(ami->cnt = (wlc_amsdu_cnt_t *)MALLOCZ(wlc->osh, sizeof(wlc_amsdu_cnt_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif /* WLCNT */

	/* register module */
	if (wlc_module_register(ami->pub, amsdu_iovars, "amsdu", ami, wlc_amsdu_doiovar,
		NULL, wlc_amsdu_up, wlc_amsdu_down)) {
		WL_ERROR(("wl%d: %s: wlc_module_register failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMSDU)
	if (!(ami->amdbg = (amsdu_dbg_t *)MALLOCZ(wlc->osh, sizeof(amsdu_dbg_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	wlc_dump_register(ami->pub, "amsdu", (dump_fn_t)wlc_amsdu_dump, (void *)ami);
#endif // endif

	ami->fifo_lowm = (uint16)WLC_AMSDU_LOW_WATERMARK;
	ami->fifo_hiwm = (uint16)WLC_AMSDU_HIGH_WATERMARK;

	if (wlc_amsdu_tx_attach(ami, wlc) < 0) {
		WL_ERROR(("wl%d: %s: Error initing the amsdu tx\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	wlc_amsdu_mtu_init(ami);

	/* to be compatible with spec limit */
	if (wlc->pub->tunables->nrxd < MAX_RX_SUBFRAMES) {
		WL_ERROR(("NRXD %d is too small to fit max amsdu rxframe\n",
		          (uint)wlc->pub->tunables->nrxd));
	}
	return ami;

fail:
	wlc_amsdu_detach(ami);
	return NULL;
}

static int
BCMATTACHFN(wlc_amsdu_tx_attach)(amsdu_info_t *ami, wlc_info_t *wlc)
{
#ifdef WLAMSDU_TX
#ifndef WLAMSDU_TX_DISABLED
	uint i;
	uint max_agg;
	int err;
	if (WLCISACPHY(wlc->band)) {
#if AMSDU_VHT_USE_HT_AGG_LIMITS_ENAB
		max_agg = HT_MIN_AMSDU;
#else
		max_agg = VHT_MAX_AMSDU;
#endif /* AMSDU_VHT_USE_HT_AGG_LIMITS_ENAB */
	} else {
		max_agg = HT_MAX_AMSDU;
	}

	/* register packet class callback */
	err = wlc_pcb_fn_set(wlc->pcb, 2, WLF2_PCB3_AMSDU, wlc_amsdu_pkt_freed);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_pcb_fn_set err=%d\n", wlc->pub->unit, __FUNCTION__, err));
		return -1;
	}

	/* reserve cubby in the scb container for per-scb private data */
	ami->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(scb_amsdu_t),
		wlc_amsdu_scb_init, wlc_amsdu_scb_deinit,
#if defined(WLAMSDU_TX) && (defined(BCMDBG) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMSDU))
		wlc_amsdu_dump_scb,
#else
		NULL,
#endif // endif
		(void *)ami);

	if (ami->scb_handle < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return -1;
	}

	/* register txmod call back */
	wlc_txmod_fn_register(wlc, TXMOD_AMSDU, ami, amsdu_txmod_fns);

	WLCNTSET(ami->cnt->version, WLC_AMSDU_CNT_VERSION);
	WLCNTSET(ami->cnt->length, sizeof(ami->cnt));

	/* init tunables */
	for (i = 0; i < NUMPRIO; i++) {
		uint fifo_size;

		ami->amsdu_agg_allowprio[i] = TRUE;

		/* set agg_bytes_limit to standard maximum if hw fifo allows
		 *  this value can be changed via iovar or fragthreshold later
		 *  it can never exceed hw fifo limit since A-MSDU is not streaming
		 */
		fifo_size = wlc->xmtfifo_szh[prio2fifo[i]];
		fifo_size = fifo_size * TXFIFO_SIZE_UNIT;	/* blocks to bytes */

		ami->amsdu_agg_bytes_limit[i] = MIN(max_agg, fifo_size);

		if (WLCISACPHY(wlc->band)) {
			ami->amsdu_agg_sframes_limit[i] = MAX_TX_SUBFRAMES_ACPHY;
		} else {
			ami->amsdu_agg_sframes_limit[i] = MAX_TX_SUBFRAMES;
		}

		/* DMA: leave empty room for DMA descriptor table */
		if (ami->amsdu_agg_sframes_limit[i] > (uint)(wlc->pub->tunables->ntxd/3)) {
			WL_ERROR(("NTXD %d is too small to fit max amsdu txframe\n",
			          (uint)wlc->pub->tunables->ntxd));
			ASSERT(0);
		}

		/* TODO: PIO */
	}

	wlc->pub->_amsdu_tx_support = TRUE;
#else
	BCM_REFERENCE(amsdu_txmod_fns);
#endif /* WLAMSDU_TX_DISABLED */
#endif /* WLAMSDU_TX */

	return 0;
}

void
BCMATTACHFN(wlc_amsdu_detach)(amsdu_info_t *ami)
{
	if (!ami)
		return;

	wlc_amsdu_down(ami);

	wlc_module_unregister(ami->pub, "amsdu", ami);

#ifdef BCMDBG
	if (ami->amdbg) {
		MFREE(ami->pub->osh, ami->amdbg, sizeof(amsdu_dbg_t));
		ami->amdbg = NULL;
	}
#endif // endif

#ifdef WLCNT
	if (ami->cnt)
		MFREE(ami->pub->osh, ami->cnt, sizeof(wlc_amsdu_cnt_t));
#endif /* WLCNT */
	MFREE(ami->pub->osh, ami->amsdu_deagg, sizeof(amsdu_deagg_t) * RX_FIFO_NUMBER);
	MFREE(ami->pub->osh, ami, sizeof(amsdu_info_t));
}

static void
BCMATTACHFN(wlc_amsdu_mtu_init)(amsdu_info_t *ami)
{
#ifdef DONGLEBUILD
	ami->amsdu_rxcap_big = FALSE;
#else /* DONGLEBUILD */
	ami->mac_rcvfifo_limit = wlc_rcvfifo_limit_get(ami->wlc);
	if (D11REV_GE(ami->wlc->pub->corerev, 31) && D11REV_LE(ami->wlc->pub->corerev, 38))
		ami->amsdu_rxcap_big = FALSE;
	else
		ami->amsdu_rxcap_big =
			((ami->mac_rcvfifo_limit - ami->wlc->hwrxoff - 100) >= HT_MAX_AMSDU);
#endif /* DONGLEBUILD */

	ami->amsdu_rx_mtu = ami->amsdu_rxcap_big ? HT_MAX_AMSDU : HT_MIN_AMSDU;

	/* For A/C enabled chips only */
	if (WLCISACPHY(ami->wlc->band) &&
	    ami->amsdu_rxcap_big &&
	    ((ami->mac_rcvfifo_limit - ami->wlc->hwrxoff - 100) >= VHT_MAX_AMSDU)) {
		ami->amsdu_rx_mtu = VHT_MAX_AMSDU;
	}
	WL_AMSDU(("%s:ami->amsdu_rx_mtu=%d\n", __FUNCTION__, ami->amsdu_rx_mtu));
}

bool
wlc_amsdu_is_rxmax_valid(amsdu_info_t *ami)
{
	if (wlc_amsdu_mtu_get(ami) < HT_MAX_AMSDU) {
		return TRUE;
	} else {
		return FALSE;
	}
}

uint16
wlc_amsdu_mtu_get(amsdu_info_t *ami)
{
	return ami->amsdu_rx_mtu;
}

/* AMSDU tx is optional, sw can turn it on or off even HW supports */
bool
wlc_amsdutx_cap(amsdu_info_t *ami)
{
#ifdef WLC_HIGH_ONLY	/* complicate to support */
	return FALSE;
#else

#if defined(WL11N) && defined(WLAMSDU_TX)
	if (AMSDU_TX_SUPPORT(ami->pub) && ami->pub->phy_11ncapable)
		return (TRUE);
#endif // endif
	return (FALSE);
#endif /* WLC_HIGH_ONLY */
}

/* AMSDU rx is mandatory for NPHY */
bool
wlc_amsdurx_cap(amsdu_info_t *ami)
{
#ifdef WL11N
	if (ami->pub->phy_11ncapable)
		return (TRUE);
#endif // endif

	return (FALSE);
}

#ifdef WLAMSDU_TX
int
wlc_amsdu_set(amsdu_info_t *ami, bool on)
{
	wlc_info_t *wlc = ami->wlc;

	WL_AMSDU(("wlc_amsdu_set val=%d\n", on));

	if (on) {
		if (!N_ENAB(wlc->pub)) {
			WL_AMSDU(("wl%d: driver not nmode enabled\n", wlc->pub->unit));
			return BCME_UNSUPPORTED;
		}
		if (!wlc_amsdutx_cap(ami)) {
			WL_AMSDU(("wl%d: device not amsdu capable\n", wlc->pub->unit));
			return BCME_UNSUPPORTED;
		} else if (AMPDU_ENAB(wlc->pub) &&
		           D11REV_LT(wlc->pub->corerev, 40)) {
			/* AMSDU + AMPDU ok for core-rev 40+ with AQM */
			WL_AMSDU(("wl%d: A-MSDU not supported with AMPDU on d11 rev %d\n",
			          wlc->pub->unit, wlc->pub->corerev));
			return BCME_UNSUPPORTED;
		}
	}

	/* This controls AMSDU agg only, AMSDU deagg is on by default per spec */
	wlc->pub->_amsdu_tx = on;
#ifdef DISABLE_AMSDUTX_FOR_VI
	/* Disabling amsdu for VI access category to avoid packet loss */
	ami->amsdu_agg_allowprio[PRIO_8021D_CL] = FALSE;
	ami->amsdu_agg_allowprio[PRIO_8021D_VI] = FALSE;
#endif // endif
	wlc_update_brcm_ie(ami->wlc);

	/* tx descriptors should be higher -- AMPDU max when both AMSDU and AMPDU set */
	wlc_set_default_txmaxpkts(wlc);

	if (!wlc->pub->_amsdu_tx)
		wlc_amsdu_agg_flush(ami);

	return (0);
}

#ifndef WLAMSDU_TX_DISABLED
static int
wlc_amsdu_scb_init(void *context, struct scb *scb)
{
	uint i;
	amsdu_info_t *ami = (amsdu_info_t *)context;
	scb_amsdu_t *scb_amsdu = SCB_AMSDU_CUBBY(ami, scb);
	amsdu_scb_prio *amsduprio;

	WL_AMSDU(("wlc_amsdu_scb_init scb %p\n", scb));

	ASSERT(scb_amsdu);
	for (i = 0; i < NUMPRIO; i++) {
		amsduprio = &scb_amsdu->prio[i];
		amsduprio->amsdu_agg_p = NULL;
		amsduprio->amsdu_agg_ptail = NULL;
		amsduprio->amsdu_agg_sframes = 0;
		amsduprio->amsdu_agg_bytes = 0;
		amsduprio->amsdu_vht_agg_bytes_max = AMSDU_MAX_MSDU_PKTLEN;
		amsduprio->amsdu_ht_agg_bytes_max = HT_MAX_AMSDU;
		amsduprio->amsdu_agg_padlast = 0;
		amsduprio->amsdu_agg_txpending = 0;
		amsduprio->amsdu_agg_allowtid = TRUE;
		amsduprio->headroom_pad_need = 0;
	}
	return 0;
}

static void
wlc_amsdu_scb_deinit(void *context, struct scb *scb)
{
	uint i;
	amsdu_info_t *ami = (amsdu_info_t *)context;
	scb_amsdu_t *scb_amsdu = SCB_AMSDU_CUBBY(ami, scb);

	WL_AMSDU(("wlc_amsdu_scb_deinit scb %p\n", scb));

	ASSERT(scb_amsdu);

	/* release tx agg pkts */
	for (i = 0; i < NUMPRIO; i++) {
		if (scb_amsdu->prio[i].amsdu_agg_p) {
			PKTFREE(ami->wlc->osh, scb_amsdu->prio[i].amsdu_agg_p, TRUE);
			/* needs clearing, so subsequent access to this cubby doesn't ASSERT */
			/* and/or access bad memory */
			scb_amsdu->prio[i].amsdu_agg_p = NULL;
		}
	}
}
#endif /* !WLAMSDU_TX_DISABLED */
#endif /* WLAMSDU_TX */

/* handle AMSDU related items when going down */
static int
wlc_amsdu_down(void *hdl)
{
	int fifo;
	amsdu_info_t *ami = (amsdu_info_t *)hdl;

	WL_AMSDU(("wlc_amsdu_down: entered\n"));

	/* Flush the deagg Q, there may be packets there */
	for (fifo = 0; fifo < RX_FIFO_NUMBER; fifo++)
		wlc_amsdu_deagg_flush(ami, fifo);

	return 0;
}

static int
wlc_amsdu_up(void *hdl)
{
	/* limit max size pkt ucode lets through to what we use for dma rx descriptors */
	/* else rx of amsdu can cause dma rx errors and potentially impact performance */
	wlc_info_t *wlc = ((amsdu_info_t *)hdl)->wlc;
	hnddma_t *di;
	uint16 rxbufsz;
	uint16 rxoffset;

	if (!PIO_ENAB(wlc->pub)) {
		di = WLC_HW_DI(wlc, 0);
		dma_rxparam_get(di, &rxoffset, &rxbufsz);
		rxbufsz =  rxbufsz - rxoffset;
	}
	else {
		rxbufsz = wlc->pub->tunables->rxbufsz - wlc->hwrxoff;
	}

	/* ensure tunable is a valid value which fits in a uint16 */
	ASSERT(rxbufsz > 0 && rxbufsz <= 0xffff);

	wlc_write_shm(wlc, M_MAXRXFRM_LEN, (uint16)rxbufsz);
	if (D11REV_GE(wlc->pub->corerev, 64)) {
		W_REG(wlc->osh, &wlc->regs->u.d11acregs.DAGG_LEN_THR, (uint16)rxbufsz);
	}

	return BCME_OK;
}

/* handle AMSDU related iovars */
static int
wlc_amsdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif)
{
	amsdu_info_t *ami = (amsdu_info_t *)hdl;
	int32 int_val = 0;
	bool bool_val;
	int err = 0;
	wlc_info_t *wlc;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	wlc = ami->wlc;
	ASSERT(ami == wlc->ami);

	switch (actionid) {
	case IOV_GVAL(IOV_AMSDU):
		int_val = wlc->pub->_amsdu_tx;
		bcopy(&int_val, a, val_size);
		break;

#ifdef WLAMSDU_TX
	case IOV_SVAL(IOV_AMSDU):
		if (AMSDU_TX_SUPPORT(wlc->pub))
			err = wlc_amsdu_set(ami, bool_val);
		else
			err = BCME_UNSUPPORTED;
		break;
#endif // endif

	case IOV_GVAL(IOV_AMSDUNOACK):
		int_val = wlc->_amsdu_noack;
		bcopy(&int_val, a, val_size);
		break;

	case IOV_SVAL(IOV_AMSDUNOACK):
		wlc->_amsdu_noack = bool_val;
		break;

#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	case IOV_GVAL(IOV_AMSDU_BLOCK):
		int_val = ami->amsdu_agg_block;
		bcopy(&int_val, a, val_size);
		break;

	case IOV_SVAL(IOV_AMSDU_BLOCK):
		ami->amsdu_agg_block = bool_val;
		break;

#ifdef WLCNT
	case IOV_GVAL(IOV_AMSDU_COUNTERS):
		bcopy(&ami->cnt, a, sizeof(ami->cnt));
		break;

	case IOV_SVAL(IOV_AMSDU_CLEAR_COUNTERS):
		bzero(ami->cnt, sizeof(*ami->cnt));
		bzero(ami->amdbg, sizeof(*ami->amdbg));
		break;
#endif /* WLCNT */
#endif /* BCMDBG */

#ifdef WLAMSDU_TX
	case IOV_GVAL(IOV_AMSDU_AGGBYTES):
		if (AMSDU_TX_SUPPORT(wlc->pub)) {
			/* TODO, support all priorities ? */
			int_val = ami->amsdu_agg_bytes_limit[PRIO_8021D_BE];
			bcopy(&int_val, a, val_size);
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_AMSDU_AGGBYTES):
		if (AMSDU_TX_SUPPORT(wlc->pub)) {
			struct scb *scb;
			struct scb_iter scbiter;
			uint8 i;
			uint32 uint_val = (uint)int_val;

			if (WLCISACPHY(wlc->band) && uint_val > VHT_MAX_AMSDU) {
				err = BCME_RANGE;
				break;
			}
			if (!WLCISACPHY(wlc->band) && (uint_val > (uint)HT_MAX_AMSDU)) {
				err = BCME_RANGE;
				break;
			}

			if (uint_val < AMSDU_AGGBYTES_MIN) {
				err = BCME_RANGE;
				break;
			}

			/* if smaller, flush existing aggregation, care only BE for now */
			if (uint_val < ami->amsdu_agg_bytes_limit[PRIO_8021D_BE])
				wlc_amsdu_agg_flush(ami);

			for (i = 0; i < NUMPRIO; i++) {
				uint fifo_size;
				fifo_size = wlc->xmtfifo_szh[prio2fifo[i]];
				fifo_size = fifo_size * TXFIFO_SIZE_UNIT;    /* blocks to bytes */
				ami->amsdu_agg_bytes_limit[i] = MIN(uint_val, fifo_size);
			}

			/* update amsdu agg bytes for ALL scbs */
			FOREACHSCB(wlc->scbstate, &scbiter, scb)
				wlc_amsdu_scb_agglimit_upd(ami, scb);
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_AMSDU_AGGSF):
		if (AMSDU_TX_SUPPORT(wlc->pub)) {
			/* TODO, support all priorities ? */
			*(uint*)a = ami->amsdu_agg_sframes_limit[PRIO_8021D_BE];
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_AMSDU_AGGSF):
#if defined(PROP_TXSTATUS) && defined(BCMPCIEDEV)
		/* XXX: AMSDU suppression at pciedev layer
		 * XXX: cannot handle > 2 at the moment.
		 * XXX: TBD: Remove this once it is fixed
		 */
		if (BCMPCIEDEV_ENAB() && PROP_TXSTATUS_ENAB(wlc->pub) &&
			WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl)) &&
			(int_val > 2)) {
			err = BCME_UNSUPPORTED;
		} else
#endif // endif
		if (AMSDU_TX_SUPPORT(wlc->pub)) {
			uint8 i;

			if ((int_val > MAX_TX_SUBFRAMES_LIMIT) ||
			    (int_val > wlc->pub->tunables->ntxd/2) ||
			    (int_val < 1)) {
				err = BCME_RANGE;
				break;
			}

			for (i = 0; i < NUMPRIO; i++)
				ami->amsdu_agg_sframes_limit[i] = int_val;
		} else
			err = BCME_UNSUPPORTED;
		break;
#endif /* WLAMSDU_TX */

#ifdef WLAMSDU
	case IOV_GVAL(IOV_RX_AMSDU_IN_AMPDU):
		int_val = (int8)(wlc->_rx_amsdu_in_ampdu);
		bcopy(&int_val, a, val_size);
		break;

	case IOV_SVAL(IOV_RX_AMSDU_IN_AMPDU):
		if (bool_val && D11REV_LT(wlc->pub->corerev, 40)) {
			WL_AMSDU(("wl%d: Not supported < corerev (40)\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
		} else {
			wlc->_rx_amsdu_in_ampdu = bool_val;
		}

		break;
#endif /* WLAMSDU */

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

#ifdef WLAMSDU_TX
/*
 * called from fragthresh changes ONLY: update agg bytes limit, toss buffered A-MSDU
 * This is expected to happen very rarely since user should use very standard 802.11 fragthreshold
 *  to "disabled" fragmentation when enable A-MSDU. We can even ignore that. But to be
 *  full spec compliant, we reserve this capability.
 *   ??? how to inform user the requirement that not changing FRAGTHRESHOLD to screw up A-MSDU
 */
void
wlc_amsdu_agglimit_frag_upd(amsdu_info_t *ami)
{
	uint i;
	wlc_info_t *wlc = ami->wlc;
	struct scb *scb;
	struct scb_iter scbiter;
	bool flush = FALSE;
	bool frag_disabled = FALSE;
	WL_AMSDU(("wlc_amsdu_agg_limit_upd\n"));

	if (!AMSDU_TX_SUPPORT(wlc->pub))
		return;

	if (!(WLC_PHY_11N_CAP(wlc->band)))
		return;

	for (i = 0; i < NUMPRIO; i++) {
		frag_disabled = (wlc->fragthresh[WME_PRIO2AC(i)] == DOT11_MAX_FRAG_LEN);

		if (!frag_disabled &&
			wlc->fragthresh[WME_PRIO2AC(i)] < ami->amsdu_agg_bytes_limit[i]) {
			flush = TRUE;
			ami->amsdu_agg_bytes_limit[i] = wlc->fragthresh[WME_PRIO2AC(i)];
			WL_INFORM(("wlc_amsdu_agg_frag_upd: amsdu_aggbytes[%d] = %d due to frag!\n",
				i, ami->amsdu_agg_bytes_limit[i]));
		} else if (frag_disabled ||
			wlc->fragthresh[WME_PRIO2AC(i)] > ami->amsdu_agg_bytes_limit[i]) {
			uint max_agg;
			uint fifo_size;
			if (WLCISACPHY(wlc->band)) {
#if AMSDU_VHT_USE_HT_AGG_LIMITS_ENAB
				max_agg = HT_MIN_AMSDU;
#else
				max_agg = VHT_MAX_AMSDU;
#endif /* AMSDU_VHT_USE_HT_AGG_LIMITS_ENAB */
			} else {
				max_agg = HT_MAX_AMSDU;
			}
			fifo_size = wlc->xmtfifo_szh[prio2fifo[i]];
			/* blocks to bytes */
			fifo_size = fifo_size * TXFIFO_SIZE_UNIT;

			if (frag_disabled &&
				ami->amsdu_agg_bytes_limit[i] == MIN(max_agg, fifo_size)) {
				/* nothing to be done; no update needed */
				continue;
			}
#ifdef BCMDBG
			if (wlc->fragthresh[WME_PRIO2AC(i)] > MIN(max_agg, fifo_size)) {
				WL_AMSDU(("wl%d:%s: MIN(max_agg=%d, fifo_sz=%d)=>amsdu_max_agg\n",
					ami->wlc->pub->unit, __FUNCTION__, max_agg, fifo_size));
			}
#endif /* BCMDBG */
			ami->amsdu_agg_bytes_limit[i] = MIN(fifo_size, max_agg);
			/* if frag not disabled, then take into account the fragthresh */
			if (!frag_disabled) {
				ami->amsdu_agg_bytes_limit[i] =
					MIN(ami->amsdu_agg_bytes_limit[i],
					wlc->fragthresh[WME_PRIO2AC(i)]);
			}
		}
		ami->amsdu_agg_allowprio[i] = (ami->amsdu_agg_bytes_limit[i] > AMSDU_AGGBYTES_MIN);
		if (!ami->amsdu_agg_allowprio[i])
			WL_INFORM(("wlc_amsdu_agg_frag_upd: fragthresh is too small for AMSDU %d\n",
				i));
	}

	/* toss A-MSDU since bust it up is very expensive, can't push through */
	if (flush)
		wlc_amsdu_agg_flush(ami);

	/* update all scb limit */
	FOREACHSCB(wlc->scbstate, &scbiter, scb)
		wlc_amsdu_scb_agglimit_upd(ami, scb);
}

/* deal with WME txop dynamically shrink */
void
wlc_amsdu_txop_upd(amsdu_info_t *ami)
{
	/* XXX this is tricky.
	 * this can happen dynamically when rate is changed,
	 * how to take remaining txop into account
	 * refer to wme_txop[ac]
	 */
}

void
wlc_amsdu_scb_agglimit_upd(amsdu_info_t *ami, struct scb *scb)
{
	wlc_amsdu_scb_ht_agglimit_upd(ami, scb);
#ifdef WL11AC
	wlc_amsdu_scb_vht_agglimit_upd(ami, scb);
#endif /* WL11AC */
}

#ifdef WL11AC
void
wlc_amsdu_scb_vht_agglimit_upd(amsdu_info_t *ami, struct scb *scb)
{
	uint i;
	scb_amsdu_t *scb_ami;
	uint16 vht_pref = 0;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	vht_pref = wlc_vht_get_scb_amsdu_mtu_pref(ami->wlc->vhti, scb);
#ifdef BCMDBG
	WL_AMSDU(("wl%d: %s: scb=%s scb->vht_mtu_pref %d\n",
		ami->wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&(scb->ea), eabuf),
		vht_pref));
#endif // endif

	for (i = 0; i < NUMPRIO; i++) {
		WL_AMSDU(("old: prio[%d].vhtaggbytesmax = %d", i,
			scb_ami->prio[i].amsdu_vht_agg_bytes_max));
#ifndef BCMSIM
		scb_ami->prio[i].amsdu_vht_agg_bytes_max =
			MIN(vht_pref,
			ami->amsdu_agg_bytes_limit[i]);
#else
		/* BCMSIM has limited 2K buffer size */
		scb_ami->prio[i].amsdu_agg_bytes_max = AMSDU_MAX_MSDU_PKTLEN;
#endif // endif
		WL_AMSDU((" new: prio[%d].vht_agg_bytes_max = %d\n", i,
			scb_ami->prio[i].amsdu_vht_agg_bytes_max));
	}
}
#endif /* WL11AC */

void
wlc_amsdu_scb_ht_agglimit_upd(amsdu_info_t *ami, struct scb *scb)
{
	uint i;
	scb_amsdu_t *scb_ami;
	uint16 ht_pref = wlc_ht_get_scb_amsdu_mtu_pref(ami->wlc->hti, scb);

#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];

	WL_AMSDU(("wl%d: %s: scb=%s scb->amsdu_ht_mtu_pref %d\n",
		ami->wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&(scb->ea), eabuf),
		ht_pref));
#endif // endif

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	for (i = 0; i < NUMPRIO; i++) {
		WL_AMSDU(("old: prio[%d].ht_agg_bytes_max = %d", i,
			scb_ami->prio[i].amsdu_ht_agg_bytes_max));
#ifndef BCMSIM
		scb_ami->prio[i].amsdu_ht_agg_bytes_max = MIN(ht_pref,
			ami->amsdu_agg_bytes_limit[i]);
#else
		/* BCMSIM has limited 2K buffer size */
		scb_ami->prio[i].amsdu_agg_bytes_max = AMSDU_MAX_MSDU_PKTLEN;
#endif // endif
		WL_AMSDU((" new: prio[%d].ht_agg_bytes_max = %d\n", i,
			scb_ami->prio[i].amsdu_ht_agg_bytes_max));
	}
}

/* A-MSDU admission control, per-scb-tid.
 * called from tx completion, to decrement agg_txpend, compare with LOWM/HIWM
 * - this is called regardless the tx frame is AMSDU or not. the amsdu_agg_txpending
 *   increment/decrement for any traffic for scb-tid.
 * - work on best-effort traffic only for now, can be expanded to other in the future
 * - amsdu_agg_txpending never go below 0
 * - amsdu_agg_txpending may not be accurate before/after A-MSDU agg is added to txmodule
 *   config/unconfig dynamically
 */
static void
wlc_amsdu_dotxstatus(amsdu_info_t *ami, struct scb *scb, void* p)
{
	uint tid;
	scb_amsdu_t *scb_ami;

	WL_AMSDU(("wlc_amsdu_dotxstatus\n"));

	ASSERT(scb && SCB_AMSDU(scb));

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	ASSERT(scb_ami);

	tid = (uint8)PKTPRIO(p);

	if (PRIO_8021D_BE != tid) {
		WL_AMSDU(("wlc_amsdu_dotxstatus, tid %d\n", tid));
		return;
	}

	if (scb_ami->prio[tid].amsdu_agg_txpending > 0)
		scb_ami->prio[tid].amsdu_agg_txpending--;
	WL_AMSDU(("wlc_amsdu_dotxstatus: scb txpending reduce to %d\n",
		scb_ami->prio[tid].amsdu_agg_txpending));

	/* close all aggregation if changed to disable
	 * XXX to optimize and reduce per-pkt callback
	 */
	if ((scb_ami->prio[tid].amsdu_agg_txpending < ami->fifo_lowm)) {
		if (scb_ami->prio[tid].amsdu_agg_p && !(scb->flags & SCB_PENDING_FREE)) {
			WL_AMSDU(("wlc_amsdu_dotxstatus: release amsdu due to low watermark!!\n"));
			wlc_amsdu_agg_close(ami, scb, tid);
			WLCNTINCR(ami->cnt->agg_stop_lowwm);
		}
	}
}

#ifdef DISABLE_AMSDUTX_FOR_VI
/* Check if AMSDU is enabled for given tid */
bool
wlc_amsdu_chk_priority_enable(amsdu_info_t *ami, uint8 tid)
{
	return ami->amsdu_agg_allowprio[tid];
}
#endif // endif

/* centralize A-MSDU tx policy */
void
wlc_amsdu_txpolicy_upd(amsdu_info_t *ami)
{
	WL_AMSDU(("wlc_amsdu_txpolicy_upd\n"));

	if (PIO_ENAB(ami->pub))
		ami->amsdu_agg_block = TRUE;
	else {
		int idx;
		wlc_bsscfg_t *cfg;
		FOREACH_BSS(ami->wlc, idx, cfg) {
			if (!cfg->BSS)
				ami->amsdu_agg_block = TRUE;
		}
	}

	ami->amsdu_agg_block = FALSE;
}

/* Return vht max if to send at vht rates
* Return ht max if to send at ht rates
* Return 0 otherwise
*/
static uint
wlc_amsdu_get_max_agg_bytes(struct scb *scb, scb_amsdu_t *scb_ami,
	uint tid, void *p, ratespec_t rspec)
{
	if (RSPEC_ISVHT(rspec)) {
		WL_AMSDU(("vht rate max agg used = %d\n",
			scb_ami->prio[tid].amsdu_vht_agg_bytes_max));
		return scb_ami->prio[tid].amsdu_vht_agg_bytes_max;
	} else {
		WL_AMSDU(("ht rate max agg used = %d\n",
			scb_ami->prio[tid].amsdu_ht_agg_bytes_max));
		return scb_ami->prio[tid].amsdu_ht_agg_bytes_max;
	}
}

#endif /* WLAMSDU_TX */

#if defined(PKTC) || defined(PKTC_TX_DONGLE)

#ifdef DUALPKTAMSDU
/*
 * Do AMSDU aggregation for chained packets.
 */
void * BCMFASTPATH
wlc_amsdu_pktc_agg(amsdu_info_t *ami, struct scb *scb, void *p, void *n,
	uint8 tid, uint32 lifetime)
{
	wlc_info_t *wlc;
	int32 pad, totlen = 0;
	struct ether_header *eh = NULL;
	scb_amsdu_t *scb_ami;
	void *p1 = NULL;
	uint32 p_len, n_len = 0;

	pad = 0;
	wlc = ami->wlc;
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	p_len = pkttotlen(wlc->osh, p);
	n_len = pkttotlen(wlc->osh, n);

	if (ami->amsdu_agg_sframes_limit[tid] <= 1) {
		WLCNTINCR(ami->cnt->agg_stop_sf);
		return n;
	} else if ((n_len + p_len)
		>= scb_ami->prio[tid].amsdu_vht_agg_bytes_max) {
		WLCNTINCR(ami->cnt->agg_stop_len);
		return n;
	}
#if defined(PROP_TXSTATUS) && defined(BCMPCIEDEV)
	else if (BCMPCIEDEV_ENAB() && PROP_TXSTATUS_ENAB(wlc->pub) &&
		WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
		/*
		 * This comparison does the following:
		 *  - For suppressed pkts in AMSDU, the mask should be same
		 *      so re-aggregate them
		 *  - For new pkts, pkttag->seq is zero so same
		 *  - Prevents suppressed pkt from being aggregated with non-suppressed pkt
		 *  - Prevents suppressed MPDU from being aggregated with suppressed MSDU
		 */
		if ((WLPKTTAG(p)->seq & WL_SEQ_AMSDU_SUPPR_MASK) !=
			(WLPKTTAG(n)->seq & WL_SEQ_AMSDU_SUPPR_MASK)) {
			return n;
		}
	}
#endif /* PROP_TXSTATUS && BCMPCIEDEV */

	/* Padding of A-MSDU sub-frame to 4 bytes */
	pad = (uint)((-(int)(p_len)) & 3);

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	/* For first msdu init ether header (amsdu header) */
	if (BCMDHDHDR_ENAB() && PKTISTXFRAG(wlc->osh, p)) {
		/* When BCM_DHDHDR enabled the DHD host driver will prepare the
		 * dot3_mac_llc_snap_header, now we adjust ETHER_HDR_LEN bytes
		 * in host data addr to include the ether header 14B part.
		 * The ether header 14B in dongle D3_BUFFER is going to be used as
		 * amsdu header.
		 * Now, first msdu has all DHDHDR 22B in host.
		 */
		PKTSETFRAGDATA_LO(wlc->osh, p, 1,
			PKTFRAGDATA_LO(wlc->osh, p, 1) - ETHER_HDR_LEN);
		PKTSETFRAGLEN(wlc->osh, p, 1,
			PKTFRAGLEN(wlc->osh, p, 1) + ETHER_HDR_LEN);
		PKTSETFRAGTOTLEN(wlc->osh, p,
			PKTFRAGTOTLEN(wlc->osh, p) + ETHER_HDR_LEN);

		/* use ether header 14B space in D3_BUFFER to save the amsdu header */
		eh = (struct ether_header *)PKTDATA(wlc->osh, p);
	}
	else
#endif /* BCM_DHDHDR && DONGLEBUILD */
		eh = (struct ether_header *)PKTPUSH(wlc->osh, p, ETHER_HDR_LEN);

	eacopy(&scb->ea, eh->ether_dhost);
	eacopy(&(SCB_BSSCFG(scb)->cur_etheraddr), eh->ether_shost);
	WLPKTTAG(p)->flags |= WLF_AMSDU;

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	/* For second msdu */
	if (BCMDHDHDR_ENAB() && PKTISTXFRAG(wlc->osh, n)) {
		/* Before we free the second msdu D3_BUFFER we have to include the
		 * ether header 14B part in host.
		 * Now, second msdu has all DHDHDR 22B in host.
		 */
		PKTSETFRAGDATA_LO(wlc->osh, n, 1,
			PKTFRAGDATA_LO(wlc->osh, n, 1) - ETHER_HDR_LEN);
		PKTSETFRAGLEN(wlc->osh, n, 1,
			PKTFRAGLEN(wlc->osh, n, 1) + ETHER_HDR_LEN);
		PKTSETFRAGTOTLEN(wlc->osh, n,
			PKTFRAGTOTLEN(wlc->osh, n) + ETHER_HDR_LEN);

		if (!AMSDUFRAG_ENAB()) {
			/* Free the second msdu D3_BUFFER, we don't need it */
			PKTBUFEARLYFREE(wlc->osh, n);
		}
	}
#endif /* BCM_DHDHDR && DONGLEBUILD */

	/* Append msdu p1 to p */
	p1 = n;
	n = PKTCLINK(p1);
	PKTSETCLINK(p1, NULL);
	PKTCLRCHAINED(wlc->osh, p1);
	PKTSETNEXT(wlc->osh, pktlast(wlc->osh, p), p1);
	if (n)
		wlc_pktc_sdu_prep(wlc, scb, p1, n, lifetime);

	if (pad) {
#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
		if (BCMDHDHDR_ENAB() && PKTISTXFRAG(wlc->osh, p)) {
			/* pad data will be the garbage at the end of host data */
			PKTSETFRAGLEN(wlc->osh, p, 1, PKTFRAGLEN(wlc->osh, p, 1) + pad);
			PKTSETFRAGTOTLEN(wlc->osh, p, PKTFRAGTOTLEN(wlc->osh, p) + pad);
		}
		else
#endif // endif
		/* If a padding was required for the previous packet, apply it here */
		PKTPUSH(wlc->osh, p1, pad);
	}

	totlen = n_len + p_len + pad;
	eh->ether_type = HTON16(totlen);

	WLCNTINCR(ami->cnt->agg_amsdu);
	WLCNTADD(ami->cnt->agg_msdu, 2);
#ifdef WL11K
	WLCNTADD(ami->cnt->agg_amsdu_bytes_l, totlen);
	if (ami->cnt->agg_amsdu_bytes_l < totlen)
		WLCNTINCR(ami->cnt->agg_amsdu_bytes_h);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	/* update statistics histograms */
	ami->amdbg->tx_msdu_histogram[1]++;
	ami->amdbg->tx_length_histogram[MIN(totlen / AMSDU_LENGTH_BIN_BYTES,
		AMSDU_LENGTH_BINS-1)]++;
#endif /* BCMDBG */
#if defined(BCM_DHDHDR) && defined(AMSDU_FRAG_OPT)
	/*
	 * With AMSDU_FRAG_OPT two AMSDU linked lfrags are shrinked into one.
	 * copy relevant contents from p1 inot p and free p1 back to pool
	 */
	if (BCMDHDHDR_ENAB() && AMSDUFRAG_ENAB() && p && p1) {
		/* Copy host frag info form p1 to p */
		PKTFRAGCOPY(wlc->osh, p, p1);

		/* Unlink both the packets */
		PKTSETNEXT(wlc->osh, p, NULL);

		/* Dont process txstatus for this packet from bus layer */
		PKTRESETHASMETADATA(wlc->osh, p1);

		/* Free Now */
		PKTFREE(wlc->osh, p1, TRUE);
	}
#endif /* BCM_DHDHDR && AMSDU_FRAG_OPT */

	return n;
}

#else /* DUALPKTAMSDU */

/*
 * Do AMSDU aggregation for chained packets.
 */
void * BCMFASTPATH
wlc_amsdu_pktc_agg(amsdu_info_t *ami, struct scb *scb, void *p, void *n,
	uint8 tid, uint32 lifetime)
{
	wlc_info_t *wlc;
	int32 pad, i, totlen = 0;
	int32 lastpad = 0;
	struct ether_header *eh = NULL;
	scb_amsdu_t *scb_ami;
	void *p1 = NULL;
	bool pad_at_head = FALSE;

#ifdef BCMDBG
	if (ami->amsdu_agg_block) {
		WLCNTINCR(ami->cnt->agg_passthrough);
		return (n);
	}
#endif /* BCMDBG */

	pad = 0;
	i = 1;
	wlc = ami->wlc;
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	totlen = pkttotlen(wlc->osh, p);

	while (1) {
#ifdef WLOVERTHRUSTER
		if (OVERTHRUST_ENAB(wlc->pub)) {
			/* Check if the next subframe is possibly TCP ACK */
			if (pkttotlen(wlc->osh, n) <= TCPACKSZSDU)
				break;
		}
#endif /* WLOVERTHRUSTER */
		if (i >= ami->amsdu_agg_sframes_limit[tid]) {
			WLCNTINCR(ami->cnt->agg_stop_sf);
			break;
		} else if ((totlen + pkttotlen(wlc->osh, n))
			>= scb_ami->prio[tid].amsdu_vht_agg_bytes_max) {
			WLCNTINCR(ami->cnt->agg_stop_len);
			break;
		}
#if defined(PROP_TXSTATUS) && defined(BCMPCIEDEV)
		else if (BCMPCIEDEV_ENAB() && PROP_TXSTATUS_ENAB(wlc->pub) &&
			WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
			/*
			 * This comparison does the following:
			 *  - For suppressed pkts in AMSDU, the mask should be same
			 *      so re-aggregate them
			 *  - For new pkts, pkttag->seq is zero so same
			 *  - Prevents suppressed pkt from being aggregated with non-suppressed pkt
			 *  - Prevents suppressed MPDU from being aggregated with suppressed MSDU
			 */
			if ((WLPKTTAG(p)->seq & WL_SEQ_AMSDU_SUPPR_MASK) !=
				(WLPKTTAG(n)->seq & WL_SEQ_AMSDU_SUPPR_MASK)) {
				break;
			}
		}
#endif /* PROP_TXSTATUS && BCMPCIEDEV */

		/* Padding of A-MSDU sub-frame to 4 bytes */
		pad = (uint)((-(int)(pkttotlen(wlc->osh, p) - lastpad)) & 3);

		if (i == 1) {
			/* Init ether header */
			eh = (struct ether_header *)PKTPUSH(wlc->osh, p, ETHER_HDR_LEN);
			eacopy(&scb->ea, eh->ether_dhost);
			eacopy(&(SCB_BSSCFG(scb)->cur_etheraddr), eh->ether_shost);
			WLPKTTAG(p)->flags |= WLF_AMSDU;
			totlen = pkttotlen(wlc->osh, p);
		}

#ifdef DMATXRC
		/* Add padding to next pkt if current is phdr */
		if (DMATXRC_ENAB(wlc->pub) && (WLPKTTAG(p)->flags & WLF_PHDR)) {
			/* Possible p is not phdr but is flagged as WLF_PHDR
			 * in which case there's no next pkt.
			 */
			if (PKTNEXT(wlc->osh, p))
				p = PKTNEXT(wlc->osh, p);
		}
#endif // endif

		/* Add padding to next pkt (at headroom) if current is a lfrag */
		if (BCMLFRAG_ENAB() && PKTISTXFRAG(wlc->osh, p)) {
			/*
			 * Store the padding value. We need this to be accounted for, while
			 * calculating the padding for the subsequent packet.
			 * For example, if we need padding of 3 bytes for the first packet,
			 * we add those 3 bytes padding in the head of second packet. Now,
			 * to check if padding is required for the second packet, we should
			 * calculate the padding without considering these 3 bytes that
			 * we have already put in.
			 */
			lastpad = pad;

			if (pad) {
				/*
				 * Let's just mark that padding is required at the head of next
				 * packet. Actual padding needs to be done after the next sdu
				 * header has been copied from the current sdu.
				 */
				pad_at_head = TRUE;

#ifdef WLCNT
				WLCNTINCR(ami->cnt->tx_padding_in_head);
#endif /* WLCNT */
			}
		} else {
			PKTSETLEN(wlc->osh, p, pkttotlen(wlc->osh, p) + pad);
			totlen += pad;
#ifdef WLCNT
			WLCNTINCR(ami->cnt->tx_padding_in_tail);
#endif /* WLCNT */
		}

		/* Append msdu p1 to p */
		p1 = n;
		n = PKTCLINK(p1);
		PKTSETCLINK(p1, NULL);
		PKTCLRCHAINED(wlc->osh, p1);
		if (BCMLFRAG_ENAB()) {
			PKTSETNEXT(wlc->osh, pktlast(wlc->osh, p), p1);
			PKTSETNEXT(wlc->osh, pktlast(wlc->osh, p1), NULL);
		} else {
			PKTSETNEXT(wlc->osh, p, p1);
			PKTSETNEXT(wlc->osh, p1, NULL);
		}
		totlen += pkttotlen(wlc->osh, p1);
		i++;

		/* End of pkt chain */
		if (n == NULL)
			break;

		wlc_pktc_sdu_prep(wlc, scb, p1, n, lifetime);

		if (pad_at_head == TRUE) {
			/* If a padding was required for the previous packet, apply it here */
			PKTPUSH(wlc->osh, p1, pad);
			totlen += pad;
			pad_at_head = FALSE;
		}
		p = p1;
	}

	WLCNTINCR(ami->cnt->agg_amsdu);
	WLCNTADD(ami->cnt->agg_msdu, i);

	if (pad_at_head == TRUE) {
		/* If a padding was required for the previous/last packet, apply it here */
		PKTPUSH(wlc->osh, p1, pad);
		totlen += pad;
		pad_at_head = FALSE;
	}

#ifdef WL11K
	WLCNTADD(ami->cnt->agg_amsdu_bytes_l, totlen);
	if (ami->cnt->agg_amsdu_bytes_l < totlen)
		WLCNTINCR(ami->cnt->agg_amsdu_bytes_h);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	/* update statistics histograms */
	ami->amdbg->tx_msdu_histogram[MIN(i-1,
		MAX_TX_SUBFRAMES_LIMIT-1)]++;
	ami->amdbg->tx_length_histogram[MIN(totlen / AMSDU_LENGTH_BIN_BYTES,
		AMSDU_LENGTH_BINS-1)]++;
#endif /* BCMDBG */

	if (i > 1)
		eh->ether_type = HTON16(totlen - ETHER_HDR_LEN);

	return n;
}
#endif /* DUALPKTAMSDU */
#endif /* PKTC */

#ifdef WLAMSDU_TX
/*
 * MSDU aggregation, per-A1-TID(priority)
 * Return TRUE if packet consumed, otherwise FALSE
 */
static void
wlc_amsdu_agg(void *ctx, struct scb *scb, void *p, uint prec)
{
	amsdu_info_t *ami;
	osl_t *osh;
	uint tid = 0;
	scb_amsdu_t *scb_ami;
	wlc_bsscfg_t *bsscfg;
	uint totlen;
	ratespec_t rspec;
	uint maxagglen;

	ami = (amsdu_info_t *)ctx;
	osh = ami->pub->osh;
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);

	WL_AMSDU(("%s\n", __FUNCTION__));

	tid = PKTPRIO(p);
	ASSERT(tid < NUMPRIO);

#if defined(PKTC) || defined(PKTC_TX_DONGLE)
	if (PKTC_ENAB(ami->pub)) {
		/* When chaining and amsdu tx are enabled try doing AMSDU agg
		 * while queing the frames to per scb queues.
		 */
		WL_TRACE(("%s: skipping amsdu mod\n", __FUNCTION__));
		goto passthrough;
	}
#endif // endif
	if (ami->amsdu_agg_block)
		goto passthrough;

	/* doesn't handle MPDU,  */
	if (WLPKTTAG(p)->flags & WLF_MPDU)
		goto passthrough;

	/* non best-effort, skip for now */
	if (tid != PRIO_8021D_BE)
		goto passthrough;

	/* admission control */
	if (!ami->amsdu_agg_allowprio[tid])
		goto passthrough;

	if (WLPKTTAG(p)->flags & WLF_8021X)
		goto passthrough;

#ifdef BCMWAPI_WAI
	if (WLPKTTAG(p)->flags & WLF_WAI)
		goto passthrough;
#endif /* BCMWAPI_WAI */

	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb);

	/* the scb must be A-MSDU capable */
	ASSERT(SCB_AMSDU(scb));
	ASSERT(SCB_QOS(scb));

	rspec = wlc_scb_ratesel_get_primary(scb->bsscfg->wlc, scb, p);

#ifdef VHT_TESTBED
	if (RSPEC_ISLEGACY(rspec)) {
		WL_AMSDU(("pass thru due to use legacy rate\n"));
		goto passthrough;
	}
#else
	if (WLCISACPHY(scb->bsscfg->wlc->band)) {
		if (!RSPEC_ISVHT(rspec)) {
			WL_AMSDU(("pass thru due to non-VHT on VHT phy\n"));
			goto passthrough;
		}
	}
	else {
		/* add for WNM support (r365273) */
		if (WLC_PHY_11N_CAP(scb->bsscfg->wlc->band) && !RSPEC_ISHT(rspec)) {
			WL_AMSDU(("pass thru due to non-HT on HT phy\n"));
			goto passthrough;
		}
	}
#endif /* VHT_TESTBED */

	if (SCB_AMPDU(scb)) {
		/* Bypass AMSDU agg if the destination has AMPDU enabled but does not
		 * advertise AMSDU in AMPDU.
		 * This capability is advertised in the ADDBA response.
		 * This check is not taking into account whether or not the AMSDU will
		 * be encapsulated in an AMPDU, so it is possibly too conservative.
		 */
		if (!SCB_AMSDU_IN_AMPDU(scb)) {
			goto passthrough;
		}

#ifdef WLOVERTHRUSTER
		if (OVERTHRUST_ENAB(ami->wlc->pub)) {
			/* Allow TCP ACKs to flow through to Overthruster if it is enabled */
			if (wlc_ampdu_tx_get_tcp_ack_ratio(ami->wlc->ampdu_tx) > 0 &&
				wlc_amsdu_is_tcp_ack(ami, p)) {
				goto passthrough;
			}
		}
#endif /* WLOVERTHRUSTER */
	}

#ifdef WLAMPDU_PRECEDENCE
	/* check existing AMSDU (if there is one) has same WLF2_FAVORED attribute as
	 * new packet
	 */
	if (scb_ami->prio[tid].amsdu_agg_p) {
		if ((WLPKTTAG(p)->flags2 & WLF3_FAVORED) !=
		       (WLPKTTAG(scb_ami->prio[tid].amsdu_agg_p)->flags2 & WLF3_FAVORED)) {
			WL_AMSDU(("close amsdu due to change in favored status\n"));
			wlc_amsdu_agg_close(ami, scb, tid);
		}
	}
#endif // endif

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	if (WSEC_ENABLED(bsscfg->wsec) && !WSEC_AES_ENABLED(bsscfg->wsec)) {
		WL_AMSDU(("%s: target scb %p is has wrong WSEC\n", __FUNCTION__, scb));
		goto passthrough;
	}

	WL_AMSDU(("%s: txpend %d\n", __FUNCTION__, scb_ami->prio[tid].amsdu_agg_txpending));
	if (scb_ami->prio[tid].amsdu_agg_txpending < ami->fifo_hiwm) {
		goto passthrough;
	} else {
		WL_AMSDU(("%s: Starts aggregation due to hiwm %d reached\n",
			__FUNCTION__, ami->fifo_hiwm));
	}

	totlen = pkttotlen(osh, p) + scb_ami->prio[tid].amsdu_agg_bytes +
		scb_ami->prio[tid].headroom_pad_need;
	maxagglen = wlc_amsdu_get_max_agg_bytes(scb, scb_ami, tid, p, rspec);

	if ((totlen > maxagglen - ETHER_HDR_LEN) ||
		(scb_ami->prio[tid].amsdu_agg_sframes + 1 >
		ami->amsdu_agg_sframes_limit[tid])) {
		WL_AMSDU(("%s: terminte A-MSDU for txbyte %d or txframe %d\n",
			__FUNCTION__, maxagglen,
			ami->amsdu_agg_sframes_limit[tid]));
		wlc_amsdu_agg_close(ami, scb, tid);
#ifdef WLCNT
		if (totlen > maxagglen - ETHER_HDR_LEN) {
			WLCNTINCR(ami->cnt->agg_stop_len);
		} else {
			WLCNTINCR(ami->cnt->agg_stop_sf);
		}
#endif	/* WLCNT */

		/* if the new pkt itself is more than aggmax, can't continue with agg_append
		 *   add here to avoid per pkt checking for this rare case
		 */
		if (pkttotlen(osh, p) > maxagglen - ETHER_HDR_LEN) {
			WL_AMSDU(("%s: A-MSDU aggmax is smaller than pkt %d, pass\n",
				__FUNCTION__, pkttotlen(osh, p)));

			goto passthrough;
		}
	}

	/* agg this one and return on success */
	if (wlc_amsdu_agg_append(ami, scb, p, tid, rspec))
		return;

passthrough:
	/* A-MSDU agg rejected, pass through to next tx module */

	/* release old first before passthrough new one to maintain sequence */
	if (scb_ami->prio[tid].amsdu_agg_p) {
		WL_AMSDU(("%s: release amsdu for passthough!!\n", __FUNCTION__));
		wlc_amsdu_agg_close(ami, scb, tid);
		WLCNTINCR(ami->cnt->agg_stop_passthrough);
	}
	scb_ami->prio[tid].amsdu_agg_txpending++;
	WLCNTINCR(ami->cnt->agg_passthrough);
	WL_AMSDU(("%s: passthrough scb %p txpending %d\n", __FUNCTION__, scb,
		scb_ami->prio[tid].amsdu_agg_txpending));
	WLF2_PCB3_REG(p, WLF2_PCB3_AMSDU);
	SCB_TX_NEXT(TXMOD_AMSDU, scb, p, WLC_PRIO_TO_PREC(tid));
	return;
}

/* close A-MSDU
 * ??? cck rate is not supported in hw, how to restrict rate algorithm later
 */
static void
wlc_amsdu_agg_close(amsdu_info_t *ami, struct scb *scb, uint tid)
{
	scb_amsdu_t *scb_ami;
	void *ptid;
	osl_t *osh;
	struct ether_header *eh;
	amsdu_scb_prio *amsduprio;
	uint16 amsdu_body_len;

	WL_AMSDU(("wlc_amsdu_agg_close\n"));

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	osh = ami->pub->osh;
	amsduprio = &scb_ami->prio[tid];
	ptid = amsduprio->amsdu_agg_p;

	if (ptid == NULL)
		return;

	ASSERT(WLPKTFLAG_AMSDU(WLPKTTAG(ptid)));

	/* wlc_pcb_fn_register(wlc->pcb, wlc_amsdu_tx_complete, ami, ptid); */

	/* check */
	ASSERT(PKTLEN(osh, ptid) >= ETHER_HDR_LEN);
	ASSERT(tid == (uint)PKTPRIO(ptid));

	/* FIXUP lastframe pad --- the last subframe must not be padded,
	 * reset pktlen to the real length(strip off pad) using previous
	 * saved value.
	 * amsduprio->amsdu_agg_ptail points to the last buf(not last pkt)
	 */
	if (amsduprio->amsdu_agg_padlast) {
		PKTSETLEN(osh, amsduprio->amsdu_agg_ptail,
			PKTLEN(osh, amsduprio->amsdu_agg_ptail) -
			amsduprio->amsdu_agg_padlast);
		amsduprio->amsdu_agg_bytes -= amsduprio->amsdu_agg_padlast;
		WL_AMSDU(("wlc_amsdu_agg_close: strip off padlast %d\n",
			amsduprio->amsdu_agg_padlast));
#ifdef WLCNT
		WLCNTDECR(ami->cnt->tx_padding_in_tail);
		WLCNTINCR(ami->cnt->tx_padding_no_pad);
#endif /* WLCNT */
	}
	amsdu_body_len = (uint16) amsduprio->amsdu_agg_bytes;

	eh = (struct ether_header*) PKTDATA(osh, ptid);
	eh->ether_type = hton16(amsdu_body_len);

	WLCNTINCR(ami->cnt->agg_amsdu);
	WLCNTADD(ami->cnt->agg_msdu, amsduprio->amsdu_agg_sframes);
#ifdef WL11K
	WLCNTADD(ami->cnt->agg_amsdu_bytes_l, amsdu_body_len);
	if (ami->cnt->agg_amsdu_bytes_l < amsdu_body_len)
		WLCNTINCR(ami->cnt->agg_amsdu_bytes_h);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	/* update statistics histograms */
	ami->amdbg->tx_msdu_histogram[MIN(amsduprio->amsdu_agg_sframes-1,
	                                  MAX_TX_SUBFRAMES_LIMIT-1)]++;
	ami->amdbg->tx_length_histogram[MIN(amsdu_body_len / AMSDU_LENGTH_BIN_BYTES,
	                                    AMSDU_LENGTH_BINS-1)]++;
#endif /* BCMDBG */

	amsduprio->amsdu_agg_txpending++;

	WL_AMSDU(("wlc_amsdu_agg_close: valid AMSDU, add to txq %d bytes, scb %p, txpending %d\n",
		amsduprio->amsdu_agg_bytes, scb, scb_ami->prio[tid].amsdu_agg_txpending));

	/* clear state prior to calling next txmod, else crash may occur if next mod frees pkt */
	/* due to amsdu use of pcb to track pkt freeing */
	amsduprio->amsdu_agg_p = NULL;
	amsduprio->amsdu_agg_ptail = NULL;
	amsduprio->amsdu_agg_sframes = 0;
	amsduprio->amsdu_agg_bytes = 0;
	amsduprio->headroom_pad_need = 0;
	WLF2_PCB3_REG(ptid, WLF2_PCB3_AMSDU);
	SCB_TX_NEXT(TXMOD_AMSDU, scb, ptid, WLC_PRIO_TO_PREC(tid));
}

/* create a psudo ether_header,
 *   the len field will be udpated when aggregating more frames
 */
static void *
wlc_amsdu_agg_open(amsdu_info_t *ami, wlc_bsscfg_t *bsscfg, struct ether_addr *ea, void *p)
{
	struct ether_header* eh;
	uint headroom;
	void *pkt;
	osl_t *osh;
	wlc_info_t *wlc = ami->wlc;
	wlc_pkttag_t *pkttag;

	osh = wlc->osh;

	/* allocate enough room once for all cases */
	headroom = TXOFF;

#ifdef BCMCCX
	headroom += CKIP_MIC_SIZE + CKIP_SEQ_SIZE + 2;
	/* the extra 2 bytes is needed for the difference between ckip_hdr and 802.3 hdr */
#endif // endif

	if (BCMLFRAG_ENAB() && PKTISTXFRAG(osh, p)) {
		/*
		 * LFRAGs have enough headroom to accomodate the MPDU headers
		 * in the same packet. We do not need a new packet to be allocated
		 * for this. Let's try to reuse the same packet.
		 */
		ASSERT(headroom <= (uint)PKTHEADROOM(osh, p));

		/* Update the PKTTAG with AMSDU flag */
		pkttag = WLPKTTAG(p);
		ASSERT(!WLPKTFLAG_AMSDU(pkttag));
		pkttag->flags |= WLF_AMSDU;

		/*
		 * Prepend the |DA SA LEN| to the same packet. At the end of this
		 * operation, the packet would contain the following.
		 * +-----------------------------+-------------------+---------------------------+
		 * |A-MSDU header(ethernet like) | Subframe1 8023hdr | Subframe1 body(Host Frag) |
		 * +-----------------------------+-------------------+---------------------------+
		 */
		eh = (struct ether_header*) PKTPUSH(osh, p, ETHER_HDR_LEN);
		bcopy((char*)ea, eh->ether_dhost, ETHER_ADDR_LEN);
		bcopy((char*)&bsscfg->cur_etheraddr, eh->ether_shost, ETHER_ADDR_LEN);
		eh->ether_type = hton16(0);	/* no payload bytes yet */
		return p;
	}

	/* alloc new frame buffer */
	if ((pkt = PKTGET(osh, headroom, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s, PKTGET headroom %d failed\n",
			wlc->pub->unit, __FUNCTION__, headroom));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return NULL;
	}

	ASSERT(ISALIGNED(PKTDATA(osh, pkt), sizeof(uint32)));

	/* construct AMSDU frame as
	 * | DA SA LEN | Sub-Frame1 | Sub-Frame2 | ...
	 * its header is not converted to 8023hdr, it will be replaced by dot11 hdr directly
	 * the len is the totlen of the whole aggregated AMSDU, including padding
	 * need special flag for later differentiation
	 */

	/* adjust the data point for correct pkttotlength */
	PKTPULL(osh, pkt, headroom);
	PKTSETLEN(osh, pkt, 0);

	/* init ether_header */
	eh = (struct ether_header*) PKTPUSH(osh, pkt, ETHER_HDR_LEN);

	bcopy((char*)ea, eh->ether_dhost, ETHER_ADDR_LEN);
	bcopy((char*)&bsscfg->cur_etheraddr, eh->ether_shost, ETHER_ADDR_LEN);
	eh->ether_type = hton16(0);	/* no payload bytes yet */

	/* transfer pkttag, scb, add AMSDU flag */
	/* ??? how many are valid and should be transferred */
	wlc_pkttag_info_move(wlc, p, pkt);
	PKTSETPRIO(pkt, PKTPRIO(p));
	WLPKTTAGSCBSET(pkt, WLPKTTAGSCBGET(p));
	pkttag = WLPKTTAG(pkt);
	ASSERT(!WLPKTFLAG_AMSDU(pkttag));
	pkttag->flags |= WLF_AMSDU;

	return pkt;
}

/* return true on consumed, false others if
 *      -- first header buffer allocation failed
 *      -- no enough tailroom for pad bytes
 *      -- tot size goes beyond A-MSDU limit
 *
 *  amsdu_agg_p[tid] points to the header lbuf, amsdu_agg_ptail[tid] points to the tail lbuf
 *
 * The A-MSDU format typically will be below
 *   | A-MSDU header(ethernet like) |
 *	|subframe1 8023hdr |
 *		|subframe1 body | pad |
 *			|subframe2 8023hdr |
 *				|subframe2 body | pad |
 *					...
 *						|subframeN 8023hdr |
 *							|subframeN body |
 * It's not required to have pad bytes on the last frame
*/
static bool
wlc_amsdu_agg_append(amsdu_info_t *ami, struct scb *scb, void *p, uint tid,
	ratespec_t rspec)
{
	uint len, totlen;
	bool pad_end_supported = TRUE;
	osl_t *osh;
	scb_amsdu_t *scb_ami;
	void *ptid;
	amsdu_scb_prio *amsduprio;
	uint max_agg_bytes;
	uint pkt_tail_room, pkt_head_room;
	uint8 headroom_pad_need, pad;
	WL_AMSDU(("%s\n", __FUNCTION__));

	osh = ami->pub->osh;
	pkt_tail_room = (uint)PKTTAILROOM(osh, pktlast(osh, p));
	pkt_head_room = (uint)PKTHEADROOM(osh, p);
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	amsduprio = &scb_ami->prio[tid];
	headroom_pad_need = amsduprio->headroom_pad_need;
	max_agg_bytes = wlc_amsdu_get_max_agg_bytes(scb, scb_ami, tid, p, rspec);

	/* length of 802.3/LLC frame, equivalent to A-MSDU sub-frame length, DA/SA/Len/Data */
	len = pkttotlen(osh, p);
	/* padding of A-MSDU sub-frame to 4 bytes */
	pad = (uint)((-(int)len) & 3);
	/* START: check ok to append p to queue */
	/* if we have a pkt being agged */
	/* ensure that we can stick any padding needed on front of pkt */
	/* check here, coz later on we will append p on agg_p after adding pad */
	if (amsduprio->amsdu_agg_p) {
		if (pkt_head_room < headroom_pad_need ||
			(len + headroom_pad_need + amsduprio->amsdu_agg_bytes > max_agg_bytes)) {
				/* start over if we cant' make it */
				/* danger here is if no tailroom or headroom in packets */
				/* we will send out only 1 sdu long amsdu -- slow us down */
				wlc_amsdu_agg_close(ami, scb, tid);
#ifdef WLCNT
				if (pkt_head_room < headroom_pad_need) {
					WLCNTINCR(ami->cnt->agg_stop_tailroom);
				} else {
					WLCNTINCR(ami->cnt->agg_stop_len);
				}
#endif /* WLCNT */
		}
	}
	/* END: check ok to append pkt to queue */
	/* START: allocate header */
	/* alloc new pack tx buffer if necessary */
	if (amsduprio->amsdu_agg_p == NULL) {
		/* catch a common case of a stream of incoming packets with no tailroom */
		/* also throw away if headroom doesn't look like can accomodate */
		/* the assumption is that current pkt headroom is good gauge of next packet */
		/* and if both look insufiicient now, it prob will be insufficient later */
		if (pad > pkt_tail_room && pad > pkt_head_room) {
			WLCNTINCR(ami->cnt->agg_stop_tailroom);
			return FALSE;
		}

		if ((ptid = wlc_amsdu_agg_open(ami, SCB_BSSCFG(scb), &scb->ea, p)) == NULL) {
			WLCNTINCR(ami->cnt->agg_openfail);
			return FALSE;
		}

		amsduprio->amsdu_agg_p = ptid;
		amsduprio->amsdu_agg_ptail = ptid;
		amsduprio->amsdu_agg_sframes = 0;
		amsduprio->amsdu_agg_bytes = 0;
		amsduprio->amsdu_agg_padlast = 0;
		WL_AMSDU(("%s: open a new AMSDU, hdr %d bytes\n",
			__FUNCTION__, amsduprio->amsdu_agg_bytes));
	} else {
		/* step 1: mod cur pkt to take care of prev pkt */
		if (headroom_pad_need) {
			PKTPUSH(osh, p, amsduprio->headroom_pad_need);
			len += headroom_pad_need;
			amsduprio->headroom_pad_need = 0;
#ifdef WLCNT
			WLCNTINCR(ami->cnt->tx_padding_in_head);
#endif /* WLCNT */
		}
	}
	/* END: allocate header */

	/* use short name for convenience */
	ptid = amsduprio->amsdu_agg_p;

	/* START: append packet */
	/* step 2: chain the pkts at the end of current one */
	ASSERT(amsduprio->amsdu_agg_ptail != NULL);

	/* If the AMSDU header was prepended in the same packet, we have */
	/* only one packet to work with and do not need any linking. */
	if (p != amsduprio->amsdu_agg_ptail) {
		PKTSETNEXT(osh, amsduprio->amsdu_agg_ptail, p);
		amsduprio->amsdu_agg_ptail = pktlast(osh, p);

		/* Append any packet callbacks from p to *ptid */
		wlc_pcb_fn_move(ami->wlc->pcb, ptid, p);
	}

	/* step 3: update total length in agg queue */
	totlen = len + amsduprio->amsdu_agg_bytes;
	/* caller already makes sure this frame fits */
	ASSERT(totlen < max_agg_bytes);

	/* END: append pkt */

	/* START: pad current
	 * If padding for this pkt (for 4 bytes alignment) is needed
	 * and feasible(enough tailroom and
	 * totlen does not exceed limit), then add it, adjust length and continue;
	 * Otherwise, close A-MSDU
	 */
	amsduprio->amsdu_agg_padlast = 0;
	if (pad != 0) {
		if (BCMLFRAG_ENAB() && PKTISTXFRAG(osh, amsduprio->amsdu_agg_ptail))
			pad_end_supported = FALSE;

		/* first try using tailroom -- append pad immediately */
		if (((uint)PKTTAILROOM(osh, amsduprio->amsdu_agg_ptail) >= pad) &&
		    (totlen + pad < max_agg_bytes) && (pad_end_supported)) {
			amsduprio->amsdu_agg_padlast = pad;
			PKTSETLEN(osh, amsduprio->amsdu_agg_ptail,
				PKTLEN(osh, amsduprio->amsdu_agg_ptail) + pad);
			totlen += pad;
#ifdef WLCNT
			WLCNTINCR(ami->cnt->tx_padding_in_tail);
#endif /* WLCNT */
		} else if (totlen + pad < max_agg_bytes) {
			/* next try using headroom -- wait til next pkt to take care of padding */
			amsduprio->headroom_pad_need = pad;
		} else {
			WL_AMSDU(("%s: terminate A-MSDU for tailroom/aggmax\n", __FUNCTION__));
			amsduprio->amsdu_agg_sframes++;
			amsduprio->amsdu_agg_bytes = totlen;
			wlc_amsdu_agg_close(ami, scb, tid);
#ifdef WLCNT
			if (totlen + pad < max_agg_bytes) {
				WLCNTINCR(ami->cnt->agg_stop_len);
			} else {
				WLCNTINCR(ami->cnt->agg_stop_tailroom);
			}
#endif /* WLCNT */
			return (TRUE);
		}
	}
#ifdef WLCNT
	else {
		WLCNTINCR(ami->cnt->tx_padding_no_pad);
	}
#endif /* WLCNT */
	/* END: pad current */
	/* sync up agg counter */
	WL_AMSDU(("%s: add one more frame len %d pad %d\n", __FUNCTION__, len, pad));
	ASSERT(totlen == (pkttotlen(osh, ptid) - ETHER_HDR_LEN));
	amsduprio->amsdu_agg_sframes++;
	amsduprio->amsdu_agg_bytes = totlen;
	return (TRUE);
}

void
wlc_amsdu_agg_flush(amsdu_info_t *ami)
{
	wlc_info_t *wlc;
	uint i;
	struct scb *scb;
	struct scb_iter scbiter;
	scb_amsdu_t *scb_ami;
	amsdu_scb_prio *amsduprio;

	if (!AMSDU_TX_SUPPORT(ami->pub))
		return;

	WL_AMSDU(("wlc_amsdu_agg_flush\n"));

	wlc = ami->wlc;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		for (i = 0; i < NUMPRIO; i++) {
			scb_ami = SCB_AMSDU_CUBBY(ami, scb);
			amsduprio = &scb_ami->prio[i];

			if (amsduprio->amsdu_agg_p)
				PKTFREE(ami->wlc->osh, scb_ami->prio[i].amsdu_agg_p, TRUE);

			amsduprio->amsdu_agg_p = NULL;
			amsduprio->amsdu_agg_ptail = NULL;
			amsduprio->amsdu_agg_sframes = 0;
			amsduprio->amsdu_agg_bytes = 0;
			amsduprio->amsdu_agg_padlast = 0;
			amsduprio->headroom_pad_need = 0;
		}
	}
}

#ifdef PROP_TXSTATUS
void
wlc_amsdu_flush_flowid_pkts(amsdu_info_t *ami, struct scb *scb, uint16 flowid)
{
	int i;
	scb_amsdu_t *scb_ami;
	amsdu_scb_prio *amsduprio;

	if (!AMSDU_TX_SUPPORT(ami->pub))
		return;

	WL_AMSDU(("wlc_amsdu_agg_flush_tid\n"));

	scb_ami = SCB_AMSDU_CUBBY(ami, scb);

	for (i = 0; i < NUMPRIO; i++) {
		amsduprio = &scb_ami->prio[i];

		if (amsduprio->amsdu_agg_p &&
			flowid == PKTFRAGFLOWRINGID(ami->wlc->osh, amsduprio->amsdu_agg_p)) {
			PKTFREE(ami->wlc->osh, amsduprio->amsdu_agg_p, TRUE);
			amsduprio->amsdu_agg_p = NULL;
			amsduprio->amsdu_agg_ptail = NULL;
			amsduprio->amsdu_agg_sframes = 0;
			amsduprio->amsdu_agg_bytes = 0;
			amsduprio->amsdu_agg_padlast = 0;
			amsduprio->headroom_pad_need = 0;
		}
	}
}
#endif /* PROP_TXSTATUS */

#ifdef WLOVERTHRUSTER
/* Identify TCP ACK frames to skip AMSDU agg */
static bool
wlc_amsdu_is_tcp_ack(amsdu_info_t *ami, void *p)
{
	uint8 *ip_header;
	uint8 *tcp_header;
	uint32 eth_len;
	uint32 ip_hdr_len;
	uint32 ip_len;
	uint32 pktlen;
	uint16 ethtype;
	wlc_info_t *wlc = ami->wlc;
	osl_t *osh = wlc->osh;

	pktlen = pkttotlen(osh, p);

	/* make sure we have enough room for a minimal IP + TCP header */
	if (pktlen < (ETHER_HDR_LEN +
	              DOT11_LLC_SNAP_HDR_LEN +
	              IPV4_MIN_HEADER_LEN +
	              TCP_MIN_HEADER_LEN)) {
		return FALSE;
	}

	/* find length of ether payload */
	eth_len = pktlen - (ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN);

	/* bail out early if pkt is too big for an ACK
	 *
	 * A TCP ACK has only TCP header and no data.  Max size for both IP header and TCP
	 * header is 15 words, 60 bytes. So if the ether payload is more than 120 bytes, we can't
	 * possibly have a TCP ACK. This test optimizes an early exit for MTU sized TCP.
	 */
	if (eth_len > 120) {
		return FALSE;
	}

	ethtype = wlc_sdu_etype(wlc, p);
	if (ethtype != ntoh16(ETHER_TYPE_IP)) {
		return FALSE;
	}

	/* find protocol headers and actual IP lengths */
	ip_header = wlc_sdu_data(wlc, p);
	ip_hdr_len = IPV4_HLEN(ip_header);
	ip_len = ntoh16_ua(&ip_header[IPV4_PKTLEN_OFFSET]);

	/* check for IP VER4 and carrying TCP */
	if (IP_VER(ip_header) != IP_VER_4 ||
	    IPV4_PROT(ip_header) != IP_PROT_TCP) {
		return FALSE;
	}

	/* verify pkt len in case of ip hdr has options */
	if (eth_len < ip_hdr_len + TCP_MIN_HEADER_LEN) {
		return FALSE;
	}

	tcp_header = ip_header + ip_hdr_len;

	/* fail if no TCP ACK flag or payload bytes present
	 * (payload bytes are present if IP len is not eq to IP header + TCP header)
	 */
	if ((tcp_header[TCP_FLAGS_OFFSET] & TCP_FLAG_ACK) == 0 ||
	    ip_len != ip_hdr_len + 4 * TCP_HDRLEN(tcp_header[TCP_HLEN_OFFSET])) {
		return FALSE;
	}

	return TRUE;
}
#endif /* WLOVERTHRUSTER */

/* Return the transmit packets held by AMSDU */
static uint
wlc_amsdu_txpktcnt(void *ctx)
{
	amsdu_info_t *ami = (amsdu_info_t *)ctx;
	uint i;
	scb_amsdu_t *scb_ami;
	int pktcnt = 0;
	struct scb_iter scbiter;
	wlc_info_t *wlc = ami->wlc;
	struct scb *scb;

	FOREACHSCB(wlc->scbstate, &scbiter, scb)
		if (SCB_AMSDU(scb)) {
			scb_ami = SCB_AMSDU_CUBBY(ami, scb);
			for (i = 0; i < NUMPRIO; i++) {
				if (scb_ami->prio[i].amsdu_agg_p)
					pktcnt++;
			}
		}

	return pktcnt;
}

static void
wlc_amsdu_scb_deactive(void *ctx, struct scb *scb)
{
	amsdu_info_t *ami;
	uint i;
	scb_amsdu_t *scb_ami;

	WL_AMSDU(("wlc_amsdu_scb_deactive scb %p\n", scb));

	ami = (amsdu_info_t *)ctx;
	scb_ami = SCB_AMSDU_CUBBY(ami, scb);
	for (i = 0; i < NUMPRIO; i++) {

		if (scb_ami->prio[i].amsdu_agg_p)
			PKTFREE(ami->wlc->osh, scb_ami->prio[i].amsdu_agg_p, TRUE);

		scb_ami->prio[i].amsdu_agg_p = NULL;
		scb_ami->prio[i].amsdu_agg_ptail = NULL;
		scb_ami->prio[i].amsdu_agg_sframes = 0;
		scb_ami->prio[i].amsdu_agg_bytes = 0;
		scb_ami->prio[i].amsdu_agg_padlast = 0;
	}
}

#endif /* WLAMSDU_TX */

/*
We should not come here typically!!!!
if we are here indicates we received a corrupted packet
which is tagged as AMSDU by the ucode. so flushing invalid
AMSDU chain. When anything other than AMSDU is received when
AMSDU state is not idle, flush the collected
intermediate amsdu packets
*/
void BCMFASTPATH
wlc_amsdu_flush(amsdu_info_t *ami)
{
	int fifo;
	amsdu_deagg_t *deagg;

	for (fifo = 0; fifo < RX_FIFO_NUMBER; fifo++) {
		deagg = &ami->amsdu_deagg[fifo];
		if (deagg->amsdu_deagg_state != WLC_AMSDU_DEAGG_IDLE)
			wlc_amsdu_deagg_flush(ami, fifo);
	}
}

/* return FALSE if filter failed
 *   caller needs to toss all buffered A-MSDUs and p
 *   Enhancement: in case of out of sequences, try to restart to
 *     deal with lost of last MSDU, which can occur frequently due to fcs error
 *   Assumes receive status is in host byte order at this point.
 *   PKTDATA points to start of receive descriptor when called.
 */
void * BCMFASTPATH
wlc_recvamsdu(amsdu_info_t *ami, wlc_d11rxhdr_t *wrxh, void *p, uint16 *padp, bool chained_sendup)
{
	osl_t *osh;
	amsdu_deagg_t *deagg;
	uint aggtype;
	int fifo;
	d11rxhdrshort_t *srxh = NULL;   /* short receive header. first and intermediate frags */
	bool pad_present;               /* TRUE if headroom is padded for alignment */
	uint16 pad = 0;                 /* Number of bytes of pad */

	/* packet length starting at 802.11 mac header (first frag) or eth header (others) */
	uint32 pktlen;

	uint32 pktlen_w_plcp;           /* packet length starting at PLCP */
	struct dot11_header *h;
#ifdef BCMDBG
	int msdu_cnt = -1;              /* just used for debug */
#endif // endif

	osh = ami->pub->osh;

	ASSERT(padp != NULL);

	if ((D11REV_GE(ami->wlc->pub->corerev, 64)) &&
	    (wrxh->rxhdr.dma_flags & RXS_SHORT_MASK)) {
		srxh = (d11rxhdrshort_t*) &wrxh->rxhdr;
		aggtype = (srxh->mrxs & RXSS_AGGTYPE_MASK) >> RXSS_AGGTYPE_SHIFT;
		pad_present = ((srxh->mrxs & RXSS_PBPRES) != 0);
#ifdef BCMDBG
		msdu_cnt = (srxh->mrxs & RXSS_MSDU_CNT_MASK) >> RXSS_MSDU_CNT_SHIFT;
#endif  /* BCMDBG */
	} else {
		aggtype = (wrxh->rxhdr.RxStatus2 & RXS_AGGTYPE_MASK) >> RXS_AGGTYPE_SHIFT;
		pad_present = ((wrxh->rxhdr.RxStatus1 & RXS_PBPRES) != 0);
	}
	if (pad_present) {
		pad = 2;
	}

	/* PKTDATA points to rxh. Get length of packet w/o rxh, but incl plcp */
	pktlen_w_plcp = PKTLEN(osh, p) - (ami->wlc->hwrxoff + pad);

	/* Packet length w/o PLCP */
	pktlen = pktlen_w_plcp - D11_PHY_HDR_LEN;

#ifdef DONGLEBUILD
	/* XXX Manage memory pressure by right sizing the packet since there
	 * can be lots of subframes that are going to be chained together
	 * pending the reception of the final subframe with its FCS for
	 * MPDU.  Note there is a danger of a "low memory deadlock" if you
	 * exhaust the available supply of packets by chaining subframes and
	 * as result you can't rxfill the wlan rx fifo and thus receive the
	 * final AMSDU subframe or even another MPDU to invoke a AMSDU cleanup
	 * action.  Also you might be surprised this can happen, to the extreme,
	 * if you receive a normal, but corrupt, MPDU which looks like a ASMDU
	 * to ucode because it releases frames to the FIFO/driver before it sees
	 * the CRC, i.e. ucode AMSDU deagg.  A misinterpted MPDU with lots of
	 * zero data can look like a large number of 16 byte minimum ASMDU
	 * subframes.  This is more than a theory, this actually happened repeatedly
	 * in open, congested air.
	 */
	if (PKTISRXFRAG(osh, p)) {
		/* for splitRX enabled case, pkt clonning is not valid */
		/* since part of the packet is in host */
	} else
	{
		uint headroom;
		void *dup_p;
		wlc_info_t *wlc = ami->wlc;
		uint16 pkt_len = 0;          /* packet length, including rxh */

		/* Make virtually an identical copy of the original packet with the same
		 * headroom and data.  Only the tailroom will be diffent, i.e the packet
		 * is right sized.
		 *
		 * What about making sure we have the same alignment if necessary?
		 * If you can't get a "right size" packets,just continue
		 * with the current one. You don't have much choice
		 * because the rest of the AMSDU could be in the
		 * rx dma ring and/or FIFO and be ack'ed already by ucode.
		 */
		pkt_len = PKTLEN(osh, p);

		if (pkt_len < wlc->pub->tunables->amsdu_resize_buflen) {
			headroom = PKTHEADROOM(osh, p);
			if ((dup_p = PKTGET(osh, headroom + pkt_len,
				FALSE)) != NULL) {
				PKTPULL(osh, dup_p, headroom);
				bcopy(PKTDATA(osh, p)-headroom,
				  PKTDATA(osh, dup_p)- headroom,
				  PKTLEN(osh, p)+headroom);
				PKTFREE(osh, p, FALSE);
				p = dup_p;
			}
		}
	}
#endif /* DONGLEBUILD */
	fifo = wrxh->rxhdr.fifo;
	ASSERT((fifo < RX_FIFO_NUMBER));
	deagg = &ami->amsdu_deagg[fifo];

	WLCNTINCR(ami->cnt->deagg_msdu);

	WL_AMSDU(("wlc_recvamsdu: aggtype %d, msdu count %d\n", aggtype, msdu_cnt));

	h = (struct dot11_header *)(PKTDATA(osh, p) + ami->wlc->hwrxoff + pad + D11_PHY_HDR_LEN);

	switch (aggtype) {
	case RXS_AMSDU_FIRST:
		/* PKTDATA starts with PLCP */
		if (deagg->amsdu_deagg_state != WLC_AMSDU_DEAGG_IDLE) {
			WL_AMSDU(("wlc_recvamsdu: wrong A-MSDU deagg sequence, cur_state=%d\n",
				deagg->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			wlc_amsdu_deagg_flush(ami, fifo);
			/* keep this valid one and reset to improve throughput */
		}

		deagg->amsdu_deagg_state = WLC_AMSDU_DEAGG_FIRST;

		/* Store the frontpad value of the first subframe */
		deagg->first_pad = *padp;

		if (!wlc_amsdu_deagg_open(ami, fifo, p, h, pktlen_w_plcp)) {
			goto abort;
		}

		WL_AMSDU(("wlc_recvamsdu: first A-MSDU buffer\n"));
		break;

	case RXS_AMSDU_INTERMEDIATE:
		/* PKTDATA starts with subframe header */
		if (deagg->amsdu_deagg_state != WLC_AMSDU_DEAGG_FIRST) {
			WL_AMSDU(("%s: wrong A-MSDU deagg sequence, cur_state=%d\n",
				__FUNCTION__, deagg->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			goto abort;
		}

		/* intermediate frames should have 2 byte padding if wlc->hwrxoff is aligned
		* on mod 4 address
		*/
		if ((ami->wlc->hwrxoff % 4) == 0) {
			if (!pad_present) {
				ASSERT(pad_present);
				goto abort;
			}
		} else {
			if (pad_present) {
				ASSERT(!pad_present);
				goto abort;
			}
		}

		if ((pktlen) <= ETHER_HDR_LEN) {
			WL_AMSDU(("%s: rxrunt\n", __FUNCTION__));
			WLCNTINCR(ami->pub->_cnt->rxrunt);
			goto abort;
		}

		ASSERT(deagg->amsdu_deagg_ptail);
		PKTSETNEXT(osh, deagg->amsdu_deagg_ptail, p);
		deagg->amsdu_deagg_ptail = p;
		WL_AMSDU(("wlc_recvamsdu:   mid A-MSDU buffer\n"));
		break;

	case RXS_AMSDU_LAST:
		/* PKTDATA starts with last subframe header */
		if (deagg->amsdu_deagg_state != WLC_AMSDU_DEAGG_FIRST) {
			WL_AMSDU(("%s: wrong A-MSDU deagg sequence, cur_state=%d\n",
				__FUNCTION__, deagg->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			goto abort;
		}

		deagg->amsdu_deagg_state = WLC_AMSDU_DEAGG_LAST;

		/* last frame should have 2 byte padding if wlc->hwrxoff is aligned
		* on mod 4 address
		*/
		if ((ami->wlc->hwrxoff % 4) == 0) {
			if (!pad_present) {
				goto abort;
			}
		} else {
			if (pad_present) {
				goto abort;
			}
		}

		if ((pktlen) < (ETHER_HDR_LEN + DOT11_FCS_LEN)) {
			WL_AMSDU(("%s: rxrunt\n", __FUNCTION__));
			WLCNTINCR(ami->pub->_cnt->rxrunt);
			goto abort;
		}

		ASSERT(deagg->amsdu_deagg_ptail);
		PKTSETNEXT(osh, deagg->amsdu_deagg_ptail, p);
		deagg->amsdu_deagg_ptail = p;
		WL_AMSDU(("wlc_recvamsdu: last A-MSDU buffer\n"));
		break;

	case RXS_AMSDU_N_ONE:
		/* this frame IS AMSDU, checked by caller */

		if (deagg->amsdu_deagg_state != WLC_AMSDU_DEAGG_IDLE) {
			WL_AMSDU(("wlc_recvamsdu: wrong A-MSDU deagg sequence, cur_state=%d\n",
				deagg->amsdu_deagg_state));
			WLCNTINCR(ami->cnt->deagg_wrongseq);
			wlc_amsdu_deagg_flush(ami, fifo);

			/* keep this valid one and reset to improve throughput */
		}

		ASSERT((deagg->amsdu_deagg_p == NULL) && (deagg->amsdu_deagg_ptail == NULL));
		deagg->amsdu_deagg_state = WLC_AMSDU_DEAGG_LAST;

		/* Store the frontpad value of this single subframe */
		deagg->first_pad = *padp;

		if (!wlc_amsdu_deagg_open(ami, fifo, p, h, pktlen_w_plcp)) {
			goto abort;
		}

		break;

	default:
		/* can't be here */
		ASSERT(0);
		goto abort;
	}

	/* Note that pkttotlen now includes the length of the rxh for each frag */
	WL_AMSDU(("wlc_recvamsdu: add one more A-MSDU buffer %d bytes, accumulated %d bytes\n",
	         pktlen_w_plcp, pkttotlen(osh, deagg->amsdu_deagg_p)));

	if (deagg->amsdu_deagg_state == WLC_AMSDU_DEAGG_LAST) {
		void *pp = deagg->amsdu_deagg_p;
#ifdef WL11K
		uint tot_len = pkttotlen(osh, pp);
#endif // endif
		deagg->amsdu_deagg_p = deagg->amsdu_deagg_ptail = NULL;
		deagg->amsdu_deagg_state = WLC_AMSDU_DEAGG_IDLE;

		/* ucode/hw deagg happened */
		WLPKTTAG(pp)->flags |= WLF_HWAMSDU;

		/* First frame has fully defined Receive Frame Header,
		 * handle it to normal MPDU process.
		 */
		WLCNTINCR(ami->pub->_cnt->rxfrag);
		WLCNTINCR(ami->cnt->deagg_amsdu);
#ifdef WL11K
		WLCNTADD(ami->cnt->deagg_amsdu_bytes_l, tot_len);
		if (ami->cnt->deagg_amsdu_bytes_l < tot_len)
			WLCNTINCR(ami->cnt->deagg_amsdu_bytes_h);
#endif // endif
#ifdef WL_MU_RX
		/* Update the murate to the plcp
		 * last rxhdr has murate information
		 * plcp is in the first packet
		 */
		if (MU_RX_ENAB(ami->wlc)) {
			wlc_d11rxhdr_t *frag_wrxh;   /* rx status of an AMSDU frag in chain */
			d11rxhdr_t *rxh;
			uchar *plcp;
			uint16 pad_cnt;
			frag_wrxh = (wlc_d11rxhdr_t *)PKTDATA(osh, pp);
			rxh = &frag_wrxh->rxhdr;

			/* Check for short or long format */
			if (rxh->dma_flags & RXS_SHORT_MASK) {
				/* short rx status received */
				srxh = (d11rxhdrshort_t *)rxh;
				pad_cnt = (srxh->mrxs & RXSS_PBPRES) ? 2 : 0;
			} else {
				/* long rx status received */
				pad_cnt = ((rxh->RxStatus1 & RXS_PBPRES) ? 2 : 0);
			}

			plcp = (uchar *)(PKTDATA(ami->wlc->osh, pp) + ami->wlc->hwrxoff + pad_cnt);

			wlc_bmac_upd_murate(ami->wlc, &wrxh->rxhdr, plcp);
		}
#endif /* WL_MU_RX */
#if defined(PKTC) || defined(PKTC_DONGLE)
		/* if chained sendup, return back head pkt and front padding of first sub-frame */
		/* if unchained wlc_recvdata takes pkt till bus layer */
		if (chained_sendup == TRUE) {
			*padp = deagg->first_pad;
			deagg->first_pad = 0;
			return (pp);
		} else
#endif // endif
		{
			/* Strip rxh from all amsdu frags in amsdu chain before send up */
			void *np = pp;
			uint16 pad_cnt;
			wlc_d11rxhdr_t *frag_wrxh;   /* rx status of an AMSDU frag in chain */
			d11rxhdr_t *rxh;
			while (np) {
				frag_wrxh = (wlc_d11rxhdr_t*) PKTDATA(osh, np);
				rxh = &frag_wrxh->rxhdr;
				if (D11REV_GE(ami->pub->corerev, 64) &&
				    (rxh->dma_flags & RXS_SHORT_MASK)) {
					/* short rx status received */
					srxh = (d11rxhdrshort_t*) rxh;
					pad_cnt = (srxh->mrxs & RXSS_PBPRES) ? 2 : 0;
				} else {
					/* long rx status received */
					pad_cnt = ((rxh->RxStatus1 & RXS_PBPRES) ? 2 : 0);
				}
				PKTPULL(osh, np, ami->wlc->hwrxoff + pad_cnt);
				np = PKTNEXT(osh, np);
			}
			wlc_recvdata(ami->wlc, ami->pub->osh, wrxh, pp);
		}
		deagg->first_pad = 0;
	}

	/* all other cases needs no more action, just return */
	return  NULL;

abort:
	wlc_amsdu_deagg_flush(ami, fifo);
	PKTFREE(osh, p, FALSE);
	return  NULL;
}

/* return FALSE if A-MSDU verification failed */
static bool BCMFASTPATH
wlc_amsdu_deagg_verify(amsdu_info_t *ami, uint16 fc, void *h)
{
	bool is_wds;
	uint16 *pqos;
	uint16 qoscontrol;

	/* it doesn't make sense to aggregate other type pkts, toss them */
	if ((fc & FC_KIND_MASK) != FC_QOS_DATA) {
		WL_AMSDU(("wlc_amsdu_deagg_verify fail: fc 0x%x is not QoS data type\n", fc));
		return FALSE;
	}

	is_wds = ((fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
	pqos = (uint16*)((uchar*)h + (is_wds ? DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN));
	qoscontrol = ltoh16_ua(pqos);

	if (qoscontrol & QOS_AMSDU_MASK)
		return TRUE;

	WL_ERROR(("%s fail: qos field 0x%x\n", __FUNCTION__, *pqos));
	return FALSE;
}

/* Start a new AMSDU receive chain. Verifies that the frame is a data frame
 * with QoS field indicating AMSDU, and that the frame is long enough to
 * include PLCP, 802.11 mac header, QoS field, and AMSDU subframe header.
 * Inputs:
 *   ami    - AMSDU state
 *   fifo   - queue on which current frame was received
 *   p      - first frag in a sequence of AMSDU frags. PKTDATA(p) points to
 *            start of receive descriptor
 *   h      - start of ethernet header in received frame
 *   pktlen - frame length, starting at PLCP
 *
 * Returns:
 *   TRUE if new AMSDU chain is started
 *   FALSE otherwise
 */
static bool BCMFASTPATH
wlc_amsdu_deagg_open(amsdu_info_t *ami, int fifo, void *p, struct dot11_header *h, uint32 pktlen)
{
	osl_t *osh = ami->pub->osh;
	amsdu_deagg_t *deagg = &ami->amsdu_deagg[fifo];
	uint16 fc;

	BCM_REFERENCE(osh);

	if (pktlen < D11_PHY_HDR_LEN + DOT11_MAC_HDR_LEN + DOT11_QOS_LEN + ETHER_HDR_LEN) {
		WL_AMSDU(("%s: rxrunt\n", __FUNCTION__));
		WLCNTINCR(ami->pub->_cnt->rxrunt);
		goto fail;
	}

	fc = ltoh16(h->fc);

	if (!wlc_amsdu_deagg_verify(ami, fc, h)) {
		WL_AMSDU(("%s: AMSDU verification failed, toss\n", __FUNCTION__));
		WLCNTINCR(ami->cnt->deagg_badfmt);
		goto fail;
	}

	/* explicitly test bad src address to avoid sending bad deauth */
	if ((ETHER_ISNULLADDR(&h->a2) || ETHER_ISMULTI(&h->a2))) {
		WL_AMSDU(("%s: wrong address 2\n", __FUNCTION__));
		WLCNTINCR(ami->pub->_cnt->rxbadsrcmac);
		goto fail;
	}

	deagg->amsdu_deagg_p = p;
	deagg->amsdu_deagg_ptail = p;
	return TRUE;

fail:
	WLCNTINCR(ami->cnt->deagg_openfail);
	return FALSE;
}

static void BCMFASTPATH
wlc_amsdu_deagg_flush(amsdu_info_t *ami, int fifo)
{
	amsdu_deagg_t *deagg = &ami->amsdu_deagg[fifo];
	WL_AMSDU(("%s\n", __FUNCTION__));

	if (deagg->amsdu_deagg_p)
		PKTFREE(ami->pub->osh, deagg->amsdu_deagg_p, FALSE);

	deagg->first_pad = 0;
	deagg->amsdu_deagg_state = WLC_AMSDU_DEAGG_IDLE;
	deagg->amsdu_deagg_p = deagg->amsdu_deagg_ptail = NULL;
#ifdef WLCNT
	WLCNTINCR(ami->cnt->deagg_flush);
#endif /* WLCNT */
}

static void
wlc_amsdu_to_dot11(amsdu_info_t *ami, struct scb *scb, struct dot11_header *hdr, void *pkt)
{
	osl_t *osh;

	/* ptr to 802.3 or eh in incoming pkt */
	struct ether_header *eh;
	struct dot11_llc_snap_header *lsh;

	/* ptr to 802.3 or eh in new pkt */
	struct ether_header *neh;
	struct dot11_header *phdr;

	wlc_bsscfg_t *cfg = scb->bsscfg;
	osh = ami->pub->osh;

	/*
	* If we discover an ethernet header, replace it by an
	* 802.3 hdr + SNAP header.
	*/
	eh = (struct ether_header *)PKTDATA(osh, pkt);

	if (ntoh16(eh->ether_type) > ETHER_MAX_LEN) {
		neh = (struct ether_header *)PKTPUSH(osh, pkt, DOT11_LLC_SNAP_HDR_LEN);

		/* Avoid constructing 802.3 header as optimization.
		 * 802.3 header(14 bytes) is going to be overwritten by the 802.11 header.
		 * This will save writing 14-bytes for every MSDU.
		 */

		/* Construct LLC SNAP header */
		lsh = (struct dot11_llc_snap_header *)
			((char *)neh + ETHER_HDR_LEN);
		lsh->dsap = 0xaa;
		lsh->ssap = 0xaa;
		lsh->ctl = 0x03;
		lsh->oui[0] = 0;
		lsh->oui[1] = 0;
		lsh->oui[2] = 0;
		/*
		* The snap type code is already in place, inherited
		* from the ethernet header that is now overlaid.
		*/
	}
	else {
		neh = (struct ether_header *)PKTDATA(osh, pkt);
	}

	if (BSSCFG_AP(cfg)) {
		/* Force the 802.11 a2 address to be
		* the ethernet source address
		*/
		bcopy((char *)neh->ether_shost,
			(char *)&hdr->a2, ETHER_ADDR_LEN);
	} else {
		if (!cfg->BSS) {
			/* Force the 802.11 a3 address to be
			* the ethernet source address
			* IBSS has BSS as a3, so leave a3 alone for win7+
			*/
			bcopy((char *)neh->ether_shost,
				(char *)&hdr->a3, ETHER_ADDR_LEN);
		}
	}

	/*
	 * Replace the 802.3 header, if present, by an 802.11 header
	 * The original 802.11 header was appended to the frame along
	 * with the receive data needed by microsoft.
	 */
	phdr = (struct dot11_header *)
		PKTPUSH(osh, pkt, DOT11_A3_HDR_LEN - ETHER_HDR_LEN);

	bcopy((char *)hdr, (char *)phdr, DOT11_A3_HDR_LEN);

	/*
	 * Clear all frame control bits except version, type,
	 * data subtype & from-ds/to-ds
	 */
	phdr->fc = htol16(ltoh16(phdr->fc) & (FC_FROMDS | FC_TODS | FC_TYPE_MASK |
		(FC_SUBTYPE_MASK & ~QOS_AMSDU_MASK) | FC_PVER_MASK));
}

/*
 * A-MSDU decomposition: break A-MSDU(chained buffer) to individual buffers
 *
 *    | 80211 MAC HEADER | subFrame 1 |
 *			               --> | subFrame 2 |
 *			                                 --> | subFrame 3... |
 * where, 80211 MAC header also includes QOS and/or IV fields
 *        f->pbody points to beginning of subFrame 1,
 *        f->totlen is the total body len(chained, after mac/qos/iv header) w/o icv and FCS
 *
 *        each subframe is in the form of | 8023hdr | body | pad |
 *                subframe other than the last one may have pad bytes
*/
void
wlc_amsdu_deagg_hw(amsdu_info_t *ami, struct scb *scb, struct wlc_frminfo *f)
{
	osl_t *osh;
	void *sf[MAX_RX_SUBFRAMES], *newpkt;
	struct ether_header *eh;
	uint16 body_offset, sflen = 0, len = 0;
	uint num_sf = 0, i;
	int resid;
	wlc_pkttag_t * pkttag = WLPKTTAG(f->p);
	struct dot11_header hdr_copy;

	if (WLEXTSTA_ENAB(ami->wlc->pub)) {
		/* Save the header before being overwritten */
		bcopy((char *)f->h, (char *)&hdr_copy, DOT11_A3_HDR_LEN);

		/* Assume f->h pointer is not valid any more */
		f->h = NULL;
	}

	ASSERT(pkttag->flags & WLF_HWAMSDU);
	osh = ami->pub->osh;

	/* strip mac header, move to start from A-MSDU body */
	body_offset = (uint)(f->pbody - (uchar*)PKTDATA(osh, f->p));
	PKTPULL(osh, f->p, body_offset);

	WL_AMSDU(("wlc_amsdu_deagg_hw: body_len(exclude icv and FCS) %d\n", f->totlen));

	resid = f->totlen;
	newpkt = f->p;
#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	WLCNTINCR(ami->amdbg->rx_length_histogram[
		MIN(PKTLEN(osh, f->p)/AMSDU_LENGTH_BIN_BYTES,
		AMSDU_LENGTH_BINS-1)]);
#endif /* BCMDBG */

	/* break chained AMSDU into N independent MSDU */
	while (newpkt != NULL) {
		/* there must be a limit to stop in order to prevent memory/stack overflow */
		if (num_sf >= MAX_RX_SUBFRAMES) {
			WL_ERROR(("%s: more than %d MSDUs !\n", __FUNCTION__, num_sf));
			break;
		}

		/* each subframe is 802.3 frame */
		eh = (struct ether_header*) PKTDATA(osh, newpkt);

		len = (uint16)(PKTLEN(osh, newpkt) + PKTFRAGUSEDLEN(osh, newpkt));

		sflen = ntoh16(eh->ether_type) + ETHER_HDR_LEN;

		if ((((uintptr)eh + (uint)ETHER_HDR_LEN) % 4)  != 0) {
			WL_ERROR(("%s: sf body is not 4 bytes aligned!\n", __FUNCTION__));
			WLCNTINCR(ami->cnt->deagg_badsfalign);
			goto toss;
		}

		/* last MSDU: has FCS, but no pad, other MSDU: has pad, but no FCS */
		if (len != (PKTNEXT(osh, newpkt) ? ROUNDUP(sflen, 4) : sflen)) {
			WL_AMSDU(("%s: len mismatch buflen %d sflen %d, sf %d\n",
				__FUNCTION__, len, sflen, num_sf));
			WLCNTINCR(ami->cnt->deagg_badsflen);
			goto toss;
		}

		/* strip trailing optional pad */
		if (PKTFRAGUSEDLEN(osh, newpkt)) {
			/* set total length to sflen */
			PKTSETFRAGUSEDLEN(osh, newpkt, (sflen - PKTLEN(osh, newpkt)));
		} else {
			PKTSETLEN(osh, newpkt, sflen);
		}

		if (WLEXTSTA_ENAB(ami->wlc->pub)) {
			/* convert 802.3 to 802.11 */
			wlc_amsdu_to_dot11(ami, scb, &hdr_copy, newpkt);

			if (f->p != newpkt) {
				wlc_pkttag_t * new_pkttag = WLPKTTAG(newpkt);

				new_pkttag->rxchannel = pkttag->rxchannel;
				new_pkttag->pktinfo.misc.rssi = pkttag->pktinfo.misc.rssi;
				new_pkttag->rspec = pkttag->rspec;
			}
		} else {
			/* convert 8023hdr to ethernet if necessary */
			wlc_8023_etherhdr(ami->wlc, osh, newpkt);
		}

		/* propogate prio, NO need to transfer other tags, it's plain stack packet now */
		PKTSETPRIO(newpkt, f->prio);

		WL_AMSDU(("wlc_amsdu_deagg_hw: deagg MSDU buffer %d, frame %d\n", len, sflen));

		sf[num_sf] = newpkt;
		num_sf++;
		newpkt = PKTNEXT(osh, newpkt);

		resid -= len;
	}

	if (resid != 0) {
		ASSERT(0);
		WLCNTINCR(ami->cnt->deagg_badtotlen);
		goto toss;
	}

	/* cut the chain: set PKTNEXT to NULL */
	for (i = 0; i < num_sf; i++)
		PKTSETNEXT(osh, sf[i], NULL);

	/* toss the remaining MSDU, which we couldn't handle */
	if (newpkt != NULL) {
		WL_ERROR(("%s: toss MSDUs > %d !\n", __FUNCTION__, num_sf));
		PKTFREE(osh, newpkt, FALSE);
	}

	/* sendup */
	for (i = 0; i < num_sf; i++) {
		struct ether_addr * ea;

		WL_AMSDU(("wlc_amsdu_deagg_hw: sendup subframe %d\n", i));

		if (WLEXTSTA_ENAB(ami->wlc->pub)) {
			ea = &hdr_copy.a1;
		} else {
			ea = (struct ether_addr *) PKTDATA(osh, sf[i]);
		}

		wlc_recvdata_sendup(ami->wlc, scb, f->wds, ea, sf[i]);
	}
	WL_AMSDU(("wlc_amsdu_deagg_hw: this A-MSDU has %d MSDU, done\n", num_sf));
#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	WLCNTINCR(ami->amdbg->rx_msdu_histogram[MIN(num_sf, AMSDU_RX_SUBFRAMES_BINS-1)]);
#endif /* BCMDBG */

#ifdef WLSCB_HISTO
	WLSCB_HISTO_RX_INC_RECENT(scb, num_sf);
#endif /* WLSCB_HISTO */

	return;

toss:
	/*
	 * toss the whole A-MSDU since we don't know where the error starts
	 *  e.g. a wrong subframe length for mid frame can slip through the ucode
	 *       and the only syptom may be the last MSDU frame has the mismatched length.
	 */
	for (i = 0; i < num_sf; i++)
		sf[i] = NULL;

#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	WLCNTINCR(ami->amdbg->rx_msdu_histogram[0]);
#endif /* BCMDBG */
	WL_AMSDU(("%s: tossing amsdu in deagg -- error seen\n", __FUNCTION__));
	PKTFREE(osh, f->p, FALSE);
}

#if defined(PKTC) || defined(PKTC_DONGLE)
int32 BCMFASTPATH
wlc_amsdu_pktc_deagg_hw(amsdu_info_t *ami, void **pp, wlc_rfc_t *rfc, uint16 *index,
                        bool *chained_sendup)
{
	osl_t *osh;
	wlc_info_t *wlc = ami->wlc;
	void *newpkt, *head, *tail, *tmp_next;
	struct ether_header *eh;
	uint16 sflen = 0, len = 0;
	uint num_sf = 0;
	int resid = 0;
	uint8 *da;

	osh = ami->pub->osh;
	newpkt = tail = head = *pp;
	resid = pkttotlen(osh, head) - DOT11_FCS_LEN;
#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	len = MIN(PKTLEN(osh, head)/AMSDU_LENGTH_BIN_BYTES, AMSDU_LENGTH_BINS-1);
	WLCNTINCR(ami->amdbg->rx_length_histogram[len]);
#endif // endif

	/* insert MSDUs in to current packet chain */
	while (newpkt != NULL) {
		/* strip off FCS in last MSDU */
		if (PKTNEXT(osh, newpkt) == NULL)
			PKTFRAG_TRIM_TAILBYTES(osh, newpkt, DOT11_FCS_LEN, TAIL_BYTES_TYPE_FCS);

		/* there must be a limit to stop in order to prevent memory/stack overflow */
		if (num_sf >= MAX_RX_SUBFRAMES) {
			WL_ERROR(("%s: more than %d MSDUs !\n", __FUNCTION__, num_sf));
			break;
		}

		/* Frame buffer still points to the start of the receive descriptor. For each
		 * MPDU in chain, move pointer past receive descriptor.
		 */
		if ((WLPKTTAG(newpkt)->flags & WLF_HWAMSDU) == 0) {
			wlc_d11rxhdr_t *wrxh;
			uint pad = 0;
			/* determine whether packet has 2-byte pad */
			d11rxhdrshort_t *srxh = NULL;   /* short receive header. */
			wrxh = (wlc_d11rxhdr_t*) PKTDATA(osh, newpkt);
			if ((D11REV_GE(wlc->pub->corerev, 64)) &&
			    (wrxh->rxhdr.dma_flags & RXS_SHORT_MASK)) {
				srxh = (d11rxhdrshort_t*) &wrxh->rxhdr;
				pad = ((srxh->mrxs & RXSS_PBPRES) != 0) ? 2 : 0;
			} else {
				pad = ((wrxh->rxhdr.RxStatus1 & RXS_PBPRES) != 0) ? 2 : 0;
			}
			PKTPULL(wlc->osh, newpkt, wlc->hwrxoff + pad);
			resid -= (wlc->hwrxoff + pad);
		}

		/* each subframe is 802.3 frame */
		eh = (struct ether_header *)PKTDATA(osh, newpkt);
		len = (uint16)PKTLEN(osh, newpkt) + (uint16)PKTFRAGUSEDLEN(osh, newpkt);

		sflen = NTOH16(eh->ether_type) + ETHER_HDR_LEN;

		if ((((uintptr)eh + (uint)ETHER_HDR_LEN) % 4) != 0) {
			WL_ERROR(("%s: sf body is not 4b aligned!\n", __FUNCTION__));
			WLCNTINCR(ami->cnt->deagg_badsfalign);
			goto toss;
		}

		/* last MSDU: has FCS, but no pad, other MSDU: has pad, but no FCS */
		if (len != (PKTNEXT(osh, newpkt) ? ROUNDUP(sflen, 4) : sflen)) {
			WL_AMSDU(("%s: len mismatch buflen %d sflen %d, sf %d\n",
				__FUNCTION__, len, sflen, num_sf));
			WLCNTINCR(ami->cnt->deagg_badsflen);
			goto toss;
		}

		/* strip trailing optional pad */
		if (PKTFRAGUSEDLEN(osh, newpkt)) {
			PKTSETFRAGUSEDLEN(osh, newpkt, (sflen - PKTLEN(osh, newpkt)));
		} else {
			PKTSETLEN(osh, newpkt, sflen);
		}

		/* convert 8023hdr to ethernet if necessary */
		wlc_8023_etherhdr(wlc, osh, newpkt);

		eh = (struct ether_header *)PKTDATA(osh, newpkt);
#ifdef PSTA
		if (BSSCFG_STA(rfc->bsscfg) &&
		    PSTA_IS_REPEATER(wlc) &&
		    TRUE)
			da = (uint8 *)&rfc->ds_ea;
		else
#endif // endif
			da = eh->ether_dhost;

		if (ETHER_ISNULLDEST(da))
			goto toss;

		if (*chained_sendup) {
			if (wlc->pktc_info->h_da == NULL)
				wlc->pktc_info->h_da = (struct ether_addr *)da;

			*chained_sendup = !ETHER_ISMULTI(da) &&
				!eacmp(wlc->pktc_info->h_da, da) &&
#if defined(HNDCTF) && !defined(BCM_GMAC3)
				CTF_HOTBRC_CMP(wlc->pub->brc_hot, da, rfc->wlif_dev) &&
#endif /* HNDCTF && ! BCM_GMAC3 */
#if defined(PKTC_TBL)
				PKTC_TBL_FN_CMP(wlc->pub->pktc_tbl, da, rfc->wlif_dev) &&
#endif // endif
				((eh->ether_type == HTON16(ETHER_TYPE_IP)) ||
				(eh->ether_type == HTON16(ETHER_TYPE_IPV6)));
		}

		WL_AMSDU(("%s: deagg MSDU buffer %d, frame %d\n",
		          __FUNCTION__, len, sflen));

		/* remove from AMSDU chain and insert in to MPDU chain. skip
		 * the head MSDU since it is already in chain.
		 */
		tmp_next = PKTNEXT(osh, newpkt);
		if (num_sf > 0) {
			/* remove */
			PKTSETNEXT(osh, head, tmp_next);
			PKTSETNEXT(osh, newpkt, NULL);
			/* insert */
			PKTSETCLINK(newpkt, PKTCLINK(tail));
			PKTSETCLINK(tail, newpkt);
			tail = newpkt;
			/* set prio */
			PKTSETPRIO(newpkt, PKTPRIO(head));
			WLPKTTAGSCBSET(newpkt, rfc->scb);
		}

		*pp = newpkt;
		PKTCADDLEN(head, len);
		PKTSETCHAINED(wlc->osh, newpkt);

		num_sf++;
		newpkt = tmp_next;
		resid -= len;
	}

	if (resid != 0) {
		ASSERT(0);
		WLCNTINCR(ami->cnt->deagg_badtotlen);
		goto toss;
	}

	/* toss the remaining MSDU, which we couldn't handle */
	if (newpkt != NULL) {
		WL_ERROR(("%s: toss MSDUs > %d !\n", __FUNCTION__, num_sf));
		PKTFREE(osh, newpkt, FALSE);
	}

	WL_AMSDU(("%s: this A-MSDU has %d MSDUs, done\n", __FUNCTION__, num_sf));
#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	WLCNTINCR(ami->amdbg->rx_msdu_histogram[MIN(num_sf, AMSDU_RX_SUBFRAMES_BINS-1)]);
#endif // endif

#ifdef WLSCB_HISTO
	WLSCB_HISTO_RX_INC_RECENT(rfc->scb, num_sf);
#endif /* WLSCB_HISTO */
	(*index) += num_sf;

	return BCME_OK;

toss:
	/*
	 * toss the whole A-MSDU since we don't know where the error starts
	 *  e.g. a wrong subframe length for mid frame can slip through the ucode
	 *       and the only syptom may be the last MSDU frame has the mismatched length.
	 */
	if (PKTNEXT(osh, head)) {
		PKTFREE(osh, PKTNEXT(osh, head), FALSE);
		PKTSETNEXT(osh, head, NULL);
	}

	if (head != tail) {
		while ((tmp_next = PKTCLINK(head)) != NULL) {
			PKTSETCLINK(head, PKTCLINK(tmp_next));
			PKTSETCLINK(tmp_next, NULL);
			PKTCLRCHAINED(wlc->osh, tmp_next);
			PKTFREE(osh, tmp_next, FALSE);
			if (tmp_next == tail) {
				/* assign *pp to head so that wlc_sendup_chain
				 * does not try to free tmp_next again
				 */
				*pp = head;
				break;
			}
		}
	}

#if defined(BCMDBG) || defined(BCMDBG_AMSDU)
	WLCNTINCR(ami->amdbg->rx_msdu_histogram[0]);
#endif // endif
	WL_AMSDU(("%s: tossing amsdu in deagg -- error seen\n", __FUNCTION__));
	return BCME_ERROR;
}
#endif /* PKTC */

#ifdef WLAMSDU_SWDEAGG
/* A-MSDU sw deaggragation - for testing only due to lower performance to align payload.
 *
 *    | 80211 MAC HEADER | subFrame 1 | subFrame 2 | subFrame 3 | ... |
 * where, 80211 MAC header also includes WDS and/or QOS and/or IV fields
 *        f->pbody points to beginning of subFrame 1,
 *        f->body_len is the total length of all sub frames, exclude ICV and/or FCS
 *
 *        each subframe is in the form of | 8023hdr | body | pad |
 *                subframe other than the last one may have pad bytes
*/
/*
 * Note: This headroom calculation comes out to 10 byte.
 * Arithmetically, this amounts to two 4-byte blocks plus
 * 2. 2 bytes are needed anyway to achieve 4-byte alignment.
 */
#define HEADROOM  DOT11_A3_HDR_LEN-ETHER_HDR_LEN
void
wlc_amsdu_deagg_sw(amsdu_info_t *ami, struct scb *scb, struct wlc_frminfo *f)
{
	osl_t *osh;
	struct ether_header *eh;
	struct ether_addr *ea;
	uchar *data;
	void *newpkt;
	int resid;
	uint16 body_offset, sflen, len;
	wlc_pkttag_t * pkttag = WLPKTTAG(f->p);

	struct dot11_header hdr_copy;

	if (WLEXTSTA_ENAB(ami->wlc->pub)) {
		/* Save the header before being overwritten */
		bcopy((char *)f->h, (char *)&hdr_copy, DOT11_A3_HDR_LEN);

		/* Assume f->h pointer is not valid any more */
		f->h = NULL;
	}

	osh = ami->pub->osh;

	/* all in one buffer, no chain */
	ASSERT(PKTNEXT(osh, f->p) == NULL);

	/* throw away mac header all together, start from A-MSDU body */
	body_offset = (uint)(f->pbody - (uchar*)PKTDATA(osh, f->p));
	PKTPULL(osh, f->p, body_offset);
	ASSERT(f->pbody == (uchar *)PKTDATA(osh, f->p));
	data = f->pbody;
	resid = f->totlen;

	WL_AMSDU(("wlc_amsdu_deagg_sw: body_len(exclude ICV and FCS) %d\n", resid));

	/* loop over orig unpacking and copying frames out into new packet buffers */
	while (resid > 0) {
		if (resid < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN)
			break;

		/* each subframe is 802.3 frame */
		eh = (struct ether_header*) data;
		sflen = ntoh16(eh->ether_type) + ETHER_HDR_LEN;

		/* swdeagg is mainly for testing, not intended to support big buffer.
		 *  there are also the 2K hard limit for rx buffer we posted.
		 *  We can increase to 4K, but it wastes memory and A-MSDU often goes
		 *  up to 8K. HW deagg is the preferred way to handle large A-MSDU.
		 */
		if (sflen > ETHER_MAX_DATA + DOT11_LLC_SNAP_HDR_LEN + ETHER_HDR_LEN) {
			WL_ERROR(("%s: unexpected long pkt, toss!", __FUNCTION__));
			WLCNTINCR(ami->cnt->deagg_swdeagglong);
			goto done;
		}

		/*
		 * Alloc new rx packet buffer, add headroom bytes to
		 * achieve 4-byte alignment and to allow for changing
		 * the hdr from 802.3 to 802.11 (EXT_STA only)
		 */
		if ((newpkt = PKTGET(osh, sflen + HEADROOM, FALSE)) == NULL) {
			WL_ERROR(("wl: %s: pktget error\n", __FUNCTION__));
			WLCNTINCR(ami->pub->_cnt->rxnobuf);
			goto done;
		}
		PKTPULL(osh, newpkt, HEADROOM);
		/* copy next frame into new rx packet buffer, pad bytes are dropped */
		bcopy(data, PKTDATA(osh, newpkt), sflen);
		PKTSETLEN(osh, newpkt, sflen);

		if (WLEXTSTA_ENAB(ami->wlc->pub)) {
			/* convert 802.3 to 802.11 */
			wlc_amsdu_to_dot11(ami, scb, &hdr_copy, newpkt);
			if (f->p != newpkt) {
				wlc_pkttag_t * new_pkttag = WLPKTTAG(newpkt);

				new_pkttag->rxchannel = pkttag->rxchannel;
				new_pkttag->pktinfo.misc.rssi = pkttag->pktinfo.misc.rssi;
				new_pkttag->rspec = pkttag->rspec;
			}

		} else {
			/* convert 8023hdr to ethernet if necessary */
			wlc_8023_etherhdr(ami->wlc, osh, newpkt);
		}

		if (WLEXTSTA_ENAB(ami->wlc->pub)) {
			ea = &hdr_copy.a1;
		}
		else {
			ea = (struct ether_addr *) PKTDATA(osh, newpkt);
		}

		/* transfer prio, NO need to transfer other tags, it's plain stack packet now */
		PKTSETPRIO(newpkt, f->prio);

		wlc_recvdata_sendup(ami->wlc, scb, f->wds, ea, newpkt);

		/* account padding bytes */
		len = ROUNDUP(sflen, 4);

		WL_AMSDU(("wlc_amsdu_deagg_sw: deagg one frame datalen=%d, buflen %d\n",
			sflen, len));

		data += len;
		resid -= len;

		/* last MSDU doesn't have pad, may overcount */
		if (resid < -4) {
			WL_ERROR(("wl: %s: error: resid %d\n", __FUNCTION__, resid));
			break;
		}
	}

done:
	/* all data are copied, free the original amsdu frame */
	PKTFREE(osh, f->p, FALSE);
}
#endif /* WLAMSDU_SWDEAGG */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMSDU)
static int
wlc_amsdu_dump(amsdu_info_t *ami, struct bcmstrbuf *b)
{
	uint i, last;
	uint32 total = 0;

	bcm_bprintf(b, "amsdu_agg_block %d amsdu_rx_mtu %d rcvfifo_limit %d\n",
		ami->amsdu_agg_block, ami->amsdu_rx_mtu, ami->mac_rcvfifo_limit);
	bcm_bprintf(b, "amsdu_rxcap_big %d fifo_lowm %d fifo_hiwm %d\n",
		ami->amsdu_rxcap_big, ami->fifo_lowm, ami->fifo_hiwm);

	for (i = 0; i < RX_FIFO_NUMBER; i++) {
		amsdu_deagg_t *deagg = &ami->amsdu_deagg[i];
		bcm_bprintf(b, "%d amsdu_deagg_state %d\n", i, deagg->amsdu_deagg_state);
	}

	for (i = 0; i < NUMPRIO; i++) {
		bcm_bprintf(b, "%d agg_allowprio %d agg_bytes_limit %d agg_sf_limit %d\n",
			i, ami->amsdu_agg_allowprio[i], ami->amsdu_agg_bytes_limit[i],
			ami->amsdu_agg_sframes_limit[i]);
	}

#ifdef WLCNT
	wlc_amsdu_dump_cnt(ami, b);
#endif // endif

	for (i = 0, last = 0, total = 0; i < MAX_TX_SUBFRAMES_LIMIT; i++) {
		total += ami->amdbg->tx_msdu_histogram[i];
		if (ami->amdbg->tx_msdu_histogram[i])
			last = i;
	}
	bcm_bprintf(b, "TxMSDUdens:");
	for (i = 0; i <= last; i++) {
		bcm_bprintf(b, " %6u(%2d%%)", ami->amdbg->tx_msdu_histogram[i],
			(total == 0) ? 0 :
			(ami->amdbg->tx_msdu_histogram[i] * 100 / total));
		if (((i+1) % 8) == 0 && i != last)
			bcm_bprintf(b, "\n        :");
	}
	bcm_bprintf(b, "\n\n");

	for (i = 0, last = 0, total = 0; i < AMSDU_LENGTH_BINS; i++) {
		total += ami->amdbg->tx_length_histogram[i];
		if (ami->amdbg->tx_length_histogram[i])
			last = i;
	}
	bcm_bprintf(b, "TxAMSDU Len:");
	for (i = 0; i <= last; i++) {
		bcm_bprintf(b, " %2u-%uk%s %6u(%2d%%)", i, i+1, (i < 9)?" ":"",
		            ami->amdbg->tx_length_histogram[i],
		            (total == 0) ? 0 :
		            (ami->amdbg->tx_length_histogram[i] * 100 / total));
		if (((i+1) % 4) == 0 && i != last)
			bcm_bprintf(b, "\n         :");
	}
	bcm_bprintf(b, "\n");

	for (i = 0, last = 0, total = 0; i < AMSDU_RX_SUBFRAMES_BINS; i++) {
		total += ami->amdbg->rx_msdu_histogram[i];
		if (ami->amdbg->rx_msdu_histogram[i])
			last = i;
	}
	bcm_bprintf(b, "RxMSDUdens:");
	for (i = 0; i <= last; i++) {
		bcm_bprintf(b, " %6u(%2d%%)", ami->amdbg->rx_msdu_histogram[i],
			(total == 0) ? 0 :
			(ami->amdbg->rx_msdu_histogram[i] * 100 / total));
		if (((i+1) % 8) == 0 && i != last)
			bcm_bprintf(b, "\n        :");
	}
	bcm_bprintf(b, "\n\n");

	for (i = 0, last = 0, total = 0; i < AMSDU_LENGTH_BINS; i++) {
		total += ami->amdbg->rx_length_histogram[i];
		if (ami->amdbg->rx_length_histogram[i])
			last = i;
	}
	bcm_bprintf(b, "RxAMSDU Len:");
	for (i = 0; i <= last; i++) {
		bcm_bprintf(b, " %2u-%uk%s %6u(%2d%%)", i, i+1, (i < 9)?" ":"",
		            ami->amdbg->rx_length_histogram[i],
		            (total == 0) ? 0 :
		            (ami->amdbg->rx_length_histogram[i] * 100 / total));
		if (((i+1) % 4) == 0 && i != last)
			bcm_bprintf(b, "\n         :");
	}
	bcm_bprintf(b, "\n");

	return 0;
}

#ifdef WLCNT
void
wlc_amsdu_dump_cnt(amsdu_info_t *ami, struct bcmstrbuf *b)
{
	wlc_amsdu_cnt_t *cnt = ami->cnt;

	bcm_bprintf(b, "agg_openfail %u\n", cnt->agg_openfail);
	bcm_bprintf(b, "agg_passthrough %u\n", cnt->agg_passthrough);
	bcm_bprintf(b, "agg_block %u\n", cnt->agg_openfail);
	bcm_bprintf(b, "agg_amsdu %u\n", cnt->agg_amsdu);
	bcm_bprintf(b, "agg_msdu %u\n", cnt->agg_msdu);
	bcm_bprintf(b, "agg_stop_tailroom %u\n", cnt->agg_stop_tailroom);
	bcm_bprintf(b, "agg_stop_sf %u\n", cnt->agg_stop_sf);
	bcm_bprintf(b, "agg_stop_len %u\n", cnt->agg_stop_len);
	bcm_bprintf(b, "agg_stop_lowwm %u\n", cnt->agg_stop_lowwm);
	bcm_bprintf(b, "deagg_msdu %u\n", cnt->deagg_msdu);
	bcm_bprintf(b, "deagg_amsdu %u\n", cnt->deagg_amsdu);
	bcm_bprintf(b, "deagg_badfmt %u\n", cnt->deagg_badfmt);
	bcm_bprintf(b, "deagg_wrongseq %u\n", cnt->deagg_wrongseq);
	bcm_bprintf(b, "deagg_badsflen %u\n", cnt->deagg_badsflen);
	bcm_bprintf(b, "deagg_badsfalign %u\n", cnt->deagg_badsfalign);
	bcm_bprintf(b, "deagg_badtotlen %u\n", cnt->deagg_badtotlen);
	bcm_bprintf(b, "deagg_openfail %u\n", cnt->deagg_openfail);
	bcm_bprintf(b, "deagg_swdeagglong %u\n", cnt->deagg_swdeagglong);
	bcm_bprintf(b, "deagg_flush %u\n", cnt->deagg_flush);
	bcm_bprintf(b, "tx_pkt_free_ignored %u\n", cnt->tx_pkt_free_ignored);
	bcm_bprintf(b, "tx_padding_in_tail %u\n", cnt->tx_padding_in_tail);
	bcm_bprintf(b, "tx_padding_in_head %u\n", cnt->tx_padding_in_head);
	bcm_bprintf(b, "tx_padding_no_pad %u\n", cnt->tx_padding_no_pad);
}
#endif	/* WLCNT */

#if defined(WLAMSDU_TX) && !defined(WLAMSDU_TX_DISABLED)
static void
wlc_amsdu_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	amsdu_info_t *ami = (amsdu_info_t *)ctx;
	scb_amsdu_t *scb_amsdu = SCB_AMSDU_CUBBY(ami, scb);
	amsdu_scb_prio *amsduprio;

	if (!AMSDU_TX_SUPPORT(ami->pub) || !scb_amsdu || !SCB_AMSDU(scb))
		return;

	bcm_bprintf(b, "\n");

	/* add \t to be aligned with other scb stuff */
	bcm_bprintf(b, "\tAMSDU scb best-effort\n");

	amsduprio = &scb_amsdu->prio[PRIO_8021D_BE];

	bcm_bprintf(b, "\tamsdu_agg_sframes %u amsdu_agg_bytes %u amsdu_agg_txpending %u\n",
		amsduprio->amsdu_agg_sframes, amsduprio->amsdu_agg_bytes,
		amsduprio->amsdu_agg_txpending);
	bcm_bprintf(b, "\tamsdu_ht_agg_bytes_max %d vht_agg_max %d amsdu_agg_allowtid %d\n",
		amsduprio->amsdu_ht_agg_bytes_max, amsduprio->amsdu_vht_agg_bytes_max,
		amsduprio->amsdu_agg_allowtid);

	bcm_bprintf(b, "\n");
}
#endif /* WLAMSDU_TX && !WLAMSDU_TX_DISABLED */
#endif	/* BCMDBG || BCMDBG_DUMP || BCMDBG_AMSDU */
