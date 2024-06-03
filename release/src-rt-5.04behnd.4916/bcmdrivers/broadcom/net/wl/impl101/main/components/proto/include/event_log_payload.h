/*
 * EVENT_LOG System Definitions
 *
 * This file describes the payloads of event log entries that are data buffers
 * rather than formatted string entries. The contents are generally XTLVs.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifndef _EVENT_LOG_PAYLOAD_H_
#define _EVENT_LOG_PAYLOAD_H_

#include <typedefs.h>
#include <bcmutils.h>
#include <ethernet.h>
#include <event_log_tag.h>
#include <wlioctl.h>

/**
 * A (legacy) timestamp message
 */
typedef struct ts_message {
	uint32 timestamp;
	uint32 cyclecount;
} ts_msg_t;

/**
 * Enhanced timestamp message
 */
typedef struct enhanced_ts_message {
	uint32 version;
	/* More data, depending on version */
	uint8 data[];
} ets_msg_t;

#define ENHANCED_TS_MSG_VERSION_1 (1u)

/**
 * Enhanced timestamp message, version 1
 */
typedef struct enhanced_ts_message_v1 {
	uint32 version;
	uint32 timestamp; /* PMU time, in milliseconds */
	uint32 cyclecount;
	uint32 cpu_freq;
} ets_msg_v1_t;

/**
 * Enhanced timestamp message, version 2
 */
#define ENHANCED_TS_MSG_VERSION_2 (2u)
typedef struct enhanced_ts_message_v2 {
	uint64 sysuptime_ns;		/* sysuptime in ns */
	uint64 ets_write_ptm_time;	/* PTM time at ETS message */
	uint64 host_time_offset;	/* Host time offset in ns from PTM time */
} ets_msg_v2_t;

#define EVENT_LOG_XTLV_ID_STR                   0u  /**< XTLV ID for a string */
#define EVENT_LOG_XTLV_ID_TXQ_SUM               1u  /**< XTLV ID for txq_summary_t */
#define EVENT_LOG_XTLV_ID_SCBDATA_SUM           2u  /**< XTLV ID for cb_subq_summary_t */
#define EVENT_LOG_XTLV_ID_SCBDATA_AMPDU_TX_SUM  3u  /**< XTLV ID for scb_ampdu_tx_summary_t */
#define EVENT_LOG_XTLV_ID_BSSCFGDATA_SUM        4u  /**< XTLV ID for bsscfg_q_summary_t */
#define EVENT_LOG_XTLV_ID_UCTXSTATUS            5u  /**< XTLV ID for ucode TxStatus array */
#define EVENT_LOG_XTLV_ID_TXQ_SUM_V2            6u  /**< XTLV ID for txq_summary_v2_t */
#define EVENT_LOG_XTLV_ID_BUF                   7u  /**< XTLV ID for event_log_buffer_t */

/**
 * An XTLV holding a string
 * String is not null terminated, length is the XTLV len.
 */
typedef struct xtlv_string {
	uint16 id;                  /* XTLV ID: EVENT_LOG_XTLV_ID_STR */
	uint16 len;                 /* XTLV Len (String length) */
	char   str[]; /* var len array characters */
} xtlv_string_t;

#define XTLV_STRING_FULL_LEN(str_len)     (BCM_XTLV_HDR_SIZE + (str_len) * sizeof(char))

/**
 * Summary for a single TxQ context
 * Two of these will be used per TxQ context---one for the high TxQ, and one for
 * the low txq that contains DMA prepared pkts. The high TxQ is a full multi-precidence
 * queue and also has a BSSCFG map to identify the BSSCFGS associated with the queue context.
 * The low txq counterpart does not populate the BSSCFG map.
 * The excursion queue will have no bsscfgs associated and is the first queue dumped.
 */
typedef struct txq_summary {
	uint16 id;                    /* XTLV ID: EVENT_LOG_XTLV_ID_TXQ_SUM */
	uint16 len;                   /* XTLV Len */
	uint32 bsscfg_map;            /* bitmap of bsscfg indexes associated with this queue */
	uint32 stopped;               /* flow control bitmap */
	uint8  prec_count;            /* count of precedences/fifos and len of following array */
	uint8  pad;
	uint16 plen[];  /* array of lengths of each prec/fifo in the queue */
} txq_summary_t;

#define TXQ_SUMMARY_LEN                   (OFFSETOF(txq_summary_t, plen))
#define TXQ_SUMMARY_FULL_LEN(num_q)       (TXQ_SUMMARY_LEN + (num_q) * sizeof(uint16))

typedef struct txq_summary_v2 {
	uint16 id;                    /* XTLV ID: EVENT_LOG_XTLV_ID_TXQ_SUM_V2 */
	uint16 len;                   /* XTLV Len */
	uint32 bsscfg_map;            /* bitmap of bsscfg indexes associated with this queue */
	uint32 stopped;               /* flow control bitmap */
	uint32 hw_stopped;            /* flow control bitmap */
	uint8  prec_count;            /* count of precedences/fifos and len of following array */
	uint8  pad;
	uint16 plen[];  /* array of lengths of each prec/fifo in the queue */
} txq_summary_v2_t;

#define TXQ_SUMMARY_V2_LEN                (OFFSETOF(txq_summary_v2_t, plen))
#define TXQ_SUMMARY_V2_FULL_LEN(num_q)    (TXQ_SUMMARY_V2_LEN + (num_q) * sizeof(uint16))

/**
 * Summary for tx datapath of an SCB cubby
 * This is a generic summary structure (one size fits all) with
 * a cubby ID and sub-ID to differentiate SCB cubby types and possible sub-queues.
 */
typedef struct scb_subq_summary {
	uint16 id;                    /* XTLV ID: EVENT_LOG_XTLV_ID_SCBDATA_SUM */
	uint16 len;                   /* XTLV Len */
	uint32 flags;                 /* cubby specficic flags */
	uint8  cubby_id;              /* ID registered for cubby */
	uint8  sub_id;                /* sub ID if a cubby has more than one queue */
	uint8  prec_count;            /* count of precedences/fifos and len of following array */
	uint8  pad;
	uint16 plen[];  /* array of lengths of each prec/fifo in the queue */
} scb_subq_summary_t;

#define SCB_SUBQ_SUMMARY_LEN              (OFFSETOF(scb_subq_summary_t, plen))
#define SCB_SUBQ_SUMMARY_FULL_LEN(num_q)  (SCB_SUBQ_SUMMARY_LEN + (num_q) * sizeof(uint16))

/* scb_subq_summary_t.flags for APPS */
#define SCBDATA_APPS_F_PS               0x00000001
#define SCBDATA_APPS_F_PSPEND           0x00000002
#define SCBDATA_APPS_F_INPVB            0x00000004
#define SCBDATA_APPS_F_APSD_USP         0x00000008
#define SCBDATA_APPS_F_TXBLOCK          0x00000010
#define SCBDATA_APPS_F_APSD_HPKT_TMR    0x00000020
#define SCBDATA_APPS_F_APSD_TX_PEND     0x00000040
#define SCBDATA_APPS_F_INTRANS          0x00000080
#define SCBDATA_APPS_F_OFF_PEND         0x00000100
#define SCBDATA_APPS_F_OFF_BLOCKED      0x00000200
#define SCBDATA_APPS_F_OFF_IN_PROG      0x00000400

/**
 * Summary for tx datapath AMPDU SCB cubby
 * This is a specific data structure to describe the AMPDU datapath state for an SCB
 * used instead of scb_subq_summary_t.
 * Info is for one TID, so one will be dumped per BA TID active for an SCB.
 */
typedef struct scb_ampdu_tx_summary {
	uint16 id;              /* XTLV ID: EVENT_LOG_XTLV_ID_SCBDATA_AMPDU_TX_SUM */
	uint16 len;             /* XTLV Len */
	uint32 flags;           /* misc flags */
	uint8  tid;             /* initiator TID (priority) */
	uint8  ba_state;        /* internal BA state */
	uint8  bar_cnt;         /* number of bars sent with no progress */
	uint8  retry_bar;       /* reason code if bar to be retried at watchdog */
	uint16 barpending_seq;  /* seqnum for bar */
	uint16 bar_ackpending_seq; /* seqnum of bar for which ack is pending */
	uint16 start_seq;       /* seqnum of the first unacknowledged packet */
	uint16 max_seq;         /* max unacknowledged seqnum sent */
	uint32 released_bytes_inflight; /* Number of bytes pending in bytes */
	uint32 released_bytes_target;
} scb_ampdu_tx_summary_t;

/* scb_ampdu_tx_summary.flags defs */
#define SCBDATA_AMPDU_TX_F_BAR_ACKPEND          0x00000001 /* bar_ackpending */

/** XTLV stuct to summarize a BSSCFG's packet queue */
typedef struct bsscfg_q_summary {
	uint16 id;                   /* XTLV ID: EVENT_LOG_XTLV_ID_BSSCFGDATA_SUM */
	uint16 len;                  /* XTLV Len */
	struct ether_addr BSSID;     /* BSSID */
	uint8  bsscfg_idx;           /* bsscfg index */
	uint8  type;                 /* bsscfg type enumeration: BSSCFG_TYPE_XXX */
	uint8  subtype;              /* bsscfg subtype enumeration: BSSCFG_SUBTYPE_XXX */
	uint8  prec_count;           /* count of precedences/fifos and len of following array */
	uint16 plen[]; /* array of lengths of each prec/fifo in the queue */
} bsscfg_q_summary_t;

#define BSSCFG_Q_SUMMARY_LEN              (OFFSETOF(bsscfg_q_summary_t, plen))
#define BSSCFG_Q_SUMMARY_FULL_LEN(num_q)  (BSSCFG_Q_SUMMARY_LEN + (num_q) * sizeof(uint16))

/**
 * An XTLV holding a TxStats array
 * TxStatus entries are 8 or 16 bytes, size in words (2 or 4) givent in
 * entry_size field.
 * Array is uint32 words
 */
typedef struct xtlv_uc_txs {
	uint16 id;                  /* XTLV ID: EVENT_LOG_XTLV_ID_UCTXSTATUS */
	uint16 len;                 /* XTLV Len */
	uint8  entry_size;          /* num uint32 words per entry */
	uint8  pad[3];              /* reserved, zero */
	uint32 w[];   /* var len array of words */
} xtlv_uc_txs_t;

#define XTLV_UCTXSTATUS_LEN                (OFFSETOF(xtlv_uc_txs_t, w))
#define XTLV_UCTXSTATUS_FULL_LEN(words)    (XTLV_UCTXSTATUS_LEN + (words) * sizeof(uint32))

#define SCAN_SUMMARY_VERSION_1	1u
#ifndef WLSCAN_SUMMARY_VERSION_ALIAS
#define SCAN_SUMMARY_VERSION	SCAN_SUMMARY_VERSION_1
#endif
/* Scan flags */
#define SCAN_SUM_CHAN_INFO	0x1
/* Scan_sum flags */
#define BAND5G_SIB_ENAB		0x2
#define BAND2G_SIB_ENAB		0x4
#define PARALLEL_SCAN		0x8
#define SCAN_ABORT		0x10
/* Note: Definitions  being reused in chan_info as SCAN_SUM_SCAN_CORE need clean up */
#define SC_LOWSPAN_SCAN		0x20
/* Note: Definitions  being reused in scan summary info as WL_SSUM_CLIENT_MASK need clean up */
#define SC_SCAN			0x40

#define WL_SSUM_CLIENT_MASK	0x1C0u	/* bit 8 - 6 */
#define WL_SSUM_CLIENT_SHIFT	6u	/* shift client scan opereration */

#define WL_SSUM_MODE_MASK	0xE00u	/* bit 11 - 9 */
#define WL_SSUM_MODE_SHIFT	9u	/* shift mode scan operation */

/* Common bits for channel and scan summary info */
#define SCAN_SUM_CHAN_RESHED	0x1000 /* Bit 12 as resched scan for chaninfo and scan summary */

#define WL_SSUM_CLIENT_ASSOCSCAN	0x0u	/* Log as scan requested client is assoc scan */
#define WL_SSUM_CLIENT_ROAMSCAN		0x1u	/* Log as scan requested client is roam scan */
#define WL_SSUM_CLIENT_FWSCAN		0x2u	/* Log as scan requested client is other fw scan */
#define WL_SSUM_CLIENT_HOSTSCAN		0x3u	/* Log as scan requested client is host scan */

#define WL_SSUM_SCANFLAG_INVALID	0x7u	/* Log for invalid scan client or mode */

/* scan_channel_info flags */
#define ACTIVE_SCAN_SCN_SUM	0x2
#define SCAN_SUM_WLC_CORE0	0x4
#define SCAN_SUM_WLC_CORE1	0x8
#define HOME_CHAN		0x10
#define SCAN_SUM_SCAN_CORE	0x20

typedef struct wl_scan_ssid_info
{
	uint8		ssid_len;	/* the length of SSID */
	uint8		ssid[32];	/* SSID string */
} wl_scan_ssid_info_t;

typedef struct wl_scan_channel_info {
	uint16 chanspec;	/* chanspec scanned */
	uint16 reserv;
	uint32 start_time;		/* Scan start time in
				* milliseconds for the chanspec
				* or home_dwell time start
				*/
	uint32 end_time;		/* Scan end time in
				* milliseconds for the chanspec
				* or home_dwell time end
				*/
	uint16 probe_count;	/* No of probes sent out. For future use
				*/
	uint16 scn_res_count;	/* Count of scan_results found per
				* channel. For future use
				*/
} wl_scan_channel_info_t;

typedef struct wl_scan_summary_info {
	uint32 total_chan_num;	/* Total number of channels scanned */
	uint32 scan_start_time;	/* Scan start time in milliseconds */
	uint32 scan_end_time;	/* Scan end time in milliseconds */
	wl_scan_ssid_info_t ssid[];	/* SSID being scanned in current
				* channel. For future use
				*/
} wl_scan_summary_info_t;

struct wl_scan_summary {
	uint8 version;		/* Version */
	uint8 reserved;
	uint16 len;		/* Length of the data buffer including SSID
				 * list.
				 */
	uint16 sync_id;		/* Scan Sync ID */
	uint16 scan_flags;		/* flags [0] or SCAN_SUM_CHAN_INFO = */
				/* channel_info, if not set */
				/* it is scan_summary_info */
				/* when channel_info is used, */
				/* the following flag bits are overridden: */
				/* flags[1] or ACTIVE_SCAN_SCN_SUM = active channel if set */
				/* passive if not set */
				/* flags[2] or WLC_CORE0 = if set, represents wlc_core0 */
				/* flags[3] or WLC_CORE1 = if set, represents wlc_core1 */
				/* flags[4] or HOME_CHAN = if set, represents home-channel */
				/* flags[5] or SCAN_SUM_SCAN_CORE = if set,
				 * represents chan_info from scan core.
				 */
				/* flags[12] SCAN_SUM_CHAN_RESHED indicate scan rescheduled */
				/* flags[6:11, 13:15] = reserved */
				/* when scan_summary_info is used, */
				/* the following flag bits are used: */
				/* flags[1] or BAND5G_SIB_ENAB = */
				/* allowSIBParallelPassiveScan on 5G band */
				/* flags[2] or BAND2G_SIB_ENAB = */
				/* allowSIBParallelPassiveScan on 2G band */
				/* flags[3] or PARALLEL_SCAN = Parallel scan enabled or not */
				/* flags[4] or SCAN_ABORT = SCAN_ABORTED scenario */
				/* flags[5] = reserved */
				/* flags[6:8] is used as count value to identify SCAN CLIENT
				 * WL_SSUM_CLIENT_ASSOCSCAN 0x0u, WL_SSUM_CLIENT_ROAMSCAN 0x1u,
				 * WL_SSUM_CLIENT_FWSCAN	0x2u, WL_SSUM_CLIENT_HOSTSCAN 0x3u
				 */
				/* flags[9:11] is used as count value to identify SCAN MODE
				 * WL_SCAN_MODE_HIGH_ACC 0u, WL_SCAN_MODE_LOW_SPAN 1u,
				 * WL_SCAN_MODE_LOW_POWER 2u
				 */
				/* flags[12] SCAN_SUM_CHAN_RESHED indicate scan rescheduled */
				/* flags[13:15] = reserved */
	union {
		wl_scan_channel_info_t scan_chan_info;	/* scan related information
							* for each channel scanned
							*/
		wl_scan_summary_info_t scan_sum_info;	/* Cumulative scan related
							* information.
							*/
	} u;
};

#define SCAN_SUMMARY_VERSION_2	2u
struct wl_scan_summary_v2 {
	uint8 version;		/* Version */
	uint8 reserved;
	uint16 len;		/* Length of the data buffer including SSID
				 * list.
				 */
	uint16 sync_id;		/* Scan Sync ID */
	uint16 scan_flags;		/* flags [0] or SCAN_SUM_CHAN_INFO = */
				/* channel_info, if not set */
				/* it is scan_summary_info */
				/* when channel_info is used, */
				/* the following flag bits are overridden: */
				/* flags[1] or ACTIVE_SCAN_SCN_SUM = active channel if set */
				/* passive if not set */
				/* flags[2] or WLC_CORE0 = if set, represents wlc_core0 */
				/* flags[3] or WLC_CORE1 = if set, represents wlc_core1 */
				/* flags[4] or HOME_CHAN = if set, represents home-channel */
				/* flags[5] or SCAN_SUM_SCAN_CORE = if set,
				 * represents chan_info from scan core.
				 */
				/* flags[12] SCAN_SUM_CHAN_RESHED indicate scan rescheduled */
				/* flags[6:11, 13:15] = reserved */
				/* when scan_summary_info is used, */
				/* the following flag bits are used: */
				/* flags[1] or BAND5G_SIB_ENAB = */
				/* allowSIBParallelPassiveScan on 5G band */
				/* flags[2] or BAND2G_SIB_ENAB = */
				/* allowSIBParallelPassiveScan on 2G band */
				/* flags[3] or PARALLEL_SCAN = Parallel scan enabled or not */
				/* flags[4] or SCAN_ABORT = SCAN_ABORTED scenario */
				/* flags[5] = reserved */
				/* flags[6:8] is used as count value to identify SCAN CLIENT
				 * WL_SSUM_CLIENT_ASSOCSCAN 0x0u, WL_SSUM_CLIENT_ROAMSCAN 0x1u,
				 * WL_SSUM_CLIENT_FWSCAN	0x2u, WL_SSUM_CLIENT_HOSTSCAN 0x3u
				 */
				/* flags[9:11] is used as count value to identify SCAN MODE
				 * WL_SCAN_MODE_HIGH_ACC 0u, WL_SCAN_MODE_LOW_SPAN 1u,
				 * WL_SCAN_MODE_LOW_POWER 2u
				 */
				/* flags[12] SCAN_SUM_CHAN_RESHED indicate scan rescheduled */
				/* flags[13:15] = reserved */
	/* scan_channel_ctx_t chan_cnt; */
	uint8 channel_cnt_aux;			/* Number of channels to be scanned on Aux core */
	uint8 channel_cnt_main;			/* Number of channels to be scanned on Main core */
	uint8 channel_cnt_sc;			/* Number of channels to be scanned on Scan core */
	uint8 active_channel_cnt;
	uint8 passive_channel_cnt;
	char pad[3];				/* Pad to keep it 32 bit aligned */
	union {
		wl_scan_channel_info_t scan_chan_info;	/* scan related information
							* for each channel scanned
							*/
		wl_scan_summary_info_t scan_sum_info;	/* Cumulative scan related
							* information.
							*/
	} u;
};

/* Raom target evaluation detail data type */
typedef enum {
	WL_ROAM_TRGT_EVAL_BASIC		= 1u,
	WL_ROAM_TRGT_EVAL_RSSI_SCORE	= 2u,
	WL_ROAM_TRGT_EVAL_LOAD_SCORE	= 3u,
	WL_ROAM_TRGT_EVAL_SUMMARY	= 4u,
	WL_ROAM_TRGT_EVAL_MAX
} wl_roam_trgt_eval_msg_type_t;

struct wl_roam_target_cur_state {
	uint32	type;		/* = WL_ROAM_TRGT_EVAL_BASIC (1) */
	uint32	roam_reason;	/* existing definition of roam reasons */
	uint32	roam_type;	/* 0:ROAM_FULL 1:ROAM_PARTIAL 2:ROAM_SPLIT_PHASE */
	uint32	home_bssid_hi;	/* 32-bit MSB MAC address */
	uint32	home_bssid_lo;	/* 16-bit LSB MAC address */
	uint32	channel;	/* chanspec_t */
	int32	rssi;		/* raw value */
	int32	roam_prof_idx;	/* Roam profile index */
	uint32	prof_flags;	/* Info related to the current applicable roam profile */
	int32	prof_trigger;
	uint32	prof_delta;
	uint32	user_ov;	/* User roam cache override mode */
};

struct wl_roam_target_rssi_score {
	uint32	type;		/* = WL_ROAM_TRGT_EVAL_RSSI_SCORE (2) */
	uint32	bssid_hi;	/* 32-bit MSB MAC address */
	uint32	bssid_lo;	/* 16-bit LSB MAC address */
	uint32	channel;	/* chanspec_t */
	int32	rssi;		/* raw value */
	int32	rssi_boosted;
	uint32	rssi_score;	/* = 65536 + MIN(0, rssi_boosted) */
	uint32	load_aac;	/* qbss load available admission capacity (when available) */
	uint32	chan_free;	/* IEEE802.11 format with 255 = 100% (when available) */
};

struct wl_roam_target_load_score {
	uint32	type;		/* = WL_ROAM_TRGT_EVAL_LOAD_SCORE (3) */
	uint32	bssid_hi;	/* 32-bit MSB MAC address */
	uint32	bssid_lo;	/* 16-bit LSB MAC address */
	uint32	channel;	/* chanspec_t */
	int32	rssi;		/* raw value */
	int32	rssi_boosted;
	uint32	rate_family;	/* two-letter for  A/ B/ G/ N/AC/AX/ */
	uint32	rate;		/* mapped rate (x500Kbps) */
	uint32	bw;		/* 20, 40, 80, 160, or ... */
	uint32	nss;
	uint32	chan_free;	/* IEEE802.11 format with 255 = 100% */
	uint32	load_score;	/* = rate * nss * bw * chan_free */
};

struct wl_roam_target_summary {
	uint32	type;		/* = WL_ROAM_TRGT_EVAL_SUMMARY (4) */
	uint32	ref_score;	/* score reference (minimum score to beat) */
	uint32	score_delta;	/* score delta - additional marging */
	uint32	num_of_targets;	/* number of qualifying target */
};

/* Channel switch log record structure
 * Host may map the following structure on channel switch event log record
 * received from dongle. Note that all payload entries in event log record are
 * uint32/int32.
 */
typedef struct wl_chansw_event_log_record {
	uint32 time;			/* Time in us */
	uint32 old_chanspec;		/* Old channel spec */
	uint32 new_chanspec;		/* New channel spec */
	uint32 chansw_reason;		/* Reason for channel change */
	int32 dwell_time;
} wl_chansw_event_log_record_t;

typedef struct wl_chansw_event_log_record_v2 {
	uint32 time;			/* Time in us */
	uint32 old_chanspec;		/* Old channel spec */
	uint32 new_chanspec;		/* New channel spec */
	uint32 chansw_reason;		/* Reason for channel change */
	int32 dwell_time;
	uint32 core;
	int32 phychanswtime;		/* channel switch time */
} wl_chansw_event_log_record_v2_t;

/* Sub-block type for EVENT_LOG_TAG_AMPDU_DUMP */
typedef enum {
	WL_AMPDU_STATS_TYPE_RXMCSx1		= 0,	/* RX MCS rate (Nss = 1) */
	WL_AMPDU_STATS_TYPE_RXMCSx2		= 1,
	WL_AMPDU_STATS_TYPE_RXMCSx3		= 2,
	WL_AMPDU_STATS_TYPE_RXMCSx4		= 3,
	WL_AMPDU_STATS_TYPE_RXVHTx1		= 4,	/* RX VHT rate (Nss = 1) */
	WL_AMPDU_STATS_TYPE_RXVHTx2		= 5,
	WL_AMPDU_STATS_TYPE_RXVHTx3		= 6,
	WL_AMPDU_STATS_TYPE_RXVHTx4		= 7,
	WL_AMPDU_STATS_TYPE_TXMCSx1		= 8,	/* TX MCS rate (Nss = 1) */
	WL_AMPDU_STATS_TYPE_TXMCSx2		= 9,
	WL_AMPDU_STATS_TYPE_TXMCSx3		= 10,
	WL_AMPDU_STATS_TYPE_TXMCSx4		= 11,
	WL_AMPDU_STATS_TYPE_TXVHTx1		= 12,	/* TX VHT rate (Nss = 1) */
	WL_AMPDU_STATS_TYPE_TXVHTx2		= 13,
	WL_AMPDU_STATS_TYPE_TXVHTx3		= 14,
	WL_AMPDU_STATS_TYPE_TXVHTx4		= 15,
	WL_AMPDU_STATS_TYPE_RXMCSSGI		= 16,	/* RX SGI usage (for all MCS rates) */
	WL_AMPDU_STATS_TYPE_TXMCSSGI		= 17,	/* TX SGI usage (for all MCS rates) */
	WL_AMPDU_STATS_TYPE_RXVHTSGI		= 18,	/* RX SGI usage (for all VHT rates) */
	WL_AMPDU_STATS_TYPE_TXVHTSGI		= 19,	/* TX SGI usage (for all VHT rates) */
	WL_AMPDU_STATS_TYPE_RXMCSPER		= 20,	/* RX PER (for all MCS rates) */
	WL_AMPDU_STATS_TYPE_TXMCSPER		= 21,	/* TX PER (for all MCS rates) */
	WL_AMPDU_STATS_TYPE_RXVHTPER		= 22,	/* RX PER (for all VHT rates) */
	WL_AMPDU_STATS_TYPE_TXVHTPER		= 23,	/* TX PER (for all VHT rates) */
	WL_AMPDU_STATS_TYPE_RXDENS		= 24,	/* RX AMPDU density */
	WL_AMPDU_STATS_TYPE_TXDENS		= 25,	/* TX AMPDU density */
	WL_AMPDU_STATS_TYPE_RXMCSOK		= 26,	/* RX all MCS rates */
	WL_AMPDU_STATS_TYPE_RXVHTOK		= 27,	/* RX all VHT rates */
	WL_AMPDU_STATS_TYPE_TXMCSALL		= 28,	/* TX all MCS rates */
	WL_AMPDU_STATS_TYPE_TXVHTALL		= 29,	/* TX all VHT rates */
	WL_AMPDU_STATS_TYPE_TXMCSOK		= 30,	/* TX all MCS rates */
	WL_AMPDU_STATS_TYPE_TXVHTOK		= 31,	/* TX all VHT rates */
	WL_AMPDU_STATS_TYPE_RX_HE_SUOK		= 32,	/* DL SU MPDU frame per MCS */
	WL_AMPDU_STATS_TYPE_RX_HE_SU_DENS	= 33,	/* DL SU AMPDU DENSITY */
	WL_AMPDU_STATS_TYPE_RX_HE_MUMIMOOK	= 34,	/* DL MUMIMO Frame per MCS */
	WL_AMPDU_STATS_TYPE_RX_HE_MUMIMO_DENS	= 35,	/* DL MUMIMO AMPDU Density */
	WL_AMPDU_STATS_TYPE_RX_HE_DLOFDMAOK	= 36,	/* DL OFDMA Frame per MCS */
	WL_AMPDU_STATS_TYPE_RX_HE_DLOFDMA_DENS	= 37,	/* DL OFDMA AMPDU Density */
	WL_AMPDU_STATS_TYPE_RX_HE_DLOFDMA_HIST	= 38,	/* DL OFDMA frame RU histogram */
	WL_AMPDU_STATS_TYPE_TX_HE_MCSALL	= 39,	/* TX HE (SU+MU) frames, all rates */
	WL_AMPDU_STATS_TYPE_TX_HE_MCSOK		= 40,	/* TX HE (SU+MU) frames succeeded */
	WL_AMPDU_STATS_TYPE_TX_HE_MUALL		= 41,	/* TX MU (UL OFDMA) frames all rates */
	WL_AMPDU_STATS_TYPE_TX_HE_MUOK		= 42,	/* TX MU (UL OFDMA) frames succeeded */
	WL_AMPDU_STATS_TYPE_TX_HE_RUBW		= 43,	/* TX UL RU by BW histogram */
	WL_AMPDU_STATS_TYPE_TX_HE_PADDING	= 44,	/* TX padding total (single value) */
	WL_AMPDU_STATS_TYPE_RX_COUNTERS		= 45,	/* Additional AMPDU_RX module counters
							 * per-slice
							 */
	WL_AMPDU_STATS_TYPE_RXHEx1		= 46,	/* RX HE rate (Nss = 1) */
	WL_AMPDU_STATS_TYPE_RXHEx2		= 47,
	WL_AMPDU_STATS_TYPE_RXHEx3		= 48,
	WL_AMPDU_STATS_TYPE_RXHEx4		= 49,
	WL_AMPDU_STATS_TYPE_TXHEx1		= 50,	/* TX HE rate (Nss = 1) */
	WL_AMPDU_STATS_TYPE_TXHEx2		= 51,
	WL_AMPDU_STATS_TYPE_TXHEx3		= 52,
	WL_AMPDU_STATS_TYPE_TXHEx4		= 53
} wl_ampdu_stat_enum_t;
#define	WL_AMPDU_STATS_MAX_CNTS	(64)	/* Possible max number of counters in any sub-categary */

typedef struct {
	uint16	type;		/* AMPDU statistics sub-type */
	uint16	len;		/* Number of 32-bit counters */
	uint32	counters[WL_AMPDU_STATS_MAX_CNTS];
} wl_ampdu_stats_generic_t;

typedef wl_ampdu_stats_generic_t wl_ampdu_stats_rx_t;
typedef wl_ampdu_stats_generic_t wl_ampdu_stats_tx_t;

typedef struct {
	uint16	type;		/* AMPDU statistics sub-type */
	uint16	len;		/* Number of 32-bit counters + 2 */
	uint32	total_ampdu;
	uint32	total_mpdu;
	uint32	aggr_dist[WL_AMPDU_STATS_MAX_CNTS + 1];
} wl_ampdu_stats_aggrsz_t;

/* AMPDU_RX module's per-slice counters. Sent by ecounters as subtype of
 * WL_IFSTATS_XTLV_RX_AMPDU_STATS ecounters type
 */
#define WLC_AMPDU_RX_STATS_V1	(1u)
typedef struct wlc_ampdu_rx_stats {
	uint16 version;
	uint16 len;
	/* responder side counters */
	uint32 rxampdu;		/**< ampdus recd */
	uint32 rxmpdu;		/**< mpdus recd in a ampdu */
	/* Using union to have a smooth transition for renaming rxht ro rxsingle for all branches */
	union {
	uint32 rxht;		/**< obsolete - mpdus recd at ht rate and not in a ampdu */
	uint32 rxsingle;	/**< mpdus recd not in a ampdu */
	};
	uint32 rxlegacy;	/**< mpdus recd at legacy rate */
	uint32 rxampdu_sgi;	/**< ampdus recd with sgi */
	uint32 rxampdu_stbc;    /**< ampdus recd with stbc */
	uint32 rxnobapol;	/**< mpdus recd without a ba policy */
	uint32 rxholes;		/**< missed seq numbers on rx side */
	uint32 rxqed;		/**< pdus buffered before sending up */
	uint32 rxdup;		/**< duplicate pdus */
	uint32 rxstuck;		/**< watchdog bailout for stuck state */
	uint32 rxoow;		/**< out of window pdus */
	uint32 rxoos;		/**< out of seq pdus */
	uint32 rxaddbareq;	/**< addba req recd */
	uint32 txaddbaresp;	/**< addba resp sent */
	uint32 rxbar;		/**< bar recd */
	uint32 txba;		/**< ba sent */

	/* general: both initiator and responder */
	uint32 rxunexp;		/**< unexpected packets */
	uint32 txdelba;		/**< delba sent */
	uint32 rxdelba;		/**< delba recd */

	uint32 rxretrynobapol;	/**< retried mpdus rxed without a ba policy */
} wlc_ampdu_rx_stats_t;

/* Sub-block type for WL_IFSTATS_XTLV_HE_TXMU_STATS */
typedef enum {
	/* Reserve 0 to avoid potential concerns */
	WL_HE_TXMU_STATS_TYPE_TIME		= 1,	/* per-dBm, total usecs transmitted */
	WL_HE_TXMU_STATS_TYPE_PAD_TIME		= 2,	/* per-dBm, padding usecs transmitted */
} wl_he_txmu_stat_enum_t;
#define WL_IFSTATS_HE_TXMU_MAX	32u

/* Sub-block type for EVENT_LOG_TAG_MSCHPROFILE */
#define WL_MSCH_PROFILER_START		0	/* start event check */
#define WL_MSCH_PROFILER_EXIT		1	/* exit event check */
#define WL_MSCH_PROFILER_REQ		2	/* request event */
#define WL_MSCH_PROFILER_CALLBACK	3	/* call back event */
#define WL_MSCH_PROFILER_MESSAGE	4	/* message event */
#define WL_MSCH_PROFILER_PROFILE_START	5
#define WL_MSCH_PROFILER_PROFILE_END	6
#define WL_MSCH_PROFILER_REQ_HANDLE	7
#define WL_MSCH_PROFILER_REQ_ENTITY	8
#define WL_MSCH_PROFILER_CHAN_CTXT	9
#define WL_MSCH_PROFILER_EVENT_LOG	10
#define WL_MSCH_PROFILER_REQ_TIMING	11
#define WL_MSCH_PROFILER_TYPE_MASK	0x00ff
#define WL_MSCH_PROFILER_WLINDEX_SHIFT	8
#define WL_MSCH_PROFILER_WLINDEX_MASK	0x0f00
#define WL_MSCH_PROFILER_VER_SHIFT	12
#define WL_MSCH_PROFILER_VER_MASK	0xf000

/* MSCH Event data current verion */
#define WL_MSCH_PROFILER_VER		2

/* msch version history */
#define WL_MSCH_PROFILER_RSDB_VER	1
#define WL_MSCH_PROFILER_REPORT_VER	2

/* msch collect header size */
#define WL_MSCH_PROFILE_HEAD_SIZE	OFFSETOF(msch_collect_tlv_t, value)

/* msch event log header size */
#define WL_MSCH_EVENT_LOG_HEAD_SIZE	OFFSETOF(msch_event_log_profiler_event_data_t, data)

/* MSCH data buffer size */
#define WL_MSCH_PROFILER_BUFFER_SIZE	512

/* request type used in wlc_msch_req_param_t struct */
#define WL_MSCH_RT_BOTH_FIXED	0	/* both start and end time is fixed */
#define WL_MSCH_RT_START_FLEX	1	/* start time is flexible and duration is fixed */
#define WL_MSCH_RT_DUR_FLEX	2	/* start time is fixed and end time is flexible */
#define WL_MSCH_RT_BOTH_FLEX	3	/* Both start and duration is flexible */

/* Flags used in wlc_msch_req_param_t struct */
#define WL_MSCH_REQ_FLAGS_CHAN_CONTIGUOUS  (1 << 0) /* Don't break up channels in chanspec_list */
#define WL_MSCH_REQ_FLAGS_MERGE_CONT_SLOTS (1 << 1)  /* No slot end if slots are continous */
#define WL_MSCH_REQ_FLAGS_PREMTABLE        (1 << 2) /* Req can be pre-empted by PREMT_CURTS req */
#define WL_MSCH_REQ_FLAGS_PREMT_CURTS      (1 << 3) /* Pre-empt request at the end of curts */
#define WL_MSCH_REQ_FLAGS_PREMT_IMMEDIATE  (1 << 4) /* Pre-empt cur_ts immediately */

/* Requested slot Callback states
 * req->pend_slot/cur_slot->flags
 */
#define WL_MSCH_RC_FLAGS_ONCHAN_FIRE		(1 << 0)
#define WL_MSCH_RC_FLAGS_START_FIRE_DONE	(1 << 1)
#define WL_MSCH_RC_FLAGS_END_FIRE_DONE		(1 << 2)
#define WL_MSCH_RC_FLAGS_ONFIRE_DONE		(1 << 3)
#define WL_MSCH_RC_FLAGS_SPLIT_SLOT_START	(1 << 4)
#define WL_MSCH_RC_FLAGS_SPLIT_SLOT_END		(1 << 5)
#define WL_MSCH_RC_FLAGS_PRE_ONFIRE_DONE	(1 << 6)

/* Request entity flags */
#define WL_MSCH_ENTITY_FLAG_MULTI_INSTANCE	(1 << 0)

/* Request Handle flags */
#define WL_MSCH_REQ_HDL_FLAGS_NEW_REQ		(1 << 0) /* req_start callback */

/* MSCH state flags (msch_info->flags) */
#define	WL_MSCH_STATE_IN_TIEMR_CTXT		0x1
#define WL_MSCH_STATE_SCHD_PENDING		0x2

/* MSCH callback type */
#define	WL_MSCH_CT_REQ_START		0x1
#define	WL_MSCH_CT_ON_CHAN		0x2
#define	WL_MSCH_CT_SLOT_START		0x4
#define	WL_MSCH_CT_SLOT_END		0x8
#define	WL_MSCH_CT_SLOT_SKIP		0x10
#define	WL_MSCH_CT_OFF_CHAN		0x20
#define WL_MSCH_CT_OFF_CHAN_DONE	0x40
#define	WL_MSCH_CT_REQ_END		0x80
#define	WL_MSCH_CT_PARTIAL		0x100
#define	WL_MSCH_CT_PRE_ONCHAN		0x200
#define	WL_MSCH_CT_PRE_REQ_START	0x400

/* MSCH command bits */
#define WL_MSCH_CMD_ENABLE_BIT		0x01
#define WL_MSCH_CMD_PROFILE_BIT		0x02
#define WL_MSCH_CMD_CALLBACK_BIT	0x04
#define WL_MSCH_CMD_REGISTER_BIT	0x08
#define WL_MSCH_CMD_ERROR_BIT		0x10
#define WL_MSCH_CMD_DEBUG_BIT		0x20
#define WL_MSCH_CMD_INFOM_BIT		0x40
#define WL_MSCH_CMD_TRACE_BIT		0x80
#define WL_MSCH_CMD_ALL_BITS		0xfe
#define WL_MSCH_CMD_SIZE_MASK		0x00ff0000
#define WL_MSCH_CMD_SIZE_SHIFT		16
#define WL_MSCH_CMD_VER_MASK		0xff000000
#define WL_MSCH_CMD_VER_SHIFT		24

/* maximum channels returned by the get valid channels iovar */
#define WL_MSCH_NUMCHANNELS		64

typedef struct msch_collect_tlv {
	uint16	type;
	uint16	size;
	char	value[];
} msch_collect_tlv_t;

typedef struct msch_profiler_event_data {
	uint32	time_lo;		/* Request time */
	uint32	time_hi;
} msch_profiler_event_data_t;

typedef struct msch_start_profiler_event_data {
	uint32	time_lo;		/* Request time */
	uint32	time_hi;
	uint32	status;
} msch_start_profiler_event_data_t;

typedef struct msch_message_profiler_event_data {
	uint32	time_lo;			/* Request time */
	uint32	time_hi;
	char	message[];	/* message */
} msch_message_profiler_event_data_t;

typedef struct msch_event_log_profiler_event_data {
	uint32	time_lo;		/* Request time */
	uint32	time_hi;
	event_log_hdr_t hdr;		/* event log header */
	uint32	data[9];		/* event data */
} msch_event_log_profiler_event_data_t;

typedef struct msch_req_param_profiler_event_data {
	uint16  flags;			/* Describe various request properties */
	uint8	req_type;		/* Describe start and end time flexiblilty */
	uint8	priority;		/* Define the request priority */
	uint32	start_time_l;		/* Requested start time offset in us unit */
	uint32	start_time_h;
	uint32	duration;		/* Requested duration in us unit */
	uint32	interval;		/* Requested periodic interval in us unit,
					 * 0 means non-periodic
					 */
	union {
		uint32	dur_flex;	/* MSCH_REG_DUR_FLEX, min_dur = duration - dur_flex */
		struct {
			uint32 min_dur;	/* min duration for traffic, maps to home_time */
			uint32 max_away_dur; /* max acceptable away dur, maps to home_away_time */
			uint32 hi_prio_time_l;
			uint32 hi_prio_time_h;
			uint32 hi_prio_interval; /* repeated high priority interval */
		} bf;
	} flex;
} msch_req_param_profiler_event_data_t;

typedef struct msch_req_timing_profiler_event_data {
	uint32 p_req_timing;
	uint32 p_prev;
	uint32 p_next;
	uint16 flags;
	uint16 timeslot_ptr;
	uint32 fire_time_l;
	uint32 fire_time_h;
	uint32 pre_start_time_l;
	uint32 pre_start_time_h;
	uint32 start_time_l;
	uint32 start_time_h;
	uint32 end_time_l;
	uint32 end_time_h;
	uint32 p_timeslot;
} msch_req_timing_profiler_event_data_t;

typedef struct msch_chan_ctxt_profiler_event_data {
	uint32 p_chan_ctxt;
	uint32 p_prev;
	uint32 p_next;
	uint16 chanspec;
	uint16 bf_sch_pending;
	uint32 bf_link_prev;
	uint32 bf_link_next;
	uint32 onchan_time_l;
	uint32 onchan_time_h;
	uint32 actual_onchan_dur_l;
	uint32 actual_onchan_dur_h;
	uint32 pend_onchan_dur_l;
	uint32 pend_onchan_dur_h;
	uint16 req_entity_list_cnt;
	uint16 req_entity_list_ptr;
	uint16 bf_entity_list_cnt;
	uint16 bf_entity_list_ptr;
	uint32 bf_skipped_count;
} msch_chan_ctxt_profiler_event_data_t;

typedef struct msch_req_entity_profiler_event_data {
	uint32 p_req_entity;
	uint32 req_hdl_link_prev;
	uint32 req_hdl_link_next;
	uint32 chan_ctxt_link_prev;
	uint32 chan_ctxt_link_next;
	uint32 rt_specific_link_prev;
	uint32 rt_specific_link_next;
	uint32 start_fixed_link_prev;
	uint32 start_fixed_link_next;
	uint32 both_flex_list_prev;
	uint32 both_flex_list_next;
	uint16 chanspec;
	uint16 priority;
	uint16 cur_slot_ptr;
	uint16 pend_slot_ptr;
	uint16 pad;
	uint16 chan_ctxt_ptr;
	uint32 p_chan_ctxt;
	uint32 p_req_hdl;
	uint32 bf_last_serv_time_l;
	uint32 bf_last_serv_time_h;
	uint16 onchan_chn_idx;
	uint16 cur_chn_idx;
	uint32 flags;
	uint32 actual_start_time_l;
	uint32 actual_start_time_h;
	uint32 curts_fire_time_l;
	uint32 curts_fire_time_h;
} msch_req_entity_profiler_event_data_t;

typedef struct msch_req_handle_profiler_event_data {
	uint32 p_req_handle;
	uint32 p_prev;
	uint32 p_next;
	uint32 cb_func;
	uint32 cb_ctxt;
	uint16 req_param_ptr;
	uint16 req_entity_list_cnt;
	uint16 req_entity_list_ptr;
	uint16 chan_cnt;
	uint32 flags;
	uint16 chanspec_list;
	uint16 chanspec_cnt;
	uint16 chan_idx;
	uint16 last_chan_idx;
	uint32 req_time_l;
	uint32 req_time_h;
} msch_req_handle_profiler_event_data_t;

typedef struct msch_profiler_profiler_event_data {
	uint32 time_lo;			/* Request time */
	uint32 time_hi;
	uint32 free_req_hdl_list;
	uint32 free_req_entity_list;
	uint32 free_chan_ctxt_list;
	uint32 free_chanspec_list;
	uint16 cur_msch_timeslot_ptr;
	uint16 next_timeslot_ptr;
	uint32 p_cur_msch_timeslot;
	uint32 p_next_timeslot;
	uint32 cur_armed_timeslot;
	uint32 flags;
	uint32 ts_id;
	uint32 service_interval;
	uint32 max_lo_prio_interval;
	uint16 flex_list_cnt;
	uint16 msch_chanspec_alloc_cnt;
	uint16 msch_req_entity_alloc_cnt;
	uint16 msch_req_hdl_alloc_cnt;
	uint16 msch_chan_ctxt_alloc_cnt;
	uint16 msch_timeslot_alloc_cnt;
	uint16 msch_req_hdl_list_cnt;
	uint16 msch_req_hdl_list_ptr;
	uint16 msch_chan_ctxt_list_cnt;
	uint16 msch_chan_ctxt_list_ptr;
	uint16 msch_req_timing_list_cnt;
	uint16 msch_req_timing_list_ptr;
	uint16 msch_start_fixed_list_cnt;
	uint16 msch_start_fixed_list_ptr;
	uint16 msch_both_flex_req_entity_list_cnt;
	uint16 msch_both_flex_req_entity_list_ptr;
	uint16 msch_start_flex_list_cnt;
	uint16 msch_start_flex_list_ptr;
	uint16 msch_both_flex_list_cnt;
	uint16 msch_both_flex_list_ptr;
	uint32 slotskip_flag;
} msch_profiler_profiler_event_data_t;

typedef struct msch_req_profiler_event_data {
	uint32 time_lo;			/* Request time */
	uint32 time_hi;
	uint16 chanspec_cnt;
	uint16 chanspec_ptr;
	uint16 req_param_ptr;
	uint16 pad;
} msch_req_profiler_event_data_t;

typedef struct msch_callback_profiler_event_data {
	uint32 time_lo;			/* Request time */
	uint32 time_hi;
	uint16 type;			/* callback type */
	uint16 chanspec;		/* actual chanspec, may different with requested one */
	uint32 start_time_l;		/* time slot start time low 32bit */
	uint32 start_time_h;		/* time slot start time high 32bit */
	uint32 end_time_l;		/* time slot end time low 32 bit */
	uint32 end_time_h;		/* time slot end time high 32 bit */
	uint32 timeslot_id;		/* unique time slot id */
	uint32 p_req_hdl;
	uint32 onchan_idx;		/* Current channel index */
	uint32 cur_chan_seq_start_time_l; /* start time of current sequence */
	uint32 cur_chan_seq_start_time_h;
} msch_callback_profiler_event_data_t;

typedef struct msch_timeslot_profiler_event_data {
	uint32 p_timeslot;
	uint32 timeslot_id;
	uint32 pre_start_time_l;
	uint32 pre_start_time_h;
	uint32 end_time_l;
	uint32 end_time_h;
	uint32 sch_dur_l;
	uint32 sch_dur_h;
	uint32 p_chan_ctxt;
	uint32 fire_time_l;
	uint32 fire_time_h;
	uint32 state;
} msch_timeslot_profiler_event_data_t;

typedef struct msch_register_params	{
	uint16 wlc_index;		/* Optional wlc index */
	uint16 flags;			/* Describe various request properties */
	uint32 req_type;		/* Describe start and end time flexiblilty */
	uint16 id;			/* register id */
	uint16 priority;		/* Define the request priority */
	uint32 start_time;		/* Requested start time offset in ms unit */
	uint32 duration;		/* Requested duration in ms unit */
	uint32 interval;		/* Requested periodic interval in ms unit,
					 * 0 means non-periodic
					 */
	uint32 dur_flex;		/* MSCH_REG_DUR_FLEX, min_dur = duration - dur_flex */
	uint32 min_dur;			/* min duration for traffic, maps to home_time */
	uint32 max_away_dur;		/* max acceptable away dur, maps to home_away_time */
	uint32 hi_prio_time;
	uint32 hi_prio_interval;	/* repeated high priority interval */
	uint32 chanspec_cnt;
	uint16 chanspec_list[WL_MSCH_NUMCHANNELS];
} msch_register_params_t;

typedef struct {
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				* Control Management (includes retransmissions)
				*/
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				* expecting a response
				*/
	uint32	rxstrt;		/**< number of received frames with a good PLCP */
	uint32  rxbadplcp;	/**< number of parity check of the PLCP header failed */
	uint32  rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32  rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	uint32  bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32  rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastmbss;	/**< number of received DATA frames with good FCS and matching RA */
	uint32	rxdtocast;	/**< number of received DATA frames (good FCS and no matching RA) */
	uint32  rxtoolate;	/**< receive too late */
	uint32  goodfcs;        /**< Good fcs counters  */
	uint32  rxf0ovfl;	/** < Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;	/** < Rx FIFO1 overflow counters information */
} phy_periodic_counters_v1_t;

typedef struct {
	uint32	txallfrm;		/**< total number of frames sent, incl. Data, ACK, RTS,
					* CTS, Control Management
					*  (includes retransmissions)
					*/
	uint32	rxrsptmout;		/**< number of response timeouts for transmitted frames
								* expecting a response
								*/
	uint32	rxstrt;				/**< number of received frames with a good PLCP */
	uint32  rxbadplcp;	/**< number of parity check of the PLCP header failed */
	uint32  rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32  rxnodelim;	/**< number of no valid delimiter detected by ampdu parser */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	uint32  bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32  rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32	rxanyerr;	/**< Any RX error that is not counted by other counters. */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastmbss;	/**< number of received DATA frames with good FCS and matching RA */
	uint32	rxdtocast;	/**< number of received DATA frames (good FCS and no matching RA) */
	uint32  rxtoolate;	/**< receive too late */
	uint32  goodfcs;	/**< Good fcs counters  */
	uint32  rxf0ovfl;	/** < Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;	/** < Rx FIFO1 overflow counters information */

	uint32 txctl;
	uint32 rxctl;
	uint32 txbar;
	uint32 rxbar;
	uint32 rxbeaconbss;

	uint32 txrts;
	uint32	txucast;	/**< number of unicast tx expecting response other than cts/cwcts */
	uint32	rxackucast;	/**< number of ucast ACKS received (good FCS) */
	uint32	txackfrm;	/**< number of ACK frames sent out */

	uint32	txrtsfrm;	/**< number of RTS sent out by the MAC */
	uint32	txrtsfail;	/**< number of rts transmission failure that reach retry limit */
	uint32	rxctsucast;	/**< number of unicast CTS addressed to the MAC (good FCS) */

	uint32 rxrtsucast;	/**< number of unicast RTS addressed to the MAC (good FCS) */
	uint32	txctsfrm;	/**< number of CTS sent out by the MAC */

	uint32	rxback;		/**< blockack rxcnt */
	uint32	txback;		/**< blockack txcnt */
	uint32	rxctlucast;	/**< number of received CNTRL frames
						* with good FCS and matching RA
						*/

	uint32 last_bcn_seq_num;	/**< * L_TSF and Seq # from the last beacon received */
	uint32 last_bcn_ltsf;		/**< * L_TSF and Seq # from the last beacon received */

	uint16 counter_noise_request;	/**< counters of requesting noise samples for noisecal> */
	uint16 counter_noise_crsbit;	/**< counters of noise mmt being interrupted
									* due to PHYCRS>
									*/
	uint16 counter_noise_apply;		/**< counts of applying noisecal result> */

	uint8 phylog_noise_mode;		/**< noise cal mode> */
	uint8 total_desense_on;			/**< total desense on flag> */
	uint8 initgain_desense;			/**< initgain desense when total desense is on> */
	uint8 crsmin_init;		/**< crsmin_init threshold when total desense is on> */
	uint8 lna1_gainlmt_desense;	/**< lna1_gain limit desense when applying desense> */
	uint8 clipgain_desense0;		/**< clipgain desense when applying desense > */

	uint32 desense_reason;			/**< desense paraemters to indicate reasons
									* for bphy and ofdm_desense>
									*/
	uint8 lte_ofdm_desense;			/**< ofdm desense dut to lte> */
	uint8 lte_bphy_desense;			/**< bphy desense due to lte> */

	uint8 hw_aci_status;			/**< HW ACI status flag> */
	uint8 aci_clipgain_desense0;		/**< clipgain desense due to aci> */
	uint8 lna1_tbl_desense;			/**< LNA1 table desense due to aci> */

	int8 crsmin_high;			/**< crsmin high when applying desense> */
	int8 weakest_rssi;			/**< weakest link RSSI> */
	int8 phylog_noise_pwr_array[8][2];	/**< noise buffer array> */
} phy_periodic_counters_v5_t;

typedef struct {

	/* RX error related */
	uint32	rxrsptmout;	/* number of response timeouts for transmitted frames
				* expecting a response
				*/
	uint32	rxbadplcp;	/* number of parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/* PHY was able to correlate the preamble but not the header */
	uint32	rxnodelim;	/* number of no valid delimiter detected by ampdu parser */
	uint32	bphy_badplcp;	/* number of bad PLCP reception on BPHY rate */
	uint32	bphy_rxcrsglitch;	/* PHY count of bphy glitches */
	uint32	rxbadfcs;	/* number of frames for which the CRC check failed in the MAC */
	uint32  rxtoolate;	/* receive too late */
	uint32  rxf0ovfl;	/* Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;	/* Rx FIFO1 overflow counters information */
	uint32	rxanyerr;	/* Any RX error that is not counted by other counters. */
	uint32	rxdropped;	/* Frame dropped */
	uint32	rxnobuf;	/* Rx error due to no buffer */
	uint32	rxrunt;		/* Runt frame counter */
	uint32	rxfrmtoolong;	/* Number of received frame that are too long */
	uint32	rxdrop20s;

	/* RX related */
	uint32	rxstrt;		/* number of received frames with a good PLCP */
	uint32	rxbeaconmbss;	/* beacons received from member of BSS */
	uint32	rxdtucastmbss;	/* number of received DATA frames with good FCS and matching RA */
	uint32	rxdtocast;	/* number of received DATA frames (good FCS and no matching RA) */
	uint32  goodfcs;        /* Good fcs counters  */
	uint32	rxctl;		/* Number of control frames */
	uint32	rxaction;	/* Number of action frames */
	uint32	rxback;		/* Number of block ack frames rcvd */
	uint32	rxctlucast;	/* Number of received unicast ctl frames */
	uint32	rxframe;	/* Number of received frames */

	/* TX related */
	uint32	txallfrm;	/* total number of frames sent, incl. Data, ACK, RTS, CTS,
				* Control Management (includes retransmissions)
				*/
	uint32	txmpdu;			/* Numer of transmitted mpdus */
	uint32	txackbackctsfrm;	/* Number of ACK + BACK + CTS */

	/* TX error related */
	uint32	txrtsfail;		/* RTS TX failure count */
	uint32	txphyerr;		/* PHY TX error count */

	uint16	nav_cntr_l;		/* The state of the NAV */
	uint16	nav_cntr_h;
} phy_periodic_counters_v3_t;

typedef struct phy_periodic_counters_v4 {
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				* Control Management (includes retransmissions)
				*/
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				* expecting a response
				*/
	uint32	rxstrt;		/**< number of received frames with a good PLCP */
	uint32  rxbadplcp;	/**< number of parity check of the PLCP header failed */
	uint32  rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	uint32  bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastmbss;	/**< number of received DATA frames with good FCS and matching RA */
	uint32  rxf0ovfl;	/** < Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;	/** < Rx FIFO1 overflow counters information */
	uint32	rxdtocast;	/**< number of received DATA frames (good FCS and no matching RA) */
	uint32  rxtoolate;	/**< receive too late */
	uint32  rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
	uint32  rxdropped;
	uint32  rxcrc;
	uint32  rxnobuf;
	uint32  rxrunt;
	uint32  rxgiant;
	uint32  rxctl;
	uint32  rxaction;
	uint32  rxdrop20s;
	uint32  rxctsucast;
	uint32  rxrtsucast;
	uint32  txctsfrm;
	uint32  rxackucast;
	uint32  rxback;
	uint32  txphyerr;
	uint32  txrtsfrm;
	uint32  txackfrm;
	uint32  txback;
	uint32	rxnodelim;
	uint32	rxfrmtoolong;
	uint32	rxctlucast;
	uint32	txbcnfrm;
	uint32	txdnlfrm;
	uint32	txampdu;
	uint32	txmpdu;
	uint32  txinrtstxop;
	uint32	prs_timeout;
} phy_periodic_counters_v4_t;

/* phy_periodic_counters_v5_t  reserved for 4378 */

typedef struct phy_periodic_counters_v6 {
	/* RX error related */
	uint32	rxrsptmout;	/* number of response timeouts for transmitted frames
				* expecting a response
				*/
	uint32	rxbadplcp;	/* number of parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/* PHY was able to correlate the preamble but not the header */
	uint32	rxnodelim;	/* number of no valid delimiter detected by ampdu parser */
	uint32	bphy_badplcp;	/* number of bad PLCP reception on BPHY rate */
	uint32	bphy_rxcrsglitch;	/* PHY count of bphy glitches */
	uint32	rxbadfcs;	/* number of frames for which the CRC check failed in the MAC */
	uint32  rxtoolate;	/* receive too late */
	uint32  rxf0ovfl;	/* Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;	/* Rx FIFO1 overflow counters information */
	uint32	rxanyerr;	/* Any RX error that is not counted by other counters. */
	uint32	rxdropped;	/* Frame dropped */
	uint32	rxnobuf;	/* Rx error due to no buffer */
	uint32	rxrunt;		/* Runt frame counter */
	uint32	rxfrmtoolong;	/* Number of received frame that are too long */
	uint32	rxdrop20s;

	/* RX related */
	uint32	rxstrt;		/* number of received frames with a good PLCP */
	uint32	rxbeaconmbss;	/* beacons received from member of BSS */
	uint32	rxdtucastmbss;	/* number of received DATA frames with good FCS and matching RA */
	uint32	rxdtocast;	/* number of received DATA frames (good FCS and no matching RA) */
	uint32  goodfcs;        /* Good fcs counters  */
	uint32	rxctl;		/* Number of control frames */
	uint32	rxaction;	/* Number of action frames */
	uint32	rxback;		/* Number of block ack frames rcvd */
	uint32	rxctlucast;	/* Number of received unicast ctl frames */
	uint32	rxframe;	/* Number of received frames */

	uint32	rxbar;		/* Number of block ack requests rcvd */
	uint32	rxackucast;	/* number of ucast ACKS received (good FCS) */
	uint32	rxbeaconobss;	/* number of OBSS beacons received */
	uint32	rxctsucast;	/* number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxrtsucast;	/* number of unicast RTS addressed to the MAC (good FCS) */

	/* TX related */
	uint32	txallfrm;	/* total number of frames sent, incl. Data, ACK, RTS, CTS,
				* Control Management (includes retransmissions)
				*/
	uint32	txmpdu;			/* Numer of transmitted mpdus */
	uint32	txackbackctsfrm;	/* Number of ACK + BACK + CTS */
	uint32	txackfrm;	/* number of ACK frames sent out */
	uint32	txrtsfrm;	/* number of RTS sent out by the MAC */
	uint32	txctsfrm;	/* number of CTS sent out by the MAC */

	uint32	txctl;		/* Number of control frames txd */
	uint32	txbar;		/* Number of block ack requests txd */
	uint32	txrts;		/* Number of RTS txd */
	uint32	txback;		/* Number of block ack frames txd */
	uint32	txucast;	/* number of unicast tx expecting response other than cts/cwcts */

	/* TX error related */
	uint32	txrtsfail;	/* RTS TX failure count */
	uint32	txphyerr;	/* PHY TX error count */

	uint32	last_bcn_seq_num;	/* last beacon seq no. */
	uint32	last_bcn_ltsf;		/* last beacon ltsf */

	uint16	nav_cntr_l;		/* The state of the NAV */
	uint16	nav_cntr_h;
} phy_periodic_counters_v6_t;

typedef struct {
	uint32	txallfrm;		/* total number of frames sent, incl. Data, ACK, RTS,
					* CTS, Control Management
					*  (includes retransmissions)
					*/
	uint32	rxrsptmout;		/* number of response timeouts for transmitted frames
					* expecting a response
					*/
	uint32	rxstrt;			/* number of received frames with a good PLCP */
	uint32	rxcrsglitch;		/* PHY was able to correlate the preamble but not
					* the header
					*/
	uint32	bphy_badplcp;		/* number of bad PLCP reception on BPHY rate */
	uint32	bphy_rxcrsglitch;	/* PHY count of bphy glitches */
	uint32	rxbadfcs;		/* number of frames for which the CRC check failed
					* in the MAC
					*/
	uint32	rxbeaconmbss;		/* beacons received from member of BSS */
	uint32	rxdtucastmbss;		/* number of received DATA frames with good FCS and
					* matching RA
					*/
	uint32	rxdtocast;		/* number of received DATA frames
					* (good FCS and no matching RA)
					*/
	uint32	goodfcs;		/* Good fcs counters  */

	uint32	txctl;
	uint32	rxctl;
	uint32	txbar;
	uint32	rxbar;
	uint32	rxbeaconobss;

	uint32	txrts;
	uint32	txucast;		/* number of unicast tx expecting response
					* other than cts/cwcts
					*/
	uint32	rxackucast;		/* number of ucast ACKS received (good FCS) */
	uint32	txackfrm;		/* number of ACK frames sent out */

	uint32	txrtsfrm;		/* number of RTS sent out by the MAC */
	uint32	txrtsfail;		/* number of rts transmission failure
					* that reach retry limit
					*/
	uint32	rxctsucast;		/* number of unicast CTS addressed to the MAC (good FCS) */

	uint32	rxrtsucast;		/* number of unicast RTS addressed to the MAC (good FCS) */
	uint32	txctsfrm;		/* number of CTS sent out by the MAC */

	uint32	rxback;			/* blockack rxcnt */
	uint32	txback;			/* blockack txcnt */
	uint32	rxctlucast;		/* number of received CNTRL frames
					* with good FCS and matching RA
					*/

	uint32	last_bcn_seq_num;	/* L_TSF and Seq # from the last beacon received */
	uint32	last_bcn_ltsf;		/* L_TSF and Seq # from the last beacon received */

	uint32	desense_reason;		/* desense paraemters to indicate reasons
					* for bphy and ofdm_desense
					*/

	/* RX error related */
	uint32	rxdropped;		/* Frame dropped */
	uint32	rxnobuf;		/* Rx error due to no buffer */
	uint32	rxrunt;			/* Runt frame counter */

	uint32	rxframe;		/* Number of received frames */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint32	debug_01;
	uint32	debug_02;
	uint32	debug_03;
	uint32	debug_04;
	uint32	debug_05;

	uint32	rxanyerr;		/* Any RX error that is not counted by other counters. */

	uint32	phyovfl_cnt;		/* RX PHY FIFO overflow */
	uint32  rxf0ovfl;		/* Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;		/* Rx FIFO1 overflow counters information */

	uint32	lenfovfl_cnt;		/* RX LFIFO overflow */
	uint32	weppeof_cnt;		/* WEP asserted premature end-of-frame */
	uint32	rxbadplcp;		/* number of parity check of the PLCP header failed */
	uint32	strmeof_cnt;		/* RX frame got aborted because PHY FIFO did not have
					* sufficient bytes
					*/
	uint32	pfifo_drop;		/* PHY FIFO was not empty when a new frame arrived */
	uint32	ctx_fifo_full;		/* Low Priority Context FIFO is full */
	uint32	ctx_fifo2_full;		/* High Priority Context FIFO is full */
	uint32	rxnodelim;		/* number of no valid delimiter detected by ampdu parser */
	uint32	rx20s_cnt;		/* secondary 20 counter */
	uint32	rxdrop20s;		/* RX was discarded as the CRS was not seen on
					* primary channel
					*/
	uint32	new_rxin_plcp_wait_cnt;	/* A new reception started waiting for PLCP bytes
						* from a previous receive
						*/
	uint32  rxtoolate;		/* receive too late */
	uint32	laterx_cnt;		/* RX frame dropped as it was seen too (30us) late
					* from the start of reception
					*/
	uint32	rxfrmtoolong;		/* Number of received frame that are too long */
	uint32	rxfrmtooshrt;		/* RX frame was dropped as it did not meet minimum
					* number of bytes to be a valid 802.11 frame
					*/

	uint32	rxlegacyfrminvalid;	/* Invalid BPHY or L-OFDM reception */
	uint32	txsifserr;		/* A frame arrived in SIFS while we were about to
					* transmit B/ACK
					*/
	uint32	ooseq_macsusp;		/* Ucode is out of sequence in processing reception
					* (especially due to macsuspend).
					* RX MEND is seen without RX STRT
					*/

	uint16	counter_noise_request;	/* counters of requesting noise samples for noisecal */
	uint16	counter_noise_crsbit;	/* counters of noise mmt being interrupted
					* due to PHYCRS>
					*/
	uint16	counter_noise_apply;	/* counts of applying noisecal result */

	uint16	deaf_count;		/* Depth of stay_in_carrier_search function */

	uint8	phylog_noise_mode;	/* noise cal mode */
	uint8	total_desense_on;	/* total desense on flag */
	uint8	initgain_desense;	/* initgain desense when total desense is on */
	uint8	crsmin_init;		/* crsmin_init threshold when total desense is on */
	uint8	lna1_gainlmt_desense;	/* lna1_gain limit desense when applying desense */
	uint8	clipgain_desense0;	/* clipgain desense when applying desense */

	uint8	lte_ofdm_desense;	/* ofdm desense dut to lte */
	uint8	lte_bphy_desense;	/* bphy desense due to lte */

	uint8	hw_aci_status;		/* HW ACI status flag */
	uint8	aci_clipgain_desense0;	/* clipgain desense due to aci */
	uint8	lna1_tbl_desense;	/* LNA1 table desense due to aci */

	int8	crsmin_high;		/* crsmin high when applying desense */
	int8	weakest_rssi;		/* weakest link RSSI */

	uint8	max_fp;
	uint8	crsminpwr_initgain;
	uint8	crsminpwr_clip1_high;
	uint8	crsminpwr_clip1_med;
	uint8	crsminpwr_clip1_lo;
	uint8	pad1;
	uint8	pad2;
} phy_periodic_counters_v7_t;

typedef struct phy_periodic_counters_v8 {
	/* RX error related */
	uint32	rxrsptmout;		/* number of response timeouts for transmitted frames
					* expecting a response
					*/
	uint32	rxcrsglitch;		/* PHY was able to correlate the preamble
					* but not the header
					*/
	uint32	bphy_badplcp;		/* number of bad PLCP reception on BPHY rate */
	uint32	bphy_rxcrsglitch;	/* PHY count of bphy glitches */
	uint32	rxdropped;		/* Frame dropped */
	uint32	rxnobuf;		/* Rx error due to no buffer */
	uint32	rxrunt;			/* Runt frame counter */
	uint32	rxbadfcs;		/* number of frames for which the CRC check failed
					* in the MAC
					*/

	/* RX related */
	uint32	rxstrt;			/* number of received frames with a good PLCP */
	uint32	rxbeaconmbss;		/* beacons received from member of BSS */
	uint32	rxdtucastmbss;		/* number of received DATA frames with good FCS
					* and matching RA
					*/
	uint32	rxdtocast;		/* number of received DATA frames
					* (good FCS and no matching RA)
					*/
	uint32  goodfcs;		/* Good fcs counters  */
	uint32	rxctl;			/* Number of control frames */
	uint32	rxaction;		/* Number of action frames */
	uint32	rxback;			/* Number of block ack frames rcvd */
	uint32	rxctlucast;		/* Number of received unicast ctl frames */
	uint32	rxframe;		/* Number of received frames */

	uint32	rxbar;			/* Number of block ack requests rcvd */
	uint32	rxackucast;		/* number of ucast ACKS received (good FCS) */
	uint32	rxbeaconobss;		/* number of OBSS beacons received */
	uint32	rxctsucast;		/* number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxrtsucast;		/* number of unicast RTS addressed to the MAC (good FCS) */

	/* TX related */
	uint32	txallfrm;		/* total number of frames sent, incl. Data, ACK, RTS, CTS,
					* Control Management (includes retransmissions)
					*/
	uint32	txmpdu;			/* Numer of transmitted mpdus */
	uint32	txackbackctsfrm;	/* Number of ACK + BACK + CTS */
	uint32	txackfrm;		/* number of ACK frames sent out */
	uint32	txrtsfrm;		/* number of RTS sent out by the MAC */
	uint32	txctsfrm;		/* number of CTS sent out by the MAC */

	uint32	txctl;			/* Number of control frames txd */
	uint32	txbar;			/* Number of block ack requests txd */
	uint32	txrts;			/* Number of RTS txd */
	uint32	txback;			/* Number of block ack frames txd */
	uint32	txucast;		/* number of unicast tx expecting response
					* other than cts/cwcts
					*/

	/* TX error related */
	uint32	txrtsfail;		/* RTS TX failure count */
	uint32	txphyerr;		/* PHY TX error count */

	uint32	last_bcn_seq_num;	/* last beacon seq no. */
	uint32	last_bcn_ltsf;		/* last beacon ltsf */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint32	debug_01;
	uint32  debug_02;
	uint32  debug_03;
	uint32  debug_04;
	uint32  debug_05;

	uint32	rxanyerr;		/* Any RX error that is not counted by other counters. */

	uint32	phyovfl_cnt;		/* RX PHY FIFO overflow */
	uint32  rxf0ovfl;		/* Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;		/* Rx FIFO1 overflow counters information */

	uint32	lenfovfl_cnt;		/* RX LFIFO overflow */
	uint32	weppeof_cnt;		/* WEP asserted premature end-of-frame */
	uint32	rxbadplcp;		/* number of parity check of the PLCP header failed */
	uint32	strmeof_cnt;		/* RX frame got aborted because PHY FIFO did not have
					* sufficient bytes
					*/
	uint32	pfifo_drop;		/* PHY FIFO was not empty when a new frame arrived */
	uint32	ctx_fifo_full;		/* Low Priority Context FIFO is full */
	uint32	ctx_fifo2_full;		/* High Priority Context FIFO is full */
	uint32	rxnodelim;		/* number of no valid delimiter detected by ampdu parser */
	uint32	rx20s_cnt;		/* secondary 20 counter */
	uint32	rxdrop20s;		/* RX was discarded as the CRS was not seen on
					* primary channel
					*/
	uint32	new_rxin_plcp_wait_cnt;	/* A new reception started waiting for PLCP bytes
					* from a previous receive
					*/
	uint32  rxtoolate;		/* receive too late */
	uint32	laterx_cnt;		/* RX frame dropped as it was seen too (30us) late
					* from the start of reception
					*/
	uint32	rxfrmtoolong;		/* Number of received frame that are too long */
	uint32	rxfrmtooshrt;		/* RX frame was dropped as it did not meet minimum
					* number of bytes to be a valid 802.11 frame
					*/

	uint32	rxlegacyfrminvalid;	/* Invalid BPHY or L-OFDM reception */
	uint32	txsifserr;		/* A frame arrived in SIFS while we were about to
					* transmit B/ACK
					*/
	uint32	ooseq_macsusp;		/* Ucode is out of sequence in processing reception
					* (especially due to macsuspend).
					* RX MEND is seen without RX STRT
					*/
	uint32	desense_reason;		/* desense paraemters to indicate reasons
					* for bphy and ofdm_desense
					*/

	uint16	nav_cntr_l;		/* The state of the NAV */
	uint16	nav_cntr_h;
} phy_periodic_counters_v8_t;

typedef struct phy_periodic_counters_v255 {
	/* RX error related */
	uint32 rxrsptmout;          /* number of response timeouts for transmitted frames
					 * expecting a response
					 */
	uint32 rxcrsglitch;         /* PHY was able to correlate the preamble
					 * but not the header
					 */
	uint32 bphy_badplcp;        /* number of bad PLCP reception on BPHY rate */
	uint32 bphy_rxcrsglitch;    /* PHY count of bphy glitches */
	uint32 rxdropped;           /* Frame dropped */
	uint32 rxnobuf;             /* Rx error due to no buffer */
	uint32 rxrunt;              /* Runt frame counter */
	uint32 rxbadfcs;            /* number of frames for which the CRC check failed
					 * in the MAC
					 */

	/* RX related */
	uint32 rxstrt;              /* number of received frames with a good PLCP */
	uint32 rxbeaconmbss;        /* beacons received from member of BSS */
	uint32 rxdtucastmbss;       /* number of received DATA frames with good FCS
					 * and matching RA
					 */
	uint32 rxdtucastobss;       /* number of received DATA frames with good FCS
					 * but no-matching RA
					 */
	uint32 rxdtocast;           /* number of received DATA frames
					 * (good FCS and no matching RA)
					 */
	uint32 rxctl;               /* Number of control frames */
	uint32 rxaction;            /* Number of action frames */
	uint32 rxback;                     /* Number of block ack frames rcvd */
	uint32 rxctlucast;          /* Number of received unicast ctl frames */
	uint32 rxframe;             /* Number of received frames */

	uint32 rxbar;               /* Number of block ack requests rcvd */
	uint32 rxackucast;          /* number of ucast ACKS received (good FCS) */
	uint32 rxbeaconobss;        /* number of OBSS beacons received */
	uint32 rxctsucast;          /* number of unicast CTS addressed to the MAC (good FCS) */
	uint32 rxrtsucast;          /* number of unicast RTS addressed to the MAC (good FCS) */

	/* TX related */
	uint32 txallfrm;            /* total number of frames sent, incl. Data, ACK, RTS, CTS,
					 * Control Management (includes retransmissions)
					 */
	uint32 txbcnfrm;            /* Numer of transmitted beacons */
	uint32 txmpdu;              /* Numer of transmitted mpdus */
	uint32 txampdu;             /* Numer of transmitted ampdus */
	uint32 txackfrm;            /* number of ACK frames sent out */
	uint32 txrtsfrm;            /* number of RTS sent out by the MAC */
	uint32 txctsfrm;            /* number of CTS sent out by the MAC */

	uint32 txctl;               /* Number of control frames txd */
	uint32 txbar;               /* Number of block ack requests txd */
	uint32 txrts;               /* Number of RTS txd */
	uint32 txback;                     /* Number of block ack frames txd */
	uint32 txucast;             /* number of unicast tx expecting response
					 * other than cts/cwcts
					 */

	/* TX error */
	uint32 txrtsfail;           /* RTS TX failure count */
	uint32 txphyerr;            /* PHY TX error count */

	uint32 last_bcn_seq_num;    /* last beacon seq no. */
	uint32 last_bcn_ltsf;              /* last beacon ltsf */

	uint32 rxanyerr;            /* Any RX error that is not counted by other counters. */

	uint32  rxf0ovfl;           /* Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;           /* Rx FIFO1 overflow counters information */
	uint32  rxhlovfl;           /* Rx LFIFO overflow counters information */

	uint32 rxbadplcp;           /* number of parity check of the PLCP header failed */
	uint32 pfifo_drop;          /* PHY FIFO was not empty when a new frame arrived */
	uint32 rxnodelim;           /* number of no valid delimiter detected by ampdu parser */
	uint32 rxdrop20s;           /* RX was discarded as the CRS was not seen on
					 * primary channel
					 */
	uint32 new_rxin_plcp_wait_cnt;     /* A new reception started waiting for PLCP bytes
						 * from a previous receive
						 */
	uint32  rxtoolate;          /* receive too late */
	uint32 laterx_cnt;          /* RX frame dropped as it was seen too (30us) late
					 * from the start ofreception
					 */
	uint32 rxfrmtoolong;        /* Number of received frame that are too long */
	uint32 rxfrmtooshrt;        /* RX frame was dropped as it did not meet minimum
					 * number of bytes to be a valid 802.11 frame
					 */

	uint32 rxlegacyfrminvalid;  /* Invalid BPHY or L-OFDM reception */
	uint32 txsifserr;           /* A frame arrived in SIFS while we were about to
					 * transmit
					 * B/ACK
					 */
	uint32 ooseq_macsusp;       /* Ucode is out of sequence in processing reception
					 * (especially due to macsuspend).
					 * RX MEND is seen without RX STRT
					 */
	uint32 desense_reason;      /* desense paraemters to indicate reasons
					 * for bphy and ofdm_desense
					 */

} phy_periodic_counters_v255_t;

typedef struct phycal_log_cmn {
	uint16 chanspec; /* Current phy chanspec */
	uint8  last_cal_reason;  /* Last Cal Reason */
	uint8  pad1;  /* Padding byte to align with word */
	uint32  last_cal_time; /* Last cal time in sec */
} phycal_log_cmn_t;

typedef struct phycal_log_cmn_v2 {
	uint16 chanspec;	/* current phy chanspec */
	uint8  reason;		/* cal reason */
	uint8  phase;		/* cal phase */
	uint32 time;		/* time at which cal happened in sec */
	uint16 temp;		/* temperature at the time of cal */
	uint16 dur;		/* duration of cal in usec */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16 debug_01;	/* reused for slice info */
	uint16 debug_02;
	uint16 debug_03;
	uint16 debug_04;
} phycal_log_cmn_v2_t;

typedef struct phycal_log_core {
	uint16 ofdm_txa; /* OFDM Tx IQ Cal a coeff */
	uint16 ofdm_txb; /* OFDM Tx IQ Cal b coeff */
	uint16 ofdm_txd; /* contain di & dq */
	uint16 bphy_txa; /* BPHY Tx IQ Cal a coeff */
	uint16 bphy_txb; /* BPHY Tx IQ Cal b coeff */
	uint16 bphy_txd; /* contain di & dq */

	uint16 rxa; /* Rx IQ Cal A coeffecient */
	uint16 rxb; /* Rx IQ Cal B coeffecient */
	int32 rxs;  /* FDIQ Slope coeffecient */

	uint8 baseidx; /* TPC Base index */
	uint8 adc_coeff_cap0_adcI; /* ADC CAP Cal Cap0 I */
	uint8 adc_coeff_cap1_adcI; /* ADC CAP Cal Cap1 I */
	uint8 adc_coeff_cap2_adcI; /* ADC CAP Cal Cap2 I */
	uint8 adc_coeff_cap0_adcQ; /* ADC CAP Cal Cap0 Q */
	uint8 adc_coeff_cap1_adcQ; /* ADC CAP Cal Cap1 Q */
	uint8 adc_coeff_cap2_adcQ; /* ADC CAP Cal Cap2 Q */
	uint8 pad; /* Padding byte to align with word */
} phycal_log_core_t;

typedef struct phycal_log_core_v3 {
	uint16 ofdm_txa; /* OFDM Tx IQ Cal a coeff */
	uint16 ofdm_txb; /* OFDM Tx IQ Cal b coeff */
	uint16 ofdm_txd; /* contain di & dq */
	uint16 bphy_txa; /* BPHY Tx IQ Cal a coeff */
	uint16 bphy_txb; /* BPHY Tx IQ Cal b coeff */
	uint16 bphy_txd; /* contain di & dq */

	uint16 rxa; /* Rx IQ Cal A coeffecient */
	uint16 rxb; /* Rx IQ Cal B coeffecient */
	int32 rxs;  /* FDIQ Slope coeffecient */

	uint8 baseidx; /* TPC Base index */
	uint8 adc_coeff_cap0_adcI; /* ADC CAP Cal Cap0 I */
	uint8 adc_coeff_cap1_adcI; /* ADC CAP Cal Cap1 I */
	uint8 adc_coeff_cap2_adcI; /* ADC CAP Cal Cap2 I */
	uint8 adc_coeff_cap0_adcQ; /* ADC CAP Cal Cap0 Q */
	uint8 adc_coeff_cap1_adcQ; /* ADC CAP Cal Cap1 Q */
	uint8 adc_coeff_cap2_adcQ; /* ADC CAP Cal Cap2 Q */
	uint8 pad; /* Padding byte to align with word */

	/* Gain index based txiq ceffiecients for 2G(3 gain indices) */
	uint16 txiqlo_2g_a0; /* 2G TXIQ Cal a coeff for high TX gain */
	uint16 txiqlo_2g_b0; /* 2G TXIQ Cal b coeff for high TX gain */
	uint16 txiqlo_2g_a1; /* 2G TXIQ Cal a coeff for mid TX gain */
	uint16 txiqlo_2g_b1; /* 2G TXIQ Cal b coeff for mid TX gain */
	uint16 txiqlo_2g_a2; /* 2G TXIQ Cal a coeff for low TX gain */
	uint16 txiqlo_2g_b2; /* 2G TXIQ Cal b coeff for low TX gain */

	uint16	rxa_vpoff; /* Rx IQ Cal A coeff Vp off */
	uint16	rxb_vpoff; /* Rx IQ Cal B coeff Vp off */
	uint16	rxa_ipoff; /* Rx IQ Cal A coeff Ip off */
	uint16	rxb_ipoff; /* Rx IQ Cal B coeff Ip off */
	int32	rxs_vpoff; /* FDIQ Slope coeff Vp off */
	int32	rxs_ipoff; /* FDIQ Slope coeff Ip off */
} phycal_log_core_v3_t;

#define PHYCAL_LOG_VER1         (1u)

typedef struct phycal_log_v1 {
	uint8  version; /* Logging structure version */
	uint8  numcores; /* Numbe of cores for which core specific data present */
	uint16 length;  /* Length of the entire structure */
	phycal_log_cmn_t phycal_log_cmn; /* Logging common structure */
	/* This will be a variable length based on the numcores field defined above */
	phycal_log_core_t phycal_log_core[];
} phycal_log_v1_t;

typedef struct phy_periodic_log_cmn {
	uint16  chanspec; /* Current phy chanspec */
	uint16  vbatmeas; /* Measured VBAT sense value */
	uint16  featureflag; /* Currently active feature flags */
	int8    chiptemp; /* Chip temparature */
	int8    femtemp;  /* Fem temparature */

	uint32  nrate; /* Current Tx nrate */

	uint8   cal_phase_id; /* Current Multi phase cal ID */
	uint8   rxchain; /* Rx Chain */
	uint8   txchain; /* Tx Chain */
	uint8   ofdm_desense; /* OFDM desense */

	uint8   bphy_desense; /* BPHY desense */
	uint8   pll_lockstatus; /* PLL Lock status */
	uint8   pad1; /* Padding byte to align with word */
	uint8   pad2; /* Padding byte to align with word */

	uint32 duration;	/**< millisecs spent sampling this channel */
	uint32 congest_ibss;	/**< millisecs in our bss (presumably this traffic will */
				/**<  move if cur bss moves channels) */
	uint32 congest_obss;	/**< traffic not in our bss */
	uint32 interference;	/**< millisecs detecting a non 802.11 interferer. */

} phy_periodic_log_cmn_t;

typedef struct phy_periodic_log_cmn_v2 {
	uint16  chanspec; /* Current phy chanspec */
	uint16  vbatmeas; /* Measured VBAT sense value */
	uint16  featureflag; /* Currently active feature flags */
	int8    chiptemp; /* Chip temparature */
	int8    femtemp;  /* Fem temparature */

	uint32  nrate; /* Current Tx nrate */

	uint8   cal_phase_id; /* Current Multi phase cal ID */
	uint8   rxchain; /* Rx Chain */
	uint8   txchain; /* Tx Chain */
	uint8   ofdm_desense; /* OFDM desense */

	uint8   bphy_desense; /* BPHY desense */
	uint8   pll_lockstatus; /* PLL Lock status */

	uint32 duration;	/* millisecs spent sampling this channel */
	uint32 congest_ibss;	/* millisecs in our bss (presumably this traffic will */
				/*  move if cur bss moves channels) */
	uint32 congest_obss;	/* traffic not in our bss */
	uint32 interference;	/* millisecs detecting a non 802.11 interferer. */

	uint8 slice;
	uint8 version;		/* version of fw/ucode for debug purposes */
	bool phycal_disable;		/* Set if calibration is disabled */
	uint8 pad;
	uint16 phy_log_counter;
	uint16 noise_mmt_overdue;	/* Count up if ucode noise mmt is overdue for 5 sec */
	uint16 chan_switch_tm; /* Channel switch time */

	/* HP2P related params */
	uint16 shm_mpif_cnt_val;
	uint16 shm_thld_cnt_val;
	uint16 shm_nav_cnt_val;
	uint16 shm_cts_cnt_val;

	uint16 shm_m_prewds_cnt;	/* Count of pre-wds fired in the ucode */
	uint32 last_cal_time;		/* Last cal execution time */
	uint16 deaf_count;		/* Depth of stay_in_carrier_search function */
	uint32 ed20_crs0;		/* ED-CRS status on core 0 */
	uint32 ed20_crs1;		/* ED-CRS status on core 1 */
	uint32 noise_cal_req_ts;	/* Time-stamp when noise cal was requested */
	uint32 noise_cal_intr_ts;	/* Time-stamp when noise cal was completed */
	uint32 phywdg_ts;		/* Time-stamp when wd was fired */
	uint32 phywd_dur;			/* Duration of the watchdog */
	uint32 noise_mmt_abort_crs; /* Count of CRS during noise mmt */
	uint32 chanspec_set_ts;		/* Time-stamp when chanspec was set */
	uint32 vcopll_failure_cnt;	/* Number of VCO cal failures
					* (including failures detected in ucode).
					*/
	uint32 dcc_fail_counter;	/* Number of DC cal failures */
	uint32 log_ts;			/* Time-stamp when this log was collected */

	uint16 btcxovrd_dur;		/* Cumulative btcx overide between WDGs */
	uint16 btcxovrd_err_cnt;	/* BTCX override flagged errors */

	uint16  femtemp_read_fail_counter; /* Fem temparature read fail counter */
	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16 debug_01;
	uint16 debug_02;
} phy_periodic_log_cmn_v2_t;

typedef struct phy_periodic_log_cmn_v3 {
	uint32  nrate; /* Current Tx nrate */
	uint32	duration;	/**< millisecs spent sampling this channel */
	uint32	congest_ibss;	/**< millisecs in our bss (presumably this traffic will */
				/**<  move if cur bss moves channels) */
	uint32	congest_obss;	/**< traffic not in our bss */
	uint32	interference;	/**< millisecs detecting a non 802.11 interferer. */
	uint32  noise_cfg_exit1;
	uint32  noise_cfg_exit2;
	uint32  noise_cfg_exit3;
	uint32  noise_cfg_exit4;
	uint32	ed20_crs0;
	uint32	ed20_crs1;
	uint32	noise_cal_req_ts;
	uint32	noise_cal_crs_ts;
	uint32	log_ts;
	uint32	last_cal_time;
	uint32	phywdg_ts;
	uint32	chanspec_set_ts;
	uint32	noise_zero_inucode;
	uint32	phy_crs_during_noisemmt;
	uint32	wd_dur;

	int32	deaf_count;

	uint16  chanspec; /* Current phy chanspec */
	uint16  vbatmeas; /* Measured VBAT sense value */
	uint16  featureflag; /* Currently active feature flags */
	uint16	nav_cntr_l;
	uint16	nav_cntr_h;
	uint16	chanspec_set_last;
	uint16	ucode_noise_fb_overdue;
	uint16	phy_log_counter;
	uint16	shm_mpif_cnt_val;
	uint16	shm_thld_cnt_val;
	uint16	shm_nav_cnt_val;
	uint16	shm_dc_cnt_val;
	uint16	shm_txff_cnt_val;
	uint16	shm_cts_cnt_val;
	uint16	shm_m_prewds_cnt;

	uint8   cal_phase_id; /* Current Multi phase cal ID */
	uint8   rxchain; /* Rx Chain */
	uint8   txchain; /* Tx Chain */
	uint8   ofdm_desense; /* OFDM desense */
	uint8   bphy_desense; /* BPHY desense */
	uint8   pll_lockstatus; /* PLL Lock status */
	int8    chiptemp; /* Chip temparature */
	int8    femtemp;  /* Fem temparature */

	bool	phycal_disable;
	uint8   pad; /* Padding byte to align with word */
} phy_periodic_log_cmn_v3_t;

typedef struct phy_periodic_log_cmn_v4 {
	uint16  chanspec; /* Current phy chanspec */
	uint16  vbatmeas; /* Measured VBAT sense value */

	uint16  featureflag; /* Currently active feature flags */
	int8    chiptemp; /* Chip temparature */
	int8    femtemp;  /* Fem temparature */

	uint32  nrate; /* Current Tx nrate */

	uint8   cal_phase_id; /* Current Multi phase cal ID */
	uint8   rxchain; /* Rx Chain */
	uint8   txchain; /* Tx Chain */
	uint8   ofdm_desense; /* OFDM desense */

	uint8   slice;
	uint8   dbgfw_ver;	/* version of fw/ucode for debug purposes */
	uint8   bphy_desense; /* BPHY desense */
	uint8   pll_lockstatus; /* PLL Lock status */

	uint32 duration;	/* millisecs spent sampling this channel */
	uint32 congest_ibss;	/* millisecs in our bss (presumably this traffic will */
				/*  move if cur bss moves channels) */
	uint32 congest_obss;	/* traffic not in our bss */
	uint32 interference;	/* millisecs detecting a non 802.11 interferer. */

	/* HP2P related params */
	uint16 shm_mpif_cnt_val;
	uint16 shm_thld_cnt_val;
	uint16 shm_nav_cnt_val;
	uint16 shm_cts_cnt_val;

	uint16 shm_m_prewds_cnt;	/* Count of pre-wds fired in the ucode */
	uint16 deaf_count;		/* Depth of stay_in_carrier_search function */
	uint32 last_cal_time;		/* Last cal execution time */
	uint32 ed20_crs0;		/* ED-CRS status on core 0: TODO change to uint16 */
	uint32 ed20_crs1;		/* ED-CRS status on core 1: TODO change to uint16 */
	uint32 noise_cal_req_ts;	/* Time-stamp when noise cal was requested */
	uint32 noise_cal_intr_ts;	/* Time-stamp when noise cal was completed */
	uint32 phywdg_ts;		/* Time-stamp when wd was fired */
	uint32 phywd_dur;		/* Duration of the watchdog */
	uint32 noise_mmt_abort_crs;	/* Count of CRS during noise mmt: TODO change to uint16 */
	uint32 chanspec_set_ts;		/* Time-stamp when chanspec was set */
	uint32 vcopll_failure_cnt;	/* Number of VCO cal failures
					* (including failures detected in ucode).
					*/
	uint16 dcc_attempt_counter;	/* Number of DC cal attempts */
	uint16 dcc_fail_counter;	/* Number of DC cal failures */
	uint32 log_ts;			/* Time-stamp when this log was collected */

	uint16 btcxovrd_dur;		/* Cumulative btcx overide between WDGs */
	uint16 btcxovrd_err_cnt;	/* BTCX override flagged errors */

	uint16 femtemp_read_fail_counter; /* Fem temparature read fail counter */
	uint16 phy_log_counter;
	uint16 noise_mmt_overdue;	/* Count up if ucode noise mmt is overdue for 5 sec */
	uint16 chan_switch_tm;		/* Channel switch time */

	bool phycal_disable;		/* Set if calibration is disabled */

	/* dccal dcoe & idacc */
	uint8 dcc_err;			/* dccal health check error status */
	uint8 dcoe_num_tries;		/* number of retries on dcoe cal */
	uint8 idacc_num_tries;		/* number of retries on idac cal */

	uint8 dccal_phyrxchain;		/* phy rxchain during dc calibration */
	uint8 dccal_type;		/* DC cal type: single/multi phase, chan change, etc. */
	uint16 dcc_hcfail;		/* dcc health check failure count */
	uint16 dcc_calfail;		/* dcc failure count */

	uint16 noise_mmt_request_cnt;	/* Count of ucode noise cal request from phy */
	uint16 crsmin_pwr_apply_cnt;	/* Count of desense power threshold update to phy */
	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16 debug_03;
	uint16 debug_04;
	uint16 debug_05;
} phy_periodic_log_cmn_v4_t;

typedef struct phy_periodic_log_cmn_v5 {

	uint32 nrate;		/* Current Tx nrate */
	uint32 duration;	/* millisecs spent sampling this channel */
	uint32 congest_ibss;	/* millisecs in our bss (presumably this traffic will */
				/*  move if cur bss moves channels) */
	uint32 congest_obss;	/* traffic not in our bss */
	uint32 interference;	/* millisecs detecting a non 802.11 interferer. */
	uint32 last_cal_time;	/* Last cal execution time */

	uint32 noise_cal_req_ts;	/* Time-stamp when noise cal was requested */
	uint32 noise_cal_intr_ts;	/* Time-stamp when noise cal was completed */
	uint32 phywdg_ts;		/* Time-stamp when wd was fired */
	uint32 phywd_dur;		/* Duration of the watchdog */
	uint32 chanspec_set_ts;		/* Time-stamp when chanspec was set */
	uint32 vcopll_failure_cnt;	/* Number of VCO cal failures including */
					/* failures detected in ucode */
	uint32 log_ts;			/* Time-stamp when this log was collected */

	/* glitch based desense input from cca */
	uint32 cca_stats_total_glitch;
	uint32 cca_stats_bphy_glitch;
	uint32 cca_stats_total_badplcp;
	uint32 cca_stats_bphy_badplcp;
	uint32 cca_stats_mbsstime;

	uint32 counter_noise_request;	/* count of noisecal request */
	uint32 counter_noise_crsbit;	/* count of crs high during noisecal request */
	uint32 counter_noise_apply;	/* count of applying noisecal result to crsmin */
	uint32 fullphycalcntr;		/* count of performing single phase cal */
	uint32 multiphasecalcntr;	/* count of performing multi-phase cal */

	uint16 chanspec;		/* Current phy chanspec */
	uint16 vbatmeas;		/* Measured VBAT sense value */
	uint16 featureflag;		/* Currently active feature flags */

	/* HP2P related params */
	uint16 shm_mpif_cnt_val;
	uint16 shm_thld_cnt_val;
	uint16 shm_nav_cnt_val;
	uint16 shm_cts_cnt_val;

	uint16 shm_m_prewds_cnt;	/* Count of pre-wds fired in the ucode */
	uint16 deaf_count;		/* Depth of stay_in_carrier_search function */

	uint16 ed20_crs0;		/* ED-CRS status on core 0 */
	uint16 ed20_crs1;		/* ED-CRS status on core 1 */

	uint16 dcc_attempt_counter;	/* Number of DC cal attempts */
	uint16 dcc_fail_counter;	/* Number of DC cal failures */

	uint16 btcxovrd_dur;		/* Cumulative btcx overide between WDGs */
	uint16 btcxovrd_err_cnt;	/* BTCX override flagged errors */

	uint16 femtemp_read_fail_counter;	/* Fem temparature read fail counter */
	uint16 phy_log_counter;
	uint16 noise_mmt_overdue;	/* Count up if ucode noise mmt is overdue for 5 sec */
	uint16 chan_switch_tm;		/* Channel switch time */

	uint16 dcc_hcfail;		/* dcc health check failure count */
	uint16 dcc_calfail;		/* dcc failure count */
	uint16 crsmin_pwr_apply_cnt;	/* Count of desense power threshold update to phy */

	uint16 txpustatus;		/* txpu off definations */
	uint16 tempinvalid_count;	/* Count no. of invalid temp. measurements */
	uint16 log_event_id;		/* reuse debug_01, logging event id */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16 debug_02;
	uint16 debug_03;
	uint16 debug_04;
	uint16 debug_05;

	int8 chiptemp;			/* Chip temparature */
	int8 femtemp;			/* Fem temparature */

	uint8 cal_phase_id;		/* Current Multi phase cal ID */
	uint8 rxchain;			/* Rx Chain */
	uint8 txchain;			/* Tx Chain */
	uint8 ofdm_desense;		/* OFDM desense */

	uint8 slice;
	uint8 dbgfw_ver;		/* version of fw/ucode for debug purposes */
	uint8 bphy_desense;		/* BPHY desense */
	uint8 pll_lockstatus;		/* PLL Lock status */

	/* dccal dcoe & idacc */
	uint8 dcc_err;			/* dccal health check error status */
	uint8 dcoe_num_tries;		/* number of retries on dcoe cal */
	uint8 idacc_num_tries;		/* number of retries on idac cal */

	uint8 dccal_phyrxchain;		/* phy rxchain during dc calibration */
	uint8 dccal_type;		/* DC cal type: single/multi phase, chan change, etc. */

	uint8 gbd_bphy_sleep_counter;	/* gbd sleep counter */
	uint8 gbd_ofdm_sleep_counter;	/* gbd sleep counter */
	uint8 curr_home_channel;	/* gbd input channel from cca */

	/* desense data */
	int8 btcx_mode;			/* btcoex desense mode */
	int8 ltecx_mode;		/* lte coex desense mode */
	uint8 gbd_ofdm_desense;		/* gbd ofdm desense level */
	uint8 gbd_bphy_desense;		/* gbd bphy desense level */
	uint8 current_elna_bypass;	/* init gain desense: elna bypass */
	uint8 current_tia_idx;		/* init gain desense: tia index */
	uint8 current_lpf_idx;		/* init gain desense: lpf index */
	uint8 crs_auto_thresh;		/* crs auto threshold after desense */

	int8 weakest_rssi;		/* weakest link RSSI */
	uint8 noise_cal_mode;		/* noisecal mode */

	bool phycal_disable;		/* Set if calibration is disabled */
	bool hwpwrctrlen;		/* tx hwpwrctrl enable */
} phy_periodic_log_cmn_v5_t;

typedef struct phy_periodic_log_cmn_v6 {
	uint32	nrate;			/* Current Tx nrate */
	uint32	duration;		/* millisecs spent sampling this channel */
	uint32	congest_ibss;		/* millisecs in our bss (presumably this traffic will */
					/* move if cur bss moves channels) */
	uint32	congest_obss;		/* traffic not in our bss */
	uint32	interference;		/* millisecs detecting a non 802.11 interferer. */

	uint32	macsusp_dur;		/* mac suspend duration */

	uint16	chanspec;		/* Current phy chanspec */
	uint16	vbatmeas;		/* Measured VBAT sense value */
	uint16	featureflag;		/* Currently active feature flags */

	uint16	macsusp_cnt;		/* mac suspend counter */
	uint16	log_event_id;		/* logging event id */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16	debug_01;
	uint16	debug_02;
	uint16	debug_03;
	uint16	debug_04;
	uint16	debug_05;

	int8	chiptemp;		/* Chip temparature */
	int8	femtemp;		/* Fem temparature */

	uint8	cal_phase_id;		/* Current Multi phase cal ID */
	uint8	rxchain;		/* Rx Chain */

	uint8	txchain;		/* Tx Chain */
	uint8	ofdm_desense;		/* OFDM desense */

	uint8	bphy_desense;		/* BPHY desense */
	uint8	pll_lockstatus;		/* PLL Lock status */

	uint8	amtbitmap;		/* AMT status bitamp */
	uint8	pad1;			/* Padding byte to align with word */
	uint8	pad2;			/* Padding byte to align with word */
	uint8	pad3;			/* Padding byte to align with word */
} phy_periodic_log_cmn_v6_t;

typedef struct phy_periodic_log_cmn_v7 {

	uint32	nrate;			/* Current Tx nrate */
	uint32	duration;		/* millisecs spent sampling this channel */
	uint32	congest_ibss;		/* millisecs in our bss (presumably this traffic will */
					/*  move if cur bss moves channels) */
	uint32	congest_obss;		/* traffic not in our bss */
	uint32	interference;		/* millisecs detecting a non 802.11 interferer. */
	uint32	last_cal_time;		/* Last cal execution time */

	uint32	noise_cal_req_ts;	/* Time-stamp when noise cal was requested */
	uint32	noise_cal_intr_ts;	/* Time-stamp when noise cal was completed */
	uint32	phywdg_ts;		/* Time-stamp when wd was fired */
	uint32	phywd_dur;		/* Duration of the watchdog */
	uint32	chanspec_set_ts;	/* Time-stamp when chanspec was set */
	uint32	vcopll_failure_cnt;	/* Number of VCO cal failures including */
					/* failures detected in ucode */
	uint32	log_ts;			/* Time-stamp when this log was collected */

	/* glitch based desense input from cca */
	uint32	cca_stats_total_glitch;
	uint32	cca_stats_bphy_glitch;
	uint32	cca_stats_total_badplcp;
	uint32	cca_stats_bphy_badplcp;
	uint32	cca_stats_mbsstime;

	uint32	counter_noise_request;	/* count of noisecal request */
	uint32	counter_noise_crsbit;	/* count of crs high during noisecal request */
	uint32	counter_noise_apply;	/* count of applying noisecal result to crsmin */
	uint32	fullphycalcntr;		/* count of performing single phase cal */
	uint32	multiphasecalcntr;	/* count of performing multi-phase cal */

	uint32	macsusp_dur;		/* mac suspend duration */

	uint16	chanspec;		/* Current phy chanspec */
	uint16	vbatmeas;		/* Measured VBAT sense value */

	uint16	featureflag;		/* Currently active feature flags */

	/* HP2P related params */
	uint16	shm_mpif_cnt_val;
	uint16	shm_thld_cnt_val;
	uint16	shm_nav_cnt_val;
	uint16	shm_cts_cnt_val;
	uint16	shm_m_prewds_cnt;	/* Count of pre-wds fired in the ucode */

	uint16	deaf_count;		/* Depth of stay_in_carrier_search function */

	uint16	ed20_crs0;		/* ED-CRS status on core 0 */
	uint16	ed20_crs1;		/* ED-CRS status on core 1 */

	uint16	dcc_attempt_counter;	/* Number of DC cal attempts */
	uint16	dcc_fail_counter;	/* Number of DC cal failures */

	uint16	btcxovrd_dur;		/* Cumulative btcx overide between WDGs */
	uint16	btcxovrd_err_cnt;	/* BTCX override flagged errors */

	uint16	femtemp_read_fail_counter;	/* Fem temparature read fail counter */
	uint16	phy_log_counter;
	uint16	noise_mmt_overdue;	/* Count up if ucode noise mmt is overdue for 5 sec */
	uint16	chan_switch_tm;		/* Channel switch time */

	uint16	dcc_hcfail;		/* dcc health check failure count */
	uint16	dcc_calfail;		/* dcc failure count */
	uint16	crsmin_pwr_apply_cnt;	/* Count of desense power threshold update to phy */

	uint16	txpustatus;		/* txpu off definations */
	uint16	tempinvalid_count;	/* Count no. of invalid temp. measurements */
	uint16	log_event_id;		/* logging event id */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16	debug_01;
	uint16	debug_02;
	uint16	debug_03;
	uint16	debug_04;
	uint16	debug_05;

	uint16	macsusp_cnt;		/* mac suspend counter */
	uint8	amtbitmap;		/* AMT status bitamp */

	int8	chiptemp;		/* Chip temparature */
	int8	femtemp;		/* Fem temparature */

	uint8	cal_phase_id;		/* Current Multi phase cal ID */
	uint8	rxchain;		/* Rx Chain */
	uint8	txchain;		/* Tx Chain */
	uint8	ofdm_desense;		/* OFDM desense */

	uint8	slice;
	uint8	dbgfw_ver;		/* version of fw/ucode for debug purposes */
	uint8	bphy_desense;		/* BPHY desense */
	uint8	pll_lockstatus;		/* PLL Lock status */

	/* dccal dcoe & idacc */
	uint8	dcc_err;		/* dccal health check error status */
	uint8	dcoe_num_tries;		/* number of retries on dcoe cal */
	uint8	idacc_num_tries;	/* number of retries on idac cal */

	uint8	dccal_phyrxchain;	/* phy rxchain during dc calibration */
	uint8	dccal_type;		/* DC cal type: single/multi phase, chan change, etc. */

	uint8	gbd_bphy_sleep_counter;	/* gbd sleep counter */
	uint8	gbd_ofdm_sleep_counter;	/* gbd sleep counter */
	uint8	curr_home_channel;	/* gbd input channel from cca */

	/* desense data */
	int8	btcx_mode;		/* btcoex desense mode */
	int8	ltecx_mode;		/* lte coex desense mode */
	uint8	gbd_ofdm_desense;	/* gbd ofdm desense level */
	uint8	gbd_bphy_desense;	/* gbd bphy desense level */
	uint8	current_elna_bypass;	/* init gain desense: elna bypass */
	uint8	current_tia_idx;	/* init gain desense: tia index */
	uint8	current_lpf_idx;	/* init gain desense: lpf index */
	uint8	crs_auto_thresh;	/* crs auto threshold after desense */

	int8	weakest_rssi;		/* weakest link RSSI */
	uint8	noise_cal_mode;		/* noisecal mode */

	bool	phycal_disable;		/* Set if calibration is disabled */
	bool	hwpwrctrlen;		/* tx hwpwrctrl enable */
	uint8	pad1;			/* Padding byte to align with word */
	uint8	pad2;			/* Padding byte to align with word */
	uint8	pad3;			/* Padding byte to align with word */
} phy_periodic_log_cmn_v7_t;

typedef struct phy_periodic_log_cmn_v8 {

	uint32	nrate;			/* Current Tx nrate */
	uint32	duration;		/* millisecs spent sampling this channel */
	uint32	congest_ibss;		/* millisecs in our bss (presumably this traffic will */
					/*  move if cur bss moves channels) */
	uint32	congest_obss;		/* traffic not in our bss */
	uint32	interference;		/* millisecs detecting a non 802.11 interferer. */
	uint32	last_cal_time;		/* Last cal execution time */

	uint32	noise_cal_req_ts;	/* Time-stamp when noise cal was requested */
	uint32	noise_cal_intr_ts;	/* Time-stamp when noise cal was completed */
	uint32	phywdg_ts;		/* Time-stamp when wd was fired */
	uint32	phywd_dur;		/* Duration of the watchdog */
	uint32	chanspec_set_ts;	/* Time-stamp when chanspec was set */
	uint32	vcopll_failure_cnt;	/* Number of VCO cal failures including */
					/* failures detected in ucode */
	uint32	log_ts;			/* Time-stamp when this log was collected */

	/* glitch based desense input from cca */
	uint32	cca_stats_total_glitch;
	uint32	cca_stats_bphy_glitch;
	uint32	cca_stats_total_badplcp;
	uint32	cca_stats_bphy_badplcp;
	uint32	cca_stats_mbsstime;

	uint32	counter_noise_request;	/* count of noisecal request */
	uint32	counter_noise_crsbit;	/* count of crs high during noisecal request */
	uint32	counter_noise_apply;	/* count of applying noisecal result to crsmin */
	uint32	fullphycalcntr;		/* count of performing single phase cal */
	uint32	multiphasecalcntr;	/* count of performing multi-phase cal */

	uint32	macsusp_dur;		/* mac suspend duration */

	uint16	chanspec;		/* Current phy chanspec */
	uint16	vbatmeas;		/* Measured VBAT sense value */

	uint16	featureflag;		/* Currently active feature flags */

	/* HP2P related params */
	uint16	shm_mpif_cnt_val;
	uint16	shm_thld_cnt_val;
	uint16	shm_nav_cnt_val;
	uint16	shm_cts_cnt_val;
	uint16	shm_m_prewds_cnt;	/* Count of pre-wds fired in the ucode */

	uint16	deaf_count;		/* Depth of stay_in_carrier_search function */

	uint16	ed20_crs0;		/* ED-CRS status on core 0 */
	uint16	ed20_crs1;		/* ED-CRS status on core 1 */

	uint16	dcc_attempt_counter;	/* Number of DC cal attempts */
	uint16	dcc_fail_counter;	/* Number of DC cal failures */

	uint16	btcxovrd_dur;		/* Cumulative btcx overide between WDGs */
	uint16	btcxovrd_err_cnt;	/* BTCX override flagged errors */

	uint16	femtemp_read_fail_counter;	/* Fem temparature read fail counter */
	uint16	phy_log_counter;
	uint16	noise_mmt_overdue;	/* Count up if ucode noise mmt is overdue for 5 sec */
	uint16	chan_switch_tm;		/* Channel switch time */

	uint16	dcc_hcfail;		/* dcc health check failure count */
	uint16	dcc_calfail;		/* dcc failure count */
	uint16	crsmin_pwr_apply_cnt;	/* Count of desense power threshold update to phy */

	uint16	txpustatus;		/* txpu off definations */
	uint16	tempinvalid_count;	/* Count no. of invalid temp. measurements */
	uint16	log_event_id;		/* logging event id */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16	debug_01;
	uint16	debug_02;
	uint16	debug_03;
	uint16	debug_04;
	uint16	debug_05;

	uint16	macsusp_cnt;		/* mac suspend counter */
	uint8	amtbitmap;		/* AMT status bitamp */

	int8	chiptemp;		/* Chip temparature */
	int8	femtemp;		/* Fem temparature */

	uint8	cal_phase_id;		/* Current Multi phase cal ID */
	uint8	rxchain;		/* Rx Chain */
	uint8	txchain;		/* Tx Chain */
	uint8	ofdm_desense;		/* OFDM desense */

	uint8	slice;
	uint8	dbgfw_ver;		/* version of fw/ucode for debug purposes */
	uint8	bphy_desense;		/* BPHY desense */
	uint8	pll_lockstatus;		/* PLL Lock status */

	/* dccal dcoe & idacc */
	uint8	dcc_err;		/* dccal health check error status */
	uint8	dcoe_num_tries;		/* number of retries on dcoe cal */
	uint8	idacc_num_tries;	/* number of retries on idac cal */

	uint8	dccal_phyrxchain;	/* phy rxchain during dc calibration */
	uint8	dccal_type;		/* DC cal type: single/multi phase, chan change, etc. */

	uint8	gbd_bphy_sleep_counter;	/* gbd sleep counter */
	uint8	gbd_ofdm_sleep_counter;	/* gbd sleep counter */
	uint8	curr_home_channel;	/* gbd input channel from cca */

	/* desense data */
	int8	btcx_mode;		/* btcoex desense mode */
	int8	ltecx_mode;		/* lte coex desense mode */
	uint8	gbd_ofdm_desense;	/* gbd ofdm desense level */
	uint8	gbd_bphy_desense;	/* gbd bphy desense level */
	uint8	current_elna_bypass;	/* init gain desense: elna bypass */
	uint8	current_tia_idx;	/* init gain desense: tia index */
	uint8	current_lpf_idx;	/* init gain desense: lpf index */
	uint8	crs_auto_thresh;	/* crs auto threshold after desense */

	int8	weakest_rssi;		/* weakest link RSSI */
	uint8	noise_cal_mode;		/* noisecal mode */

	bool	phycal_disable;		/* Set if calibration is disabled */
	bool	hwpwrctrlen;		/* tx hwpwrctrl enable */
	int8	ed_threshold;		/* Threshold applied for ED */
	uint16	ed_crs_status;		/* Status of ED and CRS during noise cal */
	uint16	preempt_status1;	/* status of preemption */
	uint16	preempt_status2;	/* status of preemption */
	uint16	preempt_status3;	/* status of preemption */
	uint16	preempt_status4;	/* status of preemption */
	uint32	ed_duration;		/* ccastats: ed_duration */
} phy_periodic_log_cmn_v8_t;

/* Inherited from v8 */
typedef struct phy_periodic_log_cmn_v9 {

	uint32	nrate;			/* Current Tx nrate */
	uint32	duration;		/* millisecs spent sampling this channel */
	uint32	congest_ibss;		/* millisecs in our bss (presumably this traffic will */
					/*  move if cur bss moves channels) */
	uint32	congest_obss;		/* traffic not in our bss */
	uint32	interference;		/* millisecs detecting a non 802.11 interferer. */
	uint32	last_cal_time;		/* Last cal execution time */

	uint32	noise_cal_req_ts;	/* Time-stamp when noise cal was requested */
	uint32	noise_cal_intr_ts;	/* Time-stamp when noise cal was completed */
	uint32	phywdg_ts;		/* Time-stamp when wd was fired */
	uint32	phywd_dur;		/* Duration of the watchdog */
	uint32	chanspec_set_ts;	/* Time-stamp when chanspec was set */
	uint32	vcopll_failure_cnt;	/* Number of VCO cal failures including */
					/* failures detected in ucode */
	uint32	log_ts;			/* Time-stamp when this log was collected */

	/* glitch based desense input from cca */
	uint32	cca_stats_total_glitch;
	uint32	cca_stats_bphy_glitch;
	uint32	cca_stats_total_badplcp;
	uint32	cca_stats_bphy_badplcp;
	uint32	cca_stats_mbsstime;

	uint32	counter_noise_request;	/* count of noisecal request */
	uint32	counter_noise_crsbit;	/* count of crs high during noisecal request */
	uint32	counter_noise_apply;	/* count of applying noisecal result to crsmin */
	uint32	fullphycalcntr;		/* count of performing single phase cal */
	uint32	multiphasecalcntr;	/* count of performing multi-phase cal */

	uint32	macsusp_dur;		/* mac suspend duration */

	uint32	featureflag;		/* Currently active feature flags */

	uint16	chanspec;		/* Current phy chanspec */
	uint16	vbatmeas;		/* Measured VBAT sense value */

	/* HP2P related params */
	uint16	shm_mpif_cnt_val;
	uint16	shm_thld_cnt_val;
	uint16	shm_nav_cnt_val;
	uint16	shm_cts_cnt_val;
	uint16	shm_m_prewds_cnt;	/* Count of pre-wds fired in the ucode */

	uint16	deaf_count;		/* Depth of stay_in_carrier_search function */

	uint16	ed20_crs0;		/* ED-CRS status on core 0 */
	uint16	ed20_crs1;		/* ED-CRS status on core 1 */

	uint16	dcc_attempt_counter;	/* Number of DC cal attempts */
	uint16	dcc_fail_counter;	/* Number of DC cal failures */

	uint16	btcxovrd_dur;		/* Cumulative btcx overide between WDGs */
	uint16	btcxovrd_err_cnt;	/* BTCX override flagged errors */

	uint16	femtemp_read_fail_counter;	/* Fem temparature read fail counter */
	uint16	phy_log_counter;
	uint16	noise_mmt_overdue;	/* Count up if ucode noise mmt is overdue for 5 sec */
	uint16	chan_switch_tm;		/* Channel switch time */

	uint16	dcc_hcfail;		/* dcc health check failure count */
	uint16	dcc_calfail;		/* dcc failure count */
	uint16	crsmin_pwr_apply_cnt;	/* Count of desense power threshold update to phy */

	uint16	txpustatus;		/* txpu off definations */
	uint16	tempinvalid_count;	/* Count no. of invalid temp. measurements */
	uint16	log_event_id;		/* logging event id */

	/* Misc general purpose debug counters (will be used for future debugging) */
	uint16	debug_01;
	uint16	debug_02;
	uint16	debug_03;
	uint16	debug_04;
	uint16	debug_05;

	uint16	macsusp_cnt;		/* mac suspend counter */
	uint8	amtbitmap;		/* AMT status bitamp */

	int8	chiptemp;		/* Chip temparature */
	int8	femtemp;		/* Fem temparature */

	uint8	cal_phase_id;		/* Current Multi phase cal ID */
	uint8	rxchain;		/* Rx Chain */
	uint8	txchain;		/* Tx Chain */
	uint8	ofdm_desense;		/* OFDM desense */

	uint8	slice;
	uint8	dbgfw_ver;		/* version of fw/ucode for debug purposes */
	uint8	bphy_desense;		/* BPHY desense */
	uint8	pll_lockstatus;		/* PLL Lock status */

	/* dccal dcoe & idacc */
	uint8	dcc_err;		/* dccal health check error status */
	uint8	dcoe_num_tries;		/* number of retries on dcoe cal */
	uint8	idacc_num_tries;	/* number of retries on idac cal */

	uint8	dccal_phyrxchain;	/* phy rxchain during dc calibration */
	uint8	dccal_type;		/* DC cal type: single/multi phase, chan change, etc. */

	uint8	gbd_bphy_sleep_counter;	/* gbd sleep counter */
	uint8	gbd_ofdm_sleep_counter;	/* gbd sleep counter */
	uint8	curr_home_channel;	/* gbd input channel from cca */

	/* desense data */
	int8	btcx_mode;		/* btcoex desense mode */
	int8	ltecx_mode;		/* lte coex desense mode */
	uint8	gbd_ofdm_desense;	/* gbd ofdm desense level */
	uint8	gbd_bphy_desense;	/* gbd bphy desense level */
	uint8	current_elna_bypass;	/* init gain desense: elna bypass */
	uint8	current_tia_idx;	/* init gain desense: tia index */
	uint8	current_lpf_idx;	/* init gain desense: lpf index */
	uint8	crs_auto_thresh;	/* crs auto threshold after desense */

	int8	weakest_rssi;		/* weakest link RSSI */
	uint8	noise_cal_mode;		/* noisecal mode */

	bool	phycal_disable;		/* Set if calibration is disabled */
	bool	hwpwrctrlen;		/* tx hwpwrctrl enable */
	int8	ed_threshold;		/* Threshold applied for ED */
	uint16	ed_crs_status;		/* Status of ED and CRS during noise cal */
	uint16	preempt_status1;	/* status of preemption */
	uint16	preempt_status2;	/* status of preemption */
	uint16	preempt_status3;	/* status of preemption */
	uint16	preempt_status4;	/* status of preemption */
	uint16	debug_06;
	uint32	ed_duration;		/* ccastats: ed_duration */
} phy_periodic_log_cmn_v9_t;

typedef struct phy_periodic_log_cmn_v255 {
	uint32 nrate;               /* Current Tx nrate */
	uint32 duration;            /* millisecs spent sampling this channel */
	uint32 congest_ibss;        /* millisecs in our bss (presumably this traffic will */
	/*  move if cur bss moves channels) */
	uint32 congest_obss;        /* traffic not in our bss */
	uint32 interference;        /* millisecs detecting a non 802.11 interferer. */
	uint32 last_cal_time;              /* Last cal execution time */

	uint32 noise_cal_req_ts;    /* Time-stamp when noise cal was requested */
	uint32 noise_cal_intr_ts;   /* Time-stamp when noise cal was completed */
	uint32 phywdg_ts;           /* Time-stamp when wd was fired */
	uint32 phywd_dur;           /* Duration of the watchdog */
	uint32 chanspec_set_ts;     /* Time-stamp when chanspec was set */
	uint32 vcopll_failure_cnt;  /* Number of VCO cal failures including */
	/* failures detected in ucode */
	uint32 log_ts;                     /* Time-stamp when this log was collected */

	uint16  phymode;             /* phymode when this log was collected */

	/* glitch  based desense input from cca */
	uint32 cca_stats_total_glitch;
	uint32 cca_stats_bphy_glitch;
	uint32 cca_stats_total_badplcp;
	uint32 cca_stats_bphy_badplcp;
	uint32 cca_stats_mbsstime;

	uint32 counter_noise_request;      /* count of noisecal request */
	uint32 counter_noise_crsbit;       /* count of crs high during noisecal request */
	uint32 counter_noise_apply; /* count of applying noisecal result to crsmin */
	uint32 fullphycalcntr;             /* count of performing single phase cal */

	uint32 macsusp_dur;         /* mac suspend duration */

	uint32 featureflag;         /* Currently active feature flags */

	uint16 chanspec;            /* Current phy chanspec */
	uint16 vbatmeas;            /* Measured VBAT sense value */

	uint16 deaf_count;          /* Depth of stay_in_carrier_search function */

	uint16 ed20_crs;            /* ED-CRS status on all cores */

	uint16 dcc_attempt_counter; /* Number of DC cal attempts */
	uint16 dcc_fail_counter;    /* Number of DC cal failures */

	uint16 btcxovrd_dur;        /* Cumulative btcx overide between WDGs */
	uint16 btcxovrd_err_cnt;    /* BTCX override flagged errors */

	uint16 femtemp_read_fail_counter;  /* Fem temparature read fail counter */
	uint16 phy_log_counter;
	uint16 noise_mmt_overdue;   /* Count up if ucode noise mmt is overdue for 5 sec */
	uint16 chan_switch_tm;             /* Channel switch time */

	uint16 crsmin_pwr_apply_cnt;       /* Count of desense power threshold update to phy */

	uint16 txpustatus;          /* txpu off definations */
	uint16 tempinvalid_count;   /* Count no. of invalid temp. measurements */
	uint16 log_event_id;        /* logging event id */

	uint16 macsusp_cnt;         /* mac suspend counter */

	uint16 throttle_cnt;        /* temp throttling counter */

	uint16 resetcca_cnt;        /* resetcca acphy counter */

	int8   chiptemp;            /* Chip temparature */
	int8   femtemp;             /* Fem temparature */

	uint8  rxchain;             /* Rx Chain */
	uint8  txchain;             /* Tx Chain */
	uint8  ofdm_desense;        /* OFDM desense */

	uint8  dbgfw_ver;           /* version of fw/ucode for debug purposes */
	uint8  bphy_desense;        /* BPHY desense */
	uint8  pll_lockstatus;             /* PLL Lock status */

	/* dccal dcoe & idacc */
	int8  dcc_err;             /* dccal health check error status */

	uint8  dccal_phyrxchain;    /* phy rxchain during dc calibration */
	uint8  dccal_type;          /* DC cal type: single/multi phase, chan change, etc. */

	uint8  gbd_bphy_sleep_counter;     /* gbd sleep counter */
	uint8  gbd_ofdm_sleep_counter;     /* gbd sleep counter */
	uint8  curr_home_channel;   /* gbd input channel from cca */

	/* desense data */
	int8   btcx_mode;           /* btcoex desense mode */
	int8   ltecx_mode;          /* lte coex desense mode */
	uint8  gbd_ofdm_desense;    /* gbd ofdm desense level */
	uint8  gbd_bphy_desense;    /* gbd bphy desense level */
	uint8  current_elna_bypass; /* init gain desense: elna bypass */
	uint8  current_tia_idx;     /* init gain desense: tia index */
	uint8  current_lpf_idx;     /* init gain desense: lpf index */

	int8   weakest_rssi;        /* weakest link RSSI */
	uint8  noise_cal_mode;             /* noisecal mode */

	bool   phycal_disable;             /* Set if calibration is disabled */
	bool   hwpwrctrlen;         /* tx hwpwrctrl enable */
	int8   ed_threshold;        /* Threshold applied for ED */
	uint16 ed_crs_status;              /* Status of ED and CRS during noise cal */
	uint16 preempt_status1;     /* status of preemption */
	uint16 preempt_status2;     /* status of preemption */
	uint16 preempt_status3;     /* status of preemption */
	uint16 preempt_status4;     /* status of preemption */
	uint32 ed_duration;         /* ccastats: ed_duration */

} phy_periodic_log_cmn_v255_t;

typedef struct phy_periodic_log_core {
	uint8	baseindxval; /* TPC Base index */
	int8	tgt_pwr; /* Programmed Target power */
	int8	estpwradj; /* Current Est Power Adjust value */
	int8	crsmin_pwr; /* CRS Min/Noise power */
	int8	rssi_per_ant; /* RSSI Per antenna */
	int8	snr_per_ant; /* SNR Per antenna */
	int8	pad1; /* Padding byte to align with word */
	int8	pad2; /* Padding byte to align with word */
} phy_periodic_log_core_t;

typedef struct phy_periodic_log_core_v3 {
	uint8	baseindxval; /* TPC Base index */
	int8	tgt_pwr; /* Programmed Target power */
	int8	estpwradj; /* Current Est Power Adjust value */
	int8	crsmin_pwr; /* CRS Min/Noise power */
	int8	rssi_per_ant; /* RSSI Per antenna */
	int8	snr_per_ant; /* SNR Per antenna */

	/* dccal dcoe & idacc */
	uint16	dcoe_done_0;	/* dccal control register 44 */
	uint16	dcoe_done_1;	/* dccal control register 45 */
	uint16	dcoe_done_2;	/* dccal control register 46 */
	uint16	idacc_done_0;	/* dccal control register 21 */
	uint16	idacc_done_1;	/* dccal control register 60 */
	uint16	idacc_done_2;	/* dccal control register 61 */
	int16	psb;		/* psb read during dccal health check */
	uint8	pktproc;	/* pktproc read during dccal health check */

	int8	noise_level;	/* noise level = crsmin_pwr if gdb desense is lesser */
	int8	pad2; /* Padding byte to align with word */
	int8	pad3; /* Padding byte to align with word */
} phy_periodic_log_core_v3_t;

typedef struct phy_periodic_log_core_v4 {
	/* dccal dcoe & idacc */
	uint16	dcoe_done_0;	/* dccal control register 44 */
	uint16	dcoe_done_1;	/* dccal control register 45 */
	uint16	dcoe_done_2;	/* dccal control register 46 */
	uint16	idacc_done_0;	/* dccal control register 21 */
	uint16	idacc_done_1;	/* dccal control register 60 */
	uint16	idacc_done_2;	/* dccal control register 61 */
	int16	psb;		/* psb read during dccal health check */
	int16	txcap;		/* Txcap value */

	uint16	debug_01;	/* multipurpose debug register */
	uint16	debug_02;	/* multipurpose debug register */
	uint16	debug_03;	/* multipurpose debug register */
	uint16	debug_04;	/* multipurpose debug register */

	uint8	pktproc;	/* pktproc read during dccal health check */
	uint8	baseindxval;	/* TPC Base index */
	int8	tgt_pwr;	/* Programmed Target power */
	int8	estpwradj;	/* Current Est Power Adjust value */
	int8	crsmin_pwr;		/* CRS Min/Noise power */
	int8	rssi_per_ant;	/* RSSI Per antenna */
	int8	snr_per_ant;	/* SNR Per antenna */

	int8	noise_level;	/* noise pwr after filtering & averageing */
	int8	noise_level_inst;	/* instantaneous noise cal pwr */
	int8	estpwr;		/* tx powerDet value */
} phy_periodic_log_core_v4_t;

typedef struct phy_periodic_log_core_v5 {
	uint8	baseindxval;			/* TPC Base index */
	int8	tgt_pwr;			/* Programmed Target power */
	int8	estpwradj;			/* Current Est Power Adjust value */
	int8	crsmin_pwr;			/* CRS Min/Noise power */
	int8	rssi_per_ant;			/* RSSI Per antenna */
	int8	snr_per_ant;			/* SNR Per antenna */
	uint8	fp;
	int8	phylog_noise_pwr_array[8];	/* noise buffer array */
	int8	noise_dbm_ant;			/* from uCode shm read, afer converting to dBm */
} phy_periodic_log_core_v5_t;

#define PHY_NOISE_PWR_ARRAY_SIZE	(8u)
typedef struct phy_periodic_log_core_v6 {
	/* dccal dcoe & idacc */
	uint16	dcoe_done_0;	/* dccal control register 44 */
	uint16	dcoe_done_1;	/* dccal control register 45 */
	uint16	dcoe_done_2;	/* dccal control register 46 */
	uint16	idacc_done_0;	/* dccal control register 21 */
	uint16	idacc_done_1;	/* dccal control register 60 */
	uint16	idacc_done_2;	/* dccal control register 61 */
	int16	psb;		/* psb read during dccal health check */
	int16	txcap;		/* Txcap value */

	uint16	debug_01;	/* multipurpose debug register */
	uint16	debug_02;	/* multipurpose debug register */
	uint16	debug_03;	/* multipurpose debug register */
	uint16	debug_04;	/* multipurpose debug register */

	uint8	pktproc;	/* pktproc read during dccal health check */
	uint8	baseindxval;	/* TPC Base index */
	int8	tgt_pwr;	/* Programmed Target power */
	int8	estpwradj;	/* Current Est Power Adjust value */
	int8	crsmin_pwr;		/* CRS Min/Noise power */
	int8	rssi_per_ant;	/* RSSI Per antenna */
	int8	snr_per_ant;	/* SNR Per antenna */

	int8	noise_level;	/* noise pwr after filtering & averageing */
	int8	noise_level_inst;	/* instantaneous noise cal pwr */
	int8	estpwr;		/* tx powerDet value */
	int8	crsmin_th_idx;	/* idx used to lookup crs min thresholds */
	int8	debug_05;	/* multipurpose debug register */

	int8	phy_noise_pwr_array[PHY_NOISE_PWR_ARRAY_SIZE];	/* noise buffer array */
} phy_periodic_log_core_v6_t;

typedef struct phy_periodic_log_core_v255 {
	/* dccal dcoe & idacc */
	uint16 dcoe_done_0;  /* dccal control register 44 */
	uint16 dcoe_done_1;  /* dccal control register 45 */
	uint16 dcoe_done_2;  /* dccal control register 46 */
	uint16 idacc_done_0; /* dccal control register 21 */
	uint16 idacc_done_1; /* dccal control register 60 */
	uint8  dcoe_num_tries;             /* number of retries on dcoe cal */
	uint8  idacc_num_tries;     /* number of retries on idac cal */
	int16  txcap;        /* Txcap value */

	uint8  pktproc;      /* pktproc read during dccal health check */
	uint8  baseindxval;  /* TPC Base index */
	int8   tgt_pwr;      /* Programmed Target power */
	int8   estpwradj;    /* Current Est Power Adjust value */
	int8   crsmin_pwr;          /* CRS Min/Noise power */
	int8   rssi_per_ant; /* RSSI Per antenna */
	int8   snr_per_ant;  /* SNR Per antenna */

	int8   noise_level;  /* noise pwr after filtering & averageing */
	int8   noise_level_inst;    /* instantaneous noise cal pwr */
	int8   estpwr;              /* tx powerDet value */
	int8   crsmin_th_idx;       /* idx used to lookup crs min thresholds */

	int8   phy_noise_pwr_array[PHY_NOISE_PWR_ARRAY_SIZE];   /* noise buffer array */
} phy_periodic_log_core_v255_t;

typedef struct phy_periodic_log_core_v2 {
	int32 rxs; /* FDIQ Slope coeffecient */

	uint16	ofdm_txa; /* OFDM Tx IQ Cal a coeff */
	uint16	ofdm_txb; /* OFDM Tx IQ Cal b coeff */
	uint16	ofdm_txd; /* contain di & dq */
	uint16	rxa; /* Rx IQ Cal A coeffecient */
	uint16	rxb; /* Rx IQ Cal B coeffecient */
	uint16	baseidx; /* TPC Base index */

	uint8	baseindxval; /* TPC Base index */

	int8	tgt_pwr; /* Programmed Target power */
	int8	estpwradj; /* Current Est Power Adjust value */
	int8	crsmin_pwr; /* CRS Min/Noise power */
	int8	rssi_per_ant; /* RSSI Per antenna */
	int8	snr_per_ant; /* SNR Per antenna */
	int8	pad1; /* Padding byte to align with word */
	int8	pad2; /* Padding byte to align with word */
} phy_periodic_log_core_v2_t;

/* Shared BTCX Statistics for BTCX and PHY Logging, storing the accumulated values. */
#define BTCX_STATS_PHY_LOGGING_VER 1
typedef struct wlc_btc_shared_stats {
	uint32 bt_req_cnt;		/* #BT antenna requests since last stats sampl */
	uint32 bt_gnt_cnt;		/* #BT antenna grants since last stats sample */
	uint32 bt_gnt_dur;		/* usec BT owns antenna since last stats sample */
	uint16 bt_pm_attempt_cnt;	/* PM1 attempts */
	uint16 bt_succ_pm_protect_cnt;	/* successful PM protection */
	uint16 bt_succ_cts_cnt;		/* successful CTS2A protection */
	uint16 bt_wlan_tx_preempt_cnt;	/* WLAN TX Preemption */
	uint16 bt_wlan_rx_preempt_cnt;	/* WLAN RX Preemption */
	uint16 bt_ap_tx_after_pm_cnt;	/* AP TX even after PM protection */
	uint16 bt_crtpri_cnt;		/* Ant grant by critical BT task */
	uint16 bt_pri_cnt;		/* Ant grant by high BT task */
	uint16 antgrant_lt10ms;		/* Ant grant duration cnt 0~10ms */
	uint16 antgrant_lt30ms;		/* Ant grant duration cnt 10~30ms */
	uint16 antgrant_lt60ms;		/* Ant grant duration cnt 30~60ms */
	uint16 antgrant_ge60ms;		/* Ant grant duration cnt 60~ms */
} wlc_btc_shared_stats_v1_t;

/* BTCX Statistics for PHY Logging */
typedef struct wlc_btc_stats_phy_logging {
	uint16 ver;
	uint16 length;
	uint32 btc_status;		/* btc status log */
	uint32 bt_req_type_map;		/* BT Antenna Req types since last stats sample */
	wlc_btc_shared_stats_v1_t shared;
} phy_periodic_btc_stats_v1_t;

/* Trunk (KUDU_BRANCH_17_10) ONLY */
#define PHYCAL_LOG_VER255         (255u)
typedef struct phycal_log_cmn_v255 {
	uint16 chanspec;     /* current phy chanspec */
	uint8  reason;              /* cal reason */
	uint32 time;         /* time at which cal happened in sec */
	uint16 temp;         /* temperature at the time of cal */
	uint16 dur;          /* duration of cal in usec */

	uint8  rxgtbl_en;  /* Rx IQ Cal gain-table based */

	uint8  rcal_value;  /* rcal value */
	uint16 rccal_gmult;  /* rccal gmult value */
	uint16 rccal_cmult;  /* rccal cmult value */
} phycal_log_cmn_v255_t;

typedef struct phycal_log_core_v255 {
	uint16 ofdm_txa; /* OFDM Tx IQ Cal a coeff */
	uint16 ofdm_txb; /* OFDM Tx IQ Cal b coeff */
	uint16 ofdm_txd; /* contain di & dq */
	uint16 bphy_txa; /* BPHY Tx IQ Cal a coeff */
	uint16 bphy_txb; /* BPHY Tx IQ Cal b coeff */
	uint16 bphy_txd; /* contain di & dq */

	uint16 rxa; /* Rx IQ Cal A coeffecient */
	uint16 rxb; /* Rx IQ Cal B coeffecient */
	int32 rxs;  /* FDIQ Slope coeffecient */

	uint8 baseidx; /* TPC Base index */
	uint8 adc_coeff_cap0_adc0_I; /* ADC-0 CAP Cal Cap0 I */
	uint8 adc_coeff_cap1_adc0_I; /* ADC-0 CAP Cal Cap1 I */
	uint8 adc_coeff_cap2_adc0_I; /* ADC-0 CAP Cal Cap2 I */
	uint8 adc_coeff_cap0_adc0_Q; /* ADC-0 CAP Cal Cap0 Q */
	uint8 adc_coeff_cap1_adc0_Q; /* ADC-0 CAP Cal Cap1 Q */
	uint8 adc_coeff_cap2_adc0_Q; /* ADC-0 CAP Cal Cap2 Q */
	uint8 adc_coeff_cap0_adc1_I; /* ADC-1 CAP Cal Cap0 I */
	uint8 adc_coeff_cap1_adc1_I; /* ADC-1 CAP Cal Cap1 I */
	uint8 adc_coeff_cap2_adc1_I; /* ADC-1 CAP Cal Cap2 I */
	uint8 adc_coeff_cap0_adc1_Q; /* ADC-1 CAP Cal Cap0 Q */
	uint8 adc_coeff_cap1_adc1_Q; /* ADC-1 CAP Cal Cap1 Q */
	uint8 adc_coeff_cap2_adc1_Q; /* ADC-1 CAP Cal Cap2 Q */
	uint8 pad; /* Padding byte to align with word */
} phycal_log_core_v255_t;

typedef struct phycal_log_v255 {
	uint8  version; /* Logging structure version */
	uint8  numcores; /* Number of cores for which core specific data present */
	uint16 length;  /* Length of the entire structure */
	phycal_log_cmn_v255_t phycal_log_cmn; /* Logging common structure */
	/* This will be a variable length based on the numcores field defined above */
	phycal_log_core_v255_t phycal_log_core[4];
} phycal_log_v255_t;

/* OBSS Statistics for PHY Logging */
typedef struct phy_periodic_obss_stats_v1 {
	uint32	obss_last_read_time;			/* last stats read time */
	uint16	obss_mit_bw;				/* selected mitigation BW */
	uint16	obss_stats_cnt;				/* stats count */
	uint8	obss_mit_mode;				/* mitigation mode */
	uint8	obss_mit_status;			/* obss mitigation status */
	uint8	obss_curr_det[ACPHY_OBSS_SUBBAND_CNT];	/* obss curr detection */
	uint16	dynbw_init_reducebw_cnt;		/*
							 * bandwidth reduction cnt of
							 * initiator (txrts+rxcts)
							 */
	uint16	dynbw_resp_reducebw_cnt;		/*
							 * bandwidth reduction cnt of
							 * responder (rxrts+txcts)
							 */
	uint16	dynbw_rxdata_reducebw_cnt;		/*
							 * rx data cnt with reduced bandwidth
							 * as txcts requested
							 */
} phy_periodic_obss_stats_v1_t;

typedef struct phy_periodic_obss_stats_v255 {
	uint32	obss_last_read_time;			/* last stats read time */
	uint16	obss_mit_bw;				/* selected mitigation BW */
	uint16	obss_stats_cnt;				/* stats count */
	uint8	obss_mit_mode;				/* mitigation mode */
	uint8	obss_mit_status;			/* obss mitigation status */
	uint8	obss_curr_det[ACPHY_OBSS_SUBBAND_CNT];	/* obss curr detection */
	uint16	dynbw_init_reducebw_cnt;		/*
							 * bandwidth reduction cnt of
							 * initiator (txrts+rxcts)
							 */
	uint16	dynbw_resp_reducebw_cnt;		/*
							 * bandwidth reduction cnt of
							 * responder (rxrts+txcts)
							 */
	uint16	dynbw_rxdata_reducebw_cnt;		/*
							 * rx data cnt with reduced bandwidth
							 * as txcts requested
							 */
} phy_periodic_obss_stats_v255_t;

/* SmartCCA related PHY Logging */
typedef struct phy_periodic_scca_stats_v1 {
	uint32 asym_intf_cmplx_pwr[2];
	uint32 asym_intf_ncal_time;
	uint32 asym_intf_host_req_mit_turnon_time;
	uint32 core1_smask_val_bk;	/* bt fem control related */
	int32 asym_intf_ed_thresh;

	int16 crsminpoweru0;			/* crsmin thresh */
	int16 crsminpoweroffset0;		/* ac_offset core0 */
	int16 crsminpoweroffset1;		/* ac_offset core1 */
	int16 Core0InitGainCodeB;		/* rx mitigation: eLNA bypass setting */
	int16 Core1InitGainCodeB;		/* rx mitigation: eLNA bypass setting */
	int16 ed_crsEn;				/* phyreg(ed_crsEn) */
	int16 nvcfg0;				/* LLR deweighting coefficient */
	int16 SlnaRxMaskCtrl0;
	int16 SlnaRxMaskCtrl1;
	uint16 save_SlnaRxMaskCtrl0;
	uint16 save_SlnaRxMaskCtrl1;
	uint16 asym_intf_ncal_req_chspec;	/* channel request noisecal */
	/* asym_intf_stats includes the following bits:
	* b[0]:   bool asym_intf_rx_noise_mit_on;
	* b[1]:   bool asym_intf_tx_smartcca_on;
	* b[3:2]: bool asym_intf_elna_bypass[2];
	* b[4]:   bool asym_intf_valid_noise_samp;
	* b[5]:   bool asym_intf_fill_noise_buf;
	* b[6]:   bool asym_intf_ncal_discard;
	* b[7]:   bool slna_reg_saved;
	* b[8]:   bool asym_intf_host_ext_usb;		//Host control related variable
	* b[9]:   bool asym_intf_host_ext_usb_chg;	// Host control related variable
	* b[10]:  bool asym_intf_host_en;		// Host control related variable
	* b[11]:  bool asym_intf_host_enable;
	* b[12]:  bool asym_intf_pending_host_req;	// Set request pending if clk not present
	*/
	uint16 asym_intf_stats;

	uint8 elna_bypass;	/* from bt_desense.elna_bypass in gainovr_shm_config() */
	uint8 btc_mode;		/* from bt_desense in gainovr_shm_config() */
	/* noise at antenna from phy_ac_noise_ant_noise_calc() */
	int8 noise_dbm_ant[2];
	/* in phy_ac_noise_calc(), also used by wl noise */
	int8 noisecalc_cmplx_pwr_dbm[2];
	uint8 gain_applied;		/* from phy_ac_rxgcrs_set_init_clip_gain() */

	int8 asym_intf_tx_smartcca_cm;
	int8 asym_intf_rx_noise_mit_cm;
	int8 asym_intf_avg_noise[2];
	int8 asym_intf_latest_noise[2];
	/* used to calculate noise_delta for rx mitigation on/off */
	int8 asym_intf_prev_noise_lvl[2];
	uint8 asym_intf_noise_calc_gain_2g[2];
	uint8 asym_intf_noise_calc_gain_5g[2];
	uint8 asym_intf_ant_noise_idx;
	uint8 asym_intf_least_core_idx;
	uint8 phy_crs_thresh_save[2];
	uint8 asym_intf_initg_desense[2];
	uint8 asym_intf_pending_host_req_type;	/* Set request pending if clk not present */
} phy_periodic_scca_stats_v1_t;

/* SmartCCA related PHY Logging */
typedef struct phy_periodic_scca_stats_v2 {
	uint32 asym_intf_cmplx_pwr[2];
	uint32 asym_intf_ncal_time;
	uint32 asym_intf_host_req_mit_turnon_time;
	uint32 core1_smask_val_bk;	/* bt fem control related */
	int32 asym_intf_ed_thresh;

	int16 crsminpoweru0;			/* crsmin thresh */
	int16 crsminpoweroffset0;		/* ac_offset core0 */
	int16 crsminpoweroffset1;		/* ac_offset core1 */
	int16 Core0InitGainCodeB;		/* rx mitigation: eLNA bypass setting */
	int16 Core1InitGainCodeB;		/* rx mitigation: eLNA bypass setting */
	int16 ed_crsEn;				/* phyreg(ed_crsEn) */
	int16 nvcfg0;				/* LLR deweighting coefficient */
	int16 SlnaRxMaskCtrl0;
	int16 SlnaRxMaskCtrl1;
	uint16 save_SlnaRxMaskCtrl0;
	uint16 save_SlnaRxMaskCtrl1;
	uint16 asym_intf_ncal_req_chspec;	/* channel request noisecal */
	/* asym_intf_stats includes the following bits:
	* b[0]:   bool asym_intf_rx_noise_mit_on;
	* b[1]:   bool asym_intf_tx_smartcca_on;
	* b[3:2]: bool asym_intf_elna_bypass[2];
	* b[4]:   bool asym_intf_valid_noise_samp;
	* b[5]:   bool asym_intf_fill_noise_buf;
	* b[6]:   bool asym_intf_ncal_discard;
	* b[7]:   bool slna_reg_saved;
	* b[8]:   bool asym_intf_host_ext_usb;		//Host control related variable
	* b[9]:   bool asym_intf_host_ext_usb_chg;	// Host control related variable
	* b[10]:  bool asym_intf_host_en;		// Host control related variable
	* b[11]:  bool asym_intf_host_enable;
	* b[12]:  bool asym_intf_pending_host_req;	// Set request pending if clk not present
	*/
	uint16 asym_intf_stats;

	uint8 elna_bypass;	/* from bt_desense.elna_bypass in gainovr_shm_config() */
	uint8 btc_mode;		/* from bt_desense in gainovr_shm_config() */
	/* noise at antenna from phy_ac_noise_ant_noise_calc() */
	int8 noise_dbm_ant[2];
	/* in phy_ac_noise_calc(), also used by wl noise */
	int8 noisecalc_cmplx_pwr_dbm[2];
	uint8 gain_applied;		/* from phy_ac_rxgcrs_set_init_clip_gain() */

	int8 asym_intf_tx_smartcca_cm;
	int8 asym_intf_rx_noise_mit_cm;
	int8 asym_intf_avg_noise[2];
	int8 asym_intf_latest_noise[2];
	/* used to calculate noise_delta for rx mitigation on/off */
	int8 asym_intf_prev_noise_lvl[2];
	uint8 asym_intf_noise_calc_gain_2g[2];
	uint8 asym_intf_noise_calc_gain_5g[2];
	uint8 asym_intf_ant_noise_idx;
	uint8 asym_intf_least_core_idx;
	uint8 phy_crs_thresh_save[2];
	uint8 asym_intf_initg_desense[2];
	uint8 asym_intf_pending_host_req_type;	/* Set request pending if clk not present */
	uint16	asym_intf_ncal_crs_stat[12];
	uint8	asym_intf_ncal_crs_stat_idx;
} phy_periodic_scca_stats_v2_t;

/* SmartCCA related PHY Logging */
typedef struct phy_periodic_scca_stats_v255 {
	uint32 asym_intf_cmplx_pwr[2];
	uint32 asym_intf_ncal_time;
	uint32 asym_intf_host_req_mit_turnon_time;
	uint32 core1_smask_val_bk;	/* bt fem control related */
	int32 asym_intf_ed_thresh;

	int16 crsminpoweru0;			/* crsmin thresh */
	int16 crsminpoweroffset0;		/* ac_offset core0 */
	int16 crsminpoweroffset1;		/* ac_offset core1 */
	int16 Core0InitGainCodeB;		/* rx mitigation: eLNA bypass setting */
	int16 Core1InitGainCodeB;		/* rx mitigation: eLNA bypass setting */
	int16 ed_crsEn;				/* phyreg(ed_crsEn) */
	int16 nvcfg0;				/* LLR deweighting coefficient */
	int16 SlnaRxMaskCtrl0;
	int16 SlnaRxMaskCtrl1;
	uint16 save_SlnaRxMaskCtrl0;
	uint16 save_SlnaRxMaskCtrl1;
	uint16 asym_intf_ncal_req_chspec;	/* channel request noisecal */
	/* asym_intf_stats includes the following bits:
	* b[0]:   bool asym_intf_rx_noise_mit_on;
	* b[1]:   bool asym_intf_tx_smartcca_on;
	* b[3:2]: bool asym_intf_elna_bypass[2];
	* b[4]:   bool asym_intf_valid_noise_samp;
	* b[5]:   bool asym_intf_fill_noise_buf;
	* b[6]:   bool asym_intf_ncal_discard;
	* b[7]:   bool slna_reg_saved;
	* b[8]:   bool asym_intf_host_ext_usb;		//Host control related variable
	* b[9]:   bool asym_intf_host_ext_usb_chg;	// Host control related variable
	* b[10]:  bool asym_intf_host_en;		// Host control related variable
	* b[11]:  bool asym_intf_host_enable;
	* b[12]:  bool asym_intf_pending_host_req;	// Set request pending if clk not present
	*/
	uint16 asym_intf_stats;

	uint8 elna_bypass;	/* from bt_desense.elna_bypass in gainovr_shm_config() */
	uint8 btc_mode;		/* from bt_desense in gainovr_shm_config() */
	/* noise at antenna from phy_ac_noise_ant_noise_calc() */
	int8 noise_dbm_ant[2];
	/* in phy_ac_noise_calc(), also used by wl noise */
	int8 noisecalc_cmplx_pwr_dbm[2];
	uint8 gain_applied;		/* from phy_ac_rxgcrs_set_init_clip_gain() */

	int8 asym_intf_tx_smartcca_cm;
	int8 asym_intf_rx_noise_mit_cm;
	int8 asym_intf_avg_noise[2];
	int8 asym_intf_latest_noise[2];
	/* used to calculate noise_delta for rx mitigation on/off */
	int8 asym_intf_prev_noise_lvl[2];
	uint8 asym_intf_noise_calc_gain_2g[2];
	uint8 asym_intf_noise_calc_gain_5g[2];
	uint8 asym_intf_ant_noise_idx;
	uint8 asym_intf_least_core_idx;
	uint8 phy_crs_thresh_save[2];
	uint8 asym_intf_initg_desense[2];
	uint8 asym_intf_pending_host_req_type;	/* Set request pending if clk not present */
	uint16	asym_intf_ncal_crs_stat[12];
	uint8	asym_intf_ncal_crs_stat_idx;
} phy_periodic_scca_stats_v255_t;

#define PHY_PERIODIC_LOG_VER1         (1u)

typedef struct phy_periodic_log_v1 {
	uint8  version; /* Logging structure version */
	uint8  numcores; /* Number of cores for which core specific data present */
	uint16 length;  /* Length of the entire structure */
	phy_periodic_log_cmn_t phy_perilog_cmn;
	phy_periodic_counters_v1_t counters_peri_log;
	/* This will be a variable length based on the numcores field defined above */
	phy_periodic_log_core_t phy_perilog_core[];
} phy_periodic_log_v1_t;

#define PHYCAL_LOG_VER3		(3u)
#define PHY_PERIODIC_LOG_VER3	(3u)

/* 4387 onwards */
typedef struct phy_periodic_log_v3 {
	uint8  version; /* Logging structure version */
	uint8  numcores; /* Number of cores for which core specific data present */
	uint16 length;  /* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v2_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v3_t counters_peri_log;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_t phy_perilog_core[];
} phy_periodic_log_v3_t;

#define PHY_PERIODIC_LOG_VER5	(5u)

typedef struct phy_periodic_log_v5 {
	uint8  version; /* Logging structure version */
	uint8  numcores; /* Number of cores for which core specific data present */
	uint16 length;  /* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v4_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v3_t counters_peri_log;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v3_t phy_perilog_core[];
} phy_periodic_log_v5_t;

#define PHY_PERIODIC_LOG_VER6	(6u)

typedef struct phy_periodic_log_v6 {
	uint8  version; /* Logging structure version */
	uint8  numcores; /* Number of cores for which core specific data present */
	uint16 length;  /* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v5_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v6_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_v1_t phy_perilog_btc_stats;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v4_t phy_perilog_core[];
} phy_periodic_log_v6_t;

typedef struct phycal_log_v3 {
	uint8  version; /* Logging structure version */
	uint8  numcores; /* Number of cores for which core specific data present */
	uint16 length;  /* Length of the entire structure */
	phycal_log_cmn_v2_t phycal_log_cmn; /* Logging common structure */
	/* This will be a variable length based on the numcores field defined above */
	phycal_log_core_v3_t phycal_log_core[];
} phycal_log_v3_t;

/* Note: The version 2 is reserved for 4357 only. Future chips must not use this version. */

#define MAX_CORE_4357		(2u)
#define PHYCAL_LOG_VER2		(2u)
#define PHY_PERIODIC_LOG_VER2	(2u)

typedef struct {
	uint32	txallfrm;	/**< total number of frames sent, incl. Data, ACK, RTS, CTS,
				* Control Management (includes retransmissions)
				*/
	uint32	rxrsptmout;	/**< number of response timeouts for transmitted frames
				* expecting a response
				*/
	uint32	rxstrt;		/**< number of received frames with a good PLCP */
	uint32  rxbadplcp;	/**< number of parity check of the PLCP header failed */
	uint32  rxcrsglitch;	/**< PHY was able to correlate the preamble but not the header */
	uint32  bphy_badplcp;	/**< number of bad PLCP reception on BPHY rate */
	uint32  bphy_rxcrsglitch;	/**< PHY count of bphy glitches */
	uint32	rxbeaconmbss;	/**< beacons received from member of BSS */
	uint32	rxdtucastmbss;	/**< number of received DATA frames with good FCS and matching RA */
	uint32  rxf0ovfl;	/** < Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;	/** < Rx FIFO1 overflow counters information */
	uint32	rxdtocast;	/**< number of received DATA frames (good FCS and no matching RA) */
	uint32  rxtoolate;	/**< receive too late */
	uint32  rxbadfcs;	/**< number of frames for which the CRC check failed in the MAC */
} phy_periodic_counters_v2_t;

/* Note: The version 2 is reserved for 4357 only. All future chips must not use this version. */

typedef struct phycal_log_core_v2 {
	uint16 ofdm_txa; /* OFDM Tx IQ Cal a coeff */
	uint16 ofdm_txb; /* OFDM Tx IQ Cal b coeff */
	uint16 ofdm_txd; /* contain di & dq */
	uint16 rxa; /* Rx IQ Cal A coeffecient */
	uint16 rxb; /* Rx IQ Cal B coeffecient */
	uint8 baseidx; /* TPC Base index */
	uint8 pad;
	int32 rxs; /* FDIQ Slope coeffecient */
} phycal_log_core_v2_t;

/* Note: The version 2 is reserved for 4357 only. All future chips must not use this version. */

typedef struct phycal_log_v2 {
	uint8  version; /* Logging structure version */
	uint16 length;  /* Length of the entire structure */
	uint8 pad;
	phycal_log_cmn_t phycal_log_cmn; /* Logging common structure */
	phycal_log_core_v2_t phycal_log_core[MAX_CORE_4357];
} phycal_log_v2_t;

/* Note: The version 2 is reserved for 4357 only. All future chips must not use this version. */

typedef struct phy_periodic_log_v2 {
	uint8  version; /* Logging structure version */
	uint16 length;  /* Length of the entire structure */
	uint8 pad;
	phy_periodic_log_cmn_t phy_perilog_cmn;
	phy_periodic_counters_v2_t counters_peri_log;
	phy_periodic_log_core_t phy_perilog_core[MAX_CORE_4357];
} phy_periodic_log_v2_t;

#define PHY_PERIODIC_LOG_VER4	(4u)

/*
 * Note: The version 4 is reserved for 4357 Deafness Debug only.
 * All future chips must not use this version.
 */
typedef struct phy_periodic_log_v4 {
	uint8  version; /* Logging structure version */
	uint8  pad;
	uint16 length;  /* Length of the entire structure */
	phy_periodic_log_cmn_v3_t  phy_perilog_cmn;
	phy_periodic_counters_v4_t counters_peri_log;
	phy_periodic_log_core_v2_t phy_perilog_core[MAX_CORE_4357];
} phy_periodic_log_v4_t;

/* Event log payload for enhanced roam log */
typedef enum {
	ROAM_LOG_SCANSTART = 1,		/* EVT log for roam scan start */
	ROAM_LOG_SCAN_CMPLT = 2,	/* EVT log for roam scan completeted */
	ROAM_LOG_ROAM_CMPLT = 3,	/* EVT log for roam done */
	ROAM_LOG_NBR_REQ = 4,		/* EVT log for Neighbor REQ */
	ROAM_LOG_NBR_REP = 5,		/* EVT log for Neighbor REP */
	ROAM_LOG_BCN_REQ = 6,		/* EVT log for BCNRPT REQ */
	ROAM_LOG_BCN_REP = 7,		/* EVT log for BCNRPT REP */
	ROAM_LOG_BTM_REP = 8,		/* EVT log for BTM REP */
	ROAM_LOG_WIPS_EVENT = 9,	/* EVT log for WIPS Event */
	ROAM_LOG_6G_NOVLP_REP = 10,	/* EVT log for 6G NoVLP Report */
	ROAM_LOG_WTC_BTM_REP = 11,	/* EVT log for WTC BTM Req/Resp Report */
	ROAM_LOG_BTM_QUERY = 12,	/* EVT log for WTC BTM Query */
	PRSV_PERIODIC_ID_MAX
} prsv_periodic_id_enum_t;

typedef struct prsv_periodic_log_hdr {
	uint8 version;
	uint8 id;
	uint16 length;
} prsv_periodic_log_hdr_t;

#define ROAM_LOG_VER_1	(1u)
#define ROAM_LOG_VER_2	(2u)
#define ROAM_LOG_VER_3	(3u)
#define ROAM_SSID_LEN	(32u)
typedef struct roam_log_trig_v1 {
	prsv_periodic_log_hdr_t hdr;
	int8 rssi;
	uint8 current_cu;
	uint8 pad[2];
	uint reason;
	int result;
	union {
		struct {
			uint rcvd_reason;
		} prt_roam;
		struct {
			uint8 req_mode;
			uint8 token;
			uint16 nbrlist_size;
			uint32 disassoc_dur;
			uint32 validity_dur;
			uint32 bss_term_dur;
		} bss_trans;
	};
} roam_log_trig_v1_t;

typedef struct roam_log_trig_v2 {
	prsv_periodic_log_hdr_t hdr;
	int8 rssi;
	uint8 current_cu;
	uint8 full_scan;
	uint8 pad;
	uint reason;
	int result;
	union {
		struct {
			uint rcvd_reason;
		} prt_roam;
		struct {
			uint8 req_mode;
			uint8 token;
			uint16 nbrlist_size;
			uint32 disassoc_dur;
			uint32 validity_dur;
			uint32 bss_term_dur;
		} bss_trans;
		struct {
			int rssi_threshold;
		} low_rssi;
	};
} roam_log_trig_v2_t;

#define ROAM_LOG_RPT_SCAN_LIST_SIZE 3
#define ROAM_LOG_INVALID_TPUT 0xFFFFFFFFu
typedef struct roam_scan_ap_info {
	int8 rssi;
	uint8 cu;
	uint8 pad[2];
	uint32 score;
	uint16 chanspec;
	struct ether_addr addr;
	uint32 estm_tput;
} roam_scan_ap_info_t;

typedef struct roam_log_scan_cmplt_v1 {
	prsv_periodic_log_hdr_t hdr;
	uint8 full_scan;
	uint8 scan_count;
	uint8 scan_list_size;
	uint8 pad;
	int32 score_delta;
	roam_scan_ap_info_t cur_info;
	roam_scan_ap_info_t scan_list[ROAM_LOG_RPT_SCAN_LIST_SIZE];
} roam_log_scan_cmplt_v1_t;

#define ROAM_CHN_UNI_2A		36u
#define ROAM_CHN_UNI_2A_MAX	64u
#define ROAM_CHN_UNI_2C		100u
#define ROAM_CHN_UNI_2C_MAX	144u
#define ROAM_CHN_UNI_3		149u
#define ROAM_CHN_UNI_3_MAX	165u
#define ROAM_CHN_SPACE		2u /* channel index space for 5G */

typedef struct roam_log_scan_cmplt_v2 {
	prsv_periodic_log_hdr_t hdr;
	uint8 scan_count;
	uint8 scan_list_size;
	uint8 chan_num;
	uint8 pad;
	uint16 band2g_chan_list;
	uint16 uni2a_chan_list;
	uint8 uni2c_chan_list[3];
	uint8 uni3_chan_list;
	int32 score_delta;
	roam_scan_ap_info_t cur_info;
	roam_scan_ap_info_t scan_list[ROAM_LOG_RPT_SCAN_LIST_SIZE];
} roam_log_scan_cmplt_v2_t;

typedef struct roam_log_cmplt_v1 {
	prsv_periodic_log_hdr_t hdr;
	uint status;	/* status code WLC_E STATUS */
	uint reason;	/* roam trigger reason */
	uint16	chanspec; /* new bssid chansepc */
	struct ether_addr addr; /* ether addr */
	uint8 pad[3];
	uint8 retry;
} roam_log_cmplt_v1_t;

typedef roam_log_cmplt_v1_t roam_log_cmplt_v2_t;

typedef struct roam_log_nbrrep {
	prsv_periodic_log_hdr_t hdr;
	uint channel_num;
} roam_log_nbrrep_v1_t;

typedef struct roam_log_nbrrep_v2 {
	prsv_periodic_log_hdr_t hdr;
	uint channel_num;
	uint16 band2g_chan_list; /* channel bit map */
	uint16 uni2a_chan_list;
	uint8 uni2c_chan_list[3];
	uint8 uni3_chan_list;
} roam_log_nbrrep_v2_t;

typedef struct roam_log_nbrreq {
	prsv_periodic_log_hdr_t hdr;
	uint token;
} roam_log_nbrreq_v1_t;

typedef roam_log_nbrreq_v1_t roam_log_nbrreq_v2_t;

typedef struct roam_log_bcnrptreq {
	prsv_periodic_log_hdr_t hdr;
	int32 result;
	uint8 reg;	/* operating class */
	uint8 channel;  /* number of requesting channel */
	uint8 mode;		/* request mode d11 rmreq bcn */
	uint8 bssid_wild; /* is wild bssid */
	uint8 ssid_len;		/* length of SSID */
	uint8 pad;
	uint16 duration;	/* duration */
	uint8 ssid[ROAM_SSID_LEN];
} roam_log_bcnrpt_req_v1_t;

typedef roam_log_bcnrpt_req_v1_t roam_log_bcnrpt_req_v2_t;

typedef struct roam_log_bcnrptrep {
	prsv_periodic_log_hdr_t hdr;
	uint32 count;
} roam_log_bcnrpt_rep_v1_t;

typedef struct roam_log_bcnrptrep_v2 {
	prsv_periodic_log_hdr_t hdr;
	uint8 scan_inprogress; /* if scan in progress TRUE */
	uint8 reason;			/* report mode d11 RMREP mode */
	uint32 count;
} roam_log_bcnrpt_rep_v2_t;

typedef struct roam_log_btmrep_v2 {
	prsv_periodic_log_hdr_t hdr;
	uint8 req_mode; /* d11 BSSTRANS req mode */
	uint8 status; /* d11 BSSTRANS response status code */
	uint16 pad[2];
	int	result;
} roam_log_btm_rep_v2_t;

/* ROAM_LOG_VER_3 specific structures */
typedef struct roam_log_btmrep_v3 {
	prsv_periodic_log_hdr_t hdr;
	uint8 req_mode; /* d11 BSSTRANS req mode */
	uint8 status; /* d11 BSSTRANS response status code */
	uint16 pad[2];
	struct ether_addr target_addr; /* bssid to move */
	int	result;
} roam_log_btm_rep_v3_t;

typedef struct roam_log_bcnrptreq_v3 {
	prsv_periodic_log_hdr_t hdr;
	int32 result;
	uint8 reg;	/* operating class */
	uint8 channel;  /* number of requesting channel */
	uint8 mode;		/* request mode d11 rmreq bcn */
	uint8 bssid_wild; /* is wild bssid */
	uint8 ssid_len;		/* length of SSID */
	uint8 pad;
	uint16 duration;	/* duration */
	uint8 ssid[ROAM_SSID_LEN];
	uint channel_num;	/* number of scan channel */
	uint16 band2g_chan_list; /* channel bit map */
	uint16 uni2a_chan_list;
	uint8 uni2c_chan_list[3];
	uint8 uni3_chan_list;
} roam_log_bcnrpt_req_v3_t;

#define BCNRPT_RSN_SUCCESS	0
#define BCNRPT_RSN_BADARG	1
#define BCNRPT_RSN_SCAN_ING	2
#define BCNRPT_RSN_SCAN_FAIL	3

typedef struct roam_log_bcnrptrep_v3 {
	prsv_periodic_log_hdr_t hdr;
	uint8 scan_status;		/* scan status */
	uint8 reason;			/* report mode d11 RMREP mode */
	uint16 reason_detail;
	uint32 count;
	uint16 duration;		/* duration */
	uint16 pad;
} roam_log_bcnrpt_rep_v3_t;

typedef struct roam_log_wips_evt_v3 {
	prsv_periodic_log_hdr_t hdr;
	uint32	timestamp;
	struct ether_addr bssid; /* ether addr */
	uint16	misdeauth;
	int16	current_rssi;
	int16	deauth_rssi;
} roam_log_wips_evt_v3_t;

typedef struct roam_log_6g_novlp_v3 {
	prsv_periodic_log_hdr_t hdr;
	struct ether_addr bssid;	/* ether addr */
	uint16	chanspec;		/* Chanspec */
} roam_log_6g_novlp_v3_t;

#define WTC_BTMREQ	0		/* WTC BTM request type */
#define WTC_BTMRESP	1		/* WTC BTM response type */

typedef struct roam_log_wtc_btmrep_v3 {
	prsv_periodic_log_hdr_t hdr;
	uint8	wtc_type;		/* 0:WTC_BTMREQ, 1:WTC_BTMRESP */
	uint8	ie_length;
	uint8	wtc_ver;
	uint8	pad;
	union {
		struct {
			/* WTC configuration from iovar */
			uint8	mode;
			uint8	scantype;
			int8	rssithresh[WTC_MAX_BAND];
			int8	ap_rssithresh[WTC_MAX_BAND];
			/* WTC Request IE */
			uint8	rsn_code;
			uint8	subcode;
			uint8	duration;	/* by default 0 */
			uint8	status;		/* WTC request is invalid */
		} wtcreq;
		struct {
			/* WTC Response IE */
			uint8	rsn_code;	/* WTC VSIE reason code */
			uint8	status;		/* BTM response status */
			uint16	pad;
		} wtcresp;
	};
} roam_log_wtc_btmrep_v3_t;

typedef struct roam_log_btm_query_v3 {
	prsv_periodic_log_hdr_t hdr;
	uint8 token;
	uint8 reason;
	uint8 pad[2];
} roam_log_btm_query_v3_t;

#define EVENT_LOG_BUFFER_ID_PMK			0
#define EVENT_LOG_BUFFER_ID_ANONCE		1
#define EVENT_LOG_BUFFER_ID_SNONCE		2
#define EVENT_LOG_BUFFER_ID_WPA_M3_KEYDATA	3
#define EVENT_LOG_BUFFER_ID_WPA_CACHED_KEYDATA	4

typedef struct event_log_buffer {
	uint16 id;	/* XTLV ID: EVENT_LOG_XTLV_ID_BUF */
	uint16 len;	/* XTLV Len */
	uint16 buf_id;	/* One of the above EVENT_LOG_BUFFER_ID_XXXs */
	uint16 pad;	/* for 4-byte start alignment of data */
	uint8 data[];	/* the payload of interest */
} event_log_buffer_t;

#define XTLV_EVENT_LOG_BUFFER_LEN		(OFFSETOF(event_log_buffer_t, data))
#define XTLV_EVENT_LOG_BUFFER_FULL_LEN(buf_len)	ALIGN_SIZE((XTLV_EVENT_LOG_BUFFER_LEN + \
							(buf_len) * sizeof(uint8)), sizeof(uint32))

/* Structures for parsing FSM log data
 * Only used by host to parse data coming in FSM log set
 * Following log tags use this structured data:
 * EVENT_LOG_TAG_ASSOC_SM
 * EVENT_LOG_TAG_SUP_SM
 * EVENT_LOG_TAG_AUTH_SM
 * EVENT_LOG_TAG_SAE_SM
 * EVENT_LOG_TAG_FTM_SM
 * EVENT_LOG_TAG_NAN_SM
 * More state machine log tags may also use this format
 */

/* Generic FSM structure for logging. Must be wrapped into a proper structure. The wrapper
 * structure can add more information but this needs to be one of the members of the wrapper
 * structure.
 */
typedef struct event_log_generic_fsm_struct {
	uint32 old_state;
	uint32 new_state;
	uint32 reason;
	uint32 caller;
} event_log_generic_fsm_struct_t;

typedef struct event_log_wl_fsm_struct {
	uint32 unit;
	uint32 bsscfg_idx;
	event_log_generic_fsm_struct_t generic_fsm;
	uint32 data[]; /* Any other information relevant to this state transition */
} event_log_wl_fsm_struct_t;

/* To be used by  DVFS event log FSM logging */
typedef struct event_log_rte_dvfs_fsm_struct {
	event_log_generic_fsm_struct_t generic_fsm;
	uint32 data[];		/* Any other information relevant to this state transition */
} event_log_rte_dvfs_fsm_struct_t;

#define PHY_PERIODIC_LOG_VER7		(7u)
typedef struct phy_periodic_log_v7 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the entire structure */
	phy_periodic_log_cmn_t phy_perilog_cmn;
	phy_periodic_counters_v5_t counters_peri_log;
	phy_periodic_btc_stats_v1_t btc_stats_peri_log;
	/* This will be a variable length based on the numcores field defined above */
	phy_periodic_log_core_t phy_perilog_core[];
} phy_periodic_log_v7_t;

/* Slotted BSS timer reference for RX deafness debug */
#define WLC_SLOTTED_BSS_TIMEREF_VERSION_1	(1u)
typedef struct wlc_slotted_bss_timeref_v1 {
	uint16 version;
	uint8 wlc_unit;	/* WLC unit that triggered generation of this timestamp */
	uint8 idx;	/* Slotted BSS index for which this timestamp is present */
	uint32 nan_timeref_h;	/* NAN reference TSF high */
	uint32 nan_timeref_l;	/* NAN reference TSF low */
	uint32 aw_timeref;	/* aw counter value in AWDL case */
} wlc_slotted_bss_timeref_v1_t;

/* Slotted BSS timer reference for RX deafness debug */
typedef struct wlc_slotted_bss_timeref_values_v1 {
	uint32 nan_timeref_h;	/* NAN reference TSF high */
	uint32 nan_timeref_l;	/* NAN reference TSF low */
	uint32 aw_timeref;	/* aw counter value in AWDL case */
} wlc_slotted_bss_timeref_values_v1_t;

typedef struct wlc_slotted_bss_timeref_v2 {
	uint16 length;  /* Length of the entire structure */
	uint8 wlc_unit;	/* WLC unit that triggered generation of this timestamp */
	uint8  pad;
	wlc_slotted_bss_timeref_values_v1_t timerefs[];
} wlc_slotted_bss_timeref_v2_t;

#define	TRIG_LOG_EVENTS_XTLV_CONTAINER_VERSION_1	(1u)
typedef struct trig_log_events_xtlv_container {
	uint16	version;	/**< see definition of TRIG_LOG_EVENTS_XTLV_CONTAINER_VERSION */
	uint16	len;		/**< length of data including all paddings. */
	uint8   data [];	/**< variable length payload:
				 * 1 or more bcm_xtlv_t type of tuples.
				 * each tuple is padded to multiple of 4 bytes.
				 * 'len' field of this structure includes all paddings.
				 */
} trig_log_events_xtlv_container_v1_t;

/* Bus device HTOD RX dump info. Sent in triggered log events container above */
#define PCIEDEV_HTOD_RX_INFO_VERSION_1		(1u)
typedef struct pciedev_htod_rx_ring_info_v1 {
	uint16 version;
	uint16 g_rxcplist_max;
	uint16 g_rxcplist_avail;
	uint16 htod_rx_rd_idx;
	uint16 htod_rx_wr_idx;
	uint16 dtoh_rxcpl_rd_idx;
	uint16 dtoh_rxcpl_wr_idx;
	uint16 htod_rx_buf_pool_buf_cnt;
	uint16 htod_rx_buf_pool_item_cnt;
	uint16 htod_rx_buf_pool_availcnt;
	uint16 htod_rx_buf_pool_pend_item_cnt;
} pciedev_htod_rx_ring_info_v1_t;

/* WL RX fifo overflow info. Sent in triggered log events container above */
#define WLC_RX_FIFO_DMA_NUM				(3u)
typedef struct wlc_rx_fifo_overflow_info_v1 {
	uint8 unit;
	uint8 rxfifo_bitmap;
	uint8 d3_state;
	uint8 pad;
	uint32 macintstatus;

	/* portions of DMA registers */
	uint32 rx_dma_reg_status0;
	uint32 rx_dma_reg_status1;
	uint32 rx_dma_reg_addrlow;
	uint32 rx_dma_reg_addrhigh;
	uint32 rx_dma_reg_control;
	uint32 rx_dma_reg_ptr;
	uint16 pktpool_avail;
	uint16 pktpool_n_pkts;

	uint32 dma_stall_check_wd_time;
	uint32 rxfifo0ovfl;
	uint32 rxfifo1ovfl;
	uint32 dma_stall_check_rxfifo0ovfl;
	uint32 dma_stall_check_rxfifo1ovfl;

	uint32 rxfill[WLC_RX_FIFO_DMA_NUM];
	uint32 dma_stall_check_rxfill[WLC_RX_FIFO_DMA_NUM];
	uint32 rx_dma_fill_success_time[WLC_RX_FIFO_DMA_NUM];
	uint32 rx_dma_fill_fail_time[WLC_RX_FIFO_DMA_NUM];

	uint32 rxposts[WLC_RX_FIFO_DMA_NUM];
	uint32 dma_stall_check_rxposts[WLC_RX_FIFO_DMA_NUM];
	uint32 rx_dma_posts_success_time[WLC_RX_FIFO_DMA_NUM];

	uint32 rx_dma_desc_count[WLC_RX_FIFO_DMA_NUM];
} wlc_rx_fifo_overflow_info_v1_t;

#define PHY_PERIODIC_LOG_VER8		(8u)
typedef struct phy_periodic_log_v8 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the entire structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v6_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v7_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_v1_t btc_stats_peri_log;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v5_t phy_perilog_core[];
} phy_periodic_log_v8_t;

#define PHY_PERIODIC_LOG_VER9	(9u)
typedef struct phy_periodic_log_v9 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v7_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v8_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_v1_t phy_perilog_btc_stats;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v4_t phy_perilog_core[];
} phy_periodic_log_v9_t;

#define PHY_PERIODIC_LOG_VER10	10u
typedef struct phy_periodic_log_v10 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the entire structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v6_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v7_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_v1_t btc_stats_peri_log;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v5_t phy_perilog_core[2];

	/* log data for smartCCA */
	phy_periodic_scca_stats_v1_t scca_counters_peri_log;
} phy_periodic_log_v10_t;

#define PHY_PERIODIC_LOG_VER11	(11u)
typedef struct phy_periodic_log_v11 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v8_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v8_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_v1_t phy_perilog_btc_stats;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v6_t phy_perilog_core[];
} phy_periodic_log_v11_t;

#define AMT_MATCH_INFRA_BSSID	(1 << 0)
#define AMT_MATCH_INFRA_MYMAC	(1 << 1)

#define PHY_PERIODIC_LOG_VER20	20u
typedef struct phy_periodic_log_v20 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v9_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v8_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_v1_t phy_perilog_btc_stats;

	/* log data for obss/dynbw */
	phy_periodic_obss_stats_v1_t phy_perilog_obss_stats;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v6_t phy_perilog_core[2];

	/* log data for smartCCA */
	phy_periodic_scca_stats_v2_t scca_counters_peri_log;
} phy_periodic_log_v20_t;

#define PHY_PERIODIC_LOG_VER21	21u
typedef struct phy_periodic_log_v21 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v9_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v8_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_v1_t phy_perilog_btc_stats;

	/* log data for obss/dynbw */
	phy_periodic_obss_stats_v1_t phy_perilog_obss_stats;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v6_t phy_perilog_core[2];

	/* log data for smartCCA */
	phy_periodic_scca_stats_v2_t scca_counters_peri_log;
} phy_periodic_log_v21_t;

/* ************************************************** */
/* The version 255 for the logging data structures    */
/* is for use in trunk ONLY. In release branches the  */
/* next available version for the particular data     */
/* structure should be used.                          */
/* ************************************************** */

#define PHY_PERIODIC_LOG_VER255	255u
typedef struct phy_periodic_log_v255 {
	uint8  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_v255_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_v255_t counters_peri_log;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_v255_t phy_perilog_core[2];
} phy_periodic_log_v255_t;

#ifdef EVENT_LOG_ACCESS
/* ********************************************************************************* */
/* These structures defined to allow  version number to use uint16 instead of uint8  */
/* Access phy team will use these struct with different eventlog tagid(2048 onwards) */
/* Mobility team can switch to these structures when they ran out of versions(uint8) */
/* or comfortable to switch.This comment added inside compiler so that mogrify can   */
/* delete this when delivering source file to mobility  customers                    */
/* ********************************************************************************* */
typedef struct wlc_btc_shared_stats_gen2_v1 {
	uint32 bt_req_cnt;		/* #BT antenna requests since last stats sampl */
	uint32 bt_gnt_cnt;		/* #BT antenna grants since last stats sample */
	uint32 bt_gnt_dur;		/* usec BT owns antenna since last stats sample */
	uint16 bt_pm_attempt_cnt;	/* PM1 attempts */
	uint16 bt_succ_pm_protect_cnt;	/* successful PM protection */
	uint16 bt_succ_cts_cnt;		/* successful CTS2A protection */
	uint16 bt_wlan_tx_preempt_cnt;	/* WLAN TX Preemption */
	uint16 bt_wlan_rx_preempt_cnt;	/* WLAN RX Preemption */
	uint16 bt_ap_tx_after_pm_cnt;	/* AP TX even after PM protection */
	uint16 bt_crtpri_cnt;		/* Ant grant by critical BT task */
	uint16 bt_pri_cnt;		/* Ant grant by high BT task */
	uint16 antgrant_lt10ms;		/* Ant grant duration cnt 0~10ms */
	uint16 antgrant_lt30ms;		/* Ant grant duration cnt 10~30ms */
	uint16 antgrant_lt60ms;		/* Ant grant duration cnt 30~60ms */
	uint16 antgrant_ge60ms;		/* Ant grant duration cnt 60~ms */
} wlc_btc_shared_stats_gen2_v1_t;

/* BTCX Statistics for PHY Logging */
typedef struct wlc_btc_stats_phy_logging_gen2_v1 {
	uint16 ver;
	uint16 length;
	uint32 btc_status;		/* btc status log */
	uint32 bt_req_type_map;		/* BT Antenna Req types since last stats sample */
	wlc_btc_shared_stats_gen2_v1_t shared;
} phy_periodic_btc_stats_gen2_v1_t;

typedef struct phy_periodic_counters_gen2_v1 {
	/* RX error related */
	uint32 rxrsptmout;          /* number of response timeouts for transmitted frames
					 * expecting a response
					 */
	uint32 rxcrsglitch;         /* PHY was able to correlate the preamble
					 * but not the header
					 */
	uint32 bphy_badplcp;        /* number of bad PLCP reception on BPHY rate */
	uint32 bphy_rxcrsglitch;    /* PHY count of bphy glitches */
	uint32 rxdropped;           /* Frame dropped */
	uint32 rxnobuf;             /* Rx error due to no buffer */
	uint32 rxrunt;              /* Runt frame counter */
	uint32 rxbadfcs;            /* number of frames for which the CRC check failed
					 * in the MAC
					 */

	/* RX related */
	uint32 rxstrt;              /* number of received frames with a good PLCP */
	uint32 rxbeaconmbss;        /* beacons received from member of BSS */
	uint32 rxdtucastmbss;       /* number of received DATA frames with good FCS
					 * and matching RA
					 */
	uint32 rxdtucastobss;       /* number of received DATA frames with good FCS
					 * but no-matching RA
					 */
	uint32 rxdtocast;           /* number of received DATA frames
					 * (good FCS and no matching RA)
					 */
	uint32 rxctl;               /* Number of control frames */
	uint32 rxaction;            /* Number of action frames */
	uint32 rxback;                     /* Number of block ack frames rcvd */
	uint32 rxctlucast;          /* Number of received unicast ctl frames */
	uint32 rxframe;             /* Number of received frames */

	uint32 rxbar;               /* Number of block ack requests rcvd */
	uint32 rxackucast;          /* number of ucast ACKS received (good FCS) */
	uint32 rxbeaconobss;        /* number of OBSS beacons received */
	uint32 rxctsucast;          /* number of unicast CTS addressed to the MAC (good FCS) */
	uint32 rxrtsucast;          /* number of unicast RTS addressed to the MAC (good FCS) */

	/* TX related */
	uint32 txallfrm;            /* total number of frames sent, incl. Data, ACK, RTS, CTS,
					 * Control Management (includes retransmissions)
					 */
	uint32 txbcnfrm;            /* Numer of transmitted beacons */
	uint32 txmpdu;              /* Numer of transmitted mpdus */
	uint32 txampdu;             /* Numer of transmitted ampdus */
	uint32 txackfrm;            /* number of ACK frames sent out */
	uint32 txrtsfrm;            /* number of RTS sent out by the MAC */
	uint32 txctsfrm;            /* number of CTS sent out by the MAC */

	uint32 txctl;               /* Number of control frames txd */
	uint32 txbar;               /* Number of block ack requests txd */
	uint32 txrts;               /* Number of RTS txd */
	uint32 txback;                     /* Number of block ack frames txd */
	uint32 txucast;             /* number of unicast tx expecting response
					 * other than cts/cwcts
					 */

	/* TX error */
	uint32 txrtsfail;           /* RTS TX failure count */
	uint32 txphyerr;            /* PHY TX error count */

	uint32 last_bcn_seq_num;    /* last beacon seq no. */
	uint32 last_bcn_ltsf;              /* last beacon ltsf */

	uint32 rxanyerr;            /* Any RX error that is not counted by other counters. */

	uint32  rxf0ovfl;           /* Rx FIFO0 overflow counters information */
	uint32  rxf1ovfl;           /* Rx FIFO1 overflow counters information */
	uint32  rxhlovfl;           /* Rx LFIFO overflow counters information */

	uint32 rxbadplcp;           /* number of parity check of the PLCP header failed */
	uint32 pfifo_drop;          /* PHY FIFO was not empty when a new frame arrived */
	uint32 rxnodelim;           /* number of no valid delimiter detected by ampdu parser */
	uint32 rxdrop20s;           /* RX was discarded as the CRS was not seen on
					 * primary channel
					 */
	uint32 new_rxin_plcp_wait_cnt;     /* A new reception started waiting for PLCP bytes
						 * from a previous receive
						 */
	uint32  rxtoolate;          /* receive too late */
	uint32 laterx_cnt;          /* RX frame dropped as it was seen too (30us) late
					 * from the start ofreception
					 */
	uint32 rxfrmtoolong;        /* Number of received frame that are too long */
	uint32 rxfrmtooshrt;        /* RX frame was dropped as it did not meet minimum
					 * number of bytes to be a valid 802.11 frame
					 */

	uint32 rxlegacyfrminvalid;  /* Invalid BPHY or L-OFDM reception */
	uint32 txsifserr;           /* A frame arrived in SIFS while we were about to
					 * transmit
					 * B/ACK
					 */
	uint32 ooseq_macsusp;       /* Ucode is out of sequence in processing reception
					 * (especially due to macsuspend).
					 * RX MEND is seen without RX STRT
					 */
	uint32 desense_reason;      /* desense paraemters to indicate reasons
					 * for bphy and ofdm_desense
					 */
} phy_periodic_counters_gen2_v1_t;

typedef struct phy_periodic_log_cmn_gen2_v1 {
	uint32 nrate;               /* Current Tx nrate */
	uint32 duration;            /* millisecs spent sampling this channel */
	uint32 congest_ibss;        /* millisecs in our bss (presumably this traffic will */
	/*  move if cur bss moves channels) */
	uint32 congest_obss;        /* traffic not in our bss */
	uint32 interference;        /* millisecs detecting a non 802.11 interferer. */
	uint32 last_cal_time;              /* Last cal execution time */

	uint32 noise_cal_req_ts;    /* Time-stamp when noise cal was requested */
	uint32 noise_cal_intr_ts;   /* Time-stamp when noise cal was completed */
	uint32 phywdg_ts;           /* Time-stamp when wd was fired */
	uint32 phywd_dur;           /* Duration of the watchdog */
	uint32 chanspec_set_ts;     /* Time-stamp when chanspec was set */
	uint32 vcopll_failure_cnt;  /* Number of VCO cal failures including */
	/* failures detected in ucode */
	uint32 log_ts;                     /* Time-stamp when this log was collected */

	uint8  phymode;             /* phymode when this log was collected */

	/* glitch  based desense input from cca */
	uint32 cca_stats_total_glitch;
	uint32 cca_stats_bphy_glitch;
	uint32 cca_stats_total_badplcp;
	uint32 cca_stats_bphy_badplcp;
	uint32 cca_stats_mbsstime;

	uint32 counter_noise_request;      /* count of noisecal request */
	uint32 counter_noise_crsbit;       /* count of crs high during noisecal request */
	uint32 counter_noise_apply; /* count of applying noisecal result to crsmin */
	uint32 fullphycalcntr;             /* count of performing single phase cal */

	uint32 macsusp_dur;         /* mac suspend duration */

	uint32 featureflag;         /* Currently active feature flags */

	uint16 chanspec;            /* Current phy chanspec */
	uint16 vbatmeas;            /* Measured VBAT sense value */

	uint16 deaf_count;          /* Depth of stay_in_carrier_search function */

	uint16 ed20_crs;            /* ED-CRS status on all cores */

	uint16 dcc_attempt_counter; /* Number of DC cal attempts */
	uint16 dcc_fail_counter;    /* Number of DC cal failures */

	uint16 btcxovrd_dur;        /* Cumulative btcx overide between WDGs */
	uint16 btcxovrd_err_cnt;    /* BTCX override flagged errors */

	uint16 femtemp_read_fail_counter;  /* Fem temparature read fail counter */
	uint16 phy_log_counter;
	uint16 noise_mmt_overdue;   /* Count up if ucode noise mmt is overdue for 5 sec */
	uint16 chan_switch_tm;             /* Channel switch time */

	uint16 crsmin_pwr_apply_cnt;       /* Count of desense power threshold update to phy */

	uint16 txpustatus;          /* txpu off definations */
	uint16 tempinvalid_count;   /* Count no. of invalid temp. measurements */
	uint16 log_event_id;        /* logging event id */

	uint16 macsusp_cnt;         /* mac suspend counter */

	uint16 throttle_cnt;        /* temp throttling counter */

	uint16 resetcca_cnt;        /* resetcca acphy counter */

	int8   chiptemp;            /* Chip temparature */
	int8   femtemp;             /* Fem temparature */

	uint8  rxchain;             /* Rx Chain */
	uint8  txchain;             /* Tx Chain */
	uint8  ofdm_desense;        /* OFDM desense */

	uint8  dbgfw_ver;           /* version of fw/ucode for debug purposes */
	uint8  bphy_desense;        /* BPHY desense */
	uint8  pll_lockstatus;             /* PLL Lock status */

	/* dccal dcoe & idacc */
	int8  dcc_err;             /* dccal health check error status */

	uint8  dccal_phyrxchain;    /* phy rxchain during dc calibration */
	uint8  dccal_type;          /* DC cal type: single/multi phase, chan change, etc. */

	uint8  gbd_bphy_sleep_counter;     /* gbd sleep counter */
	uint8  gbd_ofdm_sleep_counter;     /* gbd sleep counter */
	uint8  curr_home_channel;   /* gbd input channel from cca */

	/* desense data */
	int8   btcx_mode;           /* btcoex desense mode */
	int8   ltecx_mode;          /* lte coex desense mode */
	uint8  gbd_ofdm_desense;    /* gbd ofdm desense level */
	uint8  gbd_bphy_desense;    /* gbd bphy desense level */
	uint8  current_elna_bypass; /* init gain desense: elna bypass */
	uint8  current_tia_idx;     /* init gain desense: tia index */
	uint8  current_lpf_idx;     /* init gain desense: lpf index */

	int8   weakest_rssi;        /* weakest link RSSI */
	uint8  noise_cal_mode;             /* noisecal mode */

	bool   phycal_disable;             /* Set if calibration is disabled */
	bool   hwpwrctrlen;         /* tx hwpwrctrl enable */
	int8   ed_threshold;        /* Threshold applied for ED */
	uint16 ed_crs_status;              /* Status of ED and CRS during noise cal */
	uint16 preempt_status1;     /* status of preemption */
	uint16 preempt_status2;     /* status of preemption */
	uint16 preempt_status3;     /* status of preemption */
	uint16 preempt_status4;     /* status of preemption */
	uint32 ed_duration;         /* ccastats: ed_duration */

} phy_periodic_log_cmn_gen2_v1_t;

typedef struct phy_periodic_log_core_gen2_v1 {
	/* dccal dcoe & idacc */
	uint16 dcoe_done_0;  /* dccal control register 44 */
	uint16 dcoe_done_1;  /* dccal control register 45 */
	uint16 dcoe_done_2;  /* dccal control register 46 */
	uint16 idacc_done_0; /* dccal control register 21 */
	uint16 idacc_done_1; /* dccal control register 60 */
	uint8  dcoe_num_tries;             /* number of retries on dcoe cal */
	uint8  idacc_num_tries;     /* number of retries on idac cal */
	int16  txcap;        /* Txcap value */

	uint8  pktproc;      /* pktproc read during dccal health check */
	uint8  baseindxval;  /* TPC Base index */
	int8   tgt_pwr;      /* Programmed Target power */
	int8   estpwradj;    /* Current Est Power Adjust value */
	int8   crsmin_pwr;          /* CRS Min/Noise power */
	int8   rssi_per_ant; /* RSSI Per antenna */
	int8   snr_per_ant;  /* SNR Per antenna */

	int8   noise_level;  /* noise pwr after filtering & averageing */
	int8   noise_level_inst;    /* instantaneous noise cal pwr */
	int8   estpwr;              /* tx powerDet value */
	int8   crsmin_th_idx;       /* idx used to lookup crs min thresholds */

	int8   phy_noise_pwr_array[PHY_NOISE_PWR_ARRAY_SIZE];   /* noise buffer array */
} phy_periodic_log_core_gen2_v1_t;

typedef struct phy_periodic_log_gen2_v1 {
	uint16  version;		/* Logging structure version */
	uint8  numcores;	/* Number of cores for which core specific data present */
	uint16 length;		/* Length of the structure */

	/* Logs general PHY parameters */
	phy_periodic_log_cmn_gen2_v1_t phy_perilog_cmn;

	/* Logs ucode counters and NAVs */
	phy_periodic_counters_gen2_v1_t counters_peri_log;

	/* log data for BTcoex */
	phy_periodic_btc_stats_gen2_v1_t phy_perilog_btc_stats;

	/* Logs data pertaining to each core */
	phy_periodic_log_core_gen2_v1_t phy_perilog_core[4];
} phy_periodic_log_gen2_v1_t;

typedef struct phycal_log_cmn_gen2_v1 {
	uint16 chanspec;     /* current phy chanspec */
	uint8  reason;              /* cal reason */
	uint32 time;         /* time at which cal happened in sec */
	uint16 temp;         /* temperature at the time of cal */
	uint16 dur;          /* duration of cal in usec */

	uint8  rxgtbl_en;  /* Rx IQ Cal gain-table based */

	uint8  rcal_value;  /* rcal value */
	uint16 rccal_gmult;  /* rccal gmult value */
	uint16 rccal_cmult;  /* rccal cmult value */
} phycal_log_cmn_gen2_v1_t;

typedef struct phycal_log_core_gen2_v1 {
	uint16 ofdm_txa; /* OFDM Tx IQ Cal a coeff */
	uint16 ofdm_txb; /* OFDM Tx IQ Cal b coeff */
	uint16 ofdm_txd; /* contain di & dq */
	uint16 bphy_txa; /* BPHY Tx IQ Cal a coeff */
	uint16 bphy_txb; /* BPHY Tx IQ Cal b coeff */
	uint16 bphy_txd; /* contain di & dq */

	uint16 rxa; /* Rx IQ Cal A coeffecient */
	uint16 rxb; /* Rx IQ Cal B coeffecient */
	int32 rxs;  /* FDIQ Slope coeffecient */

	uint8 baseidx; /* TPC Base index */
	uint8 adc_coeff_cap0_adc0_I; /* ADC-0 CAP Cal Cap0 I */
	uint8 adc_coeff_cap1_adc0_I; /* ADC-0 CAP Cal Cap1 I */
	uint8 adc_coeff_cap2_adc0_I; /* ADC-0 CAP Cal Cap2 I */
	uint8 adc_coeff_cap0_adc0_Q; /* ADC-0 CAP Cal Cap0 Q */
	uint8 adc_coeff_cap1_adc0_Q; /* ADC-0 CAP Cal Cap1 Q */
	uint8 adc_coeff_cap2_adc0_Q; /* ADC-0 CAP Cal Cap2 Q */
	uint8 adc_coeff_cap0_adc1_I; /* ADC-1 CAP Cal Cap0 I */
	uint8 adc_coeff_cap1_adc1_I; /* ADC-1 CAP Cal Cap1 I */
	uint8 adc_coeff_cap2_adc1_I; /* ADC-1 CAP Cal Cap2 I */
	uint8 adc_coeff_cap0_adc1_Q; /* ADC-1 CAP Cal Cap0 Q */
	uint8 adc_coeff_cap1_adc1_Q; /* ADC-1 CAP Cal Cap1 Q */
	uint8 adc_coeff_cap2_adc1_Q; /* ADC-1 CAP Cal Cap2 Q */
	uint8 pad; /* Padding byte to align with word */
} phycal_log_core_gen2_v1_t;

typedef struct phycal_log_gen2_v1 {
	uint16  version; /* Logging structure version */
	uint8  numcores; /* Number of cores for which core specific data present */
	uint16 length;  /* Length of the entire structure */
	phycal_log_cmn_gen2_v1_t phycal_log_cmn; /* Logging common structure */
	/* This will be a variable length based on the numcores
	 * field defined above
	 */
	phycal_log_core_gen2_v1_t phycal_log_core[4];
} phycal_log_gen2_v1_t;

#endif /* EVENT_LOG_ACCESS */

#endif /* _EVENT_LOG_PAYLOAD_H_ */
