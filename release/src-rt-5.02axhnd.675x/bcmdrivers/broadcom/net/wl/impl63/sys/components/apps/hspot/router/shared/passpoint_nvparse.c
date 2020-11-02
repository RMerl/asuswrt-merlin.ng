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
 * $Id: passpoint_nvparse.c 490618 2014-07-11 11:23:19Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <shutils.h>

#include "802.11.h"
#include "common_utils.h"
#include "passpoint_enc_dec.h"
#include "passpoint_nvparse.h"

/* ========================= UTILITY FUNCTIONS ============================= */
static bool
strToEther(char *str, struct ether_addr *bssid)
{
	int hex[ETHER_ADDR_LEN] = {0};
	int i;

	if (sscanf(str, MAC_FORMAT,
		&hex[0], &hex[1], &hex[2], &hex[3], &hex[4], &hex[5]) != 6)
		return FALSE;

	for (i = 0; i < ETHER_ADDR_LEN; i++)
		bssid->octet[i] = hex[i];

	return TRUE;
}

const char*
idtostr_eapmethod(uint8 iEap_method)
{
	switch (iEap_method)
	{
	case REALM_EAP_TLS:
		return "EAP-TLS";

	case REALM_EAP_LEAP:
		return "EAP-LEAP";

	case REALM_EAP_SIM:
		return "EAP-SIM";

	case REALM_EAP_TTLS:
		return "EAP-TTLS";

	case REALM_EAP_AKA:
		return "EAP-AKA";

	case REALM_EAP_PEAP:
		return "EAP-PEAP";

	case REALM_EAP_FAST:
		return "EAP-FAST";

	case REALM_EAP_PSK:
		return "EAP-PSK";

	case REALM_EAP_AKAP:
		return "EAP-AKAP";

	case REALM_EAP_EXPANDED:
		return "EAP-EXPANDED";

	default :
		break;
	}

	return EMPTY_STR;
}

const char*
idtostr_authid(uint8 iAuth_id, int *authparamtype)
{
	switch (iAuth_id)
	{
	case REALM_NON_EAP_INNER_AUTHENTICATION:
		*authparamtype = AUTH_PARAM_TYPE_NONEAPINNER;
		return "NonEAPInner";

	case REALM_INNER_AUTHENTICATION_EAP:
		*authparamtype = AUTH_PARAM_TYPE_INNERAUTHEAP;
		return "InnerAuthEAP";

	case REALM_CREDENTIAL:
		*authparamtype = AUTH_PARAM_TYPE_CREDENTIAL;
		return "Credential";

	case REALM_TUNNELED_EAP_CREDENTIAL:
		*authparamtype = AUTH_PARAM_TYPE_CREDENTIAL;
		return "TunnledEAPCred";

	default :
		break;
	}

	*authparamtype = AUTH_PARAM_TYPE_UNKNOWN;
	return EMPTY_STR;
}

const char*
idtostr_authparam(int authparamtype, int iAuth_param)
{
	if (AUTH_PARAM_TYPE_UNKNOWN == authparamtype)
		return EMPTY_STR;

	switch (authparamtype) {

	case AUTH_PARAM_TYPE_NONEAPINNER :
		switch (iAuth_param) {
			case REALM_MSCHAPV2:
				return "MSCHAPV2";

			case REALM_MSCHAP:
				return "MSCHAP";

			case REALM_CHAP:
				return "CHAP";

			case REALM_PAP:
				return "PAP";

			default :
				break;
		}
		break;

	case AUTH_PARAM_TYPE_INNERAUTHEAP :
		switch (iAuth_param) {
			case REALM_EAP_LEAP:
				return "LEAP";

			case REALM_EAP_PEAP:
				return "PEAP";

			case REALM_EAP_TLS:
				return "EAP-TLS";

			case REALM_EAP_FAST:
				return "EAP-FAST";

			case REALM_EAP_SIM:
				return "EAP-SIM";

			case REALM_EAP_TTLS:
				return "EAP-TTLS";

			case REALM_EAP_AKA:
				return "EAP-AKA";

			default :
				break;
		}
		break;

	case AUTH_PARAM_TYPE_CREDENTIAL :
		switch (iAuth_param) {
			case REALM_SIM:
				return "SIM";

			case REALM_USIM:
				return "USIM";

			case REALM_NFC:
				return "NFC";

			case REALM_HARDWARE_TOKEN:
				return "HARDTOKEN";

			case REALM_SOFTOKEN:
				return "SOFTTOKEN";

			case REALM_CERTIFICATE:
				return "CERTIFICATE";

			case REALM_USERNAME_PASSWORD:
				return "USERNAME_PASSWORD";

			case REALM_SERVER_SIDE:
				return "RESERVED";

			case REALM_RESERVED_CRED:
				return "ANNONYMOUS";

			case REALM_VENDOR_SPECIFIC_CRED:
				return "VENDOR_SPECIFIC";

			default :
				break;
		}
		break;

	default :
		break;

	}
	return EMPTY_STR;
}

int
get_icon_geometry(const char *filename, uint16 *width, uint16 *height)
{
	unsigned char buf[24] = {0};
	int ret = 0;
	long len = 0;
	FILE *f = fopen(filename, "rb");

	if (f == 0) {
		ret = -1;
		return ret;
	}

	if (fseek(f, 0, SEEK_END) == -1) {
		ret = -1;
		goto GETIMAGESIZE_DONE;
	}

	len = ftell(f);

	if (fseek(f, 0, SEEK_SET) == -1) {
		ret = -1;
		goto GETIMAGESIZE_DONE;
	}

	if (len < 24) {
		ret = -1;
		goto GETIMAGESIZE_DONE;
	}

	/*
	Strategy:
	reading GIF dimensions requires the first 10 bytes of the file
	reading PNG dimensions requires the first 24 bytes of the file
	reading JPEG dimensions requires scanning through jpeg chunks
	In all formats, the file is at least 24 bytes big, so we'll read that always
	*/

	if (fread(buf, 1, 24, f) < 24) {
		ret = -1;
		goto GETIMAGESIZE_DONE;
	}

	/* For JPEGs, we need to read the first 12 bytes of each chunk.
	 We'll read those 12 bytes at buf+2...buf+14, i.e. overwriting the existing buf.
	*/

	if ((buf[0] == 0xFF) && (buf[1] == 0xD8) &&
		(buf[2] == 0xFF) && (buf[3] == 0xE0) &&
		(buf[6] == 'J') && (buf[7] == 'F') &&
		(buf[8] == 'I') && (buf[9] == 'F')) {
		long pos = 2;
		while (buf[2] == 0xFF) {
			if ((buf[3] == 0xC0) || (buf[3] == 0xC1) ||
				(buf[3] == 0xC2) || (buf[3] == 0xC3) ||
				(buf[3] == 0xC9) || (buf[3] == 0xCA) ||
				(buf[3] == 0xCB))
					break;
			pos += 2 + (buf[4]<<8) + buf[5];

			if (pos+12 > len)
				break;

			if (fseek(f, pos, SEEK_SET) == -1) {
				ret = -1;
				goto GETIMAGESIZE_DONE;
			}

			if (fread(buf+2, 1, 12, f) < 12) {
				ret = -1;
				goto GETIMAGESIZE_DONE;
			}
		}
	}
	/*
	JPEG:first two bytes of buf are
	first two bytes of the jpeg file;
	rest of buf is the DCT frame
	*/

	if ((buf[0] == 0xFF) &&
		(buf[1] == 0xD8) && (buf[2] == 0xFF)) {
		*height = (buf[7] << 8) + buf[8];
		*width = (buf[9] << 8) + buf[10];
		ret = 0;
		goto GETIMAGESIZE_DONE;
	}

	/* GIF: first three bytes say "GIF", next three give version number. Then dimensions */
	if ((buf[0] == 'G') &&
		(buf[1] == 'I') &&
		(buf[2] == 'F')) {
		*width = buf[6] + (buf[7] << 8);
		*height = buf[8] + (buf[9] << 8);
		ret = 0;
		goto GETIMAGESIZE_DONE;
	}

	/* PNG: the first frame is by definition an IHDR frame, which gives dimensions */

	if ((buf[0] == 0x89) && (buf[1] == 'P') &&
		(buf[2] == 'N') && (buf[3] == 'G') &&
		(buf[4] == 0x0D) && (buf[5] == 0x0A) &&
		(buf[6] == 0x1A) && (buf[7] == 0x0A) &&
		(buf[12] == 'I') &&
		(buf[13] == 'H') &&
		(buf[14] == 'D') &&
		(buf[15] == 'R')) {
		*width = (buf[16]<<24) + (buf[17]<<16) + (buf[18]<<8) + (buf[19]<<0);
		*height = (buf[20]<<24) + (buf[21]<<16) + (buf[22]<<8) + (buf[23]<<0);
		ret = 0;
		goto GETIMAGESIZE_DONE;
	}

GETIMAGESIZE_DONE :
	fclose(f);
	return ret;
}

int
get_icon_mimetype(const char *filename, char *mime_type, int size)
{
	/* From File Extension decide the MIME Type */
	char *pch = strrchr(filename, '.');
	if (pch != NULL) {
		if ((!strncasecmp(pch, EXT_BMP, strlen(EXT_BMP))) ||
			(!strncasecmp(pch, EXT_DIB, strlen(EXT_DIB))) ||
			(!strncasecmp(pch, EXT_RLE, strlen(EXT_RLE))))
			strncpy_n(mime_type, MIME_BMP, size);
		else if ((!strncasecmp(pch, EXT_BMP, strlen(EXT_BMP))) ||
			(!strncasecmp(pch, EXT_JPEG, strlen(EXT_JPEG))) ||
			(!strncasecmp(pch, EXT_JPE, strlen(EXT_JPE))) ||
			(!strncasecmp(pch, EXT_JFIF, strlen(EXT_JFIF))))
			strncpy_n(mime_type, MIME_JPEG, size);
		else if ((!strncasecmp(pch, EXT_GIF, strlen(EXT_GIF))))
			strncpy_n(mime_type, MIME_GIF, size);
		else if ((!strncasecmp(pch, EXT_EMF, strlen(EXT_EMF))))
			strncpy_n(mime_type, MIME_EMF, size);
		else if ((!strncasecmp(pch, EXT_WMF, strlen(EXT_WMF))))
			strncpy_n(mime_type, MIME_WMF, size);
		else if ((!strncasecmp(pch, EXT_TIF, strlen(EXT_TIF))) ||
			(!strncasecmp(pch, EXT_TIFF, strlen(EXT_TIFF))))
			strncpy_n(mime_type, MIME_TIFF, size);
		else if ((!strncasecmp(pch, EXT_PNG, strlen(EXT_PNG))))
			strncpy_n(mime_type, MIME_PNG, size);
		else if ((!strncasecmp(pch, EXT_ICO, strlen(EXT_ICO))))
			strncpy_n(mime_type, MIME_XICO, size);
		else
			strncpy_n(mime_type, MIME_UKWN, size);
	}
	return 0;
}

int
get_icon_metadata(const char* path, const char* filename,
bcm_decode_hspot_anqp_icon_metadata_t *metadata)
{
	assert(filename);
	assert(metadata);

	char fullpath[BUFF_256];
	memset(fullpath, 0, sizeof(fullpath));

	/* Set filename and filenameLength */
	strncpy_n(metadata->filename, filename,
		BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH + 1);

	metadata->filenameLength = strlen(filename);

	/* Append path and filename to get Fullpath */
	snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

	/* Set width and height */
	get_icon_geometry(fullpath, &metadata->width, &metadata->height);

	/* Set lang */
	strncpy_n(metadata->lang, strstr(metadata->filename, "_zxx.") ?
		LANG_ZXX : ENGLISH, VENUE_LANGUAGE_CODE_SIZE +1);

	/* Set type and typeLength */
	get_icon_mimetype(filename, (char*)metadata->type,
		BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH+1);

	metadata->typeLength = strlen((char*)metadata->type);

	return 0;
}

int
get_hspot_flag(const char *prefix, unsigned int bit)
{
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char *val = nvram_get_bitflag(strcat_r(prefix, NVNM_HSFLAG, varname), bit);
	return val != NULL ? atoi(val) : 0;
}

int
set_hspot_flag(const char *prefix, unsigned int bit, int value)
{
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	return nvram_set_bitflag(strcat_r(prefix, NVNM_HSFLAG, varname), bit, value);
}

/* ========================= UTILITY FUNCTIONS ============================= */

/* ======================== DECODING FUNCTIONS ============================ */
int
decode_iw_ie(const char* prefix,
	bcm_decode_interworking_t* iwIe, char* err_nvram)
{
	assert(prefix);
	assert(iwIe);
	assert(err_nvram);

	char *ptr = NULL;
	char varname[NVRAM_MAX_PARAM_LEN] = {0};

	memset(iwIe, 0, sizeof(*iwIe));

	iwIe->isInternet = get_hspot_flag(prefix, HSFLG_IWINT_EN);
	iwIe->isAsra = get_hspot_flag(prefix, HSFLG_IWASRA_EN);

	ptr = nvram_get(strcat_r(prefix, NVNM_IWNETTYPE, varname));
	if (ptr) {
		iwIe->accessNetworkType = atoi(ptr);
	} else {
		strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
		return 0;
	}

	ptr = nvram_get(strcat_r(prefix, NVNM_HESSID, varname));
	if (ptr) {
		if (!strToEther(ptr, &iwIe->hessid)) {
			strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
			return 0;
		} else if (ETHER_ISNULLADDR(iwIe->hessid.octet)) {
			iwIe->isHessid = FALSE;
		} else {
			iwIe->isHessid = TRUE;
		}
	} else {
		strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
		return 0;
	}

	return 1;
}

int
decode_qosmap_ie(const char* prefix,
	bcm_decode_qos_map_t* qosMapSetIe)
{
	assert(prefix);
	assert(qosMapSetIe);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *exceptstr = NULL, *upstr = NULL, *low = NULL, *high = NULL;
	int data_len = 0, except_count = 0;
	uint8 hexdata[BUFF_256] = {0};

	memset(hexdata, 0, sizeof(hexdata));
	memset(qosMapSetIe, 0, sizeof(*qosMapSetIe));

	ptr = nvram_get(strcat_r(prefix, NVNM_QOSMAPIE, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {

		upstr = ptrv;
		exceptstr = strsep(&upstr, "+");

		/* decode except string, if exists */
		if (exceptstr && (strlen(exceptstr) > 0)) {

			data_len = strlen(exceptstr) / 2;
			except_count = data_len / 2;

			if (data_len) {
				get_hex_data((uchar *)exceptstr, hexdata, data_len);

				for (i = 0; i < except_count; i++) {
					qosMapSetIe->except[i].dscp = hexdata[i*2];
					qosMapSetIe->except[i].up = hexdata[(i*2)+1];
				}
				qosMapSetIe->exceptCount = except_count;
			}
		}

		/* decode up string */
		if (upstr && (strlen(upstr) > 0)) {
			tokenParse = NULL;
			i = 0;
			while ((i < BCM_DECODE_QOS_MAP_MAX_UP) &&
				((tokenParse =
				strtok_r(i ? NULL : upstr, ";", &saveptr)) != NULL)) {

				strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

				high = item_value;
				low = strsep(&high, ",");

				if (low && high) {
					qosMapSetIe->up[i].low = (uint8)atoi(low);
					qosMapSetIe->up[i].high = (uint8)atoi(high);
				}
				i++;
			}
		}
		return 1;
	}
	}
	return 0;
}

int
decode_u11_ipaddr_typeavail(const char* prefix,
	bcm_decode_anqp_ip_type_t* ipaddrAvail, char* err_nvram)
{
	assert(prefix);
	assert(ipaddrAvail);
	assert(err_nvram);

	char *ptr = NULL;
	char varname[NVRAM_MAX_PARAM_LEN] = {0};

	memset(err_nvram, 0, sizeof(*err_nvram));

	memset(ipaddrAvail, 0, sizeof(*ipaddrAvail));

	ptr = nvram_get(strcat_r(prefix, NVNM_IPV4ADDR, varname));
	if (ptr) {
		ipaddrAvail->ipv4 = atoi(ptr);
	} else {
		strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
		return 0;
	}
	ptr = nvram_get(strcat_r(prefix, NVNM_IPV6ADDR, varname));
	if (ptr) {
		ipaddrAvail->ipv6 = atoi(ptr);
	} else {
		strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
		return 0;
	}

	ipaddrAvail->isDecodeValid = TRUE;
	return 1;

}

int
decode_u11_netauth_list(const char* prefix,
	bcm_decode_anqp_network_authentication_type_t* netauthlist)
{
	assert(prefix);
	assert(netauthlist);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *nettype = NULL, *redirecturl = NULL;

	memset(netauthlist, 0, sizeof(*netauthlist));

	ptr = nvram_get(strcat_r(prefix, NVNM_NETAUTHLIST, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {
		tokenParse = NULL;
		i = 0;

		while ((i < BCM_DECODE_ANQP_MAX_AUTHENTICATION_UNIT) &&
			((tokenParse =
			strtok_r(i ? NULL : ptrv, "+", &saveptr)) != NULL)) {

			strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

			redirecturl = item_value;
			nettype = strsep(&redirecturl, "=");

			if (nettype) {

			if (!strncasecmp(nettype, NATI_ACCEPTTC, strlen(NATI_ACCEPTTC)))
				netauthlist->unit[i].type = NATI_ACCEPTANCE_OF_TERMS_CONDITIONS;
			else if (!strncasecmp(nettype, NATI_ONLINE, strlen(NATI_ONLINE)))
				netauthlist->unit[i].type = NATI_ONLINE_ENROLLMENT_SUPPORTED;
			else if (!strncasecmp(nettype, NATI_HTTPRED, strlen(NATI_HTTPRED)))
				netauthlist->unit[i].type = NATI_HTTP_HTTPS_REDIRECTION;
			else if (!strncasecmp(nettype, NATI_DNSRED, strlen(NATI_DNSRED)))
				netauthlist->unit[i].type = NATI_DNS_REDIRECTION;
			else
				netauthlist->unit[i].type = NATI_UNSPECIFIED;

			if (((netauthlist->unit[i].type
				== NATI_HTTP_HTTPS_REDIRECTION) && redirecturl) ||
				((netauthlist->unit[i].type
				== NATI_DNS_REDIRECTION) && redirecturl)) {
				strncpy_n((char*)netauthlist->unit[i].url, redirecturl,
					BCM_DECODE_ANQP_MAX_URL_LENGTH + 1);
				netauthlist->unit[i].urlLen = strlen(redirecturl);
			} else {
				strncpy_n((char*)netauthlist->unit[i].url, EMPTY_STR,
					BCM_DECODE_ANQP_MAX_URL_LENGTH + 1);
				netauthlist->unit[i].urlLen = 0;
			}
			}

			i++;
		}
		netauthlist->numAuthenticationType = i;
		netauthlist->isDecodeValid = TRUE;
		return 1;
	}
	}
	return 0;
}

int
decode_u11_realm_list(const char* prefix,
	bcm_decode_anqp_nai_realm_list_ex_t* realmlist)
{
	assert(prefix);
	assert(realmlist);

	char *ptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};

	char *realm = NULL, *realm_left = NULL, *realm_right = NULL;
	char *eap = NULL, *eap_left = NULL, *eap_right = NULL;
	char *auth = NULL, *auth_left = NULL, *auth_right = NULL;
	char *realm_name = NULL, *realm_encode = NULL;
	char *eap_method = NULL, *auth_id = NULL, *auth_param = NULL;
	int iR = 0, iE = 0, iA = 0;
	int iEap_method = 0, iAuth_id = 0, iAuth_param = 0;
	uint8 auth_param_val[1] = {0};

	memset(realmlist, 0, sizeof(*realmlist));

	ptr = nvram_get(strcat_r(prefix, NVNM_REALMLIST, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {

	realm = ptrv;
	realm_right = realm;

	while (realm) {

		iE = 0;
		realm_left = strsep(&realm_right, "?");

		eap = realm_left;
		realm_name = strsep(&eap, "+");
		realm_encode = strsep(&eap, "+");

		/* Fill Realm Encoding */
		realmlist->realm[iR].encoding
			= (uint8)atoi(realm_encode);

		/* Fill Realm Name and length */
		realmlist->realm[iR].realmLen = strlen(realm_name);
		strncpy_n((char*)realmlist->realm[iR].realm,
			realm_name, BCM_DECODE_ANQP_MAX_REALM_LENGTH + 1);

		/* Fill EAP Info, to print in EapAuth Field on UI */
		strncpy_n((char*)realmlist->realm[iR].eapInfo,
			eap, BCM_DECODE_ANQP_MAX_REALM_LENGTH+1);

		while (eap) {

			iA = 0;
			eap_right = eap;
			eap_left = strsep(&eap_right, ";");

			auth = eap_left;
			eap_method = strsep(&auth, "=");
			iEap_method = atoi(eap_method);

			/* Fill EAP Method */
			realmlist->realm[iR].eap[iE].eapMethod
				= (uint8)iEap_method;

			if (iE)
			strncat((char*)realmlist->realm[iR].realmInfo, ";",
			min(1, REALM_INFO_LENGTH -
			strlen((char*)realmlist->realm[iR].realmInfo)));

			const char *realmptr = idtostr_eapmethod(iEap_method);

			if (realmptr) {
				strncat((char*)realmlist->realm[iR].realmInfo,
				realmptr, min(strlen(realmptr),
				REALM_INFO_LENGTH -
				strlen((char*)realmlist->realm[iR].realmInfo)));
			}

			strncat((char*)realmlist->realm[iR].realmInfo, "=",
				min(1, REALM_INFO_LENGTH -
				strlen((char*)realmlist->realm[iR].realmInfo)));

			while (auth) {

				auth_right = auth;
				auth_left = strsep(&auth_right, "#");

				auth_param = auth_left;
				auth_id = strsep(&auth_param, ",");

				iAuth_id = atoi(auth_id);
				iAuth_param = atoi(auth_param);

				/* Fill Auth ID */
				realmlist->realm[iR].eap[iE].auth[iA].id
					= (uint8)iAuth_id;

				if (iA)
				strncat((char*)realmlist->realm[iR].realmInfo, "#",
				min(1, REALM_INFO_LENGTH -
				strlen((char*)realmlist->realm[iR].realmInfo)));

				int authparamtype = 0;
				const char *authptr =
					idtostr_authid(iAuth_id, &authparamtype);

				if ((authptr) &&
				(AUTH_PARAM_TYPE_UNKNOWN != authparamtype)) {
					strncat((char*)realmlist->realm[iR].realmInfo,
						authptr, min(strlen(authptr),
						REALM_INFO_LENGTH -
						strlen((char*)realmlist->realm[iR].realmInfo)));
				}

				strncat((char*)realmlist->realm[iR].realmInfo, ",",
					min(1, REALM_INFO_LENGTH -
					strlen((char*)realmlist->realm[iR].realmInfo)));

				/* Fill Auth Param */
				auth_param_val[0] = iAuth_param;
				memcpy(realmlist->realm[iR].eap[iE].auth[iA].value,
					auth_param_val, sizeof(auth_param_val));

				const char *authparamval =
					idtostr_authparam(authparamtype, iAuth_param);

				if ((authparamval) &&
				(AUTH_PARAM_TYPE_UNKNOWN != authparamtype)) {
					strncat((char*)realmlist->realm[iR].realmInfo,
						authparamval, min(strlen(authparamval),
						REALM_INFO_LENGTH -
						strlen((char*)realmlist->realm[iR].realmInfo)));
				}
				/* Fill Auth Len */
				realmlist->realm[iR].eap[iE].auth[iA].len
					= sizeof(auth_param_val);

				auth = auth_right;
				iA++;
			}

			/* Fill Auth Count */
			realmlist->realm[iR].eap[iE].authCount = iA;
			eap = eap_right;
			iE++;
		}

		/* Fill Eap Count */
		realmlist->realm[iR].eapCount = iE;
		realm = realm_right;
		iR++;
	}

	/* Fill Realm Count */
	realmlist->realmCount = iR;
	realmlist->isDecodeValid = TRUE;
	return 1;
	}
	}
	return 0;
}

int
decode_u11_venue_list(const char* prefix,
	bcm_decode_anqp_venue_name_t* venuelist, char* err_nvram)
{
	assert(prefix);
	assert(venuelist);
	assert(err_nvram);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char ptrUTF8[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *venue = NULL, *lang = NULL;
	memset(err_nvram, 0, sizeof(*err_nvram));

	memset(venuelist, 0, sizeof(*venuelist));

	ptr = nvram_get(strcat_r(prefix, NVNM_VENUETYPE, varname));
	if (ptr) {
		venuelist->type = atoi(ptr);
	} else {
	       strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
	       return 0;
	}

	ptr = nvram_get(strcat_r(prefix, NVNM_VENUEGRP, varname));
	if (ptr) {
		venuelist->group = atoi(ptr);
	} else {
		strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
		return 0;
	}

	ptr = nvram_get(strcat_r(prefix, NVNM_VENUELIST, varname));
	if (ptr) {
		strncpy_n(ptrUTF8, ptr, NVRAM_MAX_VAL_LEN);
		hex_to_bytes((uchar*)ptrv, NVRAM_MAX_VAL_LEN,
			(uchar*)ptrUTF8, NVRAM_MAX_VAL_LEN);

		venuelist->numVenueName = 0;
		if (strlen(ptrv) > 0) {
			tokenParse = NULL;
			i = 0;
			while ((i < BCM_DECODE_ANQP_MAX_VENUE_NAME) &&
				((tokenParse =
				strtok_r(i ? NULL : ptrv, "|", &saveptr)) != NULL)) {

				strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

				lang = item_value;
				venue = strsep(&lang, "!");

				if (venue && lang) {
				strncpy_n(venuelist->venueName[i].name, venue,
					VENUE_NAME_SIZE + 1);
				venuelist->venueName[i].nameLen = strlen(venue);
				strncpy_n(venuelist->venueName[i].lang, lang,
					VENUE_LANGUAGE_CODE_SIZE +1);
				venuelist->venueName[i].langLen = strlen(lang);
				}
				i++;
			}
			venuelist->numVenueName = i;
			venuelist->isDecodeValid = TRUE;
			return 1;
		}
	} else {
	       strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
	       return 0;
	}

	return 1;
}

int
decode_u11_oui_list(const char* prefix,
	bcm_decode_anqp_roaming_consortium_ex_t* ouilist)
{
	assert(prefix);
	assert(ouilist);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *oi = NULL, *is_beacon = NULL;
	int data_len = 0;

	memset(ouilist, 0, sizeof(*ouilist));

	ptr = nvram_get(strcat_r(prefix, NVNM_OUILIST, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {
		i = 0;
		tokenParse = NULL;
		while ((i < BCM_DECODE_ANQP_MAX_OI) &&
			((tokenParse =
			strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {

			strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

			is_beacon = item_value;
			oi = strsep(&is_beacon, ":");

			if (oi && is_beacon) {
				data_len = strlen(oi) / 2;

			if (data_len && (data_len <= BCM_DECODE_ANQP_MAX_OI_LENGTH)) {

				get_hex_data((uchar *)oi,
					ouilist->oi[i].oi, data_len);
				ouilist->oi[i].oiLen = data_len;
				ouilist->oi[i].isBeacon = atoi(is_beacon);
			}
			}
			i++;
		}
		ouilist->numOi = i;
		ouilist->isDecodeValid = TRUE;
		return 1;
	}
	}
	return 0;
}

int
decode_u11_3gpp_list(const char* prefix,
	bcm_decode_anqp_3gpp_cellular_network_t* gpp3list)
{
	assert(prefix);
	assert(gpp3list);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *mcc = NULL, *mnc = NULL;

	memset(gpp3list, 0, sizeof(*gpp3list));

	ptr = nvram_get(strcat_r(prefix, NVNM_3GPPLIST, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {
			i = 0;
			tokenParse = NULL;

			while ((i < BCM_DECODE_ANQP_MAX_PLMN) &&
				((tokenParse =
				strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {

				strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

				mnc  = item_value;
				mcc  = strsep(&mnc, ":");

				if (mcc && mnc) {
					strncpy_n(gpp3list->plmn[i].mcc, mcc,
						BCM_DECODE_ANQP_MCC_LENGTH + 1);
					strncpy_n(gpp3list->plmn[i].mnc, mnc,
						BCM_DECODE_ANQP_MNC_LENGTH + 1);
				}
				i++;
			}
			gpp3list->plmnCount = i;
			gpp3list->isDecodeValid = TRUE;
			return 1;
		}
	}
	return 0;
}

int
decode_u11_domain_list(const char* prefix,
	bcm_decode_anqp_domain_name_list_t* domainlist)
{
	assert(prefix);
	assert(domainlist);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};

	memset(domainlist, 0, sizeof(*domainlist));

	ptr = nvram_get(strcat_r(prefix, NVNM_DOMAINLIST, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {
			tokenParse = NULL;
			i = 0;

			while ((i < BCM_DECODE_ANQP_MAX_DOMAIN) &&
				((tokenParse =
				strtok_r(i ? NULL : ptrv, " ", &saveptr)) != NULL)) {

				strncpy_n(domainlist->domain[i].name, tokenParse,
					BCM_DECODE_ANQP_MAX_DOMAIN_NAME_SIZE+1);
				domainlist->domain[i].len = strlen(tokenParse);
				i++;
			}
			domainlist->numDomain = i;
			domainlist->isDecodeValid = TRUE;
			return 1;
		}
	}
	return 0;
}

int
decode_hspot_oper_class(const char* prefix,
	bcm_decode_hspot_anqp_operating_class_indication_t* opclass)
{
	assert(prefix);
	assert(opclass);

	char *ptr = NULL;
	char varname[NVRAM_MAX_PARAM_LEN] = {0};

	int operating_class = 1;
	uint8 opClass1[1] = {OPCLS_2G};
	uint8 opClass2[1] = {OPCLS_5G};
	uint8 opClass3[2] = {OPCLS_2G, OPCLS_5G};

	memset(opclass, 0, sizeof(*opclass));

	ptr = nvram_get(strcat_r(prefix, NVNM_OPERCLS, varname));
	if (ptr) {
		operating_class = atoi(ptr);

		if (operating_class == 3) {
			opclass->opClassLen = sizeof(opClass3);
			memcpy(opclass->opClass, opClass3, sizeof(opClass3));
		} else if (operating_class == 2) {
			opclass->opClassLen = sizeof(opClass2);
			memcpy(opclass->opClass, opClass2, sizeof(opClass2));
		} else if (operating_class == 1) {
			opclass->opClassLen = sizeof(opClass1);
			memcpy(opclass->opClass, opClass1, sizeof(opClass1));
		}
		opclass->isDecodeValid = TRUE;
		return 1;
	}
	return 0;
}

int
decode_hspot_anonai(const char* prefix,
	bcm_decode_hspot_anqp_anonymous_nai_t* anonai)
{
	assert(prefix);
	assert(anonai);

	char *ptr = NULL;
	char varname[NVRAM_MAX_PARAM_LEN] = {0};

	memset(anonai, 0, sizeof(*anonai));

	ptr = nvram_get(strcat_r(prefix, NVNM_ANONAI, varname));
	if (ptr) {
		strncpy_n(anonai->nai, ptr,
			BCM_DECODE_HSPOT_ANQP_MAX_NAI_SIZE + 1);
		anonai->naiLen = strlen(anonai->nai);
		anonai->isDecodeValid = TRUE;
		return 1;
	}
	return 0;
}

int
decode_hspot_wan_metrics(const char* prefix,
	bcm_decode_hspot_anqp_wan_metrics_t* wanmetrics)

{
	assert(prefix);
	assert(wanmetrics);

	char *ptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};

	char *p_linkStatus = NULL, *p_symmetricLink = NULL, *p_atCapacity = NULL;
	char *p_dlinkSpeed = NULL, *p_ulinkLoad = NULL, *p_lmd = NULL;
	char *p_ulinkSpeed = NULL, *p_dlinkLoad = NULL;

	memset(wanmetrics, 0, sizeof(*wanmetrics));

	ptr = nvram_get(strcat_r(prefix, NVNM_WANMETRICS, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {
		p_dlinkSpeed = ptrv;
		p_linkStatus = strsep(&p_dlinkSpeed, "=");

		p_dlinkLoad = p_dlinkSpeed;
		p_dlinkSpeed = strsep(&p_dlinkLoad, "=");

		p_lmd = p_dlinkLoad;
		p_dlinkLoad = strsep(&p_lmd, "=");

		/* Parse next 2 params >>	p_dlinkLoad>p_ulinkLoad */
		p_ulinkLoad = p_dlinkLoad;
		p_dlinkLoad = strsep(&p_ulinkLoad, ">");

		/* Parse next 2 params >>	p_dlinkSpeed>p_ulinkSpeed */
		p_ulinkSpeed = p_dlinkSpeed;
		p_dlinkSpeed = strsep(&p_ulinkSpeed, ">");

		/* Parse first 3 params >> */
		/* p_linkStatus:p_symmetricLink:p_atCapacity */
		p_atCapacity	= p_linkStatus;
		p_linkStatus	= strsep(&p_atCapacity, ":");
		p_symmetricLink = strsep(&p_atCapacity, ":");

		if (p_linkStatus && p_symmetricLink &&
			p_atCapacity && p_dlinkSpeed && p_ulinkSpeed &&
			p_dlinkLoad && p_ulinkLoad && p_lmd) {

			wanmetrics->linkStatus = atoi(p_linkStatus);
			wanmetrics->symmetricLink = atoi(p_symmetricLink);
			wanmetrics->atCapacity = atoi(p_atCapacity);
			wanmetrics->dlinkSpeed = atoi(p_dlinkSpeed);
			wanmetrics->ulinkSpeed = atoi(p_ulinkSpeed);
			wanmetrics->dlinkLoad = atoi(p_dlinkLoad);
			wanmetrics->ulinkLoad = atoi(p_ulinkLoad);
			wanmetrics->lmd = atoi(p_lmd);
		}
		wanmetrics->isDecodeValid = TRUE;
		return 1;
	}
	}
	return 0;
}

int
decode_hspot_op_list(const char* prefix,
	bcm_decode_hspot_anqp_operator_friendly_name_t* oplist)
{
	assert(prefix);
	assert(oplist);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *oper = NULL, *lang = NULL;

	memset(oplist, 0, sizeof(*oplist));

	ptr = nvram_get(strcat_r(prefix, NVNM_OPLIST, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {
			tokenParse = NULL;
			i = 0;
			while ((i < BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME) &&
				((tokenParse =
				strtok_r(i ? NULL : ptrv, "|", &saveptr)) != NULL)) {

				strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

				lang = item_value;
				oper = strsep(&lang, "!");

				if (oper && lang) {
					strncpy_n(oplist->duple[i].name, oper,
						VENUE_NAME_SIZE + 1);
					oplist->duple[i].nameLen = strlen(oper);
					strncpy_n(oplist->duple[i].lang, lang,
						VENUE_LANGUAGE_CODE_SIZE +1);
					oplist->duple[i].langLen = strlen(lang);
				}
				i++;
			}
			oplist->numName  = i;
			oplist->isDecodeValid = TRUE;
			return 1;
		}
	}
	return 0;
}

int
decode_hspot_homeq_list(const char* prefix,
	bcm_decode_hspot_anqp_nai_home_realm_query_t* homeqlist)
{
	assert(prefix);
	assert(homeqlist);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *homerealm = NULL, *encode = NULL;

	memset(homeqlist, 0, sizeof(*homeqlist));

	ptr = nvram_get(strcat_r(prefix, NVNM_HOMEQLIST, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {
		tokenParse = NULL;
		i = 0;
		while ((i < BCM_DECODE_HSPOT_ANQP_MAX_HOME_REALM) &&
			((tokenParse =
			strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {

			strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

			encode = item_value;
			homerealm = strsep(&encode, ":");

			if (homerealm && encode) {
				strncpy_n(homeqlist->data[i].name, homerealm,
					VENUE_NAME_SIZE + 1);
				homeqlist->data[i].nameLen = strlen(homerealm);

				if (!strncasecmp(encode, ENC_RFC4282, strlen(ENC_RFC4282)))
					homeqlist->data[i].encoding
						= REALM_ENCODING_RFC4282;
				else if (!strncasecmp(encode, ENC_UTF8, strlen(ENC_UTF8)))
					homeqlist->data[i].encoding
						= REALM_ENCODING_UTF8;
			}
			i++;
		}
		homeqlist->count = i;
		homeqlist->isDecodeValid = TRUE;
		return 1;
	}
	}
	return 0;
}

int
decode_hspot_conncap_list(const char* prefix,
	bcm_decode_hspot_anqp_connection_capability_t* concaplist)
{
	assert(prefix);
	assert(concaplist);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *status = NULL, *protocol = NULL, *port = NULL;

	memset(concaplist, 0, sizeof(*concaplist));

	ptr = nvram_get(strcat_r(prefix, NVNM_CONCAPLIST, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {
		tokenParse = NULL;
		i = 0;
		while ((i < BCM_DECODE_HSPOT_ANQP_MAX_CONNECTION_CAPABILITY) &&
			((tokenParse =
			strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {

			strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

			status = item_value;
			protocol = strsep(&status, ":");
			port = strsep(&status, ":");

			if (protocol && port && status) {
				concaplist->tuple[i].ipProtocol	= atoi(protocol);
				concaplist->tuple[i].portNumber	= atoi(port);
				concaplist->tuple[i].status	= atoi(status);
			}
			i++;
		}
		concaplist->numConnectCap = i;
		concaplist->isDecodeValid = TRUE;
		return 1;
	}
	}
	return 0;
}

int
decode_hspot_osup_list(const char* prefix,
	bcm_decode_hspot_anqp_osu_provider_list_ex_t* osuplist, char* err_nvram)
{
	assert(osuplist);
	assert(osuplist);
	assert(err_nvram);

	int i = 0, ret = 1;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	uint8 osu_method[1] = {0};
	char *provider = NULL, *provider_left = NULL, *provider_right = NULL;
	char ptrFrndlyName[NVRAM_MAX_VAL_LEN] = {0};
	int iP = 0, iD = 0;
	char *oper = NULL, *lang = NULL;

	memset(err_nvram, 0, sizeof(*err_nvram));
	memset(osuplist, 0, sizeof(*osuplist));

	/* ------------------- OSU ICON ID ------------------------ */
	osuplist->osuicon_id = get_hspot_flag(prefix, HSFLG_OSUICON_ID);
	osuplist->osuicon_id++;

	/* --------------------- OSU SSID ------------------------- */
	ptr = nvram_get(strcat_r(prefix, NVNM_OSU_SSID, varname));
	if (ptr) {
		strncpy_n((char*)osuplist->osuSsid, ptr,
			BCM_DECODE_HSPOT_ANQP_MAX_OSU_SSID_LENGTH + 1);
		osuplist->osuSsidLength = strlen(ptr);
		osuplist->isDecodeValid = TRUE;
	} else {
		ret = 0;
		goto DECODE_HSPOT_OSUP_LIST_ERROR;
	}

	/* --------------------- OSU URI -------------------------- */
	ptr = nvram_get(strcat_r(prefix, NVNM_OSU_URI, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {
			tokenParse = NULL;
			i = 0;
			while ((i < BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER) &&
				((tokenParse =
				strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {

				strncpy_n((char*)osuplist->osuProvider[i].uri,
					tokenParse,
					BCM_DECODE_HSPOT_ANQP_MAX_URI_LENGTH + 1);
				osuplist->osuProvider[i].uriLength
					= strlen(tokenParse);
				i++;
			}
			osuplist->osuProviderCount = i;
		}
	} else {
		ret = 0;
		goto DECODE_HSPOT_OSUP_LIST_ERROR;
	}

	/* --------------------- OSU NAI -------------------------- */
	ptr = nvram_get(strcat_r(prefix, NVNM_OSU_NAI, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {
			tokenParse = NULL;
			i = 0;
			while ((i < BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER) &&
				((tokenParse =
				strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {
				strncpy_n((char*)osuplist->osuProvider[i].nai, tokenParse,
					BCM_DECODE_HSPOT_ANQP_MAX_NAI_LENGTH + 1);
				osuplist->osuProvider[i].naiLength = strlen(tokenParse);
				i++;
			}
		}
	}

	/* -------------------- OSU Method ------------------------- */
	ptr = nvram_get(strcat_r(prefix, NVNM_OSU_METHOD, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {
			tokenParse = NULL;
			i = 0;
			while ((i < BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER) &&
				((tokenParse =
				strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {

				osu_method[0] = (!strncasecmp(tokenParse,
					OMADM_NVVAL, strlen(OMADM_NVVAL))) ?
					HSPOT_OSU_METHOD_OMA_DM : HSPOT_OSU_METHOD_SOAP_XML;
				memcpy(osuplist->osuProvider[i].method,
					osu_method, sizeof(osu_method));
				osuplist->osuProvider[i].methodLength
					= sizeof(osu_method);
				i++;
			}
		}
	} else {
		ret = 0;
		goto DECODE_HSPOT_OSUP_LIST_ERROR;
	}

	/* ----------------- OSU Friendly Name ----------------------- */
	memset(ptrFrndlyName, 0, sizeof(ptrFrndlyName));
	ptr = nvram_get(strcat_r(prefix, NVNM_OSU_FRNDNAME, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {
			provider = ptrv;
			provider_right = provider;

			while ((iP < BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER) && provider) {
				tokenParse = NULL;
				iD = 0;

				provider_left = strsep(&provider_right, ";");
				strncpy_n(ptrFrndlyName, provider_left, NVRAM_MAX_VAL_LEN);

				strncpy_n((char*)osuplist->osuProvider[iP].nameInfo,
					provider_left, FRIENDLY_NAME_INFO_LENGTH+1);

				while ((iD < BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME) &&
					((tokenParse =
					strtok_r(iD ? NULL : ptrFrndlyName, "|", &saveptr))
					!= NULL)) {

					strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

					lang = item_value;
					oper = strsep(&lang, "!");

					if (oper && lang) {
					strncpy_n(osuplist->osuProvider[iP].name.duple[iD].name,
						oper, VENUE_NAME_SIZE + 1);
					osuplist->osuProvider[iP].name.duple[iD].nameLen
						= strlen(oper);
					strncpy_n(osuplist->osuProvider[iP].name.duple[iD].lang,
						lang, VENUE_LANGUAGE_CODE_SIZE +1);
					osuplist->osuProvider[iP].name.duple[iD].langLen
						= strlen(lang);
					}
					iD++;
				}
				osuplist->osuProvider[iP].name.numName	= iD;
				provider = provider_right;
				iP++;
			}
		}
	} else {
		ret = 0;
		goto DECODE_HSPOT_OSUP_LIST_ERROR;
	}

	/* ----------------- OSU Server Desc ------------------------- */
	provider = NULL, provider_left = NULL, provider_right = NULL;
	memset(ptrFrndlyName, 0, sizeof(ptrFrndlyName));
	iP = 0, iD = 0;
	oper = NULL,  lang = NULL;
	ptr = nvram_get(strcat_r(prefix, NVNM_OSU_SERVDESC, varname));
	if (ptr) {
		strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
		if (strlen(ptrv) > 0) {

			provider = ptrv;
			provider_right = provider;

			while ((iP < BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER) && provider) {

				tokenParse = NULL;
				iD = 0;

				provider_left = strsep(&provider_right, ";");

				strncpy_n(ptrFrndlyName, provider_left, NVRAM_MAX_VAL_LEN);

				strncpy_n((char*)osuplist->osuProvider[iP].descInfo,
					provider_left, FRIENDLY_NAME_INFO_LENGTH+1);

				while ((iD < BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME) &&
					((tokenParse =
					strtok_r(iD ? NULL : ptrFrndlyName, "|", &saveptr))
					!= NULL)) {

					strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

					lang = item_value;
					oper = strsep(&lang, "!");

					if (oper && lang) {
					strncpy_n(osuplist->osuProvider[iP].desc.duple[iD].name,
						oper, VENUE_NAME_SIZE + 1);
					osuplist->osuProvider[iP].desc.duple[iD].nameLen
						= strlen(oper);
					strncpy_n(osuplist->osuProvider[iP].desc.duple[iD].lang,
						lang,
						VENUE_LANGUAGE_CODE_SIZE +1);
					osuplist->osuProvider[iP].desc.duple[iD].langLen
						= strlen(lang);
					}
					iD++;
				}
				osuplist->osuProvider[iP].desc.numName	= iD;
				provider = provider_right;
				iP++;
			}
		}
	}

	/* --------------------- OSU Icons -------------------------- */
	provider = NULL, provider_left = NULL, provider_right = NULL;
	memset(ptrFrndlyName, 0, sizeof(ptrFrndlyName));
	iP = 0, iD = 0;
	oper = NULL,  lang = NULL;
	ptr = nvram_get(strcat_r(prefix, NVNM_OSU_ICONS, varname));

	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0)
	{
		provider = ptrv;
		provider_right = provider;

		while ((iP < BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER) && provider) {

			tokenParse = NULL;
			iD = 0;

			provider_left = strsep(&provider_right, ";");

			strncpy_n(ptrFrndlyName, provider_left, NVRAM_MAX_VAL_LEN);

			strncpy_n((char*)osuplist->osuProvider[iP].iconInfo,
				provider_left, FRIENDLY_NAME_INFO_LENGTH+1);

			while ((iD < BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME) &&
				((tokenParse =
				strtok_r(iD ? NULL : ptrFrndlyName, "+", &saveptr))
				!= NULL)) {

				/* Fill Icon Metadata */
				get_icon_metadata(ICONPATH, tokenParse,
				&osuplist->osuProvider[iP].iconMetadata[iD]);

				iD++;
			}
			osuplist->osuProvider[iP].iconMetadataCount = iD;
			provider = provider_right;
			iP++;
		}
	}
	} else {
		ret = 0;
		goto DECODE_HSPOT_OSUP_LIST_ERROR;
	}

	/* Reset Other Providers data */
	for (i = iP; i < MAX_OSU_PROVIDERS; i++)
		memset(&osuplist->osuProvider[i], 0,
			sizeof(osuplist->osuProvider[i]));

DECODE_HSPOT_OSUP_LIST_ERROR :
	if (!ret)
		strncpy_n(err_nvram, varname, NVRAM_MAX_PARAM_LEN);
	else
		osuplist->isDecodeValid = TRUE;

	return ret;
}

int
decode_u11_urifqdn_list(const char* prefix,
	bcm_decode_hspot_anqp_location_uri_fqdn_query_t* urifqdnlist)
{
	assert(prefix);
	assert(urifqdnlist);

	int i = 0;
	char *ptr = NULL, *tokenParse = NULL, *saveptr = NULL;
	char ptrv[NVRAM_MAX_VAL_LEN] = {0};
	char varname[NVRAM_MAX_PARAM_LEN] = {0};
	char item_value[NVRAM_MAX_VALUE_LEN] = {0};

	char *itemname = NULL, *encode = NULL;

	memset(urifqdnlist, 0, sizeof(*urifqdnlist));

	ptr = nvram_get(strcat_r(prefix, NVNM_PUBIDURIFQDNLIST, varname));
	if (ptr) {
	strncpy_n(ptrv, ptr, NVRAM_MAX_VAL_LEN);
	if (strlen(ptrv) > 0) {
		tokenParse = NULL;
		i = 0;
		while ((i < BCM_DECODE_HSPOT_ANQP_MAX_URI_FQDN) &&
			((tokenParse =
			strtok_r(i ? NULL : ptrv, ";", &saveptr)) != NULL)) {

			strncpy_n(item_value, tokenParse, NVRAM_MAX_VALUE_LEN);

			encode = item_value;
			itemname = strsep(&encode, ":");

			if (itemname && encode) {
				strncpy_n(urifqdnlist->data[i].name, itemname,
					URI_FQDN_SIZE + 1);
				urifqdnlist->data[i].nameLen = strlen(itemname);

				if (!strncasecmp(encode, ENC_HELD, strlen(ENC_HELD)))
					urifqdnlist->data[i].encoding
						= LOCATION_ENCODING_HELD;
				else if (!strncasecmp(encode, ENC_SUPL, strlen(ENC_SUPL)))
					urifqdnlist->data[i].encoding
						= LOCATION_ENCODING_SUPL;
			}
			i++;
		}
		urifqdnlist->count = i;
		urifqdnlist->isDecodeValid = TRUE;
		return 1;
	}
	}
	return 0;
}
/* ======================== DECODING FUNCTIONS ============================ */
