/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abg
 * PHY ioctl processing of Broadcom BCM43XX 802.11abg
 * Networking Device Driver.
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

/*
 * This file contains high portion PHY ioctl processing and table.
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLC_HIGH
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wl_dbg.h>
#include <wlc.h>
#include <bcmwifi_channels.h>

static const wlc_ioctl_cmd_t phy_ioctls[] = {
	{WLC_RESTART, 0, 0},
#if defined(BCMDBG) || defined(WLTEST)
	{WLC_GET_RADIOREG, 0, 0},
	{WLC_SET_RADIOREG, 0, 0},
#endif // endif
#if defined(BCMDBG)
	{WLC_GET_TX_PATH_PWR, 0, 0},
	{WLC_SET_TX_PATH_PWR, 0, 0},
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
	{WLC_GET_PHYREG, 0, 0},
	{WLC_SET_PHYREG, 0, 0},
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
	{WLC_GET_TSSI, 0, 0},
	{WLC_GET_ATTEN, 0, 0},
	{WLC_SET_ATTEN, 0, 0},
	{WLC_GET_PWRIDX, 0, 0},
	{WLC_SET_PWRIDX, 0, 0},
	{WLC_LONGTRAIN, 0, 0},
	{WLC_EVM, 0, 0},
	{WLC_FREQ_ACCURACY, 0, 0},
	{WLC_CARRIER_SUPPRESS, 0, 0},
#endif // endif
	{WLC_GET_INTERFERENCE_MODE, 0, 0},
	{WLC_SET_INTERFERENCE_MODE, 0, 0},
	{WLC_GET_INTERFERENCE_OVERRIDE_MODE, 0, 0},
	{WLC_SET_INTERFERENCE_OVERRIDE_MODE, 0, 0},
};

static int
phy_legacy_vld_proc(wlc_info_t *wlc, uint16 cmd, void *arg, uint len)
{
	int bcmerror = BCME_OK;

#if defined(BCMDBG) || defined(WLTEST)
	uint band;

	/* optional band is stored in the second integer of incoming buffer */
	band = (len < (int)(2 * sizeof(int))) ? WLC_BAND_AUTO : ((int *)arg)[1];
#endif // endif

	switch (cmd) {
#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_RADIOREG:
	case WLC_SET_RADIOREG:
		bcmerror = wlc_iocregchk(wlc, band);
		break;

#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_PHYREG:
	case WLC_SET_PHYREG:
		bcmerror = wlc_iocregchk(wlc, band);
		break;

#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
	case WLC_GET_TSSI:
	case WLC_GET_ATTEN:
	case WLC_LONGTRAIN:
	case WLC_FREQ_ACCURACY:
	case WLC_CARRIER_SUPPRESS:
		bcmerror = wlc_iocregchk(wlc, WLC_BAND_AUTO);
		break;

	case WLC_SET_ATTEN:
	case WLC_GET_PWRIDX:
	case WLC_SET_PWRIDX:
		bcmerror = wlc_iocbandchk(wlc, NULL, 0, NULL, FALSE);
		break;

	case WLC_EVM: {
		/* EVM is only defined for CCK rates */
		ratespec_t *rspec = (((uint *)arg) + 1);

		if (!rspec || !IS_CCK(*rspec)) {
			bcmerror = BCME_BADARG;
			break;
		}

		bcmerror = wlc_iocregchk(wlc, WLC_BAND_AUTO);
		break;
	}
#endif // endif

#if defined(BCMDBG)
	case WLC_GET_TX_PATH_PWR:
	case WLC_SET_TX_PATH_PWR:
		/* check for bandlock */
		if (wlc->band->radioid != BCM2050_ID)
			bcmerror = wlc_iocbandchk(wlc, NULL, 0, NULL, FALSE);

		break;
#endif /* BCMDBG */
	}

	return bcmerror;
}

#ifdef WLC_HIGH_ONLY
#include <bcm_xdr.h>

static bool
phy_legacy_cmd_proc(wlc_info_t *wlc, uint16 cmd, void *arg, uint len, bcm_xdr_buf_t *b)
{
	switch (cmd) {
#if defined(BCMDBG) || defined(WLTEST)
	case WLC_EVM: {
		/* EVM is only defined for CCK rates */
		ratespec_t *rspec = (((uint *)arg) + 1);
		wlc_rateset_t rs;

		/* init basic rate lookup :
		 * XXX confirm this change is ok as it does
		 * not pick up "current" default rateset but
		 * instead gets the phy default rateset.
		 */
		wlc_default_rateset(wlc, &rs);
		wlc_rate_lookup_init(wlc, &rs);

		*(((uint *)arg) + 1) = RSPEC2RATE(*rspec);
		break;
	}
#endif // endif
	}

	return FALSE;
}

static bool
phy_legacy_result_proc(wlc_info_t *wlc, uint16 cmd, bcm_xdr_buf_t *b, void *arg, uint len)
{
	if ((cmd == WLC_EVM) || (cmd == WLC_FREQ_ACCURACY) ||
	    (cmd == WLC_LONGTRAIN) || (cmd == WLC_CARRIER_SUPPRESS)) {
		if (arg != NULL)
			wlc->pub->phytest_on = !!(*(uint32 *)arg);
	}

	return FALSE;
}
#endif /* WLC_HIGH_ONLY */
#endif /* WLC_HIGH */

#ifdef WLC_LOW
#include <typedefs.h>
#include <wlc_phy_int.h>

static int
phy_legacy_doioctl(void *ctx, uint16 cmd, void *arg, uint len, bool *ta_ok)
{
	return wlc_phy_ioctl_dispatch((phy_info_t *)ctx, (int)cmd, (int)len, arg, ta_ok);
}
#endif /* WLC_LOW */

/* register ioctl table to the system */
#include <phy_api.h>

#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>

int phy_legacy_register_ioct(phy_info_t *pi, wlc_iocv_info_t *ii);

int
BCMATTACHFN(phy_legacy_register_ioct)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_ioct_desc_t iocd;

	ASSERT(ii != NULL);

	wlc_iocv_init_iocd(phy_ioctls, ARRAYSIZE(phy_ioctls),
	                   phy_legacy_vld_proc,
	                   phy_legacy_cmd_proc, phy_legacy_result_proc,
	                   phy_legacy_doioctl, pi,
	                   &iocd);

	return wlc_iocv_register_ioct(ii, &iocd);
}
