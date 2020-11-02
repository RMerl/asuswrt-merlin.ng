/*
 * PHY Core module internal interface - connect PHY type specific layer to common layer.
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

#ifndef _phy_type_disp_h_
#define _phy_type_disp_h_

#include <typedefs.h>
#include <phy_api.h>
#include "phy_type.h"

#include <wlc_iocv_types.h>

/* forward declaration */
typedef struct phy_type_disp phy_type_disp_t;

/* Attach/detach PHY type specific implementation dispatch info */
phy_type_disp_t *phy_type_disp_attach(phy_info_t *pi);
void phy_type_disp_detach(phy_type_disp_t *disp);

/*
 * Attach/detach PHY type specific implementation.
 *
 * Call phy_type_attach() after all PHY modules are attached to the system.
 * Call phy_type_detach() before any PHY module detaches from the system.
 */
phy_type_info_t *phy_type_attach(phy_type_disp_t *disp, int bandtype);
void phy_type_detach(phy_type_disp_t *disp, phy_type_info_t *ti);

/*
 * Register/unregister PHY type specific implementations to/from PHY modules.
 *
 * Return BCME_OK when all registrations are successfully done; BCME_XXXX otherwise.
 *
 * Call phy_type_register_impl() after all PHY modules are attached to the system.
 * Call phy_type_unregister_impl() before any PHY module detaches from the system.
 */
int phy_type_register_impl(phy_type_disp_t *disp, int bandtype);
void phy_type_unregister_impl(phy_type_disp_t *disp);

/*
 * Reset h/w and/or s/w states upon attach
 */
void phy_type_reset_impl(phy_type_disp_t *disp);

/*
 * Init h/w and/or s/w states upon init (band switch)
 */
int phy_type_init_impl(phy_type_disp_t *disp);

/*
 * Register PHY type specific implementation iovar tables/handlers.
 *
 * Call phy_type_register_iovt() after all PHY modules are attached to the system.
 */
int phy_type_register_iovt(phy_type_disp_t *disp, wlc_iocv_info_t *ii);

/*
 * Register PHY type specific implementation ioctl tables/handlers.
 *
 * Call phy_type_register_ioct() after all PHY modules are attached to the system.
 */
int phy_type_register_ioct(phy_type_disp_t *disp, wlc_iocv_info_t *ii);

#if ((defined(BCMDBG) || defined(BCMDBG_DUMP)) && defined(DBG_PHY_IOV)) || \
	defined(BCMDBG_PHYDUMP)
/* dump phy type specific phy registers */
uint16 phy_type_read_phyreg(phy_type_disp_t *disp, uint addr);
int phy_type_dump_phyregs(phy_type_disp_t *disp, struct bcmstrbuf *b);
#endif // endif

#endif /* _phy_type_disp_h_ */
