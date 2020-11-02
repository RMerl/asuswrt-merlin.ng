/*
 * WatchDog module interface (to other PHY modules).
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

#ifndef _phy_wd_h_
#define _phy_wd_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_wd_info phy_wd_info_t;

/* attach/detach */
phy_wd_info_t *phy_wd_attach(phy_info_t *pi);
void phy_wd_detach(phy_wd_info_t *ri);

/*
 * Add a watchdog callback fn for a module.
 *
 * The callback is invoked at every period in general when now % period == 0.
 * The callback returns TRUE to indicate it has finished the task for the current
 * period; it returns FALSE otherwise. In the case the callback indicates it hasn't
 * finished the task for the current period when the callback is invoked the callback
 * will be invoked again at the next watchdog tick (1s later) repeately until the
 * callback indicates it's done the task.
 */
typedef void phy_wd_ctx_t;
typedef bool (*phy_wd_fn_t)(phy_wd_ctx_t *ctx);
/*
 * WATCHDOG callback periods.
 */
typedef enum phy_wd_prd {
	PHY_WD_PRD_1TICK = 1,		/* 1s */
	PHY_WD_PRD_FAST = 15,		/* 15s */
	PHY_WD_PRD_SLOW = 60,		/* 60s */
	PHY_WD_PRD_GLACIAL = 120,	/* 120s */
} phy_wd_prd_t;
/*
 * WATCHDOG callback execution orders.
 * Note: Keep the enums between 0 and 255!
 */
typedef enum phy_wd_order {
	PHY_WD_1TICK_START = 0,
	PHY_WD_1TICK_TPC,
	PHY_WD_1TICK_NOISE_STOP,
	PHY_WD_1TICK_INTF_NOISE,
	PHY_WD_1TICK_NOISE_ACI,
	PHY_WD_FAST_BTCX,
	PHY_WD_FAST_RADIO,
	PHY_WD_1TICK_NOISE_START,
	PHY_WD_1TICK_NOISE_RESET,
	PHY_WD_1TICK_CALMGR,
	PHY_WD_GLACIAL_CAL
} phy_wd_order_t;
/*
 * WATCHDOG callback flags.
 * Note: Keep the enums between 0 and 2^16 - 1!
 */
typedef enum phy_wd_flag {
	PHY_WD_FLAG_NONE = 0,

	/* defer until next watchdog tick (1s) */
	PHY_WD_FLAG_SCAN_DEFER = 0x01,
	PHY_WD_FLAG_PLT_DEFER = 0x02,
	PHY_WD_FLAG_AS_DEFER = 0x04,

	/* skip the period if in progress */
	PHY_WD_FLAG_SCAN_SKIP = 0x10,
	PHY_WD_FLAG_PLT_SKIP = 0x20,
	PHY_WD_FLAG_AS_SKIP = 0x40,

	/* combinations */
	PHY_WD_FLAG_DEF_DEFER = (PHY_WD_FLAG_SCAN_DEFER |
	                         PHY_WD_FLAG_PLT_DEFER |
	                         PHY_WD_FLAG_AS_DEFER),
	PHY_WD_FLAG_DEF_SKIP = (PHY_WD_FLAG_SCAN_SKIP |
	                        PHY_WD_FLAG_PLT_SKIP |
	                        PHY_WD_FLAG_AS_SKIP),

	/* multi-channel aware callback */
	PHY_WD_FLAG_MCHAN_AWARE = 0x100
} phy_wd_flag_t;

/* Add a watchdog callback fn. Return BCME_XXXX. */
#ifndef BCM_OL_DEV
int phy_wd_add_fn(phy_wd_info_t *wi, phy_wd_fn_t fn, phy_wd_ctx_t *ctx,
	phy_wd_prd_t prd, phy_wd_order_t order, phy_wd_flag_t flags);
#else
#define phy_wd_add_fn(wi, fn, ctx, prd, order, flags) ((void)(fn), BCME_OK)
#endif /* BCM_OL_DEV */

#endif /* _phy_wd_h_ */
