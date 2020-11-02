/*
 * ACPHY RSSI Compute module implementation
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
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <qmath.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rssi.h"
#include <phy_ac.h>
#include <phy_ac_rssi.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_ac.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_ac_rssi_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_rssi_info_t *ri;
	int8 gain_err[PHY_CORE_MAX];
};

/* local functions */
static void phy_ac_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);
static uint8 phy_ac_rssi_11b_WAR(phy_ac_info_t *aci, d11rxhdr_t *rxh);
static void _phy_ac_rssi_init_gain_err(phy_type_rssi_ctx_t *ctx);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int phy_ac_rssi_dump(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ac_rssi_dump NULL
#endif // endif

/* register phy type specific implementation */
phy_ac_rssi_info_t *
BCMATTACHFN(phy_ac_rssi_register_impl)(phy_info_t *pi, phy_ac_info_t *aci, phy_rssi_info_t *ri)
{
	phy_ac_rssi_info_t *info;
	phy_type_rssi_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_ac_rssi_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->aci = aci;
	info->ri = ri;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.compute = phy_ac_rssi_compute;
	fns.init_gain_err = _phy_ac_rssi_init_gain_err;
	fns.dump = phy_ac_rssi_dump;
	fns.ctx = info;

	phy_rssi_register_impl(ri, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_ac_rssi_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_rssi_unregister_impl)(phy_ac_rssi_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_rssi_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_rssi_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_ac_rssi_info_t));
}

/* calculate rssi */
static void BCMFASTPATH
phy_ac_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh)
{
	phy_ac_rssi_info_t *info = (phy_ac_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_ac_info_t *aci = info->aci;
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	int16 rxpwr;
	int16 rxpwr_core[PHY_CORE_MAX];
	int8 int8_rxpwr_core[PHY_CORE_MAX];
	int16 is_status_hacked;
	uint8 core, ant;
	int8  bw_idx, subband_idx;
	int16 gain_err_temp_adj_for_rssi;

	/* mode = 0: rxpwr = max(rxpwr0, rxpwr1)
	 * mode = 1: rxpwr = min(rxpwr0, rxpwr1)
	 * mode = 2: rxpwr = (rxpwr0+rxpwr1)/2
	 */
	bzero(int8_rxpwr_core, sizeof(int8)*PHY_CORE_MAX);
	int8_rxpwr_core[0] = (int8)ACPHY_RXPWR_ANT0(rxh);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 1)
		int8_rxpwr_core[1] = (int8)ACPHY_RXPWR_ANT1(rxh);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 2)
		int8_rxpwr_core[2] = (int8)ACPHY_RXPWR_ANT2(rxh);
	if (PHYCORENUM(pi->pubpi.phy_corenum) > 3)
		int8_rxpwr_core[3] = (int8)ACPHY_RXPWR_ANT4(rxh);
	is_status_hacked = ACPHY_HACK_PWR_STATUS(rxh);

	FOREACH_CORE(pi, core) {
		rxpwr_core[core] = (int16)int8_rxpwr_core[core];
	}

	if ((ACMAJORREV_1(pi->pubpi.phy_rev)) && (is_status_hacked == 1)) {
		FOREACH_CORE(pi, core) {
			rxpwr_core[core] = WLC_RSSI_INVALID;
		}
		rxpwr_core[0] = phy_ac_rssi_11b_WAR(aci, rxh);
	}

	/* Sign extend */
	FOREACH_CORE(pi, core) {
	  if (rxpwr_core[core] > 127)
	    rxpwr_core[core] -= 256;
	}
	/*
	 * 22nd Oct
	 * if (rxgaincal_rssical == false)
	 * 	include the current implementation
	 * else (rxgaincal_rssical == true)
	 * 	have only rssi_gain_cal
	 * 	in true condition
	 * 	if rssi_cal_rev == 1
	 *		use gain_cal_temp
	 * 	else
	 * 		use raw_tempsense (as it is currently
	 *              being populated for rssigain delta calibration)
	 * 	fi
	 * fi
	 * Eventually, rssi_cal_rev == 0 condition has to be deprecated.
	 */
	if (aci->rxgaincal_rssical == FALSE) {
		wlc_phy_upd_gain_wrt_temp_phy(pi, &gain_err_temp_adj_for_rssi);

		/* Apply gain-error correction with temperature compensation: */
		FOREACH_CORE(pi, core) {
			if (rxpwr_core[core] != WLC_RSSI_INVALID) {
				int16 tmp;

				tmp = info->gain_err[core] * 2 -
				    gain_err_temp_adj_for_rssi;
				tmp = ((tmp >= 0) ? ((tmp + 2) >> 2) : -1 *
				 ((-1 * tmp + 2) >> 2));
				  rxpwr_core[core] -= tmp;
			}
		}
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			bw_idx = (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		} else {
			bw_idx = (CHSPEC_IS80(pi->radio_chanspec) ||
				CHSPEC_IS160(pi->radio_chanspec))
				? 2 : (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		}

		/* Apply nvram based offset: */
		FOREACH_CORE(pi, core) {
		ant = phy_get_rsdbbrd_corenum(pi, core);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				rxpwr_core[core] +=
				aci->sromi->rssioffset.rssi_corr_normal[ant][bw_idx];
				rxpwr_core[core] +=
				aci->sromi->rssioffset.rssi_corr_gain_delta_2g[core][0][bw_idx];
			} else {
				uint8 bands[NUM_CHANS_IN_CHAN_BONDING];
				if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
					wlc_phy_get_chan_freq_range_80p80_acphy(pi, 0, bands);
					if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
						subband_idx = (core <= 1) ? (bands[0] - 1)
							: (bands[1] - 1);
					} else {
						subband_idx = bands[0] - 1;
						ASSERT(0); // FIXME
					}
				} else {
					subband_idx = wlc_phy_get_chan_freq_range_acphy(pi,
							pi->radio_chanspec)-1;
				}

				  if (!ACMAJORREV_5(pi->pubpi.phy_rev)) {
				  rxpwr_core[core] +=
				    aci->sromi->rssioffset.rssi_corr_normal_5g[ant][subband_idx]
				    [bw_idx];
				  }
				  rxpwr_core[core] +=
				    aci->sromi->rssioffset.rssi_corr_gain_delta_5g
				    [core][0][bw_idx][subband_idx];
			}
		}

	} else {
		uint8 bands[NUM_CHANS_IN_CHAN_BONDING];

		if (aci->rssi_cal_rev == FALSE) {
			wlc_phy_upd_gain_wrt_temp_phy(pi, &gain_err_temp_adj_for_rssi);
		} else {
			wlc_phy_upd_gain_wrt_gain_cal_temp_phy(pi, &gain_err_temp_adj_for_rssi);
		}

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			bw_idx = (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		} else {
			bw_idx = (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) ? 2 :
				CHSPEC_IS160(pi->radio_chanspec) ? 3 :
				(CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		}

		/* Apply nvram based offset: */
		FOREACH_CORE(pi, core) {

			if (aci->rssi_cal_rev == FALSE) {
				if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
					wlc_phy_get_chan_freq_range_80p80_acphy(pi, 0, bands);
					if (ACMAJORREV_33(pi->pubpi.phy_rev)) {
						subband_idx = (core <= 1) ? (bands[0] - 1)
							: (bands[1] - 1);
					} else {
						subband_idx = bands[0] - 1;
						ASSERT(0); // FIXME
					}
				} else {
					subband_idx = wlc_phy_get_chan_freq_range_acphy(pi,
							pi->radio_chanspec)-1;
				}
			} else {
				subband_idx = wlc_phy_rssi_get_chan_freq_range_acphy(pi, core);
			}

			ant = phy_get_rsdbbrd_corenum(pi, core);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
			  int8 rssi_corr_gain_delta_2g;
			  if (aci->rssi_cal_rev == FALSE) {
			    rssi_corr_gain_delta_2g = aci->sromi->rssioffset
			      .rssi_corr_gain_delta_2g[core][0][bw_idx];
			  } else {
			    rssi_corr_gain_delta_2g = aci->sromi->rssioffset
			      .rssi_corr_gain_delta_2g_sub[core][0][bw_idx][subband_idx];
			  }
			  rxpwr_core[core] +=
			    aci->sromi->rssioffset.rssi_corr_normal[ant][bw_idx];
			  rxpwr_core[core] += rssi_corr_gain_delta_2g - gain_err_temp_adj_for_rssi;
			} else {

				  if (!ACMAJORREV_5(pi->pubpi.phy_rev)) {
			  rxpwr_core[core] +=
			    aci->sromi->rssioffset.rssi_corr_normal_5g[ant][subband_idx]
			        [bw_idx];
				  }
			  rxpwr_core[core] +=
			    aci->sromi->rssioffset.rssi_corr_gain_delta_5g_sub
			    [core][0][bw_idx][subband_idx] - gain_err_temp_adj_for_rssi;
			}
		}
	}

	/* only 3 antennas are valid for now */
	FOREACH_CORE(pi, core) {
		/* Cap Max/Min RSSI to -1/-128 */
		rxpwr_core[core] = MAX(-128, rxpwr_core[core]);
		rxpwr_core[core] = MIN(-1, rxpwr_core[core]);
		wrxh->rxpwr[core] = (int8)rxpwr_core[core];
	}
	wrxh->do_rssi_ma = 0;

	/* legacy interface */
	if (PHYCORENUM(pi->pubpi.phy_corenum) == 1) {
		rxpwr = rxpwr_core[0];
	} else {
		uint8 num_activecores = 0;

		rxpwr = 0;
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			if (num_activecores++ == 0) {
				rxpwr = rxpwr_core[core];
			} else {
				switch (pi->sh->rssi_mode) {
				case RSSI_ANT_MERGE_MAX:
					rxpwr = MAX(rxpwr, rxpwr_core[core]);
					break;
				case RSSI_ANT_MERGE_MIN:
					rxpwr = MIN(rxpwr, rxpwr_core[core]);
					break;
				case RSSI_ANT_MERGE_AVG:
					rxpwr += rxpwr_core[core];
					break;
				default:
					ASSERT(0);
				}
			}
		}

		if (pi->sh->rssi_mode == RSSI_ANT_MERGE_AVG) {
			int16 qrxpwr;

			ASSERT(num_activecores > 0);

			rxpwr = (int8)qm_div16(rxpwr, num_activecores, &qrxpwr);
		}
	}

	wrxh->rssi = (int8)rxpwr;
	wrxh->rssi_qdb = 0;

	PHY_TRACE(("%s: rssi %d\n", __FUNCTION__, (int8)rxpwr));

	aci->last_rssi = (int8)rxpwr;
}

static uint8
phy_ac_rssi_11b_WAR(phy_ac_info_t *aci, d11rxhdr_t *rxh)
{
	int16 PhyStatsGainInfo0, Auxphystats0;
	int8 lna1, lna2, mixer, biq0, biq1, trpos, dvga;
	int8 elna;
	int8 trloss;
	int8 elna_byp_tr;
	int8 lna1_gain, lna2_gain, rxmix_gain,
	        biq0_gain, biq1_gain, dvga_gain, fem_gain, total_rx_gain;

	PhyStatsGainInfo0 = ((ACPHY_RXPWR_ANT2(rxh) << 8) | (ACPHY_RXPWR_ANT1(rxh)));
	Auxphystats0 = ((ACPHY_RXPWR_ANT0(rxh) << 8) | (ACPHY_RXPWR_ANT4(rxh)));

	/* Parsing the gaininfo */
	lna1 = (PhyStatsGainInfo0 >> 0) & 0x7;
	lna2 = (PhyStatsGainInfo0 >> 3) & 0x7;
	mixer = (PhyStatsGainInfo0 >> 6) & 0xf;
	biq0 = (PhyStatsGainInfo0 >> 10) & 0x7;
	biq1 = (PhyStatsGainInfo0 >> 13) & 0x7;

	trpos = (Auxphystats0 >> 0) & 0x1;
	dvga = (Auxphystats0 >> 2) & 0xf;

	elna = aci->fem_rxgains[0].elna;
	trloss = aci->fem_rxgains[0].trloss;
	elna_byp_tr = aci->fem_rxgains[0].elna_bypass_tr;

	/* get gains of each block */
	dvga_gain  = 3*dvga;
	lna1_gain  = aci->rxgainctrl_params[0].gaintbl[1][lna1];
	lna2_gain  = aci->rxgainctrl_params[0].gaintbl[2][lna2];
	rxmix_gain = aci->rxgainctrl_params[0].gaintbl[3][mixer];
	biq0_gain = aci->rxgainctrl_params[0].gaintbl[4][biq0];
	biq1_gain = aci->rxgainctrl_params[0].gaintbl[5][biq1];

	/* Get fem gain */
	if (elna_byp_tr == 1) {
		if (trpos == 0) {
			fem_gain = elna;
		} else {
			fem_gain = elna - trloss;
		}
	} else {
		if (trpos == 0) {
			fem_gain = 0;
		} else {
			fem_gain = (-1*trloss);
		}
	}

	/* Total Rx gain */
	total_rx_gain = (lna1_gain + lna2_gain + rxmix_gain
		 + biq0_gain + biq1_gain + dvga_gain + fem_gain);

	return (2 - total_rx_gain + 256);
}

/* init gain error table. */
static void
_phy_ac_rssi_init_gain_err(phy_type_rssi_ctx_t *ctx)
{
	phy_ac_rssi_info_t *info = (phy_ac_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* XXX
	 * Gain error computed as follows:
	 * 1) Backoff subband dependent init_gain from gaintable,
	 * 2) add back fixed init-gain assumed by rxiqest,
	 * 3) add subband-dependent rxiqest gain error (retrieved from srom)
	 */
	int16 gainerr[PHY_CORE_MAX], tmp;
	int16 initgain_dB[PHY_CORE_MAX];
	int16 rxiqest_gain;
	uint8 core;
	bool srom_isempty = FALSE;
	uint8 dummy[ACPHY_MAX_RX_GAIN_STAGES];

	/* Retrieve rxiqest gain error: */
	srom_isempty = wlc_phy_get_rxgainerr_phy(pi, gainerr);
	if (srom_isempty) {
		/* XXX
		 * Do not apply gain error correction
		 * if nothing was written to SROM
		 */
		FOREACH_CORE(pi, core) {
			info->gain_err[core] = 0;
		}
		return;
	}

	/* Retrieve rxiqest gain: */
	/* XXX
	 * Must sync with rxiqest if gain assumed changes there
	 */
	if (BFCTL(pi->u.pi_acphy) == 2) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			rxiqest_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_2G);
		} else {
			rxiqest_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_5G);
		}
	} else {
		rxiqest_gain = (int16)(ACPHY_NOISE_INITGAIN);
	}

	/* Compute correction */
	FOREACH_CORE(pi, core) {
		/* Retrieve initgains in dB */
		initgain_dB[core] = wlc_phy_rxgainctrl_encode_gain_acphy(pi, 0, core,
		                                                         ACPHY_INIT_GAIN,
		                                                         FALSE, FALSE, dummy) - 2;

		/* gainerr is in 0.5dB steps; round to nearest dB */
		tmp = gainerr[core];
		tmp = ((tmp >= 0) ? ((tmp + 1) >> 1) : -1*((-1*tmp + 1) >> 1));
		/* report rssi gainerr in 0.5dB steps */
		info->gain_err[core] =
		        (int8)((rxiqest_gain << 1) - (initgain_dB[core] << 1) + gainerr[core]);
	}
}

void
phy_ac_rssi_init_gain_err(phy_ac_rssi_info_t *info)
{
	_phy_ac_rssi_init_gain_err(info);
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
phy_ac_rssi_dump(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ac_rssi_info_t *info = (phy_ac_rssi_info_t *)ctx;
	uint i;

	/* DUMP gain_err... */

	bcm_bprintf(b, "gain err:\n");
	for (i = 0; i < ARRAYSIZE(info->gain_err); i ++)
		bcm_bprintf(b, "  core %u: %u\n", i, info->gain_err[i]);

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
