/*
 * Common (OS-independent) portion of
 * Broadcom 802.11 offload Driver
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
 * $Id: wlc_dngl_ol.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * Functions in this file are intended for full NIC model offload. Only called when BCM_OL_DEV is
 * defined.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [OffloadsDesign] [OffloadsPhase2]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <proto/802.11.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <proto/802.1d.h>
#include <pcie_core.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wl_export.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_hw_priv.h>
#include <proto/802.3.h>
#include <proto/eapol.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/bcmarp.h>
#include <bcmcrypto/rc4.h>
#include <bcmcrypto/tkmic.h>
#include <wlc_bcnol.h>
#include <wl_arpoe.h>
#include <wl_tcpkoe.h>
#include <wl_ndoe.h>
#include <wlc_wowlol.h>
#include <wlc_eventlog_ol.h>
#include <wlc_pktfilterol.h>
#include <rte_cons.h>

#ifdef BCMDBG_DNGL_OL
void wlc_dngl_print_dot11_mac_hdr(uint8* buf, int len);
void wlc_dngl_print_txdesc_ac(wlc_info_t *wlc, void* hdrsBegin);
static void wlc_dngl_print_per_pkt_desc_ac(d11actxh_t* acHdrPtr);
static void wlc_dngl_print_per_pkt_cache_ac(wlc_info_t *wlc, d11actxh_t* acHdrPtr);
void wlc_dngl_print_txdesc(wlc_info_t *wlc, wlc_txd_t *txd);
void wlc_dngl_print_txdesc(wlc_info_t *wlc, wlc_txd_t *txd);
void wlc_dngl_print_hdrs(wlc_info_t *wlc, const char *prefix, uint8 *frame,
               wlc_txd_t *txd, wlc_d11rxhdr_t *wrxh, uint len);
uint wlc_dngl_tso_hdr_length(d11ac_tso_t* tso);
static void wlc_dngl_print_buf(uint8 *p, uint size);

static const char* errstr = "802.11 Header INCOMPLETE\n";
static const char* fillstr = "------------";
#endif /* BCMDBG_DNGL_OL */

#ifdef MFP
static bool wlc_dngl_ol_mfp_rx(wlc_dngl_ol_info_t *wlc_dngl_ol,
	d11rxhdr_t *rxhdr, struct dot11_management_header *hdr, void *pp);
#endif /* MFP */

#ifdef L2KEEPALIVEOL
#include <wlc_l2keepaliveol.h>
#endif // endif
#ifdef GTKOL
#include <wlc_gtkol.h>
#endif // endif
#ifdef LTROL
#include <wlc_ltrol.h>
#endif // endif
#include <wl_mdns.h>
#include <wlc_dngl_ol.h>

#define	OL_HWRXOFF		40	/* chip rx buffer offset */
#if (WLC_KEY_BASE_RX_SEQ != 4)
#error 'Expecting WLC_KEY_BASE_RX_SEQ == 4 for offloads'
#endif // endif

/* Delete frame */
#define SUPR_FRAME	0xffff
#define RXS_FRAMEPTR	0xfe00

#define INVALID_ETHER_TYPE 0xffff

extern pktpool_t *pktpool_shared_msgs;

extern void wl_tcp_keepalive_event_handler(wl_tcp_keep_info_t *tcpkeepi, uint32 event,
	void *event_data);

static void
wlc_dngl_ol_disassoc(wlc_dngl_ol_info_t *wlc_dngl_ol);

static void
wlc_dngl_ol_event_handler(wlc_dngl_ol_info_t *wlc_dngl_ol, uint32 event, void *event_data);

extern void wl_icmp_event_handler(wl_icmp_info_t *icmpi, uint32 event, void *event_data);

extern wlc_dngl_ol_rssi_info_t *wlc_dngl_ol_rssi_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wlc_dngl_ol_rssi_send_proc(wlc_dngl_ol_rssi_info_t *rssi_ol, void *buf, int len);
extern int wlc_dngl_ol_phy_rssi_compute_offload(wlc_dngl_ol_rssi_info_t *rssi_ol,
	wlc_d11rxhdr_t *wlc_rxh);

const uint8 prio2fifo[NUMPRIO] = {
	TX_AC_BE_FIFO,	/* 0	BE	AC_BE	Best Effort */
	TX_AC_BK_FIFO,	/* 1	BK	AC_BK	Background */
	TX_AC_BK_FIFO,	/* 2	--	AC_BK	Background */
	TX_AC_BE_FIFO,	/* 3	EE	AC_BE	Best Effort */
	TX_AC_VI_FIFO,	/* 4	CL	AC_VI	Video */
	TX_AC_VI_FIFO,	/* 5	VI	AC_VI	Video */
	TX_AC_VO_FIFO,	/* 6	VO	AC_VO	Voice */
	TX_AC_VO_FIFO	/* 7	NC	AC_VO	Voice */
};

/* TX FIFO number to WME/802.1E Access Category */
const uint8 wme_fifo2ac[] = { AC_BK, AC_BE, AC_VI, AC_VO, AC_BE, AC_BE };

/* WME/802.1E Access Category to TX FIFO number */
static const uint8 wme_ac2fifo[] = { 1, 0, 2, 3 };
static void
wlc_dngl_ol_recvdata(wlc_dngl_ol_info_t *wlc_dngl_ol, osl_t *osh,
	void **pp, d11rxhdr_t *rxh);
static void
wlc_dngl_ol_compute_ofdm_plcp(ratespec_t rspec, uint32 length, uint8 *plcp);
static uint16
wlc_dngl_ol_d11hdrs(wlc_dngl_ol_info_t *wlc_dngl_ol, void *p, ratespec_t rspec_override,
	const wlc_key_info_t *key_info, d11actxh_t **out_txh, uint16 *txh_off, int fifo);
static void
wlc_dngl_ol_toe_add_hdr(wlc_dngl_ol_info_t *wlc_dngl_ol, void *p, uint16 *pushlen);

#ifdef SCANOL
extern void wlc_dngl_ol_scan_send_proc(wlc_hw_info_t *wlc_hw, uint8 *buf, uint len);
extern void wlc_scanol_event_handler(
	wlc_dngl_ol_info_t *wlc_dngl_ol,
	uint32 event,
	void *event_data);
#else
#define wlc_dngl_ol_scan_send_proc(a, b, c)	do { } while (0)
#define wlc_scanol_event_handler(a, b, c)	do { } while (0)
#endif /* SCANOL */

#if defined(BCMDBG) || defined(BCMDBG_ERR)
const char *bcm_ol_event_str[BCM_OL_E_MAX] =  {
	"BCM_OL_E_WOWL_START",
	"BCM_OL_E_WOWL_COMPLETE",
	"BCM_OL_E_TIME_SINCE_BCN",
	"BCM_OL_E_BCN_LOSS",
	"BCM_OL_E_DEAUTH",
	"BCM_OL_E_DISASSOC",
	"BCM_OL_E_RETROGRADE_TSF",
	"BCM_OL_E_RADIO_HW_DISABLED",
	"BCM_OL_E_PME_ASSERTED",
	"BCM_OL_E_UNASSOC",
	"BCM_OL_E_SCAN_BEGIN",
	"BCM_OL_E_SCAN_END",
	"BCM_OL_E_PREFSSID_FOUND",
	"BCM_OL_E_CSA"
};
#endif /* BCMDBG */

wlc_dngl_ol_info_t *
wlc_dngl_ol_attach(wlc_info_t *wlc)
{
	/* uint msglevel = 2; */
	wlc_dngl_ol_info_t *wlc_dngl_ol;

	/* shared info size must agree between host and dongle */
	#define SHARED_INFO_SZ sizeof(olmsg_shared_info_t) /* 11648 */
	#define WDOG_CNTR_OFFSET 8232

	STATIC_ASSERT(OLMSG_SHARED_INFO_SZ == OLMSG_SHARED_INFO_SZ_NUM);

	/* allocate private info */
	if (!(wlc_dngl_ol = (wlc_dngl_ol_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_dngl_ol_info_t)))) {
		WL_ERROR(("wl%d: MALLOCZ failed\n", 0));
		goto fail;
	}

	wlc_dngl_ol->osh = wlc->osh;
	wlc_dngl_ol->wlc_hw = wlc->hw;
	wlc_dngl_ol->wlc = wlc;
	wlc_dngl_ol->shared_msgpool = pktpool_shared_msgs;
	wlc_dngl_ol->regs = (void *)wlc_dngl_ol->wlc_hw->regs;
	wlc_dngl_ol->shared = ppcie_shared;

	/*
	 * Do module specific attach. We need to always attach WoWL first
	 * in order for other modules to register WoWL callback functions.
	 */

	/* allocate the wowl info struct */
	if ((wlc_dngl_ol->wowl_ol = wlc_wowl_ol_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc: wlc_wowl_ol_attach failed\n"));
		goto fail;
	}
	/* beacon offloads attach */
	if ((wlc_dngl_ol->bcn_ol = wlc_dngl_ol_bcn_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc_dngl_ol_bcn_attach failed\n"));
		goto fail;
	}

	/* allocate the packet filter info struct */
	if ((wlc_dngl_ol->pkt_filter_ol = wlc_pkt_filter_ol_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc: wlc_pkt_filter_ol_attach failed\n"));
		goto fail;
	}
#ifdef L2KEEPALIVEOL
	/* allocate l2 keepalive info struct */
	if ((wlc_dngl_ol->l2keepalive_ol = wlc_dngl_ol_l2keepalive_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc_dngl_ol_l2keepalive_attach failed\n"));
		goto fail;
	}
#endif // endif
#ifdef GTKOL
	/* allocate gtk offloads info struct */
	if ((wlc_dngl_ol->ol_gtk = wlc_dngl_ol_gtk_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc_dngl_ol_gtk_attach failed\n"));
		goto fail;
	}
#endif // endif
#ifdef MDNS
	/* mDNS/Bonjour offloads attach */
	if ((wlc_dngl_ol->mdns_ol = wlc_dngl_ol_mdns_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc_dngl_ol_mdns_attach failed\n"));
		goto fail;
	}
#endif // endif
	if ((wlc_dngl_ol->rssi_ol = wlc_dngl_ol_rssi_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc_dngl_ol_rssi_attach failed\n"));
		goto fail;
	}

	if ((wlc_dngl_ol->eventlog_ol = wlc_dngl_ol_eventlog_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc_dngl_ol_eventlog_attach failed\n"));
		goto fail;
	}

#ifdef LTROL
	if ((wlc_dngl_ol->ltr_ol = wlc_dngl_ol_ltr_attach(wlc_dngl_ol)) == NULL) {
		WL_ERROR(("wlc_dngl_ol_ltr_attach failed\n"));
		goto fail;
	}
#endif /* LTROL */

	wlc_dngl_ol->pso_blk = wlc_bmac_read_shm(wlc_dngl_ol->wlc_hw, M_ARM_PSO_BLK_PTR) * 2;

	wlc_dngl_ol->max_bsscfg = 1;
	wlc_dngl_ol->bsscfg = (wlc_bsscfg_t *)MALLOCZ(wlc->osh,
		sizeof(wlc_bsscfg_t) * wlc_dngl_ol->max_bsscfg);
	if (wlc_dngl_ol->bsscfg == NULL) {
		WL_ERROR(("allocation failed for bsscfg\n"));
		goto fail;
	}

	wlc_dngl_ol->max_scb = 1;
	wlc_dngl_ol->scb = (scb_t *)MALLOCZ(wlc->osh,
		sizeof(scb_t) * wlc_dngl_ol->max_scb);
	if (wlc_dngl_ol->scb == NULL) {
		WL_ERROR(("allocation failed for scb\n"));
		goto fail;
	}

	wlc_dngl_ol->last_mic_fail_time = -(WPA_TKIP_CM_DETECT + 1);

	return wlc_dngl_ol;

fail:
	ASSERT(FALSE);
	return NULL;
}

static void
wlc_dngl_ol_radio_monitor(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	int radio_hw_disabled;

	if (!wlc_dngl_ol->wowl_cfg.wowl_enabled ||
		!(wlc_dngl_ol->wowl_cfg.wowl_flags & WL_WOWL_ENAB_HWRADIO) ||
		wlc_dngl_ol->pme_asserted)
		return;

	radio_hw_disabled = wlc_bmac_radio_read_hwdisabled(wlc_dngl_ol->wlc_hw);

	if (radio_hw_disabled != wlc_dngl_ol->radio_hw_disabled) {
		WL_ERROR(("%s: HW RADIO is tunrded %s\n",
			__FUNCTION__, radio_hw_disabled? "OFF":"ON"));

		wlc_dngl_ol->radio_hw_disabled = radio_hw_disabled;

		/* Now announce the radio event */
		wlc_dngl_ol_event(
			wlc_dngl_ol,
			BCM_OL_E_RADIO_HW_DISABLED,
			&radio_hw_disabled);
	}
}

void
wlc_dngl_ol_watchdog(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	wlc_dngl_ol_radio_monitor(wlc_dngl_ol);
	wlc_dngl_ol_bcn_watchdog(wlc_dngl_ol->bcn_ol);
	RXOEINCCNTR(wlc_dngl_ol);
}

static void
wlc_dngl_ol_disassoc(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	wlc_dngl_ol->wowl_cfg.associated = FALSE;

	/* put ucode into PM mode by clear & deassert awake bit */
	wlc_dngl_ol->stay_awake = 0;
	wlc_bmac_set_wake_ctrl(wlc_dngl_ol->wlc_hw, FALSE);
}

static void
wlc_dngl_ol_event_handler(wlc_dngl_ol_info_t *wlc_dngl_ol, uint32 event, void *event_data)
{
	uint32 etype = WLC_EL_LAST;
	uint32 edata = 0;
	wowl_host_info_t volatile *pshared = &RXOESHARED(wlc_dngl_ol)->wowl_host_info;

	switch (event) {
		case BCM_OL_E_WOWL_START:
		{
			wowl_cfg_t *wowl_cfg = (wowl_cfg_t *)event_data;

			/*
			 * All OL modules can query WoWL config from the global
			 * wlc_dngl_ol context
			 */
			bcopy(wowl_cfg, &wlc_dngl_ol->wowl_cfg, sizeof(wowl_cfg_t));

			/* Sanitize the setting of WL_WOWL_ENAB_HWRADIO */
			if (wlc_dngl_ol->wowl_cfg.associated &&
				(wlc_dngl_ol->wowl_cfg.wowl_flags & WL_WOWL_ENAB_HWRADIO)) {
				WL_ERROR(("RADIO Button Handling "
					"is supported only for Net Detect feature."));

				ASSERT(0);

				/* Clear this bit for non-debug image */
				wlc_dngl_ol->wowl_cfg.wowl_enabled &= ~WL_WOWL_ENAB_HWRADIO;
			}

			if (wlc_dngl_ol->wowl_cfg.PM == PM_OFF) {
				wlc_dngl_ol->stay_awake |= WAKE_FOR_PMMODE;
			}

			break;
		}

		case BCM_OL_E_WOWL_COMPLETE:
			ASSERT(wlc_dngl_ol->wowl_cfg.wowl_enabled);

			/* Wowl mode should always have TX enabled */
			if (wlc_dngl_ol->TX) {
				/* Send a NULL data frame to tell AP the PS mode */
				wlc_dngl_ol_sendnulldata(wlc_dngl_ol, PRIO_8021D_BE);
			}
			break;

		case BCM_OL_E_TIME_SINCE_BCN:
			if (*((uint32 *)event_data)) {
				WL_ERROR((":LOST BCN\n"));
				wlc_dngl_ol->stay_awake |= WAKE_FOR_UATBTT;
			} else {
				WL_ERROR((":GOT  BCN\n"));
				wlc_dngl_ol->stay_awake &= ~WAKE_FOR_UATBTT;
			}

			/* Update Wake State */
			wlc_bmac_set_wake_ctrl(wlc_dngl_ol->wlc_hw, (wlc_dngl_ol->stay_awake != 0));
			break;

		case BCM_OL_E_BCN_LOSS:
			wlc_dngl_ol_disassoc(wlc_dngl_ol);
			break;
		case BCM_OL_E_DEAUTH:
			etype = WLC_EL_DEAUTH;
			edata = *(uint16 *)event_data;
			wlc_dngl_ol_disassoc(wlc_dngl_ol);
			break;
		case BCM_OL_E_DISASSOC:
			etype = WLC_EL_DISASSOC;
			edata = *(uint16 *)event_data;
			wlc_dngl_ol_disassoc(wlc_dngl_ol);
			break;
		case BCM_OL_E_RETROGRADE_TSF:
			break;
		case BCM_OL_E_RADIO_HW_DISABLED:
			etype = WLC_EL_RADIO_HW_DISABLED;
			edata = *(int *)event_data;
			break;
		case BCM_OL_E_PME_ASSERTED:
			etype = WLC_EL_PME_ASSERTED;
			wlc_dngl_ol->pme_asserted = TRUE;
			break;
		case BCM_OL_E_SCAN_BEGIN:
			etype = WLC_EL_SCAN_BEGIN;
			break;
		case BCM_OL_E_SCAN_END:
			etype = WLC_EL_SCAN_END;
			break;
		case BCM_OL_E_PREFSSID_FOUND:
			etype = WLC_EL_PREFSSID_FOUND;
			break;
		case BCM_OL_E_CSA:
			etype = WLC_EL_CSA;
			edata = *(uint16 *)event_data;
			WL_ERROR(("%s: Receive CSA, move to channel %d\n", __FUNCTION__, edata));
			break;

		default:
			WL_INFORM(("EVENT (%d)\n", event));
			break;
		}
	/* skip current event == previous event */
	if (event != pshared->eventlog[pshared->eventidx]) {
		pshared->eventidx = (pshared->eventidx + 1) & (MAX_OL_EVENTS - 1);
		pshared->eventlog[pshared->eventidx] = event;
	}
	if (etype != WLC_EL_LAST)
		wlc_dngl_ol_eventlog_write(wlc_dngl_ol->eventlog_ol, etype, edata);
}

void
wlc_dngl_ol_event(wlc_dngl_ol_info_t *wlc_dngl_ol, uint32 event, void *event_data)
{
#ifdef TCPKAOE
	wl_tcp_keep_info_t *tcpkeepi;
	wl_icmp_info_t	* icmpi;

#endif // endif
	ASSERT(event < BCM_OL_E_MAX);

	WL_ERROR(("%s: Event type: %s (%d)\n", __FUNCTION__, bcm_ol_event_str[event], event));

	/* NOTE: wlc_dngl_ol_event_handler() must be the first handler called */
	wlc_dngl_ol_event_handler(wlc_dngl_ol, event, event_data);

#ifdef TCPKAOE
	tcpkeepi = (wl_tcp_keep_info_t *)wl_get_tcpkeepi(wlc_dngl_ol->wlc->wl, NULL);
	icmpi = (wl_icmp_info_t	*)wl_get_icmpi(wlc_dngl_ol->wlc->wl, NULL);

	wl_tcp_keepalive_event_handler(tcpkeepi, event, event_data);
	wl_icmp_event_handler(icmpi, event, event_data);
#endif // endif
	wlc_dngl_ol_bcn_event_handler(wlc_dngl_ol, event, event_data);
	wlc_pkt_filter_ol_event_handler(wlc_dngl_ol, event, event_data);

#ifdef SCANOL
	wlc_scanol_event_handler(wlc_dngl_ol, event, event_data);
#endif // endif

#ifdef L2KEEPALIVEOL
	wlc_l2_keepalive_event_handler(wlc_dngl_ol, event, event_data);
#endif // endif

#ifdef MDNS
	wl_mDNS_event_handler(wlc_dngl_ol->mdns_ol, event, event_data);
#endif // endif
	wlc_dngl_ol_eventlog_handler(wlc_dngl_ol->eventlog_ol, event, event_data);
}

static void
wlc_dngl_ol_PSpoll_resp(wlc_dngl_ol_info_t *wlc_dngl_ol, uint16 fc)
{
	/* If there's more, send another PS-Poll */
	if (fc & FC_MOREDATA) {
		if (wlc_dngl_ol_sendpspoll(wlc_dngl_ol) != FALSE)
			return;
		WL_ERROR(("%s: wlc_dngl_ol_sendpspoll() failed\n", __FUNCTION__));
	}

	wlc_dngl_ol_staywake_check(wlc_dngl_ol, FALSE);
}

static void
wlc_dngl_ol_handle_mic_fail(wlc_dngl_ol_info_t *dngl_ol,
	wlc_bsscfg_t *bsscfg, const struct ether_addr *dst,
	wlc_key_t *key, const wlc_key_info_t *key_info)
{
	wlc_info_t *wlc;
	volatile wowl_mic_fail_info_t *fail_info;
	int fail_idx;

	if (key_info->algo != CRYPTO_ALGO_TKIP)
		return;

	wlc = dngl_ol->wlc;
	if (((int)wlc->pub->now - dngl_ol->last_mic_fail_time) <= WPA_TKIP_CM_DETECT)
		fail_idx = 1;
	else
		fail_idx = 0;

	dngl_ol->last_mic_fail_time = wlc->pub->now;
	RXOEMICFAILINFO(dngl_ol)->fail_count = fail_idx  + 1;

	fail_info = &RXOEMICFAILINFO(dngl_ol)->fail_info[fail_idx];
	fail_info->fail_time = wlc->pub->now;
	fail_info->flags = 0;
	if (WLC_KEY_IS_GROUP(key_info))
		fail_info->flags |= WOWL_MIC_FAIL_F_GROUP_KEY;
	if (ETHER_ISMULTI(dst))
		fail_info->flags |= WOWL_MIC_FAIL_F_MULTICAST;
	fail_info->key_id = key_info->key_id;

	/* unless overridden, wake host on two mic failures in 60 seconds */
	if (fail_idx > 0 ||
		(bsscfg->flags & WLC_OL_BSSCFG_F_WAKEON1MICERR)) {
		wlc_wowl_ol_wake_host(dngl_ol->wowl_ol, NULL, 0, NULL, 0, WL_WOWL_MIC_FAIL);
	}
}

static void
wlc_dngl_ol_appendfrag(wlc_info_t *wlc, void *fragbuf, uint *fragresid,
	uchar *body, uint body_len, void *osh)
{
	uchar *dst;
	uint fraglen;

	/* append frag payload to end of partial packet */
	fraglen = PKTLEN(osh, fragbuf);
	dst = PKTDATA(osh, fragbuf) + fraglen;
	bcopy(body, dst, body_len);
	PKTSETLEN(osh, fragbuf, (fraglen + body_len));
	*fragresid -= body_len;
}

static int
wlc_dngl_ol_parse_hdr(wlc_dngl_ol_info_t *wlc_dngl_ol, osl_t *osh, void **pp,
	d11rxhdr_t *rxh, bool *snap)
{
	struct dot11_header *h;
	struct ether_addr a1, a2, a3, a4;
	struct dot11_llc_snap_header *lsh;
	uint len, min_len;
	bool wds, qos;
	uchar *pbody;
	uint16 type, subtype;
	uint16 qoscontrol;
	uint8 prio = 0;
	d11regs_t *d11_regs;
	d11_regs = (d11regs_t *)wlc_dngl_ol->regs;
	wlc_frminfo_t f;	/* frame info to be passed to intermediate functions */
	wlc_dngl_ol_pkt_filter_info_t *pkt_filter_ol = wlc_dngl_ol->pkt_filter_ol;
	bool more_frag;
	uint16 prev_seqctl = 0;
	uint16 fc;
	wlc_bsscfg_t *bsscfg;
	scb_t *scb;
	void *p = *pp;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG_ERR */

	bsscfg = &wlc_dngl_ol->bsscfg[0];
	scb = &wlc_dngl_ol->scb[0];

	bzero((void *)&f, sizeof(wlc_frminfo_t));
	f.p = p;
	f.ismulti = FALSE;
	f.rxh = rxh;

	*snap = FALSE;

	f.h = h = (struct dot11_header *)(PKTDATA(osh, p));

	len = PKTLEN(osh, p);
	f.len = len;
	f.seq = ltoh16(f.h->seq);

	min_len = DOT11_A3_HDR_LEN;
	if ((len < min_len) ||
		((FC_TYPE(h->fc) == FC_TYPE_DATA) && ETHER_ISNULLADDR(&h->a2)))  {
		WL_ERROR(("Bad data frame\n"));
		goto err;
	}

	if ((rxh->RxStatus1 & RXS_FCSERR) != 0)
		return -1;
	ASSERT((rxh->RxStatus1 & RXS_FCSERR) == 0);

	fc = ltoh16(h->fc);
	f.fc = fc;
	wds = ((fc & (FC_TODS | FC_FROMDS)) == (FC_TODS | FC_FROMDS));
	type = FC_TYPE(fc);
	subtype = (fc & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT;
	qos = (type == FC_TYPE_DATA && FC_SUBTYPE_ANY_QOS(subtype));
	more_frag = ((fc & FC_MOREFRAG) != 0);
	f.rx_wep = f.fc & FC_WEP;

	/* check for a broadcast/multicast A1 */
	f.ismulti = ETHER_ISMULTI(&(h->a1));

	/* Check for a PS-Poll response */
	if ((wlc_dngl_ol->stay_awake & WAKE_FOR_PSPOLL)&&	/* PS-Poll outstanding */
		!f.ismulti &&	/* unicast */
		!more_frag ) {	/* last frag in burst, or non-frag frame */
		wlc_dngl_ol_PSpoll_resp(wlc_dngl_ol, fc);
	}

	if (wds)
		min_len += ETHER_ADDR_LEN;

	if (len < min_len) {
		WL_TRACE(("Pkt length %d less than min len %d\n", len, min_len));
		goto err;
	}

	if (((fc & FC_KIND_MASK) == FC_NULL_DATA) ||
		((fc & FC_KIND_MASK) == FC_QOS_NULL)) {
		/* Return on QOS_NULL or NULL_DATA */
#ifdef L2KEEPALIVEOL
		if (wlc_dngl_ol->wowl_cfg.wowl_enabled &&
			(wlc_l2_keepalive_get_flags(wlc_dngl_ol) &
			BCM_OL_KEEPALIVE_RX_SILENT_DISCARD)) {
			WL_ERROR(("wowl mode deleting NULL/QOS-NULL frame\n"));
			return -1;
		}
		else
#endif // endif
		return 0;
	}

	/* Copy the received 802.11 header in case packet needs to be sent to the host */
	if (pkt_filter_ol != NULL)
		wlc_pkt_filter_ol_save_header(pkt_filter_ol, p);

	pbody = (uchar*)h + min_len;

	if (qos) {
		min_len += DOT11_QOS_LEN;
		qoscontrol = ltoh16_ua(pbody);
		prio = (uint8)QOS_PRIO(qoscontrol);
		pbody += DOT11_QOS_LEN;
	}

	f.prio = prio;

	/* Detect and discard duplicates */
	if (!f.ismulti) {
		if ((f.fc & FC_RETRY) && (scb->seqctl[prio] == f.seq)) {
			WL_ERROR(("%s discarding duplicate packet\n", __FUNCTION__));
			goto err;
		} else {
			prev_seqctl = scb->seqctl[prio];
			scb->seqctl[prio] = f.seq;
		}
	}

	f.pbody = pbody;
	f.body_len = f.len - min_len;
	f.totlen = pkttotlen(osh, p) - min_len;
	if (f.rx_wep || (WSEC_ENABLED(bsscfg->wsec) && bsscfg->wsec_restrict)) {
		WLPKTTAGSCBSET(f.p, scb);
		if (wlc_keymgmt_recvdata(DNGL_OL_KEYMGMT(wlc_dngl_ol), &f) != BCME_OK)
			goto err;
		pbody = f.pbody;
	}

	min_len = pbody - (uint8 *)f.h;

	/* frag reassembly */
	if (!f.ismulti) {
		if ((f.seq & FRAGNUM_MASK) == 0) { /* new frag */
			if (scb->fragbuf[prio]) {
				/* Discard old partially-received fragment sequence */
				WL_ERROR(("%s: discarding partial fragment\n",
					__FUNCTION__));
				PKTFREE(osh, scb->fragbuf[prio], FALSE);
				scb->fragbuf[prio] = NULL;
				scb->fragresid[prio] = 0;
				scb->fragtimestamp[prio] = 0;
			}

			if (more_frag) {
				int pkt_len = f.len;

				/* map the contents of 1st frag */
				PKTCTFMAP(osh, f.p);

				scb->fragbuf[prio] = PKTGET(osh,
					wlc_dngl_ol->wlc->pub->tunables->rxbufsz, FALSE);

				if (scb->fragbuf[prio] == NULL) {
					WL_ERROR(("%s(): Allocate %d rxbuf for"
						" fragment pkt failed!\n",
						__FUNCTION__,
						(int)wlc_dngl_ol->wlc->pub->tunables->rxbufsz));
					goto err;
				}

				memcpy(PKTDATA(osh, scb->fragbuf[prio]),
					(PKTDATA(osh, f.p) - PKTHEADROOM(osh, f.p)),
					(PKTHEADROOM(osh, f.p) + PKTLEN(osh, f.p)));
				PKTPULL(osh, scb->fragbuf[prio],
					(int)PKTHEADROOM(osh, f.p));
				PKTSETLEN(osh, scb->fragbuf[prio], PKTLEN(osh, f.p));

				scb->fragresid[prio] =
					(wlc_dngl_ol->wlc->pub->tunables->rxbufsz -
					wlc_dngl_ol->wlc->hwrxoff - D11_PHY_HDR_LEN) - pkt_len;

				PKTSETLEN(osh, scb->fragbuf[prio], pkt_len);

				/* more frags, tell caller not to process packet */
				return -1;
			}
		} else { /* subsequent frag */
			/*
			 * This isn't the first frag, but we don't have a partially-
			 * received MSDU.  We must have somehow missed the previous
			 * frags or timed-out the partially-received MSDU (not implemented yet).
			 */
			if (!scb->fragbuf[prio]) {
				WL_ERROR(("%s: discarding fragment %04x with "
					"prio %d; previous fragments missed or partially-received "
					"fragments timed-out\n", __FUNCTION__,
					f.seq, prio));
				WLCNTINCR(wlc_dngl_ol->wlc->pub->_cnt->rxfragerr);
				goto err;
			}

			/* Make sure this MPDU:
			 * - matches the partially-received MSDU
			 * - is the one we expect (next in sequence)
			 */
			if (((f.seq & ~FRAGNUM_MASK) != (prev_seqctl & ~FRAGNUM_MASK)) ||
			    ((f.seq & FRAGNUM_MASK) != ((prev_seqctl & FRAGNUM_MASK) + 1))) {
				/* discard the partially-received MSDU */
				WL_ERROR(("%s: discarding partial "
					"fragment %03x with prio:%d received from %s\n",
					__FUNCTION__,
					prev_seqctl >> SEQNUM_SHIFT, prio,
					bcm_ether_ntoa(&(f.h->a2), eabuf)));
				PKTFREE(osh, scb->fragbuf[prio], FALSE);
				scb->fragbuf[prio] = NULL;
				scb->fragresid[prio] = 0;
				scb->fragtimestamp[prio] = 0;

				/* discard the MPDU */
				WL_INFORM(("%s: discarding fragment %04x with "
					"prio %d; previous fragments missed\n",
					__FUNCTION__, f.seq, prio));
				WLCNTINCR(wlc_dngl_ol->wlc->pub->_cnt->rxfragerr);
				goto err;
			}

			/* detect fragbuf overflow */

			if (f.body_len > scb->fragresid[prio]) {
				/* discard the partially-received MSDU */
				WL_ERROR(("%s: discarding partial "
					"fragment %03x with prio %d received from %s\n",
					__FUNCTION__,
					prev_seqctl >> SEQNUM_SHIFT, prio,
					bcm_ether_ntoa(&(f.h->a2), eabuf)));
				PKTFREE(osh, scb->fragbuf[prio], FALSE);
				scb->fragbuf[prio] = NULL;
				scb->fragresid[prio] = 0;
				scb->fragtimestamp[prio] = 0;

				/* discard the MPDU */
				WL_ERROR(("%s: discarding fragment %04x "
					"with prio %d; resulting fragment too big\n",
					__FUNCTION__, f.seq, prio));
				WLCNTINCR(wlc_dngl_ol->wlc->pub->_cnt->rxfragerr);
				goto err;
			}

			/* map the contents of each subsequent frag before copying */
			PKTCTFMAP(osh, f.p);

			/* copy frame into fragbuf */
			wlc_dngl_ol_appendfrag(wlc_dngl_ol->wlc, scb->fragbuf[prio],
				&scb->fragresid[prio], f.pbody, f.body_len, osh);

			if (!more_frag) {
				/* last frag...fall through and sendup reassembled MSDU */

				PKTFREE(osh, p, FALSE);

				*pp = p = f.p = scb->fragbuf[prio];

				scb->fragbuf[prio] = NULL;
				scb->fragresid[prio] = 0;
				scb->fragtimestamp[prio] = 0;

				/* reset packet pointers to beginning */
				f.h = (struct dot11_header *) PKTDATA(osh, f.p);
				f.len = PKTLEN(osh, f.p);
				f.pbody = (uchar*)(f.h) + DOT11_A3_HDR_LEN;
				f.body_len = f.len - DOT11_A3_HDR_LEN;

				if (wds) {
					f.pbody += ETHER_ADDR_LEN;
					f.body_len -= ETHER_ADDR_LEN;
				}

				/* WME: account for QoS Control Field */
				if (qos) {
					f.prio = prio;
					f.pbody += DOT11_QOS_LEN;
					f.body_len -= DOT11_QOS_LEN;
				}

				f.fc = ltoh16(f.h->fc);
				if ((f.rx_wep) && (f.key)) {
					/* strip WEP IV */
					f.pbody += f.key_info.iv_len;
					f.body_len -= f.key_info.iv_len;
				}
			} else {
				/* more frags, tell caller not to process this packet */
				return -1;
			}
		}
	}

	PKTSETPRIO(f.p, prio);

	/*
	 * 802.11 -> 802.3/Ethernet header conversion
	 * Part 1: eliminate possible overwrite problems, find right eh pointer
	 */
	bcopy((char *)&(f.h->a1), (char *)&a1, ETHER_ADDR_LEN);
	bcopy((char *)&(f.h->a2), (char *)&a2, ETHER_ADDR_LEN);
	bcopy((char *)&(f.h->a3), (char *)&a3, ETHER_ADDR_LEN);
	if (wds)
		bcopy((char *)&(f.h->a4), (char *)&a4, ETHER_ADDR_LEN);

	/*
	 * 802.11 -> 802.3/Ethernet header conversion
	 * Part 2: find sa/da pointers
	 */
	if ((f.fc & FC_TODS) == 0) {
		f.da = &a1;
		if ((f.fc & FC_FROMDS) == 0)
			f.sa = &a2;
		else
			f.sa = &a3;
	} else {
		f.da = &a3;
		if ((f.fc & FC_FROMDS) == 0)
			f.sa = &a2;
		else
			f.sa = &a4;
	}

	if (f.key != NULL && f.rx_wep) {
		int err;
		struct ether_header eh, *ehp;

		bcopy(pbody - sizeof(eh), &eh, sizeof(eh));
		ehp = (struct ether_header*)PKTPULL(osh, f.p, min_len - ETHER_HDR_LEN);
		eacopy((char*)(f.da), ehp->ether_dhost);
		eacopy((char*)(f.sa), ehp->ether_shost);
		ehp->ether_type = hton16((uint16)f.body_len);

		err = wlc_key_rx_msdu(f.key, f.p, f.rxh);
		if (err != BCME_OK) {
			if (err == BCME_MICERR) {
				wlc_dngl_ol_handle_mic_fail(wlc_dngl_ol, bsscfg,
					f.da, f.key, &f.key_info);
			}
			goto err;
		}

		bcopy(&eh, ehp, sizeof(eh));
		PKTPUSH(osh, f.p, min_len - ETHER_HDR_LEN);
	}

	lsh = (struct dot11_llc_snap_header *)pbody;
	if (lsh->dsap == 0xaa && lsh->ssap == 0xaa && lsh->ctl == 0x03 &&
		lsh->oui[0] == 0 && lsh->oui[1] == 0 && lsh->oui[2] == 0x00) {
			 pbody += SNAP_HDR_LEN;
			 min_len += SNAP_HDR_LEN;
			*snap = TRUE;
	}

	f.eh = (struct ether_header *)PKTPULL(osh, f.p, min_len - ETHER_TYPE_OFFSET);
	if (!(*snap))
		f.eh->ether_type = hton16((uint16)f.body_len);	/* length */
	/*
	 * 802.11 -> 802.3/Ethernet header conversion
	 * Part 3: do the conversion
	 */
	bcopy((char*)(f.da), (char*)&(f.eh->ether_dhost), ETHER_ADDR_LEN);
	bcopy((char*)(f.sa), (char*)&(f.eh->ether_shost), ETHER_ADDR_LEN);

#ifdef GTKOL
	if ((wlc_dngl_ol->wowl_cfg.wowl_enabled == TRUE) &&
		wlc_dngl_ol->ol_gtk && (f.key_info.algo != CRYPTO_ALGO_OFF)) {
		if (ntoh16(f.eh->ether_type) == ETHER_TYPE_802_1X) {
			if (!wlc_dngl_ol_eapol(wlc_dngl_ol->ol_gtk,
			(eapol_header_t*) f.eh, (f.rx_wep != 0))) {
				goto err;
			}
		}
	}
#endif // endif

	return 0;

err:
	return -1;
}

void wlc_dngl_ol_push_to_host(wlc_info_t *wlc)
{
	wlc_dngl_ol_info_t *wlc_dngl_ol = wlc->wlc_dngl_ol;
	d11regs_t *d11_regs = (d11regs_t *)wlc_dngl_ol->regs;

	AND_REG(wlc_dngl_ol->osh, &d11_regs->u.d11acregs.PSOCtl, ~PSO_MODE);
}

bool wlc_dngl_ol_supr_frame(wlc_info_t *wlc, uint16 frame_ptr)
{
	wlc_dngl_ol_info_t *wlc_dngl_ol = wlc->wlc_dngl_ol;
	int supr_status1 = 0;
	int supr_status2 = 0;

	/* Cannot suppress in wowl mode */
	if (wlc_dngl_ol->wowl_cfg.wowl_enabled == TRUE)
		return FALSE;

	supr_status1 = wlc_bmac_read_shm(wlc_dngl_ol->wlc_hw,
		(wlc_dngl_ol->pso_blk + 4));
	supr_status2 = wlc_bmac_read_shm(wlc_dngl_ol->wlc_hw,
		(wlc_dngl_ol->pso_blk + 6));
	if (supr_status1 == SUPR_FRAME) {
		wlc_bmac_write_shm(wlc_dngl_ol->wlc_hw,
		(wlc_dngl_ol->pso_blk + 4), frame_ptr);	/* M_RXF0_SUPR_PTRS[0] */
		return TRUE;
	}
	else if (supr_status2 == SUPR_FRAME) {
		wlc_bmac_write_shm(wlc_dngl_ol->wlc_hw,
		(wlc_dngl_ol->pso_blk + 6), frame_ptr); /* M_RXF0_SUPR_PTRS[1] */
		return TRUE;
	}
	else
		WL_ERROR(("Delete failed - frame_ptr:0x%x \n", frame_ptr));

	return FALSE;
}

void*
wlc_dngl_ol_frame_get_ctl(wlc_dngl_ol_info_t *wlc_dngl_ol, uint len)
{
	void *p;
	osl_t *osh;

	ASSERT(len != 0);

	osh = wlc_dngl_ol->osh;
	if ((p = PKTGET(osh, (TXOFF + len), TRUE)) == NULL) {
		WL_ERROR(("%s: pktget error for len %d\n",
			__FUNCTION__, ((int)TXOFF + len)));
		return (NULL);
	}
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, len);

	PKTSETPRIO(p, 0);

	return (p);
}

void *
wlc_dngl_ol_frame_get_ps_ctl(wlc_dngl_ol_info_t *wlc_dngl_ol, const struct ether_addr *bssid,
	const struct ether_addr *sa)
{
	void *p;
	struct dot11_ps_poll_frame *hdr;

	if ((p = wlc_dngl_ol_frame_get_ctl(wlc_dngl_ol, DOT11_PS_POLL_LEN)) == NULL) {
		return (NULL);
	}

	/* construct a PS-Poll frame */
	hdr = (struct dot11_ps_poll_frame *)PKTDATA(wlc_dngl_ol->osh, p);
	hdr->fc = htol16(FC_PS_POLL);
	hdr->durid = htol16(RXOETXINFO(wlc_dngl_ol)->aid);
	bcopy((const char*)bssid, (char*)&hdr->bssid, ETHER_ADDR_LEN);
	bcopy((const char*)sa, (char*)&hdr->ta, ETHER_ADDR_LEN);

	return (p);
}

void
wlc_dngl_ol_staywake_check(wlc_dngl_ol_info_t *wlc_dngl_ol, bool tim_set)
{
	/* This function is call in WoWL mode if AID in TIM IE is NOT set */
	if ((wlc_dngl_ol->stay_awake & WAKE_FOR_PSPOLL) == 0 || tim_set)
		return;

	ASSERT(wlc_dngl_ol->wowl_cfg.wowl_enabled);

	/* if WAKE_FOR_PSPOLL is set clear it due to TIM bit is not set */
	wlc_dngl_ol->stay_awake &= ~WAKE_FOR_PSPOLL;

	/* no more wake bits are set then go back to sleep */
	if (wlc_dngl_ol->stay_awake == 0)
		wlc_bmac_set_wake_ctrl(wlc_dngl_ol->wlc_hw, FALSE);
}

bool
wlc_dngl_ol_sendpspoll(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	void *pkt;
	ratespec_t rspec;
	uint16 frameid = 0;
	int fifo = TX_ATIM_FIFO;
	volatile ol_tx_info *txinfo;

	txinfo = RXOETXINFO(wlc_dngl_ol);
	pkt = wlc_dngl_ol_frame_get_ps_ctl(wlc_dngl_ol,
		(const struct ether_addr *)&txinfo->BSSID,
		(const struct ether_addr *)&txinfo->cur_etheraddr);
	if (pkt == NULL) {
		WL_ERROR(("%s: wlc_frame_get_ps_ctl failed\n", __FUNCTION__));
		return FALSE;
	}

	wlc_dngl_ol->stay_awake |= WAKE_FOR_PSPOLL;

	/* Force d11 to wake */
	if (wlc_dngl_ol->stay_awake) {
		wlc_bmac_set_wake_ctrl(wlc_dngl_ol->wlc_hw, TRUE);
	}

	rspec = txinfo->rate;
	frameid = wlc_dngl_ol_d11hdrs(wlc_dngl_ol, pkt, rspec, NULL, NULL, NULL, fifo);

	wlc_bmac_txfifo(wlc_dngl_ol->wlc_hw, fifo, pkt, TRUE, frameid, 1);
	RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.pspoll);
	RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.tottxpkt);

	return TRUE;
}

static void *
wlc_dngl_ol_frame_get_mgmt(wlc_dngl_ol_info_t *wlc_dngl_ol, uint16 fc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid, uint body_len,
	uint8 **pbody)
{
	uint len;
	void *p = NULL;
	osl_t *osh;
	struct dot11_management_header *hdr;

	osh = wlc_dngl_ol->osh;

	len = DOT11_MGMT_HDR_LEN + body_len;

	if ((p = PKTGET(osh, (TXOFF + len), TRUE)) == NULL) {
		WL_ERROR(("wlc_frame_get_mgmt: pktget error for len %d fc %x\n",
		           ((int)TXOFF + len), fc));
		return NULL;
	}

	ASSERT(ISALIGNED((uintptr)PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, len);

	/* construct a management frame */
	hdr = (struct dot11_management_header *)PKTDATA(osh, p);
	hdr->fc = htol16(fc);
	hdr->durid = 0;
	bcopy((const char*)da, (char*)&hdr->da, ETHER_ADDR_LEN);
	bcopy((const char*)sa, (char*)&hdr->sa, ETHER_ADDR_LEN);
	bcopy((const char*)bssid, (char*)&hdr->bssid, ETHER_ADDR_LEN);
	hdr->seq = 0;

	*pbody = (uint8*)&hdr[1];

	/* Set Prio for MGMT packets */
	PKTSETPRIO(p, MAXPRIO);

	return (p);
}

/*
 * For non-WMM association: sends a Null Data frame.
 *
 * For WMM association: if prio is -1, sends a Null Data frame;
 * otherwise sends a QoS Null frame with priority prio.
 */
static void *
wlc_dngl_ol_alloc_nulldata(wlc_dngl_ol_info_t *wlc_dngl_ol, int prio)
{
	void *p;
	uint8 *pbody;
	uint16 fc;
	int qos, body_len;
	struct ether_addr *bssid;

	if (prio < 0) {
		qos = FALSE;
		prio = PRIO_8021D_BE;
	}
	else
		qos = SCB_QOS(&wlc_dngl_ol->scb[0]);

	if (qos) {
		fc = FC_QOS_NULL;
		body_len = 2;
	} else {
		fc = FC_NULL_DATA;
		body_len = 0;
	}

	fc |= FC_TODS;

	bssid = (struct ether_addr *) &RXOETXINFO(wlc_dngl_ol)->BSSID;

	if ((p = wlc_dngl_ol_frame_get_mgmt(wlc_dngl_ol, fc,
		bssid, (struct ether_addr *)&RXOETXINFO(wlc_dngl_ol)->cur_etheraddr,
		bssid, body_len, &pbody)) == NULL)
		return p;

#ifdef L2KEEPALIVEOL
	/* WLF_NULLPKT reused to identify NULL pkt */
	if (wlc_dngl_ol->wowl_cfg.wowl_enabled == TRUE)
		WLPKTTAG(p)->flags |= WLF_NULLPKT;
#endif // endif
	PKTSETPRIO(p, prio);

	if (qos) {
		uint16 *pqos;

		/* Initialize the QoS Control field that comes after all addresses. */
		pqos = (uint16 *)((uint8 *)PKTDATA(wlc_dngl_ol->osh, p) +
			DOT11_MGMT_HDR_LEN + body_len - DOT11_QOS_LEN);
		ASSERT(ISALIGNED(pqos, sizeof(*pqos)));
		*pqos = htol16(((prio << QOS_PRIO_SHIFT) & QOS_PRIO_MASK));

	}

	return p;
}

void *
wlc_dngl_ol_sendnulldata(wlc_dngl_ol_info_t *wlc_dngl_ol, int prio)
{
	void *pkt;
	ratespec_t rspec;
	uint16 frameid = 0;
	int fifo = TX_ATIM_FIFO;

	ENTER();

	if (wlc_dngl_ol->pme_asserted || wlc_dngl_ol->radio_hw_disabled) {
		return NULL;
	}

	if (wlc_dngl_ol->wowl_cfg.wowl_enabled &&
		(!wlc_dngl_ol->wowl_cfg.associated)) {
		WL_ERROR(("%s: About NULL Data in Un-associated mode.\n", __FUNCTION__));
		return NULL;
	}

	if ((pkt = wlc_dngl_ol_alloc_nulldata(wlc_dngl_ol, prio)) == NULL) {
		WL_ERROR(("%s:Pkt allocation failed\n", __FUNCTION__));
		return pkt;
	}

	rspec = RXOETXINFO(wlc_dngl_ol)->rate;
	frameid = wlc_dngl_ol_d11hdrs(wlc_dngl_ol, pkt, rspec, NULL, NULL, NULL, fifo);

	wlc_bmac_txfifo(wlc_dngl_ol->wlc_hw, fifo, pkt, TRUE, frameid, 1);
	RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.nullfrm);
	RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.tottxpkt);

	EXIT();

	return NULL;
}

static void
wlc_dngl_ol_recvdata(wlc_dngl_ol_info_t *wlc_dngl_ol, osl_t *osh, void **pp, d11rxhdr_t *rxh)
{
	int ret;
	bool snap;
	void *p = *pp;

	PKTPULL(osh, p, D11_PHY_HDR_LEN);
	PKTSETLEN(osh, p, PKTLEN(osh, p) - DOT11_FCS_LEN);

	/* TBD: put check for arp/nd/pktfilter enable */
	/* Retrieve ether type */
	ret = wlc_dngl_ol_parse_hdr(wlc_dngl_ol, osh, pp, rxh, &snap);
	if (ret == -1) {
		if (!(wlc_dngl_ol->wowl_cfg.wowl_enabled)) {
			wlc_dngl_ol_push_to_host(wlc_dngl_ol->wlc);
			WL_ERROR(("Data pkt error: Clearing deferral\n"));
		}
		return;
	}

	/* parse_hdr may substitute a reassembled fragment sequence for the */
	/* packet if this is the final fragment */
	p = *pp;

	WLPKTTAG(p)->frameptr = ((rxh->RxStatus2 & RXS_FRAMEPTR) >> 9);
#ifdef MDNS
	mdns_rx(wlc_dngl_ol->mdns_ol, (uchar *)PKTDATA(osh, p), PKTLEN(osh, p));
#endif // endif

	wl_sendup(wlc_dngl_ol->wlc->wl, NULL, p, 1);

#ifdef LTROL
	if (LTR_ENAB(wlc_dngl_ol)) {
		wlc_dngl_ol_ltr_update(wlc_dngl_ol, osh);
	}
#endif /* LTROL */
}

static void
wlc_dngl_ol_recvctl(wlc_dngl_ol_info_t *wlc_dngl_ol, osl_t *osh, void *p, wlc_d11rxhdr_t *wrxh)
{
	uint16 fc;
	struct dot11_management_header *h;
	uint16 ft, fk;
	bool supr_frame = 0;
	d11regs_t *d11_regs;

	d11_regs = (d11regs_t *)wlc_dngl_ol->regs;

	h = (struct dot11_management_header *)(PKTDATA(osh, p) + D11_PHY_HDR_LEN);
	fc = ltoh16(h->fc);
	ft = FC_TYPE(fc);
	fk = (fc & FC_KIND_MASK);

	if (wlc_dngl_ol->wowl_cfg.wowl_enabled &&
		((fk == FC_DEAUTH) || (fk == FC_DISASSOC))) {
		uint8 *body;
		int body_len;
		bool htc = FALSE;
		uint32 event_type = BCM_OL_E_MAX;
		uint16 reason = DOT11_RC_UNSPECIFIED;

		htc = ((wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) &&
		      (fc & FC_ORDER) && (ft == FC_TYPE_MNG);
		PKTPULL(osh, p, D11_PHY_HDR_LEN);
		if (htc)
			PKTPULL(osh, p, DOT11_HTC_LEN);
		/* set pointer to frame body and set frame body length */
		PKTPULL(osh, p, sizeof(struct dot11_management_header));
		body  = PKTDATA(osh, p);
		body_len = PKTLEN(osh, p) - DOT11_FCS_LEN;	/* w/o 4 bytes CRC */

		ASSERT(body_len >= 2);
		if (body_len < 2) {
			WL_ERROR(("%s: Bad proto packet\n", __FUNCTION__));
			return;
		}
#ifdef MFP
		/* whether frame kind is relevant to MFP */
		/* Decrypt Deauth and Disassoc (checked above) but not Action */
		/* Action frames received in sleep are discarded */
		if (!wlc_dngl_ol_mfp_rx(wlc_dngl_ol, &wrxh->rxhdr, h, p))
			return;
#endif /* MFP */
		reason = ltoh16(*(uint16*)body);

		if (fk == FC_DEAUTH) {
			WL_ERROR(("**DEAUTH Reason %d!!!!**\n", reason));
			event_type = BCM_OL_E_DEAUTH;
		} else if (fk == FC_DISASSOC) {
			WL_ERROR(("**DISASSOC Reason %d!!!**\n", reason));
			event_type = BCM_OL_E_DISASSOC;
		}

		ASSERT(event_type != BCM_OL_E_MAX);
		if (event_type == BCM_OL_E_MAX) {
			WL_ERROR(("%s: Bad Event Type\n", __FUNCTION__));
			return;
		}

		if (wlc_dngl_ol->wowl_cfg.wowl_flags & WL_WOWL_DIS) {
			/* Wake up the host */
			wlc_wowl_ol_wake_host(wlc_dngl_ol->wowl_ol, NULL, 0, NULL, 0, WL_WOWL_DIS);
		} else {
			/* Announce the event */
			wlc_dngl_ol_event(wlc_dngl_ol, event_type, &reason);
		}
	} else if (fk == FC_BEACON) {
		if (wlc_dngl_ol->bcn_ol) {
			supr_frame = wlc_dngl_ol_bcn_process(wlc_dngl_ol->bcn_ol, p, h);
			WLPKTTAG(p)->frameptr = (wrxh->rxhdr.RxStatus2 & RXS_FRAMEPTR) >> 9;
			if (supr_frame && wlc_dngl_ol_bcn_delete(wlc_dngl_ol->bcn_ol)) {
				if (wlc_dngl_ol_supr_frame(wlc_dngl_ol->wlc, WLPKTTAG(p)->frameptr))
					RXOEINC(wlc_dngl_ol, rxoe_bcndelcnt);
			}
		}
	} else {
		if (!wlc_dngl_ol->wowl_cfg.wowl_enabled) {
			WL_ERROR(("Unhandled Mgmt frame to ARM in wake mode\n"));
			wlc_dngl_ol_push_to_host(wlc_dngl_ol->wlc);
		}
	}
}

void
wlc_dngl_ol_recv(wlc_dngl_ol_info_t *wlc_dngl_ol, void *p)
{
	struct dot11_header *h;
	uint16 fc, ft, fk;
	uint len;
	wlc_d11rxhdr_t *wrxh;
	d11rxhdr_t *rxh;
	bool rxstatus_rxchan = FALSE;

	RXOEINC(wlc_dngl_ol, rxoe_rxpktcnt.totrxpkt);
	if (wlc_dngl_ol->pme_asserted || wlc_dngl_ol->radio_hw_disabled) {
		return;
	}

#ifdef BCMDBG_DNGL_OL
	/* in sleep mode (PM1), HPS should always be set
	 * check and verify HPS bit on every beacon received
	 */
	if (wlc_dngl_ol->wowl_cfg.wowl_enabled && wlc_dngl_ol->wowl_cfg.PM == PM_MAX) {
		uint32 maccontrol;
		wlc_hw_info_t *wlc_hw = wlc_dngl_ol->wlc_hw;
		maccontrol = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
		if ((maccontrol & MCTL_HPS) == 0) {
			WL_INFORM(("%s: HPS is NOT set 0x%x\n", __FUNCTION__, maccontrol));
		}
	}
#endif /* BCMDBG_DNGL_OL */

	/* frame starts with rxhdr */
	wrxh = (wlc_d11rxhdr_t *)PKTDATA(wlc_dngl_ol->osh, p);
	rxh = &wrxh->rxhdr;
	ASSERT(CHSPEC_CHANNEL(wrxh->rxhdr.RxChan));

	if (!rxstatus_rxchan && (rxh->RxStatus1 & RXS_FCSERR) != 0) {
		WL_INFORM(("%s: Toss bad FCS packet 0x%x\n", __FUNCTION__, rxh->RxStatus1));
		RXOEINC(wlc_dngl_ol, rxoe_rxpktcnt.badfcs);
		goto exit;
	}

	PKTPULL(wlc_dngl_ol->osh, p, OL_HWRXOFF);

	/* MAC inserts 2 pad bytes for a4 headers or QoS or A-MSDU subframes */
	if (rxstatus_rxchan || rxh->RxStatus1 & RXS_PBPRES) {
		if (PKTLEN(wlc_dngl_ol->osh, p) < 2) {
			WL_ERROR(("Pkt length less than 2 %d\n", PKTLEN(wlc_dngl_ol->osh, p)));
			return;
		}
		PKTPULL(wlc_dngl_ol->osh, p, 2);
	}

	h = (struct dot11_header *)(PKTDATA(wlc_dngl_ol->osh, p) + D11_PHY_HDR_LEN);
	len = PKTLEN(wlc_dngl_ol->osh, p);
	if (len >= D11_PHY_HDR_LEN + sizeof(h->fc)) {
		fc = ltoh16(h->fc);
		ft = FC_TYPE(fc);
		fk = (fc & FC_KIND_MASK);
		if (ft == FC_TYPE_MNG) {
			RXOEINC(wlc_dngl_ol, rxoe_rxpktcnt.mgmtfrm);
			if (wlc_macol_rxpkt_consumed(wlc_dngl_ol->wlc_hw, p, wrxh))
				RXOEINC(wlc_dngl_ol, rxoe_rxpktcnt.scanprocessfrm);
			else
				wlc_dngl_ol_recvctl(wlc_dngl_ol, wlc_dngl_ol->osh, p, wrxh);
		}
		if (ft == FC_TYPE_DATA) {
			RXOEINC(wlc_dngl_ol, rxoe_rxpktcnt.datafrm);
			if (wlc_dngl_ol->wowl_cfg.wowl_enabled &&
			    (!wlc_dngl_ol->wowl_cfg.associated)) {
				WL_ERROR(("%s:Drop data frame in unassociated wowl mode.\n",
					__FUNCTION__));
				RXOEINC(wlc_dngl_ol, rxoe_rxpktcnt.unassfrmdrop);
				goto exit;
			}
			wlc_dngl_ol_recvdata(wlc_dngl_ol, wlc_dngl_ol->osh, &p, rxh);
		}
	} else
		RXOEINC(wlc_dngl_ol, rxoe_rxpktcnt.badfrmlen);
exit:
	PKTFREE(wlc_dngl_ol->osh, p, FALSE);
}

static void
wlc_dngl_ol_ether_8023hdr(wlc_dngl_ol_info_t *wlc_dngl_ol,
	osl_t *osh, struct ether_header *eh, void *p)
{
	struct ether_header *neh;
	struct dot11_llc_snap_header *lsh;
	uint16 plen, ether_type;

	ether_type = ntoh16(eh->ether_type);
	neh = (struct ether_header *)PKTPUSH(osh, p, DOT11_LLC_SNAP_HDR_LEN);

	/* 802.3 MAC header */

	bcopy((char*)eh->ether_dhost, (char*)neh->ether_dhost, ETHER_ADDR_LEN);

	/* force the source ethernet address as ours,
	 * irrespective of what came in header
	 */
	bcopy((char*)&wlc_dngl_ol->cur_etheraddr, (char*)neh->ether_shost, ETHER_ADDR_LEN);
	plen = (uint16)pkttotlen(osh, p) - ETHER_HDR_LEN;
	neh->ether_type = hton16(plen);
	/* 802.2 LLC header */
	lsh = (struct dot11_llc_snap_header *)&neh[1];
	lsh->dsap = 0xaa;
	lsh->ssap = 0xaa;
	lsh->ctl = 0x03;

	/* 802.2 SNAP header Use RFC1042 or bridge-tunnel if type in SST per 802.1H */
	lsh->oui[0] = 0x00;
	lsh->oui[1] = 0x00;

	/*
	 * XXX - Eventually, we need a real selective translation table with
	 *       ioctls for adding and deleting entries and initialization for
	 *		 the protocols required by WECA.  For now, just hardwire them:
	 *		 - 0x80f3: Apple AARP
	 *		 - 0x8137: Novell "Raw"
	 */
	if (ether_type == 0x80f3 || ether_type == 0x8137)
		lsh->oui[2] = 0xf8;
	else
		lsh->oui[2] = 0x00;
	lsh->type = hton16(ether_type);
}

static bool
wlc_dngl_ol_80211hdr(wlc_dngl_ol_info_t *wlc_dngl_ol, void *pkt)
{
	osl_t *osh;
	osh = wlc_dngl_ol->osh;
	struct ether_header *eh;
	struct ether_header temp;
	struct ether_addr *dst;
	ratespec_t rate;
	uint16 frameid = 0;
	uint16 offset;
	struct dot11_header *h;
	uint16 fc = 0;
	uint16 *pqos;
	int fifo = TX_ATIM_FIFO;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	int prio;
	int err;
	d11actxh_t *txh;
	uint16 txh_off = 0;

	eh = (struct ether_header *) PKTDATA(osh, pkt);
	bcopy(eh, &temp, sizeof(struct ether_header));
	dst = (struct ether_addr*)eh->ether_dhost;

	/* Allow BCAST and ISMULTI for multicast DNS/ARP but trap NULL addr */
	if (ETHER_ISNULLADDR(dst)) {
		WL_ERROR(("wlc_dngl_ol_send_data_pkt: reject NULL\n"));
		return FALSE;
	}

	prio = SCB_QOS(&wlc_dngl_ol->scb[0]) ? PRIO_8021D_BK : 0;

	WLPKTTAGSCBSET(pkt, &wlc_dngl_ol->scb[0]);

	/* use scb key to transmit. if none, use bss tx key */
	key = wlc_keymgmt_get_scb_key(DNGL_OL_KEYMGMT(wlc_dngl_ol),
		&wlc_dngl_ol->scb[0], WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
	if (key_info.algo == CRYPTO_ALGO_OFF)
		key = wlc_keymgmt_get_bss_tx_key(DNGL_OL_KEYMGMT(wlc_dngl_ol),
			&wlc_dngl_ol->bsscfg[0], FALSE, &key_info);

	err = wlc_key_prep_tx_msdu(key, pkt, 0 /* no fragments */ , prio);
	if (err != BCME_OK) {
		WL_ERROR(("%s: error %d from wlc_key_prep_tx_msdu\n", __FUNCTION__, err));
		return FALSE;
	}

	offset = DOT11_A3_HDR_LEN - ETHER_HDR_LEN;
	if (SCB_QOS(&wlc_dngl_ol->scb[0]))
		offset += DOT11_QOS_LEN;

	offset += key_info.iv_len;

	h = (struct dot11_header *)PKTPUSH(osh, pkt, offset);
	bzero((char *)h, offset);

	bcopy((char *)&wlc_dngl_ol->bsscfg[0].BSSID, (char *)&h->a1, ETHER_ADDR_LEN);
	bcopy((char *)&temp.ether_shost, (char*)&h->a2, ETHER_ADDR_LEN);
	bcopy((char *)&temp.ether_dhost, (char*)&h->a3, ETHER_ADDR_LEN);

	fc |= FC_TODS;
	if (SCB_QOS(&wlc_dngl_ol->scb[0])) {
		pqos = (uint16 *)((uchar *)h + DOT11_A3_HDR_LEN);
		PKTSETPRIO(pkt, prio);
		*pqos = htol16(prio);
	}

	fc |= (FC_TYPE_DATA << FC_TYPE_SHIFT);
	if (SCB_QOS(&wlc_dngl_ol->scb[0]))
		fc |= (FC_SUBTYPE_QOS_DATA << FC_SUBTYPE_SHIFT);

	h->fc = htol16(fc);
	rate = RXOETXINFO(wlc_dngl_ol)->rate;
	frameid = wlc_dngl_ol_d11hdrs(wlc_dngl_ol, pkt, rate, &key_info, &txh, &txh_off, fifo);

	PKTPULL(osh, pkt, txh_off + D11AC_TXH_LEN);
	err = wlc_key_prep_tx_mpdu(key, pkt, (wlc_txd_t *)txh);
	if (err != BCME_OK)
		WL_ERROR(("%s: error %d from wlc_key_prep_tx_mpdu\n", __FUNCTION__, err));
	PKTPUSH(osh, pkt, txh_off + D11AC_TXH_LEN);

	wlc_bmac_txfifo(wlc_dngl_ol->wlc_hw, fifo, pkt, TRUE, frameid, 1);
	RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.datafrm);
	RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.tottxpkt);

	return TRUE;
}

static void *
wlc_dngl_ol_hdr_proc(wlc_dngl_ol_info_t *wlc_dngl_ol, void *sdu)
{
	void *pkt;
	osl_t *osh;
	struct ether_header *eh;
	uint16 ether_type;
	osh = wlc_dngl_ol->osh;

	if ((uint)PKTHEADROOM(osh, sdu) < TXOFF) {
		pkt = PKTGET(osh, TXOFF, TRUE);
		if (pkt == NULL) {
			WL_ERROR(("%s, PKTGET headroom %d failed\n",
				__FUNCTION__, (int)TXOFF));
			return NULL;
		}
		PKTPULL(osh, pkt, TXOFF);

		/* move ether_hdr from data buffer to header buffer */
		eh = (struct ether_header*) PKTDATA(osh, sdu);
		PKTPULL(osh, sdu, ETHER_HDR_LEN);
		PKTPUSH(osh, pkt, ETHER_HDR_LEN);
		bcopy((char*)eh, (char*)PKTDATA(osh, pkt), ETHER_HDR_LEN);

		/* chain original sdu onto newly allocated header */
		PKTSETNEXT(osh, pkt, sdu);
		sdu = pkt;
	}

	/*
	 * Original Ethernet (header length = 14):
	 * ----------------------------------------------------------------------------------------
	 * |                                                     |   DA   |   SA   | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *                                                            6        6     2
	 *
	 * Conversion to 802.3 (header length = 22):
	 *                     (LLC includes ether_type in last 2 bytes):
	 * ----------------------------------------------------------------------------------------
	 * |                                      |   DA   |   SA   | L | LLC/SNAP | T |  Data... |
	 * ----------------------------------------------------------------------------------------
	 *                                             6        6     2       6      2
	 */

	eh = (struct ether_header *)PKTDATA(osh, sdu);
	ether_type = ntoh16(eh->ether_type);
	if (ether_type > ETHER_MAX_DATA) {
		wlc_dngl_ol_ether_8023hdr(wlc_dngl_ol, osh, eh, sdu);
	}

	return sdu;
}

bool
wlc_dngl_ol_sendpkt(wlc_dngl_ol_info_t *wlc_dngl_ol, void *sdu)
{
	void *pkt;
	osl_t *osh;

	osh = wlc_dngl_ol->osh;

	if (wlc_dngl_ol->pme_asserted || wlc_dngl_ol->radio_hw_disabled) {
		goto toss;
	}

	pkt = wlc_dngl_ol_hdr_proc(wlc_dngl_ol, sdu);
	if (pkt == NULL)
		goto toss;
	sdu = pkt;
	if (wlc_dngl_ol_80211hdr(wlc_dngl_ol, sdu))
		return TRUE;
toss:
	PKTFREE(osh, sdu, TRUE);
	return TRUE;
}

#ifdef BCMDBG_DNGL_OL
static void
wlc_dngl_print_buf(uint8 *p, uint size)
{
	uint i;
	for (i = 0; i < size; i++) {
		printf("%02x ", p[i]);
		if (((i+1) % 16) == 0)
			printf("\n");
	}
	printf("\n");
}

void
wlc_dngl_print_dot11_mac_hdr(uint8* buf, int len)
{
	char hexbuf[(2*D11B_PHY_HDR_LEN)+1];
	char a1[(2*ETHER_ADDR_LEN)+1], a2[(2*ETHER_ADDR_LEN)+1];
	char a3[(2*ETHER_ADDR_LEN)+1];
	char flagstr[64];
	uint16 fc, kind, toDS, fromDS;
	uint16 v;
	int fill_len = 0;
	static const bcm_bit_desc_t fc_flags[] = {
		{FC_TODS, "ToDS"},
		{FC_FROMDS, "FromDS"},
		{FC_MOREFRAG, "MoreFrag"},
		{FC_RETRY, "Retry"},
		{FC_PM, "PM"},
		{FC_MOREDATA, "MoreData"},
		{FC_WEP, "WEP"},
		{FC_ORDER, "Order"},
		{0, NULL}
	};

	if (len < 2) {
		printf("FC: ------ ");
		printf("%s\n", errstr);
		return;
	}

	fc = buf[0] | (buf[1] << 8);
	kind = fc & FC_KIND_MASK;
	toDS = (fc & FC_TODS) != 0;
	fromDS = (fc & FC_FROMDS) != 0;

	bcm_format_flags(fc_flags, fc, flagstr, 64);

	printf("FC: 0x%04x ", fc);
	if (flagstr[0] != '\0')
		printf("(%s) ", flagstr);

	len -= 2;
	buf += 2;

	if (len < 2) {
		printf("Dur/AID: ----- ");
		printf("%s\n", errstr);
		return;
	}

	v = buf[0] | (buf[1] << 8);
	if (kind == FC_PS_POLL) {
		printf("AID: 0x%04x", v);
	} else {
		printf("Dur: 0x%04x", v);
	}
	printf("\n");
	len -= 2;
	buf += 2;

	strncpy(a1, fillstr, sizeof(a1)-1);
	a1[2*ETHER_ADDR_LEN] = '\0';

	strncpy(a2, fillstr, sizeof(a2)-1);
	a2[2*ETHER_ADDR_LEN] = '\0';

	strncpy(a3, fillstr, sizeof(a3)-1);
	a3[2*ETHER_ADDR_LEN] = '\0';

	if (len < ETHER_ADDR_LEN) {
		bcm_format_hex(a1, buf, len);
		strncpy(a1+(2*len), fillstr, 2*(ETHER_ADDR_LEN-len));
	} else if (len < 2*ETHER_ADDR_LEN) {
		bcm_format_hex(a1, buf, ETHER_ADDR_LEN);
		bcm_format_hex(a2, buf+ETHER_ADDR_LEN, len-ETHER_ADDR_LEN);
		fill_len = len - ETHER_ADDR_LEN;
		strncpy(a2+(2*fill_len), fillstr, 2*(ETHER_ADDR_LEN-fill_len));
	} else if (len < 3*ETHER_ADDR_LEN) {
		bcm_format_hex(a1, buf, ETHER_ADDR_LEN);
		bcm_format_hex(a2, buf+ETHER_ADDR_LEN, ETHER_ADDR_LEN);
		bcm_format_hex(a3, buf+(2*ETHER_ADDR_LEN), len-(2*ETHER_ADDR_LEN));
		fill_len = len - (2*ETHER_ADDR_LEN);
		strncpy(a3+(2*fill_len), fillstr, 2*(ETHER_ADDR_LEN-fill_len));
	} else {
		bcm_format_hex(a1, buf, ETHER_ADDR_LEN);
		bcm_format_hex(a2, buf+ETHER_ADDR_LEN, ETHER_ADDR_LEN);
		bcm_format_hex(a3, buf+(2*ETHER_ADDR_LEN), ETHER_ADDR_LEN);
	}

	if (kind == FC_RTS) {
		printf("RA: %s ", a1);
		printf("TA: %s ", a2);
		if (len < 2*ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (kind == FC_CTS || kind == FC_ACK) {
		printf("RA: %s ", a1);
		if (len < ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (kind == FC_PS_POLL) {
		printf("BSSID: %s", a1);
		printf("TA: %s ", a2);
		if (len < 2*ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (kind == FC_CF_END || kind == FC_CF_END_ACK) {
		printf("RA: %s ", a1);
		printf("BSSID: %s ", a2);
		if (len < 2*ETHER_ADDR_LEN)
			printf("%s ", errstr);
	} else if (FC_TYPE(fc) == FC_TYPE_DATA) {
		if (!toDS) {
			printf("DA: %s ", a1);
			if (!fromDS) {
				printf("SA: %s ", a2);
				printf("BSSID: %s ", a3);
			} else {
				printf("BSSID: %s ", a2);
				printf("SA: %s ", a3);
			}
		} else if (!fromDS) {
			printf("BSSID: %s ", a1);
			printf("SA: %s ", a2);
			printf("DA: %s ", a3);
		} else {
			printf("RA: %s ", a1);
			printf("TA: %s ", a2);
			printf("DA: %s ", a3);
		}
		if (len < 3*ETHER_ADDR_LEN) {
			printf("%s ", errstr);
		} else if (len < 20) {
			printf("SeqCtl: ------ ");
			printf("%s ", errstr);
		} else {
			len -= 3*ETHER_ADDR_LEN;
			buf += 3*ETHER_ADDR_LEN;
			v = buf[0] | (buf[1] << 8);
			printf("SeqCtl: 0x%04x ", v);
			len -= 2;
			buf += 2;
		}
	} else if (FC_TYPE(fc) == FC_TYPE_MNG) {
		printf("DA: %s ", a1);
		printf("SA: %s ", a2);
		printf("BSSID: %s ", a3);
		if (len < 3*ETHER_ADDR_LEN) {
			printf("%s ", errstr);
		} else if (len < 20) {
			printf("SeqCtl: ------ ");
			printf("%s ", errstr);
		} else {
			len -= 3*ETHER_ADDR_LEN;
			buf += 3*ETHER_ADDR_LEN;
			v = buf[0] | (buf[1] << 8);
			printf("SeqCtl: 0x%04x ", v);
			len -= 2;
			buf += 2;
		}
	}

	if ((FC_TYPE(fc) == FC_TYPE_DATA) && toDS && fromDS) {

		if (len < ETHER_ADDR_LEN) {
			bcm_format_hex(hexbuf, buf, len);
			strncpy(hexbuf+(2*len), fillstr, 2*(ETHER_ADDR_LEN-len));
		} else {
			bcm_format_hex(hexbuf, buf, ETHER_ADDR_LEN);
		}

		printf("SA: %s ", hexbuf);

		if (len < ETHER_ADDR_LEN) {
			printf("%s ", errstr);
		} else {
			len -= ETHER_ADDR_LEN;
			buf += ETHER_ADDR_LEN;
		}
	}

	if ((FC_TYPE(fc) == FC_TYPE_DATA) && (kind == FC_QOS_DATA)) {
		if (len < 2) {
			printf("QoS: ------");
			printf("%s ", errstr);
		} else {
			v = buf[0] | (buf[1] << 8);
			printf("QoS: 0x%04x ", v);
			len -= 2;
			buf += 2;
		}
	}

	printf("\n");
	return;
}

static void
wlc_dngl_print_dot11_plcp(uint8* buf, int len)
{
	char hexbuf[(2*D11B_PHY_HDR_LEN)+1];

	if (len < D11B_PHY_HDR_LEN) {
		bcm_format_hex(hexbuf, buf, len);
		strncpy(hexbuf + (2 * len), fillstr, 2 * (D11B_PHY_HDR_LEN - len));
		hexbuf[sizeof(hexbuf) - 1] = '\0';
	} else {
		bcm_format_hex(hexbuf, buf, D11B_PHY_HDR_LEN);
	}

	printf("PLCPHdr: %s ", hexbuf);
	if (len < D11B_PHY_HDR_LEN) {
		printf("%s\n", errstr);
	}
}

static void
wlc_dngl_print_dot11hdr(uint8* buf, int len)
{
	if (len == 0) {
		printf("802.11 Header MISSING\n");
		return;
	}

	wlc_dngl_print_dot11_plcp(buf, len);

	if (len < D11B_PHY_HDR_LEN) {
		return;
	}

	len -= D11B_PHY_HDR_LEN;
	buf += D11B_PHY_HDR_LEN;

	wlc_dngl_print_dot11_mac_hdr(buf, len);
}

static void
wlc_dngl_print_d11txh(d11txh_t* txh)
{
	uint16 mtcl = ltoh16(txh->MacTxControlLow);
	uint16 mtch = ltoh16(txh->MacTxControlHigh);
	uint16 mfc = ltoh16(txh->MacFrameControl);
	uint16 tfest = ltoh16(txh->TxFesTimeNormal);
	uint16 ptcw = ltoh16(txh->PhyTxControlWord);
	uint16 ptcw_1 = ltoh16(txh->PhyTxControlWord_1);
	uint16 ptcw_1_Fbr = ltoh16(txh->PhyTxControlWord_1_Fbr);
	uint16 ptcw_1_Rts = ltoh16(txh->PhyTxControlWord_1_Rts);
	uint16 ptcw_1_FbrRts = ltoh16(txh->PhyTxControlWord_1_FbrRts);
	uint16 mainrates = ltoh16(txh->MainRates);
	uint16 xtraft = ltoh16(txh->XtraFrameTypes);
	uint8 *iv = txh->IV;
	uint8 *ra = txh->TxFrameRA;
	uint16 tfestfb = ltoh16(txh->TxFesTimeFallback);
	uint8 *rtspfb = txh->RTSPLCPFallback;
	uint16 rtsdfb = ltoh16(txh->RTSDurFallback);
	uint8 *fragpfb = txh->FragPLCPFallback;
	uint16 fragdfb = ltoh16(txh->FragDurFallback);
	uint16 mmodelen = ltoh16(txh->MModeLen);
	uint16 mmodefbrlen = ltoh16(txh->MModeFbrLen);
	uint16 tfid = ltoh16(txh->TxFrameID);
	uint16 txs = ltoh16(txh->TxStatus);
	uint16 mnmpdu = ltoh16(txh->MaxNMpdus);
	uint16 maxdur = ltoh16(txh->u1.MaxAggDur);
	uint8 maxrnum = txh->u2.s1.MaxRNum;
	uint8 maxaggbytes = txh->u2.s1.MaxAggBytes;
	uint16 mmbyte = ltoh16(txh->MinMBytes);

	uint8 *rtsph = txh->RTSPhyHeader;
	struct dot11_rts_frame rts = txh->rts_frame;
	char hexbuf[256];

	prhex("Raw TxDesc", (uchar *) txh, sizeof(d11txh_t));

	printf("TxCtlLow: %04x ", mtcl);
	printf("TxCtlHigh: %04x ", mtch);
	printf("FC: %04x ", mfc);
	printf("FES Time: %04x\n", tfest);
	printf("PhyCtl: %04x%s ", ptcw, (ptcw & PHY_TXC_SHORT_HDR) ? " short" : "");
	printf("PhyCtl_1: %04x ", ptcw_1);
	printf("PhyCtl_1_Fbr: %04x\n", ptcw_1_Fbr);
	printf("PhyCtl_1_Rts: %04x ", ptcw_1_Rts);
	printf("PhyCtl_1_Fbr_Rts: %04x\n", ptcw_1_FbrRts);
	printf("MainRates: %04x ", mainrates);
	printf("XtraFrameTypes: %04x ", xtraft);
	printf("\n");

	bcm_format_hex(hexbuf, iv, sizeof(txh->IV));
	printf("SecIV:       %s\n", hexbuf);
	bcm_format_hex(hexbuf, ra, sizeof(txh->TxFrameRA));
	printf("RA:          %s\n", hexbuf);

	printf("Fb FES Time: %04x ", tfestfb);
	bcm_format_hex(hexbuf, rtspfb, sizeof(txh->RTSPLCPFallback));
	printf("RTS PLCP: %s ", hexbuf);
	printf("RTS DUR: %04x ", rtsdfb);
	bcm_format_hex(hexbuf, fragpfb, sizeof(txh->FragPLCPFallback));
	printf("PLCP: %s ", hexbuf);
	printf("DUR: %04x", fragdfb);
	printf("\n");

	printf("MModeLen: %04x ", mmodelen);
	printf("MModeFbrLen: %04x\n", mmodefbrlen);

	printf("FrameID:     %04x\n", tfid);
	printf("TxStatus:    %04x\n", txs);

	printf("MaxNumMpdu:  %04x\n", mnmpdu);
	printf("MaxAggDur:   %04x\n", maxdur);
	printf("MaxRNum:     %04x\n", maxrnum);
	printf("MaxAggBytes: %04x\n", maxaggbytes);
	printf("MinByte:     %04x\n", mmbyte);

	bcm_format_hex(hexbuf, rtsph, sizeof(txh->RTSPhyHeader));
	printf("RTS PLCP: %s ", hexbuf);
	bcm_format_hex(hexbuf, (uint8 *) &rts, sizeof(txh->rts_frame));
	printf("RTS Frame: %s", hexbuf);
	printf("\n");

	if (mtcl & TXC_SENDRTS) {
		wlc_dngl_print_dot11hdr((uint8 *) &rts, sizeof(txh->rts_frame));
	}
}

void
wlc_dngl_print_txdesc_ac(wlc_info_t *wlc, void* hdrsBegin)
{
	/* tso header */
	d11actxh_t* acHdrPtr;
	uint len;
/*	uint8 rateNum, rate_count; */

	/* d11ac headers */
	acHdrPtr = (d11actxh_t*)(hdrsBegin);

	if (acHdrPtr->PktInfo.MacTxControlLow & htol16(D11AC_TXC_HDR_FMT_SHORT)) {
		len = D11AC_TXH_SHORT_LEN;
	} else {
		len = D11AC_TXH_LEN;
	}

	printf("tx hdr len=%d dump follows:\n", len);

	prhex("Raw TxACDesc", (uchar *)hdrsBegin, len);
	wlc_dngl_print_per_pkt_desc_ac(acHdrPtr);
	wlc_dngl_print_per_pkt_cache_ac(wlc, acHdrPtr);

}

uint
wlc_dngl_tso_hdr_length(d11ac_tso_t* tso)
{
	uint len;

	if (tso->flag[0] & TOE_F0_HDRSIZ_NORMAL)
		len = TSO_HEADER_LENGTH;
	else
		len = TSO_HEADER_PASSTHROUGH_LENGTH;

	return len;
}

void
wlc_dngl_print_txdesc(wlc_info_t *wlc, wlc_txd_t *txd)
{
	if (WLCISACPHY(wlc->band)) {
#ifdef WLTOEHW
		if (wlc->toe_capable && !wlc->toe_bypass) {
			uint8 *tsoPtr = (uint8*)txd;
			uint tsoLen = wlc_dngl_tso_hdr_length((d11ac_tso_t*)tsoPtr);

			prhex("TSO hdr:", (uint8 *)tsoPtr, tsoLen);
			txd = (wlc_txd_t*)(tsoPtr + tsoLen);
		}
#endif /* WLTOEHW */
		wlc_dngl_print_txdesc_ac(wlc, txd);
	} else {
		wlc_dngl_print_d11txh(&txd->d11txh);
	}
}

static void
wlc_dngl_print_byte(const char* desc, uint8 val)
{
	printf("%s: %02x\n", desc, val);
}

static void
wlc_dngl_print_word(const char* desc, uint16 val)
{
	printf("%s: %04x\n", desc, val);
}

static void
wlc_dngl_print_per_pkt_desc_ac(d11actxh_t* acHdrPtr)
{
	d11actxh_pkt_t *pi = &acHdrPtr->PktInfo;
	uint16 mcl, mch;

	printf("TxD Pkt Info:\n");
	/* per packet info */
	mcl = ltoh16(pi->MacTxControlLow);
	ASSERT(mcl != 0 || pi->MacTxControlLow == 0);
	mch = ltoh16(pi->MacTxControlHigh);

	printf(" MacTxControlLow 0x%04X MacTxControlHigh 0x%04X Chspec 0x%04X\n",
	       mcl, mch, ltoh16(pi->Chanspec));
	printf(" TxDShrt %u UpdC %u CacheID %u AMPDU %u ImmdAck %u LFRM %u IgnPMQ %u\n",
	       (mcl & D11AC_TXC_HDR_FMT_SHORT) != 0,
	       (mcl & D11AC_TXC_UPD_CACHE) != 0,
	       (mcl & D11AC_TXC_CACHE_IDX_MASK) >> D11AC_TXC_CACHE_IDX_SHIFT,
	       (mcl & D11AC_TXC_AMPDU) != 0,
	       (mcl & D11AC_TXC_IACK) != 0,
	       (mcl & D11AC_TXC_LFRM) != 0,
	       (mcl & D11AC_TXC_HDR_FMT_SHORT) != 0);
	printf(" MBurst %u ASeq %u Aging %u AMIC %u STMSDU %u RIFS %u ~FCS %u FixRate %u\n",
	       (mcl & D11AC_TXC_MBURST) != 0,
	       (mcl & D11AC_TXC_ASEQ) != 0,
	       (mcl & D11AC_TXC_AGING) != 0,
	       (mcl & D11AC_TXC_AMIC) != 0,
	       (mcl & D11AC_TXC_STMSDU) != 0,
	       (mcl & D11AC_TXC_URIFS) != 0,
	       (mch & D11AC_TXC_DISFCS) != 0,
	       (mch & D11AC_TXC_FIX_RATE) != 0);

	printf(" IVOffset %u PktCacheLen %u FrameLen %u\n",
	       pi->IVOffset, pi->PktCacheLen, ltoh16(pi->FrameLen));
	printf(" Seq 0x%04X TxFrameID 0x%04X Tstamp 0x%04X TxStatus 0x%04X\n",
	       ltoh16(pi->Seq), ltoh16(pi->TxFrameID),
	       ltoh16(pi->Tstamp), ltoh16(pi->TxStatus));
}

static void
wlc_dngl_print_per_pkt_cache_ac(wlc_info_t *wlc, d11actxh_t* acHdrPtr)
{
	d11actxh_cache_t	*cache_info;

	cache_info = WLC_TXD_CACHE_INFO_GET(acHdrPtr, wlc->pub->corerev);

	printf("TxD Pkt Cache Info:\n");
	wlc_dngl_print_byte(" BssIdEncAlg", cache_info->BssIdEncAlg);
	wlc_dngl_print_byte(" KeyIdx", cache_info->KeyIdx);
	wlc_dngl_print_byte(" PrimeMpduMax", cache_info->PrimeMpduMax);
	wlc_dngl_print_byte(" FallbackMpduMax", cache_info->FallbackMpduMax);
	wlc_dngl_print_word(" AmpduDur", ltoh16(cache_info->AmpduDur));
	wlc_dngl_print_byte(" BAWin", cache_info->BAWin);
	wlc_dngl_print_byte(" MaxAggLen", cache_info->MaxAggLen);
	prhex(" TkipPH1Key", (uchar *)cache_info->TkipPH1Key, 10);
	prhex(" TSCPN", (uchar *)cache_info->TSCPN, 6);
}

void
wlc_dngl_print_hdrs(wlc_info_t *wlc, const char *prefix, uint8 *frame,
               wlc_txd_t *txd, wlc_d11rxhdr_t *wrxh, uint len)
{
	ASSERT(!(txd && wrxh));

	printf("\nwl%d: len %d %s:\n", wlc->pub->unit, len, prefix);

	if (txd) {
		wlc_dngl_print_txdesc(wlc, txd);
	}
	else if (wrxh) {
		wlc_recv_print_rxh(wlc, wrxh);
	}

	if (len > 0) {
		ASSERT(frame != NULL);
		wlc_dngl_print_buf(frame, len);
		wlc_dngl_print_dot11_mac_hdr(frame, len);
	}
}
#endif /* BCMDBG_DNGL_OL */

uint16
wlc_dngl_ol_d11hdrs(wlc_dngl_ol_info_t *wlc_dngl_ol, void *p, ratespec_t rspec,
	const wlc_key_info_t *key_info, d11actxh_t **out_txh, uint16 *txh_off, int fifo)
{
	struct dot11_header *h;
	d11actxh_t *txh;
	osl_t *osh;
	int len, phylen;
	uint16 fc, type, frameid, mch;
	uint16 seq = 0, mcl = 0;
	uint8 *plcp;
	d11actxh_rate_t *rate_blk;
	d11actxh_rate_t *rate_hdr;
	uint frag = 0;
	wlc_pkttag_t *pkttag;

	pkttag = WLPKTTAG(p);
	osh = wlc_dngl_ol->osh;

	h = (struct dot11_header*) PKTDATA(osh, p);

	fc = ltoh16(h->fc);
	type = FC_TYPE(fc);

	len = pkttotlen(osh, p);
	phylen = len + DOT11_FCS_LEN;

	if (key_info != NULL) {
		/* note: dongle not expected to tx igtk protected frames */
		phylen += key_info->icv_len;
		if (WLC_KEY_FRAG_HAS_TKIP_MIC(p, key_info, 0, 1))
			phylen += TKIP_MIC_SIZE;
	}

	/* add Broadcom tx descriptor header */
	txh = (d11actxh_t*)PKTPUSH(osh, p, D11AC_TXH_LEN);
	bzero((char*)txh, D11AC_TXH_LEN);

	rate_blk = WLC_TXD_RATE_INFO_GET(txh, wlc_dngl_ol->wlc->pub->corerev);
	rate_hdr = &rate_blk[0];

	plcp = rate_hdr->plcp;

	txh->PktInfo.TSOInfo = 0;

	mcl |= D11AC_TXC_IPMQ;
	mcl |= D11AC_TXC_STMSDU;
	mcl |= D11AC_TXC_IACK;

	mch = D11AC_TXC_FIX_RATE;

	txh->PktInfo.Chanspec = htol16(RXOETXINFO(wlc_dngl_ol)->chanspec);
	txh->PktInfo.IVOffset = DOT11_A3_HDR_LEN;
	if ((fc & FC_KIND_MASK) == FC_QOS_DATA)
		txh->PktInfo.IVOffset += DOT11_QOS_LEN;

#ifdef UCODE_SEQ
	if ((key_info != NULL && key_info->algo == CRYPTO_ALGO_OFF) ||
		(fc & FC_KIND_MASK) != FC_QOS_DATA) {
		mcl |= D11AC_TXC_ASEQ;
	}
#else
	mcl |= D11AC_TXC_ASEQ;
#endif // endif

	txh->PktInfo.FrameLen = htol16((uint16)phylen);

	/* Increment the sequence number only after the last fragment */
	h->seq = 0;
	txh->PktInfo.Seq = h->seq;

	seq = (wlc_dngl_ol->counter << SEQNUM_SHIFT) | (frag & FRAGNUM_MASK);
	frameid = ((seq << TXFID_SEQ_SHIFT) & TXFID_SEQ_MASK) |
		(WLC_TXFID_SET_QUEUE(fifo));
	wlc_dngl_ol->counter++;

	txh->PktInfo.TxStatus = 0;
	txh->PktInfo.PktCacheLen = 0;
	txh->PktInfo.Tstamp = 0;
	rate_hdr->TxRate = rspec;
	rate_hdr->RtsCtsControl = 	htol16(D11AC_RTSCTS_LAST_RATE);
	rate_hdr->PhyTxControlWord_0 = RXOETXINFO(wlc_dngl_ol)->PhyTxControlWord_0;
	rate_hdr->PhyTxControlWord_1 = RXOETXINFO(wlc_dngl_ol)->PhyTxControlWord_1;
	rate_hdr->PhyTxControlWord_2 = RXOETXINFO(wlc_dngl_ol)->PhyTxControlWord_2;

	txh->PktInfo.MacTxControlLow = htol16(mcl);
	txh->PktInfo.MacTxControlHigh = htol16(mch);

	/* PLCP: determine PLCP header and MAC duration, fill d11txh_t */

	wlc_dngl_ol_compute_ofdm_plcp(rspec, phylen, plcp);

	/* TxFrameID */
	txh->PktInfo.TxFrameID = htol16(frameid);

	/* return txh to caller if asked */
	if (out_txh)
		*out_txh = txh;

	wlc_dngl_ol_toe_add_hdr(wlc_dngl_ol, p, txh_off);
	return (frameid);
}

static void
wlc_dngl_ol_compute_ofdm_plcp(ratespec_t rspec, uint32 length, uint8 *plcp)
{
	uint8 rate_signal;
	uint32 tmp = 0;
	int rate;

	/* extract the 500Kbps rate for rate_info lookup */
	rate = (rspec & RSPEC_RATE_MASK);

	/* encode rate per 802.11a-1999 sec 17.3.4.1, with lsb transmitted first */
	rate_signal = rate_info[rate] & RATE_MASK;
	ASSERT(rate_signal != 0);

	bzero(plcp, D11_PHY_HDR_LEN);
	D11A_PHY_HDR_SRATE((ofdm_phy_hdr_t *)plcp, rate_signal);

	tmp = (length & 0xfff) << 5;
	plcp[2] |= (tmp >> 16) & 0xff;
	plcp[1] |= (tmp >> 8) & 0xff;
	plcp[0] |= tmp & 0xff;

	return;
}

void
wlc_dngl_ol_toe_add_hdr(wlc_dngl_ol_info_t *wlc_dngl_ol, void *p,
	uint16 *pushlen)
{
	d11ac_tso_t tso;
	d11ac_tso_t * tsohdr;
	int len;

	/* No CSO, prepare a passthrough TOE header */
	bzero(&tso, TSO_HEADER_PASSTHROUGH_LENGTH);
	tso.flag[0] |= TOE_F0_PASSTHROUGH;
	len = TSO_HEADER_PASSTHROUGH_LENGTH;

	tsohdr = (d11ac_tso_t*)PKTPUSH(wlc_dngl_ol->osh, p, len);
	bcopy(&tso, tsohdr, len);

	if (pushlen != NULL)
		*pushlen = (uint16)len;
}

static void wlc_dngl_ol_update_key(wlc_dngl_ol_info_t *wlc_dngl_ol,
	wlc_key_t *key, const ol_key_info *ol_ki)
{
	int retval;
	wlc_info_t *wlc;
	int i;

	wlc = wlc_dngl_ol->wlc;

	/* note: the key data length used is sizeof buffer rather than ol_ki->key_len
	 * because key_len in key_info is 16 for TKIP, but MIC keys are included.
	 */
	retval = wlc_key_set_data(key, ol_ki->info.algo, ol_ki->data, sizeof(ol_ki->data));
	if (retval != BCME_OK)
		goto done;

	retval = wlc_key_set_seq(key, ol_ki->txiv.buf,
		sizeof(ol_ki->txiv.buf), 0, TRUE);
	if (retval != BCME_OK)
		goto done;

	for (i = 0; i < WLC_KEY_BASE_RX_SEQ; ++i) {
		int retval2 = wlc_key_set_seq(key, ol_ki->rxiv[i].buf,
			sizeof(ol_ki->rxiv[i].buf), (wlc_key_seq_id_t)i, FALSE);
		if (retval2 != BCME_OK)
			retval = retval2;
	}

	(void)wlc_key_set_hw_idx(key, ol_ki->hw_idx, WLC_KEY_MIC_IN_HW(&ol_ki->info));

done:
	if (retval != BCME_OK)
		WL_ERROR(("wl%d: %s: error %d updating key from ol key info, key idx %d\n",
			WLCWLUNIT(wlc), __FUNCTION__, retval, ol_ki->info.key_idx));
}

static void wlc_dngl_ol_update_ol_key(wlc_dngl_ol_info_t *wlc_dngl_ol,
	ol_key_info *ol_ki, wlc_key_t *key, const wlc_key_info_t *key_info)
{
	int retval = BCME_OK;
	wlc_info_t *wlc;
	int i;

	wlc = wlc_dngl_ol->wlc;

	bzero(ol_ki, sizeof(*ol_ki));
	ol_ki->info = *key_info;
	ol_ki->hw_idx = wlc_key_get_hw_idx(key);
	if (ol_ki->info.algo == CRYPTO_ALGO_OFF)
		goto done;

	retval = wlc_key_get_data(key, ol_ki->data, sizeof(ol_ki->data), NULL);
	if (retval < 0)
		retval = BCME_BUFTOOSHORT;
	else
		retval = BCME_OK;

	retval = wlc_key_get_seq(key, ol_ki->txiv.buf,
		sizeof(ol_ki->txiv.buf), 0, TRUE);
	if (retval < 0)
		retval = BCME_BUFTOOSHORT;
	else
		retval = BCME_OK;

	for (i = 0; i < WLC_KEY_BASE_RX_SEQ; i++) {
		int retval2 = wlc_key_get_seq(key, ol_ki->rxiv[i].buf,
			sizeof(ol_ki->rxiv[i].buf), (wlc_key_seq_id_t)i, FALSE);
		if (retval2 < 0)
			retval = BCME_BUFTOOSHORT;
	}

done:
	if (retval != BCME_OK) {
		WL_ERROR(("wl%d: %s: error %d updating ol key from key, key idx %d\n",
			WLCWLUNIT(wlc), __FUNCTION__, retval, ol_ki->info.key_idx));
	}
}

void wlc_dngl_ol_sec_info_from_host(wlc_dngl_ol_info_t *dngl_ol,
	const struct ether_addr *host_addr, const struct ether_addr *bssid,
	const ol_sec_info *sec_info)
{
	wlc_bsscfg_t *bsscfg;
	scb_t *scb;
	wlc_keymgmt_t *km;
	wlc_key_t *key;
	wlc_key_id_t key_id;
	ol_tx_info *shared_txi;

	km = DNGL_OL_KEYMGMT(dngl_ol);
	bsscfg = &dngl_ol->bsscfg[0];
	scb = &dngl_ol->scb[0];

	/* if bss parameters are changing, reset keymgmt and set up for reinit */
	if (bsscfg->flags & WLC_OL_BSSCFG_F_INITED) {
		if (bsscfg->wsec != sec_info->wsec ||
			bsscfg->WPA_auth != sec_info->WPA_auth ||
			bsscfg->wsec_restrict != sec_info->wsec_restrict ||
			bcmp(host_addr, &bsscfg->cur_etheraddr, sizeof(*host_addr)) ||
			bcmp(bssid, &bsscfg->BSSID, sizeof(*bssid)) ||
			((sec_info->flags & OL_SEC_F_QOS) && !(SCB_QOS(scb))) ||
			((sec_info->flags & OL_SEC_F_MFP) && !(SCB_MFP(scb))) ||
			((sec_info->flags & OL_SEC_F_IBSS) && !BSSCFG_IBSS(bsscfg))) {
			wlc_keymgmt_reset(km, bsscfg, scb);
			bzero(bsscfg, sizeof(*bsscfg));
			bzero(scb, sizeof(*scb));
		}
	}

	if (!(bsscfg->flags & WLC_OL_BSSCFG_F_INITED)) {
		bsscfg->idx = 0;
		bsscfg->flags = WLC_OL_BSSCFG_F_INITED | WLC_OL_BSSCFG_F_UP |
			WLC_OL_BSSCFG_F_ASSOCIATED;
		if (!(sec_info->flags & OL_SEC_F_IBSS))
			bsscfg->flags |= WLC_OL_BSSCFG_F_BSS;
		if (sec_info->flags & OL_SEC_F_WIN7PLUS)
			bsscfg->flags |= WLC_OL_BSSCFG_F_WIN7PLUS;

		bsscfg->wsec = sec_info->wsec;
		bsscfg->WPA_auth = sec_info->WPA_auth;
		bsscfg->wsec_restrict = sec_info->wsec_restrict;
		bcopy(host_addr, &bsscfg->cur_etheraddr, sizeof(struct ether_addr));
		bcopy(bssid, &bsscfg->BSSID, sizeof(struct ether_addr));

		scb->idx = 0;
		bcopy(bssid, &scb->ea, sizeof(struct ether_addr));
		scb->flags = WLC_OL_SCB_F_INITED;

		if (sec_info->flags & OL_SEC_F_QOS)
			scb->flags |= WLC_OL_SCB_F_QOS;

		if (sec_info->flags & OL_SEC_F_MFP)
			scb->flags |= WLC_OL_SCB_F_MFP;

		scb->bsscfg = bsscfg;
		scb->WPA_auth = bsscfg->WPA_auth;

		wlc_keymgmt_reset(km, bsscfg, scb);
	}

	ASSERT(SCB_BSSCFG(scb) == bsscfg);

	shared_txi = (ol_tx_info *)RXOETXINFO(dngl_ol);
	bcopy(sec_info, &shared_txi->sec_info, sizeof(ol_sec_info));

	key = wlc_keymgmt_get_scb_key(km, scb, WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, NULL);
	wlc_dngl_ol_update_key(dngl_ol, key, &sec_info->scb_key_info);

	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, NULL);
		wlc_dngl_ol_update_key(dngl_ol, key, &sec_info->bss_key_info[key_id]);
	}

	(void)wlc_keymgmt_set_bss_tx_key_id(km, bsscfg, sec_info->bss_tx_key_id, FALSE);

#ifdef MFP
	for (key_id = WLC_KEY_ID_IGTK_1; key_id <= WLC_KEY_ID_IGTK_2; ++key_id) {
		key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, NULL);
		wlc_dngl_ol_update_key(dngl_ol, key,
			&sec_info->igtk_key_info[OL_IGTK_IDX_POS(key_id)]);
	}
#endif /* MFP */
	/* update key_rot_id_mask from host; if we get an update from host, assume
	 * that all rotations during sleep have been propagated to host
	 */
	RXOEUPDKEYROT(dngl_ol, sec_info->key_rot_id_mask);

	bzero((void *)RXOEMICFAILINFO(dngl_ol), sizeof(wowl_mic_fail_pkt_info_t));
	dngl_ol->last_mic_fail_time = -(WPA_TKIP_CM_DETECT + 1);

	bsscfg->flags &= ~WLC_OL_BSSCFG_F_WAKEON1MICERR;
	if (sec_info->flags & OL_SEC_F_WAKEON1MICERR)
		bsscfg->flags |= WLC_OL_BSSCFG_F_WAKEON1MICERR;

	scb->flags &= ~WLC_OL_SCB_F_LEGACY_AES;
	if (sec_info->flags & OL_SEC_F_LEGACY_AES)
		scb->flags |= WLC_OL_SCB_F_LEGACY_AES;
}

void wlc_dngl_ol_mic_fail_info_to_host(wlc_dngl_ol_info_t *dngl_ol)
{
	uint32 fail_time;
	wlc_info_t *wlc;
	uint8 fail_count;
	wowl_mic_fail_pkt_info_t *dfpi;

	dfpi = (wowl_mic_fail_pkt_info_t *)RXOEMICFAILINFO(dngl_ol);

	/* fix up the failure time to be relative to wake */
	fail_count = dfpi->fail_count;
	ASSERT(fail_count <= 2);

	if (fail_count < 1)
		goto done;

	wlc = dngl_ol->wlc;

	fail_time = wlc->pub->now - dfpi->fail_info[0].fail_time;
	dfpi->fail_info[0].fail_time = fail_time;
	if (fail_count < 2)
		goto done;
	fail_time = wlc->pub->now - dfpi->fail_info[1].fail_time;
	dfpi->fail_info[1].fail_time = fail_time;
done:;
}

/* sync dongle state with keymgmt */
void wlc_dngl_ol_tx_info_to_host(wlc_dngl_ol_info_t *dngl_ol,
	wlc_key_t *key, const wlc_key_info_t *key_info)
{
	volatile ol_tx_info *dtxi;
	volatile ol_key_info *ol_ki;

	ASSERT(key != NULL && key_info != NULL);

	dtxi = RXOETXINFO(dngl_ol);
	if (WLC_KEY_IS_PAIRWISE(key_info))
		ol_ki = &dtxi->sec_info.scb_key_info;
	else if (WLC_KEY_IS_GROUP(key_info))
		ol_ki = &dtxi->sec_info.bss_key_info[key_info->key_id];
#ifdef MFP
	else if (WLC_KEY_IS_IGTK(key_info))
		ol_ki = &dtxi->sec_info.igtk_key_info[OL_IGTK_IDX_POS(key_info->key_id)];
#endif /* MFP */
	else
		goto done;

	wlc_dngl_ol_update_ol_key(dngl_ol, (ol_key_info *)ol_ki, key, key_info);
done:;
}

void
wlc_dngl_ol_reset(wlc_dngl_ol_info_t *wlc_dngl_ol)
{
	d11regs_t *d11_regs;
	d11_regs = (d11regs_t *)wlc_dngl_ol->regs;

	bzero((void *)RXOETXINFO(wlc_dngl_ol), sizeof(ol_tx_info));
	bzero((void *)&wlc_dngl_ol->cur_etheraddr, sizeof(struct ether_addr));
	wlc_dngl_ol->TX = FALSE;
	wlc_dngl_ol->counter = 0;
	if (wlc_dngl_ol->bcn_ol)
		wlc_dngl_ol_bcn_clear(wlc_dngl_ol->bcn_ol, wlc_dngl_ol);
}

void wlc_dngl_ol_armtx(wlc_dngl_ol_info_t *wlc_dngl_ol, void *buf, int len)
{
	olmsg_armtx *armtx = buf;
	ol_tx_info *txi = (ol_tx_info *)RXOETXINFO(wlc_dngl_ol);

	ENTER();

	WL_TRACE(("New pm state %d Old pm state %d\n",
		armtx->TX, wlc_dngl_ol->TX));

	wlc_dngl_ol->TX = armtx->TX;
	if (wlc_dngl_ol->TX) {
		bcopy(&armtx->txinfo, (ol_tx_info *)txi, sizeof(ol_tx_info));
		wlc_dngl_ol_sec_info_from_host(wlc_dngl_ol, &txi->cur_etheraddr,
			&txi->BSSID, &txi->sec_info);
	}

	EXIT();
}

static void wlc_dngl_ol_cons(void *buf)
{
	olmsg_ol_conscmd *msg = (olmsg_ol_conscmd *)buf;
	uint cmdlen;

	cmdlen = strlen((char *)(msg->cmdline)) + 1;
	if (cmdlen > 1)
		process_ccmd((char *)(msg->cmdline), cmdlen);
}

void
wlc_recv(wlc_info_t *wlc, void *p)
{
	wlc_dngl_ol_recv(wlc->wlc_dngl_ol, p);
}
bool
wlc_sendpkt(wlc_info_t *wlc, void *sdu, struct wlc_if *wlcif)
{
	BCM_REFERENCE(wlcif);
	wlc_dngl_ol_sendpkt(wlc->wlc_dngl_ol, sdu);
	/* returns true if pkt is discarded --- this needs to checked */
	return FALSE;
}

bool arm_dotx(wlc_info_t *wlc)
{
	return wlc->wlc_dngl_ol->TX;
}

int
wlc_dngl_ol_process_msg(wlc_dngl_ol_info_t *wlc_dngl_ol, void *buf, int len)
{
	olmsg_test *msg_hdr;
	uchar *pktdata;
#ifdef ARPOE
	wl_arp_info_t *arpi = (wl_arp_info_t *)wl_get_arpi(wlc_dngl_ol->wlc->wl, NULL);
#endif // endif
#ifdef TCPKAOE
	wl_tcp_keep_info_t *tcpkeepi = (wl_tcp_keep_info_t *)wl_get_tcpkeepi(wlc_dngl_ol->wlc->wl,
	    NULL);
#endif // endif
#ifdef WLNDOE
	wl_nd_info_t *ndi = (wl_nd_info_t *)wl_get_ndi(wlc_dngl_ol->wlc->wl, NULL);
#endif /* WLNDOE */

	if (len) {
		pktdata = (uint8 *) buf;
		msg_hdr = (olmsg_test *) pktdata;
		switch (msg_hdr->hdr.type) {
			case BCM_OL_ARM_TX:
				wlc_dngl_ol_armtx(wlc_dngl_ol, buf, len);
				break;

			case BCM_OL_RESET:
				wlc_dngl_ol_reset(wlc_dngl_ol);
				break;

			case BCM_OL_CONS:
				wlc_dngl_ol_cons(buf);
				break;

			case BCM_OL_SCAN_ENAB:
			case BCM_OL_SCAN:
			case BCM_OL_SCAN_RESULTS:
			case BCM_OL_SCAN_CONFIG:
			case BCM_OL_SCAN_BSS:
			case BCM_OL_SCAN_QUIET:
			case BCM_OL_SCAN_VALID2G:
			case BCM_OL_SCAN_VALID5G:
			case BCM_OL_SCAN_CHANSPECS:
			case BCM_OL_SCAN_BSSID:
			case BCM_OL_MACADDR:
			case BCM_OL_SCAN_TXRXCHAIN:
			case BCM_OL_SCAN_COUNTRY:
			case BCM_OL_SCAN_PARAMS:
			case BCM_OL_SSIDS:
			case BCM_OL_PREFSSIDS:
			case BCM_OL_PFN_LIST:
			case BCM_OL_PFN_ADD:
			case BCM_OL_PFN_DEL:
			case BCM_OL_ULP:
			case BCM_OL_CURPWR:
			case BCM_OL_SARLIMIT:
			case BCM_OL_TXCORE:
#ifdef BCMDBG
			case BCM_OL_SCAN_DUMP:
			case BCM_OL_DMA_DUMP:
			case BCM_OL_BCNS_PROMISC:
			case BCM_OL_SETCHANNEL:
#endif /* BCMDBG */
				wlc_dngl_ol_scan_send_proc(wlc_dngl_ol->wlc_hw, buf, len);
				break;
			case BCM_OL_MSGLEVEL:
				wl_msg_level = (uint32)msg_hdr->data;
				break;
			case BCM_OL_MSGLEVEL2:
				wl_msg_level2 = (uint32)msg_hdr->data;
				break;
#ifdef BCMDBG
			case BCM_OL_PANIC: {
				void (*p)(void) = NULL;
				int i = 0;

				i /= i;
				(*p)();
				break;
		        }
#endif // endif
			case BCM_OL_PKT_FILTER_ENABLE:
			case BCM_OL_PKT_FILTER_ADD:
			case BCM_OL_PKT_FILTER_DISABLE:
				wlc_pkt_filter_ol_send_proc(wlc_dngl_ol->pkt_filter_ol, buf, len);
				break;

			case BCM_OL_WOWL_ENABLE_START:
			case BCM_OL_WOWL_ENABLE_COMPLETE:
				wlc_wowl_ol_send_proc(wlc_dngl_ol->wowl_ol, buf, len);
				break;

#ifdef WLNDOE
			case BCM_OL_ND_ENABLE:
			case BCM_OL_ND_SETIP:
			case BCM_OL_ND_DISABLE:
				wl_nd_proc_msg(wlc_dngl_ol, ndi, buf);
				break;
#endif // endif

#ifdef ARPOE
			case BCM_OL_ARP_ENABLE:
			case BCM_OL_ARP_SETIP:
			case BCM_OL_ARP_DISABLE:
				wl_arp_proc_msg(wlc_dngl_ol, arpi, buf);
				break;
#endif // endif
#ifdef TCPKAOE
			case BCM_OL_TCP_KEEP_TIMERS:
			case BCM_OL_TCP_KEEP_CONN:
			wl_tcp_keepalive_proc_msg(wlc_dngl_ol, tcpkeepi, buf);
			break;
#endif // endif

			case BCM_OL_BEACON_ENABLE:
			case BCM_OL_BEACON_DISABLE:
			case BCM_OL_MSG_IE_NOTIFICATION:
			case BCM_OL_MSG_IE_NOTIFICATION_FLAG:
				wlc_dngl_ol_bcn_send_proc(wlc_dngl_ol->bcn_ol, buf, len);
				break;

			case BCM_OL_RSSI_INIT:
				wlc_dngl_ol_rssi_send_proc(wlc_dngl_ol->rssi_ol, buf, len);
				break;

#ifdef L2KEEPALIVEOL
			case BCM_OL_L2KEEPALIVE_ENABLE:
				wlc_dngl_ol_l2keepalive_send_proc(wlc_dngl_ol->l2keepalive_ol,
					buf, len);
				break;
#endif // endif
#ifdef GTKOL
			case BCM_OL_GTK_ENABLE:
				wlc_dngl_ol_gtk_send_proc(wlc_dngl_ol->ol_gtk, buf, len);
				break;
#endif // endif
#ifdef LTROL
			case BCM_OL_LTR:
				wlc_dngl_ol_ltr_proc_msg(wlc_dngl_ol, buf, len);
				break;
#endif // endif
			case BCM_OL_EL_START:
			case BCM_OL_EL_SEND_REPORT:
			case BCM_OL_EL_REPORT:
				/*
				 * currently enable/disable is done when system enters
				 * WoWL mode. So These commands will be enabled later
				 * when we reallhy need it
				 */
				/*
				wlc_dngl_ol_eventlog_send_proc(wlc_dngl_ol, msg_hdr->type);
				*/
				break;
			default:
				WL_ERROR(("%s: Error: Unsupported hdr.type 0x%x\n",
					__FUNCTION__, msg_hdr->hdr.type));
				break;
		}
	}
	return 0;
}

/* Send a character array out */
int
generic_send_packet(wlc_dngl_ol_info_t *ol_info, uchar *params, uint p_len)
{
	bool status;
	osl_t *osh = ol_info->osh;
	void *pkt;

	/* Reject runts and jumbos */
	if (p_len > ETHER_MAX_LEN || params == NULL) {
		WL_ERROR(("%s: Error: p_len %d\n", __FUNCTION__, p_len));
		return BCME_BADARG;
	}
	pkt = PKTGET(osh, p_len + TXOFF, TRUE);
	if (pkt == NULL) {
		WL_ERROR(("%s: TXOFF failed\n", __FUNCTION__));
		return BCME_NOMEM;
	}

	PKTPULL(osh, pkt, TXOFF);
	bcopy(params, PKTDATA(osh, pkt), p_len);
	PKTSETLEN(osh, pkt, p_len);

	/* WL_LOCK(); */
	status = wlc_sendpkt(ol_info->wlc, pkt, NULL);
	/* W_UNLOCK(); */

	if (status) {
		WL_ERROR(("%s Error: wlc_sendpkt returns %d\n", __FUNCTION__, status));
		return BCME_NORESOURCE;
	}
	return 0;
}

void
wlc_dngl_cntinc(wlc_dngl_ol_info_t *wlc_dngl_ol, uint counter)
{
	switch (counter) {
		case TXSUPPRESS:
			RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.txsupressed);
			break;
		case TXACKED:
			RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.txacked);
			break;
		case TXPROBEREQ:
			RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.probereq);
			RXOEINC(wlc_dngl_ol, rxoe_txpktcnt.tottxpkt);
			break;
		default:
			break;
	}
}

wlc_bsscfg_t*
wlc_dngl_ol_get_bsscfg(wlc_dngl_ol_info_t *dngl_ol, int idx)
{
	return (idx < dngl_ol->max_bsscfg) ? &dngl_ol->bsscfg[idx] : NULL;
}

scb_t *
wlc_dngl_ol_get_scb(wlc_dngl_ol_info_t *dngl_ol, int idx)
{
	return (idx < dngl_ol->max_scb) ? &dngl_ol->scb[idx] : NULL;
}

void
wlc_dngl_ol_iv_update(wlc_dngl_ol_info_t *dngl_ol,
	const wlc_key_info_t *key_info, const uint8 *seq, size_t seq_len,
	wlc_key_seq_id_t seq_id, bool tx)
{
	volatile ol_sec_info* ol_si;
	volatile ol_key_info *ol_ki = NULL;
	volatile ol_iv_t *ol_iv;

	ASSERT(seq_len == WOWL_TSCPN_SIZE);

	/* we skip updating the tx_info in the dngl_ol, but just update wake tx info
	 * in case of sudden wakeup/reset of host/arm. Under orderly wake, dngl_ol is updated
	 * from key mgmt state, which is then copied into wake info.
	 * we skip some error checking here because this is called in rx/tx path
	 */
	ol_si = &RXOETXINFO(dngl_ol)->sec_info;
	if (WLC_KEY_IS_GROUP(key_info)) {
		ol_ki  = &ol_si->bss_key_info[key_info->key_id];
	} else {
		ASSERT(WLC_KEY_IS_PAIRWISE(key_info)); /* rules out ibss and mgmt group */
		ol_ki  = &ol_si->scb_key_info;
	}

	ASSERT(ol_ki != NULL);
	ol_iv = tx ? &ol_ki->txiv : &ol_ki->rxiv[seq_id];
	bcopy(seq, (uint8 *)ol_iv->buf, MIN(WOWL_TSCPN_SIZE, (int)seq_len));
}

#ifdef MFP
static bool
wlc_dngl_ol_mfp_rx(wlc_dngl_ol_info_t *wlc_dngl_ol,
	d11rxhdr_t *rxhdr, struct dot11_management_header *hdr, void *p)
{
	uint16 fc = ltoh16(hdr->fc);
	bool ret = TRUE;
	uint8 *body;
	int body_len;
	wlc_bsscfg_t *bsscfg = &wlc_dngl_ol->bsscfg[0];
	wlc_keymgmt_t *km = DNGL_OL_KEYMGMT(wlc_dngl_ol);
	wlc_key_t *key;
	wlc_key_info_t key_info;
	scb_t *scb = &wlc_dngl_ol->scb[0];

	WL_TRACE(("%s: management frame "
		"type = 0x%02x, subtype = 0x%02x\n", __FUNCTION__,
		FC_TYPE(fc), FC_SUBTYPE(fc)));

	body = (uchar*)PKTDATA(wlc_dngl_ol->osh, p);
	body_len = PKTLEN(wlc_dngl_ol->osh, p) - DOT11_FCS_LEN;

	if (ETHER_ISMULTI(&hdr->da)) { /* bcast/mcast */
		mmic_ie_t *ie;
		int ie_len;
		wlc_key_algo_t bss_algo;

		if (fc & FC_WEP) { /* 8.2.4.1.9 IEEE 802.11/2012 */
			WL_WSEC(("%s: multicast frame "
				"with protected frame bit set, toss\n",
				__FUNCTION__));
			WL_ERROR(("%s: multicast frame "
				"with protected frame bit set, toss\n",
				__FUNCTION__));
			return FALSE;
		}

		bss_algo = wlc_keymgmt_get_bss_key_algo(wlc_dngl_ol->wlc->keymgmt, bsscfg, TRUE);
		if (bss_algo == CRYPTO_ALGO_NONE)
			return FALSE;

		ie_len = OFFSETOF(mmic_ie_t, mic) +
			((bss_algo == CRYPTO_ALGO_BIP) ?  BIP_MIC_SIZE : AES_BLOCK_SZ);

		if (body_len < ie_len) {
			WL_WSEC(("%s: mmie error: body length %d too small\n",
				__FUNCTION__, body_len));
			WL_ERROR(("%s: mmie error: body length %d too small\n",
				__FUNCTION__, body_len));
			return FALSE;
		}

		ie = (mmic_ie_t *)(body + body_len - ie_len);
		key = wlc_keymgmt_get_bss_key(km, bsscfg,
			(wlc_key_id_t)ltoh16_ua(&ie->key_id), &key_info);
	} else {
		ASSERT(scb != NULL);
		key = wlc_keymgmt_get_scb_key(km, scb, WLC_KEY_ID_PAIRWISE,
			WLC_KEY_FLAG_NONE, &key_info);
	}

	PKTPUSH(wlc_dngl_ol->osh, p, sizeof(struct dot11_management_header));

	/* Set SCB in packet to be parsed by wlc_key_rx_mpdu */
	WLPKTTAGSCBSET(p, &wlc_dngl_ol->scb[0]);
	if (wlc_key_rx_mpdu(key, p, rxhdr) != BCME_OK) {
		ret = FALSE;
	}

	return ret;
}

#endif /* MFP */
