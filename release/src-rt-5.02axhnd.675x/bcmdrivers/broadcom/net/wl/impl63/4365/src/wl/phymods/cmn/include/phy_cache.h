/*
 * CACHE module internal interface (to other PHY modules).
 *
 * This cache is dedicated to operating chanspec contexts (see phy_chanmgr_notif.h).
 *
 * Each cache entry once 'used' has a corresponding operating chanspec context.
 * The current operating chanspec context is the operating chanspec context whose
 * chanspec is programmed as the current radio chanspec. The cache entry that is
 * associated with the current operating chanspec context is called the current
 * cache entry.
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

#ifndef _phy_cache_h_
#define _phy_cache_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_cache_info phy_cache_info_t;

/* ================== interface for attach/detach =================== */

/* attach/detach */
phy_cache_info_t *phy_cache_attach(phy_info_t *pi);
void phy_cache_detach(phy_cache_info_t *ci);

/* ================== interface for calibration modules =================== */

/* forward declaration */
typedef uint32 phy_cache_cubby_id_t;

/*
 * Reserve a cubby in each cache entry and register the client callbacks and context.
 * - 'init' callback is optional and is invoked whenver a cache entry is made current
 *   but before any 'save' operation is performed to the cubby. It gives the cubby
 *   client a chance to initialize its relevant states to known ones.
 * - 'save' callback is mandatory and is invoked to save client states to the cubby.
 *   The invocation can happen automatically when a cache entry is make non-current
 *   and the cubby is reserved as AUTO SAVE cubby; or manually when phy_cache_save
 *   function is called.
 * - 'restore' callback is mandatory and is invoked to restore client states from
 *   from the cubby. It happens when a cache entry is made current.
 * - 'dump' callback is optional and is for debugging and dumpping the cubby.
 *
 * - 'ccid' is the cubby ID when the function is successfully called. It is used
 *   to call other cache module functions. It is also used to register a calibration
 *   callback to the calibration management (calmgr) module.
 */
typedef void phy_cache_ctx_t;

typedef void (*phy_cache_init_fn_t)(phy_cache_ctx_t *ctx);
typedef int (*phy_cache_save_fn_t)(phy_cache_ctx_t *ctx, uint8 *buf);
typedef int (*phy_cache_restore_fn_t)(phy_cache_ctx_t *ctx, uint8 *buf);
typedef int (*phy_cache_dump_fn_t)(phy_cache_ctx_t *ctx, uint8 *buf, struct bcmstrbuf *b);

int phy_cache_reserve_cubby(phy_cache_info_t *ci, phy_cache_init_fn_t init,
	phy_cache_save_fn_t save, phy_cache_restore_fn_t restore, phy_cache_dump_fn_t dump,
	phy_cache_ctx_t *ctx, uint32 size, uint32 flags, phy_cache_cubby_id_t *ccid);

/* cache cubby reservation flag */
#define PHY_CACHE_FLAG_AUTO_SAVE (1<<0)	/* save callback is invoked automatically
					 * by cache module when one makes the
					 * current cache entry non-current.
					 */

/* cache cubby reservation id */
#define PHY_CACHE_CUBBY_INV_ID	-1	/* invalid id - indicate to calmgr
					 * to bypass cache save operation.
					 */

/* ================== interface for calibration mgmt. module =================== */

/*
 * Save results after a calibration phase or module is finished.
 */
int phy_cache_save(phy_cache_info_t *ci, phy_cache_cubby_id_t ccid);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
extern int	wlc_phy_cal_cache_init(wlc_phy_t *ppi);
extern void wlc_phy_cal_cache_deinit(wlc_phy_t *ppi);
#endif // endif
#if defined(PHYCAL_CACHING) || defined(PHYCAL_CACHE_SMALL)
extern void wlc_phy_cal_cache_set(wlc_phy_t *ppi, bool state);
extern bool wlc_phy_cal_cache_get(wlc_phy_t *ppi);
#endif // endif
/* Calibration caching Module (regular caching, per channel, smart cal) */
#if defined(PHYCAL_CACHING) || defined(WLMCHAN)
extern int wlc_phy_create_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern void wlc_phy_destroy_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern int wlc_phy_invalidate_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern int wlc_phy_reuse_chanctx(wlc_phy_t *ppi, chanspec_t chanspec);
extern void wlc_phy_get_cachedchans(wlc_phy_t *ppi, chanspec_t *chanlist);
extern int32 wlc_phy_get_est_chanset_time(wlc_phy_t *ppi, chanspec_t chanspec);
extern bool wlc_phy_chan_iscached(wlc_phy_t *ppi, chanspec_t chanspec);
#endif // endif

#endif /* _phy_cache_h_ */
