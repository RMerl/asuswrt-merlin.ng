/*
 * MU-MIMO receive module for Broadcom 802.11 Networking Adapter Device Drivers
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
 * $Id: wlc_murx.c 589130 2015-09-28 07:25:59Z $
 */

/*
 * An MU-MIMO receiver listens for Group ID management frames, which it uses
 * to determine which MU-MIMO groups it is a member of, and for each group, the
 * user position in the VHT-SIG-A that indicates the number of spatial streams
 * in received MU-PPDUs to demodulate. This module extracts this information
 * from Group ID management frames, stores this state on a per-BSS basis, and
 * updates the hardware with new membership data.
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <wlc_types.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wl_dbg.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_txbf.h>
#include <wlc_murx.h>
#include <phy_mu_api.h>
#ifdef WLCNT
#include <wlc_rate.h>
#include <wlc_vht.h>
#endif // endif
#ifdef WL_MODESW
#include <wlc_modesw.h>
#endif // endif

#ifdef WL_MU_RX

/* Maximum VHT MCS index. MCS 0 - 11, up to 4 streams */
#define MURX_MCS_NUM  12
#define MURX_NSS_NUM   4
#define MURX_MCS_INDEX_NUM  (MURX_MCS_NUM*MURX_NSS_NUM)

#define WLC_MURX_BSSCFG_CONFIG_SIZE (sizeof(murx_bsscfg_t))

/* IOVAR table */
enum {
	IOV_MURX_MEMB_CLEAR,
	IOV_MURX_CLEAR_DUMP,	/* clear MURX counters */
	IOV_MURX_LAST
};

static const bcm_iovar_t murx_iovars[] = {
#ifdef BCMDBG
	/* This really just here for development test. Will likely remove. */
	{"mu_membership_clear", IOV_MURX_MEMB_CLEAR, 0, IOVT_UINT32, 0},
#endif // endif
#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU))
	{"murx_clear_dump", IOV_MURX_CLEAR_DUMP, 0, IOVT_UINT32, 0},
#endif  /* defined(BCMDBG) || defined(WLTEST) */
	{NULL, 0, 0, 0, 0 }
};

/* State structure for the MU-MIMO receive module created by
 * wlc_murx_module_attach().
 */
struct wlc_murx_info {
	osl_t *osh;              /* OSL handle */
	wlc_info_t *wlc;         /* wlc_info_t handle */
	wlc_pub_t *pub;          /* wlc_pub_t handle */

	/* BSS config cubby offset */
	int bsscfg_handle;

	/* MU group membership can only be written to hardware for a single BSS.
	 * A receiver can associate to multiple BSSs and receive GID update messages
	 * from each. To avoid overwriting the group membership in hardware, record
	 * the bsscfg whose group membership is currently in hardware. A GID message
	 * received in the same BSS can update the hw. GID messages received in other
	 * BSSs are ignored.
	 */
	wlc_bsscfg_t *mu_bss;

	/* Total number of Group ID mgmt frames received in all BSSs. */
	uint16 gid_msg_rx;

	/* Number of GID messages ignored because of memory allocation failure. */
	uint16 gid_msg_no_mem;

	/* Number of GID messages received from a sender not in an infrastructure BSS,
	 * or in a BSS where this device is not a STA.
	 */
	uint16 gid_msg_not_infrabss;

	/* Number of GID messages received from a sender not in the BSS this station
	 * is accepting group membership from.
	 */
	uint16 gid_msg_other_bss;
#ifdef WLCNT
	/* Flag to track MU-AMPDU reception progress. When this flag is true, rxmpdu_mu counter is
	 * incremented for every MPDU reception.
	 */
	bool murx_inprog;
#endif /* WLCNT */
};

/* Data structure used to store the GID information received from AP */
typedef struct murx_gid_info {
	/* Bit mask indicating STA's membership in each MU-MIMO group. Indexed by
	 * group ID. A 1 at index N indicates the STA is a member of group N. Lowest
	 * order bit indicates membership status in group ID 0. Groups 0 and 63 are
	 * reserved for SU.
	 */
	uint8 membership[MU_MEMBERSHIP_SIZE];

	/* Bit string indicating STA's user position within each MU-MIMO group.
	 * A user position is represented by two bits (positions 0 - 3), under the
	 * assumption that no more than 4 MPDUs are included in an MU-PPDU. A
	 * position value is only valid if the corresponding membership flag is set.
	 */
	uint8 position[MU_POSITION_SIZE];
} murx_gid_info_t;

typedef struct murx_stats {
#ifdef WLCNT
	/* Number of data frames received for each group ID. Counts both SU and MU GIDs. */
	uint32 gid_cnt[MIMO_GROUP_NUM];

	/* Last receive rate for each group ID. The rate value is represented as
	 * 2 bits of NSS (actual NSS - 1) followed by 4 bits of MCS.
	 */
	uint8 gid_last_rate[MIMO_GROUP_NUM];

	/* Number of MU-MIMO frames received for each VHT MCS. Indexed by
	 * mcs + ((nss - 1) * mcs_num)
	 */
	uint32 mu_mcs_cnt[MURX_MCS_INDEX_NUM];

	/* Same for SU frames */
	uint32 su_mcs_cnt[MURX_MCS_INDEX_NUM];

	/* To store last received valid PLCP */
	uint8 plcp[D11_PHY_HDR_LEN];
#endif   /* WLCNT */
} murx_stats_t;

/* STA may participate in multiple BSSs with other APs or STAs that send MU-MIMO
 * frames. So membership and user position is stored per BSS.
 */
typedef struct murx_bsscfg {
	/* Flag to indicate if MURX association is active in bsscfg */
	bool murx_assoc;

	/* Number of Group ID Management frames received in this BSS */
	uint16 gid_msg_cnt;

	/* Number of times the hardware update of the group membership failed */
	uint16 hw_update_err;

	/* To store GID information received from the AP */
	murx_gid_info_t *gid_info;

#ifdef WLCNT
	/* Receive data frame statistics */
	murx_stats_t *murx_stats;	/* counters/stats */
#endif /* WLCNT */
} murx_bsscfg_t;

/* Actual BSS config cubby is just a pointer to dynamically allocated structure. */
typedef struct murx_bsscfg_cubby {
	murx_bsscfg_t *murx_bsscfg;
} murx_bsscfg_cubby_t;

/* Basic module infrastructure */
static int wlc_murx_up(void *mi);
static int murx_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
                        const char *name, void *p, uint plen, void *a,
                        int alen, int vsize, struct wlc_if *wlcif);

/* BSS configuration cubby routines */
static void wlc_murx_scb_state_upd_cb(void *ctx, scb_state_upd_data_t *notif_data);
static int murx_bsscfg_init(void *hdl, wlc_bsscfg_t *bsscfg);
static void murx_bsscfg_deinit(void *hdl, wlc_bsscfg_t *cfg);
static int wlc_murx_bsscfg_config_get(void *hdl, wlc_bsscfg_t *bsscfg, uint8 *data, int *len);
static int wlc_murx_bsscfg_config_set(void *hdl, wlc_bsscfg_t *bsscfg, const uint8 *data,
	int len);
static int wlc_murx_alloc_bsscfg_gid_info(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg);
static void wlc_murx_reset_bsscfg_gid_info(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg);
static void wlc_murx_free_bsscfg_gid_info(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg);
#if defined(BCMDBG) || defined(WLDUMP)
void murx_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define murx_bsscfg_dump NULL
#endif // endif

#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
static int wlc_murx_dump(wlc_murx_info_t *mu_info, struct bcmstrbuf *b);
void wlc_murx_clear_dump(wlc_murx_info_t *mu_info);
#endif // endif
static wlc_bsscfg_t *wlc_murx_get_murx_bsscfg(wlc_murx_info_t *mu_info);

static int
murx_grp_memb_hw_update(wlc_info_t *wlc, murx_bsscfg_t *mu_bsscfg, struct scb *scb,
	uint8 *new_membership, uint8 *user_position);

#define MURX_BSSCFG_CUBBY(murx, cfg) ((murx_bsscfg_cubby_t *)BSSCFG_CUBBY((cfg), \
	                                    (murx)->bsscfg_handle))
#define MURX_BSSCFG(murx, cfg) ((murx_bsscfg_t *)(MURX_BSSCFG_CUBBY(murx, cfg)->murx_bsscfg))

static uint8
mu_user_pos_get(uint8 *pos_array, uint16 group_id)
{
	uint8 user_pos;
	uint8 pos_mask;

	/* Index of first (low) bit in user position field */
	uint16 bit_index = group_id * MU_POSITION_BIT_LEN;

	if (pos_array == NULL)
		return 0;

	pos_mask = 0x3 << (bit_index % NBBY);

	user_pos = (pos_array[(bit_index) / NBBY] & pos_mask) >> (bit_index % NBBY);
	return user_pos;
}

/* Return TRUE if the membership array indicates membership in at least one group. */
static bool
mu_is_group_member(uint8 *membership)
{
	uint32 *mw = (uint32*) membership;  /* start of one 4-byte word of membership array */
	int i;

	if (!membership)
		return FALSE;

	for (i = 0; i < (MU_MEMBERSHIP_SIZE / sizeof(uint32)); i++) {
		if (*mw != 0) {
			return TRUE;
		}
		mw++;
	}
	return FALSE;
}

/**
 * Create the MU-MIMO receive module infrastructure for the wl
 * driver. wlc_module_register() is called to register the
 * module's handlers.
 *
 * Returns
 *    A wlc_murx_info_t structure, or NULL in case of failure.
 */
wlc_murx_info_t*
BCMATTACHFN(wlc_murx_attach)(wlc_info_t *wlc)
{
	wlc_murx_info_t *mu_info;
	int err;

	/* allocate the main state structure */
	mu_info = MALLOCZ(wlc->osh, sizeof(wlc_murx_info_t));
	if (mu_info == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));

		return NULL;
	}

	mu_info->wlc = wlc;
	mu_info->pub = wlc->pub;
	mu_info->osh = wlc->osh;
	wlc->_mu_rx = FALSE;

	/* Avoid registering callbacks if phy is not MU capable */
	if (!WLC_MU_BFE_CAP_PHY(wlc)) {
		return mu_info;
	}

	err = wlc_module_register(mu_info->pub, murx_iovars, "murx", mu_info,
	                          murx_doiovar, NULL, wlc_murx_up, NULL);

	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed with error %d (%s).\n",
		          wlc->pub->unit, __FUNCTION__, err, bcmerrorstr(err)));

		/* use detach as a common failure deallocation */
		wlc_murx_detach(mu_info);
		return NULL;
	}

	/* reserve cubby in the bsscfg container for private data. Allocation of cubby
	 * structure is deferred to receipt of first GID message. So no need to
	 * register init function.
	 */
	if ((mu_info->bsscfg_handle = wlc_bsscfg_cubby_reserve_ext(wlc,
	                                                       sizeof(struct murx_bsscfg_t*),
	                                                       murx_bsscfg_init,
	                                                       murx_bsscfg_deinit,
	                                                       murx_bsscfg_dump,
	                                                       WLC_MURX_BSSCFG_CONFIG_SIZE,
	                                                       wlc_murx_bsscfg_config_get,
	                                                       wlc_murx_bsscfg_config_set,
	                                                       (void *)mu_info)) < 0) {
		WL_ERROR(("wl%d: %s: MURX failed to reserve BSS config cubby\n",
		          wlc->pub->unit, __FUNCTION__));
		wlc_murx_detach(mu_info);
		return NULL;
	}

	/* Add client callback to the scb state notification list */
	if ((wlc_scb_state_upd_register(wlc, (bcm_notif_client_callback)wlc_murx_scb_state_upd_cb,
		mu_info)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_scb_state_upd_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		wlc_murx_detach(mu_info);
		return NULL;
	}

#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
	wlc_dump_register(mu_info->pub, "murx", (dump_fn_t)wlc_murx_dump, mu_info);
#endif /* WLCNT && (BCMDBG || WLDUMP || BCMDBG_MU || BCMDBG_DUMP) */

	return mu_info;
}

/* Free all resources associated with the MU-MIMO receive module.
 * This is done at the cleanup stage when freeing the driver.
 *
 * mu_info    MU-MIMO receive module state structure
 */
void
BCMATTACHFN(wlc_murx_detach)(wlc_murx_info_t *mu_info)
{
	if (mu_info == NULL) {
		return;
	}
#ifndef WL_MU_RX_DISABLED
	wlc_module_unregister(mu_info->pub, "murx", mu_info);
#endif // endif

	MFREE(mu_info->osh, mu_info, sizeof(wlc_murx_info_t));
}

#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
/* Dump MU-MIMO state information. */
static int
wlc_murx_dump(wlc_murx_info_t *mu_info, struct bcmstrbuf *b)
{
	char ssidbuf[SSID_FMT_BUF_LEN];
	uint8 nss, mcs;
	int i;
	murx_stats_t *stats;
	wlc_bsscfg_t *bsscfg = wlc_murx_get_murx_bsscfg(mu_info);
	murx_bsscfg_t *mu_bsscfg;

	if (bsscfg) {
		wlc_format_ssid(ssidbuf, bsscfg->SSID, bsscfg->SSID_len);
	} else {
		wlc_format_ssid(ssidbuf, (uchar *)"<none>", 6);
	}

	bcm_bprintf(b, "MU-MIMO receive state:\n");
	bcm_bprintf(b, "MU RX active in BSS %s\n", ssidbuf);
	bcm_bprintf(b, "GID msgs rxd                  %u\n", mu_info->gid_msg_rx);
	bcm_bprintf(b, "GID msgs dropped. No mem.     %u\n", mu_info->gid_msg_no_mem);
	bcm_bprintf(b, "GID msgs rxd not in infra BSS %u\n", mu_info->gid_msg_not_infrabss);
	bcm_bprintf(b, "GID msgs rxd in other BSS     %u\n", mu_info->gid_msg_other_bss);

	if (bsscfg == NULL)
		return BCME_OK;

	mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);
	if ((mu_bsscfg != NULL) && (mu_bsscfg->murx_stats != NULL)) {
		stats = mu_bsscfg->murx_stats;
		bcm_bprintf(b, "\nMU rx stats for current BSS:\n");

		bcm_bprintf(b, "Frames received for each MU and SU group ID:\n");
		bcm_bprintf(b, " %5s %10s %10s\n", "GID", "FramesRxd", "LastRate");
		for (i = 0; i < MIMO_GROUP_NUM; i++) {
			if (stats->gid_cnt[i] != 0) {
				mcs = stats->gid_last_rate[i] % MURX_MCS_NUM;
				nss = (stats->gid_last_rate[i] / MURX_MCS_NUM) + 1;
				bcm_bprintf(b, " %5u %10u %2ux%1u\n",
					i, stats->gid_cnt[i], mcs, nss);
			}
		}

		bcm_bprintf(b, "\nMU frames received at each rate:\n");
		bcm_bprintf(b, " %4s  %10s", "Rate", "FramesRxd\n");
		for (i = 0; i < MURX_MCS_INDEX_NUM; i++) {
			if (stats->mu_mcs_cnt[i] != 0) {
				mcs = i % MURX_MCS_NUM;
				nss = (i / MURX_MCS_NUM) + 1;
				bcm_bprintf(b, " %2ux%1u  %10u\n",
					mcs, nss, stats->mu_mcs_cnt[i]);
			}
		}

		bcm_bprintf(b, "\nSU VHT frames received at each rate:\n");
		bcm_bprintf(b, " %4s  %10s", "Rate", "FramesRxd\n");
		for (i = 0; i < MURX_MCS_INDEX_NUM; i++) {
			if (stats->su_mcs_cnt[i] != 0) {
				mcs = i % MURX_MCS_NUM;
				nss = (i / MURX_MCS_NUM) + 1;
				bcm_bprintf(b, " %2ux%1u  %10u\n",
					mcs, nss, stats->su_mcs_cnt[i]);
			}
		}
	}

	/* To avoid too many rates in one dump */
	wlc_murx_clear_dump(mu_info);

	return BCME_OK;
}
#endif /* BCMDBG || WLDUMP */

/* IOVar handler for the MU-MIMO infrastructure module */
static int
murx_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
             const char *name,
             void *p, uint plen,
             void *a, int alen,
             int vsize,
             struct wlc_if *wlcif)
{
	int32 int_val = 0;
	int err = 0;
#ifdef BCMDBG
	wlc_murx_info_t *mu_info = (wlc_murx_info_t*) hdl;
	wlc_info_t *wlc = mu_info->wlc;
	wlc_bsscfg_t *bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	murx_bsscfg_t *mu_bsscfg;
#endif // endif

	if (plen >= (int)sizeof(int_val))
		memcpy(&int_val, p, sizeof(int_val));

	switch (actionid) {
#ifdef BCMDBG
		case IOV_SVAL(IOV_MURX_MEMB_CLEAR):
			mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);
			if (mu_bsscfg) {
				(void) murx_grp_memb_hw_update(wlc, mu_bsscfg, NULL, NULL, NULL);
			}
			break;
#endif // endif
#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU))
		case IOV_SVAL(IOV_MURX_CLEAR_DUMP):
		{
			wlc_murx_clear_dump((wlc_murx_info_t*) hdl);
			break;
		}
#endif /* defined(BCMDBG) || defined(WLTEST) */
		default:
			err = BCME_UNSUPPORTED;
			break;
	}

	return err;
}

/* Registered callback when radio comes up. */
static int
wlc_murx_up(void *mi)
{
	wlc_murx_info_t *mu_info = (wlc_murx_info_t*) mi;
	wlc_info_t *wlc = mu_info->wlc;

	/* Check if system is MU BFE capable, both physically and by current
	 * configuration, and if so, set _mu_rx.
	 */
	if (!AP_ENAB(wlc->pub) && !PSTA_IS_REPEATER(wlc) && wlc_txbf_murx_capable(wlc->txbf)) {
		wlc->_mu_rx = TRUE;
	} else {
		wlc->_mu_rx = FALSE;
	}

	/* Have not yet decided which BSS we will do MU in */
	mu_info->mu_bss = NULL;

	return BCME_OK;
}

static void
wlc_murx_scb_state_upd_cb(void *ctx, scb_state_upd_data_t *notif_data)
{
	wlc_murx_info_t *mu_info = (wlc_murx_info_t *)ctx;
	struct scb *scb;
	uint8 oldstate;
	wlc_bsscfg_t *bsscfg;
	murx_bsscfg_t *mu_bsscfg;

	ASSERT(notif_data != NULL);

	scb = notif_data->scb;
	ASSERT(scb != NULL);
	bsscfg = scb->bsscfg;
	oldstate = notif_data->oldstate;

	/* Check if the state transited SCB is internal. In that case we have to do nothing,
	 * just return back
	 */
	if (SCB_INTERNAL(scb))
		return;

	mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);
	if (mu_bsscfg == NULL) {
		return;
	}

	/* When SCB is moving to associated state and bsscfg corresponding to the SCB is marked
	 * as MURX bsscfg, if the AP is MU-Beamformer Capable. This is to ensure that we block
	 * all further association in MURX mode on this WLC
	 */
	if (SCB_ASSOCIATED(scb) && !(oldstate & ASSOCIATED)) {
		if (BSSCFG_INFRA_STA(bsscfg) &&
#ifdef WLP2P
			!BSS_P2P_ENAB(mu_info->wlc, bsscfg) &&
#endif /* WLP2P */
			wlc_murx_is_bi_mu_bfr_cap(mu_info, bsscfg->target_bss) &&
			TRUE) {
			mu_bsscfg->murx_assoc = TRUE;
		} else {
			mu_bsscfg->murx_assoc = FALSE;
		}
	}

	/* For a STA BSSCFG, when SCB is deleted check if we need to clear Group information stored
	 * from this association
	 */
	if (BSSCFG_STA(bsscfg) && ((oldstate & ASSOCIATED) && !SCB_ASSOCIATED(scb))) {
		/* GID information is no more useful once STA is disconnected from the AP.
		 * Reset and free GID information, if present.
		 */
		wlc_murx_free_bsscfg_gid_info(mu_info, bsscfg);
		mu_bsscfg->murx_assoc = FALSE;
	}
}

/* bsscfg cubby init fn */
static int
murx_bsscfg_init(void *hdl, wlc_bsscfg_t *bsscfg)
{
	wlc_murx_info_t *mu_info = (wlc_murx_info_t *)hdl;
	murx_bsscfg_cubby_t *cubby = MURX_BSSCFG_CUBBY(mu_info, bsscfg);
	murx_bsscfg_t *mu_bsscfg;

	/* For any NON-STA bsscfg we don`t really need a MURX cubby */
	if (!BSSCFG_STA(bsscfg)) {
		cubby->murx_bsscfg = NULL;
		return BCME_OK;
	}

	/* Allocate memory and point bsscfg cubby to it */
	if ((mu_bsscfg = MALLOCZ(mu_info->osh, sizeof(murx_bsscfg_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			mu_info->pub->unit, __FUNCTION__, MALLOCED(mu_info->osh)));
		return BCME_NOMEM;
	}

	cubby->murx_bsscfg = mu_bsscfg;

#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
	if ((mu_bsscfg->murx_stats = MALLOCZ(mu_info->osh, sizeof(murx_stats_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			mu_info->pub->unit, __FUNCTION__, MALLOCED(mu_info->osh)));
		return BCME_NOMEM;
	}
#endif /* defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU)) */

	return BCME_OK;
}

/* Deinitialize BSS config cubby. Free memory for cubby. */
static void
murx_bsscfg_deinit(void *hdl, wlc_bsscfg_t *bsscfg)
{
	wlc_murx_info_t *mu_info = (wlc_murx_info_t *)hdl;
	murx_bsscfg_cubby_t *cubby = MURX_BSSCFG_CUBBY(mu_info, bsscfg);
	murx_bsscfg_t *mu_bsscfg = cubby->murx_bsscfg;

	if (mu_bsscfg == NULL)
		return;

#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
	if (mu_bsscfg->murx_stats != NULL) {
		MFREE(mu_info->osh, mu_bsscfg->murx_stats, sizeof(murx_stats_t));
		mu_bsscfg->murx_stats = NULL;
	}
#endif // endif
	/* Reset GID information in cubby + HW and free stored GID information
	 */
	wlc_murx_free_bsscfg_gid_info(mu_info, bsscfg);

	MFREE(mu_info->osh, mu_bsscfg, sizeof(murx_bsscfg_t));
	cubby->murx_bsscfg = NULL;
}

static int
wlc_murx_bsscfg_config_get(void *hdl, wlc_bsscfg_t *bsscfg, uint8 *data, int *len)
{
	wlc_murx_info_t *mu_info = (wlc_murx_info_t *)hdl;
	murx_bsscfg_t *mu_bsscfg;

	if (len == NULL) {
		WL_ERROR(("wl%d: %s: Null length passed\n", mu_info->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	if ((data == NULL) || (*len < WLC_MURX_BSSCFG_CONFIG_SIZE)) {
		WL_ERROR(("wl%d: %s: Buffer too short\n", mu_info->pub->unit, __FUNCTION__));
		*len = WLC_MURX_BSSCFG_CONFIG_SIZE;
		return BCME_BUFTOOSHORT;
	}

	ASSERT(bsscfg != NULL);
	if (!BSSCFG_STA(bsscfg)) {
		*len = 0;
		return BCME_OK;
	}

	mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);

	if (mu_bsscfg == NULL) {
		WL_ERROR(("wl%d: %s: NULL MURX Cubby\n", mu_info->pub->unit, __FUNCTION__));
		*len = 0;
		return BCME_OK;
	}

	/* If current bsscfg is not a MURX cfg, or if current bsscfg clone has been triggered due
	 * to ASSOC ROAM, then we return without doing any cubby copy
	 */
	if (!mu_bsscfg->murx_assoc || !WLC_BSS_ASSOC_NOT_ROAM(bsscfg)) {
		WL_INFORM(("wl%d: %s: Not copying cubby\n", mu_info->pub->unit, __FUNCTION__));
		*len = 0;
		return BCME_OK;
	}

	memcpy(data, (uint8 *)mu_bsscfg, WLC_MURX_BSSCFG_CONFIG_SIZE);
	*len = WLC_MURX_BSSCFG_CONFIG_SIZE;
	return BCME_OK;
}

static int
wlc_murx_bsscfg_config_set(void *hdl, wlc_bsscfg_t *bsscfg, const uint8 *data, int len)
{
	wlc_murx_info_t *mu_info = (wlc_murx_info_t *)hdl;
	murx_bsscfg_t *mu_bsscfg;

	if ((data == NULL) || (len < WLC_MURX_BSSCFG_CONFIG_SIZE)) {
		WL_ERROR(("wl%d: %s: data(%p) len(%d/%d)\n", mu_info->pub->unit, __FUNCTION__,
			data, len, (int)WLC_MURX_BSSCFG_CONFIG_SIZE));
		return BCME_ERROR;
	}

	ASSERT(bsscfg != NULL);
	mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);

	if ((mu_bsscfg == NULL) || !BSSCFG_STA(bsscfg)) {
		WL_ERROR(("wl%d: %s: Not copying cubby\n", mu_info->pub->unit, __FUNCTION__));
		return BCME_OK;
	}

	memcpy((uint8 *)mu_bsscfg, data, WLC_MURX_BSSCFG_CONFIG_SIZE);

	/* Since GID information will be freed as part bsscfg-deinit done on src-bsscfg, we need to
	 * allocate memory to keep a copy in dst-bsscfg.
	 */
	if (mu_bsscfg->gid_info) {
		murx_gid_info_t *gid_info = mu_bsscfg->gid_info;

		mu_bsscfg->gid_info = NULL;
		if (wlc_murx_alloc_bsscfg_gid_info(mu_info, bsscfg)) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				mu_info->pub->unit, __FUNCTION__, MALLOCED(mu_info->osh)));

			/* Since the ucode has already ACK'd the GID message but group state
			 * will not be updated in hardware, it is likely this station will
			 * receive MU-PPDUs that it is unable to demodulate.
			 */
			WLCNTINCR(mu_info->gid_msg_no_mem);
			return BCME_ERROR;
		}

		/* Push group membership and user pos to hardware. */
		if (murx_grp_memb_hw_update(mu_info->wlc, mu_bsscfg, NULL,
			gid_info->membership,
			gid_info->position) != BCME_OK) {
			WLCNTINCR(mu_bsscfg->hw_update_err);
			return BCME_ERROR;
		}
		mu_info->mu_bss = bsscfg;   /* Remember the BSS where GID is RXed */
	}

#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
	/* Since Stats will be freed as part bsscfg-deinit done on src-bsscfg, we need to allocate
	 * memory to keep a copy in dst-bsscfg
	 */
	if (mu_bsscfg->murx_stats) {
		murx_stats_t *murx_stats = mu_bsscfg->murx_stats;
		if ((mu_bsscfg->murx_stats = MALLOCZ(mu_info->osh,
			sizeof(murx_stats_t))) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				mu_info->pub->unit, __FUNCTION__, MALLOCED(mu_info->osh)));
			return BCME_ERROR;
		}
		memcpy((uint8 *)mu_bsscfg->murx_stats, (uint8 *)murx_stats, sizeof(murx_stats_t));
	}
#endif // endif

	return BCME_OK;
}

/* This function handles allocaiton GID information for specified bsscfg */
static int
wlc_murx_alloc_bsscfg_gid_info(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg)
{
	murx_bsscfg_t *mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);

	if ((mu_bsscfg == NULL) || (mu_bsscfg->gid_info != NULL)) {
		WL_ERROR(("wl%d: %s: GID Error!\n", mu_info->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	if ((mu_bsscfg->gid_info = MALLOCZ(mu_info->osh, sizeof(murx_gid_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			mu_info->pub->unit, __FUNCTION__, MALLOCED(mu_info->osh)));
		return BCME_NOMEM;
	}

	return BCME_OK;
}

/* This function handles resetting GID information configured in HW for specified bsscfg */
static void
wlc_murx_reset_bsscfg_gid_info(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg)
{
	wlc_info_t *wlc = mu_info->wlc;

	/* If GID reset request is for the BSS whose MU group membership was written
	 * to hardware, clear the membership info in hw.
	 */
	if (bsscfg == mu_info->mu_bss) {
		murx_bsscfg_t *mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);

		if ((mu_bsscfg != NULL) && (mu_bsscfg->gid_info != NULL)) {
			(void) murx_grp_memb_hw_update(wlc, mu_bsscfg, NULL, NULL, NULL);
		}
		mu_info->mu_bss = NULL;
	}
}

/* This function resets any configured GID information in HW and frees it */
void
wlc_murx_free_bsscfg_gid_info(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg)
{
	murx_bsscfg_t *mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);

	if ((mu_bsscfg != NULL) && (mu_bsscfg->gid_info != NULL)) {
		/* Reset any GID information configured in HW */
		wlc_murx_reset_bsscfg_gid_info(mu_info, bsscfg);

		MFREE(mu_info->osh, mu_bsscfg->gid_info, sizeof(murx_gid_info_t));
		mu_bsscfg->gid_info = NULL;
	}
}

#if defined(BCMDBG) || defined(WLDUMP)
void
murx_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_murx_info_t *mu_info = (wlc_murx_info_t *)ctx;
	murx_bsscfg_t *mu_bsscfg;
	int g;
	uint8 pos;

	bcm_bprintf(b, "MU receive %sactive in this BSS\n",
		(mu_info->mu_bss == cfg) ? "" : "not ");

	mu_bsscfg = MURX_BSSCFG(mu_info, cfg);
	if ((mu_bsscfg != NULL) && (mu_bsscfg->gid_info != NULL)) {
		/* Dump group membership */
		bcm_bprintf(b, "MU-MIMO group membership\n");
		for (g = MU_GROUP_ID_MIN; g < MU_GROUP_ID_MAX; g++) {
			if (isset(mu_bsscfg->gid_info->membership, g)) {
				pos = mu_user_pos_get(mu_bsscfg->gid_info->position, g);
				bcm_bprintf(b, "    Group %u Pos %u\n", g, pos);
			}
		}
#ifdef WLCNT
		bcm_bprintf(b, "Number of Group ID msgs received: %u\n", mu_bsscfg->gid_msg_cnt);
		bcm_bprintf(b, "Number of hw update errors: %u\n", mu_bsscfg->hw_update_err);
#endif // endif
	}
}
#endif   /* BCMDBG || WLDUMP */

/* Filter advertisement of MU BFE capability for a given BSS. A STA can only do
 * MU receive in one BSS. If doing MU rx in a BSS already, then do not advertise
 * MU BFE capability in other BSSs.
 * Inputs:
 *   mu_info - MU receive state
 *   bsscfg  - BSS configuration for BSS where capabilities are to be advertised
 *   cap     - (in/out) vht capabilities
 */
void
wlc_murx_filter_bfe_cap(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg, uint32 *cap)
{
	/* Reset MURX (MU-Beamformee) capability for following:
	 * - AP BSSCFG
	 * - IBSS STA
	 * - P2P BSSCFG
	 * - If MU-RX is already active on some other interface (bsscfg)
	 */
	if (BSSCFG_AP(bsscfg) || BSSCFG_IBSS(bsscfg) ||
#ifdef WLP2P
		BSS_P2P_ENAB(mu_info->wlc, bsscfg) ||
#endif /* WLP2P */
		(wlc_murx_active(mu_info) && wlc_murx_get_murx_bsscfg(mu_info) != bsscfg)) {
		*cap &= ~VHT_CAP_INFO_MU_BEAMFMEE;
	}
}

/* This function checks if specified bss_info is MU Beamformer Capable */
bool
wlc_murx_is_bi_mu_bfr_cap(wlc_murx_info_t *mu_info, wlc_bss_info_t *bi)
{
	return (!!(bi->vht_capabilities & VHT_CAP_INFO_MU_BEAMFMR));
}

/* Function returns bsscfg on which MURX is currently active */
static wlc_bsscfg_t *
wlc_murx_get_murx_bsscfg(wlc_murx_info_t *mu_info)
{
	wlc_info_t *wlc = mu_info->wlc;
	int idx;
	wlc_bsscfg_t *cfg, *murx_bsscfg = NULL;

	FOREACH_AS_BSS(wlc, idx, cfg) {
		murx_bsscfg_t *mu_bsscfg = MURX_BSSCFG(mu_info, cfg);
		if (mu_bsscfg && mu_bsscfg->murx_assoc) {
			murx_bsscfg = cfg;
			break;
		}
	}

	return murx_bsscfg;
}

/* Function to return if we have any active MURX connection is present in given WLC */
bool
wlc_murx_active(wlc_murx_info_t *mu_info)
{
	return (wlc_murx_get_murx_bsscfg(mu_info) != NULL);
}

#ifdef WLRSDB
/* Function to return if we have any active MURX connection is present in case of RSDB mode */
bool
wlc_murx_anymurx_active(wlc_murx_info_t *mu_info)
{
	wlc_info_t *wlc = mu_info->wlc;
	int idx;
	wlc_info_t *wlc_iter;
	bool retval = FALSE;

	FOREACH_WLC(wlc->cmn, idx, wlc_iter) {
		if (wlc_murx_active(wlc_iter->murx)) {
			retval = TRUE;
			break;
		}
	}

	return retval;
}
#endif /* WLRSDB */

/* Push the group membership and user positions for a given BSS to hardware.
 * Called after receiving a GID mgmt frame, but before updating the membership and
 * user position on the bsscfg, so that new membership can be compared with old.
 *
 * Inputs:
 *   wlc - radio
 *   mu_bsscfg - The current group membership and user positions for one BSS.
 *   scb - Source of the group membership information. May be NULL if clearing
 *         all group membership for the BSS.
 *   new_membership - membership array just received in GID mgmt frame. If NULL,
 *                    clear all group membership for this BSS.
 *   user_position - user position for new memberships. May be NULL if
 *                   new_membership is NULL.
 *
 * Returns:
 *   BCME_OK if membership successfully set
 *   BCME_ERROR if any other error
 */
static int
murx_grp_memb_hw_update(wlc_info_t *wlc, murx_bsscfg_t *mu_bsscfg, struct scb *scb,
	uint8 *new_membership, uint8 *user_position)
{
	int g;
	uint8 pos = 0;
	uint8 old_pos = 0;
	wlc_phy_t *pih = WLC_PI(wlc);
	int err;
	int rv = BCME_OK;
	bool was_member;         /* TRUE if STA was previously a member of a given group */
	bool is_member;          /* TRUE if STA is now a member of a given group */
	murx_gid_info_t *gid_info = mu_bsscfg->gid_info;
#if defined(BCMDBG)
	char ssidbuf[SSID_FMT_BUF_LEN];
	char eabuf[ETHER_ADDR_STR_LEN];
	char *change_type;
#endif // endif

	BCM_REFERENCE(scb);

#if defined(BCMDBG)
	if (scb) {
		wlc_format_ssid(ssidbuf, scb->bsscfg->SSID, scb->bsscfg->SSID_len);
	} else {
		wlc_format_ssid(ssidbuf, "", 0);
	}
#endif // endif

	for (g = MU_GROUP_ID_MIN; g < MU_GROUP_ID_MAX; g++) {
		err = BCME_OK;
		was_member = isset(gid_info->membership, g);
		is_member = (new_membership && isset(new_membership, g));

		if (was_member && !is_member) {
			/* Lost membership in this group. pos value irrelevant. */
			DBGONLY(change_type = "remove"; )
			err = phy_mu_group_set((phy_info_t *)pih, g, 0, 0);
		}
		else if (!was_member && is_member) {
			/* Gained membership in this group. */
			DBGONLY(change_type = "add"; )
			pos = mu_user_pos_get(user_position, g);
			err = phy_mu_group_set((phy_info_t *)pih, g, pos, 1);
		} else if (was_member && is_member) {
			/* Still a member of group g. Check in user pos changed */
			DBGONLY(change_type = "modify"; )
			old_pos = mu_user_pos_get(gid_info->position, g);
			pos = mu_user_pos_get(user_position, g);

			if (pos != old_pos) {
				err = phy_mu_group_set((phy_info_t *)pih, g, pos, 1);
			}
		}
		/* no-op if was not and is not a member */
		if (err != BCME_OK) {
#ifdef BCMDBG
			WL_ERROR(("wl%d: %s: Failed to %s MU group %d "
				  "user position %u received from %s in BSS %s. "
				  "Error %d (%s).\n",
				  wlc->pub->unit, __FUNCTION__,
				  change_type, g, pos,
				  scb ? bcm_ether_ntoa(&scb->ea, eabuf) : "", ssidbuf,
				  err, bcmerrorstr(err)));
#endif /* BCMDBG */
			rv = err;
		}
	}
	if (rv == BCME_OK) {
		if (new_membership) {
			memcpy(gid_info->membership, new_membership, MU_MEMBERSHIP_SIZE);
		} else {
			memset(gid_info->membership, 0, MU_MEMBERSHIP_SIZE);
		}

		if (user_position) {
			memcpy(gid_info->position, user_position, MU_POSITION_SIZE);
		} else {
			memset(gid_info->position, 0, MU_POSITION_SIZE);
		}
	}
	return rv;
}

/* Update MU-MIMO group membership and this STA's user position for all MU groups.
 * Called in response to receipt of a Group ID Management action frame.
 * Inputs:
 *   wlc - radio
 *   scb - Neighbor that sent the frame. Implies a BSS.
 *   membership_status - The membership status field in the GID message
 *   user_position - The user position array in the GID message
 * Returns:  BCME_OK
 *           BCME_NOMEM if memory allocation for bsscfg cubby fails
 */
int
wlc_murx_gid_update(wlc_info_t *wlc, struct scb *scb,
                    uint8 *membership_status, uint8 *user_position)
{
	wlc_murx_info_t *mu_info = wlc->murx;
	murx_bsscfg_t *mu_bsscfg;
#ifdef BCMDBG
	int g;
	uint8 pos = 0;
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	if (!MU_RX_ENAB(wlc))
		return BCME_OK;

	if (!scb || SCB_INTERNAL(scb))
		return BCME_OK;

	WLCNTINCR(mu_info->gid_msg_rx);

	/* If this device is not a STA in an infrastructure BSS where the message
	 * was received, ignore the GID message.
	 */
	if (!BSSCFG_INFRA_STA(scb->bsscfg)) {
		WLCNTINCR(mu_info->gid_msg_not_infrabss);
		return BCME_OK;
	}

	/* STA can only accept the MU group membership from a single BSS. If we have
	 * already written group membership to hardware, then can only update the
	 * group membership if this message was received in the same BSS.
	 */
	if (mu_info->mu_bss && (mu_info->mu_bss != scb->bsscfg)) {
		WLCNTINCR(mu_info->gid_msg_other_bss);
		return BCME_OK;
	}

	mu_bsscfg = MURX_BSSCFG(mu_info, scb->bsscfg);
	ASSERT(mu_bsscfg != NULL);
	if (mu_bsscfg->gid_info == NULL) {
		/* Allocate memory for storing received GID information */
		if (wlc_murx_alloc_bsscfg_gid_info(mu_info, scb->bsscfg)) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				mu_info->pub->unit, __FUNCTION__, MALLOCED(mu_info->osh)));

			/* Since the ucode has already ACK'd the GID message but group state
			 * will not be updated in hardware, it is likely this station will
			 * receive MU-PPDUs that it is unable to demodulate.
			 */
			WLCNTINCR(mu_info->gid_msg_no_mem);
			return BCME_NOMEM;
		}
	}

	WLCNTINCR(mu_bsscfg->gid_msg_cnt);
	if (mu_is_group_member(membership_status))
		mu_info->mu_bss = scb->bsscfg;   /* Remember the BSS where GID is RXed */

	/* Push group membership and user pos to hardware. */
	if (murx_grp_memb_hw_update(wlc, mu_bsscfg, scb, membership_status,
		user_position) != BCME_OK) {
		WLCNTINCR(mu_bsscfg->hw_update_err);
		return BCME_ERROR;
	}

#ifdef BCMDBG
	if (WL_MUMIMO_ON()) {
		/* Trace new group membership */
		WL_MUMIMO(("wl%d: Received Group ID Mgmt frame from %s. "
		           "New MU-MIMO group membership:\n",
		           wlc->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf)));
		for (g = MU_GROUP_ID_MIN; g < MU_GROUP_ID_MAX; g++) {
			if (isset(mu_bsscfg->gid_info->membership, g)) {
				pos = mu_user_pos_get(mu_bsscfg->gid_info->position, g);
				WL_MUMIMO(("    Group %u Pos %u\n", g, pos));
			}
		}
	}
#endif  /* BCMDBG */

	return BCME_OK;
}

#if defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU) || \
	defined(BCMDBG_DUMP))
void
wlc_murx_update_rxcounters(wlc_murx_info_t *mu_info, uint32 ft, struct scb *scb,
	struct dot11_header *h)
{
	uint8 *plcp;
	uint8 vht_mcs = 0;
	uint8 nss, mcs;
	uint8 rate_index;
	uint8 gid;
	bool is_mu = FALSE;
	murx_bsscfg_t *mu_bsscfg;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	if (!mu_info || !scb) {
		return;
	}

	mu_bsscfg = MURX_BSSCFG(mu_info, scb->bsscfg);

	if (!mu_bsscfg || !mu_bsscfg->murx_stats) {
		return;
	}

	if (ft != FT_VHT) {
		/* MU-MIMO frames always VHT */
		return;
	}

	plcp = ((uint8 *)h) - D11_PHY_HDR_LEN;
	/* In case of AMPDU, since PLCP is valid only on first MPDU in an AMPDU,
	 * a MU-PLCP marks the start of an MU-AMPDU, and end is determined my
	 * another SU/MU PLCP. We store every received valid PLCP to ensure
	 * appropriate counting of MU and SU MPDUs happen.
	 */
	if (!(plcp[0] | plcp[1] | plcp[2])) {
		plcp = mu_bsscfg->murx_stats->plcp;
	} else {
		memcpy(mu_bsscfg->murx_stats->plcp, plcp, D11_PHY_HDR_LEN);
	}

	/* Update group ID count */
	gid = wlc_vht_get_gid(plcp);
	mu_bsscfg->murx_stats->gid_cnt[gid]++;

	if ((gid >= MU_GROUP_ID_MIN) && (gid <= MU_GROUP_ID_MAX)) {
		is_mu = TRUE;
	}

	/* Update MCS count */
	vht_mcs = wlc_vht_get_rate_from_plcp(plcp);

	ASSERT(vht_mcs & 0x80);
	mcs = vht_mcs & RSPEC_VHT_MCS_MASK;
	nss = (vht_mcs & 0x70) >> RSPEC_VHT_NSS_SHIFT;
	rate_index = mcs + ((nss - 1) * MURX_MCS_NUM);
	if (is_mu) {
		mu_bsscfg->murx_stats->mu_mcs_cnt[rate_index]++;
	} else {
		mu_bsscfg->murx_stats->su_mcs_cnt[rate_index]++;
	}

	mu_bsscfg->murx_stats->gid_last_rate[gid] = rate_index;

#ifdef BCMDBG
	if (is_mu) {
		/* Trace MU frame receipt */
		WL_MUMIMO(("wl%d: Received MU frame from %s with group ID %u, mcs %ux%u.\n",
			mu_info->pub->unit, bcm_ether_ntoa(&scb->ea, eabuf), gid, mcs, nss));
	}
#endif /* BCMDBG */
}

void wlc_murx_clear_dump(wlc_murx_info_t *mu_info)
{
	wlc_bsscfg_t *bsscfg = wlc_murx_get_murx_bsscfg(mu_info);
	murx_bsscfg_t *mu_bsscfg;

	if (!bsscfg)
		return;

	mu_bsscfg = MURX_BSSCFG(mu_info, bsscfg);
	if (!mu_bsscfg || !mu_bsscfg->murx_stats) {
	       return;
	}

	bzero(mu_bsscfg->murx_stats, sizeof(murx_stats_t));

}
#endif /* defined(WLCNT) && (defined(BCMDBG) || defined(WLDUMP) || defined(BCMDBG_MU)) */

#ifdef WLCNT
void wlc_murx_update_murx_inprog(wlc_murx_info_t *mu_info, bool bval)
{
	mu_info->murx_inprog = bval;
}

bool wlc_murx_get_murx_inprog(wlc_murx_info_t *mu_info)
{
	return mu_info->murx_inprog;
}
#endif   /* WLCNT */

#ifdef WL_MODESW
void wlc_murx_sync_oper_mode(wlc_murx_info_t *mu_info, wlc_bsscfg_t *bsscfg, wlc_bss_info_t *bi)
{
	wlc_info_t *wlc = mu_info->wlc;

	if (WLC_MODESW_ENAB(wlc->pub) && !bsscfg->oper_mode_enabled) {
		bsscfg->oper_mode_enabled = TRUE;
		bsscfg->oper_mode = wlc_modesw_derive_opermode(wlc->modesw, bi->chanspec, bsscfg,
			wlc->stf->op_rxstreams);
	}
}
#endif /* WL_MODESW */
#endif   /* WL_MU_RX */
