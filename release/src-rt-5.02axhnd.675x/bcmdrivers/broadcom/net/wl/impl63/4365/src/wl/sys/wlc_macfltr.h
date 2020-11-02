/*
 * mac filter module header file
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
 * $Id: wlc_macfltr.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_macflter_h_
#define _wlc_macflter_h_

/* This module provides association control:
 * - for AP to decide if an association request is granted
 * - for STA to decide if a join target is considered
 * - for others to control the peer list
 * - ...
 */

#include <wlc_types.h>
#include <wlioctl.h>

/* module entries */
/* attach/detach */
extern wlc_macfltr_info_t *wlc_macfltr_attach(wlc_info_t *wlc);
extern void wlc_macfltr_detach(wlc_macfltr_info_t *mfi);

/* APIs */
/* check if the 'addr' is in the allow/deny list by the return code defined below */
extern int wlc_macfltr_addr_match(wlc_macfltr_info_t *mfi, wlc_bsscfg_t *cfg,
	const struct ether_addr *addr);

/* address match return code */
/* mac filter mode is DISABLE */
#define WLC_MACFLTR_DISABLED		0
/* mac filter mode is DENY */
#define WLC_MACFLTR_ADDR_DENY		1	/* addr is in mac list */
#define WLC_MACFLTR_ADDR_NOT_DENY	2	/* addr is not in mac list */
/* mac filter mode is ALLOW */
#define WLC_MACFLTR_ADDR_ALLOW		3	/* addr is in mac list */
#define WLC_MACFLTR_ADDR_NOT_ALLOW	4	/* addr is not in mac list */

/* set/get mac allow/deny list based on mode */
extern int wlc_macfltr_list_set(wlc_macfltr_info_t *mfi, wlc_bsscfg_t *cfg,
	struct maclist *maclist, uint len);
extern int wlc_macfltr_list_get(wlc_macfltr_info_t *mfi, wlc_bsscfg_t *cfg,
	struct maclist *maclist, uint len);
#ifdef ACKSUPR_MAC_FILTER
extern void wlc_macfltr_addrmatch_move(wlc_info_t *wlc);
extern int wlc_macfltr_find_and_add_addrmatch(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct ether_addr *addr, uint16 attr);
extern int wlc_macfltr_find_and_clear_addrmatch(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	struct ether_addr *addr, uint16 attr);
extern bool wlc_macfltr_acksupr_is_duplicate(wlc_info_t *wlc, struct ether_addr *ea);
#endif /* ACKSUPR_MAC_FILTER */
/* set/get mac list mode */
#define MFIWLC(mfi) (*(wlc_info_t **)mfi)	/* expect wlc to be the first field */
#define wlc_macfltr_mode_set(mfi, cfg, mode) \
	(wlc_ioctl(MFIWLC(mfi), WLC_SET_MACMODE, &(mode), sizeof(mode), (cfg)->wlcif))
#define wlc_macfltr_mode_get(mfi, cfg, mode) \
	(wlc_ioctl(MFIWLC(mfi), WLC_GET_MACMODE, mode, sizeof(*(mode)), (cfg)->wlcif))

#endif /* _wlc_macflter_h_ */
