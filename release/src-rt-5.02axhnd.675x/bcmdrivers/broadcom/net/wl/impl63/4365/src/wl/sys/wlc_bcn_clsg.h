/**
 * Beacon Coalescing related source file
 * Broadcom 802.11abg Networking Device Driver
 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BeaconOffload
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
 * $Id: wlc_bcn_clsg.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_bcn_clsg_h_
#define _wlc_bcn_clsg_h_

#ifdef WL_BCN_COALESCING
extern wlc_bcn_clsg_info_t *wlc_bcn_clsg_attach(wlc_info_t *bc);
extern void wlc_bcn_clsg_detach(wlc_bcn_clsg_info_t *bc);
extern bool wlc_bcn_clsg_in_ucode(wlc_bcn_clsg_info_t *bc, bool time_since_bcn, bool *flushed);
extern void wlc_bcn_clsg_store_rxh(wlc_bcn_clsg_info_t *bc, wlc_d11rxhdr_t *wrxh);
extern void wlc_bcn_clsg_update_rxh(wlc_bcn_clsg_info_t *bc, wlc_d11rxhdr_t *wrxh);
extern uint32 wlc_get_len_from_plcp(wlc_d11rxhdr_t *wrxh, uint8 *plcp);

#define BCN_CLSG_CONFIG_MASK    0x01	/* For disabled by config/user */
#define BCN_CLSG_ASSOC_MASK     0x02	/* For associated station */
#define BCN_CLSG_BSS_MASK       0x08	/* For more than 1 BSS is Active */
#define BCN_CLSG_SCAN_MASK      0x10	/* For SCANNING */
#define BCN_CLSG_UATBTT_MASK    0x20	/* For unaligned TBTT */
#define BCN_CLSG_PRMS_MASK      0x40	/* For promiscous mode */
#define BCN_CLSG_CORE_MASK      0x80	/* For not supported by device core rev */
#define BCN_CLSG_UPDN_MASK      0x100	/* For up/down mode */
#define BCN_CLSG_PM_MASK        0x200	/* Power Save Mode Mask */

extern void wlc_bcn_clsg_disable(wlc_bcn_clsg_info_t *bc, uint32 mask, uint32 val);

#endif /* WL_BCN_COALESCING */

#endif /* _wlc_bcn_clsg_h_ */
