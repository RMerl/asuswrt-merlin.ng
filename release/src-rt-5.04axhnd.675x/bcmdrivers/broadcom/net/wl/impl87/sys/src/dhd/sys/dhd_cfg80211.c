/*
 * Linux cfg80211 driver - Dongle Host Driver (DHD) related
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: wl_cfg80211.c,v 1.1.4.1.2.14 2011/02/09 01:40:07 Exp $
 */

#include <linux/vmalloc.h>
#include <net/rtnetlink.h>
#include <typedefs.h>

#include <bcmutils.h>
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <wl_core.h>
#include <dhd_cfg80211.h>

#if defined(STB) && !defined(STBAP)
#include <dhd_linux.h>
#endif

#ifdef PKT_FILTER_SUPPORT
#include <dngl_stats.h>
#include <dhd.h>
#endif

#ifdef PKT_FILTER_SUPPORT
extern uint dhd_pkt_filter_enable;
extern uint dhd_master_mode;
extern void dhd_pktfilter_offload_enable(dhd_pub_t * dhd, char *arg, int enable, int master_mode);
#endif

#if defined(STB) && !defined(STBAP)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39))
#if defined(BCMDBUS) && !defined(USB_SUSPEND_AVAILABLE)
void dhd_set_wowl(dhd_pub_t *dhdp, int state);
#endif /* BCMDBUS && USB_SUSPEND_AVAILABLE */
#endif /* KERNEL_VERSION(2, 6, 39) */
#endif /* STB && STBAP */

static int dhd_dongle_up = FALSE;

#if defined(BCMDONGLEHOST)
#include <dngl_stats.h>
#include <dhd.h>
#include <dhdioctl.h>
#include <wlioctl.h>
#include <brcm_nl80211.h>
#include <dhd_cfg80211.h>
#endif /* defined(BCMDONGLEHOST) */

static s32 wl_dongle_up(struct net_device *ndev);
static s32 wl_dongle_down(struct net_device *ndev);
#ifndef OEM_ANDROID
static s32 wl_dongle_power(struct net_device *ndev, u32 power_mode);
static s32 wl_dongle_roam(struct net_device *ndev, u32 roamvar,	u32 bcn_timeout);
static s32 wl_dongle_scantime(struct net_device *ndev, s32 scan_assoc_time, s32 scan_unassoc_time);
#ifndef BCA_HNDROUTER
static s32 wl_dongle_offload(struct net_device *ndev, s32 arpoe, s32 arp_ol);
static s32 wl_dongle_filter(struct net_device *ndev, u32 filter_mode);
static s32 wl_pattern_atoh(s8 *src, s8 *dst);
#endif /* BCA_HNDROUTER */
#endif /* OEM_ANDROID */

/**
 * Function implementations
 */

s32 dhd_cfg80211_init(struct bcm_cfg80211 *cfg)
{
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_deinit(struct bcm_cfg80211 *cfg)
{
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_down(struct bcm_cfg80211 *cfg)
{
	struct net_device *ndev;
	s32 err = 0;

	WL_TRACE(("In\n"));
	if (!dhd_dongle_up) {
		WL_ERR(("Dongle is already down\n"));
		return err;
	}

	ndev = bcmcfg_to_prmry_ndev(cfg);
	wl_dongle_down(ndev);
	dhd_dongle_up = FALSE;
	return 0;
}

s32 dhd_cfg80211_set_p2p_info(struct bcm_cfg80211 *cfg, int val)
{
	cfg->wlcore->op_mode |= val;
	WL_ERR(("Set : op_mode=0x%04x\n", cfg->wlcore->op_mode));
#ifdef ARP_OFFLOAD_SUPPORT
	if (cfg->wlcore->arp_version == 1) {
		/* IF P2P is enabled, disable arpoe */
		wl_cfg80211_arp_offload_set(cfg, 0);
		wl_cfg80211_arp_offload_enable(cfg, false);
	}
#endif /* ARP_OFFLOAD_SUPPORT */

	return 0;
}

s32 dhd_cfg80211_clean_p2p_info(struct bcm_cfg80211 *cfg)
{
	cfg->wlcore->op_mode &= ~(DHD_FLAG_P2P_GC_MODE | DHD_FLAG_P2P_GO_MODE);
	WL_ERR(("Clean : op_mode=0x%04x\n", cfg->wlcore->op_mode));

#ifdef ARP_OFFLOAD_SUPPORT
	if (cfg->wlcore->arp_version == 1) {
		/* IF P2P is disabled, enable arpoe back for STA mode. */
		wl_cfg80211_arp_offload_set(cfg, dhd_arp_mode);
		wl_cfg80211_arp_offload_enable(cfg, true);
	}
#endif /* ARP_OFFLOAD_SUPPORT */

	return 0;
}

struct net_device* wl_cfg80211_allocate_if(struct bcm_cfg80211 *cfg, int ifidx, const char *name,
	uint8 *mac, uint8 bssidx, const char *dngl_name)
{
	struct net_device* net;

	DHD_LOCK(cfg->pub);
	net = dhd_allocate_if(cfg->pub, ifidx, name, mac, bssidx, FALSE, dngl_name);
	DHD_UNLOCK(cfg->pub);

	return net;
}

int wl_register_interface(void *pub,
	int ifidx, struct net_device* ndev, bool rtnl_lock_reqd)
{
	return dhd_register_if(pub, ifidx, rtnl_lock_reqd);
}

int wl_cfg80211_remove_if(struct bcm_cfg80211 *cfg,
	int ifidx, struct net_device* ndev, bool rtnl_lock_reqd)
{
	int ret;

	DHD_LOCK(cfg->pub);
	ret = dhd_remove_if(cfg->pub, ifidx, rtnl_lock_reqd);
	DHD_UNLOCK(cfg->pub);

	return ret;
}
static s32
wl_dongle_up(struct net_device *ndev)
{
	s32 err = 0;
	u32 local_up = 0;

	err = wldev_ioctl_set(ndev, WLC_UP, &local_up, sizeof(local_up));
	if (unlikely(err)) {
		WL_ERR(("WLC_UP error (%d)\n", err));
	}
	return err;
}

	static s32
wl_dongle_down(struct net_device *ndev)
{
	s32 err = 0;
	u32 local_down = 0;

	err = wldev_ioctl_set(ndev, WLC_DOWN, &local_down, sizeof(local_down));
	if (unlikely(err)) {
		WL_ERR(("WLC_DOWN error (%d)\n", err));
	}
	return err;
}

#ifndef OEM_ANDROID
static s32 wl_dongle_power(struct net_device *ndev, u32 power_mode)
{
	s32 err = 0;

	WL_TRACE(("In\n"));
	err = wldev_ioctl_set(ndev, WLC_SET_PM, &power_mode, sizeof(power_mode));
	if (unlikely(err)) {
		WL_ERR(("WLC_SET_PM error (%d)\n", err));
	}
	return err;
}

static s32
wl_dongle_roam(struct net_device *ndev, u32 roamvar, u32 bcn_timeout)
{
	s32 err = 0;

	/* Setup timeout if Beacons are lost and roam is off to report link down */
	if (roamvar) {
		err = wldev_iovar_setint(ndev, "bcn_timeout", bcn_timeout);
		if (unlikely(err)) {
			WL_ERR(("bcn_timeout error (%d)\n", err));
			goto dongle_rom_out;
		}
	}
#ifdef MEDIA_CFG
	/* Enable/Disable built-in roaming to allow supplicant to take care of roaming */
	err = wldev_iovar_setint(ndev, "roam_off", roamvar);
	if (unlikely(err)) {
		WL_ERR(("roam_off error (%d)\n", err));
		goto dongle_rom_out;
	}
#endif /* MEDIA_CFG */
dongle_rom_out:
	return err;
}

static s32
wl_dongle_scantime(struct net_device *ndev, s32 scan_assoc_time,
	s32 scan_unassoc_time)
{
	s32 err = 0;

	err = wldev_ioctl_set(ndev, WLC_SET_SCAN_CHANNEL_TIME, &scan_assoc_time,
		sizeof(scan_assoc_time));
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("Scan assoc time is not supported\n"));
		} else {
			WL_ERR(("Scan assoc time error (%d)\n", err));
		}
		goto dongle_scantime_out;
	}
	err = wldev_ioctl_set(ndev, WLC_SET_SCAN_UNASSOC_TIME, &scan_unassoc_time,
		sizeof(scan_unassoc_time));
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("Scan unassoc time is not supported\n"));
		} else {
			WL_ERR(("Scan unassoc time error (%d)\n", err));
		}
		goto dongle_scantime_out;
	}

dongle_scantime_out:
	return err;
}

#ifndef BCA_HNDROUTER
static s32
wl_dongle_offload(struct net_device *ndev, s32 arpoe, s32 arp_ol)
{
	s32 err = 0;

	/* Set ARP offload */
	err = wldev_iovar_setint(ndev, "arpoe", arpoe);
	if (err) {
		if (err == -EOPNOTSUPP)
			WL_INFORM(("arpoe is not supported\n"));
		else
			WL_ERR(("arpoe error (%d)\n", err));

		goto dongle_offload_out;
	}
	err = wldev_iovar_setint(ndev, "arp_ol", arp_ol);
	if (err) {
		if (err == -EOPNOTSUPP)
			WL_INFORM(("arp_ol is not supported\n"));
		else
			WL_ERR(("arp_ol error (%d)\n", err));

		goto dongle_offload_out;
	}

dongle_offload_out:
	return err;
}

static s32 wl_dongle_filter(struct net_device *ndev, u32 filter_mode)
{
	const s8 *str;
	struct wl_pkt_filter pkt_filter;
	struct wl_pkt_filter *pkt_filterp;
	s32 buf_len;
	s32 str_len;
	u32 mask_size;
	u32 pattern_size;
	s8 buf[64] = {0};
	s32 err = 0;

	/* add a default packet filter pattern */
	str = "pkt_filter_add";
	str_len = strlen(str);
	strncpy(buf, str, sizeof(buf) - 1);
	buf[ sizeof(buf) - 1 ] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (struct wl_pkt_filter *)(buf + str_len + 1);

	/* Parse packet filter id. */
	pkt_filter.id = htod32(100);

	/* Parse filter polarity. */
	pkt_filter.negate_match = htod32(0);

	/* Parse filter type. */
	pkt_filter.type = htod32(0);

	/* Parse pattern filter offset. */
	pkt_filter.u.pattern.offset = htod32(0);

	/* Parse pattern filter mask. */
	mask_size = htod32(wl_pattern_atoh("0xff",
		(char *)pkt_filterp->u.pattern.
		    mask_and_pattern));

	/* Parse pattern filter pattern. */
	pattern_size = htod32(wl_pattern_atoh("0x00",
		(char *)&pkt_filterp->u.pattern.mask_and_pattern[mask_size]));

	if (mask_size != pattern_size) {
		WL_ERR(("Mask and pattern not the same size\n"));
		err = -EINVAL;
		goto dongle_filter_out;
	}

	pkt_filter.u.pattern.size_bytes = mask_size;
	buf_len += WL_PKT_FILTER_FIXED_LEN;
	buf_len += (WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * mask_size);

	/* Keep-alive attributes are set in local
	 * variable (keep_alive_pkt), and
	 * then memcpy'ed into buffer (keep_alive_pktp) since there is no
	 * guarantee that the buffer is properly aligned.
	 */
	memcpy((char *)pkt_filterp, &pkt_filter,
		WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN);

	err = wldev_ioctl_set(ndev, WLC_SET_VAR, buf, buf_len);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("filter not supported\n"));
		} else {
			WL_ERR(("filter (%d)\n", err));
		}
		goto dongle_filter_out;
	}

	/* set mode to allow pattern */
	err = wldev_iovar_setint(ndev, "pkt_filter_mode", filter_mode);
	if (err) {
		if (err == -EOPNOTSUPP) {
			WL_INFORM(("filter_mode not supported\n"));
		} else {
			WL_ERR(("filter_mode (%d)\n", err));
		}
		goto dongle_filter_out;
	}

dongle_filter_out:
	return err;
}

static s32 wl_pattern_atoh(s8 *src, s8 *dst)
{
	int i;
	if (strncmp(src, "0x", 2) != 0 && strncmp(src, "0X", 2) != 0) {
		WL_ERR(("Mask invalid format. Needs to start with 0x\n"));
		return -1;
	}
	src = src + 2;		/* Skip past 0x */
	if (strlen(src) % 2 != 0) {
		WL_ERR(("Mask invalid format. Needs to be of even length\n"));
		return -1;
	}
	for (i = 0; *src != '\0'; i++) {
		char num[3];
		strncpy(num, src, 2);
		num[2] = '\0';
		dst[i] = (u8) simple_strtoul(num, NULL, 16);
		src += 2;
	}
	return i;
}
#endif /* BCA_HNDROUTER */

#endif /* OEM_ANDROID */

s32 dhd_config_dongle(struct bcm_cfg80211 *cfg)
{
#ifndef DHD_SDALIGN
#define DHD_SDALIGN	32
#endif
	struct net_device *ndev;
	s32 err = 0;

	WL_TRACE(("In\n"));
	if (dhd_dongle_up) {
		WL_ERR(("Dongle is already up\n"));
		return err;
	}

	ndev = bcmcfg_to_prmry_ndev(cfg);

	err = wl_dongle_up(ndev);
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_up failed\n"));
		goto default_conf_out;
	}
#ifndef OEM_ANDROID
#ifdef BCA_HNDROUTER
	err = wl_dongle_power(ndev, PM_OFF);
#else
	err = wl_dongle_power(ndev, PM_FAST);
#endif
	if (unlikely(err)) {
		WL_ERR(("wl_dongle_power failed\n"));
		goto default_conf_out;
	}
	if (ndev->ieee80211_ptr->iftype != NL80211_IFTYPE_AP) {
		err = wl_dongle_roam(ndev, (cfg->roam_on ? 0 : 1), 3);
		if (unlikely(err)) {
			WL_ERR(("wl_dongle_roam failed\n"));
			goto default_conf_out;
		}
	}
	wl_dongle_scantime(ndev, 40, 80);

#ifndef BCA_HNDROUTER
	/* ARP offload and PKT filter not supported for router platform */
	wl_dongle_offload(ndev, 1, 0xf);
	wl_dongle_filter(ndev, 1);
#endif /* BCA_HNDROUTER */

#endif /* OEM_ANDROID */
	dhd_dongle_up = true;

default_conf_out:

	return err;

}

#ifdef WL_VENDOR_EXT_SUPPORT
int dhd_cfgvendor_priv_string_handler(struct bcm_cfg80211 *cfg, struct wireless_dev *wdev,
	const struct bcm_nlmsg_hdr *nlioc, void *buf)
{
	struct net_device *ndev = NULL;
	dhd_pub_t *dhd;
	dhd_ioctl_t ioc = { 0 };
	int ret = 0;
	int8 index;

	WL_TRACE(("entry: cmd = %d\n", nlioc->cmd));

	dhd = cfg->pub;
	DHD_OS_WAKE_LOCK(dhd);
	DHD_LOCK(dhd);

#if defined(OEM_ANDROID)
	/* send to dongle only if we are not waiting for reload already */
	if (dhd->hang_was_sent) {
		WL_ERR(("HANG was sent up earlier\n"));
		DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE(dhd, DHD_EVENT_TIMEOUT_MS);
		DHD_UNLOCK(dhd);
		DHD_OS_WAKE_UNLOCK(dhd);
		return OSL_ERROR(BCME_DONGLE_DOWN);
	}
#endif /* (OEM_ANDROID) */

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	index = dhd_net2idx(dhd->info, ndev);
	if (index == DHD_BAD_IF) {
		WL_ERR(("Bad ifidx from wdev:%p\n", wdev));
		ret = BCME_ERROR;
		goto done;
	}

	ioc.cmd = nlioc->cmd;
	ioc.len = nlioc->len;
	ioc.set = nlioc->set;
	ioc.driver = nlioc->magic;
	ret = dhd_ioctl_process(dhd, index, &ioc, buf);
	if (ret) {
		WL_TRACE(("dhd_ioctl_process return err %d\n", ret));
		ret = OSL_ERROR(ret);
		goto done;
	}

done:
	DHD_UNLOCK(dhd);
	DHD_OS_WAKE_UNLOCK(dhd);
	return ret;
}
#endif /* WL_VENDOR_EXT_SUPPORT */

#if defined(STB) && !defined(STBAP)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39))
#if defined(BCMDBUS) && !defined(USB_SUSPEND_AVAILABLE)
void dhd_cfg80211_set_wowl(struct bcm_cfg80211 *cfg, int state)
{
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	dhd_set_wowl(dhd, state);
}
#endif /* BCMDBUS && USB_SUSPEND_AVAILABLE */
#endif /* KERNEL_VERSION(2, 6, 39) */
#endif /* STB && STBAP */

void
wl_os_set_ioctl_resp_timeout(unsigned int timeout_msec)
{
	dhd_os_set_ioctl_resp_timeout(timeout_msec);
}
#if defined(OEM_ANDROID) && !defined(AP) && defined(WLP2P)
uint32
wl_cfg80211_get_concurrent_capabilites(struct  bcm_cfg80211 *cfg)
{
	return dhd_get_concurrent_capabilites((dhd_pub_t *)cfg->pub);
}
#endif /* OEM_ANDROID && !AP && WLP2P */
void
wl_wlfc_enable(struct bcm_cfg80211 *cfg, bool enable)
{
#ifdef PROP_TXSTATUS_VSDB
#endif /* PROP_TXSTATUS_VSDB */
}
#ifdef PROP_TXSTATUS
	int
wl_cfg80211_wlfc_deinit(struct bcm_cfg80211 *cfg)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd_wlfc_deinit(dhd);
}
#endif
	int
wl_net2idx(struct bcm_cfg80211 *cfg, struct net_device *net)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd_net2idx(dhd->info, net);
}
#if defined(CUSTOMER_HW4)
#if defined(FORCE_DISABLE_SINGLECORE_SCAN)
	void
wl_cfg80211_force_disable_singlcore_scan(struct bcm_cfg80211 *cfg)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	dhd_force_disable_singlcore_scan(dhd);

}
#endif /* FORCE_DISABLE_SINGLECORE_SCAN */
int wl_get_roam_env_detection(struct bcm_cfg80211 *cfg)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd->roam_env_detection;
}
#endif /* CUSTOMER_HW4 */
#ifdef CUSTOM_SET_CPUCORE
void wl_set_cpucore(struct bcm_cfg80211 *cfg, int set)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	dhd_set_cpucore(dhd, set);
}
#endif /* CUSTOM_SET_CPUCORE */
#if defined(PKT_FILTER_SUPPORT) && defined(APSTA_BLOCK_ARP_DURING_DHCP)
	int
wl_cfg80211_packet_filter_add_remove(struct bcm_cfg80211 *cfg, int add_remove, int num)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd_packet_filter_add_remove(dhd, add_remove, num);
}
#endif /* wl_cfg80211_packet_filter_add_remove */

#ifdef PCIE_FULL_DONGLE
/* Set interface specific ap_isolate configuration */
int wl_cfg80211_set_ap_isolate(struct bcm_cfg80211 *cfg, uint32 idx, int val)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd_set_ap_isolate(dhd, idx, val);
}
#endif /* PCIE_FULL_DONGLE */
#ifdef PNO_SUPPORT
/* Linux wrapper to call common dhd_pno_set_for_ssid */
	int
wl_cfg80211_dev_pno_set_for_ssid(struct net_device *dev, wlc_ssid_ext_t* ssids_local, int nssid,
		uint16  scan_fr, int pno_repeat, int pno_freq_expo_max,
		uint16 *channel_list, int nchan)
{
	return dhd_dev_pno_set_for_ssid(dev, ssids_local, nssid, scan_fr, pno_repeat,
			pno_freq_expo_max, channel_list, nchan);
}
#endif /* PNO_SUPPORT */
	int
wl_ifname2idx(struct bcm_cfg80211 *cfg, char *name)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd_ifname2idx(dhd->info, name);
}
#ifdef PCIE_FULL_DONGLE
	void
wl_flow_rings_delete_for_peer(struct bcm_cfg80211 *cfg, uint8 ifindex, char *addr)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	dhd_flow_rings_delete_for_peer(dhd, ifindex, addr);
}
#endif /* PCIE_FULL_DONGLE */
#ifdef GSCAN_SUPPORT
void *wl_dev_process_epno_result(struct net_device *dev,
		const void  *data, uint32 event, int *send_evt_bytes)
{
	return dhd_dev_process_epno_result(dev, data, event, send_evt_bytes);
}
	bool
wl_dev_is_legacy_pno_enabled(struct net_device *dev)
{
	return dhd_dev_is_legacy_pno_enabled(dev);
}
#endif /* GSCAN_SUPPORT */
int wl_do_driver_init(struct net_device *net)
{
	return dhd_do_driver_init(net);
}
int wl_get_busstate(struct bcm_cfg80211 *cfg)
{
	dhd_pub_t *dhd;
	dhd = (dhd_pub_t *)(cfg->pub);
	return dhd->busstate;
}
struct net_device *wl_idx2net(void *pub, int ifidx)
{
	return dhd_idx2net(pub, ifidx);
}
