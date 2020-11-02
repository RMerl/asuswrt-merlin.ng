/*
 * INIT/DOWN control module internal interface (to other modules)
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

#ifndef _phy_init_h_
#define _phy_init_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_init_info phy_init_info_t;

/* ******** interface for Core module ******** */

/* attach/detach */
phy_init_info_t *phy_init_attach(phy_info_t *pi);
void phy_init_detach(phy_init_info_t *);

/* invoke init callbacks */
int phy_init_invoke_init_fns(phy_init_info_t *ii);
#ifndef BCMNODOWN
/* invoke down callbacks */
void phy_init_invoke_down_fns(phy_init_info_t *ii);
#endif // endif

/* ******** interface for other modules ******** */

/* Add a callback fn in the INIT/DOWN sequence.
 *
 * The callback fn returns BCME_XXXX.
 * The INIT sequence will be aborted when a callback returns an error.
 * The DOWN sequence will ignore callback returns.
 */
typedef void phy_init_ctx_t;
typedef int (*phy_init_fn_t)(phy_init_ctx_t *ctx);
/*
 * INIT/DOWN callbacks execution order.
 * Note: Keep the enums between 0 and 255!
 */
typedef enum phy_init_order {
	/* Insert new INIT callbacks at appropriate place. */
	PHY_INIT_START = 0,
	PHY_INIT_CACHE,		/* CACHE (s/w) */
	PHY_INIT_RSSI,		/* RSSICompute */
	PHY_INIT_ANA,		/* ANAcore */
	PHY_INIT_CHBW,		/* CHannelBandWidth */
	PHY_INIT_PHYIMPL,	/* PHYIMPLementation */
	PHY_INIT_NOISERST,	/* NOISEReSeT */
	PHY_INIT_RADIO,		/* RADIO */
	PHY_INIT_PHYTBL,	/* phyTaBLes */
	PHY_INIT_TPC,		/* TxPowerControl */
	PHY_INIT_RADAR,		/* RADARdetection */
	PHY_INIT_ANTDIV,	/* ANTennaDIVersity */
	PHY_INIT_NOISE,		/* NOISE */
	PHY_INIT_CHSPEC,	/* CHannelSPEC */
	PHY_INIT_TXIQLOCAL, /* Tx IQLO Cal */
	PHY_INIT_RXIQCAL,	/* Rx IQ Cal */
	PHY_INIT_PAPDCAL,	/* PAPD IQ Cal */
	PHY_INIT_VCOCAL,	/* VCO IQ Cal */

	/* Insert new DOWN callbacks at appropriate place. */
	PHY_DOWN_START = 0,
	PHY_DOWN_PHYTBL
} phy_init_order_t;

/* Add an init callback entry. Returns BCME_XXXX. */
int phy_init_add_init_fn(phy_init_info_t *ii,
	phy_init_fn_t fn, phy_init_ctx_t *ctx,
	phy_init_order_t order);
#ifndef BCMNODOWN
/* Add a down callback entry. Returns BCME_XXXX. */
int phy_init_add_down_fn(phy_init_info_t *ii,
	phy_init_fn_t fn, phy_init_ctx_t *ctx,
	phy_init_order_t order);
#endif // endif

#endif /* _phy_init_h_ */
