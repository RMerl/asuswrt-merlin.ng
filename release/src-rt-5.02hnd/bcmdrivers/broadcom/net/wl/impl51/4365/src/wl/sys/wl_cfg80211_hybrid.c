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
 * $Id: wl_cfg80211_hybrid.c 483367 2014-06-09 09:10:02Z $
 */

/* Based on:
	Intel Wireless Multicomm 3200 WiFi driver
	drivers/net/wireless/iwmc3200wi
*/

#if defined(USE_CFG80211)

#define LINUX_PORT
#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include <linux/nl80211.h>
#include <net/rtnetlink.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <proto/802.11.h>
#include <wl_cfg80211_hybrid.h>

#define EVENT_TYPE(e) dtoh32((e)->event_type)
#define EVENT_FLAGS(e) dtoh16((e)->flags)
#define EVENT_STATUS(e) dtoh32((e)->status)

u32 wl_dbg_level = WL_DBG_ERR | WL_DBG_INFO;

/*
** cfg80211_ops api/callback list
*/
static s32 wl_cfg80211_change_iface(struct wiphy *wiphy, struct net_device *ndev,
           enum nl80211_iftype type, u32 *flags, struct vif_params *params);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
static s32
wl_cfg80211_scan(struct wiphy *wiphy,
                 struct cfg80211_scan_request *request);
#else
static s32 wl_cfg80211_scan(struct wiphy *wiphy, struct net_device *ndev,
           struct cfg80211_scan_request *request);
#endif
static s32 wl_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed);
static s32 wl_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
           struct cfg80211_ibss_params *params);
static s32 wl_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev);
static s32 wl_cfg80211_get_station(struct wiphy *wiphy,
           struct net_device *dev, u8 *mac, struct station_info *sinfo);
static s32 wl_cfg80211_set_power_mgmt(struct wiphy *wiphy,
           struct net_device *dev, bool enabled, s32 timeout);
static int wl_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
           struct cfg80211_connect_params *sme);
static s32 wl_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev, u16 reason_code);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev,
                         enum nl80211_tx_power_setting type, s32 dbm);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static s32 wl_cfg80211_set_tx_power(struct wiphy *wiphy,
           enum nl80211_tx_power_setting type, s32 dbm);
#else
static s32 wl_cfg80211_set_tx_power(struct wiphy *wiphy,
           enum tx_power_setting type, s32 dbm);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev, s32 *dbm);
#else
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy, s32 *dbm);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)
static s32 wl_cfg80211_config_default_key(struct wiphy *wiphy,
           struct net_device *dev, u8 key_idx, bool unicast, bool multicast);
#else
static s32 wl_cfg80211_config_default_key(struct wiphy *wiphy,
           struct net_device *dev, u8 key_idx);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static s32 wl_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
           u8 key_idx, bool pairwise, const u8 *mac_addr, struct key_params *params);
static s32 wl_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
           u8 key_idx, bool pairwise, const u8 *mac_addr);
static s32 wl_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
           u8 key_idx, bool pairwise, const u8 *mac_addr,
           void *cookie, void (*callback) (void *cookie, struct key_params *params));
#else
static s32 wl_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
           u8 key_idx, const u8 *mac_addr, struct key_params *params);
static s32 wl_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
           u8 key_idx, const u8 *mac_addr);
static s32 wl_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
           u8 key_idx, const u8 *mac_addr,
           void *cookie, void (*callback) (void *cookie, struct key_params *params));
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static s32 wl_cfg80211_set_pmksa(struct wiphy *wiphy, struct net_device *dev,
           struct cfg80211_pmksa *pmksa);
static s32 wl_cfg80211_del_pmksa(struct wiphy *wiphy, struct net_device *dev,
           struct cfg80211_pmksa *pmksa);
static s32 wl_cfg80211_flush_pmksa(struct wiphy *wiphy, struct net_device *dev);
#endif

/*
** event & event Q handlers for cfg80211 interfaces
*/
static s32 wl_create_event_handler(struct wl_cfg80211_priv *wl);
static void wl_destroy_event_handler(struct wl_cfg80211_priv *wl);
static s32 wl_event_handler(void *data);
static void wl_init_eq(struct wl_cfg80211_priv *wl);
static void wl_flush_eq(struct wl_cfg80211_priv *wl);
static void wl_lock_eq(struct wl_cfg80211_priv *wl);
static void wl_unlock_eq(struct wl_cfg80211_priv *wl);
static void wl_init_eq_lock(struct wl_cfg80211_priv *wl);
static void wl_init_eloop_handler(struct wl_cfg80211_event_loop *el);
static struct wl_cfg80211_event_q *wl_deq_event(struct wl_cfg80211_priv *wl);
static s32 wl_enq_event(struct wl_cfg80211_priv *wl, u32 type,
	const wl_event_msg_t *msg, void *data);
static void wl_put_event(struct wl_cfg80211_event_q *e);
static void wl_wakeup_event(struct wl_cfg80211_priv *wl);

static s32 wl_notify_connect_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
           const wl_event_msg_t *e, void *data);
static s32 wl_notify_roaming_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
           const wl_event_msg_t *e, void *data);
static s32 wl_notify_scan_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
           const wl_event_msg_t *e, void *data);
static s32 wl_bss_connect_done(struct wl_cfg80211_priv *wl, struct net_device *ndev,
           const wl_event_msg_t *e, void *data, bool completed);
static s32 wl_bss_roaming_done(struct wl_cfg80211_priv *wl, struct net_device *ndev,
           const wl_event_msg_t *e, void *data);
static s32 wl_notify_mic_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
           const wl_event_msg_t *e, void *data);

/*
** ioctl utilites
*/
static s32 wl_dev_bufvar_get(struct net_device *dev, s8 *name, s8 *buf, s32 buf_len);
static __used s32 wl_dev_bufvar_set(struct net_device *dev, s8 *name, s8 *buf, s32 len);
static s32 wl_dev_intvar_set(struct net_device *dev, s8 *name, s32 val);
static s32 wl_dev_intvar_get(struct net_device *dev, s8 *name, s32 *retval);
static s32 wl_dev_ioctl(struct net_device *dev, u32 cmd, void *arg, u32 len);

/*
** cfg80211 set_wiphy_params utilities
*/
static s32 wl_set_frag(struct net_device *dev, u32 frag_threshold);
static s32 wl_set_rts(struct net_device *dev, u32 frag_threshold);
static s32 wl_set_retry(struct net_device *dev, u32 retry, bool l);

/*
** wl profile utilities
*/
static void wl_init_prof(struct wl_cfg80211_profile *prof);

/*
** cfg80211 connect utilites
*/
static s32 wl_set_wpa_version(struct net_device *dev, struct cfg80211_connect_params *sme);
static s32 wl_set_auth_type(struct net_device *dev, struct cfg80211_connect_params *sme);
static s32 wl_set_set_cipher(struct net_device *dev, struct cfg80211_connect_params *sme);
static s32 wl_set_key_mgmt(struct net_device *dev, struct cfg80211_connect_params *sme);
static s32 wl_set_set_sharedkey(struct net_device *dev, struct cfg80211_connect_params *sme);
static s32 wl_get_assoc_ies(struct wl_cfg80211_priv *wl);
static void wl_ch_to_chanspec(struct ieee80211_channel *chan,
            struct wl_join_params *join_params, size_t *join_params_size);

/*
** information element utilities
*/
static void wl_rst_ie(struct wl_cfg80211_priv *wl);
static __used s32 wl_add_ie(struct wl_cfg80211_priv *wl, u8 t, u8 l, u8 *v);
static s32 wl_mrg_ie(struct wl_cfg80211_priv *wl, u8 *ie_stream, u16 ie_size);
static s32 wl_cp_ie(struct wl_cfg80211_priv *wl, u8 *dst, u16 dst_size);
static u32 wl_get_ielen(struct wl_cfg80211_priv *wl);

static s32 wl_mode_to_nl80211_iftype(s32 mode);

static s32 wl_alloc_wdev(struct device *dev, struct wireless_dev **rwdev);
static void wl_free_wdev(struct wl_cfg80211_priv *wl);

static s32 wl_inform_bss(struct wl_cfg80211_priv *wl, struct wl_scan_results *bss_list);
static s32 wl_inform_single_bss(struct wl_cfg80211_priv *wl, struct wl_bss_info *bi);
static s32 wl_update_bss_info(struct wl_cfg80211_priv *wl);

/*
** key endianess swap utilities
*/
static void key_endian_to_device(struct wl_wsec_key *key);
static void key_endian_to_host(struct wl_wsec_key *key);

/*
** wl_cfg80211_priv memory init/deinit utilities
*/
static s32 wl_init_priv_mem(struct wl_cfg80211_priv *wl);
static void wl_deinit_priv_mem(struct wl_cfg80211_priv *wl);

/*
** ibss mode utilities
*/
static bool wl_is_ibssmode(struct wl_cfg80211_priv *wl);

/*
** link up/down , default configuration utilities
*/
static void wl_link_up(struct wl_cfg80211_priv *wl);
static void wl_link_down(struct wl_cfg80211_priv *wl);
static s32 wl_set_mode(struct net_device *ndev, s32 iftype);

static void wl_init_conf(struct wl_cfg80211_conf *conf);

static s32 wl_update_wiphybands(struct wl_cfg80211_priv *wl);

/*
* update pmklist to dongle
*/
static __used s32 wl_update_pmklist(struct net_device *dev,
                  struct wl_cfg80211_pmk_list *pmk_list, s32 err);

#if defined(WL_DBGMSG_ENABLE)
#define WL_DBG_ESTR_MAX	32
static s8 wl_dbg_estr[][WL_DBG_ESTR_MAX] = {
	"SET_SSID", "JOIN", "START", "AUTH", "AUTH_IND",
	"DEAUTH", "DEAUTH_IND", "ASSOC", "ASSOC_IND", "REASSOC",
	"REASSOC_IND", "DISASSOC", "DISASSOC_IND", "QUIET_START", "QUIET_END",
	"BEACON_RX", "LINK", "MIC_ERROR", "NDIS_LINK", "ROAM",
	"TXFAIL", "PMKID_CACHE", "RETROGRADE_TSF", "PRUNE", "AUTOAUTH",
	"EAPOL_MSG", "SCAN_COMPLETE", "ADDTS_IND", "DELTS_IND", "BCNSENT_IND",
	"BCNRX_MSG", "BCNLOST_MSG", "ROAM_PREP", "PFN_NET_FOUND",
	"PFN_NET_LOST",
	"RESET_COMPLETE", "JOIN_START", "ROAM_START", "ASSOC_START",
	"IBSS_ASSOC",
	"RADIO", "PSM_WATCHDOG",
	"PROBREQ_MSG",
	"SCAN_CONFIRM_IND", "PSK_SUP", "COUNTRY_CODE_CHANGED",
	"EXCEEDED_MEDIUM_TIME", "ICV_ERROR",
	"UNICAST_DECODE_ERROR", "MULTICAST_DECODE_ERROR", "TRACE",
	"IF",
	"RSSI", "PFN_SCAN_COMPLETE", "ACTION_FRAME", "ACTION_FRAME_COMPLETE",
};
#endif				/* WL_DBG_LEVEL */

#define CHAN2G(_channel, _freq, _flags) {			\
	.band			= IEEE80211_BAND_2GHZ,		\
	.center_freq		= (_freq),			\
	.hw_value		= (_channel),			\
	.flags			= (_flags),			\
	.max_antenna_gain	= 0,				\
	.max_power		= 30,				\
}

#define CHAN5G(_channel, _flags) {				\
	.band			= IEEE80211_BAND_5GHZ,		\
	.center_freq		= 5000 + (5 * (_channel)),	\
	.hw_value		= (_channel),			\
	.flags			= (_flags),			\
	.max_antenna_gain	= 0,				\
	.max_power		= 30,				\
}

#define RATE_TO_BASE100KBPS(rate)   (((rate) * 10) / 2)
#define RATETAB_ENT(_rateid, _flags) \
	{                                                               \
		.bitrate        = RATE_TO_BASE100KBPS(_rateid),     \
		.hw_value       = (_rateid),                            \
		.flags          = (_flags),                             \
	}

static struct ieee80211_rate __wl_rates[] = {
	RATETAB_ENT(DOT11_RATE_1M, 0),
	RATETAB_ENT(DOT11_RATE_2M, IEEE80211_RATE_SHORT_PREAMBLE),
	RATETAB_ENT(DOT11_RATE_5M5, IEEE80211_RATE_SHORT_PREAMBLE),
	RATETAB_ENT(DOT11_RATE_11M, IEEE80211_RATE_SHORT_PREAMBLE),
	RATETAB_ENT(DOT11_RATE_6M, 0),
	RATETAB_ENT(DOT11_RATE_9M, 0),
	RATETAB_ENT(DOT11_RATE_12M, 0),
	RATETAB_ENT(DOT11_RATE_18M, 0),
	RATETAB_ENT(DOT11_RATE_24M, 0),
	RATETAB_ENT(DOT11_RATE_36M, 0),
	RATETAB_ENT(DOT11_RATE_48M, 0),
	RATETAB_ENT(DOT11_RATE_54M, 0),
};

#define wl_a_rates		(__wl_rates + 4)
#define wl_a_rates_size	8
#define wl_g_rates		(__wl_rates + 0)
#define wl_g_rates_size	12

static struct ieee80211_channel __wl_2ghz_channels[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

static struct ieee80211_channel __wl_5ghz_a_channels[] = {
	CHAN5G(34, 0), CHAN5G(36, 0),
	CHAN5G(38, 0), CHAN5G(40, 0),
	CHAN5G(42, 0), CHAN5G(44, 0),
	CHAN5G(46, 0), CHAN5G(48, 0),
	CHAN5G(52, 0), CHAN5G(56, 0),
	CHAN5G(60, 0), CHAN5G(64, 0),
	CHAN5G(100, 0), CHAN5G(104, 0),
	CHAN5G(108, 0), CHAN5G(112, 0),
	CHAN5G(116, 0), CHAN5G(120, 0),
	CHAN5G(124, 0), CHAN5G(128, 0),
	CHAN5G(132, 0), CHAN5G(136, 0),
	CHAN5G(140, 0), CHAN5G(149, 0),
	CHAN5G(153, 0), CHAN5G(157, 0),
	CHAN5G(161, 0), CHAN5G(165, 0),
	CHAN5G(184, 0), CHAN5G(188, 0),
	CHAN5G(192, 0), CHAN5G(196, 0),
	CHAN5G(200, 0), CHAN5G(204, 0),
	CHAN5G(208, 0), CHAN5G(212, 0),
	CHAN5G(216, 0),
};

static struct ieee80211_channel __wl_5ghz_n_channels[] = {
	CHAN5G(32, 0), CHAN5G(34, 0),
	CHAN5G(36, 0), CHAN5G(38, 0),
	CHAN5G(40, 0), CHAN5G(42, 0),
	CHAN5G(44, 0), CHAN5G(46, 0),
	CHAN5G(48, 0), CHAN5G(50, 0),
	CHAN5G(52, 0), CHAN5G(54, 0),
	CHAN5G(56, 0), CHAN5G(58, 0),
	CHAN5G(60, 0), CHAN5G(62, 0),
	CHAN5G(64, 0), CHAN5G(66, 0),
	CHAN5G(68, 0), CHAN5G(70, 0),
	CHAN5G(72, 0), CHAN5G(74, 0),
	CHAN5G(76, 0), CHAN5G(78, 0),
	CHAN5G(80, 0), CHAN5G(82, 0),
	CHAN5G(84, 0), CHAN5G(86, 0),
	CHAN5G(88, 0), CHAN5G(90, 0),
	CHAN5G(92, 0), CHAN5G(94, 0),
	CHAN5G(96, 0), CHAN5G(98, 0),
	CHAN5G(100, 0), CHAN5G(102, 0),
	CHAN5G(104, 0), CHAN5G(106, 0),
	CHAN5G(108, 0), CHAN5G(110, 0),
	CHAN5G(112, 0), CHAN5G(114, 0),
	CHAN5G(116, 0), CHAN5G(118, 0),
	CHAN5G(120, 0), CHAN5G(122, 0),
	CHAN5G(124, 0), CHAN5G(126, 0),
	CHAN5G(128, 0), CHAN5G(130, 0),
	CHAN5G(132, 0), CHAN5G(134, 0),
	CHAN5G(136, 0), CHAN5G(138, 0),
	CHAN5G(140, 0), CHAN5G(142, 0),
	CHAN5G(144, 0), CHAN5G(145, 0),
	CHAN5G(146, 0), CHAN5G(147, 0),
	CHAN5G(148, 0), CHAN5G(149, 0),
	CHAN5G(150, 0), CHAN5G(151, 0),
	CHAN5G(152, 0), CHAN5G(153, 0),
	CHAN5G(154, 0), CHAN5G(155, 0),
	CHAN5G(156, 0), CHAN5G(157, 0),
	CHAN5G(158, 0), CHAN5G(159, 0),
	CHAN5G(160, 0), CHAN5G(161, 0),
	CHAN5G(162, 0), CHAN5G(163, 0),
	CHAN5G(164, 0), CHAN5G(165, 0),
	CHAN5G(166, 0), CHAN5G(168, 0),
	CHAN5G(170, 0), CHAN5G(172, 0),
	CHAN5G(174, 0), CHAN5G(176, 0),
	CHAN5G(178, 0), CHAN5G(180, 0),
	CHAN5G(182, 0), CHAN5G(184, 0),
	CHAN5G(186, 0), CHAN5G(188, 0),
	CHAN5G(190, 0), CHAN5G(192, 0),
	CHAN5G(194, 0), CHAN5G(196, 0),
	CHAN5G(198, 0), CHAN5G(200, 0),
	CHAN5G(202, 0), CHAN5G(204, 0),
	CHAN5G(206, 0), CHAN5G(208, 0),
	CHAN5G(210, 0), CHAN5G(212, 0),
	CHAN5G(214, 0), CHAN5G(216, 0),
	CHAN5G(218, 0), CHAN5G(220, 0),
	CHAN5G(222, 0), CHAN5G(224, 0),
	CHAN5G(226, 0), CHAN5G(228, 0),
};

static struct ieee80211_supported_band __wl_band_2ghz = {
	.band = IEEE80211_BAND_2GHZ,
	.channels = __wl_2ghz_channels,
	.n_channels = ARRAY_SIZE(__wl_2ghz_channels),
	.bitrates = wl_g_rates,
	.n_bitrates = wl_g_rates_size,
};

static struct ieee80211_supported_band __wl_band_5ghz_a = {
	.band = IEEE80211_BAND_5GHZ,
	.channels = __wl_5ghz_a_channels,
	.n_channels = ARRAY_SIZE(__wl_5ghz_a_channels),
	.bitrates = wl_a_rates,
	.n_bitrates = wl_a_rates_size,
};

static struct ieee80211_supported_band __wl_band_5ghz_n = {
	.band = IEEE80211_BAND_5GHZ,
	.channels = __wl_5ghz_n_channels,
	.n_channels = ARRAY_SIZE(__wl_5ghz_n_channels),
	.bitrates = wl_a_rates,
	.n_bitrates = wl_a_rates_size,
};

static const u32 __wl_cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
	WLAN_CIPHER_SUITE_AES_CMAC,
};


static void key_endian_to_device(struct wl_wsec_key *key)
{
	key->index = htod32(key->index);
	key->len = htod32(key->len);
	key->algo = htod32(key->algo);
	key->flags = htod32(key->flags);
	key->rxiv.hi = htod32(key->rxiv.hi);
	key->rxiv.lo = htod16(key->rxiv.lo);
	key->iv_initialized = htod32(key->iv_initialized);
}


static void key_endian_to_host(struct wl_wsec_key *key)
{
	key->index = dtoh32(key->index);
	key->len = dtoh32(key->len);
	key->algo = dtoh32(key->algo);
	key->flags = dtoh32(key->flags);
	key->rxiv.hi = dtoh32(key->rxiv.hi);
	key->rxiv.lo = dtoh16(key->rxiv.lo);
	key->iv_initialized = dtoh32(key->iv_initialized);
}


static s32
wl_dev_ioctl(struct net_device *dev, u32 cmd, void *arg, u32 len)
{
	struct ifreq ifr;
	struct wl_ioctl ioc;
	mm_segment_t fs;
	s32 err = 0;

	BUG_ON(len < sizeof(int));

	memset(&ioc, 0, sizeof(ioc));
	ioc.cmd = cmd;
	ioc.buf = arg;
	ioc.len = len;
	strcpy(ifr.ifr_name, dev->name);
	ifr.ifr_data = (caddr_t)&ioc;

	fs = get_fs();
	set_fs(get_ds());
#if defined(WL_USE_NETDEV_OPS)
	err = dev->netdev_ops->ndo_do_ioctl(dev, &ifr, SIOCDEVPRIVATE);
#else
	err = dev->do_ioctl(dev, &ifr, SIOCDEVPRIVATE);
#endif
	set_fs(fs);

	return err;
}


static s32
wl_cfg80211_change_iface(struct wiphy *wiphy, struct net_device *ndev,
                         enum nl80211_iftype type, u32 *flags,
   struct vif_params *params)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	struct wireless_dev *wdev;
	s32 infra = 0;
	s32 ap = 0;
	s32 err = 0;

	switch (type) {
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_WDS:
		WL_ERR(("type (%d) : currently we do not support this type\n",
			type));
		return -EOPNOTSUPP;
	case NL80211_IFTYPE_ADHOC:
		wl->conf->mode = WL_MODE_IBSS;
		break;
	case NL80211_IFTYPE_STATION:
		wl->conf->mode = WL_MODE_BSS;
		infra = 1;
		break;
	default:
		return -EINVAL;
	}
	infra = htod32(infra);
	ap = htod32(ap);
	wdev = ndev->ieee80211_ptr;
	wdev->iftype = type;
	WL_DBG(("%s : ap (%d), infra (%d)\n", ndev->name, ap, infra));
	err = wl_dev_ioctl(ndev, WLC_SET_INFRA, &infra, sizeof(infra));
	if (err) {
		WL_ERR(("WLC_SET_INFRA error (%d)\n", err));
		return err;
	}
	err = wl_dev_ioctl(ndev, WLC_SET_AP, &ap, sizeof(ap));
	if (err) {
		WL_ERR(("WLC_SET_AP error (%d)\n", err));
		return err;
	}

	return 0;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
static s32
wl_cfg80211_scan(struct wiphy *wiphy,
                 struct cfg80211_scan_request *request)
#else
static s32
wl_cfg80211_scan(struct wiphy *wiphy,
                 struct net_device *ndev,
                 struct cfg80211_scan_request *request)
#endif

{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
	struct net_device *ndev = request->wdev->netdev;
#endif
	struct wl_cfg80211_priv *wl = ndev_to_wl(ndev);
	struct cfg80211_ssid *ssids;
	struct wl_cfg80211_scan_req *sr = wl_to_sr(wl);
	s32 passive_scan;
	s32 err = 0;

	if (request) {		/* scan bss */
		ssids = request->ssids;
	}
	else {		/* scan in ibss */
		/* we don't do iscan in ibss */
		ssids = NULL;
	}
	wl->scan_request = request;

	memset(&sr->ssid, 0, sizeof(sr->ssid));

	if (ssids) {
		WL_DBG(("ssid \"%s\", ssid_len (%d)\n", ssids->ssid, ssids->ssid_len));
		sr->ssid.SSID_len = min_t(u8, sizeof(sr->ssid.SSID), ssids->ssid_len);
	}

	if (sr->ssid.SSID_len) {
		memcpy(sr->ssid.SSID, ssids->ssid, sr->ssid.SSID_len);
		sr->ssid.SSID_len = htod32(sr->ssid.SSID_len);
		WL_DBG(("Specific scan ssid=\"%s\" len=%d\n", sr->ssid.SSID, sr->ssid.SSID_len));
	} else {
		WL_DBG(("Broadcast scan\n"));
	}
	WL_DBG(("sr->ssid.SSID_len (%d)\n", sr->ssid.SSID_len));
	passive_scan = wl->active_scan ? 0 : 1;
	err = wl_dev_ioctl(ndev, WLC_SET_PASSIVE_SCAN, &passive_scan, sizeof(passive_scan));
	if (err) {
		WL_ERR(("WLC_SET_PASSIVE_SCAN error (%d)\n", err));
		goto scan_out;
	}
	err = wl_dev_ioctl(ndev, WLC_SCAN, &sr->ssid, sizeof(sr->ssid));
	if (err) {
		if (err == -EBUSY) {
			WL_INF(("system busy : scan for \"%s\" "
				"canceled\n", sr->ssid.SSID));
		} else {
			WL_ERR(("WLC_SCAN error (%d)\n", err));
		}
		goto scan_out;
	}

	return 0;

scan_out:
	wl->scan_request = NULL;
	return err;
}


static s32 wl_dev_intvar_set(struct net_device *dev, s8 *name, s32 val)
{
	s8 buf[WLC_IOCTL_SMLEN];
	u32 len;
	s32 err = 0;

	val = htod32(val);
	len = bcm_mkiovar(name, (char *)(&val), sizeof(val), buf, sizeof(buf));
	BUG_ON(!len);

	err = wl_dev_ioctl(dev, WLC_SET_VAR, buf, len);
	if (err) {
		WL_ERR(("error (%d)\n", err));
	}

	return err;
}


static s32
wl_dev_intvar_get(struct net_device *dev, s8 *name, s32 *retval)
{
	union {
		s8 buf[WLC_IOCTL_SMLEN];
		s32 val;
	} var;
	u32 len;
	u32 data_null;
	s32 err = 0;

	len = bcm_mkiovar(name, (char *)(&data_null), 0, (char *)(&var), sizeof(var.buf));
	BUG_ON(!len);
	err = wl_dev_ioctl(dev, WLC_GET_VAR, &var, len);
	if (err) {
		WL_ERR(("error (%d)\n", err));
	}
	*retval = dtoh32(var.val);

	return err;
}


static s32 wl_set_rts(struct net_device *dev, u32 rts_threshold)
{
	s32 err = 0;

	err = wl_dev_intvar_set(dev, "rtsthresh", rts_threshold);
	if (err) {
		WL_ERR(("Error (%d)\n", err));
		return err;
	}
	return err;
}


static s32 wl_set_frag(struct net_device *dev, u32 frag_threshold)
{
	s32 err = 0;

	err = wl_dev_intvar_set(dev, "fragthresh", frag_threshold);
	if (err) {
		WL_ERR(("Error (%d)\n", err));
		return err;
	}
	return err;
}


static s32 wl_set_retry(struct net_device *dev, u32 retry, bool l)
{
	s32 err = 0;
	u32 cmd = (l ? WLC_SET_LRL : WLC_SET_SRL);

	retry = htod32(retry);
	err = wl_dev_ioctl(dev, cmd, &retry, sizeof(retry));
	if (err) {
		WL_ERR(("cmd (%d) , error (%d)\n", cmd, err));
		return err;
	}
	return err;
}


static s32 wl_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	struct net_device *ndev = wl_to_ndev(wl);
	s32 err = 0;

	if (changed & WIPHY_PARAM_RTS_THRESHOLD &&
	    (wl->conf->rts_threshold != wiphy->rts_threshold)) {
		wl->conf->rts_threshold = wiphy->rts_threshold;
		err = wl_set_rts(ndev, wl->conf->rts_threshold);
		if (!err)
			return err;
	}
	if (changed & WIPHY_PARAM_FRAG_THRESHOLD &&
	    (wl->conf->frag_threshold != wiphy->frag_threshold)) {
		wl->conf->frag_threshold = wiphy->frag_threshold;
		err = wl_set_frag(ndev, wl->conf->frag_threshold);
		if (!err)
			return err;
	}
	if (changed & WIPHY_PARAM_RETRY_LONG && (wl->conf->retry_long != wiphy->retry_long)) {
		wl->conf->retry_long = wiphy->retry_long;
		err = wl_set_retry(ndev, wl->conf->retry_long, true);
		if (!err)
			return err;
	}
	if (changed & WIPHY_PARAM_RETRY_SHORT && (wl->conf->retry_short != wiphy->retry_short)) {
		wl->conf->retry_short = wiphy->retry_short;
		err = wl_set_retry(ndev, wl->conf->retry_short, false);
		if (!err) {
			return err;
		}
	}

	return err;
}


static s32
wl_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
                      struct cfg80211_ibss_params *params)
{
	struct wl_join_params join_params;
	size_t join_params_size;
	s32 val;
	s32 err = 0;

	WL_DBG(("\n"));

	if (params->bssid) {
		WL_ERR(("Invalid bssid\n"));
		return -EOPNOTSUPP;
	}

	if ((err = wl_dev_intvar_set(dev, "auth", 0))) {
		return err;
	}
	if ((err = wl_dev_intvar_set(dev, "wpa_auth", WPA_AUTH_NONE))) {
		return err;
	}
	if ((err = wl_dev_intvar_get(dev, "wsec", &val))) {
		return err;
	}
	val &= ~(WEP_ENABLED | TKIP_ENABLED | AES_ENABLED);
	if ((err = wl_dev_intvar_set(dev, "wsec", val))) {
		return err;
	}

	/*
	 ** Join with specific BSSID and cached SSID
	 ** If SSID is zero join based on BSSID only
	 */
	memset(&join_params, 0, sizeof(join_params));
	join_params_size = sizeof(join_params.ssid);

	memcpy((void *)join_params.ssid.SSID, (void *)params->ssid, params->ssid_len);
	join_params.ssid.SSID_len = htod32(params->ssid_len);
	if (params->bssid)
		memcpy(&join_params.params.bssid, params->bssid, ETHER_ADDR_LEN);
	else
		memset(&join_params.params.bssid, 0, ETHER_ADDR_LEN);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
	wl_ch_to_chanspec(params->chandef.chan, &join_params, &join_params_size);
#else
	wl_ch_to_chanspec(params->channel, &join_params, &join_params_size);
#endif
	err = wl_dev_ioctl(dev, WLC_SET_SSID, &join_params, join_params_size);
	if (err) {
		WL_ERR(("Error (%d)\n", err));
		return err;
	}
	return err;
}


static s32 wl_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	s32 err = 0;

	WL_DBG(("\n"));

	wl_link_down(wl);

	return err;
}


static s32
wl_set_wpa_version(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	s32 val = 0;
	s32 err = 0;

	if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_1)
		val = WPA_AUTH_PSK | WPA_AUTH_UNSPECIFIED;
	else if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_2)
		val = WPA2_AUTH_PSK | WPA2_AUTH_UNSPECIFIED;
	else
		val = WPA_AUTH_DISABLED;
	WL_DBG(("setting wpa_auth to 0x%0x\n", val));
	err = wl_dev_intvar_set(dev, "wpa_auth", val);
	if (err) {
		WL_ERR(("set wpa_auth failed (%d)\n", err));
		return err;
	}
	wl->profile->sec.wpa_versions = sme->crypto.wpa_versions;
	return err;
}


static s32
wl_set_auth_type(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	s32 val = 0;
	s32 err = 0;

	switch (sme->auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
		val = 0;
		WL_DBG(("open system\n"));
		break;
	case NL80211_AUTHTYPE_SHARED_KEY:
		val = 1;
		WL_DBG(("shared key\n"));
		break;
	case NL80211_AUTHTYPE_AUTOMATIC:
		val = 2;
		WL_DBG(("automatic\n"));
		break;
	case NL80211_AUTHTYPE_NETWORK_EAP:
		WL_DBG(("network eap\n"));
	default:
		val = 2;
		WL_ERR(("invalid auth type (%d)\n", sme->auth_type));
		break;
	}

	err = wl_dev_intvar_set(dev, "auth", val);
	if (err) {
		WL_ERR(("set auth failed (%d)\n", err));
		return err;
	}

	wl->profile->sec.auth_type = sme->auth_type;
	return err;
}


static s32
wl_set_set_cipher(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	s32 pval = 0;
	s32 gval = 0;
	s32 val = 0;
	s32 err = 0;

	if (sme->crypto.n_ciphers_pairwise) {
		switch (sme->crypto.ciphers_pairwise[0]) {
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
			pval = WEP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			pval = TKIP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			pval = AES_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_AES_CMAC:
			pval = AES_ENABLED;
			break;
		default:
			WL_ERR(("invalid cipher pairwise (%d)\n", sme->crypto.ciphers_pairwise[0]));
			return -EINVAL;
		}
	}
	if (sme->crypto.cipher_group) {
		switch (sme->crypto.cipher_group) {
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
			gval = WEP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			gval = TKIP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			gval = AES_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_AES_CMAC:
			gval = AES_ENABLED;
			break;
		default:
			WL_ERR(("invalid cipher group (%d)\n", sme->crypto.cipher_group));
			return -EINVAL;
		}
	}

	if ((err = wl_dev_intvar_get(dev, "wsec", &val))) {
		return err;
	}
	val &= ~(WEP_ENABLED | TKIP_ENABLED | AES_ENABLED);
	val |= pval | gval;
	WL_DBG(("set wsec to %d\n", val));
	err = wl_dev_intvar_set(dev, "wsec", val);
	if (err) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}

	wl->profile->sec.cipher_pairwise = sme->crypto.ciphers_pairwise[0];
	wl->profile->sec.cipher_group = sme->crypto.cipher_group;

	return err;
}


static s32
wl_set_key_mgmt(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	s32 val = 0;
	s32 err = 0;

	if (sme->crypto.n_akm_suites) {
		err = wl_dev_intvar_get(dev, "wpa_auth", &val);
		if (err) {
			WL_ERR(("could not get wpa_auth (%d)\n", err));
			return err;
		}
		if (val & (WPA_AUTH_PSK | WPA_AUTH_UNSPECIFIED)) {
			switch (sme->crypto.akm_suites[0]) {
			case WLAN_AKM_SUITE_8021X:
				val = WPA_AUTH_UNSPECIFIED;
				break;
			case WLAN_AKM_SUITE_PSK:
				val = WPA_AUTH_PSK;
				break;
			default:
				WL_ERR(("invalid cipher group (%d)\n", sme->crypto.cipher_group));
				return -EINVAL;
			}
		} else if (val & (WPA2_AUTH_PSK | WPA2_AUTH_UNSPECIFIED)) {
			switch (sme->crypto.akm_suites[0]) {
			case WLAN_AKM_SUITE_8021X:
				val = WPA2_AUTH_UNSPECIFIED;
				break;
			case WLAN_AKM_SUITE_PSK:
				val = WPA2_AUTH_PSK;
				break;
			default:
				WL_ERR(("invalid cipher group (%d)\n", sme->crypto.cipher_group));
				return -EINVAL;
			}
		}

		WL_DBG(("setting wpa_auth to %d\n", val));
		err = wl_dev_intvar_set(dev, "wpa_auth", val);
		if (err) {
			WL_ERR(("could not set wpa_auth (%d)\n", err));
			return err;
		}
	}

	wl->profile->sec.wpa_auth = sme->crypto.akm_suites[0];

	return err;
}


static s32
wl_set_set_sharedkey(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	struct wl_cfg80211_security *sec;
	struct wl_wsec_key key;
	s32 err = 0;

	WL_DBG(("key len (%d)\n", sme->key_len));
	if (sme->key_len) {
		sec = &wl->profile->sec;
		WL_DBG(("wpa_versions 0x%x cipher_pairwise 0x%x\n",
		         sec->wpa_versions, sec->cipher_pairwise));
		if (!(sec->wpa_versions & (NL80211_WPA_VERSION_1 | NL80211_WPA_VERSION_2)) &&
		     (sec->cipher_pairwise &
		     (WLAN_CIPHER_SUITE_WEP40 | WLAN_CIPHER_SUITE_WEP104))) {
			memset(&key, 0, sizeof(key));
			key.len = (u32) sme->key_len;
			key.index = (u32) sme->key_idx;
			if (key.len > sizeof(key.data)) {
				WL_ERR(("Too long key length (%u)\n", key.len));
				return -EINVAL;
			}
			memcpy(key.data, sme->key, key.len);
			key.flags = WL_PRIMARY_KEY;
			switch (sec->cipher_pairwise) {
			case WLAN_CIPHER_SUITE_WEP40:
				key.algo = CRYPTO_ALGO_WEP1;
				break;
			case WLAN_CIPHER_SUITE_WEP104:
				key.algo = CRYPTO_ALGO_WEP128;
				break;
			default:
				WL_ERR(("Invalid algorithm (%d)\n",
				        sme->crypto.ciphers_pairwise[0]));
				return -EINVAL;
			}
			/* Set the new key/index */
			WL_DBG(("key length (%d) key index (%d) algo (%d)\n", key.len,
				key.index, key.algo));
			WL_DBG(("key \"%s\"\n", key.data));
			key_endian_to_device(&key);
			err = wl_dev_ioctl(dev, WLC_SET_KEY, &key, sizeof(key));
			if (err) {
				WL_ERR(("WLC_SET_KEY error (%d)\n", err));
				return err;
			}
		}
	}
	return err;
}


static s32
wl_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
                    struct cfg80211_connect_params *sme)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	struct wl_join_params join_params;
	size_t join_params_size;
	char valc;
	s32 err = 0;

	if (!sme->ssid) {
		WL_ERR(("Invalid ssid\n"));
		return -EOPNOTSUPP;
	}

	WL_DBG(("ie (%p), ie_len (%zd)\n", sme->ie, sme->ie_len));

	err = wl_set_auth_type(dev, sme);
	if (err)
		return err;

	err = wl_set_wpa_version(dev, sme);
	if (err)
		return err;

	err = wl_set_set_cipher(dev, sme);
	if (err)
		return err;

	err = wl_set_key_mgmt(dev, sme);
	if (err)
		return err;

	err = wl_set_set_sharedkey(dev, sme);
	if (err)
		return err;

	valc = 1;
	wl_dev_bufvar_set(dev, "wsec_restrict", &valc, 1);

	if (sme->bssid) {
		memcpy(wl->profile->bssid, sme->bssid, ETHER_ADDR_LEN);
	}
	else {
		memset(wl->profile->bssid, 0, ETHER_ADDR_LEN);
	}

	/*
	 **  Join with specific BSSID and cached SSID
	 **  If SSID is zero join based on BSSID only
	 */
	memset(&join_params, 0, sizeof(join_params));
	join_params_size = sizeof(join_params.ssid);

	join_params.ssid.SSID_len = min(sizeof(join_params.ssid.SSID), sme->ssid_len);
	memcpy(&join_params.ssid.SSID, sme->ssid, join_params.ssid.SSID_len);
	join_params.ssid.SSID_len = htod32(join_params.ssid.SSID_len);
	memcpy(&join_params.params.bssid, &ether_bcast, ETHER_ADDR_LEN);

	memcpy(wl->profile->ssid.SSID, &join_params.ssid.SSID, join_params.ssid.SSID_len);
	wl->profile->ssid.SSID_len = join_params.ssid.SSID_len;

	wl_ch_to_chanspec(sme->channel, &join_params, &join_params_size);
	WL_DBG(("join_param_size %u\n", (unsigned int)join_params_size));

	if (join_params.ssid.SSID_len < IEEE80211_MAX_SSID_LEN) {
		WL_DBG(("ssid \"%s\", len (%d)\n", join_params.ssid.SSID,
		        join_params.ssid.SSID_len));
	}
	err = wl_dev_ioctl(dev, WLC_SET_SSID, &join_params, join_params_size);
	if (err) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}

	set_bit(WL_STATUS_CONNECTING, &wl->status);

	return err;
}


static s32
wl_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev, u16 reason_code)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	scb_val_t scbval;
	s32 err = 0;

	WL_DBG(("Reason %d\n", reason_code));

	if (wl->profile->active) {
		scbval.val = reason_code;
		memcpy(&scbval.ea, &wl->bssid, ETHER_ADDR_LEN);
		scbval.val = htod32(scbval.val);
		err = wl_dev_ioctl(dev, WLC_DISASSOC, &scbval, sizeof(scb_val_t));
		if (err) {
			WL_ERR(("error (%d)\n", err));
			return err;
		}
	}

	return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev,
                         enum nl80211_tx_power_setting type, s32 dbm)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy, enum nl80211_tx_power_setting type, s32 dbm)
#else
#define NL80211_TX_POWER_AUTOMATIC TX_POWER_AUTOMATIC
#define NL80211_TX_POWER_LIMITED TX_POWER_LIMITED
#define NL80211_TX_POWER_FIXED TX_POWER_FIXED
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy, enum tx_power_setting type, s32 dbm)
#endif
{

	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	struct net_device *ndev = wl_to_ndev(wl);
	u16 txpwrmw;
	s32 err = 0;
	s32 disable = 0;

	switch (type) {
	case NL80211_TX_POWER_AUTOMATIC:
		break;
	case NL80211_TX_POWER_LIMITED:
		if (dbm < 0) {
			WL_ERR(("TX_POWER_LIMITTED - dbm is negative\n"));
			return -EINVAL;
		}
		break;
	case NL80211_TX_POWER_FIXED:
		if (dbm < 0) {
			WL_ERR(("TX_POWER_FIXED - dbm is negative..\n"));
			return -EINVAL;
		}
		break;
	}

	/* Make sure radio is off or on as far as software is concerned */
	disable = WL_RADIO_SW_DISABLE << 16;
	disable = htod32(disable);
	err = wl_dev_ioctl(ndev, WLC_SET_RADIO, &disable, sizeof(disable));
	if (err) {
		WL_ERR(("WLC_SET_RADIO error (%d)\n", err));
		return err;
	}

	if (dbm > 0xffff)
		txpwrmw = 0xffff;
	else
		txpwrmw = (u16) dbm;
	err = wl_dev_intvar_set(ndev, "qtxpower", (s32) (bcm_mw_to_qdbm(txpwrmw)));
	if (err) {
		WL_ERR(("qtxpower error (%d)\n", err));
		return err;
	}
	wl->conf->tx_power = dbm;

	return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev, s32 *dbm)
#else
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy, s32 *dbm)
#endif
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	struct net_device *ndev = wl_to_ndev(wl);
	s32 txpwrdbm;
	u8 result;
	s32 err = 0;

	err = wl_dev_intvar_get(ndev, "qtxpower", &txpwrdbm);
	if (err) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}
	result = (u8) (txpwrdbm & ~WL_TXPWR_OVERRIDE);
	*dbm = (s32) bcm_qdbm_to_mw(result);

	return err;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)
static s32
wl_cfg80211_config_default_key(struct wiphy *wiphy,
	struct net_device *dev, u8 key_idx, bool unicast, bool multicast)
#else
static s32
wl_cfg80211_config_default_key(struct wiphy *wiphy,
	struct net_device *dev, u8 key_idx)
#endif
{
	u32 index;
	s32 err = 0;

	WL_DBG(("key index (%d)\n", key_idx));

	/* Just select a new current key */
	index = (u32) key_idx;
	index = htod32(index);
	err = wl_dev_ioctl(dev, WLC_SET_KEY_PRIMARY, &index, sizeof(index));
	if (err) {
		WL_DBG(("error (%d)\n", err));
	}

	return 0;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static s32
wl_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
                    u8 key_idx, bool pairwise, const u8 *mac_addr, struct key_params *params)
#else
static s32
wl_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
                    u8 key_idx, const u8 *mac_addr, struct key_params *params)
#endif
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	struct wl_wsec_key key;
	s32 secval, secnew = 0;
	s32 err = 0;

	WL_DBG(("key index %u    len %u\n", (unsigned)key_idx, params->key_len));

	memset(&key, 0, sizeof(key));

	key.index = (u32) key_idx;

	switch (params->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
		key.algo = CRYPTO_ALGO_WEP1;
		secnew = WEP_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_WEP40\n"));
		break;
	case WLAN_CIPHER_SUITE_WEP104:
		key.algo = CRYPTO_ALGO_WEP128;
		secnew = WEP_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_WEP104\n"));
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		key.algo = CRYPTO_ALGO_TKIP;
		secnew = TKIP_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_TKIP\n"));
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
		key.algo = CRYPTO_ALGO_AES_CCM;
		secnew = AES_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_AES_CMAC\n"));
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		key.algo = CRYPTO_ALGO_AES_CCM;
		secnew = AES_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_CCMP\n"));
		break;
	default:
		WL_ERR(("Invalid cipher (0x%x)\n", params->cipher));
		return -EINVAL;
	}

	if (mac_addr) {
		if (!ETHER_ISMULTI(mac_addr)) {
			memcpy((char *)&key.ea, (void *)mac_addr, ETHER_ADDR_LEN);
		}
	}

	key.len = (u32) params->key_len;
	if (key.len > sizeof(key.data)) {
		WL_ERR(("Too long key length (%u)\n", key.len));
		return -EINVAL;
	}
	memcpy(key.data, params->key, key.len);

	if (params->cipher == WLAN_CIPHER_SUITE_TKIP) {
		u8 keybuf[8];
		memcpy(keybuf, &key.data[24], sizeof(keybuf));
		memcpy(&key.data[24], &key.data[16], sizeof(keybuf));
		memcpy(&key.data[16], keybuf, sizeof(keybuf));
	}

	if (params->seq_len) { /* IW_ENCODE_EXT_RX_SEQ_VALID set */
		u8 *ivptr;
		if (params->seq_len != 6) {
			WL_ERR(("seq_len %d is unexpected, check implementation.\n",
				params->seq_len));
		}
		ivptr = (u8 *) params->seq;
		key.rxiv.hi = (ivptr[5] << 24) | (ivptr[4] << 16) | (ivptr[3] << 8) | ivptr[2];
		key.rxiv.lo = (ivptr[1] << 8) | ivptr[0];
		key.iv_initialized = true;
	}

	/* Set the new key/index */
	key_endian_to_device(&key);
	err = wl_dev_ioctl(dev, WLC_SET_KEY, &key, sizeof(key));
	if (err) {
		WL_ERR(("WLC_SET_KEY error (%d)\n", err));
		return err;
	}

	if ((err = wl_dev_intvar_get(dev, "wsec", &secval))) {
		return err;
	}
	if (secnew == WEP_ENABLED) {
		secval &= ~(TKIP_ENABLED | AES_ENABLED);
	}
	else {
		secval &= ~(WEP_ENABLED);
	}
	secval |= secnew;
	WL_DBG(("set wsec to %d\n", secval));
	err = wl_dev_intvar_set(dev, "wsec", secval);
	if (err) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}

	if (mac_addr) {
		wl->profile->sec.cipher_pairwise = params->cipher;
	}
	else {
		wl->profile->sec.cipher_group = params->cipher;
	}

	return err;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static s32
wl_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
                    u8 key_idx, bool pairwise, const u8 *mac_addr)
#else
static s32
wl_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
                    u8 key_idx, const u8 *mac_addr)
#endif
{
	struct wl_wsec_key key;
	s32 err = 0;

	WL_DBG(("key index (%d)\n", key_idx));

	memset(&key, 0, sizeof(key));

	key.index = (u32) key_idx;
	key.len = 0;
	if (mac_addr) {
		if (!ETHER_ISMULTI(mac_addr)) {
			memcpy((char *)&key.ea, (void *)mac_addr, ETHER_ADDR_LEN);
		}
	}
	key.algo = CRYPTO_ALGO_OFF;

	/* Set the new key/index */
	key_endian_to_device(&key);
	err = wl_dev_ioctl(dev, WLC_SET_KEY, &key, sizeof(key));
	if (err) {
		if (err == -EINVAL) {
			if (key.index >= DOT11_MAX_DEFAULT_KEYS) {
				/* we ignore this key index in this case */
				WL_DBG(("invalid key index (%d)\n", key_idx));
			}
		} else {
			WL_ERR(("WLC_SET_KEY error (%d)\n", err));
		}
		return err;
	}

	return err;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
static s32
wl_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
                    u8 key_idx, bool pairwise, const u8 *mac_addr, void *cookie,
                    void (*callback) (void *cookie, struct key_params * params))
#else
static s32
wl_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
                    u8 key_idx, const u8 *mac_addr, void *cookie,
                    void (*callback) (void *cookie, struct key_params * params))
#endif
{
	struct key_params params;
	struct wl_wsec_key key;
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	struct wl_cfg80211_security *sec;
	s32 wsec;
	s32 err = 0;

	WL_DBG(("key index (%d)\n", key_idx));

	memset(&params, 0, sizeof(params));

	memset(&key, 0, sizeof(key));
	key.index = key_idx;
	key_endian_to_device(&key);

	if ((err = wl_dev_ioctl(dev, WLC_GET_KEY, &key, sizeof(key)))) {
		return err;
	}
	key_endian_to_host(&key);

	params.key_len = (u8) min_t(u8, DOT11_MAX_KEY_SIZE, key.len);
	memcpy(params.key, key.data, params.key_len);

	if ((err = wl_dev_ioctl(dev, WLC_GET_WSEC, &wsec, sizeof(wsec)))) {
		return err;
	}
	wsec = dtoh32(wsec);
	switch (wsec) {
	case WEP_ENABLED:
		sec = &wl->profile->sec;
		if (sec->cipher_pairwise & WLAN_CIPHER_SUITE_WEP40) {
			params.cipher = WLAN_CIPHER_SUITE_WEP40;
			WL_DBG(("WLAN_CIPHER_SUITE_WEP40\n"));
		} else if (sec->cipher_pairwise & WLAN_CIPHER_SUITE_WEP104) {
			params.cipher = WLAN_CIPHER_SUITE_WEP104;
			WL_DBG(("WLAN_CIPHER_SUITE_WEP104\n"));
		}
		break;
	case TKIP_ENABLED:
		params.cipher = WLAN_CIPHER_SUITE_TKIP;
		WL_DBG(("WLAN_CIPHER_SUITE_TKIP\n"));
		break;
	case AES_ENABLED:
		params.cipher = WLAN_CIPHER_SUITE_AES_CMAC;
		WL_DBG(("WLAN_CIPHER_SUITE_AES_CMAC\n"));
		break;
	default:
		WL_ERR(("Invalid algo (0x%x)\n", wsec));
		return -EINVAL;
	}

	callback(cookie, &params);
	return err;
}


static s32
wl_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
                        u8 *mac, struct station_info *sinfo)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	scb_val_t scb_val;
	int rssi;
	s32 rate;
	s32 err = 0;

	if (memcmp(mac, wl->profile->bssid, ETHER_ADDR_LEN)) {
		WL_ERR(("Wrong Mac address, mac = %pM   profile =%pM\n", mac, wl->profile->bssid));
		return -ENOENT;
	}

	/* Report the current tx rate */
	err = wl_dev_ioctl(dev, WLC_GET_RATE, &rate, sizeof(rate));
	if (err) {
		WL_DBG(("Could not get rate (%d)\n", err));
	} else {
		rate = dtoh32(rate);
		sinfo->filled |= STATION_INFO_TX_BITRATE;
		sinfo->txrate.legacy = rate * 5;
		WL_DBG(("Rate %d Mbps\n", (rate / 2)));
	}

	if (test_bit(WL_STATUS_CONNECTED, &wl->status)) {
		memset(&scb_val, 0, sizeof(scb_val));
		err = wl_dev_ioctl(dev, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t));
		if (err) {
			WL_DBG(("Could not get rssi (%d)\n", err));
			return err;
		}
		rssi = dtoh32(scb_val.val);
		sinfo->filled |= STATION_INFO_SIGNAL;
		sinfo->signal = rssi;
		WL_DBG(("RSSI %d dBm\n", rssi));
	}

	return err;
}


static s32
wl_cfg80211_set_power_mgmt(struct wiphy *wiphy, struct net_device *dev,
                           bool enabled, s32 timeout)
{
	s32 pm;
	s32 err = 0;

	pm = enabled ? PM_FAST : PM_OFF;
	pm = htod32(pm);
	WL_DBG(("power save %s\n", (pm ? "enabled" : "disabled")));
	err = wl_dev_ioctl(dev, WLC_SET_PM, &pm, sizeof(pm));
	if (err) {
		if (err == -ENODEV)
			WL_DBG(("net_device is not ready yet\n"));
		else
			WL_ERR(("error (%d)\n", err));
		return err;
	}
	return err;
}


static __used s32
wl_update_pmklist(struct net_device *dev, struct wl_cfg80211_pmk_list *pmk_list, s32 err)
{
	int i, j;

	WL_DBG(("No of elements %d\n", pmk_list->pmkids.npmkid));
	for (i = 0; i < pmk_list->pmkids.npmkid; i++) {
		WL_DBG(("PMKID[%d]: %pM =\n", i,
			&pmk_list->pmkids.pmkid[i].BSSID));
		for (j = 0; j < WPA2_PMKID_LEN; j++) {
			WL_DBG(("%02x\n", pmk_list->pmkids.pmkid[i].PMKID[j]));
		}
	}
	if (!err) {
		err = wl_dev_bufvar_set(dev, "pmkid_info", (char *)pmk_list, sizeof(*pmk_list));
	}

	return err;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)

static s32
wl_cfg80211_set_pmksa(struct wiphy *wiphy, struct net_device *dev,
                      struct cfg80211_pmksa *pmksa)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	s32 err = 0;
	int i;

	for (i = 0; i < wl->pmk_list->pmkids.npmkid; i++)
		if (!memcmp(pmksa->bssid, &wl->pmk_list->pmkids.pmkid[i].BSSID, ETHER_ADDR_LEN))
			break;
	if (i < WL_NUM_PMKIDS_MAX) {
		memcpy(&wl->pmk_list->pmkids.pmkid[i].BSSID, pmksa->bssid, ETHER_ADDR_LEN);
		memcpy(&wl->pmk_list->pmkids.pmkid[i].PMKID, pmksa->pmkid, WPA2_PMKID_LEN);
		if (i == wl->pmk_list->pmkids.npmkid)
			wl->pmk_list->pmkids.npmkid++;
	} else {
		err = -EINVAL;
	}
	WL_DBG(("set_pmksa,IW_PMKSA_ADD - PMKID: %pM =\n",
		&wl->pmk_list->pmkids.pmkid[wl->pmk_list->pmkids.npmkid].BSSID));
	for (i = 0; i < WPA2_PMKID_LEN; i++) {
		WL_DBG(("%02x\n",
			wl->pmk_list->pmkids.pmkid[wl->pmk_list->pmkids.npmkid].PMKID[i]));
	}

	err = wl_update_pmklist(dev, wl->pmk_list, err);

	return err;
}


static s32
wl_cfg80211_del_pmksa(struct wiphy *wiphy, struct net_device *dev,
                      struct cfg80211_pmksa *pmksa)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	struct _pmkid_list pmkid;
	s32 err = 0;
	int i;

	memcpy(&pmkid.pmkid[0].BSSID, pmksa->bssid, ETHER_ADDR_LEN);
	memcpy(&pmkid.pmkid[0].PMKID, pmksa->pmkid, WPA2_PMKID_LEN);

	WL_DBG(("del_pmksa,IW_PMKSA_REMOVE - PMKID: %pM =\n",
		&pmkid.pmkid[0].BSSID));
	for (i = 0; i < WPA2_PMKID_LEN; i++) {
		WL_DBG(("%02x\n", pmkid.pmkid[0].PMKID[i]));
	}

	for (i = 0; i < wl->pmk_list->pmkids.npmkid; i++)
		if (!memcmp(pmksa->bssid, &wl->pmk_list->pmkids.pmkid[i].BSSID, ETHER_ADDR_LEN))
			break;

	if ((wl->pmk_list->pmkids.npmkid > 0) && (i < wl->pmk_list->pmkids.npmkid)) {
		memset(&wl->pmk_list->pmkids.pmkid[i], 0, sizeof(pmkid_t));
		for (; i < (wl->pmk_list->pmkids.npmkid - 1); i++) {
			memcpy(&wl->pmk_list->pmkids.pmkid[i].BSSID,
			       &wl->pmk_list->pmkids.pmkid[i + 1].BSSID, ETHER_ADDR_LEN);
			memcpy(&wl->pmk_list->pmkids.pmkid[i].PMKID,
			       &wl->pmk_list->pmkids.pmkid[i + 1].PMKID, WPA2_PMKID_LEN);
		}
		wl->pmk_list->pmkids.npmkid--;
	} else {
		err = -EINVAL;
	}

	err = wl_update_pmklist(dev, wl->pmk_list, err);

	return err;

}


static s32
wl_cfg80211_flush_pmksa(struct wiphy *wiphy, struct net_device *dev)
{
	struct wl_cfg80211_priv *wl = wiphy_to_wl(wiphy);
	s32 err = 0;

	memset(wl->pmk_list, 0, sizeof(*wl->pmk_list));
	err = wl_update_pmklist(dev, wl->pmk_list, err);
	return err;

}

#endif  /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33) */


static struct cfg80211_ops wl_cfg80211_ops = {
	.change_virtual_intf = wl_cfg80211_change_iface,
	.scan = wl_cfg80211_scan,
	.set_wiphy_params = wl_cfg80211_set_wiphy_params,
	.join_ibss = wl_cfg80211_join_ibss,
	.leave_ibss = wl_cfg80211_leave_ibss,
	.get_station = wl_cfg80211_get_station,
	.set_tx_power = wl_cfg80211_set_tx_power,
	.get_tx_power = wl_cfg80211_get_tx_power,
	.add_key = wl_cfg80211_add_key,
	.del_key = wl_cfg80211_del_key,
	.get_key = wl_cfg80211_get_key,
	.set_default_key = wl_cfg80211_config_default_key,
	.set_power_mgmt = wl_cfg80211_set_power_mgmt,
	.connect = wl_cfg80211_connect,
	.disconnect = wl_cfg80211_disconnect,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	.set_pmksa = wl_cfg80211_set_pmksa,
	.del_pmksa = wl_cfg80211_del_pmksa,
	.flush_pmksa = wl_cfg80211_flush_pmksa
#endif
};


static s32 wl_mode_to_nl80211_iftype(s32 mode)
{
	s32 err = 0;

	switch (mode) {
	case WL_MODE_BSS:
		return NL80211_IFTYPE_STATION;
	case WL_MODE_IBSS:
		return NL80211_IFTYPE_ADHOC;
	default:
		return NL80211_IFTYPE_UNSPECIFIED;
	}

	return err;
}


static s32 wl_alloc_wdev(struct device *dev, struct wireless_dev **rwdev)
{
	struct wireless_dev *wdev;
	s32 err = 0;

	wdev = kzalloc(sizeof(*wdev), GFP_KERNEL);
	if (!wdev) {
		WL_ERR(("Could not allocate wireless device\n"));
		err = -ENOMEM;
		goto early_out;
	}
	wdev->wiphy = wiphy_new(&wl_cfg80211_ops, sizeof(struct wl_cfg80211_priv));
	if (!wdev->wiphy) {
		WL_ERR(("Couldn not allocate wiphy device\n"));
		err = -ENOMEM;
		goto wiphy_new_out;
	}
	set_wiphy_dev(wdev->wiphy, dev);
	wdev->wiphy->max_scan_ssids = WL_NUM_SCAN_MAX;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	wdev->wiphy->max_num_pmkids = WL_NUM_PMKIDS_MAX;
#endif
	wdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_ADHOC);
	wdev->wiphy->bands[IEEE80211_BAND_2GHZ] = &__wl_band_2ghz;
	wdev->wiphy->bands[IEEE80211_BAND_5GHZ] = &__wl_band_5ghz_a; /* Set it as 11a by default */
	wdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wdev->wiphy->cipher_suites = __wl_cipher_suites;
	wdev->wiphy->n_cipher_suites = ARRAY_SIZE(__wl_cipher_suites);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	/* Change PM to default off, same as in wext implementation. */
	wdev->wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;
#endif
	err = wiphy_register(wdev->wiphy);
	if (err < 0) {
		WL_ERR(("Couldn not register wiphy device (%d)\n", err));
		goto wiphy_register_out;
	}

	*rwdev = wdev;
	return err;

wiphy_register_out:
	wiphy_free(wdev->wiphy);

wiphy_new_out:
	kfree(wdev);

early_out:
	*rwdev = wdev;
	return err;
}


static void wl_free_wdev(struct wl_cfg80211_priv *wl)
{
	struct wireless_dev *wdev = wl_to_wdev(wl);

	if (!wdev) {
		WL_ERR(("wdev is invalid\n"));
		return;
	}
	wiphy_unregister(wdev->wiphy);
	wiphy_free(wdev->wiphy);
	kfree(wdev);
	wl_to_wdev(wl) = NULL;
}


static s32 wl_inform_bss(struct wl_cfg80211_priv *wl, struct wl_scan_results *bss_list)
{
	struct wl_bss_info *bi = NULL;	/* must be initialized */
	s32 err = 0;
	int i;

	if (bss_list->version != WL_BSS_INFO_VERSION) {
		WL_ERR(("Version %d != WL_BSS_INFO_VERSION\n", bss_list->version));
		return -EOPNOTSUPP;
	}
	WL_DBG(("scanned AP count (%d)\n", bss_list->count));
	bi = next_bss(bss_list, bi);
	for_each_bss(bss_list, bi, i) {
		err = wl_inform_single_bss(wl, bi);
		if (err)
			break;
	}
	return err;
}

static s32 wl_inform_single_bss(struct wl_cfg80211_priv *wl, struct wl_bss_info *bi)
{
	struct wiphy *wiphy = wl_to_wiphy(wl);
	struct ieee80211_mgmt *mgmt;
	struct ieee80211_channel *channel;
	struct wl_cfg80211_bss_info *notif_bss_info;
	struct wl_cfg80211_scan_req *sr = wl_to_sr(wl);
	struct beacon_proberesp *beacon_proberesp;
	struct cfg80211_bss *cbss = NULL;
	s32 mgmt_type;
	u32 signal;
	u32 freq;
	s32 err = 0;
	u8 *notify_ie;
	size_t notify_ielen;

	if (dtoh32(bi->length) > WL_BSS_INFO_MAX) {
		WL_DBG(("Beacon is larger than buffer. Discarding\n"));
		return err;
	}
	notif_bss_info = kzalloc(sizeof(*notif_bss_info) + sizeof(*mgmt) - sizeof(u8) +
	                         WL_BSS_INFO_MAX, GFP_KERNEL);
	if (!notif_bss_info) {
		WL_ERR(("notif_bss_info alloc failed\n"));
		return -ENOMEM;
	}
	mgmt = (struct ieee80211_mgmt *)notif_bss_info->frame_buf;
	notif_bss_info->channel = wf_chspec_ctlchan(bi->chanspec);

	notif_bss_info->rssi = bi->RSSI;
	memcpy(mgmt->bssid, &bi->BSSID, ETHER_ADDR_LEN);
	mgmt_type = wl->active_scan ?	IEEE80211_STYPE_PROBE_RESP : IEEE80211_STYPE_BEACON;
	if (!memcmp(bi->SSID, sr->ssid.SSID, bi->SSID_len)) {
		mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | mgmt_type);
	}
	beacon_proberesp = wl->active_scan ? (struct beacon_proberesp *)&mgmt->u.probe_resp :
	                   (struct beacon_proberesp *)&mgmt->u.beacon;
	beacon_proberesp->timestamp = 0;
	beacon_proberesp->beacon_int = cpu_to_le16(bi->beacon_period);
	beacon_proberesp->capab_info = cpu_to_le16(bi->capability);
	wl_rst_ie(wl);
	/*
	* wl_add_ie is not necessary because it can only add duplicated
	* SSID, rate information to frame_buf
	*/
	/*
	* wl_add_ie(wl, WLAN_EID_SSID, bi->SSID_len, bi->SSID);
	* wl_add_ie(wl, WLAN_EID_SUPP_RATES, bi->rateset.count,
	* bi->rateset.rates);
	*/
	wl_mrg_ie(wl, ((u8 *) bi) + bi->ie_offset, bi->ie_length);
	wl_cp_ie(wl, beacon_proberesp->variable, WL_BSS_INFO_MAX -
	         offsetof(struct wl_cfg80211_bss_info, frame_buf));
	notif_bss_info->frame_len = offsetof(struct ieee80211_mgmt, u.beacon.variable) +
	                            wl_get_ielen(wl);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
	freq = ieee80211_channel_to_frequency(notif_bss_info->channel,
		(notif_bss_info->channel <= CH_MAX_2G_CHANNEL) ?
		IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ);
#else
	freq = ieee80211_channel_to_frequency(notif_bss_info->channel);
#endif
	if (freq == 0) {
		WL_ERR(("Invalid channel, fail to chcnage channel to freq\n"));
		kfree(notif_bss_info);
		return -EINVAL;
	}
	channel = ieee80211_get_channel(wiphy, freq);
	if (unlikely(!channel)) {
		WL_ERR(("ieee80211_get_channel error\n"));
		kfree(notif_bss_info);
		return -EINVAL;
	}

	WL_DBG(("SSID : \"%s\", rssi %d, channel %d, capability : 0x04%x, bssid %pM\n",
		bi->SSID, notif_bss_info->rssi, notif_bss_info->channel,
		mgmt->u.beacon.capab_info, &bi->BSSID));

	signal = notif_bss_info->rssi * 100;
	cbss = cfg80211_inform_bss_frame(wiphy, channel, mgmt,
	    le16_to_cpu(notif_bss_info->frame_len), signal, GFP_KERNEL);
	if (unlikely(!cbss)) {
		WL_ERR(("cfg80211_inform_bss_frame error\n"));
		kfree(notif_bss_info);
		return -EINVAL;
	}

	notify_ie = (u8 *)bi + le16_to_cpu(bi->ie_offset);
	notify_ielen = le32_to_cpu(bi->ie_length);
	cbss = cfg80211_inform_bss(wiphy, channel, (const u8 *)(bi->BSSID.octet),
		0, beacon_proberesp->capab_info, beacon_proberesp->beacon_int,
		(const u8 *)notify_ie, notify_ielen, signal, GFP_KERNEL);

	if (unlikely(!cbss))
		return -ENOMEM;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
	cfg80211_put_bss(wiphy, cbss);
#else
	cfg80211_put_bss(cbss);
#endif

	kfree(notif_bss_info);

	return err;
}


static s32
wl_notify_connect_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
                         const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
	u32 event = EVENT_TYPE(e);
	u16 flags = EVENT_FLAGS(e);
	u32 status = EVENT_STATUS(e);

	WL_DBG(("\n"));

	if (!wl_is_ibssmode(wl)) {
		if (event == WLC_E_LINK && (flags & WLC_EVENT_MSG_LINK)) {
			wl_link_up(wl);
			wl_bss_connect_done(wl, ndev, e, data, true);
			wl->profile->active = true;
		}
		else if ((event == WLC_E_LINK && ~(flags & WLC_EVENT_MSG_LINK)) ||
			event == WLC_E_DEAUTH_IND || event == WLC_E_DISASSOC_IND) {
			cfg80211_disconnected(ndev, 0, NULL, 0, GFP_KERNEL);
			clear_bit(WL_STATUS_CONNECTED, &wl->status);
			wl_link_down(wl);
			wl_init_prof(wl->profile);
		}
		else if (event == WLC_E_SET_SSID && status == WLC_E_STATUS_NO_NETWORKS) {
			wl_bss_connect_done(wl, ndev, e, data, false);
		}
		else {
			WL_DBG(("no action (BSS mode)\n"));
		}
	}
	else {
		if (event == WLC_E_JOIN) {
			WL_DBG(("joined in IBSS network\n"));
		}
		if (event == WLC_E_START) {
			WL_DBG(("started IBSS network\n"));
		}
		if (event == WLC_E_JOIN || event == WLC_E_START) {
			wl_link_up(wl);
			wl_get_assoc_ies(wl);
			memcpy(&wl->bssid, &e->addr, ETHER_ADDR_LEN);
			wl_update_bss_info(wl);
			cfg80211_ibss_joined(ndev, (u8 *)&wl->bssid, GFP_KERNEL);
			set_bit(WL_STATUS_CONNECTED, &wl->status);
			wl->profile->active = true;
		}
		else if ((event == WLC_E_LINK && ~(flags & WLC_EVENT_MSG_LINK)) ||
			event == WLC_E_DEAUTH_IND || event == WLC_E_DISASSOC_IND) {
			clear_bit(WL_STATUS_CONNECTED, &wl->status);
			wl_link_down(wl);
			wl_init_prof(wl->profile);
		}
		else if (event == WLC_E_SET_SSID && status == WLC_E_STATUS_NO_NETWORKS) {
			WL_DBG(("no action - join fail (IBSS mode)\n"));
		}
		else {
			WL_DBG(("no action (IBSS mode)\n"));
		}
	}

	return err;
}


static s32
wl_notify_roaming_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
                         const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
	u32 status = EVENT_STATUS(e);

	WL_DBG(("\n"));

	if (status == WLC_E_STATUS_SUCCESS) {
		err = wl_bss_roaming_done(wl, ndev, e, data);
		wl->profile->active = true;
	}

	return err;
}


static __used s32
wl_dev_bufvar_set(struct net_device *dev, s8 *name, s8 *buf, s32 len)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	u32 buflen;

	buflen = bcm_mkiovar(name, buf, len, wl->ioctl_buf, WL_IOCTL_LEN_MAX);
	BUG_ON(!buflen);

	return wl_dev_ioctl(dev, WLC_SET_VAR, wl->ioctl_buf, buflen);
}


static s32
wl_dev_bufvar_get(struct net_device *dev, s8 *name, s8 *buf, s32 buf_len)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(dev);
	u32 len;
	s32 err = 0;

	len = bcm_mkiovar(name, NULL, 0, wl->ioctl_buf, WL_IOCTL_LEN_MAX);
	BUG_ON(!len);
	err = wl_dev_ioctl(dev, WLC_GET_VAR, (void *)wl->ioctl_buf, WL_IOCTL_LEN_MAX);
	if (err) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}
	memcpy(buf, wl->ioctl_buf, buf_len);

	return err;
}


static s32 wl_get_assoc_ies(struct wl_cfg80211_priv *wl)
{
	struct net_device *ndev = wl_to_ndev(wl);
	struct wl_cfg80211_assoc_ielen *assoc_info;
	struct wl_cfg80211_connect_info *conn_info = wl_to_conn(wl);
	u32 req_len;
	u32 resp_len;
	s32 err = 0;

	err = wl_dev_bufvar_get(ndev, "assoc_info", wl->extra_buf, WL_ASSOC_INFO_MAX);
	if (err) {
		WL_ERR(("could not get assoc info (%d)\n", err));
		return err;
	}
	assoc_info = (struct wl_cfg80211_assoc_ielen *)wl->extra_buf;
	req_len = assoc_info->req_len;
	resp_len = assoc_info->resp_len;
	if (req_len) {
		err = wl_dev_bufvar_get(ndev, "assoc_req_ies", wl->extra_buf, WL_ASSOC_INFO_MAX);
		if (err) {
			WL_ERR(("could not get assoc req (%d)\n", err));
			return err;
		}
		conn_info->req_ie_len = req_len;
		conn_info->req_ie =
		    kmemdup(wl->extra_buf, conn_info->req_ie_len, GFP_KERNEL);
	} else {
		conn_info->req_ie_len = 0;
		conn_info->req_ie = NULL;
	}
	if (resp_len) {
		err = wl_dev_bufvar_get(ndev, "assoc_resp_ies", wl->extra_buf, WL_ASSOC_INFO_MAX);
		if (err) {
			WL_ERR(("could not get assoc resp (%d)\n", err));
			return err;
		}
		conn_info->resp_ie_len = resp_len;
		conn_info->resp_ie =
		    kmemdup(wl->extra_buf, conn_info->resp_ie_len, GFP_KERNEL);
	} else {
		conn_info->resp_ie_len = 0;
		conn_info->resp_ie = NULL;
	}
	WL_DBG(("req len (%d) resp len (%d)\n", conn_info->req_ie_len,
		conn_info->resp_ie_len));

	return err;
}


static void wl_ch_to_chanspec(struct ieee80211_channel *chan, struct wl_join_params *join_params,
	size_t *join_params_size)
{
	chanspec_t chanspec = 0;

	if (chan) {
		join_params->params.chanspec_num = 1;
		join_params->params.chanspec_list[0] =
		    ieee80211_frequency_to_channel(chan->center_freq);

		if (chan->band == IEEE80211_BAND_2GHZ) {
			chanspec |= WL_CHANSPEC_BAND_2G;
		}
		else if (chan->band == IEEE80211_BAND_5GHZ) {
			chanspec |= WL_CHANSPEC_BAND_5G;
		}
		else {
			WL_ERR(("Unknown band\n"));
			BUG();
		}

		chanspec |= WL_CHANSPEC_BW_20;

		*join_params_size += WL_ASSOC_PARAMS_FIXED_SIZE +
			join_params->params.chanspec_num * sizeof(chanspec_t);

		join_params->params.chanspec_list[0] &= WL_CHANSPEC_CHAN_MASK;
		join_params->params.chanspec_list[0] |= chanspec;
		join_params->params.chanspec_list[0] =
		    htodchanspec(join_params->params.chanspec_list[0]);

		join_params->params.chanspec_num = htod32(join_params->params.chanspec_num);

		WL_DBG(("join_params->params.chanspec_list[0]= %#X, channel %d, chanspec %#X\n",
		        join_params->params.chanspec_list[0],
		        join_params->params.chanspec_list[0], chanspec));
	}
}


static s32 wl_update_bss_info(struct wl_cfg80211_priv *wl)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
	struct wiphy *wiphy = wl_to_wiphy(wl);
#endif
	struct cfg80211_bss *bss;
	struct wl_bss_info *bi;
	struct wlc_ssid *ssid;
	struct bcm_tlv *tim;
	s32 dtim_period;
	size_t ie_len;
	u8 *ie;
	s32 err = 0;

	ssid = &wl->profile->ssid;
	bss = cfg80211_get_bss(wl_to_wiphy(wl), NULL, (s8 *)&wl->bssid,
	      ssid->SSID, ssid->SSID_len, WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

	rtnl_lock();
	if (!bss) {
		WL_DBG(("Could not find the AP\n"));
		*(u32 *) wl->extra_buf = htod32(WL_EXTRA_BUF_MAX);
		err = wl_dev_ioctl(wl_to_ndev(wl), WLC_GET_BSS_INFO, wl->extra_buf,
		                   WL_EXTRA_BUF_MAX);
		if (err) {
			WL_ERR(("Could not get bss info %d\n", err));
			goto update_bss_info_out;
		}
		bi = (struct wl_bss_info *)(wl->extra_buf + 4);
		if (memcmp(&bi->BSSID, &wl->bssid, ETHER_ADDR_LEN)) {
			err = -EIO;
			goto update_bss_info_out;
		}
		err = wl_inform_single_bss(wl, bi);
		if (err)
			goto update_bss_info_out;

		ie = ((u8 *)bi) + bi->ie_offset;
		ie_len = bi->ie_length;
	} else {
		WL_DBG(("Found the AP in the list - BSSID %pM\n", bss->bssid));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
		ie = (u8 *)(bss->ies->data);
		ie_len = bss->ies->len;
#else
		ie = bss->information_elements;
		ie_len = bss->len_information_elements;
#endif
		wl->conf->channel = *bss->channel;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
		cfg80211_put_bss(wiphy, bss);
#else
		cfg80211_put_bss(bss);
#endif
	}

	tim = bcm_parse_tlvs(ie, ie_len, WLAN_EID_TIM);
	if (tim) {
		dtim_period = tim->data[1];
	} else {
		/*
		* active scan was done so we could not get dtim
		* information out of probe response.
		* so we speficially query dtim information to dongle.
		*/
		err = wl_dev_ioctl(wl_to_ndev(wl), WLC_GET_DTIMPRD,
			&dtim_period, sizeof(dtim_period));
		if (err) {
			WL_ERR(("WLC_GET_DTIMPRD error (%d)\n", err));
			goto update_bss_info_out;
		}
	}

update_bss_info_out:
	rtnl_unlock();
	return err;
}


static s32
wl_bss_roaming_done(struct wl_cfg80211_priv *wl, struct net_device *ndev,
                    const wl_event_msg_t *e, void *data)
{
	struct wl_cfg80211_connect_info *conn_info = wl_to_conn(wl);
	s32 err = 0;

	wl_get_assoc_ies(wl);
	memcpy(wl->profile->bssid, &e->addr, ETHER_ADDR_LEN);
	memcpy(&wl->bssid, &e->addr, ETHER_ADDR_LEN);
	wl_update_bss_info(wl);
	cfg80211_roamed(ndev,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39)
			&wl->conf->channel,	 /* channel of the new AP */
#endif
			(u8 *)&wl->bssid,
			conn_info->req_ie, conn_info->req_ie_len,
			conn_info->resp_ie, conn_info->resp_ie_len, GFP_KERNEL);
	WL_DBG(("Report roaming result\n"));

	set_bit(WL_STATUS_CONNECTED, &wl->status);

	return err;
}


static s32
wl_bss_connect_done(struct wl_cfg80211_priv *wl, struct net_device *ndev,
                    const wl_event_msg_t *e, void *data, bool completed)
{
	struct wl_cfg80211_connect_info *conn_info = wl_to_conn(wl);
	s32 err = 0;

	if (wl->scan_request) {
		WL_DBG(("%s: Aborting scan\n", __FUNCTION__));
		cfg80211_scan_done(wl->scan_request, true);     /* abort */
		wl->scan_request = NULL;
	}

	if (test_and_clear_bit(WL_STATUS_CONNECTING, &wl->status)) {
		if (completed) {
			wl_get_assoc_ies(wl);
			memcpy(&wl->bssid, &e->addr, ETHER_ADDR_LEN);
			memcpy(wl->profile->bssid, &e->addr, ETHER_ADDR_LEN);
			wl_update_bss_info(wl);
			set_bit(WL_STATUS_CONNECTED, &wl->status);
		}

		WL_DBG(("Reporting BSS network join result \"%s\"\n",
			wl->profile->ssid.SSID));
		cfg80211_connect_result(ndev, (u8 *)&wl->bssid,	conn_info->req_ie,
		    conn_info->req_ie_len, conn_info->resp_ie, conn_info->resp_ie_len,
		    completed ? WLAN_STATUS_SUCCESS : WLAN_STATUS_AUTH_TIMEOUT,	GFP_KERNEL);
		WL_DBG(("Connection %s\n", completed ? "Succeeded" : "FAILed"));
	}

	return err;
}


static s32
wl_notify_mic_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
                     const wl_event_msg_t *e, void *data)
{
	u16 flags = EVENT_FLAGS(e);
	enum nl80211_key_type key_type;

	WL_DBG(("\n"));

	rtnl_lock();
	if (flags & WLC_EVENT_MSG_GROUP)
		key_type = NL80211_KEYTYPE_GROUP;
	else
		key_type = NL80211_KEYTYPE_PAIRWISE;

	cfg80211_michael_mic_failure(ndev, (u8 *)&e->addr, key_type, -1, NULL, GFP_KERNEL);
	rtnl_unlock();

	return 0;
}


static s32
wl_notify_scan_status(struct wl_cfg80211_priv *wl, struct net_device *ndev,
                      const wl_event_msg_t *e, void *data)
{
	struct channel_info channel_inform;
	struct wl_scan_results *bss_list;
	u32 buflen;
	s32 err = 0;

	WL_DBG(("\n"));

	rtnl_lock();
	err = wl_dev_ioctl(ndev, WLC_GET_CHANNEL, &channel_inform, sizeof(channel_inform));
	if (err) {
		WL_ERR(("scan busy (%d)\n", err));
		goto scan_done_out;
	}
	channel_inform.scan_channel = dtoh32(channel_inform.scan_channel);
	if (channel_inform.scan_channel) {

		WL_DBG(("channel_inform.scan_channel (%d)\n",	channel_inform.scan_channel));
	}

	for (buflen = WL_SCAN_BUF_BASE; ; ) {
		bss_list = (struct wl_scan_results *) kmalloc(buflen, GFP_KERNEL);
		if (!bss_list) {
			WL_ERR(("%s Out of memory for scan results, (%d)\n", ndev->name, err));
			goto scan_done_out;
		}
		memset(bss_list, 0, buflen);
		bss_list->buflen = htod32(buflen);
		err = wl_dev_ioctl(ndev, WLC_SCAN_RESULTS, bss_list, buflen);
		if (!err) {
			break;
		}
		else if (err == -E2BIG) {
			kfree(bss_list);
			buflen *= 2;
		}
		else {
			WL_ERR(("%s Scan_results error (%d)\n", ndev->name, err));
			kfree(bss_list);
			err = -EINVAL;
			goto scan_done_out;
		}
	}

	bss_list->buflen = dtoh32(bss_list->buflen);
	bss_list->version = dtoh32(bss_list->version);
	bss_list->count = dtoh32(bss_list->count);

	err = wl_inform_bss(wl, bss_list);
	kfree(bss_list);

scan_done_out:
	if (wl->scan_request) {
		cfg80211_scan_done(wl->scan_request, false);
		wl->scan_request = NULL;
	}
	rtnl_unlock();
	return err;
}


static void wl_init_conf(struct wl_cfg80211_conf *conf)
{
	conf->mode = (u32)-1;
	conf->frag_threshold = (u32)-1;
	conf->rts_threshold = (u32)-1;
	conf->retry_short = (u32)-1;
	conf->retry_long = (u32)-1;
	conf->tx_power = -1;
}


static void wl_init_prof(struct wl_cfg80211_profile *prof)
{
	memset(prof, 0, sizeof(*prof));
}


static void wl_init_eloop_handler(struct wl_cfg80211_event_loop *el)
{
	memset(el, 0, sizeof(*el));
	el->handler[WLC_E_SCAN_COMPLETE] = wl_notify_scan_status;
	el->handler[WLC_E_JOIN] = wl_notify_connect_status;
	el->handler[WLC_E_START] = wl_notify_connect_status;
	el->handler[WLC_E_LINK] = wl_notify_connect_status;
	el->handler[WLC_E_NDIS_LINK] = wl_notify_connect_status;
	el->handler[WLC_E_SET_SSID] = wl_notify_connect_status;
	el->handler[WLC_E_DISASSOC_IND] = wl_notify_connect_status;
	el->handler[WLC_E_DEAUTH_IND] = wl_notify_connect_status;
	el->handler[WLC_E_ROAM] = wl_notify_roaming_status;
	el->handler[WLC_E_MIC_ERROR] = wl_notify_mic_status;
}


static s32 wl_init_priv_mem(struct wl_cfg80211_priv *wl)
{
	wl->conf = (void *)kzalloc(sizeof(*wl->conf), GFP_KERNEL);
	if (!wl->conf) {
		WL_ERR(("wl_cfg80211_conf alloc failed\n"));
		goto init_priv_mem_out;
	}
	wl->profile = (void *)kzalloc(sizeof(*wl->profile), GFP_KERNEL);
	if (!wl->profile) {
		WL_ERR(("wl_cfg80211_profile alloc failed\n"));
		goto init_priv_mem_out;
	}
	wl->scan_req_int = (void *)kzalloc(sizeof(*wl->scan_req_int), GFP_KERNEL);
	if (!wl->scan_req_int) {
		WL_ERR(("Scan req alloc failed\n"));
		goto init_priv_mem_out;
	}
	wl->ioctl_buf = (void *)kzalloc(WL_IOCTL_LEN_MAX, GFP_KERNEL);
	if (!wl->ioctl_buf) {
		WL_ERR(("Ioctl buf alloc failed\n"));
		goto init_priv_mem_out;
	}
	wl->extra_buf = (void *)kzalloc(WL_EXTRA_BUF_MAX, GFP_KERNEL);
	if (!wl->extra_buf) {
		WL_ERR(("Extra buf alloc failed\n"));
		goto init_priv_mem_out;
	}

	wl->pmk_list = (void *)kzalloc(sizeof(*wl->pmk_list), GFP_KERNEL);
	if (!wl->pmk_list) {
		WL_ERR(("pmk list alloc failed\n"));
		goto init_priv_mem_out;
	}

	return 0;

init_priv_mem_out:
	wl_deinit_priv_mem(wl);

	return -ENOMEM;
}


static void wl_deinit_priv_mem(struct wl_cfg80211_priv *wl)
{
	kfree(wl->conf);
	wl->conf = NULL;
	kfree(wl->profile);
	wl->profile = NULL;
	kfree(wl->scan_req_int);
	wl->scan_req_int = NULL;
	kfree(wl->ioctl_buf);
	wl->ioctl_buf = NULL;
	kfree(wl->extra_buf);
	wl->extra_buf = NULL;
	kfree(wl->pmk_list);
	wl->pmk_list = NULL;
}


static s32 wl_create_event_handler(struct wl_cfg80211_priv *wl)
{
	sema_init(&wl->event_sync, 0);
	wl->event_tsk = kthread_run(wl_event_handler, wl, "wl_event_handler");
	if (IS_ERR(wl->event_tsk)) {
		wl->event_tsk = NULL;
		WL_ERR(("failed to create event thread\n"));
		return -ENOMEM;
	}
	return 0;
}


static void wl_destroy_event_handler(struct wl_cfg80211_priv *wl)
{
	if (wl->event_tsk) {
		send_sig(SIGTERM, wl->event_tsk, 1);
		kthread_stop(wl->event_tsk);
		wl->event_tsk = NULL;
	}
}


static s32 wl_init_cfg80211_priv(struct wl_cfg80211_priv *wl, struct wireless_dev *wdev)
{
	s32 err = 0;

	wl->wdev = wdev;

	wl->scan_request = NULL;
	wl->active_scan = true;
	wl_init_eq(wl);
	err = wl_init_priv_mem(wl);
	if (err)
		return err;

	if (wl_create_event_handler(wl))
		return -ENOMEM;

	wl_init_eloop_handler(&wl->el);

	if (err)
		return err;

	wl_init_conf(wl->conf);
	wl_init_prof(wl->profile);
	wl_link_down(wl);

	return err;
}


static void wl_deinit_cfg80211_priv(struct wl_cfg80211_priv *wl)
{
	wl_destroy_event_handler(wl);
	wl_flush_eq(wl);
	wl_link_down(wl);
	wl_deinit_priv_mem(wl);
}


s32 wl_cfg80211_attach(struct net_device *ndev, struct device *dev)
{
	struct wireless_dev *wdev;
	struct wl_cfg80211_priv *wl;
	s32 err = 0;

	if (!ndev) {
		WL_ERR(("ndev is invaild\n"));
		return -ENODEV;
	}

	err = wl_alloc_wdev(dev, &wdev);
	if (err < 0) {
		return err;
	}

	wdev->iftype = wl_mode_to_nl80211_iftype(WL_MODE_BSS);
	wl = wdev_to_wl(wdev);
	ndev->ieee80211_ptr = wdev;
	SET_NETDEV_DEV(ndev, wiphy_dev(wdev->wiphy));
	wdev->netdev = ndev;
	err = wl_init_cfg80211_priv(wl, wdev);
	if (err) {
		WL_ERR(("Failed to init iwm_priv (%d)\n", err));
		goto cfg80211_attach_out;
	}

	if (!err) {
		WL_INF(("Registered CFG80211 phy\n"));
	}
	return err;

cfg80211_attach_out:
	wl_free_wdev(wl);
	return err;
}


void wl_cfg80211_detach(struct net_device *ndev)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(ndev);

	wl_deinit_cfg80211_priv(wl);
	wl_free_wdev(wl);
}


static void wl_wakeup_event(struct wl_cfg80211_priv *wl)
{
	up(&wl->event_sync);
}


static s32 wl_event_handler(void *data)
{
	struct wl_cfg80211_priv *wl = (struct wl_cfg80211_priv *)data;
	struct wl_cfg80211_event_q *e;

	allow_signal(SIGTERM);
	while (!down_interruptible(&wl->event_sync)) {
		if (kthread_should_stop())
			break;
		e = wl_deq_event(wl);
		if (!e) {
			WL_ERR(("eqeue empty..\n"));
			BUG();
		}
		if (wl->el.handler[e->etype]) {
			WL_DBG(("event type (%d)\n", e->etype));
			wl->el.handler[e->etype] (wl, wl_to_ndev(wl), &e->emsg, e->edata);
		} else {
			WL_DBG(("Unknown Event (%d): ignoring\n", e->etype));
		}
		wl_put_event(e);
	}
	WL_DBG(("%s was terminated\n", __func__));
	return 0;
}


void
wl_cfg80211_event(struct net_device *ndev, const wl_event_msg_t * e, void *data)
{

	u32 event_type = EVENT_TYPE(e);

	struct wl_cfg80211_priv *wl = ndev_to_wl(ndev);
#if defined(WL_DBGMSG_ENABLE)
	s8 *estr = (event_type <= sizeof(wl_dbg_estr) / WL_DBG_ESTR_MAX - 1) ?
	    wl_dbg_estr[event_type] : (s8 *) "Unknown";
	WL_DBG(("event_type (%d):" "WLC_E_" "%s\n", event_type, estr));
#endif				/* (WL_DBG_LEVEL > 0) */
	if (!wl_enq_event(wl, event_type, e, data))
		wl_wakeup_event(wl);
}

static void wl_init_eq(struct wl_cfg80211_priv *wl)
{
	wl_init_eq_lock(wl);
	INIT_LIST_HEAD(&wl->eq_list);
}

static void wl_flush_eq(struct wl_cfg80211_priv *wl)
{
	struct wl_cfg80211_event_q *e;

	wl_lock_eq(wl);
	while (!list_empty(&wl->eq_list)) {
		e = list_first_entry(&wl->eq_list, struct wl_cfg80211_event_q, eq_list);
		list_del(&e->eq_list);
		kfree(e);
	}
	wl_unlock_eq(wl);
}


/*
* retrieve first queued event from head
*/
static struct wl_cfg80211_event_q *wl_deq_event(struct wl_cfg80211_priv *wl)
{
	struct wl_cfg80211_event_q *e = NULL;

	wl_lock_eq(wl);
	if (!list_empty(&wl->eq_list)) {
		e = list_first_entry(&wl->eq_list, struct wl_cfg80211_event_q, eq_list);
		list_del(&e->eq_list);
	}
	wl_unlock_eq(wl);

	return e;
}


/*
** push event to tail of the queue
*/
static s32
wl_enq_event(struct wl_cfg80211_priv *wl, u32 event, const wl_event_msg_t *msg, void *data)
{
	struct wl_cfg80211_event_q *e;
	s32 err = 0;

	e = kzalloc(sizeof(struct wl_cfg80211_event_q), GFP_ATOMIC);
	if (!e) {
		WL_ERR(("event alloc failed\n"));
		return -ENOMEM;
	}

	e->etype = event;
	memcpy(&e->emsg, msg, sizeof(wl_event_msg_t));
	if (data) {
	}

	spin_lock(&wl->eq_lock);
	list_add_tail(&e->eq_list, &wl->eq_list);
	spin_unlock(&wl->eq_lock);

	return err;
}


static void wl_put_event(struct wl_cfg80211_event_q *e)
{
	kfree(e);
}


static s32 wl_set_mode(struct net_device *ndev, s32 iftype)
{
	s32 infra = 0;
	s32 ap = 0;
	s32 err = 0;

	switch (iftype) {
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_WDS:
		WL_ERR(("type (%d) : currently we do not support this mode\n",
			iftype));
		err = -EINVAL;
		return err;
	case NL80211_IFTYPE_ADHOC:
		break;
	case NL80211_IFTYPE_STATION:
		infra = 1;
		break;
	default:
		err = -EINVAL;
		WL_ERR(("invalid type (%d)\n", iftype));
		return err;
	}
	infra = htod32(infra);
	ap = htod32(ap);
	WL_DBG(("%s ap (%d), infra (%d)\n", ndev->name, ap, infra));
	err = wl_dev_ioctl(ndev, WLC_SET_INFRA, &infra, sizeof(infra));
	if (err) {
		WL_ERR(("WLC_SET_INFRA error (%d)\n", err));
		return err;
	}
	err = wl_dev_ioctl(ndev, WLC_SET_AP, &ap, sizeof(ap));
	if (err) {
		WL_ERR(("WLC_SET_AP error (%d)\n", err));
		return err;
	}

	return 0;
}


static s32 wl_update_wiphybands(struct wl_cfg80211_priv *wl)
{
	struct wiphy *wiphy;
	s32 phy_list;
	s8 phy;
	s32 err = 0;

	err = wl_dev_ioctl(wl_to_ndev(wl), WLC_GET_PHYLIST, &phy_list, sizeof(phy_list));
	if (err) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}

	phy = ((char *)&phy_list)[0];
	WL_DBG(("%c phy\n", phy));

	if (phy == 'n' || phy == 'a' || phy == 'v') {
		wiphy = wl_to_wiphy(wl);
		wiphy->bands[IEEE80211_BAND_5GHZ] = &__wl_band_5ghz_n;
	}

	return err;
}


s32 wl_cfg80211_up(struct net_device *ndev)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(ndev);
	s32 err = 0;
	struct wireless_dev *wdev = ndev->ieee80211_ptr;

	wl_set_mode(ndev, wdev->iftype);

	err = wl_update_wiphybands(wl);

	return err;
}


s32 wl_cfg80211_down(struct net_device *ndev)
{
	struct wl_cfg80211_priv *wl = ndev_to_wl(ndev);
	s32 err = 0;

	if (wl->scan_request) {
		cfg80211_scan_done(wl->scan_request, true);	/* abort */
		wl->scan_request = NULL;
	}

	return err;
}


static bool wl_is_ibssmode(struct wl_cfg80211_priv *wl)
{
	return wl->conf->mode == WL_MODE_IBSS;
}


static void wl_rst_ie(struct wl_cfg80211_priv *wl)
{
	struct wl_cfg80211_ie *ie = wl_to_ie(wl);

	ie->offset = 0;
}


static __used s32 wl_add_ie(struct wl_cfg80211_priv *wl, u8 t, u8 l, u8 *v)
{
	struct wl_cfg80211_ie *ie = wl_to_ie(wl);
	s32 err = 0;

	if (ie->offset + l + 2 > WL_TLV_INFO_MAX) {
		WL_ERR(("ei crosses buffer boundary\n"));
		return -ENOSPC;
	}
	ie->buf[ie->offset] = t;
	ie->buf[ie->offset + 1] = l;
	memcpy(&ie->buf[ie->offset + 2], v, l);
	ie->offset += l + 2;

	return err;
}


static s32 wl_mrg_ie(struct wl_cfg80211_priv *wl, u8 *ie_stream, u16 ie_size)
{
	struct wl_cfg80211_ie *ie = wl_to_ie(wl);
	s32 err = 0;

	if (ie->offset + ie_size > WL_TLV_INFO_MAX) {
		WL_ERR(("ei_stream crosses buffer boundary\n"));
		return -ENOSPC;
	}
	memcpy(&ie->buf[ie->offset], ie_stream, ie_size);
	ie->offset += ie_size;

	return err;
}


static s32 wl_cp_ie(struct wl_cfg80211_priv *wl, u8 *dst, u16 dst_size)
{
	struct wl_cfg80211_ie *ie = wl_to_ie(wl);
	s32 err = 0;

	if (ie->offset > dst_size) {
		WL_ERR(("dst_size is not enough\n"));
		return -ENOSPC;
	}
	memcpy(dst, &ie->buf[0], ie->offset);

	return err;
}


static u32 wl_get_ielen(struct wl_cfg80211_priv *wl)
{
	struct wl_cfg80211_ie *ie = wl_to_ie(wl);

	return ie->offset;
}


static void wl_link_up(struct wl_cfg80211_priv *wl)
{
	WL_DBG(("\n"));
}


static void wl_link_down(struct wl_cfg80211_priv *wl)
{
	struct wl_cfg80211_connect_info *conn_info = wl_to_conn(wl);

	WL_DBG(("\n"));

	kfree(conn_info->req_ie);
	conn_info->req_ie = NULL;
	conn_info->req_ie_len = 0;
	kfree(conn_info->resp_ie);
	conn_info->resp_ie = NULL;
	conn_info->resp_ie_len = 0;
}


static void wl_lock_eq(struct wl_cfg80211_priv *wl)
{
	spin_lock_irq(&wl->eq_lock);
}


static void wl_unlock_eq(struct wl_cfg80211_priv *wl)
{
	spin_unlock_irq(&wl->eq_lock);
}


static void wl_init_eq_lock(struct wl_cfg80211_priv *wl)
{
	spin_lock_init(&wl->eq_lock);
}

#endif /* USE_CFG80211 */
