/*
 * WLAN functions.
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "802.11.h"
#include "trace.h"
#include "dsp.h"
#include "wlu_api.h"
#include "wlan.h"

struct wlanStruct {
	/* ether addr */
	struct ether_addr etherAddr;

	/* escan parameters */
	uint16 syncId;
	int numNonDfsChannels;
	uint16 nonDfsChannels[WL_NUMCHANNELS];
};

static struct {
	void (*fn)(void *context, uint32 eventType,
		wl_event_msg_t *wlEvent, uint8 *data, uint32 length);
	void *context;
} gEventCallback;

typedef struct wlanReq wlanReqT;

/* request handler */
typedef void (*requestHandlerT)(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspData);

typedef struct {
	wlanT *wlan;
} wlanCreateRspT;

typedef struct {
	int enable;
} wlanEnableT;

typedef struct {
	int event;
} wlanEventMsgReqT;

typedef struct {
	uint32 pktflag;
	int len;
	uchar data[VNDR_IE_MAX_LEN];
} wlanVendorIeReqT;

typedef struct {
	uint8 id;
	uint8 len;
	uchar data[VNDR_IE_MAX_LEN];
} wlanIeReqT;

typedef struct {
	int isActive;
	int numProbes;			/* num probes per channel */
	int activeDwellTime;	/* dwell time per channel for active scanning */
	int passiveDwellTime;	/* dwell time per channel for passive scanning */
} wlanStartEscanReqT;

typedef struct {
	uint32 packetId;
	uint32 channel;
	int32 dwellTime;
	struct ether_addr bssid;
	struct ether_addr da;
	uint16 len;
	uint8 data[ACTION_FRAME_SIZE];
} wlanActionFrameReqT;

typedef struct {
	int *isAssociated;
	int biBufferSize;
	wl_bss_info_t *biBuffer;
} wlanAssociationStatusRspT;

typedef struct {
	struct ether_addr ea;
} wlanTdlsReqT;

typedef struct {
	int mask;
} wlanWnmT;

typedef struct {
	uint8 reqmode;
	uint16 disassocTimer;
	char url[256 + 1];	/* null terminated */
} wlanWnmBssTransReqEssDisassocImminentT;

typedef struct {
	int mode;
} wlanPmfT;

typedef struct {
	int mode;
	int count;
	struct ether_addr addr[1];	/* variable length */
} wlanMacT;

typedef struct {
	uint16 len;
	uint8 data[ACTION_FRAME_SIZE];
} wlanSendFrameReqT;

typedef struct {
	bool isStatic;
	uint16 staCount;
	uint8 utilization;
	uint16 aac;
} wlanBssLoadStaticReqT;

struct wlanReq {
	requestHandlerT handler;
	union {
		wlanEventMsgReqT eventMsg;		/* enable/disable */
		wlanVendorIeReqT vendorIe;		/* add/delete vendor IE */
		wlanIeReqT ie;					/* add/delete IE */
		wlanStartEscanReqT startEscan;
		wlanActionFrameReqT actionFrame;
		wlanTdlsReqT tdls;
		wlanEnableT dropGratuitousArp;
		wlanWnmT wnm;
		wlanWnmBssTransReqEssDisassocImminentT wnmBssTransReqEssDisassocImminent;
		wlanPmfT pmf;
		wlanMacT mac;
		wlanEnableT interworking;
		wlanEnableT osen;
		wlanSendFrameReqT sendFrame;
		wlanEnableT bssLoad;
		wlanBssLoadStaticReqT bssLoadStatic;
	};
};

/* dispatch handler */
static void wlanProcessWlanEvent(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length);

/* ----------------------------------------------------------- */

/* initialize wlan */
int wlanInitialize(void)
{
	return dspSubscribe(dsp(), 0, wlanProcessWlanEvent);
}

/* deinitialize wlan */
int wlanDeinitialize(void)
{
	return dspUnsubscribe(dsp(), wlanProcessWlanEvent);
}

/* ----------------------------------------------------------- */

static void wlanCreateHandler(wlanT *wlanNull,
	int reqLength, wlanReqT *req, wlanCreateRspT *rsp)
{
	wlanT *wlan;
	uint16 channels[WL_NUMCHANNELS];
	int numChannels = 0;
	int i;

	(void)wlanNull;
	if (reqLength != sizeof(wlanReqT) || req == 0 || rsp == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanCreateHandler\n");

	rsp->wlan = 0;

	/* check driver loaded */
	if (wl() == 0) {
		TRACE(TRACE_ERROR, "wl.ko driver not loaded\n");
		return;
	}

	wlan = malloc(sizeof(*wlan));
	if (wlan == 0)
		return;
	memset(wlan, 0, sizeof(*wlan));

	/* get the ethernet address */
	if (wl_cur_etheraddr(wl(), DEFAULT_BSSCFG_INDEX, &wlan->etherAddr) != 0) {
		TRACE(TRACE_ERROR, "wl_cur_etheraddr failed\n");
	}

	/* get the supported channels */
	if (wl_get_channels(wl(), WL_NUMCHANNELS, &numChannels, channels) != 0) {
		TRACE(TRACE_ERROR, "wl_get_channels failed\n");
	}

	/* remove DFS channels */
	wlan->numNonDfsChannels = 0;
	for (i = 0; i < numChannels; i++) {
		if (!wl_is_dfs(wl(), channels[i]))
			wlan->nonDfsChannels[wlan->numNonDfsChannels++] = channels[i];
	}

	/* enable event */
	if (wl_enable_event_msg(wl(), WLC_E_ESCAN_RESULT) < 0) {
		TRACE(TRACE_ERROR, "failed to enable escan event\n");
	}
	if (wl_enable_event_msg(wl(), WLC_E_DEAUTH) < 0) {
		TRACE(TRACE_ERROR, "failed to enable deauth event\n");
	}
	if (wl_enable_event_msg(wl(), WLC_E_DISASSOC) < 0) {
		TRACE(TRACE_ERROR, "failed to enable disassoc event\n");
	}
	if (wl_enable_event_msg(wl(), WLC_E_DEAUTH_IND) < 0) {
		TRACE(TRACE_ERROR, "failed to enable deauth event\n");
	}
	if (wl_enable_event_msg(wl(), WLC_E_DISASSOC_IND) < 0) {
		TRACE(TRACE_ERROR, "failed to enable disassoc event\n");
	}
	if (wl_enable_event_msg(wl(), WLC_E_LINK) < 0) {
		TRACE(TRACE_ERROR, "failed to enable link event\n");
	}
	if (wl_enable_event_msg(wl(), WLC_E_SET_SSID) < 0) {
		TRACE(TRACE_ERROR, "failed to enable set ssid event\n");
	}

	/* return created instance */
	rsp->wlan = wlan;
}

/* create wlan instance */
wlanT *wlanCreate(void)
{
	wlanReqT req;
	wlanCreateRspT rsp;

	TRACE(TRACE_VERBOSE, "wlanCreate\n");

	memset(&req, 0, sizeof(req));
	req.handler = (requestHandlerT)wlanCreateHandler;
	if (!dspRequestSynch(dsp(), 0, sizeof(req), (uint8 *)&req, (uint8 *)&rsp))
	{
		return 0;
	}

	return rsp.wlan;
}

/* ----------------------------------------------------------- */

static void wlanDestroyHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanDestroyHandler\n");
	free(wlan);
}

/* destroy wlan instance */
int wlanDestroy(wlanT *wlan)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanDestroy\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanDestroyHandler;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

/* get WLAN interface name */
char *wlanIfName(wlanT *wlan)
{
	(void)wlan;
	return wl_ifname(wl());
}

/* ----------------------------------------------------------- */

/* get WLAN ethernet address */
int wlanEtherAddr(wlanT *wlan, struct ether_addr *addr)
{
	memcpy(addr, &wlan->etherAddr, sizeof(*addr));
	return TRUE;
}

/* ----------------------------------------------------------- */

static void wlanEnableEventMsgHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanEnableEventMsgHandler\n");

	if (wl_enable_event_msg(wl(), req->eventMsg.event) < 0) {
		TRACE(TRACE_ERROR, "failed to enable event msg %d\n",
			req->eventMsg.event);
	}
}

/* enable event msg */
int wlanEnableEventMsg(wlanT *wlan, int event)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanEnableEventMsg\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanEnableEventMsgHandler;
	req.eventMsg.event = event;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanDisableEventMsgHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanDisableEventMsgHandler\n");

	if (wl_disable_event_msg(wl(), req->eventMsg.event) < 0) {
		TRACE(TRACE_ERROR, "failed to disable event msg %d\n",
			req->eventMsg.event);
	}
}

/* disable event msg */
int wlanDisableEventMsg(wlanT *wlan, int event)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanDisableEventMsg\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanDisableEventMsgHandler;
	req.eventMsg.event = event;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanAddVendorIeHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanAddVendorIeHandler\n");

	if (wl_add_vndr_ie(wl(), DEFAULT_BSSCFG_INDEX,
		req->vendorIe.pktflag, req->vendorIe.len, req->vendorIe.data) < 0) {
		TRACE(TRACE_ERROR, "failed to add vendor IE\n");
	}
}

/* add vendor IEs */
int wlanAddVendorIe(wlanT *wlan, uint32 pktflag, int len, uchar *data)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanAddVendorIe\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanAddVendorIeHandler;
	req.vendorIe.pktflag = pktflag;
	req.vendorIe.len = len;
	memcpy(req.vendorIe.data, data, req.vendorIe.len);
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanDeleteVendorIeHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanDeleteVendorIeHandler\n");

	/* may return fail but just means IE has been deleted already */
	wl_del_vndr_ie(wl(), DEFAULT_BSSCFG_INDEX,
		req->vendorIe.pktflag, req->vendorIe.len, req->vendorIe.data);
}

/* delete vendor IEs */
int wlanDeleteVendorIe(wlanT *wlan, uint32 pktflag, int len, uchar *data)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanDeleteVendorIe\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanDeleteVendorIeHandler;
	req.vendorIe.pktflag = pktflag;
	req.vendorIe.len = len;
	memcpy(req.vendorIe.data, data, req.vendorIe.len);
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanDeleteAllVendorIeHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanDeleteAllVendorIeHandler\n");

	wl_del_all_vndr_ie(wl(), DEFAULT_BSSCFG_INDEX);
}

/* delete all vendor IEs */
int wlanDeleteAllVendorIe(wlanT *wlan)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanDeleteAllVendorIe\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanDeleteAllVendorIeHandler;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanIeHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanIeHandler\n");

	if (wl_ie(wl(), req->ie.id, req->ie.len, req->ie.data) < 0) {
		TRACE(TRACE_ERROR, "wl_ie failed %d\n", req->ie.id);
	}
}

/* add/del IE */
int wlanIe(wlanT *wlan, uint8 id, uint8 len, uchar *data)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanIe\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanIeHandler;
	req.ie.id = id;
	req.ie.len = len;
	memcpy(req.ie.data, data, req.ie.len);
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanStartEscanHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
#define ESCAN_RETRY		10
	int numChannels = 0;
	uint16 *channels = 0;
	int i;

	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanStartEscanHandler isActive=%d numProbes=%d "
		"activeDwellTime=%d passiveDwellTime=%d\n",
		req->startEscan.isActive, req->startEscan.numProbes,
		req->startEscan.activeDwellTime, req->startEscan.passiveDwellTime);

	/* use scan abort to abort all escan, actframe, etc. */
	if (wl_scan_abort(wl()) != 0) {
		TRACE(TRACE_ERROR, "wl_scan_abort failed\n");
	}

	/* retry the scan as it may be busy */
	for (i = 0; i < ESCAN_RETRY; i++) {
		if (wl_escan(wl(), ++wlan->syncId, req->startEscan.isActive,
			req->startEscan.numProbes, req->startEscan.activeDwellTime,
			req->startEscan.passiveDwellTime,
			numChannels, channels) == 0) {
				break;
			}
			else {
				usleep(500 * 1000);
				printf("escan retry: %d\n", i);
			}
	}
	if (i >= ESCAN_RETRY) {
		TRACE(TRACE_ERROR, "wl_escan failed\n");
	}
}

/* start escan */
int wlanStartEscan(wlanT *wlan, int isActive, int numProbes,
	int activeDwellTime, int passiveDwellTime)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanStartEscan isActive=%d\n", isActive);

	memset(&req, 0, sizeof(req));
	req.handler = wlanStartEscanHandler;
	req.startEscan.isActive = isActive;
	req.startEscan.numProbes = numProbes;
	req.startEscan.activeDwellTime = activeDwellTime;
	req.startEscan.passiveDwellTime = passiveDwellTime;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanStopScanHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanStopScanHandler\n");
	/* use scan abort to abort all escan, actframe, etc. */
	if (wl_scan_abort(wl()) != 0) {
		TRACE(TRACE_ERROR, "wl_scan_abort failed\n");
	}
}

/* stop scan engine (scan, escan, action frame, etc.) */
int wlanStopScan(wlanT *wlan)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanStopScan\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanStopScanHandler;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanDisassociateHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanDisassociateHandler\n");

	if (wl_disassoc(wl()) < 0) {
		TRACE(TRACE_ERROR, "wl_disassoc failed\n");
	}
}

/* disassociate */
int wlanDisassociate(wlanT *wlan)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanDisassociate\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanDisassociateHandler;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanPmfDisassociateHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanPmfDisassociateHandler\n");

	if (wl_pmf_disassoc(wl()) < 0) {
		TRACE(TRACE_ERROR, "wl_pmf_disassoc failed\n");
	}
}

/* PMF disassociate */
int wlanPmfDisassociate(wlanT *wlan)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanPmfDisassociate\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanPmfDisassociateHandler;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanBssTransitionQueryHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanBssTransitionQueryHandler\n");

	if (wl_wnm_bsstrans_query(wl()) < 0) {
		TRACE(TRACE_ERROR, "wl_wnm_bsstrans_query failed\n");
	}
}

/* send BSS transition query */
int wlanBssTransitionQuery(wlanT *wlan)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanBssTransitionQuery\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanBssTransitionQueryHandler;
	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanBssTransReqEssDisassocImminentHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanBssTransReqEssDisassocImminentHandler\n");

	if (wl_wnm_url(wl(), strlen(req->wnmBssTransReqEssDisassocImminent.url),
		(uchar *)req->wnmBssTransReqEssDisassocImminent.url) < 0) {
		TRACE(TRACE_ERROR, "wl_wnm_url failed\n");
		return;
	}

	if (wl_wnm_bsstrans_req(wl(),
		DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL |
		DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT |
		DOT11_BSSTRANS_REQMODE_ESS_DISASSOC_IMNT,
		req->wnmBssTransReqEssDisassocImminent.disassocTimer,
		0, TRUE) < 0) {
		TRACE(TRACE_ERROR, "wl_wnm_bsstrans_req failed\n");
	}
}

/* send BSS transition request - ESS disassociation imminent */
int wlanBssTransReqEssDisassocImminent(wlanT *wlan,
	uint16 disassocTimer, char *url)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanBssTransReqEssDisassocImminent\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanBssTransReqEssDisassocImminentHandler;
	req.wnmBssTransReqEssDisassocImminent.disassocTimer = disassocTimer;
	strncpy(req.wnmBssTransReqEssDisassocImminent.url, url,
		sizeof(req.wnmBssTransReqEssDisassocImminent.url) - 1);
	req.wnmBssTransReqEssDisassocImminent.url[
		sizeof(req.wnmBssTransReqEssDisassocImminent.url) - 1] = 0;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanActionFrameHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanActionFrameHandler\n");

	if (wl_actframe(wl(), DEFAULT_BSSCFG_INDEX,
		req->actionFrame.packetId, req->actionFrame.channel,
		req->actionFrame.dwellTime, &req->actionFrame.bssid, &req->actionFrame.da,
		req->actionFrame.len, req->actionFrame.data) < 0) {
		TRACE(TRACE_ERROR, "wl_actframe failed\n");
	}
}

/* send action frame */
int wlanActionFrame(wlanT *wlan, uint32 packetId, uint32 channel, int32 dwellTime,
	struct ether_addr *bssid, struct ether_addr *da, uint16 len, uint8 *data)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanActionFrame\n");

	if (len > ACTION_FRAME_SIZE)
		return 0;

	memset(&req, 0, sizeof(req));
	req.handler = wlanActionFrameHandler;
	req.actionFrame.packetId = packetId;
	req.actionFrame.channel = channel;
	req.actionFrame.dwellTime = dwellTime;
	memcpy(&req.actionFrame.bssid, bssid, sizeof(req.actionFrame.bssid));
	memcpy(&req.actionFrame.da, da, sizeof(req.actionFrame.da));
	req.actionFrame.len = len;
	memcpy(req.actionFrame.data, data, req.actionFrame.len);

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanAssociationStatusHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, wlanAssociationStatusRspT *rsp)
{
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0 || rsp == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanAssociationStatusHandler\n");

	/* get the ethernet address */
	if (wl_status(wl(), rsp->isAssociated, rsp->biBufferSize, rsp->biBuffer) != 0) {
		TRACE(TRACE_ERROR, "wl_status failed\n");
	}
}

/* wlan association status */
int wlanAssociationStatus(wlanT *wlan, int *isAssociated,
	int biBufferSize, wl_bss_info_t *biBuffer)
{
	wlanReqT req;
	wlanAssociationStatusRspT rsp;

	TRACE(TRACE_VERBOSE, "wlanAssociationStatus\n");

	memset(&req, 0, sizeof(req));
	req.handler = (requestHandlerT)wlanAssociationStatusHandler;
	rsp.isAssociated = isAssociated;
	rsp.biBufferSize = biBufferSize;
	rsp.biBuffer = biBuffer;

	return dspRequestSynch(dsp(), wlan, sizeof(req), (uint8 *)&req,
		(uint8 *)&rsp);
}

/* ----------------------------------------------------------- */

static void wlanTdlsDiscoveryRequestHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanTdlsDiscoveryRequestHandler\n");

	/* enable TDLS */
	if (wl_tdls_enable(wl(), 1) < 0) {
		TRACE(TRACE_ERROR, "wl_tdls_enable failed\n");
	}

	/* discovery request */
	if (wl_tdls_endpoint(wl(), "disc", &req->tdls.ea) < 0) {
		TRACE(TRACE_ERROR, "wl_tdls_enable failed\n");
	}

	/* disable TDLS */
	if (wl_tdls_enable(wl(), 0) < 0) {
		TRACE(TRACE_ERROR, "wl_tdls_enable failed\n");
	}
}

/* send TDLS discovery request */
int wlanTdlsDiscoveryRequest(wlanT *wlan, struct ether_addr *ea)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanTdlsDiscoveryRequest\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanTdlsDiscoveryRequestHandler;
	memcpy(&req.tdls.ea, ea, sizeof(req.tdls.ea));

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanTdlsSetupRequestHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanTdlsSetupRequestHandler\n");

	/* enable TDLS */
	if (wl_tdls_enable(wl(), 1) < 0) {
		TRACE(TRACE_ERROR, "wl_tdls_enable failed\n");
	}

	/* setup request */
	if (wl_tdls_endpoint(wl(), "create", &req->tdls.ea) < 0) {
		TRACE(TRACE_ERROR, "wl_tdls_endpoint failed\n");
	}

	/* disable TDLS */
	if (wl_tdls_enable(wl(), 0) < 0) {
		TRACE(TRACE_ERROR, "wl_tdls_enable failed\n");
	}
}

/* send TDLS setup request */
int wlanTdlsSetupRequest(wlanT *wlan, struct ether_addr *ea)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanTdlsSetupRequest\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanTdlsSetupRequestHandler;
	memcpy(&req.tdls.ea, ea, sizeof(req.tdls.ea));

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanDropGratuitousArpHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanDropGratuitousArpHandler\n");

	if (wl_grat_arp(wl(), req->dropGratuitousArp.enable) < 0) {
		TRACE(TRACE_ERROR, "wl_grat_arp failed\n");
	}
}

/* drop gratuitous ARP */
int wlanDropGratuitousArp(wlanT *wlan, int enable)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanDropGratuitousArp\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanDropGratuitousArpHandler;
	req.dropGratuitousArp.enable = enable;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanWnmHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanWnmHandler\n");

	if (wl_wnm(wl(), req->wnm.mask) < 0) {
		TRACE(TRACE_ERROR, "wl_wnm failed\n");
	}
}

/* WNM configuration enable */
int wlanWnm(wlanT *wlan, int mask)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanWnm\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanWnmHandler;
	req.wnm.mask = mask;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanWnmGetHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rsp)
{
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0 || rsp == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanWnmGetHandler\n");

	if (wl_wnm_get(wl(), rsp) < 0) {
		TRACE(TRACE_ERROR, "wl_wnm_get failed\n");
	}
}

/* WNM configuration get */
int wlanWnmGet(wlanT *wlan, int *mask)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanWnmGet\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanWnmGetHandler;

	return dspRequestSynch(dsp(), wlan, sizeof(req), (uint8 *)&req,
		(uint8 *)mask);
}

/* ----------------------------------------------------------- */

static void wlanPmfHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanPmfHandler\n");

	if (wl_pmf(wl(), req->pmf.mode) < 0) {
		TRACE(TRACE_ERROR, "wl_pmf failed\n");
	}
}

/* PMF mode (0=disable, 1=capable, 2=required) */
int wlanPmf(wlanT *wlan, wlanPmfModeT mode)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanPmf\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanPmfHandler;
	req.pmf.mode = mode;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanMacHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength == 0 || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanMacHandler\n");

	if (wl_mac(wl(), req->mac.count, req->mac.addr) < 0) {
		TRACE(TRACE_ERROR, "wl_mac failed\n");
	}
	if (wl_macmode(wl(), req->mac.mode) < 0) {
		TRACE(TRACE_ERROR, "wl_macmode failed\n");
	}
}

/* set MAC mode and list */
int wlanMac(wlanT *wlan, int mode, int count, struct ether_addr *addr)
{
	int ret;
	int len;
	wlanReqT *req;

	TRACE(TRACE_VERBOSE, "wlanMac\n");

	len = sizeof(wlanReqT) - sizeof(*addr) + count * sizeof(*addr);
	req = malloc(len);
	if (req == 0)
		return 0;
	req->handler = wlanMacHandler;
	req->mac.mode = mode;
	req->mac.count = count;
	if (addr != 0)
		memcpy(req->mac.addr, addr, count * sizeof(*addr));

	ret = dspRequest(dsp(), wlan, len, (uint8 *)req);
	free(req);
	return ret;
}

/* ----------------------------------------------------------- */

static void wlanInterworkingHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanInterworkingHandler\n");

	if (wl_interworking(wl(), req->interworking.enable) < 0) {
		TRACE(TRACE_ERROR, "wl_interworking failed\n");
	}
}

/* enable/disable interworking */
int wlanInterworking(wlanT *wlan, int enable)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanInterworking\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanInterworkingHandler;
	req.interworking.enable = enable;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanOsenHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanOsenHandler\n");

	if (wl_osen(wl(), req->osen.enable) < 0) {
		TRACE(TRACE_ERROR, "wl_osen failed\n");
	}
}

/* enable/disable OSEN */
int wlanOsen(wlanT *wlan, int enable)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanOsen\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanOsenHandler;
	req.osen.enable = enable;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanSendFrameHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanSendFrameHandler\n");

	if (wl_send_frame(wl(),	req->sendFrame.len, req->sendFrame.data) < 0) {
		TRACE(TRACE_ERROR, "wl_send_frame failed\n");
	}
}

/* send frame */
int wlanSendFrame(wlanT *wlan, uint16 len, uint8 *data)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanSendFrame\n");

	if (len > ACTION_FRAME_SIZE)
		return 0;

	memset(&req, 0, sizeof(req));
	req.handler = wlanSendFrameHandler;
	req.sendFrame.len = len;
	memcpy(req.sendFrame.data, data, req.sendFrame.len);

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanBssLoadHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanBssLoadHandler\n");

	if (wl_bssload(wl(), req->bssLoad.enable) < 0) {
		TRACE(TRACE_ERROR, "wl_bssload failed\n");
	}
}

/* enable/disable BSS load */
int wlanBssLoad(wlanT *wlan, int enable)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanBssLoad\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanBssLoadHandler;
	req.bssLoad.enable = enable;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanBssLoadStaticHandler(wlanT *wlan,
	int reqLength, wlanReqT *req, void *rspNull)
{
	(void)rspNull;
	if (wlan == 0 || reqLength != sizeof(wlanReqT) || req == 0) {
		TRACE(TRACE_ERROR, "invalid parameter\n");
		return;
	}

	TRACE(TRACE_VERBOSE, "wlanBssLoadStaticHandler\n");

	if (wl_bssload_static(wl(), req->bssLoadStatic.isStatic,
		req->bssLoadStatic.staCount, req->bssLoadStatic.utilization,
		req->bssLoadStatic.aac) < 0) {
		TRACE(TRACE_ERROR, "wl_bssload_static failed\n");
	}
}

/* configure static BSS load */
int wlanBssLoadStatic(wlanT *wlan, bool isStatic, uint16 staCount,
	uint8 utilization, uint16 aac)
{
	wlanReqT req;

	TRACE(TRACE_VERBOSE, "wlanBssLoadStatic\n");

	memset(&req, 0, sizeof(req));
	req.handler = wlanBssLoadStaticHandler;
	req.bssLoadStatic.isStatic = isStatic;
	req.bssLoadStatic.staCount = staCount;
	req.bssLoadStatic.utilization = utilization;
	req.bssLoadStatic.aac = aac;

	return dspRequest(dsp(), wlan, sizeof(req), (uint8 *)&req);
}

/* ----------------------------------------------------------- */

static void wlanProcessWlanEvent(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length)
{
	(void)context;
#ifdef BCMDBG
	WL_P2PO(("WLAN event %s (%d)\n", bcmevent_get_name(eventType), eventType));
#endif	/* BCMDBG */

	if (gEventCallback.fn != 0) {
		gEventCallback.fn(gEventCallback.context,
			eventType, wlEvent, data, length);
	}
}

/* subscribe for event notification callback */
int wlanSubscribeEvent(void *context, void (*fn)(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length))
{
	gEventCallback.fn = fn;
	gEventCallback.context = context;
	return TRUE;
}

/* unsubscribe for event notification callback */
int wlanUnsubscribeEvent(void (*fn)(void *context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length))
{
	(void)fn;
	memset(&gEventCallback, 0, sizeof(gEventCallback));
	return TRUE;
}
