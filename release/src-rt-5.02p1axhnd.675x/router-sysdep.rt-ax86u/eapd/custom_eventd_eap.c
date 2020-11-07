/*
 * Application-specific portion of EAPD
 * (custom_eventd)
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
 * $Id: custom_eventd_eapd.c $
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
#include <wlutils.h>
#include <eapd.h>
#include <shutils.h>
#include <UdpLib.h>
#include <security_ipc.h>

/* Customer event handler code for customized feature */
/* Receive message from evt module (Located in wldaemon)  */
void evt_app_recv_handler(eapd_wksp_t *nwksp, char *wlifname, eapd_cb_t *from,
	uint8 *pData, int *pLen, struct ether_addr *ap_ea)
{
	/* This is a stub function for further usage */
	return;
}

/* Set Event Mask to allow wlmngr to receive wlan Event from Driver */
void evt_app_set_eventmask(eapd_app_t *app)
{
	memset(app->bitvec, 0, sizeof(app->bitvec));

	setbit(app->bitvec, WLC_E_AUTH_IND);
	setbit(app->bitvec, WLC_E_DEAUTH_IND);
	setbit(app->bitvec, WLC_E_ASSOC_IND);
	setbit(app->bitvec, WLC_E_REASSOC_IND);
	setbit(app->bitvec, WLC_E_DISASSOC_IND);
	/* add desired event map here */
	setbit(app->bitvec, WLC_E_PSK_SUP);
	return;
}

/* Initialize the parameters */
int evt_app_init(eapd_wksp_t *nwksp)
{
	int ret, reuse = 1;
	eapd_evt_t *evt;
	eapd_cb_t *cb;

	struct sockaddr_in sockaddr;

	if (nwksp == NULL)
		return -1;

	evt = &nwksp->evt;
	evt->appSocket = -1;

	cb = evt->cb;
	if (cb == NULL) {
		EAPD_INFO("No any interface is running EVT !\n");
		return 0;
	}

	while (cb) {
		EAPD_INFO("init brcm interface %s \n", cb->ifname);
		cb->brcmSocket = eapd_add_brcm(nwksp, cb->ifname);
		if (!cb->brcmSocket)
			return -1;

		cb = cb->next;
	}

	evt->appSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (evt->appSocket < 0) {
		EAPD_ERROR("UDP Open failed.\n");
		return -1;
	}

	if (setsockopt(evt->appSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) < 0) {
		EAPD_ERROR("UDP setsockopt failed.\n");
		close(evt->appSocket);
		evt->appSocket = -1;
		return -1;
	}

	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* htonl(INADDR_LOOPBACK); */
	sockaddr.sin_port = htons(EAPD_WKSP_EVT_UDP_RPORT);
	ret = bind(evt->appSocket, (struct sockaddr *)&sockaddr, sizeof(sockaddr));

	if (ret < 0) {
		EAPD_ERROR("UDP Bind failed, close evt appSocket %d\n", evt->appSocket);
		close(evt->appSocket);
		evt->appSocket = -1;
		return -1;
	}
	EAPD_INFO("EVT appSocket %d opened\n", evt->appSocket);

	return 0;
}

int evt_app_deinit(eapd_wksp_t *nwksp)
{
	eapd_evt_t *evt;
	eapd_cb_t *cb, *tmp_cb;

	if (nwksp == NULL) {
		EAPD_ERROR("Wrong argument...\n");
		return -1;
	}
	evt = &nwksp->evt;
	cb = evt->cb;
	while (cb) {
		/* brcm drvSocket delete */
		if (cb->brcmSocket) {
			EAPD_INFO("close evt brcmSocket %d\n", cb->brcmSocket->drvSocket);
			eapd_del_brcm(nwksp, cb->brcmSocket);
		}

		tmp_cb = cb;
		cb = cb->next;
		free(tmp_cb);
	}

	/* close appSocket for evt */
	if (evt->appSocket >= 0) {
		EAPD_INFO("close  evt appSocket %d\n", evt->appSocket);
		close(evt->appSocket);
		evt->appSocket = -1;
	}

	return 0;
}

/* Send WLAN Event to Wlmngr */
static int evt_app_sendup(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from)
{
	eapd_evt_t *evt;
	bcm_event_t *dpkt = (bcm_event_t *) pData;
	wl_event_msg_t *event = &(dpkt->event);

	if (nwksp == NULL) {
		EAPD_ERROR("Wrong argument...\n");
		return -1;
	}

	evt = &nwksp->evt;
	if (evt->appSocket >= 0) {
		/* send to evt */
		int sentBytes = 0;
		struct sockaddr_in to;

		to.sin_addr.s_addr = inet_addr(EAPD_WKSP_UDP_ADDR);
		to.sin_family = AF_INET;
		to.sin_port = htons(EAPD_WKSP_EVT_UDP_SPORT);

		EAPD_INFO("%s@%d evt->appSocket=%d\n", __FUNCTION__, __LINE__, evt->appSocket);

		/* prepend ifname - wlmngr expect it, we reserved IFNAMSIZ length already */
		pData -= IFNAMSIZ;
		Len += IFNAMSIZ;
		memcpy(pData, event->ifname, IFNAMSIZ);

		/* Send Event Data to wlmngr */
		sentBytes = sendto(evt->appSocket, pData, Len, 0,
				(struct sockaddr *)&to, sizeof(struct sockaddr_in));

		if (sentBytes != Len) {
			EAPD_ERROR("UDP send to evt on %s failed; sentBytes = %d\n",
				from, sentBytes);
		}
		else {
			EAPD_INFO("send %d bytes to evt on %s: port=%d\n",
				sentBytes, from, to.sin_port);
		}
	}
	else {
		EAPD_ERROR("evt appSocket not created\n");
	}

	return 0;
}

/* First handling WLAN Event */
int evt_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int Len, char *from)
{
	int type;
	eapd_evt_t *evt;
	eapd_cb_t *cb;
	bcm_event_t *dpkt = (bcm_event_t *) pData;
	wl_event_msg_t *event;

	event = &(dpkt->event);
	type = ntohl(event->event_type);

	evt = &nwksp->evt;
	cb = evt->cb;
	while (cb) {
		if (isset(evt->bitvec, type) &&
			!strcmp(cb->ifname, from)) {

			EAPD_INFO("Send from intf: %s\n", cb->ifname );
			evt_app_sendup(nwksp, pData, Len, cb->ifname);
			break;
		}
		cb = cb->next;
	}
	return 0;
}
