/*
 * EXTLOG Module log ID to log Format String mapping table
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
 * $Id: wlc_extlog_idstr.h 708017 2017-06-29 14:11:45Z $
 */
#ifndef _WLC_EXTLOG_IDSTR_H_
#define _WLC_EXTLOG_IDSTR_H_

#include "wlioctl.h"

/* Strings corresponding to the IDs defined in wlioctl.h
 * This file is only included by the apps and not included by the external driver
 * Formats of pre-existing ids should NOT be changed
 */
log_idstr_t extlog_fmt_str[ ] = {
	{FMTSTR_DRIVER_UP_ID, 0, LOG_ARGTYPE_NULL,
	"Driver is Up\n"},

	{FMTSTR_DRIVER_DOWN_ID, 0, LOG_ARGTYPE_NULL,
	"Driver is Down\n"},

	{FMTSTR_SUSPEND_MAC_FAIL_ID, 0, LOG_ARGTYPE_INT,
	"wlc_suspend_mac_and_wait() failed with psmdebug 0x%08x\n"},

	{FMTSTR_NO_PROGRESS_ID, 0, LOG_ARGTYPE_INT,
	"No Progress on TX for %d seconds\n"},

	{FMTSTR_RFDISABLE_ID, 0, LOG_ARGTYPE_INT,
	"Detected a change in RF Disable Input 0x%x\n"},

	{FMTSTR_REG_PRINT_ID, 0, LOG_ARGTYPE_STR_INT,
	"Register %s = 0x%x\n"},

	{FMTSTR_EXPTIME_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Strong RF interference detected\n"},

	{FMTSTR_JOIN_START_ID, FMTSTRF_USER, LOG_ARGTYPE_STR,
	"Searching for networks with ssid %s\n"},

	{FMTSTR_JOIN_COMPLETE_ID, FMTSTRF_USER, LOG_ARGTYPE_STR,
	"Successfully joined network with BSSID %s\n"},

	{FMTSTR_NO_NETWORKS_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"No networks found. Please check if the network exists and is in range\n"},

	{FMTSTR_SECURITY_MISMATCH_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"AP rejected due to security mismatch. Change the security settings and try again...\n"},

	{FMTSTR_RATE_MISMATCH_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"AP rejected due to rate mismatch\n"},

	{FMTSTR_AP_PRUNED_ID, 0, LOG_ARGTYPE_INT,
	"AP rejected due to reason %d\n"},

	{FMTSTR_KEY_INSERTED_ID, 0, LOG_ARGTYPE_INT,
	"Inserting keys for algorithm %d\n"},

	{FMTSTR_DEAUTH_ID, FMTSTRF_USER, LOG_ARGTYPE_STR_INT,
	"Received Deauth from %s with Reason %d\n"},

	{FMTSTR_DISASSOC_ID, FMTSTRF_USER, LOG_ARGTYPE_STR_INT,
	"Received Disassoc from %s with Reason %d\n"},

	{FMTSTR_LINK_UP_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Link Up\n"},

	{FMTSTR_LINK_DOWN_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Link Down\n"},

	{FMTSTR_RADIO_HW_OFF_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Radio button is turned OFF. Please turn it on...\n"},

	{FMTSTR_RADIO_HW_ON_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Hardware Radio button is turned ON\n"},

	{FMTSTR_EVENT_DESC_ID, 0, LOG_ARGTYPE_INT_STR,
	"Generated event id %d: (result status) is (%s)\n"},

	{FMTSTR_PNP_SET_POWER_ID, 0, LOG_ARGTYPE_INT,
	"Device going into power state %d\n"},

	{FMTSTR_RADIO_SW_OFF_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Software Radio is disabled. Please enable it through the UI...\n"},

	{FMTSTR_RADIO_SW_ON_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Software Radio is enabled\n"},

	{FMTSTR_PWD_MISMATCH_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Potential passphrase mismatch. Please try a different one...\n"},

	{FMTSTR_FATAL_ERROR_ID, 0, LOG_ARGTYPE_INT,
	"Fatal Error: intstatus 0x%x\n"},

	{FMTSTR_AUTH_FAIL_ID, 0, LOG_ARGTYPE_STR_INT,
	"Authentication to %s Failed with status %d\n"},

	{FMTSTR_ASSOC_FAIL_ID, 0, LOG_ARGTYPE_STR_INT,
	"Association to %s Failed with status %d\n"},

	{FMTSTR_IBSS_FAIL_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Unable to start IBSS since PeerNet is already active\n"},

	{FMTSTR_EXTAP_FAIL_ID, FMTSTRF_USER, LOG_ARGTYPE_NULL,
	"Unable to start Ext-AP since PeerNet is already active\n"},

	{FMTSTR_MAX_ID, 0, 0, "\0"}
};

#endif /* _WLC_EXTLOG_IDSTR_H_ */
