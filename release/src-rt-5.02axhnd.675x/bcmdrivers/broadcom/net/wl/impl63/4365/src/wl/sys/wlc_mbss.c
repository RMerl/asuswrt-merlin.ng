/*
 * MBSS module
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
 * $Id$
 */

/**
 * @file
 * @brief
 * Twiki: [MBSSIDSupport]
 */

#include "wlc_cfg.h"

#ifdef MBSS

#include <osl.h>
#include <wlioctl.h>
#include <siutils.h>
#include <bcmutils.h>
#include <bcmendian.h>

#include "wlc_rate.h"
#include "wl_dbg.h"
#include "wlc.h"
#include "wlc_mbss.h"
#include "wlc_pub.h"
#include "wlc_bsscfg.h"
#include "wlc_bmac.h"
#include "wlc_ap.h"
#include "wlc_tx.h"
#include "wlc_tso.h"
#include "wlc_scan.h"
#include "wlc_apps.h"
#include "wlc_stf.h"
#include "wlc_pcb.h"

#define WLC_MBSS_BSSCFG_IDX_INVALID	-1

/* forward function declarations */
static int wlc_mbss_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_mbss_up(void *hdl);
static int wlc_bsscfg_macgen(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static int wlc_spt_init(wlc_info_t *wlc, wlc_spt_t *spt, int count, int len);
static void wlc_spt_deinit(wlc_info_t *wlc, wlc_spt_t *spt, int pkt_free_force);
static void mbss_ucode_set(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static void wlc_ap_mbss16_write_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static void wlc_ap_mbss16_write_prbrsp(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool suspend);
static void wlc_mbss16_setup(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static void wlc_mbss16_updssid_len(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
static void bcmc_fid_shm_commit(wlc_bsscfg_t *bsscfg);
#ifdef WLC_LOW
static void wlc_apps_update_bss_bcmc_fid(wlc_info_t *wlc);
#endif // endif
static void wlc_mbss_bcmc_free_cb(wlc_info_t *wlc, void *pkt, uint txs);

/* MBSS debug counters */
typedef struct wlc_mbss_cnt {
	uint32		prq_directed_entries; /* Directed PRQ entries seen */
	uint32		prb_resp_alloc_fail;  /* Failed allocation on probe response */
	uint32		prb_resp_tx_fail;     /* Failed to TX probe response */
	uint32		prb_resp_retrx_fail;  /* Failed to TX probe response */
	uint32		prb_resp_retrx;       /* Retransmit suppressed probe resp */
	uint32		prb_resp_ttl_expy;    /* Probe responses suppressed due to TTL expry */
	uint32		bcn_tx_failed;	      /* Failed to TX beacon frame */

	uint32		mc_fifo_max;	/* Max number of BC/MC pkts in DMA queues */
	uint32		bcmc_count;	/* Total number of pkts sent thru BC/MC fifo */

	/* Timing and other measurements for PS transitions */
	uint32		ps_trans;	/* Total number of transitions started */
} wlc_mbss_cnt_t;

/* bsscfg cubby */
typedef struct {
	wlc_pkt_t	probe_template;	/* Probe response master packet, including PLCP */
	wlc_pkt_t	lprs_template;	/* Legacy probe response master packet */
	bool		prb_modified;	/* Ucode version: push to shm if true */
	wlc_spt_t	*bcn_template;	/* Beacon DMA template */
	int8		_ucidx;		/* the uCode index of this bsscfg,
					 * assigned at wlc_bsscfg_up()
					 */
	uint32		mc_fifo_pkts;	/* Current number of BC/MC pkts sent to DMA queues */
	uint32		prb_ttl_us;     /* Probe rsp time to live since req. If 0, disabled */
	wlc_mbss_cnt_t  *cnt;		/* MBSS debug counters */
#if defined(BCMDBG_MBSS_PROFILE)
	uint32		ps_start_us;	/* When last PS (off) transition started */
	uint32		max_ps_off_us;	/* Max delay time for out-of-PS transitions */
	uint32		tot_ps_off_us;	/* Total time delay for out-of-PS transitions */
	uint32		ps_off_count;	/* Number of deferred out-of-PS transitions completed */
	bool		bcn_tx_done;	/* TX done on sw beacon */
#endif /* BCMDBG_MBSS_PROFILE */
} bss_mbss_info_t;

static int wlc_mbss_info_init(void *hdl, wlc_bsscfg_t *cfg);
static void wlc_mbss_info_deinit(void *hdl, wlc_bsscfg_t *cfg);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_mbss_info_dump(void *hdl, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_mbss_info_dump NULL
#endif // endif

/* bsscfg specific info access accessor */
#define MBSS_BSSCFG_CUBBY_LOC(mbss, cfg) ((bss_mbss_info_t **)BSSCFG_CUBBY((cfg), (mbss)->cfgh))
#define MBSS_BSSCFG_CUBBY(mbss, cfg)	 (*(MBSS_BSSCFG_CUBBY_LOC(mbss, cfg)))
#define BSS_MBSS_INFO(mbss, cfg)	 MBSS_BSSCFG_CUBBY(mbss, cfg)

/* iovar table */
enum {
	IOV_BCN_ROTATE,	/* enable/disable beacon rotation */
	IOV_MBSS
};

static const bcm_iovar_t mbss_iovars[] = {
	{"bcn_rotate", IOV_BCN_ROTATE, 0, IOVT_BOOL, 0},	/* enable/disable beacon rotation */
	{"mbss", IOV_MBSS, IOVF_SET_DOWN, IOVT_BOOL, 0},	/* enable/disable MBSS */
	{NULL, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_mbss_info_t *
BCMATTACHFN(wlc_mbss_attach)(wlc_info_t *wlc)
{
	wlc_mbss_info_t *mbss;

	ASSERT(wlc != NULL);

	/* Allocate info structure */
	if ((mbss = (wlc_mbss_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_mbss_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	mbss->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((mbss->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_mbss_info_t *),
		wlc_mbss_info_init, wlc_mbss_info_deinit, wlc_mbss_info_dump, mbss)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, mbss_iovars, "mbss",
		mbss, wlc_mbss_doiovar, NULL, wlc_mbss_up, NULL)) {
		WL_ERROR(("wl%d: MBSS module register failed\n", wlc->pub->unit));
		goto fail;
	}

	wlc->pub->_mbss_support = TRUE;
	wlc->pub->tunables->maxucodebss = WLC_MAX_AP_BSS(wlc->pub->corerev);

	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_MBSS_BCMC, wlc_mbss_bcmc_free_cb) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	return mbss;

fail:
	wlc_mbss_detach(mbss);

	return NULL;
}

void
BCMATTACHFN(wlc_mbss_detach)(wlc_mbss_info_t *mbss)
{
	if (mbss != NULL) {
		ASSERT(mbss->wlc != NULL && mbss->wlc->pub != NULL);

		mbss->wlc->pub->_mbss_support = FALSE;

		/* unregister */
		wlc_module_unregister(mbss->wlc->pub, "mbss", mbss);

		/* free info structure */
		MFREE(mbss->wlc->osh, mbss, sizeof(wlc_mbss_info_t));
	}
}

/* bsscfg cubby */
static int
wlc_mbss_info_init(void *hdl, wlc_bsscfg_t *cfg)
{
	wlc_mbss_info_t *mbss = (wlc_mbss_info_t *)hdl;
	wlc_info_t *wlc = mbss->wlc;
	bss_mbss_info_t **pbmi = MBSS_BSSCFG_CUBBY_LOC(mbss, cfg);
	bss_mbss_info_t *bmi;
	int err;

	/*
	 * Allocate the MBSS BSSCFG cubby if MBSS is supported.
	 * Note: This would occur even if MBSS is not enabled when wlc_bsscfg_init() is called,
	 * because it may become enabled later (via IOVAR), and therefore the cubby needs to exist.
	 */
	if (!MBSS_SUPPORT(wlc->pub))
		return BCME_OK;

	ASSERT(*pbmi == NULL);

	/* allocate memory and point bsscfg cubby to it */
	if ((bmi = MALLOCZ(wlc->osh, sizeof(*bmi))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	*pbmi = bmi;

	/* allocate bcn_template */
	if ((bmi->bcn_template = (wlc_spt_t *) MALLOCZ(wlc->osh, sizeof(wlc_spt_t))) == NULL) {
		err = BCME_NOMEM;
		goto fail;
	}

#ifdef WLCNT
	if ((bmi->cnt = (wlc_mbss_cnt_t *) MALLOCZ(wlc->osh, sizeof(wlc_mbss_cnt_t))) == NULL) {
		err = BCME_NOMEM;
		goto fail;
	}
#endif	/* WLCNT */

	return BCME_OK;

fail:
	wlc_mbss_info_deinit(hdl, cfg);

	return err;
}

static void
wlc_mbss_info_deinit(void *hdl, wlc_bsscfg_t *cfg)
{
	wlc_mbss_info_t *mbss = (wlc_mbss_info_t *)hdl;
	wlc_info_t *wlc = mbss->wlc;
	bss_mbss_info_t **pbmi = MBSS_BSSCFG_CUBBY_LOC(mbss, cfg);
	bss_mbss_info_t *bmi = *pbmi;

	if (bmi == NULL)
		return;

#ifdef WLCNT
	if (bmi->cnt)
		MFREE(wlc->osh, bmi->cnt, sizeof(*(bmi->cnt)));
#endif	/* WLCNT */

	if (bmi->bcn_template)
		MFREE(wlc->osh, bmi->bcn_template, sizeof(*(bmi->bcn_template)));

	MFREE(wlc->osh, bmi, sizeof(*bmi));
	*pbmi = NULL;
}

/* bsscfg cubby accessors */
wlc_pkt_t
wlc_mbss_get_probe_template(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bss_mbss_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	return bmi->probe_template;
}

wlc_spt_t *
wlc_mbss_get_bcn_template(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bss_mbss_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	return bmi->bcn_template;
}

uint32
wlc_mbss_get_bcmc_pkts_sent(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bss_mbss_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	return bmi->mc_fifo_pkts;
}

static int
wlc_mbss_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_mbss_info_t *mbss = (wlc_mbss_info_t *)hdl;
	wlc_info_t *wlc = mbss->wlc;

	int32 int_val = 0;
	bool bool_val;
	uint32 *ret_uint_ptr;
	int err = BCME_OK;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	ret_uint_ptr = (uint32 *)arg;

	switch (actionid) {
	case IOV_GVAL(IOV_BCN_ROTATE):
		if (mbss->beacon_bssidx < 0)
			*ret_uint_ptr = 0;
		else
			*ret_uint_ptr = 1;
		break;

	case IOV_SVAL(IOV_BCN_ROTATE):
		if (bool_val)
			mbss->beacon_bssidx = 0;
		else
			mbss->beacon_bssidx = -1;
		break;

	case IOV_GVAL(IOV_MBSS):
		*ret_uint_ptr = (wlc->pub->_mbss_mode != 0) ? TRUE : FALSE;
		break;

	case IOV_SVAL(IOV_MBSS): {
		bool curstat = (wlc->pub->_mbss_mode != 0);
		bool rev16 = D11REV_ISMBSS16(wlc->pub->corerev) ? TRUE : FALSE;
		bool rev4 = D11REV_ISMBSS4(wlc->pub->corerev) ? TRUE : FALSE;

		/* No change requested */
		if (curstat == bool_val)
			break;

		if (!MBSS_SUPPORT(wlc->pub) || (!rev4 && !rev16)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		/* Reject if insufficient template memory */
		if (rev16 && (wlc_bmac_ucodembss_hwcap(wlc->hw) == FALSE)) {
			err = BCME_NORESOURCE;
			break;
		}

		if (curstat) {
			/* if turning off mbss, disable extra bss configs */
			wlc_bsscfg_disablemulti(wlc);
			wlc_bmac_set_defmacintmask(wlc->hw, MI_PRQ, ~MI_PRQ);
			wlc_bmac_set_defmacintmask(wlc->hw, MI_DTIM_TBTT, ~MI_DTIM_TBTT);
			wlc->pub->_mbss_mode = 0;
		}
		else {
			if (!rev16)
				wlc_bmac_set_defmacintmask(wlc->hw, MI_PRQ, MI_PRQ);
			wlc_bmac_set_defmacintmask(wlc->hw, MI_DTIM_TBTT, MI_DTIM_TBTT);
			wlc->pub->_mbss_mode = rev4 ? MBSS4_ENABLED : MBSS16_ENABLED;
		}
#ifdef WLLPRS
		/* Enable/disable legacy prs support in ucode based on mbss state */
		wlc_mhf(wlc, MHF5, MHF5_LEGACY_PRS, (wlc->pub->_mbss_mode ? MHF5_LEGACY_PRS : 0),
			WLC_BAND_ALL);
#endif /* WLLPRS */
		}
		break;

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

/* Write the base MAC/BSSID into shared memory.  For MBSS, the MAC and BSSID
 * are required to be the same.
 */
int
wlc_write_mbss_basemac(wlc_info_t *wlc, const struct ether_addr *addr)
{
	uint16 mac_l;
	uint16 mac_m;
	uint16 mac_h;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	mac_l = addr->octet[0] | (addr->octet[1] << 8);
	mac_m = addr->octet[2] | (addr->octet[3] << 8);
	/* Mask low bits of BSSID base */
	mac_h = addr->octet[4] |
		((addr->octet[5] & ~(WLC_MBSS_UCIDX_MASK(wlc->pub->corerev))) << 8);

	wlc_write_shm(wlc, SHM_MBSS_BSSID0, mac_l);
	wlc_write_shm(wlc, SHM_MBSS_BSSID1, mac_m);
	wlc_write_shm(wlc, SHM_MBSS_BSSID2, mac_h);

	return BCME_OK;
}

static int
wlc_mbss_up(void *hdl)
{
	wlc_mbss_info_t *mbss = (wlc_mbss_info_t *)hdl;
	uint8 i;

	ASSERT(MBSS_SUPPORT(mbss->wlc->pub));

	/* Initialize the HW to SW BSS configuration index map */
	for (i = 0; i < WLC_MAXBSSCFG; i++) {
		mbss->hw2sw_idx[i] = WLC_MBSS_BSSCFG_IDX_INVALID;
	}
#if defined(WLC_HIGH) && defined(WLC_LOW)
	if (D11REV_ISMBSS4(mbss->wlc->pub->corerev)) {
		wlc_info_t *wlc = mbss->wlc;

		/* Initialize the cached PRQ base pointer */
		mbss->prq_base = wlc_read_shm(wlc, SHM_MBSS_PRQ_BASE);
		mbss->prq_rd_ptr = mbss->prq_base;
	}
#endif /* WLC_HIGH && WLC_LOW */

	return BCME_OK;
}

/*
 * Allocate and set up a software packet template
 * @param count The number of packets to use; must be <= WLC_SPT_COUNT_MAX
 * @param len The length of the packets to be allocated
 *
 * Returns 0 on success, < 0 on error.
 */

static int
wlc_spt_init(wlc_info_t *wlc, wlc_spt_t *spt, int count, int len)
{
	int idx;
	int tso_hdr_overhead = (wlc->toe_bypass ? 0 : sizeof(d11ac_tso_t));

	/* Pad for header overhead */
	len += tso_hdr_overhead;

	ASSERT(spt != NULL);
	ASSERT(count <= WLC_SPT_COUNT_MAX);

	for (idx = 0; idx < count; idx++) {
		if (spt->pkts[idx] == NULL &&
		    (spt->pkts[idx] = PKTGET(wlc->osh, len, TRUE)) == NULL) {
			return -1;
		}
	}

	spt->latest_idx = -1;

	return 0;
}

/*
 * Clean up a software template object;
 * if pkt_free_force is TRUE, will not check if the pkt is in use before freeing.
 * Note that if "in use", the assumption is that some other routine owns
 * the packet and will free appropriately.
 */

static void
wlc_spt_deinit(wlc_info_t *wlc, wlc_spt_t *spt, int pkt_free_force)
{
	int idx;

	if (spt != NULL) {
		for (idx = 0; idx < WLC_SPT_COUNT_MAX; idx++) {
			if (spt->pkts[idx] != NULL) {
				if (pkt_free_force || !SPT_IN_USE(spt, idx)) {
					PKTFREE(wlc->osh, spt->pkts[idx], TRUE);
					spt->pkts[idx] = NULL;
				} else {
					WLPKTFLAG_BSS_DOWN_SET(WLPKTTAG(spt->pkts[idx]), TRUE);
				}
			}
		}
		bzero(spt, sizeof(*spt));
	}
}

static void
mbss_ucode_set(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bool cur_val, new_val;

	/* Assumes MBSS_EN has same value in all cores */
	cur_val = ((wlc_mhf_get(wlc, MHF1, WLC_BAND_AUTO) & MHF1_MBSS_EN) != 0);
	new_val = (MBSS_ENAB(wlc->pub) != 0);

	if (cur_val != new_val) {
		wlc_suspend_mac_and_wait(wlc);
		/* enable MBSS in uCode */
		WL_MBSS(("%s MBSS mode\n", new_val ? "Enabling" : "Disabling"));
		(void)wlc_mhf(wlc, MHF1, MHF1_MBSS_EN, new_val ? MHF1_MBSS_EN : 0, WLC_BAND_ALL);
		wlc_enable_mac(wlc);
	}
}

/* Generate a MAC address for the MBSS AP BSS config */
static int
wlc_bsscfg_macgen(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int ii, jj;
	bool collision = TRUE;
	int cfg_idx = WLC_BSSCFG_IDX(cfg);
	struct ether_addr newmac;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	ASSERT(MBSS_SUPPORT(wlc->pub));

	if (ETHER_ISNULLADDR(&wlc->mbss->vether_base)) {
		/* initialize virtual MAC base for MBSS
		 * the base should come from an external source,
		 * this initialization is in case one isn't provided
		 */
		bcopy(&wlc->pub->cur_etheraddr, &wlc->mbss->vether_base, ETHER_ADDR_LEN);
		/* avoid collision */
		wlc->mbss->vether_base.octet[5] += 1;

		/* force locally administered address */
		ETHER_SET_LOCALADDR(&wlc->mbss->vether_base);
	}

	bcopy(&wlc->mbss->vether_base, &newmac, ETHER_ADDR_LEN);

	/* brute force attempt to make a MAC for this interface,
	 * the user didn't provide one.
	 * outside loop limits the # of times we increment the low byte of
	 * the MAC address we're attempting to create, and the inner loop
	 * checks for collisions with other configs.
	 */
	for (ii = 0; (ii < WLC_MAXBSSCFG) && (collision == TRUE); ii++) {
		collision = FALSE;
		for (jj = 0; jj < WLC_MAXBSSCFG; jj++) {
			/* don't compare with the bss config we're updating */
			if (jj == cfg_idx || (!wlc->bsscfg[jj]))
				continue;
			if (EADDR_TO_UC_IDX(wlc->bsscfg[jj]->cur_etheraddr,
			    WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)) ==
			    EADDR_TO_UC_IDX(newmac, WLC_MBSS_UCIDX_MASK(wlc->pub->corerev))) {
				collision = TRUE;
				break;
			}
		}
		if (collision == TRUE) /* increment and try again */
			newmac.octet[5] =
				(newmac.octet[5] & ~(WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)))
			        | (WLC_MBSS_UCIDX_MASK(wlc->pub->corerev) & (newmac.octet[5] + 1));
		else
			bcopy(&newmac, &cfg->cur_etheraddr, ETHER_ADDR_LEN);
	}

	if (ETHER_ISNULLADDR(&cfg->cur_etheraddr)) {
		WL_MBSS(("wl%d.%d: wlc_bsscfg_macgen couldn't generate MAC address\n",
		         wlc->pub->unit, cfg_idx));

		return BCME_BADADDR;
	}
	else {
		WL_MBSS(("wl%d.%d: wlc_bsscfg_macgen assigned MAC %s\n",
		         wlc->pub->unit, cfg_idx,
		         bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf)));
		return BCME_OK;
	}
}

static int
mbss_bsscfg_up(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int result = 0;
	int idx;
	wlc_bsscfg_t *bsscfg;
	int  bcn_tmpl_len = wlc->pub->bcn_tmpl_len;
	bss_mbss_info_t *bmi;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	/* Assumes MBSS is enabled for this BSS config herein */

	/* Set pre TBTT interrupt timer to 10 ms for now; will be shorter */
	wlc_write_shm(wlc, SHM_MBSS_PRE_TBTT, wlc->ap->pre_tbtt_us);

	/* if the BSS configs hasn't been given a user defined address or
	 * the address is duplicated, we'll generate our own.
	 */
	FOREACH_BSS(wlc, idx, bsscfg) {
		if (bsscfg == cfg)
			continue;
		if (bcmp(&bsscfg->cur_etheraddr, &cfg->cur_etheraddr, ETHER_ADDR_LEN) == 0)
			break;
	}
	if (ETHER_ISNULLADDR(&cfg->cur_etheraddr) || idx < WLC_MAXBSSCFG) {
		result = wlc_bsscfg_macgen(wlc, cfg);
		if (result != BCME_OK) {
			WL_ERROR(("wl%d.%d: %s: unable to generate MAC address\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
			return result;
		}
	}

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	/* Set the uCode index of this config */
	bmi->_ucidx = EADDR_TO_UC_IDX(cfg->cur_etheraddr, WLC_MBSS_UCIDX_MASK(wlc->pub->corerev));
	ASSERT(bmi->_ucidx <= WLC_MBSS_UCIDX_MASK(wlc->pub->corerev));
	wlc->mbss->hw2sw_idx[bmi->_ucidx] = WLC_BSSCFG_IDX(cfg);

	/* Allocate DMA space for beacon software template */
	result = wlc_spt_init(wlc, bmi->bcn_template, BCN_TEMPLATE_COUNT, bcn_tmpl_len);
	if (result != BCME_OK) {
		WL_ERROR(("wl%d.%d: %s: unable to allocate beacon templates",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return result;
	}
	/* Set the BSSCFG index in the packet tag for beacons */
	for (idx = 0; idx < BCN_TEMPLATE_COUNT; idx++) {
		WLPKTTAGBSSCFGSET(bmi->bcn_template->pkts[idx], WLC_BSSCFG_IDX(cfg));
	}

	/* Make sure that our SSID is in the correct uCode
	 * SSID slot in shared memory
	 */
	wlc_shm_ssid_upd(wlc, cfg);

	wlc_mbss_bcmc_reset(wlc, cfg);

	cfg->flags &= ~(WLC_BSSCFG_SW_BCN | WLC_BSSCFG_SW_PRB);
	cfg->flags &= ~(WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB);
	if (!MBSS_ENAB16(wlc->pub)) {
		cfg->flags |= (WLC_BSSCFG_SW_BCN | WLC_BSSCFG_SW_PRB);
	} else {
		cfg->flags |= (WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB);
	}

	return result;
}

/**
 * This function is used to reinitialize the bcmc firmware/ucode interface for a certain bsscfg.
 * Only to be called when there is no bcmc traffic pending.
 *
 * For MBSS, for each bss, every DTIM the last transmitted bcmc fid is set in SHM.
 * Ucode will only transmit up to and including this fid even if more bcmc packets are added later.
 * When there is no more bcmc traffic pending for a certain bss:
 * 1) bcmc_fid and bcmc_fid_shm are set to INVALIDFID, in case last traffic was flushed shm needs to
 *    be updated as well.
 * 2) mc_fifo_pkts is set to 0.
 * 3) a ps off transition can be marked as complete so that the PS bit for the bcmc scb of that bss
 *    is properly set.
 */
void
wlc_mbss_bcmc_reset(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint fid_addr = 0;
	bss_mbss_info_t *bmi;

	if (TXPKTPENDGET(wlc, TX_BCMC_FIFO) != 0) {
		WL_ERROR(("wl%d.%d: There are still bcmc packets in transit!! %p\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __builtin_return_address(0)));
	}

	cfg->bcmc_fid = INVALIDFID;
	cfg->bcmc_fid_shm = INVALIDFID;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	if (bmi == NULL) {
		return;
	}

	WL_MBSS(("wl%d.%d: %s: resetting fids %d, %d; mc pkts %d\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		__FUNCTION__, cfg->bcmc_fid, cfg->bcmc_fid_shm, bmi->mc_fifo_pkts));

	bmi->mc_fifo_pkts = 0;

	if (wlc->pub->hw_up) {
		/* Let's also update the shm */
		if (MBSS_ENAB16(wlc->pub)) {
			fid_addr = SHM_MBSS_BC_FID_ADDR16(bmi->_ucidx);
		}
#ifdef WLC_LOW
		else {
			fid_addr = SHM_MBSS_BC_FID_ADDR(bmi->_ucidx);
		}
#endif /* WLC_LOW */
		if (fid_addr) {
			wlc_write_shm((wlc), fid_addr, INVALIDFID);
		}
	}
	if (cfg->flags & WLC_BSSCFG_PS_OFF_TRANS) {
		wlc_apps_bss_ps_off_done(wlc, cfg);
	}
}

int
wlc_mbss_bsscfg_up(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	int ret = BCME_OK;

	if (MBSS_ENAB(wlc->pub)) {
		if ((ret = mbss_bsscfg_up(wlc, cfg)) != BCME_OK)
			return ret;

		/* XXX JFH - This is redundant right now, we're also
		 * writing the MAC in wlc_BSSinit() but we want this
		 * to be done prior to enabling MBSS per George
		 * We should probably be using wlc_set_mac() here..?
		 */
		wlc_write_mbss_basemac(wlc, &wlc->mbss->vether_base);
	}
	mbss_ucode_set(wlc, cfg);

	return ret;
}

void
wlc_mbss_bsscfg_down(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	if (MBSS_SUPPORT(wlc->pub)) {
		uint clear_len = FALSE;
		wlc_bsscfg_t *bsscfg;
		bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
		uint8 ssidlen = cfg->SSID_len;
		uint i;

		ASSERT(bmi != NULL);

		wlc_spt_deinit(wlc, bmi->bcn_template, FALSE);

		if (bmi->probe_template != NULL) {
			PKTFREE(wlc->osh, bmi->probe_template, TRUE);
			bmi->probe_template = NULL;
		}

#ifdef WLLPRS
		if (bmi->lprs_template != NULL) {
			PKTFREE(wlc->osh, bmi->lprs_template, TRUE);
			bmi->lprs_template = NULL;
		}
#endif /* WLLPRS */

		/* If we clear ssid length of all bsscfgs while doing
		* a wl down the ucode can get into a state where it
		* will keep searching  for non-zero ssid length thereby
		* causing mac_suspend_and_wait messages. To avoid that
		* we will prevent clearing the ssid len of at least one BSS
		*/
		FOREACH_BSS(wlc, i, bsscfg) {
			if (bsscfg->up) {
				clear_len = TRUE;
				break;
			}
		}
		if (clear_len) {
			cfg->SSID_len = 0;

			/* update uCode shared memory */
			wlc_shm_ssid_upd(wlc, cfg);
			cfg->SSID_len = ssidlen;

			/* Clear the HW index */
			wlc->mbss->hw2sw_idx[bmi->_ucidx] = WLC_MBSS_BSSCFG_IDX_INVALID;
		}
	}
}

#ifdef BCMDBG
void
wlc_mbss_dump_spt_pkt_state(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int i)
{
	bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, cfg);

	ASSERT(bmi != NULL);

	/* Dump SPT pkt state */
	if (bmi->bcn_template != NULL) {
		int j;
		wlc_spt_t *spt = bmi->bcn_template;

		for (j = 0; j < WLC_SPT_COUNT_MAX; j++) {
			if (spt->pkts[j] != NULL) {
				wlc_pkttag_t *tag = WLPKTTAG(spt->pkts[j]);

				WL_ERROR(("bss[%d] spt[%d]=%p in_use=%d flags=%08x flags2=%04x\n",
				          i, j, spt->pkts[j],
				          SPT_IN_USE(bmi->bcn_template, j),
				          tag->flags, tag->flags2));
			}
		}
	}
}
#endif /* BCMDBG */

/* Under MBSS, this routine handles all TX dma done packets from the ATIM fifo. */
void
wlc_mbss_dotxstatus(wlc_info_t *wlc, tx_status_t *txs, void *pkt, uint16 fc,
                    wlc_pkttag_t *pkttag, uint supr_status)
{
	wlc_bsscfg_t *bsscfg = NULL;
	int bss_idx;
	bool free_pkt = FALSE;
	bss_mbss_info_t *bmi;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	bss_idx = (int)(WLPKTTAG_BSSIDX_GET(pkttag));
#if defined(BCMDBG) /* Verify it's a reasonable index */
	if ((bss_idx < 0) || (bss_idx >= WLC_MAXBSSCFG) ||
	    (wlc->bsscfg[bss_idx] == NULL)) {
		WL_ERROR(("%s: bad BSS idx\n", __FUNCTION__));
		ASSERT(!"MBSS dotxstatus: bad BSS idx");
		return;
	}
#endif /* BCMDBG */

	/* For probe resp, this is really only used for counters */
	ASSERT(bss_idx < WLC_MAXBSSCFG);
	bsscfg = wlc->bsscfg[bss_idx];
	ASSERT(bsscfg != NULL);

	bmi = BSS_MBSS_INFO(wlc->mbss, bsscfg);
	ASSERT(bmi != NULL);

	/* Being in the ATIM fifo, it must be a beacon or probe response */
	switch (fc & FC_KIND_MASK) {
	case FC_PROBE_RESP:
		/* Requeue suppressed probe response if due to TBTT */
		if (supr_status == TX_STATUS_SUPR_TBTT) {
			int txerr;

			WLCNTINCR(bmi->cnt->prb_resp_retrx);
#ifdef WLC_LOW
			txerr = wlc_bmac_dma_txfast(wlc, TX_ATIM_FIFO, pkt, TRUE);
#else
			txerr = wlc_rpctx_tx(wlc->rpctx, TX_ATIM_FIFO, pkt, TRUE, -1, 1);
#endif /* WLC_LOW */

			if (txerr < 0) {
				WL_MBSS(("Failed to retransmit suppressed probe resp for bss %d\n",
					WLC_BSSCFG_IDX(bsscfg)));
				WLCNTINCR(bmi->cnt->prb_resp_retrx_fail);
				free_pkt = TRUE;
			}
		} else {
			free_pkt = TRUE;
			if (supr_status == TX_STATUS_SUPR_EXPTIME) {
				WLCNTINCR(bmi->cnt->prb_resp_ttl_expy);
			}
		}
		break;
	case FC_BEACON:
		if (supr_status != TX_STATUS_SUPR_NONE)
			WL_ERROR(("%s: Suppressed Beacon frame = 0x%x\n", __FUNCTION__,
			          supr_status));

		if (WLPKTFLAG_BSS_DOWN_GET(pkttag)) { /* Free the pkt since BSS is gone */
			WL_MBSS(("BSSCFG down on bcn done\n"));
			WL_ERROR(("%s: in_use_bitmap = 0x%x pkt: %p\n", __FUNCTION__,
			          bmi->bcn_template->in_use_bitmap, pkt));
			free_pkt = TRUE;
			break; /* All done */
		}
		ASSERT(bsscfg->up);
		/* Assume only one beacon in use at a time */
		bmi->bcn_template->in_use_bitmap = 0;
#if defined(WLC_SPT_DEBUG) && defined(BCMDBG)
		if (supr_status != TX_STATUS_SUPR_NONE) {
			bmi->bcn_template->suppressed++;
		}
#endif /* WLC_STP_DEBUG && BCMDBG */
		break;
	default: /* Bad packet type for ATIM fifo */
		ASSERT(!"TX done ATIM packet neither BCN or PRB");
		break;
	}

	if (supr_status != TX_STATUS_SUPR_NONE) {
		WLCNTINCR(wlc->pub->_cnt->atim_suppress_count);
	}

	if (free_pkt) {
		PKTFREE(wlc->osh, pkt, TRUE);
	}
}

void
wlc_mbss_dotxstatus_mcmx(wlc_info_t *wlc, wlc_bsscfg_t *cfg, tx_status_t *txs)
{
	bss_mbss_info_t *bmi;

	ASSERT(MBSS_ENAB(wlc->pub));

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	bmi->mc_fifo_pkts--; /* Decrement mc fifo counter */

	/* Check if this was last frame uCode knew about */
	if (WLC_TXFID_GET_SEQ(cfg->bcmc_fid_shm) == WLC_TXFID_GET_SEQ(txs->frameid)) {
		cfg->bcmc_fid_shm = INVALIDFID;
		if ((cfg->flags & WLC_BSSCFG_PS_OFF_TRANS) && (cfg->bcmc_fid == INVALIDFID)) {
			/* Mark transition complete as pkts out of BCMC fifo */
			wlc_apps_bss_ps_off_done(wlc, cfg);
		}
	}
}

void
wlc_mbss_update_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint16 *bcn;
	int idx;
	wlc_pkt_t pkt;
	ratespec_t bcn_rspec;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	int len = wlc->pub->bcn_tmpl_len;
	bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, cfg);

	ASSERT(MBSS_BCN_ENAB(wlc, cfg));
	ASSERT(bmi != NULL);

	/* Find a non-inuse buffer */
	if ((idx = SPT_PAIR_AVAIL(bmi->bcn_template)) == -1) {
		ASSERT(!"Beacon template not available");
		return;
	}

	pkt = bmi->bcn_template->pkts[idx];
	ASSERT(pkt != NULL);

	/* account for tso_hdr before txh */
#ifdef WLTOEHW
	{
		uint16 tso_hdr_size;
		d11ac_tso_t *tso_hdr;

		/* Pull off space so that d11hdrs below works */
		tso_hdr = (d11ac_tso_t *) PKTDATA(wlc->osh, pkt);
		tso_hdr_size = (uint16) (wlc->toe_bypass ? 0 : wlc_tso_hdr_length(tso_hdr));
		PKTPULL(wlc->osh, pkt, tso_hdr_size);
		len -= tso_hdr_size;
	}
#endif /* WLTOEHW */

	PKTPULL(wlc->osh, pkt, D11_TXH_LEN_EX(wlc));
	len -= D11_TXH_LEN_EX(wlc);
	/* Use the lowest basic rate for beacons if no user specified bcn rate */
	bcn_rspec =
#ifdef AP
		wlc_force_bcn_rspec(wlc) ? wlc_force_bcn_rspec(wlc) :
#endif /* AP */
		wlc_lowest_basic_rspec(wlc, &current_bss->rateset);
	ASSERT(wlc_valid_rate(wlc, bcn_rspec,
	                      CHSPEC_IS2G(current_bss->chanspec) ? WLC_BAND_2G : WLC_BAND_5G,
	                      TRUE));
	wlc->bcn_rspec = bcn_rspec;

	bcn = (uint16 *)PKTDATA(wlc->osh, pkt);
	wlc_bcn_prb_template(wlc, FC_BEACON, bcn_rspec, cfg, bcn, &len);
	PKTSETLEN(wlc->osh, pkt, len);
	wlc_write_hw_bcnparams(wlc, cfg, bcn, len, bcn_rspec, FALSE);
	wlc_d11hdrs(wlc, pkt, wlc->band->hwrs_scb, FALSE, 0, 0,
	            TX_ATIM_FIFO, 0, NULL, bcn_rspec, NULL);

	/* Indicate most recently updated index */
	bmi->bcn_template->latest_idx = idx;
	bmi->bcn_template->bcn_modified = TRUE;
}

/* Select SSID length register based on HW index */
#define _MBSS_SSID_LEN_SELECT(wlc, idx) \
	(MBSS_ENAB(wlc->pub) ? \
	(((idx) == 0 || (idx) == 1) ? SHM_MBSS_SSID_LEN0 : SHM_MBSS_SSID_LEN1) : M_SSIDLEN)

/* Use to access a specific SSID length */
#define WLC_MBSS_SSID_LEN_GET(wlc, idx, out_val) \
	do { \
		out_val = wlc_read_shm(wlc, _MBSS_SSID_LEN_SELECT(wlc, idx)); \
		if ((idx) % 2) \
			out_val = ((out_val) >> 8) & 0xff; \
		else \
			out_val = (out_val) & 0xff; \
	} while (0)

/* Internal macro to set SSID length register values properly */
#define _MBSS_SSID_LEN_SET(idx, reg_val, set_val) \
	do { \
		if ((idx) % 2) { \
			(reg_val) &= ~0xff00; \
			(reg_val) |= ((set_val) & 0xff) << 8; \
		} else { \
			(reg_val) &= ~0xff; \
			(reg_val) |= (set_val) & 0xff; \
		} \
	} while (0)

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* Get the SSID for the indicated (idx) bsscfg from SHM Return the length */
static void
wlc_shm_ssid_get(wlc_info_t *wlc, int idx, wlc_ssid_t *ssid)
{
	int i;
	int base;
	uint16 tmpval;
	int ucode_idx;
	bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, wlc->bsscfg[idx]);

	ASSERT(bmi != NULL);

	ucode_idx = bmi->_ucidx;

	if (MBSS_ENAB16(wlc->pub)) {
		base = SHM_MBSS_SSIDSE_BASE_ADDR + (ucode_idx * SHM_MBSS_SSIDSE_BLKSZ);
		wlc_bmac_copyfrom_objmem(wlc->hw, base, &ssid->SSID_len,
		                       SHM_MBSS_SSIDLEN_BLKSZ, OBJADDR_SRCHM_SEL);
		/* search mem length field is always little endian */
		ssid->SSID_len = ltoh32(ssid->SSID_len);
		base += SHM_MBSS_SSIDLEN_BLKSZ;
		wlc_bmac_copyfrom_objmem(wlc->hw, base, ssid->SSID,
		                       SHM_MBSS_SSID_BLKSZ, OBJADDR_SRCHM_SEL);
		return;
	}

	WLC_MBSS_SSID_LEN_GET(wlc, ucode_idx, ssid->SSID_len);
	base = SHM_MBSS_SSID_ADDR(ucode_idx);
	for (i = 0; i < DOT11_MAX_SSID_LEN; i += 2) {
		tmpval = wlc_read_shm(wlc, base + i);
		ssid->SSID[i] = tmpval & 0xFF;
		ssid->SSID[i + 1] = tmpval >> 8;
	}
}

static void
wlc_mbss_info_dump(void *hdl, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_mbss_info_t *mbss = (wlc_mbss_info_t *)hdl;
	wlc_info_t *wlc = mbss->wlc;
	bss_mbss_info_t *bmi;
	int bsscfg_idx;

	ASSERT(cfg != NULL);

	bmi = BSS_MBSS_INFO(mbss, cfg);
	bsscfg_idx = WLC_BSSCFG_IDX(cfg);

	if (!MBSS_SUPPORT(wlc->pub) || bmi == NULL)
		return;

	bcm_bprintf(b, "PS trans %u.\n", WLCNTVAL(bmi->cnt->ps_trans));
#if defined(WLC_SPT_DEBUG)
	bcm_bprintf(b, "BCN: bcn tx cnt %u. bcn suppressed %u\n",
		bmi->bcn_template->tx_count, bmi->bcn_template->suppressed);
#endif /* WLC_SPT_DEBUG */
	bcm_bprintf(b,
		"PrbResp: soft-prb-resp %s. directed req %d, alloc_fail %d, tx_fail %d\n",
		SOFTPRB_ENAB(cfg) ? "enabled" : "disabled",
		WLCNTVAL(bmi->cnt->prq_directed_entries),
		WLCNTVAL(bmi->cnt->prb_resp_alloc_fail),
		WLCNTVAL(bmi->cnt->prb_resp_tx_fail));
	bcm_bprintf(b, "PrbResp: TBTT suppressions %d. TTL expires %d. retrx fail %d.\n",
		WLCNTVAL(bmi->cnt->prb_resp_retrx), WLCNTVAL(bmi->cnt->prb_resp_ttl_expy),
		WLCNTVAL(bmi->cnt->prb_resp_retrx_fail));
	bcm_bprintf(b, "BCN: soft-bcn %s. bcn in use bmap 0x%x. bcn fail %u\n",
		SOFTBCN_ENAB(cfg) ? "enabled" : "disabled",
		bmi->bcn_template->in_use_bitmap, WLCNTVAL(bmi->cnt->bcn_tx_failed));
	bcm_bprintf(b, "BCN: HW MBSS %s. bcn in use bmap 0x%x. bcn fail %u\n",
		HWBCN_ENAB(cfg) ? "enabled" : "disabled",
		bmi->bcn_template->in_use_bitmap, WLCNTVAL(bmi->cnt->bcn_tx_failed));
	bcm_bprintf(b, "PRB: HW MBSS %s.\n",
		HWPRB_ENAB(cfg) ? "enabled" : "disabled");
	bcm_bprintf(b, "MC pkts in fifo %u. Max %u\n", bmi->mc_fifo_pkts,
		WLCNTVAL(bmi->cnt->mc_fifo_max));
	if (wlc->clk) {
		char ssidbuf[SSID_FMT_BUF_LEN];
		wlc_ssid_t ssid;
		uint16 shm_fid;

		shm_fid = wlc_read_shm(wlc, SHM_MBSS_WORD_OFFSET_TO_ADDR(5 + bmi->_ucidx));
		bcm_bprintf(b, "bcmc_fid 0x%x. bcmc_fid_shm 0x%x. shm last fid 0x%x. "
			"bcmc TX pkts %u\n", cfg->bcmc_fid, cfg->bcmc_fid_shm, shm_fid,
			WLCNTVAL(bmi->cnt->bcmc_count));
		wlc_shm_ssid_get(wlc, bsscfg_idx, &ssid);
		if (ssid.SSID_len > DOT11_MAX_SSID_LEN) {
			WL_ERROR(("Warning: Invalid MBSS ssid length %d for BSS %d\n",
				ssid.SSID_len, bsscfg_idx));
			ssid.SSID_len = DOT11_MAX_SSID_LEN;
		}
		wlc_format_ssid(ssidbuf, ssid.SSID, ssid.SSID_len);
		bcm_bprintf(b, "MBSS: ucode idx %d; shm ssid >%s< of len %d\n",
			bmi->_ucidx, ssidbuf, ssid.SSID_len);
	} else {
		bcm_bprintf(b, "Core clock disabled, not dumping SHM info\n");
	}
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* Use to set a specific SSID length */
static void
wlc_mbss_ssid_len_set(wlc_info_t *wlc, int idx, uint8 in_val)
{
	uint16 tmp_val;
	tmp_val = wlc_read_shm(wlc, _MBSS_SSID_LEN_SELECT(wlc, idx));
	_MBSS_SSID_LEN_SET(idx, tmp_val, in_val);
	wlc_write_shm(wlc, _MBSS_SSID_LEN_SELECT(wlc, idx), tmp_val);
}

void
wlc_mbss_shm_ssid_upd(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 *base)
{
	ASSERT(MBSS_ENAB(wlc->pub));

	/* Update based on uCode index of BSS */
	if (MBSS_ENAB16(wlc->pub)) {
		wlc_mbss16_updssid(wlc, cfg);
		/* tell ucode where to find the probe responses */
		if (D11REV_GE(wlc->pub->corerev, 40))
			wlc_write_shm(wlc, SHM_MBSS_PRS_TPLPTR,
			              (MBSS_PRS_BLKS_START(WLC_MAX_AP_BSS(wlc->pub->corerev))));
		else if (D11REV_GE(wlc->pub->corerev, 16))
			/* for corerev >= 16 the value is in multiple of 4 */
			wlc_write_shm(wlc, SHM_MBSS_PRS_TPLPTR,
				(MBSS_PRS_BLKS_START(WLC_MAX_AP_BSS(wlc->pub->corerev)) >> 2));
		else
			wlc_write_shm(wlc, SHM_MBSS_PRS_TPLPTR,
				MBSS_PRS_BLKS_START(WLC_MAX_AP_BSS(wlc->pub->corerev)));

		wlc_write_shm(wlc, SHM_MBSS_BC_FID1, WLC_MBSS_UCIDX_MASK(wlc->pub->corerev));
	} else {
		int uc_idx;
		bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, cfg);

		ASSERT(bmi != NULL);

		uc_idx = bmi->_ucidx;

		*base = SHM_MBSS_SSID_ADDR(uc_idx);	/* Update base addr for ssid */
		wlc_mbss_ssid_len_set(wlc, uc_idx, cfg->SSID_len);
	}
}

static void
wlc_mbss_bcmc_free_cb(wlc_info_t *wlc, void *pkt, uint txs)
{
	int err;
	wlc_bsscfg_t *cfg;
	uint16 frameid;

	ASSERT(MBSS_ENAB(wlc->pub));

	cfg = wlc_bsscfg_find(wlc, WLPKTTAGBSSCFGGET(pkt), &err);

	/* if bsscfg or scb are stale or bad, then ignore this pkt for acctg purposes */
	if (!err && cfg) {
		/* Check if this was last frame uCode knew about */
#if !defined(DONGLEBUILD)
		frameid = wlc_get_txh_frameid(wlc, pkt);
#else
		frameid = BCMDHDHDR_ENAB() ? wlc_get_txh_frameid(wlc, pkt) : PKTGETFRAMEID(pkt);
#endif // endif
		if ((cfg->bcmc_fid_shm != INVALIDFID) && frameid != 0 &&
			(WLC_TXFID_GET_SEQ(cfg->bcmc_fid_shm) == WLC_TXFID_GET_SEQ(frameid))) {
			cfg->bcmc_fid_shm = INVALIDFID;
			if ((cfg->flags & WLC_BSSCFG_PS_OFF_TRANS) &&
				(cfg->bcmc_fid == INVALIDFID)) {
				/* Mark transition complete as pkts out of BCMC fifo */
				wlc_apps_bss_ps_off_done(wlc, cfg);
			}
			WL_MBSS(("wl%d.%d: BCMC packet freed that was not accounted for"
				", fid: %d\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg), frameid));
		}
	}
}

void
wlc_mbss_txq_update_bcmc_counters(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *p)
{
	bss_mbss_info_t *bmi;

	ASSERT(MBSS_ENAB(wlc->pub));

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	bmi->mc_fifo_pkts++;
	WLF2_PCB1_REG(p, WLF2_PCB1_MBSS_BCMC);

#ifdef WLCNT
	if (bmi->mc_fifo_pkts > bmi->cnt->mc_fifo_max)
		bmi->cnt->mc_fifo_max = bmi->mc_fifo_pkts;
	bmi->cnt->bcmc_count++;
#endif /* WLCNT */
}

void
wlc_mbss_increment_ps_trans_cnt(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	bss_mbss_info_t *bmi;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	WLCNTINCR(bmi->cnt->ps_trans);
}

#ifdef AP

static void
wlc_mbss16_setup(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint8 *bcn;
	void *pkt;
	uint16 tim_offset;
	bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, cfg);

	ASSERT(bmi != NULL);

	/* find the TIM elt offset in the bcn template, push to shm */
	pkt = SPT_LATEST_PKT(bmi->bcn_template);
	bcn = (uint8 *)(PKTDATA(wlc->osh, pkt));
	tim_offset = (uint16)(bmi->bcn_template->tim_ie - bcn);
	/* we want it less the actual ssid length */
	tim_offset -= cfg->SSID_len;
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		/* not sure what do to for 4360... */
		tim_offset -= D11_TXH_LEN_EX(wlc);
		tim_offset += 8;
	}
	else {
		/* and less the D11_TXH_LEN too */
		tim_offset -= D11_TXH_LEN;
	}

	wlc_write_shm(wlc, M_TIMBPOS_INBEACON, tim_offset);
}

/* Write the BSSCFG's beacon template into HW */
static void
wlc_ap_mbss16_write_beacon(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_pkt_t pkt;
	uint32 ucidx;
	int start;
	uint16 len;
	uint8 * pt;
	d11actxh_t *txh = NULL;
	d11actxh_rate_t* rate_info;
	int ac_hdr = 0;
	uint8 phy_hdr[D11AC_PHY_HDR_LEN];
	int  bcn_tmpl_len;
	bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, cfg);

	ASSERT(bmi != NULL);

	bcn_tmpl_len = wlc->pub->bcn_tmpl_len;

	bzero(phy_hdr, sizeof(phy_hdr));

	ucidx = bmi->_ucidx;
	ASSERT(ucidx != (uint32)WLC_MBSS_BSSCFG_IDX_INVALID);

	ASSERT(bmi->bcn_template->latest_idx >= 0);
	ASSERT(bmi->bcn_template->latest_idx < WLC_SPT_COUNT_MAX);

	pkt = SPT_LATEST_PKT(bmi->bcn_template);
	ASSERT(pkt != NULL);

	/* beacon */
	if (bmi->bcn_template->bcn_modified == TRUE) {

		WL_MBSS(("wl%d: %s: bcn_modified on bsscfg %d\n",
		         wlc->pub->unit, __FUNCTION__, WLC_BSSCFG_IDX(cfg)));

		start = wlc->shm_bcn0_tpl_base + (ucidx * bcn_tmpl_len);
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			uint8 offset = wlc_template_plcp_offset(wlc, wlc->bcn_rspec);

			/* Get pointer TX header and build the phy header */
			pt = (uint8 *) PKTDATA(wlc->osh, pkt);
			len = (uint16) PKTLEN(wlc->osh, pkt);
#ifdef WLTOEHW
			{
				uint16 tso_hdr_size;
				d11ac_tso_t *tso_hdr;

				tso_hdr = (d11ac_tso_t *) pt;

				/* Skip overhead */
				tso_hdr_size = (uint16) (wlc->toe_bypass ? 0 :
					wlc_tso_hdr_length(tso_hdr));
				pt += tso_hdr_size;
				len -= tso_hdr_size;
			}
#endif /* WLTOEHW */
			txh = (d11actxh_t *) pt;
			rate_info = WLC_TXD_RATE_INFO_GET(txh, wlc->pub->corerev);

			bcopy(rate_info[0].plcp,
			      &phy_hdr[offset], D11_PHY_HDR_LEN);

			/* Now get the MAC frame */
			pt += D11AC_TXH_LEN;
			len -= D11AC_TXH_LEN;
			ac_hdr = 1;
		}
		else {
			pt = ((uint8 *)(PKTDATA(wlc->osh, pkt)) + D11_TXH_LEN);
			len = PKTLEN(wlc->osh, pkt) - D11_TXH_LEN;
		}

		ASSERT(len <= bcn_tmpl_len);

		if (ac_hdr) {
			/* Write the phy header (PLCP) first */
			wlc_write_template_ram(wlc, start, D11AC_PHY_HDR_LEN, phy_hdr);

			/* Now, write the remaining beacon frame */
			wlc_write_template_ram(wlc, start + D11AC_PHY_HDR_LEN,
			                       (len + 3) & (~3), pt);

			/* bcn len */
			wlc_write_shm(wlc, SHM_MBSS_BCNLEN0 + (2 * ucidx), len+D11AC_PHY_HDR_LEN);
		} else {
			wlc_write_template_ram(wlc, start, (len + 3) & (~3), pt);

			/* bcn len */
			wlc_write_shm(wlc, SHM_MBSS_BCNLEN0 + (2 * ucidx), len);
		}

		/* Push the ssidlen out to shm after bcnlen pushed */
		wlc_mbss16_updssid_len(wlc, cfg);

		wlc_mbss16_setup(wlc, cfg);
		bmi->bcn_template->bcn_modified = FALSE;
	}
}

/* Committing FID to SHM; move driver's value to bcmc_fid_shm */
static void
bcmc_fid_shm_commit(wlc_bsscfg_t *bsscfg)
{
	bsscfg->bcmc_fid_shm = bsscfg->bcmc_fid;
	bsscfg->bcmc_fid = INVALIDFID;
}

#ifdef WLC_LOW
/* DTIM pre-TBTT interrupt has occurred: Update SHM last_fid registers. */
static void
wlc_apps_update_bss_bcmc_fid(wlc_info_t *wlc)
{
	int i = 0;
	uint fid_addr = SHM_MBSS_BC_FID0;
	wlc_bsscfg_t *bsscfg;
	bss_mbss_info_t *bmi;

	if (!MBSS_ENAB(wlc->pub)) {
		return;
	}

	FOREACH_UP_AP(wlc, i, bsscfg) {
		bmi = BSS_MBSS_INFO(wlc->mbss, bsscfg);
		ASSERT(bmi != NULL);

		fid_addr = SHM_MBSS_BC_FID_ADDR(bmi->_ucidx);

		/* If BC/MC packets have been written, update shared mem */
		if (bsscfg->bcmc_fid != INVALIDFID) {
			wlc_write_shm((wlc), fid_addr, bsscfg->bcmc_fid);
			bcmc_fid_shm_commit(bsscfg);
		}
	}
}
#endif /* WLC_LOW */

/* Assumes SW beaconing active */
#define BSS_BEACONING(cfg) ((cfg) && BSSCFG_AP(cfg) && (cfg)->up)

#ifdef WLC_LOW
/* MBSS4 is not supported by bmac driver. Hence, wrapping code related to MBSS4 here
 * with WLC_LOW (only for monolithic driver). wlc_mbss.c is a high side file for a
 * split driver, so if WLC_LOW is defined for the file, it means it is a monolithic build,
 * not split. Only WLC_HIGH is defined in wlc_mbss.c in a split build.
 */

/* MBSS4 MI_TBTT and MI_DTIM_TBTT handler */
int
wlc_ap_mbss4_tbtt(wlc_info_t *wlc, uint32 macintstatus)
{
	int i, idx;
	wlc_bsscfg_t *cfg;
	uint16 beacon_count = 0;
	wlc_pkt_t pkt;
	bool dtim;
	uint32 tbtt_delay;
	bss_mbss_info_t *bmi;
	wlc_mbss_info_t *mbss = wlc->mbss;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	dtim = ((macintstatus & MI_DTIM_TBTT) != 0);
	/* Update our dtim count and determine which BSS config will beacon first */
	if (dtim) {
		mbss->cur_dtim_count = 0x00;
		/* disabled beacon rotate when beacon_bssidx < 0  */
		if (mbss->beacon_bssidx >= 0) {
			/* Bump the "starting" bss index up for bss rotation */
			for (i = 0; i < WLC_MAXBSSCFG; i++) {
				if (++mbss->beacon_bssidx >= WLC_MAXBSSCFG) {
					mbss->beacon_bssidx = 0;
				}
				if (BSS_BEACONING(wlc->bsscfg[mbss->beacon_bssidx])) {
					/* Found the next beaconing BSS index; break */
					break;
				}
			}
		}
	}
	else  {
		if (mbss->cur_dtim_count)
			mbss->cur_dtim_count--;
		else
			mbss->cur_dtim_count = wlc->default_bss->dtim_period - 1;
	}

	/* If we've taken too long to get beacons ready, don't bother queuing them */
	tbtt_delay = (R_REG(wlc->osh, &wlc->regs->tsf_timerlow) - mbss->last_tbtt_us);
	if (tbtt_delay > MBSS_PRE_TBTT_MAX_LATENCY_us) {
		WLCNTINCR(wlc->pub->_cnt->late_tbtt_dpc);
		WL_MBSS(("wl%d: ERROR: TBTT latency: %u; skipping bcn\n",
			wlc->pub->unit, tbtt_delay));
		return 0;
	}

#ifdef RADIO_PWRSAVE
	/* just out of pwr save with a need to skip bcn in level 1/2. */
	if (!dtim && wlc_radio_pwrsave_bcm_cancelled(wlc->ap)) {
		WL_INFORM(("wl%d: radio pwrsave skipping bcn.\n", wlc->pub->unit));
		return 0;
	}
#endif /* RADIO_PWRSAVE */

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		WL_MBSS(("wl%d: WARNING: MBSS Not beaconing due to scan in progress.\n",
		         wlc->pub->unit));
		return 0;
	}

	for (i = 0; i < WLC_MAXBSSCFG; i++) {
		idx = i;
		if (mbss->beacon_bssidx >= 0)
			idx = (i + mbss->beacon_bssidx) % WLC_MAXBSSCFG;
		cfg = wlc->bsscfg[idx];
		if (!BSS_BEACONING(cfg)) {
			continue; /* Skip cfgs not present or not AP or not up */
		}

		bmi = BSS_MBSS_INFO(mbss, cfg);

		ASSERT(bmi != NULL);
		ASSERT(bmi->bcn_template->latest_idx >= 0);
		ASSERT(bmi->bcn_template->latest_idx < WLC_SPT_COUNT_MAX);
		if (bmi->bcn_template->in_use_bitmap != 0) {
			WLCNTINCR(wlc->pub->_cnt->bcn_template_not_ready);
#if defined(BCMDBG_MBSS_PROFILE)
			if (cfg->bcn_tx_done) {
				WLCNTINCR(wlc->pub->_cnt->bcn_template_not_ready_done);
			}
#endif // endif
			continue;
		}

		pkt = SPT_LATEST_PKT(bmi->bcn_template);
		ASSERT(pkt != NULL);

		/* Update DTIM count */
		BCN_TIM_DTIM_COUNT_SET(bmi->bcn_template->tim_ie, mbss->cur_dtim_count);

		/*
		 * Update BCMC flag in the beacon.
		 * At this point, the driver has not yet written the last FID SHM locations;
		 * so either bcmc_fid or bcmc_fid_shm may be indicate pkts in the BC/MC fifo
		 */
		if (BCMC_PKTS_QUEUED(cfg)) {
			BCN_TIM_BCMC_FLAG_SET(bmi->bcn_template->tim_ie);
		} else {
			BCN_TIM_BCMC_FLAG_RESET(bmi->bcn_template->tim_ie);
		}
		if (wlc_bmac_dma_txfast(wlc, TX_ATIM_FIFO, pkt, TRUE) < 0) {
			PKTFREE(wlc->osh, pkt, TRUE);
			WLCNTINCR(bmi->cnt->bcn_tx_failed);
		} else {
			++beacon_count;
			bmi->bcn_template->in_use_bitmap |= (1 << bmi->bcn_template->latest_idx);
#if defined(BCMDBG_MBSS_PROFILE)
			cfg->bcn_tx_done = FALSE;
#endif /* BCMDBG_MBSS_PROFILE */
#if defined(WLC_SPT_DEBUG)
			bmi->bcn_template->tx_count++;
#endif /* WLC_SPT_DEBUG */
		}
	}
	wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, beacon_count);

	if (dtim) {
		wlc_apps_update_bss_bcmc_fid(wlc);
	}

	return 0;
}
#endif /* WLC_LOW */

/* MBSS16 MI_TBTT and MI_DTIM_TBTT handler */
int
wlc_ap_mbss16_tbtt(wlc_info_t *wlc, uint32 macintstatus)
{
	bool dtim;
	int cfgidx;
	int ucidx;
	wlc_bsscfg_t *cfg = NULL;
	uint16 beacon_count = 0;
	uint16 dtim_map = 0;
	uint32 time_left_for_next_tbtt_us; /* time left for next TBTT */
	uint32 tsf_timerlow_us; /* current tfs */
	uint32 beacon_period_us; /* beacon period in us */
	bool ucode_bcn_suspend = FALSE; /* Indicates whether driver have to suspend ucode
					 * for beaconing.
					 */
	bool ucode_bcn_suspended = FALSE; /* Indicates whether ucode is in suspended
					   * state for beaconing.
					   */
	bss_mbss_info_t *bmi;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	dtim = ((macintstatus & MI_DTIM_TBTT) != 0);

#ifdef RADIO_PWRSAVE
	if (!dtim && wlc_radio_pwrsave_bcm_cancelled(wlc->ap)) {
		wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, 0);
		WL_INFORM(("wl%d: radio pwrsave skipping bcn.\n", wlc->pub->unit));
		return 0;
	}
#endif /* RADIO_PWRSAVE */

	/* Calculating the time left for next TBTT */
	beacon_period_us = wlc->default_bss->beacon_period * DOT11_TU_TO_US;
	tsf_timerlow_us = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
	time_left_for_next_tbtt_us = beacon_period_us -
		(tsf_timerlow_us % beacon_period_us);

	if (time_left_for_next_tbtt_us <= MBSS_PRE_TBTT_MIN_THRESH_us)
		ucode_bcn_suspend = TRUE;

	/* Traverse the bsscfg's
	 * create a count of "active" bss's
	 *
	 * if we're at a DTIM:
	 * create a DTIM map,  push "last" bc/mc fid's to shm
	 *
	 * if a beacon has been modified push to shm
	 */
	for (cfgidx = 0; cfgidx < WLC_MAXBSSCFG; cfgidx++) {
		cfg = wlc->bsscfg[cfgidx];
		if (!BSS_BEACONING(cfg))
			continue;

		bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
		ASSERT(bmi != NULL);
		ASSERT(bmi->bcn_template->latest_idx >= 0);
		ASSERT(bmi->bcn_template->latest_idx < WLC_SPT_COUNT_MAX);

		++beacon_count;

		ucidx = bmi->_ucidx;
		ASSERT(ucidx != WLC_MBSS_BSSCFG_IDX_INVALID);
		/* Update BCMC flag in the beacon. */
		if (dtim && (cfg->bcmc_fid != INVALIDFID)) {
			uint fid_addr;

			dtim_map |= NBITVAL(ucidx);
			fid_addr = SHM_MBSS_BC_FID_ADDR16(ucidx);
			if (ucode_bcn_suspend && !ucode_bcn_suspended) {
				ucode_bcn_suspended = TRUE;
				/* Suspending ucode from beaconing for next TBTT
				* due to we are updating bcmc_fid in shared memory
				* very close to next TBTT
				*/
				wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, 0xFFFF);
			}
			wlc_write_shm((wlc), fid_addr, cfg->bcmc_fid);
			bcmc_fid_shm_commit(cfg);
		}

		if ((bmi->bcn_template->bcn_modified == TRUE) &&
			ucode_bcn_suspend && !ucode_bcn_suspended) {
			ucode_bcn_suspended = TRUE;
			/* Suspending ucode from beaconing for next TBTT
			* due to the beacon template is going to be updated
			* in shared memory very close to next TBTT
			*/
			wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, 0xFFFF);
		}

		/* Update the HW beacon template */
		wlc_ap_mbss16_write_beacon(wlc, cfg);

	} /* cfgidx loop */

	wlc_write_shm(wlc, SHM_MBSS_BCN_COUNT, beacon_count);
	wlc_write_shm(wlc, SHM_MBSS_BSSID_NUM, beacon_count);

	if (dtim)
		wlc_write_shm(wlc, SHM_MBSS_BC_BITMAP, dtim_map);

	return 0;
}

/* Write the BSSCFG's probe response template into HW, suspend MAC if requested */
static void
wlc_ap_mbss16_write_prbrsp(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool suspend)
{
	wlc_pkt_t pkt;
	uint32 ucidx;
	int start;
	uint16 len;
	uint8 * pt;
	int  bcn_tmpl_len = wlc->pub->bcn_tmpl_len;
	bss_mbss_info_t *bmi;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	ucidx = bmi->_ucidx;
	ASSERT(ucidx != (uint32)WLC_MBSS_BSSCFG_IDX_INVALID);

	pkt = bmi->probe_template;
	ASSERT(pkt != NULL);

	WL_MBSS(("%s: wl%d.%d %smodified %d\n", __FUNCTION__, wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	         suspend ? "w/suspend " : "", bmi->prb_modified));

	/* probe response */
	if (bmi->prb_modified == TRUE) {
		if (suspend)
			wlc_suspend_mac_and_wait(wlc);

		start = MBSS_PRS_BLKS_START(WLC_MAX_AP_BSS(wlc->pub->corerev)) +
			(ucidx * bcn_tmpl_len);

		if (D11REV_GE(wlc->pub->corerev, 40)) {
#ifdef WLTOEHW
			uint16 tso_hdr_size;
			d11ac_tso_t *tso_hdr;

			tso_hdr = (d11ac_tso_t *)PKTDATA(wlc->osh, pkt);
			tso_hdr_size = (uint16) (wlc->toe_bypass ? 0 :
			                         wlc_tso_hdr_length(tso_hdr));
			PKTPULL(wlc->osh, pkt, tso_hdr_size);
#endif /* WLTOEHW */
			pt = ((uint8 *)(PKTDATA(wlc->osh, pkt)) + (D11AC_TXH_LEN));
			len = PKTLEN(wlc->osh, pkt) - (D11AC_TXH_LEN);
		}
		else {
			pt = ((uint8 *)(PKTDATA(wlc->osh, pkt)) + D11_TXH_LEN);
			len = PKTLEN(wlc->osh, pkt) - D11_TXH_LEN;
		}

		ASSERT(len <= bcn_tmpl_len);

		wlc_write_template_ram(wlc, start, (len + 3) & (~3), pt);
		/* probe response len */
		wlc_write_shm(wlc, SHM_MBSS_PRSLEN0 + (2 * ucidx), len);

#ifdef WLLPRS
		if (N_ENAB(wlc->pub)) {
			wlc_pkt_t prspkt;
			uint16 lgcyprs_len_ptr;

			prspkt = bmi->lprs_template;
			ASSERT(prspkt != NULL);

			/* 11g probe resp, which follows the ht probe resp */
			start = MBSS_PRS_BLKS_START(WLC_MAX_AP_BSS(wlc->pub->corerev)) +
				(WLC_MAX_AP_BSS(wlc->pub->corerev) * bcn_tmpl_len) +
				(ucidx * LPRS_TMPL_LEN);

			if (D11REV_GE(wlc->pub->corerev, 40)) {
				pt = ((uint8 *)(PKTDATA(wlc->osh, prspkt)) + D11AC_TXH_LEN);
				len = PKTLEN(wlc->osh, prspkt) - D11AC_TXH_LEN;
			} else {
				pt = ((uint8 *)(PKTDATA(wlc->osh, prspkt)) + D11_TXH_LEN);
				len = PKTLEN(wlc->osh, prspkt) - D11_TXH_LEN;
			}

			ASSERT(len <= LPRS_TMPL_LEN);

			wlc_write_template_ram(wlc, start, (len + 3) & (~3), pt);

			lgcyprs_len_ptr = wlc_read_shm(wlc, SHM_MBSS_BC_FID3);

			wlc_write_shm(wlc, ((lgcyprs_len_ptr + ucidx) * 2), len);
		}
#endif /* WLLPRS */

		bmi->prb_modified = FALSE;

		if (suspend)
			wlc_enable_mac(wlc);
	}
}

void
wlc_mbss_update_probe_resp(wlc_info_t *wlc, wlc_bsscfg_t *cfg, bool suspend)
{
	uint8 *pbody;
	wlc_pkt_t pkt;
	int len = wlc->pub->bcn_tmpl_len;
	bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, cfg);

	ASSERT(bmi != NULL);

	/* Probe response template includes everything from the PLCP header on */
	if ((pkt = bmi->probe_template) == NULL) {
		pkt = wlc_frame_get_mgmt(wlc, FC_PROBE_RESP, &ether_null,
		                         &cfg->cur_etheraddr, &cfg->BSSID, len, &pbody);
		if (pkt == NULL) {
			WL_ERROR(("Could not allocate SW probe template\n"));
			return;
		}
		bmi->probe_template = pkt;
	} else {
		/* Pull back PLCP and TX headers since wlc_d11hdrs puts them back */
		PKTPULL(wlc->osh, pkt, D11_TXH_LEN_EX(wlc));
		/* PKTDATA is now at start of D11 hdr; find packet body */
		pbody = (uint8 *)PKTDATA(wlc->osh, pkt) + DOT11_MGMT_HDR_LEN;
	}

	/* At this point, PKTDATA is at start of D11 hdr; pbody past D11 hdr */

	/* Generate probe body */
	wlc_bcn_prb_body(wlc, FC_PROBE_RESP, cfg, pbody, &len, FALSE);

	/* Set the length and set up the D11, PLCP and transmit headers */
	PKTSETLEN(wlc->osh, pkt, len + DOT11_MGMT_HDR_LEN);
	wlc_d11hdrs(wlc, pkt, wlc->band->hwrs_scb, FALSE, 0, 0, TX_ATIM_FIFO, 0, NULL, 0, NULL);

#ifdef WLLPRS
	if (N_ENAB(wlc->pub) && D11REV_ISMBSS16(wlc->pub->corerev)) {
		/* Probe response template includes everything from the PLCP header on. */
		if ((pkt = bmi->lprs_template) == NULL) {
			pkt = wlc_frame_get_mgmt(wlc, FC_PROBE_RESP, &ether_null,
			                         &cfg->cur_etheraddr, &cfg->BSSID,
			                         LPRS_TMPL_LEN, &pbody);
			if (pkt == NULL) {
				WL_ERROR(("Could not alloc SW 11g probe template\n"));
				return;
			}
			bmi->lprs_template = pkt;
		} else {
			/* XXX 4360: needs rev40 update for TxD len, and ACPHY update for
			 * phy hdr len
			 */
			/* Pull back PLCP and TX headers since wlc_d11hdrs pushes them. */
			PKTPULL(wlc->osh, pkt, D11_TXH_LEN_EX(wlc));

			/* PKTDATA is now at start of D11 hdr; find packet body */
			pbody = (uint8 *)PKTDATA(wlc->osh, pkt) + DOT11_MGMT_HDR_LEN;
		}

		/* At this point, PKTDATA is at start of D11 hdr; pbody past D11 hdr */

		/* Generate probe body */
		len = LPRS_TMPL_LEN;
		wlc_bcn_prb_body(wlc, FC_PROBE_RESP, cfg, pbody, &len, TRUE);

		/* Set the length and set up the D11, PLCP and transmit headers */
		PKTSETLEN(wlc->osh, pkt, len + DOT11_MGMT_HDR_LEN);
		wlc_d11hdrs(wlc, pkt, wlc->band->hwrs_scb, FALSE, 0, 0,
		            TX_ATIM_FIFO, 0, NULL, 0, NULL);
	}
#endif /* WLLPRS */

	bmi->prb_modified = TRUE;

	if (HWPRB_ENAB(cfg)) {
		/* Update HW template for MBSS16 */
		wlc_ap_mbss16_write_prbrsp(wlc, cfg, suspend);
	}
}

void
wlc_mbss16_updssid(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint32 start;
	uint32 ssidlen = cfg->SSID_len;
	uint32 swplen;
	bss_mbss_info_t *bmi;
	int8 ucidx;
	uint8 ssidbuf[DOT11_MAX_SSID_LEN];

	ASSERT(MBSS_SUPPORT(wlc->pub));

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	ucidx = bmi->_ucidx;
	ASSERT((ucidx >= 0) && (ucidx <= WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)));

	/* push ssid, ssidlen out to ucode Search Engine */
	start = SHM_MBSS_SSIDSE_BASE_ADDR + (ucidx * SHM_MBSS_SSIDSE_BLKSZ);
	/* search mem length field is always little endian */
	swplen = htol32(ssidlen);
	/* invent new function like wlc_write_shm using OBJADDR_SRCHM_SEL */
	wlc_bmac_copyto_objmem(wlc->hw, start, &swplen, SHM_MBSS_SSIDLEN_BLKSZ, OBJADDR_SRCHM_SEL);

	bzero(ssidbuf, DOT11_MAX_SSID_LEN);
	bcopy(cfg->SSID, ssidbuf, cfg->SSID_len);

	start += SHM_MBSS_SSIDLEN_BLKSZ;
	wlc_bmac_copyto_objmem(wlc->hw, start, ssidbuf, SHM_MBSS_SSID_BLKSZ, OBJADDR_SRCHM_SEL);

	/* push ssidlen out to shm
	 * XXX: again? Already sent to search engine ...
	 * XXX: awkward rmw sequence
	 */

	/* Defer pushing the ssidlen out to shm until the beacon len is pushed.
	 * Except ssidlen is 0, push ssidlen 0 imply disabling beaconing on that
	 * BSS.
	 */
	if (ssidlen == 0)
		wlc_mbss16_updssid_len(wlc, cfg);
}

static void
wlc_mbss16_updssid_len(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	uint32 start;
	uint16 val;
	uint32 ssidlen = cfg->SSID_len;
	bss_mbss_info_t *bmi;
	int8 ucidx;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	bmi = BSS_MBSS_INFO(wlc->mbss, cfg);
	ASSERT(bmi != NULL);

	ucidx = bmi->_ucidx;
	ASSERT((ucidx >= 0) && (ucidx <= WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)));

	/* push ssidlen out to shm */
	start = SHM_MBSS_SSIDLEN0 + (ucidx & 0xFE);
	val = wlc_read_shm(wlc, start);
	/* set bit indicating closed net if appropriate */
	if (cfg->closednet_nobcnssid)
		ssidlen |= SHM_MBSS_CLOSED_NET;

	if (ucidx & 0x01) {
		val &= 0xff;
		val |= ((uint8)ssidlen << 8);

	} else {
		val &= 0xff00;
		val |= (uint8)ssidlen;
	}

	wlc_write_shm(wlc, start, val);
}

/* Updates SHM for closed net */
void
wlc_mbss16_upd_closednet(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	wlc_mbss_info_t *mbss = wlc->mbss;
	uint32 start;
	bss_mbss_info_t *bmi;
	int8 ucidx;
	uint16 val;
	uint8 shift = 0;

	ASSERT(MBSS_SUPPORT(wlc->pub));

	bmi = BSS_MBSS_INFO(mbss, cfg);
	ASSERT(bmi != NULL);

	ucidx = bmi->_ucidx;
	ASSERT((ucidx >= 0) && (ucidx <= WLC_MBSS_UCIDX_MASK(wlc->pub->corerev)));

	if (ucidx & 0x01)
		shift = 8;
	start = SHM_MBSS_SSIDLEN0 + (ucidx & 0xFE);
	val = wlc_read_shm(wlc, start);
	val = val & ~(SHM_MBSS_CLOSED_NET << shift);
	if (cfg->closednet_nobcnssid)
		val = val | (SHM_MBSS_CLOSED_NET << shift);
	wlc_write_shm(wlc, start, val);
}

#if defined(WLC_HIGH) && defined(WLC_LOW)

/* ******* Probe Request Fifo Handling: Generate Software Probe Responses ******* */

/*
 * Convert raw PRQ entry to info structure
 * Returns error if bsscfg not found in wlc structure
 */

static int
prq_entry_convert(wlc_info_t *wlc, shm_mbss_prq_entry_t *in, wlc_prq_info_t *out)
{
	int uc_idx, sw_idx;

	bzero(out, sizeof(*out));
	bcopy(in, &out->source, sizeof(shm_mbss_prq_entry_t));
	if (ETHER_ISNULLADDR(&out->source.ta)) {
		WL_ERROR(("prq_entry_convert: PRQ Entry for Transmitter Address is NULL\n"));
		return -1;
	}

	out->directed_ssid = SHM_MBSS_PRQ_ENT_DIR_SSID(in);
	out->directed_bssid = SHM_MBSS_PRQ_ENT_DIR_BSSID(in);
	out->is_directed = (out->directed_ssid || out->directed_bssid);
	out->frame_type = SHM_MBSS_PRQ_ENT_FRAMETYPE(in);
	out->up_band = SHM_MBSS_PRQ_ENT_UPBAND(in);
	out->plcp0 = SHM_MBSS_PRQ_ENT_PLCP0(in);
#ifdef WLLPRS
	out->is_htsta = SHM_MBSS_PRQ_ENT_HTSTA(in);
#endif /* WLLPRS */

	if (out->is_directed) {
		uc_idx = SHM_MBSS_PRQ_ENT_UC_BSS_IDX(in);
		sw_idx = WLC_BSSCFG_HW2SW_IDX(wlc, uc_idx);
		if (sw_idx < 0) {
			return -1;
		}
		out->bsscfg = wlc->bsscfg[sw_idx];
		ASSERT(out->bsscfg != NULL);
	}

	return 0;
}

#if defined(BCMDBG)
static void
prq_entry_dump(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint16 rd_ptr, shm_mbss_prq_entry_t *entry)
{
	uint8 *ptr;
	char ssidbuf[SSID_FMT_BUF_LEN];

	ptr = (uint8 *)entry;
	if (rd_ptr != 0) {
		WL_MBSS(("Dump of raw PRQ entry from offset 0x%x (word offset 0x%x)\n",
			rd_ptr * 2, rd_ptr));
	} else {
		WL_MBSS(("    Dump of raw PRQ entry\n"));
	}
	WL_MBSS(("    %02x%02x %02x%02x %02x%02x %02x%02x %04x\n",
		ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5],
		entry->prq_info[0], entry->prq_info[1], entry->time_stamp));
	WL_MBSS(("    %sdirected SSID. %sdirected BSSID. uc_idx: %d. type %d. upband %d.\n",
		SHM_MBSS_PRQ_ENT_DIR_SSID(entry) ? "" : "not ",
		SHM_MBSS_PRQ_ENT_DIR_BSSID(entry) ? "" : "not ",
		SHM_MBSS_PRQ_ENT_UC_BSS_IDX(entry),
		SHM_MBSS_PRQ_ENT_FRAMETYPE(entry),
		SHM_MBSS_PRQ_ENT_UPBAND(entry)));
	if (bsscfg != NULL) {
		wlc_format_ssid(ssidbuf, bsscfg->SSID, bsscfg->SSID_len);
		WL_MBSS(("    Entry mapped to bss %d, ssid %s\n", WLC_BSSCFG_IDX(bsscfg), ssidbuf));
	}
}

static void
prq_info_dump(wlc_info_t *wlc, wlc_prq_info_t *info)
{
	WL_MBSS(("Dump of PRQ info: dir %d. dir ssid %d. dir bss %d. bss cfg idx %d\n",
		info->is_directed, info->directed_ssid, info->directed_bssid,
		WLC_BSSCFG_IDX(info->bsscfg)));
	WL_MBSS(("    frame type %d, up_band %d, plcp0 0x%x\n",
		info->frame_type, info->up_band, info->plcp0));
	prq_entry_dump(wlc, info->bsscfg, 0, &info->source);
}
#else
#define prq_entry_dump(a, b, c, d)
#define prq_info_dump(a, b)
#endif /* BCMDBG */

/*
 * PRQ FIFO Processing
 */

/* Given a PRQ entry info structure, generate a PLCP header for a probe response and fixup the
 * txheader of the probe response template
 */
static void
wlc_prb_resp_plcp_hdrs(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_prq_info_t *info,
	int length, uint8 *plcp, d11txh_t *txh, uint8 *d11_hdr)
{
	uint32 tmp;

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		WL_INFORM(("wl%d: %s: need rev40 update\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* generate the PLCP header */
	switch (info->frame_type) {
	case SHM_MBSS_PRQ_FT_OFDM:
		bzero(plcp, D11_PHY_HDR_LEN);
		/* Low order 4 bits preserved from plcp0 */
		plcp[0] = (info->plcp0 & 0xf);
		/* The rest is the length, shifted over 5 bits, little endian */
		tmp = (length & 0xfff) << 5;
		plcp[2] |= (tmp >> 16) & 0xff;
		plcp[1] |= (tmp >> 8) & 0xff;
		plcp[0] |= tmp & 0xff;
		break;
	case SHM_MBSS_PRQ_FT_MIMO:
		plcp[0] = info->plcp0;
		WLC_SET_MIMO_PLCP_LEN(plcp, length);
		break;
	case SHM_MBSS_PRQ_FT_CCK:
		wlc_cck_plcp_set(info->plcp0 / 5, length, plcp);
		break;
	default:
		WL_ERROR(("Received illegal frame type in PRQ\n"));
		break;
	}

	/* for OFDM, MIMO and CCK fixup txheader, d11hdr */
	if (info->frame_type == SHM_MBSS_PRQ_FT_OFDM || info->frame_type == SHM_MBSS_PRQ_FT_MIMO ||
		info->frame_type == SHM_MBSS_PRQ_FT_CCK) {
		uint16 phyctl = 0;
		uint16 mainrates;
		uint16 xfts;
		uint16 durid;
		ratespec_t rspec;
		struct dot11_header *h = (struct dot11_header *)d11_hdr;
		uint8 preamble;
#ifdef WLMCHAN
		chanspec_t chanspec = bsscfg->current_bss->chanspec;
#else
		chanspec_t chanspec = wlc->chanspec;
#endif /* WLMCHAN */
		wlcband_t *band = wlc->bandstate[CHSPEC_BANDUNIT(chanspec)];

		/* plcp0 low 4 bits have incoming rate, we'll respond at same rate */
		if (info->frame_type == SHM_MBSS_PRQ_FT_OFDM) {
			rspec = ofdm_plcp_to_rspec(plcp[0]);
			phyctl = FT_OFDM;
		}
		else if (info->frame_type == SHM_MBSS_PRQ_FT_MIMO) {
			rspec = HT_RSPEC(plcp[0]);
			phyctl = FT_HT;
		}
		else {
			rspec = plcp[0]/5;
			phyctl = FT_CCK;
		}

#if defined(WL11N) || defined(WL11AC)
		if (WLCISHTPHY(band) || WLCISACPHY(band)) {
			uint16 phytxant;
			phytxant = wlc_stf_txcore_get(wlc, rspec) << PHY_TXC_ANT_SHIFT;
			phyctl |= phytxant & PHY_TXC_HTCORE_MASK;
		} else
#endif /* WL11N || WL11AC */
			phyctl |= wlc->stf->phytxant & PHY_TXC_ANT_MASK;

		txh->PhyTxControlWord = htol16(phyctl);

		mainrates = D11A_PHY_HDR_GRATE((ofdm_phy_hdr_t *)plcp);
		txh->MainRates = htol16(mainrates);

		/* leave "most" of existing XtraFrameTypes, but make sure Fallback Frame Type
		 * is set to FT_OFDM.
		 */
		xfts = ltoh16(txh->XtraFrameTypes);
		xfts &= 0xFFFC;
		if (info->frame_type == SHM_MBSS_PRQ_FT_OFDM)
			xfts |= FT_OFDM;
		else if (info->frame_type == SHM_MBSS_PRQ_FT_MIMO)
			xfts |= FT_HT;
		else
			xfts |= FT_CCK;

		txh->XtraFrameTypes = htol16(xfts);

		/* dup plcp as fragplcp, same fallback rate etc */
		bcopy(plcp, (char*)&txh->FragPLCPFallback, sizeof(txh->FragPLCPFallback));

		/* Possibly fixup some more fields */
		if (WLCISNPHY(band) || WLCISLPPHY(band) || WLCISSSLPNPHY(band)) {
			uint16 phyctl1 = 0;

			/* When uCode handles probe responses, they use same rate for fallback
			 * as the main rate, so we'll do the same.
			 */

			/* the following code expects the BW setting in the ratespec */
			rspec &= ~RSPEC_BW_MASK;
			rspec |= RSPEC_BW_20MHZ;

			phyctl1 = wlc_phytxctl1_calc(wlc, rspec, chanspec);
			txh->PhyTxControlWord_1 = htol16(phyctl1);
			txh->PhyTxControlWord_1_Fbr = htol16(phyctl1);
		}

		/* fixup dur based on our tx rate */
		if (RSPEC_ISHT(rspec) || RSPEC_ISVHT(rspec)) {
			preamble = WLC_MM_PREAMBLE;
		} else {
			preamble = WLC_LONG_PREAMBLE;
		}

		durid = wlc_compute_frame_dur(wlc, rspec, preamble, 0);
		h->durid = htol16(durid);
		txh->FragDurFallback = h->durid;
	}
}

static void
prb_pkt_final_setup(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_prq_info_t *info,
	uint8 *pkt_start, int len)
{
/* Time to live for probe response frames in microseconds; timed from when request arrived */
#if !defined(WLC_PRB_TTL_us)
#define WLC_PRB_TTL_us 40000
#endif /* ttl defined */

	d11txh_t *txh;
	uint8 *plcp_hdr;
	uint8 *d11_hdr;
	uint8 *d11_da;
	int plcp_len;
	uint32 exptime;
	uint16 exptime_low;
	uint16 mcl;

	txh = (d11txh_t *)pkt_start;
	plcp_hdr = &pkt_start[D11_TXH_LEN];
	d11_hdr = &pkt_start[D11_TXH_LEN + D11_PHY_HDR_LEN];
	d11_da = &d11_hdr[2 * sizeof(uint16)]; /* skip fc and duration/id */

	/* Set up the PHY header */
	plcp_len = len - D11_TXH_LEN - D11_PHY_HDR_LEN + DOT11_FCS_LEN;
	wlc_prb_resp_plcp_hdrs(wlc, bsscfg, info, plcp_len, plcp_hdr, txh, d11_hdr);

#if WLC_PRB_TTL_us > 0
	/* Set the packet expiry time */
	exptime = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
	exptime_low = (uint16)exptime;
	exptime = (exptime & 0xffff0000) | info->source.time_stamp;
	if (exptime_low < info->source.time_stamp) {	/* Rollover occurred */
		exptime -= 0x10000;	/* Decrement upper 16 bits. */
	}
	exptime += WLC_PRB_TTL_us;

	txh->TstampLow = htol16(exptime & 0xffff);
	txh->TstampHigh = htol16((exptime >> 16) & 0xffff);
	mcl = ltoh16(txh->MacTxControlLow);
	mcl |= TXC_LIFETIME;
	txh->MacTxControlLow = htol16(mcl);
#endif /* WLC_PRB_TTL_us > 0 */

	/* Set up the dest addr */
	bcopy(&info->source.ta, d11_da, sizeof(struct ether_addr));
}

/* Respond to the given PRQ entry on the given bss cfg */
static void
wlc_prq_directed(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wlc_prq_info_t *info)
{
	wlc_pkt_t pkt, template;
	uint8 *pkt_start;
	int len;
	bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, bsscfg);

	ASSERT(bmi != NULL);

#ifdef WLLPRS
	/* We need to change from the template */
	if (!info->is_htsta)
		template = bsscfg->lprs_template;
	else
#endif // endif
	template = bmi->probe_template;
	if (template == NULL) {
		return;
	}
	len = PKTLEN(wlc->osh, template);

	/* Allocate a new pkt for DMA; copy from template; set up hdrs */
	pkt = PKTGET(wlc->osh, TXOFF + len, TRUE);
	if (pkt == NULL) {
		WLCNTINCR(bmi->cnt->prb_resp_alloc_fail);
		return;
	}
	/* Template includes TX header, PLCP header and D11 header */
	pkt_start = PKTDATA(wlc->osh, pkt);
	bcopy(PKTDATA(wlc->osh, template), pkt_start, len);
	WLPKTTAGBSSCFGSET(pkt, WLC_BSSCFG_IDX(bsscfg));

	prb_pkt_final_setup(wlc, bsscfg, info, pkt_start, len);
	if (wlc_bmac_dma_txfast(wlc, TX_ATIM_FIFO, pkt, TRUE) < 0) {
		PKTFREE(wlc->osh, pkt, TRUE);
		WL_MBSS(("Failed to transmit probe resp for bss %d\n", WLC_BSSCFG_IDX(bsscfg)));
		WLCNTINCR(bmi->cnt->prb_resp_tx_fail);
	}
}

/* Process a PRQ entry, whether broadcast or directed, generating probe response(s) */

/* Is the given config up and responding to probe responses in SW? */
#define CFG_SOFT_PRB_RESP(cfg) \
	(((cfg) != NULL) && ((cfg)->up) && ((cfg)->enable) && SOFTPRB_ENAB(cfg) &&\
		BSS_MATCH_WLC(wlc, cfg))

static void
wlc_prq_response(wlc_info_t *wlc, wlc_prq_info_t *info)
{
	int idx;
	wlc_bsscfg_t *cfg;

#ifdef WLPROBRESP_SW
	if (WLPROBRESP_SW_ENAB(wlc))
		return;
#endif /* WLPROBRESP_SW */

	if (info->is_directed) {
		ASSERT(info->bsscfg != NULL);
		wlc_prq_directed(wlc, info->bsscfg, info);
	} else if (MBSS_SUPPORT(wlc->pub)) {	/* Broadcast probe response */
		for (idx = 0; idx < WLC_MAXBSSCFG; idx++) {
			cfg = wlc->bsscfg[(idx + wlc->mbss->bcast_next_start) % WLC_MAXBSSCFG];
			if (CFG_SOFT_PRB_RESP(cfg) && !cfg->closednet_nobcprbresp) {
				wlc_prq_directed(wlc, cfg, info);
			}
		}
		/* Move "next start" up to next BSS skipping inactive BSSes */

		for (idx = 0; idx < WLC_MAXBSSCFG; idx++) {
			if (++wlc->mbss->bcast_next_start == WLC_MAXBSSCFG) {
				wlc->mbss->bcast_next_start = 0;
			}
			if (CFG_SOFT_PRB_RESP(wlc->bsscfg[wlc->mbss->bcast_next_start])) {
				break;
			}
		}
	}
}

/*
 * Process the PRQ Fifo.
 * Note that read and write pointers are (uint16 *) in the ucode
 * Return TRUE if more entries to process.
 */

/*
 * After some investigation, it looks like there's never more than 1 PRQ
 * entry to be serviced at a time.  So the bound here is probably inconsequential.
 */
#define PRQBND 5

bool
wlc_prq_process(wlc_info_t *wlc, bool bounded)
{
	uint16 rd_ptr, wr_ptr, prq_base, prq_top;
	shm_mbss_prq_entry_t entry;
	wlc_prq_info_t info;
	int count = 0;
	bool rv = FALSE;  /* Default, no more to be done */
	bool set_rd_ptr = FALSE;

	if (!MBSS_ENAB(wlc->pub)) {
		return FALSE;
	}

	prq_base = wlc->mbss->prq_base;
	prq_top = prq_base + (SHM_MBSS_PRQ_TOT_BYTES / 2);

	rd_ptr = wlc->mbss->prq_rd_ptr;
	wr_ptr = wlc_read_shm(wlc, SHM_MBSS_PRQ_WRITE_PTR);

#if defined(BCMDBG)
	/* Debug checks for rd and wr ptrs */
	if (wr_ptr < prq_base || wr_ptr >= prq_top) {
		WL_ERROR(("Error: PRQ fifo write pointer 0x%x out of bounds (%d, %d)\n",
			wr_ptr, prq_base, prq_top));
		return FALSE;
	}
	if (rd_ptr < prq_base || rd_ptr >= prq_top) {
		WL_ERROR(("Error: PRQ read pointer 0x%x out of bounds; clearing fifo\n", rd_ptr));
		/* Reset read pointer to write pointer, emptying the fifo */
		rd_ptr = wr_ptr;
		set_rd_ptr = TRUE;
	}
#endif /* BCMDBG */

	while (rd_ptr != wr_ptr) {
		WLCNTINCR(wlc->pub->_cnt->prq_entries_handled);
		set_rd_ptr = TRUE;

		/* Copy entry from PRQ; convert and respond; update rd ptr */
		wlc_copyfrom_shm(wlc, rd_ptr * 2, &entry, sizeof(entry));
		if (prq_entry_convert(wlc, &entry, &info) < 0) {
			WL_ERROR(("Error reading prq fifo at offset 0x%x\n", rd_ptr));
			prq_entry_dump(wlc, NULL, rd_ptr, &entry);
			WLCNTINCR(wlc->pub->_cnt->prq_bad_entries);
		} else if (info.is_directed && !(info.bsscfg->up)) { /* Ignore rqst */
			WL_MBSS(("MBSS: Received PRQ entry on down BSS (%d)\n",
				WLC_BSSCFG_IDX(info.bsscfg)));
		} else {
#if defined(BCMDBG) || defined(WLCNT)
			if (info.is_directed && info.bsscfg != NULL) {
				bss_mbss_info_t *bmi = BSS_MBSS_INFO(wlc->mbss, info.bsscfg);
				ASSERT(bmi != NULL);

				if (0) { /* Extra dump for directed requests */
					prq_info_dump(wlc, &info);
				}
				WLCNTINCR(bmi->cnt->prq_directed_entries);
			} else {
				WLCNTINCR(wlc->pub->_cnt->prq_undirected_entries);
			}
#endif // endif
			wlc_prq_response(wlc, &info);
		}

		/* Update the read pointer */
		rd_ptr += sizeof(entry) / 2;
		if (rd_ptr >= prq_top) {
			rd_ptr = prq_base;
		}

		if (bounded && (count++ >= PRQBND)) {
			rv = TRUE; /* Possibly more to do */
			break;
		}
	}

	if (set_rd_ptr) { /* Write the value back when done processing */
		wlc_write_shm(wlc, SHM_MBSS_PRQ_READ_PTR, rd_ptr);
		wlc->mbss->prq_rd_ptr = rd_ptr;
	}

	return rv;
}
#endif /* WLC_HIGH && WLC_LOW */

#endif /* AP */

#endif /* MBSS */
