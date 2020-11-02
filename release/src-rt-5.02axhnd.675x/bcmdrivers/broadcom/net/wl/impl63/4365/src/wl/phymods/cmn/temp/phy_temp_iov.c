/*
 * TEMPsense module implementation - iovar table/handlers & registration
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

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <phy_api.h>
#include <phy_temp.h>
#include "phy_temp_st.h"
#include <phy_temp_iov.h>

#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>

/* iovar table */
enum {
	IOV_PHY_TEMPTHRESH = 1,
	IOV_PHY_TEMP_HYSTERESIS = 2,
	IOV_PHY_TEMPOFFSET = 3,
	IOV_PHY_TEMPSENSE_OVERRIDE = 4
};

#ifdef WLC_HIGH
#include <siutils.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>

static const bcm_iovar_t phy_temp_iovars[] = {
	{"phy_tempthresh", IOV_PHY_TEMPTHRESH, 0, IOVT_INT16, 0},
	{"phy_temp_hysteresis", IOV_PHY_TEMP_HYSTERESIS, 0, IOVT_UINT8, 0},
#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX)
	{"phy_tempoffset", IOV_PHY_TEMPOFFSET, 0, IOVT_INT8, 0},
	{"phy_tempsense_override", IOV_PHY_TEMPSENSE_OVERRIDE, 0, IOVT_UINT16, 0},
#endif /* BCMDBG || WLTEST || MACOSX */
	{NULL, 0, 0, 0, 0}
};
#endif /* WLC_HIGH */

#ifndef ALL_NEW_PHY_MOD
#include <wlc_phy_int.h>
#endif // endif

#ifdef WLC_LOW
static int
phy_temp_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	phy_temp_info_t *ti = pi->tempi;
	phy_txcore_temp_t *temp = phy_temp_get_st(ti);
	int err = BCME_OK;
	int32 *ret_int_ptr;
	int int_val = 0;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)a;

	(void)temp;
	(void)ret_int_ptr;

	switch (aid) {
#if defined(BCMDBG) || defined(WLTEST)
	case IOV_GVAL(IOV_PHY_TEMPTHRESH):
		*ret_int_ptr = (int32) temp->disable_temp;
		break;

	case IOV_SVAL(IOV_PHY_TEMPTHRESH):
		temp->disable_temp = (uint8) int_val;
		temp->enable_temp = temp->disable_temp - temp->hysteresis;
		break;

	case IOV_GVAL(IOV_PHY_TEMPOFFSET):
		*ret_int_ptr = (int32) pi->phy_tempsense_offset;
		break;

	case IOV_SVAL(IOV_PHY_TEMPOFFSET):
		pi->phy_tempsense_offset = (int8) int_val;
		break;

	case IOV_GVAL(IOV_PHY_TEMPSENSE_OVERRIDE):
		*ret_int_ptr = (int32)pi->tempsense_override;
		break;

	case IOV_SVAL(IOV_PHY_TEMPSENSE_OVERRIDE):
		pi->tempsense_override = (uint16)int_val;
		break;

	case IOV_GVAL(IOV_PHY_TEMP_HYSTERESIS):
		*ret_int_ptr = (int32)temp->hysteresis;
		break;

	case IOV_SVAL(IOV_PHY_TEMP_HYSTERESIS):
		temp->hysteresis = (uint8)int_val;
		temp->enable_temp = temp->disable_temp - temp->hysteresis;
		break;

#endif /* BCMDBG || WLTEST */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}
#endif /* WLC_LOW */

/* register iovar table to the system */
int
BCMATTACHFN(phy_temp_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_temp_iovars,
	                   NULL, NULL,
	                   phy_temp_doiovar, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
