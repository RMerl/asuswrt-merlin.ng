/**
 * Dongle packet filter.
 *
 * This feature implements a packet filter engine that determines whether
 * packets received by the dongle are forwarded to the host processor.
 *
 * When packet filters are installed, received packets are run through the
 * set of enabled filters. The filter criteria is specified by the host
 * application. If a match is found, the configured action is performed.
 *
 * If no packet filters are installed, no special filtering is performed.
 * Received packets are forwarded to the host as normal.
 *
 * The host application creates filters based upon packet content pattern
 * matches - it specifies an offset within received packets to start matching,
 * the pattern to match, the size of the pattern, and a bitmask that indicates
 * which bits within the pattern should be matched. The bitmask used for filter
 * pattern matching uses a bit-for-bit bitmask. Each bit of the pattern can
 * be selectively matched, in order to support sub-byte matches.
 *
 * Host applications can install multiple packet filters, and each can be
 * independently enabled or disabled.
 *
 * Each filter is independent. i.e. There is a match if any of the filters match.
 * The filter results are not ANDed together.
 *
 * The filter engine must be configured to perform an action based upon the
 * filter match results. Two actions are supported:
 *
 *   - Forward packets from dongle to host on match, else discard packet (default).
 *   - Discard packets on match, else forward packet from dongle to host.
 *
 * This allows the filter engine to be configured to either forward packets
 * that match any of the enabled filters. Or, alternatively, to forward packets
 * that do not match any of the enabled filters. Received packets iterate
 * through only the list of enabled filters. If a match is found, the
 * requested "match" action is performed (without iterating through any
 * remaining enabled filters). If no match is found after iterating through
 * all enabled filters, the requested "non-match" action is performed.
 *
 * In addition, each filter has an associated match action "polarity"
 * attribute. This allows the result of each filter match to be negated.
 * For example, to create a filter to discard all packets except UDP,
 * create a pattern match filter for UDP, set the polarity attribute, and
 * configure the filter engine to discard packets on match. Note, that this
 * has the same result as creating a pattern match filter for UDP, not setting
 * the polarity attribute, and configuring the filter engine to forward
 * packets on match. However, the polarity attribute is useful in situations
 * where multiple filters are enabled. e.g. discard all non-UDP packets, and
 * discard all UDP packets with source port xxx.
 *
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
 *   $Id: wl_d0_filter.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wlc_d0_filter_h_
#define _wlc_d0_filter_h_

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */

/* Forward declaration */
typedef struct d0_filter_info	wlc_d0_filter_info_t;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef PACKET_FILTER

/*
*****************************************************************************
* Function:   wlc_d0_filter_attach
*
* Purpose:    Initialize packet filter private context.
*
* Parameters: wlc	(mod)	Common driver context.
*
* Returns:    Pointer to the packet filter private context. Returns NULL on error.
*****************************************************************************
*/
extern wlc_d0_filter_info_t *wlc_d0_filter_attach(wlc_info_t *wlc);

/*
*****************************************************************************
* Function:   wlc_d0_filter_detach
*
* Purpose:    Cleanup packet filter private context.
*
* Parameters: info	(mod)	Packet filter engine private context.
*
* Returns:    Nothing.
*****************************************************************************
*/
extern void wlc_d0_filter_detach(wlc_d0_filter_info_t *info);

/*
*****************************************************************************
* Function:   wlc_pkt_fitler_recv_proc
*
* Purpose:    Process received frames.
*
* Parameters: info	(mod)	Packet filter engine private context.
*				  sdu		(in)	Received packet.
*
* Returns:    TRUE if received packet should be forwarded. FALSE if
*             it should be discarded.
*****************************************************************************
*/
extern bool wlc_d0_filter_recv_proc(wlc_d0_filter_info_t *info, void *sdu);

#else	/* stubs */

#define wlc_d0_filter_attach(a)	(wlc_d0_filter_info_t *)0x0dadbeef
#define wlc_d0_filter_detach(a)	do {} while (0)
#define wlc_d0_filter_recv_proc(a, b)	(TRUE)

#endif /* PACKET_FILTER */

#endif	/* _wlc_d0_filter_h_ */
