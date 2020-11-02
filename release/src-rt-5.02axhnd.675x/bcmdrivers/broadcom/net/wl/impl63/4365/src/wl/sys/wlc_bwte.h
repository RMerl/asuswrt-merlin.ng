/*
* BT WLAN TUNNEL ENGINE public header file
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
* $Id: wlc_bwte.h 708017 2017-06-29 14:11:45Z $
*
*/
#ifndef _WLC_BWTE_H_
#define _WLC_BWTE_H_

typedef enum {
	WLC_BWTE_CLIENT_TBOW = 0
} wlc_bwte_client_t;

typedef enum {
	WLC_BWTE_CTL_MSG = 0,
	WLC_BWTE_LO_DATA = 1,
	WLC_BWTE_HI_DATA = 2,
	WLC_BWTE_PAYLOAD_CNT
} wlc_bwte_payload_t;

typedef int (*wlc_bwte_cb)(void* arg, uchar* p, int len);

void* wlc_bwte_attach(wlc_info_t* wlc);
void wlc_bwte_detach(bwte_info_t *bwte_info);

/* Register/Unregister client to BT WLAN TUNNEL ENGINE
*
*  To register, client need provide valid client id and proper callback function pointer.
*  BWTE support 3 type payload: control message, low priority data and high priority data.
*  Client can selectively register the callback function, but at least one.
*
*  When the provided callback get called, client need process the provided buffer synchronously
*  and give back the ownership of the buffer when exit from the callback function.
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*  f_ctl - callback function pointer for control message
*  f_lo_data - callback function pointer for low priority data
*  f_hi_data - callback function pointer for high priority data
*  arg - argument will be passed in callback function
*
*  return - bcm error return
*/
int wlc_bwte_register_client(bwte_info_t *bwte_info, int client_id, wlc_bwte_cb f_ctl,
	wlc_bwte_cb f_lo_data, wlc_bwte_cb f_hi_data, void* arg);
void wlc_bwte_unregister_client(bwte_info_t *bwte_info, int client_id);

/* Send msg/data to BT module through the tunnel
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*  payload - payload type
*  buf - pointer to buffer containing control message/low_high priority data.
*	 Clien will give up ownership from succeed return of this function
*        until free callback function get called.
*  len - lenght of provided buffer
*  free_func - callback function pointer to return owner ship of provided buffer
*  arg - argument will be passed in callback function
*
*  return - bcm error return
*/
int wlc_bwte_send(bwte_info_t *bwte_info, int client_id, wlc_bwte_payload_t payload, uchar* buf,
	int len, wlc_bwte_cb free_func, void* arg);

/* Help function for client, optional for most clients */

/* Manually inovke BT->Wlan ISR
*
*  Sometimes ISR processing may be skipped because of client ia in special condition.
*  After the special condition cleared, client may want to manually trigger ISR processing.
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*/
void wlc_bwte_process_bt_intr(bwte_info_t *bwte_info, int client_id);

/* Manually reclaim wlan buf from BWTE
*
*  Sometimes client may want to force reclaim all wlan buffer.
*
*  bwte_info - bwte module context pointer
*  client_id - pre-assigned unique id
*	       shared with corresponding WLAN and BT module for this client
*  payload - payload type
*  cleanup - indication if need force reclaim ownership of wlan buffer
*/
void wlc_bwte_reclaim_wlan_buf(bwte_info_t *bwte_info, int client_id, wlc_bwte_payload_t payload,
	bool cleanup);
#endif /* _WLC_BWTE_H_ */
