/*
 * PHY utils - status access functions.
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
#include <bcmutils.h>

#include <phy_utils_reg.h>
#include <phy_utils_status.h>
#include <phy_utils_api.h>

#include <wlc_phy_int.h>

uint16
phy_utils_get_bwstate(phy_info_t *pi)
{
	return pi->bw;
}

void
phy_utils_set_bwstate(phy_info_t *pi, uint16 bw)
{
	pi->bw = bw;
}

bool
phy_utils_ismuted(phy_info_t *pi)
{
	return PHY_MUTED(pi);
}

uint64
phy_utils_get_time_usec(phy_info_t *pi)
{
	uint64 time_lo, time_hi;

	time_lo = R_REG(GENERIC_PHY_INFO(pi)->osh, &pi->regs->tsf_timerlow);
	time_hi = R_REG(GENERIC_PHY_INFO(pi)->osh, &pi->regs->tsf_timerhigh);

	return ((time_hi << 32) | time_lo);
}

bool
BCMATTACHFN(phy_utils_get_phyversion)(phy_info_t *pi, uint16 *phytype, uint16 *phyrev,
	uint16 *radioid, uint16 *radiover, uint16 *phy_minor_rev)
{
	*phytype = (uint16)pi->pubpi.phy_type;
	*phyrev = (uint16)pi->pubpi.phy_rev;
	*radioid = RADIOID(pi->pubpi.radioid);
	*radiover = RADIOREV(pi->pubpi.radiorev);
	*phy_minor_rev = (uint16)pi->pubpi.phy_minor_rev;

	return TRUE;
}

uint32
BCMATTACHFN(phy_utils_get_coreflags)(phy_info_t *pi)
{
	return pi->pubpi.coreflags;
}

uint8
BCMATTACHFN(phy_utils_get_corenum)(phy_info_t *pi)
{
	return PHYCORENUM(pi->pubpi.phy_corenum);
}
