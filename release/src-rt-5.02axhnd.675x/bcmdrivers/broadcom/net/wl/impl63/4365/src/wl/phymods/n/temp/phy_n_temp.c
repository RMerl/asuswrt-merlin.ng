/*
 * NPHY TEMPerature sense module implementation
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_temp.h"
#include "phy_temp_st.h"
#include <phy_n.h>
#include <phy_n_temp.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_n_temp_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_temp_info_t *ti;
};

/* local functions */
static uint8 phy_n_temp_throttle(phy_type_temp_ctx_t *ctx);

/* Register/unregister NPHY specific implementation to common layer */
phy_n_temp_info_t *
BCMATTACHFN(phy_n_temp_register_impl)(phy_info_t *pi, phy_n_info_t *ni, phy_temp_info_t *ti)
{
	phy_n_temp_info_t *info;
	phy_type_temp_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_n_temp_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_temp_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.throt = phy_n_temp_throttle;
	fns.ctx = info;

	phy_temp_register_impl(ti, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_temp_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_temp_unregister_impl)(phy_n_temp_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_temp_info_t *ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_temp_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_n_temp_info_t));
}

/* throttle txcores based temp. */
static uint8
phy_n_temp_throttle(phy_type_temp_ctx_t *ctx)
{
	phy_n_temp_info_t *info = (phy_n_temp_info_t *)ctx;
	phy_temp_info_t *ti = info->ti;
	phy_info_t *pi = info->pi;
	uint8 chainmap;
	phy_txcore_temp_t *temp;

	PHY_TRACE(("%s\n", __FUNCTION__));

	temp = phy_temp_get_st(ti);
	ASSERT(temp != NULL);

	/* XXX Watchdog override is a master switch to kill all PHY related
	 *  periodic activities in the driver
	 */
	if (!pi->phywatchdog_override)
		return temp->bitmap;

	if (NREV_IS(pi->pubpi.phy_rev, 6) ||
	    NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV) ||
	    NREV_IS(pi->pubpi.phy_rev, LCNXN_BASEREV+1) ||
	    NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV+2) ||
	    (NREV_GE(pi->pubpi.phy_rev, 7) &&
	     (((RADIOREV(pi->pubpi.radiorev) == 5) &&
	       ((RADIOVER(pi->pubpi.radiover) == 0x1) ||
	        (RADIOVER(pi->pubpi.radiover) == 0x2))) ||
	      (((pi->sh->chip == BCM43235_CHIP_ID) ||
	        (pi->sh->chip == BCM43236_CHIP_ID) ||
	        (pi->sh->chip == BCM43238_CHIP_ID)) &&
	       (pi->sh->chiprev >= 2)) ||
	      (pi->sh->chip == BCM43237_CHIP_ID)))) {

		/* Degrade-to-single-txchain is enabled for:
		 *     - NPHY rev 6:  all chips
		 *     - NPHY rev 7+: 5357B0 (radio 5v1), 5357B1 (radio5v2), 4323XB0+
		 */

		int16 nphy_currtemp;
		uint8 active_bitmap = temp->bitmap;

		PHY_CAL(("Backoff tempthreshold %d | Restore tempthreshold %d\n",
		temp->disable_temp, temp->enable_temp));

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		nphy_currtemp = wlc_phy_tempsense_nphy(pi);
		wlapi_enable_mac(pi->sh->physhim);

		PHY_CAL(("currtemp %d active_bitmap %x\n",
		nphy_currtemp, active_bitmap));

		if (!temp->heatedup) {
			if (nphy_currtemp >= temp->disable_temp) {
				/* conditioning the RSSI compare chainmap to only Sulley board */
				if (NREV_GE(pi->pubpi.phy_rev, 8) &&
				    (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)) {
					chainmap = phy_rssi_compare_ant(pi->rssii);
					active_bitmap = (active_bitmap & 0xf0) | chainmap;
				} else if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3)) {
					PHY_CAL(("HEADTED UP! : backing off C0\n"));
					active_bitmap &= 0xFE; /* shutoff core 0 */
				} else {
					active_bitmap &= 0xFD; /* shutoff core 1 */
				}
				temp->heatedup = TRUE;
				temp->bitmap = active_bitmap;
			}
		} else {
			if (nphy_currtemp <= temp->enable_temp) {
				if (NREV_GE(pi->pubpi.phy_rev, 8) &&
				    (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID)) {
					active_bitmap |= 0x33;
					temp->degrade1RXen = FALSE;
				} else if (NREV_GE(pi->pubpi.phy_rev, LCNXN_BASEREV + 3)) {
					PHY_CAL(("COOL DOWN! : enabling C0\n"));
					active_bitmap |= 0x1;
				} else {
					active_bitmap |= 0x2;
				}
				temp->heatedup = FALSE;
				temp->bitmap = active_bitmap;
			} else {
				/* Sulley: degrade to 1-RX feature pending on MAC support */
				if (0) {
					if (!temp->degrade1RXen) {
						/* if temperature is still greater  */
						/* than disable temp_thres then degrade 1RX */
						if (nphy_currtemp >= temp->disable_temp) {
							chainmap = phy_rssi_compare_ant(pi->rssii);
							active_bitmap = (chainmap << 4) | chainmap;
							temp->degrade1RXen = TRUE;
							temp->bitmap = active_bitmap;
						}
					}
				}
			}
		}
	}

	return temp->bitmap;
}
