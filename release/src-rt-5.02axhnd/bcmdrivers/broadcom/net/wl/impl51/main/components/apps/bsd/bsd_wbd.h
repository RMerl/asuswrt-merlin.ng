/*
 * WBD related include file
 *
 * Copyright 2019 Broadcom
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bsd_wbd.h 767389 2018-09-10 03:17:33Z $
 */

#ifndef _BSD_WBD_H_
#define _BSD_WBD_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <security_ipc.h>
#include <typedefs.h>

#include "wbd_rc_shared.h"
#define BSD_WBD_REQ_BUFSIZE	512

/* Loopback IP address */
#define BSD_WBD_LOOPBACK_IP	"127.0.0.1"
#define BSD_WBD_SERVERT_PORT	(EAPD_WKSP_WBD_UDP_PORT + 0x103)

/* WiFi Blanket related NVRAMS */
#define BSD_WBD_NVRAM_MAP_MODE		"multiap_mode"
#define BSD_WBD_NVRAM_IFNAMES		"wbd_ifnames"
#define BSD_WBD_NVRAM_WEAK_STA_ALGO	"wbd_wc_algo"
#define BSD_WBD_NVRAM_WEAK_STA_POLICY	"wbd_weak_sta_policy"
#define BSD_WBD_NVRAM_WEAK_STA_CFG	"wbd_weak_sta_cfg"
#define BSD_WBD_NVRAM_MCHAN_SLAVE       "wbd_mchan"

/* STA status flags */
#define BSD_WBD_STA_WEAK_PENDING	0x0001	/* STA is weak and reported to slave */
#define BSD_WBD_STA_WEAK		0x0002	/* STA is weak and accepted by master */
#define BSD_WBD_STA_DWDS		0x0004	/* STA is DWDS STA, no need to check for weak STA */
#define BSD_WBD_STA_IGNORE		0x0008	/* Ignore the STA from steering */
#define BSD_WBD_STA_DWELL		0x0010	/* STA is in dwell period */

/* WBD's error codes */
#define BSDE_WBD_OK			100
#define BSDE_WBD_FAIL			101
#define BSDE_WBD_IGNORE_STA		102
#define BSDE_WBD_NO_SLAVE_TO_STEER	103
#define BSDE_WBD_BOUNCING_STA		104
#define BSDE_WBD_UN_ASCSTA		105

/* MulitAP Modes */
#define MAP_MODE_FLAG_DISABLED		0x0000	/* Disabled */
#define MAP_MODE_FLAG_CONTROLLER	0x0001	/* Controller */
#define MAP_MODE_FLAG_AGENT		0x0002	/* Agent */

#define BSD_WBD_DISABLED(mode) (((mode) <= MAP_MODE_FLAG_DISABLED))

typedef struct bsd_bssinfo bsd_bssinfo_t;
typedef struct bsd_info bsd_info_t;

/* Rules and Threshold parameters for finding weak clients */
typedef struct bsd_wbd_weak_sta_policy {
	int idle_rate;		/* data rate threshold to measure STA is idle */
	int rssi;		/* rssi threshold */
	uint32 phyrate;		/* phyrate threshold in Mbps */
	int tx_failures;	/* threshold for tx retry */
	uint32 flags;		/* extension flags (Rules) */
} bsd_wbd_weak_sta_policy_t;

/* List of bss info on which WiFi Blanket is enabled */
typedef struct bsd_wbd_bss_list {
	uint8 algo;					/* Which find weak STA algorithm to use */
	uint8 policy;					/* Which policy to use to find weak sta */
	wbd_weak_sta_policy_t *weak_sta_cfg;	/* Configuration of the policy chosen */
	bsd_bssinfo_t *bssinfo;				/* Pointer to BSS info structure on which
							 * WBD is enabled
							 */
	int wbd_band_type;				/* WBD Band Type fm Chanspec & Bridge */

	struct bsd_wbd_bss_list *next;
} bsd_wbd_bss_list_t;

/* Information regarding WiFi Blanket */
typedef struct bsd_wbd_info {
	bsd_wbd_bss_list_t *bss_list;	/* List of BSS on which WBD is enabled */
} bsd_wbd_info_t;

/* Extern Declarations */
extern int bsd_wbd_set_ifnames(bsd_info_t *info);
extern int bsd_wbd_init(bsd_info_t *info);
extern void bsd_wbd_reinit(bsd_info_t *info);
extern void bsd_cleanup_wbd(bsd_wbd_info_t *info);
extern void bsd_wbd_check_weak_sta(bsd_info_t *info);
extern void bsd_wbd_update_bss_info(bsd_info_t *info, char *ifname);
#endif /*  _BSD_WBD_H_ */
