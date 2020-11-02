/*
 * TCP Keep-Alive & ICMP  Offload interface
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
 *   $Id: wl_tcpkoe.h 708017 2017-06-29 14:11:45Z $
 */

/**
 * XXX
 * Used in 'NIC+offload' (Apple) builds. Goal is to increase power saving of the host by waking up
 * the host less. Firmware should:
 *     - respond to ICMP PING requests (up to full MTU packet size)
 *     - send out periodic keep alive packets to keep TCP connections alive
 *
 * Twiki: [OffloadsPhase2]
 */

#ifndef _wl_tcpkoe_h_
#define _wl_tcpkoe_h_

/* Forward declaration */
typedef struct wl_icmp_info wl_icmp_info_t;
typedef struct tcp_keep_info    wl_tcp_keep_info_t;
#define IP_DEFAULT_TTL     32

#ifdef TCPKAOE

extern wl_icmp_info_t *wl_icmp_attach(wlc_info_t *wlc);
extern wl_tcp_keep_info_t *wl_tcp_keep_attach(wlc_info_t *wlc);
extern void wl_tcp_keep_detach(wl_tcp_keep_info_t *tcp_keep_info);

extern void wl_icmp_detach(wl_icmp_info_t *icmpi);
extern int wl_icmp_recv_proc(wl_icmp_info_t *icmpi, void *sdu);
extern int wl_tcpkeep_recv_proc(wl_tcp_keep_info_t *tcp_keep_info, void *sdu);

#ifdef BCM_OL_DEV
extern void
wl_tcp_keepalive_proc_msg(wlc_dngl_ol_info_t * wlc_dngl_ol, wl_tcp_keep_info_t *tcpkeepi,
    void *buf);
#endif /* BCM_OL_DEV */

#else /* TCPKAOE */
#define wl_icmp_attach(a)		(wl_icmp_info_t *)0x0dadbeef
#define wl_icmp_detach(a)		do {} while (0)
#define wl_tcp_keep_attach(a)		(wl_tcp_keep_info_t *)0x0dadbeef
#ifdef BCM_OL_DEV
#define wl_tcp_keepalive_proc_msg(a, b, c)	do {} while (0)
#endif /* BCM_OL_DEV */
#endif /* TCPKAOE */

#endif	/* _wl_tcpkoe_h_ */
