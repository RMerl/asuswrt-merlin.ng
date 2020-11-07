/*
 * WL Packet Forwarder Interface definitions
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
 * $Id: wl_pkt_fwder.c 473326 2014-04-29 00:37:35Z $
 */

/* ---- Include Files ---------------------------------------------------- */

#include "typedefs.h"
#include "wl_pkt_fwder.h"

/* Map between public WL driver handle and WL Pkt handle info struct. */
#define WL_PKT_FWD_HDL_TO_WL_PKT_FWD_INFO(hdl) ((wl_pkt_fwd_info *) (hdl))
#define WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(hdl) ((wl_pkt_fwd_hdl) (&hdl))

/* Packet forwarder cb handle */
typedef struct wl_pkt_fwd_client_cb {
	wl_pkt_fwd_client_pri priority;
	wl_drv_netif_callbacks_t	callbacks;
	bool inUse;
} wl_pkt_fwd_client_cb_t;

typedef struct _wl_pkt_fwd_info {
	wl_drv_hdl	drv_handle;
	wl_pkt_fwd_client_cb_t *list;
} wl_pkt_fwd_info;

static wl_pkt_fwd_client_cb_t g_Client_cblist[WL_PKT_FWD_MAX_CLIENTS];

static wl_pkt_fwd_info g_Pkt_fwd_info;

/* ----------------------------------------------------------------------- */
static wl_pkt_fwd_client_cb_t *
wl_client_cb_get(wl_drv_netif_callbacks_t *callbacks, wl_pkt_fwd_client_pri pri)
{
	wl_pkt_fwd_client_cb_t *client;

	if (pri >= WL_PKT_FWD_MAX_CLIENTS)
		return NULL; /* Priority can't be more than or equal to max Clients */

	client = &(g_Client_cblist[pri]);

	if (client->inUse)
		return NULL; /* Already a client with same priority registered */

	memset((void *)client, 0, sizeof(wl_pkt_fwd_client_cb_t));

	client->priority = pri;
	client->callbacks = *callbacks;
	client->inUse = 1;

	return client;
}

/* ----------------------------------------------------------------------- */
static void
wl_client_cb_free(wl_pkt_fwd_client_cb_t  *client)
{
	if (client->inUse)
		memset((void *)client, 0, sizeof(wl_pkt_fwd_client_cb_t));

}

/* ----------------------------------------------------------------------- */
int
wl_pkt_fwd_process_rx_pkt(wl_drv_netif_pkt pkt, unsigned int len)
{
	wl_pkt_fwd_client_cb_t *client = g_Client_cblist;
	wl_drv_netif_callbacks_t *cb;
	int     index = 0;

	for (; index < WL_PKT_FWD_MAX_CLIENTS; client++, index++) {

		if (!client->inUse)
			continue;

		cb = &(client->callbacks);

		if (!cb->rx_pkt)
			continue;

		/* If a client absorbs the packet it would return 0/success */
		if (!cb->rx_pkt(pkt, len))
			return 0;

	}

	return -1; /* None of the client has absorbed the Rx Packet */
}

/* ----------------------------------------------------------------------- */
int
wl_pkt_fwd_process_start_queue(void)
{

	wl_pkt_fwd_client_cb_t *client = g_Client_cblist;
	wl_drv_netif_callbacks_t *cb;
	int 	index = 0;

	for (; index < WL_PKT_FWD_MAX_CLIENTS; client++, index++) {

		if (!client->inUse)
			continue;

		cb = &(client->callbacks);

		if (cb->start_queue != NULL)
			cb->start_queue();
	}

	return 0;

}

/* ----------------------------------------------------------------------- */
int
wl_pkt_fwd_process_stop_queue(void)
{

	wl_pkt_fwd_client_cb_t *client = g_Client_cblist;
	wl_drv_netif_callbacks_t *cb;
	int 	index = 0;

	for (; index < WL_PKT_FWD_MAX_CLIENTS; client++, index++) {

		if (!client->inUse)
			continue;

		cb = &(client->callbacks);

		if (cb->stop_queue != NULL)
			cb->stop_queue();
	}

	return 0;

}

/* ----------------------------------------------------------------------- */
wl_pkt_fwd_hdl wl_pkt_fwd_init(wl_drv_hdl hdl)
{
	wl_drv_netif_callbacks_t netif_callbacks;

	if (!hdl)
		return NULL;

	if (g_Pkt_fwd_info.drv_handle) /* Packet Fwder is already initialised */
		return WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(g_Pkt_fwd_info);

	memset((void *)&netif_callbacks, 0, sizeof(wl_drv_netif_callbacks_t));

	/* Register network interface callbacks. */
	netif_callbacks.rx_pkt        = wl_pkt_fwd_process_rx_pkt;
	netif_callbacks.start_queue   = wl_pkt_fwd_process_start_queue;
	netif_callbacks.stop_queue    = wl_pkt_fwd_process_stop_queue;

	wl_drv_register_netif_callbacks(hdl, &netif_callbacks);

	memset((void *)&g_Pkt_fwd_info, 0, sizeof(g_Pkt_fwd_info));
	memset((void *)&g_Client_cblist, 0, sizeof(g_Client_cblist));

	g_Pkt_fwd_info.drv_handle = hdl;
	g_Pkt_fwd_info.list = g_Client_cblist;

	return WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(g_Pkt_fwd_info);
}

/* ----------------------------------------------------------------------- */
void wl_pkt_fwd_deinit(wl_pkt_fwd_hdl hdl)
{
	wl_pkt_fwd_info *info;

	if (hdl) {
		info = WL_PKT_FWD_HDL_TO_WL_PKT_FWD_INFO(hdl);
		memset((void *)&g_Pkt_fwd_info, 0, sizeof(g_Pkt_fwd_info));
		memset((void *)&g_Client_cblist, 0, sizeof(g_Client_cblist));
	}

}

/* ----------------------------------------------------------------------- */
wl_pkt_fwd_hdl wl_pkt_fwd_get_handle()
{
	return WL_PKT_FWD_INFO_TO_WL_PKT_FWD_HDL(g_Pkt_fwd_info);
}

/* ----------------------------------------------------------------------- */
wl_pkt_fwd_client_hdl wl_pkt_fwd_register_netif(wl_pkt_fwd_hdl hdl,
                                            wl_drv_netif_callbacks_t *callbacks,
                                            wl_pkt_fwd_client_pri pri)
{
	wl_pkt_fwd_client_cb_t *cb;

	if (!hdl)
		return NULL;

	cb = wl_client_cb_get(callbacks, pri);

	return ((wl_pkt_fwd_client_hdl) cb);

}

/* ----------------------------------------------------------------------- */
int wl_pkt_fwd_unregister_netif(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl)
{
	wl_pkt_fwd_client_cb_t *cb = (wl_pkt_fwd_client_cb_t *)client_hdl;
	int status = -1;

	if (!hdl)
		return status;

	if (cb) {
		wl_client_cb_free(cb);
		status = 0;
	}

	return status;

}

/* ----------------------------------------------------------------------- */
int wl_pkt_fwd_tx(wl_pkt_fwd_hdl hdl, wl_pkt_fwd_client_hdl client_hdl,
                          wl_drv_netif_pkt pkt, unsigned int len)
{
	wl_pkt_fwd_info *info = WL_PKT_FWD_HDL_TO_WL_PKT_FWD_INFO(hdl);
	int status = -1;

	if (!client_hdl || !info)
		return status;

	/* Length of the packet cant be 0 */
	ASSERT(len);

	return wl_drv_tx_pkt(info->drv_handle, pkt, len);

}
