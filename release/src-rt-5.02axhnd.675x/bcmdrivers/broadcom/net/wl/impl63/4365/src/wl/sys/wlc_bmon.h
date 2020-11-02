/**
 * Ucode monitored BSSID feature driver support API
 *
 * Ucode compares the incoming packet's a3 (if any) with the configured
 * monitored BSSID(s) (currently only one) and passes the packet up to
 * the driver if the a3 matches one of the BSSID(s).
 *
 * This driver code adds support for users to configure the BSSID(s) and
 * to register the callbacks to be invoked when a matching packet comes.
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
 * $Id: wlc_bmon.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_bmon_h_
#define _wlc_bmon_h_

/* _bmon is enabled whenever there is entry in the match table */
#ifdef WLBMON
#define BMON_ENAB(wlc)	((wlc)->bmon != NULL && (wlc)->pub->_bmon)
#else
#define BMON_ENAB(wlc)	FALSE
#endif // endif

/* match entry user */
#define BMON_USER_NIC	0x1
#define BMON_USER_WLU	0x2

/* callback info */
typedef struct {
	int me;		/* match entry index returned from wlc_bmon_pktrx_match() */
	wlc_d11rxhdr_t *wrxh;
	uint8 *plcp;
	void *pkt;
} wlc_bmon_pktrx_data_t;
typedef void (*wlc_bmon_pktrx_fn_t)(void *arg, const wlc_bmon_pktrx_data_t *notif_data);

/* registration info */
typedef struct {
	uint user;
	struct ether_addr *bssid;
	wlc_bmon_pktrx_fn_t fn;
	void *arg;
} wlc_bmon_reg_info_t;

#ifdef WLBMON

/* module entries */
extern wlc_bmon_info_t *wlc_bmon_attach(wlc_info_t *wlc);
extern void wlc_bmon_detach(wlc_bmon_info_t *bmi);

/* Add/del address into the match table. */
extern int wlc_bmon_bssid_add(wlc_bmon_info_t *bmi, wlc_bmon_reg_info_t *reg);
extern int wlc_bmon_bssid_del(wlc_bmon_info_t *bmi, wlc_bmon_reg_info_t *reg);

/* determine if the given packet represented by the 'hdr' parameter
 * matches one of our entries and return the entry index if it does
 * or return -1 otherwise.
 */
extern int wlc_bmon_pktrx_match(wlc_bmon_info_t *bmi, struct dot11_header *hdr);

/* Notify interested parties of the given matching packet 'pkt'.
 * 'pkt' is shared among all parties and must not be freed nor be modified,
 * it points to the dot11 header, it has no pkttag info (not initialized)...
 */
extern void wlc_bmon_pktrx_notif(wlc_bmon_info_t *bmi, wlc_bmon_pktrx_data_t *notif_data);

#else /* WLBMON */

#define wlc_bmon_bssid_add(bmi, reg) ((void)(reg), BCME_OK)
#define wlc_bmon_bssid_del(bmi, reg) ((void)(reg), BCME_OK)

#define wlc_bmon_pktrx_match(bmi, hdr) ((void)(hdr), -1)

#define wlc_bmon_pktrx_notif(bmi, notif_data) do {(void)(notif_data);} while (0)

#endif /* !WLBMON */

#endif /* _wlc_bmon_h_ */
