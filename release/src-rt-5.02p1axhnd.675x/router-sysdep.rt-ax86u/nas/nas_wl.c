/*
 * Broadcom 802.11 device interface
 *
 * Copyright 2019 Broadcom
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
 * $Id: nas_wl.c 764254 2018-05-24 10:59:55Z $
 */

#include <typedefs.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <bcmutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <bcmendian.h>

#include <nas.h>

#define BUFFER_SIZE		256
#define HSPOT_WNM_TYPE		1
#define ACTION_FRAME_SIZE 1800
#define HSPOT_WNM_SUBSCRIPTION_REMEDIATION		0x00
#define HSPOT_WNM_DEAUTHENTICATION_IMMINENT		0x01
#define WL_WIFI_AF_PARAMS_SIZE sizeof(struct wl_af_params)

static bool g_swap = FALSE;
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))

typedef struct
{
	int maxLength;
	int length;
	uint8 *buf;
} wnm_encode_t;

#define bcm_encode_length(pkt)	\
	((pkt)->length)

#define bcm_encode_buf(pkt)		\
	((pkt)->buf)

static int isLengthValid(wnm_encode_t *pkt, int length)
{
	/* assert(pkt != 0); */
	if (pkt == 0)
		return FALSE;

	if (pkt->buf == 0)
		return FALSE;

	if (pkt->length + length > pkt->maxLength) {
		printf("length %d exceeds remaining buffer %d\n",
			length, pkt->maxLength - pkt->length);
		return 0;
	}

	return 1;
}

int bcm_encode_byte(wnm_encode_t *pkt, uint8 byte)
{
	/* assert(pkt != 0); */
	if (!isLengthValid(pkt, 1))
		return 0;

	pkt->buf[pkt->length++] = byte;
	return 1;
}

int bcm_encode_bytes(wnm_encode_t *pkt, int length, uint8 *bytes)
{
	/* assert(pkt != 0); */
	/* assert(bytes != 0); */
	if (!isLengthValid(pkt, length))
		return 0;

	memcpy(&pkt->buf[pkt->length], bytes, length);
	pkt->length += length;
	return length;
}

int bcm_encode_le16(wnm_encode_t *pkt, uint16 value)
{
	/* assert(pkt != 0); */

	if (!isLengthValid(pkt, 2))
		return 0;

	pkt->buf[pkt->length++] = value;
	pkt->buf[pkt->length++] = value >> 8;
	return 2;
}

int bcm_encode_init(wnm_encode_t *pkt, int maxLength, uint8 *buf)
{
	/* assert(pkt != 0); */
	if (buf == 0)
		return FALSE;

	pkt->maxLength = maxLength;
	pkt->length = 0;
	pkt->buf = buf;

	return TRUE;
}

/* encode WNM-notification request for subscription remediation */
int bcm_encode_wnm_subscription_remediation(wnm_encode_t *pkt,
	uint8 dialogToken, uint8 urlLen, char *url, uint8 serverMethod)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_WNM);
	bcm_encode_byte(pkt, DOT11_WNM_ACTION_NOTFCTN_REQ);
	bcm_encode_byte(pkt, dialogToken);
	bcm_encode_byte(pkt, HSPOT_WNM_TYPE);
	bcm_encode_byte(pkt, DOT11_MNG_VS_ID);
	if (urlLen > 0) {
		bcm_encode_byte(pkt, 6 + urlLen);
	}
	else
		bcm_encode_byte(pkt, 5);
	bcm_encode_bytes(pkt, WFA_OUI_LEN, (uint8 *)WFA_OUI);
	bcm_encode_byte(pkt, HSPOT_WNM_SUBSCRIPTION_REMEDIATION);
	bcm_encode_byte(pkt, urlLen);
	if (urlLen > 0) {
		bcm_encode_bytes(pkt, urlLen, (uint8 *)url);
		bcm_encode_byte(pkt, serverMethod);
	}

	return bcm_encode_length(pkt) - initLen;
}

/* encode WNM-notification request for deauthentication imminent */
int bcm_encode_wnm_deauthentication_imminent(wnm_encode_t *pkt,
	uint8 dialogToken, uint8 reason, uint16 reauthDelay, uint8 urlLen, char *url)
{
	int initLen = bcm_encode_length(pkt);

	bcm_encode_byte(pkt, DOT11_ACTION_CAT_WNM);
	bcm_encode_byte(pkt, DOT11_WNM_ACTION_NOTFCTN_REQ);
	bcm_encode_byte(pkt, dialogToken);
	bcm_encode_byte(pkt, HSPOT_WNM_TYPE);
	bcm_encode_byte(pkt, DOT11_MNG_VS_ID);
	bcm_encode_byte(pkt, 8 + urlLen);
	bcm_encode_bytes(pkt, WFA_OUI_LEN, (uint8 *)WFA_OUI);
	bcm_encode_byte(pkt, HSPOT_WNM_DEAUTHENTICATION_IMMINENT);
	bcm_encode_byte(pkt, reason);
	bcm_encode_le16(pkt, reauthDelay);
	bcm_encode_byte(pkt, urlLen);
	if (urlLen > 0) {
		bcm_encode_bytes(pkt, urlLen, (uint8 *)url);
	}

	return bcm_encode_length(pkt) - initLen;
}

int wl_actframe(nas_t *nas, int bsscfg_idx, uint32 packet_id,
	uint32 channel, int32 dwell_time,
	struct ether_addr *BSSID, struct ether_addr *da,
	uint16 len, uint8 *data)
{
	wl_action_frame_t * action_frame;
	wl_af_params_t * af_params;
	struct ether_addr *bssid;
	int err = 0;

	if (len > ACTION_FRAME_SIZE)
		return -1;

	if ((af_params = (wl_af_params_t *) malloc(WL_WIFI_AF_PARAMS_SIZE)) == NULL) {
		return -1;
	}
	action_frame = &af_params->action_frame;

	/* Add the packet Id */
	action_frame->packetId = packet_id;

	memcpy(&action_frame->da, (char*)da, ETHER_ADDR_LEN);

	/* set default BSSID */
	bssid = da;
	if (BSSID != 0)
		bssid = BSSID;
	memcpy(&af_params->BSSID, (char*)bssid, ETHER_ADDR_LEN);

	action_frame->len = htod16(len);
	af_params->channel = htod32(channel);
	af_params->dwell_time = htod32(dwell_time);
	memcpy(action_frame->data, data, len);

	/* Calculate Lengths of Action Frame and af_param */
	int act_frame_len = sizeof(action_frame->da) + sizeof(action_frame->len)
		+ sizeof(action_frame->packetId) +  action_frame->len;
	int af_param_len = act_frame_len + sizeof(af_params->channel)
		+ sizeof(af_params->dwell_time)	+ sizeof(af_params->BSSID) + 2;

	char smbuf[WLC_IOCTL_MEDLEN] = {0};
	err =  wl_iovar_setbuf(nas->interface, "actframe", af_params,
		af_param_len, smbuf, sizeof(smbuf));

	free(af_params);
	return (err);
}

int
wl_wnm_url(char *ifname, uchar datalen, uchar *url_data)
{
	wnm_url_t *url;
	int err;
	uchar *data;
	uchar count;
	count = sizeof(wnm_url_t) + datalen - 1;
	data = malloc(count);
	if (data == NULL) {
		fprintf(stderr, "memory alloc failure\n");
		return -1;
	}

	url = (wnm_url_t *) data;
	url->len = datalen;
	if (datalen > 0) {
		memcpy(url->data, url_data, datalen);
	}

	err = wl_iovar_set(ifname, "wnm_url", data, count);
	free(data);

	return (err);
}

int
wl_wnm_bsstrans_req(char *ifname, uint8 reqmode, uint16 tbtt, uint16 dur, uint8 unicast)
{
	char *cmd = "wnm_bsstrans_req";

	wl_bsstrans_req_t bsstrans_req;

	memset(&bsstrans_req, 0, sizeof(bsstrans_req));

	bsstrans_req.tbtt = tbtt;
	bsstrans_req.dur = dur;
	bsstrans_req.reqmode = reqmode;
	bsstrans_req.unicast = unicast;

	return wl_iovar_set(ifname, cmd, &bsstrans_req, sizeof(bsstrans_req));
}

static int
nas_send_wnm_subscription_remediation(nas_t *nas, char* url,
	struct ether_addr *ea, uint8 serverMethod)
{
	/* struct ether_addr bssid; */
	wnm_encode_t enc;
	uint8 buffer[WLC_IOCTL_SMLEN];
	uint8 dialogtoken = 1;
	int urlLength = url ? strlen(url) : 0;

	if (urlLength > 255) {
		printf("<url> too long");
		return 0;
	}

	bcm_encode_init(&enc, sizeof(buffer), buffer);

	bcm_encode_wnm_subscription_remediation(&enc,
		dialogtoken, urlLength, url, serverMethod);

	/* send action frame */
	wl_actframe(nas, -1,
		(uintptr_t)bcm_encode_buf(&enc), 0, 250, &nas->ea, ea,
		bcm_encode_length(&enc), bcm_encode_buf(&enc));

	return 1;
}

static int
nas_send_wnm_deauthentication_imminent(nas_t *nas, uint8 reason,
	int16 reauthDelay, char* url, struct ether_addr *ea)
{
	/* struct ether_addr bssid; */
	wnm_encode_t enc;
	uint8 buffer[WLC_IOCTL_SMLEN];
	uint8 dialogtoken = 1;
	int urlLength = url ? strlen(url) : 0;

	if (urlLength > 255) {
		printf("<url> too long");
		return 0;
	}

	bcm_encode_init(&enc, sizeof(buffer), buffer);

	bcm_encode_wnm_deauthentication_imminent(&enc,
		dialogtoken, reason, reauthDelay, urlLength, url);

	/* send action frame */
	wl_actframe(nas, -1,
		(uintptr_t)bcm_encode_buf(&enc), 0, 250, &nas->ea, ea,
		bcm_encode_length(&enc), bcm_encode_buf(&enc));

	return 1;
}

static int
nas_send_bss_transition_mgmt_req(nas_t *nas, char* url,
	uint8 reqmode, uint16 tbtt, uint16 dur, uint8 unicast)
{
	int urlLength = url ? strlen(url) : 0;

	if (urlLength > 255) {
		printf("<url> too long");
		return 0;
	}

	if (wl_wnm_url(nas->interface, urlLength, (uchar *)url) < 0) {
		printf("wl_wnm_url failed\n");
		return 0;
	}

	/* Sending BSS Transition Request Frame */
	if (wl_wnm_bsstrans_req(nas->interface,
		reqmode, tbtt, dur, unicast) < 0)
	{
		printf("wl_wnm_bsstrans_req failed\n");
		return 0;
	}

	return 1;
}

int
nas_send_wnm_notifications(nas_t *nas, struct ether_addr *ea)
{
	/* send WNM Notification for Deauthentication Imminent Action Frame to STA */
	if (nas->m_deauth_params.deauth_req) {
		nas->m_deauth_params.deauth_req = 0;

		dbg(nas, "Deauthentication Request URL Code = %d",
			nas->m_deauth_params.reason_code);
		dbg(nas, "Deauthentication Reauth Delay = %d",
			nas->m_deauth_params.reauth_delay);
		dbg(nas, "Deauthentication Request URL = %s",
			nas->m_deauth_params.reason_url);

		nas_send_wnm_deauthentication_imminent(nas, nas->m_deauth_params.reason_code,
			nas->m_deauth_params.reauth_delay, nas->m_deauth_params.reason_url, ea);

		if (nas->m_deauth_params.reason_url) {
			free(nas->m_deauth_params.reason_url);
			nas->m_deauth_params.reason_url = NULL;
		}
		memset(&nas->m_deauth_params, 0, sizeof(deauth_params_t));
	}

	/* send WNM Notification for Subscription Remediation Action Frame to STA */
	if (nas->m_subrem_params.subrem_req) {
		nas->m_subrem_params.subrem_req = 0;

		dbg(nas, "Subscription Remediation URL = %s",
			nas->m_subrem_params.subrem_url);

		nas_send_wnm_subscription_remediation(nas, nas->m_subrem_params.subrem_url, ea,
			nas->m_subrem_params.serverMethod);

		if (nas->m_subrem_params.subrem_url) {
			free(nas->m_subrem_params.subrem_url);
			nas->m_subrem_params.subrem_url = NULL;
		}
		memset(&nas->m_subrem_params, 0, sizeof(subrem_params_t));
	}

	/* send BSS Transition Request Frame to STA */
	if (nas->m_bsstran_params.bsstran_req) {
		nas->m_bsstran_params.bsstran_req = 0;

		dbg(nas, "BSS Transition Session Warning Timer = %d",
			nas->m_bsstran_params.bsstran_swt);
		dbg(nas, "BSS Transition Request Mode = %d",
			nas->m_bsstran_params.bsstran_reqmode);
		dbg(nas, "BSS Transition Session Info URL = %s",
			nas->m_bsstran_params.bsstran_url);

		nas_send_bss_transition_mgmt_req(nas, nas->m_bsstran_params.bsstran_url,
		nas->m_bsstran_params.bsstran_reqmode, nas->m_bsstran_params.bsstran_swt * 600,
		nas->m_bsstran_params.bsstran_dur, TRUE);

		if (nas->m_bsstran_params.bsstran_url) {
			free(nas->m_bsstran_params.bsstran_url);
			nas->m_bsstran_params.bsstran_url = NULL;
		}
		memset(&nas->m_bsstran_params, 0, sizeof(bsstran_params_t));
	}

	return 1;
}
int
nas_authorize(nas_t *nas, struct ether_addr *ea)
{
	int ret = 0;

	ret = wl_ioctl(nas->interface, WLC_SCB_AUTHORIZE, ea, ETHER_ADDR_LEN);

	return ret;
}

int
nas_deauthorize(nas_t *nas, struct ether_addr *ea)
{
	return wl_ioctl(nas->interface, WLC_SCB_DEAUTHORIZE, ea, ETHER_ADDR_LEN);
}

int
nas_deauthenticate(nas_t *nas, struct ether_addr *ea, int reason)
{
	scb_val_t scb_val;

	/* remove the key if one is installed */
	nas_set_key(nas, ea, NULL, 0, 0, 0, 0, 0, 0);
	scb_val.val = (uint32) reason;
	memcpy(&scb_val.ea, ea, ETHER_ADDR_LEN);
	return wl_ioctl(nas->interface, WLC_SCB_DEAUTHENTICATE_FOR_REASON,
	                &scb_val, sizeof(scb_val));
}

int
nas_get_group_rsc(nas_t *nas, uint8 *buf, int index)
{
	union {
		int index;
		uint8 rsc[EAPOL_WPA_KEY_RSC_LEN];
	} u;

	u.index = index;
	if (wl_ioctl(nas->interface, WLC_GET_KEY_SEQ, &u, sizeof(u)) != 0)
		return -1;

	bcopy(u.rsc, buf, EAPOL_WPA_KEY_RSC_LEN);

	return 0;
}

/* XXX nas_set_key must be changed to specify the algo, rather than expecting
 * the driver to infer it based on key length. With 256 bit keys and potentially
 * different algorithms (CCM, GCM) this is difficult, if not impossible for the
 * driver.
 */
int
nas_set_key(nas_t *nas, struct ether_addr *ea, uint8 *key, int len, int index,
            int tx, uint32 hi, uint16 lo, int force_init_iv)
{
	wl_wsec_key_t wep;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	char ki[] = "index XXXXXXXXXXX";

	memset(&wep, 0, sizeof(wep));
	wep.index = index;
	if (ea)
		memcpy(&wep.ea, ea, ETHER_ADDR_LEN);
	else {
		wep.flags = tx ? WL_PRIMARY_KEY : 0;
		snprintf(ki, sizeof(ki), "index %d", index);
	}

	wep.len = MIN(len, sizeof(wep.data));
	if (len) {
		memcpy(wep.data, key, wep.len);
	}
	dbg(nas, "%s, flags %x, len %d",
		ea ? (char *)ether_etoa((uchar *)ea, eabuf) : ki,
		wep.flags, wep.len);
	if (lo || hi) {
		wep.rxiv.hi = hi;
		wep.rxiv.lo = lo;
		wep.iv_initialized = 1;
	}

	if (force_init_iv)
		wep.iv_initialized = 1;

	return wl_ioctl(nas->interface, WLC_SET_KEY, &wep, sizeof(wep));
}

int
nas_wl_tkip_countermeasures(nas_t *nas, int enable)
{
	return wl_ioctl(nas->interface, WLC_TKIP_COUNTERMEASURES, &enable, sizeof(int));
}

int
nas_set_ssid(nas_t *nas, char *ssid)
{
	wlc_ssid_t wl_ssid;
	int  len = strlen(ssid);
	strncpy((char *)wl_ssid.SSID, ssid, sizeof(wl_ssid.SSID));
	wl_ssid.SSID_len = (len >= sizeof(wl_ssid.SSID))? sizeof(wl_ssid.SSID): len;
	return wl_ioctl(nas->interface, WLC_SET_SSID, &wl_ssid, sizeof(wl_ssid));
}

int
nas_disassoc(nas_t *nas)
{
	return wl_ioctl(nas->interface, WLC_DISASSOC, NULL, 0);
}

/* get current encryption algo bitvec */
int
nas_get_wpawsec(nas_t *nas, uint32 *wsec)
{
	int err;
	int32 val;

	err = wl_iovar_getint(nas->interface, "wsec", &val);
	if (!err)
		*wsec = val;

	return err;
}

/* get current WPA auth mode */
int
nas_get_wpaauth(nas_t *nas, uint32 *wpa_auth)
{
	int err;
	int32 val;

	err = wl_iovar_getint(nas->interface, "wpa_auth", &val);
	if (!err)
		*wpa_auth = val;

	return err;
}

/* get WPA capabilities */
int
nas_get_wpacap(nas_t *nas, uint8 *cap)
{
	int err;
	int cap_val;

	err = wl_iovar_getint(nas->interface, "wpa_cap", &cap_val);
	if (!err) {
		cap[0] = (cap_val & 0xff);
		cap[1] = ((cap_val >> 8) & 0xff);
	}

	return err;
}
/* get STA info */
int
nas_get_stainfo(nas_t *nas, char *macaddr, int len, char *ret_buf, int ret_buf_len)
{
	char *tmp_ptr;
	tmp_ptr = ret_buf;
	strcpy(ret_buf, "sta_info");
	tmp_ptr += strlen(ret_buf);
	tmp_ptr++;
	memcpy(tmp_ptr, macaddr, len);

	return wl_ioctl(nas->interface, WLC_GET_VAR, ret_buf, ret_buf_len);
}

int nas_get_assoc_req_ies(nas_t *nas, char *buf, uint32 size)
{
	if (nas == NULL || buf == NULL)
	{
		return -1;
	}
	return  wl_iovar_getbuf(nas->interface, "assoc_req_ies", &nas->ea, 6, buf, size);
}

int
nas_get_wpa_ie(nas_t *nas, char *ret_buf, int ret_buf_len, uint32 sta_mode)
{
	int err, wpa_len = 0, ie_len = 0;
	char buf[WLC_IOCTL_SMLEN];
	bcm_tlv_t *ie_getbuf;
	char *buf_ptr;
	int i;

	if (ret_buf == NULL)
		return 0;

	memset(buf, 0, sizeof(buf));

	err = wl_iovar_getbuf(nas->interface, "wpaie", &nas->ea, 6, buf, sizeof(buf));
	if (err != 0)
		return 0;

	buf_ptr = buf;
	for (i = 0; i < 2; i++) {
		ie_getbuf = (bcm_tlv_t *)buf_ptr;
		wpa_len = ie_getbuf->len;
		if (wpa_len == 0)
			break;

		if (ie_getbuf->id == DOT11_MNG_RSN_ID) {
			dbg(nas, "found RSN IE of length %d\n", wpa_len);
			if (sta_mode == WPA || sta_mode == WPA_PSK) {
				buf_ptr += wpa_len + TLV_HDR_LEN;
				continue;
			}
		}
		else if (ie_getbuf->id != DOT11_MNG_RSN_ID) {
			dbg(nas, "found WPA IE of length %d\n", wpa_len);
			if (sta_mode == WPA2 || sta_mode == WPA2_PSK || sta_mode == WPA2_FT) {
				buf_ptr += wpa_len + TLV_HDR_LEN;
				continue;
			}
		}

		ie_len = wpa_len + TLV_HDR_LEN;
		/* Got WPA ie */
		if (ie_len <= ret_buf_len) {
			memcpy(ret_buf, buf_ptr, wpa_len + TLV_HDR_LEN);
			return ie_len;
		}
		else {
			dbg(nas, "Not enough buffer (%d) to copy %d bytes wpa ie\n",
			    ret_buf_len, ie_len);
			return 0;
		}
	}

	return 0;
}

int
nas_send_brcm_event(nas_t *nas, uint8* mac, int reason)
{
	int err;
	scb_val_t scb_val;
	scb_val.val = (uint32) reason;
	memcpy(&scb_val.ea, mac, ETHER_ADDR_LEN);
	err = wl_iovar_set(nas->interface, "nas_notify_event", &scb_val, sizeof(scb_val));
	if (err) {
		printf("set nas_notify_event iovar error = %d, name %s\n", err, nas->interface);
	}
	return err;
}

int
nas_get_ssid(nas_t *nas, uint8 *ssid_ptr, uint32 *ssid_len)
{
	char buf[WLC_IOCTL_SMLEN];

	if (nas == NULL)
	{
		return -1;
	}

	wl_iovar_getbuf(nas->interface, "ssid", &nas->ea, ETHER_ADDR_LEN, buf, sizeof(buf));
	wlc_ssid_t * ssid = (wlc_ssid_t *) buf;
	if (ssid->SSID_len > 0 && ssid->SSID_len < DOT11_MAX_SSID_LEN - 1) {
		ssid->SSID[ssid->SSID_len] = 0;
		*ssid_len = ssid->SSID_len;
		memcpy(ssid_ptr, ssid->SSID, *ssid_len);
	}
	return 0;
}

int
nas_get_bssid(nas_t *nas, char *buf, int buf_len)
{
	char *tmp_ptr;

	if (nas == NULL || buf == NULL) {
		return -1;
	}

	tmp_ptr = buf;
	strcpy(buf, "bssid");
	tmp_ptr += strlen(buf);
	tmp_ptr++;
	memcpy(tmp_ptr, nas->ea.octet, ETHER_ADDR_LEN);

	return wl_ioctl(nas->interface, WLC_GET_BSSID, buf, buf_len);
}

#ifdef WLHOSTFBT

/* get current FBT MDID */
int
nas_get_fbt_mdid(nas_t *nas, uint16 *mdid)
{
	int err;
	int val;

	err = wl_iovar_getint(nas->interface, "fbt_mdid", &val);
	if (!err)
		*mdid = val;

	return err;
}

int
nas_set_fbt_auth_resp(nas_t *nas, uint8* resp_ies, int resp_ies_len)
{
	int err;
	char mdbuf[WLC_IOCTL_MEDLEN];

	if (nas == NULL || resp_ies == NULL)
	{
		return -1;
	}

	err = wl_iovar_setbuf(nas->interface, "fbt_auth_resp", resp_ies, resp_ies_len,
			mdbuf, sizeof(mdbuf));
	if (err) {
		dbg(nas, "set fbt_auth_resp iovar error %d, ifname %s", err, nas->interface);
	}
	return err;
}

int nas_set_fbt_action(nas_t *nas, uint8 *fbt_act_ies, int fbt_act_ies_len)
{
	int err;
	char mdbuf[WLC_IOCTL_MEDLEN];

	if (nas == NULL || fbt_act_ies == NULL)
	{
		return -1;
	}

	err = wl_iovar_setbuf(nas->interface, "fbt_act_resp", fbt_act_ies, fbt_act_ies_len,
			mdbuf, sizeof(mdbuf));
	if (err) {
		dbg(nas, "set fbt_act_resp iovar error %d, ifname %s", err, nas->interface);
	}

	return err;
}

int nas_set_fbt_ds_add_sta(nas_t *nas, uint8 *fbt_ds_add_sta_ies, int fbt_add_sta_ies_len)
{
	int err;
	char mdbuf[WLC_IOCTL_MEDLEN];

	if (nas == NULL || fbt_ds_add_sta_ies == NULL)
	{
		return -1;
	}

	err = wl_iovar_setbuf(nas->interface, "fbt_ds_add_sta", fbt_ds_add_sta_ies,
			fbt_add_sta_ies_len, mdbuf, sizeof(mdbuf));
	if (err) {
		dbg(nas, "set fbt_act_resp iovar error %d, ifname %s", err, nas->interface);
	}

	return err;
}

/* get current FBT Reassoc timer */
int
nas_get_reassoc_timer(nas_t *nas, uint32 *reassoc_deadline)
{
	int err;
	int val;

	err = wl_iovar_getint(nas->interface, "fbt_reassoc_time", &val);
	if (!err)
		*reassoc_deadline = val;

	return err;
}

int
nas_get_fbt_overds(nas_t *nas, uint32 *fbt_overds)
{
	int err;
	int val;

	err = wl_iovar_getint(nas->interface, "fbtoverds", &val);
	if (!err)
		*fbt_overds = val;

	return err;
}

int nas_get_fbt_r0kh(nas_t *nas, char *buf, uint32 size)
{
	if (nas == NULL || buf == NULL)
	{
		return -1;
	}
	return  wl_iovar_getbuf(nas->interface, "fbt_r0kh_id", &nas->ea, 6, buf, size);
}

int nas_get_fbt_r1kh(nas_t *nas, char *buf, uint32 buf_len)
{
	if (nas == NULL || buf == NULL)
	{
		return -1;
	}
	return  wl_iovar_getbuf(nas->interface, "fbt_r1kh_id", &nas->ea, 6, buf, buf_len);
}

#endif /* WLHOSTFBT */
