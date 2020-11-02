/*
 * Common interface to the 802.11 Station Control Block (scb) structure
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
 * $Id: wlc_scb.h 778872 2019-09-12 09:31:53Z $
 */

#ifndef _wlc_scb_h_
#define _wlc_scb_h_

#include <proto/802.1d.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */

#include <wlc_keymgmt.h>

#ifdef PSTA
/* In Proxy STA mode we support up to max 50 hosts and repeater itself */
#define SCB_BSSCFG_BITSIZE ROUNDUP(51, NBBY)/NBBY
#else /* PSTA */
#define SCB_BSSCFG_BITSIZE ROUNDUP(32, NBBY)/NBBY
#if (WLC_MAXBSSCFG > 32)
#error "auth_bsscfg cannot handle WLC_MAXBSSCFG"
#endif // endif
#endif /* PSTA */

/** Information node for scb packet transmit path */
struct tx_path_node {
	txmod_tx_fn_t next_tx_fn;		/* Next function to be executed */
	void *next_handle;
	uint8 next_fid;			/* Next fid in transmit path */
	bool configured;		/* Whether this feature is configured */
};

/* gets WLPKTTAG from p and checks if it is an AMSDU or not to return 1 or 0 respectively */
#define WLPKTTAG_AMSDU(p) (WLPKTFLAG_AMSDU(WLPKTTAG(p)) ? 1 : 0)

#ifdef WLSCB_HISTO
/* pass VHT rspec to get histo index (MCS + 12 * NSS) */
#define VHT2HISTO(rspec) (((rspec) & RSPEC_VHT_MCS_MASK) + \
		12 * (((rspec & 0x70) >> RSPEC_VHT_NSS_SHIFT) - 1))

/* pass HT rspec to get VHT histo index mapping; (or WL_NUM_VHT_RATES at most) */
#define HT2HISTO(rspec) (((rspec) & RSPEC_RATE_MASK) >= WL_NUM_HT_RATES ? (WL_NUM_VHT_RATES - 1) : \
		(((rspec) & RSPEC_RATE_MASK) % 8) + 12 * (((rspec) & RSPEC_RATE_MASK) / 8))

/* array of 55 elements (0-54); elements provide mapping to index for
 *	1, 2, 5 (5.5), 6, 9, 11, 12, 18, 24, 36, 48, 54
 * eg.
 *	wlc_legacy_rate_index[1] is 0
 *	wlc_legacy_rate_index[2] is 1
 *	wlc_legacy_rate_index[5] is 2
 *	...
 *	wlc_legacy_rate_index[48] is 10
 *	wlc_legacy_rate_index[54] is 11
 * For numbers in between (eg. 3, 4, 49, 50) maps to closest lower index
 * eg.
 *	wlc_legacy_rate_index[3] is 1 (same as 2Mbps)
 *	wlc_legacy_rate_index[4] is 1 (same as 2Mbps)
 */
extern uint8 wlc_legacy_rate_index[];
extern const uint8 wlc_legacy_rate_index_len;

/* pass legacy rspec to get index 0-11 */
#define LEGACY2HISTO(rspec) (((rspec) & RSPEC_RATE_MASK) > 108 ? (WL_NUM_LEGACY_RATES - 1) : \
		wlc_legacy_rate_index[((rspec) & RSPEC_RATE_MASK) >> 1])

/* maps rspec to histogram index */
#define RSPEC2HISTO(rspec) ((RSPEC_ISVHT(rspec) || ((rspec) & 0x80) != 0) ? VHT2HISTO(rspec): (\
		RSPEC_ISHT(rspec) ? HT2HISTO(rspec) : (\
		RSPEC_ISLEGACY(rspec) ? LEGACY2HISTO(rspec): (0))))

/* true only when passed FC is for DATA which is neither NULL nor QOS_NULL */
#define FC_NON_NULL_DATA(fc) (FC_TYPE(fc) == FC_TYPE_DATA && \
		FC_SUBTYPE(fc) != FC_SUBTYPE_NULL && \
		FC_SUBTYPE(fc) != FC_SUBTYPE_QOS_NULL)
#define WLSCB_HISTO_ADD(map, rspec, count) do { \
	const uint8 rIdx = RSPEC2HISTO(rspec) % WL_NUM_VHT_RATES; \
	const uint8 bw = RSPEC2BW(rspec), bIdx = (bw > 0 && bw < 5) ? (bw - 1) : 0; \
	const uint8 rType = RSPEC2ENCODING(rspec); \
	const uint8 idx = (rType == WL_HISTO_RATE_TYPE_LEGACY) ? (rIdx % WL_NUM_LEGACY_RATES) : \
		((WL_NUM_LEGACY_RATES + WL_NUM_VHT_RATES * bIdx) + rIdx); \
		if ((rspec) && bw && idx < WL_HISTO_MAP1_ARR_LEN) { \
			(map).recent_type = rType; \
			(map).recent_index = idx; \
			if ((count) > 0) { \
				(map).arr[idx] += (count); \
			} \
		} else if ((map).recent_index && (count) > 0) { \
			(map).arr[(map).recent_index] += (count); \
		} \
} while (0)

/* increment counter of recent rate index */
#define WLSCB_HISTO_INC_RECENT(map, count) ((map).arr[(map).recent_index] += (count))

/* increments rxmap for the rspec */
#define WLSCB_HISTO_RX(scb, rspec, count) WLSCB_HISTO_ADD((scb)->histo.rx, rspec, count)

/* call WLSCB_HISTO_RX conditionally */
#define WLSCB_HISTO_RX_COND(cond, scb, rspec, count) do { \
	if (cond) { \
		WLSCB_HISTO_RX(scb, rspec, count); \
	} \
} while (0)

/* increments rxmap for the recent rspec */
#define WLSCB_HISTO_RX_INC_RECENT(scb, count) WLSCB_HISTO_INC_RECENT((scb)->histo.rx, count)

/* increments txmap for the rspec by given count */
#define WLSCB_HISTO_TX(scb, rspec, count) WLSCB_HISTO_ADD((scb)->histo.tx, rspec, count)

/* checks FC for non-NULL DATA before issuing WLSCB_HISTO_TX() */
#define WLSCB_HISTO_TX_DATA(scb, rspec, fc, count) do { \
	if (FC_NON_NULL_DATA(fc)) { \
		WLSCB_HISTO_TX(scb, rspec, count); \
	} \
} while (0)

/* checks for LEGACY before updating WLSCB_HISTO_TX_DATA() */
#define WLSCB_HISTO_TX_DATA_IF_LEGACY(scb, rspec, fc, count) do { \
	if (((rspec) & RSPEC_ENCODING_MASK) == 0) { \
		WLSCB_HISTO_TX_DATA(scb, rspec, fc, count); \
	} \
} while (0)

/* checks for HT/VHT before updating WLSCB_HISTO_TX() */
#define WLSCB_HISTO_TX_IF_HTVHT(scb, rspec, count) do { \
	if (((rspec) & RSPEC_ENCODING_MASK) != 0) { \
		WLSCB_HISTO_TX(scb, rspec, count); \
	} \
} while (0)

#else /* WLSCB_HISTO */
#define WLSCB_HISTO_RX(a, b, c) do { BCM_REFERENCE(a); BCM_REFERENCE(b); BCM_REFERENCE(c); \
} while (0)
#define WLSCB_HISTO_RX_COND(a, b, c, d) do { BCM_REFERENCE(a); BCM_REFERENCE(b); BCM_REFERENCE(c); \
BCM_REFERENCE(d); } while (0)
#define WLSCB_HISTO_RX_INC_RECENT(a, b) do { BCM_REFERENCE(a); BCM_REFERENCE(b); } while (0)
#define WLSCB_HISTO_TX(a, b, c) do { BCM_REFERENCE(a); BCM_REFERENCE(b); BCM_REFERENCE(c); \
} while (0)
#define WLSCB_HISTO_TX_DATA(a, b, c, d) do { BCM_REFERENCE(a); BCM_REFERENCE(b); BCM_REFERENCE(c); \
BCM_REFERENCE(d); } while (0)
#define WLSCB_HISTO_TX_DATA_IF_LEGACY(a, b, c, d) do { BCM_REFERENCE(a); BCM_REFERENCE(b); \
BCM_REFERENCE(c); BCM_REFERENCE(d); } while (0)
#define WLSCB_HISTO_TX_IF_HTVHT(a, b, c) do { BCM_REFERENCE(a); BCM_REFERENCE(b); \
BCM_REFERENCE(c); } while (0)
#endif /* WLSCB_HISTO */

#define WLC_SCB_REPLAY_LIMIT	64	/* Maximal successive replay failure */

#ifdef WLCNTSCB
typedef struct wlc_scb_stats {
	uint32 tx_pkts;			/* # of packets transmitted (ucast) */
	uint32 tx_failures;		/* # of packets failed */
	uint32 rx_ucast_pkts;		/* # of unicast packets received */
	uint32 rx_mcast_pkts;		/* # of multicast packets received */
	ratespec_t tx_rate;		/* Rate of last successful tx frame */
	ratespec_t rx_rate;		/* Rate of last successful rx frame */
	uint32 rx_decrypt_succeeds;	/* # of packet decrypted successfully */
	uint32 rx_decrypt_failures;	/* # of packet decrypted unsuccessfully */
	uint32 tx_mcast_pkts;		/* # of mcast pkts txed */
	uint32 rx_succ_replay_failures;	/* # of successive replay failure  */
	uint64 tx_ucast_bytes;		/* data bytes txed (ucast) */
	uint64 tx_mcast_bytes;		/* data bytes txed (mcast) */
	uint64 rx_ucast_bytes;		/* data bytes recvd ucast */
	uint64 rx_mcast_bytes;		/* data bytes recvd mcast */
	uint32 tx_pkts_retried;		/* # of packets where a retry was necessary */
	uint32 tx_pkts_retry_exhausted;	/* # of packets where a retry was exhausted */
	ratespec_t tx_rate_mgmt;	/* Rate of last transmitted management frame */
	uint32 tx_rate_fallback;	/* last used lowest fallback TX rate */
	uint32 rx_pkts_retried;		/* # rx with retry bit set */
	uint32 tx_pkts_total;
	uint32 tx_pkts_retries;
	uint32 tx_pkts_fw_total;
	uint32 tx_pkts_fw_retries;
	uint32 tx_pkts_fw_retry_exhausted;
} wlc_scb_stats_t;
#endif /* WLCNTSCB */

typedef struct wlc_rate_histo {
	uint		vitxrspecidx;	/* Index into the video TX rate array */
	ratespec_t	vitxrspec[NVITXRATE][2];	/* History of Video MPDU's txrate */
	uint32		vitxrspectime[NVITXRATE][2];	/* Timestamp for each Video Tx */
	uint32		txrspectime[NTXRATE][2];	/* Timestamp for each Tx */
	uint		txrspecidx; /* Index into the TX rate array */
	ratespec_t	txrspec[NTXRATE][2];	/* History of MPDU's txrate */
	uint		rxrspecidx; /* Index into the Rx rate array */
	ratespec_t	rxrspec[NTXRATE];	/* History of MPDU's rxrate */
} wlc_rate_histo_t;

#if defined(BCMPCIEDEV)
/* Max number of samples in running buffer:
 * Keep max samples number a power of two, thus
 * all the division would be a single-cycle
 * bit-shifting.
 */
#define RAVG_EXP_PKT 2
#define RAVG_EXP_WGT 2

typedef struct _ravg {
	uint32 sum;
	uint8 idx;
} wlc_ravg_info_t;

#define RAVG_IDX(_obj_) ((_obj_)->idx)
#define RAVG_SUM(_obj_) ((_obj_)->sum)
#define RAVG_AVG(_obj_, _exp_) ((_obj_)->sum >> (_exp_))

/* Basic running average algorithm:
 * Keep a running buffer of the last N values, and a running SUM of all the
 * values in the buffer. Each time a new sample comes in, subtract the oldest
 * value in the buffer from SUM, replace it with the new sample, add the new
 * sample to SUM, and output SUM/N.
 */
#define RAVG_ADD(_obj_, _buf_, _sample_, _exp_) \
{ \
	if ((_buf_) != NULL) { \
		RAVG_SUM((_obj_)) -= _buf_[RAVG_IDX((_obj_))]; \
		RAVG_SUM((_obj_)) += (_sample_); \
		(_buf_)[RAVG_IDX((_obj_))] = (_sample_); \
		RAVG_IDX((_obj_)) = (RAVG_IDX((_obj_)) + 1) % (1 << (_exp_)); \
	} \
}

/* Initializing running buffer with value (_sample_) */
#define RAVG_INIT(_obj_, _buf_, _sample_, _exp_) \
{ \
	int v_ii; \
	if ((_obj_) != NULL) \
		memset((_obj_), 0, sizeof(*(_obj_))); \
	if ((_buf_) != NULL) { \
		memset((_buf_), 0, (sizeof((_buf_)[0]) * (1 << (_exp_)))); \
		for (v_ii = 0; v_ii < (1 << (_exp_)); v_ii++) { \
			RAVG_ADD((_obj_), (_buf_), (_sample_), (_exp_)); \
		} \
	} \
}
#define SZ_FLR_RBUF_TXPKTLEN (1 << RAVG_EXP_PKT)
#define SZ_FLR_RBUF_WEIGHT	(1 << RAVG_EXP_WGT)
#define PRIOMAP(_wlc_) ((_wlc_)->pciedev_prio_map)

#if defined(FLOW_PRIO_MAP_AC)
/* XXX: FLOW_PRIO_MAP_AC would be defined for PCIE FD router builds.
 * XXX: Router supports only 4 flow-rings per STA, therefore average
 * XXX: buffers would be 4.
 */
#define FLOWRING_PER_SCB_MAX AC_COUNT
#define RAVG_PRIO2FLR(_map_, _prio_) WME_PRIO2AC((_prio_))
#else  /* !FLOW_PRIO_MAP_AC */
/* XXX: Supports maximum up to 8 flow-rings, therefore average
 * XXX: buffers would be maximum 8.
 */
#define FLOWRING_PER_SCB_MAX NUMPRIO
/* XXX: Following the currently used prio mapping
 * XXX: configuration scheme in pcie bus layer.
 */
#define RAVG_PRIO2FLR(_map_, _prio_) \
	((((_map_) == PCIEDEV_AC_PRIO_MAP) ? WME_PRIO2AC((_prio_)) : (_prio_)))
#endif  /* FLOW_PRIO_MAP_AC */
#define SCB_FL_TXPKTS_RATIO		(20)
#endif /* BCMPCIEDEV */
#ifdef WLATF_PERC
#define	MIN_RESET_ATF_PERC		(1)
#endif /* WLATF_PERC */

/**
 * Information about a specific remote entity, and the relation between the local and that remote
 * entity. Station Control Block.
 */
struct scb {
	void *scb_priv;		/* internal scb data structure */
#ifdef MACOSX
	uint32 magic;
#endif // endif
	uint32	flags;		/* various bit flags as defined below */
	uint32	flags2;		/* various bit flags2 as defined below */
	wlc_bsscfg_t	*bsscfg;	/* bsscfg to which this scb belongs */
	struct ether_addr ea;		/* station address, must be aligned */
#if defined(PKTC) || defined(PKTC_DONGLE)
	uint32	pktc_pps;		/* pps counter for activating pktc */
#endif // endif
	uint8   auth_bsscfg[SCB_BSSCFG_BITSIZE]; /* authentication state w/ respect to bsscfg(s) */
	uint8	state; /* current state bitfield of auth/assoc process */
	bool		permanent;	/* scb should not be reclaimed */
	uint		used;		/* time of last use */
	uint32		assoctime;	/* time of association */
	uint		bandunit;	/* tha band it belongs to */
	uint32	 WPA_auth;	/* WPA: authenticated key management */
	uint32	 wsec;	/* ucast security algo. should match key->algo. Needed before key is set */

	wlc_rateset_t	rateset;	/* operational rates for this remote station */

	void	*fragbuf[NUMPRIO];	/* defragmentation buffer per prio */
	uint	fragresid[NUMPRIO];	/* #bytes unused in frag buffer per prio */

	uint16	 seqctl[NUMPRIO];	/* seqctl of last received frame (for dups) */
	uint16	 seqctl_nonqos;		/* seqctl of last received frame (for dups) for
					 * non-QoS data and management
					 */
	uint16	 seqnum[NUMPRIO];	/* WME: driver maintained sw seqnum per priority */

	/* APSD configuration */
	struct {
		uint16		maxsplen;   /* Maximum Service Period Length from assoc req */
		ac_bitmap_t	ac_defl;    /* Bitmap of ACs enabled for APSD from assoc req */
		ac_bitmap_t	ac_trig;    /* Bitmap of ACs currently trigger-enabled */
		ac_bitmap_t	ac_delv;    /* Bitmap of ACs currently delivery-enabled */
	} apsd;

#ifdef AP
	uint16		aid;		/* association ID */
	uint8		*challenge;	/* pointer to shared key challenge info element */
	uint16		tbtt;		/* count of tbtt intervals since last ageing event */
	uint8		auth_alg;	/* 802.11 authentication mode */
	bool		PS;		/* remote STA in PS mode */
	uint8           ps_pretend;     /* AP pretending STA is in PS mode */
	uint		grace_attempts;	/* Additional attempts made beyond scb_timeout
					 * before scb is removed
					 */
#endif /* AP */
	uint8		*wpaie;		/* WPA IE */
	uint		wpaie_len;	/* Length of wpaie */
	wlc_if_t	*wds;		/* per-port WDS cookie */
	int		*rssi_window;	/* rssi samples */
	int		rssi_index;
	int		rssi_enabled;	/* enable rssi collection */
	uint16		cap;		/* sta's advertized capability field */
	uint16		listen;		/* minimum # bcn's to buffer PS traffic */
	struct tx_path_node	*tx_path; /* Function chain for tx path for a pkt */
#ifdef WLCNTSCB
	wlc_scb_stats_t scb_stats;
#endif /* WLCNTSCB */
	bool		stale_remove;
#ifdef PROP_TXSTATUS
	uint8		mac_address_handle;
#endif // endif
	bool		rssi_upd;		/* per scb rssi is enabled by ... */
#if defined(STA) && defined(DBG_BCN_LOSS)
	struct wlc_scb_dbg_bcn dbg_bcn;
#endif // endif
	uint32  flags3;     /* various bit flags3 as defined below */
	struct	scb *psta_prim;	/* pointer to primary proxy sta */
	int	rssi_chain[WL_RSSI_ANT_MAX][MA_WINDOW_SZ];
	txdelay_params_t	*txdelay_params;
	scb_delay_stats_t	*delay_stats[AC_COUNT];	/* per-AC delay stats */
#ifdef PROP_TXSTATUS
	uint16	first_sup_pkt;
#endif // endif
#ifdef PSPRETEND
	uint32 ps_pretend_start;
	uint32 ps_pretend_probe;
	uint32 ps_pretend_count;
	uint8  ps_pretend_succ_count;
	uint8  ps_pretend_failed_ack_count;
#ifdef BCMDBG
	uint32 ps_pretend_total_time_in_pps;
	uint32 ps_pretend_suppress_count;
	uint32 ps_pretend_suppress_index;
#endif /* BCMDBG */
#endif /* PSPRETEND */
#ifdef WL_CS_RESTRICT_RELEASE
	uint16	restrict_txwin;
	uint8	restrict_deadline;
#endif /* WL_CS_RESTRICT_RELEASE */
#if defined(BCMPCIEDEV)
	/* Running average buffer for tx packet's len per flow ring */
	uint16 flr_txpktlen_rbuf[FLOWRING_PER_SCB_MAX][SZ_FLR_RBUF_TXPKTLEN];
	/* Running average buffer for weight per flow ring */
	uint32 flr_weight_rbuf[FLOWRING_PER_SCB_MAX][SZ_FLR_RBUF_WEIGHT];
	/* Running average info for tx packet len per flow ring */
	wlc_ravg_info_t flr_txpktlen_ravg[FLOWRING_PER_SCB_MAX];
	/* Running average info for weight per flow ring */
	wlc_ravg_info_t flr_weigth_ravg[FLOWRING_PER_SCB_MAX];
#endif /* BCMPCIEDEV */
	uint8 link_bw;
#ifdef WL11K
	void *scb_rrm_stats;
	void *scb_rrm_tscm;
#endif /* WL11K */
#if defined(BCMPCIEDEV)
	uint32 flr_txpkt[FLOWRING_PER_SCB_MAX];
	uint32 phyrate;
#endif /* BCMPCIEDEV */
	uint16	 vhtcap_orig_mcsmap;	/* Actual VHT cap mcs map of STA */
	uint8 sup_chan_width;	/* Channel width supported */
	uint8 ext_nss_bw_sup;	/* IEEE 802.11 REVmc Draft 8.0 EXT_NSS_BW support */
#ifdef WLATF_PERC
	uint32 flr_staperc; /* atf percentage of sta flow ring */
#endif /* WLATF_PERC */
#ifdef WLSCB_HISTO
	wl_rate_histo_maps1_t histo;	/* mapped histogram of rates */
#endif /* WLSCB_HISTO */
#ifdef AP
	uint		wsec_auth_timeout;	/* timeout to handle nas/eap restart */
#endif /* AP */
	uint8   basic_rate_indx; /* First rate index which is flagged as basic in rateset */
#ifdef WL_SAE
	uint8 pmkid_included; /* PMKID included in assoc request */
#endif /* WL_SAE */
};

#if defined(BCMPCIEDEV)
#define TXPKTLEN_RAVG(_scb_, _ac_) (&(_scb_)->flr_txpktlen_ravg[(_ac_)])
#define TXPKTLEN_RBUF(_scb_, _ac_) ((_scb_)->flr_txpktlen_rbuf[(_ac_)])
#define WEIGHT_RAVG(_scb_, _ac_) (&(_scb_)->flr_weigth_ravg[(_ac_)])
#define WEIGHT_RBUF(_scb_, _ac_) ((_scb_)->flr_weight_rbuf[(_ac_)])
#define WEIGHT_TXPKT(_scb_, _ac_) ((_scb_)->flr_txpkt[(_ac_)])
#define WEIGHT_PHYRATE(_scb_) ((_scb_)->phyrate)
#endif /* BCMPCIEDEV */

typedef struct {
	struct scb *scb;
	uint8	oldstate;
} scb_state_upd_data_t;

#ifdef PSPRETEND
/* bit flags for (uint8) scb.PS_pretend */
#define PS_PRETEND_NOT_ACTIVE    0

/* PS_PRETEND_PROBING states to do probing to the scb */
#define PS_PRETEND_PROBING       (1 << 0)

/* PS_PRETEND_ACTIVE indicates that ps pretend is currently active */
#define	PS_PRETEND_ACTIVE        (1 << 1)

/* PS_PRETEND_ACTIVE_PMQ indicates that we have had a PPS PMQ entry */
#define	PS_PRETEND_ACTIVE_PMQ    (1 << 2)

/* PS_PRETEND_NO_BLOCK states that we should not expect to see a PPS
 * PMQ entry, hence, not to block ourselves waiting to get one
 */
#define PS_PRETEND_NO_BLOCK      (1 << 3)

/* PS_PRETEND_PREVENT states to not do normal ps pretend for a scb */
#define PS_PRETEND_PREVENT       (1 << 4)

/* PS_PRETEND_RECENT indicates a ps pretend was triggered recently */
#define PS_PRETEND_RECENT        (1 << 5)

/* PS_PRETEND_THRESHOLD indicates that the successive failed TX status
 * count has exceeded the threshold
 */
#define PS_PRETEND_THRESHOLD     (1 << 6)

/* PS_PRETEND_ON is a bit mask of all active states that is used
 * to clear the scb state when ps pretend exits
 */
#define PS_PRETEND_ON	(PS_PRETEND_ACTIVE | PS_PRETEND_PROBING | \
						PS_PRETEND_ACTIVE_PMQ | PS_PRETEND_THRESHOLD)
#endif /* PSPRETEND */

typedef enum {
	RSSI_UPDATE_FOR_WLC = 0,	       /* Driver level */
	RSSI_UPDATE_FOR_TM	       /* Traffic Management */
} scb_rssi_requestor_t;

extern bool wlc_scb_rssi_update_enable(struct scb *scb, bool enable, scb_rssi_requestor_t);

/* Test whether RSSI update is enabled. Made a macro to reduce fn call overhead. */
#define WLC_SCB_RSSI_UPDATE_ENABLED(scb) (scb->rssi_upd != 0)

/** Iterator for scb list */
struct scb_iter {
	struct scb	*next;			/* next scb in bss */
	wlc_bsscfg_t	*next_bss;		/* next bss pointer */
	bool		all;			/* walk all bss or not */
};

#define SCB_BSSCFG(a)           ((a)->bsscfg)

/** Initialize an scb iterator pre-fetching the next scb as it moves along the list */
void wlc_scb_iterinit(scb_module_t *scbstate, struct scb_iter *scbiter,
	wlc_bsscfg_t *bsscfg);
/** move the iterator */
struct scb *wlc_scb_iternext(scb_module_t *scbstate, struct scb_iter *scbiter);

/* Iterate thru' scbs of specified bss */
#define FOREACH_BSS_SCB(scbstate, scbiter, bss, scb) \
	for (wlc_scb_iterinit((scbstate), (scbiter), (bss)); \
	     ((scb) = wlc_scb_iternext((scbstate), (scbiter))) != NULL; )

/* Iterate thru' scbs of all bss. Use this only when needed. For most of
 * the cases above one should suffice.
 */
#define FOREACHSCB(scbstate, scbiter, scb) \
	for (wlc_scb_iterinit((scbstate), (scbiter), NULL); \
	     ((scb) = wlc_scb_iternext((scbstate), (scbiter))) != NULL; )

scb_module_t *wlc_scb_attach(wlc_info_t *wlc);
void wlc_scb_detach(scb_module_t *scbstate);

/* scb cubby cb functions */
typedef int (*scb_cubby_init_t)(void *, struct scb *);
typedef void (*scb_cubby_deinit_t)(void *, struct scb *);
typedef void (*scb_cubby_dump_t)(void *, struct scb *, struct bcmstrbuf *b);

/**
 * This function allocates an opaque cubby of the requested size in the scb container.
 * The cb functions fn_init/fn_deinit are called when a scb is allocated/freed.
 * The functions are called with the context passed in and a scb pointer.
 * It returns a handle that can be used in macro SCB_CUBBY to retrieve the cubby.
 * Function returns a negative number on failure
 */
int wlc_scb_cubby_reserve(wlc_info_t *wlc, uint size, scb_cubby_init_t fn_init,
	scb_cubby_deinit_t fn_deinit, scb_cubby_dump_t fn_dump, void *context);

/* macro to retrieve pointer to module specific opaque data in scb container */
#define SCB_CUBBY(scb, handle)	(void *)(((uint8 *)(scb)) + handle)

/*
 * Accessors
 */

struct wlcband * wlc_scbband(struct scb *scb);

/** Find station control block corresponding to the remote id */
struct scb *wlc_scbfind(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea);

/** Lookup station control for ID. If not found, create a new entry. */
struct scb *wlc_scblookup(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea);

/** Lookup station control for ID. If not found, create a new entry. */
struct scb *wlc_scblookupband(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
                              const struct ether_addr *ea, int bandunit);

/** Get scb from band */
struct scb *wlc_scbfindband(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
                            const struct ether_addr *ea, int bandunit);

/** Determine if any SCB associated to ap cfg */
bool wlc_scb_associated_to_ap(wlc_info_t *wlc, wlc_bsscfg_t *cfg);

/** Move the scb's band info */
void wlc_scb_update_band_for_cfg(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, chanspec_t chanspec);

extern struct scb *wlc_scbibssfindband(wlc_info_t *wlc, const struct ether_addr *ea,
	int bandunit, wlc_bsscfg_t **bsscfg);

/** Find the STA acorss all APs */
extern struct scb *wlc_scbapfind(wlc_info_t *wlc, const struct ether_addr *ea,
	wlc_bsscfg_t **bsscfg);

extern struct scb *wlc_scbbssfindband(wlc_info_t *wlc, const struct ether_addr *hwaddr,
	const struct ether_addr *ea, int bandunit, wlc_bsscfg_t **bsscfg);

struct scb *wlc_internalscb_alloc(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	const struct ether_addr *ea, struct wlcband *band);
void wlc_internalscb_free(wlc_info_t *wlc, struct scb *scb);

bool wlc_scbfree(wlc_info_t *wlc, struct scb *remove);

/** * "|" operation */
void wlc_scb_setstatebit(struct scb *scb, uint8 state);

/** * "& ~" operation . */
void wlc_scb_clearstatebit(struct scb *scb, uint8 state);

/** * "|" operation . idx = position of the bsscfg in the wlc array of multi ssids. */

void wlc_scb_setstatebit_bsscfg(struct scb *scb, uint8 state, int idx);

/** * "& ~" operation . idx = position of the bsscfg in the wlc array of multi ssids. */
void wlc_scb_clearstatebit_bsscfg(struct scb *scb, uint8 state, int idx);

/** * reset all state. the multi ssid array is cleared as well. */
void wlc_scb_resetstate(struct scb *scb);

void wlc_scb_reinit(wlc_info_t *wlc);

/** free all scbs of a bsscfg */
void wlc_scb_bsscfg_scbclear(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg, bool perm);

/** (de)authorize/(de)authenticate single station */
void wlc_scb_set_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb, bool enable,
                      uint32 flag, int rc);

/** sort rates for a single scb */
void wlc_scb_sortrates(wlc_info_t *wlc, struct scb *scb);

/** sort rates for all scb in wlc */
void BCMINITFN(wlc_scblist_validaterates)(wlc_info_t *wlc);

extern void wlc_pktq_scb_free(wlc_info_t *wlc, struct pktq *pktq, struct scb *remove);
extern int wlc_txq_scb_init(void *ctx, struct scb *scb);
extern void wlc_pktq_scb_free(wlc_info_t *wlc, struct pktq *q, struct scb *remove);
extern void wlc_txq_scb_deinit(void *context, struct scb *remove);
extern void wlc_txmod_fn_register(wlc_info_t *wlc, scb_txmod_t feature_id, void *ctx,
                                  txmod_fns_t fns);
extern void wlc_txmod_config(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid);
extern void wlc_txmod_unconfig(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid);

#ifdef PROP_TXSTATUS
extern void wlc_scb_update_available_traffic_info(wlc_info_t *wlc, uint8 mac_handle, uint8 ta_bmp);
extern bool wlc_flow_ring_scb_update_available_traffic_info(wlc_info_t *wlc, uint8 mac_handle,
	uint8 tid, bool op);
uint16 wlc_flow_ring_get_scb_handle(wlc_info_t *wlc, struct wlc_if *wlcif, uint8 *da);
extern void wlc_flush_flowring_pkts(wlc_info_t *wlc, struct wlc_if *wlcif, uint8 *addr,
	uint16 flowid, uint8 tid_ac);
#if defined(BCMPCIEDEV)
extern uint32 wlc_flow_ring_reset_weight(wlc_info_t *wlc,
	struct wlc_if *wlcif, uint8 *da, uint8 fl);
extern void wlc_scb_upd_all_flr_weight(wlc_info_t *wlc, struct scb *scb);
extern void wlc_scb_upd_all_flr_perc(wlc_info_t *wlc, struct scb *scb, uint32 perc);
#endif /* BCMPCIEDEV */
#endif /* PROP_TXSTATUS */

extern void wlc_scb_set_bsscfg(struct scb *scb, wlc_bsscfg_t *cfg);

#if defined(BCMPCIEDEV)
extern void wlc_ravg_add_weight(wlc_info_t *wlc, struct scb *scb, int fl,
	ratespec_t rspec);
extern ratespec_t wlc_ravg_get_scb_cur_rspec(wlc_info_t *wlc, struct scb *scb);
#endif /* BCMPCIEDEV */
extern uint32 wlc_scb_dot11hdrsize(struct scb *scb);
extern uint8 wlc_scb_link_bw_update(wlc_info_t *wlc, struct scb *scb);

#ifdef WLTAF
extern uint32 wlc_ts2_traffic_shaper(struct scb *scb, uint32 weight);
#endif // endif

/* average rssi over window */
#if defined(AP) || defined(WLTDLS)
int wlc_scb_rssi(struct scb *scb);
void wlc_scb_rssi_init(struct scb *scb, int rssi);
int8 wlc_scb_rssi_chain(struct scb *scb, int chain);
/* rssi of last received packet per scb and per antenna chain */
int8 wlc_scb_pkt_rssi_chain(struct scb *scb, int chain);
#else
#define wlc_scb_rssi(a) 0
#define wlc_scb_rssi_init(a, b) 0
#define wlc_scb_rssi_chain(a, b) 0
#define wlc_scb_pkt_rssi_chain(a, b) 0
#endif /* AP || WLTDLS */
#ifdef WLAWDL
extern void wlc_scb_awdl_free(struct wlc_info *wlc);
extern void wlc_awdl_update_all_scb(struct wlc_info *wlc);
#endif // endif

/* SCB flags */
#define SCB_NONERP		0x0001		/* No ERP */
#define SCB_LONGSLOT		0x0002		/* Long Slot */
#define SCB_SHORTPREAMBLE	0x0004		/* Short Preamble ok */
#define SCB_8021XHDR		0x0008		/* 802.1x Header */
#define SCB_WPA_SUP		0x0010		/* 0 - authenticator, 1 - supplicant */
#define SCB_DEAUTH		0x0020		/* 0 - ok to deauth, 1 - no (just did) */
#define SCB_WMECAP		0x0040		/* WME Cap; may ONLY be set if WME_ENAB(wlc) */
#define SCB_USME2		0x0080
#define SCB_BRCM		0x0100		/* BRCM AP or STA */
#define SCB_WDS_LINKUP		0x0200		/* WDS link up */
#define SCB_LEGACY_AES		0x0400		/* legacy AES device */
#define SCB_USME1		0x0800
#define SCB_MYAP		0x1000		/* We are associated to this AP */
#define SCB_PENDING_PROBE	0x2000		/* Probe is pending to this SCB */
#define SCB_AMSDUCAP		0x4000		/* A-MSDU capable */
#define SCB_USEME		0x8000
#define SCB_HTCAP		0x10000		/* HT (MIMO) capable device */
#define SCB_RECV_PM		0x20000		/* state of PM bit in last data frame recv'd */
#define SCB_AMPDUCAP		0x40000		/* A-MPDU capable */
#define SCB_IS40		0x80000		/* 40MHz capable */
#define SCB_NONGF		0x100000	/* Not Green Field capable */
#define SCB_APSDCAP		0x200000	/* APSD capable */
#define SCB_PENDING_FREE	0x400000	/* marked for deletion - clip recursion */
#define SCB_PENDING_PSPOLL	0x800000	/* PS-Poll is pending to this SCB */
#define SCB_RIFSCAP		0x1000000	/* RIFS capable */
#define SCB_HT40INTOLERANT	0x2000000	/* 40 Intolerant */
#define SCB_WMEPS		0x4000000	/* PS + WME w/o APSD capable */
#define SCB_SENT_APSD_TRIG	0x8000000	/* APSD Trigger Null Frame was recently sent */
#define SCB_COEX_MGMT		0x10000000	/* Coexistence Management supported */
#define SCB_IBSS_PEER		0x20000000	/* Station is an IBSS peer */
#define SCB_STBCCAP		0x40000000	/* STBC Capable */
#ifdef WLBTAMP
#define SCB_11ECAP		0x80000000	/* 802.11e Cap; ONLY set if BTA_ENAB(wlc->pub) */
#endif // endif

/* scb flags2 */
#define SCB2_SGI20_CAP          0x00000001      /* 20MHz SGI Capable */
#define SCB2_SGI40_CAP          0x00000002      /* 40MHz SGI Capable */
#define SCB2_RX_LARGE_AGG       0x00000004      /* device can rx large aggs */
#define SCB2_INTERNAL           0x00000008      /* This scb is an internal scb */
#define SCB2_IN_ASSOC           0x00000010      /* Incoming assocation in progress */
#define SCB2_WAIHDR             0x00000020      /* WAI Header */
#define SCB2_P2P                0x00000040      /* WiFi P2P */
#define SCB2_LDPCCAP            0x00000080      /* LDPC Cap */
#define SCB2_BCMDCS             0x00000100      /* BCM_DCS */
#define SCB2_MFP                0x00000200      /* 802.11w MFP_ENABLE */
#define SCB2_SHA256             0x00000400      /* sha256 for AKM */
#define SCB2_VHTCAP             0x00000800      /* VHT (11ac) capable device */
#define SCB2_HT_PROP_RATES_CAP  0x00001000      /* Broadcom proprietary 11n rates */

#ifdef PROP_TXSTATUS
#define SCB2_PROPTXTSTATUS_SUPPR_STATEMASK      0x00001000
#define SCB2_PROPTXTSTATUS_SUPPR_STATESHIFT     12
#define SCB2_PROPTXTSTATUS_SUPPR_GENMASK        0x00002000
#define SCB2_PROPTXTSTATUS_SUPPR_GENSHIFT       13
#define SCB2_PROPTXTSTATUS_PKTWAITING_MASK      0x00004000
#define SCB2_PROPTXTSTATUS_PKTWAITING_SHIFT     14
#define SCB2_PROPTXTSTATUS_POLLRETRY_MASK       0x00008000
#define SCB2_PROPTXTSTATUS_POLLRETRY_SHIFT      15
/* 4 bits for AC[0-3] traffic pending status from the host */
#define SCB2_PROPTXTSTATUS_TIM_SHIFT            16
#define SCB2_PROPTXTSTATUS_TIM_MASK             (0xf << SCB2_PROPTXTSTATUS_TIM_SHIFT)
#endif // endif
#define SCB2_TDLS_PROHIBIT      0x00100000      /* TDLS prohibited */
#define SCB2_TDLS_CHSW_PROHIBIT 0x00200000      /* TDLS channel switch prohibited */
#define SCB2_TDLS_SUPPORT       0x00400000
#define SCB2_TDLS_PU_BUFFER_STA 0x00800000
#define SCB2_TDLS_PEER_PSM      0x01000000
#define SCB2_TDLS_CHSW_SUPPORT  0x02000000
#define SCB2_TDLS_PU_SLEEP_STA  0x04000000
#define SCB2_TDLS_MASK          0x07f00000
#define SCB2_IGN_SMPS		0x08000000 	/* ignore SM PS update */
#define SCB2_IS80               0x10000000      /* 80MHz capable */
#define SCB2_AMSDU_IN_AMPDU_CAP	0x20000000      /* AMSDU over AMPDU */
#define SCB2_CCX_MFP		0x40000000	/* CCX MFP enable */
#define SCB2_DWDS_ACTIVE		0x80000000      /* DWDS is active */

/* scb flags3 */
#define SCB3_A4_DATA		0x00000001      /* scb does 4 addr data frames */
#define SCB3_A4_NULLDATA	0x00000002	/* scb does 4-addr null data frames */
#define SCB3_A4_8021X		0x00000004	/* scb does 4-addr 8021x frames */

#ifdef WL_RELMCAST
#define SCB3_RELMCAST		0x00000800		/* Reliable Multicast */
#define SCB3_RELMCAST_NOACK	0x00001000		/* Reliable Multicast No ACK rxed */
#endif // endif

#define SCB3_PKTC		0x00002000      /* Enable packet chaining */
#define SCB3_OPER_MODE_NOTIF    0x00004000      /* 11ac Oper Mode Notif'n */

#ifdef WL11K
#define SCB3_RRM		0x00008000      /* Radio Measurement */
#define SCB_RRM(a)		((a)->flags3 & SCB3_RRM)
#else
#define SCB_RRM(a)		FALSE
#endif /* WL11K */

#define SCB3_DWDS_CAP		0x00010000      /* DWDS capable */
#define SCB3_IS_160		0x00020000      /* VHT 160 cap */
#define SCB3_IS_80_80		0x00040000      /* VHT 80+80 cap */
#define SCB3_1024QAM_CAP	0x00080000      /* VHT 1024QAM rates cap */
#define SCB3_MU_CAP		0x00100000      /* VHT MU cap */

#define SCB3_TS_MASK		0x00600000	/* Traffic Stream 2 flags */

#define SCB3_IS_10	0x00100000      /* ULB 10 MHz Capable */
#define SCB3_IS_5	0x00200000      /* ULB 5 MHz Capable */
#define SCB3_IS_2P5	0x00400000      /* ULB 2.5MHz Capable */

#define SCB3_TS_ATOS		0x00000000
#define SCB3_TS_ATOS2		0x00200000
#define SCB3_TS_EBOS		0x00400000

#define SCB3_HT_BEAMFORMEE      0x00800000      /* Receive NDP Capable */
#define SCB3_HT_BEAMFORMER      0x01000000      /* Transmit NDP Capable */
#define SCB3_MAP_CAP		0x02000000	/* MultiAP capable */

#ifdef WL_MBO
#define SCB3_MBO		0x04000000      /* MBO */
#define SCB_MBO(a)		((a)->flags3 & SCB3_MBO)
#else
#define SCB_MBO(a)		FALSE
#endif /* WL_MBO */

#define SCB3_GLOBAL_RCLASS_CAP	0X08000000
#define SCB_SUPPORTS_GLOBAL_RCLASS(a)	(((a)->flags3 & SCB3_GLOBAL_RCLASS_CAP) != 0)

#define SCB_TS_EBOS(a)		(((a)->flags3 & SCB3_TS_MASK) == SCB3_TS_EBOS)
#define SCB_TS_ATOS(a)		(((a)->flags3 & SCB3_TS_MASK) == SCB3_TS_ATOS)
#define SCB_TS_ATOS2(a)		(((a)->flags3 & SCB3_TS_MASK) == SCB3_TS_ATOS2)

#ifdef PROP_TXSTATUS
#define SCB_PROPTXTSTATUS_SUPPR_STATE(s)	(((s)->flags2 & \
	SCB2_PROPTXTSTATUS_SUPPR_STATEMASK) >> SCB2_PROPTXTSTATUS_SUPPR_STATESHIFT)
#define SCB_PROPTXTSTATUS_SUPPR_GEN(s)		(((s)->flags2 & SCB2_PROPTXTSTATUS_SUPPR_GENMASK) \
	>> SCB2_PROPTXTSTATUS_SUPPR_GENSHIFT)
#define SCB_PROPTXTSTATUS_TIM(s)		(((s)->flags2 & \
	SCB2_PROPTXTSTATUS_TIM_MASK) >> SCB2_PROPTXTSTATUS_TIM_SHIFT)
#define SCB_PROPTXTSTATUS_PKTWAITING(s)		(((s)->flags2 & \
	SCB2_PROPTXTSTATUS_PKTWAITING_MASK) >> SCB2_PROPTXTSTATUS_PKTWAITING_SHIFT)
#define SCB_PROPTXTSTATUS_POLLRETRY(s)		(((s)->flags2 & \
	SCB2_PROPTXTSTATUS_POLLRETRY_MASK) >> SCB2_PROPTXTSTATUS_POLLRETRY_SHIFT)

#define SCB_PROPTXTSTATUS_SUPPR_SETSTATE(s, state)	(s)->flags2 = ((s)->flags2 & \
		~SCB2_PROPTXTSTATUS_SUPPR_STATEMASK) | \
		(((state) << SCB2_PROPTXTSTATUS_SUPPR_STATESHIFT) & \
		SCB2_PROPTXTSTATUS_SUPPR_STATEMASK)
#define SCB_PROPTXTSTATUS_SUPPR_SETGEN(s, gen)	(s)->flags2 = ((s)->flags2 & \
		~SCB2_PROPTXTSTATUS_SUPPR_GENMASK) | \
		(((gen) << SCB2_PROPTXTSTATUS_SUPPR_GENSHIFT) & SCB2_PROPTXTSTATUS_SUPPR_GENMASK)
#define SCB_PROPTXTSTATUS_SETPKTWAITING(s, waiting)	(s)->flags2 = ((s)->flags2 & \
		~SCB2_PROPTXTSTATUS_PKTWAITING_MASK) | \
		(((waiting) << SCB2_PROPTXTSTATUS_PKTWAITING_SHIFT) & \
		SCB2_PROPTXTSTATUS_PKTWAITING_MASK)
#define SCB_PROPTXTSTATUS_SETPOLLRETRY(s, retry)	(s)->flags2 = ((s)->flags2 & \
		~SCB2_PROPTXTSTATUS_POLLRETRY_MASK) | \
		(((retry) << SCB2_PROPTXTSTATUS_POLLRETRY_SHIFT) & \
		SCB2_PROPTXTSTATUS_POLLRETRY_MASK)
#define SCB_PROPTXTSTATUS_SETTIM(s, tim)	(s)->flags2 = ((s)->flags2 & \
		~SCB2_PROPTXTSTATUS_TIM_MASK) | \
		(((tim) << SCB2_PROPTXTSTATUS_TIM_SHIFT) & SCB2_PROPTXTSTATUS_TIM_MASK)
#endif /* PROP_TXSTATUS */

/* scb vht flags */
#define SCB_VHT_LDPCCAP		0x0001
#define SCB_SGI80       0x0002
#define SCB_SGI160		0x0004
#define SCB_VHT_TX_STBCCAP	0x0008
#define SCB_VHT_RX_STBCCAP	0x0010
#define SCB_SU_BEAMFORMER	0x0020
#define SCB_SU_BEAMFORMEE	0x0040
#define SCB_MU_BEAMFORMER	0x0080
#define SCB_MU_BEAMFORMEE	0x0100
#define SCB_VHT_TXOP_PS		0x0200
#define SCB_HTC_VHT_CAP		0x0400

/* scb association state bitfield */
#define UNAUTHENTICATED		0	/* unknown */
#define AUTHENTICATED		1	/* 802.11 authenticated (open or shared key) */
#define ASSOCIATED		2	/* 802.11 associated */
#define PENDING_AUTH		4	/* Waiting for 802.11 authentication response */
#define PENDING_ASSOC		8	/* Waiting for 802.11 association response */
#define AUTHORIZED		0x10	/* 802.1X authorized */
#define MARKED_FOR_DELETION	0x20	/* Delete this scb after timeout */
#define TAKEN4IBSS		0x80	/* Taken */

/* scb association state helpers */
#define SCB_ASSOCIATED(a)	((a)->state & ASSOCIATED)
#define SCB_ASSOCIATING(a)	((a)->state & PENDING_ASSOC)
#define SCB_AUTHENTICATING(a)   ((a)->state & PENDING_AUTH)
#define SCB_AUTHENTICATED(a)	((a)->state & AUTHENTICATED)
#define SCB_AUTHORIZED(a)	((a)->state & AUTHORIZED)
#define SCB_MARKED_FOR_DELETION(a) ((a)->state & MARKED_FOR_DELETION)

/* flag access */
#define SCB_ISMYAP(a)           ((a)->flags & SCB_MYAP)
#define SCB_ISPERMANENT(a)      ((a)->permanent)
#define	SCB_INTERNAL(a) 	((a)->flags2 & SCB2_INTERNAL)
/* scb association state helpers w/ respect to ssid (in case of multi ssids)
 * The bit set in the bit field is relative to the current state (i.e. if
 * the current state is "associated", a 1 at the position "i" means the
 * sta is associated to ssid "i"
 */
#define SCB_ASSOCIATED_BSSCFG(a, i)	\
	(((a)->state & ASSOCIATED) && isset((scb->auth_bsscfg), i))

#define SCB_AUTHENTICATED_BSSCFG(a, i)	\
	(((a)->state & AUTHENTICATED) && isset((scb->auth_bsscfg), i))

#define SCB_AUTHORIZED_BSSCFG(a, i)	\
	(((a)->state & AUTHORIZED) && isset((scb->auth_bsscfg), i))

#define SCB_LONG_TIMEOUT	3600	/* # seconds of idle time after which we proactively
					 * free an authenticated SCB
					 */
#define SCB_SHORT_TIMEOUT	  60	/* # seconds of idle time after which we will reclaim an
					 * authenticated SCB if we would otherwise fail
					 * an SCB allocation.
					 */
#ifdef WLMEDIA_CUSTOMER_1
#define SCB_TIMEOUT		  10	/* # seconds: interval to probe idle STAs */
#else
#define SCB_TIMEOUT		  60	/* # seconds: interval to probe idle STAs */
#endif // endif
#define SCB_ACTIVITY_TIME	   5	/* # seconds: skip probe if activity during this time */
#define SCB_GRACE_ATTEMPTS	   10	/* # attempts to probe sta beyond scb_activity_time */

#define SCB_AUTH_TIMEOUT	   10	/* # seconds: interval to wait for auth reply from
					 * supplicant/authentication server.
					 */

/* scb_info macros */
#ifdef AP
#define SCB_PS(a)		((a) && (a)->PS)
#ifdef WDS
#define SCB_WDS(a)		((a)->wds)
#else
#define SCB_WDS(a)		NULL
#endif // endif
#define SCB_INTERFACE(a)        ((a)->wds ? (a)->wds->wlif : (a)->bsscfg->wlcif->wlif)
#define SCB_WLCIFP(a)           ((a)->wds ? (a)->wds : ((a)->bsscfg->wlcif))
#define WLC_BCMC_PSMODE(wlc, bsscfg) (SCB_PS(WLC_BCMCSCB_GET(wlc, bsscfg)))
#else
#define SCB_PS(a)		FALSE
#define SCB_WDS(a)		NULL
#define SCB_INTERFACE(a)        ((a)->bsscfg->wlcif->wlif)
#define SCB_WLCIFP(a)           (((a)->bsscfg->wlcif))
#define WLC_BCMC_PSMODE(wlc, bsscfg) (TRUE)
#endif /* AP */

#ifdef PSPRETEND
#define	SCB_PS_PRETEND(a)            ((a) && ((a)->ps_pretend & PS_PRETEND_ACTIVE))
#define SCB_PS_PRETEND_NORMALPS(a)   (SCB_PS(a) && !SCB_PS_PRETEND(a))
#define SCB_PS_PRETEND_THRESHOLD(a)  ((a) && ((a)->ps_pretend & PS_PRETEND_THRESHOLD))

/* Threshold mode never expects active PMQ, so do not block waiting for PMQ */
#define	SCB_PS_PRETEND_BLOCKED(a)    \
						(SCB_PS_PRETEND(a) && \
						!SCB_PS_PRETEND_THRESHOLD(a) && \
						!(((a)->ps_pretend & PS_PRETEND_ACTIVE_PMQ) || \
						((a)->ps_pretend & PS_PRETEND_NO_BLOCK)))

#define	SCB_PS_PRETEND_PROBING(a)	 \
						(SCB_PS_PRETEND(a) && \
						((a)->ps_pretend & \
						PS_PRETEND_PROBING))

#define SCB_PS_PRETEND_ENABLED(a)  \
						(PS_PRETEND_ENABLED(SCB_BSSCFG((a))) && \
						!((a)->ps_pretend & PS_PRETEND_PREVENT) && \
						!SCB_ISMULTI(a))

#define SCB_PS_PRETEND_THRESHOLD_ENABLED(a)  \
						(PS_PRETEND_THRESHOLD_ENABLED(SCB_BSSCFG((a))) && \
						!((a)->ps_pretend & PS_PRETEND_PREVENT) && \
						!SCB_ISMULTI(a))

#define SCB_PS_PRETEND_CSA_PREVENT(scb, bsscfg)    \
						((scb) && \
						((scb)->ps_pretend & PS_PRETEND_PREVENT) && \
						(BSSCFG_IS_CSA_IN_PROGRESS(bsscfg)))

#define SCB_PS_PRETEND_WAS_RECENT(a)	((a) && ((a)->ps_pretend & PS_PRETEND_RECENT))
#else
#define	SCB_PS_PRETEND(a)                       (0)
#define SCB_PS_PRETEND_NORMALPS(a)              SCB_PS(a)
#define	SCB_PS_PRETEND_BLOCKED(a)               (0)
#define SCB_PS_PRETEND_THRESHOLD(a)				(0)
#define	SCB_PS_PRETEND_PROBING(a)               (0)
#define SCB_PS_PRETEND_ENABLED(w, a)            (0)
#define SCB_PS_PRETEND_THRESHOLD_ENABLED(w, a)  (0)
#define SCB_PS_PRETEND_WAS_RECENT(a)            (0)
#define SCB_PS_PRETEND_CSA_PREVENT(scb, bsscfg) (0)
#endif /* PSPRETEND */

#ifdef WME
#define SCB_WME(a)		((a)->flags & SCB_WMECAP)	/* Also implies WME_ENAB(wlc) */
#else
#define SCB_WME(a)		((void)(a), FALSE)
#endif // endif

#ifdef WLAMPDU
#define SCB_AMPDU(a)		((a)->flags & SCB_AMPDUCAP)
#else
#define SCB_AMPDU(a)		FALSE
#endif // endif

#ifdef WLAMSDU
#define SCB_AMSDU(a)		((a)->flags & SCB_AMSDUCAP)
#define SCB_AMSDU_IN_AMPDU(a) ((a)->flags2 & SCB2_AMSDU_IN_AMPDU_CAP)
#else
#define SCB_AMSDU(a)		FALSE
#define SCB_AMSDU_IN_AMPDU(a) FALSE
#endif // endif

#ifdef WL11N
#define SCB_HT_CAP(a)		(((a)->flags & SCB_HTCAP) != 0)
#define SCB_VHT_CAP(a)		(((a)->flags2 & SCB2_VHTCAP) != 0)
#define SCB_ISGF_CAP(a)		(((a)->flags & (SCB_HTCAP | SCB_NONGF)) == SCB_HTCAP)
#define SCB_NONGF_CAP(a)	(((a)->flags & (SCB_HTCAP | SCB_NONGF)) == \
					(SCB_HTCAP | SCB_NONGF))
#define SCB_COEX_CAP(a)		((a)->flags & SCB_COEX_MGMT)
#define SCB_STBC_CAP(a)		((a)->flags & SCB_STBCCAP)
#define SCB_LDPC_CAP(a)		(SCB_HT_CAP(a) && ((a)->flags2 & SCB2_LDPCCAP))
#define SCB_HT_PROP_RATES_CAP(a) (((a)->flags2 & SCB2_HT_PROP_RATES_CAP) != 0)
#else /* WL11N */
#define SCB_HT_CAP(a)		FALSE
#define SCB_VHT_CAP(a)		FALSE
#define SCB_ISGF_CAP(a)		FALSE
#define SCB_NONGF_CAP(a)	FALSE
#define SCB_COEX_CAP(a)		FALSE
#define SCB_STBC_CAP(a)		FALSE
#define SCB_LDPC_CAP(a)		FALSE
#define SCB_HT_PROP_RATES_CAP(a) FALSE
#endif /* WL11N */

#ifdef WL11AC
#define SCB_VHT_LDPC_CAP(v, a)	(SCB_VHT_CAP(a) && \
	(wlc_vht_get_scb_flags(v, a) & SCB_VHT_LDPCCAP))
#define SCB_VHT_TX_STBC_CAP(v, a)	(SCB_VHT_CAP(a) && \
	(wlc_vht_get_scb_flags(v, a) & SCB_VHT_TX_STBCCAP))
#define SCB_VHT_RX_STBC_CAP(v, a)	(SCB_VHT_CAP(a) && \
	(wlc_vht_get_scb_flags(v, a) & SCB_VHT_RX_STBCCAP))
#define SCB_VHT_SGI80(v, a)	(SCB_VHT_CAP(a) && \
	(wlc_vht_get_scb_flags(v, a) & SCB_SGI80))
#define SCB_VHT_SGI160(v, a)	(SCB_VHT_CAP(a) && \
		(wlc_vht_get_scb_flags(v, a) & SCB_SGI160))
#define SCB_OPER_MODE_NOTIF_CAP(a) ((a)->flags3 & SCB3_OPER_MODE_NOTIF)
#else /* WL11AC */
#define SCB_VHT_LDPC_CAP(v, a)		FALSE
#define SCB_VHT_TX_STBC_CAP(v, a)	FALSE
#define SCB_VHT_RX_STBC_CAP(v, a)	FALSE
#define SCB_VHT_SGI80(v, a)		FALSE
#define SCB_VHT_SGI160(v, a)		FALSE
#define SCB_OPER_MODE_NOTIF_CAP(a) (0)
#endif /* WL11AC */

#define SCB_IS_IBSS_PEER(a)	((a)->flags & SCB_IBSS_PEER)
#define SCB_SET_IBSS_PEER(a)	((a)->flags |= SCB_IBSS_PEER)
#define SCB_UNSET_IBSS_PEER(a)	((a)->flags &= ~SCB_IBSS_PEER)

#if defined(PKTC) || defined(PKTC_DONGLE)
#define SCB_PKTC_ENABLE(a)	((a)->flags3 |= SCB3_PKTC)
#define SCB_PKTC_DISABLE(a)	((a)->flags3 &= ~SCB3_PKTC)
#define SCB_PKTC_ENABLED(a)	((a)->flags3 & SCB3_PKTC)
#else
#define SCB_PKTC_ENABLE(a)
#define SCB_PKTC_DISABLE(a)
#define SCB_PKTC_ENABLED(a)	FALSE
#endif // endif

#ifdef WLBTAMP
#define SCB_11E(a)		((a)->flags & SCB_11ECAP)
#else
#define SCB_11E(a)		FALSE
#endif // endif

#ifdef WLBTAMP
#define SCB_QOS(a)		((a)->flags & (SCB_WMECAP | SCB_HTCAP | SCB_11ECAP))
#else
#define SCB_QOS(a)		((a)->flags & (SCB_WMECAP | SCB_HTCAP))
#endif /* WLBTAMP */

#ifdef WLP2P
#define SCB_P2P(a)		((a)->flags2 & SCB2_P2P)
#else
#define SCB_P2P(a)		FALSE
#endif // endif

#ifdef DWDS
#define SCB_DWDS_CAP(a)		((a)->flags3 & SCB3_DWDS_CAP)
#define SCB_DWDS(a)		((a)->flags2 & SCB2_DWDS_ACTIVE)
#else
#define SCB_DWDS(a)		FALSE
#define SCB_DWDS_CAP(a)		FALSE
#endif // endif

#ifdef MULTIAP
#define SCB_MAP_CAP(a)		((a)->flags3 & SCB3_MAP_CAP)
#else
#define SCB_MAP_CAP(a)		FALSE
#endif // endif

#define SCB_DWDS_ACTIVATE(a)	((a)->flags2 |= SCB2_DWDS_ACTIVE)
#define SCB_DWDS_DEACTIVATE(a)	((a)->flags2 &= ~SCB2_DWDS_ACTIVE)

#define SCB_LEGACY_WDS(a)	(SCB_WDS(a) && !SCB_DWDS(a))

#define SCB_A4_DATA(a)		((a)->flags3 & SCB3_A4_DATA)
#define SCB_A4_DATA_ENABLE(a)	((a)->flags3 |= SCB3_A4_DATA)
#define SCB_A4_DATA_DISABLE(a)	((a)->flags3 &= ~SCB3_A4_DATA)

#define SCB_A4_NULLDATA(a)	((a)->flags3 & SCB3_A4_NULLDATA)
#define SCB_A4_8021X(a)		((a)->flags3 & SCB3_A4_8021X)

#define SCB_MFP(a)		((a) && ((a)->flags2 & SCB2_MFP))
#define SCB_SHA256(a)		((a) && ((a)->flags2 & SCB2_SHA256))
#define SCB_CCX_MFP(a)	((a) && ((a)->flags2 & SCB2_CCX_MFP))

#define SCB_MFP_ENABLE(a)       ((a)->flags2 |= SCB2_MFP)
#define SCB_MFP_DISABLE(a)      ((a)->flags2 &= ~SCB2_MFP)

#define SCB_1024QAM_CAP(a)	((a)->flags3 & SCB3_1024QAM_CAP)
#define SCB_BW160_CAP(a)	((a)->flags3 & (SCB3_IS_160 | SCB3_IS_80_80))

#define SCB_MU(a)		((a)->flags3 & SCB3_MU_CAP)
#define SCB_MU_ENABLE(a)	((a)->flags3 |= SCB3_MU_CAP)
#define SCB_MU_DISABLE(a)	((a)->flags3 &= ~SCB3_MU_CAP)

#define SCB_SEQNUM(scb, prio)	(scb)->seqnum[(prio)]

#define SCB_ISMULTI(a)	ETHER_ISMULTI((a)->ea.octet)

#ifdef WLCNTSCB
#define WLCNTSCBINCR(a)			((a)++)	/* Increment by 1 */
#define WLCNTSCBDECR(a)			((a)--)	/* Decrement by 1 */
#define WLCNTSCBADD(a,delta)		((a) += (delta)) /* Increment by specified value */
#define WLCNTSCBSET(a,value)		((a) = (value)) /* Set to specific value */
#define WLCNTSCBVAL(a)			(a)	/* Return value */
#define WLCNTSCB_COND_SET(c, a, v)	do { if (c) (a) = (v); } while (0)
#define WLCNTSCB_COND_ADD(c, a, d)	do { if (c) (a) += (d); } while (0)
#define WLCNTSCB_COND_INCR(c, a)	do { if (c) (a) += (1); } while (0)
#else /* WLCNTSCB */
#define WLCNTSCBINCR(a)			/* No stats support */
#define WLCNTSCBDECR(a)			/* No stats support */
#define WLCNTSCBADD(a,delta)		/* No stats support */
#define WLCNTSCBSET(a,value)		/* No stats support */
#define WLCNTSCBVAL(a)		0	/* No stats support */
#define WLCNTSCB_COND_SET(c, a, v)	/* No stats support */
#define WLCNTSCB_COND_ADD(c, a, d) 	/* No stats support */
#define WLCNTSCB_COND_INCR(c, a)	/* No stats support */
#endif /* WLCNTSCB */

/* Given the 'feature', invoke the next stage of transmission in tx path */
#ifdef TXQ_MUX
extern void wlc_scb_tx_next(uint fid, struct scb *scb, void *pkt, uint prec, const char *fn);
#define SCB_TX_NEXT(fid, scb, pkt, prec) wlc_scb_tx_next((fid), (scb), (pkt), (prec), __FUNCTION__)
#else
#define SCB_TX_NEXT(fid, scb, pkt, prec) \
	(scb->tx_path[(fid)].next_tx_fn((scb->tx_path[(fid)].next_handle), (scb), (pkt), (prec)))
#endif // endif

/* Is the feature currently in the path to handle transmit. ACTIVE implies CONFIGURED */
#define SCB_TXMOD_ACTIVE(scb, fid) (scb->tx_path[(fid)].next_tx_fn != NULL)

/* Is the feature configured? */
#define SCB_TXMOD_CONFIGURED(scb, fid) (scb->tx_path[(fid)].configured)

/* Next feature configured */
#define SCB_TXMOD_NEXT_FID(scb, fid) (scb->tx_path[(fid)].next_fid)

extern void wlc_scb_txmod_activate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid);
extern void wlc_scb_txmod_deactivate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid);

extern void wlc_scb_switch_band(wlc_info_t *wlc, struct scb *scb, int new_bandunit,
	wlc_bsscfg_t *bsscfg);

#ifdef WLAWDL
extern struct scb * wlc_scbfind_dualband(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct ether_addr *addr);
#endif // endif

extern int wlc_scb_save_wpa_ie(wlc_info_t *wlc, struct scb *scb, bcm_tlv_t *ie);

extern void wlc_internal_scb_switch_band(wlc_info_t *wlc, struct scb *scb, int new_bandunit);
extern int wlc_scb_state_upd_register(wlc_info_t *wlc, bcm_notif_client_callback fn, void *arg);
extern int wlc_scb_state_upd_unregister(wlc_info_t *wlc, bcm_notif_client_callback fn, void *arg);

#ifdef WL_CS_RESTRICT_RELEASE
/**
 * Limit number of packets in transit, starting from minimal number.
 * Each time packet sent successfully using primary rate limit is
 * exponentially grow till some number, then unlimited.
 * In case of failure while limit is growing, it fall back to
 * original minimal number.
 */

extern void wlc_scb_restrict_start(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
extern void wlc_scb_restrict_wd(wlc_info_t *wlc);

#define SCB_RESTRICT_WD_TIMEOUT		1
#define SCB_RESTRICT_MIN_TXWIN_SHIFT	1
#define SCB_RESTRICT_MAX_TXWIN_SHIFT	6

#define SCB_RESTRICT_MIN_TXWIN		(1 << (SCB_RESTRICT_MIN_TXWIN_SHIFT))
#define SCB_RESTRICT_MAX_TXWIN		(1 << (SCB_RESTRICT_MAX_TXWIN_SHIFT))

static INLINE void
wlc_scb_restrict_txstatus(struct scb *scb, bool success)
{
	if (!scb->restrict_txwin) {
		/* Disabled. Most likely case. */
	} else if (!success) {
		scb->restrict_txwin = SCB_RESTRICT_MIN_TXWIN;
	} else {
		scb->restrict_txwin = scb->restrict_txwin << 1;
		if (scb->restrict_txwin >= SCB_RESTRICT_MAX_TXWIN) {
			scb->restrict_txwin = 0;
		}
	}
}

static INLINE bool
wlc_scb_restrict_can_txq(wlc_info_t *wlc, struct scb *scb)
{
	if (!scb->restrict_txwin) {
		/* Disabled. Most likely case. Can release single packet. */
		return TRUE;
	} else {
		/*
		 * Return TRUE if number of packets in transit is less then restriction window.
		 * If TRUE caller can release single packet.
		 */
		return (TXPKTPENDTOT(wlc) < scb->restrict_txwin);
	}
}

static INLINE uint16
wlc_scb_restrict_can_ampduq(wlc_info_t *wlc, struct scb *scb, uint16 in_transit, uint16 release)
{
	if (!scb->restrict_txwin) {
		/* Disabled. Most likely case. Can release same number of packets as queried. */
		return release;
	} else if (in_transit >= scb->restrict_txwin) {
		/* Already too many packets in transit. Release denied. */
		return 0;
	} else {
		/* Return how many packets can be released. */
		return MIN(release, scb->restrict_txwin - in_transit);
	}
}

static INLINE bool
wlc_scb_restrict_can_txeval(wlc_info_t *wlc)
{
	/*
	 * Whether AMPDU txeval function can proceed or not.
	 * Prevents to release packets into txq if DATA_BLOCK_QUIET set
	 * (preparations to channel switch are in progress).
	 * Idea is in case of point-to-multipoint traffic
	 * better to have restrictions on boundary between AMPDU queue
	 * and txq, so single bad link would not affect much other good links.
	 * And to have this boundary efficient we need nothing at txq
	 * after channel switch, so control between AMPDU queue and txq
	 * would work.
	 */
	return ((wlc->block_datafifo & DATA_BLOCK_QUIET) == 0);
}

static INLINE bool
wlc_scb_restrict_do_probe(struct scb *scb)
{
	/* If restriction is not disabled yet, then frequent probing should be used. */
	return (scb->restrict_txwin != 0);
}

#if defined(BCMPCIEDEV)
static INLINE uint32
wlc_scb_calc_weight(uint32 pktlen_bytes, uint32 rate, bool legacy)
{
#define SCALE 100000
	uint32 rate_KBps;
	uint32 weight = 0;

	if (BCMPCIEDEV_ENAB()) {
		if (legacy) {
		/* For legacy rates the input rate unit is
		 * in Mbits/sec multiplied by 2.
		 * Converting to KBytes/sec.
		 * Formula is : rate * 1024 / 16
		 */
		rate_KBps = rate << 6;

		} else {
	   /* For HT and VHT rates the input rate unit is in
		* Kbits/sec. Converting to KBytes/sec.
		* Formula is: rate / 8
		*/
		rate_KBps = rate >> 3;
	}

	pktlen_bytes *= SCALE;
	weight = pktlen_bytes / rate_KBps;
	ASSERT(weight);
	/* XXX: The weight (which is airtime) never supposed to be 0.
	 * XXX: Assigning bare minimum weight.
	 */
	if (!weight)
		weight = 1;
	}
	return weight;
#undef SCALE
}
#endif /* BCMPCIEDEV */
#else
static INLINE void wlc_scb_restrict_start(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg) {}
static INLINE void wlc_scb_restrict_wd(wlc_info_t *wlc) {}
static INLINE void wlc_scb_restrict_txstatus(struct scb *scb, bool success) {}
static INLINE bool wlc_scb_restrict_can_txq(wlc_info_t *wlc, struct scb *scb) {return TRUE;}
static INLINE uint16 wlc_scb_restrict_can_ampduq(wlc_info_t *wlc,
	struct scb *scb, uint16 in_transit, uint16 release) {return release;}
static INLINE bool wlc_scb_restrict_can_txeval(wlc_info_t *wlc) {return TRUE;}
static INLINE bool wlc_scb_restrict_do_probe(struct scb *scb) {return FALSE;}
static INLINE uint32 wlc_scb_calc_weight(uint32 pktlen_bytes, uint32 rate, bool legacy) {return 0;}
#endif /* WL_CS_RESTRICT_RELEASE */

#ifdef PROP_TXSTATUS
extern int wlc_scb_wlfc_entry_add(wlc_info_t *wlc, struct scb *scb);
struct scb * wlc_scbfind_from_wlcif(wlc_info_t *wlc, struct wlc_if *wlcif, uint8 *addr);
#endif /* PROP_TXSTATUS */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
void wlc_scb_dump_scb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb,
	struct bcmstrbuf *b, int idx);
#endif // endif

void wlc_scbfind_delete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,	struct ether_addr *ea);

#if defined(PKTC) || defined(PKTC_DONGLE)
void wlc_scb_pktc_enable(struct scb *scb, const wlc_key_info_t *key_info);
void wlc_scb_pktc_disable(struct scb *scb);
#endif /* PKTC || PKTC_DONGLE */

#endif /* _wlc_scb_h_ */
