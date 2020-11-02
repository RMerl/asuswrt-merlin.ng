/*
 * ABGPHY module header file
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
 * $Id: wlc_phy_ht.h 442777 2013-12-12 21:05:59Z $
 */

#ifndef _wlc_phy_ht_h_
#define _wlc_phy_ht_h_

#include <typedefs.h>

#define HTPHY_GAIN_VS_TEMP_SLOPE_2G 8   /* units: db/100C */
#define HTPHY_GAIN_VS_TEMP_SLOPE_5G 8   /* units: db/100C */
#define HTPHY_TEMPSENSE_TIMER 10

typedef struct _htphy_dac_adc_decouple_war {
	bool   is_on;
	uint16 PapdCalShifts[PHY_CORE_MAX];
	uint16 PapdEnable[PHY_CORE_MAX];
	uint16 PapdCalCorrelate;
	uint16 PapdEpsilonUpdateIterations;
	uint16 PapdCalSettle;
} htphy_dac_adc_decouple_war_t;

typedef struct _htphy_rxcal_radioregs {
	bool   is_orig;
	uint16 RF_TX_txrxcouple_2g_pwrup[PHY_CORE_MAX];
	uint16 RF_TX_txrxcouple_2g_atten[PHY_CORE_MAX];
	uint16 RF_TX_txrxcouple_5g_pwrup[PHY_CORE_MAX];
	uint16 RF_TX_txrxcouple_5g_atten[PHY_CORE_MAX];
	uint16 RF_afe_vcm_cal_master[PHY_CORE_MAX];
	uint16 RF_afe_set_vcm_i[PHY_CORE_MAX];
	uint16 RF_afe_set_vcm_q[PHY_CORE_MAX];
	uint16 RF_rxbb_vgabuf_idacs[PHY_CORE_MAX];
	uint16 RF_rxbuf_degen[PHY_CORE_MAX];
} htphy_rxcal_radioregs_t;

typedef struct _htphy_rxcal_phyregs {
	bool   is_orig;
	uint16 BBConfig;
	uint16 bbmult[PHY_CORE_MAX];
	uint16 rfseq_txgain[PHY_CORE_MAX];
	uint16 Afectrl[PHY_CORE_MAX];
	uint16 AfectrlOverride[PHY_CORE_MAX];
	uint16 RfseqCoreActv2059;
	uint16 RfctrlIntc[PHY_CORE_MAX];
	uint16 RfctrlCorePU[PHY_CORE_MAX];
	uint16 RfctrlOverride[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfCT[PHY_CORE_MAX];
	uint16 RfctrlOverrideLpfCT[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfPU[PHY_CORE_MAX];
	uint16 RfctrlOverrideLpfPU[PHY_CORE_MAX];
	uint16 PapdEnable[PHY_CORE_MAX];
} htphy_rxcal_phyregs_t;

typedef struct _htphy_txcal_radioregs {
	bool   is_orig;
	uint16 RF_TX_tx_ssi_master[PHY_CORE_MAX];
	uint16 RF_TX_tx_ssi_mux[PHY_CORE_MAX];
	uint16 RF_TX_tssia[PHY_CORE_MAX];
	uint16 RF_TX_tssig[PHY_CORE_MAX];
	uint16 RF_TX_iqcal_vcm_hg[PHY_CORE_MAX];
	uint16 RF_TX_iqcal_idac[PHY_CORE_MAX];
	uint16 RF_TX_tssi_misc1[PHY_CORE_MAX];
	uint16 RF_TX_tssi_vcm[PHY_CORE_MAX];
} htphy_txcal_radioregs_t;

typedef struct _htphy_txcal_phyregs {
	bool   is_orig;
	uint16 BBConfig;
	uint16 Afectrl[PHY_CORE_MAX];
	uint16 AfectrlOverride[PHY_CORE_MAX];
	uint16 Afectrl_AuxADCmode[PHY_CORE_MAX];
	uint16 RfctrlIntc[PHY_CORE_MAX];
	uint16 RfctrlPU[PHY_CORE_MAX];
	uint16 RfctrlOverride[PHY_CORE_MAX];
	uint16 PapdEnable[PHY_CORE_MAX];
} htphy_txcal_phyregs_t;

typedef struct _htphy_rxgain_phyregs {
	bool   is_orig;
	uint16 RfctrlOverride[PHY_CORE_MAX];
	uint16 RfctrlRXGAIN[PHY_CORE_MAX];
	uint16 Rfctrl_lpf_gain[PHY_CORE_MAX];
} htphy_rxgain_phyregs_t;

typedef struct _htphy_lpfCT_phyregs {
	bool   is_orig;
	uint16 RfctrlOverrideLpfCT[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfCT[PHY_CORE_MAX];
} htphy_lpfCT_phyregs_t;

typedef struct _htphy_rcal_rccal_cache {
	bool cache_valid;
	uint16 rcal_val;
	uint16 rccal_bcap_val;
	uint16 rccal_scap_val;
	uint16 rccal_hpc_val;
} htphy_rcal_rccal_cache;

typedef struct _htphy_nshapetbl_mon {
	uint8 start_addr[NTONES_BW40];
	uint8 mod_length[NTONES_BW40];
	uint8 length;
} htphy_nshapetbl_mon_t;

/* ********************************************************* */
#include "phy_api.h"
#include "phy_ht_ana.h"
#include "phy_ht_radio.h"
#include "phy_ht_tbl.h"
#include "phy_ht_tpc.h"
#include "phy_ht_radar.h"
#include "phy_ht_noise.h"
#include "phy_ht_temp.h"
#include "phy_ht_rssi.h"
#include "phy_ht_rxiqcal.h"
#include "phy_ht_txiqlocal.h"
#include "phy_ht_papdcal.h"
#include "phy_ht_vcocal.h"
#include <phy_ht_calmgr.h>
/* ********************************************************* */

struct phy_info_htphy {
/* ********************************************************* */
	phy_info_t *pi;
	phy_ht_ana_info_t			*anai;
	phy_ht_radio_info_t			*radioi;
	phy_ht_tbl_info_t			*tbli;
	phy_ht_tpc_info_t			*tpci;
	phy_ht_radar_info_t			*radari;
	phy_ht_noise_info_t			*noisei;
	phy_ht_temp_info_t			*tempi;
	phy_ht_rssi_info_t			*rssii;
	phy_ht_rxiqcal_info_t		*rxiqcali;
	phy_ht_txiqlocal_info_t		*txiqlocali;
	phy_ht_papdcal_info_t		*papdcali;
	phy_ht_vcocal_info_t		*vcocali;
	phy_ht_calmgr_info_t		*calmgri;
/* ********************************************************* */
	uint16 classifier_state;
	uint16 clip_state[PHY_CORE_MAX];
	uint16 deaf_count;
	uint16 saved_bbconf;
	uint16 rfctrlIntc_save[PHY_CORE_MAX];
	uint16 bb_mult_save[PHY_CORE_MAX];
	uint8  bb_mult_save_valid;
	uint8  txpwrindex_hw_save[PHY_CORE_MAX]; /* txpwr start index for hwpwrctrl */
	int8   idle_tssi[PHY_CORE_MAX];
	int8   txpwr_offset[PHY_CORE_MAX];	/* qdBm signed offset for per-core tx pwr */

	uint32 pstart; /* sample collect fifo begins */
	uint32 pstop;  /* sample collect fifo ends */
	uint32 pfirst; /* sample collect trigger begins */
	uint32 plast;  /* sample collect trigger ends */

	bool   ht_rxldpc_override;	/* LDPC override for RX, both band */

	htphy_dac_adc_decouple_war_t ht_dac_adc_decouple_war_info;
	htphy_txcal_radioregs_t htphy_txcal_radioregs_orig;
	htphy_txcal_phyregs_t ht_txcal_phyregs_orig;
	htphy_rxcal_radioregs_t ht_rxcal_radioregs_orig;
	htphy_rxcal_phyregs_t  ht_rxcal_phyregs_orig;
	htphy_rxgain_phyregs_t ht_rxgain_phyregs_orig;
	htphy_lpfCT_phyregs_t ht_lpfCT_phyregs_orig;

	/* rcal and rccal caching */
	htphy_rcal_rccal_cache rcal_rccal_cache;

	/* nvnoiseshapingtbl monitor */
	htphy_nshapetbl_mon_t nshapetbl_mon;

	bool ht_ofdm20_filt_war_req;
	bool ht_ofdm20_filt_war_active;
	int8    txpwrindex[PHY_CORE_MAX]; 		/* index if hwpwrctrl if OFF */

	txcal_coeffs_t txcal_cache[PHY_CORE_MAX];
	uint16	txcal_cache_cookie;
	uint8   radar_cal_active; /* to mask radar detect during cal's tone-play */

	uint8	elna2g;
	uint8	elna5g;

	int16 current_temperature;

	bool btc_restage_rxgain;          /* indicates if rxgain restaging is active */
	uint16 btc_saved_init_regs[PHY_CORE_MAX][2];  /* store phyreg values prior to
						       * restaging rxgain
						       */
	uint16 btc_saved_init_rfseq[PHY_CORE_MAX];    /* store rfseq table values prior to
						       * restaging rxgain
						       */
	uint16 btc_saved_nbclip[PHY_CORE_MAX];     /* store nbclip thresholds prior to
						    * restaging rxgain
						    */
	int8 btc_saved_elnagain[PHY_CORE_MAX][2];  /* store elna gains prior to
						    * restaging rxgain
						    */
	uint16 btc_saved_cliphi[PHY_CORE_MAX][2];  /* store cliphi gains prior to
						    * restaging rxgain
						    */
	uint16 btc_saved_cliplo[PHY_CORE_MAX][2];  /* store cliplo gains prior to
						    * restaging rxgain
						    */
	int8 btc_saved_lna1gains[PHY_CORE_MAX][4];  /* store lna1 gains prior to
						    * restaging rxgain
						    */
	int8 btc_saved_lna2gains[PHY_CORE_MAX][4];  /* store lna2 gains prior to
						    * restaging rxgain
						    */
	int8 btc_saved_lna1gainbits[PHY_CORE_MAX][4];  /* store lna1 gainbits prior to
							* restaging rxgain
							*/
	int8 btc_saved_lna2gainbits[PHY_CORE_MAX][4];  /* store lna2 gainbits prior to
							* restaging rxgain
							*/

};

/* **************************** REMOVE ************************** */
void wlc_phy_get_initgain_dB_htphy(phy_info_t *pi, int16 *initgain_dB);

#endif /* _wlc_phy_ht_h_ */
