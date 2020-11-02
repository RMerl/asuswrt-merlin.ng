/*
 * ACPHY Calibration Manager module interface (to other PHY modules).
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

#ifndef _phy_ac_calmgr_h_
#define _phy_ac_calmgr_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_calmgr.h>

#include <phy_utils_math.h>

/* forward declaration */
typedef struct phy_ac_calmgr_info phy_ac_calmgr_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_calmgr_info_t *phy_ac_calmgr_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_calmgr_info_t *cmn_info);
void phy_ac_calmgr_unregister_impl(phy_ac_calmgr_info_t *info);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* mphase phases for ACPHY */
enum {
	ACPHY_CAL_PHASE_IDLE = 0,
	ACPHY_CAL_PHASE_INIT = 1,
	ACPHY_CAL_PHASE_TX0,
	ACPHY_CAL_PHASE_TX1,
	ACPHY_CAL_PHASE_TX2,
	ACPHY_CAL_PHASE_TX3,
	ACPHY_CAL_PHASE_TX4,
	ACPHY_CAL_PHASE_TX5,
	ACPHY_CAL_PHASE_TX6,
	ACPHY_CAL_PHASE_TX7,
	ACPHY_CAL_PHASE_TX8,
	ACPHY_CAL_PHASE_TX9,
	ACPHY_CAL_PHASE_TX_LAST,
	ACPHY_CAL_PHASE_PAPDCAL,	/* IPA */
	ACPHY_CAL_PHASE_TXPRERXCAL0,	/* bypass Biq2 pre rx cal */
	ACPHY_CAL_PHASE_TXPRERXCAL1,	/* bypass Biq2 pre rx cal */
	ACPHY_CAL_PHASE_TXPRERXCAL2,	/* bypass Biq2 pre rx cal */
	ACPHY_CAL_PHASE_RXCAL,
	ACPHY_CAL_PHASE_RSSICAL,
	ACPHY_CAL_PHASE_IDLETSSI
};

#endif /* _phy_ac_calmgr_h_ */
