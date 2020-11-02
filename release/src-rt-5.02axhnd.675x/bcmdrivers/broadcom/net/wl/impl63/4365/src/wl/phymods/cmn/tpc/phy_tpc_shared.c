/*
 * TxPowerCtrl module implementation (shared between PHY type specific implementations).
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

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_cfg.h>
#include "phy_tpc_shared.h"
#include <phy_tpc.h>

#include <wlc_ppr.h>
#include <wlc_phy_shim.h>
#include <wlc_phy_int.h>

/*
 * For SW based power control, target power represents CCK and ucode reduced OFDM by the opo.
 * For hw based power control, we lower the target power so it represents OFDM and
 * ucode boosts CCK by the opo.
 */
void
phy_tpc_upd_shm(phy_info_t *pi)
{
	int j;

	if (ISNPHY(pi) || ISHTPHY(pi) || ISACPHY(pi)) {
		PHY_ERROR(("%s is for legacy phy\n", __FUNCTION__));
		ASSERT(0);
		return;
	}

	if (!pi->sh->clk)
		return;
	if (!pi->tx_power_offset)
		return;

	if (pi->hwpwrctrl) {
		uint16 offset;
		ppr_ofdm_rateset_t ofdm_offsets;
		ppr_get_ofdm(pi->tx_power_offset, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_offsets);

		/* Rate based ucode power control */
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_MAX, 63);
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_N, 1 << NUM_TSSI_FRAMES);

		/* Need to lower the target power to OFDM level, then add boost for CCK. */
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_TARGET,
			wlc_phy_txpower_get_target_min((wlc_phy_t*)pi) << NUM_TSSI_FRAMES);

		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWR_CUR, pi->hwpwr_txcur);

		/* OFDM */
		PHY_TXPWR(("M_RATE_TABLE_A:\n"));
		for (j = 0; j < WL_RATESET_SZ_OFDM; j++) {
			const uint8 ucode_ofdm_rates[] =
			        { /*	   6,   9,    12,   18,   24,   36,   48,   54 Mbps */
					0x0c, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c
				};
			offset = wlapi_bmac_rate_shm_offset(pi->sh->physhim,
				ucode_ofdm_rates[j]);
			wlapi_bmac_write_shm(pi->sh->physhim, offset + 6, ofdm_offsets.pwr[j]);
			PHY_TXPWR(("[0x%x] = 0x%x\n", offset + 6, ofdm_offsets.pwr[j]));
			wlapi_bmac_write_shm(pi->sh->physhim, offset + 14,
				-(ofdm_offsets.pwr[j] / 2));
			PHY_TXPWR(("[0x%x] = 0x%x\n", offset + 14, -(ofdm_offsets.pwr[j] / 2)));
		}
		wlapi_bmac_mhf(pi->sh->physhim, MHF2, MHF2_PPR_HWPWRCTL, MHF2_PPR_HWPWRCTL,
		               WLC_BAND_ALL);
	} else {
		int i;

		/* ucode has 2 db granularity when doing sw pwrctrl,
		 * so round up to next 8 .25 units = 2 db.
		 *   HW based power control has .25 db granularity
		 */
		/* Populate only OFDM power offsets, since ucode can only offset OFDM packets */
		ppr_ofdm_rateset_t ofdm_offsets;

		ppr_get_ofdm(pi->tx_power_offset, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_offsets);
		for (i = 0; i < WL_RATESET_SZ_OFDM; i++)
			ofdm_offsets.pwr[i] = (uint8)ROUNDUP(ofdm_offsets.pwr[i], 8);
		ppr_set_ofdm(pi->tx_power_offset, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&ofdm_offsets);
		wlapi_bmac_write_shm(pi->sh->physhim, M_OFDM_OFFSET,
			(uint16)((ofdm_offsets.pwr[0] + 7) >> 3));
	}
}
