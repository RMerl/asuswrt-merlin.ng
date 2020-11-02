/*
 * Dynamic WDS module header file
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
 * $Id: wlc_wds.h 765913 2018-07-19 13:14:24Z $
*/

#ifndef _wlc_wds_h_
#define _wlc_wds_h_

/* flags for wlc_wds_create() */
#define WDS_INFRA_BSS	0x1	/* WDS link is part of the infra mode BSS */
#define WDS_DYNAMIC	0x2	/* WDS link is dynamic */

/* APIs */
#ifdef WDS
/* module */
extern wlc_wds_info_t *wlc_wds_attach(wlc_info_t *wlc);
extern void wlc_wds_detach(wlc_wds_info_t *wds);
extern void wlc_ap_wds_probe_complete(wlc_info_t *wlc, uint txstatus, struct scb *scb);
extern int wlc_wds_create(wlc_info_t *wlc, struct scb *scb, uint flags);
extern void wlc_scb_wds_free(struct wlc_info *wlc);
extern bool wlc_wds_lazywds_is_enable(wlc_wds_info_t *mwds);
extern int wlc_wds_create_link_event(wlc_info_t *wlc, struct scb *scb, bool isup);
extern bool wlc_wds_is_active(wlc_info_t *wlc);
#ifdef WLCSA
extern void wlc_wds_process_csa(wlc_info_t *wlc, wlc_bsscfg_t *cfg, wl_chan_switch_t *csa);
#endif /* WLCSA */
#ifdef DPSTA
#if defined(STA) && defined(DWDS)
extern struct scb *wlc_dwds_client_is_ds_sta(wlc_info_t *wlc, struct ether_addr *mac);
extern bool wlc_dwds_is_ds_sta(wlc_info_t *wlc, struct ether_addr *mac);
extern bool wlc_dwds_authorized(wlc_bsscfg_t *cfg);
#endif /* STA && DWDS */
#endif /* DPSTA */
#ifdef DWDS
extern dwds_sa_t *wlc_dwds_findsa(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *ea);
extern dwds_sa_t *wlc_dwds_addsa(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *ea);
extern void wlc_dwds_expire_sa(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_dwds_flush_salist(wlc_info_t *wlc, wlc_bsscfg_t *cfg);
extern void wlc_dwds_dump_sa_list(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#endif /* DWDS */
#else /* !WDS */

#define wlc_wds_attach(wlc) NULL
#define wlc_wds_detach(mwds) do {} while (0)
#define wlc_ap_wds_probe_complete(a, b, c) 0
#define wlc_wds_create(a, b, c)	0
#define wlc_scb_wds_free(a) do {} while (0)
#define wlc_wds_lazywds_is_enable(a) 0
#define wlc_wds_create_link_event(a, b, c) do {} while (0)
#define wlc_wds_process_csa(a, b, c) do {} while (0)
#define wlc_dwds_findsa(a, b, c)	NULL
#define wlc_dwds_addsa(a, b, c)		NULL
#define wlc_dwds_expire_sa(a, b) do {}	while (0)
#define wlc_dwds_flush_salist(a, b) do {} while (0)
#define wlc_dwds_dump_sa_list(a, b, c)	do {} while (0)
#endif /* !WDS */

#endif /* _wlc_wds_h_ */
