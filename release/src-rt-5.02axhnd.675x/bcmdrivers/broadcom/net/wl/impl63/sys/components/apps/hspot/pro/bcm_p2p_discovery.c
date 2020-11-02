/*
 * P2P discovery state machine.
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
#include <time.h>
#include "bcmendian.h"
#ifndef BCMDRIVER
#include "dsp.h"
#include "tmr.h"
#include "wlu_api.h"
#else
#include "wlc_p2po_disc.h"
#endif /* BCMDRIVER */
#ifdef BCMDBG_ESCAN
#include "bcm_decode_ie.h"
#endif /* BCMDBG_ESCAN */
#include "trace.h"
#include "bcm_p2p_discovery.h"

#ifdef BCMDRIVER
#define BCM_P2P_DISCOVERY_NO_DISPATCHER			/* direct function call */
#else
/* create discovery bsscfg (else host to create) */
#define BCM_P2P_DISCOVERY_CREATE_DISCOVERY_BSSCFG

/* add and delete IEs (else host to add/del) */
#define BCM_P2P_DISCOVERY_ADD_DELETE_IES
#endif // endif

/* 802.11 scan at the start of discovery (else scan after search) */
#undef BCM_P2P_DISCOVERY_SCAN_AT_START_OF_DISCOVERY

/* number of searches before a full scan */
#define SEARCHES_PER_SCAN	32

/* state machine states */
typedef enum {
	STATE_IDLE,
	STATE_SCAN,
	STATE_LISTEN,
	STATE_SEARCH,
	STATE_EXT_LISTEN_ON,
	STATE_EXT_LISTEN_OFF
} stateT;

#ifdef BCMDBG
/* state to string */
static char *state_str[] = {
	"STATE_IDLE",
	"STATE_SCAN",
	"STATE_LISTEN",
	"STATE_SEARCH",
	"STATE_EXT_LISTEN_ON",
	"STATE_EXT_LISTEN_OFF"
};
#endif	/* BCMDBG */

/* state machine events */
typedef enum {
	EVENT_RESET,
	EVENT_START_DISCOVERY,
	EVENT_START_EXT_LISTEN,
	EVENT_SCAN_COMPLETE,
	EVENT_LISTEN_TIMEOUT
} eventT;

#ifdef BCMDBG
/* event to string */
static char *event_str[] = {
	"EVENT_RESET",
	"EVENT_START_DISCOVERY",
	"EVENT_START_EXT_LISTEN",
	"EVENT_SCAN_COMPLETE",
	"EVENT_LISTEN_TIMEOUT"
};
#endif	/* BCMDBG */

#define NUM_SOCIAL_CHANNEL	3

struct bcm_p2p_discovery
{
	/* wl driver handle */
	struct bcm_p2p_discovery_wl_drv_hdl *drv;

	/* bsscfg index */
	int bsscfgIndex;

#ifdef BCM_P2P_DISCOVERY_ADD_DELETE_IES
	/* discovery ethernet address */
	struct ether_addr addr;
#endif	/* BCM_P2P_DISCOVERY_ADD_DELETE_IES */

	/* channel */
	uint16 listenChannel;

	/* exteneded listen */
	uint16 extListenOnTimeout;
	uint16 extListenOffTimeout;

	/* social channels */
	uint16 socialChannel[NUM_SOCIAL_CHANNEL];

	/* timers */
	tmrT *listenTimer;

	/* escan sync id */
	uint16 sync_id;

	/* FSM state */
	stateT state;
	stateT nextState;

	/* search counter */
	uint32 searchCount;
};

/* single instance */
static bcm_p2p_discovery_t gDisc;

typedef struct bcm_p2p_discovery_req bcm_p2p_discovery_req_t;

/* request handler */
typedef void (*request_handler_t)(bcm_p2p_discovery_t *disc,
	int reqLength, bcm_p2p_discovery_req_t *req, void *rspData);

typedef struct {
	struct bcm_p2p_discovery_wl_drv_hdl *drv;
	uint16 listenChannel;
} create_req_t;

typedef struct {
	bcm_p2p_discovery_t *disc;
} create_rsp_t;

typedef struct {
	uint16 listenOnTimeout;
	uint16 listenOffTimeout;
} start_ext_listen_req_t;

struct bcm_p2p_discovery_req {
	request_handler_t handler;
	union {
		create_req_t create;
		start_ext_listen_req_t startExtListen;
	};
};

/* forward declarations */
static void fsm(bcm_p2p_discovery_t *disc, eventT event);

/* ----------------------------------------------------------- */

#ifdef BCM_P2P_DISCOVERY_ADD_DELETE_IES
static uchar prbrspIe[] = {
	0x50, 0x6f, 0x9a, 0x09, 0x02, 0x02, 0x00, 0x27,
	0x0c, 0x0d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x88, 0x00, 0x01, 0x00, 0x50,
	0xf2, 0x04, 0x00, 0x05, 0x00, 0x10, 0x11, 0x00,
	0x08, 0x42, 0x72, 0x6f, 0x61, 0x64, 0x63, 0x6f,
	0x6d
};

static void addIes(bcm_p2p_discovery_t *disc)
{
	int i;

	/* update IE with the discovery address */
	for (i = 0; i < (int)sizeof(disc->addr); i++) {
		prbrspIe[12 + i] = disc->addr.octet[i];
	}

	if (wl_add_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBRSP_FLAG, 133, (uchar *)
		"\x00\x50\xf2\x04\x10\x4a\x00\x01\x10\x10\x44\x00\x01\x02\x10\x41"
		"\x00\x01\x01\x10\x12\x00\x02\x00\x00\x10\x53\x00\x02\x01\x88\x10"
		"\x3b\x00\x01\x03\x10\x47\x00\x10\x22\x21\x02\x03\x04\x05\x06\x07"
		"\x08\x09\x7a\xe4\x00\x4a\xf8\x5e\x10\x21\x00\x08\x42\x72\x6f\x61"
		"\x64\x63\x6f\x6d\x10\x23\x00\x06\x53\x6f\x66\x74\x41\x50\x10\x24"
		"\x00\x01\x30\x10\x42\x00\x01\x30\x10\x54\x00\x08\x00\x01\x00\x50"
		"\xf2\x04\x00\x05\x10\x11\x00\x08\x42\x72\x6f\x61\x64\x63\x6f\x6d"
		"\x10\x08\x00\x02\x01\x08\x10\x3c\x00\x01\x01\x10\x49\x00\x06\x00"
		"\x37\x2a\x00\x01\x20") < 0) {
		WL_ERROR(("failed to add vendor IE\n"));
	}
	if (wl_add_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBRSP_FLAG, sizeof(prbrspIe), prbrspIe) < 0) {
		WL_ERROR(("failed to add vendor IE\n"));
	}
	if (wl_add_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBREQ_FLAG, 124, (uchar *)
		"\x00\x50\xf2\x04\x10\x4a\x00\x01\x10\x10\x3a\x00\x01\x00\x10\x08"
		"\x00\x02\x01\x88\x10\x47\x00\x10\x22\x21\x02\x03\x04\x05\x06\x07"
		"\x08\x09\x7a\xe4\x00\x4a\xf8\x5e\x10\x54\x00\x08\x00\x01\x00\x50"
		"\xf2\x04\x00\x05\x10\x3c\x00\x01\x01\x10\x02\x00\x02\x00\x00\x10"
		"\x09\x00\x02\x00\x00\x10\x12\x00\x02\x00\x00\x10\x21\x00\x08\x42"
		"\x72\x6f\x61\x64\x63\x6f\x6d\x10\x23\x00\x06\x53\x6f\x66\x74\x41"
		"\x50\x10\x24\x00\x01\x30\x10\x11\x00\x08\x42\x72\x6f\x61\x64\x63"
		"\x6f\x6d\x10\x49\x00\x06\x00\x37\x2a\x00\x01\x20") < 0) {
		WL_ERROR(("failed to add vendor IE\n"));
	}
	if (wl_add_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBREQ_FLAG, 25, (uchar *)
		"\x50\x6f\x9a\x09\x02\x02\x00\x27\x0c\x06\x05\x00\x55\x53\x04\x51"
		"\x0b\x11\x05\x00\x55\x53\x04\x51\x0b") < 0) {
		WL_ERROR(("failed to add vendor IE\n"));
	}
}

static void deleteIes(bcm_p2p_discovery_t *disc)
{
	if (wl_del_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBRSP_FLAG, 133, (uchar *)
		"\x00\x50\xf2\x04\x10\x4a\x00\x01\x10\x10\x44\x00\x01\x02\x10\x41"
		"\x00\x01\x01\x10\x12\x00\x02\x00\x00\x10\x53\x00\x02\x01\x88\x10"
		"\x3b\x00\x01\x03\x10\x47\x00\x10\x22\x21\x02\x03\x04\x05\x06\x07"
		"\x08\x09\x7a\xe4\x00\x4a\xf8\x5e\x10\x21\x00\x08\x42\x72\x6f\x61"
		"\x64\x63\x6f\x6d\x10\x23\x00\x06\x53\x6f\x66\x74\x41\x50\x10\x24"
		"\x00\x01\x30\x10\x42\x00\x01\x30\x10\x54\x00\x08\x00\x01\x00\x50"
		"\xf2\x04\x00\x05\x10\x11\x00\x08\x42\x72\x6f\x61\x64\x63\x6f\x6d"
		"\x10\x08\x00\x02\x01\x08\x10\x3c\x00\x01\x01\x10\x49\x00\x06\x00"
		"\x37\x2a\x00\x01\x20") < 0) {
		WL_P2PO(("failed to delete vendor IE\n"));
	}
	if (wl_del_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBRSP_FLAG, sizeof(prbrspIe), prbrspIe) < 0) {
		WL_P2PO(("failed to delete vendor IE\n"));
	}
	if (wl_del_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBREQ_FLAG, 124, (uchar *)
		"\x00\x50\xf2\x04\x10\x4a\x00\x01\x10\x10\x3a\x00\x01\x00\x10\x08"
		"\x00\x02\x01\x88\x10\x47\x00\x10\x22\x21\x02\x03\x04\x05\x06\x07"
		"\x08\x09\x7a\xe4\x00\x4a\xf8\x5e\x10\x54\x00\x08\x00\x01\x00\x50"
		"\xf2\x04\x00\x05\x10\x3c\x00\x01\x01\x10\x02\x00\x02\x00\x00\x10"
		"\x09\x00\x02\x00\x00\x10\x12\x00\x02\x00\x00\x10\x21\x00\x08\x42"
		"\x72\x6f\x61\x64\x63\x6f\x6d\x10\x23\x00\x06\x53\x6f\x66\x74\x41"
		"\x50\x10\x24\x00\x01\x30\x10\x11\x00\x08\x42\x72\x6f\x61\x64\x63"
		"\x6f\x6d\x10\x49\x00\x06\x00\x37\x2a\x00\x01\x20") < 0) {
		WL_P2PO(("failed to delete vendor IE\n"));
	}
	if (wl_del_vndr_ie(disc->drv, disc->bsscfgIndex,
		VNDR_IE_PRBREQ_FLAG, 25, (uchar *)
		"\x50\x6f\x9a\x09\x02\x02\x00\x27\x0c\x06\x05\x00\x55\x53\x04\x51"
		"\x0b\x11\x05\x00\x55\x53\x04\x51\x0b") < 0) {
		WL_P2PO(("failed to delete vendor IE\n"));
	}
}
#endif	/* BCM_P2P_DISCOVERY_ADD_DELETE_IES */

/* ----------------------------------------------------------- */

static void listenTimeout(void *arg)
{
	bcm_p2p_discovery_t *disc = (bcm_p2p_discovery_t *)arg;

	WL_TRACE(("listenTimeout callback\n"));
	fsm(disc, EVENT_LISTEN_TIMEOUT);
}

/* ----------------------------------------------------------- */

#ifndef BCMDRIVER
static void initialize_handler(bcm_p2p_discovery_t *discNull,
	int reqLength, bcm_p2p_discovery_req_t *req, void *rspNull)
{
	(void)discNull;
	(void)rspNull;
	if (reqLength != sizeof(bcm_p2p_discovery_req_t) || req == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("initialize_handler\n"));

	if (wl_enable_event_msg(wl(), WLC_E_ESCAN_RESULT) < 0) {
		WL_ERROR(("failed to enable escan result event\n"));
	}
}

/* initialize P2P discovery */
int bcm_p2p_discovery_initialize(void)
{
	bcm_p2p_discovery_req_t req;

	WL_TRACE(("bcm_p2p_discovery_initialize\n"));

	/* attach handler to dispatcher */
	dspSubscribe(dsp(), 0, bcm_p2p_discovery_process_wlan_event);

	req.handler = initialize_handler;
	return dspRequestSynch(dsp(), 0, sizeof(req), (uint8 *)&req, 0);
}

/* ----------------------------------------------------------- */

/* deinitialize P2P discovery */
int bcm_p2p_discovery_deinitialize(void)
{
	/* detach handler */
	dspUnsubscribe(dsp(), bcm_p2p_discovery_process_wlan_event);
	return TRUE;
}
#endif /* BCMDRIVER */

/* ----------------------------------------------------------- */

static void create_handler(bcm_p2p_discovery_t *discNull,
	int reqLength, bcm_p2p_discovery_req_t *req, void *rspData)
{
	bcm_p2p_discovery_t *disc;
	create_rsp_t *rsp = (create_rsp_t *)rspData;

	(void)discNull;
	if (reqLength != sizeof(bcm_p2p_discovery_req_t) || req == 0 || rspData == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("create_handler\n"));

#ifndef BCMDRIVER
	/* seed the random generator */
	srand((unsigned)time(NULL));
#endif /* BCMDRIVER */

	rsp->disc = 0;
	disc = &gDisc;
	memset(disc, 0, sizeof(*disc));

	disc->drv = req->create.drv;

	disc->listenChannel = req->create.listenChannel;

	/* initialize social channels */
	disc->socialChannel[0] = 1;
	disc->socialChannel[1] = 6;
	disc->socialChannel[2] = 11;

#ifdef BCM_P2P_DISCOVERY_CREATE_DISCOVERY_BSSCFG
	/* disable P2P discovery - to ensure bsscfg does not exist */
	wl_p2p_disc(disc->drv, FALSE);
	/* enable P2P discovery */
	wl_p2p_disc(disc->drv, TRUE);
#endif	/* BCM_P2P_DISCOVERY_CREATE_DISCOVERY_BSSCFG */

	if (wl_p2p_dev(disc->drv, &disc->bsscfgIndex) < 0) {
		WL_ERROR(("failed to get bsscfg index\n"));
	}
	WL_P2PO(("bsscfg index=%d\n", disc->bsscfgIndex));
#ifdef BCM_P2P_DISCOVERY_ADD_DELETE_IES
	wl_cur_etheraddr(disc->drv, disc->bsscfgIndex, &disc->addr);
	WL_PRMAC("discovery MAC address", &disc->addr);
#endif /* BCM_P2P_DISCOVERY_ADD_DELETE_IES */

#ifdef BCM_P2P_DISCOVERY_ADD_DELETE_IES
	addIes(disc);
#endif	/* BCM_P2P_DISCOVERY_ADD_DELETE_IES */

	/* create timers */
	disc->listenTimer = tmrCreate(
#ifndef BCMDRIVER
		dsp(),
#else
		disc->drv,
#endif	/* BCMDRIVER */
		listenTimeout, disc, "listenTimer");
	if (disc->listenTimer == 0) {
		WL_ERROR(("failed to create timer\n"));
		goto fail;
	}

	/* reset state machine */
	fsm(disc, EVENT_RESET);

	/* return created instance */
	rsp->disc = disc;
	return;

fail:
	memset(disc, 0, sizeof(*disc));
}

/* create P2P discovery */
bcm_p2p_discovery_t *bcm_p2p_discovery_create(
	struct bcm_p2p_discovery_wl_drv_hdl *drv, uint16 listenChannel)
{
	bcm_p2p_discovery_req_t req;
	create_rsp_t rsp;

	WL_TRACE(("bcm_p2p_discovery_create\n"));

	req.handler = create_handler;
	req.create.drv = drv;
	req.create.listenChannel = listenChannel;
#ifdef BCM_P2P_DISCOVERY_NO_DISPATCHER
	create_handler(0, sizeof(req), &req, &rsp);
#else
	if (!dspRequestSynch(dsp(), 0, sizeof(req), (uint8 *)&req, (uint8 *)&rsp))
	{
		return 0;
	}
#endif /* BCM_P2P_DISCOVERY_NO_DISPATCHER */

	return rsp.disc;
}

/* ----------------------------------------------------------- */

static void destroy_handler(bcm_p2p_discovery_t *disc,
	int reqLength, bcm_p2p_discovery_req_t *req, void *rspNull)
{
	(void)rspNull;
	if (disc == 0 || reqLength != sizeof(bcm_p2p_discovery_req_t) || req == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("destroy_handler\n"));

#ifdef BCM_P2P_DISCOVERY_ADD_DELETE_IES
	deleteIes(disc);
#endif	/* BCM_P2P_DISCOVERY_ADD_DELETE_IES */

#ifdef BCM_P2P_DISCOVERY_CREATE_DISCOVERY_BSSCFG
	/* disable P2P discovery */
	wl_p2p_disc(disc->drv, FALSE);
#endif	/* BCM_P2P_DISCOVERY_CREATE_DISCOVERY_BSSCFG */

	disc->bsscfgIndex = 0;

	/* stop timers */
	tmrStop(disc->listenTimer);

	/* destroy timers */
	tmrDestroy(disc->listenTimer);

	memset(disc, 0, sizeof(*disc));
}

/* destroy P2P discovery */
int bcm_p2p_discovery_destroy(bcm_p2p_discovery_t *disc)
{
	bcm_p2p_discovery_req_t req;

	WL_TRACE(("bcm_p2p_discovery_destroy\n"));

	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}

	req.handler = destroy_handler;
#ifdef BCM_P2P_DISCOVERY_NO_DISPATCHER
	destroy_handler(disc, sizeof(req), &req, 0);
	return 1;
#else
	return dspRequestSynch(dsp(), disc, sizeof(req), (uint8 *)&req, 0);
#endif /* BCM_P2P_DISCOVERY_NO_DISPATCHER */
}

/* ----------------------------------------------------------- */

static void reset_handler(bcm_p2p_discovery_t *disc,
	int reqLength, bcm_p2p_discovery_req_t *req, void *rspNull)
{
	(void)rspNull;
	if (disc == 0 || reqLength != sizeof(bcm_p2p_discovery_req_t) || req == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("reset_handler\n"));
	fsm(disc, EVENT_RESET);
}

/* reset P2P discovery */
int bcm_p2p_discovery_reset(bcm_p2p_discovery_t *disc)
{
	bcm_p2p_discovery_req_t req;

	WL_TRACE(("bcm_p2p_discovery_reset\n"));

	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}

	req.handler = reset_handler;
#ifdef BCM_P2P_DISCOVERY_NO_DISPATCHER
	reset_handler(disc, sizeof(req), &req, 0);
	return 1;
#else
	return dspRequest(dsp(), disc, sizeof(req), (uint8 *)&req);
#endif /* BCM_P2P_DISCOVERY_NO_DISPATCHER */
}

/* ----------------------------------------------------------- */

static void start_discovery_handler(bcm_p2p_discovery_t *disc,
	int reqLength, bcm_p2p_discovery_req_t *req, void *rspNull)
{
	(void)rspNull;
	if (disc == 0 || reqLength != sizeof(bcm_p2p_discovery_req_t) || req == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("start_discovery_handler\n"));
	fsm(disc, EVENT_START_DISCOVERY);
}

/* start P2P discovery */
int bcm_p2p_discovery_start_discovery(bcm_p2p_discovery_t *disc)
{
	bcm_p2p_discovery_req_t req;

	WL_TRACE(("bcm_p2p_discovery_start_discovery\n"));

	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}

	req.handler = start_discovery_handler;
#ifdef BCM_P2P_DISCOVERY_NO_DISPATCHER
	start_discovery_handler(disc, sizeof(req), &req, 0);
	return 1;
#else
	return dspRequest(dsp(), disc, sizeof(req), (uint8 *)&req);
#endif /* BCM_P2P_DISCOVERY_NO_DISPATCHER */
}

/* ----------------------------------------------------------- */

static void start_ext_listen_handler(bcm_p2p_discovery_t *disc,
	int reqLength, bcm_p2p_discovery_req_t *req, void *rspNull)
{
	(void)rspNull;
	if (disc == 0 || reqLength != sizeof(bcm_p2p_discovery_req_t) || req == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("start_ext_listen_handler %d %d\n",
		req->startExtListen.listenOnTimeout,
		req->startExtListen.listenOffTimeout));
	disc->extListenOnTimeout = req->startExtListen.listenOnTimeout;
	disc->extListenOffTimeout = req->startExtListen.listenOffTimeout;
	fsm(disc, EVENT_START_EXT_LISTEN);
}

/* start P2P extended listen */
int bcm_p2p_discovery_start_ext_listen(bcm_p2p_discovery_t *disc,
	uint16 listenOnTimeout, uint16 listenOffTimeout)
{
	bcm_p2p_discovery_req_t req;

	WL_TRACE(("bcm_p2p_discovery_start_ext_listen\n"));

	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}

	req.handler = start_ext_listen_handler;
	req.startExtListen.listenOnTimeout = listenOnTimeout;
	req.startExtListen.listenOffTimeout = listenOffTimeout;
#ifdef BCM_P2P_DISCOVERY_NO_DISPATCHER
	start_ext_listen_handler(disc, sizeof(req), &req, 0);
	return 1;
#else
	return dspRequest(dsp(), disc, sizeof(req), (uint8 *)&req);
#endif /* BCM_P2P_DISCOVERY_NO_DISPATCHER */
}

/* ----------------------------------------------------------- */

/* get bsscfg index of P2P discovery interface */
/* bsscfg index is valid only after started */
int bcm_p2p_discovery_get_bsscfg_index(bcm_p2p_discovery_t *disc)
{
	return disc->bsscfgIndex;
}

/* ----------------------------------------------------------- */

void bcm_p2p_discovery_process_wlan_event(void * context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length)
{
	bcm_p2p_discovery_t *disc = &gDisc;
	(void)context;
#ifndef BCMDBG_ESCAN
	(void)data;
	(void)length;
#endif // endif

#ifdef BCMDBG
	WL_P2PO(("WLAN event %s (%d)\n", bcmevent_get_name(eventType), eventType));
#endif	/* BCMDBG */

	if (eventType == WLC_E_ESCAN_RESULT) {
		if (wlEvent->status == WLC_E_STATUS_PARTIAL) {
#ifdef BCMDBG_ESCAN
			wl_escan_result_t *escan_data = (wl_escan_result_t *)data;

			if (length >= sizeof(*escan_data)) {
				wl_bss_info_t *bi = &escan_data->bss_info[0];
				bcm_decode_probe_response_t pr;
				struct ether_addr *addr;

				if (!bcm_decode_ie_probe_response(bi, &pr)) {
					return;
				}

				/* default address */
				addr = &bi->BSSID;

				/* P2P not supported */
				if (!pr.isP2P) {
					char ssidbuf[4*32+1];
					wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);
					printf(" AP   %-20.20s   %s   %d\n", ssidbuf,
						wl_ether_etoa(addr), pr.channel);
					return;
				}

				if (pr.isP2PDeviceInfoDecoded) {
					/* use device address */
					addr = &pr.p2pDeviceInfo.deviceAddress;
					printf("P2P   %-20.20s   %s   %d\n",
						pr.p2pDeviceInfo.deviceName,
						wl_ether_etoa(addr), pr.channel);
				}
			}
#endif /* BCMDBG_ESCAN */
		}
		else if (wlEvent->status == WLC_E_STATUS_SUCCESS) {
			WL_P2PO(("WLC_E_ESCAN_RESULT status=WLC_E_STATUS_SUCCESS\n"));
			fsm(disc, EVENT_SCAN_COMPLETE);
		}
		else {
			WL_P2PO(("WLC_E_ESCAN_RESULT status=%d\n", wlEvent->status));
			/* escan may have failed/restarted but keep state machine running */
			fsm(disc, EVENT_SCAN_COMPLETE);
		}
	}
}

static void change_state(bcm_p2p_discovery_t *disc, stateT next)
{
	/* state transition occurs at the end of fsm() processing */
	disc->nextState = next;
}

/* reset all variables returning to idle state */
static void idle_reset(bcm_p2p_discovery_t *disc)
{
	/* default P2P state */
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SCAN, 0, 0);

	/* stop timers */
	tmrStop(disc->listenTimer);

	disc->searchCount = 0;
	change_state(disc, STATE_IDLE);
}

/* 802.11 scan all channels */
static void scan(bcm_p2p_discovery_t *disc)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SCAN, 0, 0);
	wl_p2p_scan(disc->drv, ++disc->sync_id, TRUE, -1, -1, -1, 0, 0);
	change_state(disc, STATE_SCAN);
}

/* random timeout for listen */
static uint16 randomListenTimeout(void)
{
	/* 100, 200, or 300 msec */
	return ((rand() % 3) + 1) * 100;
}

/* listen mode */
static void listen(bcm_p2p_discovery_t *disc, uint16 timeout)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_LISTEN,
		CH20MHZ_CHSPEC(disc->listenChannel), timeout);
	WL_P2PO(("listen timer started %d msec\n", timeout));
	tmrStart(disc->listenTimer, timeout, FALSE);
	change_state(disc, STATE_LISTEN);
}

/* extended listen on mode */
static void extendedListenOn(bcm_p2p_discovery_t *disc, uint16 timeout)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_LISTEN,
		CH20MHZ_CHSPEC(disc->listenChannel), timeout);
	if (timeout > 0) {
		WL_P2PO(("listen timer started %d msec\n", timeout));
		tmrStart(disc->listenTimer, timeout, FALSE);
	}
	change_state(disc, STATE_EXT_LISTEN_ON);
}

/* extended listen off mode */
static void extendedListenOff(bcm_p2p_discovery_t *disc, uint16 timeout)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SCAN, 0, 0);
	if (timeout > 0) {
		WL_P2PO(("listen timer started %d msec\n", timeout));
		tmrStart(disc->listenTimer, timeout, FALSE);
	}
	change_state(disc, STATE_EXT_LISTEN_OFF);
}

/* search mode */
static void search(bcm_p2p_discovery_t *disc)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SEARCH, 0, 0);
	wl_p2p_scan(disc->drv, ++disc->sync_id, TRUE, -1, -1, -1,
		NUM_SOCIAL_CHANNEL, disc->socialChannel);
	disc->searchCount++;
	change_state(disc, STATE_SEARCH);
}

/* common event processing for all states */
static void default_event_processing(bcm_p2p_discovery_t *disc, eventT event)
{
	switch (event) {
	case EVENT_RESET:
		WL_TRACE(("default_event_processing: EVENT_RESET\n"));
		idle_reset(disc);
		break;
	case EVENT_START_DISCOVERY:
		break;
	case EVENT_START_EXT_LISTEN:
		break;
	case EVENT_SCAN_COMPLETE:
		break;
	case EVENT_LISTEN_TIMEOUT:
		break;
	default:
		WL_ERROR(("invalid event %d\n", event));
		break;
	}
}

static void state_idle(bcm_p2p_discovery_t *disc, eventT event)
{
	switch (event) {
	case EVENT_START_DISCOVERY:
	case EVENT_START_EXT_LISTEN:
		if (event == EVENT_START_DISCOVERY) {
#ifdef BCM_P2P_DISCOVERY_SCAN_AT_START_OF_DISCOVERY
			scan(disc);
#else
			search(disc);
#endif	/* BCM_P2P_DISCOVERY_SCAN_AT_START_OF_DISCOVERY */
		}
		else {
			extendedListenOn(disc, disc->extListenOnTimeout);
		}
		break;
	case EVENT_SCAN_COMPLETE:
		break;
	case EVENT_LISTEN_TIMEOUT:
		break;
	default:
		default_event_processing(disc, event);
		break;
	}
}

static void stateScan(bcm_p2p_discovery_t *disc, eventT event)
{
	switch (event) {
	case EVENT_START_DISCOVERY:
		break;
	case EVENT_START_EXT_LISTEN:
		break;
	case EVENT_SCAN_COMPLETE:
		listen(disc, randomListenTimeout());
		break;
	case EVENT_LISTEN_TIMEOUT:
		break;
	default:
		default_event_processing(disc, event);
		break;
	}
}

static void stateListen(bcm_p2p_discovery_t *disc, eventT event)
{
	switch (event) {
	case EVENT_START_DISCOVERY:
		break;
	case EVENT_SCAN_COMPLETE:
		break;
	case EVENT_LISTEN_TIMEOUT:
		search(disc);
		break;
	default:
		default_event_processing(disc, event);
		break;
	}
}

static void stateSearch(bcm_p2p_discovery_t *disc, eventT event)
{
	switch (event) {
	case EVENT_START_DISCOVERY:
		break;
	case EVENT_START_EXT_LISTEN:
		break;
	case EVENT_SCAN_COMPLETE:
		WL_P2PO(("search count=%d\n", disc->searchCount));
		if ((disc->searchCount % SEARCHES_PER_SCAN) == 0)
			scan(disc);
		else
			listen(disc, randomListenTimeout());
		break;
	case EVENT_LISTEN_TIMEOUT:
		break;
	default:
		default_event_processing(disc, event);
		break;
	}
}

static void stateExtListenOn(bcm_p2p_discovery_t *disc, eventT event)
{
	switch (event) {
	case EVENT_START_DISCOVERY:
		break;
	case EVENT_START_EXT_LISTEN:
		break;
	case EVENT_SCAN_COMPLETE:
		break;
	case EVENT_LISTEN_TIMEOUT:
		if (disc->extListenOffTimeout > 0)
			extendedListenOff(disc, disc->extListenOffTimeout);
		else
			extendedListenOn(disc, disc->extListenOnTimeout);
		break;
	default:
		default_event_processing(disc, event);
		break;
	}
}

static void stateExtListenOff(bcm_p2p_discovery_t *disc, eventT event)
{
	switch (event) {
	case EVENT_START_DISCOVERY:
		break;
	case EVENT_START_EXT_LISTEN:
		break;
	case EVENT_SCAN_COMPLETE:
		break;
	case EVENT_LISTEN_TIMEOUT:
		extendedListenOn(disc, disc->extListenOnTimeout);
		break;
	default:
		default_event_processing(disc, event);
		break;
	}
}

/* P2P discovery finite state machine */
static void fsm(bcm_p2p_discovery_t *disc, eventT event)
{
	WL_P2PO(("--------------------------------------------------------\n"));
	WL_P2PO(("current state=%s event=%s\n",
		state_str[disc->state], event_str[event]));

	switch (disc->state) {
	case STATE_IDLE:
		state_idle(disc, event);
		break;
	case STATE_SCAN:
		stateScan(disc, event);
		break;
	case STATE_LISTEN:
		stateListen(disc, event);
		break;
	case STATE_SEARCH:
		stateSearch(disc, event);
		break;
	case STATE_EXT_LISTEN_ON:
		stateExtListenOn(disc, event);
		break;
	case STATE_EXT_LISTEN_OFF:
		stateExtListenOff(disc, event);
		break;
	default:
		WL_ERROR(("invalid state %d\n", disc->state));
		break;
	}

	/* transition to next state */
	if (disc->state != disc->nextState) {
		WL_P2PO(("state change %s -> %s\n",
			state_str[disc->state], state_str[disc->nextState]));
		disc->state = disc->nextState;
	}

	WL_P2PO(("--------------------------------------------------------\n"));
}
