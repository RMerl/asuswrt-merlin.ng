/*
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
 * Copyright (C) 2012, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 *
 * $Id$
 */
#ifdef MULTIAP

#include <stdio.h>
#include <tutrace.h>
#include <time.h>
#include <wps_wl.h>
#include <wps_1905.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <wps_ap.h>
#include <wps_enr.h>
#include <security_ipc.h>
#include <wps.h>
#include <wps_pb.h>
#include <wps_led.h>
#include <wps_upnp.h>
#include <wps_ie.h>
#include <wpserror.h>
#include <wps_ui.h>
#include <ap_ssr.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ap_eap_sm.h>
#include <unistd.h>
#include <ctype.h>
#include <bcmnvram.h>
#include <portability.h>
#include <wpsheaders.h>
#include <wpscommon.h>
#include <sminfo.h>
#include <wpserror.h>
#include <reg_protomsg.h>
#include <reg_proto.h>
#include <statemachine.h>
#include <wps_wl.h>
#include <wps_apapi.h>
#include <wpsapi.h>
#include <shutils.h>

static wps_hndl_t wps_1905_hndl;
static unsigned short wps_1905_listener_port = 0;

extern int  wps_osl_1905_handle_init();
extern void wps_osl_1905_handle_deinit(int handle);
extern int  wps_osl_1905_send_response(wps_hndl_t wpsHandle, unsigned short port, char *msg,
	uint32 len);

#define MAX_WLAN_ADAPTER 4

/* Get wl interface no. */
static int
wlgetintfNo(void)
{
	struct ifreq ifr;
	int s = 0;
	int num = 0, i = 0;
	char ifcName[12] = {0};

	for (i = 0; i < MAX_WLAN_ADAPTER; i++) {
		sprintf(ifcName, "wl%d", i);
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			return -1;

		strcpy(ifr.ifr_name, ifcName);
		if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		} else {
			num++;
		}
		close(s);
	}

	return num;
}

/* Init 1905_hndl. */
int
wps_1905_init()
{
	memset(&wps_1905_hndl, 0, sizeof(wps_1905_hndl));

	wps_1905_hndl.type = WPS_RECEIVE_PKT_1905;
	wps_1905_hndl.handle = wps_osl_1905_handle_init();
	if (wps_1905_hndl.handle == -1)
		return -1;

	wps_hndl_add(&wps_1905_hndl);

	return 0;
}

/* Denit 1905_hndl. */
void
wps_1905_deinit()
{
	wps_hndl_del(&wps_1905_hndl);
	wps_osl_1905_handle_deinit(wps_1905_hndl.handle);
	return;
}

/* Check whether wps is enabled or not. */
static int
wps_1905_wscenabled(char const *ifname)
{
	char tmp[20];

	sprintf(tmp, "%s_wps_mode", ifname);
	if (strcmp(nvram_safe_get(tmp), "enabled") == 0) {
		return WPS_1905_WSCAP_ENABLED;
	} else {
		return WPS_1905_WSCAP_DISABLED;
	}
}

/* Check  wps is status. */
static int
wps_1905_wscstatus()
{
	wps_app_t *wps_app = get_wps_app();

	if (!wps_app->wksp)
	{
		/* no session, check if WSC is configured */
		if (!strcmp(nvram_safe_get("lan_wps_oob"), "enabled")) {
			return WPS_1905_WSCAP_UNCONFIGURED;
		} else {
			return WPS_1905_WSCAP_CONFIGURED;
		}
	} else {
		return WPS_1905_WSCAP_SESSIONONGOING;
	}
}

void
wps_1905_start_PBC(char *ifname)
{
	wps_ui_set_env("map_pbc_method", "1");
}

/* Get 1905 message info. */
int
wps_1905_GetMessageInfo(int cmd, char *pData, short len, WPS_1905_M_MESSAGE_INFO *pInfo)
{
	int tlvLen;
	int found = 0;

	while (len > 0) {
		if (WPS_ID_MSG_TYPE == WpsNtohs((uint8 *)pData)) {
			pData += 2;
			tlvLen = WpsNtohs((uint8 *)pData);
			pData += 2;
			if (tlvLen != 1)
			{
				break;
			}
			pInfo->mtype = *pData;
			pData += tlvLen;
			found++;
		} else if (WPS_ID_RF_BAND == WpsNtohs((uint8 *)pData)) {
			pData += 2;
			tlvLen = WpsNtohs((uint8 *)pData);
			pData += 2;
			if (tlvLen != 1)
			{
				break;
			}
			pInfo->rfband = *pData;
			pData += tlvLen;
			found++;
		} else {
			pData += 2;
			tlvLen = WpsNtohs((uint8 *)pData);
			pData += 2 + tlvLen;
		}
		len = len - (4 + tlvLen);

		if (found == 2) {
			break;
		}
	}

	if (found != 2) {
		return WPS_1905_UNKNOWNWSCMESSAGE;
	}

	return 0;

}

/* Send 1905 ap response. */
int
wps_1905_send_response(char *msg, unsigned int len, int cmd, int status)
{
	uint32 buflen;
	WPS_1905_MESSAGE *pMsg;
	int ret;

	if (wps_1905_listener_port == 0) {
		return WPS_ERROR_1905_SENDFAILURE;
	}

	if ((msg != NULL) && (len == 0)) {
		len = strlen(msg);
	}

	buflen = sizeof(WPS_1905_MESSAGE) + len + 1;
	pMsg = (WPS_1905_MESSAGE *)malloc(buflen);
	memset(pMsg, '\0', buflen);
	pMsg->cmd = cmd;
	pMsg->len = len;
	pMsg->status = status;
	if (msg != NULL) {
		memcpy(pMsg+1, msg, len);
	}

	ret = wps_osl_1905_send_response(wps_1905_hndl, wps_1905_listener_port, (char *)pMsg,
		buflen);
	TUTRACE((TUTRACE_ERR, " SEND Message to 1905  len = %d\r\n", len));
	free(pMsg);

	if (ret != WPS_SUCCESS) {
		return WPS_ERROR_1905_SENDFAILURE;
	}
	return ret;
}

/* for i5ctl command response ,integration will use this as an example */
int
wps_1905_process_msg(char *buf, int buflen)
{
	int ret  = WPS_CONT;
	WPS_1905_MESSAGE *pMsg = (WPS_1905_MESSAGE *)buf;
	switch (pMsg->cmd) {

		case WPS_1905_CTL_CLIENT_REGISTER:
		{
			char number[10];
			wps_1905_listener_port = *(unsigned short *)WPS1905MSG_DATAPTR(pMsg);
			wps_1905_send_response("1905", 4, pMsg->cmd, 1);
			sprintf(number, "%d", wps_1905_listener_port);
			nvram_set("1905_com_socket", number);
		}
		break;
		case WPS_1905_CTL_WLINSTANCE:
			wps_1905_send_response("1905", 4, pMsg->cmd, wlgetintfNo());
		break;
		case WPS_1905_CTL_WSCENABLED:
			wps_1905_send_response("1905", 4, pMsg->cmd,
				wps_1905_wscenabled(pMsg->ifName));
		break;
		case WPS_1905_CTL_WSCSTATUS:
			wps_1905_send_response("1905", 4, pMsg->cmd, wps_1905_wscstatus());
		break;
		case WPS_1905_CTL_WSCCANCEL:
			wps_close_session();
			wps_1905_send_response("1905", 4, pMsg->cmd, 1);
		break;
		case WPS_1905_CTL_GETMINFO:
		{
			WPS_1905_M_MESSAGE_INFO info;

			strcpy(info.id, "1905");
			ret = wps_1905_GetMessageInfo(pMsg->cmd, WPS1905MSG_DATAPTR(pMsg),
				pMsg->len, &info);
			wps_1905_send_response((char *)&info, sizeof(WPS_1905_M_MESSAGE_INFO),
				pMsg->cmd, ret);
			if (ret == 0) {
				ret = WPS_CONT;
			}
		}
		break;
		case WPS_1905_CTL_PBC:
		{
			(void)wps_1905_start_PBC(pMsg->ifName);
		}
		break;

		default:
		break;
	}
	return ret;
}
#endif /* MULTIAP */
