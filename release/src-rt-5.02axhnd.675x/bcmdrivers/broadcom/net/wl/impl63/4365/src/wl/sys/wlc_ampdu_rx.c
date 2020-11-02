/*
 * A-MPDU Rx (with extended Block Ack protocol) source file
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
 * $Id: wlc_ampdu_rx.c 769205 2018-11-06 10:34:56Z $
 */

/**
 * C preprocessor defines used in this file:
 * WLAMPDU_HOSTREORDER: saves dongle memory by maintaining RX buffers in host instead.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [AmpduUcode] [AmpduHwAgg] [AmpduAQM]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifndef WLAMPDU
#error "WLAMPDU is not defined"
#endif	/* WLAMPDU */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_scb.h>
#include <wlc_frmutil.h>
#ifdef AP
#include <wlc_apps.h>
#endif // endif
#ifdef WLAMPDU
#include <wlc_ampdu_rx.h>
#include <wlc_ampdu_cmn.h>
#endif // endif
#include <wlc_vht.h>
#include <wlc_scb_ratesel.h>
#include <wl_export.h>
#include <wlc_rm.h>
#if defined(BCMCCX) && defined(CCX_SDK)
#include <wlc_ccx.h>
#endif /* BCMCCX && CCX_SDK */

#ifdef PROP_TXSTATUS         /* saves dongle memory by queuing tx packets on the host \
	*/
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif // endif
#include <wlc_objregistry.h>

#include <wlc_ht.h>

#include <wlc_rx.h>
#include <event_log.h>
#include <event_trace.h>

#ifdef WL_MU_RX
#include <wlc_murx.h>
#endif // endif
#ifdef WL11K
#include <wlc_rrm.h>
#endif // endif

/* iovar table */
enum {
	IOV_AMPDU_RX,		/* enable/disable ampdu rx */
	IOV_AMPDU_RX_TID,		/* RX BA Enable per-tid */
	IOV_AMPDU_RX_DENSITY,	/* ampdu density */
	IOV_AMPDU_RX_FACTOR,	/* ampdu rcv len */
	IOV_AMPDU_RESP_TIMEOUT_B, /* timeout (ms) for left edge of win move for brcm peer */
	IOV_AMPDU_RESP_TIMEOUT_NB, /* timeout (ms) for left edge of win move for non-brcm peer */
	IOV_AMPDU_RX_BA_WSIZE,	/* ampdu RX ba window size */
	IOV_AMPDU_HOSTREORDER, /* enable host reordering of packets */
	IOV_AMPDU_RXAGGR,	/* enable/disable ampdu rx aggregation per-TID and per-bsscfg */
	IOV_AMPDU_LAST
};

static const bcm_iovar_t ampdu_iovars[] = {
	{"ampdu_rx", IOV_AMPDU_RX, (IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_BOOL, 0},
	{"ampdu_rx_tid", IOV_AMPDU_RX_TID, (0), IOVT_BUFFER, sizeof(struct ampdu_tid_control)},
	{"ampdu_rx_density", IOV_AMPDU_RX_DENSITY, (IOVF_RSDB_SET), IOVT_UINT8, 0},
	{"ampdu_rx_factor", IOV_AMPDU_RX_FACTOR, (IOVF_SET_DOWN), IOVT_UINT32, 0},
#ifdef  WLAMPDU_HOSTREORDER
	{"ampdu_hostreorder", IOV_AMPDU_HOSTREORDER, (IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_BOOL, 0},
#endif /* WLAMPDU_HOSTREORDER */
	{"ampdu_rxaggr", IOV_AMPDU_RXAGGR, IOVF_BSS_SET_DOWN, IOVT_BUFFER,
	sizeof(struct ampdu_aggr)},
	{NULL, 0, 0, 0, 0}
};

#if defined(DONGLEBUILD) && defined(BCMPCIEDEV) && defined(WLAMPDU_HOSTREORDER)
#ifndef BCMPKTIDMAP
#error "PCIE dongle AMPDU reordering should have BCMPKTIDMAP defined"
#endif // endif
#endif /* defined(DONGLEBUILD) && defined(BCMPCIEDEV) && defined(WLAMPDU_HOSTREORDER) */

/*
 * When BCMPKTIDMAP is defined, a responder's rxq[] is an array of 16bit pktids
 * instead of 32bit pktptrs. The PKTPTR() macro converts a pktid to a pktptr,
 * and PKTID() macro fetches the unique 16bit pktid associated with packet.
 * Also, the d11rxhdr is saved in the packet's pkttag (32bit systems).
 */
#if defined(BCMPKTIDMAP)
#define AMPDU_RXQ_SETPKT(resp, index, p)    ((resp)->rxq[index] = PKTID(p))
#define AMPDU_RXQ_GETPKT(resp, index)       (PKTPTR((resp)->rxq[index]))
#define AMPDU_RXQ_CLRPKT(resp, index)       ((resp)->rxq[index] = PKT_NULL_ID)
#define AMPDU_RXQ_HASPKT(resp, index)       ((resp)->rxq[index] != PKT_NULL_ID)
#define AMPDU_GET_WRXH(resp, index, p)      (WLPKTTAG(p)->u.wrxh)
#define AMPDU_SET_WRXH(resp, index, p, h)   \
	({ ASSERT(WLPKTTAG(p)->u.wrxh == NULL); WLPKTTAG(p)->u.wrxh = (h); })
#else  /* ! BCMPKTIDMAP */
#define AMPDU_RXQ_SETPKT(resp, index, p)    ((resp)->rxq[index] = (p))
#define AMPDU_RXQ_GETPKT(resp, index)       ((resp)->rxq[index])
#define AMPDU_RXQ_CLRPKT(resp, index)       ((resp)->rxq[index] = NULL)
#define AMPDU_RXQ_HASPKT(resp, index)       ((resp)->rxq[index] != NULL)
#define AMPDU_GET_WRXH(resp, index, p)      ((resp)->wrxh[index])
#define AMPDU_SET_WRXH(resp, index, p, h)   ((resp)->wrxh[index] = (h))
#endif /* ! BCMPKTIDMAP */

#ifdef WLAMPDU_HOSTREORDER
#define AMPDU_CHECK_HOST_HASPKT(resp, index)    isset((resp)->host_pkt_pending, (index))
#define AMPDU_SET_HOST_HASPKT(resp, index, osh, p) \
	{   \
		setbit((resp)->host_pkt_pending, (index)); \
		AMPDU_CONSOLIDATE_AMSDU_RXCHAIN(osh, p, TRUE);	\
		AMPDU_STORE_RXCPLID(resp, index, osh, p);	\
		PKTSETNORXCPL(osh, p);			\
	}
#define AMPDU_CLEAR_HOSTPKT(resp, index) \
	{\
		clrbit((resp)->host_pkt_pending, (index)); \
		AMPDU_CLEAR_RXCPLID(resp, index); \
	}

#if defined(BCMPKTIDMAP)
#define AMPDU_CLEAR_RXCPLID(resp, index)			((resp)->rxq[index] = 0)
#define AMPDU_CONSOLIDATE_AMSDU_RXCHAIN(osh, p, norxcpl)   \
	wl_chain_rxcompletions_amsdu(osh, p, norxcpl)
#define AMPDU_STORE_RXCPLID(resp, index, osh, p)	((resp)->rxq[index] = PKTRXCPLID(osh, p))
#define AMPDU_CHAIN_RXCPLID_TAIL(a, b)			wl_chain_rxcomplete_id_tail(a, b)
#define AMPDU_CHAIN_RXCPLID_HEAD(a, b)			wl_chain_rxcomplete_id_head(a, b)
#define AMPDU_CHAIN_RXCPLID_RESET(a)	({	\
	(a)->cnt = 0;	\
	(a)->head = 0;	\
	(a)->tail = 0;	\
})
#define AMPDU_CHAIN_RXCPLID_EMPTY(a)			((a)->cnt == 0)
#define AMPDU_CHAIN_RXCPLID_FLUSH(a, b)	({	\
	wl_flush_rxreorderqeue_flow(a, b);	\
	AMPDU_CHAIN_RXCPLID_RESET(b);	\
})
#else /* BCMPKTIDMAP */
#define AMPDU_CLEAR_RXCPLID(resp, index)		do {} while (0)
#define AMPDU_CONSOLIDATE_AMSDU_RXCHAIN(osh, p, norxcpl) do {} while (0)
#define AMPDU_STORE_RXCPLID(resp, index, osh, p) do {} while (0)
#define AMPDU_CHAIN_RXCPLID_TAIL(a, b) do {} while (0)
#define AMPDU_CHAIN_RXCPLID_HEAD(a, b)	do {} while (0)
#define AMPDU_CHAIN_RXCPLID_RESET(a) do {} while (0)
#define AMPDU_CHAIN_RXCPLID_EMPTY(a)	FALSE
#define AMPDU_CHAIN_RXCPLID_FLUSH(a, b) do {} while (0)
#endif /* BCMPKTIDMAP */
#else /* WLAMPDU_HOSTREORDER */
#define AMPDU_CHECK_HOST_HASPKT(resp, index)	FALSE
#define AMPDU_SET_HOST_HASPKT(resp, index, osh, p)	do {} while (0)
#define AMPDU_CLEAR_HOSTPKT(resp, index)		do {} while (0)
#define AMPDU_CHAIN_RXCPLID_TAIL(a, b)			do {} while (0)
#define AMPDU_CHAIN_RXCPLID_HEAD(a, b)			do {} while (0)
#define AMPDU_CONSOLIDATE_AMSDU_RXCHAIN(osh, p, norxcpl) do {} while (0)
#define AMPDU_CHAIN_RXCPLID_RESET(a)			do {} while (0)
#define AMPDU_CHAIN_RXCPLID_EMPTY(a)			FALSE
#define AMPDU_CHAIN_RXCPLID_FLUSH(a, b)			do {} while (0)
#endif /* WLAMPDU_HOSTREORDER */

#define AMPDU_IS_PKT_PENDING(wlc, resp, index)	\
	(AMPDU_RXQ_HASPKT((resp), (index)) || \
	 (AMPDU_HOST_REORDER_ENAB(wlc->pub) && AMPDU_CHECK_HOST_HASPKT((resp), (index))))

#ifndef AMPDU_RX_BA_MAX_WSIZE
#define AMPDU_RX_BA_MAX_WSIZE	64		/* max Rx ba window size (in pdu) */
#endif /* AMPDU_RX_BA_MAX_WSIZE */
#ifndef AMPDU_RX_BA_DEF_WSIZE
#define AMPDU_RX_BA_DEF_WSIZE	64		/* default Rx ba window size (in pdu) */
#endif /* AMPDU_RX_BA_DEF_WSIZE */

#define VHT_DEFAULT_RX_FACTOR	AMPDU_RX_FACTOR_1024K

#define AMPDU_RESP_TIMEOUT_B		1000	/* # of ms wo resp prog with brcm peer */
#define AMPDU_RESP_TIMEOUT_NB		200	/* # of ms wo resp prog with non-brcm peer */
#define AMPDU_RESP_TIMEOUT		100	/* timeout interval in msec for resp prog */

/* internal BA states */
#define	AMPDU_TID_STATE_BA_OFF		0x00	/* block ack OFF for tid */
#define	AMPDU_TID_STATE_BA_ON		0x01	/* block ack ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_ON	0x02	/* block ack pending ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_OFF	0x03	/* block ack pending OFF for tid */

/* useful macros */
#define NEXT_SEQ(seq) MODINC_POW2((seq), SEQNUM_MAX)
#define NEXT_RX_INDEX(index) MODINC_POW2((index), (wlc->ampdu_rx->config->ba_max_rx_wsize))
#define RX_SEQ_TO_INDEX(ampdu_rx, seq) ((seq) & (((ampdu_rx)->config->ba_max_rx_wsize) - 1))

/* ampdu related stats */
typedef struct wlc_ampdu_rx_cnt {
#ifdef WLCNT
	/* responder side counters */
	uint32 rxampdu;		/* ampdus recd */
	uint32 rxmpdu;		/* mpdus recd in a ampdu */
	uint32 rxht;		/* mpdus recd at ht rate and not in a ampdu */
	uint32 rxlegacy;	/* mpdus recd at legacy rate */
	uint32 rxampdu_sgi;	/* ampdus recd with sgi */
	uint32 rxampdu_stbc; /* ampdus recd with stbc */
	uint32 rxnobapol;	/* mpdus recd without a ba policy */
	uint32 rxholes;		/* missed seq numbers on rx side */
	uint32 rxqed;		/* pdus buffered before sending up */
	uint32 rxdup;		/* duplicate pdus */
	uint32 rxstuck;		/* watchdog bailout for stuck state */
	uint32 rxoow;		/* out of window pdus */
	uint32 rxoos;		/* out of seq pdus */
	uint32 rxaddbareq;	/* addba req recd */
	uint32 txaddbaresp;	/* addba resp sent */
	uint32 rxbar;		/* bar recd */
	uint32 txba;		/* ba sent */

	/* general: both initiator and responder */
	uint32 rxunexp;		/* unexpected packets */
	uint32 txdelba;		/* delba sent */
	uint32 rxdelba;		/* delba recd */

	uint32 rxampdubyte_h;	/* ampdu recd bytes */
	uint32 rxampdubyte_l;
#endif /* WLCNT */
} wlc_ampdu_rx_cnt_t;

typedef struct {
	uint32 rxmcs[AMPDU_HT_MCS_ARRAY_SIZE];		/* mcs of rx pkts */
	uint32 rxmcssgi[AMPDU_HT_MCS_ARRAY_SIZE];		/* mcs of rx pkts */
	uint32 rxmcsstbc[AMPDU_HT_MCS_ARRAY_SIZE];		/* mcs of rx pkts */
#ifdef WL11AC
	uint32 rxvht[AMPDU_MAX_VHT];		/* vht of rx pkts */
	uint32 rxvhtsgi[AMPDU_MAX_VHT];		/* vht of rx pkts */
	uint32 rxvhtstbc[AMPDU_MAX_VHT];		/* vht of rx pkts */
#endif /* WL11AC */

} ampdu_rx_dbg_t;

/* AMPDU module specific state */
typedef struct ampdu_rx_config {
	uint8 ba_policy;	/* ba policy; immediate vs delayed */
	uint8 ba_rx_wsize;      /* Rx ba window size (in pdu) */
	uint8 delba_timeout;	/* timeout after which to send delba (sec) */
	uint8 rx_factor;	/* maximum rx ampdu factor (0-3) ==> 2^(13+x) bytes */
	uint8 mpdu_density;	/* min mpdu spacing (0-7) ==> 2^(x-1)/8 usec */
	uint16 resp_timeout_b;	/* timeout (ms) for left edge of win move for brcm peer */
	uint16 resp_timeout_nb;	/* timeout (ms) for left edge of win move for non-brcm peer */
	uint8 rxba_enable[AMPDU_MAX_SCB_TID]; /* per-tid responder enable/disable of ampdu */
	uint8	ba_max_rx_wsize;	/* Rx ba window size (in pdu) */
} ampdu_rx_config_t;

struct ampdu_rx_info {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	int scb_handle;		/* scb cubby handle to retrieve data from scb */
	uint16 resp_cnt;	/* count of resp reorder queues */
	struct wl_timer *resp_timer;	/* timer for resp reorder q flush */
	bool	resp_timer_running; /* ampdu resp timer state */
	uint16  flow_id;
	ampdu_rx_config_t *config;
	bool rxaggr_support;	  /* Support ampdu rx aggregation */
#ifdef WLCNT
	wlc_ampdu_rx_cnt_t *cnt;	/* counters/stats */
#endif // endif
	ampdu_rx_dbg_t *amdbg;
#ifdef WL_FRWD_REORDER
	struct ampdu_reorder_info **dngl_reorder_bufs;
#endif // endif
	int bsscfg_handle;	/* BSSCFG cubby offset */
};

/** structure to store per-tid (=traffic class) state for the ampdu responder */
typedef struct scb_ampdu_tid_resp {
	uint8 ba_state;		/* ampdu ba state */
	uint8 ba_wsize;		/* negotiated ba window size (in pdu) */
	uint8 queued;		/* number of queued packets */
	uint8 dead_cnt;		/* number of sec without any progress */
	bool alive;		/* true if making forward progress */
	uint16 exp_seq;		/* next expected seq */
#if defined(BCMPKTIDMAP)
	uint16 rxq[AMPDU_BA_MAX_WSIZE];     /* rx reorder queue of 16bit pktid */
#else  /* ! BCMPKTIDMAP */
	void *rxq[AMPDU_BA_MAX_WSIZE];		/* rx reorder queue */
	void *wrxh[AMPDU_BA_MAX_WSIZE];		/* saved rxh queue */
#endif /* ! BCMPKTIDMAP */

	/* rx reorder pending in host rxh queue */
	uint32 host_pkt_pending[AMPDU_BA_MAX_WSIZE / NBITS(uint32)];
	uint16 flow_id;
	void *tohost_ctrlpkt;
	struct reorder_rxcpl_id_list rxcpl_list;
} scb_ampdu_tid_resp_t;

typedef struct ampdu_reorder_info {
	scb_ampdu_tid_resp_t *resp;
	uint8 cur_idx;
	uint8 exp_idx;
	uint8 pend_pkts;
	uint8 new_hole_informed;
	uint8 last_idx;
} ampdu_reorder_info_t;

/** structure to store per-tid state for the ampdu resp when off. statically allocated. */
typedef struct scb_ampdu_tid_resp_off {
	bool ampdu_recd;	/* TRUE is ampdu was recd in the 1 sec window */
	uint8 ampdu_cnt;	/* number of secs during which ampdus are recd */
} scb_ampdu_tid_resp_off_t;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
typedef struct scb_ampdu_cnt_rx {
	uint32 rxunexp;
	uint32 rxholes;
	uint32 rxstuck;
	uint32 txaddbaresp;
	uint32 sduretry;
	uint32 sdurejected;
	uint32 noba;
	uint32 rxampdu;
	uint32 rxmpdu;
	uint32 rxlegacy;
	uint32 rxdup;
	uint32 rxoow;
	uint32 rxdelba;
	uint32 rxbar;
} scb_ampdu_cnt_rx_t;
#endif	/* BCMDBG */

/**
 * Scb cubby structure, so related to a specific remote party. ini and resp are dynamically
 * allocated if needed.
 */
typedef struct scb_ampdu_rx {
	struct scb *scb;		/* back pointer for easy reference */
	scb_ampdu_tid_resp_t *resp[AMPDU_MAX_SCB_TID];	/* responder info */
	scb_ampdu_tid_resp_off_t resp_off[AMPDU_MAX_SCB_TID];	/* info when resp is off */
	ampdu_rx_info_t *ampdu_rx; /* back ref to main ampdu_rx */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	scb_ampdu_cnt_rx_t cnt;
#endif	/* BCMDBG */
} scb_ampdu_rx_t;

#ifdef WLAMPDU_HOSTREORDER

#undef NO_NEWHOLE
#undef NEWHOLE
#undef NO_DEL_FLOW
#undef DEL_FLOW
#undef NO_FLUSH_ALL
#undef FLUSH_ALL
#undef AMPDU_INVALID_INDEX

#define NO_NEWHOLE		FALSE
#define NEWHOLE			TRUE
#define NO_DEL_FLOW		FALSE
#define DEL_FLOW		TRUE
#define NO_FLUSH_ALL		FALSE
#define FLUSH_ALL		TRUE
#define AMPDU_INVALID_INDEX	0xFFFF

static void wlc_ampdu_setpkt_hostreorder_info(wlc_info_t *wlc, scb_ampdu_tid_resp_t *resp,
	void *p, uint16 cur_idx, bool new_hole, bool del_flow, bool flush_all);
static int wlc_ampdu_alloc_flow_id(ampdu_rx_info_t *ampdu);
static int wlc_ampdu_free_flow_id(ampdu_rx_info_t *ampdu, scb_ampdu_tid_resp_t *resp,
	struct scb *scb);
#else
#define wlc_ampdu_setpkt_hostreorder_info(a, b, c, d, e, f, g) do {} while (0)
#define wlc_ampdu_alloc_flow_id(a)			0
#define wlc_ampdu_free_flow_id(a, b, c)		0
#endif /* WLAMPDU_HOSTREORDER */

#ifdef BCMDBG
#define AMPDUSCBCNTADD(cnt, upd) ((cnt) += (upd))
#define AMPDUSCBCNTINCR(cnt) ((cnt)++)
#else
#define AMPDUSCBCNTADD(a, b) do { } while (0)
#define AMPDUSCBCNTINCR(a)  do { } while (0)
#endif // endif

struct ampdu_rx_cubby {
	scb_ampdu_rx_t *scb_rx_cubby;
};

#define SCB_AMPDU_INFO(ampdu_rx, scb) (SCB_CUBBY((scb), (ampdu_rx)->scb_handle))
#define SCB_AMPDU_RX_CUBBY(ampdu_rx, scb) \
	(((struct ampdu_rx_cubby *)SCB_AMPDU_INFO(ampdu_rx, scb))->scb_rx_cubby)

/** bsscfg cubby structure. */
typedef struct bsscfg_ampdu_rx {
	int8 rxaggr_override;	/* rxaggr override for all TIDs */
	uint16 rxaggr_TID_bmap; /* aggregation enabled TIDs bitmap */
} bsscfg_ampdu_rx_t;

#define BSSCFG_AMPDU_RX_CUBBY(ampdu_rx, bsscfg) \
	((bsscfg_ampdu_rx_t *)BSSCFG_CUBBY((bsscfg), (ampdu_rx)->bsscfg_handle))

/* local prototypes */
void wlc_ampdu_rxcfg_init(wlc_info_t *wlc, ampdu_rx_config_t *ampdu_rx_cfg);
/* scb cubby */
static int scb_ampdu_rx_init(void *context, struct scb *scb);
static void scb_ampdu_rx_deinit(void *context, struct scb *scb);
/* bsscfg cubby */
static int bsscfg_ampdu_rx_init(void *context, wlc_bsscfg_t *bsscfg);
static void bsscfg_ampdu_rx_deinit(void *context, wlc_bsscfg_t *bsscfg);
static int wlc_ampdu_rx_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static void wlc_ampdu_rx_watchdog(void *hdl);
static int wlc_ampdu_rx_down(void *hdl);
static int wlc_ampdu_rx_up(void *hdl);

static INLINE void wlc_ampdu_release_all_ordered(wlc_info_t *wlc, scb_ampdu_rx_t *scb_ampdu,
	uint8 tid);

static void ampdu_create_f(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f,
	void *p, wlc_d11rxhdr_t *wrxh);

static void wlc_ampdu_resp_timeout(void *arg);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

#if defined(BCMDBG_ASSERT) || (defined(BCM47XX_CA9) && defined(WL_PL310_WAR))
/** given a packet, returns sequence control number as specified per 802.11 */
static INLINE uint16
pkt_h_seqnum(wlc_info_t *wlc, void *p)
{
	struct dot11_header *h;
	h = (struct dot11_header *)PKTDATA(wlc->osh, p);
	return (ltoh16(h->seq) >> SEQNUM_SHIFT);
}
#endif // endif

/**
 * Called on packet reception. MPDU's can be received out-of-order, this function forwards, if
 * available, a continuous block of packets that are in the original order to an upper software. In
 * the case of 'host reordering', the function behaves differently, by instructing the host.
 */
static INLINE void
wlc_ampdu_release_ordered(wlc_info_t *wlc, scb_ampdu_rx_t *scb_ampdu, uint8 tid)
{
	void *p = NULL;
	struct wlc_frminfo f;
	uint16 indx;
	uint bandunit;
	struct ether_addr ea;
	struct scb *newscb;
	struct scb *scb = scb_ampdu->scb;
	scb_ampdu_tid_resp_t *resp = scb_ampdu->resp[tid];
	wlc_bsscfg_t *bsscfg;
	if (resp == NULL)
		return;

	for (indx = RX_SEQ_TO_INDEX(wlc->ampdu_rx, resp->exp_seq);
		AMPDU_IS_PKT_PENDING(wlc, resp, indx); indx = NEXT_RX_INDEX(indx))
	{
		resp->queued--;

#ifdef  WLAMPDU_HOSTREORDER
		if (!AMPDU_HOST_REORDER_ENAB(wlc->pub))
#endif // endif
		{
			p = AMPDU_RXQ_GETPKT(resp, indx);
			AMPDU_RXQ_CLRPKT(resp, indx); /* free rxq[indx] slot */
#if defined(BCM47XX_CA9) && defined(WL_PL310_WAR)
			if (resp->exp_seq != pkt_h_seqnum(wlc, p)) {
				WL_ERROR(("wl%d: %s: sequence number mismatched\n",
					wlc->pub->unit, __FUNCTION__));
				PKTFREE(wlc->osh, p, FALSE);
				resp->alive = TRUE;
				resp->exp_seq = NEXT_SEQ(resp->exp_seq);
				wlc_ampdu_rx_send_delba(wlc->ampdu_rx, scb, tid, FALSE,
					DOT11_RC_UNSPECIFIED);
				return;
			}
#else
			ASSERT(resp->exp_seq == pkt_h_seqnum(wlc, p));
#endif /* defined(BCM47XX_CA9) && defined(WL_PL310_WAR) */
		}
		resp->alive = TRUE;
		resp->exp_seq = NEXT_SEQ(resp->exp_seq);

#ifdef  WLAMPDU_HOSTREORDER
		if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
			if (BCMPCIEDEV_ENAB()) {
				AMPDU_CHAIN_RXCPLID_TAIL(&resp->rxcpl_list, resp->rxq[indx]);
			}
			AMPDU_CLEAR_HOSTPKT(resp, indx);
			continue;
		}
#endif // endif
		WL_AMPDU_RX(("wl%d: wlc_ampdu_release_ordered: releasing seq 0x%x\n",
			wlc->pub->unit, resp->exp_seq));

		/* create the fields of frminfo f */
		ampdu_create_f(wlc, scb, &f, p, AMPDU_GET_WRXH(resp, indx, p));

		bsscfg = scb->bsscfg;
		bandunit = scb->bandunit;
		bcopy(&scb->ea, &ea, ETHER_ADDR_LEN);

		wlc_recvdata_ordered(wlc, scb, &f);

		/* validate that the scb is still around and some path in
		 * wlc_recvdata_ordered() did not free it
		 */
		newscb = wlc_scbfindband(wlc, bsscfg, &ea, bandunit);
		if ((newscb == NULL) || (newscb != scb)) {
			WL_ERROR(("wl%d: %s: scb freed; bail out\n",
				wlc->pub->unit, __FUNCTION__));
			return;
		}

		/* Make sure responder was not freed when we gave up the lock in sendup */
		if ((resp = scb_ampdu->resp[tid]) == NULL)
			return;
	}
} /* wlc_ampdu_release_ordered */

/**
 * Called on frame reception. Release next n pending ordered packets starting from index going over
 * holes.
 */
static INLINE void
wlc_ampdu_release_n_ordered(wlc_info_t *wlc, scb_ampdu_rx_t *scb_ampdu, uint8 tid, uint16 offset)
{
	void *p;
	struct wlc_frminfo f;
	uint16 indx;
	uint bandunit;
	struct ether_addr ea;
	struct scb *newscb;
	struct scb *scb = scb_ampdu->scb;
	scb_ampdu_tid_resp_t *resp = scb_ampdu->resp[tid];
	wlc_bsscfg_t *bsscfg;

	ASSERT(resp);
	if (resp == NULL)
		return;

	for (; offset; offset--) {
	        indx = RX_SEQ_TO_INDEX(wlc->ampdu_rx, resp->exp_seq);
		if (AMPDU_IS_PKT_PENDING(wlc, resp, indx)) {

			resp->queued--;

#ifdef  WLAMPDU_HOSTREORDER
			if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
				if (BCMPCIEDEV_ENAB()) {
					AMPDU_CHAIN_RXCPLID_TAIL(&resp->rxcpl_list,
						resp->rxq[indx]);
				}
				AMPDU_CLEAR_HOSTPKT(resp, indx);
			}
			else
#endif // endif
				{
				p = AMPDU_RXQ_GETPKT(resp, indx);
				AMPDU_RXQ_CLRPKT(resp, indx); /* free rxq[indx] slot */

#if defined(BCM47XX_CA9) && defined(WL_PL310_WAR)
				if (resp->exp_seq != pkt_h_seqnum(wlc, p)) {
					WL_ERROR(("wl%d: %s: sequence number mismatched\n",
						wlc->pub->unit, __FUNCTION__));
					PKTFREE(wlc->osh, p, FALSE);
					resp->alive = TRUE;
					resp->exp_seq = NEXT_SEQ(resp->exp_seq);
					wlc_ampdu_rx_send_delba(wlc->ampdu_rx, scb, tid, FALSE,
						DOT11_RC_UNSPECIFIED);
					return;
				}
#else
				ASSERT(resp->exp_seq == pkt_h_seqnum(wlc, p));
#endif /* defined(BCM47XX_CA9) && defined(WL_PL310_WAR) */
				/* set the fields of frminfo f */
				ampdu_create_f(wlc, scb, &f, p, AMPDU_GET_WRXH(resp, indx, p));

				bsscfg = scb->bsscfg;
				bandunit = scb->bandunit;
				bcopy(&scb->ea, &ea, ETHER_ADDR_LEN);
				WL_AMPDU_RX(("wl%d: wlc_ampdu_release_n_ordered: release "
					"seq 0x%x\n",
					wlc->pub->unit, resp->exp_seq));

				wlc_recvdata_ordered(wlc, scb, &f);

				/* validate that the scb is still around and some path in
				 * wlc_recvdata_ordered() did not free it
				*/
				newscb = wlc_scbfindband(wlc, bsscfg, &ea, bandunit);
				if ((newscb == NULL) || (newscb != scb)) {
					WL_ERROR(("wl%d: %s: scb freed; bail out\n",
						wlc->pub->unit, __FUNCTION__));
					return;
				}

				/* Make sure responder was not freed when we gave up
				 * the lock in sendup
				 */
				if ((resp = scb_ampdu->resp[tid]) == NULL)
					return;
			}

		} else {
			WLCNTINCR(wlc->ampdu_rx->cnt->rxholes);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.rxholes);
		}
		resp->alive = TRUE;
		resp->exp_seq = NEXT_SEQ(resp->exp_seq);
	}
} /* wlc_ampdu_release_n_ordered */

/**
 * Called on AMPDU response time out. Releases all pending ordered packets starting from index going
 * over holes.
 */
static INLINE void
wlc_ampdu_release_all_ordered(wlc_info_t *wlc, scb_ampdu_rx_t *scb_ampdu, uint8 tid)
{
	uint16 seq, max_seq, offset, i, indx;
	scb_ampdu_tid_resp_t *resp = scb_ampdu->resp[tid];
	ampdu_rx_config_t *ampdu_rx_cfg = scb_ampdu->ampdu_rx->config;

	ASSERT(resp);
	if (resp == NULL)
		return;

	if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
		wlc_ampdu_release_n_ordered(wlc, scb_ampdu, tid,
			ampdu_rx_cfg->ba_max_rx_wsize);
		if (BCMPCIEDEV_ENAB())
			AMPDU_CHAIN_RXCPLID_FLUSH(wlc->wl, &resp->rxcpl_list);
	} else {
		for (i = 0, seq = resp->exp_seq, max_seq = resp->exp_seq;
			i < ampdu_rx_cfg->ba_max_rx_wsize;
			i++, seq = NEXT_SEQ(seq)) {
		indx = RX_SEQ_TO_INDEX(wlc->ampdu_rx, seq);
		if (AMPDU_RXQ_HASPKT(resp, indx)) /* rxq[indx] slot is not empty */
			max_seq = seq;
		}

		offset = MODSUB_POW2(max_seq, resp->exp_seq, SEQNUM_MAX) + 1;
		wlc_ampdu_release_n_ordered(wlc, scb_ampdu, tid, offset);
	}
}

/** Data structures need to be initialized during system initialization */
void
BCMATTACHFN(wlc_ampdu_rxcfg_init)(wlc_info_t *wlc, ampdu_rx_config_t *ampdu_rx_cfg)
{
	int i;
	ampdu_rx_cfg->ba_max_rx_wsize = AMPDU_RX_BA_MAX_WSIZE;
	/* enable rxba_enable on TIDs */
	for (i = 0; i < AMPDU_MAX_SCB_TID; i++)
		ampdu_rx_cfg->rxba_enable[i] = TRUE;

	ampdu_rx_cfg->ba_policy = DOT11_ADDBA_POLICY_IMMEDIATE;
	ampdu_rx_cfg->ba_rx_wsize = AMPDU_RX_BA_DEF_WSIZE;

/* set optional wsize override from bmac dongle */
#ifdef WLC_HIGH_ONLY
	if (wlc->pub->ampdu_ba_rx_wsize)
		ampdu_rx_cfg->ba_rx_wsize = wlc->pub->ampdu_ba_rx_wsize;
#endif /* WLC_HIGH_ONLY */

	if (ampdu_rx_cfg->ba_rx_wsize > ampdu_rx_cfg->ba_max_rx_wsize) {
		WL_ERROR(("wl%d: The Default AMPDU_RX_BA_WSIZE is greater than MAX value\n",
			wlc->pub->unit));
		ampdu_rx_cfg->ba_rx_wsize = ampdu_rx_cfg->ba_max_rx_wsize;
	}

	if (D11REV_IS(wlc->pub->corerev, 17) || D11REV_IS(wlc->pub->corerev, 28))
		ampdu_rx_cfg->mpdu_density = AMPDU_DENSITY_8_US;
	else
		ampdu_rx_cfg->mpdu_density = AMPDU_DEF_MPDU_DENSITY;

	ampdu_rx_cfg->resp_timeout_b = AMPDU_RESP_TIMEOUT_B;
	ampdu_rx_cfg->resp_timeout_nb = AMPDU_RESP_TIMEOUT_NB;

#ifdef WL11AC
	if (VHT_ENAB(wlc->pub)) {
		/* Initialize VHT AMPDU defaults */
		ampdu_rx_cfg->rx_factor = VHT_DEFAULT_RX_FACTOR;
	} else
#endif /* WL11AC */
	{
		/* bump max ampdu rcv size to 64k for all 11n devices except 4321A0 and 4321A1 */
		if (WLCISNPHY(wlc->band) && NREV_LT(wlc->band->phyrev, 2))
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_32K;
		else
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_64K;
#ifdef WLC_HIGH_ONLY
		if (wlc->pub->is_ss) {
			/* at super speed BRF1_RX_LARGE_AGG is enabled */
			/* which requires higher rx_factor */
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_64K;
		} else {
			/* Restrict to smaller rcv size for BMAC dongle */
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_32K;
		}
#endif // endif
	}

	ampdu_rx_cfg->delba_timeout = 0; /* AMPDUXXX: not yet supported */

}

/** Data structures need to be initialized during system initialization */
ampdu_rx_info_t *
BCMATTACHFN(wlc_ampdu_rx_attach)(wlc_info_t *wlc)
{
	ampdu_rx_info_t *ampdu_rx;

	/* some code depends on packed structures */
	STATIC_ASSERT(sizeof(struct dot11_bar) == DOT11_BAR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ba) == DOT11_BA_LEN + DOT11_BA_BITMAP_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ctl_header) == DOT11_CTL_HDR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_addba_req) == DOT11_ADDBA_REQ_LEN);
	STATIC_ASSERT(sizeof(struct dot11_addba_resp) == DOT11_ADDBA_RESP_LEN);
	STATIC_ASSERT(sizeof(struct dot11_delba) == DOT11_DELBA_LEN);
	STATIC_ASSERT(DOT11_MAXNUMFRAGS == NBITS(uint16));
	STATIC_ASSERT(ISPOWEROF2(AMPDU_RX_BA_MAX_WSIZE));

	ASSERT(wlc->pub->tunables->ampdunummpdu2streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu2streams > 0);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams > 0);

	if (!(ampdu_rx = (ampdu_rx_info_t *)MALLOCZ(wlc->osh, sizeof(ampdu_rx_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	ampdu_rx->wlc = wlc;
#ifndef WLRSDB_DVT
	ampdu_rx->config = (ampdu_rx_config_t*) obj_registry_get(wlc->objr,
		OBJR_AMPDURX_CONFIG);
#endif // endif
	if (ampdu_rx->config  == NULL) {
		if ((ampdu_rx->config =  (ampdu_rx_config_t*) MALLOCZ(wlc->pub->osh,
			sizeof(ampdu_rx_config_t))) == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", wlc->pub->unit,
				__FUNCTION__, MALLOCED(wlc->pub->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_AMPDURX_CONFIG, ampdu_rx->config);
		wlc_ampdu_rxcfg_init(wlc, ampdu_rx->config);
	}
#ifndef WLRSDB_DVT
	(void)obj_registry_ref(wlc->objr, OBJR_AMPDURX_CONFIG);
#endif // endif

#ifdef WLCNT
	if (!(ampdu_rx->cnt = (wlc_ampdu_rx_cnt_t *)MALLOCZ(wlc->osh,
		sizeof(wlc_ampdu_rx_cnt_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif /* WLCNT */

	/* Read nvram param to see if it disables AMPDU rx aggregation */
	if ((getintvar(wlc->pub->vars, "11n_disable") &
		WLFEATURE_DISABLE_11N_AMPDU_RX)) {
		ampdu_rx->rxaggr_support = FALSE;
	} else {
		ampdu_rx->rxaggr_support = TRUE;
	}

	/* reserve cubby in the bsscfg container for private data */
	if ((ampdu_rx->bsscfg_handle = wlc_bsscfg_cubby_reserve(wlc,
		sizeof(bsscfg_ampdu_rx_t), bsscfg_ampdu_rx_init, bsscfg_ampdu_rx_deinit,
		NULL, (void *)ampdu_rx)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container */
	ampdu_rx->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct ampdu_rx_cubby),
		scb_ampdu_rx_init, scb_ampdu_rx_deinit, NULL, (void *)ampdu_rx);

	if (ampdu_rx->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		goto fail;
	}

	if (!(ampdu_rx->resp_timer =
		wl_init_timer(wlc->wl, wlc_ampdu_resp_timeout, ampdu_rx, "resp"))) {
		WL_ERROR(("wl%d: ampdu_rx wl_init_timer() failed\n", wlc->pub->unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
	if (!(ampdu_rx->amdbg = (ampdu_rx_dbg_t *)MALLOCZ(wlc->osh, sizeof(ampdu_rx_dbg_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif /*  defined(BCMDBG) || defined(WLTEST) */

	/* needs to be last failure prone op in this function for attach/detach to work correctly */
	/* register module */
	if (wlc_module_register(wlc->pub, ampdu_iovars, "ampdu_rx", ampdu_rx, wlc_ampdu_rx_doiovar,
		wlc_ampdu_rx_watchdog, wlc_ampdu_rx_up, wlc_ampdu_rx_down)) {
		WL_ERROR(("wl%d: ampdu_rx wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* try to set ampdu to the default value */
	wlc_ampdu_rx_set(ampdu_rx, wlc->pub->_ampdu_rx);

	/* ampdu_resp_timer state is inited to not running */
	ampdu_rx->resp_timer_running = FALSE;

#if defined(WL_FRWD_REORDER) && !defined(WL_FRWD_REORDER_DISABLED)
	ampdu_rx->dngl_reorder_bufs = (struct ampdu_reorder_info **)MALLOC(wlc->osh,
		sizeof(struct ampdu_reorder_info *) * 256);
	bzero(ampdu_rx->dngl_reorder_bufs, sizeof(struct ampdu_reorder_info *) * 256);
#endif // endif

	return ampdu_rx;

fail:

	if (ampdu_rx->config && obj_registry_unref(wlc->objr, OBJR_AMPDURX_CONFIG) == 0) {
		obj_registry_set(wlc->objr, OBJR_AMPDURX_CONFIG, NULL);
		MFREE(wlc->osh, ampdu_rx->config, sizeof(ampdu_rx_config_t));
	}
#ifdef WLCNT
	if (ampdu_rx->cnt)
		MFREE(wlc->osh, ampdu_rx->cnt, sizeof(wlc_ampdu_rx_cnt_t));
#endif /* WLCNT */
#if defined(WL_FRWD_REORDER)
	if (ampdu_rx->dngl_reorder_bufs != NULL)
		MFREE(wlc->osh, ampdu_rx->dngl_reorder_bufs,
			sizeof(struct ampdu_reorder_info *) * 256);
#endif // endif

	MFREE(wlc->osh, ampdu_rx, sizeof(ampdu_rx_info_t));
	return NULL;
}

/** Data structures need to be freed at driver unload */
void
BCMATTACHFN(wlc_ampdu_rx_detach)(ampdu_rx_info_t *ampdu_rx)
{
	wlc_info_t *wlc;

	if (!ampdu_rx)
		return;
	wlc = ampdu_rx->wlc;

	ASSERT(ampdu_rx->resp_cnt == 0);
	ASSERT(ampdu_rx->resp_timer_running == FALSE);
	if (ampdu_rx->resp_timer) {
		if (ampdu_rx->resp_timer_running)
			wl_del_timer(wlc->wl, ampdu_rx->resp_timer);
		wl_free_timer(wlc->wl, ampdu_rx->resp_timer);
		ampdu_rx->resp_timer = NULL;
	}

	if (obj_registry_unref(wlc->objr, OBJR_AMPDURX_CONFIG) == 0) {
		obj_registry_set(wlc->objr, OBJR_AMPDURX_CONFIG, NULL);
		MFREE(wlc->osh, ampdu_rx->config, sizeof(ampdu_rx_config_t));
	}

#ifdef WLCNT
	if (ampdu_rx->cnt)
		MFREE(wlc->osh, ampdu_rx->cnt, sizeof(wlc_ampdu_rx_cnt_t));
#endif // endif
#if defined(WL_FRWD_REORDER)
	if (ampdu_rx->dngl_reorder_bufs != NULL)
		MFREE(ampdu_rx->wlc->osh, ampdu_rx->dngl_reorder_bufs,
			sizeof(struct ampdu_reorder_info *) * 256);
#endif // endif
#if defined(BCMDBG) || defined(WLTEST) || defined(BCMDBG_AMPDU)
	if (ampdu_rx->amdbg) {
		MFREE(wlc->osh, ampdu_rx->amdbg, sizeof(ampdu_rx_dbg_t));
		ampdu_rx->amdbg = NULL;
	}
#endif // endif

	wlc_module_unregister(wlc->pub, "ampdu_rx", ampdu_rx);
	MFREE(wlc->osh, ampdu_rx, sizeof(ampdu_rx_info_t));
}

/** Allocate and initialize structure related to a specific remote party */
static int
scb_ampdu_rx_init(void *context, struct scb *scb)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)context;
	struct ampdu_rx_cubby *cubby_info = SCB_AMPDU_INFO(ampdu_rx, scb);
	scb_ampdu_rx_t *scb_ampdu;

	if (scb && !SCB_INTERNAL(scb)) {
		scb_ampdu = MALLOCZ(ampdu_rx->wlc->osh, sizeof(scb_ampdu_rx_t));
		if (!scb_ampdu)
			return BCME_NOMEM;
		cubby_info->scb_rx_cubby = scb_ampdu;
		scb_ampdu->scb = scb;
		scb_ampdu->ampdu_rx = ampdu_rx;
	}
	return 0;
}

/** De-initialize and free structure related to a specific remote party */
static void
scb_ampdu_rx_deinit(void *context, struct scb *scb)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)context;
	struct ampdu_rx_cubby *cubby_info = SCB_AMPDU_INFO(ampdu_rx, scb);
	scb_ampdu_rx_t *scb_ampdu = NULL;

	WL_AMPDU_UPDN(("scb_ampdu_deinit: enter\n"));

	ASSERT(cubby_info);

	if (cubby_info)
		scb_ampdu = cubby_info->scb_rx_cubby;
	if (!scb_ampdu)
		return;

	scb_ampdu_rx_flush(ampdu_rx, scb);

	MFREE(ampdu_rx->wlc->osh, scb_ampdu, sizeof(scb_ampdu_rx_t));
	cubby_info->scb_rx_cubby = NULL;
}

/** bsscfg cubby init fn */
static int
bsscfg_ampdu_rx_init(void *context, wlc_bsscfg_t *bsscfg)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)context;
	bsscfg_ampdu_rx_t *bsscfg_ampdu = BSSCFG_AMPDU_RX_CUBBY(ampdu_rx, bsscfg);
	ASSERT(bsscfg_ampdu != NULL);

	if (ampdu_rx->rxaggr_support) {
		/* Enable for all TID by default */
		bsscfg_ampdu->rxaggr_override = AUTO;
		bsscfg_ampdu->rxaggr_TID_bmap = AMPDU_ALL_TID_BITMAP;
	} else {
		/* AMPDU RX module does not allow rx aggregation */
		bsscfg_ampdu->rxaggr_override = OFF;
		bsscfg_ampdu->rxaggr_TID_bmap = 0;
	}

	return BCME_OK;
}

/** bsscfg cubby deinit fn */
static void
bsscfg_ampdu_rx_deinit(void *context, wlc_bsscfg_t *bsscfg)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)context;
	bsscfg_ampdu_rx_t *bsscfg_ampdu = BSSCFG_AMPDU_RX_CUBBY(ampdu_rx, bsscfg);

	WL_AMPDU_UPDN(("bsscfg_ampdu_rx_deinit: enter\n"));

	bsscfg_ampdu->rxaggr_override = OFF;
	bsscfg_ampdu->rxaggr_TID_bmap = 0;
}

/** called on e.g. loss of association, AMPDU connection tear down, during 'wl down' */
void
scb_ampdu_rx_flush(ampdu_rx_info_t *ampdu_rx, struct scb *scb)
{
	uint8 tid;

	WL_AMPDU_UPDN(("scb_ampdu_rx_flush: enter\n"));

	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ampdu_cleanup_tid_resp(ampdu_rx, scb, tid);
	}
}

/** called during 'wl up' */
static int
wlc_ampdu_rx_up(void *hdl)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)hdl;
	/* this was in attach where it was calling ht module */
	wlc_ampdu_update_ie_param(ampdu_rx);
	return 0;
}

/** frees all the buffers and cleanup everything on down */
static int
wlc_ampdu_rx_down(void *hdl)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)hdl;
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("%s: enter\n", __FUNCTION__));

	FOREACHSCB(ampdu_rx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_rx_flush(ampdu_rx, scb);
	}

	return 0;
}

/** timer function, called after approx 100msec */
static void
wlc_ampdu_rx_handle_resp_dead(wlc_info_t *wlc, struct scb *scb, scb_ampdu_tid_resp_t *resp,
	scb_ampdu_rx_t *scb_ampdu, uint8 tid)
{
	void *p = NULL;
#ifdef WLAMPDU_HOSTREORDER
	struct reorder_rxcpl_id_list *rx_list = &resp->rxcpl_list;
	BCM_REFERENCE(rx_list);
#endif // endif

	if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
		if (BCMPCIEDEV_ENAB())
			AMPDU_CHAIN_RXCPLID_RESET(rx_list);
		else
			p = PKTGET(wlc->osh, TXOFF, FALSE);
	}
	wlc_ampdu_release_all_ordered(wlc, scb_ampdu, tid);
	if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
		if (BCMPCIEDEV_ENAB()) {
			AMPDU_CHAIN_RXCPLID_FLUSH(wlc->wl, rx_list);
		}
		else if (p) {
			wlc_ampdu_setpkt_hostreorder_info(wlc, resp, p, AMPDU_INVALID_INDEX,
				NO_NEWHOLE, NO_DEL_FLOW, FLUSH_ALL);

			/* reserving headroom for sdpcmd_tx */
			PKTPULL(wlc->osh, p, TXOFF);
			WLPKTTAGBSSCFGSET(p, scb->bsscfg->_idx);
			wl_sendup(wlc->wl,
			SCB_INTERFACE(scb), p, 1);
		}
		wlc_ampdu_rx_send_delba(wlc->ampdu_rx, scb_ampdu->scb, tid, FALSE,
			DOT11_RC_UNSPECIFIED);
	}
}

static void
wlc_ampdu_resp_timeout(void *arg)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)arg;
	ampdu_rx_config_t *ampdu_rx_cfg = ampdu_rx->config;
	wlc_info_t *wlc = ampdu_rx->wlc;
	scb_ampdu_rx_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;
	uint32 lim;
	bool start_timer = FALSE;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb))
			continue;

		scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			if ((resp = scb_ampdu->resp[tid]) == NULL)
				continue;

			if (resp->queued)
				start_timer = TRUE;

			/* check on resp forward progress */

			if (resp->alive) {
				resp->alive = FALSE;
				resp->dead_cnt = 0;
			} else {
				if (!resp->queued)
					continue;

				resp->dead_cnt++;
				lim = (scb->flags & SCB_BRCM) ?
					(ampdu_rx_cfg->resp_timeout_b / AMPDU_RESP_TIMEOUT) :
					(ampdu_rx_cfg->resp_timeout_nb / AMPDU_RESP_TIMEOUT);

				if (resp->dead_cnt >= lim) {
					WL_ERROR(("wl%d: %s: cleaning up resp tid %d waiting for"
						"seq 0x%x for %d ms\n",
						wlc->pub->unit, __FUNCTION__, tid, resp->exp_seq,
						lim*AMPDU_RESP_TIMEOUT));
					WLCNTINCR(ampdu_rx->cnt->rxstuck);
					AMPDUSCBCNTINCR(scb_ampdu->cnt.rxstuck);
					wlc_ampdu_rx_handle_resp_dead(wlc, scb, resp, scb_ampdu,
						tid);
				}
				start_timer = TRUE;
			}
		}
	}

	if (!start_timer && (ampdu_rx->resp_timer_running == TRUE)) {
		ampdu_rx->resp_timer_running = FALSE;
		wl_del_timer(ampdu_rx->wlc->wl, ampdu_rx->resp_timer);
	}
}

/** called during e.g. BT coexistence
 * If wish to do for all TIDs, input AMPDU_ALL_TID_BITMAP for conf_TID_bmap
 */
static void
wlc_ampdu_rx_cleanup(ampdu_rx_info_t *ampdu_rx, wlc_bsscfg_t *bsscfg,
	uint16 conf_TID_bmap)
{
	uint8 tid;
	scb_ampdu_rx_t *scb_ampdu = NULL;
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_info_t *wlc = ampdu_rx->wlc;

	scb_ampdu_tid_resp_t *resp;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (!SCB_AMPDU(scb)) {
			continue;
		}

		if (!SCB_ASSOCIATED(scb)) {
			continue;
		}

		scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			if (!(isbitset(conf_TID_bmap, tid))) {
				continue;
			}

			resp = scb_ampdu->resp[tid];

			if (resp != NULL) {
				if ((resp->ba_state == AMPDU_TID_STATE_BA_ON) ||
					(resp->ba_state == AMPDU_TID_STATE_BA_PENDING_ON))
					wlc_ampdu_rx_send_delba(ampdu_rx, scb, tid, FALSE,
						DOT11_RC_TIMEOUT);

				ampdu_cleanup_tid_resp(ampdu_rx, scb, tid);
			}
		}
	}
}

/** resends ADDBA-Req if the ADDBA-Resp has not come back */
static void
wlc_ampdu_rx_watchdog(void *hdl)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)hdl;
	wlc_info_t *wlc = ampdu_rx->wlc;
	scb_ampdu_rx_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;
	scb_ampdu_tid_resp_off_t *resp_off;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb))
			continue;
		scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {

			resp = scb_ampdu->resp[tid];
			resp_off = &scb_ampdu->resp_off[tid];

			if (resp) {
				resp_off->ampdu_cnt = 0;
				resp_off->ampdu_recd = FALSE;
			}
			if (resp_off->ampdu_recd) {
				resp_off->ampdu_recd = FALSE;
				resp_off->ampdu_cnt++;
				if (resp_off->ampdu_cnt >= AMPDU_RESP_NO_BAPOLICY_TIMEOUT) {
					resp_off->ampdu_cnt = 0;
					WL_ERROR(("wl%d: %s: ampdus recd for"
						" tid %d with no BA policy in effect\n",
						ampdu_rx->wlc->pub->unit, __FUNCTION__, tid));
					wlc_ampdu_rx_send_delba(ampdu_rx, scb, tid,
						FALSE, DOT11_RC_SETUP_NEEDED);
				}
			}
		}
	}
}

/** handle AMPDU related iovars */
static int
wlc_ampdu_rx_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	ampdu_rx_info_t *ampdu_rx = (ampdu_rx_info_t *)hdl;
	ampdu_rx_config_t *ampdu_rx_cfg = ampdu_rx->config;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *) a;
	int err = 0;
	wlc_info_t *wlc;
	bool bool_val;
	wlc_bsscfg_t *bsscfg;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);

	wlc = ampdu_rx->wlc;
	ASSERT(ampdu_rx == wlc->ampdu_rx);

	if (ampdu_rx->rxaggr_support == FALSE) {
		WL_OID(("wl%d: %s: ampdu_rx->rxaggr_support is FALSE\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	BCM_REFERENCE(bsscfg);

	switch (actionid) {
	case IOV_GVAL(IOV_AMPDU_RX):
		*ret_int_ptr = (int32)wlc->pub->_ampdu_rx;
		break;

	case IOV_SVAL(IOV_AMPDU_RX):
		return wlc_ampdu_rx_set(ampdu_rx, (bool)int_val);

	case IOV_GVAL(IOV_AMPDU_RX_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)p;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		ampdu_tid->enable = ampdu_rx_cfg->rxba_enable[ampdu_tid->tid];
		bcopy(ampdu_tid, a, sizeof(*ampdu_tid));
		break;
		}

	case IOV_SVAL(IOV_AMPDU_RX_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)a;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		ampdu_rx_cfg->rxba_enable[ampdu_tid->tid] = ampdu_tid->enable ? TRUE : FALSE;
		break;
		}

	case IOV_GVAL(IOV_AMPDU_RX_FACTOR):
		*ret_int_ptr = (int32)ampdu_rx_cfg->rx_factor;
		break;

	case IOV_GVAL(IOV_AMPDU_RX_DENSITY):
		*ret_int_ptr = (int32)ampdu_rx_cfg->mpdu_density;
		break;

	case IOV_SVAL(IOV_AMPDU_RX_DENSITY):
		if (int_val > AMPDU_MAX_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}

		if (int_val < AMPDU_DEF_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}
		ampdu_rx_cfg->mpdu_density = (uint8)int_val;
		wlc_ampdu_update_ie_param(wlc->ampdu_rx);
		break;

	case IOV_SVAL(IOV_AMPDU_RX_FACTOR):
#ifdef WL11AC
		if (VHT_ENAB(wlc->pub)) {
			if (int_val > AMPDU_RX_FACTOR_1024K) {
				err = BCME_RANGE;
				break;
			}

			ampdu_rx_cfg->rx_factor = (uint8)int_val;
			wlc_vht_update_ampdu_cap(wlc->vhti, ampdu_rx_cfg->rx_factor);
			wlc_ampdu_update_ie_param(ampdu_rx);
			break;
		}
#endif /* WL11AC */
		/* limit to the max aggregation size possible based on chip
		 * limitations
		 */
		if ((int_val > AMPDU_RX_FACTOR_64K) ||
		    (int_val > AMPDU_RX_FACTOR_32K &&
		     D11REV_LE(wlc->pub->corerev, 11))) {
			err = BCME_RANGE;
			break;
		}
		ampdu_rx_cfg->rx_factor = (uint8)int_val;
		wlc_ampdu_update_ie_param(ampdu_rx);
		break;

#ifdef  WLAMPDU_HOSTREORDER
	case IOV_GVAL(IOV_AMPDU_HOSTREORDER):
		*ret_int_ptr = (int32)wlc->pub->_ampdu_hostreorder;
		break;

	case IOV_SVAL(IOV_AMPDU_HOSTREORDER):
		wlc->pub->_ampdu_hostreorder = bool_val;
		break;
#endif /* WLAMPDU_HOSTREORDER */
	case IOV_GVAL(IOV_AMPDU_RXAGGR):
	{
		struct ampdu_aggr *rxaggr = p;
		bsscfg_ampdu_rx_t *bsscfg_ampdu = BSSCFG_AMPDU_RX_CUBBY(ampdu_rx, bsscfg);
		bzero(rxaggr, sizeof(*rxaggr));
		rxaggr->aggr_override = bsscfg_ampdu->rxaggr_override;
		rxaggr->enab_TID_bmap = bsscfg_ampdu->rxaggr_TID_bmap;
		bcopy(rxaggr, a, sizeof(*rxaggr));
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

/** enable/disable rxaggr_override control.
 * AUTO: rxaggr operates according to per-TID per-bsscfg control()
 * OFF: turn rxaggr off for all TIDs.
 * ON: Not supported and treated the same as AUTO.
 */
void
wlc_ampdu_rx_set_bsscfg_aggr_override(ampdu_rx_info_t *ampdu_rx, wlc_bsscfg_t *bsscfg, int8 rxaggr)
{
	bsscfg_ampdu_rx_t *bsscfg_ampdu = BSSCFG_AMPDU_RX_CUBBY(ampdu_rx, bsscfg);

	if (ampdu_rx->rxaggr_support == FALSE) {
		/* rxaggr_override should already be OFF */
		ASSERT(bsscfg_ampdu->rxaggr_override == OFF);
		return;
	}

	/* rxaggr_override ON would mean that rx aggregation will be allowed for all TIDs
	 * even if bsscfg_ampdu->rxaggr_TID_bmap is set OFF for some TIDs.
	 * As there is no requirement of such rxaggr_override ON, just treat it as AUTO.
	 */
	if (rxaggr == ON) {
		rxaggr = AUTO;
	}

	if (bsscfg_ampdu->rxaggr_override == rxaggr) {
		return;
	}

	bsscfg_ampdu->rxaggr_override = rxaggr;

	if (rxaggr == OFF) {
		wlc_ampdu_rx_cleanup(ampdu_rx, bsscfg, AMPDU_ALL_TID_BITMAP);
	}
}

/** Configure ampdu rx aggregation per-TID and per-bsscfg */
void
wlc_ampdu_rx_set_bsscfg_aggr(ampdu_rx_info_t *ampdu_rx, wlc_bsscfg_t *bsscfg,
	bool rxaggr, uint16 conf_TID_bmap)
{
	bsscfg_ampdu_rx_t *bsscfg_ampdu = BSSCFG_AMPDU_RX_CUBBY(ampdu_rx, bsscfg);

	if (ampdu_rx->rxaggr_support == FALSE) {
		/* rxaggr should already be OFF for all TIDs,
		 * and do not set rxaggr_TID_bmap.
		 */
		ASSERT(bsscfg_ampdu->rxaggr_TID_bmap == 0);
		return;
	}

	if (rxaggr == ON) {
		bsscfg_ampdu->rxaggr_TID_bmap |= (conf_TID_bmap & AMPDU_ALL_TID_BITMAP);
	} else {
		uint16 stateChangedTID = bsscfg_ampdu->rxaggr_TID_bmap;
		bsscfg_ampdu->rxaggr_TID_bmap &= ((~conf_TID_bmap) & AMPDU_ALL_TID_BITMAP);
		stateChangedTID ^= bsscfg_ampdu->rxaggr_TID_bmap;
		stateChangedTID &= AMPDU_ALL_TID_BITMAP;

		/* Override should have higher priority if not AUTO */
		if (bsscfg_ampdu->rxaggr_override == AUTO && stateChangedTID) {
			wlc_ampdu_rx_cleanup(ampdu_rx, bsscfg, stateChangedTID);
		}
	}
}

/* ampdu_create_f() has to be kept up to date as fields get added to 'f' */

/**
 * Called when a frame is received. The WLC layer makes use of a meta structure containing
 * information about a received frame ('wlc_frminfo'). Before passing a received MPDU up to the
 * (higher) WLC layer, this caller supplied meta structure 'f' is initialized using information held
 * by caller supplied packet 'p' and caller supplied d11rxhdr_t (which was generated by ucode).
 */
static void BCMFASTPATH
ampdu_create_f(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f, void *p,
	wlc_d11rxhdr_t *wrxh)
{
	uint16 offset = DOT11_A3_HDR_LEN;

	bzero((void *)f, sizeof(struct wlc_frminfo));
	f->p = p;
	f->h = (struct dot11_header *) PKTDATA(wlc->osh, f->p);
	f->fc = ltoh16(f->h->fc);
	f->type = FC_TYPE(f->fc);
	f->subtype = (f->fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT;
	f->ismulti = ETHER_ISMULTI(&(f->h->a1));
	f->len = PKTLEN(wlc->osh, f->p) + PKTFRAGUSEDLEN(wlc->osh, f->p);
	f->seq = ltoh16(f->h->seq);
#if defined(WLTDLS)
	f->istdls =  BSSCFG_IS_TDLS(scb->bsscfg);
#endif /* WLTDLS */
	f->wds = ((f->fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
	if (f->wds)
		offset += ETHER_ADDR_LEN;
	f->pbody = (uchar*)(f->h) + offset;

	/* account for QoS Control Field */
	f->qos = (f->type == FC_TYPE_DATA && FC_SUBTYPE_ANY_QOS(f->subtype));
	if (f->qos) {
		uint16 qoscontrol = ltoh16_ua(f->pbody);
		f->isamsdu = (qoscontrol & QOS_AMSDU_MASK) != 0;
		f->prio = (uint8)QOS_PRIO(qoscontrol);
		f->ac = WME_PRIO2AC(f->prio);
		f->apsd_eosp = QOS_EOSP(qoscontrol);
		f->pbody += DOT11_QOS_LEN;
		offset += DOT11_QOS_LEN;
	}
	f->ht = ((wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
		((f->fc & FC_ORDER) && FC_SUBTYPE_ANY_QOS(f->subtype));
	if (f->ht) {
		f->pbody += DOT11_HTC_LEN;
		offset += DOT11_HTC_LEN;
	}

	f->body_len = f->len - offset;
	f->totlen = pkttotlen(wlc->osh, p) - offset;
	/* AMPDUXXX: WPA_auth may not be valid for wds */
	f->WPA_auth = scb->WPA_auth;
	f->wrxh = wrxh;
	f->rxh = &wrxh->rxhdr;
	f->rx_wep = 0;
	f->key = NULL;
}

#if defined(PKTC) || defined(PKTC_DONGLE) /* receive packet chaining */

bool BCMFASTPATH
wlc_ampdu_chainable(ampdu_rx_info_t *ampdu_rx, void *p, struct scb *scb, uint16 seq, uint16 tid)
{
	scb_ampdu_rx_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;
	uint16 indx;

	scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
	ASSERT(scb_ampdu != NULL);
	resp = scb_ampdu->resp[tid];

	/* return if ampdu_rx not enabled on TID */
	if (resp == NULL)
		return FALSE;

	/* send up if expected seq */
	seq = seq >> SEQNUM_SHIFT;
	if (seq != resp->exp_seq) {
		WLCNTINCR(ampdu_rx->cnt->rxoos);
		return FALSE;
	}

	resp->alive = TRUE;

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub) && resp->queued) {
		return FALSE;
	}
#endif // endif

	indx = RX_SEQ_TO_INDEX(ampdu_rx, NEXT_SEQ(resp->exp_seq));
	if (AMPDU_RXQ_HASPKT(resp, indx))
		return FALSE;

	resp->exp_seq = NEXT_SEQ(resp->exp_seq);

	return TRUE;
}

#endif /* PKTC || PKTC_DONGLE */

/** called on packet reception */
void
wlc_ampdu_update_rxcounters(ampdu_rx_info_t *ampdu_rx, uint32 ft, struct scb *scb,
	struct dot11_header *h, void *p, uint8 prio)
{
	scb_ampdu_rx_t *scb_ampdu;
	uint8 *plcp;
#ifdef WL11K
	wlc_info_t *wlc = ampdu_rx->wlc;
#else
	UNUSED_PARAMETER(prio);
#endif // endif

	scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
	BCM_REFERENCE(scb_ampdu);
	ASSERT(scb_ampdu != NULL);

	plcp = ((uint8 *)h) - D11_PHY_HDR_LEN;
	if (ft == PRXS0_PREN) {
		if (WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
			WLCNTINCR(ampdu_rx->cnt->rxampdu);
			WLCNTINCR(ampdu_rx->cnt->rxmpdu);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.rxampdu);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.rxmpdu);
#ifdef WL11K
			wlc_rrm_stat_qos_counter(scb, prio, OFFSETOF(rrm_stat_group_qos_t, rxmpdu));
			WLCNTADD(ampdu_rx->cnt->rxampdubyte_l, PKTLEN(wlc->osh, p));
			if (ampdu_rx->cnt->rxampdubyte_l < PKTLEN(wlc->osh, p))
				WLCNTINCR(ampdu_rx->cnt->rxampdubyte_h);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || (defined(WLTEST) && \
	!defined(WLTEST_DISABLED)) || defined(BCMDBG_AMPDU)
			if (ampdu_rx->amdbg && VALID_MCS(plcp[0] & MIMO_PLCP_MCS_MASK))
				ampdu_rx->amdbg->rxmcs[MCS2IDX(plcp[0] & MIMO_PLCP_MCS_MASK)]++;
#endif // endif
			if (PLCP3_ISSGI(plcp[3])) {
				WLCNTINCR(ampdu_rx->cnt->rxampdu_sgi);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || (defined(WLTEST) && \
	!defined(WLTEST_DISABLED)) || defined(BCMDBG_AMPDU)
			if (ampdu_rx->amdbg && VALID_MCS(plcp[0] & MIMO_PLCP_MCS_MASK))
					ampdu_rx->amdbg->rxmcssgi[
						MCS2IDX(plcp[0] & MIMO_PLCP_MCS_MASK)]++;
#endif // endif
			}
			if (PLCP3_ISSTBC(plcp[3])) {
				WLCNTINCR(ampdu_rx->cnt->rxampdu_stbc);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || (defined(WLTEST) && \
	!defined(WLTEST_DISABLED)) || defined(BCMDBG_AMPDU)
				if (ampdu_rx->amdbg && VALID_MCS(plcp[0] & MIMO_PLCP_MCS_MASK))
					ampdu_rx->amdbg->rxmcsstbc[
						MCS2IDX(plcp[0] & MIMO_PLCP_MCS_MASK)]++;
#endif // endif
			}
		} else if (!(plcp[0] | plcp[1] | plcp[2])) {
			WLCNTINCR(ampdu_rx->cnt->rxmpdu);
#ifdef WL11K
			WLCNTADD(ampdu_rx->cnt->rxampdubyte_l, PKTLEN(wlc->osh, p));
			if (ampdu_rx->cnt->rxampdubyte_l < PKTLEN(wlc->osh, p))
				WLCNTINCR(ampdu_rx->cnt->rxampdubyte_h);
#endif // endif
		} else
			WLCNTINCR(ampdu_rx->cnt->rxht);
	}
#ifdef WL11AC
	 else if (ft == FT_VHT) {
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || (defined(WLTEST) && \
	!defined(WLTEST_DISABLED)) || defined(BCMDBG_AMPDU)
		uint8  vht = 0;
#endif // endif
		if ((plcp[0] | plcp[1] | plcp[2])) {
			WLCNTINCR(ampdu_rx->cnt->rxampdu);
			WLCNTINCR(ampdu_rx->cnt->rxmpdu);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.rxampdu);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.rxmpdu);
#ifdef WL11K
			wlc_rrm_stat_qos_counter(scb, prio, OFFSETOF(rrm_stat_group_qos_t, rxmpdu));
			WLCNTADD(ampdu_rx->cnt->rxampdubyte_l, PKTLEN(wlc->osh, p));
			if (ampdu_rx->cnt->rxampdubyte_l < PKTLEN(wlc->osh, p))
				WLCNTINCR(ampdu_rx->cnt->rxampdubyte_h);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || (defined(WLTEST) && \
	!defined(WLTEST_DISABLED)) || defined(BCMDBG_AMPDU)
			if (ampdu_rx->amdbg) {
				vht = wlc_vht_get_rate_from_plcp(plcp);
				ASSERT(vht & 0x80);
				vht  = (vht & 0xf) + (((vht & 0x70) >> 4)-1) * MAX_VHT_RATES;
				ampdu_rx->amdbg->rxvht[vht]++;
			}
#endif // endif
			if (VHT_PLCP3_ISSGI(plcp[3])) {
				WLCNTINCR(ampdu_rx->cnt->rxampdu_sgi);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || (defined(WLTEST) && \
	!defined(WLTEST_DISABLED)) || defined(BCMDBG_AMPDU)
				if (ampdu_rx->amdbg)
					ampdu_rx->amdbg->rxvhtsgi[vht]++;
#endif // endif
			}
			if (VHT_PLCP0_ISSTBC(plcp[0])) {
				WLCNTINCR(ampdu_rx->cnt->rxampdu_stbc);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || (defined(WLTEST) && \
	!defined(WLTEST_DISABLED)) || defined(BCMDBG_AMPDU)
				if (ampdu_rx->amdbg)
					ampdu_rx->amdbg->rxvhtstbc[vht]++;
#endif // endif
			}
		} else {
			WLCNTINCR(ampdu_rx->cnt->rxmpdu);
#ifdef WL11K
			WLCNTADD(ampdu_rx->cnt->rxampdubyte_l, PKTLEN(wlc->osh, p));
			if (ampdu_rx->cnt->rxampdubyte_l < PKTLEN(wlc->osh, p))
				WLCNTINCR(ampdu_rx->cnt->rxampdubyte_h);
#endif // endif
		}
	}
#endif /* WL11AC */
	else {
		WLCNTINCR(ampdu_rx->cnt->rxlegacy);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.rxlegacy);
	}
} /* wlc_ampdu_update_rxcounters */

/**
 * called by higher (wlc.c) layer when an AMPDU frame was received. MPDU's need to be extracted
 * from the frame and passed on to the wlc.c layer, ACKs/NAKs need to be sent to the remote party,
 * receive window needs to be moved.
 */
void BCMFASTPATH
wlc_ampdu_recvdata(ampdu_rx_info_t *ampdu_rx, struct scb *scb, struct wlc_frminfo *f)
{
	scb_ampdu_rx_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;
	wlc_info_t *wlc;
	uint16 seq, offset, indx, delta;
	uint8 *plcp;
	uint8 tid = f->prio;
	uint8  vht = 0;
	bool new_hole = FALSE;
	BCM_REFERENCE(vht);
	BCM_REFERENCE(new_hole);

	wlc = ampdu_rx->wlc;

	if (f->subtype != FC_SUBTYPE_QOS_DATA) {
		wlc_recvdata_ordered(wlc, scb, f);
		return;
	}

	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	ASSERT(tid < AMPDU_MAX_SCB_TID);
	ASSERT(!f->ismulti);

	scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
	ASSERT(scb_ampdu);

	plcp = ((uint8 *)(f->h)) - D11_PHY_HDR_LEN;

	wlc_ampdu_update_rxcounters(ampdu_rx,
		f->rxh->PhyRxStatus_0 & PRXS0_FT_MASK, scb, f->h, f->p, tid);

#if defined(WL_MU_RX) && defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || \
	defined(BCMDBG_MU) || defined(BCMDBG_DUMP))
		if (MU_RX_ENAB(wlc)) {
			wlc_murx_update_rxcounters(wlc->murx, f->rxh->PhyRxStatus_0 & PRXS0_FT_MASK,
				scb, f->h);
		}
#endif  /* WL_MU_RX && (BCMDBG || WLDUMP) */

	resp = scb_ampdu->resp[tid];

	/* return if ampdu_rx not enabled on TID */
	if (resp == NULL) {
		if (((f->rxh->PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
			WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
			scb_ampdu->resp_off[tid].ampdu_recd = TRUE;
			WLCNTINCR(ampdu_rx->cnt->rxnobapol);
		}
		wlc_recvdata_ordered(wlc, scb, f);
		return;
	}

	/* track if receiving aggregates from non-HT device */
	if (!SCB_HT_CAP(scb) &&
	    ((f->rxh->PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
	    WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
		scb_ampdu->resp_off[tid].ampdu_recd = TRUE;
		WLCNTINCR(ampdu_rx->cnt->rxnobapol);
	}

	/* fragments not allowed */
	if (f->seq & FRAGNUM_MASK) {
		WL_ERROR(("wl%d: %s: unexp frag seq 0x%x, exp seq 0x%x\n",
			wlc->pub->unit, __FUNCTION__, f->seq, resp->exp_seq));
		goto toss;
	}

	seq = f->seq >> SEQNUM_SHIFT;
	indx = RX_SEQ_TO_INDEX(ampdu_rx, seq);

	WL_AMPDU_RX(("wl%d: %s: receiving seq 0x%x tid %d, exp seq %d indx %d\n",
	          wlc->pub->unit, __FUNCTION__, seq, tid, resp->exp_seq, indx));

	/* send up if expected seq */
	if (seq == resp->exp_seq) {
		uint bandunit;
		struct ether_addr ea;
		struct scb *newscb;
		wlc_bsscfg_t *bsscfg;
		bool update_host = FALSE;

		BCM_REFERENCE(update_host);

		ASSERT(resp->exp_seq == pkt_h_seqnum(wlc, f->p));

#ifdef  WLAMPDU_HOSTREORDER
		if (AMPDU_HOST_REORDER_ENAB(wlc->pub))
			ASSERT(!AMPDU_CHECK_HOST_HASPKT(resp, indx));
		else
#endif // endif
			ASSERT(!AMPDU_RXQ_HASPKT(resp, indx));
		resp->alive = TRUE;
		resp->exp_seq = NEXT_SEQ(resp->exp_seq);

		bsscfg = scb->bsscfg;
		bandunit = scb->bandunit;
		bcopy(&scb->ea, &ea, ETHER_ADDR_LEN);

#ifdef  WLAMPDU_HOSTREORDER
		if (!AMPDU_HOST_REORDER_ENAB(wlc->pub))
#endif // endif
		{
			wlc_recvdata_ordered(wlc, scb, f);
			/* validate that the scb is still around and some path in
			 * wlc_recvdata_ordered() did not free it
			*/
			newscb = wlc_scbfindband(wlc, bsscfg, &ea, bandunit);
			if ((newscb == NULL) || (newscb != scb)) {
				WL_ERROR(("wl%d: %s: scb freed; bail out\n",
					wlc->pub->unit, __FUNCTION__));
				return;
			}
		}
		/* release pending ordered packets */
		WL_AMPDU_RX(("wl%d: %s: Releasing pending packets\n",
			wlc->pub->unit, __FUNCTION__));

#ifdef  WLAMPDU_HOSTREORDER
		if (AMPDU_HOST_REORDER_ENAB(wlc->pub) && resp->queued) {
			if (BCMPCIEDEV_ENAB())
				AMPDU_CHAIN_RXCPLID_RESET(&resp->rxcpl_list);
			update_host = TRUE;
		}
#endif // endif

		wlc_ampdu_release_ordered(wlc, scb_ampdu, tid);

#ifdef  WLAMPDU_HOSTREORDER
		if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
			/* exp_seq is updated now, cur is known,
			 * set the right flags and call wlc_recvdata_ordered
			*/
			if (update_host) {
				wlc_ampdu_setpkt_hostreorder_info(wlc, resp, f->p, indx, NO_NEWHOLE,
					NO_DEL_FLOW, NO_FLUSH_ALL);
				if (BCMPCIEDEV_ENAB()) {
					/* chain rxcpl of amsdu subframes before linking list */
					AMPDU_CONSOLIDATE_AMSDU_RXCHAIN(wlc->osh, f->p, FALSE);
					AMPDU_CHAIN_RXCPLID_HEAD(&resp->rxcpl_list,
						PKTRXCPLID(wlc->osh, f->p));
					AMPDU_CHAIN_RXCPLID_RESET(&resp->rxcpl_list);
				}
			}
			if (BCMPCIEDEV_ENAB())
				PKTRESETNORXCPL(wlc->osh, f->p);
			wlc_recvdata_ordered(wlc, scb, f);
		}
#endif /* WLAMPDU_HOSTREORDER */
		return;
	}

	/* out of order packet; validate and enq */
	offset = MODSUB_POW2(seq, resp->exp_seq, SEQNUM_MAX);

	/* check for duplicate or beyond half the sequence space */
	if (((offset < resp->ba_wsize) && AMPDU_IS_PKT_PENDING(wlc, resp, indx)) ||
		(offset > (SEQNUM_MAX >> 1))) {
		ASSERT(seq == pkt_h_seqnum(wlc, f->p));
		WL_AMPDU_RX(("wl%d: wlc_ampdu_recvdata: duplicate seq 0x%x(dropped)\n",
			wlc->pub->unit, seq));
		PKTFREE(wlc->osh, f->p, FALSE);
		WLCNTINCR(ampdu_rx->cnt->rxdup);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.rxdup);
		return;
	}
#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(wlc->pub) && (resp->queued == 0)) {
		new_hole = TRUE;
	}
#endif // endif

	/* move the start of window if acceptable out of window pkts */
	if (offset >= resp->ba_wsize) {
		delta = offset - resp->ba_wsize + 1;
		WL_AMPDU_RX(("wl%d: wlc_ampdu_recvdata: out of window pkt with"
			" seq 0x%x delta %d (exp seq 0x%x): moving window fwd\n",
			wlc->pub->unit, seq, delta, resp->exp_seq));

		if (AMPDU_HOST_REORDER_ENAB(wlc->pub) && BCMPCIEDEV_ENAB()) {
			AMPDU_CHAIN_RXCPLID_RESET(&resp->rxcpl_list);
		}

		wlc_ampdu_release_n_ordered(wlc, scb_ampdu, tid, delta);

		/* recalc resp since may have been freed while releasing frames */
		if ((resp = scb_ampdu->resp[tid])) {

#ifdef  WLAMPDU_HOSTREORDER
			if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
				ASSERT(!AMPDU_CHECK_HOST_HASPKT(resp, indx));
				/* set the index to say pkt is pending */
				AMPDU_SET_HOST_HASPKT(resp, indx, wlc->osh, f->p);
			}
			else
#endif // endif
			{
				ASSERT(!AMPDU_RXQ_HASPKT(resp, indx)); /* rxq[indx] is free */
				ASSERT(f->p != NULL);

				AMPDU_RXQ_SETPKT(resp, indx, f->p); /* save packet ptr */
				AMPDU_SET_WRXH(resp, indx, f->p, f->wrxh); /* save d11rxhdr */
			}

			resp->queued++;
			if (ampdu_rx->resp_timer_running == FALSE) {
				ampdu_rx->resp_timer_running = TRUE;
				wl_add_timer(wlc->wl, ampdu_rx->resp_timer,
					AMPDU_RESP_TIMEOUT, TRUE);
			}
		}
		wlc_ampdu_release_ordered(wlc, scb_ampdu, tid);

#ifdef  WLAMPDU_HOSTREORDER
		if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
			if (delta > resp->ba_wsize) {
				wlc_ampdu_setpkt_hostreorder_info(wlc, resp, f->p, indx, new_hole,
					NO_DEL_FLOW, FLUSH_ALL);
			}
			else {
				wlc_ampdu_setpkt_hostreorder_info(wlc, resp, f->p, indx, new_hole,
					NO_DEL_FLOW, NO_FLUSH_ALL);
			}
			wlc_recvdata_ordered(wlc, scb, f);
			if (BCMPCIEDEV_ENAB()) {
				if (!AMPDU_CHAIN_RXCPLID_EMPTY(&resp->rxcpl_list)) {
					AMPDU_CHAIN_RXCPLID_FLUSH(wlc->wl, &resp->rxcpl_list);
				}
			}
		}
#endif /* WLAMPDU_HOSTREORDER */

		WLCNTINCR(ampdu_rx->cnt->rxoow);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.rxoow);
		return;
	}

	WL_AMPDU_RX(("wl%d: wlc_ampdu_recvdata: q out of order seq 0x%x(exp 0x%x)\n",
		wlc->pub->unit, seq, resp->exp_seq));

	resp->queued++;

	if (AMPDU_HOST_REORDER_ENAB(wlc->pub)) {
		ASSERT(!AMPDU_CHECK_HOST_HASPKT(resp, indx));
		/* set the index to say pkt is pending */
		AMPDU_SET_HOST_HASPKT(resp, indx, wlc->osh, f->p);
		wlc_ampdu_setpkt_hostreorder_info(wlc, resp, f->p, indx, new_hole,
			NO_DEL_FLOW, NO_FLUSH_ALL);
		wlc_recvdata_ordered(wlc, scb, f);
	}
	else {
		ASSERT(!AMPDU_RXQ_HASPKT(resp, indx)); /* rxq[indx] is free */

		ASSERT(f->p != NULL);

		AMPDU_RXQ_SETPKT(resp, indx, f->p); /* save packet pointer */
		AMPDU_SET_WRXH(resp, indx, f->p, f->wrxh); /* save d11rxhdr pointer */
	}
	if (ampdu_rx->resp_timer_running == FALSE) {
		ampdu_rx->resp_timer_running = TRUE;
		wl_add_timer(wlc->wl, ampdu_rx->resp_timer, AMPDU_RESP_TIMEOUT, TRUE);
	}

	WLCNTINCR(ampdu_rx->cnt->rxqed);

	return;

toss:
	WL_AMPDU_RX(("wl%d: %s: Received some unexpected packets\n", wlc->pub->unit, __FUNCTION__));
	PKTFREE(wlc->osh, f->p, FALSE);
	WLCNTINCR(ampdu_rx->cnt->rxunexp);

	/* AMPDUXXX: protocol failure, send delba */
	wlc_ampdu_rx_send_delba(ampdu_rx, scb, tid, FALSE, DOT11_RC_UNSPECIFIED);
} /* wlc_ampdu_recvdata */

/**
 * Called when setting up or tearing down an AMPDU connection with a remote party (scb) for a
 * specific traffic class (tid).
 */
void
ampdu_cleanup_tid_resp(ampdu_rx_info_t *ampdu_rx, struct scb *scb, uint8 tid)
{
	scb_ampdu_rx_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;

	scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
	ASSERT(scb_ampdu);
	ASSERT(scb_ampdu->scb);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	AMPDU_VALIDATE_TID(ampdu_rx, tid, "ampdu_cleanup_tid_resp");

	if (scb_ampdu->resp[tid] == NULL)
		return;
	resp = scb_ampdu->resp[tid];

	WL_AMPDU_CTL(("wl%d: ampdu_cleanup_tid_resp: tid %d\n", ampdu_rx->wlc->pub->unit, tid));

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub) && !BCMPCIEDEV_ENAB()) {
		wlc_ampdu_free_flow_id(ampdu_rx, scb_ampdu->resp[tid], scb_ampdu->scb);
	}
#endif /* WLAMPDU_HOSTREORDER */

	/* send up all the pending pkts in order from the rx reorder q going over holes */
	wlc_ampdu_release_n_ordered(ampdu_rx->wlc, scb_ampdu, tid,
		ampdu_rx->config->ba_max_rx_wsize);

	/* recheck scb_ampdu->resp[] since it may have been freed while releasing */
	if ((resp = scb_ampdu->resp[tid])) {
		ASSERT(resp->queued == 0);
#ifdef WLAMPDU_HOSTREORDER
		if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub)) {
			if (BCMPCIEDEV_ENAB()) {
				AMPDU_CHAIN_RXCPLID_FLUSH(ampdu_rx->wlc->wl, &resp->rxcpl_list);
			}
			else if (resp->tohost_ctrlpkt != NULL)
				PKTFREE(ampdu_rx->wlc->osh, resp->tohost_ctrlpkt, FALSE);
		}
#endif /* WLAMPDU_HOSTREORDER */
		MFREE(ampdu_rx->wlc->osh, resp, sizeof(scb_ampdu_tid_resp_t));
		scb_ampdu->resp[tid] = NULL;
	}

	ampdu_rx->resp_cnt--;
	if ((ampdu_rx->resp_cnt == 0) && (ampdu_rx->resp_timer_running == TRUE)) {
		wl_del_timer(ampdu_rx->wlc->wl, ampdu_rx->resp_timer);
		ampdu_rx->resp_timer_running = FALSE;
	}
}

/** remote party (scb) requests us to set up an AMPDU connection */
void
wlc_ampdu_recv_addba_req_resp(ampdu_rx_info_t *ampdu_rx, struct scb *scb,
	dot11_addba_req_t *addba_req, int body_len)
{
	scb_ampdu_rx_t *scb_ampdu_rx;
	bsscfg_ampdu_rx_t *bsscfg_ampdu_rx;
	ampdu_rx_config_t *ampdu_rx_cfg;
	scb_ampdu_tid_resp_t *resp;
	uint16 param_set, timeout, start_seq;
	uint8 tid, wsize, policy;
	void *tohost_ctrlpkt = NULL;

	BCM_REFERENCE(tohost_ctrlpkt);

	ASSERT(scb);
	ASSERT(ampdu_rx);

	scb_ampdu_rx = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
	ASSERT(scb_ampdu_rx);
	ampdu_rx_cfg = scb_ampdu_rx->ampdu_rx->config;

	bsscfg_ampdu_rx = BSSCFG_AMPDU_RX_CUBBY(ampdu_rx, SCB_BSSCFG(scb));
	ASSERT(bsscfg_ampdu_rx);

	timeout = ltoh16_ua(&addba_req->timeout);
	start_seq = ltoh16_ua(&addba_req->start_seqnum);
	param_set = ltoh16_ua(&addba_req->addba_param_set);

	/* accept the min of our and remote timeout */
	timeout = MIN(timeout, ampdu_rx_cfg->delba_timeout);

	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu_rx, tid, "wlc_ampdu_recv_addba_req_resp");

	if (bsscfg_ampdu_rx->rxaggr_override == OFF ||
		!isbitset(bsscfg_ampdu_rx->rxaggr_TID_bmap, tid)) {
		wlc_send_addba_resp(ampdu_rx->wlc, scb, DOT11_SC_DECLINED,
			addba_req->token, timeout, param_set);
		return;
	}

	if (!AMPDU_ENAB(ampdu_rx->wlc->pub) || (scb->bsscfg->BSS && !SCB_HT_CAP(scb))) {
		wlc_send_addba_resp(ampdu_rx->wlc, scb, DOT11_SC_DECLINED,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu_rx->cnt->txaddbaresp);
		return;
	}

	if (!ampdu_rx_cfg->rxba_enable[tid]) {
		wlc_send_addba_resp(ampdu_rx->wlc, scb, DOT11_SC_DECLINED,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu_rx->cnt->txaddbaresp);
		return;
	}

	policy = (param_set & DOT11_ADDBA_PARAM_POLICY_MASK) >> DOT11_ADDBA_PARAM_POLICY_SHIFT;
	if (policy != ampdu_rx_cfg->ba_policy) {
		wlc_send_addba_resp(ampdu_rx->wlc, scb, DOT11_SC_INVALID_PARAMS,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu_rx->cnt->txaddbaresp);
		return;
	}

	/* cleanup old state */
	ampdu_cleanup_tid_resp(ampdu_rx, scb, tid);

	ASSERT(scb_ampdu_rx->resp[tid] == NULL);

	resp = MALLOCZ(ampdu_rx->wlc->osh, sizeof(scb_ampdu_tid_resp_t));
	if (resp == NULL) {
		wlc_send_addba_resp(ampdu_rx->wlc, scb, DOT11_SC_FAILURE,
			addba_req->token, timeout, param_set);
		WLCNTINCR(ampdu_rx->cnt->txaddbaresp);
		return;
	}

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub) && !BCMPCIEDEV_ENAB()) {
		tohost_ctrlpkt = PKTGET(ampdu_rx->wlc->osh, TXOFF, FALSE);
		if (tohost_ctrlpkt == NULL) {
			wlc_send_addba_resp(ampdu_rx->wlc, scb, DOT11_SC_FAILURE,
				addba_req->token, timeout, param_set);
			WLCNTINCR(ampdu_rx->cnt->txaddbaresp);

			MFREE(ampdu_rx->wlc->osh, resp, sizeof(scb_ampdu_tid_resp_t));
			return;
		}
	}
#endif /* WLAMPDU_HOSTREORDER */

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub) && !BCMPCIEDEV_ENAB())
		resp->tohost_ctrlpkt = tohost_ctrlpkt;
#endif /* WLAMPDU_HOSTREORDER */

	wsize =	(param_set & DOT11_ADDBA_PARAM_BSIZE_MASK) >> DOT11_ADDBA_PARAM_BSIZE_SHIFT;
	/* accept the min of our and remote wsize if remote has the advisory set */
	if (wsize)
		wsize = MIN(wsize, ampdu_rx_cfg->ba_rx_wsize);
	else
		wsize = ampdu_rx_cfg->ba_rx_wsize;
	WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_req: BA ON: seq 0x%x tid %d wsize %d\n",
		ampdu_rx->wlc->pub->unit, start_seq, tid, wsize));

	param_set &= ~DOT11_ADDBA_PARAM_BSIZE_MASK;
	param_set |= (wsize << DOT11_ADDBA_PARAM_BSIZE_SHIFT) & DOT11_ADDBA_PARAM_BSIZE_MASK;

	scb_ampdu_rx->resp[tid] = resp;
	resp->exp_seq = start_seq >> SEQNUM_SHIFT;
	resp->ba_wsize = wsize;
	resp->ba_state = AMPDU_TID_STATE_BA_ON;

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub) && !BCMPCIEDEV_ENAB())
		resp->flow_id = wlc_ampdu_alloc_flow_id(ampdu_rx);
#endif /* WLAMPDU_HOSTREORDER */

#ifdef WLAMSDU
	/* just clear if we don't support; we advertise our support in our addba req */
	/* setting if other side has it cleared, may result in problems assoc */
	/* we don't support for non-AC; also none for win virtual if */
	if (!(D11REV_GE(ampdu_rx->wlc->pub->corerev, 40) && ampdu_rx->wlc->_rx_amsdu_in_ampdu)) {
		param_set &= ~DOT11_ADDBA_PARAM_AMSDU_SUP;
	}
#if defined(EXT_STA)
	else if (WLEXTSTA_ENAB(ampdu_rx->wlc->pub) && scb->bsscfg &&
		!BSSCFG_HAS_NATIVEIF(scb->bsscfg)) {
		param_set &= ~DOT11_ADDBA_PARAM_AMSDU_SUP;
	}
#endif /* defined(EXT_STA) */

#else
	/* no AMSDU can't support */
	param_set &= ~DOT11_ADDBA_PARAM_AMSDU_SUP;
#endif /* WLAMSDU */

	WLCNTINCR(ampdu_rx->cnt->rxaddbareq);

	wlc_send_addba_resp(ampdu_rx->wlc, scb, DOT11_SC_SUCCESS, addba_req->token,
		timeout, param_set);
	WLCNTINCR(ampdu_rx->cnt->txaddbaresp);
	AMPDUSCBCNTINCR(scb_ampdu_rx->cnt.txaddbaresp);

	ampdu_rx->resp_cnt++;
} /* wlc_ampdu_recv_addba_req_resp */

/**
 * Remote party (scb) requests us to tear down an AMPDU connection for a given traffic class (tid).
 */
void
wlc_ampdu_rx_recv_delba(ampdu_rx_info_t *ampdu_rx, struct scb *scb, uint8 tid, uint8 category,
	uint16 initiator, uint16 reason)
{
	scb_ampdu_rx_t *scb_ampdu_rx;

	ASSERT(scb);

	scb_ampdu_rx = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
	BCM_REFERENCE(scb_ampdu_rx);
	ASSERT(scb_ampdu_rx);

	if (category & DOT11_ACTION_CAT_ERR_MASK) {
		WL_ERROR(("wl%d: %s: unexp error action frame\n",
			ampdu_rx->wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(ampdu_rx->cnt->rxunexp);
		return;
	}

	ampdu_cleanup_tid_resp(ampdu_rx, scb, tid);

	WLCNTINCR(ampdu_rx->cnt->rxdelba);
	AMPDUSCBCNTINCR(scb_ampdu_rx->cnt.rxdelba);

	WL_AMPDU(("wl%d: %s: AMPDU OFF: tid %d initiator %d reason %d\n",
		ampdu_rx->wlc->pub->unit, __FUNCTION__, tid, initiator, reason));
}

/** Remote party sent us a block ack request. Moves the window forward on receipt of a bar. */
void
wlc_ampdu_recv_bar(ampdu_rx_info_t *ampdu_rx, struct scb *scb, uint8 *body, int body_len)
{
	scb_ampdu_rx_t *scb_ampdu_rx;
	struct dot11_bar *bar = (struct dot11_bar *)body;
	scb_ampdu_tid_resp_t *resp;
	uint8 tid;
	uint16 seq, tmp, offset;
	void *p = NULL;

	BCM_REFERENCE(p);

	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	scb_ampdu_rx = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
	ASSERT(scb_ampdu_rx);

	tmp = ltoh16(bar->bar_control);
	tid = (tmp & DOT11_BA_CTL_TID_MASK) >> DOT11_BA_CTL_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu_rx, tid, "wlc_ampdu_recv_bar");

	if (tmp & DOT11_BA_CTL_MTID) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: multi tid not supported\n",
			ampdu_rx->wlc->pub->unit));
		WLCNTINCR(ampdu_rx->cnt->rxunexp);
		return;
	}

	resp = scb_ampdu_rx->resp[tid];
	if (resp == NULL) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: uninitialized tid %d\n",
			ampdu_rx->wlc->pub->unit, tid));
		WLCNTINCR(ampdu_rx->cnt->rxunexp);
		return;
	}

	WLCNTINCR(ampdu_rx->cnt->rxbar);
	AMPDUSCBCNTINCR(scb_ampdu_rx->cnt.rxbar);

	seq = (ltoh16(bar->seqnum)) >> SEQNUM_SHIFT;

	WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: length %d tid %d seq 0x%x\n",
		ampdu_rx->wlc->pub->unit, body_len, tid, seq));

	offset = MODSUB_POW2(seq, resp->exp_seq, SEQNUM_MAX);

	/* ignore if it is in the "old" half of sequence space */
	if (offset > (SEQNUM_MAX >> 1)) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_bar: ignore bar with offset 0x%x\n",
			ampdu_rx->wlc->pub->unit, offset));
		return;
	}

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub) && resp->queued) {
		if (BCMPCIEDEV_ENAB()) {
			AMPDU_CHAIN_RXCPLID_RESET(&resp->rxcpl_list);
		}
		else {
		p = PKTGET(ampdu_rx->wlc->osh, TXOFF, FALSE);
		if (p == NULL) {
			/* what to do...fine if there is going to be more packets from peer */
			return;
		}
		PKTPULL(ampdu_rx->wlc->osh, p, TXOFF);
		PKTSETLEN(ampdu_rx->wlc->osh, p, 0);
	}
	}
#endif // endif
	/* release all received pkts till the seq */
	wlc_ampdu_release_n_ordered(ampdu_rx->wlc, scb_ampdu_rx, tid, offset);

	/* release more pending ordered packets if possible */
	wlc_ampdu_release_ordered(ampdu_rx->wlc, scb_ampdu_rx, tid);

#ifdef WLAMPDU_HOSTREORDER
	if (AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub)) {
		if (BCMPCIEDEV_ENAB()) {
			if (!AMPDU_CHAIN_RXCPLID_EMPTY(&resp->rxcpl_list)) {
				AMPDU_CHAIN_RXCPLID_FLUSH(ampdu_rx->wlc->wl, &resp->rxcpl_list);
			}
		}
		else if (p != NULL) {
	/* send this info to Host move the exp_seq to new value */
		if (offset > ampdu_rx->config->ba_max_rx_wsize) {
			wlc_ampdu_setpkt_hostreorder_info(ampdu_rx->wlc, resp,
				p, AMPDU_INVALID_INDEX, NO_NEWHOLE, NO_DEL_FLOW, FLUSH_ALL);
		}
		else {
			wlc_ampdu_setpkt_hostreorder_info(ampdu_rx->wlc, resp, p,
				AMPDU_INVALID_INDEX, NO_NEWHOLE, NO_DEL_FLOW, NO_FLUSH_ALL);
		}
		WLPKTTAGBSSCFGSET(p, scb->bsscfg->_idx);
		wl_sendup(ampdu_rx->wlc->wl, SCB_INTERFACE(scb), p, 1);
	}
	}
#endif /* WLAMPDU_HOSTREORDER */
}

void
wlc_ampdu_rx_send_delba(ampdu_rx_info_t *ampdu_rx, struct scb *scb, uint8 tid,
	uint16 initiator, uint16 reason)
{
	ampdu_cleanup_tid_resp(ampdu_rx, scb, tid);

	WL_AMPDU(("wl%d: %s: tid %d initiator %d reason %d\n",
		ampdu_rx->wlc->pub->unit, __FUNCTION__, tid, initiator, reason));

	wlc_send_delba(ampdu_rx->wlc, scb, tid, initiator, reason);

	WLCNTINCR(ampdu_rx->cnt->txdelba);
}

/** called during system initialization and as a result of a 'wl' command */
int
wlc_ampdu_rx_set(ampdu_rx_info_t *ampdu_rx, bool on)
{
	wlc_info_t *wlc = ampdu_rx->wlc;
	int err = BCME_OK;

	wlc->pub->_ampdu_rx = FALSE;

	if (on) {
		if (!N_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver not nmode enabled\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
		if (!wlc_ampdu_rx_cap(ampdu_rx)) {
			WL_AMPDU_ERR(("wl%d: device not ampdu capable\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
		if (PIO_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver is pio mode\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
	}

	if (wlc->pub->_ampdu_rx != on) {
#ifdef WLCNT
		bzero(ampdu_rx->cnt, sizeof(wlc_ampdu_rx_cnt_t));
#endif // endif
		wlc->pub->_ampdu_rx = on;
	}

exit:
	return err;
}

bool
wlc_ampdu_rx_cap(ampdu_rx_info_t *ampdu_rx)
{
	if (WLC_PHY_11N_CAP(ampdu_rx->wlc->band))
		return TRUE;
	else
		return FALSE;
}

/** rx_factor represents the maximum AMPDU size (normally 64KB) */
INLINE uint8
wlc_ampdu_get_rx_factor(wlc_info_t *wlc)
{
	return wlc->ampdu_rx->config->rx_factor;
}

/** rx_factor represents the maximum AMPDU size (normally 64KB) */
void
wlc_ampdu_update_rx_factor(wlc_info_t *wlc, int vhtmode)
{
	ampdu_rx_info_t *ampdu_rx = wlc->ampdu_rx;
	ampdu_rx_config_t *ampdu_rx_cfg = ampdu_rx->config;

#ifdef WL11AC
	if (vhtmode) {
		/* Initialize VHT AMPDU defaults */
		ampdu_rx_cfg->rx_factor = VHT_DEFAULT_RX_FACTOR;

		/* Update the VHT Capability IE */
		wlc_vht_update_ampdu_cap(wlc->vhti, ampdu_rx_cfg->rx_factor);
	} else
#endif /* WL11AC */
	{
		/* bump max ampdu rcv size to 64k for all 11n devices except 4321A0 and 4321A1 */
		if (WLCISNPHY(wlc->band) && NREV_LT(wlc->band->phyrev, 2))
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_32K;
		else
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_64K;
#ifdef WLC_HIGH_ONLY
		if (wlc->pub->is_ss) {
			/* at super speed BRF1_RX_LARGE_AGG is enabled */
			/* which requires higher rx_factor */
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_64K;
		} else {
			/* Restrict to smaller rcv size for BMAC dongle */
			ampdu_rx_cfg->rx_factor = AMPDU_RX_FACTOR_32K;
		}
#endif // endif
	}

	wlc_ampdu_update_ie_param(ampdu_rx);
}

/**
 * When rx_factor or mpdu_density was changed by higher software layer, beacons and probe responses
 * have to contain these updated values.
 */
void
wlc_ampdu_update_ie_param(ampdu_rx_info_t *ampdu_rx)
{
	wlc_info_t *wlc = ampdu_rx->wlc;
	ampdu_rx_config_t *ampdu_rx_cfg = ampdu_rx->config;
	wlc_ht_update_ampdu_rx_cap_params(wlc->hti,
		ampdu_rx_cfg->rx_factor, ampdu_rx_cfg->mpdu_density);
}

/** Inform ucode on updated parameters */
void
wlc_ampdu_shm_upd(ampdu_rx_info_t *ampdu_rx)
{
	wlc_info_t *wlc = ampdu_rx->wlc;

	if (AMPDU_ENAB(wlc->pub) && (WLC_PHY_11N_CAP(wlc->band)))	{
		/* Extend ucode internal watchdog timer to match larger received frames */
		if (ampdu_rx->config->rx_factor >= AMPDU_RX_FACTOR_64K) {
			wlc_write_shm(wlc, M_MIMO_MAXSYM, MIMO_MAXSYM_MAX);
			wlc_write_shm(wlc, M_WATCHDOG_8TU, WATCHDOG_8TU_MAX);
		} else {
			wlc_write_shm(wlc, M_MIMO_MAXSYM, MIMO_MAXSYM_DEF);
			wlc_write_shm(wlc, M_WATCHDOG_8TU, WATCHDOG_8TU_DEF);
		}
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
int
wlc_ampdu_rx_dump(ampdu_rx_info_t *ampdu_rx, struct bcmstrbuf *b)
{
#ifdef WLCNT
	wlc_ampdu_rx_cnt_t *cnt = ampdu_rx->cnt;
#endif // endif
	int i;
	uint32 max_val;
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_rx_t *scb_ampdu;
	int resp = 0;
	char eabuf[ETHER_ADDR_STR_LEN];
#ifdef WL11AC
	int last;
#endif /* WL11AC */

	bcm_bprintf(b, "AMPDU Rx counters:\n");

#ifdef WLCNT
	bcm_bprintf(b, "rxdelba %d rxunexp %d\n",
		cnt->rxdelba, cnt->rxunexp);
	bcm_bprintf(b, "rxampdu_sgi %d rxampdu_stbc %d\n",
		cnt->rxampdu_sgi, cnt->rxampdu_stbc);
	bcm_bprintf(b, "rxampdu %d rxmpdu %d rxmpduperampdu %d rxht %d rxlegacy %d\n",
		cnt->rxampdu, cnt->rxmpdu,
		cnt->rxampdu ? CEIL(cnt->rxmpdu, cnt->rxampdu) : 0,
		cnt->rxht, cnt->rxlegacy);
	bcm_bprintf(b, "rxholes %d rxqed %d rxdup %d rxnobapol %d "
		"rxstuck %d rxoow %d rxoos %d\n",
		cnt->rxholes, cnt->rxqed, cnt->rxdup, cnt->rxnobapol,
		cnt->rxstuck, cnt->rxoow, cnt->rxoos);
	bcm_bprintf(b, "rxaddbareq %d rxbar %d txba %d\n",
		cnt->rxaddbareq, cnt->rxbar, cnt->txba);

	bcm_bprintf(b, "txaddbaresp %d\n", cnt->txaddbaresp);
#endif /* WLCNT */

	FOREACHSCB(ampdu_rx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
			ASSERT(scb_ampdu);
			for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
				if (scb_ampdu->resp[i])
					resp++;
			}
		}
	}

	bcm_bprintf(b, "resp %d\n", resp);

	for (i = 0, max_val = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
		max_val += ampdu_rx->amdbg->rxmcs[i];
	bcm_bprintf(b, "RX MCS  :");
	if (max_val) {
		for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu_rx->amdbg->rxmcs[i],
				(ampdu_rx->amdbg->rxmcs[i] * 100) / max_val);
			if ((i % 8) == 7 && i != AMPDU_HT_MCS_LAST_EL)
				bcm_bprintf(b, "\n        :");
		}
	}
#ifdef WL11AC
	for (i = 0, max_val = 0, last = 0; i < AMPDU_MAX_VHT; i++) {
		max_val += ampdu_rx->amdbg->rxvht[i];
		if (ampdu_rx->amdbg->rxvht[i]) last = i;
	}
	last = MAX_VHT_RATES * (last/MAX_VHT_RATES + 1) - 1;
	bcm_bprintf(b, "\nRX VHT  :");
	if (max_val) {
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu_rx->amdbg->rxvht[i],
				(ampdu_rx->amdbg->rxvht[i] * 100) / max_val);
			if ((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1) && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
#endif /* WL11AC */
	bcm_bprintf(b, "\n");

	if (WLC_SGI_CAP_PHY(ampdu_rx->wlc)) {
		for (i = 0, max_val = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
			max_val += ampdu_rx->amdbg->rxmcssgi[i];

		bcm_bprintf(b, "RX MCS SGI:");
		if (max_val) {
			for (i = 0; i <= 7; i++)
				bcm_bprintf(b, "  %d(%d%%)", ampdu_rx->amdbg->rxmcssgi[i],
				            (ampdu_rx->amdbg->rxmcssgi[i] * 100) / max_val);
			for (i = 8; i <= AMPDU_HT_MCS_LAST_EL; i++) {
				if ((i % 8) == 0)
					bcm_bprintf(b, "\n          :");
				bcm_bprintf(b, "  %d(%d%%)", ampdu_rx->amdbg->rxmcssgi[i],
				            (ampdu_rx->amdbg->rxmcssgi[i] * 100) / max_val);
			}
		}
#ifdef WL11AC
		bcm_bprintf(b, "\nRX VHT SGI:");
		for (i = 0, max_val = 0; i < AMPDU_MAX_VHT; i++) {
			max_val += ampdu_rx->amdbg->rxvhtsgi[i];
			if (ampdu_rx->amdbg->rxvhtsgi[i]) last = i;
		}
		last = MAX_VHT_RATES * (last/MAX_VHT_RATES + 1) - 1;
		if (max_val) {
			for (i = 0; i <= last; i++) {
					bcm_bprintf(b, "  %d(%d%%)", ampdu_rx->amdbg->rxvhtsgi[i],
						(ampdu_rx->amdbg->rxvhtsgi[i] * 100) / max_val);
					if ((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1) && i != last)
						bcm_bprintf(b, "\n          :");
			}
		}
#endif /* WL11AC */
		bcm_bprintf(b, "\n");

		if (WLCISLCNPHY(ampdu_rx->wlc->band) || (NREV_GT(ampdu_rx->wlc->band->phyrev, 3) &&
			NREV_LE(ampdu_rx->wlc->band->phyrev, 6)))
		{
			bcm_bprintf(b, "RX MCS STBC:");
			for (i = 0, max_val = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
				max_val += ampdu_rx->amdbg->rxmcsstbc[i];

			if (max_val) {
				for (i = 0; i <= 7; i++)
					bcm_bprintf(b, "  %d(%d%%)", ampdu_rx->amdbg->rxmcsstbc[i],
						(ampdu_rx->amdbg->rxmcsstbc[i] * 100) / max_val);
			}
			bcm_bprintf(b, "\n");
#ifdef WL11AC
			for (i = 0, max_val = 0; i < AMPDU_MAX_VHT; i++)
				max_val += ampdu_rx->amdbg->rxvhtstbc[i];

			bcm_bprintf(b, "RX VHT STBC:");
			if (max_val) {
				for (i = 0; i < MAX_VHT_RATES; i++) {
					bcm_bprintf(b, "  %d(%d%%)", ampdu_rx->amdbg->rxvhtstbc[i],
					(ampdu_rx->amdbg->rxvhtstbc[i] * 100) / max_val);
				}
			}
			bcm_bprintf(b, "\n");
#endif /* WL11AC */
		}
	}

	bcm_bprintf(b, "\n");
	FOREACHSCB(ampdu_rx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
			bcm_bprintf(b, "%s: \n", bcm_ether_ntoa(&scb->ea, eabuf));
#if defined(BCMDBG) || defined(BCMDBG_DUMP)

	bcm_bprintf(b, "\trxampdu %u rxmpdu %u rxlegacy %u rxbar %u rxdelba %u\n"
			            "\trxholes %u rxstuck %u rxoow %u rxdup %u\n",
			            scb_ampdu->cnt.rxampdu, scb_ampdu->cnt.rxmpdu,
			            scb_ampdu->cnt.rxlegacy, scb_ampdu->cnt.rxbar,
			            scb_ampdu->cnt.rxdelba, scb_ampdu->cnt.rxholes,
			            scb_ampdu->cnt.rxstuck, scb_ampdu->cnt.rxoow,
			            scb_ampdu->cnt.rxdup);
#endif /* BCMDBG  || BCMDBG_DUMP */
		}
	}

	bcm_bprintf(b, "\n");

	return 0;
} /* wlc_ampdu_rx_dump */
#endif /* BCMDBG || WLTEST */

/**
 * Called when remote party sent us a 'Add Block Ack'. Formulates and sends a response back.
 * Does not have any dependency on ampdu, so can be used for delayed ba as well.
 */
int
wlc_send_addba_resp(wlc_info_t *wlc, struct scb *scb, uint16 status,
	uint8 token, uint16 timeout, uint16 param_set)
{
	dot11_addba_resp_t *addba_resp;
	void *p;
	uint8 *pbody;
	uint16 tid;

	ASSERT(wlc);
	ASSERT(scb);
	ASSERT(scb->bsscfg);

	if (wlc->block_datafifo)
		return BCME_NOTREADY;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, &scb->ea, &scb->bsscfg->cur_etheraddr,
		&scb->bsscfg->BSSID, DOT11_ADDBA_RESP_LEN, &pbody);
	if (p == NULL)
		return BCME_NOMEM;

	addba_resp = (dot11_addba_resp_t *)pbody;
	addba_resp->category = DOT11_ACTION_CAT_BLOCKACK;
	addba_resp->action = DOT11_BA_ACTION_ADDBA_RESP;
	addba_resp->token = token;
	htol16_ua_store(status, (uint8 *)&addba_resp->status);
	htol16_ua_store(param_set, (uint8 *)&addba_resp->addba_param_set);
	htol16_ua_store(timeout, (uint8 *)&addba_resp->timeout);

#ifdef WL_EVENT_LOG_COMPILE
	{
		wl_event_log_tlv_hdr_t tlv_log = {{0, 0}};
		wl_event_log_blk_ack_t sts = {{0, 0}};

		tlv_log.tag = TRACE_TAG_VENDOR_SPECIFIC;
		tlv_log.length = sizeof(wl_event_log_blk_ack_t);

		sts.status = status;
		sts.paraset = param_set;

		WL_EVENT_LOG(EVENT_LOG_TAG_TRACE_WL_INFO, TRACE_BLOCK_ACK_NEGOTIATION_COMPLETE,
			tlv_log.t, sts.t);
	}
#endif /* WL_EVENT_LOG_COMPILE */
	WL_AMPDU_CTL(("wl%d: wlc_send_addba_resp: status %d param_set 0x%x\n",
		wlc->pub->unit, status, param_set));

	/* set same priority as tid */
	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	PKTSETPRIO(p, tid);

	wlc_sendmgmt(wlc, p, SCB_WLCIFP(scb)->qi, scb);

	return 0;
}

#if defined(BCMDBG) || defined(WLTEST) || defined(WLPKTDLYSTAT) || \
	defined(BCMDBG_AMPDU)
#ifdef WLCNT
void
wlc_ampdu_clear_rx_dump(ampdu_rx_info_t *ampdu_rx)
{
#ifdef BCMDBG
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_rx_t *scb_ampdu_rx;
#endif /* BCMDBG */

	/* zero the counters */
	bzero(ampdu_rx->cnt, sizeof(wlc_ampdu_rx_cnt_t));

	/* reset the histogram as well */
	if (ampdu_rx->amdbg) {
		bzero(ampdu_rx->amdbg->rxmcs, sizeof(ampdu_rx->amdbg->rxmcs));
		bzero(ampdu_rx->amdbg->rxmcssgi, sizeof(ampdu_rx->amdbg->rxmcssgi));
		bzero(ampdu_rx->amdbg->rxmcsstbc, sizeof(ampdu_rx->amdbg->rxmcsstbc));
#ifdef WL11AC
		bzero(ampdu_rx->amdbg->rxvht, sizeof(ampdu_rx->amdbg->rxvht));
		bzero(ampdu_rx->amdbg->rxvhtsgi, sizeof(ampdu_rx->amdbg->rxvhtsgi));
		bzero(ampdu_rx->amdbg->rxvhtstbc, sizeof(ampdu_rx->amdbg->rxvhtstbc));
#endif // endif
	}

#ifdef WLAMPDU_MAC
		/* zero out shmem counters */
		if (AMPDU_MAC_ENAB(ampdu_rx->wlc->pub)) {
			/* must have clk to write shmem */
			if (ampdu_rx->wlc->clk) {
				wlc_write_shm(ampdu_rx->wlc, MACSTAT_ADDR(MCSTOFF_RXBACK), 0);
			}
		}
#endif /* WLAMPDU_MAC */

#ifdef BCMDBG
	FOREACHSCB(ampdu_rx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			/* reset the per-SCB statistics */
			scb_ampdu_rx = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);
			bzero(&scb_ampdu_rx->cnt, sizeof(scb_ampdu_cnt_rx_t));
		}
	}
#endif /* BCMDBG */
}
#endif /* WLCNT */
#endif /* defined(BCMDBG) || defined(WLTEST) */

bool
wlc_ampdu_rxba_enable(ampdu_rx_info_t *ampdu_rx, uint8 tid)
{
	return (ampdu_rx->config->rxba_enable[tid]);
}

/** gets size of gaps between MPDUs in an AMPDU */
uint8
wlc_ampdu_rx_get_mpdu_density(ampdu_rx_info_t *ampdu_rx)
{
	return (ampdu_rx->config->mpdu_density);
}

/** sets size of gaps between MPDUs in an AMPDU */
void
wlc_ampdu_rx_set_mpdu_density(ampdu_rx_info_t *ampdu_rx, uint8 mpdu_density)
{
	ampdu_rx->config->mpdu_density = mpdu_density;
}

/** set current max number of MPDUs in a window */
void
wlc_ampdu_rx_set_ba_rx_wsize(ampdu_rx_info_t *ampdu_rx, uint8 wsize)
{
	ampdu_rx->config->ba_rx_wsize = wsize;
}

/** get max allowable number of MPDUs in a window */
uint8
wlc_ampdu_rx_get_ba_max_rx_wsize(ampdu_rx_info_t *ampdu_rx)
{
	return (ampdu_rx->config->ba_max_rx_wsize);
}

#ifdef WLAMPDU_HOSTREORDER

/** host reorder feature specific */
static int
wlc_ampdu_alloc_flow_id(ampdu_rx_info_t *ampdu_rx)
{
	if (!AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub)) {
		WL_ERROR(("%s: ERROR: AMPDU Host reordering not enabled, so shouldn't be here\n",
			__FUNCTION__));
		ASSERT(0);
		return -1;
	}
	/* PCIE full dongle doesn't deal with flow IDs */
	if (BCMPCIEDEV_ENAB())
		return 0;
	ampdu_rx->flow_id++;
	return (ampdu_rx->flow_id);
}

/** host reorder feature specific */
static int
wlc_ampdu_free_flow_id(ampdu_rx_info_t *ampdu_rx, scb_ampdu_tid_resp_t *resp, struct scb *scb)
{
	void *p;

	if (!AMPDU_HOST_REORDER_ENAB(ampdu_rx->wlc->pub)) {
		WL_ERROR(("%s: ERROR: AMPDU Host reordering not enabled, so shouldn't be here\n",
			__FUNCTION__));
		ASSERT(0);
		return -1;
	}
	/* PCIE full dongle doesn't deal with flow IDs */
	if (BCMPCIEDEV_ENAB())
		return 0;

	p = resp->tohost_ctrlpkt;
	resp->tohost_ctrlpkt = NULL;
	if (p == NULL) {
		p = PKTGET(ampdu_rx->wlc->osh, TXOFF, FALSE);
		ASSERT(p != NULL);
	}

	if (p == NULL) {
		/* serious case ...what to do */
		WL_ERROR(("error couldn't alloc packet to cleanup the ampdu host reorder flow\n"));
		return -1;
	}
	PKTPULL(ampdu_rx->wlc->osh, p, TXOFF);
	PKTSETLEN(ampdu_rx->wlc->osh, p, 0);

	wlc_ampdu_setpkt_hostreorder_info(ampdu_rx->wlc, resp, p, AMPDU_INVALID_INDEX,
		NO_NEWHOLE, DEL_FLOW, FLUSH_ALL);
	WLPKTTAGBSSCFGSET(p, scb->bsscfg->_idx);
	wl_sendup(ampdu_rx->wlc->wl, SCB_INTERFACE(scb), p, 1);
	return 0;
}

static void
wlc_ampdu_setpkt_hostreorder_info(wlc_info_t *wlc, scb_ampdu_tid_resp_t *resp, void *p,
	uint16 cur_idx, bool new_hole, bool del_flow, bool flush_all)
{
	wlc_pkttag_t *pkttag =  WLPKTTAG(p);

#ifdef  WLAMPDU_HOSTREORDER
	if (!AMPDU_HOST_REORDER_ENAB(wlc->pub))
#endif // endif
	{
		WL_ERROR(("%s: ERROR: AMPDU Host reordering not enabled, so shouldn't be here\n",
			__FUNCTION__));
		ASSERT(0);
		return;
	}
	/*
	 * different cases of AMPDU host reordering here..
	 * 1. In some cases(SDIO/USB full dongle ) all the metadata need to be sent along
	 *    with the current packet *    so that the host coudl take care of the whole
	 *    reordering business
	 * 2. In other cases(PCIE full dongle) need to link the rxcompletions
	 *    so that the bus layer could take care of reordering
	*/
	/* pcie full dongle doesn't need metadata, as reorder is done on the dongle itself
	 * but in a different mode, resp->rxq is used for rxcplid storage
	 *
	 */
	if (BCMPCIEDEV_ENAB())
		return;

	pkttag->flags2 |= WLF2_HOSTREORDERAMPDU_INFO;
	pkttag->u.ampdu_info_to_host.ampdu_flow_id = resp->flow_id;
	if (del_flow) {
		pkttag->u.ampdu_info_to_host.flags = WLHOST_REORDERDATA_DEL_FLOW;
		return;
	}

	if (cur_idx != AMPDU_INVALID_INDEX) {
		pkttag->u.ampdu_info_to_host.flags = WLHOST_REORDERDATA_CURIDX_VALID;
		if (flush_all) {
			printf("setting the flush all flag");
			pkttag->u.ampdu_info_to_host.flags |= WLHOST_REORDERDATA_FLUSH_ALL;
		}

		pkttag->u.ampdu_info_to_host.cur_idx = cur_idx;
	}
	pkttag->u.ampdu_info_to_host.flags |= WLHOST_REORDERDATA_EXPIDX_VALID;
	pkttag->u.ampdu_info_to_host.exp_idx =  RX_SEQ_TO_INDEX(wlc->ampdu_rx, resp->exp_seq);

	if (new_hole) {
		pkttag->u.ampdu_info_to_host.flags |= WLHOST_REORDERDATA_NEW_HOLE;
		WL_INFORM(("AMPDU_HOSTREORDER message to host...curidx %d expidx %d, "
			"flags 0x%02x\n",
			pkttag->u.ampdu_info_to_host.cur_idx,
			pkttag->u.ampdu_info_to_host.exp_idx,
			pkttag->u.ampdu_info_to_host.flags));
	}
}

#ifdef WL_FRWD_REORDER

/** host reorder feature specific */
static scb_ampdu_tid_resp_t *
wlc_ampdu_get_resp_ptr(ampdu_rx_info_t *ampdu_rx, struct scb *scb, void *p)
{
	scb_ampdu_rx_t *scb_ampdu;
	scb_ampdu_tid_resp_t *resp;

	scb_ampdu = SCB_AMPDU_RX_CUBBY(ampdu_rx, scb);

	if (scb_ampdu == NULL)
		return NULL;

	resp = scb_ampdu->resp[PKTPRIO(p)];

	return resp;
}

/** host reorder feature specific */
static void
wlc_ampdu_release_frwd_pkts(ampdu_rx_info_t *ampdu_rx, void *p)
{
	wlc_info_t *wlc = ampdu_rx->wlc;

	while (p != NULL) {
		void *next_p = PKTLINK(p);
		int8 bsscfgidx = WLPKTTAG(p)->_bsscfgidx;
		wlc_bsscfg_t *bsscfg = wlc->bsscfg[bsscfgidx];

		PKTSETLINK(p, NULL);
		WLPKTTAGCLEAR(p);
		/* Before forwarding, fix the priority */
		if (QOS_ENAB(wlc->pub) && (PKTPRIO(p) == 0))
			pktsetprio(p, FALSE);
		wlc_sendpkt(wlc, p, bsscfg->wlcif);

		p = next_p;
	}
}

/** host reorder feature specific */
static void
wlc_ampdu_get_frwd_reorder_pkts(uint8 start, uint8 end, void **pkt_list,
	ampdu_reorder_info_t *ptr, void **plast)
{
	void *p;
	int i;
	uint8 cnt = 0;

	*plast = NULL;

	/* Return if there is no pending packets */
	if (ptr->pend_pkts == 0) {
		*pkt_list = NULL;
		return;
	}

	/* Find the number of slots */
	if (start == end) {
		/* Flush all the packets if start and end is equal */
		i = AMPDU_BA_MAX_WSIZE;
	} else {
		if (start > end)
			i = (AMPDU_BA_MAX_WSIZE - start) + end;
		else
			i = end - start;
	}

	/* Create a list of packets to be released */
	while (i) {
		p = AMPDU_RXQ_GETPKT(ptr->resp, start);
		if (p != NULL) {
			cnt++;
			AMPDU_RXQ_CLRPKT(ptr->resp, start);
			if (*plast == NULL) {
				*pkt_list = p;
			} else {
				PKTSETLINK(*plast, p);
			}
			*plast = p;
		}
		i--;
		start++;
		if (start == AMPDU_BA_MAX_WSIZE)
			start = 0;
	}
	ptr->pend_pkts -= cnt;

}

void *
wlc_ampdu_frwd_handle_host_reorder(ampdu_rx_info_t *ampdu_rx, void *pkt, bool forward)
{
	wlc_pkttag_t *pkttag = WLPKTTAG(pkt);
	uint8 flow_id, cur_idx, exp_idx;
	uint8 flags;
	ampdu_reorder_info_t *ptr;
	void *pkt_list = NULL, *plast = NULL;
	wlc_info_t *wlc = ampdu_rx->wlc;

	/* Return if the packet didnt have to do anything with host reorder
	  *or if it is already processed
	  */
	if (!AMPDU_HOST_REORDER_ENAB(wlc->pub) ||
		(WLPKTTAG(pkt)->flags3 & WLF3_REORDERDATA_PROCESSED) ||
		!(WLPKTTAG(pkt)->flags2 & WLF2_HOSTREORDERAMPDU_INFO))
		return pkt;

	WLPKTTAG(pkt)->flags3 |= WLF3_REORDERDATA_PROCESSED;

	/* Need to implement - Returning if forwarding is not enabled */

	flow_id = pkttag->u.ampdu_info_to_host.ampdu_flow_id;
	flags = pkttag->u.ampdu_info_to_host.flags;
	ptr = ampdu_rx->dngl_reorder_bufs[flow_id];

	if (flags & WLHOST_REORDERDATA_DEL_FLOW) {
		int i = 0;
		void *p;

		/* Return if there is no flow created */
		if (ptr == NULL) {
			PKTFREE(wlc->osh, pkt, TRUE);
			return NULL;
		}

		/* Free all the queued packets */
		while (i < AMPDU_BA_MAX_WSIZE) {
			p = AMPDU_RXQ_GETPKT(ptr->resp, i);
			if (p != NULL) {
				PKTFREE(wlc->osh, p, TRUE);
				AMPDU_RXQ_CLRPKT(ptr->resp, i);
			}
			i++;
		}
		/* Sent this packet to host only if flow info is known to host */
		p = pkt;

		/* Clear the flow information */
		MFREE(wlc->osh, ptr, sizeof(ampdu_reorder_info_t));
		ampdu_rx->dngl_reorder_bufs[flow_id] = NULL;
		return p;
	}

	/* Create a flow if it is not already existing */
	if (ptr == NULL) {
		/* Create the flow and reorder only if scb is available */
		if (pkttag->_scb != NULL) {
			ptr = (ampdu_reorder_info_t *)MALLOC(wlc->osh,
				sizeof(ampdu_reorder_info_t));
			bzero(ptr, sizeof(ampdu_reorder_info_t));
			ampdu_rx->dngl_reorder_bufs[flow_id] = ptr;

			ptr->resp = wlc_ampdu_get_resp_ptr(ampdu_rx, pkttag->_scb, pkt);
			if (ptr->resp == NULL) {
				MFREE(wlc->osh, ptr, sizeof(ampdu_reorder_info_t));
				return pkt;
			}
		} else {
			return pkt;
		}
	}

	/* New Hole information */
	if (flags & WLHOST_REORDERDATA_NEW_HOLE) {
		/* Flush all the previous packets */
		wlc_ampdu_get_frwd_reorder_pkts(ptr->exp_idx, ptr->exp_idx, &pkt_list, ptr, &plast);

		ptr->cur_idx = pkttag->u.ampdu_info_to_host.cur_idx;
		ptr->exp_idx = pkttag->u.ampdu_info_to_host.exp_idx;

		if (forward) {
			/* If host has pending packets, sent a packet to host
			  * for flushing all the old packets
			  */
			if (ptr->new_hole_informed) {
				void *p_ahr;
				if ((p_ahr = PKTGET(wlc->osh, TXOFF, TRUE)) != NULL) {
					int8 bsscfgidx = WLPKTTAG(pkt)->_bsscfgidx;
					wlc_bsscfg_t *bsscfg = wlc->bsscfg[bsscfgidx];
					PKTPULL(wlc->osh, p_ahr, TXOFF);
					PKTSETLEN(wlc->osh, p_ahr, 0);
					wlc_pkttag_info_move(wlc, pkt, p_ahr);
					WLPKTTAG(p_ahr)->u.ampdu_info_to_host.flags &=
						~(WLHOST_REORDERDATA_NEW_HOLE |
						WLHOST_REORDERDATA_CURIDX_VALID);
					WLPKTTAG(p_ahr)->u.ampdu_info_to_host.flags |=
						WLHOST_REORDERDATA_FLUSH_ALL;
					WLPKTTAG(p_ahr)->flags2 |= WLF2_HOSTREORDERAMPDU_INFO;
					wl_sendup(wlc->wl, bsscfg->wlcif->wlif, p_ahr, 1);
				}
				/* Clear host packet pending flag as all the packets
				  * should have flushed by the above packet
				  */
				ptr->new_hole_informed = FALSE;
			}
			/* Save the current packet pointer */
			AMPDU_RXQ_SETPKT(ptr->resp, ptr->cur_idx, pkt);
			ptr->pend_pkts++;
		} else {
			/* Sent out all the older forward packets corresponding to this flow */
			wlc_ampdu_release_frwd_pkts(ampdu_rx, pkt_list);
			ptr->new_hole_informed = TRUE;
			pkt_list = pkt;
			ptr->last_idx = pkttag->u.ampdu_info_to_host.cur_idx;
		}
	} else if (flags & WLHOST_REORDERDATA_CURIDX_VALID) {
		void *p = NULL;
		/* This case have a valid packet to sent */
		cur_idx = pkttag->u.ampdu_info_to_host.cur_idx;
		exp_idx = pkttag->u.ampdu_info_to_host.exp_idx;

		if ((exp_idx == ptr->exp_idx) && (cur_idx != ptr->exp_idx)) {
			/* If still in the current hole */
			if (forward) {
				/* No need to sent information to host if we are still in the
				  * current hole and the pkt is forward packet
				  */
				p = AMPDU_RXQ_GETPKT(ptr->resp, cur_idx);
				if (p != NULL) {
					PKTFREE(wlc->osh, p, TRUE);
					ptr->pend_pkts--;
					AMPDU_RXQ_CLRPKT(ptr->resp, cur_idx);
				}
				AMPDU_RXQ_SETPKT(ptr->resp, cur_idx, pkt);
				ptr->pend_pkts++;
			} else {
				if (!ptr->new_hole_informed) {
					/* Set NEW_HOLE flag if New hole information
					  * is not sent to host before
					  */
					pkttag->u.ampdu_info_to_host.flags |=
						WLHOST_REORDERDATA_NEW_HOLE;
					ptr->new_hole_informed = TRUE;
					ptr->last_idx = cur_idx;
				} else if (MODSUB(cur_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE) >
					MODSUB(ptr->last_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE)) {
					/* Update the host packet index to the latest one
					  * if current index is greater than last stored index
					  */
					ptr->last_idx = cur_idx;
				}
				pkt_list = pkt;
			}
			ptr->cur_idx = cur_idx;

		}
		else if (ptr->exp_idx == cur_idx) {
			/* Got the expected packet */
			if (forward) {
				p = AMPDU_RXQ_GETPKT(ptr->resp, cur_idx);
				if (p != NULL) {
					PKTFREE(wlc->osh, p, TRUE);
					ptr->pend_pkts--;
					AMPDU_RXQ_CLRPKT(ptr->resp, cur_idx);
				}
				/* Save the current packet pointer */
				AMPDU_RXQ_SETPKT(ptr->resp, cur_idx, pkt);
				ptr->pend_pkts++;
				if (ptr->new_hole_informed) {
					void *p_ahr;
					/* Create a packet and sent it to host
					  * if already some packets are held in host.
					  * This is to move the exp seq in host and
					  * to release in-order packets
					  */
					if ((p_ahr = PKTGET(wlc->osh, TXOFF, TRUE)) != NULL) {
						int8 bsscfgidx = WLPKTTAG(pkt)->_bsscfgidx;
						wlc_bsscfg_t *bsscfg = wlc->bsscfg[bsscfgidx];
						PKTPULL(wlc->osh, p_ahr, TXOFF);
						PKTSETLEN(wlc->osh, p_ahr, 0);
						wlc_pkttag_info_move(wlc, pkt, p_ahr);
						WLPKTTAG(p_ahr)->flags2 |=
							WLF2_HOSTREORDERAMPDU_INFO;
						wl_sendup(wlc->wl, bsscfg->wlcif->wlif, p_ahr, 1);
					}

					if (MODSUB(ptr->last_idx, ptr->exp_idx,
						AMPDU_BA_MAX_WSIZE) <
						MODSUB(exp_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE)) {
						/* Clear if all the packet in host is flushed */
						ptr->new_hole_informed = FALSE;
					}
				}
				wlc_ampdu_get_frwd_reorder_pkts(cur_idx, exp_idx,
					&pkt_list, ptr, &plast);
			} else {
				if (ptr->new_hole_informed) {
					if (MODSUB(ptr->last_idx, ptr->exp_idx,
						AMPDU_BA_MAX_WSIZE) <
						MODSUB(exp_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE)) {
						/* Clear if all the packet in host is flushed */
						ptr->new_hole_informed = FALSE;
					}
				} else {
					/* Dont sent the host reorder information if it is
					  * in order and host didnt have any pending packets
					  */
					WLPKTTAG(pkt)->flags2 &= ~WLF2_HOSTREORDERAMPDU_INFO;
				}

				/* Release the forward packets upto new expected seq */
				wlc_ampdu_get_frwd_reorder_pkts(cur_idx, exp_idx,
					&pkt_list, ptr, &plast);

				wlc_ampdu_release_frwd_pkts(ampdu_rx, pkt_list);
				pkt_list = pkt;
			}

			ptr->cur_idx = cur_idx;
			ptr->exp_idx = exp_idx;
		}
		else {
			/* Both cur idx and exp idx is moved in this case */
			uint8 end_idx;
			if (flags & WLHOST_REORDERDATA_FLUSH_ALL)
				end_idx = ptr->exp_idx;
			else
				end_idx = exp_idx;

			/* Release the forward packets till exp idx */
			wlc_ampdu_get_frwd_reorder_pkts(ptr->exp_idx, end_idx,
				&pkt_list, ptr, &plast);

			if (forward) {
				if (exp_idx == MODADD(cur_idx, 1, AMPDU_BA_MAX_WSIZE)) {
					if (plast)
						PKTSETLINK(plast, pkt);
					else
						pkt_list = pkt;
				}
				else {
					AMPDU_RXQ_SETPKT(ptr->resp, cur_idx, pkt);
					ptr->pend_pkts++;
				}
				if (ptr->new_hole_informed) {
					void *p_ahr;
					/* Create a packet and sent it to host if already
					  * some packets are held in host. This is to move the
					  * exp seq in host and to release in-order packets
					  */
					if ((p_ahr = PKTGET(wlc->osh, TXOFF, TRUE)) != NULL) {
						int8 bsscfgidx = WLPKTTAG(pkt)->_bsscfgidx;
						wlc_bsscfg_t *bsscfg = wlc->bsscfg[bsscfgidx];
						PKTPULL(wlc->osh, p_ahr, TXOFF);
						PKTSETLEN(wlc->osh, p_ahr, 0);
						wlc_pkttag_info_move(wlc, pkt, p_ahr);
						WLPKTTAG(p_ahr)->flags2 |=
							WLF2_HOSTREORDERAMPDU_INFO;
						wl_sendup(wlc->wl, bsscfg->wlcif->wlif, p_ahr, 1);
					}
					if (flags & WLHOST_REORDERDATA_FLUSH_ALL) {
						/* Clear the host has packet flag if information
						  * passed to flush all the packets
						  */
						ptr->new_hole_informed = FALSE;
					} else if (MODSUB(ptr->last_idx, ptr->exp_idx,
						AMPDU_BA_MAX_WSIZE) <
						MODSUB(exp_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE)) {
						/* Clear if all the packet in host is flushed */
						ptr->new_hole_informed = FALSE;
					}
				}
			} else {
				wlc_ampdu_release_frwd_pkts(ampdu_rx, pkt_list);

				if (!ptr->new_hole_informed) {
					if (exp_idx == MODADD(cur_idx, 1, AMPDU_BA_MAX_WSIZE)) {
						/* Dont sent the host reorder information if it is
						  * in order and host didnt have any pending packets
						  */
						WLPKTTAG(pkt)->flags2 &=
							~WLF2_HOSTREORDERAMPDU_INFO;
					} else {
						/* Set NEW_HOLE flag if New hole information
						  * is not sent to host befores
						  */
						ptr->last_idx = cur_idx;
						ptr->new_hole_informed = TRUE;
						pkttag->u.ampdu_info_to_host.flags |=
							WLHOST_REORDERDATA_NEW_HOLE;
					}
				} else {
					if (exp_idx == MODADD(cur_idx, 1, AMPDU_BA_MAX_WSIZE)) {
						if ((flags & WLHOST_REORDERDATA_FLUSH_ALL) ||
							(MODSUB(ptr->last_idx, ptr->exp_idx,
							AMPDU_BA_MAX_WSIZE) <
							MODSUB(exp_idx, ptr->exp_idx,
							AMPDU_BA_MAX_WSIZE))) {
							/* Clear the host has pkt flag if all the
							  * packets in host is flushed
							  */
							ptr->new_hole_informed = FALSE;
						}
					} else if (flags & WLHOST_REORDERDATA_FLUSH_ALL) {
						ptr->last_idx = cur_idx;
					} else if (MODSUB(ptr->last_idx, ptr->exp_idx,
						AMPDU_BA_MAX_WSIZE) <
						MODSUB(exp_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE)) {
						ptr->last_idx = cur_idx;
					} else if (MODSUB(cur_idx, exp_idx,
						AMPDU_BA_MAX_WSIZE) >
						MODSUB(ptr->last_idx, exp_idx,
						AMPDU_BA_MAX_WSIZE)) {
						ptr->last_idx = cur_idx;
					}
				}
				pkt_list = pkt;
			}
			ptr->exp_idx = exp_idx;
			ptr->cur_idx = cur_idx;
		}
	} else {
		/* No actual packet - Window movement */
		uint8 end_idx;
		exp_idx = pkttag->u.ampdu_info_to_host.exp_idx;

		if (flags & WLHOST_REORDERDATA_FLUSH_ALL)
			end_idx =  ptr->exp_idx;
		else
			end_idx =  exp_idx;

		/* Release the saved forward packets till new exp sequence */
		wlc_ampdu_get_frwd_reorder_pkts(ptr->exp_idx, end_idx, &pkt_list, ptr, &plast);

		wlc_ampdu_release_frwd_pkts(ampdu_rx, pkt_list);

		if (ptr->new_hole_informed) {
			if (flags & WLHOST_REORDERDATA_FLUSH_ALL) {
				/* Clear the host has packet flag if information
				  * passed to flush all the packets
				  */
				ptr->new_hole_informed = FALSE;
			} else if (MODSUB(ptr->last_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE) <
						MODSUB(exp_idx, ptr->exp_idx, AMPDU_BA_MAX_WSIZE)) {
				/* Clear if information passed to flush all the packets */
				ptr->new_hole_informed = FALSE;
			}
			pkt_list = pkt;
		} else {
			PKTFREE(wlc->osh, pkt, TRUE);
			pkt_list = NULL;
		}

		ptr->exp_idx = exp_idx;
	}
	return pkt_list;
} /* wlc_ampdu_frwd_handle_host_reorder */

#endif /* WL_FRWD_REORDER */
#endif /* WLAMPDU_HOSTREORDER */

void
wlc_ampdu_agg_state_update_rx_all(wlc_info_t *wlc, bool aggr)
{
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_BSS(wlc, idx, cfg) {
		wlc_ampdu_rx_set_bsscfg_aggr_override(wlc->ampdu_rx, cfg, aggr);
	}
}
#ifdef WL11K_ALL_MEAS
uint32 wlc_ampdu_getstat_rxampdu(wlc_info_t *wlc)
{
	ASSERT(wlc);
	return (wlc->ampdu_rx->cnt->rxampdu);
}

uint32 wlc_ampdu_getstat_rxmpdu(wlc_info_t *wlc)
{
	ASSERT(wlc);
	return (wlc->ampdu_rx->cnt->rxmpdu);
}

uint32 wlc_ampdu_getstat_rxampdubyte_h(wlc_info_t *wlc)
{
	ASSERT(wlc);
	return (wlc->ampdu_rx->cnt->rxampdubyte_h);
}

uint32 wlc_ampdu_getstat_rxampdubyte_l(wlc_info_t *wlc)
{
	ASSERT(wlc);
	return (wlc->ampdu_rx->cnt->rxampdubyte_l);
}

uint32 wlc_ampdu_getstat_ampducrcfail(wlc_info_t *wlc)
{
	ASSERT(wlc);
	return (wlc->ampdu_rx->cnt->rxunexp);
}
#endif /* WL11K_ALL_MEAS */
