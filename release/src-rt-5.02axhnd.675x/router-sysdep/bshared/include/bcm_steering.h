/*
 * Broadcom steering include file
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: bcm_steering.h 767649 2018-09-20 09:56:25Z $
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

/* BSS transition return code */
#define WLIFU_BSS_TRANS_RESP_ACCEPT	0
#define WLIFU_BSS_TRANS_RESP_REJECT	1
#define WLIFU_BSS_TRANS_RESP_UNKNOWN	2

/* App specific handler function for bss-trans api to handle the response. */
typedef int (*bss_trans_resp_hndlr)(void *data, int response);

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
	bss_trans_resp_hndlr resp_hndlr;	/* bss-trans response handler. */
	void *resp_hndlr_data;			/* resp handler callback data. */
} wl_wlif_bss_trans_data_t;

/* Stores the response from the BSS transition response field */
typedef struct wl_wlif_bss_trans_resp {
	uint8 response_valid;			/* 1 If BSS Transition response recieved */
	uint8 status;				/* Transition status */
	struct ether_addr trgt_bssid;		/* BSSID to which STA transitions to.
						 * Valid only if status is 0
						 */
} wl_wlif_bss_trans_resp_t;

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
	callback_hndlr ioctl_hndlr, void *data);

/*
 * Deinit routine to free memory for wlif handle.
 * Params:
 * @hdl: Wlif handle.
 */
extern void wl_wlif_deinit(wl_wlif_hdl *hdl);

/*
 * BSS-Trans routine for sending act frame and receiving bss-trans response
 * if event_da is invalid lib will not get the bss-trans response.
 * Params:
 * @hdl: wlif lib handle.
 * @data: BSS-Trans action frame data.
 * @event_fd: Socket descriptor to receive bss-trans resp.
 * @bss transition response
 */
extern int wl_wlif_do_bss_trans(wl_wlif_hdl *hdl, wl_wlif_bss_trans_data_t *data,
	int event_fd, wl_wlif_bss_trans_resp_t *out_resp);

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

#endif  /* _BCM_STEERING_H_ */
