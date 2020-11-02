/*
 * RADIO control module implementation - shared by PHY type specific implementations.
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
#include <bcmdevs.h>
#include <bcmutils.h>
#include <phy_api.h>
#include <phy_utils_radio.h>

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>

void
phy_utils_parse_idcode(phy_info_t *pi, uint32 idcode)
{
	pi->pubpi.radioid = (idcode & IDCODE_ID_MASK) >> IDCODE_ID_SHIFT;
	pi->pubpi.radiorev = (idcode & IDCODE_REV_MASK) >> IDCODE_REV_SHIFT;
	pi->pubpi.radiover = (idcode & IDCODE_VER_MASK) >> IDCODE_VER_SHIFT;

#if defined(DSLCPE) && defined(CONFIG_BCM963268)
	if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
		/* overriding radiover to 8 */
		pi->pubpi.radiorev = 8;
	}
#endif /* defined(DSLCPE) && defined(CONFIG_BCM963268) */
}

int
phy_utils_valid_radio(phy_info_t *pi)
{
	if (VALID_RADIO(pi, RADIOID(pi->pubpi.radioid))) {
		/* ensure that the built image matches the target */
#ifdef BCMRADIOID
		if (pi->pubpi.radioid != BCMRADIOID)
			PHY_ERROR(("%s: Chip's radioid=0x%x, BCMRADIOID=0x%x\n",
			           __FUNCTION__, pi->pubpi.radioid, BCMRADIOID));
		ASSERT(pi->pubpi.radioid == BCMRADIOID);
#endif // endif
#ifdef BCMRADIOREV
		if (pi->pubpi.radiorev != BCMRADIOREV)
			PHY_ERROR(("%s: Chip's radiorev=%d, BCMRADIOREV=%d\n",
			           __FUNCTION__, pi->pubpi.radiorev, BCMRADIOREV));
		ASSERT(pi->pubpi.radiorev == BCMRADIOREV);
#endif // endif
		return BCME_OK;
	} else {
		PHY_ERROR(("wl%d: %s: Unknown radio ID: 0x%x rev 0x%x phy %d, phyrev %d\n",
		           pi->sh->unit, __FUNCTION__,
		           RADIOID(pi->pubpi.radioid), RADIOREV(pi->pubpi.radiorev),
		           pi->pubpi.phy_type, pi->pubpi.phy_rev));
		return BCME_UNSUPPORTED;
	}
}
