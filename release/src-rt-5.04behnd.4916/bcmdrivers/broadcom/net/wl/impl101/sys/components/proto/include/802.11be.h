/*
 * Basic types and constants relating to 802.11be/EHT standards.
 * This is a portion of 802.11be definition. The rest are in 802.11.h.
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
 * $Id: 802.11be.h 828078 2023-08-01 12:22:28Z $
 */

#ifndef _802_11be_h_
#define _802_11be_h_

#include <typedefs.h>

/* ======================= define protocol structures/constants ======================= */

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* *********************************
 * ***********  RNR IE  ************
 * *********************************
 */

/* See fils.h for RNR IE structures definitions */

/* MLD Parameters */
/* D0.4 Figure 9-632b */
#define EHT_MLD_MLD_ID_POS		0u	/* MLD ID */
#define EHT_MLD_MLD_ID_SZ		8u
#define EHT_MLD_LINK_ID_POS		8u	/* Link ID */
#define EHT_MLD_LINK_ID_SZ		4u
#define EHT_MLD_CHG_SEQ_POS		12u	/* Change Sequence */
#define EHT_MLD_CHG_SEQ_SZ		8u

/* *********************************
 * **********  EHT OP IE  **********
 * *********************************
 */

/* EHT Operation element (fixed portion) */
/* D2.0 Figure 9-1002a */
#define EHT_MCS_MAP_BASIC_LEN		4u
BWL_PRE_PACKED_STRUCT struct eht_op_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	uint8 parms;				/* EHT Operation Parameters */
	uint8 basic_mcs_nss_set[EHT_MCS_MAP_BASIC_LEN];
	/*
	 * followed by:
	 * - optional: EHT Operation Information
	 * - optional: disabled channel submap
	 */
} BWL_POST_PACKED_STRUCT;
typedef struct eht_op_ie eht_op_ie_t;

#define EHT_OP_PARMS_OP_INFO_PST_IDX		0u	/* EHT Operation Information Present */
#define EHT_OP_PARMS_DISA_CH_BMP_PST_IDX	1u	/* Disabled Subchannel Bitmap Present */
#define EHT_OP_PARMS_DEF_PE_DUR_IDX		2u	/* EHT Default PE Duration */
#define EHT_OP_PARMS_GRP_ADDR_BU_IND_LMT_IDX	3u	/* Group Addressed BU Indication Limit */
#define EHT_OP_PARMS_GRP_ADDR_BU_IND_EXP_POS	4u	/* Group Addressed BU Indication Exponent */
#define EHT_OP_PARMS_GRP_ADDR_BU_IND_EXP_SZ	2u

/* EHT Operation Information field format per 11be D3.0 Figure 9-1002c */
#define EHT_OP_INFO_MIN_LEN		3u	/* minimum length excluding disabled subchan bmp */

/* EHT Operation Information subfields */
#define EHT_OP_INFO_CW_POS		0u	/* Channel Width */
#define EHT_OP_INFO_CW_SZ		3u	/* three bits */
#define EHT_OP_INFO_CCFS0_POS		8u	/* CCFS0 */
#define EHT_OP_INFO_CCFS0_SZ		8u	/* eight bits */
#define EHT_OP_INFO_CCFS1_POS		16u	/* CCFS1 */
#define EHT_OP_INFO_CCFS1_SZ		8u	/* eight bits */
#define EHT_OP_INFO_DIS_CH_BMP_LEN	2u	/* optional two bytes Disabled Subchannel Bitmap */

/* IEEE 802.11be D3.0 Table 9-401a EHT Operation Information subfields */
typedef enum eht_op_info_chan_width {
	EHT_OP_INFO_CW_20MHZ	= 0u,
	EHT_OP_INFO_CW_40MHZ	= 1u,
	EHT_OP_INFO_CW_80MHZ	= 2u,
	EHT_OP_INFO_CW_160MHZ	= 3u,
	EHT_OP_INFO_CW_320MHZ	= 4u
} eht_op_info_chan_width_t;

/* Common Info subfields lengths (when subfield present) */
/* IEEE 802.11be D1.5 Figure 9-1002h Common Info field
 * of the Basic variant Multi-Link element format
 */
#define EHT_ML_IE_FIXED_LEN		12u	/* Multilink element(5) +
						 * Fixed common info fields(7)
						 */
#define EHT_ML_CTL_LEN			2u	/* Multi-link Control Len */
#define EHT_ML_COMMON_INFO_LEN		1u	/* Common Info Len */
#define EHT_ML_LINK_ID_LEN              1u      /* Link ID subfield */
#define EHT_ML_BSS_CHG_LEN              1u      /* BSS Parms Change Count subfield */
#define EHT_ML_MSDI_LEN                 2u      /* Medium Sync Delay Info subfield */
#define EHT_ML_EML_CAP_LEN              2u      /* EML Capabilities subfield */
#define EHT_ML_MLD_CAP_LEN              2u      /* MLD Capabilities subfield */

#define EHT_PRB_REQ_ML_IE_FIXED_LEN	6u	/* (ML IE(5) + Fixed Common info field(1)) */

/* IEE 802.11be D1.5 Figure 9-1002r Common info field of the probe request multi link
 * element format
 */
#define EHT_ML_MLD_ID_LEN		1u	/* MLD ID filed */
/* *********************************
 * ************ ML IE  *************
 * *********************************
 */
/* Multi-Link element (fixed portion) */
/* D1.5 Figure 9-1002e */
typedef BWL_PRE_PACKED_STRUCT struct eht_ml_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	uint16 ml_ctrl;
	/* followed by variable 'common info' field */
	/* followed by variable 'link info' field */
} BWL_POST_PACKED_STRUCT eht_ml_ie_t;

/* Multi-Link Control field element */
/* D1.5 Figure 9-1002f */
#define EHT_ML_CTL_TYPE_POS		0u	/* type */
#define EHT_ML_CTL_TYPE_SZ		3u
/* Presence Bitmap subfield */
/* IEEE P802.11be D1.5 Figure 9-1002g Presence Bitmap subfield
 * of the Basic variant Multi-Link element format
 */

#define EHT_ML_CTL_LINK_ID_PST_POS      4u      /* Link ID Info Present */
#define EHT_ML_CTL_LINK_ID_PST_SZ       1u
#define EHT_ML_CTL_BSS_CHG_PST_POS      5u      /* BSS Parms Chg Cnt Present */
#define EHT_ML_CTL_BSS_CHG_PST_SZ       1u
#define EHT_ML_CTL_MSDI_PST_POS         6u      /* Medium Sync Dly Info Present */
#define EHT_ML_CTL_MSDI_PST_SZ          1u
#define EHT_ML_CTL_EML_CAP_PST_POS      7u      /* EML Capabilities Present */
#define EHT_ML_CTL_EML_CAP_PST_SZ       1u
#define EHT_ML_CTL_MLD_CAP_PST_POS      8u      /* MLD Capabilities Present */
#define EHT_ML_CTL_MLD_CAP_PST_SZ       1u

/* D1.5 Table 9-401c Type subfield encoding */
#define EHT_ML_CTL_TYPE_BASIC		0u	/* Basic */
#define EHT_ML_CTL_TYPE_PRBREQ		1u	/* Probe Request */
#define EHT_ML_CTL_TYPE_RECONFIG	2u	/* Probe Request */

/* IEEE P802.11be D1.5 Figure 9-1002q Presence Bitmap subfield
 * of the probe request multi-Link element format
 */
#define EHT_ML_CTL_MLD_ID_PST_POS      4u      /* MLD ID Present */
#define EHT_ML_CTL_MLD_ID_PST_SZ       1u

#define EHT_ML_CTL_MLD_ID_PRBRESP_PST_POS      9u
#define EHT_ML_CTL_MLD_ID_PRBRESP_PST_SZ       1u

/* Link ID subfield */
/* IEEE P802.11be D1.5 Figure 9-1002i Link ID info subfield format */
#define EHT_ML_LINK_ID_POS              0u
#define EHT_ML_LINK_ID_SZ               4u

/* Medium Synchronization Delay Information subfield */
/* Figure 9-1002j Medium Synchronization Delay Information subfield format */
#define EHT_MSDI_DUR_POS                0u      /* Medium Sync Duration */
#define EHT_MSDI_DUR_SZ                 8u
#define EHT_MSDI_OFDM_ED_POS            8u      /* Medium Sync OFDM ED Threshold */
#define EHT_MSDI_OFDM_ED_SZ             4u
#define EHT_MSDI_MAX_TXOP_POS           12u     /* Medium Sync Max. Num of TXOPs */
#define EHT_MSDI_MAX_TXOP_SZ            4u

#define EHT_EML_CAP_EMLSR_SUP_MASK       0x0001       /* EMLSR Support subfield */
#define EHT_EML_CAP_EMLMR_SUP_MASK       0x0080       /* EMLMR Support subfield */

/* EML Capabilities subfield */
/* IEEE P802.11be D1.5 Figure 9-1002k EML Capabilities subfield format */
#define EHT_EML_CAP_SR_SUP_POS          0u      /* EMLSR Support subfield */
#define EHT_EML_CAP_SR_SUP_SZ           1u
#define EHT_EML_CAP_SR_DLY_POS          1u      /* EMLSR Delay subfield */
#define EHT_EML_CAP_SR_DLY_SZ           3u
#define EHT_EML_CAP_SR_TRN_DLY_POS          4u      /* EMLSR Delay subfield */
#define EHT_EML_CAP_SR_TRN_DLY_SZ           3u
#define EHT_EML_CAP_MR_SUP_POS          7u      /* EMLMR Support subfield */
#define EHT_EML_CAP_MR_SUP_SZ           1u
#define EHT_EML_CAP_MR_DLY_POS          8u      /* EMLMR Delay subfield */
#define EHT_EML_CAP_MR_DLY_SZ           3u
#define EHT_EML_TRANS_TIMEOUT_POS	11u	/* EML Transition timeout */
#define EHT_EML_TRANS_TIMEOUT_SZ	4u

/* EMLSR Delay Subfield */
/* IEEE P802.11be D1.5 Figure 9-1002k EML Capabilities - EMLSR Delay Subfield */
#define EHT_EML_CAP_SR_DLY_0_USEC       0u
#define EHT_EML_CAP_SR_DLY_32_USEC      1u
#define EHT_EML_CAP_SR_DLY_64_USEC      2u
#define EHT_EML_CAP_SR_DLY_128_USEC     3u
#define EHT_EML_CAP_SR_DLY_256_USEC     4u

/* MLD Capabilities subfield */
/* IEEE P802.11be D1.5 Figure 9-1002l MLD Capabilities subfield format */
#define EHT_MLD_CAP_MAX_LINKS_POS       0u      /* Maximum Number Of Simultaneous Links subfield */
#define EHT_MLD_CAP_MAX_LINKS_SZ        4u
#define EHT_MLD_CAP_TID_MAP_POS         5u      /* TID-To-Link Mapping Negotiation Supported */
#define EHT_MLD_CAP_TID_MAP_SZ          2
#define EHT_MLD_CAP_STR_FREQ_SEP_POS    7u      /* Frequency Separation For STR subfield */
#define EHT_MLD_CAP_STR_FREQ_SEP_SZ     5u

/* Multi-Link element optional subelements */
/* D1.5 Table 9-401d */
#define EHT_ML_SE_PSTA_PRF_ID		0u	/* Per-STA Profile */
#define EHT_ML_SE_VS_ID			221u	/* Vendor Specific */
#define EHT_ML_SE_FRAG_ID               254u
/* *****************************************
 * **********  Per-STA Profile SE  *********
 * *****************************************
 */

/* D1.5 Figure 9-1002m TBD */
typedef BWL_PRE_PACKED_STRUCT struct eht_psta_prf_se {
	uint8 id;
	uint8 len;
	uint16 sta_ctrl;
	uint8 sta_info_len;
	/* STA Info:
	 * optional Link MAC Address
	 * optional Beacon Interval
	 * optional DTIM Info
	 * optional NSTR indication bitmap
	 */
	/* STA Profile:
	 * optional Fields and Elements
	 */
} BWL_POST_PACKED_STRUCT eht_psta_prf_se_t;

typedef BWL_PRE_PACKED_STRUCT struct eht_psta_prf_defrag_se {
	uint8 id;
	uint8 len;
	uint16 defrag_len;
	uint16 sta_ctrl;
	uint8 sta_info_len;
	/* STA Info:
	 * optional Link MAC Address
	 * optional Beacon Interval
	 * optional DTIM Info
	 * optional NSTR indication bitmap
	 */
	/* STA Profile:
	 * optional Fields and Elements
	 */
} BWL_POST_PACKED_STRUCT eht_psta_prf_defrag_se_t;

/* Per-STA Control field */
/* IEEE P802.11be D1.5 Figure 9-1002n STA Control field format */
#define EHT_PSTA_CTL_LINK_ID_POS        0u      /* Link ID */
#define EHT_PSTA_CTL_LINK_ID_SZ         4u
#define EHT_PSTA_CTL_CPLT_PRF_POS       4u      /* Complete Profile */
#define EHT_PSTA_CTL_CPLT_PRF_SZ        1u
#define EHT_PSTA_CTL_LINK_MAC_PST_POS   5u      /* Link MAC Address Present */
#define EHT_PSTA_CTL_LINK_MAC_PST_SZ    1u
#define EHT_PSTA_CTL_BCN_INT_PST_POS    6u      /* Beacon Interval Present */
#define EHT_PSTA_CTL_BCN_INT_PST_SZ     1u
#define EHT_PSTA_CTL_TSF_OFF_PST_POS    7u      /* TSF offset Present */
#define EHT_PSTA_CTL_TSF_OFF_PST_SZ     1u
#define EHT_PSTA_CTL_DTIM_INFO_PST_POS  8u      /* DTIM Info Present */
#define EHT_PSTA_CTL_DTIM_INFO_PST_SZ   1u
#define EHT_PSTA_CTL_NSTR_LINK_PST_POS  9u      /* NSTR Link Pair Present */
#define EHT_PSTA_CTL_NSTR_LINK_PST_SZ   1u
#define EHT_PSTA_CTL_NSTR_BMP_SZ_POS    10u     /* NSTR Bitmap Size */
#define EHT_PSTA_CTL_NSTR_BMP_SZ_SZ     1u
#define EHT_PSTA_CTL_BSS_PRM_CC_PST_POS 11u     /* BSS Parameters Change Count Present */
#define EHT_PSTA_CTL_BSS_PRM_CC_PST_SZ  1u

#define EHT_PSTA_CTL_LINK_ID_MASK	0x000F
#define EHT_PSTA_CTL_CPLT_PRF_MASK	0x0010
#define EHT_PSTA_CTL_MAC_PST_MASK	0x0020
#define EHT_PSTA_CTL_BCN_INT_PST_MASK	0x0040
#define EHT_PSTA_CTL_TSF_OFF_PST_MASK	0x0080
#define EHT_PSTA_CTL_DTIM_INFO_PST_MASK 0x0100
#define EHT_PSTA_CTL_NSTR_LINK_PAIR_MASK 0x0200
#define EHT_PSTA_CTL_NSTR_BITMASK_SIZE_MASK 0x0400
#define EHT_PSTA_CTL_BSS_PRM_CC_PST_MASK 0x0800

/* STA Info subfields length */
#define EHT_PSTA_CTL_LEN		2u	/* STA Control field */
#define EHT_PSTA_MACADDR_LEN		6u	/* STA MAC Address */
#define EHT_PSTA_BCN_INT_LEN            2u      /* Beacon Interval subfield */
#define EHT_PSTA_TSF_OFF_LEN            8u      /* TSF offset subfield */
#define EHT_PSTA_DTIM_INFO_LEN          2u      /* DTIM Info subfield */
#define EHT_PSTA_NSTR_IND_BMP_SZ0_LEN   1u      /* NSTR Indication Bitmap subfield */
#define EHT_PSTA_NSTR_IND_BMP_SZ1_LEN   2u
#define EHT_PSTA_BSS_PRM_CC_LEN         1u      /* BSS Parameters Change Count subfield */

#define EHT_RECONFIG_PSTA_DEL_TIMER_POS 6u
#define EHT_RECONFIG_PSTA_DEL_TIMER_SZ 1u

/* *********************************
 * **********  EHT CAP IE  *********
 * *********************************
 */

/* EHT MAC Capabilities Information field - D1.4 figure 9-1002x */
#define EHT_MAC_CAP_INFO_SIZE	2u
typedef uint8 eht_mac_cap_t[EHT_MAC_CAP_INFO_SIZE];

/* bitfields within the EHT MAC capabilities information field, D2.0 figure 9-1002af */
#define EHT_MAC_CAP_EPCS_PRIORITY_ACCESS_SUPP_IDX	0
#define EHT_MAC_CAP_OM_CONTROL_SUPPORTED_IDX		1
#define EHT_MAC_CAP_TRIG_TXOP_SHARE_MODE1_SUPP_IDX	2
#define EHT_MAC_CAP_TRIG_TXOP_SHARE_MODE2_SUPP_IDX	3
#define EHT_MAC_CAP_RESTRICTED_TWT_SUPP_IDX		4
#define EHT_MAC_CAP_SCS_TRAFFIC_DESC_SUPP_IDX		5
#define EHT_MAC_CAP_MAX_MPDU_LEN_POS			6
#define EHT_MAC_CAP_MAX_MPDU_LEN_SZ			2
#define EHT_MAC_CAP_MAX_AMPDU_LEN_EXP_EXT_IDX		8
#define EHT_MAC_CAP_TRS_SUPP_IDX			9
#define EHT_MAC_CAP_TXOP_RETURN_SUPP_SHARE_MODE2_IDX	10
#define EHT_MAC_CAP_TWO_BQRS_SUPPORT_IDX		11
#define EHT_MAC_CAP_EHT_LINK_ADAPTATION_SUPPORT_POS	12
#define EHT_MAC_CAP_EHT_LINK_ADAPTATION_SUPPORT_SZ	2

enum eht_mac_cap_max_mpdu_len_e {
	EHT_MAC_CAP_MAX_MPDU_LEN_3895 = 0,
	EHT_MAC_CAP_MAX_MPDU_LEN_7991 = 1,
	EHT_MAC_CAP_MAX_MPDU_LEN_11454 = 2,
	EHT_MAC_CAP_MAX_MPDU_LEN_RSVD = 3
};

enum eht_mac_cap_eht_link_adaptation_e {
	EHT_MAC_CAP_LINK_ADAPT_NO_FEEDBACK = 0,
	EHT_MAC_CAP_LINK_ADAPT_RESERVED = 1,
	EHT_MAC_CAP_LINK_ADAPT_UNSOL = 2,
	EHT_MAC_CAP_LINK_ADAPT_SOL_AND_UNSOL = 3
};

/* EHT PHY Capabilities Information field (Draft D1.4 Figure 9-1002y
 * EHT PHY Capabilities Information field format)
 */
#define EHT_PHY_CAP_INFO_SIZE	9u
typedef uint8 eht_phy_cap_t[EHT_PHY_CAP_INFO_SIZE];

/* bitfields within the EHT PHY capabilities information field, D1.4 Figure 9-1002y */

/* first bit reserved */
#define EHT_PHY_320MHZ_IN_6GHZ_IDX		1
/* Indicates support for reception of a 242-tone RU in a PPDU with a bandwidth larger than
 * 20 MHz, by a 20 MHz-only non-AP STA.
 */
#define EHT_PHY_RU242_IN_BW_GT_20MHZ_IDX	2
/* For a beamformee, indicates support for receiving an EHT sounding NDP with 4x EHT-LTF and
 * 3.2us guard interval duration.
 */
#define EHT_PHY_NDP_4xLTF_IDX			3 /**< NDP with 4x EHT-LTF and 3.2us GI */
#define EHT_PHY_PART_BW_ULMUMIMO_IDX		4
#define EHT_PHY_SU_BEAMFORMER_IDX		5
#define EHT_PHY_SU_BEAMFORMEE_IDX		6
/* for BW<=80MHz, indicates the maximum number of spatial streams that the STA can receive in an
 * EHT sounding NDP
 */
#define EHT_PHY_BEAMFORMEE_SS_LE_80MHZ_POS	 7
#define EHT_PHY_BEAMFORMEE_SS_LE_80MHZ_SZ	 3 /**< 3 bits wide */
#define EHT_PHY_BEAMFORMEE_SS_160MHZ_POS	10
#define EHT_PHY_BEAMFORMEE_SS_160MHZ_SZ		 3
#define EHT_PHY_BEAMFORMEE_SS_320MHZ_POS	13
#define EHT_PHY_BEAMFORMEE_SS_320MHZ_SZ		 3
#define EHT_PHY_N_SND_DIM_LE_80MHZ_POS		16 /**< number of sounding dimensions (BW<=80MHz) */
#define EHT_PHY_N_SND_DIM_LE_80MHZ_SZ		 3
#define EHT_PHY_N_SND_DIM_160MHZ_POS		19 /**< number of sounding dimensions (BW=160MHz) */
#define EHT_PHY_N_SND_DIM_160MHZ_SZ		 3
#define EHT_PHY_N_SND_DIM_320MHZ_POS		22 /**< number of sounding dimensions (BW=320MHz) */
#define EHT_PHY_N_SND_DIM_320MHZ_SZ		 3
/* Indicates EHT beamformee support for a subcarrier grouping of 16 in the EHT Compressed
 * Beamforming Report field for SU feedback
 */
#define EHT_PHY_NG_EQ_16SU_FEEDBACK_IDX		25
#define EHT_PHY_NG_EQ_16MU_FEEDBACK_IDX		26
#define EHT_PHY_CBOOK_SIZE_SU_FBACK_IDX		27 /**< {4,2} SU feedback */
#define EHT_PHY_CBOOK_SIZE_MU_FBACK_IDX		28 /**< {7,5} MU feedback */
/* For an AP, indicates support for the reception of partial bandwidth MU feedback in an EHT TB
 * sounding sequence.
 */
#define EHT_PHY_TRIG_SU_BFORMING_FBACK_IDX	29 /**< triggered SU beamforming feedback */
#define EHT_PHY_TRIG_MU_BF_PART_BW_FBACK_IDX	30 /**< trig'ed MU beamformng partial BW feedback */
#define EHT_PHY_TRIG_CQI_FEEDBACK_IDX		31 /**< triggered CQI feedback */
#define EHT_PHY_PART_BW_DL_MUMIMO_IDX		32 /**< partial bandwidth DL MU-MIMO */
#define EHT_PHY_PSR_BASED_SR_IDX		33 /**< PSR-based SR support */
#define EHT_PHY_POW_BOOST_FACTOR_IDX		34 /**< power boost factor support */
#define EHT_PHY_MU_PPDU_4xLTF			35 /**< Rx EHT MU PPDU with 4x EHT-LTF + 0.8us GI */
#define EHT_PHY_MAX_NC_POS			36 /**< max Nc for EHT comprsd beamfrm/CQI report */
#define EHT_PHY_MAX_NC_SZ			 4
#define EHT_PHY_NONTRIG_CQI_FEEDBACK_IDX	40 /**< non-triggered CQI feedback */
#define EHT_PHY_TX_QAM1024_4096_LT_RU242_IDX	41 /**< TX 1024 + 4096-QAM < 242-tone RU support */
#define EHT_PHY_RX_QAM1024_4096_LT_RU242_IDX	42 /**< RX 1024 + 4096-QAM < 242-tone RU support */
#define EHT_PHY_PPE_THRESH_PRESENT_IDX		43 /**< PPE Thresholds field present */
#define EHT_PHY_CMN_NOM_PKT_PADDING_POS		44 /**< nominal pkt padding for all constellatns */
#define EHT_PHY_CMN_NOM_PKT_PADDING_SZ		 2
#define EHT_PHY_MAX_NUM_SUP_EHT_LTFS_IDX	46 /**< maximum number of supported EHT-LTFs */
#define EHT_PHY_MAX_NUM_SUP_EHT_LTFS_SU_POS	47 /**< rx in non-OFDMA transm to a single user */
#define EHT_PHY_MAX_NUM_SUP_EHT_LTFS_SU_SZ	 2
#define EHT_PHY_MAX_NUM_SUP_EHT_LTFS_MU_POS	49 /**< rx in non-OFDMA transm to multiple users */
#define EHT_PHY_MAX_NUM_SUP_EHT_LTFS_MU_SZ	 2
#define EHT_PHY_MCS15MRU_52TONE_IDX		51 /**< DCM in 52+26-tone and 106+26-tone MRUs */
#define EHT_PHY_MCS15MRU_484TONE_IDX		52 /**< DCM in 484+242-tone MRU if 80MHz support */
#define EHT_PHY_MCS15MRU_996TONE_IDX		53 /**< DCM in 996+484(+242)-tone MRU (160MHz) */
#define EHT_PHY_MCS15MRU_3x996TONE_IDX		54 /**< DCM in 3x996 tone MRU (320MHz) */
#define EHT_PHY_EHT_DUP_6GHZ_IDX		55 /**< support of EHT DUP (MCS14) in 6GHz */
#define EHT_PHY_20MHZSTA_RX_NDP_GT20_IDX	56 /**< 20MHz oper. STA rx'ing NDP with wider bw */
#define EHT_PHY_NONOFDMA_ULMUMIMO_BW_LE_80MHZ_IDX 57 /**< Non-OFDMA UL MU-MIMO (BW<=80MHz) */
#define EHT_PHY_NONOFDMA_ULMUMIMO_BW_160MHZ_IDX 58 /**< Non-OFDMA UL MU-MIMO (BW=160MHz) */
#define EHT_PHY_NONOFDMA_ULMUMIMO_BW_320MHZ_IDX 59 /**< Non-OFDMA UL MU-MIMO (BW=320MHz) */
#define EHT_PHY_MU_BEAMFORMER_BW_LE_80MHZ_IDX	60 /**< MU beamformer (BW<=80MHz) */
#define EHT_PHY_MU_BEAMFORMER_BW_160MHZ_IDX	61 /**< MU beamformer (BW=160MHz) */
#define EHT_PHY_MU_BEAMFORMER_BW_320MHZ_IDX	62 /**< MU beamformer (BW=320MHz) */
#define EHT_PHY_TB_SND_FBACK_RATE_LIM_IDX	63 /**< Trig based sounding feedback rate limit */
#define EHT_PHY_RX_QAM1024_WIDE_BW_DL_OFDMA_IDX	64 /**< Rx 1024-QAM In Wider BW DL OFDMA Support */
#define EHT_PHY_RX_QAM4096_WIDE_BW_DL_OFDMA_IDX	65 /**< Rx 4096-QAM In Wider BW DL OFDMA Support */
#define EHT_PHY_20MHZONLY_LIMITED_CAP_SUP_IDX	66 /**< 20 MHz-Only Limited Capabilities Support */
#define EHT_PHY_20MHZONLY_TRIGGERED_MUBFFULLBW_IDX  67  /**<  20 MHz-Only Triggered MU Beamforming
							 *    Full BW Feedback And DL MU-MIMO
							 */
#define EHT_PHY_20MHZONLY_MRU_SUPPORT_IDX	68 /**< 20 MHz-Only M-RU Support */

/* EHT PPET command values */
enum eht_phy_cmn_nom_pkt_padding_e {
	EHT_PHY_CMN_NOM_PKT_PADDING_0US = 0,
	EHT_PHY_CMN_NOM_PKT_PADDING_8US = 1,
	EHT_PHY_CMN_NOM_PKT_PADDING_16US = 2,
	EHT_PHY_CMN_NOM_PKT_PADDING_20US = 3
};

/* EHT Capabilities element - D1.3 */
BWL_PRE_PACKED_STRUCT struct eht_cap_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	eht_mac_cap_t mac_cap;		/* MAC Capabilities Information */
	eht_phy_cap_t phy_cap;		/* PHY Capabilities Information */
	/* followed by variable field: eht_mcs_map_t */
	/* followed by variable+optional field: EHT PPE Thresholds field */
} BWL_POST_PACKED_STRUCT;
typedef struct eht_cap_ie eht_cap_ie_t;

/**
 * \def EHT_MCS_MAP_NSS_MAX
 * Max number of streams possible, might become 16 in wave-2.
 * D1.0 Table 9-322at: eht_mcs_nss_set has max 8 nss encoded.
 */
#define EHT_MCS_MAP_NSS_MAX	8u

/*
 * bit offsets of the nibbles within D1.0 Figure 9-788ex EHT-MCS Map (20 MHz-Only STA) subfield.
 * Each nibble contains a 'Max NSS' value. Note that this differs from the HE MCSMAP format.
 */
#define EHT_MCS_MAP20_MCS_CODE_0_7_RX_NSS_IDX	0u	/**< 1st nibble */
#define EHT_MCS_MAP20_MCS_CODE_0_7_TX_NSS_IDX	4u	/**< 2nd nibble */
#define EHT_MCS_MAP20_MCS_CODE_8_9_RX_NSS_IDX	8u
#define EHT_MCS_MAP20_MCS_CODE_8_9_TX_NSS_IDX	12u
#define EHT_MCS_MAP20_MCS_CODE_10_11_RX_NSS_IDX	16u
#define EHT_MCS_MAP20_MCS_CODE_10_11_TX_NSS_IDX	20u
#define EHT_MCS_MAP20_MCS_CODE_12_13_RX_NSS_IDX	24u
#define EHT_MCS_MAP20_MCS_CODE_12_13_TX_NSS_IDX	28u
#define EHT_MCS_MAP_STA20_NBYTES 4	/**< size in [bytes], for BW=20 only capable STA */

/*
 * bit offsets of the nibbles within D1.0 Figure 9-788ey EHT-MCS Map. Each nibble contains a
 * 'Max NSS' value. Note that this differs from the HE MCSMAP format.
 */
#define EHT_MCS_MAP_MCS_CODE_0_9_RX_NSS_IDX	0u	/**< 1st nibble */
#define EHT_MCS_MAP_MCS_CODE_0_9_TX_NSS_IDX	4u	/**< 2nd nibble */
#define EHT_MCS_MAP_MCS_CODE_10_11_RX_NSS_IDX	8u
#define EHT_MCS_MAP_MCS_CODE_10_11_TX_NSS_IDX	12u
#define EHT_MCS_MAP_MCS_CODE_12_13_RX_NSS_IDX	16u
#define EHT_MCS_MAP_MCS_CODE_12_13_TX_NSS_IDX	20u
#define EHT_MCS_MAP_NBYTES 3		/**< size of mcs map in [bytes] */

#define EHT_MCS_MAP_NSS_SZ 4		/** size of each Nss field within an mcs map in [bits] */
#define EHT_MCS_MAP_NSS_MASK 0xF	/** mask for those 4 Nss bits */

/*
 * Supported EHT-MCS And NSS Set field, consists of one mcs map per bandwidth category.
 */

/* One unit-map length (TX or RX) */
#define EHT_MCS_NSS_SUPP_FLD_UNIT_MAP_LEN	(EHT_MCS_NSS_MAP_SIZE)
#define EHT_MCS_NSS_SUPP_FLD_UNIT_MAP_SZ \
	(EHT_MCS_NSS_SUPP_FLD_UNIT_MAP_LEN * 8u)	/* bits */

/* Two unit-map length (TX and RX) */
#define EHT_MCS_NSS_SUPP_FLD_TXRX_MAP_LEN	(EHT_MCS_NSS_SUPP_FLD_UNIT_MAP_LEN * 2u)
#define EHT_MCS_NSS_SUPP_FLD_TXRX_MAP_SZ \
	(EHT_MCS_NSS_SUPP_FLD_TXRX_MAP_LEN * 8u)	/* bits */

/* *********************************************
 * **********  TID-To-Link Mapping IE  *********
 * *********************************************
 */

/* TID-To-Link Mapping element */
/* IEEE P802.11be D1.0 Figure 9-788eab TID-To-Link Mapping element format */
typedef BWL_PRE_PACKED_STRUCT struct eht_tid_map_ie {
	uint8   id;
	uint8   len;
	uint8   id_ext;
	uint16   tid_map_ctrl;           /* TID-To-Link Control fixed fields */
	/* Link Mapping[] */
} BWL_POST_PACKED_STRUCT eht_tid_map_ie_t;
/* TID-To-Link Mapping Negotiation Supported subfield */
/* IEEE P802.11be D2.1 Table 9-401j Subfields of the MLD Capabilities field */
#define EHT_TID_MAP_SAME_LINK_SET	1u	/* the MLD supports mapping each TID
						 * to the same link set
						 */
#define EHT_TID_MAP_ANY_LINK_SET	3u	/* the MLD supports mapping each TID
						* to the same or different link set
						*/
/* TID-To-Link  field */
/* IEEE P802.11be D3.0 Figure 9-1002am TID-To-Link Control field format */
#define EHT_TID_MAP_CTL_DIR_POS			0u      /* Direction */
#define EHT_TID_MAP_CTL_DIR_SZ			2u
#define EHT_TID_MAP_CTL_DEF_MAP_POS		2u      /* Default Link Mapping */
#define EHT_TID_MAP_CTL_DEF_MAP_SZ		1u
#define EHT_TID_MAP_CTL_SW_TIME_PST_POS		3u	/* Mapping Switch Time Present */
#define EHT_TID_MAP_CTL_SW_TIME_PST_SZ		1u
#define EHT_TID_MAP_CTL_EXP_DUR_PST_POS		4u	/* Expected Duration Present */
#define EHT_TID_MAP_CTL_EXP_DUR_PST_SZ		1u
#define EHT_TID_MAP_CTL_T2L_MAP_SZ_POS		5u	/* Link Mapping size  indicator */
#define EHT_TID_MAP_CTL_T2L_MAP_SZ_SZ		1u	/* Link Mapping size indicator */
#define EHT_TID_MAP_CTL_MAP_PST_POS		8u      /* Link Mapping Presence Indicator */
#define EHT_TID_MAP_CTL_MAP_PST_SZ		8u

/* The Direction subfield */
#define EHT_TID_MAP_DIR_UPLINK          0u      /* Uplink */
#define EHT_TID_MAP_DIR_DNLINK          1u      /* Downlink */
#define EHT_TID_MAP_DIR_BOTH            2u      /* Uplink & Downlink */

/* Mapping Switch Time field */
#define EHT_TID_MAP_SW_TIME_POS		0u
#define EHT_TID_MAP_SW_TIME_SZ		16u	/* 16 bits */
#define EHT_TID_MAP_SW_TIME_LEN		2u	/* 2 octets */

/* Expected Duration field */
#define EHT_TID_MAP_EXP_DUR_POS		0u
#define EHT_TID_MAP_EXP_DUR_SZ		24u	/* 24 bits */
#define EHT_TID_MAP_EXP_DUR_LEN		3u	/* 3 octets */

/* Link Mapping of TID t field */
/* IEEE P802.11be D1.0 Figure 9-788eab TID-To-Link Mapping element format */
#define EHT_TID_LINK_MAP_POS            0u
#define EHT_TID_LINK_MAP_SZ             16u     /* 16 bits */

#define EHT_TID_LINK_MAP_LEN            2u      /* 2 octets */

/* bit within the Link Mapping bitmap */
/* IEEE P802.11be D1.0 p153l16 */
#define EHT_TID_LINK_MAP_MASK(link_id)  (1u << (link_id))

/* TID-to-Link Mapping Request */
/* Draft D2.0 Table 9-526q TID-To-Link Mapping Request frame Action field format */
typedef BWL_PRE_PACKED_STRUCT struct eht_paf_eml_oper_mode {
	uint8	category;
	uint8	action;
	uint8	dialog;
	uint8   data[0]; /* 9.4.1.74 EML Control field */
} BWL_POST_PACKED_STRUCT eht_paf_eml_oper_mode_t;

/* emlsr ctl fixed fields
* Bit:0 - EMLSR mode
* Bit:1 - EMLMR mode
* Bit:2 - EMLSR parameter update control
* Bit:3-7 - Reserved
*/
#define EHT_EML_CTRL_FIXED_FLD_LEN      1u

/* EML Operating Mode Notification */
#define EHT_EML_CTRL_EMLSR_POS			0u      /* EMLSR Mode */
#define EHT_EML_CTRL_EMLSR_SZ			1u
#define EHT_EML_CTRL_EMLMR_POS			1u      /* EMLMR Mode */
#define EHT_EML_CTRL_EMLMR_SZ			1u
#define EHT_EML_CTRL_EMLSR_PARAM_UPD_POS	2u
#define EHT_EML_CTRL_EMLSR_PARAM_UPD_SZ		1u
#define EHT_EML_CTRL_EMLSR_BITMAP_POS		8u
#define EHT_EML_CTRL_EMLSR_BITMAP_SZ		16u      /* EMLSR Link Bitmap */
#define EHT_EML_CTRL_EMLMR_BITMAP_POS		8u
#define EHT_EML_CTRL_EMLMR_BITMAP_SZ		16u      /* EMLMR Link Bitmap */
#define EHT_EML_CTRL_EMLMR_MCS_MAP_CNT_POS	40u
#define EHT_EML_CTRL_EMLMR_MCS_MAP_CNT_SZ	8u       /* EMLMR MAP COUNT */
#define EHT_EML_CTRL_EMLMR_MCS_NSS_80_POS	42u
#define EHT_EML_CTRL_EMLMR_MCS_NSS_80_SZ	24u
#define EHT_EML_CTRL_EMLMR_MCS_NSS_160_POS	66u
#define EHT_EML_CTRL_EMLMR_MCS_NSS_160_SZ	24u
#define EHT_EML_CTRL_EMLMR_MCS_NSS_320_POS	90u
#define EHT_EML_CTRL_EMLMR_MCS_NSS_320_SZ	24u

/* *********************************
 * ************ ML TRAFFIC IE ******
 * *********************************
 */
/* Multi-Link element (fixed portion) */
/* D1.0 Figure 9-788ef */
typedef BWL_PRE_PACKED_STRUCT struct eht_ml_traffic_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	uint16 ml_ctrl;
	uint8 data[0];
} BWL_POST_PACKED_STRUCT eht_ml_traffic_ie_t;

/* Multi-Link traffic Control field element */
#define EHT_ML_TRF_ML_CTL_AID_OFFSET      4

/******************************************
* *****  Protected EHT Action Frames ******
* *****************************************
*/
/* TID-to-Link Mapping Request */
/* Draft D1.0 Table 9-526q TID-To-Link Mapping Request frame Action field format */
typedef BWL_PRE_PACKED_STRUCT struct eht_paf_tid_map_req {
	uint8	category;
	uint8	action;
	uint8	dialog;
	eht_tid_map_ie_t tid_map[];		/* TID-to-Link Mapping */
} BWL_POST_PACKED_STRUCT eht_paf_tid_map_req_t;

/* TID-to-Link Mapping Response */
/* Draft D1.0 Table 9-526r TID-To-Link Mapping Response frame Action field format */
typedef BWL_PRE_PACKED_STRUCT struct eht_paf_tid_map_resp {
	uint8	category;
	uint8	action;
	uint8	dialog;
	uint16	status;
	eht_tid_map_ie_t tid_map[];		/* TID-to-Link Mapping */
} BWL_POST_PACKED_STRUCT eht_paf_tid_map_resp_t;

/* TID-to-Link Mapping Teardown */
/* Draft D1.0 Table 9-526s TID-To-Link Mapping Teardown frame Action field format */
typedef BWL_PRE_PACKED_STRUCT struct eht_paf_tid_map_teardn {
	uint8	category;
	uint8	action;
} BWL_POST_PACKED_STRUCT eht_paf_tid_map_teardn_t;

/* IEEE P802.11be D3.0 Figure 9-1002ay AID Bitmap element format */
typedef BWL_PRE_PACKED_STRUCT struct eht_aid_bitmap_ie {
	uint8	id;
	uint8	len;
	uint8	id_ext;
	uint8	aid_bitmap_len;	/* Partial AID Bitmap Length */
	uint8	bitmap_ctrl;	/* Bitmap Control */
	uint8	aid_bitmap[];	/* Partial AID Bitmap */
} BWL_POST_PACKED_STRUCT eht_aid_bitmap_ie_t;

#define EHT_AID_BMP_CTL_BMP_OFFSET_POS	1u
#define EHT_AID_BMP_CTL_BMP_OFFSET_SZ	7u

#define EHT_AID_BMP_FIXED_LENGTH	2u
#define EHT_AID_BMP_OCTETS_NUM_MAX	251u

/* Link Recommendation */
/* Draft D3.0 Table 9-623k Link Recommendation frame Action field format */
typedef BWL_PRE_PACKED_STRUCT struct eht_paf_link_rec {
	uint8	category;
	uint8	action;
	uint16	reason;
	uint8	fields[]; /* subfields as listed below */
	/* AID Bitmap element */
	/* Multi-link Traffic Indication element */
} BWL_POST_PACKED_STRUCT eht_paf_link_rec_t;

#define EHT_PAF_ACT_LINK_RECOMMEND	7u	/* Link Recommendation */
/* Table 9-526p Protected EHT Action field values */
#define EHT_PAF_ACT_TID_MAP_REQ		0u	/* TID-To-Link Mapping Request */
#define EHT_PAF_ACT_TID_MAP_RESP	1u	/* TID-To-Link Mapping Response */
#define EHT_PAF_ACT_TID_MAP_TEARDN	2u	/* TID-To-Link Mapping Teardown */
#define EHT_PAF_ACT_TID_MAP_NSEP_REQ	3u	/* Priority Access Enable Request */
#define EHT_PAF_ACT_TID_MAP_NSEP_RESP	4u	/* Priority Access Enable Response */
#define EHT_PAF_ACT_TID_MAP_NSEP_TEARDN	5u	/* Priority Access Teardown */
#define EHT_PAF_ACT_EML_OPER_MODE_NOTIF	6u	/* Priority Access Teardown */

/* EHT Timing related parameters(11be Draft 0.4 Table 36-17-Timing-related constants) */
#define EHT_T_LEG_STF			8u  /* Non-HT STF duration */
#define EHT_T_LEG_LTF			8u  /* Non-HT LTF duration */
#define EHT_T_LEG_SIG			4u  /* Non-HT Signal duration */
#define EHT_T_RL_SIG			4u  /* Repeated Non-HT Signal duration */
#define EHT_T_USIG			8u  /* EHT U-SIG field duration */
#define EHT_T_EHT_SIG			4u  /* OFDM symbol dur in EHT-SIG field */
#define EHT_T_STF			4u  /* STF for SU / MU EHT PPDUs */
#define EHT_T_EHT_MU_PPDU_STF		4u  /* STF for SU / MU EHT PPDUs */
#define EHT_T_EHT_TB_PPDU_STF		8u  /* STF for TB EHT PPDUs */
#define EHT_T_LEG_PREAMBLE		(EHT_T_LEG_STF + EHT_T_LEG_LTF + EHT_T_LEG_SIG)

#define EHT_RU_26_TONE			26
#define EHT_RU_52_TONE			52
#define EHT_RU_106_TONE			106
#define EHT_RU_242_TONE			242
#define EHT_RU_484_TONE			484
#define EHT_RU_996_TONE			996
#define EHT_RU_2x996_TONE		(2*996)
#define EHT_RU_4x996_TONE		(4*996)
/* MRU tones */
#define EHT_MRU_52_26_TONE		(EHT_RU_52_TONE + EHT_RU_26_TONE)
#define EHT_MRU_106_26_TONE		(EHT_RU_106_TONE + EHT_RU_26_TONE)
#define EHT_MRU_484_242_TONE		(EHT_RU_484_TONE + EHT_RU_242_TONE)
#define EHT_MRU_996_484_TONE		(EHT_RU_996_TONE + EHT_RU_484_TONE)
#define EHT_MRU_996_484_242_TONE	(EHT_RU_996_TONE + EHT_RU_484_TONE + EHT_RU_242_TONE)
#define EHT_MRU_2x996_484_TONE		(2*EHT_RU_996_TONE + EHT_RU_484_TONE)
#define EHT_MRU_3x996_TONE		(3*EHT_RU_996_TONE)
#define EHT_MRU_3x996_484_TONE		(3*EHT_RU_996_TONE + EHT_RU_484_TONE)

/* In multiples of 10 */
#define EHT_T_LTF_1X			32u    /* 1x EHT-LTF OFDM symbol without GI */
#define EHT_T_LTF_2X			64u    /* 2x EHT-LTF OFDM symbol without GI */
#define EHT_T_LTF_4X			128u   /* 3x EHT-LTF OFDM symbol without GI */
#define EHT_T_SYM1			136u   /* OFDM symbol duration with base GI */
#define EHT_T_SYM2			144u   /* OFDM symbol duration with double GI */
#define EHT_T_SYM4			160u   /* OFDM symbol duration with quad GI */

#define EHT_N_LEG_SYM			3u     /* bytes per legacy symbol */
#define EHT_N_TAIL			6u     /* tail field bits for BCC */
#define EHT_N_SERVICE			16u    /* bits in service field */

#define EHT_CAP_MCS_MAP_NSS_MAX		8	/* Max number of streams possible */

/**
 * A phy has some latency on decoding rx data before it can supply the necessary info for feedback.
 * The PPE Thresholds field defines this information, so the transmitter can add padding bits which
 * will give the receiver time to formulate a response.
 */

/* Optional PPE Threshold field in phy_cap (D1.4 Figure 9-1002ac) */
#define EHT_PPE_THRESH_NSS_IDX		0	/**< #NSS value */
#define EHT_PPE_THRESH_NSS_FSZ		4
#define EHT_PPE_THRESH_NSS_MASK	0xf
#define EHT_PPE_RU_INDEX_MASK_IDX	4	/**< RU Index mask field */
#define EHT_PPE_RU_INDEX_MASK_FSZ	5
#define EHT_PPE_RU_INDEX_BMASK_MASK	0x1f

#define EHT_PPE_THRESH_NSS_RU_FSZ	3	/**< both PPET8 as PPETmax fields are 3 bits long */
#define EHT_PPE_THRESH_NSS_RU_MASK	0x7

/**
 * Total PPE Threshold field byte length (D1.4 Figure 9-1002ad)
 * @param[in] nss  Max supported number of spatial streams, must be > 0
 * @param[in] ruc  RU count, max supported number of resource units
 * @return         Length of field in [bytes]
 *
 * The PPE Thresholds Info field contains 6 x (NSS+1) bits, where NSS is the value in the NSS
 * field, for every bit in the RU Index Bitmask subfield that is nonzero.
 */
#define EHT_PPE_THRESH_LEN(nss, ruc) \
	(CEIL((EHT_PPE_THRESH_NSS_FSZ + EHT_PPE_RU_INDEX_MASK_FSZ + \
		((nss) * (ruc) * 6)), 8))

/**
 * (PHY) packet extension threshold for Xus (D1.4 Figure 9-1002ad)
 * @param[in] ruc     RU count, max supported number of resource units
 * @param[in] NSSnM1  Number of spatial streams minus 1
 * @param[in] RUmM1   Resource unit index minus 1
 */
#define EHT_PPETMAX_BIT_OFFSET(ruc, NSSnM1, RUmM1) \
	(EHT_PPE_THRESH_NSS_FSZ + EHT_PPE_RU_INDEX_MASK_FSZ + \
	 ((NSSnM1) * (ruc) + (RUmM1)) * 2 * EHT_PPE_THRESH_NSS_RU_FSZ)

/**
 * (PHY) packet extension threshold for 8us (D1.4 Figure 9-1002ad)
 * @param[in] ruc     RU count, max supported number of resource units
 * @param[in] NSSnM1  Number of spatial streams minus 1
 * @param[in] RUmM1   Resource unit index minus 1
 */
#define EHT_PPET8_BIT_OFFSET(ruc, NSSnM1, RUmM1) \
	(EHT_PPETMAX_BIT_OFFSET((ruc), (NSSnM1), (RUmM1)) + EHT_PPE_THRESH_NSS_RU_FSZ)

/** RU Allocation Index encoding (D1.4 Table 9-401o) */
enum eht_ru_alloc_idx_e {
	EHT_RU_ALLOC_IDX_242 = 0,	/* RU alloc: 282 tones */
	EHT_RU_ALLOC_IDX_484 = 1,	/* RU alloc: 484 tones - 40Mhz */
	EHT_RU_ALLOC_IDX_996 = 2,	/* RU alloc: (484+242, 996) - <= 80Mhz */
	EHT_RU_ALLOC_IDX_2x996 = 3,	/* RU alloc: (996+484, 996+484+242, 2x996) */
	EHT_RU_ALLOC_IDX_GT2x996 = 4,	/* RU alloc: (2x996+484, 3x996, 3x996+484, 4x996) */
	EHT_MAX_RU_COUNT = 5		/* 0 through 4, so 5 in total */
};

/**
 * From the column 'B7-B1 of the RU Allocation subfield' in the table 'Encoding of PS160 and
 * RU Allocation subfields in an EHT  variant User Info field' in the .11be standard.
 */
#define EHT_4x996_TONE_RU_INDX		69  /**< 320MHz */
#define EHT_MAX_4x996_TONE_RU_INDX	EHT_4x996_TONE_RU_INDX /* obsolete name */
#define EHT_MIN_52_26_TONE_RU_INDX	70  /**< MRU */
#define EHT_52_26_TONE_RU_INDX_71	71  /**< MRU */
#define EHT_52_26_TONE_RU_INDX_74	74  /**< MRU */
#define EHT_52_26_TONE_RU_INDX_77	77  /**< MRU */
#define EHT_52_26_TONE_RU_INDX_78	78  /**< MRU */
#define EHT_52_26_TONE_RU_INDX_80	80  /**< MRU */
#define EHT_MAX_52_26_TONE_RU_INDX	81  /**< MRU */
#define EHT_MIN_106_26_TONE_RU_INDX     82  /**< MRU */
#define EHT_MAX_106_26_TONE_RU_INDX	89  /**< MRU */
#define EHT_MIN_484_242_TONE_RU_INDX	90  /**< MRU */
#define EHT_484_242_TONE_RU_INDX_91	91  /**< MRU */
#define EHT_484_242_TONE_RU_INDX_92	92  /**< MRU */
#define EHT_MAX_484_242_TONE_RU_INDX	93  /**< MRU */
#define EHT_MIN_996_484_RU_INDX		94  /**< MRU */
#define EHT_MAX_996_484_RU_INDX		95  /**< MRU */
#define EHT_MIN_996_484_242_RU_INDX	96  /**< MRU */
#define EHT_MAX_996_484_242_RU_INDX	99  /**< MRU */
#define EHT_MIN_2x996_484_RU_INDX	100 /**< MRU */
#define EHT_2x996_484_RU_INDX_101	101 /**< MRU */
#define EHT_2x996_484_RU_INDX_102	102 /**< MRU */
#define EHT_MAX_2x996_484_RU_INDX	103 /**< MRU */
#define EHT_3x996_RU_INDX		104 /**< MRU */
#define EHT_MIN_3x996_484_RU_INDX	105 /**< MRU */
#define EHT_MAX_3x996_484_RU_INDX	106 /**< MRU */

/** Constellation Index encoding (D1.4 Table 9-401n) */
enum eht_const_idx_e {
	EHT_CONST_IDX_BPSK = 0,
	EHT_CONST_IDX_QPSK = 1,
	EHT_CONST_IDX_16QAM = 2,
	EHT_CONST_IDX_64QAM = 3,
	EHT_CONST_IDX_256QAM = 4,
	EHT_CONST_IDX_1024QAM = 5,
	EHT_CONST_IDX_4096QAM = 6,
	EHT_CONST_IDX_NONE = 7
};

/** EHT U-SIG definitions, per D1.1 Table 36-28 */
/* using 32-bits for encoding of first part */
#define EHT_PLCP1_USIG_1_VERSION_MASK		0x00000007	/* b0-b2 */
#define EHT_PLCP1_USIG_1_VERSION_SHIFT		0
#define EHT_PLCP1_USIG_1_BW_MASK		0x00000038	/* b3-b5 */
#define EHT_PLCP1_USIG_1_BW_SHIFT		3
#define EHT_PLCP1_USIG_1_UL_DL_ID_MASK		0x00000040	/* b6 */
#define EHT_PLCP1_USIG_1_UL_DL_ID_SHIFT		6
#define EHT_PLCP1_USIG_1_BSSCOLOR_MASK		0x00001F80	/* b7-b12 */
#define EHT_PLCP1_USIG_1_BSSCOLOR_SHIFT		7
#define EHT_PLCP1_USIG_1_TXOP_MASK		0x000FE000	/* b13-b19 */
#define EHT_PLCP1_USIG_1_TXOP_SHIFT		13
#define EHT_PLCP1_USIG_1_RSVD_MASK		0x03F00000	/* b20-b25 (inc validate bit) */
#define EHT_PLCP1_USIG_1_RSVD_SHIFT		20
/* U-SIG-2 starts from bit 26 */
#define EHT_PLCP1_USIG_2_PPDU_TYPE_COMP_MASK	0x0C000000	/* b0-b1 -> b26-b27 */
#define EHT_PLCP1_USIG_2_PPDU_TYPE_COMP_SHIFT	26
#define EHT_PLCP1_USIG_2_RSVD_MASK		0x10000000	/* b2 -> b28 */
#define EHT_PLCP1_USIG_2_RSVD_SHIFT		28
#define EHT_PLCP1_USIG_2_PUNCTMAP_MASK		0xE0000000	/* b3-b7 -> b29-b33 -> b29-b31 */
#define EHT_PLCP1_USIG_2_PUNCTMAP_SHIFT	29
/* using 16-bits for remainder of U_SIG-2 (b6=b32->b0) */
#define EHT_PLCP2_USIG_2_PUNCTMAP_MASK		0x0003		/* b6-b7 -> b0-b1 */
#define EHT_PLCP2_USIG_2_PUNCTMAP_SHIFT		0
#define EHT_PLCP2_USIG_2_RSVD_MASK		0x0004		/* b8 -> b2 */
#define EHT_PLCP2_USIG_2_RSVD_SHIFT		2
#define EHT_PLCP2_USIG_2_EHTSIGMCS_MASK		0x0018		/* b9-b10 -> b3-b4 */
#define EHT_PLCP2_USIG_2_EHTSIGMCS_SHIFT	3
#define EHT_PLCP2_USIG_2_NUMEHTSIGSYM_MASK	0x03E0		/* b11-b15 -> b5-b9 */
#define EHT_PLCP2_USIG_2_NUMEHTSIGSYM_SHIFT	5
#define EHT_PLCP2_USIG_2_CRC_MASK		0x3C00		/* b16-b19 -> b10-b13 */
#define EHT_PLCP2_USIG_2_CRC_SHIFT		10
#define EHT_PLCP2_USIG_2_TAIL_MASK		0xC000		/* b20-b26 -> b14-b20 -> b14-b15 */
#define EHT_PLCP2_USIG_2_TAIL_SHIFT		14
/* using 16-bits for U_SIG Overflow, common fields from table 36-33 */
#define EHT_PLCP_USIG_OVFL_SPATIALREUSE_MASK	0x000F		/* b0-b3 */
#define EHT_PLCP_USIG_OVFL_SPATIALREUSE_SHIFT	0
#define EHT_PLCP_USIG_OVFL_CPLTF_MASK		0x0030		/* b4-b5 */
#define EHT_PLCP_USIG_OVFL_CPLTF_SHIFT		4
#define EHT_PLCP_USIG_OVFL_NUMLTF_MASK		0x01C0		/* b6-b8 */
#define EHT_PLCP_USIG_OVFL_NUMLTF_SHIFT		6
#define EHT_PLCP_USIG_OVFL_LDPCEXT_MASK		0x0200		/* b9 */
#define EHT_PLCP_USIG_OVFL_LDPCEXT_SHIFT	9
#define EHT_PLCP_USIG_OVFL_AFACTOR_MASK		0x0C00		/* b10-b11 */
#define EHT_PLCP_USIG_OVFL_AFACTOR_SHIFT	10
#define EHT_PLCP_USIG_OVFL_PEDISMB_MASK		0x1000		/* b12 */
#define EHT_PLCP_USIG_OVFL_PEDISMB_SHIFT	12
#define EHT_PLCP_USIG_OVFL_RSVD_MASK		0xE000		/* b13-b16 -> b13-b15 */
#define EHT_PLCP_USIG_OVFL_RSVD_SHIFT		13

enum eht_plcp_bw_e {
	EHT_PLCP1_USIG_1_BW_20MHZ = 0,
	EHT_PLCP1_USIG_1_BW_40MHZ = 1,
	EHT_PLCP1_USIG_1_BW_80MHZ = 2,
	EHT_PLCP1_USIG_1_BW_160MHZ = 3,
	EHT_PLCP1_USIG_1_BW_320_1MHZ = 4,
	EHT_PLCP1_USIG_1_BW_320_2MHZ = 5
};

enum eht_plcp_cpltf_e {
	EHT_PLCP_USIG_OVFL_CPLTF_2x_LTF_GI_0_8us = 0,
	EHT_PLCP_USIG_OVFL_CPLTF_2x_LTF_GI_1_6us = 1,
	EHT_PLCP_USIG_OVFL_CPLTF_4x_LTF_GI_0_8us = 2,
	EHT_PLCP_USIG_OVFL_CPLTF_4x_LTF_GI_3_2us = 3
};

/* EHT OM Control: See Draft P802.11be D1.2, 9.2.4.6a.8; see also 802.11ax.h */
#define HTC_EHT_OM_CONTROL_RX_NSS_EXT_MASK		0x01
#define HTC_EHT_OM_CONTROL_RX_NSS_EXT_OFFSET		0
#define HTC_EHT_OM_CONTROL_CHANNEL_WIDTH_EXT_MASK	0x02
#define HTC_EHT_OM_CONTROL_CHANNEL_WIDTH_EXT_OFFSET	1
#define HTC_EHT_OM_CONTROL_TX_NSTS_EXT_MASK		0x04
#define HTC_EHT_OM_CONTROL_TX_NSTS_EXT_OFFSET		2
#define HTC_EHT_OM_CONTROL_RESERVED_MASK		0x38
#define HTC_EHT_OM_CONTROL_RESERVED_OFFSET		3

/* EHT OM control decode helpers */
#define HTC_EHT_OM_CONTROL_CHANNEL_WIDTH_EXT(omi)	\
	(((omi) & HTC_EHT_OM_CONTROL_CHANNEL_WIDTH_EXT_MASK) \
		>> HTC_EHT_OM_CONTROL_CHANNEL_WIDTH_EXT_OFFSET)
#define HTC_EHT_OM_CONTROL_RX_NSS_EXT(omi)		\
	(((omi) & HTC_EHT_OM_CONTROL_RX_NSS_EXT_MASK) \
		>> HTC_EHT_OM_CONTROL_RX_NSS_EXT_OFFSET)
#define HTC_EHT_OM_CONTROL_TX_NSTS_EXT(omi)		\
	(((omi) & HTC_EHT_OM_CONTROL_TX_NSTS_EXT_MASK) \
		>> HTC_EHT_OM_CONTROL_TX_NSTS_EXT_OFFSET)

/* EHT OM control encode helpers */
#define HTC_EHT_OMI_ENCODE(rx, tx, bw) \
	(((rx) << HTC_EHT_OM_CONTROL_RX_NSS_EXT_OFFSET) | \
	((tx) << HTC_EHT_OM_CONTROL_TX_NSTS_EXT_OFFSET) | \
	((bw) << HTC_EHT_OM_CONTROL_CHANNEL_WIDTH_EXT_OFFSET))
#define HTC_EHT_OMI_BW_HE_ENCODE(bw_idx)	((bw_idx <= DOT11_OPER_MODE_160MHZ) ? bw_idx : 0)
#define HTC_EHT_OMI_BW_EXT_ENCODE(bw_idx)	((bw_idx <= DOT11_OPER_MODE_160MHZ) ? 0 : 1)
#define HTC_EHT_OMI_RXNSS_HE_ENCODE(nss)	((nss < 8)? nss : nss - 8)
#define HTC_EHT_OMI_RXNSS_EXT_ENCODE(nss)	((nss < 8)? 0 : 1)
#define HTC_EHT_OMI_TXNSTS_HE_ENCODE(nsts)	((nsts < 8)? nsts : nsts - 8)
#define HTC_EHT_OMI_TXNSTS_EXT_ENCODE(nsts)	((nsts < 8)? 0 : 1)

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

/* EHT Basic trigger frame common info fields */

/* bit position and field width */
#define EHT_TRIG_CMNINFO_FRMTYPE_INDX		0	/* Trigger frame type */
#define EHT_TRIG_CMNINFO_FRMTYPE_FSZ		4
#define EHT_TRIG_CMNINFO_LSIGLEN_INDX		4	/* L-sig length */
#define EHT_TRIG_CMNINFO_LSIGLEN_FSZ		12
#define EHT_TRIG_CMNINFO_CASCADEIND_INDX	16	/* Cascade indication */
#define EHT_TRIG_CMNINFO_CASCADEIND_FSZ		1
#define EHT_TRIG_CMNINFO_CSREQ_INDX		17	/* Carrier sense indication */
#define EHT_TRIG_CMNINFO_CSREQ_FSZ		1
#define EHT_TRIG_CMNINFO_BWINFO_INDX		18	/* Bw info */
#define EHT_TRIG_CMNINFO_BWINFO_FSZ		2
#define EHT_TRIG_CMNINFO_GI_LTF_INDX		20	/* Cp-LTF size */
#define EHT_TRIG_CMNINFO_GI_LTF_FSZ		2
#define EHT_TRIG_CMNINFO_MUMIMO_LTF_INDX	22	/* EHT-LTF mask enable */
#define EHT_TRIG_CMNINFO_MUMIMO_LTF_FSZ		1
#define EHT_TRIG_CMNINFO_HELTF_SYM_INDX		23	/* EHT-LTF symbols */
#define EHT_TRIG_CMNINFO_HELTF_SYM_FSZ		3
#define EHT_TRIG_CMNINFO_STBC_INDX		26	/* STBC support */
#define EHT_TRIG_CMNINFO_STBC_FSZ		1
#define EHT_TRIG_CMNINFO_LDPC_EXTSYM_INDX	27	/* LDPC extra symbol */
#define EHT_TRIG_CMNINFO_LDPC_EXTSYM_FSZ	1
#define EHT_TRIG_CMNINFO_AP_TXPWR_INDX		28	/* AP TX power */
#define EHT_TRIG_CMNINFO_AP_TXPWR_FSZ		6
#define EHT_TRIG_CMNINFO_AFACT_INDX		34	/* a-factor */
#define EHT_TRIG_CMNINFO_AFACT_FSZ		2
#define EHT_TRIG_CMNINFO_PEDISAMBIG_INDX	36	/* PE disambiguity */
#define EHT_TRIG_CMNINFO_PEDISAMBIG_FSZ		1
#define EHT_TRIG_CMNINFO_SPTIAL_REUSE_INDX	37	/* spatial re-use */
#define EHT_TRIG_CMNINFO_SPTIAL_REUSE_FSZ	16
#define EHT_TRIG_CMNINFO_DOPPLER_INDX		53	/* doppler supoort */
#define EHT_TRIG_CMNINFO_DOPPLER_FSZ		1
#define EHT_TRIG_CMNINFO_P160_RSVD_INDX		54	/* P160 */
#define EHT_TRIG_CMNINFO_P160_RSVD_FSZ		1
#define EHT_TRIG_CMNINFO_SPUSR_INDX		55	/* special user present */
#define EHT_TRIG_CMNINFO_SPUSR_FSZ		1
#define EHT_TRIG_CMNINFO_RSVD1_INDX		56	/* reseved bit */
#define EHT_TRIG_CMNINFO_RSVD1_FSZ		7
#define EHT_TRIG_CMNINFO_RSVD2_INDX		63	/* reseved bit */
#define EHT_TRIG_CMNINFO_RSVD2_FSZ		1

/* EHT Basic trigger frame user info fields */

/* bit position and field width */
#define EHT_TRIG_USRINFO_AID_INDX		0	/* AID */
#define EHT_TRIG_USRINFO_AID_FSZ		12
#define EHT_TRIG_USRINFO_RU_ALLOC_INDX		12	/* RU allocation index */
#define EHT_TRIG_USRINFO_RU_ALLOC_FSZ		8
#define EHT_TRIG_USRINFO_CODING_INDX		20	/* coding type (BCC/LDPC) */
#define EHT_TRIG_USRINFO_CODING_FSZ		1
#define EHT_TRIG_USRINFO_MCS_INDX		21	/* MCS index value */
#define EHT_TRIG_USRINFO_MCS_FSZ		4
#define EHT_TRIG_USRINFO_RSVD_INDX		25	/* rsvd */
#define EHT_TRIG_USRINFO_RSVD_FSZ		1
#define EHT_TRIG_USRINFO_SSALLOC_STRMOFFSET_INDX	26	/* stream offset */
#define EHT_TRIG_USRINFO_SSALLOC_STRMOFFSET_FSZ		4
#define EHT_TRIG_USRINFO_SSALLOC_NSS_INDX		30	/* number of spatial streams */
#define EHT_TRIG_USRINFO_SSALLOC_NSS_FSZ		2
#define EHT_TRIG_USRINFO_TARGET_RSSI_INDX	32	/* Target RSSI */
#define EHT_TRIG_USRINFO_TARGET_RSSI_FSZ	7
#define EHT_TRIG_USRINFO_PS160_INDX		39	/* PS160 */
#define EHT_TRIG_USRINFO_PS160_FSZ		1
#define EHT_TRIG_USRINFO_TYPEDEP_INDX		40	/* type-dep per user info byte */
#define EHT_TRIG_USRINFO_TYPEDEP_FSZ		8

/* EHT Basic trigger frame special user info fields */

/* bit position and field width */
#define EHT_TRIG_SPUSRINFO_AID_INDX		0	/* AID */
#define EHT_TRIG_SPUSRINFO_AID_FSZ		12
#define EHT_TRIG_SPUSRINFO_PHYVER_INDX		12	/* PHY version ID */
#define EHT_TRIG_SPUSRINFO_PHYVER_FSZ		3
#define EHT_TRIG_SPUSRINFO_ULBWEXT_INDX		15	/* UL bandwidth extension */
#define EHT_TRIG_SPUSRINFO_ULBWEXT_FSZ		2
#define EHT_TRIG_SPUSRINFO_SR1_INDX		17	/* spatial reuse 1 */
#define EHT_TRIG_SPUSRINFO_SR1_FSZ		4
#define EHT_TRIG_SPUSRINFO_SR2_INDX		21	/* spatial reuse 2 */
#define EHT_TRIG_SPUSRINFO_SR2_FSZ		4
#define EHT_TRIG_SPUSRINFO_USIGDIS_INDX		25	/* usig disregard and validate */
#define EHT_TRIG_SPUSRINFO_USIGDIS_FSZ		12
#define EHT_TRIG_SPUSRINFO_RSVD_INDX		37	/* rsvd */
#define EHT_TRIG_SPUSRINFO_RSVD_FSZ		3

/* =========================== define derivative constants =========================== */

#define EHT_MCS_MAP_NSS_SIZE 4 /**< each Nss field within an mcs map takes 4 bits */
#define EHT_MCS_MAP_GET_NSS_IDX(nss) (((nss) - 1) * EHT_MCS_MAP_MCSCODE_SIZE)

/* fixed fields in the psta info of ML IE */
#define EHT_PSTA_FIXED_FIELD_LEN_PRBRESP       2
#define EHT_PSTA_FIXED_FIELD_LEN_ASSOCRESP     4 /* ASSOC/REASSOC RESP */
#define EHT_PSTA_FIXED_FIELD_LEN_ASSOCREQ      2 /* ASSOC/REASSOC REQ */

#endif /* _802_11be_h_ */
