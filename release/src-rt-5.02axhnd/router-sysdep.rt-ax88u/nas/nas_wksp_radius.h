/*
 * Radius support for NAS workspace
 * Copyright 2019 Broadcom
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
 * $Id: nas_wksp_radius.h 241182 2011-02-17 21:50:03Z $:
 */

#ifndef _NAS_WKSP_RADIUS_H_
#define _NAS_WKSP_RADIUS_H_

#ifdef NAS_RADIUS
/* open connection to receive radius messages */
extern int nas_wksp_open_radius(nas_wksp_t *nwksp);
extern void nas_wksp_close_radius(nas_wksp_t *nwksp);

extern int nas_radius_open(nas_wksp_t *nwksp, nas_wpa_cb_t *nwcb);
extern void nas_radius_close(nas_wksp_t *nwksp, nas_wpa_cb_t *nwcb);

extern int nas_radius_send_packet(nas_t *nas, radius_header_t *radius, int length);

#define NAS_WKSP_OPEN_RADIUS(wksp)  nas_wksp_open_radius(wksp)
#define NAS_WKSP_CLOSE_RADIUS(wksp) nas_wksp_close_radius(wksp)

#define NAS_RADIUS_OPEN(nwksp, nwcb) nas_radius_open(nwksp, nwcb)
#define NAS_RADIUS_CLOSE(nwksp, nwcb) nas_radius_close(nwksp, nwcb)

#define NAS_RADIUS_SEND_PACKET(nas, radius, length)	nas_radius_send_packet(nas, radius, length)
#else
#define NAS_WKSP_OPEN_RADIUS(wksp) (-1)
#define NAS_WKSP_CLOSE_RADIUS(wksp)

#define NAS_RADIUS_OPEN(nwksp, nwcb) (-1)
#define NAS_RADIUS_CLOSE(nwksp, nwcb)

#define NAS_RADIUS_SEND_PACKET(nas, radius, length)	(-1)
#endif /* NAS_RADIUS */

#endif /* !defined(_NAS_WKSP_RADIUS_H_) */
