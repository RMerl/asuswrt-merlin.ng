/*
 * Linux-specific portion of Broadcom 802.11abg Networking Device Driver
 * cfg80211 interface
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: wl_cfg80211_hybrid.h 467328 2014-04-03 01:23:40Z $
 */

#ifndef _wl_cfg80211_h_
#define _wl_cfg80211_h_

#include <net/cfg80211.h>
#include <wlioctl.h>
struct wl_cfg80211_conf;
struct wl_cfg80211_priv;
struct wl_cfg80211_security;

#define htod32(i) (i)
#define htod16(i) (i)
#define dtoh32(i) (i)
#define dtoh16(i) (i)
#define htodchanspec(i) (i)
#define dtohchanspec(i) (i)
#define dtoh32(i) (i)
#define dtoh16(i) (i)

#define WL_DBGMSG_ENABLE

#define WL_DBG_NONE	0
#define WL_DBG_DBG 	(1 << 2)
#define WL_DBG_INFO	(1 << 1)
#define WL_DBG_ERR	(1 << 0)
#define WL_DBG_MASK ((WL_DBG_DBG | WL_DBG_INFO | WL_DBG_ERR) << 1)

#if defined(WL_DBGMSG_ENABLE)
#define	WL_DBG(args)								\
do {									\
	if (wl_dbg_level & WL_DBG_DBG) {			\
		printk(KERN_ERR "DEBUG @%s :", __func__);	\
		printk args;							\
	}									\
} while (0)
#else				/* WL_DBGMSG_ENABLE */
#define	WL_DBG(args)
#endif				/* WL_DBGMSG_ENABLE */

#define	WL_ERR(args)									\
do {										\
	if (wl_dbg_level & WL_DBG_ERR) {				\
		if (net_ratelimit()) {						\
			printk(KERN_ERR "ERROR @%s : ", __func__);	\
			printk args;						\
		} 								\
	}									\
} while (0)

#define	WL_INF(args)									\
do {										\
	if (wl_dbg_level & WL_DBG_INFO) {				\
		if (net_ratelimit()) {						\
			printk(KERN_ERR "INFO @%s : ", __func__);	\
			printk args;						\
		}								\
	}									\
} while (0)


#define WL_NUM_SCAN_MAX		1
#define WL_NUM_PMKIDS_MAX	MAXPMKID	/* will be used for 2.6.33 kernel or later */
#define WL_SCAN_BUF_BASE 		(16*1024)
#define WL_TLV_INFO_MAX 		1024
#define WL_BSS_INFO_MAX			2048
#define WL_ASSOC_INFO_MAX	512
#define WL_IOCTL_LEN_MAX	2048
#define WL_EXTRA_BUF_MAX	2048
#define WL_AP_MAX	256	/* virtually unlimitted as long as kernel memory allows */

/* status */
enum wl_cfg80211_status {
	WL_STATUS_CONNECTING,
	WL_STATUS_CONNECTED
};

/* wi-fi mode */
enum wl_cfg80211_mode {
	WL_MODE_BSS,
	WL_MODE_IBSS,
	WL_MODE_AP
};

/* beacon / probe_response */
struct beacon_proberesp {
	__le64 timestamp;
	__le16 beacon_int;
	__le16 capab_info;
	u8 variable[0];
} __attribute__ ((packed));

/* configuration */
struct wl_cfg80211_conf {
	u32 mode;		/* adhoc , infrastructure or ap */
	u32 frag_threshold;
	u32 rts_threshold;
	u32 retry_short;
	u32 retry_long;
	s32 tx_power;
	struct ieee80211_channel channel;
};

/* cfg80211 main event loop */
struct wl_cfg80211_event_loop {
	s32(*handler[WLC_E_LAST]) (struct wl_cfg80211_priv *wl, struct net_device *ndev,
	                           const wl_event_msg_t *e, void *data);
};

/* bss inform structure for cfg80211 interface */
struct wl_cfg80211_bss_info {
	u16 band;
	u16 channel;
	s16 rssi;
	u16 frame_len;
	u8 frame_buf[1];
};

/* basic structure of scan request */
struct wl_cfg80211_scan_req {
	struct wlc_ssid ssid;
};

/* basic structure of information element */
struct wl_cfg80211_ie {
	u16 offset;
	u8 buf[WL_TLV_INFO_MAX];
};

/* event queue for cfg80211 main event */
struct wl_cfg80211_event_q {
	struct list_head eq_list;
	u32 etype;
	wl_event_msg_t emsg;
	s8 edata[1];
};

/* security information with currently associated ap */
struct wl_cfg80211_security {
	u32 wpa_versions;
	u32 auth_type;
	u32 cipher_pairwise;
	u32 cipher_group;
	u32 wpa_auth;
};

/* profile */
struct wl_cfg80211_profile {
	struct wlc_ssid ssid;
	u8 bssid[ETHER_ADDR_LEN];
	struct wl_cfg80211_security sec;
	bool active;
};

/* association inform */
struct wl_cfg80211_connect_info {
	u8 *req_ie;
	s32 req_ie_len;
	u8 *resp_ie;
	s32 resp_ie_len;
};

/* assoc ie length */
struct wl_cfg80211_assoc_ielen {
	u32 req_len;
	u32 resp_len;
};

/* wpa2 pmk list */
struct wl_cfg80211_pmk_list {
	pmkid_list_t pmkids;
	pmkid_t foo[MAXPMKID - 1];
};

/* private data of cfg80211 interface */
struct wl_cfg80211_priv {
	struct wireless_dev *wdev;	/* representing wl cfg80211 device */
	struct wl_cfg80211_conf *conf;	/* configuration */
	struct cfg80211_scan_request *scan_request;	/* scan request object */
	struct wl_cfg80211_event_loop el;	/* main event loop */
	struct list_head eq_list;	/* used for event queue */
	spinlock_t eq_lock;	/* for event queue synchronization */
	struct wl_cfg80211_scan_req *scan_req_int;  /* scan request object for internal purpose */
	struct wl_cfg80211_ie ie;	 /* information element object for internal purpose */
	struct ether_addr bssid;	/* bssid of currently engaged network */
	struct semaphore event_sync;	/* for synchronization of main event thread */
	struct wl_cfg80211_profile *profile;	/* holding profile */
	struct wl_cfg80211_connect_info conn_info;	/* association information container */
	struct wl_cfg80211_pmk_list *pmk_list;	/* wpa2 pmk list */
	struct task_struct *event_tsk;	/* task of main event handler thread */
	unsigned long status;		/* current status */
	bool active_scan;	/* current scan mode */
	u8 *ioctl_buf;	/* ioctl buffer */
	u8 *extra_buf;	/* maily to grab assoc information */
	u8 ci[0] __attribute__ ((__aligned__(NETDEV_ALIGN)));
};

#define wl_to_dev(w) (wiphy_dev(wl->wdev->wiphy))
#define wl_to_wiphy(w) (w->wdev->wiphy)
#define wiphy_to_wl(w) ((struct wl_cfg80211_priv *)(wiphy_priv(w)))
#define wl_to_wdev(w) (w->wdev)
#define wdev_to_wl(w) ((struct wl_cfg80211_priv *)(wdev_priv(w)))
#define wl_to_ndev(w) (w->wdev->netdev)
#define ndev_to_wl(n) (wdev_to_wl(n->ieee80211_ptr))
#define wl_to_sr(w) (w->scan_req_int)
#define wl_to_ie(w) (&w->ie)
#define wl_to_conn(w) (&w->conn_info)

static inline struct wl_bss_info *next_bss(struct wl_scan_results *list, struct wl_bss_info *bss)
{
	return bss = bss ? (struct wl_bss_info *)((unsigned long)bss +
	             dtoh32(bss->length)) : list->bss_info;
}

#define for_each_bss(list, bss, __i)	\
	for (__i = 0; __i < list->count && __i < WL_AP_MAX; __i++, bss = next_bss(list, bss))

extern s32 wl_cfg80211_attach(struct net_device *ndev, struct device *dev);
extern void wl_cfg80211_detach(struct net_device *ndev);

/* event handler */
extern void wl_cfg80211_event(struct net_device *ndev, const wl_event_msg_t *e, void *data);
extern s32 wl_cfg80211_up(struct net_device *ndev);
extern s32 wl_cfg80211_down(struct net_device *ndev);

#endif /* _wl_cfg80211_h_ */
