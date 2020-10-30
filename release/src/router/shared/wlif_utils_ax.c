/*
 * Wireless interface translation utility functions
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
 * $Id: wlif_utils.c 790839 2020-09-04 11:22:55Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
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
#include <common_utils.h>
#include <wlif_utils.h>

#include <stdarg.h>   // for va_list
#include "shared.h"

#ifdef BCA_HNDROUTER
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <board.h>
#endif /* BCA_HNDROUTER */

#ifndef MAX_NVPARSE
#define MAX_NVPARSE 16
#endif

#define WL_WLIF_BUFSIZE_4K	4096
#define WL_MACMODE_MASK		0x6000
#define WL_MACPROBE_MASK	0x1000
#define WL_MACMODE_SHIFT	13
#define WL_MACPROBE_SHIFT	12
/* Static info set */
#define WL_SINFOSET_SHIFT	15

#define WL_MAC_PROB_MODE_SET(m, p, f)	((m << WL_MACMODE_SHIFT) | (p << WL_MACPROBE_SHIFT) \
		| (f << WL_SINFOSET_SHIFT))
#define	WL_MACMODE_GET(f)	(f & WL_MACMODE_MASK >> WL_MACMODE_SHIFT)
#define	WL_MACPROBE_GET(f)	(f & WL_MACPROBE_MASK >> WL_MACPROBE_SHIFT)
#define TIMER_MAX_COUNT	16
#define WL_MAC_ADD	1
#define WL_MAC_DEL	0

#define SEC_TO_MICROSEC(x)	((x) * 1000 * 1000)

#ifdef BCMDBG
#define WLIF_BSSTRANS_DBG	1	/* Turn on the debug */
#else
#define WLIF_BSSTRANS_DBG	0	/* Turn off the debug */
#endif /* BCMDBG */

#if WLIF_BSSTRANS_DBG
#define WLIF_BSSTRANS(fmt, args...) printf("WLIF_BSSTRANS >> %s: " fmt, __FUNCTION__, ##args)
#else
#define WLIF_BSSTRANS(fmt, args...)
#endif /* WLIF_BSSTRANS_DBG */

#ifdef RTCONFIG_BRCM_HOSTAPD
#define WPA_CLI_APP "wpa_cli-2.7"
#else
#define WPA_CLI_APP "wpa_cli"
#endif

/* From wlc_rate.[ch] */
#define MCS_TABLE_SIZE				33

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#endif /* ARRAYSIZE */

#define WLIF_MIN_BUF           128
#define WLIF_DUMP_BUF_LEN      8 * 1024
#define WLIF_MAP_BHSTA_NVVAL   0x4
#define WLIF_SCAN_TRY_COUNT	5

/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
/* The below macros handle endian mis-matches between wl utility and wl driver. */
bool g_swap = FALSE;
#define htod64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define htod16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define dtoh64(i) (g_swap?bcmswap64(i):(uint64)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh16(i) (g_swap?bcmswap16(i):(uint16)(i))
#define htodchanspec(i) (g_swap?htod16(i):i)
#define dtohchanspec(i) (g_swap?dtoh16(i):i)
#define htodenum(i) (g_swap?((sizeof(i) == 4) ? htod32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)
#define dtohenum(i) (g_swap?((sizeof(i) == 4) ? dtoh32(i) : ((sizeof(i) == 2) ? htod16(i) : i)):i)

/* From wlc_rate.[ch] */
#define MCS_TABLE_SIZE         33

/* nvram lan_ifname/wan_ifname cache */
static int nv_first = 0;
char nv_lan_ifnames[WLIFU_MAX_NO_BRIDGE][256];
char nv_lan_ifname[WLIFU_MAX_NO_BRIDGE][64];
char nv_wan_ifnames[64];
char nv_wan_ifname[64];

/*
 * Flag Bit Values
 * 0-0	: Static info set
 * 1-1	: Probe Mode
 * 2-3	: Mac mode
 * 4-7	: Reserved
 * 8-15 : Static mac list count
 */
typedef struct static_maclist_ {
	char ifname[IFNAMSIZ];
	short flag;
	struct ether_addr addr;
} static_maclist_t;

/* wireless interface name descriptors */
typedef struct _wlif_name_desc {
        char            *name;          /* wlif name */
        bool            wds;            /* wds interface */
        bool            subunit;                /* subunit existance */
} wlif_name_desc_t;

wlif_name_desc_t wlif_name_array[] = {
/*        name  wds             subunit */
/* PARIMARY */
#if defined(linux)
        { "eth",        0,              0}, /* primary */
#else
        { "wl", 0,              0}, /* primary */
#endif

/* MBSS */
        { "wl", 0,              1}, /* mbss */

/* WDS */
        { "wds",        1,              1} /* wds */
};

#ifdef RTCONFIG_BCM_7114
/* Counter to match the response for BSS transition request */
static uint8 bss_token = 0;

static int wl_wlif_unblock_mac_cb(bcm_timer_id timer, static_maclist_t *data);
static int wl_wlif_unblock_mac_usched_cb(bcm_usched_handle* hdl, void* arg);
static int wl_wlif_send_bss_transreq(char *ifname, uint8 rclass, chanspec_t chanspec, struct ether_addr bssid, struct ether_addr addr, int event_fd);
#endif

/*
 * Translate virtual interface mac to spoof mac
 * Rule:
 *              00:aa:bb:cc:dd:ee                                              00:00:00:x:y:z
 *              wl0 ------------ [wlx/wlx.y/wdsx.y]0.1 ------ x=1/2/3, y=0, z=1
 *                   +----------- [wlx/wlx.y/wdsx.y]0.2 ------ x=1/2/3, y=0, z=2
 *              wl1 ------------ [wlx/wlx.y/wdsx.y]1.1 ------ x=1/2/3, y=1, z=1
 *                   +----------- [wlx/wlx.y/wdsx.y]1.2 ------ x=1/2/3, y=1, z=2
 *
 *              URE ON  : wds/mbss not support and wlx.y have same mac as wlx
 *              URE OFF : wlx.y have unique mac and wdsx.y have same mac as wlx
 *
 */
int
get_spoof_mac(const char *osifname, char *mac, int maclen)
{
        char nvifname[16];
        int i, unit, subunit;
        wlif_name_desc_t *wlif_name;

        if (osifname == NULL ||
                mac == NULL ||
                maclen < ETHER_ADDR_LEN)
                return -1;
        if (osifname_to_nvifname(osifname, nvifname, sizeof(nvifname)) < 0)
                return -1;

        /* translate to spoof mac */
        if (!get_ifname_unit(nvifname, &unit, &subunit)) {
                memset(mac, 0, maclen);
                for (i = 0; i < ARRAYSIZE(wlif_name_array); i++) {
                        wlif_name = &wlif_name_array[i];
                        if (!strncmp(osifname, wlif_name->name, strlen(wlif_name->name))) {
                                if (subunit >= 0 && wlif_name->subunit)
                                        break;
                                else if (subunit < 0 && !wlif_name->subunit) {
                                        subunit = 0; /* reset to zero */
                                        break;
                                }
                        }
                }

                /* not found */
                if (i == ARRAYSIZE(wlif_name_array))
                        return -1;

                /* translate it */
                mac[3] = i+1;
                mac[4] = unit;
                mac[5] = subunit;

                return 0;
        }
        return -1;
}

int
get_spoof_ifname(char *mac, char *osifname, int osifnamelen)
{
        int idx, unit, subunit;
        char nvifname[16];
        wlif_name_desc_t *wlif_name;

        if (osifname == NULL ||
                mac == NULL)
                return -1;

        if (mac[0] != 0 || mac[1] != 0 ||
                mac[2] != 0)
                return -1; /* is a real mac, fast check */

        idx = mac[3];
        idx --; /* map to wlif_name_array index */
        unit = mac[4];
        subunit = mac[5];
        if (idx < 0 || idx >= ARRAYSIZE(wlif_name_array))
                return -1;

        /* get nvname format */
        wlif_name = &wlif_name_array[idx];
        if (wlif_name->subunit)
                snprintf(nvifname, sizeof(nvifname), "%s%d.%d", (wlif_name->wds) ? "wds" : "wl",
                              unit, subunit);
        else
                snprintf(nvifname, sizeof(nvifname), "wl%d", unit);

        /* translate to osifname */
        if (nvifname_to_osifname(nvifname, osifname, osifnamelen) < 0)
                return -1;

        return 0;
}

int
get_real_mac(char *mac, int maclen)
{
        int idx, unit, subunit;
        char *ptr, ifname[32];
        wlif_name_desc_t *wlif_name;

        if (mac == NULL ||
            maclen < ETHER_ADDR_LEN)
                return -1;

        if (mac[0] != 0 || mac[1] != 0 ||
                mac[2] != 0)
                return 0; /* is a real mac, fast path */

        idx = mac[3];
        idx --; /* map to wlif_name_array index */
        unit = mac[4];
        subunit = mac[5];
        if (idx < 0 || idx >= ARRAYSIZE(wlif_name_array))
                return -1;

        /* get wlx.y mac addr */
        wlif_name = &wlif_name_array[idx];
        if (wlif_name->subunit && !wlif_name->wds)
                snprintf(ifname, sizeof(ifname), "wl%d.%d_hwaddr", unit, subunit);
        else
                snprintf(ifname, sizeof(ifname), "wl%d_hwaddr", unit);

        ptr = nvram_get(ifname);
        if (ptr == NULL)
                return -1;

        ether_atoe(ptr, (unsigned char *) mac);
        return 0;
}

unsigned char *
get_wlmacstr_by_unit(char *unit)
{
        char tmptr[] = "wlXXXXX_hwaddr";
        char *macaddr;

        sprintf(tmptr, "wl%s_hwaddr", unit);

        macaddr = nvram_get(tmptr);

        if (!macaddr)
                return NULL;

        return (unsigned char *) macaddr;
}

int
get_lan_mac(unsigned char *mac)
{
        char *lanmac_str = nvram_get("lan_hwaddr");

        if (mac)
                memset(mac, 0, 6);

        if (!lanmac_str || mac == NULL)
                return -1;

        ether_atoe(lanmac_str, mac);

        return 0;
}

int
get_wlname_by_mac(unsigned char *mac, char *wlname)
{
	char eabuf[18];
	char tmptr[] = "wlXXXXX_hwaddr";
	char bss_en[] = "wlXXX_bss_enabled";
	char *wl_hw;
	int i, j;

	ether_etoa(mac, eabuf);
	/* find out the wl name from mac */
	for (i = 0; i < MAX_NVPARSE; i++) {
		sprintf(wlname, "wl%d", i);
		sprintf(tmptr, "wl%d_hwaddr", i);
		sprintf(bss_en, "wl%d_bss_enabled", i);
		wl_hw = nvram_get(tmptr);
		if (wl_hw) {
			if (!strncasecmp(wl_hw, eabuf, sizeof(eabuf)) &&
				nvram_match(bss_en, "1"))
				return 0;
		}

		for (j = 1; j < WL_MAXBSSCFG; j++) {
			sprintf(wlname, "wl%d.%d", i, j);
			sprintf(tmptr, "wl%d.%d_hwaddr", i, j);
			sprintf(bss_en, "wl%d.%d_bss_enabled", i, j);
			wl_hw = nvram_get(tmptr);
			if (wl_hw) {
				if (!strncasecmp(wl_hw, eabuf, sizeof(eabuf)) &&
					nvram_match(bss_en, "1"))
					return 0;
			}
		}
	}

	return -1;
}

bool
wl_wlif_is_wet_ap(char *ifname)
{
	int wet = 0, ap = 0;

#ifdef __CONFIG_DHDAP__
	if (!dhd_probe(ifname)) {
		wl_iovar_getint(ifname, "wet_enab", &wet);
	} else
#endif /* __CONFIG_DHDAP__ */
	{
		wl_iovar_getint(ifname, "wet", &wet);
	}

	if (wl_probe(ifname) < 0)
		return FALSE;

	wl_iovar_getint(ifname, "ap", &ap);

	return (wet && ap);
}

void get_ifname_by_wlmac_prep(void)
{
	int i;
	char if_name[16], if_names[16];

	memset(nv_lan_ifnames, 0, sizeof(nv_lan_ifnames));
	memset(nv_lan_ifname, 0, sizeof(nv_lan_ifname));
	memset(nv_wan_ifnames, 0, sizeof(nv_wan_ifnames));
	memset(nv_wan_ifname, 0, sizeof(nv_wan_ifname));

	for (i = 0; i < WLIFU_MAX_NO_BRIDGE; i++) {
		if (i == 0 ) {
			sprintf(if_names, "lan_ifnames");
			sprintf(if_name, "lan_ifname");
		}
		else {
			sprintf(if_names, "lan%d_ifnames", i);
			sprintf(if_name, "lan%d_ifname", i);
		}

		strncpy(nv_lan_ifnames[i], nvram_safe_get(if_names), 256);
		strncpy(nv_lan_ifname[i], nvram_safe_get(if_name), 64);
	}
	strncpy(nv_wan_ifnames, nvram_safe_get("wan_ifnames"), 64);
	strncpy(nv_wan_ifname, nvram_safe_get("wan0_ifname"), 64);
	return;
}


bool
wl_wlif_is_psta(char *ifname)
{
#ifndef PSTA
	/* PSTA not defined */
	return FALSE;
#else
	int32 psta = FALSE;

	if (wl_probe(ifname) < 0)
		return FALSE;

	if (wl_iovar_getint(ifname, "psta_if", &psta) < 0)
		return FALSE;

	return psta ? TRUE : FALSE;
#endif /* !PSTA */
}

bool
wl_wlif_is_dwds(char *ifname)
{
#ifdef RTCONFIG_BCMARM
	int32 wds_type = FALSE;

	if (wl_probe(ifname) < 0)
		return FALSE;

	return (!wl_iovar_getint(ifname, "wds_type", &wds_type) && wds_type == WL_WDSIFTYPE_DWDS);
#else
	return FALSE;
#endif
}

bool
wl_wlif_is_psr_ap(char *ifname)
{
	int32 psta = FALSE;
	int32 psta_mode = 0;

	if (wl_probe(ifname) < 0)
		return FALSE;

	wl_iovar_getint(ifname, "psta", &psta_mode);
	if (psta_mode == 2) {
		wl_iovar_getint(ifname, "psta_if", &psta);
		if (!psta) {
			return strncmp(ifname, "wl", 2) ? FALSE : TRUE;
		}
	} else
		return FALSE;

	return FALSE;
}

/*
 * Get LAN or WAN ifname by wl mac
 * NOTE: We pass ifname in case of same mac in vifs (like URE TR mode)
 */
char *
get_ifname_by_wlmac(unsigned char *mac, char *name)
{
	char nv_name[16], os_name[16];
	static char if_name[16];
	char tmptr[] = "lanXX_ifnames";
	char *ifnames, *ifname;
	int i;

	if (!nv_first) {
		nv_first = 1;
		get_ifname_by_wlmac_prep();
	}

	/*
	  * In case of URE mode, wl0.1 and wl0 have same mac,
	  * we need extra identity (name).
	  */
	if (name && !strncmp(name, "wl", 2))
		snprintf(nv_name, sizeof(nv_name), "%s", name);
	else if (get_wlname_by_mac(mac, nv_name))
		return 0;

	if (nvifname_to_osifname(nv_name, os_name, sizeof(os_name)) < 0)
		return 0;

	if (osifname_to_nvifname(os_name, nv_name, sizeof(nv_name)) < 0)
		return 0;

	/* find for dpsta */
	if (wl_wlif_is_psta(os_name))
		return name;

	/* find LAN ifname for wlmac which is in dpsta */
	ifnames = nvram_get("dpsta_ifnames");
	if (ifnames && (find_in_list(ifnames, nv_name) || find_in_list(ifnames, os_name))) {
		/* find dpsta in which bridge */
		for (i = 0; i < WLIFU_MAX_NO_BRIDGE; i++) {
			sprintf(tmptr, "br%d_ifnames", i);
			sprintf(if_name, "br%d", i);
			ifnames = nvram_get(tmptr);
			if (!ifnames && !i)
			ifnames = nvram_get("lan_ifnames");
			ifname = if_name;

			if (ifnames) {
				/* the name in ifnames may nvifname or osifname */
				if (find_in_list(ifnames, nv_name) ||
					find_in_list(ifnames, os_name))
					return ifname;
			}
		}
	}

	/* find for lan */
	for (i = 0; i < WLIFU_MAX_NO_BRIDGE; i++) {

		ifnames = nv_lan_ifnames[i];
		ifname = nv_lan_ifname[i];
		if (ifname[0]) {
			/* the name in ifnames may nvifname or osifname */
			if (find_in_list(ifnames, nv_name) ||
				find_in_list(ifnames, os_name))
				return ifname;
		}
#if 0
		if (i == 0) {
			ifnames = nvram_get("lan_ifnames");
			ifname = nvram_get("lan_ifname");
			if (ifname) {
				/* the name in ifnames may nvifname or osifname */
				if (find_in_list(ifnames, nv_name) ||
				    find_in_list(ifnames, os_name))
					return ifname;
			}
		}
		else {
			sprintf(if_name, "lan%d_ifnames", i);
			sprintf(tmptr, "lan%d_ifname", i);
			ifnames = nvram_get(if_name);
			ifname = nvram_get(tmptr);
			if (ifname) {
				/* the name in ifnames may nvifname or osifname */
				if (find_in_list(ifnames, nv_name) ||
				    find_in_list(ifnames, os_name))
					return ifname;
			}
		}
#endif
	}

	/* find for wan  */
	ifnames = nv_wan_ifnames;
	ifname = nv_wan_ifname;
#if 0
	ifnames = nvram_get("wan_ifnames");
	ifname = nvram_get("wan0_ifname");
#endif
	/* the name in ifnames may nvifname or osifname */
	if (find_in_list(ifnames, nv_name) ||
	    find_in_list(ifnames, os_name))
		return ifname;

	return 0;
}

#if defined(HND_ROUTER)
#define CHECK_NAS(mode) ((mode) & (WPA_AUTH_UNSPECIFIED | WPA_AUTH_PSK | \
			 	   WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_PSK | WPA2_AUTH_FT))
#define CHECK_PSK(mode) ((mode) & (WPA_AUTH_PSK | WPA2_AUTH_PSK | WPA2_AUTH_FT))
#else
#define CHECK_NAS(mode) ((mode) & (WPA_AUTH_UNSPECIFIED | WPA_AUTH_PSK | \
				   WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_PSK))
#define CHECK_PSK(mode) ((mode) & (WPA_AUTH_PSK | WPA2_AUTH_PSK))

#endif
#define CHECK_RADIUS(mode) ((mode) & (WPA_AUTH_UNSPECIFIED | WLIFU_AUTH_RADIUS | \
				      WPA2_AUTH_UNSPECIFIED))

/* Get wireless security setting by interface name */
int
get_wsec(wsec_info_t *info, unsigned char *mac, char *osifname)
{
	int i, unit, wds = 0, wds_wsec = 0;
	char nv_name[16], os_name[16], wl_prefix[16], comb[32], key[8];
	char wds_role[8], wds_ssid[48], wds_psk[80], wds_akms[16], wds_crypto[16],
	        remote[ETHER_ADDR_LEN];
	char akm[16], *akms, *akmnext, *value, *infra;

	if (info == NULL || mac == NULL)
		return WLIFU_ERR_INVALID_PARAMETER;

	if (nvifname_to_osifname(osifname, os_name, sizeof(os_name))) {
		if (get_wlname_by_mac(mac, nv_name))
			return WLIFU_ERR_INVALID_PARAMETER;
		else if (nvifname_to_osifname(nv_name, os_name, sizeof(os_name)))
			return WLIFU_ERR_INVALID_PARAMETER;
	}
	else if (osifname_to_nvifname(os_name, nv_name, sizeof(nv_name)))
			return WLIFU_ERR_INVALID_PARAMETER;

	/* check if i/f exists and retrieve the i/f index */
	if (wl_probe(os_name) ||
		wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
		return WLIFU_ERR_NOT_WL_INTERFACE;

	/* get wl_prefix.
	 *
	 * Due to DWDS and WDS may be enabled at the same time,
	 * checking whether this is WDS interface in order to
	 * get per WDS interface security settings from NVRAM.
	 */
	if (strstr(os_name, "wds") && (wl_wlif_is_dwds(os_name) == FALSE)) {
		/* the wireless interface must be configured to run NAS */
		snprintf(wl_prefix, sizeof(wl_prefix), "wl%d", unit);
		wds = 1;
	}
	else if (wl_wlif_is_psta(os_name))
		snprintf(wl_prefix, sizeof(wl_prefix), "wl%d", unit);
	else if (osifname_to_nvifname(os_name, wl_prefix, sizeof(wl_prefix)))
		return WLIFU_ERR_INVALID_PARAMETER;

	strcat(wl_prefix, "_");
	memset(info, 0, sizeof(wsec_info_t));

	/* get wds setting */
	if (wds) {
		/* remote address */
		if (wl_ioctl(os_name, WLC_WDS_GET_REMOTE_HWADDR, remote, ETHER_ADDR_LEN))
			return WLIFU_ERR_WL_REMOTE_HWADDR;
		memcpy(info->remote, remote, ETHER_ADDR_LEN);

		/* get per wds settings */
		for (i = 0; i < MAX_NVPARSE; i ++) {
			char macaddr[18];
			uint8 ea[ETHER_ADDR_LEN];

			if (get_wds_wsec(unit, i, macaddr, wds_role, wds_crypto, wds_akms, wds_ssid,
			                 wds_psk) &&
			    ((ether_atoe(macaddr, ea) && !bcmp(ea, remote, ETHER_ADDR_LEN)) ||
			     ((mac[0] == '*') && (mac[1] == '\0')))) {
			     /* found wds settings */
			     wds_wsec = 1;
			     break;
			}
		}
	}

	/* interface unit */
	info->unit = unit;
	/* interface os name */
	strcpy(info->osifname, os_name);
	/* interface address */
	memcpy(info->ea, mac, ETHER_ADDR_LEN);
	/* ssid */
	if (wds && wds_wsec)
		strncpy(info->ssid, wds_ssid, MAX_SSID_LEN);
	else {
		value = nvram_safe_get(strcat_r(wl_prefix, "ssid", comb));
		strncpy(info->ssid, value, MAX_SSID_LEN);
	}
	/* auth */
	if (nvram_match(strcat_r(wl_prefix, "auth", comb), "1"))
		info->auth = 1;
	/* nas auth mode */
	value = nvram_safe_get(strcat_r(wl_prefix, "auth_mode", comb));
	info->akm = !strcmp(value, "radius") ? WLIFU_AUTH_RADIUS : 0;
	if (wds && wds_wsec)
		akms = wds_akms;
	else
		akms = nvram_safe_get(strcat_r(wl_prefix, "akm", comb));
	foreach(akm, akms, akmnext) {
		if (!strcmp(akm, "wpa"))
			info->akm |= WPA_AUTH_UNSPECIFIED;
		if (!strcmp(akm, "psk"))
			info->akm |= WPA_AUTH_PSK;
		if (!strcmp(akm, "wpa2"))
			info->akm |= WPA2_AUTH_UNSPECIFIED;
		if (!strcmp(akm, "psk2"))
			info->akm |= WPA2_AUTH_PSK;
#ifdef HND_ROUTER
		if (!strcmp(akm, "psk2ft"))
			info->akm |= WPA2_AUTH_PSK | WPA2_AUTH_FT;
#endif
	}
	/* wsec encryption */
	value = nvram_safe_get(strcat_r(wl_prefix, "wep", comb));
	info->wsec = !strcmp(value, "enabled") ? WEP_ENABLED : 0;
	if (wds && wds_wsec)
		value = wds_crypto;
	else
		value = nvram_safe_get(strcat_r(wl_prefix, "crypto", comb));
	if (CHECK_NAS(info->akm)) {
		if (!strcmp(value, "tkip"))
			info->wsec |= TKIP_ENABLED;
		else if (!strcmp(value, "aes"))
			info->wsec |= AES_ENABLED;
		else if (!strcmp(value, "tkip+aes"))
			info->wsec |= TKIP_ENABLED|AES_ENABLED;
	}
	/* nas role setting, may overwrite later in wds case */
	value = nvram_safe_get(strcat_r(wl_prefix, "mode", comb));
	infra = nvram_safe_get(strcat_r(wl_prefix, "infra", comb));
	if (!strcmp(value, "ap")) {
		info->flags |= WLIFU_WSEC_AUTH;
	}
	else if (!strcmp(value, "sta") || !strcmp(value, "wet") ||
	         !strcmp(value, "psr") || !strcmp(value, "psta")) {
		if (!strcmp(infra, "0")) {
			/* IBSS, so we must act as Authenticator and Supplicant */
			info->flags |= WLIFU_WSEC_AUTH;
			info->flags |= WLIFU_WSEC_SUPPL;
			/* Adhoc Mode */
			info->ibss = TRUE;
		}
		else {
			info->flags |= WLIFU_WSEC_SUPPL;
		}
	}
	else if (!strcmp(value, "wds")) {
		;
	}
	else {
		/* Unsupported network mode */
		return WLIFU_ERR_NOT_SUPPORT_MODE;
	}
	/* overwrite flags */
	if (wds) {
		char buf[32];
		unsigned char *ptr, lrole;

		/* did not find WDS link configuration, use wireless' */
		if (!wds_wsec)
			strcpy(wds_role, "auto");

		/* get right role */
		if (!strcmp(wds_role, "sup"))
			lrole = WL_WDS_WPA_ROLE_SUP;
		else if (!strcmp(wds_role, "auth"))
			lrole = WL_WDS_WPA_ROLE_AUTH;
		else /* if (!strcmp(wds_role, "auto")) */
			lrole = WL_WDS_WPA_ROLE_AUTO;

		strcpy(buf, "wds_wpa_role");
		ptr = (unsigned char *)buf + strlen(buf) + 1;
		bcopy(info->remote, ptr, ETHER_ADDR_LEN);
		ptr[ETHER_ADDR_LEN] = lrole;
		if (wl_ioctl(os_name, WLC_SET_VAR, buf, sizeof(buf)))
			return WLIFU_ERR_WL_WPA_ROLE;
		else if (wl_ioctl(os_name, WLC_GET_VAR, buf, sizeof(buf)))
			return WLIFU_ERR_WL_WPA_ROLE;
		lrole = *buf;

		/* overwrite these flags */
		info->flags = WLIFU_WSEC_WDS;
		if (lrole == WL_WDS_WPA_ROLE_SUP) {
			info->flags |= WLIFU_WSEC_SUPPL;
		}
		else if (lrole == WL_WDS_WPA_ROLE_AUTH) {
			info->flags |= WLIFU_WSEC_AUTH;
		}
		else {
			/* unable to determine WPA role */
			return WLIFU_ERR_WL_WPA_ROLE;
		}
	}
	/* user-supplied psk passphrase */
	if (CHECK_PSK(info->akm)) {
		if (wds && wds_wsec) {
			strncpy((char *)info->psk, wds_psk, MAX_USER_KEY_LEN);
			info->psk[MAX_USER_KEY_LEN] = 0;
		}
		else {
			value = nvram_safe_get(strcat_r(wl_prefix, "wpa_psk", comb));
			strncpy((char *)info->psk, value, MAX_USER_KEY_LEN);
			info->psk[MAX_USER_KEY_LEN] = 0;
		}
	}
	/* user-supplied radius server secret */
	if (CHECK_RADIUS(info->akm))
		info->secret = nvram_safe_get(strcat_r(wl_prefix, "radius_key", comb));
	/* AP specific settings */
	value = nvram_safe_get(strcat_r(wl_prefix, "mode", comb));
	if (!strcmp(value, "ap")) {
		/* gtk rekey interval */
		if (CHECK_NAS(info->akm)) {
			value = nvram_safe_get(strcat_r(wl_prefix, "wpa_gtk_rekey", comb));
			info->gtk_rekey_secs = (int)strtoul(value, NULL, 0);
		}
		/* wep key */
		if (info->wsec & WEP_ENABLED) {
			/* key index */
			value = nvram_safe_get(strcat_r(wl_prefix, "key", comb));
			info->wep_index = (int)strtoul(value, NULL, 0);
			/* key */
			sprintf(key, "key%s", nvram_safe_get(strcat_r(wl_prefix, "key", comb)));
			info->wep_key = nvram_safe_get(strcat_r(wl_prefix, key, comb));
		}
		/* radius server host/port */
		if (CHECK_RADIUS(info->akm)) {
			/* update radius server address */
			info->radius_addr = nvram_safe_get(strcat_r(wl_prefix, "radius_ipaddr",
			                                            comb));
			value = nvram_safe_get(strcat_r(wl_prefix, "radius_port", comb));
			info->radius_port = htons((int)strtoul(value, NULL, 0));
			/* 802.1x session timeout/pmk cache duration */
			value = nvram_safe_get(strcat_r(wl_prefix, "net_reauth", comb));
			info->ssn_to = (int)strtoul(value, NULL, 0);
		}
	}
	/* preauth */
	value = nvram_safe_get(strcat_r(wl_prefix, "preauth", comb));
	info->preauth = (int)strtoul(value, NULL, 0);

	/* verbose */
	value = nvram_safe_get(strcat_r(wl_prefix, "nas_dbg", comb));
	info->debug = (int)strtoul(value, NULL, 0);

	/* get mfp setting */
	info->mfp = atoi(nvram_safe_get(strcat_r(wl_prefix, "mfp", comb)));

	return WLIFU_WSEC_SUCCESS;
}

#ifdef RTCONFIG_BCM_7114
#ifndef BCM_EVENT_HEADER_LEN
#define BCM_EVENT_HEADER_LEN   (sizeof(bcm_event_t))
#endif

/* listen to sockets for bss response event */
static int
wl_wlif_proc_event_socket(int event_fd, struct timeval *tv, char *ifreq, uint8 token,
	struct ether_addr *bssid)
{
	fd_set fdset;
	int fdmax;
	int width, status = 0, bytes;
	char buf_ptr[WL_WLIF_BUFSIZE_4K], *pkt = buf_ptr;
	char ifname[IFNAMSIZ+1];
	int pdata_len;
	dot11_bsstrans_resp_t *bsstrans_resp;
	dot11_neighbor_rep_ie_t *neighbor;
	bcm_event_t *dpkt;
	uint32 event_id;
	struct ether_addr *addr;

	if (bssid == NULL) {
		WLIF_BSSTRANS("bssid is NULL\n");
		return WLIFU_BSS_TRANS_RESP_UNKNOWN;
	}

	WLIF_BSSTRANS("For event_fd[%d] ifname[%s] bss_token[%d] bssid["MACF"]\n",
		event_fd, ifreq, token, ETHERP_TO_MACF(bssid));
	/* init file descriptor set */
	FD_ZERO(&fdset);
	fdmax = -1;

	/* build file descriptor set now to save time later */
	if (event_fd != -1) {
		FD_SET(event_fd, &fdset);
		fdmax = event_fd;
	}

	width = fdmax + 1;

	/* listen to data availible on all sockets */
	status = select(width, &fdset, NULL, NULL, tv);

	if ((status == -1 && errno == EINTR) || (status == 0)) {
		WLIF_BSSTRANS("ifname[%s] status[%d] errno[%d]- No event\n",
			ifreq, status, errno);
		return WLIFU_BSS_TRANS_RESP_UNKNOWN;
	}

	if (status <= 0) {
		WLIF_BSSTRANS("ifname[%s]: err from select: %s\n", ifreq, strerror(errno));
		return WLIFU_BSS_TRANS_RESP_UNKNOWN;
	}

	/* handle brcm event */
	if (event_fd !=  -1 && FD_ISSET(event_fd, &fdset)) {
		memset(pkt, 0, sizeof(buf_ptr));
		if ((bytes = recv(event_fd, pkt, sizeof(buf_ptr), 0)) <= IFNAMSIZ) {
			WLIF_BSSTRANS("ifname[%s] BSS Transit Response: recv err\n", ifreq);
			return WLIFU_BSS_TRANS_RESP_UNKNOWN;
		}

		strncpy(ifname, pkt, IFNAMSIZ);
		ifname[IFNAMSIZ] = '\0';

		pkt = pkt + IFNAMSIZ;
		pdata_len = bytes - IFNAMSIZ;

		if (pdata_len <= BCM_EVENT_HEADER_LEN) {
			WLIF_BSSTRANS("ifname[%s] BSS Transit Response: data_len %d too small\n",
				ifreq, pdata_len);
		}

		dpkt = (bcm_event_t *)pkt;
		event_id = ntohl(dpkt->event.event_type);

		pkt += BCM_EVENT_HEADER_LEN; /* payload (bss response) */
		pdata_len -= BCM_EVENT_HEADER_LEN;

		bsstrans_resp = (dot11_bsstrans_resp_t *)pkt;
		addr = (struct ether_addr *)(bsstrans_resp->data);
		neighbor = (dot11_neighbor_rep_ie_t *)bsstrans_resp->data;

		WLIF_BSSTRANS("BSS Transit Response: ifname[%s], event[%d], "
			"token[%d], status[%d], mac["MACF"]\n",
			ifname, event_id, bsstrans_resp->token,
			bsstrans_resp->status, ETHERP_TO_MACF(addr));

		/* check interface */
		if (strncmp(ifname, ifreq, strlen(ifreq)) != 0) {
			/* not for the requested interface */
			WLIF_BSSTRANS("BSS Transit Response: not for interface %s its for %s\n",
				ifreq, ifname);
			return WLIFU_BSS_TRANS_RESP_UNKNOWN;
		}

		/* check token */
		if (bsstrans_resp->token != token) {
			/* not for the requested interface */
			WLIF_BSSTRANS("BSS Transit Response: not for token %x it's for %x\n",
				token, bsstrans_resp->token);
			return WLIFU_BSS_TRANS_RESP_UNKNOWN;
		}

		/* reject */
		if (bsstrans_resp->status) {
			/* If there is some neigbor report */
			if (status == 6) {
				WLIF_BSSTRANS("id[%d] len[%d] bssid["MACF"] bssid_info[%d] reg[%d] "
					"channel[%d] phytype[%d] \n",
					neighbor->id, neighbor->len, ETHER_TO_MACF(neighbor->bssid),
					neighbor->bssid_info, neighbor->reg, neighbor->channel,
					neighbor->phytype);
			}
			return WLIFU_BSS_TRANS_RESP_REJECT;
		}

		/* accept, but use another target bssid */
		if (eacmp(bssid, addr) != 0) {
			WLIF_BSSTRANS("BSS Transit Response: target bssid not same "
				"["MACF"] != ["MACF"]\n", ETHERP_TO_MACF(bssid),
				ETHERP_TO_MACF(addr));
			return WLIFU_BSS_TRANS_RESP_REJECT;
		}

		return WLIFU_BSS_TRANS_RESP_ACCEPT;
	}

	return WLIFU_BSS_TRANS_RESP_UNKNOWN;
}

/* create blocking list of macs */
static void
wl_update_block_mac_list(maclist_t *static_maclist, maclist_t *maclist,
	int macmode, struct ether_addr *addr, unsigned char block)
{
	uint16 count = 0, idx, action;

	/* Action table
	 * ALLOW  BLOCK		DEL
	 * DENY   BLOCK		ADD
	 * ALLOW  UNBLOCK	ADD
	 * DENY   UNBLOCK	DEL
	 */
	action = ((macmode == WLC_MACMODE_ALLOW) ^ block) ? WL_MAC_ADD : WL_MAC_DEL;
	switch (action) {
		case WL_MAC_DEL:
			for (idx = 0; idx < static_maclist->count; idx++) {
				if (eacmp(addr, &static_maclist->ea[idx]) == 0) {
					continue;
				}
				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
					ETHER_ADDR_LEN);
				count++;
			}
			maclist->count = count;
			break;

		case WL_MAC_ADD:
			for (idx = 0; static_maclist != NULL && idx < static_maclist->count;
					idx++) {
				memcpy(&(maclist->ea[count]), &static_maclist->ea[idx],
					ETHER_ADDR_LEN);
				count++;
			}

			memcpy(&(maclist->ea[count]), addr,  ETHER_ADDR_LEN);
			count++;
			maclist->count = count;
			break;

		default:
			printf("Wrong action= %d\n", action);
			ASSERT(0);
	}
}

int wl_wlif_block_mac(void *hdl, char *ifname, struct ether_addr addr, int timeout)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode, ret, macprobresp;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;
	static_maclist_t *data;
	static short flag;
	bcm_timer_module_id timer;
	bcm_timer_id timerid;
	struct itimerspec  its;

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);

	ret = wl_ioctl(ifname, WLC_GET_MACMODE,	&(macmode), sizeof(macmode));
	if (ret < 0) {
		printf("Err: get %s macmode fails %d\n", ifname, ret);
		return FALSE;
	}

	/* retrive static maclist */
	if (wl_ioctl(ifname, WLC_GET_MACLIST, (void *)s_maclist, sizeof(maclist_buf)) < 0) {
		printf("Err: get %s maclist fails\n", ifname);
		return FALSE;
	}

	ret = wl_iovar_getint(ifname, "probresp_mac_filter", &macprobresp);
	if (ret != 0)
		printf("Err: %s Probresp mac filter set failed %d\n", ifname, ret);

	if (flag == 0) {
		bcm_timer_module_init(TIMER_MAX_COUNT, &timer);
		flag = s_maclist->count;
		flag |= WL_MAC_PROB_MODE_SET(macmode, macprobresp, TRUE);
	}

	if (timeout != 0) {
		data = (static_maclist_t *) malloc(sizeof(static_maclist_t));
		strncpy(data->ifname, ifname, IFNAMSIZ - 1);
		data->ifname[IFNAMSIZ - 1] = '\0';
		data->flag = flag;
		memcpy(&(data->addr), &addr, ETHER_ADDR_LEN);
		if (hdl == NULL) {
			if (bcm_timer_create(timer, &timerid)) {
				free(data);
				return FALSE;
			}
			else if (bcm_timer_connect(timerid, (bcm_timer_cb)wl_wlif_unblock_mac_cb,
				(int)data)) {
				free(data);
				return FALSE;
			}

			/* set up retry timer */
			its.it_interval.tv_sec = timeout;
			its.it_interval.tv_nsec = 0;
			its.it_value.tv_sec = timeout;
			its.it_value.tv_nsec = 0;
			if (bcm_timer_settime(timerid, &its)) {
				free(data);
				return FALSE;
			}
		}
		else {
			ret = bcm_usched_add_timer(hdl, SEC_TO_MICROSEC(timeout), FALSE,
				wl_wlif_unblock_mac_usched_cb, (void*)data);
			if (ret != BCM_USCHEDE_OK) {
				free(data);
				printf("Failed to add timer Err:%s\n", bcm_usched_strerror(ret));
				return FALSE;
			}
		}
	}
	wl_update_block_mac_list(s_maclist, maclist, macmode, &addr, TRUE);
	macmode = (macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY;
	wl_ioctl(ifname, WLC_SET_MACLIST, maclist, ETHER_ADDR_LEN * maclist->count
			+ sizeof(uint));
	wl_ioctl(ifname, WLC_SET_MACMODE, &macmode, sizeof(int));

	ret = wl_iovar_setint(ifname, "probresp_mac_filter", TRUE);
	if (ret != 0)
		printf("Err: %s Probresp mac filter set failed %d\n", ifname, ret);

	return TRUE;
}

int wl_wlif_do_bss_trans(void *hdl, char *ifname, uint8 rclass, chanspec_t chanspec,
	struct ether_addr bssid, struct ether_addr addr, int timeout, int event_fd)
{
	int ret, no_deauth = 0;
	char *value;

	ret = wl_wlif_send_bss_transreq(ifname, rclass, chanspec, bssid, addr, event_fd);
	/* If the ret WLIFU_BSS_TRANS_RESP_ACCEPT:
	 *	STA will steer, no need to send deauth
	 * If the ret WLIFU_BSS_TRANS_RESP_REJECT:
	 *	Just return
	 * For WLIFU_BSS_TRANS_RESP_UNKNOWN: (Legacy sta)
	 *	Will be based on NVRAM setting to determine if send deauth or not
	 */
	if (ret == WLIFU_BSS_TRANS_RESP_REJECT) {
		WLIF_BSSTRANS("STA["MACF"] Rejected from BSSID["MACF"]\n", ETHER_TO_MACF(addr),
			ETHER_TO_MACF(bssid));
		return ret;
	} else if (ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) {
		/* Read the NVRAM to deauth or not */
		value = nvram_safe_get(WLIFU_NVRAM_BSS_TRANS_NO_DEAUTH);
		no_deauth = (int)strtoul(value, NULL, 0);
		if (no_deauth) {
			WLIF_BSSTRANS("Not sending deauth to STA["MACF"] from %s by "
				"NVRAM WLIFU_NVRAM_BSS_TRANS_NO_DEAUTH setting\n",
				ETHER_TO_MACF(addr), ifname);
			return ret;
		}
	}

	wl_wlif_block_mac(hdl, ifname, addr, timeout);

	if (ret == WLIFU_BSS_TRANS_RESP_UNKNOWN) {
		/* no_deauth = 0 here, so send deauth */
		if (wl_ioctl(ifname, WLC_SCB_DEAUTHENTICATE, &addr, ETHER_ADDR_LEN) < 0) {
			WLIF_BSSTRANS("Deauth to STA["MACF"] from %s failed\n",
				ETHER_TO_MACF(addr), ifname);
		}
	}

	return ret;
}

static int
wl_wlif_unblock_mac_cb(bcm_timer_id timer, static_maclist_t *data)
{
	int ret;

	ret = wl_wlif_unblock_mac(data->ifname, data->addr, data->flag);

	free(data);
	bcm_timer_delete(timer);

	return ret;
}

static int
wl_wlif_unblock_mac_usched_cb(bcm_usched_handle* hdl, void* arg)
{
	int ret;

	static_maclist_t *data = (static_maclist_t*)arg;
	ret = wl_wlif_unblock_mac(data->ifname, data->addr, data->flag);

	free(data);
	return ret;
}

static int
wl_wlif_send_bss_transreq(char *ifname, uint8 rclass, chanspec_t chanspec,
	struct ether_addr bssid, struct ether_addr addr, int event_fd)
{
	int ret, buflen;
	char *param, ioctl_buf[WLC_IOCTL_MAXLEN];
	struct timeval tv; /* timed out for bss response */

	dot11_bsstrans_req_t *transreq;
	dot11_neighbor_rep_ie_t *nbr_ie;

	wl_af_params_t *af_params;
	wl_action_frame_t *action_frame;

	memset(ioctl_buf, 0, sizeof(ioctl_buf));
	strcpy(ioctl_buf, "actframe");
	buflen = strlen(ioctl_buf) + 1;
	param = (char *)(ioctl_buf + buflen);

	af_params = (wl_af_params_t *)param;
	action_frame = &af_params->action_frame;

	af_params->channel = 0;
	af_params->dwell_time = -1;

	memcpy(&action_frame->da, (char *)&(addr), ETHER_ADDR_LEN);
	action_frame->packetId = (uint32)(uintptr)action_frame;
	action_frame->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN +
		TLV_HDR_LEN + DOT11_BSSTRANS_REQ_LEN;

	transreq = (dot11_bsstrans_req_t *)&action_frame->data[0];
	transreq->category = DOT11_ACTION_CAT_WNM;
	transreq->action = DOT11_WNM_ACTION_BSSTRANS_REQ;
	if (++bss_token == 0)
		bss_token = 1;
	transreq->token = bss_token;
	transreq->reqmode = DOT11_BSSTRANS_REQMODE_PREF_LIST_INCL;
	/* set bit1 to tell STA the BSSID in list recommended */
	transreq->reqmode |= DOT11_BSSTRANS_REQMODE_ABRIDGED;
	/*
		remove bit2 DOT11_BSSTRANS_REQMODE_DISASSOC_IMMINENT
		because WBD will deauth sta based on BSS response
	*/
	transreq->disassoc_tmr = 0x0000;
	transreq->validity_intrvl = 0x00;

	nbr_ie = (dot11_neighbor_rep_ie_t *)&transreq->data[0];
	nbr_ie->id = DOT11_MNG_NEIGHBOR_REP_ID;
	nbr_ie->len = DOT11_NEIGHBOR_REP_IE_FIXED_LEN;
	memcpy(&nbr_ie->bssid, &bssid, ETHER_ADDR_LEN);
	nbr_ie->bssid_info = 0x00000000;
	nbr_ie->reg = rclass;
	nbr_ie->channel = wf_chspec_ctlchan(chanspec);
	nbr_ie->phytype = 0x00;

	ret = wl_ioctl(ifname, WLC_SET_VAR, ioctl_buf, WL_WIFI_AF_PARAMS_SIZE);

	if (ret < 0) {
		printf("Err: intf:%s actframe %d\n", ifname, ret);
	} else if (event_fd != -1) {
		/* read the BSS transition response only if event_fd is valid */
		usleep(1000*500);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		/* wait for bss response and compare token/ifname/status/bssid etc  */
		return (wl_wlif_proc_event_socket(event_fd, &tv, ifname,
			bss_token, &bssid));
	}
	return WLIFU_BSS_TRANS_RESP_UNKNOWN;
}

int wl_wlif_unblock_mac(char *ifname, struct ether_addr addr, int flag)
{
	char maclist_buf[WLC_IOCTL_MAXLEN];
	char smaclist_buf[WLC_IOCTL_MAXLEN];
	int macmode;
	maclist_t *s_maclist = (maclist_t *)smaclist_buf;
	maclist_t *maclist = (maclist_t *)maclist_buf;

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	memset(smaclist_buf, 0, WLC_IOCTL_MAXLEN);

	if (wl_ioctl(ifname, WLC_GET_MACMODE, &(macmode), sizeof(macmode)) != 0) {
		printf("Err: get %s macmode fails \n", ifname);
		return FALSE;
	}

	/* retrive static maclist */
	if (wl_ioctl(ifname, WLC_GET_MACLIST, (void *)s_maclist, sizeof(maclist_buf)) < 0) {
		printf("Err: get %s maclist fails\n", ifname);
		return FALSE;
	}

	wl_update_block_mac_list(s_maclist, maclist, macmode, &(addr), FALSE);
	macmode = (macmode == WLC_MACMODE_ALLOW) ? WLC_MACMODE_ALLOW : WLC_MACMODE_DENY;

	if (flag && (maclist->count == (flag & 0xFF))) {
		macmode = WL_MACMODE_GET(flag);
		if (wl_iovar_setint(ifname, "probresp_mac_filter", WL_MACPROBE_GET(flag)) != 0) {
			printf("Error:%s Probresp mac filter set failed \n", ifname);
		}
	}

	wl_ioctl(ifname, WLC_SET_MACLIST, maclist, ETHER_ADDR_LEN * maclist->count
			+ sizeof(uint));
	wl_ioctl(ifname, WLC_SET_MACMODE, &macmode, sizeof(int));

	return TRUE;
}

/* get the Max NSS */
int
wl_wlif_get_max_nss(wl_bss_info_t *bi)
{
	int i = 0, mcs_idx = 0;
	int mcs = 0, isht = 0;
	int nss = 0;

	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
		if (bi->vht_cap) {
			uint mcs_cap = 0;

			for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
				mcs_cap = VHT_MCS_MAP_GET_MCS_PER_SS(i,
					dtoh16(bi->vht_txmcsmap));
				if (mcs_cap != VHT_CAP_MCS_MAP_NONE) {
					nss++; /* Calculate the number of streams */
				}
			}

			if (nss) {
				return nss;
			}
		}

		/* For 802.11n networks, use MCS table */
		for (mcs_idx = 0; mcs_idx < (MCSSET_LEN * 8); mcs_idx++) {
			if (isset(bi->basic_mcs, mcs_idx) && mcs_idx < MCS_TABLE_SIZE) {
				mcs = mcs_idx;
				isht = 1;
			}
		}

		if (isht) {
			int nss = 0;

			if (mcs > 32) {
				printf("MCS is Out of range \n");
			} else if (mcs == 32) {
				nss = 1;
			} else {
				nss = 1 + (mcs / 8);
			}

			return nss;
		}
	}

	return nss;
}

#ifdef __CONFIG_RSDB__
int
wl_wlif_get_rsdb_mode()
{
        char *mode;
        int rsdb_mode = WLIF_RSDB_MODE_2X2; /* default rsdb_mode is mimo */

        mode = nvram_get("rsdb_mode");
        if (mode)
                rsdb_mode = atoi(mode);
        return rsdb_mode;
}
#endif /* __CONFIG_RSDB__ */

/* Generic utility function to check for a known capability */
int
wl_wlif_get_chip_cap(char *ifname, char *cap)
{
        char caps[WLC_IOCTL_MEDLEN], var[WLC_IOCTL_SMLEN], *next;

        if (wl_iovar_get(ifname, "cap", (void *)caps, sizeof(caps)) != BCME_OK)
                return FALSE;

        foreach(var, caps, next) {
                if (strncmp(var, cap, sizeof(var)) == 0)
                        return TRUE;
        }

        return FALSE;
}

int
get_bridge_by_ifname(char* ifname, char** brname)
{
	char name[IFNAMSIZ] = {0}, *next = NULL;
	char nv_name[16] = {0};
	char *br_ifnames = NULL;
	int i, found = 0;

	/* Search in LAN network */
	br_ifnames = nvram_safe_get("lan_ifnames");
	foreach(name, br_ifnames, next) {
		if (!strcmp(name, ifname)) {
			found = 1;
			break;
		}
	}

	if (found) {
		*brname = nvram_safe_get("lan_ifname");
		return 0;
	}

	/* Search in GUEST networks */
	for (i = 1; i < WLIFU_MAX_NO_BRIDGE; i++) {
		snprintf(nv_name, 16, "lan%d_ifnames", i);
		br_ifnames = nvram_safe_get(nv_name);
		foreach(name, br_ifnames, next) {
			if (!strcmp(name, ifname)) {
				found = 1;
				break;
			}
		}
		if (found) {
			snprintf(nv_name, 16, "lan%d_ifname", i);
			*brname = nvram_safe_get(nv_name);
			return 0;
		}
	}

	return -1;
}

/* Get associated AP ifname for WDS link */
int
wl_wlif_wds_ap_ifname(char *ifname, char *apname)
{
        int ret;
        char wdsap_nvifname[IFNAMSIZ] = {0};

        if (wl_probe(ifname) < 0) {
                return -1;
        }

        /* Get associated AP ifname and convert it to OS ifname */
        ret = wl_iovar_get(ifname, "wds_ap_ifname", (void *)wdsap_nvifname, IFNAMSIZ);

        if (!ret) {
                ret = nvifname_to_osifname(wdsap_nvifname, apname, IFNAMSIZ);
        } else {
                printf("Err: get %s wds_ap_ifname fails %d\n", ifname, ret);
        }

        return ret;
}
#endif

#if defined(CONFIG_HOSTAPD) && defined(BCA_HNDROUTER)
#define WLIF_WPS_LED_OFFSET			0
#define WLIF_WPS_LED_MASK			0x03L
#define WLIF_WPS_LED_STATUS_OFFSET		8
#define WLIF_WPS_LED_STATUS_MASK		0x0000ff00L
#define WLIF_WPS_LED_EVENT_OFFSET		16
#define WLIF_WPS_LED_EVENT_MASK			0x00ff0000L
#define WLIF_WPS_LED_BLINK_OFFSET		24
#define WLIF_WPS_LED_BLINK_MASK			0xff000000L

/* WPS led states */
#define WLIF_WPS_LED_OFF			0
#define WLIF_WPS_LED_ON				1
#define WLIF_WPS_LED_BLINK			2

/* WPS SM State */
#define WLIF_WPS_STATE_IDLE			0
#define WLIF_WPS_STATE_WAITING			1
#define WLIF_WPS_STATE_SUCC			2
#define WLIF_WPS_STATE_TIMEOUT			3
#define WLIF_WPS_STATE_FAIL			4
#define WLIF_WPS_STATE_OVERLAP			5

#define WLIF_WPS_EVENT_START			2
#define WLIF_WPS_EVENT_IDLE			(WLIF_WPS_STATE_IDLE + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_WAITING			(WLIF_WPS_STATE_WAITING + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_SUCC			(WLIF_WPS_STATE_SUCC + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_TIMEOUT			(WLIF_WPS_STATE_TIMEOUT + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_FAIL			(WLIF_WPS_STATE_FAIL + WLIF_WPS_EVENT_START)
#define WLIF_WPS_EVENT_OVERLAP			(WLIF_WPS_STATE_OVERLAP + WLIF_WPS_EVENT_START)

/* Sets the wps led status */
static void
wl_wlif_wps_led_set(int board_fp, int led_action, int led_blink_type,
	int led_event, int led_status)
{
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	led_action &= WLIF_WPS_LED_MASK;
	led_action |= ((led_status << WLIF_WPS_LED_STATUS_OFFSET) & WLIF_WPS_LED_STATUS_MASK);
	led_action |= ((led_event << WLIF_WPS_LED_EVENT_OFFSET) & WLIF_WPS_LED_EVENT_MASK);
	led_action |= ((led_blink_type << WLIF_WPS_LED_BLINK_OFFSET) & WLIF_WPS_LED_BLINK_MASK);

	ioctl_parms.result = -1;
	ioctl_parms.string = (char *)&led_action;
	ioctl_parms.strLen = sizeof(led_action);
	ioctl(board_fp, BOARD_IOCTL_SET_SES_LED, &ioctl_parms);

	return;
}

/* Routine for changing wps led status */
void
wl_wlif_wps_gpio_led_blink(int board_fp, wlif_wps_blinktype_t blinktype)
{
	if (board_fp <= 0) {
		return;
	}

	switch ((int)blinktype) {
		case WLIF_WPS_BLINKTYPE_STOP:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_OFF, 0, 0, 0);
			break;

		case WLIF_WPS_BLINKTYPE_INPROGRESS:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_BLINK,
				kLedStateUserWpsInProgress, WLIF_WPS_EVENT_WAITING, 0);
			break;

		case WLIF_WPS_BLINKTYPE_ERROR:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_BLINK,
				kLedStateUserWpsError, WLIF_WPS_EVENT_FAIL, 0);
			break;

		case WLIF_WPS_BLINKTYPE_OVERLAP:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_BLINK,
				kLedStateUserWpsSessionOverLap, WLIF_WPS_EVENT_OVERLAP, 0);
			break;

		case WLIF_WPS_BLINKTYPE_SUCCESS:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_ON,
				kLedStateOn, WLIF_WPS_EVENT_SUCC, 0);
			break;

		default:
			wl_wlif_wps_led_set(board_fp, WLIF_WPS_LED_OFF, 0, 0, 0);
			break;
	}
}

/* wps gpio init fn */
int
wl_wlif_wps_gpio_init()
{

	int board_fp = open("/dev/brcmboard", O_RDWR);

	if (board_fp <= 0) {
		cprintf("shared: Info %s %d failed to open /dev/brcmboard\n", __func__, __LINE__);
		return -1;
	}

	return board_fp;
}

/* wps gpio cleanup fn */
void
wl_wlif_wps_gpio_cleanup(int board_fp)
{
	if (board_fp > 0) {
		close(board_fp);
	}
}
#endif	/* CONFIG_HOSTAPD && BCA_HNDROUTER */

#ifdef CONFIG_HOSTAPD

#define WLIF_HAPD_DIR		"/var/run/hostapd"

#ifdef MULTIAP
/* Struct to hold the list of type wlif_bss_t */
typedef struct wlif_bss_list {
	int count;
	wlif_bss_t bss[WL_MAXBSSCFG];
} wlif_bss_list_t;

// Struct to pass backhaul credentials to hostapd using cli cmd
typedef struct wlif_bh_creds_hapd_clicmd_data {
	char ssid[WLIF_SSID_MAX_SZ];	// SSID
	char key[WLIF_PSK_MAX_SZ];	// PSK
	char auth[16];			// Auth can be WPAPSK/WPA2PSK/OPEN
	char encr[16];			// Encr	can be TKIP/CCMP/NONE
} wlif_bh_creds_hapd_clicmd_data_t;

static void wl_wlif_update_hapd_bh_creds(char *wps_ifname, char *bh_ifname);
static int wl_wlif_fill_bh_creds_from_nvram(char *nvifname, wlif_bh_creds_hapd_clicmd_data_t *cmd);
#endif	/* MULTIAP */

static void wl_wlif_wpa_supplicant_update_ap_scan(char *ifname, char *nvifname, int val);

// WPS Status id and code value pairs
typedef struct wlif_wps_status {
	wlif_wps_ui_status_code_id_t idx;
	char *val;
} wlif_wps_ui_status_t;


// Array of wps ui status codes
static wlif_wps_ui_status_t wlif_wps_ui_status_arr[] =
{
	{WLIF_WPS_UI_INIT, "0"},
	{WLIF_WPS_UI_FINDING_PBC_STA, "1"},
	{WLIF_WPS_UI_OK, "2"},
	{WLIF_WPS_UI_ERR, "3"},
	{WLIF_WPS_UI_TIMEOUT, "4"},
	{WLIF_WPS_UI_PBCOVERLAP, "8"},
	{WLIF_WPS_UI_FIND_PBC_AP, "9"},
	{WLIF_WPS_UI_FIND_PIN_AP, "11"}
};

/* Updates the wps_proc_status nvram to reflect the wps status in wps.asp page */
void
wl_wlif_update_wps_ui(wlif_wps_ui_status_code_id_t idx)
{
	if (idx < 0 || idx >= ARRAYSIZE(wlif_wps_ui_status_arr)) {
		return;
	}

	nvram_set("wps_proc_status", wlif_wps_ui_status_arr[idx].val);
}

/* Gets the status code from the wps_proc_status nvram value */
int
wl_wlif_get_wps_status_code()
{
	wlif_wps_ui_status_code_id_t idx = WLIF_WPS_UI_INIT;
	char *nvval = nvram_safe_get("wps_proc_status");

	if (nvval[0] == '\0') {
		return -1;
	}

	for (idx = 0; idx < ARRAYSIZE(wlif_wps_ui_status_arr); idx++) {
		if (!strcmp(nvval, wlif_wps_ui_status_arr[idx].val)) {
			return idx;
		}
	}

	return -1;
}
/* Saves the network credentials received from the wps session */
static void
wl_wlif_save_wpa_settings(char *type, char *val, wlif_wps_nw_creds_t *creds)
{

	int len = 0;

	/* In wpa_supplicant config file double quotes (") being added for ssid and
	 * psk configurations eg. ssid="BCM_TST". Before saving these values to nvrams we need to
	 * remove the double quotes from start and end of the string.
	 */
	if (strstr(type, "scan_ssid")) {
		// Do nothing.
	} else if (strstr(type, "ssid")) {
		len = strlen(val) > (WLIF_SSID_MAX_SZ + 1) ? WLIF_SSID_MAX_SZ + 1 : strlen(val);
		strncpy(creds->ssid, val + 1, sizeof(creds->ssid) -1);
		creds->ssid[len - 2 /* for " at the start and end of string */]  = '\0';
	} else if (strstr(type, "psk")) {
		len = strlen(val) > (WLIF_PSK_MAX_SZ + 1) ? WLIF_PSK_MAX_SZ + 1 : strlen(val);
		strncpy(creds->nw_key, val + 1, sizeof(creds->nw_key) -1);
		creds->nw_key[len - 2 /* for " at the start and end of string */] = '\0';
	} else if (strstr(type, "key_mgmt")) {
		if (strstr(val, "WPA-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK;
		}
		if (strstr(val, "WPA2-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK2;
		}
	} else if (strstr(type, "auth_alg")) {
		if (strstr(val, "OPEN")) {
			creds->auth_type |= WLIF_WPA_AUTH_OPEN;
		}
		if (strstr(val, "SHARED")) {
			creds->auth_type |= WLIF_WPA_AUTH_SHARED;
		}
	} else if (strstr(type, "pairwise")) {
		if (strstr(val, "TKIP")) {
			creds->encr |= WLIF_WPA_ENCR_TKIP;
		}
		if (strstr(val, "CCMP")) {
			creds->encr |= WLIF_WPA_ENCR_AES;
		}
	}
}

/* Apply the DPP credentials to radio interface received from the wps session */
int
wl_wlif_apply_dpp_creds(wlif_bss_t *bss, wlif_dpp_creds_t *dpp_creds)
{
	char nv_name[WLIF_MIN_BUF] = {0};
	int ret = -1;
	char *prefix;
	bool ap = 0;

	if (!bss || !dpp_creds) {
		cprintf("Err: shared %s bss and credentials can not be null \n", __func__);
		return -1;
	}

	prefix = bss->nvifname;
	dprintf("Info: shared %s received credentials after dpp:"
		"\nssid = [%s] akm = [%s] for ifname = [%s]\n",
		__func__, dpp_creds->ssid, dpp_creds->akm, prefix);

	// ssid
	snprintf(nv_name, sizeof(nv_name), "%s_ssid", prefix);
	if (!nvram_match(nv_name, dpp_creds->ssid)) {
		nvram_set(nv_name, dpp_creds->ssid);
		ret = 0;
	}

	memset(nv_name, 0, sizeof(nv_name));
	snprintf(nv_name, sizeof(nv_name), "%s_mode", prefix);
	ap = nvram_match(nv_name, "ap");

	// akm
	memset(nv_name, 0, sizeof(nv_name));
	snprintf(nv_name, sizeof(nv_name), "%s_akm", prefix);
	/* For STA, DPP_EVENT_CONFOBJ_AKM is not received, so dpp_creds->akm is not populated.
	 * For AP, DPP_EVENT_CONFOBJ_AKM is received as part of DPP configuration response,
	 * so use the akm received in the event.
	 */
	if (ap && !nvram_match(nv_name, dpp_creds->akm)) {
		nvram_set(nv_name, dpp_creds->akm);
		ret = 0;
	}

	// If akm is dpp, set dpp specific nvrams.
	if (nvram_match(nv_name, "dpp")) {
		/* crypto */
		memset(nv_name, 0, sizeof(nv_name));
		snprintf(nv_name, sizeof(nv_name), "%s_crypto", prefix);
		nvram_set(nv_name, "aes");

		/* mfp */
		memset(nv_name, 0, sizeof(nv_name));
		snprintf(nv_name, sizeof(nv_name), "%s_mfp", prefix);
		nvram_set(nv_name, "2");

		/* dpp_connector */
		memset(nv_name, 0, sizeof(nv_name));
		snprintf(nv_name, sizeof(nv_name), "%s_dpp_connector", prefix);
		nvram_set(nv_name, dpp_creds->dpp_connector);

		/* dpp_csign */
		memset(nv_name, 0, sizeof(nv_name));
		snprintf(nv_name, sizeof(nv_name), "%s_dpp_csign", prefix);
		nvram_set(nv_name, dpp_creds->dpp_csign);

		/* dpp_netaccess_key */
		memset(nv_name, 0, sizeof(nv_name));
		snprintf(nv_name, sizeof(nv_name), "%s_dpp_netaccess_key", prefix);
		nvram_set(nv_name, dpp_creds->dpp_netaccess_key);

		ret = 0;
	}

	if (!ret) {
		/* dpp_conf_recvd */
		memset(nv_name, 0, sizeof(nv_name));
		snprintf(nv_name, sizeof(nv_name), "%s_dpp_conf_recvd", prefix);
		dprintf("Set nv_name:%s to 1\n", nv_name);
		nvram_set(nv_name, "1");
		nvram_commit();
	}

	return ret;
}

/* Apply the network credentials to radio interface received from the wps session */
int
wl_wlif_apply_creds(wlif_bss_t *bss, wlif_wps_nw_creds_t *creds)
{
	char nv_name[WLIF_MIN_BUF] = {0};
	char *val = "";
	bool wps_v2 = FALSE;
	int ret = -1;
	char *prefix;
	int add_sae = 0;
	char tmp[128];
	char pfcred[] = "wlc_";
	char pfcred0[] = "wlc0_";
	char pfcred1[] = "wlc1_";
	char pfcred2[] = "wlc2_";
	int wlif_num = num_of_wl_if();
	int i, unit = -1;
	char prefix2[] = "wlXXXXXXX_";
	bool wps_configured = nvram_match("w_Setting", "1");

	if (!bss || !creds) {
		cprintf("Err: shared %s bss and credentials can not be null \n", __func__);
		return -1;
	}

	dprintf("Info: shared %s received credentials after wps:"
		"\nssid = [%s] \nakm = [%d] \nencr = [%d] \npsk = [%s]\n",
		__func__, creds->ssid, creds->akm, creds->encr, creds->nw_key);

	if (creds->ssid[0] == '\0' || creds->invalid) {
		cprintf("Info: shared %s invalid credentials are provided \n", __func__);
		return -1;
	}

	prefix = bss->nvifname;
	wl_ioctl(bss->ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));

	// ssid
	snprintf(nv_name, sizeof(nv_name), "%s_ssid", prefix);
	//if (!nvram_match(nv_name, creds->ssid))
	{
		nvram_set(nv_name, creds->ssid);
		if (!wps_configured)
		for (i = 0; i < wlif_num; i++) {
			if (i == unit) continue;
			snprintf(prefix2, sizeof(prefix2), "wl%d", i);
			nvram_set(strcat_r(prefix2, "_ssid", tmp), creds->ssid);
		}

		if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
			nvram_set(strcat_r(pfcred, "ssid", tmp), creds->ssid);
			nvram_set(strcat_r(pfcred0, "ssid", tmp), creds->ssid);
			nvram_set(strcat_r(pfcred1, "ssid", tmp), creds->ssid);
			if (wlif_num == 3)
			nvram_set(strcat_r(pfcred2, "ssid", tmp), creds->ssid);
		}

		ret = 0;
	}

	val = nvram_safe_get("wps_version2");
	if (!strcmp(val, "enabled")) {
		wps_v2 = TRUE;
	}

	snprintf(nv_name, sizeof(nv_name), "%s_wps_cred_add_sae", prefix);
	add_sae = nvram_match(nv_name, "1");

	// for wps version 2 force psk2 if psk1 is provided.
	if (wps_v2 == TRUE) {
		if (creds->akm & WLIF_WPA_AKM_PSK) {
			creds->akm |= WLIF_WPA_AKM_PSK2;
		}
		if (creds->encr & WLIF_WPA_ENCR_TKIP) {
			creds->encr |= WLIF_WPA_ENCR_AES;
		}
	}

	// akm
	val = "";
	snprintf(nv_name, sizeof(nv_name), "%s_akm", prefix);
	if (add_sae && creds->akm & WLIF_WPA_AKM_PSK2) {
		creds->encr = WLIF_WPA_ENCR_AES;
		val = "psk2 sae";
	} else {
		switch (creds->akm) {
			case WLIF_WPA_AKM_PSK:
				val = "psk";
				break;
			case WLIF_WPA_AKM_PSK2:
				val = "psk2";
				break;
			case (WLIF_WPA_AKM_PSK | WLIF_WPA_AKM_PSK2):
				val = "psk psk2";
				break;
		}
	}
	//if (!nvram_match(nv_name, val))
	{
		nvram_set(nv_name, val);
		if (!wps_configured)
		for (i = 0; i < wlif_num; i++) {
			if (i == unit) continue;
			snprintf(prefix2, sizeof(prefix2), "wl%d", i);
			nvram_set(strcat_r(prefix2, "_akm", tmp), val);
		}

		switch (creds->akm) {
			case WLIF_WPA_AKM_PSK:
				if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
					nvram_set(strcat_r(pfcred, "auth_mode", tmp), "psk");
					nvram_set(strcat_r(pfcred0, "auth_mode", tmp), "psk");
					nvram_set(strcat_r(pfcred1, "auth_mode", tmp), "psk");
					if (wlif_num == 3)
					nvram_set(strcat_r(pfcred2, "auth_mode", tmp), "psk");
				} else {
					nvram_set(strcat_r(prefix, "_auth_mode_x", tmp), "psk");
					if (!wps_configured)
					for (i = 0; i < wlif_num; i++) {
						if (i == unit) continue;
						snprintf(prefix2, sizeof(prefix2), "wl%d", i);
						nvram_set(strcat_r(prefix2, "_auth_mode_x", tmp), "psk");
					}
				}
			break;
			case WLIF_WPA_AKM_PSK2:
				if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
					nvram_set(strcat_r(pfcred, "auth_mode", tmp), "psk2");
					nvram_set(strcat_r(pfcred0, "auth_mode", tmp), "psk2");
					nvram_set(strcat_r(pfcred1, "auth_mode", tmp), "psk2");
					if (wlif_num == 3)
					nvram_set(strcat_r(pfcred2, "auth_mode", tmp), "psk2");
				} else {
					nvram_set(strcat_r(prefix, "_auth_mode_x", tmp), "psk2");
					if (!wps_configured)
					for (i = 0; i < wlif_num; i++) {
						if (i == unit) continue;
						snprintf(prefix2, sizeof(prefix2), "wl%d", i);
						nvram_set(strcat_r(prefix2, "_auth_mode_x", tmp), "psk2");
					}
				}
			break;
			case (WLIF_WPA_AKM_PSK | WLIF_WPA_AKM_PSK2):
				if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
					nvram_set(strcat_r(pfcred, "auth_mode", tmp), "psk2");
					nvram_set(strcat_r(pfcred0, "auth_mode", tmp), "psk2");
					nvram_set(strcat_r(pfcred1, "auth_mode", tmp), "psk2");
					if (wlif_num == 3)
					nvram_set(strcat_r(pfcred2, "auth_mode", tmp), "psk2");
				} else {
					nvram_set(strcat_r(prefix, "_auth_mode_x", tmp), "pskpsk2");
					if (!wps_configured)
					for (i = 0; i < wlif_num; i++) {
						if (i == unit) continue;
						snprintf(prefix2, sizeof(prefix2), "wl%d", i);
						nvram_set(strcat_r(prefix2, "_auth_mode_x", tmp), "pskpsk2");
					}
				}
			break;
			default:
				if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
					nvram_set(strcat_r(pfcred, "auth_mode", tmp), "open");
					nvram_set(strcat_r(pfcred0, "auth_mode", tmp), "open");
					nvram_set(strcat_r(pfcred1, "auth_mode", tmp), "open");
					if (wlif_num == 3)
						nvram_set(strcat_r(pfcred2, "auth_mode", tmp), "open");
				} else {
					nvram_set(strcat_r(prefix, "_auth_mode_x", tmp), "open");
					if (!wps_configured)
					for (i = 0; i < wlif_num; i++) {
						if (i == unit) continue;
						snprintf(prefix2, sizeof(prefix2), "wl%d", i);
						nvram_set(strcat_r(prefix2, "_auth_mode_x", tmp), "open");
					}
				}
			break;
		}

		nvram_set(strcat_r(prefix, "_wep", tmp), "0");
		if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
			nvram_set(strcat_r(pfcred, "wep", tmp), "0");
			nvram_set(strcat_r(pfcred0, "wep", tmp), "0");
			nvram_set(strcat_r(pfcred1, "wep", tmp), "0");
			if (wlif_num == 3)
			nvram_set(strcat_r(pfcred2, "wep", tmp), "0");
		} else {
			nvram_set(strcat_r(prefix, "_wep_x", tmp), "0");
			if (!wps_configured)
			for (i = 0; i < wlif_num; i++) {
				if (i == unit) continue;
				snprintf(prefix2, sizeof(prefix2), "wl%d", i);
				nvram_set(strcat_r(prefix2, "_wep_x", tmp), "0");
			}
		}

		nvram_set(strcat_r(prefix, "_auth", tmp), "0");
		if (!wps_configured)
		for (i = 0; i < wlif_num; i++) {
			if (i == unit) continue;
			snprintf(prefix2, sizeof(prefix2), "wl%d", i);
			nvram_set(strcat_r(prefix2, "_auth", tmp), "0");
		}

		ret = 0;
	}

	// crypto
	snprintf(nv_name, sizeof(nv_name), "%s_crypto", prefix);
	val = "";
	switch (creds->encr) {
		case WLIF_WPA_ENCR_TKIP:
			val = "tkip";
		break;

		case WLIF_WPA_ENCR_AES:
			val = "aes";
		break;

		case (WLIF_WPA_ENCR_TKIP | WLIF_WPA_ENCR_AES):
			val = "tkip+aes";
		break;
	}
	//if (!nvram_match(nv_name, val))
	{
		nvram_set(nv_name, val);
		if (!wps_configured)
		for (i = 0; i < wlif_num; i++) {
			if (i == unit) continue;
			snprintf(prefix2, sizeof(prefix2), "wl%d", i);
			nvram_set(strcat_r(prefix2, "_crypto", tmp), val);
		}

		if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
			if (creds->encr == (WLIF_WPA_ENCR_TKIP | WLIF_WPA_ENCR_AES))
				val = "aes";

			nvram_set(strcat_r(pfcred, "crypto", tmp), val);
			nvram_set(strcat_r(pfcred0, "crypto", tmp), val);
			nvram_set(strcat_r(pfcred1, "crypto", tmp), val);
			if (wlif_num == 3)
			nvram_set(strcat_r(pfcred2, "crypto", tmp), val);
		}

		ret = 0;
	}

	snprintf(nv_name, sizeof(nv_name), "%s_wpa_psk", prefix);
	//if (!nvram_match(nv_name, creds->nw_key))
	{
		nvram_set(nv_name, creds->nw_key);
		if (!wps_configured)
		for (i = 0; i < wlif_num; i++) {
			if (i == unit) continue;
			snprintf(prefix2, sizeof(prefix2), "wl%d", i);
			nvram_set(strcat_r(prefix2, "_wpa_psk", tmp), creds->nw_key);
		}

		if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr")) {
			nvram_set(strcat_r(pfcred, "wpa_psk", tmp), creds->nw_key);
			nvram_set(strcat_r(pfcred0, "wpa_psk", tmp), creds->nw_key);
			nvram_set(strcat_r(pfcred1, "wpa_psk", tmp), creds->nw_key);
			if (wlif_num == 3)
			nvram_set(strcat_r(pfcred2, "wpa_psk", tmp), creds->nw_key);
		}
		ret = 0;
	}

	if (!ret) {
		if (!wps_configured) {
			nvram_set("w_Setting", "1");
		}

#ifdef RTCONFIG_HND_ROUTER_AX
		if (nvram_get_int("amesh_wps_enr") || nvram_get_int("rpx_wps_enr"))
#endif
		{
			if (nvram_get_int("wps_enr_hw") == 1)
				nvram_set("x_Setting", "1");
#ifdef RTCONFIG_AMAS
			nvram_set("obd_Setting", "1");
#endif
		}
		nvram_commit();
	}

#ifdef RTCONFIG_HND_ROUTER_AX
	if (nvram_get_int("rpx_wps_enr") && !nvram_match("chk_wpsnv", "1")) {
		_dprintf("rp wps setting applied, reboot...\n");
		sleep(1);
		kill(1, SIGTERM);
	}
#endif

	return ret;
}

/* Parses the wpa supplicant config file to save the network credentials received from wps */
int
wl_wlif_parse_wpa_config_file(char *prefix, wlif_wps_nw_creds_t *creds)
{
	FILE *fp;
	int sz = 0, skip = 1;
	char buf[256], *ptr = NULL, *val = NULL;

	if (!prefix || !creds) {
		cprintf("Err: shared %s prefix and credentials can not be null \n", __func__);
		return -1;
	}

	snprintf(buf, sizeof(buf), "/tmp/%s_wpa_supplicant.conf", prefix);

	fp = fopen(buf, "r");
	if (fp == NULL) {
		cprintf("Err: shared %s failed to open file = [%s] \n", __func__, buf);
		return -1;
	}

	sz = sizeof(buf) - 1;
	while (!feof(fp)) {
		if (fgets(buf, sz, fp)) {
			buf[strcspn(buf, "\r\n")] = 0;

			// We need to parse only network block other part we can skip.
			if (!strcmp(buf, "network={")) {
				skip = 0;
				memset(creds, 0, sizeof(*creds));
			}
			if (!strcmp(buf, "}") && !skip) {
				skip = 1;
			}

			if (skip) {
				continue;
			}
			ptr = strtok_r(buf, "=", &val);
			if (!ptr || !val) {
				continue;
			}
			/* Since after successful wps session the network settings will be updated
			 * to the last network block. So here we are overwriting the settings.
			 */
			wl_wlif_save_wpa_settings(ptr, val, creds);
		}
	}

	fclose(fp);

	return 0;
}

/* Returns configuration state of ap */
bool
wl_wlif_does_ap_needs_to_be_configured(char *ifname)
{
	char *lan_ifnames = nvram_safe_get("lan_ifnames");
	char *lan1_ifnames = nvram_safe_get("lan1_ifnames");

	if (find_in_list(lan_ifnames, ifname)) {
		if (nvram_match("lan_wps_oob", "disabled")) {
			return FALSE;
		}
	} else if (find_in_list(lan1_ifnames, ifname)) {
		if (nvram_match("lan1_wps_oob", "disabled")) {
			return FALSE;
		}
	}

	return TRUE;
}

/* Sets the AP state to configured */
void
wl_wlif_set_ap_as_configured(char *ifname)
{
	char *lan_ifnames = nvram_safe_get("lan_ifnames");
	char *lan1_ifnames = nvram_safe_get("lan1_ifnames");

	if (find_in_list(lan_ifnames, ifname)) {
		nvram_set("lan_wps_oob", "disabled");
	} else if (find_in_list(lan1_ifnames, ifname)) {
		nvram_set("lan1_wps_oob", "disabled");
	}

	nvram_commit();
}

/* Saves the ap settings updated by hostapd  */
static void
wl_wlif_save_hapd_settings(char *type, char *val, wlif_wps_nw_creds_t *creds)
{
	if (!strcmp(type, "ssid")) {
		WLIF_STRNCPY(creds->ssid, val, sizeof(creds->ssid));
	} else if (!strcmp(type, "passphrase")) {
		WLIF_STRNCPY(creds->nw_key, val, sizeof(creds->nw_key));
	} else if (!strcmp(type, "key_mgmt")) {
		if (strstr(val, "WPA-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK;
		}
		if (strstr(val, "WPA2-PSK")) {
			creds->akm |= WLIF_WPA_AKM_PSK2;
		}
	} else if (!strcmp(type, "rsn_pairwise_cipher")) {
		if (strstr(val, "TKIP")) {
			creds->encr = WLIF_WPA_ENCR_TKIP;
		}
		if (strstr(val, "CCMP")) {
			creds->encr = WLIF_WPA_ENCR_AES;
		}
	}
}

/* Parse hostapd config which gets updated after wps completion */
int
wl_wlif_parse_hapd_config(char *ifname, wlif_wps_nw_creds_t *creds)
{
	FILE *fp;
	int sz = 0;
	char cmd[WLIF_MIN_BUF], *ptr = NULL, *val = NULL;

	if (!ifname || !creds) {
		cprintf("Err: shared %s prefix and credentials can not be null \n", __func__);
		return -1;
	}

	snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s -i %s get_config", WLIF_HAPD_DIR, ifname);

	fp = popen(cmd, "r");
	if (fp == NULL) {
		dprintf("Err: shared %s failed to open cmd = [%s] \n", __func__, cmd);
		return -1;
	}

	memset(creds, 0, sizeof(*creds));
	while (!feof(fp)) {
		char buf[WLIF_MIN_BUF] = {0};
		sz = sizeof(buf) - 1;
		if (fgets(buf, sz, fp)) {
			buf[strcspn(buf, "\r\n")] = 0;

			ptr = strtok_r(buf, "=", &val);
			if (!ptr || !val) {
				continue;
			}
			wl_wlif_save_hapd_settings(ptr, val, creds);
		}
	}

	pclose(fp);

	return 0;
}

// Routine to create a joinable/detached thread
int
wl_wlif_create_thrd(pthread_t *thread_id, wlif_thrd_func fptr, void *arg, bool is_detached)
{
	pthread_attr_t attr;
	int ret = -1;

	if (!fptr || !thread_id) {
		cprintf("Err : shared %s invalid thread params \n", __func__);
		goto exit;
	}

	if (is_detached) {
		ret = pthread_attr_init(&attr);
		if (ret != 0) {
			cprintf("Err : shared %s pthread_attr_init failed \n", __func__);
			goto exit;
		}

		ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (ret != 0) {
			cprintf("Err : shared %s %d shared pthread_attr_setdetachstat failed\n",
				__func__, __LINE__);
			pthread_attr_destroy(&attr);
			goto exit;
		}
	}

	if ((ret = pthread_create(thread_id, is_detached ? &attr : NULL, fptr, arg)) != 0) {
		cprintf("Err : shared %s %d pthread_create failed \n", __func__, __LINE__);
	}

	if (is_detached) {
		pthread_attr_destroy(&attr);
	}

exit:
	return ret;
}

/* Invokes the hostapd/wpa_supplicant pbc cli to the provided interface.
 * In case caller does specify any interface, first wireless interface will be taken
 * from the lan_ifnames nvram for wps operation.
 * If bh_ifname is non null than the backhaul credentials will be updated with the
 * settings of bh_ifname.
 */
int
wl_wlif_wps_pbc_hdlr(char *wps_ifname, char *bh_ifname)
{
	char cmd[WLIF_MAX_BUF];
	char mode[WLIF_MIN_BUF] = {0};
	char nvifname[IFNAMSIZ] = {0};
	wlif_wps_ui_status_code_id_t status_code;
	int ret = -1;

	if (!wps_ifname) {
		cprintf("Err: shared %s ifname can not be null\n", __func__);
		goto end;
	}

	if (bh_ifname) {
		cprintf("Info: shared inside %s wps ifname %s backhaul ifname %s\n",
			__func__, wps_ifname, bh_ifname);
	} else {
		cprintf("Info: shared inside %s wps ifname %s\n", __func__, wps_ifname);
	}

#ifdef MULTIAP
	if (bh_ifname) {
		wl_wlif_update_hapd_bh_creds(wps_ifname, bh_ifname);
	}
#endif	/* MULTIAP */

	if (osifname_to_nvifname(wps_ifname, nvifname, sizeof(nvifname))) {
		cprintf("Err: shared %s osifname_to_nvifname failed for %s\n",
			__func__, wps_ifname);
		goto end;
	}
	snprintf(mode, sizeof(mode), "%s_mode", nvifname);
	if (nvram_match(mode, "ap")) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s -i %s wps_pbc",
			WLIF_HAPD_DIR, wps_ifname);
		status_code = WLIF_WPS_UI_FINDING_PBC_STA;
	} else {
		/* Before starting WPS set ap_scan parameter to 1 in supplicant */
		wl_wlif_wpa_supplicant_update_ap_scan(wps_ifname, nvifname, 1);

		snprintf(cmd, sizeof(cmd), "%s -p /var/run/"
			"%s_wpa_supplicant -i %s wps_pbc", WPA_CLI_APP, nvifname, wps_ifname);
		status_code = WLIF_WPS_UI_FIND_PBC_AP;
	}

	if ((ret = system(cmd)) != 0) {
		cprintf("Err: shared %s cli cmd %s failed for interface %s in %s mode\n", __func__,
			cmd, wps_ifname, nvram_safe_get(mode));
	}

	wl_wlif_update_wps_ui(status_code);
end:
	return ret;
}

// Stops the ongoing wps session for the interface provided in wps_ifname
int
wl_wlif_wps_stop_session(char *wps_ifname)
{
	char cmd[WLIF_MAX_BUF] = {0};
	char mode[WLIF_MIN_BUF] = {0};
	char nvifname[IFNAMSIZ] = {0};
	int ret = -1;

	if (!wps_ifname) {
		cprintf("Err: shared %s ifname can not be null\n", __func__);
		goto end;
	}

	if ((ret = osifname_to_nvifname(wps_ifname, nvifname, sizeof(nvifname)))) {
		cprintf("Err: shared %s osifname_to_nvifname failed for %s ret = %d\n",
			__func__, wps_ifname, ret);
		goto end;
	}

	snprintf(mode, sizeof(mode), "%s_mode", nvifname);
	if (nvram_match(mode, "ap")) {
		snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s -i %s wps_cancel",
			WLIF_HAPD_DIR, wps_ifname);
	} else {
		snprintf(cmd, sizeof(cmd), "%s -p /var/run/"
			"%s_wpa_supplicant -i %s wps_cancel", WPA_CLI_APP, nvifname, wps_ifname);
	}

	if ((ret = system(cmd)) != 0) {
		dprintf("Info: shared %s cli cmd %s failed for interface %s ret = %d\n", __func__,
			cmd, wps_ifname, ret);
	}

	wl_wlif_update_wps_ui(WLIF_WPS_UI_INIT);
end:
	return ret;
}

// Update the ap_scan parameter for wpa-supplicant
static void
wl_wlif_wpa_supplicant_update_ap_scan(char *ifname, char *nvifname, int val)
{
	char cmd[WLIF_MIN_BUF] = {0};

	if (val < 0 || val > 2) {
		cprintf("Err: %s invalid value %d is provided for ap scan parameter\n",
			__func__, val);
		return;
	}

	snprintf(cmd, sizeof(cmd), "%s -p /var/run/%s_wpa_supplicant -i %s ap_scan %d",
		WPA_CLI_APP, nvifname, ifname, val);
	system(cmd);
}

#ifdef MULTIAP
/* Retrieves the backhaul credentials from the nvram */
static int
wl_wlif_fill_bh_creds_from_nvram(char *nvifname, wlif_bh_creds_hapd_clicmd_data_t *cmd)
{
	char *ptr, tmp[WLIF_MIN_BUF] = {0};
	char *next;
	bool psk_required = FALSE, sae = FALSE;

	snprintf(tmp, sizeof(tmp), "%s_wep", nvifname);
	ptr = nvram_safe_get(tmp);
	if (!strcmp(ptr, "enabled")) {
		cprintf("Info: shared %s wep is enabled for %s skiping update of "
			"backhaul credentials\n", __func__, nvifname);
		return -1;
	}

	snprintf(tmp, sizeof(tmp), "%s_ssid", nvifname);
	ptr = nvram_safe_get(tmp);
	WLIF_STRNCPY(cmd->ssid, ptr, sizeof(cmd->ssid));

	snprintf(tmp, sizeof(tmp), "%s_akm", nvifname);
	ptr = nvram_safe_get(tmp);
	foreach(tmp, ptr, next) {
		if (!strcmp(tmp, "psk")) {
			WLIF_STRNCPY(cmd->auth, "WPAPSK", sizeof(cmd->encr));
			psk_required = TRUE;
		}

		if (!strcmp(tmp, "psk2")) {
			WLIF_STRNCPY(cmd->auth, "WPA2PSK", sizeof(cmd->encr));
			psk_required = TRUE;
		}

		if (!strcmp(tmp, "sae")) {
			WLIF_STRNCPY(cmd->auth, "WPA2PSK", sizeof(cmd->encr));
			sae = psk_required = TRUE;
		}
	}

	if (psk_required) {
		/* force crypto to CCMP for sae or sae-transition mode */
		if (sae) {
			WLIF_STRNCPY(cmd->encr, "CCMP", sizeof(cmd->encr));
		} else {
			snprintf(tmp, sizeof(tmp), "%s_crypto", nvifname);
			ptr = nvram_safe_get(tmp);
			if (!strcmp(ptr, "tkip")) {
				WLIF_STRNCPY(cmd->encr, "TKIP", sizeof(cmd->encr));
			} else if (!strcmp(ptr, "aes") || !strcmp(ptr, "tkip+aes")) {
				WLIF_STRNCPY(cmd->encr, "CCMP", sizeof(cmd->encr));
			} else {
				cprintf("Info: shared %s uknown crypto type (%s) for ifname %s\n",
					__func__, ptr, nvifname);
				return -1;
			}
		}

		snprintf(tmp, sizeof(tmp), "%s_wpa_psk", nvifname);
		ptr = nvram_safe_get(tmp);
		WLIF_STRNCPY(cmd->key, ptr, sizeof(cmd->key));
	} else {
		WLIF_STRNCPY(cmd->auth, "OPEN", sizeof(cmd->encr));
		WLIF_STRNCPY(cmd->encr, "NONE", sizeof(cmd->encr));
	}

	cprintf("Info: shared inside %s bh_ssid[%s] bh_auth[%s] bh_encr[%s] bh_psk[%s]\n",
		__func__, cmd->ssid, cmd->auth, cmd->encr, cmd->key);
	return 0;
}

/* Updates the hostapd backhaul credentials with settings of bh_ifname by
 * invoking hostapd wps_mapbh_config cli cmd.
 */
static void
wl_wlif_update_hapd_bh_creds(char *wps_ifname, char *bh_ifname)
{
	wlif_bh_creds_hapd_clicmd_data_t clidata;
	char nvifname[IFNAMSIZ] = {0};
	char cmd[WLIF_MIN_BUF * 2] = {0};

	cprintf("Info: shared inside %s wps_ifname %s bh_ifname %s\n",
		__func__, wps_ifname, bh_ifname);
	if (osifname_to_nvifname(bh_ifname, nvifname, sizeof(nvifname))) {
		cprintf("Info: shared %s osifname_to_nvifname failed for %s\n",
			__func__, bh_ifname);
		return;
	}

	memset(&clidata, 0, sizeof(clidata));
	if (wl_wlif_fill_bh_creds_from_nvram(nvifname, &clidata) == 0) {
		if (clidata.key[0] != '\0') {
			snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s -i %s wps_mapbh_config "
				"%s %s %s %s", WLIF_HAPD_DIR, wps_ifname, clidata.ssid,
				clidata.auth, clidata.encr, clidata.key);
		} else {
			snprintf(cmd, sizeof(cmd), "hostapd_cli -p %s -i %s wps_mapbh_config "
				"%s %s %s", WLIF_HAPD_DIR, wps_ifname, clidata.ssid,
				clidata.auth, clidata.encr);
		}

		if (system(cmd)) {
			cprintf("Info: shared %s cli cmd %s failed for interface %s\n", __func__,
				cmd, wps_ifname);
		}
	}
}

/* Fn to check whether present wps session for multiap onboarding */
bool
wl_wlif_is_map_onboarding(char *prefix)
{
	char map[WLIF_MIN_BUF] = {0}, *ptr = NULL;
	uint16 map_val = 0;
	bool ret;

	snprintf(map, sizeof(map), "%s_map", prefix);
	ptr = nvram_safe_get(map);
	if (ptr[0] != '\0') {
		map_val = (uint16)strtoul(ptr, NULL, 0);
	}

	ret = (map_val & WLIF_MAP_BHSTA_NVVAL) ? TRUE : FALSE;

	return ret;
}

/* Checks whether given interface is operabe on particular channel or not */
static bool
wl_wlif_is_ifr_operable_on_channel(char *ifname, uint8 channel)
{
	wl_uint32_list_t *list = NULL;
	uint32 buf[WL_NUMCHANNELS + 1] = {0};
	int idx = 0;

	list = (wl_uint32_list_t *)buf;
	list->count = htod32(WL_NUMCHANNELS);
	if (wl_ioctl(ifname, WLC_GET_VALID_CHANNELS, buf, sizeof(buf)) < 0) {
		cprintf("Err: shared %s ifname %s get valid channels iovar is failed \n",
			__func__, ifname);
		goto end;
	}

	list->count = dtoh32(list->count);

	for (idx = 0; idx < list->count; idx++) {
		if (channel == list->element[idx]) {
			return TRUE;
		}
	}

end:
	return FALSE;
}

/* Fn to get the channel from scanresults filtered using ssid */
static uint8
wl_wlif_get_channel_from_scanresults(char *ifname, char *ssid)
{
	wl_scan_results_t *list = NULL;
	wl_bss_info_t *bi;
	int idx, ret;
	char *buf;
	uint8 channel = 0;

	buf = calloc(1, WLIF_DUMP_BUF_LEN);
	if (buf == NULL) {
		cprintf("Err: shared %s calloc failed for len %d\n", __func__, WLIF_DUMP_BUF_LEN);
		return -1;
	}

	list = (wl_scan_results_t*)buf;
	list->buflen = htod32(WLIF_DUMP_BUF_LEN);

	ret = wl_ioctl(ifname, WLC_SCAN_RESULTS, list, WLIF_DUMP_BUF_LEN);
	if (ret) {
		dprintf("Err: shared %s ifname %s iovar call to get scan results failed ret = %d\n",
			__func__, ifname, ret);
		goto end;
	}

	list->count = dtoh32(list->count);
	if (list->count == 0) {
		dprintf("Info: shared %s ifname %s scan did not find any results for ssid %s \n",
			__func__, ifname, ssid);
		goto end;
	}

	bi = list->bss_info;
	for (idx = 0; idx < list->count; idx++) {
		if (!strcmp(ssid, (char *)bi->SSID)) {
			channel = wf_chspec_ctlchan(dtoh16(bi->chanspec));
			break;
		}
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}

end:
	free(buf);
	return channel;
}

/* Fn to issue the scan cmd */
static int
wl_wlif_scan(char *ifname, char *ssid)
{
	wl_scan_params_t* params;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + WL_NUMCHANNELS * sizeof(uint16);
	int ret = 0;

	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL) {
		cprintf("Err: shared %s malloc of %d bytes failed for scan params\n",
			__func__, params_size);
		return -1;
	}

	memset(params, 0, params_size);
	if (ssid != NULL && (strlen(ssid) < sizeof(params->ssid.SSID))) {
		memcpy(params->ssid.SSID, ssid, sizeof(params->ssid.SSID));
		params->ssid.SSID_len = strlen(ssid);
	}

	params->bss_type = DOT11_BSSTYPE_ANY;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = 0; // Do active scan
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	if ((ret = wl_ioctl(ifname, WLC_SCAN, params, params_size))) {
		cprintf("Err: shared %s ifname %s scan command for SSID %s failed ret = %d\n",
			__func__, ifname, ssid ? ssid : "empty", ret);
	}

	free(params);

	return ret;
}

/* Selects the backhaul sta interface from bss list.
 * 1: For each interface scan the backhaul ssid.
 * 2: Fetch the channel from the scanresults.
 * 3: Check whether the interface is operable on the channel if yes than copy it to bh_ifname.
 */
static void
wl_wlif_select_bhsta_from_bsslist(wlif_bss_list_t *bss_list, char *bh_ssid,
	char *bh_ifname, int bh_ifname_sz)
{
	int idx = 0;
	uint8 channel = 0;
	int val;

	for (idx = 0; idx < bss_list->count; idx++) {
		int count = 0;
		val = 0;
		wlif_bss_t *bss = &bss_list->bss[idx];

		wl_ioctl(bss->ifname, WLC_GET_BAND, &val, sizeof(val));

		while (count++ < WLIF_SCAN_TRY_COUNT) {
			char cmd[WLIF_MIN_BUF] = {0};
			/*
			 * Before doing wl scan set ap_scan parameter to 0 in supplicant. So that
			 * supplicant avoids performing join-scan.Additionally call abort_scan and
			 * disconnect to cancel any ongoing join-scan.
			 */
			wl_wlif_wpa_supplicant_update_ap_scan(bss->ifname, bss->nvifname, 0);
			snprintf(cmd, sizeof(cmd), "wpa_cli -p /var/run/%s_wpa_supplicant -i %s "
				"abort_scan", bss->nvifname, bss->ifname);
			system(cmd);
			snprintf(cmd, sizeof(cmd), "wpa_cli -p /var/run/%s_wpa_supplicant -i %s "
				"disconnect", bss->nvifname, bss->ifname);
			system(cmd);

			if (wl_wlif_scan(bss->ifname, bh_ssid)) {
				sleep(4);
				continue;
			}

			if (val == WLC_BAND_6G) {
				/* full scan on 6G's 59 channels...
				 * TODO: explore RNR option for directed scan
				 */
				sleep(10);
			} else {
				sleep(3);
			}
			channel = wl_wlif_get_channel_from_scanresults(bss->ifname, bh_ssid);
			if (channel > 0) {
				break;
			}
		}

		if (channel <= 0) {
			cprintf("Info: shared %s did not find any scanresults for ssid %s "
				"in interface %s \n", __func__, bh_ssid, bss->ifname);
			continue;
		}
		if (wl_wlif_is_ifr_operable_on_channel(bss->ifname, channel)) {
			WLIF_STRNCPY(bh_ifname, bss->ifname, bh_ifname_sz);
			break;
		}
	}
}

/* Gets the candidate multiap backhaul stations list from the list of interfaces */
static void
wl_wlif_map_get_candidate_bhsta_bsslist(char *list, wlif_bss_list_t *bss_list, char *skip_ifname)
{
	char ifname[IFNAMSIZ] = {0}, os_name[IFNAMSIZ] = {0}, tmp[WLIF_MIN_BUF] = {0};
	char *next = NULL, *ptr = NULL;
	char prefix[] = "wlxxxxxxxxxxxxxx_";

	if (!list || !bss_list) {
		return;
	}

	foreach(ifname, list, next) {
		uint16 map = 0;
		wlif_bss_t *bss;

		if (bss_list->count == WL_MAXBSSCFG) {
			continue;
		}

		if (nvifname_to_osifname(ifname, os_name, sizeof(os_name))) {
			continue;
		}

		if (wl_probe(os_name)) {
			continue;
		}

		if (osifname_to_nvifname(os_name, prefix, sizeof(prefix))) {
			continue;
		}

		// Skip adding already present interface
		if (skip_ifname && strcmp(skip_ifname, os_name) == 0) {
			continue;
		}

		snprintf(tmp, sizeof(tmp), "%s_%s", prefix, "mode");
		if (!nvram_match(tmp, "sta")) {
			continue;
		}

		snprintf(tmp, sizeof(tmp), "%s_%s", prefix, "map");
		ptr = nvram_safe_get(tmp);
		if (ptr[0] != '\0') {
			map = (uint16) strtoul(ptr, NULL, 0);
		}
		if (!(map & WLIF_MAP_BHSTA_NVVAL)) {
			continue;
		}

		bss = &bss_list->bss[bss_list->count];
		WLIF_STRNCPY(bss->ifname, os_name, sizeof(bss->ifname));
		WLIF_STRNCPY(bss->nvifname, prefix, sizeof(bss->nvifname));
		bss_list->count++;
	}
}

/* Apply the network credentials received from wps session to the bss */
static void
wl_wlif_apply_map_backhaul_creds(wlif_bss_t *bss, wlif_wps_nw_creds_t *creds)
{
	char nv_name[WLIF_MIN_BUF] = {0};
	char *val = "", *prefix;
	bool wps_v2 = FALSE, sae_transition_mode = FALSE;
	char bh_sta_list[NVRAM_MAX_VALUE_LEN] = {0};

	prefix = bss->nvifname;

	/* ssid */
	snprintf(nv_name, sizeof(nv_name), "%s_ssid", prefix);
	nvram_set(nv_name, creds->ssid);

	val = nvram_safe_get("wps_version2");
	if (!strcmp(val, "enabled")) {
		wps_v2 = TRUE;
	}

	/* for wps version 2 force psk2 if psk1 is provided. */
	if (wps_v2 == TRUE) {
		if (creds->akm & WLIF_WPA_AKM_PSK) {
			creds->akm |= WLIF_WPA_AKM_PSK2;
		}
		if (creds->encr & WLIF_WPA_ENCR_TKIP) {
			creds->encr |= WLIF_WPA_ENCR_AES;
		}
	}

	/* akm */
	val = "";
	snprintf(nv_name, sizeof(nv_name), "%s_akm", prefix);
	switch (creds->akm) {
	case WLIF_WPA_AKM_PSK:
		val = "psk";
		break;

	case WLIF_WPA_AKM_PSK2:
		val = "psk2 sae";
		sae_transition_mode = TRUE;
		break;

	case WLIF_WPA_AKM_PSK | WLIF_WPA_AKM_PSK2:
		val = "psk psk2";
		break;

	default:
		dprintf("Received akm %d \n", creds->akm);
		break;
	}
	nvram_set(nv_name, val);

	/* crypto */
	snprintf(nv_name, sizeof(nv_name), "%s_crypto", prefix);
	val = sae_transition_mode ? "aes" : "";	/* Force crypto to aes for sae transition mode */
	if (!sae_transition_mode) {
		switch (creds->encr) {
		case WLIF_WPA_ENCR_TKIP:
			val = "tkip";
			break;

		case WLIF_WPA_ENCR_AES:
			val = "aes";
			break;

		case WLIF_WPA_ENCR_TKIP | WLIF_WPA_ENCR_AES:
			val = "tkip+aes";
			break;

		default:
			dprintf("Received encr %d \n", creds->encr);
			break;
		}
	}
	nvram_set(nv_name, val);

	/* wpa-psk */
	snprintf(nv_name, sizeof(nv_name), "%s_wpa_psk", prefix);
	nvram_set(nv_name, creds->nw_key);

	/* Add ifname to map_bhsta_ifnames, if not already */
	val = nvram_safe_get("map_bhsta_ifnames");
	if (!find_in_list(val, bss->ifname)) {
		strncpy_n(bh_sta_list, val, sizeof(bh_sta_list));
		/* If add_to_list is sucessfull set the NVRAM */
		if (add_to_list(bss->ifname, bh_sta_list, sizeof(bh_sta_list)) == 0) {
			nvram_set("map_bhsta_ifnames", bh_sta_list);
		}
	}

	nvram_commit();
}

/* Configuration of multiap backhaul station interface from the credentials received using wps
 * 1: Get the candidate multiap backhaul sta list from the lan_ifnames nvram.
 * 2: From the above list select backhaul sta based on the backhaul ssid using scan results.
 * 3: Apply the setting to the selected interface.
 */
int
wl_wlif_map_configure_backhaul_sta_interface(wlif_bss_t *bss_in, wlif_wps_nw_creds_t *creds)
{
	char *list = nvram_safe_get("lan_ifnames");
	wlif_bss_list_t	bss_list;
	wlif_bss_t *bss = NULL;
	int idx = 0, ret = -1;
	char bh_ifname[IFNAMSIZ] = {0};
	char tmp[WLIF_MIN_BUF] = {0};
	char *skip_ifname = NULL;

	if (creds->ssid[0] == '\0') {
		cprintf("Err: shared %s empty backhaul ssid received after wps\n", __func__);
		return ret;
	}

	memset(&bss_list, 0, sizeof(bss_list));

	// In the list add first entry for the bss where wps is performed
	if (bss_in != NULL) {
		bss = &bss_list.bss[bss_list.count];
		memcpy(bss, bss_in, sizeof(*bss));
		bss_list.count++;
		skip_ifname = bss_in->ifname;
	}

	wl_wlif_map_get_candidate_bhsta_bsslist(list, &bss_list, skip_ifname);
	wl_wlif_select_bhsta_from_bsslist(&bss_list, creds->ssid, bh_ifname, sizeof(bh_ifname));
	if (bh_ifname[0] != '\0') {
		cprintf("Info: shared %s selected backhaul station ifname "
			" %s for backhaul ssid %s\n", __func__, bh_ifname, creds->ssid);
		for (idx = 0; idx < bss_list.count; idx++) {
			bss = &bss_list.bss[idx];
			if (!strcmp(bss->ifname, bh_ifname)) {
				// apply the settings received from wps;
				wl_wlif_apply_map_backhaul_creds(bss, creds);
				nvram_set("map_onboarded", "1");
				nvram_unset("wps_on_sta");
			} else {
				// uneset  the map settings and change mode from sta to AP
				nvram_unset(strcat_r(bss->nvifname, "_map", tmp));
				nvram_set(strcat_r(bss->nvifname, "_mode", tmp), "ap");
			}
		}
		ret = 0;
	} else {
		cprintf("Err: shared %s multiap backhaul ifname not found for "
			"backhaul ssid = %s \n", __func__, creds->ssid);
		// In case of failure restore ap_scan parameter to 1 in supplicant.
		for (idx = 0; idx < bss_list.count; idx++) {
			bss = &bss_list.bss[idx];
			wl_wlif_wpa_supplicant_update_ap_scan(bss->ifname, bss->nvifname, 1);
		}
	}

	return ret;
}

/* Returns the count of the wps timeout value for multiap onboarding based on below criteria:
 * If wps_custom_ifnames nvram is present and there are more than 1 backhaul sta interfaces
 * present in it. Timeout will be complete wps session timeout(120) divided by the backhaul
 * sta count. This value can be overwritten by setting wps_map_timeout nvram.
 */
int
wl_wlif_wps_map_timeout()
{
	int timeout = -1, count = 0;
	char ifname[IFNAMSIZ] = {0}, *next, nvifname[IFNAMSIZ];
	char tmp[WLIF_MIN_BUF] = {0}, *mode, *map;
	char *list = nvram_safe_get("wps_custom_ifnames");
	char *map_timeout = nvram_safe_get("wps_map_timeout");
	char *multiap_mode = nvram_safe_get("multiap_mode");

	if (multiap_mode[0] == '\0') {
		goto end;
	}

	if ((list[0] == '\0') || (strtoul(multiap_mode, NULL, 0) <= 0)) {
		goto end;
	}

	foreach(ifname, list, next) {
		if (wl_probe(ifname)) {
			continue;
		}

		if (osifname_to_nvifname(ifname, nvifname, sizeof(nvifname))) {
			continue;
		}

		snprintf(tmp, sizeof(tmp), "%s_map", nvifname);
		map = nvram_safe_get(tmp);

		snprintf(tmp, sizeof(tmp), "%s_mode", nvifname);
		mode = nvram_safe_get(tmp);

		// Check mode is sta and map value is set to multiap backhaul sta
		if (!strcmp(mode, "sta") && map[0] != '\0' &&
			(strtoul(map, NULL, 0) == WLIF_MAP_BHSTA_NVVAL)) {
			count++;
		}
	}

	if (count > 1) {
		timeout = WLIF_WPS_TIMEOUT/count;
		if (map_timeout[0] != '\0') {
			timeout = atoi(map_timeout);
		}
	}

end:
	dprintf("Info: Shared %s the multiap timeout is %d \n", __func__, timeout);
	return timeout;
}
#endif	/* MULTIAP */
#endif	/* CONFIG_HOSTAPD */
