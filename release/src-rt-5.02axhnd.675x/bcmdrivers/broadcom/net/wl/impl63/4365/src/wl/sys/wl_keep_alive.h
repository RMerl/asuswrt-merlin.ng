/*
 * Keep-alive offloading.
 *
 *	This feature implements periodic keep-alive packet transmission offloading.
 * The intended purpose is to keep an active session within a network address
 * translator (NAT) with a public server. This allows incoming packets sent
 * by the public server to the STA to traverse the NAT.
 *
 * An example application is to keep an active session between the STA and
 * a call control server in order for the STA to be able to receive incoming
 * voice calls.
 *
 * The keep-alive functionality is offloaded from the host processor to the
 * WLAN processor to eliminate the need for the host processor to wake-up while
 * it is idle; therefore, conserving power.
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
 *   $Id: wl_keep_alive.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef _wl_keep_alive_h_
#define _wl_keep_alive_h_

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */

/* Forward declaration */
typedef struct wl_keep_alive_info wl_keep_alive_info_t;

#define WL_MKEEP_ALIVE_IDMAX		3

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef KEEP_ALIVE

/*
*****************************************************************************
* Function:   wl_keep_alive_attach
*
* Purpose:    Initialize keep-alive private context.
*
* Parameters: wlc	(mod)	Common driver context.
*
* Returns:    Pointer to the keep-alive private context. Returns NULL on error.
*****************************************************************************
*/
extern wl_keep_alive_info_t *wl_keep_alive_attach(wlc_info_t *wlc);

/*
*****************************************************************************
* Function:   wl_keep_alive_detach
*
* Purpose:    Cleanup keep-alive private context.
*
* Parameters: info	(mod)	Keep-alive private context.
*
* Returns:    Nothing.
*****************************************************************************
*/
extern void wl_keep_alive_detach(wl_keep_alive_info_t *info);

/*
*****************************************************************************
* Function:   wl_keep_alive_up
*
* Purpose:	  Install periodic timer.
*
* Parameters: info	(mod)	Keep-alive private context.
*
* Returns:    0 on success.
*****************************************************************************
*/
extern int wl_keep_alive_up(wl_keep_alive_info_t *info);

/*
*****************************************************************************
* Function:   wl_keep_alive_down
*
* Purpose:    Cancel periodic timer.
*
* Parameters: info	(mod)	Keep-alive private context.
*
* Returns:    Number of periodic timers canceled..
*****************************************************************************
*/
extern unsigned int wl_keep_alive_down(wl_keep_alive_info_t *info);

extern int wl_keep_alive_upd_override_period(wlc_info_t *wlc, uint8 mkeepalive_index,
	uint32 override_period);
#else	/* stubs */

#define wl_keep_alive_attach(a)		(wl_keep_alive_info_t *)0x0dadbeef
#define wl_keep_alive_detach(a)		do {} while (0)
#define wl_keep_alive_up(a)		(0)
#define wl_keep_alive_down(a)		((void)(0))

#endif /* KEEP_ALIVE */

#endif	/* _wl_keep_alive_h_ */
