/*
 * Broadcom STA monitor implementation
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bcm_stamon.c 788014 2020-06-18 04:18:18Z $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wlutils.h>
#include <shutils.h>
#include <bcmendian.h>
#include <bcmnvram.h>

#include "bcmutils.h"
#include "bcm_stamon.h"
#include "bcm_usched.h"

#define STAMON_ERROR_PREFIX	"BCMSTAMON - "

#define STAMON_MAX_IFNAME_LEN		16	/* Maximum length of interface name */
#define STAMON_MAX_RSSI_LEN		5	/* Number of RSSI counts to be stored */
#define STAMON_MAX_RSSI_AVG		5	/* Number of RSSI's to be considered for average */
#define STAMON_MAX_STA_MONITORED	4	/* Max number of STA monitored at one instance */
#define STAMON_MIN_DRIVER_BUFLEN	256	/* Minimum Buffer length required for driver */

#define STAMON_IOVAR_NAME	"sta_monitor" /* Name of the IOVAR for sta monitoring */

/* Some utility macros */
#define STAMON_SEC_MICROSEC(x)	((x) * 1000 * 1000)
#define STAMON_MSEC_USEC(x)	((x) * 1000)
#define STAMON_RSSI_VALID(x)	((x) < (0) && (x) > (-125))

/* IOCTL swapping mode for Big Endian host with Little Endian dongle. */
/* The below macros handle endian mis-matches between host and dongle. */
extern bool gg_swap; /* Swap variable set by wl_endian_probe */
#define htod32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))
#define htod16(i) (gg_swap ? bcmswap16(i) : (uint16)(i))
#define htod(i) (sizeof(i) == 2 ? htod16(i) : ((sizeof(i) == 4) ? htod32(i) : i))
#define dtoh64(i) (gg_swap ? bcmswap64(i) : (uint64)(i))
#define dtoh32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))

typedef enum stamon_mode {
	MODE_NOT_PROCESSED = 0,
	MODE_PROCESSING = 1,
	MODE_PROCESSED = 2
} stamon_mode_t;

/* List of all STA's to be monitored */
typedef struct stamon_sta_list {
	dll_t node;			/* Previous and Next node pointer */
	stamon_mode_t mode;		/* STA Monitoring mode for this STA */
	BCM_STAMON_STATUS status;	/* Status of the STA */
	bcm_stamon_prio_t priority;	/* Priority for monitoring the STA */
	struct ether_addr ea;		/* MAC address of the STA */
	uint8 rssi_idx;			/* Next index of RSSI array */
	int rssi[STAMON_MAX_RSSI_LEN];	/* RSSI's of the STA */
	chanspec_t chspec;		/* Chanspec of th STA (offchannel support) */
	bcm_offchan_sta_cbfn *offchancbfn;	/* Indicate the offchan STA is montiored now */
	void *arg;			/* argument for offchancbfn */
	uint32 stamon_count;
} stamon_sta_list_t;

/* Handle to the stamon module */
typedef struct stamon_handle {
	uint16 version;
	bcm_stamon_config_t stamon_config;	/* Stamon configuration information */
	int cur_sta_cnt;			/* Number of STA's in the stamon_sta_list_t */
	int is_timer_created;			/* Whether BCM_USCHED timer created or not */
	char ifname[STAMON_MAX_IFNAME_LEN];	/* Name of the interface */
	dll_t stalist_head;			/* Head node to the stamon_sta_list_t */
	bcm_usched_handle *usched_hndl;		/* Handle to the BCM_USCHED library */
} stamon_handle_t;

/* to print message level */
unsigned int g_stamon_debug_level;

/* For debug prints */
#define STAMON_DEBUG_ERROR		0x0001
#define STAMON_DEBUG_WARNING		0x0002
#define STAMON_DEBUG_INFO		0x0004
#define STAMON_DEBUG_DETAIL		0x0008

#define STAMON_PRINT(fmt, arg...) printf("STAMON >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg)

#define STAMON_ERROR(fmt, arg...) \
	do { if (g_stamon_debug_level & STAMON_DEBUG_ERROR) \
		STAMON_PRINT(fmt, ##arg); } while (0)

#define STAMON_WARNING(fmt, arg...) \
	do { if (g_stamon_debug_level & STAMON_DEBUG_WARNING) \
		STAMON_PRINT(fmt, ##arg); } while (0)

#define STAMON_INFO(fmt, arg...) \
	do { if (g_stamon_debug_level & STAMON_DEBUG_INFO) \
		STAMON_PRINT(fmt, ##arg); } while (0)

#define STAMON_DEBUG(fmt, arg...) \
	do { if (g_stamon_debug_level & STAMON_DEBUG_DETAIL) \
		STAMON_PRINT(fmt, ##arg); } while (0)

#define STAMON_DEFAULT_DEBUG_LEVEL	0x01	/* STAMON_DEBUG_ERROR */

#define RSSIF	"[%d %d %d %d %d]"
#define RSSI_TO_RSSIF(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4]

void bcm_stamon_timer_cb(bcm_usched_handle *usched_hndl, void *arg);

/* Helper function to print all STA's */
static void
stamon_print_all_stas(stamon_handle_t *hndl)
{
	dll_t *item_p;
	int i = 1;

	STAMON_DEBUG("%s: Total Monitored STAs=%d\n", hndl->ifname, hndl->cur_sta_cnt);

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;
		STAMON_DEBUG("%s: STA%d: "MACF" chanspec=0x%x Priority=%d RSSI="RSSIF"\n",
			hndl->ifname, i++, ETHER_TO_MACF(tmp->ea), tmp->chspec, tmp->priority,
			RSSI_TO_RSSIF(tmp->rssi));
	}
}

/* Gets the config val from NVARM, if not found applies the default value */
uint16
bcm_stamon_get_config_val_int(const char *c, uint16 def)
{
	uint16 ret = def;
#ifndef PCTARGET
	char *val;

	val = nvram_safe_get(c);
	if (val && (val[0] != '\0')) {
		ret = strtoul(val, NULL, 0);
	}
#endif // endif
	return ret;
}

/* Checks whether STA is already there in the list */
static int
bcm_stamon_sta_already_exists(stamon_handle_t* hndl, struct ether_addr* ea)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		if (!eacmp(&((stamon_sta_list_t*)item_p)->ea, ea)) {
			return 1;
		}
	}

	return 0;
}

/* Updates the mode from processed to not processed */
static void
bcm_stamon_update_mode_to_not_processed(stamon_handle_t* hndl)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *entry = (stamon_sta_list_t*)item_p;
		if (entry->mode != MODE_PROCESSING) {
			entry->mode = MODE_NOT_PROCESSED;
			STAMON_DEBUG("%s: For STA "MACF" Setting mode to MODE_NOT_PROCESSED(%d)\n",
				hndl->ifname, ETHER_TO_MACF(entry->ea), MODE_NOT_PROCESSED);
		}
	}
}

/* Add STA's to driver for monitoring */
static BCM_STAMON_STATUS
bcm_stamon_add_stas_to_driver(stamon_handle_t* hndl)
{
	dll_t *item_p;
	int ncount = 0;
	int loop = 1;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;
	wlc_stamon_sta_config_t stamon_cfg;
	chanspec_t last_chspec = 0;

	memset(&stamon_cfg, 0, sizeof(stamon_cfg));

	wl_endian_probe(hndl->ifname);
	stamon_cfg.cmd = htod(STAMON_CFG_CMD_ADD);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;
		if (tmp->mode == MODE_PROCESSING) {
			ncount++;
			tmp->stamon_count++;
		}
	}
	/* First, iterate the STA list for offchannel STA. If an offchannel STA
	 * is found add all the STAs with same chanspec. This improves
	 * probability for receving monitor information
	 */
	for (item_p = dll_head_p(&hndl->stalist_head); ncount < STAMON_MAX_STA_MONITORED &&
		!dll_end(&hndl->stalist_head, item_p); item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;

		STAMON_DEBUG("%s: Trying to add offchan STA "MACF" to driver for monitoring\n",
			hndl->ifname, ETHER_TO_MACF(tmp->ea));
		if (tmp->mode != MODE_NOT_PROCESSED) {
			STAMON_DEBUG("%s: Skip "MACF" as mode(%d) != MODE_NOT_PROCESSED(%d)\n",
				hndl->ifname, ETHER_TO_MACF(tmp->ea), tmp->mode,
				MODE_NOT_PROCESSED);
			continue;
		}

		if (!tmp->offchancbfn) {
			STAMON_DEBUG("%s: Skip "MACF" as it is not offchan STA\n", hndl->ifname,
				ETHER_TO_MACF(tmp->ea));
			continue;
		}

		if (tmp->stamon_count >= hndl->stamon_config.max_offchan_count) {
			STAMON_DEBUG("%s: Skip "MACF" as stamon_count(%d) >= max_offchan_count"
				"(%d)\n", hndl->ifname, ETHER_TO_MACF(tmp->ea), tmp->stamon_count,
				hndl->stamon_config.max_offchan_count);
			continue;
		}

		/* last_chspec holds chanspec of the offchannel STA last added to driver */
		if (last_chspec && last_chspec !=  tmp->chspec) {
			STAMON_DEBUG("%s: Skip "MACF" as last_chspec(0x%x) != STA chanspec(0x%x)\n",
				hndl->ifname, ETHER_TO_MACF(tmp->ea), last_chspec, tmp->chspec);
			continue;
		}

		last_chspec = stamon_cfg.chanspec = tmp->chspec;
		stamon_cfg.chanspec = htod32(stamon_cfg.chanspec);
		stamon_cfg.offchan_time = htod32(hndl->stamon_config.offchan_time);
		memcpy(&stamon_cfg.ea, &tmp->ea, sizeof(stamon_cfg.ea));
		STAMON_INFO("%s: Adding offchan STA "MACF" to driver. chspec=0x%x "
			"offchan_time=%d\n", hndl->ifname, ETHER_TO_MACF(tmp->ea),
			stamon_cfg.chanspec, stamon_cfg.offchan_time);

		tmp->offchancbfn(tmp->arg, &tmp->ea);
		tmp->stamon_count++;
		status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg,
			sizeof(stamon_cfg));
		if (status) {
			STAMON_ERROR("%s: Failed to add "MACF" to driver. wl error: %d\n",
				hndl->ifname, ETHER_TO_MACF(stamon_cfg.ea), status);
			status = BCM_STAMONE_WL;
			tmp->status = BCM_STAMONE_WL_ADD;
			return status;
		}
		STAMON_DEBUG("%s: For STA "MACF" Setting mode to MODE_PROCESSING(%d)\n",
			hndl->ifname, ETHER_TO_MACF(tmp->ea), MODE_PROCESSING);
		tmp->mode = MODE_PROCESSING;
		ncount++;
	}

start:
	/* Now, add only home channel STAs */
	for (item_p = dll_head_p(&hndl->stalist_head); ncount < STAMON_MAX_STA_MONITORED &&
		!dll_end(&hndl->stalist_head, item_p); item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;

		STAMON_DEBUG("%s: Trying to add onchan STA "MACF" to driver for monitoring\n",
			hndl->ifname, ETHER_TO_MACF(tmp->ea));
		if (tmp->mode != MODE_NOT_PROCESSED) {
			STAMON_DEBUG("%s: Skip "MACF" as mode(%d) != MODE_NOT_PROCESSED(%d)\n",
				hndl->ifname, ETHER_TO_MACF(tmp->ea), tmp->mode,
				MODE_NOT_PROCESSED);
			continue;
		}

		if (tmp->offchancbfn) {
			STAMON_DEBUG("%s: Skip "MACF" as it is offchan STA\n", hndl->ifname,
				ETHER_TO_MACF(tmp->ea));
			continue;
		}

		tmp->stamon_count++;
		stamon_cfg.chanspec = htod32(tmp->chspec);
		memcpy(&stamon_cfg.ea, &tmp->ea, sizeof(stamon_cfg.ea));
		STAMON_INFO("%s: Adding onchannel STA "MACF" to driver. chspec=0x%x\n",
			hndl->ifname, ETHER_TO_MACF(tmp->ea), stamon_cfg.chanspec);
		status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg,
			sizeof(stamon_cfg));
		if (status) {
			STAMON_ERROR("%s: Failed to add "MACF" to driver. wl error: %d\n",
				hndl->ifname, ETHER_TO_MACF(stamon_cfg.ea), status);
			status = BCM_STAMONE_WL;
			tmp->status = BCM_STAMONE_WL_ADD;
			return status;
		}
		STAMON_DEBUG("%s: For STA "MACF" Setting mode to MODE_PROCESSING(%d)\n",
			hndl->ifname, ETHER_TO_MACF(tmp->ea), MODE_PROCESSING);
		tmp->mode = MODE_PROCESSING;
		ncount++;
	}

	/* Loop once, if the number of STA's added are less after updating STA status */
	if (hndl->cur_sta_cnt > STAMON_MAX_STA_MONITORED &&
		ncount < STAMON_MAX_STA_MONITORED && loop) {
		loop = 0;
		bcm_stamon_update_mode_to_not_processed(hndl);
		goto start;
	}

	return status;
}

/* Get the average RSSI value */
static int
bcm_stamon_get_average_rssi(int *rssi)
{
	int tmp_rssi = 0, max_count = 0, i;

	for (i = 0; i < sizeof(rssi); i++) {
		if (STAMON_RSSI_VALID(rssi[i])) {
			tmp_rssi += rssi[i];
			max_count++;
		}
	}

	return ((max_count == 0) ? 0 : (tmp_rssi / max_count));
}

/* Gets all stamon stats from list for sending to user */
static bcm_stamon_list_info_t*
bcm_stamon_get_all_stamon_stats(stamon_handle_t* hndl)
{
	int i = 0, buflen;
	bcm_stamon_list_info_t *infobuf = NULL;
	dll_t *item_p;

	buflen = sizeof(bcm_stamon_list_info_t) + (sizeof(bcm_stamon_info_t) * hndl->cur_sta_cnt);
	infobuf = (bcm_stamon_list_info_t*)malloc(buflen);
	if (!infobuf) {
		STAMON_ERROR("%s: Failed to allocate memory for bcm_stamon_list_info_t\n",
			hndl->ifname);
		return NULL;
	}

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;

		memcpy(&infobuf->info[i].ea, &tmp->ea, sizeof(infobuf->info[i].ea));
		infobuf->info[i].status = tmp->status;
		infobuf->info[i].rssi = bcm_stamon_get_average_rssi(tmp->rssi);
		i++;
	}
	infobuf->list_info_len = i;

	return infobuf;
}

/* Get the stamon_sta_list_t for particular MAC */
static stamon_sta_list_t*
bcm_stamon_find_and_get_stainfo(stamon_handle_t *hndl, bcm_stamon_macinfo_t *info)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		if (!eacmp(&((stamon_sta_list_t*)item_p)->ea, &info->ea)) {
			return ((stamon_sta_list_t*)item_p);
		}
	}

	return NULL;
}

/* Delete a STA from driver */
static BCM_STAMON_STATUS
bcm_stamon_delete_from_driver(stamon_handle_t *hndl, struct ether_addr *ea)
{
	BCM_STAMON_STATUS status;
	wlc_stamon_sta_config_t stamon_cfg;

	memset(&stamon_cfg, 0, sizeof(stamon_cfg));

	wl_endian_probe(hndl->ifname);
	stamon_cfg.cmd = htod(STAMON_CFG_CMD_DEL);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);
	memcpy(&stamon_cfg.ea, ea, sizeof(stamon_cfg.ea));

	STAMON_DEBUG("%s: Deleting STA "MACF" from driver.\n", hndl->ifname,
		ETHER_TO_MACF(stamon_cfg.ea));
	status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg, sizeof(stamon_cfg));
	if (status) {
		STAMON_ERROR("%s: Failed to delete "MACF" from driver. wl error : %d\n",
			hndl->ifname, ETHER_TO_MACF(stamon_cfg.ea), status);
		status = BCM_STAMONE_WL;
	}

	return status;
}

/* Deletes the STA from driver as well as from local list */
static BCM_STAMON_STATUS
bcm_stamon_find_and_delete_entry(stamon_handle_t* hndl, bcm_stamon_macinfo_t *macinfo)
{
	dll_t *item_p, *next_p;

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = next_p) {
		next_p = dll_next_p(item_p);
		if (!eacmp(&((stamon_sta_list_t*)item_p)->ea, &macinfo->ea)) {
			/* If its in processing state, delete it from driver */
			if (((stamon_sta_list_t*)item_p)->mode == MODE_PROCESSING) {
				bcm_stamon_delete_from_driver(hndl,
					&((stamon_sta_list_t*)item_p)->ea);
			}
			dll_delete((dll_t*)item_p);
			free(item_p);
			STAMON_DEBUG("%s: Deleted STA "MACF" from stamonlib\n", hndl->ifname,
				ETHER_TO_MACF(macinfo->ea));
			hndl->cur_sta_cnt--;
			return BCM_STAMONE_OK;
		}
	}

	return BCM_STAMONE_NOT_FOUND;
}

static void
bcm_stamon_del_stas_from_driver(stamon_handle_t* hndl)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		if (((stamon_sta_list_t*)item_p)->mode == MODE_PROCESSED) {
			bcm_stamon_delete_from_driver(hndl, &((stamon_sta_list_t*)item_p)->ea);
		}
	}
	STAMON_DEBUG("%s: Deleted all processed STAs from driver\n", hndl->ifname);
}

/* Adds the STA into local list in a sorted manner(based on priority */
static BCM_STAMON_STATUS
bcm_stamon_add_sorted_entry(stamon_handle_t *hndl, stamon_sta_list_t *sta_info)
{
	dll_t *item_p, *prev = NULL;

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		if (sta_info->priority > ((stamon_sta_list_t*)item_p)->priority) {
			break;
		}
		prev = item_p;
	}

	if (prev == NULL) {
		dll_prepend(&hndl->stalist_head, (dll_t*)sta_info);
	} else {
		dll_insert((dll_t*)sta_info, prev);
	}
	STAMON_INFO("%s: Added STA "MACF" chspec=0x%x priority=%d to stamonlib.\n",
		hndl->ifname, ETHER_TO_MACF(sta_info->ea), sta_info->chspec, sta_info->priority);

	return BCM_STAMONE_OK;
}

/* Adds the STA to local list. Before adding checks whether STA already exists or not */
static BCM_STAMON_STATUS
bcm_stamon_find_and_add_entry(stamon_handle_t* hndl, bcm_stamon_macinfo_t *macinfo)
{
	stamon_sta_list_t *sta_info;

	if (bcm_stamon_sta_already_exists(hndl, &macinfo->ea)) {
		STAMON_DEBUG("%s: STA "MACF" already exists\n", hndl->ifname,
			ETHER_TO_MACF(macinfo->ea));
		return BCM_STAMONE_OK;
	}

	sta_info = (stamon_sta_list_t*)malloc(sizeof(*sta_info));
	if (!sta_info) {
		STAMON_ERROR("%s: Failed to allocate memory for STA : "MACF"\n", hndl->ifname,
			ETHER_TO_MACF(macinfo->ea));
		return BCM_STAMONE_MEMORY;
	}
	memset(sta_info, 0, sizeof(*sta_info));
	memcpy(&sta_info->ea, &macinfo->ea, sizeof(sta_info->ea));
	sta_info->priority = macinfo->priority;
	sta_info->chspec = macinfo->chspec;
	sta_info->offchancbfn = macinfo->cbfn;
	sta_info->arg = macinfo->arg;
	sta_info->stamon_count = 0;

	bcm_stamon_add_sorted_entry(hndl, sta_info);
	hndl->cur_sta_cnt++;

	return BCM_STAMONE_OK;
}

/* Delete all STA's from driver which are added for monitoring */
static BCM_STAMON_STATUS
bcm_stamon_delete_all_from_driver(stamon_handle_t *hndl, stamon_info_t *info)
{
	uint i;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;

	if (!info) {
		return status;
	}

	for (i = 0; i < info->count; i++) {
		status = bcm_stamon_delete_from_driver(hndl, &info->sta_data[i].ea);
	}
	STAMON_DEBUG("%s: Deleted all STAs from driver\n", hndl->ifname);

	return status;
}

/* Remove all the STA's from sta list as well as from driver */
static BCM_STAMON_STATUS
bcm_stamon_delete_all_stamon_stas(stamon_handle_t *hndl)
{
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;
	stamon_info_t *info = NULL;
	int buflen;
	dll_t *item_p, *next_p;

	/* Allocate memory to hold the STA's which are added to the driver for monitoring */
	buflen = sizeof(stamon_info_t) + (sizeof(stamon_data_t) * STAMON_MAX_STA_MONITORED);
	info = (stamon_info_t*)malloc(buflen);
	if (!info) {
		STAMON_ERROR("%s: Failed to allocate memory for stamon_info_t\n", hndl->ifname);
		return BCM_STAMONE_MEMORY;
	}
	memset(info, 0, sizeof(*info));

	/* Fill with all the STA's which are in processing mode, i.e added to driver for monitor */
	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = next_p) {
		next_p = dll_next_p(item_p);
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;
		if (tmp->mode == MODE_PROCESSING && info->count < STAMON_MAX_STA_MONITORED) {
			memcpy(&info->sta_data[info->count].ea, &tmp->ea,
				sizeof(info->sta_data[info->count].ea));
			info->count++;
		}
		dll_delete(item_p);
		free(item_p);
	}
	hndl->cur_sta_cnt = 0;

	status = bcm_stamon_delete_all_from_driver(hndl, info);
	if (info)
		free(info);

	if (hndl->is_timer_created) {
		STAMON_DEBUG("%s: No STA's to monitor. Delete the timer\n", hndl->ifname);
		bcm_usched_remove_timer(hndl->usched_hndl, bcm_stamon_timer_cb, hndl);
		hndl->is_timer_created = 0;
	}

	return status;
}

/* gets one stats from driver, updates the local structure and deletes STA from driver */
static BCM_STAMON_STATUS
bcm_stamon_get_stats_from_driver_and_update(stamon_handle_t *hndl, stamon_sta_list_t *entry,
	stamon_info_t *info, int buflen)
{
	int status;
	wlc_stamon_sta_config_t stamon_cfg;

	/* Initialize param for sending command */
	wl_endian_probe(hndl->ifname);
	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod(STAMON_CFG_CMD_GET_STATS);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);

	memset(info, 0, sizeof(*info));
	memcpy(&stamon_cfg.ea, &entry->ea, sizeof(stamon_cfg.ea));

	status = wl_iovar_getbuf(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg,
		sizeof(stamon_cfg), info, buflen);
	STAMON_DEBUG("%s: Collected stats from driver for "MACF". Possible RSSI=%d status=%d\n",
		hndl->ifname, ETHER_TO_MACF(entry->ea), info->sta_data[0].rssi, status);

	if (!status) {
		int idx = 0;
		if (sizeof(uint) == 4) {
			info->count = dtoh32(info->count);
			for (idx = 0; idx < info->count; idx++) {
				info->sta_data[idx].rssi = dtoh32(info->sta_data[idx].rssi);
			}
		} else {
			info->count = dtoh64(info->count);
			for (idx = 0; idx < info->count; idx++) {
				info->sta_data[idx].rssi = dtoh64(info->sta_data[idx].rssi);
			}
		}
	}

	if (status) {
		status = BCM_STAMONE_WL;
	}
	else if (info->count <= 0) {
		STAMON_ERROR("%s: Empty list from driver\n", hndl->ifname);
		status = BCM_STAMONE_WL;
	}
	else if (ETHER_ISNULLADDR(&info->sta_data[0].ea)) {
		STAMON_ERROR("%s: NULL STA address from driver\n", hndl->ifname);
		status = BCM_STAMONE_WL;
	} else if (STAMON_RSSI_VALID(info->sta_data[0].rssi)) {
		entry->rssi[entry->rssi_idx++] = info->sta_data[0].rssi;
		STAMON_DEBUG("%s: For STA "MACF" RSSI=%d\n", hndl->ifname,
			ETHER_TO_MACF(info->sta_data[0].ea), info->sta_data[0].rssi);
		/* Update rssi_idx */
		entry->rssi_idx = (entry->rssi_idx >= STAMON_MAX_RSSI_LEN) ? 0 : entry->rssi_idx;
		status = BCM_STAMONE_OK;
	} else {
		STAMON_DEBUG("%s: For STA "MACF" RSSI=0\n", hndl->ifname,
			ETHER_TO_MACF(info->sta_data[0].ea));
		status = BCM_STAMONE_INV_RSSI;
	}

	entry->status = status;

	if (!(entry->stamon_count % hndl->stamon_config.sta_load_freq)) {
		STAMON_DEBUG("%s: For STA "MACF" Setting mode to MODE_PROCESSED(%d)\n",
			hndl->ifname, ETHER_TO_MACF(info->sta_data[0].ea), MODE_PROCESSED);
		entry->mode = MODE_PROCESSED;
	}

	return status;
}

/* Get the stats from driver for all the STA's which are added to driver */
static BCM_STAMON_STATUS
bcm_stamon_get_stats_from_driver(stamon_handle_t *hndl)
{
	int buflen = 0;
	BCM_STAMON_STATUS ret = BCM_STAMONE_OK;
	dll_t *item_p;
	stamon_info_t *info = NULL;

	/* Allocated memory for output buffer to hold 1 STA */
	buflen = sizeof(stamon_info_t) + (sizeof(stamon_data_t) * 1);
	if (buflen < STAMON_MIN_DRIVER_BUFLEN)
		buflen = STAMON_MIN_DRIVER_BUFLEN;
	info = (stamon_info_t*)malloc(buflen);
	if (!info) {
		STAMON_ERROR("%s : Failed to allocate memory for stamon_info_t\n", hndl->ifname);
		return BCM_STAMONE_MEMORY;
	}

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;
		if (tmp->mode == MODE_PROCESSING) {
			ret = bcm_stamon_get_stats_from_driver_and_update(hndl, tmp, info, buflen);
		}
	}

	if (info)
		free(info);

	stamon_print_all_stas(hndl);

	return ret;
}

/* Callback function called from scheduler library */
void
bcm_stamon_timer_cb(bcm_usched_handle *usched_hndl, void *arg)
{
	stamon_handle_t *hndl = (stamon_handle_t*)arg;

	STAMON_DEBUG("%s : Timer Called\n", hndl->ifname);

	/* Get the STATS from driver */
	bcm_stamon_get_stats_from_driver(hndl);

	/* If there are only few number of STA's, After processing change the status to not
	 * processed to process it again
	 */
	if (hndl->cur_sta_cnt <= STAMON_MAX_STA_MONITORED)
		bcm_stamon_update_mode_to_not_processed(hndl);
	else
		bcm_stamon_del_stas_from_driver(hndl);

	/* Now Add the remaining STA's for monitoring */
	bcm_stamon_add_stas_to_driver(hndl);
}

/* Add timers to micro scheduler */
static int
bcm_stamon_add_timers(stamon_handle_t *hndl, unsigned long long timeout,
	bcm_usched_timerscbfn *cbfn)
{
	BCM_USCHED_STATUS ret = 0;

	ret = bcm_usched_add_timer(hndl->usched_hndl, timeout, 1, cbfn, hndl);
	if (ret != BCM_USCHEDE_OK) {
		STAMON_ERROR("%s: Failed to add timer. Error: %s\n", hndl->ifname,
			bcm_usched_strerror(ret));
		return BCM_STAMONE_USCHED;
	}

	return BCM_STAMONE_OK;
}

/* Enables STA monitoring feature */
static BCM_STAMON_STATUS
bcm_stamon_cmd_enable(stamon_handle_t *hndl)
{
	wlc_stamon_sta_config_t stamon_cfg;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;

	/* Enable STA monitor module in Firmware */
	wl_endian_probe(hndl->ifname);
	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod(STAMON_CFG_CMD_ENB);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);
	status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg, sizeof(stamon_cfg));

	if (status) {
		STAMON_ERROR("%s: Failed to enable stamon. wl error %d\n", hndl->ifname, status);
		status = BCM_STAMONE_WL;
	}

	return status;
}

/* Disbles STA monitoring feature */
static BCM_STAMON_STATUS
bcm_stamon_cmd_disable(stamon_handle_t *hndl)
{
	wlc_stamon_sta_config_t stamon_cfg;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;

	/* Disable STA monitor module in Firmware */
	wl_endian_probe(hndl->ifname);
	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = htod(STAMON_CFG_CMD_DSB);
	stamon_cfg.version = htod16(STAMON_STACONFIG_VER);
	stamon_cfg.length = htod16(STAMON_STACONFIG_LENGTH);
	status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg, sizeof(stamon_cfg));

	if (status) {
		STAMON_ERROR("%s: Failed to disable stamon. wl error %d\n", hndl->ifname, status);
		status = BCM_STAMONE_WL;
	}

	return status;
}

/* Process command ADD */
static BCM_STAMON_STATUS
bcm_stamon_cmd_add(stamon_handle_t* hndl, void *param)
{
	int i = 0;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;
	bcm_stamon_maclist_t *maclist = (bcm_stamon_maclist_t*)param;

	/* Check for input param */
	if (maclist == NULL || maclist->count == 0) {
		STAMON_ERROR("%s: Empty MAC list sent\n", hndl->ifname);
		return BCM_STAMONE_NULL_PARAM;
	}

	/* From the input param add the STA's to local list */
	for (i = 0; i < maclist->count; i ++) {
		STAMON_DEBUG("%s: Adding "MACF" with priority=%d chanspec=0x%x\n", hndl->ifname,
			ETHER_TO_MACF(maclist->macinfo[i].ea), maclist->macinfo[i].priority,
			maclist->macinfo[i].chspec);
		status = bcm_stamon_find_and_add_entry(hndl, &maclist->macinfo[i]);
		if (status != BCM_STAMONE_OK)
			return status;
	}

	/* Check if the Timer created for station monitoring, if not create it */
	if (!hndl->is_timer_created) {
		STAMON_DEBUG("%s: Creating the timer\n", hndl->ifname);
		if (bcm_stamon_add_timers(hndl,
			STAMON_MSEC_USEC((unsigned long long)hndl->stamon_config.interval),
			bcm_stamon_timer_cb) != BCM_STAMONE_OK) {
			return BCM_STAMONE_USCHED;
		}
		hndl->is_timer_created = 1;
	}

	return status;
}

/* Process command Delete */
static BCM_STAMON_STATUS
bcm_stamon_cmd_delete(stamon_handle_t* hndl, void *param)
{
	int i = 0;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;
	bcm_stamon_maclist_t *maclist = (bcm_stamon_maclist_t*)param;

	/* If the param is delete all stamon stats */
	if (maclist == NULL || maclist->count == 0) {
		return bcm_stamon_delete_all_stamon_stas(hndl);
	}

	/* Else delete only the STA list passed in param */
	for (i = 0; i < maclist->count; i ++) {
		STAMON_DEBUG("%s: Deleting "MACF"\n", hndl->ifname,
			ETHER_TO_MACF(maclist->macinfo[i].ea));
		status = bcm_stamon_find_and_delete_entry(hndl, &maclist->macinfo[i]);
		if (status != BCM_STAMONE_OK)
			return status;
	}

	/* If there are no STA's present after delete remove the timer created */
	if (hndl->cur_sta_cnt <= 0 && hndl->is_timer_created) {
		STAMON_DEBUG("%s: No STAs to monitor. Delete the timer\n", hndl->ifname);
		bcm_usched_remove_timer(hndl->usched_hndl, bcm_stamon_timer_cb, hndl);
		hndl->is_timer_created = 0;
	}

	return status;
}

/* Process command Get stamon stats */
static BCM_STAMON_STATUS
bcm_stamon_cmd_get(stamon_handle_t* hndl, void *param, void **outbuf)
{
	int i, buflen;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;
	bcm_stamon_maclist_t *maclist = (bcm_stamon_maclist_t*)param;
	bcm_stamon_list_info_t *infobuf = NULL;
	stamon_sta_list_t *tmp = NULL;

	/* If the param is empty get all stamon stats */
	if (maclist == NULL || maclist->count == 0) {
		STAMON_DEBUG("Empty MAC List. Getting stats for all STAs\n");
		infobuf = bcm_stamon_get_all_stamon_stats(hndl);
		if (!infobuf)
			status = BCM_STAMONE_MEMORY;
		goto done;
	}

	/* Else get only the requested STA stamon stats. Allocate memory for output buffer */
	buflen = sizeof(bcm_stamon_list_info_t) + (sizeof(bcm_stamon_info_t) * maclist->count);
	infobuf = (bcm_stamon_list_info_t*)malloc(buflen);
	if (!infobuf) {
		STAMON_ERROR("%s: Failed to allocate memory for bcm_stamon_list_info_t\n",
			hndl->ifname);
		*outbuf = NULL;
		return BCM_STAMONE_MEMORY;
	}

	/* Copy the stats to output buffer */
	for (i = 0; i < maclist->count; i++) {
		memcpy(&infobuf->info[i].ea, &maclist->macinfo[i].ea,
			sizeof(infobuf->info[i].ea));
		tmp = bcm_stamon_find_and_get_stainfo(hndl, &maclist->macinfo[i]);
		if (tmp) {
			infobuf->info[i].status = tmp->status;
			infobuf->info[i].rssi = bcm_stamon_get_average_rssi(tmp->rssi);
		} else {
			infobuf->info[i].status = BCM_STAMONE_NOT_FOUND;
		}
		STAMON_INFO("%s: For "MACF" RSSI=%d chspec=0x%x status=%d\n", hndl->ifname,
			ETHER_TO_MACF(maclist->macinfo[i].ea), infobuf->info[i].rssi, tmp->chspec,
			infobuf->info[i].status);
	}
	infobuf->list_info_len = i;

done:
	stamon_print_all_stas(hndl);

	*outbuf = (void*)infobuf;

	return status;
}

/* One function for all the STA monitoring functionality */
BCM_STAMON_STATUS
bcm_stamon_command(bcm_stamon_handle *handle, bcm_stamon_cmd_t cmd, void *param, void **outbuf)
{
	stamon_handle_t *hndl = (stamon_handle_t*)handle;
	BCM_STAMON_STATUS status = BCM_STAMONE_FAIL;

	/* Check for handle */
	if (!hndl) {
		STAMON_ERROR("Invalid Handle (null)\n");
		return BCM_STAMONE_INV_HDL;
	}

	switch (cmd) {
		case BCM_STAMON_CMD_ENB:
			STAMON_INFO("%s: Running command Enable stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_enable(hndl);
			break;

		case BCM_STAMON_CMD_DSB:
			STAMON_INFO("%s: Running command Disable stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_disable(hndl);
			break;

		case BCM_STAMON_CMD_ADD:
			STAMON_INFO("%s: Running command Add to stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_add(hndl, param);
			break;

		case BCM_STAMON_CMD_DEL:
			STAMON_INFO("%s: Running command Delete from stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_delete(hndl, param);
			break;

		case BCM_STAMON_CMD_GET:
			STAMON_INFO("%s: Running command Get stamon stats\n", hndl->ifname);
			status = bcm_stamon_cmd_get(hndl, param, outbuf);
			break;

		default:
			STAMON_ERROR("Invalid command %d\n", cmd);
			status = BCM_STAMONE_INV_CMD;
			break;
	}

	return status;
}

/* Initialize the stamon module. This handle should be used in all the functions */
bcm_stamon_handle*
bcm_stamon_init(char *ifname)
{
	stamon_handle_t *hndl;
	bcm_stamon_config_t temp;
	char *str;
	int num = 0;
	char wl_name[STAMON_MAX_IFNAME_LEN];
	char prefix[STAMON_MAX_IFNAME_LEN] = {0};
	char tmp[100];
	int bandtype;
	int ret;

	g_stamon_debug_level = bcm_stamon_get_config_val_int("bcm_stamon_debug_level",
		STAMON_DEFAULT_DEBUG_LEVEL);

	/* Allocate handle */
	hndl = (stamon_handle_t*)malloc(sizeof(*hndl));
	if (!hndl) {
		STAMON_ERROR("Failed to allocate memory for stamon_handle_t\n");
		return NULL;
	}

	memset(hndl, 0, sizeof(*hndl));
	hndl->version = BCM_STAMON_VERSION;
	strncpy(hndl->ifname, ifname, sizeof(hndl->ifname)-1);

	ret = osifname_to_nvifname(ifname, wl_name, sizeof(wl_name));
	if (ret != 0) {
		STAMON_ERROR("%s: Not a wireless interface. ret=%d\n", ifname, ret);
		return NULL;
	}
	make_wl_prefix(prefix, sizeof(prefix), 1, wl_name);

	/* read stamon configuration from NVRAM for this interface */
	str = nvram_safe_get(strcat_r(prefix, BCM_NVRAM_STAMON_CONFIG, tmp));
	if (str[0] != '\0') {
		num = sscanf(str, "%u %u %u %u",
			&temp.interval,
			&temp.max_offchan_count,
			&temp.sta_load_freq,
			&temp.offchan_time);
		if (num == 4) {
			memcpy(&hndl->stamon_config, &temp,
				sizeof(hndl->stamon_config));
		} else {
			STAMON_WARNING("Invalid NVRAM : %s=[%s]\n",
				BCM_NVRAM_STAMON_CONFIG, str);
		}
	}
	if (!str || num != 4) {
		/* Get configured phy type */
		wl_ioctl(ifname, WLC_GET_BAND, &bandtype, sizeof(bandtype));
		if (bandtype == WLC_BAND_5G) {
			hndl->stamon_config.interval = STAMON_DEFAULT_MONITOR_TIMEOUT_5G;
			hndl->stamon_config.max_offchan_count = STAMON_MAX_OFFCHAN_COUNT_5G;
			hndl->stamon_config.sta_load_freq = STAMON_DEFAULT_STA_LOAD_FREQ_5G;
			hndl->stamon_config.offchan_time = STAMON_DEFAULT_OFFCHAN_TIME_5G;
		} else {
			hndl->stamon_config.interval = STAMON_DEFAULT_MONITOR_TIMEOUT_2G;
			hndl->stamon_config.max_offchan_count = STAMON_MAX_OFFCHAN_COUNT_2G;
			hndl->stamon_config.sta_load_freq = STAMON_DEFAULT_STA_LOAD_FREQ_2G;
			hndl->stamon_config.offchan_time = STAMON_DEFAULT_OFFCHAN_TIME_2G;
		}
	}
	STAMON_DEBUG("%s: Stamon config. interval=%d max_offchan_count=%d sta_load_freq=%d "
		"offchan_time=%d\n", hndl->ifname, hndl->stamon_config.interval,
		hndl->stamon_config.max_offchan_count, hndl->stamon_config.sta_load_freq,
		hndl->stamon_config.offchan_time);

	/* Initialize the stalist head */
	dll_init(&hndl->stalist_head);

	/* Get micro scheduler handle for timer operation */
	if ((hndl->usched_hndl = bcm_usched_init()) == NULL) {
		STAMON_ERROR("%s: Failed to get BCM_USCHED handle\n", ifname);
		free(hndl);
		return NULL;
	}

	STAMON_DEBUG("%s: Initialized successfully\n", hndl->ifname);

	return (bcm_stamon_handle*)hndl;
}

/* DeInitialize the STA monitoring module. After deinitialize the handle will be invalid */
BCM_STAMON_STATUS
bcm_stamon_deinit(bcm_stamon_handle *handle)
{
	stamon_handle_t *hndl = (stamon_handle_t*)handle;

	/* Check for handle */
	if (!hndl) {
		STAMON_ERROR("Invalid Handle\n");
		return BCM_STAMONE_INV_HDL;
	}

	STAMON_DEBUG("%s: Deinitializing\n", hndl->ifname);
	bcm_stamon_delete_all_stamon_stas(hndl);
	bcm_usched_stop(hndl->usched_hndl);
	bcm_usched_deinit(hndl->usched_hndl);
	free(hndl);
	hndl = NULL;

	return BCM_STAMONE_OK;
}

/* Return error string for the error code */
const char*
bcm_stamon_strerror(BCM_STAMON_STATUS errorcode)
{
	switch (errorcode) {
		case BCM_STAMONE_OK:
			return STAMON_ERROR_PREFIX"Success";

		case BCM_STAMONE_FAIL:
			return STAMON_ERROR_PREFIX"Failed";

		case BCM_STAMONE_INV_HDL:
			return STAMON_ERROR_PREFIX"Invalid Handle";

		case BCM_STAMONE_INV_CMD:
			return STAMON_ERROR_PREFIX"Invalid Stamon Command";

		case BCM_STAMONE_MEMORY:
			return STAMON_ERROR_PREFIX"Memory Allocation Failed";

		case BCM_STAMONE_WL:
			return STAMON_ERROR_PREFIX"WL Error";

		case BCM_STAMONE_WL_ADD:
			return STAMON_ERROR_PREFIX"Failed to Add to Driver";

		case BCM_STAMONE_NOT_FOUND:
			return STAMON_ERROR_PREFIX"MAC Not Found";

		case BCM_STAMONE_NULL_PARAM:
			return STAMON_ERROR_PREFIX"NULL Param Passed";

		case BCM_STAMONE_INV_RSSI:
			return STAMON_ERROR_PREFIX"RSSI is Invalid";

		case BCM_STAMONE_USCHED:
			return STAMON_ERROR_PREFIX"Error From BCM USCHED";

		default:
			return STAMON_ERROR_PREFIX"Unknown Error";
	}
}
