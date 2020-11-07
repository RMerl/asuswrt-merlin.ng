/*
 * Broadcom steering include file
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
 * $Id: bcm_steering.h 789955 2020-08-12 10:21:09Z $
 */

#ifndef _BCM_STEERING_H_
#define _BCM_STEERING_H_

#include "bcm_usched.h"
#include "ethernet.h"
#include <wlioctl.h>

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif // endif

/* BSS Transition flags */
#define WLIFU_BSS_TRANS_FLAGS_DISASSOC_IMNT	0x0001  /* BTM Disassociation Imminent bit */
#define WLIFU_BSS_TRANS_FLAGS_BTM_ABRIDGED	0x0002  /* BTM Abridged bit */
#define WLIFU_BSS_TRANS_FLAGS_LEGACY_ACTIVE_STA	0x0004  /* Legacy active sta */

/* BSS transition return code */
#define WLIFU_BSS_TRANS_RESP_ACCEPT	0
#define WLIFU_BSS_TRANS_RESP_REJECT	1
#define WLIFU_BSS_TRANS_RESP_UNKNOWN	2

#define WLIFU_NVRAM_STEER_FLAGS			"steer_flags" // NVRAM for steering flags
#define WLIFU_DEF_STEER_FLAGS			0x0	// Default NVRAM val for steer_flags

/* Steer Flags, Mask Values */			/* Bit	Purpose */
#define STEER_FLAGS_MASK_BTR_UNWN		0x0001	// 1 BT Resp UNKNOWN, Block Brute Force
#define STEER_FLAGS_MASK_BTR_RJCT		0x0002	// 2 BT Resp REJECT, Apply Brute Force
#define STEER_FLAGS_MASK_BTR_ACPT		0x0004	// 3 BT Resp ACCEPT, Enbl Defiant Watchdog
#define STEER_FLAGS_DISABLE_NOBCNSSID		0x0008	// 4 Disable nobcnssid def is enabled
#define STEER_FLAGS_5GTO5G_BRUTE_FORCE		0x0010	// 5 Use Brute Force for 5G to 5G steer
#define STEER_FLAGS_5GTO2G_BRUTE_FORCE		0x0020	// 6 Use Brute Force for 5G to 2G steer
#define STEER_FLAGS_2GTO5G_BRUTE_FORCE		0x0040	// 7 Use Brute Force for 2G to 5G steer
#define STEER_FLAGS_2GTO2G_BRUTE_FORCE		0x0080	// 8 Use Brute Force for 2G to 2G steer
#define STEER_FLAGS_LEGACY_ACT_STA_BRUTE_FORCE	0x0100	/* 9 Disable Brute Force for legacy sta
							 * which are active default is enabled
							 */
#define STEER_FLAGS_FAKE_CHANNEL_UTIL		0x0200	/* 10 Disable Fake channel utilization
							 * along with no bcnssid def is enabled
							*/

/* For Individual BSS Transition Response types, Get Brute Force Steer Behavior */
#define BTR_UNWN_BRUTFORCE_OFF(f)		((f) & STEER_FLAGS_MASK_BTR_UNWN)
#define BTR_RJCT_BRUTFORCE_ON(f)		((f) & STEER_FLAGS_MASK_BTR_RJCT)
#define BTR_5GTO5G_BRUTEFORCE_ON(f)		((f) & STEER_FLAGS_5GTO5G_BRUTE_FORCE)
#define BTR_5GTO2G_BRUTEFORCE_ON(f)		((f) & STEER_FLAGS_5GTO2G_BRUTE_FORCE)
#define BTR_2GTO5G_BRUTEFORCE_ON(f)		((f) & STEER_FLAGS_2GTO5G_BRUTE_FORCE)
#define BTR_2GTO2G_BRUTEFORCE_ON(f)		((f) & STEER_FLAGS_2GTO2G_BRUTE_FORCE)
/* For BSS Transition Response ACCEPT, Check if Dafiant STA Watchdog is required or not */
#define BTR_ACPT_DEFIANT_WD_ON(f)		((f) & STEER_FLAGS_MASK_BTR_ACPT)
/* Check whether nobcnssid is enabled or not */
#define IS_NOBCNSSID_ENABLED(f)			(!((f) & STEER_FLAGS_DISABLE_NOBCNSSID))
#define IS_LEGACY_ACT_STA_BRUTEFORCE_OFF(f)	((f) & STEER_FLAGS_LEGACY_ACT_STA_BRUTE_FORCE)
#define IS_FAKE_CHAN_UTIL_ENABLED(f)		(!((f) & STEER_FLAGS_FAKE_CHANNEL_UTIL))

/* Forward declaration */
typedef struct wl_wlif_bss_trans_resp wl_wlif_bss_trans_resp_t;

/* This is used to inform the BTM response to application. Application can pass this callback in
 * wl_wlif_bss_trans_data_t structure. Once application sends the "WLC_E_BSSTRANS_RESP" event data
 * library will parse it and without doing any post BTM response operation, Library will
 * call this callback. Library will also call in case of timeout if there is no
 * "WLC_E_BSSTRANS_RESP" event data.
 */
typedef int (*bss_trans_resp_hndlr)(void *data, int response);

/* This is used to inform the BTM response to application. Application can pass this callback in
 * wl_wlif_bss_trans_data_t structure. Once application sends the "WLC_E_BSSTRANS_RESP" event data
 * library will parse it and do some post BTM response operation. Then, Library will
 * call this callback. Library will also call in case of timeout if there is no
 * "WLC_E_BSSTRANS_RESP" event data.
 */
typedef int (*post_bss_trans_resp_cb)(char *ifname, void *data, int steer_resp,
	wl_wlif_bss_trans_resp_t *resp);

/* Struct for common bss-trans action frame data. */
typedef struct wl_wlif_bss_trans_data {
	uint8 rclass;				/* Rclass */
	chanspec_t chanspec;			/* Channel */
	uint16 flags;				/* Flags of type WLIFU_BSS_TRANS_FLAGS_XXX */
	uint16 disassoc_timer;			/* Disassociation Timer */
	struct ether_addr bssid;		/* Target bssid. */
	struct ether_addr addr;			/* Sta addr. */
	int timeout;				/* Timeout to clear mac from maclist. */
	uint32 bssid_info;			/* Target AP bssid_info */
	uint8 phytype;				/* Target AP phytype */
	uint8 reason;				/* Reason code for steering */
	bss_trans_resp_hndlr resp_hndlr;	/* This is used to inform the BTM response to
						 * application. Once application sends the
						 * "WLC_E_BSSTRANS_RESP" event data library will
						 * parse it and without doing any post BTM response
						 * operation, Library will call this callback.
						 * Library will also call in case of timeout if
						 * there is no "WLC_E_BSSTRANS_RESP" event data.
						 */
	void *resp_hndlr_data;			/* Data to be passed to the application in
						 * "bss_trans_resp_hndlr" callback
						 */
	post_bss_trans_resp_cb post_resp_cb;	/* This is used to inform the BTM response to
						 * application. Once application sends the
						 * "WLC_E_BSSTRANS_RESP" event data library will
						 * parse it and after doing some post BTM response
						 * operation, Library will call this callback.
						 * Library will also call in case of timeout if
						 * there is no "WLC_E_BSSTRANS_RESP" event data.
						 */
	void *post_resp_cb_data;		/* Data to be passed to the application in
						 * "post_bss_trans_resp_cb" callback
						 */
} wl_wlif_bss_trans_data_t;

/* Stores the response from the BSS transition response field */
struct wl_wlif_bss_trans_resp {
	uint8 response_valid;			/* 1 If BSS Transition response recieved */
	uint8 status;				/* Transition status */
	struct ether_addr trgt_bssid;		/* BSSID to which STA transitions to.
						 * Valid only if status is 0
						 */
};

/* wlif lib handle. */
typedef void wl_wlif_hdl;

/* Callback handler for ioctl calls. */
typedef int (*callback_hndlr)(char *ifname, int cmd, void *buf, int len, void *data);

/*
 * Init routine for wlif handle
 * Which allocates and initializes memory for wlif handle.
 * Params:
 * @uschd_hdl: Uschd handle.
 * @ifname: Interface name.
 * @ioctl_hndlr: Module specific ioctl handler routine.
 * @data: The data will be passed in ioctl_hndler callback function.
 */
extern wl_wlif_hdl* wl_wlif_init(void *uschd_hdl, char *ifname,
	callback_hndlr ioctl_hndlr, void *data, char *appname);

/*
 * Deinit routine to free memory for wlif handle.
 * Params:
 * @hdl: Wlif handle.
 */
extern void wl_wlif_deinit(wl_wlif_hdl *hdl);

/*
 * BSS-Trans routine for sending BTM request to the STA. This routine will return immediately
 * after sending the request. Applications should pass "post_bss_trans_resp_cb" or
 * "bss_trans_resp_hndlr" callback in "wl_wlif_bss_trans_data_t" structure to get the response.
 * Applications should listen for "WLC_E_BSSTRANS_RESP" event and pass the event data to library to
 * get the BTM response. Else, the library will call those above registered callbacks to inform
 * that there is no response(steer_resp type is WLIFU_BSS_TRANS_RESP_UNKNOWN).
 * Params:
 * @hdl: wlif lib handle.
 * @data: BSS-Trans action frame data.
 *
 * @return	0 Success. Non zero failure. There will not be any callback.
 */
extern int wl_wlif_do_bss_trans(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *data);

/*
 * Routine for setting macmode and maclist for valid timeout
 * a timer will be registered to unblock the sta.
 * Params:
 * @hdl: Wlif lib handle.
 * @addr: Sta mac address.
 * @timeout: Timeout interval.
 */
extern int wl_wlif_block_mac(wl_wlif_hdl *hdl, struct ether_addr addr,
	int timeout);

/*
 * Routine for removing sta entry from maclist.
 * Params:
 * @hdl: wlif lib handle.
 * @addr: Sta mac address.
 * @flag: Bitflag for sta count and macmode.
 */
extern int wl_wlif_unblock_mac(wl_wlif_hdl *hdl, struct ether_addr addr, int flag);

/*
 * Routine for processing the "WLC_E_BSSTRANS_RESP" event. Application listening to this event
 * should pass the event data after "wl_wlif_do_bss_trans" for getting the response data from
 * event data. If application doesn't pass the event data, library will call the
 * "bss_trans_resp_hndlr" or "post_bss_trans_resp_cb" to inform that there is no
 * response(steer_resp type is WLIFU_BSS_TRANS_RESP_UNKNOWN).
 * Params:
 * @hdl: wlif lib handle.
 * @pkt: Event data recieved from firmware.
 * @len: Length of data.
 */
extern void wl_wlif_process_bss_trans_resp_event(wl_wlif_hdl *hdl, char *pkt, int len);

#endif  /* _BCM_STEERING_H_ */
