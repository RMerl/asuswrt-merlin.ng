/*
 * Required functions exported by the wlc_pdsvc.c
 * to common driver code
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
 * $Id: wlc_pdsvc.h 491282 2014-07-15 19:26:17Z $
 */

/********************************************************************
* Funcitonal Specification:
* pdsvc (proximity detection service) is a frond end interface to wlc layer. The service
* creates a particular proximity detection method based on WL IOVAR configuration.
* Each method has interfaces that are defined in wlc_pdmthd.h. Each method has an
* object which abstracts the implementation from the service layer.
*********************************************************************
*/

#ifndef _wlc_pdscv_h
#define _wlc_pdsvc_h

#include <typedefs.h>
#include <wlc_types.h>

/************************************************************
 * Function Purpose:
 * This funciton attaches the Proximity detection service module to wlc.
 * It reserves space for wlc_pdsvc_info_t.
 * This function is common across all proximity detection objects.
 * PARAMETERS:
 * wlc_info_t pointer
 * Return value:
 *  on Sucess: It returns a pointer to the wlc_pdsvc_info_t.
 *  on Failure: It returns NULL.
**************************************************************
*/
extern wlc_pdsvc_info_t *wlc_proxd_attach(wlc_info_t *wlc);

/*************************************************************
 * Function Purpose:
 * This funciton detaches the Proximity detection  service module from wlc.
 * The pdsvc interfaces will not be called after
 * the detach.
 * PARAMETERS:
 * wlc_pdsvc_info_t pointer to wlc structure
 * Return value: positive integer(>0) on success, and 0 on any internal errors.
**************************************************************
*/
extern int wlc_proxd_detach(wlc_pdsvc_info_t *const pdsvc);

/**********************************************************
 * Function Purpose:
 * This funciton process the action frame from the network.
 * PARAMETERS:
 * wlc_info_t pointer
 * dot11_management_header pointer
 * uint8 pointer to the action frame body
 * int action frame length.
 * wlc_d11rxhdr_t pointer to d11 header
 * Return value:
 * None.
************************************************************
*/
int
wlc_proxd_recv_action_frames(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len,
	wlc_d11rxhdr_t *wrxh, uint32 rspec);

/******************************************************
 * Function Purpose:
 * This function configures the transmit power of proximity frames.
 * PARAMETERS:
 * wlc_info_t pointer to wlc
 * wlc_bsscfg_t pointer to bsscfg
 * txpwr_offset pointer to txpower offset
 * Return value:
*******************************************************
 */
extern int
wlc_proxd_txpwr_offset(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 fc, int8 *txpwr_offset);

/******************************************************
 * Function Purpose:
 * This function configures mac control / phy control  used for TOF measurement pkts
 * PARAMETERS:
 * wlc_info_t pointer to wlc
 * phyctl pointer to phyctl
 * mch pointer to upper 16 bits of mac control word
 * pkttag pointer to pkttag struct
 * Return value:
*******************************************************
 */
extern void
wlc_proxd_tx_conf(wlc_info_t *wlc, uint16 *phyctl, uint16 *mch, wlc_pkttag_t *pkttag);

/* payload size of proximity detection action frame */
#define PROXD_AF_FIXED_LEN	20

/******************************************************
 * Function Purpose:
 * This function to determine if proxd is supported or not
 * PARAMETERS:
 * wlc_info_t pointer to wlc
 * Return value: returns TRUE if supported
*******************************************************
*/
extern bool wlc_is_proxd_supported(wlc_info_t *wlc);

#endif /* _wlc_pdscv_h */
