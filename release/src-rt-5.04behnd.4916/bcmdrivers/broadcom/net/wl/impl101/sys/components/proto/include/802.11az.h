/*
 * Fundamental types and constants relating to 802.11az -
 * "Enhancements for positioning"
 *
 * PFT - Protected Fine Timeing
 * FTM - Fine Timing Measuremant
 * PASN - Preassociation security negotiation
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

#ifndef _802_11az_h_
#define _802_11az_h_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* P802.11az/D2.5 - Table 9-322h23fd Ranging Subelement IDs for Ranging Parameters */
#define RANGING_PARAMS_NTB_SUB_ELT_ID		0u	/* Non-TB specific subelement */
#define RANGING_PARAMS_TB_SUB_ELT_ID		1u	/* TB specific subelement */
#define RANGING_PARAMS_SLTF_SUB_ELT_ID		2u	/* Secure LTF subelement */
#define RANGING_PARAMS_STLF_SUB_ELT_ID		RANGING_PARAMS_SLTF_SUB_ELT_ID /* to be obsoleted */
#define RANGING_PARAMS_VNDR_SUB_ELT_ID		221u	/* Vendor specific subelement */

/* P802.11az/D3.1 - Table 9-788edm1 Secure LTF subelement format */
#define FTM_PARAM_SLTF_REQUIRED_BIT		0x08

/* Protected Fine Timing frame action field values - 802.11az/D2.5 - Table 9.353 */
#define DOT11_PFT_ACTION_RESERVED0	0u /* Reserved */
#define DOT11_PFT_ACTION_FTM_REQ	1u /* Protected Fine Timing Measurement Request */
#define DOT11_PFT_ACTION_FTM		2u /* Protected Fine Timing Measurement */
#define DOT11_PFT_ACTION_LMR		3u /* Protected Location Measurement Report */

/* FTM - fine timing measurement public action frames */
BWL_PRE_PACKED_STRUCT struct dot11_ftm_req {
	uint8 category;				/* category of action frame (4) */
	uint8 action;				/* public action (32) */
	uint8 trigger;				/* trigger/continue? */
	/* optional lci, civic loc, ftm params */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_req dot11_ftm_req_t;

BWL_PRE_PACKED_STRUCT struct dot11_ftm {
	uint8 category;				/* category of action frame (4) */
	uint8 action;				/* public action (33) */
	uint8 dialog;				/* dialog token */
	uint8 follow_up;			/* follow up dialog token */
	uint8 tod[6];				/* t1 - last depart timestamp */
	uint8 toa[6];				/* t4 - last ack arrival timestamp */
	uint8 tod_err[2];			/* t1 error */
	uint8 toa_err[2];			/* t4 error */
	/* optional lci report, civic loc report, ftm params */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm dot11_ftm_t;

BWL_PRE_PACKED_STRUCT struct dot11_ftm_lmr {
	uint8    category;          /* category of action frame (4) */
	uint8    action;            /* public action (33) */
	uint8    dialog;            /* dialog token */
	uint8    tod[6];            /* RSTA t3 or ISTA t1:
	                             * last departure of NDP
	                             */
	uint8    toa[6];            /* RSTA t2 or ISTA t4:
	                             * last arrival of NDP
	                             */
	uint8    tod_err;           /* t3 or t1 error */
	uint8    toa_err;           /* t2 or t4 error */
	uint16   cfo;               /* I2R LMR: clock difference between ISTA and RSTA. */
	uint8    r2i_ndp_tx_pw;     /* average power of previous R2I NDP */
	uint8    i2r_ndp_tgt_rssi;  /* desired power for next I2R NDP */
	uint8    sec_ltf_params[];  /* Optional Secure LTF parameters */
	/* no AOA feedback */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_lmr dot11_ftm_lmr_t;

BWL_PRE_PACKED_STRUCT struct dot11_ranging_ndpa {
	uint8	dialog_token;	/* sounding dialog token */
	uint8	sta_info[];	/* STA infos */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ranging_ndpa dot11_ranging_ndpa_t;

/* NDPA types = dialog token byte lower 2 bits */
#define DOT11_NDPA_TYPE_MASK		0x03u
#define DOT11_NDPA_TYPE_VHT		0x00u
#define DOT11_NDPA_TYPE_RANGING		0x01u
#define DOT11_NDPA_TYPE_HE		0x02u
#define DOT11_NPDA_TOKEN_SHIFT		2u

/* NDPA STA info size */
#define DOT11_NDPA_STA_INFO_SIZE	4u

#define DOT11_RANGING_TF_CMN_INFO_FIXED_SIZE 8u
BWL_PRE_PACKED_STRUCT struct dot11_ranging_trigger {
	uint8 cmn_info[DOT11_RANGING_TF_CMN_INFO_FIXED_SIZE];	/* trigger common info */
	uint8 dep_cmn_info[];			/* type dependent common info */
	/* followed by user info list */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ranging_trigger dot11_ranging_trigger_t;

/* Trigger frame types
 * P802.11ax/D8.0 & P802.11az/D4.0 Table 9-20c Trigger Type subfield encoding
 */
#define DOT11_TF_TYPE_MASK		0x0Fu
#define DOT11_TF_TYPE_BASIC		0u /* basic */
#define DOT11_TF_TYPE_BFRP		1u /* Beamforming Report Poll */
#define DOT11_TF_TYPE_MUBAR		2u /* MU-BAR */
#define DOT11_TF_TYPE_MURTS		3u /* MU-RTS */
#define DOT11_TF_TYPE_BSRP		4u /* Buffer Status Report Poll */
#define DOT11_TF_TYPE_GCR_MUBAR		5u /* GCR MU-BAR */
#define DOT11_TF_TYPE_BQRP		6u /* Bandwidth Query Report Poll */
#define DOT11_TF_TYPE_NFRP		7u /* NDP Feedback Report Poll */
#define DOT11_TF_TYPE_RANGING		8u /* Ranging */

/* Ranging TF subtypes in Ranging trigger dependent common info
 * P802.11az/D4.0 Table 9-30ka Ranging Trigger Subtype subfield encoding
 */
#define DOT11_RANGING_TF_SUBTYPE_MASK	0x0Fu
#define DOT11_RANGING_TF_SUBTYPE_POLL	0u /* Ranging TF Poll */
#define DOT11_RANGING_TF_SUBTYPE_SND	1u /* Ranging TF Sounding */
#define DOT11_RANGING_TF_SUBTYPE_SSND	2u /* Ranging TF Secure Sounding */
#define DOT11_RANGING_TF_SUBTYPE_RPT	3u /* Ranging TF Report */
#define DOT11_RANGING_TF_SUBTYPE_PSND	4u /* Ranging TF Passive Sounding */

/* Ranging TF dependent common info len */
#define DOT11_RANGING_TF_DEP_CMN_INFO_LEN	1u /* Ranging TF variant */
#define DOT11_RANGING_TF_PSND_DEP_CMN_INFO_LEN	2u /* Ranging TF Passive sounding */

/* Ranging TF user info fixed len - followed by TF dependent user info */
#define DOT11_RANGING_TF_USER_INFO_FIXED_LEN	5u

/* start of padding marker in user info */
#define DOT11_TF_USER_INFO_PADDING_START	4095u

#define DOT11_FTM_ERR_NOT_CONT_OFFSET 1
#define DOT11_FTM_ERR_NOT_CONT_MASK 0x80
#define DOT11_FTM_ERR_NOT_CONT_SHIFT 7
#define DOT11_FTM_ERR_NOT_CONT(_err) (((_err)[DOT11_FTM_ERR_NOT_CONT_OFFSET] & \
	DOT11_FTM_ERR_NOT_CONT_MASK) >> DOT11_FTM_ERR_NOT_CONT_SHIFT)
#define DOT11_FTM_ERR_SET_NOT_CONT(_err, _val) do {\
	uint8 _err2 = (_err)[DOT11_FTM_ERR_NOT_CONT_OFFSET]; \
	_err2 &= ~DOT11_FTM_ERR_NOT_CONT_MASK; \
	_err2 |= ((_val) << DOT11_FTM_ERR_NOT_CONT_SHIFT) & DOT11_FTM_ERR_NOT_CONT_MASK; \
	(_err)[DOT11_FTM_ERR_NOT_CONT_OFFSET] = _err2; \
} while (0)

#define DOT11_FTM_ERR_MAX_ERR_OFFSET 0
#define DOT11_FTM_ERR_MAX_ERR_MASK 0x7fff
#define DOT11_FTM_ERR_MAX_ERR_SHIFT 0
#define DOT11_FTM_ERR_MAX_ERR(_err) (((((_err)[1] & 0x7f) << 8) | (_err)[0]))
#define DOT11_FTM_ERR_SET_MAX_ERR(_err, _val) do {\
	uint16 _val2; \
	uint16 _not_cont; \
	_val2 =  (((_val) & DOT11_FTM_ERR_MAX_ERR_MASK) << DOT11_FTM_ERR_MAX_ERR_SHIFT); \
	_val2 = (_val2 > 0x3fff) ? 0 : _val2; /* not expecting > 16ns error */ \
	_not_cont = DOT11_FTM_ERR_NOT_CONT(_err); \
	(_err)[0] = _val2 & 0xff; \
	(_err)[1] = (_val2 >> 8) & 0xff; \
	DOT11_FTM_ERR_SET_NOT_CONT(_err, _not_cont); \
} while (0)

BWL_PRE_PACKED_STRUCT struct dot11_ftm_params {
	uint8 id; /* DOT11_MNG_FTM_PARAM_ID 8.4.2.166 11mcd2.6/2014 - revisit */
	uint8 len;
	uint8 info[9];
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_params dot11_ftm_params_t;
#define DOT11_FTM_PARAMS_IE_LEN (sizeof(dot11_ftm_params_t) - OFFSETOF(dot11_ftm_params_t, info))

/* FTM Ranging parameters element */
#define DOT11_FTM_RANGING_PARAMS_FIXED_SIZE 7u
BWL_PRE_PACKED_STRUCT struct dot11_ftm_ranging_params {
	uint8 id; /* 255 */
	uint8 len;
	uint8 ext_id; /* EXT_MNG_RANGING_PARAMS_ID */
	uint8 info[DOT11_FTM_RANGING_PARAMS_FIXED_SIZE];
	uint8 subelmts[]; /* sub-elements (NTB/TB/SLTF/Vendor) */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_ranging_params dot11_ftm_ranging_params_t;
#define DOT11_FTM_RANGING_PARAMS_MIN_LEN \
	(sizeof(dot11_ftm_ranging_params_t) - OFFSETOF(dot11_ftm_ranging_params_t, ext_id))

/* FTM NTB specific sub-element */
#define DOT11_FTM_NTB_SUBELMT_FIXED_SIZE 6u
BWL_PRE_PACKED_STRUCT struct dot11_ftm_ntb_subelmt {
	uint8 id; /* RANGING_PARAMS_NTB_SUB_ELT_ID */
	uint8 len;
	uint8 info[DOT11_FTM_NTB_SUBELMT_FIXED_SIZE];
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_ntb_subelmt dot11_ftm_ntb_subelmt_t;

/* FTM TB specific sub-element */
#define DOT11_FTM_TB_SUBELMT_FIXED_SIZE 4u
BWL_PRE_PACKED_STRUCT struct dot11_ftm_tb_subelmt {
	uint8 id; /* RANGING_PARAMS_TB_SUB_ELT_ID */
	uint8 len;
	uint8 info[DOT11_FTM_TB_SUBELMT_FIXED_SIZE];
	uint8 aw_elmts[]; /* AW elements */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_tb_subelmt dot11_ftm_tb_subelmt_t;

/* FTM ISTA availability window element */
BWL_PRE_PACKED_STRUCT struct dot11_ftm_ista_aw {
	uint8 id; /* 255 */
	uint8 len;
	uint8 ext_id; /* EXT_MNG_ISTA_AVAIL_WINDOW_ID */
	uint16 count;
	uint8 bitmap[];
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_ista_aw dot11_ftm_ista_aw_t;
#define DOT11_FTM_ISTA_AW_MIN_LEN \
	(sizeof(dot11_ftm_ista_aw_t) - OFFSETOF(dot11_ftm_ista_aw_t, ext_id))

/* FTM RSTA availability window element */
BWL_PRE_PACKED_STRUCT struct dot11_ftm_rsta_aw {
	uint8 id; /* 255 */
	uint8 len;
	uint8 ext_id; /* EXT_MNG_RSTA_AVAIL_WINDOW_ID */
	uint8 hdr;
	uint8 aw_info[]; /* AW info subfields */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_rsta_aw dot11_ftm_rsta_aw_t;
#define DOT11_FTM_RSTA_AW_MIN_LEN \
	(sizeof(dot11_ftm_rsta_aw_t) - OFFSETOF(dot11_ftm_rsta_aw_t, ext_id))

/* FTM Secure LTF sub-element */
BWL_PRE_PACKED_STRUCT struct dot11_ftm_sltf_subelmt {
	uint8 id; /* RANGING_PARAMS_STLF_SUB_ELT_ID */
	uint8 len;
	uint8 info;
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_sltf_subelmt dot11_ftm_sltf_subelmt_t;
#define DOT11_FTM_SLTF_SUBELMT_LEN \
	(sizeof(dot11_ftm_sltf_subelmt_t) - OFFSETOF(dot11_ftm_sltf_subelmt_t, info))

/* FTM Secure LTF parameters element */
#define DOT11_FTM_SLTF_PARAMS_SLTF_COUNTER_LEN 6u
#define DOT11_FTM_SLTF_PARAMS_SAC_LEN 2u
BWL_PRE_PACKED_STRUCT struct dot11_ftm_sltf_params {
	uint8 id; /* 255 */
	uint8 len;
	uint8 ext_id; /* EXT_MNG_SLTF_PARAMS_ID */
	uint8 sltf_counter[DOT11_FTM_SLTF_PARAMS_SLTF_COUNTER_LEN];
	uint8 validation_sac[DOT11_FTM_SLTF_PARAMS_SAC_LEN];
	uint8 measurement_sac[DOT11_FTM_SLTF_PARAMS_SAC_LEN];
	uint8 meas_result_ltf_offset;
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_sltf_params dot11_ftm_sltf_params_t;
#define DOT11_FTM_SLTF_PARAMS_LEN \
	(sizeof(dot11_ftm_sltf_params_t) - OFFSETOF(dot11_ftm_sltf_params_t, ext_id))

BWL_PRE_PACKED_STRUCT struct dot11_ftm_ranging_ndpa {
	uint16			fc;		/* frame control */
	uint16			durid;		/* duration/ID */
	struct ether_addr	ra;		/* receiver address */
	struct ether_addr	ta;		/* transmitter address */
	uint8                   dialog_token; /* sounding dialog token */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_ranging_ndpa dot11_ftm_ranging_ndpa_t;

typedef struct dot11_ftm_sltf_subelmt dot11_ftm_sec_ltf_subie_params_t;
#define DOT11_FTM_SLTF_PARAMS_SUB_IE_LEN (sizeof(dot11_ftm_sec_ltf_subie_params_t))

#define DOT11_FTM_RANGING_CMN_PARAM_SIZE DOT11_FTM_RANGING_PARAMS_FIXED_SIZE
#define DOT11_FTM_CMN_RANGING_PARAMS_IE_LEN (sizeof(dot11_ftm_ranging_params_t) - TLV_EXT_HDR_LEN)

/* FTM NTB specific */
BWL_PRE_PACKED_STRUCT struct dot11_ftm_ntb_params {
	uint8 id; /* RANGING_PARAMS_NTB_SUB_ELT_ID */
	uint8 len;
	uint8 info[6];
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_ntb_params dot11_ftm_ntb_params_t;
#define DOT11_FTM_NTB_PARAMS_SUB_IE_LEN (sizeof(dot11_ftm_ntb_params_t))
#define DOT11_FTM_NTB_PARAMS_IE_LEN (DOT11_FTM_CMN_RANGING_PARAMS_IE_LEN + \
	DOT11_FTM_NTB_PARAMS_SUB_IE_LEN)

/* FTM TB specific */
BWL_PRE_PACKED_STRUCT struct dot11_ftm_tb_params {
	uint8 id; /* RANGING_PARAMS_TB_SUB_ELT_ID */
	uint8 len;
	uint8 info[]; /* variable length */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_tb_params dot11_ftm_tb_params_t;
#define DOT11_FTM_TB_PARAMS_IE_LEN sizeof(dot11_ftm_tb_params_t)

BWL_PRE_PACKED_STRUCT struct dot11_ftm_tb_avail_window {
	uint8 id; /* 255 */
	uint8 len;
	uint8 ext_id; /* EXT_MNG_ISTA_AVAIL_WINDOW_ID or
		       * EXT_MNG_RSTA_AVAIL_WINDOW_ID
		       */
	uint8 info[];
}  BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_tb_avail_window dot11_ftm_tb_avail_window_t;
#define DOT11_FTM_TB_AVAIL_WINDOW_HDR_LEN sizeof(dot11_ftm_tb_avail_window_t)

BWL_PRE_PACKED_STRUCT struct dot11_ftm_sec_ltf_params {
	uint8 id; /* 255 */
	uint8 len;
	uint8 ext_id; /* DOT11_MNG_FTM_SECURE_LTF_EXT_ID */
	uint8 info[11];
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_sec_ltf_params dot11_ftm_sec_ltf_params_t;
#define DOT11_FTM_SEC_LTF_PARAMS_IE_LEN (sizeof(dot11_ftm_sec_ltf_params_t) - TLV_EXT_HDR_LEN)

#define FTM_PARAMS_FIELD(_p, _off, _mask, _shift) (((_p)->info[(_off)] & (_mask)) >> (_shift))
#define FTM_PARAMS_SET_FIELD(_p, _off, _mask, _shift, _val) do {\
	uint8 _ptmp = (_p)->info[_off] & ~(_mask); \
	(_p)->info[(_off)] = _ptmp | (((_val) << (_shift)) & (_mask)); \
} while (0)

#define FTM_PARAMS_STATUS_OFFSET 0
#define FTM_PARAMS_STATUS_MASK 0x03
#define FTM_PARAMS_STATUS_SHIFT 0
#define FTM_PARAMS_STATUS(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_STATUS_OFFSET, \
	FTM_PARAMS_STATUS_MASK, FTM_PARAMS_STATUS_SHIFT)
#define FTM_PARAMS_SET_STATUS(_p, _status) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_STATUS_OFFSET, FTM_PARAMS_STATUS_MASK, FTM_PARAMS_STATUS_SHIFT, _status)

#define FTM_PARAMS_VALUE_OFFSET 0
#define FTM_PARAMS_VALUE_MASK 0x7c
#define FTM_PARAMS_VALUE_SHIFT 2
#define FTM_PARAMS_VALUE(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_VALUE_OFFSET, \
	FTM_PARAMS_VALUE_MASK, FTM_PARAMS_VALUE_SHIFT)
#define FTM_PARAMS_SET_VALUE(_p, _value) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_VALUE_OFFSET, FTM_PARAMS_VALUE_MASK, FTM_PARAMS_VALUE_SHIFT, _value)
#define FTM_PARAMS_MAX_VALUE 32

#define FTM_PARAMS_NBURSTEXP_OFFSET 1
#define FTM_PARAMS_NBURSTEXP_MASK 0x0f
#define FTM_PARAMS_NBURSTEXP_SHIFT 0
#define FTM_PARAMS_NBURSTEXP(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_NBURSTEXP_OFFSET, \
	FTM_PARAMS_NBURSTEXP_MASK, FTM_PARAMS_NBURSTEXP_SHIFT)
#define FTM_PARAMS_SET_NBURSTEXP(_p, _bexp) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_NBURSTEXP_OFFSET, FTM_PARAMS_NBURSTEXP_MASK, FTM_PARAMS_NBURSTEXP_SHIFT, \
	_bexp)

#define FTM_PARAMS_NBURST(_p) (1 << FTM_PARAMS_NBURSTEXP(_p))

enum {
	FTM_PARAMS_NBURSTEXP_NOPREF = 15
};

enum {
	FTM_PARAMS_BURSTTMO_NOPREF = 15
};

#define FTM_PARAMS_BURSTTMO_OFFSET 1
#define FTM_PARAMS_BURSTTMO_MASK 0xf0
#define FTM_PARAMS_BURSTTMO_SHIFT 4
#define FTM_PARAMS_BURSTTMO(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_BURSTTMO_OFFSET, \
	FTM_PARAMS_BURSTTMO_MASK, FTM_PARAMS_BURSTTMO_SHIFT)
/* set timeout in params using _tmo where timeout = 2^(_tmo) * 250us */
#define FTM_PARAMS_SET_BURSTTMO(_p, _tmo) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_BURSTTMO_OFFSET, FTM_PARAMS_BURSTTMO_MASK, FTM_PARAMS_BURSTTMO_SHIFT, (_tmo)+2)

#define FTM_PARAMS_BURSTTMO_USEC(_val) ((1 << ((_val)-2)) * 250)
#define FTM_PARAMS_BURSTTMO_VALID(_val) ((((_val) < 12 && (_val) > 1)) || \
	(_val) == FTM_PARAMS_BURSTTMO_NOPREF)
#define FTM_PARAMS_BURSTTMO_MAX_MSEC 128 /* 2^9 * 250us */
#define FTM_PARAMS_BURSTTMO_MAX_USEC 128000 /* 2^9 * 250us */

#define FTM_PARAMS_MINDELTA_OFFSET 2
#define FTM_PARAMS_MINDELTA_USEC(_p) ((_p)->info[FTM_PARAMS_MINDELTA_OFFSET] * 100)
#define FTM_PARAMS_SET_MINDELTA_USEC(_p, _delta) do { \
	(_p)->info[FTM_PARAMS_MINDELTA_OFFSET] = (_delta) / 100; \
} while (0)

enum {
	FTM_PARAMS_MINDELTA_NOPREF = 0
};

#define FTM_PARAMS_PARTIAL_TSF(_p) ((_p)->info[4] << 8 | (_p)->info[3])
#define FTM_PARAMS_SET_PARTIAL_TSF(_p, _partial_tsf) do { \
	(_p)->info[3] = (_partial_tsf) & 0xff; \
	(_p)->info[4] = ((_partial_tsf) >> 8) & 0xff; \
} while (0)

#define FTM_PARAMS_PARTIAL_TSF_MASK 0x0000000003fffc00ULL
#define FTM_PARAMS_PARTIAL_TSF_SHIFT 10
#define FTM_PARAMS_PARTIAL_TSF_BIT_LEN 16
#define FTM_PARAMS_PARTIAL_TSF_MAX 0xffff

/* FTM can indicate upto 62k TUs forward and 1k TU backward */
#define FTM_PARAMS_TSF_FW_HI (63487 << 10)	/* in micro sec */
#define FTM_PARAMS_TSF_BW_LOW (64512 << 10)	/* in micro sec */
#define FTM_PARAMS_TSF_BW_HI (65535 << 10)	/* in micro sec */
#define FTM_PARAMS_TSF_FW_MAX FTM_PARAMS_TSF_FW_HI
#define FTM_PARAMS_TSF_BW_MAX (FTM_PARAMS_TSF_BW_HI - FTM_PARAMS_TSF_BW_LOW)

#define FTM_PARAMS_PTSFNOPREF_OFFSET 5
#define FTM_PARAMS_PTSFNOPREF_MASK 0x1
#define FTM_PARAMS_PTSFNOPREF_SHIFT 0
#define FTM_PARAMS_PTSFNOPREF(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_PTSFNOPREF_OFFSET, \
	FTM_PARAMS_PTSFNOPREF_MASK, FTM_PARAMS_PTSFNOPREF_SHIFT)
#define FTM_PARAMS_SET_PTSFNOPREF(_p, _nopref) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_PTSFNOPREF_OFFSET, FTM_PARAMS_PTSFNOPREF_MASK, \
	FTM_PARAMS_PTSFNOPREF_SHIFT, _nopref)

#define FTM_PARAMS_ASAP_OFFSET 5
#define FTM_PARAMS_ASAP_MASK 0x4
#define FTM_PARAMS_ASAP_SHIFT 2
#define FTM_PARAMS_ASAP(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_ASAP_OFFSET, \
	FTM_PARAMS_ASAP_MASK, FTM_PARAMS_ASAP_SHIFT)
#define FTM_PARAMS_SET_ASAP(_p, _asap) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_ASAP_OFFSET, FTM_PARAMS_ASAP_MASK, FTM_PARAMS_ASAP_SHIFT, _asap)

/* FTM1 - AKA ASAP Capable */
#define FTM_PARAMS_FTM1_OFFSET 5
#define FTM_PARAMS_FTM1_MASK 0x02
#define FTM_PARAMS_FTM1_SHIFT 1
#define FTM_PARAMS_FTM1(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_FTM1_OFFSET, \
	FTM_PARAMS_FTM1_MASK, FTM_PARAMS_FTM1_SHIFT)
#define FTM_PARAMS_SET_FTM1(_p, _ftm1) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_FTM1_OFFSET, FTM_PARAMS_FTM1_MASK, FTM_PARAMS_FTM1_SHIFT, _ftm1)

#define FTM_PARAMS_MAX_FTMS_PER_BURST	31u
#define FTM_PARAMS_FTMS_PER_BURST_OFFSET 5
#define FTM_PARAMS_FTMS_PER_BURST_MASK 0xf8
#define FTM_PARAMS_FTMS_PER_BURST_SHIFT 3
#define FTM_PARAMS_FTMS_PER_BURST(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_FTMS_PER_BURST_OFFSET, \
	FTM_PARAMS_FTMS_PER_BURST_MASK, FTM_PARAMS_FTMS_PER_BURST_SHIFT)
#define FTM_PARAMS_SET_FTMS_PER_BURST(_p, _nftms) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_FTMS_PER_BURST_OFFSET, FTM_PARAMS_FTMS_PER_BURST_MASK, \
	FTM_PARAMS_FTMS_PER_BURST_SHIFT, _nftms)

enum {
	FTM_PARAMS_FTMS_PER_BURST_NOPREF = 0
};

#define FTM_PARAMS_CHAN_INFO_OFFSET 6
#define FTM_PARAMS_CHAN_INFO_MASK 0xfc
#define FTM_PARAMS_CHAN_INFO_SHIFT 2
#define FTM_PARAMS_CHAN_INFO(_p) FTM_PARAMS_FIELD(_p, FTM_PARAMS_CHAN_INFO_OFFSET, \
	FTM_PARAMS_CHAN_INFO_MASK, FTM_PARAMS_CHAN_INFO_SHIFT)
#define FTM_PARAMS_SET_CHAN_INFO(_p, _ci) FTM_PARAMS_SET_FIELD(_p, \
	FTM_PARAMS_CHAN_INFO_OFFSET, FTM_PARAMS_CHAN_INFO_MASK, FTM_PARAMS_CHAN_INFO_SHIFT, _ci)

/* burst period - units of 100ms */
#define FTM_PARAMS_BURST_PERIOD(_p) (((_p)->info[8] << 8) | (_p)->info[7])
#define FTM_PARAMS_SET_BURST_PERIOD(_p, _bp) do {\
	(_p)->info[7] = (_bp) & 0xff; \
	(_p)->info[8] = ((_bp) >> 8) & 0xff; \
} while (0)

#define FTM_PARAMS_BURST_PERIOD_MS(_p) (FTM_PARAMS_BURST_PERIOD(_p) * 100)

enum {
	FTM_PARAMS_BURST_PERIOD_NOPREF = 0
};

/* FTM status values - last updated from 11mcD4.0 */
enum {
	FTM_PARAMS_STATUS_RESERVED	= 0,
	FTM_PARAMS_STATUS_SUCCESSFUL = 1,
	FTM_PARAMS_STATUS_INCAPABLE = 2,
	FTM_PARAMS_STATUS_FAILED = 3,
	/* Below are obsolte */
	FTM_PARAMS_STATUS_OVERRIDDEN = 4,
	FTM_PARAMS_STATUS_ASAP_INCAPABLE = 5,
	FTM_PARAMS_STATUS_ASAP_FAILED = 6,
	/* rest are reserved */
};

enum {
	FTM_PARAMS_CHAN_INFO_NO_PREF		= 0,
	FTM_PARAMS_CHAN_INFO_RESERVE1		= 1,
	FTM_PARAMS_CHAN_INFO_RESERVE2		= 2,
	FTM_PARAMS_CHAN_INFO_RESERVE3		= 3,
	FTM_PARAMS_CHAN_INFO_NON_HT_5		= 4,
	FTM_PARAMS_CHAN_INFO_RESERVE5		= 5,
	FTM_PARAMS_CHAN_INFO_NON_HT_10		= 6,
	FTM_PARAMS_CHAN_INFO_RESERVE7		= 7,
	FTM_PARAMS_CHAN_INFO_NON_HT_20		= 8, /* excludes 2.4G, and High rate DSSS */
	FTM_PARAMS_CHAN_INFO_HT_MF_20		= 9,
	FTM_PARAMS_CHAN_INFO_VHT_20		= 10,
	FTM_PARAMS_CHAN_INFO_HT_MF_40		= 11,
	FTM_PARAMS_CHAN_INFO_VHT_40		= 12,
	FTM_PARAMS_CHAN_INFO_VHT_80		= 13,
	FTM_PARAMS_CHAN_INFO_VHT_80_80		= 14,
	FTM_PARAMS_CHAN_INFO_VHT_160_2_RFLOS	= 15,
	FTM_PARAMS_CHAN_INFO_VHT_160		= 16,
	FTM_PARAMS_CHAN_INFO_HE_20		= 17,
	FTM_PARAMS_CHAN_INFO_HE_40		= 18,
	FTM_PARAMS_CHAN_INFO_HE_80		= 19,
	FTM_PARAMS_CHAN_INFO_HE_80_80		= 20,
	FTM_PARAMS_CHAN_INFO_HE_160_2_RFLOS	= 21,
	FTM_PARAMS_CHAN_INFO_HE_160		= 22,
	/* Reserved from 23 - 30 */
	FTM_PARAMS_CHAN_INFO_DMG_2160		= 31,
	/* Reserved from 32 - 63 */
	FTM_PARAMS_CHAN_INFO_MAX		= 63
};

/* the following definitions are *DEPRECATED* and moved to implementation files. They
 * are retained here because previous (May 2016) some branches use them
 */
#define FTM_TPK_LEN				16u
#define FTM_RI_RR_BUF_LEN			32u
#define FTM_TPK_RI_RR_LEN			13
#define FTM_TPK_RI_RR_LEN_SECURE_2_0		28
#define FTM_TPK_RI_PHY_LEN			7u
#define FTM_TPK_RR_PHY_LEN			7u
#define FTM_TPK_LEN_SECURE_2_0			64u
#define FTM_TPK_RI_PHY_LEN_SECURE_2_0		14u
#define FTM_TPK_RR_PHY_LEN_SECURE_2_0		14u

#define FTM_RI_RR_BUF_LEN_20MHZ			32u
#define FTM_RI_RR_BUF_LEN_80MHZ			64u

#define FTM_RI_RR_BUF_LEN_FROM_CHANSPEC(chanspec) \
	(CHSPEC_IS20((chanspec)) ? \
	FTM_RI_RR_BUF_LEN_20MHZ : FTM_RI_RR_BUF_LEN_80MHZ)

#define FTM_TPK_RI_RR_LEN_SECURE_2_0_20MHZ      28u
#define FTM_TPK_RI_RR_LEN_SECURE_2_0_80MHZ      62u
#define FTM_TPK_RI_RR_LEN_SECURE_2_0_2G		FTM_TPK_RI_RR_LEN_SECURE_2_0
#define FTM_TPK_RI_RR_LEN_SECURE_2_0_5G		FTM_TPK_RI_RR_LEN_SECURE_2_0_80MHZ

#define FTM_TPK_RI_RR_LEN_FROM_CHANSPEC(chanspec) \
	(CHSPEC_IS20((chanspec)) ? FTM_TPK_RI_RR_LEN_SECURE_2_0_20MHZ : \
	FTM_TPK_RI_RR_LEN_SECURE_2_0_80MHZ)

#define FTM_TPK_RI_PHY_LEN_SECURE_2_0_20MHZ     14u
#define FTM_TPK_RI_PHY_LEN_SECURE_2_0_80MHZ	31u

#define FTM_TPK_RI_PHY_LEN_FROM_CHANSPEC(chanspec) \
	(CHSPEC_IS20((chanspec)) ? FTM_TPK_RI_PHY_LEN_SECURE_2_0_20MHZ : \
	FTM_TPK_RI_PHY_LEN_SECURE_2_0_80MHZ)

#define FTM_TPK_RR_PHY_LEN_SECURE_2_0_20MHZ     14u
#define FTM_TPK_RR_PHY_LEN_SECURE_2_0_80MHZ	31u

#define FTM_TPK_RR_PHY_LEN_FROM_CHANSPEC(chanspec) \
	(CHSPEC_IS20((chanspec)) ? FTM_TPK_RR_PHY_LEN_SECURE_2_0_20MHZ : \
	FTM_TPK_RR_PHY_LEN_SECURE_2_0_80MHZ)
/* end *DEPRECATED* ftm definitions */

BWL_PRE_PACKED_STRUCT struct dot11_ftm_sync_info {
	uint8 id;		/* Extended - 255 11mc D4.3  */
	uint8 len;
	uint8 id_ext;
	uint8 tsf_sync_info[4];
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_ftm_sync_info dot11_ftm_sync_info_t;

/* ftm tsf sync info ie len - includes id ext */
#define DOT11_FTM_SYNC_INFO_IE_LEN (sizeof(dot11_ftm_sync_info_t) - TLV_HDR_LEN)

#define DOT11_FTM_IS_SYNC_INFO_IE(_ie) (\
	DOT11_MNG_IE_ID_EXT_MATCH(_ie, DOT11_MNG_FTM_SYNC_INFO) && \
	(_ie)->len == DOT11_FTM_SYNC_INFO_IE_LEN)

/* 11az d2.5 table 9-92 */
#define DOT11_PASN_PARAMS_EXT_ID	100u
#define DOT11_MNG_PASN_PARAMS		(DOT11_MNG_ID_EXT_ID + \
	DOT11_PASN_PARAMS_EXT_ID)

#define PASN_PARAMS_CTRL_CBINFO_PRESENT		(1u << 0)
#define PASN_PARAMS_CTRL_GROUP_KEY_PRESENT	(1u << 1)

enum {
	PASN_WRAPPED_NO_DATA	= 0,
	PASN_WRAPPED_FBT_DATA	= 1u,
	PASN_WRAPPED_FILS_DATA	= 2u,
	PASN_WRAPPED_SAE_DATA	= 3u
};

BWL_PRE_PACKED_STRUCT struct dot11_pasn_params_ie {
	uint8 id;	/* 255 */
	uint8 len;
	uint8 id_ext;	/* DOT11_PASN_PARAMS_EXT_ID */
	uint8 ctrl;
	uint8 wd_format;
	/* optional comback info, group id and public key */
} BWL_POST_PACKED_STRUCT;
typedef struct dot11_pasn_params_ie dot11_pasn_params_ie_t;

BWL_PRE_PACKED_STRUCT struct pasn_params_ap_cbinfo_field {
	uint16 after_tu;	/* ask STA to retry in TU times */
	uint8 cookie_len;	/* length of cookie */
	uint8 cookie[];		/* variable cookie */
} BWL_POST_PACKED_STRUCT;
typedef struct pasn_params_ap_cbinfo_field pasn_params_ap_cbinfo_field_t;

BWL_PRE_PACKED_STRUCT struct pasn_params_sta_cbinfo_field {
	uint8 cookie_len;	/* length of cookie */
	uint8 cookie[];		/* variable cookie */
} BWL_POST_PACKED_STRUCT;
typedef struct pasn_params_sta_cbinfo_field pasn_params_sta_cbinfo_field_t;

BWL_PRE_PACKED_STRUCT union pasn_params_cbinfo_field {
	pasn_params_ap_cbinfo_field_t ap_cbinfo;
	pasn_params_sta_cbinfo_field_t sta_cbinfo;
} BWL_POST_PACKED_STRUCT;
typedef union pasn_params_cbinfo_field pasn_params_cbinfo_field_t;

BWL_PRE_PACKED_STRUCT struct pasn_params_group_key {
	uint16 group_id;	/* IANA id of Finite Cyclic Group */
	uint8 len;		/* Public key length */
	uint8 key[];		/* public key encoded using RFC 5480 conventions */
} BWL_POST_PACKED_STRUCT;
typedef struct pasn_params_group_key pasn_params_group_key_t;

/* MIC ie */
BWL_PRE_PACKED_STRUCT struct mic_ie {
	uint8	id;		/* IE ID: DOT11_MNG_MIE_ID */
	uint8	len;		/* IE length */
	uint8	mic[];		/* mic: 16 or 24 octets */
} BWL_POST_PACKED_STRUCT;
typedef struct mic_ie mic_ie_t;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* _802_11az_h_ */
