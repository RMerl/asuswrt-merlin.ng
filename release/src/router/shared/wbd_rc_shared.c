/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Wi-Fi Blanket shared functions
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
 * <<Broadcom-WL-IPTag/Open:>>
 * $Id: wbd_rc_shared.c 764858 2018-06-06 13:27:56Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <typedefs.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmparams.h>
#include <ethernet.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <common_utils.h>

#include "wbd_rc_shared.h"

/* Wi-Fi Blanket Buffer Lengths */
#define WBD_MAX_BUF_256			256
#define WBD_MIN_PSK_LEN			8
#define WBD_MAX_PSK_LEN			63

/* WBD Backhaul Types
 * TODO : Needs to add entry for PLC and MOCA interfaces
 */
typedef enum {
	WBD_BACKHAUL_TYPE_UNDEFINED = -1,
	WBD_BACKHAUL_TYPE_MASTER = 0,
	WBD_BACKHAUL_TYPE_ETHERNET,
	WBD_BACKHAUL_TYPE_DWDS,
	WBD_BACKHAUL_TYPE_PLC
} wbd_backhaul_type_t;

#define WBD_BACKHAUL_TYPE_STR_ETH	"eth"
#define WBD_BACKHAUL_TYPE_STR_DWDS	"dwds"

/* MAP Flags */
#define IEEE1905_MAP_FLAG_FRONTHAUL 0x01  /* Fronthaul BSS */
#define IEEE1905_MAP_FLAG_BACKHAUL  0x02  /* Backhaul BSS */
#define IEEE1905_MAP_FLAG_STA       0x04  /* STA */

#define I5_IS_BSS_FRONTHAUL(flags)	((flags) & IEEE1905_MAP_FLAG_FRONTHAUL)
#define I5_IS_BSS_BACKHAUL(flags)	((flags) & IEEE1905_MAP_FLAG_BACKHAUL)
#define I5_IS_BSS_STA(flags)		((flags) & IEEE1905_MAP_FLAG_STA)

/* Convert Backhaul String to Backhaul enum type */
#define WBD_BKHL_STR_TO_TYPE(str)	 \
	((strcasecmp((str), WBD_BACKHAUL_TYPE_STR_ETH) == 0) ? WBD_BACKHAUL_TYPE_ETHERNET : \
	((strcasecmp((str), WBD_BACKHAUL_TYPE_STR_DWDS) == 0) ? WBD_BACKHAUL_TYPE_DWDS : \
	WBD_BACKHAUL_TYPE_UNDEFINED))

/* Wi-Fi Blanket Bridge Types of Interfaces */
#define	WBD_BRIDGE_INVALID	-1	/* -1 Bridge Type is Invalid */
#define	WBD_BRIDGE_LAN		0	/*  0 Bridge Type is LAN */
#define	WBD_BRIDGE_GUEST	1	/*  1 Bridge Type is GUEST */
#define WBD_BRIDGE_VALID(bridge) ((((bridge) < WBD_BRIDGE_LAN) || \
					((bridge) > WBD_BRIDGE_GUEST)) ? (0) : (1))

/* Wi-Fi Blanket Band Types */
#define	WBD_BAND_LAN_INVALID	0x00	/* 0 - auto-select */
#define	WBD_BAND_LAN_2G		0x01	/* 1 - 2.4 Ghz */
#define	WBD_BAND_LAN_5GL	0x02	/* 2 - 5 Ghz LOW */
#define	WBD_BAND_LAN_5GH	0x04	/* 4 - 5 Ghz HIGH */
#define	WBD_BAND_LAN_ALL	(WBD_BAND_LAN_2G | WBD_BAND_LAN_5GL | WBD_BAND_LAN_5GH)

/* Validate Wi-Fi Blanket Band LAN Type Digit */
#define WBD_BAND_LAN_VALID(band) ((band) & (WBD_BAND_LAN_ALL))

/* Validate Wi-Fi Blanket Band Digit */
#define WBD_BAND_VALID(band)	(WBD_BAND_LAN_VALID((band)))

/* Print Wi-Fi Blanket Band Digit */
#define WBD_BAND_DIGIT(band) (((band) & WBD_BAND_LAN_2G) ? (2) : (5))

/* General NVRAMs */
#define NVRAM_BR0_IFNAMES		"br0_ifnames"
#define NVRAM_LAN_IFNAMES		"lan_ifnames"
#define NVRAM_LAN1_IFNAMES		"lan1_ifnames"
#define NVRAM_WPS_CUSTOM_IFNAMES	"wps_custom_ifnames"
#define NVRAM_MODE			"mode"
#define NVRAM_DWDS			"dwds"
#define NVRAM_ROUTER_DISABLE		"router_disable"
#define NVRAM_LAN_DHCP			"lan_dhcp"
#define NVRAM_UNIT			"unit"
#define NVRAM_BSS_ENABLED		"bss_enabled"
#define NVRAM_HWADDR			"hwaddr"
#define NVRAM_IFNAME			"ifname"
#define NVRAM_VIFS			"vifs"
#define NVRAM_AKM			"akm"
#define NVRAM_SSID			"ssid"
#define NVRAM_CLOSED			"closed"
#define NVRAM_CRYPTO			"crypto"
#define NVRAM_WPA_PSK			"wpa_psk"
#define NVRAM_MAP			"map"
#define NVRAM_LAN_WPS_OOB		"lan_wps_oob"

/* WBD NVRAM variable names */
#define WBD_NVRAM_IFNAMES		"wbd_ifnames"
#define WBD_NVRAM_REPEAT_BACKHAUL	"wbd_repeat_backhaul"
#define WBD_NVRAM_BACKHAUL_BAND		"wbd_backhaul_band"
#define WBD_NVRAM_FBT			"wbd_fbt"
#define WBD_NVRAM_AUTO_CONFIG		"wbd_auto_config"
#define WBD_BACKHAUL_SSID		"wbd_backhaul_ssid"
#define WBD_BACKHAUL_PWD		"wbd_backhaul_pwd"
#define WBD_NVRAM_BACKHAUL_TYPE		"wbd_backhaul_type"
#define NVRAM_MAP_BSS_NAMES	        "map_bss_names"
#define NVRAM_MAP_MODE			"multiap_mode"
#define NVRAM_MAP_UAP			"map_uap"

/* FBT related NVRAMs */
#define NVRAM_FBT_APS			"fbt_aps"
#define NVRAM_FBT_MDID			"fbt_mdid"
#define NVRAM_FBT_ADDR			"addr"
#define NVRAM_FBT_R1KH_ID		"r1kh_id"
#define NVRAM_FBT_R0KH_ID		"r0kh_id"
#define NVRAM_FBT_R0KH_ID_LEN		"r0kh_id_len"
#define NVRAM_FBT_R0KH_KEY		"r0kh_key"
#define NVRAM_FBT_R1KH_KEY		"r1kh_key"
#define NVRAM_FBT_BR_ADDR		"br_addr"
#define NVRAM_FBT_ENABLED		"fbt"
#define NVRAM_FBT_OVERDS		"fbtoverds"
#define NVRAM_FBT_REASSOC_TIME		"fbt_reassoc_time"
#define NVRAM_FBT_AP			"fbt_ap"

/* FBT constants */
#define WBD_FBT_R0KH_ID_LEN		48
#define WBD_FBT_KH_KEY_LEN		17
#define WBD_MAX_AP_NAME			16
#define WBD_FBT_NVRAMS_PER_SLAVE	7

/* WBD NVRAM variables - Default values */
#define WBD_NV_DEF_FIXED_IFNAMES	1
#define WBD_NV_DEF_REPEAT_BACKHAUL	0
#define WBD_NV_DEF_BACKHAUL_BAND	WBD_BAND_LAN_5GH
#define WBD_NV_DEF_AUTO_CONFIG		1
#define WBD_NV_DEF_DWDS			"1"
#define WBD_NV_DEF_REP_MODE		"sta"
#define WBD_NV_DEF_REP_ROUTER_DISABLE	"1"
#define WBD_NV_DEF_REP_LAN_DHCP		"1"
#define WBD_NV_DEF_MODE_AP		"ap"
#define WBD_NV_DEF_BACKHAUL_SSID	"Bcm_Wbd_Hidden"
#define WBD_NV_DEF_CLOSED		"1"
#define WBD_NV_DEF_AKM			"psk2"
#define WBD_NV_DEF_CRYPTO		"aes"
#define WBD_NV_DEF_WPA_PSK		"Pv!tGcM0K&#>^mRk"
#define WBD_NV_DEF_BACKHAUL_TYPE	WBD_BACKHAUL_TYPE_DWDS

/* FBT NVRAMs default values */
#define WBD_FBT_NOT_DEFINED		-1
#define WBD_FBT_DEF_FBT_DISABLED	0
#define WBD_FBT_DEF_FBT_ENABLED		1
#define WBD_FBT_DEF_OVERDS		0
#define WBD_FBT_DEF_REASSOC_TIME	1000
#define WBD_FBT_DEF_AP			1

/* WBD Errors */
#define WBDE_OK			0
#define WBDE_INV_ARG		-3	/* Invalid arguments */
#define WBDE_WL_ERROR		-11	/* WL IOVAR error */
#define WBDE_INV_MODE		-20	/* Invalid Blanket Mode */
#define WBDE_DWDS_AP_VIF_EXST	-30	/* Virtual interface already up */
#define WBDE_DWDS_STA_PIF_NEXST	-31	/* No DWDS primary Ifr with mode STA */
#define WBDE_WBD_IFNAMES_FULL	-32	/* wbd_ifnames have 2G & 5G ifnames */
#define WBDE_WBD_IFNAMES_NEXST	-33	/* wbd_ifnames NVRAM not defined */
#define WBDE_CLI_INV_BAND	-75	/* Valid Band required */
#define WBDE_INV_IFNAME		-81	/* Invalid interface name */

#ifndef WBDSTRNCPY
#define WBDSTRNCPY(dst, src, len)	 \
	do { \
		strncpy(dst, src, len -1); \
		dst[len - 1] = '\0'; \
	} while (0)
#endif // endif

#ifdef WBD_RC_PRINT_ENB
#define WBD_RC_PRINT(fmt, arg...) \
	printf("WBD-RC-SHARED >> %s(%d) : "fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define WBD_RC_PRINT(fmt, arg...)
#endif /* WBD_RC_PRINT_ENB */

/* MulitAP Modes */
#define MAP_MODE_FLAG_DISABLED		0x0000	/* Disabled */
#define MAP_MODE_FLAG_CONTROLLER	0x0001	/* Controller */
#define MAP_MODE_FLAG_AGENT		0x0002	/* Agent */
#define MAP_IS_DISABLED(mode)	((mode) <= MAP_MODE_FLAG_DISABLED)	/* Is Disabled */
#define MAP_IS_CONTROLLER(mode)	((mode) & MAP_MODE_FLAG_CONTROLLER)	/* Is Controller */
#define MAP_IS_AGENT(mode)	((mode) & MAP_MODE_FLAG_AGENT)		/* Is Agent */

/* IOCTL swapping mode for Big Endian host with Little Endian dongle.  Default to off */
/* The below macros handle endian mis-matches between wl utility and wl driver. */
extern bool gg_swap;
#define htod32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))
#define dtoh32(i) (gg_swap ? bcmswap32(i) : (uint32)(i))

/* Extern Declarations */
extern char* strncpy_n(char *destination, const char *source, size_t num);
extern void nvram_initialize_all(char *prefix);
static inline void sys_reboot(void)
{
	kill(1, SIGTERM);
}

#if defined(BCA_CPEROUTER)
#define wbd_sys_restart() do { system("nvram restart"); } while (0)
#elif defined(STB)
#define wbd_sys_restart() \
	do { \
		char signal[] = "XXXX"; \
		snprintf(signal, sizeof(signal), "-%d", SIGHUP); \
		eval("killall", signal, "rc"); \
	} while (0)
#else
#define wbd_sys_restart() kill(1, SIGHUP)
#endif /* BCA_CPEROUTER */

#if defined(BCA_CPEROUTER)
#define wbd_sys_reboot() do { system("reboot"); } while (0)
#else
#define wbd_sys_reboot() do { sys_reboot(); } while (0)
#endif /* BCA_CPEROUTER */

#if defined(BCA_CPEROUTER)
#define wbd_nvram_commit() do { system("nvram commit"); } while (0)
#else
#define wbd_nvram_commit() do { nvram_commit(); } while (0)
#endif /* BCA_CPEROUTER */

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

	if (prefix) {
		return nvram_safe_get(strcat_r(prefix, nvram, data));
	} else {
		return nvram_safe_get(nvram);
	}
}

/* Gets the unsigned integer config val from NVARM, if not found applies the default value */
static uint16
wbd_nvram_safe_get_uint(char* prefix, const char *c, uint16 def)
{
	char *val = NULL;
	uint16 ret = def;

	if (prefix) {
		val = wbd_nvram_prefix_safe_get(prefix, c);
	} else {
		val = wbd_nvram_safe_get(c);
	}

	if (val[0] != '\0') {
		ret = (uint16)strtoul(val, NULL, 0);
	} else {
		WBD_RC_PRINT("NVRAM is not defined: %s%s \n", (prefix ? prefix : ""), c);
	}

	return ret;
}

/* Gets the integer config val from NVARM, if not found applies the default value */
static int
wbd_nvram_safe_get_int(char* prefix, const char *c, int def)
{
	char *val = NULL;
	int ret = def;

	if (prefix) {
		val = wbd_nvram_prefix_safe_get(prefix, c);
	} else {
		val = wbd_nvram_safe_get(c);
	}

	if (val[0] != '\0') {
		ret = (int)strtol(val, NULL, 0);
	} else {
		WBD_RC_PRINT("NVRAM is not defined: %s%s \n", (prefix ? prefix : ""), c);
	}

	return ret;
}

/* WBD version to set the NVRAM value for specific BSS prefix */
static int
wbd_nvram_prefix_set(const char *prefix, const char *nvram, const char *nvramval)
{
	char data[WBD_MAX_BUF_256] = {0};

	if (prefix) {
		return nvram_set(strcat_r(prefix, nvram, data), nvramval);
	} else {
		return nvram_set(nvram, nvramval);
	}
}

/* Match NVRAM and ARG value, and if mismatch, Set new value in NVRAM */
static uint32
wbd_nvram_prefix_match_set(const char* prefix, char* nvram, char* new_value,
	bool matchcase)
{
	char *nvram_value = NULL;
	int mismatch = 0;

	/* Get NVRAM Value */
	nvram_value = wbd_nvram_prefix_safe_get(prefix, nvram);

	/* Compare NVRAM and New value, and if mismatch, set New value in NVRAM */
	if (matchcase) {
		mismatch = strcmp(nvram_value, new_value);
	} else {
		mismatch = strcasecmp(nvram_value, new_value);
	}

	if (mismatch) {

		WBD_RC_PRINT("Prefix[%s] NVRAM[%s] NVRAMVal[%s] != NewVal[%s]."
			" Needs Rc Restart.\n", prefix, nvram, nvram_value, new_value);

		/* Set New Value in NVRAM */
		wbd_nvram_prefix_set(prefix, nvram, new_value);

		/* Value is Overidden, indicate NVRAM commit & RC Restart */
		return WBD_FLG_NV_COMMIT|WBD_FLG_RC_RESTART;
	}
	return 0;
}

/* Executes nvram commit/rc restart/reboot commands */
void
wbd_do_rc_restart_reboot(uint32 rc_flags)
{
	if (rc_flags & WBD_FLG_NV_COMMIT) {
		WBD_RC_PRINT("rc_flags[0x%x], Going to do nvram_commit()...\n", rc_flags);
		wbd_nvram_commit();
	}

	if (rc_flags & WBD_FLG_REBOOT) {
		WBD_RC_PRINT("rc_flags[0x%x], Going to do Reboot...\n", rc_flags);
		wbd_sys_reboot();
		return;
	}

	if (rc_flags & WBD_FLG_RC_RESTART) {
		WBD_RC_PRINT("rc_flags[0x%x], Going to do RC Restart...\n", rc_flags);
		wbd_sys_restart();
		return;
	}
}

/* Check if Interface is Virtual or not */
static bool
wbd_is_ifr_virtual(char *ifname)
{
	int unit = -1, subunit = -1;

	if (get_ifname_unit(ifname, &unit, &subunit) < 0) {
		return FALSE;
	}
	if (unit < 0) {
		return FALSE;
	}
	if (subunit >= 0) {
		return TRUE;
	}

	return FALSE;
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

	wl_endian_probe(ifname);

	/* Get instance */
	ret = wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
	if (ret < 0) {
		/* printf("%s : Failed to %s, WL error : %d\n", ifname, "get instance", ret); */
		ret = WBDE_WL_ERROR;
		goto end;
	}
	unit = dtoh32(unit);

	/* Check if Primary Radio of given Interface is enabled or not */
	snprintf(buf, sizeof(buf), "wl%d_radio", unit);
	ifr_enabled = wbd_nvram_safe_get_int(NULL, buf, 0);

	if (!ifr_enabled) {
		goto end;
	}

	/* Check if interface is vifs */
	if ((validate_vif) && (wbd_is_ifr_virtual(ifname))) {
		/* Check if vifs is enabled or not */
		snprintf(buf, sizeof(buf), "%s_bss_enabled", ifname);
		ifr_enabled = wbd_nvram_safe_get_int(NULL, buf, 0);
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

/* Gets WBD Band Enumeration from ifname's Chanspec & Bridge Type */
static int
wbd_identify_wbd_band_type(int bridge_dgt, chanspec_t ifr_chanspec, int *out_band)
{
	int ret = WBDE_CLI_INV_BAND, channel = 0;

	if (!out_band) {
		ret = WBDE_INV_ARG;
		goto end;
	}
	*out_band = WBD_BAND_LAN_INVALID;

	if (!WBD_BRIDGE_VALID(bridge_dgt)) {
		goto end;
	}

	channel = CHSPEC_CHANNEL(ifr_chanspec);

	if (bridge_dgt == WBD_BRIDGE_LAN) {
		if (CHSPEC_IS2G(ifr_chanspec)) {
			*out_band = WBD_BAND_LAN_2G;
		} else if (CHSPEC_IS5G(ifr_chanspec)) {
			if (channel < 100) {
				*out_band = WBD_BAND_LAN_5GL;
			} else {
				*out_band = WBD_BAND_LAN_5GH;
			}
		}
		ret = WBDE_OK;
	}

end:
	WBD_RC_PRINT("Bridge[br%d] Chanspec[0x%x] Channel[%d] WBD_BAND[%d]\n",
		bridge_dgt, ifr_chanspec, channel,
		((out_band) ? *out_band : WBD_BAND_LAN_INVALID));

	return ret;
}

/* Find Primary Interface Name configured on a specific WBD Band Type */
static int
wbd_find_prim_ifname_fm_wbd_band(int wbd_band, char *ifname, int len, bool check_ap_mode)
{
	int ret, ifr_band, ifr_band_digit = 0, cur_chspec, wbd_band_digit = 0;
	char lan_ifnames[NVRAM_MAX_VALUE_LEN] = {0};
	char var_intf[IFNAMSIZ] = {0}, *next_intf, tmp[NVRAM_MAX_VALUE_LEN];
	char prefix[IFNAMSIZ] = {0}, name[IFNAMSIZ] = {0}, nbr_ifname[IFNAMSIZ] = {0};
	chanspec_t ifr_chanspec;

	memset(ifname, 0, len);

	/* Validate Arg, WBD Band */
	if (!WBD_BAND_VALID((wbd_band))) {
		return WBDE_CLI_INV_BAND;
	}

	/* Get input Band Digit */
	wbd_band_digit = WBD_BAND_DIGIT((wbd_band));

	/* Get NVRAM value from "lan_ifnames" and save it */
	WBDSTRNCPY(lan_ifnames, wbd_nvram_safe_get(NVRAM_LAN_IFNAMES), sizeof(lan_ifnames) - 1);

	/* Traverse lan_ifnames for each ifname */
	foreach(var_intf, lan_ifnames, next_intf) {

		/* Copy Interface name temporarily */
		WBDSTRNCPY(name, var_intf, sizeof(name) - 1);

		/* Check if valid Wireless Interface and its Primary Radio is enabled or not */
		if (!wbd_is_ifr_enabled(name, FALSE, &ret)) {
			continue; /* Skip non-Wireless & disabled Wireless Interface */
		}

		/* Check if Interface is Virtual Interface */
		if (wbd_is_ifr_virtual(name)) {
			continue; /* skip Virtual Interface, as we are looking for Primif */
		}

		/* If Interface's mode is required as "ap" */
		if (check_ap_mode) {

			/* Get Prefix from OS specific interface name */
			wbd_get_prefix(name, prefix, sizeof(prefix));

			/* Check if Interface's NVRAM wlX_mode = "ap" or not */
			if (!nvram_match(strcat_r(prefix, NVRAM_MODE, tmp),
				WBD_NV_DEF_MODE_AP)) {
				continue; /* Skip non-ap Interface */
			}
		}

		/* Get Interface's Chanspec */
		wl_endian_probe(name);
		(void)wl_iovar_getint(name, "chanspec", &cur_chspec);
		ifr_chanspec = (chanspec_t)dtoh32(cur_chspec);

		/* Gets WBD Band Enumeration from ifname's Chanspec & Bridge Type */
		ret = wbd_identify_wbd_band_type(WBD_BRIDGE_LAN, ifr_chanspec, &ifr_band);
		if (ret != WBDE_OK) {
			continue;
		}

		/* Get Interface's Band Digit */
		ifr_band_digit = WBD_BAND_DIGIT((ifr_band));

		/* Compare This Interface's WBD Band Digit with Input Band Digit */
		if (ifr_band_digit == wbd_band_digit) {
			/* Copy Neighbor ifname, if matching ifr not found, return this ifname */
			WBDSTRNCPY(nbr_ifname, name, sizeof(nbr_ifname) - 1);
		}

		/* Compare This Interface's WBD Band Type with Dedicated Band */
		if (ifr_band == wbd_band) {
			/* Copy Interface name to return it */
			WBDSTRNCPY(ifname, name, len - 1);
			return WBDE_OK; /* if WBD Band Types matches, exit loop with Success code */
		}
	}

	/* Copy Neighbor Interface name to return it, as matching ifr not found */
	WBDSTRNCPY(ifname, nbr_ifname, len - 1);
	return WBDE_DWDS_STA_PIF_NEXST;
}

/* Check if Interface is Virtual Interface, if Disabled, Enable it */
uint32
wbd_enable_vif(char *ifname)
{
	int vif_enabled, unit, subunit;
	char prefix[IFNAMSIZ] = {0};
	uint32 rc_flags = 0;

	/* Check if interface is Primary Interface */
	if (!wbd_is_ifr_virtual(ifname)) {
		goto end; /* get out */
	}

	/* Get Prefix from OS specific interface name */
	wbd_get_prefix(ifname, prefix, sizeof(prefix));

	/* Check if Virtual AP Interface is Enabled */
	vif_enabled = wbd_nvram_safe_get_int(prefix, NVRAM_BSS_ENABLED, 0);

	if (!vif_enabled) {
		/* Get Unit & Subunit of Virtual Interface */
		sscanf(ifname, "wl%d.%d", &unit, &subunit);

		/* Create & Configure Virtual AP Interface, for DWDS Slave */
		wbd_create_vif(unit, subunit);
		/* Commit NVRAM & RC Restart */
		rc_flags |= WBD_FLG_NV_COMMIT|WBD_FLG_RC_RESTART;
	}

end:
	return rc_flags;
}

/* Check if Interface is DWDS Primary Interface, with mode = STA, Change name1 */
int
wbd_check_dwds_sta_primif(char *ifname, char *ifname1, int len1)
{
	int ret = WBDE_DWDS_STA_PIF_NEXST, dwds, sta_mode, unit, subunit = 1;
	char prefix[IFNAMSIZ] = {0}, tmp[NVRAM_MAX_VALUE_LEN];

	/* Check if interface is Virtual Interface */
	if (wbd_is_ifr_virtual(ifname)) {
		goto end; /* get out */
	}

	/* Get Prefix from OS specific interface name */
	wbd_get_prefix(ifname, prefix, sizeof(prefix));

	/* Check if on Primary Interface DWDS in ON */
	dwds = wbd_nvram_safe_get_int(prefix, NVRAM_DWDS, 0);

	/* Check if on Primary Interface Mode is STA */
	sta_mode = nvram_match(strcat_r(prefix, NVRAM_MODE, tmp), WBD_NV_DEF_REP_MODE) ? 1 : 0;

	/* printf("%sdwds = %d, sta_mode = %d\n", prefix, dwds, sta_mode); */
	/* if DWDS in ON, mode is STA */
	if (dwds && sta_mode) {

		/* Get Unit of Primary Interface */
		unit = wbd_nvram_safe_get_int(prefix, NVRAM_UNIT, 0);

		/* Get next available Virtual AP Interface for DWDS Slave */
		subunit = wbd_get_next_vif_subunit(unit, &ret);

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

/* Find Backhaul Prim Ifr Configured on this Device (RootAP/Repeater), Check if its Dedicated */
int
wbd_find_backhaul_primif_on_device(char *backhaul_if, int backhaul_if_len, int *repeat_backhaul)
{
	int ret = WBDE_DWDS_STA_PIF_NEXST, backhaul_band, map_uap;
	char ifname[IFNAMSIZ] = {0}, *val = NULL;
	wbd_backhaul_type_t backhaul_type;

	/* Get the NVRAM : Upastream AP not */
	map_uap = wbd_nvram_safe_get_int(NULL, NVRAM_MAP_UAP, 0);

	/* Get the NVRAM from "wbd_backhaul_band" and save it */
	backhaul_band = wbd_nvram_safe_get_int(NULL,
		WBD_NVRAM_BACKHAUL_BAND, WBD_NV_DEF_BACKHAUL_BAND);

	/* Find Primary Interface Name configured on RootAP/Repeater for Backhaul Band */
	ret = wbd_find_prim_ifname_fm_wbd_band(backhaul_band, ifname, sizeof(ifname),
		map_uap ? TRUE : FALSE);
	WBD_RC_PRINT("ifname for Backhaul Band[%d] is [%s]\n", backhaul_band, ifname);

	/* Remember Backhaul Link */
	WBDSTRNCPY(backhaul_if, ifname, backhaul_if_len - 1);

	if (repeat_backhaul) {
		/* Get the NVRAM which tells whether to have dedicated backhaul link or not */
		*repeat_backhaul = wbd_nvram_safe_get_int(NULL, WBD_NVRAM_REPEAT_BACKHAUL,
			WBD_NV_DEF_REPEAT_BACKHAUL);

		/* Get the NVRAM : Backhaul Type, and convert it to enum */
		val = wbd_nvram_safe_get(WBD_NVRAM_BACKHAUL_TYPE);
		backhaul_type = (val[0] != '\0') ?
			WBD_BKHL_STR_TO_TYPE(val) : WBD_NV_DEF_BACKHAUL_TYPE;

		/* If the number of interfaces is not more than two(not triband),
		 * or If the Downstream AP is other than DWDS repeater,
		 * disable dedicated backhaul feature
		 */
		if ((wbd_count_interfaces() <= 2) ||
			(!map_uap && (backhaul_type != WBD_BACKHAUL_TYPE_DWDS))) {
			*repeat_backhaul = 1;
		}

		WBD_RC_PRINT("repeat_backhaul[%d].\n", *repeat_backhaul);
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
	int ret = WBDE_OK, repeat_backhaul = 0;
	char lan_ifnames[NVRAM_MAX_VALUE_LEN] = {0};
	char backhaul_if[IFNAMSIZ] = {0}, name[IFNAMSIZ] = {0};
	char *next_intf, var_intf[IFNAMSIZ] = {0};

	memset(wbd_ifnames, 0, len);
	memset(wbd_ifnames1, 0, len1);

	/* Find Backhaul Prim Ifr on this Device (RootAP/Repeater), Check if its Dedicated */
	wbd_find_backhaul_primif_on_device(backhaul_if, sizeof(backhaul_if), &repeat_backhaul);

	/* Get NVRAM value from "lan_ifnames" and save it */
	WBDSTRNCPY(lan_ifnames, wbd_nvram_safe_get(NVRAM_LAN_IFNAMES), sizeof(lan_ifnames) - 1);

	/* Traverse lan_ifnames for non-DWDS Primary ifnames */
	foreach(var_intf, lan_ifnames, next_intf) {

		/* Copy interface name temporarily */
		WBDSTRNCPY(name, var_intf, sizeof(name) - 1);

		/* Don't add this ifname to "wbd_ifnames", if it's a Virtual Interface or
		 * if it's a Dedicated Backhaul Primary Interface and we don't want to Repeat it
		 */
		if ((wbd_is_ifr_virtual(name)) ||
			((!repeat_backhaul) && (strcmp(name, backhaul_if) == 0))) {

			WBD_RC_PRINT("name[%s] backhaul_if[%s] repeat_backhaul[%d].\n",
				name, backhaul_if, repeat_backhaul);
			continue; /* Skip it */
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

/* Get next available Virtual Interface Subunit */
int
wbd_get_next_vif_subunit(int in_unit, int *error)
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
		if (!wbd_is_ifr_virtual(name)) {
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

/* Create & Configure Virtual Interface (MBSS) in Network Type LAN */
/* Returns : WBDE_OK : if vif required to enable, gets enabled successfully */
/* Returns : WBDE_DWDS_AP_VIF_EXST : if vif required to enable, is already enabled */
int
wbd_create_vif(int unit, int subunit)
{
	int ret = WBDE_OK, vif_enabled;
	char prim_prefix[IFNAMSIZ] = {0};
	char vif_prefix[IFNAMSIZ] = {0}, vif_ifname[IFNAMSIZ] = {0};
	char vif_unit[IFNAMSIZ] = {0};
	char interface_list[NVRAM_MAX_VALUE_LEN] = {0}, *str = NULL;
	int interface_list_size = sizeof(interface_list);

	/* Create prim_prefix[wlX_] */
	snprintf(prim_prefix, sizeof(prim_prefix), "wl%d_", unit);

	/* Create vif_prefix[wlX.1_] , vif_ifname[wlX.1],  vif_unit[X.1] */
	snprintf(vif_ifname, sizeof(vif_ifname), "wl%d.%d", unit, subunit);
	snprintf(vif_prefix, sizeof(vif_prefix), "wl%d.%d_", unit, subunit);
	snprintf(vif_unit, sizeof(vif_unit), "%d.%d", unit, subunit);

	/* Check if AP vif is already up or not */
	vif_enabled = wbd_nvram_safe_get_int(vif_prefix, NVRAM_BSS_ENABLED, 0);
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

	/* Execute nvram commit & rc restart commands, skipping as Easy Setup covers this */
	/* wbd_do_rc_restart_reboot(WBD_FLG_NV_COMMIT|WBD_FLG_RC_RESTART); */

end:
	return ret;
}

#if defined(__linux__)

#ifndef	RANDOM_READ_TRY_MAX
#define RANDOM_READ_TRY_MAX	10
#endif // endif
static void linux_random(uint8 *rand, int len)
{
	static int dev_random_fd = -1;
	int status;
	int i;

	if (dev_random_fd == -1) {
		if ((dev_random_fd = open("/dev/urandom", O_RDONLY|O_NONBLOCK)) == -1) {
			WBD_RC_PRINT("Error opening /dev/urandom : %s", strerror(errno));
			return;
		}
	}

	for (i = 0; i < RANDOM_READ_TRY_MAX; i++) {
		status = read(dev_random_fd, rand, len);
		if (status == -1) {
			if (errno == EINTR)
				continue;

			assert(status != -1);
		}

		return;
	}

	assert(i != RANDOM_READ_TRY_MAX);
}

/* Get Random Bytes */
static void RAND_bytes(unsigned char *buf, int num)
{
	linux_random(buf, num);
}
#endif /* __linux__ */

#ifdef WLHOSTFBT

/* Get R0KHID from NVRAM, if not present generate and set the R0KHID */
char*
wbd_get_r0khid(char *prefix, char *r0khid, int r0khid_len, int set_nvram)
{
	char *nvval;
	uint32 id = 0;

	nvval = wbd_nvram_prefix_safe_get(prefix, NVRAM_FBT_R0KH_ID);
	if (strlen(nvval) <= 0) {
		/* Generate R0KHID */
		memset(r0khid, 0, r0khid_len);
		RAND_bytes((uint8 *)&id, sizeof(id));
		snprintf(r0khid, 6, "%05u", id);

		if (set_nvram) {
			wbd_nvram_prefix_set(prefix, NVRAM_FBT_R0KH_ID, r0khid);
			nvram_commit();
		}
	} else {
		WBDSTRNCPY(r0khid, nvval, r0khid_len);
	}

	return r0khid;
}

/* Get R0KHKEY from NVRAM, if not present generate and set the R0KHKEY */
char*
wbd_get_r0khkey(char *prefix, char *r0khkey, int r0khkey_len, int set_nvram)
{
	char *nvval;
	unsigned long long key = 0;

	nvval = wbd_nvram_prefix_safe_get(prefix, NVRAM_FBT_R0KH_KEY);
	if (strlen(nvval) <= 0) {
		/* Generate R0KHKEY */
		memset(r0khkey, 0, r0khkey_len);
		RAND_bytes((uint8 *)&key, sizeof(key));
		snprintf(r0khkey, 17, "%016llu", key);

		if (set_nvram) {
			wbd_nvram_prefix_set(prefix, NVRAM_FBT_R0KH_KEY, r0khkey);
			nvram_commit();
		}
	} else {
		WBDSTRNCPY(r0khkey, nvval, r0khkey_len);
	}

	return r0khkey;
}

/* Get R1KH_ID from NVRAM, if not present generate and set the R1KH_ID */
char*
wbd_get_r1khid(char *prefix, char *r1khid, int r1khid_len, int set_nvram)
{
	char *nvval = wbd_nvram_prefix_safe_get(prefix, NVRAM_FBT_R1KH_ID);

	if (strlen(nvval) <= 0) {
		memset(r1khid, 0, r1khid_len);

		/* Generate R1KH_ID from "wlX_hwaddr" */
		WBDSTRNCPY(r1khid, wbd_nvram_prefix_safe_get(prefix, NVRAM_HWADDR),
			r1khid_len);

		if (set_nvram) {
			wbd_nvram_prefix_set(prefix, NVRAM_FBT_R1KH_ID, r1khid);
			nvram_commit();
		}
	} else {
		WBDSTRNCPY(r1khid, nvval, r1khid_len);
	}

	return r1khid;
}

/* Get MDID from NVRAM, if not present Generate the MDID
 * The default behavior is to keep the same MDID across all
 * BSS on the same device to enable FBT between them.
 * If specific requirment is there to keep different MDID
 * across BSS then each BSS's MDID must be set in nvram
 */
uint16
wbd_get_mdid(char *prefix)
{
	uint16 mdid = 0;
	static uint16 prev_mdid = 0;

	mdid = wbd_nvram_safe_get_uint(prefix, NVRAM_FBT_MDID, 0);

	if (mdid) {
		return mdid;
	} else if (prev_mdid) {
		/* If MDID is not found in nvram and we already
		 * generated MDID once, use the same mdid
		 * to ensure FBT across bss
		 */
		return prev_mdid;
	}

	/* If MDID not found in NVRAM */
	if (mdid == 0) {
		/* Generate MDID */
		RAND_bytes((uint8 *)&mdid, sizeof(mdid));
		mdid |= 1;
	}
	/* store the generated mdid.
	 * same MDID will be returned on subsequent
	 * calls to this API to ensure FBT across BSS
	 */
	prev_mdid = mdid;

	return mdid;
}

/* Enable FBT */
int
wbd_enable_fbt(char *prefix)
{
	int fbt = WBD_FBT_DEF_FBT_ENABLED;
	char strnvval[WBD_MAX_BUF_256] = {0};
	char *nvval = NULL;

	/* Set wbd_fbt NVRAM */
	memset(strnvval, 0, sizeof(strnvval));
	snprintf(strnvval, sizeof(strnvval), "%d", fbt);
	wbd_nvram_prefix_set(prefix, WBD_NVRAM_FBT, strnvval);
	WBD_RC_PRINT("%swbd_fbt NVRAM not defined, Seting it[%s]\n", prefix, strnvval);

	memset(strnvval, 0, sizeof(strnvval));
	nvval = wbd_nvram_prefix_safe_get(prefix, NVRAM_AKM);
	WBDSTRNCPY(strnvval, nvval, sizeof(strnvval));

	/* Add psk2ft if psk2 is defined in akm NVRAM */
	if (find_in_list(nvval, "psk2")) {
		add_to_list("psk2ft", strnvval, sizeof(strnvval));
	}

	/* Add saeft if sae is defined in akm NVRAM */
	if (find_in_list(nvval, "sae")) {
		add_to_list("saeft", strnvval, sizeof(strnvval));
	}

	wbd_nvram_prefix_set(prefix, NVRAM_AKM, strnvval);
	WBD_RC_PRINT("Set %sakm NVRAM to [%s]\n", prefix, strnvval);

	return fbt;
}

/* Disable FBT */
int
wbd_disable_fbt(char *prefix)
{
	char strnvval[WBD_MAX_BUF_16] = {0}, strakm[WBD_MAX_BUF_256] = {0};
	char *nvval = NULL;

	/* Set wbd_fbt NVRAM */
	snprintf(strnvval, sizeof(strnvval), "%d", WBD_FBT_DEF_FBT_DISABLED);
	wbd_nvram_prefix_set(prefix, WBD_NVRAM_FBT, strnvval);
	WBD_RC_PRINT("Disabling FBT by setting %swbd_fbt as [%s]\n", prefix, strnvval);

	/* If psk2ft or saeft is defined in akm NVRAM, remove it */
	nvval = wbd_nvram_prefix_safe_get(prefix, NVRAM_AKM);
	WBDSTRNCPY(strakm, nvval, sizeof(strakm));
	remove_from_list("psk2ft", strakm, strlen(strakm));
	remove_from_list("saeft", strakm, strlen(strakm));
	wbd_nvram_prefix_set(prefix, NVRAM_AKM, strakm);
	WBD_RC_PRINT("Remove FBT from akm by setting %sakm as [%s]\n", prefix, strakm);

	return 0;
}

/* Check whether FBT enabling is possible or not. First it checks for psk2 and then wbd_fbt */
int
wbd_is_fbt_possible(char *prefix)
{
	int fbt = 0;
	char *nvval;

	/* Check if the akm contains psk2 or not */
	nvval = wbd_nvram_prefix_safe_get(prefix, NVRAM_AKM);

	if ((find_in_list(nvval, "psk2") == NULL) && (find_in_list(nvval, "sae") == NULL)) {
		WBD_RC_PRINT("%s%s[%s]. Not psk2 or sae. So no FBT\n", prefix, NVRAM_AKM, nvval);
		return 0;
	}

	/* Get the wlxy_wbd_ft NVRAM value, which tells whether FBT is enabled from WBD or not
	 * If the NVRAM is not defined it returns not defined(-1)
	 */
	fbt = wbd_nvram_safe_get_int(prefix, WBD_NVRAM_FBT, WBD_FBT_NOT_DEFINED);

end:
	return fbt;
}

/* If Device is Upstream AP, Initialize the FBT NVRAMs */
static uint32
wbd_uap_init_fbt_nvram_config(char *prefix)
{
	char *nvval = NULL;
	char strnvval[WBD_MAX_BUF_256] = {0};
	char new_value[WBD_MAX_BUF_256] = {0};
	char r1kh_id[WBD_FBT_R0KH_ID_LEN] = {0};
	char r0kh_id[WBD_FBT_R0KH_ID_LEN] = {0};
	char r0kh_key[WBD_FBT_KH_KEY_LEN] = {0};
	char data[WBD_MAX_BUF_256];
	int fbt = 0, invval = 0, mdid = 0;
	uint32 rc_flags = 0;

	WBD_RC_PRINT("Prefix[%s]\n", prefix);

	memset(data, 0, sizeof(data));

	/* Read MAP NVRAM, and Check if Fronthaul BSS */
	invval = wbd_nvram_safe_get_int(prefix, NVRAM_MAP, 0);

	/* Do below activity, only for Fronthaul BSS */
	if (!I5_IS_BSS_FRONTHAUL(invval)) {
		WBD_RC_PRINT("[%s] is not Fronthaul BSS. FBT settings not required.\n", prefix);
		goto end;
	}

	/* [1] If wbd_fbt NVRAM is not defined. Enable the FBT by setting the wbd_fbt */
	fbt = wbd_is_fbt_possible(prefix);
	if (fbt == WBD_FBT_NOT_DEFINED) {
		WBD_RC_PRINT("Prefix[%s]\n Enabling FBT...", prefix);
		fbt = wbd_enable_fbt(prefix);
		rc_flags |= WBD_FLG_NV_COMMIT|WBD_FLG_RC_RESTART;
	}

	/* If wbd_fbt is disabled, no need to continue further */
	if (fbt <= 0) {
		WBD_RC_PRINT("Prefix[%s] WBD_FBT[%d] is disabled\n", prefix, fbt);
		goto end;
	}

	/* [2] If FBT AP NVRAM is not defined. set it to default */
	nvval = nvram_get(strcat_r(prefix, NVRAM_FBT_AP, data));
	if (!nvval) {
		memset(strnvval, 0, sizeof(strnvval));
		snprintf(strnvval, sizeof(strnvval), "%d", WBD_FBT_DEF_AP);
		wbd_nvram_prefix_set(prefix, NVRAM_FBT_AP, strnvval);
		rc_flags |= WBD_FLG_NV_COMMIT|WBD_FLG_RC_RESTART;
	}

	/* [3] Save NVRAM R0KH_ID for this BSS */
	wbd_get_r0khid(prefix, r0kh_id, sizeof(r0kh_id), 1);

	/* [4] Save NVRAM R0KH_KEY for this BSS */
	wbd_get_r0khkey(prefix, r0kh_key, sizeof(r0kh_key), 1);

	/* [5] Get/Generate R1KH_ID for this BSS */
	wbd_get_r1khid(prefix, r1kh_id, sizeof(r1kh_id), 1);

	/* [6] Get MDID from NVRAM, if not present generate the mdid */
	mdid = wbd_get_mdid(prefix);
	memset(new_value, 0, sizeof(new_value));
	snprintf(new_value, sizeof(new_value), "%d", mdid);
	rc_flags |= wbd_nvram_prefix_match_set(prefix, NVRAM_FBT_MDID, new_value, FALSE);

	/* [7] If FBT OVERDS NVRAM is not defined. set it to default */
	nvval = nvram_get(strcat_r(prefix, NVRAM_FBT_OVERDS, data));
	if (!nvval) {
		memset(strnvval, 0, sizeof(strnvval));
		snprintf(strnvval, sizeof(strnvval), "%d", WBD_FBT_DEF_OVERDS);
		wbd_nvram_prefix_set(prefix, NVRAM_FBT_OVERDS, strnvval);
		rc_flags |= WBD_FLG_NV_COMMIT|WBD_FLG_RC_RESTART;
	}

	/* [8] Read fbt reassoc time NVRAM and set default */
	invval = wbd_nvram_safe_get_int(prefix, NVRAM_FBT_REASSOC_TIME, 0);
	if (invval <= 0) {
		memset(strnvval, 0, sizeof(strnvval));
		snprintf(strnvval, sizeof(strnvval), "%d", WBD_FBT_DEF_REASSOC_TIME);
		wbd_nvram_prefix_set(prefix, NVRAM_FBT_REASSOC_TIME, strnvval);
		rc_flags |= WBD_FLG_NV_COMMIT|WBD_FLG_RC_RESTART;
	}

	/* If any NVRAM is updated, do NVRAM commit */
	if (rc_flags & WBD_FLG_NV_COMMIT) {
		nvram_commit();
		WBD_RC_PRINT("Prefix[%s] FBT NVRAMs Modified. Needs Rc Restart.\n", prefix);
	}

end:
	return rc_flags;

}

#endif /* WLHOSTFBT */

/* Read "wbd_ifnames" NVRAM and get actual ifnames */
int
wbd_read_actual_ifnames(char *wbd_ifnames, int len, bool isWBD)
{
	int ret = WBDE_OK;
	int map_mode = MAP_MODE_FLAG_DISABLED;
	char var_intf[IFNAMSIZ] = {0}, bss_info_names[IFNAMSIZ], prefix[IFNAMSIZ];
	char *tmp_intf;
	uint32 rc_flags = 0;

	if (!wbd_ifnames) {
		WBD_RC_PRINT("wbd_ifname pointer NULL\n");
		ret = WBDE_INV_ARG;
		goto end;
	}

	memset(wbd_ifnames, 0, len);

	/* Get the NVRAM : Wi-Fi Blanket Application Mode */
	map_mode = wbd_nvram_safe_get_int(NULL, NVRAM_MAP_MODE, MAP_MODE_FLAG_DISABLED);
	WBD_RC_PRINT("map_mode[%d]\n", map_mode);

	if (MAP_IS_DISABLED(map_mode)) {
		WBD_RC_PRINT("map_mode[%d] is Disabled\n", map_mode);
		return WBDE_INV_MODE;
	}

#ifdef WLHOSTFBT
	/* If Device is Controller, Initialize the FBT NVRAMs */
	if (isWBD && (MAP_IS_CONTROLLER(map_mode))) {
		/* Read and update bss info for each bss name provided */
		WBDSTRNCPY(bss_info_names, wbd_nvram_safe_get(NVRAM_MAP_BSS_NAMES),
			sizeof(bss_info_names));

		/* Traverse bss_info_names for each ifname */
		foreach(var_intf, bss_info_names, tmp_intf) {

			snprintf(prefix, sizeof(prefix), "%s_", var_intf);
			rc_flags |= wbd_uap_init_fbt_nvram_config(prefix);
			/* For FBT related NVRAMs on controller, just the NVRAM commit is enough */
			rc_flags &= ~WBD_FLG_RC_RESTART;
		}
	}
#endif /* WLHOSTFBT */

	/* Get NVRAM value for "wbd_ifnames" */
	tmp_intf = wbd_nvram_safe_get(WBD_NVRAM_IFNAMES);
	if (tmp_intf[0] != '\0') {
		WBDSTRNCPY(wbd_ifnames, tmp_intf, len);
	} else {
		/* will create it later, so just get out */
		ret = WBDE_WBD_IFNAMES_NEXST;
		WBD_RC_PRINT("NVRAM not exists\n");
		goto end;
	}

	WBD_RC_PRINT("wbd_ifnames[%s]\n", wbd_ifnames);

end:
	/* If required, Execute nvram commit/rc restart/reboot commands */
	if (rc_flags) {
		wbd_do_rc_restart_reboot(rc_flags);
	}

	return ret;
}

/* Find Number of valid interfaces */
int
wbd_count_interfaces(void)
{
	char nvram_name[32], ifname[32];
	int ret, index, total = 0;

	/* Find out the wl interface index for the specified interface. */
	for (index = 0; index < DEV_NUMIFS; ++index) {

		snprintf(nvram_name, sizeof(nvram_name), "wl%d_ifname", index);
		WBDSTRNCPY(ifname, nvram_safe_get(nvram_name), sizeof(ifname));

		WBD_RC_PRINT("nvram_name=%s, ifname=%s\n", nvram_name, ifname);

		/* Check if valid Wireless Interface and its Primary Radio is enabled or not */
		if (!wbd_is_ifr_enabled(ifname, FALSE, &ret)) {
			continue; /* Skip non-Wireless & disabled Wireless Interface */
		}

		/* Valid Wireless Interface, Add it to count */
		total ++;
		WBD_RC_PRINT("unit=%d, total=%d\n", index, total);
	}

	return total;
}

/* Get the mask for all the rules defined in flag */
static uint32
wbd_weak_sta_get_select_mask(unsigned int flags)
{
	uint32 mask = 0;

	if (flags & WBD_WEAK_STA_POLICY_FLAG_RULE) {
		mask = ((flags) & (WBD_WEAK_STA_POLICY_FLAG_RSSI |
			WBD_WEAK_STA_POLICY_FLAG_TX_RATE |
			WBD_WEAK_STA_POLICY_FLAG_TX_FAIL));
	}

	return (mask);
}

/* Common algo to compare STA Stats and Thresholds, & identify if STA is Weak or not */
int
wbd_weak_sta_identification(struct ether_addr *sta_mac, wbd_weak_sta_metrics_t *sta_stats,
	wbd_weak_sta_policy_t *thresholds, int *out_fail_cnt, int *out_weak_flag)
{
	int isweak = FALSE, fail_cnt = 0;
	bool check_rule = 0;
	uint32 check_rule_mask = 0; /* a sta select config mask for AND logic */
	uint32 sta_rule_met = 0; /* processed mask for AND logic */

	/* check_rule, check_rule_mask and sta_rule_met are used for checking policy flags in AND
	 * and OR format. If the check_rule is TRUE, then all the rules defined in flags
	 * should be met. If it is FALSE, the STA will be weak if any one of the rules defined
	 * in the flag is met.
	 */
	check_rule = (thresholds->flags & WBD_WEAK_STA_POLICY_FLAG_RULE);

	/* Skipped idle, or active STA */
	if (thresholds->flags & WBD_WEAK_STA_POLICY_FLAG_ACTIVE_STA) {

		if (sta_stats->idle_rate &&
			(sta_stats->idle_rate > thresholds->t_idle_rate)) {

			WBD_RC_PRINT("Skip Active STA["MACF"] Data_rate[%d] threshold[%d]\n",
				ETHERP_TO_MACF(sta_mac),
				sta_stats->idle_rate, thresholds->t_idle_rate);

			return isweak;
		}
	}

	/* check for RSSI Threshold */
	if (thresholds->flags & WBD_WEAK_STA_POLICY_FLAG_RSSI) {

		sta_stats->hysterisis = abs(sta_stats->rssi - sta_stats->last_weak_rssi);

		WBD_RC_PRINT("sta->rssi[%d] sta->last_weak_rssi[%d] hysterisis[%d]\n\n",
			sta_stats->rssi, sta_stats->last_weak_rssi, sta_stats->hysterisis);

		if ((sta_stats->rssi < thresholds->t_rssi) ||
			((sta_stats->last_weak_rssi < thresholds->t_rssi) &&
			(sta_stats->hysterisis < thresholds->t_hysterisis))) {

			sta_rule_met |= WBD_WEAK_STA_POLICY_FLAG_RSSI;
			WBD_RC_PRINT("rssi[%d] threshold[%d] hysterisis[%d] threshold[%d]\n",
				sta_stats->rssi, thresholds->t_rssi,
				sta_stats->hysterisis, thresholds->t_hysterisis);
			fail_cnt++;
			if (check_rule == 0) {
				goto weakfound;
			}
		}
	}

	/* Check for Tx Rate Threshold */
	if (thresholds->flags & WBD_WEAK_STA_POLICY_FLAG_TX_RATE) {

		if (sta_stats->tx_rate &&
			(sta_stats->tx_rate < thresholds->t_tx_rate)) {

			sta_rule_met |= WBD_WEAK_STA_POLICY_FLAG_TX_RATE;
			WBD_RC_PRINT("tx_rate[%d] threshold[%d]\n",
				sta_stats->tx_rate, thresholds->t_tx_rate);
			fail_cnt++;
			if (check_rule == 0) {
				goto weakfound;
			}
		}
	}

	/* check for Tx Failure threshold */
	if (thresholds->flags & WBD_WEAK_STA_POLICY_FLAG_TX_FAIL) {

		if (sta_stats->tx_failures &&
			(sta_stats->tx_failures > thresholds->t_tx_failures)) {

			sta_rule_met |= WBD_WEAK_STA_POLICY_FLAG_TX_FAIL;
			WBD_RC_PRINT("tx_failures[%d] threshold[%d]\n",
				sta_stats->tx_failures, thresholds->t_tx_failures);
			fail_cnt++;
			if (check_rule == 0) {
				goto weakfound;
			}
		}
	}

weakfound:
	/* If the fail_cnt is not 0, then its a weak client */
	if (fail_cnt != 0) {
		/* logic AND all for the above. If check rule is TRUE, the rules which are
		 * met should be equal to the rules defined in the flags
		 */
		if (check_rule) {
			check_rule_mask = wbd_weak_sta_get_select_mask(thresholds->flags);
			WBD_RC_PRINT("STA["MACF"] fail_cnt[%d] AND check check_rule_mask[0x%X]\n",
				ETHERP_TO_MACF(sta_mac), fail_cnt, check_rule_mask);
			if ((sta_rule_met & check_rule_mask) == check_rule_mask) {
				WBD_RC_PRINT("STA["MACF"] AND logic rule met - 0x%X\n",
					ETHERP_TO_MACF(sta_mac), sta_rule_met);
			} else {
				WBD_RC_PRINT("STA["MACF"] AND logic rule not met - 0x%X, skip\n",
					ETHERP_TO_MACF(sta_mac), sta_rule_met);
				goto end;
			}
		}

		isweak = TRUE;
	}

end:
	if (out_fail_cnt) {
		*out_fail_cnt = fail_cnt;
	}

	if (out_weak_flag) {
		*out_weak_flag = sta_rule_met;
	}

	return isweak;
}

int
disable_map_bh_bss(char *name, char *ifname, int bsscfg_idx)
{
	int map, map_mode, macmode, mac_filter = 1;
	char prefix[IFNAMSIZ];
	char maclist_buf[WLC_IOCTL_MAXLEN];
	maclist_t *maclist = NULL;

	map_mode = wbd_nvram_safe_get_int(NULL, NVRAM_MAP_MODE, MAP_MODE_FLAG_DISABLED);
	if (MAP_IS_CONTROLLER(map_mode)) {
		/* Do not disable backhaul BSS of controller */
		return 0;
	}

	wbd_get_prefix(ifname, prefix, sizeof(prefix));
	map = wbd_nvram_safe_get_int(prefix, NVRAM_MAP, 0);
	if (!I5_IS_BSS_BACKHAUL(map)) {
		/* Do not disable if this is not a MAP backhaul BSS */
		return 0;
	}

	WBD_RC_PRINT("MAC Block MAP backhaul BSS (%s) till controller is detected\n", ifname);
	macmode = htod32(WLC_MACMODE_ALLOW);
	if (wl_ioctl(ifname, WLC_SET_MACMODE, &macmode, sizeof(macmode)) != 0) {
		WBD_RC_PRINT("%s: WLC_SET_MACMODE failed to set to WLC_MACMODE_ALLOW\n", ifname);
	}

	memset(maclist_buf, 0, WLC_IOCTL_MAXLEN);
	maclist = (maclist_t *)maclist_buf;
	maclist->count = 0;
	maclist->count = htod32(maclist->count);
	if (wl_ioctl(ifname, WLC_SET_MACLIST, maclist,
		(ETHER_ADDR_LEN * maclist->count + sizeof(uint32))) != 0) {
		WBD_RC_PRINT("%s: WLC_SET_MACLIST failed to set to MACLIST NONE\n", ifname);
	}

	wl_iovar_setint(ifname, "probresp_mac_filter", mac_filter);

	return 0;
}
