/*
 * hostapd - IEEE 802.11r - Fast BSS Transition
 * Copyright (c) 2004-2009, Jouni Malinen <j@w1.fi>
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
 * $Id: wpa_auth_ft.c 769827 2018-11-28 05:18:16Z $
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <aeskeywrap.h>

#include <bcmendian.h>
#include <bcmnvram.h>

#include <nas.h>
#include <wpa.h>
#include <l2_packet.h>

#ifdef WLHOSTFBT
#include <bcmwpa.h>
#include <common.h>
#include <wpa_common.h>
#include <wpa_auth_ft.h>

#define MAX_PEND_REQ	8	/* Maximum pending auth requests that can be stored */

/* To hold pending auth requests incase PMKR1 is not readily available in cache */
struct pend_auth_req_t {
	bool valid;			/* Processed or not */
	bool overds;			/* Over the DS or over the air */
	nas_sta_t *sta;			/* NAS STA instance */
	struct wpa_ft_ies *parse;	/* FT IE */
	uint8 current_ap[ETH_ALEN];	/* Current AP address to which STA is associated.
					 * Used only in case of Over the DS
					 */
	uint8 dest_bssid[ETH_ALEN];	/* Destination AP address. */
	uint8 timestamp[4];		/* To invalidate entries if they are older than
					 * 60 seconds
					 */
} pend_auth_req[MAX_PEND_REQ];

/* Callback funtion which is called when something is recived on RRB socket */
static void broadcom_handle_data(void *ctx, unsigned char *src_addr,
                 unsigned char *buf, size_t len)
{
	nas_wksp_t *nwksp = (nas_wksp_t *)ctx;
	struct l2_ethhdr *ethhdr;
	nas_wpa_cb_t *nwcb;
	int i;

	ethhdr = (struct l2_ethhdr *) buf;
	/* If RRB packet */
	if (ntohs(ethhdr->h_proto) == ETH_P_RRB) {
		buf = (unsigned char *)(ethhdr + 1);
		len = len - sizeof(*ethhdr);
		for (i = 0; i < nwksp->nwcbs; i ++) {
			nwcb = nwksp->nwcb[i];
			wpa_ft_rrb_rx(&nwcb->wpa, ethhdr->h_source,
				ethhdr->h_dest, buf, len);
		}
		return;
	}
}

/* Initialize and setup RRB socket */
int setup_rrb_socket(nas_wksp_t *nwksp, char *ifname)
{
	struct l2_packet_data *l2_data;

	l2_data = l2_packet_init(ifname, ETH_P_RRB,
			broadcom_handle_data, nwksp);
	if (l2_data == NULL)
	{
		printf("Could not setup RRB socket, interface name=%s\n", ifname);
		return -1;
	}
	nwksp->l2_rrb = l2_data;
	nwksp->l2_rrb_fd = l2_data->fd;

	printf("Setup RRB socket, interface name=%s\n", ifname);
	return 0;
}

/* Deinit RRB Socket */
int deinit_rrb_socket(nas_wksp_t *nwksp)
{
	if (nwksp->l2_rrb != NULL)
	{
		l2_packet_deinit(nwksp->l2_rrb);
		nwksp->l2_rrb = NULL;
		nwksp->l2_rrb_fd = NAS_WKSP_UNK_FILE_DESC;
	}
	return 0;
}

/* Send RRB message contained by data to a given destination */
static int wpa_ft_rrb_send(wpa_t *wpa, uint8 *dst, uint8 *data, size_t data_len)
{
	struct l2_ethhdr *eth;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
	size_t len;
	int res = -1;
	char bssid[WLC_IOCTL_SMLEN];
	char *own_br_addr = NULL;
	uint8 own_addr[ETH_ALEN];

	dbg(wpa->nas, "FT: RRB send to %s", ether_etoa(dst, eabuf));

	len = sizeof(*eth) + data_len;
	eth = malloc(len);
	if (eth == NULL)
		return -1;

	nas_get_bssid(wpa->nas, bssid, sizeof(bssid));
	dbg(wpa->nas, "FT: RRB send from %s ", ether_etoa((uint8 *)bssid, eabuf));

	own_br_addr = nvram_safe_get("lan_hwaddr");
	ether_atoe(own_br_addr, own_addr);

	/* Fill ethernet header */
	memcpy(eth->h_source, own_addr, ETH_ALEN);
	memcpy(eth->h_dest, dst, ETH_ALEN);
	eth->h_proto = htons(ETH_P_RRB);
	memcpy(eth + 1, data, data_len);

	dbg(wpa->nas, "TX RRB PKT");
	dump(wpa->nas, (uint8 *) eth, len);

	if ((memcmp(eth->h_source, eth->h_dest, ETH_ALEN)) == 0) {
		dbg(wpa->nas, "FT: RRB frame dest address %s matching "
			"own address. Bypass RRB socket send and receive.", ether_etoa(dst, eabuf));
		broadcom_handle_data(WPA_NAS_WKSP(wpa), eth->h_source, (uint8 *) eth, len);
		free(eth);
		return 0;
	}

	if (WPA_NAS_WKSP(wpa)->l2_rrb != NULL)
	{
		res = l2_packet_send(WPA_NAS_WKSP(wpa)->l2_rrb, (uint8 *) eth, len);
	}
	else
	{
		dbg(wpa->nas, "FT: RRB send to %s failed no l2 socket\n", ether_etoa(dst, eabuf));
	}

	free(eth);
	return res;
}

/* Send action response received from peer AP to driver */
static int wpa_ft_action_send(wpa_t *wpa,
		const uint8 *dst, const uint8 *data, size_t data_len)
{
	wlc_fbt_action_resp_t *fbt_act_resp;
	uint8 buf[1024], *ptr;
	int fbt_act_resp_len;

	/* Fill the action response frame to send it to driver */
	memset(buf, 0, sizeof(buf));
	ptr = buf;
	fbt_act_resp = (wlc_fbt_action_resp_t *)ptr;
	memcpy(fbt_act_resp->macaddr, dst, ETHER_ADDR_LEN);
	memcpy(&fbt_act_resp->data[0], data, data_len);
	fbt_act_resp->data_len = data_len;
	fbt_act_resp_len = sizeof(wlc_fbt_action_resp_t) + data_len - 1;

	return nas_set_fbt_action(wpa->nas, ptr, fbt_act_resp_len);
}

/* Write RSN IE to the message pointed by buf */
static int wpa_write_rsn_ie(wpa_t *wpa, nas_sta_t *sta, uint8 *buf, size_t len,
                     const uint8 *pmkid)
{
	nas_t *nas;
	wpa_suite_mcast_t *mcast = NULL;
	wpa_suite_ucast_t *ucast = NULL;
	wpa_suite_auth_key_mgmt_t *auth = NULL;
	struct rsn_ie_hdr *hdr;
	uint8 *pos;
	uint8 *cap = NULL;
	uint32 wsec;
	uint32 algo;
	uint32 count;
	uint32 mode;

	if (buf == NULL) {
		dbg(wpa->nas, "FT: Invalid buffer\n");
		return -1;
	}

	/* Fill RSN IE header */
	hdr = (struct rsn_ie_hdr *) buf;
	hdr->elem_id = DOT11_MNG_RSN_ID;
	WPA_PUT_LE16(hdr->version, RSN_VERSION);
	pos = (uint8 *) (hdr + 1);
	nas = wpa->nas;
	wsec = nas->wsec;
	mode = nas->mode;
	algo = WEP_KEY2ALGO(wpa->gtk_len);

	/* Group Cipher Suit */
	mcast = (wpa_suite_mcast_t *)pos;
	bcopy(WPA2_OUI, mcast->oui, WPA_OUI_LEN);
	mcast->type = WPA_MCAST_CIPHER(wsec, algo);
	pos += WPA_SUITE_LEN;

	count = 0;

	/* Unicast suit list */
	ucast = (wpa_suite_ucast_t *)pos;
	if (WSEC_AES_ENABLED(wsec)) {
		bcopy(WPA2_OUI, ucast->list[count].oui, WPA_OUI_LEN);
		ucast->list[count].type = WPA_CIPHER_AES_CCM;
		pos += WPA_SUITE_LEN;
		count ++;
	}
	if (WSEC_TKIP_ENABLED(wsec)) {
		bcopy(WPA2_OUI, ucast->list[count].oui, WPA_OUI_LEN);
		ucast->list[count].type = WPA_CIPHER_TKIP;
		pos += WPA_SUITE_LEN;
		count ++;
	}
	if (!count) {
		bcopy(WPA2_OUI, ucast->list[count].oui, WPA_OUI_LEN);
		ucast->list[count].type = WPA_CIPHER_NONE;
		pos += WPA_SUITE_LEN;
		count ++;
	}

	ucast->count.low = (uint8)count;
	ucast->count.high = (uint8)(count>>8);
	pos += WPA_IE_SUITE_COUNT_LEN;

	/* akm suite list */
	auth = (wpa_suite_auth_key_mgmt_t *)pos;
	count = 0;
	pos += WPA_IE_SUITE_COUNT_LEN;

	if (mode & WPA2) {
		bcopy(WPA2_OUI, auth->list[count].oui, WPA_OUI_LEN);
		auth->list[count].type = wpa_auth2akm(wpa, wpa_mode2auth(WPA2));
		pos += WPA_SUITE_LEN;
		count ++;
	}
	if (mode & WPA2_PSK) {
		if (mode & WPA2_FT) {
			bcopy(WPA2_OUI, auth->list[count].oui, WPA_OUI_LEN);
			auth->list[count].type = wpa_auth2akm(wpa, wpa_mode2auth(WPA2_FT));
			pos += WPA_SUITE_LEN;
			count ++;
		}
		else {
			bcopy(WPA2_OUI, auth->list[count].oui, WPA_OUI_LEN);
			auth->list[count].type = wpa_auth2akm(wpa, wpa_mode2auth(WPA2_PSK));
			pos += WPA_SUITE_LEN;
			count ++;
		}
	}
	if (!count) {
		bcopy(WPA2_OUI, auth->list[count].oui, WPA_OUI_LEN);
		auth->list[count].type = wpa_auth2akm(wpa, WPA_AUTH_NONE);
		pos += WPA_SUITE_LEN;
		count ++;
	}

	auth->count.low = (uint8)count;
	auth->count.high = (uint8)(count>>8);

	/* WPA capabilities */
	cap = (uint8 *)&auth->list[count];

	cap[0] = wpa->cap[0];
	cap[1] = wpa->cap[1];

	pos += WPA_CAP_LEN;

	if (pmkid) {
		if (pos + 2 + PMKID_LEN > buf + len)
			return -1;
		/* PMKID Count */
		WPA_PUT_LE16(pos, 1);
		pos += 2;
		memcpy(pos, pmkid, PMKID_LEN);
		pos += PMKID_LEN;
	}
#ifdef MFP
	if (((wpa)->cap[0] & RSN_CAP_MFPC) || ((wpa)->cap[0] & RSN_CAP_MFPR))	{
		if (pos + 2 + 4 > buf + len)
			return -1;
		if (pmkid == NULL) {
			/* PMKID Count */
			WPA_PUT_LE16(pos, 0);
			pos += 2;
		}

		/* Management Group Cipher Suite */
		mcast = NULL;
		mcast = (wpa_suite_mcast_t *)pos;
		bcopy(WPA2_OUI, mcast->oui, WPA_OUI_LEN);
		mcast->type = RSN_AKM_MFP_PSK;
		pos += WPA_SUITE_LEN;
	}
#endif /* MFP */
	hdr->len = (pos - buf) - 2;

	return pos - buf;
}

/* Write MD IE to the message pointed by buf */
int wpa_write_mdie(wpa_t *wpa, nas_sta_t *sta, uint8 *buf, size_t len)
{
	uint8 *pos = buf;
	uint8 capab;
	if (len < 2 + sizeof(struct rsn_mdie))
		return -1;

	*pos++ = DOT11_MNG_MDIE_ID;
	*pos++ = MOBILITY_DOMAIN_ID_LEN + 1;
	memcpy(pos, wpa->fbt_info.mobility_domain, MOBILITY_DOMAIN_ID_LEN);
	pos += MOBILITY_DOMAIN_ID_LEN;
	capab = 0;
	if (wpa->fbt_info.ft_over_ds)
		capab |= RSN_FT_CAPAB_FT_OVER_DS;
	*pos++ = capab;

	return pos - buf;
}

/* Write FT IE to the message pointed by buf */
int wpa_write_ftie(wpa_t *wpa, nas_sta_t *sta, const uint8 *r0kh_id,
	size_t r0kh_id_len,
	const uint8 *anonce, const uint8 *snonce,
	uint8 *buf, size_t len, const uint8 *subelem,
	size_t subelem_len)
{
	uint8 *pos = buf, *ielen;
	struct rsn_ftie *hdr;

	if (len < 2 + sizeof(*hdr) + 2 + FT_R1KH_ID_LEN + 2 + r0kh_id_len +
	    subelem_len)
		return -1;

	*pos++ = DOT11_MNG_FTIE_ID;
	ielen = pos++;

	hdr = (struct rsn_ftie *) pos;
	memset(hdr, 0, sizeof(*hdr));
	pos += sizeof(*hdr);
	WPA_PUT_LE16(hdr->mic_control, 0);
	if (anonce)
		memcpy(hdr->anonce, anonce, WPA_NONCE_LEN);
	if (snonce)
		memcpy(hdr->snonce, snonce, WPA_NONCE_LEN);

	/* Optional Parameters */
	*pos++ = FTIE_SUBELEM_R1KH_ID;
	*pos++ = FT_R1KH_ID_LEN;
	memcpy(pos, wpa->fbt_info.r1_key_holder, FT_R1KH_ID_LEN);
	pos += FT_R1KH_ID_LEN;

	if (r0kh_id) {
		*pos++ = FTIE_SUBELEM_R0KH_ID;
		*pos++ = r0kh_id_len;
		memcpy(pos, r0kh_id, r0kh_id_len);
		pos += r0kh_id_len;
	}

	if (subelem) {
		memcpy(pos, subelem, subelem_len);
		pos += subelem_len;
	}

	*ielen = pos - buf - 2;

	return pos - buf;
}

struct wpa_ft_pmk_r0_sa {
	struct wpa_ft_pmk_r0_sa *next;
	uint8 pmk_r0[PMK_LEN];
	uint8 pmk_r0_name[WPA_PMK_NAME_LEN];
	uint8 spa[ETH_ALEN];
	int pairwise; /* Pairwise cipher suite, WPA_CIPHER_* */
	/* TODO: expiration, identity, radius_class, EAP type, VLAN ID */
	int pmk_r1_pushed;
};

struct wpa_ft_pmk_r1_sa {
	struct wpa_ft_pmk_r1_sa *next;
	uint8 pmk_r1[PMK_LEN];
	uint8 pmk_r1_name[WPA_PMK_NAME_LEN];
	uint8 spa[ETH_ALEN];
	int pairwise; /* Pairwise cipher suite, WPA_CIPHER_* */
	/* TODO: expiration, identity, radius_class, EAP type, VLAN ID */
};

struct wpa_ft_pmk_cache {
	struct wpa_ft_pmk_r0_sa *pmk_r0;
	struct wpa_ft_pmk_r1_sa *pmk_r1;
};

/* Initialize PMK cache */
struct wpa_ft_pmk_cache * wpa_ft_pmk_cache_init(wpa_t *wpa)
{
	struct wpa_ft_pmk_cache *cache;

	/* Allocate memory for cache */
	cache = malloc(sizeof(*cache));
	if (cache == NULL) {
		dbg(wpa->nas, "FT: Failed to allocate memory for PMK cache\n");
		return NULL;
	}

	memset(cache, 0, sizeof(*cache));

	return cache;
}

/* Deinitialize PMK cache */
void wpa_ft_pmk_cache_deinit(struct wpa_ft_pmk_cache *cache)
{
	struct wpa_ft_pmk_r0_sa *r0, *r0prev;
	struct wpa_ft_pmk_r1_sa *r1, *r1prev;

	if (!cache)
		return;
	r0 = cache->pmk_r0;
	while (r0) {
		r0prev = r0;
		r0 = r0->next;
		memset(r0prev->pmk_r0, 0, PMK_LEN);
		free(r0prev);
	}

	r1 = cache->pmk_r1;
	while (r1) {
		r1prev = r1;
		r1 = r1->next;
		memset(r1prev->pmk_r1, 0, PMK_LEN);
		free(r1prev);
	}

	free(cache);
}

/* Deinitialize r0kh_list and r1kh_list */
void wpa_ft_r0kh_r1kh_deinit(fbt_t *fbt_info)
{
	struct ft_remote_r1kh *r1kh, *r1khprev;
	struct ft_remote_r0kh *r0kh, *r0khprev;

	/* Free r0kh list */
	r0kh = fbt_info->r0kh_list;
	while (r0kh) {
		r0khprev = r0kh;
		r0kh = r0kh->next;
		free(r0khprev);
	}
	fbt_info->r0kh_list = NULL;

	/* Free r1kh list */
	r1kh = fbt_info->r1kh_list;
	while (r1kh) {
		r1khprev = r1kh;
		r1kh = r1kh->next;
		free(r1khprev);
	}
	fbt_info->r1kh_list = NULL;
}

/* Prepare R0KH and R1KH list based on nvram configuration */
void wpa_ft_r0kh_r1kh_init(wpa_t *wpa)
{
	struct ft_remote_r1kh *r1kh;
	struct ft_remote_r0kh *r0kh;
	char *r1kh_addr, *r0kh_addr, *br_addr;
	char *r1kh_id, *r0kh_id_len, *r0kh_id;
	char *r1kh_key, *r0kh_key;
	char *next;
	char tar_name[128];
	char nv_name[128];
	char name[128];
	char *fbt_aps;

	/* Get All the FBT AP's */
	fbt_aps = malloc(sizeof(wpa->fbt_info.fbt_aps));
	if (fbt_aps == NULL) {
		dbg(wpa->nas, "FT: Failed to allocate memory to fbt_aps");
		return;
	}

	memcpy(fbt_aps, wpa->fbt_info.fbt_aps, sizeof(wpa->fbt_info.fbt_aps));
	/* Free r0kh and r1kh list if its already present */
	wpa_ft_r0kh_r1kh_deinit(&wpa->fbt_info);

	memset(name, 0, sizeof(name));
	foreach(name, fbt_aps, next)
	{
		/* For each FBT AP get ro and r1 list */
		memset(tar_name, 0, sizeof(tar_name));
		dbg(wpa->nas, "FT: tar_name = %s\n", name);
		memcpy(tar_name, name, sizeof(tar_name));

		r0kh = malloc(sizeof(*r0kh));
		if (r0kh != NULL)
		{
			memset(r0kh, 0, sizeof(*r0kh));

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_r0kh_id_len", tar_name);
			r0kh_id_len = nvram_get(nv_name);
			if (!r0kh_id_len) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r0kh);
				continue;
			}
			r0kh->id_len = atoi(r0kh_id_len);
			dbg(wpa->nas, "FT: nvname1 = %s value %s\n", nv_name, r0kh_id_len);

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_r0kh_id", tar_name);
			r0kh_id = nvram_get(nv_name);
			if (!r0kh_id) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r0kh);
				continue;
			}
			memcpy(r0kh->id, r0kh_id, r0kh->id_len);
			dbg(wpa->nas, "FT: nvname2 = %s value %s\n", nv_name, r0kh_id);

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_addr", tar_name);
			r0kh_addr = nvram_get(nv_name);
			if (!r0kh_addr) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r0kh);
				continue;
			}
			ether_atoe(r0kh_addr, r0kh->addr);
			dbg(wpa->nas, "FT: nvname3 = %s value %s\n", nv_name, r0kh_addr);

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_r0kh_key", tar_name);
			r0kh_key = nvram_get(nv_name);
			if (!r0kh_key) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r0kh);
				continue;
			}
			memcpy(r0kh->key, r0kh_key, strlen(r0kh_key));
			dbg(wpa->nas, "FT: nvname4 = %s value %s\n", nv_name, r0kh_key);

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_br_addr", tar_name);
			br_addr = nvram_get(nv_name);
			if (!br_addr) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r0kh);
				continue;
			}
			ether_atoe(br_addr, r0kh->br_addr);
			dbg(wpa->nas, "FT: nvname5 = %s value %s\n", nv_name, br_addr);

			r0kh->next = wpa->fbt_info.r0kh_list;
			wpa->fbt_info.r0kh_list = r0kh;
		}

		r1kh = malloc(sizeof(*r1kh));
		if (r1kh != NULL)
		{
			memset(r1kh, 0, sizeof(*r1kh));
			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_r1kh_id", tar_name);
			r1kh_id = nvram_get(nv_name);
			if (!r1kh_id) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r1kh);
				continue;
			}
			ether_atoe(r1kh_id, r1kh->id);
			dbg(wpa->nas, "FT: nvname1 = %s value %s\n", nv_name, r1kh_id);

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_addr", tar_name);
			r1kh_addr = nvram_get(nv_name);
			if (!r1kh_addr) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r1kh);
				continue;
			}
			ether_atoe(r1kh_addr, r1kh->addr);
			dbg(wpa->nas, "FT: nvname2 = %s value %s\n", nv_name, r1kh_addr);

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_r1kh_key", tar_name);
			r1kh_key = nvram_get(nv_name);
			if (!r1kh_key) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r1kh);
				continue;
			}
			memcpy(r1kh->key, r1kh_key, strlen(r1kh_key));
			dbg(wpa->nas, "FT: nvname3 = %s value %s\n", nv_name, r1kh_key);

			memset(nv_name, 0, sizeof(nv_name));
			snprintf(nv_name, sizeof(nv_name), "%s_br_addr", tar_name);
			br_addr = nvram_get(nv_name);
			if (!br_addr) {
				dbg(wpa->nas, "FT: %s empty\n", nv_name);
				free(r1kh);
				continue;
			}
			ether_atoe(br_addr, r1kh->br_addr);
			dbg(wpa->nas, "FT: nvname4 = %s value %s\n", nv_name, br_addr);

			r1kh->next = wpa->fbt_info.r1kh_list;
			wpa->fbt_info.r1kh_list = r1kh;
		}
	}
	free(fbt_aps);
	return;
}

/* Save PMKR0 in cache */
static int wpa_ft_store_pmk_r0(wpa_t *wpa,
		const uint8 *spa, const uint8 *pmk_r0,
		const uint8 *pmk_r0_name, int pairwise)
{
	struct wpa_ft_pmk_cache *cache = wpa->fbt_info.ft_pmk_cache;
	struct wpa_ft_pmk_r0_sa *r0;

	/* TODO: add expiration and limit on number of entries in cache */

	if (!cache) {
		dbg(wpa->nas, "FT: cache is NULL\n");
		return -1;
	}

	/* First check if there is any entry already there for the STA */
	r0 = cache->pmk_r0;
	while (r0) {
		if (memcmp(r0->spa, spa, ETH_ALEN) == 0) {
			/* If found, update the entry */
			memcpy(r0->pmk_r0, pmk_r0, PMK_LEN);
			memcpy(r0->pmk_r0_name, pmk_r0_name, WPA_PMK_NAME_LEN);
			r0->pairwise = pairwise;
			goto end;
		}
		r0 = r0->next;
	}

	/* If new entry allocate and add */
	r0 = malloc(sizeof(*r0));
	if (r0 == NULL) {
		dbg(wpa->nas, "FT: Failed to allocate memory for PMKR0 cache STA "MACF"",
			ETHERP_TO_MACF(spa));
		return -1;
	}

	memcpy(r0->pmk_r0, pmk_r0, PMK_LEN);
	memcpy(r0->pmk_r0_name, pmk_r0_name, WPA_PMK_NAME_LEN);
	memcpy(r0->spa, spa, ETH_ALEN);
	r0->pairwise = pairwise;

	r0->next = cache->pmk_r0;
	cache->pmk_r0 = r0;

end:
	return 0;
}

/* Fetch PMKR0 for a given PMKR0Name from cache */
static int wpa_ft_fetch_pmk_r0(wpa_t *wpa,
		const uint8 *spa, const uint8 *pmk_r0_name,
		uint8 *pmk_r0, int *pairwise)
{
	struct wpa_ft_pmk_cache *cache = wpa->fbt_info.ft_pmk_cache;
	struct wpa_ft_pmk_r0_sa *r0;

	if (!cache) {
		dbg(wpa->nas, "FT: cache is NULL\n");
		return -1;
	}

	/* Search in PMKR0 cache based on STA MAC and PMKR0Name */
	r0 = cache->pmk_r0;
	while (r0) {
		if (memcmp(r0->spa, spa, ETH_ALEN) == 0 &&
		    memcmp(r0->pmk_r0_name, pmk_r0_name, WPA_PMK_NAME_LEN)
		    == 0) {
			memcpy(pmk_r0, r0->pmk_r0, PMK_LEN);
			if (pairwise)
				*pairwise = r0->pairwise;
			return 0;
		}

		r0 = r0->next;
	}

	return -1;
}

/* Save PMKR1 in cache */
static int wpa_ft_store_pmk_r1(wpa_t *wpa,
		const uint8 *spa, const uint8 *pmk_r1,
		const uint8 *pmk_r1_name, int pairwise)
{
	struct wpa_ft_pmk_cache *cache = wpa->fbt_info.ft_pmk_cache;
	struct wpa_ft_pmk_r1_sa *r1;

	/* TODO: add expiration and limit on number of entries in cache */

	if (!cache) {
		dbg(wpa->nas, "FT: cache is NULL\n");
		return -1;
	}

	/* First check if there is any entry already there for the STA */
	r1 = cache->pmk_r1;
	while (r1) {
		if (memcmp(r1->spa, spa, ETH_ALEN) == 0) {
			memcpy(r1->pmk_r1, pmk_r1, PMK_LEN);
			memcpy(r1->pmk_r1_name, pmk_r1_name, WPA_PMK_NAME_LEN);
			r1->pairwise = pairwise;
			goto end;
		}
		r1 = r1->next;
	}

	/* If new entry allocate and add */
	r1 = malloc(sizeof(*r1));
	if (r1 == NULL) {
		dbg(wpa->nas, "FT: Failed to allocate memory for PMKR1 cache STA "MACF"",
			ETHERP_TO_MACF(spa));
		return -1;
	}

	memcpy(r1->pmk_r1, pmk_r1, PMK_LEN);
	memcpy(r1->pmk_r1_name, pmk_r1_name, WPA_PMK_NAME_LEN);
	memcpy(r1->spa, spa, ETH_ALEN);
	r1->pairwise = pairwise;

	r1->next = cache->pmk_r1;
	cache->pmk_r1 = r1;

end:
	return 0;
}

/* Fetch PMKR0 for a given PMKR1Name from cache */
static int wpa_ft_fetch_pmk_r1(wpa_t *wpa,
		const uint8 *spa, const uint8 *pmk_r1_name,
		uint8 *pmk_r1, int *pairwise)
{
	struct wpa_ft_pmk_cache *cache = wpa->fbt_info.ft_pmk_cache;
	struct wpa_ft_pmk_r1_sa *r1;

	if (!cache) {
		dbg(wpa->nas, "FT: cache is NULL\n");
		return -1;
	}

	/* Search in PMKR1 cache based on STA MAC and PMKR1Name */
	r1 = cache->pmk_r1;
	while (r1) {
		if (memcmp(r1->spa, spa, ETH_ALEN) == 0 &&
		    memcmp(r1->pmk_r1_name, pmk_r1_name, WPA_PMK_NAME_LEN)
		    == 0) {
			memcpy(pmk_r1, r1->pmk_r1, PMK_LEN);
			if (pairwise)
				*pairwise = r1->pairwise;
			return 0;
		}

		r1 = r1->next;
	}

	return -1;
}

/* Pull PMKR1 from peer R0KH */
static int wpa_ft_pull_pmk_r1(wpa_t *wpa,
		const uint8 *s1kh_id, const uint8 *r0kh_id,
		size_t r0kh_id_len, const uint8 *pmk_r0_name)
{
	struct ft_r0kh_r1kh_pull_frame frame, f;
	struct ft_remote_r0kh *r0kh;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	char bssid[WLC_IOCTL_SMLEN];

	r0kh = wpa->fbt_info.r0kh_list;
	while (r0kh) {
		if (r0kh->id_len == r0kh_id_len &&
			memcmp(r0kh->id, r0kh_id, r0kh_id_len) == 0)
			break;
		r0kh = r0kh->next;
	}
	if (r0kh == NULL) {
		return -1;
	}

	dbg(wpa->nas, "FT: Send PMK-R1 pull request to remote R0KH address %s ",
			ether_etoa(r0kh->addr, eabuf));

	nas_get_bssid(wpa->nas, bssid, sizeof(bssid));
	memset(&frame, 0, sizeof(frame));
	frame.frame_type = RSN_REMOTE_FRAME_TYPE_FT_RRB;
	frame.packet_type = FT_PACKET_R0KH_R1KH_PULL;
	frame.data_length = host_to_le16(FT_R0KH_R1KH_PULL_DATA_LEN);
	memcpy(frame.ap_address, bssid, ETH_ALEN);
	memcpy(frame.dest_ap_address, r0kh->addr, ETH_ALEN);

	/* aes_wrap() does not support inplace encryption, so use a temporary
	 * buffer for the data.
	 */

	wpa_generate_rand_nonce(&wpa->nas->ea, f.nonce, sizeof(f.nonce));
	memcpy(f.pmk_r0_name, pmk_r0_name, WPA_PMK_NAME_LEN);
	memcpy(f.r1kh_id, wpa->fbt_info.r1_key_holder, FT_R1KH_ID_LEN);
	memcpy(f.s1kh_id, s1kh_id, ETH_ALEN);

	aes_wrap(strlen((const char *)r0kh->key), (uint8 *)r0kh->key,
		FT_R0KH_R1KH_PULL_DATA_LEN + 4, f.nonce, frame.nonce);
	dbg(wpa->nas, "Sending rrb request");
	dump(wpa->nas, (uint8 *)&frame, sizeof(frame));
	wpa_ft_rrb_send(wpa, r0kh->br_addr, (uint8 *) &frame, sizeof(frame));

	return 0;
}

/* Derive PTK using anonce, snonce, PMKR1 etc. */
int wpa_auth_derive_ptk_ft(wpa_t *wpa, nas_sta_t *sta,
	unsigned char *ptk, size_t ptk_len)
{
	unsigned char pmk_r0[PMK_LEN], pmk_r0_name[WPA_PMK_NAME_LEN], pmk_r1_name[WPA_PMK_NAME_LEN];
	unsigned char pmk_r1[PMK_LEN];
	const unsigned char *r0kh = wpa->fbt_info.r0_key_holder;
	size_t r0kh_len = wpa->fbt_info.r0_key_holder_len;
	const unsigned char *r1kh = wpa->fbt_info.r1_key_holder;
	uint32 ssid_len;
	int pairwise;
	bool psk_local = wpa->fbt_info.ft_psk_generate_local;
	uint16 md_id;
	uint8 ssid[WLC_IOCTL_SMLEN];
	char bssid[WLC_IOCTL_SMLEN];

	memset(ssid, 0, WLC_IOCTL_SMLEN);
	nas_get_ssid(wpa->nas, ssid, &ssid_len);

	nas_get_fbt_mdid(wpa->nas, &md_id);
	pairwise = WPA_MCAST_CIPHER(wpa->nas->wsec, WEP_KEY2ALGO(wpa->gtk_len));

	wpa_calc_pmkR0((uchar *)ssid, ssid_len, md_id, (uint8 *)r0kh, r0kh_len, &sta->ea,
		sta->suppl.pmk, sta->suppl.pmk_len, pmk_r0, pmk_r0_name);
	dbg(wpa->nas, "FT: PMK-R0");
	dump(wpa->nas, pmk_r0, PMK_LEN);
	dbg(wpa->nas, "FT: PMKR0Name");
	dump(wpa->nas, pmk_r0_name, WPA_PMK_NAME_LEN);
	wpa_ft_store_pmk_r0(wpa, sta->ea.octet, pmk_r0, pmk_r0_name, pairwise);

	wpa_calc_pmkR1((struct ether_addr *)r1kh, &sta->ea, pmk_r0,
			PMK_LEN, pmk_r0_name, pmk_r1, pmk_r1_name);
	dbg(wpa->nas, "FT: PMK-R1");
	dump(wpa->nas, pmk_r1, PMK_LEN);
	dbg(wpa->nas, "FT: PMKR1Name");
	dump(wpa->nas, pmk_r1_name, WPA_PMK_NAME_LEN);
	bcopy(pmk_r1_name, wpa->fbt_info.pmk_r1name, WPA_PMK_NAME_LEN);
	if (!psk_local) {
		wpa_ft_store_pmk_r1(wpa, sta->ea.octet, pmk_r1, pmk_r1_name, pairwise);
	}

	memset(bssid, 0, WLC_IOCTL_SMLEN);
	nas_get_bssid(wpa->nas, bssid, sizeof(bssid));
	wpa_calc_ft_ptk((struct ether_addr *)bssid, &sta->ea, (uint8*)&sta->suppl.anonce,
			(uint8*)&sta->suppl.snonce, pmk_r1, PMK_LEN, ptk, ptk_len);
	dbg(wpa->nas, "FT: PTK");
	dump(wpa->nas, ptk, ptk_len);

	wpa_ft_push_pmk_r1(wpa);

	return 0;
}

/* Derive PMK-R1 from PSK locally
 */
static int wpa_ft_psk_pmk_r1(wpa_t *wpa, nas_sta_t *sta,
	const uint8 *req_pmk_r1_name,
	uint8 *out_pmk_r1, int *out_pairwise)
{
	uint8 pmk_r0[PMK_LEN], pmk_r0_name[WPA_PMK_NAME_LEN];
	uint8 pmk_r1[PMK_LEN], pmk_r1_name[WPA_PMK_NAME_LEN];
	uint16 md_id;
	uint8 *r0kh = sta->suppl.ft_info.r0kh_id;
	size_t r0kh_len = sta->suppl.ft_info.r0kh_id_len;
	const uint8 *r1kh = wpa->fbt_info.r1_key_holder;
	uint8 ssid[WLC_IOCTL_SMLEN];
	uint32 ssid_len;
	int pairwise;

	memset(ssid, 0, WLC_IOCTL_SMLEN);
	nas_get_ssid(wpa->nas, ssid, &ssid_len);

	nas_get_fbt_mdid(wpa->nas, &md_id);
	pairwise = WPA_MCAST_CIPHER(wpa->nas->wsec, WEP_KEY2ALGO(wpa->gtk_len));

	wpa_calc_pmkR0((uchar *)ssid, ssid_len, md_id, r0kh, r0kh_len, &sta->ea,
		sta->suppl.pmk, sta->suppl.pmk_len, pmk_r0, pmk_r0_name);
	wpa_calc_pmkR1((struct ether_addr *)r1kh, &sta->ea, pmk_r0,
		PMK_LEN, pmk_r0_name, pmk_r1, pmk_r1_name);
	dbg(wpa->nas, "FT: PMKR1Name");
	dump(wpa->nas, pmk_r1_name, WPA_PMK_NAME_LEN);

	if (memcmp(pmk_r1_name, req_pmk_r1_name,
		WPA_PMK_NAME_LEN) != 0) {
		dbg(wpa->nas, "FT: did not find PSK to generate PMK_R1 "
				"locally");
		return -1;
	}
	/* we found a PSK that matches the request pmk_r1_name */
	dbg(wpa->nas, "FT: found PSK to generate PMK_R1 "
			"locally");
	memcpy(out_pmk_r1, pmk_r1, PMK_LEN);
	if (out_pairwise)
		*out_pairwise = pairwise;
	return 0;
}

/* Process Over the air authenticaton request */
static int32 wpa_ft_process_auth_req(wpa_t *wpa, nas_sta_t *sta, uint8 *data, uint16 len,
		uint8 *pmk_r1, uint8 *pmk_r1_name,
		int *pairwise, struct wpa_ft_ies *parse)
{
	nas_t *nas = wpa->nas;
	struct rsn_mdie *mdie;
	struct rsn_ftie *ftie;
	struct wpa_ie_data rsnie_data;
	uint8 rsn_cap[WPA_CAP_LEN];
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	char buf[WLC_IOCTL_SMLEN];
	int ret;
	uint32 fbt_overds;

	sta->suppl.ft_info.pmk_r1_name_valid = 0;
	sta->auth_retx = 0;

	dbg(nas, "FT: Received authentication frame IEs len %d", len);

	dbg(nas, "FT: STA ALL IES");
	dump(nas, data, len);

	ret = wpa_ft_validate_ies(wpa, data, len, parse, &rsnie_data);
	if (ret) {
		dbg(nas, "FT: Validation of FT IEs failed");
		return ret;
	}

	/* RSNIE capability */
	WPA_PUT_LE16(rsn_cap, rsnie_data.capabilities);
	sta->cap[0] = rsn_cap[0];
	sta->cap[1] = rsn_cap[1];

	/* MDIE info */
	mdie = (struct rsn_mdie *) parse->mdie;
	memcpy(wpa->fbt_info.mobility_domain, mdie->mobility_domain, MOBILITY_DOMAIN_ID_LEN);
	nas_get_fbt_overds(wpa->nas, &fbt_overds);
	wpa->fbt_info.ft_over_ds = fbt_overds;

	/* R0KH-ID */
	memset(buf, 0, sizeof(buf));
	nas_get_fbt_r0kh(nas, buf, sizeof(buf));
	memcpy(wpa->fbt_info.r0_key_holder, buf, parse->r0kh_id_len);
	wpa->fbt_info.r0_key_holder_len = strlen(buf);

	/* R1KH-ID */
	memset(buf, 0, sizeof(buf));
	nas_get_fbt_r1kh(nas, buf, sizeof(buf));
	memcpy(wpa->fbt_info.r1_key_holder, buf, FT_R1KH_ID_LEN);

	ftie = (struct rsn_ftie *) parse->ftie;
	/* If snonce of the frame is the same, it is a retransmission of the auth request */
	if (memcmp(sta->suppl.snonce, ftie->snonce, WPA_NONCE_LEN) == 0) {
		sta->auth_retx = 1;
		dbg(nas, "Auth frame retransmitted!!");
		dump(nas, sta->suppl.snonce, WPA_NONCE_LEN);
		dump(nas, ftie->snonce, WPA_NONCE_LEN);
	}
	memcpy(sta->suppl.snonce, ftie->snonce, WPA_NONCE_LEN);

	memcpy(sta->suppl.ft_info.r0kh_id, parse->r0kh_id, parse->r0kh_id_len);
	sta->suppl.ft_info.r0kh_id_len = parse->r0kh_id_len;

	dbg(nas, "R1KH_ID: %s", (uint8 *)ether_etoa(wpa->fbt_info.r1_key_holder, eabuf));

	dbg(nas, "S1KH_ID: %s", (uint8 *)ether_etoa(sta->ea.octet, eabuf));

	wpa_derive_pmkR1_name((struct ether_addr *)wpa->fbt_info.r1_key_holder, &sta->ea,
			(uint8 *)parse->rsn_pmkid, pmk_r1_name);

	dbg(nas, "FT: Derived requested PMKR1Name");
	dump(nas, pmk_r1_name, WPA_PMK_NAME_LEN);

	if (wpa->fbt_info.ft_psk_generate_local) {
		sta->suppl.pmk_len = wpa->pmk_len;
		bcopy(wpa->pmk, sta->suppl.pmk, wpa->pmk_len);
		if (wpa_ft_psk_pmk_r1(wpa, sta, pmk_r1_name, pmk_r1, pairwise) < 0) {
			return DOT11_SC_INVALID_PMKID;
		}
	} else if (wpa_ft_fetch_pmk_r1(wpa, sta->ea.octet, pmk_r1_name, pmk_r1,
		pairwise) < 0) {
		if (wpa_ft_pull_pmk_r1(wpa, sta->ea.octet, sta->suppl.ft_info.r0kh_id,
			sta->suppl.ft_info.r0kh_id_len, parse->rsn_pmkid) < 0) {
			dbg(nas, "FT: Did not have matching PMK-R1 and unknown R0KH-ID");
			return DOT11_SC_INVALID_PMKID;
		}

		return -1; /* Status pending */
	}
	return 0;
}

/* Prepare authentication response with RSNIE, MDIE and FTIE */
static int32 wpa_ft_prepare_auth_resp(wpa_t *wpa, nas_sta_t *sta, uint8 *pmk_r1,
		struct wpa_ft_ies parse,  int pairwise, uint8 **resp_ies, size_t *resp_ies_len)
{
	uint8 *pos, *end;
	size_t buflen, ptk_len;
	nas_t *nas = wpa->nas;
	char bssid[WLC_IOCTL_SMLEN];
	int ret;

	/* Get the anonce */
	if (sta->auth_retx == 0) {
		/* If SNONCE is different from the previous one, then only get the new ANONCE.
		 * In case of retry the SNONCE will be same, at that time send the old ANONCE
		 */
		wpa_generate_rand_nonce(&nas->ea, sta->suppl.anonce, sizeof(sta->suppl.anonce));
	}

	dbg(nas, "FT: Received SNonce");
	dump(nas, sta->suppl.snonce, WPA_NONCE_LEN);
	dbg(nas, "FT: Generated ANonce");
	dump(nas, sta->suppl.anonce, WPA_NONCE_LEN);
	ptk_len = pairwise == WPA_CIPHER_TKIP ? 64 : 48;

	nas_get_bssid(nas, bssid, sizeof(bssid));

	wpa_calc_ft_ptk((struct ether_addr *)bssid, &sta->ea,
			sta->suppl.anonce, sta->suppl.snonce, pmk_r1, PMK_LEN,
			(uint8 *)&wpa->PTK, ptk_len);

	dbg(nas, "FT: PTK");
	dump(nas, (uint8 *) &wpa->PTK, ptk_len);

	buflen = 2 + sizeof(struct rsn_mdie) + 2 + sizeof(struct rsn_ftie) +
		2 + FT_R1KH_ID_LEN + 200;
	*resp_ies = malloc(buflen);
	if (*resp_ies == NULL) {
		return DOT11_SC_FAILURE;
	}

	pos = *resp_ies;
	end = *resp_ies + buflen;

	ret = wpa_write_rsn_ie(wpa, sta, pos, end - pos, parse.rsn_pmkid);
	if (ret < 0) {
		free(*resp_ies);
		*resp_ies = NULL;
		return DOT11_SC_FAILURE;
	}
	pos += ret;

	ret = wpa_write_mdie(wpa, sta, pos, end - pos);
	if (ret < 0) {
		free(*resp_ies);
		*resp_ies = NULL;
		return DOT11_SC_FAILURE;
	}
	pos += ret;

	ret = wpa_write_ftie(wpa, sta, parse.r0kh_id, parse.r0kh_id_len,
		sta->suppl.anonce, sta->suppl.snonce, pos, end - pos, NULL, 0);
	if (ret < 0) {
		free(*resp_ies);
		*resp_ies = NULL;
		return DOT11_SC_FAILURE;
	}
	pos += ret;

	*resp_ies_len = pos - *resp_ies;

	return DOT11_SC_SUCCESS;
}

/* Build authentication response */
int wpa_auth_ft_build_auth_resp(wpa_t *wpa, nas_sta_t *sta,
	uint8 *sta_addr, uint16 status, uint8 *resp_ies,
	uint32 resp_ie_len, uint8 *buf, uint32 *len)
{
	wlc_fbt_auth_resp_t *ft_authresp = (wlc_fbt_auth_resp_t *)buf;

	memset(ft_authresp, 0, sizeof(wlc_fbt_auth_resp_t));
	ft_authresp->status = status;
	memcpy(ft_authresp->macaddr, sta_addr, ETH_ALEN);

	*len = sizeof(wlc_fbt_auth_resp_t) - sizeof(ft_authresp->ies) + resp_ie_len;

	if (status != DOT11_SC_SUCCESS)
	{
		return 0;
	}

	memcpy(ft_authresp->pmk_r1_name, sta->suppl.ft_info.pmk_r1_name, WPA_PMK_NAME_LEN);

	ft_authresp->ie_len = resp_ie_len;
	memcpy(ft_authresp->ies, resp_ies, resp_ie_len);

	memcpy(&(ft_authresp->ptk), &(wpa->PTK), sizeof(struct wpa_ptk));
	if (!(wpa->nas->flags & NAS_FLAG_GTK_PLUMBED))
		wpa_init_gtk(wpa, sta);
#ifdef MFP
	if ((MFP_IGTK_REQUIRED(wpa, sta)) &&
		!(wpa->nas->flags & NAS_FLAG_IGTK_PLUMBED)) {
		wpa_gen_igtk(wpa);
		wpa->nas->flags |= NAS_FLAG_IGTK_PLUMBED;
	}
#endif /* MFP */
	/* Send current gtk index */
	ft_authresp->gtk.idx = wpa->gtk_index;
	ft_authresp->gtk.key_len = wpa->gtk_len;
	memcpy(ft_authresp->gtk.key, wpa->gtk, wpa->gtk_len);
	dbg(wpa->nas, "FT: GTK");
	dump(wpa->nas, (uint8 *) &wpa->gtk, wpa->gtk_len);
	ft_authresp->status = DOT11_SC_SUCCESS;
	return 0;
}

/* Build OTA authentication response, send response to the driver and install unicast and
 * multicast keys
 */
static void wpa_ft_process_ota_auth(wpa_t *wpa, nas_sta_t *sta, uint8 *resp_ies,
		uint32 resp_ies_len, uint16 status)
{
	uint8 buf[1024];
	uint32 auth_resp_len = 0;

	dbg(wpa->nas, "FT: FT authentication response: status %d\n", status);
	wpa_auth_ft_build_auth_resp(wpa, sta, sta->ea.octet,
              status, resp_ies, resp_ies_len, buf, &auth_resp_len); /* build auth resp */
	dbg(wpa->nas, "FT: Auth response");
	dump(wpa->nas, buf, auth_resp_len);
	nas_set_fbt_auth_resp(wpa->nas, buf, auth_resp_len);
	if (sta->auth_retx == 0) {
		/* plumb pairwise key */
		if (nas_set_key(wpa->nas, &sta->ea, wpa->PTK.tk1,
				sizeof(wpa->PTK.tk1), 0, 1, 0, 0, 0) < 0) {
			dbg(wpa->nas, "unicast key rejected by driver, assuming too"
				"many associated STAs");
			cleanup_sta(wpa->nas, sta, DOT11_RC_BUSY, 0);
			return;
		}
	}

	return;
}

/* Build OTD authentication response, send response to the driver and install unicast and
 * multicast keys
 */
static int wpa_ft_process_otd_auth(wpa_t *wpa, nas_sta_t *sta, const uint8 *current_ap,
	const uint8 *dest_bssid, uint8 *resp_ies, uint32 resp_ies_len, uint16 status)
{
	uint8 buf[1024], sta_addr[ETH_ALEN];
	uint32 auth_resp_len = 0;
	uint8 *pos;
	int32 err = 0;
	struct ft_rrb_frame *frame;
	char bssid[WLC_IOCTL_SMLEN];
#ifdef BCMDBG
	char eabuf_tmp1[ETHER_ADDR_STR_LEN];
	char eabuf_tmp2[ETHER_ADDR_STR_LEN];
#endif // endif
	size_t rlen;

	memcpy(sta_addr, sta->ea.octet, ETH_ALEN);

	dbg(wpa->nas, "FT: RRB authentication response: STA= %s"
	   " CurrentAP= %s status=%d\n",
	   ether_etoa(sta_addr, eabuf_tmp1), ether_etoa(current_ap, eabuf_tmp2), status);
	dbg(wpa->nas, "FT: Response IEs");
	dump(wpa->nas, resp_ies, resp_ies_len);

	nas_get_bssid(wpa->nas, bssid, sizeof(bssid));

	/* RRB - Forward action frame response to the Current AP */

	/*
	 * data: Category[1] Action[1] STA_Address[6] Target_AP_Address[6]
	 * Status_Code[2] FT Request action frame body[variable]
	 */
	rlen = 2 + 2 * ETH_ALEN + 2 + resp_ies_len;

	frame = malloc(sizeof(*frame) + rlen);
	frame->frame_type = RSN_REMOTE_FRAME_TYPE_FT_RRB;
	frame->packet_type = FT_PACKET_RESPONSE;
	frame->action_length = host_to_le16(rlen);
	memcpy(frame->ap_address, bssid, ETH_ALEN);
	memcpy(frame->dest_ap_address, dest_bssid, ETH_ALEN);
	pos = (uint8 *) (frame + 1);
	*pos++ = WLAN_ACTION_FT;
	*pos++ = 2; /* Action: Response */
	memcpy(pos, sta_addr, ETH_ALEN);
	pos += ETH_ALEN;
	memcpy(pos, bssid, ETH_ALEN);
	pos += ETH_ALEN;
	WPA_PUT_LE16(pos, status);
	pos += 2;

	wpa_auth_ft_build_auth_resp(wpa, sta,
		(uint8 *)sta_addr, status, resp_ies, resp_ies_len, buf, &auth_resp_len);
	if (resp_ies) {
		memcpy(pos, resp_ies, resp_ies_len);
	}
	if (status == DOT11_SC_SUCCESS)
	{
		err = nas_set_fbt_ds_add_sta(wpa->nas, buf, auth_resp_len);
	}
	if (err >= 0)
	{
		dbg(wpa->nas, "IEEE 802.11 FT Authentication request from %s to BSSID %s \n",
				ether_etoa(sta_addr, eabuf_tmp1),
				ether_etoa((const uint8*)bssid, eabuf_tmp2));
		wpa_ft_rrb_send(wpa, (uint8 *)current_ap, (uint8 *) frame,
				sizeof(*frame) + rlen);
		if (sta->auth_retx == 0) {
			/* plumb pairwise key */
			if (nas_set_key(wpa->nas, &sta->ea, wpa->PTK.tk1,
					sizeof(wpa->PTK.tk1), 0, 1, 0, 0, 0) < 0) {
				dbg(wpa->nas, "unicast key rejected by driver, assuming too"
						"many associated STAs");
				cleanup_sta(wpa->nas, sta, DOT11_RC_BUSY, 0);
				free(frame);
				free(sta);
				return err;
			}
		}
	}
	else
	{
		wpa_ft_rrb_send(wpa, (uint8 *)current_ap, (uint8 *) frame,
			sizeof(*frame) + rlen);
	}

	free(frame);
	free(sta);
	return err;
}

/* Process Over the air authentication request received from driver */
void wpa_ft_process_auth(wpa_t *wpa, nas_sta_t *sta, uint8* data, uint16 len)
{
	uint16 status;
	uint8 *resp_ies;
	size_t resp_ies_len;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
	int32 result;
	uint8 pend_auth_iter;
	struct timeval now;
	int pairwise = 0;
	struct wpa_ft_ies parse;
	uint8 pmk_r1[PMK_LEN], pmk_r1_name[WPA_PMK_NAME_LEN];

	dbg(wpa->nas, "FT: Received authentication frame: STA= %s",
		ether_etoa((uchar *)&sta->ea, eabuf));
	result = wpa_ft_process_auth_req(wpa, sta, data, len, pmk_r1, pmk_r1_name,
		       &pairwise, &parse); /* process ft auth request */

	if (result < 0) {
		dbg(wpa->nas, "FT: Auth Response Pending");
		/* Find a slot. It needs to be optimized when there are more entries in future */
		for (pend_auth_iter = 0; pend_auth_iter < MAX_PEND_REQ &&
				pend_auth_req[pend_auth_iter].valid == TRUE; pend_auth_iter++);
		/* If you do not find any slot, fill the very first slot */
		if (pend_auth_iter == MAX_PEND_REQ)
			pend_auth_iter = 0;

		/* Pull PMK-R1 response is not available yet. Store required
		 * info to process auth request once PMK-R1 pull response is available
		 */
		pend_auth_req[pend_auth_iter].sta = sta;
		pend_auth_req[pend_auth_iter].parse = &parse;
		pend_auth_req[pend_auth_iter].overds = FALSE;
		pend_auth_req[pend_auth_iter].valid = TRUE;
		gettimeofday(&now, NULL);
		WPA_PUT_LE32(pend_auth_req[pend_auth_iter].timestamp, now.tv_sec);
		return;
	}

	status = result;
	dbg(wpa->nas, "FT: Selected PMK-R1");
	dump(wpa->nas, pmk_r1, PMK_LEN);

	sta->suppl.ft_info.pmk_r1_name_valid = 1;
	memcpy(sta->suppl.ft_info.pmk_r1_name, pmk_r1_name, WPA_PMK_NAME_LEN);

	wpa_ft_prepare_auth_resp(wpa, sta, pmk_r1, parse, pairwise, &resp_ies, &resp_ies_len);
	wpa_ft_process_ota_auth(wpa, sta, resp_ies, resp_ies_len, status);
	free(resp_ies);
	resp_ies = NULL;
	return;
}

/* Process Over the DS authentication request received from peer AP */
static int wpa_ft_rrb_rx_request(wpa_t *wpa,
		const uint8 *current_ap, const uint8 *dest_bssid, const uint8 *sta_addr,
		const uint8 *body, size_t len)
{
	uint16 status;
	nas_sta_t *nas_sta;
	uint8 *resp_ies;
	size_t resp_ies_len;
	int32 err = -1, result;
	uint8 pend_auth_iter;
	struct timeval now;
	int pairwise = 0;
	struct wpa_ft_ies parse;
	uint8 pmk_r1[PMK_LEN], pmk_r1_name[WPA_PMK_NAME_LEN];

	nas_sta = malloc(sizeof(nas_sta_t));
	memcpy(nas_sta->ea.octet, sta_addr, ETH_ALEN);

	result = wpa_ft_process_auth_req(wpa, nas_sta, (uint8 *)body, len, pmk_r1, pmk_r1_name,
			&pairwise, &parse);
	if (result < 0) {

		dbg(wpa->nas, "FT: Auth Response Pending");
		/* Find a slot. It needs to be optimized when there are more entries in future */
		for (pend_auth_iter = 0; pend_auth_iter < MAX_PEND_REQ &&
				pend_auth_req[pend_auth_iter].valid == TRUE; pend_auth_iter++);
		/* If there is no slot available, fill the very first slot */
		if (pend_auth_iter == MAX_PEND_REQ)
			pend_auth_iter = 0;

		/* Pull PMK-R1 response is not available yet. Store required
		 * info to process auth request once PMK-R1 pull response is available
		 */
		memcpy(pend_auth_req[pend_auth_iter].current_ap, current_ap, ETH_ALEN);
		memcpy(pend_auth_req[pend_auth_iter].dest_bssid, dest_bssid, ETH_ALEN);
		pend_auth_req[pend_auth_iter].sta = nas_sta;
		pend_auth_req[pend_auth_iter].parse = &parse;
		pend_auth_req[pend_auth_iter].overds = TRUE;
		pend_auth_req[pend_auth_iter].valid = TRUE;
		gettimeofday(&now, NULL);
		WPA_PUT_LE32(pend_auth_req[pend_auth_iter].timestamp, now.tv_sec);
		return 0;
	}
	status = result;

	dbg(wpa->nas, "FT: Selected PMK-R1");
	dump(wpa->nas, pmk_r1, PMK_LEN);

	nas_sta->suppl.ft_info.pmk_r1_name_valid = 1;
	memcpy(nas_sta->suppl.ft_info.pmk_r1_name, pmk_r1_name, WPA_PMK_NAME_LEN);

	wpa_ft_prepare_auth_resp(wpa, nas_sta, pmk_r1, parse, pairwise, &resp_ies, &resp_ies_len);
	err = wpa_ft_process_otd_auth(wpa, nas_sta, current_ap, dest_bssid,
		resp_ies, resp_ies_len, status);
	free(resp_ies);
	resp_ies = NULL;
	return err;
}

/* Process PMKR1 pull request and send pull response */
static int wpa_ft_rrb_rx_pull(wpa_t *wpa,
		uint8 *src_addr, uint8 *dest_bssid,
		uint8 *data, size_t data_len)
{
	struct ft_r0kh_r1kh_pull_frame *frame, f;
	struct ft_r0kh_r1kh_resp_frame resp, r;
	uint8 pmk_r0[PMK_LEN];
	int pairwise;
	char buf[WLC_IOCTL_SMLEN];
	struct ft_remote_r1kh *r1kh;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
	char eabuf1[ETHER_ADDR_STR_LEN];
#endif // endif

	dbg(wpa->nas, "FT: Received PMK-R1 pull");

	if (data_len < sizeof(*frame))
		return -1;

	memset(&f, 0, sizeof(f));

	r1kh = wpa->fbt_info.r1kh_list;
	while (r1kh) {
		if ((memcmp(r1kh->br_addr, src_addr, ETH_ALEN) == 0) &&
			(memcmp(r1kh->addr, dest_bssid, ETH_ALEN) == 0))
			break;
		r1kh = r1kh->next;
	}
	if (r1kh == NULL) {
		dbg(wpa->nas, "FT: No matching R1KH address found for "
		"PMK-R1 pull source address %s", ether_etoa(src_addr, eabuf));
		return -1;
	}

	frame = (struct ft_r0kh_r1kh_pull_frame *) data;

	/* aes_unwrap() does not support inplace decryption, so use a temporary
	 * buffer for the data.
	 */
	if (aes_unwrap(strlen((char *)r1kh->key), (uint8 *)r1kh->key,
			FT_R0KH_R1KH_PULL_DATA_LEN + 4 + 8, frame->nonce, f.nonce) != 0) {
		dbg(wpa->nas, "FT: Failed to decrypt PMK-R1 pull "
			"request from %s", ether_etoa(src_addr, eabuf));
		return -1;
	}

	dbg(wpa->nas, "FT: PMK-R1 pull - nonce");
	dump(wpa->nas, f.nonce, sizeof(f.nonce));
	dbg(wpa->nas, "FT: PMK-R1 pull - PMKR0Name");
	dump(wpa->nas, f.pmk_r0_name, WPA_PMK_NAME_LEN);
	dbg(wpa->nas, "FT: PMK-R1 pull - R1KH-ID=%s S1KH-ID=%s",
		ether_etoa(f.r1kh_id, eabuf), ether_etoa(f.s1kh_id, eabuf1));

	memset(&resp, 0, sizeof(resp));
	resp.frame_type = RSN_REMOTE_FRAME_TYPE_FT_RRB;
	resp.packet_type = FT_PACKET_R0KH_R1KH_RESP;
	resp.data_length = host_to_le16(FT_R0KH_R1KH_RESP_DATA_LEN);
	nas_get_bssid(wpa->nas, buf, sizeof(buf));
	memcpy(resp.ap_address, buf, ETH_ALEN);
	memcpy(resp.dest_ap_address, dest_bssid, ETH_ALEN);

	/* aes_wrap() does not support inplace encryption, so use a temporary
	 * buffer for the data.
	 */
	memcpy(r.nonce, f.nonce, sizeof(f.nonce));
	memcpy(r.r1kh_id, f.r1kh_id, FT_R1KH_ID_LEN);
	memcpy(r.s1kh_id, f.s1kh_id, ETH_ALEN);

	if (wpa_ft_fetch_pmk_r0(wpa, f.s1kh_id, f.pmk_r0_name, pmk_r0,
		&pairwise) < 0) {
		dbg(wpa->nas, "FT: No matching PMKR0Name found for PMK-R1 pull");
		return -1;
	}

	wpa_calc_pmkR1((struct ether_addr *) f.r1kh_id, (struct ether_addr *)f.s1kh_id,
			pmk_r0, PMK_LEN, f.pmk_r0_name, r.pmk_r1, r.pmk_r1_name);
	dbg(wpa->nas, "FT: PMK-R1");
	dump(wpa->nas, r.pmk_r1, PMK_LEN);
	dbg(wpa->nas, "FT: PMKR1Name");
	dump(wpa->nas, r.pmk_r1_name, WPA_PMK_NAME_LEN);

	r.pairwise = host_to_le16(pairwise);

	aes_wrap(strlen((char *)r1kh->key), (uint8 *)r1kh->key, FT_R0KH_R1KH_RESP_DATA_LEN + 4,
			r.nonce, resp.nonce);
	memset(pmk_r0, 0, PMK_LEN);

	wpa_ft_rrb_send(wpa, r1kh->br_addr, (uint8 *) &resp, sizeof(resp));

	return 0;
}

/* Process PMKR1 pull response */
static int wpa_ft_rrb_rx_resp(wpa_t *wpa,
		uint8 *src_addr, uint8 *dest_bssid,
		uint8 *data, size_t data_len)
{
	struct ft_r0kh_r1kh_resp_frame *frame, f;
	struct ft_remote_r0kh *r0kh;
	int pairwise;
#ifdef BCMDBG
	char eabuf_tmp1[ETHER_ADDR_STR_LEN];
	char eabuf_tmp2[ETHER_ADDR_STR_LEN];
#endif // endif
	uint8 pend_auth_iter;
	time_t saved_ts;
	struct timeval now;
	uint8 *resp_ies;
	size_t resp_ies_len;
	uint16 status;

	dbg(wpa->nas, "FT: Received PMK-R1 pull response");

	if (data_len < sizeof(*frame))
		return -1;

	memset(&f, 0, sizeof(f));

	r0kh = wpa->fbt_info.r0kh_list;
	while (r0kh) {
		if ((memcmp(r0kh->br_addr, src_addr, ETH_ALEN) == 0) &&
			(memcmp(r0kh->addr, dest_bssid, ETH_ALEN) == 0))
			break;
		r0kh = r0kh->next;
	}
	if (r0kh == NULL) {
		dbg(wpa->nas, "FT: No matching R0KH address found for "
		"PMK-R1 pull response source address %s", ether_etoa(src_addr, eabuf_tmp1));
		return -1;
	}

	frame = (struct ft_r0kh_r1kh_resp_frame *) data;

	/* aes_unwrap() does not support inplace decryption, so use a temporary
	 * buffer for the data.
	 */
	if (aes_unwrap(strlen((const char *)r0kh->key), (uint8 *)r0kh->key,
			FT_R0KH_R1KH_RESP_DATA_LEN + 4 + 8, frame->nonce, f.nonce) != 0) {
	        dbg(wpa->nas, "FT: Failed to decrypt PMK-R1 pull "
		"response from %s\n", ether_etoa(src_addr, eabuf_tmp1));
		return -1;
	}

	if (memcmp(f.r1kh_id, wpa->fbt_info.r1_key_holder, FT_R1KH_ID_LEN) != 0) {
		dbg(wpa->nas, "FT: PMK-R1 pull response did not use a matching R1KH-ID");
		return -1;
	}

	pairwise = le_to_host16(f.pairwise);
	dbg(wpa->nas, "FT: PMK-R1 pull - nonce");
	dump(wpa->nas, f.nonce, sizeof(f.nonce));
	dbg(wpa->nas, "FT: PMK-R1 pull - R1KH-ID= %s S1KH-ID=%s pairwise=0x%x",
		ether_etoa(f.r1kh_id, eabuf_tmp1), ether_etoa(f.s1kh_id, eabuf_tmp2), pairwise);
	dbg(wpa->nas, "FT: PMK-R1 pull - PMK-R1");
	dump(wpa->nas, f.pmk_r1, PMK_LEN);
	dbg(wpa->nas, "FT: PMK-R1 pull - PMKR1Name");
	dump(wpa->nas, f.pmk_r1_name, WPA_PMK_NAME_LEN);

	wpa_ft_store_pmk_r1(wpa, f.s1kh_id, f.pmk_r1, f.pmk_r1_name,
		pairwise);
	/* Process pending auth request, if any */
	for (pend_auth_iter = 0; pend_auth_iter < MAX_PEND_REQ; pend_auth_iter++)
	{
		/* Ignore entries which are invalid */
		if (pend_auth_req[pend_auth_iter].valid == FALSE)
			continue;

		/* Invalidate entries if they are older than 60 seconds */
		gettimeofday(&now, NULL);
		saved_ts = WPA_GET_LE32(pend_auth_req[pend_auth_iter].timestamp);
		if ((now.tv_sec > saved_ts && now.tv_sec - saved_ts > 60) ||
			(now.tv_sec < saved_ts && saved_ts - now.tv_sec > 60)) {
			dbg(wpa->nas, "FT: PMK-R1 pull did not have a valid "
					"timestamp: saved time %d current time %d",
					(int) saved_ts, (int) now.tv_sec);
			pend_auth_req[pend_auth_iter].valid = FALSE;
		}
		/* Process valid entries */
		if (pend_auth_req[pend_auth_iter].valid == TRUE &&
			!memcmp(pend_auth_req[pend_auth_iter].sta->ea.octet, f.s1kh_id, ETH_ALEN)) {

			status = DOT11_SC_SUCCESS;
			dbg(wpa->nas, "FT: Selected PMK-R1");
			dump(wpa->nas, f.pmk_r1, PMK_LEN);

			pend_auth_req[pend_auth_iter].sta->suppl.ft_info.pmk_r1_name_valid = 1;
			memcpy(pend_auth_req[pend_auth_iter].sta->suppl.ft_info.pmk_r1_name,
					f.pmk_r1_name, WPA_PMK_NAME_LEN);

			if (pend_auth_req[pend_auth_iter].overds == TRUE) {
				wpa_ft_prepare_auth_resp(wpa, pend_auth_req[pend_auth_iter].sta,
						f.pmk_r1, *(pend_auth_req[pend_auth_iter].parse),
						f.pairwise, &resp_ies, &resp_ies_len);
				wpa_ft_process_otd_auth(wpa, pend_auth_req[pend_auth_iter].sta,
						pend_auth_req[pend_auth_iter].current_ap,
						pend_auth_req[pend_auth_iter].dest_bssid,
						resp_ies,
						resp_ies_len, status);
				free(resp_ies);
				resp_ies = NULL;
			}
			else {
				wpa_ft_prepare_auth_resp(wpa, pend_auth_req[pend_auth_iter].sta,
						f.pmk_r1, *(pend_auth_req[pend_auth_iter].parse),
						f.pairwise, &resp_ies, &resp_ies_len);
				wpa_ft_process_ota_auth(wpa, pend_auth_req[pend_auth_iter].sta,
						resp_ies, resp_ies_len, status);
				free(resp_ies);
				resp_ies = NULL;
			}
			/* Invalidate the entry once it is processed */
			pend_auth_req[pend_auth_iter].valid = FALSE;
			break;
		}
	}
	memset(f.pmk_r1, 0, PMK_LEN);

	return 0;
}

/* Process received PMKR1 push message and save PMKR1 in it's cache */
static int wpa_ft_rrb_rx_push(wpa_t *wpa,
		uint8 *src_addr, uint8 *dest_bssid,
		uint8 *data, size_t data_len)
{
	struct ft_r0kh_r1kh_push_frame *frame, f;
	struct ft_remote_r0kh *r0kh;
	struct timeval now;
	time_t tsend;
	int pairwise;
	char buf[WLC_IOCTL_SMLEN];
#ifdef BCMDBG
	char eabuf_tmp1[ETHER_ADDR_STR_LEN];
	char eabuf_tmp2[ETHER_ADDR_STR_LEN];
#endif // endif

	dbg(wpa->nas, "FT: Received PMK-R1 push\n");

	if (data_len < sizeof(*frame))
		return -1;

	r0kh = wpa->fbt_info.r0kh_list;
	while (r0kh) {
		if ((memcmp(r0kh->br_addr, src_addr, ETH_ALEN) == 0) &&
			(memcmp(r0kh->addr, dest_bssid, ETH_ALEN) == 0))
			break;
		r0kh = r0kh->next;
	}

	if (r0kh == NULL) {
		dbg(wpa->nas, "FT: No matching R0KH address found for "
		"PMK-R0 push source address %s", ether_etoa(src_addr, eabuf_tmp1));
		return -1;
	}

	frame = (struct ft_r0kh_r1kh_push_frame *) data;
	/* aes_unwrap() does not support inplace decryption, so use a temporary
	 * buffer for the data.
	 */
	if (aes_unwrap(strlen((const char *)r0kh->key), (uint8 *)r0kh->key,
			FT_R0KH_R1KH_PUSH_DATA_LEN + 8,
			frame->timestamp, f.timestamp) != 0) {
		dbg(wpa->nas, "FT: Failed to decrypt PMK-R1 push from %s",
				ether_etoa(src_addr, eabuf_tmp1));
		return -1;
	}

	gettimeofday(&now, NULL);
	tsend = WPA_GET_LE32(f.timestamp);
	if ((now.tv_sec > tsend && now.tv_sec - tsend > 60) ||
	    (now.tv_sec < tsend && tsend - now.tv_sec > 60)) {
	        dbg(wpa->nas, "FT: PMK-R1 push did not have a valid "
			   "timestamp: sender time %d own time %d\n",
			   (int) tsend, (int) now.tv_sec);
	}

	memset(buf, 0, sizeof(buf));
	nas_get_fbt_r1kh(wpa->nas, buf, sizeof(buf));

	if (memcmp(f.r1kh_id, buf, FT_R1KH_ID_LEN) != 0) {
		dbg(wpa->nas, "FT: PMK-R1 push did not use a matching "
				"R1KH-ID (received %s own %s)",
				ether_etoa(f.r1kh_id, eabuf_tmp1),
				ether_etoa((const unsigned char *)buf, eabuf_tmp2));
		return -1;
	}

	pairwise = le_to_host16(f.pairwise);
	dbg(wpa->nas, "FT: PMK-R1 push - R1KH-ID= %s S1KH-ID= %s"
			"pairwise=0x%x\n", ether_etoa(f.r1kh_id, eabuf_tmp1),
			ether_etoa(f.s1kh_id, eabuf_tmp2), pairwise);
	dbg(wpa->nas, "FT: PMK-R1 push - PMK-R1");
	dump(wpa->nas, f.pmk_r1, PMK_LEN);
	dbg(wpa->nas, "FT: PMK-R1 push - PMKR1Name");
	dump(wpa->nas, f.pmk_r1_name, WPA_PMK_NAME_LEN);

	wpa_ft_store_pmk_r1(wpa, f.s1kh_id, f.pmk_r1, f.pmk_r1_name, pairwise);
	memset(f.pmk_r1, 0, PMK_LEN);

	return 0;
}

/* Process received RRB messages from peer AP */
int wpa_ft_rrb_rx(wpa_t *wpa, const uint8 *src_addr, const uint8 *dest_addr,
		const uint8 *data, size_t data_len)
{
	struct ft_rrb_frame *frame;
	uint16 alen;
	const uint8 *pos, *end, *start;
	uint8 action;
	const uint8 *sta_addr, *target_ap_addr;
	char bssid[WLC_IOCTL_SMLEN];
#ifdef BCMDBG
	char eabuf_tmp1[ETHER_ADDR_STR_LEN];
	char eabuf_tmp2[ETHER_ADDR_STR_LEN];
#endif // endif
	char *own_et_mac;
	uint8 own_addr[ETH_ALEN];

	dbg(wpa->nas, "FT: RRB received frame from remote AP %s dest %s\n",
		ether_etoa(src_addr, eabuf_tmp1), ether_etoa(dest_addr, eabuf_tmp2));

	/* Get the own bridge address to drop the frame sent from own AP */
	own_et_mac = nvram_get("lan_hwaddr");
	ether_atoe(own_et_mac, own_addr);

	nas_get_bssid(wpa->nas, bssid, sizeof(bssid));

	if (memcmp(dest_addr, own_addr, ETH_ALEN) != 0) {
		dbg(wpa->nas, "FT: RRB received frame with destination MAC %s not matching"
				"own address dropping\n", eabuf_tmp2);
		return -1;
	}

	if (data_len < sizeof(*frame)) {
		dbg(wpa->nas, "FT: Too short RRB frame (data_len=%lu)", (unsigned long) data_len);
		return -1;
	}

	pos = data;
	frame = (struct ft_rrb_frame *) pos;
	pos += sizeof(*frame);

	alen = le_to_host16(frame->action_length);
	dbg(wpa->nas, "FT: RRB frame - frame_type=%d packet_type=%d "
		   "action_length=%d from ap_address=%s to ap_ddress=%s\n",
		   frame->frame_type, frame->packet_type, alen,
		   ether_etoa(frame->ap_address, eabuf_tmp1),
		   ether_etoa(frame->dest_ap_address, eabuf_tmp2));

	if (frame->frame_type != RSN_REMOTE_FRAME_TYPE_FT_RRB) {
		/* Discard frame per IEEE Std 802.11r-2008, 11A.10.3 */
		dbg(wpa->nas, "FT: RRB discarded frame with "
			   "unrecognized type %d\n", frame->frame_type);
		return -1;
	}

	if (memcmp(frame->dest_ap_address, bssid, ETH_ALEN) != 0) {
		dbg(wpa->nas, "FT: Dest AP address=%s in the "
				"RRB Request does not match with own address=%s",
				ether_etoa(frame->dest_ap_address, eabuf_tmp1),
				ether_etoa((const uint8 *)bssid, eabuf_tmp2));
		return 0;
	}

	if (alen > data_len - sizeof(*frame)) {
		dbg(wpa->nas, "FT: RRB frame too short for action frame\n");
		return -1;
	}

	if (frame->packet_type == FT_PACKET_R0KH_R1KH_PULL)
		return wpa_ft_rrb_rx_pull(wpa, (uint8 *)src_addr, frame->ap_address,
			(uint8 *)data, data_len);
	if (frame->packet_type == FT_PACKET_R0KH_R1KH_RESP)
		return wpa_ft_rrb_rx_resp(wpa, (uint8 *)src_addr, frame->ap_address,
			(uint8 *)data, data_len);
	if (frame->packet_type == FT_PACKET_R0KH_R1KH_PUSH)
		return wpa_ft_rrb_rx_push(wpa, (uint8 *)src_addr, frame->ap_address,
			(uint8 *)data, data_len);

	dbg(wpa->nas, "FT: RRB - FT Action frame:");
	dump(wpa->nas, (uint8 *)pos, alen);

	if (alen < 1 + 1 + 2 * ETH_ALEN) {
		dbg(wpa->nas, "FT: Too short RRB frame (not enough "
			   "room for Action Frame body); alen=%lu",
			   (unsigned long) alen);
		return -1;
	}
	start = pos;
	end = pos + alen;

	if (*pos != WLAN_ACTION_FT) {
		dbg(wpa->nas, "FT: Unexpected Action frame category %d\n", *pos);
		return -1;
	}

	pos++;
	action = *pos++;
	sta_addr = pos;
	pos += ETH_ALEN;
	target_ap_addr = pos;
	pos += ETH_ALEN;
	dbg(wpa->nas, "FT: RRB Action Frame: action=%d sta_addr= %s target_ap_addr= %s\n",
	   action, ether_etoa(sta_addr, eabuf_tmp1), ether_etoa(target_ap_addr, eabuf_tmp2));

	if (frame->packet_type == FT_PACKET_REQUEST) {
	        dbg(wpa->nas, "FT: FT Packet Type - Request\n");

		if (action != 1) {
	                dbg(wpa->nas, "FT: Unexpected Action %d in RRB Request\n", action);
			return -1;
		}

		if (memcmp(target_ap_addr, bssid, ETH_ALEN) != 0) {
			dbg(wpa->nas, "FT: Target AP address in the "
				"RRB Request does not match with own address");
			return -1;
		}

		if (wpa_ft_rrb_rx_request(wpa, src_addr, frame->ap_address,
			sta_addr, pos, end - pos) < 0)
			return -1;
	} else if (frame->packet_type == FT_PACKET_RESPONSE) {
#ifdef BCMDBG
		uint16 status_code;
#endif /* BCMDBG */

		if (end - pos < 2) {
			dbg(wpa->nas, "FT: Not enough room for status code in RRB Response");
			return -1;
		}
#ifdef BCMDBG
		status_code = WPA_GET_LE16(pos);
#endif /* BCMDBG */
		pos += 2;

	        dbg(wpa->nas, "FT: FT Packet Type - Response "
			   "(status_code=%d)\n", status_code);

		if (wpa_ft_action_send(wpa, sta_addr, start, alen) < 0)
			return -1;
	} else {
		dbg(wpa->nas, "FT: RRB discarded frame with unknown "
			   "packet_type %d", frame->packet_type);
		return -1;
	}

	return 0;
}

/* Calculate and push PMKR1s to all R1KHs using RRB protocol */
void wpa_ft_push_pmk_r1(wpa_t *wpa)
{
	struct ft_r0kh_r1kh_push_frame frame, f;
	struct timeval now;
	struct wpa_ft_pmk_r0_sa *r0;
	struct ft_remote_r1kh *r1kh_list;

	/* For all the R1KHs */
	r1kh_list = wpa->fbt_info.r1kh_list;
	while (r1kh_list) {
		dump(wpa->nas, r1kh_list->id, FT_R1KH_ID_LEN);
		dump(wpa->nas, r1kh_list->addr, ETH_ALEN);
		dump(wpa->nas, r1kh_list->key, KH_KEY_LEN);

		/* Generate PMKR1 for each R1KH using cached PMKR0 */
		if (!wpa->fbt_info.ft_pmk_cache) {
			dbg(wpa->nas, "FT: cache is NULL\n");
			return;
		}

		r0 = wpa->fbt_info.ft_pmk_cache->pmk_r0;
		while (r0) {
			memset(&frame, 0, sizeof(frame));
			frame.frame_type = RSN_REMOTE_FRAME_TYPE_FT_RRB;
			frame.packet_type = FT_PACKET_R0KH_R1KH_PUSH;
			frame.data_length = host_to_le16(FT_R0KH_R1KH_PUSH_DATA_LEN);
			memcpy(frame.ap_address, wpa->fbt_info.r1_key_holder, ETH_ALEN);
			memcpy(frame.dest_ap_address, r1kh_list->addr, ETH_ALEN);

			/* aes_wrap() does not support inplace encryption, so use a temporary
			 * buffer for the data.
			 */
			memcpy(f.r1kh_id, r1kh_list->id, FT_R1KH_ID_LEN);
			memcpy(f.s1kh_id, r0->spa, ETH_ALEN);
			memcpy(f.pmk_r0_name, r0->pmk_r0_name, WPA_PMK_NAME_LEN);
			wpa_calc_pmkR1((struct ether_addr *)f.r1kh_id,
					(struct ether_addr *)f.s1kh_id, (uint8 *)r0->pmk_r0,
					PMK_LEN, (uint8 *)r0->pmk_r0_name, f.pmk_r1, f.pmk_r1_name);
			dbg(wpa->nas, "FT: R1KH-ID");
			dump(wpa->nas, f.r1kh_id, FT_R1KH_ID_LEN);
			dbg(wpa->nas, "FT: PMK-R1");
			dump(wpa->nas, f.pmk_r1, PMK_LEN);
			dbg(wpa->nas, "FT: PMKR1Name");
			dump(wpa->nas, f.pmk_r1_name, WPA_PMK_NAME_LEN);
			gettimeofday(&now, NULL);
			WPA_PUT_LE32(f.timestamp, now.tv_sec);
			dbg(wpa->nas, "pairwise %d", r0->pairwise);
			f.pairwise = host_to_le16(r0->pairwise);
			dbg(wpa->nas, "b4 aes_wrap pairwise %d\n", r0->pairwise);

			if (aes_wrap(KH_KEY_LEN, (uint8 *)r1kh_list->key,
					FT_R0KH_R1KH_PUSH_DATA_LEN,
					f.timestamp, frame.timestamp) != 0) {
				dbg(wpa->nas, "Failed to encrypt message");
				return;
			}

			dbg(wpa->nas, "after aes_wrap pairwise %d", frame.pairwise);

			/* Send it to destination address */
			wpa_ft_rrb_send(wpa, r1kh_list->br_addr, (uint8 *) &frame, sizeof(frame));
			r0 = r0->next;
		}
		r1kh_list = r1kh_list->next;
	}
	return;
}

/* Fill the FBT action frame to send it to the destination */
void
wpa_ft_process_auth_action(wpa_t *wpa, uint8* data, uint16 data_len)
{
	char bssid[WLC_IOCTL_SMLEN];
	uint16 len = 0;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	struct ft_r0kh_r1kh_action_frame *fbt_rrb;
	struct ft_remote_r1kh *r1kh;
	dot11_ft_req_t *fbt_req = (dot11_ft_req_t *)data;

	/* Find the address in r1kh_list to get the bridge address */
	r1kh = wpa->fbt_info.r1kh_list;
	while (r1kh) {
		if (memcmp(r1kh->addr, fbt_req->tgt_ap_addr, ETH_ALEN) == 0)
			break;
		r1kh = r1kh->next;
	}
	if (r1kh == NULL) {
		dbg(wpa->nas, "FT: No matching R1KH address found to send action "
			"frame destination address %s", ether_etoa(fbt_req->tgt_ap_addr, eabuf));
		return;
	}

	/* Allocate memory for action frame */
	len = sizeof(struct ft_r0kh_r1kh_action_frame) + data_len;

	fbt_rrb = malloc(len);
	if (!fbt_rrb) {
		dbg(wpa->nas, "FT: Not enough memory to build RRB frame to destination %s",
			ether_etoa(fbt_req->tgt_ap_addr, eabuf));
		return;
	}

	memset(fbt_rrb, 0, len);
	fbt_rrb->frame_type = RSN_REMOTE_FRAME_TYPE_FT_RRB;
	fbt_rrb->packet_type = FT_PACKET_REQUEST;
	fbt_rrb->len = host_to_le16(data_len);
	nas_get_bssid(wpa->nas, bssid, sizeof(bssid));
	memcpy(fbt_rrb->cur_ap_addr, bssid, ETHER_ADDR_LEN);
	memcpy(fbt_rrb->dest_ap_addr, r1kh->addr, ETHER_ADDR_LEN);
	memcpy(&fbt_rrb->data[0], data, data_len);

	/* Send it to destination */
	wpa_ft_rrb_send(wpa, r1kh->br_addr, (uint8 *)fbt_rrb, len);

	free(fbt_rrb);
}

#endif /* WLHOSTFBT */
