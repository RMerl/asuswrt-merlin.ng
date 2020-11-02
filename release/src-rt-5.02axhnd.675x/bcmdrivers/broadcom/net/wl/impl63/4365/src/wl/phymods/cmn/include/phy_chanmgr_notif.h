/*
 * Channel Manager Notification internal interface (to other PHY modules).
 *
 * This is chanspec event notification client management interface.
 *
 * It provides interface for clients to register notification callbacks.
 * It invokes registered callbacks synchronously when interested event
 * is raised.
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

#ifndef _phy_chanmgr_notif_h_
#define _phy_chanmgr_notif_h_

#include <typedefs.h>
#include <bcmwifi_channels.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_chanmgr_notif_info phy_chanmgr_notif_info_t;

/* attach/detach */
phy_chanmgr_notif_info_t *phy_chanmgr_notif_attach(phy_info_t *pi);
void phy_chanmgr_notif_detach(phy_chanmgr_notif_info_t *cni);

/* register client/interest */
typedef void phy_chanmgr_notif_ctx_t;
typedef struct {
	uint16 event;		/* event (single) */
	chanspec_t new;		/* the new chanspec */
	chanspec_t old;		/* the old chanspec (for channel change related events) */
} phy_chanmgr_notif_data_t;
typedef int (*phy_chanmgr_notif_fn_t)(phy_chanmgr_notif_ctx_t *ctx,
	phy_chanmgr_notif_data_t *data);

/* event - maximum 16 events */
#define PHY_CHANMGR_NOTIF_OPCHCTX_OPEN	(1<<0)	/* operating chanspec context creation */
#define PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE	(1<<1)	/* operating chanspec context delete */
#define PHY_CHANMGR_NOTIF_OPCH_CHG	(1<<2)	/* operating chanspec change */
#define PHY_CHANMGR_NOTIF_CH_CHG		(1<<3)	/* non-operating chanspec change */

/* Callback order.
 *
 * All callbacks are iovoked when a given event is raised. Callback registered
 * with smaller numeric number is invoked first.
 *
 * It is used in attach process only so no need to assign values.
 * Maximum 256 orders.
 */
typedef enum phy_chanmgr_notif_order {
	PHY_CHANMGR_NOTIF_ORDER_START = 0,
	/* the next three clients must remain this order */
	PHY_CHANMGR_NOTIF_ORDER_CACHE,
	PHY_CHANMGR_NOTIF_ORDER_CALMGR,
	PHY_CHANMGR_NOTIF_ORDER_WD
	/* add new client here */
} phy_chanmgr_notif_order_t;

/* 'events' is a mask of the supported events */
int phy_chanmgr_notif_add_interest(phy_chanmgr_notif_info_t *cni,
	phy_chanmgr_notif_fn_t fn, phy_chanmgr_notif_ctx_t *ctx,
	phy_chanmgr_notif_order_t order, uint16 events);

#endif /* _phy_chanmgr_notif_h_ */
