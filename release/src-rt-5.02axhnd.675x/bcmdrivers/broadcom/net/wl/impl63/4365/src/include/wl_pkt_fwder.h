/*
 * This defines an interface between the WLAN driver and clients which receive/transmit
 * native packets via the WLAN driver.
 * This interface provides APIs :
 *   - To send packets to the WLAN driver.
 *   - For clients like WPS, CnxAPI or TCP/IP stack to register their callbacks to receive frames.
 *   - Init pkt_forwarder registers callbacks with the WLAN driver.
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
 * $Id: wl_pkt_fwder.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef wl_pkt_fwder_h
#define wl_pkt_fwder_h

#ifdef __cplusplus
extern "C" {
#endif // endif

/* ---- Include Files ---------------------------------------------------- */

#include "wl_drv.h"

/* ---- Constants and Types ---------------------------------------------- */
#define WL_PKT_FWD_MAX_CLIENTS 5

typedef enum _wl_pkt_fwd_client_pri {
/* Clients priority needs to be appended if max clients is changed */
					WL_PKT_FWD_CLIENT_PRI_0 = 0,
					WL_PKT_FWD_CLIENT_PRI_1 = 1,
					WL_PKT_FWD_CLIENT_PRI_2 = 2,
					WL_PKT_FWD_CLIENT_PRI_3 = 3,
					WL_PKT_FWD_CLIENT_PRI_4 = 4
} wl_pkt_fwd_client_pri;

/* Packet forwarder handle. */
typedef void* wl_pkt_fwd_hdl;

/* Client  handle for Packet forwarder */
typedef void* wl_pkt_fwd_client_hdl;

/****************************************************************************
* Function:   wl_pkt_fwd_init
*
* Purpose:    This api intializes the packet forwarder by registering the pkt_fwd rx
*             callbacks with the wlan driver as per the wl_drv.h (wl_drv_netif_callbacks_t).
*
* Parameters: hdl (in) WLAN driver handle.
*
* Returns:    Packet forwarder handle.
*****************************************************************************
*/
wl_pkt_fwd_hdl wl_pkt_fwd_init(wl_drv_hdl hdl);

/****************************************************************************
* Function:   wl_pkt_fwd_deinit
*
* Purpose:    De-initialize packet forwarder.
*
* Parameters: hdl (in) Packet forwarder handle.
*
* Returns:    Nothing.
*****************************************************************************
*/
void wl_pkt_fwd_deinit(wl_pkt_fwd_hdl hdl);

/****************************************************************************
* Function:   wl_pkt_fwd_get_handle
*
* Purpose:    To provide Packet Forwarder handle
*
* Parameters: Nothing.
*
* Returns:     hdl (out) Packet forwarder handle.
*****************************************************************************
*/
wl_pkt_fwd_hdl wl_pkt_fwd_get_handle(void);

/****************************************************************************
* Function:   wl_pkt_fwd_register_netif
*
* Purpose:    Register callbacks for processing the rx frame of the client.
*
* Parameters: hdl       (in) Packet forwarder handle.
*             callbacks (in) Client network interface callbacks.
*
* Returns:   client_hdl for the packet forwarder
*****************************************************************************
*/
wl_pkt_fwd_client_hdl wl_pkt_fwd_register_netif(wl_pkt_fwd_hdl hdl,
                                wl_drv_netif_callbacks_t *callbacks,
                                wl_pkt_fwd_client_pri pri);

/****************************************************************************
* Function:   wl_pkt_fwd_unregister_netif
*
* Purpose:    Unregisters callbacks for processing the rx frame for the corresponding client.
*
* Parameters: hdl       (in) Packet forwarder handle.
*             client_hdl (in) Client hdl to unregister.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
int wl_pkt_fwd_unregister_netif(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl);

/****************************************************************************
* Function:   wl_pkt_fwd_tx
*
* Purpose:    Transmit packet from network interface to driver.
*
* Parameters: hdl       (mod) Packet forwarder handle.
*             client_hdl (in)  Client hdl.
*             pkt       (in)  Network interface packet. The packet format is
*                             network interface stack-specific.
*             len       (in)  Length of packet to transmit in bytes.
*
* Returns:	  0 on success, else error code.
*****************************************************************************
*/
int wl_pkt_fwd_tx(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl,
                                                wl_drv_netif_pkt pkt, unsigned int len);

#ifdef __cplusplus
	}
#endif // endif

#endif  /* wl_pkt_fwder_h  */
