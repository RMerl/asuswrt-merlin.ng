/*
 * Public H/W info of
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
 * $Id: wlc_hw.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#ifdef WLC_HIGH
#include <wlc_bsscfg.h>
#endif // endif
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>

/* local functions */
#ifdef WLC_HIGH
#ifdef BCMDBG
static int wlc_hw_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
#endif /* WLC_HIGH */

wlc_hw_info_t *
BCMATTACHFN(wlc_hw_attach)(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err)
{
	wlc_hw_info_t *wlc_hw;
#ifdef WLC_LOW
	int i;
#endif // endif

	if ((wlc_hw = (wlc_hw_info_t *)
	     MALLOCZ(osh, sizeof(wlc_hw_info_t))) == NULL) {
		*err = 1010;
		goto fail;
	}
	wlc_hw->wlc = wlc;
	wlc_hw->osh = osh;

#ifdef WLC_LOW
	wlc_hw->unit = unit;

	/* Though bzero of wlc_hw takes care of macunit to zero,
	 * Explicitly showing here required in dual nic setup usage.
	 * XXX:If there is more than one card, and one card is of rsdb chip :)
	 */
	wlc_hw->macunit = 0;

#ifdef DONGLEBUILD
	wlc_hw->macunit = si_coreunit(wlc->pub->sih);
#endif /* DONGLEBUILD */

	if ((wlc_hw->btc = (wlc_hw_btc_info_t*)
	     MALLOCZ(osh, sizeof(wlc_hw_btc_info_t))) == NULL) {
		*err = 1011;
		goto fail;
	}

	if ((wlc_hw->bandstate[0] = (wlc_hwband_t*)
	     MALLOCZ(osh, sizeof(wlc_hwband_t) * MAXBANDS)) == NULL) {
		*err = 1012;
		goto fail;
	}

	for (i = 1; i < MAXBANDS; i++) {
		wlc_hw->bandstate[i] = (wlc_hwband_t *)
		        ((uintptr)wlc_hw->bandstate[0] + sizeof(wlc_hwband_t) * i);
	}

	for (i = 0; i < MAXBANDS; i++) {
		if ((wlc_hw->bandstate[i]->mhfs = (uint16 *)
			MALLOCZ(osh, sizeof(uint16)*(MXHF0+MXHFMAX))) == NULL) {
			*err = 1016;
			goto fail;
		}
	}
#endif /* WLC_LOW */

	if ((wlc_hw->pub = MALLOCZ(osh, sizeof(wlc_hw_t))) == NULL) {
		*err = 1013;
		goto fail;
	}

#ifdef WLC_HIGH
#ifdef BCMDBG
	if (wlc_dump_register(wlc->pub, "hw", wlc_hw_dump, wlc_hw) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
		          unit, __FUNCTION__));
		*err = 1014;
		goto fail;
	}
#endif // endif
#endif /* WLC_HIGH */

#ifdef WLC_LOW
	if ((wlc_hw->txavail = MALLOCZ(osh, (NFIFO_EXT*sizeof(*wlc_hw->txavail)))) == NULL) {
		*err = 1015;
		goto fail;
	}
#ifdef HOST_HDR_FETCH
	if (HOST_HDR_FETCH_ENAB()) {
		if ((wlc_hw->dma_pending = MALLOCZ(osh,
			(NFIFO_EXT * sizeof(*wlc_hw->dma_pending)))) == NULL) {
			*err = 1016;
			goto fail;
		}
	}
#endif /* HOST_HDR_FETCH */
#endif /* WLC_LOW */
	*err = 0;

	/* XXX do we have a better place for the assignment?
	 * maybe at the caller but that requires an API to return wlc_hw->pub...
	 */
	wlc->hw_pub = wlc_hw->pub;
	return wlc_hw;

fail:
	wlc_hw_detach(wlc_hw);
	return NULL;
}

void
BCMATTACHFN(wlc_hw_detach)(wlc_hw_info_t *wlc_hw)
{
	osl_t *osh;
	wlc_info_t *wlc;
#ifdef WLC_LOW
	int i;
#endif // endif

	if (wlc_hw == NULL)
		return;

	osh = wlc_hw->osh;
	wlc = wlc_hw->wlc;

	wlc->hw_pub = NULL;

#ifdef WLC_LOW
	if (wlc_hw->btc != NULL)
		MFREE(osh, wlc_hw->btc, sizeof(wlc_hw_btc_info_t));

	if (wlc_hw->bandstate[0] != NULL) {
		for (i = 0; i < MAXBANDS; i++) {
			if (wlc_hw->bandstate[i]->mhfs != NULL)
				MFREE(osh, wlc_hw->bandstate[i]->mhfs,
					sizeof(uint16)*(MXHF0+MXHFMAX));
		}
		MFREE(osh, wlc_hw->bandstate[0], sizeof(wlc_hwband_t) * MAXBANDS);
	}

	if (wlc_hw->txavail != NULL)
		MFREE(osh, wlc_hw->txavail, NFIFO_EXT*sizeof(*wlc_hw->txavail));
#ifdef HOST_HDR_FETCH
	if (wlc_hw->dma_pending != NULL)
		MFREE(osh, wlc_hw->dma_pending, NFIFO_EXT*sizeof(*wlc_hw->dma_pending));
#endif /* HOST_HDR_FETCH */
#endif /* WLC_LOW */

	/* free hw struct */
	if (wlc_hw->pub != NULL)
		MFREE(osh, wlc_hw->pub, sizeof(wlc_hw_t));

	/* free hw struct */
	MFREE(osh, wlc_hw, sizeof(wlc_hw_info_t));
}

void
wlc_hw_set_piomode(wlc_hw_info_t *wlc_hw, bool piomode)
{
	wlc_hw->_piomode = piomode;
}

bool
wlc_hw_get_piomode(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->_piomode;
}
void
wlc_hw_set_dmacommon(wlc_hw_info_t *wlc_hw, dma_common_t *dmacommon)
{
	wlc_hw->dmacommon = dmacommon;
}

void
wlc_hw_set_di(wlc_hw_info_t *wlc_hw, uint fifo, hnddma_t *di)
{
	wlc_hw->di[fifo] = di;
	wlc_hw->pub->di[fifo] = di;
}

#ifdef BCM_DMA_CT
void
wlc_hw_set_aqm_di(wlc_hw_info_t *wlc_hw, uint fifo, hnddma_t *di)
{
	wlc_hw->aqm_di[fifo] = di;
	wlc_hw->pub->aqm_di[fifo] = di;
}
#endif // endif

void
wlc_hw_set_pio(wlc_hw_info_t *wlc_hw, uint fifo, pio_t *pio)
{
	wlc_hw->pio[fifo] = pio;
	wlc_hw->pub->pio[fifo] = pio;
}

void
wlc_hw_set_nfifo_inuse(wlc_hw_info_t *wlc_hw, uint fifo_inuse)
{
	wlc_hw->nfifo_inuse = fifo_inuse;
	wlc_hw->pub->nfifo_inuse = fifo_inuse;
}

#ifdef WLC_LOW
bool
wlc_hw_deviceremoved(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->clk ?
	        (R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol) &
	         (MCTL_PSM_JMP_0 | MCTL_IHR_EN)) != MCTL_IHR_EN :
	        si_deviceremoved(wlc_hw->sih);
}

uint32
wlc_hw_get_wake_override(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->wake_override;
}

uint
wlc_hw_get_bandunit(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->band->bandunit;
}

uint32
wlc_hw_get_macintmask(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->macintmask;
}

uint32
wlc_hw_get_macintstatus(wlc_hw_info_t *wlc_hw)
{
	return wlc_hw->macintstatus;
}
#endif /* WLC_LOW */

#ifdef WLC_HIGH
#ifndef WLC_NET80211
/* MHF2_SKIP_ADJTSF muxing, clear the flag when no one requests to skip the ucode
 * TSF adjustment.
 */
void
wlc_skip_adjtsf(wlc_info_t *wlc, bool skip, wlc_bsscfg_t *cfg, uint32 user, int bands)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint global_start = NBITS(wlc_hw->skip_adjtsf) - WLC_SKIP_ADJTSF_USER_MAX;
	int b;

	ASSERT(cfg != NULL || user < WLC_SKIP_ADJTSF_USER_MAX);
	ASSERT(cfg == NULL || (uint32)WLC_BSSCFG_IDX(cfg) < global_start);

	b = (cfg == NULL) ? user + global_start : (uint32)WLC_BSSCFG_IDX(cfg);
	if (skip)
		setbit(&wlc_hw->skip_adjtsf, b);
	else
		clrbit(&wlc_hw->skip_adjtsf, b);

#ifdef BCMDBG
	if (cfg != NULL)
		WL_NONE(("wl%d.%d: wlc->skip_adjtsf 0x%x (skip %d)\n",
		         wlc->pub->unit, WLC_BSSCFG_IDX(cfg), wlc_hw->skip_adjtsf, skip));
	else
		WL_NONE(("wl%d: wlc->skip_adjtsf 0x%x (user %d skip %d)\n",
		         wlc->pub->unit, wlc_hw->skip_adjtsf, user, skip));
#endif // endif

	wlc_bmac_mhf(wlc_hw, MHF2, MHF2_SKIP_ADJTSF,
	        wlc_hw->skip_adjtsf ? MHF2_SKIP_ADJTSF : 0, bands);
}

/* MCTL_AP muxing, set the bit when no one requests to stop the AP functions (beacon, prbrsp) */
void
wlc_ap_ctrl(wlc_info_t *wlc, bool on, wlc_bsscfg_t *cfg, uint32 user)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;

	if (on && wlc_hw->mute_ap != 0) {
		WL_INFORM(("wl%d: ignore %s %d request %d mute_ap 0x%08x\n",
		           wlc->pub->unit, cfg != NULL ? "bsscfg" : "user",
		           cfg != NULL ? WLC_BSSCFG_IDX(cfg) : user, on, wlc_hw->mute_ap));
		return;
	}

	wlc_hw->mute_ap = 0;

	WL_INFORM(("wl%d: %s %d MCTL_AP %d\n",
	           wlc->pub->unit, cfg != NULL ? "bsscfg" : "user",
	           cfg != NULL ? WLC_BSSCFG_IDX(cfg) : user, on));

	wlc_bmac_mctrl(wlc_hw, MCTL_AP, on ? MCTL_AP : 0);
}

#ifdef AP
void
wlc_ap_mute(wlc_info_t *wlc, bool mute, wlc_bsscfg_t *cfg, uint32 user)
{
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint global_start = NBITS(wlc_hw->mute_ap) - WLC_AP_MUTE_USER_MAX;
	int b;
	bool ap;

	ASSERT(cfg != NULL || user < WLC_AP_MUTE_USER_MAX);
	ASSERT(cfg == NULL || (uint32)WLC_BSSCFG_IDX(cfg) < global_start);

	b = (cfg == NULL) ? user + global_start : (uint32)WLC_BSSCFG_IDX(cfg);
	if (mute)
		setbit(&wlc_hw->mute_ap, b);
	else
		clrbit(&wlc_hw->mute_ap, b);

	ap = AP_ACTIVE(wlc) && wlc_hw->mute_ap == 0;

	WL_INFORM(("wl%d: %s %d mute %d mute_ap 0x%x AP_ACTIVE() %d MCTL_AP %d\n",
	           wlc->pub->unit, cfg != NULL ? "bsscfg" : "user",
	           cfg != NULL ? WLC_BSSCFG_IDX(cfg) : user,
	           mute, wlc_hw->mute_ap, AP_ACTIVE(wlc), ap));

	wlc_bmac_mctrl(wlc_hw, MCTL_AP, ap ? MCTL_AP : 0);
}
#endif /* AP */
#endif /* WLC_NET80211 */

#ifdef BCMDBG
static int
wlc_hw_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_hw_info_t *wlc_hw = (wlc_hw_info_t *)ctx;

#ifdef WLC_LOW
	bcm_bprintf(b, "defmacintmask 0x%08x macintmask 0x%08x\n",
	            wlc_hw->defmacintmask, wlc_hw->macintmask);
	bcm_bprintf(b, "forcefastclk %d wake_override 0x%x\n",
	            wlc_hw->forcefastclk, wlc_hw->wake_override);
	bcm_bprintf(b, "fastpwrup_dly %d\n", wlc_hw->fastpwrup_dly);
#endif // endif

	bcm_bprintf(b, "skipadjtsf 0x%08x muteap 0x%08x\n", wlc_hw->skip_adjtsf, wlc_hw->mute_ap);
	bcm_bprintf(b, "p2p: %d\n", wlc_hw->_p2p);

	return BCME_OK;
}
#endif /* BCMDBG */
#endif /* WLC_HIGH */

/*
 * Initialize variables used for beacon/probe response templates.
 * These were pre-compiled constants in the past:
 *
 * SHM_MBSS_BASE_ADDR
 * MBSS_PRS_BLKS_START(n)
 * TXFIFO_SIZE_UNIT
 * TPLBLKS_PER_BCN
 * TPLBLKS_PER_PRS
 *
 * With corerev >= 40, the values changed so they need
 * to be determined at early driver initialization before
 * code start using the above macros.
 */
void BCMATTACHFN(wlc_template_cfg_init)(wlc_info_t *wlc, uint corerev)
{
	if (D11REV_GE(corerev, 40)) {
		wlc->shm_mbss_base = SHM_MBSS_UCODE_AC_BASE;
		wlc->shm_bcn0_tpl_base = D11AC_T_BCN0_TPL_BASE;
		wlc->txfifo_size_unit = TXFIFO_AC_SIZE_PER_UNIT;
		wlc->tplblks_per_bcn = TPLBLKS_AC_PER_BCN_NUM;
		wlc->tplblks_per_prs = TPLBLKS_AC_PER_PRS_NUM;
	} else {
		wlc->shm_mbss_base = SHM_MBSS_UCODE_BASE;
		wlc->shm_bcn0_tpl_base = D11_T_BCN0_TPL_BASE;
		wlc->txfifo_size_unit = TXFIFO_SIZE_PER_UNIT;
		wlc->tplblks_per_bcn = TPLBLKS_PER_BCN_NUM;
		wlc->tplblks_per_prs = TPLBLKS_PER_PRS_NUM;
	}
}
