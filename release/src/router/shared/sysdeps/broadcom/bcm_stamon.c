/*
 * Broadcom STA monitor implementation
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: bcm_stamon.c 623877 2016-03-09 13:21:51Z $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <wlioctl.h>
#include <wlioctl_utils.h>
#include <wlutils.h>
#include <shutils.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include <bcmnvram.h>

#include "bcm_stamon.h"
#include "bcm_usched.h"

#define STAMON_ERROR_PREFIX	"BCMSTAMON - "

#define STAMON_MAX_IFNAME_LEN		16	/* Maximum length of interface name */
#define STAMON_MAX_RSSI_LEN		5	/* Number of RSSI counts to be stored */
#define STAMON_MAX_RSSI_AVG		5	/* Number of RSSI's to be considered for average */
#define STAMON_MAX_STA_MONITORED	4	/* Max number of STA monitored at one instance */
#define STAMON_DEFAULT_MONITOR_TIMEOUT	1	/* In seconds */
#define STAMON_MIN_DRIVER_BUFLEN	256	/* Minimum Buffer length required for driver */

#define STAMON_IOVAR_NAME	"sta_monitor" /* Name of the IOVAR for sta monitoring */

/* Some utility macros */
#define STAMON_SEC_MICROSEC(x)	((x) * 1000 * 1000)
#define STAMON_RSSI_VALID(x)	((x) < (0) && (x) > (-125))

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
} stamon_sta_list_t;

/* Handle to the stamon module */
typedef struct stamon_handle {
	uint16 version;
	uint16 stamon_get_interval;		/* Interval between each GET command */
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

#define STAMON_DEFAULT_DEBUG_LEVEL	0xff

#define RSSIF	"[%d %d %d %d %d]"
#define RSSI_TO_RSSIF(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4]

void bcm_stamon_timer_cb(bcm_usched_handle *usched_hndl, void *arg);

/* Helper function to print all STA's */
static void
stamon_print_all_stas(stamon_handle_t *hndl)
{
	dll_t *item_p;
	int i = 1;

	STAMON_DEBUG("Number of STA's : %d\n", hndl->cur_sta_cnt);

	for (item_p = dll_head_p(&hndl->stalist_head); !dll_end(&hndl->stalist_head, item_p);
		item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;
		STAMON_DEBUG("STA%d MAC - "MACF" Priority - %d and RSSI - "RSSIF"\n", i++,
			ETHER_TO_MACF(tmp->ea), tmp->priority, RSSI_TO_RSSIF(tmp->rssi));
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
	if (val)
		ret = strtoul(val, NULL, 0);
#endif
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
		if (((stamon_sta_list_t*)item_p)->mode != MODE_PROCESSING) {
			((stamon_sta_list_t*)item_p)->mode = MODE_NOT_PROCESSED;
		}
	}
	STAMON_DEBUG("%s - Updated the status to not processed\n", hndl->ifname);
}

/* Add STA's to driver for monitoring */
static BCM_STAMON_STATUS
bcm_stamon_add_stas_to_driver(stamon_handle_t* hndl)
{
	dll_t *item_p;
	int ncount = 0;
	BCM_STAMON_STATUS status = BCM_STAMONE_OK;
	wlc_stamon_sta_config_t stamon_cfg;

	stamon_cfg.cmd = STAMON_CFG_CMD_ADD;

start:
	for (item_p = dll_head_p(&hndl->stalist_head); ncount < STAMON_MAX_STA_MONITORED &&
		!dll_end(&hndl->stalist_head, item_p); item_p = dll_next_p(item_p)) {
		stamon_sta_list_t *tmp = (stamon_sta_list_t*)item_p;

		if (tmp->mode == MODE_NOT_PROCESSED) {
			memcpy(&stamon_cfg.ea, &tmp->ea, sizeof(stamon_cfg.ea));
			STAMON_DEBUG("%s - Adding STA :"MACF" to driver\n", hndl->ifname,
				ETHER_TO_MACF(tmp->ea));
			status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg,
				sizeof(stamon_cfg));
			if (status) {
				STAMON_ERROR("%s : Failed to add "MACF" address wl error : %d\n",
					hndl->ifname, ETHER_TO_MACF(stamon_cfg.ea), status);
				status = BCM_STAMONE_WL;
				tmp->status = BCM_STAMONE_WL_ADD;
				return status;
			}
			tmp->mode = MODE_PROCESSING;
			ncount++;
		}
	}

	/* If the number of STA's added are less */
	if (hndl->cur_sta_cnt > STAMON_MAX_STA_MONITORED && ncount < STAMON_MAX_STA_MONITORED) {
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
		STAMON_ERROR("%s - Failed to allocate memory\n", hndl->ifname);
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

	stamon_cfg.cmd = STAMON_CFG_CMD_DEL;
	memcpy(&stamon_cfg.ea, ea, sizeof(stamon_cfg.ea));

	status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg, sizeof(stamon_cfg));
	if (status) {
		STAMON_ERROR("%s : Failed to delete "MACF" address wl error : %d\n",
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
			STAMON_DEBUG("Deleted "MACF" STA\n", ETHER_TO_MACF(macinfo->ea));
			hndl->cur_sta_cnt--;
			return BCM_STAMONE_OK;
		}
	}

	return BCM_STAMONE_NOT_FOUND;
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

	return BCM_STAMONE_OK;
}

/* Adds the STA to local list. Before adding checks whether STA already exists or not */
static BCM_STAMON_STATUS
bcm_stamon_find_and_add_entry(stamon_handle_t* hndl, bcm_stamon_macinfo_t *macinfo)
{
	stamon_sta_list_t *sta_info;

	if (bcm_stamon_sta_already_exists(hndl, &macinfo->ea)) {
		STAMON_DEBUG("%s - STA "MACF" Already exists\n", hndl->ifname,
			ETHER_TO_MACF(macinfo->ea));
		return BCM_STAMONE_OK;
	}

	sta_info = (stamon_sta_list_t*)malloc(sizeof(*sta_info));
	if (!sta_info) {
		STAMON_ERROR("%s - Failed to allocate memory for STA : "MACF"\n", hndl->ifname,
			ETHER_TO_MACF(macinfo->ea));
		return BCM_STAMONE_MEMORY;
	}
	memset(sta_info, 0, sizeof(*sta_info));
	memcpy(&sta_info->ea, &macinfo->ea, sizeof(sta_info->ea));
	sta_info->priority = macinfo->priority;

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
		STAMON_ERROR("%s - Failed to allocate memory\n", hndl->ifname);
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
	STAMON_DEBUG("%s - Deleted All STA's\n", hndl->ifname);

	if (hndl->is_timer_created) {
		STAMON_DEBUG("%s - No STA's so delete the timer\n", hndl->ifname);
		bcm_usched_remove_timer(hndl->usched_hndl, bcm_stamon_timer_cb, hndl);
		hndl->is_timer_created = 0;
	}

	return status;
}

/* gets one stats from driver, updates the local structure and deletes STA from driver */
static BCM_STAMON_STATUS
bcm_stamon_get_stats_from_driver_and_update(stamon_handle_t *hndl, stamon_sta_list_t *tmp,
	stamon_info_t *info, int buflen)
{
	int status;
	wlc_stamon_sta_config_t stamon_cfg;

	/* Initialize param for sending command */
	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = STAMON_CFG_CMD_GET_STATS;

	memset(info, 0, sizeof(*info));
	memcpy(&stamon_cfg.ea, &tmp->ea, sizeof(stamon_cfg.ea));

	status = wl_iovar_getbuf(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg,
		sizeof(stamon_cfg), info, buflen);
	if (status) {
		STAMON_ERROR("%s : Failed to get status wl error : %d\n",
			hndl->ifname, status);
		status = BCM_STAMONE_WL;
	}
	else if (info->count <= 0) {
		STAMON_ERROR("%s : Empty list from driver\n", hndl->ifname);
		status = BCM_STAMONE_WL;
	}
	else if (ETHER_ISNULLADDR(&info->sta_data[0].ea)) {
		STAMON_ERROR("%s : NULL address from driver\n", hndl->ifname);
		status = BCM_STAMONE_WL;
	} else if (STAMON_RSSI_VALID(info->sta_data[0].rssi)) {
		tmp->rssi[tmp->rssi_idx++] = info->sta_data[0].rssi;
		STAMON_DEBUG("For STA : "MACF" RSSI is : %d\n",
		ETHER_TO_MACF(info->sta_data[0].ea), info->sta_data[0].rssi);
		/* Update rssi_idx */
		tmp->rssi_idx =
			(tmp->rssi_idx >= STAMON_MAX_RSSI_LEN) ? 0 :
			tmp->rssi_idx;
		status = BCM_STAMONE_OK;
	} else
		status = BCM_STAMONE_INV_RSSI;

	tmp->mode = MODE_PROCESSED;
	tmp->status = status;

	bcm_stamon_delete_from_driver(hndl, &stamon_cfg.ea);

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
		STAMON_ERROR("%s : Failed to allocate memory\n", hndl->ifname);
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

	/* Now Add the remaining STA's for monitoring */
	bcm_stamon_add_stas_to_driver(hndl);
}

/* Add timers to micro scheduler */
static int
bcm_stamon_add_timers(stamon_handle_t *hndl, unsigned long timeout, bcm_usched_timerscbfn *cbfn)
{
	BCM_USCHED_STATUS ret = 0;

	ret = bcm_usched_add_timer(hndl->usched_hndl, timeout, 1, cbfn, hndl);
	if (ret != BCM_USCHEDE_OK) {
		STAMON_ERROR("Failed to add timer. Error : %s\n", bcm_usched_strerror(ret));
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
	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = STAMON_CFG_CMD_ENB;
	status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg, sizeof(stamon_cfg));

	if (status) {
		STAMON_ERROR("%s : Failed to ENABLE stamon wl error : %d\n",
			hndl->ifname, status);
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
	memset(&stamon_cfg, 0, sizeof(stamon_cfg));
	stamon_cfg.cmd = STAMON_CFG_CMD_DSB;
	status = wl_iovar_set(hndl->ifname, STAMON_IOVAR_NAME, &stamon_cfg, sizeof(stamon_cfg));

	if (status) {
		STAMON_ERROR("%s : Failed to Disable stamon wl error : %d\n",
			hndl->ifname, status);
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
		STAMON_ERROR("%s - Empty MAC list sent\n", hndl->ifname);
		return BCM_STAMONE_NULL_PARAM;
	}

	/* From the input param add the STA's to local list */
	for (i = 0; i < maclist->count; i ++) {
		STAMON_DEBUG("%s - Adding "MACF" MAC with %d priority\n", hndl->ifname,
			ETHER_TO_MACF(maclist->macinfo[i].ea), maclist->macinfo[i].priority);
		status = bcm_stamon_find_and_add_entry(hndl, &maclist->macinfo[i]);
		if (status != BCM_STAMONE_OK)
			return status;
	}

	/* Check if the Timer created for station monitoring, if not create it */
	if (!hndl->is_timer_created) {
		STAMON_DEBUG("%s - Creating the timer\n", hndl->ifname);
		if (bcm_stamon_add_timers(hndl, STAMON_SEC_MICROSEC(hndl->stamon_get_interval),
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
		STAMON_DEBUG("%s - Deleting "MACF" MAC\n", hndl->ifname,
			ETHER_TO_MACF(maclist->macinfo[i].ea));
		status = bcm_stamon_find_and_delete_entry(hndl, &maclist->macinfo[i]);
		if (status != BCM_STAMONE_OK)
			return status;
	}

	/* If there are no STA's present after delete remove the timer created */
	if (hndl->cur_sta_cnt <= 0 && hndl->is_timer_created) {
		STAMON_DEBUG("%s - No STA's so delete the timer\n", hndl->ifname);
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
		STAMON_DEBUG("Empty MAC List, so Get All\n");
		infobuf = bcm_stamon_get_all_stamon_stats(hndl);
		if (!infobuf)
			status = BCM_STAMONE_MEMORY;
		goto done;
	}

	/* Else get only the requested STA stamon stats. Allocate memory for output buffer */
	buflen = sizeof(bcm_stamon_list_info_t) + (sizeof(bcm_stamon_info_t) * maclist->count);
	infobuf = (bcm_stamon_list_info_t*)malloc(buflen);
	if (!infobuf) {
		STAMON_ERROR("%s - Failed to allocate memory\n", hndl->ifname);
		*outbuf = NULL;
		return BCM_STAMONE_MEMORY;
	}

	/* Copy the stats to output buffer */
	for (i = 0; i < maclist->count; i++) {
		STAMON_DEBUG("%s - Getting "MACF" MAC\n", hndl->ifname,
			ETHER_TO_MACF(maclist->macinfo[i].ea));
		memcpy(&infobuf->info[i].ea, &maclist->macinfo[i].ea,
			sizeof(infobuf->info[i].ea));
		tmp = bcm_stamon_find_and_get_stainfo(hndl, &maclist->macinfo[i]);
		if (tmp) {
			infobuf->info[i].status = tmp->status;
			infobuf->info[i].rssi = bcm_stamon_get_average_rssi(tmp->rssi);
		} else
			infobuf->info[i].status = BCM_STAMONE_NOT_FOUND;
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
		STAMON_ERROR("Invalid Handle\n");
		return BCM_STAMONE_INV_HDL;
	}

	switch (cmd) {
		case BCM_STAMON_CMD_ENB:
			STAMON_INFO("%s : Command Enable Stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_enable(hndl);
			break;

		case BCM_STAMON_CMD_DSB:
			STAMON_INFO("%s : Command Disable Stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_disable(hndl);
			break;

		case BCM_STAMON_CMD_ADD:
			STAMON_INFO("%s : Command Add to Stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_add(hndl, param);
			break;

		case BCM_STAMON_CMD_DEL:
			STAMON_INFO("%s : Command Delete from Stamon\n", hndl->ifname);
			status = bcm_stamon_cmd_delete(hndl, param);
			break;

		case BCM_STAMON_CMD_GET:
			STAMON_INFO("%s : Command Get Stamon Stats\n", hndl->ifname);
			status = bcm_stamon_cmd_get(hndl, param, outbuf);
			break;

		default:
			STAMON_ERROR("Command Not Found\n");
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

	/* Allocate handle */
	hndl = (stamon_handle_t*)malloc(sizeof(*hndl));
	if (!hndl) {
		STAMON_ERROR("Failed to allocate memory for stamon_handle_t\n");
		return NULL;
	}

	memset(hndl, 0, sizeof(*hndl));
	hndl->version = BCM_STAMON_VERSION;
	strncpy(hndl->ifname, ifname, sizeof(hndl->ifname)-1);

	hndl->stamon_get_interval = bcm_stamon_get_config_val_int("bcm_stamon_get_interval",
		STAMON_DEFAULT_MONITOR_TIMEOUT);
	if (hndl->stamon_get_interval <= 0)
		hndl->stamon_get_interval = STAMON_DEFAULT_MONITOR_TIMEOUT;

	g_stamon_debug_level = bcm_stamon_get_config_val_int("bcm_stamon_debug_level",
		STAMON_DEFAULT_DEBUG_LEVEL);

	/* Initialize the stalist head */
	dll_init(&hndl->stalist_head);

	/* Get micro scheduler handle for timer operation */
	if ((hndl->usched_hndl = bcm_usched_init()) == NULL) {
		STAMON_ERROR("%s - Failed to get BCM_USCHED handle\n", ifname);
		free(hndl);
		return NULL;
	}

	STAMON_DEBUG("Initialized successfully\n");

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

	bcm_stamon_delete_all_stamon_stas(hndl);
	bcm_usched_stop(hndl->usched_hndl);
	bcm_usched_deinit(hndl->usched_hndl);
	free(hndl);
	hndl = NULL;
	STAMON_DEBUG("Deinitialized successfully\n");

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
