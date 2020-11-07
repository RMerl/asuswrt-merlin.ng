/*
 * Application-specific portion of EAPD
 * (connectivity event logger)
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
 * $Id$
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
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

static void cevent_hexdump_ascii(const char *title, const unsigned char *buf, unsigned int len);
static uint64 cevent_get_curr_time();
static void cevent_encap(bcm_event_t *be, uint16 be_length, void *pData, int pData_length,
		char *fromIfname, uint16 ether_type, bool frm_drv, uint32 appmask);
static int cevent_process_brcmevent(eapd_wksp_t *nwksp, uint8 *pData, int pData_length);
static int cevent_hexstrtobitvec(const char *hexstr, uchar *bitvec, int veclen);
static void cevent_dumpmask(uchar *bitvec);

static int
cevent_hexstrtobitvec(const char *hexstr, uchar *bitvec, int veclen)
{
	unsigned char value = 0;
	int nibble;             /* index of current hex-format nibble to process */
	int even;               /* 1 if even number of nibbles, 0 if odd number */
	int i = 0;

	if (hexstr[0] == '0' && hexstr[1] == 'x') {
		hexstr += 2;
	}

	memset(bitvec, '\0', veclen);
	nibble = strlen(hexstr);

	if (!nibble) {
		return BCME_ERROR;
	}

	even = ((nibble % 2) == 0);

	/* convert from right to left (lsb is rightmost byte) */
	--nibble;
	while (nibble >= 0 && i < veclen && (isxdigit((int)hexstr[nibble]) &&
		(value = isdigit((int)hexstr[nibble]) ? hexstr[nibble]-'0' :
		(islower((int)hexstr[nibble]) ? toupper((int)hexstr[nibble]) :
		hexstr[nibble])-'A'+10) < 16)) {
		if (even == ((nibble+1) % 2)) {
			bitvec[i] += value*16;
			++i;
		} else {
			bitvec[i] = value;
		}
		--nibble;
	}

	return ((nibble == -1 && i <= veclen) ? BCME_OK : BCME_ERROR);
}

static void cevent_hexdump_ascii(const char *title, const unsigned char *buf,
	unsigned int len)
{
	int i, llen;
	const unsigned char *pos = buf;
	const int line_len = 16;

	EAPD_INFO_LEAN("%s - (data len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		EAPD_INFO_LEAN("    ");
		for (i = 0; i < llen; i++)
			EAPD_INFO_LEAN(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			EAPD_INFO_LEAN("   ");
		EAPD_INFO_LEAN("   ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i]))
				EAPD_INFO_LEAN("%c", pos[i]);
			else
				EAPD_INFO_LEAN("*");
		}
		for (i = llen; i < line_len; i++)
			EAPD_INFO_LEAN(" ");
		EAPD_INFO_LEAN("\n");
		pos += llen;
		len -= llen;
	}
}

static uint64
cevent_get_curr_time()
{
	struct timespec tp;
	uint64 curr_ts = 0;

	if (clock_gettime(CLOCK_REALTIME, &tp) < 0) {
		EAPD_ERROR("clock_gettime failed\n");
		return BCME_ERROR;
	}

	curr_ts = (uint64)tp.tv_sec*1000LL + (uint64)tp.tv_nsec/1000000LL;

	return curr_ts;
}

/* Two level encapsulation of pData which is received from either driver.
 * or applications. Currently encapsulating only standard EAPOL/PREAUTH frames received
 * from applications(NAS, WPS) or from driver (after decapsulating BRCM event as std EAPOL in EAPD).
 * First encapsulate in wl_cevent_t and then as BRCM event (bcm_event_t).
 * The dual encapsulated pData is then sent to cevent_app for logging.
 */
static
void cevent_encap(bcm_event_t *be, uint16  be_length,  void *pData, int pData_length,
		char *fromIfname, uint16 ether_type, bool frm_drv, uint32 appmask)
{
	uint64 time_in_ms;	/* in milliseconds */
	wl_cevent_t *cevent = NULL;
	struct ether_header *eth = NULL;

	if (!be || !pData || !fromIfname) {
		EAPD_ERROR("NULL be/pData/Ifname. Return\n");
		return;
	}

	if ((time_in_ms = cevent_get_curr_time()) == BCME_ERROR) {
		return;
	}

	if (pData_length < ETHER_HDR_LEN) {
		EAPD_ERROR("pData_length %d\n", pData_length);
		return;
	}
	/* eth hdr needed to fetch src MAC addr; to be filled later in evnt.addr */
	eth = (struct ether_header *)pData;

	if (be_length < (pData_length + sizeof(wl_cevent_t) + sizeof(bcm_event_t))) {
		EAPD_ERROR("buffer %u bytes is too short to encapsulate. Return\n", be_length);
		return;
	}

	/* point cevent to end of bcm_event. */
	cevent = (wl_cevent_t *)(be + 1);
	memset(cevent, 0, sizeof(*cevent));

	/* Update wl_cevent_t */
	cevent->version = WLC_E_CEVENT_VER;
	cevent->length = sizeof(*cevent) + pData_length;
					 /* from driver */
	cevent->type = ((frm_drv == TRUE) ? CEVENT_TYPE_D2A : CEVENT_TYPE_A2D);
	cevent->subtype |= appmask;
	if (ether_type != 0) CE_SET_ETH_HDR_PRESENT((cevent->flags));
	cevent->flags |= (frm_drv == TRUE) ? CEVENT_FRAME_DIR_RX : CEVENT_FRAME_DIR_TX;
	cevent->data_offset = sizeof(*cevent);
	cevent->timestamp = time_in_ms;
	/* Encapsulate EAPOL/PREAUTH frame inside wl_cevent_t */
	if ((pData_length > 0) && (pData != NULL)) {
		memcpy(&(cevent->data), pData, pData_length);
	}

	/* Update wl_event_msg_t  */
	be->event.version = hton16(BCM_EVENT_MSG_VERSION);
	be->event.flags = 0;
	be->event.event_type = hton32(WLC_E_CEVENT);
	be->event.status = hton32(WLC_E_STATUS_SUCCESS);
	be->event.reason = 0;
	be->event.auth_type = 0;
	be->event.datalen = hton32(cevent->length);
	if (frm_drv) {
		bcopy(&(eth->ether_shost), &(be->event.addr), ETHER_ADDR_LEN);
	} else {
		bcopy(&(eth->ether_dhost), &(be->event.addr), ETHER_ADDR_LEN);
	}
	memcpy(&(be->event.ifname), fromIfname, BCM_MSG_IFNAME_MAX);
	be->event.ifidx = 0;
	be->event.flags |= WLC_EVENT_MSG_UNKIF;
	be->event.bsscfgidx = 0;
	be->event.flags |= WLC_EVENT_MSG_UNKBSS;

	/* Update bcmeth_hdr_t - BCM Vendor specific header... */
	be->bcm_hdr.subtype = hton16(BCMILCP_SUBTYPE_VENDOR_LONG);
	/* Incorrect assignement of bcm_hdr.length! Picked it from driver code */
	be->bcm_hdr.length = hton16(BCMILCP_BCM_SUBTYPEHDR_MINLENGTH +
			BCM_MSG_LEN + (uint16)pData_length);
	be->bcm_hdr.version = hton16(BCMILCP_BCM_SUBTYPEHDR_VERSION);
	bcopy(BRCM_OUI, &(be->bcm_hdr.oui[0]), DOT11_OUI_LEN);

	/* Update eth hdr field of bcmevent */
	memcpy(&(be->eth), (struct ether_header *)pData, ETHER_HDR_LEN);
	be->eth.ether_type = hton16(ETHER_TYPE_BRCM);
}

void
cevent_copy_eapol_and_forward(eapd_wksp_t *nwksp, char *ifname,
		const eapol_header_t *eapol, const eap_header_t *eap, uint32 app)
{
	unsigned short pkt_length = 0;	/* eapol pkt length (eth + eapol) */
	eapd_cb_t *cb = NULL;
	eapol_header_t *eapol_ptr = NULL;

	if ((nwksp->cevent.cb) != NULL) {
		cb = nwksp->cevent.cb;
	}
					/* 18 bytes of eapol header + payload length */
	eapol_ptr = (eapol_header_t *) malloc(EAPOL_HEADER_LEN + ntohs(eapol->length));

	if (eapol_ptr == NULL) {
		EAPD_ERROR("malloc() failed.\n");
		return;
	}

	memcpy(eapol_ptr, eapol, EAPOL_HEADER_LEN); /* eth hdr (14 bytes) + eapol hdr(4 bytes) */

	if (eapol->type == EAP_PACKET) {
		memcpy(&eapol_ptr->body, eap, ntohs(eapol->length));
	}

	pkt_length = EAPOL_HEADER_LEN + ntohs(eapol->length);

	cevent_generic_hub(nwksp, ifname, cb, (uint8 *)eapol_ptr, pkt_length, FALSE, app);

	free(eapol_ptr);
}

/**
 * call this to forward events to cevent specific app socket(s)
 */
int
cevent_to_app(eapd_wksp_t *nwksp, bcm_event_t* be, uint16 be_length, char *fromIfname)
{
	int event_type;
	eapd_cb_t *cb = NULL;
	eapd_cevent_t *eapd_cevent = NULL;
	wl_event_msg_t *evt_msg = NULL;

	if (!nwksp || !be || !fromIfname) {
		EAPD_ERROR("NULL nwksp/be/Ifname. Return\n");
		return BCME_ERROR;
	}

	if (be_length < ETHER_HDR_LEN) {
		EAPD_ERROR("be_length is %u bytes. Less then eth hdr. Return\n", be_length);
		return BCME_BADLEN;
	}

	/* This function must be called for ether_type equal to ETHER_TYPE_BRCM only */
	if (ntohs(be->eth.ether_type) != ETHER_TYPE_BRCM) {
		EAPD_ERROR("Not an ETHER_TYPE_BRCM event/frame. Return\n");
		return BCME_ERROR;
	}

	if (be_length < sizeof(bcm_event_t)) {
		EAPD_ERROR("be_length is %u bytes. Return\n", be_length);
		return BCME_BADLEN;
	}

	evt_msg = &(be->event);
	event_type = ntohl(evt_msg->event_type);

	eapd_cevent = &nwksp->cevent;
	cb = eapd_cevent->cb;

	while (cb) {
		if (isset(eapd_cevent->bitvec, event_type)) {
			/* Send to cevent_app. use cb->ifname. */
			cevent_app_sendup(nwksp, be, be_length, cb->ifname);
			break;
		}
		cb = cb->next;
	}
	return BCME_OK;
}

/* Process fames of ether_type ETHER_TYPE_BRCM which can be coming from driver or applications.
 * Driver events are E_AUTH_IND, E_ASSOC_IND, E_CEVENT etc. Applications only send
 * events of type WLC_E_CEVENT.
 */
static int
cevent_process_brcmevent(eapd_wksp_t *nwksp, uint8 *pData, int pData_length)
{
	bcm_event_t *be = NULL;
	wl_cevent_t *ce = NULL;
	eapd_cevent_t *eapd_cevent = NULL;
	uint32 be_event_type;

	if (pData_length < sizeof(bcm_event_t)) {
		EAPD_ERROR("length too short to hold BRCM event\n");
		return BCME_BADLEN;
	}

	be = (bcm_event_t *)pData;
	be_event_type = ntohl(be->event.event_type);

	eapd_cevent = &nwksp->cevent;
	if (!isset(eapd_cevent->bitvec, be_event_type)) {
		EAPD_INFO("Not registered for event type %u\n", be_event_type);
		return BCME_ERROR;
	}

	cevent_hexdump_ascii(__FUNCTION__, (unsigned char *)pData,
			MIN(pData_length, CE_PKT_DUMP_LEN));
	/* if cevent, ensure to override dongle timestamp with host timestamp.
	 */
	if (be_event_type == WLC_E_CEVENT) {
		ce = (wl_cevent_t *)(be + 1);
		ce->timestamp = cevent_get_curr_time();
		if (ce->timestamp == BCME_ERROR) {
			EAPD_ERROR("ce->timestamp: %llu\n", ce->timestamp);
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}
/**
 * Messages (events or frames) sent by driver or applications (NAS, WPS ...) are recevied in EAPD
 * which are tapped by this function. From EAPD, this function taps
 * 1. BRCM events from dirver such as of event type E_AUTH_IND, E_ASSOC_IND, E_EAPOL_MSG, E_CEVENT.
 * 2. BRCM event of event type WLC_E_CEVENT from applications .
 * 3. EAPOL frames from applications that reach eapd to be sent down to driver.
 * 4. EAPOL frames obtained after decapsulating BRCM events (of event type E_EAPOL_MSG).These events
 *    are from driver and are decasulated in EAPD eapol dispatcher.
 *
 * The EAPOL frames from driver and applications are then encapsulated in CEVENT (wl_cevent_t) and
 * BRCM event (bcm_event_t) and further forwarded to cevent application for logging on console
 * /RAM/flash/disk/cevent-socket depending on configuration.
 */
void
cevent_generic_hub(eapd_wksp_t *nwksp, char *fromIfname, eapd_cb_t *cb, uint8 *pData,
	int pData_length, bool frm_drv, uint32 appmask)
{
	struct ether_header *eth = NULL;
	bcm_event_t *bcm_event = NULL;
	uint16 bcm_event_length;
	uint16 ether_type;

	if (!eapd_ceventd_enable) {
		EAPD_INFO("Entered hub unexpected.Return\n");
		return;
	}

	if (!nwksp || !fromIfname || !cb || !pData) {
		EAPD_ERROR("NULL nwksp/Ifname/cb/pData. Return\n");
		return;
	}

	if (pData_length < ETHER_HDR_LEN) {
		EAPD_ERROR("pData_length %d\n", pData_length);
		return;
	}

	/* tapped events and frames from driver and application contain ethernet hdr.
	 * Fetch ether type from it.
	 */
	eth = (struct ether_header *) pData;
	ether_type = ntohs(eth->ether_type);

	EAPD_INFO("ether_type: 0x%04x, pData_len: %d\n", ether_type, pData_length);

	switch (ether_type) {
		case ETHER_TYPE_BRCM:	/* brcmevent from driver/app (NAS, WPS etc). */
			if (cevent_process_brcmevent(nwksp, pData, pData_length) != BCME_OK) {
				EAPD_INFO("BRCM event processing fail. Return\n");
				return;
			}
			cevent_to_app(nwksp, (bcm_event_t *)pData,
					(uint16)pData_length, fromIfname);
			break;
		case ETHER_TYPE_802_1X:
		case ETHER_TYPE_802_1X_PREAUTH:
			/* encapsulate received EAPOL/PREAUTH frame in cevent and BRCM event */
			bcm_event_length = sizeof(*bcm_event) + sizeof(wl_cevent_t) + pData_length;

			if ((bcm_event = malloc(bcm_event_length)) == NULL) {
				EAPD_ERROR("%s: malloc() failed\n", __FUNCTION__);
				return;
			}
			memset(bcm_event, 0, bcm_event_length);

			cevent_encap(bcm_event, bcm_event_length, (eapol_header_t *)pData,
					pData_length, fromIfname, ether_type, frm_drv, appmask);
			cevent_hexdump_ascii(__FUNCTION__, (unsigned char *)bcm_event,
					MIN(bcm_event_length, CE_PKT_DUMP_LEN));
			/* send (cevent + BRCM event) encapsulated events/frames to cevent_app */
			cevent_to_app(nwksp, bcm_event, bcm_event_length, fromIfname);
			free(bcm_event);
			break;
		default:
			EAPD_INFO("%s: Unknown type\n", __FUNCTION__);
			cevent_hexdump_ascii(__FUNCTION__, pData,
					MIN(pData_length, CE_PKT_DUMP_LEN));
			break;
	}

}

/*
 * Receive events from cevent application socket,
 * This could be used as control signal for eapd cevent handlers.
 * Do NOT forward events received here to hub for logging again.
 */
void
cevent_app_recv_handler(eapd_wksp_t *nwksp, char *fromIfname, eapd_cb_t *cb, uint8 *pData,
	int *pLen)
{
	cevent_hexdump_ascii(__FUNCTION__, pData, MIN(*pLen, CE_PKT_DUMP_LEN));
}

static void
cevent_dumpmask(uchar *bitvec)
{
	int i, j, flag = 1;

	for (i = 0; i < EAPD_WL_EVENTING_MASK_LEN; i++) {
		for (j = 0; j < 8; j++) {
			if (isset(&bitvec[i], j)) {
				if (flag) {
					EAPD_PRINT("cevent_bitvec [%d", (i*8+j));
					flag = 0;
				} else {
					EAPD_PRINT(" %d", (i*8+j));
				}
			}
		}
	}

	EAPD_PRINT("]\n");
}

void
cevent_app_set_eventmask(eapd_app_t *app)
{
	int ret = BCME_OK;
	uchar *bitvec = NULL;
	const char *nv_ce_enabled = NULL;
	const char *nv_ce_eventmask = NULL;

	if (app == NULL) {
		EAPD_ERROR("nwskp->cevent is NULL\n");
		return;
	}

	nv_ce_enabled = nvram_safe_get("ceventd_enable");

	if (nv_ce_enabled && nv_ce_enabled[0] != '\0') {
		EAPD_INFO("ceventd is enabled\n");

		bitvec = app->bitvec;

		memset(bitvec, 0, EAPD_WL_EVENTING_MASK_LEN);

		nv_ce_eventmask = nvram_safe_get("ceventd_eventmask");

		if (nv_ce_eventmask && nv_ce_eventmask[0] != '\0') {
			EAPD_INFO("ceventd_eventmask = %s\n", nv_ce_eventmask);

			ret = cevent_hexstrtobitvec(nv_ce_eventmask, bitvec,
					EAPD_WL_EVENTING_MASK_LEN);

			if (ret == BCME_OK) {
				cevent_dumpmask(bitvec);
			}

		}
	} else {
		EAPD_ERROR("ceventd_enable = %s\n", nv_ce_enabled);
	}

	return;
}

int
cevent_app_init(eapd_wksp_t *nwksp)
{
	int reuse = 1;
	eapd_cevent_t *cevent;
	eapd_cb_t *cb;
	struct sockaddr_in addr;

	if (nwksp == NULL)
		return -1;

	cevent = &nwksp->cevent;
	cevent->appSocket = -1;

	cb = cevent->cb;
	if (cb == NULL) {
		EAPD_INFO("No any cevent application need to run.\n");
		return 0;
	}

	while (cb) {
		EAPD_INFO("cevent: init brcm interface %s \n", cb->ifname);
		cb->brcmSocket = eapd_add_brcm(nwksp, cb->ifname);
		if (!cb->brcmSocket)
			return -1;
		/* set this brcmSocket have CEVENT capability */
		cb->brcmSocket->flag |= EAPD_CAP_CEVENT;

		cb = cb->next;
	}

	/* appSocket for cevent */
	cevent->appSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (cevent->appSocket < 0) {
		EAPD_ERROR("UDP Open failed.\n");
		return -1;
	}
	if (setsockopt(cevent->appSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse,
		sizeof(reuse)) < 0) {
		EAPD_ERROR("UDP setsockopt failed.\n");
		close(cevent->appSocket);
		cevent->appSocket = -1;
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = EAPD_UDP_SIN_ADDR(nwksp);
	addr.sin_port = htons(EAPD_WKSP_CEVENT_UDP_RPORT);
	if (bind(cevent->appSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		EAPD_ERROR("UDP Bind failed, close cevent appSocket %d\n", cevent->appSocket);
		close(cevent->appSocket);
		cevent->appSocket = -1;
		return -1;
	}
	EAPD_INFO("CEVENT appSocket %d opened\n", cevent->appSocket);

	return 0;
}

int
cevent_app_deinit(eapd_wksp_t *nwksp)
{
	eapd_cevent_t *cevent;
	eapd_cb_t *cb, *tmp_cb;

	if (nwksp == NULL) {
		EAPD_ERROR("Wrong argument...\n");
		return -1;
	}

	cevent = &nwksp->cevent;
	cb = cevent->cb;
	while (cb) {
		/* close  brcm drvSocket */
		if (cb->brcmSocket) {
			EAPD_INFO("close cevent brcmSocket %d\n", cb->brcmSocket->drvSocket);
			eapd_del_brcm(nwksp, cb->brcmSocket);
		}

		tmp_cb = cb;
		cb = cb->next;
		free(tmp_cb);
	}

	/* close  appSocke */
	if (cevent->appSocket >= 0) {
		EAPD_INFO("close cevent appSocket %d\n", cevent->appSocket);
		close(cevent->appSocket);
		cevent->appSocket = -1;
	}

	return 0;
}

int
cevent_app_sendup(eapd_wksp_t *nwksp, bcm_event_t *be, uint16 be_length, char *from)
{
	eapd_cevent_t *cevent = NULL;

	if (!nwksp || !be || !from) {
		EAPD_ERROR("NULL nwksp/be/Ifname. Return\n");
		return BCME_ERROR;
	}

	if (be_length < sizeof(bcm_event_t)) {
		EAPD_ERROR("be_length %u bytes. Return\n", be_length);
		return BCME_BADLEN;
	}

	cevent = &nwksp->cevent;

	if (cevent->appSocket >= 0) {
		/* send to cevent */
		int sentBytes = 0;
		struct sockaddr_in to;

		to.sin_addr.s_addr = inet_addr(EAPD_WKSP_UDP_ADDR);
		to.sin_family = AF_INET;
		to.sin_port = htons(EAPD_WKSP_CEVENT_UDP_SPORT);

		sentBytes = sendto(cevent->appSocket, be, be_length, 0,
			(struct sockaddr *)&to, sizeof(struct sockaddr_in));

		if (sentBytes != be_length) {
			EAPD_ERROR("UDP send failed; sentBytes = %d\n", sentBytes);
		} else {
			EAPD_INFO("Send %d bytes to cevent_app\n", sentBytes);
		}
	} else {
		EAPD_ERROR("cevent appSocket not created\n");
	}
	return 0;
}

#if EAPD_WKSP_AUTO_CONFIG
int
cevent_app_enabled(char *name)
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
		EAPD_INFO("CEVENT:ignored interface %s. radio disabled\n", os_name);
		return 0;
	}

	/* ignore if BSS is disabled */
	eapd_safe_get_conf(value, sizeof(value), strcat_r(prefix, "bss_enabled", comb));
	if (atoi(value) == 0) {
		EAPD_INFO("CEVENT: ignored interface %s, %s is disabled \n", os_name, comb);
		return 0;
	}

	/* if come to here return enabled */
	return 1;
}
#endif /* EAPD_WKSP_AUTO_CONFIG */

/* receive event from driver */
int
cevent_app_handle_event(eapd_wksp_t *nwksp, uint8 *pData, int len,
		char *fromIfname, bool frm_drv, uint32 appmask)
{
	eapd_cb_t *cb = NULL;

	if (nwksp == NULL) {
		EAPD_ERROR("%s: nkwsp is NULL\n", __FUNCTION__);
		return BCME_ERROR;
	}
#ifdef BCM_CEVENT
	if ((nwksp->cevent.cb) == NULL)
	{
		EAPD_ERROR("%s: cevent cb is NULL\n", __FUNCTION__);
		return BCME_ERROR;
	}

	cb = nwksp->cevent.cb;

	cevent_generic_hub(nwksp, fromIfname, cb, pData, len, frm_drv, appmask);
#endif /* BCM_CEVENT */

	return BCME_OK;
}

/*
 * Application-specific portion of EAPD
 * (connectivity event logger)
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
 * $Id$
 */
