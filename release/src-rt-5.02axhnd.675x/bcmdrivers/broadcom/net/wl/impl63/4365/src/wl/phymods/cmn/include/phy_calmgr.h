/*
 * CALibrationManaGeR module internal interface (to other PHY modules).
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
 * Calibration manager works with calibration modules and controls calibration process.
 *
 * It provides two registries for calibration and other modules to register callbacks:
 *
 * - calibration trigger registry
 * - calibration registry
 *
 * Modules register callbacks in each regitry at attach time by calling interfaces provided
 * in this header file in order to participate in the calibration trigger or calibration
 * process.
 *
 * The calibration manager queries each registered callback in the "calibration trigger
 * registry" for a decision if it should start the calibration process. The result of
 * these callbacks decides if it is time to start the calibration process.
 *
 * Once a calibration process is started the calibration manager interacts with the registered
 * callbacks in the "calibration registry" and controls the entire calibration process. The
 * calibration process may be done in a single calibration manager invocation or can be broken
 * into multiple phases and be done in multipel calibration manager invocations based on the
 * requested mode. In multiple phase calibration mode the calibration manager is responsible
 * for maintaining the calibration module state (index) needed to run the calibration while
 * each calibration module is responsible for maintaining the phase state (index) across
 * multiple calibration manager invocations.
 */

#ifndef _phy_calmgr_h_
#define _phy_calmgr_h_

#include <typedefs.h>
#include <phy_api.h>
#include <phy_cache.h>

/* forward declaration */
typedef struct phy_calmgr_info phy_calmgr_info_t;

/* ******** interface for Core module ******** */

/* attach/detach */
phy_calmgr_info_t *phy_calmgr_attach(phy_info_t *pi);
void phy_calmgr_detach(phy_calmgr_info_t *ci);

/* ******** interface for calibration modules ******** */

/*
 * Add a calibration trigger function in calibration trigger registry.
 */

/* Calibration trigger ID.
 */
typedef enum phy_calmgr_trigger_id {
	PHY_CALMGR_TRIGGER_INITIAL = 1,
	PHY_CALMGR_TRIGGER_PERIOD = 2,
	PHY_CALMGR_TRIGGER_TEMP_SENSE = 3
} phy_calmgr_trigger_id_t;

/* Trigger callback fn prototype in calibration trigger registry. */
typedef void phy_calmgr_trigger_ctx_t;

typedef bool (*phy_calmgr_trigger_fn_t)(phy_calmgr_trigger_ctx_t *ctx);

/* Add a calibration trigger fn in calibration trigger registry. Returns BCME_XXXX. */
int phy_calmgr_add_trigger_fn(phy_calmgr_info_t *ci,
	phy_calmgr_trigger_fn_t fn, phy_calmgr_trigger_ctx_t *ctx,
	phy_calmgr_trigger_id_t tid);

/*
 * Add a calibration function in calibration registry.
 */

/* Calibration mode */
typedef enum phy_calmgr_cal_mode {
	PHY_CALMGR_CAL_MODE_SSHOT = 1,	/* Perform cal all in once. */
	PHY_CALMGR_CAL_MODE_MPHASE = 2	/* Perform cal one phase at a time. */
} phy_calmgr_cal_mode_t;

/* Calibration callback return status. */
typedef enum phy_calmgr_cal_status {
	PHY_CALMGR_CAL_ST_PHASE_DONE = 1,	/* more phase(s) in this module are expected */
	PHY_CALMGR_CAL_ST_CAL_DONE = 2		/* calibration for this module is done */
} phy_calmgr_cal_status_t;

/* Calibration module ID as well as calibration callback execution order.
 */
/* !Don't manually assign any value! */
typedef enum phy_calmgr_cal_id {
	PHY_CALMGR_CAL_MOD_START = 0,
	PHY_CALMGR_CAL_MOD_TXIQLO
} phy_calmgr_cal_id_t;

/* Calibration callback fn prototype in calibration registry. */
typedef void phy_calmgr_cal_ctx_t;

typedef phy_calmgr_cal_status_t (*phy_calmgr_cal_fn_t)(phy_calmgr_cal_ctx_t *ctx,
	phy_calmgr_cal_mode_t mode);
typedef void (*phy_calmgr_reset_fn_t)(phy_calmgr_cal_ctx_t *ctx);

/* Add a calibration callback entry. Returns BCME_XXXX. 'reset' is optional.
 *
 * Pass the cache entry id returned from phy_cache_reserve_cubby() call to
 * enable saving the calibration or the phase result to the corresponding
 * cache entry cubby; pass PHY_CACHE_ENTRY_INV_ID to bypass the cache entry
 * cubby save operation.
 */
int phy_calmgr_add_cal_fn(phy_calmgr_info_t *ci,
	phy_calmgr_cal_fn_t cal, phy_calmgr_reset_fn_t reset, phy_calmgr_cal_ctx_t *ctx,
        phy_calmgr_cal_id_t cmid, phy_cache_cubby_id_t ccid);

/* ******** interface for other modules ******** */

/* Request a calibration */
typedef struct phy_calmgr_cal_req {
	phy_calmgr_cal_mode_t mode;	/* calibration mode */
	uint8 cache;			/* save results to cache? */
} phy_calmgr_cal_req_t;
int phy_calmgr_req_cal(phy_calmgr_info_t *ci, phy_calmgr_cal_req_t *req);

/* Configure calibration */
typedef struct phy_calmgr_cal_cfg {
	phy_calmgr_cal_mode_t init_mode;	/* initial calibration mode */
	uint32 cal_prd;				/* calibration interval */
} phy_calmgr_cal_cfg_t;
int phy_calmgr_config_cal(phy_calmgr_info_t *ci, phy_calmgr_cal_cfg_t *cfg);

/* Query calibration */
typedef struct phy_calmgr_cal_info {
	uint8 in_prog;			/* cal is progressing? */
	phy_calmgr_cal_id_t cur_cmid;	/* cal module id */
	int last_err;			/* last error */
	phy_calmgr_cal_id_t last_cmid;	/* last cal module id */
} phy_calmgr_cal_info_t;
void phy_calmgr_query_cal(phy_calmgr_info_t *ci, phy_calmgr_cal_info_t *st);

#endif /* _phy_calmgr_h_ */
