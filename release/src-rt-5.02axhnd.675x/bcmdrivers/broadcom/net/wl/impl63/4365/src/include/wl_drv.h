/*
 * This defines the API between the Wireless LAN driver and the host application.
 * It may be used independent of the WLAN mode (NIC mode, dongle mode), and also
 * independent of the WLAN split-model (BMAC, full-dongle etc). It is
 * intended to be used on platforms where the interface between the application
 * and driver is not defined or coupled with the operating system. This interface
 * provides APIs to:
 *   - Init/de-init the WLAN driver.
 *   - Send IOCTLs to the driver.
 *   - Register application callbacks for driver events.
 *   - Send and receive packets to the network stack.
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
 * $Id: wl_drv.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef wl_drv_h
#define wl_drv_h

#ifdef __cplusplus
extern "C" {
#endif // endif

/* ---- Include Files ---------------------------------------------------- */

#include "dhdioctl.h"
#include "wlioctl.h"
#include "proto/bcmevent.h"

/* ---- Constants and Types ---------------------------------------------- */

/* Wireless driver handle. */
typedef void* wl_drv_hdl;

/* IOCTL declaration. */
typedef struct wl_drv_ioctl_t
{
	dhd_ioctl_t		d;
	wl_ioctl_t		w;
	unsigned int		ifidx;
} wl_drv_ioctl_t;

/****************************************************************************
* Function:   wl_drv_event_callback
*
* Purpose:    User registered callback to receive events.
*
* Parameters: event      (in) Driver event.
*             event_data (in) Optional event data.
*
* Returns:    Nothing.
*****************************************************************************
*/
typedef void (*wl_drv_event_callback)(wl_event_msg_t* event, void* event_data);

/****************************************************************************
* Function:   wl_drv_event_packet_callback
*
* Purpose:    User registered callback to receive event packet.
*
* Parameters: event      (in) Driver event packet.
*
* Returns:    Nothing.
*****************************************************************************
*/
typedef void (*wl_drv_event_packet_callback)(bcm_event_t* event);

/* ----------------------------------------------------------------------- */
/* Network interface packet. */
typedef void* wl_drv_netif_pkt;

/****************************************************************************
* Function:   wl_drv_netif_start_queue
*
* Purpose:    Callback invoked by driver to start tx of packets from network
*             interface to driver.
*
* Parameters: None.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
typedef int (*wl_drv_netif_start_queue)(void);

/****************************************************************************
* Function:   wl_drv_netif_stop_queue
*
* Purpose:    Callback invoked by driver to stop tx of packets from network
*             interface to driver.
*
* Parameters: None.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
typedef int (*wl_drv_netif_stop_queue)(void);

/****************************************************************************
* Function:   wl_drv_netif_rx_pkt
*
* Purpose:    Callback invoked by driver to send packet from driver to network
*             interface.
*
* Parameters: pkt (in) Received network packet. The packet format is network
*                      interface stack-specific.
*             len (in) Length of received packet.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
typedef int (*wl_drv_netif_rx_pkt)(wl_drv_netif_pkt pkt, unsigned int len);

/* Network interface callback functions registered with driver. */
typedef struct wl_drv_netif_callbacks_t
{
	wl_drv_netif_start_queue	start_queue;
	wl_drv_netif_stop_queue		stop_queue;
	wl_drv_netif_rx_pkt		rx_pkt;

} wl_drv_netif_callbacks_t;

/* ----------------------------------------------------------------------- */

/****************************************************************************
* Function:   wl_drv_btamp_rx_pkt
*
* Purpose:    Callback invoked by driver to send BTAMP packet from driver to
*             network interface.
*
* Parameters: pkt (in) Received BTAMP packet.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
typedef int (*wl_drv_btamp_rx_pkt_callback)(wl_drv_netif_pkt pkt, unsigned int len);

/****************************************************************************
* Function:   wl_drv_btamp_rx_pkt
*
* Purpose:    Callback invoked by driver to send BTAMP packet from driver to
*             network interface.
*
* Parameters: pkt (in) Received BTAMP packet.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
typedef void* (*wl_drv_btamp_pkt_alloc_callback)(unsigned int len);

/* BTAMP callback functions registered with driver. */
typedef struct wl_drv_btamp_callbacks_t
{
	wl_drv_btamp_rx_pkt_callback		rx_pkt;
	wl_drv_btamp_pkt_alloc_callback		pkt_alloc;

} wl_drv_btamp_callbacks_t;

/* ----------------------------------------------------------------------- */
/* File handle. */
typedef void* wl_drv_file_hdl;

/****************************************************************************
* Function:   wl_drv_file_open
*
* Purpose:    Callback invoked by driver to open a file. This may be used to
*             open a nvram text file or dongle binary image stored on the
*             file-system.
*
* Parameters: filename (in) Full path of file to open.
*
* Returns:    File handle.
*****************************************************************************
*/
typedef wl_drv_file_hdl(*wl_drv_file_open)(const char *filename);

/****************************************************************************
* Function:   wl_drv_file_read
*
* Purpose:    Callback invoked by driver to read contents of a file.
*
* Parameters: buf      (mod) Destination buffer to read file contents.
*             len      (in)  Maximum number of bytes to read.
*             file_hdl (mod) File handle returned by wl_drv_file_open().
*
* Returns:    Number of bytes actually read.
*****************************************************************************
*/
typedef int (*wl_drv_file_read)(char *buf, int len, wl_drv_file_hdl file_hdl);

/****************************************************************************
* Function:   wl_drv_file_close
*
* Purpose:    Callback invoked by driver to close a file.
*
* Parameters: file_hdl (mod) File handle returned by wl_drv_file_open().
*
* Returns:    Nothing.
*****************************************************************************
*/
typedef void (*wl_drv_file_close)(wl_drv_file_hdl file_hdl);

/* File-system access functions registered with driver. */
typedef struct wl_drv_file_callbacks_t
{
	wl_drv_file_open	open;
	wl_drv_file_read	read;
	wl_drv_file_close	close;

} wl_drv_file_callbacks_t;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

/****************************************************************************
* Function:   wl_drv_init
*
* Purpose:    Initialize driver.
*
* Parameters: firmware_path (in) Full file-system path of firmware binary image.
*                                This paramemeter is optional, and specifies
*                                the firmware image to download at initialization
*                                time. NULL may be specified if the driver contains
*                                an embedded firmware image. If a path is specified
*                                and an embedded firmware image is present, the
*                                firmware image specified via the path will take
*                                precedence.
*             nvram_path (in)    Full file-system path of non-volatile WLAN
*                                configuration parameters text file.
*                                (e.g. board identifiers). These suppliment the
*                                on-chip params in the OTP (one-time programmable)
*                                memory. The format of the text file is
*                                param=value pairs, e.g. "manfid=0x1234". May be
*                                set to NULL if no configuration parameters
*                                are required. This parameter is mutually exclusive
*                                with 'nvram_params'.
*             nvram_params (in)  This is an alternate to 'nvram_path'. It is a
*                                string representation of the text file.
*                                e.g. "manfid=0x1234\nprodid=0x5678". May be
*                                set to NULL if no configuration parameters
*                                are required.
*             callbacks (in)     Callbacks used to interface driver to file-system.
*                                May be set to NULL if both 'firmware_path' and
*                                'nvram_path' are set to NULL.
*
* Returns:    Pointer to driver context.
*****************************************************************************
*/
wl_drv_hdl wl_drv_init(const char *firmware_path, const char *nvram_path,
                       const char *nvram_params, wl_drv_file_callbacks_t *callbacks);

/****************************************************************************
* Function:   wl_drv_deinit
*
* Purpose:    De-initialize driver.
*
* Parameters: hdl (mod) Pointer to driver context (returned by wl_drv_init()).
*
* Returns:    Nothing.
*****************************************************************************
*/
void wl_drv_deinit(wl_drv_hdl hdl);

/****************************************************************************
* Function:   wl_drv_ioctl
*
* Purpose:    Issue an I/O control command to the driver.
*
* Parameters: hdl (mod) Pointer to driver context (returned by wl_drv_init()).
*             ioc (in)  I/O control command.
*
* Returns:    0 on success, else -1.
*****************************************************************************
*/
int wl_drv_ioctl(wl_drv_hdl hdl, wl_drv_ioctl_t *ioc);

/****************************************************************************
* Function:   wl_drv_register_event
*
* Purpose:    Register callback for driver events.
*
* Parameters: hdl (mod) Pointer to driver context (returned by wl_drv_init()).
*             cb  (in)  Callback invoked to send driver events to application.
*
* Returns:    Nothing.
*****************************************************************************
*/
#define wl_drv_register_event_callback(hdl, cb) \
	wl_drv_register_event_callback_if((hdl), 0, (cb))
void wl_drv_register_event_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
                                       wl_drv_event_callback cb);

/****************************************************************************
* Function:   wl_drv_deregister_event
*
* Purpose:    Deregister callback for driver events.
*
* Parameters: hdl (mod) Pointer to driver context (returned by wl_drv_init()).
*             cb  (in)  Callback to deregister.
*
* Returns:    Nothing.
*****************************************************************************
*/
#define wl_drv_deregister_event_callback(hdl, cb) \
	wl_drv_deregister_event_callback_if((hdl), 0, (cb))
void wl_drv_deregister_event_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
                                         wl_drv_event_callback cb);

/****************************************************************************
* Function:   wl_drv_register_event_packet
*
* Purpose:    Register callback for driver event packets.
*
* Parameters: hdl (mod) Pointer to driver context (returned by wl_drv_init()).
*             cb  (in)  Callback invoked to send driver event packets to application.
*
* Returns:    Nothing.
*****************************************************************************
*/
#define wl_drv_register_event_packet_callback \
	wl_drv_register_event_packet_callback_if((hdl), 0, (cb))
void wl_drv_register_event_packet_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
	wl_drv_event_packet_callback cb);

/****************************************************************************
* Function:   wl_drv_deregister_event_packet
*
* Purpose:    Deregister callback for driver events.
*
* Parameters: hdl (mod) Pointer to driver context (returned by wl_drv_init()).
*             cb  (in)  Callback to deregister.
*
* Returns:    Nothing.
*****************************************************************************
*/
#define wl_drv_deregister_event_packet_callback \
	wl_drv_deregister_event_packet_callback_if((hdl), 0, (cb))
void wl_drv_deregister_event_packet_callback_if(wl_drv_hdl hdl, unsigned int ifidx,
	wl_drv_event_packet_callback cb);

/****************************************************************************
* Function:   wl_drv_tx_pkt
*
* Purpose:    Transmit packet from network interface to driver.
*
* Parameters: hdl (mod) Pointer to driver context (returned by wl_drv_init()).
*             pkt (in)  Network interface packet. The packet format is
*                       network interface stack-specific.
*             len (in)  Length of packet to transmit in bytes.
*
* Returns:	  0 on success, else error code.
*****************************************************************************
*/
#define wl_drv_tx_pkt(hdl, pkt, len) wl_drv_tx_pkt_if((hdl), 0, (pkt), (len))
int wl_drv_tx_pkt_if(wl_drv_hdl hdl, unsigned int ifidx, wl_drv_netif_pkt pkt,
                     unsigned int len);

/****************************************************************************
* Function:   wl_drv_register_netif_callbacks
*
* Purpose:    Register callbacks for network interface.
*
* Parameters: hdl       (mod) Pointer to driver context (returned by wl_drv_init()).
*             callbacks (in)  Callbacks used to interface driver to network stack.
*
* Returns:    Nothing.
*****************************************************************************
*/
#define wl_drv_register_netif_callbacks(hdl, cb) \
	   wl_drv_register_netif_callbacks_if((hdl), 0, (cb))
void wl_drv_register_netif_callbacks_if(wl_drv_hdl hdl, unsigned int ifidx,
                                        wl_drv_netif_callbacks_t *callbacks);

/****************************************************************************
* Function:   wl_drv_deregister_netif_callbacks
*
* Purpose:    Deregister callbacks from network interface.
*
* Parameters: hdl       (mod) Pointer to driver context (returned by wl_drv_init()).
*             callbacks (in)  Callbacks used to interface driver to network stack.
*
* Returns:    Nothing.
*****************************************************************************
*/
#define wl_drv_deregister_netif_callbacks(hdl, cb) \
	   wl_drv_deregister_netif_callbacks_if((hdl), 0, (cb))
void wl_drv_deregister_netif_callbacks_if(wl_drv_hdl hdl, unsigned int ifidx,
	wl_drv_netif_callbacks_t *callbacks);

/****************************************************************************
* Function:   wl_drv_register_btamp_callbacks
*
* Purpose:    Register callbacks for BTAMP interface.
*
* Parameters: hdl       (mod) Pointer to driver context (returned by wl_drv_init()).
*             cb        (in)  Receive packet callback.
*
* Returns:    Nothing.
*****************************************************************************
*/
#define wl_drv_register_btamp_callbacks(hdl, cb) \
	wl_drv_register_btamp_callbacks_if(hdl, 0, cb)
void wl_drv_register_btamp_callbacks_if(wl_drv_hdl hdl, unsigned int ifidx,
                                        wl_drv_btamp_callbacks_t *callbacks);

/****************************************************************************
* Function:   wl_drv_get_handle
*
* Purpose:    To provide wl driver handle
*
* Parameters: Nothing
*
* Returns:    hdl (out) wl driver context/handle.
*****************************************************************************
*/

wl_drv_hdl wl_drv_get_handle(void);

#ifdef __cplusplus
	}
#endif // endif

#endif  /* wl_drv_h  */
