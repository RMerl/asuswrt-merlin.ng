/*
 * Beacon Trim Functionality module source, 802.1x related.
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
 * $Id: wlc_bcntrim.c 480336 2014-07-22 14:01:26Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_bsscfg.h>
#include <wlc_bcntrim.h>
#include <wlc_bmac.h>
#include <wlc_assoc.h>
#include <wlc_ie_mgmt.h>

/*
 * iovars
 */
enum {
	IOV_BCNTRIM = 0,
	IOV_BCNTRIM_STATS = 1,
	IOV_LAST
};

/* iovar table */
static const bcm_iovar_t bcntrim_iovars[] = {
	{"bcntrim", IOV_BCNTRIM, IOVF_SET_CLK, IOVT_INT32, 0},
	{"bcntrim_stats", IOV_BCNTRIM_STATS, IOVF_SET_CLK,
	IOVT_BUFFER, BCNTRIM_STATS_NUMPARAMS * 2},
	{NULL, 0, 0, 0, 0}
};

/*
 * module private states
 */
struct wlc_bcntrim_info {
	wlc_info_t *wlc;
	int cfgh;
	bool bcntrim_allowed;
	uint32 bcntrim_shmem_addr;
};

/* bsscfg private states */
typedef struct {
	/* Beacon trim feature related vars */
	bool				bcntrim_enabled;
	uint8				bcntrim_threshold;
	struct ether_addr	bcntrim_BSSID;
	uint16				bcntrim_channel;
} bcntrim_bsscfg_cubby_t;

/* access macros */
#define BCNTRIM_BSSCFG_CUBBY_LOC(bcntrim, cfg) \
		((bcntrim_bsscfg_cubby_t **)BSSCFG_CUBBY(cfg, (bcntrim)->cfgh))
#define BCNTRIM_BSSCFG_CUBBY(bcntrim, cfg) \
		(*BCNTRIM_BSSCFG_CUBBY_LOC(bcntrim, cfg))

/* Beacon trim feature related definitions */
#define BCNTRIM_TIMEND				(100)	/* bytes */
#define BCNTRIM_TSF_TOLERENCE			(0x900)	/* 2300 usecs */
#define BCNTRIM_MAX_N_VAL			(10)	/* # of expected beacon */
#define BCNTRIM_MACHDR_SZ			(24)	/* bytes */
#define BCNTRIM_TIMIDDIST_TOLERANCE		(20)	/* bytes */
#define BCNTRIM_TIMIE_MINLEN			(5)	/* bytes */
#define C_BCNTRIM_RSSI_VALID			(1 << 15)
#define C_BCNTRIM_RSSI_MASK			(0xFF)

static int wlc_bcntrim_handle_up_state(void *ctx);
static void wlc_bcntrim_handle_assoc_state(void *ctx, bss_assoc_state_data_t *evt_data);
static int wlc_bcntrim_parse_tim_ie(void *ctx, wlc_iem_parse_data_t *data);
/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* module entries */
static int
wlc_bcntrim_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_bcntrim_info_t *bcntrim = (wlc_bcntrim_info_t *)ctx;
	wlc_info_t *wlc = bcntrim->wlc;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	wlc_bsscfg_t *bsscfg;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	bcntrim_bsscfg_cubby_t *bcntrim_bsscfg = BCNTRIM_BSSCFG_CUBBY(bcntrim, bsscfg);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	switch (actionid) {
	case IOV_GVAL(IOV_BCNTRIM_STATS): {

		if (bcntrim->bcntrim_allowed) {
			uint16 *ret_params = (uint16 *) arg;
			uint8 i;

			for (i = 0; i < BCNTRIM_STATS_NUMPARAMS; i++, ret_params++) {
				*ret_params = (int16) wlc_bmac_read_shm(wlc->hw,
					bcntrim->bcntrim_shmem_addr + (i * 2));
			}
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	}

	case IOV_SVAL(IOV_BCNTRIM_STATS): {
		/* Setting of params not allowed */
		err = BCME_UNSUPPORTED;
		break;
	}

	case IOV_GVAL(IOV_BCNTRIM): {

		WL_BCNTRIM_DBG(("%s: bcntrim_allowed = %d\n", __FUNCTION__,
			bcntrim->bcntrim_allowed));

		if (bcntrim->bcntrim_allowed) {
			/* Read from the wlc_info var, rather than the shmem */
			*ret_int_ptr = bcntrim_bsscfg->bcntrim_threshold;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	}

	case IOV_SVAL(IOV_BCNTRIM): {

		/* Value in range */
		if (int_val < 0 || int_val > BCNTRIM_MAX_N_VAL) {
			err = BCME_RANGE;
			break;
		}

		if (bcntrim->bcntrim_allowed) {
			/* Reset the counters */
			wlc_bmac_write_shm(wlc->hw,
				bcntrim->bcntrim_shmem_addr + (M_BCNTRIM_SBCNRXED * 2), 0);
			wlc_bmac_write_shm(wlc->hw,
				bcntrim->bcntrim_shmem_addr + (M_BCNTRIM_CANTRIM * 2), 0);
			wlc_bmac_write_shm(wlc->hw,
				bcntrim->bcntrim_shmem_addr + (M_BCNTRIM_TRIMMED * 2), 0);
			wlc_bmac_write_shm(wlc->hw,
				bcntrim->bcntrim_shmem_addr + (M_BCNTRIM_BCNLENCNG * 2), 0);
			wlc_bmac_write_shm(wlc->hw,
				bcntrim->bcntrim_shmem_addr + (M_BCNTRIM_TSFADJ * 2), 0);
			wlc_bmac_write_shm(wlc->hw,
				bcntrim->bcntrim_shmem_addr + (M_BCNTRIM_TIMNOTFOUND * 2),
				0);

			/* Write to Shmem to configure the feature */
			wlc_bmac_write_shm(wlc->hw, bcntrim->bcntrim_shmem_addr
				+(M_BCNTRIM_TIMEND * 2),
				BCNTRIM_TIMEND);
			wlc_bmac_write_shm(wlc->hw, bcntrim->bcntrim_shmem_addr
				+ (M_BCNTRIM_TSFTLRN * 2),
				BCNTRIM_TSF_TOLERENCE);
			wlc_bmac_write_shm(wlc->hw, bcntrim->bcntrim_shmem_addr
				+ (M_BCNTRIM_N * 2),
				int_val);

			/* Set the flag if required */
			bcntrim_bsscfg->bcntrim_threshold = int_val;
			bcntrim_bsscfg->bcntrim_enabled = (int_val > 1);

			WL_BCNTRIM_DBG(("BCNTRIM: bcntrim_enabled = %d..bcntrim_threshold = %d "
				"...bcntrim_shmem_addr = %x \n", bcntrim_bsscfg->bcntrim_enabled,
				bcntrim_bsscfg->bcntrim_threshold,
				bcntrim->bcntrim_shmem_addr));
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* bsscfg cubby */
static int
wlc_bcntrim_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_bcntrim_info_t *bcntrim_info = (wlc_bcntrim_info_t *)ctx;
	wlc_info_t *wlc = bcntrim_info->wlc;
	bcntrim_bsscfg_cubby_t **pbcntrim_bsscfg = BCNTRIM_BSSCFG_CUBBY_LOC(bcntrim_info, cfg);
	bcntrim_bsscfg_cubby_t *bcntrim_bsscfg = NULL;

	/* allocate memory and point bsscfg cubby to it */
	if ((bcntrim_bsscfg = MALLOCZ(wlc->osh, sizeof(bcntrim_bsscfg_cubby_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}
	*pbcntrim_bsscfg = bcntrim_bsscfg;
	return BCME_OK;

}

static void
wlc_bcntrim_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_bcntrim_info_t *bcntrim_info = (wlc_bcntrim_info_t *)ctx;
	wlc_info_t *wlc = bcntrim_info->wlc;
	bcntrim_bsscfg_cubby_t **pbcntrim_bsscfg = BCNTRIM_BSSCFG_CUBBY_LOC(bcntrim_info, cfg);
	bcntrim_bsscfg_cubby_t *bcntrim_bsscfg = NULL;

	/* free the Cubby reserve allocated memory  */
	bcntrim_bsscfg = *pbcntrim_bsscfg;
	if (bcntrim_bsscfg) {
		MFREE(wlc->osh, bcntrim_bsscfg, sizeof(bcntrim_bsscfg_cubby_t));
		*pbcntrim_bsscfg = NULL;
	}
}

static const char BCMATTACHDATA(rstr_bcntrim)[] = "bcntrim";

wlc_bcntrim_info_t *
BCMATTACHFN(wlc_bcntrim_attach)(wlc_info_t *wlc)
{
	wlc_bcntrim_info_t *bcntrim_info;
	if ((bcntrim_info = MALLOCZ(wlc->osh, sizeof(wlc_bcntrim_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	bcntrim_info->wlc = wlc;

	/* reserve bsscfg cubby */
	if ((bcntrim_info->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bcntrim_bsscfg_cubby_t *),
	                wlc_bcntrim_bsscfg_init, wlc_bcntrim_bsscfg_deinit, NULL,
	                (void *)bcntrim_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* assoc join-start/done callback */
	if (wlc_bss_assoc_state_register(wlc,
		(bss_assoc_state_fn_t)wlc_bcntrim_handle_assoc_state,
		bcntrim_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register wlc module */
	if (wlc_module_register(wlc->pub, bcntrim_iovars, rstr_bcntrim, bcntrim_info,
		wlc_bcntrim_doiovar, NULL, wlc_bcntrim_handle_up_state, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

#ifdef STA
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_TIM_ID,
		wlc_bcntrim_parse_tim_ie, bcntrim_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, tim ie in bcn\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */

	wlc->pub->_bcntrim = TRUE;

	return bcntrim_info;

fail:
	wlc_bcntrim_detach(bcntrim_info);
	return NULL;
}

void
BCMATTACHFN(wlc_bcntrim_detach)(wlc_bcntrim_info_t *bcntrim_info)
{
	wlc_info_t *wlc;

	if (bcntrim_info == NULL)
		return;

	wlc = bcntrim_info->wlc;
	wlc_module_unregister(wlc->pub, rstr_bcntrim, bcntrim_info);

	/* unregister assoc state update callbacks */
	wlc_bss_assoc_state_unregister(wlc, wlc_bcntrim_handle_assoc_state, bcntrim_info);

	MFREE(wlc->osh, bcntrim_info, sizeof(wlc_bcntrim_info_t));
}

/* This function is used to set bcntrim allowed flag depending on the
* M_BCNTRIM_BLK_PTR SHM value
*  This will be called during wl up functionality.
*/
static int
wlc_bcntrim_handle_up_state(void *ctx)
{
	wlc_bcntrim_info_t *bcntrim_info = (wlc_bcntrim_info_t *)ctx;
	wlc_info_t *wlc = bcntrim_info->wlc;
	uint32 bcntrim_shmem_addr = 2 * wlc_bmac_read_shm(wlc->hw,
		M_BCNTRIM_BLK_PTR);
	bcntrim_info->bcntrim_shmem_addr = bcntrim_shmem_addr;
	if (wlc_bmac_read_shm(wlc->hw, M_BCNTRIM_BLK_PTR)) {
		bcntrim_info->bcntrim_allowed = TRUE;
	} else {
		bcntrim_info->bcntrim_allowed = FALSE;
	}
	WL_BCNTRIM_DBG(("%s: bcntrim_allowed = %d bcntrim_shmem_addr = %x\n",
		__FUNCTION__,
		bcntrim_info->bcntrim_allowed,
		bcntrim_info->bcntrim_shmem_addr));
	return BCME_OK;
}

/* This function is used to update  M_AID_NBIT , bcntrim_BSSID &
*  bcntrim_channel value
*  This will be called after successful association
*/
static void
wlc_bcntrim_handle_assoc_state(void *ctx, bss_assoc_state_data_t *evt_data)
{
	wlc_bcntrim_info_t *bcntrim_info = (wlc_bcntrim_info_t *)ctx;
	wlc_info_t *wlc = bcntrim_info->wlc;
	wlc_bss_info_t	*target_bss;
	if (evt_data->cfg == NULL)
		return;

	if (evt_data->state == AS_JOIN_ADOPT) {

		bcntrim_bsscfg_cubby_t *bcntrim_bsscfg =
			BCNTRIM_BSSCFG_CUBBY(wlc->bcntrim, evt_data->cfg);
		target_bss = evt_data->cfg->target_bss;

		WL_BCNTRIM_DBG(("%s: bcntrim_allowed = %d "
			"bssid = %x:%x chanspec = 0x%x\n", __FUNCTION__,
			bcntrim_info->bcntrim_allowed,
			evt_data->cfg->target_bss->BSSID.octet[4],
			evt_data->cfg->target_bss->BSSID.octet[5],
			evt_data->cfg->target_bss->chanspec));

		if (bcntrim_info->bcntrim_allowed) {
			/* Write AID to the Shmem and note the BSSID and channel */
			wlc_bmac_write_shm(wlc->hw, (M_AID_NBIT), (wlc->AID & DOT11_AID_MASK));
			memcpy(&bcntrim_bsscfg->bcntrim_BSSID, &target_bss->BSSID,
				sizeof(struct ether_addr));
			bcntrim_bsscfg->bcntrim_channel = CHSPEC_CHANNEL(target_bss->chanspec);
		}
	}
}

/* This function will enable bcntrim functionality in SHM
*  This will be called after successful association
*/
void
wlc_bcntrim_update_bcntrim_enab(wlc_bcntrim_info_t *bcntrim_info, wlc_bsscfg_t * bsscfg,
uint32 new_mc)
{
	wlc_info_t *wlc = bcntrim_info->wlc;
	bcntrim_bsscfg_cubby_t *bcntrim_bsscfg = BCNTRIM_BSSCFG_CUBBY(wlc->bcntrim, bsscfg);

	WL_BCNTRIM_DBG(("%s: bcntrim_enabled = %d new_mc = %d \n", __FUNCTION__,
		bcntrim_bsscfg->bcntrim_enabled, new_mc));

	if (bcntrim_bsscfg->bcntrim_enabled) {
		if (new_mc) {
			/* Enable the beacon trimming in shmem */
			wlc_bmac_write_shm(wlc->hw, bcntrim_info->bcntrim_shmem_addr +
				(M_BCNTRIM_N * 2), bcntrim_bsscfg->bcntrim_threshold);
		} else {
			/* Disable the beacon trimming in shmem */
			wlc_bmac_write_shm(wlc->hw, bcntrim_info->bcntrim_shmem_addr +
				(M_BCNTRIM_N * 2),	0);
		}
	}
}

/* This function is used to update TIM distance to M_BCNTRIM_TIMEND
*  SHM address
*  This will be called from wlc_bcn_parse_tim_ie
*/
static int
wlc_bcntrim_parse_tim_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_bcntrim_info_t *bcntrim_info = (wlc_bcntrim_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	bcntrim_bsscfg_cubby_t *bcntrim_bsscfg;
	dot11_tim_ie_t *tim_ie;
	uint8* body;
	uint16 tim_end, tim_len;

	if ((data == NULL)||(cfg == NULL))
		return BCME_ERROR;

	tim_ie = (dot11_tim_ie_t*)data->ie;
	body = data->pparm->body;
	bcntrim_bsscfg = BCNTRIM_BSSCFG_CUBBY(bcntrim_info, cfg);

	if ((tim_ie == NULL)||(body == NULL)||(bcntrim_bsscfg == NULL))
		return BCME_ERROR;

	WL_BCNTRIM_DBG(("%s: bcntrim_enabled = %d PM = %d \n", __FUNCTION__,
		bcntrim_bsscfg->bcntrim_enabled,
		cfg->pm->PM_override));

	if ((bcntrim_bsscfg->bcntrim_enabled) &&
		(cfg->pm->PM_override == FALSE)) {

		/* Compute the max distance to end of TIM */
		tim_len = CEIL((cfg->AID & DOT11_AID_MASK), 8);
		if ((tim_len % 8) == 0)
			tim_len++;
		tim_end = ((uint8 *) tim_ie - (uint8 *) body) + tim_len +
			BCNTRIM_TIMIE_MINLEN + BCNTRIM_MACHDR_SZ +
			BCNTRIM_TIMIDDIST_TOLERANCE;	/* bytes */

		/* Write to TIM distance */
		wlc_bmac_write_shm(cfg->wlc->hw,
			bcntrim_info->bcntrim_shmem_addr + (M_BCNTRIM_TIMEND * 2), tim_end);

		WL_BCNTRIM_DBG(("%s: tim_len= %d tim_end = %d \n", __FUNCTION__,
			tim_len, tim_end));
	}
	return BCME_OK;
}

/* This function is used to process partial beacon and update
*  SM variables properly
*  This will be called from wlc_high_dpc function
*/
void
wlc_bcntrim_recv_process_partial_beacon(wlc_bcntrim_info_t *bcntrim_info)
{
	bcntrim_bsscfg_cubby_t *bcntrim_bsscfg = NULL;
	wlc_bsscfg_t *bsscfg = NULL;
	wlc_roam_t *roam = NULL;
	uint16 rssi_word, chan_num;
	int8 rssi;
#ifdef BCMDBG
	uint16 counter;
#endif // endif
	wlc_info_t *wlc = bcntrim_info->wlc;

	bcntrim_bsscfg = BCNTRIM_BSSCFG_CUBBY(wlc->bcntrim, bsscfg);
	bsscfg = wlc_bsscfg_find_by_bssid(wlc, &bcntrim_bsscfg->bcntrim_BSSID);

	WL_BCNTRIM_DBG(("%s: bcntrim_enabled = %d \n", __FUNCTION__,
		bcntrim_bsscfg->bcntrim_enabled));

	if ((!bcntrim_bsscfg->bcntrim_enabled) || (bsscfg == NULL))
		return;

	/* Make sure device is STA and associated */
	ASSERT(BSSCFG_STA(bsscfg));
	ASSERT(bsscfg->associated);

	/* This feature should not be enabled along with the following */
	ASSERT(!MCHAN_ENAB(wlc->pub));
	ASSERT(!AP_ACTIVE(wlc));
	ASSERT(!WME_ENAB(wlc->pub));
	ASSERT(!CCX_ENAB(wlc->pub));

	/* Read & reset the RSSI in shmem */
	rssi_word = wlc_bmac_read_shm(wlc->hw, bcntrim_info->bcntrim_shmem_addr +
		(M_BCNTRIM_RSSI * 2));
	wlc_bmac_write_shm(wlc->hw, bcntrim_info->bcntrim_shmem_addr +
		(M_BCNTRIM_RSSI * 2), 0);
	/* Quit if RSSI has not been updated */
	if (!(rssi_word & C_BCNTRIM_RSSI_VALID))
		return;
	rssi = rssi_word & C_BCNTRIM_RSSI_MASK;

	/* Read the Channel from Shmem */
	chan_num = wlc_bmac_read_shm(wlc->hw, bcntrim_info->bcntrim_shmem_addr +
		(M_BCNTRIM_CHANNEL * 2));
	if (chan_num == bcntrim_bsscfg->bcntrim_channel)
		roam = bsscfg->roam;
	else
		return;

#ifdef BCMDBG
	counter = wlc_bmac_read_shm(wlc->hw, bcntrim_info->bcntrim_shmem_addr +
		(M_BCNTRIM_N_COUNTER * 2));
	WL_BCNTRIM_DBG(("P = %d 0x%x %d\n", counter, rssi, chan_num));
#endif // endif
	/* Update the RSSI into the algo */
	wlc_lq_rssi_update_ma(bsscfg, rssi, 0, FALSE);
	wlc_lq_rssi_event_update(bsscfg);

	if (BSSCFG_STA(bsscfg)) {
		bsscfg->current_bss->RSSI = (int16)bsscfg->link->rssi;
		if (bsscfg->BSS &&
#ifdef WLP2P
			!BSS_P2P_ENAB(wlc, bsscfg) &&
#endif // endif
			!roam->off)
				wlc_roamscan_start(bsscfg, WLC_E_REASON_LOW_RSSI);
	}

	roam->time_since_bcn = 0;
#ifdef BCMDBG
	roam->tbtt_since_bcn = 0;
#endif // endif
}
