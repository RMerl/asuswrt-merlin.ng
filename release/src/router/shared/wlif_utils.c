/*
 * Wireless interface translation utility functions
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
 * $Id: wlif_utils.c 669471 2016-11-09 17:01:43Z $
 */

#include <typedefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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
#include <wlif_utils.h>

#include <stdarg.h>   // for va_list
#include "shared.h"

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

/* From wlc_rate.[ch] */
#define MCS_TABLE_SIZE				33

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
	char *wl_hw;
	int i, j;

	ether_etoa(mac, eabuf);
	/* find out the wl name from mac */
	for (i = 0; i < MAX_NVPARSE; i++) {
		sprintf(wlname, "wl%d", i);
		sprintf(tmptr, "wl%d_hwaddr", i);
		wl_hw = nvram_get(tmptr);
		if (wl_hw) {
			if (!strncasecmp(wl_hw, eabuf, sizeof(eabuf)))
				return 0;
		}

		for (j = 1; j < WL_MAXBSSCFG; j++) {
			sprintf(wlname, "wl%d.%d", i, j);
			sprintf(tmptr, "wl%d.%d_hwaddr", i, j);
			wl_hw = nvram_get(tmptr);
			if (wl_hw) {
				if (!strncasecmp(wl_hw, eabuf, sizeof(eabuf)))
					return 0;
			}
		}
	}

	return -1;
}

bool
wl_wlif_is_psta(char *ifname)
{
	int32 psta = FALSE;

	if (wl_probe(ifname) < 0)
		return FALSE;

	if (wl_iovar_getint(ifname, "psta_if", &psta) < 0)
		return FALSE;

	return psta ? TRUE : FALSE;
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
	}

	/* find for wan  */
	ifnames = nvram_get("wan_ifnames");
	ifname = nvram_get("wan0_ifname");
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
#endif
