/*
 * Basic types and constants relating to 802.11ax/HE STA
 * This is a portion of 802.11ax definition. The rest are in 802.11.h.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: 802.11ax.h 787811 2020-06-12 08:11:34Z $
 */

#ifndef _802_11ax_h_
#define _802_11ax_h_

#include <typedefs.h>

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* special STA-IDs (Table 28-22) */
#define HE_STAID_BSS_BCAST		0
#define HE_STAID_MBSS_BCAST		2047
#define HE_STAID_RU_NODATA		2046

#define HE_STAID_MASK			0x07FF

/**
 * HE Capabilites element (IEEE Draft P802.11ax D3.0 9.4.2.237)
 */

/* HE MAC Capabilities Information field (figure 9-589ck) */
#define HE_MAC_CAP_INFO_SIZE	6
typedef uint8 he_mac_cap_t[HE_MAC_CAP_INFO_SIZE];

/* bit position and field width */
#define HE_MAC_HTC_HE_SUPPORT_IDX			0	/* HTC HE Support */
#define HE_MAC_HTC_HE_SUPPORT_FSZ			1
#define HE_MAC_TWT_REQ_SUPPORT_IDX			1	/* TWT Requestor Support */
#define HE_MAC_TWT_REQ_SUPPORT_FSZ			1
#define HE_MAC_TWT_RESP_SUPPORT_IDX			2	/* TWT Responder Support */
#define HE_MAC_TWT_RESP_SUPPORT_FSZ			1
#define HE_MAC_FRAG_SUPPORT_IDX				3	/* Fragmentation Support */
#define HE_MAC_FRAG_SUPPORT_FSZ				2
#define HE_MAC_MAX_MSDU_FRAGS_IDX			5	/* Max. Fragmented MSDUs */
#define HE_MAC_MAX_MSDU_FRAGS_FSZ			3
#define HE_MAC_MIN_FRAG_SIZE_IDX			8	/* Min. Fragment Size */
#define HE_MAC_MIN_FRAG_SIZE_FSZ			2
#define HE_MAC_TRG_PAD_DUR_IDX				10	/* Trigger Frame MAC Pad Dur */
#define HE_MAC_TRG_PAD_DUR_FSZ				2
#define HE_MAC_MULTI_TID_AGG_IDX			12	/* Multi TID Aggregation */
#define HE_MAC_MULTI_TID_AGG_FSZ			3
#define HE_MAC_LINK_ADAPT_IDX				15	/* HE Link Adaptation */
#define HE_MAC_LINK_ADAPT_FSZ				2
#define HE_MAC_ALL_ACK_SUPPORT_IDX			17	/* All Ack Support */
#define HE_MAC_ALL_ACK_SUPPORT_FSZ			1
#define HE_MAC_UL_MU_RESP_SCHED_IDX			18	/* UL MU Response Scheduling */
#define HE_MAC_UL_MU_RESP_SCHED_FSZ			1
#define HE_MAC_A_BSR_IDX				19	/* A-BSR Support */
#define HE_MAC_A_BSR_FSZ				1
#define HE_MAC_BCAST_TWT_SUPPORT_IDX			20	/* Broadcast TWT Support */
#define HE_MAC_BCAST_TWT_SUPPORT_FSZ			1
#define HE_MAC_BA_32BITMAP_SUPPORT_IDX			21	/* 32-bit BA Bitmap Support */
#define HE_MAC_BA_32BITMAP_SUPPORT_FSZ			1
#define HE_MAC_MU_CASCADE_SUPPORT_IDX			22	/* MU Cascade Support */
#define HE_MAC_MU_CASCADE_SUPPORT_FSZ			1
#define HE_MAC_MULTI_TID_AGG_ACK_IDX			23	/* Ack Enabled Multi TID Agg. */
#define HE_MAC_MULTI_TID_AGG_ACK_FSZ			1
#define HE_MAC_B24_RESERVED_IDX				24	/* Reserved */
#define HE_MAC_B24_RESERVED_FSZ				1
#define HE_MAC_OM_CONTROL_SUPPORT_IDX			25	/* OM Control Support */
#define HE_MAC_OM_CONTROL_SUPPORT_FSZ			1
#define HE_MAC_OFDMA_RA_SUPPORT_IDX			26	/* OFDMA RA Support */
#define HE_MAC_OFDMA_RA_SUPPORT_FSZ			1
#define HE_MAC_MAX_AMPDU_LEN_EXP_IDX			27	/* Max AMPDU Length Exponent */
#define HE_MAC_MAX_AMPDU_LEN_EXP_FSZ			2
#define HE_MAC_AMSDU_FRAG_SUPPORT_IDX			29	/* AMSDU Fragementation Support */
#define HE_MAC_AMSDU_FRAG_SUPPORT_FSZ			1
#define HE_MAC_FLEX_TWT_SCHEDULE_IDX			30	/* Flexible TWT Schedule */
#define HE_MAC_FLEX_TWT_SCHEDULE_FSZ			1
#define HE_MAC_RX_MBSS_CTL_FRAME_IDX			31	/* Rx Control Frame to MultiBSS */
#define HE_MAC_RX_MBSS_CTL_FRAME_FSZ			1
#define HE_MAC_BSRP_BQRP_AGG_IDX			32	/* BSRP BQRP AMPDU Aggregation */
#define HE_MAC_BSRP_BQRP_AGG_FSZ			1
#define HE_MAC_QTP_SUPPORT_IDX				33	/* Support Quiet time period */
#define HE_MAC_QTP_SUPPORT_FSZ				1
#define HE_MAC_BQR_SUPPORT_IDX				34	/* BQR Support */
#define HE_MAC_BQR_SUPPORT_FSZ				1
#define HE_MAC_SRP_RESPONDER_IDX			35	/* SRP Responder */
#define HE_MAC_SRP_RESPONDER_FSZ			1
#define HE_MAC_NDP_FB_SUPPORT_IDX			36	/* NDP Feedback Report Support */
#define HE_MAC_NDP_FB_SUPPORT_FSZ			1
#define HE_MAC_OPS_SUPPORT_IDX				37	/* OPS Support */
#define HE_MAC_OPS_SUPPORT_FSZ				1
#define HE_MAC_AMSDU_IN_AMPDU_IDX			38	/* A-MSDU In A-MPDU Support */
#define HE_MAC_AMSDU_IN_AMPDU_FSZ			1
#define HE_MAC_MULTI_TID_AGG_TX_IDX			39	/* Multi-TID Aggregation Tx Supp. */
#define HE_MAC_MULTI_TID_AGG_TX_FSZ			3
#define HE_MAC_HE_SUBCHAN_SEL_TX_IDX			42	/* HE Subchannel Selective Trans. */
#define HE_MAC_HE_SUBCHAN_SEL_TX_FSZ			1
#define HE_MAC_UL_2X996_TONE_RU_IDX			43	/* UL 2x996-tone RU Support */
#define HE_MAC_UL_2X996_TONE_RU_FSZ			1
#define HE_MAC_OM_UL_MU_DATA_DISABLE_IDX		44	/* OM Ctrl UL MU Data Disable RX */
#define HE_MAC_OM_UL_MU_DATA_DISABLE_FSZ		1

/* HT Control Field: (Table 9-9a) */
#define HTC_HE_VARIANT					0x3

/* HT Control IDs: (Table 9-18a) */
#define HTC_HE_CTLID_SHIFT				0x2
#define HTC_HE_CTLID_ULMURSP				0x0
#define HTC_HE_CTLID_OPMODE				0x1
#define HTC_HE_CTLID_LNKADPT				0x2
#define HTC_HE_CTLID_BSR				0x3
#define HTC_HE_CTLID_ULPWR				0x4
#define HTC_HE_CTLID_BQR				0x5
#define HTC_HE_CTLID_CCI				0x6

/* HE PHY Capabilities Information field (figure 9-589cl) */
#define HE_PHY_CAP_INFO_SIZE				11
typedef uint8 he_phy_cap_t[HE_PHY_CAP_INFO_SIZE];

/* bit position and field width */
#define HE_PHY_B0_RESERVED_IDX				0	/* Reserved */
#define HE_PHY_B0_RESERVED_FSZ				1
#define HE_PHY_CH_WIDTH_SET_IDX				1	/* Channel Width Set */
#define HE_PHY_CH_WIDTH_SET_FSZ				7
#define HE_PHY_PREAMBLE_PUNCT_RX_IDX			8	/* Preamble Puncturing Rx */
#define HE_PHY_PREAMBLE_PUNCT_RX_FSZ			4
#define HE_PHY_DEVICE_CLASS_IDX				12	/* Device Class */
#define HE_PHY_DEVICE_CLASS_FSZ				1
#define HE_PHY_LDPC_PYLD_IDX				13	/* LDPC Coding In Payload */
#define HE_PHY_LDPC_PYLD_FSZ				1
#define HE_PHY_SU_PPDU_1x_LTF_0_8_GI_IDX		14	/* SU PPDU 1x LTF GI 0.8 us */
#define HE_PHY_SU_PPDU_1x_LTF_0_8_GI_FSZ		1
#define HE_PHY_MIDAMBLE_TXRX_MAX_NSTS_IDX		15	/* Midamble Tx/Rx Max NSTS */
#define HE_PHY_MIDAMBLE_TXRX_MAX_NSTS_FSZ		2
#define HE_PHY_NDP_4x_LTF_3_2_GI_RX_IDX			17	/* NDP with 4xLTF 3.2us GI Rx */
#define HE_PHY_NDP_4x_LTF_3_2_GI_RX_FSZ			1
#define HE_PHY_STBC_TX_IDX				18	/* STBC Tx <= 80 Mhz */
#define HE_PHY_STBC_TX_FSZ				1
#define HE_PHY_STBC_RX_IDX				19	/* STBC Rx <= 80 Mhz */
#define HE_PHY_STBC_RX_FSZ				1
#define HE_PHY_DOPPLER_TX_IDX				20	/* Doppler Tx */
#define HE_PHY_DOPPLER_TX_FSZ				1
#define HE_PHY_DOPPLER_RX_IDX				21	/* Doppler Rx */
#define HE_PHY_DOPPLER_RX_FSZ				1
#define HE_PHY_FULL_BW_UL_MU_IDX			22	/* Full bandwidth UL MU */
#define HE_PHY_FULL_BW_UL_MU_FSZ			1
#define HE_PHY_PART_BW_UL_MU_IDX			23	/* Partial bandwidth UL MU */
#define HE_PHY_PART_BW_UL_MU_FSZ			1
#define HE_PHY_DCM_TX_CONST_IDX				24	/* DCM Max Constellation Tx */
#define HE_PHY_DCM_TX_CONST_FSZ				2
#define HE_PHY_DCM_TX_NSS_IDX				26	/* DCM Max NSS Tx */
#define HE_PHY_DCM_TX_NSS_FSZ				1
#define HE_PHY_DCM_RX_CONST_IDX				27	/* DCM Max Constellation Rx */
#define HE_PHY_DCM_RX_CONST_FSZ				2
#define HE_PHY_DCM_RX_NSS_IDX				29	/* DCM Max NSS Rx */
#define HE_PHY_DCM_RX_NSS_FSZ				1
#define HE_PHY_UL_MU_PYLD_IDX				30	/* UL MU Payload Support */
#define HE_PHY_UL_MU_PYLD_FSZ				1
#define HE_PHY_SU_BEAMFORMER_IDX			31	/* SU Beamformer */
#define HE_PHY_SU_BEAMFORMER_FSZ			1
#define HE_PHY_SU_BEAMFORMEE_IDX			32	/* SU Beamformee */
#define HE_PHY_SU_BEAMFORMEE_FSZ			1
#define HE_PHY_MU_BEAMFORMER_IDX			33	/* MU Beamformer */
#define HE_PHY_MU_BEAMFORMER_FSZ			1
#define HE_PHY_BEAMFORMEE_STS_BELOW80MHZ_IDX		34	/* Beamformee STS <= 80MHz */
#define HE_PHY_BEAMFORMEE_STS_BELOW80MHZ_FSZ		3
#define HE_PHY_BEAMFORMEE_STS_ABOVE80MHZ_IDX		37	/* Beamformee STS >80 MHz */
#define HE_PHY_BEAMFORMEE_STS_ABOVE80MHZ_FSZ		3
#define HE_PHY_SOUND_DIM_BELOW80MHZ_IDX			40	/* Num. Sounding Dim.<= 80 MHz */
#define HE_PHY_SOUND_DIM_BELOW80MHZ_FSZ			3
#define HE_PHY_SOUND_DIM_ABOVE80MHZ_IDX			43	/* Num. Sounding Dim.> 80 MHz */
#define HE_PHY_SOUND_DIM_ABOVE80MHZ_FSZ			3
#define HE_PHY_SU_FEEDBACK_NG16_SUPPORT_IDX		46	/* Ng=16 SU Feedback */
#define HE_PHY_SU_FEEDBACK_NG16_SUPPORT_FSZ		1
#define HE_PHY_MU_FEEDBACK_NG16_SUPPORT_IDX		47	/* Ng=16 MU Feedback */
#define HE_PHY_MU_FEEDBACK_NG16_SUPPORT_FSZ		1
#define HE_PHY_SU_CODEBOOK_SUPPORT_IDX			48	/* Codebook Sz {4, 2} SU Feedback */
#define HE_PHY_SU_CODEBOOK_SUPPORT_FSZ			1
#define HE_PHY_MU_CODEBOOK_SUPPORT_IDX			49	/* Codebook Sz {7, 5} MU Feedback */
#define HE_PHY_MU_CODEBOOK_SUPPORT_FSZ			1
#define HE_PHY_TRG_SU_BFM_FEEDBACK_IDX			50	/* Triggered SU TXBF Feedback */
#define HE_PHY_TRG_SU_BFM_FEEDBACK_FSZ			1
#define HE_PHY_TRG_MU_BFM_FEEDBACK_IDX			51	/* Triggered MU TXBF Feedback */
#define HE_PHY_TRG_MU_BFM_FEEDBACK_FSZ			1
#define HE_PHY_TRG_CQI_FEEDBACK_IDX			52	/* Triggered CQI Feedback */
#define HE_PHY_TRG_CQI_FEEDBACK_FSZ			1
#define HE_PHY_EXT_RANGE_PART_BW_IDX			53	/* Partial Bandwidth Ext. Range */
#define HE_PHY_EXT_RANGE_PART_BW_FSZ			1
#define HE_PHY_DL_MU_MIMO_PART_BW_IDX			54	/* Partial Bandwidth DL MUMIMO */
#define HE_PHY_DL_MU_MIMO_PART_BW_FSZ			1
#define HE_PHY_PPE_THRESH_PRESENT_IDX			55	/* PPE Threshold Present */
#define HE_PHY_PPE_THRESH_PRESENT_FSZ			1
#define HE_PHY_SRP_SR_SUPPORT_IDX			56	/* SRPbased SR Support */
#define HE_PHY_SRP_SR_SUPPORT_FSZ			1
#define HE_PHY_POWER_BOOST_FACTOR_IDX			57	/* Power Boost Factor Support */
#define HE_PHY_POWER_BOOST_FACTOR_FSZ			1
#define HE_PHY_LONG_LTF_SHORT_GI_SU_PPDU_IDX		58	/* HE SU PPDU - Long LTF Short GI */
#define HE_PHY_LONG_LTF_SHORT_GI_SU_PPDU_FSZ		1
#define HE_PHY_MAX_NC_IDX				59	/* Max Nc */
#define HE_PHY_MAX_NC_FSZ				3
#define HE_PHY_STBC_TX_ABOVE80MHZ_IDX			62	/* STBC Tx > 80 MHz */
#define HE_PHY_STBC_TX_ABOVE80MHZ_FSZ			1
#define HE_PHY_STBC_RX_ABOVE80MHZ_IDX			63	/* STBC Rx > 80 MHz */
#define HE_PHY_STBC_RX_ABOVE80MHZ_FSZ			1
#define HE_PHY_SUPPDU_4xLTF_08GI_IDX			64	/* HE ER SU PPDU With 4x HE-LTF */
#define HE_PHY_SUPPDU_4xLTF_08GI_FSZ			1	/* And 0.8 us GI */
#define HE_PHY_20M_IN_40M_PPDU_2G_IDX			65	/* 20 MHz In 40 MHz HE PPDU In */
#define HE_PHY_20M_IN_40M_PPDU_2G_FSZ			1	/* 2.4 GHz Band */
#define HE_PHY_20M_IN_160MOR80P80_PPDU_IDX		66	/* 20 MHz In 160/80+80MHz HE PPDU */
#define HE_PHY_20M_IN_160MOR80P80_PPDU_FSZ		1
#define HE_PHY_80M_IN_160MOR80P80_PPDU_IDX		67	/* 80 MHz In 160/80+80MHz HE PPDU */
#define HE_PHY_80M_IN_160MOR80P80_PPDU_FSZ		1
#define HE_PHY_ER_SU_PPDU_1xLTF_08GI_IDX		68	/* HE ER SU PPDU With 1x HE-LTF */
#define HE_PHY_ER_SU_PPDU_1xLTF_08GI_FSZ		1	/* And 0.8 us GI */
#define HE_PHY_MIDAMBLE_2xRx_1xLTF_IDX			69	/* Midamble Rx 2x And 1x HE-LTF */
#define HE_PHY_MIDAMBLE_2xRx_1xLTF_FSZ			1
#define HE_PHY_DCM_MAX_BW_IDX				70	/* DCM Max BW */
#define HE_PHY_DCM_MAX_BW_FSZ				2
#define HE_PHY_LONGER_THAN_16_HE_SIG_B_OFDM_SYMBOLS_IDX	72	/* Lgr than 16 HE SIG-B OFDM Syms */
#define HE_PHY_LONGER_THAN_16_HE_SIG_B_OFDM_SYMBOLS_FSZ	1
#define HE_PHY_NON_TRIGGERED_CQI_FEEDBACK_IDX		73	/* Non-Triggered CQI Feedback */
#define HE_PHY_NON_TRIGGERED_CQI_FEEDBACK_FSZ		1
#define HE_PHY_TX_1024_QAM_LESS_242_TONE_RU_IDX		74	/* Tx 1024-QAM < 242-tone RU */
#define HE_PHY_TX_1024_QAM_LESS_242_TONE_RU_FSZ		1
#define HE_PHY_RX_1024_QAM_LESS_242_TONE_RU_IDX		75	/* Rx 1024-QAM < 242-tone RU */
#define HE_PHY_RX_1024_QAM_LESS_242_TONE_RU_FSZ		1
#define HE_PHY_RX_FULL_BW_SU_HE_MU_COMPRD_SIGB_IDX	76	/* Rx Full BW SU Using HE U PPDU */
#define HE_PHY_RX_FULL_BW_SU_HE_MU_COMPRD_SIGB_FSZ	1	/* With Compressed SIGB */
#define HE_PHY_RX_FULL_BW_SU_HE_MU_NONCOMPRD_SIGB_IDX	77	/* Rx Full BW SU Using HE MU PPDU */
#define HE_PHY_RX_FULL_BW_SU_HE_MU_NONCOMPRD_SIGB_FSZ	1	/* With Non-Compressed SIGB */

/* HE Mac Capabilities values (table 9-262z) */

/* b3-b4: Fragmentation Support field */
#define HE_MAC_FRAG_NOSUPPORT		0	/* dynamic frag not supported */
#define HE_MAC_FRAG_VHT_MPDU		1	/* Frag support for VHT single MPDU only */
#define HE_MAC_FRAG_ONE_PER_AMPDU	2	/* 1 frag per MPDU in A-MPDU */
#define HE_MAC_FRAG_MULTI_PER_AMPDU	3	/* 2+ frag per MPDU in A-MPDU */

/* b8-b9: Minimum payload size of first fragment */
#define HE_MAC_MINFRAG_NO_RESTRICT	0	/* no restriction on min. payload size */
#define HE_MAC_MINFRAG_SIZE_128		1	/* minimum payload size of 128 Bytes */
#define HE_MAC_MINFRAG_SIZE_256		2	/* minimum payload size of 256 Bytes */
#define HE_MAC_MINFRAG_SIZE_512		3	/* minimum payload size of 512 Bytes */

/* b10-b11: Trigger Frame MAC Padding Duration */
#define HE_MAC_TRG_PAD_DUR_0		0	/* no additional processing time */
#define HE_MAC_TRG_PAD_DUR_8		1	/* 8 usec processing time */
#define HE_MAC_TRG_PAD_DUR_16		2	/* 16 usec processing time */

/* b15-b16: HE Link Adaptation */
#define HE_MAC_SEND_NO_MFB		0	/* if STA does not provide HE MFB */
#define HE_MAC_SEND_UNSOLICATED_MFB	2	/* if STA provides unsolicited HE MFB */
#define HE_MAC_SEND_MFB_IN_RESPONSE	3	/* if STA can provide HE MFB in response to HE
						 * MRQ and if the STA provides unsolicited HE MFB.
						 */

#define HE_MAC_AMPDU_MAX_LEN		6500631 /* ref 26.6.1 of 802.11axD6.0 */

/* HE PHY Capabilities values */
/* b1-b7: Channel Width Support field */
#define HE_PHY_CH_WIDTH_2G_40		0x01
#define HE_PHY_CH_WIDTH_5G_80		0x02
#define HE_PHY_CH_WIDTH_5G_160		0x04
#define HE_PHY_CH_WIDTH_5G_80P80	0x08
#define HE_PHY_CH_WIDTH_2G_242TONE	0x10
#define HE_PHY_CH_WIDTH_5G_242TONE	0x20
/* Keep next bits for TWIG build. Needs updating later once removed there */
#define HE_PHY_CH_WIDTH_2G_40_RU	0x10
#define HE_PHY_CH_WIDTH_5G_80_RU	0x20

/* b8-b11: Preamble puncturing Rx */
#define HE_PHY_PREAMBLE_PUNC_RX_0	0x1
#define HE_PHY_PREAMBLE_PUNC_RX_1	0x2
#define HE_PHY_PREAMBLE_PUNC_RX_2	0x4
#define HE_PHY_PREAMBLE_PUNC_RX_3	0x8

/* b24-b29: DCM Encoding at Tx and Rx */
#define HE_PHY_TX_DCM_ENC_NOSUPPORT	0x00
#define HE_PHY_TX_DCM_ENC_BPSK		0x01
#define HE_PHY_TX_DCM_ENC_QPSK		0x02
#define HE_PHY_TX_DCM_ENC_QAM		0x03

#define HE_PHY_TX_DCM_1_SS		0x00
#define HE_PHY_TX_DCM_2_SS		0x04

#define HE_PHY_RX_DCM_ENC_NOSUPPORT	0x00
#define HE_PHY_RX_DCM_ENC_BPSK		0x08
#define HE_PHY_RX_DCM_ENC_QPSK		0x10
#define HE_PHY_RX_DCM_ENC_QAM		0x18

#define HE_PHY_RX_DCM_1_SS		0x00
#define HE_PHY_RX_DCM_2_SS		0x20

/* HE Duration based RTS Threshold IEEE Draft P802.11ax D2.0 Figure 9-589cr */
#define HE_RTS_THRES_DISABLED		1023
#define HE_RTS_THRES_ALL_FRAMES		0
#define HE_RTS_THRES_MASK		0x3ff

/* HE Capabilities element */
BWL_PRE_PACKED_STRUCT struct he_cap_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	he_mac_cap_t mac_cap;		/* MAC Capabilities Information */
	he_phy_cap_t phy_cap;		/* PHY Capabilities Information */
	/* Supported HE-MCS And NSS Set (4, 8 or 12 bytes) */
	/* PPE Thresholds (optional) (variable length) */
} BWL_POST_PACKED_STRUCT;

typedef struct he_cap_ie he_cap_ie_t;

/* IEEE Draft P802.11ax D3.0 Text following figure 9-589cn-Rx HE-MCS Map and Tx HE-MCS Map
 * subfields and Basic HE-MCS And NSS Set field
 */
#define HE_CAP_MAX_MCS_0_7		0
#define HE_CAP_MAX_MCS_0_9		1
#define HE_CAP_MAX_MCS_0_11		2
#define HE_CAP_MAX_MCS_NONE		3
#define HE_CAP_MAX_MCS_SIZE		2	/* num bits for 1-stream */
#define HE_CAP_MAX_MCS_MASK		0x3	/* mask for 1-stream */

#define HE_CAP_MCS_MAP_NSS_MAX		8	/* Max number of streams possible */

#define HE_MAX_RU_COUNT			4	/* Max number of RU allocation possible */

#define HE_NSSM1_IDX			0	/* Offset of NSSM1 field */
#define HE_NSSM1_LEN			3	/* length of NSSM1 field in bits */
#define HE_RU_INDEX_MASK_IDX		3	/* Offset of RU index mask field */
#define HE_RU_INDEX_MASK_LEN		4	/* length of RU Index mask field in bits */

BWL_PRE_PACKED_STRUCT struct he_mu_ac_param {
	uint8 aci_aifsn;
	uint8 ecw_min_max;
	uint8 muedca_timer;
} BWL_POST_PACKED_STRUCT;

typedef struct he_mu_ac_param he_mu_ac_param_t;

/* HE MU EDCA Parameter Set element */
BWL_PRE_PACKED_STRUCT struct he_muedca_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	uint8 mu_qos_info;
	he_mu_ac_param_t ac_param[AC_COUNT];
} BWL_POST_PACKED_STRUCT;

typedef struct he_muedca_ie he_muedca_ie_t;

/* Figure 9-768p - SR Control field format */
#define HE_SR_CONTROL_SRP_DISALLOWED_IDX			0
#define HE_SR_CONTROL_SRP_DISALLOWED_FSZ			1
#define HE_SR_CONTROL_NON_SRG_OBSS_PD_SR_DISALLOWED_IDX		1
#define HE_SR_CONTROL_NON_SRG_OBSS_PD_SR_DISALLOWED_FSZ		1
#define HE_SR_CONTROL_NON_SRG_OFFSET_PRESENT_IDX		2
#define HE_SR_CONTROL_NON_SRG_OFFSET_PRESENT_FSZ		1
#define HE_SR_CONTROL_SRG_INFORMATION_PRESENT_IDX		3
#define HE_SR_CONTROL_SRG_INFORMATION_PRESENT_FSZ		1
#define HE_SR_CONTROL_HESIGA_SPATIAL_REUSE_VALUE15_ALLOWED_IDX	4
#define HE_SR_CONTROL_HESIGA_SPATIAL_REUSE_VALUE15_ALLOWED_FSZ	1

/* 9.4.2.245 HE Spatial Reuse Parameter Set element */
BWL_PRE_PACKED_STRUCT struct he_spatial_reuse_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	uint8 sr_control;
} BWL_POST_PACKED_STRUCT;

typedef struct he_spatial_reuse_ie he_spatial_reuse_ie_t;

/* After HE Spatial reuse two sub parameters can be added. first type for Non-SRG Offset Present: */
typedef uint8 non_srg_obss_pd_max_offset_t;

/* HE Spatial Reuse Parameter Set element sub fields - when SRG Information Present is 1: */
BWL_PRE_PACKED_STRUCT struct he_spatial_reuse_sub_ie {
	uint8 srg_obss_pd_min_offset;
	uint8 srg_obss_pd_max_offset;
	uint8 srg_bss_color_bitmap[8];
	uint8 srg_partial_bssid_bitmap[8];
} BWL_POST_PACKED_STRUCT;

typedef struct he_spatial_reuse_sub_ie he_spatial_reuse_sub_ie_t;

/* HE Color Change Announcement element */
BWL_PRE_PACKED_STRUCT struct he_colorchange_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	uint8 countdown;
	uint8 newcolor;
} BWL_POST_PACKED_STRUCT;

typedef struct he_colorchange_ie he_colorchange_ie_t;

/* New approach: different define for SU/MU/TB. Need to be cleaned up */
#define HE_SIGA_MCS_MASK			0x00000078
#define HE_SIGA_BEAM_CHANGE_SHIFT	1
#define HE_SIGA_UL_DL_SHIFT			2
#define HE_SIGA_MCS_SHIFT			3
#define HE_SIGA_DCM_SHIFT			7
#define HE_SIGA_BSS_COLOR_MASK		0x00003F00
#define HE_SIGA_BSS_COLOR_SHIFT		8
#define HE_SIGA_BW_MASK				0x00180000
#define HE_SIGA_BW_SHIFT			19
#define HE_SIGA2_FEC_MASK			0x00001800
#define HE_SIGA2_LDPC_EXTSYM_SHIFT	8
#define HE_SIGA2_STBC_SHIFT			9
#define HE_SIGA2_TXBF_SHIFT			10
#define HE_SIGA2_FEC_SHIFT			11
#define HE_SIGA2_PED_SHIFT			13
#define HE_SIGA2_DOPPLER_SHIFT		15
#define HE_SIGA_CPLTF_MASK			0x00600000
#define HE_SIGA_CPLTF_SHIFT			21

/* For HE SIG A : PLCP0 bit fields [assuming 32bit plcp] */
#define HE_SIGA_MASK			0x03FFFFFF
#define HE_SIGA_FORMAT_HE_SU		0x00000001
#define HE_SIGA_BEAM_CHANGE_PLCP0	0x00000002
#define HE_SIGA_UL_DL_PLCP0		0x00000004
#define HE_SIGA_TXOP_PLCP0		0xFC000000
#define HE_SIGA_RESERVED_PLCP0		0x00004000

/* For HE SIG A : PLCP1 bit fields [assuming 32bit plcp] */
#define HE_SIGA_TXOP_PLCP1		0x00000001
#define HE_SIGA_CODING_LDPC		0x00000002
#define HE_SIGA_STBC			0x00000008
#define HE_SIGA_BEAMFORM_ENABLE		0x00000010
#define HE_SIGA_RESERVED_PLCP1		0x00000100

#define HE_PPDU_SU		0
#define HE_PPDU_ERSU	1
#define HE_PPDU_MU		2
#define HE_PPDU_TB		3

/* For HE SIG A : PLCP0 bit shifts/masks/vals */
#define HESU_SIGA_BEAM_CHANGE_SHIFT	1
#define HESU_SIGA_UL_DL_SHIFT		2
#define HESU_SIGA_UL_DL(siga)		((siga >> HESU_SIGA_UL_DL_SHIFT) & 0x1)
#define HEMU_SIGA_UL_DL_SHIFT		0
#define HEMU_SIGA_UL_DL(siga)		((siga >> HEMU_SIGA_UL_DL_SHIFT) & 0x1)
#define HESU_SIGA_MCS_MASK		0x00000078
#define HESU_SIGA_MCS_SHIFT		3
#define HESU_SIGA_MCS(siga)		((siga & HESU_SIGA_MCS_MASK) >> HESU_SIGA_MCS_SHIFT)
#define HESU_SIGA_DCM_SHIFT		7
#define HESU_SIGA_DCM(siga)		((siga >> HESU_SIGA_DCM_SHIFT) & 0x1)

#define HESU_SIGA_BSS_COLOR_MASK	0x00003F00
#define HEMU_SIGA_BSS_COLOR_MASK	0x000007E0
#define HETB_SIGA_BSS_COLOR_MASK	0x0000007E
#define HESU_SIGA_BSS_COLOR_SHIFT	8
#define HEMU_SIGA_BSS_COLOR_SHIFT	5
#define HETB_SIGA_BSS_COLOR_SHIFT	1
#define HE_SIGA_BSS_COLOR_MASK_(he_format) \
	((he_format == HE_PPDU_MU) ? HEMU_SIGA_BSS_COLOR_MASK : \
	(he_format == HE_PPDU_TB) ? HETB_SIGA_BSS_COLOR_MASK : \
	HESU_SIGA_BSS_COLOR_MASK)
#define HE_SIGA_BSS_COLOR_SHIFT_(he_format) \
	((he_format == HE_PPDU_MU) ? HEMU_SIGA_BSS_COLOR_SHIFT : \
	(he_format == HE_PPDU_TB) ? HETB_SIGA_BSS_COLOR_SHIFT : \
	HESU_SIGA_BSS_COLOR_SHIFT)
#define HE_SIGA_BSS_COLOR(siga, he_format) \
	((siga & HE_SIGA_BSS_COLOR_MASK_(he_format)) >> HE_SIGA_BSS_COLOR_SHIFT_(he_format))
#define HEMU_SIGA_SPATIAL_REUSE_SHIFT	11
#define HETB_SIGA_SPATIAL_REUSE_SHIFT	7
#define HESU_SIGA_SPATIAL_REUSE_SHIFT	15
#define HE_SIGA_SPATIAL_REUSE_SHIFT(he_format) \
	((he_format == HE_PPDU_MU) ? HEMU_SIGA_SPATIAL_REUSE_SHIFT : \
	(he_format == HE_PPDU_TB) ? HETB_SIGA_SPATIAL_REUSE_SHIFT : \
	HESU_SIGA_SPATIAL_REUSE_SHIFT)
#define HE_SIGA_SPATIAL_REUSE(siga, he_format)	\
	((siga >> HE_SIGA_SPATIAL_REUSE_SHIFT(he_format)) & 0xf)
#define HETB_SIGA_SPATIAL_REUSE2_SHIFT	11
#define HETB_SIGA_SPATIAL_REUSE3_SHIFT	15
#define HETB_SIGA_SPATIAL_REUSE4_SHIFT	19
#define HETB_SIGA_SPATIAL_REUSE2(siga)	((siga >> HETB_SIGA_SPATIAL_REUSE2_SHIFT) & 0xf)
#define HETB_SIGA_SPATIAL_REUSE3(siga)	((siga >> HETB_SIGA_SPATIAL_REUSE3_SHIFT) & 0xf)
#define HETB_SIGA_SPATIAL_REUSE4(siga)	((siga >> HETB_SIGA_SPATIAL_REUSE4_SHIFT) & 0xf)

#define HESU_SIGA_BW_MASK		0x00180000
#define HESU_SIGA_BW_SHIFT		19
#define HEMU_SIGA_BW_MASK		0x00038000
#define HEMU_SIGA_BW_SHIFT		15
#define HETB_SIGA_BW_MASK		0x03000000
#define HETB_SIGA_BW_SHIFT		24
#define HE_SIGA_BW_MASK_(he_format) \
	((he_format == HE_PPDU_MU) ? HEMU_SIGA_BW_MASK : \
	(he_format == HE_PPDU_TB) ? HETB_SIGA_BW_MASK : HESU_SIGA_BW_MASK)
#define HE_SIGA_BW_SHIFT_(he_format) \
	((he_format == HE_PPDU_MU) ? HEMU_SIGA_BW_SHIFT : \
	(he_format == HE_PPDU_TB) ? HETB_SIGA_BW_SHIFT : HESU_SIGA_BW_SHIFT)
#define HE_SIGA_BW(siga, he_format) \
	((siga & HE_SIGA_BW_MASK_(he_format)) >> HE_SIGA_BW_SHIFT_(he_format))

#define HE_SIGA_NSTS_MASK		0x03800000
#define HE_SIGA_NSTS_SHIFT		23
#define HE_SIGA2_MASK			0x03FFFFFF
#define HE_SIGA2_TXOP_MASK		0x0000007F	/** The same define for HESU,
								HEMU and HETB
								*/
#define HE_SIGA2_TXOP(siga2)		(siga2 & HE_SIGA2_TXOP_MASK)

#define HESU_SIGA2_LDPC_EXTSYM_SHIFT	8
#define HESU_SIGA2_LDPC_EXTSYM(siga2)	((siga2 >> HESU_SIGA2_LDPC_EXTSYM_SHIFT) & 0x1)
#define HEMU_SIGA2_LDPC_EXTSYM_SHIFT	11
#define HEMU_SIGA2_LDPC_EXTSYM(siga2)	((siga2 >> HEMU_SIGA2_LDPC_EXTSYM_SHIFT) & 0x1)
#define HESU_SIGA2_STBC_SHIFT		9
#define HESU_SIGA2_STBC(siga2)		((siga2 >> HESU_SIGA2_STBC_SHIFT) & 0x1)
#define HEMU_SIGA2_STBC_SHIFT		12
#define HEMU_SIGA2_STBC(siga2)		((siga2 >> HEMU_SIGA2_STBC_SHIFT) & 0x1)

#define HESU_SIGA2_TXBF_SHIFT		10
#define HESU_SIGA2_FEC_MASK		0x00001800
#define HESU_SIGA2_FEC_SHIFT		11
#define HESU_SIGA2_FEC(siga2)		((siga2 & HESU_SIGA2_FEC_MASK) >> HESU_SIGA2_FEC_SHIFT)
#define HESU_SIGA2_PED_SHIFT		13
#define HESU_SIGA2_PED(siga2)		((siga2 >> HESU_SIGA2_PED_SHIFT) & 0x1)
#define HESU_SIGA2_DOPPLER_SHIFT	15
#define HESU_SIGA2_DOPPLER(siga2)	((siga2 >> HESU_SIGA2_DOPPLER_SHIFT) & 0x1)

#define HEMU_SIGA2_FEC_MASK		0x00006000
#define HEMU_SIGA2_FEC_SHIFT		13
#define HEMU_SIGA2_FEC(siga2)		((siga2 & HEMU_SIGA2_FEC_MASK) >> HEMU_SIGA2_FEC_SHIFT)
#define HEMU_SIGA2_PED_SHIFT		15
#define HEMU_SIGA2_PED(siga2)		((siga2 >> HEMU_SIGA2_PED_SHIFT) & 0x1)
#define HEMU_SIGA2_DOPPLER_SHIFT	25
#define HEMU_SIGA2_DOPPLER(siga2)	((siga2 >> HEMU_SIGA2_DOPPLER_SHIFT) & 0x1)

#define HEMU_SIGA2_NUM_LTF_SHIFT	8
#define HEMU_SIGA2_NUM_LTF_MASK		0x000000700
#define HEMU_SIGA2_NUM_LTF(siga2)	\
	((siga2 & HEMU_SIGA2_NUM_LTF_MASK) >> HEMU_SIGA2_NUM_LTF_SHIFT)
#define HEMU_SIGA_SIGB_COMPRESSION_SHIFT	22
#define HEMU_SIGA_SIGB_COMPRESSION(siga)	((siga >> HEMU_SIGA_SIGB_COMPRESSION_SHIFT) & 0x1)

#define HE_SIGA_20MHZ_VAL		0x00000000
#define HE_SIGA_40MHZ_VAL		0x00080000
#define HE_SIGA_80MHZ_VAL		0x00100000
#define HE_SIGA_160MHZ_VAL		0x00180000

#define HESU_SIGA_GILTF_MASK		0x00600000
#define HESU_SIGA_GILTF_SHIFT		21
#define HESU_SIGA_GILTF(siga)		((siga & HESU_SIGA_GILTF_MASK) >> HESU_SIGA_GILTF_SHIFT)
#define HEMU_SIGA_GILTF_MASK		0x01800000
#define HEMU_SIGA_GILTF_SHIFT		23
#define HEMU_SIGA_GILTF(siga)		((siga & HEMU_SIGA_GILTF_MASK) >> HEMU_SIGA_GILTF_SHIFT)

#define HE_SIGA_2x_LTF_GI_0_8us_VAL	0x00200000
#define HE_SIGA_2x_LTF_GI_1_6us_VAL	0x00400000
#define HE_SIGA_4x_LTF_GI_3_2us_VAL	0x00600000

/* PPE Threshold field (figure 9-589co) */
#define HE_PPE_THRESH_NSS_RU_FSZ	3
#define HE_PPE_THRESH_NSS_RU_MASK	0x7

/* PPE Threshold Info field (figure 9-589cp) */
/* ruc: RU Count; NSSnM1: NSSn - 1; RUmM1: RUm - 1 */
/* bit offset in PPE Threshold field */
#define HE_PPET16_BIT_OFFSET(ruc, NSSnM1, RUmM1) \
	(HE_NSSM1_LEN + HE_RU_INDEX_MASK_LEN + ((NSSnM1) * (ruc) + (RUmM1)) * 6)

#define HE_PPET8_BIT_OFFSET(ruc, NSSnM1, RUmM1) \
	(HE_NSSM1_LEN + HE_RU_INDEX_MASK_LEN + ((NSSnM1) * (ruc) + (RUmM1)) * 6 + 3)

/* Total PPE Threshold field byte length (Figure 9-589cq) */
#define HE_PPE_THRESH_LEN(nss, ruc) \
	(CEIL((HE_NSSM1_LEN + HE_RU_INDEX_MASK_LEN + ((nss) * (ruc) * 6)), 8))

/* RU Allocation Index encoding (table 9-262ad) */
#define HE_RU_ALLOC_IDX_242		0	/* RU alloc: 282 tones */
#define HE_RU_ALLOC_IDX_484		1	/* RU alloc: 484 tones - 40Mhz */
#define HE_RU_ALLOC_IDX_996		2	/* RU alloc: 996 tones - 80Mhz */
#define HE_RU_ALLOC_IDX_2x996		3	/* RU alloc: 2x996 tones - 80p80/160Mhz */

/* Constellation Index encoding (table 9-262ac) */
#define HE_CONST_IDX_BPSK		0
#define HE_CONST_IDX_QPSK		1
#define HE_CONST_IDX_16QAM		2
#define HE_CONST_IDX_64QAM		3
#define HE_CONST_IDX_256QAM		4
#define HE_CONST_IDX_1024QAM		5
#define HE_CONST_IDX_RSVD		6
#define HE_CONST_IDX_NONE		7

/**
 * HE Operation IE (IEEE Draft P802.11ax D3.0 9.4.2.238)
 */
#define HE_OP_PARAMS_SIZE		3
typedef uint8 he_op_parms_t[HE_OP_PARAMS_SIZE];

/* bit position and field width */
#define HE_OP_DEF_PE_DUR_IDX			0	/* Default PE Duration */
#define HE_OP_DEF_PE_DUR_FSZ			3
#define HE_OP_TWT_REQD_IDX			3	/* TWT Required */
#define HE_OP_TWT_REQD_FSZ			1
#define HE_OP_HE_DUR_RTS_THRESH_IDX		4	/* TXOP Duration RTS Threshold */
#define HE_OP_HE_DUR_RTS_THRESH_FSZ		10
#define HE_OP_VHT_OP_INFO_PRESENT_IDX		14	/* VHT Operation Information Present */
#define HE_OP_VHT_OP_INFO_PRESENT_FSZ		1
#define HE_OP_CO_LOCATED_BSS_IDX		15	/* Co-Located BSS */
#define HE_OP_CO_LOCATED_BSS_FSZ		1
#define HE_OP_ER_SU_DISABLE_IDX			16	/* ER SU Disable */
#define HE_OP_ER_SU_DISABLE_FSZ			1
#define HE_OP_6G_OP_IE_PRESENT_IDX		17	/* 6 GHz Operation Information Present */
#define HE_OP_6G_OP_IE_PRESENT_FSZ		1

/* BSS Color Information field (Figure 9-589cs) */
#define HE_OP_BSS_COLOR_IDX			0	/* BSS Color */
#define HE_OP_BSS_COLOR_FSZ			6
#define HE_OP_BSS_COLOR_PARTIAL_IDX		6	/* Partial BSS Color */
#define HE_OP_BSS_COLOR_PARTIAL_FSZ		1
#define HE_OP_BSS_COLOR_DISABLED_IDX		7	/* BSS Color Disabled */
#define HE_OP_BSS_COLOR_DISABLED_FSZ		1

typedef uint8 he_bss_color_info_t;

/* HE Operation element */
BWL_PRE_PACKED_STRUCT struct he_op_ie {
	uint8 id;
	uint8 len;
	uint8 id_ext;
	he_op_parms_t parms;
	he_bss_color_info_t color;
	uint16 basic_mcs_nss_set;
} BWL_POST_PACKED_STRUCT;

typedef struct he_op_ie he_op_ie_t;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

/* HE Basic trigger frame common info fields */
#define HE_TRIG_CMNINFO_SZ	8
typedef uint8 he_trig_cmninfo_set_t[HE_TRIG_CMNINFO_SZ];

/* bit position and field width */
#define HE_TRIG_CMNINFO_FRMTYPE_INDX		0	/* Trigger frame type */
#define HE_TRIG_CMNINFO_FRMTYPE_FSZ		4
#define HE_TRIG_CMNINFO_LSIGLEN_INDX		4	/* L-sig length */
#define HE_TRIG_CMNINFO_LSIGLEN_FSZ		12
#define HE_TRIG_CMNINFO_CASCADEIND_INDX		16	/* Cascade indication */
#define HE_TRIG_CMNINFO_CASCADEIND_FSZ		1
#define HE_TRIG_CMNINFO_CSREQ_INDX		17	/* Carrier sense indication */
#define HE_TRIG_CMNINFO_CSREQ_FSZ		1
#define HE_TRIG_CMNINFO_BWINFO_INDX		18	/* Bw info */
#define HE_TRIG_CMNINFO_BWINFO_FSZ		2
#define HE_TRIG_CMNINFO_GI_LTF_INDX		20	/* Cp-LTF size */
#define HE_TRIG_CMNINFO_GI_LTF_FSZ		2
#define HE_TRIG_CMNINFO_MUMIMO_LTF_INDX		22	/* HE-LTF mask enable */
#define HE_TRIG_CMNINFO_MUMIMO_LTF_FSZ		1
#define HE_TRIG_CMNINFO_HELTF_SYM_INDX		23	/* He-LTF sumbols */
#define HE_TRIG_CMNINFO_HELTF_SYM_FSZ		3
#define HE_TRIG_CMNINFO_STBC_INDX		26	/* STBC support */
#define HE_TRIG_CMNINFO_STBC_FSZ		1
#define HE_TRIG_CMNINFO_LDPC_EXTSYM_INDX	27	/* LDPC extra symbol */
#define HE_TRIG_CMNINFO_LDPC_EXTSYM_FSZ		1
#define HE_TRIG_CMNINFO_AP_TXPWR_INDX		28	/* AP TX power */
#define HE_TRIG_CMNINFO_AP_TXPWR_FSZ		6
#define HE_TRIG_CMNINFO_AFACT_INDX		34	/* a-factor */
#define HE_TRIG_CMNINFO_AFACT_FSZ		2
#define HE_TRIG_CMNINFO_PEDISAMBIG_INDX		36	/* PE disambiguity */
#define HE_TRIG_CMNINFO_PEDISAMBIG_FSZ		1
#define HE_TRIG_CMNINFO_SPTIAL_REUSE_INDX	37	/* spatial re-use */
#define HE_TRIG_CMNINFO_SPTIAL_REUSE_FSZ	16
#define HE_TRIG_CMNINFO_DOPPLER_INDX		53	/* doppler supoort */
#define HE_TRIG_CMNINFO_DOPPLER_FSZ		1
#define HE_TRIG_CMNINFO_HESIGA_RSVD_INDX	54	/* rsvd bits from HE-SIGA */
#define HE_TRIG_CMNINFO_HESIGA_RSVD_FSZ		9
#define HE_TRIG_CMNINFO_RSVD_INDX		63	/* reseved bit from HE-SIGA  */
#define HE_TRIG_CMNINFO_RSVD_FSZ		1

/* HE Basic trigger frame user info fields */
#define HE_TRIG_USRINFO_SZ	5
typedef uint8 he_trig_usrinfo_set_t[HE_TRIG_USRINFO_SZ];

/* HE Basic trigger frame user info with type-dependent per user byte */
#define HE_TRIG_USRINFO_TYPEDEP_SZ	(HE_TRIG_USRINFO_SZ + 1)
typedef uint8 he_trig_usrinfo_typedep_set_t[HE_TRIG_USRINFO_TYPEDEP_SZ];

/* bit position and field width */
#define HE_TRIG_USRINFO_AID_INDX		0	/* AID */
#define HE_TRIG_USRINFO_AID_FSZ			12
#define HE_TRIG_USRINFO_RU_ALLOC_INDX		12	/* RU allocation index */
#define HE_TRIG_USRINFO_RU_ALLOC_FSZ		8
#define HE_TRIG_USRINFO_CODING_INDX		20	/* coding type (BCC/LDPC) */
#define HE_TRIG_USRINFO_CODING_FSZ		1
#define HE_TRIG_USRINFO_MCS_INDX		21	/* MCS index value */
#define HE_TRIG_USRINFO_MCS_FSZ			4
#define HE_TRIG_USRINFO_DCM_INDX		25	/* Dual carrier modulation */
#define HE_TRIG_USRINFO_DCM_FSZ			1
#define HE_TRIG_USRINFO_SSALLOC_STRMOFFSET_INDX		26	/* stream offset */
#define HE_TRIG_USRINFO_SSALLOC_STRMOFFSET_FSZ		3
#define HE_TRIG_USRINFO_SSALLOC_NSS_INDX		29	/* number of spatial streams */
#define HE_TRIG_USRINFO_SSALLOC_NSS_FSZ		3
#define HE_TRIG_USRINFO_TARGET_RSSI_INDX	32	/* Target RSSI */
#define HE_TRIG_USRINFO_TARGET_RSSI_FSZ		7
#define HE_TRIG_USRINFO_RSVD_INDX		39	/* Reserved bit */
#define HE_TRIG_USRINFO_RSVD_FSZ		1
#define HE_TRIG_USRINFO_TYPEDEP_INDX		40	/* type-dep per user info byte */
#define HE_TRIG_USRINFO_TYPEDEP_FSZ		8

/* Different types of trigger frame */
#define HE_TRIG_TYPE_BASIC_FRM			0	/* basic trigger frame */
#define HE_TRIG_TYPE_BEAM_RPT_POLL_FRM		1	/* beamforming report poll frame */
#define HE_TRIG_TYPE_MU_BAR_FRM			2	/* MU-BAR frame */
#define HE_TRIG_TYPE_MU_RTS__FRM		3	/* MU-RTS frame */
#define HE_TRIG_TYPE_BSR_FRM			4	/* Buffer status report poll */

/* dot11 ax definitions for HETB frame */
#define DOT11_HETB_1XLTF_1U6S_GI	0
#define DOT11_HETB_2XLTF_1U6S_GI	1
#define DOT11_HETB_4XLTF_3U2S_GI	2
#define DOT11_HETB_RSVD_LTF_GI		3

#define DOT11_HETB_1XHELTF_NLTF		0
#define DOT11_HETB_2XHELTF_NLTF		1
#define DOT11_HETB_4XHELTF_NLTF		2
#define DOT11_HETB_6XHELTF_NLTF		3
#define DOT11_HETB_8XHELTF_NLTF		4
#define DOT11_HETB_RSVD_NLTF		5

/* HE Timing related parameters (802.11ax D0.5 Table: 26.9 */
#define HE_T_LEG_STF			8
#define HE_T_LEG_LTF			8
#define HE_T_LEG_LSIG			4
#define HE_T_RL_SIG			4
#define HE_T_SIGA			8
#define HE_T_STF			8
#define HE_T_LEG_PREAMBLE		(HE_T_LEG_STF + HE_T_LEG_LTF + HE_T_LEG_LSIG)
#define HE_T_LEG_SYMB			4
#define HE_RU_26_TONE			26
#define HE_RU_52_TONE			52
#define HE_RU_106_TONE			106
#define HE_RU_242_TONE			242
#define HE_RU_484_TONE			484
#define HE_RU_996_TONE			996
#define HE_RU_2x996_TONE		(2*996)
#define HE_MAX_26_TONE_RU_INDX		36
#define HE_MAX_52_TONE_RU_INDX		52
#define HE_MAX_106_TONE_RU_INDX		60
#define HE_MAX_242_TONE_RU_INDX		64
#define HE_MAX_484_TONE_RU_INDX		66
#define HE_MAX_996_TONE_RU_INDX		67
#define HE_MAX_2x996_TONE_RU_INDX	68

/**
 * ref: (802.11ax D2.1 Table 28-12 Page 391)
 *
 * - for calculation purpose - in multiples of 10 (*10)
 */
#define HE_T_LTF_1X			32
#define HE_T_LTF_2X			64
#define HE_T_LTF_4X			128
#define HE_T_SYM1			136	/* OFDM symbol duration with base GI */
#define HE_T_SYM2			144	/* OFDM symbol duration with double GI */
#define HE_T_SYM4			160	/* OFDM symbol duration with quad GI */

#define HE_N_LEG_SYM			3	/* bytes per legacy symbol */
#define HE_N_TAIL			6	/* tail field bits for BCC */
#define HE_N_SERVICE			16	/* bits in service field */
#define HE_T_MAX_PE			16	/* max Packet extension duration */

/* ADDBA Capability field */
#define ADDBA_CAP_SIZE	1
typedef uint8 addba_cap_t[ADDBA_CAP_SIZE];

/* bit position and field width */
#define ADDBA_CAP_NOFRAG_IDX		0	/* No-Fragment */
#define ADDBA_CAP_NOFRAG_FSZ		1
#define ADDBA_CAP_HE_FRAGOP_IDX		1	/* HE Fragment Operation */
#define ADDBA_CAP_HE_NOFRAG_FSZ		2

#endif /* _802_11ax_h_ */
