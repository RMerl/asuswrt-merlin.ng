/*
 * RadarDetect module internal interface (to PHY specific implementation).
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

#ifndef _phy_type_radar_h_
#define _phy_type_radar_h_

#include <typedefs.h>
#include <phy_radar_api.h>
#include <phy_radar.h>
#include "phy_radar_st.h"

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_radar_ctx_t;

typedef int (*phy_type_radar_init_fn_t)(phy_type_radar_ctx_t *ctx, bool on);
typedef void (*phy_type_radar_update_fn_t)(phy_type_radar_ctx_t *ctx);
typedef void (*phy_type_radar_mode_fn_t)(phy_type_radar_ctx_t *ctx, phy_radar_detect_mode_t mode);
typedef int (*phy_type_radar_run_fn_t)(phy_type_radar_ctx_t *ctx, int PLL_idx, int BW80_80_mode);
typedef int (*phy_type_radar_dump_fn_t)(phy_type_radar_ctx_t *ctx, struct bcmstrbuf *b);
typedef struct {
	phy_type_radar_init_fn_t init;
	phy_type_radar_update_fn_t update;
	phy_type_radar_mode_fn_t mode;
	phy_type_radar_run_fn_t run;
	phy_type_radar_ctx_t *ctx;
} phy_type_radar_fns_t;

/*
 * Register/unregister PHY type implementation to the RadarDetect module.
 * It returns BCME_XXXX.
 */
int phy_radar_register_impl(phy_radar_info_t *ri, phy_type_radar_fns_t *fns);
void phy_radar_unregister_impl(phy_radar_info_t *ri);

#endif /* _phy_type_radar_h_ */
