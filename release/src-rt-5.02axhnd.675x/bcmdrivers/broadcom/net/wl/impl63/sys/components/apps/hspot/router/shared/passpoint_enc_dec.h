/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Passpoint NVRAM Parsing functions
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
 * $Id: passpoint_enc_dec.h 490618 2014-07-11 11:23:19Z $
 */

#ifndef _PASSPOINT_ENC_DEC_H_
#define _PASSPOINT_ENC_DEC_H_

#include "802.11.h"
#include "typedefs.h"

/* -------------------------------------- bcm_hspot.h -------------------------------------- */
/* Passpoint IE OUI type */
#define HSPOT_IE_OUI_TYPE				0x10

/* Passpoint ANQP OUI type */
#define HSPOT_ANQP_OUI_TYPE				0x11
#define HSPOT_ANQP_OUI					"\x50\x6F\x9A\x11"

/* WNM type */
#define HSPOT_WNM_TYPE					1

/* Passpoint WNM type */
#define HSPOT_WNM_SUBSCRIPTION_REMEDIATION		0x00
#define HSPOT_WNM_DEAUTHENTICATION_IMMINENT		0x01

/* Passpoint config - Downstream Group Addressed Forward */
#define HSPOT_IE_DGAF_DISABLED				0x01

/* Passpoint config release2 */
#define HSPOT_DGAF_DISABLED_SHIFT			0
#define HSPOT_DGAF_DISABLED_MASK			(0x01 << HSPOT_DGAF_DISABLED_SHIFT)
#define HSPOT_PPS_MO_ID_SHIFT				1
#define HSPOT_PPS_MO_ID_MASK				(0x01 << HSPOT_PPS_MO_ID_SHIFT)
#define HSPOT_ANQP_DOMAIN_ID_SHIFT			2
#define HSPOT_ANQP_DOMAIN_ID_MASK			(0x01 << HSPOT_ANQP_DOMAIN_ID_SHIFT)

#define HSPOT_RELEASE_SHIFT				4
#define HSPOT_RELEASE_MASK				(0x0f << HSPOT_RELEASE_SHIFT)

/* Passpoint release numbers */
#define HSPOT_RELEASE_1					0
#define HSPOT_RELEASE_2					1

/* length includes OUI + type + subtype + reserved */
#define HSPOT_LENGTH_OVERHEAD				(WFA_OUI_LEN + 1 + 1 + 1)

/* subtype */
#define HSPOT_SUBTYPE_RESERVED				0
#define HSPOT_SUBTYPE_QUERY_LIST			1
#define HSPOT_SUBTYPE_CAPABILITY_LIST			2
#define HSPOT_SUBTYPE_OPERATOR_FRIENDLY_NAME		3
#define HSPOT_SUBTYPE_WAN_METRICS			4
#define HSPOT_SUBTYPE_CONNECTION_CAPABILITY		5
#define HSPOT_SUBTYPE_NAI_HOME_REALM_QUERY		6
#define HSPOT_SUBTYPE_OPERATING_CLASS_INDICATION	7
#define HSPOT_SUBTYPE_ONLINE_SIGNUP_PROVIDERS		8
#define HSPOT_SUBTYPE_ANONYMOUS_NAI			9
#define HSPOT_SUBTYPE_ICON_REQUEST			10
#define HSPOT_SUBTYPE_ICON_BINARY_FILE			11

/* WAN info - link status */
#define HSPOT_WAN_LINK_STATUS_SHIFT			0
#define HSPOT_WAN_LINK_STATUS_MASK			(0x03 << HSPOT_WAN_LINK_STATUS_SHIFT)
#define	HSPOT_WAN_LINK_UP				0x01
#define HSPOT_WAN_LINK_DOWN				0x02
#define HSPOT_WAN_LINK_TEST				0x03

/* WAN info - symmetric link */
#define HSPOT_WAN_SYMMETRIC_LINK_SHIFT			2
#define HSPOT_WAN_SYMMETRIC_LINK_MASK			(0x01 << HSPOT_WAN_SYMMETRIC_LINK_SHIFT)
#define HSPOT_WAN_SYMMETRIC_LINK			0x01
#define HSPOT_WAN_NOT_SYMMETRIC_LINK			0x00

/* WAN info - at capacity */
#define HSPOT_WAN_AT_CAPACITY_SHIFT			3
#define HSPOT_WAN_AT_CAPACITY_MASK			(0x01 << HSPOT_WAN_AT_CAPACITY_SHIFT)
#define HSPOT_WAN_AT_CAPACITY				0x01
#define HSPOT_WAN_NOT_AT_CAPACITY			0x00

/* IP Protocols for Connection Capability */
#define HSPOT_CC_IPPROTO_NONE				-1
#define HSPOT_CC_IPPROTO_ICMP				1
#define HSPOT_CC_IPPROTO_TCP				6
#define HSPOT_CC_IPPROTO_UDP				17
#define HSPOT_CC_IPPROTO_ESP				50

/* Port Numbers for Connection Capability */
#define HSPOT_CC_PORT_NONE				-1
#define HSPOT_CC_PORT_RESERVED				0
#define HSPOT_CC_PORT_FTP				20
#define HSPOT_CC_PORT_SSH				22
#define HSPOT_CC_PORT_HTTP				80
#define HSPOT_CC_PORT_HTTPS				443
#define HSPOT_CC_PORT_ISAKMP				500
#define HSPOT_CC_PORT_PPTP				1723
#define HSPOT_CC_PORT_IPSEC				4500
#define HSPOT_CC_PORT_SIP				5060

/* Port Status for Connection Capability */
#define HSPOT_CC_STATUS_NONE				-1
#define HSPOT_CC_STATUS_CLOSED				0
#define HSPOT_CC_STATUS_OPEN				1
#define HSPOT_CC_STATUS_UNKNOWN				2

/* OSU method */
#define HSPOT_OSU_METHOD_OMA_DM				0
#define HSPOT_OSU_METHOD_SOAP_XML			1

/* icon download status */
#define HSPOT_ICON_STATUS_SUCCESS			0
#define HSPOT_ICON_STATUS_FILE_NOT_FOUND		1
#define HSPOT_ICON_STATUS_UNSPECIFIED_FILE_ERROR	2

/* deauthentication reason */
#define HSPOT_DEAUTH_RC_BSS_DISALLOW			0
#define HSPOT_DEAUTH_RC_ESS_DISALLOW			1
/* -------------------------------------- bcm_hspot.h -------------------------------------- */

/* ---------------------------------- bcm_decode_anqp.h  ----------------------------------- */
typedef struct
{
	uint8 langLen;
	char lang[VENUE_LANGUAGE_CODE_SIZE + 1];			/* null terminated */
	uint8 nameLen;
	char name[VENUE_NAME_SIZE + 1];					/* null terminated */
} bcm_decode_anqp_venue_name_duple_t;

#define BCM_DECODE_ANQP_MAX_VENUE_NAME			4
typedef struct
{
	int isDecodeValid;
	uint8 group;
	uint8 type;
	int numVenueName;
	bcm_decode_anqp_venue_name_duple_t venueName[BCM_DECODE_ANQP_MAX_VENUE_NAME];
} bcm_decode_anqp_venue_name_t;

#define BCM_DECODE_ANQP_MAX_URL_LENGTH			128
typedef struct
{
	uint8 type;
	uint16 urlLen;
	uint8 url[BCM_DECODE_ANQP_MAX_URL_LENGTH + 1];			/* null terminated */
} bcm_decode_anqp_network_authentication_unit_t;

#define BCM_DECODE_ANQP_MAX_AUTHENTICATION_UNIT		8
typedef struct
{
	int isDecodeValid;
	int numAuthenticationType;
	bcm_decode_anqp_network_authentication_unit_t unit[BCM_DECODE_ANQP_MAX_AUTHENTICATION_UNIT];
} bcm_decode_anqp_network_authentication_type_t;

#define BCM_DECODE_ANQP_MAX_OI_LENGTH			8
typedef struct
{
	uint8 oiLen;
	uint8 oi[BCM_DECODE_ANQP_MAX_OI_LENGTH];
} bcm_decode_anqp_oi_duple_t;

#define BCM_DECODE_ANQP_MAX_OI				16
typedef struct
{
	int isDecodeValid;
	int numOi;
	bcm_decode_anqp_oi_duple_t oi[BCM_DECODE_ANQP_MAX_OI];
} bcm_decode_anqp_roaming_consortium_t;

typedef struct
{
	int isDecodeValid;
	uint8 ipv6;
	uint8 ipv4;
} bcm_decode_anqp_ip_type_t;

#define BCM_DECODE_ANQP_MAX_AUTH_PARAM			16
typedef struct
{
	uint8 id;
	uint8 len;
	uint8 value[BCM_DECODE_ANQP_MAX_AUTH_PARAM];
} bcm_decode_anqp_auth_t;

#define BCM_DECODE_ANQP_MAX_AUTH			4
typedef struct
{
	uint8 eapMethod;
	uint8 authCount;
	bcm_decode_anqp_auth_t auth[BCM_DECODE_ANQP_MAX_AUTH];
} bcm_decode_anqp_eap_method_t;

#define BCM_DECODE_ANQP_MAX_REALM_LENGTH		255
#define BCM_DECODE_ANQP_MAX_EAP_METHOD			4
#define BCM_DECODE_ANQP_MAX_REALM			16

#define BCM_DECODE_ANQP_MCC_LENGTH			3
#define BCM_DECODE_ANQP_MNC_LENGTH			3
typedef struct
{
	char mcc[BCM_DECODE_ANQP_MCC_LENGTH + 1];
	char mnc[BCM_DECODE_ANQP_MNC_LENGTH + 1];
} bcm_decode_anqp_plmn_t;

#define BCM_DECODE_ANQP_MAX_PLMN			16
typedef struct
{
	int isDecodeValid;
	uint8 plmnCount;
	bcm_decode_anqp_plmn_t plmn[BCM_DECODE_ANQP_MAX_PLMN];
} bcm_decode_anqp_3gpp_cellular_network_t;

#define BCM_DECODE_ANQP_MAX_DOMAIN_NAME_SIZE 		128
typedef struct
{
	uint8 len;
	char name[BCM_DECODE_ANQP_MAX_DOMAIN_NAME_SIZE + 1];		/* null terminated */
} bcm_decode_anqp_domain_name_t;

#define BCM_DECODE_ANQP_MAX_DOMAIN 			16
typedef struct
{
	int isDecodeValid;
	int numDomain;
	bcm_decode_anqp_domain_name_t domain[BCM_DECODE_ANQP_MAX_DOMAIN];
} bcm_decode_anqp_domain_name_list_t;
/* ---------------------------------- bcm_decode_anqp.h  ----------------------------------- */

/* -------------------------------- bcm_decode_hspot_anqp.h  -------------------------------- */
typedef struct {
	uint8 langLen;
	char lang[VENUE_LANGUAGE_CODE_SIZE + 1];			/* null terminated */
	uint8 nameLen;
	char name[VENUE_NAME_SIZE + 1];					/* null terminated */
} bcm_decode_hspot_anqp_name_duple_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME		4
typedef struct {
	int isDecodeValid;
	int numName;
	bcm_decode_hspot_anqp_name_duple_t duple[BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME];
} bcm_decode_hspot_anqp_operator_friendly_name_t;

typedef struct {
	int isDecodeValid;
	uint8 linkStatus;
	uint8 symmetricLink;
	uint8 atCapacity;
	uint32 dlinkSpeed;
	uint32 ulinkSpeed;
	uint8 dlinkLoad;
	uint8 ulinkLoad;
	uint16 lmd;
	uint32 dlinkAvailable;
	uint32 ulinkAvailable;
} bcm_decode_hspot_anqp_wan_metrics_t;

typedef struct {
	uint8 ipProtocol;
	uint16 portNumber;
	uint8 status;
} bcm_decode_hspot_anqp_proto_port_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_CONNECTION_CAPABILITY	16
typedef struct {
	int isDecodeValid;
	int numConnectCap;
	bcm_decode_hspot_anqp_proto_port_t tuple[BCM_DECODE_HSPOT_ANQP_MAX_CONNECTION_CAPABILITY];
} bcm_decode_hspot_anqp_connection_capability_t;

typedef struct {
	uint8 encoding;
	uint8 nameLen;
	char name[VENUE_NAME_SIZE + 1];					/* null terminated */
} bcm_decode_hspot_anqp_nai_home_realm_data_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_HOME_REALM		16
typedef struct {
	int isDecodeValid;
	uint8 count;
	bcm_decode_hspot_anqp_nai_home_realm_data_t data[BCM_DECODE_HSPOT_ANQP_MAX_HOME_REALM];
} bcm_decode_hspot_anqp_nai_home_realm_query_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_OPCLASS_LIST_SIZE	255
typedef struct
{
	int isDecodeValid;
	uint16 opClassLen;
	uint8 opClass[BCM_DECODE_HSPOT_ANQP_MAX_OPCLASS_LIST_SIZE];
} bcm_decode_hspot_anqp_operating_class_indication_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH	128
#define BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH	128
typedef struct
{
	uint16 width;
	uint16 height;
	char lang[VENUE_LANGUAGE_CODE_SIZE + 1];			/* null terminated */
	uint8 typeLength;
	uint8 type[BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH + 1];	/* null terminated */
	uint8 filenameLength;
	char filename[
		BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH + 1];	/* null terminated */
} bcm_decode_hspot_anqp_icon_metadata_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_NAI_LENGTH		128
#define BCM_DECODE_HSPOT_ANQP_MAX_METHOD_LENGTH		2
#define BCM_DECODE_HSPOT_ANQP_MAX_ICON_METADATA_LENGTH	8
#define BCM_DECODE_HSPOT_ANQP_MAX_URI_LENGTH		128
#define BCM_DECODE_HSPOT_ANQP_MAX_OSU_SSID_LENGTH	255
#define BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER		16

#define BCM_DECODE_HSPOT_ANQP_MAX_NAI_SIZE		255
typedef struct {
	int isDecodeValid;
	uint16 naiLen;
	char nai[BCM_DECODE_HSPOT_ANQP_MAX_NAI_SIZE + 1];		/* null terminated */
} bcm_decode_hspot_anqp_anonymous_nai_t;

typedef struct {
	uint8 encoding;
	uint8 nameLen;
	char name[URI_FQDN_SIZE + 1];					/* null terminated */
} bcm_decode_hspot_anqp_location_uri_fqdn_data_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_URI_FQDN		16
typedef struct {
	int isDecodeValid;
	uint8 count;
	bcm_decode_hspot_anqp_location_uri_fqdn_data_t data[BCM_DECODE_HSPOT_ANQP_MAX_URI_FQDN];
} bcm_decode_hspot_anqp_location_uri_fqdn_query_t;
/* -------------------------------- bcm_decode_hspot_anqp.h  -------------------------------- */

/* ------------------------------------ bcm_decode_ie.h  ------------------------------------ */
typedef struct
{
	uint8 accessNetworkType;
	int isInternet;
	int isAsra;
	int isEsr;
	int isUesa;
	int isVenue;
	uint8 venueGroup;
	uint8 venueType;
	int isHessid;
	struct ether_addr hessid;
} bcm_decode_interworking_t;
/* ------------------------------------ bcm_decode_ie.h  ------------------------------------ */

/* ------------------------------------ bcm_decode_qos.h  ----------------------------------- */
#define BCM_DECODE_QOS_MAP_MAX_EXCEPT_LENGTH		128
#define BCM_DECODE_QOS_MAP_MAX_UP			8
typedef struct
{
	uint8 exceptCount;
	struct {
		uint8 dscp;
		uint8 up;
	} except[BCM_DECODE_QOS_MAP_MAX_EXCEPT_LENGTH];
	struct {
		uint8 low;
		uint8 high;
	} up[BCM_DECODE_QOS_MAP_MAX_UP];
} bcm_decode_qos_map_t;
/* ------------------------------------ bcm_decode_qos.h  ----------------------------------- */

#endif /* _PASSPOINT_ENC_DEC_H_ */
