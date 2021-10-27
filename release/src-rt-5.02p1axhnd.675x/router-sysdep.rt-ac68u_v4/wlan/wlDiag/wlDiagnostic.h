/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <ethernet.h>
#include <bcmevent.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <bcmwifi_rspec.h>
#include <bcmendian.h>

#ifndef WLDIAGNOSTIC_H
#define WLDIAGNOSTIC_H


#define MAX_ESCAN_BSS 256

#define SECURITY_MODE_WEP      0b00000001
#define SECURITY_MODE_WPA_PSK  0b00000010
#define SECURITY_MODE_WPA      0b00000100
#define SECURITY_MODE_WPA2_PSK 0b00001000
#define SECURITY_MODE_WPA2     0b00010000

#define ENCRYPT_MODE_TKIP      0b00000001
#define ENCRYPT_MODE_AES       0b00000010

#define SECURITY_MODE_S_NONE                 "None"
#define SECURITY_MODE_S_WEP                  "WEP"
#define SECURITY_MODE_S_WPA                  "WPA"
#define SECURITY_MODE_S_WPA2                 "WPA2"
#define SECURITY_MODE_S_WPA_WPA2             "WPA-WPA2"
#define SECURITY_MODE_S_WPA_ENTERPRISE       "WPA-Enterprise"
#define SECURITY_MODE_S_WPA2_ENTERPRISE      "WPA2-Enterprise"
#define SECURITY_MODE_S_WPA_WPA2_ENTERPRISE  "WPA-WPA2-Enterprise"

#define ENCRYPT_MODE_S_TKIP                 "TKIP"
#define ENCRYPT_MODE_S_AES                  "AES"


/* 802.11i/WPA RSN IE parsing utilities */
typedef struct {
	uint16 version;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *akm;
	uint8 *capabilities;
} rsn_parse_info_t;

typedef struct {
	wl_bss_info_t bssInfo;
	uint8 reserved[1024];
} wl_bss_info_resv_t;


#endif


