/*
 * Broadcom STA monitor include file
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: bcm_stamon.h 776562 2019-07-02 10:32:53Z $
 */

#ifndef _BCM_STAMON_H_
#define _BCM_STAMON_H_

#include "typedefs.h"
#include "bcmutils.h"

#define BCM_STAMON_VERSION			1
#define STAMON_DEFAULT_MONITOR_TIMEOUT_2G	200  /* In milliseconds for 2G */
#define STAMON_DEFAULT_STA_LOAD_FREQ_2G		1    /* Default sta load frequency for 2G */
#define STAMON_MAX_OFFCHAN_COUNT_2G		1000 /* Max count off-channel monitoring for 2G */
#define STAMON_DEFAULT_OFFCHAN_TIME_2G		20   /* off-channel time in ms for 2G */
#define STAMON_DEFAULT_MONITOR_TIMEOUT_5G	200  /* In milliseconds for 5G */
#define STAMON_DEFAULT_STA_LOAD_FREQ_5G		5    /* Default sta load frequency for 5G */
#define STAMON_MAX_OFFCHAN_COUNT_5G		1000 /* Max count off-channel monitoring for 5G */
#define STAMON_DEFAULT_OFFCHAN_TIME_5G		10   /* off-channel time in ms for 5G */
#define BCM_NVRAM_STAMON_CONFIG         "bcm_stamon_config"

/* Define error codes */
#define BCM_STAMONE_OK			0
#define BCM_STAMONE_FAIL		-1
#define BCM_STAMONE_INV_HDL		-2
#define BCM_STAMONE_INV_CMD		-3
#define BCM_STAMONE_MEMORY		-4
#define BCM_STAMONE_WL			-5
#define BCM_STAMONE_WL_ADD		-6
#define BCM_STAMONE_NOT_FOUND		-7
#define BCM_STAMONE_NULL_PARAM		-8
#define BCM_STAMONE_INV_RSSI		-9
#define BCM_STAMONE_USCHED		-10
#define BCM_STAMONE_VERSION_ERROR	-11

typedef int BCM_STAMON_STATUS;
/* Callback function for offchannel STA */
typedef void bcm_offchan_sta_cbfn(void *arg, struct ether_addr *ea);

/* Command types for the STA monitoring module */
typedef enum bcm_stamon_cmd {
	BCM_STAMON_CMD_ENB = 0,	/* To Enable STA monitoring */
	BCM_STAMON_CMD_DSB = 1,	/* To disable STA monitoring */
	BCM_STAMON_CMD_ADD = 2,	/* To Add STA's for monitoring */
	BCM_STAMON_CMD_DEL = 3,	/* To delete STA's from monitoring */
	BCM_STAMON_CMD_GET = 4	/* To get STA's monitored stats */
} bcm_stamon_cmd_t;

/* Priority for STA monitoring */
typedef enum bcm_stamon_prio {
	BCM_STAMON_PRIO_LOW		= 0,
	BCM_STAMON_PRIO_MEDIUM		= 1,
	BCM_STAMON_PRIO_HIGH		= 2,
	BCM_STAMON_PRIO_VERYHIGH	= 3
} bcm_stamon_prio_t;

typedef struct bcm_stamon_macinfo {
	bcm_stamon_prio_t priority;	/* Priority for monitoring the STA */
	struct ether_addr ea;		/* MAC address of the STA */
	chanspec_t chspec;		/* Chanspec of th STA (offchannel support) */
	bcm_offchan_sta_cbfn *cbfn;	/* Indicate this offchan STA is montiored now */
	void *arg;
} bcm_stamon_macinfo_t;

/* For list of mac address */
typedef struct bcm_stamon_maclist {
	uint count;				/* Number of STA's */
	bcm_stamon_macinfo_t macinfo[1];	/* Variable length array of MAC info */
} bcm_stamon_maclist_t;

/* For individual STA's info */
typedef struct bcm_stamon_info {
	BCM_STAMON_STATUS status;	/* Error while doing STA monitoring */
	struct ether_addr ea;		/* MAC address of the STA */
	int32 rssi;			/* RSSI of the STA */
} bcm_stamon_info_t;

/* Output structure for get stamon details */
typedef struct bcm_stamon_info_list {
	uint list_info_len;		/* Length of bcm_stamon_info_t */
	bcm_stamon_info_t info[1];	/* stamon info */
} bcm_stamon_list_info_t;

/* Stamon configuration information */
typedef struct bcm_stamon_config {
	unsigned int interval;
	unsigned int max_offchan_count;
	unsigned int sta_load_freq;
	unsigned int offchan_time;
} bcm_stamon_config_t;

typedef void bcm_stamon_handle;

/**
 * Initialize the stamon module. This handle should be used in all the functions.
 *
 * @param ifname	Name of the interface on which the STA monitoring will happen. The ifname is
 *			because user can call this function to multiple times to monitor on other
 *			interfaces
 *
 * @return		handle to the stamon module
 */
bcm_stamon_handle* bcm_stamon_init(char *ifname);

/**
 * DeInitialize the STA monitoring module. After deinitialize the handle will be invalid
 *
 * @param handle	Handle to the STA monitoring module which is returned while initializing
 *
 * @return		Status of the call
 */
BCM_STAMON_STATUS bcm_stamon_deinit(bcm_stamon_handle *handle);

/**
 * One function for all the STA monitoring functionality.
 *
 * @param handle	Handle to the STA monitoring module which is returned while initializing
 * @param cmd		STA monitoring command
 * @param param		buffer containing the parameter to the STA monitoring like
 *			bcm_stamon_maclist_t in BCM_STAMON_CMD_ADD/BCM_STAMON_CMD_DEL
 *			/BCM_STAMON_CMD_GET in other cases NULL. If passed NULL for DEL and
 *			GET, the module deletes and gets all the STA's respectively.
 * @param outbuf	Output buffer containing monitored STA details in case of BCM_STAMON_CMD_GET
 *			or else NULL. If the outbuf is not NULL free the memory
 *
 * @return		status of the call
 */
BCM_STAMON_STATUS bcm_stamon_command(bcm_stamon_handle *handle, bcm_stamon_cmd_t cmd,
	void *param, void **outbuf);

/**
 * Return error string for the error code
 *
 * @param errorcode	Error code
 *
 * @return		Error string corresponding to the error code
 */
const char* bcm_stamon_strerror(BCM_STAMON_STATUS errorcode);

#endif  /* _BCM_STAMON_H_ */
