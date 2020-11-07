/*
 * WPA/RSN - Shared functions for supplicant and authenticator
 * Copyright (c) 2002-2008, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
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
 *
 * $Id: wpa_common.c 769827 2018-11-28 05:18:16Z $
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <802.11.h>

#include "common.h"
#include "nas.h"
#include "wpa_common.h"

#ifdef WLHOSTFBT
static int wpa_ft_parse_ftie(wpa_t *wpa, const uint8 *ie, size_t ie_len,
		struct wpa_ft_ies *parse)
{
	const uint8 *end, *pos;

	parse->ftie = ie;
	parse->ftie_len = ie_len;

	if ((ie_len + TLV_HDR_LEN) < sizeof(dot11_ft_ie_t)) {
		dbg(wpa->nas, "FT: Invalid FTIE length: %zd", ie_len);
		return -1;
	}

	pos = ie + sizeof(struct rsn_ftie);
	end = ie + ie_len;

	while (pos + 2 <= end && pos + 2 + pos[1] <= end) {
		switch (pos[0]) {
		case FTIE_SUBELEM_R1KH_ID:
			if (pos[1] != FT_R1KH_ID_LEN) {
			dbg(wpa->nas, "FT: Invalid R1KH-ID length in FTIE: %d", pos[1]);
				return -1;
			}
			parse->r1kh_id = pos + 2;
			break;
		case FTIE_SUBELEM_GTK:
			parse->gtk = pos + 2;
			parse->gtk_len = pos[1];
			break;
		case FTIE_SUBELEM_R0KH_ID:
			if (pos[1] < 1 || pos[1] > FT_R0KH_ID_MAX_LEN) {
			dbg(wpa->nas, "FT: Invalid R0KH-ID length in FTIE: %d", pos[1]);
				return -1;
			}
			parse->r0kh_id = pos + 2;
			parse->r0kh_id_len = pos[1];
			break;
#ifdef MFP
		case FTIE_SUBELEM_IGTK:
			parse->igtk = pos + 2;
			parse->igtk_len = pos[1];
			break;
#endif /* MFP */
		}

		pos += 2 + pos[1];
	}

	return 0;
}

int wpa_ft_parse_ies(wpa_t *wpa, const uint8 *ies, size_t ies_len,
		struct wpa_ft_ies *parse, struct wpa_ie_data *rsnie_data)
{
	const uint8 *end, *pos;
	int ret;
	const struct rsn_ftie *ftie;
	int prot_ie_count = 0;

	memset(parse, 0, sizeof(*parse));
	if (ies == NULL)
		return 0;

	pos = ies;
	end = ies + ies_len;
	while (pos + 2 <= end && pos + 2 + pos[1] <= end) {
		switch (pos[0]) {
		case DOT11_MNG_RSN_ID:
			parse->rsn = pos + 2;
			parse->rsn_len = pos[1];
			ret = wpa_parse_wpa_ie_rsn(wpa, parse->rsn - 2,
					parse->rsn_len + 2, rsnie_data);
			if (ret < 0) {
			dbg(wpa->nas, "FT: Failed to parse RSN IE: %d", ret);
				return -1;
			}
			if (rsnie_data->num_pmkid == 1 && rsnie_data->pmkid)
				parse->rsn_pmkid = rsnie_data->pmkid;
			break;
		case DOT11_MNG_MDIE_ID:
			parse->mdie = pos + 2;
			parse->mdie_len = pos[1];
			break;
		case DOT11_MNG_FTIE_ID:
			if (pos[1] < sizeof(*ftie))
				return -1;
			ftie = (const struct rsn_ftie *) (pos + 2);
			prot_ie_count = ftie->mic_control[1];
			if (wpa_ft_parse_ftie(wpa, pos + 2, pos[1], parse) < 0)
				return -1;
			break;
		case DOT11_MNG_FT_TI_ID:
			parse->tie = pos + 2;
			parse->tie_len = pos[1];
			break;
		case DOT11_MNG_RDE_ID:
			if (parse->ric == NULL)
				parse->ric = pos;
			break;
		default:
			dbg(wpa->nas, "Found element ID %d of length %d. Skipping.",
					pos[0], pos[1]);
			break;
		}

		pos += 2 + pos[1];
	}

	if (prot_ie_count == 0)
		return 0; /* no MIC */

	/*
	 * Check that the protected IE count matches with IEs included in the
	 * frame.
	 */
	if (parse->rsn)
		prot_ie_count--;
	if (parse->mdie)
		prot_ie_count--;
	if (parse->ftie)
		prot_ie_count--;
	if (prot_ie_count < 0) {
		dbg(wpa->nas, "FT: Some required IEs not included in "
			   "the protected IE count");
		return -1;
	}

	if (prot_ie_count == 0 && parse->ric) {
		dbg(wpa->nas, "FT: RIC IE(s) in the frame, but not "
			   "included in protected IE count");
		return -1;
	}

	/* Determine the end of the RIC IE(s) */
	pos = parse->ric;
	while (pos && pos + 2 <= end && pos + 2 + pos[1] <= end &&
	       prot_ie_count) {
		prot_ie_count--;
		pos += 2 + pos[1];
	}
	parse->ric_len = pos - parse->ric;
	if (prot_ie_count) {
		dbg(wpa->nas, "FT: %d protected IEs missing from "
			   "frame", (int) prot_ie_count);
		return -1;
	}

	return 0;
}

int wpa_ft_validate_ies(wpa_t *wpa, const uint8 *ies, size_t ies_len,
		struct wpa_ft_ies *parse, struct wpa_ie_data *rsnie_data)
{
	struct rsn_mdie *mdie;
	struct rsn_ftie *ftie;
	uint16 mdid, c_mdid;

	if (wpa_ft_parse_ies(wpa, ies, ies_len, parse, rsnie_data) < 0) {
		dbg(wpa->nas, "FT: Failed to parse FT IEs");
		return DOT11_SC_INVALID_FTIE;
	}

	mdie = (struct rsn_mdie *) parse->mdie;

	if (mdie == NULL || parse->mdie_len < sizeof(*mdie)) {
	      dbg(wpa->nas, "FT: Invalid MDIE");
	      return DOT11_SC_INVALID_MDID;
	}

	mdid = WPA_GET_LE16(mdie->mobility_domain);
	nas_get_fbt_mdid(wpa->nas, &c_mdid);
	if (mdid != c_mdid) {
	      dbg(wpa->nas, "FT: MISMATCH MDIE");
	      dbg(wpa->nas, "Received MDID = %d", mdid);
	      dbg(wpa->nas, "Configured MDID = %d", c_mdid);
	      return DOT11_SC_INVALID_MDID;
	}

	if (parse->rsn_pmkid == NULL) {
		dbg(wpa->nas, "FT: Invalid RSNIE - no PMKID");
		return DOT11_SC_INVALID_PMKID;
	}

	ftie = (struct rsn_ftie *) parse->ftie;
	if (ftie == NULL || parse->ftie_len < sizeof(*ftie)) {
		dbg(wpa->nas, "FT: Invalid FTIE");
		return DOT11_SC_INVALID_FTIE;
	}

	if (parse->r0kh_id == NULL) {
		dbg(wpa->nas, "FT: Invalid FTIE - no R0KH-ID");
		return DOT11_SC_INVALID_FTIE;
	}

	if (parse->r1kh_id == NULL) {
		dbg(wpa->nas, "FT: FTIE - no R1KH-ID");
	}
	return 0;
}
#endif /* WLHOSTFBT */

int rsn_selector_to_bitfield(const uint8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE_BIT;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40_BIT;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP_BIT;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP_BIT;
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104_BIT;
#ifdef MFP
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_AES_128_CMAC)
		return WPA_CIPHER_AES_128_CMAC_BIT;
#endif /* MFP */
	return 0;
}

int rsn_key_mgmt_to_bitfield(const uint8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;
#ifdef WLHOSTFBT
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_FT_802_1X)
		return WPA_KEY_MGMT_FT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_FT_PSK)
		return WPA_KEY_MGMT_FT_PSK;
#endif /* WLHOSTFBT */
#ifdef MFP
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_802_1X_SHA256)
		return WPA_KEY_MGMT_IEEE8021X_SHA256;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_PSK_SHA256)
		return WPA_KEY_MGMT_PSK_SHA256;
#endif /* MFP */
#ifdef CONFIG_SAE
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_SAE)
		return WPA_KEY_MGMT_SAE;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_FT_SAE)
		return WPA_KEY_MGMT_FT_SAE;
#endif /* CONFIG_SAE */
	return 0;
}

/**
 * wpa_parse_wpa_ie_rsn - Parse RSN IE
 * @rsn_ie: Buffer containing RSN IE
 * @rsn_ie_len: RSN IE buffer length (including IE number and length octets)
 * @data: Pointer to structure that will be filled in with parsed data
 * Returns: 0 on success, <0 on failure
 */
int wpa_parse_wpa_ie_rsn(wpa_t *wpa, const uint8 *rsn_ie, size_t rsn_ie_len,
		struct wpa_ie_data *data)
{
	const struct rsn_ie_hdr *hdr;
	const uint8 *pos;
	int left;
	int i, count;

	memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_RSN;
	data->pairwise_cipher = WPA_CIPHER_CCMP_BIT;
	data->group_cipher = WPA_CIPHER_CCMP_BIT;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
#ifdef MFP
	data->mgmt_group_cipher = WPA_CIPHER_AES_128_CMAC_BIT;
#else /* MFP */
	data->mgmt_group_cipher = 0;
#endif /* MFP */

	if (rsn_ie_len == 0) {
		/* No RSN IE - fail silently */
		return -1;
	}

	if (rsn_ie_len < sizeof(struct rsn_ie_hdr)) {
		dbg(wpa->nas, "%s: ie len too short %lu",
				__func__, (unsigned long) rsn_ie_len);
		return -1;
	}

	hdr = (const struct rsn_ie_hdr *) rsn_ie;

	if (hdr->elem_id != DOT11_MNG_RSN_ID ||
			hdr->len != rsn_ie_len - 2 ||
			WPA_GET_LE16(hdr->version) != RSN_VERSION) {
		dbg(wpa->nas, "%s: malformed ie or unknown version", __func__);
		return -2;
	}

	pos = (const uint8 *) (hdr + 1);
	left = rsn_ie_len - sizeof(*hdr);

	if (left >= RSN_SELECTOR_LEN) {
		data->group_cipher = rsn_selector_to_bitfield(pos);
#ifdef MFP
		if (data->group_cipher == WPA_CIPHER_AES_128_CMAC_BIT) {
			dbg(wpa->nas, "%s: AES-128-CMAC used as group cipher", __func__);
			return -1;
		}
#endif /* MFP */
		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	} else if (left > 0) {
		dbg(wpa->nas, "%s: ie length mismatch, %u too much", __func__, left);
		return -3;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			dbg(wpa->nas, "%s: ie count botch (pairwise), "
					"count %u left %u", __func__, count, left);
			return -4;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= rsn_selector_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
#ifdef MFP
		if (data->pairwise_cipher & WPA_CIPHER_AES_128_CMAC_BIT) {
			dbg(wpa->nas, "%s: AES-128-CMAC used as pairwise cipher", __func__);
			return -1;
		}
#endif /* MFP */
	} else if (left == 1) {
		dbg(wpa->nas, "%s: ie too short (for key mgmt)", __func__);
		return -5;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			dbg(wpa->nas, "%s: ie count botch (key mgmt), "
					"count %u left %u", __func__, count, left);
			return -6;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= rsn_key_mgmt_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
		dbg(wpa->nas, "%s: ie too short (for capabilities)", __func__);
		return -7;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left >= 2) {
		data->num_pmkid = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (left < (int) data->num_pmkid * PMKID_LEN) {
			dbg(wpa->nas, "%s: PMKID underflow num_pmkid=%lu left=%d",
					__func__, (unsigned long) data->num_pmkid, left);
			data->num_pmkid = 0;
			return -9;
		} else {
			data->pmkid = pos;
			pos += data->num_pmkid * PMKID_LEN;
			left -= data->num_pmkid * PMKID_LEN;
		}
	}

#ifdef MFP
	if (left >= 4) {
		data->mgmt_group_cipher = rsn_selector_to_bitfield(pos);
		if (data->mgmt_group_cipher != WPA_CIPHER_AES_128_CMAC_BIT) {
			dbg(wpa->nas, "%s: Unsupported management group cipher 0x%x",
					__func__, data->mgmt_group_cipher);
			return -10;
		}
		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	}
#endif /* MFP */

	if (left > 0) {
		dbg(wpa->nas, "%s: ie has %u trailing bytes - ignored", __func__, left);
	}

	return 0;
}

static int wpa_selector_to_bitfield(const uint8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_NONE)
		return WPA_CIPHER_NONE_BIT;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP40)
		return WPA_CIPHER_WEP40_BIT;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_TKIP)
		return WPA_CIPHER_TKIP_BIT;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_CCMP)
		return WPA_CIPHER_CCMP_BIT;
	if (RSN_SELECTOR_GET(s) == WPA_CIPHER_SUITE_WEP104)
		return WPA_CIPHER_WEP104_BIT;
	return 0;
}

int wpa_key_mgmt_to_bitfield(const uint8 *s)
{
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_UNSPEC_802_1X)
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X)
		return WPA_KEY_MGMT_PSK;
	if (RSN_SELECTOR_GET(s) == WPA_AUTH_KEY_MGMT_NONE)
		return WPA_KEY_MGMT_WPA_NONE;
	return 0;
}

int wpa_parse_wpa_ie_wpa(const uint8 *wpa_ie, size_t wpa_ie_len,
		struct wpa_ie_data *data)
{
	const struct wpa_ie_hdr *hdr;
	const uint8 *pos;
	int left;
	int i, count;

	memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_WPA;
	data->pairwise_cipher = WPA_CIPHER_TKIP;
	data->group_cipher = WPA_CIPHER_TKIP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
	data->mgmt_group_cipher = 0;

	if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}

	if (wpa_ie_len < sizeof(struct wpa_ie_hdr)) {
		printf("%s: ie len too short %lu",
				__func__, (unsigned long) wpa_ie_len);
		return -1;
	}

	hdr = (const struct wpa_ie_hdr *) wpa_ie;
	printf("%d", RSN_SELECTOR_GET(hdr->oui));
	printf("%d", WPA_OUI_TP);
	if (hdr->elem_id != DOT11_MNG_VS_ID ||
			hdr->len != wpa_ie_len - 2 ||
			RSN_SELECTOR_GET(hdr->oui) != WPA_OUI_TP ||
			WPA_GET_LE16(hdr->version) != WPA_VERSION) {
		printf("%s: malformed ie or unknown version",
				__func__);
		return -2;
	}

	pos = (const uint8 *) (hdr + 1);
	left = wpa_ie_len - sizeof(*hdr);

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0) {
		printf("%s: ie length mismatch, %u too much",
				__func__, left);
		return -3;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			printf("%s: ie count botch (pairwise), "
					"count %u left %u", __func__, count, left);
			return -4;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		printf("%s: ie too short (for key mgmt)",
				__func__);
		return -5;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			printf("%s: ie count botch (key mgmt), "
					"count %u left %u", __func__, count, left);
			return -6;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		printf("%s: ie too short (for capabilities)",
				__func__);
		return -7;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
		printf("%s: ie has %u trailing bytes - ignored",
				__func__, left);
	}

	return 0;
}

int rsn_cipher_put_suites(uint8 *pos, int ciphers)
{
	int num_suites = 0;

	if (ciphers & WPA_CIPHER_CCMP_BIT) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_CCMP);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (ciphers & WPA_CIPHER_TKIP_BIT) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_TKIP);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}
	if (ciphers & WPA_CIPHER_NONE_BIT) {
		RSN_SELECTOR_PUT(pos, RSN_CIPHER_SUITE_NONE);
		pos += RSN_SELECTOR_LEN;
		num_suites++;
	}

	return num_suites;
}
