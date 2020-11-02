/*
 * ACPHY FCBS module implementation
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
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_fcbs.h"
#include <phy_ac.h>
#include <phy_ac_fcbs.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <phy_utils_reg.h>
#include <phy_utils_channel.h>
#include <phy_utils_math.h>
#include <wlc_radioreg_20691.h>
#include <wlc_phy_radio.h>
#include <wlc_phy_shim.h>

/* time to spinwait while waiting for the FCBS
 * switch trigger bit to go low after FCBS
 */
#define ACPHY_SPINWAIT_FCBS_SWITCH 2000
#define ACPHY_FCBS_PHYTBL16_LEN 400

typedef struct _acphy_fcbs_info {
	uint16 phytbl16_buf_ChanA[ACPHY_FCBS_PHYTBL16_LEN];
	uint16 phytbl16_buf_ChanB[ACPHY_FCBS_PHYTBL16_LEN];
} acphy_fcbs_info;

/* module private states */
struct phy_ac_fcbs_info {
	phy_info_t			*pi;
	phy_ac_info_t		*aci;
	phy_fcbs_info_t		*cmn_info;
	acphy_fcbs_info     *ac_fcbs;
};

/* local functions */

/* register phy type specific implementation */
phy_ac_fcbs_info_t *
BCMATTACHFN(phy_ac_fcbs_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_fcbs_info_t *cmn_info)
{
	phy_ac_fcbs_info_t *ac_info;
	phy_type_fcbs_fns_t fns;
	acphy_fcbs_info     *ac_fcbs = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_fcbs_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	if ((ac_fcbs = phy_malloc(pi, sizeof(acphy_fcbs_info))) == NULL) {
		PHY_ERROR(("%s: phy_malloc ac_fcbs failed\n", __FUNCTION__));
		goto fail;
	}
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;
	ac_info->ac_fcbs = ac_fcbs;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;

	if (phy_fcbs_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_fcbs_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_fcbs != NULL)
		phy_mfree(pi, ac_fcbs, sizeof(acphy_fcbs_info));

	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_fcbs_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_fcbs_unregister_impl)(phy_ac_fcbs_info_t *ac_info)
{
	phy_info_t *pi;
	phy_fcbs_info_t *cmn_info;

	ASSERT(ac_info);
	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_fcbs_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info->ac_fcbs, sizeof(acphy_fcbs_info));

	phy_mfree(pi, ac_info, sizeof(phy_ac_fcbs_info_t));
}

/* ********************************************* */
/*				Internal Definitions					 */
/* ********************************************* */
#ifdef ENABLE_FCBS

/* PHY specific on-chip RAM offset of the FCBS cache */
#define FCBS_ACPHY_TMPLRAM_STARTADDR	0x1000

/* PHY specific shmem locations for specifying the length
 * of radio reg cache, phytbl cache, phyreg cache
 */
#define M_FCBS_BLK_BASE			(0x1da * 2)
#define M_FCBS_ACPHY_RADIOREG		(M_FCBS_BLK_BASE + 0)
#define M_FCBS_ACPHY_PHYTBL16		(M_FCBS_BLK_BASE + 2)
#define M_FCBS_ACPHY_PHYTBL32		(M_FCBS_BLK_BASE + 4)
#define M_FCBS_ACPHY_PHYREG		(M_FCBS_BLK_BASE + 6)
#define M_FCBS_ACPHY_BPHYCTRL		(M_FCBS_BLK_BASE + 8)
#define M_FCBS_ACPHY_TEMPLATE_PTR	(M_FCBS_BLK_BASE + 10)

/* List of all PHY TBL segments to be saved during FCBS */
static fcbs_phytbl_list_entry fcbs_phytbl16_list_acphy [ ] =
{
	{ ACPHY_TBL_ID_RFSEQ, 0x30, 6 },
	{ ACPHY_TBL_ID_RFSEQ, 0x40, 6 },
	{ ACPHY_TBL_ID_RFSEQ, 0x50, 6 },
	{ ACPHY_TBL_ID_RFSEQ, 0xa0, 5 },
	{ ACPHY_TBL_ID_RFSEQ, 0xb0, 5 },
	{ ACPHY_TBL_ID_RFSEQ, 0xc0, 4 },
	{ ACPHY_TBL_ID_RFSEQ, 0xF9, 3 },
	{ ACPHY_TBL_ID_RFSEQ, 0x100, 9 },
	{ ACPHY_TBL_ID_RFSEQ, 0x14a, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x15a, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x16a, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x18e, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x3cd, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x3cf, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x3dd, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x3df, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x3ed, 1 },
	{ ACPHY_TBL_ID_RFSEQ, 0x3ef, 1 },
	{ ACPHY_TBL_ID_NVRXEVMSHAPINGTBL, 0x1, 3},
	{ ACPHY_TBL_ID_NVRXEVMSHAPINGTBL, 0x3d, 3},
	{0xFFFF, 0xFFFF, 0}
};

/* List of all Radio regs to be saved during FCBS */
static fcbs_radioreg_core_list_entry fcbs_radioreg_list_acphy [ ] =
{
	{ RF1_2069_REV0_TXGM_LOFT_COARSE_I, 0x1 },
	{ RF0_2069_REV0_TXGM_LOFT_COARSE_Q, 0x0 },
	{ RF2_2069_REV0_TXGM_LOFT_FINE_I, 0x2   },
	{ RFP_2069_REV0_PLL_ADC4, 0x4           },
	{ RF0_2069_REV0_TXGM_LOFT_FINE_Q, 0x3           },
	{ RF1_2069_REV0_NBRSSI_BIAS, 0x1                },
	{ RF0_2069_REV0_LPF_BIAS_LEVELS_HIGH, 0x0       },
	{ RF0_2069_REV0_LPF_BIAS_LEVELS_LOW, 0x3        },
	{ RF2_2069_REV0_LPF_BIAS_LEVELS_MID, 0x2        },
	{ RF0_2069_REV0_LOGEN2G_IDAC2, 0x0              },
	{ RFP_2069_REV0_PLL_ADC1, 0x4                   },
	{ RF2_2069_REV0_BG_TRIM2, 0x2                   },
	{ RF0_2069_REV0_PA2G_TSSI, 0x3                  },
	{ RF0_2069_REV0_LNA2G_CFG1, 0x3                 },
	{ RF0_2069_REV0_LNA2G_CFG2, 0x3                 },
	{ RF1_2069_REV0_LNA2G_RSSI, 0x1                 },
	{ RF0_2069_REV0_LNA5G_CFG1, 0x3                 },
	{ RF0_2069_REV0_LNA5G_CFG2, 0x3                 },
	{ RF0_2069_REV0_LNA5G_RSSI, 0x3                 },
	{ RF0_2069_REV0_RXMIX2G_CFG1, 0x3               },
	{ RF0_2069_REV0_RXMIX5G_CFG1, 0x3               },
	{ RF0_2069_REV0_RXRF2G_CFG1, 0x3                },
	{ RF0_2069_REV0_RXRF5G_CFG1, 0x3                },
	{ RF0_2069_REV0_TIA_CFG1,	0x3                 },
	{ RF0_2069_REV0_LPF_MAIN_CONTROLS, 0x3          },
	{ RF0_2069_REV0_LPF_CORNER_FREQUENCY_TUNING, 0x3},
	{ RF0_2069_REV0_TXGM_CFG1, 0x3 },
	{ RF0_2069_REV0_PGA2G_CFG1, 0x3},
	{ RF2_2069_REV0_PGA5G_CFG1, 0x2},
	{ RF0_2069_REV0_PAD5G_CFG1, 0x3},
	{ RF0_2069_REV0_PA5G_CFG1, 0x3 },
	{ RF0_2069_REV0_LOGEN2G_CFG1, 0x0 },
	{ RF0_2069_REV0_LOGEN2G_CFG2, 0x3 },
	{ RF0_2069_REV0_LOGEN5G_CFG2, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE1, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE2, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE3, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE4, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE5, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE6, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE7, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE9, 0x3 },
	{ RF0_2069_REV0_ADC_CALCODE10, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE11, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE12, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE13, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE14, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE15, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE16, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE17, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE18, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE19, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE20, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE21, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE23, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE24, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE25, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE26, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE27, 0x3},
	{ RF0_2069_REV0_ADC_CALCODE28, 0x3},
	{ RFP_2069_REV0_PLL_VCOCAL5, 0x4},
	{ RFP_2069_REV0_PLL_VCOCAL6, 0x4},
	{ RFP_2069_REV0_PLL_VCOCAL2, 0x4},
	{ RFP_2069_REV0_PLL_VCOCAL1, 0x4},
	{ RFP_2069_REV0_PLL_VCOCAL11, 0x4},
	{ RFP_2069_REV0_PLL_VCOCAL12, 0x4},
	{ RFP_2069_REV0_PLL_FRCT2, 0x4},
	{ RFP_2069_REV0_PLL_FRCT3, 0x4},
	{ RFP_2069_REV0_PLL_VCOCAL10, 0x4},
	{ RFP_2069_REV0_PLL_XTAL3, 0x4},
	{ RFP_2069_REV0_PLL_VCO2, 0x4},
	{ RF0_2069_REV0_LOGEN5G_CFG1, 0x0},
	{ RFP_2069_REV0_PLL_VCO8, 0x4},
	{ RFP_2069_REV0_PLL_VCO6, 0x4},
	{ RFP_2069_REV0_PLL_VCO3, 0x4},
	{ RFP_2069_REV0_PLL_XTALLDO1, 0x4},
	{ RFP_2069_REV0_PLL_HVLDO1, 0x4},
	{ RFP_2069_REV0_PLL_HVLDO2, 0x4},
	{ RFP_2069_REV0_PLL_VCO5, 0x4},
	{ RFP_2069_REV0_PLL_VCO4, 0x4},
	{ RFP_2069_REV0_PLL_LF4, 0x4},
	{ RFP_2069_REV0_PLL_LF5, 0x4},
	{ RFP_2069_REV0_PLL_LF7, 0x4},
	{ RFP_2069_REV0_PLL_LF2, 0x4},
	{ RFP_2069_REV0_PLL_LF3, 0x4},
	{ RFP_2069_REV0_PLL_CP4, 0x4},
	{ RFP_2069_REV0_PLL_DSP1, 0x4},
	{ RFP_2069_REV0_PLL_DSP2, 0x4},
	{ RFP_2069_REV0_PLL_DSP3, 0x4},
	{ RFP_2069_REV0_PLL_DSP4, 0x4},
	{ RFP_2069_REV0_PLL_DSP6, 0x4},
	{ RFP_2069_REV0_PLL_DSP7, 0x4},
	{ RFP_2069_REV0_PLL_DSP8, 0x4},
	{ RFP_2069_REV0_PLL_DSP9, 0x4},
	{ RF0_2069_REV0_LOGEN2G_TUNE, 0x0},
	{ RF0_2069_REV0_LNA2G_TUNE, 0x3},
	{ RF0_2069_REV0_TXMIX2G_CFG1, 0x3},
	{ RF0_2069_REV0_PGA2G_CFG2, 0x3},
	{ RF0_2069_REV0_PAD2G_TUNE, 0x3},
	{ RF0_2069_REV0_LOGEN5G_TUNE1, 0x0},
	{ RF0_2069_REV0_LOGEN5G_TUNE2, 0x0},
	{ RF0_2069_REV0_LOGEN5G_RCCR, 0x3},
	{ RF0_2069_REV0_LNA5G_TUNE, 0x3},
	{ RF0_2069_REV0_TXMIX5G_CFG1, 0x3},
	{ RF0_2069_REV0_PGA5G_CFG2, 0x3},
	{ RF0_2069_REV0_PAD5G_TUNE, 0x3},
	{ RFP_2069_REV0_PLL_CP5, 0x4},
	{ RF0_2069_REV0_AFEDIV1, 0x0},
	{ RF0_2069_REV0_AFEDIV2, 0x0},
	{ RF1_2069_REV0_ADC_CFG5, 0x1},
	{ RF2_2069_REV0_ADC_CFG5, 0x2},
	{ RFP_2069_REV0_OVR15, 0x4 },
	{ RFP_2069_REV0_OVR16, 0x4 },
	{ RF1_2069_REV0_NBRSSI_CONFG, 0x1},
	{ RF0_2069_REV0_ADC_CFG1, 0x3},
	{ RF0_2069_REV0_ADC_CFG4, 0x3},
	{ RF0_2069_REV0_ADC_BIAS1, 0x3},
	{ 0xFFFF, 0}
};

/* List of all PHY regs to be saved during FCBS */
static uint16 fcbs_phyreg_list_acphy[83];

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
bool
wlc_phy_prefcbsinit_acphy(phy_info_t *pi, int chanidx)
{
	MOD_PHYREG(pi, ChannelSwitch, ChannelIndicator, (uint16)chanidx);

	return TRUE;
}

uint16
wlc_phy_channelindicator_obtain_acphy(phy_info_t *pi)
{
	return READ_PHYREGFLD(pi, ChannelSwitch, ChannelIndicator);
}

bool
wlc_phy_postfcbsinit_acphy(phy_info_t *pi, int chanidx)
{
	return TRUE;
	/* Nothing to do right now */
}

bool
wlc_phy_fcbsinit_acphy(phy_info_t *pi, int chanidx, chanspec_t chanspec)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	/* Values of these registers may depend on phy_rev. */
	fcbs_phyreg_list_acphy[0] = ACPHY_TxResamplerMuDelta0u(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[1] = ACPHY_TxResamplerMuDelta0l(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[2] = ACPHY_TxResamplerMuDeltaInit0u(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[3] = ACPHY_TxResamplerMuDeltaInit0l(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[4] = ACPHY_TxResamplerMuDelta1u(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[5] = ACPHY_TxResamplerMuDelta1l(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[6] = ACPHY_TxResamplerMuDeltaInit1u(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[7] = ACPHY_TxResamplerMuDeltaInit1l(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[8] = ACPHY_TxResamplerMuDelta2u(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[9] = ACPHY_TxResamplerMuDelta2l(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[10] = ACPHY_TxResamplerMuDeltaInit2u(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[11] = ACPHY_TxResamplerMuDeltaInit2l(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[12] = ACPHY_RfctrlCoreLowPwr0(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[13] = ACPHY_RfctrlCoreLowPwr1(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[14] = ACPHY_RfctrlCoreLowPwr2(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[15] = ACPHY_nvcfg3(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[16] = ACPHY_DcFiltAddress(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[17] = ACPHY_RxFilt40Num00(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[18] = ACPHY_RxFilt40Num01(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[19] = ACPHY_RxFilt40Num02(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[20] = ACPHY_RxFilt40Den00(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[21] = ACPHY_RxFilt40Den01(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[22] = ACPHY_RxFilt40Num10(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[23] = ACPHY_RxFilt40Num11(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[24] = ACPHY_RxFilt40Num12(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[25] = ACPHY_RxFilt40Den10(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[26] = ACPHY_RxFilt40Den11(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[27] = ACPHY_RxStrnFilt40Num00(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[28] = ACPHY_RxStrnFilt40Num01(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[29] = ACPHY_RxStrnFilt40Num02(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[30] = ACPHY_RxStrnFilt40Den00(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[31] = ACPHY_RxStrnFilt40Den01(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[32] = ACPHY_RxStrnFilt40Num10(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[33] = ACPHY_RxStrnFilt40Num11(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[34] = ACPHY_RxStrnFilt40Num12(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[35] = ACPHY_RxStrnFilt40Den10(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[36] = ACPHY_RxStrnFilt40Den11(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[37] = ACPHY_rxfdiqImbCompCtrl(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[38] = ACPHY_RfctrlCoreAfeCfg20(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[39] = ACPHY_RfctrlCoreAfeCfg21(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[40] = ACPHY_RfctrlCoreAfeCfg22(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[41] = ACPHY_crsminpoweroffset0(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[42] = ACPHY_crsminpoweroffsetSub10(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[43] = ACPHY_crsmfminpoweroffset0(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[44] = ACPHY_crsmfminpoweroffsetSub10(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[45] = ACPHY_crsminpoweroffset1(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[46] = ACPHY_crsminpoweroffsetSub11(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[47] = ACPHY_crsmfminpoweroffset1(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[48] = ACPHY_crsmfminpoweroffsetSub11(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[49] = ACPHY_crsminpoweroffset2(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[50] = ACPHY_crsminpoweroffsetSub12(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[51] = ACPHY_crsmfminpoweroffset2(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[52] = ACPHY_crsmfminpoweroffsetSub12(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[53] = ACPHY_Core0RssiClipMuxSel(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[54] = ACPHY_Core1RssiClipMuxSel(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[55] = ACPHY_Core2RssiClipMuxSel(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[56] = ACPHY_Core0FastAgcClipCntTh(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[57] = ACPHY_Core1FastAgcClipCntTh(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[58] = ACPHY_Core2FastAgcClipCntTh(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[59] = ACPHY_TssiEnRate(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[60] = ACPHY_energydroptimeoutLen(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[61] = ACPHY_FSTRMetricTh(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[62] = ACPHY_ClassifierCtrl6(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[63] = ACPHY_ClassifierLogAC1(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[64] = ACPHY_crsacidetectThreshl(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[65] = ACPHY_crsacidetectThreshu(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[66] = ACPHY_crsacidetectThreshlSub1(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[67] = ACPHY_crsacidetectThreshuSub1(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[68] = ACPHY_Core0computeGainInfo(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[69] = ACPHY_Core0clip2GainCodeA(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[70] = ACPHY_Core0Adcclip(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[71] = ACPHY_Core1computeGainInfo(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[72] = ACPHY_Core1clip2GainCodeA(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[73] = ACPHY_Core1Adcclip(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[74] = ACPHY_Core2computeGainInfo(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[75] = ACPHY_Core2clip2GainCodeA(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[76] = ACPHY_Core2Adcclip(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[77] = ACPHY_Core0clip2GainCodeB(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[78] = ACPHY_Core1clip2GainCodeB(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[79] = ACPHY_Core2clip2GainCodeB(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[80] = ACPHY_defer_setClip1_CtrLen(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[81] = ACPHY_defer_setClip2_CtrLen(pi->pubpi.phy_rev);
	fcbs_phyreg_list_acphy[82] = 0xFFFF;

	pi->phy_fcbs->fcbs_phytbl16_list = fcbs_phytbl16_list_acphy;
	pi->phy_fcbs->fcbs_radioreg_list = fcbs_radioreg_list_acphy;
	pi->phy_fcbs->fcbs_phyreg_list = fcbs_phyreg_list_acphy;
	/* Obtain the buffer pointers for the appropriate FCBS channel */
	pi->phy_fcbs->phytbl16_buf[chanidx] = pi_ac->fcbsi->ac_fcbs.phytbl16_buf_ChanA;

	if (pi->phy_fcbs->FCBS_ucode) {
		/* Starting address of the FCBS cache on the on-chip RAM */
		pi->phy_fcbs->cache_startaddr = FCBS_ACPHY_TMPLRAM_STARTADDR;

		/* Shared memory locations for specifying the starting offset
		   of the radio register cache, phytbl16 cache, phytbl32 cache
		   phyreg cache, bphyctrl register and the FCBS channel specific
		   starting cache address
		*/
		pi->phy_fcbs->shmem_radioreg = M_FCBS_ACPHY_RADIOREG;
		pi->phy_fcbs->shmem_phytbl16 = M_FCBS_ACPHY_PHYTBL16;
		pi->phy_fcbs->shmem_phytbl32 = M_FCBS_ACPHY_PHYTBL32;
		pi->phy_fcbs->shmem_phyreg = M_FCBS_ACPHY_PHYREG;
		pi->phy_fcbs->shmem_bphyctrl = M_FCBS_ACPHY_BPHYCTRL;
		pi->phy_fcbs->shmem_cache_ptr = M_FCBS_ACPHY_TEMPLATE_PTR;
	}
	return TRUE;
}

bool
wlc_phy_prefcbs_acphy(phy_info_t *pi, int chanidx)
{
	/* CRDOT11ACPHY-176 :: Timing issues cause the VCO cal not to be triggered during
	 * channel switch. we need to clear these ovr bits before the switch and
	 * set them during switch (throough the FCBS TBL which then triggers the VCO cal
	 */
	/* Reg conflict with 2069 rev 16 */
	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);
	if (RADIOMAJORREV(pi) == 0) {
		MOD_RADIO_REG(pi, RFP, OVR15, ovr_rfpll_vcocal_cal, 0);
		MOD_RADIO_REG(pi, RFP, OVR15, ovr_rfpll_rst_n, 0);
		MOD_RADIO_REG(pi, RFP, OVR16, ovr_rfpll_vcocal_rstn, 0);
	} else {
		MOD_RADIO_REG(pi, RFP, GE16_OVR16, ovr_rfpll_vcocal_cal, 0);
		MOD_RADIO_REG(pi, RFP, GE16_OVR16, ovr_rfpll_rst_n, 0);
		MOD_RADIO_REG(pi, RFP, GE16_OVR17, ovr_rfpll_vcocal_rstn, 0);

	}

	return TRUE;
}

bool
wlc_phy_postfcbs_acphy(phy_info_t *pi, int chanidx)
{
	return TRUE;
	/* Nothing to do right now */
}

bool
wlc_phy_fcbs_acphy(phy_info_t *pi, int chanidx)
{
	uint16 chanspec;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	MOD_PHYREG(pi, ChannelSwitch, ChannelIndicator, (uint16)chanidx);
	MOD_PHYREG(pi, ChannelSwitch, VCO_cal_reqd, 0x1);
	MOD_PHYREG(pi, ChannelSwitch, SwitchTrigger, 0x1);
	SPINWAIT(READ_PHYREG(pi, ChannelSwitch) &
	        (0x1 << ACPHY_ChannelSwitch_SwitchTrigger_SHIFT(pi->pubpi.phy_rev)),
		ACPHY_SPINWAIT_FCBS_SWITCH);
	ASSERT(!(READ_PHYREG(pi, ChannelSwitch) &
	      (0x1 << ACPHY_ChannelSwitch_SwitchTrigger_SHIFT(pi->pubpi.phy_rev))));

	/* Set bandwidth bits in the SI Core flags, which is not cached.
	 * Some radio control lines are set after reset2rx with correct bandwidth bits.
	 * PHY seems to be a weird state after switch and we need rset CCA
	 */
	chanspec = pi->phy_fcbs->chanspec[chanidx];
	if (pi_ac->curr_bw != CHSPEC_BW(chanspec)) {
		pi_ac->curr_bw = CHSPEC_BW(chanspec);
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(chanspec));
	}

	wlc_phy_resetcca_acphy(pi);
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RESET2RX);

	return TRUE;
}
#endif /* ENABLE_FCBS */
