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
 * $Id: passpoint_nvparse.h 490618 2014-07-11 11:23:19Z $
 */

#ifndef _PASSPOINT_NVPARSE_H_
#define _PASSPOINT_NVPARSE_H_

/* --------------------------------- Constatnts & Macros  --------------------------------- */

/* Hspotap Flags ------------- Hex Codes ---- Bit --Initial Val - Usage --- */
#define HSFLG_OSUICON_ID	16	/* Bit 16 -  0  -	YES */
#define HSFLG_4FRAMEGAS		15	/* Bit 15 -  0  -	YES */
#define HSFLG_DS_ANQP_RESP	14	/* Bit 14 -  0  -	YES */
#define HSFLG_USE_SIM		13	/* Bit 13 -  0  -	YES */
#define HSFLG_ICMPV4_ECHO	12	/* Bit 12 -  1  -	YES */
#define HSFLG_L2_TRF		11	/* Bit 11 -  1  -	YES */
#define HSFLG_DGAF_DS		10	/* Bit 10 -  0  -	YES */
#define HSFLG_PROXY_ARP		9	/* Bit 09 -  1  -	YES */
#define HSFLG_P2P_CRS		8	/* Bit 08 -  0  -	YES */
#define HSFLG_P2P		7	/* Bit 07 -  1  -	YES */
#define HSFLG_MIH		6	/* Bit 06 -  0  -	YES */
#define HSFLG_ANQP		5	/* Bit 05 -  1  -	YES */
#define HSFLG_IWASRA_EN		4	/* Bit 04 -  0  -	YES */
#define HSFLG_IWINT_EN		3	/* Bit 03 -  0  -	YES */
#define HSFLG_U11_EN		2	/* Bit 02 -  0  -	YES */
#define HSFLG_OSEN		1	/* Bit 01 -  0  -	YES */
#define HSFLG_HS_EN		0	/* Bit 00 -  0  -	YES */

/* local buffer size */
#define BUFF_256			256
#define BUFF_512			512
#define BUFF_4K				4096
#define BUFF_PRINTGASEVENT		64
#define CODE_BUFF			20

#define REALM_INFO_LENGTH		1024
#define FRIENDLY_NAME_INFO_LENGTH	2048
#define NVRAM_MAX_VAL_LEN		2048

#define MAX_OSU_PROVIDERS		4

/* Auth Param Types */
#define AUTH_PARAM_TYPE_UNKNOWN		-1
#define AUTH_PARAM_TYPE_NONEAPINNER	1
#define AUTH_PARAM_TYPE_INNERAUTHEAP	2
#define AUTH_PARAM_TYPE_CREDENTIAL	3

/* Operating Indication Class */
#define OPCLS_2G			81
#define OPCLS_5G			115

/* General Strings */
#define DISABLED_S			"disabled"

/* Icon Paths */
#ifndef ICONPATH
#define ICONPATH			"/www/hspot"
#endif /* ICONPATH */

#define USERDEFINED_ICONPATH		"/tmp/confmtd/hspot/"

/* Valid Icon File Extensions & MIME Types */
#define EXT_BMP				".bmp"
#define EXT_DIB				".dib"
#define EXT_RLE				".rle"
#define EXT_JPG				".jpg"
#define EXT_JPEG			".jpeg"
#define EXT_JPE				".jpe"
#define EXT_JFIF			".jfif"
#define EXT_GIF				".gif"
#define EXT_EMF				".emf"
#define EXT_WMF				".wmf"
#define EXT_TIF				".tif"
#define EXT_TIFF			".tiff"
#define EXT_PNG				".png"
#define EXT_ICO				".ico"
#define MIME_BMP			"image/bmp"
#define MIME_JPEG			"image/jpeg"
#define MIME_GIF			"image/gif"
#define MIME_EMF			"image/emf"
#define MIME_WMF			"image/wmf"
#define MIME_TIFF			"image/tiff"
#define MIME_PNG			"image/png"
#define MIME_XICO			"image/x-icon"
#define MIME_UKWN			"image/unknown"

/* Language Codes */
#define KOREAN				"kor"
#define SPANISH				"spa"
#define LANG_ZXX			"zxx"
#define ENGLISH				"eng"
#define CHINESE				"chi"

/* Encoding Types */
#define ENC_RFC4282			"rfc4282"
#define ENC_UTF8			"utf8"

#define ENC_HELD			"held"
#define ENC_SUPL			"supl"

/* OSU Methods */
#define OMADM_S				"OMADM"
#define OMA_DM				"OMA-DM"
#define OMADM_NVVAL			"0"
#define SOAP_S 				"SOAP"
#define SOAP_XML			"SOAP-XML"
#define SOAP_NVVAL			"1"

/* Network Authentication List Types */
#define NATI_NA				"na"
#define NATI_ACCEPTTC			"accepttc"
#define NATI_ONLINE			"online"
#define NATI_HTTPRED			"httpred"
#define NATI_DNSRED			"dnsred"
/* --------------------------------- Constatnts & Macros  --------------------------------- */

/* ----------------------------------- NVRAM Strings ------------------------------------ */
/* Passpoint NVRAMS */
#define NVNM_HSFLAG			"hsflag"
#define NVNM_HS2CAP			"hs2cap"
#define NVNM_OPERCLS			"opercls"
#define NVNM_ANONAI			"anonai"
#define NVNM_WANMETRICS			"wanmetrics"
#define NVNM_OPLIST			"oplist"
#define NVNM_HOMEQLIST			"homeqlist"
#define NVNM_OSU_SSID			"osu_ssid"
#define NVNM_OSU_FRNDNAME		"osu_frndname"
#define NVNM_OSU_URI			"osu_uri"
#define NVNM_OSU_METHOD			"osu_method"
#define NVNM_OSU_NAI			"osu_nai"
#define NVNM_OSU_ICONS			"osu_icons"
#define NVNM_OSU_SERVDESC		"osu_servdesc"
#define NVNM_QOSMAPIE			"qosmapie"
#define NVNM_GASCBDEL			"gascbdel"
#define NVNM_CONCAPLIST			"concaplist"
#define NVNM_IWNETTYPE			"iwnettype"
#define NVNM_HESSID			"hessid"
#define NVNM_IPV4ADDR			"ipv4addr"
#define NVNM_IPV6ADDR			"ipv6addr"
#define NVNM_NETAUTHLIST		"netauthlist"
#define NVNM_VENUEGRP			"venuegrp"
#define NVNM_VENUETYPE			"venuetype"
#define NVNM_VENUELIST			"venuelist"
#define NVNM_OUILIST			"ouilist"
#define NVNM_3GPPLIST			"3gpplist"
#define NVNM_DOMAINLIST			"domainlist"
#define NVNM_REALMLIST			"realmlist"
#define NVNM_PUBIDURIFQDNLIST		"urifqdnlist"

/* General NVRAMS */
#define NVNM_RADIO			"radio"
#define NVNM_AKM			"akm"
#define NVVAL_WPA2			"wpa2"
#define NVNM_HS2_DEBUG_LEVEL		"hs2_debug_level"
#define NVFMT_IFNAME_PRIM		"wl%d_ifname"
#define NVFMT_IFNAME_SECO		"wl%d.%d_ifname"
#define NVFMT_WLIF_PRIM			"wl%d_"
#define NVFMT_WLIF_SECO			"wl%d.%d_"
/* ----------------------------------- NVRAM Strings ------------------------------------ */

/* -------------------------------- Structure Definitions ----------------------------------- */
/* Structure to hold nai realm data */
typedef struct {
	uint8 encoding;
	uint8 realmLen;
	uint8 realm[BCM_DECODE_ANQP_MAX_REALM_LENGTH + 1];	/* null terminated */
	uint8 eapCount;
	uint8 realmInfoLen;
	uint8 realmInfo[REALM_INFO_LENGTH + 1];			/* null terminated */
	uint8 eapInfo[BCM_DECODE_ANQP_MAX_REALM_LENGTH + 1];
	bcm_decode_anqp_eap_method_t eap[BCM_DECODE_ANQP_MAX_EAP_METHOD];
} bcm_decode_anqp_nai_realm_data_ex_t;

/* Structure to hold nai realm list */
typedef struct {
	int isDecodeValid;
	uint16 realmCount;
	bcm_decode_anqp_nai_realm_data_ex_t realm[BCM_DECODE_ANQP_MAX_REALM];
} bcm_decode_anqp_nai_realm_list_ex_t;

/* Structure to hold osu provider data */
typedef struct
{
	bcm_decode_hspot_anqp_operator_friendly_name_t name;
	uint8 uriLength;
	uint8 uri[BCM_DECODE_HSPOT_ANQP_MAX_URI_LENGTH + 1];	/* null terminated */
	uint8 methodLength;
	uint8 method[BCM_DECODE_HSPOT_ANQP_MAX_METHOD_LENGTH];
	int iconMetadataCount;
	bcm_decode_hspot_anqp_icon_metadata_t iconMetadata[
		BCM_DECODE_HSPOT_ANQP_MAX_ICON_METADATA_LENGTH];
	uint8 naiLength;
	uint8 nai[BCM_DECODE_HSPOT_ANQP_MAX_NAI_LENGTH + 1];	/* null terminated */
	bcm_decode_hspot_anqp_operator_friendly_name_t desc;
	char nameInfo[FRIENDLY_NAME_INFO_LENGTH + 1];
	char descInfo[FRIENDLY_NAME_INFO_LENGTH + 1];
	char iconInfo[FRIENDLY_NAME_INFO_LENGTH + 1];
} bcm_decode_hspot_anqp_osu_provider_ex_t;

/* Structure to hold osu provider list */
typedef struct
{
	int isDecodeValid;
	int osuicon_id;
	uint8 osuSsidLength;
	uint8 osuSsid[BCM_DECODE_HSPOT_ANQP_MAX_OSU_SSID_LENGTH + 1];	/* null terminated */
	uint8 osuProviderCount;
	bcm_decode_hspot_anqp_osu_provider_ex_t
		osuProvider[BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER];
} bcm_decode_hspot_anqp_osu_provider_list_ex_t;

/* Structure to hold oui data */
typedef struct
{
	int isBeacon;
	uint8 oiLen;
	uint8 oi[BCM_DECODE_ANQP_MAX_OI_LENGTH];
} bcm_decode_anqp_oi_duple_ex_t;

/* Structure to hold oui list */
typedef struct
{
	int isDecodeValid;
	int numOi;
	bcm_decode_anqp_oi_duple_ex_t oi[BCM_DECODE_ANQP_MAX_OI];
} bcm_decode_anqp_roaming_consortium_ex_t;
/* -------------------------------- Structure Definitions ----------------------------------- */

/* ------------------------------ Passpoint Utility Routines --------------------------------- */
extern const char* idtostr_eapmethod(uint8 iEap_method);

extern const char* idtostr_authid(uint8 iAuth_id, int *authparamtype);

extern const char* idtostr_authparam(int authparamtype, int iAuth_param);

extern int get_icon_geometry(const char *filename, uint16 *width, uint16 *height);

extern int get_icon_mimetype(const char *filename, char *mime_type, int size);

extern int get_icon_metadata(const char* path, const char* filename,
	bcm_decode_hspot_anqp_icon_metadata_t *metadata);

extern int get_hspot_flag(const char *prefix, unsigned int bit);

extern int set_hspot_flag(const char *prefix, unsigned int bit, int value);
/* ------------------------------ Passpoint Utility Routines --------------------------------- */

/* ----------------------------- Passpoint Decode Routines --------------------------------- */
extern int decode_iw_ie(const char* prefix,
	bcm_decode_interworking_t* iwIe, char* err_nvram);

extern int decode_qosmap_ie(const char* prefix,
	bcm_decode_qos_map_t* qosMapSetIe);

extern int decode_u11_ipaddr_typeavail(const char* prefix,
	bcm_decode_anqp_ip_type_t* ipaddrAvail, char* err_nvram);

extern int decode_u11_netauth_list(const char* prefix,
	bcm_decode_anqp_network_authentication_type_t* netauthlist);

extern int decode_u11_realm_list(const char* prefix,
	bcm_decode_anqp_nai_realm_list_ex_t* realmlist);

extern int decode_u11_venue_list(const char* prefix,
	bcm_decode_anqp_venue_name_t* venuelist, char* err_nvram);

extern int decode_u11_oui_list(const char* prefix,
	bcm_decode_anqp_roaming_consortium_ex_t* ouilist);

extern int decode_u11_3gpp_list(const char* prefix,
	bcm_decode_anqp_3gpp_cellular_network_t* gpp3list);

extern int decode_u11_domain_list(const char* prefix,
	bcm_decode_anqp_domain_name_list_t* domainlist);

extern int decode_hspot_oper_class(const char* prefix,
	bcm_decode_hspot_anqp_operating_class_indication_t* opclass);

extern int decode_hspot_anonai(const char* prefix,
	bcm_decode_hspot_anqp_anonymous_nai_t* anonai);

extern int decode_hspot_wan_metrics(const char* prefix,
	bcm_decode_hspot_anqp_wan_metrics_t* wanmetrics);

extern int decode_hspot_op_list(const char* prefix,
	bcm_decode_hspot_anqp_operator_friendly_name_t* oplist);

extern int decode_hspot_homeq_list(const char* prefix,
	bcm_decode_hspot_anqp_nai_home_realm_query_t* homeqlist);

extern int decode_hspot_conncap_list(const char* prefix,
	bcm_decode_hspot_anqp_connection_capability_t* concaplist);

extern int decode_hspot_osup_list(const char* prefix,
	bcm_decode_hspot_anqp_osu_provider_list_ex_t* osuplist, char* err_nvram);

extern int decode_u11_urifqdn_list(const char* prefix,
	bcm_decode_hspot_anqp_location_uri_fqdn_query_t* urifqdnlist);
/* ----------------------------- Passpoint Decode Routines --------------------------------- */

#endif /* _PASSPOINT_NVPARSE_H_ */
