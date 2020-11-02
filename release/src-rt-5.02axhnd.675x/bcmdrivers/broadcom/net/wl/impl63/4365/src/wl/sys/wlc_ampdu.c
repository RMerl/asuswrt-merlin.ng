/*
 * A-MPDU Tx (with extended Block Ack protocol) source file
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
 * $Id: wlc_ampdu.c 776666 2019-07-05 06:34:40Z $
 */

/**
 * Preprocessor defines used in this file:
 *
 * WLAMPDU:       firmware/driver image supports AMPDU functionality
 * WLAMPDU_MAC:   aggregation by d11 core (being ucode, ucode hw assisted or AQM)
 * WLAMPDU_UCODE: aggregation by ucode
 * WLAMPDU_HW:    hardware assisted aggregation by ucode
 * WLAMPDU_AQM:   aggregation by AQM module in d11 core (AC chips only)
 * WLAMPDU_PRECEDENCE: transmit certain traffic earlier than other traffic
 * PSPRETEND:     increase robustness against bad signal conditions by performing resends later
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
#if defined(PKTC)            /* host variant of packet chaining to reduce per-packet \
	overhead */
#include <proto/ethernet.h>
#endif // endif
#if defined(PKTC) || defined(PKTC_TX_DONGLE)
#include <proto/802.3.h>
#endif // endif
#include <wlioctl.h>
#ifdef WLOVERTHRUSTER        /* Opportunistic Tcp Ack Consolidation */
#include <proto/ethernet.h>
#endif // endif
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_scb.h>         /* one SCB represents one remote party */
#include <wlc_frmutil.h>
#include <wlc_p2p.h>
#include <wlc_apps.h>
#ifdef WLTOEHW           /* TCP segmentation / checksum hardware assisted offload. AC \
	chips only. */
#include <wlc_tso.h>
#endif // endif
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#include <wlc_ampdu_cmn.h>
#endif /* WLAMPDU */
#if defined(WLAMSDU) || defined(WLAMSDU_TX)
#include <wlc_amsdu.h>
#endif // endif
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
#ifdef WLP2P
#include <wlc_p2p.h>
#endif // endif
#ifdef WLC_HIGH_ONLY
#include <bcm_rpc_tp.h>
#include <wlc_rpctx.h>
#endif // endif
#include <wlc_pcb.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif

#ifdef WLAWDL
#include <wlc_awdl.h>
#endif // endif

#ifdef WL_MU_TX
#include <wlc_mutx.h>
#endif // endif

#if defined(WLATF) || defined(PKTQ_LOG)                 /* Air Time Fairness */
#include <wlc_airtime.h>
#include <wlc_prot.h>
#endif // endif
#ifdef WLATF
#include <wlc_prot_g.h>
#include <wlc_prot_n.h>
#endif /* WLATF */
#if defined(TXQ_MUX)
#include <wlc_txtime.h>
#endif /* TXQ_MUX */

#include <wlc_btcx.h>

#include <wlc_txc.h>
#include <wlc_objregistry.h>
#ifdef WL11AC
#include <wlc_vht.h>
#endif /* WL11AC */
#include <wlc_ht.h>

#if defined(SCB_BS_DATA)     /* band steering */
#include <wlc_bs_data.h>
#endif /* SCB_BS_DATA */

/* Enable NON AQM code only when builds require ucode/Hw */
/* Dongle 11ac builds will define WLAMPDU_AQM */
#ifndef WLAMPDU_AQM
#define AMPDU_NON_AQM
#endif // endif
#include <wlc_tx.h>
#include <wlc_bmac.h>
#include <wlc_ulb.h>

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMPDU)
#include <wlc_bmac.h>
#endif /*  defined(BCMDBG) ... */
#include <wlc_macdbg.h>

#ifdef WL11K
#include <wlc_rrm.h>
#endif // endif

#ifdef WLNAR
#include <wlc_nar.h>
#endif /* WLNAR */

#ifdef WLAMPDU_PRECEDENCE
#define wlc_ampdu_pktq_plen(pq, tid) \
	(pktq_plen(pq, WLC_PRIO_TO_HI_PREC(tid))\
	+ pktq_plen(pq, WLC_PRIO_TO_PREC(tid)))
static bool wlc_ampdu_prec_enq(wlc_info_t *wlc, struct pktq *q, void *pkt, int tid);
static void * wlc_ampdu_pktq_pdeq(struct pktq *pq, int tid);
static void
wlc_ampdu_pktq_pflush(osl_t *osh, struct pktq *pq, int tid, bool dir, ifpkt_cb_t fn, int arg);
#define wlc_ampdu_pktqlog_cnt(q, tid, prec)	(q)->pktqlog->_prec_cnt[(prec)]
static void *wlc_ampdu_pktq_ppeek(struct pktq *pq, int tid);
#else
#define wlc_ampdu_prec_enq(wlc, q, pkt, prec) 	wlc_prec_enq_head(wlc, q, pkt, prec, FALSE)
#define wlc_ampdu_pktq_plen 					pktq_plen
#define wlc_ampdu_pktq_pdeq 					pktq_pdeq
#define wlc_ampdu_pktq_pflush 					pktq_pflush
#define wlc_ampdu_pktqlog_cnt(q, tid, prec)	(q)->pktqlog->_prec_cnt[(tid)]
#define wlc_ampdu_pktq_ppeek 					pktq_ppeek
#endif /* WLAMPDU_PRECEDENCE */

#define WLAMPDU_PREC_TID2PREC(p, tid) \
	((WLPKTTAG(p)->flags3 & WLF3_FAVORED) ? WLC_PRIO_TO_HI_PREC(tid) : WLC_PRIO_TO_PREC(tid))

#ifdef WLAMPDU_MAC
static void BCMFASTPATH
wlc_ampdu_dotxstatus_aqm_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
                                  void *p, tx_status_t *txs, wlc_txh_info_t *txh_info);
static void BCMFASTPATH
wlc_frmburst_dotxstatus(ampdu_tx_info_t *ampdu_tx, tx_status_t *txs);
#endif /* WLAMPDU_MAC */

#ifdef WLATF
/* Default AMPDU txq time allowance. */
#define AMPDU_TXQ_TIME_ALLOWANCE_US	4000
#define AMPDU_TXQ_TIME_MIN_ALLOWANCE_US	1000

/* Minimum retry threshold after which we will compensate for the retries */
#define AMPDU_RETRY_COMPENSATE_THRESH	10

/* Low water mark packet count threshold for ampdu release per scb per tid */
#define AMPDU_ATF_LOWAT_REL_CNT		24
/* Low water mark threshold over aggregate packet count for ampdu release */
#define AMPDU_ATF_AGG_LOWAT_REL_CNT	256

/* Note: Structure members are referred directly to reduce the overhead as these macros are used
 * in the per-packet part of the datapath
 */

#define AMPDU_ATF_STATE(ini) (&(ini)->atf_state)
#define AMPDU_ATF_ENABLED(ini) ((ini)->atf_state.atf != 0)
#ifdef WLCNT
#define AMPDU_ATF_STATS(ini) (&(ini)->atf_state.atf_stats)
#else
#define AMPDU_ATF_STATS(ini) 0 /* This will force a kernel panic if WLCNT has not been defined */
#endif /* WLCNT */
#endif /* WLATF */

#ifdef BCMDBG
static  int 	_txq_prof_enab;
#endif // endif

#if defined(WLCNT) && defined(WLATF)
#define WLATF_CNT
#endif // endif

#ifndef AMPDU_DYNFB_RTSPER_OFF
#define AMPDU_DYNFB_RTSPER_OFF 50 /* Frameburst disable RTS PER threshold (percent) */
#endif // endif
#ifndef AMPDU_DYNFB_RTSPER_ON
#define AMPDU_DYNFB_RTSPER_ON  20 /* Frameburst enable RTS PER threshold (percent) */
#endif // endif
#ifndef AMPDU_DYNFB_MINRTSCNT
#define AMPDU_DYNFB_MINRTSCNT  80 /* Min number of Tx RTS after which RTS PER is calculated */
#endif // endif

/* iovar table */
enum {
	IOV_AMPDU_TX,		/* enable/disable ampdu tx */
	IOV_AMPDU_TID,		/* enable/disable per-tid ampdu */
	IOV_AMPDU_TX_DENSITY,	/* ampdu density */
	IOV_AMPDU_SEND_ADDBA,	/* send addba req to specified scb and tid */
	IOV_AMPDU_SEND_DELBA,	/* send delba req to specified scb and tid */
	IOV_AMPDU_MANUAL_MODE,	/* addba tx to be driven by cmd line */
	IOV_AMPDU_NO_BAR,	/* do not send bars */
	IOV_AMPDU_MPDU,		/* max number of mpdus in an ampdu */
	IOV_AMPDU_DUR,		/* max duration of an ampdu (in usec) */
	IOV_AMPDU_RTS,		/* use rts for ampdu */
	IOV_AMPDU_TX_BA_WSIZE,	/* ampdu TX ba window size */
	IOV_AMPDU_RETRY_LIMIT,	/* ampdu retry limit */
	IOV_AMPDU_RR_RETRY_LIMIT, /* regular rate ampdu retry limit */
	IOV_AMPDU_TX_LOWAT,	/* ampdu tx low wm */
	IOV_AMPDU_HIAGG_MODE,	/* agg mpdus with diff retry cnt */
	IOV_AMPDU_PROBE_MPDU,	/* number of mpdus/ampdu for rate probe frames */
	IOV_AMPDU_TXPKT_WEIGHT,	/* weight of ampdu in the txfifo; helps rate lag */
	IOV_AMPDU_FFPLD_RSVD,	/* bytes to reserve in pre-loading info */
	IOV_AMPDU_FFPLD,        /* to display pre-loading info */
	IOV_AMPDU_MAX_TXFUNFL,   /* inverse of acceptable proportion of tx fifo underflows */
	IOV_AMPDU_RETRY_LIMIT_TID,	/* ampdu retry limit per-tid */
	IOV_AMPDU_RR_RETRY_LIMIT_TID, /* regular rate ampdu retry limit per-tid */
	IOV_AMPDU_MFBR,		/* Use multiple fallback rate */
#ifdef WLOVERTHRUSTER
	IOV_AMPDU_TCP_ACK_RATIO, 	/* max number of ack to suppress in a row. 0 disable */
	IOV_AMPDU_TCP_ACK_RATIO_DEPTH, 	/* max number of multi-ack. 0 disable */
#endif /* WLOVERTHRUSTER */
	IOV_AMPDUMAC,		/* Use ucode or hw assisted agregation */
	IOV_AMPDU_AGGMODE,	/* agregation mode: HOST or MAC */
	IOV_AMPDU_AGGFIFO,      /* aggfifo depth */
#ifdef WLMEDIA_TXFAILEVENT
	IOV_AMPDU_TXFAIL_EVENT, /* Tx failure event to the host */
#endif /* WLMEDIA_TXFAILEVENT */
	IOV_FRAMEBURST_OVR,	/* Override framebursting in absense of ampdu */
	IOV_AMPDU_TXQ_PROFILING_START,	/* start sampling TXQ profiling data */
	IOV_AMPDU_TXQ_PROFILING_DUMP,	/* dump TXQ histogram */
	IOV_AMPDU_TXQ_PROFILING_SNAPSHOT,	/* take a snapshot of TXQ histogram */
	IOV_AMPDU_RELEASE,	 /* max # of mpdus released at a time */
	IOV_AMPDU_ATF_US,
	IOV_AMPDU_ATF_MIN_US,
	IOV_AMPDU_CS_PKTRETRY,
	IOV_AMPDU_TXAGGR,
	IOV_DYNFB_RTSPER_ON,
	IOV_DYNFB_RTSPER_OFF,
	IOV_AMPDU_ATF_LOWAT,
	IOV_AMPDU_LAST
};

static const bcm_iovar_t ampdu_iovars[] = {
	{"ampdu_tx", IOV_AMPDU_TX, (IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_BOOL, 0},
	{"ampdu_tid", IOV_AMPDU_TID, (0), IOVT_BUFFER, sizeof(struct ampdu_tid_control)},
	{"ampdu_tx_density", IOV_AMPDU_TX_DENSITY, (0), IOVT_UINT8, 0},
	{"ampdu_send_addba", IOV_AMPDU_SEND_ADDBA, (0), IOVT_BUFFER, sizeof(struct ampdu_ea_tid)},
	{"ampdu_send_delba", IOV_AMPDU_SEND_DELBA, (0), IOVT_BUFFER, sizeof(struct ampdu_ea_tid)},
	{"ampdu_manual_mode", IOV_AMPDU_MANUAL_MODE, (IOVF_SET_DOWN), IOVT_BOOL, 0},
	{"ampdu_mpdu", IOV_AMPDU_MPDU, (0), IOVT_INT8, 0}, /* need to mark IOVF2_RSDB */
#ifdef WLOVERTHRUSTER
	{"ack_ratio", IOV_AMPDU_TCP_ACK_RATIO, (0), IOVT_UINT8, 0},
	{"ack_ratio_depth", IOV_AMPDU_TCP_ACK_RATIO_DEPTH, (0), IOVT_UINT8, 0},
#endif /* WLOVERTHRUSTER */
#ifdef BCMDBG
	{"ampdu_aggfifo", IOV_AMPDU_AGGFIFO, 0, IOVT_UINT8, 0},
	{"ampdu_ffpld", IOV_AMPDU_FFPLD, (0), IOVT_UINT32, 0},
#endif // endif

	{"ampdumac", IOV_AMPDUMAC, (0), IOVT_UINT8, 0},
#if defined(WL_EXPORT_AMPDU_RETRY)
	{"ampdu_retry_limit_tid", IOV_AMPDU_RETRY_LIMIT_TID, (0), IOVT_BUFFER,
	sizeof(struct ampdu_retry_tid)},
	{"ampdu_rr_retry_limit_tid", IOV_AMPDU_RR_RETRY_LIMIT_TID, (0), IOVT_BUFFER,
	sizeof(struct ampdu_retry_tid)},
#endif // endif
#ifdef WLMEDIA_TXFAILEVENT
	{"ampdu_txfail_event", IOV_AMPDU_TXFAIL_EVENT, (0), IOVT_BOOL, 0},
#endif /* WLMEDIA_TXFAILEVENT */
	{"ampdu_aggmode", IOV_AMPDU_AGGMODE, (IOVF_SET_DOWN|IOVF_RSDB_SET), IOVT_INT8, 0},
	{"frameburst_override", IOV_FRAMEBURST_OVR, (0), IOVT_BOOL, 0},
	{"ampdu_txq_prof_start", IOV_AMPDU_TXQ_PROFILING_START, (0), IOVT_VOID, 0},
	{"ampdu_txq_prof_dump", IOV_AMPDU_TXQ_PROFILING_DUMP, (0), IOVT_VOID, 0},
	{"ampdu_txq_ss", IOV_AMPDU_TXQ_PROFILING_SNAPSHOT, (0), IOVT_VOID, 0},
	{"ampdu_release", IOV_AMPDU_RELEASE, (0), IOVT_UINT8, 0},
#ifdef WLATF
	{"ampdu_atf_us", IOV_AMPDU_ATF_US, (IOVF_NTRL), IOVT_UINT32, 0},
	{"ampdu_atf_min_us", IOV_AMPDU_ATF_MIN_US, (IOVF_NTRL), IOVT_UINT32, 0},
#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
	{"ampdu_atf_lowat", IOV_AMPDU_ATF_LOWAT, (IOVF_NTRL), IOVT_UINT32, 0},
#endif /* defined(BCMDBG) || defined(BCMDBG_AMPDU) */
#endif /* WLATF */
	{"ampdu_txaggr", IOV_AMPDU_TXAGGR, IOVF_BSS_SET_DOWN, IOVT_BUFFER,
	sizeof(struct ampdu_aggr)},
#if defined(WLAMPDU_MAC) && (defined(BCMDBG) || defined(TUNE_FBOVERRIDE))
	{"dyn_fb_rtsper_on", IOV_DYNFB_RTSPER_ON, (0), IOVT_UINT8, 0},
	{"dyn_fb_rtsper_off", IOV_DYNFB_RTSPER_OFF, (0), IOVT_UINT8, 0},
#endif // endif
	{NULL, 0, 0, 0, 0},
};

#ifdef WLPKTDLYSTAT
static const char ac_names[4][6] = {"AC_BE", "AC_BK", "AC_VI", "AC_VO"};
#endif // endif

#define AMPDU_DEF_PROBE_MPDU	2		/* def number of mpdus in a rate probe ampdu */

#ifndef AMPDU_TX_BA_MAX_WSIZE
#define AMPDU_TX_BA_MAX_WSIZE	64		/* max Tx ba window size (in pdu) */
#endif /* AMPDU_TX_BA_MAX_WSIZE */
#ifndef AMPDU_TX_BA_DEF_WSIZE
#define AMPDU_TX_BA_DEF_WSIZE	64		/* default Tx ba window size (in pdu) */
#endif /* AMPDU_TX_BA_DEF_WSIZE */

#define	AMPDU_BA_BITS_IN_BITMAP	64		/* number of bits in bitmap */
#define	AMPDU_MAX_DUR		5416		/* max dur of tx ampdu (in usec) */
#define AMPDU_MAX_RETRY_LIMIT	32		/* max tx retry limit */
#define AMPDU_DEF_RETRY_LIMIT	5		/* default tx retry limit */
#define AMPDU_DEF_RR_RETRY_LIMIT	2	/* default tx retry limit at reg rate */
#define AMPDU_DEF_TX_LOWAT	1		/* default low transmit wm */
#define AMPDU_DEF_TXPKT_WEIGHT	2		/* default weight of ampdu in txfifo */
#define AMPDU_DEF_FFPLD_RSVD	2048		/* default ffpld reserved bytes */
#define AMPDU_MIN_FFPLD_RSVD	512		/* minimum ffpld reserved bytes */
#define AMPDU_BAR_RETRY_CNT	50		/* # of bar retries before delba */
#define AMPDU_ADDBA_REQ_RETRY_CNT	4	/* # of addbareq retries before delba */
#define AMPDU_INI_OFF_TIMEOUT	60		/* # of sec in off state */
#define	AMPDU_SCB_MAX_RELEASE	32		/* max # of mpdus released at a time */
#ifndef AMPDU_SCB_MAX_RELEASE_AQM
#define	AMPDU_SCB_MAX_RELEASE_AQM	64	/* max # of mpdus released at a time */
#endif /* AMPDU_SCB_MAX_RELEASE_AQM */

#define AMPDU_INI_DEAD_TIMEOUT	2		/* # of sec without ini progress */
#define AMPDU_INI_CLEANUP_TIMEOUT	2	/* # of sec in pending off state */
#define AMPDU_TXNOPROG_LIMIT	10		/* max # of sec with any ini in pending off state
						 * while ucode is consuming no pkts before
						 * we dump more debugging messages
						 */

/* internal BA states */
#define	AMPDU_TID_STATE_BA_OFF		0x00	/* block ack OFF for tid */
#define	AMPDU_TID_STATE_BA_ON		0x01	/* block ack ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_ON	0x02	/* block ack pending ON for tid */
#define	AMPDU_TID_STATE_BA_PENDING_OFF	0x03	/* block ack pending OFF for tid */

#define NUM_FFPLD_FIFO 4                        /* number of fifo concerned by pre-loading */
#define FFPLD_TX_MAX_UNFL   200                 /* default value of the average number of ampdu
						 * without underflows
						 */
#define FFPLD_MPDU_SIZE 1800                    /* estimate of maximum mpdu size */
#define FFPLD_PLD_INCR 1000                     /* increments in bytes */
#define FFPLD_MAX_AMPDU_CNT 5000                /* maximum number of ampdu we
						 * accumulate between resets.
						 */
/* retry BAR in watchdog reason codes */
#define AMPDU_R_BAR_DISABLED		0	/* disabled retry BAR in watchdog */
#define AMPDU_R_BAR_NO_BUFFER		1	/* retry BAR due to out of buffer */
#define AMPDU_R_BAR_CALLBACK_FAIL	2	/* retry BAR due to callback register fail */
#define AMPDU_R_BAR_HALF_RETRY_CNT	3	/* retry BAR due to reach to half
						 * AMPDU_BAR_RETRY_CNT
						 */
#define AMPDU_R_BAR_BLOCKED		4	/* retry BAR due to blocked data fifo */

#define AMPDU_DEF_TCP_ACK_RATIO		2	/* default TCP ACK RATIO */
#define AMPDU_DEF_TCP_ACK_RATIO_DEPTH	0	/* default TCP ACK RATIO DEPTH */

#ifndef AMPDU_SCB_DRAIN_CNT
#define AMPDU_SCB_DRAIN_CNT	8
#endif // endif

/* maximum number of frames can be released to HW (in-transit limit) */
#define AMPDU_AQM_RELEASE_MAX          256	/* for BE */
#define AMPDU_AQM_RELEASE_DEFAULT      256

#define IS_SEQ_ADVANCED(a, b) \
	(MODSUB_POW2((a), (b), SEQNUM_MAX) < (SEQNUM_MAX >> 1))

int wl_ampdu_drain_cnt = AMPDU_SCB_DRAIN_CNT;

/* useful macros */
#define NEXT_SEQ(seq) MODINC_POW2((seq), SEQNUM_MAX)
#define NEXT_TX_INDEX(index) MODINC_POW2((index), (ampdu_tx->config->ba_max_tx_wsize))
#define TX_SEQ_TO_INDEX(seq) ((seq) & ((ampdu_tx->config->ba_max_tx_wsize) - 1))

/* max possible overhead per mpdu in the ampdu; 3 is for roundup if needed */
#define AMPDU_MAX_MPDU_OVERHEAD (DOT11_FCS_LEN + DOT11_ICV_AES_LEN + AMPDU_DELIMITER_LEN + 3 \
	+ DOT11_A4_HDR_LEN + DOT11_QOS_LEN + DOT11_IV_MAX_LEN)

/** ampdu related transmit stats */
typedef struct wlc_ampdu_cnt {
	/* initiator stat counters */
	uint32 txampdu;		/* ampdus sent */
#ifdef WLCNT
	uint32 txmpdu;		/* mpdus sent */
	uint32 txregmpdu;	/* regular(non-ampdu) mpdus sent */
	union {
		uint32 txs;		/* MAC agg: txstatus received */
		uint32 txretry_mpdu;	/* retransmitted mpdus */
	} u0;
	uint32 txretry_ampdu;	/* retransmitted ampdus */
	uint32 txfifofull;	/* release ampdu due to insufficient tx descriptors */
	uint32 txfbr_mpdu;	/* retransmitted mpdus at fallback rate */
	uint32 txfbr_ampdu;	/* retransmitted ampdus at fallback rate */
	union {
		uint32 txampdu_sgi;	/* ampdus sent with sgi */
		uint32 txmpdu_sgi;	/* ucode agg: mpdus sent with sgi */
	} u1;
	union {
		uint32 txampdu_stbc;	/* ampdus sent with stbc */
		uint32 txmpdu_stbc;	/* ucode agg: mpdus sent with stbc */
	} u2;
	uint32 txampdu_mfbr_stbc; /* ampdus sent at mfbr with stbc */
	uint32 txrel_wm;	/* mpdus released due to lookahead wm */
	uint32 txrel_size;	/* mpdus released due to max ampdu size */
	uint32 sduretry;	/* sdus retry returned by sendsdu() */
	uint32 sdurejected;	/* sdus rejected by sendsdu() */
	uint32 txdrop;		/* dropped packets */
	uint32 txr0hole;	/* lost packet between scb and sendampdu */
	uint32 txrnhole;	/* lost retried pkt */
	uint32 txrlag;		/* laggard pkt (was considered lost) */
	uint32 txreg_noack;	/* no ack for regular(non-ampdu) mpdus sent */
	uint32 txaddbareq;	/* addba req sent */
	uint32 rxaddbaresp;	/* addba resp recd */
	uint32 txlost;		/* lost packets reported in txs */
	uint32 txbar;		/* bar sent */
	uint32 rxba;		/* ba recd */
	uint32 noba;            /* ba missing */
	uint32 txstuck;		/* watchdog bailout for stuck state */
	uint32 orphan;		/* orphan pkts where scb/ini has been cleaned */

#ifdef WLAMPDU_MAC
	uint32 epochdelta;	/* How many times epoch has changed */
	uint32 echgr1;          /* epoch change reason -- plcp */
	uint32 echgr2;          /* epoch change reason -- rate_probe */
	uint32 echgr3;          /* epoch change reason -- a-mpdu as regmpdu */
	uint32 echgr4;          /* epoch change reason -- regmpdu */
	uint32 echgr5;          /* epoch change reason -- dest/tid */
	uint32 echgr6;          /* epoch change reason -- seq no */
	uint32 tx_mrt, tx_fbr;  /* number of MPDU tx at main/fallback rates */
	uint32 txsucc_mrt;      /* number of successful MPDU tx at main rate */
	uint32 txsucc_fbr;      /* number of successful MPDU tx at fallback rate */
	uint32 enq;             /* totally enqueued into aggfifo */
	uint32 cons;            /* totally reported in txstatus */
	uint32 pending;         /* number of entries currently in aggfifo or txsq */
#endif /* WLAMPDU_MAC */

	/* general: both initiator and responder */
	uint32 rxunexp;		/* unexpected packets */
	uint32 txdelba;		/* delba sent */
	uint32 rxdelba;		/* delba recd */

#ifdef WLPKTDLYSTAT
	/* PER (per mcs) statistics */
	uint32 txmpdu_cnt[AMPDU_HT_MCS_ARRAY_SIZE];		/* MPDUs per mcs */
	uint32 txmpdu_succ_cnt[AMPDU_HT_MCS_ARRAY_SIZE];	/* acked MPDUs per MCS */
#ifdef WL11AC
	uint32 txmpdu_vht_cnt[AMPDU_MAX_VHT];	/* MPDUs per vht */
	uint32 txmpdu_vht_succ_cnt[AMPDU_MAX_VHT]; /* acked MPDUs per vht */
#endif /* WL11AC */
#endif /* WLPKTDLYSTAT */

#ifdef WL_CS_PKTRETRY
	uint32 cs_pktretry_cnt;
#endif // endif

	uint32 ampdu_wds;	/* AMPDU watchdogs */
	uint32	txampdubyte_h;		/* tx ampdu data bytes */
	uint32	txampdubyte_l;
#endif /* WLCNT */
} wlc_ampdu_tx_cnt_t;

/**
 * structure to hold tx fifo information and pre-loading state counters specific to tx underflows of
 * ampdus. Some counters might be redundant with the ones in wlc or ampdu structures.
 * This allows to maintain a specific state independently of how often and/or when the wlc counters
 * are updated.
 */
typedef struct wlc_fifo_info {
	uint16 ampdu_pld_size;	/* number of bytes to be pre-loaded */
	uint8 mcs2ampdu_table[AMPDU_HT_MCS_ARRAY_SIZE]; /* per-mcs max # of mpdus in an ampdu */
	uint16 prev_txfunfl;	/* num of underflows last read from the HW macstats counter */
	uint32 accum_txfunfl;	/* num of underflows since we modified pld params */
	uint32 accum_txampdu;	/* num of tx ampdu since we modified pld params  */
	uint32 prev_txampdu;	/* previous reading of tx ampdu */
	uint32 dmaxferrate;	/* estimated dma avg xfer rate in kbits/sec */
} wlc_fifo_info_t;

#ifdef WLAMPDU_MAC
/** frame type enum for epoch change */
enum {
	AMPDU_NONE,
	AMPDU_11N,
	AMPDU_11VHT
};

typedef struct ampdumac_info {
#ifdef WLAMPDU_UCODE
	uint16 txfs_addr_strt;
	uint16 txfs_addr_end;
	uint16 txfs_wptr;
	uint16 txfs_rptr;
	uint16 txfs_wptr_addr;
	uint8  txfs_wsize;
#endif // endif
	uint8 epoch;
	bool change_epoch;
	/* any change of following elements will change epoch */
	struct scb *prev_scb;
	uint8 prev_tid;
	uint8 prev_ft;
	uint16 prev_txphyctl0, prev_txphyctl1;
	/* To keep ordering consistent with pre-rev40 prev_plcp[] use,
	 * plcp to prev_plcp mapping is not straightforward
	 *
	 * prev_plcp[0] holds plcp[0] (all revs)
	 * prev_plcp[1] holds plcp[3] (all revs)
	 * prev_plcp[2] holds plcp[2] (rev >= 40)
	 * prev_plcp[3] holds plcp[1] (rev >= 40)
	 * prev_plcp[4] holds plcp[4] (rev >= 40)
	 */
#if D11CONF_GE(40)
	uint8 prev_plcp[5];
#else
	uint8 prev_plcp[2];
#endif // endif
	/* stats */
	int in_queue;
	uint8 depth;
	uint8 prev_shdr;
} ampdumac_info_t;
#endif /* WLAMPDU_MAC */

#ifdef WLOVERTHRUSTER
typedef struct wlc_tcp_ack_info {
	uint8 tcp_ack_ratio;
	uint32 tcp_ack_total;
	uint32 tcp_ack_dequeued;
	uint32 tcp_ack_multi_dequeued;
	uint32 current_dequeued;
	uint8 tcp_ack_ratio_depth;
} wlc_tcp_ack_info_t;
#endif // endif

typedef struct {
	uint32 retry_histogram[AMPDU_MAX_MPDU+1]; /* histogram for retried pkts */
	uint32 end_histogram[AMPDU_MAX_MPDU+1];	/* errs till end of ampdu */
	uint32 mpdu_histogram[AMPDU_MAX_MPDU+1]; /* mpdus per ampdu histogram */
	/* txmcs in sw agg is ampdu cnt, and is mpdu cnt for mac agg */
	uint32 txmcs[AMPDU_HT_MCS_ARRAY_SIZE];		/* mcs of tx pkts */
	uint32 supr_reason[NUM_TX_STATUS_SUPR];	/* reason for suppressed err code */

#ifdef WLAMPDU_UCODE
	uint32 schan_depth_histo[AMPDU_MAX_MPDU+1]; /* side channel depth */
#endif /* WLAMPDU_UCODE */
	uint32 txmcssgi[AMPDU_HT_MCS_ARRAY_SIZE];		/* mcs of tx pkts */
	uint32 txmcsstbc[AMPDU_HT_MCS_ARRAY_SIZE];	/* mcs of tx pkts */
#ifdef WLAMPDU_MAC
	/* used by aqm_agg to get PER */
	uint32 txmcs_succ[AMPDU_HT_MCS_ARRAY_SIZE];   /* succ mpdus tx per mcs */
#endif // endif

#ifdef WL11AC
	uint32 txvht[AMPDU_MAX_VHT];		/* vht of tx pkts */
#ifdef WLAMPDU_MAC
	/* used by aqm_agg to get PER */
	uint32 txvht_succ[AMPDU_MAX_VHT];    /* succ mpdus tx per vht */
#endif // endif
	uint32 txvhtsgi[AMPDU_MAX_VHT];		/* vht of tx pkts */
	uint32 txvhtstbc[AMPDU_MAX_VHT];		/* vht of tx pkts */
#endif /* WL11AC */
} ampdu_dbg_t;

typedef struct {
	uint16 txallfrm;
	uint16 txbcn;
	uint16 txrts;
	uint16 rxcts;
	uint16 rsptmout;
	uint16 rxstrt;
	uint32 txop;
} mac_dbg_t;

/** ampdu config info, mostly config information that is common across WLC */
typedef struct ampdu_tx_config {
	bool manual_mode;	/* addba tx to be driven by user */
	bool no_bar;		/* do not send bar on failure */
	uint8 ini_enable[AMPDU_MAX_SCB_TID]; /* per-tid initiator enable/disable of ampdu */
	uint8 ba_policy;	/* ba policy; immediate vs delayed */
	uint8 ba_tx_wsize;      /* Tx ba window size (in pdu) */
	uint8 retry_limit;	/* mpdu transmit retry limit */
	uint8 rr_retry_limit;	/* mpdu transmit retry limit at regular rate */
	uint8 retry_limit_tid[AMPDU_MAX_SCB_TID];	/* per-tid mpdu transmit retry limit */
	/* per-tid mpdu transmit retry limit at regular rate */
	uint8 rr_retry_limit_tid[AMPDU_MAX_SCB_TID];
	uint8 mpdu_density;	/* min mpdu spacing (0-7) ==> 2^(x-1)/8 usec */
	int8 max_pdu;		/* max pdus allowed in ampdu */
	uint16 dur;		/* max duration of an ampdu (in usec) */
	uint8 hiagg_mode;	/* agg mpdus with different retry cnt */
	uint8 probe_mpdu;	/* max mpdus allowed in ampdu for probe pkts */
	uint8 txpkt_weight;	/* weight of ampdu in txfifo; reduces rate lag */
	uint8 delba_timeout;	/* timeout after which to send delba (sec) */
	uint8 tx_rel_lowat;	/* low watermark for num of pkts in transit */
#ifdef AMPDU_NON_AQM
	uint32 ffpld_rsvd;	/* number of bytes to reserve for preload */
#if defined(WLPROPRIETARY_11N_RATES)
	uint32 max_txlen[AMPDU_HT_MCS_ARRAY_SIZE][2][2]; /* max size of ampdu per mcs, bw and sgi */
#else
	uint32 max_txlen[MCS_TABLE_SIZE][2][2];
#endif /* WLPROPRIETARY_11N_RATES */

	bool mfbr;		/* enable multiple fallback rate */
	uint32 tx_max_funl;              /* underflows should be kept such that
					  * (tx_max_funfl*underflows) < tx frames
					  */
	wlc_fifo_info_t fifo_tb[NUM_FFPLD_FIFO]; /* table of fifo infos  */

	uint8  aggfifo_depth;   /* soft capability of AGGFIFO */
#endif /* non-AQM */
	int8 ampdu_aggmode;	/* aggregation mode, HOST or MAC */
	int8 default_pdu;	/* default pdus allowed in ampdu */
#ifdef WLMEDIA_TXFAILEVENT
	bool tx_fail_event;     /* Whether host requires TX failure event: ON/OFF */
#endif /* WLMEDIA_TXFAILEVENT */
	bool	fb_override;		/* override for frame bursting in absense of ba session */
	bool	fb_override_enable; /* configuration to enable/disable ampd_no_frameburst */
	uint8	ba_max_tx_wsize;	/* Tx ba window size (in pdu) */
	uint8	release;			/* # of mpdus released at a time */
	uint	txq_time_allowance_us;
	uint    txq_time_min_allowance_us;
	/* dynamic frameburst variables */
	uint8  dyn_fb_rtsper_on;
	uint8  dyn_fb_rtsper_off;
	uint32 dyn_fb_minrtscnt;
	/* AMPDU atf low water mark release count threshold */
	uint	ampdu_atf_lowat_rel_cnt;
} ampdu_tx_config_t;

/* DBG and counters are replicated per WLC */
/** AMPDU tx module specific state */
struct ampdu_tx_info {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	int scb_handle;		/* scb cubby handle to retrieve data from scb (=remote party) */
#ifdef WLCNT
	wlc_ampdu_tx_cnt_t *cnt;	/* counters/stats */
#endif // endif
#ifdef WLAMPDU_MAC
	ampdumac_info_t unused[AC_COUNT];
#endif // endif
	ampdu_dbg_t *amdbg;
	mac_dbg_t *mdbg;
#ifdef WLC_HIGH_ONLY
	void *p;
	tx_status_t txs;
	bool waiting_status; /* To help sanity checks */
#endif // endif
#ifdef WLOVERTHRUSTER
	wlc_tcp_ack_info_t tcp_ack_info;  /* stores a mix of config & runtime */
#endif // endif
	bool txaggr_support;	/* Support ampdu tx aggregation */
	uint16  aqm_max_release[AMPDU_MAX_SCB_TID];
	struct ampdu_tx_config *config;
	int	bsscfg_handle;		/* BSSCFG cubby offset */
#ifdef WLAMPDU_MAC
	ampdumac_info_t hagg[NFIFO_EXT];
#endif // endif
#ifdef WLTAF
	uint16	taf_fb_accum_usec;
	uint16	taf_fb_last_usec;
	uint16	taf_fb_last_index;
#endif /* WLTAF */
	/* dynamic frameburst variables */
	uint32 rtstxcnt;	/* # rts sent */
	uint32 ctsrxcnt;	/* # cts received */
	uint8  rtsper_avg;	/* avg rtsper for stats */
	uint8 txnoprog_cnt;	/* number of sec having no pkt consumed while PENDING_OFF */
#ifdef WLATF
	uint32	tx_intransit;
#endif /* WLATF */
};

#ifdef WLMEDIA_TXFAILEVENT
static void wlc_tx_failed_event(wlc_info_t *wlc, struct scb *scb, wlc_bsscfg_t *bsscfg,
	tx_status_t *txs, void *p, uint32 flags);
#endif /* WLMEDIA_TXFAILEVENT */

#if defined(TXQ_MUX)
/**
 * @brief a structure to hold parameters for packet tx time calculations
 *
 * This structure holds parameters for the time estimate calculation of an MPDU in an AQM A-MPDU.
 * This allows less work per-packet to calculation tx time.
 */
typedef struct ampdu_aqm_timecalc {
	ratespec_t rspec;               /**< @brief ratespec used in calculation */
	uint32 max_bytes;               /**< @brief maximum bytes in an AMPDU at current rate */
	uint16 Ndbps;                   /**< @brief bits to 4us symbol ratio */
	uint8 sgi;                      /**< @brief 1 if tx uses SGI */
	uint16 fixed_overhead_us;       /**< @brief length independent fixed time overhead */
	uint16 fixed_overhead_pmpdu_us; /**< @brief length independent fixed time overhead
	                                 *          minimum per-mpdu
	                                 */
	uint16 min_limit;               /**< @brief length boundary for fixed overhead
	                                 *          contribution methods
	                                 */
} ampdu_aqm_timecalc_t;
#endif /* TXQ_MUX */

#ifdef WLATF
typedef struct atf_stats_range {
	uint32	max; /* Maximum value */
	uint32	min; /* Minimum value */
	uint32	avg; /* Average value */
	unsigned long accum; /* Accmulator for average calculations */
	uint32	iter; /* Number of samples in acccmulator */
} atf_stats_range_t;

typedef struct atf_stats {
	uint32 timelimited; /* Times AMPDU release aborted due to time limit */
	uint32 framelimited; /* number of times full time limit was not released */
	uint32 reloverride; /* number of times ATF PMODE overrode release limit */
	uint32 uflow; /* number of times input queue went empty */
	uint32 oflow; /* nmber of times output queue was full */
	uint32 npkts; /* number of packets dequeued */
	uint32 ndequeue; /* Number of dequeue attempts */
	uint32 qempty; /* Incremented when packets in flight hit zero */
	uint32 minrel; /* Num times a dequeue request was below minimum time release */
	uint32 singleton; /* Count of single packets released */
	uint32 flush; /* Flush ATF accounting due to previous ATF reset */
	uint32 eval; /* number times ATF evaluated for packet release */
	uint32 neval; /* num times ATF stopped release */
	uint32 rskip; /* Release of packet skipped */
	uint32 cache_hit; /* ratespec cache hit */
	uint32 cache_miss; /* ratespec cache miss */
	uint32 proc; /* Number of packets completed */
	uint32 reproc; /* Times a completed packet was reprocessed */
	uint32 reg_mpdu; /* Packet sent as a regular MPDU from the AMPDU path */
	atf_stats_range_t chunk_bytes;
	atf_stats_range_t chunk_pkts;
	atf_stats_range_t rbytes;
	atf_stats_range_t inflight;
	atf_stats_range_t pdu;
	atf_stats_range_t release;
	atf_stats_range_t transit;
	atf_stats_range_t inputq;
} atf_stats_t;

typedef struct atf_rbytes {
	uint max;
	uint min;
} atf_rbytes_t;

/* CSTYLED */
typedef ratespec_t (*atf_rspec_fn_t)(void *);

/* ATF rate calculation function.
 * Can be changed in realtime if fixed rates are imposed in runtime
 */
typedef struct {
	atf_rspec_fn_t	function;
	void *arg;
} atf_rspec_action_t;

struct scb_ampdu_tx;
struct scb_ampdu_tid_ini;

typedef struct atf_state {
	uint32 		released_bytes_inflight; /* Number of bytes pending in bytes */
	uint32 		released_packets_inflight; /* Number of pending pkts,
						 * the AMPDU TID structure has a similar counter
						 * but its used is overloaded to
						 * track powersave packets
						 */
	uint32 		last_est_rate;	 /* Last estimated rate */
	uint		released_bytes_target;
	uint		released_minbytes_target;
	uint   		ac;
	wlcband_t 	*band;
	uint 		atf;
	uint 		txq_time_allowance_us;
	uint 		txq_time_min_allowance_us;
	uint16		last_ampdu_fid;
	atf_rbytes_t	rbytes_target;
	uint 		reset; /* Number times ATF state was reset */
	rcb_t 		*rcb; /* Pointer to rate control block for this TID */
	ampdu_tx_info_t *ampdu_tx;	/* Back pointer to ampdu_tx structure */
	struct scb_ampdu_tx *scb_ampdu;	/* Back pointer to scb_ampdu structure */
	struct scb_ampdu_tid_ini *ini;	/* Back pointer to ini structure */
	wlc_info_t	*wlc;
	struct scb *scb;

	/* Pointer to access function that returns rate to be used */
	atf_rspec_action_t	rspec_action;	/* Get current rspec function */
	ratespec_t		rspec_override;	/* Fixed rate rspec override */
#ifdef WLCNT
	atf_stats_t	atf_stats;
#endif /* WLCNT */
} atf_state_t;
#endif /* WLATF */

/** structure to store per-tid state for the ampdu initiator */
struct scb_ampdu_tid_ini {
	uint8 ba_state;		/* ampdu ba state */
	uint8 ba_wsize;		/* negotiated ba window size (in pdu) */
	uint8 tid;		/* initiator tid for easy lookup */
	uint8 txretry[AMPDU_BA_MAX_WSIZE];	/* tx retry count; indexed by seq modulo */
	uint8 ackpending[AMPDU_BA_MAX_WSIZE/NBBY];	/* bitmap: set if ack is pending */
	uint8 barpending[AMPDU_BA_MAX_WSIZE/NBBY];	/* bitmap: set if bar is pending */
	uint16 tx_in_transit;	/* number of pending mpdus in transit in driver */
	uint16 barpending_seq;	/* seqnum for bar */
	uint16 acked_seq;	/* last ack recevied */
	uint16 start_seq;	/* seqnum of the first unacknowledged packet */
	uint16 max_seq;		/* max unacknowledged seqnum sent */
	uint16 tx_exp_seq;	/* next exp seq in sendampdu */
	uint16 next_enq_seq;    /* last pkt seq that has been sent to txfifo */
	uint16 rem_window;	/* remaining ba window (in pdus) that can be txed */
	uint16 bar_ackpending_seq; /* seqnum of bar for which ack is pending */
	uint16 retry_seq[AMPDU_BA_MAX_WSIZE]; /* seq of released retried pkts */
	uint16 retry_head;	/* head of queue ptr for retried pkts */
	uint16 retry_tail;	/* tail of queue ptr for retried pkts */
	uint16 retry_cnt;	/* cnt of retried pkts */
	bool bar_ackpending;	/* true if there is a bar for which ack is pending */
	bool alive;		/* true if making forward progress */
	uint8 retry_bar;	/* reason code if bar to be retried at watchdog */
	uint8 bar_cnt;		/* number of bars sent with no progress */
	uint8 addba_req_cnt;	/* number of addba_req sent with no progress */
	uint8 cleanup_ini_cnt;	/* number of sec waiting in pending off state */
	uint8 dead_cnt;		/* number of sec without any progress */
	uint8 off_cnt;		/* number of sec in off state before next try */
	struct scb *scb;	/* backptr for easy lookup */
#ifdef WLATF
	atf_state_t atf_state;
#endif // endif
#if defined(TXQ_MUX)
#if !defined(WLATF)
	uint   ac;              /* Access Category of this TID */
#endif /* WLATF */
	ampdu_aqm_timecalc_t timecalc[AC_COUNT];
#endif /* TXQ_MUX */

#ifdef PROP_TXSTATUS
	/* rem_window is used to record the remaining ba window for new packets.
	 * when suppression happened, some holes may exist in current ba window,
	 * but they are not counted in rem_window.
	 * Then suppr_window is introduced to record suppressed packet counts inside ba window.
	 * Using suppr_window, we can keep rem_window untouched during suppression.
	 * When suppressed packets are resent by host, we need take both rem_window and
	 * suppr_window into account for decision of packet release.
	 */
	uint16 suppr_window;	/* suppr packet count inside ba window */
#endif /* PROP_TXSTATUS */
};

#ifdef BCMDBG
typedef struct scb_ampdu_cnt_tx {
	uint32 txampdu;
	uint32 txmpdu;
	uint32 txdrop;
	uint32 txstuck;
	uint32 txaddbareq;
	uint32 txrlag;
	uint32 txnoroom;
	uint32 sduretry;
	uint32 sdurejected;
	uint32 txlost;
	uint32 txbar;
	uint32 txreg_noack;
	uint32 noba;
	uint32 rxaddbaresp;
	uint32 rxdelba;
	uint32 rxba;
} scb_ampdu_cnt_tx_t;
#endif	/* BCMDBG */

/**
 * Scb cubby structure. Ini and resp are dynamically allocated if needed. A lot of instances of this
 * structure can be generated on e.g. APs, so be careful with respect to memory size of this struct.
 */
struct scb_ampdu_tx {
	struct scb *scb;                /* back pointer for easy reference */
	ampdu_tx_info_t *ampdu_tx;      /* back ref to main ampdu */
	scb_ampdu_tid_ini_t *ini[AMPDU_MAX_SCB_TID];    /* initiator info */
	uint8 tidmask;			/* bitmask indicating which tids have BA on; used for NAR */
	uint8 mpdu_density;		/* mpdu density */
	uint8 max_pdu;			/* max pdus allowed in ampdu */
	uint8 release;			/* # of mpdus released at a time */
	uint16 min_len;			/* min mpdu len to support the density */
	uint32 max_rxlen;		/* max ampdu rcv length; 8k, 16k, 32k, 64k */
	struct pktq txq;		/* sdu transmit queue pending aggregation */
	uint16 min_lens[AMPDU_HT_MCS_ARRAY_SIZE]; /* min mpdu lens per mcs */
	uint8 PAD;			/* was max_rxlen_factor */
	uint8 max_rxfactor;             /* max ampdu length exponent + 13 */
	                                /* (see Table 8-183v in IEEE Std 802.11ac-2012) */
#ifdef BCMDBG
	scb_ampdu_cnt_tx_t cnt;
#endif	/* BCMDBG */
	ampdu_tx_scb_stats_t *ampdu_scb_stats;
#ifdef WLATF
	uint32 txretry_pkt;
	uint32 tot_pkt;
#endif // endif
};

struct ampdu_tx_cubby {
	scb_ampdu_tx_t *scb_tx_cubby;
};

#define SCB_AMPDU_INFO(ampdu, scb) (SCB_CUBBY((scb), (ampdu)->scb_handle))
#define SCB_AMPDU_TX_CUBBY(ampdu, scb) \
	(((struct ampdu_tx_cubby *)SCB_AMPDU_INFO(ampdu, scb))->scb_tx_cubby)

extern uint8 wme_ac2fifo[];

/** bsscfg cubby structure. */
typedef struct bsscfg_ampdu_tx {
	int8 txaggr_override;	/* txaggr override for all TIDs */
	uint16 txaggr_TID_bmap; /* aggregation enabled TIDs bitmap */
} bsscfg_ampdu_tx_t;

#define BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg) \
	((bsscfg_ampdu_tx_t *)BSSCFG_CUBBY((bsscfg), (ampdu_tx)->bsscfg_handle))

/* local prototypes */
/* scb cubby */
static int scb_ampdu_tx_init(void *context, struct scb *scb);
static void scb_ampdu_tx_deinit(void *context, struct scb *scb);
/* bsscfg cubby */
static int bsscfg_ampdu_tx_init(void *context, wlc_bsscfg_t *cfg);
static void bsscfg_ampdu_tx_deinit(void *context, wlc_bsscfg_t *cfg);
static void scb_ampdu_txflush(void *context, struct scb *scb);
static int wlc_ampdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static void wlc_ampdu_watchdog(void *hdl);
static int wlc_ampdu_up(void *hdl);
static int wlc_ampdu_down(void *hdl);
static void wlc_ampdu_init_min_lens(scb_ampdu_tx_t *scb_ampdu);

static INLINE void wlc_ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);
static void ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);

static void wlc_ampdu_assoc_state_change(void *client, bss_assoc_state_data_t *notif_data);

#ifdef AMPDU_NON_AQM
static void ampdu_update_max_txlen(ampdu_tx_config_t *ampdu_tx_cfg, uint16 dur);
static void wlc_ffpld_init(ampdu_tx_config_t *ampdu_tx_cfg);
static int wlc_ffpld_check_txfunfl(wlc_info_t *wlc, int f);
static void wlc_ffpld_calc_mcs2ampdu_table(ampdu_tx_info_t *ampdu_tx, int f);
#ifdef BCMDBG
static void wlc_ffpld_show(ampdu_tx_info_t *ampdu_tx);
#endif /* BCMDBG */
#ifdef WLAMPDU_MAC
#if !defined(TXQ_MUX)
static int aggfifo_enque(ampdu_tx_info_t *ampdu_tx, int length, int qid);
static uint16 wlc_ampdu_calc_maxlen(ampdu_tx_info_t *ampdu_tx, uint8 plcp0, uint plcp3,
	uint32 txop);
#endif /* TXQ_MUX */
#if defined(BCMDBG) || !defined(TXQ_MUX)
static uint get_aggfifo_space(ampdu_tx_info_t *ampdu_tx, int qid);
#endif // endif
#endif /* WLAMPDU_MAC */
#endif /* non-AQM */

#ifdef WLOVERTHRUSTER
static void wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx_info_t *ampdu_tx, uint8 tcp_ack_ratio);
static void wlc_ampdu_tcp_ack_suppress(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
                                       void *p, uint8 tid);
#else
static bool wlc_ampdu_is_tcpack(ampdu_tx_info_t *ampdu_tx, void *p);
#endif // endif
#ifdef WLAMPDU_MAC
#ifdef BCMDBG
static void wlc_print_ampdu_txstatus(ampdu_tx_info_t *ampdu_tx,
	tx_status_t *pkg1, uint32 s1, uint32 s2, uint32 s3, uint32 s4);
#endif // endif
#endif /* WLAMPDU_MAC */

#if defined(TXQ_MUX)

#if defined(WLAMPDU_MAC)

/*
 * Following block of routines for AQM AMPDU aggregation (d11 core rev >= 40)
 */
static uint wlc_ampdu_output_aqm(void *ctx, uint ac, uint request_time, struct spktq *output_q);
static int wlc_ampdu_output_nonba(struct pktq *q, struct scb *scb, wlc_info_t *wlc,
                                  ratespec_t rspec, uint ac, uint tid,
                                  uint request_time, struct spktq *output_q);
static void wlc_ampdu_init_aqm_timecalc(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini,
                                        ratespec_t rspec, uint ac, ampdu_aqm_timecalc_t *tc);
static uint wlc_ampdu_aqm_timecalc(ampdu_aqm_timecalc_t *tc, uint mpdu_len);

#endif /* WLAMPDU_MAC */

#else

#ifdef WLAMPDU_MAC

#ifdef NEW_TXQ
static int BCMFASTPATH
_wlc_sendampdu_aqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time);
#else
static int BCMFASTPATH
_wlc_sendampdu_aqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec);
#endif /* NEW_TXQ */

#endif /* WLAMPDU_MAC */

#ifdef AMPDU_NON_AQM
#ifdef NEW_TXQ
static int BCMFASTPATH
_wlc_sendampdu_noaqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time);
#else
static int BCMFASTPATH
_wlc_sendampdu_noaqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec);
#endif /* NEW_TXQ */
#endif /* AMPDU_NON_AQM */

#endif /* TXQ_MUX */

#ifdef WLAMPDU_MAC
static void wlc_ampdu_dotxstatus_regmpdu_aqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs, wlc_txh_info_t *txh_info);

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

static bool BCMFASTPATH ampdu_is_exp_seq_aqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt);

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

static INLINE void wlc_ampdu_ini_move_window_aqm(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);
#endif /* WLAMPDU_MAC */

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

static void wlc_ampdu_dotxstatus_regmpdu_noaqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs);
static bool BCMFASTPATH ampdu_is_exp_seq_noaqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt);

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

static INLINE void wlc_ampdu_ini_move_window_noaqm(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);

static void wlc_ampdu_send_bar(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, bool start);

#if (defined(PKTC) || defined(PKTC_TX_DONGLE))
static void wlc_ampdu_agg_pktc(void *ctx, struct scb *scb, void *p, uint prec);
#else
static void wlc_ampdu_agg(void *ctx, struct scb *scb, void *p, uint prec);
#endif // endif
static scb_ampdu_tid_ini_t *wlc_ampdu_init_tid_ini(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, uint8 tid, bool override);
static void ampdu_ba_state_off(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini);
static bool wlc_ampdu_txeval(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, bool force);
static void wlc_ampdu_release(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, uint16 release, uint16 qavail, taf_scheduler_public_t* taf);
static void wlc_ampdu_free_chain(ampdu_tx_info_t *ampdu_tx, void *p, tx_status_t *txs,
	bool txs3_done);

static void wlc_send_bar_complete(wlc_info_t *wlc, uint txstatus, void *arg);

#if defined(DONGLEBUILD)
static void wlc_ampdu_txflowcontrol(wlc_info_t *wlc, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini);
#else
#define wlc_ampdu_txflowcontrol(a, b, c)	do {} while (0)
#endif // endif

static void wlc_ampdu_dotxstatus_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb, void *p,
	tx_status_t *txs, uint32 frmtxstatus, uint32 frmtxstatus2, uint32 s3, uint32 s4);

#ifdef BCMDBG
static void ampdu_dump_ini(scb_ampdu_tid_ini_t *ini);
#else
#define	ampdu_dump_ini(a)
#endif // endif

#if defined(BCMDBG) && defined(WLAMPDU_MAC)
static int wlc_dyn_fb_dump(wlc_info_t *wlc, struct bcmstrbuf *b);
#else
#define wlc_dyn_fb_dump(wlc, b)
#endif // endif

static uint wlc_ampdu_txpktcnt(void *hdl);

static void wlc_ampdu_ini_adjust(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, void *pkt);
static void wlc_ampdu_pkt_freed(wlc_info_t *wlc, void *pkt, uint txs);

#ifdef WLAWDL
static bool wlc_awdl_ampdu_txeval_chanspec(wlc_info_t *wlc, scb_ampdu_tx_t *scb_ampdu);
#endif // endif

#ifdef WLATF
static void wlc_ampdu_atf_tid_set_release_time(scb_ampdu_tid_ini_t *ini,
	uint txq_time_allowance_us);
static void wlc_ampdu_atf_scb_set_release_time(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint txq_time_allowance_us);
static void wlc_ampdu_atf_set_default_release_time(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint txq_time_allowance_us);

static void wlc_ampdu_atf_tid_set_release_mintime(scb_ampdu_tid_ini_t *ini,
	uint txq_time_allowance_us);
static void wlc_ampdu_atf_scb_set_release_mintime(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint txq_time_allowance_us);
static void wlc_ampdu_atf_set_default_release_mintime(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint txq_time_allowance_us);

static void wlc_ampdu_atf_tid_setmode(scb_ampdu_tid_ini_t *ini, uint32 mode);
static void wlc_ampdu_atf_scb_setmode(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint32 mode);
static void wlc_ampdu_atf_ini_set_rspec_action(atf_state_t *atf_state, ratespec_t rspec);

static atf_rbytes_t* BCMFASTPATH wlc_ampdu_atf_calc_rbytes(atf_state_t *atf_state,
	ratespec_t rspec);

#if defined(WLATF_CNT)
static BCMFASTPATH void wlc_ampdu_atf_update_rstat(atf_stats_range_t *range, uint32 val);
static void wlc_ampdu_atf_clear_counters(scb_ampdu_tid_ini_t *ini);
#define AMPDU_ATF_INCRSTAT(r, v) wlc_ampdu_atf_update_rstat((r), (v))
#define AMPDU_ATF_CLRCNT(ini) wlc_ampdu_atf_clear_counters((ini))
#define ATF_WLCNT_INCR(x) WLCNTINCR(x)
#else
#define AMPDU_ATF_INCRSTAT(r, v)
#define AMPDU_ATF_CLRCNT(ini)
#endif /* WLATF_CNT */
#else
#define AMPDU_ATF_INCRSTAT(r, v)
#define AMPDU_ATF_CLRCNT(ini)
#define ATF_WLCNT_INCR(x)
#endif /* WLATF_CNT */

static txmod_fns_t BCMATTACHDATA(ampdu_txmod_fns) = {
#if (defined(PKTC) || defined(PKTC_TX_DONGLE)) /* packet chaining */
	wlc_ampdu_agg_pktc,
#else
	wlc_ampdu_agg,
#endif // endif
	wlc_ampdu_txpktcnt,
	scb_ampdu_txflush,
	NULL
};

typedef struct {
	struct scb *scb;
	uint8 tid;
	ampdu_tx_info_t *ampdu_tx;
} ampdu_tx_map_pkts_cb_params_t;
static bool
wlc_ampdu_map_pkts_cb(void *ctx, void *pkt);
static void
wlc_ampdu_tx_map_pkts(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid);

#ifdef WLAMPDU_MAC
/** ncons marks last packet in tx chain that can be freed */
static uint16
wlc_ampdu_rawbits_to_ncons(uint16 raw_bits)
{
	return ((raw_bits & TX_STATUS40_NCONS) >> TX_STATUS40_NCONS_SHIFT);
}
#endif /* WLAMPDU_MAC */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static INLINE uint16
pkt_txh_seqnum(wlc_info_t *wlc, void *p)
{
	wlc_txh_info_t txh_info;
	wlc_get_txh_info(wlc, p, &txh_info);
	return txh_info.seq;
}

/** Selects the max Tx PDUs tunables for 2 stream and 3 stream depending on the hardware support. */
static void BCMATTACHFN(wlc_set_ampdu_tunables)(wlc_info_t *wlc)
{
	if (D11REV_GE(wlc->pub->corerev, D11AQM_CORE)) {
		wlc->pub->tunables->ampdunummpdu1stream  = AMPDU_NUM_MPDU_1SS_D11AQM;
		wlc->pub->tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU_2SS_D11AQM;
		wlc->pub->tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3SS_D11AQM;
	} else if (D11REV_GE(wlc->pub->corerev, D11HT_CORE)) {
		wlc->pub->tunables->ampdunummpdu1stream  = AMPDU_NUM_MPDU_1SS_D11HT;
		wlc->pub->tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU_2SS_D11HT;
		wlc->pub->tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3SS_D11HT;
	} else {
		wlc->pub->tunables->ampdunummpdu1stream  = AMPDU_NUM_MPDU_1SS_D11LEGACY;
		wlc->pub->tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU_2SS_D11LEGACY;
		wlc->pub->tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3SS_D11LEGACY;
	}
}

/** as part of system init, a transmit ampdu related data structure has to be initialized */
static void
BCMATTACHFN(wlc_ampdu_tx_cfg_init)(wlc_info_t *wlc, ampdu_tx_config_t *ampdu_tx_cfg)
{
	int i;
	ampdu_tx_cfg->ba_max_tx_wsize = AMPDU_TX_BA_MAX_WSIZE;

	/* initialize all priorities to allow AMPDU aggregation */
	for (i = 0; i < AMPDU_MAX_SCB_TID; i++)
		ampdu_tx_cfg->ini_enable[i] = TRUE;

	/* For D11 revs < 40, disable AMPDU on some priorities due to underlying fifo space
	 * allocation.
	 * D11 rev >= 40 has a shared tx fifo space managed by BMC hw that allows AMPDU agg
	 * to be used for any fifo.
	 */
	if (D11REV_LT(wlc->pub->corerev, 40)) {
		/* Disable ampdu for VO by default */
		ampdu_tx_cfg->ini_enable[PRIO_8021D_VO] = FALSE;
		ampdu_tx_cfg->ini_enable[PRIO_8021D_NC] = FALSE;

#ifndef MACOSX
		/* Disable ampdu for BK by default since not enough fifo space except for MACOS */
		ampdu_tx_cfg->ini_enable[PRIO_8021D_NONE] = FALSE;
		ampdu_tx_cfg->ini_enable[PRIO_8021D_BK] = FALSE;
#endif /* MACOSX */

		/* Enable aggregation for BK */
		if (D11REV_IS(wlc->pub->corerev, 28) ||
		    D11REV_IS(wlc->pub->corerev, 16) ||
		    D11REV_IS(wlc->pub->corerev, 17) ||
		    D11REV_IS(wlc->pub->corerev, 26) ||
		    (D11REV_IS(wlc->pub->corerev, 29) && WLCISHTPHY(wlc->band))) {
			ampdu_tx_cfg->ini_enable[PRIO_8021D_NONE] = TRUE;
			ampdu_tx_cfg->ini_enable[PRIO_8021D_BK] = TRUE;
		}
	}

	ampdu_tx_cfg->ba_policy = DOT11_ADDBA_POLICY_IMMEDIATE;
	ampdu_tx_cfg->ba_tx_wsize = AMPDU_TX_BA_DEF_WSIZE;

	if (ampdu_tx_cfg->ba_tx_wsize > ampdu_tx_cfg->ba_max_tx_wsize) {
		WL_ERROR(("wl%d: The Default AMPDU_TX_BA_WSIZE is greater than MAX value\n",
			wlc->pub->unit));
		ampdu_tx_cfg->ba_tx_wsize = ampdu_tx_cfg->ba_max_tx_wsize;
	}
	/* PR:106844: Adjust default tx AMPDU density for d11rev40/41 to 8us
	 * to avoid tx underflow. The bug is fixed in corerev42
	 */
	if (D11REV_IS(wlc->pub->corerev, 40) || D11REV_IS(wlc->pub->corerev, 41) ||
		D11REV_IS(wlc->pub->corerev, 17) || D11REV_IS(wlc->pub->corerev, 28)) {
		ampdu_tx_cfg->mpdu_density = AMPDU_DENSITY_8_US;
	} else {
		ampdu_tx_cfg->mpdu_density = AMPDU_DEF_MPDU_DENSITY;
	}
	ampdu_tx_cfg->max_pdu = AUTO;
	ampdu_tx_cfg->default_pdu =
		(wlc->stf->op_txstreams < 3) ? AMPDU_NUM_MPDU_LEGACY : AMPDU_MAX_MPDU;
	ampdu_tx_cfg->dur = AMPDU_MAX_DUR;
	ampdu_tx_cfg->hiagg_mode = FALSE;
	ampdu_tx_cfg->probe_mpdu = AMPDU_DEF_PROBE_MPDU;

	ampdu_tx_cfg->retry_limit = AMPDU_DEF_RETRY_LIMIT;
	ampdu_tx_cfg->rr_retry_limit = AMPDU_DEF_RR_RETRY_LIMIT;

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		ampdu_tx_cfg->retry_limit_tid[i] = ampdu_tx_cfg->retry_limit;
		ampdu_tx_cfg->rr_retry_limit_tid[i] = ampdu_tx_cfg->rr_retry_limit;
	}

	ampdu_tx_cfg->delba_timeout = 0; /* AMPDUXXX: not yet supported */
	ampdu_tx_cfg->tx_rel_lowat = AMPDU_DEF_TX_LOWAT;

#ifdef WLMEDIA_TXFAILEVENT
	ampdu_tx_cfg->tx_fail_event = FALSE; /* Tx failure notification off by default */
#endif // endif

#ifdef WLATF
	ampdu_tx_cfg->txq_time_allowance_us = AMPDU_TXQ_TIME_ALLOWANCE_US;
	ampdu_tx_cfg->txq_time_min_allowance_us = AMPDU_TXQ_TIME_MIN_ALLOWANCE_US;
	ampdu_tx_cfg->ampdu_atf_lowat_rel_cnt = AMPDU_ATF_LOWAT_REL_CNT;
#endif // endif
	ampdu_tx_cfg->ampdu_aggmode = AMPDU_AGGMODE_AUTO;

#ifdef WLAMPDU_UCODE
	if (CHIPID(wlc->pub->sih->chip) == BCM4330_CHIP_ID)
		ampdu_tx_cfg->ampdu_aggmode = AMPDU_AGGMODE_HOST;
#endif // endif

#ifdef AMPDU_NON_AQM
	ampdu_update_max_txlen(ampdu_tx_cfg, ampdu_tx_cfg->dur);
	ampdu_tx_cfg->ffpld_rsvd = AMPDU_DEF_FFPLD_RSVD;
	ampdu_tx_cfg->mfbr = TRUE;
	ampdu_tx_cfg->tx_max_funl = FFPLD_TX_MAX_UNFL;
	/* XXX 4314/43142 has throughput dip issue with default 200.
	 * When it happens, ffpld first try to increase preload size, then ampdu watchdog kicks
	 * in and down/up the driver
	 */
	if (CHIPID(wlc->pub->sih->chip) == BCM4314_CHIP_ID ||
		CHIPID(wlc->pub->sih->chip) == BCM43142_CHIP_ID)
		ampdu_tx_cfg->tx_max_funl = 1;
	wlc_ffpld_init(ampdu_tx_cfg);
#endif /* non-AQM */
} /* wlc_ampdu_tx_cfg_init */

/* cfg inits that must be done after certain variables/states are initialized:
 * 1) wlc->pub->_ampdumac
 */
static void
BCMATTACHFN(wlc_ampdu_tx_cfg_init_post)(wlc_info_t *wlc, ampdu_tx_config_t *ampdu_tx_cfg)
{
	if (AMPDU_MAC_ENAB(wlc->pub))
		ampdu_tx_cfg->txpkt_weight = 1;
	else
		ampdu_tx_cfg->txpkt_weight = AMPDU_DEF_TXPKT_WEIGHT;

	if (!AMPDU_AQM_ENAB(wlc->pub))
		ampdu_tx_cfg->release = AMPDU_SCB_MAX_RELEASE;
	else
		ampdu_tx_cfg->release = AMPDU_SCB_MAX_RELEASE_AQM;

#ifdef WLAMPDU_MAC
	ampdu_tx_cfg->dyn_fb_rtsper_on  = AMPDU_DYNFB_RTSPER_ON;
	ampdu_tx_cfg->dyn_fb_rtsper_off = AMPDU_DYNFB_RTSPER_OFF;
	ampdu_tx_cfg->dyn_fb_minrtscnt  = AMPDU_DYNFB_MINRTSCNT;
#endif // endif
}

ampdu_tx_info_t *
BCMATTACHFN(wlc_ampdu_tx_attach)(wlc_info_t *wlc)
{
	ampdu_tx_info_t *ampdu_tx;
	int i;

	/* some code depends on packed structures */
	STATIC_ASSERT(sizeof(struct dot11_bar) == DOT11_BAR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ba) == DOT11_BA_LEN + DOT11_BA_BITMAP_LEN);
	STATIC_ASSERT(sizeof(struct dot11_ctl_header) == DOT11_CTL_HDR_LEN);
	STATIC_ASSERT(sizeof(struct dot11_addba_req) == DOT11_ADDBA_REQ_LEN);
	STATIC_ASSERT(sizeof(struct dot11_addba_resp) == DOT11_ADDBA_RESP_LEN);
	STATIC_ASSERT(sizeof(struct dot11_delba) == DOT11_DELBA_LEN);
	STATIC_ASSERT(DOT11_MAXNUMFRAGS == NBITS(uint16));
	STATIC_ASSERT(ISPOWEROF2(AMPDU_TX_BA_MAX_WSIZE));

	wlc_set_ampdu_tunables(wlc);

	ASSERT(wlc->pub->tunables->ampdunummpdu2streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu2streams > 0);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams <= AMPDU_MAX_MPDU);
	ASSERT(wlc->pub->tunables->ampdunummpdu3streams > 0);

	if (!(ampdu_tx = (ampdu_tx_info_t *)MALLOC(wlc->osh, sizeof(ampdu_tx_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	bzero((char *)ampdu_tx, sizeof(ampdu_tx_info_t));
	ampdu_tx->wlc = wlc;
#ifndef WLRSDB_DVT
	ampdu_tx->config = (ampdu_tx_config_t*) obj_registry_get(wlc->objr,
		OBJR_AMPDUTX_CONFIG);
#endif // endif
	if (ampdu_tx->config == NULL) {
		if ((ampdu_tx->config =  (ampdu_tx_config_t*) MALLOCZ(wlc->pub->osh,
			sizeof(ampdu_tx_config_t))) == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes", wlc->pub->unit,
				__FUNCTION__, MALLOCED(wlc->pub->osh)));
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_AMPDUTX_CONFIG, ampdu_tx->config);
		wlc_ampdu_tx_cfg_init(wlc, ampdu_tx->config);
	}
#ifndef WLRSDB_DVT
		(void)obj_registry_ref(wlc->objr, OBJR_AMPDUTX_CONFIG);
#endif // endif

#ifdef WLCNT
	if (!(ampdu_tx->cnt = (wlc_ampdu_tx_cnt_t *)MALLOC(wlc->osh, sizeof(wlc_ampdu_tx_cnt_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	bzero((char *)ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif /* WLCNT */

	/* Read nvram param to see if it disables AMPDU tx aggregation */
	if ((getintvar(wlc->pub->vars, "11n_disable") &
		WLFEATURE_DISABLE_11N_AMPDU_TX)) {
		ampdu_tx->txaggr_support = FALSE;
	} else {
		ampdu_tx->txaggr_support = TRUE;
	}

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		if (i == PRIO_8021D_BE) {
			ampdu_tx->aqm_max_release[i] = AMPDU_AQM_RELEASE_MAX;
		} else {
			ampdu_tx->aqm_max_release[i] = AMPDU_AQM_RELEASE_DEFAULT;
		}
	}

#ifdef WLOVERTHRUSTER
	if ((WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) && (OVERTHRUST_ENAB(wlc->pub))) {
		wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx, AMPDU_DEF_TCP_ACK_RATIO);
		ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth = AMPDU_DEF_TCP_ACK_RATIO_DEPTH;
	}
#endif // endif

	/* reserve cubby in the bsscfg container for private data */
	if ((ampdu_tx->bsscfg_handle = wlc_bsscfg_cubby_reserve(wlc,
		sizeof(bsscfg_ampdu_tx_t), bsscfg_ampdu_tx_init, bsscfg_ampdu_tx_deinit,
		NULL, (void *)ampdu_tx)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container */
	ampdu_tx->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct ampdu_tx_cubby),
		scb_ampdu_tx_init, scb_ampdu_tx_deinit, NULL, (void *)ampdu_tx);

	if (ampdu_tx->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)|| defined(WLTEST) || defined(BCMDBG_AMPDU)
	if (!(ampdu_tx->amdbg = (ampdu_dbg_t *)MALLOCZ(wlc->osh, sizeof(ampdu_dbg_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif /*  defined(BCMDBG) || defined(WLTEST) */

	/* register assoc state change callback for reassociation */
	if (wlc_bss_assoc_state_register(wlc, wlc_ampdu_assoc_state_change, ampdu_tx) != BCME_OK) {
		WL_ERROR(("wl%d: wlc_bss_assoc_state_register() failed\n", wlc->pub->unit));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMPDU)
	if (!(ampdu_tx->mdbg = (mac_dbg_t *)MALLOCZ(wlc->osh, sizeof(mac_dbg_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes for mac_dbg_t\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
#endif /*  defined(BCMDBG) ... */

	/* register packet class callback */
	if (wlc_pcb_fn_set(wlc->pcb, 3, WLF2_PCB4_AMPDU, wlc_ampdu_pkt_freed) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_pcb_fn_set() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module -- needs to be last failure prone operation in this function */
	if (wlc_module_register(wlc->pub, ampdu_iovars, "ampdu_tx", ampdu_tx, wlc_ampdu_doiovar,
	                        wlc_ampdu_watchdog, wlc_ampdu_up, wlc_ampdu_down)) {
		WL_ERROR(("wl%d: ampdu_tx wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	/* register txmod function */
	wlc_txmod_fn_register(wlc, TXMOD_AMPDU, ampdu_tx, ampdu_txmod_fns);

	/* try to set ampdu to the default value */
	wlc_ampdu_tx_set(ampdu_tx, wlc->pub->_ampdu_tx);

	wlc_ampdu_tx_cfg_init_post(wlc, ampdu_tx->config);

	/* Attach debug function for dynamic framebursting */
#if defined(BCMDBG) && defined(WLAMPDU_MAC)
	wlc_dump_register(wlc->pub, "dynfb", (dump_fn_t)wlc_dyn_fb_dump, (void *)wlc);
#endif // endif

	return ampdu_tx;

fail:
	if (obj_registry_unref(wlc->objr, OBJR_AMPDUTX_CONFIG) == 0) {
		obj_registry_set(wlc->objr, OBJR_AMPDUTX_CONFIG, NULL);
		if (ampdu_tx->config != NULL) {
			MFREE(wlc->osh, ampdu_tx->config, sizeof(ampdu_tx_config_t));
		}
	}
#ifdef WLCNT
	if (ampdu_tx->cnt)
		MFREE(wlc->osh, ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif /* WLCNT */
	MFREE(wlc->osh, ampdu_tx, sizeof(ampdu_tx_info_t));
	return NULL;
}

#if defined(MACOSX) && !defined(TXQ_MUX)
/** called when MACOSX requests tx packets pending */
bool
wlc_ampdu_tid_enabled(wlc_info_t *wlc, uint tid)
{
	ampdu_tx_info_t *ampdu_tx = wlc ? wlc->ampdu_tx : NULL;
	scb_ampdu_tx_t *scb_ampdu = NULL;
	scb_ampdu_tid_ini_t *ini = NULL;
	struct scb *scb = NULL;
	struct scb_iter scbiter;
	int i;

	if (tid > MAXPRIO || (!ampdu_tx))
		return FALSE;

	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {

				ini = scb_ampdu->ini[i];

				if (ini && (tid == ini->tid) &&
					ampdu_tx->config->ini_enable[tid] &&
					(ini->ba_state == AMPDU_TID_STATE_BA_ON)) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}
#endif /* defined(MACOSX) && !defined(TXQ_MUX) */

void
BCMATTACHFN(wlc_ampdu_tx_detach)(ampdu_tx_info_t *ampdu_tx)
{
	ampdu_tx_config_t *config;
	wlc_info_t *wlc;

	if (!ampdu_tx)
		return;

	wlc = ampdu_tx->wlc;
	config = ampdu_tx->config;

	if (obj_registry_unref(wlc->objr, OBJR_AMPDUTX_CONFIG) == 0) {
		obj_registry_set(wlc->objr, OBJR_AMPDUTX_CONFIG, NULL);
		MFREE(wlc->osh, config, sizeof(ampdu_tx_config_t));
	}

#ifdef WLCNT
	if (ampdu_tx->cnt)
		MFREE(wlc->osh, ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
	if (ampdu_tx->amdbg) {
		MFREE(wlc->osh, ampdu_tx->amdbg, sizeof(ampdu_dbg_t));
		ampdu_tx->amdbg = NULL;
	}
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMPDU)
	if (ampdu_tx->mdbg) {
		MFREE(wlc->osh, ampdu_tx->mdbg, sizeof(mac_dbg_t));
		ampdu_tx->mdbg = NULL;
	}
#endif // endif

	wlc_bss_assoc_state_unregister(wlc, wlc_ampdu_assoc_state_change, ampdu_tx);
	wlc_module_unregister(wlc->pub, "ampdu_tx", ampdu_tx);
	MFREE(wlc->osh, ampdu_tx, sizeof(ampdu_tx_info_t));
}

#ifdef PKTQ_LOG              /* maintain packet statistics to debug e.g. packet drops \
	*/
struct pktq * scb_ampdu_prec_pktq(wlc_info_t* wlc, struct scb* scb)
{
	struct pktq *q;
	scb_ampdu_tx_t *scb_ampdu;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	q = &scb_ampdu->txq;

	return q;
}
#endif /* PKTQ_LOG */

/**
 * Per 'conversation partner' an SCB is maintained. The 'cubby' in this SCB contains ampdu tx info
 * for a specific conversation partner.
 */
static int
scb_ampdu_tx_init(void *context, struct scb *scb)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	struct ampdu_tx_cubby *cubby_info = (struct ampdu_tx_cubby *)SCB_AMPDU_INFO(ampdu_tx, scb);
	scb_ampdu_tx_t *scb_ampdu;
	wlc_tunables_t *tunables = ampdu_tx->wlc->pub->tunables;
#if defined(WLAMPDU_PRECEDENCE) || !defined(DONGLEBUILD)
	int i;
#endif // endif

	if (scb && !SCB_INTERNAL(scb)) {
		scb_ampdu = MALLOCZ(ampdu_tx->wlc->osh, sizeof(scb_ampdu_tx_t) +
			sizeof(ampdu_tx_scb_stats_t));
		if (!scb_ampdu)
			return BCME_NOMEM;
		cubby_info->scb_tx_cubby = scb_ampdu;
		scb_ampdu->scb = scb;
		scb_ampdu->ampdu_tx = ampdu_tx;
		scb_ampdu->ampdu_scb_stats = (ampdu_tx_scb_stats_t *)((char *)scb_ampdu +
			sizeof(scb_ampdu_tx_t));

#ifdef WLAMPDU_PRECEDENCE
#ifdef DONGLEBUILD
		pktq_init(&scb_ampdu->txq, WLC_PREC_COUNT,
			MAX(tunables->ampdu_pktq_size, tunables->ampdu_pktq_fav_size));
#else

		/* Set the overall queue packet limit to the max, just rely on per-prec limits */
		pktq_init(&scb_ampdu->txq, WLC_PREC_COUNT, PKTQ_LEN_MAX);
#endif /* DONGLEBUILD */
		for (i = 0; i < WLC_PREC_COUNT; i += 2) {
			pktq_set_max_plen(&scb_ampdu->txq, i, tunables->ampdu_pktq_size);
			pktq_set_max_plen(&scb_ampdu->txq, i+1, tunables->ampdu_pktq_fav_size);
		}
#else
#ifdef DONGLEBUILD
		pktq_init(&scb_ampdu->txq, AMPDU_MAX_SCB_TID, tunables->ampdu_pktq_size);
#else
		/* Set the overall queue packet limit to the max, just rely on per-prec limits */
		pktq_init(&scb_ampdu->txq, AMPDU_MAX_SCB_TID, PKTQ_LEN_MAX);
		for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
			pktq_set_max_plen(&scb_ampdu->txq, i, tunables->ampdu_pktq_size);
		}
#endif /* DONGLEBUILD */
#endif /* WLAMPDU_PRECEDENCE */
	}
	return 0;
}

/** scb cubby deinit fn */
static void
scb_ampdu_tx_deinit(void *context, struct scb *scb)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	struct ampdu_tx_cubby *cubby_info = (struct ampdu_tx_cubby *)SCB_AMPDU_INFO(ampdu_tx, scb);
	scb_ampdu_tx_t *scb_ampdu = NULL;

	WL_AMPDU_UPDN(("scb_ampdu_deinit: enter\n"));

	ASSERT(cubby_info);

	if (cubby_info)
		scb_ampdu = cubby_info->scb_tx_cubby;
	if (!scb_ampdu)
		return;

	scb_ampdu_tx_flush(ampdu_tx, scb);
#ifdef PKTQ_LOG
	wlc_pktq_stats_free(ampdu_tx->wlc, &scb_ampdu->txq);
#endif // endif

	pktq_deinit(&scb_ampdu->txq);

	MFREE(ampdu_tx->wlc->osh, scb_ampdu, sizeof(scb_ampdu_tx_t) +
		sizeof(ampdu_tx_scb_stats_t));
	cubby_info->scb_tx_cubby = NULL;
}

/* bsscfg cubby init fn */
static int
bsscfg_ampdu_tx_init(void *context, wlc_bsscfg_t *bsscfg)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);
	ASSERT(bsscfg != NULL);

	if (ampdu_tx->txaggr_support) {
		/* Enable for all TID by default */
		bsscfg_ampdu->txaggr_override = AUTO;
		bsscfg_ampdu->txaggr_TID_bmap = AMPDU_ALL_TID_BITMAP;
	} else {
		/* AMPDU TX module does not allow tx aggregation */
		bsscfg_ampdu->txaggr_override = OFF;
		bsscfg_ampdu->txaggr_TID_bmap = 0;
	}

	return BCME_OK;
}

/* bsscfg cubby deinit fn */
static void
bsscfg_ampdu_tx_deinit(void *context, wlc_bsscfg_t *bsscfg)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	WL_AMPDU_UPDN(("bsscfg_ampdu_tx_deinit: enter\n"));

	bsscfg_ampdu->txaggr_override = OFF;
	bsscfg_ampdu->txaggr_TID_bmap = 0;
}

/** part of the tx module (txmod_fns_t) interface */
static void
scb_ampdu_txflush(void *context, struct scb *scb)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)context;

	scb_ampdu_tx_flush(ampdu_tx, scb);
}

/** flush is needed on deinit, ampdu deactivation, etc. */
void
scb_ampdu_tx_flush(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	uint8 tid;

	WL_AMPDU_UPDN(("scb_ampdu_tx_flush: enter\n"));

	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
	}

	/* scb_ampdu->txq must be empty */
	BCM_REFERENCE(scb_ampdu);
	ASSERT(pktq_empty(&scb_ampdu->txq));

#ifdef WLC_HIGH_ONLY
	ASSERT(ampdu_tx->p == NULL);
#endif // endif
}

/**
 * Called as part of 'wl up' specific initialization. Reset the ampdu state machine so that it can
 * gracefully handle packets that were freed from the dma and tx queues during reinit.
 */
void
wlc_ampdu_tx_reset(ampdu_tx_info_t *ampdu_tx)
{
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	WL_AMPDU_UPDN(("wlc_ampdu_tx_reset: enter\n"));
	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb))
			continue;
		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;

			/* Shell we consider to cleaup always? */
			if (ini->ba_state != AMPDU_TID_STATE_BA_ON || ini->tx_in_transit)
				ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
		}
	}
}

/**
 * (Re)initialize tx config related to a specific 'conversation partner'. Called when setting up a
 * new conversation, as a result of a wl IOV_AMPDU_MPDU command or transmit underflow.
 */
void
scb_ampdu_update_config(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	int peer3ss = 0, peer2ss = 0;

	/* The maximum number of TX MPDUs supported for the device is determined at the
	  * AMPDU attach time
	 * At this point we know the largest size the device can send, here the intersection
	 * with the peer's capability is done.
	 */

	/* 3SS peer check */
	if (SCB_VHT_CAP(scb)) {
		/* VHT capable peer */
		peer3ss = VHT_MCS_SS_SUPPORTED(3, scb->rateset.vht_mcsmap);
		peer2ss = VHT_MCS_SS_SUPPORTED(2, scb->rateset.vht_mcsmap);
	} else {
		/* HT Peer */
		peer3ss = (scb->rateset.mcs[2] != 0);
		peer2ss = (scb->rateset.mcs[1] != 0);
	}

	if ((scb->flags & SCB_BRCM) && !(scb->flags2 & SCB2_RX_LARGE_AGG) && !(peer3ss))
	{
		scb_ampdu->max_pdu =
			MIN(ampdu_tx->wlc->pub->tunables->ampdunummpdu2streams,
			AMPDU_NUM_MPDU_LEGACY);
	} else {
		if (peer3ss) {
			scb_ampdu->max_pdu =
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu3streams;
		} else if (peer2ss) {
			scb_ampdu->max_pdu =
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu2streams;
		} else {
			scb_ampdu->max_pdu =
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu1stream;
		}
#ifdef WL_MU_TX
		if (SCB_MU(scb)) {
			/* XXX: In case of MU client, if tx amsdu_in_ampdu, set the 2nd highest
			 * ampdu_mpdu, otherwise the highest ampdu_mpdu.
			 * This is experimental conclusion, and once more studied,
			 * a dynamic tuning mechanism will be applied.
			 */
			scb_ampdu->max_pdu = SCB_AMSDU_IN_AMPDU(scb) ?
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu2streams :
				(uint8)ampdu_tx->wlc->pub->tunables->ampdunummpdu3streams;
		}
#endif // endif
	}

#ifdef WLATF
	wlc_ampdu_atf_scb_set_release_time(ampdu_tx, scb, ampdu_tx_cfg->txq_time_allowance_us);
#endif // endif

	ampdu_tx_cfg->default_pdu = scb_ampdu->max_pdu;

#ifdef AMPDU_NON_AQM
	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		int i;
		/* go back to legacy size if some preloading is occuring */
		for (i = 0; i < NUM_FFPLD_FIFO; i++) {
			if (ampdu_tx_cfg->fifo_tb[i].ampdu_pld_size > FFPLD_PLD_INCR)
				scb_ampdu->max_pdu = ampdu_tx_cfg->default_pdu;
		}
	}
#endif /* non-AQM */

	/* apply user override */
	if (ampdu_tx_cfg->max_pdu != AUTO)
		scb_ampdu->max_pdu = (uint8)ampdu_tx_cfg->max_pdu;

	if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
		scb_ampdu->release = ampdu_tx_cfg->ba_max_tx_wsize;
	} else {

		scb_ampdu->release = MIN(scb_ampdu->max_pdu, ampdu_tx_cfg->release);

#ifdef AMPDU_NON_AQM
		if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {

			if (scb_ampdu->max_rxlen)
				scb_ampdu->release =
				    MIN(scb_ampdu->release, scb_ampdu->max_rxlen/1600);

			/* XXX: limit based on BE preload restriction for highest MCS. May not be
			 * ideal, but is a good approximation.
			 */
			scb_ampdu->release = MIN(scb_ampdu->release,
			ampdu_tx_cfg->fifo_tb[TX_AC_BE_FIFO].mcs2ampdu_table[AMPDU_MAX_MCS]);

		}
#endif /* AMPDU_NON_AQM */
	}
	ASSERT(scb_ampdu->release);
}

/**
 * Initialize tx config related to all 'conversation partners'. Called as a result of a
 * wl IOV_AMPDU_MPDU command or transmit underflow.
 */
void
scb_ampdu_update_config_all(ampdu_tx_info_t *ampdu_tx)
{
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("scb_ampdu_update_config_all: enter\n"));
	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_update_config(ampdu_tx, scb);
	}
	/* The ampdu config affects values that are in the tx headers of outgoing packets.
	 * Invalidate tx header cache entries to clear out the stale information
	 */
	if (WLC_TXC_ENAB(ampdu_tx->wlc))
		wlc_txc_inv_all(ampdu_tx->wlc->txc);
}

/** Returns the number of transmit packets held by AMPDU. Part of the txmod_fns_t interface. */
static uint
wlc_ampdu_txpktcnt(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	struct scb *scb;
	struct scb_iter scbiter;
	int pktcnt = 0;
	scb_ampdu_tx_t *scb_ampdu;

	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			pktcnt += pktq_len(&scb_ampdu->txq);
		}
	}

	return pktcnt;
}

/** frees all the buffers and cleanup everything on down */
static int
wlc_ampdu_down(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	struct scb *scb;
	struct scb_iter scbiter;

	WL_AMPDU_UPDN(("wlc_ampdu_down: enter\n"));

	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb))
			scb_ampdu_tx_flush(ampdu_tx, scb);
	}

#ifdef AMPDU_NON_AQM
	/* we will need to re-run the pld tuning */
	wlc_ffpld_init(ampdu_tx->config);
#endif // endif

	return 0;
}

/* Init/Cleanup after Reinit/BigHammer */
static int
wlc_ampdu_up(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	wlc_info_t *wlc = ampdu_tx->wlc;
	struct scb_iter scbiter;
	struct scb *scb;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	if (!wlc->pub->up)
		return BCME_OK;

	/* reinit/bighammer */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb) &&
		    (scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb)) != NULL) {
			for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid ++) {
				if ((ini = scb_ampdu->ini[tid]) != NULL) {
					ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);
				}
			}
		}
	}

	return BCME_OK;
}

/* Reset BA agreement after a reassociation (roam/reconnect) */
static void
wlc_ampdu_assoc_state_change(void *client, bss_assoc_state_data_t *notif_data)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)client;
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_bsscfg_t *cfg = notif_data->cfg;
	struct scb *scb;

	/* force BA reset */

	if (BSSCFG_STA(cfg) &&
	    notif_data->type == AS_ROAM &&
	    /* We should do it on entering AS_JOIN_INIT state but
	     * unfortunately we don't have it for roam/reconnect.
	     */
	    notif_data->state == AS_JOIN_ADOPT) {
		if ((scb = wlc_scbfindband(wlc, cfg, &cfg->BSSID,
			CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec))) != NULL &&
		    SCB_AMPDU(scb)) {
			wlc_scb_ampdu_disable(wlc, scb);
			wlc_scb_ampdu_enable(wlc, scb);
		}
	}
}

/**
 * When tx aggregation is disabled by a higher software layer, AMPDU connections for all
 * 'conversation partners' in the caller supplied bss have to be torn down.
 */
/* If wish to do for all TIDs, input AMPDU_ALL_TID_BITMAP for conf_TDI_bmap */
static void
wlc_ampdu_tx_cleanup(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg,
	uint16 conf_TID_bmap)
{
	uint8 tid;
	scb_ampdu_tx_t *scb_ampdu = NULL;
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_info_t *wlc = ampdu_tx->wlc;

	scb_ampdu_tid_ini_t *ini;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (!SCB_AMPDU(scb))
			continue;

		if (!SCB_ASSOCIATED(scb))
			continue;

		scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			if (!(isbitset(conf_TID_bmap, tid))) {
				continue;
			}

			ini = scb_ampdu->ini[tid];

			if (ini != NULL)
			{
				if ((ini->ba_state == AMPDU_TID_STATE_BA_ON) ||
					(ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {

					/* Send delba */
					wlc_send_delba(ampdu_tx->wlc, scb, tid, TRUE,
						DOT11_RC_TIMEOUT);

					WLCNTINCR(ampdu_tx->cnt->txdelba);
				}

				/* Set state to off, and push the packets in ampdu_pktq to next */
				ampdu_ba_state_off(scb_ampdu, ini);

				/* Force cleanup */
				ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
			}
		}
	}
}

/**
 * frame bursting waits a shorter time (RIFS) between tx frames which increases throughput but
 * potentially decreases airtime fairness.
 */
bool
wlc_ampdu_frameburst_override(ampdu_tx_info_t *ampdu_tx)
{
#ifdef FRAMEBURST_RTSCTS_PER_AMPDU
	/* Frameburst always ON */
	return FALSE;
#else
	if (ampdu_tx == NULL)
		return FALSE;
	else
		return ampdu_tx->config->fb_override;
#endif /* FRAMEBURST_RTSCTS_PER_AMPDU */
}

#if defined(BCMDBG) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP) || \
	defined(BCMDBG_AMPDU)
/* offset of cntmember by sizeof(uint32) from the first cnt variable. */
#define NUM_MCST_IN_MAC_DBG_T 6
static const uint8 mac_dbg_t_to_macstat_t[NUM_MCST_IN_MAC_DBG_T] = {
	MCSTOFF_TXFRAME,
	MCSTOFF_TXBCNFRM,
	MCSTOFF_TXRTSFRM,
	MCSTOFF_RXCTSUCAST,
	MCSTOFF_RXRSPTMOUT,
	MCSTOFF_RXSTRT
};

static void
wlc_ampdu_dbg_stats(wlc_info_t *wlc, struct scb *scb, scb_ampdu_tid_ini_t *ini)
{
	uint8 i;
	mac_dbg_t now_mdbg;
	mac_dbg_t *mdbg;
	int corerev;
	osl_t *osh;
	d11regs_t *regs;

	if (ini->dead_cnt == 0) return;

	mdbg = wlc->ampdu_tx->mdbg;
	if (!mdbg)
		return;

	osh = wlc->osh;
	regs = wlc->regs;
	corerev = wlc->pub->corerev;
	memset(&now_mdbg, 0, sizeof(now_mdbg));
	for (i = 0; i < NUM_MCST_IN_MAC_DBG_T; i++) {
		*((uint16 *)&now_mdbg) =
			wlc_read_shm(wlc, MACSTAT_ADDR(mac_dbg_t_to_macstat_t[i]));
	}
	now_mdbg.txop = wlc_bmac_cca_read_counter(wlc->hw, M_CCA_TXOP_L, M_CCA_TXOP_H);

	if (ini->dead_cnt >= 2) {
		WL_ERROR(("ampdu_dbg: wl%d.%d "MACF" scb:%p tid:%d \n",
			wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			ETHER_TO_MACF(scb->ea), scb, ini->tid));
		if (D11REV_GE(corerev, 64)) {
			uint32 fifordy, cpbusy;
			fifordy = R_REG(osh, &regs->u.d11acregs.AQMFifoRdy_L);
			fifordy = fifordy | (R_REG(osh, &regs->u.d11acregs.AQMFifoRdy_H) << 16);
			cpbusy = R_REG(osh, &regs->u.d11acregs.AQMTBCP_Busy_L);
			cpbusy = cpbusy | (R_REG(osh, &regs->u.d11acregs.AQMTBCP_Busy_H) << 16);
			WL_ERROR(("ampdu_dbg: wl%d.%d dead_cnt %d tx_in_transit %d psm_mux %#x "
				"aqmqmap 0x%x aqmfifo_status 0x%x fifordy %#x cpbusy %#x\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
				ini->dead_cnt, ini->tx_in_transit,
				R_REG(osh, &regs->u.d11acregs.psm_reg_mux),
				R_REG(osh, &regs->u.d11acregs.AQMQMAP),
				R_REG(osh, &regs->u.d11acregs.AQMFifo_Status),
				fifordy, cpbusy));
		} else {
			uint16 fifordy, frmcnt, fifosel;
			if (D11REV_GE(corerev, 40)) {
				fifordy = R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady);
				fifosel = R_REG(osh, &regs->u.d11acregs.BMCCmd);
				frmcnt = R_REG(osh, &regs->u.d11acregs.XmtFifoFrameCnt);

			} else {
				fifordy = R_REG(osh, &regs->u.d11regs.xmtfifordy);
				fifosel = R_REG(osh, &regs->u.d11regs.xmtsel);
				frmcnt = R_REG(osh, &regs->u.d11regs.xmtfifo_frame_cnt);
			}
			WL_ERROR(("ampdu_dbg: wl%d.%d dead_cnt %d tx_in_transit %d "
				"fifordy 0x%x frmcnt 0x%x fifosel 0x%x\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
				ini->dead_cnt, ini->tx_in_transit,
				fifordy, frmcnt, fifosel));
			BCM_REFERENCE(fifordy);
			BCM_REFERENCE(fifosel);
			BCM_REFERENCE(frmcnt);
		}

		WL_ERROR(("ampdu_dbg: ifsstat 0x%x nav_stat 0x%x txop %u\n",
			R_REG(osh, &regs->u.d11regs.ifsstat),
			R_REG(osh, &regs->u.d11regs.navstat),
			now_mdbg.txop - mdbg->txop));

		WL_ERROR(("ampdu_dbg: pktpend: %d %d %d %d %d ap %d\n",
			TXPKTPENDGET(wlc, TX_AC_BK_FIFO), TXPKTPENDGET(wlc, TX_AC_BE_FIFO),
			TXPKTPENDGET(wlc, TX_AC_VI_FIFO), TXPKTPENDGET(wlc, TX_AC_VO_FIFO),
			TXPKTPENDGET(wlc, TX_BCMC_FIFO), AP_ENAB(wlc->pub)));

		WL_ERROR(("ampdu_dbg: txall %d txbcn %d txrts %d rxcts %d rsptmout %d rxstrt %d\n",
			now_mdbg.txallfrm - mdbg->txallfrm, now_mdbg.txbcn - mdbg->txbcn,
			now_mdbg.txrts - mdbg->txrts, now_mdbg.rxcts - mdbg->rxcts,
			now_mdbg.rsptmout - mdbg->rsptmout, now_mdbg.rxstrt - mdbg->rxstrt));

		WL_ERROR(("ampdu_dbg: cwcur0-3 %x %x %x %x bslots cur/0-3 %d %d %d %d %d "
			 "ifs_boff %d\n",
			 wlc_read_shm(wlc, 0x240 + 3*2), wlc_read_shm(wlc, 0x260 + 3*2),
			 wlc_read_shm(wlc, 0x280 + 3*2), wlc_read_shm(wlc, 0x2a0 + 3*2),
			 wlc_read_shm(wlc, 0x16f*2),
			 wlc_read_shm(wlc, 0x240 + 5*2), wlc_read_shm(wlc, 0x260 + 5*2),
			 wlc_read_shm(wlc, 0x280 + 5*2), wlc_read_shm(wlc, 0x2a0 + 5*2),
			 R_REG(osh, &regs->u.d11regs.ifs_boff)));

		for (i = 0; i < 2; i++) {
			WL_ERROR(("ampdu_dbg: again%d ifsstat 0x%x nav_stat 0x%x\n",
				i + 1,
				R_REG(osh, &regs->u.d11regs.ifsstat),
				R_REG(osh, &regs->u.d11regs.navstat)));
		}
	}

	/* Copy macstat_t counters */
	memcpy(mdbg, &now_mdbg, sizeof(now_mdbg));

#if defined(BCM_DMA_CT)
	if (BCM_DMA_CT_ENAB(wlc) && (ini->dead_cnt >= 2)) {
		int npkts;
		for (i = 0; i < WLC_HW_NFIFO_INUSE(wlc); i++) {
			npkts = TXPKTPENDGET(wlc, i);
			if (npkts == 0) continue;
			WL_ERROR(("FIFO-%d TXPEND = %d TX-DMA%d =>\n", i, npkts, i));
#if (defined(BCMDBG) || defined(BCMDBG_DUMP))
			if (ini->dead_cnt >= 8 || wlc->ampdu_tx->txnoprog_cnt > 0) {
				hnddma_t *di = WLC_HW_DI(wlc, i);
				dma_dumptx(di, NULL, FALSE);
				WL_ERROR(("CT-DMA%d =>\n", i));
				di = WLC_HW_AQM_DI(wlc, i);
				dma_dumptx(di, NULL, FALSE);
			}
#endif /* BCMDBG || BCMDBG_DUMP */
		}
	}
#endif /* BCM_DMA_CT */

	if (wlc->block_datafifo) {
		WL_ERROR(("block_datafifo 0x%x\n", wlc->block_datafifo));
	}
}
#else
static void
wlc_ampdu_dbg_stats(wlc_info_t *wlc, struct scb *scb, scb_ampdu_tid_ini_t *ini)
{
}
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */

/**
 * Timer function, called approx every second. Resends ADDBA-Req if the ADDBA-Resp has not come
 * back.
 */
static void
wlc_ampdu_watchdog(void *hdl)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	wlc_info_t *wlc = ampdu_tx->wlc;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;
	bool any_ba_state_on = FALSE;
	bool any_ba_state_pendoff = FALSE;
	bool any_ht_cap = FALSE;
	uint8 rtsper_avg;
#ifdef WLATF
	uint	txq_time_allowance_us = 0;
	uint16  retry_percentage = 0;
#endif // endif

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_HT_CAP(scb))any_ht_cap = TRUE;

		if (!SCB_AMPDU(scb))
			continue;
#ifdef WLAWDL
		if (AWDL_SUPPORT(wlc->pub)) {
			if (BSSCFG_AWDL(wlc, scb->bsscfg) &&
				(!wlc_awdl_in_aw(wlc) || !wlc_awdl_valid_channel(wlc) ||
				wlc_awdl_in_flow_ctrl(wlc) ||
				!wlc_awdl_in_valid_peer_chan(wlc, scb)))
				continue;
			if (AWDL_ENAB(wlc->pub) && (!(BSSCFG_AWDL(wlc, scb->bsscfg)) &&
				(wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC) !=
				wf_chspec_ctlchan(scb->bsscfg->current_bss->chanspec))))
				continue;
		}
#endif // endif
		scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);

		if (!scb_ampdu) {
			if (scb->flags & SCB_PENDING_FREE) {
				WL_ERROR(("%s: scb->flags(%x) SCB_PENDING_FREE set, mac="MACF"\n",
					__FUNCTION__, scb->flags, ETHER_TO_MACF(scb->ea)));
			} else {
				ASSERT(0);
				WL_ERROR(("%s: bypass NULL scb_ampdu, mac="MACF"\n",
					__FUNCTION__, ETHER_TO_MACF(scb->ea)));
			}
			continue;
		}

#ifdef WLATF

#ifdef WL_MU_TX
		/* Skipping retry compensation for MU since the retry
		 * counters are not accurate for MU - needs investgation
		 */
		if ((!MU_TX_ENAB(wlc)) || (!SCB_MU(scb)))
#endif // endif
		{
			txq_time_allowance_us = ampdu_tx_cfg->txq_time_allowance_us;
			retry_percentage = scb_ampdu->tot_pkt ?
				(scb_ampdu->txretry_pkt*100)/scb_ampdu->tot_pkt: 0;

			/* Compensate only if retry is greater than compensate threshold */
			if (retry_percentage > AMPDU_RETRY_COMPENSATE_THRESH) {
				txq_time_allowance_us = (ampdu_tx_cfg->txq_time_allowance_us * 100)
					/(100 + retry_percentage);
				if (txq_time_allowance_us <
					ampdu_tx_cfg->txq_time_min_allowance_us) {
					txq_time_allowance_us =
						ampdu_tx_cfg->txq_time_min_allowance_us;
				}
			}
			wlc_ampdu_atf_scb_set_release_time(ampdu_tx, scb, txq_time_allowance_us);

		}

		scb_ampdu->tot_pkt = 0;
		scb_ampdu->txretry_pkt = 0;
#endif /* WLATF */

		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {

			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;

			switch (ini->ba_state) {
			case AMPDU_TID_STATE_BA_ON:
				/* tickle the sm and release whatever pkts we can */
				wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, TRUE);

				any_ba_state_on = TRUE;

				if (ini->retry_bar) {
#if !defined(TXQ_MUX)
					/* Free queued packets when system is short of memory;
					 * release 1/4 buffered and at most wl_ampdu_drain_cnt pkts.
					 */
					if (ini->retry_bar == AMPDU_R_BAR_NO_BUFFER) {
						void *p;
						int rel_cnt, i;

						WL_ERROR(("wl%d: %s: no memory\n",
							wlc->pub->unit, __FUNCTION__));
						rel_cnt = wlc_ampdu_pktq_plen(&scb_ampdu->txq,
							ini->tid) >> 2;
						if (wl_ampdu_drain_cnt <= 0)
							wl_ampdu_drain_cnt = AMPDU_SCB_DRAIN_CNT;
						rel_cnt = MIN(rel_cnt, wl_ampdu_drain_cnt);
						for (i = 0; i < rel_cnt; i++) {
							p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq,
								ini->tid);
							ASSERT(p != NULL);
							ASSERT(PKTPRIO(p) == ini->tid);
							PKTFREE(wlc->pub->osh, p, TRUE);
							AMPDUSCBCNTINCR(scb_ampdu->cnt.txdrop);
							WLCNTINCR(ampdu_tx->cnt->txdrop);
#ifdef WL11K
							wlc_rrm_stat_qos_counter(scb, tid,
								OFFSETOF(rrm_stat_group_qos_t,
								txdrop));
#endif // endif
						}
					}
#endif /* TXQ_MUX */
#ifdef WLAWDL
					if (BSSCFG_AWDL(wlc, scb->bsscfg))
						wlc_awdl_update_peer_info(wlc, &scb->ea);
#endif // endif
					wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
				}
				else if (ini->bar_cnt >= AMPDU_BAR_RETRY_CNT) {
#ifdef WLAWDL
					if (BSSCFG_AWDL(wlc, scb->bsscfg))
						wlc_awdl_update_peer_info(wlc, &scb->ea);
#endif // endif
					wlc_ampdu_tx_send_delba(ampdu_tx, scb, tid,
						TRUE, DOT11_RC_TIMEOUT);
				}
				/* check on forward progress */
				else if (ini->alive) {
					ini->alive = FALSE;
					ini->dead_cnt = 0;
				} else {
					uint16 interval = 1 + ((scb->listen *
						scb->bsscfg->current_bss->beacon_period)/1000);
					if (ini->tx_in_transit)
						ini->dead_cnt++;

					wlc_ampdu_dbg_stats(wlc, scb, ini);

					/* using  maximum of listen interval+1 sec
					 * or AMPDU_INI_DEAD_TIMEOUT
					 * to avoid packet drop due dead count
					 */
					if (ini->dead_cnt == MAX(AMPDU_INI_DEAD_TIMEOUT,
						interval)) {
#if defined(AP) && defined(BCMDBG)
						if (SCB_PS(scb)) {
							char* mode = "PS";

#ifdef PSPRETEND
							if (SCB_PS_PRETEND(scb)) {
								if (SCB_PS_PRETEND_THRESHOLD(scb)) {
									mode = "threshold PPS";
								}
								else {
									mode = "PPS";
								}
							}
#endif /* PSPRETEND */
							WL_ERROR(("wl%d.%d: %s: "MACF" in %s "
							     "mode may be stuck or receiver died\n",
							     wlc->pub->unit,
							     WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
							     __FUNCTION__, ETHER_TO_MACF(scb->ea),
							     mode));
						}
#endif /* AP && BCMDBG */

						WL_ERROR(("wl%d: %s: cleaning up ini"
							" tid %d due to no progress for %d secs"
							" tx_in_transit %d\n",
							wlc->pub->unit, __FUNCTION__,
							tid, ini->dead_cnt, ini->tx_in_transit));
						WLCNTINCR(wlc->pub->_cnt->ampdu_wds);
						WLCNTINCR(ampdu_tx->cnt->ampdu_wds);
						WLCNTINCR(ampdu_tx->cnt->txstuck);
						AMPDUSCBCNTINCR(scb_ampdu->cnt.txstuck);
						/* AMPDUXXX: unknown failure, send delba */
#ifdef WLAWDL
						if (BSSCFG_AWDL(wlc, scb->bsscfg))
							wlc_awdl_update_peer_info(wlc, &scb->ea);
#endif // endif
						wlc_ampdu_tx_send_delba(ampdu_tx, scb, tid,
							TRUE, DOT11_RC_TIMEOUT);
						ampdu_dump_ini(ini);
					}
				}
				break;

			case AMPDU_TID_STATE_BA_PENDING_ON: {
				int ampdu_addba_req_retry = AMPDU_ADDBA_REQ_RETRY_CNT;
#if defined(WLAWDL) && defined(AWDL_FAMILY)
				/* increase addba_req retry count for AWDL bsscfg */
				if (BSSCFG_AWDL(wlc, scb->bsscfg))
					ampdu_addba_req_retry += 5;
				WL_AMPDU(("%s(): BA_PENDING_ON, addba_req_cnt=%d\n",
					__FUNCTION__, ini->addba_req_cnt));
#endif // endif
				/* resend addba if not enough retries */
				if (ini->addba_req_cnt++ >= ampdu_addba_req_retry) {
					ampdu_ba_state_off(scb_ampdu, ini);
				} else {
#ifdef WLAWDL
					if (BSSCFG_AWDL(wlc, scb->bsscfg))
						wlc_awdl_update_peer_info(wlc, &scb->ea);
#endif // endif
#ifdef MFP
					if (!WLC_MFP_ENAB(wlc->pub) ||
#if defined(WLAWDL) && defined(AWDL_FAMILY)
						(BSSCFG_AWDL(wlc, scb->bsscfg)) ||
#endif // endif
						(WLC_MFP_ENAB(wlc->pub) && SCB_AUTHORIZED(scb)))
#endif /* WLAWDL */
					{
						WL_AMPDU_ERR(("%s: addba timed out\n",
							__FUNCTION__));
						wlc_send_addba_req(wlc, scb, tid,
							ampdu_tx_cfg->ba_tx_wsize,
							ampdu_tx_cfg->ba_policy,
							ampdu_tx_cfg->delba_timeout);
					}

					WLCNTINCR(ampdu_tx->cnt->txaddbareq);
					AMPDUSCBCNTINCR(scb_ampdu->cnt.txaddbareq);
				}
				break;
			}
			case AMPDU_TID_STATE_BA_PENDING_OFF: {
				any_ba_state_pendoff = TRUE;
				if (ini->cleanup_ini_cnt++ >= AMPDU_INI_CLEANUP_TIMEOUT) {
					uint32 phydebug, txop, fifordy, txe_ctl;
					osl_t *osh = wlc->osh;
					d11regs_t *regs = wlc->regs;

					phydebug = R_REG(osh, &regs->phydebug);
					txop = wlc_bmac_cca_read_counter(wlc->hw, M_CCA_TXOP_L,
						M_CCA_TXOP_H);
					if (D11REV_GE(wlc->pub->corerev, 64)) {
						fifordy = R_REG(osh,
							&regs->u.d11acregs.AQMFifoRdy_L);
						fifordy = fifordy | (R_REG(osh,
							&regs->u.d11acregs.AQMFifoRdy_H) << 16);
					} else if (D11REV_GE(wlc->pub->corerev, 40)) {
						fifordy = R_REG(osh,
							&regs->u.d11acregs.u0.lt64.AQMFifoReady);
					} else {
						fifordy = R_REG(osh, &regs->u.d11regs.xmtfifordy);
					}
					txe_ctl = R_REG(osh, &regs->txe_ctl);
					WL_ERROR(("wl%d: %s: cleaning up tid %d from poff"
						" phydebug %08x txop %08x"
						" fifordy %08x txe_ctl %08x txnoprog_cnt %d\n",
						wlc->pub->unit, __FUNCTION__, tid, phydebug,
						txop, fifordy, txe_ctl, ampdu_tx->txnoprog_cnt));
					BCM_REFERENCE(phydebug);
					BCM_REFERENCE(txop);
					BCM_REFERENCE(fifordy);
					BCM_REFERENCE(txe_ctl);
					WLCNTINCR(ampdu_tx->cnt->txstuck);
					AMPDUSCBCNTINCR(scb_ampdu->cnt.txstuck);
					ampdu_dump_ini(ini);
					wlc_ampdu_dbg_stats(wlc, scb, ini);
					ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);
				} else {
					ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, FALSE);
				}
				break;
			}
			case AMPDU_TID_STATE_BA_OFF:
				if (ini->off_cnt++ >= AMPDU_INI_OFF_TIMEOUT) {
					ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu,
						tid, FALSE);
					/* make a single attempt only */
					if (ini)
						ini->addba_req_cnt = AMPDU_ADDBA_REQ_RETRY_CNT;
				}
				break;
			default:
				break;
			}
			/* dont do anything with ini here since it might be freed */

		}
	}

	/* Dynamic Frameburst */
	if (ampdu_tx->wlc->hti->frameburst && ampdu_tx_cfg->fb_override_enable) {
		if (any_ht_cap && (!any_ba_state_on)) {
			ampdu_tx_cfg->fb_override = TRUE;
		} else {
#ifndef WLAMPDU_MAC
			ampdu_tx_cfg->fb_override = FALSE;
#else
#ifdef WLMCHAN
			/* dynamic fb currently not supported with MCHAN */
			if (MCHAN_ACTIVE(wlc->pub)) {
				ampdu_tx_cfg->fb_override = FALSE;
				return;
			}
#endif // endif
			if (ampdu_tx->rtstxcnt >= ampdu_tx_cfg->dyn_fb_minrtscnt) {
				/* Calculate average RTS PER percentage
				 * rts per % = 100 - rts success percentage
				 */
				rtsper_avg = 100 - ((ampdu_tx->ctsrxcnt * 100) /
				             ampdu_tx->rtstxcnt);
			} else {
				rtsper_avg = 0;
			}

			/* record rtsper_avg for stats */
			ampdu_tx->rtsper_avg = rtsper_avg;

			/* If frameburst is currently enabled (fb_override = false) &&
			 * rtsper exceeds threshold, disable FB (fb_override = true).
			 * OR
			 * If frameburst is currently disabled (fb_override = true) &&
			 * rtsper falls below threshold, re-enable FB (fb_override = false).
			 */
			if ((ampdu_tx_cfg->fb_override == FALSE) &&
			    rtsper_avg >= ampdu_tx_cfg->dyn_fb_rtsper_off) {
				ampdu_tx_cfg->fb_override = TRUE;	/* frameburst = 0 */
				WL_AMPDU_TX(("%s: frameburst disabled (rtsper_avg %d%%)\n",
				            __FUNCTION__, rtsper_avg));
			} else if (ampdu_tx_cfg->fb_override &&
				rtsper_avg <= ampdu_tx_cfg->dyn_fb_rtsper_on) {
				ampdu_tx_cfg->fb_override = FALSE;	/* frameburst = 1 */
				WL_AMPDU_TX(("%s: frameburst enabled (rtsper_avg %d%%)\n",
				            __FUNCTION__, rtsper_avg));
			}

#ifdef FRAMEBURST_RTSCTS_PER_AMPDU
			/* Frameburst always ON */
			if (ampdu_tx_cfg->fb_override == TRUE) {
				/* Do not disable frame burst instead enable RTS/CTS per ampdu */
				wlc_mhf(wlc, MHF4, MHF4_RTS_INFB, TRUE, WLC_BAND_ALL);
			} else {
				/* Enable frame burst without RTS/CTS per AMPDU,
				 * RTS/CTS only for first AMPDU.
				 */
				wlc_mhf(wlc, MHF4, MHF4_RTS_INFB, FALSE, WLC_BAND_ALL);
			}
#endif // endif

		}

		/* Reset accumulated rts/cts counters */
		ampdu_tx->ctsrxcnt = 0;
		ampdu_tx->rtstxcnt = 0;
#endif /* WLAMPDU_MAC */
	} /* if (ampdu_tx->wlc->frameburst ..) */

	if (any_ba_state_pendoff) {
		/* If data fifo is blocked or phy muted due to DFS, CAC, or ..
		 * it is expected that there will not be any progress.
		 * Else, we need to trigger PSM WDS.
		 */
		if (!(ampdu_tx->wlc->block_datafifo & DATA_BLOCK_QUIET) &&
				!wlc_phy_ismuted(WLC_PI(wlc))) {
			if (ampdu_tx->txnoprog_cnt++ >= AMPDU_TXNOPROG_LIMIT) {
				/* dump mac and macx dump */
				wlc_dump_mac_fatal(wlc, PSM_FATAL_TXSTUCK);
			}
		}
	}
}

#ifdef WLAWDL
void wlc_awdl_ampdu_txeval(void *hdl, bool awdlorbss)
{
	ampdu_tx_info_t *ampdu = (ampdu_tx_info_t *)hdl;
	wlc_info_t *wlc = ampdu->wlc;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 tid;
	void *p;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!SCB_AMPDU(scb) || (awdlorbss && !BSSCFG_AWDL(wlc, scb->bsscfg)) ||
			(!awdlorbss && BSSCFG_AWDL(wlc, scb->bsscfg)))
			continue;
		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;
			if (ini->ba_state == AMPDU_TID_STATE_BA_ON) {
				/* tickle the sm and release whatever pkts we can */
				wlc_ampdu_txeval(ampdu, scb_ampdu, ini, TRUE);

				if (ini->retry_bar) {
					/* Free queued packets when system is short of memory;
					 * release 1/4 buffered and at most wl_ampdu_drain_cnt pkts.
					 */
					if (ini->retry_bar == AMPDU_R_BAR_NO_BUFFER) {
						int rel_cnt, i;

						WL_ERROR(("wl%d: wlc_ampdu_watchdog: "
							"no memory\n", wlc->pub->unit));
						rel_cnt = wlc_ampdu_pktq_plen(&scb_ampdu->txq,
							ini->tid) >> 2;
						if (wl_ampdu_drain_cnt <= 0)
							wl_ampdu_drain_cnt = AMPDU_SCB_DRAIN_CNT;
						rel_cnt = MIN(rel_cnt, wl_ampdu_drain_cnt);
						for (i = 0; i < rel_cnt; i++) {
							p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq,
								ini->tid);
							ASSERT(p != NULL);
							ASSERT(PKTPRIO(p) == ini->tid);
							PKTFREE(wlc->pub->osh, p, TRUE);
							AMPDUSCBCNTINCR(scb_ampdu->cnt.txdrop);
							WLCNTINCR(ampdu->cnt->txdrop);
						}
					}

					if (BSSCFG_AWDL(wlc, scb->bsscfg))
						wlc_awdl_update_peer_info(wlc, &scb->ea);

					wlc_ampdu_send_bar(ampdu, ini, FALSE);
				}
			}
		}
	}

}

static bool
wlc_awdl_ampdu_txeval_chanspec(wlc_info_t *wlc, scb_ampdu_tx_t *scb_ampdu)
{
	if (((BSSCFG_AWDL(wlc, scb_ampdu->scb->bsscfg)) &&
		((!wlc_awdl_tx_outside_aw_allowed(wlc) && !wlc_awdl_in_aw(wlc)) ||
		!wlc_awdl_valid_channel(wlc) || wlc_awdl_in_flow_ctrl(wlc) ||
		!wlc_awdl_in_valid_peer_chan(wlc, scb_ampdu->scb))) ||
		(AWDL_ENAB(wlc->pub) && !(BSSCFG_AWDL(wlc, scb_ampdu->scb->bsscfg)) &&
		(wf_chspec_ctlchan(WLC_BAND_PI_RADIO_CHANSPEC) !=
		wf_chspec_ctlchan(scb_ampdu->scb->bsscfg->current_bss->chanspec))))
		return FALSE;

	return TRUE;
}

#endif /* WLAWDL */

/** handle AMPDU related iovars */
static int
wlc_ampdu_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)hdl;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	int32 int_val = 0;
	int32 *ret_int_ptr = (int32 *) a;
	bool bool_val;
	int err = 0;
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	wlc = ampdu_tx->wlc;
	ASSERT(ampdu_tx == wlc->ampdu_tx);

	if (ampdu_tx->txaggr_support == FALSE) {
		WL_OID(("wl%d: %s: ampdu_tx->txaggr_support is FALSE\n",
			wlc->pub->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {
	case IOV_GVAL(IOV_AMPDU_TX):
		*ret_int_ptr = (int32)wlc->pub->_ampdu_tx;
		break;

	case IOV_SVAL(IOV_AMPDU_TX):
		return wlc_ampdu_tx_set(ampdu_tx, (bool)int_val);

	case IOV_GVAL(IOV_AMPDU_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)p;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		ampdu_tid->enable = ampdu_tx_cfg->ini_enable[ampdu_tid->tid];
		bcopy(ampdu_tid, a, sizeof(*ampdu_tid));
		break;
		}

	case IOV_SVAL(IOV_AMPDU_TID): {
		struct ampdu_tid_control *ampdu_tid = (struct ampdu_tid_control *)a;

		if (ampdu_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}
		ampdu_tx_cfg->ini_enable[ampdu_tid->tid] = ampdu_tid->enable ? TRUE : FALSE;
		break;
		}

	case IOV_GVAL(IOV_AMPDU_TX_DENSITY):
		*ret_int_ptr = (int32)ampdu_tx_cfg->mpdu_density;
		break;

	case IOV_SVAL(IOV_AMPDU_TX_DENSITY):
		if (int_val > AMPDU_MAX_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}

		if (int_val < AMPDU_DEF_MPDU_DENSITY) {
			err = BCME_RANGE;
			break;
		}
		ampdu_tx_cfg->mpdu_density = (uint8)int_val;
		break;

	case IOV_SVAL(IOV_AMPDU_SEND_ADDBA):
	{
		struct ampdu_ea_tid *ea_tid = (struct ampdu_ea_tid *)a;
		struct scb *scb;
		scb_ampdu_tx_t *scb_ampdu;

		if (!AMPDU_ENAB(wlc->pub) || (ea_tid->tid >= AMPDU_MAX_SCB_TID)) {
			err = BCME_BADARG;
			break;
		}

		if (!(scb = wlc_scbfind(wlc, bsscfg, &ea_tid->ea))) {
			err = BCME_NOTFOUND;
			break;
		}

		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		if (!scb_ampdu || !SCB_AMPDU(scb)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (scb_ampdu->ini[ea_tid->tid]) {
			err = BCME_NOTREADY;
			break;
		}

		if (!wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, ea_tid->tid, TRUE)) {
			err = BCME_ERROR;
			break;
		}

		break;
	}

	case IOV_SVAL(IOV_AMPDU_SEND_DELBA):
	{
		struct ampdu_ea_tid *ea_tid = (struct ampdu_ea_tid *)a;
		struct scb *scb;
		scb_ampdu_tx_t *scb_ampdu;

		if (!AMPDU_ENAB(wlc->pub) || (ea_tid->tid >= AMPDU_MAX_SCB_TID)) {
			err = BCME_BADARG;
			break;
		}

		if (!(scb = wlc_scbfind(wlc, bsscfg, &ea_tid->ea))) {
			err = BCME_NOTFOUND;
			break;
		}

		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		if (!scb_ampdu || !SCB_AMPDU(scb)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		wlc_ampdu_tx_send_delba(ampdu_tx, scb, ea_tid->tid,
			(ea_tid->initiator == 0 ? FALSE : TRUE),
			DOT11_RC_BAD_MECHANISM);

		break;
	}

	case IOV_GVAL(IOV_AMPDU_MANUAL_MODE):
		*ret_int_ptr = (int32)ampdu_tx_cfg->manual_mode;
		break;

	case IOV_SVAL(IOV_AMPDU_MANUAL_MODE):
		ampdu_tx_cfg->manual_mode = (uint8)int_val;
		break;

#ifdef WLOVERTHRUSTER
	case IOV_SVAL(IOV_AMPDU_TCP_ACK_RATIO):
		wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx, (uint8)int_val);
		break;
	case IOV_GVAL(IOV_AMPDU_TCP_ACK_RATIO):
		*ret_int_ptr = (uint32)ampdu_tx->tcp_ack_info.tcp_ack_ratio;
		break;
	case IOV_SVAL(IOV_AMPDU_TCP_ACK_RATIO_DEPTH):
		ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth = (uint8)int_val;
		break;
	case IOV_GVAL(IOV_AMPDU_TCP_ACK_RATIO_DEPTH):
		*ret_int_ptr = (uint32)ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth;
		break;
#endif /* WLOVERTHRUSTER */

	case IOV_GVAL(IOV_AMPDU_MPDU):
		*ret_int_ptr = (int32)ampdu_tx_cfg->max_pdu;
		break;

	case IOV_SVAL(IOV_AMPDU_MPDU):
		if ((!int_val) || (int_val > AMPDU_MAX_MPDU)) {
			err = BCME_RANGE;
			break;
		}
		ampdu_tx_cfg->max_pdu = (int8)int_val;
		scb_ampdu_update_config_all(ampdu_tx);
		break;

#if defined(WL_EXPORT_AMPDU_RETRY)
	case IOV_GVAL(IOV_AMPDU_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)p;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}

		retry_tid->retry = ampdu_tx_cfg->retry_limit_tid[retry_tid->tid];
		bcopy(retry_tid, a, sizeof(*retry_tid));
		break;
	}

	case IOV_SVAL(IOV_AMPDU_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)a;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID ||
			retry_tid->retry > AMPDU_MAX_RETRY_LIMIT) {
			err = BCME_RANGE;
			break;
		}

		ampdu_tx_cfg->retry_limit_tid[retry_tid->tid] = retry_tid->retry;
		break;
	}

	case IOV_GVAL(IOV_AMPDU_RR_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)p;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID) {
			err = BCME_BADARG;
			break;
		}

		retry_tid->retry = ampdu_tx_cfg->rr_retry_limit_tid[retry_tid->tid];
		bcopy(retry_tid, a, sizeof(*retry_tid));
		break;
	}

	case IOV_SVAL(IOV_AMPDU_RR_RETRY_LIMIT_TID):
	{
		struct ampdu_retry_tid *retry_tid = (struct ampdu_retry_tid *)a;

		if (retry_tid->tid >= AMPDU_MAX_SCB_TID ||
			retry_tid->retry > AMPDU_MAX_RETRY_LIMIT) {
			err = BCME_RANGE;
			break;
		}

		ampdu_tx_cfg->rr_retry_limit_tid[retry_tid->tid] = retry_tid->retry;
		break;
	}
#endif // endif

#ifdef AMPDU_NON_AQM
#ifdef BCMDBG
#ifdef WLC_LOW
	case IOV_GVAL(IOV_AMPDU_FFPLD):
		wlc_ffpld_show(ampdu_tx);
		*ret_int_ptr = 0;
		break;
#endif // endif
#endif /* BCMDBG */
#endif /* AMPDU_NON_AQM */

#ifdef BCMDBG
#ifdef WLAMPDU_HW
	case IOV_GVAL(IOV_AMPDU_AGGFIFO):
		*ret_int_ptr = ampdu_tx_cfg->aggfifo_depth;
		break;
	case IOV_SVAL(IOV_AMPDU_AGGFIFO): {
		uint8 depth = (uint8)int_val;
		if (depth < 8 || depth > AGGFIFO_CAP) {
			WL_ERROR(("aggfifo depth has to be in [8, %d]\n", AGGFIFO_CAP));
			return BCME_BADARG;
		}
		ampdu_tx_cfg->aggfifo_depth = depth;
	}
		break;
#endif /* WLAMPDU_HW */
#endif /* BCMDBG */
	case IOV_GVAL(IOV_AMPDUMAC):
		*ret_int_ptr = (int32)wlc->pub->_ampdumac;
		break;

	case IOV_GVAL(IOV_AMPDU_AGGMODE):
		*ret_int_ptr = (int32)ampdu_tx_cfg->ampdu_aggmode;
		break;

	case IOV_SVAL(IOV_AMPDU_AGGMODE):
		if (int_val != AMPDU_AGGMODE_AUTO &&
		    int_val != AMPDU_AGGMODE_HOST &&
		    int_val != AMPDU_AGGMODE_MAC) {
			err = BCME_RANGE;
			break;
		}
		ampdu_tx_cfg->ampdu_aggmode = (int8)int_val;
		wlc_ampdu_tx_set(ampdu_tx, wlc->pub->_ampdu_tx);
		break;

#ifdef WLMEDIA_TXFAILEVENT
	case IOV_GVAL(IOV_AMPDU_TXFAIL_EVENT):
		*ret_int_ptr = (int32)ampdu_tx_cfg->tx_fail_event;
		break;

	case IOV_SVAL(IOV_AMPDU_TXFAIL_EVENT):
		ampdu_tx_cfg->tx_fail_event = (bool)int_val;
		break;
#endif /* WLMEDIA_TXFAILEVENT */

	case IOV_GVAL(IOV_FRAMEBURST_OVR):
		*ret_int_ptr = ampdu_tx_cfg->fb_override_enable;
		break;

	case IOV_SVAL(IOV_FRAMEBURST_OVR):
		ampdu_tx_cfg->fb_override_enable = bool_val;
#ifdef FRAMEBURST_RTSCTS_PER_AMPDU
		if (!ampdu_tx_cfg->fb_override_enable) {
			/* Clear RTS/CTS per ampdu flag */
			wlc_mhf(wlc, MHF4, MHF4_RTS_INFB, FALSE, WLC_BAND_ALL);
		}
#endif /* FRAMEBURST_RTSCTS_PER_AMPDU */
		break;

#ifdef BCMDBG
	case IOV_SVAL(IOV_AMPDU_TXQ_PROFILING_START):
		/* one shot mode, stop once circular buffer is full */
		wlc_ampdu_txq_prof_enab();
		break;

	case IOV_SVAL(IOV_AMPDU_TXQ_PROFILING_DUMP):
		wlc_ampdu_txq_prof_print_histogram(-1);
		break;

	case IOV_SVAL(IOV_AMPDU_TXQ_PROFILING_SNAPSHOT): {
		int tmp = _txq_prof_enab;
		WLC_AMPDU_TXQ_PROF_ADD_ENTRY(ampdu_tx->wlc, NULL);
		wlc_ampdu_txq_prof_print_histogram(1);
		_txq_prof_enab = tmp;
	}
		break;
#endif /* BCMDBG */

#ifdef WLATF
	case IOV_GVAL(IOV_AMPDU_ATF_US):
		*ret_int_ptr = ampdu_tx_cfg->txq_time_allowance_us;
		break;

	case IOV_SVAL(IOV_AMPDU_ATF_US):
		wlc_ampdu_atf_set_default_release_time(ampdu_tx, wlc->scbstate, int_val);
		ampdu_tx_cfg->txq_time_allowance_us = int_val;
		break;

	case IOV_SVAL(IOV_AMPDU_ATF_MIN_US):
		wlc_ampdu_atf_set_default_release_mintime(ampdu_tx, wlc->scbstate, int_val);
		ampdu_tx_cfg->txq_time_min_allowance_us = int_val;
		break;
#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
	case IOV_GVAL(IOV_AMPDU_ATF_LOWAT):
		*ret_int_ptr = (int32)ampdu_tx_cfg->ampdu_atf_lowat_rel_cnt;
		break;

	case IOV_SVAL(IOV_AMPDU_ATF_LOWAT):
		ampdu_tx_cfg->ampdu_atf_lowat_rel_cnt = int_val;
		break;
#endif /* defined(BCMDBG) || defined(BCMDBG_AMPDU) */
#endif /* WLATF */

	case IOV_GVAL(IOV_AMPDU_RELEASE):
		*ret_int_ptr = (uint32)ampdu_tx_cfg->release;
		break;

	case IOV_SVAL(IOV_AMPDU_RELEASE):
		ampdu_tx_cfg->release = (int8)int_val;
		scb_ampdu_update_config_all(ampdu_tx);
		break;

	case IOV_GVAL(IOV_AMPDU_TXAGGR):
	{
		struct ampdu_aggr *txaggr = p;
		bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);
		bzero(txaggr, sizeof(*txaggr));
		txaggr->aggr_override = bsscfg_ampdu->txaggr_override;
		txaggr->enab_TID_bmap = bsscfg_ampdu->txaggr_TID_bmap;
		bcopy(txaggr, a, sizeof(*txaggr));
		break;
	}
	case IOV_SVAL(IOV_AMPDU_TXAGGR):
	{
		struct ampdu_aggr *txaggr = a;
		uint16  enable_TID_bmap =
			(txaggr->enab_TID_bmap & txaggr->conf_TID_bmap) & AMPDU_ALL_TID_BITMAP;
		uint16  disable_TID_bmap =
			((~txaggr->enab_TID_bmap) & txaggr->conf_TID_bmap) & AMPDU_ALL_TID_BITMAP;

		if (enable_TID_bmap) {
			wlc_ampdu_tx_set_bsscfg_aggr(ampdu_tx, bsscfg, ON, enable_TID_bmap);
		}
		if (disable_TID_bmap) {
			wlc_ampdu_tx_set_bsscfg_aggr(ampdu_tx, bsscfg, OFF, disable_TID_bmap);
		}
		break;
	}
#if defined(WLAMPDU_MAC) && (defined(BCMDBG) || defined(TUNE_FBOVERRIDE))
	case IOV_SVAL(IOV_DYNFB_RTSPER_ON):
		ampdu_tx_cfg->dyn_fb_rtsper_on = (uint8) int_val;
		break;
	case IOV_GVAL(IOV_DYNFB_RTSPER_ON):
		*ret_int_ptr = ampdu_tx_cfg->dyn_fb_rtsper_on;
		break;
	case IOV_SVAL(IOV_DYNFB_RTSPER_OFF):
		ampdu_tx_cfg->dyn_fb_rtsper_off = (uint8) int_val;
		break;
	case IOV_GVAL(IOV_DYNFB_RTSPER_OFF):
		*ret_int_ptr = ampdu_tx_cfg->dyn_fb_rtsper_off;
		break;
#endif // endif
	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

/** Minimal spacing between transmit MPDU's in an AMPDU, used to avoid overflow at the receiver */
void
wlc_ampdu_tx_set_mpdu_density(ampdu_tx_info_t *ampdu_tx, uint8 mpdu_density)
{
	ampdu_tx->config->mpdu_density = mpdu_density;
}

/** Limits max outstanding transmit PDU's */
void
wlc_ampdu_tx_set_ba_tx_wsize(ampdu_tx_info_t *ampdu_tx, uint8 wsize)
{
	ampdu_tx->config->ba_tx_wsize = wsize;
}

uint8
wlc_ampdu_tx_get_ba_tx_wsize(ampdu_tx_info_t *ampdu_tx)
{
	return (ampdu_tx->config->ba_tx_wsize);
}

uint8
wlc_ampdu_tx_get_ba_max_tx_wsize(ampdu_tx_info_t *ampdu_tx)
{
	return (ampdu_tx->config->ba_max_tx_wsize);
}

uint8
wlc_ampdu_get_txpkt_weight(ampdu_tx_info_t *ampdu_tx)
{
	return (ampdu_tx->config->txpkt_weight);
}

#ifdef AMPDU_NON_AQM

static void
wlc_ffpld_init(ampdu_tx_config_t *ampdu_tx_cfg)
{
	int i, j;
	wlc_fifo_info_t *fifo;

	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = (ampdu_tx_cfg->fifo_tb + j);
		fifo->ampdu_pld_size = 0;
		for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
			fifo->mcs2ampdu_table[i] = 255;
		fifo->dmaxferrate = 0;
		fifo->accum_txampdu = 0;
		fifo->prev_txfunfl = 0;
		fifo->accum_txfunfl = 0;

	}
}

/**
 * Called when the d11 core signals tx completion, and the tx completion status indicates that an
 * tx underflow condition occurred.
 *
 * Evaluate the dma transfer rate using the tx underflows as feedback.
 * If necessary, increase tx fifo preloading. If not enough,
 * decrease maximum ampdu_tx size for each mcs till underflows stop
 * Return 1 if pre-loading not active, -1 if not an underflow event,
 * 0 if pre-loading module took care of the event.
 */
static int
wlc_ffpld_check_txfunfl(wlc_info_t *wlc, int fid)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	uint32 phy_rate = MCS_RATE(AMPDU_MAX_MCS, TRUE, FALSE);
	uint32 txunfl_ratio;
	uint8  max_mpdu;
	uint32 current_ampdu_cnt = 0;
	uint16 max_pld_size;
	uint32 new_txunfl;
	wlc_fifo_info_t *fifo = (ampdu_tx->config->fifo_tb + fid);
	uint xmtfifo_sz;
	uint16 cur_txunfl;

	/* return if we got here for a different reason than underflows */
	cur_txunfl = wlc_read_shm(wlc, MACSTAT_ADDR(MCSTOFF_TXFUNFL + fid));
	new_txunfl = (uint16)(cur_txunfl - fifo->prev_txfunfl);
	if (new_txunfl == 0) {
		WL_FFPLD(("check_txunfl : TX status FRAG set but no tx underflows\n"));
		return -1;
	}
	fifo->prev_txfunfl = cur_txunfl;

	if (!ampdu_tx->config->tx_max_funl)
		return 1;

	/* check if fifo is big enough */
	xmtfifo_sz = wlc->xmtfifo_szh[fid];

	if ((TXFIFO_SIZE_UNIT * (uint32)xmtfifo_sz) <= ampdu_tx->config->ffpld_rsvd)
		return 1;

	max_pld_size = TXFIFO_SIZE_UNIT * xmtfifo_sz - ampdu_tx->config->ffpld_rsvd;
#ifdef WLCNT
#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(wlc->pub) && !AMPDU_AQM_ENAB(wlc->pub))
		ampdu_tx->cnt->txampdu = wlc_read_shm(wlc, MACSTAT_ADDR(MCSTOFF_TXAMPDU));
#endif // endif
	current_ampdu_cnt = ampdu_tx->cnt->txampdu - fifo->prev_txampdu;
#endif /* WLCNT */
	fifo->accum_txfunfl += new_txunfl;

	/* we need to wait for at least 10 underflows */
	if (fifo->accum_txfunfl < 10)
		return 0;

	WL_FFPLD(("ampdu_count %d  tx_underflows %d\n",
		current_ampdu_cnt, fifo->accum_txfunfl));

	/*
	   compute the current ratio of tx unfl per ampdu.
	   When the current ampdu count becomes too
	   big while the ratio remains small, we reset
	   the current count in order to not
	   introduce too big of a latency in detecting a
	   large amount of tx underflows later.
	*/

	txunfl_ratio = current_ampdu_cnt/fifo->accum_txfunfl;

	if (txunfl_ratio > ampdu_tx->config->tx_max_funl) {
		if (current_ampdu_cnt >= FFPLD_MAX_AMPDU_CNT) {
#ifdef WLCNT
			fifo->prev_txampdu = ampdu_tx->cnt->txampdu;
#endif // endif
			fifo->accum_txfunfl = 0;
		}
		return 0;
	}
	max_mpdu = MIN(fifo->mcs2ampdu_table[AMPDU_MAX_MCS], ampdu_tx->config->default_pdu);

	/* In case max value max_pdu is already lower than
	   the fifo depth, there is nothing more we can do.
	*/
	/* XXX : if max_mpdu happens to fit entirely inside the fifo
	 * we should assume that tx underflows are due to higher ampdu values for lower
	 * mcs than MAX_MCS. For now, just declare us done if it happens. TO BE FIXED.
	 */

	if (fifo->ampdu_pld_size >= max_mpdu * FFPLD_MPDU_SIZE) {
		WL_FFPLD(("tx fifo pld : max ampdu fits in fifo\n)"));
#ifdef WLCNT
		fifo->prev_txampdu = ampdu_tx->cnt->txampdu;
#endif // endif
		fifo->accum_txfunfl = 0;
		return 0;
	}

	if (fifo->ampdu_pld_size < max_pld_size) {

		/* increment by TX_FIFO_PLD_INC bytes */
		fifo->ampdu_pld_size += FFPLD_PLD_INCR;
		if (fifo->ampdu_pld_size > max_pld_size)
			fifo->ampdu_pld_size = max_pld_size;

		/* update scb release size */
		scb_ampdu_update_config_all(ampdu_tx);

		/*
		   compute a new dma xfer rate for max_mpdu @ max mcs.
		   This is the minimum dma rate that
		   can acheive no unferflow condition for the current mpdu size.
		*/
		/* note : we divide/multiply by 100 to avoid integer overflows */
		fifo->dmaxferrate =
		        (((phy_rate/100)*(max_mpdu*FFPLD_MPDU_SIZE-fifo->ampdu_pld_size))
		         /(max_mpdu * FFPLD_MPDU_SIZE))*100;

		WL_FFPLD(("DMA estimated transfer rate %d; pre-load size %d\n",
		          fifo->dmaxferrate, fifo->ampdu_pld_size));
	} else {

		/* decrease ampdu size */
		if (fifo->mcs2ampdu_table[AMPDU_MAX_MCS] > 1) {
			if (fifo->mcs2ampdu_table[AMPDU_MAX_MCS] == 255)
				fifo->mcs2ampdu_table[AMPDU_MAX_MCS] =
					ampdu_tx->config->default_pdu - 1;
			else
				fifo->mcs2ampdu_table[AMPDU_MAX_MCS] -= 1;

			/* recompute the table */
			wlc_ffpld_calc_mcs2ampdu_table(ampdu_tx, fid);

			/* update scb release size */
			scb_ampdu_update_config_all(ampdu_tx);
		}
	}
#ifdef WLCNT
	fifo->prev_txampdu = ampdu_tx->cnt->txampdu;
#endif // endif
	fifo->accum_txfunfl = 0;
	return 0;
} /* wlc_ffpld_check_txfunfl */

/** tx underflow related */
static void
wlc_ffpld_calc_mcs2ampdu_table(ampdu_tx_info_t *ampdu_tx, int f)
{
	int i;
	uint32 phy_rate, dma_rate, tmp;
	uint8 max_mpdu;
	wlc_fifo_info_t *fifo = (ampdu_tx->config->fifo_tb + f);

	/* recompute the dma rate */
	/* note : we divide/multiply by 100 to avoid integer overflows */
	max_mpdu = MIN(fifo->mcs2ampdu_table[AMPDU_MAX_MCS], ampdu_tx->config->default_pdu);
	phy_rate = MCS_RATE(AMPDU_MAX_MCS, TRUE, FALSE);
	dma_rate = (((phy_rate/100) * (max_mpdu*FFPLD_MPDU_SIZE-fifo->ampdu_pld_size))
	         /(max_mpdu*FFPLD_MPDU_SIZE))*100;
	fifo->dmaxferrate = dma_rate;

	/* fill up the mcs2ampdu table; do not recalc the last mcs */
	dma_rate = dma_rate >> 7;

#if defined(WLPROPRIETARY_11N_RATES)
	i = -1;
	while (TRUE) {
		i = NEXT_MCS(i); /* iterate through both standard and prop ht rates */
		if (i > WLC_11N_LAST_PROP_MCS)
			break;
#else
	for (i = 0; i < AMPDU_MAX_MCS; i++) {
#endif /* WLPROPRIETARY_11N_RATES */
		/* shifting to keep it within integer range */
		phy_rate = MCS_RATE(i, TRUE, FALSE) >> 7;
		if (phy_rate > dma_rate) {
			tmp = ((fifo->ampdu_pld_size * phy_rate) /
				((phy_rate - dma_rate) * FFPLD_MPDU_SIZE)) + 1;
			tmp = MIN(tmp, 255);
			fifo->mcs2ampdu_table[MCS2IDX(i)] = (uint8)tmp;
		}
	}

#ifdef BCMDBG
	wlc_ffpld_show(ampdu_tx);
#endif /* BCMDBG */
} /* wlc_ffpld_calc_mcs2ampdu_table */

#ifdef BCMDBG
static void
wlc_ffpld_show(ampdu_tx_info_t *ampdu_tx)
{
	int i, j;
	wlc_fifo_info_t *fifo;

	WL_ERROR(("MCS to AMPDU tables:\n"));
	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = ampdu_tx->config->fifo_tb + j;
		WL_ERROR(("  FIFO %d : Preload settings: size %d dmarate %d kbps\n",
		          j, fifo->ampdu_pld_size, fifo->dmaxferrate));
		for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
			WL_ERROR(("  %d", fifo->mcs2ampdu_table[i]));
		}
		WL_ERROR(("\n\n"));
	}
}
#endif /* BCMDBG */
#endif /* !WLAMPDU_AQM */

#if defined(DONGLEBUILD)
/** enable or disable flow of tx packets from a higher layer to this layer to prevent overflow */
static void
wlc_ampdu_txflowcontrol(wlc_info_t *wlc, scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	wlc_txq_info_t *qi = SCB_WLCIFP(scb_ampdu->scb)->qi;

	if (wlc_txflowcontrol_override_isset(wlc, qi, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL)) {
		if ((pktq_len(&scb_ampdu->txq) + ini->tx_in_transit) <=
		    wlc->pub->tunables->ampdudatahiwat) {
			wlc_txflowcontrol_override(wlc, qi, OFF, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL);
		}
	} else {
		if ((pktq_len(&scb_ampdu->txq) + ini->tx_in_transit) >=
		    wlc->pub->tunables->ampdudatahiwat) {
			wlc_txflowcontrol_override(wlc, qi, ON, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL);
		}
	}
}
#endif	/* DONGLEBUILD */

/** enable/disable txaggr_override control.
 * AUTO: txaggr operates according to per-TID per-bsscfg control()
 * OFF: turn txaggr off for all TIDs.
 * ON: Not supported and treated the same as AUTO.
 */
void
wlc_ampdu_tx_set_bsscfg_aggr_override(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg, int8 txaggr)
{
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	if (ampdu_tx->txaggr_support == FALSE) {
		/* txaggr_override should already be OFF */
		ASSERT(bsscfg_ampdu->txaggr_override == OFF);
		return;
	}

	/* txaggr_override ON would mean that tx aggregation will be allowed for all TIDs
	 * even if bsscfg_ampdu->txaggr_TID_bmap is set OFF for some TIDs.
	 * As there is no requirement of such txaggr_override ON, just treat it as AUTO.
	 */
	if (txaggr == ON) {
		txaggr = AUTO;
	}

	if (bsscfg_ampdu->txaggr_override == txaggr) {
		return;
	}

	bsscfg_ampdu->txaggr_override = txaggr;

	if (txaggr == OFF) {
		wlc_ampdu_tx_cleanup(ampdu_tx, bsscfg, AMPDU_ALL_TID_BITMAP);
	}
}

uint16
wlc_ampdu_tx_get_bsscfg_aggr(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg)
{
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	if (ampdu_tx->txaggr_support == FALSE) {
		/* txaggr should be OFF for all TIDs */
		ASSERT(bsscfg_ampdu->txaggr_TID_bmap == 0);
	}
	return (bsscfg_ampdu->txaggr_TID_bmap & AMPDU_ALL_TID_BITMAP);
}

/** Configure ampdu tx aggregation per-TID and per-bsscfg */
void
wlc_ampdu_tx_set_bsscfg_aggr(ampdu_tx_info_t *ampdu_tx, wlc_bsscfg_t *bsscfg,
	bool txaggr, uint16 conf_TID_bmap)
{
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, bsscfg);

	if (ampdu_tx->txaggr_support == FALSE) {
		/* txaggr should already be OFF for all TIDs,
		 * and do not set txaggr_TID_bmap.
		 */
		ASSERT(bsscfg_ampdu->txaggr_TID_bmap == 0);
		return;
	}

	if (txaggr == ON) {
		bsscfg_ampdu->txaggr_TID_bmap |= (conf_TID_bmap & AMPDU_ALL_TID_BITMAP);
	} else {
		uint16 stateChangedTID = bsscfg_ampdu->txaggr_TID_bmap;
		bsscfg_ampdu->txaggr_TID_bmap &= ((~conf_TID_bmap) & AMPDU_ALL_TID_BITMAP);
		stateChangedTID ^= bsscfg_ampdu->txaggr_TID_bmap;
		stateChangedTID &= AMPDU_ALL_TID_BITMAP;

		/* Override should have higher priority if not AUTO */
		if (bsscfg_ampdu->txaggr_override == AUTO && stateChangedTID) {
			wlc_ampdu_tx_cleanup(ampdu_tx, bsscfg, stateChangedTID);
		}
	}
}

/**
 * Enable A-MPDU aggregation for the specified SCB
 *
 * This function is used to enable the Tx A-MPDU aggregation path for a given SCB
 * and is called by the AMPDU common config code.
 *
 * NOTE: this code hides the difference between TxQ MUX implementaion and the
 * previous Tx data path model.
 */
void
wlc_ampdu_tx_scb_enable(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	wlc_info_t *wlc = ampdu_tx->wlc;

	/* NOTE: the TxQ MUX implementation and previous TxQ path implement the AMPDU
	 * path differently.
	 * In the previous Tx implemenation, AMPDU inserted a TxModule
	 * to process and queue ampdu packets to an SCB ampdu queue before releasing the packets
	 * onto the driver TxQ. Then as the TxQ is processed by wlc_pull_q() (formerly wlc_sendq()),
	 * WPF_AMPDU_MPDU tagged packets were processed by wlc_sendampdu().
	 * This function configures the TXMOD_AMPDU for the given SCB to enable the AMPDU path.
	 *
	 * In the TxQ MUX implementation, there is no AMPDU TxModule. Instead packets always flow
	 * onto the SCBQs of each SCB. This function enables the AMPDU Tx path by configuring the
	 * the SCBQ MUX source output function with the the ampdu-specific output function.
	 */

#if defined(TXQ_MUX)
#if defined(WLAMPDU_MAC)
	/* enable the MUX output fn for aggregation */
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		/* enable the MUX output fn for AQM aggregation */

		scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		wlc_scbq_set_output_fn(wlc->tx_scbq, scb, scb_ampdu, wlc_ampdu_output_aqm);
	}
	else
#endif /* WLAMPDU_MAC */
	{

		BCM_REFERENCE(wlc);
		WL_ERROR(("wl%d: TXQ_MUX not ready for non-aqm ampdu\n",
		          wlc->pub->unit));
	}

#else
	/* to enable AMPDU, configure the AMPDU TxModule for this SCB */
	wlc_txmod_config(wlc, scb, TXMOD_AMPDU);
#endif /* TXQ_MUX */
}

/**
 * Disable A-MPDU aggregation for the specified SCB
 *
 * This function is used to disable the Tx A-MPDU aggregation path for a given SCB
 * and is called by the AMPDU common config code.
 *
 * NOTE: this code hides the difference between TxQ MUX implementaion and the
 * previous Tx data path model.
 */
void
wlc_ampdu_tx_scb_disable(ampdu_tx_info_t *ampdu_tx, struct scb *scb)
{
	wlc_info_t *wlc = ampdu_tx->wlc;

	/* NOTE: the TxQ MUX implementation and previous TxQ path implement the AMPDU
	 * path differently.
	 * In the previous Tx implemenation, AMPDU inserted a TxModule
	 * to process and queue ampdu packets to an SCB ampdu queue before releasing the packets
	 * onto the driver TxQ. Then as the TxQ is processed by wlc_pull_q() (formerly wlc_sendq()),
	 * WPF_AMPDU_MPDU tagged packets were processed by wlc_sendampdu().
	 * This function un-configures the TXMOD_AMPDU for the given SCB to disable the AMPDU path.
	 *
	 * In the TxQ MUX implementation, there is no AMPDU TxModule. Instead packets always flow
	 * onto the SCBQs of each SCB. This function disables the AMPDU Tx path by resetting
	 * the SCBQ MUX source output function to the regular non-ampdu output function.
	 */

#if defined(TXQ_MUX)
	/* reset the MUX output fn to the non-agg fn */
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		wlc_scbq_reset_output_fn(wlc->tx_scbq, scb);
	}
#else
	/* to disable AMPDU, unconfigure the AMPDU TxModule for this SCB */
	wlc_txmod_unconfig(wlc, scb, TXMOD_AMPDU);
#endif /* TXQ_MUX */
}

/** after wlc_ampdu_agg finishes enq of pkt, must call this */
INLINE static void
wlc_ampdu_agg_complete(wlc_info_t *wlc, ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini, bool force_release)
{
	wlc_ampdu_txflowcontrol(wlc, scb_ampdu, ini);
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, force_release);
}

#if (defined(PKTC) || defined(PKTC_TX_DONGLE))
/** AMPDU aggregation function called through txmod */
static void BCMFASTPATH
wlc_ampdu_agg_pktc(void *ctx, struct scb *scb, void *p, uint prec)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)ctx;
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;
	void *n;
	uint32 lifetime = 0;
	uint32 drop_cnt = 0;
	bool amsdu_in_ampdu;
	bool force_release = FALSE;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, SCB_BSSCFG(scb));

	ASSERT(ampdu_tx);
	wlc = ampdu_tx->wlc;

#ifdef DMATXRC
	if (!DMATXRC_ENAB(wlc->pub))
#endif // endif
		ASSERT((PKTNEXT(wlc->osh, p) == NULL) || !PKTISCHAINED(p));

	tid = (uint8)PKTPRIO(p);

	/* Set packet exptime */
	if (!(WLPKTTAG(p)->flags & WLF_EXPTIME))
		lifetime = wlc->lifetime[(SCB_WME(scb) ? WME_PRIO2AC(tid) : AC_BE)];

	if ((bsscfg_ampdu->txaggr_override == OFF) ||
		(!isbitset(bsscfg_ampdu->txaggr_TID_bmap, tid)) ||
		(WLPKTTAG(p)->flags3 & WLF3_TDLS_BYPASS_AMPDU)) {
		WL_AMPDU(("%s: tx agg off (txaggr ovrd %d, TID bmap 0x%x)-- returning\n",
			__FUNCTION__,
			bsscfg_ampdu->txaggr_override,
			bsscfg_ampdu->txaggr_TID_bmap));
		goto txmod_ampdu;
	}

	ASSERT(tid < AMPDU_MAX_SCB_TID);
	if (tid >= AMPDU_MAX_SCB_TID) {
		WL_AMPDU(("%s: tid wrong -- returning\n", __FUNCTION__));
		goto txmod_ampdu;
	}

	ASSERT(SCB_AMPDU(scb));
	if (scb->flags & SCB_PENDING_FREE) {
		WL_AMPDU(("%s: scb->flags(%x) SCB_PENDING_FREE  bit set-- returning\n",
			__FUNCTION__, scb->flags));
		goto txmod_ampdu;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

#ifdef AMPDU_COMPATIBILITY
	/* For best ampdu compatibility, initialize tid 0 first if it is not initialized yet */
	if ((tid != 0) && !scb_ampdu->ini[0]) {
		if (!wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, 0, FALSE)) {
			WL_AMPDU(("%s: tid 0 ini NULL -- returning\n", __FUNCTION__));
			goto txmod_ampdu;
		}
	}
#endif /* AMPDU_COMPATIBILITY */

	/* initialize initiator on first packet; sends addba req */
	if (!(ini = scb_ampdu->ini[tid])) {
		ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, tid, FALSE);
		if (!ini) {
			WL_AMPDU(("%s: ini NULL -- returning\n", __FUNCTION__));
			goto txmod_ampdu;
		}
	}

	if ((ini->ba_state != AMPDU_TID_STATE_BA_ON) |
		(WLF2_PCB1(p) & WLF2_PCB1_NAR)) {
		goto txmod_ampdu;
	}

	if (WLF2_PCB1(p) & WLF2_PCB1_NAR) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

	amsdu_in_ampdu = ((SCB_AMSDU_IN_AMPDU(scb) != 0) &
			(SCB_AMSDU(scb) != 0) &
#ifdef WLAMSDU_TX
			(AMSDU_TX_AC_ENAB(wlc->ami, tid)) &
#endif /* WLAMSDU_TX */
#ifdef WLCNTSCB
	                  RSPEC_ISVHT(scb->scb_stats.tx_rate) &
#else
	                  FALSE &
#endif // endif
	                  TRUE);

	while (1) {
		ASSERT(PKTISCHAINED(p) || (PKTCLINK(p) == NULL));
		n = PKTCLINK(p);
		PKTSETCLINK(p, NULL);
		if (!BCMPCIEDEV_ENAB())
		{
		/* Avoid fn call if the packet is too long to be an ACK */
			if (pkttotlen(wlc->osh, p) <= TCPACKSZSDU) {
#ifdef WLOVERTHRUSTER
				if (OVERTHRUST_ENAB(wlc->pub)) {
					wlc_ampdu_tcp_ack_suppress(ampdu_tx, scb_ampdu, p, tid);
				}
#else
				if (wlc_ampdu_is_tcpack(ampdu_tx, p))
					WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
#endif // endif
			}

		}
#ifdef PROP_TXSTATUS
		/* If any suppress pkt in the chain, force ampdu release */
		if (WL_SEQ_GET_FROMDRV(WLPKTTAG(p)->seq)) {
			force_release = TRUE;
		}
#endif /* PROP_TXSTATUS */
		if (!BCMPCIEDEV_ENAB())
		{
			/* If any pkt in the chain is TCPACK, force ampdu release */
			if (ampdu_tx->wlc->tcpack_fast_tx &&
				(WLPKTTAG(p)->flags3 & WLF3_DATA_TCP_ACK))
				force_release = TRUE;
		}

		/* Queue could overflow while walking the chain */
		if (!wlc_ampdu_prec_enq(wlc, &scb_ampdu->txq, p, tid)) {
			PKTSETCLINK(p, n);
			wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, force_release);
			break;
		}
		PKTCLRCHAINED(wlc->osh, p);
		/* Last packet in chain */
		if ((n == NULL) ||
		    (wlc_pktc_sdu_prep(wlc, scb, p, n, lifetime),
		     (amsdu_in_ampdu &&
#ifdef WLOVERTHRUSTER
		      (OVERTHRUST_ENAB(wlc->pub)?(pkttotlen(wlc->osh, p) > TCPACKSZSDU):1) &&
#endif // endif
#ifdef WLAMSDU_TX
		      ((n = wlc_amsdu_pktc_agg(wlc->ami, scb, p, n, tid, lifetime)) == NULL) &&
#endif // endif
		      TRUE))) {
			wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, force_release);
			return;
		}

#ifdef DMATXRC
		if (DMATXRC_ENAB(wlc->pub)) {
			void *phdr;

			/* Check next n to prepend phdr */
			phdr = wlc_pktc_sdu_prep_phdr(wlc, scb, n, lifetime);
			if (phdr == NULL) {
				/* No phdr so leave n alone to begin next chain */
				WLCNTINCR(wlc->pub->_cnt->txnobuf);
			} else {
				/* n has prepended phdr */
				n = phdr;
			}
		}
#endif /* DMATXRC */

		/* Start a new pkt chain */
		p = n;
	}

#ifdef MACOSX
	WL_AMPDU_ERR(("wl%d: %s: txq overflow, pktq_len[%d] "
		"pktq_max[%d] ini->tx_in_transit[%d] tid[%d] prec[%d]\n",
		wlc->pub->unit, __FUNCTION__, pktq_len(&scb_ampdu->txq),
		pktq_max(&scb_ampdu->txq), ini->tx_in_transit, tid, prec));
#else
	WL_AMPDU_ERR(("wl%d: %s: txq overflow\n", wlc->pub->unit, __FUNCTION__));
#endif // endif

	FOREACH_CHAINED_PKT(p, n) {
		PKTCLRCHAINED(wlc->osh, p);
		PKTFREE(wlc->osh, p, TRUE);
		WLCIFCNTINCR(scb, txnobuf);
		drop_cnt++;
	}

	if (drop_cnt) {
#ifdef PKTQ_LOG
		struct pktq *q = &scb_ampdu->txq;
		if (q->pktqlog) {
			pktq_counters_t *prec_cnt = wlc_ampdu_pktqlog_cnt(q, tid, prec);
			WLCNTCONDADD(prec_cnt, prec_cnt->dropped, drop_cnt - 1);
		}
#endif // endif
		WLCNTADD(wlc->pub->_cnt->txnobuf, drop_cnt);
		WLCNTADD(ampdu_tx->cnt->txdrop, drop_cnt);
		AMPDUSCBCNTADD(scb_ampdu->cnt.txdrop, drop_cnt);
		WLCNTSCBADD(scb->scb_stats.tx_failures, drop_cnt);
	}

	return;

txmod_ampdu:
	FOREACH_CHAINED_PKT(p, n) {
		PKTCLRCHAINED(wlc->osh, p);
		if (n != NULL)
			wlc_pktc_sdu_prep(wlc, scb, p, n, lifetime);
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
	}
} /* wlc_ampdu_agg_pktc */

#else /* PKTC || PKTC_TX_DONGLE */

/** AMPDU aggregation function called through txmod */
static void BCMFASTPATH
wlc_ampdu_agg(void *ctx, struct scb *scb, void *p, uint prec)
{
	ampdu_tx_info_t *ampdu_tx = (ampdu_tx_info_t *)ctx;
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;
	bool force_release = FALSE;
	bsscfg_ampdu_tx_t *bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, SCB_BSSCFG(scb));

	ASSERT(ampdu_tx);
	wlc = ampdu_tx->wlc;

	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	if (tid >= AMPDU_MAX_SCB_TID) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		WL_AMPDU(("%s: tid wrong -- returning\n", __FUNCTION__));
		return;
	}

#ifdef BCMINTSUP
	if (WLPKTTAG(p)->flags & WLF_8021X) {
		WL_AMPDU(("%s: bypassing 802.1x traffic\n", __FUNCTION__));
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}
#endif /* BCMINTSUP */

	if ((bsscfg_ampdu->txaggr_override == OFF) ||
		(!isbitset(bsscfg_ampdu->txaggr_TID_bmap, tid)) ||
		(WLPKTTAG(p)->flags3 & WLF3_TDLS_BYPASS_AMPDU)) {
		WL_AMPDU(("%s: tx agg off (txaggr ovrd %d, TID bmap 0x%x)-- returning\n",
			__FUNCTION__,
			bsscfg_ampdu->txaggr_override,
			bsscfg_ampdu->txaggr_TID_bmap));
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

	ASSERT(SCB_AMPDU(scb));
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

#ifdef AMPDU_COMPATIBILITY
	/* For AMPDU compatibility, initialize tid 0 first if it is not initialized yet */
	if ((tid != 0) && !scb_ampdu->ini[0]) {
		if (!wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, 0, FALSE)) {
			SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
			WL_AMPDU(("%s: tid 0 ini NULL -- returning\n", __FUNCTION__));
			return;
		}
	}
#endif /* AMPDU_COMPATIBILITY */

	/* initialize initiator on first packet; sends addba req */
	if (!(ini = scb_ampdu->ini[tid])) {
		ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, tid, FALSE);
		if (!ini) {
			SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
			WL_AMPDU(("%s: ini NULL -- returning\n", __FUNCTION__));
			return;
		}
	}

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON) {
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, prec);
		return;
	}

	/* Avoid fn call if the packet is too long to be an ACK */
	if (PKTLEN(wlc->osh, p) <= TCPACKSZSDU) {
#ifdef WLOVERTHRUSTER
		if (OVERTHRUST_ENAB(wlc->pub)) {
			wlc_ampdu_tcp_ack_suppress(ampdu_tx, scb_ampdu, p, tid);
		}
#else
		if (wlc_ampdu_is_tcpack(ampdu_tx, p))
			WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
#endif // endif
	}

#ifdef PROP_TXSTATUS
	/* If any suppress pkt in the chain, force ampdu release */
	if (WL_SEQ_GET_FROMDRV(WLPKTTAG(p)->seq)) {
		force_release = TRUE;
	}
#endif /* PROP_TXSTATUS */

	/* If any TCPACK pkt, force ampdu release */
	if (ampdu_tx->wlc->tcpack_fast_tx && (WLPKTTAG(p)->flags3 & WLF3_DATA_TCP_ACK))
		force_release = TRUE;

	if (wlc_ampdu_prec_enq(ampdu_tx->wlc, &scb_ampdu->txq, p, tid)) {
		wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, force_release);
		return;
	}

	WL_AMPDU_ERR(("wl%d: %s: txq overflow\n", wlc->pub->unit, __FUNCTION__));

	PKTFREE(wlc->osh, p, TRUE);
	WLCNTINCR(wlc->pub->_cnt->txnobuf);
	WLCNTINCR(ampdu_tx->cnt->txdrop);
	AMPDUSCBCNTINCR(scb_ampdu->cnt.txdrop);
	WLCIFCNTINCR(scb, txnobuf);
	WLCNTSCBINCR(scb->scb_stats.tx_failures);
#ifdef WL11K
	wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, txdrop));
	wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, txfail));
#endif // endif

} /* wlc_ampdu_agg */
#endif /* PKTC || PKTC_TX_DONGLE */

#ifdef WLOVERTHRUSTER
uint
wlc_ampdu_tx_get_tcp_ack_ratio(ampdu_tx_info_t *ampdu_tx)
{
	return (uint)ampdu_tx->tcp_ack_info.tcp_ack_ratio;
}

static void
wlc_ampdu_tx_set_tcp_ack_ratio(ampdu_tx_info_t *ampdu_tx, uint8 tcp_ack_ratio)
{
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		BCM_REFERENCE(tcp_ack_ratio);
		/* XXX: For PCIe full-dongle, we are using host buffers so no access to data.
		   Only allow setting ack_ratio when not using PCIE Full dongle
		 */
		ampdu_tx->tcp_ack_info.tcp_ack_ratio = 0;
	} else
#endif /* BCMPCIEDEV */
	{
		ampdu_tx->tcp_ack_info.tcp_ack_ratio = tcp_ack_ratio;
	}
}

/** overthruster related */
static void BCMFASTPATH
wlc_ampdu_tcp_ack_suppress(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
                           void *p, uint8 tid)
{
	wlc_tcp_ack_info_t *tcp_info = &(ampdu_tx->tcp_ack_info);
	uint8 *ip_header;
	uint8 *tcp_header;
	uint32 ack_num;
	void *prev_p;
	uint16 totlen;
	uint32 ip_hdr_len;
	uint32 tcp_hdr_len;
	uint32 pktlen;
	uint16 ethtype;
	uint8 prec;
	osl_t *osh = ampdu_tx->wlc->pub->osh;

	/* Even with tcp_ack_ratio set to 0, we want to examine the packet
	 * to find TCP ACK packet in case tcpack_fast_tx is true
	 */
	if ((tcp_info->tcp_ack_ratio == 0) && !ampdu_tx->wlc->tcpack_fast_tx)
		return;

	ethtype = wlc_sdu_etype(ampdu_tx->wlc, p);
	if (ethtype != ntoh16(ETHER_TYPE_IP))
		return;

	pktlen = pkttotlen(osh, p);

	ip_header = wlc_sdu_data(ampdu_tx->wlc, p);
	ip_hdr_len = 4*(ip_header[0] & 0x0f);

	if (pktlen < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + ip_hdr_len)
		return;

	if (ip_header[9] != 0x06)
		return;

	tcp_header = ip_header + ip_hdr_len;

#ifdef WLAMPDU_PRECEDENCE
	prec = WLC_PRIO_TO_PREC(tid);
#else
	prec = tid;
#endif // endif

	if (tcp_header[13] & 0x10) {
		totlen =  (ip_header[3] & 0xff) + (ip_header[2] << 8);
		tcp_hdr_len = 4*((tcp_header[12] & 0xf0) >> 4);

		if (totlen ==  ip_hdr_len + tcp_hdr_len) {
			tcp_info->tcp_ack_total++;
			if (ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth) {
				WLPKTTAG(p)->flags3 |= WLF3_FAVORED;
#ifdef WLAMPDU_PRECEDENCE
				prec = WLC_PRIO_TO_HI_PREC(tid);
#endif // endif
			}
			if (tcp_header[13] == 0x10) {
				WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
				if (tcp_info->current_dequeued >= tcp_info->tcp_ack_ratio) {
					/* XXX We reached ack_ratio limit, don't mark the pkt
					 * and so "forget" the ack we are
					 * enqueing so it won't get
					 * suppressed the next time around
					 */
					tcp_info->current_dequeued = 0;
					return;
				}
			} else if (ampdu_tx->wlc->tcpack_fast_tx) {
				/* Here, we want to set TCP ACK flag even to SYN ACK packets,
				 * as we are not in this function to suppress any packet.
				 */
				WLPKTTAG(p)->flags3 |= WLF3_DATA_TCP_ACK;
				return;
			}
		}
	}

	/* XXX not tcp ack to consider
	 * In case tcp_ack_ratio is 0, this function may return even before coming here,
	 * but check tcp_ack_ratio again for sure.
	 */
	if (!(WLPKTTAG(p)->flags3 & WLF3_DATA_TCP_ACK) ||
		tcp_info->tcp_ack_ratio == 0) {
		return;
	}

	ack_num = (tcp_header[8] << 24) + (tcp_header[9] << 16) +
	        (tcp_header[10] << 8) +  tcp_header[11];

	if ((prev_p = pktq_ppeek_tail(&scb_ampdu->txq, prec)) &&
		(WLPKTTAG(prev_p)->flags3 & WLF3_DATA_TCP_ACK) &&
		!(WLPKTTAG(prev_p)->flags & WLF_AMSDU)) {
		uint8 *prev_ip_hdr = wlc_sdu_data(ampdu_tx->wlc, prev_p);
		uint8 *prev_tcp_hdr = prev_ip_hdr + 4*(prev_ip_hdr[0] & 0x0f);
		uint32 prev_ack_num = (prev_tcp_hdr[8] << 24) +
			(prev_tcp_hdr[9] << 16) +
			(prev_tcp_hdr[10] << 8) +  prev_tcp_hdr[11];

#ifdef PROP_TXSTATUS
		/* Don't drop suppress packets */
		if (WL_SEQ_GET_FROMDRV(WLPKTTAG(prev_p)->seq)) {
			return;
		}
#endif /* PROP_TXSTATUS */

		/* XXX is it the same dest/src IP addr, port # etc ??
		 * IPs: compare 8 bytes at IP hdrs offset 12
		 * compare tcp hdrs for 8 bytes : includes both ports and
		 * sequence number.
		 */
		if ((ack_num > prev_ack_num) &&
			!memcmp(prev_ip_hdr+12, ip_header+12, 8) &&
			!memcmp(prev_tcp_hdr, tcp_header, 4)) {
			prev_p = pktq_pdeq_tail(&scb_ampdu->txq, prec);
			PKTFREE(ampdu_tx->wlc->pub->osh, prev_p, TRUE);
			tcp_info->tcp_ack_dequeued++;
			tcp_info->current_dequeued++;
			return;
		}
	}

	/* XXX nothing worked so far, now check beginning of the queue, if no pkt were removed yet
	 * We only do this if the queue is smaller than an ampdu to avoid
	 * removing an ack that might delay the tcp_ack transmission
	 */
	if ((pktq_plen(&scb_ampdu->txq, prec) < ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth)) {
		int count = 0;
		void *previous_p = NULL;
		prev_p = pktq_ppeek(&scb_ampdu->txq, prec);

		while (prev_p && (count < ampdu_tx->tcp_ack_info.tcp_ack_ratio_depth)) {
			if ((WLPKTTAG(prev_p)->flags3 & WLF3_DATA_TCP_ACK) &&
			    !(WLPKTTAG(prev_p)->flags & WLF_AMSDU)) {
				uint8 *prev_ip_hdr = wlc_sdu_data(ampdu_tx->wlc, prev_p);
				uint8 *prev_tcp_hdr = prev_ip_hdr +
					4*(prev_ip_hdr[0] & 0x0f);
				uint32 prev_ack_num = (prev_tcp_hdr[8] << 24) +
					(prev_tcp_hdr[9] << 16) +
					(prev_tcp_hdr[10] << 8) +  prev_tcp_hdr[11];

#ifdef PROP_TXSTATUS
				/* Don't drop suppress packets */
				if (WL_SEQ_GET_FROMDRV(WLPKTTAG(prev_p)->seq)) {
					return;
				}
#endif /* PROP_TXSTATUS */
				/* is it the same dest/src IP addr, port # etc ??
				 * IPs: compare 8 bytes at IP hdrs offset 12
				 * compare tcp hdrs for 8 bytes : includes both ports and
				 * sequence number.
				 */
				if ((ack_num > prev_ack_num) &&
					(!memcmp(prev_ip_hdr+12, ip_header+12, 8)) &&
					(!memcmp(prev_tcp_hdr, tcp_header, 4))) {
					if (!previous_p) {
						prev_p = pktq_pdeq(&scb_ampdu->txq, prec);
					} else {
						prev_p = pktq_pdeq_prev(&scb_ampdu->txq,
							prec, previous_p);
					}
					if (prev_p) {
						PKTFREE(ampdu_tx->wlc->pub->osh,
							prev_p, TRUE);
						tcp_info->tcp_ack_multi_dequeued++;
						tcp_info->current_dequeued++;
					}
					return;
				}
			}
			previous_p = prev_p;
			prev_p = PKTLINK(prev_p);
			count++;
		}
	}

	tcp_info->current_dequeued = 0;
} /* wlc_ampdu_tcp_ack_suppress */

#else /* WLOVERTHRUSTER */

/** send TCP ACKs earlier to increase throughput */
static bool BCMFASTPATH
wlc_ampdu_is_tcpack(ampdu_tx_info_t *ampdu_tx, void *p)
{
	uint8 *ip_header;
	uint8 *tcp_header;
	uint16 totlen;
	uint32 ip_hdr_len;
	uint32 tcp_hdr_len;
	uint32 pktlen;
	uint16 ethtype;
	osl_t *osh = ampdu_tx->wlc->pub->osh;
	bool ret = FALSE;

	/* No reason to find TCP ACK packets */
	if (!ampdu_tx->wlc->tcpack_fast_tx)
		goto done;

	ethtype = wlc_sdu_etype(ampdu_tx->wlc, p);
	if (ethtype != ntoh16(ETHER_TYPE_IP))
		goto done;

	pktlen = pkttotlen(osh, p);

	ip_header = wlc_sdu_data(ampdu_tx->wlc, p);
	ip_hdr_len = 4*(ip_header[0] & 0x0f);

	if (pktlen < ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + ip_hdr_len)
		goto done;

	if (ip_header[9] != 0x06)
		goto done;

	tcp_header = ip_header + ip_hdr_len;

	if (tcp_header[13] & 0x10) {
		totlen =  (ip_header[3] & 0xff) + (ip_header[2] << 8);
		tcp_hdr_len = 4*((tcp_header[12] & 0xf0) >> 4);

		if (totlen ==  ip_hdr_len + tcp_hdr_len)
			ret = TRUE;
	}

done:
	return ret;
} /* wlc_ampdu_is_tcpack */

#endif /* WLOVERTHRUSTER */

/* Find basic rate for a given rate */
#define AMPDU_BASIC_RATE(band, rspec)	((band)->basic_rate[RSPEC_REFERENCE_RATE(rspec)])

#ifdef WLATF
/**
 * Air Time Fairness. Calculate number of bytes to be released in a time window at the current tx
 * rate for the maximum and minimum txq release time window.
 */
static atf_rbytes_t* BCMFASTPATH
wlc_ampdu_atf_calc_rbytes(atf_state_t *atf_state, ratespec_t rspec)
{
	ampdu_tx_info_t *ampdu_tx;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	struct scb *scb;
	wlc_info_t *wlc;
	bool gProt;
	bool nProt;
	uint ctl_rspec;
	uint ack_rspec;
	uint pkt_overhead_us;
	uint max_pdu_time_us;
	uint ampdu_rate_kbps;
	wlc_bsscfg_t *bsscfg;
	wlcband_t *band;
	uint dur;
	uint8 max_pdu;
	uint subframe_time_us;
	uint adjusted_max_pdu;
	uint flags;
	uint16 pkt_size;
#ifdef WLCNT
	atf_stats_t *atf_stats = &atf_state->atf_stats;
#endif // endif
	ampdu_tx = atf_state->ampdu_tx;
	scb_ampdu = atf_state->scb_ampdu;
	ini = atf_state->ini;
	scb = ini->scb;
	wlc = ampdu_tx->wlc;

	ASSERT(scb->bsscfg);

	bsscfg = scb->bsscfg;
	band = atf_state->band;
	dur = ampdu_tx ->config->dur;
	max_pdu = scb_ampdu->max_pdu;
	flags = WLC_AIRTIME_AMPDU;

	/* XXX  Assume Mixed mode as the cost of calculating  is too high
	 * Can change if we find an advantage to GF calculation
	 *
	 */
	flags |= (WLC_AIRTIME_MIXEDMODE);

	gProt = (band->gmode && !IS_CCK(rspec) && WLC_PROT_G_CFG_G(wlc->prot_g, bsscfg));
	nProt =	((WLC_PROT_N_CFG_N(wlc->prot_n, bsscfg) == WLC_N_PROTECTION_20IN40) &&
		RSPEC_IS40MHZ(rspec));

	if (WLC_HT_GET_AMPDU_RTS(wlc->hti) || nProt || gProt)
		flags |= (WLC_AIRTIME_RTSCTS);

	ack_rspec = AMPDU_BASIC_RATE(band, rspec);

	/* Short preamble */
	if (WLC_PROT_CFG_SHORTPREAMBLE(wlc->prot, bsscfg) &&
		(scb->flags & SCB_SHORTPREAMBLE) && IS_CCK(ack_rspec)) {
			ack_rspec |= RSPEC_SHORT_PREAMBLE;
	}

	if (gProt) {
		/*
		  * Use long preamble and long slots if Protection bit is set
		  * per Sect 9.23.2 IEEE802.11-2012
		  * IEEE Clause 16 stuff is all long slot
		  *
		  * Use 11Mbps or lower for ACKs in 2G Band
		  */
		ctl_rspec = AMPDU_BASIC_RATE(band, LEGACY_RSPEC(WLC_RATE_11M));
	} else {
		ctl_rspec = ack_rspec;
	}

	/*
	 * Short slot for ERP STAs
	 * The AP bsscfg will keep track of all sta's shortslot/longslot cap,
	 * and keep current_bss->capability up to date.
	 */
	if (BAND_5G(band->bandtype) || bsscfg->current_bss->capability & DOT11_CAP_SHORTSLOT)
			flags |= (WLC_AIRTIME_SHORTSLOT);

	pkt_overhead_us =
		wlc_airtime_pkt_overhead_us(flags, ctl_rspec, ack_rspec, bsscfg, atf_state->ac);

	/* XXX
	* Calculate the number of maxsize AMPDUS will fit in max duration
	* If less than orig max pdu, update max_pdu.
	* This tradeoff releases more small sized packets than expected
	* as the overhead is underestimated.
	* PLCP overhead not included.
	*/
	if ((SCB_AMSDU_IN_AMPDU(scb) != 0) &
		(SCB_AMSDU(scb) != 0) &
#ifdef WLAMSDU_TX
		(AMSDU_TX_AC_ENAB(wlc->ami, ini->tid)) &
#endif /* WLAMSDU_TX */
#ifdef WLCNTSCB
		RSPEC_ISVHT(scb->scb_stats.tx_rate) &
#else
		FALSE &
#endif // endif
	        TRUE)
	{
		pkt_size = 2 * ETHER_MAX_DATA;
	} else {
		pkt_size = ETHER_MAX_DATA;
	}
	subframe_time_us = wlc_airtime_payload_time_us(flags, rspec,
		(pkt_size + wlc_airtime_dot11hdrsize(scb->wsec)));

	adjusted_max_pdu = (dur - wlc_airtime_plcp_time_us(rspec, flags))/subframe_time_us;
	max_pdu = MIN(adjusted_max_pdu, max_pdu);
	AMPDU_ATF_INCRSTAT(&atf_stats->pdu, max_pdu);

	/* Max PDU time including header */
	max_pdu_time_us = (max_pdu * subframe_time_us) + pkt_overhead_us;

	ampdu_rate_kbps = ((max_pdu * pkt_size * 8000)/max_pdu_time_us);

	atf_state->rbytes_target.max = (ampdu_rate_kbps/8000) *
		atf_state->txq_time_allowance_us;
	atf_state->rbytes_target.min = (ampdu_rate_kbps/8000) *
		atf_state->txq_time_min_allowance_us;

	atf_state->last_est_rate = rspec;
	WLCNTINCR(atf_stats->cache_miss);

	return (&atf_state->rbytes_target);

} /* wlc_ampdu_atf_calc_rbytes */

static INLINE atf_rbytes_t*
wlc_ampdu_atf_rbytes(atf_state_t *atf_state)
{
	ratespec_t rspec = atf_state->rspec_action.function(atf_state->rspec_action.arg);

	/* If rspec is the same use the previously calculated result to reduce CPU overhead */
	if (rspec == atf_state->last_est_rate) {
		WLCNTINCR(atf_state->atf_stats.cache_hit);
		return (&atf_state->rbytes_target);
	} else {
		return  wlc_ampdu_atf_calc_rbytes(atf_state, rspec);
	}
	return NULL;
} /* wlc_ampdu_atf_rbytes */

static INLINE bool
wlc_ampdu_atf_holdrelease(atf_state_t *atf_state)
{
	if (atf_state->atf != 0) {
		atf_rbytes_t *rbytes = wlc_ampdu_atf_rbytes(atf_state);

		WLCNTINCR(atf_state->atf_stats.eval);

		if (atf_state->released_bytes_inflight > rbytes->max) {
			WLCNTINCR(atf_state->atf_stats.neval);
			return TRUE;
		}
	}
	return FALSE;
}

static INLINE bool
wlc_ampdu_atf_lowat_release(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini)
{
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		/* ATF adaptive watermark tested only with AQM devices only */
		return (AMPDU_ATF_ENABLED(ini) &&
			(AMPDU_ATF_STATE(ini)->released_bytes_inflight <=
			AMPDU_ATF_STATE(ini)->rbytes_target.min) &&
			(AMPDU_ATF_STATE(ini)->released_packets_inflight <=
			ampdu_tx->config->ampdu_atf_lowat_rel_cnt) &&
			(ampdu_tx->tx_intransit <=
			AMPDU_ATF_AGG_LOWAT_REL_CNT));
	} else {
		return FALSE;
	}
}

static void
wlc_ampdu_atf_reset_bytes_inflight(scb_ampdu_tid_ini_t *ini)
{
	WLCNTINCR(AMPDU_ATF_STATE(ini)->reset);
	AMPDU_ATF_STATE(ini)->released_bytes_inflight = 0;
	AMPDU_ATF_STATE(ini)->released_packets_inflight = 0;
}

static void
wlc_ampdu_atf_reset_state(scb_ampdu_tid_ini_t *ini)
{
	atf_state_t *atf_state = AMPDU_ATF_STATE(ini);
	wlc_ampdu_atf_reset_bytes_inflight(ini);
	atf_state->last_est_rate = 0;
	atf_state->last_ampdu_fid = wme_ac2fifo[atf_state->ac];
	bzero(&atf_state->rbytes_target, sizeof(atf_rbytes_t));
	wlc_ampdu_atf_ini_set_rspec_action(atf_state, atf_state->band->rspec_override);
	WLCNTINCR(atf_state->reset);
	AMPDU_ATF_CLRCNT(ini);
}

static void BCMFASTPATH
wlc_ampdu_atf_dec_bytes_inflight(scb_ampdu_tid_ini_t *ini, void *p)
{
	atf_state_t *atf_state;
	uint32 inflight;
	uint32 pktlen;
#if defined(WLCNT)
	atf_stats_t *atf_stats;
#endif // endif

	ASSERT(p);
	pktlen = WLPKTTAG(p)->pktinfo.atf.pkt_len;

	/* Pkttag has to be cleared to make sure the completed packets are cleaned up
	 * in the atf dynamic transition from on->off
	 */
	WLPKTTAG(p)->pktinfo.atf.pkt_len = 0;

	if (!AMPDU_ATF_ENABLED(ini)) {
		return;
	}

#if defined(WLCNT)
	atf_stats = AMPDU_ATF_STATS(ini);
#endif // endif
	if (!pktlen) {
		WLCNTINCR(atf_stats->reproc);
		return;
	}

	atf_state = AMPDU_ATF_STATE(ini);

	WLCNTINCR(atf_stats->proc);

	inflight = atf_state->released_bytes_inflight;

	if (inflight >= pktlen) {
		inflight -= pktlen;
		AMPDU_ATF_INCRSTAT(&atf_stats->inflight, inflight);
	} else {
		inflight = 0;
		WLCNTINCR(atf_stats->flush);
	}

	if (atf_state->released_packets_inflight) {
		atf_state->released_packets_inflight--;
	} else {
		WLCNTINCR(atf_stats->qempty);
	}

	atf_state->released_bytes_inflight = inflight;
}

#define wlc_ampdu_dec_bytes_inflight(ini, p) wlc_ampdu_atf_dec_bytes_inflight((ini), (p))

#else

#define wlc_ampdu_dec_bytes_inflight(ini, p) do {} while (0)
#define wlc_ampdu_atf_lowat_release(ampdu_tx, ini) (FALSE)
#define wlc_ampdu_atf_holdrelease(atf_state) (FALSE)

#endif /* WLATF */

/* AMPDU packet class callback */
static void BCMFASTPATH
wlc_ampdu_pkt_freed(wlc_info_t *wlc, void *pkt, uint txs)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	struct scb *scb;
	scb_ampdu_tx_t *scb_ampdu_tx;
	scb_ampdu_tid_ini_t *ini;
	bool acked = (txs & TX_STATUS_ACK_RCV) != 0;
	wlc_pkttag_t *pkttag = WLPKTTAG(pkt);
	uint16 seq = pkttag->seq;
	bool aqm = AMPDU_AQM_ENAB(wlc->pub);

	ASSERT((pkttag->flags & WLF_AMPDU_MPDU) != 0);

	scb = WLPKTTAGSCBGET(pkt);
	if (scb == NULL || !SCB_AMPDU(scb))
		return;

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	if (scb_ampdu_tx == NULL)
		return;

	ini = scb_ampdu_tx->ini[PKTPRIO(pkt)];
	if (!ini) {
		WL_AMPDU_ERR(("wl%d: ampdu_pkt_freed: NULL ini or pon state for tid %d\n",
		              wlc->pub->unit, PKTPRIO(pkt)));
		return;
	}

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON) {
		WL_AMPDU_ERR(("wl%d: ampdu_pkt_freed: ba state %d for tid %d\n",
		             wlc->pub->unit, ini->ba_state, PKTPRIO(pkt)));
		ASSERT(ini->tx_in_transit != 0);
		ini->tx_in_transit--;
#ifdef WLATF
		ASSERT(ampdu_tx->tx_intransit != 0);
		ampdu_tx->tx_intransit--;
#endif /* WLATF */
		return;
	}

#ifdef PROP_TXSTATUS
	seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */

	/* This seq # check could possibly affect the performance so keep it
	 * in a narrow scope i.e. only for legacy d11 core revs...also start_seq
	 * and max_seq may not apply to aqm anyway...
	 */
	if (!aqm &&
	    MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >
	    MODSUB_POW2(ini->max_seq, ini->start_seq, SEQNUM_MAX)) {
		WL_ERROR(("wl%d: %s: unexpected seq 0x%x, start seq 0x%x, max seq %0x, "
		          "tx_in_transit %u\n", ampdu_tx->wlc->pub->unit, __FUNCTION__,
		          seq, ini->start_seq, ini->max_seq, ini->tx_in_transit));
		return;
	}

	wlc_ampdu_dec_bytes_inflight(ini, pkt);

	ASSERT(ini->tx_in_transit != 0);
	ini->tx_in_transit--;
#ifdef WLATF
	ASSERT(ampdu_tx->tx_intransit != 0);
	ampdu_tx->tx_intransit--;
#endif /* WLATF */

	/* packet treated as regular MPDU */
	if (pkttag->flags3 & WLF3_AMPDU_REGMPDU) {
		if (acked) {
			wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu_tx, ini);
		}
		/* send bar to move the window */
		else if (!aqm) {
			uint indx = TX_SEQ_TO_INDEX(seq);
			setbit(ini->barpending, indx);
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}
		/*
		 *  send bar to move the window, if it is still within the window from
		 *  the last frame released to HW
		 */
		else if (MODSUB_POW2(ini->max_seq, seq, SEQNUM_MAX) < ini->ba_wsize) {
			uint16 bar_seq = MODINC_POW2(seq, SEQNUM_MAX);
			/* check if there is another bar with advence seq no */
			if (!ini->bar_ackpending ||
			    IS_SEQ_ADVANCED(bar_seq, ini->barpending_seq)) {
				ini->barpending_seq = bar_seq;
				wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
			}
		}

		/* Check whether the packets on tx side
		 * is less than watermark level and disable flow
		 */
		wlc_ampdu_txflowcontrol(wlc, scb_ampdu_tx, ini);
	}
	/* packet treated as MPDU in AMPDU */
	else {
		if (!acked &&
#if !defined(PROP_TXSTATUS)
			/* For NIC drivers, avoid adjusting the ini for PM STAs as
			 * some packets can be queued to PS queues, and
			 * some would get freed after the queues are full.
			 */
			!SCB_PS(ini->scb) &&
#endif // endif
			1) {
			wlc_ampdu_ini_adjust(ampdu_tx, ini, pkt);
		}
	}
}

#ifdef PROP_TXSTATUS
#define wlc_ampdu_release_count(ini, qlen, scb_ampdu) \
	MIN(((ini)->rem_window + (ini)->suppr_window), MIN((qlen), (scb_ampdu)->release))
#define wlc_ampdu_window_is_passed(count, ini) \
	((count) > ((ini)->rem_window + (ini)->suppr_window))
#else
#define wlc_ampdu_release_count(ini, qlen, scb_ampdu) \
	MIN((ini)->rem_window, MIN((qlen), (scb_ampdu)->release))
#define wlc_ampdu_window_is_passed(count, ini) \
		((count) > (ini)->rem_window)
#endif /* PROP_TXSTATUS */

#ifdef WLTAF
void * BCMFASTPATH
wlc_ampdu_get_taf_scb_info(ampdu_tx_info_t *ampdu_tx, struct scb* scb)
{
	void *scb_ampdu = NULL;
	bsscfg_ampdu_tx_t *bsscfg_ampdu;

	if (!scb || !ampdu_tx || !(SCB_AMPDU(scb)))
		goto exit;

	bsscfg_ampdu = BSSCFG_AMPDU_TX_CUBBY(ampdu_tx, SCB_BSSCFG(scb));

	/* If overrided OFF or aggregation enabled for no TID, return NULL */
	if (bsscfg_ampdu->txaggr_override == OFF || bsscfg_ampdu->txaggr_TID_bmap == 0)
		goto exit;

	scb_ampdu = (void*)SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
exit:
	return scb_ampdu;
}

void * BCMFASTPATH
wlc_ampdu_get_taf_scb_tid_info(scb_ampdu_tx_t *scb_ampdu, int tid)
{
	if (scb_ampdu) {
		scb_ampdu_tid_ini_t* ini = scb_ampdu->ini[tid];

		if (ini && ini->ba_state == AMPDU_TID_STATE_BA_ON) {
			return (void*)ini;
		}
	}
	return NULL;
}

uint16 BCMFASTPATH
wlc_ampdu_get_taf_scb_tid_pktlen(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	return (scb_ampdu && ini) ? wlc_ampdu_pktq_plen(&scb_ampdu->txq, ini->tid) : 0;
}

uint8 BCMFASTPATH
wlc_ampdu_get_taf_scb_tid_rel(scb_ampdu_tx_t *scb_ampdu)
{
	return (scb_ampdu && scb_ampdu->ampdu_tx) ? scb_ampdu->release : 0;

}

uint8 BCMFASTPATH
wlc_ampdu_get_taf_txq_fullness_pct(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{

	unsigned plen;
	unsigned pmax;

	if (!scb_ampdu || !ini) {
		return 0;
	}

	plen = wlc_ampdu_pktq_plen(&scb_ampdu->txq, ini->tid);
	pmax = pktq_pmax(&scb_ampdu->txq, ini->tid);

	return (uint8)(plen * 100 / pmax);
}

#endif /* WLTAF */

/**
 * Function which contains the smarts of when to release the pdu. Can be called from various places.
 * Returns TRUE if released else FALSE.
 */
bool BCMFASTPATH
wlc_ampdu_txeval_action(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
                        scb_ampdu_tid_ini_t *ini, bool force,
                        taf_scheduler_public_t* taf)
{
	uint16 qlen, release;
	uint16 in_transit;
	wlc_txq_info_t *qi;
	struct pktq *q;
	uint16 wlc_txq_avail;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
#ifdef WLAWDL
	wlc_info_t *wlc = ampdu_tx->wlc;
	void *p;
#endif /* WLAWDL */

	ASSERT(scb_ampdu);
	ASSERT(ini);

	if (!wlc_scb_restrict_can_txeval(ampdu_tx->wlc)) {
		return FALSE;
	}

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON)
		return FALSE;

	qlen = wlc_ampdu_pktq_plen(&scb_ampdu->txq, ini->tid);
#ifdef WLTAF
	if (taf) {
		taf->was_emptied = (qlen == 0);
	}
#endif // endif
	if (qlen == 0)
		return FALSE;
#ifdef WLAWDL
	/* XXX: this still allows for packets that need suppression but ended up
	 * just outside AW to either remain pending in AMPDU txq or leak to send_q
	 */
	/* For AWDL interface, flush packets back to host when out of AW */
	/* also for non-awdl interface not being in right channel flush it */
	if (!wlc_awdl_ampdu_txeval_chanspec(wlc, scb_ampdu)) {
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			while ((p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid)) != NULL) {
#ifdef PROP_TXSTATUS
				if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub))
					wlc_suppress_sync_fsm(wlc, scb_ampdu->scb, p, TRUE);
				wlc_process_wlhdr_txstatus(wlc,
					WLFC_CTL_PKTFLAG_WLSUPPRESS, p, FALSE);
#endif // endif
				wlc_incr_awdl_counter(wlc, AWDL_DATA_TX_SUPR);
				PKTFREE(wlc->pub->osh, p, TRUE);
			}
		}
		else {
			WL_AMPDU_TX(("wl%d.%d: txeval: wrong chanspec 0x%04x, Stop Releasing %d "
				"in_transit %d tid %d ", ampdu_tx->wlc->pub->unit,
				WLC_BSSCFG_IDX(scb_ampdu->scb->bsscfg),
				WLC_BAND_PI_RADIO_CHANSPEC, release, in_transit, ini->tid));
		}

		return FALSE;
	}
#endif /* WLAWDL */

	release = wlc_ampdu_release_count(ini, qlen, scb_ampdu);
	in_transit = ini->tx_in_transit;

	qi = SCB_WLCIFP((scb_ampdu->scb))->qi;
	q = WLC_GET_TXQ(qi);
	wlc_txq_avail = pktq_pavail(q, WLC_PRIO_TO_PREC(ini->tid));

#ifdef WLAMPDU_MAC
	/*
	 * For AQM, limit the number of packets in transit simultaneously so that other
	 * stations get a chance to transmit as well.
	 */
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
#ifdef PSPRETEND
		struct scb * scb = scb_ampdu->scb;
		bool ps_pretend_limit_transit = SCB_PS_PRETEND(scb);
#else
		const bool ps_pretend_limit_transit = FALSE;
#endif // endif

		if (IS_SEQ_ADVANCED(ini->max_seq, ini->start_seq)) {
			in_transit = MODSUB_POW2(ini->max_seq, ini->start_seq, SEQNUM_MAX);
#ifdef PROP_TXSTATUS
			if (in_transit >= ini->suppr_window) {
				in_transit -= ini->suppr_window;
			} else {
				in_transit = 0;
			}
#endif /* PROP_TXSTATUS */
		} else {
			in_transit = 0;
		}

		ASSERT(in_transit < SEQNUM_MAX/2);

#ifdef PSPRETEND
		/* scb->ps_pretend_start contains the TSF time for when ps pretend was
		 * last activated. In poor link conditions, there is a high chance that
		 * ps pretend will be re-triggered.
		 * Within a short time window following ps pretend, this code is trying to
		 * constrain the number of packets in transit and so avoid a large number of
		 * packets repetitively going between transmit and ps mode.
		 */
		if (!ps_pretend_limit_transit && SCB_PS_PRETEND_WAS_RECENT(scb)) {
			/* time elapsed in us since last pretend ps event */
			uint32 ps_pretend_time = R_REG(ampdu_tx->wlc->osh,
			                               &ampdu_tx->wlc->regs->tsf_timerlow) -
			                         scb->ps_pretend_start;

			/* arbitrary calculation, roughly we don't allow in_transit to be big
			 * when we recently have done a pretend ps. As time goes by, we gradually
			 * allow in_transit to increase. There could be some tuning to do for this.
			 * This is to prevent a large in_transit in the case where we are performing
			 * pretend ps regularly (of order every second or faster), as the time
			 * required to suppress a large number of packets in transit can cause
			 * the data path to choke.
			 * When pretend ps is currently active, we always prevent packet release.
			 */
			if (in_transit > (40 + (ps_pretend_time >> 13 /* divide 8192 */))) {
				ps_pretend_limit_transit = TRUE;
			}

			if ((ps_pretend_time >> 10 /* divide 1024 */) > 2000) {
				/* After 2 seconds (approx) elapsed since last ps pretend, clear
				 * recent flag to avoid doing this calculation
				 */
				scb->ps_pretend &= ~PS_PRETEND_RECENT;
			}
		}
#ifdef PROP_TXSTATUS
		if (SCB_TXMOD_ACTIVE(scb, TXMOD_APPS) &&
			PROP_TXSTATUS_ENAB(ampdu_tx->wlc->pub) &&
			HOST_PROPTXSTATUS_ACTIVATED(ampdu_tx->wlc)) {
			/* If TXMOD_APPS is active, and proptxstatus is enabled, force release.
			 * With proptxstatus enabled, they are meant to go to wlc_apps_ps_enq()
			 * to get their pktbufs freed in dongle, and instead stored in the host.
			 */
			ASSERT(SCB_PS(scb));
			ps_pretend_limit_transit = FALSE;
		}
#endif /* PROP_TXSTATUS */
#endif /* PSPRETEND */

		/*
		 * Unless forced, ensure that we do not have too many packets in transit
		 * for this priority.
		 *
		 * XXX Also, leave at least AMPDU_AQM_RELEASE_MAX(256) or 50% room on the common Q.
		 */

		if (!taf && !force && (
			(wlc_ampdu_atf_holdrelease(AMPDU_ATF_STATE(ini))) ||
			(in_transit > ampdu_tx->aqm_max_release[ini->tid]) ||
			(wlc_txq_avail <  MIN(AMPDU_AQM_RELEASE_MAX, (pktq_max(q) / 2))) ||
			ps_pretend_limit_transit)) {

			WL_AMPDU_TX(("wl%d: txeval: Stop Releasing %d in_transit %d tid %d "
				"wm %d rem_wdw %d qlen %d force %d txq_avail %d pps %d\n",
				ampdu_tx->wlc->pub->unit, release, in_transit,
				ini->tid, ampdu_tx_cfg->tx_rel_lowat, ini->rem_window, qlen,
				force, wlc_txq_avail, ps_pretend_limit_transit));
			AMPDUSCBCNTINCR(scb_ampdu->cnt.txnoroom);
			release = 0;
		}
	}
#endif /* WLAMPDU_MAC */

	if (wlc_txq_avail <= release) {
	    /* change decision, as we are running out of space in common queue */
#ifdef WLTAF
		if (taf) {
			WL_TAF(("%s: common queue full (%u/%u)\n", __FUNCTION__,
			        release, wlc_txq_avail));
			release = wlc_txq_avail ? wlc_txq_avail - 1 : 0;
		}
		else
#endif // endif
		{
			release = 0;
			AMPDUSCBCNTINCR(scb_ampdu->cnt.txnoroom);
		}
	}

	if (release == 0)
		return FALSE;

	/* release mpdus if any one of the following conditions are met */
	if (taf || (in_transit < ampdu_tx_cfg->tx_rel_lowat) ||
	    wlc_ampdu_atf_lowat_release(ampdu_tx, ini) ||
	    (release == MIN(scb_ampdu->release, ini->ba_wsize)) ||
	    force || AMPDU_HW_ENAB(ampdu_tx->wlc->pub) /* || AMPDU_AQM_ENAB(ampdu_tx->wlc->pub) */ )
	{

		if (!taf) {
			AMPDU_ATF_INCRSTAT(&AMPDU_ATF_STATS(ini)->inputq, qlen);
			if (in_transit < ampdu_tx_cfg->tx_rel_lowat)
				WLCNTADD(ampdu_tx->cnt->txrel_wm, release);
			if (release == scb_ampdu->release)
				WLCNTADD(ampdu_tx->cnt->txrel_size, release);

			WL_AMPDU_TX(("wl%d: wlc_ampdu_txeval: Releasing %d mpdus: in_transit %d "
				"tid %d wm %d rem_wdw %d qlen %d force %d "
				"start_seq %x, max_seq %x\n",
				ampdu_tx->wlc->pub->unit, release, in_transit,
				ini->tid, ampdu_tx_cfg->tx_rel_lowat, ini->rem_window, qlen, force,
				ini->start_seq, ini->max_seq));
			WL_NONE(("%s "MACF" rel %u\n", __FUNCTION__,
				ETHER_TO_MACF(scb_ampdu->scb->ea), release));

		}
#ifdef WLTAF
		if (taf) {
			uint16 new_qlen = wlc_ampdu_pktq_plen(&scb_ampdu->txq, ini->tid);
			if (new_qlen == 0) {
				taf->was_emptied = TRUE;
				WL_NONE(("%s "MACF" tid %u is empty\n", __FUNCTION__,
				         ETHER_TO_MACF(scb_ampdu->scb->ea), ini->tid));
			}
		}
#endif // endif

		if (wlc_scb_restrict_can_ampduq(ampdu_tx->wlc, scb_ampdu->scb,
			ini->tx_in_transit, release) == 0) {
			WL_AMPDU_TX(("wl%d: scb_restrict for "MACF"\n", ampdu_tx->wlc->pub->unit,
					ETHER_TO_MACF(scb_ampdu->scb->ea)));
			return FALSE;
		}

		wlc_ampdu_release(ampdu_tx, scb_ampdu, ini, release, wlc_txq_avail, taf);
#ifdef BCMDBG
		WLC_AMPDU_TXQ_PROF_ADD_ENTRY(ampdu_tx->wlc, scb_ampdu->scb);
#endif // endif
		return TRUE;
	}
	/* Code below this point only executes where taf is NULL */

	if ((ini->tx_in_transit == 0) && (in_transit != 0)) {
		WL_AMPDU_ERR(("wl%d.%d Cannot release: in_transit %d tid %d "
			"start_seq 0x%x barpending_seq 0x%x max_seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb_ampdu->scb)),
			in_transit, ini->tid, ini->start_seq, ini->barpending_seq, ini->max_seq));
	}

	WL_AMPDU_TX(("wl%d: wlc_ampdu_txeval: Cannot release: in_transit %d tid %d wm %d "
	             "rem_wdw %d qlen %d release %d\n",
	             ampdu_tx->wlc->pub->unit, in_transit, ini->tid, ampdu_tx_cfg->tx_rel_lowat,
	             ini->rem_window, qlen, release));

	return FALSE;
} /* wlc_ampdu_txeval_action */

static bool BCMFASTPATH
wlc_ampdu_txeval(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
                 scb_ampdu_tid_ini_t *ini, bool force)

{
#ifdef WLTAF
	struct wlc_taf_info* taf_handle = ampdu_tx->wlc->taf_handle;
	bool taf_enable = wlc_taf_enabled(taf_handle);

	if (taf_enable) {
		bool finished = wlc_taf_schedule(taf_handle, ini->tid, scb_ampdu->scb,
		                                 force);

		if (finished) {
			return TRUE;
		}
	}
#endif // endif
	return wlc_ampdu_txeval_action(ampdu_tx, scb_ampdu,
	                               ini,  force, NULL);
} /* wlc_ampdu_txeval */

/** release transmit ampdu's for the caller supplied tid/ini, as a result of a received ACK */
static void BCMFASTPATH
wlc_ampdu_release(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini, uint16 release, uint16 q_avail, taf_scheduler_public_t* taf)
{
	void* p = NULL;
	uint16 seq = 0, indx, delta;
	bool do_release;
	uint16 chunk_pkts;
	uint16 tx_in_transit = 0;
	bool pkts_remaining;
	uint16 suppr_count = 0;
	bool aqm_enab;
	bool reset_suppr_count = FALSE;
	uint pktbytes;
#ifdef WLATF
	atf_state_t *atf_state;
	atf_rbytes_t *atf_rbytes;
	bool use_atf;
	uint32 max_rbytes;
	uint32 chunk_bytes;
	uint32 atf_rbytes_min;
	uint atf_mode;
#ifdef WLATF_CNT
	atf_stats_t *atf_stats;
	uint32 npkts;
	uint32 uflow;
	uint32 oflow;
	uint32 minrel;
	uint32 reloverride;
#endif /* WLATF_CNT */
#endif /* WLATF */
#if defined(BCMPCIEDEV)
	uint16 frmbytes;
	ratespec_t cur_rspec = 0;
	/* The variables below are used in ampdu do_release cycle. */
	uint8 fl;
	uint32 dot11hdrsize = 0;
	wlc_ravg_info_t *ravg_info = NULL;
	uint16 *txpktlen_rbuf = NULL;
#endif /* BCMPCIEDEV */
	struct spktq spq;
	uint next_fid;

	BCM_REFERENCE(suppr_count);
	BCM_REFERENCE(reset_suppr_count);

	next_fid = SCB_TXMOD_NEXT_FID(ini->scb, TXMOD_AMPDU);

	if (next_fid == TXMOD_TRANSMIT) {
		pktqinit(&spq, PKTQ_LEN_MAX);
	}

	WL_AMPDU_TX(("%s start release %d\n", __FUNCTION__, release));

	if (release && AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		release = wlc_scb_restrict_can_ampduq(ampdu_tx->wlc, scb_ampdu->scb,
			ini->tx_in_transit, release);
	}

	do_release = (release != 0);
	if (!do_release) {
		return;
	}

	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		seq = SCB_SEQNUM(ini->scb, ini->tid) & (SEQNUM_MAX - 1);
		delta = MODSUB_POW2(seq, MODADD_POW2(ini->max_seq, 1, SEQNUM_MAX), SEQNUM_MAX);

		/* do not release past window (seqnum based check). There might be pkts
		 * in txq which are not marked AMPDU so will use up a seq number
		 */
		if (wlc_ampdu_window_is_passed(delta + release, ini)) {
			WL_AMPDU_ERR(("wl%d: wlc_ampdu_release: cannot release %d"
				"(out of window)\n", ampdu_tx->wlc->pub->unit, release));
			return;
		}
	} else {
		delta = 0;
	}

	chunk_pkts = 0;
	aqm_enab = AMPDU_AQM_ENAB(ampdu_tx->wlc->pub);
	pkts_remaining = FALSE;

	BCM_REFERENCE(aqm_enab);
#if defined(TINY_PKTJOIN)
	struct pktq tmp_q;
	bool tiny_join = (release > 1);

	if (TINY_PKTJOIN_ENAB(ampdu_tx->wlc->pub)) {
		if (tiny_join) {
			uint8 count = release;
			void *prev_p = NULL, *phdr;
			int plen;

			pktq_init(&tmp_q, 1, AMPDU_PKTQ_LEN);
			while (count--) {
				p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid);
				ASSERT(p != NULL);

#ifdef DMATXRC
				if (DMATXRC_ENAB(ampdu_tx->wlc->pub) &&
					(WLPKTTAG(p)->flags & WLF_PHDR)) {
					phdr = p;
					p = PKTNEXT(ampdu_tx->wlc->osh, p);
					ASSERT(p);
				} else
#endif // endif
					phdr = NULL;

				plen = PKTLEN(ampdu_tx->wlc->osh, p);
				if ((plen <= TCPACKSZSDU) && (prev_p  != NULL)) {
					void *new_pkt;

					ASSERT(PKTTAILROOM(ampdu_tx->wlc->osh, prev_p) > 512);

					new_pkt = hnd_pkt_clone(ampdu_tx->wlc->osh,
						prev_p,
						PKTLEN(ampdu_tx->wlc->osh, prev_p) + 64,
						PKTTAILROOM(ampdu_tx->wlc->osh, prev_p)-64);
					if (new_pkt) {
						PKTPULL(ampdu_tx->wlc->osh, new_pkt, 192);
						bcopy(PKTDATA(ampdu_tx->wlc->osh, p),
						   PKTDATA(ampdu_tx->wlc->osh, new_pkt),
						   plen);
						PKTSETLEN(ampdu_tx->wlc->osh,
						   new_pkt, plen);

						if (phdr) {
						   PKTSETNEXT(ampdu_tx->wlc->osh,
						      phdr, new_pkt);
						} else {
						   wlc_pkttag_info_move(ampdu_tx->wlc,
						      p, new_pkt);
						      PKTSETPRIO(new_pkt, PKTPRIO(p));
						}

						PKTSETNEXT(ampdu_tx->wlc->osh, new_pkt,
							PKTNEXT(ampdu_tx->wlc->osh, p));
						PKTSETNEXT(ampdu_tx->wlc->osh, p, NULL);

						PKTFREE(ampdu_tx->wlc->osh, p, TRUE);
						p = phdr ? phdr : new_pkt;
					}

					/* Used so clear to re-load */
					prev_p = NULL;
				}

				pktq_penq(&tmp_q, 0, phdr ? phdr : p);

				/*
				 * Traverse in case of ASMDU pkt chain
				 * and only set prev_p with enough tail room
				 * If prev_p != NULL, it was not used so don't update
				 * since there's no guarantee the pkt immediately
				 * before it has tail room.
				 */
				if (prev_p == NULL) {
					/* No tail room in phdr */
					prev_p = phdr ? PKTNEXT(ampdu_tx->wlc->osh, phdr) : p;

					while (prev_p) {
						if (PKTTAILROOM(ampdu_tx->wlc->osh, prev_p) > 512)
							break;

						prev_p = PKTNEXT(ampdu_tx->wlc->osh, prev_p);
					}
				}
			}
		}
	}
#endif /* TINY_PKTJOIN */
#ifdef WLATF
	chunk_bytes = 0;
#ifdef WLATF_CNT
	atf_stats = AMPDU_ATF_STATS(ini);
	npkts = 0;
	uflow = 0;
	oflow = 0;
	minrel = 0;
	reloverride = 0;
#endif /* WLATF_CNT */
	if (!taf && AMPDU_ATF_ENABLED(ini)) {
		atf_state = AMPDU_ATF_STATE(ini);
		atf_rbytes = wlc_ampdu_atf_rbytes(atf_state);
		use_atf  = TRUE;
		atf_rbytes_min = atf_rbytes->min;
		atf_mode = atf_state->atf;
		max_rbytes = (atf_rbytes->max > atf_state->released_bytes_inflight) ?
			(atf_rbytes->max - atf_state->released_bytes_inflight) :  0;
#if defined(BCMPCIEDEV)
		if (BCMPCIEDEV_ENAB()) {
			cur_rspec = atf_state->last_est_rate;
		}
#endif /* BCMPCIEDEV */
	} else {
	/* XXX
	 * Unfortunate that this init has to be done as gcc is not smart enough
	 * to figure out this is never used unless use_atf is TRUE
	 * it mindlessly issues a warning causing the compile to fail
	 */
		use_atf = FALSE;
		atf_state = NULL;
		atf_rbytes = NULL;
		max_rbytes = 0;
		pktbytes = 0;
		chunk_bytes = 0;
		atf_rbytes_min = 0;
		atf_mode = 0;
	}
#else
	/* q_avail not used if ATF is not compiled in, coverity pacifier */
	BCM_REFERENCE(q_avail);
#endif /* WLATF */

	BCM_REFERENCE(reset_suppr_count);
	tx_in_transit = ini->tx_in_transit;
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		if (!cur_rspec)
			cur_rspec = wlc_scb_ratesel_get_primary(ampdu_tx->wlc, ini->scb, NULL);
		fl = RAVG_PRIO2FLR(PRIOMAP(ampdu_tx->wlc), ini->tid);
		dot11hdrsize = wlc_scb_dot11hdrsize(ini->scb);
		ravg_info = TXPKTLEN_RAVG(ini->scb, fl);
		txpktlen_rbuf = TXPKTLEN_RBUF(ini->scb, fl);
	}
#endif /* BCMPCIEDEV */

	while (do_release) {
#if defined(TINY_PKTJOIN)
		if (TINY_PKTJOIN_ENAB(ampdu_tx->wlc->pub) && tiny_join)
			p = pktq_pdeq(&tmp_q, 0);
		else
		p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid);
#else
		p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid);
#endif // endif

		if (p == NULL) {
			ATF_WLCNT_INCR(uflow);
			break;
		}
		ATF_WLCNT_INCR(npkts);
		chunk_pkts++;

		WL_TRACE(("%p now released to sendampdu q\n", p));
		ASSERT(PKTPRIO(p) == ini->tid);

		WLPKTTAG(p)->flags |= WLF_AMPDU_MPDU;

#ifdef PROP_TXSTATUS
		if (WL_SEQ_GET_FROMDRV(WLPKTTAG(p)->seq)) {
			seq = WL_SEQ_GET_NUM(WLPKTTAG(p)->seq);
#ifdef PROP_TXSTATUS_SUPPR_WINDOW
			suppr_count++;
#endif // endif
		} else
#endif /* PROP_TXSTATUS */
		{
		/* assign seqnum and save in pkttag */
		seq = SCB_SEQNUM(ini->scb, ini->tid) & (SEQNUM_MAX - 1);
		SCB_SEQNUM(ini->scb, ini->tid)++;
		WLPKTTAG(p)->seq = seq;
		ini->max_seq = seq;
		reset_suppr_count = TRUE;
		}

#ifdef PROP_TXSTATUS
		if (WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wl))) {
			WL_SEQ_SET_FROMFW(WLPKTTAG(p)->seq, 1);
		}
#endif /* PROP_TXSTATUS */

		/* ASSERT(!isset(ini->ackpending, indx)); */
		/* ASSERT(ini->txretry[indx] == 0); */
		if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			indx = TX_SEQ_TO_INDEX(seq);
			setbit(ini->ackpending, indx);
		}

		ASSERT(!PKTISCHAINED(p));

		pktbytes = pkttotlen(ampdu_tx->wlc->osh, p);
		BCM_REFERENCE(pktbytes);
		WLCNTSCBINCR(scb_ampdu->ampdu_scb_stats->tx_pkts[ini->tid]);
		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_bytes[ini->tid], pktbytes);

#ifdef WLTAF
	if (taf) {
		uint32 pkt_time_units;
		uint32 pkt_tag;

		ASSERT(!use_atf);

		pkt_time_units = TAF_PKTBYTES_TO_UNITS((uint16)pktbytes, taf->pkt_rate,
		                                       taf->byte_rate);
		taf->actual_release++;
		taf->released_units += pkt_time_units;
		taf->released_bytes += (uint16)pktbytes;

		pkt_tag = TAF_UNITS_TO_PKTTAG(taf->released_units +
		                    taf->total_released_units);

		WLPKTTAG(p)->pktinfo.taf.index = taf->index;
		WLPKTTAG(p)->pktinfo.taf.units = pkt_tag;

		taf->last_release_pkttag = WLPKTTAG(p)->pktinfo.taf.units;

		if ((taf->released_units + taf->total_released_units) >=
		                                  taf->time_limit_units) {
			release = 0;
		}
	}
#endif /* WLTAF */
#ifdef WLATF
		/* The downstream TXMOD may delete and free the packet, do the ATF
		 * accounting before the TXMOD to prevent the counter from going
		 * out of sync.
		 * Similarly the pkttag update, updating the pkttag after it has
		 * been freed is to be avoided
		 */
		if (use_atf) {
			ASSERT(!taf);
			pktbytes = pkttotlen(ampdu_tx->wlc->osh, p);
			WLPKTTAG(p)->pktinfo.atf.pkt_len = (uint16)pktbytes;
			atf_state->released_bytes_inflight += pktbytes;
			chunk_bytes += pktbytes;
		}
		ampdu_tx->tx_intransit++;
#endif // endif

		/* enable packet callback for every MPDU in AMPDU */
		WLF2_PCB4_REG(p, WLF2_PCB4_AMPDU);
		ini->tx_in_transit++;
#if defined(BCMPCIEDEV)
		if (BCMPCIEDEV_ENAB()) {
			frmbytes = pkttotlen(ampdu_tx->wlc->osh, p) + dot11hdrsize;
			/* adding pktlen into the moving average buffer */
			RAVG_ADD(ravg_info, txpktlen_rbuf, frmbytes, RAVG_EXP_PKT);
		}
#endif /* BCMPCIEDEV */
		if (next_fid == TXMOD_TRANSMIT) {
			/* Enqueue p to spq */
			pktenq(&spq, p);
		} else {
			/* calls transmit module (wlc_txq_enq) : */
			SCB_TX_NEXT(TXMOD_AMPDU, ini->scb, p, WLC_PRIO_TO_PREC(ini->tid));
		}

		/* pkts_remaining is TRUE is we are below the frame release limit */
		pkts_remaining = ((ini->tx_in_transit - tx_in_transit) < release);

		/* Logic in the remainder of the scope
		 * evaluates if the next dequeue is to be done
		 */
#ifdef WLATF
		/* Apply ATF release overrides */
		if (use_atf) {
			/* Assumes SNAP header already present */
			/* XXX this is a shortcut we are taking to reduce calculation time,
			 * We also do not know the final packet sizes with out computing so the
			 * amount of intra-AMPDU frame padding cannot be determined.
			 *
			 * ATF mode 2 is not applicable to non AQM devices
			 * as the required code is not done.
			 */

			q_avail--;

			/* Out of space on outbound queue. We are done. */
			if (q_avail == 0) {
				WLCNTINCR(oflow);
				do_release = FALSE;
			} else {
				/* can_release is TRUE below ATF byte time limit */
				bool can_release = (chunk_bytes <= max_rbytes);

				/* Primary release condition
				 * Meet release up to ATF time low watermark, AQM only.
				 * Non AQM devices rely on the driver to track the BA window
				 * so we may end up releasing too many packets and thus have
				 * to add another check to make sure it does not happen,
				 * better to avoid the matter entirely in the time critical loop.
				 */
				do_release = ((aqm_enab) && (chunk_bytes <= atf_rbytes_min));

				/* Secondary release condition, there packets remaining to be
				 * released and we are below ATF time limit.
				 */
				do_release |= ((pkts_remaining) && (can_release));

				/* Tertiary release.
				 * ATF_PMODE release up to ATF time limit
				 */
				do_release |= ((can_release) && (atf_mode == WLC_AIRTIME_PMODE));
#ifdef WLCNT
				/* Update stats */

				if (!pkts_remaining && can_release) {
					/* Log minimum release if all frames have been release
					 * and we are still below the time low watermark
					 */
					if (chunk_bytes <= atf_rbytes_min) {
						WLCNTINCR(minrel);
					}
					/* Log PMODE override count */
					else if (atf_mode == WLC_AIRTIME_PMODE) {
						WLCNTINCR(reloverride);
					} /* (!pkts_remaining && can_release) */
				}
#endif // endif
			} /* (q_avail == 0) */
		} else
#endif /* WLATF */
		{
			do_release = pkts_remaining;
		}

	} /* while (do_release) */

	if (next_fid == TXMOD_TRANSMIT) {
		wlc_txq_enq_spq(ampdu_tx->wlc, ini->scb, &spq, WLC_PRIO_TO_PREC(ini->tid));
	}

	chunk_pkts = ini->tx_in_transit - tx_in_transit;
#ifdef WLATF
	if (use_atf)
		atf_state->released_packets_inflight += chunk_pkts;
#endif /* WLATF */

#ifdef PROP_TXSTATUS
	if (WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wl))) {
		if (reset_suppr_count || (ini->suppr_window <= suppr_count)) {
			ini->suppr_window = 0;
		} else {
			ini->suppr_window -= suppr_count;
		}
	}
#endif /* PROP_TXSTATUS */

	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		ini->rem_window -= (chunk_pkts + delta - suppr_count);
		if ((int16)ini->rem_window < 0) {
			WL_ERROR(("rem_window %u chunk_pkts %u delta %u suppr_count %u\n",
			          ini->rem_window, chunk_pkts, delta, suppr_count));
			ASSERT((int16)ini->rem_window >= 0);
		}
	}

#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB() &&
		chunk_pkts > 0) {
		fl = RAVG_PRIO2FLR(PRIOMAP(ampdu_tx->wlc), ini->tid);
		wlc_ravg_add_weight(ampdu_tx->wlc, ini->scb, fl, cur_rspec);
	}
#endif /* BCMPCIEDEV */

#if defined(WLATF_CNT)
	if (use_atf) {
		WLCNTADD(atf_stats->npkts, npkts);
		WLCNTADD(atf_stats->uflow, uflow);
		WLCNTADD(atf_stats->oflow, oflow);
		WLCNTADD(atf_stats->minrel, minrel);
		WLCNTADD(atf_stats->reloverride, reloverride);

		WLCNTINCR(atf_stats->ndequeue);
		if (chunk_pkts == 1) {
			WLCNTINCR(atf_stats->singleton);
		}

		if (p != NULL) {
			if (pkts_remaining)
				WLCNTINCR(atf_stats->timelimited);
			if (atf_rbytes->max > atf_state->released_bytes_inflight)
				WLCNTINCR(atf_stats->framelimited);
		}

		AMPDU_ATF_INCRSTAT(&atf_stats->rbytes, max_rbytes);
		AMPDU_ATF_INCRSTAT(&atf_stats->chunk_bytes, chunk_bytes);
		AMPDU_ATF_INCRSTAT(&atf_stats->chunk_pkts, chunk_pkts);
		AMPDU_ATF_INCRSTAT(&atf_stats->inflight, atf_state->released_bytes_inflight);
		AMPDU_ATF_INCRSTAT(&atf_stats->transit, atf_state->released_packets_inflight);
	}
#endif /* WLATF_CNT */

} /* wlc_ampdu_release */

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * returns TRUE if receives the next expected sequence
 * protects against pkt frees in the txpath by others
 */
static bool BCMFASTPATH
ampdu_is_exp_seq(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt)
{
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		return ampdu_is_exp_seq_aqm(ampdu_tx, ini, seq, suppress_pkt);
	else
#endif /* WLAMPDU_MAC */
		return ampdu_is_exp_seq_noaqm(ampdu_tx, ini, seq, suppress_pkt);

}

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

#ifdef WLAMPDU_MAC

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/** AC chip specific function */
static bool BCMFASTPATH
ampdu_is_exp_seq_aqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt)
{
	uint16 offset;
#ifdef BCMDBG
	scb_ampdu_tx_t *scb_ampdu;
	ASSERT(ini);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);
	ASSERT(scb_ampdu);
#endif // endif

#ifdef PROP_TXSTATUS
	if (suppress_pkt) {
		if (IS_SEQ_ADVANCED(seq, ini->tx_exp_seq)) {
			ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
		}
		return TRUE;
	}
#endif /* PROP_TXSTATUS */

	if (ini->tx_exp_seq != seq) {
		offset = MODSUB_POW2(seq, ini->tx_exp_seq, SEQNUM_MAX);
		WL_AMPDU_ERR(("wl%d: r0hole: tx_exp_seq 0x%x, seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));

		WL_TAF(("%s:%u\n", __FUNCTION__, __LINE__));
		/* pkts bypassed (or lagging) on way down */
		if (offset > (SEQNUM_MAX >> 1)) {
			WL_ERROR(("wl%d: rlag: tx_exp_seq 0x%x, seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));
			WLCNTINCR(ampdu_tx->cnt->txrlag);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
			return FALSE;
		}

		ini->barpending_seq = seq;
		wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		WLCNTADD(ampdu_tx->cnt->txr0hole, offset);
	}
	ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
	return TRUE;
}

static bool BCMFASTPATH
ampdu_detect_seq_hole(ampdu_tx_info_t *ampdu_tx, uint16 seq, scb_ampdu_tid_ini_t *ini)
{
	bool has_hole = FALSE;

	if (seq != ini->next_enq_seq) {
		WL_AMPDU_TX(("%s %d: seq hole detected! 0x%x 0x%x\n",
			__FUNCTION__, __LINE__, seq, ini->next_enq_seq));
		has_hole = TRUE;
	}
	ini->next_enq_seq = NEXT_SEQ(seq);
	return has_hole;
}
#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

#endif /* WLAMPDU_MAC */

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/** Non AC chip specific function */
static bool BCMFASTPATH
ampdu_is_exp_seq_noaqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini,
	uint16 seq, bool suppress_pkt)
{
	uint16 txretry, offset, indx;
	uint i;
	bool found;
#ifdef BCMDBG
	scb_ampdu_tx_t *scb_ampdu;
	ASSERT(ini);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);
	ASSERT(scb_ampdu);
#endif // endif

#ifdef PROP_TXSTATUS
	if (suppress_pkt) {
		if (IS_SEQ_ADVANCED(seq, ini->tx_exp_seq)) {
			ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
		}
		return TRUE;
	}
#endif /* PROP_TXSTATUS */

	txretry = ini->txretry[TX_SEQ_TO_INDEX(seq)];
	if (txretry == 0) {
		if (ini->tx_exp_seq != seq) {
			offset = MODSUB_POW2(seq, ini->tx_exp_seq, SEQNUM_MAX);
			WL_AMPDU_ERR(("wl%d: r0hole: tx_exp_seq 0x%x, seq 0x%x\n",
				ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));

			/* pkts bypassed (or lagging) on way down */
			if (offset > (SEQNUM_MAX >> 1)) {
				WL_ERROR(("wl%d: rlag: tx_exp_seq 0x%x, seq 0x%x\n",
					ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));
				WLCNTINCR(ampdu_tx->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
				return FALSE;
			}

			if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
				WL_ERROR(("wl%d: unexpected seq: start_seq 0x%x, seq 0x%x\n",
					ampdu_tx->wlc->pub->unit, ini->start_seq, seq));
				WLCNTINCR(ampdu_tx->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
				return FALSE;
			}

			WLCNTADD(ampdu_tx->cnt->txr0hole, offset);
			/* send bar to move the window */
			indx = TX_SEQ_TO_INDEX(ini->tx_exp_seq);
			for (i = 0; i < offset; i++, indx = NEXT_TX_INDEX(indx))
				setbit(ini->barpending, indx);
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}

		ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
	} else {
		if (ini->retry_cnt == 0 || ini->retry_seq[ini->retry_head] != seq) {
			found = FALSE;
			indx = ini->retry_head;
			for (i = 0; i < ini->retry_cnt; i++, indx = NEXT_TX_INDEX(indx)) {
				if (ini->retry_seq[indx] == seq) {
					found = TRUE;
					break;
				}
			}
			if (!found) {
				WL_ERROR(("wl%d: rnlag: tx_exp_seq 0x%x, seq 0x%x\n",
					ampdu_tx->wlc->pub->unit, ini->tx_exp_seq, seq));
				WLCNTINCR(ampdu_tx->cnt->txrlag);
				AMPDUSCBCNTINCR(scb_ampdu->cnt.txrlag);
				return FALSE;
			}
			while (ini->retry_seq[ini->retry_head] != seq) {
				WL_AMPDU_ERR(("wl%d: rnhole: retry_seq 0x%x, seq 0x%x\n",
				              ampdu_tx->wlc->pub->unit,
				              ini->retry_seq[ini->retry_head], seq));
				setbit(ini->barpending,
				       TX_SEQ_TO_INDEX(ini->retry_seq[ini->retry_head]));
				WLCNTINCR(ampdu_tx->cnt->txrnhole);
				ini->retry_seq[ini->retry_head] = 0;
				ini->retry_head = NEXT_TX_INDEX(ini->retry_head);
				ASSERT(ini->retry_cnt > 0);
				ini->retry_cnt--;
			}
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}
		ini->retry_seq[ini->retry_head] = 0;
		ini->retry_head = NEXT_TX_INDEX(ini->retry_head);
		ASSERT(ini->retry_cnt > 0);
		ini->retry_cnt--;
		ASSERT(ini->retry_cnt < ampdu_tx->config->ba_max_tx_wsize);
	}

	/* discard the frame if it is a retry and we are in pending off state */
	if (txretry && (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF)) {
		setbit(ini->barpending, TX_SEQ_TO_INDEX(seq));
		return FALSE;
	}

	if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
		ampdu_dump_ini(ini);
		ASSERT(MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) < ini->ba_wsize);
	}
	return TRUE;
}

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

#ifdef WLAMPDU_MAC

/**
 * The epoch field indicates whether the frame can be aggregated into the same A-MPDU as the
 * previous MPDU.
 */
void
wlc_ampdu_change_epoch(ampdu_tx_info_t *ampdu_tx, int fifo, int reason_code)
{
#ifdef BCMDBG
	const char *str = "Undefined";

	switch (reason_code) {
	case AMU_EPOCH_CHG_PLCP:
		str = "PLCP";
		WLCNTINCR(ampdu_tx->cnt->echgr1);
		break;
	case AMU_EPOCH_CHG_FID:
		WLCNTINCR(ampdu_tx->cnt->echgr2);
		str = "FRAMEID";
		break;
	case AMU_EPOCH_CHG_NAGG:
		WLCNTINCR(ampdu_tx->cnt->echgr3);
		str = "ampdu_tx OFF";
		break;
	case AMU_EPOCH_CHG_MPDU:
		WLCNTINCR(ampdu_tx->cnt->echgr4);
		str = "reg mpdu";
		break;
	case AMU_EPOCH_CHG_DSTTID:
		WLCNTINCR(ampdu_tx->cnt->echgr5);
		str = "DST/TID";
		break;
	case AMU_EPOCH_CHG_SEQ:
		WLCNTINCR(ampdu_tx->cnt->echgr6);
		str = "SEQ No";
		break;
	case AMU_EPOCH_CHG_TXC_UPD:
		WLCNTINCR(ampdu_tx->cnt->echgr6);
		str = "TXC UPD";
		break;
	case AMU_EPOCH_CHG_TXHDR:
		WLCNTINCR(ampdu_tx->cnt->echgr6);
		str = "TX_S_L_HDR";
		break;
	}

	WL_AMPDU_HWDBG(("wl%d: %s: fifo %d: change epoch for %s\n",
		ampdu_tx->wlc->pub->unit, __FUNCTION__, fifo, str));
#endif /* BCMDBG */
	ASSERT(fifo < NFIFO_EXT);
	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		ampdu_tx->hagg[fifo].change_epoch = TRUE;
	} else {
		ampdu_tx->hagg[fifo].epoch = (ampdu_tx->hagg[fifo].epoch) ? 0 : 1;
		WL_NONE(("wl%d: %s: fifo %d: change epoch for %s to %d\n",
		         ampdu_tx->wlc->pub->unit, __FUNCTION__, fifo, str,
		         ampdu_tx->hagg[fifo].epoch));
	}
} /* wlc_ampdu_change_epoch */

#ifdef AMPDU_NON_AQM

#if !defined(TXQ_MUX)

static uint16
wlc_ampdu_calc_maxlen(ampdu_tx_info_t *ampdu_tx, uint8 plcp0, uint plcp3, uint32 txop)
{
	uint8 is40, sgi;
	uint8 mcs = 0;
	uint16 txoplen = 0;
	uint16 maxlen = 0xffff;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;

	is40 = (plcp0 & MIMO_PLCP_40MHZ) ? 1 : 0;
	sgi = PLCP3_ISSGI(plcp3) ? 1 : 0;
	mcs = plcp0 & ~MIMO_PLCP_40MHZ;
	ASSERT(VALID_MCS(mcs));

	if (txop) {
		/* rate is in Kbps; txop is in usec
		 * ==> len = (rate * txop) / (1000 * 8)
		 */
		txoplen = (MCS_RATE(mcs, is40, sgi) >> 10) * (txop >> 3);

		maxlen = MIN((txoplen),
			(ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][is40][sgi] *
			ampdu_tx_cfg->ba_tx_wsize));
	}
	WL_AMPDU_HW(("wl%d: %s: txop %d txoplen %d maxlen %d\n",
		ampdu_tx->wlc->pub->unit, __FUNCTION__, txop, txoplen, maxlen));

	return maxlen;
}
#endif /* TXQ_MUX */

#endif /* WLAMPDU_NON_AQM */
#endif /* WLAMPDU_MAC */

#ifdef WLAMPDU_MAC

/** Called by higher layer to determine if a transmit packet is an AMPDU packet */
extern bool
wlc_ampdu_was_ampdu(ampdu_tx_info_t *ampdu_tx, int fifo)
{
	return (ampdu_tx->hagg[fifo].prev_ft != AMPDU_NONE);
}

/** Change epoch and save epoch determining params. Return updated epoch. */
uint8
wlc_ampdu_chgnsav_epoch(ampdu_tx_info_t *ampdu_tx, int fifo, int reason_code,
	struct scb *scb, uint8 tid, wlc_txh_info_t* txh_info)
{
	ampdumac_info_t *hagg;
	bool isAmpdu;
	int8* tsoHdrPtr = txh_info->tsoHdrPtr;

	/* when switch from non-aggregate ampdu to aggregate ampdu just change
	 * epoch determining parameters. do not change epoch.
	 */
	if (reason_code != AMU_EPOCH_NO_CHANGE)
		wlc_ampdu_change_epoch(ampdu_tx, fifo, reason_code);

	isAmpdu = wlc_txh_get_isAMPDU(ampdu_tx->wlc, txh_info);

	hagg = &(ampdu_tx->hagg[fifo]);
	hagg->prev_scb = scb;
	hagg->prev_tid = tid;
	hagg->prev_ft = AMPDU_NONE;
	if (isAmpdu) {
		uint16 tmp;
		tmp = ltoh16(txh_info->PhyTxControlWord0) & D11AC_PHY_TXC_FT_MASK;
		ASSERT(tmp ==  D11AC_PHY_TXC_FT_11N ||
		       tmp == D11AC_PHY_TXC_FT_11AC);
		hagg->prev_ft = (tmp == D11AC_PHY_TXC_FT_11N) ? AMPDU_11N : AMPDU_11VHT;
	}
	hagg->prev_txphyctl0 = txh_info->PhyTxControlWord0;
	hagg->prev_txphyctl1 = txh_info->PhyTxControlWord1;
	hagg->prev_plcp[0] = txh_info->plcpPtr[0];
	hagg->prev_plcp[1] = txh_info->plcpPtr[3];
#if D11CONF_GE(40)
	if (hagg->prev_ft == AMPDU_11VHT) {
		hagg->prev_plcp[3] = txh_info->plcpPtr[1];
		hagg->prev_plcp[2] = txh_info->plcpPtr[2];
		hagg->prev_plcp[4] = txh_info->plcpPtr[4];
	}
#endif /* D11CONF_GE(40) */

	hagg->prev_shdr = (tsoHdrPtr[2] & TOE_F2_TXD_HEAD_SHORT);

	WL_AMPDU_HWDBG(("%s fifo %d epoch %d\n", __FUNCTION__, fifo, hagg->epoch));

	return hagg->epoch;
} /* wlc_ampdu_chgnsav_epoch */

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * Check whether txparams have changed for ampdu tx, this would be a reason to complete the current
 * epoch.
 *
 * txparams include <frametype: 11n or 11vht>, <rate info in plcp>, <antsel>
 * Call this function *only* for aggregatable frames
 */
static bool
wlc_ampdu_epoch_params_chg(wlc_info_t *wlc, ampdumac_info_t *hagg, d11actxh_t* txh)
{
	uint8 *plcpPtr;
	d11actxh_rate_t * rateInfo;
#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
	uint16 mcl;
#endif // endif
	bool chg;
	uint8 phy_ft, frametype;

#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
	mcl = ltoh16(tx_info.MacTxControlLow);

	if (mcl & D11AC_TXC_HDR_FMT_SHORT) {
		int cache_idx;

		cache_idx = (mcl & D11AC_TXC_CACHE_IDX_MASK) >> D11AC_TXC_CACHE_IDX_SHIFT;
		rateInfo = (d11actxh_rate_t *)wlc_txc_get_rate_info_shdr(wlc->txc, cache_idx);
		plcpPtr = rateInfo[0].plcp;
	} else
#endif // endif
	{
		rateInfo = WLC_TXD_RATE_INFO_GET(txh, wlc->pub->corerev);
		plcpPtr = rateInfo[0].plcp;
	}

	/* map phy dependent frametype to independent frametype enum */
	phy_ft = ltoh16(rateInfo[0].PhyTxControlWord_0) & D11AC_PHY_TXC_FT_MASK;
	if (phy_ft == D11AC_PHY_TXC_FT_11N)
		frametype = AMPDU_11N;
	else if (phy_ft == D11AC_PHY_TXC_FT_11AC)
		frametype = AMPDU_11VHT;
	else {
		ASSERT(0);
		return TRUE;
	}

	chg = (frametype != hagg->prev_ft) ||
		(rateInfo[0].PhyTxControlWord_0 != hagg->prev_txphyctl0) ||
		(rateInfo[0].PhyTxControlWord_1 != hagg->prev_txphyctl1);
	if (chg)
		return TRUE;

#if D11CONF_GE(40)
	if (frametype == AMPDU_11VHT) {
		/* VHT frame: compare plcp0-4 */
		if (plcpPtr[0] != hagg->prev_plcp[0] ||
		    plcpPtr[1] != hagg->prev_plcp[3] ||
		    plcpPtr[2] != hagg->prev_plcp[2] ||
		    plcpPtr[3] != hagg->prev_plcp[1] ||
		    plcpPtr[4] != hagg->prev_plcp[4])
			chg = TRUE;
	} else
#endif // endif
	{
		/* HT frame: only care about plcp0 and plcp3 */
		if (plcpPtr[0] != hagg->prev_plcp[0] ||
		    plcpPtr[3] != hagg->prev_plcp[1])
			chg = TRUE;
	}

	return chg;
} /* wlc_ampdu_epoch_params_chg */

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

typedef struct ampdu_creation_params
{
	wlc_bsscfg_t *cfg;
	scb_ampdu_tx_t *scb_ampdu;
	wlc_txh_info_t* tx_info;

	scb_ampdu_tid_ini_t *ini;
	ampdu_tx_info_t *ampdu;

} ampdu_create_info_t;

static INLINE bool
wlc_txs_was_acked(wlc_info_t *wlc, tx_status_macinfo_t *status, uint16 idx)
{
	ASSERT(idx < 64);
	/* Byte 4-11 of 2nd pkg */
	if (idx < 32) {
		return (status->ack_map1 & (1 << idx)) ? TRUE : FALSE;
	} else {
		idx -= 32;
		return (status->ack_map2 & (1 << idx)) ? TRUE : FALSE;
	}
}

#ifdef BCMDBG_ASSERT

#if !(defined(TXQ_MUX) && !defined(WLAMPDU_MAC))
/* Fn is not needed if TXQ_MUX && !WLAMPDU_MAC */

/**
 * Helper function to double-check the packet header prep for AQM (AC chips only).
 * This function verifies the prepared packet header (mostly the d11actxh_cache_t sub-structure)
 * against the current settings.
 * This is intended to help catch logic errors in packet header preparation and
 * the txc header caching logic.
 */
static bool
wlc_ampdu_check_percache_info(wlc_info_t * wlc, ampdu_tx_info_t *ampdu_tx,
struct scb *scb, uint8 tid, d11actxh_t *txh)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 ampdu_dur;
	d11actxh_cache_t *cache_info;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	ASSERT(scb_ampdu != NULL);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	ini = scb_ampdu->ini[tid];

	ampdu_dur = (ampdu_tx->config->dur & D11AC_AMPDU_MAX_DUR_MASK);
	ampdu_dur |= (scb_ampdu->mpdu_density << D11AC_AMPDU_MIN_DUR_IDX_SHIFT);

	if (ltoh16(txh->PktInfo.MacTxControlLow) & D11AC_TXC_HDR_FMT_SHORT) {
		return TRUE;
	}

	cache_info = WLC_TXD_CACHE_INFO_GET(txh, wlc->pub->corerev);
	return ((cache_info->PrimeMpduMax == scb_ampdu->max_pdu) &&
		(cache_info->FallbackMpduMax == scb_ampdu->max_pdu) &&
		(cache_info->AmpduDur == htol16(ampdu_dur)) &&
		(cache_info->BAWin == ini->ba_wsize - 1) &&
		(cache_info->MaxAggLen == scb_ampdu->max_rxfactor) ?
		TRUE : FALSE);
}

#endif /* !(TXQ_MUX && !WLAMPDU_MAC) */

#endif /* BCMDBG_ASSERT */

#ifdef WL_MUPKTENG
void
wlc_ampdu_mupkteng_fill_percache_info(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid,
	d11actxh_t *txh)
{
	scb_ampdu_tx_t *scb_ampdu;
	uint16 ampdu_dur;
	d11actxh_cache_t	*cache_info;

	cache_info = WLC_TXD_CACHE_INFO_GET(txh, ampdu_tx->wlc->pub->corerev);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	BCM_REFERENCE(scb_ampdu);

	cache_info->PrimeMpduMax = 64;
	cache_info->FallbackMpduMax = 64;

	/* maximum duration is in 16 usec, putting into upper 12 bits
	 * ampdu density is in low 4 bits
	 */

	ampdu_dur = (ampdu_tx->config->dur & D11AC_AMPDU_MAX_DUR_MASK);
	ampdu_dur |= (AMPDU_DEF_MPDU_DENSITY << D11AC_AMPDU_MIN_DUR_IDX_SHIFT);
	cache_info->AmpduDur = htol16(ampdu_dur);

	/* Fill in as BAWin of recvr -1 */
	cache_info->BAWin = 63;
	cache_info->MaxAggLen = 0;
}
#endif /* WL_MUPKTENG */
void
wlc_ampdu_fill_percache_info(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid,
	d11actxh_t *txh)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 ampdu_dur;
	d11actxh_cache_t	*cache_info;

	cache_info = WLC_TXD_CACHE_INFO_GET(txh, ampdu_tx->wlc->pub->corerev);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	ASSERT(scb_ampdu != NULL);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	ini = scb_ampdu->ini[tid];

	cache_info->PrimeMpduMax = scb_ampdu->max_pdu;
	cache_info->FallbackMpduMax = scb_ampdu->max_pdu;

	/* maximum duration is in 16 usec, putting into upper 12 bits
	 * ampdu density is in low 4 bits
	 */
	ampdu_dur = (ampdu_tx->config->dur & D11AC_AMPDU_MAX_DUR_MASK);
	ampdu_dur |= (scb_ampdu->mpdu_density << D11AC_AMPDU_MIN_DUR_IDX_SHIFT);

	cache_info->AmpduDur = htol16(ampdu_dur);

	/* Fill in as BAWin of recvr -1 */
	cache_info->BAWin = ini->ba_wsize - 1;
	/* max agg len in bytes, specified as 2^X */
	cache_info->MaxAggLen = scb_ampdu->max_rxfactor;
}

#endif /* WLAMPDU_MAC */

#if !defined(TXQ_MUX)
/**
 * Called by higher layer when it wants to enqueue a PDU for transmission.
 * Returns BCME_BUSY when there is no room to queue the packet.
 */
#ifdef NEW_TXQ
int BCMFASTPATH
wlc_sendampdu(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time)
#else
int BCMFASTPATH
wlc_sendampdu(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec)
#endif /* NEW_TXQ */
{
	wlc_info_t *wlc;
	osl_t *osh;
	void *p;
	struct scb *scb;
	int err = 0;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	ASSERT(ampdu_tx);
	ASSERT(qi);
	ASSERT(pdu);

	wlc = ampdu_tx->wlc;
	osh = wlc->osh;
	p = *pdu;
	ASSERT(p);
	if (p == NULL)
		return err;
	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb != NULL);

	/* SCB setting may have changed during transition, so just discard the frame */
	if (!SCB_AMPDU(scb)) {
		WL_AMPDU(("wlc_sendampdu: entry func: scb ampdu flag null -- error\n"));
		/* ASSERT removed for 4360 as */
		/* the much longer tx pipeline (amsdu+ampdu) made it even more likely */
		/* to see this case */
		WLCNTINCR(ampdu_tx->cnt->orphan);
		PKTFREE(osh, p, TRUE);
		*pdu = NULL;
		return err;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	ASSERT(scb_ampdu);

	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);

	ini = scb_ampdu->ini[tid];

	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU(("wlc_sendampdu: bailing out for bad ini (%p)/bastate\n",
			ini));
		WLCNTINCR(ampdu_tx->cnt->orphan);
		if (ini) {
#ifdef WLTAF
			if (!wlc_taf_reset_scheduling(ampdu_tx->wlc->taf_handle, tid))
#endif // endif
			{
				wlc_ampdu_dec_bytes_inflight(ini, p);
			}
		}
#ifdef WLTAF
		else {
			wlc_taf_reset_scheduling(ampdu_tx->wlc->taf_handle, tid);
		}
#endif // endif
		PKTFREE(osh, p, TRUE);
		*pdu = NULL;
		return err;
	}

	/* Something is blocking data packets */
	if (wlc->block_datafifo) {
		WL_AMPDU(("wlc_sendampdu: datafifo blocked\n"));
		return BCME_BUSY;
	}

#if defined(WLAWDL) && defined(AWDL_FAMILY)
		if (!wlc_awdl_ampdu_txeval_chanspec(wlc, scb_ampdu)) {
			WL_AMPDU(("wlc_sendampdu: not in the right chanspec: 0x%04x, exp 0x%04x\n",
				WLC_BAND_PI_RADIO_CHANSPEC,
				scb_ampdu->scb->bsscfg->current_bss->chanspec));
			return BCME_BUSY;
		}
#endif /* WLAWDL && AWDL_FAMILY */

#ifdef NEW_TXQ
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		return _wlc_sendampdu_aqm(ampdu_tx, qi, pdu, prec, output_q, supplied_time);
	} else
#endif /* WLAMPDU_MAC */
#ifdef AMPDU_NON_AQM
	{
		return _wlc_sendampdu_noaqm(ampdu_tx, qi, pdu, prec, output_q, supplied_time);
	}
#endif /* AMPDU_NON_AQM */
#else
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		return _wlc_sendampdu_aqm(ampdu_tx, qi, pdu, prec);
	} else
#endif /* WLAMPDU_MAC */
#ifdef AMPDU_NON_AQM
	{
		return _wlc_sendampdu_noaqm(ampdu_tx, qi, pdu, prec);
	}
#endif /* AMPDU_NON_AQM */
#endif /* NEW_TXQ */
	return err;
} /* wlc_sendampdu */
#endif /* TXQ_MUX */

#if !defined(TXQ_MUX)
#ifdef AMPDU_NON_AQM

#if AMPDU_MAX_MPDU > (TXFS_WSZ_AC_BE + TXFS_WSZ_AC_BK + TXFS_WSZ_AC_VI + \
	TXFS_WSZ_AC_VO)
#define AMPDU_MAX_PKTS	AMPDU_MAX_MPDU
#else
#define AMPDU_MAX_PKTS	(TXFS_WSZ_AC_BE + TXFS_WSZ_AC_BK + TXFS_WSZ_AC_VI + TXFS_WSZ_AC_VO)
#endif // endif

/**
 * Invoked when higher layer tries to queue a transmit MPDU. Not called for AC chips unless AQM is
 * disabled. Returns BCME_BUSY when there is no room to queue the packet.
 */
static int BCMFASTPATH
#ifdef NEW_TXQ
_wlc_sendampdu_noaqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time)
#else
_wlc_sendampdu_noaqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec)
#endif /* NEW_TXQ */
{
	wlc_info_t *wlc;
	osl_t *osh;
	void *p;
#ifdef WLAMPDU_HW
	void *pkt[AGGFIFO_CAP];
#else
	void *pkt[AMPDU_MAX_PKTS];
#endif // endif
	uint8 tid, ndelim, ac;
	int err = 0, txretry = -1;
	uint8 preamble_type = WLC_GF_PREAMBLE;
	uint8 fbr_preamble_type = WLC_GF_PREAMBLE;
	uint8 rts_preamble_type = WLC_LONG_PREAMBLE;
	uint8 rts_fbr_preamble_type = WLC_LONG_PREAMBLE;

	bool rr = FALSE, fbr = FALSE, vrate_probe = FALSE;
	uint i, count = 0, fifo, npkts, nsegs, seg_cnt = 0;
	uint16 plen, len, seq, mcl, mch, indx, frameid, dma_len = 0;
	uint32 ampdu_len, maxlen = 0;
#ifdef WL11K
	uint32 pktlen = 0;
#endif // endif
	d11txh_t *txh = NULL;
	uint8 *plcp;
	struct dot11_header *h;
	struct scb *scb;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 mcs = 0;
	bool regmpdu = FALSE;
	bool use_rts = FALSE, use_cts = FALSE;
	ratespec_t rspec = 0, rspec_fallback = 0;
	ratespec_t rts_rspec = 0, rts_rspec_fallback = 0;
	struct dot11_rts_frame *rts = NULL;
	uint8 rr_retry_limit;
	wlc_fifo_info_t *f;
	bool fbr_iscck, use_mfbr = FALSE;
	uint32 txop = 0;
#ifdef WLAMPDU_MAC
	ampdumac_info_t *hagg;
	int sc; /* Side channel index */
	uint aggfifo_space = 0;
#endif // endif
	wlc_bsscfg_t *cfg;
#if !defined(NEW_TXQ)
	wlc_txh_info_t txh_info;
#endif // endif
	uint16 txburst_limit;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	bool suppress_pkt = FALSE;
#ifdef PKTC
	wlc_key_t *key;
	wlc_key_info_t key_info;
	bool slow_path = FALSE;
	uint8 txc_hit = 0;
#endif /* PKTC */
	wlc = ampdu_tx->wlc;
	osh = wlc->osh;
	p = *pdu;

#if !defined(NEW_TXQ)
	wlc_get_txh_info(wlc, p, &txh_info);
#endif // endif

	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	ac = WME_PRIO2AC(tid);
	ASSERT(ac < AC_COUNT);

	f = ampdu_tx_cfg->fifo_tb + prio2fifo[tid];

	scb = WLPKTTAGSCBGET(p);
	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[tid];
	rr_retry_limit = ampdu_tx_cfg->rr_retry_limit_tid[tid];

	ASSERT(ini->scb == scb);

#ifdef WLAMPDU_MAC
	sc = prio2fifo[tid];
	hagg = &ampdu_tx->hagg[sc];

	/* agg-fifo space */
	if (AMPDU_MAC_ENAB(wlc->pub)) {
		aggfifo_space = get_aggfifo_space(ampdu_tx, sc);
		WL_AMPDU_HWDBG(("%s: aggfifo_space %d txfifo %d\n", __FUNCTION__, aggfifo_space,
			R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
#if (defined(BCMDBG) || defined(BCMDBG_AMPDU)) && defined(WLAMPDU_UCODE)
		ampdu_tx->amdbg->schan_depth_histo[MIN(AMPDU_MAX_MPDU, aggfifo_space)]++;
#endif // endif
		if (aggfifo_space == 0) {
			return BCME_BUSY;
		}
		/* check if there are enough descriptors available */
		if (wlc->dma_avoidance_war)
			nsegs = pktsegcnt_war(osh, p);
		else
			nsegs = pktsegcnt(osh, p);
		if (TXAVAIL(wlc, sc) <= nsegs) {
			WLCNTINCR(ampdu_tx->cnt->txfifofull);
			return BCME_BUSY;
		}
	}
#endif /* WLAMPDU_MAC */

	/* compute txop once for all */
	txop = cfg->wme->edcf_txop[wme_fifo2ac[prio2fifo[tid]]];
	txburst_limit = WLC_HT_CFG_TXBURST_LIMIT(wlc->hti, cfg);

	if (txop && txburst_limit)
		txop = MIN(txop, txburst_limit);
	else if (txburst_limit)
		txop = txburst_limit;

	ampdu_len = 0;
	dma_len = 0;

#ifdef PKTC
	key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
			WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
	if (key_info.algo == CRYPTO_ALGO_OFF)
		key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt,
				SCB_BSSCFG(scb), FALSE, &key_info);

	if (key_info.algo != CRYPTO_ALGO_OFF && WLC_KEY_SW_ONLY(&key_info))
		slow_path = TRUE;
#endif /* PKTC */
	while (p) {
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		seq = WLPKTTAG(p)->seq;
#ifdef PROP_TXSTATUS
		seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */

		/* N.B.: WLF_MPDU flag indicates the MPDU is being retransmitted
		 * due to ampdu retry or d11 suppression.
		 */
		if (WLPKTTAG(p)->flags & WLF_MPDU) {
			err = wlc_prep_pdu(wlc, scb, p, &fifo);
		} else
#ifdef PKTC
		if (!slow_path &&
			(err = wlc_prep_sdu_fast(wlc, cfg, scb, p, &fifo,
			key, &key_info, &txc_hit)) == BCME_UNSUPPORTED)
#endif /* PKTC */
		{
			err = wlc_prep_sdu(wlc, scb, &p, (int*)&npkts, &fifo);
		}

		if (err) {
			if (err == BCME_BUSY) {
				WL_AMPDU_TX(("wl%d: wlc_sendampdu: prep_xdu retry; seq 0x%x\n",
					wlc->pub->unit, seq));
				WLCNTINCR(ampdu_tx->cnt->sduretry);
				*pdu = p;
				break;
			}

			/* error in the packet; reject it */
			WL_AMPDU_ERR(("wl%d: wlc_sendampdu: prep_xdu rejected; seq 0x%x\n",
				wlc->pub->unit, seq));
			WLCNTINCR(ampdu_tx->cnt->sdurejected);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.sdurejected);

			/* send bar since this may be blocking pkts in scb */
			if (ini->tx_exp_seq == seq) {
				setbit(ini->barpending, TX_SEQ_TO_INDEX(seq));
				wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
				ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
			}
			*pdu = NULL;
			break;
		}

		WL_AMPDU_TX(("wl%d: wlc_sendampdu: prep_xdu success; seq 0x%x tid %d\n",
			wlc->pub->unit, seq, tid));

		/* pkt is good to be aggregated */

		ASSERT(WLPKTTAG(p)->flags & WLF_MPDU);
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		txh = (d11txh_t *)PKTDATA(osh, p);
		plcp = (uint8 *)(txh + 1);
		h = (struct dot11_header*)(plcp + D11_PHY_HDR_LEN);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;
		indx = TX_SEQ_TO_INDEX(seq);

		/* during roam we get pkts with null a1. kill it here else
		 * does not find scb on dotxstatus and things gets out of sync
		 */
		if (ETHER_ISNULLADDR(&h->a1)) {
			WL_AMPDU_ERR(("wl%d: sendampdu; dropping frame with null a1\n",
				wlc->pub->unit));
			WLCNTINCR(ampdu_tx->cnt->txdrop);
			if (ini->tx_exp_seq == seq) {
				setbit(ini->barpending, indx);
				wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
				ini->tx_exp_seq = MODINC_POW2(seq, SEQNUM_MAX);
			}
			PKTFREE(osh, p, TRUE);
			err = BCME_ERROR;
			*pdu = NULL;
			break;
		}

#ifdef PROP_TXSTATUS
		if (WL_SEQ_GET_FROMDRV(WLPKTTAG(p)->seq)) {
			suppress_pkt = TRUE;
			/* Now the dongle owns the packet */
			WL_SEQ_SET_FROMDRV(WLPKTTAG(p)->seq, 0);
		}
#endif /* PROP_TXSTATUS */

		/* If the packet was not retrieved from HW FIFO before attempting
		 * over the air for multi-queue or p2p
		 */
		if (WLPKTTAG(p)->flags & WLF_FIFOPKT) {
			/* seq # in suppressed frames have been validated before
			 * so don't validate them again...unless there are queuing
			 * issue(s) causing them to be out-of-order.
			 */
		}
		else if (!ampdu_is_exp_seq(ampdu_tx, ini, seq, suppress_pkt)) {
			PKTFREE(osh, p, TRUE);
			err = BCME_ERROR;
			*pdu = NULL;
			break;
		}

		if (!isset(ini->ackpending, indx)) {
			WL_ERROR(("wl%d: %s: seq 0x%x\n",
			          ampdu_tx->wlc->pub->unit, __FUNCTION__, seq));
			ampdu_dump_ini(ini);
			ASSERT(isset(ini->ackpending, indx));
		}

		/* check mcl fields and test whether it can be agg'd */
		mcl = ltoh16(txh->MacTxControlLow);
		mcl &= ~TXC_AMPDU_MASK;
		fbr_iscck = !(ltoh16(txh->XtraFrameTypes) & 0x3);
		txh->PreloadSize = 0; /* always default to 0 */

		/*  Handle retry limits */
		if (AMPDU_HOST_ENAB(wlc->pub)) {

			/* first packet in ampdu */
			if (txretry == -1) {
			    txretry = ini->txretry[indx];
			    vrate_probe = (WLPKTTAG(p)->flags & WLF_VRATE_PROBE) ? TRUE : FALSE;
			}

			ASSERT(txretry == ini->txretry[indx]);

			if ((txretry == 0) ||
			    (txretry < rr_retry_limit &&
			     !(vrate_probe && ampdu_tx_cfg->probe_mpdu))) {
				rr = TRUE;
				ASSERT(!fbr);
			} else {
				/* send as regular mpdu if not a mimo frame or in ps mode
				 * or we are in pending off state
				 * make sure the fallback rate can be only HT or CCK
				 */
				fbr = TRUE;
				ASSERT(!rr);

				regmpdu = fbr_iscck;
				if (regmpdu) {
					mcl &= ~(TXC_SENDRTS | TXC_SENDCTS | TXC_LONGFRAME);
					mcl |= TXC_STARTMSDU;
					txh->MacTxControlLow = htol16(mcl);

					mch = ltoh16(txh->MacTxControlHigh);
					mch |= TXC_AMPDU_FBR;
					txh->MacTxControlHigh = htol16(mch);
					break;
				}
			}
		} /* AMPDU_HOST_ENAB */

		/* extract the length info */
		len = fbr_iscck ? WLC_GET_CCK_PLCP_LEN(txh->FragPLCPFallback)
			: WLC_GET_MIMO_PLCP_LEN(txh->FragPLCPFallback);

		if (!(WLPKTTAG(p)->flags & WLF_MIMO) ||
		    (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
		    (ini->ba_state == AMPDU_TID_STATE_BA_OFF) ||
		    SCB_PS(scb)) {
			/* clear ampdu related settings if going as regular frame */
			if ((WLPKTTAG(p)->flags & WLF_MIMO) &&
			    (SCB_PS(scb) ||
			     (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
			     (ini->ba_state == AMPDU_TID_STATE_BA_OFF))) {
				WLC_CLR_MIMO_PLCP_AMPDU(plcp);
				WLC_SET_MIMO_PLCP_LEN(plcp, len);
			}
			regmpdu = TRUE;
			mcl &= ~(TXC_SENDRTS | TXC_SENDCTS | TXC_LONGFRAME);
			txh->MacTxControlLow = htol16(mcl);
			break;
		}

		/* pkt is good to be aggregated */

		if (ini->txretry[indx]) {
			WLCNTINCR(ampdu_tx->cnt->u0.txretry_mpdu);
		}

		/* retrieve null delimiter count */
		ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
		if (wlc->dma_avoidance_war)
			seg_cnt += pktsegcnt_war(osh, p);
		else
			seg_cnt += pktsegcnt(osh, p);

		if (!WL_AMPDU_HW_ON())
			WL_AMPDU_TX(("wl%d: wlc_sendampdu: mpdu %d plcp_len %d\n",
				wlc->pub->unit, count, len));

#ifdef WLAMPDU_MAC
		/*
		 * aggregateable mpdu. For ucode/hw agg,
		 * test whether need to break or change the epoch
		 */
		if (AMPDU_MAC_ENAB(wlc->pub)) {
			uint8 max_pdu;

			ASSERT((uint)sc == fifo);

			/* check if link or tid has changed */
			if (scb != hagg->prev_scb || tid != hagg->prev_tid) {
				hagg->prev_scb = scb;
				hagg->prev_tid = tid;
				wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_DSTTID);
			}

			/* mark every MPDU's plcp to indicate ampdu */
			WLC_SET_MIMO_PLCP_AMPDU(plcp);

			/* rate/sgi/bw/etc phy info */
			if (plcp[0] != hagg->prev_plcp[0] ||
			    (plcp[3] & 0xf0) != hagg->prev_plcp[1]) {
				hagg->prev_plcp[0] = plcp[0];
				hagg->prev_plcp[1] = plcp[3] & 0xf0;
				wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_PLCP);
			}
			/* frameid -- force change by the rate selection algo */
			frameid = ltoh16(txh->TxFrameID);
			if (frameid & TXFID_RATE_PROBE_MASK) {
				wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_FID);
				wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid, TRUE, 0, ac);
			}

			/* Set bits 9:10 to any nonzero value to indicate to
			 * ucode that this packet can be aggregated
			 */
			mcl |= (TXC_AMPDU_FIRST << TXC_AMPDU_SHIFT);
			/* set sequence number in tx-desc */
			txh->AmpduSeqCtl = htol16(h->seq);

			/*
			 * compute aggregation parameters
			 */
			/* limit on number of mpdus per ampdu */
			mcs = plcp[0] & ~MIMO_PLCP_40MHZ;
			max_pdu = scb_ampdu->max_pdu;

			/* MaxAggSize in duration(usec) and bytes for hw/ucode agg, respectively */
			if (AMPDU_HW_ENAB(wlc->pub)) {
				/* use duration limit in usec */
				uint32 maxdur;
				maxdur = ampdu_tx_cfg->dur ?
					(uint32)ampdu_tx_cfg->dur : 0xffff;
				if (txop && (txop < maxdur))
					maxdur = txop;
				if ((maxdur & 0xffff0000) != 0)
					maxdur = 0xffff;
				txh->u1.MaxAggDur = htol16(maxdur & 0xffff);
				ASSERT(txh->u1.MaxAggDur != 0);
				/* hack for now :
				 * should compute from xmtfifo_sz[fifo] * 256 / max_bytes_per_mpdu
				 * e.g. fifo1: floor(255 * 256, 1650)
				 */
				txh->u2.s1.MaxRNum =
					MIN((uint8)htol16(wlc->xmtfifo_frmmaxh[fifo]),
					ini->ba_wsize);
				ASSERT(txh->u2.s1.MaxRNum > 0);
				txh->u2.s1.MaxAggBytes = scb_ampdu->max_rxfactor;
			} else {
				/* HW PR#37644 WAR : stop aggregating if mpdu
				 * count above a threshold for current rate
				 */
				if (MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) {
					uint16 preloadsize = f->ampdu_pld_size;
					if (D11REV_IS(wlc->pub->corerev, 29) ||
					    D11REV_GE(wlc->pub->corerev, 31))
						preloadsize >>= 2;
					txh->PreloadSize = htol16(preloadsize);
					if (max_pdu > f->mcs2ampdu_table[MCS2IDX(mcs)])
						max_pdu = f->mcs2ampdu_table[MCS2IDX(mcs)];
				}
				txh->u1.MaxAggLen = htol16(wlc_ampdu_calc_maxlen(ampdu_tx,
					plcp[0], plcp[3], txop));
				if (!fbr_iscck)
					txh->u2.MaxAggLen_FBR =
						htol16(wlc_ampdu_calc_maxlen(ampdu_tx,
						txh->FragPLCPFallback[0], txh->FragPLCPFallback[3],
						txop));
				else
					txh->u2.MaxAggLen_FBR = 0;
			}

			txh->MaxNMpdus = htol16((max_pdu << 8) | max_pdu);
			/* retry limits - overload MModeFbrLen */
			if (WLPKTTAG(p)->flags & WLF_VRATE_PROBE) {
				txh->MModeFbrLen = htol16((1 << 8) |
					(ampdu_tx_cfg->retry_limit_tid[tid] & 0xff));
				txh->MaxNMpdus = htol16((max_pdu << 8) |
					(ampdu_tx_cfg->probe_mpdu));
			} else {
				txh->MModeFbrLen = htol16(((rr_retry_limit & 0xff) << 8) |
					(ampdu_tx_cfg->retry_limit_tid[tid] & 0xff));
			}

			WL_AMPDU_HW(("wl%d: %s len %d MaxNMpdus 0x%x null delim %d MinMBytes %d "
				     "MaxAggDur %d MaxRNum %d rr_lmnt 0x%x\n", wlc->pub->unit,
				     __FUNCTION__, len, txh->MaxNMpdus, ndelim, txh->MinMBytes,
				     txh->u1.MaxAggDur, txh->u2.s1.MaxRNum, txh->MModeFbrLen));

			/* Ucode agg:
			 * length to be written into side channel includes:
			 * leading delimiter, mac hdr to fcs, padding, null delimiter(s)
			 * i.e. (len + 4 + 4*num_paddelims + padbytes)
			 * padbytes is num required to round to 32 bits
			 */
			if (AMPDU_UCODE_ENAB(wlc->pub)) {
				if (ndelim) {
					len = ROUNDUP(len, 4);
					len += (ndelim  + 1) * AMPDU_DELIMITER_LEN;
				} else {
					len += AMPDU_DELIMITER_LEN;
				}
			}

			/* flip the epoch? */
			if (hagg->change_epoch) {
				hagg->epoch = !hagg->epoch;
				hagg->change_epoch = FALSE;
				WL_AMPDU_HWDBG(("sendampdu: fifo %d new epoch: %d frameid %x\n",
					sc, hagg->epoch, frameid));
				WLCNTINCR(ampdu_tx->cnt->epochdelta);
			}
			aggfifo_enque(ampdu_tx, len, sc);
		} else
#endif /* WLAMPDU_MAC */
		{ /* host agg */
			if (count == 0) {
				uint16 fc;
				mcl |= (TXC_AMPDU_FIRST << TXC_AMPDU_SHIFT);
				/* refill the bits since might be a retx mpdu */
				mcl |= TXC_STARTMSDU;
				rts = (struct dot11_rts_frame*)&txh->rts_frame;
				fc = ltoh16(rts->fc);
				if ((fc & FC_KIND_MASK) == FC_RTS) {
					mcl |= TXC_SENDRTS;
					use_rts = TRUE;
				}
				if ((fc & FC_KIND_MASK) == FC_CTS) {
					mcl |= TXC_SENDCTS;
					use_cts = TRUE;
				}
			} else {
				mcl |= (TXC_AMPDU_MIDDLE << TXC_AMPDU_SHIFT);
				mcl &= ~(TXC_STARTMSDU | TXC_SENDRTS | TXC_SENDCTS);
			}

			len = ROUNDUP(len, 4);
			ampdu_len += (len + (ndelim + 1) * AMPDU_DELIMITER_LEN);
			ASSERT(ampdu_len <= scb_ampdu->max_rxlen);
			dma_len += (uint16)pkttotlen(osh, p);

			WL_AMPDU_TX(("wl%d: wlc_sendampdu: ampdu_len %d seg_cnt %d null delim %d\n",
				wlc->pub->unit, ampdu_len, seg_cnt, ndelim));
		}
		txh->MacTxControlLow = htol16(mcl);

		/* this packet is added */
		pkt[count++] = p;
#ifdef WL11K
		pktlen += pkttotlen(osh, p);
#endif // endif

		/* patch the first MPDU */
		if (AMPDU_HOST_ENAB(wlc->pub) && count == 1) {
			uint8 plcp0, plcp3, is40, sgi;
			uint32 txoplen = 0;

			if (rr) {
				plcp0 = plcp[0];
				plcp3 = plcp[3];
			} else {
				plcp0 = txh->FragPLCPFallback[0];
				plcp3 = txh->FragPLCPFallback[3];

				/* Support multiple fallback rate:
				 * For txtimes > rr_retry_limit, use fallback -> fallback.
				 * To get around the fix-rate case check if primary == fallback rate
				 */
				if (ini->txretry[indx] > rr_retry_limit && ampdu_tx_cfg->mfbr &&
				    plcp[0] != plcp0) {
					ratespec_t fbrspec;
					uint8 fbr_mcs;
					uint16 phyctl1;

					use_mfbr = TRUE;
					txh->FragPLCPFallback[3] &=  ~PLCP3_STC_MASK;
					plcp3 = txh->FragPLCPFallback[3];
					fbrspec = wlc_scb_ratesel_getmcsfbr(wlc->wrsi, scb,
						ac, plcp0);
					fbr_mcs = RSPEC_RATE_MASK & fbrspec;

					/* XXX: below is an awkward way to re-construct phyCtrl_1,
					 * and can cause inconsistence if the logic in wlc.c has
					 * changed. Need to find an elegant way ... later.
					 */
					/* restore bandwidth */
					fbrspec &= ~RSPEC_BW_MASK;
					switch (ltoh16(txh->PhyTxControlWord_1_Fbr)
						 & PHY_TXC1_BW_MASK)
					{
						case PHY_TXC1_BW_20MHZ:
						case PHY_TXC1_BW_20MHZ_UP:
							fbrspec |= RSPEC_BW_20MHZ;
						break;
						case PHY_TXC1_BW_40MHZ:
						case PHY_TXC1_BW_40MHZ_DUP:
							fbrspec |= RSPEC_BW_40MHZ;
						break;
						default:
							ASSERT(0);
					}
					if ((RSPEC_IS40MHZ(fbrspec) &&
						(CHSPEC_IS20(wlc->chanspec))) ||
						(IS_CCK(fbrspec))) {
						fbrspec &= ~RSPEC_BW_MASK;
						fbrspec |= RSPEC_BW_20MHZ;
					}

					if (IS_SINGLE_STREAM(fbr_mcs)) {
						fbrspec &= ~(RSPEC_TXEXP_MASK | RSPEC_STBC);

						/* For SISO MCS use STBC if possible */
						if (WLC_IS_STBC_TX_FORCED(wlc) ||
							(RSPEC_ISHT(fbrspec) &&
							WLC_STF_SS_STBC_HT_AUTO(wlc, scb))) {
							uint8 stc = 1;

							ASSERT(WLC_STBC_CAP_PHY(wlc));
							fbrspec |= RSPEC_STBC;

							/* also set plcp bits 29:28 */
							plcp3 = (plcp3 & ~PLCP3_STC_MASK) |
							        (stc << PLCP3_STC_SHIFT);
							txh->FragPLCPFallback[3] = plcp3;
							WLCNTINCR(ampdu_tx->cnt->txampdu_mfbr_stbc);
						} else if (wlc->stf->ss_opmode == PHY_TXC1_MODE_CDD)
						{
							fbrspec |= (1 << RSPEC_TXEXP_SHIFT);
						}
					}
					/* rspec returned from getmcsfbr() doesn't have SGI info. */
					if (WLC_HT_GET_SGI_TX(wlc->hti) == ON)
						fbrspec |= RSPEC_SHORT_GI;
					phyctl1 = wlc_phytxctl1_calc(wlc, fbrspec, wlc->chanspec);
					txh->PhyTxControlWord_1_Fbr = htol16(phyctl1);

					txh->FragPLCPFallback[0] = fbr_mcs |
						(plcp0 & MIMO_PLCP_40MHZ);
					plcp0 = txh->FragPLCPFallback[0];
				}
			}
			is40 = (plcp0 & MIMO_PLCP_40MHZ) ? 1 : 0;
			sgi = PLCP3_ISSGI(plcp3) ? 1 : 0;
			mcs = plcp0 & ~MIMO_PLCP_40MHZ;
			ASSERT(VALID_MCS(mcs));
			maxlen = MIN(scb_ampdu->max_rxlen,
				ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][is40][sgi]);

			/* take into account txop and tx burst limit restriction */
			if (txop) {
				/* rate is in Kbps; txop is in usec
				 * ==> len = (rate * txop) / (1000 * 8)
				 */
				txoplen = (MCS_RATE(mcs, is40, sgi) >> 10) * (txop >> 3);
				maxlen = MIN(maxlen, txoplen);
			}

			/* rebuild the rspec and rspec_fallback */
			rspec = HT_RSPEC(plcp[0] & MIMO_PLCP_MCS_MASK);

			if (plcp[0] & MIMO_PLCP_40MHZ)
				rspec |= RSPEC_BW_40MHZ;

			if (fbr_iscck) /* CCK */
				rspec_fallback =
					CCK_RSPEC(CCK_PHY2MAC_RATE(txh->FragPLCPFallback[0]));
			else { /* MIMO */
				rspec_fallback =
				        HT_RSPEC(txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK);
				if (txh->FragPLCPFallback[0] & MIMO_PLCP_40MHZ)
					rspec_fallback |= RSPEC_BW_40MHZ;
			}

			if (use_rts || use_cts) {
				uint16 fc;
				rts_rspec = wlc_rspec_to_rts_rspec(cfg, rspec, FALSE);
				rts_rspec_fallback = wlc_rspec_to_rts_rspec(cfg, rspec_fallback,
				                                            FALSE);
				/* need to reconstruct plcp and phyctl words for rts_fallback */
				if (use_mfbr) {
					int rts_phylen;
					uint8 rts_plcp_fallback[D11_PHY_HDR_LEN], old_ndelim;
					uint16 phyctl1, xfts;

					old_ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
					phyctl1 = wlc_phytxctl1_calc(wlc, rts_rspec_fallback,
						wlc->chanspec);
					txh->PhyTxControlWord_1_FbrRts = htol16(phyctl1);
					if (use_cts)
						rts_phylen = DOT11_CTS_LEN + DOT11_FCS_LEN;
					else
						rts_phylen = DOT11_RTS_LEN + DOT11_FCS_LEN;

					fc = ltoh16(rts->fc);
					wlc_compute_plcp(wlc, rts_rspec_fallback, rts_phylen,
					fc, rts_plcp_fallback);

					bcopy(rts_plcp_fallback, (char*)&txh->RTSPLCPFallback,
						sizeof(txh->RTSPLCPFallback));

					/* restore it */
					txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM] = old_ndelim;

					/* update XtraFrameTypes in case it has changed */
					xfts = ltoh16(txh->XtraFrameTypes);
					xfts &= ~(PHY_TXC_FT_MASK << XFTS_FBRRTS_FT_SHIFT);
					xfts |= ((IS_CCK(rts_rspec_fallback) ? FT_CCK : FT_OFDM)
						<< XFTS_FBRRTS_FT_SHIFT);
					txh->XtraFrameTypes = htol16(xfts);
				}
			}
		} /* if (first mpdu for host agg) */

		/* test whether to add more */
		if (AMPDU_HOST_ENAB(wlc->pub)) {
			/* HW PR#37644 WAR : stop aggregating if mpdu
			 * count above a threshold for current rate
			 */
			if ((MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) &&
			    (count == f->mcs2ampdu_table[MCS2IDX(mcs)])) {
				WL_AMPDU_ERR(("wl%d: PR 37644: stopping ampdu at %d for mcs %d\n",
					wlc->pub->unit, count, mcs));
				break;
			}

			/* limit the size of ampdu for probe packets */
			if (vrate_probe && (count == ampdu_tx_cfg->probe_mpdu))
				break;

			if (count == scb_ampdu->max_pdu)
				break;
		}
#ifdef WLAMPDU_MAC
		else {
			if (count >= aggfifo_space) {
				uint space;
				space = get_aggfifo_space(ampdu_tx, sc);
				if (space > 0)
					aggfifo_space += space;
				else {
					WL_AMPDU_HW(("sendampdu: break due to aggfifo full."
					     "count %d space %d\n", count, aggfifo_space));
					break;
				}
			}
		}
#endif /* WLAMPDU_MAC */

		/* You can process maximum of AMPDU_MAX_PKTS/AGGFIFO_CAP packets at a time. */
#ifdef WLAMPDU_HW
		if (count >= AGGFIFO_CAP)
			break;
#else
		if (count >= AMPDU_MAX_PKTS)
			break;
#endif // endif

		/* check to see if the next pkt is a candidate for aggregation */
		p = pktq_ppeek(WLC_GET_TXQ(qi), prec);

		if (ampdu_tx_cfg->hiagg_mode) {
			if (!p && (prec & 1)) {
				prec = prec & ~1;
				p = pktq_ppeek(WLC_GET_TXQ(qi), prec);
			}
		}
		if (p) {
			if (WLPKTFLAG_AMPDU(WLPKTTAG(p)) &&
				(WLPKTTAGSCBGET(p) == scb) &&
				((uint8)PKTPRIO(p) == tid)) {
				if (wlc->dma_avoidance_war)
					nsegs = pktsegcnt_war(osh, p);
				else
					nsegs = pktsegcnt(osh, p);
				WL_NONE(("%s: txavail %d  < seg_cnt %d nsegs %d\n", __FUNCTION__,
					TXAVAIL(wlc, fifo), seg_cnt, nsegs));
				/* check if there are enough descriptors available */
				if (TXAVAIL(wlc, fifo) <= (seg_cnt + nsegs)) {
					WLCNTINCR(ampdu_tx->cnt->txfifofull);
					p = NULL;
					continue;
				}

				if (AMPDU_HOST_ENAB(wlc->pub)) {
					plen = pkttotlen(osh, p) + AMPDU_MAX_MPDU_OVERHEAD;
					plen = MAX(scb_ampdu->min_len, plen);
					if ((plen + ampdu_len) > maxlen) {
						p = NULL;
						continue;
					}

					ASSERT((rr && !fbr) || (!rr && fbr));
#ifdef PROP_TXSTATUS
					indx = TX_SEQ_TO_INDEX(WL_SEQ_GET_NUM(WLPKTTAG(p)->seq));
#else
					indx = TX_SEQ_TO_INDEX(WLPKTTAG(p)->seq);
#endif /* PROP_TXSTATUS */
					if (ampdu_tx_cfg->hiagg_mode) {
						if (((ini->txretry[indx] < rr_retry_limit) &&
						     fbr) ||
						    ((ini->txretry[indx] >= rr_retry_limit) &&
						     rr)) {
							p = NULL;
							continue;
						}
					} else {
						if (txretry != ini->txretry[indx]) {
							p = NULL;
							continue;
						}
					}
				}

				p = pktq_pdeq(WLC_GET_TXQ(qi), prec);
				ASSERT(p);
			} else {
#ifdef WLAMPDU_MAC
				if (AMPDU_MAC_ENAB(wlc->pub))
					wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_DSTTID);
#endif // endif
				p = NULL;
			}
		}
	}  /* end while(p) */

	if (count) {

		WL_AMPDU_HWDBG(("%s: tot_mpdu %d txfifo %d\n", __FUNCTION__, count,
			R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
		WLCNTADD(ampdu_tx->cnt->txmpdu, count);
		AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
#ifdef WL11K
		WLCNTADD(wlc->ampdu_tx->cnt->txampdubyte_l, pktlen);
		if (wlc->ampdu_tx->cnt->txampdubyte_l < pktlen)
			WLCNTINCR(wlc->ampdu_tx->cnt->txampdubyte_h);
#endif // endif
		if (AMPDU_HOST_ENAB(wlc->pub)) {
#ifdef WLCNT
			ampdu_tx->cnt->txampdu++;
#endif // endif
#if defined(BCMDBG) || defined(WLTEST) || defined(BCMDBG_AMPDU)
			if (ampdu_tx->amdbg)
				ampdu_tx->amdbg->txmcs[MCS2IDX(mcs)]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
#ifdef WLPKTDLYSTAT
			WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(mcs)], count);
#endif /* WLPKTDLYSTAT */

			/* patch up the last txh */
			txh = (d11txh_t *)PKTDATA(osh, pkt[count - 1]);
			mcl = ltoh16(txh->MacTxControlLow);
			mcl &= ~TXC_AMPDU_MASK;
			mcl |= (TXC_AMPDU_LAST << TXC_AMPDU_SHIFT);
			txh->MacTxControlLow = htol16(mcl);

			/* remove the null delimiter after last mpdu */
			ndelim = txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM];
			txh->RTSPLCPFallback[AMPDU_FBR_NULL_DELIM] = 0;
			ampdu_len -= ndelim * AMPDU_DELIMITER_LEN;

			/* remove the pad len from last mpdu */
			fbr_iscck = ((ltoh16(txh->XtraFrameTypes) & 0x3) == 0);
			len = fbr_iscck ? WLC_GET_CCK_PLCP_LEN(txh->FragPLCPFallback)
				: WLC_GET_MIMO_PLCP_LEN(txh->FragPLCPFallback);
			ampdu_len -= ROUNDUP(len, 4) - len;

			/* patch up the first txh & plcp */
			txh = (d11txh_t *)PKTDATA(osh, pkt[0]);
			plcp = (uint8 *)(txh + 1);

			WLC_SET_MIMO_PLCP_LEN(plcp, ampdu_len);
			/* mark plcp to indicate ampdu */
			WLC_SET_MIMO_PLCP_AMPDU(plcp);

			/* reset the mixed mode header durations */
			if (txh->MModeLen) {
				uint16 mmodelen = wlc_calc_lsig_len(wlc, rspec, ampdu_len);
				txh->MModeLen = htol16(mmodelen);
				preamble_type = WLC_MM_PREAMBLE;
			}
			if (txh->MModeFbrLen) {
				uint16 mmfbrlen = wlc_calc_lsig_len(wlc, rspec_fallback, ampdu_len);
				txh->MModeFbrLen = htol16(mmfbrlen);
				fbr_preamble_type = WLC_MM_PREAMBLE;
			}

			/* set the preload length */
			if (MCS_RATE(mcs, TRUE, FALSE) >= f->dmaxferrate) {
				dma_len = MIN(dma_len, f->ampdu_pld_size);
				if (D11REV_IS(wlc->pub->corerev, 29) ||
				    D11REV_GE(wlc->pub->corerev, 31))
					dma_len >>= 2;
				txh->PreloadSize = htol16(dma_len);
			} else
				txh->PreloadSize = 0;

			mch = ltoh16(txh->MacTxControlHigh);

			/* update RTS dur fields */
			if (use_rts || use_cts) {
				uint16 durid;
				rts = (struct dot11_rts_frame*)&txh->rts_frame;
				if ((mch & TXC_PREAMBLE_RTS_MAIN_SHORT) ==
				    TXC_PREAMBLE_RTS_MAIN_SHORT)
					rts_preamble_type = WLC_SHORT_PREAMBLE;

				if ((mch & TXC_PREAMBLE_RTS_FB_SHORT) == TXC_PREAMBLE_RTS_FB_SHORT)
					rts_fbr_preamble_type = WLC_SHORT_PREAMBLE;

				durid = wlc_compute_rtscts_dur(wlc, use_cts, rts_rspec, rspec,
					rts_preamble_type, preamble_type, ampdu_len, TRUE);
				rts->durid = htol16(durid);
				durid = wlc_compute_rtscts_dur(wlc, use_cts,
					rts_rspec_fallback, rspec_fallback, rts_fbr_preamble_type,
					fbr_preamble_type, ampdu_len, TRUE);
				txh->RTSDurFallback = htol16(durid);
				/* set TxFesTimeNormal */
				txh->TxFesTimeNormal = rts->durid;
				/* set fallback rate version of TxFesTimeNormal */
				txh->TxFesTimeFallback = txh->RTSDurFallback;
			}

			/* set flag and plcp for fallback rate */
			if (fbr) {
				WLCNTADD(ampdu_tx->cnt->txfbr_mpdu, count);
				WLCNTINCR(ampdu_tx->cnt->txfbr_ampdu);
				mch |= TXC_AMPDU_FBR;
				txh->MacTxControlHigh = htol16(mch);
				WLC_SET_MIMO_PLCP_AMPDU(plcp);
				WLC_SET_MIMO_PLCP_AMPDU(txh->FragPLCPFallback);
				if (PLCP3_ISSGI(txh->FragPLCPFallback[3])) {
					WLCNTINCR(ampdu_tx->cnt->u1.txampdu_sgi);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcssgi[MCS2IDX(mcs)]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
				}
				if (PLCP3_ISSTBC(txh->FragPLCPFallback[3])) {
					WLCNTINCR(ampdu_tx->cnt->u2.txampdu_stbc);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcsstbc[MCS2IDX(mcs)]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
				}
			} else {
				if (PLCP3_ISSGI(plcp[3])) {
					WLCNTINCR(ampdu_tx->cnt->u1.txampdu_sgi);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcssgi[MCS2IDX(mcs)]++;
#endif /* defined(BCMDBG) || defined(WLTEST) */
				}
				if (PLCP3_ISSTBC(plcp[3])) {
					WLCNTINCR(ampdu_tx->cnt->u2.txampdu_stbc);
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)
					if (ampdu_tx->amdbg)
						ampdu_tx->amdbg->txmcsstbc[MCS2IDX(mcs)]++;
#endif // endif
				}
			}

			if (txretry)
				WLCNTINCR(ampdu_tx->cnt->txretry_ampdu);

			WL_AMPDU_TX(("wl%d: wlc_sendampdu: count %d ampdu_len %d\n",
				wlc->pub->unit, count, ampdu_len));

			/* inform rate_sel if it this is a rate probe pkt */
			frameid = ltoh16(txh->TxFrameID);
			if (frameid & TXFID_RATE_PROBE_MASK) {
				wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid, TRUE,
					(uint8)txretry, ac);
			}
		}
#if defined(WLC_HIGH_ONLY) && !defined(NEW_TXQ)
		/* start aggregating rpc calls as we pass down the individual pkts in this AMPDU */
		if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU)
			bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU,
				TRUE);
#endif // endif

#ifdef NEW_TXQ
		/* pass on all packets to the consumer */
		for (i = 0; i < count; i++) {
			pktenq(output_q, pkt[i]);
		}
		/* fake a time of txpkt_weight for the ampdu */
		*supplied_time += ampdu_tx_cfg->txpkt_weight;
#else
		/* send all mpdus atomically to the dma engine */
		if (AMPDU_MAC_ENAB(wlc->pub)) {
			for (i = 0; i < count; i++) {
				/* always commit */
				wlc_txfifo(wlc, fifo, pkt[i], &txh_info, TRUE, 1);
			}
		} else {
			for (i = 0; i < count; i++) {
				wlc_txfifo(wlc, fifo, pkt[i], &txh_info,
				           i == (count-1), ampdu_tx_cfg->txpkt_weight);
			}
		}
#endif /* NEW_TXQ */

#if defined(WLC_HIGH_ONLY) && !defined(NEW_TXQ)
		/* stop the rpc packet aggregation and release all queued rpc packets */
		if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU)
			bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU,
				FALSE);
#endif // endif

	} /* endif (count) */

	if (regmpdu) {
		WL_AMPDU_TX(("wl%d: wlc_sendampdu: sending regular mpdu\n", wlc->pub->unit));

		/* mark the pkt regmpdu */
		WLPKTTAG(p)->flags3 |= WLF3_AMPDU_REGMPDU;

		/* inform rate_sel if it this is a rate probe pkt */
		txh = (d11txh_t *)PKTDATA(osh, p);
		frameid = ltoh16(txh->TxFrameID);
		if (frameid & TXFID_RATE_PROBE_MASK) {
			wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid, FALSE, 0, ac);
		}

		WLCNTINCR(ampdu_tx->cnt->txregmpdu);
#ifdef NEW_TXQ
		pktenq(output_q, p);
		/* fake a time of 1 for each pkt */
		*supplied_time += 1;
#else
		wlc_txfifo(wlc, fifo, p, &txh_info, TRUE, 1);
#endif /* NEW_TXQ */

#ifdef WLAMPDU_MAC
		if (AMPDU_MAC_ENAB(wlc->pub))
			wlc_ampdu_change_epoch(ampdu_tx, sc, AMU_EPOCH_CHG_NAGG);
#endif // endif
	}

	return err;
} /* _wlc_sendampdu_noaqm */

#endif /* AMPDU_NON_AQM */

#endif /* TXQ_MUX */

#ifdef WLAMPDU_MAC

#ifdef WL_CS_PKTRETRY

/** PSPRETEND related feature */
INLINE static void
wlc_ampdu_chanspec_update(ampdu_tx_info_t *ampdu_tx, void *p)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_txh_info_t txh_info;

	wlc_get_txh_info(wlc, p, &txh_info);

	wlc_txh_set_chanspec(wlc, &txh_info, wlc->chanspec);
}

/** PSPRETEND related feature */
INLINE static bool
wlc_ampdu_cs_retry(ampdu_tx_info_t *ampdu_tx, tx_status_t *txs, void *p)
{
	const uint supr_status = txs->status.suppr_ind;

	bool pktRetry = TRUE;
#ifdef PROP_TXSTATUS
	/* pktretry will return TRUE on basis of following conditions:
	 * 1: if PROP_TX_STATUS is not enabled OR
	 * 2: if HOST_PROP_TXSTATUS is not enabled OR
	 * 3: if suppresed packet is not from host OR
	 * 4: if suppresed packet is from host AND requested one from Firmware
	 */

	if (!PROP_TXSTATUS_ENAB(ampdu_tx->wlc->pub) ||
		!HOST_PROPTXSTATUS_ACTIVATED(ampdu_tx->wlc) ||
		!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKTFROMHOST) ||
		((WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKTFROMHOST) &&
		(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(p)->wl_hdr_information) &
		WLFC_PKTFLAG_PKT_REQUESTED))) {
		pktRetry = !txs->status.was_acked;
	}
	else
		pktRetry = FALSE;
#endif /* ifdef PROP_TXSTATUS */

	if (!(ampdu_tx->wlc->block_datafifo & DATA_BLOCK_QUIET)) {
		/* Not during channel transition time */
		return FALSE;
	}

	switch (supr_status) {
	case TX_STATUS_SUPR_BADCH:
	case TX_STATUS_SUPR_PPS:
		return pktRetry;
	case 0:
		/*
		 * Packet was not acked.
		 * Likely STA already has changed channel.
		 */
		return (pktRetry ? !txs->status.was_acked : pktRetry);
	default:
		return FALSE;
	}
}
#else
#define wlc_ampdu_chanspec_update(ampdu_tx, p)
#define wlc_ampdu_cs_retry(ampdu_tx, txs, p) FALSE
#endif /* WL_CS_PKTRETRY */

#endif /* WLAMPDU_MAC */

#if !defined(TXQ_MUX)
#ifdef WLAMPDU_MAC

/**
 * Invoked when higher layer tries to queue a transmit MPDU. Called for AC chips when AQM is
 * enabled. Returns BCME_BUSY when there is no room to queue the packet.
 */
static int BCMFASTPATH
#ifdef NEW_TXQ
_wlc_sendampdu_aqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec,
	struct spktq *output_q, int *supplied_time)
#else
_wlc_sendampdu_aqm(ampdu_tx_info_t *ampdu_tx, wlc_txq_info_t *qi, void **pdu, int prec)
#endif /* NEW_TXQ */
{
	wlc_info_t *wlc;
	osl_t* osh;
	void* p;
	struct scb *scb;
	uint8 tid, ac;
	scb_ampdu_tid_ini_t *ini;
	wlc_bsscfg_t *cfg = NULL;
	scb_ampdu_tx_t *scb_ampdu;
	ampdumac_info_t *hagg;
	wlc_txh_info_t tx_info;
	int err = 0;
	uint fifo, npkts,  fifoavail = 0;
	int count = 0;
	uint16 seq;
	struct dot11_header* h;
	bool fill_txh_info = FALSE;
#ifdef WL11K
	uint32 pktlen = 0;
#endif // endif
	wlc_pkttag_t* pkttag;
	bool suppress_pkt = FALSE;
	d11actxh_t* vhtHdr = NULL;
	bool regmpdu_pkt_cmn;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	bool slow_path = FALSE;
#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	uint16 d11buf_avail = 0;
	uint16 success_cnt;
#endif // endif
	uint8 txc_hit = 0;

	BCM_REFERENCE(fifoavail);

	wlc = ampdu_tx->wlc;
	osh = wlc->osh;
	p = *pdu;

	ASSERT(p != NULL);

	/*
	 * since we obtain <scb, tid, fifo> etc outside the loop,
	 * have to break and return if either of them has changed
	 */
	tid = (uint8)PKTPRIO(p);
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	ac = WME_PRIO2AC(tid);
	ASSERT(ac < AC_COUNT);
	fifo = prio2fifo[tid];
	scb = WLPKTTAGSCBGET(p);
	cfg = SCB_BSSCFG(scb);

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
	if (BCMDHDHDR_ENAB()) {
		/* Keep a count of available d11 buffers */
		success_cnt = 0;
		d11buf_avail = lfbufpool_avail(D11_LFRAG_BUF_POOL);
		if (PKTISTXFRAG(osh, p) && !d11buf_avail) {
			return BCME_BUSY;
		}
	}
#endif // endif

#ifdef WL_MU_TX
	if ((BSSCFG_AP(cfg) && MU_TX_ENAB(wlc))) {
		wlc_mutx_sta_txfifo(wlc->mutx, scb, &fifo);
	}
#endif // endif
	ASSERT(fifo < NFIFO_EXT);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ini = scb_ampdu->ini[tid];

#if defined(WLC_HIGH_ONLY) && !defined(NEW_TXQ)
	fifoavail = TXAVAIL(wlc, fifo);
	if (!fifoavail) {
		return BCME_BUSY;
	}
	/* start aggregating rpc calls as we pass down the individual pkts in this AMPDU */
	if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU) {
		bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU,
		TRUE);
	}
#endif // endif
	pkttag = WLPKTTAG(p);

	regmpdu_pkt_cmn = (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
		(ini->ba_state == AMPDU_TID_STATE_BA_OFF) ||
		SCB_PS(scb);

	key = wlc_keymgmt_get_scb_key(wlc->keymgmt, scb,
			WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
	if (key_info.algo == CRYPTO_ALGO_OFF)
		key = wlc_keymgmt_get_bss_tx_key(wlc->keymgmt,
				SCB_BSSCFG(scb), FALSE, &key_info);

	if (key_info.algo != CRYPTO_ALGO_OFF && WLC_KEY_SW_ONLY(&key_info))
		slow_path = TRUE;

	/* exit loop via break */
	while (TRUE) {
		bool regmpdu_pkt = FALSE;
		bool reque_pkt = FALSE;
		bool seq_hole = FALSE;

		ASSERT(pkttag->flags & WLF_AMPDU_MPDU);

		seq = pkttag->seq;
#ifdef PROP_TXSTATUS
		seq = WL_SEQ_GET_NUM(seq);

#ifdef BCMDBG_ASSERT
#ifdef BCMPCIEDEV
		/* Sanity check to ensure suppressed MSDUs are aggregated back together */
		if (BCMPCIEDEV_ENAB() && PROP_TXSTATUS_ENAB(wlc->pub) &&
			WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
			if (WLPKTFLAG_AMSDU(WLPKTTAG(p)) &&
				WL_SEQ_GET_FROMDRV(WLPKTTAG(p)->seq)) {
				uint16 amsdu_seq1 = WLPKTTAG(p)->seq & WL_SEQ_AMSDU_SUPPR_MASK;
				uint16 amsdu_seqn;
				void *amsdu_n = PKTNEXT(wlc->osh, p);

				if (!AMSDUFRAG_ENAB()) {
					/* AMSDU_FRAG_OPT handles AMSDU with single LFRAG */
					ASSERT(amsdu_n); /* minimum two for ASMDU */
				}

				while (amsdu_n != NULL) {
					amsdu_seqn = WLPKTTAG(amsdu_n)->seq &
						WL_SEQ_AMSDU_SUPPR_MASK;

					if (amsdu_seq1 != amsdu_seqn) {
					   WL_PRINT(("ERR: Suppr ASMDU seq: 0x%p:0x%x 0x%p:0x%x\n",
					   p, amsdu_seq1, amsdu_n, amsdu_seqn));
					}

					/* AMSDU could be > 2 subframes */
					amsdu_n = PKTNEXT(wlc->osh, amsdu_n);
				};
			}
		}
#endif /* BCMPCIEDEV */
#endif /* BCMDBG_ASSERT */
#endif /* PROP_TXSTATUS */

		if (!wlc_scb_restrict_can_txq(wlc, scb)) {
			err = BCME_BUSY;
		}
		/* N.B.: WLF_MPDU flag indicates the MPDU is being retransmitted
		 * due to d11 suppression
		 */
		else if (pkttag->flags & WLF_MPDU) {
			err = wlc_prep_pdu(wlc, scb, p, &fifo);
			reque_pkt = TRUE;
		} else if (!slow_path &&
			(err = wlc_prep_sdu_fast(wlc, cfg, scb, p, &fifo, key,
			&key_info, &txc_hit)) == BCME_UNSUPPORTED) {
			err = wlc_prep_sdu(wlc, scb, &p, (int*)&npkts, &fifo);
			pkttag = WLPKTTAG(p);
		}

		/* Update the ampdumac pointer to point correct fifo */
		hagg = &ampdu_tx->hagg[fifo];

		if (err) {
			if (err == BCME_BUSY) {
				WL_AMPDU_TX(("wl%d: %s prep_xdu retry; seq 0x%x\n",
					wlc->pub->unit, __FUNCTION__, seq));
				/* XXX 4360: need a way to differentiate
				 * between txmaxpkts and txavail fail
				 */
				WLCNTINCR(ampdu_tx->cnt->txfifofull);
				WLCNTINCR(ampdu_tx->cnt->sduretry);
				WL_TAF(("%s:%u\n", __FUNCTION__, __LINE__));
				*pdu = p;
				break;
			}

			/* error in the packet; reject it */
			WL_AMPDU_ERR(("wl%d: wlc_sendampdu: prep_xdu rejected; seq 0x%x\n",
				wlc->pub->unit, seq));
			WLCNTINCR(ampdu_tx->cnt->sdurejected);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.sdurejected);

			/* let ampdu_is_exp_seq() catch it, update tx_exp_seq, and send_bar */
			*pdu = NULL;
			WL_TAF(("%s:%u\n", __FUNCTION__, __LINE__));
			break;
		}
		WL_AMPDU_TX(("wl%d: %s: prep_xdu success; seq 0x%x tid %d\n",
			wlc->pub->unit, __FUNCTION__, seq, tid));

		regmpdu_pkt = !(pkttag->flags & WLF_MIMO) || regmpdu_pkt_cmn;

		/* if no error, populate tx hdr info and continue */
		ASSERT(pkttag->flags & WLF_MPDU);
		if (!fill_txh_info) {
			fill_txh_info = (count == 0 ||
			                 reque_pkt ||
			                 regmpdu_pkt ||
			                 (pkttag->flags & WLF_TXCMISS) != 0);
		}
		if (fill_txh_info) {
			wlc_get_txh_info(wlc, p, &tx_info);
			vhtHdr = &(tx_info.hdrPtr->txd);
		} else {
			int tsoHdrSize;
			d11actxh_rate_t * rateInfo;

			tsoHdrSize = wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
#ifdef WLTOEHW
			tx_info.tsoHdrSize = tsoHdrSize;
			tx_info.tsoHdrPtr = (void*)((tsoHdrSize != 0) ?
				PKTDATA(wlc->osh, p) : NULL);
#endif /* WLTOEHW */

			tx_info.hdrPtr = (wlc_txd_t *)(PKTDATA(wlc->osh, p) + tsoHdrSize);

			/* XXX 4360: items past PktInfo only available when using long txd header
			* Need to get from txc cache if short txd
			*/
			tx_info.TxFrameID = vhtHdr->PktInfo.TxFrameID;
			tx_info.MacTxControlLow = vhtHdr->PktInfo.MacTxControlLow;
			tx_info.MacTxControlHigh = vhtHdr->PktInfo.MacTxControlHigh;
#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
			if (ltoh16(tx_info.MacTxControlLow) & D11AC_TXC_HDR_FMT_SHORT) {
				int cache_idx = (ltoh16(tx_info.MacTxControlLow) &
					D11AC_TXC_CACHE_IDX_MASK) >> D11AC_TXC_CACHE_IDX_SHIFT;
				rateInfo = (d11actxh_rate_t *)wlc_txc_get_rate_info_shdr(wlc->txc,
					cache_idx);
				tx_info.hdrSize = D11AC_TXH_SHORT_LEN;
			 } else
#endif // endif
			 {
				 rateInfo = WLC_TXD_RATE_INFO_GET(vhtHdr, wlc->pub->corerev);
				 tx_info.hdrSize = D11AC_TXH_LEN;
			 }
			tx_info.d11HdrPtr = ((uint8 *)tx_info.hdrPtr) + tx_info.hdrSize;
			tx_info.plcpPtr = (rateInfo[0].plcp);
		}

		h = (struct dot11_header*)(tx_info.d11HdrPtr);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;

#ifdef PROP_TXSTATUS
		if (WL_SEQ_GET_FROMDRV(pkttag->seq)) {
			suppress_pkt = TRUE;
			/* Now the dongle owns the packet */
			WL_SEQ_SET_FROMDRV(pkttag->seq, 0);
		}
#endif /* PROP_TXSTATUS */

		/* during roam we get pkts with null a1. kill it here else
		 * does not find scb on dotxstatus and things gets out of sync
		 */
		if (ETHER_ISNULLADDR(&h->a1)) {
			WL_AMPDU_ERR(("wl%d: %s: dropping frame with null a1\n",
				wlc->pub->unit, __FUNCTION__));
			WL_TAF(("%s:%u\n", __FUNCTION__, __LINE__));
			err = BCME_ERROR;

		} else if (pkttag->flags & WLF_FIFOPKT) {
			wlc_ampdu_chanspec_update(ampdu_tx, p);
			/* seq # in suppressed frames have been validated before
			 * so don't validate them again...unless there are queuing
			 * issue(s) causing them to be out-of-order.
			 */
		} else if (!ampdu_is_exp_seq(ampdu_tx, ini, seq, suppress_pkt)) {
			/* If the packet was not retrieved from HW FIFO before
			 * attempting over the air for multi-queue
			 */
			WL_AMPDU_TX(("%s: multi-queue error\n", __FUNCTION__));
			WL_TAF(("%s:%u\n", __FUNCTION__, __LINE__));
			err = BCME_ERROR;
		}

		if (err == BCME_ERROR) {
			WL_TAF(("%s:%u\n", __FUNCTION__, __LINE__));
#ifdef WLTAF
			if (!wlc_taf_reset_scheduling(ampdu_tx->wlc->taf_handle, tid))
#endif // endif
			{
				wlc_ampdu_dec_bytes_inflight(ini, p);
			}
			WLCNTINCR(ampdu_tx->cnt->txdrop);
			PKTFREE(osh, p, TRUE);
			*pdu = NULL;
			break;
		}

		seq_hole = ampdu_detect_seq_hole(ampdu_tx, seq, ini);

		if (regmpdu_pkt) {
			WL_AMPDU_TX(("%s: reg mpdu found\n", __FUNCTION__));

			/* clear ampdu bit, set svht bit */
			tx_info.MacTxControlLow &= htol16(~D11AC_TXC_AMPDU);
#ifdef WL_MU_TX
			/* clear mu bit */
			tx_info.MacTxControlHigh &= htol16(~D11AC_TXC_MU);
#endif // endif
			/* non-ampdu must have svht bit set */
			tx_info.MacTxControlHigh |= htol16(D11AC_TXC_SVHT);
			vhtHdr->PktInfo.MacTxControlLow = tx_info.MacTxControlLow;
			vhtHdr->PktInfo.MacTxControlHigh = tx_info.MacTxControlHigh;

			if (hagg->prev_ft != AMPDU_NONE) {
				/* ampdu switch to regmpdu */
				wlc_ampdu_chgnsav_epoch(ampdu_tx, fifo,
					AMU_EPOCH_CHG_NAGG, scb, tid, &tx_info);
				wlc_txh_set_epoch(ampdu_tx->wlc, tx_info.tsoHdrPtr, hagg->epoch);
				hagg->prev_ft = AMPDU_NONE;
			}
			/* if prev is regmpdu already, don't bother to set epoch at all */

			/* mark the MPDU is regmpdu */
			pkttag->flags3 |= WLF3_AMPDU_REGMPDU;
		} else {
			bool change = FALSE;
			int8 reason_code = 0;

			/* clear svht bit, set ampdu bit */
			tx_info.MacTxControlLow |= htol16(D11AC_TXC_AMPDU);
			/* for real ampdu, clear vht single mpdu ampdu bit */
			tx_info.MacTxControlHigh &= htol16(~D11AC_TXC_SVHT);
			vhtHdr->PktInfo.MacTxControlLow = tx_info.MacTxControlLow;
			vhtHdr->PktInfo.MacTxControlHigh = tx_info.MacTxControlHigh;
#ifdef BCMDBG_ASSERT
			ASSERT(wlc_ampdu_check_percache_info(wlc, ampdu_tx, scb, tid, vhtHdr));
#endif /* BCMDBG_ASSERT */

			if (fill_txh_info) {
				uint16 frameid;
				int8* tsoHdrPtr = tx_info.tsoHdrPtr;

				change = TRUE;
				frameid = ltoh16(tx_info.TxFrameID);
				if (frameid & TXFID_RATE_PROBE_MASK) {
					/* check vrate probe flag
					 * this flag is only get the frame to become first mpdu
					 * of ampdu and no need to save epoch_params
					 */
					wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid,
					                            FALSE, 0, ac);
					reason_code = AMU_EPOCH_CHG_FID;
				} else if (scb != hagg->prev_scb || tid != hagg->prev_tid) {
					reason_code = AMU_EPOCH_CHG_DSTTID;
				} else if (wlc_ampdu_epoch_params_chg(wlc, hagg, vhtHdr)) {
					/* rate/sgi/bw/stbc/antsel tx params */
					reason_code = AMU_EPOCH_CHG_PLCP;
				} else if (ltoh16(tx_info.MacTxControlLow) & D11AC_TXC_UPD_CACHE) {
					reason_code = AMU_EPOCH_CHG_TXC_UPD;
				} else if ((tsoHdrPtr != NULL) &&
				           ((tsoHdrPtr[2] & TOE_F2_TXD_HEAD_SHORT) !=
				            hagg->prev_shdr)) {
					reason_code = AMU_EPOCH_CHG_TXHDR;
				} else {
					change = FALSE;
				}
				fill_txh_info = FALSE;
			}

			if (!change) {
				if (hagg->prev_ft == AMPDU_NONE) {
					/* switching from non-aggregate to aggregate ampdu,
					 * just update the epoch changing parameters(hagg) but
					 * not the epoch.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_NO_CHANGE;
				} else if (seq_hole) {
					/* Flip the epoch if there is a hole in sequence of pkts
					 * sent to txfifo. It is a AQM HW requirement
					 * not to have the holes in txfifo.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_CHG_SEQ;
				}
			}

			if (change) {
				if (ltoh16(tx_info.MacTxControlLow) & D11AC_TXC_UPD_CACHE) {
					fill_txh_info = TRUE;
				}
				wlc_ampdu_chgnsav_epoch(ampdu_tx, fifo, reason_code,
					scb, tid, &tx_info);
			}

			/* always set epoch for ampdu */
			wlc_txh_set_epoch(ampdu_tx->wlc, tx_info.tsoHdrPtr, hagg->epoch);
		}

		/*
		 * XXX 4360 : pending change to: not commit in while loop
		 * and commit with null p after loop
		 */
#ifdef NEW_TXQ
		pktenq(output_q, p);
		/* fake a time of 1 for each pkt */
		*supplied_time += 1;
#else
		wlc_txfifo(wlc, fifo, p, &tx_info, TRUE, 1);
#endif /* NEW_TXQ */
		count++;
#ifdef WL11K
		pktlen += pkttotlen(osh, p);
#endif // endif
#if defined(WLC_HIGH_ONLY) && !defined(NEW_TXQ)
		/* stop sending packets to the fifo  if full */
		if (count == fifoavail) {
			break;
		}
#endif /* WLC_HIGH_ONLY && !NEW_TXQ */

#if defined(BCM_DHDHDR) && defined(DONGLEBUILD)
		if (BCMDHDHDR_ENAB()) {
			if (++success_cnt >= d11buf_avail) {
				/* DHDHDR feature requires a d11buffer to be available
				 * for filling tx headers.
				 */
				break;
			}
		}
#endif /* BCM_DHDHDR && DONGLEBUILD */

		/* check to see if the next pkt is a candidate for aggregation */
		p = pktq_ppeek(WLC_GET_TXQ(qi), prec);

		if (p != NULL) {
			/* XXX 4360: as long as we don't switch fifo, *can* continue
			 * the fifo limit kicks in because of txavil check.
			 * but should we hold to accumulate to high water mark?
			 */
			pkttag = PKTTAG(p);
			if (WLPKTFLAG_AMPDU(pkttag) &&
				(pkttag->_scb == scb) &&
				((uint8)PKTPRIO(p) == tid)) {
				p = pktq_pdeq(WLC_GET_TXQ(qi), prec);
				ASSERT(p);
			} else {
				break;
			}
		} else {
			break;
		}
	} /* end while(TRUE) */

	if (count) {
#ifdef WLCNT
		ASSERT(ampdu_tx->cnt);
#endif // endif
		AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
#ifdef WL11K
		WLCNTADD(wlc->ampdu_tx->cnt->txampdubyte_l, pktlen);
		if (wlc->ampdu_tx->cnt->txampdubyte_l < pktlen)
			WLCNTINCR(wlc->ampdu_tx->cnt->txampdubyte_h);
#endif // endif
	}
#if defined(WLC_HIGH_ONLY) && !defined(NEW_TXQ)
	/* stop the rpc packet aggregation and release all queued rpc packets */
	if (wlc->rpc_agg & BCM_RPC_TP_HOST_AGG_AMPDU) {
		bcm_rpc_tp_agg_set(bcm_rpc_tp_get(wlc->rpc), BCM_RPC_TP_HOST_AGG_AMPDU,
		FALSE);
	}
#endif // endif

	WL_AMPDU_TX(("%s: fifo %d count %d epoch %d txpktpend %d",
		__FUNCTION__, fifo, count, ampdu_tx->hagg[fifo].epoch,
		TXPKTPENDGET(wlc, fifo)));
	if (D11REV_GE(wlc->pub->corerev, 64)) {
		WL_AMPDU_TX((" psm_reg_mux 0x%x aqmqmap 0x%x aqmfifo_status 0x%x\n",
			R_REG(osh, &wlc->regs->u.d11acregs.psm_reg_mux),
			R_REG(osh, &wlc->regs->u.d11acregs.AQMQMAP),
			R_REG(osh, &wlc->regs->u.d11acregs.AQMFifo_Status)));
	} else {
		WL_AMPDU_TX((" fifocnt 0x%x\n",
			R_REG(wlc->osh, &wlc->regs->u.d11acregs.XmtFifoFrameCnt)));
	}

	return err;
} /* _wlc_sendampdu_aqm */

#endif /* WLAMPDU_MAC */

#endif /* TXQ_MUX */

/**
 * Requests the 'communication partner' to send a block ack, so that on reception the transmit
 * window can be advanced.
 *
 * If 'start' is set move window to start_seq
 */
static void
wlc_ampdu_send_bar(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, bool start)
{
	int indx, i;
	scb_ampdu_tx_t *scb_ampdu;
	void *p = NULL;
	uint16 seq;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;

	/* stop retry BAR if subsequent tx has made the BAR seq. stale  */
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub) &&
		(ini->retry_bar != AMPDU_R_BAR_DISABLED)) {
		uint16 offset, stop_retry = 0;

		WL_AMPDU(("retry bar for reason %x, bar_seq 0x%04x, max_seq 0x%04x,"
			"acked_seq 0x%04x, bar_cnt %d, tx_in_transit %d\n",
			ini->retry_bar, ini->barpending_seq, ini->max_seq, ini->acked_seq,
			ini->bar_cnt, ini->tx_in_transit));

		ini->retry_bar = AMPDU_R_BAR_DISABLED;
		offset = MODSUB_POW2(ini->acked_seq, ini->barpending_seq, SEQNUM_MAX);
		if ((offset > ini->ba_wsize) && (offset < (SEQNUM_MAX >> 1)))
			stop_retry = 1;
		/* It is very much possible that bar_pending seq is more than */
		/* max seq by at most 1 */
		if (MODSUB_POW2(ini->max_seq + 1, ini->barpending_seq, SEQNUM_MAX)
			>= (SEQNUM_MAX >> 1))
			stop_retry = 1;
		if (stop_retry) {
			WL_AMPDU(("wl%d: stop retry bar %x\n", ampdu_tx->wlc->pub->unit,
				ini->barpending_seq));
			ini->barpending_seq = SEQNUM_MAX;
			return;
		}
	}
	ini->retry_bar = AMPDU_R_BAR_DISABLED;
	if (ini->bar_ackpending)
		return;

	if (ini->ba_state != AMPDU_TID_STATE_BA_ON)
		return;

	if (ini->bar_cnt >= AMPDU_BAR_RETRY_CNT)
		return;

	if (ini->bar_cnt == (AMPDU_BAR_RETRY_CNT >> 1)) {
		ini->bar_cnt++;
		ini->retry_bar = AMPDU_R_BAR_HALF_RETRY_CNT;
		return;
	}

	if (start)
		seq = ini->start_seq;
	else {
		int offset = -1;

		if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			indx = TX_SEQ_TO_INDEX(ini->start_seq);
			for (i = 0; i < (ini->ba_wsize - ini->rem_window); i++) {
				if (isset(ini->barpending, indx))
					offset = i;
				indx = NEXT_TX_INDEX(indx);
			}

			if (offset == -1) {
				ini->bar_cnt = 0;
				return;
			}

			seq = MODADD_POW2(ini->start_seq, offset + 1, SEQNUM_MAX);
		} else {
			seq = ini->barpending_seq;
			ASSERT(ini->barpending_seq < SEQNUM_MAX);
		}
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);
	ASSERT(scb_ampdu);
	BCM_REFERENCE(scb_ampdu);

	if (ampdu_tx_cfg->no_bar == FALSE) {
		bool blocked;
		p = wlc_send_bar(ampdu_tx->wlc, ini->scb, ini->tid, seq,
		                 DOT11_BA_CTL_POLICY_NORMAL, TRUE, &blocked);
		if (blocked) {
			ini->retry_bar = AMPDU_R_BAR_BLOCKED;
			return;
		}

		if (p == NULL) {
			ini->retry_bar = AMPDU_R_BAR_NO_BUFFER;
			return;
		}

		/* Cancel any pending wlc_send_bar_complete packet callbacks. */
		wlc_pcb_fn_find(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, TRUE);

		/* pcb = packet tx complete callback management */
		if (wlc_pcb_fn_register(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, p)) {
			ini->retry_bar = AMPDU_R_BAR_CALLBACK_FAIL;
			return;
		}

		WLCNTINCR(ampdu_tx->cnt->txbar);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.txbar);
	}

	ini->bar_cnt++;
	ini->bar_ackpending = TRUE;
	ini->bar_ackpending_seq = seq;
	ini->barpending_seq = seq;

	if (ampdu_tx_cfg->no_bar == TRUE) {
		/* Cancel any pending wlc_send_bar_complete packet callbacks. */
		wlc_pcb_fn_find(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, TRUE);
		wlc_send_bar_complete(ampdu_tx->wlc, TX_STATUS_ACK_RCV, ini);
	}
} /* wlc_ampdu_send_bar */

#ifdef WLAMPDU_MAC

/**
 * Called upon tx packet completion indication of d11 core. Used by AC chips with AQM enabled
 * only.
 */
static INLINE void
wlc_ampdu_ini_move_window_aqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini)
{

	ASSERT(ini);
	ASSERT(ampdu_tx);
	ASSERT(ampdu_tx->wlc);
	ASSERT(ampdu_tx->wlc->pub);

	if (ini->acked_seq == ini->start_seq || IS_SEQ_ADVANCED(ini->acked_seq, ini->start_seq)) {
		ini->start_seq = MODINC_POW2(ini->acked_seq, SEQNUM_MAX);
		ini->alive = TRUE;
	}
	/* if possible, release some buffered pdu's */
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, FALSE);
	return;
}

#endif /* WLAMPDU_MAC */

/**
 * Called upon tx packet completion indication of d11 core. Not called for AC chips with AQM
 * enabled.
 */
static INLINE void
wlc_ampdu_ini_move_window_noaqm(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini)
{
	uint16 indx, i, range;

	ASSERT(ini);
	ASSERT(ampdu_tx);
	ASSERT(ampdu_tx->wlc);
	ASSERT(ampdu_tx->wlc->pub);

	/* ba_wsize can be zero in AMPDU_TID_STATE_BA_OFF or PENDING_ON state */
	if (ini->ba_wsize == 0)
		return;

	range = MODSUB_POW2(MODADD_POW2(ini->max_seq, 1, SEQNUM_MAX), ini->start_seq, SEQNUM_MAX);
	for (i = 0, indx = TX_SEQ_TO_INDEX(ini->start_seq);
		(i < range) && (!isset(ini->ackpending, indx)) &&
		(!isset(ini->barpending, indx));
		i++, indx = NEXT_TX_INDEX(indx));

	ini->start_seq = MODADD_POW2(ini->start_seq, i, SEQNUM_MAX);
	/* mark alive only if window moves forward */
	if (i > 0) {
		ini->rem_window += i;
		ini->alive = TRUE;
	}

	/* if possible, release some buffered pdus */
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, FALSE);

	WL_AMPDU_TX(("wl%d: wlc_ampdu_ini_move_window: tid %d start_seq bumped to 0x%x\n",
		ampdu_tx->wlc->pub->unit, ini->tid, ini->start_seq));

	if (ini->rem_window > ini->ba_wsize) {
		WL_ERROR(("wl%d: %s: rem_window %d, ba_wsize %d "
			"i %d range %d sseq 0x%x\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__, ini->rem_window, ini->ba_wsize,
			i, range, ini->start_seq));
		ASSERT(ini->rem_window <= ini->ba_wsize);
	}
}

/**
 * Called upon tx packet completion indication of d11 core. Not called for AC chips with AQM
 * enabled. Bumps up the start seq to move the window.
 */
static INLINE void
wlc_ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu,
	scb_ampdu_tid_ini_t *ini)
{
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		wlc_ampdu_ini_move_window_aqm(ampdu_tx, scb_ampdu, ini);
	else
#endif /* WLAMPDU_MAC */
		wlc_ampdu_ini_move_window_noaqm(ampdu_tx, scb_ampdu, ini);
}

/**
 * Called when the D11 core indicates that transmission of a 'block ack request' packet completed.
 */
static void
wlc_send_bar_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	scb_ampdu_tid_ini_t *ini = (scb_ampdu_tid_ini_t *)arg;
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	uint16 indx, s_seq;
	bool aqm_no_bitmap = AMPDU_AQM_ENAB(ampdu_tx->wlc->pub);

	WL_AMPDU_CTL(("wl%d: wlc_send_bar_complete for tid %d status 0x%x\n",
		wlc->pub->unit, ini->tid, txstatus));

	ASSERT(ini->bar_ackpending);
	ini->bar_ackpending = FALSE;

	/* ack received */
	if (txstatus & TX_STATUS_ACK_RCV) {
		if (!aqm_no_bitmap) {
			for (s_seq = ini->start_seq;
			     s_seq != ini->bar_ackpending_seq;
			     s_seq = NEXT_SEQ(s_seq)) {
				indx = TX_SEQ_TO_INDEX(s_seq);
				if (isset(ini->barpending, indx)) {
					clrbit(ini->barpending, indx);
					if (isset(ini->ackpending, indx)) {
						clrbit(ini->ackpending, indx);
						ini->txretry[indx] = 0;
#ifdef ATF
						wlc_ampdu_atf_reset_bytes_inflight(ini);
#endif // endif
					}
				}
			}
			/* bump up the start seq to move the window */
			wlc_ampdu_ini_move_window(ampdu_tx,
				SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb), ini);
		} else {
			if (IS_SEQ_ADVANCED(ini->bar_ackpending_seq, ini->start_seq) &&
				((IS_SEQ_ADVANCED(ini->max_seq, ini->bar_ackpending_seq) ||
				MODINC_POW2(ini->max_seq, SEQNUM_MAX) == ini->bar_ackpending_seq)))
			{
				ini->acked_seq = ini->bar_ackpending_seq;
				wlc_ampdu_ini_move_window(ampdu_tx,
					SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb), ini);
			}

			if (ini->barpending_seq == ini->bar_ackpending_seq)
				ini->barpending_seq = SEQNUM_MAX;
			ini->bar_cnt--;
		}
	}

	wlc_ampdu_txflowcontrol(wlc, SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb), ini);
	if (aqm_no_bitmap) {
		/* not acked or need to send  bar for another seq no.  */
		if (ini->barpending_seq != SEQNUM_MAX) {
			wlc_ampdu_send_bar(wlc->ampdu_tx, ini, FALSE);
		} else {
			ini->bar_cnt = 0;
			ini->retry_bar = AMPDU_R_BAR_DISABLED;
		}
	} else {
		wlc_ampdu_send_bar(wlc->ampdu_tx, ini, FALSE);
	}
} /* wlc_send_bar_complete */

#ifdef WLAMPDU_MAC

/**
 * Higher layer invokes this function as a result of the d11 core indicating tx completion. Function
 * is only active for AC chips that have AQM enabled. 'Regular' tx MPDU is a non-AMPDU contained
 * MPDU.
 */
static void
wlc_ampdu_dotxstatus_regmpdu_aqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs, wlc_txh_info_t *txh_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 seq;

	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

	if (!scb || !SCB_AMPDU(scb))
		return;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[PKTPRIO(p)];
	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_ERR(("wl%d: ampdu_dotxstatus_regmpdu: NULL ini or pon state for tid %d\n",
			ampdu_tx->wlc->pub->unit, PKTPRIO(p)));
		return;
	}
	ASSERT(ini->scb == scb);

#ifdef WLTAF
	if (!wlc_taf_handle_star(ampdu_tx->wlc->taf_handle, ini->tid,
	                         WLPKTTAG(p)->pktinfo.taf.units,
	                         WLPKTTAG(p)->pktinfo.taf.index))
#endif // endif
	{
		wlc_ampdu_dec_bytes_inflight(ini, p);
	}
#ifdef WLATF
	/* Some of the devices may stop aggregating packets under noisy conditions
	 * This counter tracks the time a MPDU is sent from the AMPDU path
	 */
	WLCNTINCR(AMPDU_ATF_STATS(ini)->reg_mpdu);
#endif // endif
	seq = txh_info->seq;
	if (txs->status.was_acked) {
		ini->acked_seq = seq;
	} else {
#ifdef WL11K
		uint8 tid = PKTPRIO(p);
		wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, ackfail));
#endif // endif
		WLCNTINCR(ampdu_tx->cnt->txreg_noack);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.txreg_noack);
		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus_regmpdu: ack not recd for "
			"seq 0x%x tid %d status 0x%x\n",
			ampdu_tx->wlc->pub->unit, seq, ini->tid, txs->status.raw_bits));

		if ((AP_ENAB(ampdu_tx->wlc->pub) &&
		     (txs->status.suppr_ind == TX_STATUS_SUPR_PMQ)) ||
		     P2P_ABS_SUPR(ampdu_tx->wlc, txs->status.suppr_ind)) {
			/* N.B.: wlc_dotxstatus() will continue
			 * the rest of suppressed packet processing...
			 */
			return;
		}

	}
	return;
} /* wlc_ampdu_dotxstatus_regmpdu_aqm */

#endif /* WLAMPDU_MAC */

/**
 * Higher layer invokes this function as a result of the d11 core indicating tx completion. Function
 * is not active for AC chips that have AQM enabled. 'Regular' tx MPDU is a non-AMPDU contained
 * MPDU.
 */
static void
wlc_ampdu_dotxstatus_regmpdu_noaqm(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint16 indx, seq;

	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

	if (!scb || !SCB_AMPDU(scb))
		return;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[PKTPRIO(p)];
	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_ERR(("wl%d: ampdu_dotxstatus_regmpdu: NULL ini or pon state for tid %d\n",
			ampdu_tx->wlc->pub->unit, PKTPRIO(p)));
		return;
	}
	ASSERT(ini->scb == scb);

	wlc_ampdu_dec_bytes_inflight(ini, p);
#ifdef WLATF
	/* Some of the devices like the 43226 and 43228 may stop
	 * aggregating packets under noisy conditions
	 * This counter tracks the time a MPDU is sent from the AMPDU path
	 */
	WLCNTINCR(AMPDU_ATF_STATS(ini)->reg_mpdu);
#endif // endif

	seq = pkt_txh_seqnum(ampdu_tx->wlc, p);

	if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
		WL_ERROR(("wl%d: %s: unexpected completion: seq 0x%x, "
			"start seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__, seq, ini->start_seq));
		return;
	}

	indx = TX_SEQ_TO_INDEX(seq);
	if (!isset(ini->ackpending, indx)) {
		WL_ERROR(("wl%d: %s: seq 0x%x\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__, seq));
		ampdu_dump_ini(ini);
		ASSERT(isset(ini->ackpending, indx));
		return;
	}

	if (txs->status.was_acked) {
		clrbit(ini->ackpending, indx);
		if (isset(ini->barpending, indx)) {
			clrbit(ini->barpending, indx);
		}
		ini->txretry[indx] = 0;
	} else {
#ifdef WL11K
		uint8 tid = PKTPRIO(p);
		wlc_rrm_stat_qos_counter(scb, tid, OFFSETOF(rrm_stat_group_qos_t, ackfail));
#endif // endif
		WLCNTINCR(ampdu_tx->cnt->txreg_noack);
		AMPDUSCBCNTINCR(scb_ampdu->cnt.txreg_noack);
		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus_regmpdu: ack not recd for "
			"seq 0x%x tid %d status 0x%x\n",
			ampdu_tx->wlc->pub->unit, seq, ini->tid, txs->status.raw_bits));

		/* suppressed pkts are resent; so dont move window */
		if ((AP_ENAB(ampdu_tx->wlc->pub) &&
		     (txs->status.suppr_ind == TX_STATUS_SUPR_PMQ)) ||
		     P2P_ABS_SUPR(ampdu_tx->wlc, txs->status.suppr_ind)) {
			/* N.B.: wlc_dotxstatus() will continue
			 * the rest of suppressed packet processing...
			 */
			return;
		}

	}
} /* wlc_ampdu_dotxstatus_regmpdu_noaqm */

/** function to process tx completion of a 'regular' mpdu. Called by higher layer. */
void
wlc_ampdu_dotxstatus_regmpdu(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	void *p, tx_status_t *txs, wlc_txh_info_t *txh_info)
{
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		wlc_ampdu_dotxstatus_regmpdu_aqm(ampdu_tx, scb, p, txs, txh_info);
	else
#endif /*  WLAMPDU_MAC */
		wlc_ampdu_dotxstatus_regmpdu_noaqm(ampdu_tx, scb, p, txs);
}

/**
 * Upon the d11 core indicating transmission of a packet, AMPDU packet(s) can be freed.
 */
static void
wlc_ampdu_free_chain(ampdu_tx_info_t *ampdu_tx, void *p, tx_status_t *txs, bool txs3_done)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
#if defined(TXQ_MUX)
	int tx_time = 0;
#endif /* TXQ_MUX */
	wlc_txh_info_t txh_info;
	uint16 mcl;
	uint8 queue;
	uint16 count = 0;
#if defined(WLAMPDU_MAC)
	uint16 ncons = 0;

	if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub))
		ncons = wlc_ampdu_rawbits_to_ncons(txs->status.raw_bits);
	else
		ncons = txs->sequence;
#endif // endif
#ifdef HOST_HDR_FETCH
	void *p0 = NULL;
#endif // endif

	WL_AMPDU_TX(("wl%d: wlc_ampdu_free_chain: free ampdu_tx chain\n", wlc->pub->unit));

#if !defined(WLAMPDU_MAC) && !defined(TXQ_MUX)
	/* packet 'count' is only directly used for WLAMPDU_MAC paths, not SW AMPDU.
	 * SW AMPDU uses txpkt_weight as the txpktpend adjustment for an AMDPU.
	 */
	BCM_REFERENCE(count);
#endif // endif

	queue = WLC_TXFID_GET_QUEUE(txs->frameid);

	/* loop through all packets in the dma ring and free */
	while (p) {

		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

		count++;
#if defined(TXQ_MUX)
		tx_time += WLPKTTIME(p);
#endif /* TXQ_MUX */

		wlc_get_txh_info(wlc, p, &txh_info);
		mcl = ltoh16(txh_info.MacTxControlLow);

		WLCNTINCR(ampdu_tx->cnt->orphan);
#ifdef HOST_HDR_FETCH
		/* Pull out next pkt */
		/* With tx header in Host feature subsequent pkt in dma ring is linked to p  */
		if (txs->head_pkt != NULL) {
			p0 = PKTLINK(p);
			/* reset the link */
			PKTSETLINK(p, NULL);

			/* Re use the same header buffer for rest of the packets */
			if (p0) {
				PKTREUSED11BUF(wlc->osh, p, p0);

				/* reset header in host flag */
				ASSERT(__PKTISHDRINHOST(wlc->osh, p0));
				PKTRESETHDRINHOST(wlc->osh, p0);
			}
		}
#endif /* HOST_HDR_FETCH */

		PKTFREE(wlc->osh, p, TRUE);

		if (AMPDU_HOST_ENAB(wlc->pub)) {
#ifdef HOST_HDR_FETCH
			/* HOST_HDR_FETCH doesnt work with AMPDU_HOST_ENAB */
			ASSERT(!HOST_HDR_FETCH_ENAB());
#endif // endif

			/* if not the last ampdu, take out the next packet from dma chain */
			if (((mcl & TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT) == TXC_AMPDU_LAST)
				break;
		} else {
#if defined(WLAMPDU_MAC)
			if (count == ncons) {
				break;
			}
#endif // endif
		}
#ifdef HOST_HDR_FETCH
		if (txs->head_pkt != NULL) {
			p = p0;
		} else
#endif // endif
		{
			/* get next pkt */
			p = GETNEXTTXP(wlc, queue);

		}

#ifdef HOST_HDR_FETCH
		if (HOST_HDR_FETCH_ENAB() && txs->flush_rqst == TRUE) {
			/* Flush path uses sbtopcie scheme to access headers in host */
			/* Remap header location in lfrag to host address */
			ASSERT(txs->head_pkt == NULL);
			PKTSETHOSTREMAP(wlc->osh, p);

		}
#endif /* HOST_HDR_FETCH */
	}

#if defined(TXQ_MUX)
	/* Host mode ampdu_tx->config->txpkt_weight not used in time based queuing */
	WLC_TXFIFO_COMPLETE(wlc, queue, count, tx_time);
#else
	if (AMPDU_ENAB(wlc->pub) && AMPDU_HOST_ENAB(wlc->pub)) {
		WLC_TXFIFO_COMPLETE(wlc, queue, ampdu_tx->config->txpkt_weight, tx_time);
	} else {
		WLC_TXFIFO_COMPLETE(wlc, queue, count, tx_time);
	}
#endif /* TXQ_MUX */

	WL_AMPDU_TX(("%s: fifo %d pkts %d txpktpend %d\n",
		__FUNCTION__, queue, count, TXPKTPENDGET(wlc, queue)));

	/* for corerev >= 40, all txstatus have been read already */
	if (D11REV_GE(wlc->pub->corerev, 40))
		return;

#ifdef WLC_LOW
	/* retrieve the next status if it is there */
	if (txs->status.was_acked) {
		uint32 s1;
		uint8 status_delay = 0;

		ASSERT(txs->status.is_intermediate);

		/* wait till the next 8 bytes of txstatus is available */
		while (((s1 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
			OSL_DELAY(1);
			status_delay++;
			if (status_delay > 10) {
				ASSERT(0);
				return;
			}
		}

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		ASSERT(s1 & TX_STATUS_AMPDU);

#ifdef WLAMPDU_HW
		if (!txs3_done && AMPDU_HW_ENAB(wlc->pub)) {
			/* yet another txs to retrieve */
			status_delay = 0;
			while (((s1 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
				OSL_DELAY(1);
				status_delay++;
				if (status_delay > 10) {
					ASSERT(0);
					return;
				}
			}
		}
#endif /* WLAMPDU_HW */
	}
#endif /* WLC_LOW */
} /* wlc_ampdu_free_chain */

/** Called by higher layer when d11 core indicates transmit completion */
bool BCMFASTPATH
wlc_ampdu_dotxstatus(ampdu_tx_info_t *ampdu_tx, struct scb *scb, void *p, tx_status_t *txs,
	wlc_txh_info_t *txh_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	scb_ampdu_tid_ini_t *ini;
	uint32 s1 = 0, s2 = 0, s3 = 0, s4 = 0;
#ifdef WLC_LOW
	/* For low driver, we don't want this function to return TRUE for certain err conditions
	 * returning true will cause a wlc_fatal_error() to get called.
	 * we want to reserve wlc_fatal_error() call for hw related err conditions.
	 */
	bool need_reset = FALSE;
#else
	/* For high only driver, we need to return TRUE for all error conditions. */
	bool need_reset = TRUE;
#endif /* WLC_LOW */
	BCM_REFERENCE(wlc);
	ASSERT(p);
	ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

#ifdef WLAMPDU_MAC
	/* update aggfifo/side_channel upfront in case we bail out early */
	if (!AMPDU_AQM_ENAB(wlc->pub) && AMPDU_MAC_ENAB(wlc->pub)) {
		ampdumac_info_t *hagg;
		uint16 max_entries = 0;
		uint16 queue, ncons;

		queue = WLC_TXFID_GET_QUEUE(txs->frameid);
		ASSERT(queue < AC_COUNT);
		hagg = &(ampdu_tx->hagg[queue]);
		ncons = txs->sequence;
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub)) {
			max_entries = ampdu_tx->config->aggfifo_depth + 1;
		} else
#endif // endif
		{
#ifdef WLAMPDU_UCODE
		max_entries = hagg->txfs_addr_end - hagg->txfs_addr_strt + 1;
		hagg->txfs_rptr += ncons;
		if (hagg->txfs_rptr > hagg->txfs_addr_end)
			hagg->txfs_rptr -= max_entries;
#endif // endif
		}
		ASSERT(ncons > 0 && ncons < max_entries && ncons <= hagg->in_queue);
		BCM_REFERENCE(max_entries);
		hagg->in_queue -= ncons;
	}
#endif /* WLAMPDU_MAC */

	if (!scb || !SCB_AMPDU(scb)) {
		/* null scb may happen if pkts were in txq when scb removed */
		if (!scb) {
			WL_AMPDU(("wl%d: %s: scb is null\n",
				wlc->pub->unit, __FUNCTION__));
		}
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return need_reset;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	ini = scb_ampdu->ini[(uint8)PKTPRIO(p)];
	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		WL_ERROR(("wl%d: %s: bail: bad ini (%p) or ba_state (%d)\n",
			wlc->pub->unit, __FUNCTION__, ini, (ini ? ini->ba_state : -1)));
		return need_reset;
	}
	ASSERT(ini->scb == scb);
	ASSERT(WLPKTTAGSCBGET(p) == scb);

#ifdef WLAMPDU_MAC
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		wlc_ampdu_dotxstatus_aqm_complete(ampdu_tx, scb, p, txs, txh_info);
		/* agg may be half done here */
		wlc_ampdu_agg_complete(wlc, ampdu_tx, scb_ampdu, ini, FALSE);
#ifdef BCMDBG
		WLC_AMPDU_TXQ_PROF_ADD_ENTRY(ampdu_tx->wlc, scb_ampdu->scb);
#endif // endif
		return FALSE;
	}
#endif /* WLAMPDU_MAC */

	/* BMAC_NOTE: For the split driver, second level txstatus comes later
	 * So if the ACK was received then wait for the second level else just
	 * call the first one
	 */
	if (txs->status.was_acked) {
#ifdef WLC_LOW
		uint8 status_delay = 0;

		scb->used = wlc->pub->now;
		/* wait till the next 8 bytes of txstatus is available */
		while (((s1 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
			OSL_DELAY(1);
			status_delay++;
			if (status_delay > 10) {
				ASSERT(0);
				wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
				return TRUE;
			}
		}

		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: 2nd status in delay %d\n",
			wlc->pub->unit, status_delay));

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		ASSERT(s1 & TX_STATUS_AMPDU);

		s2 = R_REG(wlc->osh, &wlc->regs->frmtxstatus2);

#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub)) {
			/* wait till the next 8 bytes of txstatus is available */
			status_delay = 0;
			while (((s3 = R_REG(wlc->osh, &wlc->regs->frmtxstatus)) & TXS_V) == 0) {
				OSL_DELAY(1);
				status_delay++;
				if (status_delay > 10) {
					ASSERT(0);
					wlc_ampdu_free_chain(ampdu_tx, p, txs, TRUE);
					return TRUE;
				}
			}
			s4 = R_REG(wlc->osh, &wlc->regs->frmtxstatus2);
		}
#endif /* WLAMPDU_HW */
#else
		scb->used = wlc->pub->now;

		/* Store the relevant information in ampdu structure */
		WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: High Recvd \n", wlc->pub->unit));

		ASSERT(!ampdu_tx->p);
		ampdu_tx->p = p;
		bcopy(txs, &ampdu_tx->txs, sizeof(tx_status_t));
		ampdu_tx->waiting_status = TRUE;
		return FALSE;
#endif /* WLC_LOW */
	}

	wlc_ampdu_dotxstatus_complete(ampdu_tx, scb, p, txs, s1, s2, s3, s4);
	wlc_ampdu_txflowcontrol(wlc, scb_ampdu, ini);

	return FALSE;
} /* wlc_ampdu_dotxstatus */

#ifdef WLC_HIGH_ONLY         /* BMAC high driver */

void
wlc_ampdu_txstatus_complete(ampdu_tx_info_t *ampdu_tx, uint32 s1, uint32 s2)
{
	WL_AMPDU_TX(("wl%d: wlc_ampdu_txstatus_complete: High Recvd 0x%x 0x%x p:%p\n",
	          ampdu_tx->wlc->pub->unit, s1, s2, ampdu_tx->p));

	ASSERT(ampdu_tx->waiting_status);

	/* The packet may have been freed if the SCB went away, if so, then still free the
	 * DMA chain
	 */
	if (ampdu_tx->p) {
		wlc_ampdu_dotxstatus_complete(ampdu_tx, WLPKTTAGSCBGET(ampdu_tx->p), ampdu_tx->p,
			&ampdu_tx->txs, s1, s2, 0, 0);
		ampdu_tx->p = NULL;
	}

	ampdu_tx->waiting_status = FALSE;
}

#endif /* WLC_HIGH_ONLY */

#ifdef WLMEDIA_TXFAILEVENT

/**
 * To indicate to the host a TX fail event and provide information such as destination mac address
 * and a portion of the packet.
 */
static void BCMFASTPATH
wlc_tx_failed_event(wlc_info_t *wlc, struct scb *scb, wlc_bsscfg_t *bsscfg, tx_status_t *txs,
	void *p, uint32 flags)
{
	txfailinfo_t txinfo;
	d11txh_t *txh = NULL;
	struct dot11_header *h;
	uint16 fc;
	char *p1;

	/* Just in case we need events only for certain clients or
	 * additional information is needed in the future
	 */
	UNUSED_PARAMETER(scb);
	UNUSED_PARAMETER(flags);

	memset(&txinfo, 0, sizeof(txfailinfo_t));
	wlc_read_tsf(wlc, &txinfo.tsf_l, &txinfo.tsf_h);

	if (txs)
		txinfo.txstatus = txs->status;

	if (p) {
		txinfo.prio  = PKTPRIO(p);
		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		if (txh)
			txinfo.rates = ltoh16(txh->MainRates);
		p1 = (char*) PKTDATA(wlc->osh, p)
			 + sizeof(d11txh_t) + D11_PHY_HDR_LEN;
		h = (struct dot11_header*)(p1);
		fc = ltoh16(h->fc);
		if ((fc & FC_TODS))
			memcpy(&txinfo.dest, h->a3.octet, ETHER_ADDR_LEN);
		else if ((fc & FC_FROMDS))
			memcpy(&txinfo.dest, h->a1.octet, ETHER_ADDR_LEN);
	}

	wlc_bss_mac_event(wlc, bsscfg, WLC_E_TXFAIL, (struct ether_addr *)txh->TxFrameRA,
		WLC_E_STATUS_TIMEOUT, 0, 0, &txinfo, sizeof(txinfo));

} /* wlc_tx_failed_event */

#endif /* WLMEDIA_TXFAILEVENT */

#ifdef WLAMPDU_MAC
static INLINE void
wlc_ampdu_cs_pktretry(struct scb *scb, void *p, uint8 tid)
{
	wlc_pkttag_t *pkttag = WLPKTTAG(p);

	pkttag->flags |= WLF_FIFOPKT; /* retrieved from HW FIFO */
	pkttag->flags3 |= WLF3_SUPR; /* suppressed */
	WLPKTTAGSCBSET(p, scb);

	SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_HI_PREC(tid));
}

static INLINE bool
wlc_ampdu_suppr_pktretry(wlc_info_t *wlc, struct scb *scb, void *p,
	tx_status_t *txs, bool abs_suppr)
{
	wlc_pkttag_t *pkttag = WLPKTTAG(p);

	pkttag->flags |= WLF_FIFOPKT; /* retrieved from HW FIFO */
	WLPKTTAGSCBSET(p, scb);

	return abs_suppr ?
		wlc_pkt_abs_supr_enq(wlc, scb, p) :
		/* last_frag TRUE as fragmentation not allowed for AMPDU */
		wlc_apps_suppr_frame_enq(wlc, p, txs, TRUE);
}

static void BCMFASTPATH
wlc_frmburst_dotxstatus(ampdu_tx_info_t *ampdu_tx, tx_status_t *txs)
{
	if (ampdu_tx->wlc->hti->frameburst && ampdu_tx->config->fb_override_enable &&
	    WLC_HT_GET_AMPDU_RTS(ampdu_tx->wlc->hti)) {
		ampdu_tx->rtstxcnt += txs->status.rts_tx_cnt;
		ampdu_tx->ctsrxcnt += txs->status.cts_rx_cnt;
	} /* if (ampdu_tx->wlc->hti->frameburst ...) */
} /* wlc_frmburst_dotxstatus */

/**
 * Process txstatus which reports how many frames are consumed in txs::ncons.
 * The <ncons> pkts have been either
 * -- suppressed, or
 * -- successfully transmitted and acked, or
 * -- transmitted upto retry limits and been given up.
 * If suppressed, driver should find a way to retry them with seqnumber
 * handled carefully (and change epoch properly).
 * XXX: this path is *not* handled. (Chunyu: I don't know how.)
 *
 * Missing or unverified parts:
 * 1. p2p retry
 * 2. pktlog
 * 3. txmonitor
 * 4. pktdelay
 */
#if defined(WL_MU_TX)
#ifdef WLCNT
void BCMFASTPATH
wlc_ampdu_aqm_mutx_dotxinterm_status(ampdu_tx_info_t *ampdu_tx, tx_status_t *txs)
{
	bool txs_mu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	void *p;
	struct scb *scb = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_txh_info_t txh_info;
	int err;
	uint16 qid = WLC_TXFID_GET_QUEUE(txs->frameid);
	BCM_REFERENCE(err);

	txs_mu = (txs->status.s5 & TX_STATUS64_MUTX) ? TRUE : FALSE;

	if (!txs_mu)
		return;

	p = wlc_bmac_dmatx_peeknexttxp(wlc, qid);
	if (p == NULL) {
		return;
	}

	if (!(WLPKTTAG(p)->flags & WLF_TXHDR)) {
		return;
	}

#ifdef HOST_HDR_FETCH
	if (__PKTISHDRINHOST(wlc->osh, p)) {
		/* Header still in host; Wait for callback after hdr fetch */
		return;
	}
#endif // endif

	wlc_get_txh_info(wlc, p, &txh_info);
	if (txs->frameid != htol16(txh_info.TxFrameID)) {
		WL_ERROR(("wl%d: %s: txs->frameid 0x%x txh->TxFrameID 0x%x\n",
			wlc->pub->unit, __FUNCTION__,
			txs->frameid, htol16(txh_info.TxFrameID)));
		return;
	}

	if (!ETHER_ISMULTI(txh_info.TxFrameRA)) {
		/* use the bsscfg index in the packet tag to find out which
		 * bsscfg this packet belongs to
		 */
		bsscfg = wlc_bsscfg_find(wlc, WLPKTTAGBSSCFGGET(p), &err);
		/* For ucast frames, lookup the scb directly by the RA.
		 * The scb may not be found if we sent a frame with no scb, or if the
		 * scb was deleted while the frame was queued.
		 */
		if (bsscfg != NULL) {
			scb = wlc_scbfindband(wlc, bsscfg,
				(struct ether_addr*)txh_info.TxFrameRA,
				(wlc_txh_info_is5GHz(wlc, &txh_info)
				?
				BAND_5G_INDEX : BAND_2G_INDEX));

			if (scb == NULL)
				return;
			wlc_mutx_upd_interm_counters(wlc->mutx, scb, txs);
		}
	}
}
#endif /* WLCNT */
#endif /* defined(WL_MU_TX) */
static void BCMFASTPATH
wlc_ampdu_dotxstatus_aqm_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
                                  void *p, tx_status_t *txs, wlc_txh_info_t *txh_info)
{
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	scb_ampdu_tid_ini_t *ini;
	wlc_bsscfg_t *bsscfg;
	uint8 queue, tid, tot_mpdu = 0;
#ifdef HOST_HDR_FETCH
	void *p0 = NULL;
#endif // endif

	/* per txstatus info */
	uint ncons, supr_status;
	bool update_rate, retry = FALSE;
	bool fix_rate = FALSE;

	/* per packet info */
	bool acked = FALSE, send_bar = FALSE;
	uint16 seq = 0, bar_seq = 0;

	/* for first packet */
	int rnum = 0;
	ratesel_txs_t rs_txs;

#if defined(TXQ_MUX)
	int tx_time = 0;
#endif /* TXQ_MUX */

	bool txs_mu = FALSE;	/* If txed as MU */
#if defined(WL_MU_TX) && defined(WLCNT)
	bool is_mu = FALSE;	/* If queued as MU */
#endif /* WL_MU_TX && WLCNT */
#ifdef PKTQ_LOG
	uint16 mcs_txrate[RATESEL_MFBR_NUM];
#endif // endif
#if defined(SCB_BS_DATA)
	wlc_bs_data_counters_t *bs_data_counters = NULL;
#endif // endif

	uint16 start_seq = 0xFFFF; /* invalid */

	uint8 ac;
#if defined(WLPKTDLYSTAT) || defined(WL11K)
	uint32 delay, now;
#endif // endif
#if defined WLPKTDLYSTAT
	uint tr;
	scb_delay_stats_t *delay_stats;
#endif // endif

	int k;
	uint16 nlost = 0;
#ifdef PKTQ_LOG
	pktq_counters_t *prec_cnt = NULL;
	pktq_counters_t *actq_cnt = NULL;
	uint32           prec_pktlen = 0;
#endif // endif
#ifdef PSPRETEND
	bool pps_retry = FALSE;
	bool pps_recvd_ack = FALSE;
	d11actxh_t* vhtHdr = NULL;
	uint16	macCtlHigh = 0;
#else
	const bool pps_retry = FALSE;
#endif // endif
#if defined(WLAWDL) && defined(AWDL_FAMILY)
	bool awdl_retry = FALSE;
#endif // endif
	wlc_pkttag_t* pkttag;
	const bool cs_retry = wlc_ampdu_cs_retry(ampdu_tx, txs, p);
	struct scb *scb1;

#ifdef BULK_PKTLIST
	void *list_head;
#endif // endif

#if defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED)) || \
	defined(BCMDBG_AMPDU) || defined(PKTQ_LOG)
	bool is_vht[RATESEL_MFBR_NUM];
	bool is_stbc[RATESEL_MFBR_NUM];

#ifdef WL11AC
	uint8 vht[RATESEL_MFBR_NUM];
	BCM_REFERENCE(vht);
#endif /* WL11AC */

	bzero(is_vht, sizeof(is_vht));
	bzero(is_stbc, sizeof(is_stbc));
#endif /* BCMDBG || (WLTEST && !WLTEST_DISABLED) || BCMDBG_AMPDU */
	bzero(&rs_txs, sizeof(rs_txs));

	BCM_REFERENCE(start_seq);
	BCM_REFERENCE(bsscfg);

	ncons = wlc_ampdu_rawbits_to_ncons(txs->status.raw_bits);

	if (!ncons) {
		WL_ERROR(("ncons is %d in %s", ncons, __FUNCTION__));
		ASSERT(ncons);
		return;
	}

	WLCNTADD(ampdu_tx->cnt->cons, ncons);

	if (!scb || !SCB_AMPDU(scb)) {
		if (!scb)
			WL_ERROR(("wl%d: %s: scb is null\n",
				wlc->pub->unit, __FUNCTION__));

		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	SCB_BS_DATA_CONDFIND(bs_data_counters, wlc, scb);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(scb_ampdu);
	ASSERT(p);
	tid = (uint8)PKTPRIO(p);

	queue = WLC_TXFID_GET_QUEUE(txs->frameid);
	ASSERT(queue < NFIFO_EXT);

	ac = WME_PRIO2AC(tid);
#ifdef WLPKTDLYSTAT
	delay_stats = scb->delay_stats[ac];
#endif // endif

	/*
	2 	RTS tx count
	3 	CTS rx count
	11:4 	Acked bitmap for each of consumed mpdus reported in this txstatus, sequentially.
	That is, bit 0 is for the first consumed mpdu, bit 1 for the second consumed frame.
	*/
	ini = scb_ampdu->ini[tid];

	if (!ini) {
		WL_ERROR(("%s: bad ini error\n", __FUNCTION__));
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	update_rate = (WLPKTTAG(p)->flags & WLF_RATE_AUTO) ? TRUE : FALSE;
	supr_status = txs->status.suppr_ind;

	if (supr_status != TX_STATUS_SUPR_NONE) {
		update_rate = FALSE;
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
		ampdu_tx->amdbg->supr_reason[supr_status] += ncons;
#endif // endif
		WL_AMPDU_TX(("wl%d: %s: supr_status 0x%x\n",
			wlc->pub->unit, __FUNCTION__, supr_status));
		if (supr_status == TX_STATUS_SUPR_BADCH) {
			WLCNTADD(wlc->pub->_cnt->txchanrej, ncons);
#if defined(WLAWDL) && defined(AWDL_FAMILY)
			if (AWDL_ENAB(wlc->pub) && wlc_awdl_suppressed_pkt(wlc, scb, p)) {
				retry = TRUE;
				awdl_retry = TRUE;
			}
#endif // endif
		} else if (supr_status == TX_STATUS_SUPR_EXPTIME) {
			/* XXX 4360 FIXME: retry - there is a need
			 * to update the epoch and make sure the send ampdu code will
			 * correctly handle the sequence numbers
			 */
			WLCNTADD(wlc->pub->_cnt->txexptime, ncons);
#ifdef WL11K
			wlc_rrm_tscm_upd(scb, tid,
				OFFSETOF(rrm_tscm_t, msdu_exp), ncons);
#endif // endif
			/* Interference detected */
			if (wlc->rfaware_lifetime)
				wlc_exptime_start(wlc);
		} else if (supr_status == TX_STATUS_SUPR_FRAG) {
			WL_ERROR(("wl%d: %s: tx underflow?!\n", wlc->pub->unit, __FUNCTION__));
		}
#ifdef PSPRETEND
		else if (supr_status == TX_STATUS_SUPR_PPS) {
			/* explicity set retry FALSE because we are not sure yet (will be set
			 * later in complicated logical test)
			 */
			retry = FALSE;
		}
#endif /* PSPRETEND */
		else if (wlc_should_retry_suppressed_pkt(wlc, p, supr_status)) {
			/* N.B.: we'll transmit the packet when coming out of the
			 * absence period....use the retry logic blow to reenqueue
			 * the packet.
			 */
			retry = TRUE;
		}
	} else if (txs->phyerr) {
		update_rate = FALSE;
		WLCNTADD(wlc->pub->_cnt->txphyerr, ncons);
		WL_ERROR(("wl%d: %s: tx phy error (0x%x)\n",
			wlc->pub->unit, __FUNCTION__, txs->phyerr));
	}

#if defined(BCMDBG) && defined(PSPRETEND)
	/* we are draining the fifo yet we received a tx status which isn't suppress - this
	 * is an error we should trap if we are still in the same ps pretend instance
	 */
	if (supr_status == TX_STATUS_SUPR_PPS) {
		/* if this is the first packet to be suppressed, record the
		 * ps pretend instance where we start suppressing packets
		 */
		if (scb->ps_pretend_suppress_count == 0) {
			scb->ps_pretend_suppress_index = scb->ps_pretend_count;
		}
		/* increment packet suppression counter */
		scb->ps_pretend_suppress_count++;
	} else if (supr_status) {
		/* other suppression started while pspretend is still active */
	} else if (scb->ps_pretend_suppress_count > 0) {
		if (SCB_PS_PRETEND(scb) && (wlc->block_datafifo & DATA_BLOCK_PS) &&
		       (scb->ps_pretend_suppress_index == scb->ps_pretend_count)) {
			WL_ERROR(("wl%d.%d: "MACF" packet not suppressed when fifo still draining "
					  "(%d drained/%d remaining)%s\n", wlc->pub->unit,
					  WLC_BSSCFG_IDX(bsscfg),
					  ETHER_TO_MACF(scb->ea), scb->ps_pretend_suppress_count,
					  TXPKTPENDTOT(wlc),
					  SCB_PS_PRETEND_BLOCKED(scb) ? "  BLOCKED":""));
		}
		scb->ps_pretend_suppress_count = 0;
	}
#endif /* BCMDBG && PSPRETEND */

	/* Check if interference is still there */
	if (wlc->rfaware_lifetime && wlc->exptime_cnt && (supr_status != TX_STATUS_SUPR_EXPTIME))
		wlc_exptime_check_end(wlc);

	if (supr_status == TX_STATUS_SUPR_NONE) {
		wlc_frmburst_dotxstatus(ampdu_tx, txs);
	}

#if defined(WLPKTDLYSTAT) || defined(WL11K)
	/* Get the current time */
	now = WLC_GET_CURR_TIME(wlc);
#endif // endif

#ifdef BULK_PKTLIST
	/* Use BULK TX DMA completion if there are at least 2 packets to consume */
	if (ncons > 1 &&
#ifdef HOST_HDR_FETCH
		(txs->head_pkt == NULL) &&
#endif // endif
		TRUE) {
#ifdef BCMDBG_ASSERT
		uint16 nproc = wlc_bmac_dma_bulk_txcomplete(wlc, queue, (ncons - 1),
				&list_head, NULL, HNDDMA_RANGE_ALL);
		/* Bulk TX completion error, hard failure if (ncons - 1) != nproc */
		ASSERT(nproc == (ncons - 1));
#else
		/* In the non-ASSERT builds we would have had to add a redundant BCM_REFERENCE()
		 * directive to shut the compiler up, it consumes unnecessary cycles
		 */
		wlc_bmac_dma_bulk_txcomplete(wlc, queue, (ncons - 1),
				&list_head, NULL, HNDDMA_RANGE_ALL);
#endif // endif
	}
#endif /* BULK_PKTLIST */

	/* ncons is non-zero, so can enter unconditionally */
	/* exit via break in loop body */
	while (TRUE) {
		bool free_pkt = TRUE;

		pkttag = WLPKTTAG(p);

		ASSERT(pkttag->flags & WLF_AMPDU_MPDU);

		seq = pkttag->seq;

#ifdef PROP_TXSTATUS
		seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */
#ifdef WLTAF
		if (!wlc_taf_handle_star(wlc->taf_handle, ini->tid,
		                         WLPKTTAG(p)->pktinfo.taf.units,
		                         WLPKTTAG(p)->pktinfo.taf.index))
#endif // endif
		{
			wlc_ampdu_dec_bytes_inflight(ini, p);
		}
#if defined(TXQ_MUX)
		tx_time += WLPKTTIME(p);
#endif /* TXQ_MUX */

#ifdef PKTQ_LOG
		if (scb_ampdu->txq.pktqlog) {
			prec_cnt = wlc_ampdu_pktqlog_cnt(&scb_ampdu->txq, tid,
				WLAMPDU_PREC_TID2PREC(p, tid));
		}

		if ((WLC_GET_TXQ(wlc->active_queue))->pktqlog) {
			actq_cnt =
				(WLC_GET_TXQ(wlc->active_queue))->
					pktqlog->_prec_cnt[WLC_PRIO_TO_PREC(tid)];
		}

		if (supr_status != TX_STATUS_SUPR_NONE) {
			WLCNTCONDINCR(prec_cnt, prec_cnt->suppress);
			WLCNTCONDINCR(actq_cnt, actq_cnt->suppress);
		}
#endif /* PKTQ_LOG */
		if (tot_mpdu == 0) {
			bool last_rate, is_sgi = FALSE;
			uint8 bw;
			uint16 ft;
			d11actxh_rate_t *rinfo;

#ifdef PKTQ_LOG
			prec_pktlen = txh_info->d11FrameSize - DOT11_LLC_SNAP_HDR_LEN -
			              DOT11_MAC_HDR_LEN - DOT11_QOS_LEN;
#endif // endif

#ifdef BCMDBG
			if (txs->frameid != htol16(txh_info->TxFrameID)) {
				WL_ERROR(("wl%d: %s: txs->frameid 0x%x"
					" txh->TxFrameID 0x%x\n",
					wlc->pub->unit, __FUNCTION__,
					txs->frameid, htol16(txh_info->TxFrameID)));
				ASSERT(txs->frameid == htol16(txh_info->TxFrameID));
			}
#endif // endif
			start_seq = seq;
			fix_rate = ltoh16(txh_info->MacTxControlHigh) & D11AC_TXC_FIX_RATE;
			rinfo = (d11actxh_rate_t *)
				(txh_info->plcpPtr - OFFSETOF(d11actxh_rate_t, plcp[0]));
			rnum = 0;
			do {
				ft = ltoh16(rinfo->PhyTxControlWord_0) & PHY_TXC_FT_MASK;
				/* note: rs_txs.txrspec[rnum] can be MCS32 */
				rs_txs.txrspec[rnum] = ltoh16(rinfo->PhyTxControlWord_2) &
					D11AC_PHY_TXC_PHY_RATE_MASK;

#if defined(WLPROPRIETARY_11N_RATES)
				if (ltoh16(rinfo->PhyTxControlWord_1) & D11AC_PHY_TXC_11N_PROP_MCS)
					rs_txs.txrspec[rnum] |= (1 << HT_MCS_BIT6_SHIFT);
#endif /* WLPROPRIETARY_11N_RATES */

#ifdef PKTQ_LOG
				mcs_txrate[rnum] = rinfo->TxRate;
#endif // endif
				is_sgi = FALSE;
				if (ft == FT_VHT) {
					/* VHT rate */
					uint8 nss, modu;
					modu = rs_txs.txrspec[rnum] & 0xf;
					nss = (rs_txs.txrspec[rnum] >> 4) & 0xf;
					is_sgi = VHT_PLCP3_ISSGI(rinfo->plcp[3]);
					bw = VHT_PLCP0_BW(rinfo->plcp[0]);
#if (defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED)) || \
	defined(BCMDBG_AMPDU))
#ifdef WL11AC
					vht[rnum] = nss * MAX_VHT_RATES + modu;
#endif /* WL11AC */
					is_vht[rnum] = TRUE;
					is_stbc[rnum] =
						(rinfo->plcp[0] & VHT_SIGA1_STBC) ? TRUE : FALSE;
#endif /* BCMDBG || WLTEST || BCMDBG_AMPDU */
					rs_txs.txrspec[rnum] =
#ifdef WLSCB_HISTO
					/* Generic fix; to avoid abandons / mem change */
						RSPEC_ENCODE_VHT |
#endif /* WLSCB_HISTO */
						0x80 | ((nss + 1) << 4) | modu;
				}
				else {
#ifdef WLSCB_HISTO
					/* Generic fix; to avoid abandons / mem change */
					if (ft == FT_HT) {
						rs_txs.txrspec[rnum] |= RSPEC_ENCODE_HT;
			                }
#endif /* WLSCB_HISTO */
#if (defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED)))
					is_stbc[rnum] = PLCP3_ISSTBC(rinfo->plcp[3]);
#endif // endif
					is_sgi = PLCP3_ISSGI(rinfo->plcp[3]);
					bw = (rinfo->plcp[0] & MIMO_PLCP_40MHZ) ? 1 : 0;
				}
				rs_txs.txrspec[rnum] |= is_sgi ? RSPEC_SHORT_GI : 0;
				/* plcp bw to rspec bw (plcp bw + 1 = rspec bw) */
				rs_txs.txrspec[rnum] |= ((bw + BW_20MHZ) << RSPEC_BW_SHIFT);

				last_rate = (ltoh16(rinfo->RtsCtsControl) &
				             D11AC_RTSCTS_LAST_RATE) ?
					TRUE : FALSE;

				rnum ++;
				rinfo++;
			} while (!last_rate && rnum < RATESEL_MFBR_NUM);
#if defined(WL_MU_TX)
			txs_mu = (txs->status.s5 & TX_STATUS64_MUTX) ? TRUE : FALSE;
#if defined(WLCNT)
			is_mu = ((ltoh16(txh_info->MacTxControlHigh) & D11AC_TXC_MU) != 0);
#endif /* WLCNT */
#endif /* WL_MU_TX */
		} /* tot_mpdu == 0 */

		acked = wlc_txs_was_acked(wlc, &(txs->status), tot_mpdu);

#ifdef WL_CS_PKTRETRY
		if (cs_retry) {
			/* Retry packet */
			wlc_ampdu_cs_pktretry(scb, p, tid);

			/* Mark that packet retried */
			free_pkt = FALSE;
			acked = FALSE;

			WLCNTINCR(ampdu_tx->cnt->cs_pktretry_cnt);
		}
#endif /* WL_CS_PKTRETRY */

		if (!acked && supr_status == TX_STATUS_SUPR_NONE) {
			nlost ++;
		}

#ifdef PSPRETEND
		if (supr_status == TX_STATUS_SUPR_PPS) {
			/* a */
			acked = FALSE;
		}

		if (acked) {
			pps_recvd_ack = TRUE;
		}

		/* if packet retried make it invisible for ps pretend
		 * the packet has to actually not have been sent ok (not acked)
		 */
		if (!cs_retry && !acked) {
			pps_retry = wlc_apps_pps_retry(wlc, scb, p, txs);
		} else {
			pps_retry = FALSE;
		}

		if (pps_retry) {
			wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
			macCtlHigh = ltoh16(vhtHdr->PktInfo.MacTxControlHigh);
		} else {
			/* unset vhtHdr, macCtlHigh for next p, because we assume they are
			 * only valid when pps_retry is TRUE
			 */
			vhtHdr = NULL;
			macCtlHigh = 0;
		}
#endif /* PSPRETEND */

#ifdef PKTQ_LOG
		if (!acked && supr_status) {
			WLCNTCONDINCR(prec_cnt, prec_cnt->suppress);
			WLCNTCONDINCR(actq_cnt, actq_cnt->suppress);
		}
#endif // endif
		if (acked) {
			WL_AMPDU_TX(("wl%d: %s pkt ack seq 0x%04x idx %d\n",
				wlc->pub->unit, __FUNCTION__, seq, tot_mpdu));
			ini->acked_seq = seq;

			/* update the scb used time */
			scb->used = wlc->pub->now;
#ifdef WLATF
			AMPDU_ATF_STATE(ini)->last_ampdu_fid = txs->frameid;
#endif // endif

#ifdef PKTQ_LOG
			WLCNTCONDINCR(prec_cnt, prec_cnt->acked);
			WLCNTCONDINCR(actq_cnt, actq_cnt->acked);
			SCB_BS_DATA_CONDINCR(bs_data_counters, acked);
			WLCNTCONDADD(prec_cnt, prec_cnt->throughput, prec_pktlen);
			WLCNTCONDADD(actq_cnt, actq_cnt->throughput, prec_pktlen);
			SCB_BS_DATA_CONDADD(bs_data_counters, throughput, prec_pktlen);
#endif // endif
			WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_bytes_total[ini->tid],
				prec_pktlen);
#ifdef BCMPCIEDEV
			WLCNTSCBINCR(scb->flr_txpkt[ac]);
#endif // endif

#if defined(BCMCCX) && defined(CCX_SDK)
			if (IHV_ENAB(wlc->ccx) &&
			    BSSCFG_STA(bsscfg) && bsscfg->BSS &&
			    wlc->ccx->frame_log) {
				wlc_get_txh_info(wlc, p, txh_info);
				wlc_ccx_log_tx_frame(wlc->ccx, (uint8*)txh_info->d11HdrPtr,
					txh_info->d11FrameSize, TX_STATUS_ACK_RCV, FALSE, TRUE);
			}
			/* call any matching pkt callbacks */
			if (pkttag->flags & WLF_IHV_TX_PKT) {
				/* additional information for IHV pkt callback */
				wlc->ccx->ihv_txpkt = p;
				wlc->ccx->ihv_txpkt_sent = TRUE;
				wlc->ccx->ihv_txpkt_max_retries = FALSE;
			}
#endif /* BCMCCX && CCX_SDK */
			wlc_pcb_fn_invoke(wlc->pcb, p, TX_STATUS_ACK_RCV);

		}

#ifdef PSPRETEND
		if (pps_retry) {
			ASSERT(!acked);

			if (!supr_status) {
#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->ps_retry);
				WLCNTCONDINCR(actq_cnt, actq_cnt->ps_retry);
#endif // endif
#ifdef WLINTFERSTAT
				wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt, scb);
#endif // endif
			}

			/* in case we haven't yet processed PMQ status change because of
			 * previously blocked data path or delayed PMQ interrupt
			 */
			wlc_apps_scb_pspretend_on(wlc, scb, 0);
			ASSERT(SCB_PS(scb));

			/* check to see if successive pps transitions exceed limit */
			if (scb->ps_pretend_succ_count >= bsscfg->ps_pretend_retry_limit) {

				/* The retry limit has been hit, so clear the PPS bit so
				 * they no longer trigger another ps pretend.
				 */

				macCtlHigh &= ~D11AC_TXC_PPS;
				vhtHdr->PktInfo.MacTxControlHigh = htol16(macCtlHigh);
			}
			if ((pkttag->flags & WLF_TXHDR) &&
			      !(macCtlHigh & D11AC_TXC_FIX_RATE))
			{
				/* XXX To do. Re-calculate (or else remove completely)
				* the tx hdr, because rate information may no longer
				* be appropriate
				*/
			}

			free_pkt = wlc_ampdu_suppr_pktretry(wlc, scb, p, txs, FALSE);

			/* unset vhtHdr, macCtlHigh for next p, because we assume they are
			 * only valid when pps_retry is TRUE
			 */
			vhtHdr = NULL;
			macCtlHigh = 0;
		}
#endif /* PSPRETEND */

		/* either retransmit or send bar if ack not recd */
		if (!acked && !pps_retry && !cs_retry) {

			if (retry) {
				if (AP_ENAB(wlc->pub) &&
				    supr_status == TX_STATUS_SUPR_PMQ) {
					/* last_frag TRUE as fragmentation not allowed for AMPDU */
					free_pkt = wlc_apps_suppr_frame_enq(wlc, p, txs, TRUE);
				}
				else if (P2P_ABS_SUPR(wlc, supr_status) &&
				         BSS_TX_SUPR_ENAB(bsscfg)) {
					/* This is possible if we got a packet suppression
					 * before getting ABS interrupt
					 */
					if (!BSS_TX_SUPR(bsscfg) &&
#ifdef WLP2P
					    (wlc_p2p_noa_valid(wlc->p2p, bsscfg) ||
					     wlc_p2p_ops_valid(wlc->p2p, bsscfg)) &&
#endif // endif
					    TRUE)
						wlc_bsscfg_tx_stop(bsscfg);
					if (BSSCFG_AP(bsscfg) &&
					    SCB_ASSOCIATED(scb) && SCB_P2P(scb))
						wlc_apps_scb_tx_block(wlc, scb, 1, TRUE);
					/* With Proptxstatus enabled in dongle and host,
					 * pkts sent by host will not come here as retry is set
					 * to FALSE by wlc_should_retry_suppressed_pkt().
					 * With Proptxstatus disabled in dongle or
					 * not active in host, all the packets will come here
					 * and need to be re-enqueued.
					 */
					free_pkt = wlc_pkt_abs_supr_enq(wlc, scb, p);
				}
				if (free_pkt)
					goto not_retry;
			} else {
			not_retry:
				bar_seq = seq;
#ifdef PROP_TXSTATUS
#if !defined(REUSE_SEQ_DISABLED)
				if (!WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl)) || !supr_status)
#else
				if (!supr_status)
#endif // endif
#endif /* PROP_TXSTATUS */
				{
					send_bar = TRUE;
				}

				WL_AMPDU_TX(("wl%d: %s: pkt seq 0x%04x not acked\n",
					wlc->pub->unit, __FUNCTION__, seq));

#ifdef WLMEDIA_TXFAILEVENT
				wlc_tx_failed_event(wlc, scb, bsscfg, txs, p, 0);
#endif /* WLMEDIA_TXFAILEVENT */

#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->retry_drop);
				WLCNTCONDINCR(actq_cnt, actq_cnt->retry_drop);
				SCB_BS_DATA_CONDINCR(bs_data_counters, retry_drop);
#endif // endif
#ifdef WLINTFERSTAT
				/* Count only failed packets and indicate failure if required */
				wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt, scb);
#endif // endif
			}

#ifdef PROP_TXSTATUS
			if (free_pkt &&
			    supr_status && WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl))) {
			}
#endif /* PROP_TXSTATUS */
		}

#if defined(WLPKTDLYSTAT) || defined(WL11K)
		if (free_pkt) {
			/* calculate latency and packet loss statistics */
			/* Ignore wrap-around case and error case */
			if (now > pkttag->shared.enqtime) {
				delay = (now - pkttag->shared.enqtime);
#ifdef WLPKTDLYSTAT
				if (delay_stats) {
					tr = 0;
					wlc_delay_stats_upd(delay_stats, delay, tr, acked);
					WL_AMPDU_STAT(("Enq %d retry %d cnt %d: acked %d delay/min"
						"/max/sum %d %d %d %d\n", pkttag->shared.enqtime,
						tr, delay_stats->txmpdu_cnt[tr], acked, delay,
						delay_stats->delay_min, delay_stats->delay_max,
						delay_stats->delay_sum[tr]));
				}
#endif /* WLPKTDLYSTAT */
#ifdef WL11K
				if (acked) {
					wlc_rrm_delay_upd(scb, tid, delay);
				}
#endif /* WL11K */
			}
		}
#endif /* WLPKTDLYSTAT || WL11K */
#ifdef PROP_TXSTATUS
		if ((free_pkt || WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub)) &&
			PROP_TXSTATUS_ENAB(wlc->pub)) {
			uint8 wlfc_status = WLFC_D11_STATUS_INTERPRET(acked,
					TXS_SUPR_MAGG_DONE(txs->status.suppr_ind));
#ifdef PSPRETEND
			if (pps_retry && (wlfc_status == WLFC_CTL_PKTFLAG_DISCARD_NOACK))
				wlfc_status = WLFC_CTL_PKTFLAG_D11SUPPRESS;
#endif // endif

			if (!acked && scb &&
					(wlfc_status == WLFC_CTL_PKTFLAG_D11SUPPRESS)) {
				if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub))
					wlc_suppress_sync_fsm(wlc, scb, p, TRUE);
				wlc_process_wlhdr_txstatus(wlc, wlfc_status, p, FALSE);
				wlc_ampdu_ini_adjust(wlc->ampdu_tx, ini, p);
			} else {
				wlfc_process_wlhdr_complete_txstatus(wlc->wl, wlfc_status,
						p, txs, ((tot_mpdu < (ncons - 1)) ? TRUE : FALSE));
			}
		}
#endif /* PROP_TXSTATUS */

free_and_next:
#ifdef HOST_HDR_FETCH
		if (txs->head_pkt != NULL) {
			p0 = PKTLINK(p);
			/* reset the link */
			PKTSETLINK(p, NULL);

			/* Re use the same header buffer for rest of the packets */
			if (p0) {
				PKTREUSED11BUF(wlc->osh, p, p0);

				/* reset header in host flag */
				ASSERT(__PKTISHDRINHOST(wlc->osh, p0));
				PKTRESETHDRINHOST(wlc->osh, p0);
			}
		}
#endif /* HOST_HDR_FETCH */

#ifdef WLSCB_HISTO
		if (fix_rate && !txs_mu) {
			rs_txs.txsucc_cnt[0] = (txs->status.s3 >> 16) & 0xffff;
			WLSCB_HISTO_TX_IF_HTVHT(scb, rs_txs.txrspec[0], 1 << WLPKTTAG_AMSDU(p));
		} else {
			rs_txs.txsucc_cnt[0] = (txs->status.s3 >> 8) & 0xff;
			if (!txs_mu) {
				rs_txs.txsucc_cnt[1] = (txs->status.s3 >> 24) & 0xff;
				rs_txs.txsucc_cnt[2] = (txs->status.s4 >>  8) & 0xff;
				rs_txs.txsucc_cnt[3] = (txs->status.s4 >> 24) & 0xff;
			}
			if (rs_txs.txsucc_cnt[0]) {
				WLSCB_HISTO_TX_IF_HTVHT(scb, rs_txs.txrspec[0],
						1 << WLPKTTAG_AMSDU(p));
			}
			if (rs_txs.txsucc_cnt[1]) {
				WLSCB_HISTO_TX_IF_HTVHT(scb, rs_txs.txrspec[1],
						1 << WLPKTTAG_AMSDU(p));
			}
			if (rs_txs.txsucc_cnt[2]) {
				WLSCB_HISTO_TX_IF_HTVHT(scb, rs_txs.txrspec[2],
						1 << WLPKTTAG_AMSDU(p));
			}
			if (rs_txs.txsucc_cnt[3]) {
				WLSCB_HISTO_TX_IF_HTVHT(scb, rs_txs.txrspec[3],
						1 << WLPKTTAG_AMSDU(p));
			}
		}
#endif /* WLSCB_HISTO */

		if (free_pkt) {
			PKTFREE(wlc->osh, p, TRUE);
		}
		tot_mpdu++;

		/* only loop ncons times */
		if (tot_mpdu >= ncons) {
			break;
		}

		/* get next pkt -- check and handle NULL */
#ifdef HOST_HDR_FETCH
		if (txs->head_pkt != NULL) {
			p = p0;
		} else
#endif /* HOST_HDR_FETCH */
		{
#ifdef BULK_PKTLIST
			if (ncons > 1) {
				BULK_GETNEXTTXP(list_head, p);
			}
#else
			p = GETNEXTTXP(wlc, queue);
#endif // endif
		}

#ifdef HOST_HDR_FETCH
		if (HOST_HDR_FETCH_ENAB() && txs->flush_rqst == TRUE) {
			/* Remap lfrag header loation to host scratch area
			 * Instead of fetching headers from host to local memory,
			 * PKTDATA in lfrag is pointed to host section.
			 * Use sbtopcie to do further access of headers over pcie bus.
			 */
			ASSERT(txs->head_pkt == NULL);
			PKTSETHOSTREMAP(wlc->osh, p);
		}
#endif // endif

		/* if tot_mpdu < ncons, should have pkt in queue */
		if (p == NULL) {
			WL_ERROR(("%s: p is NULL. tot_mpdu: %d, ncons: %d\n",
				__FUNCTION__, tot_mpdu, ncons));
			ASSERT(p);
			break;
		}

		/* validate scb pointer */
		scb1 = WLPKTTAGSCBGET(p);
		if (scb1 != scb) {
			WL_ERROR(("%s: scb 0x%p scb1 0x%p\n", __FUNCTION__, scb, scb1));
			ASSERT(0);
			free_pkt = TRUE;
			goto free_and_next;
		}

#ifdef PKTQ_LOG
		if (prec_cnt) {
			/* get info from next header to give accurate packet length */
			d11actxh_t* vhtHdr = NULL;
			int tsoHdrSize = wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
			int hdrSize;
			if (vhtHdr->PktInfo.MacTxControlLow & htol16(D11AC_TXC_HDR_FMT_SHORT)) {
				hdrSize = D11AC_TXH_SHORT_LEN;
			} else {
				hdrSize = D11AC_TXH_LEN;
			}

			prec_pktlen = pkttotlen(wlc->osh, p) - (tsoHdrSize + hdrSize) -
			              DOT11_LLC_SNAP_HDR_LEN - DOT11_MAC_HDR_LEN -
			              DOT11_QOS_LEN;
		}
#endif /* PKTQ_LOG */
	} /* while (TRUE) */

	WLC_TXFIFO_COMPLETE(wlc, queue, tot_mpdu, tx_time);
#ifdef PKTQ_LOG
	/* cannot distinguish hi or lo prec hereafter - so just standardise lo prec */
	if (scb_ampdu->txq.pktqlog) {
		prec_cnt = wlc_ampdu_pktqlog_cnt(&scb_ampdu->txq, tid, WLC_PRIO_TO_PREC(tid));
	}

	/* we are in 'aqm' complete function so expect at least corerev 40 to get this */
	WLCNTCONDADD(prec_cnt, prec_cnt->airtime, TX_STATUS40_TX_MEDIUM_DELAY(txs));
#endif // endif

	if (send_bar) {
		if  ((MODSUB_POW2(ini->max_seq, bar_seq, SEQNUM_MAX) >= ini->ba_wsize) &&
		     (MODSUB_POW2(bar_seq, (ini->acked_seq), SEQNUM_MAX) < ini->ba_wsize * 2)) {
			send_bar = FALSE;
			WL_AMPDU(("wl%d: %s: skipping BAR %x, max_seq %x, acked_seq %x\n",
				wlc->pub->unit, __FUNCTION__, MODINC_POW2(seq, SEQNUM_MAX),
				ini->max_seq, ini->acked_seq));
		} else if (ini->bar_ackpending) {
			/*
			 * check if no bar with newer seq has been send out due to other condition,
			 * like unexpected tx seq
			 */
			if (!IS_SEQ_ADVANCED(MODINC_POW2(bar_seq, SEQNUM_MAX), ini->barpending_seq))
				send_bar = FALSE;
		}

		if (send_bar && (ini->retry_bar != AMPDU_R_BAR_BLOCKED)) {
			bar_seq = MODINC_POW2(seq, SEQNUM_MAX);
			if (IS_SEQ_ADVANCED (bar_seq, ini->barpending_seq) ||
			    (ini->barpending_seq == SEQNUM_MAX))
				ini->barpending_seq = bar_seq;
			wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
		}
	}

	if (nlost) {
		AMPDUSCBCNTADD(scb_ampdu->cnt.txlost, nlost);
		WLCNTADD(ampdu_tx->cnt->txlost, nlost);
		WLCNTADD(wlc->pub->_cnt->txlost, nlost);
		WL_AMPDU_ERR(("wl%d aqm_txs: nlost %d send_bar %d "
			     "vht %d mcs[0-3] %04x %04x %04x %04x\n",
			      wlc->pub->unit, nlost, send_bar,
			      is_vht[0], rs_txs.txrspec[0], rs_txs.txrspec[1], rs_txs.txrspec[2],
			      rs_txs.txrspec[3]));
		WL_AMPDU_ERR(("raw txstatus %04X %04X %04X | s3-5 %08X %08X %08X | "
			"%08X %08X | s8 %08X\n",
			txs->status.raw_bits, txs->frameid, txs->sequence,
			txs->status.s3, txs->status.s4, txs->status.s5,
			txs->status.ack_map1, txs->status.ack_map2, txs->status.s8));
	}

#ifdef PSPRETEND
	if (BSSCFG_AP(bsscfg) || BSSCFG_IBSS(bsscfg)) {
		wlc_apps_process_pspretend_status(wlc, scb, pps_recvd_ack);
	}
#endif /* PSPRETEND */

	/* bump up the start seq to move the window */
	wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);

	WL_AMPDU_TX(("wl%d: %s: ncons %d tot_mpdus %d start_seq 0x%04x\n",
		wlc->pub->unit, __FUNCTION__, ncons, tot_mpdu, start_seq));

#ifdef STA
	/* PM state change */
	wlc_update_pmstate(bsscfg, (uint)(wlc_txs_alias_to_old_fmt(wlc, &(txs->status))));
#endif /* STA */

#ifdef PKTQ_LOG
	WLCNTCONDADD(prec_cnt, prec_cnt->rtsfail, txs->status.rts_tx_cnt - txs->status.cts_rx_cnt);
	WLCNTCONDADD(actq_cnt, actq_cnt->rtsfail, txs->status.rts_tx_cnt - txs->status.cts_rx_cnt);
	SCB_BS_DATA_CONDADD(bs_data_counters,
		rtsfail, txs->status.rts_tx_cnt - txs->status.cts_rx_cnt);
#endif // endif

	/* Per-scb statitstics: scb must be valid here */
	WLCNTADD(wlc->pub->_cnt->txfail, nlost);
	WLCNTSCBADD(scb->scb_stats.tx_pkts_retry_exhausted, nlost);
	WLCNTSCBADD(scb->scb_stats.tx_failures, nlost);
#ifdef WL11K
	wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_fail), nlost);
#endif // endif

	/* Update rs_txs except for txrspec which is already updated */
	rs_txs.ncons = (ncons > 0xff) ? 0xff : (uint8)ncons;
	rs_txs.nlost = nlost;
	rs_txs.txrts_cnt = txs->status.rts_tx_cnt;
	rs_txs.rxcts_cnt = txs->status.cts_rx_cnt;
	rs_txs.ack_map1 = txs->status.ack_map1;
	rs_txs.ack_map2 = txs->status.ack_map2;
	rs_txs.frameid = txs->frameid;
	/* zero for aqm */
	rs_txs.antselid = 0;
	rs_txs.ac = ac;

	/* MU txstatus uses RT1~RT3 for other purposes */
	if (fix_rate && !txs_mu) {
		/* if using fix rate, retrying 64 mpdus >=4 times can overflow 8-bit cnt.
		 * So ucode treats fix rate specially.
		 */
		uint32    retry_count;
#ifdef PKTQ_LOG
		uint32    txrate_succ = 0;
#endif // endif
		rs_txs.tx_cnt[0]     = txs->status.s3 & 0xffff;
		rs_txs.txsucc_cnt[0] = (txs->status.s3 >> 16) & 0xffff;
		retry_count = rs_txs.tx_cnt[0] - rs_txs.txsucc_cnt[0];
		BCM_REFERENCE(retry_count);

		WLCNTADD(wlc->pub->_cnt->txfrag, rs_txs.txsucc_cnt[0]);
		WLCNTADD(wlc->pub->_cnt->txfrmsnt, rs_txs.txsucc_cnt[0]);
		WLCNTADD(wlc->pub->_cnt->txretrans, retry_count);
		WLCNTSCBADD(scb->scb_stats.tx_pkts_total, rs_txs.txsucc_cnt[0]);
		WLCNTSCBADD(scb->scb_stats.tx_pkts_retries, retry_count);
#ifdef WLATF
		scb_ampdu->txretry_pkt += retry_count;
		scb_ampdu->tot_pkt += rs_txs.tx_cnt[0];
#endif // endif
#ifdef WL11K
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_tx), rs_txs.txsucc_cnt[0]);
#endif // endif

		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_pkts_total[ini->tid],
			rs_txs.txsucc_cnt[0]);
#ifdef PKTQ_LOG
		if (rs_txs.txsucc_cnt[0]) {
			txrate_succ = mcs_txrate[0] * rs_txs.txsucc_cnt[0];
		}

		WLCNTCONDADD(prec_cnt, prec_cnt->retry, retry_count);
		WLCNTCONDADD(actq_cnt, actq_cnt->retry, retry_count);
		SCB_BS_DATA_CONDADD(bs_data_counters, retry, retry_count);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_main, mcs_txrate[0] * rs_txs.tx_cnt[0]);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_main, mcs_txrate[0] * rs_txs.tx_cnt[0]);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_main,
			mcs_txrate[0] * rs_txs.tx_cnt[0]);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_succ, txrate_succ);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_succ, txrate_succ);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_succ, txrate_succ);
#endif // endif
	} else {
		uint32    succ_count;
		uint32    try_count;
		uint32    retry_count;
#ifdef PKTQ_LOG
		uint32    txrate_succ = 0;
#endif // endif
		rs_txs.tx_cnt[0]     = txs->status.s3 & 0xff;
		rs_txs.txsucc_cnt[0] = (txs->status.s3 >> 8) & 0xff;
		if (!txs_mu) {
			rs_txs.tx_cnt[1]     = (txs->status.s3 >> 16) & 0xff;
			rs_txs.txsucc_cnt[1] = (txs->status.s3 >> 24) & 0xff;
			rs_txs.tx_cnt[2]     = (txs->status.s4 >>  0) & 0xff;
			rs_txs.txsucc_cnt[2] = (txs->status.s4 >>  8) & 0xff;
			rs_txs.tx_cnt[3]     = (txs->status.s4 >> 16) & 0xff;
			rs_txs.txsucc_cnt[3] = (txs->status.s4 >> 24) & 0xff;
		}
		succ_count = rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1] +
			rs_txs.txsucc_cnt[2] + rs_txs.txsucc_cnt[3];
		try_count = rs_txs.tx_cnt[0] + rs_txs.tx_cnt[1] +
			rs_txs.tx_cnt[2] + rs_txs.tx_cnt[3];
		retry_count = try_count - succ_count;
		BCM_REFERENCE(succ_count);
		BCM_REFERENCE(try_count);
		BCM_REFERENCE(retry_count);

		WLCNTADD(wlc->pub->_cnt->txfrag, succ_count);
		WLCNTADD(wlc->pub->_cnt->txfrmsnt, succ_count);
		WLCNTADD(wlc->pub->_cnt->txretrans, retry_count);
		WLCNTSCBADD(scb->scb_stats.tx_pkts_total, succ_count);
		WLCNTSCBADD(scb->scb_stats.tx_pkts_retries, retry_count);
#ifdef WLATF
		scb_ampdu->txretry_pkt += retry_count;
		scb_ampdu->tot_pkt += try_count;
#endif // endif
#ifdef WL11K
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_tx), succ_count);
#endif // endif

		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_pkts_total[ini->tid], succ_count);
#ifdef PKTQ_LOG
		if (succ_count) {
			txrate_succ = mcs_txrate[0] * rs_txs.txsucc_cnt[0] +
			              mcs_txrate[1] * rs_txs.txsucc_cnt[1] +
			              mcs_txrate[2] * rs_txs.txsucc_cnt[2] +
			              mcs_txrate[3] * rs_txs.txsucc_cnt[3];
		}

		WLCNTCONDADD(prec_cnt, prec_cnt->retry, retry_count);
		WLCNTCONDADD(actq_cnt, actq_cnt->retry, retry_count);
		SCB_BS_DATA_CONDADD(bs_data_counters, retry, retry_count);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_main, mcs_txrate[0] * try_count);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_main, mcs_txrate[0] * try_count);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_main, mcs_txrate[0] * try_count);
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_succ, txrate_succ);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_succ, txrate_succ);
		SCB_BS_DATA_CONDADD(bs_data_counters, txrate_succ, txrate_succ);
#endif /* PKTQ_LOG */
	}
#ifdef BCMDBG
	for (k = 0; k < rnum; k++) {
		if (rs_txs.txsucc_cnt[k] > rs_txs.tx_cnt[k]) {
			WL_AMPDU_ERR(("%s: rs_txs.txsucc_cnt[%d] > rs_txs.tx_cnt[%d]: %d > %d\n",
				__FUNCTION__, k, k, rs_txs.txsucc_cnt[k], rs_txs.tx_cnt[k]));
			WL_AMPDU_ERR(("ncons %d raw txstatus %08X | %04X %04X | %08X %08X || "
				 "%08X | %08X %08X\n", ncons,
				  txs->status.raw_bits, txs->sequence, txs->phyerr,
				  txs->status.s3, txs->status.s4, txs->status.s5,
				  txs->status.ack_map1, txs->status.ack_map2));
		}
	}
#endif // endif
#if defined(BCMDBG) || (defined(WLTEST) && !defined(WLTEST_DISABLED)) || \
	defined(BCMDBG_AMPDU)
	for (k = 0; k < rnum; k++) {
		uint8 ht_mcs = rs_txs.txrspec[k] & RSPEC_RATE_MASK;
		if (ampdu_tx->amdbg) {
			if (!is_vht[k])  {
				WLCNTADD(ampdu_tx->amdbg->txmcs[MCS2IDX(ht_mcs)], rs_txs.tx_cnt[k]);
				WLCNTADD(ampdu_tx->amdbg->txmcs_succ[MCS2IDX(ht_mcs)],
					rs_txs.txsucc_cnt[k]);
			}
#ifdef WL11AC
			else {
				WLCNTADD(ampdu_tx->amdbg->txvht[vht[k]], rs_txs.tx_cnt[k]);
				WLCNTADD(ampdu_tx->amdbg->txvht_succ[vht[k]], rs_txs.txsucc_cnt[k]);
			}
#endif // endif
			if (rs_txs.txrspec[k] & RSPEC_SHORT_GI) {
				WLCNTADD(ampdu_tx->cnt->u1.txmpdu_sgi, rs_txs.tx_cnt[k]);
				if (!is_vht[k])
					WLCNTADD(ampdu_tx->amdbg->txmcssgi[MCS2IDX(ht_mcs)],
					rs_txs.tx_cnt[k]);
#ifdef WL11AC
				else
					WLCNTADD(ampdu_tx->amdbg->txvhtsgi[vht[k]],
						rs_txs.tx_cnt[k]);
#endif // endif
			}

			if (is_stbc[k]) {
				WLCNTADD(ampdu_tx->cnt->u2.txmpdu_stbc, rs_txs.tx_cnt[k]);
				if (!is_vht[k])
					WLCNTADD(ampdu_tx->amdbg->txmcsstbc[MCS2IDX(ht_mcs)],
						rs_txs.tx_cnt[k]);
#ifdef WL11AC
				else
					WLCNTADD(ampdu_tx->amdbg->txvhtstbc[vht[k]],
						rs_txs.tx_cnt[k]);
#endif // endif
			}
		}
#ifdef WLPKTDLYSTAT
		if (!is_vht[k]) {
			WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(ht_mcs)], rs_txs.tx_cnt[k]);
			WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(ht_mcs)],
				rs_txs.txsucc_cnt[k]);
		}
#ifdef WL11AC
		else {
			WLCNTADD(ampdu_tx->cnt->txmpdu_vht_cnt[vht[k]], rs_txs.tx_cnt[k]);
			WLCNTADD(ampdu_tx->cnt->txmpdu_vht_succ_cnt[vht[k]], rs_txs.txsucc_cnt[k]);
		}
#endif /* WL11AC */
#endif /* WLPKTDLYSTAT */
	}
#endif /* BCMDBG || (WLTEST && !WLTEST_DISABLED) */

#if defined(WL_MU_TX) && defined(WLCNT)
	if (MU_TX_ENAB(wlc) && SCB_MU(scb) && is_mu &&
		(supr_status == TX_STATUS_SUPR_NONE) && (txs->phyerr == 0)) {
		wlc_mutx_update_txcounters(wlc->mutx, scb,
			txs_mu, txs, &rs_txs, rnum);
	}
#endif /* WL_MU_TX && WLCNT */

	if (update_rate && !txs_mu) { /* No rate update for mu txstatus */
		uint fbr_cnt = 0, fbr_succ = 0;

		if (rs_txs.tx_cnt[0] > 0xff) rs_txs.tx_cnt[0] = 0xff;
		if (rs_txs.txsucc_cnt[0] > 0xff) rs_txs.txsucc_cnt[0] = 0xff;

		for (k = 1; k < rnum; k++) {
			fbr_cnt += rs_txs.tx_cnt[k];
			fbr_succ += rs_txs.txsucc_cnt[k];
		}

		if (rs_txs.tx_cnt[0] == 0 && fbr_cnt == 0) {
			/*
			 * XXX 4360 TO DO: the frames must've failed because of rts failure
			 * 1. we could feedback rate sel <rts_txcnt> and <cts_rxcnt>.
			 * 2. once we put 4 rates in, we should also put tx_fbr2/tx_fbr3 in.
			 */
			rs_txs.tx_cnt[0] = (ncons > 0xff) ? 0xff : (uint8)ncons;
			rs_txs.txsucc_cnt[0] = 0;
			rs_txs.tx_cnt[1] = 0;
			rs_txs.txsucc_cnt[1] = 0;
#ifdef PSPRETEND
			if (pps_retry) {
				WL_PS(("wl%d.%d: "MACF" pps retry with only RTS failure\n",
				        wlc->pub->unit,
				        WLC_BSSCFG_IDX(bsscfg), ETHER_TO_MACF(scb->ea)));
			}
#endif // endif
		} else {
#ifdef PSPRETEND
			/* we have done successive attempts to send the packet and it is
			 * not working, despite RTS going through. Suspect that the rate
			 * info is wrong so reset it
			 */
			/* XXX if we were reprogramming tx header between ps pretend transmissions,
			 * we would do this rate reset before reaching the retry limit
			 */
			if (SCB_PS_PRETEND(scb) && !SCB_PS_PRETEND_THRESHOLD(scb) &&
				(scb->ps_pretend_succ_count >= bsscfg->ps_pretend_retry_limit)) {

				WL_PS(("wl%d.%d: "MACF" ps pretend rate adaptation (count %d)\n",
				        wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
				        ETHER_TO_MACF(scb->ea),
				        scb->ps_pretend_succ_count));
				 wlc_scb_ratesel_rfbr(wlc->wrsi, scb, ac);
			}
#endif /* PSPRETEND */
		}
		/* notify rate selection */
		wlc_scb_ratesel_upd_txs_ampdu(wlc->wrsi, scb, &rs_txs, txs, FALSE);
	}

	if (!supr_status) {
		/* if primary succeeds, no restrict */
		wlc_scb_restrict_txstatus(scb, rs_txs.txsucc_cnt[0] != 0);
	}
} /* wlc_ampdu_dotxstatus_aqm_complete */

#endif /* WLAMPDU_MAC */

static void BCMFASTPATH
wlc_ampdu_dotxstatus_complete(ampdu_tx_info_t *ampdu_tx, struct scb *scb, void *p, tx_status_t *txs,
	uint32 s1, uint32 s2, uint32 s3, uint32 s4)
{
	scb_ampdu_tx_t *scb_ampdu;
	wlc_info_t *wlc = ampdu_tx->wlc;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	scb_ampdu_tid_ini_t *ini;
	uint8 bitmap[8], queue, tid, requeue = 0;
	d11txh_t *txh = NULL;
	uint8 *plcp = NULL;
	struct dot11_header *h;
	uint16 seq, start_seq = 0, bindex, indx, mcl;
	uint8 mcs = 0;
	bool is_sgi = FALSE, is40 = FALSE;
	bool ba_recd = FALSE, ack_recd = FALSE, send_bar = FALSE;
	uint8 suc_mpdu = 0, tot_mpdu = 0;
	uint supr_status;
	bool update_rate = TRUE, retry = TRUE;
	int txretry = -1;
	uint16 mimoantsel = 0;
	uint8 antselid = 0;
	uint8 retry_limit, rr_retry_limit;
	wlc_bsscfg_t *bsscfg;
	bool pkt_freed = FALSE;
#if defined(TXQ_MUX)
	int tx_time = 0;
#endif /* TXQ_MUX */
#if defined(WLPKTDLYSTAT) || defined(PKTQ_LOG)
	uint8 mcs_fbr = 0;
#endif /* WLPKTDLYSTAT PKTQ_LOG */
	uint8 ac;
#if defined(WLPKTDLYSTAT) || defined(WL11K)
	uint32 delay, now;
#endif // endif
#ifdef WLPKTDLYSTAT
	scb_delay_stats_t *delay_stats;
	uint tr;
#endif /* WLPKTDLYSTAT */
#ifdef PKTQ_LOG
	int prec_index;

	pktq_counters_t *prec_cnt = NULL;
	pktq_counters_t *actq_cnt = NULL;
	uint32           prec_pktlen;
	uint32 tot_len = 0, pktlen = 0;
	uint32 txrate_succ = 0, current_rate, airtime = 0;
	uint bw;
	ratespec_t rspec;
	uint ctl_rspec;
	uint flg;
	uint32 rts_sifs, cts_sifs, ba_sifs;
	uint32 ampdu_dur;
#endif /* PKTQ_LOG */
#if defined(SCB_BS_DATA)
	wlc_bs_data_counters_t *bs_data_counters = NULL;
#endif /* SCB_BS_DATA */

#ifdef WLAMPDU_MAC
	/* used for ucode/hw agg */
	int pdu_count = 0;
	uint16 first_frameid = 0xffff, fid, ncons = 0;
#endif /* WLAMPDU_MAC */
	ratesel_txs_t rs_txs;

#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
	uint8 hole[AMPDU_MAX_MPDU];
	uint8 idx = 0;
	bzero(hole, sizeof(hole));
#endif /* defined(BCMDBG) || defined(BCMDBG_AMPDU) */

	bzero(&rs_txs, sizeof(rs_txs));

	ASSERT(wlc);
	ASSERT(p && (WLPKTTAG(p)->flags & WLF_AMPDU_MPDU));

	if (!scb || !SCB_AMPDU(scb)) {
		if (!scb)
			WL_ERROR(("wl%d: %s: scb is null\n",
				wlc->pub->unit, __FUNCTION__));

		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	SCB_BS_DATA_CONDFIND(bs_data_counters, wlc, scb);

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	tid = (uint8)PKTPRIO(p);
	ac = WME_PRIO2AC(tid);
	ASSERT(ac < AC_COUNT);
	ini = scb_ampdu->ini[tid];
	retry_limit = ampdu_tx_cfg->retry_limit_tid[tid];
	rr_retry_limit = ampdu_tx_cfg->rr_retry_limit_tid[tid];

	if (!ini || (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
		wlc_ampdu_free_chain(ampdu_tx, p, txs, FALSE);
		return;
	}

	ASSERT(ini->scb == scb);
	ASSERT(WLPKTTAGSCBGET(p) == scb);

	bzero(bitmap, sizeof(bitmap));
	queue = WLC_TXFID_GET_QUEUE(txs->frameid);
	ASSERT(queue < AC_COUNT);

	update_rate = (WLPKTTAG(p)->flags & WLF_RATE_AUTO) ? TRUE : FALSE;
	supr_status = txs->status.suppr_ind;

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);
	BCM_REFERENCE(bsscfg);

#ifdef PKTQ_LOG
	prec_index = WLAMPDU_PREC_TID2PREC(p, tid);

	if (scb_ampdu->txq.pktqlog) {
		prec_cnt = wlc_ampdu_pktqlog_cnt(&scb_ampdu->txq, tid, prec_index);
	}

	if ((WLC_GET_TXQ(wlc->active_queue))->pktqlog) {
		actq_cnt =
		(WLC_GET_TXQ(wlc->active_queue))->pktqlog->_prec_cnt[prec_index];
	}
#endif /* PKTQ_LOG */

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		WL_AMPDU_HWDBG(("%s: cons %d burst 0x%04x aggfifo1_space %d txfifo_cnt %d\n",
			__FUNCTION__, txs->sequence, (s4 >> 16) & 0xffff, /* s4 is overloaded */
			get_aggfifo_space(ampdu_tx, 1),
			R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
	}
#endif /* WLAMPDU_HW */
	if (AMPDU_HOST_ENAB(wlc->pub)) {
		if (txs->status.was_acked) {
			/*
			 * Underflow status is reused  for BTCX to indicate AMPDU preemption.
			 * This prevents un-necessary rate fallback.
			 */
			if (TX_STATUS_SUPR_UF == supr_status) {
				WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: BT preemption, skip rate "
					     "fallback\n", wlc->pub->unit));
				update_rate = FALSE;
			}

			ASSERT(txs->status.is_intermediate);
			start_seq = txs->sequence >> SEQNUM_SHIFT;
			bitmap[0] = (txs->status.raw_bits & TX_STATUS_BA_BMAP03_MASK) >>
				TX_STATUS_BA_BMAP03_SHIFT;

			ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
			ASSERT(s1 & TX_STATUS_AMPDU);

			bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
			bitmap[1] = (s1 >> 8) & 0xff;
			bitmap[2] = (s1 >> 16) & 0xff;
			bitmap[3] = (s1 >> 24) & 0xff;

			/* remaining 4 bytes in s2 */
			bitmap[4] = s2 & 0xff;
			bitmap[5] = (s2 >> 8) & 0xff;
			bitmap[6] = (s2 >> 16) & 0xff;
			bitmap[7] = (s2 >> 24) & 0xff;

			ba_recd = TRUE;

#if defined(STA) && defined(DBG_BCN_LOSS)
			scb->dbg_bcn.last_tx = wlc->pub->now;
#endif /* defined(STA) && defined(DBG_BCN_LOSS) */

			WL_AMPDU_TX(("%s: Block ack received: start_seq is 0x%x bitmap "
				"%02x %02x %02x %02x %02x %02x %02x %02x\n",
				__FUNCTION__, start_seq, bitmap[0], bitmap[1], bitmap[2],
				bitmap[3], bitmap[4], bitmap[5], bitmap[6], bitmap[7]));

		}
	}
#ifdef WLAMPDU_MAC
	else {
		/* AMPDU_HW txstatus package */

		ASSERT(txs->status.is_intermediate);
#ifdef BCMDBG
		if (WL_AMPDU_HWTXS_ON()) {
			wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
		}
#endif /* BCMDBG */

		ASSERT(!(s1 & TX_STATUS_INTERMEDIATE));
		start_seq = ini->start_seq;
		ncons = txs->sequence;
		bitmap[0] = (txs->status.raw_bits & TX_STATUS_BA_BMAP03_MASK) >>
			TX_STATUS_BA_BMAP03_SHIFT;
		bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
		bitmap[1] = (s1 >> 8) & 0xff;
		bitmap[2] = (s1 >> 16) & 0xff;
		bitmap[3] = (s1 >> 24) & 0xff;
		ba_recd = bitmap[0] || bitmap[1] || bitmap[2] || bitmap[3];
#ifdef WLAMPDU_HW
		if (AMPDU_HW_ENAB(wlc->pub)) {
			/* 3rd txstatus */
			bitmap[4] = (s3 >> 16) & 0xff;
			bitmap[5] = (s3 >> 24) & 0xff;
			bitmap[6] = s4 & 0xff;
			bitmap[7] = (s4 >> 8) & 0xff;
			ba_recd = ba_recd || bitmap[4] || bitmap[5] || bitmap[6] || bitmap[7];
		}
#endif /* WLAMPDU_HW */
		/* remaining 4 bytes in s2 */
		rs_txs.tx_cnt[0]	= (uint8)(s2 & 0xff);
		rs_txs.txsucc_cnt[0]	= (uint8)((s2 >> 8) & 0xff);
		rs_txs.tx_cnt[1]	= (uint8)((s2 >> 16) & 0xff);
		rs_txs.txsucc_cnt[1]	= (uint8)((s2 >> 24) & 0xff);

		WLCNTINCR(ampdu_tx->cnt->u0.txs);
		WLCNTADD(ampdu_tx->cnt->tx_mrt, rs_txs.tx_cnt[0]);
		WLCNTADD(ampdu_tx->cnt->txsucc_mrt, rs_txs.txsucc_cnt[0]);
		WLCNTADD(ampdu_tx->cnt->tx_fbr, rs_txs.tx_cnt[1]);
		WLCNTADD(ampdu_tx->cnt->txsucc_fbr, rs_txs.txsucc_cnt[1]);
		WLCNTADD(wlc->pub->_cnt->txfrag, rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
		WLCNTADD(wlc->pub->_cnt->txfrmsnt, rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
		WLCNTADD(wlc->pub->_cnt->txretrans, (rs_txs.tx_cnt[0] - rs_txs.txsucc_cnt[0]) +
			(rs_txs.tx_cnt[1] - rs_txs.txsucc_cnt[1]));
		WLCNTSCBADD(scb->scb_stats.tx_pkts_total,
			rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);
		WLCNTSCBADD(scb->scb_stats.tx_pkts_retries,
			(rs_txs.tx_cnt[0] - rs_txs.txsucc_cnt[0]) +
			(rs_txs.tx_cnt[1] - rs_txs.txsucc_cnt[1]));
#ifdef WLSCB_HISTO
		WLSCB_HISTO_TX_IF_HTVHT(scb, rs_txs.txrspec[0], rs_txs.txsucc_cnt[0] <<
				WLPKTTAG_AMSDU(p));
		WLSCB_HISTO_TX_IF_HTVHT(scb, rs_txs.txrspec[1], rs_txs.txsucc_cnt[1] <<
				WLPKTTAG_AMSDU(p));
#endif /* WLSCB_HISTO */
#ifdef WL11K
		wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_tx),
			(rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]));
#endif // endif

		WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_pkts_total[ini->tid],
			rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1]);

		if (!(txs->status.was_acked)) {

			WL_ERROR(("Status: "));
			if ((txs->status.was_acked) == 0)
				WL_ERROR(("Not ACK_RCV "));
			WL_ERROR(("\n"));
			WL_ERROR(("Total attempts %d succ %d\n", rs_txs.tx_cnt[0],
				rs_txs.txsucc_cnt[0]));
		}
	}
#endif /* WLAMPDU_MAC */

	if (!ba_recd || AMPDU_MAC_ENAB(wlc->pub)) {
		if (!ba_recd) {
			WLCNTINCR(ampdu_tx->cnt->noba);
			AMPDUSCBCNTINCR(scb_ampdu->cnt.noba);
			if (AMPDU_MAC_ENAB(wlc->pub)) {
				WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: error txstatus 0x%x\n",
					wlc->pub->unit, txs->status.raw_bits));
			}
		}
		if (supr_status != TX_STATUS_SUPR_NONE) {
#ifdef PKTQ_LOG
			WLCNTCONDINCR(prec_cnt, prec_cnt->suppress);
			WLCNTCONDINCR(actq_cnt, actq_cnt->suppress);
#endif /* PKTQ_LOG */

			update_rate = FALSE;
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU)
			ampdu_tx->amdbg->supr_reason[supr_status]++;
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(BCMDBG_AMPDU) */
			WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: supr_status 0x%x\n",
				wlc->pub->unit, supr_status));
			/* no need to retry for badch; will fail again */
			if (supr_status == TX_STATUS_SUPR_BADCH) {
				retry = FALSE;
				WLCNTINCR(wlc->pub->_cnt->txchanrej);
			} else if (supr_status == TX_STATUS_SUPR_EXPTIME) {

				WLCNTINCR(wlc->pub->_cnt->txexptime);
#ifdef WL11K
				wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_exp), 1);
#endif // endif

				/* Interference detected */
				if (wlc->rfaware_lifetime)
					wlc_exptime_start(wlc);
			/* TX underflow : try tuning pre-loading or ampdu size */
			} else if (supr_status == TX_STATUS_SUPR_FRAG) {
				/* if there were underflows, but pre-loading is not active,
				   notify rate adaptation.
				*/
#ifdef AMPDU_NON_AQM
				if (wlc_ffpld_check_txfunfl(wlc, prio2fifo[tid]) > 0) {
#ifdef WLC_HIGH_ONLY
					/* With BMAC, TX Underflows should not happen */
					WL_ERROR(("wl%d: BMAC TX Underflow?", wlc->pub->unit));
#endif /* WLC_HIGH_ONLY */
				}
#endif /* AMPDU_NON_AQM */
			}
		} else if (txs->phyerr) {
			update_rate = FALSE;
			WLCNTINCR(wlc->pub->_cnt->txphyerr);
			WL_ERROR(("wl%d: %s: tx phy error (0x%x)\n",
				wlc->pub->unit, __FUNCTION__, txs->phyerr));

#ifdef BCMDBG
			if (WL_ERROR_ON()) {
				prpkt("txpkt (AMPDU)", wlc->osh, p);
				if (AMPDU_HOST_ENAB(wlc->pub)) {
					wlc_print_txdesc(wlc, (wlc_txd_t*)PKTDATA(wlc->osh, p));
					wlc_print_txstatus(wlc, txs);
				}
#ifdef WLAMPDU_MAC
				else {
					wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
					wlc_dump_aggfifo(wlc, NULL);
				}
#endif /* WLAMPDU_MAC */
			}
#endif /* BCMDBG */
		}
	}

	/* Check if interference is still there */
	if (wlc->rfaware_lifetime && wlc->exptime_cnt && (supr_status != TX_STATUS_SUPR_EXPTIME))
		wlc_exptime_check_end(wlc);

#if defined(WLPKTDLYSTAT) || defined(WL11K)
	/* Get the current time */
	now = WLC_GET_CURR_TIME(wlc);
#endif // endif

	/* loop through all pkts and retry if not acked */
	while (p) {
		ASSERT(WLPKTTAG(p)->flags & WLF_AMPDU_MPDU);

#if defined(TXQ_MUX)
		tx_time += WLPKTTIME(p);
#endif /* TXQ_MUX */

		txh = (d11txh_t *)PKTDATA(wlc->osh, p);
		mcl = ltoh16(txh->MacTxControlLow);
		plcp = (uint8 *)(txh + 1);
		h = (struct dot11_header*)(plcp + D11_PHY_HDR_LEN);
		seq = ltoh16(h->seq) >> SEQNUM_SHIFT;
		pkt_freed = FALSE;
#ifdef WLAMPDU_MAC
		fid = ltoh16(txh->TxFrameID);
		BCM_REFERENCE(fid);
#endif /* WLAMPDU_MAC */
#ifdef WLTAF
		if (!wlc_taf_handle_star(wlc->taf_handle, ini->tid,
		                         WLPKTTAG(p)->pktinfo.taf.units,
		                         WLPKTTAG(p)->pktinfo.taf.index))
#endif // endif
		{
			/* Complete ATF packet here, PS, un-acked packets are requeued */
			wlc_ampdu_dec_bytes_inflight(ini, p);
		}
#ifdef PKTQ_LOG
		pktlen = pkttotlen(wlc->osh, p) - sizeof(d11txh_t);
		prec_pktlen = pktlen - DOT11_LLC_SNAP_HDR_LEN - DOT11_MAC_HDR_LEN - DOT11_QOS_LEN;
		tot_len += pktlen;
#endif /* PKTQ_LOG */
		if (tot_mpdu == 0) {
			mcs = plcp[0] & MIMO_PLCP_MCS_MASK;
			mimoantsel = ltoh16(txh->ABI_MimoAntSel);
			is_sgi = PLCP3_ISSGI(plcp[3]);
			is40 = (plcp[0] & MIMO_PLCP_40MHZ) ? 1 : 0;

			if (AMPDU_HOST_ENAB(wlc->pub)) {
				/* this is only needed for host agg */
#if defined(WLPKTDLYSTAT) || defined(PKTQ_LOG)
				mcs_fbr = txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK;
#endif /* WLPKTDLYSTAT PKTQ_LOG */
			}
#ifdef WLAMPDU_MAC
			else {
				first_frameid = ltoh16(txh->TxFrameID);
#if defined(BCMDBG) || defined(WLTEST) || defined(BCMDBG_AMPDU)
				if (PLCP3_ISSGI(plcp[3])) {
					WLCNTADD(ampdu_tx->cnt->u1.txmpdu_sgi, rs_txs.tx_cnt[0]);
					if (ampdu_tx->amdbg)
						WLCNTADD(ampdu_tx->amdbg->txmcssgi[MCS2IDX(mcs)],
							rs_txs.tx_cnt[0]);
				}
				if (PLCP3_ISSTBC(plcp[3])) {
					WLCNTADD(ampdu_tx->cnt->u2.txmpdu_stbc, rs_txs.tx_cnt[0]);
					if (ampdu_tx->amdbg)
						WLCNTADD(ampdu_tx->amdbg->txmcsstbc[MCS2IDX(mcs)],
							rs_txs.tx_cnt[0]);
				}
				if (ampdu_tx->amdbg)
					WLCNTADD(ampdu_tx->amdbg->txmcs[MCS2IDX(mcs)],
						rs_txs.tx_cnt[0]);
#ifdef WLPKTDLYSTAT
				WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(mcs)], rs_txs.tx_cnt[0]);
				WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs)],
					rs_txs.txsucc_cnt[0]);
#endif /* WLPKTDLYSTAT */
				if ((ltoh16(txh->XtraFrameTypes) & 0x3) != 0) {
#ifdef WLCNT
					uint8 mcs_fbr;
					/* if not fbr_cck */
					mcs_fbr = txh->FragPLCPFallback[0] & MIMO_PLCP_MCS_MASK;
#endif /* WLCNT */
					if (ampdu_tx->amdbg)
						WLCNTADD(ampdu_tx->amdbg->txmcs[MCS2IDX(mcs_fbr)],
							rs_txs.tx_cnt[1]);
#ifdef WLPKTDLYSTAT
					WLCNTADD(ampdu_tx->cnt->txmpdu_cnt[MCS2IDX(mcs_fbr)],
						rs_txs.tx_cnt[1]);
					WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs_fbr)],
						rs_txs.txsucc_cnt[1]);
#endif /* WLPKTDLYSTAT */
					if (PLCP3_ISSGI(txh->FragPLCPFallback[3])) {
						WLCNTADD(ampdu_tx->cnt->u1.txmpdu_sgi,
							rs_txs.tx_cnt[1]);
						if (ampdu_tx->amdbg)
							WLCNTADD(ampdu_tx->amdbg->txmcssgi
								[MCS2IDX(mcs_fbr)],
								rs_txs.tx_cnt[1]);
					}
					if (PLCP3_ISSTBC(txh->FragPLCPFallback[3])) {
						WLCNTADD(ampdu_tx->cnt->u2.txmpdu_stbc,
							rs_txs.tx_cnt[1]);
						if (ampdu_tx->amdbg)
						   WLCNTADD(ampdu_tx->amdbg->txmcsstbc
							[MCS2IDX(mcs_fbr)], rs_txs.tx_cnt[1]);
					}
				}
#endif /* BCMDBG || WLTEST */
			}
#endif /* WLAMPDU_MAC */
		}

		indx = TX_SEQ_TO_INDEX(seq);
		if (AMPDU_HOST_ENAB(wlc->pub)) {
			txretry = ini->txretry[indx];
		}

		if (MODSUB_POW2(seq, ini->start_seq, SEQNUM_MAX) >= ini->ba_wsize) {
			WL_ERROR(("wl%d: %s: unexpected completion: seq 0x%x, "
				"start seq 0x%x\n",
				wlc->pub->unit, __FUNCTION__, seq, ini->start_seq));
			pkt_freed = TRUE;
#ifdef BCMDBG
#ifdef WLAMPDU_MAC
			wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
#endif // endif
#endif /* BCMDBG */
			goto nextp;
		}

		if (!isset(ini->ackpending, indx)) {
			WL_ERROR(("wl%d: %s: seq 0x%x\n",
				wlc->pub->unit, __FUNCTION__, seq));
			ampdu_dump_ini(ini);
			ASSERT(isset(ini->ackpending, indx));
		}

		ack_recd = FALSE;
		if (ba_recd || AMPDU_MAC_ENAB(wlc->pub)) {
			if (AMPDU_MAC_ENAB(wlc->pub))
				/* use position bitmap */
				bindex = tot_mpdu;
			else
				bindex = MODSUB_POW2(seq, start_seq, SEQNUM_MAX);

			WL_AMPDU_TX(("%s: tid %d seq is 0x%x, start_seq is 0x%x, "
			          "bindex is %d set %d, txretry %d, index %d\n",
			          __FUNCTION__, tid, seq, start_seq, bindex,
			          isset(bitmap, bindex), txretry, indx));

			/* if acked then clear bit and free packet */
			if ((bindex < AMPDU_BA_BITS_IN_BITMAP) && isset(bitmap, bindex)) {
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					if (isset(ini->barpending, indx)) {
						clrbit(ini->barpending, indx);
					}
					ini->txretry[indx] = 0;
				}
#if defined(BCMCCX) && defined(CCX_SDK)
				if (IHV_ENAB(wlc->ccx) &&
				    BSSCFG_STA(bsscfg) && bsscfg->BSS &&
					wlc->ccx->frame_log)
					wlc_ccx_log_tx_frame(wlc->ccx, (uint8*)h,
						PKTLEN(wlc->osh, p) - sizeof(d11txh_t) -
						D11_PHY_HDR_LEN, TX_STATUS_ACK_RCV, FALSE, TRUE);
				/* call any matching pkt callbacks */
				if (WLPKTTAG(p)->flags & WLF_IHV_TX_PKT) {
					/* additional information for IHV pkt callback */
					wlc->ccx->ihv_txpkt = p;
					wlc->ccx->ihv_txpkt_sent = TRUE;
					wlc->ccx->ihv_txpkt_max_retries = FALSE;
				}
#endif /* BCMCCX && CCX_SDK */
				wlc_pcb_fn_invoke(wlc->pcb, p, TX_STATUS_ACK_RCV);

#ifdef WLTXMONITOR
				if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub))
					wlc_tx_monitor(wlc, txh, txs, p, NULL);
#endif /* WLTXMONITOR */
				pkt_freed = TRUE;
				ack_recd = TRUE;
#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->acked);
				WLCNTCONDINCR(actq_cnt, actq_cnt->acked);
				SCB_BS_DATA_CONDINCR(bs_data_counters, acked);
				WLCNTCONDADD(prec_cnt, prec_cnt->throughput, prec_pktlen);
				WLCNTCONDADD(actq_cnt, actq_cnt->throughput, prec_pktlen);
				SCB_BS_DATA_CONDADD(bs_data_counters, throughput, prec_pktlen);
#endif /* PKTQ_LOG */
				WLCNTSCBADD(scb_ampdu->ampdu_scb_stats->tx_bytes_total[ini->tid],
					prec_pktlen);
#ifdef BCMPCIEDEV
				WLCNTSCBINCR(scb->flr_txpkt[ac]);
#endif // endif
				suc_mpdu++;
			}
		}
		/* either retransmit or send bar if ack not recd */
		if (!ack_recd) {
			bool free_pkt = FALSE;
			bool relist = FALSE;

#if defined(PSPRETEND)
			if (BSSCFG_AP(bsscfg)) {

				int32 max_ps_retry = (int32)retry_limit +
				                     (int32)bsscfg->ps_pretend_retry_limit;

				if (max_ps_retry > (uint8)(-1)) {
					max_ps_retry = (uint8)(-1);
				}

				relist = (((txretry >= (int32)retry_limit) &&
				         (txretry < max_ps_retry)) ||
				         SCB_PS_PRETEND_THRESHOLD(scb)) &&
				         !SCB_PS_PRETEND_NORMALPS(scb);
			}
#endif /* PSPRETEND */

			if (AMPDU_HOST_ENAB(wlc->pub) &&
			    retry) {

				if (AP_ENAB(wlc->pub) &&
				    supr_status == TX_STATUS_SUPR_PMQ) {
					/* last_frag TRUE as fragmentation not allowed for AMPDU */
					free_pkt = wlc_apps_suppr_frame_enq(wlc, p, txs, TRUE);
				}
				else if (P2P_ABS_SUPR(wlc, supr_status) &&
				         BSS_TX_SUPR_ENAB(bsscfg)) {
					/* This is possible if we got a packet suppression
					 * before getting ABS interrupt
					 */
					if (!BSS_TX_SUPR(bsscfg) &&
#ifdef WLP2P
					    (wlc_p2p_noa_valid(wlc->p2p, bsscfg) ||
					     wlc_p2p_ops_valid(wlc->p2p, bsscfg)) &&
#endif // endif
					    TRUE)
						wlc_bsscfg_tx_stop(bsscfg);
					if (BSSCFG_AP(bsscfg) &&
					    SCB_ASSOCIATED(scb) && SCB_P2P(scb))
						wlc_apps_scb_tx_block(wlc, scb, 1, TRUE);
					/* With Proptxstatus enabled in dongle and host,
					 * pkts sent by host will not come here as retry is set
					 * to FALSE by wlc_should_retry_suppressed_pkt().
					 * With Proptxstatus disabled in dongle or not active
					 * in host, all the packets will come here and
					 * need to be re-enqueued.
					 */
					free_pkt = wlc_pkt_abs_supr_enq(wlc, scb, p);
				}
				else if ((txretry < (int)retry_limit) || relist) {

					/* Set retry bit */
					h->fc |= htol16(FC_RETRY);

					ini->txretry[indx]++;
					ASSERT(ini->retry_cnt < ampdu_tx_cfg->ba_max_tx_wsize);
					ASSERT(ini->retry_seq[ini->retry_tail] == 0);
					ini->retry_seq[ini->retry_tail] = seq;
					ini->retry_tail = NEXT_TX_INDEX(ini->retry_tail);
					ini->retry_cnt++;

#ifdef PKTQ_LOG
					WLCNTCONDINCR(prec_cnt, prec_cnt->retry);
					WLCNTCONDINCR(actq_cnt, actq_cnt->retry);
					SCB_BS_DATA_CONDINCR(bs_data_counters, retry);
#endif /* PKTQ_LOG */

#if defined(PSPRETEND)
					if (relist) {

						/* not using pmq for ps pretend, so indicate
						 * no block flag
						 */
						wlc_apps_scb_pspretend_on(wlc, scb,
						                          PS_PRETEND_NO_BLOCK);
#ifdef PKTQ_LOG
						WLCNTCONDINCR(prec_cnt, prec_cnt->ps_retry);
						WLCNTCONDINCR(actq_cnt, actq_cnt->ps_retry);
#endif /* PKTQ_LOG */
#ifdef WLINTFERSTAT
						wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt,
						                               scb);
#endif  /* WLINTFERSTAT */
					}
#endif /* PSPRETEND */
					SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_HI_PREC(tid));
				}
				else {
					WL_ERROR(("%s: # retries exceeds limit\n", __FUNCTION__));
					free_pkt = TRUE;
				}

				if (free_pkt)
					goto not_retry;

				requeue++;
			} else {
			not_retry:
#ifdef PROP_TXSTATUS
				if (WLFC_GET_REUSESEQ(wlfc_query_mode(wlc->wl)) && supr_status) {
				} else
#endif /* PROP_TXSTATUS */
				{
					setbit(ini->barpending, indx);
					send_bar = TRUE;
				}

#ifdef WLMEDIA_TXFAILEVENT
				wlc_tx_failed_event(wlc, scb, bsscfg, txs, p, 0);
#endif /* WLMEDIA_TXFAILEVENT */

#ifdef WLTXMONITOR
				if (MONITOR_ENAB(wlc) || PROMISC_ENAB(wlc->pub))
					wlc_tx_monitor(wlc, txh, txs, p, NULL);
#endif /* WLTXMONITOR */
#ifdef PKTQ_LOG
				WLCNTCONDINCR(prec_cnt, prec_cnt->retry_drop);
				WLCNTCONDINCR(actq_cnt, actq_cnt->retry_drop);
				SCB_BS_DATA_CONDINCR(bs_data_counters, retry_drop);
#endif /* PKTQ_LOG */
#ifdef WLINTFERSTAT
				/* Count only failed packets and indicate failure if required */
				wlc_trf_mgmt_scb_txfail_detect(wlc->trf_mgmt_ctxt, scb);
#endif /* WLINTFERSTAT */
				WLCNTINCR(wlc->pub->_cnt->txfail);
				WLCNTSCBINCR(scb->scb_stats.tx_pkts_retry_exhausted);
				WLCNTSCBINCR(scb->scb_stats.tx_failures);
#ifdef WL11K
				wlc_rrm_tscm_upd(scb, tid, OFFSETOF(rrm_tscm_t, msdu_fail), 1);
#endif // endif
				pkt_freed = TRUE;
			}
		} else if (AMPDU_MAC_ENAB(wlc->pub)) {
			ba_recd = TRUE;
		}
#ifdef BCMDBG
		/* tot_mpdu may exceeds AMPDU_MAX_MPDU if doing ucode/hw agg */
		if (AMPDU_HOST_ENAB(wlc->pub) && ba_recd && !ack_recd)
			hole[idx]++;
#endif /* BCMDBG */

nextp:
#ifdef BCMDBG
		if ((idx = tot_mpdu) >= AMPDU_MAX_MPDU) {
			WL_ERROR(("%s: idx out-of-bound to array size (%d)\n", __FUNCTION__, idx));
			idx = AMPDU_MAX_MPDU - 1;
		}
#endif /* BCMDBG */
		tot_mpdu++;
#ifdef PROP_TXSTATUS
		if ((pkt_freed || WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub)) &&
			PROP_TXSTATUS_ENAB(wlc->pub)) {
			uint8 wlfc_status = WLFC_D11_STATUS_INTERPRET(ack_recd,
				TXS_SUPR_MAGG_DONE(txs->status.suppr_ind));
			if (!ack_recd && scb &&
				WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) &&
				(wlfc_status == WLFC_CTL_PKTFLAG_D11SUPPRESS)) {
				wlc_suppress_sync_fsm(wlc, scb, p, TRUE);
			}
			wlfc_process_wlhdr_complete_txstatus(wlc->wl, wlfc_status,
				p, txs, FALSE);
		}
#endif // endif
		/* calculate latency and packet loss statistics */
		if (pkt_freed) {
#if defined(WLPKTDLYSTAT) || defined(WL11K)
			/* Ignore wrap-around case and error case */
			if (now > WLPKTTAG(p)->shared.enqtime) {
				delay = now - WLPKTTAG(p)->shared.enqtime;
#ifdef WLPKTDLYSTAT
				delay_stats = scb->delay_stats[ac];
				if (delay_stats) {
					if (AMPDU_HOST_ENAB(ampdu_tx->wlc->pub))
						tr = (txretry >= AMPDU_DEF_RETRY_LIMIT) ?
							AMPDU_DEF_RETRY_LIMIT : txretry;
					else
						tr = 0;

					wlc_delay_stats_upd(delay_stats, delay, tr, ack_recd);
					WL_AMPDU_STAT(("Enq %d retry %d cnt %d: acked %d delay/min"
						"/max/sum %d %d %d %d\n",
						WLPKTTAG(p)->shared.enqtime, tr,
						delay_stats->txmpdu_cnt[tr], ack_recd, delay,
						delay_stats->delay_min, delay_stats->delay_max,
						delay_stats->delay_sum[tr]));
				}
#endif /* WLPKTDLYSTAT */
#ifdef WL11K
				if (ack_recd) {
					wlc_rrm_delay_upd(scb, tid, delay);
				}
#endif // endif
			}
#endif /* WLPKTDLYSTAT || WL11K */
			PKTFREE(wlc->osh, p, TRUE);
		}

		/* break out if last packet of ampdu */
		if (AMPDU_MAC_ENAB(wlc->pub)) {
#ifdef WLAMPDU_MAC
			if (tot_mpdu == ncons) {
				ASSERT(txs->frameid == fid);
				break;
			}
			if (++pdu_count >= ampdu_tx_cfg->ba_max_tx_wsize) {
				WL_ERROR(("%s: Reach max num of MPDU without finding frameid\n",
					__FUNCTION__));
				ASSERT(pdu_count >= ampdu_tx_cfg->ba_max_tx_wsize);
			}
#endif /* WLAMPDU_MAC */
		} else {
			if (((mcl & TXC_AMPDU_MASK) >> TXC_AMPDU_SHIFT) == TXC_AMPDU_LAST)
				break;
		}

		p = GETNEXTTXP(wlc, queue);
		/* For external use big hammer to restore the sync */
		if (p == NULL) {
#ifdef WLAMPDU_MAC
			WL_ERROR(("%s: p is NULL. tot_mpdu %d suc %d pdu %d first_frameid %x"
				"fifocnt %d\n", __FUNCTION__, tot_mpdu, suc_mpdu, pdu_count,
				first_frameid,
				R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));
#endif /* WLAMPDU_MAC */

#ifdef BCMDBG
#ifdef WLAMPDU_MAC
			wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
			wlc_dump_aggfifo(wlc, NULL);
#endif /* WLAMPDU_MAC */
#endif	/* BCMDBG */
			ASSERT(p);
			break;
		}

		WLPKTTAGSCBSET(p, scb);

	} /* while(p) */

#if defined(BCMDBG) || defined(BCMDBG_AMPDU)
	/* post-process the statistics */
	if (AMPDU_HOST_ENAB(wlc->pub)) {
		int i, j;
		ampdu_tx->amdbg->mpdu_histogram[idx]++;
		for (i = 0; i < idx; i++) {
			if (hole[i])
				ampdu_tx->amdbg->retry_histogram[i]++;
			if (hole[i]) {
				for (j = i + 1; (j < idx) && hole[j]; j++)
					;
				if (j == idx)
					ampdu_tx->amdbg->end_histogram[i]++;
			}
		}

#ifdef WLPKTDLYSTAT
		if (txretry < rr_retry_limit)
			WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs)], suc_mpdu);
		else
			WLCNTADD(ampdu_tx->cnt->txmpdu_succ_cnt[MCS2IDX(mcs_fbr)], suc_mpdu);
#endif /* WLPKTDLYSTAT */
	}
#endif /* BCMDBG */

#ifdef PKTQ_LOG
	ASSERT(txh && plcp);
	bw = (txh->PhyTxControlWord_1 & PHY_TXC1_BW_MASK) >> 1;

	/* Note HOST AMPDU path is HT only, no 10MHz support */
	ASSERT((bw >= BW_20MHZ) && (bw <= BW_40MHZ));

	if (txretry < rr_retry_limit) {
		/* Primary rate computation */
		rspec = MCS_TO_RSPEC(mcs, bw, plcp[3]);
		current_rate = wlc_rate_rspec2rate(rspec);
	} else {
		/* Fall-back rate computation */
		rspec = MCS_TO_RSPEC(mcs_fbr, bw, plcp[3]);
		current_rate = wlc_rate_rspec2rate(rspec);
	}

	if (suc_mpdu) {
		txrate_succ = BPS_TO_500K(current_rate) * suc_mpdu;
		WLCNTCONDADD(prec_cnt, prec_cnt->txrate_succ, txrate_succ);
		WLCNTCONDADD(actq_cnt, actq_cnt->txrate_succ, txrate_succ);
	}

	/* Air Time Computation */

	flg = WLC_AIRTIME_AMPDU;

	if (WLC_HT_GET_AMPDU_RTS(wlc->hti)) {
		flg |= (WLC_AIRTIME_RTSCTS);
	}
	ctl_rspec = AMPDU_BASIC_RATE(wlc->band, rspec);

	/* Short preamble */
	if (WLC_PROT_CFG_SHORTPREAMBLE(wlc->prot, bsscfg) &&
			(scb->flags & SCB_SHORTPREAMBLE) && IS_CCK(ctl_rspec)) {
		ctl_rspec |= RSPEC_SHORT_PREAMBLE;
	}

	rts_sifs = airtime_rts_usec(flg, ctl_rspec);
	cts_sifs = airtime_cts_usec(flg, ctl_rspec);
	ba_sifs = airtime_ba_usec(flg, ctl_rspec);
	ampdu_dur = wlc_airtime_packet_time_us(flg, rspec, tot_len);

	if (ba_recd) {
		/* If BA received */
		airtime = (txs->status.rts_tx_cnt + 1)* rts_sifs +
			cts_sifs + ampdu_dur + ba_sifs;
	} else {
		/* No Block Ack received
		 * 1. AMPDU transmitted but block ack not rcvd (RTS enabled/RTS disabled).
		 * 2. AMPDU not transmitted at all (RTS retrie timeout)
		 */
		if (txs->status.frag_tx_cnt) {
			airtime = (txs->status.rts_tx_cnt + 1)* rts_sifs +
				cts_sifs + ampdu_dur;
		} else {
			airtime = (txs->status.rts_tx_cnt + 1)* rts_sifs;
		}
	}
	WLCNTCONDADD(actq_cnt, actq_cnt->airtime, airtime);
#endif /* PKTQ_LOG */
	/* send bar to move the window */
	if (send_bar)
		wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);

	/* update rate state */
	if (WLANTSEL_ENAB(wlc))
		antselid = wlc_antsel_antsel2id(wlc->asi, mimoantsel);

	rs_txs.txrts_cnt = txs->status.rts_tx_cnt;
	rs_txs.rxcts_cnt = txs->status.cts_rx_cnt;
#ifdef WLAMPDU_MAC
	rs_txs.ncons = ncons;
	rs_txs.nlost = tot_mpdu - suc_mpdu;
#endif // endif
	rs_txs.ack_map1 = *(uint32 *)&bitmap[0];
	rs_txs.ack_map2 = *(uint32 *)&bitmap[4];
	/* init now to txs->frameid for host agg; for hw agg it is
	 * first_frameid.
	 */
	rs_txs.frameid = txs->frameid;
	rs_txs.antselid = antselid;
	rs_txs.ac = ac;
#ifdef WLAMPDU_MAC
	WL_AMPDU_HWDBG(("%s: consume %d txfifo_cnt %d\n", __FUNCTION__, tot_mpdu,
		R_REG(wlc->osh, &wlc->regs->u.d11regs.xmtfifo_frame_cnt)));

	if (AMPDU_MAC_ENAB(wlc->pub)) {
		/* primary rspec */
		rs_txs.txrspec[0] = HT_RSPEC(mcs);
		/* use primary rate's sgi & bw info */
		rs_txs.txrspec[0] |= is_sgi ? RSPEC_SHORT_GI : 0;
		rs_txs.txrspec[0] |= (is40 ? RSPEC_BW_40MHZ : RSPEC_BW_20MHZ);
		rs_txs.frameid = first_frameid;

		if (rs_txs.tx_cnt[0] + rs_txs.tx_cnt[1] == 0) {
			/* must have failed because of rts failure */
			ASSERT(rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1] == 0);
#ifdef BCMDBG
			if (rs_txs.txsucc_cnt[0] + rs_txs.txsucc_cnt[1] != 0) {
				WL_ERROR(("%s: wrong condition\n", __FUNCTION__));
				wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
			}
			if (WL_AMPDU_ERR_ON()) {
				wlc_print_ampdu_txstatus(ampdu_tx, txs, s1, s2, s3, s4);
			}
#endif /* BCMDBG */
			/* fix up tx_cnt */
			rs_txs.tx_cnt[0] = ncons;
		}

		if (update_rate)
			wlc_scb_ratesel_upd_txs_ampdu(wlc->wrsi, scb, &rs_txs, txs, FALSE);
	} else
#endif /* WLAMPDU_MAC */
	if (update_rate) {
		wlc_scb_ratesel_upd_txs_blockack(wlc->wrsi, scb,
			txs, suc_mpdu, tot_mpdu, !ba_recd, (uint8)txretry,
			rr_retry_limit, FALSE, mcs & MIMO_PLCP_MCS_MASK, is_sgi, antselid, ac);
	}

	WL_AMPDU_TX(("wl%d: wlc_ampdu_dotxstatus: requeueing %d packets, consumed %d\n",
		wlc->pub->unit, requeue, tot_mpdu));

	/* Call only once per ampdu_tx for SW agg */
	if (AMPDU_MAC_ENAB(wlc->pub)) {
#ifdef WLAMPDU_MAC
		WLCNTADD(ampdu_tx->cnt->pending, -tot_mpdu);
		WLCNTADD(ampdu_tx->cnt->cons, tot_mpdu);
		WLC_TXFIFO_COMPLETE(wlc, queue, tot_mpdu, tx_time);
#endif /* WLAMPDU_MAC */
	} else {
#if defined(TXQ_MUX)
		WLC_TXFIFO_COMPLETE(wlc, queue, tot_mpdu, tx_time);
#else
		WLC_TXFIFO_COMPLETE(wlc, queue, ampdu_tx_cfg->txpkt_weight, tx_time);
#endif /* TXQ_MUX */
	}

#ifdef PSPRETEND
	if (BSSCFG_AP(bsscfg)) {
		wlc_apps_process_pspretend_status(wlc, scb, ba_recd);
	}
#endif /* PSPRETEND */
	/* bump up the start seq to move the window */
	wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);

#ifdef MACOSX
	if (wlc->oscallback_dotxstatus)
		wlc->oscallback_dotxstatus(wlc, (void*)scb, (void*)NULL, (void *)NULL, (uint)0);
#endif /* MACOSX */

#ifdef STA
	/* PM state change */
	wlc_update_pmstate(bsscfg, (uint)(wlc_txs_alias_to_old_fmt(wlc, &(txs->status))));
#endif /* STA */
} /* wlc_ampdu_dotxstatus_complete */

#if defined(WLAMPDU_MAC) && defined(BCMDBG)

static void
wlc_print_ampdu_txstatus(ampdu_tx_info_t *ampdu_tx,
	tx_status_t *pkg1, uint32 s1, uint32 s2, uint32 s3, uint32 s4)
{
	uint16 *p = (uint16 *)pkg1;
	printf("%s: txstatus 0x%04x\n", __FUNCTION__, pkg1->status.raw_bits);
	if (AMPDU_MAC_ENAB(ampdu_tx->wlc->pub) && !AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		uint16 last_frameid, txstat;
		uint8 bitmap[8];
		uint16 txcnt_mrt, succ_mrt, txcnt_fbr, succ_fbr;
		uint16 ncons = pkg1->sequence;

		last_frameid = p[2]; txstat = p[3];

		bitmap[0] = (txstat & TX_STATUS_BA_BMAP03_MASK) >> TX_STATUS_BA_BMAP03_SHIFT;
		bitmap[0] |= (s1 & TX_STATUS_BA_BMAP47_MASK) << TX_STATUS_BA_BMAP47_SHIFT;
		bitmap[1] = (s1 >> 8) & 0xff;
		bitmap[2] = (s1 >> 16) & 0xff;
		bitmap[3] = (s1 >> 24) & 0xff;
		txcnt_mrt = s2 & 0xff;
		succ_mrt = (s2 >> 8) & 0xff;
		txcnt_fbr = (s2 >> 16) & 0xff;
		succ_fbr = (s2 >> 24) & 0xff;
		printf("\t\t ncons %d last frameid: 0x%x\n", ncons, last_frameid);
		printf("\t\t txcnt: mrt %2d succ %2d fbr %2d succ %2d\n",
		       txcnt_mrt, succ_mrt, txcnt_fbr, succ_fbr);

		if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub)) {
			printf("\t\t bitmap: %02x %02x %02x %02x\n",
				bitmap[0], bitmap[1], bitmap[2], bitmap[3]);
		} else {
			bitmap[4] = (s3 >> 16) & 0xff;
			bitmap[5] = (s3 >> 24) & 0xff;
			bitmap[6] = s4 & 0xff;
			bitmap[7] = (s4 >> 8) & 0xff;
			printf("\t\t bitmap: %02x %02x %02x %02x %02x %02x %02x %02x\n",
			       bitmap[0], bitmap[1], bitmap[2], bitmap[3],
			       bitmap[4], bitmap[5], bitmap[6], bitmap[7]);
			printf("\t\t timestamp 0x%04x\n", (s4 >> 16) & 0xffff);
		}
#ifdef WLAMPDU_UCODE
		if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub) && txcnt_mrt + txcnt_fbr == 0) {
			uint16 strt, end, wptr, rptr, rnum, qid;
			uint16 base =
				(wlc_read_shm(ampdu_tx->wlc, M_TXFS_PTR) + C_TXFSD_WOFFSET) * 2;
			qid = WLC_TXFID_GET_QUEUE(last_frameid);
			strt = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_STRT_POS(base, qid));
			end = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_END_POS(base, qid));
			wptr = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_WPTR_POS(base, qid));
			rptr = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_RPTR_POS(base, qid));
			rnum = wlc_read_shm(ampdu_tx->wlc, C_TXFSD_RNUM_POS(base, qid));
			printf("%s: side channel %d ---- \n", __FUNCTION__, qid);
			printf("\t\t strt : shm(0x%x) = 0x%x\n", C_TXFSD_STRT_POS(base, qid), strt);
			printf("\t\t end  : shm(0x%x) = 0x%x\n", C_TXFSD_END_POS(base, qid), end);
			printf("\t\t wptr : shm(0x%x) = 0x%x\n", C_TXFSD_WPTR_POS(base, qid), wptr);
			printf("\t\t rptr : shm(0x%x) = 0x%x\n", C_TXFSD_RPTR_POS(base, qid), rptr);
			printf("\t\t rnum : shm(0x%x) = 0x%x\n", C_TXFSD_RNUM_POS(base, qid), rnum);
		}
#endif /* WLAMPDU_UCODE */
	}
}

#endif  /* AMPDU_MAC && BCMDBG */

#ifdef WLAMPDU_MAC

int
wlc_dump_aggfifo(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	osl_t *osh = wlc->osh;
	d11regs_t *regs = wlc->regs;
	int ret = BCME_OK;

	if (!wlc->clk) {
		printf("%s: clk off\n", __FUNCTION__);
		return ret;
	}

	printf("%s:\n", __FUNCTION__);

	if (AMPDU_AQM_ENAB(wlc->pub)) {
		if (D11REV_GE(wlc->pub->corerev, 64)) {
			printf("psm_reg_mux 0x%x aqmqmap 0x%x aqmfifo_status 0x%x\n",
				R_REG(osh, &regs->u.d11acregs.psm_reg_mux),
				R_REG(osh, &regs->u.d11acregs.AQMQMAP),
				R_REG(osh, &regs->u.d11acregs.AQMFifo_Status));
			printf("AQM agg params 0x%x maxlen hi/lo 0x%x 0x%x "
				"minlen 0x%x adjlen 0x%x\n",
				R_REG(osh, &regs->u.d11acregs.AQMAggParams),
				R_REG(osh, &regs->u.d11acregs.AQMMaxAggLenHi),
				R_REG(osh, &regs->u.d11acregs.AQMMaxAggLenLow),
				R_REG(osh, &regs->u.d11acregs.AQMMinMpduLen),
				R_REG(osh, &regs->u.d11acregs.AQMMacAdjLen));
			printf("AQM agg results 0x%x num %d len hi/lo 0x%x 0x%x "
				"BAbitmap(0-3) %x %x %x %x\n",
				R_REG(osh, &regs->u.d11acregs.AQMAggStats),
				R_REG(osh, &regs->u.d11acregs.AQMAggNum),
				R_REG(osh, &regs->u.d11acregs.AQMAggLenHi),
				R_REG(osh, &regs->u.d11acregs.AQMAggLenLow),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA0),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA1),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA2),
				R_REG(osh, &regs->u.d11acregs.AQMUpdBA3));
			printf("AQM agg rdptr %d mpdu_len 0x%x idx 0x%x info 0x%x\n",
			       R_REG(osh, &regs->u.d11acregs.AQMAggRptr),
			       R_REG(osh, &regs->u.d11acregs.AQMMpduLen),
			       R_REG(osh, &regs->u.d11acregs.AQMAggIdx),
			       R_REG(osh, &regs->u.d11acregs.AQMAggEntry));

		} else {
			printf("framerdy 0x%x bmccmd %d framecnt 0x%x \n",
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMFifoReady),
				R_REG(osh, &regs->u.d11acregs.BMCCmd),
				R_REG(osh, &regs->u.d11acregs.XmtFifoFrameCnt));
			printf("AQM agg params 0x%x maxlen hi/lo 0x%x 0x%x "
				"minlen 0x%x adjlen 0x%x\n",
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggParams),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMaxAggLenHi),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMaxAggLenLow),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMinMpduLen),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMMacAdjLen));
			printf("AQM agg results 0x%x len hi/lo 0x%x 0x%x "
				"BAbitmap(0-3) %x %x %x %x\n",
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggStats),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggLenHi),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMAggLenLow),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA0),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA1),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA2),
				R_REG(osh, &regs->u.d11acregs.u0.lt64.AQMUpdBA3));
		}
		return ret;
	}

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(wlc->pub)) {
		int i;
		ampdu_tx_info_t* ampdu_tx = wlc->ampdu_tx;
		for (i = 0; i < 4; i++) {
			int k = 0, addr;
			printf("fifo %d: rptr %x wptr %x\n",
			       i, ampdu_tx->hagg[i].txfs_rptr,
			       ampdu_tx->hagg[i].txfs_wptr);
			for (addr = ampdu_tx->hagg[i].txfs_addr_strt;
			     addr <= ampdu_tx->hagg[i].txfs_addr_end; addr++) {
				printf("\tentry %d addr 0x%x: 0x%x\n",
				       k, addr, wlc_read_shm(wlc, addr * 2));
				k++;
			}
		}
	}
#endif /* WLAMPDU_UCODE */
#if defined(WLCNT) && defined(WLAMPDU_MAC)
	printf("driver statistics: aggfifo pending %d enque/cons %d %d\n",
	       wlc->ampdu_tx->cnt->pending,
	       wlc->ampdu_tx->cnt->enq,
	       wlc->ampdu_tx->cnt->cons);
#endif // endif

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		int i;
		printf("AGGFIFO regs: availcnt 0x%x txfifo fr_count %d by_count %d\n",
		       R_REG(osh, &regs->aggfifocnt),
		       R_REG(osh, &regs->u.d11regs.xmtfifo_frame_cnt),
		       R_REG(osh, &regs->u.d11regs.xmtfifo_byte_cnt));
		printf("cmd 0x%04x stat 0x%04x cfgctl 0x%04x cfgdata 0x%04x mpdunum 0x%02x"
		       " len 0x%04x bmp 0x%04x ackedcnt %d\n",
		       R_REG(osh, &regs->u.d11regs.aggfifo_cmd),
		       R_REG(osh, &regs->u.d11regs.aggfifo_stat),
		       R_REG(osh, &regs->u.d11regs.aggfifo_cfgctl),
		       R_REG(osh, &regs->u.d11regs.aggfifo_cfgdata),
		       R_REG(osh, &regs->u.d11regs.aggfifo_mpdunum),
		       R_REG(osh, &regs->u.d11regs.aggfifo_len),
		       R_REG(osh, &regs->u.d11regs.aggfifo_bmp),
		       R_REG(osh, &regs->u.d11regs.aggfifo_ackedcnt));

		/* aggfifo */
		printf("AGGFIFO dump: \n");
		for (i = 1; i < 2; i ++) {
			int j;
			printf("AGGFIFO %d:\n", i);
			for (j = 0; j < AGGFIFO_CAP; j ++) {
				uint16 entry;
				W_REG(osh, &regs->u.d11regs.aggfifo_sel, (i << 6) | j);
				W_REG(osh, &regs->u.d11regs.aggfifo_cmd, (6 << 2) | i);
				entry = R_REG(osh, &regs->u.d11regs.aggfifo_data);
				if (j % 4 == 0) {
					printf("\tEntry 0x%02x: 0x%04x	", j, entry);
				} else if (j % 4 == 3) {
					printf("0x%04x\n", entry);
				} else {
					printf("0x%04x	", entry);
				}
			}
		}
	}
#endif /* WLAMPDU_HW */

	return ret;
}
#endif /* WLAMPDU_MAC */

/** called as a result of an ADDBA sequence */
static void
ampdu_ba_state_off(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
#if !defined(TXQ_MUX)
	void *p;
	struct scb *scb = scb_ampdu->scb;
#endif // endif

	ini->ba_state = AMPDU_TID_STATE_BA_OFF;
	scb_ampdu->tidmask &= ~(1 << ini->tid);

#if !defined(TXQ_MUX)
	/* release all buffered pkts */
	while ((p = wlc_ampdu_pktq_pdeq(&scb_ampdu->txq, ini->tid))) {
		ASSERT(PKTPRIO(p) == ini->tid);
		SCB_TX_NEXT(TXMOD_AMPDU, scb, p, WLC_PRIO_TO_PREC(ini->tid));
	}
#endif /* !TXQ_MUX */

	/* XXX SWWLAN-56360 after wlc_detach gets called wlc->txqueues's parameters will be set to
	 * 0xdeadbeefdeadbeef.  However we see an instant where TXQ_STOP_FOR_AMPDU_FLOW_CNTRL
	 * bit get cleared afterward which causes 0xdeadbeefdeadbeef to become 0xdeadbeefdeadbcef,
	 * the ifdef below is a work around to prevent this clear from happening unless it's for
	 * DONGLEBUILD.
	 */
#if defined(DONGLEBUILD)
	wlc_txflowcontrol_override(scb_ampdu->ampdu_tx->wlc, SCB_WLCIFP(scb_ampdu->scb)->qi,
		OFF, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL);
#endif /* DONGLEBUILD */
}

/* Move window forward when BAR isn't pending.
 * In Reinit/BigHammer situation it must be called to advance the window
 * which will be stuck otherwise when no BAR is pending...
 */
static void
ampdu_ini_move_window(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	uint16 indx;

	if (ini->bar_ackpending)
		return;

	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		for (indx = 0; indx < ampdu_tx->config->ba_max_tx_wsize; indx++) {
			if (isset(ini->barpending, indx)) {
				clrbit(ini->barpending, indx);
				if (isset(ini->ackpending, indx)) {
					clrbit(ini->ackpending, indx);
					ini->txretry[indx] = 0;
				}
			}
		}
	} else {
		ini->acked_seq = ini->bar_ackpending_seq;
	}

	wlc_ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);
}

static bool
wlc_ampdu_map_pkts_cb(void *ctx, void *pkt)
{
	ampdu_tx_map_pkts_cb_params_t *cb_params = ctx;
	struct scb *scb = cb_params->scb;
	int tid = cb_params->tid;

	if ((scb != WLPKTTAGSCBGET(pkt)) ||
		(tid != PKTPRIO(pkt)) ||
		(!WLF2_PCB4(pkt)) ||
		!(WLPKTTAG(pkt)->flags & WLF_AMPDU_MPDU)) {
		return FALSE;
	}
	else {
		ampdu_tx_info_t *ampdu_tx;
		scb_ampdu_tx_t *scb_ampdu;
		scb_ampdu_tid_ini_t *ini;

		/* Disable the AMPDU packet callback for the packet */
		WLF2_PCB4_UNREG(pkt);

		/* Clear tx_in_transit */
		ampdu_tx = cb_params->ampdu_tx;
		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

		ini = scb_ampdu->ini[tid];
		if (ini == NULL) {
			WL_ERROR(("%s: ini is NULL\n", __FUNCTION__));
			return FALSE;
		}

		ASSERT(ini->tx_in_transit);
		ini->tx_in_transit--;
#ifdef WLATF
		ASSERT(ampdu_tx->tx_intransit);
		ampdu_tx->tx_intransit--;
#endif // endif
	}
	return FALSE;
}

static void
wlc_ampdu_tx_map_pkts(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid)
{
	ampdu_tx_map_pkts_cb_params_t cb_params;
	wlc_info_t *wlc = ampdu_tx->wlc;

	cb_params.scb = scb;
	cb_params.tid = tid;
	cb_params.ampdu_tx = ampdu_tx;

#ifdef NEW_TXQ
	wlc_low_txq_map_pkts(wlc, wlc->active_queue, wlc_ampdu_map_pkts_cb, &cb_params);
#ifdef WL_MULTIQUEUE
	if (MQUEUE_ENAB(wlc->pub) && wlc->active_queue != wlc->primary_queue)
		wlc_low_txq_map_pkts(wlc, wlc->primary_queue, wlc_ampdu_map_pkts_cb, &cb_params);
#endif /* WL_MULTIQUEUE */
#endif /* NEW_TXQ */
	wlc_txq_map_pkts(wlc, wlc->active_queue, wlc_ampdu_map_pkts_cb, &cb_params);
#ifdef WL_MULTIQUEUE
	if (MQUEUE_ENAB(wlc->pub) && wlc->active_queue != wlc->primary_queue)
		wlc_txq_map_pkts(wlc, wlc->primary_queue, wlc_ampdu_map_pkts_cb, &cb_params);
#endif /* WL_MULTIQUEUE */

#ifdef AP
	wlc_apps_map_pkts(wlc, scb, wlc_ampdu_map_pkts_cb, &cb_params);
#endif // endif

#ifdef WL_BSSCFG_TX_SUPR
	if (scb->bsscfg != NULL && scb->bsscfg->psq != NULL)
		wlc_bsscfg_psq_map_pkts(wlc, scb->bsscfg->psq, wlc_ampdu_map_pkts_cb, &cb_params);
#endif // endif

#ifdef WLC_LOW
	wlc_dma_map_pkts(wlc->hw, wlc_ampdu_map_pkts_cb, &cb_params);
#else
	wlc_rpctx_map_pkts(wlc->rpctx, wlc_ampdu_map_pkts_cb, &cb_params);
#endif /* WLC_LOW */

#ifdef HOST_HDR_FETCH
	if (HOST_HDR_FETCH_ENAB()) {
		wl_map_bus_txpkts(wlc->wl, wlc_ampdu_map_pkts_cb, &cb_params);
		wlc_bmac_tx_hdr_fetch_map_pkts(wlc, wlc_ampdu_map_pkts_cb, &cb_params);
	}
#endif // endif
}
/** Called when tearing down a connection with a conversation partner (DELBA or otherwise) */
void
ampdu_cleanup_tid_ini(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid, bool force)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);
	ASSERT(scb_ampdu->scb);
	AMPDU_VALIDATE_TID(ampdu_tx, tid, "ampdu_cleanup_tid_ini");

	if (!(ini = scb_ampdu->ini[tid]))
		return;

	WL_AMPDU_CTL(("wl%d: ampdu_cleanup_tid_ini: tid %d force %d tx_in_transit %u rem_window %u "
	              "ba_state %u\n",
	              ampdu_tx->wlc->pub->unit, tid, force, ini->tx_in_transit, ini->rem_window,
	              ini->ba_state));

	ini->ba_state = AMPDU_TID_STATE_BA_PENDING_OFF;
	scb_ampdu->tidmask &= ~(1 << tid);

	/* cleanup stuff that was to be done on bar send complete */
	ampdu_ini_move_window(ampdu_tx, scb_ampdu, ini);

#ifdef WLATF
	wlc_ampdu_atf_reset_state(ini);
#endif /* WLATF */

	if (ini->tx_in_transit && !force)
		return;

	if (ini->tx_in_transit == 0 &&
#ifdef PROP_TXSTATUS
	    !WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wl)) &&
#endif /* PROP_TXSTATUS */
	    TRUE) {
		/* ASSERT only when !ini->bar_ackpending - there are packets pending
		 * on ackpending state otherwise.
		 * Sequence of events leading to above situation:
		 * - AMPDU retry exceeds limit, ackpending is still set
		 * - As a result we send bar, bar_ackpending set to TRUE
		 * - AP disassociates, rem_window is not updated because of above states
		 */
		if (!ini->bar_ackpending &&
		    ini->rem_window != ini->ba_wsize) {
			WL_ERROR(("%s: out of sync: tid %d rem_window %u ba_wsize %u "
			          "bar_ackpending %u\n",
			          __FUNCTION__, tid, ini->rem_window, ini->ba_wsize,
			          ini->bar_ackpending));
			ASSERT(ini->rem_window == ini->ba_wsize);
		}
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, ini->scb);

	ASSERT(ini == scb_ampdu->ini[ini->tid]);

	/* free all buffered tx packets */
	wlc_ampdu_pktq_pflush(ampdu_tx->wlc->osh, &scb_ampdu->txq, ini->tid, TRUE, NULL, 0);

	/* Cancel any pending wlc_send_bar_complete packet callbacks. */
	wlc_pcb_fn_find(ampdu_tx->wlc->pcb, wlc_send_bar_complete, ini, TRUE);

	/* Disable the AMPDU packet callback for packets found in the map */
	wlc_ampdu_tx_map_pkts(ampdu_tx, scb, tid);

	/* Free ini immediately as no callbacks are pending */
	scb_ampdu->ini[ini->tid] = NULL;

	MFREE(ampdu_tx->wlc->osh, ini, sizeof(scb_ampdu_tid_ini_t));

	/* XXX SWWLAN-56360 after wlc_detach gets called wlc->txqueues's parameters will be set to
	 * 0xdeadbeefdeadbeef.  However we see an instant where TXQ_STOP_FOR_AMPDU_FLOW_CNTRL
	 * bit get cleared afterward which causes 0xdeadbeefdeadbeef to become 0xdeadbeefdeadbcef,
	 * the ifdef below is a work around to prevent this clear from happening unless it's for
	 * DONGLEBUILD.
	 */
#if defined(DONGLEBUILD)
	wlc_txflowcontrol_override(ampdu_tx->wlc, SCB_WLCIFP(scb_ampdu->scb)->qi,
	                           OFF, TXQ_STOP_FOR_AMPDU_FLOW_CNTRL);
#endif /* DONGLEBUILD */
} /* ampdu_cleanup_tid_ini */

void
wlc_ampdu_recv_addba_resp(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 *body, int body_len)
{
	scb_ampdu_tx_t *scb_ampdu_tx;
	wlc_info_t *wlc = ampdu_tx->wlc;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	struct pktq *txq;
	dot11_addba_resp_t *addba_resp;
	scb_ampdu_tid_ini_t *ini;
	uint16 param_set, status;
	uint8 tid, wsize, policy;
	uint16 current_wlctxq_len = 0, i = 0;
	void *p = NULL;

	ASSERT(scb);

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu_tx);

	addba_resp = (dot11_addba_resp_t *)body;

	if (addba_resp->category & DOT11_ACTION_CAT_ERR_MASK) {
		WL_ERROR(("wl%d: %s: unexp error action frame\n", wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

	status = ltoh16_ua(&addba_resp->status);
	param_set = ltoh16_ua(&addba_resp->addba_param_set);

	wsize =	(param_set & DOT11_ADDBA_PARAM_BSIZE_MASK) >> DOT11_ADDBA_PARAM_BSIZE_SHIFT;
	policy = (param_set & DOT11_ADDBA_PARAM_POLICY_MASK) >> DOT11_ADDBA_PARAM_POLICY_SHIFT;
	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu_tx, tid, "wlc_ampdu_recv_addba_resp");

	ini = scb_ampdu_tx->ini[tid];

	if ((ini == NULL) || (ini->ba_state != AMPDU_TID_STATE_BA_PENDING_ON)) {
		WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_resp: unsolicited packet\n",
			wlc->pub->unit));
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

	if ((status != DOT11_SC_SUCCESS) ||
	    (policy != ampdu_tx_cfg->ba_policy) ||
	    (wsize > ampdu_tx_cfg->ba_max_tx_wsize)) {
		WL_INFORM(("wl%d: %s: Failed. status %d wsize %d policy %d\n",
			wlc->pub->unit, __FUNCTION__, status, wsize, policy));
		ampdu_ba_state_off(scb_ampdu_tx, ini);
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

#ifdef WLAMSDU_TX
	if (AMSDU_TX_ENAB(wlc->pub)) {
		/* Record whether this scb supports amsdu over ampdu */
		scb->flags2 &= ~SCB2_AMSDU_IN_AMPDU_CAP;
		if ((param_set & DOT11_ADDBA_PARAM_AMSDU_SUP) != 0) {
			scb->flags2 |= SCB2_AMSDU_IN_AMPDU_CAP;
			/*  */
#ifdef WL_MU_TX
			if (SCB_MU(scb)) {
				int8 prev_max_pdu = scb_ampdu_tx->max_pdu;
				/* With amsdu_in_ampdu, scb max_pdu may change */
				scb_ampdu_update_config(ampdu_tx, scb);
				if (WLC_TXC_ENAB(ampdu_tx->wlc) &&
					(prev_max_pdu != scb_ampdu_tx->max_pdu)) {
					/* If max_pdu changes, invalid tx cache */
					wlc_txc_inv_all(ampdu_tx->wlc->txc);
				}
			}
#endif // endif
		}
	}
#endif /* WLAMSDU_TX */

	ini->ba_wsize = wsize;
	ini->rem_window = wsize;

#ifdef PROP_TXSTATUS
	ini->suppr_window = 0;
#endif /* PROP_TXSTATUS */

#if defined(USBAP) && defined(WLC_HIGH_ONLY)
	/* In split driver, if AMPDU aggregated by HOST driver, reduce
	 * max_pdu/release for small BA Wsize STA to enhance channel utilization.
	 */
	if (wlc->pub->_ampdu_tx && !wlc->pub->_ampdumac) {
		if (scb_ampdu_tx->max_pdu == AMPDU_NUM_MPDU_LEGACY &&
		    ini->ba_wsize == AMPDU_NUM_MPDU_LEGACY) {
			scb_ampdu_tx->max_pdu >>= 1;
			scb_ampdu_tx->release >>= 1;
		}
		else if (scb_ampdu_tx->max_pdu == AMPDU_MAX_MPDU) {
			/* half mpdu per ampdu in split driver architecture to enhace throughput */
			scb_ampdu_tx->release >>= 1;
		}
	}
#endif /* defined(USBAP) && defined(WLC_HIGH_ONLY) */

#if defined(WLOFFLD) && defined(UCODE_SEQ)
	/* for BK FIFO, if offload is enabled, we need to use the sequence number maintained by
	 * ucode, so read start_seq from shmem to keep driver and ucode in sync
	*/
	if (WLOFFLD_CAP(wlc) && (tid == PRIO_8021D_BK))
		wlc_ol_update_seq_iv(wlc, TRUE, scb);
#endif /* WLOFFLD && UCODE_SEQ */
	ini->start_seq = (SCB_SEQNUM(scb, tid) & (SEQNUM_MAX - 1));
	ini->tx_exp_seq = ini->start_seq;
	ini->acked_seq = ini->max_seq = MODSUB_POW2(ini->start_seq, 1, SEQNUM_MAX);
	ini->ba_state = AMPDU_TID_STATE_BA_ON;
	scb_ampdu_tx->tidmask |= (1 << tid);
#ifdef WLATF
	wlc_ampdu_atf_reset_state(ini);
#endif /* WLATF */

	WLCNTINCR(ampdu_tx->cnt->rxaddbaresp);
	AMPDUSCBCNTINCR(scb_ampdu_tx->cnt.rxaddbaresp);

	WL_AMPDU_CTL(("wl%d: wlc_ampdu_recv_addba_resp: Turning BA ON: tid %d wsize %d\n",
		wlc->pub->unit, tid, wsize));

	/* send bar to set initial window */
	wlc_ampdu_send_bar(ampdu_tx, ini, TRUE);
	/* XXX:TXQ
	 * WES: This code scrubs the TxQ for pkts with this SCB, but now the big TxQ is eliminated
	 * WES: and pkts should already be on the SCB specific Q.
	 * WES: Only pkts in the low TxQ and DMA ring might
	 * WES: exist. The existing code does not clean these up,
	 * WES: so ditching this code when we switch
	 * WES: to SCB specific Qs should not introduce any new problems.
	 * WES: Maybe there are not packets previously queued in the TxQ,
	 * WES: in practice since AMPDU is configured.
	 * WES: early and holds traffic until ADDBA is done?
	 * WES: When would there be a transition between non-BA frames and BA agreement starting?
	*/

	/* if packets already exist in wlcq, conditionally transfer them to ampduq
		to avoid barpending stuck issue.
	*/
	txq = WLC_GET_TXQ(SCB_WLCIFP(scb)->qi);

	current_wlctxq_len = pktq_plen(txq, WLC_PRIO_TO_PREC(tid));

	for (i = 0; i < current_wlctxq_len; i++) {
		p = pktq_pdeq(txq, WLC_PRIO_TO_PREC(tid));

		if (!p) break;
		/* omit control/mgmt frames and queue them to the back
		 * only the frames relevant to this scb should be moved to ampduq;
		 * otherwise, queue them back.
		 */
		if (!(WLPKTTAG(p)->flags & WLF_DATA) || 	/* mgmt/ctl */
		    scb != WLPKTTAGSCBGET(p) || /* frame belong to some other scb */
		    (WLPKTTAG(p)->flags & WLF_AMPDU_MPDU)) { /* possibly retried AMPDU */
			/* WES: Direct TxQ reference */
			if (wlc_prec_enq(wlc, txq, p, WLC_PRIO_TO_PREC(tid)))
				continue;
		} else { /* XXXCheck if Flow control/watermark issues */
			if (wlc_ampdu_prec_enq(wlc, &scb_ampdu_tx->txq, p, tid))
				continue;
		}

		/* Toss packet as queuing failed */
		WL_AMPDU_ERR(("wl%d: txq overflow\n", wlc->pub->unit));
		PKTFREE(wlc->osh, p, TRUE);
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		WLCNTINCR(ampdu_tx->cnt->txdrop);
		AMPDUSCBCNTINCR(scb_ampdu_tx->cnt.txdrop);
		WLCIFCNTINCR(scb, txnobuf);
		WLCNTSCBINCR(scb->scb_stats.tx_failures);
	}

#ifdef WLNAR
	wlc_nar_release_all_from_queue(wlc->nar_handle, scb, WLC_PRIO_TO_PREC(tid));
#endif /* WLNAR */
	/* release pkts */
	wlc_ampdu_txeval(ampdu_tx, scb_ampdu_tx, ini, FALSE);
} /* wlc_ampdu_recv_addba_resp */

void
wlc_ampdu_recv_ba(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 *body, int body_len)
{
	/* AMPDUXXX: silently ignore the ba since we don't handle it for immediate ba */
	WLCNTINCR(ampdu_tx->cnt->rxba);
}

#ifdef WLATF_CNT
/* ATF counter clearing code */
static void wlc_ampdu_atf_clear_counters(scb_ampdu_tid_ini_t *ini)
{
	bzero(AMPDU_ATF_STATS(ini), sizeof(atf_stats_t));
}

static BCMFASTPATH void wlc_ampdu_atf_update_rstat(atf_stats_range_t *range, uint32 val)
{
	unsigned long accum = range->accum + val;
	uint32 iter = range->iter + 1;

	if (val > range->max) {
		range->max = val;
	}

	if ((val < range->min) || (range->min == 0)) {
		range->min = val;
	}

	/* Overflow check */
	if ((accum < val) || (!iter)) {
		accum = val;
		iter = 1;
	}
	range->accum = accum;
	range->iter = iter;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)

static uint wlc_ampdu_atf_rstat_avg(atf_stats_range_t *rstat)
{
	rstat->avg = (rstat->iter) ? rstat->accum/rstat->iter : 0;
	return (rstat->avg);
}

static void wlc_ampdu_atf_print_rstat(struct bcmstrbuf *b,
	char *hdr, atf_stats_range_t *rstat, char *tlr)
{
	bcm_bprintf(b, "%s(%u/%u/%u)%s", hdr, wlc_ampdu_atf_rstat_avg(rstat),
		rstat->min, rstat->max, tlr);
}

static void wlc_ampdu_atf_dump(atf_state_t *atf_state, struct bcmstrbuf *b)
{
				bcm_bprintf(b, "\tatf %d ta %dus tm %dus rso:0x%x\n",
					atf_state->atf,
					atf_state->txq_time_allowance_us,
					atf_state->txq_time_min_allowance_us,
					atf_state->rspec_override);
				if (atf_state->atf) {
				atf_stats_t *atf_stats = &atf_state->atf_stats;

				bcm_bprintf(b, "\t tlimit %d flimit %d uflow %d oflow %d\n",
					atf_stats->timelimited, atf_stats->framelimited,
					atf_stats->uflow, atf_stats->oflow);
				bcm_bprintf(b, "\t reset %d flush %d po %d minrel %d sgl %d\n",
					atf_state->reset, atf_stats->flush,
					atf_stats->reloverride, atf_stats->minrel,
					atf_stats->singleton);
				bcm_bprintf(b, "\t eval %d neval %d dq %d skp %d npkts %d\n",
					atf_stats->eval, atf_stats->neval, atf_stats->ndequeue,
					atf_stats->rskip, atf_stats->npkts);
				bcm_bprintf(b, "\t qe %d proc %d rp %d nA %d\n",
					atf_stats->qempty, atf_stats->proc,
					atf_stats->reproc, atf_stats->reg_mpdu);
				bcm_bprintf(b, "\t cache hit/miss(%d/%d)\n",
					atf_stats->cache_hit, atf_stats->cache_miss);
				bcm_bprintf(b, "\t avg/min/max\n");

				wlc_ampdu_atf_print_rstat(b,
					"\t  rbytes", &atf_stats->rbytes, "\n");
				wlc_ampdu_atf_print_rstat(b,
					"\t  in_flt", &atf_stats->inflight, "\n");
				wlc_ampdu_atf_print_rstat(b,
					"\t  inputq", &atf_stats->inputq, "\n");

				wlc_ampdu_atf_print_rstat(b,
					"\t  rpkts", &atf_stats->release, "  ");
				wlc_ampdu_atf_print_rstat(b,
					"transit", &atf_stats->transit, "\n");

				wlc_ampdu_atf_print_rstat(b,
					"\t  chunk bytes", &atf_stats->chunk_bytes, " ");
				wlc_ampdu_atf_print_rstat(b,
					"pkts", &atf_stats->chunk_pkts, "  ");
				wlc_ampdu_atf_print_rstat(b,
					"pdu", &atf_stats->pdu, "\n");
				}
}
#else
#define wlc_ampdu_atf_dump(atf_state, b)
#endif /* BCMDBG || BCMDBG_DUMP || WLTEST || BCMDBG_AMPDU */
#else /* WLATF_CNT */
#define wlc_ampdu_atf_dump(atf_state, b)
#endif /* WLATF_CNT */

#ifdef WLATF
/* Set per TID atf mode */
static void wlc_ampdu_atf_tid_setmode(scb_ampdu_tid_ini_t *ini, uint32 mode)
{
	wlc_ampdu_atf_reset_state(ini);
	AMPDU_ATF_STATE(ini)->atf = mode;
}

/* Set per SCB atf mode */
static void wlc_ampdu_atf_scb_setmode(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint32 mode)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu && scb_ampdu->ini[n])
			wlc_ampdu_atf_tid_setmode(scb_ampdu->ini[n], mode);
	}
}

/* Set global ATF default mode, used each time a TID for a SCB is created */
void wlc_ampdu_atf_set_default_mode(ampdu_tx_info_t *ampdu_tx, scb_module_t *scbstate, uint32 mode)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			wlc_ampdu_atf_scb_setmode(ampdu_tx, scb, mode);
		}
	}
}

/* Set per TID atf txq release time allowance in microseconds */
static void wlc_ampdu_atf_tid_set_release_time(scb_ampdu_tid_ini_t *ini, uint txq_time_allowance_us)
{
	AMPDU_ATF_STATE(ini)->txq_time_allowance_us = txq_time_allowance_us;

#ifdef WL_MU_TX
	ASSERT(ini->scb);
	if (MU_TX_ENAB(ini->scb->bsscfg->wlc) && SCB_MU(ini->scb)) {
		/* For MU clients, allow double time to meet performance requirements */
		AMPDU_ATF_STATE(ini)->txq_time_allowance_us *= 2;
	}
#endif /* WL_MU_TX */

	/* Reset the last cached ratespec value in order to force to recalculate the
	 * rbytes using new value of txq_time_allowance_us.
	 */
	AMPDU_ATF_STATE(ini)->last_est_rate = 0;
}

/* Set per SCB atf txq release time allowance in microseconds */
static void wlc_ampdu_atf_scb_set_release_time(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint txq_time_allowance_us)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu && scb_ampdu->ini[n])
			wlc_ampdu_atf_tid_set_release_time(
				scb_ampdu->ini[n], txq_time_allowance_us);
	}
}

/* Set per SCB atf txq release time allowance in microseconds for all SCBs */
static void wlc_ampdu_atf_set_default_release_time(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint txq_time_allowance_us)
{
	struct scb *scb;
	struct scb_iter scbiter;
	FOREACHSCB(scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			wlc_ampdu_atf_scb_set_release_time(ampdu_tx, scb, txq_time_allowance_us);
		}
	}
}

/* Set per TID atf txq release minimum time allowance in microseconds */
static void wlc_ampdu_atf_tid_set_release_mintime(scb_ampdu_tid_ini_t *ini, uint allowance_us)
{
	AMPDU_ATF_STATE(ini)->txq_time_min_allowance_us = allowance_us;
	/* Reset the last cached ratespec value in order to force to recalculate the
	 * rbytes using new value of txq_time_min_allowance_us.
	 */
	AMPDU_ATF_STATE(ini)->last_est_rate = 0;
}

/* Set per SCB atf txq release time allowance in microseconds */
static void wlc_ampdu_atf_scb_set_release_mintime(ampdu_tx_info_t *ampdu_tx,
	struct scb *scb, uint allowance_us)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu && scb_ampdu->ini[n])
			wlc_ampdu_atf_tid_set_release_mintime(
				scb_ampdu->ini[n], allowance_us);
	}
}

/* Set per SCB atf txq release time allowance in microseconds for all SCBs */
static void wlc_ampdu_atf_set_default_release_mintime(ampdu_tx_info_t *ampdu_tx,
	scb_module_t *scbstate, uint allowance_us)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			wlc_ampdu_atf_scb_set_release_mintime(ampdu_tx, scb, allowance_us);
		}
	}
}

/* Set the function pointer that returns rate state information */
static void
wlc_ampdu_atf_set_rspec_action(atf_state_t *atf_state, atf_rspec_fn_t fn, void *arg)
{
	atf_state->rspec_action.function = fn;
	atf_state->rspec_action.arg = arg;
}

/* Alternate rate function, used whe the rate selection engine is manually over-ridden */
static ratespec_t BCMFASTPATH
wlc_ampdu_atf_ratespec_override(void *ptr)
{
#ifdef WLATF_FASTRATE
	return ((atf_state_t *)(ptr))->rspec_override;
#else
	atf_state_t *atf_state = (atf_state_t *)(ptr);
	return wlc_scb_ratesel_get_primary(atf_state->wlc, atf_state->scb, NULL);
#endif // endif
}

/* Set the rate override condition in ATF */
static void
wlc_ampdu_atf_ini_set_rspec_action(atf_state_t *atf_state, ratespec_t rspec)
{
	/* Sets the rspec action which is a function and an argument
	 * If rspec is zero default to wlc_ratesel_rawcurspec()
	 */

	if (rspec) {
		wlc_ampdu_atf_set_rspec_action(atf_state,
			wlc_ampdu_atf_ratespec_override, atf_state);
	} else {
		/* Set the default function pointer that returns rate state information */
		wlc_ampdu_atf_set_rspec_action(atf_state,
			(atf_rspec_fn_t)wlc_ratesel_rawcurspec, (void *)atf_state->rcb);
	}

	atf_state->rspec_override = rspec;
}

/* Wrapper function to enable/disable rate overide for all ATF tids in the SCB
 * Called by per-SCB rate override code.
 */
void
wlc_ampdu_atf_scb_rate_override(ampdu_tx_info_t *ampdu_tx, struct scb *scb, ratespec_t rspec)
{
	uint32 n = 0;
	scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);

	if (!scb_ampdu)
		return;

	for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
		if (scb_ampdu->ini[n])
			wlc_ampdu_atf_ini_set_rspec_action(
			AMPDU_ATF_STATE(scb_ampdu->ini[n]), rspec);
	}
}
/* Wrapper function to enable/disable rate overide for the entire driver,
 * called by rate override code
 */
void
wlc_ampdu_atf_rate_override(wlc_info_t *wlc, ratespec_t rspec, wlcband_t *band)
{
	/* Process rate update notification */
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (band->bandtype == wlc_scbband(scb)->bandtype) {
			wlc_ampdu_atf_scb_rate_override(wlc->ampdu_tx, scb, rspec);
		}
	}
}

static void wlc_ampdu_atf_tid_ini(ampdu_tx_info_t *ampdu_tx,
	scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
	atf_state_t *atf_state = AMPDU_ATF_STATE(ini);

	atf_state->ac = WME_PRIO2AC(ini->tid);
	atf_state->band = wlc->bandstate[
		CHSPEC_WLCBANDUNIT(scb_ampdu->scb->bsscfg->current_bss->chanspec)];
	atf_state->rcb = wlc_scb_ratesel_getrcb(wlc, scb_ampdu->scb, WME_PRIO2AC(ini->tid));
	ASSERT(atf_state->rcb);
	atf_state->ampdu_tx = ampdu_tx;
	atf_state->scb_ampdu = scb_ampdu;
	atf_state->ini = ini;
	atf_state->wlc = wlc;
	atf_state->scb = ini->scb;

	/* Set operating state, this resets state machine */
	wlc_ampdu_atf_tid_setmode(ini, wlc->atf);

	/* Reset ATF state machine again, it is assumed the tid is coming from an
	 * unknown and possibly invalid state.
	 */
	wlc_ampdu_atf_reset_state(ini);
	/* Set other operating parameters */
	wlc_ampdu_atf_tid_set_release_time(ini, ampdu_tx->config->txq_time_allowance_us);
	wlc_ampdu_atf_tid_set_release_mintime(ini, ampdu_tx->config->txq_time_min_allowance_us);
}
#else /* WLATF */
#define wlc_ampdu_atf_tid_ini(ampdu_tx, scb_ampdu, ini)
#endif /* WLATF */

/** initialize the initiator code for tid. Called around ADDBA exchange. */
static scb_ampdu_tid_ini_t*
wlc_ampdu_init_tid_ini(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tx_t *scb_ampdu, uint8 tid,
	bool override)
{
	scb_ampdu_tid_ini_t *ini;
	wlc_info_t *wlc = ampdu_tx->wlc;
	ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;
	struct scb *scb;
	uint8 peer_density;
	uint32 max_rxlen;
	uint8 max_rxlen_factor;

	ASSERT(scb_ampdu);
	ASSERT(scb_ampdu->scb);
	ASSERT(SCB_AMPDU(scb_ampdu->scb));
	ASSERT(tid < AMPDU_MAX_SCB_TID);
	scb = scb_ampdu->scb;

	if (ampdu_tx_cfg->manual_mode && !override)
		return NULL;

	/* check for per-tid control of ampdu */
	if (!ampdu_tx_cfg->ini_enable[tid])
		return NULL;

	/* AMPDUXXX: No support for dynamic update of density/len from peer */
	/* retrieve the density and max ampdu len from scb */
	wlc_ht_get_scb_ampdu_params(wlc->hti, scb, &peer_density,
		&max_rxlen, &max_rxlen_factor);

	/* our density requirement is for tx side as well */
	scb_ampdu->mpdu_density = MAX(peer_density, ampdu_tx_cfg->mpdu_density);

	/* should call after mpdu_density is init'd */
	wlc_ampdu_init_min_lens(scb_ampdu);

	/* max_rxlen is only used for host aggregation (corerev < 40) */
	scb_ampdu->max_rxlen = max_rxlen;

	scb_ampdu->max_rxfactor = max_rxlen_factor;

#if defined(WL11AC)
	if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype) && SCB_VHT_CAP(scb)) {
		scb_ampdu->max_rxfactor = wlc_vht_get_scb_ampdu_max_exp(wlc->vhti, scb) +
			AMPDU_RX_FACTOR_BASE_PWR;
	}
#endif /* WL11AC */

	scb_ampdu_update_config(ampdu_tx, scb);

	if (scb_ampdu->ini[tid])
		ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);

	if (!scb_ampdu->ini[tid]) {
		ini = MALLOCZ(wlc->osh, sizeof(scb_ampdu_tid_ini_t));
		if (ini == NULL)
			return NULL;

		scb_ampdu->ini[tid] = ini;
	} else {
		/* Shell not happen */
		return NULL;
	}

	memset(ini, 0, sizeof(scb_ampdu_tid_ini_t));

	ini->ba_state = AMPDU_TID_STATE_BA_PENDING_ON;
	ini->tid = tid;
	ini->scb = scb;
	ini->retry_bar = AMPDU_R_BAR_DISABLED;
#if defined(TXQ_MUX) && !defined(WLATF)
	ini->ac = WME_PRIO2AC(tid);
#endif // endif
	wlc_ampdu_atf_tid_ini(ampdu_tx, scb_ampdu, ini);
	wlc_send_addba_req(wlc, ini->scb, tid, ampdu_tx_cfg->ba_tx_wsize,
		ampdu_tx->config->ba_policy, ampdu_tx_cfg->delba_timeout);

	WLCNTINCR(ampdu_tx->cnt->txaddbareq);
	AMPDUSCBCNTINCR(scb_ampdu->cnt.txaddbareq);

	return ini;
} /* wlc_ampdu_init_tid_ini */

/** Remote side sent us a ADDBA request */
void
wlc_ampdu_recv_addba_req_ini(ampdu_tx_info_t *ampdu_tx, struct scb *scb,
	dot11_addba_req_t *addba_req, int body_len)
{
	scb_ampdu_tx_t *scb_ampdu_tx;
	scb_ampdu_tid_ini_t *ini;
	uint16 param_set;
	uint8 tid;

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu_tx);

	param_set = ltoh16_ua(&addba_req->addba_param_set);
	tid = (param_set & DOT11_ADDBA_PARAM_TID_MASK) >> DOT11_ADDBA_PARAM_TID_SHIFT;
	AMPDU_VALIDATE_TID(ampdu_tx, tid, "wlc_ampdu_recv_addba_req_ini");

	/* check if it is action err frame */
	if (addba_req->category & DOT11_ACTION_CAT_ERR_MASK) {
		ini = scb_ampdu_tx->ini[tid];
		if ((ini != NULL) && (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)) {
			ampdu_ba_state_off(scb_ampdu_tx, ini);
			WL_ERROR(("wl%d: %s: error action frame\n",
				ampdu_tx->wlc->pub->unit, __FUNCTION__));
		} else {
			WL_ERROR(("wl%d: %s: unexp error action "
				"frame\n", ampdu_tx->wlc->pub->unit, __FUNCTION__));
		}
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}
}

/** called due to 'wl' utility or as part of the attach phase */
int
wlc_ampdu_tx_set(ampdu_tx_info_t *ampdu_tx, bool on)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
	uint8 ampdu_mac_agg = AMPDU_AGG_OFF;
	uint16 value = 0;
	int err = BCME_OK;
	int8 aggmode;

	wlc->pub->_ampdu_tx = FALSE;

	if (ampdu_tx->config->ampdu_aggmode == AMPDU_AGGMODE_AUTO) {
		if (D11REV_IS(wlc->pub->corerev, 31) && WLCISNPHY(wlc->band)) {
			aggmode = AMPDU_AGGMODE_HOST;
		} else if ((D11REV_IS(wlc->pub->corerev, 26) ||
			D11REV_IS(wlc->pub->corerev, 29)) &&
			WLCISHTPHY(wlc->band)) {
			aggmode = AMPDU_AGGMODE_HOST;
		}
#ifdef WLP2P
		else if (D11REV_IS(wlc->pub->corerev, 24) && WLCISLCNPHY(wlc->band))
			aggmode = AMPDU_AGGMODE_HOST;
#endif // endif
		else if (D11REV_GE(wlc->pub->corerev, 16) && D11REV_LT(wlc->pub->corerev, 26))
			aggmode = AMPDU_AGGMODE_MAC;
		else if (D11REV_GE(wlc->pub->corerev, 40))
			aggmode = AMPDU_AGGMODE_MAC;
		else
			aggmode = AMPDU_AGGMODE_HOST;
	} else
		aggmode = ampdu_tx->config->ampdu_aggmode;

	if (on) {
		if (!N_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver not nmode enabled\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
		if (!wlc_ampdu_tx_cap(ampdu_tx)) {
			WL_AMPDU_ERR(("wl%d: device not ampdu capable\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}
		if (PIO_ENAB(wlc->pub)) {
			WL_AMPDU_ERR(("wl%d: driver is pio mode\n", wlc->pub->unit));
			err = BCME_UNSUPPORTED;
			goto exit;
		}

#ifdef WLAMPDU_MAC
		/* XXX if AMPDU_AGGMODE_MAC is selected, further decide ucode or hw agg
		 *  chip has to be capable to support HW agg
		 */
		if (aggmode == AMPDU_AGGMODE_MAC) {
			if (D11REV_GE(wlc->pub->corerev, 16) && D11REV_LT(wlc->pub->corerev, 26)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: 26 > Corerev >= 16 required for ucode agg\n",
					wlc->pub->unit));
#endif // endif
			}
			if (D11REV_IS(wlc->pub->corerev, 24) && WLCISNPHY(wlc->band)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: Corerev is 24 NPHY support ucode agg only\n",
				wlc->pub->unit));
#endif // endif
			}
			if ((D11REV_IS(wlc->pub->corerev, 26) ||
				D11REV_IS(wlc->pub->corerev, 29)) && WLCISHTPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is %d HTPHY support hw agg only\n",
					wlc->pub->unit, wlc->pub->corerev));
			}
			if (D11REV_IS(wlc->pub->corerev, 31) && WLCISNPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 31 NPHY support hw agg only\n",
					wlc->pub->unit));
			}
			if (D11REV_IS(wlc->pub->corerev, 34) && WLCISNPHY(wlc->band)) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: Corerev is 34 NPHY support ucode agg only\n",
					wlc->pub->unit));
#endif // endif
			}
			if (D11REV_IS(wlc->pub->corerev, 36)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 36 support hw agg only\n",
					wlc->pub->unit));
			}
			if (D11REV_IS(wlc->pub->corerev, 37) && WLCISNPHY(wlc->band)) {
				ampdu_mac_agg = AMPDU_AGG_HW;
				WL_AMPDU(("wl%d: Corerev is 37 NPHY support hw agg only\n",
					wlc->pub->unit));
			}
			else if (D11REV_IS(wlc->pub->corerev, 35) ||
				(D11REV_GE(wlc->pub->corerev, 37) &&
				D11REV_LE(wlc->pub->corerev, 39))) {
#ifdef WLAMPDU_UCODE
				ampdu_mac_agg = AMPDU_AGG_UCODE;
				WL_AMPDU(("wl%d: Corerev is %d support ucode agg only\n",
					wlc->pub->unit, wlc->pub->corerev));
#endif // endif
			}
			if (D11REV_GE(wlc->pub->corerev, 40)) {
				ampdu_mac_agg = AMPDU_AGG_AQM;
				WL_AMPDU(("wl%d: Corerev is 40 aqm agg\n",
					wlc->pub->unit));
			}

			/* Catch building a particular mode which is not supported by ucode rev */
#if defined(DONGLEBUILD)
#ifdef WLAMPDU_HW
			ASSERT(ampdu_mac_agg == AMPDU_AGG_HW);
#endif /* WLAMPDU_HW */
#ifdef WLAMPDU_UCODE
			ASSERT(ampdu_mac_agg == AMPDU_AGG_UCODE);
#endif /* WLAMPDU_UCODE */
#ifdef WLAMPDU_AQM
			ASSERT(ampdu_mac_agg == AMPDU_AGG_AQM);
#endif /* WLAMPDU_AQM */
#endif /* DONGLEBUILD */
		}
#endif /* WLAMPDU_MAC */
	}

	/* if aggmode define as MAC, but mac_agg is set to OFF,
	 * then set aggmode to HOST as defaultw
	 */
	if ((aggmode == AMPDU_AGGMODE_MAC) && (ampdu_mac_agg == AMPDU_AGG_OFF))
		aggmode = AMPDU_AGGMODE_HOST;

	/* setup ucode tx-retrys for MAC mode */
	if ((aggmode == AMPDU_AGGMODE_MAC) && (ampdu_mac_agg != AMPDU_AGG_AQM)) {
			value = MHF3_UCAMPDU_RETX;
	}

	if (wlc->pub->_ampdu_tx != on) {
#ifdef WLCNT
		bzero(ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
#endif // endif
		wlc->pub->_ampdu_tx = on;
	}

exit:
	if (wlc->pub->_ampdumac != ampdu_mac_agg) {
		wlc->pub->_ampdumac = ampdu_mac_agg;
		wlc_mhf(wlc, MHF3, MHF3_UCAMPDU_RETX, value, WLC_BAND_ALL);
		/* just differentiate HW or not for now */
		wlc_ampdu_mode_upd(wlc, AMPDU_HW_ENAB(wlc->pub) ?
			AMPDU_AGG_HW : AMPDU_AGGMODE_HOST);
	}

	/* setup max pkt aggr for MAC mode */
#if defined(NEW_TXQ)
	wlc_set_default_txmaxpkts(wlc);
#else
	if (aggmode == AMPDU_AGGMODE_MAC) {
		if (ampdu_mac_agg == AMPDU_AGG_AQM) {
			wlc->pub->txmaxpkts = MAXTXPKTS_AMPDUAQM;
		} else {
			wlc->pub->txmaxpkts = MAXTXPKTS_AMPDUMAC;
		}
	} else {
		wlc->pub->txmaxpkts = MAXTXPKTS;
	}
#endif /* NEW_TXQ */

	WL_ERROR(("wl%d: %s: %s txmaxpkts %d\n", wlc->pub->unit, __FUNCTION__,
	          ((aggmode == AMPDU_AGGMODE_HOST) ? " AGG Mode = HOST" :
	           ((ampdu_mac_agg == AMPDU_AGG_UCODE) ? "AGG Mode = MAC+Ucode" :
	           ((ampdu_mac_agg == AMPDU_AGG_HW) ? "AGG Mode = MAC+HW" : "AGG Mode = MAC+AQM"))),
	          wlc_get_txmaxpkts(wlc)));

	return err;
} /* wlc_ampdu_tx_set */

bool
wlc_ampdu_tx_cap(ampdu_tx_info_t *ampdu_tx)
{
	if (WLC_PHY_11N_CAP(ampdu_tx->wlc->band))
		return TRUE;
	else
		return FALSE;
}

#ifdef AMPDU_NON_AQM

/** called during attach phase and as a result of a 'wl' command */
static void
ampdu_update_max_txlen(ampdu_tx_config_t *ampdu_tx_cfg, uint16 dur)
{
#if !defined(WLPROPRIETARY_11N_RATES) /* avoiding ROM abandons */
	uint32 rate, mcs;
#else
	uint32 rate;
	int mcs = -1;
#endif /* WLPROPRIETARY_11N_RATES */

	dur = dur >> 10;

#if defined(WLPROPRIETARY_11N_RATES)
	while (TRUE) {
		mcs = NEXT_MCS(mcs); /* iterate through both standard and prop ht rates */
		if (mcs > WLC_11N_LAST_PROP_MCS)
			break;
#else
	for (mcs = 0; mcs < MCS_TABLE_SIZE; mcs++) {
#endif /* WLPROPRIETARY_11N_RATES */
		/* rate is in Kbps; dur is in msec ==> len = (rate * dur) / 8 */
		/* 20MHz, No SGI */
		rate = MCS_RATE(mcs, FALSE, FALSE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][0][0] = (rate * dur) >> 3;
		/* 40 MHz, No SGI */
		rate = MCS_RATE(mcs, TRUE, FALSE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][1][0] = (rate * dur) >> 3;
		/* 20MHz, SGI */
		rate = MCS_RATE(mcs, FALSE, TRUE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][0][1] = (rate * dur) >> 3;
		/* 40 MHz, SGI */
		rate = MCS_RATE(mcs, TRUE, TRUE);
		ampdu_tx_cfg->max_txlen[MCS2IDX(mcs)][1][1] = (rate * dur) >> 3;
	}
}

#endif /* AMPDU_NON_AQM */

/**
 * Applicable to pre-AC chips. Reduces tx underflows on lower MCS rates (e.g. MCS 15) on 40Mhz with
 * frame bursting enabled. If frameburst is enabled, and 40MHz BW, and ampdu_density is smaller than
 * 7, transmit uses adjusted min_bytes.
 *
 * Called during AMPDU connection setup (ADDBA related).
 */
static void
wlc_ampdu_init_min_lens(scb_ampdu_tx_t *scb_ampdu)
{
	int mcs, tmp;
	tmp = 1 << (16 - scb_ampdu->mpdu_density);
#if defined(WLPROPRIETARY_11N_RATES)
	mcs = -1;
	while (TRUE) {
		mcs = NEXT_MCS(mcs); /* iterate through both standard and prop ht rates */
		if (mcs > WLC_11N_LAST_PROP_MCS)
			break;
#else
	for (mcs = 0; mcs <= AMPDU_MAX_MCS; mcs++) {
#endif /* WLPROPRIETARY_11N_RATES */
		scb_ampdu->min_lens[MCS2IDX(mcs)] =  0;
		if (scb_ampdu->mpdu_density > AMPDU_DENSITY_4_US)
			continue;
		if (mcs >= 12 && mcs <= 15) {
			/* use rate for mcs21, 40Mhz bandwidth, no sgi */
			scb_ampdu->min_lens[MCS2IDX(mcs)] = CEIL(MCS_RATE(21, 1, 0), tmp);
		} else if (mcs == 21 || mcs == 22) {
			/* use mcs23 */
			scb_ampdu->min_lens[MCS2IDX(mcs)] = CEIL(MCS_RATE(23, 1, 0), tmp);
#if !defined(WLPROPRIETARY_11N_RATES)
		}
#else
		} else if (mcs >= WLC_11N_FIRST_PROP_MCS && mcs <= WLC_11N_LAST_PROP_MCS) {
			scb_ampdu->min_lens[MCS2IDX(mcs)] = CEIL(MCS_RATE(mcs, 1, 0), tmp);
		}
#endif /* WLPROPRIETARY_11N_RATES */
	}

	if (WLPROPRIETARY_11N_RATES_ENAB(scb_ampdu->scb->bsscfg->wlc->pub) &&
		scb_ampdu->scb->bsscfg->wlc->pub->ht_features != WLC_HT_FEATURES_PROPRATES_DISAB) {
			scb_ampdu->min_len = CEIL(MCS_RATE(WLC_11N_LAST_PROP_MCS, 1, 1), tmp);
	} else {
		scb_ampdu->min_len = CEIL(MCS_RATE(AMPDU_MAX_MCS, 1, 1), tmp);
	}
}

/** Called by higher software layer. Related to AMPDU density (gaps between PDUs in AMPDU) */
uint8 BCMFASTPATH
wlc_ampdu_null_delim_cnt(ampdu_tx_info_t *ampdu_tx, struct scb *scb, ratespec_t rspec,
	int phylen, uint16* minbytes)
{
	scb_ampdu_tx_t *scb_ampdu;
	int bytes = 0, cnt, tmp;
	uint8 tx_density;

	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	ASSERT(scb_ampdu);

	if (scb_ampdu->mpdu_density == 0)
		return 0;

	/* RSPEC2RATE is in kbps units ==> ~RSPEC2RATE/2^13 is in bytes/usec
	   density x is in 2^(x-3) usec
	   ==> # of bytes needed for req density = rate/2^(16-x)
	   ==> # of null delimiters = ceil(ceil(rate/2^(16-x)) - phylen)/4)
	 */

	tx_density = scb_ampdu->mpdu_density;

	/* XXX: ucode with BTC 3-wire adds some additional instructions; so need to
	 * bump up TX ampdu density to 8us
	 */
	if (wlc_btc_wire_get(ampdu_tx->wlc) >= WL_BTC_3WIRE)
		tx_density = AMPDU_DENSITY_8_US;

	ASSERT(tx_density <= AMPDU_MAX_MPDU_DENSITY);
	if (tx_density < 6 && WLC_HT_GET_FRAMEBURST(ampdu_tx->wlc->hti) &&
		RSPEC_IS40MHZ(rspec)) {
		int mcs = rspec & RSPEC_RATE_MASK;
		ASSERT(mcs <= AMPDU_MAX_MCS || mcs == 32 || IS_PROPRIETARY_11N_MCS(mcs));
		if (WLPROPRIETARY_11N_RATES_ENAB(ampdu_tx->wlc->pub)) {
			if (mcs <= AMPDU_MAX_MCS || mcs == 32 || IS_PROPRIETARY_11N_MCS(mcs))
				bytes = scb_ampdu->min_lens[MCS2IDX(mcs)];
		} else {
			if (mcs <= AMPDU_MAX_MCS)
				bytes = scb_ampdu->min_lens[mcs];
		}
	}
	if (bytes == 0) {
		tmp = 1 << (16 - tx_density);
		bytes = CEIL(RSPEC2RATE(rspec), tmp);
	}
	*minbytes = (uint16)bytes;

	if (bytes > phylen) {
		cnt = CEIL(bytes - phylen, AMPDU_DELIMITER_LEN);
		ASSERT(cnt <= 255);
		return (uint8)cnt;
	} else
		return 0;
} /* wlc_ampdu_null_delim_cnt */

void
wlc_ampdu_macaddr_upd(wlc_info_t *wlc)
{
	char template[T_RAM_ACCESS_SZ * 2];

	/* driver needs to write the ta in the template; ta is at offset 16 */
	bzero(template, sizeof(template));
	bcopy((char*)wlc->pub->cur_etheraddr.octet, template, ETHER_ADDR_LEN);
	wlc_write_template_ram(wlc, (T_BA_TPL_BASE + 16), (T_RAM_ACCESS_SZ * 2), template);
}

#if defined(MACOSX) && !defined(TXQ_MUX)

/** Return the # of transmit packets pending for given PRIO */
uint
wlc_ampdu_txpktcnt_tid(ampdu_tx_info_t *ampdu_tx, uint tid)
{
	struct scb *scb;
	struct scb_iter scbiter;
	int pktcnt = 0;
	scb_ampdu_tx_t *scb_ampdu;

	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			pktcnt += pktq_plen(&scb_ampdu->txq, tid);
		}
	}

	return pktcnt;
}

/** Return the pmax for given PRIO */
uint
wlc_ampdu_pmax_tid(ampdu_tx_info_t *ampdu_tx, uint tid)
{
	struct scb *scb;
	struct scb_iter scbiter;
	int pktmax = 0;
	scb_ampdu_tx_t *scb_ampdu;

	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			pktmax += pktq_pmax(&scb_ampdu->txq, tid);
			if (pktmax) /* kludge: non-zero just break out */
				break;
		}
	}

	return pktmax;
}

#endif /* defined(MACOSX) && !defined(TXQ_MUX) */

#if defined(WLNAR)
/**
 * WLNAR: provides balance amongst MPDU and AMPDU traffic by regulating the number of in-transit
 * packets for non-aggregating stations.
 */

/**
 * wlc_ampdu_ba_on_tidmask() - return a bitmask of aggregating TIDs (priorities).
 *
 * Inputs:
 *	scb	pointer to station control block to examine.
 *
 * Returns:
 *	The return value is a bitmask of TIDs for which a block ack agreement exists.
 *	Bit <00> = TID 0, bit <01> = TID 1, etc. A set bit indicates a BA agreement.
 */
extern uint8 BCMFASTPATH
wlc_ampdu_ba_on_tidmask(const struct scb *scb)
{
	wlc_info_t *wlc;
	uint8 mask = 0;
	scb_ampdu_tx_t *scb_ampdu;
	/*
	 * If AMPDU_MAX_SCB_TID changes to a larger value, we will need to adjust the
	 * return value from this function, as well as the calling functions.
	 */
	STATIC_ASSERT(AMPDU_MAX_SCB_TID <= NBITS(uint8));
	ASSERT(scb);
	ASSERT(SCB_AMPDU(scb));

	/* Figure out the wlc this scb belongs to. */
	wlc = SCB_BSSCFG(scb)->wlc;

	ASSERT(wlc);
	ASSERT(wlc->ampdu_tx);

	/* Locate the ampdu scb cubby, then check all TIDs */
	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	ASSERT(wlc->ampdu_tx->txaggr_support);
	if (scb_ampdu) {
		mask = scb_ampdu->tidmask;
	}

	return mask;
}

#endif /* WLNAR */

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST) || defined(BCMDBG_AMPDU)

int
wlc_ampdu_tx_dump(ampdu_tx_info_t *ampdu_tx, struct bcmstrbuf *b)
{
#ifdef WLCNT
	wlc_ampdu_tx_cnt_t *cnt = ampdu_tx->cnt;
#endif // endif
	wlc_info_t *wlc = ampdu_tx->wlc;
	wlc_pub_t *pub = wlc->pub;
	int i, last;
	uint32 max_val, total = 0;
	struct scb *scb;
	struct scb_iter scbiter;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	int inic = 0, ini_on = 0, ini_off = 0, ini_pon = 0, ini_poff = 0;
	int nbuf = 0;
#ifdef AMPDU_NON_AQM
	int j;
	wlc_fifo_info_t *fifo;
#endif // endif
	char eabuf[ETHER_ADDR_STR_LEN];
#ifdef WLPKTDLYSTAT
	uint ac;
	uint32 sum;
	scb_delay_stats_t *delay_stats;
#endif // endif
#ifdef  WLOVERTHRUSTER
	wlc_tcp_ack_info_t *tcp_ack_info = &ampdu_tx->tcp_ack_info;
#endif  /* WLOVERTHRUSTER */
#if defined(BCMDBG) || defined(WLATF_CNT)
	int tid;
#endif // endif
	BCM_REFERENCE(pub);

	bcm_bprintf(b, "HOST_ENAB %d UCODE_ENAB %d 4331_HW_ENAB %d AQM_ENAB %d\n",
		AMPDU_HOST_ENAB(pub), AMPDU_UCODE_ENAB(pub),
		AMPDU_HW_ENAB(pub), AMPDU_AQM_ENAB(pub));

	bcm_bprintf(b, "AMPDU Tx counters:\n");
#ifdef WLAMPDU_MAC
	if (wlc->clk && AMPDU_MAC_ENAB(pub) &&
		!AMPDU_AQM_ENAB(pub)) {

		uint32 hwampdu, hwmpdu, hwrxba, hwnoba;
		hwampdu = wlc_read_shm(wlc, MACSTAT_ADDR(MCSTOFF_TXAMPDU));
		hwmpdu = wlc_read_shm(wlc, MACSTAT_ADDR(MCSTOFF_TXMPDU));
		hwrxba = wlc_read_shm(wlc, MACSTAT_ADDR(MCSTOFF_RXBACK));
		hwnoba = (hwampdu < hwrxba) ? 0 : (hwampdu - hwrxba);
		bcm_bprintf(b, "%s: txampdu %d txmpdu %d txmpduperampdu %d noba %d (%d%%)\n",
			AMPDU_UCODE_ENAB(pub) ? "Ucode" : "HW",
			hwampdu, hwmpdu,
			hwampdu ? CEIL(hwmpdu, hwampdu) : 0, hwnoba,
			hwampdu ? CEIL(hwnoba*100, hwampdu) : 0);
	}
#endif /* WLAMPDU_MAC */

#ifdef WLCNT

#ifdef WLAMPDU_MAC
	if (wlc->clk && AMPDU_MAC_ENAB(pub)) {
		if (!AMPDU_AQM_ENAB(pub))
		bcm_bprintf(b, "txmpdu(enq) %d txs %d txmrt/succ %d %d (f %d%%) "
			"txfbr/succ %d %d (f %d%%)\n", cnt->txmpdu, cnt->u0.txs,
			cnt->tx_mrt, cnt->txsucc_mrt,
			cnt->tx_mrt ? CEIL((cnt->tx_mrt - cnt->txsucc_mrt)*100, cnt->tx_mrt) : 0,
			cnt->tx_fbr, cnt->txsucc_fbr,
			cnt->tx_fbr ? CEIL((cnt->tx_fbr - cnt->txsucc_fbr)*100, cnt->tx_fbr) : 0);
	} else
#endif /* WLAMPDU_MAC */
	{
		bcm_bprintf(b, "txampdu %d txmpdu %d txmpduperampdu %d noba %d (%d%%)\n",
			cnt->txampdu, cnt->txmpdu,
			cnt->txampdu ? CEIL(cnt->txmpdu, cnt->txampdu) : 0, cnt->noba,
			cnt->txampdu ? CEIL(cnt->noba*100, cnt->txampdu) : 0);
		bcm_bprintf(b, "retry_ampdu %d retry_mpdu %d (%d%%) txfifofull %d\n",
		  cnt->txretry_ampdu, cnt->u0.txretry_mpdu,
		  (cnt->txmpdu ? CEIL(cnt->u0.txretry_mpdu*100, cnt->txmpdu) : 0), cnt->txfifofull);
		bcm_bprintf(b, "fbr_ampdu %d fbr_mpdu %d\n",
			cnt->txfbr_ampdu, cnt->txfbr_mpdu);
	}
	bcm_bprintf(b, "txregmpdu %d txreg_noack %d txfifofull %d txdrop %d txstuck %d orphan %d\n",
		cnt->txregmpdu, cnt->txreg_noack, cnt->txfifofull,
		cnt->txdrop, cnt->txstuck, cnt->orphan);
	bcm_bprintf(b, "txrel_wm %d txrel_size %d sduretry %d sdurejected %d\n",
		cnt->txrel_wm, cnt->txrel_size, cnt->sduretry, cnt->sdurejected);

#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(pub)) {
		bcm_bprintf(b, "aggfifo_w %d epochdeltas %d mpduperepoch %d\n",
			cnt->enq, cnt->epochdelta,
			(cnt->epochdelta+1) ? CEIL(cnt->enq, (cnt->epochdelta+1)) : 0);
		bcm_bprintf(b, "epoch_change reason: plcp %d rate %d fbr %d reg %d link %d"
			" seq no %d\n", cnt->echgr1, cnt->echgr2, cnt->echgr3,
			cnt->echgr4, cnt->echgr5, cnt->echgr6);
	}
#endif // endif
	bcm_bprintf(b, "txr0hole %d txrnhole %d txrlag %d rxunexp %d\n",
		cnt->txr0hole, cnt->txrnhole, cnt->txrlag, cnt->rxunexp);
	bcm_bprintf(b, "txaddbareq %d rxaddbaresp %d txlost %d txbar %d rxba %d txdelba %d \n",
		cnt->txaddbareq, cnt->rxaddbaresp, cnt->txlost, cnt->txbar,
		cnt->rxba, cnt->txdelba);

#ifdef WLAMPDU_MAC
	if (AMPDU_MAC_ENAB(pub))
		bcm_bprintf(b, "txmpdu_sgi %d txmpdu_stbc %d\n",
		  cnt->u1.txmpdu_sgi, cnt->u2.txmpdu_stbc);
	else
#endif // endif
		bcm_bprintf(b, "txampdu_sgi %d txampdu_stbc %d "
			"txampdu_mfbr_stbc %d\n", cnt->u1.txampdu_sgi,
			cnt->u2.txampdu_stbc, cnt->txampdu_mfbr_stbc);

#ifdef WL_CS_PKTRETRY
	bcm_bprintf(b, "cs_pktretry_cnt %u\n", cnt->cs_pktretry_cnt);
#endif // endif

#endif /* WLCNT */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			ASSERT(scb_ampdu);
			for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
				if ((ini = scb_ampdu->ini[i])) {
					inic++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_OFF)
						ini_off++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_ON)
						ini_on++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF)
						ini_poff++;
					if (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_ON)
						ini_pon++;
				}
			}
			nbuf += pktq_len(&scb_ampdu->txq);
		}
	}

	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "ini %d ini_off %d ini_on %d ini_poff %d ini_pon %d nbuf %d ampdu_wds %u\n",
		inic, ini_off, ini_on, ini_poff, ini_pon, nbuf, cnt->ampdu_wds);
#ifdef WLOVERTHRUSTER
	if ((OVERTHRUST_ENAB(pub)) && (tcp_ack_info->tcp_ack_ratio)) {
		bcm_bprintf(b, "tcp_ack_ratio %d/%d total %d/%d dequeued %d multi_dequeued %d\n",
			tcp_ack_info->tcp_ack_ratio, tcp_ack_info->tcp_ack_ratio_depth,
			tcp_ack_info->tcp_ack_total - tcp_ack_info->tcp_ack_dequeued
			- tcp_ack_info->tcp_ack_multi_dequeued,
			tcp_ack_info->tcp_ack_total,
			tcp_ack_info->tcp_ack_dequeued, tcp_ack_info->tcp_ack_multi_dequeued);
	}
#endif /* WLOVERTHRUSTER */

	bcm_bprintf(b, "Supr Reason: pmq(%d) flush(%d) frag(%d) badch(%d) exptime(%d) uf(%d)",
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_PMQ],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_FLUSH],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_FRAG],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_BADCH],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_EXPTIME],
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_UF]);

#ifdef WLP2P
	if (P2P_ENAB(pub))
		bcm_bprintf(b, " abs(%d)",
			ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_NACK_ABS]);
#endif // endif
#ifdef AP
	bcm_bprintf(b, " pps(%d)",
		ampdu_tx->amdbg->supr_reason[TX_STATUS_SUPR_PPS]);
#endif // endif
	bcm_bprintf(b, "\n\n");

#ifdef WLAMPDU_HW
	if (wlc->clk && AMPDU_HW_ENAB(pub)) {
		uint16 stat_addr = 2 * wlc_read_shm(wlc, M_HWAGG_STATS_PTR);
		stat_addr += C_HWAGG_STATS_MPDU_WSZ * 2;
		for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
			ampdu_tx->amdbg->txmcs[i] = wlc_read_shm(wlc, stat_addr + i * 2);
	}
#endif // endif

	/* determines highest MCS array *index* on which a transmit took place */
	for (i = 0, total = 0, last = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
		total += ampdu_tx->amdbg->txmcs[i];
		if (ampdu_tx->amdbg->txmcs[i]) last = i;
	}

	if (last <= AMPDU_MAX_MCS) { /* skips proprietary 11N MCS'es */
		/* round up to highest MCS array index with same number of spatial streams */
		last = 8 * (last / 8 + 1) - 1;
	}

	bcm_bprintf(b, "TX MCS  :");
	if (total) {
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txmcs[i],
				(ampdu_tx->amdbg->txmcs[i] * 100) / total);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
	bcm_bprintf(b, "\n");

#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(pub)) {
		bcm_bprintf(b, "MCS PER :");
		if (total) {
			for (i = 0; i <= last; i++) {
				int unacked = 0, per = 0;
				unacked = ampdu_tx->amdbg->txmcs[i] -
					ampdu_tx->amdbg->txmcs_succ[i];
				if (unacked < 0) unacked = 0;
				if ((unacked > 0) && (ampdu_tx->amdbg->txmcs[i]))
					per = (unacked * 100) / ampdu_tx->amdbg->txmcs[i];
				bcm_bprintf(b, "  %d(%d%%)", unacked, per);
				if ((i % 8) == 7 && i != last)
					bcm_bprintf(b, "\n        :");
			}
		}
		bcm_bprintf(b, "\n");
	}
#endif /* WLAMPDU_MAC */

#ifdef WL11AC
	for (i = 0, total = 0, last = 0; i < AMPDU_MAX_VHT; i++) {
		total += ampdu_tx->amdbg->txvht[i];
		if (ampdu_tx->amdbg->txvht[i]) last = i;
	}
	last = MAX_VHT_RATES * (last/MAX_VHT_RATES + 1) - 1;
	bcm_bprintf(b, "TX VHT  :");
	if (total) {
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txvht[i],
				(ampdu_tx->amdbg->txvht[i] * 100) / total);
			if ((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1) && i != last)
				bcm_bprintf(b, "\n        :");
		}
	}
	bcm_bprintf(b, "\n");
#ifdef WLAMPDU_MAC
	if (AMPDU_AQM_ENAB(pub)) {
		bcm_bprintf(b, "VHT PER :");
		if (total) {
			for (i = 0; i <= last; i++) {
				int unacked = 0, per = 0;
				unacked = ampdu_tx->amdbg->txvht[i] -
					ampdu_tx->amdbg->txvht_succ[i];
				if (unacked < 0) unacked = 0;
				if ((unacked > 0) && (ampdu_tx->amdbg->txvht[i]))
					per = (unacked * 100) / ampdu_tx->amdbg->txvht[i];
				bcm_bprintf(b, "  %d(%d%%)", unacked, per);
				if (((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1)) && (i != last))
					bcm_bprintf(b, "\n        :");
			}
		}
		bcm_bprintf(b, "\n");
	}
#endif /* WLAMPDU_MAC */
#endif /* WL11AC */

#if defined(WLPKTDLYSTAT) && defined(WLCNT)
#if defined(WLAMPDU_HW)
	if (wlc->clk && AMPDU_HW_ENAB(pub)) {
		uint16 stat_addr = 2 * wlc_read_shm(wlc, M_HWAGG_STATS_PTR);
		stat_addr += C_HWAGG_STATS_MPDU_WSZ * 2;

#if defined(WLPROPRIETARY_11N_RATES)
		i = -1;
		while (TRUE) {
			i = NEXT_MCS(i);
			if (i > WLC_11N_LAST_PROP_MCS)
				break;
#else
		for (i  = 0; i <= AMPDU_MAX_MCS; i++) {
#endif /* WLPROPRIETARY_11N_RATES */
			cnt->txmpdu_cnt[i] = wlc_read_shm(wlc, stat_addr + i * 2);
			/*
			 * cnt->txmpdu_succ_cnt[i] =
			 * 	wlc_read_shm(wlc, stat_addr + i * 2 + C_HWAGG_STATS_TXMCS_WSZ * 2);
			 */
		}
	}
#endif /* WLAMPDU_HW */

	/* Report PER statistics (for each MCS) */
	for (i = 0, max_val = 0, last = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
		max_val += cnt->txmpdu_cnt[i];
		if (cnt->txmpdu_cnt[i])
			last = i; /* highest used MCS array index */
	}

	if (last <= AMPDU_MAX_MCS) { /* skips proprietary 11N MCS'es */
		/* round up to highest MCS array index with same number of spatial streams */
		last = 8 * (last / 8 + 1) - 1;
	}

	if (max_val) {
		bcm_bprintf(b, "PER : ");
		for (i = 0; i <= last; i++) {
			int unacked = 0;
			unacked = cnt->txmpdu_cnt[i] - cnt->txmpdu_succ_cnt[i];
			if (unacked < 0) {
				unacked = 0;
			}
			bcm_bprintf(b, "  %d/%d (%d%%)",
			            (cnt->txmpdu_cnt[i] - cnt->txmpdu_succ_cnt[i]),
			            cnt->txmpdu_cnt[i],
			            ((cnt->txmpdu_cnt[i] > 0) ?
			             (unacked * 100) / cnt->txmpdu_cnt[i] : 0));
			if ((i % 8) == 7 && i != last) {
				bcm_bprintf(b, "\n    : ");
			}
		}
		bcm_bprintf(b, "\n");
	}
#endif /* WLPKTDLYSTAT && WLCNT */

#if defined(WLAMPDU_HW) || defined(WLAMPDU_AQM)
	if (wlc->clk) {
		uint16 stat_addr = 0;
		uint16 val, ucode_ampdu;
		int cnt1 = 0, cnt2 = 0;
		uint16 nbins;

		nbins = sizeof(ampdu_tx->amdbg->mpdu_histogram);
		if (nbins > C_MPDUDEN_NBINS)
			nbins = C_MPDUDEN_NBINS;
		if (AMPDU_HW_ENAB(pub) || AMPDU_AQM_ENAB(pub)) {
			stat_addr = 2 * wlc_read_shm(wlc,
				(AMPDU_HW_ENAB(pub)) ? M_HWAGG_STATS_PTR : M_AMP_STATS_PTR);
			for (i = 0, total = 0; i < nbins; i++) {
				val = wlc_read_shm(wlc, stat_addr + i * 2);
				ampdu_tx->amdbg->mpdu_histogram[i] = val;
				total += val;
				cnt1 += (val * (i+1));
			}
			bcm_bprintf(b, "--------------------------\n");
			if (AMPDU_HW_ENAB(pub)) {
				int rempty;
				rempty = wlc_read_shm(wlc, stat_addr + C_HWAGG_RDEMP_WOFF * 2);
				ucode_ampdu = wlc_read_shm(wlc, stat_addr + C_HWAGG_NAMPDU_WOFF*2);
				bcm_bprintf(b, "tot_mpdus %d tot_ampdus %d mpduperampdu %d "
				    "ucode_ampdu %d rempty %d (%d%%)\n",
				    cnt1, total,
				    (total == 0) ? 0 : CEIL(cnt1, total),
				    ucode_ampdu, rempty,
				    (ucode_ampdu == 0) ? 0 : CEIL(rempty * 100, ucode_ampdu));
			} else {
				bcm_bprintf(b, "tot_mpdus %d tot_ampdus %d mpduperampdu %d\n",
					cnt1, total, (total == 0) ? 0 : CEIL(cnt1, total));
			}
		}

		if (AMPDU_AQM_ENAB(pub)) {
			/* print out agg stop reason */
			uint16 stop_len, stop_mpdu, stop_bawin, stop_epoch, stop_fempty;
			stat_addr  += (C_MPDUDEN_NBINS * 2);
			stop_len    = wlc_read_shm(wlc, stat_addr);
			stop_mpdu   = wlc_read_shm(wlc, stat_addr + 2);
			stop_bawin  = wlc_read_shm(wlc, stat_addr + 4);
			stop_epoch  = wlc_read_shm(wlc, stat_addr + 6);
			stop_fempty = wlc_read_shm(wlc, stat_addr + 8);
			total = stop_len + stop_mpdu + stop_bawin + stop_epoch + stop_fempty;
			if (total) {
				bcm_bprintf(b, "agg stop reason: len %d (%d%%) ampdu_mpdu %d (%d%%)"
					    " bawin %d (%d%%) epoch %d (%d%%) fempty %d (%d%%)\n",
					    stop_len, (stop_len * 100) / total,
					    stop_mpdu, (stop_mpdu * 100) / total,
					    stop_bawin, (stop_bawin * 100) / total,
					    stop_epoch, (stop_epoch * 100) / total,
					    stop_fempty, (stop_fempty * 100) / total);
			}
			stat_addr  += C_AGGSTOP_NBINS * 2;
		}

		if (WLC_HT_GET_FRAMEBURST(ampdu_tx->wlc->hti) &&
			(D11REV_IS(pub->corerev, 26) ||
		    D11REV_IS(pub->corerev, 29) || AMPDU_AQM_ENAB(pub))) {
			/* burst size */
			cnt1 = 0;
			if (!AMPDU_AQM_ENAB(pub))
				stat_addr += (C_MBURST_WOFF * 2);
			bcm_bprintf(b, "Frameburst histogram:");
			for (i = 0; i < C_MBURST_NBINS; i++) {
				val = wlc_read_shm(wlc, stat_addr + i * 2);
				cnt1 += val;
				cnt2 += (i+1) * val;
				bcm_bprintf(b, "  %d", val);
			}
			bcm_bprintf(b, " avg %d\n", (cnt1 == 0) ? 0 : CEIL(cnt2, cnt1));
			bcm_bprintf(b, "--------------------------\n");
		}
	}
#endif /* WLAMPDU_HW */

	for (i = 0, last = 0, total = 0; i <= AMPDU_MAX_MPDU; i++) {
		total += ampdu_tx->amdbg->mpdu_histogram[i];
		if (ampdu_tx->amdbg->mpdu_histogram[i])
			last = i;
	}
	last = 8 * (last/8 + 1) - 1;
	bcm_bprintf(b, "MPDUdens:");
	for (i = 0; i <= last; i++) {
		bcm_bprintf(b, " %3d (%d%%)", ampdu_tx->amdbg->mpdu_histogram[i],
			(total == 0) ? 0 :
			(ampdu_tx->amdbg->mpdu_histogram[i] * 100 / total));
		if ((i % 8) == 7 && i != last)
			bcm_bprintf(b, "\n        :");
	}
	bcm_bprintf(b, "\n");

	if (AMPDU_HOST_ENAB(pub)) {
		for (i = 0, last = 0; i <= AMPDU_MAX_MPDU; i++)
			if (ampdu_tx->amdbg->retry_histogram[i])
				last = i;
		bcm_bprintf(b, "Retry   :");
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, " %3d", ampdu_tx->amdbg->retry_histogram[i]);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
		bcm_bprintf(b, "\n");

		for (i = 0, last = 0; i <= AMPDU_MAX_MPDU; i++)
			if (ampdu_tx->amdbg->end_histogram[i])
				last = i;
		bcm_bprintf(b, "Till End:");
		for (i = 0; i <= last; i++) {
			bcm_bprintf(b, " %3d", ampdu_tx->amdbg->end_histogram[i]);
			if ((i % 8) == 7 && i != last)
				bcm_bprintf(b, "\n        :");
		}
		bcm_bprintf(b, "\n");
	}

	if (WLC_SGI_CAP_PHY(wlc)) {
		bcm_bprintf(b, "TX MCS SGI:");
		for (i = 0, max_val = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
			max_val += ampdu_tx->amdbg->txmcssgi[i];
			if (ampdu_tx->amdbg->txmcssgi[i]) last = i;
		}

		if (last <= AMPDU_MAX_MCS) { /* skips proprietary 11N MCS'es */
			/* round up to highest MCS array idx with same number of spatial streams */
			last = 8 * (last / 8 + 1) - 1;
		}

		if (max_val) {
			for (i = 0; i <= last; i++) {
				bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txmcssgi[i],
				            (ampdu_tx->amdbg->txmcssgi[i] * 100) / max_val);
				if ((i % 8) == 7 && i != last)
					bcm_bprintf(b, "\n          :");
			}
		}
		bcm_bprintf(b, "\n");
#ifdef WL11AC
		bcm_bprintf(b, "TX VHT SGI:");
		for (i = 0, max_val = 0; i < AMPDU_MAX_VHT; i++) {
			max_val += ampdu_tx->amdbg->txvhtsgi[i];
			if (ampdu_tx->amdbg->txvhtsgi[i]) last = i;
		}
		last = MAX_VHT_RATES * (last/MAX_VHT_RATES + 1) - 1;
		if (max_val) {
			for (i = 0; i <= last; i++) {
					bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txvhtsgi[i],
						(ampdu_tx->amdbg->txvhtsgi[i] * 100) / max_val);
				if (((i % MAX_VHT_RATES) == (MAX_VHT_RATES - 1)) && i != last)
						bcm_bprintf(b, "\n          :");
			}
		}
		bcm_bprintf(b, "\n");
#endif /* WL11AC */

		if (WLC_STBC_CAP_PHY(wlc)) {
			bcm_bprintf(b, "TX MCS STBC:");
			for (i = 0, max_val = 0; i <= AMPDU_HT_MCS_LAST_EL; i++)
				max_val += ampdu_tx->amdbg->txmcsstbc[i];
			if (max_val) {
				for (i = 0; i <= 7; i++)
					bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txmcsstbc[i],
						(ampdu_tx->amdbg->txmcsstbc[i] * 100) / max_val);
			}
			bcm_bprintf(b, "\n");
#ifdef WL11AC
			for (i = 0, max_val = 0; i < AMPDU_MAX_VHT; i++)
				max_val += ampdu_tx->amdbg->txvhtstbc[i];
			bcm_bprintf(b, "TX VHT STBC:");
			if (max_val) {
				for (i = 0; i < MAX_VHT_RATES; i++) {
					bcm_bprintf(b, "  %d(%d%%)", ampdu_tx->amdbg->txvhtstbc[i],
					(ampdu_tx->amdbg->txvhtstbc[i] * 100) / max_val);
				}
			}
			bcm_bprintf(b, "\n");
#endif /* WL11AC */
		}
	}

#ifdef AMPDU_NON_AQM
	bcm_bprintf(b, "MCS to AMPDU tables:\n");
	for (j = 0; j < NUM_FFPLD_FIFO; j++) {
		fifo = ampdu_tx->config->fifo_tb + j;
		if (fifo->ampdu_pld_size || fifo->dmaxferrate) {
			bcm_bprintf(b, " FIFO %d: Preload settings: size %d dmarate %d kbps\n",
			          j, fifo->ampdu_pld_size, fifo->dmaxferrate);
			bcm_bprintf(b, "       :");
			for (i = 0; i <= AMPDU_HT_MCS_LAST_EL; i++) {
				bcm_bprintf(b, " %d", fifo->mcs2ampdu_table[i]);
				if ((i % 8) == 7 && i != AMPDU_HT_MCS_LAST_EL)
					bcm_bprintf(b, "\n       :");
			}
			bcm_bprintf(b, "\n");
		}
	}
	bcm_bprintf(b, "\n");
#endif /* AMPDU_NON_AQM */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
			bcm_bprintf(b, "%s: max_pdu %d release %d\n",
				bcm_ether_ntoa(&scb->ea, eabuf),
				scb_ampdu->max_pdu, scb_ampdu->release);
#if defined(BCMDBG) || defined(WLATF_CNT)
#ifdef BCMDBG
			bcm_bprintf(b, "\ttxdrop %u txstuck %u "
				  "txaddbareq %u txrlag %u sdurejected %u\n"
				  "\ttxmpdu %u txlost %u txbar %d txreg_noack %u noba %u "
				  "rxaddbaresp %u\n",
				  scb_ampdu->cnt.txdrop, scb_ampdu->cnt.txstuck,
				  scb_ampdu->cnt.txaddbareq, scb_ampdu->cnt.txrlag,
				  scb_ampdu->cnt.sdurejected, scb_ampdu->cnt.txmpdu,
				  scb_ampdu->cnt.txlost, scb_ampdu->cnt.txbar,
				  scb_ampdu->cnt.txreg_noack, scb_ampdu->cnt.noba,
				  scb_ampdu->cnt.rxaddbaresp);
			bcm_bprintf(b, "\ttxnoroom %u\n", scb_ampdu->cnt.txnoroom);
#endif /* BCMDBG */

			for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
				ini = scb_ampdu->ini[tid];
				if (!ini)
					continue;
				if (!(ini->ba_state == AMPDU_TID_STATE_BA_ON))
					continue;
#ifdef BCMDBG
				bcm_bprintf(b, "\tba_state %d ba_wsize %d tx_in_transit %d "
					"tid %d rem_window %d\n",
					ini->ba_state, ini->ba_wsize, ini->tx_in_transit,
					ini->tid, ini->rem_window);
				bcm_bprintf(b, "\tstart_seq 0x%x max_seq 0x%x tx_exp_seq 0x%x"
					" bar_ackpending_seq 0x%x\n",
					ini->start_seq, ini->max_seq, ini->tx_exp_seq,
					ini->bar_ackpending_seq);
				bcm_bprintf(b, "\tbar_ackpending %d alive %d retry_bar %d\n",
					ini->bar_ackpending, ini->alive, ini->retry_bar);
#endif /* BCMDBG */
				wlc_ampdu_atf_dump(AMPDU_ATF_STATE(ini), b);
			}
#endif /* BCMDBG || WLATF_CNT */
		}
#ifdef WLPKTDLYSTAT
		/* Report Latency and packet loss statistics (for each AC) */
		for (ac = 0; ac < AC_COUNT; ac++) {
			if (!scb->delay_stats[ac])
				continue;
			delay_stats = scb->delay_stats[ac];
			for (i = 0, total = 0, sum = 0; i < RETRY_SHORT_DEF; i++) {
				total += delay_stats->txmpdu_cnt[i];
				sum += delay_stats->delay_sum[i];
			}
			if (total) {
				bcm_bprintf(b, "%s:  PLoss %d/%d (%d%%)\n",
				 ac_names[ac], delay_stats->txmpdu_lost, total,
				  (delay_stats->txmpdu_lost * 100) / total);
#if defined(WLC_HIGH_ONLY)
				bcm_bprintf(b, "\tDelay stats in ms (avg/min/max): %d %d %d\n",
				 CEIL(sum, total), delay_stats->delay_min, delay_stats->delay_max);
#else
				bcm_bprintf(b, "\tDelay stats in usec (avg/min/max): %d %d %d\n",
				 CEIL(sum, total), delay_stats->delay_min, delay_stats->delay_max);
#endif // endif
				bcm_bprintf(b, "\tTxcnt Hist    :");
				for (i = 0; i < RETRY_SHORT_DEF; i++) {
					bcm_bprintf(b, "  %d", delay_stats->txmpdu_cnt[i]);
				}
				bcm_bprintf(b, "\n\tDelay vs Txcnt:");
				for (i = 0; i < RETRY_SHORT_DEF; i++) {
					if (delay_stats->txmpdu_cnt[i])
						bcm_bprintf(b, "  %d", delay_stats->delay_sum[i]/
						delay_stats->txmpdu_cnt[i]);
					else
						bcm_bprintf(b, "  -1");
				}
				bcm_bprintf(b, "\n");

				for (i = 0, max_val = 0, last = 0; i < WLPKTDLY_HIST_NBINS; i++) {
					max_val += delay_stats->delay_hist[i];
					if (delay_stats->delay_hist[i]) last = i;
				}
				last = 8 * (last/8 + 1) - 1;
				if (max_val) {
					bcm_bprintf(b, "\tDelay Hist \n\t\t:");
					for (i = 0; i <= last; i++) {
						bcm_bprintf(b, " %d(%d%%)", delay_stats->
						delay_hist[i], (delay_stats->delay_hist[i] * 100)
						 / max_val);
						if ((i % 8) == 7 && i != last)
							bcm_bprintf(b, "\n\t\t:");
					}
				}
				bcm_bprintf(b, "\n");
			}
		}
		bcm_bprintf(b, "\n");
#endif /* WLPKTDLYSTAT */
	}

#ifdef WLAMPDU_HW
	if (wlc->clk && AMPDU_HW_ENAB(pub) && b) {
		d11regs_t* regs = wlc->regs;
		bcm_bprintf(b, "AGGFIFO regs: availcnt 0x%x\n", R_REG(wlc->osh, &regs->aggfifocnt));
		bcm_bprintf(b, "cmd 0x%04x stat 0x%04x cfgctl 0x%04x cfgdata 0x%04x mpdunum 0x%02x "
			"len 0x%04x bmp 0x%04x ackedcnt %d\n",
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_cmd),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_stat),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_cfgctl),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_cfgdata),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_mpdunum),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_len),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_bmp),
			R_REG(wlc->osh, &regs->u.d11regs.aggfifo_ackedcnt));
#if defined(WLCNT) && defined(WLAMPDU_MAC)
		bcm_bprintf(b, "driver statistics: aggfifo pending %d enque/cons %d %d",
			cnt->pending, cnt->enq, cnt->cons);
#endif // endif
	}
#endif /* WLAMPDU_HW */
	bcm_bprintf(b, "\n");

	return 0;
} /* wlc_ampdu_tx_dump */

#endif /* BCMDBG || WLTEST */

#ifdef BCMDBG
static void
ampdu_dump_ini(scb_ampdu_tid_ini_t *ini)
{
	printf("ba_state %d ba_wsize %d tx_in_transit %d tid %d rem_window %d\n",
		ini->ba_state, ini->ba_wsize, ini->tx_in_transit, ini->tid, ini->rem_window);
	printf("start_seq 0x%x max_seq 0x%x tx_exp_seq 0x%x bar_ackpending_seq 0x%x\n",
		ini->start_seq, ini->max_seq, ini->tx_exp_seq, ini->bar_ackpending_seq);
	printf("bar_ackpending %d alive %d retry_bar %d\n",
		ini->bar_ackpending, ini->alive, ini->retry_bar);
	printf("retry_head %d retry_tail %d retry_cnt %d\n",
		ini->retry_head, ini->retry_tail, ini->retry_cnt);
	if (AMPDU_HOST_ENAB(ini->scb->bsscfg->wlc->pub)) {
	prhex("ackpending", &ini->ackpending[0], sizeof(ini->ackpending));
	prhex("barpending", &ini->barpending[0], sizeof(ini->barpending));
	prhex("txretry", &ini->txretry[0], sizeof(ini->txretry));
	prhex("retry_seq", (uint8 *)&ini->retry_seq[0], sizeof(ini->retry_seq));
	}
}
#endif /* BCMDBG */

#if defined(BCMDBG) && defined(WLAMPDU_MAC)
static int
wlc_dyn_fb_dump(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	ampdu_tx_config_t *ampdu_tx_cfg;
	bool dynfb = 0;
	if (ampdu_tx) {
		ampdu_tx_cfg = ampdu_tx->config;
		if (wlc->hti->frameburst && ampdu_tx_cfg->fb_override_enable) {
			dynfb = 1;
		}
#ifdef WLMCHAN
		if (MCHAN_ACTIVE(wlc->pub)) {
			bcm_bprintf(b, "Dynamic framebursting is unsupported for MCHAN\n");
			return 0;
		}
#endif // endif
		bcm_bprintf(b, "dynfb configured %d, rtsper %d%%, fb state %s\n",
				dynfb,
				ampdu_tx->rtsper_avg,
				(dynfb && !ampdu_tx_cfg->fb_override) ? "ON":"OFF");
	}
	return 0;
}
#endif /* BCMDBG && WLAMPDU_MAC */

/**
 * Sidechannel is a firmware<->ucode interface, intended to prepare the ucode for upcoming
 * transmits, so ucode can allocate data structures.
 */
void
wlc_sidechannel_init(ampdu_tx_info_t *ampdu_tx)
{
	wlc_info_t *wlc = ampdu_tx->wlc;
#ifdef WLAMPDU_UCODE
	uint16 txfs_waddr; /* side channel base addr in 16 bit units */
	uint16 txfsd_addr; /* side channel descriptor base addr */
	/* Size of side channel fifos in 16 bit units */
	uint8 txfs_wsz[AC_COUNT] =
	        {TXFS_WSZ_AC_BK, TXFS_WSZ_AC_BE, TXFS_WSZ_AC_VI, TXFS_WSZ_AC_VO};
	ampdumac_info_t *hagg;
	int i;
#endif /* WLAMPDU_UCODE */

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!(D11REV_IS(wlc->pub->corerev, 26) || D11REV_IS(wlc->pub->corerev, 29)) ||
	    !AMPDU_MAC_ENAB(wlc->pub) || AMPDU_AQM_ENAB(wlc->pub)) {
		WL_INFORM(("wl%d: %s; NOT UCODE or HW aggregation or side channel"
			"not supported\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	ASSERT(ampdu_tx);

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(wlc->pub)) {
		if (!(ampdu_tx->config->ini_enable[PRIO_8021D_VO] ||
			ampdu_tx->config->ini_enable[PRIO_8021D_NC])) {
			txfs_wsz[1] = txfs_wsz[1] + txfs_wsz[3];
			txfs_wsz[3] = 0;
		}
		if (!(ampdu_tx->config->ini_enable[PRIO_8021D_NONE] ||
			ampdu_tx->config->ini_enable[PRIO_8021D_BK])) {
				txfs_wsz[1] = txfs_wsz[1] + txfs_wsz[0];
				txfs_wsz[0] = 0;
		}
		ASSERT(txfs_wsz[0] + txfs_wsz[1] + txfs_wsz[2] + txfs_wsz[3] <= TOT_TXFS_WSIZE);
		txfs_waddr = wlc_read_shm(wlc, M_TXFS_PTR);
		txfsd_addr = (txfs_waddr + C_TXFSD_WOFFSET) * 2;
		for (i = 0; i < AC_COUNT; i++) {
			/* 16 bit word arithmetic */
			hagg = &(ampdu_tx->hagg[i]);
			hagg->txfs_addr_strt = txfs_waddr;
			hagg->txfs_addr_end = txfs_waddr + txfs_wsz[i] - 1;
			hagg->txfs_wptr = hagg->txfs_addr_strt;
			hagg->txfs_rptr = hagg->txfs_addr_strt;
			hagg->txfs_wsize = txfs_wsz[i];
			/* write_shmem takes a 8 bit address and 16 bit pointer */
			wlc_write_shm(wlc, C_TXFSD_STRT_POS(txfsd_addr, i),
				hagg->txfs_addr_strt);
			wlc_write_shm(wlc, C_TXFSD_END_POS(txfsd_addr, i),
				hagg->txfs_addr_end);
			wlc_write_shm(wlc, C_TXFSD_WPTR_POS(txfsd_addr, i), hagg->txfs_wptr);
			wlc_write_shm(wlc, C_TXFSD_RPTR_POS(txfsd_addr, i), hagg->txfs_rptr);
			wlc_write_shm(wlc, C_TXFSD_RNUM_POS(txfsd_addr, i), 0);
			WL_AMPDU_HW(("%d: start 0x%x 0x%x, end 0x%x 0x%x, w 0x%x 0x%x,"
				" r 0x%x 0x%x, sz 0x%x 0x%x\n",
				i,
				C_TXFSD_STRT_POS(txfsd_addr, i), hagg->txfs_addr_strt,
				C_TXFSD_END_POS(txfsd_addr, i),  hagg->txfs_addr_end,
				C_TXFSD_WPTR_POS(txfsd_addr, i), hagg->txfs_wptr,
				C_TXFSD_RPTR_POS(txfsd_addr, i), hagg->txfs_rptr,
				C_TXFSD_RNUM_POS(txfsd_addr, i), 0));
			hagg->txfs_wptr_addr = C_TXFSD_WPTR_POS(txfsd_addr, i);
			txfs_waddr += txfs_wsz[i];
		}
	}
#endif /* WLAMPDU_UCODE */

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(wlc->pub)) {
		ampdu_tx->config->aggfifo_depth = AGGFIFO_CAP - 1;
	}
#endif // endif
} /* wlc_sidechannel_init */

#ifdef WLAMPDU_MAC

/**
 * For ucode aggregation:
 * enqueue an entry to side channel (a.k.a. aggfifo) and inc the wptr
 * The actual capacity of the buffer is one less than the actual length
 * of the buffer so that an empty and a full buffer can be
 * distinguished.  An empty buffer will have the readPostion and the
 * writePosition equal to each other.  A full buffer will have
 * the writePosition one less than the readPostion.
 *
 * The Objects available to be read go from the readPosition to the writePosition,
 * wrapping around the end of the buffer.  The space available for writing
 * goes from the write position to one less than the readPosition,
 * wrapping around the end of the buffer.
 *
 * For hw agg: it simply enqueues an entry to aggfifo
 */
#ifdef AMPDU_NON_AQM

#if !defined(TXQ_MUX)
static int
aggfifo_enque(ampdu_tx_info_t *ampdu_tx, int length, int qid)
{
	ampdumac_info_t *hagg = &(ampdu_tx->hagg[qid]);
#if defined(WLAMPDU_HW) || defined(WLAMPDU_UCODE)
	uint16 epoch = hagg->epoch;
	uint32 entry;

	if (length > (MPDU_LEN_MASK >> MPDU_LEN_SHIFT))
		WL_ERROR(("%s: Length too long %d\n", __FUNCTION__, length));
#endif // endif

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
		d11regs_t *regs = ampdu_tx->wlc->regs;

		entry = length |
			((epoch ? 1 : 0) << MPDU_EPOCH_HW_SHIFT) | (qid << MPDU_FIFOSEL_SHIFT);

		W_REG(ampdu_tx->wlc->osh, &regs->aggfifodata, entry);
		WLCNTINCR(ampdu_tx->cnt->enq);
		WLCNTINCR(ampdu_tx->cnt->pending);
		hagg->in_queue ++;
		WL_AMPDU_HW(("%s: aggfifo %d entry 0x%04x\n", __FUNCTION__, qid, entry));
	}
#endif /* WLAMPDU_HW */

#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub)) {
		uint16 rptr = hagg->txfs_rptr;
		uint16 wptr_next;

		entry = length | (((epoch ? 1 : 0)) << MPDU_EPOCH_SHIFT);

		/* wptr always points to the next available entry to be written */
		if (hagg->txfs_wptr == hagg->txfs_addr_end) {
			wptr_next = hagg->txfs_addr_strt;	/* wrap */
		} else {
			wptr_next = hagg->txfs_wptr + 1;
		}

		if (wptr_next == rptr) {
			WL_ERROR(("%s: side channel %d is full !!!\n", __FUNCTION__, qid));
			ASSERT(0);
			return -1;
		}

		/* Convert word addr to byte addr */
		wlc_write_shm(ampdu_tx->wlc, hagg->txfs_wptr * 2, entry);

		WL_AMPDU_HW(("%s: aggfifo %d rptr 0x%x wptr 0x%x entry 0x%04x\n",
			__FUNCTION__, qid, rptr, hagg->txfs_wptr, entry));

		hagg->txfs_wptr = wptr_next;
		wlc_write_shm(ampdu_tx->wlc, hagg->txfs_wptr_addr, hagg->txfs_wptr);
	}
#endif /* WLAMPDU_UCODE */

	WLCNTINCR(ampdu_tx->cnt->enq);
	WLCNTINCR(ampdu_tx->cnt->pending);
	hagg->in_queue ++;
	return 0;
}
#endif /* TXQ_MUX */

#if defined(BCMDBG) || !defined(TXQ_MUX)
/**
 * For ucode agg:
 * Side channel queue is setup with a read and write ptr.
 * - R == W means empty.
 * - (W + 1 % size) == R means full.
 * - Max buffer capacity is size - 1 elements (one element remains unused).
 * For hw agg:
 * simply read ihr reg
 */
static uint
get_aggfifo_space(ampdu_tx_info_t *ampdu_tx, int qid)
{
	uint ret = 0;

#ifdef WLAMPDU_HW
	if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
		uint actual;
		d11regs_t *regs = ampdu_tx->wlc->regs;
		ampdu_tx_config_t *ampdu_tx_cfg = ampdu_tx->config;

		ret = R_REG(ampdu_tx->wlc->osh, &regs->aggfifocnt);
		switch (qid) {
			case 3:
				ret >>= 8;
			case 2:
				ret >>= 8;
			case 1:
				ret >>= 8;
			case 0:
				ret &= 0x7f;
				break;
			default:
				ASSERT(0);
		}
		actual = ret;
		BCM_REFERENCE(actual);

		if (ret >= (uint)(AGGFIFO_CAP - ampdu_tx_cfg->aggfifo_depth))
			ret -= (AGGFIFO_CAP - ampdu_tx_cfg->aggfifo_depth);
		else
			ret = 0;

		/* due to the txstatus can only hold 32 bits in bitmap, limit the size to 32 */
		WL_AMPDU_HW(("%s: fifo %d fifo availcnt %d ret %d\n", __FUNCTION__, qid, actual,
			ret));
	}
#endif	/* WLAMPDU_HW */
#ifdef WLAMPDU_UCODE
	if (AMPDU_UCODE_ENAB(ampdu_tx->wlc->pub)) {
		if (ampdu_tx->hagg[qid].txfs_wptr < ampdu_tx->hagg[qid].txfs_rptr)
			ret = ampdu_tx->hagg[qid].txfs_rptr - ampdu_tx->hagg[qid].txfs_wptr - 1;
		else
			ret = (ampdu_tx->hagg[qid].txfs_wsize - 1) -
			      (ampdu_tx->hagg[qid].txfs_wptr - ampdu_tx->hagg[qid].txfs_rptr);
		ASSERT(ret < ampdu_tx->hagg[qid].txfs_wsize);
		WL_AMPDU_HW(("%s: fifo %d rptr %04x wptr %04x availcnt %d\n", __FUNCTION__,
			qid, ampdu_tx->hagg[qid].txfs_rptr, ampdu_tx->hagg[qid].txfs_wptr, ret));
	}
#endif /* WLAMPDU_UCODE */
	return ret;
}
#endif /* BCMDBG) || !TXQ_MUX */

#endif /* #ifdef AMPDU_NON_AQM */
#endif /* WLAMPDU_MAC */

#ifdef WLAMPDU_PRECEDENCE

static bool BCMFASTPATH
wlc_ampdu_prec_enq(wlc_info_t *wlc, struct pktq *q, void *pkt, int tid)
{
	return wlc_prec_enq_head(wlc, q, pkt, WLAMPDU_PREC_TID2PREC(pkt, tid), FALSE);
}

static void * BCMFASTPATH
wlc_ampdu_pktq_pdeq(struct pktq *pq, int tid)
{
	void *p;

	p = pktq_pdeq(pq, WLC_PRIO_TO_HI_PREC(tid));

	if (p == NULL)
		p = pktq_pdeq(pq, WLC_PRIO_TO_PREC(tid));

	return p;
}

static void
wlc_ampdu_pktq_pflush(osl_t *osh, struct pktq *pq, int tid, bool dir, ifpkt_cb_t fn, int arg)
{
	pktq_pflush(osh, pq, WLC_PRIO_TO_HI_PREC(tid), dir, fn, arg);
	pktq_pflush(osh, pq, WLC_PRIO_TO_PREC(tid), dir, fn, arg);
}

static void *wlc_ampdu_pktq_ppeek(struct pktq *pq, int tid)
{
	void *p = NULL;

	if (!(p = pktq_ppeek(pq, WLC_PRIO_TO_HI_PREC(tid))))
		p = pktq_ppeek(pq, WLC_PRIO_TO_PREC(tid));

	return p;
}

#endif /* WLAMPDU_PRECEDENCE */

#ifdef PROP_TXSTATUS
void wlc_ampdu_flush_pkts(wlc_info_t *wlc, struct scb *scb, uint8 tid)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	wlc_ampdu_pktq_pflush(wlc->osh,  &scb_ampdu->txq, tid, TRUE, NULL, 0);
}

void wlc_ampdu_flush_flowid_pkts(wlc_info_t *wlc, struct scb *scb, uint16 flowid)
{
	uint8 tid;
	scb_ampdu_tid_ini_t *ini;
	scb_ampdu_tx_t *scb_ampdu;
	void *p = NULL;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		ini = scb_ampdu->ini[tid];
		if (!ini)
			continue;
		if (wlc_ampdu_pktq_plen(&scb_ampdu->txq, tid)) {
			p = wlc_ampdu_pktq_ppeek(&scb_ampdu->txq, tid);
			if (p && (flowid == PKTFRAGFLOWRINGID(wlc->osh, p))) {
				wlc_ampdu_flush_pkts(wlc, scb, tid);
				continue;
			}
		}
	}
}

#endif /* PROP_TXSTATUS */

#if defined(BCMDBG) || defined(WLTEST) || defined(WLPKTDLYSTAT) || \
	defined(BCMDBG_AMPDU)
#ifdef WLCNT
void
wlc_ampdu_clear_tx_dump(ampdu_tx_info_t *ampdu_tx)
{
#if defined(BCMDBG) || defined(WLPKTDLYSTAT) || defined(WLATF_CNT)
	struct scb *scb;
	struct scb_iter scbiter;
#endif /* BCMDBG || WLPKTDLYSTAT || WLATF_CNT */
#if defined(BCMDBG) || defined(WLATF_CNT)
	scb_ampdu_tx_t *scb_ampdu;
#ifdef WLATF_CNT
	int n;
#endif // endif
#endif /* BCMDBG || WLATF_CNT */
#ifdef WLPKTDLYSTAT
	uint8 ac;
	scb_delay_stats_t *delay_stats;
#endif // endif

	/* zero the counters */
	bzero(ampdu_tx->cnt, sizeof(wlc_ampdu_tx_cnt_t));
	/* reset the histogram as well */
	if (ampdu_tx->amdbg) {
		bzero(ampdu_tx->amdbg->retry_histogram, sizeof(ampdu_tx->amdbg->retry_histogram));
		bzero(ampdu_tx->amdbg->end_histogram, sizeof(ampdu_tx->amdbg->end_histogram));
		bzero(ampdu_tx->amdbg->mpdu_histogram, sizeof(ampdu_tx->amdbg->mpdu_histogram));
		bzero(ampdu_tx->amdbg->supr_reason, sizeof(ampdu_tx->amdbg->supr_reason));
		bzero(ampdu_tx->amdbg->txmcs, sizeof(ampdu_tx->amdbg->txmcs));
		bzero(ampdu_tx->amdbg->txmcssgi, sizeof(ampdu_tx->amdbg->txmcssgi));
		bzero(ampdu_tx->amdbg->txmcsstbc, sizeof(ampdu_tx->amdbg->txmcsstbc));
#ifdef WLAMPDU_MAC
		bzero(ampdu_tx->amdbg->txmcs_succ, sizeof(ampdu_tx->amdbg->txmcs_succ));
#endif // endif

#ifdef WL11AC
		bzero(ampdu_tx->amdbg->txvht, sizeof(ampdu_tx->amdbg->txvht));
		bzero(ampdu_tx->amdbg->txvhtsgi, sizeof(ampdu_tx->amdbg->txvhtsgi));
		bzero(ampdu_tx->amdbg->txvhtstbc, sizeof(ampdu_tx->amdbg->txvhtstbc));
#ifdef WLAMPDU_MAC
		bzero(ampdu_tx->amdbg->txvht_succ, sizeof(ampdu_tx->amdbg->txvht_succ));
#endif // endif
#endif /* WL11AC */
	}
#ifdef WLOVERTHRUSTER
	ampdu_tx->tcp_ack_info.tcp_ack_total = 0;
	ampdu_tx->tcp_ack_info.tcp_ack_dequeued = 0;
	ampdu_tx->tcp_ack_info.tcp_ack_multi_dequeued = 0;
#endif /* WLOVERTHRUSTER */
#if defined(BCMDBG) || defined(WLPKTDLYSTAT) || defined(WLATF_CNT)
	FOREACHSCB(ampdu_tx->wlc->scbstate, &scbiter, scb) {
#ifdef WLPKTDLYSTAT
		for (ac = 0; ac < AC_COUNT; ac++) {
			if (!scb->delay_stats[ac])
				continue;
			delay_stats = scb->delay_stats[ac];
			/* reset the per-SCB delay statistics */
			bzero(delay_stats, sizeof(*delay_stats));
		}
#endif // endif
#if defined(BCMDBG) || defined(WLATF_CNT)
		if (SCB_AMPDU(scb)) {
			/* reset the per-SCB statistics */
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
#ifdef BCMDBG
			bzero(&scb_ampdu->cnt, sizeof(scb_ampdu_cnt_tx_t));
#endif // endif
#ifdef WLATF_CNT
			for (n = 0; n < AMPDU_MAX_SCB_TID; n++) {
				if (scb_ampdu && scb_ampdu->ini[n])
					AMPDU_ATF_CLRCNT(scb_ampdu->ini[n]);
			}
#endif // endif
		}
#endif /* BCMDBG || WLATF_CNT */
	}
#endif /* BCMDBG || WLPKTDLYSTAT */

#ifdef WLAMPDU_MAC
	if (ampdu_tx->wlc->clk) {
#ifdef WLAMPDU_UCODE
		if (ampdu_tx->amdbg) {
			bzero(ampdu_tx->amdbg->schan_depth_histo,
				sizeof(ampdu_tx->amdbg->schan_depth_histo));
		}
#endif // endif
	/* zero out shmem counters */
		if (AMPDU_MAC_ENAB(ampdu_tx->wlc->pub) && !AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			wlc_write_shm(ampdu_tx->wlc, MACSTAT_ADDR(MCSTOFF_TXMPDU), 0);
			wlc_write_shm(ampdu_tx->wlc, MACSTAT_ADDR(MCSTOFF_TXAMPDU), 0);
		}

#if defined(WLAMPDU_HW) || defined(WLAMPDU_AQM)
		if (AMPDU_HW_ENAB(ampdu_tx->wlc->pub)) {
			if (D11REV_IS(ampdu_tx->wlc->pub->corerev, 26) ||
				D11REV_IS(ampdu_tx->wlc->pub->corerev, 29) ||
				D11REV_IS(ampdu_tx->wlc->pub->corerev, 37)) {
				int i;
				uint16 stat_addr = 2 * wlc_read_shm(ampdu_tx->wlc,
					M_HWAGG_STATS_PTR);
				for (i = 0; i < (C_MBURST_WOFF + C_MBURST_NBINS); i++)
					wlc_write_shm(ampdu_tx->wlc, stat_addr + i * 2, 0);
			}
		} else if (AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
			int i;
			uint16 stat_addr = 2 * wlc_read_shm(ampdu_tx->wlc, M_AMP_STATS_PTR);
			for (i = 0; i < C_AMP_STATS_SIZE; i++)
				wlc_write_shm(ampdu_tx->wlc, stat_addr + i * 2, 0);
		}
#endif /* WLAMPDU_HW */
	}
#endif /* WLAMPDU_MAC */
} /* wlc_ampdu_clear_tx_dump */

#endif /* WLCNT */
#endif /* defined(BCMDBG) || defined(WLTEST) */

void
wlc_ampdu_tx_send_delba(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid,
	uint16 initiator, uint16 reason)
{
	ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, FALSE);

	WL_AMPDU(("wl%d: %s: tid %d initiator %d reason %d\n",
		ampdu_tx->wlc->pub->unit, __FUNCTION__, tid, initiator, reason));

	wlc_send_delba(ampdu_tx->wlc, scb, tid, initiator, reason);

	WLCNTINCR(ampdu_tx->cnt->txdelba);
}

void
wlc_ampdu_tx_recv_delba(ampdu_tx_info_t *ampdu_tx, struct scb *scb, uint8 tid, uint8 category,
	uint16 initiator, uint16 reason)
{
	scb_ampdu_tx_t *scb_ampdu_tx;
	scb_ampdu_tid_ini_t *ini;

	ASSERT(scb);

	scb_ampdu_tx = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
	BCM_REFERENCE(scb_ampdu_tx);
	ASSERT(scb_ampdu_tx);

	if (category & DOT11_ACTION_CAT_ERR_MASK) {
		WL_ERROR(("wl%d: %s: unexp error action frame\n",
			ampdu_tx->wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(ampdu_tx->cnt->rxunexp);
		return;
	}

	/* Set state to off, and push the packets in ampdu_pktq to next */
	ini = scb_ampdu_tx->ini[tid];
	if (ini) {
		/* Push the packets in ampdu_pktq to next to
		 * prevent packet lost
		 */
		ampdu_ba_state_off(scb_ampdu_tx, ini);
	}

	ampdu_cleanup_tid_ini(ampdu_tx, scb, tid, TRUE);

	WLCNTINCR(ampdu_tx->cnt->rxdelba);
	AMPDUSCBCNTINCR(scb_ampdu_tx->cnt.rxdelba);

	WL_AMPDU(("wl%d: %s: AMPDU OFF: tid %d initiator %d reason %d\n",
		ampdu_tx->wlc->pub->unit, __FUNCTION__, tid, initiator, reason));
}

#ifdef WLAMPDU_MAC
void wlc_ampdu_upd_pm(wlc_info_t *wlc, uint8 PM_mode)
{
	if (PM_mode == 1)
		wlc->ampdu_tx->aqm_max_release[PRIO_8021D_BE] = AMPDU_AQM_RELEASE_DEFAULT;
	else
		wlc->ampdu_tx->aqm_max_release[PRIO_8021D_BE] = AMPDU_AQM_RELEASE_MAX;
}
#endif /* WLAMPDU_MAC */

/**
 * This function moves the window for the pkt freed during DMA TX reclaim
 * for which status is not received till flush for channel switch
 */
static void
wlc_ampdu_ini_adjust(ampdu_tx_info_t *ampdu_tx, scb_ampdu_tid_ini_t *ini, void *pkt)
{
	/* This func is no longer called as it stated in the comment above
	 * the function hence unnecessary to keep...
	 */
	uint16 seq = WLPKTTAG(pkt)->seq;

#ifdef PROP_TXSTATUS
	seq = WL_SEQ_GET_NUM(seq);
#endif /* PROP_TXSTATUS */

#ifdef PROP_TXSTATUS
	if (WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wl))) {
#ifdef PROP_TXSTATUS_SUPPR_WINDOW
		/* adjust the pkts in transit during flush */
		ini->suppr_window++;
#endif // endif
		return;
	}
#endif /* PROP_TXSTATUS */

	if (!AMPDU_AQM_ENAB(ampdu_tx->wlc->pub)) {
		uint16 index = TX_SEQ_TO_INDEX(seq);
		if (!isset(ini->ackpending, index)) {
			return;
		}
		setbit(ini->barpending, index);
	}
	/* XXX: Need to validate this condition. It's inherited from the following functions
	 * wlc_ampdu_dotxstatus_regmpdu_aqm(),  wlc_ampdu_dotxstatus_aqm_complete()
	 * In both these funcs 'barpendig_seq' is updated and sendbar is done only if the
	 * condition is satisfied.
	 */
	else if (MODSUB_POW2(ini->max_seq, seq, SEQNUM_MAX) < ini->ba_wsize) {
		uint16 bar_seq = MODINC_POW2(seq, SEQNUM_MAX);
		/* check if there is another bar with advence seq no */
		if (!ini->bar_ackpending ||
		    IS_SEQ_ADVANCED(bar_seq, ini->barpending_seq)) {
			WL_AMPDU(("%s: set barpending_seq ini->bar_ackpending:%d bar_seq:%d "
			"ini->barpending_seq:%d seq:%d ini->max_seq:%d ini->ba_wsize:%d\n",
			__FUNCTION__, ini->bar_ackpending, bar_seq, ini->barpending_seq, seq,
			          ini->max_seq, ini->ba_wsize));
			ini->barpending_seq = bar_seq;
			/* Also update the exp seq number if required */
			ini->tx_exp_seq = ini->barpending_seq;
		}
		else  {
			WL_AMPDU(("%s:cannot set barpending_seq:"
			          "ini->bar_ackpending:%d bar_seq:%d"
			          "ini->barpending_seq:%d", __FUNCTION__,
			          ini->bar_ackpending, bar_seq, ini->barpending_seq));
		}
	}
} /* wlc_ampdu_ini_adjust */

#ifdef PROP_TXSTATUS
void wlc_ampdu_flush_ampdu_q(ampdu_tx_info_t *ampdu, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	struct scb *scb = NULL;
	scb_ampdu_tx_t *scb_ampdu;

	FOREACHSCB(ampdu->wlc->scbstate, &scbiter, scb) {
		if (scb->bsscfg == cfg) {
			if (!SCB_AMPDU(scb))
				continue;
			scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);

			wlc_wlfc_flush_queue(ampdu->wlc, &scb_ampdu->txq);
			wlc_check_ampdu_fc(ampdu, scb);
		}
	}

}

void wlc_ampdu_send_bar_cfg(ampdu_tx_info_t * ampdu, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint32 pktprio;
	if (!scb || !SCB_AMPDU(scb))
		return;

	/* when reuse seq, no need to send bar */
	if (WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu->wlc->wl))) {
		return;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);

	for (pktprio = 0; pktprio <  AMPDU_MAX_SCB_TID; pktprio++) {
		ini = scb_ampdu->ini[pktprio];
		if (!ini) {
			continue;
		}
		/* We have no bitmap for AQM so use barpending_seq */
		if ((!AMPDU_AQM_ENAB(ampdu->wlc->pub)) || (ini->barpending_seq != SEQNUM_MAX))
			wlc_ampdu_send_bar(ampdu, ini, FALSE);
	}
}

#endif /* PROP_TXSTATUS */

/** returns tx queue to use for a caller supplied scb (= one remote party) */
struct pktq* wlc_ampdu_txq(ampdu_tx_info_t *ampdu, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu;
	if (!scb || !SCB_AMPDU(scb))
		return NULL;
	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);
	return &scb_ampdu->txq;
}

/** PROP_TXSTATUS, flow control related */
void wlc_check_ampdu_fc(ampdu_tx_info_t *ampdu, struct scb *scb)
{
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	int i;

	if (!scb || !SCB_AMPDU(scb)) {
		return;
	}

	scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu, scb);
	if (!scb_ampdu) {
		return;
	}

	for (i = 0; i < AMPDU_MAX_SCB_TID; i++) {
		ini = scb_ampdu->ini[i];
		if (ini) {
			wlc_ampdu_txflowcontrol(ampdu->wlc, scb_ampdu, ini);
		}
	}
}

#ifdef BCMDBG
struct wlc_ampdu_txq_prof_entry {
	uint32 timestamp;
	struct scb *scb;

	uint32 ampdu_q_len;
	uint32 rem_window;
	uint32 tx_in_transit;
	uint32 wlc_txq_len;	/* qlen of all precedence */
	uint32 wlc_pktq_qlen;	/* qlen of a single precedence */
	uint32 prec_map;
	uint32 txpkt_pend; /* TXPKTPEND */
	uint32 dma_desc_avail;
	uint32 dma_desc_pending; /* num of descriptor that has not been processed */
	const char * func;
	uint32 line;
	uint32 tid;
};

#define AMPDU_TXQ_HIS_MAX_ENTRY (1 << 7)
#define AMPDU_TXQ_HIS_MASK (AMPDU_TXQ_HIS_MAX_ENTRY - 1)

static  struct wlc_ampdu_txq_prof_entry _txq_prof[AMPDU_TXQ_HIS_MAX_ENTRY];
static  uint32 _txq_prof_index;
static  struct scb *_txq_prof_last_scb;
static  int 	_txq_prof_cnt;

void wlc_ampdu_txq_prof_enab(void)
{
	_txq_prof_enab = 1;
}

/** tx queue profiling */
void wlc_ampdu_txq_prof_add_entry(wlc_info_t *wlc, struct scb *scb, const char * func, uint32 line)
{
	struct wlc_ampdu_txq_prof_entry *p_entry;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	wlc_txq_info_t *qi;
	struct pktq *q;

	if (!_txq_prof_enab)
		return;

	if (!scb)
		scb = _txq_prof_last_scb;

	if (!scb)
		return;

	scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
	if (!scb_ampdu) {
		return;
	}

	if (!(ini = scb_ampdu->ini[PRIO_8021D_BE])) {
		return;
	}

	/* only gather statistics for BE */
	if (ini->tid !=  PRIO_8021D_BE) {
		return;
	}

	if (prio2fifo[ini->tid] != TX_AC_BE_FIFO)
		return;

	_txq_prof_cnt++;
	if (_txq_prof_cnt == AMPDU_TXQ_HIS_MAX_ENTRY) {
		_txq_prof_enab = 0;
		_txq_prof_cnt = 0;
	}
	_txq_prof_index = (_txq_prof_index + 1) & AMPDU_TXQ_HIS_MASK;
	p_entry = &_txq_prof[_txq_prof_index];

	p_entry->timestamp = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
	p_entry->scb = scb;
	p_entry->func = func;
	p_entry->line = line;
	p_entry->ampdu_q_len =
		wlc_ampdu_pktq_plen((&scb_ampdu->txq), (ini->tid));
	p_entry->rem_window = ini->rem_window;
	p_entry->tx_in_transit = ini->tx_in_transit;
	qi = SCB_WLCIFP(scb)->qi;
	q = WLC_GET_TXQ(qi);
	p_entry->wlc_txq_len = pktq_len(q);
	p_entry->wlc_pktq_qlen = pktq_plen(q, WLC_PRIO_TO_PREC(ini->tid));
#if defined(NEW_TXQ)
	p_entry->prec_map = txq_stopped_map(qi->low_txq);
#else
	p_entry->prec_map = wlc->tx_prec_map;
#endif // endif

	p_entry->txpkt_pend = TXPKTPENDGET(wlc, (prio2fifo[ini->tid])),
	p_entry->dma_desc_avail = TXAVAIL(wlc, (prio2fifo[ini->tid]));
	p_entry->dma_desc_pending = dma_txpending(WLC_HW_DI(wlc, (prio2fifo[ini->tid])));

	_txq_prof_last_scb = scb;

	p_entry->tid = ini->tid;
} /* wlc_ampdu_txq_prof_add_entry */

void wlc_ampdu_txq_prof_print_histogram(int entries)
{
	int i, index;
	struct wlc_ampdu_txq_prof_entry *p_entry;

	printf("TXQ HISTOGRAM\n");

	index = _txq_prof_index;
	if (entries == -1)
		entries = AMPDU_TXQ_HIS_MASK;
	for (i = 0; i < entries; i++) {
		p_entry = &_txq_prof[index];
		printf("ts: %u	@ %s:%d\n", p_entry->timestamp, p_entry->func, p_entry->line);
		printf("ampdu_q  rem_win   in_trans    wlc_q prec_map pkt_pend "
			"Desc_avail Desc_pend\n");
		printf("%7d  %7d  %7d  %7d %7x %7d %7d   %7d\n",
			p_entry->ampdu_q_len,
			p_entry->rem_window,
			p_entry->tx_in_transit,
			p_entry->wlc_txq_len,
			p_entry->prec_map,
			p_entry->txpkt_pend,
			p_entry->dma_desc_avail,
			p_entry->dma_desc_pending);
		index = (index - 1) & AMPDU_TXQ_HIS_MASK;
	}
}

#endif /* BCMDBG */

#ifdef WL_CS_RESTRICT_RELEASE
static void
wlc_ampdu_txeval_tid(wlc_info_t *wlc, uint8 tid)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_AMPDU(scb)) {
			scb_ampdu_tx_t *scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
			if (scb_ampdu) {
				scb_ampdu_tid_ini_t *ini = scb_ampdu->ini[tid];
				if (ini) {
					wlc_ampdu_txeval(wlc->ampdu_tx,
						scb_ampdu, ini,	FALSE);
				}
			}
		}
	}
}

void
wlc_ampdu_txeval_all(wlc_info_t *wlc)
{
	uint8 tid;

	for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
		bool txeval_tid = TRUE;
#ifdef WLTAF
		if (wlc_taf_reset_scheduling(wlc->taf_handle, tid)) {
			wlc_taf_schedule(wlc->taf_handle, tid, NULL, FALSE);
			txeval_tid = FALSE;
		}
#endif /* WLTAF */
		if (txeval_tid) {
			wlc_ampdu_txeval_tid(wlc, tid);
		}
	}
}
#endif /* WL_CS_RESTRICT_RELEASE */

void
wlc_ampdu_agg_state_update_tx_all(wlc_info_t *wlc, bool aggr)
{
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_BSS(wlc, idx, cfg) {
		wlc_ampdu_tx_set_bsscfg_aggr_override(wlc->ampdu_tx, cfg, aggr);
	}
}

/* After Channel switch send a bar to reset and resume the tx
*/
void
wlc_ampdu_cleanup_cs(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb *scb;
	struct scb_iter scbiter;
	ampdu_tx_info_t *ampdu_tx = wlc->ampdu_tx;
	scb_ampdu_tx_t *scb_ampdu;
	scb_ampdu_tid_ini_t *ini;
	uint8 tid;

	ASSERT(ampdu_tx);

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		scb_ampdu = SCB_AMPDU_TX_CUBBY(wlc->ampdu_tx, scb);
		ASSERT(scb_ampdu);
		for (tid = 0; tid < AMPDU_MAX_SCB_TID; tid++) {
			ini = scb_ampdu->ini[tid];
			if (!ini)
				continue;

			if (ini->ba_state == AMPDU_TID_STATE_BA_ON) {
				wlc_ampdu_txeval(ampdu_tx, scb_ampdu, ini, TRUE);
				if (ini->retry_bar) {
					wlc_ampdu_send_bar(ampdu_tx, ini, FALSE);
				}
			}
		}
	}
}
#if defined(TXQ_MUX)

#if defined(WLAMPDU_MAC)

/*
 * Following block of routines for AQM AMPDU aggregation (d11 core rev >= 40)
 */

/**
 * Tx MUX Output function for an SCB Queue that has AQM A-MPDU aggregation enabled.
 *
 * This function is configured as the Tx MUX outout function when A-MPDU is desired for
 * an A-MPDU capable SCB.
 *
 * The prototype matches the @ref mux_output_fn_t function type.
 *
 * @param ctx           a pointer to the scb_ampdu_tx_t structure for the configured SCB
 * @param ac            Access Category for traffic being requested
 * @param request_time  the total estimated TX time the caller is requesting from the SCB
 * @param output_q      pointer to a packet queue to use to store the output packets
 * @return              The estimated TX time of the packets returned in the output_q. The time
 *                      returned may be greater than the 'request_time'
 */
static uint BCMFASTPATH
wlc_ampdu_output_aqm(void *ctx, uint ac, uint request_time, struct spktq *output_q)
{
	scb_ampdu_tx_t *scb_ampdu = ctx;
	scb_ampdu_tid_ini_t *ini = NULL;
	struct scb *scb;
	ampdumac_info_t *hagg;
	ampdu_tx_info_t *ampdu_tx;
	wlc_bsscfg_t *cfg = NULL;
	wlc_info_t *wlc;
	osl_t* osh;
	void* p;
	struct pktq *q;
	ampdu_aqm_timecalc_t *tc;
	struct dot11_header* h;
	wlc_pkttag_t* pkttag;
	d11actxh_t* vhtHdr;
	uint8* txd;
	uint d11_frame_size;
	uint d11_offset;
	int8 check_epoch = TRUE;
	int err = 0;
	int count = 0;
	bool suppressed_pkt = FALSE;
	uint8 tid, prev_tid = 0;
	uint16 seq;
	uint16 prec_map;
	ratespec_t rspec;
	uint supplied_time = 0;
	uint current_pktlen = 0;
	uint current_pkttime = 0;
	uint pkttime;
	int prec;
	uint16 suppr_count = 0;
	bool reset_suppr_count = FALSE;

	BCM_REFERENCE(suppr_count);
	BCM_REFERENCE(reset_suppr_count);

	ampdu_tx = scb_ampdu->ampdu_tx;
	scb = scb_ampdu->scb;

	cfg = SCB_BSSCFG(scb);

	ASSERT(ac < AC_COUNT);
	hagg = &ampdu_tx->hagg[ac];

	wlc = ampdu_tx->wlc;
	osh = wlc->osh;
	q = wlc_scbq_txq(wlc->tx_scbq, scb);

	/* XXX MUX: Need to change this to be a stop reason for the release instead of an
	 * err.  This is the logic to limit pkts when unsure of tx good ...
	 */
	if (!wlc_scb_restrict_can_txq(wlc, scb)) {
		ASSERT(0);
		return 0;
	}

	/* AMPDUs are not allowed for a station in PS. This output function should not be configured
	 * for an SCB that is in power save mode.
	 */
	ASSERT(!SCB_PS(scb));

	/* XXX MUX: consider the scb ratesel caching. Use the txc rate invalidate to inval the scb
	 * ratesel cache.  We want a lightweight query on the current rate, so caching would be
	 * good.
	 */
	rspec = wlc_scb_ratesel_get_primary(ampdu_tx->wlc, scb, NULL);

	/* use a prec_map that matches the AC fifo parameter */
	prec_map = wlc->fifo2prec_map[ac];

	/* loop may exit via break in addition to these tests */
	while (supplied_time < request_time && (p = pktq_mdeq(q, prec_map, &prec))) {
		bool regmpdu_pkt = FALSE;
		bool reque_pkt = FALSE;
		bool seq_hole = FALSE;

		pkttag = WLPKTTAG(p);

		/* Packets for two <tid, ini> may be in the queue for one AC, the loop needs to be
		 * able to transition from one tid to another
		 * This check will also trigger if 'ini' has not yet been assigned.
		 */
		tid = (uint8)PKTPRIO(p);

		/* All packets should have a prio that matches the precidence queue they were on.
		 * Otherwise, the TID draining code in wlc_ampdu_output_nonba() will not drain all
		 * the of the matching priority.
		 */
		ASSERT(prec == WLC_PRIO_TO_PREC(tid) || prec == WLC_PRIO_TO_HI_PREC(tid));

		if (ini == NULL || tid != prev_tid) {
			ASSERT(tid < AMPDU_MAX_SCB_TID);

			/* if we are changing TID, we need to update the epoch */
			check_epoch = TRUE;

			if (count) {
				AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
				if (ini) {
					ini->tx_in_transit += (uint16)count;
#ifdef WLATF
					ampdu_tx->tx_intransit += (uint16)count;
#endif /* WLATF */
				}
				count = 0;
			}

			prev_tid = tid;
			ini = scb_ampdu->ini[tid];

			/* check if an initiator exists for this TID */
			if (ini == NULL) {
				/* initialize initiator on first packet; sends addba req */
				ini = wlc_ampdu_init_tid_ini(ampdu_tx, scb_ampdu, tid, FALSE);
				if (ini == NULL) {
					WL_AMPDU(("wl%d: %s: ini creation failed for "MACF" "
					          "tid %u\n", wlc->pub->unit, __FUNCTION__,
					          ETHER_TO_MACF(scb->ea), tid));
				}
			}

			/* if there is no initiator for this TID, or the state is not ON,
			 * then all the pkts in this TID need to be sent on outside a BA
			 * aggrement
			 */
			if ((ini == NULL) ||
			    (ini->ba_state != AMPDU_TID_STATE_BA_ON)) {
				/* put the current packet back at the head of the queue to be
				 * processed by wlc_ampdu_output_nonba().
				 */
				pktq_penq_head(q, prec, p);

				/* drain all pkts at the current TID for non-aggregated output */
				supplied_time +=
				        wlc_ampdu_output_nonba(q, scb, wlc, rspec, ac, tid,
				                               (request_time - supplied_time),
				                               output_q);

				/* The pktq queue should now be empty, or have a pkt with a
				 * different TID at the head, or the supplied_time exhausted
				 * the requested_time.
				 * Jump back to the top of the loop.
				 */
				ini = NULL;
				continue;
			}
		}

		tc = &ini->timecalc[ac];
		if (tc->rspec != rspec) {
			wlc_ampdu_init_aqm_timecalc(scb_ampdu, ini, rspec, ac, tc);
		}

		/* XXX MUX: Sequence number assignment code here is modeled from the
		 * txmod path, wlc_ampud_release().
		 */

#ifdef PROP_TXSTATUS
		/* Use the seq number the host driver provides if present. The host may be resending
		 * a previously suppressed frame that was already assigned a sequence nubmer.
		 */
		suppressed_pkt = (WL_SEQ_GET_FROMDRV(pkttag->seq) != 0);
		if (suppressed_pkt) {
			seq = WL_SEQ_GET_NUM(pkttag->seq);
#ifdef PROP_TXSTATUS_SUPPR_WINDOW
			suppr_count++;
#endif // endif
		} else
#endif /* PROP_TXSTATUS */
		/* Use next seq number for this scb and tid */
		{
			/* assign seqnum and save in pkttag */
			seq = SCB_SEQNUM(ini->scb, ini->tid) & (SEQNUM_MAX - 1);
			SCB_SEQNUM(ini->scb, ini->tid)++;
			pkttag->seq = seq;
			/* record the highest seq number in the BA session */
			ini->max_seq = seq;
			reset_suppr_count = TRUE;

			/* XXX MUX: Need to be able to use same seq if pkt is dropped instead of
			 * assigning a new seq number and creating a seq number hole.
			 */
		}

		/* separate packet preparation and time calculation for
		 * MPDU (802.11 formatted packet with txparams), and
		 * MSDU (Ethernet or 802.3 stack packet)
		 */

		if ((pkttag->flags & WLF_MPDU) == 0) {
			uint fifo;
			/*
			 * MSDU packet prep
			 */

			/* mark this as an MPDU in a BA */
			pkttag->flags |= WLF_AMPDU_MPDU;
#ifdef PROP_TXSTATUS
			/* Set FROMFW to note that the seq number is assigned.
			 * FROMFW is used in PROP_TXSTATUS suppression
			 */
			if (WLFC_GET_REUSESEQ(wlfc_query_mode(ampdu_tx->wlc->wl))) {
				WL_SEQ_SET_FROMFW(pkttag->seq, 1);
				if (reset_suppr_count || (ini->suppr_window <= suppr_count)) {
					ini->suppr_window = 0;
				} else {
					ini->suppr_window -= suppr_count;
				}
			}
#endif /* PROP_TXSTATUS */

			/* use the streamlined version of prep_sdu if possible,
			 * otherwise fall back to full version of prep_sdu.
			 */
			err = wlc_prep_sdu_fast(wlc, cfg, scb, p, &fifo);
			if (err == BCME_UNSUPPORTED) {
				uint npkts = 1;

				err = wlc_prep_sdu(wlc, scb, &p, (int*)&npkts, &fifo);

				/* fragmentation is not for A-MPDU, A-MSDU, or any MSDUs under an
				 * HT-immediate BA aggrement (IEEE802.11-2012 sec 9.2.7), so npkts
				 * (the number of fragments for the input frame) should always be 1
				 * if there was no error reported.
				 */
				ASSERT(npkts == 1 || err);
			}

			/* Determine if the packet can be sent in an AMPDU.
			 *
			 * Since the packet is an SDU data packet, we know it can be sent in an
			 * A-MPDU as long as the phy frame type is HT or VHT, and the encryption is
			 * compatible with A-MPDU.
			 *
			 * The WLF_MIMO flag is set in wlc_d11ac_hdrs()/wlc_d11ht_hdrs() if the
			 * frame is HT or VHT, and if the key is compatible (or no key at all).
			 */
			regmpdu_pkt = !(pkttag->flags & WLF_MIMO);

			if (!err) {
				/* the fifo that wlc_prep_sdu() calculates should match the AC since
				 * we are pulling packets from the matching set of precidences.
				 * ASSERT if this assumption breaks since we are suppling these
				 * packets to the AC defined fifo.
				 */
				ASSERT(fifo == ac);

				/* Fetch the TxD (DMA hw hdr), vhtHdr (rev 40+ ucode hdr),
				 * and 'h' (d11 frame header).
				 */
				wlc_txprep_pkt_get_hdrs(wlc, p, &txd, &vhtHdr, &h);
				d11_offset = (uint32)((int8*)h - (int8*)txd);
				d11_frame_size = pkttotlen(osh, p) - d11_offset;

				if (regmpdu_pkt) {
					/* use the non-ampdu time calc since
					 * this will not be in an ampdu
					 */

					/* calculate and store the estimated pkt tx time */
					pkttime = wlc_tx_mpdu_time(wlc, scb, rspec, ac,
					                           d11_frame_size);
				} else if (current_pktlen == d11_frame_size) {
					/* optimization: skip the txtime calculation if the MPDU
					 * pkt len is the same as the last time through the loop
					 */

					/* estimated pkt tx time should be the same as last time */
					pkttime = current_pkttime;
				} else {
					/* Calculate an estimated airtime for this part of
					 * an A-MPDU
					 */

					/* calculate the estimated pkt tx time */
					pkttime = wlc_ampdu_aqm_timecalc(tc, d11_frame_size);
					current_pktlen = d11_frame_size;
					current_pkttime = pkttime;
				}

				/* store the estimated pkt tx time */
				pkttag->pktinfo.atf.pkt_time = (uint16)pkttime;

			} else {
				if (err == BCME_ERROR) {
					/* BCME_ERROR indicates a tossed packet */

					/* error in the packet; reject it */
					WL_AMPDU_ERR(("wl%d: %s: prep_sdu rejected pkt\n",
					              wlc->pub->unit, __FUNCTION__));
					WLCNTINCR(ampdu_tx->cnt->sdurejected);
					AMPDUSCBCNTINCR(scb_ampdu->cnt.sdurejected);
				} else {
					/* should be no other errors */
					ASSERT(err == BCME_OK);
					/* XXX, Well BUSY can still happen for frags, but should not
					 * run into since no fragmentation for frames under an
					 * HT-immediate BA aggrement,
					 * BUSY for block_datafifo, but we check before this loop
					 * BUSY for SCB_ISMULTI(), but until we run AP, no SCB
					 * is multi AND Multicast cannot use AMPDU
					 */
					PKTFREE(osh, p, TRUE);
				}

				/* reclaim the unused sequence number */
#ifdef PROP_TXSTATUS
				/* if the sequence number did not come from the host driver */
				if (!suppressed_pkt)
#endif // endif
				{
					SCB_SEQNUM(ini->scb, ini->tid)--;
					ini->max_seq = (SCB_SEQNUM(ini->scb, ini->tid) &
					                (SEQNUM_MAX - 1));
				}

				/* XXX MUX: Need to change error flow. Maybe do a 'continue' here?
				 * XXX MUX: What does PROP_TXSTATUS need to
				 * do for an err failed packet?
				 */
				continue;
			}
#ifdef PROP_TXSTATUS
			/*
			 * Now the dongle owns the packet
			 * Toggle Prop TXSTATUS ownership bits
			 * Pull it out of the pkttag again to ensure preceefding routines
			 * did not change pkttag.
			 */
			if (WL_SEQ_GET_FROMDRV(pkttag->seq)) {
				WL_SEQ_SET_FROMDRV(pkttag->seq, 0);
			}
#endif /* PROP_TXSTATUS */
		} else {
			uint fifo;
			/*
			 * MPDU packet prep
			 */

			/* if there are pdu txparams or if the d11 header was already preped, we
			 * cannot make assumptions about this pkt being aggregatable with previous
			 * pkts in this loop.
			 * Flag to do full epoch checking.
			 */
			check_epoch = TRUE;

			/* WLF_TXHDR flag indicates the MPDU is being retransmitted
			 * after having d11 header prep previously
			 */
			reque_pkt = (pkttag->flags & WLF_TXHDR);

			if (!reque_pkt) {
				ratespec_t tmp_rspec;

				/* If this packet is not requeued, fetch the rspec saved in tx_prams
				 * at the head of the pkt before tx_params are removed by
				 * wlc_prep_pdu()
				 */
				tmp_rspec = wlc_pdu_txparams_rspec(osh, p);

				/* Note: currently no errors from wlc_prep_pdu() */
				(void)wlc_prep_pdu(wlc, scb, p, &fifo);
				ASSERT(fifo == ac);

				/* Fetch the TxD (DMA hw hdr), vhtHdr (rev 40+ ucode hdr),
				 * and 'h' (d11 frame header)
				 */
				wlc_txprep_pkt_get_hdrs(wlc, p, &txd, &vhtHdr, &h);
				d11_offset = (uint32)((int8*)h - (int8*)txd);
				d11_frame_size = pkttotlen(osh, p) - d11_offset;

				/* calculate and store the estimated pkt tx time */
				pkttime = wlc_tx_mpdu_time(wlc, scb, tmp_rspec, ac, d11_frame_size);
				WLPKTTIME(p) = (uint16)pkttime;

				/* Determine if the packet can be sent in an AMPDU.
				 * Since the packet is an MPDU without re-queuing, the packet should
				 * be a management frame, which are not aggregatable.
				 * Assert that the frame type is not DATA, and note that this pkt
				 * should be sent as a regular mpdu.
				 */
				regmpdu_pkt = TRUE;
				ASSERT((ltoh16(h->fc) & FC_KIND_MASK) != FC_QOS_DATA);

			} else {
				/* Note: currently no errors from wlc_prep_pdu() */
				(void)wlc_prep_pdu(wlc, scb, p, &fifo);
				ASSERT(fifo == ac);

				/* Fetch the TxD (DMA hw hdr), vhtHdr (rev 40+ ucode hdr),
				 * and 'h' (d11 frame header)
				 */
				wlc_txprep_pkt_get_hdrs(wlc, p, &txd, &vhtHdr, &h);
				d11_offset = (uint32)((int8*)h - (int8*)txd);
				d11_frame_size = pkttotlen(osh, p) - d11_offset;

				/* Determine if the packet can be sent in an AMPDU.
				 * Since the packet is a re-queued MPDU, the packet could
				 * be a DATA frame, which are aggregatable.
				 * Check if the AMPDU flag was set, and if WLF_MIMO was set just
				 * as in the MSDU prep code above.
				 */

				if ((pkttag->flags & (WLF_MIMO | WLF_AMPDU_MPDU)) ==
				    (WLF_MIMO | WLF_AMPDU_MPDU)) {
					/* The packet had both flags set, so it is AMPDU ready */
					regmpdu_pkt = FALSE;
				} else {
					/* The packet did not have both flags set,
					 * so it is a regular non-ampdu packet
					 */
					regmpdu_pkt = TRUE;
				}

				/* If the pkt already had d11 header prep,
				 * the airtime has already been calculated
				 */
				pkttime = WLPKTTIME(p);
			}
		}

		WL_AMPDU_TX(("wl%d: %s: prep_xdu success; seq 0x%x tid %d\n",
			wlc->pub->unit, __FUNCTION__, seq, tid));

		/* MUX Implementation note:
		 * The non-MUX code for _wlc_sendampdu_aqm() had logic for regmpdu_pkt like this:
		 * regmpdu_pkt =
		 *      !(pkttag->flags & WLF_MIMO) ||
		 *      (ini->ba_state == AMPDU_TID_STATE_BA_PENDING_OFF) ||
		 *      (ini->ba_state == AMPDU_TID_STATE_BA_OFF) ||
		 *      SCB_PS(scb);
		 *
		 * In this routine, the 'ini' (initiator state, or BA Originator) validation above
		 * makes sure that the ba_state is always ON, so the two checks on ba_state are not
		 * needed. The non-MUX code marks packets with the pkt flag WLF_AMPDU_MPDU in the
		 * TxModule before putting the pakcet in the serialized TxQ. By the time
		 * _wlc_sendampdu_aqm() runs and dequeues packets from the TxQ the BA state may have
		 * changed. In the MUX code there is no AMPDU TxModule so this routine checks the
		 * state of the 'ini' as it starts drianing packets from the SCBQ at a particular
		 * TID (priority).
		 *
		 * For MUX code, this routine is only handling packets for one SCB. Stations in PS
		 * mode are not allowed to send A-MPDUs. So when a station transitions to PS, this
		 * routine will be un-configured as the SCBQ output function. As a result, there is
		 * no need for an SCB_PS() check, and instead this routine asserts !SCB_PS() near
		 * the top.
		 *
		 * In non-MUX code, _wlc_sendampdu_aqm() drains from the driver's serialized TxQ
		 * packets for any SCB as long as the packets are marked as WMF_AMPDU_MPDU. So the
		 * non-MUX code needed to check this condition.
		 *
		 * This routine handles any packets queued for an SCB including Bufferable MMPDUs
		 * (PS deliverable management frames). Because of this checks for Data vs Management
		 * frame types need to be made. So the 'regmpud_pkt' logic is moved up in the cases
		 * for MSDU and MPDU packet prep just above this comment.
		 */

		/* frames should all be prepared with a header by now */
		ASSERT(pkttag->flags & WLF_MPDU);

		/* XXX MUX: non-MUX code used to assign seq here from h->seq, but should already be
		 * assigned. Just change to ASSERT to make sure there is not logic that changes seq.
		 */
		ASSERT(seq == ltoh16(h->seq) >> SEQNUM_SHIFT);

		/* XXX MUX: non-MUX code used to have a condition here handling a NULLADDR a1
		 * with the following comment:
		 * >  during roam we get pkts with null a1. kill it here else
		 * >  does not find scb on dotxstatus and things gets out of sync
		 * replaced with ASSERT and added asserts in wlc_80211hdr() for non-null addrs
		 */
		ASSERT(!ETHER_ISNULLADDR(&h->a1));

		if (pkttag->flags & WLF_FIFOPKT) {
			wlc_ampdu_chanspec_update(ampdu_tx, p);
			pkttag->flags &= ~WLF_FIFOPKT;
		} else if (!ampdu_is_exp_seq(ampdu_tx, ini, seq, suppressed_pkt)) {
			/* XXX MUX: since seq numbers are assigned here,
			 * there should be no error of this sort
			 */
			ASSERT(0);
			PKTFREE(osh, p, TRUE);
			WLCNTINCR(ampdu_tx->cnt->txdrop);
			continue;
		}

		seq_hole = ampdu_detect_seq_hole(ampdu_tx, seq, ini);

		/*
		 * Do the AMPDU specific header preparation
		 */

		if (regmpdu_pkt) {
			WL_AMPDU_TX(("%s: reg mpdu found: WLF_MIMO %d Type/Subtype 0x%x\n",
			             __FUNCTION__,
			             ((pkttag->flags & WLF_MIMO) != 0),
			             (ltoh16(h->fc) & FC_KIND_MASK)));

			/* clear ampdu bit, set svht bit */
			vhtHdr->PktInfo.MacTxControlLow &= htol16(~D11AC_TXC_AMPDU);
#ifdef WL_MU_TX
			/* clear mu bit */
			vhtHdr->PktInfo.MacTxControlHigh &= htol16(~D11AC_TXC_MU);
#endif // endif
			/* non-ampdu must have svht bit set */
			vhtHdr->PktInfo.MacTxControlHigh |= htol16(D11AC_TXC_SVHT);

			/* if previous frame is an AMPDU mpdu, flip the epoch */
			if (hagg->prev_ft != AMPDU_NONE) {
				wlc_txh_info_t tx_info;

				/* ampdu switch to regmpdu */
				wlc_get_txh_info(wlc, p, &tx_info);
				wlc_ampdu_chgnsav_epoch(ampdu_tx, ac,
					AMU_EPOCH_CHG_NAGG, scb, tid, &tx_info);
				wlc_txh_set_epoch(ampdu_tx->wlc, txd, hagg->epoch);
				hagg->prev_ft = AMPDU_NONE;
			}
			/* otherwise, if prev is regmpdu already, don't set epoch at all */
		} else {
			bool change = FALSE;
			int8 reason_code = 0;

			/* clear svht bit, set ampdu bit */
			vhtHdr->PktInfo.MacTxControlLow |= htol16(D11AC_TXC_AMPDU);
			/* for real ampdu, clear vht single mpdu ampdu bit */
			vhtHdr->PktInfo.MacTxControlHigh &= htol16(~D11AC_TXC_SVHT);
#ifdef BCMDBG_ASSERT
			ASSERT(wlc_ampdu_check_percache_info(wlc, ampdu_tx, scb, tid, vhtHdr));
#endif // endif

			if (check_epoch) {
				uint16 frameid;

				change = TRUE;
				frameid = ltoh16(vhtHdr->PktInfo.TxFrameID);
				if (frameid & TXFID_RATE_PROBE_MASK) {
					/* check vrate probe flag
					 * this flag is only get the frame to become first mpdu
					 * of ampdu and no need to save epoch_params
					 */
					wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid,
					                            FALSE, 0, ac);
					reason_code = AMU_EPOCH_CHG_FID;
				} else if (scb != hagg->prev_scb || tid != hagg->prev_tid) {
					reason_code = AMU_EPOCH_CHG_DSTTID;
				} else if (wlc_ampdu_epoch_params_chg(wlc, hagg, vhtHdr)) {
					/* rate/sgi/bw/stbc/antsel tx params */
					reason_code = AMU_EPOCH_CHG_PLCP;
				} else if (ltoh16(vhtHdr->PktInfo.MacTxControlLow) &
				           D11AC_TXC_UPD_CACHE) {
					reason_code = AMU_EPOCH_CHG_TXC_UPD;
				} else if ((txd[2] & TOE_F2_TXD_HEAD_SHORT) != hagg->prev_shdr) {
					reason_code = AMU_EPOCH_CHG_TXHDR;
				} else {
					change = FALSE;
				}

				check_epoch = FALSE;
#if defined(WLC_UCODE_CACHE) && D11CONF_GE(42)
				/* If this frame signals a HW Cache update, then it will have
				 * a long txd ucode header. The next frame (if it is a cache hit)
				 * will have a short txd.
				 * Reset check_epoch so next time through the loop, the next
				 * packet will be checked for the long/short txd header.
				 */
				if (ltoh16(vhtHdr->PktInfo.MacTxControlLow) & D11AC_TXC_UPD_CACHE) {
					check_epoch = TRUE;
				}
#endif /* WLC_UCODE_CACHE */
			}

			if (!change) {
				if (hagg->prev_ft == AMPDU_NONE) {
					/* switching from non-aggregate to aggregate ampdu,
					 * just update the epoch changing parameters(hagg) but
					 * not the epoch.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_NO_CHANGE;
				} else if (seq_hole) {
					/* Flip the epoch if there is a hole in sequence of pkts
					 * sent to txfifo. It is a AQM HW requirement
					 * not to have the holes in txfifo.
					 */
					change = TRUE;
					reason_code = AMU_EPOCH_CHG_SEQ;
				}
			}

			if (change) {
				wlc_txh_info_t tx_info;
				wlc_get_txh_info(wlc, p, &tx_info);
				wlc_ampdu_chgnsav_epoch(ampdu_tx, ac, reason_code,
					scb, tid, &tx_info);
			}

			/* always set epoch for ampdu */
			wlc_txh_set_epoch(ampdu_tx->wlc, txd, hagg->epoch);
		}

		/* Supply the MPDU to output queue */
		pktenq(output_q, p);
		supplied_time += pkttime;
		count++;
	}

	if (count) {
		AMPDUSCBCNTADD(scb_ampdu->cnt.txmpdu, count);
		if (ini) {
			ini->tx_in_transit += (uint16)count;
#ifdef WLATF
			ampdu_tx->tx_intransit += (uint16)count;
#endif /* WLATF */
		}
	}

	/* if nothing was output, then this mux source is stalled. Update the scbq state
	 * so that scbq can restart the mux source when pkts arrive.
	 */
	if (supplied_time == 0) {
		wlc_scbq_scb_stall_set(wlc->tx_scbq, scb, ac);
	}

	WL_AMPDU_TX(("%s: "MACF" fifo %d epoch %d count %d supplied_time %d\n",
	             __FUNCTION__, ETHER_TO_MACF(scb->ea),
	             ac, ampdu_tx->hagg[ac].epoch, count, supplied_time));

	return supplied_time;
}

/**
 * Helper function for the A-MPDU Tx MUX output functions. This fn will handle output
 * of packets from a TID (priority) that does not have a BA Agreement. All the packets
 * at the given TID will be drained an sent non-aggregated and outside of a BA Agreement.
 *
 * Similar to the Tx MUX output fuctions that use this, the function will return the total estimated
 * tx airtime of packets that it adds to the output_q.
 *
 * @param q             A pointer to the SCBQ packet queue.
 * @param scb           A pointer to the SCB we are working on.
 * @param wlc           wlc_info_t pointer
 * @param ac            Access Category traffic being requested
 * @param tid           The TID (priority) of pakcets to drain and prepare.
 * @param request_time  the total estimated TX time the caller is requesting from the SCB
 * @param output_q      pointer to a packet queue to use to store the output packets
 * @return              The estimated TX time of the packets returned in the output_q. The time
 *                      returned may be greater than the 'request_time'
 */
static int BCMFASTPATH
wlc_ampdu_output_nonba(struct pktq *q, struct scb *scb, wlc_info_t *wlc, ratespec_t rspec,
                       uint ac, uint tid, uint request_time, struct spktq *output_q)
{
	wlc_txh_info_t txh_info;
	wlc_pkttag_t *pkttag;
	uint16 prec_map;
	int prec;
	void *pkt[DOT11_MAXNUMFRAGS] = {0};
	int i, count;
	DBGONLY(int pkt_count = 0; )
	uint pkttime;
	uint supplied_time = 0;
#ifdef WLAMPDU_MAC
	bool check_epoch = TRUE;
#endif /* WLAMPDU_MAC */

	/* use a prec_map that matches the tid prio parameter */
	prec_map = ((1 << WLC_PRIO_TO_PREC(tid)) | (1 << WLC_PRIO_TO_HI_PREC(tid)));

	while (supplied_time < request_time && (pkt[0] = pktq_mdeq(q, prec_map, &prec))) {
		pkttag = WLPKTTAG(pkt[0]);

		if (!(pkttag->flags & WLF_MPDU)) {
			/*
			 * MSDU packet prep
			 */

			int err;
			uint fifo; /* is this param needed anymore ? */

			pkttime = 0;

			err = wlc_prep_sdu(wlc, scb, pkt, &count, &fifo);
			/* WES:XXX Should the error be checked for memory error in the frag case?
			 * wlc_prep_sdu() is currently returning BCME_BUSY when it cannot allocate
			 * frags.
			 * Also returns BCME_ERROR on toss.
			 */
			if (err == BCME_OK) {
				for (i = 0; i < count; i++) {
					uint16 frag_time;

					wlc_get_txh_info(wlc, pkt[i], &txh_info);

					/* calculate and store the estimated pkt tx time */
					frag_time =
					        (uint16)wlc_tx_mpdu_time(wlc, scb, rspec, ac,
					                                 txh_info.d11FrameSize);

					WLPKTTIME(pkt[i]) = frag_time;

					pkttime += frag_time;
				}
			} else if (err == BCME_ERROR) {
				/* BCME_ERROR indicates a tossed packet */

				/* pkt[] should be invalid and count zero */
				ASSERT(count == 0);

				/* let the code finish the loop adding no time
				 * for this dequeued packet, and enqueue nothing to
				 * output_q since count == 0
				 */
				pkttime = 0;
			} else {
				/* should be no other errors */
				ASSERT(err == BCME_OK);
				/* XXX, Well BUSY can still happen for frags, but should not
				 * run into that. Need to implement.
				 * BUSY for block_datafifo, but we check before this loop
				 * BUSY for SCB_ISMULTI(), but until we run AP, no SCB is multi
				 */
				for (i = 0; i < count; i++) {
					PKTFREE(wlc->osh, pkt[i], TRUE);
				}
				pkttime = 0;
			}
		} else {
			/*
			 * MPDU packet prep
			 */

			uint fifo; /* is this param needed anymore ? */

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
			WLPKTTIME(pkt[0]) = (uint16)pkttime;
		}

		DBGONLY(pkt_count += count; )
		supplied_time += pkttime;

		for (i = 0; i < count; i++) {
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
					bool epoch;

					wlc_get_txh_info(wlc, pkt[i], &txh_info);
					epoch = wlc_ampdu_chgnsav_epoch(wlc->ampdu_tx,
					                                ac,
					                                AMU_EPOCH_CHG_MPDU,
					                                scb,
					                                (uint8)tid,
					                                &txh_info);
					wlc_txh_set_epoch(wlc, txh_info.tsoHdrPtr, epoch);
				}
			}
#endif /* WLAMPDU_MAC */

			{
				uint16 frameid;

				wlc_get_txh_info(wlc, pkt[i], &txh_info);

				frameid = ltoh16(txh_info.TxFrameID);
				if (frameid & TXFID_RATE_PROBE_MASK) {
					wlc_scb_ratesel_probe_ready(wlc->wrsi, scb, frameid,
					                            FALSE, 0, ac);
				}
			}

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

	WL_AMPDU_TX(("%s: fifo %d tid %d count %d supplied_time %d\n",
	             __FUNCTION__,
	             ac, tid, pkt_count, supplied_time));

	return supplied_time;
}

/* Max Mixed Mode time:
 * The time protected by a Mixed Mode OFDM PLCP header.
 * The legacy header is a 6Mbps OFDM header with a max LENGTH value of 4095 bytes.
 * The time in us after the legacy header (L-PREAMBLE/L-SIG) with max L_LENGTH=4095 is
 * 4095 bytes + (16bit(service) + 6bit(tail)) = 4098 bytes
 * 4098B / 3 bytes per symbol = 1366 symbols
 * 1366 symbols * 4us per OFDM symbol = 5464 us
 */
#define MM_TIME          5464

/* DATA field of VHT or HT PPDU inside the Mixed Mode frame
 * follows 48 us of SIGNAL and PREAMBLE in the worst case.
 *
 * Longest VHT header time from P802.11ac_D7.0, sec 22.4.3 "TXTIME and PSDU_LENGTH calculation",
 * shows preamble as
 *        VHT_T_SIG_A(8) + VHT_T_STF(4) + Nvhtltf*VHT_T_LTF(4) + VHT_T_SIG_B(4)
 * with largest Nvhtltf=8 (22.3.8.3.5 VHT-LTF definition).
 * VHT Header time max = 48us
 * Longest HT header time from IEEE Std 802.11-2012, se 20.4.3 "TXTIME calculation",
 * shows preamble as
 *        HT_T_SIG(8) + HT_T_STF(4) + (HT_T_LTF1(4) - HT_T_LTFs(4)) + Nltf * HT_T_LTFs(4)
 * with largest Nltf=5 (20.3.9.4.6 HT-LTF definition)
 * HT Header time max = 32us
 *
 * MM_TIME - 48 = 5464 - 48 = 5416
 */
#define MM_MAX_DATA_TIME 5416

/* Number of SGI symbols in DATA field will be FLOOR( MM_MAX_DATA_TIME / 3.6 ) */
#define MM_SGI_MAX_SYM   1504
/* Number of Non-SGI symbols in DATA field will be FLOOR( MM_MAX_DATA_TIME / 4 ) */
#define MM_MAX_SYM       1354

/**
 * @brief Initialize an ampdu_aqm_timecalc struct for fast packet TX time calculations
 *
 * Initialize an ampdu_aqm_timecalc struct for fast packet TX time calculations.
 * This function will initialize a ampdu_aqm_timecalc struct with parameters derived from the
 * current band and BSS, and the length independent fixed time overhead. The initialized struct
 * may be used by wlc_ampdu_timecalc() to do a TX time estimate for a packet.
 *
 * @param scb_ampdu     a pointer to the scb_ampdu_tx_t per-scb ampdu tx state
 * @param ini           a pointer to the scb_ampdu_tid_ini_t initiator state
 * @param rspec         the rate for all calculations
 * @param ac            Access Category for all calculations, used to estimate MAC access delay
 * @param tc            pointer to a ampdu_aqm_timecalc struct
 */
static void
wlc_ampdu_init_aqm_timecalc(scb_ampdu_tx_t *scb_ampdu, scb_ampdu_tid_ini_t *ini,
                        ratespec_t rspec, uint ac, ampdu_aqm_timecalc_t *tc)
{
	struct scb* scb;
	wlcband_t *band;
	wlc_bsscfg_t *bsscfg;
	uint fixed_overhead_us;
	uint empty_frame_us;
	uint Ndbps;
	uint Nes;
	uint sgi;
	uint max_sym;
	uint32 max_bytes;
	uint max_pdu;
	uint min_limit;

	scb = scb_ampdu->scb;
	bsscfg = scb->bsscfg;
	band = wlc_scbband(scb);

	/* Calculate the fixed overhead (length independent) for a single A-MPDU.
	 * The caclulation used the time for the frame sequence not including the
	 * AMPDU itself, plus the time for a frame with no payload (len = 0)
	 * This is the time from preamble to FCS, no medium access or time
	 * for ACK frames in the frame exchange.
	 */
	/* time for a frame with len = 0 */
	empty_frame_us = wlc_txtime(rspec, BAND_2G(band->bandtype), TRUE, 0);

	/* time for frame exchange other than AMPDU */
	fixed_overhead_us = wlc_tx_mpdu_frame_seq_overhead(rspec, bsscfg, band, ac);

	/* total overhead without any A-MPDU payload */
	fixed_overhead_us += empty_frame_us;

	/* Ndbps is saved to calc the A-MPDU sub-frame time, and used here
	 * to calculate max byte length based on the max time in a Mixedmode Format
	 * HT or VHT frame.
	 * Ndbps is the number of payload bits carried in one PHY symbol.
	 */
	Ndbps = wlc_txtime_Ndbps(rspec);

	/* find the max number of symbols in a mixed mode frame */
	sgi = RSPEC_ISSGI(rspec);
	if (sgi) {
		max_sym = MM_SGI_MAX_SYM; /* SGI symbols */
	} else {
		max_sym = MM_MAX_SYM;     /* regular GI symbols */
	}

	/* Max number of bytes in an A-MPDU protected by a MM header.
	 *
	 * The calculation is the maximum number of bits in the DATA field (max_sym * Ndbps)
	 * minus the overhead of SERVICE and TAIL, and the remaining bits converted to bytes.
	 *
	 * The overhead is the bits of the SERVICE (16) field, and bits in (TAIL(6) * Nes).
	 * The SERVICE and TAIL bit size are the same in both VHT and HT frames.
	 */
	Nes = wlc_txtime_Nes(rspec);
	max_bytes = ((max_sym * Ndbps) - (VHT_N_SERVICE + Nes * VHT_N_TAIL)) / 8;

	/* Max number of bytes in an A-MPDU may be further limited by
	 * the receiver's advertized rx size.
	 */
	max_bytes = MIN(max_bytes, scb_ampdu->max_rxlen);

	/* Calc the max number of mpdus in an A-MPDU as the min of our self-imposed
	 * tx max_pdu, and the recipient's advertised BA WinSize (Block Ack Parameter Set
	 * "Buffer Size" field.)
	 */
	max_pdu = MIN(scb_ampdu->max_pdu, ini->ba_wsize);

	/* Calculate the boundary size for small frames in an A-MPDU.
	 * If a sub-frame is less than this byte length, then it should get
	 * 1/max_pdu of the A-MPDU overhead.
	 */
	min_limit = max_bytes / max_pdu;
	/* the min_limit value will be compared against the mpdu_len without the
	 * A-MPDU delimiter and FCS, so adjust
	 */
	min_limit -= AMPDU_DELIMITER_LEN + DOT11_FCS_LEN;

	tc->rspec = rspec;
	tc->Ndbps = (uint16)Ndbps;
	tc->sgi = (uint8)sgi;
	tc->fixed_overhead_us = (uint16)fixed_overhead_us;
	tc->fixed_overhead_pmpdu_us = (uint16)(fixed_overhead_us / max_pdu);
	tc->min_limit = (uint16)min_limit;
	tc->max_bytes = max_bytes;
}

/**
 * @brief Calculate a TX airtime estimate using a previously initialized ampdu_aqm_timecalc struct
 *
 * Calculate a TX airtime estimate using a previously initialized ampdu_aqm_timecalc struct.
 * This function uses an initialzed timecalc struct to calculate the TX airtime for a
 * packet with the given MPDU length.
 * The time is the time cotribution of the A-MPDU sub-frame plus a weigthed portion of the
 * A-MPDU frame exchange.
 *
 * @param tc            pointer to a timecalc_t struct
 * @param mpdu_len      length of packet starting with 802.11 header not including the FCS
 *
 * @return              The estimated TX time of the packet
 */
static uint
wlc_ampdu_aqm_timecalc(ampdu_aqm_timecalc_t *tc, uint mpdu_len)
{
	uint mpdu_subframe_time;
	uint ampdu_overhead_time;
	uint Nsym;
	uint Ndbps = tc->Ndbps;

	/* number of symbols in the A-MPDU for this frame is
	 * CEILING( bits / Ndbps )
	 *   FLOOR( (bits + (Ndbps-1)) / Ndbps )
	 * where Ndbps is "Number of Data Bits per Symbol" for the current rate.
	 * Include the bits for an A-MPDU delimiter and FCS
	 */
	Nsym = ((((mpdu_len + AMPDU_DELIMITER_LEN + DOT11_FCS_LEN) * 8) + (Ndbps - 1)) /
	        Ndbps);

	/* Could wrap in the min AMPDU desity limit calc. If Nsym is less then the min density time,
	 * round up to the density value.
	 */

	/* time for the A-MPDU sub frame is symbols times symbol time */
	if (tc->sgi) {
		/* SGI symbol time is 3.6us, but needs to be rounded up to 4us sym time
		 * VHT_T_SYML * CIELING( (Nsym * 3.6) / 4 )
		 * VHT_T_SYML * CIELING( Nsym * (4 * (9 / 10)) / 4 )
		 * VHT_T_SYML * CIELING( Nsym * 36 / 40 )
		 * VHT_T_SYML * FLOOR( (Nsym * 36 + 39) / 40 )
		 */
		mpdu_subframe_time = VHT_T_SYML * ((Nsym * 36 + 39) / 40);
	} else {
		/* Regular GI symbol time is just VHT_T_SYML = 4 */
		mpdu_subframe_time = VHT_T_SYML * Nsym;
	}

	/* If the overhead weighted by the ratio of the MPDU length to the max possible
	 * A-MPDU length is larger than the the overhead divided by the max MPDUs, then
	 * use it.
	 * Otherwise, use the overhead divided by the max MPDUs.
	 *
	 * The motivation for this can be show with an example.
	 * Take an example connection that can have A-MPDUS with at most 8 MPDUs or 4K bytes per
	 * aggregate. If we construct an A-MPDU out of 8 tiny (100 byte) frames, then
	 * the 8 tiny frames will fill the A-MPDU even though they total only 800 bytes.
	 * The total A-MPDU overhead should be attributed to the 8 frames ---
	 * (total_overhead / 8) for each frame.
	 *
	 * But if we instead sent 1500byte frames, only 2 would fit in the aggregate. The
	 * total A-MPDU overhead should be attributed to the 2 frames ---
	 * (total_overhead * ( MPDU Len (1500) / Max A-MPDU len (4K) ) for each
	 * That would distribute 3000/4000 of the ampdu overhead total, closer
	 * to the actual airtime than (1/8) of the overhead for each frame.
	 */
	if (mpdu_len > tc->min_limit) {
		ampdu_overhead_time = (mpdu_len * tc->fixed_overhead_us) / tc->max_bytes;
	} else {
		ampdu_overhead_time = tc->fixed_overhead_pmpdu_us;
	}

	return (mpdu_subframe_time + ampdu_overhead_time);
}

#endif /* WLAMPDU_MAC */

#endif /* TXQ_MUX */

#if defined(NEW_TXQ) && defined(WLAMPDU_MAC)
void
wlc_ampdu_set_epoch(ampdu_tx_info_t *ampdu_tx, int fifo, uint8 epoch)
{
	ASSERT(fifo < NFIFO_EXT);
	ASSERT(AMPDU_AQM_ENAB(ampdu_tx->wlc->pub));

	ampdu_tx->hagg[fifo].epoch = epoch;
	/* set prev_ft to AMPDU_11VHT, so wlc_ampdu_chgnsav_epoch will always be called. */
	ampdu_tx->hagg[fifo].prev_ft = AMPDU_11VHT;
} /* wlc_ampdu_set_epoch */
#endif /* NEW_TXQ && WLAMPDU_MAC */

#ifdef WL11K_ALL_MEAS
void wlc_ampdu_get_stats(wlc_info_t *wlc, rrm_stat_group_12_t *g12)
{
	ASSERT(wlc);
	ASSERT(g12);

#ifdef WLAMPDU_MAC
	if (wlc->clk && AMPDU_MAC_ENAB(wlc->pub)) {
		if (AMPDU_AQM_ENAB(wlc->pub)) {
			uint16 stat_addr = 0;
			uint16 val;
			uint32 total;
			int cnt = 0, i;

			stat_addr = 2 * wlc_read_shm(wlc, M_AMP_STATS_PTR);
			for (i = 0, total = 0; i < C_MPDUDEN_NBINS; i++) {
				val = wlc_read_shm(wlc, stat_addr + i * 2);
				total += val;
				cnt += (val * (i+1));
			}
			g12->txampdu = total;
			g12->txmpdu = cnt;
		} else {
			g12->txampdu = wlc_read_shm(wlc, MACSTAT_ADDR(MCSTOFF_TXAMPDU));
			g12->txmpdu = wlc_read_shm(wlc, MACSTAT_ADDR(MCSTOFF_TXMPDU));
		}
	} else
#endif /* WLAMPDU_MAC */
	{
		g12->txampdu = wlc->ampdu_tx->cnt->txampdu;
		g12->txmpdu = wlc->ampdu_tx->cnt->txmpdu;
	}
	g12->txampdubyte_h = wlc->ampdu_tx->cnt->txampdubyte_h;
	g12->txampdubyte_l = wlc->ampdu_tx->cnt->txampdubyte_l;

	/* from wlc_ampdu_rx.c */
	g12->rxampdu = wlc_ampdu_getstat_rxampdu(wlc);
	g12->rxmpdu = wlc_ampdu_getstat_rxmpdu(wlc);
	g12->rxampdubyte_h = wlc_ampdu_getstat_rxampdubyte_h(wlc);
	g12->rxampdubyte_l = wlc_ampdu_getstat_rxampdubyte_l(wlc);
	g12->ampducrcfail = wlc_ampdu_getstat_ampducrcfail(wlc);
}
#endif /* WL11K_ALL_MEAS */

bool
wlc_ampdu_scbstats_get_and_clr(wlc_info_t *wlc, struct scb *scb,
	ampdu_tx_scb_stats_t *ampdu_scb_stats)
{
	ampdu_tx_info_t *ampdu_tx = wlc ? wlc->ampdu_tx : NULL;
	scb_ampdu_tx_t *scb_ampdu = NULL;

	if ((!ampdu_scb_stats) || (!ampdu_tx))
		return FALSE;

	if (SCB_AMPDU(scb)) {
		scb_ampdu = SCB_AMPDU_TX_CUBBY(ampdu_tx, scb);
		memcpy(ampdu_scb_stats, scb_ampdu->ampdu_scb_stats, sizeof(ampdu_tx_scb_stats_t));
		memset(scb_ampdu->ampdu_scb_stats, 0, sizeof(ampdu_tx_scb_stats_t));
		return TRUE;
	}

	return FALSE;
}

void
wlc_ampdu_reset_txnoprog(ampdu_tx_info_t *ampdu_tx)
{
	if (!ampdu_tx) {
		return;
	}
	ampdu_tx->txnoprog_cnt = 0;
}
