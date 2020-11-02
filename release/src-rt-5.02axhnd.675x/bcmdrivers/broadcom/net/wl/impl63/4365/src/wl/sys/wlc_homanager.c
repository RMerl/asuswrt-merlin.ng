/*
* HandOver manager for TBOW
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
* $Id: wlc_homanager.c 708017 2017-06-29 14:11:45Z $
*
*/
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <epivers.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <d11.h>

#include <wlc_pub.h>
#include <wlc_cca.h>
#include <wlc_interfere.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_tbow.h>
#include <wlc_tbow_priv.h>

static int tbow_get_random(uint8 *buf, int len, uint32 now)
{
	uint8 val;
	uint8 i;
	uint8 letters = 'Z' - 'A' + 1;
	uint8 numbers = 10;

	ASSERT(buf);

	if (len > sizeof(now) * 2)
		return -1;

	for (i = 0; i < sizeof(now) * 2; i++) {
		buf[i] = (uint8) ((now & (0xf << (i * 4))) >> (i * 4));
	}

	/* Character set: 'A'-'Z', 'a'-'z', '0'-'9' */
	for (i = 0; i < len; i++) {
		val = buf[i];
		val %= 2 * letters + numbers;
		if (val < letters)
			buf[i] = 'A' + val;
		else if (val < 2 * letters)
			buf[i] = 'a' + (val - letters);
		else
			buf[i] = '0' + (val - 2 * letters);
	}

	return 0;
}

/* This function is called only when it is GO/master */
void tbow_send_goinfo(tbow_info_t *tbow_info)
{
	tbow_ho_setupmsg_t *setupmsg;
	uint32 now = OSL_SYSUPTIME(), now2 = now;
	int i;

	ASSERT(tbow_info);

	if (!(setupmsg = MALLOC(tbow_info->wlc->osh, sizeof(tbow_ho_setupmsg_t)))) {
		WL_ERROR(("out of memory: %dbytes\n", sizeof(tbow_ho_setupmsg_t)));
		return;
	}
	bzero(setupmsg, sizeof(tbow_ho_setupmsg_t));

	tbow_info->ho_state = TBOW_HO_IDLE; /* init to dile */
	tbow_info->ho_rate = 0; /* Do not apply fixed rate by default */
	if (getvar(NULL, "horate")) {
		tbow_info->ho_rate = (uint8)getintvar(NULL, "horate") << 1;
		if (tbow_info->ho_rate == 10)
			tbow_info->ho_rate = 11;
		if ((tbow_info->ho_rate != 2) && (tbow_info->ho_rate != 4) &&
			(tbow_info->ho_rate != 11) && (tbow_info->ho_rate != 22))
			tbow_info->ho_rate = 0;
		WL_TRACE(("fix rate for HO: %d\n", tbow_info->ho_rate));
	}
	/* store all the setup info in tbow_info->goinfo */
	bzero(&tbow_info->goinfo, sizeof(tbow_setup_netinfo_t));
	tbow_info->goinfo.opmode = TBOW_HO_MODE_START_GO;
	tbow_info->goinfo.channel = 1;
	if (getvar(NULL, "hochan")) {
		tbow_info->goinfo.channel = (uint8)getintvar(NULL, "hochan");
		WL_TRACE(("change default channel to %d\n", tbow_info->goinfo.channel));
	}
	tbow_info->goinfo.ssid_len = 11;
	memcpy(tbow_info->goinfo.ssid, "DIRECT-", 7);
	tbow_get_random(tbow_info->goinfo.ssid + 7, tbow_info->goinfo.ssid_len- 7, now);
	tbow_info->goinfo.passphrase_len = 10;
	now = ((now & 0xffff) << 8) + ((now & 0xffff) << 16) + now;
	tbow_get_random(tbow_info->goinfo.passphrase, 8, now);
	for (i = 8; i < tbow_info->goinfo.passphrase_len; i++) {
		tbow_info->goinfo.passphrase[i] = '0' + now2 % 10;
		now2 = now2 / 10;
	}

	/* generate p2p interface address for GO,
	 * consistent with wl_cfgp2p_generate_bss_mac() in wl_cfgp2p.c
	 */
	memcpy(tbow_info->goinfo.macaddr,
		tbow_info->wlc->bsscfg[0]->cur_etheraddr.octet, ETHER_ADDR_LEN);
	tbow_info->goinfo.macaddr[0] |= 0x02; /* set locally administered bit */
	tbow_info->goinfo.macaddr[4] ^= 0x80;

	/* send it to BT */
	setupmsg->type = TBOW_HO_MSG_GO_STA_SETUP;
	if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GO)
		setupmsg->opmode = TBOW_HO_MODE_START_GC;
	else if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_STA ||
		tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GC)
		setupmsg->opmode = TBOW_HO_MODE_START_GO;
	setupmsg->channel = tbow_info->goinfo.channel;
	if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GO ||
		tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GC)
		memcpy(setupmsg->sender_mac, tbow_info->goinfo.macaddr, ETHER_ADDR_LEN);
	else if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_STA)
		memcpy(setupmsg->sender_mac,
			&tbow_info->wlc->bsscfg[0]->current_bss->BSSID, ETHER_ADDR_LEN);
	setupmsg->ssid_len = tbow_info->goinfo.ssid_len;
	memcpy(setupmsg->ssid, tbow_info->goinfo.ssid, tbow_info->goinfo.ssid_len);
	setupmsg->pph_len = tbow_info->goinfo.passphrase_len;
	memcpy(setupmsg->pph, tbow_info->goinfo.passphrase, tbow_info->goinfo.passphrase_len);

{	/* for debugging */
	int i;
	WL_TRACE(("send_goinfo to slave: type:%d, opmode:%d, channel:%d, "
		"mac%02x:%02x:%02x:%02x:%02x:%02x\n",
		setupmsg->type, setupmsg->opmode, setupmsg->channel,
		setupmsg->sender_mac[0], setupmsg->sender_mac[1], setupmsg->sender_mac[2],
		setupmsg->sender_mac[3], setupmsg->sender_mac[4], setupmsg->sender_mac[5]));

	WL_TRACE(("ssid_len=%d, ssid: ", setupmsg->ssid_len));
	for (i = 0; i < setupmsg->ssid_len; i++)
		WL_TRACE(("%c", setupmsg->ssid[i]));
	WL_TRACE(("\n"));
	WL_TRACE(("passphrase_len=%d, passphrase: ", setupmsg->pph_len));
	for (i = 0; i < setupmsg->pph_len; i++)
		WL_TRACE(("%c", setupmsg->pph[i]));
	WL_TRACE(("\n"));
}
	/* call API to send this ctrl message to BT */
	tbow_send_bt_msg(tbow_info, (uchar *)setupmsg, sizeof(tbow_ho_setupmsg_t));
}

static int tbow_recv_goinfo(tbow_info_t *tbow_info, uchar *msg, int len)
{
	tbow_ho_ack_setupmsg_t *ack_setupmsg;
	tbow_ho_setupmsg_t *setupmsg = (tbow_ho_setupmsg_t *)msg;

	ASSERT(tbow_info && msg);

	if (len != sizeof(tbow_ho_setupmsg_t)) {
		WL_ERROR(("bad control len:%d, size:%d\n", len, sizeof(tbow_ho_setupmsg_t)));
		return -1;
	}

	if (!(ack_setupmsg = MALLOC(tbow_info->wlc->osh, sizeof(tbow_ho_ack_setupmsg_t)))) {
		WL_ERROR(("out of memory: %dbytes\n", sizeof(tbow_ho_ack_setupmsg_t)));
		return -1;
	}
	bzero(ack_setupmsg, sizeof(tbow_ho_ack_setupmsg_t));

	tbow_info->ho_state = TBOW_HO_IDLE;
	bzero(&tbow_info->goinfo, sizeof(tbow_setup_netinfo_t));
	tbow_info->goinfo.opmode = setupmsg->opmode;
	tbow_info->goinfo.channel = setupmsg->channel;
	tbow_info->goinfo.ssid_len = setupmsg->ssid_len;
	memcpy(tbow_info->goinfo.ssid, setupmsg->ssid, setupmsg->ssid_len);
	tbow_info->goinfo.passphrase_len = setupmsg->pph_len;
	memcpy(tbow_info->goinfo.passphrase, setupmsg->pph, setupmsg->pph_len);
	if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_STA)
		memcpy(tbow_info->goinfo.macaddr,
			&tbow_info->wlc->bsscfg[0]->cur_etheraddr, ETHER_ADDR_LEN);
	else if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GC) {
		/* store GO's mac, i.e., BSSID, for GC to connect */
		memcpy(&tbow_info->go_BSSID, setupmsg->sender_mac, ETHER_ADDR_LEN);
		/* generate p2p virtual interface address for GC,
		 * consistent with wl_cfgp2p_generate_bss_mac() in wl_cfgp2p.c
		 */
		memcpy(tbow_info->goinfo.macaddr,
			tbow_info->wlc->bsscfg[0]->cur_etheraddr.octet, ETHER_ADDR_LEN);
		tbow_info->goinfo.macaddr[0] |= 0x02; /* set locally administered bit */
		tbow_info->goinfo.macaddr[4] ^= 0x80;
	} else {
		WL_ERROR(("%s: Unsupported opmode(%d)\n", __FUNCTION__, tbow_info->goinfo.opmode));
		return -1;
	}

	{	/* for debugging */
		int i;
		WL_TRACE(("STA/GC rev: opmode:%d, channel:%d, mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
			tbow_info->goinfo.opmode, tbow_info->goinfo.channel,
			setupmsg->sender_mac[0], setupmsg->sender_mac[1], setupmsg->sender_mac[2],
			setupmsg->sender_mac[3], setupmsg->sender_mac[4], setupmsg->sender_mac[5]));
		WL_TRACE(("ssid_len=%d, ssid: ", tbow_info->goinfo.ssid_len));
		for (i = 0; i < tbow_info->goinfo.ssid_len; i++)
			WL_TRACE(("%c", tbow_info->goinfo.ssid[i]));
		WL_TRACE(("\n"));
		WL_TRACE(("passphrase_len=%d, passphrase: ", tbow_info->goinfo.passphrase_len));
		for (i = 0; i < tbow_info->goinfo.passphrase_len; i++)
			WL_TRACE(("%c", tbow_info->goinfo.passphrase[i]));
		WL_TRACE(("\n"));
		if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GC) {
			WL_TRACE(("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
				tbow_info->go_BSSID.octet[0], tbow_info->go_BSSID.octet[1],
				tbow_info->go_BSSID.octet[2],	tbow_info->go_BSSID.octet[3],
				tbow_info->go_BSSID.octet[4], tbow_info->go_BSSID.octet[5]));
		}
		WL_TRACE(("Own mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
			tbow_info->goinfo.macaddr[0], tbow_info->goinfo.macaddr[1],
			tbow_info->goinfo.macaddr[2], tbow_info->goinfo.macaddr[3],
			tbow_info->goinfo.macaddr[4], tbow_info->goinfo.macaddr[5]));
	}

	/* set peer's (setupmsg->sender_mac) and its own mac */
	tbow_set_mac(tbow_info, (struct ether_addr *)&tbow_info->goinfo.macaddr,
		(struct ether_addr *)setupmsg->sender_mac);

	/* make up ack to setup message */
	ack_setupmsg->type = TBOW_HO_MSG_SETUP_ACK;
	memcpy(&ack_setupmsg->recv_mac, tbow_info->goinfo.macaddr, ETHER_ADDR_LEN);
	/* call API to send ack to BT */
	tbow_send_bt_msg(tbow_info, (uchar *)ack_setupmsg, sizeof(tbow_ho_ack_setupmsg_t));

	return 0;
}

static int tbow_recv_ack2goinfo(tbow_info_t *tbow_info, uchar *msg, int len)
{
	tbow_ho_ack_setupmsg_t *ack_setupmsg = (tbow_ho_ack_setupmsg_t *)msg;

	ASSERT(tbow_info && msg);

	if (len != sizeof(tbow_ho_ack_setupmsg_t)) {
		WL_ERROR(("bad control message ack2goinfo\n"));
		return -1;
	}
	WL_TRACE(("Recv ack to goinfo\n"));
	/* set peer's and its own mac */
	tbow_set_mac(tbow_info, (struct ether_addr *)tbow_info->goinfo.macaddr,
		(struct ether_addr *)&ack_setupmsg->recv_mac);
	return 0;
}

int tbow_start_ho(tbow_info_t *tbow_info)
{
	wlc_bsscfg_t *bsscfg;
	tbow_setup_netinfo_t *netinfo;

	ASSERT(tbow_info);
	/* use bsscfg w/primary interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(tbow_info->wlc, NULL);
	ASSERT(bsscfg != NULL);

	netinfo = MALLOC(tbow_info->wlc->osh, sizeof(tbow_setup_netinfo_t));
	if (!netinfo) {
		WL_ERROR(("out of memory: %dbyte\n", sizeof(tbow_setup_netinfo_t)));
		return -1;
	}
	memcpy(netinfo, &tbow_info->goinfo, sizeof(tbow_setup_netinfo_t));
	if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GC) {
		memcpy(netinfo->macaddr, &tbow_info->go_BSSID, ETHER_ADDR_LEN);
	}

	/* send network setup info in event */
	wlc_bss_mac_event(tbow_info->wlc, bsscfg,
		WLC_E_BT_WIFI_HANDOVER_REQ, NULL,
		WLC_E_STATUS_SUCCESS, 0, 0, netinfo, sizeof(tbow_setup_netinfo_t));

	tbow_info->ho_state = TBOW_HO_START;

	return 0;
}

int tbow_ho_parse_ctrlmsg(tbow_info_t *tbow_info, uchar *msg, int len)
{
	ASSERT(tbow_info && msg);

	switch (*msg) {
		case TBOW_HO_MSG_BT_READY:
			tbow_send_goinfo(tbow_info);
			return 0;
		case TBOW_HO_MSG_GO_STA_SETUP:
			return tbow_recv_goinfo(tbow_info, msg, len);
		case TBOW_HO_MSG_SETUP_ACK:
			return tbow_recv_ack2goinfo(tbow_info, msg, len);
		case TBOW_HO_MSG_START:
			return tbow_start_ho(tbow_info);
		default:
			WL_ERROR(("invalid control message from BT\n"));
			break;
	}

	return -1;
}

int tbow_ho_stop(tbow_info_t *tbow_info)
{
	tbow_setup_netinfo_t netinfo;
	wlc_bsscfg_t *bsscfg;

	ASSERT(tbow_info);

	WL_TRACE(("process ho stop\n"));
	/* use bsscfg w/primary interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(tbow_info->wlc, NULL);
	ASSERT(bsscfg != NULL);

	bzero(&netinfo, sizeof(tbow_setup_netinfo_t));
	if (tbow_info->goinfo.opmode == TBOW_HO_MODE_START_GO) {
		netinfo.opmode = TBOW_HO_MODE_STOP_GO;
		wlc_bss_mac_event(tbow_info->wlc, bsscfg,
			WLC_E_BT_WIFI_HANDOVER_REQ, NULL,
			WLC_E_STATUS_SUCCESS, 0, 0, &netinfo, sizeof(tbow_setup_netinfo_t));
	}
	tbow_info->ho_state = TBOW_HO_IDLE;

	return 0;
}

uint32 tbow_ho_connection_done(tbow_info_t *tbow_info, wlc_bsscfg_t *bsscfg, wl_wsec_key_t *key)
{
	ASSERT(tbow_info && bsscfg && key);

	if ((tbow_info->ho_state != TBOW_HO_START) || ETHER_ISNULLADDR(&key->ea)) {
		return 0;
	}

	WL_TRACE(("HO connection set up\n"));
	if (memcmp(&bsscfg->cur_etheraddr, tbow_info->goinfo.macaddr, ETHER_ADDR_LEN) == 0) {
		/* match */
		uchar *ready = MALLOC(tbow_info->wlc->osh, 1);
		if (!ready) {
			WL_ERROR(("out of memory: 1 byte\n"));
			return 0;
		}

		*ready = TBOW_HO_MSG_WLAN_READY;
		WL_TRACE(("indicate HO connection done\n"));
		tbow_send_bt_msg(tbow_info, ready, 1);
		tbow_info->ho_state = TBOW_HO_FINISH;
	}

	if ((tbow_info->ho_rate != 0) && (tbow_info->goinfo.channel <= 11))  {
		WL_TRACE(("HO use fixed b rate: %d\n", tbow_info->ho_rate));
		return (uint32)(tbow_info->ho_rate);
	} else {
		return 0;
	}
}
