/*
 * Linux cfg80211 driver
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
 * $Id: wl_core.h $
 */

/*
 * Older Linux versions support the 'iw' interface, more recent ones the 'cfg80211' interface.
 */

#ifndef _wl_core_h_
#define _wl_core_h_

#if defined(WL_CFG80211)
#include <wl_cfg80211.h>
#endif // endif
#if defined(BCMDONGLEHOST)
#include <dhd.h>
#else
#include <wl_linux.h>
#endif /* BCMDONGLEHOST */
#define DEFAULT_IOCTL_RESP_TIMEOUT      2000
#ifndef IOCTL_RESP_TIMEOUT
#ifdef BCMQT
#define IOCTL_RESP_TIMEOUT  30000 /* In milli second */
#else
/* In milli second default value for Production FW */
#define IOCTL_RESP_TIMEOUT  DEFAULT_IOCTL_RESP_TIMEOUT
#endif /* BCMQT */
#endif /* IOCTL_RESP_TIMEOUT */

#ifndef WL_MAX_IFS
#ifdef DHD_MAX_IFS
#define WL_MAX_IFS DHD_MAX_IFS
#endif /* DHD_MAX_IFS */
#endif /* WL_MAX_IFS */

#ifndef WL_MAX_IFS
#error "WL_MAX_IFS not defined!"
#endif // endif

#ifndef MFG_IOCTL_RESP_TIMEOUT
#define MFG_IOCTL_RESP_TIMEOUT  20000  /* In milli second default value for MFG FW */
#endif /* MFG_IOCTL_RESP_TIMEOUT */
struct wl_core_priv {
#ifdef PNO_SUPPORT
	int pno_enable;                 /* pno status : "1" is pno enable */
	int pno_suspend;                /* pno suspend status : "1" is pno suspended */
	void *pno_state;
#endif /* PNO_SUPPORT */
	char * pktfilter[100];
	int pktfilter_count;
	int     op_mode;                                /* STA, HostAPD, WFD, SoftAP */
#ifdef WLTDLS
	bool tdls_enable;
	uint32 tdls_mode;
#endif // endif
#ifdef CUSTOM_SET_CPUCORE
	int chan_isvht80;
#endif // endif
#if defined(ARP_OFFLOAD_SUPPORT)
	uint32 arp_version;
#endif // endif
#if defined(PKT_FILTER_SUPPORT)
	int dhcp_in_progress;   /* DHCP period */
	int early_suspended;    /* Early suspend status */
#endif // endif
#define WLC_IOCTL_MAXBUF_FWCAP  1024
	char  fw_capabilities[WLC_IOCTL_MAXBUF_FWCAP];
};

int wl_core_deinit(void *pub);
void wl_core_init(void *pub);
osl_t * wl_get_pub_osh(void *pub);
enum dhd_op_flags {
	/* Firmware requested operation mode */
	DHD_FLAG_STA_MODE                               = (1 << (0)), /* STA only */
	DHD_FLAG_HOSTAP_MODE                            = (1 << (1)), /* SOFTAP only */
	DHD_FLAG_P2P_MODE                               = (1 << (2)), /* P2P Only */
	/* STA + P2P */
	DHD_FLAG_CONCURR_SINGLE_CHAN_MODE = (DHD_FLAG_STA_MODE | DHD_FLAG_P2P_MODE),
	DHD_FLAG_CONCURR_MULTI_CHAN_MODE                = (1 << (4)), /* STA + P2P */
	/* Current P2P mode for P2P connection */
	DHD_FLAG_P2P_GC_MODE                            = (1 << (5)),
	DHD_FLAG_P2P_GO_MODE                            = (1 << (6)),
	DHD_FLAG_MBSS_MODE                              = (1 << (7)), /* MBSS in future */
	DHD_FLAG_IBSS_MODE                              = (1 << (8)),
	DHD_FLAG_MFG_MODE                               = (1 << (9)),
	DHD_FLAG_RSDB_MODE                              = (1 << (10)),
	DHD_FLAG_MP2P_MODE                              = (1 << (11))
};
#ifdef WLTDLS
#ifndef CUSTOM_TDLS_IDLE_MODE_SETTING
#define CUSTOM_TDLS_IDLE_MODE_SETTING  60000 /* 60sec to tear down TDLS of not active */
#endif // endif
#ifndef CUSTOM_TDLS_RSSI_THRESHOLD_HIGH
#define CUSTOM_TDLS_RSSI_THRESHOLD_HIGH -70 /* rssi threshold for establishing TDLS link */
#endif // endif
#ifndef CUSTOM_TDLS_RSSI_THRESHOLD_LOW
#define CUSTOM_TDLS_RSSI_THRESHOLD_LOW -80 /* rssi threshold for tearing down TDLS link */
#endif // endif
#endif /* WLTDLS */
#define DHD_IOVAR_BUF_SIZE      128

#ifdef PKT_FILTER_SUPPORT
#define WL_ARP_FILTER_NUM              5
#endif /* PKT_FILTER_SUPPORT */
bool wl_is_associated(struct net_device *dev, int *retval);
int wl_register_interface(void *pub, int ifidx, struct net_device *dev, bool rtnl_is_needed);
bool _turn_on_arp_filter(struct wl_core_priv *wlcore, int op_mode);
void wl_netdev_free(struct net_device *ndev);
int wl_get_fw_mode(void *drv_pub);
bool wl_support_sta_mode(struct wl_core_priv *core_priv);
#ifdef WLTDLS
int _wl_tdls_enable(struct net_device *net, struct wl_core_priv *wlcore,
		bool tdls_on, bool auto_on, struct ether_addr *mac);
#endif // endif
#define FW_SUPPORTED(wlcore, capa) ((strstr(wlcore->fw_capabilities, " " #capa " ") != NULL))
#if defined(BCMDONGLEHOST)
#define WL_CORE_OPMODE_SUPPORTED(wlcore, opmode_flag) \
	(wlcore ? ((((struct wl_core_priv *)wlcore)->op_mode) & opmode_flag): -1)
#define WL_OPMODE_SUPPORTED(cfg, opmode_flag) \
	(cfg ? WL_CORE_OPMODE_SUPPORTED((((struct bcm_cfg80211 *)cfg)->wlcore), opmode_flag) : -1)
#else
#define WL_OPMODE_SUPPORTED(cfg, opmode_flag)  -1
#endif /* defined (BCMDONGLEHOST) */
#endif /* _wl_core_h_ */
