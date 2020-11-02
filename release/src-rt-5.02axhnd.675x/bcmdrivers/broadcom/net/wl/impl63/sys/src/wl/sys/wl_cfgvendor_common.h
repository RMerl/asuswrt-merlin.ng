/*
 * Linux cfg80211 Vendor Extension Code
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
 * $Id: wl_cfgvendor.h 764377 2018-05-25 03:02:33Z $
 */

/*
 * xxx New vendor interface additon to nl80211/cfg80211 to allow vendors
 * to implement proprietary features over the cfg80211 stack.
 */

#ifndef _wl_cfgvendor_common_h_
#define _wl_cfgvendor_common_h_
#if defined(BCMDONGLEHOST)
#include <dhd.h>
#endif // endif
#define OUI_BRCM    0x001018
#define OUI_GOOGLE  0x001A11
#define BRCM_VENDOR_SUBCMD_PRIV_STR	1
#define ATTRIBUTE_U32_LEN                  (NLA_HDRLEN  + 4)
#define VENDOR_ID_OVERHEAD                 ATTRIBUTE_U32_LEN
#define VENDOR_SUBCMD_OVERHEAD             ATTRIBUTE_U32_LEN
#define VENDOR_DATA_OVERHEAD               (NLA_HDRLEN)

enum brcm_vendor_attr {
	BRCM_ATTR_DRIVER_CMD,
	BRCM_ATTR_DRIVER_MAX
};

#define VENDOR_REPLY_OVERHEAD       (VENDOR_ID_OVERHEAD + \
									VENDOR_SUBCMD_OVERHEAD + \
									VENDOR_DATA_OVERHEAD)
typedef enum {
	/* don't use 0 as a valid subcommand */
	VENDOR_NL80211_SUBCMD_UNSPECIFIED,

	/* define all vendor startup commands between 0x0 and 0x0FFF */
	VENDOR_NL80211_SUBCMD_RANGE_START = 0x0001,
	VENDOR_NL80211_SUBCMD_RANGE_END   = 0x0FFF,

	/* define all GScan related commands between 0x1000 and 0x10FF */
	ANDROID_NL80211_SUBCMD_GSCAN_RANGE_START = 0x1000,
	ANDROID_NL80211_SUBCMD_GSCAN_RANGE_END   = 0x10FF,

	/* define all RTT related commands between 0x1100 and 0x11FF */
	ANDROID_NL80211_SUBCMD_RTT_RANGE_START = 0x1100,
	ANDROID_NL80211_SUBCMD_RTT_RANGE_END   = 0x11FF,

	ANDROID_NL80211_SUBCMD_LSTATS_RANGE_START = 0x1200,
	ANDROID_NL80211_SUBCMD_LSTATS_RANGE_END   = 0x12FF,

	ANDROID_NL80211_SUBCMD_TDLS_RANGE_START = 0x1300,
	ANDROID_NL80211_SUBCMD_TDLS_RANGE_END	= 0x13FF,

	ANDROID_NL80211_SUBCMD_DEBUG_RANGE_START = 0x1400,
	ANDROID_NL80211_SUBCMD_DEBUG_RANGE_END	= 0x14FF,

	/* define all NearbyDiscovery related commands between 0x1500 and 0x15FF */
	ANDROID_NL80211_SUBCMD_NBD_RANGE_START = 0x1500,
	ANDROID_NL80211_SUBCMD_NBD_RANGE_END   = 0x15FF,

	/* define all wifi calling related commands between 0x1600 and 0x16FF */
	ANDROID_NL80211_SUBCMD_WIFI_OFFLOAD_RANGE_START = 0x1600,
	ANDROID_NL80211_SUBCMD_WIFI_OFFLOAD_RANGE_END	= 0x16FF,

	/* define all NAN related commands between 0x1700 and 0x17FF */
	ANDROID_NL80211_SUBCMD_NAN_RANGE_START = 0x1700,
	ANDROID_NL80211_SUBCMD_NAN_RANGE_END   = 0x17FF,

	/* define all packet filter related commands between 0x1800 and 0x18FF */
	ANDROID_NL80211_SUBCMD_PKT_FILTER_RANGE_START = 0x1800,
	ANDROID_NL80211_SUBCMD_PKT_FILTER_RANGE_END   = 0x18FF,
	/* This is reserved for future usage */

} ANDROID_VENDOR_SUB_COMMAND;

enum debug_attributes {
	DEBUG_ATTRIBUTE_GET_DRIVER,
	DEBUG_ATTRIBUTE_GET_FW,
	DEBUG_ATTRIBUTE_RING_ID,
	DEBUG_ATTRIBUTE_RING_NAME,
	DEBUG_ATTRIBUTE_RING_FLAGS,
	DEBUG_ATTRIBUTE_LOG_LEVEL,
	DEBUG_ATTRIBUTE_LOG_TIME_INTVAL,
	DEBUG_ATTRIBUTE_LOG_MIN_DATA_SIZE,
	DEBUG_ATTRIBUTE_FW_DUMP_LEN,
	DEBUG_ATTRIBUTE_FW_DUMP_DATA,
	DEBUG_ATTRIBUTE_RING_DATA,
	DEBUG_ATTRIBUTE_RING_STATUS,
	DEBUG_ATTRIBUTE_RING_NUM,
	DEBUG_ATTRIBUTE_DRIVER_DUMP_LEN,
	DEBUG_ATTRIBUTE_DRIVER_DUMP_DATA,
	DEBUG_ATTRIBUTE_PKT_FATE_NUM,
	DEBUG_ATTRIBUTE_PKT_FATE_DATA
};

typedef enum wl_vendor_event {
	BRCM_VENDOR_EVENT_UNSPEC,
	BRCM_VENDOR_EVENT_PRIV_STR,
#ifdef WL_VENDOR_EXT_GOOGLE_SUPPORT
#ifdef GSCAN_SUPPORT
	GOOGLE_GSCAN_SIGNIFICANT_EVENT,
	GOOGLE_GSCAN_GEOFENCE_FOUND_EVENT,
	GOOGLE_GSCAN_BATCH_SCAN_EVENT,
	GOOGLE_SCAN_FULL_RESULTS_EVENT,
#endif /* GSCAN_SUPPORT */
#ifdef RTT_SUPPORT
	GOOGLE_RTT_COMPLETE_EVENT,
#endif /* RTT_SUPPORT */
#ifdef GSCAN_SUPPORT
	GOOGLE_SCAN_COMPLETE_EVENT,
	GOOGLE_GSCAN_GEOFENCE_LOST_EVENT,
	GOOGLE_SCAN_EPNO_EVENT,
#endif /* GSCAN_SUPPORT */
	GOOGLE_DEBUG_RING_EVENT,
	GOOGLE_FW_DUMP_EVENT,
#ifdef GSCAN_SUPPORT
	GOOGLE_PNO_HOTSPOT_FOUND_EVENT,
#endif /* GSCAN_SUPPORT */
	GOOGLE_RSSI_MONITOR_EVENT,
	GOOGLE_MKEEP_ALIVE_EVENT,
	/*
	 * BRCM specific events should be placed after
	 * the Generic events so that enums don't mismatch
	 * between the DHD and HAL
	 */
#endif /* WL_VENDOR_EXT_GOOGLE_SUPPORT */
} wl_vendor_event_t;

#ifdef WL_VENDOR_EXT_SUPPORT
int wl_cfgvendor_attach(struct wiphy *wiphy);
int wl_cfgvendor_detach(struct wiphy *wiphy);
int wl_cfgvendor_send_async_event(struct wiphy *wiphy,
		struct net_device *dev, int event_id, const void  *data, int len);
struct net_device *
wl_cfgvendor_get_ndev(struct bcm_cfg80211 *cfg, struct wireless_dev *wdev,
	const void *data, unsigned long int *out_addr);
#else
static INLINE int wl_cfgvendor_attach(struct wiphy *wiphy,
		dhd_pub_t *dhd) { UNUSED_PARAMETER(wiphy); UNUSED_PARAMETER(dhd); return 0; }
static INLINE int wl_cfgvendor_detach(struct wiphy *wiphy) { UNUSED_PARAMETER(wiphy); return 0; }
static INLINE struct net_device *
wl_cfgvendor_get_ndev(struct bcm_cfg80211 *cfg, struct wireless_dev *wdev,
	const void *data, unsigned long int *out_addr) { UNUSED_PARAMETER(cfg);
	UNUSED_PARAMETER(wdev); UNUSED_PARAMETER(data); UNUSED_PARAMETER(out_addr); return NULL;}
#endif /* WL_VENDOR_EXT_SUPPORT */

#ifdef VENDOR_EXT_SUPPORT
int cfgvendor_attach(struct wiphy *wiphy);
int cfgvendor_detach(struct wiphy *wiphy);
#endif /*  VENDOR_EXT_SUPPORT */

#ifdef CONFIG_COMPAT
#define COMPAT_ASSIGN_VALUE(normal_structure, member, value)	\
	do { \
		if (compat_task_state) {	\
			compat_ ## normal_structure.member = value; \
		} else { \
			normal_structure.member = value; \
		} \
	} while (0)
#else
#define COMPAT_ASSIGN_VALUE(normal_structure, member, value) \
	normal_structure.member = value;
#endif /* CONFIG_COMPAT */

#endif /* _wl_cfgvendor_common_h_ */
