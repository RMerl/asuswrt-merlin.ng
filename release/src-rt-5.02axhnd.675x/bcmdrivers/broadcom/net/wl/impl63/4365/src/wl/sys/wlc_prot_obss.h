/*
 * OBSS Protection support
 * Broadcom 802.11 Networking Device Driver
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

#ifndef _wlc_prot_obss_h_
#define _wlc_prot_obss_h_

/* Function: record secondary rssi histogram
 * three bins [hi, med, low] with
 * hi  : counting sec_rssi >= M_SECRSSI0_MIN (hi_thresh)
 * med : counting sec_rssi in [ M_SECRSSI1_MIN, M_SECRSSI0_MIN )
 * low : counting sec_rssi <= M_SECRSSI1_MIN (low_thresh)
 */
#define OBSS_SEC_RSSI_LIM0_DEFAULT				-50	/* in dBm */
#define OBSS_SEC_RSSI_LIM1_DEFAULT				-70	/* in dBm */
#define OBSS_INACTIVITY_PERIOD_DEFAULT				30	/* in seconds */
#define OBSS_DUR_THRESHOLD_DEFAULT				30	/* OBSS
* protection trigger for RX CRS Sec
*/

struct wlc_prot_obss_info {
	bool protection;	/* TRUE if full phy bw CTS2SELF */
};

wlc_prot_obss_info_t *wlc_prot_obss_attach(wlc_info_t *wlc);
void wlc_prot_obss_detach(wlc_prot_obss_info_t *prot);

#ifdef WL_PROT_OBSS
#define WLC_PROT_OBSS_PROTECTION(prot)	((prot)->protection)
#else
#define WLC_PROT_OBSS_PROTECTION(prot)	(0)
#endif /* WL_PROT_OBSS */

#endif /* _wlc_prot_obss_h_ */
