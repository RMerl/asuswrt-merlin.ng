/*
 * FTM module IOCTL structure definitions.
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
 * <<Broadcom-WL-IPTag/Dual:>>
 *
 * $Id$
 */

#ifndef _ftm_ioctl_h
#define _ftm_ioctl_h

#include <typedefs.h>

typedef uint16		wl_ftm_session_id_t;
typedef int16		wl_ftm_result_flags_t;
typedef int16		wl_ftm_method_t;
#ifndef BCMUTILS_ERR_CODES
typedef int32		wl_ftm_status_t;
#endif /* BCMUTILS_ERR_CODES  */

/** 11az ftm types */
enum wl_ftm_type {
	WL_FTM_TYPE_NONE	= 0, /* ftm type unspecified */
	WL_FTM_TYPE_MC		= 1, /* Legacy MC ftm */
	WL_FTM_TYPE_TB		= 2, /* 11az Trigger based */
	WL_FTM_TYPE_NTB		= 3, /* 11az Non-trigger based */
	WL_FTM_TYPE_MAX
};
typedef uint8 wl_ftm_type_t;

/** session flags for 11AZ */

/** global and method configuration flags */
enum wl_ftm_flags {
	WL_FTM_FLAG_NONE			= 0x00000000,
	WL_FTM_FLAG_RX_ENABLED			= 0x00000001, /* respond to requests, per bss */
	WL_FTM_FLAG_RX_RANGE_REQ		= 0x00000002, /* 11mc range requests enabled */
	WL_FTM_FLAG_TX_LCI			= 0x00000004, /* tx lci, if known */
	WL_FTM_FLAG_TX_CIVIC			= 0x00000008, /* tx civic, if known */
	WL_FTM_FLAG_RX_AUTO_BURST		= 0x00000010, /* auto respond w/o host action */
	WL_FTM_FLAG_TX_AUTO_BURST		= 0x00000020, /* continue tx w/o host action */
	WL_FTM_FLAG_TX_ENABLED			= 0x00000040, /* per bss */
	WL_FTM_FLAG_RESERVED			= 0x00000080, /* reserve. obsolete sched w/ avail */
	WL_FTM_FLAG_ASAP_CAPABLE		= 0x00000100, /* ASAP capable */
	WL_FTM_FLAG_MBURST_FOLLOWUP		= 0x00000200, /* new multi-burst algorithm */
	WL_FTM_FLAG_SECURE			= 0x00000400, /* per bsscfg option */
	WL_FTM_FLAG_NO_TSF_SYNC			= 0x00000800, /* disable tsf sync, per bss */
	WL_FTM_FLAG_AVB_TS			= 0x00001000, /* Force AVB TimeStamping */
	WL_FTM_FLAG_PHYTS_BME			= 0x00002000, /* use BME for sampcap copy */
	WL_FTM_FLAG_ALL				= 0xffffffff
};
typedef uint32	wl_ftm_flags_t;

/* mask for per bss global configuration flags */
#define WL_FTM_FLAG_BSS_MASK \
	(WL_FTM_FLAG_RX_ENABLED \
	| WL_FTM_FLAG_TX_ENABLED \
	| WL_FTM_FLAG_SECURE \
	| WL_FTM_FLAG_NO_TSF_SYNC)

/** session flags */
#define	WL_FTM_SESSION_FLAG_NONE		0x0000000000000000llu	/* no flags */
#define WL_FTM_SESSION_FLAG_INITIATOR		0x0000000000000001llu	/* local is initiator */
#define WL_FTM_SESSION_FLAG_TARGET		0x0000000000000002llu	/* local is target */
#define WL_FTM_SESSION_FLAG_CORE_ROTATE		0x0000000000000004llu	/* initiator core rotate */
#define WL_FTM_SESSION_FLAG_AUTO_BURST		0x0000000000000008llu	/* rx_auto_burst */
#define WL_FTM_SESSION_FLAG_PERSIST		0x0000000000000010llu	/* good until cancelled */
#define WL_FTM_SESSION_FLAG_RTT_DETAIL		0x0000000000000020llu	/* rtt detail results */
#define WL_FTM_SESSION_FLAG_SECURE		0x0000000000000040llu	/* session is secure */
#define WL_FTM_SESSION_FLAG_AOA			0x0000000000000080llu	/* AOA along w/ RTT */
#define WL_FTM_SESSION_FLAG_SEC_LTF_SUPPORTED	0x0000000000000100llu	/* 11AZ sec LTF supported */
#define WL_FTM_SESSION_FLAG_SEC_LTF_REQUIRED	0x0000000000000200llu	/* 11AZ sec LTF required */
#define WL_FTM_SESSION_FLAG_HEACK		0x0000000000000400llu	/* HEACK BSS */
#define WL_FTM_SESSION_FLAG_ASAP_CAPABLE	0x0000000000000800llu	/* ASAP-capable */
#define WL_FTM_SESSION_FLAG_RANDMAC		0x0000000000001000llu	/* use random mac */
#define WL_FTM_SESSION_FLAG_REPORT_FAILURE	0x0000000000002000llu	/* failure to target */
#define WL_FTM_SESSION_FLAG_INITIATOR_RPT	0x0000000000004000llu	/* distance to target */
#define WL_FTM_SESSION_FLAG_NOCHANSWT		0x0000000000008000llu	/* reserved in comp ? */
#define WL_FTM_SESSION_FLAG_RSVD1		0x0000000000010000llu	/* reserved */
#define WL_FTM_SESSION_FLAG_SEQ_EN		0x0000000000020000llu	/* Toast */
#define WL_FTM_SESSION_FLAG_NO_PARAM_OVRD	0x0000000000040000llu	/* no override from tgt */
#define WL_FTM_SESSION_FLAG_ASAP		0x0000000000080000llu	/* ASAP session */
#define WL_FTM_SESSION_FLAG_REQ_LCI		0x0000000000100000llu	/* tx LCI req */
#define WL_FTM_SESSION_FLAG_REQ_CIV		0x0000000000200000llu	/* tx civic loc req */
#define WL_FTM_SESSION_FLAG_PRE_SCAN		0x0000000000400000llu	/* pre-scan for asap=1 */
#define WL_FTM_SESSION_FLAG_AUTO_VHTACK		0x0000000000800000llu	/* vhtack based on brcmie */
#define WL_FTM_SESSION_FLAG_VHTACK		0x0000000001000000llu	/* vht ack is in use */
#define WL_FTM_SESSION_FLAG_BDUR_NOPREF		0x0000000002000000llu	/* burst-duration no pref */
#define WL_FTM_SESSION_FLAG_NUM_FTM_NOPREF	0x0000000004000000llu	/* num of FTM: no pref */
#define WL_FTM_SESSION_FLAG_FTM_SEP_NOPREF	0x0000000008000000llu	/* time btw FTM: no pref */
#define WL_FTM_SESSION_FLAG_NUM_BURST_NOPREF	0x0000000010000000llu	/* num of bursts: no pref */
#define WL_FTM_SESSION_FLAG_BURST_PERIOD_NOPREF	0x0000000020000000llu	/* burst period: no pref */
#define WL_FTM_SESSION_FLAG_MBURST_FOLLOWUP	0x0000000040000000llu	/* new mburst algo */
#define WL_FTM_SESSION_FLAG_MBURST_NODELAY	0x0000000080000000llu	/* good until cancelled */
#define WL_FTM_SESSION_FLAG_FULL_BW		0x0000000100000000llu	/* use all bandwidth */
#define WL_FTM_SESSION_FLAG_R2I_TOA_PHASE_SHIFT	0x0000000200000000llu	/* phase shft average toa */
#define WL_FTM_SESSION_FLAG_I2R_TOA_PHASE_SHIFT	0x0000000400000000llu	/* phase shft average toa */
#define WL_FTM_SESSION_FLAG_I2R_IMMEDIATE_RPT	0x0000000800000000llu	/* immediate I2R feedback */
#define WL_FTM_SESSION_FLAG_R2I_IMMEDIATE_RPT	0x0000001000000000llu	/* immediate R2R report */
#define WL_FTM_SESSION_FLAG_DEV_CLASS_A		0x0000002000000000llu	/* class A device */
#define WL_FTM_SESSION_FLAG_RX_ENABLED		0x0000004000000000llu	/* respond to requests */
#define WL_FTM_SESSION_FLAG_I2R_LMR_POLICY	0x0000008000000000llu	/* I2R LMR policy */
#define WL_FTM_SESSION_FLAG_TX_LCI		0x0000010000000000llu	/* tx lci, if known */
#define WL_FTM_SESSION_FLAG_TX_CIVIC		0x0000020000000000llu	/* tx civic, if known */
#define WL_FTM_SESSION_FLAG_I2R_REQ_ACCEPT	0x0000040000000000llu	/* ISTA flag to honor
									 * i2r request from RSTA
									 * even when its LMR
									 * feedback is false.
									 */
#define WL_FTM_SESSION_FLAG_RNM_MFP_REQ		0x0000080000000000llu	/* Ranging mgmt/meas frame
									 * protection required
									 */
#define WL_FTM_SESSION_FLAG_TX_HE_LMR		0x0000100000000000llu	/* TX LMR with HE rate */
#define WL_FTM_SESSION_FLAG_NO_TSF_SYNC		0x0000200000000000llu	/* disable tsf sync */
#define WL_FTM_SESSION_FLAG_CONT_ON_BURST_ERR	0x0000400000000000llu	/* continue to subsequent
									 * burst(s) on error.
									 */
#define WL_FTM_SESSION_FLAG_PASSIVE_TB_RANGING	0x0000800000000000llu	/* Passive TB ranging */
#define WL_FTM_SESSION_FLAG_ONE_WAY		0x0001000000000000llu	/* ONE_WAY RTT */

#define WL_FTM_SESSION_FLAG_ALL			0xffffffffffffffffllu

typedef uint64 wl_ftm_session_flags_t;

/* alias */
#define WL_FTM_SESSION_FLAG_I2R_RPT WL_FTM_SESSION_FLAG_INITIATOR_RPT

/* flags common across mc/ntb/tb.
 * Explicit for the ones that are currently used.
 * Currently not used ones still reserve their bits in above.
 */
#define FTM_COMMON_CONFIG_MASK \
	(WL_FTM_SESSION_FLAG_INITIATOR \
	| WL_FTM_SESSION_FLAG_TARGET \
	| WL_FTM_SESSION_FLAG_PRE_SCAN \
	| WL_FTM_SESSION_FLAG_REQ_LCI \
	| WL_FTM_SESSION_FLAG_REQ_CIV \
	| WL_FTM_SESSION_FLAG_RTT_DETAIL \
	| WL_FTM_SESSION_FLAG_NO_PARAM_OVRD \
	| WL_FTM_SESSION_FLAG_AUTO_BURST \
	| WL_FTM_SESSION_FLAG_NO_TSF_SYNC \
	| WL_FTM_SESSION_FLAG_CONT_ON_BURST_ERR)

/* flags relevant to MC sessions */
#define FTM_MC_CONFIG_MASK \
	(WL_FTM_SESSION_FLAG_AUTO_VHTACK \
	| WL_FTM_SESSION_FLAG_INITIATOR_RPT \
	| WL_FTM_SESSION_FLAG_MBURST_NODELAY \
	| WL_FTM_SESSION_FLAG_ASAP_CAPABLE \
	| WL_FTM_SESSION_FLAG_ASAP \
	| WL_FTM_SESSION_FLAG_PRE_SCAN \
	| WL_FTM_SESSION_FLAG_CORE_ROTATE \
	| WL_FTM_SESSION_FLAG_VHTACK \
	| WL_FTM_SESSION_FLAG_HEACK \
	| WL_FTM_SESSION_FLAG_BDUR_NOPREF \
	| WL_FTM_SESSION_FLAG_NUM_FTM_NOPREF \
	| WL_FTM_SESSION_FLAG_FTM_SEP_NOPREF \
	| WL_FTM_SESSION_FLAG_NUM_BURST_NOPREF \
	| WL_FTM_SESSION_FLAG_BURST_PERIOD_NOPREF \
	| WL_FTM_SESSION_FLAG_NOCHANSWT \
	| WL_FTM_SESSION_FLAG_MBURST_FOLLOWUP \
	| WL_FTM_SESSION_FLAG_ONE_WAY)

/* flags common for TB/NTB sessions */
#define FTM_TB_NTB_COMMON_CONFIG_MASK \
	(WL_FTM_SESSION_FLAG_INITIATOR_RPT \
	| WL_FTM_SESSION_FLAG_R2I_TOA_PHASE_SHIFT \
	| WL_FTM_SESSION_FLAG_I2R_TOA_PHASE_SHIFT \
	| WL_FTM_SESSION_FLAG_I2R_IMMEDIATE_RPT \
	| WL_FTM_SESSION_FLAG_R2I_IMMEDIATE_RPT \
	| WL_FTM_SESSION_FLAG_I2R_LMR_POLICY \
	| WL_FTM_SESSION_FLAG_I2R_REQ_ACCEPT \
	| WL_FTM_SESSION_FLAG_RNM_MFP_REQ \
	| WL_FTM_SESSION_FLAG_SECURE \
	| WL_FTM_SESSION_FLAG_SEC_LTF_SUPPORTED \
	| WL_FTM_SESSION_FLAG_SEC_LTF_REQUIRED \
	| WL_FTM_SESSION_FLAG_TX_HE_LMR)

/* flags relevant to NTB sessions */
#define FTM_NTB_CONFIG_MASK	FTM_TB_NTB_COMMON_CONFIG_MASK

/* flags relevant to TB sessions. */
#define FTM_TB_CONFIG_MASK \
	(FTM_TB_NTB_COMMON_CONFIG_MASK \
	| WL_FTM_SESSION_FLAG_FULL_BW \
	| WL_FTM_SESSION_FLAG_DEV_CLASS_A \
	| WL_FTM_SESSION_FLAG_PASSIVE_TB_RANGING)

typedef uint64 wl_ftm_session_mask_t;

/* wl_ftm_ftm_session_status_t core_info stores the following local core info. */
#define WL_FTM_SESSION_STATUS_CORE_INFO_CORE_SHIFT		8u
#define WL_FTM_SESSION_STATUS_CORE_INFO_CORE_MASK		0xff00u
#define WL_FTM_SESSION_STATUS_CORE_INFO_NUM_CORE_SHIFT		0u
#define WL_FTM_SESSION_STATUS_CORE_INFO_NUM_CORE_MASK		0x00ffu

/** time units - mc supports up to 0.1ns resolution */
enum wl_ftm_tmu {
	WL_FTM_TMU_TU			= 0,	/**< 1024us */
	WL_FTM_TMU_SEC			= 1,
	WL_FTM_TMU_MILLI_SEC		= 2,
	WL_FTM_TMU_MICRO_SEC		= 3,
	WL_FTM_TMU_NANO_SEC		= 4,
	WL_FTM_TMU_PICO_SEC		= 5
};
typedef int16 wl_ftm_tmu_t;

typedef struct wl_ftm_intvl {
	uint32 intvl;
	wl_ftm_tmu_t tmu;
	uint8	pad[2];
} wl_ftm_intvl_t;

/** commands that can apply to proxd, method or a session */
enum wl_ftm_cmd {
	WL_FTM_CMD_NONE			= 0,
	WL_FTM_CMD_GET_VERSION		= 1,
	WL_FTM_CMD_ENABLE		= 2,
	WL_FTM_CMD_DISABLE		= 3,
	WL_FTM_CMD_CONFIG		= 4,
	WL_FTM_CMD_START_SESSION	= 5,
	WL_FTM_CMD_BURST_REQUEST	= 6,
	WL_FTM_CMD_STOP_SESSION		= 7,
	WL_FTM_CMD_DELETE_SESSION	= 8,
	WL_FTM_CMD_GET_RESULT		= 9,
	WL_FTM_CMD_GET_INFO		= 10,
	WL_FTM_CMD_GET_STATUS		= 11,
	WL_FTM_CMD_GET_SESSIONS		= 12,
	WL_FTM_CMD_GET_COUNTERS		= 13,
	WL_FTM_CMD_CLEAR_COUNTERS	= 14,
	WL_FTM_CMD_COLLECT		= 15,	/* not supported, see 'wl proxd_collect' */
	WL_FTM_CMD_TUNE			= 16,	/* not supported, see 'wl proxd_tune' */
	WL_FTM_CMD_DUMP			= 17,
	WL_FTM_CMD_START_RANGING	= 18,
	WL_FTM_CMD_STOP_RANGING		= 19,
	WL_FTM_CMD_GET_RANGING_INFO	= 20,
	WL_FTM_CMD_IS_TLV_SUPPORTED	= 21,
	WL_FTM_CMD_TEST_MODE		= 22,	/* for test mode (i.e. WFA CTT) */
	WL_FTM_CMD_DBG_EVENT_CHECK	= 23,	/* for debugging */

	WL_FTM_CMD_MAX
};
typedef int16 wl_ftm_cmd_t;

enum wl_ftm_event_type {
	WL_FTM_EVENT_NONE			= 0,	/* reserved */
	WL_FTM_EVENT_SESSION_CREATE		= 1,
	WL_FTM_EVENT_RES_WAIT			= 2,	/* wait for resources (SESSION_START) */
	WL_FTM_EVENT_FTM_REQ			= 3,
	WL_FTM_EVENT_BURST_START		= 4,
	WL_FTM_EVENT_BURST_END			= 5,
	WL_FTM_EVENT_SESSION_END		= 6,
	WL_FTM_EVENT_SESSION_RESET		= 7,
	WL_FTM_EVENT_BURST_RESCHED		= 8,	/* burst rescheduled-e.g. partial TSF */
	WL_FTM_EVENT_SESSION_DESTROY		= 9,
	WL_FTM_EVENT_RANGE_REQ			= 10,
	WL_FTM_EVENT_FTM_FRAME			= 11,
	WL_FTM_EVENT_USER_WAIT			= 12,	/* was DELAY */
	WL_FTM_EVENT_VS_INITIATOR_RPT		= 13,	/* target rx initiator-report */
	WL_FTM_EVENT_RANGING			= 14,
	WL_FTM_EVENT_LCI_MEAS_REP		= 15,	/* LCI measurement report */
	WL_FTM_EVENT_CIVIC_MEAS_REP		= 16,	/* civic measurement report */
	WL_FTM_EVENT_COLLECT			= 17,
	/*
	 * WL_FTM_EVENT_START_WAIT(18) is reserved
	 */
	WL_FTM_EVENT_MF_STATS			= 19,	/* mf stats event */
	WL_FTM_EVENT_RESERVED20			= 20,	/* reserved */
	WL_FTM_EVENT_SESSION_READY		= 21,	/* ready for burst */
	WL_FTM_EVENT_CSI_DUMP			= 22,	/* dump RAW CSI data for debug */
	WL_FTM_EVENT_SESSION_STOPPING		= 23,
	WL_FTM_EVENT_LMR_FRAME			= 24
};
typedef uint16 wl_ftm_event_type_t;

#define WL_FTM_EVENT_MASK_ALL			0xfffffffe
#define WL_FTM_EVENT_ENABLED(_mask, _event_type) \
	(((_mask) & (1u << (_event_type))) != 0)

/** FTM session states */
enum wl_ftm_session_state {
	WL_FTM_SESSION_STATE_NONE		= 0,
	WL_FTM_SESSION_STATE_CREATED		= 1,
	WL_FTM_SESSION_STATE_CONFIGURED		= 2,
	WL_FTM_SESSION_STATE_RSVD		= 3,
	WL_FTM_SESSION_STATE_RES_WAIT		= 4,
	WL_FTM_SESSION_STATE_USER_WAIT		= 5,
	WL_FTM_SESSION_STATE_RSVD2		= 6,
	WL_FTM_SESSION_STATE_BURST		= 7,
	WL_FTM_SESSION_STATE_STOPPING		= 8,
	WL_FTM_SESSION_STATE_ENDED		= 9,
	WL_FTM_SESSION_STATE_RSVD3		= 10,
	WL_FTM_SESSION_STATE_READY		= 11,
	WL_FTM_SESSION_STATE_DESTROYING		= -2,
	WL_FTM_SESSION_STATE_INVALID		= -1
};
typedef int16 wl_ftm_session_state_t;

/* FTM TLV IDs. Mirror of existing legacy PROXD with added entries for
 * new 11AZ ranging parameters.
 */
typedef enum {
	WL_FTM_TLV_ID_NONE			= 0,
	WL_FTM_TLV_ID_METHOD			= 1,
	WL_FTM_TLV_ID_FLAGS			= 2,
	WL_FTM_TLV_ID_CHANSPEC			= 3,	/**< note: uint32 */
	WL_FTM_TLV_ID_TX_POWER			= 4,
	WL_FTM_TLV_ID_RATESPEC			= 5,
	WL_FTM_TLV_ID_BURST_DURATION		= 6,	/**< intvl - length of burst */
	WL_FTM_TLV_ID_BURST_PERIOD		= 7,	/**< intvl - between bursts */
	WL_FTM_TLV_ID_BURST_FTM_SEP		= 8,	/**< intvl - between FTMs */
	WL_FTM_TLV_ID_BURST_NUM_MEAS		= 9,	/**< uint16 - per burst */
	WL_FTM_TLV_ID_NUM_BURST			= 10,	/**< uint16 */
	WL_FTM_TLV_ID_FTM_RETRIES		= 11,	/**< uint16 at FTM level */
	WL_FTM_TLV_ID_BSS_INDEX			= 12,	/**< uint8 */
	WL_FTM_TLV_ID_BSSID			= 13,
	WL_FTM_TLV_ID_INIT_DELAY		= 14,	/**< intvl - optional,non-standalone only */
	WL_FTM_TLV_ID_BURST_TIMEOUT		= 15,	/**< expect response within - intvl */
	WL_FTM_TLV_ID_EVENT_MASK		= 16,	/**< interested events - in/out */
	WL_FTM_TLV_ID_FLAGS_MASK		= 17,	/**< interested flags - in only */
	WL_FTM_TLV_ID_PEER_MAC			= 18,	/**< mac address of peer */
	WL_FTM_TLV_ID_FTM_REQ			= 19,	/**< dot11_ftm_req */
	WL_FTM_TLV_ID_LCI_REQ			= 20,
	WL_FTM_TLV_ID_LCI			= 21,
	WL_FTM_TLV_ID_CIVIC_REQ			= 22,
	WL_FTM_TLV_ID_CIVIC			= 23,
	WL_FTM_TLV_ID_AVAIL24			= 24,	/**< ROM compatibility */
	WL_FTM_TLV_ID_SESSION_FLAGS		= 25,	/**< 64 bits */
	WL_FTM_TLV_ID_SESSION_FLAGS_MASK	= 26,	/**< 64 bits. in only */
	WL_FTM_TLV_ID_RX_MAX_BURST		= 27,	/**< uint16 - limit bursts per session */
	WL_FTM_TLV_ID_RANGING_INFO		= 28,	/**< ranging info */
	WL_FTM_TLV_ID_RANGING_FLAGS		= 29,	/**< uint16 */
	WL_FTM_TLV_ID_RANGING_FLAGS_MASK	= 30,	/**< uint16, in only */
	WL_FTM_TLV_ID_NAN_MAP_ID		= 31,
	WL_FTM_TLV_ID_DEV_ADDR			= 32,
	WL_FTM_TLV_ID_AVAIL			= 33,	/**< wl_proxd_avail_t */
	WL_FTM_TLV_ID_TLV_ID			= 34,	/* uint16 tlv-id */
	WL_FTM_TLV_ID_FTM_REQ_RETRIES		= 35,	/* uint16 FTM request retries */
	WL_FTM_TLV_ID_TPK			= 36,	/* 32byte TPK */
	WL_FTM_TLV_ID_RI_RR			= 36,	/* RI_RR */
	WL_FTM_TLV_ID_TUNE			= 37,	/* wl_proxd_pararms_tof_tune_t */
	WL_FTM_TLV_ID_CUR_ETHER_ADDR		= 38,	/* Source Address used for Tx */

	/* TB and NTB TLVs */
	WL_FTM_TLV_ID_MIN_DELTA			= 39,	/* Minimum time between NDPAs */
	WL_FTM_TLV_ID_MAX_DELTA			= 40,	/* Maximum time between NDPAs */
	WL_FTM_TLV_ID_MAX_I2R_REP		= 41,	/* Maximum repetitions in I2R NDP */
	WL_FTM_TLV_ID_MAX_R2I_REP		= 42,	/* Maximum repetitions in R2I NDP */
	WL_FTM_TLV_ID_MAX_I2R_STS_LEQ_80	= 43,	/* Maximum streams <= 80mhz in I2R NDP */
	WL_FTM_TLV_ID_MAX_R2I_STS_LEQ_80	= 44,	/* Maximum streams <= 80mhz in R2I NDP */
	WL_FTM_TLV_ID_MAX_I2R_STS_GT_80		= 45,	/* Maximum streams > 80mhz in I2R NDP */
	WL_FTM_TLV_ID_MAX_R2I_STS_GT_80		= 46,	/* Maximum streams > 80mhz in R2I NDP */
	WL_FTM_TLV_ID_FORMAT_BW			= 47,	/* Format and bandwidth */
	WL_FTM_TLV_ID_MAX_I2R_TOTAL_LTF		= 48,	/* Max LFT number in all I2R NDP streams */
	WL_FTM_TLV_ID_MAX_R2I_TOTAL_LTF		= 49,	/* Max LFT number in all R2I NDP streams */
	WL_FTM_TLV_ID_LMR_FRAME			= 50,	/* dot11_ftm_lmr_t */
	WL_FTM_TLV_ID_TF_MAC_PAD_DUR		= 51,	/* Trigger Frame MAC padding duration */
	WL_FTM_TLV_ID_TB_ISTA_AW		= 52,	/* TB ISTA Availabilty Window */
	WL_FTM_TLV_ID_AZ_NDP_MCS		= 53,	/* MCS index of HE Ranging NDP frame */

	/* Extra common TLV IDs */
	WL_FTM_TLV_ID_TYPE_FLAGS		= 128,

	/* output - 512 + x */
	WL_FTM_TLV_ID_STATUS			= 512,
	WL_FTM_TLV_ID_COUNTERS_V1		= 513,	/* wl_proxd_counters_t */
	WL_FTM_TLV_ID_INFO			= 514,
	WL_FTM_TLV_ID_RTT_RESULT		= 515,
	WL_FTM_TLV_ID_AOA_RESULT		= 516,
	WL_FTM_TLV_ID_SESSION_INFO		= 517,
	WL_FTM_TLV_ID_SESSION_STATUS		= 518,
	WL_FTM_TLV_ID_SESSION_ID_LIST		= 519,
	WL_FTM_TLV_ID_RTT_RESULT_V2		= 520,
	WL_FTM_TLV_ID_RTT_RESULT_V3		= 521,
	WL_FTM_TLV_ID_COUNTERS_V2		= 522,	/* wl_ftm_counters_v2_t */
	WL_FTM_TLV_ID_AZ_RTT_SAMPLE_V1		= 523,	/* wl_ftm_az_rtt_sample_v1_t */
	WL_FTM_TLV_ID_AZ_RTT_RESULT_V1		= 524,	/* wl_ftm_az_rtt_result_v1_t */
	WL_FTM_TLV_ID_AZ_COUNTERS_V1		= 525,	/* wl_ftm_az_counters_v1_t */
	WL_FTM_TLV_ID_HAL_COUNTERS_V1		= 526,	/* wl_ftm_hal_counters_v1_t */

	/* debug tlvs can be added starting 1024 */
	WL_FTM_TLV_ID_DEBUG_MASK		= 1024,
	WL_FTM_TLV_ID_COLLECT			= 1025,	/**< output only */
	WL_FTM_TLV_ID_STRBUF			= 1026,

	WL_FTM_TLV_ID_COLLECT_HEADER		= 1025,	/* wl_proxd_collect_header_t */
	WL_FTM_TLV_ID_COLLECT_INFO		= 1028,	/* wl_proxd_collect_info_t */
	WL_FTM_TLV_ID_COLLECT_DATA		= 1029,	/* wl_proxd_collect_data_t */
	WL_FTM_TLV_ID_COLLECT_CHAN_DATA		= 1030,	/* wl_proxd_collect_data_t */
	WL_FTM_TLV_ID_MF_STATS_DATA		= 1031,	/* mf_stats_buffer */

	WL_FTM_TLV_ID_COLLECT_INLINE_HEADER	= 1032,
	WL_FTM_TLV_ID_COLLECT_INLINE_FRAME_INFO	= 1033,
	WL_FTM_TLV_ID_COLLECT_INLINE_FRAME_DATA	= 1034,
	WL_FTM_TLV_ID_COLLECT_INLINE_RESULTS	= 1035,

	WL_FTM_TLV_ID_TEST_MODE			= 1036,	/* wl_ftm_test_mode_t */
	WL_FTM_TLV_ID_CSI_DUMP			= 1037,	/* wl_ftm_csi_dump_t */
	WL_FTM_TLV_ID_CSI_RANGING_PARAMS	= 1038,	/* wl_ftm_csi_ranging_params_t */
	WL_FTM_TLV_ID_CSI_PROC_RESULT		= 1039,	/* wl_ftm_csi_proc_result_t */
	WL_FTM_TLV_ID_CSI_PROC_RTT		= 1040	/* wl_ftm_csi_proc_rtt_t */
} wl_ftm_tlv_types_t;

enum wl_ftm_wait_reason {
	WL_FTM_WAIT_NONE	= 0x0000,
	WL_FTM_WAIT_KEY		= 0x0001,
	WL_FTM_WAIT_SCHED	= 0x0002,
	WL_FTM_WAIT_TSF		= 0x0004
};
typedef uint16 wl_ftm_wait_reason_t;

/** global stats */
typedef struct wl_ftm_counters_v2 {
	uint32 tx;			/* tx frame count */
	uint32 rx;			/* rx frame count */
	uint32 burst;			/* total number of burst */
	uint32 sessions;		/* total number of sessions */
	uint32 max_sessions;		/* max concurrency */
	uint32 sched_fail;		/* scheduling failures */
	uint32 timeouts;		/* timeouts */
	uint32 protoerr;		/* protocol errors */
	uint32 noack;			/* tx w/o ack */
	uint32 txfail;			/* any tx falure */
	uint32 lci_req_tx;		/* tx LCI requests */
	uint32 lci_req_rx;		/* rx LCI requests */
	uint32 lci_rep_tx;		/* tx LCI reports */
	uint32 lci_rep_rx;		/* rx LCI reports */
	uint32 civic_req_tx;		/* tx civic requests */
	uint32 civic_req_rx;		/* rx civic requests */
	uint32 civic_rep_tx;		/* tx civic reports */
	uint32 civic_rep_rx;		/* rx civic reports */
	uint32 rctx;			/* ranging contexts created */
	uint32 rctx_done;		/* count of ranging done */
	uint32 publish_err;		/* availability publishing errors */
	uint32 on_chan;			/* count of scheduler onchan */
	uint32 off_chan;		/* count of scheduler offchan */
	uint32 tsf_lo;			/* local tsf or session tsf */
	uint32 tsf_hi;
	uint32 num_meas;		/* number of measurements */
	uint32 num_ready;		/* number of entrances to ready state */
	uint32 response_time;		/* in ms.
					 * ISTA: time between FTMR tx and FTM1 rx.
					 * RSTA: time between FTMR rx and FTM1 tx.
					 */
} wl_ftm_counters_v2_t;

enum wl_test_mode_flag {
	/* WFA CTT device behavior */
	WL_FTM_TEST_MODE_FLAG_CTT_STA		= 0x00000001,
	WL_FTM_TEST_MODE_FLAG_CTT_AP		= 0x00000002,
	WL_FTM_TEST_MODE_FLAG_NOPFTIVCHK	= 0x00000004,
	WL_FTM_TEST_MODE_FLAG_NOMFP		= 0x00000008,
	/* for internal testing purpose */
	WL_FTM_TEST_MODE_FLAG_TRAP_ON_CSI_TMO	= 0x00010000,
	WL_FTM_TEST_MODE_FLAG_NO_TS_IN_LMR	= 0x00020000
};
typedef uint32 wl_test_mode_flag_t;

/* WL_FTM_TLV_ID_TEST_MODE */
typedef struct wl_ftm_test_mode {
	uint32 flags; /* wl_test_mode_flag_t */
} wl_ftm_test_mode_t;

/* WL_FTM_TLV_ID_TB_ISTA_AW */
typedef struct wl_ftm_tb_ista_aw {
	uint16 slot_cnt; /* number of valid slots in the bitmap */
	uint8 bitmap_len; /* len of bitmap array */
	uint8 pad;
	uint8 bitmap[];
} wl_ftm_tb_ista_aw_t;

/* WL_FTM_TLV_ID_CSI_DUMP */
enum wl_ftm_csi_dump_flags {
	CSI_DUMP_FLAG_RX_CSI_DATA	= 0x01,	/* RX CSI data */
	CSI_DUMP_FLAG_START		= 0x80	/* start of CSI data */
};
typedef uint8 wl_ftm_csi_dump_flags_t;

typedef struct wl_ftm_csi_dump {
	uint16 sid;
	uint8 seq;		/* seq # of measurement (++ for a measurement) */
	uint8 flags;		/* wl_ftm_csi_dump_flags */
	uint32 csi_tot_len;	/* size of complete CSI data */
	uint16 data_len;	/* size of partial CSI data in this TLV (< 64K since in XTLV) */
	uint8 csi_hdr_ver;	/* version of CSI hdr in payload (from Phy) */
	uint8 dtoken;		/* sounding dialog token of measurement */
	uint8 data[];
} wl_ftm_csi_dump_t;

/* WL_FTM_TLV_ID_CSI_RANGING_PARAMS */
enum wl_ftm_csi_ranging_params_flags {
	CSI_RNG_PARAM_FLAG_BIT_ISTA	= 0u, /* ranging as ISTA */
	CSI_RNG_PARAM_FLAG_BIT_SLTF	= 1u, /* secure LTF */
	CSI_RNG_PARAM_FLAG_BIT_NTB	= 2u, /* 11az NTB ranging */
	CSI_RNG_PARAM_FLAG_BIT_TB	= 3u, /* 11az TB ranging */
	CSI_RNG_PARAM_FLAG_BIT_PSV	= 4u  /* 11az Passive ranging */
};
#define CSI_RNG_PARAM_FLAG(_bit)	(1u << CSI_RNG_PARAM_FLAG_BIT_##_bit)

#define CSI_RNG_ISTA(_params)		((_params)->flags & CSI_RNG_PARAM_FLAG(ISTA))
#define CSI_RNG_SLTF(_params)		((_params)->flags & CSI_RNG_PARAM_FLAG(SLTF))
#define CSI_RNG_NTB(_params)		((_params)->flags & CSI_RNG_PARAM_FLAG(NTB))
#define CSI_RNG_TB(_params)		((_params)->flags & CSI_RNG_PARAM_FLAG(TB))
#define CSI_RNG_PSV(_params)		((_params)->flags & CSI_RNG_PARAM_FLAG(PSV))

/* negotiated ranging parameters for CSI data */
typedef struct wl_ftm_csi_ranging_params {
	uint32 flags;	/* wl_ftm_csi_ranging_params_flags */
	uint8 bw;
	uint8 tx_sts;	/* # of stream (1x ~ Nx) */
	uint8 rx_sts;
	uint8 tx_rep;	/* # of repetition (0 ~ N) */
	uint8 rx_rep;
	uint8 PAD[3];
} wl_ftm_csi_ranging_params_t;

/* WL_FTM_TLV_ID_CSI_PROC_RESULT */
/* CSI processing result of local device */
typedef struct wl_ftm_csi_proc_result {
	uint32 ts_tx;
	uint32 ts_rx;
} wl_ftm_csi_proc_result_t;

/* WL_FTM_TLV_ID_CSI_PROC_RTT */
/* time stamp and rtt of CSI data */
typedef struct wl_ftm_csi_proc_rtt {
	uint64 t1;
	uint64 t2;
	uint64 t3;
	uint64 t4;
	uint64 rtt;
	uint8 dtoken;
	uint8 PAD[3];
} wl_ftm_csi_proc_rtt_t;

typedef struct wl_ftm_tlv {
	uint16 id;
	uint16 len;
	uint8 data[1];
} wl_ftm_tlv_t;

/** proxd iovar - applies to proxd, method or session */
typedef struct wl_ftm_iov {
	uint16			version;
	uint16			len;
	wl_ftm_cmd_t		cmd;
	wl_ftm_method_t		method;
	wl_ftm_session_id_t	sid;
	wl_ftm_type_t		ftm_type;	/* 11az ftm type. valid for PROXD ver >= 0x0400 */
	uint8			PAD[1];
	wl_ftm_tlv_t		tlvs[];		/**< variable */
} wl_ftm_iov_t;

typedef struct wl_ftm_event {
	uint16			version;
	uint16			len;
	wl_ftm_event_type_t	type;
	wl_ftm_method_t	method;
	wl_ftm_session_id_t	sid;
	uint8			pad[2];		/* This field is used fragmentation purpose */
	wl_ftm_tlv_t tlvs[];			/**< variable */
} wl_ftm_event_t;

/* 11az RTT sample flags */
enum wl_ftm_az_rtt_sample_flags {
	WL_FTM_AZ_RTT_SAMPLE_FLAG_NONE		= 0x0000u,
	WL_FTM_AZ_RTT_SAMPLE_FLAG_VALID		= 0x0001u,
	/* add new flag here */
	WL_FTM_AZ_RTT_SAMPLE_FLAG_ALL		= 0xffffu
};
typedef uint16 wl_ftm_az_rtt_sample_flags_t;

/* WL_FTM_TLV_ID_AZ_RTT_SAMPLE_V1
 * 11az RTT sample from a measurement
 */
typedef struct wl_ftm_az_rtt_sample_v1 {
	wl_ftm_az_rtt_sample_flags_t	flags;
	uint8				sdt;	/* OTA sounding dialog token of sample */
	uint8				pad;
	uint32				rtt;	/* RTT in ps unit */
	uint32				dist;	/* distance in cm unit */
	int8				rssi[WL_RSSI_ANT_MAX];
} wl_ftm_az_rtt_sample_v1_t;

#define WL_FTM_AZ_RTT_SAMPLE_VALID(_sp) \
	(((_sp)->flags & WL_FTM_AZ_RTT_SAMPLE_FLAG_VALID) != 0u)

/* 11az RTT result flags */
enum wl_ftm_az_rtt_result_flags {
	WL_FTM_AZ_RTT_RESULT_FLAG_NONE		= 0x00000000u,
	WL_FTM_AZ_RTT_RESULT_FLAG_RTT_IN_100PS	= 0x00000001u, /* RTT in 100 ps unit */
	WL_FTM_AZ_RTT_RESULT_FLAG_DIST_IN_4CM	= 0x00000002u, /* Distance in 1/256 m unit */
	WL_FTM_AZ_RTT_RESULT_FLAG_SLTF		= 0x00000004u, /* Secure LTF */
	/* add new flag here */
	WL_FTM_AZ_RTT_RESULT_FLAG_ALL		= 0xffffffffu
};
typedef uint32 wl_ftm_az_rtt_result_flags_t;

#define WL_FTM_AZ_RTT_RESULT_RTT_IN_100PS(_rp) \
	(((_rp)->flags & WL_FTM_AZ_RTT_RESULT_FLAG_RTT_IN_100PS) != 0u)
#define WL_FTM_AZ_RTT_RESULT_DIST_IN_4CM(_rp) \
	(((_rp)->flags & WL_FTM_AZ_RTT_RESULT_FLAG_DIST_IN_4CM) != 0u)
#define WL_FTM_AZ_RTT_RESULT_SLTF(_rp) \
	(((_rp)->flags & WL_FTM_AZ_RTT_RESULT_FLAG_SLTF) != 0u)

/* WL_FTM_TLV_ID_AZ_RTT_RESULT_V1
 * 11az RTT result from a session
 */
typedef struct wl_ftm_az_rtt_result_v1 {
	wl_ftm_session_id_t		sid;
	struct ether_addr		peer;
	wl_ftm_az_rtt_result_flags_t	flags;
	wl_ftm_status_t			status;		/* session status */
	wl_ftm_session_state_t		state;		/* session state */
	uint16				max_num_meas;	/* configured num of measurement */
	uint16				num_meas;	/* num of measurement done */
	uint16				num_rtt;	/* num of rtt */
	uint32				rtt_mean;	/* mean RTT (in ps by default) */
	uint32				rtt_sd;		/* standard deviation of RTT */
	uint32				dist;		/* Distance (in cm unit by default) */
	int8				rssi_mean[WL_RSSI_ANT_MAX];
	uint16				num_sample;	/* number of rtt sample */
	uint16				sample_fmt;	/* format of rtt sample (TLV ID) */
	uint8				rtt_samples[];	/* optional variable length fields */
} wl_ftm_az_rtt_result_v1_t;

/* WL_FTM_TLV_ID_AZ_COUNTERS_V1
 * 11az ranging counters
 */
typedef struct wl_ftm_az_counters_v1 {
	uint32 active;		/* 11az measurement activation */
	uint32 meas;		/* successful measurement */
	uint32 timeout;		/* measurement timeout */
	uint32 rtt;		/* rtt from measurement */
	uint32 txndpa;		/* TX NDPA */
	uint32 rxndpa;		/* RX NDPA */
	uint32 txndp;		/* TX NDP */
	uint32 rxndp;		/* RX NDP */
	uint32 txlmr;		/* TX LMR */
	uint32 rxlmr;		/* RX LMR */
	uint32 rxrtfpoll;	/* RX Ranging TF Poll */
	uint32 rxrtfsnd;	/* RX Ranging TF Sounding */
	uint32 rxrtfssnd;	/* RX Ranging TF Secure Sounding */
	uint32 rxrtfrpt;	/* RX Ranging TF Report */
	uint32 rxrtfpsnd;	/* RX Ranging TF Passive Sounding */
} wl_ftm_az_counters_v1_t;

/* WL_FTM_TLV_ID_HAL_COUNTERS_V1
 * ftm HAL counters
 */
typedef struct wl_ftm_hal_counters_v1 {
	uint32 mcsetup;		/* 11mc HAL setup */
	uint32 mcsetupfail;	/* 11mc HAL setup fail */
	uint32 mccleanup;	/* 11mc HAL clean up */
	uint32 azsetup;		/* 11az HAL setup */
	uint32 azsetupfail;	/* 11az HAL setup fail */
	uint32 azcleanup;	/* 11az HAL clean up */
	uint32 phyazsetup;	/* phy 11az setup */
	uint32 phyazcleanup;	/* phy 11az cleanup */
	uint32 phyazprocess;	/* phy 11az csi processing */
	uint32 phycal;		/* Phy calibration */
	uint32 csiwait;		/* CSI data wait */
	uint32 csitimeout;	/* CSI data timeout */
	uint32 csifrm_tx;	/* CSI loopback frames */
	uint32 csifrm_rx;	/* CSI RX frames */
	uint32 csifrm_txfrag;	/* CSI loopback fragments */
	uint32 csifrm_rxfrag;	/* CSI RX fragments */
	uint32 csifrm_txbytes;	/* CSI loopback bytes total */
	uint32 csifrm_rxbytes;	/* CSI RX bytes total */
	uint32 csifrm_total;	/* total CSI frames */
	uint32 csifrm_good;	/* CSI frames successully handled */
	uint32 csifrm_bad;	/* CSI frames unsuccessully handled */
	uint32 csifrm_discard;	/* CSI frames discarded */
	uint32 csifrm_first;	/* first CSI frames */
	uint32 csifrm_last;	/* last CSI frames */
	uint32 csifrm_phyerr;	/* CSI frame with error from phy */
	uint32 csifrm_bmfull;	/* BM overflow while receiving CSI frame */
	uint32 csifrm_coll;	/* Collision happened while receiving CSI frame */
} wl_ftm_hal_counters_v1_t;

/* FTM error codes [-1024, -2047] */
enum {
	WL_FTM_E_LAST			= -1091,
	WL_FTM_E_FORCE_DELETED		= -1091,
	WL_FTM_E_ONE_WAY_RTT		= -1090,
	WL_FTM_E_PRIMARY_CLONE_START	= -1089,
	WL_FTM_E_DEFER_ACK_LOST		= -1088,
	WL_FTM_E_NSTS_INCAPABLE		= -1087,
	WL_FTM_E_KDK_NOT_READY		= -1086,
	WL_FTM_E_INVALID_SLTF_COUNTER	= -1085,
	WL_FTM_E_BAD_KEY_INFO_IDX	= -1084,
	WL_FTM_E_VALID_SAC_GEN_FAIL	= -1083,
	WL_FTM_E_NDPA_SAC_MISMATCH	= -1082,
	WL_FTM_E_MEAS_SAC_MISMATCH	= -1081,
	WL_FTM_E_VALID_SAC_MISMATCH	= -1080,
	WL_FTM_E_OUTSIDE_RSTA_AW	= -1079,
	WL_FTM_E_NO_STA_INFO		= -1078,
	WL_FTM_E_NO_SLTF_KEY_INFO	= -1077,
	WL_FTM_E_TOKEN_MISMATCH		= -1076,
	WL_FTM_E_IE_NOTFOUND		= -1075,
	WL_FTM_E_IE_BADLEN		= -1074,
	WL_FTM_E_INVALID_BW		= -1073,
	WL_FTM_E_INVALID_ST_CH		= -1072,
	WL_FTM_E_RSTA_AND_ISTA		= -1071,
	WL_FTM_E_NO_SLTF_INFO		= -1070,
	WL_FTM_E_INVALID_NBURST		= -1069,
	WL_FTM_E_FATAL			= -1068,
	WL_FTM_E_PASN			= -1067,
	WL_FTM_E_PERM			= -1066,
	WL_FTM_E_BURST			= -1065,
	WL_FTM_E_RESCHED		= -1064,
	WL_FTM_E_OFF_CHAN		= -1063,
	WL_FTM_E_NO_SCB			= -1062,
	WL_FTM_E_NOT_READY		= -1061,
	WL_FTM_E_DELETED		= -1060,
	WL_FTM_E_TX_PENDING		= -1059,
	WL_FTM_E_BAD_CONFIG		= -1058,
	WL_FTM_E_ASSOC_INPROG		= -1057,
	WL_FTM_E_NOAVAIL		= -1056,
	WL_FTM_E_EXT_SCHED		= -1055,
	WL_FTM_E_NOT_BCM		= -1054,
	WL_FTM_E_FRAME_TYPE		= -1053,
	WL_FTM_E_VERNOSUPPORT		= -1052,
	WL_FTM_E_SEC_NOKEY		= -1051,
	WL_FTM_E_SEC_POLICY		= -1050,
	WL_FTM_E_SCAN_INPROCESS		= -1049,
	WL_FTM_E_BAD_PARTIAL_TSF	= -1048,
	WL_FTM_E_SCANFAIL		= -1047,
	WL_FTM_E_NOTSF			= -1046,
	WL_FTM_E_POLICY			= -1045,
	WL_FTM_E_INCOMPLETE		= -1044,
	WL_FTM_E_OVERRIDDEN		= -1043,
	WL_FTM_E_ASAP_FAILED		= -1042,
	WL_FTM_E_NOTSTARTED		= -1041,
	WL_FTM_E_INVALIDMEAS		= -1040,
	WL_FTM_E_INCAPABLE		= -1039,
	WL_FTM_E_MISMATCH		= -1038,
	WL_FTM_E_DUP_SESSION		= -1037,
	WL_FTM_E_REMOTE_FAIL		= -1036,
	WL_FTM_E_REMOTE_INCAPABLE	= -1035,
	WL_FTM_E_SCHED_FAIL		= -1034,
	WL_FTM_E_PROTO			= -1033,
	WL_FTM_E_EXPIRED		= -1032,
	WL_FTM_E_TIMEOUT		= -1031,
	WL_FTM_E_NOACK			= -1030,
	WL_FTM_E_DEFERRED		= -1029,
	WL_FTM_E_INVALID_SID		= -1028,
	WL_FTM_E_REMOTE_CANCEL		= -1027,
	WL_FTM_E_CANCELED		= -1026,	/**< local */
	WL_FTM_E_INVALID_SESSION	= -1025,
	WL_FTM_E_BAD_STATE		= -1024,
	WL_FTM_E_OK			= 0
};

#endif /* _ftm_ioctl_h */
