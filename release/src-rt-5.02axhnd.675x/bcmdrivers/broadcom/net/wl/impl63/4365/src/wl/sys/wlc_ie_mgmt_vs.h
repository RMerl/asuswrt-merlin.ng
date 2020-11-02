/*
 * IE management module Vendor Specific IE utilities
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
 * $Id: wlc_ie_mgmt_vs.h 744331 2018-02-01 11:15:53Z $
 */

#ifndef _wlc_ie_mgmt_vs_h_
#define _wlc_ie_mgmt_vs_h_

#include <typedefs.h>
#include <wlc_types.h>

/*
 * Special id (250 - 255).
 */
#define WLC_IEM_VS_IE_ID_UNK		250

/*
 * Priority/id (0 - 249).
 *
 * - It is used as a Priority when registering a Vendor Specific IE's
 *   calc_len/build callback pair. The IE management's IE calc_len/build
 *   function invokes the callbacks in their priorities' ascending order.
 *
 * - It is used as an ID when registering a Vendor Specific IE's parse
 *   callback. The IE management's IE parse function queries the user
 *   supplied classifier callback (which may use the OUI plus some other
 *   information in the IE being parsed to decide the ID), and invokes the
 *   callback.
 */
/* !Please leave some holes in between priorities when possible! */
#define WLC_IEM_VS_IE_PRIO_VNDR		64
#define WLC_IEM_VS_IE_PRIO_BRCM_HT	80
#define WLC_IEM_VS_IE_PRIO_BRCM_EXT_CH	88
#define WLC_IEM_VS_IE_PRIO_BRCM_VHT	104
#define WLC_IEM_VS_IE_PRIO_BRCM_TPC	136
#define WLC_IEM_VS_IE_PRIO_BRCM_RMC	140
#define WLC_IEM_VS_IE_PRIO_BRCM		152
#define WLC_IEM_VS_IE_PRIO_BRCM_PSTA	156
/* XXX: Note, the Feb 11 '03 windows WPA supplicant code requires that the WPA IE follow
 * this element
 */
#define WLC_IEM_VS_IE_PRIO_WPA		160
#define WLC_IEM_VS_IE_PRIO_WPS		164
#define WLC_IEM_VS_IE_PRIO_WME		168
#ifdef BCMCCX
#define WLC_IEM_VS_IE_PRIO_CCX_QOS	170
#define WLC_IEM_VS_IE_PRIO_CCX_IHV	171
#define WLC_IEM_VS_IE_PRIO_CCX_RM_CAP	173
#define WLC_IEM_VS_IE_PRIO_CCX_VER	174
#endif // endif
#define WLC_IEM_VS_IE_PRIO_WME_TS	176
#ifdef BCMCCX
#define WLC_IEM_VS_IE_PRIO_CCX_TS_RS	178
#endif // endif
#ifdef WLFBT
#define WLC_IEM_VS_IE_PRIO_CCX_EXT_CAP  179
#endif // endif
#define WLC_IEM_VS_IE_PRIO_HS20		190
#define WLC_IEM_VS_IE_PRIO_P2P		192
#define WLC_IEM_VS_IE_PRIO_OSEN		196
#define WLC_IEM_VS_IE_PRIO_NAN		197

#define WLC_IEM_VS_IE_PRIO_ULB		204
#define WLC_IEM_VS_IE_PRIO_MULTIAP	205
#define WLC_IEM_VS_IE_PRIO_MBO_OCE	210
/*
 * Map Vendor Specific IE to an id
 */
extern uint8 wlc_iem_vs_get_id(wlc_iem_info_t *iem, uint8 *ie);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern int wlc_iem_vs_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

#endif /* _wlc_ie_mgmt_vs_h_ */
