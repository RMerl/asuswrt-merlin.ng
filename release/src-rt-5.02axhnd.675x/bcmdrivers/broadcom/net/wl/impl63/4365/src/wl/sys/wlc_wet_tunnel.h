/*
 * Wireless Ethernet (WET) tunnel
 *
 *   Copyright 2020 Broadcom
 *
 *   This program is the proprietary software of Broadcom and/or
 *   its licensors, and may only be used, duplicated, modified or distributed
 *   pursuant to the terms and conditions of a separate, written license
 *   agreement executed between you and Broadcom (an "Authorized License").
 *   Except as set forth in an Authorized License, Broadcom grants no license
 *   (express or implied), right to use, or waiver of any kind with respect to
 *   the Software, and Broadcom expressly reserves all rights in and to the
 *   Software and all intellectual property rights therein.  IF YOU HAVE NO
 *   AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *   WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *   THE SOFTWARE.
 *
 *   Except as expressly set forth in the Authorized License,
 *
 *   1. This program, including its structure, sequence and organization,
 *   constitutes the valuable trade secrets of Broadcom, and you shall use
 *   all reasonable efforts to protect the confidentiality thereof, and to
 *   use this information only in connection with your use of Broadcom
 *   integrated circuit products.
 *
 *   2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *   REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *   OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *   DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *   NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *   ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *   CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *   OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *   3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *   BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *   SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *   IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *   IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *   ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *   OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *   NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *   $Id: wlc_wet_tunnel.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_wet_tunnel_h_
#define _wlc_wet_tunnel_h_

#ifdef WET_TUNNEL
#define WET_TUNNEL_ENAB(pub)	((pub)->wet_tunnel)
#define SCB_WET_TUNNEL(a)		((a) && ((a)->flags_brcm_syscap & SCBBS_WET_TUNNEL))
#else
#define WET_TUNNEL_ENAB(pub)		FALSE
#define SCB_WET_TUNNEL(a)	FALSE
#endif // endif

/* forward declaration */
typedef struct wlc_wet_tunnel_info wlc_wet_tunnel_info_t;

/*
 * Initialize wet tunnel private context.It returns a pointer to the
 * wet tunnel private context if succeeded. Otherwise it returns NULL.
 */
extern wlc_wet_tunnel_info_t *wlc_wet_tunnel_attach(wlc_info_t *wlc);

/* Cleanup wet tunnel private context */
extern void wlc_wet_tunnel_detach(wlc_wet_tunnel_info_t *weth);

/* Process frames in transmit direction */
extern int wlc_wet_tunnel_send_proc(wlc_wet_tunnel_info_t *weth, void *sdu);

/* Process frames in receive direction */
extern int wlc_wet_tunnel_recv_proc(wlc_wet_tunnel_info_t *weth, void *sdu);

/* Process multicast frames in receive direction */
extern int wlc_wet_tunnel_multi_packet_forward(wlc_info_t *wlc, osl_t *osh,
	struct scb *scb, struct wlc_if *wlcif, void *sdu);

#ifdef BCMDBG
extern int wlc_wet_tunnel_dump(wlc_wet_tunnel_info_t *weth, struct bcmstrbuf *b);
#endif /* BCMDBG */

#endif	/* _wlc_wet_tunnel_h_ */
