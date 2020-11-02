/*
 * HND HIGH driver RPC Tx module
 * Broadcom 802.11abgn Networking Device Driver
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
 * $Id: wlc_rpctx.h 570958 2015-07-14 01:53:00Z $
 */
#ifndef _wlc_rpctx_h_
#define _wlc_rpctx_h_

/* forward declaration */
#include <wlc_types.h>

struct wlc_pub;
struct rpc_info;

/* This controls how many packets are given to the dongle. This is required as
 * NTXD needs to be power of 2 but we may not have enough memory to absorb that
 * large number of frames
 */
#ifndef NRPCTXBUFPOST
#define NRPCTXBUFPOST NTXD
#endif // endif

#if defined(WLC_HIGH_ONLY)

struct wlc_rpc_phy {
	struct rpc_info *rpc;
};

#define RPCTX_ENAB(pub)	(TRUE)

extern rpctx_info_t *wlc_rpctx_attach(struct wlc_pub *pub, wlc_info_t *wlc);
extern int wlc_rpctx_fifoinit(rpctx_info_t *rpctx, uint fifo, uint ntxd);
extern void wlc_rpctx_detach(rpctx_info_t *rpctx);
extern int wlc_rpctx_dump(rpctx_info_t *rpctx, struct bcmstrbuf *b);
extern void *wlc_rpctx_getnexttxp(rpctx_info_t *rpctx, uint fifo);
extern void wlc_rpctx_txreclaim(rpctx_info_t *rpctx);
extern uint wlc_rpctx_txavail(rpctx_info_t *rpctx, uint fifo);
extern int wlc_rpctx_pktenq(rpctx_info_t *rpctx, uint fifo, void *p);
extern int wlc_rpctx_tx(rpctx_info_t *rpctx, uint fifo, void *p, bool commit, uint16 frameid,
                        uint8 txpktpend);
extern uint wlc_rpctx_fifo_enabled(rpctx_info_t *rpctx, uint fifo);
extern void wlc_rpctx_map_pkts(rpctx_info_t *rpctx, map_pkts_cb_fn cb, void *ctx);
#else

#define RPCTX_ENAB(pub)                         (FALSE)
#define wlc_rpctx_attach(pub, wlc)              (NULL)
#define wlc_rpctx_fifoinit(rpctx, fifo, ntxd)   (0)
#define wlc_rpctx_detach(rpctx)                 ASSERT(0)
#define wlc_rpctx_txavail(rpctx, f)             (FALSE)
#define wlc_rpctx_dump(rpctx, b)                (0)
#define wlc_rpctx_getnexttxp(rpctx, f)          (NULL)
#define wlc_rpctx_txreclaim(rpctx)              ASSERT(0)
#define wlc_rpctx_pktenq(rpctx, fifo, p)        do { } while (0)
#define wlc_rpctx_tx(rpctx, f, p, c, fid, t)    (0)
#define wlc_rpctx_fifo_enabled(rpctx, f)        (FALSE)

#endif	/* WLC_HIGH_ONLY */

#endif /* _wlc_rpctx_h_ */
