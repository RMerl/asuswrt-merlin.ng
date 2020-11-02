/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11 Networking Device Driver.
 * code to handle the Nokia nvmem speciifc details
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
 * $Id: wlc_phy_noknvmem.c 413549 2013-07-19 12:23:52Z $:
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <proto/802.11.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>

#include <d11.h>

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phyreg_abg.h>
#include <wlc_phyreg_n.h>
#include <wlc_phyreg_lp.h>
#include <wlc_phyreg_ssn.h>
#include <wlc_phyreg_lcn.h>
#include <wlc_phytbl_n.h>
#include <wlc_phy_radio.h>
#include <wlc_phy_lcn.h>
#include <wlc_phy_lp.h>

#include <bcmwifi_channels.h>
#include <bcmotp.h>

#include <wlc_phy_noknvmem.h>

#define NOKSUBBAND_MAX			7
typedef struct noksubband_5g {
	uint8 start;
	uint8 end;
	uint8 subband;
} noksubband_5g_t;

const static noksubband_5g_t nok_5g_subbands[] = {
	{184,	196,	1},
	{8,	16,	2},
	{34,	48,	3},
	{52,	64,	4},
	{100,	116,	5},
	{120,	140,	6},
	{149,	165,	7},
};

#define NOK_CONSTELLATION_MAX_2G		5
#define NOK_CONSTELLATION_MAX_5G		4
#define	NOKNVMEM_2G_RATE_ELEMENTS		10
#define	NOKNVMEM_5G_RATE_ELEMENTS		9

#define NOKCONSTELLATION_64QAM_IDX	0
#define NOKCONSTELLATION_16QAM_IDX	1
#define NOKCONSTELLATION_QPSK_IDX	2
#define NOKCONSTELLATION_BPSK_IDX	3
#define NOKCONSTELLATION_CCK_IDX	4

#define NOKNVMEM_MAXTXPWR_PER_RATE_dBm		30
#define NOKNVMEM_MAXTXPWR_PER_RATE_qdBm	(NOKNVMEM_MAXTXPWR_PER_RATE_dBm * 4)
#define NOKNVMEM_VOLTAGE(a, b)	((a) * 10 + (b))

#define WLCPHY_NOKNVMEM_ENV_PREG	0
#define WLCPHY_NOKNVMEM_ENV_PDEG	1
#define WLCPHY_NOKNVMEM_ENV_PEXT	2

typedef struct phy_noknvmem {
	int8		fe_loss_offset[NOKSUBBAND_MAX + 1];
	int8		rx_fem_loss_offset[NOKSUBBAND_MAX + 1];
	int8		antgain_offset[NOKSUBBAND_MAX + 1];
	int8		pwrdet_offset_2G[NOK_CONSTELLATION_MAX_2G];
	int8		pwrdet_offset_5G[NOK_CONSTELLATION_MAX_5G];
	int8		extlow_vbat_threshold;
	int8		low_vbat_threshold;
	int8		high_vbat_threshold;
	int8		high_temp_threshold;
	int8		low_temp_threshold;
	int8		txpwr_limit_2G_norm[NOKNVMEM_2G_RATE_ELEMENTS];
	int8		txpwr_limit_2G_degraded[NOKNVMEM_2G_RATE_ELEMENTS];
	int8		txpwr_limit_2G_extreme[NOKNVMEM_2G_RATE_ELEMENTS];
	int8		txpwr_limit_2G_max[NOKNVMEM_2G_RATE_ELEMENTS];
	int8		txpwr_limit_2G_min[NOKNVMEM_2G_RATE_ELEMENTS];
	int8		txpwr_limit_5G_norm[NOKNVMEM_5G_RATE_ELEMENTS];
	int8		txpwr_limit_5G_degraded[NOKNVMEM_5G_RATE_ELEMENTS];
	int8		txpwr_limit_5G_extreme[NOKNVMEM_5G_RATE_ELEMENTS];
	int8		txpwr_limit_5G_max[NOKNVMEM_5G_RATE_ELEMENTS];
	int8		txpwr_limit_5G_min[NOKNVMEM_5G_RATE_ELEMENTS];
	uint8		cur_pwr_limit;
} phy_noknvmem_t;

static void wlc_phyinfo_init_noknvmem(phy_info_t *pi, phy_noknvmem_t *noknvmem);
static void wlc_phyinfo_dump_noknvmem(phy_info_t *pi, phy_noknvmem_t *noknvmem);
static uint8 wlc_phy_noknvmem_env_pwrlimit_check(phy_noknvmem_t *noknvmem, int8 vbat, int8 temp);

/* from noknvmem setction 4.7 */
static uint8 noktxpwr_ofdm_index[8] = {8, 8, 6, 6, 4, 4, 2, 2};
static uint8 noktxpwr_mcs_index[8] = {7, 5, 5, 3, 3, 1, 1, 0};
static uint mcs_idx_array[] = {
	TXP_FIRST_OFDM_20_CDD, TXP_FIRST_MCS_20_SISO, TXP_FIRST_MCS_20_CDD,
	TXP_FIRST_MCS_20_STBC, TXP_FIRST_MCS_20_SDM, TXP_FIRST_OFDM_40_SISO,
	TXP_FIRST_OFDM_40_CDD, TXP_FIRST_MCS_40_SISO, TXP_FIRST_MCS_40_CDD,
	TXP_FIRST_MCS_40_STBC, TXP_FIRST_MCS_40_SDM };

void *
BCMATTACHFN(wlc_phy_noknvmem_attach)(osl_t * osh, phy_info_t *pi)
{
	phy_noknvmem_t *noknvmem = NULL;

	noknvmem = (phy_noknvmem_t *)MALLOC(osh, sizeof(phy_noknvmem_t));
	if (noknvmem == NULL) {
		PHY_ERROR(("wl%d: %s: out of mem to alloc noknvmem, malloced %d bytes\n",
		           pi->sh->unit, __FUNCTION__, MALLOCED(osh)));
		goto err;
	}
	bzero(noknvmem, sizeof(phy_noknvmem_t));

	/* init the nok nvmem */
	wlc_phyinfo_init_noknvmem(pi, noknvmem);

	/* dump nvmem for debug purposes */
	wlc_phyinfo_dump_noknvmem(pi, noknvmem);

	return ((void *)noknvmem);
err:
	wlc_phy_noknvmem_detach(osh, noknvmem);
	return NULL;
}

void
BCMATTACHFN(wlc_phy_noknvmem_detach)(osl_t *osh, void *nvmem)
{
	if (nvmem != NULL)
		MFREE(osh, nvmem, sizeof(phy_noknvmem_t));
}

static void
BCMATTACHFN(wlc_phyinfo_dump_noknvmem)(phy_info_t *pi, phy_noknvmem_t *noknvmem)
{
#ifdef BCMDBG
	int i;
#ifndef WL_PRINT
#define WL_PRINT(args) printf args
#endif // endif

	WL_PRINT(("\nfe_loss: \n\t"));
	for (i = 0; i < NOKSUBBAND_MAX + 1; i++)
		WL_PRINT(("%d, ", noknvmem->fe_loss_offset[i]));

	WL_PRINT(("\nrx_fem_loss: \n\t"));
	for (i = 0; i < NOKSUBBAND_MAX + 1; i++)
		WL_PRINT(("%d, ", noknvmem->rx_fem_loss_offset[i]));

	WL_PRINT(("\nantgain_offset: \n\t"));
	for (i = 0; i < NOKSUBBAND_MAX + 1; i++)
		WL_PRINT(("%d, ", noknvmem->antgain_offset[i]));

	WL_PRINT(("\n2G_pdet_offset_hqdb: \n\t"));
	for (i = 0; i < NOK_CONSTELLATION_MAX_2G; i++)
		WL_PRINT(("%d, ", noknvmem->pwrdet_offset_2G[i]));

	WL_PRINT(("\n5G_pdet_offset_hqdb: \n\t"));
	for (i = 0; i < NOK_CONSTELLATION_MAX_5G; i++)
		WL_PRINT(("%d, ", noknvmem->pwrdet_offset_5G[i]));

	WL_PRINT(("\n2G_pnorm_txpwr: \n\t"));
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++)
		WL_PRINT(("%d, ", noknvmem->txpwr_limit_2G_norm[i]));
	WL_PRINT(("\n2G_pdeg_txpwr: \n\t"));
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++)
		WL_PRINT(("%d, ", noknvmem->txpwr_limit_2G_degraded[i]));
	WL_PRINT(("\n2G_pext_txpwr: \n\t"));
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++)
		WL_PRINT(("%d, ", noknvmem->txpwr_limit_2G_extreme[i]));

	WL_PRINT(("\n5G_pnorm_txpwr: \n\t"));
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++)
		WL_PRINT(("%d, ", noknvmem->txpwr_limit_5G_norm[i]));
	WL_PRINT(("\n5G_pdeg_txpwr: \n\t"));
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++)
		WL_PRINT(("%d, ", noknvmem->txpwr_limit_5G_degraded[i]));
	WL_PRINT(("\n5G_pext_txpwr: \n\t"));
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++)
		WL_PRINT(("%d, ", noknvmem->txpwr_limit_5G_extreme[i]));

	WL_PRINT(("\n"));
	WL_PRINT(("vbat_el_th : %d\n",  noknvmem->extlow_vbat_threshold));
	WL_PRINT(("vbat_l_th : %d\n",  noknvmem->low_vbat_threshold));
	WL_PRINT(("vbat_h_th : %d\n",  noknvmem->high_vbat_threshold));
	WL_PRINT(("temp_h_th : %d\n",  noknvmem->high_temp_threshold));
	WL_PRINT(("temp_l_th : %d\n",  noknvmem->low_temp_threshold));
#endif /* BCMDBG */
}

static void
BCMATTACHFN(wlc_phyinfo_init_noknvmem)(phy_info_t *pi, phy_noknvmem_t *noknvmem)
{
	int i;
	char *var;

	bzero(&noknvmem->fe_loss_offset[0], NOKSUBBAND_MAX + 1);
	if (getvar(pi->vars,  "fe_loss_hdb") != NULL) {
		for (i = 0; i < NOKSUBBAND_MAX + 1; i++)
			noknvmem->fe_loss_offset[i] =
				((uint8)getintvararray(pi->vars, "fe_loss_hdb", i) * 2);
	}

	bzero(&noknvmem->rx_fem_loss_offset[0], NOKSUBBAND_MAX + 1);
	if (getvar(pi->vars,  "rx_fem_loss_hdb") != NULL) {
		for (i = 0; i < NOKSUBBAND_MAX + 1; i++)
			noknvmem->rx_fem_loss_offset[i] =
				((uint8)getintvararray(pi->vars, "rx_fem_loss_hdb", i) * 2);
	}

	bzero(&noknvmem->antgain_offset[0], NOKSUBBAND_MAX + 1);
	if (getvar(pi->vars,  "antgain_offset_hdb") != NULL) {
		for (i = 0; i < NOKSUBBAND_MAX + 1; i++)
			noknvmem->antgain_offset[i] =
				((int8)getintvararray(pi->vars, "antgain_offset_hdb", i) * 2);
	}

	bzero(&noknvmem->pwrdet_offset_2G[0], NOK_CONSTELLATION_MAX_2G);
	if (getvar(pi->vars,  "2G_pdet_offset_hqdb") != NULL) {
		for (i = 0; i < NOK_CONSTELLATION_MAX_2G; i++)
			noknvmem->pwrdet_offset_2G[i] =
				((int8)getintvararray(pi->vars, "2G_pdet_offset_hqdb", i));
	}

#ifdef BAND5G
	bzero(&noknvmem->pwrdet_offset_5G[0], NOK_CONSTELLATION_MAX_5G);
	if (getvar(pi->vars,  "5G_pdet_offset_hqdb") != NULL) {
		for (i = 0; i < NOK_CONSTELLATION_MAX_5G; i++)
			noknvmem->pwrdet_offset_5G[i] =
				((int8)getintvararray(pi->vars, "5G_pdet_offset_hqdb", i));
	}
#endif // endif

	var = getvar(pi->vars,  "2G_pnorm_txpwr_hdb");
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_2G_norm[i] =
				((uint8)getintvararray(pi->vars, "2G_pnorm_txpwr_hdb", i) * 2);
		else
			noknvmem->txpwr_limit_2G_norm[i] = (uint8)NOKNVMEM_MAXTXPWR_PER_RATE_qdBm;
	}
	var = getvar(pi->vars,  "2G_pdeg_txpwr_hdb");
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_2G_degraded[i] =
				((uint8)getintvararray(pi->vars, "2G_pdeg_txpwr_hdb", i) * 2);
		else
			noknvmem->txpwr_limit_2G_degraded[i] =
				(uint8)NOKNVMEM_MAXTXPWR_PER_RATE_qdBm;
	}
	var = getvar(pi->vars,  "2G_pext_txpwr_hdb");
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_2G_extreme[i] =
				((uint8)getintvararray(pi->vars, "2G_pext_txpwr_hdb", i) * 2);
		else
			noknvmem->txpwr_limit_2G_extreme[i] =
				(uint8)NOKNVMEM_MAXTXPWR_PER_RATE_qdBm;
	}
	var = getvar(pi->vars,  "2G_maxtxpwr_limit_hdb");
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_2G_max[i] =
				((uint8)getintvararray(pi->vars, "2G_maxtxpwr_limit_hdb", i) * 2);
	}
	var = getvar(pi->vars,  "2G_mintxpwr_fs_hdb");
	for (i = 0; i < NOKNVMEM_2G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_2G_min[i] =
				((uint8)getintvararray(pi->vars, "2G_mintxpwr_fs_hdb", i) * 2);
	}

#ifdef BAND5G
	var = getvar(pi->vars,  "5G_pnorm_txpwr_hdb");
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_5G_norm[i] =
				((uint8)getintvararray(pi->vars, "5G_pnorm_txpwr_hdb", i) * 2);
		else
			noknvmem->txpwr_limit_5G_norm[i] = (uint8)NOKNVMEM_MAXTXPWR_PER_RATE_qdBm;
	}
	var = getvar(pi->vars,  "5G_pdeg_txpwr_hdb");
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_5G_degraded[i] =
				((uint8)getintvararray(pi->vars, "5G_pdeg_txpwr_hdb", i) * 2);
		else
			noknvmem->txpwr_limit_5G_degraded[i] =
				(uint8)NOKNVMEM_MAXTXPWR_PER_RATE_qdBm;
	}
	var = getvar(pi->vars,  "5G_pext_txpwr_hdb");
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_5G_extreme[i] =
				((uint8)getintvararray(pi->vars, "5G_pext_txpwr_hdb", i) * 2);
		else
			noknvmem->txpwr_limit_5G_extreme[i] =
				(uint8)NOKNVMEM_MAXTXPWR_PER_RATE_qdBm;
	}
	var = getvar(pi->vars,  "5G_maxtxpwr_limit_hdb");
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_5G_max[i] =
				((uint8)getintvararray(pi->vars, "5G_maxtxpwr_limit_hdb", i) * 2);
	}
	var = getvar(pi->vars,  "5G_mintxpwr_fs_hdb");
	for (i = 0; i < NOKNVMEM_5G_RATE_ELEMENTS; i++) {
		if (var)
			noknvmem->txpwr_limit_5G_min[i] =
				((uint8)getintvararray(pi->vars, "5G_mintxpwr_fs_hdb", i) * 2);
	}
#endif /* BAND5G */

	if (getvar(pi->vars, "vbat_el_th"))
		noknvmem->extlow_vbat_threshold = (uint8)getintvar(pi->vars, "vbat_el_th");
	else
		noknvmem->extlow_vbat_threshold = (uint8)NOKNVMEM_VOLTAGE(2, 7);

	if (getvar(pi->vars, "vbat_l_th"))
		noknvmem->low_vbat_threshold = (uint8)getintvar(pi->vars, "vbat_l_th");
	else
		noknvmem->low_vbat_threshold = (uint8)NOKNVMEM_VOLTAGE(3, 0);

	if (getvar(pi->vars, "vbat_h_th"))
		noknvmem->high_vbat_threshold = (uint8)getintvar(pi->vars, "vbat_h_th");
	else
		noknvmem->high_vbat_threshold = (uint8)NOKNVMEM_VOLTAGE(4, 8);

	if (getvar(pi->vars, "temp_h_th"))
		noknvmem->high_temp_threshold = (int8)getintvar(pi->vars, "temp_h_th");
	else
		noknvmem->high_temp_threshold = (int8)75;

	if (getvar(pi->vars, "temp_l_th"))
		noknvmem->low_temp_threshold = (int8)getintvar(pi->vars, "temp_l_th");
	else
		noknvmem->low_temp_threshold = (int8)-30;
}

static uint8
channel2noksubband(uint chan, uint band)
{
#ifdef BAND5G
	const noksubband_5g_t *b;
	int i;
#endif /* BAND5G */

	if (band != WLC_BAND_5G)
		return 0;
#ifdef BAND5G
	for (i = 0; i < ARRAYSIZE(nok_5g_subbands); i++) {
		b = &nok_5g_subbands[i];
		if ((chan >= (uint)b->start) && (chan <= (uint)b->end))
			return (b->subband);
	}
	ASSERT(0);
#endif /* BAND5G */
	return 0;
}

uint8
txp_rateindex2nokconstellation(uint rate_idx)
{
	int o_idx = -1;
	int m_idx = -1;

	/* mcs 5, 6, 7, 48/54Mbps	: 64QAM */
	/* mcs 4, 3, 24/36Mbps		: 16QAM */
	/* mcs 2, 1, 12/18Mbps 		: QPSK */
	/* mcs 0, 6/9Mbps 		: BPSK */

	ASSERT(rate_idx >= TXP_FIRST_OFDM_40_SISO);

	if (rate_idx < TXP_LAST_CCK)
		return NOKCONSTELLATION_CCK_IDX;

	if ((rate_idx >= TXP_FIRST_OFDM) && (rate_idx <= TXP_LAST_OFDM))
		o_idx = rate_idx - TXP_FIRST_OFDM;
	else {
		if (rate_idx >= TXP_FIRST_MCS_20_SDM)
			m_idx = rate_idx - TXP_FIRST_MCS_20_SDM;
		else if (rate_idx >= TXP_FIRST_MCS_20_STBC)
			m_idx = rate_idx - TXP_FIRST_MCS_20_STBC;
		else if (rate_idx >= TXP_FIRST_MCS_20_CDD)
			m_idx = rate_idx - TXP_FIRST_MCS_20_CDD;
		else if (rate_idx >= TXP_FIRST_MCS_20_SISO)
			m_idx = rate_idx - TXP_FIRST_MCS_20_SISO;
		else if (rate_idx >= TXP_FIRST_OFDM_20_CDD)
			m_idx = rate_idx - TXP_FIRST_OFDM_20_CDD;
	}
	if ((o_idx == 0) || (o_idx == 1) || (m_idx == 0))
		return NOKCONSTELLATION_BPSK_IDX;
	else if ((o_idx == 2) || (o_idx == 3) || (m_idx == 1) || (m_idx == 2))
		return NOKCONSTELLATION_QPSK_IDX;
	else if ((o_idx == 4) || (o_idx == 5) || (m_idx == 3) || (m_idx == 4))
		return NOKCONSTELLATION_16QAM_IDX;
	else if ((o_idx == 6) || (o_idx == 7) || (m_idx == 5) || (m_idx == 6) || (m_idx == 7))
		return NOKCONSTELLATION_64QAM_IDX;
	else {
		ASSERT(0);
		return NOKCONSTELLATION_64QAM_IDX;
	}

}

void
wlc_phy_nokia_brdtxpwr_limits(phy_info_t *pi, uint chan, int txp_idx, uint8 *minpwr, uint8 *maxpwr)
{
	uint8 const_idx;
	phy_noknvmem_t *noknvmem = pi->noknvmem;

	*minpwr = 0;
	*maxpwr = 0;

	if ((txp_idx  >= TXP_FIRST_CCK) && (txp_idx <= TXP_LAST_CCK))
		const_idx = 9;
	else if ((txp_idx >= TXP_FIRST_OFDM) && (txp_idx <= TXP_LAST_OFDM))
		const_idx = noktxpwr_ofdm_index[txp_idx - TXP_FIRST_OFDM];
	else if ((txp_idx >= TXP_FIRST_MCS_20_SS) && (txp_idx <= TXP_LAST_MCS_20_SISO_SS))
		const_idx = noktxpwr_mcs_index[txp_idx - TXP_FIRST_MCS_20_SS];
	else if ((txp_idx >= TXP_FIRST_MCS_20_SISO) && (txp_idx <= TXP_LAST_MCS_20_SISO))
		const_idx = noktxpwr_mcs_index[txp_idx - TXP_FIRST_MCS_20_SISO];
	else if ((txp_idx >= TXP_FIRST_MCS_20_CDD) && (txp_idx <= TXP_LAST_MCS_20_CDD))
		const_idx = noktxpwr_mcs_index[txp_idx - TXP_FIRST_MCS_20_CDD];
	else if ((txp_idx >= TXP_FIRST_MCS_20_SDM) && (txp_idx <= TXP_LAST_MCS_20_SDM))
		const_idx = noktxpwr_mcs_index[txp_idx - TXP_FIRST_MCS_20_SDM];
	else if ((txp_idx >= TXP_FIRST_MCS_20_STBC) && (txp_idx <= TXP_LAST_MCS_20_STBC))
		const_idx = noktxpwr_mcs_index[txp_idx - TXP_FIRST_MCS_20_STBC];
	else
		return;
	if (chan <= 14) {
		*minpwr = noknvmem->txpwr_limit_2G_min[const_idx];
		*maxpwr = noknvmem->txpwr_limit_2G_max[const_idx];
	}
#ifdef BAND5G
	else {
		*minpwr = noknvmem->txpwr_limit_5G_min[const_idx];
		*maxpwr = noknvmem->txpwr_limit_5G_max[const_idx];
	}
#endif /* BAND5G */
}

int8
wlc_phy_noknvmem_antport_to_rfport_offset(phy_info_t *pi, uint chan, uint32 band, uint rate)
{
	int8 offset = 0;
	uint8 subband_idx;
	phy_noknvmem_t *noknvmem = pi->noknvmem;

	ASSERT(noknvmem);
	subband_idx = channel2noksubband(chan, band);
	/* table 3 FE Lost */
	offset += noknvmem->fe_loss_offset[subband_idx];
	/* table 5 antgain offset */
	offset -= noknvmem->antgain_offset[subband_idx];

	return offset;
}

bool
wlc_phy_noknvmem_env_check(phy_info_t *pi, int8 vbat, int8 temp)
{
	uint8 pwr_limits;
	phy_noknvmem_t *noknvmem = pi->noknvmem;

	pwr_limits = wlc_phy_noknvmem_env_pwrlimit_check(noknvmem, vbat, temp);
	if (noknvmem->cur_pwr_limit != pwr_limits)
		return TRUE;
	else
		return FALSE;
}

static uint8
wlc_phy_noknvmem_env_pwrlimit_check(phy_noknvmem_t *noknvmem, int8 vbat_brcm, int8 temp)
{
	uint8 vbat;

	vbat = ((vbat_brcm & 0xF0) >> 4) * 10 +
		(((vbat_brcm & 0x0F) * 10) + 8)/16;

	/* vbat is in Q4.4 format and noknvmem values are in .1V represents 1 format */
	if (vbat < noknvmem->extlow_vbat_threshold) {
		return WLCPHY_NOKNVMEM_ENV_PEXT;
	}
	else if ((vbat >= noknvmem->extlow_vbat_threshold) && (vbat < noknvmem->low_vbat_threshold))
	{
		if ((temp < noknvmem->high_temp_threshold) && (temp > noknvmem->low_temp_threshold))
			return WLCPHY_NOKNVMEM_ENV_PDEG;
		else
			return WLCPHY_NOKNVMEM_ENV_PEXT;
	}
	else {
		/* vbat is more than the low threhshold */
		if ((temp < noknvmem->high_temp_threshold) && (temp > noknvmem->low_temp_threshold))
			return WLCPHY_NOKNVMEM_ENV_PREG;
		else
			return WLCPHY_NOKNVMEM_ENV_PDEG;
	}
}

void
wlc_phy_noknvmem_env_txpwrlimit_upd(phy_info_t *pi, int8 vbat, int8 temp, uint32 band)
{
	phy_noknvmem_t *noknvmem = pi->noknvmem;
	uint8 pwr_limits;
	int8 *ptr;
	uint offset, i, j;

	pwr_limits = wlc_phy_noknvmem_env_pwrlimit_check(noknvmem, vbat, temp);
	noknvmem->cur_pwr_limit = pwr_limits;
	/* get the right offset to get params from */
	if (pwr_limits == WLCPHY_NOKNVMEM_ENV_PREG) {
		if (band == WLC_BAND_2G)
			ptr = &noknvmem->txpwr_limit_2G_norm[0];
		else
			ptr = &noknvmem->txpwr_limit_5G_norm[0];
	}
	else if (pwr_limits == WLCPHY_NOKNVMEM_ENV_PDEG) {
		if (band == WLC_BAND_2G)
			ptr = &noknvmem->txpwr_limit_2G_degraded[0];
		else
			ptr = &noknvmem->txpwr_limit_5G_degraded[0];
	}
	else if (pwr_limits == WLCPHY_NOKNVMEM_ENV_PEXT) {
		if (band == WLC_BAND_2G)
			ptr = &noknvmem->txpwr_limit_2G_extreme[0];
		else
			ptr = &noknvmem->txpwr_limit_5G_extreme[0];
	}
	else {
		ASSERT(0);
		ptr = &noknvmem->txpwr_limit_2G_norm[0];
	}

	/* fill the rate limits */
	if (band == WLC_BAND_2G) {
		for (i = TXP_FIRST_CCK; i <= TXP_LAST_CCK; i++)
			pi->txpwr_env_limit[i] = ptr[9];
	}
	/* ofdm non mcs rates */
	for (i = 0; i < sizeof(noktxpwr_ofdm_index); i++)
		pi->txpwr_env_limit[TXP_FIRST_OFDM + i] = ptr[noktxpwr_ofdm_index[i]];
	/* ofdm mcs rates */
	for (i = 0; i < sizeof(mcs_idx_array); i++) {
		offset = mcs_idx_array[i];
		for (j = 0; j < sizeof(noktxpwr_mcs_index); j++)
			pi->txpwr_env_limit[offset + j] = ptr[noktxpwr_mcs_index[j]];
	}
}

void
wlc_phy_noknvmem_get_pwrdet_offsets(phy_info_t *pi, int8 *cckoffset, int8 *ofdmoffset)
{
	phy_noknvmem_t *noknvmem = pi->noknvmem;
	ASSERT(noknvmem);
	*cckoffset = 0;
	*ofdmoffset = 0;
	if (noknvmem) {
		*cckoffset = noknvmem->pwrdet_offset_2G[NOKCONSTELLATION_CCK_IDX];
		*ofdmoffset = noknvmem->pwrdet_offset_2G[NOKCONSTELLATION_64QAM_IDX];
	}
}

int8
wlc_phy_noknvmem_modify_rssi(phy_info_t *pi, int8 rssi, chanspec_t chanspec)
{
	uint8 subband_idx;
	int8 offset;
	uint32 band = CHSPEC2WLC_BAND(pi->radio_chanspec);
	uint8 chan = CHSPEC_CHANNEL(pi->radio_chanspec); /* see wlioctl.h */
	phy_noknvmem_t *noknvmem = pi->noknvmem;

	ASSERT(noknvmem);
	subband_idx = channel2noksubband(chan, band);

	/* table 3 FE Loss */
	offset = noknvmem->fe_loss_offset[subband_idx];
	/* table 4 RX FEM Loss offset */
	offset += noknvmem->rx_fem_loss_offset[subband_idx];

	return (rssi + offset);
}
