/*
 * ACPHY PAPD CAL module implementation
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
#include <phy_type_papdcal.h>
#include <phy_ac.h>
#include <phy_ac_papdcal.h>

#include <phy_ac_info.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <qmath.h>

/* module private states */
struct phy_ac_papdcal_info {
	phy_info_t			*pi;
	phy_ac_info_t		*aci;
	phy_papdcal_info_t	*cmn_info;
/* add other variable size variables here at the end */
};
#define DO_PAPD_GAINCTRL 1

/* local functions */

/* register phy type specific implementation */
phy_ac_papdcal_info_t *
BCMATTACHFN(phy_ac_papdcal_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_papdcal_info_t *cmn_info)
{
	phy_ac_papdcal_info_t *ac_info;
	phy_type_papdcal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_papdcal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;

	if (phy_papdcal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_papdcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_papdcal_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_papdcal_unregister_impl)(phy_ac_papdcal_info_t *ac_info)
{
	phy_papdcal_info_t *cmn_info;
	phy_info_t *pi;

	ASSERT(ac_info);
	cmn_info = ac_info->cmn_info;
	pi = ac_info->pi;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_papdcal_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_ac_papdcal_info_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
#define PAPD_GAIN_CTRL
#define ACPHY_SPINWAIT_PAPDCAL			5000000

static int8 GAIN_CTRL = 0; /* Use this flag for gain control using analytic PAPD */
static int8 *rfpwrlut_ptr;

static int8 pga_gain_array_2g[256] = {-14, -14, -14, -14, -14, -14, -14,
    -14, -14, -14, -14, -14, -14, -14, -14, -14,
    -13, -12, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -4, -3, -2, -2,
     -1,  0,  0,  1,  1,  2,  2,  3,  3,  3,  4,  4,  5,  5,  5,  6,
      6,  6,  6,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9, 10, 10, 10,
     10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14,
     14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 17, 17,
     17, 17, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 20,
     20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 22, 21,
     21, 22, 21, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24,
     24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25,
     26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27,
     27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
     29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30,
     30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31,
     31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
     32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33};

static int8  pga_gain_array_5g_0[256] = {-43, -37, -28, -22, -18, -14, -11, -9, -7,
	-5, -3, -2, 0, 1, 3, 4,
	5,  6,  7,  8,  9,  9, 10, 11, 11, 12, 12, 13, 14, 14, 15, 15,
	16, 16, 17, 17, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22,
	22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 26,
	26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29,
	29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 32, 32,
	32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34,
	34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38,
	38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39,
	39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
	41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
	42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
	43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45};

static int8  pga_gain_array_5g_1[256] = {-44, -37, -28, -22, -18, -14, -11, -9, -7, -5,
	-3, -2, 0, 1, 2, 4,
	4,  5,  6,  7,  8,  9, 10, 11, 11, 11, 12, 13, 13, 14, 15, 15,
	16, 16, 17, 17, 18, 18, 18, 19, 19, 20, 20, 20, 20, 21, 21, 21,
	22, 22, 22, 23, 23, 23, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
	25, 26, 26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29,
	29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31,
	31, 31, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33,
	33, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35,
	35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39,
	39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40,
	40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41,
	41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42,
	42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43,
	43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44};

static int8  pga_gain_array_5g_2[256] = {-43, -37, -28, -22, -18, -14, -11, -9, -7, -5,
	-3, -2, 0, 1, 2, 4,
	4,  5,  6,  7,  8,  9, 10, 10, 10, 11, 12, 13, 13, 14, 14, 15,
	15, 16, 16, 17, 17, 18, 18, 19, 19, 19, 19, 20, 20, 21, 21, 21,
	21, 22, 21, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 25, 25, 25,
	25, 25, 26, 26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29,
	29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31,
	31, 31, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33,
	33, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35,
	35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39,
	39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40,
	40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41,
	41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42,
	42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43,
	43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
	43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44};

static int8  pga_gain_array_5g_3[256] =  {-45, -37, -27, -21, -18, -14, -11, -8, -7, -4, -3,
	-1, 0, 1, 3, 4,
	5,  6,  7,  8,  8,  9, 10, 10, 11, 12, 12, 13, 14, 14, 15, 16,
	16, 16, 17, 17, 18, 18, 19, 19, 19, 20, 20, 21, 20, 21, 21, 21,
	21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 26, 26,
	26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29,
	29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 32, 32, 32,
	32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34,
	34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36,
	36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38,
	38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39,
	39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
	41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
	42, 42, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
	43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45};

static int8 pga_gain_array_2g_4354[256] = {	-23, -23, -23, -23, -23, -23,
	-23, -23, -20, -20, -18, -17, -15, -14, -13, -12,
	-12, -12, -11, -10, -9, -8, -7, -6, -6, -6, -6, -5, -5, -5, -5, -4,
	-4, -4, -4, -4, -4, -4, -4, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -1, -1, -1,  0,  0,  0,  0,  1,  1,  1,  2,  2,  2,  2,
	3,  3,  4,  4,  4,  4,  4,  6,  7,  7,  7,  7,  7,  7,  7,  7,
	7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,
	9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 12, 12,
	12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17,
	17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25};

static int8 pga_gain_array_5g_4354[256] = {-45, -45, -37, -32, -29, -25, -23,
	-20, -18, -16, -14, -13, -12, -10, -9, -8,
	-7, -6, -5, -4, -4, -3, -2, -1,  0,  0,  1,  2,  2,  3,  3,  4,
	4,  5,  5,  6,  6,  7,  7,  8,  8,  8,  9,  9,  9, 10, 10, 11,
	11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15,
	15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19,
	19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 22, 22, 22,
	22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24,
	24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31,
	31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35,
	35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 36, 37};

static int8 pga_gain_array_2g_epapd[2][256] = {{ -31, -27, -24, -21, -18,
	-15, -13, -10, -10, -8, -7, -5, -4, -2, -1,  0,
	1,  2,  3,  4,  5,  6,  6,  7,  7,  8,  9,  9, 10, 11, 11, 12,
	13, 13, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19,
	19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 22, 22, 23, 23, 23, 24,
	24, 25, 25, 25, 25, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 28,
	28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30,
	31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33,
	33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35,
	35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38,
	38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39,
	39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42,
	42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43,
	43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46},
	{ -31, -27, -24, -21, -18, -15, -13, -10, -10, -8, -7, -5, -4, -2, -1, 0,
	1,  2,  3,  4,  5,  6,  6,  7,  7,  8,  9,  9, 10, 11, 11, 12,
	13, 13, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19,
	19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 22, 22, 23, 23, 23, 24,
	24, 25, 25, 25, 25, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 28,
	28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30,
	31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33,
	33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35,
	35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38,
	38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39,
	39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42,
	42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43,
	43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44,
	44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46}};

static int8 pga_gain_array_5g_epapd_0[256] = {-14, -14, -14, -14, -14, -14, -14,
	-11, -11, -11, -11, -11, -10, -9, -8, -7, -6, -5, -5, -5,
	-5, -5, -4, -3, -2, -2, -1, -1,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  5,
	6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 12, 12, 12, 13, 13, 13, 13, 14, 14,
	14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 19,
	19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22,
	22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
	25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31,
	31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35,
	35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 37, 37};

static int8 pga_gain_array_5g_epapd_1[256] = { -13, -13, -13, -13, -13, -13, -13,
	-10, -10, -10, -10, -10, -9, -8, -7, -6, -5, -5, -4, -4,
	-3, -3, -2, -2, -1, -1,  0,  0,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,
	6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 12, 12, 12, 13, 13, 13, 13, 14, 14,
	14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 19,
	19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22,
	22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
	25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31,
	31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35,
	35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 37, 37};

static int8 pga_gain_array_5g_epapd_2[256] = {-14, -14, -14, -14, -14, -14, -14,
	-11, -11, -11, -11, -11, -10, -9, -8, -7, -6, -5, -4, -3,
	-2, -2, -1,  0,  1,  1, -1, -1,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  5,
	6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 12, 12, 12, 13, 13, 13, 13, 14, 14,
	14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 19,
	19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22,
	22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
	25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31,
	31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35,
	35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 37, 37};

static int8 pga_gain_array_5g_epapd_3[256] = {-16, -16, -16, -16, -16, -16, -16,
	-13, -13, -13, -13, -13, -12, -11, -10, -9, -8, -7, -6, -5,
	-4, -4, -3, -2, -1, -1,  0,  0,  1,  2,  2,  3,  3,  4,  4,  4,  5,  5,  5,  6,
	6,  6,  7,  7,  7,  8,  8,  8,  9,  9,  9, 12, 12, 12, 13, 13, 13, 13, 14, 14,
	14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 19,
	19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22,
	22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
	25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31,
	31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
	34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35,
	35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 37, 37};

#ifdef PAPD_GAIN_CTRL
static uint16 wlc_phy_papd_cal_gain_cntl_acphy(phy_info_t *pi, uint16 num_iter, uint8 core,
	uint16 startindex, uint16 yrefindex, uint16 stopindex); /* Todo: REMOVE TAG HEZI */
static void wlc_phy_write_tx_gain_acphy(phy_info_t *pi, uint8 core_no,
	acphy_txgains_t *target_gain, uint16 * bbmult);
#endif /* PAPD_GAIN_CTRL */

static void wlc_phy_papdcal_radio_cleanup_acphy(phy_info_t *pi);
static void wlc_phy_papd_radio_loopback_setup_acphy(phy_info_t *pi, uint16 tx_atten,
	uint16 rx_atten);
static int wlc_phy_tx_tone_acphy_papd(phy_info_t *pi, int32 f_kHz, uint16 max_val, uint8 iqmode,
	uint8 mac_based, bool modify_bbmult);
static uint16 wlc_phy_gen_load_samples_acphy_papd(phy_info_t *pi, int32 f_kHz, uint16 max_val,
	uint8 mac_based);
static void assign_sample80_buffer(math_cint32 *tone_buf, int16 t);
static void assign_sample40_buffer(math_cint32 *tone_buf, int16 t);
static void assign_sample20_buffer(math_cint32 *tone_buf, int16 t);
static uint8 wlc_phy_txpwr_idx_cur_get_acphy(phy_info_t *pi, uint8 core);
static void wlc_phy_get_tx_gain_acphy(phy_info_t *pi, uint8 core_no, acphy_txgains_t *target_gain);
static void wlc_phy_papd_smooth_acphy(phy_info_t *pi, uint8 core,
	uint32 winsz, uint32 start, uint32 end);
static void wlc_phy_papd_radio_loopback_setup_acphy_tiny(phy_info_t *pi,
	uint16 tx_atten, uint16 rx_atten);
static void wlc_phy_papd_rx_gain_ctrl_acphy(phy_info_t *pi);
static void wlc_phy_papd_radio_cleanup_acphy_tiny(phy_info_t *pi);
static void wlc_phy_tia_gain_tiny(phy_info_t *pi, uint16 gain, uint8 core);

#ifdef PAPD_GAIN_CTRL
static uint16
wlc_phy_papd_cal_gain_cntl_acphy(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
	uint16 yrefindex, uint16 stopindex)
{
#define EPS_MAX 4095
#define EPS_MIN -4095
	bool clipping_flag = 0;
	uint8 i = 0, num_gain_itr_max = 8;
	uint8 PAPD_FAST_GAIN_CTRL = 0;
	uint16 pga_u = 255, pga_l = 0, pga_mid = 127;
	uint32 tempclip = 0;
	uint32 clipthresholdl = 47894000, clipthresholdu = 49034000, clipthreshold = 48462000;
	acphy_txgains_t tx_gains;
	int32 eps_re, eps_im;
	uint16 bbmult;
	uint32 eps_complex;
	uint16 numidx;
	uint8 papdmode = pi->u.pi_acphy->papdmode;
	uint8 PAgain = 0xff;
	uint8 Gainoverwrite = 0;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};
	if (ACMAJORREV_1(pi->pubpi.phy_rev))
		PAPD_FAST_GAIN_CTRL = 1;

	bbmult = 0x3f;
	tx_gains.txlpf = 0x0;
	tx_gains.txgm = 0xff;
	tx_gains.pga = 0xff;
	tx_gains.pad = 0xff;
	if (PHY_IPA(pi)) {
		Gainoverwrite = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			pi->u.pi_acphy->srom_pagc2g_ovr :
			pi->u.pi_acphy->srom_pagc5g_ovr;
		PAgain = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			pi->u.pi_acphy->srom_pagc2g :
			pi->u.pi_acphy->srom_pagc5g;
	}
	tx_gains.ipa = (Gainoverwrite == 0) ? 0xff : PAgain;
	if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
		/* 4350EPAPD */
		tx_gains.txlpf = 0x0;
		tx_gains.txgm = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0x67 : 0x91;
		tx_gains.pga = 0xff;
		tx_gains.pad = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xff : 0x7f;
		tx_gains.ipa = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xc0 : 0x3;
	}

	if (PAPD_FAST_GAIN_CTRL) {
		PHY_CAL(("---------- Using Fast Gain Control ------------"));
		numidx = 4;
		if ((papdmode == PAPD_ANALYTIC) || (papdmode == PAPD_ANALYTIC_WO_YREF)) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				bbmult = 0x30;
				if (!CHSPEC_IS80(pi->radio_chanspec)) {
					clipthresholdl = 59901000;
					clipthresholdu = 61175000;
					clipthreshold = 60536000;
					/* 1.9 */
				} else {
					clipthresholdl = 47894000;
					clipthresholdu = 49034000;
					clipthreshold = 48462000;
					/* 1.7 */
				}
			}
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				clipthresholdl = 30110000;
				clipthresholdu = 31016000;
				/* clipthreshold 1.35 */
				clipthreshold = 30562000;
			}
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				bbmult = 0x30;
				if (!CHSPEC_IS80(pi->radio_chanspec)) {
					clipthresholdl = 39255000;
					clipthresholdu = 40288000;
					clipthreshold = 39769000;
				} else {
					clipthresholdl = 32399000;
					clipthresholdu = 33338000;
					clipthreshold = 32867289;
					/* 1.2 */
				}
			}
		}
	} else {
		numidx = 16;
		if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				bbmult = 0x30;
				if (!CHSPEC_IS80(pi->radio_chanspec)) {
					clipthresholdl = 59901000;
					clipthresholdu = 61175000;
					clipthreshold = 60536000;
				}
			}
		}
	}
	if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			clipthresholdl = 13886000;
			clipthresholdu = 14504000;
			clipthreshold = 14193000;
			pga_u = 127;
			pga_l = 40;
			pga_mid = 84;
		} else {
			if (CHSPEC_IS80(pi->radio_chanspec)) {
				clipthresholdl = 34772000;
				clipthresholdu = 35745000;
				clipthreshold = 35257000;
			}
		}
	}

	/* Binary search */
	for (i = 1; i <= num_gain_itr_max; i++) {
		tx_gains.pga = pga_mid;
		wlc_phy_write_tx_gain_acphy(pi, core, &tx_gains, &bbmult);
		/* TODO: check start and stop indexs do we want to check more than the last index */
		wlc_phy_papd_cal_acphy(pi, num_iter, core, stopindex - numidx + 1,
		    yrefindex, stopindex);
		wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core], 1, 63, 32,
			&eps_complex, core);
		wlc_phy_papd_decode_epsilon(eps_complex, &eps_re, &eps_im);
		tempclip =   ((4095+eps_re)*(4095+eps_re))+  (eps_im*eps_im);
		if (tempclip >= clipthreshold)
		    clipping_flag = 1;
		else
		    clipping_flag = 0;

		if (tempclip >= clipthresholdl && tempclip <= clipthresholdu) {
		    break;
		}

		if (clipping_flag)
			pga_u = pga_mid;
		else
		    pga_l = pga_mid;

		pga_mid = (pga_u + pga_l)/ 2;
	}
	return tx_gains.pga;
}

/* gain _ctrl for Tiny papd */

static uint8
wlc_phy_papd_cal_gain_cntl_tiny(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
	uint16 yrefindex, uint16 stopindex)
{
#define EPS_MAX 4095
#define EPS_MIN -4095
#define NUM_ITER_BINARY_SEARCH 7
	uint8 i;
	uint8 txidx_u = 0, txidx_l = 126, txidx_mid = 63;
	uint32 tempclip = 0;
	uint32 clipthresholdl, clipthresholdu, clipthreshold;
	int32 eps_re, eps_im;
	uint32 eps_complex;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (ACMAJORREV_4(pi->pubpi.phy_rev) && ACMINORREV_1(pi)) {
			clipthreshold = 42928704;
			clipthresholdl = 41862194;
			clipthresholdu  = 44008629;
		} else {
			clipthreshold = 29662728;
			clipthresholdl = 27046760;
			clipthresholdu  = 32867289;
		}
	} else {
		clipthreshold = 48462000;
		clipthresholdl = 47894000;
		clipthresholdu = 49034000;
	}

	/* Binary search */
	for (i = 1; i <= NUM_ITER_BINARY_SEARCH; i++) {
		wlc_phy_txpwr_by_index_acphy(pi, (uint8)(core+1), txidx_mid);
		/* TODO: check start and stop indexs do we want to check more than the last index */
		wlc_phy_papd_cal_acphy(pi, num_iter, core, stopindex - 10,
			yrefindex, stopindex);
		wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core], 1, 63, 32,
			&eps_complex, core);
		wlc_phy_papd_decode_epsilon(eps_complex, &eps_re, &eps_im);
		tempclip =   ((4095+eps_re)*(4095+eps_re))+(eps_im*eps_im);
		if (tempclip >= clipthresholdl && tempclip <= clipthresholdu)
		    break;
		if (tempclip >= clipthreshold)
			txidx_u = txidx_mid;
		else
			txidx_l = txidx_mid;

		txidx_mid = (txidx_u+txidx_l)/2;
	}
	return txidx_mid;
}

static void
wlc_phy_write_tx_gain_acphy(phy_info_t *pi, uint8 core_no, acphy_txgains_t *target_gain,
	uint16 *bbmult)
{
	uint8 core;
	uint8 stall_val = 0;
	uint16 curr_gains_0, curr_gains_1, curr_gains_2;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	ACPHY_DISABLE_STALL(pi);

	curr_gains_0 = (target_gain->txlpf & 0xFF) | ((target_gain->ipa << 8) & 0xFF00);
	curr_gains_1 = (target_gain->pad & 0xFF) | ((target_gain->pga << 8) & 0xFF00);
	curr_gains_2 = (target_gain->txgm & 0xff);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&curr_gains_0);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&curr_gains_1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&curr_gains_2);
		wlc_phy_set_tx_bbmult_acphy(pi, bbmult, core);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}
#endif /* PAPD_GAIN_CTRL */

static void
wlc_phy_papdcal_radio_cleanup_acphy(phy_info_t *pi)
{
	uint8 core;
	uint16 radio_rev_id;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		radio_rev_id = READ_RADIO_REGC(pi, RF, REV_ID, core);

		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainI, 0);
		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainQ, 0);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pa2g_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pad2g_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_cal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_cal_pa_atten_2g, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_rx_attn, 0);
			MOD_RADIO_REGC(pi, RXRF2G_CFG1, core, pwrsw_en, 1);
/*
			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
				radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf2g_pwrsw_en, 1);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf2g_pwrsw_en, 1);
			}
*/

		} else {
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pa_pu_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pad_pu_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_cal_pa_atten_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_rx_attn, 0);

			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
				radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf5g_pwrsw_en, 0);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf5g_pwrsw_en, 0);
			}

		}
	}

/*
	ASSERT(porig->is_orig);
	porig->is_orig = FALSE;
*/

	if (ACREV_IS(pi->pubpi.phy_rev, 2)) {
		MOD_PHYREG(pi, RxFeCtrl1, rxfe_bilge_cnt, 0);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);
	}

}

static void
wlc_phy_papd_radio_loopback_setup_acphy(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten)
{
	uint16 radio_rev_id;
	uint8 core;

	ASSERT(RADIOID(pi->pubpi.radioid) == BCM2069_ID);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		radio_rev_id = READ_RADIO_REGC(pi, RF, REV_ID, core);

		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainI, 1);
		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainQ, 1);
		MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 2);
		MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 2);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (RADIOREV(pi->pubpi.radiorev) == 25) {
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 0);
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 0);
			}
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pa2g_pu, 1);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pad2g_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_cal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_cal_pa_atten_2g, tx_atten);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core,
			               loopback2g_papdcal_rx_attn, rx_atten);

			/* Enable the vdd switch on mixer */
			MOD_RADIO_REGC(pi, RXRF2G_CFG1, core, pwrsw_en, 1);
			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
				radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf2g_pwrsw_en, 1);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf2g_pwrsw_en, 1);
			}
			if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
				/* 4350EPAPD */
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 4);
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 4);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pa2g_pu, 0);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pad2g_pu, 0);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_cal_pu, 0);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_pu, 1);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_rxiqcal_pu, 0);
				MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 1);
				MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_attn, 0);
			}
		} else {
			if (RADIOREV(pi->pubpi.radiorev) == 25) {
				if (CHSPEC_IS80(pi->radio_chanspec)) {
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 4);
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 4);
				} else {
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 2);
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 2);
				}
			}
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pa_pu_5g, 1);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pad_pu_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_cal_pa_atten_5g, tx_atten);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core,
				loopback5g_papdcal_rx_attn, rx_atten);
			MOD_RADIO_REGC(pi, RXRF5G_CFG1, core, pwrsw_en, 1);
			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
			    radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf5g_pwrsw_en, 1);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf5g_pwrsw_en, 1);
			}
			if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
				/* 4350EPAPD */
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 7);
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 7);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pa_pu_5g, 0);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pad_pu_5g, 0);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 0);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_pu, 1);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_rxiqcal_pu, 0);
				MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_en, 1);
				MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_attn, 0);
			}
		}
		/* #Powerdown LNA1, LNA2 */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna1_pwrup, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna2_pwrup, 0);

		/* acphy_rfctrl_override lpf_sw rxiq_rx2 $core; */
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_aux_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_aux_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_iqcal_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_iqcal_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_rc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_bq2, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_bq2, 1);

		/* acphy_rfctrl_override lpf_pu_dc 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_pu_dc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_pu_dc, 1);

		/* acphy_rfctrl_override tia_DC_loop_PU 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, tia_DC_loop_PU, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, tia_DC_loop_PU, 1);

		/* acphy_rfctrl_override fast_nap_bias_pu 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, fast_nap_bias_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, fast_nap_bias_pu, 1);

		/* acphy_rfctrl_override rxrf_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);

		/* acphy_rfctrl_override logen_rx_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, logen_rx_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, logen_rx_pwrup, 1);
	}
}

static int
wlc_phy_tx_tone_acphy_papd(phy_info_t *pi, int32 f_kHz, uint16 max_val, uint8 iqmode,
	uint8 mac_based, bool modify_bbmult)
{
	uint8 core;
	uint16 num_samps;
	uint16 bb_mult;
	uint16 loops = 0xffff;
	uint16 wait = 0;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (max_val == 0) {
		/* XXX PR90390: For a zero ampltitude signal, bypass loading samples
		 * and set bbmult = 0
		 */
		num_samps = 1;
	} else if ((num_samps = wlc_phy_gen_load_samples_acphy_papd(pi, f_kHz, max_val, mac_based))
			   == 0) {
		return BCME_ERROR;
	}

	if (pi_ac->bb_mult_save_valid == 0) {
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			wlc_phy_get_tx_bbmult_acphy(pi, &pi_ac->bb_mult_save[core], core);
		}
		pi_ac->bb_mult_save_valid = 1;
	}

	/* XXX	max_val = 0, set bbmult = 0
	 * elseif modify_bbmult = 1,
	 * set samp_play (default mag 186) power @ DAC = OFDM packet power @ DAC (9.5-bit RMS)
	 * by setting bb_mult (2.6 format) to
	 * 64/64 for bw = 20, 40, 80MHz
	 */
	if (max_val == 0 || modify_bbmult) {
		if (max_val == 0) {
			bb_mult = 0;
		} else {
			if (CHSPEC_IS80(pi->radio_chanspec))
				bb_mult = 64;
			else if (CHSPEC_IS40(pi->radio_chanspec))
				bb_mult = 64;
			else
				bb_mult = 64;
		}
		FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
			wlc_phy_set_tx_bbmult_acphy(pi, &bb_mult, core);
		}
	}

	wlc_phy_runsamples_acphy(pi, num_samps, loops, wait, iqmode, mac_based);

	return BCME_OK;
}

static uint16
wlc_phy_gen_load_samples_acphy_papd(phy_info_t *pi, int32 f_kHz, uint16 max_val, uint8 mac_based)
{
	uint8 phy_bw;
	uint16 num_samps, t;
	math_cint32* tone_buf = NULL;

	/* check phy_bw */
	if (CHSPEC_IS80(pi->radio_chanspec))
		phy_bw = 80;
	else if (CHSPEC_IS40(pi->radio_chanspec))
		phy_bw = 40;
	else
		phy_bw = 20;

	/* allocate buffer */
	if ((tone_buf = (math_cint32 *)MALLOC(pi->sh->osh, sizeof(math_cint32) * phy_bw)) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		          __FUNCTION__, MALLOCED(pi->sh->osh)));
		return 0;
	}

	/* set up params to generate tone */
	num_samps  = (uint16)phy_bw;

	/* tone freq = f_c MHz ; phy_bw = phy_bw MHz ; # samples = phy_bw (1us) */
	for (t = 0; t < num_samps; t++) {
		/* produce sample values for play buffer */
		if (CHSPEC_IS80(pi->radio_chanspec)) {
			assign_sample80_buffer(tone_buf, t);
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			assign_sample40_buffer(tone_buf, t);
		} else {
			assign_sample20_buffer(tone_buf, t);
		}
		if (TINY_RADIO(pi)) {
			int32 swap = tone_buf[t].i;
			tone_buf[t].i = tone_buf[t].q;
			tone_buf[t].q = swap;
		}
	}

	/* load sample table */
	wlc_phy_loadsampletable_acphy(pi, tone_buf, num_samps, TRUE, FALSE);

	if (tone_buf != NULL)
		MFREE(pi->sh->osh, tone_buf, sizeof(math_cint32) * phy_bw);

	return num_samps;
}

static void
assign_sample80_buffer(math_cint32 *tone_buf, int16 t)
{
	int16 imSamp80[80] = {186, 185, 184, 181, 177, 172, 166, 159, 150, 141,
		132,  121,  109,   97,   84,   71,   57,   43,   29,   15,
		0,  -15,  -29,  -43,  -57,  -71,  -84,  -97, -109, -121,
		-132, -141, -150, -159, -166, -172, -177, -181, -184, -185,
		-186, -185, -184, -181, -177, -172, -166, -159, -150, -141,
		-132, -121, -109,  -97,  -84,  -71,  -57,  -43,  -29,  -15,
		0,   15,   29,   43,   57,   71,   84,   97,  109,  121,
		132,  141,  150,  159,  166,  172,  177,  181,  184,  185};

	int16 realSamp80[80] = {0, 15, 29, 43, 57, 71, 84, 97, 109, 121,
		132,  141,  150,  159,  166,  172,  177,  181,  184,  185,
		186,  185,  184,  181,  177,  172,  166,  159,  150,  141,
		132,  121,  109,   97,   84,   71,   57,   43,   29,   15,
		0,  -15,  -29,  -43,  -57,  -71,  -84,  -97, -109, -121,
		-132, -141, -150, -159, -166, -172, -177, -181, -184, -185,
		-186, -185, -184, -181, -177, -172, -166, -159, -150, -141,
		-132, -121, -109,  -97,  -84,  -71,  -57,  -43,  -29,  -15};

	ASSERT(tone_buf);
	ASSERT(t < ARRAYSIZE(imSamp80) && t < ARRAYSIZE(realSamp80));

	tone_buf[t].q = (imSamp80[t]);
	tone_buf[t].i = (realSamp80[t]);
}

static void
assign_sample40_buffer(math_cint32 *tone_buf, int16 t)
{
	int16 realSamp40[40] = {0, 29, 57, 84, 109, 132, 150, 166, 177, 184,
		186,  184,  177,  166,  150,  132,  109,   84,   57,   29,
		0,  -29,  -57,  -84, -109, -132, -150, -166, -177, -184,
		-186, -184, -177, -166, -150, -132, -109,  -84,  -57,  -29};

	int16 imSamp40[40] = {186, 184, 177, 166, 150, 132, 109, 84, 57, 29,
		0,  -29,  -57,  -84, -109, -132, -150, -166, -177, -184,
		-186, -184, -177, -166, -150, -132, -109,  -84,  -57,  -29,
		0,   29,   57,   84,  109,  132,  150,  166,  177,  184};

	ASSERT(tone_buf);
	ASSERT(t < ARRAYSIZE(imSamp40) && t < ARRAYSIZE(realSamp40));

	tone_buf[t].q = (imSamp40[t]);
	tone_buf[t].i = (realSamp40[t]);
}

static void
assign_sample20_buffer(math_cint32 *tone_buf, int16 t)
{
	int16 realSamp20[20] = {0,  57,  109,  150,  177,  186,  177,  150,  109,  57,
		0, -57, -109, -150, -177, -186, -177, -150, -109, -57};

	int16 imSamp20[20] = {186,  177,  150,  109,  57, 0, -57, -109, -150, -177,
		-186, -177, -150, -109, -57, 0, 57, 109,  150,  177};

	ASSERT(tone_buf);
	ASSERT(t < ARRAYSIZE(imSamp20) && t < ARRAYSIZE(realSamp20));

	tone_buf[t].q = (imSamp20[t]);
	tone_buf[t].i = (realSamp20[t]);
}

static uint8
wlc_phy_txpwr_idx_cur_get_acphy(phy_info_t *pi, uint8 core)
{
	uint16 pwrCtrlStatus = READ_PHYREGCE(pi, TxPwrCtrlStatus_path, core);
	uint8 pwrIndex = 128;

	if (pwrCtrlStatus & 0xF000) {
		pwrIndex = ((pwrCtrlStatus & 0x07F0)>>8);
	}
	return (pwrIndex);
}

static void
wlc_phy_get_tx_gain_acphy(phy_info_t *pi, uint8 core_no, acphy_txgains_t *target_gain)
{
	uint16 curr_gains_0 = 0, curr_gains_1 = 0, curr_gains_2 = 0;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	if (pi->acphy_txpwrctrl == PHY_TPC_HW_OFF) {
		/* read current tx gain from RFSeq table and use as target_gain */

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core_no), 16,
			&curr_gains_0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core_no), 16,
			&curr_gains_1);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core_no), 16,
			&curr_gains_2);

		/* extract gain values */
		target_gain->txlpf = (uint16) ((curr_gains_0 & 0xFF));
		target_gain->ipa  = (uint16) ((curr_gains_0 & 0xFF00) >> 8);
		target_gain->pad  = (uint16) ((curr_gains_1 & 0xFF)   >> 0);
		target_gain->pga  = (uint16) ((curr_gains_1 & 0xFF00) >> 8);
		target_gain->txgm = (uint16) ((curr_gains_2 & 0xFF)   >> 0);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
wlc_phy_papd_radio_loopback_setup_acphy_tiny(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_radioregs_t *porig = (pi_ac->ac_txcal_radioregs_orig);
	uint8 core;

	ASSERT(TINY_RADIO(pi));

	FOREACH_CORE(pi, core) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* PAPD loopback in g-band */

			MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
				ovr_gm2g_pwrup, 1);
			/* AUXLNA2 Power Up */
			MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 1);
			if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
				/* LNA2 Power Up */
				if ((IS_4349A2_RADIO(pi)) == 1) {
					MOD_RADIO_REG_20693(pi, LNA2G_CFG2, core,
						gm2g_pwrup, 0);
				} else {
					MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_pwrup, 1);
				}
			} else {
				/* LNA2 Power Down */
				MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_pwrup, 0);
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_gm2g_auxgm_pwrup, 1);
			}

			if (!PHY_EPAPD(pi)) {
				/* Enable ipapd */
				MOD_RADIO_REG_TINY(pi, TX2G_MISC_CFG1, core, cal2g_pa_pu, 1);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core,
					loopback2g_papdcal_pu, 1);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
				MOD_RADIO_REG_TINY(pi, TX2G_MISC_CFG1, core,
					cal2g_pa_atten, tx_atten);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core,
					rf2g_papdcal_rx_attn, rx_atten);
				if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, core,
						ovr_lna2g_lna2_gain, 1);
					MOD_RADIO_REG_20693(pi, LNA2G_CFG2, core,
						lna2g_lna2_gain, 0);
					/* LNA1 Kill switch */
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_lna2g_tr_rx_en, 1);
					MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core,
						lna2g_tr_rx_en, 0);
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_lna2g_lna1_Rout, 1);
					if (HW_RADIOREV(pi->pubpi.radiorev) == 13) {
						MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core,
							lna2g_lna1_Rout, 0xb);
						MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
							ovr_lna2g_lna1_out_short_pu, 1);
						MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core,
							lna2g_lna1_out_short_pu, 1);
					} else {
						MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core,
							lna2g_lna1_Rout, 0);
					}
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_lna2g_lna1_pu, 0);
				}
			} else {
				/* Enable epapd */
				MOD_RADIO_REG_TINY(pi, TX2G_MISC_CFG1, core, cal2g_pa_pu, 0);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core,
					loopback2g_papdcal_pu, 0);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 1);
				MOD_RADIO_REG_TINY(pi, LNA2G_CFG1, core, lna2g_lna1_bypass, 1);
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_NORTH, core,
					ovr_lna2g_lna1_bypass, 1);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core,
					lna2g_epapd_attn, rx_atten);
			}
		} else {
			/* PAPD loopback in a-band */

			MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core, ovr_gm5g_pwrup, 1);
			/* AUXLNA2 Power Up */
			if (((IS_4349A2_RADIO(pi)) == 1) &&
				(RADIOID(pi->pubpi.radioid) == BCM20693_ID)) {
					MOD_RADIO_REG_20693(pi, LNA5G_CFG2, core,
						lna5g_pu_auxlna2, 0);
					MOD_RADIO_REG_20693(pi, SPARE_CFG16, core,
						loopback5g_gm5g_pu, 1);

			} else {
			MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_auxlna2, 1);
			}
			if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
				if ((IS_4349A2_RADIO(pi)) == 1) {
				MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_lna2, 0);
				} else {
				/* LNA2 Power Up */
				MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_lna2, 1);
				}
			} else {
				/* power down rxgm5g */
				MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_lna2, 0);
				/* powerup auxgm5g */
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_pu_auxlna2, 1);
			}

			if (!PHY_EPAPD(pi)) {
				/* Enable ipapd */
				MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core, cal5g_pa_pu, 1);
				MOD_RADIO_REG_TINY(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 1);
				MOD_RADIO_REG_TINY(pi, PA5G_CFG1, core, rf5g_epapd_en, 0);
				MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core,
				                    cal5g_pa_atten, tx_atten);
				MOD_RADIO_REG_TINY(pi, RXRF5G_CFG2, core,
				                    loopback5g_papdcel_rx_attn, rx_atten);
				if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
					MOD_RADIO_REG_20693(pi, LNA5G_CFG1, core,
						lna5g_lna2_gain, 0);
					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR2, core,
					                    ovr_lna5g_lna2_gain, 1);
					MOD_RADIO_REG_TINY(pi, LNA5G_CFG1, core, lna5g_tr_rx_en, 1);
					MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
					                    ovr_lna5g_tr_rx_en, 1);
				}
			} else {
				/* Enable  */
				MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core, cal5g_pa_pu, 0);
				MOD_RADIO_REG_TINY(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 0);
				MOD_RADIO_REG_TINY(pi, PA5G_CFG1, core, rf5g_epapd_en, 1);
				MOD_RADIO_REG_TINY(pi, LNA5G_CFG1, core, lna5g_lna1_bypass, 1);
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
				                    ovr_lna5g_lna1_bypass, 1);
				MOD_RADIO_REG_TINY(pi, RXRF5G_SPARE, core,
				                    rf5g_epapd_attn, rx_atten);
			}
		}

		/* PHY register writes */
		/* Farrow */
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 1);
		if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
			MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 1);
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);
		} else {
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0,
				(CHSPEC_IS5G(pi->radio_chanspec)) ? 0 : 2);
		}

		/* #Powerdown LNA1 */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna1_pwrup, 0);

		/* #Powerup LNA2 for 20693 & power up otherwise */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna2_pwrup,
			(RADIOID(pi->pubpi.radioid) == BCM20693_ID) ? 1 : 0);
		/* Configure the LPF switches and powerup the DC loop */
		/* MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, 0x3ff, 0x3ff);	*/
		/* MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, 0x3ff, 0x151);		*/
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_rc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_rc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_bq2, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_bq2, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_aux_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_aux_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_iqcal_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_iqcal_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_tia_bq1, 0);

		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_adc, 1);

		/* acphy_rfctrl_override lpf_pu_dc 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_pu_dc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_pu_dc, 1);

		/* acphy_rfctrl_override tia_DC_loop_PU 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, tia_DC_loop_PU, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, tia_DC_loop_PU, 1);

		/* acphy_rfctrl_override fast_nap_bias_pu 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, fast_nap_bias_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, fast_nap_bias_pu, 0);

		/* acphy_rfctrl_override rxrf_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);

		/* acphy_rfctrl_override logen_rx_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, logen_rx_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, logen_rx_pwrup, 1);

		MOD_RADIO_REG_TINY(pi, CLK_DIV_CFG1, core, dac_driver_size, 8);
		porig->adc_cfg10[core] = READ_RADIO_REG_TINY(pi, ADC_CFG10, core);
		porig->adc_ovr1[core] = READ_RADIO_REG_TINY(pi, ADC_OVR1, core);
		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_od_pu, 1);
			MOD_RADIO_REG_TINY(pi, ADC_CFG18, core, adc_od_pu,
				((CHSPEC_IS20(pi->radio_chanspec)) ||
				(CHSPEC_IS40(pi->radio_chanspec))) ? 0 : 1);
		} else {
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_od_pu, 0);
		}
		MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_in_test, 1);
		MOD_RADIO_REG_TINY(pi, ADC_CFG10, core, adc_in_test, 0x0);
		/* Setting TIA gain */
		if (RADIOID(pi->pubpi.radioid) == BCM20693_ID)  {
			if (HW_RADIOREV(pi->pubpi.radiorev) == 7)  {
			wlc_phy_tia_gain_tiny(pi, (CHSPEC_IS2G(pi->radio_chanspec))
				? 0x2 : 0x0, core);
			} else if (HW_RADIOREV(pi->pubpi.radiorev) == 13) {
			wlc_phy_tia_gain_tiny(pi, (CHSPEC_IS2G(pi->radio_chanspec))
				? 0x3 : 0x0, core);
			} else {
				wlc_phy_tia_gain_tiny(pi, (CHSPEC_IS2G(pi->radio_chanspec))
				?((phy_get_phymode(pi) == PHYMODE_RSDB) ? 0x5 : 0x6)
						: 0x0, core);
			}
		} else
			wlc_phy_tia_gain_tiny(pi, (CHSPEC_IS2G(pi->radio_chanspec))
				? 0x3 : 0x0, core);

	}
}

static void
wlc_phy_papd_radio_cleanup_acphy_tiny(phy_info_t *pi)
{
	uint8 core;

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_txcal_radioregs_t *porig = (pi_ac->ac_txcal_radioregs_orig);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	FOREACH_CORE(pi, core) {
		/* # Enable the loopback path */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* # Powerup the 2G iPA attenuation on Tx side (loops back to Rx) */
			MOD_RADIO_REG_TINY(pi, TX2G_MISC_CFG1, core, cal2g_pa_pu, 0x0);
			/* # Powerup the papd loopback path on 2G Rx side */
			MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core, loopback2g_papdcal_pu, 0x0);

			MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
				ovr_lna2g_tr_rx_en, 1);
			MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core,
				lna2g_tr_rx_en, 1);
			MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
				ovr_gm2g_pwrup, 0);
			if (RADIOID(pi->pubpi.radioid) != BCM20693_ID) {
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_NORTH, core,
					ovr_rf2g_mix1st_en, 0);
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
				                    ovr_gm2g_auxgm_pwrup, 1);
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_NORTH, core,
					ovr_lna2g_lna1_bypass, 0);
			}
			MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 0);
			MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
			if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
				MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_lna2g_lna2_gain, 0);
				MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_lna1_Rout, 0);
				if (HW_RADIOREV(pi->pubpi.radiorev) == 13) {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_lna2g_lna1_out_short_pu, 0);
				}
			}
		} else {
			/* # Powerdown the 5G iPA attenuation on Tx side (loops back to Rx) */
			MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core, cal5g_pa_pu, 0x0);
			/* # Powerdown the master cal pu signal on 5G Rx side (common to papd &
			 * rxiqcal). Not needed for rc/cr rxiqcal pu.
			 */
			MOD_RADIO_REG_TINY(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 0x0);
			MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core, ovr_gm5g_pwrup, 0);
			if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
				MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_lna2, 0);
			} else {
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
				                    ovr_lna5g_pu_auxlna2, 1);
			}
			MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_auxlna2, 0);
			MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core, ovr_lna5g_lna1_bypass, 0);
			MOD_RADIO_REG_TINY(pi, PA5G_CFG1, core, rf5g_epapd_en, 0);
			if ((IS_4349A2_RADIO(pi)) == 1) {
				MOD_RADIO_REG_20693(pi, SPARE_CFG16, core,
					loopback5g_gm5g_pu, 0);
			}
			if (RADIOID(pi->pubpi.radioid) == BCM20693_ID) {
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core, ovr_lna5g_lna1_pu, 0);
				MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR2, core,
				        ovr_lna5g_lna2_gain, 0);
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
				        ovr_mix5g_en, 1);
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_tr_rx_en, 0);
				MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core, ovr_pa5g_pu, 0);
				MOD_RADIO_REG_TINY(pi, PA5G_CFG4, core, pa5g_pu, 0);
			}
		}
		MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_od_pu, 0);
	}
	MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0);
	MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);

	FOREACH_CORE(pi, core) {
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_CFG10, core),
		                         porig->adc_cfg10[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_OVR1, core),
		                         porig->adc_ovr1[core]);
	}
}

static void
wlc_phy_tia_gain_tiny(phy_info_t *pi, uint16 gain, uint8 core)
{
	/* clear bits 0-5 of the RX_BB_2G_OVR_EAST radio register */
	phy_utils_write_radioreg(pi, RADIO_REG(pi, RX_BB_2G_OVR_EAST, core),
		(READ_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core) & 0xffc0));
	MOD_PHYREGCE(pi, RfctrlOverrideGains, core, rxgain, 1);
	MOD_PHYREGCE(pi, RfctrlCoreRXGAIN1, core, rxrf_tia_gain, gain);
}

void
wlc_phy_txpwr_papd_cal_run_acphy(phy_info_t *pi,
	uint8 tx_pre_cal_pwr_ctrl_state)
{
	phy_info_acphy_t *pi_acphy = NULL;
	bool suspend = TRUE, suspend_flag = FALSE;
	uint8 tx_pwr_ctrl_state;
	uint8 core;
	int16  tx_atten = 0, rx_atten = 0, i;
	uint32 eps_init_val = 0x0;
	int16 pgagain_offset = 0;
	int16 epsilonoffset = 0;
	uint8 scalar_table_ids[] = { ACPHY_TBL_ID_SCALAR0, ACPHY_TBL_ID_SCALAR1,
		ACPHY_TBL_ID_SCALAR2, ACPHY_TBL_ID_SCALAR3};
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};
#ifdef PAPD_GAIN_CTRL
	acphy_txgains_t tx_gains;
	uint8 PAgain = 0xff;
	uint16 bbmult = 0x3f;
	uint8 Gainoverwrite = 0;
#endif /* PAPD_GAIN_CTRL */
#ifdef BCMDBG
	int16 epsregval;
#endif /* BCMDBG */
	int16 pgag = 0, eps_offset = 0;
	uint16 numIterLMS_papd = 0x10, startindex = 5, yrefindex = 5, stopindex = 63;
	acphy_txgains_t txgains[4];
	uint8 saved_pwr_idx[4] = {0, 0, 0, 0};
	pi_acphy = pi->u.pi_acphy;

	if (pi_acphy->acphy_papd_skip == 1)
		return;
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		numIterLMS_papd = 0x10;
#ifdef PAPD_GAIN_CTRL
		bbmult = 0x30;
#endif /* PAPD_GAIN_CTRL */
	}
#ifdef PAPD_GAIN_CTRL
	if (ACMAJORREV_2(pi->pubpi.phy_rev)) {
		numIterLMS_papd = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0x20 : 0x40;
	}
	tx_gains.txlpf = 0x0;
	tx_gains.txgm = 0xff;
	tx_gains.pga = 0xff;
	tx_gains.pad = 0xff;
	if (PHY_IPA(pi)) {
		Gainoverwrite = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			pi->u.pi_acphy->srom_pagc2g_ovr :
			pi->u.pi_acphy->srom_pagc5g_ovr;
		PAgain = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			pi->u.pi_acphy->srom_pagc2g :
			pi->u.pi_acphy->srom_pagc5g;
	}
	tx_gains.ipa = (Gainoverwrite == 0) ? 0xff : PAgain;
	if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
		tx_gains.txlpf = 0x0;
		tx_gains.txgm = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0x67 : 0x91;
		tx_gains.pga = 0xff;
		tx_gains.pad = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xff : 0x7f;
		tx_gains.ipa = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xc0 : 0x3;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			startindex = 0;
			yrefindex = 0;
		}
	}
#endif /* PAPD_GAIN_CTRL */

	/* skip cal if phy is muted */
	if (PHY_MUTED(pi) || ISSIM_ENAB(pi->sh->sih))
		return;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		printf("MAC was not suspended before calling wlc_phy_txpwr_papd_cal_run_acphy!\n");
		suspend_flag = TRUE;
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	/* Disable BT as it affects PAPD CAL */
	wlc_btcx_override_enable(pi);
	/* Disable CRS */
	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	/* Turn off the txpowercontrol and save the txindex */
	tx_pwr_ctrl_state = pi->acphy_txpwrctrl;
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);

	/* Make it work for both 4335 and 4360 */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		saved_pwr_idx[core] = wlc_phy_txpwr_idx_cur_get_acphy(pi, core);
		/* initialize scaler table */
		wlc_phy_table_write_acphy(pi, scalar_table_ids[core], 64, 0, 32,
			acphy_papd_scaltbl);
	/* Fill up epsilon table with eps_init_val = 0 */
		for (i = 0; i < 64; i++) {
			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core], 1, i, 32,
				&eps_init_val, core);
		}
	}

	/* acphy_papd_cal_phy_setup */
	wlc_phy_papd_phy_setup_acphy(pi);

	/* 2069_papd_cal_setup */
	wlc_phy_papd_radio_loopback_setup_acphy(pi, tx_atten, rx_atten);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {

		/*
		* Not supported on 4-core PAPD devices
		* None exist as of this writing
		*/
		ASSERT(core < 3);

		pi->u.pi_acphy->txpwrindex[core] = 16;

		MOD_PHYREGCEE(pi, PapdCalShifts, core, papdYrefOverride, 0x0);
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref, 0x1);
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit, 0x0);
		MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core); /* Specify the core */
#ifdef PAPD_GAIN_CTRL
		GAIN_CTRL = 1;
		/* This is needed to put analytic cal, if enabled, in gain control mode */
		papd_gainctrl_pga[core] = wlc_phy_papd_cal_gain_cntl_acphy(pi, numIterLMS_papd,
			core, startindex, yrefindex, stopindex);
		GAIN_CTRL = 0; /* Set analytic cal, if enabled, to normal mode */
	        tx_gains.pga = papd_gainctrl_pga[core];
	        wlc_phy_write_tx_gain_acphy(pi, core, &tx_gains, &bbmult);
#endif  /* PAPD_GAIN_CTRL */

		wlc_phy_papd_cal_acphy(pi, numIterLMS_papd, core, startindex, yrefindex, stopindex);

		for (i = 0; i < startindex; i++) {
			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core], 1, i, 32,
				&eps_init_val, core);
		}

		wlc_phy_get_tx_gain_acphy(pi, core, &(txgains[core]));
		papd_gainctrl_pga[core] = txgains[core].pga;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pgag = txgains[core].pga;
			pgagain_offset = *(rfpwrlut_ptr + pgag);
			eps_offset = -4;

		} else {
			pgag = txgains[core].pga;
			pgagain_offset = *(rfpwrlut_ptr + pgag);
			eps_offset = 0;
		}

		/* papd_index_shift in tcl-- needs to be taken care of */
		epsilonoffset = -66 + pgagain_offset + eps_offset;

		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonScalar, 8);
		if (tx_pre_cal_pwr_ctrl_state == PHY_TPC_HW_OFF) {
			MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset,
				epsilonoffset);
		} else {
			MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset, 0);
		}
#ifdef BCMDBG
		epsregval = READ_PHYREGFLDCEE(pi, EpsilonTableAdjust, core, epsilonOffset);
		PHY_PAPD((" ------------------ \n"));
		PHY_PAPD(("read epsilonTableAdjust In PAPD is %d\n", epsregval));
		PHY_PAPD((" ------------------ \n"));
#endif /* BCMDBG */

		/* save this value, in case some other pro re-init phyregs */
		pi_acphy->acphy_papd_epsilon_offset[core] = epsilonoffset;
	}

	/* acradio papd_cal_cleanup */
	wlc_phy_papdcal_radio_cleanup_acphy(pi);

	/* acphy_papd_cal_phy_cleanup */
	wlc_phy_papd_phy_cleanup_acphy(pi);

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		/* acphy_rfctrl_override txgain off all */
		WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN1, core, 0);
		WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN2, core, 0);
		WRITE_PHYREGCE(pi, Dac_gain, core, 0);
		MOD_PHYREGCE(pi, RfctrlCoreLpfGain, core, lpf_bq2_gain, 0);

		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, txgain, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 0);

		/* acphy_papd_stats-- Some debug related thing. Not done here */
		/* acphy_papd on {0} or {0 1 2} */
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 0);
	}

	/* restore tx pwr index to original power index */
	/* wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF); */
	wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			pi_acphy->acphy_txpwr_idx_5G[core] = saved_pwr_idx[core];
		} else {
			pi_acphy->acphy_txpwr_idx_2G[core] = saved_pwr_idx[core];
		}
	}
	wlc_phy_papd_set_rfpwrlut(pi);
	/* Disabling BTCX Override */
	wlc_phy_btcx_override_disable(pi);
	/* Enable CRS */
	wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
	/* If the mac was suspended from this function, enable it */
	if (suspend_flag)
		wlapi_enable_mac(pi->sh->physhim);
}

/* Dump the PAPD LUT (eps table) to PHY_CAL trace */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
void
wlc_phy_papd_dump_eps_trace_acphy(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint8 core, j;
	uint32 eps_table[ACPHY_PAPD_EPS_TBL_SIZE];
	int32 eps_re, eps_im;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};

	FOREACH_CORE(pi, core) {
		wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core],
			ACPHY_PAPD_EPS_TBL_SIZE, 0, 32, eps_table, core);

		PHY_CAL(("core %d\n", core));
		bcm_bprintf(b, "  PAPD Epsilon Table  Real Image CORE %d \n", core);
		for (j = 0; j < ACPHY_PAPD_EPS_TBL_SIZE; j++) {
			wlc_phy_papd_decode_epsilon(eps_table[j], &eps_re, &eps_im);
			PHY_CAL(("{%d %d} ", eps_re, eps_im));
			bcm_bprintf(b, "{%d %d}\n ", eps_re, eps_im);
		}
		PHY_CAL(("\n"));
	}
	PHY_CAL(("\n"));
}
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

static void
wlc_phy_papd_smooth_acphy(phy_info_t *pi, uint8 core, uint32 winsz, uint32 start, uint32 end)
{
	uint32 *buf, *src, *dst, sz;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};

	PHY_CAL(("Smoothing papd cal on core: %d\n", core));

	sz = end - start + 1;
	ASSERT(end > start);
	ASSERT(end < ACPHY_PAPD_EPS_TBL_SIZE);

	/* Allocate storage for both source & destination tables */
	if ((buf = MALLOC(pi->sh->osh, 2 * sizeof(uint32) * ACPHY_PAPD_EPS_TBL_SIZE)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Setup source & destination pointers */
	src = buf;
	dst = buf + ACPHY_PAPD_EPS_TBL_SIZE;

	/* Read original table */
	wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core], ACPHY_PAPD_EPS_TBL_SIZE,
		0, 32, src, core);

	/* Average coeffs across window */
	do {
		uint32 win_start, win_end;
		int32 nAvr, eps_r, eps_i, eps_real, eps_imag;

		win_start = end - MIN(end, (winsz >> 1));
		win_end = MIN(ACPHY_PAPD_EPS_TBL_SIZE - 1, end + (winsz >> 1));
		nAvr = win_end - win_start + 1;
		eps_real = 0;
		eps_imag = 0;

		do {
			wlc_phy_papd_decode_epsilon(src[win_end], &eps_r, &eps_i);
			eps_real += eps_r;
			eps_imag += eps_i;
		} while (win_end-- != win_start);

		eps_real /= nAvr;
		eps_imag /= nAvr;
		dst[end] = ((uint32)eps_imag << 13) | ((uint32)eps_real & 0x1fff);
	} while (end-- != start);

	/* Write updated table */
	wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core], sz, start, 32, dst, core);

	/* Free allocated buffer */
	MFREE(pi->sh->osh, buf, 2 * sizeof(uint32) * ACPHY_PAPD_EPS_TBL_SIZE);
}

static void
wlc_phy_papd_rx_gain_ctrl_acphy(phy_info_t *pi)
{

	int8 gain_ctrl_done = 0;
	int16 clip_det_sel, wlc_clip_cnt_regval;
	uint16 bbmult_orig[PHY_CORE_MAX];
	uint8 core;
	int16 SAVE_rx_bb_2g_ovr1, SAVE_tia_cfg12_rssi_pwrup, SAVE_tia_cfg12_wrssi3_ref_high_sel;
	int16 SAVE_tia_cfg12_wrssi3_ref_mid_sel, SAVE_tia_cfg13_wrssi3_ref_low_sel;
	uint8 rx_attn = 0, tx_attn = 0, max_attn = 0;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	ASSERT(ACMAJORREV_3(pi->pubpi.phy_rev));
	ASSERT(RADIOID(pi->pubpi.radioid) == BCM20691_ID);

	if (PHY_EPAPD(pi)) {
		max_attn = 15;
	} else {
		max_attn = 3;
	}

	clip_det_sel = READ_PHYREGFLD(pi, RxSdFeConfig6, rx_farrow_rshift_0);
	/* tcl has a loop for this */
	/* return from Deaf */
	if (pi_ac->deaf_count > 0) {
		wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
	}

	/* start tone with full scale at DAC */
	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		wlc_phy_get_tx_bbmult_acphy(pi, &(bbmult_orig[core]), core);
		wlc_phy_tx_tone_acphy(pi, 2000, 450, 0, 0, FALSE);
		wlc_phy_set_tx_bbmult_acphy(pi, &bbmult_orig[core], core);
	}

	/* save register states */
	if (ACREV_IS(pi->pubpi.phy_rev, 4)) {
		SAVE_rx_bb_2g_ovr1 = READ_RADIO_REGFLD_20691(pi, RX_BB_2G_OVR1, 0,
		                                             ovr_tia_offset_rssi_pwrup);
	} else {
		SAVE_rx_bb_2g_ovr1 = READ_RADIO_REGFLD_20691(pi, RX_BB_2G_OVR_EAST, 0,
		                                             ovr_tia_offset_rssi_pwrup);
	}
	SAVE_tia_cfg12_rssi_pwrup = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0, rssi_pwrup);
	SAVE_tia_cfg12_wrssi3_ref_high_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0,
	                                                             wrssi3_ref_high_sel);
	SAVE_tia_cfg12_wrssi3_ref_mid_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0,
	                                                            wrssi3_ref_mid_sel);
	SAVE_tia_cfg13_wrssi3_ref_low_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG13, 0,
	                                                            wrssi3_ref_low_sel);

	/* Enable rssi clip detectors */
	if (ACREV_IS(pi->pubpi.phy_rev, 4)) {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR1, 0, ovr_tia_offset_rssi_pwrup, 1);
	} else {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR_EAST, 0, ovr_tia_offset_rssi_pwrup, 1);
	}
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, rssi_pwrup, 1);

	if (clip_det_sel == 3) {
		MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_high_sel, 4);
	} else {
		MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_high_sel, 0);
	}
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_mid_sel, 0);
	MOD_RADIO_REG_20691(pi, TIA_CFG13, 0, wrssi3_ref_low_sel, 0);

	for (rx_attn = 0; rx_attn < max_attn; rx_attn++) {
		while (tx_attn <= max_attn) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_RADIO_REG_20691(pi, TX2G_MISC_CFG1, 0, cal2g_pa_atten, tx_attn);
				MOD_RADIO_REG_20691(pi, RXRF2G_CFG2, 0,
				                    rf2g_papdcal_rx_attn, rx_attn);
			} else {
				MOD_RADIO_REG_20691(pi, TX5G_MISC_CFG1, 0, cal5g_pa_atten, tx_attn);
				MOD_RADIO_REG_20691(pi, RXRF5G_CFG2, 0,
				                    loopback5g_papdcel_rx_attn, rx_attn);
			}

			if (clip_det_sel == 3) {
				clip_det_sel = clip_det_sel - 1;
			}

			if (clip_det_sel+1 == 1) {
				wlc_clip_cnt_regval = READ_PHYREG(pi, W3ClipCnt1);
			} else if (clip_det_sel+1 == 2) {
				wlc_clip_cnt_regval = READ_PHYREG(pi, W3ClipCnt2);
			} else {
				wlc_clip_cnt_regval = READ_PHYREG(pi, W3ClipCnt3);
			}

			if (wlc_clip_cnt_regval == 0) {
				gain_ctrl_done = 1;
				break;
			}

			if (tx_attn == max_attn) {
				break;
			}
			tx_attn = tx_attn + 1;
		}
		if ((gain_ctrl_done == 1) || (rx_attn == 3)) {
			break;
		}
	}

	/* Restor Registers */
	if (ACREV_IS(pi->pubpi.phy_rev, 4)) {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR1, 0,
		                    ovr_tia_offset_rssi_pwrup, SAVE_rx_bb_2g_ovr1);
	} else {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR_EAST, 0,
		                    ovr_tia_offset_rssi_pwrup, SAVE_rx_bb_2g_ovr1);
	}

	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, rssi_pwrup, SAVE_tia_cfg12_rssi_pwrup);
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0,
	                    wrssi3_ref_high_sel, SAVE_tia_cfg12_wrssi3_ref_high_sel);
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0,
	                    wrssi3_ref_mid_sel, SAVE_tia_cfg12_wrssi3_ref_mid_sel);
	MOD_RADIO_REG_20691(pi, TIA_CFG13, 0,
	                    wrssi3_ref_low_sel, SAVE_tia_cfg13_wrssi3_ref_low_sel);

	/* Switch off test tone */
	wlc_phy_stopplayback_acphy(pi);

	/* beDeaf */
	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);
}

/* ***************************** */
/*		External Definitions			*/
/* ***************************** */

void
wlc_phy_papd_set_rfpwrlut(phy_info_t *pi)
{
	int16 epsilonoffset, pga_gain;
#ifdef BCMDBG
	int16 epsregval;
#endif /* BCMDBG */
	uint8 idx, core = 0;
	txgain_setting_t txgain_settings;
	uint16 channel = 5180;
	int8 epscaldelta = 0, eps_offset = 0, delta = 0;
	uint8 papdmode = pi->u.pi_acphy->papdmode;
	uint32 rfpwrlut_table_ids[] = { ACPHY_TBL_ID_RFPWRLUTS0,
		ACPHY_TBL_ID_RFPWRLUTS1, ACPHY_TBL_ID_RFPWRLUTS2};
	PHY_PAPD((" ------- In RFPWRLUT ------- \n"));

	FOREACH_ACTV_CORE(pi, pi->sh->phyrxchain, core) {
		if (ACMAJORREV_2(pi->pubpi.phy_rev)) {
			/* 4354A0 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				rfpwrlut_ptr = pga_gain_array_2g_4354;
				eps_offset = 19;
			} else {
				rfpwrlut_ptr = pga_gain_array_5g_4354;
				if (core == 0)
					eps_offset = 2;
				else if (core == 1)
					eps_offset = 3;
			}
			if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
				/* 4350EPAPD */
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					rfpwrlut_ptr = pga_gain_array_2g_epapd[core];
					if (core == 0)
						eps_offset = -3;
					else
						eps_offset = -6;
				} else {
				    channel = CHSPEC_CHANNEL(pi->radio_chanspec);
				    channel = 5000 + 5 * channel;

				    if (channel >= 5180 && channel <= 5320) {
					    rfpwrlut_ptr = pga_gain_array_5g_epapd_0;
				    } else if (channel >= 5500 && channel <= 5620) {
					    rfpwrlut_ptr = pga_gain_array_5g_epapd_1;
				    } else if (channel >= 5630 && channel <= 5700) {
					    rfpwrlut_ptr = pga_gain_array_5g_epapd_2;
				    } else if (channel >= 5710 && channel <= 5825) {
					    rfpwrlut_ptr = pga_gain_array_5g_epapd_3;
				    }
				    eps_offset = 6;
				}
			}
		} else if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
			/* 4335-4339 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				rfpwrlut_ptr  =  pga_gain_array_2g;
				if (pi->sh->chippkg == BCM4335_FCBGA_PKG_ID) {
					delta = 1;
					eps_offset = -3;
				} else {
					if ((papdmode == PAPD_ANALYTIC) ||
						(papdmode == PAPD_ANALYTIC_WO_YREF)) {
						eps_offset = -3;
					} else {
						eps_offset = 0;
					}
				}
			} else {
				channel = CHSPEC_CHANNEL(pi->radio_chanspec);
				channel = 5000 + 5 * channel;

				if (channel >= 5180 && channel <= 5320) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_0;
				} else if (channel >= 5500 && channel <= 5620) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_1;
				} else if (channel >= 5630 && channel <= 5700) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_2;
				} else if (channel >= 5710 && channel <= 5825) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_3;
				}

				if (pi->sh->chippkg == BCM4335_FCBGA_PKG_ID) {
					if (CHSPEC_IS80(pi->radio_chanspec)) {
						eps_offset = 0;
						if (channel >= 5710 && channel <= 5825)
							eps_offset = -1;
					} else {
						eps_offset = -1;
					}
				} else {
					if ((papdmode == PAPD_ANALYTIC) ||
						(papdmode == PAPD_ANALYTIC_WO_YREF)) {
						eps_offset = -5;
						if (channel >= 5710 && channel <= 5825)
							eps_offset = -6;
					} else {
						if (CHSPEC_IS80(pi->radio_chanspec)) {
							eps_offset = -1;
							if (channel == 5775)
								eps_offset = -2;
						} else {
							eps_offset = -2;
						}
					}
				}
			}
		}
		for (idx = 0; idx < 128; idx++)
		{
			wlc_phy_get_txgain_settings_by_index_acphy(pi, &txgain_settings, idx);
			pga_gain =  (txgain_settings.rad_gain_mi >> 8) & 0xff;
			if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 133);
				} else {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 207);
				}
			} else {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 167);
				} else {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 71);
				}
			}
			epsilonoffset = (-66 + (int8)*(rfpwrlut_ptr + pga_gain)
				- epscaldelta + eps_offset + delta) << 1;

			wlc_phy_table_write_acphy(pi, rfpwrlut_table_ids[core], 1, idx,
				16, &epsilonoffset);
		}
		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset, 0);
#ifdef BCMDBG
		epsregval = READ_PHYREGFLDCEE(pi, EpsilonTableAdjust, core, epsilonOffset);
		PHY_PAPD((" ------------------ \n"));
		PHY_PAPD(("read epsilonTableAdjust In RF PowerLUT is %d\n", epsregval));
		PHY_PAPD((" ------------------ \n"));
#endif /* BCMDBG */
	}
}

void
wlc_phy_papd_phy_cleanup_acphy(phy_info_t *pi)
{

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint8 core;
	uint8 stall_val;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(porig->is_orig);
	porig->is_orig = FALSE;

	WRITE_PHYREG(pi, RfseqCoreActv2059, porig->RfseqCoreActv2059);

	WRITE_PHYREG(pi, RfctrlCoreGlobalPus, porig->RfctrlCoreGlobalPus);
	WRITE_PHYREG(pi, RfctrlOverrideGlobalPus, porig->RfctrlOverrideGlobalPus);

	FOREACH_CORE(pi, core) {

		WRITE_PHYREGCE(pi, RfctrlIntc, core, porig->RfctrlIntc[core]);
		WRITE_PHYREGCE(pi, PapdEnable, core, porig->PapdEnable[core]);

		/* Are the following three statements redundant?
			wlc_phy_txpwr_by_index_acphy would overwrite on the following anyway
		*/
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&porig->rfseq_txgain[core + 0]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&porig->rfseq_txgain[core + 3]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&porig->rfseq_txgain[core + 6]);
		wlc_phy_txpwr_by_index_acphy(pi, (1 << core), porig->txpwridx[core]);
		wlc_phy_set_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);

		WRITE_PHYREGCE(pi, RfctrlOverrideTxPus, core, porig->RfctrlOverrideTxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideRxPus, core, porig->RfctrlOverrideRxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideGains, core, porig->RfctrlOverrideGains[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideLpfCT, core, porig->RfctrlOverrideLpfCT[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core,
			porig->RfctrlOverrideLpfSwtch[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, porig->RfctrlOverrideAfeCfg[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, porig->RfctrlOverrideAuxTssi[core]);
		WRITE_PHYREGCE(pi, RfctrlOverrideLowPwrCfg, core,
			porig->RfctrlOverrideLowPwrCfg[core]);

		WRITE_PHYREGCE(pi, RfctrlCoreTxPus, core, porig->RfctrlCoreTxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreRxPus, core, porig->RfctrlCoreRxPus[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN1, core, porig->RfctrlCoreTXGAIN1[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN2, core, porig->RfctrlCoreTXGAIN2[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN1, core, porig->RfctrlCoreRXGAIN1[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreRXGAIN2, core, porig->RfctrlCoreRXGAIN2[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfGain, core, porig->RfctrlCoreLpfGain[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfCT, core, porig->RfctrlCoreLpfCT[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfGmult, core, porig->RfctrlCoreLpfGmult[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreRCDACBuf, core, porig->RfctrlCoreRCDACBuf[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, porig->RfctrlCoreLpfSwtch[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, porig->RfctrlCoreAfeCfg1[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, porig->RfctrlCoreAfeCfg2[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreLowPwr, core, porig->RfctrlCoreLowPwr[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, porig->RfctrlCoreAuxTssi1[core]);
		WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi2, core, porig->RfctrlCoreAuxTssi2[core]);
		WRITE_PHYREGCE(pi, Dac_gain, core, porig->Dac_gain[core]);
	    WRITE_PHYREGCE(pi, forceFront, core, porig->forceFront[core]);
	}
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RESET2RX);
	if (!ACMAJORREV_4(pi->pubpi.phy_rev)) {
		WRITE_PHYREG(pi, lbFarrowCtrl, porig->lbFarrowCtrl);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_phy_papd_cal_acphy(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
	uint16 yrefindex, uint16 stopindex)
{
	bool calmode;
	uint16 m[4] = {0, 0, 0, 0};
	int8 bbmult, coremask, k, tx_idx = 40;
	int16 dac_rf_offset;
	uint32 scalartblval, papdmult, epsilonscalartemp;
	int16 epsilonoffsettemp;
	int16 cal_tone_mag = 186;
	int16 temp, temp1, qQ1, lut_shift;
	uint8 papdmode = pi->u.pi_acphy->papdmode;
	uint8 scalar_table_ids[] = { ACPHY_TBL_ID_SCALAR0, ACPHY_TBL_ID_SCALAR1,
		ACPHY_TBL_ID_SCALAR2};
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};

	PHY_PAPD(("\n\nPAPD Main Cal .. "));

	if (TINY_RADIO(pi) && (core < 3)) {

		if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
			calmode = 0;
			bbmult = (CHSPEC_IS2G(pi->radio_chanspec)) ? 64 : 70;
			tx_idx = (CHSPEC_IS2G(pi->radio_chanspec)) ?
				((core == 0) ? 60 : 68) : ((core == 0) ? 58 : 62);

			if ((phy_get_phymode(pi) == PHYMODE_MIMO)) {
				uint8 off_core = 0;
				if (core == 0) {
					off_core = 1;
				} else {
					off_core = 0;
				}
				MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, off_core, ovr_pa5g_pu, 1);
				MOD_RADIO_REG_TINY(pi, PA5G_CFG4, off_core, pa5g_pu, 0);
				MOD_PHYREGCE(pi, RfctrlOverrideTxPus, off_core, txrf_pwrup, 0x1);
				MOD_PHYREGCE(pi, RfctrlCoreTxPus, off_core, txrf_pwrup, 0x0);
				MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core, ovr_pa5g_pu, 1);
				MOD_RADIO_REG_TINY(pi, PA5G_CFG4, core, pa5g_pu, 1);
				MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, txrf_pwrup, 0x1);
				MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, txrf_pwrup, 0x1);
			}

		} else {
			calmode = 1; /* Run single index of PAPD table only */
			bbmult = (CHSPEC_IS2G(pi->radio_chanspec)) ? 64 : 60;
			if (PHY_EPAPD(pi))
				bbmult = (CHSPEC_IS2G(pi->radio_chanspec)) ? 100 : 75;
			if (CHSPEC_IS2G(pi->radio_chanspec) && (pi->papdbbmult2g != -1)) {
				bbmult = pi->papdbbmult2g;
			} else if (CHSPEC_IS5G(pi->radio_chanspec) && (pi->papdbbmult5g != -1)) {
				bbmult = pi->papdbbmult5g;
			}
			if (pi->pacalmode != -1) {
				calmode = pi->pacalmode;
			}
		}
	} else {
		calmode = 0;  /* Run the PAPD automatic machine on all indices */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			bbmult = 0x3f;
			if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
				bbmult = (core == 0) ? 100 : 110;
			}
		} else
			bbmult = 0x30;
	}
	switch (core) {
		case 0:
			m[0] = bbmult;
			break;
		case 1:
			m[1] = bbmult;
			break;
		case 2:
			m[2] = bbmult;
			break;
		case 3:
			m[3] = bbmult;
			break;
	}
	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		/* Logic added to 'turn' off tone on inactive core */
		/* Need to turn off PA on the inactive core as the next step */
		if ((!DO_PAPD_GAINCTRL) || (CHSPEC_IS2G(pi->radio_chanspec) &&
			(HW_RADIOREV(pi->pubpi.radiorev) == 5)))
			wlc_phy_txpwr_by_index_acphy(pi, core + 1, tx_idx);
	}
	if ((papdmode == PAPD_ANALYTIC) || (papdmode == PAPD_ANALYTIC_WO_YREF)) {
	        bool   corr_sat = 0;
		uint8  corr_shift = 0x4;
		uint16 index;
		int32  mag_corr_papd_index;
		uint32 dst[64];
		uint32 dst_tmp = 0;
		uint32 dst_limit = 0;
		uint16 next_index_interp = startindex;
		uint16 next_index_write = startindex;
		uint16 next_index_interp_init = startindex;
		int16  abs_c, eps_re_curr = 0, eps_im_curr = 0,
		       eps_re_prev = 0, eps_im_prev = 0,
		       eps_re_interp = 0, eps_im_interp = 0;
		int16  corr_papd_index_I = 0, corr_papd_index_Q = 0,
		       corr_papd_index_I_yref = 0, corr_papd_index_Q_yref = 0;
		uint16 vin_prev = 0, vin_curr = 0, vin_p[64];
		/* When startindex == yrefindex, the hw cal for the startindex takes long time */
		if (startindex == yrefindex)
		        startindex ++;
		/* dst_limit to reduce variation on AMAM/AMPM for last few entries */
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
			        dst_limit = 54358000; /* 1.8 */
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				dst_limit = 54358000; /* 1.8 */
			} else {
				dst_limit = 42928704; /* 1.6 */
			}
		} else {
			dst_limit = 54358000; /* 1.8 */
		}
		for (index = 0; index <= 63; index++) {
			vin_p[index] = (uint16)(acphy_papd_scaltbl[index] & 0xffff);
		}
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 1);
		MOD_PHYREG(pi, PapdEpsilonUpdateIterations, epsilonUpdateIterations, 1);
		MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
		MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x140);
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, corr_shift);
		MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x3ff);
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref, 0x1);
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit, 0x0);
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdYrefAddr, yrefindex);
		MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, 0x1);
		coremask = 7;
		wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3], coremask);
		/* Looping over yrefindex + (startindex -> stopindex); */
		wlc_phy_tx_tone_acphy_papd(pi, 2000, 186, 0, 0, FALSE);
		for (index = startindex; index <= stopindex+1; corr_sat? index : index++) {
			if (corr_sat) {
				MOD_PHYREGCEE(pi, PapdCalShifts, core,
				        papdCorrShift, ++corr_shift);
			}

			if ((index == yrefindex) || ((index >= startindex) &&
			        (index <= stopindex))) {
				MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, index);
				MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, index);
				MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref,
				(papdmode == PAPD_ANALYTIC_WO_YREF) ?
				((index == startindex) ? 0x1 : 0x0) : 0x1);
				/*
				printf("PAPD_ANALYTIC_WO_YREF= %d,YREF_INIT=%d \n",
				papdmode, (papdmode == PAPD_ANALYTIC_WO_YREF)
				? ((index == startindex) ? 0x1 : 0x0) : 0x1);
				*/
				WRITE_PHYREG(pi, papdCalCorrDebugAddr, index);
				MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
				MOD_PHYREG(pi, PapdCalStart, papdStart, 1);
			}

			if (((yrefindex == startindex) && (index > startindex+1)) ||
			    ((yrefindex != startindex) && (index > startindex))) {
				/* 1+eps = c = Yref/Y = Yref*conj(Y)/abs(Y)^2 */
				mag_corr_papd_index =
					(int32)corr_papd_index_I*
					(int32)corr_papd_index_I +
					(int32)corr_papd_index_Q*
					(int32)corr_papd_index_Q;
				eps_re_prev = eps_re_curr;
				eps_im_prev = eps_im_curr;
				if (mag_corr_papd_index >> 12) {
					eps_re_curr = (((int32)corr_papd_index_I*
						(int32)corr_papd_index_I_yref +
						(int32)corr_papd_index_Q*
						(int32)corr_papd_index_Q_yref) /
						(mag_corr_papd_index >> 12)) - 4096;
					eps_im_curr = ((int32)corr_papd_index_Q*
						(int32)corr_papd_index_I_yref -
						(int32)corr_papd_index_I*
						(int32)corr_papd_index_Q_yref) /
						(mag_corr_papd_index >> 12);
				} else {
					eps_re_curr = 0;
					eps_im_curr = 0;
				}
				abs_c = (uint16)phy_utils_sqrt_int(
					        ((int32)4096 + (int32)eps_re_curr)*
					        ((int32)4096 + (int32)eps_re_curr) +
					        ((int32)eps_im_curr*(int32)eps_im_curr));
				vin_prev = vin_curr;
				if (abs_c) {
					vin_curr = (uint16)(((uint32)vin_p[index-1]<<12)/abs_c);
				} else {
					vin_curr = vin_p[index-1];
				}
				eps_re_curr = (eps_re_curr > 4095) ? 4095 :
					((eps_re_curr < -4096) ? -4096: eps_re_curr);
				eps_im_curr = (eps_im_curr > 4095) ? 4095 :
					((eps_im_curr < -4096) ? -4096: eps_im_curr);
			}
			if (GAIN_CTRL == 0) {
				/* computing the first index to interpolate for based on
				 * the first vin computed
				 */
				if (((yrefindex != startindex) && (index == startindex+1)) ||
				    ((yrefindex == startindex) && (index == startindex+2))) {
					while ((next_index_interp_init <= 63) &&
					       (vin_curr > vin_p[next_index_interp_init])) {
						dst[next_index_interp_init++] = 0;
					}
					next_index_interp = next_index_interp_init;
					next_index_write = next_index_interp_init;
				}
				while ((next_index_interp <= 63) &&
				       (index > startindex+1) &&
				       (vin_curr >  vin_p[next_index_interp]) &&
				       (vin_prev <= vin_p[next_index_interp]) &&
				       (vin_curr != vin_prev)) {
					eps_re_interp =
						(int16)(((int32)eps_re_curr*(int32)
							 (vin_p[next_index_interp] - vin_prev) +
							 (int32)eps_re_prev*(int32)
							 (vin_curr - vin_p[next_index_interp]))/
							 (int16)(vin_curr - vin_prev));
					eps_im_interp =
						(int16)(((int32)eps_im_curr*(int32)
							 (vin_p[next_index_interp] - vin_prev) +
							 (int32)eps_im_prev*(int32)
							 (vin_curr - vin_p[next_index_interp]))/
							 (int16)(vin_curr - vin_prev));
					dst[next_index_interp] =
						(uint32)((((int32)eps_im_interp & 0x1fff) << 13) |
						((int32)eps_re_interp & 0x1fff));
					next_index_interp++;
				}
				while ((next_index_write < index) &&
				       (next_index_write < next_index_interp) &&
				       (next_index_write <= 63)) {
					wlc_phy_table_write_acphy_dac_war(pi,
						epsilon_table_ids[core], 1, next_index_write,
						32, &dst[next_index_write], core);
					next_index_write++;
				}
			}
			if (index <= stopindex) {
				SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
				ASSERT(!(READ_PHYREG(pi, PapdCalStart) & 1));
				corr_papd_index_I = READ_PHYREG(pi, papdCalFirstCorr_I0);
				corr_papd_index_Q = READ_PHYREG(pi, papdCalFirstCorr_Q0);
				corr_papd_index_I_yref = READ_PHYREG(pi, PapdCalYref_I0);
				corr_papd_index_Q_yref = READ_PHYREG(pi, PapdCalYref_Q0);
			}
			corr_sat = ((index == startindex) &&
			        ((ABS(corr_papd_index_I_yref) > 32000) ||
				(ABS(corr_papd_index_Q_yref) > 32000)));
		}
		wlc_phy_stopplayback_acphy(pi);
		/* To cover gctrl only for (the case where startindex <
		 * stopindex is already covered)
		 */
		if (GAIN_CTRL == 1) {
			dst[stopindex] =
				(uint32)((((int32)eps_im_curr & 0x1fff) << 13) |
				((int32)eps_re_curr & 0x1fff));
			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core], 1,
			        stopindex, 32, &dst[stopindex], core);
		} else {
			if ((next_index_interp_init > startindex) &&
			    (next_index_interp_init <= 63)) {
				wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core],
					next_index_interp_init - startindex, startindex, 32,
					&dst[startindex], core);
			}
			/* Fill remaining indices with last result */
			for (index = next_index_interp; index <= stopindex; index++) {
				dst_tmp =
					(uint32)((((int32)eps_im_interp & 0x1fff) << 13) |
					((int32)eps_re_interp & 0x1fff));
				if ((uint32)(((int32)4096 + (int32)eps_re_interp)*
				        ((int32)4096 + (int32)eps_re_interp) +
				        ((int32)eps_im_interp*(int32)eps_im_interp)) > dst_limit) {
					wlc_phy_table_read_acphy_dac_war(pi,
						epsilon_table_ids[core], 1, next_index_write - 2,
						32, &dst[index], core);
				} else {
					dst[index] = dst_tmp;
				}
			}
			if (stopindex >= next_index_write) {
				if ((uint32)(((int32)4096 + (int32)eps_re_interp)*
				        ((int32)4096 + (int32)eps_re_interp) +
				        ((int32)eps_im_interp*(int32)eps_im_interp)) > dst_limit) {
					wlc_phy_table_read_acphy_dac_war(pi,
						epsilon_table_ids[core], 1,
						next_index_write - 2, 32,
						&dst[next_index_write-1], core);
					wlc_phy_table_write_acphy_dac_war(pi,
						epsilon_table_ids[core],
						stopindex-next_index_write+2, next_index_write-1,
						32, &dst[next_index_write-1], core);
				} else {
					wlc_phy_table_write_acphy_dac_war(pi,
						epsilon_table_ids[core],
						stopindex-next_index_write+1,
						next_index_write, 32,
						&dst[next_index_write], core);
				}
			}
			for (index = 0; index < startindex; index++) {
				dst[index] = 0x0;
			}
			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core],
				startindex, 0, 32, dst, core);
		}
		if (GAIN_CTRL == 0)
			wlc_phy_papd_smooth_acphy(pi, 0, 5, 0, 32);
	} else {
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 1);
	        if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev)) {
			if (core == 0) {
				MOD_PHYREGCEE(pi, PapdEnable, 1, papd_compEnb, 0);
				MOD_PHYREGCEE(pi, PapdCalShifts, 1, papd_calEnb, 0);
			} else if (core == 1) {
				MOD_PHYREGCEE(pi, PapdEnable, 0, papd_compEnb, 0);
				MOD_PHYREGCEE(pi, PapdCalShifts, 0, papd_calEnb, 0);
			}
		}
		if (TINY_RADIO(pi)) {
			if ((CHSPEC_IS80(pi->radio_chanspec)) && (PHY_EPAPD(pi))) {
				MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
					epsilonUpdateIterations, 256);
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x100);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x7);
				MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x9);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x9);
			} else {
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
				MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
				if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
					MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
						epsilonUpdateIterations,
						(CHSPEC_IS2G(pi->radio_chanspec))?32:16);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I,
						(CHSPEC_IS2G(pi->radio_chanspec))?0x9:0x7);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q,
						(CHSPEC_IS2G(pi->radio_chanspec))?0x9:0x7);

					if (CHSPEC_IS80(pi->radio_chanspec)) {
						MOD_PHYREG(pi, PapdCalCorrelate,
							papd_calCorrTime, 0x80);
						MOD_PHYREG(pi, PapdCalSettle,
							papd_calSettleTime,
							(CHSPEC_IS2G(pi->radio_chanspec))
							?0x80:0x80);
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
							papdCorrShift, 0x6);

					} else if (CHSPEC_IS40(pi->radio_chanspec)) {
						MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime,
							(CHSPEC_IS2G(pi->radio_chanspec))
							?0x40:0x80);
						MOD_PHYREG(pi, PapdCalSettle,
							papd_calSettleTime,
							(CHSPEC_IS2G(pi->radio_chanspec))
							?0x40:0x40);
						MOD_PHYREGCEE(pi, PapdCalShifts,
							core, papdCorrShift,
							(CHSPEC_IS2G(pi->radio_chanspec))?0x4:0x6);
					} else {
						MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime,
							(CHSPEC_IS2G(pi->radio_chanspec))
							?0x20:0x40);
						MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime,
							(CHSPEC_IS2G(pi->radio_chanspec))
							?0x20:0x20);
						MOD_PHYREGCEE(pi, PapdCalShifts,
							core, papdCorrShift,
							(CHSPEC_IS2G(pi->radio_chanspec))?0x3:0x5);
					}

				} else {
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
					MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
						epsilonUpdateIterations, num_iter);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
				}
				if ((pi->pacalopt == 1) || (pi->pacalopt == 2)) {
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x100);
					MOD_PHYREG(pi, PapdCalShifts0, papdCorrShift0, 0x7);
				}
			}
		} else {
			if (ACMAJORREV_1(pi->pubpi.phy_rev)) {
				/* For One cores */
				MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
				        epsilonUpdateIterations, num_iter);
				/* setup LMS convergence related params */
				if (CHSPEC_IS80(pi->radio_chanspec)) {
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x40);
				} else {
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x20);
				}
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
				MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x8);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x8);
				} else {
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x9);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x9);
				}
			} else if (ACMAJORREV_2(pi->pubpi.phy_rev) ||
			           ACMAJORREV_5(pi->pubpi.phy_rev)) {
				/* For 4350 */
				MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
				        epsilonUpdateIterations, num_iter);
				/* setup LMS convergence related params */
				if (CHSPEC_IS80(pi->radio_chanspec)) {
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x100);
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x40);
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x80);
				} else {
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x20);
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
				}
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
				MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x8);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x8);
				} else {
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x9);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x9);
				}
				if ((CHSPEC_IS80(pi->radio_chanspec)) && (PHY_EPAPD(pi))) {
					MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
						epsilonUpdateIterations, num_iter);
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
					MOD_PHYREG(pi, PapdCalShifts0, papdCorrShift0, 0x4);
					MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
					MOD_PHYREG(pi, PapdCalShifts0, papdLambda_I0, 0x8);
					MOD_PHYREG(pi, PapdCalShifts0, papdLambda_Q0, 0x8);
				}
				if ((CHSPEC_IS2G(pi->radio_chanspec)) && (PHY_EPAPD(pi))) {
					MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
						epsilonUpdateIterations, num_iter);
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
					MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x8);
					MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x8);
				}
			} else {
				MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
					epsilonUpdateIterations, num_iter);
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
				MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
			}
		}
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdYrefAddr, yrefindex);

		/* use s2.10 PAPD epsilon fixed point format */
		MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, 0x1);
		if (calmode == 0) {
			/* Run the PAPD automatic machine on all indices */
			/* setup iter, Yref, start and end address */
			MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, startindex);
			MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, stopindex);

			if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
				coremask = (phy_get_phymode(pi) == PHYMODE_RSDB) ? 1 : 3;
			} else {
				coremask = 7;
			}
			wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3], coremask);
			wlc_phy_tx_tone_acphy_papd(pi, 2000, 186, 0, 0, FALSE);
			OSL_DELAY(100);
			/* start PAPD calibration */
			MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
			MOD_PHYREG(pi, PapdCalStart, papdStart, 1);
			SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
			ASSERT(!(READ_PHYREG(pi, PapdCalStart) & 1));
			SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
			wlc_phy_stopplayback_acphy(pi);
		} else { /* cal mode 1, hardware cal runs single index at a time */
			uint16 mtable_idx;
			uint32 eps_pre, eps, eps_next;
			int32 epspre_r, epspre_i, eps_r, eps_i, epsnext_r, epsnext_i;
			/* run single index of PAPD table only */
			/* (mode 1 which has been used for debug of 4335 PAPD) */
			/* start the tone */
			wlc_phy_tx_tone_acphy_papd(pi, 2000, 186, 0, 0, FALSE);
			coremask = 7;
			wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3], coremask);
			MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
			OSL_DELAY(600);

			for (mtable_idx = startindex; mtable_idx <= stopindex; mtable_idx++) {
				MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref, 0x1);

				MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit,
				           (mtable_idx == startindex) ? 0x0 : 0x1);

				MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, mtable_idx);
				MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, mtable_idx);

				/* start PAPD calibration */
				OSL_DELAY(10);
				MOD_PHYREG(pi, PapdCalStart, papdStart, 1);
				SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
				ASSERT(!(READ_PHYREG(pi, PapdCalStart) & 1));
				SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
				OSL_DELAY(10);

				/* predict the next epsilon point */
				wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core], 1,
					mtable_idx-1, 32, &eps_pre, core);
				wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core], 1,
					mtable_idx, 32, &eps, core);

				if (pi->pacalopt == 2) {
					eps_next = eps;
				} else {
					/* linear extrapolation of prev 2 points to set
					starting point for next eps
					*/

					wlc_phy_papd_decode_epsilon(eps_pre, &epspre_r, &epspre_i);
					wlc_phy_papd_decode_epsilon(eps, &eps_r, &eps_i);
					if (mtable_idx == startindex) {
						epsnext_r = eps_r;
						epsnext_i = eps_i;
					} else {
						epsnext_r = 2*eps_r-epspre_r;
						epsnext_i = 2*eps_i-epspre_i;
					}
					if (epsnext_r >= 4095) {
						epsnext_r = 4095;
					}
					if (epsnext_r <= -4095) {
						epsnext_r = -4095;
					}
					if (epsnext_r < 0) {
						epsnext_r = 8192+epsnext_r;
					}
					if (epsnext_i >= 4095) {
						epsnext_i = 4095;
					}
					if (epsnext_i <= -4095) {
						epsnext_i = -4095;
					}
					if (epsnext_i < 0) {
						epsnext_i = 8192+epsnext_i;
					}
					eps_next = ((uint32)epsnext_i << 13) |
						((uint32)epsnext_r & 0x1fff);
				}

				wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core],
					1, mtable_idx+1, 32, &eps_next, core);

				PHY_PAPD(("\n We are in %u M table iteration", mtable_idx));
			}

			wlc_phy_stopplayback_acphy(pi);
		}
		if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi) &&
		    CHSPEC_IS2G(pi->radio_chanspec)) {
			wlc_phy_papd_smooth_acphy(pi, core, 5, 0, 32);
		}
	}

	/* Calculate epsilon offset, for tiny */
	if (TINY_RADIO(pi)) {
		int32 dig_gain_dB;

		wlc_phy_table_read_acphy(pi, scalar_table_ids[core], 1, 0, 32, &scalartblval);
		papdmult = scalartblval & 0x1fff;

	        temp = (bbmult*cal_tone_mag*papdmult*1000)/(64*1024*100);
		qm_log10((int32)(temp), 0, &temp1, &qQ1);
		dig_gain_dB = ((20*temp1) - (60 << qQ1)) >> qQ1;

		lut_shift = 0;
		if (ACREV_GE(pi->pubpi.phy_rev, 4)) {
			if (CHSPEC_IS5G(pi->radio_chanspec) && (!PHY_EPAPD(pi))) {
				if (IS20MHZ(pi))
					lut_shift = (ACMAJORREV_4(pi->pubpi.phy_rev) &&
						ACMINORREV_1(pi) == 1) ? 1 : 0;
				else if (IS40MHZ(pi))
					lut_shift = (ACMAJORREV_4(pi->pubpi.phy_rev) &&
						ACMINORREV_1(pi) == 1) ? 1 : 2;
				else
					lut_shift = (ACMAJORREV_4(pi->pubpi.phy_rev) &&
						ACMINORREV_1(pi) == 1) ? 3 : 2;
			}
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (IS20MHZ(pi)) {
					lut_shift += pi->pacalshift5g[0];
				} else if (IS40MHZ(pi)) {
					lut_shift += pi->pacalshift5g[1];
				} else {
					lut_shift += pi->pacalshift5g[2];
				}
			} else {
				if (IS20MHZ(pi)) {
					lut_shift += pi->pacalshift2g[0];
				} else if (IS40MHZ(pi)) {
					lut_shift += pi->pacalshift2g[1];
				} else {
					lut_shift += pi->pacalshift2g[2];
				}
			}
		}
		k = -80;
		dac_rf_offset = READ_PHYREGFLDCEE(pi, PapdEnable, core, gain_dac_rf_reg);
		if (dac_rf_offset >= 256) {
			dac_rf_offset = dac_rf_offset - 512;
		}
		epsilonscalartemp = READ_PHYREGFLDCEE(pi, EpsilonTableAdjust, core, epsilonScalar);
		epsilonoffsettemp = k - 2*dig_gain_dB + lut_shift -
			dac_rf_offset*epsilonscalartemp/16;
		if (epsilonoffsettemp < 0) {
			epsilonoffsettemp = 512 + epsilonoffsettemp;
		}
		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset,
			epsilonoffsettemp);
	} /* TINY_RADIO */
}

/* PAPD Functions */
void
wlc_phy_papd_phy_setup_acphy(phy_info_t *pi)
{
	/* XXX Notes:
	 *   - also note that in the driver we do a resetCCA after this to be on the safe
	 *     side; may want to revisit this here, too, in case we run into issues
	 */

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint8 core;
	uint16 sdadc_config = 0;
	uint8 bw_idx;
	uint8 stall_val;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	if (CHSPEC_IS80(pi->radio_chanspec)) {
		bw_idx = 2;
		sdadc_config = sdadc_cfg80;
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

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(!porig->is_orig);
	porig->is_orig = TRUE;
	/* 4335 phy_rev is 7. For phy_rev 2, following things are required. */
	if (ACREV_IS(pi->pubpi.phy_rev, 2)) {
		MOD_PHYREG(pi, RxFeCtrl1, rxfe_bilge_cnt, 4);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);
	}

	porig->RfctrlOverrideGlobalPus = READ_PHYREG(pi, RfctrlOverrideGlobalPus);
	porig->RfctrlCoreGlobalPus = READ_PHYREG(pi, RfctrlCoreGlobalPus);

	FOREACH_CORE(pi, core) {
		porig->txpwridx[core] = pi->u.pi_acphy->txpwrindex[core];

		porig->RfctrlOverrideTxPus[core] = READ_PHYREGCE(pi, RfctrlOverrideTxPus, core);
		porig->RfctrlOverrideRxPus[core] = READ_PHYREGCE(pi, RfctrlOverrideRxPus, core);
		porig->RfctrlOverrideGains[core] = READ_PHYREGCE(pi, RfctrlOverrideGains, core);
		porig->RfctrlOverrideLpfCT[core] = READ_PHYREGCE(pi, RfctrlOverrideLpfCT, core);
		porig->RfctrlOverrideLpfSwtch[core] = READ_PHYREGCE(pi, RfctrlOverrideLpfSwtch,
		                                                    core);
		porig->RfctrlOverrideAfeCfg[core] = READ_PHYREGCE(pi, RfctrlOverrideAfeCfg, core);
		porig->RfctrlOverrideLowPwrCfg[core] = READ_PHYREGCE(pi, RfctrlOverrideLowPwrCfg,
		                                                     core);
		porig->RfctrlOverrideAuxTssi[core] = READ_PHYREGCE(pi, RfctrlOverrideAuxTssi, core);

		porig->RfctrlCoreTxPus[core] = READ_PHYREGCE(pi, RfctrlCoreTxPus, core);
		porig->RfctrlCoreRxPus[core] = READ_PHYREGCE(pi, RfctrlCoreRxPus, core);
		porig->RfctrlCoreTXGAIN1[core] = READ_PHYREGCE(pi, RfctrlCoreTXGAIN1, core);
		porig->RfctrlCoreTXGAIN2[core] = READ_PHYREGCE(pi, RfctrlCoreTXGAIN2, core);
		porig->RfctrlCoreRXGAIN1[core] = READ_PHYREGCE(pi, RfctrlCoreRXGAIN1, core);
		porig->RfctrlCoreRXGAIN2[core] = READ_PHYREGCE(pi, RfctrlCoreRXGAIN2, core);
		porig->RfctrlCoreLpfGain[core] = READ_PHYREGCE(pi, RfctrlCoreLpfGain, core);
		porig->RfctrlCoreLpfCT[core] = READ_PHYREGCE(pi, RfctrlCoreLpfCT, core);
		porig->RfctrlCoreLpfGmult[core] = READ_PHYREGCE(pi, RfctrlCoreLpfGmult, core);
		porig->RfctrlCoreRCDACBuf[core] = READ_PHYREGCE(pi, RfctrlCoreRCDACBuf, core);
		porig->RfctrlCoreLpfSwtch[core] = READ_PHYREGCE(pi, RfctrlCoreLpfSwtch, core);
		porig->RfctrlCoreAfeCfg1[core] = READ_PHYREGCE(pi, RfctrlCoreAfeCfg1, core);
		porig->RfctrlCoreAfeCfg2[core] = READ_PHYREGCE(pi, RfctrlCoreAfeCfg2, core);
		porig->RfctrlCoreLowPwr[core] = READ_PHYREGCE(pi, RfctrlCoreLowPwr, core);
		porig->RfctrlCoreAuxTssi1[core] = READ_PHYREGCE(pi, RfctrlCoreAuxTssi1, core);
		porig->RfctrlCoreAuxTssi2[core] = READ_PHYREGCE(pi, RfctrlCoreAuxTssi2, core);
		porig->Dac_gain[core] = READ_PHYREGCE(pi, Dac_gain, core);

		wlc_phy_get_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&porig->rfseq_txgain[core+0]);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&porig->rfseq_txgain[core+3]);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&porig->rfseq_txgain[core+6]);

		porig->RfctrlIntc[core] = READ_PHYREGCE(pi, RfctrlIntc, core);
	}

	porig->RfseqCoreActv2059 = READ_PHYREG(pi, RfseqCoreActv2059);

	/* XXX Core Activate/Deactivate
		for now, keep all rx's enabled for most realistic rx conditions
	 */
	/* MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, pi->sh->phyrxchain); */
	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, 0x7);
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, 0);
	if (!ACMAJORREV_4(pi->pubpi.phy_rev)) {
		porig->lbFarrowCtrl = READ_PHYREG(pi, lbFarrowCtrl);
		MOD_PHYREG(pi, lbFarrowCtrl, lb_decimator_output_shift, 2);
	}

	FOREACH_CORE(pi, core) {

		/* XXX RF External Settings
		 *   - Power Down External PA,
		 *   - T/R on T to protect against interference
		 */
		/* acphy_rfctrlintc_override  ext_pa 1  $core */
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_2g_papu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_5g_papu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_pa, 1);

		/* acphy_rfctrlintc_override  ext_lna_pu 0  $core */
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_2g_pu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_5g_pu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_lna, 1);

		MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_tx_pu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_rx_pu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_tr_sw, 1);

		/* XXX Required for loopback to work correctly
		   acphy_rfctrl_override fast_nap_bias_pu 1 $core
		*/
		/* XXX RfCtrl
		*   - turn off Internal PA
		*   - turn off LNA1 to protect against interference and reduce thermal noise
		*   - force LPF to Rx Chain
		*   - force LPF bw
		*   - NOTE: this also saves off state of possible Tx/Rx gain override states
		*/

		/* Setting the SD-ADC related stuff */
		/* acphy_rfctrl_override iqadc 1 $core */
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_mode, sdadc_config & 0x7);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_mode, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, afe_iqadc_pwrup,
		             (sdadc_config >> 3) & 0x3f);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_flashhspd,
		             (sdadc_config >> 9) & 0x1);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_flashhspd, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_ctrl_flash17lvl,
		             (sdadc_config >> 10) & 0x1);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_ctrl_flash17lvl, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_adc_bias,
		             (sdadc_config >> 11) & 0x3);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_adc_bias, 1);

		/* acphy_rfctrl_override pa_pwrup 1 $core; */
		MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, pa_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, pa_pwrup, 1);
		/* acphy_rfctrl_override lpf_nrssi_pwrup 0 $core */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_nrssi_pwrup, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_nrssi_pwrup, 1);
		/* acphy_rfctrl_override wb1_pu 0 $core */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1g_pu, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1g_pu, 1);
		} else {
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1a_pu, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1a_pu, 1);
		}

		/* xxx Debug printfs:
		printf("\nRfctrlCoreRxPus = %x", READ_PHYREG(pi, RfctrlCoreRxPus0));
		printf("\nRfctrlCoreTxPus = %x", READ_PHYREG(pi, RfctrlCoreTxPus0));
		*/

		/* xxx This is what is done in tcl:
		acphy_rfctrl_override lpf_bq1_bw [expr $def(phybw) + 0] $core;
		*/
		MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, lpf_bq1_bw, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfCT, core, lpf_bq1_bw, bw_idx);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, lpf_bq2_bw, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, lpf_rc_bw, 1);
		if ((RADIOREV(pi->pubpi.radiorev) == 17) || (RADIOREV(pi->pubpi.radiorev) == 23) ||
		(RADIOREV(pi->pubpi.radiorev) == 25)) {
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT, core, lpf_bq2_bw, 7);
			MOD_PHYREGCE(pi, RfctrlCoreRCDACBuf, core, lpf_rc_bw, 7);
		} else {
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT, core, lpf_bq2_bw, bw_idx + 5);
			MOD_PHYREGCE(pi, RfctrlCoreRCDACBuf, core, lpf_rc_bw, bw_idx + 5);
		}

		porig->PapdEnable[core] = READ_PHYREGCE(pi, PapdEnable, core);
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);

		porig->forceFront[core] =  READ_PHYREGCE(pi, forceFront, core);
		if (ACMAJORREV_2(pi->pubpi.phy_rev) || ACMAJORREV_5(pi->pubpi.phy_rev) ||
			ACMAJORREV_4(pi->pubpi.phy_rev)) {
			MOD_PHYREGCE(pi, forceFront, core, freqEst, 1);
			MOD_PHYREGCE(pi, forceFront, core, freqCor, 1);
		}
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

/* Run PAPD calibration for Tiny */
void
wlc_phy_tiny_papd_cal_run_acphy(phy_info_t *pi,
	uint8 tx_pre_cal_pwr_ctrl_state)
{
	uint8 j;
	uint8 core = 0;
	uint32 initvalue = 0;
	bool suspend = TRUE;
	uint8 yref = 5, start = 5;
	uint8 tx_atten = 3, rx_atten = 3;
	uint16 numiter = 128;
	uint8 scalar_table_ids[] = { ACPHY_TBL_ID_SCALAR0, ACPHY_TBL_ID_SCALAR1,
		ACPHY_TBL_ID_SCALAR2};
	uint8 eps_table_ids[] = {ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};

	wlc_phy_dac_rate_mode_acphy(pi, 1);

	FOREACH_CORE(pi, core) {
		/* clear eps table  */
		for (j = 0; j < 64; j++) {
			wlc_phy_table_write_acphy_dac_war(pi, eps_table_ids[core], 1, j, 32,
				&initvalue, core);
		}
		/* initialize scalar table */
		wlc_phy_table_write_acphy(pi, scalar_table_ids[core], 64, 0, 32,
			acphy_papd_scaltbl);
	}

	/* acphy_papd_cal_phy_setup */
	wlc_phy_papd_phy_setup_acphy(pi);

	if (ACMAJORREV_4(pi->pubpi.phy_rev)) {
		if (HW_RADIOREV(pi->pubpi.radiorev) == 13) {
			tx_atten = (CHSPEC_IS2G(pi->radio_chanspec) == 1) ? 3 : 2;
			rx_atten = (CHSPEC_IS2G(pi->radio_chanspec) == 1) ? 3 : 3;
		} else {
			tx_atten = (CHSPEC_IS2G(pi->radio_chanspec) == 1) ? 3 : 0;
			rx_atten = (CHSPEC_IS2G(pi->radio_chanspec) == 1) ? 3 : 1;
		}
	}
		/* 20691/20693_loopback_papd 0 $tx_atten $rx_atten */
	wlc_phy_papd_radio_loopback_setup_acphy_tiny(pi, tx_atten, rx_atten);

	if (CHSPEC_IS2G(pi->radio_chanspec) && ACREV_GE(pi->pubpi.phy_rev, 4) &&
		(!PHY_EPAPD(pi)) && (!ACMAJORREV_4(pi->pubpi.phy_rev))) {
		wlc_phy_papd_rx_gain_ctrl_acphy(pi);
	}

	if (ACMAJORREV_4(pi->pubpi.phy_rev))
		wlc_dcc_fsm_reset(pi);

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, &pi->regs->maccontrol) & MCTL_EN_MAC);
	if (!suspend) {
		printf("MAC was not suspended before calling wlc_phy_txpwr_papd_cal_run_acphy!\n");
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Disable BT as it affects PAPD CAL */
	wlc_btcx_override_enable(pi);
	/* Disable CRS */
	wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);

	FOREACH_CORE(pi, core) {
		if ((ACMAJORREV_4(pi->pubpi.phy_rev)) && (DO_PAPD_GAINCTRL)) {
			uint8 txidx = wlc_phy_papd_cal_gain_cntl_tiny(pi, 64, core, 50, 5, 63);
			wlc_phy_txpwr_by_index_acphy(pi, core + 1, txidx);
		}
		wlc_phy_papd_cal_acphy(pi, numiter, core, start, yref, 63);

		/* eps scalar */
		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonScalar, 8);

		/* Write the LUT table with 0's for index idx=0 upto idx=start */
		for (j = 0; j < start; j++) {
			wlc_phy_table_write_acphy_dac_war(pi, eps_table_ids[core], 1, j, 32,
				&initvalue, core);
		}
	}

	/* acradio papd_cal_cleanup */
	/* wlc_phy_papdcal_radio_cleanup_acphy(pi); */
	wlc_phy_papd_radio_cleanup_acphy_tiny(pi);

	/* phy clean up */
	wlc_phy_papd_phy_cleanup_acphy(pi);

	/* Enable CRS */
	wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
	/* Enable BT */
	wlc_phy_btcx_override_disable(pi);

	/* acphy_papd on */
	FOREACH_CORE(pi, core) {
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
		if ACMAJORREV_4(pi->pubpi.phy_rev) {
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compCckEnb, 1);
		} else {
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compCckEnb, 0);
		}
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 0);
	}

}
