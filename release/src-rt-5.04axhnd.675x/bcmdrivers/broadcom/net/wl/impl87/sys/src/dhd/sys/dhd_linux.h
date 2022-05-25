/*
 * DHD Linux header file (dhd_linux exports for cfg80211 and other components)
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
 * $Id: dhd_linux.h 399301 2013-04-29 21:41:52Z $
 */

/* wifi platform functions for power, interrupt and pre-alloc, either
 * from Android-like platform device data, or Broadcom wifi platform
 * device data.
 *
 */
#ifndef __DHD_LINUX_H__
#define __DHD_LINUX_H__

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <dngl_stats.h>
#include <dhd.h>
#ifdef DHD_WMF
#include <dhd_wmf_linux.h>
#endif
/* Linux wireless extension support */
#if defined(WL_WIRELESS_EXT)
#include <wl_iw.h>
#endif /* defined(WL_WIRELESS_EXT) */
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif /* defined(CONFIG_HAS_EARLYSUSPEND) && defined(DHD_USE_EARLYSUSPEND) */

#ifdef BCMDBUS
#include <dbus.h>
#define BUS_IOVAR_OP(a, b, c, d, e, f, g) dbus_iovar_op(a->dbus, b, c, d, e, f, g)
#else
#include <dhd_bus.h>
#define BUS_IOVAR_OP dhd_bus_iovar_op
#endif

#define DHD_REGISTRATION_TIMEOUT  12000  /* msec : allowed time to finished dhd registration */

typedef struct wifi_adapter_info {
	const char	*name;
	uint		irq_num;
	uint		intr_flags;
	const char	*fw_path;
	const char	*nv_path;
	void		*wifi_plat_data;	/* wifi ctrl func, for backward compatibility */
	uint		bus_type;
	uint		bus_num;
	uint		slot_num;
} wifi_adapter_info_t;

typedef struct bcmdhd_wifi_platdata {
	uint				num_adapters;
	wifi_adapter_info_t	*adapters;
} bcmdhd_wifi_platdata_t;

/** Per STA params. A list of dhd_sta objects are managed in dhd_if */
typedef struct dhd_sta {
	cumm_ctr_t cumm_ctr;    /* cummulative queue length of child flowrings */
	uint16 flowid[NUMPRIO]; /* allocated flow ring ids (by priority) */
	void * ifp;             /* associated dhd_if */
	struct ether_addr ea;   /* stations ethernet mac address */
	struct list_head list;  /* link into dhd_if::sta_list */
	int idx;                /* index of self in dhd_pub::sta_pool[] */
	int ifidx;              /* index of interface in dhd */
#ifdef DHD_WMF
	struct dhd_sta *psta_prim; /* primary index of psta interface */
#endif /* DHD_WMF */
#ifdef BCM_NBUFF_WLMCAST
	uint32 src_ip;             /* sta's ip address, which is used for emf */
#endif
} dhd_sta_t;
typedef dhd_sta_t dhd_sta_pool_t;

int dhd_wifi_platform_register_drv(void);
void dhd_wifi_platform_unregister_drv(void);
wifi_adapter_info_t* dhd_wifi_platform_get_adapter(uint32 bus_type, uint32 bus_num,
	uint32 slot_num);
int wifi_platform_set_power(wifi_adapter_info_t *adapter, bool on, unsigned long msec);
int wifi_platform_bus_enumerate(wifi_adapter_info_t *adapter, bool device_present);
int wifi_platform_get_irq_number(wifi_adapter_info_t *adapter, unsigned long *irq_flags_ptr);
int wifi_platform_get_mac_addr(wifi_adapter_info_t *adapter, unsigned char *buf);
void *wifi_platform_get_country_code(wifi_adapter_info_t *adapter, char *ccode);
void* wifi_platform_prealloc(wifi_adapter_info_t *adapter, int section, unsigned long size);
void* wifi_platform_get_prealloc_func_ptr(wifi_adapter_info_t *adapter);

int dhd_get_fw_mode(struct dhd_info *dhdinfo);
bool dhd_update_fw_nv_path(struct dhd_info *dhdinfo);

#ifdef BCM_ROUTER_DHD
void dhd_update_dpsta_interface_for_sta(dhd_pub_t* dhdp, int ifidx, void* event_data);
extern void dhd_if_inc_txpkt_cnt(dhd_pub_t *dhdp, int ifidx, void *pkt);
extern void dhd_if_inc_rxpkt_cnt(dhd_pub_t *dhdp, int ifidx, uint32 pktlen);
extern void dhd_if_inc_txpkt_drop_cnt(dhd_pub_t *dhdp, int ifidx);
extern void dhd_if_add_txpkt_drop_cnt(dhd_pub_t *dhdp, int ifidx, int cnt);
#endif /* BCM_ROUTER_DHD */

extern uint8* dhd_if_get_macaddr(dhd_pub_t *dhdp, int ifidx);

#ifdef DHD_WMF
dhd_wmf_t* dhd_wmf_conf(dhd_pub_t *dhdp, uint32 idx);
int dhd_get_wmf_psta_disable(dhd_pub_t *dhdp, uint32 idx);
int dhd_set_wmf_psta_disable(dhd_pub_t *dhdp, uint32 idx, int val);
void dhd_update_psta_interface_for_sta(dhd_pub_t *dhdp, char* ifname,
	void* mac_addr, void* event_data);
#if defined(BCM_NBUFF_WLMCAST_IPV6)
char* dhd_wmf_ifname(dhd_pub_t *dhdp, uint32 idx);
#endif
#endif /* DHD_WMF */

#if defined(STB) && !defined(STBAP)
void dhd_set_wowl(dhd_pub_t *dhdp, int state);
#endif /* STB && STBAP */

void dhd_set_monitor(dhd_pub_t *dhd, int ifidx, int val);

#if defined(BCM_DHD_RUNNER)
uint16 dhd_get_sta_cnt(void *pub, int ifidx, void *ea);
#endif /* BCM_DHD_RUNNER */

#ifdef BCM_NBUFF_WLMCAST
int dhd_client_get_info(struct net_device *dev, char *mac, int priority,
	wlan_client_info_t *info_p);
#endif

#ifdef BCM_GMAC3
extern int dhd_set_1905_almac(dhd_pub_t *dhdp, uint8 ifidx, uint8* ea, bool mcast);
extern int dhd_get_1905_almac(dhd_pub_t *dhdp, uint8 ifidx, uint8* ea, bool mcast);
#endif /* BCM_GMAC3 */

#if defined(BCMDONGLEHOST)
int dhd_locked_wait_pend8021x(struct net_device *dev);
#endif /* BCMDONGLEHOST */

#ifdef BCM_PKTFWD

/* dhd interface sturcture is restricted to dhd_linux.c
 * define a private staructure for pktfwd susbsytem
 */
typedef struct dhd_pktfwd_priv
{
	struct d3fwd_wlif * d3fwd_wlif;
	void		* ifp;
	uint32_t	radio_unit;
	uint32_t	ifidx;
} dhd_pktfwd_priv_t;

extern dhd_pktfwd_priv_t * dhd_pktfwd_get_priv(struct net_device * dev);
extern void dhd_wlan_set_dwds_client(dhd_pub_t *dhdp, uint8 ifidx, bool set);
#endif /* BCM_PKTFWD */

#endif /* __DHD_LINUX_H__ */
