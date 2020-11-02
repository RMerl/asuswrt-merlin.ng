/*
 * PHY Core module public interface (to MAC driver).
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

#ifndef _phy_api_h_
#define _phy_api_h_

#include <typedefs.h>
#include <bcmwifi_channels.h>

#ifdef ALL_NEW_PHY_MOD
typedef struct phy_shared_info phy_shared_info_t;
typedef struct phy_info phy_info_t;
#else
#include <wlc_phy_types.h>
#include <wlc_phy_hal.h>
#endif // endif

#include <wlc_iocv_types.h>

/*
 * Attach/detach all PHY modules to/from the system.
 */
phy_info_t *phy_module_attach(shared_phy_t *sh, void *regs, int bandtype, char *vars);
void phy_module_detach(phy_info_t *pi);

/*
 * Register all iovar/ioctl tables/handlers to/from the system.
 */
int phy_register_iovt_all(phy_info_t *pi, wlc_iocv_info_t *ii);
int phy_register_ioct_all(phy_info_t *pi, wlc_iocv_info_t *ii);

/*
 * TODO: These functions should be registered to bmac in phy_module_attach(),
 * which requires bmac to have some registration infrastructure...
 */

/*
 * Init/deinit the PHY h/w.
 */
/* band specific init */
void phy_bsinit(phy_info_t *pi, chanspec_t chanspec, bool forced);
/* band width init */
void phy_bwinit(phy_info_t *pi, chanspec_t chanspec);
/* generic init */
void phy_init(phy_info_t *pi, chanspec_t chanspec);
/* generic deinit */
int phy_down(phy_info_t *pi);

/* Publish phyAPI's here.. */
#define PHY_RSBD_PI_IDX_CORE0 0
#define PHY_RSBD_PI_IDX_CORE1 1

void phy_set_phymode(phy_info_t *pi, uint16 new_phymode);
uint16 phy_get_phymode(const phy_info_t *pi);
phy_info_t *phy_get_pi(const phy_info_t *pi, int idx);
bool phy_init_pending(phy_info_t *pi);
mbool phy_get_measure_hold_status(phy_info_t *pi);
void phy_set_measure_hold_status(phy_info_t *pi, mbool set);
#endif /* _phy_api_h_ */
