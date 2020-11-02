/**
 * @file
 * @brief
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
 * $Id: bcm_p2p_disc.c 708017 2017-06-29 14:11:45Z $
 */

#include "bcmendian.h"
#ifndef BCMDRIVER
#include "dsp.h"
#include "tmr.h"
#include "wlu_api.h"
#else
#include "wl_p2po_disc.h"
#endif /* BCMDRIVER */
#ifdef BCMDBG_ESCAN
#include "pktDecodeIe.h"
#endif /* BCMDBG_ESCAN */
#include "wl_dbg.h"
#include "bcm_p2p_disc.h"

#ifdef BCMDRIVER
#define BCM_P2P_DISC_NO_DISPATCHER			/* direct function call */
#else
/* create discovery bsscfg (else host to create) */
#define BCM_P2P_DISC_CREATE_DISCOVERY_BSSCFG

/* add and delete IEs (else host to add/del) */
#define BCM_P2P_DISC_ADD_DELETE_IES
#endif // endif

/* 802.11 scan at the start of discovery (else scan after search) */
#undef BCM_P2P_DISC_SCAN_AT_START_OF_DISCOVERY

/* number of searches before a full scan */
#define SEARCHES_PER_SCAN	32

/* state machine states */
typedef enum {
	STATE_IDLE,
	STATE_SCAN,
	STATE_LISTEN,
	STATE_SEARCH,
	STATE_EXT_LISTEN_ON,
	STATE_EXT_LISTEN_OFF,
	STATE_LAST
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
	EVENT_LISTEN_TIMEOUT,
	EVENT_LAST
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

/* P2P search parameters */
typedef struct {
	int32 homeTime;
	uint16 *socialChannels;
	uint8 numSocialChannels;
	uint16 socialChannelsAllocSize;
	uint8 flags;
	uint8 pad[3];
} bcm_p2p_disc_search_params_t;

struct bcm_p2p_disc_s
{
	/* wl driver handle */
	struct bcm_p2p_wl_drv_hdl *drv;

	/* bsscfg index */
	int bsscfgIndex;

#ifdef BCM_P2P_DISC_ADD_DELETE_IES
	/* discovery ethernet address */
	struct ether_addr addr;
#endif	/* BCM_P2P_DISC_ADD_DELETE_IES */

	/* channel */
	uint16 listenChannel;

	/* exteneded listen */
	uint16 extListenOnTimeout;
	uint16 extListenOffTimeout;

	uint16 pad;
	bcm_p2p_disc_search_params_t *searchParams;

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
static bcm_p2p_disc_t gDisc;

/** To prevent ROMming shdat issue because of ROMmed functions accessing RAM */
static bcm_p2p_disc_t*
BCMRAMFN(get_gDisc)(void)
{
	return &gDisc;
}
typedef struct bcm_p2p_disc_req bcm_p2p_disc_req_t;

/* request handler */
typedef void (*request_handlerT)(bcm_p2p_disc_t *disc,
	int reqLength, bcm_p2p_disc_req_t *req, void *rspData);

typedef struct {
	struct bcm_p2p_wl_drv_hdl *drv;
	uint16 listenChannel;
} bcm_p2p_disc_create_req_t;

typedef struct {
	bcm_p2p_disc_t *disc;
} bcm_p2p_disc_create_rsp_t;

typedef struct {
	uint16 listenOnTimeout;
	uint16 listenOffTimeout;
} bcm_p2p_disc_start_ext_listen_req_t;

struct bcm_p2p_disc_req {
	request_handlerT handler;
	union {
		bcm_p2p_disc_create_req_t create;
		bcm_p2p_disc_start_ext_listen_req_t startExtListen;
	};
};

/* Module persistent data with a lifespan longer than the single gDisc instance */
static bcm_p2p_disc_search_params_t gDiscPersistent;

/** To prevent ROMming shdat issue because of ROMmed functions accessing RAM */
static bcm_p2p_disc_search_params_t* BCMRAMFN(get_gDiscPersistentData)(void)
{
	return &gDiscPersistent;
}

static const uint16 defaultSocialChans[3] = { 1, 6, 11 };

/* Initialize module persistent config data */
int bcm_p2p_disc_config_init(void)
{
	bcm_p2p_disc_search_params_t *config = get_gDiscPersistentData();

	/* Set persistent config data to default values */
	config->homeTime = -1;
	config->numSocialChannels = 3;
	config->socialChannels = (uint16*) &defaultSocialChans[0];
	config->socialChannelsAllocSize = 0;

	return BCME_OK;
}

/* Clean up module persistent config data */
int bcm_p2p_disc_config_cleanup(struct bcm_p2p_wl_drv_hdl *drv)
{
	wlc_info_t *wlc = (wlc_info_t *) drv;
	bcm_p2p_disc_search_params_t *config = get_gDiscPersistentData();

	if (config->socialChannelsAllocSize > 0) {
		MFREE(wlc->osh, config->socialChannels, config->socialChannelsAllocSize);
	}

	/* Set persistent config data to default values */
	bcm_p2p_disc_config_init();

	return BCME_OK;
}

/* Set module persistent config data */
int bcm_p2p_disc_config_set(struct bcm_p2p_wl_drv_hdl *drv, int32 home_time,
	uint8 flags, uint8 num_social_channels, uint16 *social_channels)
{
	wlc_info_t *wlc = (wlc_info_t *) drv;
	bcm_p2p_disc_search_params_t *config = get_gDiscPersistentData();

	/* Free any previous allocated memory for config data */
	bcm_p2p_disc_config_cleanup(drv);

	/* Set the new config data */
	config->homeTime = home_time;
	config->flags = flags;
	config->numSocialChannels = num_social_channels;
	config->socialChannelsAllocSize = config->numSocialChannels *
		sizeof(*config->socialChannels);
	config->socialChannels = MALLOC(wlc->osh, config->socialChannelsAllocSize);
	if (config->socialChannels == NULL) {
		WL_ERROR(("%s: no mem\n", __FUNCTION__));
		/* Set persistent config data to default values */
		bcm_p2p_disc_config_init();
		return BCME_NOMEM;
	}
	memcpy(config->socialChannels, social_channels, config->socialChannelsAllocSize);

	return BCME_OK;
}

/* Get module persistent config data */
int bcm_p2p_disc_config_get(int32 *home_time, uint8 *flags,
	uint8 *num_social_channels, uint16 **social_channels)
{
	bcm_p2p_disc_search_params_t *config = get_gDiscPersistentData();

	*home_time = config->homeTime;
	*flags = config->flags;
	*num_social_channels = config->numSocialChannels;
	*social_channels = config->socialChannels;

	return BCME_OK;
}

/* state transition occurs at the end of fsm() processing */
#define changeState(disc, next)	((disc)->nextState = (next))

/* forward declarations */
static void fsm(bcm_p2p_disc_t *disc, eventT event);

/* ----------------------------------------------------------- */

#ifdef BCM_P2P_DISC_ADD_DELETE_IES
static uchar prbrspIe[] = {
	0x50, 0x6f, 0x9a, 0x09, 0x02, 0x02, 0x00, 0x27,
	0x0c, 0x0d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x88, 0x00, 0x01, 0x00, 0x50,
	0xf2, 0x04, 0x00, 0x05, 0x00, 0x10, 0x11, 0x00,
	0x08, 0x42, 0x72, 0x6f, 0x61, 0x64, 0x63, 0x6f,
	0x6d
};

static void addIes(bcm_p2p_disc_t *disc)
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

static void deleteIes(bcm_p2p_disc_t *disc)
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
#endif	/* BCM_P2P_DISC_ADD_DELETE_IES */

/* ----------------------------------------------------------- */

static void listenTimeout(void *arg)
{
	bcm_p2p_disc_t *disc = (bcm_p2p_disc_t *)arg;

	WL_TRACE(("listenTimeout callback\n"));
	fsm(disc, EVENT_LISTEN_TIMEOUT);
}

/* ----------------------------------------------------------- */

#ifndef BCMDRIVER
static void bcm_p2p_disc_init_handler(bcm_p2p_disc_t *discNull,
	int reqLength, bcm_p2p_disc_req_t *req, void *rspNull)
{
	(void)discNull;
	(void)rspNull;
	if (reqLength != sizeof(bcm_p2p_disc_req_t) || req == 0) {
		WL_ERROR(("invalid parameter\n"));
		return;
	}
	WL_TRACE(("bcm_p2p_disc_init_handler\n"));

	if (wl_enable_event_msg(wl(), WLC_E_ESCAN_RESULT) < 0) {
		WL_ERROR(("failed to enable escan result event\n"));
	}
}

/* initialize WFD discovery */
int bcm_p2p_disc_init(void)
{
	bcm_p2p_disc_req_t req;

	WL_TRACE(("bcm_p2p_disc_init\n"));

	/* attach handler to dispatcher */
	dspSubscribe(dsp(), 0, bcm_p2p_disc_process_wlan_event);

	req.handler = bcm_p2p_disc_init_handler;
	return dspRequestSynch(dsp(), 0, sizeof(req), (uint8 *)&req, 0);
}

/* ----------------------------------------------------------- */

/* deinitialize WFD discovery */
int bcm_p2p_disc_deinit(void)
{
	/* detach handler */
	dspUnsubscribe(dsp(), bcm_p2p_disc_process_wlan_event);
	return TRUE;
}
#endif /* BCMDRIVER */

/* ----------------------------------------------------------- */

bcm_p2p_disc_t *bcm_p2p_disc_create(struct bcm_p2p_wl_drv_hdl *drv, uint16 listenChannel)
{
	bcm_p2p_disc_t *disc;

	WL_TRACE(("bcm_p2p_disc_create\n"));

#ifndef BCMDRIVER
#endif /* BCMDRIVER */

	disc = get_gDisc();
	memset(disc, 0, sizeof(*disc));

	disc->drv = drv;
	disc->listenChannel = listenChannel;
	disc->searchParams = get_gDiscPersistentData();

#ifdef BCM_P2P_DISC_CREATE_DISCOVERY_BSSCFG
	/* disable P2P discovery - to ensure bsscfg does not exist */
	wl_p2p_disc(disc->drv, FALSE);
	/* enable P2P discovery */
	wl_p2p_disc(disc->drv, TRUE);
#endif	/* BCM_P2P_DISC_CREATE_DISCOVERY_BSSCFG */

	if (wl_p2p_dev(disc->drv, &disc->bsscfgIndex) < 0) {
		WL_ERROR(("failed to get bsscfg index\n"));
	}
	WL_P2PO(("bsscfg index=%d\n", disc->bsscfgIndex));
#ifdef BCM_P2P_DISC_ADD_DELETE_IES
	wl_cur_etheraddr(disc->drv, disc->bsscfgIndex, &disc->addr);
	WL_PRMAC("discovery MAC address", &disc->addr);
#endif /* BCM_P2P_DISC_ADD_DELETE_IES */

#ifdef BCM_P2P_DISC_ADD_DELETE_IES
	addIes(disc);
#endif	/* BCM_P2P_DISC_ADD_DELETE_IES */

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
	return disc;

fail:
	memset(disc, 0, sizeof(*disc));
	return 0;
}

/* ----------------------------------------------------------- */

int bcm_p2p_disc_destroy(bcm_p2p_disc_t *disc)
{
	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_p2p_disc_destroy\n"));

#ifdef BCM_P2P_DISC_ADD_DELETE_IES
	deleteIes(disc);
#endif	/* BCM_P2P_DISC_ADD_DELETE_IES */

#ifdef BCM_P2P_DISC_CREATE_DISCOVERY_BSSCFG
	/* disable P2P discovery */
	wl_p2p_disc(disc->drv, FALSE);
#endif	/* BCM_P2P_DISC_CREATE_DISCOVERY_BSSCFG */

	disc->bsscfgIndex = 0;

	/* stop timers */
	tmrStop(disc->listenTimer);

	/* destroy timers */
	tmrDestroy(disc->listenTimer);

	memset(disc, 0, sizeof(*disc));
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_p2p_disc_reset(bcm_p2p_disc_t *disc)
{
	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_p2p_disc_reset\n"));
	fsm(disc, EVENT_RESET);
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_p2p_disc_start_discovery(bcm_p2p_disc_t *disc)
{
	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_p2p_disc_start_discovery\n"));
	fsm(disc, EVENT_START_DISCOVERY);
	return 1;
}

/* ----------------------------------------------------------- */

int bcm_p2p_disc_start_ext_listen(bcm_p2p_disc_t *disc,
	uint16 listenOnTimeout, uint16 listenOffTimeout)
{
	if (disc == 0) {
		WL_ERROR(("invalid parameter\n"));
		return 0;
	}
	WL_TRACE(("bcm_p2p_disc_start_ext_listen %d %d\n",
		listenOnTimeout,
		listenOffTimeout));
	disc->extListenOnTimeout = listenOnTimeout;
	disc->extListenOffTimeout = listenOffTimeout;
	fsm(disc, EVENT_START_EXT_LISTEN);
	return 1;
}

/* ----------------------------------------------------------- */

/* get bsscfg index of WFD discovery interface */
/* bsscfg index is valid only after started */
int bcm_p2p_disc_get_bsscfg_index(bcm_p2p_disc_t *disc)
{
	return disc->bsscfgIndex;
}

/* ----------------------------------------------------------- */

void bcm_p2p_disc_process_wlan_event(void * context, uint32 eventType,
	wl_event_msg_t *wlEvent, uint8 *data, uint32 length)
{
	bcm_p2p_disc_t *disc = get_gDisc();
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
				pktProbeResponseT pr;
				struct ether_addr *addr;

				if (!bcm_decode_ie_probe_response(bi, &pr)) {
					return;
				}

				/* default address */
				addr = &bi->BSSID;

				/* WFD not supported */
				if (!pr.isWfd) {
						char ssidbuf[4*32+1];
						wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);
						printf(" AP   %-20.20s   %s   %d\n", ssidbuf,
						wl_ether_etoa(addr), pr.channel);
					return;
					}

				if (pr.isWfdDeviceInfoDecoded) {
					/* use device address */
					addr = &pr.wfdDeviceInfo.deviceAddress;
					printf("WFD   %-20.20s   %s   %d\n",
						pr.wfdDeviceInfo.deviceName,
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

/* reset all variables returning to idle state */
static void idleReset(bcm_p2p_disc_t *disc)
{
	/* default P2P state */
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SCAN, 0, 0);

	/* stop timers */
	tmrStop(disc->listenTimer);

	disc->searchCount = 0;
	changeState(disc, STATE_IDLE);
}

/* 802.11 scan all channels */
static void scan(bcm_p2p_disc_t *disc)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SCAN, 0, 0);
	wl_p2p_scan(disc->drv, ++disc->sync_id, TRUE, -1, -1, -1,
		disc->searchParams->homeTime, 0, 0, disc->searchParams->flags);
	changeState(disc, STATE_SCAN);
}

/* random timeout for listen */
static uint16 randomListenTimeout(void)
{
	/* 100, 200, or 300 msec */
	return ((OSL_RAND() % 3) + 1) * 100;
}

/* listen mode */
static void listen(bcm_p2p_disc_t *disc, uint16 timeout)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_LISTEN,
		CH20MHZ_CHSPEC(disc->listenChannel), timeout);
	WL_P2PO(("listen timer started %d msec\n", timeout));
	tmrStart(disc->listenTimer, timeout, FALSE);
	changeState(disc, STATE_LISTEN);
}

/* extended listen on mode */
static void extendedListenOn(bcm_p2p_disc_t *disc, uint16 timeout)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_LISTEN,
		CH20MHZ_CHSPEC(disc->listenChannel), timeout);
	if (timeout > 0) {
		WL_P2PO(("listen timer started %d msec\n", timeout));
		tmrStart(disc->listenTimer, timeout, FALSE);
	}
	changeState(disc, STATE_EXT_LISTEN_ON);
}

/* extended listen off mode */
static void extendedListenOff(bcm_p2p_disc_t *disc, uint16 timeout)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SCAN, 0, 0);
	if (timeout > 0) {
		WL_P2PO(("listen timer started %d msec\n", timeout));
		tmrStart(disc->listenTimer, timeout, FALSE);
	}
	changeState(disc, STATE_EXT_LISTEN_OFF);
}

/* search mode */
static void search(bcm_p2p_disc_t *disc)
{
	wl_p2p_state(disc->drv, WL_P2P_DISC_ST_SEARCH, 0, 0);
	wl_p2p_scan(disc->drv, ++disc->sync_id, TRUE, -1, -1, -1,
		disc->searchParams->homeTime, disc->searchParams->numSocialChannels,
		disc->searchParams->socialChannels, 0);
	disc->searchCount++;
	changeState(disc, STATE_SEARCH);
}

/* common event processing for all states */
static void defaultEventProcessing(bcm_p2p_disc_t *disc, eventT event)
{
	switch (event) {
	case EVENT_RESET:
		WL_TRACE(("defaultEventProcessing: EVENT_RESET\n"));
		idleReset(disc);
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

static void stateIdle(bcm_p2p_disc_t *disc, eventT event)
{
	switch (event) {
	case EVENT_START_DISCOVERY:
	case EVENT_START_EXT_LISTEN:
		if (event == EVENT_START_DISCOVERY) {
#ifdef BCM_P2P_DISC_SCAN_AT_START_OF_DISCOVERY
			scan(disc);
#else
			search(disc);
#endif	/* BCM_P2P_DISC_SCAN_AT_START_OF_DISCOVERY */
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
		defaultEventProcessing(disc, event);
		break;
	}
}

static void stateScan(bcm_p2p_disc_t *disc, eventT event)
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
		defaultEventProcessing(disc, event);
		break;
	}
}

static void stateListen(bcm_p2p_disc_t *disc, eventT event)
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
		defaultEventProcessing(disc, event);
		break;
	}
}

static void stateSearch(bcm_p2p_disc_t *disc, eventT event)
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
		defaultEventProcessing(disc, event);
		break;
	}
}

static void stateExtListenOn(bcm_p2p_disc_t *disc, eventT event)
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
		defaultEventProcessing(disc, event);
		break;
	}
}

static void stateExtListenOff(bcm_p2p_disc_t *disc, eventT event)
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
		defaultEventProcessing(disc, event);
		break;
	}
}

typedef void (*state_func_t)(bcm_p2p_disc_t *disc, eventT event);
static state_func_t table[STATE_LAST] = {
	stateIdle,
	stateScan,
	stateListen,
	stateSearch,
	stateExtListenOn,
	stateExtListenOff
};

/* WFD discovery finite state machine */
static void fsm(bcm_p2p_disc_t *disc, eventT event)
{
	WL_P2PO(("--------------------------------------------------------\n"));

	if (disc->state < STATE_LAST && event < EVENT_LAST) {
		WL_P2PO(("current state=%s event=%s\n",
			state_str[disc->state], event_str[event]));

		/* process event based on current state */
		(table[disc->state])(disc, event);

		/* transition to next state */
		if (disc->state != disc->nextState) {
			WL_P2PO(("state change %s -> %s\n",
				state_str[disc->state], state_str[disc->nextState]));
			disc->state = disc->nextState;
		}
	}
	else {
		WL_P2PO(("invalid state=%d event=%d\n", disc->state, event));
	}

	WL_P2PO(("--------------------------------------------------------\n"));
}
