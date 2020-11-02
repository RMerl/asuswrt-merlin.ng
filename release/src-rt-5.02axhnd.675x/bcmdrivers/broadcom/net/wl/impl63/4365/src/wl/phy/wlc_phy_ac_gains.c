/*
 * ACPHY Gain Table loading specific portion of Broadcom BCM43XX 802.11abgn
 * Networking Device Driver.
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
 * $Id: wlc_phy_ac_gains.c 563199 2015-06-12 02:51:32Z $
 */

#include <wlc_cfg.h>

#if (ACCONF != 0) || (ACCONF2 != 0)
#include <typedefs.h>
#include <osl.h>
#include <bcmwifi_channels.h>

#include "wlc_phy_types.h"
#include "wlc_phy_int.h"
#include "wlc_phyreg_ac.h"
#include "wlc_phytbl_ac.h"
#include "wlc_phytbl_20691.h"
#include "wlc_phytbl_ac_gains.h"
#include "wlc_phy_ac_gains.h"

#include <phy_utils_reg.h>

static const uint16 *wlc_phy_get_tiny_txgain_tbl(phy_info_t *pi);

static const uint16 *
wlc_phy_get_tiny_txgain_tbl(phy_info_t *pi)
{
	phy_info_acphy_t *pi_acphy = pi->u.pi_acphy;
	const uint16 *tx_pwrctrl_tbl = NULL;

	ASSERT((RADIOID(pi->pubpi.radioid) == BCM20691_ID) ||
		(RADIOID(pi->pubpi.radioid) == BCM20693_ID));

	tx_pwrctrl_tbl = (CHSPEC_IS2G(pi->radio_chanspec)) ? pi_acphy->gaintbl_2g
						: pi_acphy->gaintbl_5g;

	return tx_pwrctrl_tbl;
}

void
wlc_phy_ac_gains_load(phy_info_t *pi)
{
	uint idx;
	const uint16 *tx_pwrctrl_tbl = NULL;
	uint16 GainWord_Tbl[3];
	uint8 Gainoverwrite = 0;
	uint8 PAgain = 0xff;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	/* Load tx gain table */
	if (RADIOID(pi->pubpi.radioid) == BCM2069_ID)
		tx_pwrctrl_tbl = wlc_phy_get_tx_pwrctrl_tbl_2069(pi);
	else if ((RADIOID(pi->pubpi.radioid) == BCM20691_ID) ||
		(RADIOID(pi->pubpi.radioid) == BCM20693_ID))
		tx_pwrctrl_tbl = wlc_phy_get_tiny_txgain_tbl(pi);
	else {
		PHY_ERROR(("%s: Unsupported Radio!: %d\n", __FUNCTION__,
			RADIOID(pi->pubpi.radioid)));
		ASSERT(0);
		return;
	}

	ASSERT(tx_pwrctrl_tbl != NULL);

	if (PHY_IPA(pi)) {
		Gainoverwrite = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			pi->u.pi_acphy->srom_pagc2g_ovr :
			pi->u.pi_acphy->srom_pagc5g_ovr;
		PAgain = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			pi->u.pi_acphy->srom_pagc2g :
			pi->u.pi_acphy->srom_pagc5g;
	}

	if (Gainoverwrite > 0) {
		for (idx = 0; idx < 128; idx++) {
			GainWord_Tbl[0] = tx_pwrctrl_tbl[3*idx];
			GainWord_Tbl[1] = (tx_pwrctrl_tbl[3*idx+1] & 0xff00) + (0xff & PAgain);

			GainWord_Tbl[2] = tx_pwrctrl_tbl[3*idx+2];
			wlc_phy_tx_gain_table_write_acphy(pi,
				1, idx, 48, GainWord_Tbl);
		}
	} else {
		wlc_phy_tx_gain_table_write_acphy(pi,
			128, 0, 48, tx_pwrctrl_tbl);
	}

	if (TINY_RADIO(pi)) {
		uint16 k, txgaintemp[3];
		uint16 txidxcap = 0;

		if (CHSPEC_IS2G(pi->radio_chanspec) && BF3_2GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
				txidxcap = pi->sromi->txidxcap2g & 0xFF;
		}
		if (CHSPEC_IS5G(pi->radio_chanspec) && BF3_5GTXGAINTBL_BLANK(pi->u.pi_acphy)) {
				txidxcap = pi->sromi->txidxcap5g & 0xFF;
		}
		if (txidxcap != 0) {
			txgaintemp[0] = tx_pwrctrl_tbl[3*txidxcap];
			txgaintemp[1] = tx_pwrctrl_tbl[3*txidxcap+1];
			txgaintemp[2] = tx_pwrctrl_tbl[3*txidxcap+2];

			for (k = 0; k < txidxcap; k++) {
				wlc_phy_tx_gain_table_write_acphy(pi,
				1, k, 48, txgaintemp);
			}
		}
	}
}

void
wlc_phy_tx_gain_table_write_acphy(phy_info_t *pi, uint32 l, uint32 o, uint32 w, const void *d)
{
	uint8 core, tx_gain_tbl_id;
	FOREACH_CORE(pi, core) {
		tx_gain_tbl_id = wlc_phy_get_tbl_id_gainctrlbbmultluts(pi, core);
		if ((!(ACMAJORREV_4(pi->pubpi.phy_rev) ||
		       ACMAJORREV_32(pi->pubpi.phy_rev) ||
		       ACMAJORREV_33(pi->pubpi.phy_rev))) && (core != 0)) {
			continue;
		}
		wlc_phy_table_write_acphy(pi, tx_gain_tbl_id, l, o, w, d);
	}

}

void
BCMATTACHFN(wlc_phy_set_txgain_tbls)(phy_info_t *pi)
{
	const uint16 *tx_pwrctrl_tbl = NULL;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	ASSERT(pi_ac->gaintbl_2g == NULL);

	if ((pi_ac->gaintbl_2g = MALLOC(pi->sh->osh,
		sizeof(uint16) * TXGAIN_TABLES_LEN)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	if (RADIOID(pi->pubpi.radioid) == BCM20691_ID) {
		if (PHY_IPA(pi)) {
			tx_pwrctrl_tbl = (pi->sromi->extpagain2g != 0)
				? txgain_20691_phyrev13_2g_ipa
				: txgain_20691_phyrev13_ipa_2g_for_epa;
		} else {
#ifdef WLC_POINT5_DB_TX_GAIN_STEP /* 0.5 dB gain step */
			tx_pwrctrl_tbl = txgain_20691_phyrev13_2g_point5_epa;
#else
			tx_pwrctrl_tbl = txgain_20691_phyrev13_2g_epa;
#endif /* WLC_POINT5_DB_TX_GAIN_STEP */
		}
	} else if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
		tx_pwrctrl_tbl = (PHY_IPA(pi)) ? txgain_20693_phyrev12_2g_ipa
			: (RADIOREV(pi->pubpi.radiorev) == 32 ||
			   RADIOREV(pi->pubpi.radiorev) == 33) ? txgain_20693_phyrev32_2g_epa
			: (RADIOREV(pi->pubpi.radiorev) == 13) ? txgain_20693_phyrev12_2g_epa_die2
			: txgain_20693_phyrev12_2g_epa;
	} else {
		ASSERT(0);
		return;
	}

	ASSERT(tx_pwrctrl_tbl != NULL);

	/* Copy the table to the new location. */
	bcopy(tx_pwrctrl_tbl, pi_ac->gaintbl_2g, sizeof(uint16) * TXGAIN_TABLES_LEN);

#ifdef BAND5G
	ASSERT(pi_ac->gaintbl_5g == NULL);
	if ((pi_ac->gaintbl_5g = MALLOC(pi->sh->osh,
		sizeof(uint16) * TXGAIN_TABLES_LEN)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	if (RADIOID(pi->pubpi.radioid) == BCM20691_ID) {
		if (pi->ipa5g_on) {
			tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_ipa;
		} else {
#ifdef WLC_POINT5_DB_TX_GAIN_STEP /* 0.5 dB gain step */
			if (pi->txgaintbl5g == 1) {
				tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_1_point5_epa;
			} else {
				tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_point5_epa;
			}
#else
			if (pi->txgaintbl5g == 1) {
				tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_1_epa;
			} else {
				tx_pwrctrl_tbl = txgain_20691_phyrev13_5g_epa;
			}
#endif /* WLC_POINT5_DB_TX_GAIN_STEP */
		}
	} else if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
		tx_pwrctrl_tbl = (PHY_IPA(pi)) ? txgain_20693_phyrev12_5g_ipa
			: (RADIOREV(pi->pubpi.radiorev) == 32 ||
			   RADIOREV(pi->pubpi.radiorev) == 33) ? txgain_20693_phyrev32_5g_epa
			: (RADIOREV(pi->pubpi.radiorev) == 13) ? txgain_20693_phyrev12_5g_epa_die2
			: (RADIOREV(pi->pubpi.radiorev) == 10) ? txgain_20693_phyrev12_5g_epa_rev10
			: txgain_20693_phyrev12_5g_epa;
	} else {
		ASSERT(0);
		return;
	}

	ASSERT(tx_pwrctrl_tbl != NULL);

	/* Copy the table to the new location. */
	bcopy(tx_pwrctrl_tbl, pi_ac->gaintbl_5g, sizeof(uint16) * TXGAIN_TABLES_LEN);
#else
	/* Set the pointer to null if 5G is not defined. */
	pi_ac->gaintbl_5g = NULL;
#endif /* BAND5G */
}

void
BCMATTACHFN(wlc_phy_ac_delete_gain_tbl)(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (pi_ac->gaintbl_2g) {
		MFREE(pi->sh->osh, (void *)pi_ac->gaintbl_2g,
			sizeof(uint16)*TXGAIN_TABLES_LEN);
		pi_ac->gaintbl_2g = NULL;
	}

#ifdef BAND5G
	if (pi_ac->gaintbl_5g) {
		MFREE(pi->sh->osh, (void *)pi_ac->gaintbl_5g,
			sizeof(uint16)*TXGAIN_TABLES_LEN);
		pi_ac->gaintbl_5g = NULL;
	}
#endif // endif
}

#endif /* (ACCONF != 0) || (ACCONF2 != 0) */
