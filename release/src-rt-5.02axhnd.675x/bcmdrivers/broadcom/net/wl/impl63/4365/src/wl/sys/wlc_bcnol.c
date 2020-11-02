/*
 * Broadcom 802.11 Beacon offload Driver
 *
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
 * $Id: wlc_bcnol.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * This feature requirement is primarily driven by Win8 which places a limit of 3 on the number of
 * interrupts that the device can generate per second when sitting in idle/associated mode. The
 * primary aim of this Win8 requirement was to reduce the number of interrupts to the host CPU so
 * that the CPU could go to deeper sleep state for longer periods of time hence reducing system
 * power. So, our primary goal was to retrofit our currently shipping hardware (PCIe chips) with the
 * given ucode constraints, to reduce these interrupts below the Microsoft mandated threshold.
 */

/**
 * @file
 * @brief
 * XXX  Twiki: [BeaconOffload]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <proto/802.11.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_phy_ac.h>
#include <proto/802.3.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/bcmarp.h>
#include <bcm_ol_msg.h>
#include <wlc_key.h>
#include <wlc_bcnol.h>
#include <wlc_dngl_ol.h>
#include <wlc_wowlol.h>
#include <wlc_eventlog_ol.h>
#include <wlc_macol.h>

#define SET_IE(a, b) (a[b/8] |= (1 << (b%8)))
#define GET_IE(a, id) (a[id/8] & (1 << (id%8)))
#define RESET_IE(a, id) a[id/8] &= (~(1 << (id%8)))

#define NULL_OUI	"\x00\x00\x00"
#define OUI_LEN		3
#define SET_VNDRIE(a)	(a & 0xffffff)

extern uint32 rxoe_debug_level;

/* bcn offload private info structure */

struct wlc_dngl_ol_bcn_info {

	wlc_dngl_ol_info_t *wlc_dngl_ol;

	/* Deferral count to inform ucode */
	uint32   defcnt;

	uint32	tsf_h;	/* TSF high bits (to detect retrograde TSF) */
	uint32	tsf_l;	/* TSF low bits (to detect retrograde TSF) */

	/* Received message from Host to enable the bcn offload */
	bool beacon_ol_enable;
	/* send beacon to host, when beacon offload is enabled  */
	bool sendBcntoHost;
	uint32 		time_since_bcn;
	uint32 		bcn_loss_timeout;
	uint32		wowl_flags;	/* WL_WOWL_Xxx flags */
	olmsg_bcn_enable *pbcn_ol_msg;
	uint32		bcn_ol_msg_len;
	bool 	ie_ol_enable;
	uint8   iemask[IEMASK_SZ];
	vndriemask_info	vndriemask[MAX_VNDR_IE];
};

static int wlc_dngl_ol_bcn_tim(wlc_dngl_ol_bcn_info_t *bcn_ol, void* bcnframe, uint plen);
static int wlc_dngl_ol_bcn_compare(wlc_dngl_ol_bcn_info_t *bcn_ol, int ie_id,
	void *bcnframe, uint plen);
static bool
wlc_dngl_ol_bcn_vndrie(wlc_dngl_ol_bcn_info_t *bcn_ol, void *bcnframe, uint plen);
static bcm_tlv_t *
wlc_dngl_ol_find_vndrie(vndriemask_info *ouidata, void *iedata, uint plen);
static uint16
wlc_dngl_ol_find_csa_ie(wlc_dngl_ol_bcn_info_t *bcn_ol, void *bcnframe, uint plen);
#ifdef BCMDBG
static void wlc_dngl_ol_bcn_enable_dump(wlc_dngl_ol_bcn_info_t *bcn_ol, olmsg_bcn_enable *pbcnmsg);
#endif // endif

wlc_dngl_ol_bcn_info_t *
wlc_dngl_ol_bcn_attach(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	wlc_dngl_ol_bcn_info_t *bcn_ol;

	bcn_ol = (wlc_dngl_ol_bcn_info_t *)MALLOC(wlc_dngl_ol->osh, sizeof(wlc_dngl_ol_bcn_info_t));
	if (!bcn_ol) {
		WL_ERROR((" bcn_ol malloc failed: %s\n", __FUNCTION__));
		return NULL;
	}

	bzero(bcn_ol, sizeof(wlc_dngl_ol_bcn_info_t));
	bcn_ol->wlc_dngl_ol = wlc_dngl_ol;
	bcn_ol->beacon_ol_enable = FALSE;

	return bcn_ol;
}

static void
wlc_dngl_ol_bcn_event_wowl_start(wlc_dngl_ol_bcn_info_t *bcn_ol, wowl_cfg_t *wowl_cfg)
{
	if (wowl_cfg->wowl_enabled == TRUE) {
		bcn_ol->wowl_flags = wowl_cfg->wowl_flags;
		bcn_ol->bcn_loss_timeout = wowl_cfg->bcn_loss;

		WL_ERROR(("setting bcn_loss_timeout to %d:\n",
			wowl_cfg->bcn_loss));
	}
}

void
wlc_dngl_ol_bcn_event_handler(
	wlc_dngl_ol_info_t	*wlc_dngl_ol,
	uint32			event,
	void			*event_data)
{
	wlc_dngl_ol_bcn_info_t	*bcn_ol = wlc_dngl_ol->bcn_ol;

	switch (event) {
		case BCM_OL_E_WOWL_START:
			wlc_dngl_ol_bcn_event_wowl_start(bcn_ol, event_data);
			break;

		case BCM_OL_E_BCN_LOSS:
		case BCM_OL_E_DEAUTH:
		case BCM_OL_E_DISASSOC:
			/* set LINKDOWN flag in case system is wake by user */
			ppcie_shared->wowl_host_info.wake_reason |= WL_WOWL_LINKDOWN;
			WL_ERROR(("%s:Set LinkDown status(0x%04x)\n", __FUNCTION__,
				ppcie_shared->wowl_host_info.wake_reason));
			/* allow to fall through */
		case BCM_OL_E_PME_ASSERTED:
			WL_ERROR(("%s : Disabling beacon offloads\n", __FUNCTION__));
			bcn_ol->beacon_ol_enable = FALSE;
			break;
	}
}

void
wlc_dngl_ol_bcn_clear(wlc_dngl_ol_bcn_info_t *bcn_ol, wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	bzero(bcn_ol, sizeof(wlc_dngl_ol_bcn_info_t));
	bcn_ol->wlc_dngl_ol = wlc_dngl_ol;
}

bool
wlc_dngl_ol_bcn_delete(wlc_dngl_ol_bcn_info_t *bcn_ol)
{
	if (bcn_ol->beacon_ol_enable && bcn_ol->pbcn_ol_msg->frame_del)
		return TRUE;

	return FALSE;
}

void
wlc_dngl_ol_bcn_watchdog(wlc_dngl_ol_bcn_info_t *bcn_ol)
{

	wlc_dngl_ol_info_t *wlc_dngl_ol = bcn_ol->wlc_dngl_ol;

	if (!bcn_ol->beacon_ol_enable)
		return;

	if (!bcn_ol->bcn_loss_timeout)
		return;

	if (bcn_ol->time_since_bcn) {
		WL_ERROR(("%s: time_since_bcn %d, timeout %d sec\n",
		__FUNCTION__, bcn_ol->time_since_bcn, bcn_ol->bcn_loss_timeout));
		WL_ERROR(("%s: M_UCODE_BSSBCNCNT %d\n", __FUNCTION__,
			wlc_bmac_read_shm(wlc_dngl_ol->wlc_hw, M_UCODE_BSSBCNCNT)));

		/* Announce the event */
		wlc_dngl_ol_event(wlc_dngl_ol,
			BCM_OL_E_TIME_SINCE_BCN, &bcn_ol->time_since_bcn);
	}

	if (bcn_ol->time_since_bcn >= bcn_ol->bcn_loss_timeout) {

		wlc_dngl_ol_eventlog_write(
			wlc_dngl_ol->eventlog_ol,
			WLC_EL_BEACON_LOST,
			0);

		/* Clear the bcn_loss_timeout so we don't track bcn loss anymore */
		bcn_ol->bcn_loss_timeout = 0;

		if (bcn_ol->wowl_flags & WL_WOWL_BCN) {
			/* Wake up the host */
			wlc_wowl_ol_wake_host(wlc_dngl_ol->wowl_ol,
				NULL, 0, NULL, 0, WL_WOWL_BCN);
		} else {
			/* Announce the event */
			wlc_dngl_ol_event(wlc_dngl_ol, BCM_OL_E_BCN_LOSS, NULL);
		}
	}
	bcn_ol->time_since_bcn++;

}

static void
Dump_IEMask(uint8 *iemask)
{
	int i;
	WL_ERROR(("IE MASK: "));
	for (i = 0; i < 8; i++) {
		WL_ERROR(("%d:%02X ", i, iemask[i]));
	}
	WL_ERROR(("\n"));
}

bool
wlc_dngl_ol_bcn_process(wlc_dngl_ol_bcn_info_t *bcn_ol,
	void *p, struct dot11_management_header *hdr)
{
	wlc_dngl_ol_info_t *wlc_dngl_ol = bcn_ol->wlc_dngl_ol;
	d11regs_t *d11_regs;
	struct dot11_bcn_prb *bcnframe;
	uint8* body;
	uint8 *tag_params = NULL;
	int tag_params_len = 0;
	uint plen;
	int i = 0;
	bool bcn_to_host = FALSE;
	uint32 maxie = OLMSG_BCN_MAX_IE;
	olmsg_bcn_enable *pbcnmsg = bcn_ol->pbcn_ol_msg;
	d11_regs = (d11regs_t *)wlc_dngl_ol->regs;

#ifdef BCMDBG
	char a1[(2*ETHER_ADDR_LEN)+1], a2[(2*ETHER_ADDR_LEN)+1];
#endif // endif

	if (bcn_ol->beacon_ol_enable) {
		PKTPULL(wlc_dngl_ol->osh, p, D11_PHY_HDR_LEN);
		body = (uchar*)PKTPULL(wlc_dngl_ol->osh, p, sizeof(struct dot11_management_header));
		body  = PKTDATA(wlc_dngl_ol->osh, p);
		plen =	PKTLEN(wlc_dngl_ol->osh, p) - DOT11_FCS_LEN;
		bcnframe = (struct dot11_bcn_prb *)body;
		tag_params = ((uint8*)bcnframe + DOT11_BCN_PRB_LEN);
		tag_params_len = plen - DOT11_BCN_PRB_LEN;

		/* compare bssid of beacon */
		if (bcmp((char *)&hdr->bssid, (char*)&pbcnmsg->BSSID, ETHER_ADDR_LEN) != 0) {
#ifdef BCMDBG
			bcm_format_hex(a1, &hdr->bssid, ETHER_ADDR_LEN);
			bcm_format_hex(a2, &pbcnmsg->BSSID, ETHER_ADDR_LEN);
			WL_TRACE(("bcn_BSSID: %s t:%d\n", a1, hnd_time()));
			WL_TRACE(("our_BSSID: %s\n", a2));
#endif // endif
			RXOEINC(wlc_dngl_ol, rxoe_obssidcnt);
			return TRUE;
		}

		/* our beacon increment the beacon counter */
		RXOEINC(wlc_dngl_ol, rxoe_bcncnt);

		if (bcn_ol->time_since_bcn > 1) {

			WL_ERROR(("%s: Got lost bcn\n", __FUNCTION__));

			/* reset the beacon lost counter */
			bcn_ol->time_since_bcn = 0;

			/* Announce that we recovered bcn */
			wlc_dngl_ol_event(wlc_dngl_ol,
				BCM_OL_E_TIME_SINCE_BCN, &bcn_ol->time_since_bcn);
		}

		/* reset the beacon lost counter */
		bcn_ol->time_since_bcn = 0;

		/* In WoWL offload mode, CSA can cause AP move off to
		 * different channel thus in turn causes STA to
		 * disassoc due to beacon loss.
		 */
		if (wlc_dngl_ol->wowl_cfg.wowl_enabled) {
			uint16 chan = wlc_dngl_ol_find_csa_ie(bcn_ol, bcnframe, plen);
			/* Announce CSA the event if return chan info */
			if (chan)
				wlc_dngl_ol_event(wlc_dngl_ol, BCM_OL_E_CSA, &chan);
		}

		/*
		 * If beacon offload is enabled and deferral is disabled,
		 * allow a beacon to the host to sync up the host's
		 * state machine
		 */
		if (bcn_ol->sendBcntoHost) {
			bcn_ol->sendBcntoHost = FALSE;
			goto release;
		}

		/* unconditional check of capability */
		if (pbcnmsg->capability != bcnframe->capability) {
			RXOEINC(wlc_dngl_ol, rxoe_capchangedcnt);
			WL_ERROR(("cap mismatch:0x%x 0x%x\n",
				pbcnmsg->capability, bcnframe->capability));
			goto release;
		}

		/* unconditional check of bi */
		if (pbcnmsg->bi != bcnframe->beacon_interval) {
			RXOEINC(wlc_dngl_ol, rxoe_bcnintchangedcnt);
			WL_ERROR(("bi mismatch:%d %d\n", pbcnmsg->bi, bcnframe->beacon_interval));
			goto release;
		}

		/* check for retrograde tsf */
		{
			uint32 bcn_l, bcn_h;

			bcn_l = ltoh32_ua(&bcnframe->timestamp[0]);
			bcn_h = ltoh32_ua(&bcnframe->timestamp[1]);

			if ((bcn_ol->tsf_h || bcn_ol->tsf_l) &&
				(bcn_h < bcn_ol->tsf_h ||
				(bcn_h == bcn_ol->tsf_h && bcn_l < bcn_ol->tsf_l))) {

				/* Announce the event */
				wlc_dngl_ol_event(wlc_dngl_ol, BCM_OL_E_RETROGRADE_TSF, NULL);

				if (wlc_dngl_ol->wowl_cfg.wowl_enabled) {
					if (wlc_dngl_ol->wowl_cfg.wowl_flags & WL_WOWL_RETR) {
						/* Wake up the host */
						wlc_wowl_ol_wake_host(wlc_dngl_ol->wowl_ol, NULL, 0,
							NULL, 0, WL_WOWL_RETR);
						return FALSE;
					}
				} else {
					bcn_ol->tsf_l = bcn_l;
					bcn_ol->tsf_h = bcn_h;
					goto release;
				}
			}

			bcn_ol->tsf_l = bcn_l;
			bcn_ol->tsf_h = bcn_h;
		}

		/* First Handlespecific IEs */
		if (bcn_ol->ie_ol_enable) {
		    for (i = 0; i < maxie; i++) {
				if (GET_IE(bcn_ol->iemask, i)) {
					switch (i) {
						case DOT11_MNG_TIM_ID:
							bcn_to_host =
								wlc_dngl_ol_bcn_tim(bcn_ol,
								bcnframe, plen);
						break;
						case DOT11_MNG_VS_ID:
							bcn_to_host = wlc_dngl_ol_bcn_vndrie(bcn_ol,
								bcnframe, plen);
							break;
						default:
							bcn_to_host =
								wlc_dngl_ol_bcn_compare(bcn_ol, i,
								bcnframe, plen);
						break;
					}
					if (bcn_to_host) {
						RXOEINCIE(wlc_dngl_ol, rxoe_iechanged, i);

						wlc_dngl_ol_eventlog_write(
							wlc_dngl_ol->eventlog_ol,
							WLC_EL_BEACON_IE_CHANGED,
							i);
#ifdef BCMDBG
						WL_TRACE(("Found change in IE # %d\n", i));
#endif // endif
						goto release;
					}
				}
		    }
		}
		wlc_dngl_ol_phy_rssi_compute_offload(wlc_dngl_ol->rssi_ol, wrxh);
		if (!bcn_to_host)
			return TRUE;

	release:

		WL_TRACE(("clear deferral:%d\n", i));
		RXOEINC(wlc_dngl_ol, rxoe_bcntohost);

		/* update the template */
		pbcnmsg->capability = bcnframe->capability;
		pbcnmsg->bi = bcnframe->beacon_interval;
		/* check on length */
		if (tag_params_len > (pbcnmsg->iedatalen + MAX_IE_LENGTH_BUF))
			ASSERT(FALSE);
		pbcnmsg->iedatalen = tag_params_len;
		bcopy(((uint8 *)tag_params), &pbcnmsg->iedata[0], tag_params_len);
		wlc_dngl_ol_push_to_host(wlc_dngl_ol->wlc);
		return FALSE;
	}
	return FALSE;
}

void wlc_dngl_ol_bcn_send_proc(wlc_dngl_ol_bcn_info_t *bcn_ol, void *buf, int len)
{
	uchar *pktdata;
	olmsg_header *msg_hdr;
#ifdef BCMDBG
		char a1[(2*ETHER_ADDR_LEN)+1];
#endif // endif
	olmsg_bcn_enable *src_bcn_ol_msg;
	olmsg_bcn_disable *bcn_disable;
	wlc_dngl_ol_info_t *wlc_dngl_ol = bcn_ol->wlc_dngl_ol;
	olmsg_bcn_enable *pbcnmsg = bcn_ol->pbcn_ol_msg;

	/*	pktdata = (uint8 *) PKTDATA(osh, p); */
	pktdata = (uint8 *) buf;
	msg_hdr = (olmsg_header *) pktdata;
	switch (msg_hdr->type) {

		case BCM_OL_BEACON_ENABLE:
			WL_ERROR(("BCN Enable : len %d\n", len));
			src_bcn_ol_msg = (olmsg_bcn_enable *)pktdata;

			/* allocat additional space for changing IE length */
			pbcnmsg = (olmsg_bcn_enable *)MALLOC(wlc_dngl_ol->osh, len
				+ MAX_IE_LENGTH_BUF);
			if (!pbcnmsg) {
				WL_ERROR(("bcnmsg alloc failed\n"));
				ASSERT(FALSE);
			}

			bcn_ol->tsf_h = 0;
			bcn_ol->tsf_l = 0;
			bcn_ol->sendBcntoHost = TRUE;
			bzero(pbcnmsg, len + MAX_IE_LENGTH_BUF);
			bcopy(src_bcn_ol_msg, pbcnmsg, len);
			bcn_ol->pbcn_ol_msg = pbcnmsg;
			bcn_ol->bcn_ol_msg_len = len + MAX_IE_LENGTH_BUF;
			bcn_ol->beacon_ol_enable = TRUE;
			/* Reset moving averages for RSSI and NOISE */
			wlc_dngl_ol_reset_rssi_ma(wlc_dngl_ol->rssi_ol);
			wlc_phy_noise_reset_ma_acphy(wlc_dngl_ol->wlc->hw->band->pi);

			bcn_ol->ie_ol_enable = TRUE; /* Turn on IE Notification with BEACON OL */
			bcopy(pbcnmsg->iemask, bcn_ol->iemask, IEMASK_SZ);
#ifdef BCMDBG
			wlc_dngl_ol_bcn_enable_dump(bcn_ol, pbcnmsg);
#endif // endif
			break;

		case BCM_OL_BEACON_DISABLE:
			bcn_disable = (olmsg_bcn_disable *)pktdata;
#ifdef BCMDBG
			bcm_format_hex(a1, &pbcnmsg->BSSID, ETHER_ADDR_LEN);
			WL_ERROR(("BCN Disable: %s, msg_len %d\n", a1, bcn_ol->bcn_ol_msg_len));
#endif // endif
			bcn_ol->beacon_ol_enable = FALSE;
			bcn_ol->ie_ol_enable = FALSE;
			bcn_ol->sendBcntoHost = FALSE;

			if (bcn_ol->pbcn_ol_msg) {
				MFREE(wlc_dngl_ol->osh,
					bcn_ol->pbcn_ol_msg, bcn_ol->bcn_ol_msg_len);
			}

			bcn_ol->pbcn_ol_msg = NULL;
			bcn_ol->bcn_ol_msg_len = 0;
			break;

		case BCM_OL_MSG_IE_NOTIFICATION: {
				uint32 id = 0;
				uint32 enable = 0;
				olmsg_ie_notification_enable *bcn_ie_notification =
					(olmsg_ie_notification_enable *)pktdata;
				id = bcn_ie_notification->id;
				enable = bcn_ie_notification->enable;
				WL_ERROR(("IE Notification message type:%d ID: %d flag: %s\n",
					msg_hdr->type, id, ((enable) ? "enable":"disable")));
				if (enable) {
					SET_IE(bcn_ol->iemask, id);
				} else {
					RESET_IE(bcn_ol->iemask, id);
				}
				Dump_IEMask(bcn_ol->iemask);
			}
			break;

		case BCM_OL_MSG_IE_NOTIFICATION_FLAG: {
				olmsg_ie_notification_enable *bcn_ie_notification =
					(olmsg_ie_notification_enable *)pktdata;
				bcn_ol->ie_ol_enable = bcn_ie_notification->enable;
				WL_ERROR(("IE Notification message type:%d, flag: %s\n",
					msg_hdr->type,
					((bcn_ol->ie_ol_enable) ? "enable":"disable")));
			}
			break;

		default:
			WL_ERROR(("%s: INVALID message type:%d\n", __FILE__, msg_hdr->type));
			break;
	}

}

static uint16
wlc_dngl_ol_find_csa_ie(wlc_dngl_ol_bcn_info_t *bcn_ol, void *bcnframe, uint plen)
{
	dot11_chan_switch_ie_t *csa_ie;
	olmsg_bcn_enable *pbcnmsg = bcn_ol->pbcn_ol_msg;
	uint8 *tlvs = (uint8 *)bcnframe + DOT11_BCN_PRB_LEN;
	int tlvs_len = plen - DOT11_BCN_PRB_LEN;

	csa_ie = (dot11_chan_switch_ie_t *)bcm_parse_tlvs(tlvs, tlvs_len,
		DOT11_MNG_CHANNEL_SWITCH_ID);

	/* Check for CSA IE present and invalid len */
	if (!csa_ie || csa_ie->len < DOT11_SWITCH_IE_LEN)
		return 0;

	if (csa_ie->channel == pbcnmsg->ctrl_channel) {
		WL_INFORM(("%s: CSA IE channel same %d == %d\n", __FUNCTION__,
		       csa_ie->channel, pbcnmsg->ctrl_channel));
		return 0;
	}

	WL_INFORM(("%s: CSA IE channel mis-match (%d != %d)\n", __FUNCTION__,
	       csa_ie->channel, pbcnmsg->ctrl_channel));
	return csa_ie->channel;
}

bcm_tlv_t *
wlc_dngl_ol_find_vndrie(vndriemask_info *ouidata, void *iedata, uint plen)
{
	bcm_tlv_t *ie;
	uint8 *tlvs = (uint8 *)iedata;
	int tlvs_len = plen;
	vndriemask_info *oui_orig;
	uint ie_len = 0;

	while ((ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_VS_ID))) {
		oui_orig = (vndriemask_info *) ie->data;

		/* compare with vndriemask */
		if (memcmp((uint8*)oui_orig, (uint8*)ouidata, sizeof(vndriemask_info)) == 0) {
			WL_TRACE(("OUI found in BCN and from template\n"));
			return ie;
		}
		ie_len = TLV_HDR_LEN + ie->len;
		tlvs = (uint8 *)ie;
		tlvs += ie_len;
		tlvs_len -= ie_len;
	}
	return NULL;
}

static bool
wlc_dngl_ol_bcn_vndrie(wlc_dngl_ol_bcn_info_t *bcn_ol, void *bcnframe, uint plen)
{
	bcm_tlv_t *bcn_ie;
	bcm_tlv_t *orig_ie;
	olmsg_bcn_enable *pbcnmsg = bcn_ol->pbcn_ol_msg;
	uint8 *tlvs = (uint8 *)bcnframe+ DOT11_BCN_PRB_LEN;
	int tlvs_len = plen - DOT11_BCN_PRB_LEN;
	int i;
	uint32 maxie = MAX_VNDR_IE;
	/*
	* We want to check, for each vndriemask[] info:
	*  - is the IE/OUI/subtype in orig,
	* and does the contents not match in the input beacon -> send up bcn
	* - or is the IE/OUI/subtype not in orig, and appears in the input beacon -> send up bcn
	I think this code will miss the case of a vendor IE disappearing.
	*/
	/* for each vndriemask */
	for (i = 0; i < maxie; i++) {
		if (SET_VNDRIE(pbcnmsg->vndriemask[i].oui.mask)) {

			WL_TRACE(("i:%d ouimask:0x%x\n", i, pbcnmsg->vndriemask[i].oui.mask));

			/* find IE/OUI/Subtype in current RX BCN */
			bcn_ie = wlc_dngl_ol_find_vndrie(&pbcnmsg->vndriemask[i], tlvs, tlvs_len);
			/* find IE/OUI/Subtype from host template */
			orig_ie = wlc_dngl_ol_find_vndrie(&pbcnmsg->vndriemask[i],
				(uint8 *)pbcnmsg->iedata, pbcnmsg->iedatalen);
			if (!bcn_ie && !orig_ie) {
				/* IE does not exist in both, then continue until list exhausted */
				WL_TRACE(("IE/OUI not found\n"));
				continue;
			} else if (bcn_ie && orig_ie) {
			/* compare both ie */
				WL_TRACE(("IE/OUI found\n"));
			if (orig_ie->len != bcn_ie->len) {
					WL_TRACE(("Found IE length mismatch\n"));
				return TRUE;
			}
			if (memcmp(bcn_ie, orig_ie, orig_ie->len)) {
					WL_TRACE(("IE mismatch reset deferral\n"));
					return TRUE;
				}
			} else {
				/* IE in one but not other, mismatch */
				return TRUE;
			}
		}
	}
	WL_TRACE(("No reset from vndrie mask \n"));
	return FALSE;
}

static int wlc_dngl_ol_bcn_compare(wlc_dngl_ol_bcn_info_t *bcn_ol,
	int ie_id, void *bcnframe, uint plen)
{
	bcm_tlv_t *tag;
	bcm_tlv_t *orig_ie;
	olmsg_bcn_enable *pbcnmsg = bcn_ol->pbcn_ol_msg;
	uint8 *tlvs = (uint8 *)bcnframe+ DOT11_BCN_PRB_LEN;
	int tlvs_len = plen - DOT11_BCN_PRB_LEN;

	tag = bcm_parse_tlvs(tlvs, tlvs_len, ie_id);

	orig_ie = bcm_parse_tlvs((uint8 *)pbcnmsg->iedata, pbcnmsg->iedatalen,
		ie_id);

	/* If the IE is not present in the new beacon,
	* return TRUE if the original has the IE present
	*/
	if (tag == NULL) {
		 return (orig_ie != NULL);
	}

	/* return TRUE if the original does not have the IE,
	* or there is a length or contents mismatch
	*/
	if (orig_ie == NULL ||
		orig_ie->len != tag->len ||
		memcmp(tag, orig_ie, tag->len)) {
#ifdef BCMDBG
		WL_TRACE(("Changed IE ID: %d\n", ie_id));
#endif // endif
		return TRUE;
	}
	/* otherwise, the new and old IE match */
	return FALSE;
}

static int wlc_dngl_ol_bcn_tim(wlc_dngl_ol_bcn_info_t *bcn_ol, void* bcnframe, uint plen)
{

	bcm_tlv_t *tim_ie;
	uint pvboff, AIDbyte;
	uint pvblen;
	olmsg_bcn_enable *pbcnmsg = bcn_ol->pbcn_ol_msg;
	uint8 *tlvs = (uint8 *)bcnframe+ DOT11_BCN_PRB_LEN;
	int tlvs_len = plen - DOT11_BCN_PRB_LEN;
	bool intim = FALSE;
	wlc_dngl_ol_info_t *wlc_dngl_ol = bcn_ol->wlc_dngl_ol;

	tim_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_TIM_ID);

	if (!tim_ie || tim_ie->len < DOT11_MNG_TIM_FIXED_LEN ||
	    tim_ie->data[DOT11_MNG_TIM_DTIM_COUNT] >= tim_ie->data[DOT11_MNG_TIM_DTIM_PERIOD]) {
		/* sick AP, prevent going to power-save mode */
#ifdef BCMDBG
		WL_ERROR(("Sick AP\n"));
#endif // endif
		if (wlc_dngl_ol->TX) {
			if (wlc_dngl_ol->wowl_cfg.wowl_enabled) {
				wlc_dngl_ol_sendpspoll(wlc_dngl_ol);
			}
		}
	}

#ifdef BCMDBG
	WL_TRACE(("AID:0x%x\n", pbcnmsg->aid));
#endif // endif
	/* extract bitmap offset (N1) from bitmap control field */
	pvboff = tim_ie->data[DOT11_MNG_TIM_BITMAP_CTL] & 0xfe;

	/* compute bitmap length (N2 - N1) from info element length */
	pvblen = tim_ie->len - DOT11_MNG_TIM_FIXED_LEN;

	/* bail early if our AID precedes the TIM */
	AIDbyte = (pbcnmsg->aid & DOT11_AID_MASK) >> 3;
	if (AIDbyte < pvboff || AIDbyte >= pvboff + pvblen)
		return FALSE;

	/* check our AID in bitmap */
	intim = (tim_ie->data[DOT11_MNG_TIM_PVB + AIDbyte - pvboff] &
	        (1 << (pbcnmsg->aid & 0x7))) ? TRUE : FALSE;
	if (wlc_dngl_ol->wowl_cfg.wowl_enabled) {
		if (intim && wlc_dngl_ol->TX) {
			wlc_dngl_ol_sendpspoll(wlc_dngl_ol);
		} else {
			wlc_dngl_ol_staywake_check(wlc_dngl_ol, intim);
		}
	}
	return intim;
}

#ifdef BCMDBG
static void wlc_dngl_ol_bcn_enable_dump(wlc_dngl_ol_bcn_info_t *bcn_ol, olmsg_bcn_enable *pbcnmsg)
{
	WL_TRACE(("defcnt:%d bcn_length:%d bi:%d\n"
			"rxchannel:%d aid:%d iemask:0x%x iedatalen:%d\n", pbcnmsg->defcnt,
			pbcnmsg->bcn_length, pbcnmsg->bi,
			pbcnmsg->rxchannel, pbcnmsg->aid, bcn_ol->iemask[0],  pbcnmsg->iedatalen));
	WL_TRACE(("%p %p\n", bcn_ol, pbcnmsg));
	WL_TRACE(("iedata:: iedatalen:%d\n", pbcnmsg->iedatalen));
}
#endif // endif
