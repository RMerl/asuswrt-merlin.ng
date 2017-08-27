/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Wi-Fi Blanket shared functions
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
 * $Id: wbd_rc_shared.c 670128 2016-11-14 12:15:13Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <typedefs.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmparams.h>
#include <proto/ethernet.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <common_utils.h>

#include "wbd_rc_shared.h"

/* Wi-Fi Blanket Buffer Lengths */
#define WBD_MAX_BUF_256			256

/* General NVRAMs */
#define WBD_NVRAM_IFNAMES		"wbd_ifnames"
#define NVRAM_BR0_IFNAMES		"br0_ifnames"
#define NVRAM_LAN_IFNAMES		"lan_ifnames"
#define NVRAM_LAN1_IFNAMES		"lan1_ifnames"
#define NVRAM_MODE			"mode"
#define NVRAM_DWDS			"dwds"
#define NVRAM_UNIT			"unit"
#define NVRAM_BSS_ENABLED		"bss_enabled"
#define NVRAM_HWADDR			"hwaddr"
#define NVRAM_IFNAME			"ifname"
#define NVRAM_VIFS			"vifs"
#define WBD_NVRAM_FIXED_IFNAMES		"wbd_fixed_ifnames"
#define WBD_NVRAM_NO_DEDICATED_BACKHAUL	"wbd_no_dedicated_backhaul"

/* WBD Errors */
#define WBDE_OK			0
#define WBDE_WL_ERROR		-11	/* WL IOVAR error */
#define WBDE_DWDS_AP_VIF_EXST	-30	/* Virtual interface already up */
#define WBDE_DWDS_STA_PIF_NEXST	-31	/* No DWDS primary Ifr with mode STA */
#define WBDE_WBD_IFNAMES_FULL	-32	/* wbd_ifnames have 2G & 5G ifnames */
#define WBDE_WBD_IFNAMES_NEXST	-33	/* wbd_ifnames NVRAM not defined */
#define WBDE_INV_IFNAME		-81	/* Invalid interface name */

#ifndef WBDSTRNCPY
#define WBDSTRNCPY(dst, src, len)	 \
	do { \
		strncpy(dst, src, len -1); \
		dst[len - 1] = '\0'; \
	} while (0)
#endif

#ifdef WBD_RC_PRINT_ENB
#define WBD_RC_PRINT(fmt, arg...) \
	printf("WBD-RC-SHARED >> %s(%d) : "fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define WBD_RC_PRINT(fmt, arg...)
#endif /* WBD_RC_PRINT_ENB */

/* Extern Declarations */
extern char* strncpy_n(char *destination, const char *source, size_t num);
extern void nvram_initialize_all(char *prefix);

/* Convert Ethernet address string representation to binary data */
static int
wbd_ether_atoe(const char *a, struct ether_addr *n)
{
	char *c = NULL;
	int i = 0;

	memset(n, 0, ETHER_ADDR_LEN);
	for (;;) {
		n->octet[i++] = (uint8)strtoul(a, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
		a = c;
	}

	return (i == ETHER_ADDR_LEN);
}

/* WBD version to get the NVRAM value. If we run this in PC it will just return NULL */
static char*
wbd_nvram_safe_get(const char *nvram)
{
	return nvram_safe_get(nvram);
}

/* WBD version to get the NVRAM value for specific BSS prefix */
static char*
wbd_nvram_prefix_safe_get(const char *prefix, const char *nvram)
{
	char data[WBD_MAX_BUF_256] = {0};
	memset(data, 0, sizeof(data));

	if (prefix) {
		return nvram_safe_get(strcat_r(prefix, nvram, data));
	} else {
		return nvram_safe_get(nvram);
	}
}

/* Gets the config val from NVARM, if not found applies the default value */
static uint16
wbd_nvram_safe_get_int(char* prefix, const char *c, uint16 def)
{
	char *val = NULL;
	uint16 ret = def;

	if (prefix) {
		val = wbd_nvram_prefix_safe_get(prefix, c);
	} else {
		val = wbd_nvram_safe_get(c);
	}

	if (val && (val[0] != '\0')) {
		ret = strtoul(val, NULL, 0);
	} else {
		WBD_RC_PRINT("NVRAM[%s%s] is not defined\n", (prefix ? prefix : ""), c);
	}

	return ret;
}

/* WBD version to set the NVRAM value for specific BSS prefix */
static int
wbd_nvram_prefix_set(const char *prefix, const char *nvram, const char *nvramval)
{
	char data[WBD_MAX_BUF_256] = {0};
	memset(data, 0, sizeof(data));

	if (prefix) {
		return nvram_set(strcat_r(prefix, nvram, data), nvramval);
	} else {
		return nvram_set(nvram, nvramval);
	}
}

/* Check if Interface and its Primary Radio is enabled or not */
static int
wbd_is_ifr_enabled(char *ifname, bool validate_vif, int *error)
{
	int ret = WBDE_OK, unit = 0, ifr_enabled = 0;
	char buf[WBD_MAX_BUF_256] = {0};

	/* Check interface (fail for non-wl interfaces) */
	if ((ret = wl_probe(ifname))) {
		ret = WBDE_INV_IFNAME;
		goto end;
	}

	/* Get instance */
	ret = wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
	if (ret < 0) {
		/* printf("%s : Failed to %s, WL error : %d\n", ifname, "get instance", ret); */
		ret = WBDE_WL_ERROR;
		goto end;
	}

	/* Check if Primary Radio of given Interface is enabled or not */
	snprintf(buf, sizeof(buf), "wl%d_radio", unit);
	ifr_enabled = atoi(nvram_safe_get(buf));

	if (!ifr_enabled) {
		goto end;
	}

	/* Check if interface is vifs */
	if ((validate_vif) && (strncmp(ifname, "wl", 2) == 0)) {
		/* Check if vifs is enabled or not */
		snprintf(buf, sizeof(buf), "%s_bss_enabled", ifname);
		ifr_enabled = (atoi(nvram_safe_get(buf)));
	}
end:
	if (error) {
		*error = ret;
	}
	return ifr_enabled;
}

/* Get wlX_ or wlX.y_ Prefix from OS specific interface name */
static int
wbd_get_prefix(char *ifname, char *prefix, int prefix_len)
{
	int ret = WBDE_OK;
	char wl_name[IFNAMSIZ] = {0};

	/* Convert eth name to wl name - returns 0 if success */
	ret = osifname_to_nvifname(ifname, wl_name, sizeof(wl_name));
	if (ret != WBDE_OK) {
		ret = WBDE_INV_IFNAME;
		goto end;
	}

	/* Get prefix of the interface from Driver */
	make_wl_prefix(prefix, prefix_len, 1, wl_name);
	/* printf("Interface: %s, wl_name: %s, Prefix: %s\n", ifname, wl_name, prefix); */

end:
	return ret;
}

/* Check if Interface is DWDS Virtual Interface, if Disabled, Enable it */
int
wbd_enable_dwds_ap_vif(char *ifname)
{
	int ret = WBDE_OK, vif_enabled, unit, subunit;
	char prefix[IFNAMSIZ] = {0};

	/* Check if interface is Primary Interface */
	if (strncmp(ifname, "wl", 2) != 0) {
		goto end; /* get out */
	}

	/* Get Prefix from OS specific interface name */
	wbd_get_prefix(ifname, prefix, sizeof(prefix));

	/* Check if Virtual AP Interface is Enabled */
	vif_enabled = atoi(wbd_nvram_prefix_safe_get(prefix, NVRAM_BSS_ENABLED));

	if (!vif_enabled) {
		/* Get Unit & Subunit of Virtual Interface */
		sscanf(ifname, "wl%d.%d", &unit, &subunit);

		/* Create & Configure Virtual AP Interface, for DWDS Slave */
		wbd_create_dwds_ap_vif(unit, subunit);
	}

end:
	return ret;
}

/* Check if Interface is DWDS Primary Interface, with mode = STA, Change name1 */
int
wbd_check_dwds_sta_primif(char *ifname, char *ifname1, int len1)
{
	int ret = WBDE_DWDS_STA_PIF_NEXST, dwds, sta_mode, unit, subunit = 1;
	char *val, prefix[IFNAMSIZ] = {0};

	/* Check if interface is Virtual Interface */
	if (strncmp(ifname, "wl", 2) == 0) {
		goto end; /* get out */
	}

	/* Get Prefix from OS specific interface name */
	wbd_get_prefix(ifname, prefix, sizeof(prefix));

	/* Check if on Primary Interface DWDS in ON */
	dwds = atoi(wbd_nvram_prefix_safe_get(prefix, NVRAM_DWDS));

	/* Check if on Primary Interface Mode is STA */
	val = wbd_nvram_prefix_safe_get(prefix, NVRAM_MODE);
	sta_mode = (strcmp(val, "sta") == 0) ? 1 : 0;

	/* printf("%sdwds = %d, sta_mode = %d\n", prefix, dwds, sta_mode); */
	/* if DWDS in ON, mode is STA */
	if (dwds && sta_mode) {

		/* Get Unit of Primary Interface */
		unit = atoi(wbd_nvram_prefix_safe_get(prefix, NVRAM_UNIT));

		/* Get next available Virtual AP Interface for DWDS Slave */
		subunit = wbd_get_dwds_ap_vif_subunit(unit, &ret);

		/* Change Virtual AP Interface ifname */
		snprintf(ifname1, len1, "wl%d.%d", unit, subunit);

		/* DWDS Primary Interface, with mode = STA found */
		ret = WBDE_OK;
	} else {
		/* Set Primary Interface ifname as actual name */
		WBDSTRNCPY(ifname1, ifname, len1);
	}

end:
	return ret;
}

/* Find First DWDS Primary Interface, with mode = STA */
int
wbd_find_dwds_sta_primif(char *ifname, int len, char *ifname1, int len1)
{
	int ret = WBDE_DWDS_STA_PIF_NEXST;
	char lan_ifnames[NVRAM_MAX_VALUE_LEN] = {0};
	char var_intf[IFNAMSIZ] = {0}, *next_intf;

	/* Get NVRAM value from "lan_ifnames" and save it */
	WBDSTRNCPY(lan_ifnames, wbd_nvram_safe_get(NVRAM_LAN_IFNAMES), sizeof(lan_ifnames) - 1);

	/* Traverse lan_ifnames for each ifname */
	foreach(var_intf, lan_ifnames, next_intf) {

		/* Copy interface name temporarily */
		WBDSTRNCPY(ifname, var_intf, len - 1);

		/* Find DWDS Primary Interface, with mode = STA */
		ret = wbd_check_dwds_sta_primif(ifname, ifname1, len1);
		if (ret == WBDE_OK) {
			break; /* if found, exit loop, get updated ifname, ifname1 */
		}
	}

	return ret;
}

/* Add valid interfaces to "wbd_ifnames" and "wbd_ifnames1" arrays for 2G & 5G */
static int
add_ifr_to_wbd_ifnames(char *wbd_ifnames, int len, char *wbd_ifnames1, int len1,
	char *ifname, char *ifname1)
{
	int ret = WBDE_OK;

	/* Add ifname to wbd_ifnames */
	add_to_list(ifname, wbd_ifnames, len);

	/* Add ifname1 to wbd_ifnames1 */
	add_to_list(ifname1, wbd_ifnames1, len1);

	return ret;
}

/* Get "wbd_ifnames" from "lan_ifnames" */
/* "wbd_ifnames" : returns list of Primary Interfaces, used to set "wbd_ifnames" NVRAM */
/* "wbd_ifnames1" : returns list of Ifs, used by app to fetch WL infromation, can be Prim / VIFs */
int
wbd_ifnames_fm_lan_ifnames(char *wbd_ifnames, int len, char *wbd_ifnames1, int len1)
{
	int ret = WBDE_OK, no_dedicated_link = 0;
	char lan_ifnames[NVRAM_MAX_VALUE_LEN] = {0};
	char dwds_if[IFNAMSIZ] = {0}, name[IFNAMSIZ] = {0}, name1[IFNAMSIZ] = {0};
	char *next_intf, var_intf[IFNAMSIZ] = {0};

	memset(wbd_ifnames, 0, len);
	memset(wbd_ifnames1, 0, len1);

	/* Get the NVRAM which tells whether to have dedicated backhaul link or not */
	no_dedicated_link = wbd_nvram_safe_get_int(NULL, WBD_NVRAM_NO_DEDICATED_BACKHAUL, 0);

	/* If the number of interfaces is not more than two(not triband), disable dedicated
	 * backhaul
	 */
	if (wbd_count_interfaces() <= 2) {
		no_dedicated_link = 1;
	}

	/* Get NVRAM value from "lan_ifnames" and save it */
	WBDSTRNCPY(lan_ifnames, wbd_nvram_safe_get(NVRAM_LAN_IFNAMES), sizeof(lan_ifnames) - 1);

	/* Find First DWDS Primary Interface, with mode = STA */
	ret = wbd_find_dwds_sta_primif(name, sizeof(name), name1, sizeof(name1));
	if (ret == WBDE_OK) {
		/* Remember this DWDS interface to skip for next run */
		WBDSTRNCPY(dwds_if, name, sizeof(dwds_if) - 1);

		/* If dedicated link is not required, then don't add STA ifr */
		if (no_dedicated_link) {
			/* Add this interface to "wbd_ifnames" */
			add_ifr_to_wbd_ifnames(wbd_ifnames, len, wbd_ifnames1, len1, name, name1);
		} else {
			WBD_RC_PRINT("name[%s] no_dedicated_link[%d]. So, don't add STA ifr\n",
				name, no_dedicated_link);
		}
	}

	/* Traverse lan_ifnames for non-DWDS Primary ifnames */
	foreach(var_intf, lan_ifnames, next_intf) {

		/* Copy interface name temporarily */
		WBDSTRNCPY(name, var_intf, sizeof(name) - 1);

		/* Check if it is DWDS Primary Interface (already added) or Virtual Interface */
		if ((strncmp(name, "wl", 2) == 0) || (strcmp(name, dwds_if) == 0)) {
			continue; /* skip it, we are interested in non-DWDS Primary Ifr only */
		}

		/* Check if valid Wireless Interface and its Primary Radio is enabled or not */
		if (!wbd_is_ifr_enabled(name, FALSE, &ret)) {
			continue; /* Skip non-Wireless & disabled Wireless Interface */
		}

		/* Try to add this interface to "wbd_ifnames" */
		ret = add_ifr_to_wbd_ifnames(wbd_ifnames, len, wbd_ifnames1, len1, name, name);
	}

	return ret;
}

/* Get next available Virtual AP Interface for DWDS Slave */
int
wbd_get_dwds_ap_vif_subunit(int in_unit, int *error)
{
	int ret = WBDE_OK, unit, subunit, ret_subunit = 1;
	char lan_ifnames[NVRAM_MAX_VALUE_LEN] = {0};
	char name[IFNAMSIZ] = {0}, var_intf[IFNAMSIZ] = {0}, *next_intf;

	/* Get NVRAM value from "lan_ifnames" and save it */
	WBDSTRNCPY(lan_ifnames, wbd_nvram_safe_get(NVRAM_LAN_IFNAMES), sizeof(lan_ifnames) - 1);

	/* Traverse lan_ifnames for each ifname */
	foreach(var_intf, lan_ifnames, next_intf) {

		/* Re-Intialize flags */
		unit = 0, subunit = 0, ret_subunit = 1;

		/* Copy interface name temporarily */
		WBDSTRNCPY(name, var_intf, sizeof(name) - 1);

		/* Check if interface is Primary Interface */
		if (strncmp(name, "wl", 2) != 0) {
			continue; /* skip Primary Interface, as we are looking for vif */
		}

		/* Check if Virtual Interface is of our interest, if so, return this subunit */
		sscanf(name, "wl%d.%d", &unit, &subunit);
		if (unit == in_unit) {
			ret_subunit = subunit;
			/* printf("Interface : %s is enabled, don't create again\n", name); */
			break;
		}
	}

	if (error) {
		*error = ret;
	}
	return ret_subunit;
}

/* Create & Configure Virtual AP Interface, if WBD is ON and AP is WBD DWDS Slave */
/* Returns : WBDE_OK : if vif required to enable, gets enabled successfully */
/* Returns : WBDE_DWDS_AP_VIF_EXST : if vif required to enable, is already enabled */
int
wbd_create_dwds_ap_vif(int unit, int subunit)
{
	int ret = WBDE_OK, vif_enabled;
	char prim_nvifname[IFNAMSIZ] = {0};
	char prim_prefix[IFNAMSIZ] = {0}, prim_osifname[IFNAMSIZ] = {0};
	char vif_prefix[IFNAMSIZ] = {0}, vif_ifname[IFNAMSIZ] = {0};
	char vif_unit[IFNAMSIZ] = {0}, vif_hwaddr[ETHER_ADDR_LEN * 3] = {0};
	char interface_list[NVRAM_MAX_VALUE_LEN] = {0}, *str = NULL;
	int interface_list_size = sizeof(interface_list);
	struct ether_addr hwaddr;

	/* Create prim_prefix[wlX_] , prim_osifname[wlX] */
	snprintf(prim_nvifname, sizeof(prim_nvifname), "wl%d", unit);
	snprintf(prim_prefix, sizeof(prim_prefix), "wl%d_", unit);
	(void)nvifname_to_osifname(prim_nvifname, prim_osifname, sizeof(prim_osifname));

	/* Create vif_prefix[wlX.1_] , vif_ifname[wlX.1],  vif_unit[X.1], vif_hwaddr[MAC] */
	snprintf(vif_ifname, sizeof(vif_ifname), "wl%d.%d", unit, subunit);
	snprintf(vif_prefix, sizeof(vif_prefix), "wl%d.%d_", unit, subunit);
	snprintf(vif_unit, sizeof(vif_unit), "%d.%d", unit, subunit);
	str = wbd_nvram_prefix_safe_get(vif_prefix, NVRAM_HWADDR);
	strncpy_n(vif_hwaddr, str, sizeof(vif_hwaddr));

	/* Check if AP vif is already up or not */
	vif_enabled = atoi(wbd_nvram_prefix_safe_get(vif_prefix, NVRAM_BSS_ENABLED));
	str = wbd_nvram_prefix_safe_get(vif_prefix, NVRAM_IFNAME);

	/* if AP vif is already up, return */
	if ((vif_enabled) && (str) && (str[0] != '\0')) {
		ret = WBDE_DWDS_AP_VIF_EXST;
		goto end;
	}

	/* -- if AP vif is not up already, enable it  -- */
	/* Add interface to br0_ifnames, if not already */
	str = wbd_nvram_safe_get(NVRAM_BR0_IFNAMES);
	strncpy_n(interface_list, str, interface_list_size);
	ret = add_to_list(vif_ifname, interface_list, interface_list_size);
	if (ret == 0) {
		nvram_set(NVRAM_BR0_IFNAMES, interface_list);
	}

	/* Add interface to lan_ifnames, if not already */
	str = wbd_nvram_safe_get(NVRAM_LAN_IFNAMES);
	strncpy_n(interface_list, str, interface_list_size);
	ret = add_to_list(vif_ifname, interface_list, interface_list_size);
	if (ret == 0) {
		nvram_set(NVRAM_LAN_IFNAMES, interface_list);
	}

	/* Add interface to wlX_vifs, if not already */
	str = wbd_nvram_prefix_safe_get(prim_prefix, NVRAM_VIFS);
	strncpy_n(interface_list, str, interface_list_size);
	ret = add_to_list(vif_ifname, interface_list, interface_list_size);
	if (ret == 0) {
		wbd_nvram_prefix_set(prim_prefix, NVRAM_VIFS, interface_list);
	}

	/* Remove interface fm lan1_ifnames, if not already */
	str = wbd_nvram_safe_get(NVRAM_LAN1_IFNAMES);
	strncpy_n(interface_list, str, interface_list_size);
	ret = remove_from_list(vif_ifname, interface_list, interface_list_size);
	if (ret == 0) {
		nvram_set(NVRAM_LAN1_IFNAMES, interface_list);
	}

	/* Initialize other required NVRAMs to enable AP vif */
	nvram_initialize_all(vif_prefix);
	wbd_nvram_prefix_set(vif_prefix, NVRAM_IFNAME, vif_ifname);
	wbd_nvram_prefix_set(vif_prefix, NVRAM_UNIT, vif_unit);
	wbd_nvram_prefix_set(vif_prefix, NVRAM_HWADDR, vif_hwaddr);
	wbd_ether_atoe(vif_hwaddr, &hwaddr);
	(void)wl_bssiovar_set(prim_osifname, "cur_etheraddr", subunit, &hwaddr, ETHER_ADDR_LEN);
	/* printf("Interface : %s is created\n", vif_ifname); */

	/* commit NVRAM */
	nvram_commit();
	/* rc restart */
	kill(1, SIGHUP);

end:
	return ret;
}

/* Read "wbd_ifnames" NVRAM and get actual ifnames */
int
wbd_read_actual_ifnames(char *wbd_ifnames1, int len1, bool create)
{
	int ret = WBDE_OK, nvram_exists = 0, dwds_sta_pif = 0, is_fixed_ifnames = 0;
	char wbd_ifnames[NVRAM_MAX_VALUE_LEN] = {0};
	char wbd_if_new[NVRAM_MAX_VALUE_LEN] = {0}, temp[NVRAM_MAX_VALUE_LEN] = {0};
	char name[IFNAMSIZ] = {0}, var_intf[IFNAMSIZ] = {0}, outname[IFNAMSIZ];
	char *next_intf, *val;

	memset(wbd_ifnames1, 0, len1);

	/* Get NVRAM value for "wbd_ifnames" */
	val = wbd_nvram_safe_get(WBD_NVRAM_IFNAMES);
	if (val && (val[0] != '\0')) {
		WBDSTRNCPY(wbd_ifnames, val, sizeof(wbd_ifnames) - 1);
		nvram_exists = 1;
	}

	/* if app other wbd (wlconf, bsd etc ) is calling this, and NVRAM not exist */
	if ((!create) && (!nvram_exists))
	{
		/* wbd app will create it, so just get out */
		ret = WBDE_WBD_IFNAMES_NEXST;
		WBD_RC_PRINT("create[%d] nvram_exists[%d]. NVRAM not exists\n",
			create, nvram_exists);
		goto end;
	}

	/* Get NVRAM value which tells whether to recreate the NVRAM or not */
	is_fixed_ifnames = wbd_nvram_safe_get_int(NULL, WBD_NVRAM_FIXED_IFNAMES, 0);

	/* if wbd app is calling this */
	if (create) {
		/* Find First DWDS Primary Interface, with mode = STA */
		ret = wbd_find_dwds_sta_primif(name, sizeof(name), name, sizeof(name));
		dwds_sta_pif = (ret == WBDE_DWDS_STA_PIF_NEXST) ? 0 : 1;
		ret = WBDE_OK;

		/* if NVRAM not exist OR, NVRAM exists but any DWDS STA Primary Ifr found
		 * && if wbd_ifnames are not fixed
		 */
		if ((!nvram_exists) || (nvram_exists && dwds_sta_pif && (!is_fixed_ifnames))) {
			/* prepare "wbd_ifnames"  fm "lan_ifnames" */
			wbd_ifnames_fm_lan_ifnames(wbd_if_new, sizeof(wbd_if_new),
				temp, sizeof(temp));
			WBD_RC_PRINT("nvram_exists[%d] dwds_sta_pif[%d] is_fixed_ifnames[%d] "
				"wbd_if_new[%s] temp[%s]\n",
				nvram_exists, dwds_sta_pif, is_fixed_ifnames, wbd_if_new, temp);
		}

		/* if NVRAM not exist OR, NVRAM exists && any DWDS STA Primary Ifr found
		 * && if wbd_ifnames are not fixed && Old NVRAM value and newly prepared
		 * wbd_ifnames don't match
		 */
		if ((!nvram_exists) || (nvram_exists && dwds_sta_pif && (!is_fixed_ifnames) &&
			(strcmp(wbd_if_new, wbd_ifnames) != 0))) {
			/* Set NVRAM value for "wbd_ifnames", commit NVRAM, rc restart */
			nvram_set(WBD_NVRAM_IFNAMES, wbd_if_new);
			nvram_commit();
			WBD_RC_PRINT("nvram_exists[%d] dwds_sta_pif[%d] is_fixed_ifnames[%d] "
				"wbd_ifnames[%s] wbd_if_new[%s]. Going to restart...\n",
				nvram_exists, dwds_sta_pif, is_fixed_ifnames, wbd_ifnames,
				wbd_if_new);
			kill(1, SIGHUP);
		}
	}

	/* Traverse wbd_ifnames for each ifname */
	foreach(var_intf, wbd_ifnames, next_intf) {

		memset(outname, 0, sizeof(outname));
		/* Copy interface name temporarily */
		WBDSTRNCPY(name, var_intf, sizeof(name) - 1);

		/* Check if Interface is DWDS Primary Interface, with mode = STA, Change name */
		wbd_check_dwds_sta_primif(name, outname, sizeof(outname));

		/* Check if Interface is DWDS Virtual Interface, if Disabled, Enable it */
		if (create) {
			wbd_enable_dwds_ap_vif(outname);
		}

		/* Add this interface to "wbd_ifnames1" */
		add_to_list(outname, wbd_ifnames1, len1);
		WBD_RC_PRINT("wbd_ifnames1[%s] name[%s] outname[%s]\n", wbd_ifnames1,
			name, outname);
	}
	WBD_RC_PRINT("nvram_exists[%d] dwds_sta_pif[%d] is_fixed_ifnames[%d] wbd_ifnames[%s] "
		"wbd_ifnames1[%s]\n", nvram_exists, dwds_sta_pif, is_fixed_ifnames,
		wbd_ifnames, wbd_ifnames1);
end:
	return ret;
}

/* Find Number of valid interfaces */
int
wbd_count_interfaces(void)
{
	char nvram_name[32], ifname[32];
	int index, total = 0, unit = -1;

	/* Find out the wl interface index for the specified interface. */
	for (index = 0; index < DEV_NUMIFS; ++index) {

		snprintf(nvram_name, sizeof(nvram_name), "wl%d_ifname", index);
		WBDSTRNCPY(ifname, nvram_safe_get(nvram_name), sizeof(ifname));

		WBD_RC_PRINT("nvram_name=%s, ifname=%s\n", nvram_name, ifname);

		if (!wl_probe(ifname)) {
			if (!wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit))) {
				total ++;
				WBD_RC_PRINT("unit=%d, total=%d\n", unit, total);
			}
		}
	}

	return total;
}
