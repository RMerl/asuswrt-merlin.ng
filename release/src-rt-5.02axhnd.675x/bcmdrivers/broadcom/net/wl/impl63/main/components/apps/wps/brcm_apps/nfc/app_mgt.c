/*
 * NSA generic application management API
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
 * $Id: $
 */
#include <stdio.h>
#include <unistd.h>

#include "nsa_api.h"
#include "bsa_trace.h"
#include "app_nsa_utils.h"
#include "app_mgt.h"

/*
 * Local functions
 */
static void app_mgt_generic_callback(tNSA_MGT_EVT event, tNSA_MGT_MSG *p_data);

/*
 * Global variables
 */
tAPP_MGT_CB app_mgt_cb;

/*
 * Init management
 * Returns: 0 if success / -1 otherwise
 */
int app_mgt_init(void)
{
	memset(&app_mgt_cb, 0, sizeof(app_mgt_cb));
	return 0;
}

/*
 * Open communication to NSA Server
 *
 * Parameters      uipc_path: path to UIPC channels
 *                      p_mgt_callback: Application's custom callback
 * Returns: 0 if success / -1 otherwise
 */
int app_mgt_open(const char *p_uipc_path, tAPP_MGT_CUSTOM_CBACK *p_mgt_callback,
	BOOLEAN bsa_mgt_already_opened)
{
	tNSA_MGT_OPEN nsa_open_param;
	tNSA_STATUS nsa_status;
	int i;

	NSA_MgtOpenInit(&nsa_open_param);

	/* If the application passed a NULL UIPC path, use the default one */
	if (p_uipc_path == NULL) {
		strncpy(nsa_open_param.uipc_path, APP_DEFAULT_UIPC_PATH,
			sizeof(nsa_open_param.uipc_path) - 1);
		nsa_open_param.uipc_path[sizeof(nsa_open_param.uipc_path) - 1] = '\0';
	}
	else {
		/* use the given path */
		strncpy(nsa_open_param.uipc_path, p_uipc_path,
			sizeof(nsa_open_param.uipc_path) - 1);
		nsa_open_param.uipc_path[sizeof(nsa_open_param.uipc_path) - 1] = '\0';
	}

	/* Use the Generic Callback */
	nsa_open_param.callback = app_mgt_generic_callback;
	nsa_open_param.bsa_mgt_already_opened = bsa_mgt_already_opened;

	/* Save the Application's custom callback */
	app_mgt_cb.mgt_custom_cback = p_mgt_callback;

	/* Let's try to connect several time */
	for (i = 0; i < 5; i++) {
		/* Connect to NSA Server */
		nsa_status = NSA_MgtOpen(&nsa_open_param);
		if (nsa_status == NSA_SUCCESS)
			break;

		APP_ERROR1("Connection to server unsuccessful (%d), retrying...", i);
		sleep(1);
	}

	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR0("Unable to connect to server");
		return -1;
	}

	return 0;
}

/*
 * This function is used to closes the NSA connection
 * Returns: 0 if success / -1 otherwise
 */
int app_mgt_close(void)
{
	tNSA_MGT_CLOSE  nsa_close_param;
	tNSA_STATUS nsa_status;

	NSA_MgtCloseInit(&nsa_close_param);
	nsa_status = NSA_MgtClose(&nsa_close_param);
	if (nsa_status != NSA_SUCCESS) {
		APP_ERROR1("NSA_MgtClose failed status:%d", nsa_status);
		return -1;
	}

	return 0;
}

/*
 * This is an example of Generic Management callback function.
 * The Management Callback function is called in case of server
 * disconnection (e.g. server crashes) or when the Bluetooth
 * status changes (enable/disable)
 *
 * Parameters      event: the event received (Status or Disconnect event)
 *                      p_data:associated data
*/
static void app_mgt_generic_callback(tNSA_MGT_EVT event, tNSA_MGT_MSG *p_data)
{
	BOOLEAN exit_generic_cback = FALSE;

	/* If Application provided its own custom callback */
	if (app_mgt_cb.mgt_custom_cback != NULL) {
		/* Call it */
		exit_generic_cback = app_mgt_cb.mgt_custom_cback(event, p_data);
	}

	/* If custom callback indicates that does not need the generic callback to execute */
	if (exit_generic_cback != FALSE)
		return;

	switch (event) {
	case NSA_MGT_STATUS_EVT:
		APP_INFO0("app_mgt_generic_callback NSA_MGT_STATUS_EVT");
		if (p_data->status.enable == FALSE) {
			APP_INFO0("\tBluetooth Stopped");
		}
		else {
			/* The FALSE parameter indicates Starts */
			APP_INFO0("\tBluetooth restarted");
		}
		break;

	case NSA_MGT_DISCONNECT_EVT:
		/* Connection with the Server lost => Application will have to reconnect */
		APP_INFO1("app_mgt_generic_callback NSA_MGT_DISCONNECT_EVT reason:%d",
			p_data->disconnect.reason);
		break;
	}
}

/*
 * This function is used to kill the server
 * Returns: 0 if success / -1 otherwise
 */
int app_mgt_kill_server(void)
{
	tNSA_MGT_KILL_SERVER param;

	if (NSA_MgtKillServerInit(&param) == NSA_SUCCESS) {
		if (NSA_MgtKillServer(&param) == NSA_SUCCESS) {
			return 0;
		}
	}

	return -1;
}
