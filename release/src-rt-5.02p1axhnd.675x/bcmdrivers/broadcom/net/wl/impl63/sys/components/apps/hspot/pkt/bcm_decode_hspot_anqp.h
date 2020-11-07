/*
 * Decode functions which provides decoding of Hotspot2.0 ANQP packets
 * as defined in Hotspot2.0 specification.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id:$
 */

#ifndef _BCM_DECODE_HSPOT_ANQP_H_
#define _BCM_DECODE_HSPOT_ANQP_H_

#include "802.11.h"
#include "typedefs.h"
#include "bcm_decode.h"
#include "bcm_hspot.h"

typedef struct {
	int queryListLength;
	uint8 *queryListBuffer;
	int capabilityListLength;
	uint8 *capabilityListBuffer;
	int operatorFriendlyNameLength;
	uint8 *operatorFriendlyNameBuffer;
	int wanMetricsLength;
	uint8 *wanMetricsBuffer;
	int connectionCapabilityLength;
	uint8 *connectionCapabilityBuffer;
	int naiHomeRealmQueryLength;
	uint8 *naiHomeRealmQueryBuffer;
	int opClassIndicationLength;
	uint8 *opClassIndicationBuffer;
	int onlineSignupProvidersLength;
	uint8 *onlineSignupProvidersBuffer;
	int anonymousNaiLength;
	uint8 *anonymousNaiBuffer;
	int iconRequestLength;
	uint8 *iconRequestBuffer;
	int iconBinaryFileLength;
	uint8 *iconBinaryFileBuffer;
} bcm_decode_hspot_anqp_t;

/* print decoded hotspot ANQP */
void bcm_decode_hspot_anqp_print(bcm_decode_hspot_anqp_t *hspot);

/* decode hotspot ANQP frame */
int bcm_decode_hspot_anqp(bcm_decode_t *pkt, int isReset, bcm_decode_hspot_anqp_t *hspot);

#define BCM_DECODE_ANQP_MAX_LIST_SIZE	16
typedef struct
{
	int isDecodeValid;
	uint16 queryLen;
	uint8 queryId[BCM_DECODE_ANQP_MAX_LIST_SIZE];
} bcm_decode_hspot_anqp_query_list_t;

/* decode query list */
int bcm_decode_hspot_anqp_query_list(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_query_list_t *queryList);

/* print decoded query list */
void bcm_decode_hspot_anqp_query_list_print(bcm_decode_hspot_anqp_query_list_t *queryList);

typedef struct
{
	int isDecodeValid;
	uint16 capLen;
	uint8 capId[BCM_DECODE_ANQP_MAX_LIST_SIZE];
} bcm_decode_hspot_anqp_capability_list_t;

/* decode capability list */
int bcm_decode_hspot_anqp_capability_list(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_capability_list_t *capList);

/* print decoded capability list */
void bcm_decode_hspot_anqp_capability_list_print(bcm_decode_hspot_anqp_capability_list_t *capList);

/* is capability supported */
int bcm_decode_hspot_anqp_is_capability(
	bcm_decode_hspot_anqp_capability_list_t *capList, uint8 capId);

typedef struct {
	uint8 langLen;
	char lang[VENUE_LANGUAGE_CODE_SIZE + 1];	/* null terminated */
	uint8 nameLen;
	char name[VENUE_NAME_SIZE + 1];		/* null terminated */
} bcm_decode_hspot_anqp_name_duple_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME	4
typedef struct {
	int isDecodeValid;
	int numName;
	bcm_decode_hspot_anqp_name_duple_t duple[BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME];
} bcm_decode_hspot_anqp_operator_friendly_name_t;

/* decode operator friendly name */
int bcm_decode_hspot_anqp_operator_friendly_name(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_operator_friendly_name_t *op);

/* print decoded operator friendly name */
void bcm_decode_hspot_anqp_operator_friendly_name_print(
	bcm_decode_hspot_anqp_operator_friendly_name_t *op);

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

/* decode WAN metrics */
int bcm_decode_hspot_anqp_wan_metrics(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_wan_metrics_t *wanMetrics);

/* print decoded WAN metrics */
void bcm_decode_hspot_anqp_wan_metrics_print(bcm_decode_hspot_anqp_wan_metrics_t *wanMetrics);

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

/* decode connection capability */
int bcm_decode_hspot_anqp_connection_capability(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_connection_capability_t *cap);

/* print decoded connection capability */
void bcm_decode_hspot_anqp_connection_capability_print(
	bcm_decode_hspot_anqp_connection_capability_t *cap);

/* is connection capability supported */
int bcm_decode_hspot_anqp_is_connection_capability(
	bcm_decode_hspot_anqp_connection_capability_t *cap,
	uint8 ipProtocol, uint16 portNumber);

typedef struct {
	uint8 encoding;
	uint8 nameLen;
	char name[VENUE_NAME_SIZE + 1];		/* null terminated */
} bcm_decode_hspot_anqp_nai_home_realm_data_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_HOME_REALM	16
typedef struct {
	int isDecodeValid;
	uint8 count;
	bcm_decode_hspot_anqp_nai_home_realm_data_t data[BCM_DECODE_HSPOT_ANQP_MAX_HOME_REALM];
} bcm_decode_hspot_anqp_nai_home_realm_query_t;

/* decode NAI home realm query */
int bcm_decode_hspot_anqp_nai_home_realm_query(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_nai_home_realm_query_t *realm);

/* print decoded home realm query */
void bcm_decode_hspot_anqp_nai_home_realm_query_print(
	bcm_decode_hspot_anqp_nai_home_realm_query_t *realm);

#define BCM_DECODE_HSPOT_ANQP_MAX_OPCLASS_LIST_SIZE	255
typedef struct
{
	int isDecodeValid;
	uint16 opClassLen;
	uint8 opClass[BCM_DECODE_HSPOT_ANQP_MAX_OPCLASS_LIST_SIZE];
} bcm_decode_hspot_anqp_operating_class_indication_t;

/* decode operating class indication */
int bcm_decode_hspot_anqp_operating_class_indication(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_operating_class_indication_t *opClassList);

/* print decoded operating class indication */
void bcm_decode_hspot_anqp_operating_class_indication_print(
	bcm_decode_hspot_anqp_operating_class_indication_t *opClassList);

#define BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH	128
#define BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH	128
typedef struct
{
	uint16 width;
	uint16 height;
	char lang[VENUE_LANGUAGE_CODE_SIZE + 1];	/* null terminated */
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
} bcm_decode_hspot_anqp_osu_provider_t;

#define BCM_DECODE_HSPOT_ANQP_MAX_OSU_SSID_LENGTH	255
#define BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER		16
typedef struct
{
	int isDecodeValid;
	uint8 osuSsidLength;
	uint8 osuSsid[BCM_DECODE_HSPOT_ANQP_MAX_OSU_SSID_LENGTH + 1];	/* null terminated */
	uint8 osuProviderCount;
	bcm_decode_hspot_anqp_osu_provider_t osuProvider[BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER];
} bcm_decode_hspot_anqp_osu_provider_list_t;

/* decode OSU provider list */
int bcm_decode_hspot_anqp_osu_provider_list(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_osu_provider_list_t *list);

/* print decoded OSU provider list */
void bcm_decode_hspot_anqp_osu_provider_list_print(bcm_decode_hspot_anqp_osu_provider_list_t *list);

/* search decoded OSU provider list for specified provider */
bcm_decode_hspot_anqp_osu_provider_t *bcm_decode_hspot_anqp_find_osu_provider(
	bcm_decode_hspot_anqp_osu_provider_list_t *list,
	int langLength, char *lang,
	int friendlyLength, char *friendly,
	int isOsuMethod, uint8 osuMethod,
	bcm_decode_hspot_anqp_name_duple_t **duple);

/* search decoded OSU provider list for specified SSID and provider */
bcm_decode_hspot_anqp_osu_provider_t *bcm_decode_hspot_anqp_find_osu_ssid_provider(
	bcm_decode_hspot_anqp_osu_provider_list_t *list,
	int osuSsidLength, char *osuSsid, int langLength, char *lang,
	int friendlyLength, char *friendly,
	int isOsuMethod, uint8 osuMethod,
	bcm_decode_hspot_anqp_name_duple_t **duple);

/* get the providers OSU method */
int bcm_decode_hspot_get_osu_method(bcm_decode_hspot_anqp_osu_provider_t *p);

#define BCM_DECODE_HSPOT_ANQP_MAX_NAI_SIZE	255
typedef struct {
	int isDecodeValid;
	uint16 naiLen;
	char nai[BCM_DECODE_HSPOT_ANQP_MAX_NAI_SIZE + 1];		/* null terminated */
} bcm_decode_hspot_anqp_anonymous_nai_t;

/* decode anonymous NAI */
int bcm_decode_hspot_anqp_anonymous_nai(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_anonymous_nai_t *anonymous);

/* print decoded anonymous NAI */
void bcm_decode_hspot_anqp_anonymous_nai_print(
	bcm_decode_hspot_anqp_anonymous_nai_t *anonymous);

typedef struct {
	int isDecodeValid;
	uint8 filenameLength;
	char filename[
		BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH + 1];	/* null terminated */
} bcm_decode_hspot_anqp_icon_request_t;

/* decode icon request */
int bcm_decode_hspot_anqp_icon_request(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_icon_request_t *request);

/* print icon request */
void bcm_decode_hspot_anqp_icon_request_print(
	bcm_decode_hspot_anqp_icon_request_t *request);

#define BCM_DECODE_HSPOT_ANQP_MAX_ICON_BINARY_SIZE	65535
typedef struct {
	int isDecodeValid;
	uint8 status;
	uint8 typeLength;
	uint8 type[BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH + 1];	/* null terminated */
	uint16 binaryLength;
	uint8 binary[BCM_DECODE_HSPOT_ANQP_MAX_ICON_BINARY_SIZE];
} bcm_decode_hspot_anqp_icon_binary_file_t;

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

/* decode icon binary file */
int bcm_decode_hspot_anqp_icon_binary_file(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_icon_binary_file_t *icon);

/* print decoded icon binary file */
void bcm_decode_hspot_anqp_icon_binary_file_print(
	bcm_decode_hspot_anqp_icon_binary_file_t *icon);

#endif /* _BCM_DECODE_HSPOT_ANQP_H_ */
