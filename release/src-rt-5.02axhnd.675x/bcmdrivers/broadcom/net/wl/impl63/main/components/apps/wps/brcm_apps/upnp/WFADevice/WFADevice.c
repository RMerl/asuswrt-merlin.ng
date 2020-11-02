/*
 * Broadcom WPS module (for libupnp), WFADevice.c
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
 * $Id: WFADevice.c 738257 2017-12-27 22:59:37Z $
 */
#include <upnp.h>
#include <WFADevice.h>
#include <wps_upnp.h>

extern UPNP_SCBRCHAIN * get_subscriber_chain(UPNP_CONTEXT *context, UPNP_SERVICE *service);

/* Perform the SetSelectedRegistrar action */
int
wfa_SetSelectedRegistrar(UPNP_CONTEXT *context,	UPNP_TLV *NewMessage)
{
	int rc;
	UPNP_WPS_CMD *cmd;

	if (NewMessage->len < 0)
		return SOAP_INVALID_ARGS;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1 + NewMessage->len, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_SSR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = NewMessage->len;

	/* Data */
	memcpy(cmd->data, NewMessage->val.str, NewMessage->len);

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE + NewMessage->len);
	free(cmd);
	return 0;
}

/* Perform the PutMessage action */
int
wfa_PutMessage(UPNP_CONTEXT *context, UPNP_TLV *NewInMessage, UPNP_TLV *NewOutMessage)
{
	int rc;
	char *info = "";
	int info_len = 0;
	UPNP_WPS_CMD *cmd;

	if (NewInMessage->len < 0)
		return SOAP_INVALID_ARGS;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1 + NewInMessage->len, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_PMR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = NewInMessage->len;

	/* Data */
	memcpy(cmd->data, NewInMessage->val.str, NewInMessage->len);

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE + NewInMessage->len);
	free(cmd);

	/* Always check the out messge len */
	info_len = wps_libupnp_GetOutMsgLen(context->focus_ifp->ifname);
	if (info_len > 0 &&
	    (info = wps_libupnp_GetOutMsg(context->focus_ifp->ifname)) == NULL) {
		info = "";
		info_len = 0;
	}

	upnp_tlv_set_bin(NewOutMessage, (uintptr_t)info, info_len);
	return 0;
}

/* Perform the GetDeviceInfo action */
int
wfa_GetDeviceInfo(UPNP_CONTEXT *context, UPNP_TLV *NewDeviceInfo)
{
	int rc;
	char *info = "";
	int info_len = 0;
	UPNP_WPS_CMD *cmd;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_GDIR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = 0;

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE);
	free(cmd);

	/* Always check the out messge len */
	info_len = wps_libupnp_GetOutMsgLen(context->focus_ifp->ifname);
	if (info_len > 0 &&
	    (info = wps_libupnp_GetOutMsg(context->focus_ifp->ifname)) == NULL) {
		info = "";
		info_len = 0;
	}

	upnp_tlv_set_bin(NewDeviceInfo, (uintptr_t)info, info_len);
	return 0;
}

/* Perform the PutWLANResponse action */
int
wfa_PutWLANResponse(UPNP_CONTEXT *context, UPNP_TLV *NewMessage)
{
	int rc;
	UPNP_WPS_CMD *cmd;

	if (NewMessage->len < 0)
		return SOAP_INVALID_ARGS;

	cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1 + NewMessage->len, 1);
	if (!cmd)
		return SOAP_OUT_OF_MEMORY;

	/* Cmd */
	cmd->type = UPNP_WPS_TYPE_PWR;
	strncpy((char *)cmd->dst_addr, inet_ntoa(context->dst_addr.sin_addr), sizeof(cmd->dst_addr));
	cmd->length = NewMessage->len;

	/* Data */
	memcpy(cmd->data, NewMessage->val.str, NewMessage->len);

	/* Direct call wps_libupnp_ProcessMsg */
	rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
		UPNP_WPS_CMD_SIZE + NewMessage->len);
	free(cmd);

	return 0;
}

/* Close wfa */
static void
wfa_free(UPNP_CONTEXT *context)
{
	return;
}

/* Open wfa */
int
wfa_init(UPNP_CONTEXT *context)
{
	return 0;
}

/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: WFADevice_open() */
int
WFADevice_open(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	int retries;

	/* Setup default gena connect retries for WFA */
	retries = WFADevice.gena_connect_retries;
	if (retries < 1 || retries > 5)
		retries = 3;

	WFADevice.gena_connect_retries = retries;

	if (wfa_init(context) != 0)
		return -1;

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_close() */
int
WFADevice_close(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	wfa_free(context);
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_timeout() */
int
WFADevice_timeout(UPNP_CONTEXT *context, time_t now)
{
	/* << USER CODE START >> */
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_notify() */
int
WFADevice_notify(UPNP_CONTEXT *context, UPNP_SERVICE *service,
	UPNP_SUBSCRIBER *subscriber, int notify)
{
	/* << USER CODE START >> */
	int rc;
	UPNP_WPS_CMD *cmd;

	if (notify == DEVICE_NOTIFY_TIMEOUT ||
	    notify == DEVICE_NOTIFY_UNSUBSCRIBE) {
		cmd = (UPNP_WPS_CMD *)calloc(UPNP_WPS_CMD_SIZE + 1, 1);
		if (!cmd)
			return SOAP_OUT_OF_MEMORY;

		/* Cmd */
		cmd->type = UPNP_WPS_TYPE_DISCONNECT;
		strcpy((char *)cmd->dst_addr, inet_ntoa(subscriber->ipaddr));
		cmd->length = 0;

		/* Direct call wps_libupnp_ProcessMsg */
		rc = wps_libupnp_ProcessMsg(context->focus_ifp->ifname, (char *)cmd,
			UPNP_WPS_CMD_SIZE);
		free(cmd);
	}

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: WFADevice_scbrchk() */
int
WFADevice_scbrchk(UPNP_CONTEXT *context, UPNP_SERVICE *service,
	UPNP_SUBSCRIBER *subscriber, struct in_addr ipaddr, unsigned short port, char *uri)
{
	/* << USER CODE START >> */

	/*
	 * PR97482, once a STA enrollment with Win 7 ER successfully I pull out the Win 7
	 * Ethernet and plug in again intentionally the problem happen in next enrollment.
	 * The reason is Win 7 will do UPnP subscribption again when it detect the Ethernet
	 * link down and up.  On our AP we deal with the two subscriptions as different
	 * subscriptions.
	 * In fact, we think one ER should not have multiple Registrar instances, so in WPS we
	 * would like to change our AP UPnP to handle same IP and PORT subscription as same
	 * subscriber.
	 */
	if (subscriber->ipaddr.s_addr == ipaddr.s_addr && subscriber->port == port)
		return 1;

	return 0;
}
/* >> AUTO GENERATED FUNCTION */
