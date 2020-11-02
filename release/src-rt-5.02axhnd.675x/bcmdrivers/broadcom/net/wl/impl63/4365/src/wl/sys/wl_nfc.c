/*
 * NFC for Security WiFi
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
 * $Id: wl_nfc.c 43960 2013-11-26 15:57:35 $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [SecureWiFiThroughNFC]
 */

#include <wlc_cfg.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_nfc.h>
#include <bcm_nfcif.h>

/* wlc_pub_t struct access macros */
#define WLCUNIT(nfci)	((nfci)->wlc->pub->unit)
#define WLCOSH(nfci)	((nfci)->wlc->osh)

/* Self loopback testing without an involvment from UART level and below */

/* Secure WiFi thru NFC */
struct wl_nfc_info {
	wlc_info_t	*wlc;	/* Pointer back to wlc structure */
	struct wlc_if	*wlcif;

	/*
	 * Move those mac addresses into the structure for future ROMing.
	 * But please get ready to change acccording to new design.
	 */
	struct ether_addr local_mac;
	struct ether_addr remote_mac;
};

static int wl_nfc_parse(wl_nfc_info_t *nfci, void *sdu, bool sentby_host);

/*
 * Initialize NFC private context.
 * Returns a pointer to the NFC private context, NULL on failure.
 */
wl_nfc_info_t *
BCMATTACHFN(wl_nfc_attach)(wlc_info_t *wlc)
{
	wl_nfc_info_t *nfci;

	/* allocate NFC private info struct */
	nfci = MALLOCZ(wlc->osh, sizeof(wl_nfc_info_t));
	if (!nfci) {
		WL_ERROR(("wl%d: wl_nfc_attach: MALLOCZ failed; total mallocs %d bytes\n",
		          wlc->pub->unit, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init NFC private info struct */
	nfci->wlc = wlc;

	/*
	 * Ideally init here, and then enable the function when all the parameters are ready.
	 * This needs to add an extra interface between this module and UART module.
	 *
	 * For phase one demo, initialization is put in the first received package handling.
	 */

	wlc->pub->_nfc = TRUE;

	return nfci;
}

void
BCMATTACHFN(wl_nfc_detach)(wl_nfc_info_t *nfci)
{
	WL_INFORM(("wl%d: nfc_detach()\n", WLCUNIT(nfci)));

	if (!nfci)
		return;

	nfci->wlc->pub->_nfc = FALSE;

	MFREE(WLCOSH(nfci), nfci, sizeof(wl_nfc_info_t));
}

wl_nfc_info_t *
wl_nfc_alloc_ifnfci(wl_nfc_info_t *nfci_p, wlc_if_t *wlcif)
{
	wl_nfc_info_t *nfci;
	wlc_info_t *wlc = nfci_p->wlc;

	/* allocate NFC private info struct */
	nfci = MALLOCZ(wlc->osh, sizeof(wl_nfc_info_t));
	if (!nfci) {
		WL_ERROR(("wl%d: %s: MALLOCZ failed; total mallocs %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init nfc private info struct */
	nfci->wlc   = wlc;
	nfci->wlcif = wlcif;

	return nfci;
}

void
wl_nfc_free_ifnfci(wl_nfc_info_t *nfci)
{
	if (nfci != NULL) {
		MFREE(WLCOSH(nfci), nfci, sizeof(wl_nfc_info_t));
	}
}

/*
 * Process NFC frames in receive direction.
 *
 * Return value:
 *	-1	Packet parsing error
 *   0	OK
 */
int
wl_nfc_recv_proc(wl_nfc_info_t *nfci, void *sdu)
{
	int ret;

	WL_INFORM(("wl%d: wl_nfc_recv_proc()\n", WLCUNIT(nfci)));

	/* Parse NFC packet */
	ret = wl_nfc_parse(nfci, sdu, FALSE);

	return ret;
}

static int
wl_nfc_parse(wl_nfc_info_t *nfci, void *sdu, bool sentby_host)
{
	char buf[NFC_BUF_SIZE];
	int      ret = -1;
	uint8 *frame = PKTDATA(WLCOSH(nfci), sdu);
	int   length = PKTLEN(WLCOSH(nfci), sdu);
	uint8 *   pt = NULL;
	struct ether_header *eth = NULL;
	static bool    first_pkt = TRUE;

	if (length <= ETHER_HDR_LEN) {
		return ret;
	}

	if (ntoh16_ua((const void *)(frame + ETHER_TYPE_OFFSET)) >= ETHER_TYPE_MIN) {
		/* Frame is Ethernet II */
		eth = (struct ether_header *)frame;
		pt  = frame + ETHER_HDR_LEN;
	}

	if ((ntoh16(eth->ether_type) != ETHER_TYPE_NFC_SECURE_WIFI)) {
		return ret;
	}

	if (first_pkt) {
		first_pkt = FALSE;

		/* Only when the interface is fully ready, register the callback */
		bcm_nfcif_init(wl_recv_from_nfc_uart, nfci);

		bcopy(eth->ether_dhost, nfci->local_mac.octet,  sizeof(struct ether_addr));
		bcopy(eth->ether_shost, nfci->remote_mac.octet, sizeof(struct ether_addr));
	}

	length -= sizeof(struct ether_header);
	if (length < NFC_BUF_SIZE) {
		bcopy(pt, buf, length);
	}
	else {
		return ret;
	}

#ifndef WL_NFC_SELF_LOOPBACK
	ret = bcm_nfcif_send_to_nfc(buf, (uint16)length, SLIP_HDR_WIFI_PKT);
	return ret;
#else
	wl_recv_from_nfc_uart(nfci, buf, length);
	return 0;
#endif // endif
}

/*
 * Process NFC frames in transmit direction.
 * Not applied yet. Reserve for future.
 */
int
wl_nfc_send_proc(wl_nfc_info_t *nfci, void *sdu)
{
	wl_nfc_parse(nfci, sdu, TRUE);
	return 0;
}

void
wl_recv_from_nfc_uart(void *p, const char *s, uint16 len)
{
	void  *pkt;
	uint8 *frame;
	uint16 nfc_pktlen = ETHER_HDR_LEN + len;
	struct ether_header *nfc_eth_hdr = NULL;
	wl_nfc_info_t *nfci = (wl_nfc_info_t *)p;

	WL_INFORM(("wl%d: %s()\n", WLCUNIT(nfci), __FUNCTION__));

	if (!(pkt = PKTGET(WLCOSH(nfci), nfc_pktlen, TRUE))) {
		WL_ERROR(("wl%d: %s: alloc failed; dropped\n",
		          WLCUNIT(nfci), __FUNCTION__));
		WLCNTINCR(nfci->wlc->pub->_cnt->rxnobuf);
		return;
	}

	frame = PKTDATA(WLCOSH(nfci), pkt);
	bzero(frame, nfc_pktlen);

	nfc_eth_hdr = (struct ether_header *)frame;
	frame += ETHER_HDR_LEN;

	bcopy(nfci->remote_mac.octet, nfc_eth_hdr->ether_dhost, sizeof(struct ether_addr));
	bcopy(nfci->local_mac.octet,  nfc_eth_hdr->ether_shost, sizeof(struct ether_addr));

	nfc_eth_hdr->ether_type = hton16(ETHER_TYPE_NFC_SECURE_WIFI);
	bcopy(s, frame, len);

	wlc_sendpkt(nfci->wlc, pkt, nfci->wlcif);
	return;
}
