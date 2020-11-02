/*
 * ACPHY TXIQLO CAL module implementation
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_txiqlocal.h>
#include <phy_ac.h>
#include <phy_ac_txiqlocal.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <wlc_phy_radio.h>
#include <phy_ac_info.h>

/* module private states */
struct phy_ac_txiqlocal_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_txiqlocal_info_t *cmn_info;
	acphy_txcal_phyregs_t   *ac_txcal_phyregs_orig;

/* add other variable size variables here at the end */
};

/* local functions */

/* register phy type specific implementation */
phy_ac_txiqlocal_info_t *
BCMATTACHFN(phy_ac_txiqlocal_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_txiqlocal_info_t *cmn_info)
{
	phy_ac_txiqlocal_info_t *ac_info;
	acphy_txcal_phyregs_t *ac_txcal_phyregs_orig = NULL;
	phy_type_txiqlocal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_txiqlocal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	if ((ac_txcal_phyregs_orig = phy_malloc(pi, sizeof(acphy_txcal_phyregs_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc ac_txcal_phyregs_orig failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;
	ac_info->ac_txcal_phyregs_orig = ac_txcal_phyregs_orig;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;

	if (phy_txiqlocal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_txiqlocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:

	if (ac_txcal_phyregs_orig != NULL)
		phy_mfree(pi, ac_txcal_phyregs_orig, sizeof(acphy_txcal_phyregs_t));

	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_txiqlocal_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_txiqlocal_unregister_impl)(phy_ac_txiqlocal_info_t *ac_info)
{
	phy_txiqlocal_info_t *cmn_info;
	phy_info_t *pi;

	ASSERT(ac_info);
	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_txiqlocal_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info->ac_txcal_phyregs_orig, sizeof(acphy_txcal_phyregs_t));

	phy_mfree(pi, ac_info, sizeof(phy_ac_txiqlocal_info_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
#define CAL_TYPE_IQ                 0
#define CAL_TYPE_LOFT_DIG           2
#define CAL_TYPE_LOFT_ANA_FINE      3
#define CAL_TYPE_LOFT_ANA_COARSE    4

#define MAX_PAD_GAIN				0xFF
#define MAX_TX_IDX					127

#define MPHASE_TXCAL_CMDS_PER_PHASE  2 /* number of tx iqlo cal commands per phase in mphase cal */

#define WLC_PHY_PRECAL_TRACE(tx_idx, target_gains) \
	PHY_TRACE(("Index was found to be %d\n", tx_idx)); \
	PHY_TRACE(("Gain Code was found to be : \n")); \
	PHY_TRACE(("radio gain = 0x%x%x%x, bbm=%d, dacgn = %d  \n", \
		target_gains->rad_gain_hi, \
		target_gains->rad_gain_mi, \
		target_gains->rad_gain, \
		target_gains->bbmult, \
		target_gains->dac_gain))

typedef struct {
	uint8 percent;
	uint8 g_env;
} acphy_txiqcal_ladder_t;

typedef struct {
	uint8 nwords;
	uint8 offs;
	uint8 boffs;
} acphy_coeff_access_t;

typedef struct {
	acphy_txgains_t gains;
	bool useindex;
	uint8 index;
} acphy_ipa_txcalgains_t;

#define TXFILT_SHAPING_OFDM20   0
#define TXFILT_SHAPING_OFDM40   1
#define TXFILT_SHAPING_CCK      2
#define TXFILT_DEFAULT_OFDM20   3
#define TXFILT_DEFAULT_OFDM40   4

#define LPFCONF_TXIQ_RX2 0
#define LPFCONF_TXIQ_RX4 1

typedef struct acphy_papd_restore_state_t {
	uint16 fbmix[2];
	uint16 vga_master[2];
	uint16 intpa_master[2];
	uint16 afectrl[2];
	uint16 afeoverride[2];
	uint16 pwrup[2];
	uint16 atten[2];
	uint16 mm;
	uint16 tr2g_config1;
	uint16 tr2g_config1_core[2];
	uint16 tr2g_config4_core[2];
	uint16 reg10;
	uint16 reg20;
	uint16 reg21;
	uint16 reg29;
} acphy_papd_restore_state;

typedef struct _acphy_ipa_txrxgain {
	uint16 hpvga;
	uint16 lpf_biq1;
	uint16 lpf_biq0;
	uint16 lna2;
	uint16 lna1;
	int8 txpwrindex;
} acphy_ipa_txrxgain_t;

/* The following are the txiqcc offsets in the ACPHY_TBL_ID_IQLOCAL table, from acphyprocs.tcl */
static const uint8 tbl_offset_ofdm_a[] = {96, 100, 104, 108};
static const uint8 tbl_offset_bphy_a[] = {112, 116, 120, 124};

/* The following are the txlocc offsets in the ACPHY_TBL_ID_IQLOCAL table, from acphyprocs.tcl */
static const uint8 tbl_offset_ofdm_d[] = {98, 102, 106, 110};
static const uint8 tbl_offset_bphy_d[] = {114, 118, 122, 126};

static void wlc_phy_precal_target_tssi_search(phy_info_t *pi, txgain_setting_t *target_gains);
static void wlc_phy_txcal_radio_setup_acphy_tiny(phy_info_t *pi);
static void wlc_phy_txcal_radio_setup_acphy(phy_info_t *pi);
static void wlc_phy_txcal_phy_setup_acphy(phy_info_t *pi, uint8 Biq2byp);
static void wlc_phy_cal_txiqlo_update_ladder_acphy(phy_info_t *pi, uint16 bbmult, uint8 core);
static uint16 wlc_poll_adc_clamp_status(phy_info_t *pi, uint8 core, uint8 do_reset);
static void wlc_phy_txcal_phy_cleanup_acphy(phy_info_t *pi);
static void wlc_phy_txcal_radio_cleanup_acphy_tiny(phy_info_t *pi);
static void wlc_phy_txcal_radio_cleanup_acphy(phy_info_t *pi);
static void wlc_phy_precal_txgain_control(phy_info_t *pi, txgain_setting_t *target_gains);
static void wlc_phy_txcal_txgain_setup_acphy(phy_info_t *pi, txgain_setting_t *txcal_txgain,
	txgain_setting_t *orig_txgain);
static void wlc_phy_txcal_txgain_cleanup_acphy(phy_info_t *pi, txgain_setting_t *orig_txgain);
static void wlc_phy_txcal_phy_setup_acphy_core(phy_info_t *pi, acphy_txcal_phyregs_t *porig,
	uint8 core, uint16 bw_idx, uint16 sdadc_config, uint8 Biq2byp);
static void wlc_phy_txcal_phy_setup_acphy_core_disable_rf(phy_info_t *pi, uint8 core);
static void wlc_phy_txcal_phy_setup_acphy_core_loopback_path(phy_info_t *pi, uint8 core,
	uint8 lpf_config);
static void wlc_phy_txcal_phy_setup_acphy_core_sd_adc(phy_info_t *pi, uint8 core,
	uint16 sdadc_config);
static void wlc_phy_txcal_phy_setup_acphy_core_lpf(phy_info_t *pi, uint8 core, uint16 bw_idx);
static void wlc_phy_populate_tx_loft_comp_tbl_acphy(phy_info_t *pi, uint16 *loft_coeffs);
static void wlc_phy_poll_adc_acphy(phy_info_t *pi, int32 *adc_buf, uint8 nsamps,
	bool switch_gpiosel, uint16 core, bool is_tssi);
static void wlc_phy_poll_samps_acphy(phy_info_t *pi, int16 *samp, bool is_tssi,
	uint8 log2_nsamps, bool init_adc_inside,
	uint16 core);

static void
wlc_phy_precal_target_tssi_search(phy_info_t *pi, txgain_setting_t *target_gains)
{
	int8  gain_code_found, delta_threshold, dont_alter_step;
	int16  target_tssi, min_delta, prev_delta, delta_tssi;
	int16  idle_tssi[PHY_CORE_MAX] = {0};
	uint8  tx_idx;
	int16  tone_tssi[PHY_CORE_MAX] = {0};
	int16  tssi[PHY_CORE_MAX] = {0};

	int16  pad_gain_step, curr_pad_gain, pad_gain;

	txgain_setting_t orig_txgain[PHY_CORE_MAX];
	int16  sat_count, sat_threshold, sat_delta, ct;
	int16 temp_val;
	int16 tx_idx_step, pad_step_size, pad_iteration_count;

	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	/* Set the target TSSIs for different bands/Bandwidth cases.
	 * These numbers are arrived by running the TCL proc:
	 * "get_target_tssi_for_iqlocal" for a representative channel
	 * by sending a tone at a chosen Tx gain which gives best
	 * Image/LO rejection at room temp
	 */

	if (CHSPEC_IS5G(pi->radio_chanspec) == 1) {
		if (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
			target_tssi = 900;
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
			target_tssi = 900;
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			target_tssi = 900;
		} else {
			target_tssi = 950;
		}
	} else {
		if (CHSPEC_IS40(pi->radio_chanspec)) {
			target_tssi = 913;
		} else {
			target_tssi = 950;
		}

	}

	phy_ac_tssi_loopback_path_setup(pi, LOOPBACK_FOR_TSSICAL);

	gain_code_found = 0;

	/* delta_threshold is the minimum tolerable difference between
	 * target tssi and the measured tssi. This was determined by experimental
	 * observations. delta_tssi ( target_tssi - measured_tssi ) values upto
	 * 15 are found to give identical performance in terms of Tx EVM floor
	 * when compared to delta_tssi values upto 10. Threshold value of 15 instead
	 * of 10 will cut down the algo time as the algo need not search for
	 * index to meet delta of 10.
	 */
	delta_threshold = 15;

	min_delta = 1024;
	prev_delta = 1024;

	/* Measure the Idle TSSI */
	wlc_phy_poll_samps_WAR_acphy(pi, idle_tssi, TRUE, TRUE, target_gains, FALSE, TRUE, 0, 0);

	/* Measure the tone TSSI before start searching */
	tx_idx = 0;
	wlc_phy_txpwr_by_index_acphy(pi, 1, tx_idx);

	wlc_phy_get_txgain_settings_by_index_acphy(
				pi, target_gains, tx_idx);

	/* Save the original Gain code */
	wlc_phy_txcal_txgain_setup_acphy(pi, target_gains, &orig_txgain[0]);

	PHY_TRACE(("radio gain = 0x%x%x%x, bbm=%d, dacgn = %d  \n",
		target_gains->rad_gain_hi,
		target_gains->rad_gain_mi,
		target_gains->rad_gain,
		target_gains->bbmult,
		target_gains->dac_gain));

	wlc_phy_poll_samps_WAR_acphy(pi, tssi, TRUE, FALSE, target_gains, FALSE, TRUE, 0, 0);

	tone_tssi[0] = tssi[0] - idle_tssi[0];

	delta_tssi = target_tssi - tone_tssi[0];

	PHY_TRACE(("Index = %3d target_TSSI = %4i tone_TSSI = %4i"
			"delta_TSSI = %4i min_delta = %4i\n",
			tx_idx, target_tssi, tone_tssi[0], delta_tssi, min_delta));

	PHY_TRACE(("*********** Search Control loop begins now ***********\n"));

	/* When the measured tssi saturates and is unable to meet
	 * the target tssi, there is no point in continuing search
	 * for the next higher PAD gain. The variable 'sat_count'
	 * is the threshold which will control when to stop the search.
	 * change in PAD gain code by "10" ticks should atleast translate
	 * to 1dBm of power level change when not saturated. When the
	 * measured tssi is saturated, this doesnt hold good and we
	 * need to break out.
	 */
	sat_count = 10;
	sat_threshold = 20;

	/* delta_tssi > 0 ==> target_tssi is greater than tone tssi and
	 * hence we have to increase the PAD gain as the inference was
	 * drawn by measuring the tone tssi at index 0
	 */

	if (delta_tssi > 0) {
		PHY_TRACE(("delta_tssi > 0 ==> target_tssi is greater than tone tssi and\n"));
		PHY_TRACE(("hence we have to increase the PAD gain as the inference was\n"));
		PHY_TRACE(("drawn by measuring the tone tssi at index 0\n"));

		tx_idx = 0;
		wlc_phy_txpwr_by_index_acphy(pi, 1, tx_idx);

		wlc_phy_get_txgain_settings_by_index_acphy(
				pi, target_gains, tx_idx);

		min_delta = 1024;
		prev_delta = 1024;

		sat_delta = 0;
		ct = 0;

		curr_pad_gain = target_gains->rad_gain_mi & 0x00ff;

		PHY_TRACE(("Current PAD Gain (Before Search) is %d\n", curr_pad_gain));
		pad_gain_step = 1;

		for (pad_gain = curr_pad_gain; pad_gain <= MAX_PAD_GAIN; pad_gain += pad_gain_step)
		{
			target_gains->rad_gain_mi = target_gains->rad_gain_mi & 0xff00;
			target_gains->rad_gain_mi |= pad_gain;

			PHY_TRACE(("Current PAD Gain is %d\n", pad_gain));

			wlc_phy_poll_samps_WAR_acphy(pi, tssi, TRUE, FALSE,
			                             target_gains, FALSE, TRUE, 0, 0);
			tone_tssi[0] = tssi[0] - idle_tssi[0];

			delta_tssi = target_tssi - tone_tssi[0];

			/* Manipulate the step size to cut down the search time */
			if (delta_tssi > 50) {
				pad_gain_step = 10;
			} else if (delta_tssi > 30) {
				pad_gain_step = 5;
			} else if (delta_tssi > 15) {
				pad_gain_step = 2;
			} else {
				pad_gain_step = 1;
			}

			/* Check for TSSI Saturation */
			if (ct == 0) {
				sat_delta = delta_tssi;
			} else {
				sat_delta = delta_tssi - prev_delta;
				sat_delta = ABS(sat_delta);

				PHY_TRACE(("Ct=%d sat_delta=%d delta_tssi=%d sat_delta=%d\n",
					ct, sat_delta, delta_tssi, sat_delta));
			}

			if (sat_delta > sat_threshold) {
				ct = 0;
			}

			if ((ct == sat_count) && (sat_delta < sat_threshold)) {

				PHY_TRACE(("Ct = %d\t sat_delta = %d \t "
						"sat_threshold = %d\n",
						ct, sat_delta, sat_threshold));

				gain_code_found = 0;

				PHY_TRACE(("Breaking out of search as TSSI "
						" seems to have saturated\n"));
				WLC_PHY_PRECAL_TRACE(tx_idx, target_gains);

				break;
			}

			ct = ct + 1;

			PHY_TRACE(("Index = %3d target_TSSI = %4i tone_TSSI = %4i"
					"delta_TSSI = %4i min_delta = %4i radio gain = 0x%x%x%x, "
					"bbm=%d, dacgn = %d\n", tx_idx,
					target_tssi, tone_tssi[0], delta_tssi, min_delta,
					target_gains->rad_gain_hi, target_gains->rad_gain_mi,
					target_gains->rad_gain, target_gains->bbmult,
					target_gains->dac_gain));

			temp_val = ABS(delta_tssi);
			if (temp_val <= min_delta) {
				min_delta = ABS(delta_tssi);

				if (min_delta <= delta_threshold) {
					gain_code_found	= 1;

					PHY_TRACE(("Breaking out of search as min delta"
							" tssi threshold conditions are met\n"));

					WLC_PHY_PRECAL_TRACE(tx_idx, target_gains);

					break;
				}
			}
			prev_delta = delta_tssi;
		}

		if (gain_code_found == 0) {
			PHY_TRACE(("*** Search failed Again ***\n"));
		}

	/* delta_tssi < 0 ==> target tssi is less than tone tssi and we have to reduce the gain */
	} else {

		PHY_TRACE(("delta_tssi < 0 ==> target tssi is less than"
				"tone tssi and we have to reduce the gain\n"));

		tx_idx_step = 1;
		dont_alter_step = 0;
		pad_step_size = 0;
		pad_iteration_count = 0;

		sat_delta = 0;
		ct = 0;

		for (tx_idx = 0; tx_idx <= MAX_TX_IDX; tx_idx +=  tx_idx_step) {
			wlc_phy_txpwr_by_index_acphy(pi, 1, tx_idx);

			wlc_phy_get_txgain_settings_by_index_acphy(
					pi, &(target_gains[0]), tx_idx);

			if (pad_step_size != 0) {
				curr_pad_gain = target_gains->rad_gain_mi & 0x00ff;
				curr_pad_gain = curr_pad_gain -
				(pad_iteration_count * pad_step_size);

				target_gains->rad_gain_mi =
				target_gains->rad_gain_mi & 0xff00;

				target_gains->rad_gain_mi |= curr_pad_gain;
			}

			wlc_phy_poll_samps_WAR_acphy(pi, tssi, TRUE,
			                             FALSE, &(target_gains[0]), FALSE, TRUE, 0, 0);
			tone_tssi[0] = tssi[0] - idle_tssi[0];

			delta_tssi = target_tssi - tone_tssi[0];

			PHY_TRACE(("Index = %3d target_TSSI = %4i "
					"tone_TSSI = %4i delta_TSSI = %4i min_delta = %4i "
					"radio gain = 0x%x%x%x, bbm=%d, dacgn = %d\n", tx_idx,
					target_tssi, tone_tssi[0], delta_tssi, min_delta,
					target_gains->rad_gain_hi, target_gains->rad_gain_mi,
					target_gains->rad_gain,
					target_gains->bbmult, target_gains->dac_gain));

			/* Check for TSSI Saturation */
			if (ct == 0) {
				sat_delta = delta_tssi;
			} else {
				sat_delta = delta_tssi - prev_delta;
				sat_delta = ABS(sat_delta);

				PHY_TRACE(("Ct=%d sat_delta=%d delta_tssi=%d sat_delta=%d\n",
					ct, sat_delta, delta_tssi, sat_delta));
			}

			if (sat_delta > sat_threshold) {
				ct = 0;
			}

			if ((ct == sat_count) && (sat_delta < sat_threshold) &&
				(ABS(delta_tssi) < sat_threshold)) {

				PHY_TRACE(("Ct = %d\t sat_delta = %d \t sat_threshold = %d\n",
					ct, sat_delta, sat_threshold));

				gain_code_found	= 0;

				PHY_TRACE(("Breaking out of search as TSSI "
						" seems to have saturated\n"));

				WLC_PHY_PRECAL_TRACE(tx_idx, target_gains);

				break;
			}

			ct = ct + 1;

			temp_val = ABS(delta_tssi);
			if (temp_val <= min_delta) {
				min_delta = ABS(delta_tssi);

				if (min_delta <= delta_threshold) {
					gain_code_found	= 1;

					PHY_TRACE(("Breaking out of search "
							"as min delta tssi threshold "
							"conditions are met\n"));

					WLC_PHY_PRECAL_TRACE(tx_idx, target_gains);

					PHY_TRACE(("===== IQLOCAL PreCalGainControl: END =====\n"));
					break;
				}
			}

			/* Change of sign in delta tssi => increase
			 * the step size with smaller resolution
			 */

			if ((prev_delta < 0) && (delta_tssi > 0)&& (tx_idx != 0))
			{
				PHY_TRACE(("Scenario 2 -- BELOW TARGET\n"));
				/* Now that tx idx is sufficiently dropped ,
				 * there is change in sign of the delta tssi.
				 * implies, now target tssi is more than tone tssi.
				 * So increase the gain in very small steps
				 * by decrementing the index
				 */
				tx_idx_step = -1;
				dont_alter_step = 1;
			} else if ((prev_delta < 0) && (delta_tssi < 0) && (dont_alter_step == 1)) {
				PHY_TRACE(("Scenario 3 --  OSCILLATORY\n"));

				/* this case is to take care of the oscillatory
				 * behaviour of the tone tssi about the target
				 * tssi. Here tone tssi has again
				 * overshot the target tssi. So donot change the
				 * tx gain index, but reduce the PAD gain
				 */
				tx_idx_step = 0;
				pad_step_size = 1;
				pad_iteration_count += 1;

			} else {
				PHY_TRACE(("Scenario 1 -- NORMAL\n"));
				/* tone tssi is more than target tssi.
				 * So increase the index and hence reduce the gain
				 */
				if (dont_alter_step == 0) {
					/* Manipulate the step size to cut down the search time */
					if (delta_tssi >= 50) {
						tx_idx_step = 5;
					} else if (delta_tssi >= 25) {
						tx_idx_step = 3;
					} else if (delta_tssi >= 10) {
						tx_idx_step = 2;
					} else {
						tx_idx_step = 1;
					}
				}
			}
			prev_delta = delta_tssi;
		}

	}

	/* Search found the right gain code meeting required tssi conditions */
	if (gain_code_found == 1) {

		PHY_TRACE(("******* SUMMARY *******\n"));
		WLC_PHY_PRECAL_TRACE(tx_idx, target_gains);

		PHY_TRACE(("Measured TSSI Value is %d\n", tone_tssi[0]));
		PHY_TRACE(("***********************\n"));
	}
	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
	PHY_TRACE(("======= IQLOCAL PreCalGainControl : END =======\n"));

	/* Restore the original Gain code */
	wlc_phy_txcal_txgain_cleanup_acphy(pi, &orig_txgain[0]);

	return;
}

extern void
wlc_phy_tx_fdiqi_comp_acphy(phy_info_t *pi, bool enable, int fdiq_data_valid)
{
	uint8 core;
	int8 sign_slope;
	int8 idx;
	int8 slope;

	int16 filtercoeff[11][7] = {
		{	 0,    	0,	   0,  1023,   	0,     0,    0},
		{  -10,    16,   -31,  1023,   32,   -16,    10},
		{  -21,    31,   -62,  1023,   63,   -32,    21},
		{  -31,    46,   -92,  1023,   96,   -47,    31},
		{  -41,    62,  -121,  1022,  129,   -63,    42},
		{  -51,    77,  -150,  1022,  162,   -80,    53},
		{  -61,    91,  -179,  1020,  196,   -96,    63},
		{  -71,   106,  -207,  1019,  230,  -112,    74},
		{  -81,   121,  -234,  1018,  265,  -128,    85},
		{  -91,   135,  -261,  1016,  300,  -145,    95},
		{ -101,   149,  -278,  1014,  335,  -161,    106}
	};

	/* enable: 0 - disable FDIQI comp
	 *         1 - program FDIQI comp filter and enable
	 */

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

#ifdef WL_PROXDETECT
	if (pi->u.pi_acphy->tof_active) {
		return;
	}
#endif // endif

	/* write values */
	if (enable == FALSE) {
		MOD_PHYREG(pi, fdiqImbCompEnable, txfdiqImbCompEnable, 0);
		return;
	} else {

#define ACPHY_TXFDIQCOMP_STR(pi, core, tap)	((ACPHY_txfdiqcomp_str0_c0(pi->pubpi.phy_rev) +	\
	(0x200 * (core)) + (tap)))

		FOREACH_CORE(pi, core) {
		  if (((fdiq_data_valid & (1 << core)) >> core) == 1) {
		    slope = pi->u.pi_acphy->txfdiqi->slope[core];
		    sign_slope = slope >= 0 ? 1 : -1;
		    slope *= sign_slope;
		    if (slope > 10) slope = 10;

		    MOD_PHYREG(pi, txfdiqImbN_offcenter_scale_str, N_offcenter_scale, 2);
		    MOD_PHYREG(pi, fdiqImbCompEnable, txfdiq_iorq, 0);
		    if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
		        CHSPEC_IS80(pi->radio_chanspec))  {
			    MOD_PHYREG(pi, fdiqImbCompEnable, txfdiq_iorq, 1);
		    }

		    MOD_PHYREG(pi, fdiqi_tx_comp_Nshift_out, Nshift_out, 10);
		    for (idx = 0; idx < 7; idx++) {
			    if (sign_slope == -1) {
				    phy_utils_write_phyreg(pi, ACPHY_TXFDIQCOMP_STR
					(pi, core, idx), filtercoeff[slope][6-idx]);
				} else {
					phy_utils_write_phyreg(pi, ACPHY_TXFDIQCOMP_STR
					(pi, core, idx), filtercoeff[slope][idx]);
				}
			}

		  }
		}
		MOD_PHYREG(pi, fdiqImbCompEnable, txfdiqImbCompEnable, 1);
	}

}
void
wlc_phy_fdiqi_lin_reg_acphy(phy_info_t *pi, acphy_fdiqi_t *freq_ang_mag,
                            uint16 num_data, int fdiq_data_valid)
{
	int32 Sf2 = 0;
	int32 Sfa[PHY_CORE_MAX], Sa[PHY_CORE_MAX], Sm[PHY_CORE_MAX];
	int32 intcp[PHY_CORE_MAX], mag[PHY_CORE_MAX];
	int32 refBW;
	int8 idx;
	uint8 core;
	int32 sin_angle, cos_angle;
	math_cint32 cordic_out;
	int32  a, b, sign_sa;
	uint16 coeffs[2];

	/* initialize array for all cores to prevent compile warning (UNINIT) */
	FOREACH_CORE(pi, core) {
		Sfa[core] = 0; Sa[core] = 0; Sm[core] = 0;
	}

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		Sf2 = 0;
		if (((fdiq_data_valid & (1 << core)) >> core) == 1) {
			for (idx = 0; idx < num_data; idx++) {
				Sf2 += freq_ang_mag[idx].freq * freq_ang_mag[idx].freq;
				Sfa[core] += freq_ang_mag[idx].freq * freq_ang_mag[idx].angle[core];
				Sa[core] += freq_ang_mag[idx].angle[core];
				Sm[core] += freq_ang_mag[idx].mag[core];
			}

			sign_sa = Sa[core] >= 0 ? 1 : -1;
			intcp[core] = (Sa[core] + sign_sa * (num_data >> 1)) / num_data;
			mag[core]   = (Sm[core] + (num_data >> 1)) / num_data;
			phy_utils_cordic(intcp[core], &cordic_out);
			sin_angle = cordic_out.q;
			cos_angle = cordic_out.i;
			b = mag[core] * cos_angle;
			a = mag[core] * sin_angle;

			b = ((b >> 15) + 1) >> 1;
			b -= (1 << 10);  /* 10 bit */
			a = ((a >> 15) + 1) >> 1;

			a = (a < -512) ? -512 : ((a > 511) ? 511 : a);
			b = (b < -512) ? -512 : ((b > 511) ? 511 : b);

			coeffs[0] = (uint16)(a);
			coeffs[1] = (uint16)(b);
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
			                                coeffs, TB_OFDM_COEFFS_AB, core);
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
			                                coeffs, TB_BPHY_COEFFS_AB, core);
			refBW = 30;
			pi->u.pi_acphy->txfdiqi->slope[core] =
			        (((-Sfa[core] * refBW / Sf2) >> 13) + 1 ) >> 1;
		}
	}
	wlc_phy_tx_fdiqi_comp_acphy(pi, TRUE, fdiq_data_valid);
}

static void
wlc_phy_txcal_radio_setup_acphy_tiny(phy_info_t *pi)
{
	/* This stores off and sets Radio-Registers for Tx-iqlo-Calibration;
	 *
	 * Note that Radio Behavior controlled via RFCtrl is handled in the
	 * phy_setup routine, not here; also note that we use the "shotgun"
	 * approach here ("coreAll" suffix to write to all jtag cores at the
	 * same time)
	 */
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_radioregs_t *porig = (pi_ac->ac_txcal_radioregs_orig);
	uint8 core;

	ASSERT(TINY_RADIO(pi));

	/* SETUP: set 2059 into iq/lo cal state while saving off orig state */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* save off orig */
		porig->iqcal_cfg1[core] = READ_RADIO_REG_TINY(pi, IQCAL_CFG1, core);
		porig->auxpga_cfg1[core] = READ_RADIO_REG_TINY(pi, AUXPGA_CFG1, core);
		porig->iqcal_cfg3[core] = READ_RADIO_REG_TINY(pi, IQCAL_CFG3, core);
		porig->tx_top_5g_ovr1[core] = READ_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core);
		porig->adc_cfg10[core] = READ_RADIO_REG_TINY(pi, ADC_CFG10, core);
		porig->auxpga_ovr1[core] = READ_RADIO_REG_TINY(pi, AUXPGA_OVR1, core);
		porig->testbuf_ovr1[core] = READ_RADIO_REG_TINY(pi, TESTBUF_OVR1, core);
		porig->adc_ovr1[core] = READ_RADIO_REG_TINY(pi, ADC_OVR1, core);
		porig->tx5g_misc_cfg1[core] = READ_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core);
		porig->testbuf_cfg1[core] = READ_RADIO_REG_TINY(pi, TESTBUF_CFG1, core);
		porig->tia_cfg5[core] = READ_RADIO_REG_TINY(pi, TIA_CFG5, core);
		porig->tia_cfg9[core] = READ_RADIO_REG_TINY(pi, TIA_CFG9, core);
		porig->auxpga_vmid[core] = READ_RADIO_REG_TINY(pi, AUXPGA_VMID, core);
		porig->pmu_cfg4[core] = READ_RADIO_REG_TINY(pi, PMU_CFG4, core);

		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
			porig->tx_top_2g_ovr_north[core] = READ_RADIO_REG_20693(pi,
				TX_TOP_2G_OVR_NORTH, core);
			porig->tx_top_2g_ovr_east[core] = READ_RADIO_REG_20693(pi,
				TX_TOP_2G_OVR_EAST, core);
			porig->iqcal_cfg2[core] = READ_RADIO_REG_20693(pi, IQCAL_CFG2, core);
			porig->pmu_ovr[core] = READ_RADIO_REG_20693(pi, PMU_OVR, core);
			porig->tx2g_misc_cfg1[core] = READ_RADIO_REG_20693(pi,
					TX2G_MISC_CFG1, core);
			porig->adc_cfg18[core] = READ_RADIO_REG_20693(pi, ADC_CFG18, core);
			porig->tx_top_5g_ovr2[core] = READ_RADIO_REG_20693(pi,
					TX_TOP_5G_OVR2, core);
			porig->txmix5g_cfg2[core] = READ_RADIO_REG_20693(pi, TXMIX5G_CFG2, core);
			porig->pad5g_cfg1[core] = READ_RADIO_REG_20693(pi, PAD5G_CFG1, core);
			porig->pa5g_cfg1[core] = READ_RADIO_REG_20693(pi, PA5G_CFG1, core);
		} else {
			porig->iqcal_ovr1[core] = READ_RADIO_REG_TINY(pi, IQCAL_OVR1, core);
			porig->pa2g_cfg1[core] = READ_RADIO_REG_TINY(pi, PA2G_CFG1, core);
		}

		/* # Enabling and Muxing per band */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x8);
			if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
				MOD_RADIO_REG_20693(pi, TX2G_MISC_CFG1, core,
						pa2g_tssi_ctrl_sel, 0);
				MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR_NORTH, core,
						ovr_pa2g_tssi_ctrl_sel, 0x1);
				MOD_RADIO_REG_20693(pi, TX2G_MISC_CFG1, core,
						pa2g_tssi_ctrl, 0x5);
				MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR_EAST, core,
						ovr_pa2g_tssi_ctrl_range, 0x1);
				MOD_RADIO_REG_20693(pi, TX2G_MISC_CFG1, core,
						pa2g_tssi_ctrl_range, 1);
			} else
				MOD_RADIO_REG_TINY(pi, PA2G_CFG1, core, pa2g_tssi_ctrl_sel, 0);
		} else {
		        MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x0a);
				if (RADIOID(pi->pubpi.radioid) == BCM20693_ID &&
						RADIOMAJORREV(pi) == 3) {
					MOD_RADIO_REG_20693(pi, TX5G_MISC_CFG1, core,
							pa5g_tssi_ctrl_sel, 0);
					MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core,
							ovr_pa5g_tssi_ctrl_sel, 0x1);
					MOD_RADIO_REG_20693(pi, TX5G_MISC_CFG1, core,
						pa5g_tssi_ctrl, 0x8);
					MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core,
						ovr_pa5g_tssi_ctrl_range, 0x1);
					MOD_RADIO_REG_20693(pi, TX5G_MISC_CFG1, core,
						pa5g_tssi_ctrl_range, 0x0);
				} else {
					/* #tap from PA output */
					MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core,
							pa5g_tssi_ctrl_sel, 0);
					MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core,
							ovr_pa5g_tssi_ctrl_sel, 0x1);
				}
		}

		MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_tssi_GPIO_ctrl, 0x0);

		if (!(RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3)) {
			MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_iqcal, 1);
			MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 1);
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 0x0);
			/* # power off tssi */
		}
		/* # power up iqlocal */
		MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x1);

		/* ### JV: May be there is a  direct control for this, could'nt find it */
		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 0x1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_pu, 0x1); /* # power up auxpga */

		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_input, 0x1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x0);
		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
		        MOD_RADIO_REG_TINY(pi, ADC_CFG10, core, adc_in_test, 0xF);
		}
		MOD_RADIO_REG_TINY(pi, TESTBUF_OVR1, core, ovr_testbuf_sel_test_port, 1);
		MOD_RADIO_REG_TINY(pi, TESTBUF_CFG1, core, testbuf_sel_test_port, 0);
		MOD_RADIO_REG_TINY(pi, TESTBUF_OVR1, core, ovr_testbuf_PU, 0x1);
		MOD_RADIO_REG_TINY(pi, TESTBUF_CFG1, core, testbuf_PU, 0x1);

		/* #pwr up adc and set auxpga gain */
		MOD_RADIO_REG_TINY(pi, PMU_CFG4, core, wlpmu_ADCldo_pu, 1);
		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
			MOD_RADIO_REG_20693(pi, PMU_OVR, core, ovr_wlpmu_ADCldo_pu, 1);
			MOD_RADIO_REG_20693(pi, IQCAL_CFG2, core, iqcal_iq_cm_center, 0x8);
		}

		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_gain, 1);
		MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_sel_gain, 0x3);
		MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_vmid, 1);
		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
			MOD_RADIO_REG_TINY(pi, AUXPGA_VMID, 0, auxpga_i_sel_vmid, 0x97);
			MOD_RADIO_REG_TINY(pi, AUXPGA_VMID, 1, auxpga_i_sel_vmid, 0x99);
			MOD_RADIO_REG_TINY(pi, AUXPGA_VMID, 2, auxpga_i_sel_vmid, 0x98);
			MOD_RADIO_REG_TINY(pi, AUXPGA_VMID, 3, auxpga_i_sel_vmid, 0x95);
		} else {
			MOD_RADIO_REG_TINY(pi, AUXPGA_VMID, core, auxpga_i_sel_vmid, 0x9c);
		}

		/* #ensure that the dac mux is OFF because it shares a line with auxpga o/p */
		MOD_RADIO_REG_TINY(pi, TIA_CFG9, core, txbb_dac2adc, 0x0);
	        /* #these two lines disable the TIA gpaio mux and enable the */
		MOD_RADIO_REG_TINY(pi, TIA_CFG5, core, tia_out_test, 0x0);
		if ((RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3)) {
			MOD_RADIO_REG_20693(pi, ADC_OVR1, core, ovr_adc_in_test, 0x0);
			if (CHSPEC_IS80(pi->radio_chanspec) ||
					PHY_AS_80P80(pi, pi->radio_chanspec)) {
				MOD_RADIO_REG_20693(pi, ADC_OVR1, core, ovr_adc_od_pu, 0x1);
				MOD_RADIO_REG_20693(pi, ADC_CFG18, core, adc_od_pu, 0x1);
			} else if (CHSPEC_IS160(pi->radio_chanspec)) {
				ASSERT(0); // FIXME
			} else {
				MOD_RADIO_REG_20693(pi, ADC_OVR1, core, ovr_adc_od_pu, 0x1);
				MOD_RADIO_REG_20693(pi, ADC_CFG18, core, adc_od_pu, 0x0);
			}
		} else {
			MOD_RADIO_REG_TINY(pi, ADC_CFG10, core, adc_in_test, 0x3);
		}

	}
}

static void
wlc_phy_txcal_radio_setup_acphy(phy_info_t *pi)
{
	/* This stores off and sets Radio-Registers for Tx-iqlo-Calibration;
	 *
	 * Note that Radio Behavior controlled via RFCtrl is handled in the
	 * phy_setup routine, not here; also note that we use the "shotgun"
	 * approach here ("coreAll" suffix to write to all jtag cores at the
	 * same time)
	 */

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_radioregs_t *porig = (pi_ac->ac_txcal_radioregs_orig);
	uint8 core;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	/* SETUP: set 2059 into iq/lo cal state while saving off orig state */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* save off orig */
		porig->iqcal_cfg1[core]  = READ_RADIO_REGC(pi, RF, IQCAL_CFG1, core);
		porig->iqcal_cfg2[core]  = READ_RADIO_REGC(pi, RF, IQCAL_CFG2, core);
		porig->iqcal_cfg3[core]  = READ_RADIO_REGC(pi, RF, IQCAL_CFG3, core);
		porig->pa2g_tssi[core]   = READ_RADIO_REGC(pi, RF, PA2G_TSSI, core);
		porig->tx5g_tssi[core]   = READ_RADIO_REGC(pi, RF, TX5G_TSSI, core);
		porig->auxpga_cfg1[core] = READ_RADIO_REGC(pi, RF, AUXPGA_CFG1, core);

		/* Reg conflict with 2069 rev 16 */
		if (RADIOMAJORREV(pi) == 0)
			porig->OVR20[core] = READ_RADIO_REGC(pi, RF, OVR20, core);
		else
			porig->OVR21[core] = READ_RADIO_REGC(pi, RF, GE16_OVR21, core);

		/* now write desired values */

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0xb);
			MOD_RADIO_REGC(pi, TX5G_TSSI, core, pa5g_ctrl_tssi_sel, 0x1);

			/* Reg conflict with 2069 rev 16 */
			if (RADIOMAJORREV(pi) == 0) {
				MOD_RADIO_REGC(pi, OVR20, core, ovr_pa5g_ctrl_tssi_sel, 0x1);
				MOD_RADIO_REGC(pi, OVR20, core, ovr_pa2g_ctrl_tssi_sel, 0x0);
			} else {
				MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa5g_ctrl_tssi_sel, 0x1);
				MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa2g_ctrl_tssi_sel, 0x0);
			}
			MOD_RADIO_REGC(pi, PA2G_TSSI, core, pa2g_ctrl_tssi_sel, 0x0);
		} else {
			MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x8);
			MOD_RADIO_REGC(pi, TX5G_TSSI, core, pa5g_ctrl_tssi_sel, 0x0);
			/* Reg conflict with 2069 rev 16 */
			if (RADIOMAJORREV(pi) == 0) {
				MOD_RADIO_REGC(pi, OVR20, core, ovr_pa5g_ctrl_tssi_sel, 0x0);
				MOD_RADIO_REGC(pi, OVR20, core, ovr_pa2g_ctrl_tssi_sel, 0x1);
			} else {
				MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa5g_ctrl_tssi_sel, 0x0);
				MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa2g_ctrl_tssi_sel, 0x1);
			}
			MOD_RADIO_REGC(pi, PA2G_TSSI, core, pa2g_ctrl_tssi_sel, 0x1);
		}
		MOD_RADIO_REGC(pi, IQCAL_CFG1, core, tssi_GPIO_ctrl, 0x0);

		if (RADIOMAJORREV(pi) == 1) {
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_vcm_ctrl, 0x0);
			/* This bit is supposed to be controlled by phy direct control line.
			 * Please check: http://jira.broadcom.com/browse/HW11ACRADIO-45
			 */
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x0);
		}
	} /* for core */
}

static void
wlc_phy_txcal_phy_setup_acphy(phy_info_t *pi, uint8 Biq2byp)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_phyregs_t *porig = (pi_ac->txiqlocali->ac_txcal_phyregs_orig);
	uint16 sdadc_config;
	uint8  core, bw_idx;
	porig->RxFeCtrl1 = READ_PHYREG(pi, RxFeCtrl1);
	porig->AfePuCtrl = READ_PHYREG(pi, AfePuCtrl);

	if (CHSPEC_IS80(pi->radio_chanspec) || PHY_AS_80P80(pi, pi->radio_chanspec)) {
		bw_idx = 2;
		sdadc_config = sdadc_cfg80;
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		bw_idx = 3;
		sdadc_config = sdadc_cfg80;
		ASSERT(0); // FIXME
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		bw_idx = 1;
		if (pi->sdadc_config_override)
			sdadc_config = sdadc_cfg40hs;
		else
			sdadc_config = sdadc_cfg40;
	} else {
		bw_idx = 0;
		sdadc_config = sdadc_cfg20;
	}

	if (ACMAJORREV_1(pi->pubpi.phy_rev) && (ACMINORREV_0(pi) || ACMINORREV_1(pi))) {
		MOD_PHYREG(pi, RxFeCtrl1, rxfe_bilge_cnt, 4);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);
	}

	if (TINY_RADIO(pi)) {
		/* #set sd adc full scale */
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 1);
		MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 2);
		MOD_PHYREG(pi, TSSIMode, tssiADCSel, 1);
	}

	/* turn off tssi sleep feature during cal */
	MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);

	/*  SETUP: save off orig reg values and configure for cal  */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		wlc_phy_txcal_phy_setup_acphy_core(pi, porig, core, bw_idx, sdadc_config, Biq2byp);
	} /* for core */

	MOD_PHYREG(pi, RxFeCtrl1, swap_iq0, 1);
	MOD_PHYREG(pi, RxFeCtrl1, swap_iq1, 1);
	MOD_PHYREG(pi, RxFeCtrl1, swap_iq2, 1);
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq3, 1);
	}

	/* ADC pulse clamp en fix */
	wlc_phy_pulse_adc_reset_acphy(pi);

	/* we should not need spur avoidance anymore
	porig->BBConfig = READ_PHYREG(pi, BBConfig);
	MOD_PHYREG(pi, BBConfig, resample_clk160, 0);
	*/
}

static void
wlc_phy_cal_txiqlo_update_ladder_acphy(phy_info_t *pi, uint16 bbmult, uint8 core)
{
	uint8  indx;
	uint32 bbmult_scaled;
	uint16 tblentry;
	uint8 stall_val;
	uint8 iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, core);

	acphy_txiqcal_ladder_t ladder_lo[] = {
	{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
	{25, 0}, {25, 1}, {25, 2}, {25, 3}, {25, 4}, {25, 5},
	{25, 6}, {25, 7}, {35, 7}, {50, 7}, {71, 7}, {100, 7}};

	acphy_txiqcal_ladder_t ladder_iq[] = {
	{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
	{25, 0}, {35, 0}, {50, 0}, {71, 0}, {100, 0}, {100, 1},
	{100, 2}, {100, 3}, {100, 4}, {100, 5}, {100, 6}, {100, 7}};

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	for (indx = 0; indx < 18; indx++) {

		/* calculate and write LO cal gain ladder */
		bbmult_scaled = ladder_lo[indx].percent * bbmult;
		bbmult_scaled /= 100;
		tblentry = ((bbmult_scaled & 0xff) << 8) | ladder_lo[indx].g_env;
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, indx, 16, &tblentry);

		/* calculate and write IQ cal gain ladder */
		bbmult_scaled = ladder_iq[indx].percent * bbmult;
		bbmult_scaled /= 100;
		tblentry = ((bbmult_scaled & 0xff) << 8) | ladder_iq[indx].g_env;
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, indx+32, 16, &tblentry);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

static uint16
wlc_poll_adc_clamp_status(phy_info_t *pi, uint8 core, uint8 do_reset)
{
	uint16 ovr_status;

	if (TINY_RADIO(pi)) {
		ovr_status = READ_RADIO_REGFLD_TINY(pi, ADC_CFG14, core, adc_overload_i) |
			READ_RADIO_REGFLD_TINY(pi, ADC_CFG14, core, adc_overload_q);
	} else {
		ovr_status = READ_RADIO_REGFLDC(pi, RF_2069_ADC_STATUS(core), ADC_STATUS,
		                                i_wrf_jtag_afe_iqadc_overload);
	}

	if (ovr_status && do_reset) {
		/* MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, 0, afe_iqadc_reset_ov_det, 1); */
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_reset_ov_det,  1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_reset_ov_det, 0);
	}
	return ovr_status;
}

static void
wlc_phy_txcal_phy_cleanup_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_phyregs_t *porig = (pi_ac->txiqlocali->ac_txcal_phyregs_orig);
	uint8 core;
	if (TINY_RADIO(pi)) {
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0);
		MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);
	}

	/*  CLEANUP: Restore Original Values  */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* restore ExtPA PU & TR */
		WRITE_PHYREGCE(pi, RfctrlIntc, core, porig->RfctrlIntc[core]);

		/* restore Rfctrloverride setting */
		WRITE_PHYREGCE(pi, RfctrlOverrideRxPus, core, porig->RfctrlOverrideRxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreRxPus, core, porig->RfctrlCoreRxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideTxPus, core, porig->RfctrlOverrideTxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreTxPus, core, porig->RfctrlCoreTxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core,
			porig->RfctrlOverrideLpfSwtch[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, porig->RfctrlCoreLpfSwtch[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideLpfCT, core, porig->RfctrlOverrideLpfCT[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfCT, core, porig->RfctrlCoreLpfCT[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfGmult, core, porig->RfctrlCoreLpfGmult[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreRCDACBuf, core, porig->RfctrlCoreRCDACBuf[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, porig->RfctrlOverrideAuxTssi[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, porig->RfctrlCoreAuxTssi1[core]);

		WRITE_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, porig->RfctrlOverrideAfeCfg[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, porig->RfctrlCoreAfeCfg1[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, porig->RfctrlCoreAfeCfg2[core]);

		/* restore PAPD Enable
		 * FIXME: not supported (and not needed) yet
		 * phy_utils_write_phyreg(pi, NPHY_PapdEnable(core), porig->PapdEnable[core]);
		 */

	} /* for core */

	WRITE_PHYREG(pi, RxFeCtrl1, porig->RxFeCtrl1);
	WRITE_PHYREG(pi, AfePuCtrl, porig->AfePuCtrl);

	/* WRITE_PHYREG(pi, RfseqCoreActv2059, porig->RfseqCoreActv2059); */

	/* we should not need spur avoidance anymore
	WRITE_PHYREG(pi, BBConfig, porig->BBConfig);
	*/

	if (ACMAJORREV_1(pi->pubpi.phy_rev) && (ACMINORREV_0(pi) || ACMINORREV_1(pi))) {
		MOD_PHYREG(pi, RxFeCtrl1, rxfe_bilge_cnt, 0);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);
	}
	wlc_phy_resetcca_acphy(pi);
}

static void
wlc_phy_txcal_radio_cleanup_acphy_tiny(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_radioregs_t *porig = (pi_ac->ac_txcal_radioregs_orig);
	uint8 core;

	ASSERT(TINY_RADIO(pi));

	/* CLEANUP: restore reg values */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		phy_utils_write_radioreg(pi, RADIO_REG(pi, IQCAL_CFG1, core),
		                         porig->iqcal_cfg1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, AUXPGA_CFG1, core),
		                porig->auxpga_cfg1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, IQCAL_CFG3, core),
		                         porig->iqcal_cfg3[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TX_TOP_5G_OVR1, core),
		                porig->tx_top_5g_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_CFG10, core),
		                         porig->adc_cfg10[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, AUXPGA_OVR1, core),
		                porig->auxpga_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TESTBUF_OVR1, core),
		                porig->testbuf_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_OVR1, core),
		                         porig->adc_ovr1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TX5G_MISC_CFG1, core),
		                         porig->tx5g_misc_cfg1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TESTBUF_CFG1, core),
		                         porig->testbuf_cfg1[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TIA_CFG5, core),
		                         porig->tia_cfg5[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, TIA_CFG9, core),
		                         porig->tia_cfg9[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, AUXPGA_VMID, core),
		                         porig->auxpga_vmid[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, PMU_CFG4, core),
		                         porig->pmu_cfg4[core]);
		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && RADIOMAJORREV(pi) == 3) {
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, TX_TOP_2G_OVR_NORTH, core),
			                         porig->tx_top_2g_ovr_north[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, TX_TOP_2G_OVR_EAST, core),
			                         porig->tx_top_2g_ovr_east[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, IQCAL_CFG2, core),
			                         porig->iqcal_cfg2[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, PMU_OVR, core),
			                         porig->pmu_ovr[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, TX2G_MISC_CFG1, core),
			                         porig->tx2g_misc_cfg1[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, ADC_CFG18, core),
			                         porig->adc_cfg18[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core),
			                         porig->tx_top_5g_ovr2[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, TXMIX5G_CFG2, core),
			                         porig->txmix5g_cfg2[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, PAD5G_CFG1, core),
			                         porig->pad5g_cfg1[core]);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, PA5G_CFG1, core),
			                         porig->pa5g_cfg1[core]);
		} else {
			phy_utils_write_radioreg(pi, RADIO_REG(pi, IQCAL_OVR1, core),
			                         porig->iqcal_ovr1[core]);
			phy_utils_write_radioreg(pi, RADIO_REG(pi, PA2G_CFG1, core),
			                         porig->pa2g_cfg1[core]);
		}

		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID && (phy_get_phymode(pi) ==
			PHYMODE_MIMO) && (RADIOMAJORREV(pi) != 3)) {
			MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core, ovr_pa5g_pu, 0);
			MOD_RADIO_REG_TINY(pi, PA5G_CFG4, core, pa5g_pu, 0);
		}
	} /* for core */
}

static void
wlc_phy_txcal_radio_cleanup_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_radioregs_t *porig = (pi_ac->ac_txcal_radioregs_orig);
	uint8 core;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	/* CLEANUP: restore reg values */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		phy_utils_write_radioreg(pi, RF_2069_IQCAL_CFG1(core), porig->iqcal_cfg1[core]);
		phy_utils_write_radioreg(pi, RF_2069_IQCAL_CFG2(core), porig->iqcal_cfg2[core]);
		phy_utils_write_radioreg(pi, RF_2069_IQCAL_CFG3(core), porig->iqcal_cfg3[core]);
		phy_utils_write_radioreg(pi, RF_2069_PA2G_TSSI(core),  porig->pa2g_tssi[core]);
		phy_utils_write_radioreg(pi, RF_2069_TX5G_TSSI(core),  porig->tx5g_tssi[core]);
		phy_utils_write_radioreg(pi, RF_2069_AUXPGA_CFG1(core),  porig->auxpga_cfg1[core]);

		/* Reg conflict with 2069 rev 16 */
		if (RADIOMAJORREV(pi) == 0)
			phy_utils_write_radioreg(pi, RF_2069_OVR20(core),      porig->OVR20[core]);
		else
			phy_utils_write_radioreg(pi, RF_2069_GE16_OVR21(core), porig->OVR21[core]);
	} /* for core */
}

static void
wlc_phy_precal_txgain_control(phy_info_t *pi, txgain_setting_t *target_gains)
{
	int16  avvmid_set_local[2][2]     = {{1, 145}, {1,  145}};
	int16  target_tssi_set[2][5]   = {
		{710, 720, 425, 340, 370},
		{350, 350, 350, 350, 350}
	};
	int16 delta_tssi_error = 25;
	uint8  start_gain_idx[2][5][2] = {
		{{5, 31}, {5, 31}, {16, 31}, {16, 21}, {16, 24}},
		{{8, 31}, {8, 31}, {10, 31}, {14, 21}, {14, 24}}
	};
	uint8  gain_ladder[32] =
		{0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37, 0x3F,
	0x47, 0x4F, 0x57, 0x5F, 0x67, 0x6F, 0x77, 0x7F,
	0x87, 0x8F, 0x97, 0x9F, 0xA7, 0xAF, 0xB7, 0xBF,
	0xC7, 0xCF, 0xD7, 0xDF, 0xE7, 0xEF, 0xF7, 0xFF};

	uint8  band_idx, majorrev_idx, band_bw_idx, pad_gain, pga_gain, core;
	uint8  idx_min = 0, idx_max = 31, idx_curr = 0, idx_curr1 = 0, stall_val;
	uint8  need_more_gain, reduce_gain, adjust_pga, final_step;
	int16  idle_tssi[PHY_CORE_MAX], tone_tssi[PHY_CORE_MAX];
	int16  target_tssi, delta_tssi, delta_tssi1;

	struct _orig_reg_vals {
		uint8 core;
		uint16 orig_OVR3;
		uint16 orig_auxpga_cfg1;
		uint16 orig_auxpga_vmid;
		uint16 orig_iqcal_cfg1;
		uint16 orig_tx5g_tssi;
		uint16 orig_pa2g_tssi;
		uint16 orig_RfctrlIntc;
		uint16 orig_RfctrlOverrideRxPus;
		uint16 orig_RfctrlCoreRxPu;
		uint16 orig_RfctrlOverrideAuxTssi;
		uint16 orig_RfctrlCoreAuxTssi1;
	} orig_reg_vals[PHY_CORE_MAX];

	uint core_count = 0;
	uint8 max_pad_idx = 31;

	txgain_setting_t curr_gain, curr_gain1;
	bool init_adc_inside = FALSE;
	uint16 save_afePuCtrl, save_gpio, save_gpioHiOutEn;
	uint16 fval2g_orig, fval5g_orig, fval2g, fval5g;
	uint32 save_chipc = 0;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);
	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	band_idx = CHSPEC_IS5G(pi->radio_chanspec);

	if (band_idx == 0 && ACMAJORREV_1(pi->pubpi.phy_rev)) {
			max_pad_idx = 19;
	}

	if (CHSPEC_IS80(pi->radio_chanspec) || PHY_AS_80P80(pi, pi->radio_chanspec)) {
		band_bw_idx = band_idx * 2 + 2;
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		band_bw_idx = band_idx * 2 + 3;
		ASSERT(0); // FIXME
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		band_bw_idx = band_idx * 2 + 1;
	} else {
		band_bw_idx = band_idx * 2 + 0;
	}

	majorrev_idx = ACMAJORREV_1(pi->pubpi.phy_rev) ? 0 :1;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	/* Turn off epa/ipa and unused rxrf part to prevent energy go into air */
	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {

		/* save phy/radio regs going to be touched */
		orig_reg_vals[core_count].orig_RfctrlIntc = READ_PHYREGCE(pi, RfctrlIntc, core);
		orig_reg_vals[core_count].orig_RfctrlOverrideRxPus
			= READ_PHYREGCE(pi, RfctrlOverrideRxPus, core);
		orig_reg_vals[core_count].orig_RfctrlCoreRxPu
			= READ_PHYREGCE(pi, RfctrlCoreRxPus, core);
		orig_reg_vals[core_count].orig_RfctrlOverrideAuxTssi
			= READ_PHYREGCE(pi, RfctrlOverrideAuxTssi, core);
		orig_reg_vals[core_count].orig_RfctrlCoreAuxTssi1
			= READ_PHYREGCE(pi, RfctrlCoreAuxTssi1, core);

		orig_reg_vals[core_count].orig_OVR3 = READ_RADIO_REGC(pi, RF, OVR3, core);
		orig_reg_vals[core_count].orig_auxpga_cfg1 =
			READ_RADIO_REGC(pi, RF, AUXPGA_CFG1, core);
		orig_reg_vals[core_count].orig_auxpga_vmid =
			READ_RADIO_REGC(pi, RF, AUXPGA_VMID, core);
		orig_reg_vals[core_count].orig_iqcal_cfg1 =
			READ_RADIO_REGC(pi, RF, IQCAL_CFG1, core);
		orig_reg_vals[core_count].orig_tx5g_tssi = READ_RADIO_REGC(pi, RF, TX5G_TSSI, core);
		orig_reg_vals[core_count].orig_pa2g_tssi = READ_RADIO_REGC(pi, RF, PA2G_TSSI, core);

		orig_reg_vals[core_count].core = core;

		/* set proper Av/Vmid */
		MOD_RADIO_REGC(pi, OVR3, core, ovr_auxpga_i_sel_gain, 0x1);
		MOD_RADIO_REGC(pi, AUXPGA_CFG1, core,
		               auxpga_i_sel_gain, avvmid_set_local[band_idx][0]);
		MOD_RADIO_REGC(pi, OVR3, core, ovr_afe_auxpga_i_sel_vmid, 0x1);
		MOD_RADIO_REGC(pi, AUXPGA_VMID, core,
		               auxpga_i_sel_vmid, avvmid_set_local[band_idx][1]);

		/* turn off ext-pa and put ext-trsw in r position */
		WRITE_PHYREGCE(pi, RfctrlIntc, core, 0x1400);

		/* set tssi_range = 0 (it suppose to bypass 10dB attenuation before pdet) */
		MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, tssi_range, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, tssi_range, 0);

		/* turn off lna and other unsed rxrf components */
		WRITE_PHYREGCE(pi, RfctrlOverrideRxPus, core, 0x7CE0);
		WRITE_PHYREGCE(pi, RfctrlCoreRxPus,     core, 0x0);

		++core_count;
	}
	ACPHY_ENABLE_STALL(pi, stall_val);

	/* tssi loopback setup */
	phy_ac_tssi_loopback_path_setup(pi, LOOPBACK_FOR_IQCAL);

	if (!init_adc_inside) {
		wlc_phy_init_adc_read(pi, &save_afePuCtrl, &save_gpio,
		                      &save_chipc, &fval2g_orig, &fval5g_orig,
		                      &fval2g, &fval5g, &stall_val, &save_gpioHiOutEn);
	}

	FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
		if (!init_adc_inside)
			wlc_phy_gpiosel_acphy(pi, 16+core, 1);
		/* Measure the Idle TSSI */
		wlc_phy_poll_samps_WAR_acphy(pi, idle_tssi, TRUE, TRUE, NULL,
		                             TRUE, init_adc_inside, core, 0);
		/* Adjust Target TSSI based on Idle TSSI */
		target_tssi = target_tssi_set[majorrev_idx][band_bw_idx] + idle_tssi[core];

		/* set the initial txgain */
		wlc_phy_get_txgain_settings_by_index_acphy(pi, &curr_gain, 0);
		pad_gain = gain_ladder[start_gain_idx[majorrev_idx][band_bw_idx][0]];
		pga_gain = gain_ladder[start_gain_idx[majorrev_idx][band_bw_idx][1]];
		curr_gain.rad_gain_mi = (pad_gain & 0xFF) | ((pga_gain & 0xFF) << 8);
		curr_gain.bbmult = 64;

		/* Measure the tone TSSI */
		wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi, TRUE, FALSE,
		                             &curr_gain, TRUE, init_adc_inside, core, 0);
		delta_tssi  = target_tssi - tone_tssi[core];
		need_more_gain = (delta_tssi >= delta_tssi_error);
		reduce_gain = (delta_tssi < -delta_tssi_error);
		if (need_more_gain || reduce_gain) {
			/* if need more gain, try first max pad gain; otherwise, try min pga gain */
			curr_gain1 = curr_gain;
			curr_gain1.rad_gain_mi = (need_more_gain)
				? (
				   (curr_gain.rad_gain_mi & 0xFF00)
				   |((gain_ladder[max_pad_idx] & 0xFF) << 0))
				: (
				   (curr_gain.rad_gain_mi & 0x00FF)
				   |((gain_ladder[0]  & 0xFF) << 8));

			wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi, TRUE, FALSE,
			                             &curr_gain1, TRUE, init_adc_inside, core, 0);
			delta_tssi1 = target_tssi - tone_tssi[core];
			adjust_pga = (delta_tssi1 >= 0);

			if (need_more_gain) {
				idx_min = start_gain_idx[majorrev_idx][band_bw_idx][adjust_pga];
				idx_max = max_pad_idx;
				curr_gain.rad_gain_mi = (adjust_pga)?
					curr_gain1.rad_gain_mi : curr_gain.rad_gain_mi;
				curr_gain1  = curr_gain;
				delta_tssi1 = (adjust_pga)? delta_tssi1: delta_tssi;
			} else if (reduce_gain) {
				idx_min = 0;
				idx_max = start_gain_idx[majorrev_idx][band_bw_idx][adjust_pga];
				curr_gain.rad_gain_mi = (adjust_pga)?
					curr_gain.rad_gain_mi : curr_gain1.rad_gain_mi;
				curr_gain1  = curr_gain;
				delta_tssi1 = (adjust_pga)? delta_tssi: delta_tssi1;
			}

			final_step = 0;
			do {
				if (idx_min >= idx_max-1) {
					final_step = 1;
					idx_curr = (idx_curr == idx_min)? idx_max: idx_min;
				} else {
					idx_curr = (idx_min + idx_max) >> 1;
				}

				if (adjust_pga) {
					curr_gain.rad_gain_mi =
						(curr_gain.rad_gain_mi & 0x00FF) |
						((gain_ladder[idx_curr] & 0xFF) << 8);
				} else {
					curr_gain.rad_gain_mi =
						(curr_gain.rad_gain_mi & 0xFF00) |
						((gain_ladder[idx_curr] & 0xFF) << 0);
				}

				wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi, TRUE, FALSE,
				                             &curr_gain, TRUE,
				                             init_adc_inside, core, 0);
				delta_tssi  = target_tssi - tone_tssi[core];

				if (final_step) {
					if (ABS(delta_tssi) > ABS(delta_tssi1)) {
						idx_min  = idx_curr1;
						idx_max  = idx_curr1;
						idx_curr = idx_curr1;
						delta_tssi = delta_tssi1;
						curr_gain  = curr_gain1;
					} else {
						idx_min  = idx_curr;
						idx_max  = idx_curr;
					}
				} else {
					if (delta_tssi >= delta_tssi_error) {
						idx_min = idx_curr;
					} else if (delta_tssi < -delta_tssi_error) {
						idx_max = idx_curr;
					} else {
						idx_min = idx_curr;
						idx_max = idx_curr;
					}

					/* always log the current tssi & gain */
					delta_tssi1 = delta_tssi;
					curr_gain1  = curr_gain;
					idx_curr1   = idx_curr;
				}

				/* only used for debugging print */
				if (0) {
					if (adjust_pga) {
						printf("PGA-idx = (%d,%d,%d), dtssi = (%d, %d)\n",
						       idx_min, idx_curr, idx_max,
						       delta_tssi, delta_tssi1);
					} else {
						printf("PAD-idx = (%d,%d,%d), dtssi = (%d, %d)\n",
						       idx_min, idx_curr, idx_max,
						       delta_tssi, delta_tssi1);
					}
				}

			} while (idx_min < idx_max);
		}
		/* assign the best found gain */
		target_gains[core] = curr_gain;

		PHY_TRACE(("Best txgain found for Core%d: (%2x %2x %2x %2x %2x %2x)\n",
		           core, (target_gains[core].rad_gain & 0xF),
		           (target_gains[core].rad_gain >> 4) & 0xF,
		           (target_gains[core].rad_gain >> 8) & 0xFF,
		           (target_gains[core].rad_gain_mi >> 0) & 0xFF,
		           (target_gains[core].rad_gain_mi >> 8) & 0xFF,
		           (target_gains[core].rad_gain_hi >> 0) & 0xFF));
	}

	if (!init_adc_inside)
		wlc_phy_restore_after_adc_read(pi, &save_afePuCtrl, &save_gpio,
		                               &save_chipc,  &fval2g_orig,  &fval5g_orig,
		                               &fval2g,  &fval5g, &stall_val, &save_gpioHiOutEn);

	/* restore phy/radio regs */
	while (core_count > 0) {
		--core_count;
		phy_utils_write_radioreg(pi, RF_2069_OVR3(orig_reg_vals[core_count].core),
			orig_reg_vals[core_count].orig_OVR3);
		phy_utils_write_radioreg(pi, RF_2069_AUXPGA_CFG1(orig_reg_vals[core_count].core),
			orig_reg_vals[core_count].orig_auxpga_cfg1);
		phy_utils_write_radioreg(pi, RF_2069_AUXPGA_VMID(orig_reg_vals[core_count].core),
			orig_reg_vals[core_count].orig_auxpga_vmid);
		phy_utils_write_radioreg(pi, RF_2069_IQCAL_CFG1(orig_reg_vals[core_count].core),
			orig_reg_vals[core_count].orig_iqcal_cfg1);
		phy_utils_write_radioreg(pi, RF_2069_TX5G_TSSI(orig_reg_vals[core_count].core),
			orig_reg_vals[core_count].orig_tx5g_tssi);
		phy_utils_write_radioreg(pi, RF_2069_PA2G_TSSI(orig_reg_vals[core_count].core),
			orig_reg_vals[core_count].orig_pa2g_tssi);
		WRITE_PHYREGCE(pi, RfctrlIntc, orig_reg_vals[core_count].core,
			orig_reg_vals[core_count].orig_RfctrlIntc);
		WRITE_PHYREGCE(pi, RfctrlOverrideRxPus, orig_reg_vals[core_count].core,
			orig_reg_vals[core_count].orig_RfctrlOverrideRxPus);
		WRITE_PHYREGCE(pi, RfctrlCoreRxPus, orig_reg_vals[core_count].core,
			orig_reg_vals[core_count].orig_RfctrlCoreRxPu);
		WRITE_PHYREGCE(pi, RfctrlOverrideAuxTssi, orig_reg_vals[core_count].core,
			orig_reg_vals[core_count].orig_RfctrlOverrideAuxTssi);
		WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi1, orig_reg_vals[core_count].core,
			orig_reg_vals[core_count].orig_RfctrlCoreAuxTssi1);
	}

	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
	PHY_TRACE(("======= IQLOCAL PreCalGainControl : END =======\n"));

	return;
}

static void
wlc_phy_txcal_txgain_setup_acphy(phy_info_t *pi, txgain_setting_t *txcal_txgain,
	txgain_setting_t *orig_txgain)
{
	uint8 core;
	uint8 stall_val;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* store off orig and set new tx radio gain */
		if (core < 3) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
				&(orig_txgain[core].rad_gain));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
				&(orig_txgain[core].rad_gain_mi));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
				&(orig_txgain[core].rad_gain_hi));

			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
				&(txcal_txgain[core].rad_gain));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
				&(txcal_txgain[core].rad_gain_mi));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
				&(txcal_txgain[core].rad_gain_hi));
		} else {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x501, 16,
				&(orig_txgain[core].rad_gain));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x504, 16,
				&(orig_txgain[core].rad_gain_mi));
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x507, 16,
				&(orig_txgain[core].rad_gain_hi));

			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x501, 16,
				&(txcal_txgain[core].rad_gain));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x504, 16,
				&(txcal_txgain[core].rad_gain_mi));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x507, 16,
				&(txcal_txgain[core].rad_gain_hi));
		}

		PHY_NONE(("\n radio gain = 0x%x %x %x, bbm=%d, dacgn = %d  \n",
			txcal_txgain[core].rad_gain_hi,
			txcal_txgain[core].rad_gain_mi,
			txcal_txgain[core].rad_gain,
			txcal_txgain[core].bbmult,
			txcal_txgain[core].dac_gain));

		/* store off orig and set new bbmult gain */
		wlc_phy_get_tx_bbmult_acphy(pi, &(orig_txgain[core].bbmult),  core);
		wlc_phy_set_tx_bbmult_acphy(pi, &(txcal_txgain[core].bbmult), core);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
wlc_phy_txcal_txgain_cleanup_acphy(phy_info_t *pi, txgain_setting_t *orig_txgain)
{
	uint8 core;
	uint8 stall_val;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* restore gains: DAC, Radio and BBmult */
		if (core < 3) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
				&(orig_txgain[core].rad_gain));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
				&(orig_txgain[core].rad_gain_mi));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
				&(orig_txgain[core].rad_gain_hi));
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x501, 16,
				&(orig_txgain[core].rad_gain));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x504, 16,
				&(orig_txgain[core].rad_gain_mi));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x507, 16,
				&(orig_txgain[core].rad_gain_hi));
		}
		wlc_phy_set_tx_bbmult_acphy(pi, &(orig_txgain[core].bbmult), core);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
wlc_phy_txcal_phy_setup_acphy_core(phy_info_t *pi, acphy_txcal_phyregs_t *porig, uint8 core,
	uint16 bw_idx, uint16 sdadc_config, uint8 Biq2byp)
{
	/* Power Down External PA (simply always do 2G & 5G),
	 * and set T/R to T (double check TR position)
	 */
	porig->RfctrlIntc[core] = READ_PHYREGCE(pi, RfctrlIntc, core);
	WRITE_PHYREGCE(pi, RfctrlIntc, core, 0);
	MOD_PHYREGCE(pi, RfctrlIntc, core, ext_2g_papu, 0);
	MOD_PHYREGCE(pi, RfctrlIntc, core, ext_5g_papu, 0);
	MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_tx_pu, 0);
	MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_rx_pu, 0);
	MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_pa, 1);
	MOD_PHYREGCE(pi, RfctrlIntc, core, override_tr_sw, 1);

	/* Core Activate/Deactivate */
	/* MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, 0);
	   MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, (1 << core));
	 */

	/* Internal RFCtrl: save and adjust state of internal PA override */
	/* save state of Rfctrl override */
	porig->RfctrlOverrideAfeCfg[core]   = READ_PHYREGCE(pi, RfctrlOverrideAfeCfg, core);
	porig->RfctrlCoreAfeCfg1[core]      = READ_PHYREGCE(pi, RfctrlCoreAfeCfg1, core);
	porig->RfctrlCoreAfeCfg2[core]      = READ_PHYREGCE(pi, RfctrlCoreAfeCfg2, core);
	porig->RfctrlOverrideRxPus[core]    = READ_PHYREGCE(pi, RfctrlOverrideRxPus, core);
	porig->RfctrlCoreRxPus[core]        = READ_PHYREGCE(pi, RfctrlCoreRxPus, core);
	porig->RfctrlOverrideTxPus[core]    = READ_PHYREGCE(pi, RfctrlOverrideTxPus, core);
	porig->RfctrlCoreTxPus[core]        = READ_PHYREGCE(pi, RfctrlCoreTxPus, core);
	porig->RfctrlOverrideLpfSwtch[core] = READ_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core);
	porig->RfctrlCoreLpfSwtch[core]     = READ_PHYREGCE(pi, RfctrlCoreLpfSwtch, core);
	porig->RfctrlOverrideLpfCT[core]    = READ_PHYREGCE(pi, RfctrlOverrideLpfCT, core);
	porig->RfctrlCoreLpfCT[core]        = READ_PHYREGCE(pi, RfctrlCoreLpfCT, core);
	porig->RfctrlCoreLpfGmult[core]     = READ_PHYREGCE(pi, RfctrlCoreLpfGmult, core);
	porig->RfctrlCoreRCDACBuf[core]     = READ_PHYREGCE(pi, RfctrlCoreRCDACBuf, core);
	porig->RfctrlOverrideAuxTssi[core]  = READ_PHYREGCE(pi, RfctrlOverrideAuxTssi, core);
	porig->RfctrlCoreAuxTssi1[core]     = READ_PHYREGCE(pi, RfctrlCoreAuxTssi1, core);

	/* Turning off all the RF component that are not needed */
	wlc_phy_txcal_phy_setup_acphy_core_disable_rf(pi, core);

	/* Setting the loopback path */
	if (Biq2byp == 0) {
	  wlc_phy_txcal_phy_setup_acphy_core_loopback_path(pi, core, LPFCONF_TXIQ_RX2);
	} else {
	  /* select biq2 byp path */
	  wlc_phy_txcal_phy_setup_acphy_core_loopback_path(pi, core, LPFCONF_TXIQ_RX4);
	}

	/* Setting the SD-ADC related stuff */
	if (!(ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))) {
		wlc_phy_txcal_phy_setup_acphy_core_sd_adc(pi, core, sdadc_config);
	}

	/* Setting the LPF related stuff */
	wlc_phy_txcal_phy_setup_acphy_core_lpf(pi, core, bw_idx);

	/* disable PAPD (if enabled)
	 * FIXME: not supported (and not needed) yet
	 * porig->PapdEnable[core] = READ_PHYREGCE(pi, PapdEnable, core);
	 * MOD_PHYREGCE(pi, PapdEnable, core, compEnable, 0);
	 */
}

static void
wlc_phy_txcal_phy_setup_acphy_core_disable_rf(phy_info_t *pi, uint8 core)
{
	if (TINY_RADIO(pi)) {
		if ((CHSPEC_IS2G(pi->radio_chanspec) &&
			(READ_RADIO_REGFLD_TINY(pi, PA2G_CFG1, core, pa2g_tssi_ctrl_sel) == 0)) ||
			(CHSPEC_IS5G(pi->radio_chanspec) &&
			(READ_RADIO_REGFLD_TINY(pi, TX5G_MISC_CFG1, core,
			pa5g_tssi_ctrl_sel) == 0))) {
			MOD_PHYREGCE(pi, RfctrlCoreTxPus,     core, pa_pwrup,       1);
		}
	} else {
		MOD_PHYREGCE(pi, RfctrlCoreTxPus,     core, pa_pwrup,       0);
	}

	MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, pa_pwrup,               1);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_pwrup,        1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, rxrf_lna1_pwrup,        0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_5G_pwrup,     1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, rxrf_lna1_5G_pwrup,     0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_pwrup,        1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, rxrf_lna2_pwrup,        0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_nrssi_pwrup,        1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, lpf_nrssi_pwrup,        0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1g_pu,           1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, rssi_wb1g_pu,           0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1a_pu,           1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, rssi_wb1a_pu,           0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_wrssi3_pwrup,       1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus,     core, lpf_wrssi3_pwrup,       0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_wrssi2_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, rxrf_lna2_wrssi2_pwrup, 0);
}

static void
wlc_phy_txcal_phy_setup_acphy_core_loopback_path(phy_info_t *pi, uint8 core, uint8 lpf_config)
{
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, lpf_bq1_pu,             1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus,     core, lpf_bq1_pu,             1);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, lpf_bq2_pu,             1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus,     core, lpf_bq2_pu,             1);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, lpf_pu,                 1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus,     core, lpf_pu,                 1);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_pu_dc,              1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus,     core, lpf_pu_dc,              1);
	MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, tssi_pu,             1);
	MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, tssi_pu,             1);

	WRITE_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core,     0x3ff);
	if (lpf_config == LPFCONF_TXIQ_RX2) {
	  WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core,         0x152);
	} else {
	  WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core,         0x22a);
	}
}

static void
wlc_phy_txcal_phy_setup_acphy_core_sd_adc(phy_info_t *pi, uint8 core, uint16 sdadc_config)
{
	MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_mode, sdadc_config & 0x7);
	MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_mode, 1);
	MOD_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, afe_iqadc_pwrup, (sdadc_config >> 3) & 0x3f);
	MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_flashhspd, (sdadc_config >> 9) & 0x1);
	MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_flashhspd, 1);
	MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_ctrl_flash17lvl, (sdadc_config >> 10) & 0x1);
	MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_ctrl_flash17lvl, 1);
	MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_adc_bias, (sdadc_config >> 11) & 0x3);
	MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_adc_bias,  1);
}

static void
wlc_phy_txcal_phy_setup_acphy_core_lpf(phy_info_t *pi, uint8 core, uint16 bw_idx)
{
	MOD_PHYREGCE(pi, RfctrlOverrideLpfCT,    core, lpf_bq1_bw,    1);
	MOD_PHYREGCE(pi, RfctrlOverrideLpfCT,    core, lpf_bq2_bw,    1);
	MOD_PHYREGCE(pi, RfctrlOverrideLpfCT,    core, lpf_rc_bw,     1);

	MOD_PHYREGCE(pi, RfctrlCoreLpfCT,        core, lpf_bq1_bw,    3+bw_idx);

	if (ACMAJORREV_1(pi->pubpi.phy_rev) || ACMAJORREV_2(pi->pubpi.phy_rev) ||
		ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		if (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT,     core, lpf_bq2_bw,  6);
			MOD_PHYREGCE(pi, RfctrlCoreRCDACBuf,  core, lpf_rc_bw,   6);
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
			ASSERT(0); // FIXME
		} else {
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT,     core, lpf_bq2_bw,  5);
			MOD_PHYREGCE(pi, RfctrlCoreRCDACBuf,  core, lpf_rc_bw,   5);
		}
	} else if (ACMAJORREV_0(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
		MOD_PHYREGCE(pi, RfctrlCoreLpfCT,     core, lpf_bq2_bw,  3+bw_idx);
		MOD_PHYREGCE(pi, RfctrlCoreRCDACBuf,  core, lpf_rc_bw,   3+bw_idx);
	}

	if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
		/* Ideally, tssi_range should not impact IQcal at all.
		 * However, we found IQ cal performance improved with tssi_range = 0
		 */
		MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, tssi_range, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,    core, tssi_range, 0);
	}

	MOD_PHYREGCE(pi, RfctrlOverrideLpfCT,    core, lpf_q_biq2,    1);
	MOD_PHYREGCE(pi, RfctrlCoreLpfCT,        core, lpf_q_biq2,    0);
	MOD_PHYREGCE(pi, RfctrlOverrideLpfCT,    core, lpf_dc_bypass, 1);
	MOD_PHYREGCE(pi, RfctrlCoreLpfCT,        core, lpf_dc_bypass, 0);
	MOD_PHYREGCE(pi, RfctrlOverrideLpfCT,    core, lpf_dc_bw,     1);
	MOD_PHYREGCE(pi, RfctrlCoreLpfCT,        core, lpf_dc_bw,     4);  /* 133KHz */

	if (!(ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev) ||
	      ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev))) {
		MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, amux_sel_port, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, amux_sel_port, 2);
		MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, afe_iqadc_aux_en, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core, afe_iqadc_aux_en, 1);
	}
}

static void
wlc_phy_poll_adc_acphy(phy_info_t *pi, int32 *adc_buf, uint8 nsamps,
                       bool switch_gpiosel, uint16 core, bool is_tssi)
{
	/* Switching gpiosel is time consuming. We move the switch to an outer layer */
	/* and do it less frequently. */
	uint8 samp = 0;
	uint8 word_swap_flag = 1;
	int tmp = 0;

	ASSERT(core < PHY_CORE_MAX);
	adc_buf[2*core] = 0;
	adc_buf[2*core+1] = 0;

	if (switch_gpiosel)
		wlc_phy_gpiosel_acphy(pi, 16+core, word_swap_flag);
	for (samp = 0; samp < nsamps; samp++) {
		if ((ACMAJORREV_4(pi->pubpi.phy_rev) && (is_tssi == 1))) {
			/* Pulse the force_tssi_en to enable the trigger based TSSI Cap */
			if (core == 0) {
				MOD_PHYREG(pi, TxPwrCtrl_enable0, force_tssi_en, 1);
				OSL_DELAY(20);
				MOD_PHYREG(pi, TxPwrCtrl_enable0, force_tssi_en, 0);
				tmp = (int)((uint16)READ_PHYREG(pi, TssiVal_path0) & 0x3FF);
			} else if (core == 1) {
				MOD_PHYREG(pi, TxPwrCtrl_enable1, force_tssi_en, 1);
				OSL_DELAY(20);
				MOD_PHYREG(pi, TxPwrCtrl_enable1, force_tssi_en, 0);
				tmp = (int)((uint16)READ_PHYREG(pi, TssiVal_path1) & 0x3FF);
			}
			tmp = tmp > 511 ? (tmp - 1024) : tmp;
			adc_buf[2*core] += tmp;
			adc_buf[2*core+1] += tmp;
		} else if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
			(is_tssi == 1) && (core == 3)) {
			uint16 bitw_scale = (ACMAJORREV_32(pi->pubpi.phy_rev)) ? 4 : 1;
			phy_iq_est_t rx_iq_est[PHY_CORE_MAX];
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 1);
			wlc_phy_rx_iq_est_acphy(pi, rx_iq_est, 8, 32, 0, TRUE);
			adc_buf[2*core] = 0;
			adc_buf[2*core+1] = (int32)
				phy_utils_sqrt_int((uint32)(rx_iq_est[core].q_pwr >> 3))*bitw_scale;
			adc_buf[2*core+1] = ((1 << 10) - adc_buf[2*core+1]) << 3;
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);
		} else {
			/* read out the i-value */
			adc_buf[2*core] += READ_PHYREG(pi, gpioHiOut);
			/* read out the q-value */
			adc_buf[2*core+1] += READ_PHYREG(pi, gpioLoOut);
		}
	}
}

static void
wlc_phy_poll_samps_acphy(phy_info_t *pi, int16 *samp, bool is_tssi,
                         uint8 log2_nsamps, bool init_adc_inside,
                         uint16 core)
{
	int32 adc_buf[2*PHY_CORE_MAX];
	int32 k, tmp_samp, samp_accum[PHY_CORE_MAX];
	uint8 iq_swap;

	ASSERT(core < PHY_CORE_MAX);

	/* initialization */
	samp_accum[core] = 0;

	iq_swap = (is_tssi)? 1 : 0;

	wlc_phy_pulse_adc_reset_acphy(pi);
	OSL_DELAY(100);

	/* tssi val is (adc >> 2) */
	for (k = 0; k < (1 << log2_nsamps); k++) {
		if ((ACMAJORREV_4(pi->pubpi.phy_rev) && (is_tssi == 1))) {
			wlc_phy_poll_adc_acphy(pi, adc_buf, 1, 0, core, is_tssi);
		} else {
			wlc_phy_poll_adc_acphy(pi, adc_buf, 1, init_adc_inside, core, is_tssi);
		}
		if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
			if (is_tssi == 1) {
				/* Incase of trigger bassed capture TSSI out is 10bits */
				tmp_samp = adc_buf[2*core+iq_swap];
			} else {
				/* 13-10bit converstion In 4349A0 */
				tmp_samp = adc_buf[2*core+iq_swap] >> 3;
			}
		} else if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			tmp_samp = adc_buf[2*core+iq_swap] >> 3;
		} else {
			tmp_samp = adc_buf[2*core+iq_swap] >> 2;
		}
		tmp_samp -= (tmp_samp < 512) ? 0 : 1024;
		samp_accum[core] += tmp_samp;

	}

	samp[core] = (int16) (samp_accum[core] >> log2_nsamps);
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */

void
wlc_phy_populate_tx_loft_comp_tbl_acphy(phy_info_t *pi, uint16 *loft_coeffs)
{
	uint8  core, tbl_idx;
	uint8  nwords;
	uint8  txidx_thresh[2][3] = {{128, 128, 32}, {128, 26, 28}};
	/* core, band, pwridx (low then hi) */
	uint8  di_bias_tbl[3][2][2] = {{{0x00, 0x00}, {0x00, 0x00}},
	                               {{0x00, 0x00}, {0xec, 0xf6}},
	                               {{0xf6, 0xf8}, {0xf6, 0xfa}}};
	uint8  dq_bias_tbl[3][2][2] = {{{0x00, 0x00}, {0x00, 0x00}},
	                               {{0x00, 0x00}, {0x00, 0xfb}},
	                               {{0xf2, 0xfc}, {0xf6, 0xfb}}};
	uint8  i_bias, q_bias, pwr_idx;
	uint8  band_idx[3] = {0, 0, 0};
	uint8  ch_num_thresh[3] = {200, 149, 100};
	uint16 coeffs, di, dq, di_adj, dq_adj;

	/* no radio loft comp for tiny radio */
	if (TINY_RADIO(pi))
		return;

	nwords = 1;

	FOREACH_CORE(pi, core) {
		band_idx[core] = CHSPEC_CHANNEL(pi->radio_chanspec) >= ch_num_thresh[core];
	}

	for (tbl_idx = 0; tbl_idx < 128; tbl_idx ++) {
		FOREACH_CORE(pi, core) {

			switch (core) {
			case 0:
				coeffs = loft_coeffs[core];
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LOFTCOEFFLUTS0, nwords,
				    tbl_idx, 16, &coeffs);
				break;
			case 1:
			case 2:
				if (CHSPEC_IS2G(pi->radio_chanspec) == 1) {
					coeffs = loft_coeffs[core];
				} else {
					pwr_idx = tbl_idx <= txidx_thresh[band_idx[core]][core];

					i_bias = di_bias_tbl[core][band_idx[core]][pwr_idx];
					q_bias = dq_bias_tbl[core][band_idx[core]][pwr_idx];

					dq = loft_coeffs[core] & 0xff;
					di = (loft_coeffs[core] >> 8) & 0xff;
					di_adj = (di + i_bias) & 0xff;
					dq_adj = (dq + q_bias) & 0xff;

					/* for overflow protection */
					if ((di_adj & 0x80) && (i_bias & 0x80) &&
					            ((di_adj & 0x80) == 0)) {
						di_adj = 0x80;
					} else if (((di_adj & 0x80) == 0) &&
					            ((i_bias & 0x80) == 0) && (di_adj & 0x80)) {
						di_adj = 0x7f;
					}

					if ((dq_adj & 0x80) && (q_bias & 0x80) &&
					            ((dq_adj & 0x80) == 0)) {
						dq_adj = 0x80;
					} else if (((dq_adj & 0x80) == 0) &&
					            ((q_bias & 0x80) == 0) && (dq_adj & 0x80)) {
						dq_adj = 0x7f;
					}
					/* dq: 8LSB & di: 8MSB */
					coeffs = dq_adj + (di_adj << 8);
				}

				if (core == 1) {
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LOFTCOEFFLUTS1,
					      nwords, tbl_idx, 16, &coeffs);
				} else {
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_LOFTCOEFFLUTS2,
					      nwords, tbl_idx, 16, &coeffs);
				}
				break;
			}
		}
	}
}

uint16
wlc_phy_set_txpwr_by_index_acphy(phy_info_t *pi, uint8 core_mask, int8 txpwrindex)
{
	uint8 core;
	txgain_setting_t txgain_settings;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(core_mask);
	ASSERT(txpwrindex >= 0);	/* negative index not supported */

	ACPHY_DISABLE_STALL(pi);

	/* Set tx power based on an input "index"
	 * (Emulate what HW power control would use for a given table index)
	 */

	FOREACH_ACTV_CORE(pi, core_mask, core) {
		/* Read tx gain table */
		wlc_phy_get_txgain_settings_by_index_acphy(pi, &txgain_settings, txpwrindex);

		/* Override gains: DAC, Radio and BBmult */
		if (core < 3) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				(0x100 + core), 16, &(txgain_settings.rad_gain));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				(0x103 + core), 16, &(txgain_settings.rad_gain_mi));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				(0x106 + core), 16, &(txgain_settings.rad_gain_hi));
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				0x501, 16, &(txgain_settings.rad_gain));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				0x504, 16, &(txgain_settings.rad_gain_mi));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				0x507, 16, &(txgain_settings.rad_gain_hi));
		}
		wlc_phy_set_tx_bbmult_acphy(pi, &txgain_settings.bbmult, core);

		PHY_TXPWR(("wl%d: %s: Fixed txpwrindex for core%d is %d\n",
		          pi->sh->unit, __FUNCTION__, core, txpwrindex));
	}
	ACPHY_ENABLE_STALL(pi, stall_val);

	return txgain_settings.bbmult;
}

void
wlc_phy_precal_txgain_acphy(phy_info_t *pi, txgain_setting_t *target_gains)
{
	/*   This function determines the tx gain settings to be
	 *   used during tx iqlo calibration; that is, it sends back
	 *   the following settings for each core:
	 *       - radio gain
	 *       - dac gain
	 *       - bbmult
	 *   This is accomplished by choosing a predefined power-index, or by
	 *   setting gain elements explicitly to predefined values, or by
	 *   doing actual "pre-cal gain control". Either way, the idea is
	 *   to get a stable setting for which the swing going into the
	 *   envelope detectors is large enough for good "envelope ripple"
	 *   while avoiding distortion or EnvDet overdrive during the cal.
	 *
	 *   Note:
	 *       - this function and the calling infrastructure is set up
	 *         in a way not to leave behind any modified state; this
	 *         is in contrast to mimophy ("nphy"); in acphy, only the
	 *         desired gain quantities are set/found and set back
	 */

	uint8 core;
	uint8 phy_bw;
	acphy_cal_result_t *accal = &pi->cal_info->u.accal;

	uint8 en_precal_gain_control = 0;
	int8 tx_pwr_idx[3] = {20, 30, 20};
	/* reset ladder_updated flags so tx-iqlo-cal ensures appropriate recalculation */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		accal->txiqlocal_ladder_updated[core] = 0;
	}
	/* phy_bw */
	if (CHSPEC_IS80(pi->radio_chanspec) ||
			PHY_AS_80P80(pi, pi->radio_chanspec)) {
		phy_bw = 80;
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		phy_bw = 160;
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		phy_bw = 40;
	} else {
		phy_bw = 20;
	}

	/* Enable Precal gain control only for 4335 */
	if (ACMAJORREV_1(pi->pubpi.phy_rev) || ACMAJORREV_2(pi->pubpi.phy_rev)) {
		en_precal_gain_control = 2;
	}

	if (en_precal_gain_control == 0) {
		/* get target tx gain settings */

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			/* specify tx gain by index (reads from tx power table) */
			int8 target_pwr_idx;
			if (ACREV_IS(pi->pubpi.phy_rev, 1) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
				/* for 4360B0 and 43602 using 0.5dB-step, idx is lower */
				target_pwr_idx = (core != 0) ? 30 : 20;
				if (wlc_phy_get_chan_freq_range_acphy(pi, 0) ==
				        WL_CHAN_FREQ_RANGE_5G_BAND3) {
					target_pwr_idx = tx_pwr_idx[core];
				}
			} else if (RADIOID(pi->pubpi.radioid) == BCM2069_ID &&
			           RADIOMAJORREV(pi) == 1 &&
			           !(ACRADIO_2069_EPA_IS(pi->pubpi.radiorev))) {
				if (CHSPEC_IS2G(pi->radio_chanspec) == 1) {
					target_pwr_idx = 1;
				} else {
					if (phy_bw == 20)
						target_pwr_idx = 0;
					else if (phy_bw == 40)
						target_pwr_idx = 15;
					else
						target_pwr_idx = 10;
				}
			} else if (TINY_RADIO(pi)) {
				if (PHY_IPA(pi)) {
					if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
						if (CHSPEC_IS5G(pi->radio_chanspec)) {
							target_pwr_idx = 40;
						} else {
							target_pwr_idx = 50;
						}
					} else {
						if (CHSPEC_IS5G(pi->radio_chanspec)) {
							target_pwr_idx = 62;
						} else {
							target_pwr_idx = 50;
						}
					}
				} else {
					if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
						if (CHSPEC_IS5G(pi->radio_chanspec)) {
							if (RADIOREV(pi->pubpi.radiorev) == 10) {
								target_pwr_idx = 4;
							} else {
								target_pwr_idx = 10;
							}
						} else {
							target_pwr_idx = 8;
						}
					} else if (ACMAJORREV_32(pi->pubpi.phy_rev) ||
						ACMAJORREV_33(pi->pubpi.phy_rev)) {
						target_pwr_idx = 35;
					} else if (pi->txgaintbl5g == 1) {
						target_pwr_idx = 60;
					} else {
						target_pwr_idx = 50;
					}
				}
				if (CHSPEC_IS2G(pi->radio_chanspec) && (pi->txiqcalidx2g != -1)) {
					target_pwr_idx = pi->txiqcalidx2g;
				} else if (CHSPEC_IS5G(pi->radio_chanspec) &&
					(pi->txiqcalidx5g != -1)) {
					target_pwr_idx = pi->txiqcalidx5g;
				}
			} else {
				target_pwr_idx = 30;
			}

			wlc_phy_get_txgain_settings_by_index_acphy(
				pi, &(target_gains[core]), target_pwr_idx);

			if ((CHSPEC_IS5G(pi->radio_chanspec) == 1) &&
			    (RADIOID(pi->pubpi.radioid) == BCM2069_ID &&
			     RADIOMAJORREV(pi) == 1)) {
				/* use PAD gain 255 for TXIQLOCAL */
				target_gains[core].rad_gain_mi |= 0xff;
			}
		}

	} else if (en_precal_gain_control == 1) {
		PHY_TRACE(("========= Calling precal gain control =========\n"));
		wlc_phy_precal_target_tssi_search(pi, &(target_gains[0]));
	} else if (en_precal_gain_control == 2) {
		PHY_TRACE(("========= Calling precal gain control =========\n"));
		wlc_phy_precal_txgain_control(pi, &(target_gains[0]));
	}
}

int
wlc_phy_cal_txiqlo_acphy(phy_info_t *pi, uint8 searchmode, uint8 mphase, uint8 Biq2byp)
{

	uint8  bw_idx, rd_select = 0, wr_select1 = 0, wr_select2 = 0;
	uint16 tone_ampl;
	uint16 tone_freq;
	int    bcmerror = BCME_OK;
	uint8  num_cmds_total, num_cmds_per_core;
	uint8  cmd_idx, cmd_stop_idx, core, cal_type;
	uint16 *cmds;
	uint16 cmd;
	uint16 coeffs[2];
	uint16 *coeff_ptr;
	uint16 zero = 0;
	uint8  cr, k, num_cores, thidx;
	uint16 classifier_state;
	txgain_setting_t orig_txgain[4];
	acphy_cal_result_t *accal = &pi->cal_info->u.accal;
	uint8 last_phase; /* To update cal tables in the final phase of mphase cal */

	/* Tx fdiqi related: begin */
	int16 tone;
	int32 tx_fdiq_tone_list[ACPHY_TXCAL_MAX_NUM_FREQ] = {8, 4, -4, -8};
	acphy_fdiqi_t tfreq_ang_mag[ACPHY_TXCAL_MAX_NUM_FREQ];
	uint8 fdiq_loop = 0;
	math_cint32 tmp;
	math_cint32 tmp2;
	int32 ang;
	int fdiq_data_valid = 0;
	uint8 sweep_tone = 1;
	uint8 tone_idx = 0;
	/* Tx fdiqi related: end */

	/* structure for saving txpu_ovrd, txpu_val
	 * 2 register values for each core
	 */
	struct _save_regs {
		uint16 reg_val;
		uint16 reg_addr;
	} savereg[PHY_CORE_MAX*2];

	uint core_count = 0;
	uint core_off = 0;

	/* -----------
	 *  Constants
	 * -----------
	 */

	/* Table of commands for RESTART & REFINE search-modes
	 *
	 *     This uses the following format (three hex nibbles left to right)
	 *      1. cal_type: 0 = IQ (a/b),   1 = deprecated
	 *                   2 = LOFT digital (di/dq)
	 *                   3 = LOFT analog, fine,   injected at mixer      (ei/eq)
	 *                   4 = LOFT analog, coarse, injected at mixer, too (fi/fq)
	 *      2. initial stepsize (in log2)
	 *      3. number of cal precision "levels"
	 *
	 *     Notes: - functions assumes that order of LOFT cal cmds will be f => e => d,
	 *              where it's ok to have multiple cmds (say interrupted by IQ) of
	 *              the same type; this is due to zeroing out of e and/or d that happens
	 *              even during REFINE cal to avoid a coefficient "divergence" (increasing
	 *              LOFT comp over time of different types that cancel each other)
	 *            - final cal cmd should NOT be analog LOFT cal (otherwise have to manually
	 *              pick up analog LOFT settings from best_coeffs and write to radio)
	 */
	uint16 cmds_RESTART[] = { 0x434, 0x334, 0x084, 0x267, 0x056, 0x234};
	uint16 cmds_REFINE[] = { 0x423, 0x334, 0x073, 0x267, 0x045, 0x234};

	uint16 cmds_RESTART_FDIQ[] = { 0x434, 0x334, 0x084, 0x267, 0x056, 0x234};
	uint16 cmds_REFINE_FDIQ[] =  { 0x423, 0x334, 0x073, 0x267, 0x045, 0x234};

	/* if the LOFT digi coeff is negative , DAC output stays at random value for */
	/* one clock cycle after pkt ends (after tx reset) */

	/* so WAR for this is after IQ cal is finished, force digi coeff to 8,8 and then do */
	/* analog fine cal and then do digi cal with grid size of 8 , which will make sure */
	/* digi coeff is always non negative */

	uint16 cmds_RESTART_majrev1[] = { 0x434, 0x334, 0x084, 0x267, 0x056, 0x334, 0x234};
	uint16 cmds_REFINE_majrev1[] = { 0x423, 0x334, 0x073, 0x267, 0x045, 0x334, 0x234};

	/* txidx dependent digital loft comp table */
	uint16 cmds_RESTART_TDDLC[] = { 0x434, 0x334, 0x084, 0x267, 0x056, 0x234};
	uint16 cmds_REFINE_TDDLC[] = { 0x423, 0x334, 0x073, 0x267, 0x045, 0x234};

	/* Pre RX IQ cal coeffs */
	uint16 cmds_RESTART_PRERX[] = { 0x084, 0x056};
	uint16 cmds_REFINE_PRERX[] = { 0x073, 0x045};
	/* zeros start coeffs (a,b,di/dq,ei/eq,fi/fq for each core) */
	uint16 start_coeffs_RESTART[] = {0, 0, 0, 0, 0,  0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,  0, 0, 0, 0, 0};

	/* interval lengths for gain control and correlation segments
	 *   (top/bottom nibbles are for guard and measurement intlvs, resp., in log 2 # samples)
	 */
	uint8 nsamp_gctrl[3];
	uint8 nsamp_corrs[3];
	uint8 thres_ladder[7];
	uint16 loft_coeffs[] = {0, 0, 0};
	uint16 *loft_coeffs_ptr = loft_coeffs;
	uint16 idx_for_loft_comp_tbl = 5;

	uint16 papdState[PHY_CORE_MAX];
	uint16 stall_val;

	/* initialize the tfreq_ang_mag array */
	memset(&tfreq_ang_mag, 0x00, sizeof(tfreq_ang_mag));

	/* -------
	 *  Inits
	 * -------
	 */
	 /* Disable PAPD */
	FOREACH_CORE(pi, core) {
		papdState[core] = READ_PHYREGCE(pi, PapdEnable, core);
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);
	}

	num_cores = PHYCORENUM(pi->pubpi.phy_corenum);
	/* X52c should run cal only on the 2 active core
	 * Cal on 3rd core corrupts the LO/IQ ladder in
	 * IQLOCAL TBL
	 */
	if (IS_X52C_BOARDTYPE(pi)) {
		num_cores = 2;
	}
	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);
	/* phy_bw */
	if (CHSPEC_IS80(pi->radio_chanspec) ||
			PHY_AS_80P80(pi, pi->radio_chanspec)) {
		bw_idx = 2;
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		bw_idx = 3;
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		bw_idx = 1;
	} else {
		bw_idx = 0;
	}

	if (RADIOID(pi->pubpi.radioid) == BCM2069_ID &&
	    RADIOMAJORREV(pi) == 1 &&
	    !(ACRADIO_2069_EPA_IS(pi->pubpi.radiorev))) {
		if (CHSPEC_IS2G(pi->radio_chanspec) == 1) {
			nsamp_gctrl[0] = 0x87; nsamp_gctrl[1] = 0x77; nsamp_gctrl[2] = 0x77;
			nsamp_corrs[0] = 0x79; nsamp_corrs[1] = 0x79; nsamp_corrs[2] = 0x79;
		} else {
			nsamp_gctrl[0] = 0x78; nsamp_gctrl[1] = 0x88; nsamp_gctrl[2] = 0x98;
			nsamp_corrs[0] = 0x89; nsamp_corrs[1] = 0x79; nsamp_corrs[2] = 0x79;
		}

		cmds_REFINE_majrev1[0] = 0x434; cmds_REFINE_majrev1[1] = 0x334;
		cmds_REFINE_majrev1[2] = 0x084;
		cmds_REFINE_majrev1[3] = 0x267; cmds_REFINE_majrev1[4] = 0x056;
		cmds_REFINE_majrev1[5] = 0x334;
		cmds_REFINE_majrev1[6] = 0x234;

		thres_ladder[0] = 0x3d; thres_ladder[1] = 0x2d; thres_ladder[2] = 0x1d;
		thres_ladder[3] = 0x0d; thres_ladder[4] = 0x07; thres_ladder[5] = 0x03;
		thres_ladder[6] = 0x01;
	} else {
		nsamp_gctrl[0] = 0x76; nsamp_gctrl[1] = 0x87; nsamp_gctrl[2] = 0x98;
		if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
			nsamp_corrs[0] = 0x7D; nsamp_corrs[1] = 0x7E; nsamp_corrs[2] = 0x7E;
		} else {
			nsamp_corrs[0] = 0x79; nsamp_corrs[1] = 0x79; nsamp_corrs[2] = 0x79;
		}

		if (TINY_RADIO(pi)) {
			thres_ladder[0] = 0x3d; thres_ladder[1] = 0x2d; thres_ladder[2] = 0x1d;
			thres_ladder[3] = 0x0d; thres_ladder[4] = 0x07;
			thres_ladder[5] = 0x03; thres_ladder[6] = 0x01;

			cmds_RESTART[0] = 0x265; cmds_RESTART[1] = 0x234;
			cmds_RESTART[2] = 0x084; cmds_RESTART[3] = 0x074;
			cmds_RESTART[4] = 0x056;

			cmds_REFINE[0] = 0x265; cmds_REFINE[1] = 0x234;
			cmds_REFINE[2] = 0x084; cmds_REFINE[3] = 0x074;
			cmds_REFINE[4] = 0x056;

			/* using the depricated cal cmd 1 to indicate the start of FDIQ cal loop  */
			cmds_RESTART_FDIQ[0] = 0x265; cmds_RESTART_FDIQ[1] = 0x234;
			cmds_RESTART_FDIQ[2] = 0x084; cmds_RESTART_FDIQ[3] = 0x074;
			cmds_RESTART_FDIQ[4] = 0x056; cmds_RESTART_FDIQ[4] = 0x156;

			cmds_REFINE_FDIQ[0] = 0x265; cmds_REFINE_FDIQ[1] = 0x234;
			cmds_REFINE_FDIQ[2] = 0x084; cmds_REFINE_FDIQ[3] = 0x074;
			cmds_REFINE_FDIQ[4] = 0x056; cmds_REFINE_FDIQ[5] = 0x156;

		} else {
			thres_ladder[0] = 0x3d; thres_ladder[1] = 0x1e; thres_ladder[2] = 0xf;
			thres_ladder[3] = 0x07; thres_ladder[4] = 0x03;
			thres_ladder[5] = 0x01;

			if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
				cmds_REFINE_majrev1[0] =  0x423; cmds_REFINE_majrev1[1] = 0x334;
				cmds_REFINE_majrev1[2] = 0x073;
				cmds_REFINE_majrev1[3] =  0x267; cmds_REFINE_majrev1[4] = 0x045;
				cmds_REFINE_majrev1[5] = 0x334;
				cmds_REFINE_majrev1[6] = 0x234;
			} else {
				cmds_REFINE[0] =  0x423; cmds_REFINE[1] = 0x334;
				cmds_REFINE[2] = 0x073;
				cmds_REFINE[3] =  0x267; cmds_REFINE[4] = 0x045;
				cmds_REFINE[5] = 0x234;
			}
		}
	}

	/* Put the radio and phy into TX iqlo cal state, including tx gains */
	classifier_state = READ_PHYREG(pi, ClassifierCtrl);
	wlc_phy_classifier_acphy(pi, ACPHY_ClassifierCtrl_classifierSel_MASK, 4);

	if (TINY_RADIO(pi))
		wlc_phy_txcal_radio_setup_acphy_tiny(pi);
	else
		wlc_phy_txcal_radio_setup_acphy(pi);

	wlc_phy_txcal_phy_setup_acphy(pi, Biq2byp);
	wlc_phy_txcal_txgain_setup_acphy(pi, &accal->txcal_txgain[0], &orig_txgain[0]);

	/* 4350A0 FIXME: Add Jira
	 * Need to force gated clks on to allow iqcal_done to be cleared
	 * only needed for 80 MHz but enable for 20 and 40 MHz anyway
	 */
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

	if (TINY_RADIO(pi)) {
		/* # no radio LOFT or programmable radio gain for tiny */
		WRITE_PHYREG(pi, TX_iqcal_gain_bwAddress, 0);
		WRITE_PHYREG(pi, TX_loft_fine_iAddress, 0);
		WRITE_PHYREG(pi, TX_loft_fine_qAddress, 0);
		WRITE_PHYREG(pi, TX_loft_coarse_iAddress, 0);
		WRITE_PHYREG(pi, TX_loft_coarse_qAddress, 0);

	}

	/* Set IQLO Cal Engine Gain Control Parameters including engine Enable
	 * Format: iqlocal_en<15> / gain start_index / NOP / ladder_length_d2)
	 */
	WRITE_PHYREG(pi, iqloCalCmdGctl, 0x8a09);

	/*
	 *   Retrieve and set Start Coeffs
	 */
	if (pi->cal_info->cal_phase_id > ACPHY_CAL_PHASE_TX0) {
		/* mphase cal and have done at least 1 Tx phase already */
		coeff_ptr = accal->txiqlocal_interm_coeffs; /* use results from previous phase */
	} else {
		/* single-phase cal or first phase of mphase cal */
		if (searchmode == PHY_CAL_SEARCHMODE_REFINE) {
			/* recal ("refine") */
			coeff_ptr = accal->txiqlocal_coeffs; /* use previous cal's final results */
		} else {
			/* start from zero coeffs ("restart") */
			coeff_ptr = start_coeffs_RESTART; /* zero coeffs */
		}
		/* copy start coeffs to intermediate coeffs, for pairwise update from here on
		 *    (after all cmds/phases have filled this with latest values, this
		 *    will be copied to OFDM/BPHY coeffs and to accal->txiqlocal_coeffs
		 *    for use by possible REFINE cal next time around)
		 */
		for (k = 0; k < 5*num_cores; k++) {
			accal->txiqlocal_interm_coeffs[k] = coeff_ptr[k];
		}
	}
	FOREACH_CORE(pi, core) {
		wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 0,
		                                TB_START_COEFFS_AB, core);
		/* Restart or refine with Biq2byp option should not touch
		 * d,e,f coeffs
		 */

		if (Biq2byp == 0) {
		wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 2,
		                                TB_START_COEFFS_D,  core);
		wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 3,
		                                TB_START_COEFFS_E,  core);
		wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE, coeff_ptr + 5*core + 4,
		                                TB_START_COEFFS_F,  core);
		}
	}

	if ((ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) &&
	    CHSPEC_IS80(pi->radio_chanspec))  {
		wlc_phy_tx_fdiqi_comp_acphy(pi, FALSE, 0xFF);
	}

	/*
	 *   Choose Cal Commands for this Phase
	 */
	if (searchmode == PHY_CAL_SEARCHMODE_RESTART) {
		if (Biq2byp) {
			cmds = cmds_RESTART_PRERX;
			num_cmds_per_core = ARRAYSIZE(cmds_RESTART_PRERX);
		} else if (ACREV_IS(pi->pubpi.phy_rev, 1) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
			cmds = cmds_RESTART_TDDLC;
			num_cmds_per_core = ARRAYSIZE(cmds_RESTART_TDDLC);
		} else if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			cmds = cmds_RESTART_majrev1;
			num_cmds_per_core = ARRAYSIZE(cmds_RESTART_majrev1);
		} else if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev)) {
			cmds = cmds_RESTART;
			/* numb of commands in tiny are 5 only */
			num_cmds_per_core = 5;

			if (!ACMAJORREV_4(pi->pubpi.phy_rev) && CHSPEC_IS80(pi->radio_chanspec))  {
				/* Restricting FDIQ cal to 80Mhz.   */
				cmds = cmds_RESTART_FDIQ;
				num_cmds_per_core = ARRAYSIZE(cmds_RESTART_FDIQ);
			}
		} else {
			cmds = cmds_RESTART;
			num_cmds_per_core = ARRAYSIZE(cmds_RESTART);
		}
		num_cmds_total    = num_cores * num_cmds_per_core;
	} else {
		if (Biq2byp) {
			cmds = cmds_REFINE_PRERX;
			num_cmds_per_core = ARRAYSIZE(cmds_REFINE_PRERX);
		} else if (ACREV_IS(pi->pubpi.phy_rev, 1) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
			cmds = cmds_REFINE_TDDLC;
			num_cmds_per_core = ARRAYSIZE(cmds_REFINE_TDDLC);
		} else if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			cmds = cmds_REFINE_majrev1;
			num_cmds_per_core = ARRAYSIZE(cmds_REFINE_majrev1);
		} else if (ACMAJORREV_4(pi->pubpi.phy_rev) || ACMAJORREV_32(pi->pubpi.phy_rev) ||
			ACMAJORREV_33(pi->pubpi.phy_rev)) {
			cmds = cmds_REFINE;
			/* numb of commands in tiny are 5 only */
			num_cmds_per_core = 5;
			if (!ACMAJORREV_4(pi->pubpi.phy_rev) && CHSPEC_IS80(pi->radio_chanspec))  {
				/* Restricting FDIQ cal to 80Mhz.   */
				cmds = cmds_REFINE_FDIQ;
				num_cmds_per_core = ARRAYSIZE(cmds_REFINE_FDIQ);
			}
		} else {
			cmds = cmds_REFINE;
			num_cmds_per_core = ARRAYSIZE(cmds_REFINE);
		}
		num_cmds_total    = num_cores * num_cmds_per_core;
	}

	if (mphase) {
		/* multi-phase: get next subset of commands (first & last index) */
		if (Biq2byp) {
			cmd_idx = (pi->cal_info->cal_phase_id - ACPHY_CAL_PHASE_TXPRERXCAL0) *
			MPHASE_TXCAL_CMDS_PER_PHASE; /* first cmd index in this phase */
		} else {
			cmd_idx = (pi->cal_info->cal_phase_id - ACPHY_CAL_PHASE_TX0) *
			MPHASE_TXCAL_CMDS_PER_PHASE; /* first cmd index in this phase */
		}
		if ((cmd_idx + MPHASE_TXCAL_CMDS_PER_PHASE - 1) < num_cmds_total) {
			cmd_stop_idx = cmd_idx + MPHASE_TXCAL_CMDS_PER_PHASE - 1;
		} else {
			cmd_stop_idx = num_cmds_total - 1;
		}
	} else {
		/* single-phase: execute all commands for all cores */
		cmd_idx = 0;
		cmd_stop_idx = num_cmds_total - 1;
	}

	/* turn on test tone */
	tone_ampl = 250;
	/* fixme: wlc_phy_tx_tone_acphy is playing 2x frequency.
	 * once that's fixed, we should use 4/8/12 mHz for iqlocal
	 */
	tone_freq = (CHSPEC_IS80(pi->radio_chanspec) ||
		PHY_AS_80P80(pi, pi->radio_chanspec)) ?
		ACPHY_IQCAL_TONEFREQ_80MHz :
		CHSPEC_IS160(pi->radio_chanspec) ? 0 : // FIXME
		CHSPEC_IS40(pi->radio_chanspec) ? ACPHY_IQCAL_TONEFREQ_40MHz :
		ACPHY_IQCAL_TONEFREQ_20MHz;

	tone_freq = tone_freq >> 1;

	bcmerror = wlc_phy_tx_tone_acphy(pi, (int32)tone_freq, tone_ampl, 1, 0, FALSE);

	OSL_DELAY(5);

	if (TINY_RADIO(pi)) {
		/* #restore bbmult overwritten by tone */
		uint16 m[4] = {0, 0, 0, 0};

		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			m[core] = accal->txcal_txgain[core].bbmult;
		}

		wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3],
			pi->pubpi.phy_coremask);
	}

	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2,    core, afe_iqadc_reset_ov_det, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_reset_ov_det, 1);
	}
	PHY_NONE(("wlc_phy_cal_txiqlo_acphy (after inits): SearchMd=%d, MPhase=%d,"
		" CmdIds=(%d to %d), Biq2byp=%d\n",
		searchmode, mphase, cmd_idx, cmd_stop_idx, Biq2byp));

	if (TINY_RADIO(pi) && pi->iboard) {
		OSL_DELAY(10000);
	}
	/* ---------------
	 *  Cmd Execution
	 * ---------------
	 */

	if (bcmerror == BCME_OK) { /* in case tone doesn't start (still needed?) */

		/* loop over commands in this cal phase */
		for (; cmd_idx <= cmd_stop_idx; cmd_idx++) {

			/* get command, cal_type, and core */
			core     = cmd_idx / num_cmds_per_core; /* integer divide */
			if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
				if (phy_get_phymode(pi) == PHYMODE_MIMO) {
					uint8 off_core = 0;
					if (core == 0) {
						off_core = 1;
					} else {
						off_core = 0;
					}
					MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, off_core,
						ovr_pa5g_pu, 1);
					MOD_RADIO_REG_TINY(pi, PA5G_CFG4, off_core, pa5g_pu, 0);
					MOD_PHYREGCE(pi, RfctrlOverrideTxPus, off_core,
						txrf_pwrup, 0x1);
					MOD_PHYREGCE(pi, RfctrlCoreTxPus, off_core,
						txrf_pwrup, 0x0);
					MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core,
						ovr_pa5g_pu, 1);
					MOD_RADIO_REG_TINY(pi, PA5G_CFG4, core, pa5g_pu, 1);
					MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core,
						txrf_pwrup, 0x1);
					MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, txrf_pwrup, 0x1);
				}
			}
			/* only execute commands when the current core is active
			 * if ((pi->sh->phytxchain >> core) & 0x1) {
			 */
			/* Turn Off Inactive cores for 43602 to improve IQ cal */
			if (ACMAJORREV_5(pi->pubpi.phy_rev)) {
			    FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core_off) {
				if (core != core_off) {
				    savereg[core_count].reg_val =
				      READ_PHYREGCE(pi, RfctrlOverrideTxPus, core_off);
				    savereg[core_count].reg_addr =
				      ACPHYREGCE(pi, RfctrlOverrideTxPus, core_off);
				    ++core_count;
				    savereg[core_count].reg_val =
				      READ_PHYREGCE(pi, RfctrlCoreTxPus, core_off);
				    savereg[core_count].reg_addr =
				      ACPHYREGCE(pi, RfctrlCoreTxPus, core_off);
				    ++core_count;
				    MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core_off, txrf_pwrup, 1);
				    MOD_PHYREGCE(pi, RfctrlCoreTxPus, core_off, txrf_pwrup, 0);
				}
			    }
			}

			cmd = cmds[cmd_idx % num_cmds_per_core] | 0x8000 | (core << 12);
			cal_type = ((cmd & 0x0F00) >> 8);

			if ((ACMAJORREV_32(pi->pubpi.phy_rev) ||
			     ACMAJORREV_33(pi->pubpi.phy_rev)) && CHSPEC_IS80(pi->radio_chanspec)) {
				fdiq_loop = (cmds[cmd_idx % num_cmds_per_core] & 0xF00) >> 8;
			if (fdiq_loop == 1) {
				if ((fdiq_data_valid & (1 << core)) == 0) {
					fdiq_data_valid |= 1 << core;
				}
				cmd = (cmds[cmd_idx % num_cmds_per_core] & 0xFF) | 0x8000 |
				        (core << 12);
				cal_type = 0;
				sweep_tone = ACPHY_TXCAL_MAX_NUM_FREQ;
			}

			}

			/* PHY_CAL(("wlc_phy_cal_txiqlo_acphy:
			 *  Cmds => cmd_idx=%2d, Cmd=0x%04x,		\
			 *  cal_type=%d, core=%d\n", cmd_idx, cmd, cal_type, core));
			 */

			for (tone_idx = 0; tone_idx < sweep_tone; tone_idx++) {
			if ((ACMAJORREV_32(pi->pubpi.phy_rev) ||
			     ACMAJORREV_33(pi->pubpi.phy_rev)) && CHSPEC_IS80(pi->radio_chanspec)) {

				if (fdiq_loop == 1) {
					tone = (int16)tx_fdiq_tone_list[tone_idx];
					tone = (tone * 1000) >> 1;
					bcmerror = wlc_phy_tx_tone_acphy(pi, (int32)tone, tone_ampl,
					                                 1, 0, FALSE);
					tfreq_ang_mag[tone_idx].freq =
					        (int32)tx_fdiq_tone_list[tone_idx];
				}
			}

			/* set up scaled ladders for desired bbmult of current core */
			if (!accal->txiqlocal_ladder_updated[core]) {
				if (TINY_RADIO(pi)) {
					wlc_phy_cal_txiqlo_update_ladder_acphy(pi,
						accal->txcal_txgain[core].bbmult, core);
				} else {
					wlc_phy_cal_txiqlo_update_ladder_acphy(pi,
						accal->txcal_txgain[core].bbmult, core);
					accal->txiqlocal_ladder_updated[core] = TRUE;
				}
				accal->txiqlocal_ladder_updated[core] = TRUE;
			}

			/* set intervals settling and measurement intervals */
			WRITE_PHYREG(pi, iqloCalCmdNnum,
			             (nsamp_corrs[bw_idx] << 8) | nsamp_gctrl[bw_idx]);

			/* if coarse-analog-LOFT cal (fi/fq),
			 *     always zero out ei/eq and di/dq;
			 * if fine-analog-LOFT   cal (ei/dq),
			 *     always zero out di/dq
			 *   - even do this with search-type REFINE, to prevent a "drift"
			 *   - assumes that order of LOFT cal cmds will be f => e => d,
			 *     where it's ok to have multiple cmds (say interrupted by
			 *     IQ cal) of the same type
			 */
			if ((cal_type == CAL_TYPE_LOFT_ANA_COARSE) ||
			    (cal_type == CAL_TYPE_LOFT_ANA_FINE)) {
				if (!(ACMAJORREV_1(pi->pubpi.phy_rev) && (cmd_idx >= 5))) {
					wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
						&zero, TB_START_COEFFS_D, core);
				}
			}

			if (cal_type == CAL_TYPE_LOFT_ANA_COARSE) {
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					&zero, TB_START_COEFFS_E, core);
			}

			for (thidx = 0; thidx < 6; thidx++) {
				/* Set thresh_d2 */
				WRITE_PHYREG(pi, iqloCalgtlthres, thres_ladder[thidx]);

				/* now execute this command and wait max of ~20ms */
				if (ACMAJORREV_4(pi->pubpi.phy_rev) &&
				    phy_get_phymode(pi) != PHYMODE_RSDB) {
					if (core == 1) {
						WRITE_PHYREG(pi, iqloCalCmd, cmd);
					} else {
						wlapi_exclusive_reg_access_core0(
							pi->sh->physhim, 1);
						WRITE_PHYREG(pi, iqloCalCmd, cmd);
						wlapi_exclusive_reg_access_core0(
							pi->sh->physhim, 0);
					}
				} else {

					stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
					ACPHY_DISABLE_STALL(pi);
					WRITE_PHYREG(pi, iqloCalCmd, cmd);
					ACPHY_ENABLE_STALL(pi, stall_val);

				}
				SPINWAIT(((READ_PHYREG(pi, iqloCalCmd)
				           & 0xc000) != 0), ACPHY_SPINWAIT_TXIQLO);
				ASSERT((READ_PHYREG(pi, iqloCalCmd) & 0xc000) == 0);

				PHY_CAL(("wlc_phy_cal_txiqlo_acphy: Cmds => cmd_idx=%2d,",
				         cmd_idx));
				PHY_CAL(("Cmd=0x%04x, cal_type=%d, core=%d, ",
				         cmd, cal_type, core));
				PHY_CAL(("thresh_idx = %d\n", thidx));

				if (ACMAJORREV_4(pi->pubpi.phy_rev) ||
				    ACMAJORREV_32(pi->pubpi.phy_rev) ||
				    ACMAJORREV_33(pi->pubpi.phy_rev)) {
					break;
				} else if (wlc_poll_adc_clamp_status(pi, core, 1) == 0)  {
					break;
				}

				PHY_CAL(("wlc_phy_cal_txiqlo_acphy: Cmds => cmd_idx=%2d,",
				         cmd_idx));
				PHY_CAL(("Cmd=0x%04x, cal_type=%d, core=%d, ",
				         cmd, cal_type, core));
				PHY_CAL(("thresh_idx = %d\n", thidx));
			}

			/* copy coeffs best-to-start and to
			 * "intermediate" coeffs in pi state; in mphase,
			 * the latter is also used as starting point
			 * when coming back for next phase, and
			 * we always use the "intermediate" coeffs at
			 * the very end to apply to OFDM/BPHY,
			 * see below;
			 * (copy step only done for coeff pair that
			 * changed, thereby also covering ei/eq swap
			 * per PR 79353)
			 */
			switch (cal_type) {
			case CAL_TYPE_IQ:
				rd_select  = TB_BEST_COEFFS_AB;
				wr_select1 = TB_START_COEFFS_AB;
				wr_select2 = PI_INTER_COEFFS_AB;
				break;
			case CAL_TYPE_LOFT_DIG:
				rd_select  = TB_BEST_COEFFS_D;
				wr_select1 = TB_START_COEFFS_D;
				wr_select2 = PI_INTER_COEFFS_D;
				break;
			case CAL_TYPE_LOFT_ANA_FINE:
				rd_select  = TB_BEST_COEFFS_E;
				wr_select1 = TB_START_COEFFS_E;
				wr_select2 = PI_INTER_COEFFS_E;
				break;
			case CAL_TYPE_LOFT_ANA_COARSE:
				rd_select  = TB_BEST_COEFFS_F;
				wr_select1 = TB_START_COEFFS_F;
				wr_select2 = PI_INTER_COEFFS_F;
				break;
			default:
				ASSERT(0);
			}
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
			                                coeffs, rd_select,  core);

			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
			                                coeffs, wr_select1, core);
			if (ACREV_IS(pi->pubpi.phy_rev, 1) &&
			    ((cmd_idx % num_cmds_per_core) >= idx_for_loft_comp_tbl)) {
				/* write to the txpwrctrl tbls */
				*loft_coeffs_ptr++ = coeffs[0];
			}
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
			                                coeffs, wr_select2, core);

			if ((ACMAJORREV_32(pi->pubpi.phy_rev) ||
			     ACMAJORREV_33(pi->pubpi.phy_rev)) && CHSPEC_IS80(pi->radio_chanspec)) {
				if (cal_type == CAL_TYPE_IQ) {
				tmp.q = (int32)(int16)coeffs[0];
				tmp.i = (int32)((int16)coeffs[1]+1024);
				phy_utils_invcordic(tmp, &ang);
				phy_utils_cordic(ang, &tmp2);
				if (fdiq_loop == 1) {
					tfreq_ang_mag[tone_idx].angle[core] = ang;
					tfreq_ang_mag[tone_idx].mag[core] =
					        ((1024 + (int16)coeffs[1])<<16)/tmp2.i;
				}
				}
			}

			/* WAR for random spur issue in 1x1 which */
			/* happens when LOFT coeff is negative */
			if ((ACMAJORREV_1(pi->pubpi.phy_rev) ||
			     ACMAJORREV_2(pi->pubpi.phy_rev)) && (cmd_idx == 4)) {
				uint16 coeffs_digi[2];
				coeffs_digi[0] = 0x0808;
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs_digi, TB_BEST_COEFFS_D, core);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs_digi, TB_START_COEFFS_D, core);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs_digi, PI_INTER_COEFFS_D, core);
			}

			if (ACMAJORREV_5(pi->pubpi.phy_rev)) {
				/* Restore */
				while (core_count > 0) {
					--core_count;
					phy_utils_write_phyreg(pi, savereg[core_count].reg_addr,
					                       savereg[core_count].reg_val);
				}
			}
			if ((ACMAJORREV_32(pi->pubpi.phy_rev) ||
			     ACMAJORREV_33(pi->pubpi.phy_rev)) && CHSPEC_IS80(pi->radio_chanspec)) {
			/* Restart tone for CAL on core1 after fdiq cal on core0
			 * stopplayback below will stop for last core
			 */
				if ((tone_idx == (sweep_tone -1) && fdiq_loop == 1)) {
					bcmerror = wlc_phy_tx_tone_acphy(pi, (int32)tone_freq,
					                                 tone_ampl, 1, 0, FALSE);
					OSL_DELAY(5);
				}
			}
			} /* sweep tone loop */
		} /* command loop */

		/* single phase or last tx stage in multiphase cal: apply & store overall results */
		if (Biq2byp == 0) {
		last_phase = !ACREV_IS(pi->pubpi.phy_rev, 1) ?
			ACPHY_CAL_PHASE_TX8 : ACPHY_CAL_PHASE_TX_LAST;
		if ((mphase == 0) || (pi->cal_info->cal_phase_id == last_phase)) {

			PHY_CAL(("wlc_phy_cal_txiqlo_acphy (mphase = %d, refine = %d):\n",
			         mphase, searchmode == PHY_CAL_SEARCHMODE_REFINE));
			for (cr = 0; cr < num_cores; cr++) {
				/* Save and Apply IQ Cal Results */
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_AB, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_AB, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_OFDM_COEFFS_AB,  cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_BPHY_COEFFS_AB,  cr);

				/* Save and Apply Dig LOFT Cal Results */
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_D, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_D, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_OFDM_COEFFS_D,  cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, TB_BPHY_COEFFS_D,  cr);

				/* Apply Analog LOFT Comp
				 * - unncessary if final command on each core is digital
				 * LOFT-cal or IQ-cal
				 * - then the loft comp coeffs were applied to radio
				 * at the beginning of final command per core
				 * - this is assumed to be the case, so nothing done here
				 */

				/* Save Analog LOFT Comp in PI State */
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_E, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_E, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_F, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE,
					coeffs, PI_FINAL_COEFFS_F, cr);

				/* Print out Results */
				PHY_CAL(("\tcore-%d: a/b = (%4d,%4d), d = (%4d,%4d),"
					" e = (%4d,%4d), f = (%4d,%4d)\n", cr,
					accal->txiqlocal_coeffs[cr*5+0],  /* a */
					accal->txiqlocal_coeffs[cr*5+1],  /* b */
					(accal->txiqlocal_coeffs[cr*5+2] & 0xFF00) >> 8, /* di */
					(accal->txiqlocal_coeffs[cr*5+2] & 0x00FF),      /* dq */
					(accal->txiqlocal_coeffs[cr*5+3] & 0xFF00) >> 8, /* ei */
					(accal->txiqlocal_coeffs[cr*5+3] & 0x00FF),      /* eq */
					(accal->txiqlocal_coeffs[cr*5+4] & 0xFF00) >> 8, /* fi */
					(accal->txiqlocal_coeffs[cr*5+4] & 0x00FF)));   /* fq */
			} /* for cr */

			/* validate availability of results and store off channel */
			accal->txiqlocal_coeffsvalid = TRUE;
			accal->chanspec = pi->radio_chanspec;
		} /* writing of results */
		    } else {
		last_phase = ACPHY_CAL_PHASE_TXPRERXCAL2;
		if ((mphase == 0) || (pi->cal_info->cal_phase_id == last_phase)) {

			PHY_CAL(("wlc_phy_cal_txiqlo_acphy (Biq2byp = %d, mphase = %d,"
			    " refine = %d):\n", Biq2byp, mphase,
			    searchmode == PHY_CAL_SEARCHMODE_REFINE));
			for (cr = 0; cr < num_cores; cr++) {
				/* Save IQ Cal coeffs for RX-cal */
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
					coeffs, PI_INTER_COEFFS_AB, cr);
				wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_WRITE_BIQ2BYP,
					coeffs, PI_INTER_COEFFS_AB, cr);

				/* Print out Results */
				PHY_CAL(("\tcore-%d: a/b = (%4d,%4d),\n", cr,
					accal->txiqlocal_biq2byp_coeffs[cr*2+0],   /* a */
					accal->txiqlocal_biq2byp_coeffs[cr*2+1])); /* b */
			} /* for cr */

			/* validate availability of results and store off channel */
			accal->txiqlocal_coeffsvalid = TRUE;
			accal->chanspec = pi->radio_chanspec;
		} /* writing of results */
		    }

		/* Switch off test tone */
		wlc_phy_stopplayback_acphy(pi);	/* mimophy_stop_playback */

	} /* if BCME_OK */

	/* disable IQ/LO cal */
	WRITE_PHYREG(pi, iqloCalCmdGctl, 0x0000);

	/* 4350A0 FIXME: Add Jira
	 * Remove forcing of gated clks
	 */
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);

	/* LOFT WAR for 4360 and 43602 */
	if (ACREV_IS(pi->pubpi.phy_rev, 1) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
		wlc_phy_populate_tx_loft_comp_tbl_acphy(pi, loft_coeffs);
	}

	/* clean Up PHY and radio */
	wlc_phy_txcal_txgain_cleanup_acphy(pi, &orig_txgain[0]);
	wlc_phy_txcal_phy_cleanup_acphy(pi);
	if (TINY_RADIO(pi))
		wlc_phy_txcal_radio_cleanup_acphy_tiny(pi);
	else
		wlc_phy_txcal_radio_cleanup_acphy(pi);
	WRITE_PHYREG(pi, ClassifierCtrl, classifier_state);

	if ((ACMAJORREV_32(pi->pubpi.phy_rev) ||
	     ACMAJORREV_33(pi->pubpi.phy_rev)) && CHSPEC_IS80(pi->radio_chanspec)) {
		if (fdiq_data_valid != 0) {
			wlc_phy_fdiqi_lin_reg_acphy(pi, tfreq_ang_mag,
			                            ACPHY_TXCAL_MAX_NUM_FREQ, fdiq_data_valid);
		}
	}

	/* XXX FIXME: May consider saving off LOFT comp before 1st phase and
	 *  restoring LOFT comp  after each phase except for last phase
	 */

	/*
	 *-----------*
	 *  Cleanup  *
	 *-----------
	 */

	/* prevent crs trigger */
	wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);

	/* Restore PAPD state */
	FOREACH_CORE(pi, core) {
		WRITE_PHYREGCE(pi, PapdEnable, core, papdState[core]);
	}
	return bcmerror;
}

void
wlc_phy_cal_txiqlo_coeffs_acphy(phy_info_t *pi, uint8 rd_wr, uint16 *coeff_vals,
                                uint8 select, uint8 core) {
	uint8 iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, core);
	uint8 boffs_tmp = 0;

	/* handles IQLOCAL coefficients access (read/write from/to
	 * iqloCaltbl and pi State)
	 *
	 * not sure if reading/writing the pi state coeffs via this appraoch
	 * is a bit of an overkill
	 */

	/* {num of 16b words to r/w, start offset (ie address), core-to-core block offset} */
	acphy_coeff_access_t coeff_access_info[] = {
		{2, 64, 8},  /* TB_START_COEFFS_AB   */
		{1, 67, 8},  /* TB_START_COEFFS_D    */
		{1, 68, 8},  /* TB_START_COEFFS_E    */
		{1, 69, 8},  /* TB_START_COEFFS_F    */
		{2, 128, 7}, /*   TB_BEST_COEFFS_AB  */
		{1, 131, 7}, /*   TB_BEST_COEFFS_D   */
		{1, 132, 7}, /*   TB_BEST_COEFFS_E   */
		{1, 133, 7}, /*   TB_BEST_COEFFS_F   */
		{2, 96,  4}, /* TB_OFDM_COEFFS_AB    */
		{1, 98,  4}, /* TB_OFDM_COEFFS_D     */
		{2, 112, 4}, /* TB_BPHY_COEFFS_AB    */
		{1, 114, 4}, /* TB_BPHY_COEFFS_D     */
		{2, 0, 5},   /*   PI_INTER_COEFFS_AB */
		{1, 2, 5},   /*   PI_INTER_COEFFS_D  */
		{1, 3, 5},   /*   PI_INTER_COEFFS_E  */
		{1, 4, 5},   /*   PI_INTER_COEFFS_F  */
		{2, 0, 5},   /* PI_FINAL_COEFFS_AB   */
		{1, 2, 5},   /* PI_FINAL_COEFFS_D    */
		{1, 3, 5},   /* PI_FINAL_COEFFS_E    */
		{1, 4, 5}    /* PI_FINAL_COEFFS_F    */
	};
	acphy_cal_result_t *accal = &pi->cal_info->u.accal;

	uint8 nwords, offs, boffs, k;

	/* get access info for desired choice */
	nwords = coeff_access_info[select].nwords;
	offs   = coeff_access_info[select].offs;
	boffs  = coeff_access_info[select].boffs;

	boffs_tmp = boffs*core;
	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		boffs_tmp = 0;
	}

	/* read or write given coeffs */
	if (select <= TB_BPHY_COEFFS_D) { /* START and BEST coeffs in Table */
		if (rd_wr == CAL_COEFF_READ) { /* READ */
			wlc_phy_table_read_acphy(pi, iqlocal_tbl_id, nwords,
				offs + boffs_tmp, 16, coeff_vals);
		} else { /* WRITE */
			wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, nwords,
				offs + boffs_tmp, 16, coeff_vals);
		}
	} else if (select <= PI_INTER_COEFFS_F) { /* PI state intermediate coeffs */
		for (k = 0; k < nwords; k++) {
			if (rd_wr == CAL_COEFF_READ) { /* READ */
				coeff_vals[k] = accal->txiqlocal_interm_coeffs[offs +
				                                               boffs*core + k];
			} else if (rd_wr == CAL_COEFF_WRITE_BIQ2BYP) { /* store for rx cal */
				accal->txiqlocal_biq2byp_coeffs[nwords*core + k] = coeff_vals[k];
			} else { /* WRITE */
				accal->txiqlocal_interm_coeffs[offs +
				                               boffs*core + k] = coeff_vals[k];
			}
		}
	} else { /* PI state final coeffs */
		for (k = 0; k < nwords; k++) { /* PI state final coeffs */
			if (rd_wr == CAL_COEFF_READ) { /* READ */
				coeff_vals[k] = accal->txiqlocal_coeffs[offs + boffs*core + k];
			} else { /* WRITE */
				accal->txiqlocal_coeffs[offs + boffs*core + k] = coeff_vals[k];
			}
		}
	}
}

void
wlc_phy_ipa_set_bbmult_acphy(phy_info_t *pi, uint16 *m0, uint16 *m1, uint16 *m2,
		uint16 *m3, uint8 coremask)
{
	/* TODO: 4360 */
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	uint8 iqlocal_tbl_id;

	ACPHY_DISABLE_STALL(pi);

	iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, 0);

	if (PHYCOREMASK(coremask) & 0x1) {
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 99, 16, m0);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 115, 16, m0);
	}

	if ((PHYCOREMASK(coremask) >> 1) && 0x1) {
		if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
			iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, 1);
			wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 99, 16, m1);
			wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 115, 16, m1);
		} else {
			wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 103, 16, m1);
			wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 119, 16, m1);
		}
	}

	if ((PHYCOREMASK(coremask) >> 2) & 0x1) {
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 107, 16, m2);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 123, 16, m2);
	}

	if ((PHYCOREMASK(coremask) >>3) & 0x1) {
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 111, 16, m3);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, 127, 16, m3);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_acphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b)
{
	uint16 iqcc[2];

	wlc_phy_table_read_acphy(pi, wlc_phy_get_tbl_id_iqlocal(pi, 0), 2,
		tbl_offset_ofdm_a[0], 16, &iqcc);

	*a = iqcc[0];
	*b = iqcc[1];
}

void
wlc_acphy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b)
{
	uint16 iqcc[2];
	uint8 core = 0;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	uint8 iqlocal_tbl_id;

	iqcc[0] = a;
	iqcc[1] = b;

	ACPHY_DISABLE_STALL(pi);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, core);

		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 2, tbl_offset_ofdm_a[0], 16, iqcc);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 2, tbl_offset_bphy_a[0], 16, iqcc);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

uint16
wlc_acphy_get_tx_locc(phy_info_t *pi)
{
	uint16 didq;

	wlc_phy_table_read_acphy(pi, wlc_phy_get_tbl_id_iqlocal(pi, 0), 1,
		tbl_offset_ofdm_d[0], 16, &didq);
	return didq;
}

void
wlc_acphy_set_tx_locc(phy_info_t *pi, uint16 didq)
{
	uint8 core = 0;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	uint8 iqlocal_tbl_id;

	ACPHY_DISABLE_STALL(pi);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		iqlocal_tbl_id = wlc_phy_get_tbl_id_iqlocal(pi, core);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, tbl_offset_ofdm_d[0], 16, &didq);
		wlc_phy_table_write_acphy(pi, iqlocal_tbl_id, 1, tbl_offset_bphy_d[0], 16, &didq);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_phy_poll_samps_WAR_acphy(phy_info_t *pi, int16 *samp, bool is_tssi,
                             bool for_idle, txgain_setting_t *target_gains,
                             bool for_iqcal, bool init_adc_inside, uint16 ADCcore, bool champ)
{
	uint8 core;
	uint16 save_afePuCtrl = 0, save_gpio = 0, save_gpioHiOutEn = 0;
	uint16 txgain1_save[PHY_CORE_MAX] = {0};
	uint16 txgain2_save[PHY_CORE_MAX] = {0};
	uint16 dacgain_save[PHY_CORE_MAX] = {0};
	uint16 bq2gain_save[PHY_CORE_MAX] = {0};
	uint16 overridegains_save[PHY_CORE_MAX] = {0};
	uint16 overridevlin_save[PHY_CORE_MAX] = {0};
	uint16 overridevlin2_save[PHY_CORE_MAX] = {0};
	uint16 orig_OVR10[PHY_CORE_MAX] = {0};
	uint16 orig_LPF_MAIN_CONTROLS[PHY_CORE_MAX] = {0};
	uint16 fval2g_orig, fval5g_orig, fval2g, fval5g;
	uint32 save_chipc = 0;
	uint8  stall_val = 0, log2_nsamps = 0;
	uint16 bbmult_save[PHY_CORE_MAX];
	uint16 bbmult, txgain1, txgain2, lpf_gain, dac_gain, vlin_val;
	uint16 bq1_gain_addr[3] = {0x17e, 0x18e, 0x19e}, bq1_gain;

	if (init_adc_inside) {
		wlc_phy_init_adc_read(pi, &save_afePuCtrl, &save_gpio,
		                      &save_chipc, &fval2g_orig, &fval5g_orig,
		                      &fval2g, &fval5g, &stall_val, &save_gpioHiOutEn);
	}

	if (is_tssi) {
		ACPHY_DISABLE_STALL(pi);
		/* Save gain for all Tx cores */
		/* Set TX gain to 0, so that LO leakage does not affect IDLE TSSI */
		FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
			wlc_phy_get_tx_bbmult_acphy(pi, &(bbmult_save[core]), core);
			dacgain_save[core] = READ_PHYREGCE(pi, Dac_gain, core);
			txgain1_save[core] = READ_PHYREGCE(pi, RfctrlCoreTXGAIN1, core);
			txgain2_save[core] = READ_PHYREGCE(pi, RfctrlCoreTXGAIN2, core);
			bq2gain_save[core] = READ_PHYREGCE(pi, RfctrlCoreLpfGain, core);
			overridegains_save[core] = READ_PHYREGCE(pi, RfctrlOverrideGains, core);
			overridevlin_save[core] = READ_PHYREGCE(pi, RfctrlOverrideAuxTssi, core);
			overridevlin2_save[core] = READ_PHYREGCE(pi, RfctrlCoreAuxTssi1, core);
		}
		if (for_idle) {
			/* This is to measure the idle tssi */
			bbmult   = 0;
			txgain1  = 0;
			txgain2  = 0;
			lpf_gain = 0;
			dac_gain = 0;
		} else {
			/* This is to measure the tone tssi */
			bbmult   = target_gains->bbmult;
			txgain1  = ((target_gains->rad_gain & 0xFF00) >> 8) |
				((target_gains->rad_gain_mi & 0x00FF) << 8);
			txgain2  = ((target_gains->rad_gain_mi & 0xFF00) >> 8) |
				((target_gains->rad_gain_hi & 0x00FF) << 8);
			lpf_gain = (target_gains->rad_gain & 0xF0) >> 4;
			dac_gain = (target_gains->rad_gain & 0x0F) >> 0;
			if (BF3_VLIN_EN_FROM_NVRAM(pi->u.pi_acphy)) {
				vlin_val =  (target_gains->rad_gain & 0x00F0) >> 7;
				MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, tx_vlin_ovr, 1);
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, tx_vlin, vlin_val);
			}

		}

		if ((RADIOID(pi->pubpi.radioid) == BCM2069_ID) ||
			(RADIOID(pi->pubpi.radioid) == BCM20691_ID)) {
			/* set same gain for all cores */
			FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
				WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN1, core, txgain1);
				WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN2, core, txgain2);
				WRITE_PHYREGCE(pi, Dac_gain, core, dac_gain);
				MOD_PHYREGCE(pi, RfctrlCoreLpfGain, core, lpf_bq2_gain, lpf_gain);
				MOD_PHYREGCE(pi, RfctrlOverrideGains, core, txgain, 1);
				MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 1);
				wlc_phy_set_tx_bbmult_acphy(pi, &bbmult, core);

				if (!TINY_RADIO(pi)) {
					/* Enforce lpf_bq1_gain */
					orig_LPF_MAIN_CONTROLS[core] =
						READ_RADIO_REGC(pi, RF, LPF_MAIN_CONTROLS, core);
					if (RADIOMAJORREV(pi) > 0) {
						orig_OVR10[core] = READ_RADIO_REGC(pi, RF,
							GE16_OVR11, core);
					} else {
						orig_OVR10[core] = READ_RADIO_REGC(pi, RF,
							OVR10, core);
					}
					wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
						bq1_gain_addr[core], 16, &bq1_gain);
					bq1_gain = bq1_gain & 0x7; /* take the 3LSB */
					MOD_RADIO_REGC(pi, LPF_MAIN_CONTROLS, core,
						lpf_bq1_gain, bq1_gain);
					if (RADIOMAJORREV(pi) > 0) {
						MOD_RADIO_REGC(pi, GE16_OVR11, core,
							ovr_lpf_bq1_gain, 1);
					} else {
						MOD_RADIO_REGC(pi, OVR10, core,
							ovr_lpf_bq1_gain, 1);
					}
				}
			}
		}

		ACPHY_ENABLE_STALL(pi, stall_val);

		/* Enable WLAN priority */
		wlc_btcx_override_enable(pi);

		OSL_DELAY(100);
		if (for_idle) {
			wlc_phy_tx_tone_acphy(pi, 2000, 0, 0, 0, FALSE);
		} else {
			if (champ)
			{
				wlc_phy_tx_tone_acphy(pi, 2000, 120, 0, 0, FALSE);
			}
			else
			{
				wlc_phy_tx_tone_acphy(pi, 2000, 181, 0, 0, FALSE);
			}
		}
		OSL_DELAY(100);

		/* Taking a 256-samp average for 80mHz idle-tssi measuring.
		 * Note: ideally, we can apply the same averaging for 20/40mhz also,
		 *       but we don't want to change the existing 20/40mhz behavior to reduce risk.
		 */
		log2_nsamps = (for_iqcal || ACMAJORREV_32(pi->pubpi.phy_rev) ||
				ACMAJORREV_33(pi->pubpi.phy_rev)) ? 3 :
				(CHSPEC_IS80(pi->radio_chanspec)) ? 8 : 0;

		if (champ)
			log2_nsamps = 4;
		wlc_phy_poll_samps_acphy(pi, samp, TRUE, log2_nsamps, init_adc_inside, ADCcore);
		wlc_phy_stopplayback_acphy(pi);

		/* Disable WLAN priority */
		wlc_phy_btcx_override_disable(pi);
	} else {
		wlc_phy_poll_samps_acphy(pi, samp, FALSE, 3, init_adc_inside, ADCcore);
	}

	if (is_tssi) {
		if ((RADIOID(pi->pubpi.radioid) == BCM2069_ID) ||
			(RADIOID(pi->pubpi.radioid) == BCM20691_ID)) {
			FOREACH_ACTV_CORE(pi, pi->sh->hw_phyrxchain, core) {
				/* Remove TX gain & lpf_bq1 gain override */
				WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN1, core, txgain1_save[core]);
				WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN2, core, txgain2_save[core]);
				WRITE_PHYREGCE(pi, Dac_gain, core, dacgain_save[core]);
				WRITE_PHYREGCE(pi, RfctrlOverrideGains, core,
					overridegains_save[core]);
				WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core, bq2gain_save[core]);
				WRITE_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
					overridevlin_save[core]);
				WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi1, core,
					overridevlin2_save[core]);
				wlc_phy_set_tx_bbmult_acphy(pi, &(bbmult_save[core]), core);
				if (!TINY_RADIO(pi)) {
					phy_utils_write_radioreg(pi,
					                         RF_2069_LPF_MAIN_CONTROLS(core),
					                         orig_LPF_MAIN_CONTROLS[core]);
					if (RADIOMAJORREV(pi) > 0) {
						phy_utils_write_radioreg(pi,
						                         RF_2069_GE16_OVR11(core),
						                         orig_OVR10[core]);
					} else {
						phy_utils_write_radioreg(pi, RF_2069_OVR10(core),
							orig_OVR10[core]);
					}
				}
			}
		}
	}
	if (init_adc_inside) {
		wlc_phy_restore_after_adc_read(pi,  &save_afePuCtrl, &save_gpio,
		                               &save_chipc,  &fval2g_orig,  &fval5g_orig,
		                               &fval2g,  &fval5g, &stall_val, &save_gpioHiOutEn);
	}
}
