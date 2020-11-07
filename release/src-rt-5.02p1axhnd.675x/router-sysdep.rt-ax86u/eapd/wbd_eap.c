/*
 * Application-specific portion of EAPD
 * (WBD)
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
 * $Id: wbd_eap.c 762628 2018-05-15 11:38:37Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <ethernet.h>
#include <eapol.h>
#include <eap.h>
#include <bcmendian.h>
#include <wlutils.h>
#include <eapd.h>
#include <shutils.h>
#include <UdpLib.h>
#include <security_ipc.h>
#include <bcmnvram.h>

void
wbd_app_recv_handler(eapd_wksp_t *nwksp, char *wlifname, eapd_cb_t *from,
	uint8 *pData, int *pLen, struct ether_addr *ap_ea)
{
	EAPD_INFO("WBD: Do nothing for eapd-wbd receiver currently.\n");
	return;
}

void
wbd_app_set_eventmask(eapd_app_t *app)
{
	memset(app->bitvec, 0, sizeof(app->bitvec));
	setbit(app->bitvec, WLC_E_DISASSOC_IND);
	setbit(app->bitvec, WLC_E_DEAUTH_IND);
	setbit(app->bitvec, WLC_E_DEAUTH);

	/* If the multi hop is disabled, then only register for the event */
	if (nvram_match("map_no_multihop", "1")) {
		setbit(app->bitvec, WLC_E_PRE_ASSOC_IND);
		setbit(app->bitvec, WLC_E_PRE_REASSOC_IND);
	}

	setbit(app->bitvec, WLC_E_BSSTRANS_RESP);
	setbit(app->bitvec, WLC_E_RADAR_DETECTED);
	setbit(app->bitvec, WLC_E_RRM);
	setbit(app->bitvec, WLC_E_AP_CHAN_CHANGE);
	setbit(app->bitvec, WLC_E_ASSOC);
	setbit(app->bitvec, WLC_E_REASSOC);
	setbit(app->bitvec, WLC_E_ASSOC_REASSOC_IND_EXT);
	setbit(app->bitvec, WLC_E_LINK);
	setbit(app->bitvec, WLC_E_CAC_STATE_CHANGE);
	setbit(app->bitvec, WLC_E_ASSOC_FAIL);
	setbit(app->bitvec, WLC_E_REASSOC_FAIL);
	setbit(app->bitvec, WLC_E_AUTH_FAIL);
	/* Register MAP-MBO-R2 events */
	setbit(app->bitvec, WLC_E_MBO_CAPABILITY_STATUS);
	setbit(app->bitvec, WLC_E_WNM_NOTIFICATION_REQ);
	setbit(app->bitvec, WLC_E_WNM_BSSTRANS_QUERY);
	setbit(app->bitvec, WLC_E_GAS_RQST_ANQP_QUERY);
	setbit(app->bitvec, WLC_E_ESCAN_RESULT);
	setbit(app->bitvec, WLC_E_BSSTRANS_QUERY);

	return;
}

int
wbd_app_init(eapd_wksp_t *nwksp)
{
	int reuse = 1;
	eapd_wbd_t *wbd;
	eapd_cb_t *cb;
	struct sockaddr_in addr;

	if (nwksp == NULL)
		return -1;

	wbd = &nwksp->wbd;
	wbd->appSocket = -1;

	cb = wbd->cb;
	if (cb == NULL) {
		EAPD_INFO("WBD: No any wbd application need to run.\n");
		return 0;
	}

	while (cb) {
		EAPD_INFO("WBD: init brcm interface %s \n", cb->ifname);
		cb->brcmSocket = eapd_add_brcm(nwksp, cb->ifname);
		if (!cb->brcmSocket)
			return -1;
		/* set this brcmSocket have wbd capability */
		cb->brcmSocket->flag |= EAPD_CAP_WBD;

		cb = cb->next;
	}

	/* appSocket for wbd */
	wbd->appSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (wbd->appSocket < 0) {
		EAPD_ERROR("WBD: UDP Open failed.\n");
		return -1;
	}
	if (setsockopt(wbd->appSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) < 0) {
		EAPD_ERROR("WBD: UDP setsockopt failed.\n");
		close(wbd->appSocket);
		wbd->appSocket = -1;
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = EAPD_UDP_SIN_ADDR(nwksp);
	addr.sin_port = htons(EAPD_WKSP_WBD_UDP_RPORT);
	if (bind(wbd->appSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		EAPD_ERROR("WBD: UDP Bind failed, close wbd appSocket %d\n", wbd->appSocket);
		close(wbd->appSocket);
		wbd->appSocket = -1;
		return -1;
	}
	EAPD_INFO("WBD: wbd appSocket %d opened\n", wbd->appSocket);

	return 0;
}

int
wbd_app_deinit(eapd_wksp_t *nwksp)
{
	eapd_wbd_t *wbd;
	eapd_cb_t *cb, *tmp_cb;

	if (nwksp == NULL) {
		EAPD_ERROR("WBD: Wrong argument...\n");
		return -1;
	}

	wbd = &nwksp->wbd;
	cb = wbd->cb;
	while (cb) {
		/* close  brcm drvSocket */
		if (cb->brcmSocket) {
			EAPD_INFO("WBD: close wbd brcmSocket %d\n", cb->brcmSocket->drvSocket);
			eapd_del_brcm(nwksp, cb->brcmSocket);
		}

		tmp_cb = cb;
		cb = cb->next;
		free(tmp_cb);
	}

	/* close  appSocket */
	if (wbd->appSocket >= 0) {
		EAPD_INFO("WBD: close wbd appSocket %d\n", wbd->appSocket);
		close(wbd->appSocket);
		wbd->appSocket = -1;
	}

	return 0;
}

/* bss:
 *	1 : send BSS transition response event via special socket
 *	0 : send all other events via the old socket.
 */
int
wbd_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int pLen, char *from)
{
	eapd_wbd_t *wbd;

	if (nwksp == NULL) {
		EAPD_ERROR("WBD: Wrong argument...\n");
		return -1;
	}

	wbd = &nwksp->wbd;
	if (wbd->appSocket >= 0) {
		/* send to wbd */
		int sentBytes = 0;
		struct sockaddr_in to;

		to.sin_addr.s_addr = inet_addr(EAPD_WKSP_UDP_ADDR);
		to.sin_family = AF_INET;
		to.sin_port = htons(EAPD_WKSP_WBD_UDP_SPORT);

		sentBytes = sendto(wbd->appSocket, pData, pLen, 0,
			(struct sockaddr *)&to, sizeof(struct sockaddr_in));

		if (sentBytes != pLen) {
			EAPD_ERROR("WBD: UDP send failed; sentBytes = %d\n", sentBytes);
		}
	}
	else {
		EAPD_ERROR("WBD: wbd appSocket not created\n");
	}
	return 0;
}

#if EAPD_WKSP_AUTO_CONFIG
int
wbd_app_enabled(char *name)
{
	char value[128], comb[32],  prefix[8];
	char os_name[IFNAMSIZ];
	int unit;

	memset(os_name, 0, sizeof(os_name));

	if (nvifname_to_osifname(name, os_name, sizeof(os_name)))
		return 0;
	if (wl_probe(os_name) ||
		wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
		return 0;
	if (osifname_to_nvifname(name, prefix, sizeof(prefix)))
		return 0;

	strcat(prefix, "_");
	/* ignore if disabled */
	eapd_safe_get_conf(value, sizeof(value), strcat_r(prefix, "radio", comb));
	if (atoi(value) == 0) {
		EAPD_INFO("WBD: ignored interface %s. radio disabled\n", os_name);
		return 0;
	}

	/* ignore if BSS is disabled */
	eapd_safe_get_conf(value, sizeof(value), strcat_r(prefix, "bss_enabled", comb));
	if (atoi(value) == 0) {
		EAPD_INFO("WBD: ignored interface %s, %s is disabled \n", os_name, comb);
		return 0;
	}

	/* if come to here return enabled */
	return 1;
}
#endif /* EAPD_WKSP_AUTO_CONFIG */

int
wbd_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from)
{
	int type;
	eapd_wbd_t *wbd;
	eapd_cb_t *cb;
	bcm_event_t *dpkt = (bcm_event_t *) pData;
	wl_event_msg_t *event = &(dpkt->event);

	type = ntohl(event->event_type);

	wbd = &nwksp->wbd;
	cb = wbd->cb;

	while (cb) {
		if (isset(wbd->bitvec, type) &&
			!strcmp(cb->ifname, from)) {

			/* prepend ifname,  we reserved IFNAMSIZ length already */
			pData -= IFNAMSIZ;
			Len += IFNAMSIZ;
			memcpy(pData, event->ifname, IFNAMSIZ);

			/* send to wbd use cb->ifname */
			wbd_app_sendup(nwksp, pData, Len, cb->ifname);
			break;
		}
		cb = cb->next;
	}

	return 0;
}
