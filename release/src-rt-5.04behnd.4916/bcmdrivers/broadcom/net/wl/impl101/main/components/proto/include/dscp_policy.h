/*
 * Defines the WFA QoS (Quality of Service) Management vendor-specific frame attributes for the
 * DSCP (Differentiated Services Code Point) Policy which is used for the network-centric QoS.
 *
 * Please refer WFA QoS Mgmt spec:
 * https://drive.google.com/file/d/1ndJEqXsMsliy_B9A8ZME9uwwc5vsrlxR/view?usp=sharing
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
 * $Id: dscp_policy.h $
 */

#ifndef _DSCP_POLICY_H_
#define _DSCP_POLICY_H_

#include <typedefs.h>

#include <802.11.h>

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* QoS Mgmt vendor specific IE OUI type */
#define QOS_MGMT_VSIE_OUI_TYPE		0x22u

/* WFA Capabilities vendor specific IE OUI type */
#define WFA_CAP_VSIE_OUI_TYPE		0x23u

/* DSCP Policy Action Frame OUI type */
#define DSCP_POLICY_AF_OUI_TYPE		0x1Au

/* DSCP Policy Action Frame OUI subtypes */
#define DSCP_POLICY_QUERY_FRAME		0u
#define DSCP_POLICY_REQ_FRAME		1u
#define DSCP_POLICY_RESP_FRAME		2u

/* DSCP Policy request types */
#define POLICY_REQ_TYPE_ADD		0u
#define POLICY_REQ_TYPE_REMOVE		1u

/* DSCP Policy attribute field offsets */
#define DSCP_POLICY_ATTR_ID_OFF		0u
#define DSCP_POLICY_ATTR_LEN_OFF	1u
#define DSCP_POLICY_ATTR_DATA_OFF	2u

/* DSCP Policy attribute various length fields */
#define DSCP_POLICY_ATTR_ID_LEN		1u	/* attr id field length */
#define DSCP_POLICY_ATTR_LEN_LEN	1u	/* attr field length */
#define DSCP_POLICY_ATTR_HDR_LEN	2u	/* id + 1-byte length field */

/* DSCP Policy attributes as defined in the QoS Mgmt spec */
typedef enum qos_mgmt_attrs qos_mgmt_attrs_e;
enum qos_mgmt_attrs {

	/* DSCP Port Range attribute */
	DSCP_POLICY_PORT_RANGE_ATTR	= 1,

	/* DSCP Policy attribute */
	DSCP_POLICY_ATTR		= 2,

	/* DSCP Policy TCLAS attribute for classifier type 4 */
	DSCP_POLICY_TCLAS_ATTR		= 3,

	/* DSCP Domain Name attribute */
	DSCP_POLICY_DOMAIN_NAME_ATTR	= 4
};

/* DSCP Policy Query frame header */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_query_action_vs_frmhdr {
	uint8 category;		/* category VS/VSP */
	uint8 oui[WFA_OUI_LEN];	/* WFA OUI */
	uint8 oui_type;		/* DSCP_POLICY_AF_OUI_TYPE */
	uint8 oui_subtype;	/* DSCP Policy Query frame */
	uint8 dialog_token;	/* to match req/resp */
	uint8 data[];		/* zero or more QoS Mgmt elements */
} BWL_POST_PACKED_STRUCT dscp_policy_query_action_vs_frmhdr_t;
#define DSCP_POLICY_QUERY_ACTION_FRAME_HDR_SIZE (sizeof(dscp_policy_query_action_vs_frmhdr_t))

enum dscp_policy_req_control {
	DSCP_POLICY_CONTROL_MORE	= (1u << 0u),
	DSCP_POLICY_CONTROL_RESET	= (1u << 1u)
};
/* DSCP Policy request frame header */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_req_action_vs_frmhdr {
	uint8 category;		/* category VS/VSP */
	uint8 oui[WFA_OUI_LEN];	/* WFA OUI */
	uint8 oui_type;		/* DSCP_POLICY_AF_OUI_TYPE */
	uint8 oui_subtype;	/* DSCP Policy Request frame */
	uint8 dialog_token;	/* to match req/resp */
	uint8 control;		/* request control */
	uint8 data[];		/* zero or more QoS Mgmt elements */
} BWL_POST_PACKED_STRUCT dscp_policy_req_action_vs_frmhdr_t;
#define DSCP_POLICY_REQ_ACTION_FRAME_HDR_SIZE (sizeof(dscp_policy_req_action_vs_frmhdr_t))

/* DSCP Policy Response frame header */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_resp_action_vs_frmhdr {
	uint8 category;		/* category VS/VSP */
	uint8 oui[WFA_OUI_LEN];	/* WFA OUI */
	uint8 oui_type;		/* DSCP_POLICY_AF_OUI_TYPE */
	uint8 oui_subtype;	/* DSCP Policy Response frame type */
	uint8 dialog_token;	/* to validate req/resp frames */
	uint8 control;		/* response control */
	uint8 count;		/* Number of items in the data (Stauts List) */
	uint8 data[];		/* Status List */
} BWL_POST_PACKED_STRUCT dscp_policy_resp_action_vs_frmhdr_t;
#define DSCP_POLICY_RESP_ACTION_FRAME_HDR_SIZE (sizeof(dscp_policy_resp_action_vs_frmhdr_t))

/* WFA Capabilities IE */
typedef BWL_PRE_PACKED_STRUCT struct wfa_cap_ie {
	uint8 id;		/* 0xDD, IEEE 802.11 vendor specific information element */
	uint8 len;		/* length of data following */
	uint8 oui[WFA_OUI_LEN];	/* WFA OUI */
	uint8 oui_type;		/* WFA_CAP_VSIE_OUI_TYPE */
	uint8 capabilities_len;	/* WFA capabilities length */
	uint8 capabilities[];	/* WFA capability data + optional attributes */
} BWL_POST_PACKED_STRUCT wfa_cap_ie_t;
#define WFA_CAP_IE_HDR_SIZE (sizeof(wfa_cap_ie_t))

/* QoS Mgmt IE */
typedef BWL_PRE_PACKED_STRUCT struct qos_mgmt_ie {
	uint8 id;		/* 0xDD, IEEE 802.11 vendor specific information element */
	uint8 len;		/* length of data following */
	uint8 oui[WFA_OUI_LEN];	/* WFA OUI */
	uint8 oui_type;		/* QOS_MGMT_VSIE_OUI_TYPE */
	uint8 data[];		/* one or more DSCP policy attributes */
} BWL_POST_PACKED_STRUCT qos_mgmt_ie_t;
#define QOS_MGMT_IE_HDR_SIZE (sizeof(qos_mgmt_ie_t))

/* DSCP Policy capability attribute */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_capability_attr {
	uint8 id;		/* attribute id */
	uint8 len;		/* length of data */
	uint8 capabilities;	/* capabilities, 1 indicates DSCP Policy enabled */
} BWL_POST_PACKED_STRUCT dscp_policy_capability_attr_t;
#define DSCP_POLICY_CAPABILITY_ATTR_SIZE (sizeof(dscp_policy_capability_attr_t))

/* QoS Mgmt capability bits */
typedef enum qos_mgmt_cap_bits qos_mgmt_cap_bits_e;
enum qos_mgmt_cap_bits {

	/* When bit 0 is set, indicates the DSCP Policy support */
	QOS_MGMT_CAP_DSCP_POLICY			= (1u << 0u),

	/* When bit 1 is set, means that AP intends to send an unsolicited
	 * DSCP Policy Request frame to the STA imminently once association
	 * (and any security exchanges) is complete, or set to 0 to indicate it
	 * does not intend to do so.
	 */
	QOS_MGMT_CAP_UNSOLICIT_DSCP_POLICY_AT_ASSOC	= (1u << 1u),

	QOS_MGMT_CAP_SCS_TRAFFIC_DESCRIPTION		= (1u << 2u)
};

#define WFA_CAP_IE_DATA_LEN	1u	/* Current length (1 byte, 8 bits) for the QoS Mgmt
					 * capabilities field
					 */

/* DSCP Port Range attribute */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_port_range_attr {
	uint8 id;		/* attribute id */
	uint8 len;		/* length of data */
	uint16 start_port;	/* port range (both are in network byte order): start port */
	uint16 end_port;	/* end port */
} BWL_POST_PACKED_STRUCT dscp_policy_port_range_attr_t;
#define DSCP_POLICY_PORT_RANGE_ATTR_SIZE (sizeof(dscp_policy_port_range_attr_t))

/* DSCP Policy attribute */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_attr {
	uint8 id;		/* attribute id */
	uint8 len;		/* length of data */
	uint8 policy_id;	/* policy id: 1..255, 0 is reserved */
	uint8 req_type;		/* 0(Add), 1(Remove), 2..255 (Reserved) */
	uint8 dscp;		/* DSCP value associated with the policy */
} BWL_POST_PACKED_STRUCT dscp_policy_attr_t;
#define DSCP_POLICY_ATTR_SIZE (sizeof(dscp_policy_attr_t))

/* DSCP Policy TCLAS attribute */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_tclas_attr {
	uint8 id;		/* attribute id */
	uint8 len;		/* length of data */
	uint8 data[];		/* frame classifier type 4 data */
} BWL_POST_PACKED_STRUCT dscp_policy_tclas_attr_t;
#define DSCP_POLICY_TCLAS_ATTR_SIZE (sizeof(dscp_policy_tclas_attr_t))

/* DSCP Policy Domain Name attribute */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_domain_name_attr {
	uint8 id;		/* attribute id */
	uint8 len;		/* length of data */
	uint8 data[];		/* domain name */
} BWL_POST_PACKED_STRUCT dscp_policy_domain_name_attr_t;
#define DSCP_POLICY_DOMAIN_NAME_ATTR_SIZE (sizeof(dscp_policy_domain_name_attr_t))

/* Status tuple for the response */
typedef BWL_PRE_PACKED_STRUCT struct dscp_policy_status {
	uint8 policy_id;
	uint8 status;
} BWL_POST_PACKED_STRUCT dscp_policy_status_t;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* __DSCP_POLICY_H__ */
