/*
 * WPS NFC share utility
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
 * $Id: $
 */
#include <wpsheaders.h>
#include <wpscommon.h>
#include <wpserror.h>
#include <sminfo.h>
#include <reg_proto.h>
#include <reg_prototlv.h>
#include <tutrace.h>
#include <nfc_utils.h>

uint32
nfc_utils_build_cfg(DevInfo *info, uint8 *buf, uint *buflen)
{
	char *p_nwKey, *cp_data;
	uint8 *p_macAddr, wep_exist = 0;
	uint16 data16;
	uint32 ret = WPS_SUCCESS, nwKeyLen = 0;
	BufferObj *bufObj = NULL, *vendorExt_bufObj = NULL;
	CTlvVendorExt vendorExt;
	CTlvCredential *p_tlvCred = NULL;

	TUTRACE((TUTRACE_NFC, "Build NFC Configuration\n"));

	if (info == NULL || buf == NULL || *buflen == 0) {
		TUTRACE((TUTRACE_NFC, "Invalid arguments\n"));
		return WPS_ERR_SYSTEM;
	}

	if ((bufObj = buffobj_new()) == NULL) {
		TUTRACE((TUTRACE_NFC, "Out of memory\n"));
		return WPS_ERR_OUTOFMEMORY;
	}

	/* Credential */
	if ((p_tlvCred = (CTlvCredential *)malloc(sizeof(CTlvCredential))) == NULL) {
		TUTRACE((TUTRACE_NFC, "Out of memory\n"));
		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}
	memset(p_tlvCred, 0, sizeof(CTlvCredential));

	/* Credential items */
	/* nwIndex */
	tlv_set(&p_tlvCred->nwIndex, WPS_ID_NW_INDEX, (void *)1, 0);

	/* ssid */
	cp_data = info->ssid;
	data16 = strlen(cp_data);
	tlv_set(&p_tlvCred->ssid, WPS_ID_SSID, cp_data, data16);

	/* auth */
	if (info->auth)
		data16 = WPS_AUTHTYPE_SHARED;
	else if (devinfo_getKeyMgmtType(info) == WPS_WL_AKM_PSK)
		data16 = WPS_AUTHTYPE_WPAPSK;
	else if (devinfo_getKeyMgmtType(info) == WPS_WL_AKM_PSK2)
		data16 = WPS_AUTHTYPE_WPA2PSK;
	else if (devinfo_getKeyMgmtType(info) == WPS_WL_AKM_BOTH)
		data16 = WPS_AUTHTYPE_WPAPSK | WPS_AUTHTYPE_WPA2PSK;
	else
		data16 = WPS_AUTHTYPE_OPEN;
	tlv_set(&p_tlvCred->authType, WPS_ID_AUTH_TYPE, UINT2PTR(data16), 0);

	/* encrType */
	if (info->auth)
		data16 = WPS_ENCRTYPE_WEP;
	else if (data16 == WPS_AUTHTYPE_OPEN) {
		if (info->wep)
			data16 = WPS_ENCRTYPE_WEP;
		else
			data16 = WPS_ENCRTYPE_NONE;
	}
	else
		data16 = info->crypto;
	tlv_set(&p_tlvCred->encrType, WPS_ID_ENCR_TYPE, UINT2PTR(data16), 0);

	if (data16 == WPS_ENCRTYPE_WEP)
		wep_exist = 1;

	/* nwKeyIndex
	 * WSC 2.0, "Network Key Index" deprecated - only included by WSC 1.0 devices.
	 * Ignored by WSC 2.0 or newer devices.
	 */
	if (info->version2 < WPS_VERSION2) {
		tlv_set(&p_tlvCred->nwKeyIndex, WPS_ID_NW_KEY_INDEX, (void *)1, 0);
	}

	/* nwKey */
	p_nwKey = info->nwKey;
	nwKeyLen = strlen(info->nwKey);
	tlv_set(&p_tlvCred->nwKey, WPS_ID_NW_KEY, p_nwKey, nwKeyLen);

	/* enrollee's mac */
	p_macAddr = info->peerMacAddr;
	data16 = SIZE_MAC_ADDR;
	tlv_set(&p_tlvCred->macAddr, WPS_ID_MAC_ADDR, p_macAddr, data16);

	/* WepKeyIdx */
	if (wep_exist)
		tlv_set(&p_tlvCred->WEPKeyIndex, WPS_ID_WEP_TRANSMIT_KEY,
			UINT2PTR(info->wepKeyIdx), 0);

	/* WSC 2.0, WFA "Network Key Shareable" subelement */
	if (info->version2 >= WPS_VERSION2) {
		/* Always shareable */
		data16 = 1;
		subtlv_set(&p_tlvCred->nwKeyShareable, WPS_WFA_SUBID_NW_KEY_SHAREABLE,
			UINT2PTR(data16), 0);
	}
	tlv_credentialWrite(p_tlvCred, bufObj);

	/* (Optionally) Add RF-Band, AP-Channel and BSSID attribute at v2.0.2.1-rev21 */
	if (info->rfBand != 0) {
		tlv_serialize(WPS_ID_RF_BAND, bufObj, &info->rfBand, WPS_ID_RF_BAND_S);
		TUTRACE((TUTRACE_NFC, "info->rfBand=%d\n", info->rfBand));
	}

	if (info->apchannel != 0) {
		tlv_serialize(WPS_ID_AP_CHANNEL, bufObj, &info->apchannel,
			WPS_ID_AP_CHANNEL_S);
		TUTRACE((TUTRACE_NFC, "info->apchannel=%d\n", info->apchannel));
	}

	if (memcmp(info->macAddr, "\0\0\0\0\0\0", SIZE_MAC_ADDR) != 0) {
		tlv_serialize(WPS_ID_MAC_ADDR, bufObj, info->macAddr, SIZE_MAC_ADDR);
		TUTRACE((TUTRACE_NFC, "info->macAddr=%02x:%02x:%02x:%02x:%02x:%02x\n",
			info->macAddr[0], info->macAddr[1], info->macAddr[2],
			info->macAddr[3], info->macAddr[4], info->macAddr[5]));
	}

	/* Version2 */
	if (info->version2 >= WPS_VERSION2) {
		if ((vendorExt_bufObj = buffobj_new()) == NULL) {
			TUTRACE((TUTRACE_NFC, "Out of memory\n"));
			ret = WPS_ERR_OUTOFMEMORY;
			goto error;
		}

		subtlv_serialize(WPS_WFA_SUBID_VERSION2, vendorExt_bufObj,
			&info->version2, WPS_WFA_SUBID_VERSION2_S);

		/* Serialize subelemetns to Vendor Extension */
		vendorExt.vendorId = (uint8 *)WFA_VENDOR_EXT_ID;
		vendorExt.vendorData = buffobj_GetBuf(vendorExt_bufObj);
		vendorExt.dataLength = buffobj_Length(vendorExt_bufObj);
		tlv_vendorExtWrite(&vendorExt, bufObj);
	}

	/* Check copy back avaliable buf length */
	if (*buflen < buffobj_Length(bufObj)) {
		TUTRACE((TUTRACE_NFC, "In buffer len too short\n"));
		ret = WPS_ERR_SYSTEM;
		goto error;
	}

	/* Copy back data and length */
	*buflen = buffobj_Length(bufObj);
	memcpy(buf, buffobj_GetBuf(bufObj), buffobj_Length(bufObj));

error:
	/* Free local vendorExt_bufObj */
	if (vendorExt_bufObj)
		buffobj_del(vendorExt_bufObj);

	/* Free local p_tlvCred */
	if (p_tlvCred)
		tlv_credentialDelete(p_tlvCred, 0);

	/* Free local bufObj */
	if (bufObj)
		buffobj_del(bufObj);

	return ret;
}

uint32
nfc_utils_build_pw(DevInfo *info, uint8 *buf, uint *buflen)
{
	uint32 ret = WPS_SUCCESS;
	BufferObj *bufObj = NULL, *vendorExt_bufObj = NULL;
	CTlvVendorExt vendorExt;
	CTlvOobDevPwd oobDevPwd;
	unsigned char hex_pin[SIZE_32_BYTES];
	int hex_pin_len;

	TUTRACE((TUTRACE_NFC, "Build NFC Password\n"));

	if (info == NULL || buf == NULL || *buflen == 0) {
		TUTRACE((TUTRACE_NFC, "Invalid arguments\n"));
		return WPS_ERR_SYSTEM;
	}

	if ((bufObj = buffobj_new()) == NULL) {
		TUTRACE((TUTRACE_NFC, "Out of memory\n"));
		return WPS_ERR_OUTOFMEMORY;
	}

	/* OOB Device Password */
	oobDevPwd.publicKeyHash = info->pub_key_hash;
	oobDevPwd.pwdId = info->devPwdId;

	/* Do String to Hex translation */
	hex_pin_len = wps_str2hex(hex_pin, sizeof(hex_pin), info->pin);
	if (hex_pin_len == 0) {
		TUTRACE((TUTRACE_NFC, "invalid parameters\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	oobDevPwd.ip_devPwd = hex_pin;
	oobDevPwd.devPwdLength = hex_pin_len;
	tlv_oobDevPwdWrite(&oobDevPwd, bufObj);

	/* Version2 */
	if (info->version2 >= WPS_VERSION2) {
		if ((vendorExt_bufObj = buffobj_new()) == NULL) {
			TUTRACE((TUTRACE_NFC, "Out of memory\n"));
			ret = WPS_ERR_OUTOFMEMORY;
			goto error;
		}

		subtlv_serialize(WPS_WFA_SUBID_VERSION2, vendorExt_bufObj,
			&info->version2, WPS_WFA_SUBID_VERSION2_S);

		/* Serialize subelemetns to Vendor Extension */
		vendorExt.vendorId = (uint8 *)WFA_VENDOR_EXT_ID;
		vendorExt.vendorData = buffobj_GetBuf(vendorExt_bufObj);
		vendorExt.dataLength = buffobj_Length(vendorExt_bufObj);
		tlv_vendorExtWrite(&vendorExt, bufObj);
	}

	/* Check copy back avaliable buf length */
	if (*buflen < buffobj_Length(bufObj)) {
		TUTRACE((TUTRACE_NFC, "In buffer len too short\n"));
		ret = WPS_ERR_SYSTEM;
		goto error;
	}

	/* Copy back data and length */
	*buflen = buffobj_Length(bufObj);
	memcpy(buf, buffobj_GetBuf(bufObj), buffobj_Length(bufObj));

error:
	/* Free local vendorExt_bufObj */
	if (vendorExt_bufObj)
		buffobj_del(vendorExt_bufObj);

	/* Free local bufObj */
	if (bufObj)
		buffobj_del(bufObj);

	return ret;
}

/* Both CHO-S and CHO-R need providing the Public Key Hash */
uint32
nfc_utils_build_cho(DevInfo *info, uint8 *buf, uint *buflen,
#ifdef WFA_WPS_20_NFC_TESTBED
	bool b_fake_pkh,
#endif // endif
	bool b_cho_r)
{
	uint32 ret = WPS_SUCCESS;
	BufferObj *bufObj = NULL, *vendorExt_bufObj = NULL;
	CTlvVendorExt vendorExt;
	CTlvOobDevPwd oobDevPwd;
	uint8 temp[sizeof(uint32)];
	uint16 total_len = 0;
#ifdef WFA_WPS_20_NFC_TESTBED
	uint8 fake_pub_key_hash[SIZE_160_BITS];
#endif // endif

	TUTRACE((TUTRACE_NFC, "Build NFC CHO\n"));

	if (info == NULL || buf == NULL || *buflen == 0) {
		TUTRACE((TUTRACE_NFC, "Invalid arguments\n"));
		return WPS_ERR_SYSTEM;
	}

	if ((bufObj = buffobj_new()) == NULL) {
		TUTRACE((TUTRACE_NFC, "Out of memory\n"));
		return WPS_ERR_OUTOFMEMORY;
	}

	/* Set 2 octets total_len first to shift the bufObj */
	WpsHtonsPtr((uint8 *)&total_len, temp);
	buffobj_Append(bufObj, sizeof(uint16), temp);

	/* OOB Device Password */

	/* Public Key Hash */
	oobDevPwd.publicKeyHash = info->pub_key_hash;
#ifdef WFA_WPS_20_NFC_TESTBED
	if (b_fake_pkh) {
		/* compute correct public key hash of the correct public key and
		 * change at least 1 bit of the computed public key hash and offer
		 * this in the NFC message
		 */
		memcpy(fake_pub_key_hash, info->pub_key_hash, sizeof(fake_pub_key_hash));
		WPS_HexDumpAscii(TUDUMP_NFC, "Correct PKH Is",
			oobDevPwd.publicKeyHash, SIZE_20_BYTES);
		/* NOT first byte (8bits) */
		fake_pub_key_hash[0] = ~fake_pub_key_hash[0];
		oobDevPwd.publicKeyHash = fake_pub_key_hash;
		WPS_HexDumpAscii(TUDUMP_NFC, "Fake PKH Is", oobDevPwd.publicKeyHash, SIZE_20_BYTES);
	}
#endif /* WFA_WPS_20_NFC_TESTBED */

#ifdef WFA_WPS_20_NFC_TESTBED_DEBUG
	WPS_HexDumpAscii(TUDUMP_NFC, "I Build Public Key Hash Is",
		oobDevPwd.publicKeyHash, SIZE_20_BYTES);
#endif // endif
	/* Password ID */
	oobDevPwd.pwdId = WPS_DEVICEPWDID_NFC_CHO;

	oobDevPwd.devPwdLength = 0;
	tlv_oobDevPwdWrite(&oobDevPwd, bufObj);

	/* UUID-E/(CHO-R) or SSID/(CHO-S) */
	if (b_cho_r) {
		tlv_serialize(WPS_ID_UUID_E, bufObj, info->uuid, SIZE_UUID);
		WPS_HexDumpAscii(TUDUMP_NFC, "info->uuid", info->uuid, SIZE_UUID);
	}
	else {
		tlv_serialize(WPS_ID_SSID, bufObj, info->ssid, strlen(info->ssid));
		TUTRACE((TUTRACE_NFC, "info->ssid=%s\n", info->ssid));

		if (info->rfBand != 0) {
			tlv_serialize(WPS_ID_RF_BAND, bufObj, &info->rfBand, WPS_ID_RF_BAND_S);
			TUTRACE((TUTRACE_NFC, "info->rfBand=%d\n", info->rfBand));
		}

		if (info->apchannel != 0) {
			tlv_serialize(WPS_ID_AP_CHANNEL, bufObj, &info->apchannel,
				WPS_ID_AP_CHANNEL_S);
			TUTRACE((TUTRACE_NFC, "info->apchannel=%d\n", info->apchannel));
		}

		if (memcmp(info->macAddr, "\0\0\0\0\0\0", SIZE_MAC_ADDR) != 0) {
			tlv_serialize(WPS_ID_MAC_ADDR, bufObj, info->macAddr, SIZE_MAC_ADDR);
			TUTRACE((TUTRACE_NFC, "info->macAddr=%02x:%02x:%02x:%02x:%02x:%02x\n",
				info->macAddr[0], info->macAddr[1], info->macAddr[2],
				info->macAddr[3], info->macAddr[4], info->macAddr[5]));
		}
	}

	/* Version2 */
	if (info->version2 >= WPS_VERSION2) {
		if ((vendorExt_bufObj = buffobj_new()) == NULL) {
			TUTRACE((TUTRACE_NFC, "Out of memory\n"));
			ret = WPS_ERR_OUTOFMEMORY;
			goto error;
		}

		subtlv_serialize(WPS_WFA_SUBID_VERSION2, vendorExt_bufObj,
			&info->version2, WPS_WFA_SUBID_VERSION2_S);

		/* Serialize subelemetns to Vendor Extension */
		vendorExt.vendorId = (uint8 *)WFA_VENDOR_EXT_ID;
		vendorExt.vendorData = buffobj_GetBuf(vendorExt_bufObj);
		vendorExt.dataLength = buffobj_Length(vendorExt_bufObj);
		tlv_vendorExtWrite(&vendorExt, bufObj);
	}

	/* Check copy back avaliable buf length */
	if (*buflen < buffobj_Length(bufObj)) {
		TUTRACE((TUTRACE_NFC, "In buffer len too short\n"));
		ret = WPS_ERR_SYSTEM;
		goto error;
	}

	/* Set correct 2 octets total_len again */
	total_len = buffobj_Length(bufObj) - sizeof(uint16);
	buffobj_Rewind(bufObj);
	WpsHtonsPtr((uint8 *)&total_len, temp);
	buffobj_Write(bufObj, sizeof(uint16), temp);

	/* Copy back data and length */
	*buflen = buffobj_Length(bufObj);
	memcpy(buf, buffobj_GetBuf(bufObj), buffobj_Length(bufObj));

error:
	/* Free local vendorExt_bufObj */
	if (vendorExt_bufObj)
		buffobj_del(vendorExt_bufObj);

	/* Free local bufObj */
	if (bufObj)
		buffobj_del(bufObj);

	return ret;
}

uint32
nfc_utils_parse_cfg(WpsEnrCred *cred, uint8 *buf, uint buflen)
{
	uint32 ret = WPS_SUCCESS;
	char *cp_data;
	uint16 data16;
	BufferObj *bufObj = NULL;
	BufferObj *vendorExt_bufObj = NULL;
	CSubTlvVersion2 version2;
	CTlvVendorExt vendorExt;
	CTlvCredential *p_tlvCred = NULL;
	CTlvRfBand rfband;
	CTlvApChannel apchannel;
	CTlvMacAddr bssid;

	/* Sanity check */
	if (cred == NULL || buf == NULL || buflen == 0) {
		TUTRACE((TUTRACE_NFC, "Invalid parameters!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	if (buflen < 2) {
		TUTRACE((TUTRACE_NFC, "buf length too short!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	if (buf[0] != 0x10 || buf[1] != 0x0E) {
		TUTRACE((TUTRACE_NFC, "Not a Configuration Token!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	/* Dserial raw data to bufObj */
	bufObj = buffobj_new();
	if (bufObj == NULL) {
		TUTRACE((TUTRACE_NFC, "Malloc fail!\n"));
		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}

	buffobj_dserial(bufObj, buf, buflen);

	/* Configuration */
	/* Parse Credential */
	p_tlvCred = (CTlvCredential *)malloc(sizeof(CTlvCredential));
	if (!p_tlvCred) {
		TUTRACE((TUTRACE_NFC, "Malloc fail!\n"));
		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}
	memset(p_tlvCred, 0, sizeof(CTlvCredential));
	if (tlv_credentialParse(p_tlvCred, bufObj, true)) {
		ret = RPROT_ERR_REQD_TLV_MISSING;
		goto error;
	}

	/* Save CTlvCredential to WpsEnrCred */
	/* Fill in SSID */
	cp_data = (char *)(p_tlvCred->ssid.m_data);
	data16 = cred->ssidLen = p_tlvCred->ssid.tlvbase.m_len;
	strncpy(cred->ssid, cp_data, data16);
	cred->ssid[data16] = '\0';

	/* Fill in keyMgmt */
	if (p_tlvCred->authType.m_data == WPS_AUTHTYPE_SHARED) {
		strncpy(cred->keyMgmt, "SHARED", 6);
		cred->keyMgmt[6] = '\0';
	}
	else  if (p_tlvCred->authType.m_data == WPS_AUTHTYPE_WPAPSK) {
		strncpy(cred->keyMgmt, "WPA-PSK", 7);
		cred->keyMgmt[7] = '\0';
	}
	else if (p_tlvCred->authType.m_data == WPS_AUTHTYPE_WPA2PSK) {
		strncpy(cred->keyMgmt, "WPA2-PSK", 8);
		cred->keyMgmt[8] = '\0';
	}
	else if (p_tlvCred->authType.m_data == (WPS_AUTHTYPE_WPAPSK | WPS_AUTHTYPE_WPA2PSK)) {
		strncpy(cred->keyMgmt, "WPA-PSK WPA2-PSK", 16);
		cred->keyMgmt[16] = '\0';
	}
	else {
		strncpy(cred->keyMgmt, "OPEN", 4);
		cred->keyMgmt[4] = '\0';
	}

	/* get the real cypher */
	cred->encrType = p_tlvCred->encrType.m_data;

	/* Fill in WEP index, no matter it's WEP type or not */
	cred->wepIndex = p_tlvCred->WEPKeyIndex.m_data;

	/* Fill in PSK */
	data16 = p_tlvCred->nwKey.tlvbase.m_len;
	memset(cred->nwKey, 0, SIZE_64_BYTES);
	memcpy(cred->nwKey, p_tlvCred->nwKey.m_data, data16);
	/* we have to set key length here */
	cred->nwKeyLen = data16;

	/* WSC 2.0,  nwKeyShareable */
	cred->nwKeyShareable = p_tlvCred->nwKeyShareable.m_data;

	/* (Optionally) Add RF-Band, AP-Channel and BSSID attribute at v2.0.2.1-rev21 */
	/* Unused, just print it */
	if (tlv_find_dserialize(&rfband, WPS_ID_RF_BAND, bufObj, 0, 0) == 0) {
		TUTRACE((TUTRACE_NFC, "RF-Band %d\n", rfband.m_data));
	}

	if (tlv_dserialize(&apchannel, WPS_ID_AP_CHANNEL, bufObj, 0, 0) == 0) {
		TUTRACE((TUTRACE_NFC, "Channel %d\n", apchannel.m_data));
	}

	if (tlv_dserialize(&bssid, WPS_ID_MAC_ADDR, bufObj, 0, 0) == 0) {
		TUTRACE((TUTRACE_NFC, "BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
			bssid.m_data[0], bssid.m_data[1], bssid.m_data[2],
			bssid.m_data[3], bssid.m_data[4], bssid.m_data[5]));
	}

	/* Version2 */
	if (tlv_find_vendorExtParse(&vendorExt, bufObj, (uint8 *)WFA_VENDOR_EXT_ID) != 0) {
		TUTRACE((TUTRACE_NFC, "Cannot find vendor extension\n"));
		/* ret = RPROT_ERR_INCOMPATIBLE; */
		goto error;
	}

	/* Deserialize subelement */
	if ((vendorExt_bufObj = buffobj_new()) == NULL) {
		TUTRACE((TUTRACE_NFC, "Fail to allocate vendor extension buffer, "
			"Out of memory\n"));

		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}

	buffobj_dserial(vendorExt_bufObj, vendorExt.vendorData,
		vendorExt.dataLength);

	/* Get Version2 subelement */
	if ((buffobj_NextSubId(vendorExt_bufObj) != WPS_WFA_SUBID_VERSION2) ||
	    (subtlv_dserialize(&version2, WPS_WFA_SUBID_VERSION2, vendorExt_bufObj, 0, 0) != 0)) {
		TUTRACE((TUTRACE_NFC, "Cannot get Version2\n"));
		/* ret = RPROT_ERR_INCOMPATIBLE; */
		goto error;
	}

	if (version2.m_data < WPS_VERSION2) {
		TUTRACE((TUTRACE_NFC, "Invalid Version2 number\n"));
		ret = RPROT_ERR_INCOMPATIBLE;
		goto error;
	}

error:
	/* free local alloc pointers */
	if (vendorExt_bufObj)
		buffobj_del(vendorExt_bufObj);

	if (p_tlvCred)
		tlv_credentialDelete(p_tlvCred, 0);

	if (bufObj)
		buffobj_del(bufObj);

	return ret;
}

uint32
nfc_utils_parse_pw(WpsOobDevPw *devpw, uint8 *buf, uint buflen)
{
	uint32 ret = WPS_SUCCESS;
	BufferObj *bufObj = NULL;
	BufferObj *vendorExt_bufObj = NULL;
	CSubTlvVersion2 version2;
	CTlvVendorExt vendorExt;
	CTlvOobDevPwd *p_oobDevPwd = NULL;

	/* Sanity check */
	if (devpw == NULL || buf == NULL || buflen == 0) {
		TUTRACE((TUTRACE_NFC, "Invalid parameters!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	if (buflen < 2) {
		TUTRACE((TUTRACE_NFC, "buf length too short!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	if (buf[0] != 0x10 || buf[1] != 0x2C) {
		TUTRACE((TUTRACE_NFC, "Not a Password Token!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	/* Dserial raw data to bufObj */
	bufObj = buffobj_new();
	if (bufObj == NULL) {
		TUTRACE((TUTRACE_NFC, "Malloc fail!\n"));
		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}

	buffobj_dserial(bufObj, buf, buflen);

	/* OOB Device Password */
	p_oobDevPwd = (CTlvOobDevPwd *)malloc(sizeof(CTlvOobDevPwd));
	if (!p_oobDevPwd) {
		TUTRACE((TUTRACE_NFC, "Malloc fail!\n"));
		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}
	memset(p_oobDevPwd, 0, sizeof(CTlvOobDevPwd));
	if (tlv_oobDevPwdParse(p_oobDevPwd, bufObj) != 0) {
		ret = WPS_ERR_SYSTEM;
		goto error;
	}

	/* Save CTlvOobDevPwd to WpsOobDevPw */
	memcpy(devpw->pub_key_hash, p_oobDevPwd->publicKeyHash, SIZE_160_BITS);
	devpw->devPwdId = p_oobDevPwd->pwdId;
	memcpy(devpw->pin, p_oobDevPwd->ip_devPwd, p_oobDevPwd->devPwdLength);
	devpw->pin_len = p_oobDevPwd->devPwdLength;

	/* Version2 */
	if (tlv_find_vendorExtParse(&vendorExt, bufObj, (uint8 *)WFA_VENDOR_EXT_ID) != 0) {
		TUTRACE((TUTRACE_NFC, "Cannot find vendor extension\n"));
		/* ret = RPROT_ERR_INCOMPATIBLE; */
		goto error;
	}

	/* Deserialize subelement */
	if ((vendorExt_bufObj = buffobj_new()) == NULL) {
		TUTRACE((TUTRACE_NFC, "Fail to allocate "
			"vendor extension buffer, Out of memory\n"));

		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}

	buffobj_dserial(vendorExt_bufObj, vendorExt.vendorData,
		vendorExt.dataLength);

	/* Get Version2 subelement */
	if ((buffobj_NextSubId(vendorExt_bufObj) != WPS_WFA_SUBID_VERSION2) ||
	    (subtlv_dserialize(&version2, WPS_WFA_SUBID_VERSION2, vendorExt_bufObj,
	     0, 0) != 0)) {
		TUTRACE((TUTRACE_NFC, "Cannot get Version2\n"));
		/* ret = RPROT_ERR_INCOMPATIBLE; */
		goto error;
	}

	if (version2.m_data < WPS_VERSION2) {
		TUTRACE((TUTRACE_NFC, "Invalid Version2 number\n"));
		ret = RPROT_ERR_INCOMPATIBLE;
		goto error;
	}

error:
	/* free local alloc pointers */
	if (vendorExt_bufObj)
		buffobj_del(vendorExt_bufObj);

	if (p_oobDevPwd)
		free(p_oobDevPwd);

	if (bufObj)
		buffobj_del(bufObj);

	return ret;
}

uint32
nfc_utils_parse_cho(WpsCho *wpscho, WPS_SCSTATE type, uint8 *buf, uint buflen)
{
	uint32 ret = WPS_SUCCESS;
	BufferObj *bufObj = NULL;
	BufferObj *vendorExt_bufObj = NULL;
	WpsChoMsg ChoMsg;
	WpsOobDevPw *devpw;
	CTlvOobDevPwd *p_oobDevPwd = NULL;
	uint16 total_len = 0;

	/* Sanity check */
	if (wpscho == NULL || buf == NULL || buflen == 0) {
		TUTRACE((TUTRACE_NFC, "Invalid parameters!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	/* First 2 bytes is Length of WSC attribute field */
	if (buflen < 4) {
		TUTRACE((TUTRACE_NFC, "buf length too short!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	if (buf[2] != 0x10 || buf[3] != 0x2C) {
		TUTRACE((TUTRACE_NFC, "Not a Password Token!\n"));
		ret = WPS_ERR_INVALID_PARAMETERS;
		goto error;
	}

	/* Dserial raw data to bufObj */
	bufObj = buffobj_new();
	if (bufObj == NULL) {
		TUTRACE((TUTRACE_NFC, "Malloc fail!\n"));
		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}

	TUTRACE((TUTRACE_NFC, "CHO buffer length is %d\n", buflen));

	/* Get Length of WSC attributes */
	total_len = WpsNtohs(buf);
	TUTRACE((TUTRACE_NFC, "Length of WSC attributes %d\n", total_len));

	/* Only dserial length of WSC attributes */
	TUTRACE((TUTRACE_NFC, "Only parse %d bytes data\n", total_len));
	buffobj_dserial(bufObj, buf + sizeof(uint16), total_len);

	memset(&ChoMsg, 0, sizeof(ChoMsg));
	p_oobDevPwd = &ChoMsg.OobDevPw;
	devpw = &wpscho->oobdevpw;

	/* OOB Device Password */
	if (tlv_oobDevPwdParse(p_oobDevPwd, bufObj) != 0) {
		ret = WPS_ERR_SYSTEM;
		goto error;
	}
	/* Save CTlvOobDevPwd to WpsOobDevPw */
	memcpy(devpw->pub_key_hash, p_oobDevPwd->publicKeyHash, SIZE_160_BITS);
	devpw->devPwdId = p_oobDevPwd->pwdId;
	/* Ignore pin and force the ip_devPwd "NFC_CHO" to indicate CHO */
	strcpy((char *)devpw->pin, "NFC_CHO");
	devpw->pin_len = strlen((char *)devpw->pin);

	if (type == WPS_NFC_HO_S) {
		/* I'm CHO-S parse UUID-E in CHO Request */
		if (tlv_dserialize(&ChoMsg.uuid, WPS_ID_UUID_E, bufObj, SIZE_UUID, 0) != 0) {
			TUTRACE((TUTRACE_NFC,
				"Cann't parse UUID-E attribute in CHO-R message!\n"));
			ret = WPS_ERR_SYSTEM;
			goto error;
		}

		memcpy(wpscho->uuid, ChoMsg.uuid.m_data, SIZE_UUID);
	}
	else {
		/* I'm CHO-R parse SSID in CHO Select */
		if (tlv_dserialize(&ChoMsg.ssid, WPS_ID_SSID, bufObj, SIZE_32_BYTES, 0) != 0) {
			TUTRACE((TUTRACE_NFC, "Cann't parse SSID attribute in CHO-S message!\n"));
			ret = WPS_ERR_SYSTEM;
			goto error;
		}
		strncpy(wpscho->ssid, (char *)ChoMsg.ssid.m_data, ChoMsg.ssid.tlvbase.m_len);
		wpscho->ssid[ChoMsg.ssid.tlvbase.m_len] = '\0';

		/* (Optionally) Add RF-Band, AP-Channel and BSSID attribute at v2.0.2.1-rev10 */
		wpscho->rfband = 0; /* 0 means not avaliable */
		if (tlv_find_dserialize(&ChoMsg.rfband, WPS_ID_RF_BAND, bufObj, 0, 0) == 0)
			wpscho->rfband = ChoMsg.rfband.m_data;

		wpscho->apchannel = 0; /* 0 means not avaliable */
		if (tlv_dserialize(&ChoMsg.apchannel, WPS_ID_AP_CHANNEL, bufObj, 0, 0) == 0)
			wpscho->apchannel = ChoMsg.apchannel.m_data;

		memset(wpscho->bssid, 0, SIZE_6_BYTES);
		if (tlv_dserialize(&ChoMsg.bssid, WPS_ID_MAC_ADDR, bufObj, 0, 0) == 0)
			memcpy(wpscho->bssid, ChoMsg.bssid.m_data, SIZE_6_BYTES);
	}

	/* Version2 */
	if (tlv_find_vendorExtParse(&ChoMsg.vendorExt, bufObj, (uint8 *)WFA_VENDOR_EXT_ID) != 0) {
		TUTRACE((TUTRACE_NFC, "Pase WPS PW: Cannot find vendor extension\n"));
		/* ret = RPROT_ERR_INCOMPATIBLE; */
		goto error;
	}

	/* Deserialize subelement */
	if ((vendorExt_bufObj = buffobj_new()) == NULL) {
		TUTRACE((TUTRACE_NFC, "Pase WPS PW: Fail to allocate "
			"vendor extension buffer, Out of memory\n"));

		ret = WPS_ERR_OUTOFMEMORY;
		goto error;
	}

	buffobj_dserial(vendorExt_bufObj, ChoMsg.vendorExt.vendorData,
		ChoMsg.vendorExt.dataLength);

	/* Get Version2 subelement */
	if ((buffobj_NextSubId(vendorExt_bufObj) != WPS_WFA_SUBID_VERSION2) ||
	    (subtlv_dserialize(&ChoMsg.version2, WPS_WFA_SUBID_VERSION2, vendorExt_bufObj,
	     0, 0) != 0)) {
		TUTRACE((TUTRACE_NFC, "Pase WPS PW: Cannot get Version2\n"));
		/* ret = RPROT_ERR_INCOMPATIBLE; */
		goto error;
	}

	if (ChoMsg.version2.m_data < WPS_VERSION2) {
		TUTRACE((TUTRACE_NFC, "Pase WPS PW: Invalid Version2 number\n"));
		ret = RPROT_ERR_INCOMPATIBLE;
		goto error;
	}

error:
	/* free local alloc pointers */
	if (vendorExt_bufObj)
		buffobj_del(vendorExt_bufObj);

	if (bufObj)
		buffobj_del(bufObj);

	return ret;
}
