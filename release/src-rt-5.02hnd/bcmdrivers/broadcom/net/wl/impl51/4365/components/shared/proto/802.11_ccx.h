/*
 * Fundamental types and constants relating to 802.11 ccx
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: 802.11_ccx.h 608847 2015-12-29 14:09:09Z $
 */

#ifndef _802_11_CCX_H_
#define _802_11_CCX_H_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif

#ifndef _NET_ETHERNET_H_
#include <proto/ethernet.h>
#endif


/* This marks the start of a packed structure section. */
#include <packed_section_start.h>


#define DOT11_LEAP_AUTH		0x80	/* LEAP authentication frame payload constants */

/* Management Frame Information Element IDs */
#define DOT11_MNG_AIRONET_ID		133	/* AIRONET id */
#define DOT11_MNG_CELL_PWR_ID		150	/* cell power id */
#define DOT11_MNG_CCKM_REASSOC_ID	156	/* CCKM reassoc. id */
#define DOT11_MNG_SSIDL_ID		221	/* SSIDL id */

#define CISCO_BASE			8	/* base of Cisco cipher and AKM vals */

/* AIRONET IE field offsets */
#define AIRONET_IE_REFRESH_RATE	3	/* refresh rate */
#define AIRONET_IE_CWMIN	4	/* cwmin */
#define AIRONET_IE_CWMAX	6	/* cwmax */
#define AIRONET_IE_CKIP		8	/* CKIP */
#define AIRONET_IE_NAME		10	/* offset to AP/Machine name field */
#define	AIRONET_IE_DEVICE_ID	0x66	/* Dev ID */

#define AIRONET_IE_MAX_NAME_LEN		16	/* device name length, include NULL */

BWL_PRE_PACKED_STRUCT struct aironet_assoc_ie {
	uint8	id;			/* IE ID  */
	uint8	len;		/* IE length */
	uint8	load;
	uint8	hops;
	uint8	device;
	uint8	refresh_rate;
	uint16  cwmin;
	uint16  cwmax;
	uint8	flags;
	uint8	distance;
	char	name[AIRONET_IE_MAX_NAME_LEN];	/* AP or Client's machine name */
	uint16	num_assoc;	/* number of clients associated */
	uint16	radiotype;
} BWL_POST_PACKED_STRUCT;
typedef	struct aironet_assoc_ie aironet_assoc_ie_t;

/* CKIP Negotiation bit fields */
#define	CKIP_MIC		0x08	/* MIC */
#define CKIP_KP			0x10	/* KP */
#define CKIP_LLC_SNAP_LEN		8 /* SKIP LLC SNAP header length */

#define CCX_DDP_LLC_SNAP_LEN	8	/* CCX DDP/LLC/SNAP length */
#define CCX_DDP_MSG_LEN		40	/* CCX DDP MSG length */
#define CCX_DDP_ROGUE_NAME_LEN	16	/* CCX DDP rogue name length */
BWL_PRE_PACKED_STRUCT struct ccx_ddp_pkt_s {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint16 msg_len;
	uint8  msg_type;
	uint8  fcn_code;
	struct ether_addr dest_mac;
	struct ether_addr src_mac;
	uint16 fail_reason;
	struct ether_addr rogue_mac;
	uint8  rogue_name[CCX_DDP_ROGUE_NAME_LEN];
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_ddp_pkt_s	ccx_ddp_pkt_t;
#define CCX_DDP_PKT_LEN		(ETHER_HDR_LEN + DOT11_LLC_SNAP_HDR_LEN + CCX_DDP_MSG_LEN)

#define	CCX_ROGUE_INVALID_AUTH	1	/* invalid auth. */
#define	CCX_ROGUE_LEAP_TIMEOUT	2	/* LEAP timeout */
#define CCX_ROGUE_CHAN_FROM_AP	3	/* chan. from AP */
#define	CCX_ROGUE_CHAN_TO_AP	4	/* chan. to AP */

/* Cisco/Aironet IAPP packet defs */

#define CISCO_AIRONET_OUI	"\x00\x40\x96"	/* Cisco AIRONET OUI */
#define CISCO_AIRONET_SNAP	"\xAA\xAA\x03\x00\x40\x96\x00\x00"	/* Cisco AIRONET SNAP */

#define CCX_IAPP_ID_MASK	0xf000	/* IAPP id mask */
#define CCX_IAPP_LEN_MASK	0x0fff	/* IAPP len mask */
#define CCX_IAPP_ID_SHIFT	12		/* IAPP id shift */

#define CCX_IAPP_ID_CONTROL	0x0000	/* IAPP id control */
#define CCX_IAPP_TYPE_RM	0x32	/* IAPP radio measurement request type */
#define	CCX_IAPP_TYPE_ROAM	0x33	/* IAPP roam request type */
#define CCX_IAPP_TYPE_LINK_TEST	0x41	/* IAPP link test request type */
#define CCX_IAPP_SUBTYPE_REQ	0x01	/* IAPP subtype request */
#define CCX_IAPP_SUBTYPE_ROAM_REP	0x81	/* IAPP subtype report */
#define	CCX_IAPP_SUBTYPE_ROAM_REQ	0x82	/* IAPP subtype directed roam request */
#define CCXv2_IAPP_TYPE_ROAM		0x30	/* CCXv2 IAPP roam request type */
#define	CCXv2_IAPP_SUBTYPE_ROAM_REQ	0x00	/* CCXv2 IAPP roam subtype request */
#define CCX_IAPP_TYPE_DIAG_GROUP	0x60	/* IAPP type for diag. group(s64, s65 and s66) */

/* Cisco/Aironet IAPP header */
BWL_PRE_PACKED_STRUCT struct ccx_iapp_hdr {
	uint16	id_len;		/* IAPP ID & Length */
	uint8	type;		/* IAPP Type */
	uint8	subtype;	/* IAPP Subtype */
	struct ether_addr da;
	struct ether_addr sa;
	uint8	data[1];
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_iapp_hdr ccx_iapp_hdr_t;
#define CCX_IAPP_HDR_LEN	16	/* IAPP header length */

/* CCXv2 Transmit Power Control IE */
BWL_PRE_PACKED_STRUCT struct ccx_cell_pwr {
	uint8 id;	/* 150, DOT11_MNG_CELL_PWR_ID */
	uint8 len;
	uint8 oui[3];	/* 00:40:96, CISCO_AIRONET_OUI */
	uint8 ver;	/* 0 */
	uint8 power;	/* signed int dBm */
	uint8 reserved;
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_cell_pwr ccx_cell_pwr_t;

#define CCX_RM_STATE_NORMAL	1	/* Radio Management Capability IE state */
#define CCX_RM_STATE_MBSSID_MASK	0x0700  /* MBSSID Mask field */
#define CCX_RM_STATE_MBSSID_SHIFT	8       /* MBSSID Mask field */

/* CCXv2 Radio Management Capability IE */
BWL_PRE_PACKED_STRUCT struct ccx_radio_mgmt {
	uint8 id;	/* 221, DOT11_MNG_PROPR_ID */
	uint8 len;
	uint8 oui[3];	/* 00:40:96, CISCO_AIRONET_OUI */
	uint8 ver;	/* 1 */
	uint16 state;	/* Radio Mgmt state, 1->Normal.  MBSSID mask starts at bit 8 */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_radio_mgmt ccx_radio_mgmt_t;

#define CCX_QOS_IE_TYPE		4	/* CCX QoS IE type */
#define CCX_QOS_IE_LEN		0x16	/* CCX QoS IE length */
#define CCX_VERSION_IE_TYPE	3	/* CCX version IE type */
#define CCX_VERSION_IE_LEN	5	/* CCX version IE length */
#define CCX_RM_CAP_IE_TYPE	1	/* CCX RM cap IE type */
#define CCX_EXT_CAP_IE_TYPE	11	/* CCX Extended Cap IE type */

/* CCXv2 QOS Parameter set IE */
BWL_PRE_PACKED_STRUCT struct ccx_qos_params {
	uint8 id;		/* 221, DOT11_MNG_PROPR_ID */
	uint8 len;
	uint8 oui[3];		/* 00:40:96, CISCO_AIRONET_OUI */
	uint8 type;		/* 4 */
	uint8 unused;
	uint8 count;		/* incremented when element changes */
	uint8 aifsn_0;		/* AIFSN class 0 */
	uint8 ecw_0;		/* ECWmin/ECWmax class 0 */
	uint16 txop_0;		/* TXOP Limit class 0 */
	uint8 aifsn_1;		/* AIFSN class 1 */
	uint8 ecw_1;		/* ECWmin/ECWmax class 1 */
	uint16 txop_1;		/* TXOP Limit class 1 */
	uint8 aifsn_2;		/* AIFSN class 2 */
	uint8 ecw_2;		/* ECWmin/ECWmax class 2 */
	uint16 txop_2;		/* TXOP Limit class 2 */
	uint8 aifsn_3;		/* AIFSN class 3 */
	uint8 ecw_3;		/* ECWmin/ECWmax class 3 */
	uint16 txop_3;		/* TXOP Limit class 3 */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_qos_params ccx_qos_params_t;

/* CCXv2 Version IE */
BWL_PRE_PACKED_STRUCT struct ccx_version_ie {
	uint8 id;      		/* 221, DOT11_MNG_PROPR_ID */
	uint8 len;
	uint8 oui[DOT11_OUI_LEN];	/* 00:40:96, CISCO_AIRONET_OUI */
	uint8 type;		/* 3 */
	uint8 version;
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_version_ie ccx_version_ie_t;

/* CCX Extended Capability IE */
BWL_PRE_PACKED_STRUCT struct ccx_ext_cap_ie {
	uint8 id;		/* 221, DOT11_MNG_PROPR_ID */
	uint8 len;
	uint8 oui[DOT11_OUI_LEN];	/* 00:40:96, CISCO_AIRONET_OUI */
	uint8 type;		/* 11 */
	uint8 cap;
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_ext_cap_ie ccx_ext_cap_ie_t;
#define CCX_CAP_FBT		0x40	/* 802.11r support */

/* CCX Adjacent AP Report IE in roam IAPP packet */
BWL_PRE_PACKED_STRUCT struct ccx_roam_ap_ie_s {
	uint16 tag;		/* 0x9b - Adjacent AP report */
	uint16 len;
	uint8 oui[DOT11_OUI_LEN];	/* Aironet OUI 0x00 0x40 0x96 0x00 */
	uint8 ver;
	struct ether_addr mac;	/* MAC address of AP */
	uint16 channel;
	uint16 ssid_len;
	uint8 ssid[32];
	uint16 disassoc_time;	/* Seconds that the client has been disassociated */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_roam_ap_ie_s ccx_roam_ap_ie_t;
#define CCX_ROAM_AP_IE_LEN	52	/* CCX roam AP IE length */

/* CCX IAPP packet sent to AP on association with info on previous AP */
BWL_PRE_PACKED_STRUCT struct ccx_roam_iapp_pkt_s {
	struct ether_header eth;
	struct dot11_llc_snap_header snap;
	uint16 msg_len;
	uint8  msg_type;
	uint8  fcn_code;
	struct ether_addr dest_mac;
	struct ether_addr src_mac;
	ccx_roam_ap_ie_t ap_ie;
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_roam_iapp_pkt_s	ccx_roam_iapp_pkt_t;

/* CCX roam IAPP message length */
#define CCX_ROAM_IAPP_MSG_SIZE 	(sizeof(ccx_roam_iapp_pkt_t) - sizeof(struct ether_header))
#define CCX_ROAM_ADJ_AP_TAG		0x9b	/* CCX adjacent AP tag */

/* CCXv4 S51 */
/* CCX roam reason IE in roam IAPP packet */
BWL_PRE_PACKED_STRUCT struct ccx_roam_reason_ie {
	uint16 tag;		/* 0x9c - roam reason tag */
	uint16 len;
	uint8 oui[DOT11_OUI_LEN];	/* Aironet OUI 0x00 0x40 0x96 0x00 */
	uint8 ver;
	uint8 reason;		/* roam reason */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_roam_reason_ie ccx_roam_reason_ie_t;
#define CCX_ROAM_REASON_IE_LEN	8	/* CCX roam reason IE length */

/* CCX roam neighbor report IE in roam IAPP packet */
BWL_PRE_PACKED_STRUCT struct ccx_neighbor_rept_ie {
	uint8	id;		/* CCX_ROAM_NEIGHBOR_REPT_ID */
	uint8	len;		/* length beyond len */
	struct ether_addr mac;	/* MAC address of neighbor AP */
	uint8	channel;	/* current channel of neighbor AP */
	uint8	band;		/* band of current channel of neighbor AP */
	uint8	phy_type;	/* PHY type of current channel of neighbor AP */
	/* variable subelements follows */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_neighbor_rept_ie ccx_neighbor_rept_ie_t;
#define CCX_NEIGHBOR_REPT_IE_LEN	11	/* CCX neighbor report IE length */
#define CCX_NEIGHBOR_REPT_IE_LEN_W_H	9	/* CCX_NEIGHBOR_REPT_IE_LEN - header (2) */
/* RF parameter subelement for neighbor ie */
BWL_PRE_PACKED_STRUCT struct ccx_radio_param_subie {
	uint8	sub_id;		/* subelement id */
	uint8	len;		/* length beyond len */
	int8	min_rssi;	/* min. recv pwr in dBm required to associate with the AP */
	int8	ap_tx_pwr;	/* tx pwr in dBm of neighbor AP */
	int8	sta_tx_pwr;	/* tx pwr in dBm of sta advertised by neighbor AP */
	int8	roam_delta;	/* roam delta */
	int8	roam_trigger;	/* roam trigger */
	uint8	roam_time;	/* transition time(in 0.1s) permmited in roam */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_radio_param_subie ccx_radio_param_subie_t;
#define CCX_RADIO_PARAM_SUBIE_LEN	8	/* CCX RF parameter sub IE length */

/* TSF info. subelement for neighbor ie */
BWL_PRE_PACKED_STRUCT struct ccx_tsf_info_subie {
	uint8	sub_id;		/* subelement id */
	uint8	len;		/* length beyond len */
	BWL_PRE_PACKED_STRUCT struct {
		uint16 offset;	/* TSF time offset in TUs between serving AP and this AP */
		uint16 bcn_interval;	/* beacon interval of AP */
	} BWL_POST_PACKED_STRUCT TSF;
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_tsf_info_subie ccx_tsf_info_subie_t;
#define CCX_TSF_INFO_SUBIE_LEN	6	/* CCX TSF info. sub IE length */

/* subelement id */
#define	CCX_ROAM_SUB_RF_PARAMS	1	/* CCX neighbor element RF subelement id */
#define	CCX_ROAM_SUB_TSF_INFO	2	/* CCX neighbor element TSF subelement id */

/* roam reason */
#define CCX_ROAM_UNSPECIFIED	0	/* unspecified */
#define CCX_ROAM_NORMAL		1	/* poor link(excessive retries, RSSI too low...) */
#define CCX_ROAM_LOAD_BALANCING	2	/* normal roam, load balancing */
#define CCX_ROAM_AP_INCAPACITY	3	/* AP has insufficient capacity(TSPEC rejected) */
#define CCX_ROAM_DIRECTED_ROAM	4	/* Infrastructure directed roam */
#define CCX_ROAM_FIRST_ASSOC	5	/* first association to WLAN */
#define CCX_ROAM_IN_NET		6	/* roaming in from cellular or other WAN */
#define CCX_ROAM_OUT_NET	7	/* roaming out to cellular or other WAN */
#define CCX_ROAM_BETTER_AP	8	/* normal roaming, better AP found */
#define CCX_ROAM_LINK_DOWN	9	/* deauthnticated or disassociated from the previous AP */

/* band */
#define	CCX_CHAN_BAND_2G	0	/* 2.4 GHz band */
#define	CCX_CHAN_BAND_5G	1	/* 5 GHz band */

#define CCX_ROAM_NEIGHBOR_REPT_ID	0x28	/* CCX roam neighbor report id */

#define CCX_ROAM_REASON_TAG	0x9c	/* roam reason tag */

#if defined(BCMSUP_PSK) || !defined(BCMINTSUP)
#define CCKM_KRK_LEN		16	/* CCKM KRK length */
#define CCKM_BTK_LEN		32	/* CCKM BTK length */

#define CCKM_OUI_TYPE		0	/* CCKM OUI type */
#define CCKM_RSC_LEN		8	/* CCKM RSC length */
#define	CCKM_MIC_LEN		8	/* CCKM MIC length */

BWL_PRE_PACKED_STRUCT struct cckm_reassoc_req_ie_s {
	uint8	id;			/* DOT11_MNG_CCKM_REASSOC_ID */
	uint8	len;			/* length beyond len */
	uint8	oui[DOT11_OUI_LEN];	/* expect AIRONET_OUI */
	uint8	oui_type;		/* expect CCKM_OUI_TYPE */
	uint8	timestamp[DOT11_MNG_TIMESTAMP_LEN];	/* TSF timer value (LE) */
	uint32 rn;				/* reassociation request number (LE) */
	uint8	mic[CCKM_MIC_LEN];	/* MIC computed using KRK */
} BWL_POST_PACKED_STRUCT;
typedef struct cckm_reassoc_req_ie_s cckm_reassoc_req_ie_t;
#define CCKM_REASSOC_REQ_IE_LEN		26	/* CCKM reassoc. request IE length */

BWL_PRE_PACKED_STRUCT struct cckm_reassoc_resp_ie_s {
	uint8	id;			/* DOT11_MNG_CCKM_REASSOC_ID */
	uint8	len;			/* length beyond len */
	uint8	oui[DOT11_OUI_LEN];	/* expect AIRONET_OUI */
	uint8	oui_type;		/* expect CCKM_OUI_TYPE */
	uint32	rn;			/* rekey value */
	uint8	ucast_idx;		/* ucast key index; expect 0 */
	uint8	mcast_idx;		/* mcast key index */
	uint8	rsc[CCKM_RSC_LEN];	/* mcast RSC */
	uint16	gtklen;			/* mcast key len */
	uint8	mic[CCKM_MIC_LEN];	/* msg integrity code */
	uint8	egtk[1];		/* encrypted group key */
} BWL_POST_PACKED_STRUCT;
typedef struct cckm_reassoc_resp_ie_s cckm_reassoc_resp_ie_t;
#define CCKM_REASSOC_RESP_IE_LEN	31	/* CCKM reassoc. response IE length */

#endif /* BCMSUP_PSK || !BCMINTSUP */

/* CCX Radio Managment definitions, CCX spec section S36 */

#define CCX_RM_IE_HDR_LEN	4	/* length of Radio Mgmt IE ID plus Len */

/* Radio Measurement IE IDs */
#define CCX_RM_ID_REQUEST	38	/* CCX radio measurement request id */
#define CCX_RM_ID_REPORT	39	/* CCX radio measurement respond id */

/* Radio Measurement Request Type field */
#define CCX_RM_TYPE_LOAD	1	/*  CCX radio measurement load request */
#define CCX_RM_TYPE_NOISE	2	/*  CCX radio measurement noise request */
#ifndef CCX_SDK	/* CCX SDK defined the same RM types with the same names */
#define CCX_RM_TYPE_BEACON	3	/*  CCX radio measurement beacon request */
#define CCX_RM_TYPE_FRAME	4	/*  CCX radio measurement frame request */
#endif /* CCX_SDK */
#define CCXv4_RM_TYPE_PATHLOSS	6	/*  CCX radio measurement PathLoss request for CCXv4 */
#define CCX_RM_TYPE_PATHLOSS	9	/*  CCX radio measurement PathLoss request */
#define CCX_RM_TYPE_STATISTICS	10	/* CCX radio measurement Statistics request */

/* Radio Measurement Request Mode field */
#define CCX_RM_MODE_PARALLEL	(1<<0)	/*  CCX radio measurement parallel request */
#define CCX_RM_MODE_ENABLE	(1<<2)	/*  CCX radio measurement autonomous request */
#define CCX_RM_MODE_REPORT	(1<<3)	/*  CCX radio measurement report */
/* Radio Measurement Report Modes */
#define CCX_RM_MODE_INCAPABLE	(1<<1)	/*  CCX radio measurement incapable */
#define CCX_RM_MODE_REFUSED	(1<<2)	/*  CCX radio measurement refused */

/* Radio Measurement Beacon scan types */
#define CCX_RM_BEACON_PASSIVE_SCAN	0	/*  CCX radio measurement beacon passive scan */
#define CCX_RM_BEACON_ACTIVE_SCAN	1	/*  CCX radio measurement beacon active scan */
#define CCX_RM_BEACON_TABLE		2	/*  CCX radio measurement beacon table */

/* Radio Measurement Beacon PHY Types */
#define CCX_RM_PHY_FH		1	/*  CCX radio measurement FH */
#define CCX_RM_PHY_DSS		2	/*  CCX radio measurement DSS */
#define CCX_RM_PHY_OFDM		4	/*  CCX radio measurement OFDM */
#define CCX_RM_PHY_HRDSS	5	/*  CCX radio measurement HRDSS */
#define CCX_RM_PHY_ERP		6	/*  CCX radio measurement ERP */

/* CCXv2 Radio Measurment Request frame
 * Encapsulated in Cisco Aironet IAPP frame
 */
BWL_PRE_PACKED_STRUCT struct ccx_rm_req {
	uint16		token;		/* Dialog Token */
	uint8		delay;		/* Request Activation Delay (TBTTs) */
	uint8		offset;		/* Request Activation Offset (TUs) */
	uint8		data[1];	/* Request information elts */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_req ccx_rm_req_t;
#define CCX_RM_REQ_LEN 4	/*  CCX radio measurement request header length */

/* CCXv2 Radio Measurment Report frame header
 * Encapsulated in Cisco Aironet IAPP frame
 */
BWL_PRE_PACKED_STRUCT struct ccx_rm_rep {
	uint16		token;		/* Dialog Token */
	uint8		data[1];	/* Report information elts */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_rep ccx_rm_rep_t;
#define CCX_RM_REP_LEN		2	/*  CCX radio measurement reponse header length */
/* Max length of CCXv2 Radio Measurment Report data section based on
 * the maximum size of the LLC_SNAP encapsulating frame.
 */
#define CCX_RM_REP_DATA_MAX_LEN	\
	(ETHER_MAX_DATA - (DOT11_LLC_SNAP_HDR_LEN +	CCX_IAPP_HDR_LEN +	\
	CCX_RM_REP_LEN))		/*  CCX radio measurement max report data length */

/* CCXv2 Radio Measurment Request IE
 * Encapsulated in a CCXv2 Radio Measurment Request frame
 */
BWL_PRE_PACKED_STRUCT struct ccx_rm_req_ie {
	uint16	id;
	uint16	len;
	uint16	token;		/* Dialog Token */
	uint8	mode;
	uint8	type;
	/* end of fixed portion */
	/* variable data, depends on mode and type */
	uint8	channel;	/* channel for the measurment */
	uint8	param;		/* measurement parameter */
	uint16	duration;	/* measurement duration (TUs) */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_req_ie ccx_rm_req_ie_t;
#define CCX_RM_REQ_IE_FIXED_LEN	8	/*  CCX radio measurement request IE fixed length */

/* CCXv2 Radio Measurment Report IE
 * Encapsulated in a CCXv2 Radio Measurment Report frame
 */
BWL_PRE_PACKED_STRUCT struct ccx_rm_rep_ie {
	uint16	id;
	uint16	len;
	uint16	token;
	uint8	mode;
	uint8	type;
	uint8	data[1];
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_rep_ie ccx_rm_rep_ie_t;
#define CCX_RM_REP_IE_FIXED_LEN	8	/*  CCX radio measurement report IE fixed length */

BWL_PRE_PACKED_STRUCT struct ccx_rm_beacon_rep {
	uint8	channel;
	uint8	spare;
	uint16	duration;
	uint8	phy_type;
	int8	rssi;
	struct ether_addr bssid;
	uint32	parent_tsf;
	uint32	target_tsf_low;
	uint32	target_tsf_hi;
	uint16	beacon_interval;
	uint16	capability;
	uint8	data[1];
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_beacon_rep ccx_rm_beacon_rep_t;
#define CCX_RM_BEACON_REP_FIXED_LEN	28	/*  CCX rm beacon report fixed length */

BWL_PRE_PACKED_STRUCT struct ccx_rm_frm_rep_elt {
	struct ether_addr ta;	/* transmitter address */
	struct ether_addr bssid;	/* bssid transmitter belongs to */
	uint8	rssi;	/* average RSSI */
	uint8	frames;	/* total number of frames */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_frm_rep_elt ccx_rm_frm_rep_elt_t;
#define CCX_RM_FRAME_REP_ENTRY_LEN	14	/*  CCX rm frame report entry length */

BWL_PRE_PACKED_STRUCT struct ccx_rm_frame_rep {
	uint8	channel;
	uint8	spare;
	uint16	duration;
	ccx_rm_frm_rep_elt_t	elt[1];
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_frame_rep ccx_rm_frame_rep_t;
#define CCX_RM_FRAME_REP_FIXED_LEN	4	/*  CCX rm frame report fixed length */

BWL_PRE_PACKED_STRUCT struct ccx_rm_load_rep {
	uint8	channel;
	uint8	spare;
	uint16	duration;
	uint8	fraction;
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_load_rep ccx_rm_load_rep_t;
#define CCX_RM_LOAD_REP_LEN	5	/*  CCX radio measurement load report length */

BWL_PRE_PACKED_STRUCT struct ccx_rm_noise_rep {
	uint8	channel;
	uint8	spare;
	uint16	duration;
	uint8	rpi[8];
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_noise_rep ccx_rm_noise_rep_t;
#define CCX_RM_NOISE_REP_LEN	12	/*  CCX radio measurement noise report length */

/* SSIDL Elements.  CCXv4 S53 */
#define SSIDL_OUI		"\x00\x50\xf2"	/* SSIDL OUI */
#define SSIDL_OUI_LEN		3	/* SSIDL OUI length */
#define SSIDL_TYPE		5	/* SSIDL type */

/* extended capability bits */
#define PRI_EXT_CAP_8021X	1	/* primary ssid supports 802.1x */
#define PRI_EXT_CAP_WPS		2	/* primary ssid supports WPS */

/* hidden ssid capability field */
#define SEC_CAP_MC_CIPHER_MASK	0x0000000f	/* mcast cipher mask */
#define SEC_CAP_UC_CIPHER_MASK	0x000efff0	/* ucast cipher mask */
#define SEC_CAP_AKM_MASK	0x7e000000	/* AKM mask */
#define SEC_CAP_UC_CIPHER_SHIFT	4	/* ucast cipher shift */
#define SEC_CAP_AKM_SHIFT	25	/* AKM shift */

/* SEC_CAP_MC_CIPHER value */
enum {
	MC_CIPHER_NONE,
	MC_CIPHER_WEP40,
	MC_CIPHER_WEP104,
	MC_CIPHER_TKIP,
	MC_CIPHER_CCMP,
	MC_CIPHER_CKIP_CMIC,
	MC_CIPHER_CKIP,
	MC_CIPHER_CMIC
};

/* SEC_CAP_UC_CIPHER value */
#define UC_CIPHER_NONE		(1 << 0)
#define UC_CIPHER_WEP40		(1 << 1)
#define UC_CIPHER_WEP104	(1 << 2)
#define UC_CIPHER_TKIP		(1 << 3)
#define UC_CIPHER_CCMP		(1 << 4)
#define UC_CIPHER_CKIP_CMIC	(1 << 5)
#define UC_CIPHER_CKIP		(1 << 6)
#define UC_CIPHER_CMIC		(1 << 7)
#define UC_CIPHER_WPA2_WEP40	(1 << 8)
#define UC_CIPHER_WPA2_WEP104	(1 << 9)
#define UC_CIPHER_WPA2_TKIP	(1 << 10)
#define UC_CIPHER_WPA2_CCMP	(1 << 11)
#define UC_CIPHER_WPA2_CKIP_CMIC	(1 << 12)
#define UC_CIPHER_WPA2_CKIP	(1 << 13)
#define UC_CIPHER_WPA2_CMIC	(1 << 14)

/* SEC_CAP_AKM value */
#define AKM_WPA1_1X		(1 << 0)
#define AKM_WPA1_PSK		(1 << 1)
#define AKM_WPA2_1X		(1 << 2)
#define AKM_WPA2_PSK		(1 << 3)
#define AKM_WPA1_CCKM		(1 << 4)
#define AKM_WPA2_CCKM		(1 << 5)

BWL_PRE_PACKED_STRUCT struct ccx_hidden_ssid {
	uint8	ext_cap;	/* hidden ssid extended capability */
	uint32	capability;	/* hidden ssid capability */
	uint8	ssid_len;	/* ssid name length */
	uint8	ssid[1];	/* ssid.  variable length */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_hidden_ssid ccx_hidden_ssid_t;

/* CCXv4 SSIDL IE */
BWL_PRE_PACKED_STRUCT struct ccx_ssidl_ie {
	uint8	id;		/* 221, DOT11_MNG_SSIDL_ID */
	uint8	length;
	uint8 	oui[3];		/* 00:50:f2 */
	uint8	type;		/* 5 */
	uint8	pri_ext_cap;	/* primary ssid extended capability */
	/* following fields are optional */
	uint8	ssid_count;	/* number of hidden ssids followed */
	ccx_hidden_ssid_t	hidden_ssid[1];	/* hidden ssid start */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_ssidl_ie ccx_ssidl_ie_t;

/* link test.  CCXv4 S62 */
BWL_PRE_PACKED_STRUCT struct ccx_link_test_s {
	uint16	frm_num;	/* frame number */
	uint32	time;		/* time from sender */
	uint8	rsq;		/* raw signal quality of request */
	uint8	rss;		/* raw signal strength of request */
	uint8	txretried;	/* retries in sending previous response */
	uint8	rssi;		/* signal strength in dBm of request */
	uint8	sqp;		/* signal quality as percent of request */
	uint8	ssp;		/* signal strength as percent of request */
	uint8	data[1];	/* start data set by sender */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_link_test_s ccx_link_test_t;
#define CCX_LINK_TEST_REQ_LEN 12	/* line test request length */

/* Supported Features Advertisement(SFA) IE.  CCXv5 S63 */
BWL_PRE_PACKED_STRUCT struct ccx_sfa_ie {
	uint8	id;		/* 221, DOT11_MNG_PROPR_ID */
	uint8	length;		/* 5 */
	uint8 	oui[3];		/* 00:40:96 */
	uint8	type;		/* 14 */
	uint8	capability;	/* capability to support CCXv5 features */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_sfa_ie ccx_sfa_ie_t;
#define CCX_SFA_IE_TYPE	0x14	/* SFA IE type */
/* feature bit in capability field */
#define CAP_MFP			0x01	/* S67. MFP */
#define CAP_DIAG_CHANL	0x02	/* S64. Diagnostic channel */
#define CAP_LOC_SERVICE	0x04	/* S69. Location services */
#define CAP_EXP_BNDWITH	0x08	/* S70. Expedited Bandwidth Requests */

/* MHDR IE.  CCXv5 S67 */
BWL_PRE_PACKED_STRUCT struct ccx_mhdr_ie {
	uint8	id;		/* 221, DOT11_MNG_PROPR_ID */
	uint8	length;		/* 12 */
	uint8 	oui[3];		/* 00:40:96 */
	uint8	type;		/* 16 */
	uint8	fc[2];
	struct ether_addr bssid;
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_mhdr_ie ccx_mhdr_ie_t;
#define CCX_HMDR_IE_TYPE	16

#define CKIP_MIC_SIZE		4	/* size of CKIP MIC */
#define CKIP_SEQ_SIZE		4	/* size of CKIP SEQ */
#define CKIP_KEY_SIZE		16	/* size of CKIP key */

/* 802.11e MSDU Lifetime IE */
BWL_PRE_PACKED_STRUCT typedef struct {
	uint8	oui[DOT11_OUI_LEN];	/* 3 bytes OUI */
	uint8	oui_type;		/* OUI Type */
	uint8	tid;			/* traffic stream ID */
	uint16	msdu_lifetime;	/* time in TU, specify per AC */
} BWL_POST_PACKED_STRUCT ccx_msdu_lifetime_t;

/* CCX traffic stream rateset IE */
BWL_PRE_PACKED_STRUCT typedef struct {
	uint8	oui[DOT11_OUI_LEN];	/* 3 bytes OUI */
	uint8	oui_type;		/* OUI Type */
	uint8	tid;			/* traffic stream ID */
	uint8	rateset[1];		/* nominal rate to use (multiple of 500k) */
} BWL_POST_PACKED_STRUCT ccx_stream_rs_t;
#define CAC_STREAM_RS_HDR_LEN	(DOT11_OUI_LEN + 2)	/* len from oui to tid */

/* CCX traffic stream metrics IE */
BWL_PRE_PACKED_STRUCT typedef struct {
	uint8	oui[DOT11_OUI_LEN];	/* 3 bytes OUI */
	uint8	oui_type;		/* OUI Type */
	uint8	tid;			/* traffic stream ID */
	uint8	state;			/* metrics enable or disable */
	uint16	measure_interval;	/* interval time in TU */
} BWL_POST_PACKED_STRUCT ccx_ts_metrics_t;

#define CCX_CAC_TS_METRICS_TYPE		7	/* Traffic Stream Metrics type */
#define CCX_CAC_TS_RATESET_TYPE		8	/* Traffic Stream Rateset type */
#define CCX_CAC_MSDU_LIFETIME_TYPE	9	/* MSUD Lifetime type */

/* CCX Traffic Stream Metrics IE in IAPP packet */
BWL_PRE_PACKED_STRUCT typedef struct {
	uint16 dialog_token;	/* dialog token should set to 0 */
	uint16 id;		/* ID should be 0x27 */
	uint16 len;		/* len start from token to end of struct */
	uint16 token;		/* token should set to 0 */
	uint8 mode;		/* mode should set to 0 */
	uint8 type;		/* Traffic Stream Metrics type set to 6 */
	uint16 avg_delay;	/* average delay in msec */
	uint16 cnt_delay10;	/* bucket for <= 10 msec delay */
	uint16 cnt_delay20;	/* bucket for > 10 && <= 20 msec delay */
	uint16 cnt_delay40;	/* bucket for >20 && <= 40 msec delay */
	uint16 cnt_delay;	/* bucket for > 40 msec delay */
	uint32 media_delay;	/* average media delay in TU */
	uint16 pkt_loss;	/* packet loss per AC */
	uint16 pkt_cnt;		/* packet count per AC */
	uint8 roam_cnt;		/* roam count */
	uint16 roam_delay;	/* roam delay measure in TU */
	/* following fields are added in CCXv5 */
	uint16 used_time;	/* defined in section 3.5.1 in the WMM */
	uint8 tid;			/* traffic stream ID */
} BWL_POST_PACKED_STRUCT ccx_tsm_param_t;

/* ccx_tsm_param_t defines */
#define CCX_TSM_LEN_DELTA	3	/* length delta between V4 and later versions */
#define CCX_TSM_LEN	(sizeof(ccx_tsm_param_t))	/* length */
#define CCX_TSM_V4_LEN	(CCX_TSM_LEN - CCX_TSM_LEN_DELTA)
#define CCX_TSM_IE_LEN	28	/* length (from token to end of struct) */
#define CCX_TSM_IE_V4_LEN	(CCX_TSM_IE_LEN - CCX_TSM_LEN_DELTA)
#define CCX_TSM_ID  	0x27	/* ID */
#define CCX_TSM_TOKEN	0	/* Token */
#define CCX_TSM_MODE	0	/* Mode */
#define CCX_TSM_TYPE	8	/* Type */
#define CCX_TSM_V4_TYPE	6	/* Type for V4 */

/* CCX IAPP packet sent to AP on ts metrics interval */
#define CCX_TSM_IAPP_PKT_LEN	(DOT11_LLC_SNAP_HDR_LEN + CCX_TSM_IAPP_LEN)
#define CCX_TSM_IAPP_PKT_V4_LEN	(DOT11_LLC_SNAP_HDR_LEN + CCX_TSM_IAPP_V4_LEN)
#define CCX_TSM_IAPP_LEN	(CCX_IAPP_HDR_LEN + CCX_TSM_LEN)
#define CCX_TSM_IAPP_V4_LEN	(CCX_IAPP_HDR_LEN + CCX_TSM_V4_LEN)
#define CCX_TSM_IAPP_SUBTYPE	0x81	/* CCX IAPP Subtype for Traffic Stream Metrics */

/* CCXv4 S60 PathLoss Measurement req */
BWL_PRE_PACKED_STRUCT struct ccx_rm_pathlossreq {
	uint16			nbursts;	/* Number of Bursts */
	uint16			burstinterval;	/* Burst Interval in seconds */
	uint8 			burstlen;	/* Burst len */
	uint16			duration;	/* Measurement Duration in TUs 1024 us */
	uint8			txpower;	/* Desired txpower -128 to +127dBm */
	struct ether_addr 	addr;		/* multicast Address for the report */
	uint8			nchannels;	/* number of Channels */
	uint8			channel[1];	/* Channel list */
} BWL_POST_PACKED_STRUCT;
typedef struct ccx_rm_pathlossreq ccx_rm_pathlossreq_t;
#define CCX_RM_PATHLOSSREQ_FIXEDLEN	15

/* CCXv4 S60 PathLoss Measurement Frame */
BWL_PRE_PACKED_STRUCT struct ccx_rm_pathlossmeas_frame {
	uint16 			seq;
	int8 			txpower;
	uint8 			txchannel;
} BWL_POST_PACKED_STRUCT;
#define CCX_RM_PATHLOSS_SEQ_MASK		0x0FFF
#define CCX_RM_IAPP_SUBTYPE	0x82	/* CCX IAPP Subtype for Pathloss Measurement */


/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif	/* _802_11_CCX_H_ */
