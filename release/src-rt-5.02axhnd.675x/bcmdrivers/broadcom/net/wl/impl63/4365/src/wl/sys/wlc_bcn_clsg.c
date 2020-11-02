/*
 * Beacon Coalescing related source file
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_bcn_clsg.c 708017 2017-06-29 14:11:45Z $
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
 * XXX Twiki: [BeaconOffload]
 */

#ifndef WL_BCN_COALESCING
#error "WL_BCN_COALESCING is not defined"
#endif // endif

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <nicpci.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wl_export.h>

#include <wlc_bcn_clsg.h>

#ifdef WL_BCN_COALESCING

/* iovar table */
enum {
	IOV_BCN_COALESCING	/* enable/disable bcn coalescing */
};

static const bcm_iovar_t bcn_clsg_iovars[] = {
	{"bcn_coalescing", IOV_BCN_COALESCING,
	(0), IOVT_INT32, 0
	},
	{NULL, 0, 0, 0, 0 }
};

/* Beacon Coalescing module info */
struct wlc_bcn_clsg_info {
	wlc_info_t *wlc;				/* pointer to main wlc structure */
	bool 		updn;				/* flag for bsscfg_updown_register() */
	d11rxhdr_t	rxh;	/* This field stores d11rxhdr for our BSS bcn before
						 * enabling bcn offloading.
						 */
	uint32		disable_mask;	/* Current state of bcn aggregation. */
	uint32		cur_buff_size;	/* Currently aggregated bcn size in ucode */
};

#ifdef BCMDBG
static int wlc_bcn_clsg_dump(wlc_bcn_clsg_info_t *bc, struct bcmstrbuf *b);
#endif // endif
static int wlc_bcn_clsg_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                 void *p, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_bcn_clsg_up(wlc_bcn_clsg_info_t *bc);
static int wlc_bcn_clsg_down(wlc_bcn_clsg_info_t *bc);

static void wlc_bcn_clsg_bsscfg_updn(void *ctx, bsscfg_up_down_event_data_t *evt);

/*
 * Initialize bcn coalescing private context. It returns a pointer to the
 * private context if succeeded. Otherwise it returns NULL.
 */

wlc_bcn_clsg_info_t *
BCMATTACHFN(wlc_bcn_clsg_attach)(wlc_info_t *wlc)
{
	wlc_bcn_clsg_info_t *bc;

	if (!wlc)
		return NULL;

	/* Allocate psta private info struct */
	bc = MALLOCZ(wlc->pub->osh, sizeof(wlc_bcn_clsg_info_t));
	if (!bc) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
		return NULL;
	}

	/* Init bcn coalescing private info struct */
	bc->wlc = wlc;
	bc->disable_mask = BCN_CLSG_ASSOC_MASK | BCN_CLSG_BSS_MASK | BCN_CLSG_UPDN_MASK;

	/* Currently ucode supports bcn coalescing in only below core revs */
	if (D11REV_IS(wlc->pub->corerev, 23) ||
		D11REV_IS(wlc->pub->corerev, 24) ||
		(D11REV_GE(wlc->pub->corerev, 29) && D11REV_LT(wlc->pub->corerev, 40))) {
		WL_INFORM(("wl%d: %s: Enabling bcn coalescing for corerev %d\n",
			wlc->pub->unit,	__FUNCTION__, wlc->pub->corerev));
	} else {
		bc->disable_mask |= BCN_CLSG_CORE_MASK;
	}

	/* bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_bcn_clsg_bsscfg_updn, bc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
			wlc->pub->unit,
			__FUNCTION__));

		goto fail;
	}

	bc->updn = TRUE;

	/* register module */
	if (wlc_module_register(wlc->pub,
		bcn_clsg_iovars,
		"bcn_coalescing",
		bc,
		wlc_bcn_clsg_doiovar,
		NULL,
		(up_fn_t) wlc_bcn_clsg_up,
		(down_fn_t)wlc_bcn_clsg_down) != BCME_OK) {
			WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
			goto fail;
		}

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "bcn_coalescing", (dump_fn_t)wlc_bcn_clsg_dump, (void *)bc);
#endif // endif

	return bc;

	/* error handling */
fail:
	wlc_bcn_clsg_detach(bc);
	return NULL;
}

#ifdef BCMDBG
static int
wlc_bcn_clsg_dump(wlc_bcn_clsg_info_t *bc, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "bcn coalesing disable mask : 0x%x\n", bc->disable_mask);
	return 0;
}
#endif /* BCMDBG */

void
BCMATTACHFN(wlc_bcn_clsg_detach)(wlc_bcn_clsg_info_t *bc)
{
	if (bc == NULL)
		return;

	/* Free the bcn coalescing info struct */
	wlc_module_unregister(bc->wlc->pub, "bcn_coalescing", bc);

	if (bc->updn == TRUE)
		wlc_bsscfg_updown_unregister(bc->wlc, wlc_bcn_clsg_bsscfg_updn, bc);
	bc->updn = FALSE;

	MFREE(bc->wlc->osh, bc, sizeof(wlc_bcn_clsg_info_t));
}

/*
 * Process up
 */
static int
wlc_bcn_clsg_up(wlc_bcn_clsg_info_t *bc)
{
	ASSERT(bc);
	wlc_bcn_clsg_disable(bc, BCN_CLSG_UPDN_MASK, 0);
	return BCME_OK;
}

/*
 * Process down
 */
static int
wlc_bcn_clsg_down(wlc_bcn_clsg_info_t *bc)
{
	ASSERT(bc);
	wlc_bcn_clsg_disable(bc, BCN_CLSG_UPDN_MASK, BCN_CLSG_UPDN_MASK);
	return BCME_OK;
}

/* handle bcn coalescing related iovars */
static int
wlc_bcn_clsg_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                 void *p, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_bcn_clsg_info_t *bc = (wlc_bcn_clsg_info_t *)hdl;
	int32 int_val = 0;
	bool bool_val;
	int err = 0;
	wlc_info_t *wlc;
	int32 *ret_int_ptr = (int32 *)arg;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	wlc = bc->wlc;
	ASSERT(bc == wlc->bc);

	switch (actionid) {
	case IOV_GVAL(IOV_BCN_COALESCING):

		if (bc->disable_mask & BCN_CLSG_CONFIG_MASK)
			*ret_int_ptr = OFF;
		else
			*ret_int_ptr = AUTO;
		break;

	case IOV_SVAL(IOV_BCN_COALESCING):

		if ((int_val != OFF) && (int_val != AUTO)) {
			err = BCME_RANGE;
			break;
		}

		wlc_bcn_clsg_disable(bc, BCN_CLSG_CONFIG_MASK, bool_val ? 0: BCN_CLSG_CONFIG_MASK);
		break;

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

/*
 * BSS up/down handler that we register for bcn coalescing module.
 */
static void
wlc_bcn_clsg_bsscfg_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_bcn_clsg_info_t *bc  = (wlc_bcn_clsg_info_t *)ctx;
	wlc_info_t *wlc = bc->wlc;
	wlc_bsscfg_t   *cfg;
	int idx, total = 0, ap = 0;
	bool enable = FALSE;

	ASSERT(ctx != NULL);
	ASSERT(evt != NULL);

	FOREACH_BSS(wlc, idx, cfg) {
		if (cfg->up) {
			total++;
			if (BSSCFG_AP(cfg))
				ap++;
		}
	}

	if ((evt->bsscfg->up) && (evt->up == FALSE)) {
		/* This BSS is coming down */
		total--;
		if (BSSCFG_AP(evt->bsscfg))
			ap--;
	}

	cfg = wlc->cfg;

	if ((total == 1) &&
		(ap == 0) &&
		(cfg != NULL) &&
		(cfg->up) &&
		(cfg->BSS) &&
		(!BSSCFG_AP(cfg))) {
		enable = TRUE;
	}

	wlc_bcn_clsg_disable(bc, BCN_CLSG_BSS_MASK,
		enable ? 0 : BCN_CLSG_BSS_MASK);
}

/* This functions returns true if there have been more beaconns
 * coalesced in ucode from the last time this function was called
 */
bool
wlc_bcn_clsg_in_ucode(wlc_bcn_clsg_info_t *bc, bool time_since_bcn, bool *flushed)
{
	wlc_info_t *wlc = bc->wlc;
	uint32 bcn_clsg_sz = 0;
	bool coalescing = FALSE;
	*flushed = FALSE;

	/* Return false if ucode does not support bcn clsg */
	if (bc->disable_mask)
		return coalescing;

	if (!wlc->pub->up) {
		WL_ERROR(("wl%d: %s: state down\n",
			wlc->pub->unit, __FUNCTION__));
		return coalescing;
	}

	ASSERT(wlc->clk);
	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: no clock\n",
			wlc->pub->unit, __FUNCTION__));
		return coalescing;
	}

	/* Read if ucode has beacons buffered */
	bcn_clsg_sz = wlc_read_shm(wlc, SHM_BCN_AGG_CUR_BUFF_SIZE);

	if (bcn_clsg_sz &&
		(bcn_clsg_sz != bc->cur_buff_size))
		coalescing = TRUE;

	if (time_since_bcn && bcn_clsg_sz) {
		d11regs_t *regs = wlc->regs;
		WL_INFORM(("\ntime_since_bcn %d, bcn_clsg_sz %d", time_since_bcn, bcn_clsg_sz));
		/* release the beacons from ucode to make sure
		 * we get at least one rssi sample every sec
		 */
		OR_REG(wlc->osh, &regs->maccommand, MCMD_BCNREL);
		bcn_clsg_sz = 0;
		*flushed = TRUE;
	}
	bc->cur_buff_size = bcn_clsg_sz;

	return coalescing;
}

void
wlc_bcn_clsg_disable(wlc_bcn_clsg_info_t *bc, uint32 mask, uint32 val)
{

	wlc_info_t *wlc = bc->wlc;

	bc->disable_mask = (bc->disable_mask & ~mask) | (val & mask);

	if (!wlc->pub->up) {
		WL_INFORM(("wl%d: %s: state down, deferring setting of shmem, mask 0x%x\n",
			wlc->pub->unit, __FUNCTION__, bc->disable_mask));
		return;
	}

	ASSERT(wlc->clk);
	if (!wlc->clk) {
		WL_ERROR(("wl%d: %s: no clock, deferring setting of shmem\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (bc->disable_mask) {
		/* Write how many beacons to accumulate */
		wlc_write_shm(wlc, SHM_BCN_AGG_MAX_BUFF_SIZE, 0);

		/* Write AID */
		wlc_write_shm(wlc, SHM_BCN_AGG_STA_AID, 0);

		/* Clear the cur_buff_size value */
		bc->cur_buff_size = 0;

		WL_INFORM(("%s: Disable 0x%X\n", __FUNCTION__, bc->disable_mask));

		bc->rxh.RxTSFTime = 0;
	} else {

		wlc_bsscfg_t *cfg = wlc->cfg;

		/* Write how much max beacon buffer to fill */
		wlc_write_shm(wlc, SHM_BCN_AGG_MAX_BUFF_SIZE, (RXBUFSZ - 200));

		/* Write AID */
		wlc_write_shm(wlc, SHM_BCN_AGG_STA_AID, cfg->AID & DOT11_AID_MASK);
		WL_INFORM(("%s: Enable\n", __FUNCTION__));
	}
	return;
}

/* this is a generic function and not specific to bcn
 * coalescing.
 */
uint32
wlc_get_len_from_plcp(wlc_d11rxhdr_t *wrxh, uint8 *plcp)
{

	d11rxhdr_t *rxh = &wrxh->rxhdr;
	uint16 phy_ft, usec = 0;
	uint32 length = 0;
	ratespec_t rspec = RSPEC_BW_20MHZ;
	int rate_500;

	phy_ft = rxh->PhyRxStatus_0 & PRXS0_FT_MASK;

	switch (phy_ft) {
	case PRXS0_CCK:

		rspec = CCK_PHY2MAC_RATE(((cck_phy_hdr_t *)plcp)->signal);
		rate_500 = RSPEC2RATE(rspec);

		ASSERT(IS_CCK(rspec));
		if (!IS_CCK(rspec))
			break;

		usec = ((cck_phy_hdr_t *)plcp)->length;
		{

		/*
		b0	Reserved
		b1	Reserved
		b2	Locked clocks bit: 0 = not 1 = locked
		b3	Mod. selection bit: 0 = CCK	1 = PBCC
		b4	Reserved
		b5	Reserved
		b6	Reserved
		b7	Length extension bit

		Number of octets = Floor (Length  x  R / 8 - P - Length Extension)

		where

		R is the data rate in Mbit/s
		P = 0 for CCK; 1 for PBCC
		Ceiling (X) returns the smallest integer value greater than or equal to X
		Floor (X) returns the largest integer value less than or equal to X.
		*/

			int ext = 0, p = 0;
			if ((((cck_phy_hdr_t *)plcp)->service)&0x08)
				p = 1;

			if ((((cck_phy_hdr_t *)plcp)->service)&0x80)
				ext = 1;

			length = ((usec*rate_500/2)/8)- p - ext;
		}

		break;
	case PRXS0_OFDM:

		rspec = OFDM_PHY2MAC_RATE(((ofdm_phy_hdr_t *)plcp)->rlpt[0]);

		ASSERT(IS_OFDM(rspec));
		if (!IS_OFDM(rspec))
			break;

		length = D11A_PHY_HDR_GLENGTH(((ofdm_phy_hdr_t *)plcp));
		break;
	default:
		ASSERT(0);
		break;
	}

	return length;
}

void
wlc_bcn_clsg_store_rxh(wlc_bcn_clsg_info_t *bc, wlc_d11rxhdr_t *wrxh)
{
	if ((bc->disable_mask) &&
		(bc->rxh.RxTSFTime == 0)) {
		bc->rxh = wrxh->rxhdr;
	}
}

void
wlc_bcn_clsg_update_rxh(wlc_bcn_clsg_info_t *bc, wlc_d11rxhdr_t *wrxh)
{
	d11rxhdr_t *rxh;
	rxh = &wrxh->rxhdr;
	if (bc->disable_mask == 0) {
		if (rxh->RxStatus2 & RXS_BCNCLSG) {
			rxh->PhyRxStatus_0 = bc->rxh.PhyRxStatus_0;
			rxh->PhyRxStatus_4 = bc->rxh.PhyRxStatus_4;
			rxh->PhyRxStatus_5 = bc->rxh.PhyRxStatus_5;
			rxh->RxStatus1 = bc->rxh.RxStatus1;
			rxh->RxStatus2 = bc->rxh.RxStatus2 | RXS_BCNCLSG;
			rxh->RxChan = bc->rxh.RxChan;		}
	}
}

#endif /* WL_BCN_COALESCING */
