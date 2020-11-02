/*
 * ARP Offload interface
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
 *   $Id: wl_arpoe.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wl_arpoe_h_
#define _wl_arpoe_h_

/* Forward declaration */
typedef struct wl_arp_info wl_arp_info_t;

/* Return values */
#define ARP_REPLY_PEER 		0x1	/* Reply was sent to service ARP request from peer */
#define ARP_REPLY_HOST		0x2	/* Reply was sent to service ARP request from host */
#define ARP_REQ_SINK		0x4	/* Input packet should be discarded */
#define ARP_FORCE_FORWARD       0X5     /* ARP req should be forwarded to host,
					 * bypassing pktfilter
					 */

#ifdef ARPOE

#define NON_ARP				-1 /* received packet is not ARP packet */
#define TRUNCATED_ARP		-2 /* received packet is truncated ARP packet */

/*
 * Initialize ARP private context.
 * Returns a pointer to the ARP private context, NULL on failure.
 */
extern wl_arp_info_t *wl_arp_attach(wlc_info_t *wlc);

/* Cleanup ARP private context */
extern void wl_arp_detach(wl_arp_info_t *arpi);

/* Process frames in transmit direction */
extern bool wl_arp_send_pktfetch_required(wl_arp_info_t *arpi, void *sdu);
extern int wl_arp_send_proc(wl_arp_info_t *arpi, void *sdu);

/* Process frames in receive direction */
extern int wl_arp_recv_proc(wl_arp_info_t *arpi, void *sdu);

/* called when a new virtual IF is created.
 *	i/p: primary ARPIIF [arpi_p] and the new wlcif,
 *	o/p: new arpi structure populated with inputs and
 *		the global parameters duplicated from arpi_p
 *	side-effects: arpi for a new IF will inherit properties of arpi_p till
 *		the point new arpi is created. After that, for any change in
 *		arpi_p will NOT change the arpi corr to new IF. To change property
 *		of new IF, wl -i wl0.x has to be used.
*/
extern wl_arp_info_t *wl_arp_alloc_ifarpi(wl_arp_info_t *arpi_p,
	wlc_if_t *wlcif);
extern void wl_arp_clone_arpi(wl_arp_info_t *from_arpi, wl_arp_info_t *to_arpi);

extern void wl_arp_free_ifarpi(wl_arp_info_t *arpi);
#ifdef BCM_OL_DEV
extern void wl_arp_update_stats(wl_arp_info_t *arpi, bool suppressed);
extern void
wl_arp_proc_msg(wlc_dngl_ol_info_t * wlc_dngl_ol, wl_arp_info_t *arpi, void *buf);
#endif // endif
#else	/* stubs */

#define wl_arp_attach(a)		(wl_arp_info_t *)0x0dadbeef
#define	wl_arp_detach(a)		do {} while (0)
#define wl_arp_send_pktfetch_required(a, b)		(0)
#define wl_arp_send_proc(a, b)		(-1)
#define wl_arp_recv_proc(a, b)		(-1)
#define wl_arp_alloc_ifarpi(a, b)	(0)
#define wl_arp_free_ifarpi(a)		do {} while (0)
#define wl_arp_clone_arpi(a, b)     do {} while (0)
#endif /* ARPOE */

#endif	/* _wl_arpoe_h_ */
