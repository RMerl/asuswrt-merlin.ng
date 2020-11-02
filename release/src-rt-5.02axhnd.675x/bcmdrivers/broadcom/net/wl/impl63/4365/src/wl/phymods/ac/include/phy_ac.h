/*
 * ACPHY Core module internal interface (to other PHY modules).
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

#ifndef _phy_ac_h_
#define _phy_ac_h_

#include <phy.h>
#include <wlc_phy_int.h> /* *** !!! To be removed !!! *** */

#ifdef ALL_NEW_PHY_MOD
typedef struct phy_ac_info phy_ac_info_t;
#else
/* < TODO: all these are going away... */
typedef struct phy_info_acphy phy_ac_info_t;
/* TODO: all these are going away... > */
#endif /* ALL_NEW_PHY_MOD */
#define PHY_REG_READ_REV(pi, phy_type, reg_name, field, rev) \
		((phy_utils_read_phyreg(pi, phy_type##_##reg_name(rev)) & \
		phy_type##_##reg_name##_##field##_##MASK(rev)) >> \
		phy_type##_##reg_name##_##field##_##SHIFT(rev))
void phy_ac_update_phycorestate(phy_info_t *pi);
void phy_regaccess_war_acphy(phy_info_t *pi);

/* ************************************************************ */
/* ACPHY module							*/
/* function declarations / intermodule api's			*/
/* ************************************************************ */

/* ************************************************************ */

/* *** Needs to be moved to TPC header once the AC modules are created *** */
#define PHY_TXPWR_MIN_ACPHY	1	/* for acphy devices */
#define PHY_TXPWR_MIN_ACPHY1X1EPA	8	/* for acphy1x1 ipa devices */
#define PHY_TXPWR_MIN_ACPHY1X1IPA	1	/* for acphy1x1 ipa devices */
#define PHY_TXPWR_MIN_ACPHY2X2	5	/* for 2x2 acphy devices */
/* Offset of Target Power per channel in 2GHz feature,
 * designed for 4354 iPa with LTE filter, but can support any ACPHY chip
 * XXX also to be moved to TPC header once the AC modules are created
 */
#ifdef POWPERCHANNL
#define CH20MHz_NUM_2G	14 /* Number of 20MHz channels in 2G band */
#define PWR_PER_CH_NORM_TEMP	0	/* Temp zone  in norm for power per channel  */
#define PWR_PER_CH_LOW_TEMP		1	/* Temp zone  in low for power per channel  */
#define PWR_PER_CH_HIGH_TEMP	2	/* Temp zone  in high for power per channel  */
#define PWR_PER_CH_TEMP_MIN_STEP	5	/* Min temprature step for sensing  */
#define PWR_PER_CH_NEG_OFFSET_LIMIT_QDBM 20 /* maximal power reduction offset: 5dB =20 qdBm */
#define PWR_PER_CH_POS_OFFSET_LIMIT_QDBM 12 /* maximal power increase offset: 3dB =12 qdBm */
#endif // endif

#endif /* _phy_ac_h_ */
