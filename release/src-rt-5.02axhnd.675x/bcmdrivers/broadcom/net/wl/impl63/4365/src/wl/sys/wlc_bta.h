/*
 * BTA (BlueTooth Alternate Mac and Phy module aka BT-AMP)
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
 * $Id: wlc_bta.h 708017 2017-06-29 14:11:45Z $
*/

#ifndef _wlc_bta_h_
#define _wlc_bta_h_

#ifdef WLBTAMP

#define WLC_BTA_RADIO_DISABLE	0
#define WLC_BTA_AP_ASSOC	3
#define WLC_BTA_RADIO_ENABLE	6

extern bta_info_t *wlc_bta_attach(wlc_info_t *wlc);
extern void wlc_bta_detach(bta_info_t *bta);

extern bool wlc_bta_active(bta_info_t *bta);
extern bool wlc_bta_inprog(bta_info_t *bta);

extern void wlc_bta_join_complete(bta_info_t *bta, struct scb *scb, uint8 status);
extern void wlc_bta_AKM_complete(bta_info_t *bta, struct scb *scb);

extern void wlc_bta_assoc_complete(bta_info_t *bta, wlc_bsscfg_t *cfg);

extern bool wlc_bta_recv_proc(bta_info_t *bta, struct wlc_frminfo *f, struct scb *scb);
extern bool wlc_bta_send_proc(bta_info_t *bta, void *p, wlc_if_t **wlcif);

extern void wlc_bta_tx_hcidata(void *handle, uint8 *data, uint len);
extern void wlc_bta_docmd(void *handle, uint8 *cmd, uint len);
extern void wlc_bta_scb_cleanup(bta_info_t *bta, struct scb *scb);
extern void wlc_bta_radio_status_upd(bta_info_t *bta);
extern void wlc_bta_assoc_status_upd(bta_info_t *bta, wlc_bsscfg_t *cfg, uint8 state);
extern bool wlc_bta_frameburst_active(bta_info_t *bta, wlc_pkttag_t *pkttag, uint rate);
#if defined(BCMDBG) || defined(WLMSG_BTA)
extern void wlc_bta_dump_stats(bta_info_t *bta);
#else
#define wlc_bta_dump_stats(a) do {} while (0)
#endif // endif

#else	/* stubs */

#define wlc_bta_attach(a) (bta_info_t *)0x0dadbeef
#define	wlc_bta_detach(a) do {} while (0)

#endif /* WLBTAMP */

#endif /* _wlc_bta_h_ */
