/*
 * Fundamental types and constants relating to FILS AUTHENTICATION
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
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
 * $Id$
 */

#ifndef _FILSAUTH_H_
#define _FILSAUTH_H_

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* 11ai D6.0 8.6.8.36 FILS Discovery frame format
	category
	action
	fils_discovery_info_field_t
	fils_rnr_element_t
	fils_indication_element_t
	fils_vendor_specific_element_t
*/

/* 11revmc D4.0 8.4.2.25 Vendor Specific element */
typedef BWL_PRE_PACKED_STRUCT struct fils_vendor_specific_element {
	uint8		elementid;
	uint8		length;
	/* variable len info */
	uint8		orgid_vendorspecific_content[];
} BWL_POST_PACKED_STRUCT fils_vendor_specific_element_t;

#define FILS_VS_ELEM_HDR_LEN	(sizeof(fils_vendor_specific_element_t))

/* 11ai D6.0 8.4.2.178 FILS Indication element */
typedef BWL_PRE_PACKED_STRUCT struct fils_indication_element {
	uint8		elementid;
	uint8		length;
	uint16		fils_info;
	/* variable len info */
	uint8		cache_domain_publickey_id[];
} BWL_POST_PACKED_STRUCT fils_indication_element_t;

#define FILS_INDICATION_ELEM_HDR_LEN	(sizeof(fils_indication_element_t))

#define FILS_INDICATION_IE_TAG_FIXED_LEN		2

#define FI_INFO_CACHE_IND_SUBFIELD_SIZE			2

/* FILS Indication Information field */
#define FI_INFO_PUB_KEY_IDENTS_MASK		(0x0007)
#define FI_INFO_REALM_IDENTS_MASK		(0x0038)
#define FI_INFO_IP_ADDR_CFG_MASK		(0x0040)
#define FI_INFO_CACHE_IDENT_MASK		(0x0080)
#define FI_INFO_HESSID_MASK			(0x0100)
#define FI_INFO_SHRKEY_AUTH_WOPFS_MASK		(0x0200)
#define FI_INFO_SHRKEY_AUTH_WPFS_MASK		(0x0400)
#define FI_INFO_PUBKEY_AUTH_MASK		(0x0800)

#define FI_INFO_CACHE_IDENT(fc)			((fc & FI_INFO_CACHE_IDENT_MASK))
#define FI_INFO_HESSID(fc)			((fc & FI_INFO_HESSID_MASK))
#define FI_INFO_SHRKEY_AUTH_WOPFS(fc)		((fc & FI_INFO_SHRKEY_AUTH_WOPFS_MASK))
#define FI_INFO_SHRKEY_AUTH_WPFS(fc)		((fc & FI_INFO_SHRKEY_AUTH_WPFS_MASK))

#define TBTT_INFO_BSS_PARAMS_OCT_RECOMMENDED		0X01
#define TBTT_INFO_BSS_PARAMS_SAME_SSID			0X02
#define TBTT_INFO_BSS_PARAMS_MBSSID			0X04
#define TBTT_INFO_BSS_PARAMS_TRANSMITTED_BSSID		0X08
#define TBTT_INFO_BSS_PARAMS_ESS_MEMBER_WITH_2G_5G	0X10
#define TBTT_INFO_BSS_PARAMS_UPR_ACTIVE			0X20
#define TBTT_INFO_BSS_PARAMS_COLOCATED_AP		0X40
#define TBTT_INFO_BSS_RESERVED				0X80

/* 11ai D11.0 9.4.2.171.1 TBTT Information field */
typedef BWL_PRE_PACKED_STRUCT struct tbtt_info_field {
	uint8		tbtt_offset;
	uint8		bssid[ETHER_ADDR_LEN];
	uint32		short_ssid;
	uint8		bss_params;
	int8            tpe_psd;
} BWL_POST_PACKED_STRUCT tbtt_info_field_t;

#define TBTT_INFO_FIELD_HDR_LEN	(sizeof(tbtt_info_field_t))
#define MAX_TBTT_INFO_FIELDS	16u

/* 11ai D11.0 9.4.2.171.1 Neighbor AP Information field */
typedef BWL_PRE_PACKED_STRUCT struct neighbor_ap_info_field {
	uint16		tbtt_info_header;
	uint8		op_class;
	uint8		channel;
	/* variable len info */
	uint8		tbtt_info_field[];
} BWL_POST_PACKED_STRUCT neighbor_ap_info_field_t;

#define NEIGHBOR_AP_INFO_FIELD_HDR_LEN	(sizeof(neighbor_ap_info_field_t))

/* 11ai D11.0 9.4.2.171 Reduced Neighbor Report element */
typedef BWL_PRE_PACKED_STRUCT struct fils_rnr_element {
	uint8		elementid;
	uint8		length;
	/* variable len info */
	uint8		neighbor_ap_info[];
} BWL_POST_PACKED_STRUCT fils_rnr_element_t;

#define FILS_RNR_ELEM_HDR_LEN	(sizeof(fils_rnr_element_t))

/* TBTT Info Header macros */
#define TBTT_INFO_HDR_FIELD_TYPE_MASK		(0x001f)
#define TBTT_INFO_HDR_FN_AP_MASK		(0x0004)
#define TBTT_INFO_HDR_FN_AP_SHIFT		2
#define TBTT_INFO_HDR_COUNT_MASK		(0x00f0)
#define TBTT_INFO_HDR_COUNT_SHIFT		4
#define TBTT_INFO_HDR_LENGTH_MASK		(0xff00)
#define TBTT_INFO_HDR_LENGTH_SHIFT		8
#define TBTT_INFO_HDR_COLOCATED_AP_MASK		0X0008
#define TBTT_INFO_HDR_COLOCATED_AP_SHIFT	3

#define TBTT_INFO_HDR_FIELD_TYPE(hdr)\
	((hdr) & TBTT_INFO_HDR_FIELD_TYPE_MASK)
#define TBTT_INFO_HDR_FN_AP(hdr)\
	(((hdr) & TBTT_INFO_HDR_FN_AP_MASK) >> TBTT_INFO_HDR_FN_AP_SHIFT)
#define TBTT_INFO_HDR_COUNT(hdr)\
	(((hdr) & TBTT_INFO_HDR_COUNT_MASK) >> TBTT_INFO_HDR_COUNT_SHIFT)
#define TBTT_INFO_HDR_LENGTH(hdr)\
	(((hdr) & TBTT_INFO_HDR_LENGTH_MASK) >> TBTT_INFO_HDR_LENGTH_SHIFT)

#define TBTT_INFO_HDR_SET_LENGTH(hdr, len) ((hdr) |= ((len) << TBTT_INFO_HDR_LENGTH_SHIFT &\
	TBTT_INFO_HDR_LENGTH_MASK))
#define TBTT_INFO_HDR_SET_COUNT(hdr, cnt) ((hdr) |= ((cnt) << TBTT_INFO_HDR_COUNT_SHIFT &\
	TBTT_INFO_HDR_COUNT_MASK))
#define TBTT_INFO_HDR_SET_FN_AP(hdr, ap) ((hdr) |= ((ap) << TBTT_INFO_HDR_FN_AP_SHIFT &\
	TBTT_INFO_HDR_FN_AP_MASK))
#define TBTT_INFO_HDR_SET_COLOCATED_AP(hdr, ap)	\
			((hdr) |= ((ap) << TBTT_INFO_HDR_COLOCATED_AP_SHIFT &\
			TBTT_INFO_HDR_COLOCATED_AP_MASK))

/* TBTT Info field macros */
#define TBTT_INFO_FIELD_BSS_PARAMS_OCT_RECOMMENDED_SHIFT		0
#define TBTT_INFO_FIELD_BSS_PARAMS_SAME_SSID_SHIFT			1
#define TBTT_INFO_FIELD_BSS_PARAMS_MBSSID_SHIFT				2
#define TBTT_INFO_FIELD_BSS_PARAMS_TRANSMIT_BSSID_SHIFT			3
#define TBTT_INFO_FIELD_BSS_PARAMS_ESS_MEMBER_SHIFT			4
#define TBTT_INFO_FIELD_BSS_PARAMS_20TU_PRB_RSP_ENABLED_SHIFT		5

#define TBTT_INFO_FIELD_SET_OCT_RECOMMEND(params)	\
		((params) |= 1 << TBTT_INFO_FIELD_BSS_PARAMS_OCT_RECOMMENDED_SHIFT)

#define TBTT_IS_INFO_FIELD_OCT_RECOMMEND(params)	\
		((params) & (1 << TBTT_INFO_FIELD_BSS_PARAMS_OCT_RECOMMENDED_SHIFT))

#define TBTT_INFO_FIELD_SET_SAME_SSID(params)	\
		((params) |= 1 << TBTT_INFO_FIELD_BSS_PARAMS_SAME_SSID_SHIFT)

#define TBTT_IS_INFO_FIELD_SAME_SSID(params)	\
		((params) & (1 << TBTT_INFO_FIELD_BSS_PARAMS_SAME_SSID_SHIFT))

#define TBTT_INFO_FIELD_SET_MBSSID(params)	\
		((params) |= 1 << TBTT_INFO_FIELD_BSS_PARAMS_MBSSID_SHIFT)

#define TBTT_IS_INFO_FIELD_MBSSID(params)	\
		((params) & (1 << TBTT_INFO_FIELD_BSS_PARAMS_MBSSID_SHIFT))

#define TBTT_INFO_FIELD_SET_TRANSMIT_BSSID(params)	\
		((params) |= 1 << TBTT_INFO_FIELD_BSS_PARAMS_TRANSMIT_BSSID_SHIFT)

#define TBTT_IS_INFO_FIELD_TRANSMIT_BSSID(params)	\
		((params) & (1 << TBTT_INFO_FIELD_BSS_PARAMS_TRANSMIT_BSSID_SHIFT))

#define TBTT_INFO_FIELD_SET_ESS_MEMBER(params)	\
		((params) |= 1 << TBTT_INFO_FIELD_BSS_PARAMS_ESS_MEMBER_SHIFT)

#define TBTT_IS_INFO_FIELD_ESS_MEMBER(params)	\
		((params) & (1 << TBTT_INFO_FIELD_BSS_PARAMS_ESS_MEMBER_SHIFT))

#define TBTT_INFO_FIELD_SET_20TU_PRB_RSP(params)	\
		((params) |= 1 << TBTT_INFO_FIELD_BSS_PARAMS_20TU_PRB_RSP_ENABLED_SHIFT)

#define TBTT_IS_INFO_FIELD_20TU_PRB_RSP_ENABLED(params)	\
		((params) &  (1 << TBTT_INFO_FIELD_BSS_PARAMS_20TU_PRB_RSP_ENABLED_SHIFT))

/* FILS Nonce element */
#define FILS_NONCE_LENGTH 16u

typedef BWL_PRE_PACKED_STRUCT struct fils_nonce_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	uint8		fils_nonce[FILS_NONCE_LENGTH];
} BWL_POST_PACKED_STRUCT fils_nonce_element_t;

/* 8.4.2.175 FILS Session element */
#define FILS_SESSION_LENGTH 8u

typedef BWL_PRE_PACKED_STRUCT struct fils_session_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	uint8		fils_session[FILS_SESSION_LENGTH];
} BWL_POST_PACKED_STRUCT fils_session_element_t;

/* 9.4.2.179 FILS key confirmation element */
#define FILS_KEY_CONFIRMATION_HEADER_LEN 3u

typedef BWL_PRE_PACKED_STRUCT struct fils_key_conf_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	/* variable len info */
	uint8		key_auth[];
} BWL_POST_PACKED_STRUCT fils_key_conf_element_t;

#define FILS_SESSION_ELEM_LEN	(sizeof(fils_session_element_t))

/* 8.4.2.174 FILS Key Confirmation element */
typedef BWL_PRE_PACKED_STRUCT struct fils_key_confirm_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	/* variable len info */
	uint8		keyauth[];
} BWL_POST_PACKED_STRUCT fils_key_confirm_element_t;

#define FILS_CONFIRM_ELEM_HDR_LEN	(sizeof(fils_key_confirm_element_t))

/* 11ai D6.0 8.6.8.36 FILS Discovery frame format */
typedef BWL_PRE_PACKED_STRUCT struct fils_discovery_info_field {
	uint16		framecontrol;
	uint32		timestamp[2];
	uint16		bcninterval;
	/* variable len info */
	uint8		disc_info[];
} BWL_POST_PACKED_STRUCT fils_discovery_info_field_t;

#define FD_INFO_FIELD_HDR_LEN	(sizeof(fils_discovery_info_field_t))

#define FD_INFO_CAP_SUBFIELD_SIZE			2
#define FD_INFO_LENGTH_FIELD_SIZE			1

/* FILS Discovery Information field */
#define FD_INFO_SSID_LENGTH_MASK			(0x001f)
#define FD_INFO_CAP_IND_MASK				(0x0020)
#define FD_INFO_SHORT_SSID_IND_MASK			(0x0040)
#define FD_INFO_APCSN_IND_MASK				(0x0080)
#define FD_INFO_ANO_IND_MASK				(0x0100)
#define FD_INFO_CH_CENTER_FR_IND_MASK			(0x0200)
#define FD_INFO_PRIMARY_CH_IND_MASK			(0x0400)
#define FD_INFO_RSN_IND_MASK				(0x0800)
#define FD_INFO_LENGTH_IND_MASK				(0x1000)
#define FD_INFO_MD_IND_MASK				(0x2000)
#define FD_INFO_11B_AP_NBR_IND_MASK			(0X4000)
#define FD_INFO_NON_OCE_AP_IND_MASK			(0x8000)

#define FD_INFO_SET_SSID_LENGTH(fc, len)	(fc |= ((uint16)(len) & FD_INFO_SSID_LENGTH_MASK))
#define FD_INFO_SET_CAP_PRESENT(fc)			(fc |= FD_INFO_CAP_IND_MASK)
#define FD_INFO_SET_SHORT_SSID_PRESENT(fc)		(fc |= FD_INFO_SHORT_SSID_IND_MASK)
#define FD_INFO_SET_APCSN_PRESENT(fc)			((fc |= FD_INFO_APCSN_IND_MASK)
#define FD_INFO_SET_ANO_PRESENT(fc)			(fc |= FD_INFO_ANO_IND_MASK)
#define FD_INFO_SET_CH_CENTER_FR_PRESENT(fc)		(fc |= FD_INFO_CH_CENTER_FR_IND_MASK)
#define FD_INFO_SET_PRIMARY_CH_PRESENT(fc)		(fc |= FD_INFO_PRIMARY_CH_IND_MASK)
#define FD_INFO_SET_RSN_PRESENT(fc)			(fc |= FD_INFO_RSN_IND_MASK)
#define FD_INFO_SET_LENGTH_PRESENT(fc)			(fc |= FD_INFO_LENGTH_IND_MASK)
#define FD_INFO_SET_MD_PRESENT(fc)			(fc |= FD_INFO_MD_IND_MASK)
#define FD_INFO_SET_11B_AP_NBR_PRESENT(fc)		(fc |= FD_INFO_11B_AP_NBR_IND_MASK)
#define FD_INFO_SET_NON_OCE_AP_PRESENT(fc)		(fc |= FD_INFO_NON_OCE_AP_IND_MASK)

#define FD_INFO_SSID_LENGTH(fc)				((fc & FD_INFO_SSID_LENGTH_MASK))
#define FD_INFO_IS_CAP_PRESENT(fc)			((fc & FD_INFO_CAP_IND_MASK) >> 5)
#define FD_INFO_IS_SHORT_SSID_PRESENT(fc)		((fc & FD_INFO_SHORT_SSID_IND_MASK) >> 6)
#define FD_INFO_IS_APCSN_PRESENT(fc)			((fc & FD_INFO_APCSN_IND_MASK) >> 7)
#define FD_INFO_IS_ANO_PRESENT(fc)			((fc & FD_INFO_ANO_IND_MASK) >> 8)
#define FD_INFO_IS_CH_CENTER_FR_PRESENT(fc)		((fc & FD_INFO_CH_CENTER_FR_IND_MASK) >> 9)
#define FD_INFO_IS_PRIMARY_CH_PRESENT(fc)		((fc & FD_INFO_PRIMARY_CH_IND_MASK) >> 10)
#define FD_INFO_IS_RSN_PRESENT(fc)			((fc & FD_INFO_RSN_IND_MASK) >> 11)
#define FD_INFO_IS_LENGTH_PRESENT(fc)			((fc & FD_INFO_LENGTH_IND_MASK) >> 12)
#define FD_INFO_IS_MD_PRESENT(fc)			((fc & FD_INFO_MD_IND_MASK) >> 13)
#define FD_INFO_IS_11B_NBR_AP_PRESENT(fc)		((fc & FD_INFO_11B_AP_NBR_IND_MASK) >> 14)
#define FD_INFO_IS_NON_OCE_AP_PRESENT(fc)		((fc & FD_INFO_NON_OCE_AP_IND_MASK) >> 15)

#define FD_CAP_ESS_BIT_SHIFT				0
#define FD_CAP_PRIVACY_SHIFT				1
#define FD_CAP_BSS_CH_WIDTH_BIT_SHIFT			2
#define FD_CAP_MAX_NSS_BIT_SHIFT			5
#define FD_CAP_PHY_INDEX_BIT_SHIFT			10
#define FD_CAP_FILS_MIN_RATE_BIT_SHIFT			13

#define FD_CAP_BSS_CH_WIDTH_20_22_MHZ			0
#define FD_CAP_BSS_CH_WIDTH_40_MHZ			1
#define FD_CAP_BSS_CH_WIDTH_80_MHZ			2
#define FD_CAP_BSS_CH_WIDTH_160_80p80_MHZ		3
#define FD_CAP_BSS_CH_WIDTH_RESERVED			4

#define FD_CAP_NSS_VAL_0				0
#define FD_CAP_NSS_VAL_1				1
#define FD_CAP_NSS_VAL_2				2
#define FD_CAP_NSS_VAL_3				3
#define FD_CAP_NSS_VAL_4				4
#define FD_CAP_NSS_RESERVED				5

#define FD_CAP_PHY_INDEX_HR_DSSS			0
#define FD_CAP_PHY_INDEX_ERP_OFDM			1
#define FD_CAP_PHY_INDEX_HT				2
#define FD_CAP_PHY_INDEX_VHT				3
#define FD_CAP_PHY_INDEX_HE				4
#define FD_CAP_PHY_INDEX_RESERVED			5

#define FD_CAP_FILS_MIN_RATE_0				0
#define FD_CAP_FILS_MIN_RATE_1				1
#define FD_CAP_FILS_MIN_RATE_2				2
#define FD_CAP_FILS_MIN_RATE_3				3
#define FD_CAP_FILS_MIN_RATE_4				4
#define FD_CAP_FILS_MIN_RATE_RESERVED			5

/* FILS Discovery Capability subfield */
#define FD_CAP_ESS_MASK					(0x0001)
#define FD_CAP_PRIVACY_MASK				(0x0002)
#define FD_CAP_BSS_CH_WIDTH_MASK			(0x001c)
#define FD_CAP_MAX_NSS_MASK				(0x00e0)
#define FD_CAP_MULTI_BSS_MASK				(0x0200)
#define FD_CAP_PHY_INDEX_MASK				(0x1c00)
#define FD_CAP_FILS_MIN_RATE_MASK			(0xe000)

#define FD_CAP_ESS(cap)					((cap & FD_CAP_ESS_MASK))
#define FD_CAP_PRIVACY(cap)				((cap & FD_CAP_PRIVACY_MASK) >> 1)
#define FD_CAP_BSS_CH_WIDTH(cap)			((cap & FD_CAP_BSS_CH_WIDTH_MASK) >> 2)
#define FD_CAP_MAX_NSS(cap)				((cap & FD_CAP_MAX_NSS_MASK) >> 5)
#define FD_CAP_MULTI_BSS(cap)				((cap & FD_CAP_MULTI_BSS_MASK) >> 9)
#define FD_CAP_PHY_INDEX(cap)				((cap & FD_CAP_PHY_INDEX_MASK) >> 10)
#define FD_CAP_FILS_MIN_RATE(cap)			((cap & FD_CAP_FILS_MIN_RATE_MASK) >> 13)

#define FD_CAP_SET_ESS(cap)				((cap |= FD_CAP_ESS_MASK))
#define FD_CAP_SET_PRIVACY(cap)				((cap & FD_CAP_PRIVACY_MASK) >> 1)
#define FD_CAP_SET_BSS_CH_WIDTH(cap)			((cap & FD_CAP_BSS_CH_WIDTH_MASK) >> 2)
#define FD_CAP_SET_MAX_NSS(cap)				((cap & FD_CAP_MAX_NSS_MASK) >> 5)
#define FD_CAP_SET_MULTI_BSS(cap)			((cap & FD_CAP_MULTI_BSS_MASK) >> 9)
#define FD_CAP_SET_PHY_INDEX(cap)			((cap & FD_CAP_PHY_INDEX_MASK) >> 10)
#define FD_CAP_SET_FILS_MIN_RATE(cap)			((cap & FD_CAP_FILS_MIN_RATE_MASK) >> 13)

#define FILS_RQST_PARAMETERS_FILS_CRITERIA_BIT		0X01
#define FILS_RQST_PARAMETERS_MAX_DELAY_LIMIT_BIT	0X02
#define FILS_RQST_PARAMETERS_MIN_DATA_RATE_BIT		0X04
#define FILS_RQST_PARAMETERS_RCPI_LIMIT_BIT		0X08
#define FILS_RQST_PARAMETERS_OUI_RESPONSE_BIT		0X10

/* IEEE Std 802.11ai-2016 FILS Request Paramters element */
typedef BWL_PRE_PACKED_STRUCT struct fils_request_parameters_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	uint8		params_bitmap;
	uint8		max_chan_time;
	uint8		fils_criteria;
	/* variable len info */
	uint8		params_fields[];
} BWL_POST_PACKED_STRUCT fils_request_parameters_element_t;

#define FILS_PARAM_MAX_CHANNEL_TIME		(1 << 2)
#define FILS_PARAM_MIN_FIXED_LEN		4
#define FILS_PARAM_MAX_PARAMS_FIELDS_LEN	7
#define FILS_RQST_PARAM_MAX_LEN	(FILS_PARAM_MIN_FIXED_LEN + FILS_PARAM_MAX_PARAMS_FIELDS_LEN)

/* 11ai 9.4.2.184 FILS HLP Container element */
typedef BWL_PRE_PACKED_STRUCT struct fils_hlp_container_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	uint8		dest_addr[ETHER_ADDR_LEN];
	uint8		src_addr[ETHER_ADDR_LEN];
	/* variable len hlp packet */
	uint8		hlp[];
} BWL_POST_PACKED_STRUCT fils_hlp_container_element_t;

/* 11ai 9.4.2.184 FILS Wrapped Data element */
typedef BWL_PRE_PACKED_STRUCT struct fils_wrapped_data_element {
	uint8		elementid;
	uint8		length;
	uint8		element_id_ext;
	/* variable len wrapped data packet */
	uint8		wrapped_data[];
} BWL_POST_PACKED_STRUCT fils_wrapped_data_element_t;

#define FILS_HLP_CONTAINER_ELEM_LEN	(sizeof(fils_hlp_container_element_t))

typedef struct ether_addr tbtt_bssid_t;
/* As per D5.0 in 802.11ax Table 9 281 TBTT Information field contents . */

typedef BWL_PRE_PACKED_STRUCT union rnr_tbtt_info_field {
	BWL_PRE_PACKED_STRUCT struct len2 {
		uint8	tbtt_offset;
		uint8	bss_params;
	} BWL_POST_PACKED_STRUCT len2_t;

	BWL_PRE_PACKED_STRUCT struct len5 {
		uint8	tbtt_offset;
		uint32	short_ssid;
	} BWL_POST_PACKED_STRUCT len5_t;

	BWL_PRE_PACKED_STRUCT struct len6 {
		uint8	tbtt_offset;
		uint32	short_ssid;
		uint8	bss_params;
	} BWL_POST_PACKED_STRUCT len6_t;

	BWL_PRE_PACKED_STRUCT struct len7 {
		uint8		tbtt_offset;
		tbtt_bssid_t	bssid;
	} BWL_POST_PACKED_STRUCT len7_t;

	BWL_PRE_PACKED_STRUCT struct len8 {
		uint8		tbtt_offset;
		tbtt_bssid_t	bssid;
		uint8		bss_params;
	} BWL_POST_PACKED_STRUCT len8_t;

	BWL_PRE_PACKED_STRUCT struct len11 {
		uint8		tbtt_offset;
		tbtt_bssid_t	bssid;
		uint32		short_ssid;
	} BWL_POST_PACKED_STRUCT len11_t;

	BWL_PRE_PACKED_STRUCT struct len12 {
		uint8		tbtt_offset;
		tbtt_bssid_t	bssid;
		uint32		short_ssid;
		uint8		bss_params;
	} BWL_POST_PACKED_STRUCT len12_t;
} BWL_POST_PACKED_STRUCT rnr_tbtt_info_field_t;
/* BSS Params Macros */
#define RNR_BSS_PARAMS_OCT_REC_MASK		(0x01u)
#define RNR_BSS_PARAMS_SAME_SSID_MASK		(0x02u)
#define RNR_BSS_PARAMS_MUTIPLE_BSSID_MASK	(0x04u)
#define RNR_BSS_PARAMS_TRANSMITTED_BSSID_MASK	(0x08u)
#define	RNR_BSS_MEMBER_OF_ESS_MASK		(0x10u)
#define	RNR_BSS_20_TU_PRB_RSP_ACTIVE_MASK	(0x20u)
#define	RNR_BSS_COLOCATED_AP_MASK		(0x40u)

#define RNR_BSS_PARAMS_OCT_REC(bss)\
	(((bss) & RNR_BSS_PARAMS_OCT_REC_MASK) != 0)
#define RNR_BSS_PARAMS_SAME_SSID(bss)\
	(((bss) & RNR_BSS_PARAMS_SAME_SSID_MASK) != 0)
#define RNR_BSS_PARAMS_MUTIPLE_BSSID(bss)\
	(((bss) & RNR_BSS_PARAMS_MUTIPLE_BSSID_MASK) != 0)
#define RNR_BSS_PARAMS_TRANSMITTED_BSSID(bss)\
	(((bss) & RNR_BSS_PARAMS_TRANSMITTED_BSSID_MASK) != 0)
#define RNR_BSS_MEMBER_OF_ESS(bss)\
	(((bss) & RNR_BSS_MEMBER_OF_ESS_MASK) != 0)
#define RNR_BSS_20_TU_PRB_RSP_ACTIVE(bss)\
	(((bss) & RNR_BSS_20_TU_PRB_RSP_ACTIVE_MASK) != 0)
#define RNR_BSS_COLOCATED_AP(bss)\
	(((bss) & RNR_BSS_COLOCATED_AP_MASK) != 0)
/* TBTT Information field Contents */
/* NBR_AP TBTT OFFSET field ( 1 Byte) */
#define NBR_AP_TBTT_LEN				1U

/* NBR_AP TBTT OFFSETfield(1) + BSSPARAMS(1) 2Bytes */
#define NBR_AP_TBTT_BSS_LEN			2U

/* NBR_AP TBTT OFFSETfield(1) + SHORTSSID (4) 5 Bytes */
#define NBR_AP_TBTT_SHORT_SSID_LEN		5U

/* NBR_AP TBTT OFFSETfield(1)+SHORTSSID (4)+BSS(1)  6 Bytes */
#define NBR_AP_TBTT_BSS_SHORT_SSID_LEN		6U

/* NBR_AP TBTT OFFSETfield(1) + BSSID(6) 7BYTES */
#define NBR_AP_TBTT_BSSID_LEN			7U

/* NBR_AP TBTT OFFSETfield(1) + BSSID(6)+BSS(1) 8BYTES */
#define NBR_AP_TBTT_BSSID_BSS_LEN		8U

/* NBR_AP TBTT OFFSETfield(1) + BSSID(6)+SHORTSSID (4) 11Bytes */
#define NBR_AP_TBTT_BSSID_SHORT_SSID_LEN	11U

/*  NBR_AP TBTT OFFSETfield(1) + BSSID(6)+SHORTSSID (4)+BSS(1) 12 BYTES */
#define NBR_AP_TBTT_BSSID_SHORT_SSID_BSS_LEN	12U

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* __FILSAUTH_H__ */
