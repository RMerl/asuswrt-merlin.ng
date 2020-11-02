/*
 * Common (OS-independent) portion of
 * Broadcom 802.11bang Networking Device Driver
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
 * $Id: wlc_addrmatch.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * This file implements the address matching interface to be used
 * by the high driver or monolithic driver. d11 corerev >= 40 supports
 * AMT with attributes for matching in addition to the address. Prior
 * versions ignore the attributes provided in the interface
 */

#include <wlc_cfg.h>

#if defined(WLC_LOW) && !defined(WLC_HIGH)
#error "File is for use in monolithic or high driver only"
#endif  /* WLC_LOW */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <proto/ethernet.h>
#include <proto/bcmeth.h>
#include <proto/bcmevent.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_bmac.h>
#include <wlc_txbf.h>

#include <wlc_addrmatch.h>

#define HAS_AMT(wlc) D11REV_GE(wlc->pub->corerev, 40)
#define IS_PRE_AMT(wlc) D11REV_LT(wlc->pub->corerev, 40)

#ifdef ACKSUPR_MAC_FILTER
#define ADDRMATCH_INFO_STATUS_GET(wlc, idx) wlc->addrmatch_info[idx].status
#define ADDRMATCH_INFO_STATUS_SET(wlc, idx, val) wlc->addrmatch_info[idx].status = val

/* amt entry status */
enum {
	ADDRMATCH_INIT = 0,
	ADDRMATCH_EMPTY,
	ADDRMATCH_USED,
	ADDRMATCH_NEED_DELETE
};

/* for acksupr amt info */
struct wlc_addrmatch_info {
	int8 status;
};

int
wlc_addrmatch_info_alloc(wlc_info_t *wlc, int max_entry_num)
{
	if (wlc->addrmatch_info == NULL) {
		int i;
		struct ether_addr ea;
		uint16 attr;

		wlc->addrmatch_info = MALLOC(wlc->osh,
			sizeof(wlc_addrmatch_info_t) * max_entry_num);
		if (wlc->addrmatch_info == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			         wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return BCME_NOMEM;
		}
		memset(wlc->addrmatch_info, ADDRMATCH_INIT,
			sizeof(wlc_addrmatch_info_t) * max_entry_num);

		for (i = 0; i < max_entry_num; i++) {
			/* generic used entry sync */
			wlc_get_addrmatch(wlc, i, &ea, &attr);
			if (attr)
				ADDRMATCH_INFO_STATUS_SET(wlc, i, ADDRMATCH_USED);
			else
				ADDRMATCH_INFO_STATUS_SET(wlc, i, ADDRMATCH_EMPTY);
		}
	}
	return BCME_OK;
}

void
wlc_addrmatch_info_free(wlc_info_t *wlc, int max_entry_num)
{
	if (wlc->addrmatch_info != NULL) {
		MFREE(wlc->osh, wlc->addrmatch_info,
			sizeof(wlc_addrmatch_info_t) * max_entry_num);
		wlc->addrmatch_info = NULL;
	}
	return;
}
#endif /* ACKSUPR_MAC_FILTER */

uint16
wlc_set_addrmatch(wlc_info_t *wlc, int idx, const struct ether_addr *addr,
	uint16 attr)

{
	uint16 prev_attr = 0;
#ifdef ACKSUPR_MAC_FILTER
	int slot = idx;
#endif /* ACKSUPR_MAC_FILTER */

#ifdef BCMDBG
	if (WL_WSEC_ON()) {
		char addr_str[ETHER_ADDR_STR_LEN];
		WL_WSEC(("wl%d: %s: idx %d addr %s attr 0x%04x\n", WLCWLUNIT(wlc),
			__FUNCTION__, idx, bcm_ether_ntoa(addr, addr_str), attr));
	}
#endif /* BCMDBG */

	ASSERT(wlc->pub->corerev > 4);
	if (HAS_AMT(wlc)) {
		switch (idx) {
		case WLC_ADDRMATCH_IDX_MAC:
			prev_attr = wlc_bmac_write_amt(wlc->hw, AMT_IDX_MAC, addr, attr);
#ifdef ACKSUPR_MAC_FILTER
			slot = AMT_IDX_MAC;
#endif /* ACKSUPR_MAC_FILTER */
			break;
		case WLC_ADDRMATCH_IDX_BSSID:
			prev_attr = wlc_bmac_write_amt(wlc->hw, AMT_IDX_BSSID, addr, attr);
#ifdef ACKSUPR_MAC_FILTER
			slot = AMT_IDX_BSSID;
#endif /* ACKSUPR_MAC_FILTER */
			break;
		default:
			ASSERT(idx >= 0);
			if (idx < (int)wlc->pub->max_addrma_idx) {
				prev_attr = wlc_bmac_write_amt(wlc->hw, idx, addr, attr);
#ifdef WL_BEAMFORMING
				if (TXBF_ENAB(wlc->pub))
					wlc_txfbf_update_amt_idx(wlc->txbf, idx, addr);
#endif // endif
			}
			break;
		}
		goto done;
	}

	switch (idx) {
	case WLC_ADDRMATCH_IDX_MAC:
		wlc_bmac_set_rxe_addrmatch(wlc->hw, RCM_MAC_OFFSET, addr);
#ifdef ACKSUPR_MAC_FILTER
		slot = RCM_MAC_OFFSET;
#endif /* ACKSUPR_MAC_FILTER */
		break;
	case WLC_ADDRMATCH_IDX_BSSID:
		wlc_bmac_set_rxe_addrmatch(wlc->hw, RCM_BSSID_OFFSET, addr);
#ifdef ACKSUPR_MAC_FILTER
		slot = RCM_BSSID_OFFSET;
#endif /* ACKSUPR_MAC_FILTER */
		break;
	default:
		ASSERT(idx >= 0);
		if (idx < RCMTA_SIZE)
			wlc_bmac_set_rcmta(wlc->hw, idx, addr);
		break;
	}

done:
#ifdef ACKSUPR_MAC_FILTER
	if (WLC_ACKSUPR(wlc)) {
		if (attr)
			ADDRMATCH_INFO_STATUS_SET(wlc, slot, ADDRMATCH_USED);
		else
			ADDRMATCH_INFO_STATUS_SET(wlc, slot, ADDRMATCH_EMPTY);
	}
#endif /* ACKSUPR_MAC_FILTER */
	return prev_attr;
}

uint16
wlc_clear_addrmatch(wlc_info_t *wlc, int idx)
{
	return wlc_set_addrmatch(wlc, idx, &ether_null, 0);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WL_BEAMFORMING) || \
	defined(ACKSUPR_MAC_FILTER)
void
wlc_get_addrmatch(wlc_info_t *wlc, int idx, struct ether_addr *addr,
	uint16 *attr)
{
	ASSERT(wlc->pub->corerev > 4);
#ifdef ACKSUPR_MAC_FILTER
	if (WLC_ACKSUPR(wlc) &&
		(ADDRMATCH_INFO_STATUS_GET(wlc, idx) == ADDRMATCH_EMPTY)) {
		*attr = 0;
		memset(addr, 0, sizeof(*addr));
		return;
	}
#endif /* ACKSUPR_MAC_FILTER */
	if (HAS_AMT(wlc)) {
		switch (idx) {
		case WLC_ADDRMATCH_IDX_MAC: idx = AMT_IDX_MAC; break;
		case WLC_ADDRMATCH_IDX_BSSID: idx = AMT_IDX_BSSID; break;
		default: break;
		}
		wlc_bmac_read_amt(wlc->hw, idx, addr, attr);
		return;
	}

	/* no support for reading the rxe address match registers for now.
	 * can be added if necessary by supporting it in the bmac layer
	 * and the corresponding RPCs for the split driver.
	 */
	if (idx >= 0) {
		wlc_bmac_get_rcmta(wlc->hw, idx, addr);
		*attr =  !ETHER_ISNULLADDR(addr) ? AMT_ATTR_VALID : 0;
#ifdef ACKSUPR_MAC_FILTER
		if (WLC_ACKSUPR(wlc) && (*attr == 0) &&
			(ADDRMATCH_INFO_STATUS_GET(wlc, idx) == ADDRMATCH_USED))
			*attr = AMT_ATTR_VALID;
#endif /* ACKSUPR_MAC_FILTER */
	} else {
		memset(addr, 0, sizeof(*addr));
		*attr = 0;
	}
}
#endif // endif

#if defined(BCMDBG_DUMP)
static int
wlc_dump_rcmta(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int i;
	struct ether_addr ea;
	char eabuf[ETHER_ADDR_STR_LEN];

	ASSERT(IS_PRE_AMT(wlc));

	if (!wlc->clk)
		return BCME_NOCLK;

	for (i = 0; i < RCMTA_SIZE; i ++) {
		uint16 attr;
		wlc_get_addrmatch(wlc, i, &ea, &attr);
		if (ETHER_ISNULLADDR(&ea) && !(attr & AMT_ATTR_VALID))
			continue;
		bcm_bprintf(b, "%d %s\n", i, bcm_ether_ntoa(&ea, eabuf));
	}

	return BCME_OK;
}

static int
wlc_dump_amt(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int i;
	struct ether_addr ea;
	uint16 attr;
	char flagstr[64];
	static const bcm_bit_desc_t attr_flags[] = {
		{AMT_ATTR_VALID, "Valid"},
		{AMT_ATTR_A1, "A1"},
		{AMT_ATTR_A2, "A2"},
		{AMT_ATTR_A3, "A3"},
		{0, NULL}
	};

	ASSERT(HAS_AMT(wlc));

	if (!wlc->clk)
		return BCME_NOCLK;

	for (i = 0; i < (int)wlc->pub->max_addrma_idx; i ++) {
		wlc_get_addrmatch(wlc, i, &ea, &attr);
		if (ETHER_ISNULLADDR(&ea) && !(attr & AMT_ATTR_VALID))
			continue;

		bcm_bprintf(b, "%02d "MACF" 0x%04x", i, ETHER_TO_MACF(ea), attr);
		if (attr != 0) {
			bcm_format_flags(attr_flags, attr, flagstr, 64);
			bcm_bprintf(b, " (%s)", flagstr);
		}
		bcm_bprintf(b, " amtinfo 0x%04x\n", wlc_read_amtinfo_by_idx(wlc, i));
	}

	return BCME_OK;
}

int
wlc_dump_addrmatch(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	ASSERT(wlc->pub->corerev > 4);

	if (!wlc->clk)
		return BCME_NOCLK;

	if (HAS_AMT(wlc)) {
		wlc_dump_amt(wlc, b);
		return BCME_OK;
	}
	wlc_dump_rcmta(wlc, b);
	return BCME_OK;
}
#endif // endif
