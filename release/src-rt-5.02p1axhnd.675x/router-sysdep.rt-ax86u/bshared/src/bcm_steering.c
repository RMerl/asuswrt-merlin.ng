/*
 * Broadcom steering implementation which includes BSS transition, block and unblock MAC
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
 * $Id: bcm_steering.c 789955 2020-08-12 10:21:09Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <bcmparams.h>
#include <bcmtimer.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <netconf.h>
#include <nvparse.h>
#include <shutils.h>
#include <wlutils.h>

#include "bcm_steering.h"

#define WL_WLIF_BUFSIZE_1K	1024
#define WL_WLIF_BUFSIZE_4K	4096
#define WL_MACMODE_MASK		0x6000
#define WL_MACPROBE_MASK	0x1000
#define WL_MACMODE_SHIFT	13
#define WL_MACPROBE_SHIFT	12
/* Static info set */
#define WL_SINFOSET_SHIFT	15

#define WL_MAC_PROB_MODE_SET(m, p, f)	(((m) << WL_MACMODE_SHIFT) | ((p) << WL_MACPROBE_SHIFT) \
		| ((f) << WL_SINFOSET_SHIFT))
#define	WL_MACMODE_GET(f)	(((f) & WL_MACMODE_MASK) >> WL_MACMODE_SHIFT)
#define	WL_MACPROBE_GET(f)	(((f) & WL_MACPROBE_MASK) >> WL_MACPROBE_SHIFT)
#define TIMER_MAX_COUNT	16
#define WL_MAC_ADD	1
#define WL_MAC_DEL	0

#define SEC_TO_MICROSEC(x)	((x) * 1000 * 1000)
#define MILISEC_TO_SEC(x)	((x) / 1000)

/* For debug prints */
#define STEERING_DEBUG_ERROR		0x0001

/* NVRAM names */
#define WLIFU_NVRAM_STEERING_MSGLEVEL	"steering_msglevel"
#define WLIFU_NVRAM_TM_DEFIANT_WD	"steer_tm_defiant_wd" /* NVRAM for Dafiant WD Timer Tick */
#define WLIFU_NVRAM_STEER_RESP_TIMEOUT	"steer_resp_timeout"	/* BSS Trans Response timoeut */
#define WLIFU_NVRAM_NOBCNSSID_TIMEOUT	"nobcnssid_timeout"	/* Nobcnssid timoeut */
#define WLIFU_NVRAM_FAKE_CHAN_UTIL	"fake_chan_util"	/* Fake channel utilization */

#define WLIFU_DEF_TM_DEFIANT_WD		5	/* NVRAM Default value of Dafiant WD Timer Tick */
#define WLIFU_DEF_STEER_RESP_TIMEOUT	5	/* NVRAM Default value of Steer Response */
#define WLIFU_DEF_NOBCNSSID_TIMEOUT	10	/* NVRAM Default value for nobcnssid timeout */
#define WLIFU_DEF_FAKE_CHAN_UTIL	205	/* NVRAM Default value for fake */
						/* channel utilization(80%) */
/* Steering Band Digit */
#define WLIFU_BAND_DIGIT_UNWN	0
#define WLIFU_BAND_DIGIT_5GTO5G	1
#define WLIFU_BAND_DIGIT_5GTO2G	2
#define WLIFU_BAND_DIGIT_2GTO5G	3
#define WLIFU_BAND_DIGIT_2GTO2G	4

/* File macros */
#define STEERINGFILE_LINE_LEN		100
#define STEERINGFILE_DIR_NAME		"/tmp"
#define STEERINGFILE_PREFIX		"maclist"
#define STEERINGFILE_POSTFIX		".txt"
#define STEERINGFILE_TMPPREFIX		"tmpmaclist"
#define	STEERINGFILE_NAME_FORMAT	"%s/%s_%s%s"
#define STEERINGFILE_READWRITE_FORMAT	"%s\t%s\t%lu\t%x\n"

/* to print message level */
unsigned int g_steering_msglevel;

#define STEERING_PRINT(fmt, arg...) \
	printf("WLIF_BSSTRANS >>%s(%d): "fmt, __FUNCTION__, __LINE__, ##arg)

#define WLIF_BSSTRANS(fmt, arg...) \
	do { if (g_steering_msglevel & STEERING_DEBUG_ERROR) \
		STEERING_PRINT(fmt, ##arg); } while (0)

/* IOCTL swapping mode for Big Endian host with Little Endian dongle. */
/* The below macros handle endian mis-matches between host and dongle. */
extern bool gg_swap; /* Swap variable set by wl_endian_probe */
#define htod32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))
#define htod16(i) (gg_swap ? bcmswap16(i) : (uint16)(i))
#define dtoh32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))

#define OUI_ADDR_LEN  3
#define oui_cmp(a, b) ((((const uint8 *)(a))[0] ^ ((const uint8 *)(b))[0]) | \
		(((const uint8 *)(a))[1] ^ ((const uint8 *)(b))[1]) | \
		(((const uint8 *)(a))[2] ^ ((const uint8 *)(b))[2]))
#define OUI_GENERAL "ff:ff:ff"

/*
 * Convert OUI  address string representation to binary data
 * @param	a	string in xx:xx:xx notation
 * @param	e	binary data
 * @return	TRUE if conversion was successful and FALSE otherwise
 */
int
oui_atoe(const char *a, unsigned char *e)
{
	char *c = (char *) a;
	int i = 0;

	memset(e, 0, OUI_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == OUI_ADDR_LEN)
			break;
	}
	return (i == OUI_ADDR_LEN);
}

typedef int (* wnm_action_t) (wl_wlif_hdl *hdl,
	wl_wlif_bss_trans_data_t *ioctl_hndlr_data);

typedef struct sta_oui_wnm_act {
	uint8 oui[3];			/* Vendor OUI */
	uint8 pre_bt_action;		/* Pre BSS transition action value   */
	uint8 post_bt_action;		/* Post BSS transition action value  */
	int post_bt_timeout;		/* Timeout for Post action */
	struct sta_oui_wnm_act *next;
} sta_oui_wnm_act_t;

static int wl_wlif_set_fake_chan_util_for_sta(wl_wlif_hdl *hdl,
	wl_wlif_bss_trans_data_t *ioctl_hndlr_data);
static int wl_wlif_clear_fake_chan_util_for_sta(wl_wlif_hdl *hdl,
	wl_wlif_bss_trans_data_t *ioctl_hndlr_data);
static int wl_wlif_do_block_mac(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data);
static int wl_wlif_do_unblock_mac(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data);
static void wl_retrieve_wnm_oui_action_config(void);

/* Order of the function in array and ACTION position should match  */
static wnm_action_t wnm_actions[] = {
	NULL,					/* 0 - No action */
	wl_wlif_set_fake_chan_util_for_sta,	/* 1 - Action:  set fake chan util for sta */
	wl_wlif_clear_fake_chan_util_for_sta,	/* 2 - Action:  clear fake chan util for sta */
	wl_wlif_do_block_mac,			/* 3 - Action:  do block mac for sta */
	wl_wlif_do_unblock_mac,			/* 4 - Action:  do unblock mac for sta */
};

static sta_oui_wnm_act_t *oui_action = NULL;

/*
 * wnm_oui_action_config
 * wnm_oui_action_config format: xx:xx:xx,pre_action_value, post_action_value,timeout  ".
 */
#define OUI_ACT_STRLEN  14
#define WNM_ACTION_SIZE  sizeof(wnm_actions)/sizeof(wnm_action_t)

/* Forward declaration */
typedef struct static_maclist_ static_maclist_t;

/*
 * Struct for wlif handle data.
 * and app specific data.
 */
typedef struct wl_wlif_hdl_data {
	char ifname[IFNAMSIZ];		/* Interface name. */
	uint32 resp_timeout;		/* BSS Transition response timeout(In Seconds) */
	callback_hndlr ioctl_hndlr;	/* Ioctl call handler. */
	void *uschd_hdl;		/* Uschd lib handle. */
	bcm_timer_module_id timer_hdl;	/* Linux timer handle. */
	char appname[IFNAMSIZ];		/* Application that uses this library */
	void *data;			/* App specific data. */
	static_maclist_t *timer_data;	/* Last MAC block timer data */
	uint32 steer_flags;		/* Steering flags of type STEER_FLAGS_XXX */
	dll_t steer_req_list;		/* Head node of All active steering requests */
} wl_wlif_hdl_data_t;

/* Structure to hold active steering request for handling the steering response */
typedef struct wl_wlif_steer_req {
	dll_t node;				/* dll_t struct node for doubly linked list */
	uint8 token;				/* Dialog token of the request */
	bcm_timer_id timer_id;			/* BCM timer's ID in case of bcm timer */
	wl_wlif_bss_trans_data_t bt_data;	/* BSS transition request data sent from app */
	wl_wlif_hdl_data_t *hdl_data;		/* wlif handle data */
} wl_wlif_steer_req_t;

/* Struct to read from file data. */
typedef struct maclist_entry {
	char ifname[IFNAMSIZ];			/* Interface name. */
	char macaddr[ETHER_ADDR_STR_LEN];	/* Sta addr */
	time_t timeout;				/* Timeout to clear mac from maclist. */
	short flag;				/* Flag Bit Values */
} maclist_entry_t;

/*
 * Flag Bit Values
 * 0-0	: Static info set
 * 1-1	: Probe Mode
 * 2-3	: Mac mode
 * 4-7	: Reserved
 * 8-15 : Static mac list count
 */
struct static_maclist_ {
	wl_wlif_hdl_data_t *hdl_data;
	short flag;
	struct ether_addr addr;		/* Sta addr. */
	int timeout;			/* Timeout to clear mac from maclist. */
	bcm_timer_id timer_id;		/* BCM timer's ID in case of bcm timer */
};

typedef struct disassoc_ {
	wl_wlif_hdl_data_t *hdl_data;
	struct ether_addr addr;			/* STA MAC */
} disassoc_t;

/*  Trigger Post Bss transreq handler */
typedef struct static_hdl_post_bt {
	wl_wlif_hdl_data_t *hdl_data;
	struct ether_addr addr;			/* STA MAC */
	uint8 action;				/* Post bt action */
	int timeout;				/* Timeout to clear mac from maclist. */
} wl_wlif_hdl_post_bt_t;

/**
 * Callback Function pointer to be called, when linux timer is created
 *
 * @param timer	Data recieved from socket. Which will be passed to this function
 *
 * @param data	Timer data passed when timer is created
 */
typedef void wlif_linux_timer_cb(bcm_timer_id timer, void *data);

/**
 * Callback Function pointer to be called, when usched timer is created
 *
 * @param hdl	usched handle passed when timer is created
 *
 * @param data	Timer data passed when timer is created
 */
typedef void wlif_usched_timer_cb(bcm_usched_handle* hdl, void *data);

/* Steering lib default ioctl handler routine. */
static int wl_wlif_do_ioctl(char *ifname, int cmd, void *buf, int buflen,
        void *data);

/* Counter to match the response for BSS transition request */
static uint8 bss_token = 0;

/* Create Unblock MAC Timer */
static int wl_wlif_create_unblock_mac_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout, int flag);
/* Remove unblock MAC timer */
static void wl_wlif_remove_unblock_timer(wl_wlif_hdl_data_t *hdl_data);
/* Create Defiant STA Watchdog Timer */
static int wl_wlif_create_defiant_sta_watchdog_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout);
/* Set/Unset nobcnssid iovar */
static int wl_wlif_set_nobcnssid(wl_wlif_hdl *hdl);
static bool wl_wlif_unset_nobcnssid(wl_wlif_hdl *hdl);
/* Callback functions to unset nobcnssid iovar */
static void wl_wlif_unset_nobcnssid_cb(bcm_timer_id timer_id, void *arg);
static void wl_wlif_unset_nobcnssid_usched_cb(bcm_usched_handle *hdl, void *arg);
/* Creates timer to unset nobcnssid iovar */
static int wl_wlif_create_nobcnssid_timer(wl_wlif_hdl_data_t *hdl_data, int timeout);
/* Create Timer */
static int wl_wlif_create_timer(void *uschd_hdl, bcm_timer_module_id timer_hdl,
	void* cbfn, void *data, int timeout, int repeat, bcm_timer_id *out_timer_id);
/* Callback handlers for disassoc mac */
static void wl_wlif_disassoc_mac_cb(bcm_timer_id timer_id, void* arg);
static void wl_wlif_disassoc_mac_usched_cb(bcm_usched_handle* hdl, void* arg);
/* Create post bss transreq handler Timer */
static int wl_wlif_create_post_bss_transreq_timer(wl_wlif_hdl_data_t  *hdl_data,
		wl_wlif_bss_trans_data_t *ioctl_hndlr_data);
static void wl_wlif_post_bss_transreq_usched_cb(bcm_usched_handle* hdl, void* arg);
static void wl_wlif_post_bss_transreq_cb(bcm_timer_id timer_id, void* arg);
/* Traverse Maclist to find MAC address exists or not */
static bool wl_wlif_find_sta_in_assoclist(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr *find_mac);
/* Processing after getting BTM response from STA or if there is no response from the
 * STA(timeout)
 */
static void wl_wlif_post_bss_transresp_process(wl_wlif_hdl_data_t *hdl_data,
	wl_wlif_steer_req_t *steer_req, wl_wlif_bss_trans_resp_t *btm_resp, int bsstrans_ret);
/*
 * If BSS-Trans response is not ACCEPT, or if it is ACCEPT but still persists in Assoclist
 *  invokes Block MAC and Disassociates STA from current BSS
 */
static int wl_wlif_block_mac_and_disassoc(wl_wlif_hdl *hdl, struct ether_addr addr,
	int timeout, int disassoc);

#ifndef BCM_EVENT_HEADER_LEN
#define BCM_EVENT_HEADER_LEN   (sizeof(bcm_event_t))
#endif // endif

/* Get interface's chanspec */
static int
wl_wlif_get_chanspec(char *ifname, chanspec_t* out_chanspec)
{
	int ret = -1;
	uint32 val = 0;

	/* Validate arg */
	if (!out_chanspec) {
		goto end;
	}

	ret = wl_iovar_getint(ifname, "chanspec", (int*)&val);
	if (ret < 0) {
		WLIF_BSSTRANS("ifname %s Get chanspec failed ret %d\n", ifname, ret);
		goto end;
	}

	*out_chanspec = (chanspec_t)dtoh32(val);

end:
	return ret;
}

/* Get the steering band digit from the chanspecs */
static int
wl_wlif_get_steering_band_digit(chanspec_t from_chanspec, chanspec_t to_chanspec)
{
	if (CHSPEC_IS5G(from_chanspec) && CHSPEC_IS5G(to_chanspec)) {
		return WLIFU_BAND_DIGIT_5GTO5G;
	} else if (CHSPEC_IS5G(from_chanspec) && CHSPEC_IS2G(to_chanspec)) {
		return WLIFU_BAND_DIGIT_5GTO2G;
	} else if (CHSPEC_IS2G(from_chanspec) && CHSPEC_IS5G(to_chanspec)) {
		return WLIFU_BAND_DIGIT_2GTO5G;
	} else if (CHSPEC_IS2G(from_chanspec) && CHSPEC_IS2G(to_chanspec)) {
		return WLIFU_BAND_DIGIT_2GTO2G;
	}

	return WLIFU_BAND_DIGIT_UNWN;
}

/* Find steer request from the list */
static wl_wlif_steer_req_t*
wl_wlif_find_steer_req(wl_wlif_hdl_data_t *hdl_data, uint8 token, struct ether_addr *sta_mac)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hdl_data->steer_req_list);
		!dll_end(&hdl_data->steer_req_list, item_p); item_p = dll_next_p(item_p)) {

		wl_wlif_steer_req_t *tmp = (wl_wlif_steer_req_t*)item_p;

		if (token == tmp->token && (eacmp(&tmp->bt_data.addr, sta_mac) == 0)) {
			return tmp;
		}
	}

	return NULL;
}

/* Delete steer request from list */
static void
wl_wlif_delete_steer_req(wl_wlif_hdl_data_t *hdl_data, uint8 token, struct ether_addr *sta_mac)
{
	dll_t *item_p;

	for (item_p = dll_head_p(&hdl_data->steer_req_list);
		!dll_end(&hdl_data->steer_req_list, item_p); item_p = dll_next_p(item_p)) {

		wl_wlif_steer_req_t *tmp = (wl_wlif_steer_req_t*)item_p;

		if (token == tmp->token && (eacmp(&tmp->bt_data.addr, sta_mac) == 0)) {
			dll_delete(item_p);
			free(tmp);
			return;
		}
	}
}

/* Process the steer response timeout */
static void
wl_wlif_handle_steer_resp_wait_timeout(wl_wlif_steer_req_t *steer_req)
{
	wl_wlif_bss_trans_resp_t btm_resp;

	if (!steer_req) {
		WLIF_BSSTRANS("Err: Invalid STEER request handle\n");
		return;
	}

	WLIF_BSSTRANS("STEER response timeout for token[%d] and STA["MACF"]\n",
			steer_req->token, ETHER_TO_MACF(steer_req->bt_data.addr));
	memset(&btm_resp, 0, sizeof(btm_resp));
	wl_wlif_post_bss_transresp_process(steer_req->hdl_data, steer_req, &btm_resp,
		WLIFU_BSS_TRANS_RESP_UNKNOWN);

	wl_wlif_delete_steer_req(steer_req->hdl_data, steer_req->token, &steer_req->bt_data.addr);
}

/* BCM Timer Callback handler to inform steer response timeout. Library has sent the steer request
 * but there is no response from the STA
 */
static void
wl_wlif_steer_resp_wait_tiemout_cb(bcm_timer_id timer_id, void* arg)
{
	wl_wlif_handle_steer_resp_wait_timeout((wl_wlif_steer_req_t*)arg);
	bcm_timer_delete(timer_id);
}

/* BCMUSCHED Timer Callback handler to inform steer response timeout. Library has sent the steer
 * request but there is no response from the STA
 */
static void
wl_wlif_steer_resp_wait_tiemout_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	wl_wlif_handle_steer_resp_wait_timeout((wl_wlif_steer_req_t*)arg);
}

/* Remove steer response wait timer */
static void
wl_wlif_remove_bss_trans_resp_wait_timer(wl_wlif_hdl_data_t *hdl_data,
	wl_wlif_steer_req_t *steer_req)
{
	if (!hdl_data || !steer_req) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return;
	}

	/* remove the timer */
	if (hdl_data->uschd_hdl == NULL) {
		bcm_timer_delete(steer_req->timer_id);
	} else {
		bcm_usched_remove_timer(hdl_data->uschd_hdl,
			wl_wlif_steer_resp_wait_tiemout_usched_cb, steer_req);
	}
}

/* Remove all the active steer requests */
static void
wl_wlif_cleanup_steer_req_list(wl_wlif_hdl_data_t *hdl_data)
{
	dll_t *item_p, *next_p;

	for (item_p = dll_head_p(&hdl_data->steer_req_list);
		!dll_end(&hdl_data->steer_req_list, item_p); item_p = next_p) {
		next_p = dll_next_p(item_p);

		wl_wlif_steer_req_t *tmp = (wl_wlif_steer_req_t*)item_p;

		WLIF_BSSTRANS("Removing steer request entry for token[%d] and STA["MACF"]\n",
			tmp->token, ETHER_TO_MACF(tmp->bt_data.addr));
		wl_wlif_remove_bss_trans_resp_wait_timer(hdl_data, tmp);
		dll_delete(item_p);
		free(tmp);
	}
}

/* Add steer request to list to keep track of all the active steer requests. This is required to
 * process the steer response from the STA which is received in the application and pass it to the
 * library using function "wl_wlif_process_bss_trans_resp_event"
 */
static int
wl_wlif_add_steer_req_to_list(wl_wlif_hdl_data_t *hdl_data, uint8 token,
	wl_wlif_bss_trans_data_t *ioctl_data)
{
	int ret;
	wl_wlif_steer_req_t *new = NULL;
	bcm_timer_id timer_id;

	/* Check the handle */
	if (!hdl_data) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return -1;
	}

	/* If exists do not add. Actually it should not happen */
	if (wl_wlif_find_steer_req(hdl_data, token, &ioctl_data->addr)) {
		WLIF_BSSTRANS("For token[%d] and STA["MACF"] entry already exists\n",
			token, ETHER_TO_MACF(ioctl_data->addr));
		return 1;
	}

	/* Allocate for steer request item */
	new = (wl_wlif_steer_req_t*)calloc(1, sizeof(*new));
	if (new == NULL) {
		WLIF_BSSTRANS("Failed to allocate memory\n");
		return -1;
	}

	new->hdl_data = hdl_data;
	new->token = token;
	memcpy(&new->bt_data, ioctl_data, sizeof(new->bt_data));

	/* Create an Appropriate Timer, to send response if there is no BTM response from STA */
	ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
		(hdl_data->uschd_hdl ? (void*)wl_wlif_steer_resp_wait_tiemout_usched_cb :
		(void*)wl_wlif_steer_resp_wait_tiemout_cb), (void*)new, hdl_data->resp_timeout,
		FALSE, &timer_id);
	if (!ret) {
		WLIF_BSSTRANS("%s: Error creating timer for token[%d] and STA "MACF" to send "
			"response. ret=%d\n",
			hdl_data->ifname, token, ETHER_TO_MACF(ioctl_data->addr), ret);
		free(new);
		return -1;
	}
	new->timer_id = timer_id;

	dll_append(&hdl_data->steer_req_list, (dll_t*)new);

	return 0;
}

/* Perform post BTM response operation if "bss_trans_resp_hndlr" callback is not registered */
static void
wl_wlif_post_bss_transresp_process(wl_wlif_hdl_data_t *hdl_data, wl_wlif_steer_req_t *steer_req,
	wl_wlif_bss_trans_resp_t *btm_resp, int bsstrans_ret)
{
	int brute_force = 0, steer_band_digit;
	wl_wlif_bss_trans_data_t *ioctl_hndlr_data;
	chanspec_t chanspec = 0;

	/* Check the handle */
	if (!hdl_data || !steer_req) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return;
	}

	ioctl_hndlr_data = &steer_req->bt_data;

	WLIF_BSSTRANS("%s-%s: BSS Transition Resp from STA "MACF" to BSSID "MACF" is : %s\n",
		hdl_data->appname, hdl_data->ifname, ETHER_TO_MACF(ioctl_hndlr_data->addr),
		ETHER_TO_MACF(ioctl_hndlr_data->bssid),
		((bsstrans_ret == WLIFU_BSS_TRANS_RESP_REJECT) ? "REJECT" :
		((bsstrans_ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) ? "NO RESPONSE" :"ACCEPT")));

	/* If bss-trans resp handler is provided invoke it. */
	if ((ioctl_hndlr_data->resp_hndlr != NULL) &&
		ioctl_hndlr_data->resp_hndlr(ioctl_hndlr_data->resp_hndlr_data, bsstrans_ret)) {
		goto end;
	}

	/* If disassoc imminent bit is set then no need to check for defiant STA
	 * and brute force steering
	*/
	if (ioctl_hndlr_data->flags & WLIFU_BSS_TRANS_FLAGS_DISASSOC_IMNT) {
		goto end;
	}

	/* Get the chanspec of current interface */
	wl_wlif_get_chanspec(hdl_data->ifname, &chanspec);
	steer_band_digit = wl_wlif_get_steering_band_digit(chanspec, ioctl_hndlr_data->chanspec);

	/* Check if Brute Force Steer is applicable for each steer response type */
	if (bsstrans_ret == WLIFU_BSS_TRANS_RESP_REJECT) {
		/* By default no brute force for reject response */
		brute_force = 0;
		if (BTR_RJCT_BRUTFORCE_ON(hdl_data->steer_flags)) {
			/* Brute force enabled for reject response */
			brute_force = 1;
		}
	}

	if (bsstrans_ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) {
		/* By default do brute force for unknown response (legacy STAs) */
		brute_force = 1;
		if (BTR_UNWN_BRUTFORCE_OFF(hdl_data->steer_flags)) {
			/* No brute force for unknows response */
			brute_force = 0;
		}

		if (IS_LEGACY_ACT_STA_BRUTEFORCE_OFF(hdl_data->steer_flags)) {
			/* No brute force for active legacy STAs flag set */
			if (ioctl_hndlr_data->flags & WLIFU_BSS_TRANS_FLAGS_LEGACY_ACTIVE_STA) {
				brute_force = 0;
			}
		}
	}

	/* Check if brute force steer is applicable based on band digits */
	if (bsstrans_ret != WLIFU_BSS_TRANS_RESP_ACCEPT) {
		brute_force |=
			(((steer_band_digit == WLIFU_BAND_DIGIT_5GTO5G) &&
			(BTR_5GTO5G_BRUTEFORCE_ON(hdl_data->steer_flags))) ||
			((steer_band_digit == WLIFU_BAND_DIGIT_5GTO2G) &&
			(BTR_5GTO2G_BRUTEFORCE_ON(hdl_data->steer_flags))) ||
			((steer_band_digit == WLIFU_BAND_DIGIT_2GTO5G) &&
			(BTR_2GTO5G_BRUTEFORCE_ON(hdl_data->steer_flags))) ||
			((steer_band_digit == WLIFU_BAND_DIGIT_2GTO2G) &&
			(BTR_2GTO2G_BRUTEFORCE_ON(hdl_data->steer_flags))));
	}

	WLIF_BSSTRANS("%s:%s brute force disassoc of STA "MACF" after %d sec. NVRAM %s=0x%x "
		"steer_band_digit=%d\n",
		hdl_data->ifname, brute_force ? "" : " No", ETHER_TO_MACF(ioctl_hndlr_data->addr),
		ioctl_hndlr_data->timeout, WLIFU_NVRAM_STEER_FLAGS, hdl_data->steer_flags,
		steer_band_digit);
	/* If Brute Force Steer is not applicable, for this Response Type, Leave */
	if (!brute_force) {
		/* If the response is not accept, unblock the MAC from current interface and
		 * remove unblock timer
		 */
		if (bsstrans_ret != WLIFU_BSS_TRANS_RESP_ACCEPT) {
			wl_wlif_remove_unblock_timer(hdl_data);
		}
		goto end;
	}
#ifdef BCM_CEVENT
	/* cevent sent with STA MAC address for BTM Brute Force from app */
	send_cevent(hdl_data->ifname, NULL, &ioctl_hndlr_data->addr,
		0, ioctl_hndlr_data->reason, 0, CEVENT_TYPE_A2C, CEVENT_A2C_ST_BTM_BRUTE_FORCE,
		CEVENT_FRAME_DIR_TX, NULL, 0);
#endif /* BCM_CEVENT */

	/* If Brute Force is applicable, for this Response Type, Do Brute Force way */
	wl_wlif_block_mac_and_disassoc(hdl_data, ioctl_hndlr_data->addr,
		ioctl_hndlr_data->timeout, brute_force);

end:
	/* For BSS Transition Response ACCEPT, Check if Dafiant STA Watchdog is required or not */
	if (hdl_data && (bsstrans_ret == WLIFU_BSS_TRANS_RESP_ACCEPT) &&
		BTR_ACPT_DEFIANT_WD_ON(hdl_data->steer_flags)) {

		/* Create Defiant STA Watchdog Timer */
		WLIF_BSSTRANS("%s: Creating timer for %d sec to check if the STA "MACF" actually "
			"moved\n", hdl_data->ifname, ioctl_hndlr_data->timeout,
			ETHER_TO_MACF(ioctl_hndlr_data->addr));
		wl_wlif_create_defiant_sta_watchdog_timer(hdl_data,
			ioctl_hndlr_data->addr, ioctl_hndlr_data->timeout);
	}

	if (steer_req->bt_data.post_resp_cb) {
		steer_req->bt_data.post_resp_cb(hdl_data->ifname,
			steer_req->bt_data.post_resp_cb_data, bsstrans_ret, btm_resp);
	}
}

/* Routine for processing the "WLC_E_BSSTRANS_RESP" event. Application listening to this event
 * should pass the event data after "wl_wlif_do_bss_trans" for getting the response data from
 * event data. If application doesn't pass the event data, library will call the
 * "bss_trans_resp_hndlr" or "post_bss_trans_resp_cb" to inform that there is no
 * response(steer_resp type is WLIFU_BSS_TRANS_RESP_UNKNOWN).
 */
void
wl_wlif_process_bss_trans_resp_event(wl_wlif_hdl *hdl, char *pkt, int len)
{
	int pdata_len, steer_resp = WLIFU_BSS_TRANS_RESP_UNKNOWN;
	dot11_bsstrans_resp_t *bsstrans_resp;
	dot11_neighbor_rep_ie_t *neighbor = NULL;
	bcm_event_t *dpkt;
	struct ether_addr *addr;
	struct ether_addr *sta_mac;
	wl_wlif_hdl_data_t *hdl_data;
	wl_wlif_steer_req_t *steer_req = NULL;
	wl_wlif_bss_trans_resp_t btm_resp;

	/* Check the handle */
	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return;
	}

	if (!pkt) {
		WLIF_BSSTRANS("Err: Event Data is NULL\n");
		return;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;
	memset(&btm_resp, 0, sizeof(btm_resp));

	pkt = pkt + IFNAMSIZ;
	pdata_len = len - IFNAMSIZ;

	if (pdata_len <= BCM_EVENT_HEADER_LEN) {
		WLIF_BSSTRANS("%s: BTM Response: data_len %d too small\n", hdl_data->ifname,
			pdata_len);
		goto end;
	}

	dpkt = (bcm_event_t *)pkt;
	pkt += BCM_EVENT_HEADER_LEN; /* payload (bss response) */
	pdata_len -= BCM_EVENT_HEADER_LEN;
	sta_mac = (struct ether_addr *)(&(dpkt->event.addr));
	bsstrans_resp = (dot11_bsstrans_resp_t *)pkt;
	/* data will have the BSSID if the response is accept(i.e. 0) */
	addr = (struct ether_addr *)(bsstrans_resp->data);

	steer_req = wl_wlif_find_steer_req(hdl_data, bsstrans_resp->token, sta_mac);
	if (!steer_req) {
		WLIF_BSSTRANS("%s-%s: BTM Response: For STA["MACF"] token=%d status=%d "
			"ToBSS="MACF" Steer request entry not present. Do not process.\n",
			hdl_data->appname, hdl_data->ifname, ETHERP_TO_MACF(sta_mac),
			bsstrans_resp->token, bsstrans_resp->status, ETHERP_TO_MACF(addr));
		goto end;
	}

	/* If the BSS transition response response is 6, then BSSID field will
	 * be in neighbor report.
	 */
	if (bsstrans_resp->status == 6) {
		neighbor = (dot11_neighbor_rep_ie_t *)bsstrans_resp->data;
		addr = &neighbor->bssid;
	}

	WLIF_BSSTRANS("%s-%s: BTM Response: STA["MACF"] token=%d status=%d "
		"ToBSS="MACF"\n", hdl_data->appname, hdl_data->ifname, ETHERP_TO_MACF(sta_mac),
		bsstrans_resp->token, bsstrans_resp->status, ETHERP_TO_MACF(addr));

	/* Store all the values in the out_resp field */
	btm_resp.response_valid = 1;
	btm_resp.status = bsstrans_resp->status;
	/* If success then only BSSID field exists */
	if (btm_resp.status == 0) {
		eacopy(addr, &btm_resp.trgt_bssid);
	}

	/* If there is some neigbor report */
	if ((bsstrans_resp->status == 6) && neighbor) {
		WLIF_BSSTRANS("%s: BTM Response is %d : id=%d len=%d bssid["MACF"] "
			"bssid_info=0x%X Operatin class=%d channel=%d phytype=%d\n",
			hdl_data->ifname, bsstrans_resp->status, neighbor->id, neighbor->len,
			ETHER_TO_MACF(neighbor->bssid), neighbor->bssid_info,
			neighbor->reg, neighbor->channel, neighbor->phytype);
	}
	/* reject */
	if (bsstrans_resp->status) {
		steer_resp = WLIFU_BSS_TRANS_RESP_REJECT;
		goto end;
	}

	/* accept, but use another target bssid */
	if (eacmp(&steer_req->bt_data.bssid, addr) != 0) {
		WLIF_BSSTRANS("%s: BTM Response: Different Target bssid. BTM "
			"Request was for "MACF". STA responded with "MACF"\n",
			hdl_data->ifname, ETHER_TO_MACF(steer_req->bt_data.bssid),
			ETHERP_TO_MACF(addr));
		steer_resp = WLIFU_BSS_TRANS_RESP_REJECT;
		goto end;
	}

	steer_resp = WLIFU_BSS_TRANS_RESP_ACCEPT;

end:
	if (steer_req) {
		wl_wlif_post_bss_transresp_process(hdl_data, steer_req, &btm_resp, steer_resp);
		wl_wlif_remove_bss_trans_resp_wait_timer(hdl_data, steer_req);
		wl_wlif_delete_steer_req(hdl_data, steer_req->token, sta_mac);
	}
}

static void
wl_wlif_get_maclist_filename(wl_wlif_hdl_data_t *hdl, char *fname)
{
	snprintf(fname, PATH_MAX, STEERINGFILE_NAME_FORMAT, STEERINGFILE_DIR_NAME,
		STEERINGFILE_PREFIX, (hdl->appname), STEERINGFILE_POSTFIX);
}
static int
wl_wlif_get_maclist_entry(char *str, maclist_entry_t *e)
{
	int ret;
	maclist_entry_t zeroe = {0};
	*e = zeroe;

	ret = sscanf(str, STEERINGFILE_READWRITE_FORMAT, e->ifname, e->macaddr,
			&e->timeout, &e->flag);
	return ret;
}
static int
wl_wlif_restore_old_maclist_timer(wl_wlif_hdl_data_t *hdl)
{
	FILE *fp;
	char fname[PATH_MAX];
	struct maclist_entry entry;
	int timeout;
	struct ether_addr ea;
	char line[STEERINGFILE_LINE_LEN];

	wl_wlif_get_maclist_filename(hdl, fname);
	if (!(fp = fopen(fname, "r"))) {
		WLIF_BSSTRANS("Error opening file %s : %s\n", fname, strerror(errno));
		return -1;
	}
	while (fgets(line, sizeof(line), fp) != NULL) {
		wl_wlif_get_maclist_entry(line, &entry);
		if ((strncmp(entry.ifname, hdl->ifname, sizeof(entry.ifname)) == 0)) {
			timeout = (entry.timeout - time(NULL)) < 1 ?
					1 : (entry.timeout - time(NULL));
			bcm_ether_atoe((const char *)entry.macaddr, &ea);
			wl_wlif_create_unblock_mac_timer(hdl, ea, timeout, entry.flag);
			WLIF_BSSTRANS("%s: create timer for unblocking "MACF" after %d "
				"seconds\n", hdl->ifname, ETHER_TO_MACF(ea), timeout);
		}
	}
	fclose(fp);
	return 0;
}
/*
 * Init routine for wlif lib handle.
 */
wl_wlif_hdl*
wl_wlif_init(void *uschd_hdl, char *ifname, callback_hndlr ioctl_hndlr,
	void *data, char *appname)
{
	wl_wlif_hdl_data_t *hdl = (wl_wlif_hdl_data_t*)calloc(1, sizeof(*hdl));
	char *value;

	value = nvram_safe_get(WLIFU_NVRAM_STEERING_MSGLEVEL);
	g_steering_msglevel = (int)strtoul(value, NULL, 0);

	if (!hdl) {
		WLIF_BSSTRANS("Malloc failed for hdl \n");
		return NULL;
	}

	memset(hdl, 0, sizeof(*hdl));
	hdl->uschd_hdl = uschd_hdl;
	strncpy(hdl->ifname, ifname, IFNAMSIZ-1);
	hdl->ifname[IFNAMSIZ -1] = '\0';
	hdl->steer_flags = WLIFU_DEF_STEER_FLAGS;
	dll_init(&hdl->steer_req_list);

	value = nvram_safe_get(WLIFU_NVRAM_STEER_FLAGS);
	if (value[0] != '\0') {
		hdl->steer_flags = (uint32)strtoul(value, NULL, 0);
	}

	/* Read the OUI vendor and action value from the NVRAM */
	wl_retrieve_wnm_oui_action_config();

	/* Read the BSS transition response timneout from the NVRAM */
	value = nvram_safe_get(WLIFU_NVRAM_STEER_RESP_TIMEOUT);
	hdl->resp_timeout = (uint32)strtoul(value, NULL, 0);
	if (hdl->resp_timeout <= 0) {
		hdl->resp_timeout = WLIFU_DEF_STEER_RESP_TIMEOUT;
	}

	/* App does not registers own handler the default will be wl_wlif_do_ioctl */
	if (ioctl_hndlr) {
		hdl->ioctl_hndlr = ioctl_hndlr;
	} else {
		hdl->ioctl_hndlr = wl_wlif_do_ioctl;
	}

	/* If uschd library is not used, use linux timer */
	if (uschd_hdl == NULL) {
		/* Initialize Linux Timer Module Handle */
		bcm_timer_module_init(TIMER_MAX_COUNT, &(hdl->timer_hdl));
		bcm_timer_module_enable(hdl->timer_hdl, 1);
	}

	hdl->data = data;
	strncpy(hdl->appname, appname, sizeof(hdl->appname));
	hdl->appname[sizeof(hdl->appname) - 1] = '\0';
	WLIF_BSSTRANS("%s: BSS Transition Initialized Ifname[%s] Msglevel[%d] resp_timeout[%d] "
		"steer_flags[0x%x]\n",
		appname, hdl->ifname, g_steering_msglevel, hdl->resp_timeout, hdl->steer_flags);
	wl_wlif_restore_old_maclist_timer(hdl);

	return hdl;
}

/*
 * Deinit routine to wlif handle.
 */
void
wl_wlif_deinit(wl_wlif_hdl *hdl)
{
	wl_wlif_hdl_data_t *hdldata = NULL;
	sta_oui_wnm_act_t  *oui_ptr, *next_oui;

	if (!hdl) {
		return;
	}

	hdldata = (wl_wlif_hdl_data_t*)hdl;

	WLIF_BSSTRANS("%s: BSS Transition DeInitializing Ifname[%s]\n",
		hdldata->appname, hdldata->ifname);

	/* Remove all the active steer requests */
	wl_wlif_cleanup_steer_req_list(hdldata);

	/* If uschd library is not used, and Linux Timer Module Handle is valid */
	if ((hdldata->uschd_hdl == NULL) && (hdldata->timer_hdl)) {

		/* De-Initialize Linux Timer Module Handle */
		bcm_timer_module_cleanup(hdldata->timer_hdl);
	}

	free(hdldata);
	/* cleanup oui_vendor list */
	oui_ptr = oui_action;
	while (oui_ptr) {
		next_oui = oui_ptr->next;
		free(oui_ptr);
		oui_ptr = next_oui;
	}
	hdldata = NULL;
}

/* Default Callback function for ioctl calls. */
static int
wl_wlif_do_ioctl(char *ifname, int cmd, void *buf, int buflen,
	void *data)
{
	BCM_REFERENCE(data);
	return wl_ioctl(ifname, cmd, buf, buflen);
}

/* Get channel width from chanspec */
static uint8
wl_get_wb_chanwidth(chanspec_t chanspec)
{
	if (CHSPEC_IS8080(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_80_80;
	} else if (CHSPEC_IS160(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_160;
	} else if (CHSPEC_IS80(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_80;
	} else if (CHSPEC_IS40(chanspec)) {
		return WIDE_BW_CHAN_WIDTH_40;
	} else {
		return WIDE_BW_CHAN_WIDTH_20;
	}
}

/* Get wideband channel width and center frequency from chanspec */
static void
wl_get_wb_ie_from_chanspec(dot11_wide_bw_chan_ie_t *wbc_ie,
	chanspec_t chanspec)
{
	if (wbc_ie == NULL) {
		return;
	}

	wbc_ie->channel_width = wl_get_wb_chanwidth(chanspec);

	if (wbc_ie->channel_width == WIDE_BW_CHAN_WIDTH_80_80) {
		wbc_ie->center_frequency_segment_0 =
			wf_chspec_primary80_channel(chanspec);
		wbc_ie->center_frequency_segment_1 =
			wf_chspec_secondary80_channel(chanspec);
	} else {
		wbc_ie->center_frequency_segment_0 = CHSPEC_CHANNEL(chanspec);
		wbc_ie->center_frequency_segment_1 = 0;
	}
}

/*
 * Creates the BSS-Trans action frame from ioctl data.
 * Returns the BSS-Trans response.
 */
static int
wl_wlif_send_bss_transreq_actframe(wl_wlif_hdl_data_t *hdl_data,
	wl_wlif_bss_trans_data_t *ioctl_data)
{
	int ret, buflen;
	char *param, ioctl_buf[WLC_IOCTL_MAXLEN];
	dot11_bsstrans_req_t *transreq;
	dot11_neighbor_rep_ie_t *nbr_ie;
	dot11_wide_bw_chan_ie_t *wbc_ie;
	dot11_ngbr_bsstrans_pref_se_t *nbr_pref;
	wl_af_params_t *af_params;
	wl_action_frame_t *action_frame;
	disassoc_t *disassoc_data = NULL;
	int timeout = 0, tmp_time = 0, bcn_period = 0;

	wl_endian_probe(hdl_data->ifname);

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strncpy(ioctl_buf, "actframe", sizeof(ioctl_buf));
	ioctl_buf[sizeof(ioctl_buf) - 1] = '\0';
	buflen = strlen(ioctl_buf) + 1;
	param = (char *)(ioctl_buf + buflen);

	af_params = (wl_af_params_t *)param;
	action_frame = &af_params->action_frame;

	af_params->channel = 0;
	af_params->dwell_time = htod32(-1);

	memcpy(&action_frame->da, (char *)&(ioctl_data->addr), ETHER_ADDR_LEN);
	action_frame->packetId = htod32((uint32)(uintptr)action_frame);
	action_frame->len = htod16(DOT11_NEIGHBOR_REP_IE_FIXED_LEN +
		DOT11_NGBR_BSSTRANS_PREF_SE_IE_LEN +
		TLV_HDR_LEN + DOT11_BSSTRANS_REQ_LEN +
		TLV_HDR_LEN + DOT11_WIDE_BW_IE_LEN);

	transreq = (dot11_bsstrans_req_t *)&action_frame->data[0];
	transreq->category = DOT11_ACTION_CAT_WNM;
	transreq->action = DOT11_WNM_ACTION_BSSTRANS_REQ;
	if (++bss_token == 0)
		bss_token = 1;
	transreq->token = bss_token;
	transreq->reqmode = DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL;
	if (ioctl_data->flags & WLIFU_BSS_TRANS_FLAGS_BTM_ABRIDGED) {
		/* set bit1 to tell STA the BSSID in list recommended */
		transreq->reqmode |= DOT11_BSSTRANS_REQMODE_ABRIDGED;
	}

	if (ioctl_data->flags & WLIFU_BSS_TRANS_FLAGS_DISASSOC_IMNT) {
		transreq->reqmode |= DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT;
		transreq->disassoc_tmr = ioctl_data->disassoc_timer;
	} else {
		transreq->disassoc_tmr = 0x0000;
	}

	transreq->validity_intrvl = 0xFF;

	nbr_ie = (dot11_neighbor_rep_ie_t *)&transreq->data[0];
	nbr_ie->id = DOT11_MNG_NEIGHBOR_REP_ID;
	nbr_ie->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN +
		DOT11_NGBR_BSSTRANS_PREF_SE_IE_LEN +
		DOT11_WIDE_BW_IE_LEN + TLV_HDR_LEN;
	memcpy(&nbr_ie->bssid, &(ioctl_data->bssid), ETHER_ADDR_LEN);
	htol32_ua_store(ioctl_data->bssid_info, &nbr_ie->bssid_info);
	nbr_ie->reg = ioctl_data->rclass;
	nbr_ie->channel = wf_chspec_ctlchan(ioctl_data->chanspec);
	nbr_ie->phytype = ioctl_data->phytype;

	wbc_ie = (dot11_wide_bw_chan_ie_t *)&nbr_ie->data[0];
	wbc_ie->id = DOT11_NGBR_WIDE_BW_CHAN_SE_ID;
	wbc_ie->len = DOT11_WIDE_BW_IE_LEN;
	wl_get_wb_ie_from_chanspec(wbc_ie, ioctl_data->chanspec);

	param = (char *)wbc_ie + TLV_HDR_LEN + DOT11_WIDE_BW_IE_LEN;
	nbr_pref = (dot11_ngbr_bsstrans_pref_se_t *)param;
	nbr_pref->sub_id = DOT11_NGBR_BSSTRANS_PREF_SE_ID;
	nbr_pref->len = DOT11_NGBR_BSSTRANS_PREF_SE_LEN;
	nbr_pref->preference = DOT11_NGBR_BSSTRANS_PREF_SE_HIGHEST;

	/* Start disassoc timer */
	if (ioctl_data->flags & WLIFU_BSS_TRANS_FLAGS_DISASSOC_IMNT) {
		disassoc_data = (disassoc_t *)malloc(sizeof(disassoc_t));
		if (!disassoc_data) {
			WLIF_BSSTRANS("Error: malloc disassoc timer arg failed\n");
			goto done;
		}
		disassoc_data->hdl_data = hdl_data;
		eacopy(&(ioctl_data->addr), &(disassoc_data->addr));

		ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_BCNPRD, &(bcn_period),
			sizeof(bcn_period), hdl_data->data);
		if (ret < 0) {
			WLIF_BSSTRANS("%s: Error getting beacon period iovar (WLC_GET_BCNPRD). "
				"ret=%d. Stations won't be disassociated after %d beacons\n",
				hdl_data->ifname, ret, ioctl_data->disassoc_timer);
			goto done;
		}

		tmp_time = ioctl_data->disassoc_timer * bcn_period;
		timeout = MILISEC_TO_SEC(tmp_time);

		ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
			(hdl_data->uschd_hdl ? (void*)wl_wlif_disassoc_mac_usched_cb :
			(void*)wl_wlif_disassoc_mac_cb), (void*)disassoc_data, timeout, FALSE,
			NULL);
		if (!ret) {
			WLIF_BSSTRANS("Error: Failed to create disassoc timer. ret=%d\n", ret);
			free(disassoc_data);
		} else {
			WLIF_BSSTRANS("Disassoc timer created for %d sec\n", timeout);
		}
	}
done:
	WLIF_BSSTRANS("%s: Sending BTM Request via actframe to "MACF". Token=%d, Reqmode=0x%X, "
		"Disassoc bcn count=%d. Validity=%d. TBSSID["MACF"] BSSInfo=0x%X "
		"Operating class=%d Channel=%d Phytype=%d\n", hdl_data->ifname,
		ETHER_TO_MACF(action_frame->da), bss_token, transreq->reqmode,
		transreq->disassoc_tmr, transreq->validity_intrvl, ETHER_TO_MACF(nbr_ie->bssid),
		nbr_ie->bssid_info, nbr_ie->reg, nbr_ie->channel, nbr_ie->phytype);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_VAR, ioctl_buf,
		WL_WIFI_AF_PARAMS_SIZE, hdl_data->data);

	if (ret < 0) {
		printf("%s: Error sending BTM Req actframe: ret=%d\n", hdl_data->ifname, ret);
	} else {
		/* Add steer request to list to handle response */
		ret = wl_wlif_add_steer_req_to_list(hdl_data, bss_token, ioctl_data);
	}

	/* Find OUI ,vendor and do post bsstransreq after send BTM request */
	wl_wlif_create_post_bss_transreq_timer(hdl_data, ioctl_data);
	return ret;
}

/*
 * Delete previous btm neighbors, add new btm neighbor
 * and send btm request.
 * Returns the BSS-Trans response.
 */
static int
wl_wlif_send_bss_transreq(wl_wlif_hdl_data_t *hdl_data, wl_wlif_bss_trans_data_t *ioctl_data)
{
	int ret, buflen;
	char ioctl_buf[WLC_IOCTL_SMLEN];
	nbr_rpt_elem_t btq_nbr;
	wl_bsstrans_req_t bsstrans_req;

	/* Clear wnm btq  neighbors */
	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strncpy(ioctl_buf, "wnm_btq_nbr_del", sizeof(ioctl_buf));
	ioctl_buf[sizeof(ioctl_buf) - 1] = '\0';
	buflen = strlen("wnm_btq_nbr_del") + 1;

	memcpy(&ioctl_buf[buflen], &ether_null, sizeof(struct ether_addr));
	buflen += sizeof(struct ether_addr);

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_VAR, ioctl_buf,
		buflen, hdl_data->data);

	if (ret < BCME_OK) {
		/* If driver doesnt support this then fallback to action frame
		 * mechnism of sending bss transition request.
		*/
		WLIF_BSSTRANS("%s: Error in iovar wnm_btq_nbr_del. ret=%d. Fallback to BTM Request "
			"with actframe\n", hdl_data->ifname, ret);
		goto actframe;
	}

	wl_endian_probe(hdl_data->ifname);

	/* Set preferred bss as wnm neighbor */
	memset(&btq_nbr, 0, sizeof(btq_nbr));
	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strncpy(ioctl_buf, "wnm_btq_nbr_add", sizeof(ioctl_buf));
	ioctl_buf[sizeof(ioctl_buf) - 1] = '\0';
	buflen = strlen("wnm_btq_nbr_add") + 1;

	btq_nbr.version = WL_RRM_NBR_RPT_VER;
	memcpy(&(btq_nbr.bssid), &(ioctl_data->bssid), ETHER_ADDR_LEN);
	htol32_ua_store(ioctl_data->bssid_info, &(btq_nbr.bssid_info));
	btq_nbr.reg = ioctl_data->rclass;
	btq_nbr.channel = wf_chspec_ctlchan(ioctl_data->chanspec);
	btq_nbr.phytype = ioctl_data->phytype;
	btq_nbr.chanspec = htod16(ioctl_data->chanspec);
	btq_nbr.bss_trans_preference = DOT11_NGBR_BSSTRANS_PREF_SE_HIGHEST;

	memcpy(&ioctl_buf[buflen], &btq_nbr, sizeof(nbr_rpt_elem_t));
	buflen += sizeof(nbr_rpt_elem_t);

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_VAR, ioctl_buf,
		buflen, hdl_data->data);

	if (ret < BCME_OK) {
		/* If driver doesnt support this then fallback to action frame
		 * mechnism of sending bss transition request.
		*/
		WLIF_BSSTRANS("%s: Error in iovar wnm_btq_nbr_add. ret=%d. Fallback to BTM Request "
			"with actframe\n", hdl_data->ifname, ret);
		goto actframe;
	}
	WLIF_BSSTRANS("%s: Added neighbor ["MACF"] BSSInfo=0x%X Operating class=%d Channel=%d "
		"Phytype=%d Chanspec=0x%X\n", hdl_data->ifname, ETHER_TO_MACF(btq_nbr.bssid),
		btq_nbr.bssid_info, btq_nbr.reg, btq_nbr.channel, btq_nbr.phytype,
		btq_nbr.chanspec);

	/* Send BSS Transition Request */
	memset(&bsstrans_req, 0, sizeof(bsstrans_req));
	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strncpy(ioctl_buf, "wnm_bsstrans_req", sizeof(ioctl_buf));
	ioctl_buf[sizeof(ioctl_buf) - 1] = '\0';
	buflen = strlen("wnm_bsstrans_req") + 1;

	memcpy(&bsstrans_req.sta_mac, &(ioctl_data->addr), ETHER_ADDR_LEN);
	bsstrans_req.reqmode = DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL;

	if (ioctl_data->flags & WLIFU_BSS_TRANS_FLAGS_BTM_ABRIDGED) {
		/* set bit1 to tell STA the BSSID in list recommended */
		bsstrans_req.reqmode |= DOT11_BSSTRANS_REQMODE_ABRIDGED;
	}

	if (ioctl_data->flags & WLIFU_BSS_TRANS_FLAGS_DISASSOC_IMNT) {
		bsstrans_req.reqmode |= DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT;
		bsstrans_req.tbtt = ioctl_data->disassoc_timer;
	}

	/* Add steering reason */
	bsstrans_req.reason = ioctl_data->reason;

	if (++bss_token == 0)
		bss_token = 1;
	bsstrans_req.token = bss_token;

	memcpy(&ioctl_buf[buflen], &bsstrans_req, sizeof(bsstrans_req));
	buflen += sizeof(bsstrans_req);

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_VAR, ioctl_buf,
		buflen, hdl_data->data);

	if (ret < BCME_OK) {
		/* If driver doesnt support this then fallback to action frame
		 * mechnism of sending bss transition request.
		*/
		WLIF_BSSTRANS("%s: Error in iovar wnm_bsstrans_req. ret=%d. Fallback to BTM Request"
				" with actframe\n", hdl_data->ifname, ret);
		goto actframe;
	}

	WLIF_BSSTRANS("%s: Sent BTM Request via wnm_bsstrans_req. to "MACF". Token=%d, "
		"Reqmode=0x%X, Disassoc bcn count=%d.\n", hdl_data->ifname,
		ETHER_TO_MACF(bsstrans_req.sta_mac),
		bsstrans_req.token, bsstrans_req.reqmode, bsstrans_req.tbtt);

	/* Add steer request to list to handle response */
	ret = wl_wlif_add_steer_req_to_list(hdl_data, bss_token, ioctl_data);

	/* Find OUI ,vendor and do post bsstransreq after send BTM request */
	wl_wlif_create_post_bss_transreq_timer(hdl_data, ioctl_data);
	return ret;

actframe:
	return wl_wlif_send_bss_transreq_actframe(hdl_data, ioctl_data);
}

/* create blocking list of macs. Return 1 if there is any change to the MAC list, else return 0.
 * i.e if the STA is added to list or removed from the list, return 1.
 */
static int
wl_update_block_mac_list(maclist_t *static_maclist, maclist_t *maclist,
	int macmode, struct ether_addr *addr, unsigned char block)
{
	int is_changed = 1, is_mac_present = 0;
	uint16 count = 0, idx, action;

	/* Action table
	 * ALLOW  BLOCK		DEL
	 * DENY   BLOCK		ADD
	 * ALLOW  UNBLOCK	ADD
	 * DENY   UNBLOCK	DEL
	 */
	action = ((macmode == WLC_MACMODE_ALLOW) ^ block) ? WL_MAC_ADD : WL_MAC_DEL;

	WLIF_BSSTRANS("%sblocking STA "MACF" by %s the maclist. macmode=%d\n", block ? "" : "Un",
		ETHERP_TO_MACF(addr), action ? "adding to" : "deleting from", macmode);

	switch (action) {
		case WL_MAC_DEL:
			for (idx = 0; idx < static_maclist->count; idx++) {
				if (eacmp(addr, &static_maclist->ea[idx]) == 0) {
					is_mac_present = 1;
					continue;
				}
				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
					ETHER_ADDR_LEN);
				count++;
			}
			maclist->count = count;
			/* If the MAC address is not present in the list, there is
			 * no change as there is nothing to delete
			 */
			if (!is_mac_present) {
				 is_changed = 0;
			}
			break;

		case WL_MAC_ADD:
			for (idx = 0; static_maclist != NULL && idx < static_maclist->count;
					idx++) {
				/* Avoid duplicate entry in maclist. */
				if (eacmp(addr, &static_maclist->ea[idx]) == 0) {
					is_mac_present = 1;
					continue;
				}

				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
					ETHER_ADDR_LEN);
				count++;
			}

			memcpy(&(maclist->ea[count]), addr,  ETHER_ADDR_LEN);
			count++;
			maclist->count = count;
			/* If the MAC address already present in the list, there is
			 * no change as there is nothing to add
			 */
			if (is_mac_present) {
				is_changed = 0;
			}
		break;

		default:
			printf("Wrong action= %d\n", action);
			assert(0);
	}

	return is_changed;
}

/* Create a Timer */
static int
wl_wlif_create_timer(void *uschd_hdl, bcm_timer_module_id timer_hdl,
	void* cbfn, void *data, int timeout, int repeat, bcm_timer_id *out_timer_id)
{
	int ret = -1;
	bcm_timer_id timer_id;
	struct itimerspec  its;

	/* If uschd library is not used, Create linux timer */
	if (uschd_hdl == NULL) {

		/* Cast cbfn to relevant callback function */
		wlif_linux_timer_cb* linux_cbfn = (wlif_linux_timer_cb*)cbfn;

		/* Create Timer */
		ret = bcm_timer_create(timer_hdl, &timer_id);
		if (ret) {
			return FALSE;
		}

		/* Connect Timer */
		ret = bcm_timer_connect(timer_id, (bcm_timer_cb)linux_cbfn, (uintptr_t)data);
		if (ret) {
			return FALSE;
		}

		/* Set up retry timer */
		its.it_interval.tv_sec = timeout;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = timeout;
		its.it_value.tv_nsec = 0;
		ret = bcm_timer_settime(timer_id, &its);
		if (ret) {
			return FALSE;
		}

		if (out_timer_id) {
			*out_timer_id = timer_id;
		}

	/* If uschd library is used, Create uschd timer */
	} else {

		/* Cast cbfn to relevant callback function */
		wlif_usched_timer_cb* usched_cbfn = (wlif_usched_timer_cb*)cbfn;

		/* Create Timer */
		ret = bcm_usched_add_timer(uschd_hdl,
			(SEC_TO_MICROSEC((unsigned long long)timeout)),
			repeat, usched_cbfn, (void*)data);

		if (ret != BCM_USCHEDE_OK) {
			WLIF_BSSTRANS("Failed to add timer Err:%s\n", bcm_usched_strerror(ret));
			return FALSE;
		}
	}

	return TRUE;
}

static int
wl_wlif_add_macblock_to_file(wl_wlif_hdl_data_t *hdl, struct ether_addr addr,
	int timeout, short flag)
{
	FILE *fp;
	char fname[PATH_MAX];
	char line[STEERINGFILE_LINE_LEN], eabuf[ETHER_ADDR_STR_LEN];
	maclist_entry_t entry;
	bool add = TRUE;

	wl_wlif_get_maclist_filename(hdl, fname);
	if (!(fp = fopen(fname, "a+"))) {
		WLIF_BSSTRANS("Error opening file %s: %s\n", fname, strerror(errno));
		return -1;
	}
	bcm_ether_ntoa(&addr, eabuf);
	while (fgets(line, sizeof(line), fp) != NULL) {
		wl_wlif_get_maclist_entry(line, &entry);
		if ((strncmp(entry.macaddr, eabuf, strlen(entry.macaddr)) == 0) &&
			(strncmp(entry.ifname, hdl->ifname, strlen(entry.ifname)) == 0)) {
			add = FALSE;
			break;
		}
	}
	if (add) {
		snprintf(line, sizeof(line), STEERINGFILE_READWRITE_FORMAT, hdl->ifname, eabuf,
			(time(NULL) + timeout), flag);
		fputs(line, fp);
		WLIF_BSSTRANS("%s: add blocking "MACF" for %d seconds in %s\n", hdl->ifname,
			ETHER_TO_MACF(addr), timeout, fname);
	}
	fclose(fp);

	return 0;
}
static int
wl_wlif_remove_macblock_from_file(wl_wlif_hdl_data_t *hdl, struct ether_addr addr, short flag)
{
	FILE *fp, *fp_tmp;
	char fname[PATH_MAX];
	char tmpfname[PATH_MAX];
	char line[STEERINGFILE_LINE_LEN], eabuf[ETHER_ADDR_STR_LEN];
	maclist_entry_t entry;

	wl_wlif_get_maclist_filename(hdl, fname);
	if (!(fp = fopen(fname, "r"))) {
		WLIF_BSSTRANS("Error opening file %s: %s\n", fname, strerror(errno));
		return -1;
	}
	snprintf(tmpfname, PATH_MAX, STEERINGFILE_NAME_FORMAT, STEERINGFILE_DIR_NAME,
		STEERINGFILE_TMPPREFIX, (hdl->appname), STEERINGFILE_POSTFIX);
	if (!(fp_tmp = fopen(tmpfname, "w"))) {
		WLIF_BSSTRANS("Error opening file %s: %s\n", tmpfname, strerror(errno));
		fclose(fp);
		return -1;
	}
	bcm_ether_ntoa(&addr, eabuf);

	while (fgets(line, sizeof(line), fp) != NULL) {
		wl_wlif_get_maclist_entry(line, &entry);
		if ((strncmp(entry.macaddr, eabuf, strlen(entry.macaddr)) == 0) &&
			(strncmp(entry.ifname, hdl->ifname, strlen(entry.ifname)) == 0)) {
			WLIF_BSSTRANS("%s: remove "MACF" from %s\n", hdl->ifname,
				ETHER_TO_MACF(addr), fname);
			continue;
		}
		fputs(line, fp_tmp);
	}
	fclose(fp);
	fclose(fp_tmp);
	if (remove(fname) != 0) {
		WLIF_BSSTRANS("filename %s not removed %s\n", fname, strerror(errno));
		return -1;
	}
	if (rename(tmpfname, fname) != 0) {
		WLIF_BSSTRANS("filename %s not renamed to %s: %s\n", tmpfname, fname,
			strerror(errno));
		return -1;
	}

	return 0;
}
/*
 * Retrieves the current macmode, maclist and probe resp filter values
 * of interface, updates the data with STA address and sets them again.
 * If timeout is non zero and non negative, then creates timer for
 * unblock mac routine.
 * For negative timeout value unblock timer will not be created.
 * For 0 timeout value sta will not be blocked at all.
 * Only for open networks if nobcnssid disable bit is not set in steer flags
 * than sets nobcnssid iovar.
 */
int
wl_wlif_block_mac(wl_wlif_hdl *hdl, struct ether_addr addr, int timeout)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode, ret, macprobresp, closed = 0;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;
	static short flag;
	wl_wlif_hdl_data_t *hdl_data;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return FALSE;
	}

	/* For 0 timeout don't block. */
	if (!timeout) {
		WLIF_BSSTRANS("Err: timeout is zero \n");
		return FALSE;
	}

	hdl_data = (wl_wlif_hdl_data_t *)hdl;

	wl_endian_probe(hdl_data->ifname);

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACMODE, &(macmode),
		sizeof(macmode), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error in WLC_GET_MACMODE. ret=%d.\n", hdl_data->ifname, ret);
		return FALSE;
	}
	macmode = dtoh32(macmode);

	/* retrive static maclist */
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACLIST, (void *)s_maclist,
		sizeof(maclist_buf), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error in WLC_GET_MACLIST. ret=%d.\n", hdl_data->ifname, ret);
		return FALSE;
	}
	s_maclist->count = dtoh32(s_maclist->count);

	ret = wl_iovar_getint(hdl_data->ifname, "probresp_mac_filter", &macprobresp);
	if (ret != 0) {
		WLIF_BSSTRANS("%s: Error in probresp_mac_filter. ret=%d.\n", hdl_data->ifname, ret);
	}

	if (flag == 0) {
		flag = s_maclist->count;
		flag |= WL_MAC_PROB_MODE_SET(macmode, macprobresp, TRUE);
	}

	WLIF_BSSTRANS("%s: Blocking "MACF" for %d seconds\n", hdl_data->ifname,
		ETHER_TO_MACF(addr), timeout);
	/* Get the updated mac list to be set. If there is no change, then just return */
	if (!wl_update_block_mac_list(s_maclist, maclist, macmode, &addr, TRUE)) {
		WLIF_BSSTRANS("%s MAC list unchanged while blocking "MACF".\n", hdl_data->ifname,
			ETHER_TO_MACF(addr));
		return TRUE;
	}

	if (timeout > 0) {
		/* Create Unblock MAC Timer */
		ret = wl_wlif_create_unblock_mac_timer(hdl_data, addr, timeout, flag);

		/* If Unblock MAC Timer Creation Fails, return Error fm this function */
		if (ret == FALSE) {
			WLIF_BSSTRANS("%s: Failed to create timer for unblocking "MACF" after %d "
				"seconds\n", hdl_data->ifname, ETHER_TO_MACF(addr), timeout);
			return FALSE;
		}
		wl_wlif_add_macblock_to_file(hdl_data, addr, timeout, flag);
	}

	macmode = htod32((macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY);
	maclist->count = htod32(maclist->count);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACLIST, maclist,
		ETHER_ADDR_LEN * maclist->count + sizeof(uint), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error in WLC_SET_MACLIST. ret=%d\n", hdl_data->ifname, ret);
	}

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACMODE, &macmode,
		sizeof(int), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error in WLC_SET_MACMODE. ret=%d\n", hdl_data->ifname, ret);
	}

	ret = wl_iovar_setint(hdl_data->ifname, "probresp_mac_filter", TRUE);
	if (ret != 0) {
		WLIF_BSSTRANS("%s: Error in probresp_mac_filter. ret=%d\n", hdl_data->ifname, ret);
	}

	/* For open networks set nobcnssid iovar */
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_CLOSED, &closed,
		sizeof(closed), hdl_data->data);
	if (ret != 0) {
		WLIF_BSSTRANS("%s: Error in WLC_GET_CLOSED. ret=%d\n", hdl_data->ifname, ret);
	}
	if (!closed) {
		if (IS_NOBCNSSID_ENABLED(hdl_data->steer_flags) && !wl_wlif_set_nobcnssid(hdl)) {
			WLIF_BSSTRANS("%s: Error in setting nobcnssid\n", hdl_data->ifname);
		}
	}

	return TRUE;
}

/*
 * If BSS-Trans response is not ACCEPT, or if it is ACCEPT but still persists in Assoclist
 *  invokes Block MAC and Disassociates STA from current BSS
 */
static int
wl_wlif_block_mac_and_disassoc(wl_wlif_hdl *hdl, struct ether_addr addr,
	int timeout, int disassoc)
{
	int ret = -1;
	wl_wlif_hdl_data_t *hdl_data;

	/* Get wlif Handle */
	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		goto end;
	}
	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	/* Block STA on Source BSSID */
	wl_wlif_block_mac(hdl_data, addr, timeout);

	/* If Brute Force Steer is applicable, for this Response Type, Send Disassoc */
	if (disassoc) {

		/* Send Disassoc to STA from Source BSSID */
		WLIF_BSSTRANS("%s: Sending disassoc to STA "MACF"\n", hdl_data->ifname,
			ETHER_TO_MACF(addr));
		ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_DISASSOC,
			&(addr), ETHER_ADDR_LEN, hdl_data->data);

		if (ret < 0) {
			WLIF_BSSTRANS("%s: WLC_DISASSOC to STA "MACF" failed\n", hdl_data->ifname,
				ETHER_TO_MACF(addr));
			goto end;
		}
	}

end:
	return ret;
}

/* Set the Channel utilization value */
static int
wl_set_channel_util(wl_wlif_hdl *hdl, uint8 chan_util)
{
	int ret = -1;
	char  *param, maclist_buf[WLC_IOCTL_MAXLEN];
	wl_wlif_hdl_data_t *hdl_data;
	char *iovar_name = "bssload_fake_chan_util";

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return ret;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	memset(maclist_buf, 0, sizeof(maclist_buf));
	strcpy(maclist_buf, iovar_name);
	param = (char *)(maclist_buf + strlen(iovar_name) + 1);
	*param = chan_util;	/* Initialize the channel utilization value */

	WLIF_BSSTRANS("%s: Setting bssload_fake_chan_util to %d\n",
		hdl_data->ifname, chan_util);

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_VAR, (void *)maclist_buf,
			sizeof(maclist_buf), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error setting bssload_fake_chan_util\n",
			hdl_data->ifname);
		return ret;
	}
	return  ret;
}

/* Set the Channel utilization value  in Bss load element */
static int
wl_wlif_set_fake_chan_util_for_sta(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data)
{
	char *value;
	uint8 fake_chan_util = WLIFU_DEF_FAKE_CHAN_UTIL;

	/* Read NVRAM for fake channel utilization */
	value = nvram_safe_get(WLIFU_NVRAM_FAKE_CHAN_UTIL);
	if (value[0] != '\0') {
		fake_chan_util = (uint8)strtoul(value, NULL, 0);
	}

	return wl_set_channel_util(hdl, fake_chan_util);
}

/* Clear the Channel utilization value  in Bss load element */
static int
wl_wlif_clear_fake_chan_util_for_sta(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data)
{
	return wl_set_channel_util(hdl, 0);
}

static int
wl_wlif_do_block_mac(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data)
{
	wl_wlif_hdl_data_t *hdl_data;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return -1;
	}
	hdl_data = (wl_wlif_hdl_data_t*)hdl;
	/* Block mac in curr interface. */
	wl_wlif_block_mac(hdl_data, ioctl_hndlr_data->addr, ioctl_hndlr_data->timeout);
	return 0;
}

static int
wl_wlif_do_unblock_mac(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data)
{
	wl_wlif_hdl_data_t *hdl_data;
	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return -1;
	}
	hdl_data = (wl_wlif_hdl_data_t*)hdl;
	/* Block mac in curr interface. */
	wl_wlif_unblock_mac(hdl_data, ioctl_hndlr_data->addr, 0);
	return 0;
}

/*
 * wnm_oui_action_config
 * wnm_oui_action_config format: xx:xx:xx,oui_action_value1,oui_action_value2,timeout  ".
 */

static void
wl_retrieve_wnm_oui_action_config(void)
{
	char var[80], *next, *pstr;
	char *addr, *action1, *action2, *timeout;
	char *endptr = NULL;
	sta_oui_wnm_act_t oui_act, *ptr, *head, *current;

	ptr = oui_action;
	head = oui_action;
	foreach(var, nvram_safe_get("wnm_oui_action_config"), next) {
		if (strlen(var) < OUI_ACT_STRLEN) {
			WLIF_BSSTRANS("wnm_oui_action format error: %s\n", var);
			break;
		}
		pstr = var;
		if (pstr != NULL) {
			addr = strsep(&pstr, ",");
			action1 = strsep(&pstr, ",");
			action2 = strsep(&pstr, ",");
			timeout = pstr;
			WLIF_BSSTRANS("addr : %s, action :%s, action2 : %s  timeout : %s \n", addr,
					action1, action2, timeout);
			oui_act.pre_bt_action = (uint8)strtol(action1, &endptr, 0);
			oui_act.post_bt_action = (uint8)strtol(action2, &endptr, 0);
			oui_act.post_bt_timeout = (uint8)strtol(timeout, &endptr, 0);

			if (oui_atoe(addr, (unsigned char *)(&(oui_act.oui)))) {
				ptr = malloc(sizeof(sta_oui_wnm_act_t));
				if (!ptr) {
					WLIF_BSSTRANS("Malloc Err:%s\n", __FUNCTION__);
					break;
				}
				memset(ptr, 0, sizeof(sta_oui_wnm_act_t));
				memcpy(&ptr->oui, &oui_act.oui, sizeof(oui_act.oui));
				ptr->pre_bt_action = oui_act.pre_bt_action;
				ptr->post_bt_action =
					oui_act.post_bt_action;
				ptr->post_bt_timeout = oui_act.post_bt_timeout;
				if ((ptr->pre_bt_action >= WNM_ACTION_SIZE) ||
					(ptr->post_bt_action >= WNM_ACTION_SIZE)) {
					ptr->pre_bt_action = 0;
					ptr->post_bt_action = 0;
					ptr->post_bt_timeout = 0;
				}
				if (!head)
				{
					ptr->next = head;
					head = ptr;
				}
				else {
					current = head;
					while (current->next) {
						current = current->next;
					}
					ptr->next = current->next;
					current->next = ptr;
				}
				oui_action = head;
			}
		}
	}
}

/* Compare the OUI with table and current STA MAC, Based on the Vendor
 *   do the procees further
 */
static int
wl_wlif_pre_bss_transreq(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data)
{
	uint8 action;
	sta_oui_wnm_act_t *ptr = oui_action;
	unsigned char generic_oui[OUI_ADDR_LEN];	/* Generic OUI */

	if (!ptr) {
		return  0;
	}
	oui_atoe(OUI_GENERAL, generic_oui);
	while (ptr) {
		if ((!oui_cmp(ptr->oui, &ioctl_hndlr_data->addr)) ||
			(!oui_cmp(ptr->oui, generic_oui))) {
			action = ptr->pre_bt_action;
			WLIF_BSSTRANS("oui_action = %d , oui_address =%02x:%02x:%02x \n",
				ptr->pre_bt_action, ptr->oui[0], ptr->oui[1], ptr->oui[2]);
			if (wnm_actions[action] != NULL) {
				wnm_actions[action](hdl, ioctl_hndlr_data);
			}
		}
		ptr = ptr->next;
	}
	return 0;
}

/* Compare the OUI with table and current STA MAC, Based on the Vendor
 *   clean the IOVAR variable
 */
static int
wl_wlif_post_bss_transreq(wl_wlif_hdl *hdl, struct ether_addr *addr)
{
	uint8 action;
	wl_wlif_bss_trans_data_t ioctl_hndlr_data;
	sta_oui_wnm_act_t *ptr = oui_action;
	wl_wlif_hdl_post_bt_t *data;
	unsigned char generic_oui[OUI_ADDR_LEN];	/* Generic OUI */

	if (!ptr) {
		return 0;
	}
	oui_atoe(OUI_GENERAL, generic_oui);
	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return -1;
	}
	data = (wl_wlif_hdl_post_bt_t*)hdl;
	memset(&ioctl_hndlr_data, 0, sizeof(ioctl_hndlr_data));
	eacopy(addr, &(ioctl_hndlr_data.addr));
	while (ptr) {
		if (((!oui_cmp(ptr->oui, &ioctl_hndlr_data.addr)) ||
				(!oui_cmp(ptr->oui, generic_oui))) &&
				(data->action == ptr->post_bt_action)) {
			action = ptr->post_bt_action;
			WLIF_BSSTRANS("oui_action = %d , oui_address =%02x:%02x:%02x \n",
				ptr->post_bt_action, ptr->oui[0], ptr->oui[1], ptr->oui[2]);
			if (wnm_actions[action] != NULL) {
				wnm_actions[action](data->hdl_data, &ioctl_hndlr_data);
			}
		}
		ptr = ptr->next;
	}
	return 0;
}

/* BSS-Trans routine for sending BTM request to the STA. This routine will return immediately
 * after sending the request. Applications should pass "post_bss_trans_resp_cb" or
 * "bss_trans_resp_hndlr" callback in "wl_wlif_bss_trans_data_t" structure to get the response.
 * Applications should listen for "WLC_E_BSSTRANS_RESP" event and pass the event data to library to
 * get the BTM response. Else, the library will call those above registered callbacks to inform
 * that there is no response(steer_resp type is WLIFU_BSS_TRANS_RESP_UNKNOWN).
 */
int
wl_wlif_do_bss_trans(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *ioctl_hndlr_data)
{
	int bsstrans_ret = -1;
	wl_wlif_hdl_data_t *hdl_data = NULL;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		goto end;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	/* Check if Steering STA exists in ASSOCLIST */
	if (wl_wlif_find_sta_in_assoclist(hdl_data, &ioctl_hndlr_data->addr) == FALSE) {
		WLIF_BSSTRANS("%s: STA "MACF" not associated\n",
			hdl_data->ifname, ETHER_TO_MACF(ioctl_hndlr_data->addr));
		bsstrans_ret = WLIFU_BSS_TRANS_RESP_UNKNOWN;
		goto end;
	}

	/* Block mac in curr interface. */
	wl_wlif_block_mac(hdl_data, ioctl_hndlr_data->addr, ioctl_hndlr_data->timeout);

	/* Find OUI, vendor and set IOVAR before send BTM request */
	wl_wlif_pre_bss_transreq(hdl_data, ioctl_hndlr_data);

#ifdef BCM_CEVENT
	/* cevent sent with target bssid for BTM Request from app */
	send_cevent(hdl_data->ifname, NULL, &ioctl_hndlr_data->bssid,
		0, ioctl_hndlr_data->reason, 0, CEVENT_TYPE_A2C, CEVENT_A2C_ST_BTM_REQ,
		CEVENT_FRAME_DIR_TX, NULL, 0);
#endif /* BCM_CEVENT */

	/* Create & Send BSS Transition Action Frame & Return the Response */
	bsstrans_ret = wl_wlif_send_bss_transreq(hdl_data, ioctl_hndlr_data);
	WLIF_BSSTRANS("%s-%s: BSS Transition Req to STA "MACF" Target BSSID "MACF" %ssent\n",
		hdl_data->appname, hdl_data->ifname, ETHER_TO_MACF(ioctl_hndlr_data->addr),
		ETHER_TO_MACF(ioctl_hndlr_data->bssid),
		((bsstrans_ret != BCME_OK) ? "Not " :""));

end:
	return bsstrans_ret;
}

/*
 * Retrieves the current macmode, maclist and proberesp filter values.
 * Updates maclist with STA addr entry. Based on flag bits
 * updates macmode and probe resp filter values.
 */
int
wl_wlif_unblock_mac(wl_wlif_hdl *hdl, struct ether_addr addr, int flag)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode, ret = 0;
	wl_wlif_hdl_data_t *hdl_data;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle \n");
		return FALSE;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	wl_endian_probe(hdl_data->ifname);

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACMODE, &(macmode),
		sizeof(macmode), hdl_data->data);
	if (ret != 0) {
		WLIF_BSSTRANS("%s: Error in WLC_GET_MACMODE. ret=%d.\n", hdl_data->ifname, ret);
		return FALSE;
	}
	macmode = dtoh32(macmode);

	/* retrive static maclist */
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_MACLIST, (void *)s_maclist,
		sizeof(maclist_buf), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error in WLC_GET_MACLIST. ret=%d.\n", hdl_data->ifname, ret);
		return FALSE;
	}
	s_maclist->count = dtoh32(s_maclist->count);

	wl_update_block_mac_list(s_maclist, maclist, macmode, &(addr), FALSE);
	macmode = (macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY;

	if (flag && (maclist->count == (flag & 0xFF))) {
		macmode = WL_MACMODE_GET(flag);
		ret = wl_iovar_setint(hdl_data->ifname, "probresp_mac_filter",
			WL_MACPROBE_GET(flag));
		if (ret != 0) {
			WLIF_BSSTRANS("%s: Error in probresp_mac_filter. ret=%d.\n",
				hdl_data->ifname, ret);
		}
	}

	maclist->count = htod32(maclist->count);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACLIST, maclist,
		ETHER_ADDR_LEN * maclist->count + sizeof(uint), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error in WLC_SET_MACLIST. ret=%d\n", hdl_data->ifname, ret);
	}

	macmode = htod32(macmode);
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_SET_MACMODE, &macmode,
		sizeof(int), hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: Error in WLC_SET_MACMODE. ret=%d\n", hdl_data->ifname, ret);
	}
	wl_wlif_remove_macblock_from_file(hdl_data, addr, flag);

	return TRUE;
}

/* Callback handler for unblock mac */
static void
wl_wlif_unblock_mac_cb(bcm_timer_id timer_id, void* arg)
{
	static_maclist_t *data = (static_maclist_t*)arg;

	wl_wlif_unblock_mac(data->hdl_data, data->addr, data->flag);

	free(data);
	bcm_timer_delete(timer_id);
}

/* Callback handler for unblock mac */
static void
wl_wlif_unblock_mac_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	static_maclist_t *data = (static_maclist_t*)arg;

	wl_wlif_unblock_mac(data->hdl_data, data->addr, data->flag);

	free(data);
}

/* Create Unblock MAC Timer */
static int
wl_wlif_create_unblock_mac_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout, int flag)
{
	int ret = FALSE;
	static_maclist_t *tmr_data = NULL;
	bcm_timer_id timer_id;

	/* Allocate Timer Arg Struct Object */
	tmr_data = (static_maclist_t *) calloc(1, sizeof(*tmr_data));
	if (!tmr_data) {
		WLIF_BSSTRANS("Err: calloc Timer Arg Failed \n");
		return FALSE;
	}

	/* Initialize Timer Arg Struct Object */
	tmr_data->hdl_data = hdl_data;
	tmr_data->flag = flag;
	eacopy(&addr, &(tmr_data->addr));

	/* Create an Appropriate Timer, to Unblock a MAC */
	ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
		(hdl_data->uschd_hdl ? (void*)wl_wlif_unblock_mac_usched_cb :
		(void*)wl_wlif_unblock_mac_cb), (void*)tmr_data, timeout, FALSE, &timer_id);
	if (!ret) {
		WLIF_BSSTRANS("%s: Error creating timer to unblock STA "MACF". ret=%d\n",
			hdl_data->ifname, ETHER_TO_MACF(addr), ret);
		free(tmr_data);
		return FALSE;
	}
	tmr_data->timer_id = timer_id;
	hdl_data->timer_data = tmr_data;

	return TRUE;
}

/* Remove unblock MAC timer */
static void
wl_wlif_remove_unblock_timer(wl_wlif_hdl_data_t *hdl_data)
{
	static_maclist_t *data;

	if (!hdl_data || !hdl_data->timer_data) {
		return;
	}

	data = (static_maclist_t*)hdl_data->timer_data;
	WLIF_BSSTRANS("%s: Remove unblock MAC timer for STA "MACF"\n",
		hdl_data->ifname, ETHER_TO_MACF(data->addr));

	/* Unblock the MAC */
	wl_wlif_unblock_mac(hdl_data, data->addr, data->flag);

	/* remove the timer */
	if (hdl_data->uschd_hdl == NULL) {
		bcm_timer_delete(data->timer_id);
	} else {
		bcm_usched_remove_timer(hdl_data->uschd_hdl, wl_wlif_unblock_mac_usched_cb, data);
	}

	/* Free the argument */
	free(data);

	hdl_data->timer_data = NULL;
}

/* Traverse Maclist to find MAC address exists or not */
static bool
wl_wlif_find_sta_in_assoclist(wl_wlif_hdl_data_t *hdl_data, struct ether_addr *find_mac)
{
	int ret = 0, iter = 0;
	bool sta_found = FALSE;
	struct maclist *list = NULL;

	wl_endian_probe(hdl_data->ifname);

	/* Prepare Assoclist IOCTL Structure Object */
	list = (struct maclist *)calloc(1, WL_WLIF_BUFSIZE_4K);
	if (!list) {
		WLIF_BSSTRANS("Err: calloc Assoclist IOCTL buff Failed \n");
		goto end;
	}
	list->count = htod32((WL_WLIF_BUFSIZE_4K - sizeof(int)) / ETHER_ADDR_LEN);

	/* Read Assoclist */
	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_GET_ASSOCLIST, list,
		WL_WLIF_BUFSIZE_4K, hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: WLC_GET_ASSOCLIST Failed. ret=%d\n", hdl_data->ifname, ret);
		goto end;
	}

	list->count = dtoh32(list->count);
	if (list->count <= 0) {
		goto end;
	}

	/* Travese maclist items */
	for (iter = 0; iter < list->count; iter++) {

		/* STA Checking Condition */
		if (eacmp(&list->ea[iter], find_mac) == 0) {
			sta_found = TRUE;
			goto end;
		}
	}

end:
	if (list) {
		free(list);
	}
	return sta_found;
}

/* Run Defiant STA Watchdog */
static void
wl_wlif_run_defiant_sta_watchdog(void* arg)
{
	bool sta_found = FALSE;
	static_maclist_t *defiant_tmr_data = (static_maclist_t*)arg;

	/* Check if Steering STA is still persisting in ASSOCLIST of its Source AP */
	sta_found = wl_wlif_find_sta_in_assoclist(defiant_tmr_data->hdl_data,
		&defiant_tmr_data->addr);

	if (sta_found) {
		WLIF_BSSTRANS("%s: STA "MACF" still associated. Disassoc him\n",
			defiant_tmr_data->hdl_data->ifname, ETHER_TO_MACF(defiant_tmr_data->addr));
		/* If Brute Force is applicable, for this Response Type, Do Brute Force way */
		wl_wlif_block_mac_and_disassoc(defiant_tmr_data->hdl_data, defiant_tmr_data->addr,
			defiant_tmr_data->timeout, 1);
	}

	/* Free Timer Arg Struct Object */
	free(defiant_tmr_data);

}

/* For LINUX Timer, Callback handler to Start Defiant STA Watchdog */
static void
wl_wlif_defiant_sta_wd_cb(bcm_timer_id timer_id, void* arg)
{
	/* Run Defiant STA Watchdog */
	wl_wlif_run_defiant_sta_watchdog(arg);

	bcm_timer_delete(timer_id);
}

/* For USCHED Timer, Callback handler to Start Defiant STA Watchdog */
static void
wl_wlif_defiant_sta_wd_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	/* Run Defiant STA Watchdog */
	wl_wlif_run_defiant_sta_watchdog(arg);
}

/* Create Defiant STA Watchdog Timer */
static int
wl_wlif_create_defiant_sta_watchdog_timer(wl_wlif_hdl_data_t *hdl_data,
	struct ether_addr addr, int timeout)
{
	int ret = FALSE, tm_defiant_wd = WLIFU_DEF_TM_DEFIANT_WD;
	static_maclist_t *defiant_tmr_data = NULL;
	char *value = NULL;

	/* Read NVRAM for Dafiant STA Watchdog Timer Tick */
	value = nvram_safe_get(WLIFU_NVRAM_TM_DEFIANT_WD);
	if (value[0] != '\0') {
		tm_defiant_wd = (int)strtoul(value, NULL, 0);
	}

	/* Allocate Timer Arg Struct Object */
	defiant_tmr_data = (static_maclist_t *) calloc(1, sizeof(*defiant_tmr_data));
	if (!defiant_tmr_data) {
		WLIF_BSSTRANS("Err: calloc Timer Arg Failed \n");
		return FALSE;
	}

	/* Initialize Timer Arg Struct Object */
	defiant_tmr_data->hdl_data = hdl_data;
	defiant_tmr_data->flag = 0;
	defiant_tmr_data->timeout = timeout;
	eacopy(&(addr), &(defiant_tmr_data->addr));

	/* Create an Appropriate Timer, For Defiant STA Watchdog */
	ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
		(hdl_data->uschd_hdl ? (void*)wl_wlif_defiant_sta_wd_usched_cb :
		(void*)wl_wlif_defiant_sta_wd_cb), (void*)defiant_tmr_data, tm_defiant_wd, FALSE,
		NULL);
	if (!ret) {
		WLIF_BSSTRANS("%s: Error creating timer for defiant STA "MACF". ret=%d\n",
			hdl_data->ifname, ETHER_TO_MACF(addr), ret);
		free(defiant_tmr_data);
		return FALSE;
	}

	return TRUE;
}

/* Unsets the nobcnssid iovar */
static bool
wl_wlif_unset_nobcnssid(wl_wlif_hdl *hdl)
{
	wl_wlif_hdl_data_t *hdl_data = (wl_wlif_hdl_data_t*)hdl;
	int ret;

	if (IS_FAKE_CHAN_UTIL_ENABLED(hdl_data->steer_flags) &&
		!(ret = wl_wlif_clear_fake_chan_util_for_sta(hdl, NULL)))
	{
		WLIF_BSSTRANS("%s: fake chan util clear failed. ret=%d\n", hdl_data->ifname, ret);
	}

	ret = wl_iovar_setint(hdl_data->ifname, "nobcnssid", 0);
	if (ret != 0) {
		WLIF_BSSTRANS("%s: nobcnssid unset failed. ret=%d\n", hdl_data->ifname, ret);
		return FALSE;
	}

	return TRUE;
}

/* Callback fn for unsetting nobcnssid iovar */
static void
wl_wlif_unset_nobcnssid_cb(bcm_timer_id timer_id, void* arg)
{
	static_maclist_t *data = (static_maclist_t*)arg;

	wl_wlif_unset_nobcnssid(data->hdl_data);
	free(data);
	bcm_timer_delete(timer_id);
}

/* Callback fn for unsetting nobcnssid iovar */
static void
wl_wlif_unset_nobcnssid_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	static_maclist_t *data = (static_maclist_t*)arg;

	wl_wlif_unset_nobcnssid(data->hdl_data);
	free(data);
}

/* Creates the timer to unset the nobcnssid iovar */
static int
wl_wlif_create_nobcnssid_timer(wl_wlif_hdl_data_t *hdl_data, int timeout)
{
	int ret = FALSE;
	static_maclist_t *tm_data = NULL;

	tm_data = (static_maclist_t *) calloc(1, sizeof(*tm_data));
	if (!tm_data) {
		WLIF_BSSTRANS("Err: calloc failed \n");
		return FALSE;
	}

	tm_data->hdl_data = hdl_data;

	/* Create timer, either ushed or linux based on handle */
	ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
		(hdl_data->uschd_hdl ? (void*)wl_wlif_unset_nobcnssid_usched_cb :
		(void*)wl_wlif_unset_nobcnssid_cb), tm_data, timeout, FALSE, NULL);
	if (!ret) {
		WLIF_BSSTRANS("%s: Error creating nobcnssid timer. ret=%d\n", hdl_data->ifname,
			ret);
		free(tm_data);
		return FALSE;
	}

	return TRUE;
}

/* Sets nobcnssid iovar to make ssid empty in beacon frames.
 * Creates a timer to unset it, If valid timeout value is stored in
 * nvram nobcnssid_timeout it will be used otherwise default is 10 secs.
 */
static int
wl_wlif_set_nobcnssid(wl_wlif_hdl *hdl)
{
	wl_wlif_hdl_data_t *hdl_data;
	int timeout = atoi(nvram_safe_get(WLIFU_NVRAM_NOBCNSSID_TIMEOUT));
	int ret;

	if (!hdl) {
		WLIF_BSSTRANS("Err: Invalid wlif handle\n");
		return FALSE;
	}

	hdl_data = (wl_wlif_hdl_data_t*)hdl;

	wl_endian_probe(hdl_data->ifname);

	if (IS_FAKE_CHAN_UTIL_ENABLED(hdl_data->steer_flags) &&
		!(ret = wl_wlif_set_fake_chan_util_for_sta(hdl, NULL)))
	{
		WLIF_BSSTRANS("%s: fake chan util set failed. ret=%d\n", hdl_data->ifname, ret);
	}

	ret = wl_iovar_setint(hdl_data->ifname, "nobcnssid", 1);
	if (ret != 0) {
		WLIF_BSSTRANS("%s: nobcnssid set failed. ret=%d\n", hdl_data->ifname, ret);
		if (IS_FAKE_CHAN_UTIL_ENABLED(hdl_data->steer_flags)) {
			wl_wlif_clear_fake_chan_util_for_sta(hdl, NULL);
		}
		return FALSE;
	}

	if (timeout <= 0) {
		timeout =  WLIFU_DEF_NOBCNSSID_TIMEOUT;
	}

	/* In case of timer creation failure unset the nobcnssid iovar */
	if (!wl_wlif_create_nobcnssid_timer(hdl_data, timeout)) {
		if (IS_FAKE_CHAN_UTIL_ENABLED(hdl_data->steer_flags)) {
			wl_wlif_clear_fake_chan_util_for_sta(hdl, NULL);
		}
		if (!wl_wlif_unset_nobcnssid(hdl)) {
			return FALSE;
		}
	}

	return TRUE;
}

/* Callback handler for disassoc mac */
static void
wl_wlif_disassoc_mac_cb(bcm_timer_id timer_id, void* arg)
{
	disassoc_t *data = (disassoc_t*)arg;
	wl_wlif_hdl_data_t *hdl_data = data->hdl_data;
	int ret = 0;

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_DISASSOC, &(data->addr),
		ETHER_ADDR_LEN, hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: WLC_DISASSOC to STA "MACF" failed. ret=%d\n", hdl_data->ifname,
			ETHER_TO_MACF(data->addr), ret);
	}

	free(data);
	bcm_timer_delete(timer_id);
}

/* Callback handler for disassoc mac */
static void
wl_wlif_disassoc_mac_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	disassoc_t *data = (disassoc_t*)arg;
	wl_wlif_hdl_data_t *hdl_data = data->hdl_data;
	int ret = 0;

	ret = hdl_data->ioctl_hndlr(hdl_data->ifname, WLC_DISASSOC, &(data->addr),
		ETHER_ADDR_LEN, hdl_data->data);
	if (ret < 0) {
		WLIF_BSSTRANS("%s: WLC_DISASSOC to STA "MACF" failed. ret=%d\n", hdl_data->ifname,
			ETHER_TO_MACF(data->addr), ret);
	}

	free(data);
}

/* For LINUX Timer, Callback handler to trigger post bss transition request */
static void
wl_wlif_post_bss_transreq_cb(bcm_timer_id timer_id, void* arg)
{
	/* Trigger the Post bss transreq function */
	wl_wlif_hdl_post_bt_t *data = (wl_wlif_hdl_post_bt_t *)arg;
	wl_wlif_post_bss_transreq(data, &(data->addr));

	free(data);
	bcm_timer_delete(timer_id);
}

/* For USCHED Timer, Callback handler to trigger post bss transition request */
static void
wl_wlif_post_bss_transreq_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	/* Trigger the Post bss transreq function */
	wl_wlif_hdl_post_bt_t *data = (wl_wlif_hdl_post_bt_t *)arg;
	wl_wlif_post_bss_transreq(data, &(data->addr));
	free(data);
}

/* Create post bss transreq handler Timer */
static int
wl_wlif_create_post_bss_transreq_timer(wl_wlif_hdl_data_t  *hdl_data,
		wl_wlif_bss_trans_data_t *ioctl_hndlr_data)
{
	int ret = FALSE;
	wl_wlif_hdl_post_bt_t *tmr_data = NULL;
	int  timeout;
	sta_oui_wnm_act_t *ptr;
	unsigned char generic_oui[OUI_ADDR_LEN];	/* Generic OUI */

	if (!oui_action) {
		return ret;
	}
	oui_atoe(OUI_GENERAL, generic_oui);
	ptr = oui_action;
	while (ptr) {
		if ((!oui_cmp(ptr->oui, &ioctl_hndlr_data->addr)) ||
			(!oui_cmp(ptr->oui, generic_oui))) {
			timeout = ptr->post_bt_timeout;
			/* Allocate Timer Arg Struct Object */
			tmr_data = (wl_wlif_hdl_post_bt_t *) calloc(1, sizeof(*tmr_data));
			if (!tmr_data) {
				WLIF_BSSTRANS("Err: calloc Timer Arg Failed \n");
				return FALSE;
			}
			/* Initialize Timer Arg Struct Object */
			tmr_data->hdl_data = hdl_data;
			tmr_data->action = ptr->post_bt_action;
			memcpy(&tmr_data->addr, (char *)&(ioctl_hndlr_data->addr), ETHER_ADDR_LEN);

			/* Create an Appropriate Timer, For Defiant STA Watchdog */
			ret = wl_wlif_create_timer(hdl_data->uschd_hdl, hdl_data->timer_hdl,
				(hdl_data->uschd_hdl ? (void*)wl_wlif_post_bss_transreq_usched_cb :
				(void*)wl_wlif_post_bss_transreq_cb), (void*)tmr_data, timeout,
				FALSE, NULL);
			if (!ret) {
				free(tmr_data);
				return FALSE;
			}
		}
		ptr = ptr->next;
	}
	return TRUE;
}
