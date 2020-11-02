/*
 * RWL module  of
 * Broadcom 802.11bang Networking Device Driver
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
 * $Id: wlc_rwl.h 708017 2017-06-29 14:11:45Z $
 *
 */

#ifndef _wlc_rwl_h_
#define _wlc_rwl_h_

#if defined(RWL_WIFI) || defined(WIFI_REFLECTOR)

#include <rwl_wifi.h>

typedef struct rwl_info {
	wlc_info_t	*wlc;
	wlc_pub_t	*pub;
	rwl_request_t *rwl_first_action_node;
	rwl_request_t *rwl_last_action_node;
	struct ether_addr rwl_ea;
} rwl_info_t;

extern rwl_info_t* wlc_rwl_attach(wlc_info_t *wlc);
extern int wlc_rwl_detach(rwl_info_t *rwlh);
extern void wlc_rwl_init(rwl_info_t *rwlh);
extern void wlc_rwl_deinit(rwl_info_t *rwlh);
extern void wlc_rwl_up(wlc_info_t *wlc);
extern uint wlc_rwl_down(wlc_info_t *wlc);
extern void wlc_rwl_frameaction(rwl_info_t *rwlh, struct dot11_management_header *hdr,
                                uint8 *body, int body_len);
extern void wlc_recv_wifi_mgmtact(rwl_info_t *rwlh, uint8 *body, const struct ether_addr * sa);

#else /* !defined(RWL_WIFI) && !defined(WIFI_REFLECTOR) */

typedef struct rwl_info {
	wlc_info_t	*wlc;
	wlc_pub_t	*pub;
} rwl_info_t;

#define wlc_rwl_attach(a)		(rwl_info_t *)0xdeadbeef
#define wlc_rwl_detach(a)		0
#define wlc_rwl_init(a)			do {} while (0)
#define wlc_rwl_deinit(a)		do {} while (0)
#define wlc_rwl_up(a)			do {} while (0)
#define wlc_rwl_down(a)			0
#define wlc_rwl_frameaction(a)		do {} while (0)
#define wlc_recv_wifi_mgmtact(a, b, c)	do {} while (0)

#endif /* !defined(RWL_WIFI) && !defined(WIFI_REFLECTOR) */

#endif	/* _wlc_rwl_h_ */
