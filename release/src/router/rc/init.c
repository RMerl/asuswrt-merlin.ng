/*

	Copyright 2005, Broadcom Corporation
	All Rights Reserved.

	THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
	KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
	SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.

*/

#include "rc.h"

#include <termios.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <time.h>
#include <errno.h>
#include <paths.h>
#include <strings.h>
#include <sys/wait.h>
#include <sys/reboot.h>
#include <sys/klog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/resource.h>
#ifdef LINUX26
#if defined(RTCONFIG_HND_ROUTER_AX_6756)
#include <sys/sysmacros.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#endif
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
#include <confmtd_utils.h>
#endif

#ifdef RTCONFIG_RALINK
#include <ralink.h>
#include "flash_mtd.h"	//FRead
#endif

#ifdef RTCONFIG_QCA
#include <qca.h>
#endif

#ifdef RTCONFIG_ALPINE
#include <alpine.h>
#endif

#include <shutils.h>

#include <shared.h>
#include <stdio.h>
#ifdef RTCONFIG_USB
#include <disk_io_tools.h>
#include <disk_share.h>
#endif

#include <sys/utsname.h>

#ifdef RTCONFIG_BCMFA
#include <sysdeps.h>
#endif

#if defined(RTCONFIG_LP5523)
#include <lp5523led.h>
#endif

#if defined(RTCONFIG_RGBLED)
#include <aura_rgb.h>
#endif

#ifdef RTCONFIG_ISP_CUSTOMIZE
#include <sys/statfs.h>
#endif

#define SHELL "/bin/sh"
#define LOGIN "/bin/login"

#ifdef RTCONFIG_WLMODULE_MT7663E_AP
#include <l1profile.h>
#endif

#ifdef RTCONFIG_AMAS
#include <amas-utils.h>
#include <amas_cap.h>
#endif

#ifdef RTCONFIG_CFGSYNC
#include <cfg_slavelist.h>
#endif

#ifdef RTCONFIG_SCHED_V2
#include <sched_v2.h>
#endif

#if defined(RTCONFIG_DWB)
#include <amas_dwb.h>
#endif

#ifdef RTCONFIG_COMFW
#include <comfw.h>
#endif
#include <version.h>

#ifdef RTCONFIG_HND_ROUTER_AX_6756
#include <bcm_flashutil.h>
#include <bcm_hwdefs.h>
#endif

#ifdef RTCONFIG_OPENVPN
#include <openvpn_config.h>
#endif

static int fatalsigs[] = {
	SIGILL,
	SIGABRT,
	SIGFPE,
	SIGPIPE,
	SIGBUS,
	SIGSYS,
	SIGTRAP,
	SIGPWR
};

static int initsigs[] = {
	SIGHUP,
	SIGUSR1,
	SIGUSR2,
	SIGINT,
	SIGQUIT,
	SIGALRM,
	SIGTERM
};

static char *defenv[] = {
	"USER=root",	/* keep it on first item and modified at run-time. */
	"TERM=vt100",
	"HOME=/",
	//"PATH=/usr/bin:/bin:/usr/sbin:/sbin",
#ifdef RTCONFIG_LANTIQ
	"PATH=/opt/usr/bin:/opt/bin:/opt/usr/sbin:/opt/sbin:/usr/bin:/bin:/usr/sbin:/sbin:/rom/opt/lantiq/bin:/rom/opt/lantiq/usr/sbin",
#else
	"PATH=/opt/usr/bin:/opt/bin:/opt/usr/sbin:/opt/sbin:/usr/bin:/bin:/usr/sbin:/sbin",
#endif
#ifdef HND_ROUTER
	"LD_LIBRARY_PATH=/lib:/usr/lib:/lib/aarch64",
#endif
#ifdef RTCONFIG_BCM_MFG
	"PS1=# ",
#endif
#ifdef RTCONFIG_LANTIQ
	"LD_LIBRARY_PATH=/lib:/usr/lib:/opt/lantiq/usr/lib:/opt/lantiq/usr/sbin/:/tmp/wireless/lantiq/usr/lib/",
#endif
	"SHELL=" SHELL,
	"TMOUT=0",
#ifdef RTCONFIG_DMALLOC
/*
 * # dmalloc -DV
 * Debug Tokens:
 * none -- no functionality (0)
 * log-stats -- log general statistics (0x1)
 * log-non-free -- log non-freed pointers (0x2)
 * log-known -- log only known non-freed (0x4)
 * log-trans -- log memory transactions (0x8)
 * log-admin -- log administrative info (0x20)
 * log-bad-space -- dump space from bad pnt (0x100)
 * log-nonfree-space -- dump space from non-freed pointers (0x200)
 * log-elapsed-time -- log elapsed-time for allocated pointer (0x40000)
 * log-current-time -- log current-time for allocated pointer (0x80000)
 * check-fence -- check fence-post errors (0x400)
 * check-heap -- check heap adm structs (0x800)
 * check-blank -- check mem overwritten by alloc-blank, free-blank (0x2000)
 * check-funcs -- check functions (0x4000)
 * check-shutdown -- check heap on shutdown (0x8000)
 * catch-signals -- shutdown program on SIGHUP, SIGINT, SIGTERM (0x20000)
 * realloc-copy -- copy all re-allocations (0x100000)
 * free-blank -- overwrite freed memory with \0337 byte (0xdf) (0x200000)
 * error-abort -- abort immediately on error (0x400000)
 * alloc-blank -- overwrite allocated memory with \0332 byte (0xda) (0x800000)
 * print-messages -- write messages to stderr (0x2000000)
 * catch-null -- abort if no memory available (0x4000000)
 *
 * debug=0x4e48503 = log_stats, log-non-free, log-bad-space, log-elapsed-time, check-fence, check-shutdown, free-blank, error-abort, alloc-blank, catch-null
 * debug=0x3 = log-stats, log-non-free
 */
	"DMALLOC_OPTIONS=debug=0x2,inter=100,log=/jffs/dmalloc_%d.log",
#endif
#if defined(HND_ROUTER) && !defined(RTCONFIG_BCM_MFG)
	"ENV=/etc/profile",
#endif
	NULL
};

#ifdef RTCONFIG_ISP_CUSTOMIZE
#define NEED_TO_SET_DEFAULT_VALUE(name) (restore_defaults_g && !nvram_invmatch(name, ""))
#else
#define NEED_TO_SET_DEFAULT_VALUE(name) (restore_defaults_g)
#endif

extern int set_tcode_misc();
extern int g_upgrade;
int def_boot = 0;

#ifdef RTCONFIG_ASD
	extern int monitor_asd(void);
#endif

#ifdef RTCONFIG_WPS
static void
wps_restore_defaults(void)
{
	char macstr[128];
	int i;

	/* cleanly up nvram for WPS */
	nvram_unset("wps_config_state");
	nvram_unset("wps_proc_status");
	nvram_unset("wps_proc_status_x");
	nvram_unset("wps_config_method");
	nvram_unset("wps_band");

	nvram_unset("wps_restart");
	nvram_unset("wps_proc_mac");

	nvram_unset("wps_sta_devname");
	nvram_unset("wps_sta_mac");

	nvram_unset("wps_pinfail");
	nvram_unset("wps_pinfail_mac");
	nvram_unset("wps_pinfail_name");
	nvram_unset("wps_pinfail_state");

	nvram_unset("wps_enr_ssid");
	nvram_unset("wps_enr_bssid");
	nvram_unset("wps_enr_wsec");

	nvram_set("wps_device_name", get_productid());
	nvram_set("wps_modelnum", get_productid());

	strcpy(macstr, get_lan_hwaddr());

	if (strlen(macstr))
		for (i = 0; i < strlen(macstr); i++)
			macstr[i] = tolower(macstr[i]);
	nvram_set("boardnum", nvram_get("serial_no") ? : macstr);
}
#endif /* RTCONFIG_WPS */

static void
virtual_radio_restore_defaults(void)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXXXXXXXXXXXXXXXX_mssid_";
	int i, j;

	nvram_unset("unbridged_ifnames");
	nvram_unset("ure_disable");

	/* Delete dynamically generated variables */
	for (i = 0; i < MAX_NVPARSE; i++) {
		sprintf(prefix, "wl%d_", i);
		nvram_unset(strcat_r(prefix, "vifs", tmp));
		nvram_unset(strcat_r(prefix, "ssid", tmp));
		nvram_unset(strcat_r(prefix, "guest", tmp));
		nvram_unset(strcat_r(prefix, "ure", tmp));
		nvram_unset(strcat_r(prefix, "ipconfig_index", tmp));
		nvram_unset(strcat_r(prefix, "nas_dbg", tmp));
#ifdef RTCONFIG_PSR_GUEST
		nvram_unset(strcat_r(prefix, "psr_mbss", tmp));
#endif
		sprintf(prefix, "lan%d_", i);
		nvram_unset(strcat_r(prefix, "ifname", tmp));
		nvram_unset(strcat_r(prefix, "ifnames", tmp));
		nvram_unset(strcat_r(prefix, "gateway", tmp));
		nvram_unset(strcat_r(prefix, "proto", tmp));
		nvram_unset(strcat_r(prefix, "ipaddr", tmp));
		nvram_unset(strcat_r(prefix, "netmask", tmp));
		nvram_unset(strcat_r(prefix, "lease", tmp));
		nvram_unset(strcat_r(prefix, "stp", tmp));
		nvram_unset(strcat_r(prefix, "hwaddr", tmp));
		sprintf(prefix, "dhcp%d_", i);
		nvram_unset(strcat_r(prefix, "start", tmp));
		nvram_unset(strcat_r(prefix, "end", tmp));

		/* clear virtual versions */
		for (j = 0; j < 16; j++) {
			sprintf(prefix, "wl%d.%d_", i, j);
			nvram_unset(strcat_r(prefix, "ssid", tmp));
			nvram_unset(strcat_r(prefix, "ipconfig_index", tmp));
			nvram_unset(strcat_r(prefix, "guest", tmp));
			nvram_unset(strcat_r(prefix, "closed", tmp));
			nvram_unset(strcat_r(prefix, "wpa_psk", tmp));
			nvram_unset(strcat_r(prefix, "auth", tmp));
			nvram_unset(strcat_r(prefix, "wep", tmp));
			nvram_unset(strcat_r(prefix, "auth_mode", tmp));
			nvram_unset(strcat_r(prefix, "crypto", tmp));
			nvram_unset(strcat_r(prefix, "akm", tmp));
			nvram_unset(strcat_r(prefix, "hwaddr", tmp));
			nvram_unset(strcat_r(prefix, "bss_enabled", tmp));
#ifdef CONFIG_BCMWL5
			nvram_unset(strcat_r(prefix, "bss_maxassoc", tmp));
			nvram_unset(strcat_r(prefix, "wme_bss_disable", tmp));
#endif
			nvram_unset(strcat_r(prefix, "ifname", tmp));
			nvram_unset(strcat_r(prefix, "unit", tmp));
			nvram_unset(strcat_r(prefix, "ap_isolate", tmp));
			nvram_unset(strcat_r(prefix, "macmode", tmp));
#ifdef CONFIG_BCMWL5
			nvram_unset(strcat_r(prefix, "maclist", tmp));
			nvram_unset(strcat_r(prefix, "maxassoc", tmp));
#endif
			nvram_unset(strcat_r(prefix, "mode", tmp));
			nvram_unset(strcat_r(prefix, "radio", tmp));
			nvram_unset(strcat_r(prefix, "radius_ipaddr", tmp));
			nvram_unset(strcat_r(prefix, "radius_port", tmp));
			nvram_unset(strcat_r(prefix, "radius_key", tmp));
			nvram_unset(strcat_r(prefix, "key", tmp));
			nvram_unset(strcat_r(prefix, "key1", tmp));
			nvram_unset(strcat_r(prefix, "key2", tmp));
			nvram_unset(strcat_r(prefix, "key3", tmp));
			nvram_unset(strcat_r(prefix, "key4", tmp));
			nvram_unset(strcat_r(prefix, "wpa_gtk_rekey", tmp));
			nvram_unset(strcat_r(prefix, "nas_dbg", tmp));
			nvram_unset(strcat_r(prefix, "probresp_mf", tmp));

			nvram_unset(strcat_r(prefix, "bss_opmode_cap_reqd", tmp));
			nvram_unset(strcat_r(prefix, "mcast_regen_bss_enable", tmp));
			nvram_unset(strcat_r(prefix, "wmf_bss_enable", tmp));
			nvram_unset(strcat_r(prefix, "preauth", tmp));
			nvram_unset(strcat_r(prefix, "dwds", tmp));
			nvram_unset(strcat_r(prefix, "acs_dfsr_deferred", tmp));
			nvram_unset(strcat_r(prefix, "wet_tunnel", tmp));
			nvram_unset(strcat_r(prefix, "bridge", tmp));
			nvram_unset(strcat_r(prefix, "mfp", tmp));
			nvram_unset(strcat_r(prefix, "acs_dfsr_activity", tmp));
			nvram_unset(strcat_r(prefix, "acs_dfsr_immediate", tmp));
			nvram_unset(strcat_r(prefix, "wme", tmp));
			nvram_unset(strcat_r(prefix, "net_reauth", tmp));
			nvram_unset(strcat_r(prefix, "pmk_cache", tmp));
#ifdef CONFIG_BCMWL5
			nvram_unset(strcat_r(prefix, "sta_retry_time", tmp));
			nvram_unset(strcat_r(prefix, "infra", tmp));
#ifdef __CONFIG_HSPOT__
			nvram_unset(strcat_r(prefix, "hsflag", tmp));
			nvram_unset(strcat_r(prefix, "hs2cap", tmp));
			nvram_unset(strcat_r(prefix, "opercls", tmp));
			nvram_unset(strcat_r(prefix, "anonai", tmp));
			nvram_unset(strcat_r(prefix, "wanmetrics", tmp));
			nvram_unset(strcat_r(prefix, "oplist", tmp));
			nvram_unset(strcat_r(prefix, "homeqlist", tmp));
			nvram_unset(strcat_r(prefix, "osu_ssid", tmp));
			nvram_unset(strcat_r(prefix, "osu_frndname", tmp));
			nvram_unset(strcat_r(prefix, "osu_uri", tmp));
			nvram_unset(strcat_r(prefix, "osu_nai", tmp));
			nvram_unset(strcat_r(prefix, "osu_method", tmp));
			nvram_unset(strcat_r(prefix, "osu_icons", tmp));
			nvram_unset(strcat_r(prefix, "osu_servdesc", tmp));
			nvram_unset(strcat_r(prefix, "concaplist", tmp));
			nvram_unset(strcat_r(prefix, "qosmapie", tmp));
			nvram_unset(strcat_r(prefix, "gascbdel", tmp));
			nvram_unset(strcat_r(prefix, "iwnettype", tmp));
			nvram_unset(strcat_r(prefix, "hessid", tmp));
			nvram_unset(strcat_r(prefix, "ipv4addr", tmp));
			nvram_unset(strcat_r(prefix, "ipv6addr", tmp));
			nvram_unset(strcat_r(prefix, "netauthlist", tmp));
			nvram_unset(strcat_r(prefix, "venuegrp", tmp));
			nvram_unset(strcat_r(prefix, "venuetype", tmp));
			nvram_unset(strcat_r(prefix, "venuelist", tmp));
			nvram_unset(strcat_r(prefix, "ouilist", tmp));
			nvram_unset(strcat_r(prefix, "3gpplist", tmp));
			nvram_unset(strcat_r(prefix, "domainlist", tmp));
			nvram_unset(strcat_r(prefix, "realmlist", tmp));
#endif /* __CONFIG_HSPOT__ */

#if defined(__CONFIG_VISUALIZATION__) && defined(CONFIG_VISUALIZATION_ENABLED)
			nvram_unset(strcat_r(prefix, "rrm", tmp));
#endif /* __CONFIG_VISUALIZATION__ && CONFIG_VISUALIZATION_ENABLED */
#endif
		}
	}
}

#ifdef RTCONFIG_BCMARM
void
misc_ioctrl(void)
{
	/* default WAN_RED on  */
	int model = get_model();
	switch(model) {
		case MODEL_RTAC87U:
		case MODEL_RTAC68U:
		case MODEL_RTAC3200:
		case MODEL_RTAC5300:
		case MODEL_GTAC5300:
		case MODEL_RTAC88U:
		case MODEL_RTAC86U:
		case MODEL_RTAC3100:
		case MODEL_RTAX88U:
		case MODEL_BC109:
		case MODEL_EBG19:
		case MODEL_EBG15:
		case MODEL_EBP15:
		case MODEL_GTAX11000:
		case MODEL_RTAX92U:
		case MODEL_RTAX95Q:
		case MODEL_XT8PRO:
		case MODEL_BM68:
		case MODEL_XT8_V2:
		case MODEL_RTAXE95Q:
		case MODEL_ET8PRO:
		case MODEL_ET8_V2:
		case MODEL_RTAX56_XD4:
		case MODEL_XD4PRO:
		case MODEL_CTAX56_XD4:
		case MODEL_RTAX58U:
		case MODEL_RTAX82U_V2:
		case MODEL_TUFAX5400_V2:
		case MODEL_RTAX5400:
		case MODEL_RTAX82_XD6S:
		case MODEL_XD6_V2:
		case MODEL_RTAX58U_V2:
		case MODEL_RTAXE7800:
		case MODEL_TUFAX3000_V2:
		case MODEL_RTAX3000N:
		case MODEL_BR63:
		case MODEL_RTAX55:
		case MODEL_RTAX56U:
		case MODEL_RPAX56:
		case MODEL_RPAX58:
		case MODEL_RTAX86U:
		case MODEL_RTAX68U:
		case MODEL_RTAC68U_V4:
		case MODEL_DSLAX82U:
		case MODEL_GTAX6000:
		case MODEL_GTAXE11000:
		case MODEL_GTAX11000_PRO:
		case MODEL_GTAXE16000:
		case MODEL_ET12:
		case MODEL_XT12:
		case MODEL_RTAX88U_PRO:
#ifdef RTAC68U
			if (!is_ac66u_v2_series() && !is_ac68u_v3_series())
				return;
#endif

#ifdef HND_ROUTER
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(EBG19_504)
			bcm53134_led_control(2);
#endif
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(TUFAX5400_V2) || defined(RTAX5400) || defined(RTAX88U_PRO)
#if defined(TUFAX3000_V2) || defined(RTAXE7800)
			if (!nvram_get_int("wans_extwan"))
#endif
			lan_phy_led_pinmux(0);
#endif
			setLANLedOn();
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400)
			LEDGroupReset(LED_OFF);
			setLEDGroupOn();
#endif
#ifdef GTAX6000
			AntennaGroupReset(LED_ON);
			setAntennaGroupOn();
#ifndef RTCONFIG_BCM_MFG
			eval("sw", "0xff803140", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00040000");
			eval("sw", "0xff803150", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00080000");
			eval("sw", "0xff803160", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00100000");
			eval("sw", "0xff803170", "0x0000e002", "0x00c34a24", "0x00000a24", "0x00800080");
			eval("sw", "0xff80301c", "0x00200000");
#endif
#endif

#if defined(RTCONFIG_BCM_MFG) || defined(GTAC2900)
#ifdef GTAC2900
			eval("sw", "0x800c00a0", "0");	// disable event on tx/rx activity
#else
#if defined(RTAX58U_V2) || defined(GTAX6000) || defined(RTAX3000N) || defined(BR63) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX88U_PRO) || defined(RTAX5400)
			wan_phy_led_pinmux(0);
#endif
			led_control(LED_WAN_NORMAL, LED_ON);
#endif
			return;
#endif
#endif

#if defined(RPAX56) || defined(RPAX58)
			if (factory_debug()) {
				_dprintf("\n Enter ATE mode\n");
				bcm_cled_ctrl(BCM_CLED_RED, BCM_CLED_SLOW_BLINK);
			}
#endif
#if defined(XT12) || defined(ET12)
			bcm_cled_ctrl_single_white(BCM_CLED_WHITE, BCM_CLED_STEADY_NOBLINK);
#endif
			if (is_router_mode()) {
				if (nvram_get_int("wanduck_down"))
					return;

				led_control(LED_WAN, LED_ON);
#ifdef HND_ROUTER
#ifndef GTAC2900
#if defined(RTAX58U_V2) || defined(GTAX6000) || defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(RTAX3000N) || defined(BR63) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX88U_PRO) || defined(RTAX5400)
				set_specific_wan_white_led(0, LED_OFF);
#else
				led_control(LED_WAN_NORMAL, LED_OFF);
#endif
#endif
#else
				eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01fe");
				eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0x01fe");
#endif
			}
#ifdef HND_ROUTER
#ifndef GTAC2900
			else {
#if defined(RTAX58U_V2) || defined(GTAX6000) || defined(RTAX3000N) || defined(BR63) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX88U_PRO) || defined(RTAX5400)
				wan_phy_led_pinmux(0);
#else
				led_control(LED_WAN_NORMAL, LED_ON);
#endif
			}
#endif
#endif

			break;
#if defined(RTAX86U_PRO)
		case MODEL_RTAX86U_PRO:
			eval("sw", "0xff800500", "0x000bc1b6"); // orig value: 0x0001c006, let GPIO 4/5/7/8/17/19 be OUTPUT
			eval("sw", "0xff800520", "0xfdcbfdfe"); // orig value: 0xfdcbfdda or 0xfdcbfddb, let GPIO 4/5/7/8/17/19 be HIGH. let GPIO2 be HIGH to avoid POWER LED is on abnormally
			//eval("sw", "0xff800520", "0xfdc1fc4a"); // orig value: 0xfdcbfdda or 0xfdcbfddb, let GPIO 4/5/7/8/17/19 be LOW

			if (is_router_mode()) {
				_dprintf("%s: default turn WAN_RED on\n", __func__);
				if (nvram_get_int("wanduck_down"))
					return;

				led_control(LED_WAN, LED_ON);
				set_specific_wan_white_led(0, LED_OFF); // will turn off LED_WAN_NORMAL
			}

			break;
#endif
	}
}
#endif

/* assign none-exist value */
void
wl_defaults(void)
{
	struct nvram_tuple *t;
	char prefix[]="wlXXXXXXXXXXXXX_", tmp[100], tmp2[100];
	char pprefix[]="wlXXXXXXXXXXXXX_", *pssid;
	char word[256], *next;
	int unit, subunit;
	char wlx_vifnames[128], wl_vifnames[128], lan_ifnames[128];
	char wlx_vifnames2[128], wl_vifnames2[128];
	int subunit_x = 0;
#if defined(RTCONFIG_AMAS)
	char sta_ifnames[64]={0};
#endif
	int i;
#if !defined(RTCONFIG_FBWIFI)
	char *fbwifi_iface[3] = { "fbwifi_2g", "fbwifi_5g", "fbwifi_5g_2" };
#endif
#if defined (RTCONFIG_WLMODULE_RT3352_INIC_MII)
	int inic_vlan_id = INIC_VLAN_ID_START;
	char nic_lan_ifnames[64];
	char *pNic;
#endif
	unsigned int max_mssid;

	memset(wl_vifnames, 0, sizeof(wl_vifnames));
	memset(wlx_vifnames2, 0, sizeof(wlx_vifnames2));
	memset(wl_vifnames2, 0, sizeof(wl_vifnames2));
	memset(lan_ifnames, 0, sizeof(lan_ifnames));
#if defined (RTCONFIG_WLMODULE_RT3352_INIC_MII)
	memset(nic_lan_ifnames, 0, sizeof(nic_lan_ifnames));
	pNic = nic_lan_ifnames;
#endif

	if (!nvram_get("wl_country_code"))
		nvram_set("wl_country_code", "");

#ifdef RTCONFIG_LANTIQ
	int j, wlc_band = nvram_get_int("wlc_band");

	// only for the all SKIP_ABSENT_BAND().
	for(i = 0; i < MAX_NR_WL_IF; ++i){
		snprintf(prefix, sizeof(prefix), "wl%d_", i);

		//for(j = 0; j < MAX_NO_MSSID; ++j){
		for(j = 0; j < 1; ++j){
			snprintf(pprefix, sizeof(pprefix), "wl%d.%d_", i, j+1);

			if(!nvram_get(strcat_r(pprefix, "nband", tmp2)) || strlen(nvram_safe_get(tmp2)) <= 0)
				nvram_set(tmp2, nvram_safe_get(strcat_r(prefix, "nband", tmp)));
		}
	}
#endif

#if !defined(RTCONFIG_FBWIFI)
	/* If Facebook Wi-Fi has been enabled in another firmware, disable corresponding wlX.Y.
	 * If we don't do this, Open System VAP will be created later.
	 */
	for (i = 0; i < min(MAX_NR_WL_IF, ARRAY_SIZE(fbwifi_iface)); ++i) {
		SKIP_ABSENT_BAND(i);
		if (!nvram_get(fbwifi_iface[i]))
			continue;
		if (sscanf(nvram_safe_get(fbwifi_iface[i]), "wl%d.%d", &unit, &subunit) != 2)
			continue;

		nvram_unset(fbwifi_iface[i]);
		nvram_set("fbwifi_enable", "off");
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
		if (nvram_pf_match(prefix, "bss_enabled", "0"))
			continue;
		nvram_pf_set(prefix, "bss_enabled", "0");
	}
#endif
#ifdef RTCONFIG_BHCOST_OPT
	if (nvram_get_int("re_mode") == 1)
	{
		unit = 0;
		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			snprintf(prefix, sizeof(prefix), "amas_wlc%d_", unit);
			if(nvram_get(strcat_r(prefix, "check_vsie", tmp)) == NULL)
				nvram_pf_set_int(prefix, "check_vsie", 1);
			if(nvram_get(strcat_r(prefix, "ss_count", tmp)) == NULL)
				nvram_pf_set_int(prefix, "ss_count", 2);
			if(nvram_get(strcat_r(prefix, "target_bssid", tmp)) == NULL)
				nvram_pf_set(prefix, "target_bssid", "");
			unit++;
		}
	}
#endif

	unit = 0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

		for (t = router_defaults; t->name; t++) {
#ifdef CONFIG_BCMWL5
			if (!strncmp(t->name, "wl", 2) && strncmp(t->name, "wl_", 3) && strncmp(t->name, "wlc", 3) && !strcmp(&t->name[4], "nband"))
				nvram_set(t->name, t->value);
#endif
			if (strncmp(t->name, "wl_", 3)!=0) continue;
#ifdef CONFIG_BCMWL5
			if (!strcmp(&t->name[3], "nband") && nvram_match(strcat_r(prefix, &t->name[3], tmp), "-1"))
				nvram_set(strcat_r(prefix, &t->name[3], tmp), t->value);
#endif
			if (!nvram_get(strcat_r(prefix, &t->name[3], tmp))) {
				/* Add special default value handle here */
#ifdef RTCONFIG_EMF
				/* Wireless IGMP Snooping */
				if (strncmp(&t->name[3], "igs", sizeof("igs")) == 0) {
					char *value = nvram_get(strcat_r(prefix, "wmf_bss_enable", tmp2));
					nvram_set(tmp, (value && *value) ? value : t->value);
				} else
#endif
#ifdef RTCONFIG_WLMODULE_MT7615E_AP
				if(!strcmp(tmp,"wl0_txbf") || !strcmp(tmp,"wl0_txbf_en") ){
				nvram_set(tmp, "0");
				}else{
				nvram_set(tmp, t->value);
				}
#else
				nvram_set(tmp, t->value);
#endif
	}
		}

		unit++;
	}

//	virtual_radio_restore_defaults();

	unit = 0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		memset(wlx_vifnames, 0, sizeof(wlx_vifnames));
		snprintf(pprefix, sizeof(pprefix), "wl%d_", unit);
#if defined(RTCONFIG_CONCURRENTREPEATER)
		/* don't support guest network on AP mode for RP-AC66 */
		if (sw_mode() == SW_MODE_AP
#if defined(RTCONFIG_REALTEK) && defined(RTCONFIG_AMAS)
/*  in re_mode the device need to use wlx.1 as AP for realtek model
*/
			&& !nvram_match("re_mode", "1")
#endif
			)
			continue;
#endif
		/* including primary ssid */
		max_mssid = num_of_mssid_support(unit);
#if defined(RTCONFIG_PSR_GUEST) && !defined(HND_ROUTER)
		max_mssid++;
#endif
		for (subunit = 1; subunit < max_mssid+1; subunit++)
		{
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
#if defined(RTCONFIG_REALTEK) && defined(RTCONFIG_AMAS)
			if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1") && subunit == 1){
				nvram_set(strcat_r(prefix, "bss_enabled", tmp), "1");
			}
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
			if (sw_mode() == SW_MODE_REPEATER) {
#ifdef RTCONFIG_REALTEK
/* [MUST]: Why to do this ?  */
				nvram_set(strcat_r(prefix, "bss_enabled", tmp), "0");
#else
				if (
#if defined(RTCONFIG_CONCURRENTREPEATER)
					(nvram_get_int("wlc_express") == 0 || (nvram_get_int("wlc_express") - 1) != unit ) && subunit==1
#else
					nvram_get_int("wlc_band")==unit && subunit==1
#endif
				   )
				{
					nvram_set(strcat_r(prefix, "bss_enabled", tmp), "1");
				}
				else {
					nvram_set(strcat_r(prefix, "bss_enabled", tmp), "0");
				}
#endif
			}
#endif

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
			if (is_psta(unit))
				nvram_set(strcat_r(prefix, "bss_enabled", tmp), "0");
			else if (is_psr(unit) && (subunit == 1))
				nvram_set(strcat_r(prefix, "bss_enabled", tmp), "1");
#endif
#if (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)) && defined(RTCONFIG_AMAS)	/* handle wlX.1_ in AiMesh RE mode */
			if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1") && subunit == 1) {
				char wlifname[8];
#if defined(RTCONFIG_QCA)
				__get_wlifname(swap_5g_band(unit), 0, wlifname);
#elif defined(RTCONFIG_RALINK)
				__get_wlifname(unit, 0, wlifname);
#endif
				nvram_set(strcat_r(prefix, "bss_enabled", tmp), "1");
				nvram_set(strcat_r(prefix, "ifname", tmp), wlifname);
				continue;
			}
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_VIF_ONBOARDING)
			set_onboarding_vif_bss_enabled(unit, subunit);
#endif

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_MSSID_PRELINK)
			set_mssid_prelink_bss_enabled(unit, subunit);
#endif

#ifdef RTCONFIG_OWE_TRANS
			set_owe_transition_bss_enabled(unit, subunit);
#endif
			if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
			{
				int len = strlen(wlx_vifnames);
				subunit_x++;
				sprintf(wlx_vifnames + len, "%s%s", len?" ":"", get_wlifname(unit, subunit, subunit_x, tmp));
			}
			else
				nvram_set(strcat_r(prefix, "bss_enabled", tmp), "0");

#if !defined(RTCONFIG_REALTEK) && defined(RTCONFIG_CONCURRENTREPEATER)
			if (nvram_get_int("wlc_express") == 1 || nvram_get_int("wlc_express") == 2) {
				if (unit == (nvram_get_int("wlc_express") - 1)) {
					subunit_x++;

					if (strlen(wlx_vifnames) == 0)
						sprintf(wlx_vifnames, "%s", get_wlifname(unit, subunit, subunit_x, tmp));

				}
				else
					memset(wlx_vifnames, 0x0, 32);
			}
#endif
#ifndef RTCONFIG_LANTIQ
//			if (!nvram_get(strcat_r(prefix, "ifname", tmp)))
			{
				snprintf(tmp2, sizeof(tmp2), "wl%d.%d", unit, subunit);
//				nvram_set(strcat_r(prefix, "ifname", tmp), tmp2);
				nvram_set(strcat_r(prefix, "ifname", tmp), get_wlifname(unit, subunit, subunit_x, tmp2));
			}
#endif

			if (!nvram_get(strcat_r(prefix, "unit", tmp)))
			{
				snprintf(tmp2, sizeof(tmp2), "%d.%d", unit, subunit);
				nvram_set(strcat_r(prefix, "unit", tmp), tmp2);
			}

			if (!nvram_get(strcat_r(prefix, "bw_enabled", tmp)))
			{
				nvram_set(strcat_r(prefix, "bw_enabled", tmp), "0");
				nvram_set(strcat_r(prefix, "bw_dl", tmp), "");
				nvram_set(strcat_r(prefix, "bw_ul", tmp), "");
			}

#ifdef RTCONFIG_GN_WBL
			if (!nvram_get(strcat_r(prefix, "gn_wbl_enable", tmp)))
			{
				nvram_set(strcat_r(prefix, "gn_wbl_enable", tmp), "0");
				nvram_set(strcat_r(prefix, "gn_wbl_type", tmp), "1");
				nvram_set(strcat_r(prefix, "gn_wbl_rule", tmp), "");
			}
#endif

			if (!nvram_get(strcat_r(prefix, "ssid", tmp))) {
				pssid = nvram_default_get(strcat_r(pprefix, "ssid", tmp));
				if (strlen(pssid))
#if defined(RTCONFIG_CONCURRENTREPEATER)
					sprintf(tmp2, "%s", pssid);
#else
					sprintf(tmp2, "%s_Guest%d", pssid, subunit);
#endif
				else sprintf(tmp2, "%s_Guest%d", SSID_PREFIX, subunit);
				nvram_set(strcat_r(prefix, "ssid", tmp), tmp2);
			}

			if (!nvram_get(strcat_r(prefix, "auth_mode_x", tmp)))
			{
#if defined(RTCONFIG_WIFI6E)
				if(nvram_match(strcat_r(pprefix, "nband", tmp), "4"))
					nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "owe");
				else
#endif
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "open");
				nvram_set(strcat_r(prefix, "auth_mode", tmp), "none");
				nvram_set(strcat_r(prefix, "akm", tmp), "");
				nvram_set(strcat_r(prefix, "auth", tmp), "0");
				nvram_set(strcat_r(prefix, "wep_x", tmp), "0");
#ifdef RTCONFIG_RALINK
#elif defined(RTCONFIG_QCA)
#else
				nvram_set(strcat_r(prefix, "wep", tmp), "disabled");
#endif
				nvram_set(strcat_r(prefix, "crypto", tmp), "aes");
				nvram_set(strcat_r(prefix, "key", tmp), "1");
				nvram_set(strcat_r(prefix, "key1", tmp), "");
				nvram_set(strcat_r(prefix, "key2", tmp), "");
				nvram_set(strcat_r(prefix, "key3", tmp), "");
				nvram_set(strcat_r(prefix, "key4", tmp), "");
				nvram_set(strcat_r(prefix, "wpa_psk", tmp), "");

				nvram_set(strcat_r(prefix, "ap_isolate", tmp), "0");
				nvram_set(strcat_r(prefix, "bridge", tmp), "");
#ifdef RTCONFIG_RALINK
#elif defined(RTCONFIG_QCA)
#elif defined(RTCONFIG_REALTEK)
				nvram_set(strcat_r(prefix, "bss_maxassoc", tmp), "31");
#else
				nvram_set(strcat_r(prefix, "bss_maxassoc", tmp), "128");
#endif
				nvram_set(strcat_r(prefix, "closed", tmp), "0");
				nvram_set(strcat_r(prefix, "infra", tmp), "1");
				nvram_set(strcat_r(prefix, "macmode", tmp), "disabled");
#ifdef RTCONFIG_RALINK
#elif defined(RTCONFIG_QCA)
#elif defined(RTCONFIG_REALTEK)
				nvram_set(strcat_r(prefix, "maxassoc", tmp), "31");
#elif !defined(CONFIG_BCMWL5)
				nvram_set(strcat_r(prefix, "maxassoc", tmp), "128");
#endif
				nvram_set(strcat_r(prefix, "mode", tmp), "ap");
				//nvram_set(strcat_r(prefix, "net_reauth", tmp), "3600");
				nvram_set(strcat_r(prefix, "net_reauth", tmp), "36000");
				nvram_set(strcat_r(prefix, "preauth", tmp), "");
				nvram_set(strcat_r(prefix, "radio", tmp), "1");
				nvram_set(strcat_r(prefix, "radius_ipaddr", tmp), "");
				nvram_set(strcat_r(prefix, "radius_key", tmp), "");
				nvram_set(strcat_r(prefix, "radius_port", tmp), "1812");
#ifdef RTCONFIG_RALINK
#elif defined(RTCONFIG_QCA)
#elif defined(RTCONFIG_REALTEK)
#else
				nvram_set(strcat_r(prefix, "sta_retry_time", tmp), "5");
				nvram_set(strcat_r(prefix, "wfi_enable", tmp), "0");
				nvram_set(strcat_r(prefix, "wfi_pinmode", tmp), "0");
#endif
				nvram_set(strcat_r(prefix, "wme", tmp), "auto");
#ifdef RTCONFIG_RALINK
#elif defined(RTCONFIG_QCA)
#else
				nvram_set(strcat_r(prefix, "wme_bss_disable", tmp), "0");
#endif
#ifdef RTCONFIG_EMF
				nvram_set(strcat_r(prefix, "wmf_bss_enable", tmp),
					nvram_get(strcat_r(pprefix, "wmf_bss_enable", tmp)));
#else
				nvram_set(strcat_r(prefix, "wmf_bss_enable", tmp), "0");
#endif
				nvram_set(strcat_r(prefix, "wpa_gtk_rekey", tmp), nvram_default_get(strcat_r(prefix, "wpa_gtk_rekey", tmp2)));
#ifdef RTCONFIG_RALINK
#elif defined(RTCONFIG_QCA)
#else
				nvram_set(strcat_r(prefix, "wps_mode", tmp), "disabled");
#endif
				nvram_set(strcat_r(prefix, "expire", tmp), "0");
				nvram_set(strcat_r(prefix, "lanaccess", tmp), "off");
			}
#if defined (RTCONFIG_WLMODULE_RT3352_INIC_MII)
			if (is_router_mode())					// Only limite access of Guest network in Router mode
			if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1"))
			{
				if (strcmp(word, "rai0") == 0)
				{
					int vlan_id;
					if (nvram_match(strcat_r(prefix, "lanaccess", tmp), "off"))
						vlan_id = inic_vlan_id++;	// vlan id for no LAN access
					else
						vlan_id = 0;			// vlan id allow LAN access
					if (vlan_id)
						pNic += sprintf(pNic, "vlan%d ", vlan_id);
				}
			}
#endif
		}

		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

		if (strlen(wlx_vifnames))
		{
			int len;

			nvram_set(strcat_r(prefix, "vifs", tmp), wlx_vifnames);
#if defined(RTCONFIG_AMAS)
			len = strlen(sta_ifnames);
			sprintf(sta_ifnames + len, "%s%s", len?" ":"", wlx_vifnames);
#endif

			len = strlen(wl_vifnames);
			sprintf(wl_vifnames + len, "%s%s", len?" ":"", wlx_vifnames);

#if defined(RTCONFIG_CONCURRENTREPEATER)
			if(len == 0) {
					if (nvram_get_int("wlc_express") != 0) {
						nvram_set(strcat_r(prefix, "bss_enabled", tmp), "1");
					}
			}
#endif
		}
		else {
			nvram_set(strcat_r(prefix, "vifs", tmp), "");
#if defined(RTCONFIG_CONCURRENTREPEATER)
#ifndef RTCONFIG_REALTEK
			if (nvram_get_int("wlc_express") != 0) {
				nvram_set(strcat_r(prefix, "bss_enabled", tmp), "0");
				nvram_set(strcat_r(prefix, "ssid", tmp), "");
			}
#endif
#endif
		}

		unit++;
		subunit_x = 0;
	}

#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_CONCURRENTREPEATER) && !defined(RTCONFIG_REALTEK)
	nvram_set("sta_ifnames", sta_ifnames);
	nvram_set("sta_phy_ifnames", sta_ifnames);
#endif
#ifdef RTCONFIG_VIF_ONBOARDING
	set_onboarding_vif_security();
#endif
#endif

#if defined (RTCONFIG_WLMODULE_RT3352_INIC_MII)
	nvram_set("nic_lan_ifnames", nic_lan_ifnames); //reset value
	if (strlen(nic_lan_ifnames))
	{
		sprintf(wl_vifnames, "%s %s", wl_vifnames, nic_lan_ifnames);	//would be add to "lan_ifnames" in following code.
	}
#endif
	if (strlen(wl_vifnames))
	{
		strcpy(lan_ifnames, nvram_safe_get("lan_ifnames"));
		foreach (word, wl_vifnames, next)
			add_to_list(word, lan_ifnames, sizeof(lan_ifnames));

		nvram_set("lan_ifnames", lan_ifnames);
	}

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	reset_psr_hwaddr();
#endif
}

/* for WPS Reset */
void
wl_default_wps(int unit)
{
	struct nvram_tuple *t;
	char prefix[]="wlXXXXXX_", tmp[100];
#ifdef RTCONFIG_CONCURRENTREPEATER
	char *pssid = NULL, tmp2[100]={0};
#endif
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	for (t = router_defaults; t->name; t++) {
		if (strncmp(t->name, "wl_", 3) &&
			strncmp(t->name, prefix, 4))
			continue;

		if (strcmp(t->name, "wl_ssid") &&
		    strcmp(t->name, "wl_auth_mode_x") &&
		    strcmp(t->name, "wl_wep_x") &&
		    strcmp(t->name, "wl_key") &&
		    strcmp(t->name, "wl_key1") &&
		    strcmp(t->name, "wl_key2") &&
		    strcmp(t->name, "wl_key3") &&
		    strcmp(t->name, "wl_key4") &&
		    strcmp(t->name, "wl_phrase_x") &&
		    strcmp(t->name, "wl_crypto") &&
		    strcmp(t->name, "wl_wpa_psk"))
			continue;

		if (!strncmp(t->name, "wl_", 3)) {
			nvram_set(strcat_r(prefix, &t->name[3], tmp), t->value);
		}
		else
			nvram_set(t->name, t->value);
	}

#ifdef RTCONFIG_CONCURRENTREPEATER
				pssid = nvram_default_get(strcat_r(prefix, "ssid", tmp));
				if (strlen(pssid)){
					sprintf(tmp2, "%s", pssid);
					nvram_set(strcat_r(prefix, "ssid", tmp), tmp2);
				}
#endif
}

void
reset_rp_def_ssid()
{
	char ssid[32];
	char word[256], *next;
	int unit, subunit;
	unsigned int max_mssid;
	char prefix[]="wlXXXXXX_", tmp[100];

	char *macp = NULL;
	unsigned char mac_binary[6];
	macp = get_2g_hwaddr();
	ether_atoe(macp, mac_binary);

#ifndef RTCONFIG_BCM_MFG
	if((dpsr_mode() || dpsta_mode() || rp_mode()) && nvram_get_int("re_mode") == 0)
		_dprintf("%s\n", __func__);
	else {
		_dprintf("%s: abort due not client mode\n", __func__);
		return;
	}
#endif

	unit = 0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		max_mssid = num_of_mssid_support(unit);
#if defined(RPAX56) || defined(RPAX58)
                if(unit==0) {
			_dprintf("%s, reset wl0_ssid\n", __func__);
                        sprintf(ssid, "%s_%02X%02X%02X%02X%02X%02X", "ASUS_RP", mac_binary[0], mac_binary[1], mac_binary[2], mac_binary[3], mac_binary[4], mac_binary[5]);
                } else {	
			_dprintf("%s, reset wl1_ssid\n", __func__);
			sprintf(ssid, "%s_%02X%02X%02X%02X%02X%02X_5G", "ASUS_RP", mac_binary[0], mac_binary[1], mac_binary[2], mac_binary[3], mac_binary[4], mac_binary[5]);
		}
#else
		sprintf(ssid, "%s_%02X", "ASUS_RP", mac_binary[5]);
#endif
		nvram_set(strcat_r(prefix, "ssid", tmp), ssid);


		unit++;
	}
}

void
restore_rp_defaults_wifi(int all)
{
	char ssid[32];
	char word[256], *next;
	int unit, subunit;
	unsigned int max_mssid;
	char prefix[]="wlXXXXXX_", tmp[100];

	char *macp = NULL;
	unsigned char mac_binary[6];
	macp = get_2g_hwaddr();
	ether_atoe(macp, mac_binary);

	unit = 0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		max_mssid = num_of_mssid_support(unit);
#if defined(RPAX56) || defined(RPAX58)
		if(unit==0)
			sprintf(ssid, "%s_%02X", "ASUS_RP", mac_binary[5]);
		else {
			_dprintf("%s, reset wl1_ssid due rpv3 rule\n", __func__);
			sprintf(ssid, "%s_%02X_5G", "ASUS_RP", mac_binary[5]);
		}
#else
		sprintf(ssid, "%s_%02X", "ASUS_RP", mac_binary[5]);
#endif
		nvram_set(strcat_r(prefix, "ssid", tmp), ssid);


		if (all)
		for (subunit = 1; subunit < max_mssid+1; subunit++) {
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);

			sprintf(ssid, "%s_%02X", "ASUS_RP", mac_binary[5]);

				nvram_set(strcat_r(prefix, "ssid", tmp), ssid);

		}

		unit++;
	}

	nvram_set("w_Setting", "1");

}

void
restore_defaults_wifi(int all)
{
	int rev3 = 0;
	char ssid[32];
	int defpsk = strlen(nvram_safe_get("wifi_psk")) && nvram_contains_word("rc_support", "defpsk");
	char word[256], *next;
	int unit, subunit;
	unsigned int max_mssid;
	char prefix[]="wlXXXXXX_", tmp[100];
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
	int prelink = nvram_invmatch("amas_bdlkey", "");
	int plk_unit = get_prelink_unit();
#endif

#if defined(RTCONFIG_NEWSSID_REV2) || defined(RTCONFIG_NEWSSID_REV4) || defined(RTCONFIG_NEWSSID_REV5)
	rev3 = 1;
#endif
	unit = 0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		max_mssid = num_of_mssid_support(unit);

		strlcpy(ssid, get_default_ssid(unit, 0), sizeof(ssid));
#ifndef RTCONFIG_SSID_AMAPS
		if (defpsk
			|| (rev3
#ifdef RTAC68U
			&& is_ssid_rev3_series()
#endif
		))
#endif
			nvram_set(strcat_r(prefix, "ssid", tmp), ssid);

		if (defpsk)
		{
			nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk2");
			nvram_set(strcat_r(prefix, "crypto", tmp), "aes");
			nvram_set(strcat_r(prefix, "wpa_psk", tmp), nvram_safe_get("wifi_psk"));
		}

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK) && !defined(RTCONFIG_MSSID_PRELINK)
		/* set prelink config on main */
		if (prelink && plk_unit == unit)
			set_prelink_config(prefix);
#endif

		if (all)
		for (subunit = 1; subunit < max_mssid+1; subunit++) {
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);

			strlcpy(ssid, get_default_ssid(unit, subunit), sizeof(ssid));
#ifndef RTCONFIG_SSID_AMAPS
			if (defpsk
				|| (rev3
#ifdef RTAC68U
				&& is_ssid_rev3_series()
#endif
			))
#endif
				nvram_set(strcat_r(prefix, "ssid", tmp), ssid);

			if (defpsk)
			{
				nvram_set(strcat_r(prefix, "auth_mode_x", tmp), "psk2");
				nvram_set(strcat_r(prefix, "crypto", tmp), "aes");
				nvram_set(strcat_r(prefix, "wpa_psk", tmp), nvram_safe_get("wifi_psk"));
			}
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK) && defined(RTCONFIG_MSSID_PRELINK)
			/* set prelink config on max mssid */
			if (prelink && plk_unit == unit && nvram_get_int("plk_cap_subunit") == subunit) {
#ifdef RTCONFIG_FRONTHAUL_DWB
				if (!fronthaul_DWB_profile_generated(unit, subunit))
#endif
					set_prelink_config(prefix);
			}
#endif
		}

		unit++;
	}

#ifdef RTCONFIG_SINGLE_SSID
	if (strlen(nvram_safe_get("wifi_psk")))
#else
	if (defpsk)
#endif
	nvram_set("w_Setting", "1");
}

void
wl_defaults_wps(void)
{
	char word[256], *next;
	int unit;

	unit = 0;
	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		wl_default_wps(unit);
		unit++;
	}

#ifdef RTCONFIG_TCODE
	restore_defaults_wifi(0);
#endif
}

void
restore_defaults_lan(int restore_defaults)
{
	//modify by sherry 2016.2.3
	if(restore_defaults){
		//if (!strcmp(nvram_safe_get("territory_code"), "US/01"))
		if (nvram_contains_word("rc_support", "defip"))
		{
			nvram_set("lan_ipaddr", "192.168.50.1");
			nvram_set("lan_ipaddr_rt", "192.168.50.1");
			nvram_set("dhcp_start", "192.168.50.2");
			nvram_set("dhcp_end", "192.168.50.254");
		}
	}
}

/* assign none-exist value or inconsist value */
void
lan_defaults(int restore_defaults)
{
	if (is_router_mode() && nvram_invmatch("lan_proto", "static"))
		nvram_set("lan_proto", "static");
#ifdef RTCONFIG_TCODE
	restore_defaults_lan(restore_defaults);
#endif
}

#ifdef RTCONFIG_TAGGED_BASED_VLAN
void
tagged_vlan_defaults(void)
{
	char *buf, *g, *p;
	char *mac, *ip, *gateway, *lan_ipaddr, *dns;
	char dhcp_staticlist[sizeof(DHCP_STATICLIST_EXAMPLE) * STATIC_MAC_IP_BINDING_PER_LAN + 8];			/* 2240 + 8 */
	char subnet_rulelist[(sizeof(SUBNET_RULE_EXAMPLE) + sizeof(SUBNET_STATICLIST_EXAMPLE) * STATIC_MAC_IP_BINDING_PER_VLAN) * (VLAN_MAX_NUM - 1) + sizeof(dhcp_staticlist)];	/* 2954 + 2240 + 8 */
	char subnet_rulelist_ext_default[24]={0};

	memset(subnet_rulelist, 0, sizeof(subnet_rulelist));
	memset(dhcp_staticlist, 0, sizeof(dhcp_staticlist));
	nvram_set("vlan_enable", "0");
	nvram_set("vlan_rulelist", "<1>1>0>0000>00FF00FF>007F>007F>default>1>0");
	nvram_set("vlan_if_list","00>00FF>007F>007F");
	nvram_set("vlan_pvid_list","1>1>1>1>1>1>1>1");

	g = buf = strdup(nvram_default_get("dhcp_staticlist") ? : "");
	while (buf && (p = strsep(&g, "<")) != NULL) {
		if ((vstrsep(p, ">", &mac, &ip, &dns)) < 2)
			continue;
		if (*dhcp_staticlist)
			strlcat(dhcp_staticlist, ";", sizeof(dhcp_staticlist));
		strlcat(dhcp_staticlist, mac, sizeof(dhcp_staticlist));
		strlcat(dhcp_staticlist, " ", sizeof(dhcp_staticlist));
		strlcat(dhcp_staticlist, ip, sizeof(dhcp_staticlist));
	}
	free(buf);

	lan_ipaddr = nvram_safe_get("lan_ipaddr");
	gateway = nvram_safe_get("dhcp_gateway_x");
	gateway = (*gateway && inet_addr(gateway)) ? gateway : lan_ipaddr;

	sprintf(subnet_rulelist, "<%s>%s>%s>%s>%s>%s>%s>%s>%s>%s>%s", gateway,
		nvram_default_get("lan_netmask"), nvram_default_get("dhcp_enable_x"), nvram_default_get("dhcp_start"),
		nvram_default_get("dhcp_end"), nvram_default_get("dhcp_lease"), nvram_default_get("lan_domain"),
		nvram_default_get("dhcp_dns1_x"), nvram_default_get("dhcp_wins_x"), nvram_default_get("dhcp_static_x"), dhcp_staticlist);

	nvram_set("subnet_rulelist", subnet_rulelist);

	sprintf(subnet_rulelist_ext_default,"<%s>",gateway);
	nvram_set("subnet_rulelist_ext", subnet_rulelist_ext_default);

}
#endif
/* assign none-exist value */
void
wan_defaults(void)
{
	struct nvram_tuple *t;
	char prefix[]="wanXXXXXX_", tmp[100];
	char word[256], *next;
	int unit;

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		nvram_set(strcat_r(prefix, "ifname", tmp), "");

		for(t = router_defaults; t->name; ++t){
			if(strncmp(t->name, "wan_", 4)!=0) continue;

			if(!nvram_get(strcat_r(prefix, &t->name[4], tmp))){
				nvram_set(tmp, t->value);
			}
		}
	}

	unit = WAN_UNIT_FIRST;
	foreach(word, nvram_safe_get("wan_ifnames"), next){
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		if(dualwan_unit__nonusbif(unit))
			nvram_set(strcat_r(prefix, "ifname", tmp), word);

		++unit;
	}

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6) {
		unit = WAN_UNIT_IPTV;
		foreach (word, nvram_safe_get("iptv_wan_ifnames"), next) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			nvram_set(strcat_r(prefix, "ifname", tmp), word);
			if (nvram_match("switch_wantag", "singtel"))
				nvram_set(strcat_r(prefix, "vendorid", tmp),"S_iptvsys");

			for (t = router_defaults; t->name; t++) {
				if (strncmp(t->name, "wan_", 4) != 0) continue;

				if (!nvram_get(strcat_r(prefix, &t->name[4], tmp))) {
					nvram_set(tmp, t->value);
				}
			}
			++unit;
		}
	}
#endif
}

void
restore_defaults_module(char *prefix)
{
	struct nvram_tuple *t;

	if (strcmp(prefix, "wl_")==0) wl_defaults();
	else if (strcmp(prefix, "wan_")==0) wan_defaults();
	else {
		for (t = router_defaults; t->name; t++) {
			if (strncmp(t->name, prefix, strlen(prefix))!=0) continue;
			nvram_set(t->name, t->value);
		}
	}
}

int restore_defaults_g = 0;

#ifdef RTCONFIG_USB
#ifndef RTCONFIG_PERMISSION_MANAGEMENT
// increase this number each time need to reset usb related setting
// acc_num/acc_username/acc_password to acc_num/acc_list
#define USBCTRLVER 1

void usbctrl_default()
{
	char tmp[1024], *ptr;
	int buf_len = 0;
	int usbctrlver;
	int acc_num, acc_num_ret, i;
	char *acc_user, *acc_password;
	char acc_nvram_username[32], acc_nvram_password[32];
	char **account_list;
	char http_passwd[128];
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char enc_passwd_buf[256];
#endif

	usbctrlver = nvram_get_int("usbctrlver");
	if (usbctrlver != USBCTRLVER || nvram_get("acc_username0") != NULL) {
		nvram_set_int("usbctrlver", USBCTRLVER);
		acc_num = nvram_get_int("acc_num");
		acc_num_ret = 0;

		char ascii_passwd[128] = {0};
		memset(ascii_passwd, 0, sizeof(ascii_passwd));

		strlcpy(http_passwd, nvram_safe_get("http_passwd"), sizeof(http_passwd));
#ifdef RTCONFIG_NVRAM_ENCRYPT
		int declen = strlen(http_passwd)+1;
		char *dec_passwd = NULL;

		dec_passwd = malloc(declen);
		if(dec_passwd){
			memset(dec_passwd, 0, declen);
			pw_dec(http_passwd, dec_passwd, declen, 1);
			char_to_ascii_safe(ascii_passwd, dec_passwd, sizeof(ascii_passwd));
			free(dec_passwd);
		}

		int enclen = pw_enc_blen(ascii_passwd) + 1;
		char *enc_passwd = NULL;
		enc_passwd = malloc(enclen);
		if(enc_passwd){
			pw_enc(ascii_passwd, enc_passwd, 1);
			strlcpy(ascii_passwd, enc_passwd, sizeof(ascii_passwd));
			free(enc_passwd);
		}
#else
		char_to_ascii_safe(ascii_passwd, http_passwd, sizeof(ascii_passwd));
#endif
		buf_len = snprintf(tmp, sizeof(tmp), "%s>%s", nvram_safe_get("http_username"), ascii_passwd); // default account.

		ptr = tmp+buf_len;
		acc_num_ret = 1;

		for(i = 0; i < acc_num; i++) {
			sprintf(acc_nvram_username, "acc_username%d", i);
			if ((acc_user = nvram_get(acc_nvram_username)) == NULL) continue;

			sprintf(acc_nvram_password, "acc_password%d", i);
			if ((acc_password = nvram_get(acc_nvram_password)) == NULL) continue;
			memset(ascii_passwd, 0, 84);
			char_to_ascii_safe(ascii_passwd, acc_password, 84);
#ifdef RTCONFIG_NVRAM_ENCRYPT
			memset(enc_passwd_buf, 0, sizeof(enc_passwd_buf));
			pw_enc(ascii_passwd, enc_passwd_buf, 1);
			strlcpy(ascii_passwd, enc_passwd_buf, sizeof(ascii_passwd));
#endif
			if (strcmp(acc_user, "admin")) {
				buf_len += snprintf(ptr, 256-buf_len, "<%s>%s", acc_user, ascii_passwd);
				ptr = tmp+buf_len;
				acc_num_ret++;
			}

			nvram_unset(acc_nvram_username);
			nvram_unset(acc_nvram_password);
		}

		nvram_set_int("acc_num", acc_num_ret);
		nvram_set("acc_list", tmp);
	}

	acc_num = get_account_list(&acc_num, &account_list);
	if (acc_num >= 0 && acc_num != nvram_get_int("acc_num"))
		nvram_set_int("acc_num", acc_num);
	free_2_dimension_list(&acc_num, &account_list);
}
#endif
#endif

#define RESTORE_DEFAULTS() \
	(!nvram_match("restore_defaults", "0") || \
	 !nvram_match("nvramver", RTCONFIG_NVRAM_VER))	// nvram version mismatch

#if defined(RTCONFIG_DETWAN)
void set_defwan(char *wan, char *wanmask, char *lanmask)
{
	nvram_set("wan_ifnames", wan);
	nvram_set("wan0_ifname", wan);
	nvram_set("wan0_gw_ifname", wan);
	nvram_set("detwan_ifname", wan);
	nvram_set("detwan_phy", wan);
	nvram_set("wanports_mask", wanmask);
	nvram_set("lanports_mask", lanmask);
	nvram_set("detwan_wan_mask", wanmask);
	nvram_set("detwan_lan_mask", lanmask);
}
#endif

void set_default_wifi_rate(void)
{
	char *path="/etc/wifi_rate";
	char buf[128], output[128];
	char *tmp, *delim=" ";
	int offset=0;

	if (check_if_file_exist(path)) return;

	memset(buf, '\0', sizeof(buf));
	memset(output, '\0', sizeof(output));

	strncpy(buf, nvram_safe_get("wl_ifnames"), sizeof(buf));
	tmp = strtok(buf, delim);
	while (tmp!=NULL) {
		offset += snprintf(output+offset, sizeof(output)-offset, "%s:%s\n", tmp, !strncmp(tmp+3, "0", 1)?"400":"866.7");
		tmp = strtok(NULL, delim);
	}

	f_write_string(path, output, 0, 0);
}

/* Clean lanX_ifname and lanX_ifname for vlan */
void clean_vlan_ifnames(void)
{
	int i = 0;
	char nv[32];

	/* Check the ifname of lan_ifnames whether existing in other lanX_ifnames for vlan */
	for (i = 1; i <= VLAN_MAXVID; i++) {
		/* Unset lanX_ifname */
		memset(nv, 0x0, sizeof(nv));
		snprintf(nv, sizeof(nv), "lan%d_ifname", i);
		nvram_unset(nv);

		/* Unset lanX_ifnames */
		memset(nv, 0x0, sizeof(nv));
		snprintf(nv, sizeof(nv), "lan%d_ifnames", i);
		nvram_unset(nv);

		/* Unset lanX_subnet */
		memset(nv, 0x0, sizeof(nv));
		snprintf(nv, sizeof(nv), "lan%d_subnet", i);
		nvram_unset(nv);
	}

	nvram_unset("vlan_index");
	nvram_unset("lan_ifnames_t");
}

void
convert_defaults()
{
	if (nvram_get("wl_TxPower")) {
#ifdef RTCONFIG_RALINK
		nvram_set_int("wl0_txpower", nvram_get_int("wl0_TxPower"));
		nvram_set_int("wl1_txpower", nvram_get_int("wl1_TxPower"));
#else
		nvram_set_int("wl0_txpower", min(100 * nvram_get_int("wl0_TxPower") / 80, 100));
		nvram_set_int("wl1_txpower", min(100 * nvram_get_int("wl1_TxPower") / 80, 100));
#endif
		nvram_unset("wl_TxPower");
		nvram_unset("wl0_TxPower");
		nvram_unset("wl1_TxPower");

		nvram_commit();
	} else if (nvram_get("dms_rescan")) {
		nvram_unset("dms_rescan");
		nvram_commit();
	}
}

#ifdef RTCONFIG_AVBLCHAN
void
avblchan_defaults()
{
	int unit = 0;
	char word[128], *next = NULL, prefix[]="wlxxx", tmp[128];

	foreach (word, nvram_safe_get("wl_ifnames"), next) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
		nvram_unset(strcat_r(prefix, "unavbl_ch", tmp));
#else
		nvram_set(strcat_r(prefix, "acs_excl_chans_base", tmp), "");
		nvram_set(strcat_r(prefix, "acs_excl_chans_cfg", tmp), "");
		nvram_set(strcat_r(prefix, "acs_excl_chans", tmp), "");
#endif
		unit++;
	}

#ifdef CONFIG_BCMWL5
	nvram_set("excbase", "0");
#endif
}
#endif

void
misc_defaults(int restore_defaults)
{
#ifdef RTCONFIG_USB
	char prefix[32];
	int i;
#ifdef RTCONFIG_USB_MODEM
	char tmp[100];
	int modem_unit;
#endif

	// default for USB state control variables. {
	for(i = 1; i <= MAX_USB_PORT; ++i) { // MAX USB port number is 3.
		snprintf(prefix, 32, "usb_led%d", i);
		nvram_unset(prefix);
	}

	/* Unset all usb_path related nvram variables, e.g. usb_path1, usb_path2.4.3, usb_path_. */
	unset_usb_nvram(NULL);

#ifdef RTCONFIG_USB_MODEM
#ifndef RTCONFIG_INTERNAL_GOBI
	// can't support all kinds of modem.
	nvram_set("modem_mode", "0");
#endif

#ifdef RTCONFIG_USB_MULTIMODEM
	clean_modem_state(-1, 1);
#else
	clean_modem_state(MODEM_UNIT_FIRST, 1);
#endif
	// only be unset at boot. {
	for(modem_unit = MODEM_UNIT_FIRST; modem_unit < MODEM_UNIT_MAX; ++modem_unit){
		usb_modem_prefix(modem_unit, prefix, sizeof(prefix));

		nvram_unset(strcat_r(prefix, "act_reset", tmp));
		nvram_unset(strcat_r(prefix, "act_reset_path", tmp));
	}
	// only be unset at boot. }

#ifndef RTCONFIG_INTERNAL_GOBI
	int sim_num = atoi(nvram_safe_get("modem_sim_num"));
	for(i = 1; i <= sim_num; ++i){
		snprintf(prefix, 32, "modem_sim_imsi%d", i);
		nvram_unset(prefix);
	}
#endif
#endif // RTCONFIG_USB_MODEM
#endif	/* RTCONFIG_USB */
	// default for USB state control variables. }

	nvram_set_int("wlready", 0);
	nvram_set_int("init_wl_re", 0);

	/*++ Clean temporary variables of WTFast ++*/
#ifdef RTCONFIG_WTFAST
	nvram_unset("wtf_login");
	nvram_unset("wtf_account_type");
	nvram_unset("wtf_max_clients");
	nvram_unset("wtf_days_left");
	nvram_unset("wtf_game_list");
	nvram_unset("wtf_server_list");
	nvram_unset("wtf_session_hash");
#endif
	/*-- Clean temporary variables of WTFast --*/

	/* Clean lanX_ifname and lanX_ifname for vlan */
	clean_vlan_ifnames();

	/* some default state values is model deps, so handled here*/
	switch(get_model()) {
#ifdef RTCONFIG_BCMARM
#ifdef RTAC56U
		case MODEL_RTAC56U:
			nvram_set(ATE_BRCM_FACTORY_MODE_STR(), "0");
			if(chk_ate_ccode())
				fix_ate_ccode();
#endif
		case MODEL_RTAC56S:
			if (After(get_blver(nvram_safe_get("bl_version")), get_blver("1.0.2.4")))	// since 1.0.2.5
				nvram_set("reboot_time", "140");// default is 70 sec

			if (!After(get_blver(nvram_safe_get("bl_version")), get_blver("1.0.2.6"))) {	// before 1.0.2.7
				nvram_set("wl0_itxbf",	"0");
				nvram_set("wl1_itxbf",	"0");
				nvram_set("wl_itxbf",	"0");
			}

			break;

		case MODEL_GTAC5300:
		case MODEL_RTAC86U:
		case MODEL_RTAX88U:
		case MODEL_BC109:
		case MODEL_EBG19:
		case MODEL_EBG15:
		case MODEL_EBP15:
		case MODEL_GTAX11000:
		case MODEL_RTAX92U:
		case MODEL_RTAX95Q:
		case MODEL_XT8PRO:
		case MODEL_BM68:
		case MODEL_XT8_V2:
		case MODEL_RTAXE95Q:
		case MODEL_ET8PRO:
		case MODEL_ET8_V2:
		case MODEL_RTAX56_XD4:
		case MODEL_XD4PRO:
		case MODEL_CTAX56_XD4:
		case MODEL_RTAX58U:
		case MODEL_RTAX82U_V2:
		case MODEL_TUFAX5400_V2:
		case MODEL_RTAX5400:
		case MODEL_RTAX82_XD6S:
		case MODEL_XD6_V2:
		case MODEL_GT10:
		case MODEL_RTAX9000:
		case MODEL_RTAX58U_V2:
		case MODEL_RTAXE7800:
		case MODEL_TUFAX3000_V2:
		case MODEL_RTAX3000N:
		case MODEL_BR63:
		case MODEL_RTAX55:
		case MODEL_RTAX56U:
		case MODEL_RTAX86U:
		case MODEL_RTAX68U:
		case MODEL_RTAC68U_V4:
		case MODEL_DSLAX82U:
		case MODEL_GTAX6000:
		case MODEL_GTAXE11000:
		case MODEL_GTAX11000_PRO:
		case MODEL_GTAXE16000:
		case MODEL_ET12:
		case MODEL_XT12:
		case MODEL_RTAX86U_PRO:
		case MODEL_RTAX88U_PRO:
			nvram_set("reboot_time", "90");
			break;
		case MODEL_RTAC5300:
		case MODEL_RTAC88U:
		case MODEL_RTAC3100:
			nvram_set("reboot_time", "140");
			break;
		case MODEL_RTAC3200:
		case MODEL_RTAC1200G:
		case MODEL_RTAC1200GP:
		case MODEL_RPAX56:
		case MODEL_RPAX58:
			nvram_set("reboot_time", "80");

			break;
		case MODEL_RTAC68U:
#ifdef RTCONFIG_DUAL_TRX
			nvram_set("reboot_time", "90");		// default is 70 sec
#else
			if (After(get_blver(nvram_safe_get("bl_version")), get_blver("1.0.1.6")))	// since 1.0.1.7
				nvram_set("reboot_time", "140");// default is 70 sec
			else
				nvram_set("reboot_time", "95");	// extend default to 95
#endif
			break;
		case MODEL_RTAC87U:
			nvram_set("reboot_time", "160");
			break;
		case MODEL_RTN18U:
			if (After(get_blver(nvram_safe_get("bl_version")), get_blver("2.0.0.5")))
				nvram_set("reboot_time", "140");// default is 70 sec
			break;
		case MODEL_DSLAC68U:
			nvram_set("reboot_time", "170");	// default is 70 sec
			break;
#endif
		case MODEL_RTN14UHP:
			nvram_set("reboot_time", "85");		// default is 70 sec
			break;
		case MODEL_RTN10U:
			nvram_set("reboot_time", "85");		// default is 70 sec
			break;
#ifdef RTCONFIG_RALINK
#ifdef RTCONFIG_DSL
		case MODEL_DSLN55U:
			nvram_set("reboot_time", "75");		// default is 70 sec
			break;
#else
		case MODEL_RTAC52U:
			nvram_set("reboot_time", "80");		// default is 70 sec
			break;
		case MODEL_RTN56UB1:
		case MODEL_RTN56UB2:
		case MODEL_RTAC1200GU:
		case MODEL_RTN54U:
		case MODEL_RTAC54U:
		case MODEL_RTAC1200HP:
		case MODEL_RTAC51U:
		case MODEL_RTN65U:
		case MODEL_RTAC85U:
		case MODEL_RTAC85P:
		case MODEL_RTACRH26:
		case MODEL_TUFAC1750:
			nvram_set("reboot_time", "90");		// default is 70 sec
			break;
#endif
#endif
#ifdef RTCONFIG_QCA
		case MODEL_RT4GAC55U:
		case MODEL_RTAC55U:
		case MODEL_RTAC55UHP:
		case MODEL_RTN19:
		case MODEL_RTAC59U:
		case MODEL_PLN12:
		case MODEL_PLAC56:
		case MODEL_PLAC66U:
			nvram_set("reboot_time", "80");		// default is 70 sec
			break;

		case MODEL_RTAC58U:
		case MODEL_RT4GAC53U:
		case MODEL_RTAC82U:
			nvram_set("reboot_time", "100");	// default is 70 sec
			break;
		case MODEL_MAPAC1300:
		case MODEL_VZWAC1300:
		case MODEL_SHAC1300:
		case MODEL_MAPAC1750:
		case MODEL_RTAC59CD6R:
		case MODEL_RTAC59CD6N:
		case MODEL_RTAC95U:
		case MODEL_MAPAC2200:
			nvram_set("reboot_time", "80");		// default is 70 sec
			break;

		/* IPQ60XX models */
		case MODEL_PLAX56XP4:
			nvram_set("reboot_time", "110");		// temporarily
			break;

		/* IPQ50XX models */
		case MODEL_ETJ:
		case MODEL_RTAX57Q:
			nvram_set("reboot_time", "110");		// temporarily
			break;

		case MODEL_RTAC88N:
		case MODEL_BRTAC828:
		case MODEL_RTAC88S:
		case MODEL_RTAD7200:
			nvram_set("reboot_time", "110");	// default is 70 sec
#if defined(RTCONFIG_LETSENCRYPT)
			if (nvram_match("le_acme_auth", "dns"))
				nvram_set("le_acme_auth", "http");
#endif
			break;
#endif
		case MODEL_GTAXY16000:
		case MODEL_RTAX89U:
			if (!is_aqr_phy_exist()) {
				nvram_set("reboot_time", "70");	// no need to upload AQR firmware
			} else if (get_soc_version_major() == 2) {
				nvram_set("reboot_time", "90");	// Add 10s for HK2.0
			} else {
				nvram_set("reboot_time", "80");	// default is 70 sec
			}
			break;
#ifdef RTCONFIG_REALTEK
		//case MODEL_RTRTL8198C:
		case MODEL_RPAC68U:
		case MODEL_RPAC53:
		case MODEL_RPAC92:
			nvram_set("reboot_time", "90");
			break;
#endif
#ifdef RTCONFIG_ALPINE
		case MODEL_GTAC9600:
			nvram_set("reboot_time", "90");
			break;
#endif
#ifdef RTCONFIG_LANTIQ
		case MODEL_BLUECAVE:
			nvram_set("reboot_time", "160");
			break;
#endif
		default:
			break;
	}

#ifdef RTCONFIG_WEBDAV
	webdav_account_default();
#endif

	nvram_set("success_start_service", "0");

#ifdef RTCONFIG_AMAS
	nvram_set("start_service_ready", "0");
	nvram_unset("amas_bhctrl_service_ready");
	nvram_unset("amas_wlcconnect_service_ready");
	nvram_unset("amas_lanctrl_service_ready");
	nvram_unset("amas_status_service_ready");
	nvram_unset("amas_misc_service_ready");
	nvram_unset("amas_ssd_service_ready");
#endif

#if defined(RTAC66U) || defined(BCM4352)
	nvram_set("led_5g", "0");
#endif
#if !defined(RTCONFIG_HAS_5G)
	nvram_unset("wl1_ssid");
#endif	/* ! RTCONFIG_HAS_5G */

#if defined(RTCONFIG_QCA)
	nvram_unset("skip_gen_ath_config");
#endif

#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
#ifndef RTCONFIG_USB
	int i;
#endif
	for (i = WL_2G_BAND; i < MAX_NR_WL_IF; ++i) {
		char prefix[sizeof("wlXXXXXX_")];

		SKIP_ABSENT_BAND(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if (nvram_pf_get_int(prefix, "mrate_x") == 17)
			nvram_pf_set(prefix, "mrate_x", "15");
	}
#endif

#ifdef RTCONFIG_USB
	if (nvram_match("smbd_cpage", ""))
	{
		if (nvram_match("wl0_country_code", "CN"))
			nvram_set("smbd_cpage", "936");
		else if (nvram_match("wl0_country_code", "TW"))
			nvram_set("smbd_cpage", "950");
	}
#endif
	/* reset ntp status */
	nvram_set("ntp_ready", "0");
	nvram_set("svc_ready", "0");
#ifdef RTCONFIG_QTN
	nvram_unset("qtn_ready");
#endif
#ifdef RTAC68U
	nvram_unset("fw_enc_crc");
	nvram_unset("fw_check");
#endif

	if (restore_defaults)
	{
#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2)) && !defined(RTCONFIG_JFFS_NVRAM)
		nvram_set("jffs2_clean_fs", "1");
#elif defined(RTCONFIG_YAFFS)
		nvram_set("yaffs_clean_fs", "1");
#elif defined(RTCONFIG_UBIFS) && !defined(RTCONFIG_HND_ROUTER_AX_6756)
		nvram_set("ubifs_clean_fs", "1");
#endif
#ifdef RTCONFIG_QTN
		nvram_set("qtn_restore_defaults", "1");
#endif
	}
#ifdef RTCONFIG_QTN
	else nvram_unset("qtn_restore_defaults");
#endif

#ifdef WEB_REDIRECT
	nvram_set("freeze_duck", "0");
#endif
	nvram_unset("ateCommand_flag");
	nvram_unset("ateUpgarde_flag");

#ifdef RTCONFIG_VPNC
	nvram_set_int("vpnc_state_t", 0);
	nvram_set_int("vpnc_sbstate_t", 0);
#endif

#ifdef RTCONFIG_VPN_FUSION
	reset_vpnc_state();
#endif

#if (defined(PLN12) || defined(PLAC56))
	nvram_set("plc_ready", "0");
#elif defined(PLAC66)
	nvram_set("plc_ready", "1");
#endif

#if defined(RTCONFIG_APP_PREINSTALLED) || defined(RTCONFIG_APP_NETINSTALLED)
#if defined(RTCONFIG_BCMARM)  && (!defined(RTCONFIG_NEW_APP_ARM))
	nvram_set("apps_ipkg_old", "0");
	nvram_set("apps_new_arm", "0");//2016.7.1 sherry new oleg arm
	nvram_set("apps_install_folder", "asusware.arm");
	nvram_set("apps_ipkg_server", "http://nw-dlcdnet.asus.com/asusware/arm/stable");
#elif defined(RTCONFIG_NEW_APP_ARM) //2016.7.1 sherry new oleg arm{
	nvram_set("apps_ipkg_old", "0");
	nvram_set("apps_new_arm", "1");
	nvram_set("apps_install_folder", "asusware.arm");
	nvram_set("apps_ipkg_server", "http://nw-dlcdnet.asus.com/asusware/arm_new/stable");
	//end sherry add}
#elif defined(RTCONFIG_QCA) || defined(RTCONFIG_LANTIQ)
	nvram_set("apps_ipkg_old", "0");
	nvram_set("apps_new_arm", "0"); //2016.7.1 sherry new oleg arm
	nvram_set("apps_install_folder", "asusware.mipsbig");
	nvram_set("apps_ipkg_server", "http://nw-dlcdnet.asus.com/asusware/mipsbig/stable");
#else
	if(!strcmp(get_productid(), "VSL-N66U") || !strcmp(get_productid(), "RP-AC68U")){
		nvram_set("apps_ipkg_old", "0");
		nvram_set("apps_install_folder", "asusware.mipsbig");
		nvram_set("apps_ipkg_server", "http://nw-dlcdnet.asus.com/asusware/mipsbig/stable");
	}
	else{ // mipsel
		nvram_set("apps_install_folder", "asusware");
		if (nvram_match("apps_ipkg_old", "1"))
#ifdef RTCONFIG_HTTPS
			nvram_set("apps_ipkg_server", "https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT");
#else
			nvram_set("apps_ipkg_server", "http://dlcdnet.asus.com/pub/ASUS/wireless/ASUSWRT");
#endif
		else
			nvram_set("apps_ipkg_server", "http://nw-dlcdnet.asus.com/asusware/mipsel/stable");
	}
#endif
#endif

#if defined(BRTAC828) || defined(RTAD7200)
	/* Reset lan_trunk_0 and lan_trunk_1 as zero to avoid trunking is configured to switch.
	 * Don't reset lan_trunk_type here due to user asks to support LAN LACP
	 * if LAN as WAN and none of any WAN port acts as WAN.
	 */
	if (strstr(nvram_safe_get("wans_dualwan"), "lan") != NULL) {
		nvram_set("lan_trunk_0", "0");
		nvram_set("lan_trunk_1", "0");
	}
#endif

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) && !defined(RTCONFIG_SOC_IPQ60XX)
	if (nvram_get("Ate_temp_state") != NULL)
		nvram_unset("Ate_temp_state");
#if !defined(RTCONFIG_GLOBAL_INI) && \
   (!defined(RTCONFIG_BTM_11V) || !defined(RTCONFIG_BCN_RPT))
	if (nvram_match("rssi_method", "1"))
		nvram_unset("rssi_method");
#endif
#endif

#ifdef RTCONFIG_TUNNEL
	nvram_set("aae_support", "1");
#define AAE_ENABLE_AIHOME 2
#define AAE_EANBLE_AICLOUD 4
	nvram_set("aae_enable", "0");
#ifdef RTCONFIG_AIHOME_TUNNEL
	nvram_set_int("aae_enable", (nvram_get_int("aae_enable") | AAE_ENABLE_AIHOME));
#endif
#ifdef RTCONFIG_AICLOUD_TUNNEL
	nvram_set_int("aae_enable", (nvram_get_int("aae_enable") | AAE_EANBLE_AICLOUD));
#endif
#endif

	nvram_unset("wps_reset");
#ifndef RTCONFIG_SW_CTRL_ALLLED
#if !(defined(RTCONFIG_LED_BTN) || defined(RTCONFIG_WPS_ALLLED_BTN) || defined(RTCONFIG_TURBO_BTN) || defined(RTCONFIG_SW_CTRL_ALLLED) || (!defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA)))
	nvram_set_int("AllLED", 1);
#endif
#endif
	nvram_unset("reload_svc_radio");

#ifdef RTCONFIG_DPSTA
	set_dpsta_ifnames();
#endif
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	set_dpsr_ifnames();
#endif

#ifdef RTCONFIG_AMAS
	nvram_unset("amesh_found_cap");
	nvram_unset("amesh_led");
	nvram_unset("amesh_wps_enr");
#ifdef CONFIG_BCMWL5
	nvram_unset("obd_allow_scan");
	nvram_unset("obd_scan_state");
	nvram_unset("acs_skip_init_acs");
	nvram_unset("wlcscan");
#endif
#ifdef RTCONFIG_ADV_RAST
	nvram_unset("diag_chk_cap");
	nvram_unset("diag_chk_re1");
	nvram_unset("diag_chk_re2");
	nvram_unset("diag_chk_re3");
	nvram_unset("diag_chk_re4");
	nvram_unset("diag_chk_re5");
	nvram_unset("diag_chk_re6");
	nvram_unset("diag_chk_re7");
	nvram_unset("diag_chk_re8");
#endif
#endif
	nvram_unset("wlc_scan_state");
#if defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2)
	nvram_unset("ledg_scheme_tmp");
	if (nvram_get_int("ledg_scheme") >= LEDG_SCHEME_BLINKING)
		nvram_set("ledg_scheme", nvram_default_get("ledg_scheme"));
#endif
#ifdef GT10
	nvram_unset("ledg_once");
#endif
#ifdef RTCONFIG_PIPEFW
	nvram_unset("ate_upgrade_reboot");
	nvram_unset("ate_upgrade_reset");
#endif

	nvram_unset("radar_detected");
	nvram_unset("radar_detected_if");
	nvram_unset("radar_detected_count");

	nvram_unset("stop_misc");
#if defined(HND_ROUTER) || defined(RTCONFIG_BCM_7114) || defined(RTCONFIG_BCM4708)
	nvram_unset("log_wlstat");
	nvram_unset("log_wlstat_starttime");
	if (strlen(nvram_safe_get("log_wlstat_dir")) &&
		d_exists(nvram_safe_get("log_wlstat_dir")) &&
		!strncmp(nvram_safe_get("log_wlstat_dir"), "/jffs", 5)) {
		char cmd[128];
		snprintf(cmd, sizeof(cmd), "rm -rf %s", nvram_safe_get("log_wlstat_dir"));
		system(cmd);
	}
#endif
	nvram_unset("wait_httpd");
	nvram_unset("wlc_mode");
#ifdef RTCONFIG_DUALWAN
	nvram_unset("upnp_unit");
	nvram_unset("upnp_ifname");
#endif
#if defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX5400)
	nvram_set_int("bcm_usb", 1);
#endif
#if defined(RTAX3000N) || defined(RPAX58)
	nvram_set("frs_need_pt", nvram_match("webs_prtbl", "1") ? "0" : "1");
#endif
}

#ifdef RTCONFIG_ISP_CUSTOMIZE
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2)
extern unsigned int get_root_type(void);
#endif
void package_defaults(int restore_defaults)
{
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2)
	unsigned int root_type = get_root_type();
#else
	unsigned int root_type = 0x73717368;
#endif

	if (restore_defaults) {
		struct statfs sf;
		int is_jffs2_mounted = 0;
		if ((statfs("/jffs", &sf) == 0) && (sf.f_type != root_type)) {
			is_jffs2_mounted = 1;
		}
		if (!is_jffs2_mounted) {
			start_jffs2();
		}

		package_restore_defaults();
	}

#ifdef RTCONFIG_ISP_CUSTOMIZE_TOOL
	if (nvram_get("ict_status")) {
		nvram_unset("ict_status");
		nvram_commit();
		setAllLedOn();
	}
#endif
}
#endif

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP)
static void pre_restore_defaults(void)
{
	int i, he_features;
	char prefix[sizeof("wlXXXXXX_")];

	for (i = WL_2G_BAND; i < MAX_NR_WL_IF && i <= WL_5G_2_BAND; ++i) {
		if (__absent_band(i))
			continue;

		snprintf(prefix, sizeof(prefix), "wl%d_", i);
#if defined(RTCONFIG_BW160M)
#if defined(RTCONFIG_SOC_IPQ50XX)
		if ((i == WL_5G_BAND || i == WL_5G_2_BAND)
		 && !nvram_pf_get(prefix, "bw_160")) /* first time */
			nvram_pf_set(prefix, "bw_160", "1");
#else
		if ((i == WL_5G_BAND || i == WL_5G_2_BAND)
		 && !nvram_pf_get(prefix, "bw_160")
		 && (nvram_pf_match(prefix, "bw", "1") || nvram_pf_get_int(prefix, "bw") == WL_BW_160))
			nvram_pf_set(prefix, "bw_160", "1");
#endif /* RTCONFIG_SOC_IPQ50XX */
#endif
		if (!nvram_pf_get(prefix, "he_features"))
			continue;
		he_features = nvram_pf_get_int(prefix, "he_features");
		if (!nvram_pf_get(prefix, "11ax"))
			nvram_pf_set_int(prefix, "11ax", !!(he_features & 3));
		if (!nvram_pf_get(prefix, "ofdma"))
			nvram_pf_set_int(prefix, "ofdma", !!(he_features & (3 << 2)));
	}
}
#endif

#if defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ8064)
static void post_restore_defaults(void)
{
	/* Use 768KB x 2 syslog instead of 256KB x 2. */
	if (nvram_get_int("log_size") < 768)
		nvram_set("log_size", "768");
}
#elif defined(RTCONFIG_SOC_IPQ60XX)
static void post_restore_defaults(void)
{
#if defined(PLAX56_XP4)
	if (nvram_match("HwId", "B")) {
		nvram_set_int("usb_enable", 0);
	}
#endif
}
#endif

/* ASUS use erase nvram to reset default only */
static int
restore_defaults(void)
{
	struct nvram_tuple *t;
	int restore_defaults;
	char prefix[] = "usb_pathXXXXXXXXXXXXXXXXX_", tmp[100];
	int unit;
#ifdef RTCONFIG_DHDAP
	int i;
#endif
#if 1	/* TODO: define RTCONFIG_XXX for this */
	int skiplan = nvram_match("ci", "1");
#endif
#ifdef LINUX26
	FILE *fp = NULL;
#endif
	nvram_unset(ASUS_STOP_COMMIT);
	nvram_unset(LED_CTRL_HIPRIO);

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP)
	pre_restore_defaults();
#endif
#ifdef HND_ROUTER
	hnd_check_data();
#endif

	// Restore defaults if nvram version mismatch
	restore_defaults = RESTORE_DEFAULTS();

	/* Restore defaults if told to or OS has changed */
	if (!restore_defaults)
	{
		restore_defaults = !nvram_match("restore_defaults", "0");
#if defined(RTN56U)
		/* upgrade from firmware 1.x.x.x */
		if (nvram_get("HT_AutoBA") != NULL)
		{
			nvram_unset("HT_AutoBA");
			restore_defaults = 1;
		}
#endif
#if defined(RTCONFIG_REALTEK)
#if defined(RPAC55) || defined(RPAC53)
		char c;
		f_write_string("/proc/asus_ate", "restore 1", 0, 0);
		f_read("/proc/asus_ate", &c, 1);
		int restore_flag = c - '0';
#ifdef RPAC53
		if (restore_flag != 1)
			restore_flag = rtk_check_nvram_partation(); // Restore defaults if nvram value is garbled.
#endif
		if (restore_flag == 1) {
			_dprintf("Erase NVRAM...\n");
			system("mtd-erase -d nvram");
			sleep(1);
			reboot(RB_AUTOBOOT);
		}
#endif
#endif
	}

#ifdef RTCONFIG_LANTIQ
	if (nvram_get_int("Ate_power_on_off_enable") != 0) {
		restore_defaults = 0;
	}
#endif

	if (restore_defaults){
		fprintf(stderr, "\n## Restoring defaults... ##\n");
#if defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2)
		if(GPY211_ext_phy_model() == 1){
			sleep(20); /* delay 20 secs to make gpy211 / brcm sgmii ready */
		}
#endif
	}

	restore_defaults_g = restore_defaults;

	if (restore_defaults) {
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
		unlink(NAND_FILE);
#endif
#ifdef RTCONFIG_WPS
		wps_restore_defaults();
#endif
		virtual_radio_restore_defaults();
	}
#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
	else
		confmtd_restore();
#endif

#ifdef RTCONFIG_USB
#ifndef RTCONFIG_PERMISSION_MANAGEMENT
	usbctrl_default();
#endif
#endif

	/* Restore defaults */
	for (t = router_defaults; t->name; t++) {
#if 1	/* TODO: define RTCONFIG_XXX for this */
		/* Skip change setting when run auto test */
		if (skiplan) {
			if (strcmp(t->name, "lan_ipaddr") == 0)
				continue;
		}
#endif
#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_MSSID_PRELINK)
		if (nvram_get(t->name) &&
			(strcmp(t->name, "plk_cap_subunit") == 0 || strcmp(t->name, "plk_re_subunit") == 0))
			continue;
#endif
#if defined(RTCONFIG_ASUSCTRL)
		if ((!strcmp(t->name, "asusctrl_flags")
		  || !strcmp(t->name, "asusctrl_chg_sku")
		  || !strcmp(t->name, "territory_code"))
		 && nvram_get(t->name))
			continue;
#endif
		if (restore_defaults || !nvram_get(t->name))
			nvram_set(t->name, t->value);
	}
#ifdef RTCONFIG_TCODE
	/* restore tcode-based defaults */
	config_tcode(1);
#endif

#ifdef RTAX86U
	if(restore_defaults){
		if(!strcmp(get_productid(), "RT-AX86S")){
			nvram_set("bsd_ifnames", "eth5 eth6");
			nvram_set("wl0_bsd_if_select_policy", "eth6");
			nvram_set("wl1_bsd_if_select_policy", "eth5");
		}
	}
#endif

#if LINUX26
	//Expand ct_expect_max if ram size is bigger than 512MB and device is in default state.
	if(restore_defaults)
	{
		fp = fopen("/proc/meminfo", "r");
		if(fp)
		{
			char str[64], buf[512];
			int val;
			while(fgets(buf, sizeof(buf), fp))
			{
				if(sscanf(buf, "%s %d", str, &val) == 2)
				{
					if(!strncmp(str, "MemTotal:", 9))
					{
						if(val > 512*1024)
						{
							nvram_set("ct_expect_max", "4096");
						}
						break;
					}
				}
			}
			fclose(fp);
		}
	}
#endif

#if defined(RTCONFIG_QCA) && defined(RTCONFIG_MSSID_PRELINK)
	if (!restore_defaults) 
	       pre_syspara_prelink();
#endif


	wl_defaults();
#if defined(RTCONFIG_MSSID_PRELINK)
	mssid_prelink_defaults();
#endif  /* RTCONFIG_MSSID_PRELINK */
	lan_defaults(restore_defaults);

	wan_defaults();

#ifdef RTAC3200
	bsd_defaults();
#endif
#ifdef  __CONFIG_WBD__
	wbd_restore_defaults();
#endif /* __CONFIG_WBD__ */
#ifdef WLHOSTFBT
	fbt_restore_defaults();
#endif /* WLHOSTFBT */
#ifdef RTCONFIG_DHDAP
	for (i = 0; i < MAX_NVPARSE; i++) {
		snprintf(tmp, sizeof(tmp), "wl%d_cfg_maxassoc", i);
		nvram_unset(tmp);
	}
#endif
#if defined(RTCONFIG_MSSID_PRELINK)
	if (nvram_get_int("re_mode") == 1 && nvram_get_int("prelink") == 1)
		set_mssid_prelink_config();
#endif	/* RTCONFIG_MSSID_PRELINK */

	if (restore_defaults) {
		uint memsize = 0;
		char pktq_thresh[8] = {0};
		char et_pktq_thresh[8] = {0};

		memsize = get_meminfo_item("MemTotal");

		if (memsize <= 32768) {
			/* Set to 512 as long as onboard memory <= 32M */
			sprintf(pktq_thresh, "512");
			sprintf(et_pktq_thresh, "512");
		}
		else if (memsize <= 65536) {
			/* We still have to set the thresh to prevent oom killer */
			sprintf(pktq_thresh, "1024");
			sprintf(et_pktq_thresh, "1536");
		}
		else {
			sprintf(pktq_thresh, "1024");
			/* Adjust the et thresh to 3300 as long as memory > 64M.
			 * The thresh value is based on benchmark test.
			 */
			sprintf(et_pktq_thresh, "3300");
		}

#ifdef CONFIG_BCMWL5
		if (memsize > 200 * 1024)
			nvram_set_int("log_size", 512);
#endif

		nvram_set("wl_txq_thresh", pktq_thresh);
		nvram_set("et_txq_thresh", et_pktq_thresh);
#if defined(__CONFIG_USBAP__)
		nvram_set("wl_rpcq_rxthresh", pktq_thresh);
#endif
#ifdef RTCONFIG_TCODE
		restore_defaults_wifi(1);
#endif

#ifdef RTCONFIG_RP_NEWSSID_REV3
		restore_rp_defaults_wifi(1);
#endif
		char *lan_ipaddr = nvram_get("IpAddr_Lan");

		if(lan_ipaddr != NULL && strcmp(lan_ipaddr, "NONE")){
			nvram_set("lan_ipaddr", lan_ipaddr);
		}

#ifdef RTCONFIG_TAGGED_BASED_VLAN
		tagged_vlan_defaults();
#endif

#ifdef RTCONFIG_DSL
		dsl_defaults();
#endif
		nvram_set("sys_reboot_reason", "restore_default");
	}



	/* Commit values */
	if (restore_defaults) {
		nvram_commit();
		fprintf(stderr, "done\n");
	}

	if (!nvram_match("firmver_org", nvram_safe_get("firmver")) ||
	    !nvram_match("buildno_org", nvram_safe_get("buildno")) ||
	    !nvram_match("extendno_org", nvram_safe_get("extendno"))){
		nvram_commit();
	}
	else{
		nvram_set("fwpath", "");
	}

	convert_defaults();

	/* default for state control variables */
	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		for (t = router_state_defaults; t->name; t++) {
			if (!strncmp(t->name, "wan_", 4))
				nvram_set(strcat_r(prefix, &t->name[4], tmp), t->value);
			else if (unit == WAN_UNIT_FIRST) // let non-wan nvram to be set one time.
				nvram_set(t->name, t->value);
		}
	}

	misc_defaults(restore_defaults);

#ifdef RTCONFIG_BCM_MFG
	brcm_mfg_init();
#endif

#ifdef RTCONFIG_ISP_CUSTOMIZE
	package_defaults(restore_defaults);
#endif

#if defined(RTCONFIG_HTTPS)
	nvram_unset("httpds_reload_cert");
	nvram_unset("httpds6_reload_cert");
#endif

#if defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ60XX)
	/* Keep below statment at end of restore_defaults(). */
	post_restore_defaults();
#endif
	return restore_defaults;
}

/* Set terminal settings to reasonable defaults */
static void set_term(int fd)
{
	struct termios tty;

	tcgetattr(fd, &tty);

	/* set control chars */
	tty.c_cc[VINTR]  = 3;	/* C-c */
	tty.c_cc[VQUIT]  = 28;	/* C-\ */
	tty.c_cc[VERASE] = 8; /* C-H */
	tty.c_cc[VKILL]  = 21;	/* C-u */
	tty.c_cc[VEOF]   = 4;	/* C-d */
	tty.c_cc[VSTART] = 17;	/* C-q */
	tty.c_cc[VSTOP]  = 19;	/* C-s */
	tty.c_cc[VSUSP]  = 26;	/* C-z */

	/* use line dicipline 0 */
	tty.c_line = 0;

	/* Make it be sane */
	tty.c_cflag &= CBAUD|CBAUDEX|CSIZE|CSTOPB|PARENB|PARODD;
	tty.c_cflag |= CREAD|HUPCL|CLOCAL;


	/* input modes */
	tty.c_iflag = ICRNL | IXON | IXOFF;

	/* output modes */
	tty.c_oflag = OPOST | ONLCR;

	/* local modes */
	tty.c_lflag =
		ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;

	tcsetattr(fd, TCSANOW, &tty);
}

int console_fd = -1;

int close_consolefd()
{
	if(console_fd >= 0) {
		printf("close console_fd:%d\n", console_fd);
		close(console_fd);
	}
	return 0;
}

static int console_init(void)
{
	int fd;
	struct winsize win = { 0 };

	if(nvram_match("noconsole", "1")) {
		printf("!! no use console(init)\n");
		return 1;
	}

	/* Clean up */
	ioctl(0, TIOCNOTTY, 0);
	close(0);
	close(1);
	close(2);
	setsid();

	/* Reopen console */
	if ((fd = open(_PATH_CONSOLE, O_RDWR)) < 0) {
		/* Avoid debug messages is redirected to socket packet if no exist a UART chip, added by honor, 2003-12-04 */
		open("/dev/null", O_RDONLY);
		open("/dev/null", O_WRONLY);
		open("/dev/null", O_WRONLY);
		perror(_PATH_CONSOLE);
		return errno;
	}
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);

	ioctl(0, TIOCSCTTY, 1);
	tcsetpgrp(0, getpgrp());
	set_term(0);

	console_fd = fd;
	printf("console fd is %d\n", console_fd);

	if (!ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		win.ws_row = nvram_get("console_height")? nvram_get_int("console_height") : 50;
		win.ws_col = nvram_get("console_width")? nvram_get_int("console_width") : 160;
		if (ioctl(STDIN_FILENO, TIOCSWINSZ, (char *) &win)) {
			_dprintf("Set console size as %dx%d failed, errno %d (%s)\n",
				win.ws_row, win.ws_col, errno, strerror(errno));
		}
	}

	return 0;
}

static pid_t run_shell(int timeout, int nowait)
{
	char user_env[sizeof("USER=XXX") + 32];
	char *argv_shell[] = { SHELL, NULL };
	char *argv_login[] = { LOGIN, "-p", NULL };
	char **argv = argv_login;
	pid_t pid;
	int sig;

	if(nvram_match("noconsole", "1")) {
		printf("!! not run shell, no use console\n");
		return 0;
	}
	/* Wait for user input */
	if (waitfor(STDIN_FILENO, timeout) <= 0)
		return 0;

	snprintf(user_env, sizeof(user_env), "USER=%s", nvram_get("http_username")? : "admin");
	defenv[0] = user_env;
#if defined(BC109) || defined(EBG19) || defined(EBG15) || defined(EBP15)
        if (!nvram_match("shell_login", "1")) {
                printf("run SHELL as pre-test\n");
                argv = argv_shell;
        } else if (!check_if_file_exist("/etc/shadow"))
                setup_passwd();
#else
//	if (ate_factory_mode())
//	        argv = argv_shell;
//	else 
	if (!check_if_file_exist("/etc/shadow"))
		setup_passwd();
#endif

	switch (pid = fork()) {
	case -1:
		perror("fork");
		return 0;
	case 0:
		/* Reset signal handlers set for parent process */
		for (sig = 0; sig < (_NSIG-1); sig++)
			signal(sig, SIG_DFL);

		/* Reopen console */
		console_init();

		/* Now run it.  The new program will take over this PID,
		 * so nothing further in init.c should be run. */
		execve(argv[0], argv, defenv);

		/* We're still here?  Some error happened. */
		perror(argv[0]);
		_exit(errno);
	default:
		if (!nowait) {
			waitpid(pid, NULL, 0);
			pid = 0;
		}
		return pid;
	}
}

void console_hup(int sig)
{
	if (sig != SIGHUP)
		return;

	close_consolefd();
	console_init(); /* Reopen console */
}

int console_main(int argc, char *argv[])
{
	/* Reopen console */
	console_init();

	signal(SIGHUP, console_hup);
	for (; nvram_match("noconsole", "0");)
		run_shell(0, 0);

	return 0;
}

static void shutdn(int rb)
{
	int i;
	int act;
	sigset_t ss;
#ifdef RTCONFIG_RGBLED
	RGB_LED_STATUS_T rgb_cfg = { 0 };
#endif

	_dprintf("shutdn rb=%d\n", rb);

	sigemptyset(&ss);
	for (i = 0; i < sizeof(fatalsigs) / sizeof(fatalsigs[0]); i++)
		sigaddset(&ss, fatalsigs[i]);
	for (i = 0; i < sizeof(initsigs) / sizeof(initsigs[0]); i++)
		sigaddset(&ss, initsigs[i]);
	sigprocmask(SIG_BLOCK, &ss, NULL);

	for (i = 30; i > 0; --i) {
		if (((act = check_action()) == ACT_IDLE) ||
		     (act == ACT_REBOOT) || (act == ACT_UNKNOWN)) break;
		_dprintf("Busy with %d. Waiting before shutdown... %d\n", act, i);
		sleep(1);
	}
	set_action(ACT_REBOOT);

	stop_wan();
#if defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
	rtkswitch_LanPort_linkDown();
	eval("rtkswitch", "45");
#endif

#ifdef RTCONFIG_RGBLED
	__nv_to_rgb("0,0,0,5,2,0", &rgb_cfg);
	aura_rgb_led(ROUTER_AURA_SET, &rgb_cfg, 0, 0);
#endif

	_dprintf("TERM\n");
	kill(-1, SIGTERM);
	sleep(3);
	sync();

	_dprintf("KILL\n");
	kill(-1, SIGKILL);
	sleep(1);
	sync();

	// TODO LED Status for LED
#if	!(defined(RTCONFIG_RGBLED) && defined(HND_ROUTER))
	setAllLedOff();
#endif

	sync(); sync(); sync();
#ifdef HND_ROUTER
#if defined(BCM6855) || defined(GTAXE16000)
	config_jumbo_frame();
#endif
#ifdef RTCONFIG_HND_ROUTER_AX_6756
	if (f_exists("/jffs/remove_hidden_flag"))
		mtd_erase_misc2();
#endif
	printf("\nStopping bcm_boot_launcher ...\n");
	system("bcm_boot_launcher stop");
#endif
	reboot(rb ? RB_AUTOBOOT : RB_HALT_SYSTEM);

	do {
		sleep(1);
	} while (1);
}

static void handle_fatalsigs(int sig)
{
	char *message = NULL;

	switch (sig) {
		case SIGILL: message = "Illegal instruction"; break;
		case SIGABRT: message = "Abort"; break;
		case SIGFPE: message = "Floating exception"; break;
		case SIGPIPE: message = "Broken pipe"; break;
		case SIGBUS: message = "Bus error"; break;
		case SIGSYS: message = "Bad system call"; break;
		case SIGTRAP: message = "Trace trap"; break;
		case SIGPWR: message = "Power failure"; break;
	}

	if (message)
		dbg("%s\n", message);
	else
		dbg("Caught fatal signal %d\n", sig);

	shutdn(-1);
}

/* Fixed the race condition & incorrect code by using sigwait()
 * instead of pause(). But SIGCHLD is a problem, since other
 * code: 1) messes with it and 2) depends on CHLD being caught so
 * that the pid gets immediately reaped instead of left a zombie.
 * Pidof still shows the pid, even though it's in zombie state.
 * So this SIGCHLD handler reaps and then signals the mainline by
 * raising ALRM.
 */
static void handle_reap(int sig)
{
	chld_reap(sig);
	raise(SIGALRM);
}


#ifdef RTCONFIG_SWMODE_SWITCH
/*
 * * Check
 * * 1.Phy Switch status.
 * * 2.if default, set the lan proto.
 * * */
void init_swmode()
{
#if defined(PLAC66U)
	if (!button_pressed(BTN_SWMODE_SW_ROUTER)) {
		nvram_set_int("sw_mode", SW_MODE_ROUTER);
		if (nvram_match("x_Setting", "0"))
			nvram_set("lan_proto", "dhcp");
	}
	else {
		nvram_set_int("sw_mode", SW_MODE_AP);
		if (nvram_match("x_Setting", "0"))
			nvram_set("lan_proto", "dhcp");
	}

	nvram_set_int("swmode_switch", button_pressed(BTN_SWMODE_SW_ROUTER));
#else
	if (!nvram_get_int("swmode_switch")) return;

	if (button_pressed(BTN_SWMODE_SW_REPEATER)) {
		nvram_set_int("sw_mode", SW_MODE_REPEATER);
		dbg("%s: swmode: repeater", LOGNAME);
		if (nvram_match("x_Setting", "0")) {
			nvram_set("lan_proto", "dhcp");
		}
	}else if (button_pressed(BTN_SWMODE_SW_AP)) {
		nvram_set_int("sw_mode", SW_MODE_AP);
		dbg("%s: swmode: ap", LOGNAME);
		if (nvram_match("x_Setting", "0")) {
			nvram_set("lan_proto", "dhcp");
		}
	}else if (button_pressed(BTN_SWMODE_SW_ROUTER)) {
		dbg("%s: swmode: router", LOGNAME);
		nvram_set_int("sw_mode", SW_MODE_ROUTER);
		nvram_set("lan_proto", "static");
	}else{
		dbg("%s: swmode: unknow swmode", LOGNAME);
	}
#endif	/* Model */
}
#endif	/* RTCONFIG_SWMODE_SWITCH */

#if 0
void conf_swmode_support(int model)
{
	switch (model) {
		case MODEL_RTAC66U:
		case MODEL_RTAC53U:
			nvram_set("swmode_support", "router repeater ap psta");
			dbg("%s: swmode: router repeater ap psta\n", LOGNAME);
			break;
		case MODEL_APN12HP:
			nvram_set("swmode_support", "repeater ap");
			dbg("%s: swmode: repeater ap\n", LOGNAME);
			break;
		default:
			nvram_set("swmode_support", "router repeater ap");
			dbg("%s: swmode: router repeater ap\n", LOGNAME);
			break;
	}
}
#endif
char *the_wan_phy()
{
#ifdef RTCONFIG_BCMFA
	if (FA_ON(fa_mode))
		return "vlan2";
	else
#endif
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(GT10) || defined(RTAX86U_PRO)
	if (nvram_get_int("wans_extwan"))
#if defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(GT10)
		return "eth1";
#elif defined(RTAX86U_PRO)
		return "eth5";
#endif
	else
#endif
		return WAN_IF_ETH;
}

enum wan_iface_id {
	WAN_IFACE_ID = 0,
	WAN2_IFACE_ID,
	SFPP_IFACE_ID,

	MAX_WAN_IFACE_ID
};

#if defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || \
    (defined(RTCONFIG_RALINK) && !defined(RTN56U))
#if !defined(RTCONFIG_RALINK) || defined(RTCONFIG_RALINK_MT7620) || defined(RTCONFIG_RALINK_MT7621) || defined(RTCONFIG_RALINK_MT7628)
/**
 * Return true if WAN and VoIP/STB use same VLAN ID.
 * For software based IPTV implementation, if WAN doesn't use same VLAN with
 * VoIP/STB, it's okay to use vlanX as WAN interface.  If WAN use same VLAN
 * with VoIP/STB, use bridge to implement Hinet MOD profile, or use bridge to
 * implement pure STB port, we have to bridge WAN and VoIP/STB together.  In
 * this case, WAN interface must be bridge interface, nor vlan interface.
 * @return:
 * 	0:	WAN didn't use same VLAN ID with VoIP/STB.
 *  otherwise:	WAN use same VLAN ID with VoIP/STB.
 */
static int wan_iptv_same_vid(void)
{
	char *p;
	int wan_tagid = -1, lan4_tagid = -1, lan3_tagid = -1;

	if (nvram_match("switch_wantag", "meo") || nvram_match("switch_wantag", "vodafone"))
		return 1;

	if (nvram_match("switch_wantag", "manual")) {
		if ((p = nvram_get("switch_wan0tagid")) != NULL)
			wan_tagid = safe_atoi(p);
		if ((p = nvram_get("switch_wan1tagid")) != NULL)
			lan4_tagid = safe_atoi(p);
		if ((p = nvram_get("switch_wan2tagid")) != NULL)
			lan3_tagid = safe_atoi(p);

		if (!legal_vlanid(wan_tagid))
			return 0;

		if (wan_tagid == lan4_tagid || wan_tagid == lan3_tagid)
			return 1;
	}

	return 0;
}
#endif
#endif

/* If @nv nvram variable exist, append @sep and @val to it. */
void add_nv_val(const char *nv, const char sep, char *val)
{
	char *orig, *buf;

	if (!nv || !sep || !(val && *val))
		return;

	orig = nvram_safe_get(nv);
	if (*orig) {
		if (asprintf(&buf, "%s%c%s", orig, sep, val) < 0 || !buf) {
			dbg("%s(%s,%c,%s) fail\n", __func__, nv, sep, val);
			return;
		}
		nvram_set(nv, buf);
		free(buf);
	} else
		nvram_set(nv, val);
}

/**
 * Generic wan_ifnames / lan_ifnames / wl_ifnames / wl[01]_vifnames initialize helper
 * @wan_ifaces:	WAN interfaces.
 * 		@wan_ifaces[0] is old @wan parameter of this function.
 * 		@wan_ifaces[1] is old @wan2 parameter of this function.
 * 		@wan_ifaces[2] is SFP+ interface name.
 * @lan:	LAN interface.
 * @wl_ifaces:	Wireless interfaces of each band, including 2.4G, 5G, 5G-2, and 60G.
 * @usb:	USB interface, can be NULL.
 * @ap_lan:	LAN interface in AP mode.
 * 		If model-specific code don't need to bridge WAN and LAN interface in AP mode,
 * 		customize interface name of LAN interface in this parameter.
 * 		If @ap_lan is NULL or empty string, all WAN interfaces are bridged to lan_ifname in AP/RP/MB mode.
 * 		e.g. RT-N65U:
 * 		Home gateway mode:	WAN = vlan2, LAN = vlan1.
 * 		AP mode:		WAN = N/A,   LAN = vlan1, switch is reconfigured by the "rtkswitch 8 100" command.
 * @dw_wan:	WAN interface in dual-wan mode.
 * 		e.g. RT-AC68U:
 * 		Default WAN interface of RT-AC68U is eth0.
 * 		If one of LAN ports is configured as a WAN port and WAN port is enabled,
 * 		WAN interface becomes "vlanX" (X = nvram_get("switch_wan0tagid")) or "vlan2".
 * @dw_lan:	LAN interface of LAN port that is used as a WAN interface.
 * 		In most cases, it is supposed a VLAN interface, e.g. "vlan3".
 * 		This interface is used as WAN interface (wanX_ifname) if and only if any below conditions true:
 * 		1) @force_dwlan is true.
 * 		2) WAN port is enabled too.
 * 		3) IPTV is enabled, Ralink/MTK platform only.
 * 		   Because only Ralink/MTK platform always use VLAN ID 2 as WAN and use VLAN tag swapping or similar trick
 * 		   to implement IPTV, whether IPTV is enabled or not.
 * 		So, if you really need to choose @dw_lan as WAN interface as long as LAN as WAN is enabled,
 * 		pass non-zero value to @force_dwlan.
 *
 * 		WAN/LAN, LAN/WAN:
 * 			WAN:
 * 				(1) vlanX (X = nvram_get("switch_wan0tagid")), or
 * 				(2) vlan2 (most Realtek switch based model use this)
 * 				depends IPTV related implementation.
 * 			LAN:
 * 				vlan3
 *
 * 		char *wl_ifaces[WL_NR_BANDS];
 * 		memset(wl_ifaces, 0, sizeof(wl_ifaces));
 * 		wl_ifaces[WL_2G_BAND] = "eth1";
 * 		wl_ifaces[WL_5G_BAND] = "eth2";
 * 		wan_ifaces[WAN_IFACE_ID] = "wan0";
 * 		wan_ifaces[WAN2_IFACE_ID] = "wan1";
 * 		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", NULL, "vlan2", "vlan3", 0);
 * @force_dwlan:If true, always choose dw_lan as LAN interface that is used as WAN, even WAN/LAN are enabled both.
 *
 * @return:
 * 	-1:	invalid parameter.
 *	0:	success
 */
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK) || \
    (defined(RTCONFIG_RALINK) && !defined(RTN56U))
static int set_basic_ifname_vars(char *wan_ifaces[MAX_WAN_IFACE_ID], char *lan, char *wl_ifaces[WL_NR_BANDS], char *usb, char *ap_lan, char *dw_wan, char *dw_lan, int force_dwlan)
{
	const int sw_mode = sw_mode(), stb_x = nvram_get_int("switch_stb_x");
	const int switch_wan0tagid = nvram_get("switch_wan0tagid")? nvram_get_int("switch_wan0tagid") : -1;
	char *wl2g, *wl5g, *wl5g2, *wl60g, *p;
	int w, upstream_unit;
	char buf[128], prefix[sizeof("wanXXXXXX_")], *wan = wan_ifaces[WAN_IFACE_ID];
	char *wan2 = wan_ifaces[WAN2_IFACE_ID], *wan2_orig = wan_ifaces[WAN2_IFACE_ID];
#if defined(RTCONFIG_DUALWAN)
	int unit, type;
	int enable_dw_wan = 0;
#endif
	int wans_dualwan = get_wans_dualwan();

	if (!wan || !lan || *lan == '\0' || !wl_ifaces ||
	    !wl_ifaces[WL_2G_BAND] || *wl_ifaces[WL_2G_BAND] == '\0') {
		dbg("%s: invalid parameter. (wan %s, lan %s, "
		    "wl_ifaces %p [%s,%s,%s,%s], ap_lan %s, sw_mode %d)\n",
			__func__, wan? wan : "NULL", lan? lan : "NULL", wl_ifaces,
			(wl_ifaces && wl_ifaces[WL_2G_BAND])? wl_ifaces[WL_2G_BAND] : "NULL",
			(wl_ifaces && wl_ifaces[WL_5G_BAND])? wl_ifaces[WL_5G_BAND] : "NULL",
			(wl_ifaces && wl_ifaces[WL_5G_2_BAND])? wl_ifaces[WL_5G_2_BAND] : "NULL",
			(wl_ifaces && wl_ifaces[WL_60G_BAND])? wl_ifaces[WL_60G_BAND] : "NULL",
			ap_lan? ap_lan : "NULL", sw_mode);
		return -1;
	}
	if (usb && *usb == '\0')
		usb = "usb";
	if (ap_lan && *ap_lan == '\0')
		ap_lan = NULL;

	wl2g  = wl_ifaces[WL_2G_BAND];
	wl5g  = wl_ifaces[WL_5G_BAND];
	wl5g2 = wl_ifaces[WL_5G_2_BAND];
	wl60g = wl_ifaces[WL_60G_BAND];

	nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
	/* wl[01]_vifnames are virtual id only for ui control */
#if defined(RTCONFIG_HAS_5G)
	if (wl5g) {
		snprintf(buf, sizeof(buf), "%s %s", wl2g, wl5g);
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
	} else {
		strlcpy(buf, wl2g, sizeof(buf));
		nvram_set("wl1_vifnames", "");
	}
#else
	strlcpy(buf, wl2g, sizeof(buf));
	nvram_set("wl1_vifnames", "");
#endif
#if defined(RTCONFIG_HAS_5G_2)
	if (wl5g2) {
		strlcat(buf, " ", sizeof(buf));
		strlcat(buf, wl5g2, sizeof(buf));
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
	} else {
		nvram_set("wl2_vifnames", "");
	}
#else
	nvram_set("wl2_vifnames", "");
#endif
#if defined(RTCONFIG_WIGIG)
	if (wl60g) {
#if !defined(RTCONFIG_HAS_5G_2)
		/* To keep 802.11ad Wigig at wl3 band, if 2-nd 5G is not supported,
		 * we have to pad a fake 5G-2 interface to wl_ifnames.
		 */
		if (wl5g2) {
			strlcat(buf, " ", sizeof(buf));
			strlcat(buf, wl5g2, sizeof(buf));
		}
#endif
		strlcat(buf, " ", sizeof(buf));
		strlcat(buf, wl60g, sizeof(buf));
		nvram_set("wl3_vifnames", "wl3.1 wl3.2 wl3.3");
	} else {
		nvram_set("wl3_vifnames", "");
	}
#else
	nvram_set("wl3_vifnames", "");
#endif
	nvram_set("wl_ifnames", buf);

	switch (sw_mode) {
	case SW_MODE_AP:
		if (!ap_lan) {
			set_lan_phy(lan);
			/* Added the rest of WAN interfaces to LAN. */
			for (w = WAN_IFACE_ID; w < MAX_WAN_IFACE_ID; ++w) {
				if (!wan_ifaces[w])
					continue;

				add_lan_phy(wan_ifaces[w]);
			}
		} else {
			set_lan_phy(ap_lan);
		}
		set_wan_phy("");

		/* Added Wireless interfaces to LAN. */
		add_lan_phy(wl2g);
		if (wl5g)
			add_lan_phy(wl5g);
		if (wl5g2)
			add_lan_phy(wl5g2);
		if (wl60g)
			add_lan_phy(wl60g);
		break;

#if defined(RTCONFIG_WIRELESSREPEATER)
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	case SW_MODE_REPEATER:
#if defined(RTCONFIG_PROXYSTA)
		if (nvram_get_int("wlc_psta") == 1)
		{
			/* media bridge mode */
			if (!ap_lan) {
				set_lan_phy(lan);
				/* Added the rest of WAN interfaces to LAN. */
				for (w = WAN_IFACE_ID; w < MAX_WAN_IFACE_ID; ++w) {
					if (wan_ifaces[w]) {
						add_lan_phy(wan_ifaces[w]);
					}
				}
			} else {
				set_lan_phy(ap_lan);
			}
			set_wan_phy("");

			/* Added Wireless interfaces to LAN. */
			add_lan_phy(wl2g);
			if (wl5g)
				add_lan_phy(wl5g);
			if (wl5g2)
				add_lan_phy(wl5g2);
			if (wl60g)
				add_lan_phy(wl60g);
		}
		else
#endif
		{
			/* repeater mode */
			if (!ap_lan) {
				set_lan_phy(lan);
				/* Added the rest of WAN interfaces to LAN. */
				for (w = WAN_IFACE_ID; w < MAX_WAN_IFACE_ID; ++w) {
					if (wan_ifaces[w]) {
						add_lan_phy(wan_ifaces[w]);
					}
				}
			} else {
				set_lan_phy(ap_lan);
			}
			set_wan_phy("");

			/* Added Wireless interfaces to LAN. */
			add_lan_phy(wl2g);
			if (wl5g)
				add_lan_phy(wl5g);
			if (wl5g2)
				add_lan_phy(wl5g2);
			if (wl60g)
				add_lan_phy(wl60g);
		}
		break;
#elif defined(RTCONFIG_QTN)
		/* FIXME */
#else
		/* Broadcom platform */
		/* FIXME */
#endif	/* RTCONFIG_RALINK || RTCONFIG_QCA */
#endif	/* RTCONFIG_WIRELESSREPEATER */

	default:
		/* router/default */
		set_lan_phy(lan);
#if defined(RTCONFIG_DUALWAN)
		if (!dw_wan || *dw_wan == '\0')
			dw_wan = wan;
		if (!dw_lan || *dw_lan == '\0')
			dw_lan = lan;

		if (!(wans_dualwan & WANSCAP_2G))
			add_lan_phy(wl2g);
		if (wl5g && !(wans_dualwan & WANSCAP_5G))
			add_lan_phy(wl5g);
		if (wl5g2)
			add_lan_phy(wl5g2);
		if (wl60g)
			add_lan_phy(wl60g);

		upstream_unit = get_upstream_wan_unit();
		nvram_set("switch_upstream", "none");
		set_wan_phy("");
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			char *wphy = NULL;

			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			type = get_dualwan_by_unit(unit);
			switch (type) {
			case WANS_DUALWAN_IF_LAN:
				if (force_dwlan || (wans_dualwan & WANSCAP_WAN)
#if defined(RTCONFIG_RALINK)
				    || (!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", ""))
#endif
				   )
				{
					wphy = dw_lan;
				} else
					wphy = wan;	/* NOTICE */
				break;
			case WANS_DUALWAN_IF_2G:
				wphy = wl2g;
				break;
			case WANS_DUALWAN_IF_5G:
				if (wl5g)
					wphy = wl5g;
				break;
			case WANS_DUALWAN_IF_WAN:
				if (wans_dualwan & WANSCAP_LAN) {
					wphy = dw_wan;
					enable_dw_wan = 1;
				} else
					wphy = wan;
				break;
			case WANS_DUALWAN_IF_WAN2:
				wphy = wan2;
				wan2_orig = NULL;
				break;
			case WANS_DUALWAN_IF_SFPP:
				wphy = wan_ifaces[SFPP_IFACE_ID];
				break;
			case WANS_DUALWAN_IF_USB:
				wphy = usb;
				break;
			case WANS_DUALWAN_IF_NONE:
				break;
			default:
				_dprintf("%s: Unknown DUALWAN type %d\n", __func__, type);
			}

			/* Broadcom, QCA, MTK (use MT7620/MT7621 ESW) platform.
			 * Set vlanXXX to wanX_ifname for IPTV feature.
			 */
			if (unit == upstream_unit && nvram_get("switch_wantag")) {
				switch (type) {
				case WANS_DUALWAN_IF_LAN:
#if defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2) || defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
					/* For Realtek switch platforms.
					 * Because our IPTV code always untag frames at CPU port,
					 * unless platform-specific switch configuration code ignore CPU port from untag port mask.
					 * See config_switch() how to use __setup_vlan().
					 */
					if (nvram_get("switch_wantag") && nvram_match("switch_wantag", "movistar")) {
						snprintf(buf, sizeof(buf), "vlan%d", switch_wan0tagid);
						wphy = buf;
					}
#endif
					break;
				case WANS_DUALWAN_IF_WAN:	/* fall-through */
				case WANS_DUALWAN_IF_WAN2:	/* fall-through */
				case WANS_DUALWAN_IF_SFPP:	/* fall-through */
#if !defined(RTCONFIG_RALINK) || defined(RTCONFIG_RALINK_MT7620) || defined(RTCONFIG_RALINK_MT7621) || defined(RTCONFIG_RALINK_MT7628)
					if (nvram_match("switch_wantag", "") ||
					    (!switch_wan0tagid && !nvram_match("switch_wantag", "hinet") && !nvram_match("switch_wantag", "none")) ||
					    (nvram_match("switch_wantag", "none") && stb_x <= 0))
						break;

					if (sw_based_iptv()) {
						if ((nvram_match("switch_wantag", "none") && stb_x > 0) ||
						    nvram_match("switch_wantag", "hinet")) {
							snprintf(buf, sizeof(buf), "brv%d", get_sw_bridge_iptv_vid());
							wphy = buf;
						} else if (!nvram_match("switch_wantag", "none") && switch_wan0tagid >= 0 && switch_wan0tagid < 4096) {
							if (wan_iptv_same_vid())
								snprintf(buf, sizeof(buf), "brv%d", switch_wan0tagid);
							else
								snprintf(buf, sizeof(buf), "vlan%d", switch_wan0tagid);
							wphy = buf;
						}
					} else {
						if (!nvram_match("switch_wantag", "hinet") && !nvram_match("switch_wantag", "none")) {
							snprintf(buf, sizeof(buf), "vlan%d", switch_wan0tagid);
							wphy = buf;
						}
					}
#endif	/* !RTCONFIG_RALINK || RTCONFIG_RALINK_MT7620 || RTCONFIG_RALINK_MT7621 */
					break;
				}
			}

			if ((p = get_wantype_str_by_unit(upstream_unit)) != NULL)
				nvram_set("switch_upstream", p);

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
			/* If IPTV is not enabled, VLAN is enabled on primary/secondary WAN and WAN/WAN2 port is ued.
			 * vlanX shouldn't be used due to user may need to use same VLAN on two WAN ports.
			 */
			if ((type == WANS_DUALWAN_IF_WAN || type == WANS_DUALWAN_IF_WAN2) &&
			    nvram_pf_match(prefix, "dot1q", "1") &&
			    !((strlen(nvram_safe_get("switch_wantag")) > 0 && !nvram_match("switch_wantag", "none")) ||
			      (nvram_match("switch_wantag", "none") && (nvram_get_int("switch_stb_x") > 0 &&
			       nvram_get_int("switch_stb_x") <= 6)))) {
				int vid = nvram_pf_get_int(prefix, "vid");
				if (vid >= 2 && vid <= 4094) {
					snprintf(buf, sizeof(buf), "%s.%d", wphy, vid);
					wphy = buf;
				}
			}
#endif

			if (wphy)
				add_wan_phy(wphy);
		}

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
		if (!(wans_dualwan & WANSCAP_LAN) || !nvram_match("lan_trunk_type", "2")) {
			/* If wan2 or wan is not used as WAN and they are not used as LAN LACP port, bridge it to LAN. */
			if (wan2_orig && !(wans_dualwan & WANSCAP_WAN2))
				add_lan_phy(wan2_orig);
			if (wan && !(wans_dualwan & WANSCAP_WAN))
				add_lan_phy(wan);
		}
#elif defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033) || \
       defined(RTCONFIG_SWITCH_QCA8337N)
		/* If wan2 or wan is not used as WAN, bridge it to LAN. */
		if (wan2_orig && !(wans_dualwan & WANSCAP_WAN2)) {
			add_lan_phy(wan2_orig);
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
			add_nv_val("wired_ifnames", ' ', wan2_orig);
#endif
		}
		if (wan && !(wans_dualwan & WANSCAP_WAN)) {
			add_lan_phy(wan);
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
			add_nv_val("wired_ifnames", ' ', wan);
#endif
		}
		if (wan_ifaces[SFPP_IFACE_ID] && !(wans_dualwan & WANSCAP_SFPP)) {
			add_lan_phy(wan_ifaces[SFPP_IFACE_ID]);
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
			add_nv_val("wired_ifnames", ' ', wan_ifaces[SFPP_IFACE_ID]);
#endif
		}
#endif
#else /* !RTCONFIG_DUALWAN */
		add_lan_phy(wl2g);
		if (wl5g)
			add_lan_phy(wl5g);
		if (wl5g2)
			add_lan_phy(wl5g2);
		if (wl60g)
			add_lan_phy(wl60g);
		set_wan_phy(wan);
#endif /* !RTCONFIG_DUALWAN */
	}

#if (defined(RTCONFIG_DUALWAN) && defined(RTCONFIG_QCA))
#if defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X) || defined(RTCONFIG_SOC_IPQ40XX)
#else /* RT-AC55U || 4G-AC55U */
	if(enable_dw_wan) {
		nvram_set("vlan2hwname", "et0");
	}
	else {
		nvram_unset("vlan2hwname");
	}
#endif
#endif

	_dprintf("%s: WAN %s LAN %s [%s] "
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		"WIRED [%s] "
		"ETH_IFNAMES [%s] "
#endif
		"2G %s 5G %s "
#if defined(RTCONFIG_HAS_5G_2)
		"5G2 %s "
#endif
		"60G %s "
		"USB %s AP_LAN %s DW_WAN %s DW_LAN %s force_dwlan %d, sw_mode %d\n",
		__func__, wan, lan, nvram_safe_get("lan_ifnames"),
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_safe_get("wired_ifnames"),
		nvram_safe_get("eth_ifnames"),
#endif
		wl2g, wl5g? wl5g:"N/A",
#if defined(RTCONFIG_HAS_5G_2)
		wl5g2? wl5g2:"N/A",
#endif
		wl60g? wl60g:"N/A", usb, ap_lan? ap_lan:"N/A",
		dw_wan, dw_lan, force_dwlan, sw_mode);
	_dprintf("wan_ifnames: %s\n", nvram_safe_get("wan_ifnames"));

	return 0;
}
#endif

#ifdef RTCONFIG_BCM_7114
#define GMAC3_ENVRAM_RESTORE(name)				\
do {								\
	char *var;						\
	if ((var = cfe_nvram_get(name))) {			\
		nvram_set(name, var);				\
	}							\
} while (0)

static void
gmac3_restore_nvram()
{
	switch(get_model()) {
		case MODEL_RTAC87U:
		case MODEL_RTAC5300:
		case MODEL_RTAC88U:
			GMAC3_ENVRAM_RESTORE("et1macaddr");
			GMAC3_ENVRAM_RESTORE("et1phyaddr");
			GMAC3_ENVRAM_RESTORE("et1mdcport");

			nvram_unset("et0macaddr");
			nvram_unset("et0mdcport");
			nvram_unset("et0phyaddr");
			break;
		default: // MODEL_RTAC3100
			GMAC3_ENVRAM_RESTORE("et0macaddr");
			GMAC3_ENVRAM_RESTORE("et0mdcport");
			GMAC3_ENVRAM_RESTORE("et0phyaddr");

			nvram_unset("et1macaddr");
			nvram_unset("et1mdcport");
			nvram_unset("et1phyaddr");
			break;
	}

	nvram_unset("et2macaddr");
	nvram_unset("et2mdcport");
	nvram_unset("et2phyaddr");
	nvram_unset("fwd_wlandevs");
	nvram_unset("fwd_cpumap");
	nvram_unset("fwddevs");
}

#ifdef RTCONFIG_GMAC3
/* Override GAMC3 nvram */
static void
gmac3_override_nvram()
{
	switch(get_model()) {
		case MODEL_RTAC87U:
		case MODEL_RTAC5300:
		case MODEL_RTAC88U:
			nvram_set("et2mdcport", cfe_nvram_get("et1mdcport"));
			nvram_set("et2phyaddr", cfe_nvram_get("et1phyaddr"));
			nvram_set("et2macaddr", cfe_nvram_get("et1macaddr"));

			nvram_set("et1macaddr", "00:00:00:00:00:00");

			nvram_set("et0macaddr", "00:00:00:00:00:00");
			nvram_set("et0mdcport", cfe_nvram_get("et1mdcport"));
			nvram_set("et0phyaddr", cfe_nvram_get("et1phyaddr"));
			break;
		default: // MODEL_RTAC3100
			nvram_set("et2mdcport", cfe_nvram_get("et0mdcport"));
			nvram_set("et2phyaddr", cfe_nvram_get("et0phyaddr"));
			nvram_set("et2macaddr", cfe_nvram_get("et0macaddr"));

			nvram_set("et0macaddr", "00:00:00:00:00:00");

			nvram_set("et1macaddr", "00:00:00:00:00:00");
			nvram_set("et1mdcport", cfe_nvram_get("et0mdcport"));
			nvram_set("et1phyaddr", cfe_nvram_get("et0phyaddr"));
			break;
	}
	nvram_set("lan_hwaddr", nvram_safe_get("et2macaddr"));

	/* set model's fwd specific */
	switch(get_model()) {
		case MODEL_RTAC87U:

			break;
		case MODEL_RTAC5300:
			nvram_set("fwddevs", "fwd0 fwd1");
			nvram_set("fwd_cpumap", "d:l:5:163:0 d:x:2:163:0 d:u:5:169:1");
			nvram_set("fwd_wlandevs", "eth1 eth2 eth3");

			break;
		case MODEL_RTAC88U:
			nvram_set("fwddevs", "fwd1");
			if(is_router_mode()){
				if(*nvram_safe_get("new_map"))
					nvram_set("fwd_cpumap", nvram_safe_get("new_map"));
				else
					nvram_set("fwd_cpumap", "d:l:5:169:1");

				if(*nvram_safe_get("new_fwd_devs"))
					nvram_set("fwd_wlandevs", nvram_safe_get("new_fwd_devs"));
				else
					nvram_set("fwd_wlandevs", "eth2");
			}
			else if(is_psta(nvram_get_int("wlc_band")) || is_psr(nvram_get_int("wlc_band"))) {
				if(nvram_get_int("wlc_band") == 0) {
					/* 2G */
					nvram_set("fwd_cpumap", "d:x:2:163:1");
					nvram_set("fwd_wlandevs", "eth1");
				} else {
					/* 5G */
					nvram_set("fwd_cpumap", "d:l:5:169:1");
					nvram_set("fwd_wlandevs", "eth2");
				}
			} else {
				nvram_set("fwd_cpumap", "d:l:5:169:1");
				nvram_set("fwd_wlandevs", "eth2");
			}
			break;
		case MODEL_RTAC3100:
			nvram_set("fwddevs", "fwd0 fwd1");
			nvram_set("fwd_cpumap", "d:x:2:163:0 d:l:5:169:1");
			nvram_set("fwd_wlandevs", "eth1 eth2");

			break;
		case MODEL_RTAC3200:
			nvram_set("fwddevs", "fwd0 fwd1");
			nvram_set("fwd_cpumap", "d:u:5:163:0 d:x:2:169:1 d:l:5:169:1");
			nvram_set("fwd_wlandevs", "eth1 eth2 eth3");

			break;
		default:
			nvram_set("fwddevs", "fwd0 fwd1");
			nvram_set("fwd_wlandevs", "eth1 eth2 eth3");

			break;
	}

	/* vlan ports/hw are left in init_switch */
}

void chk_gmac3_excludes()
{

	if (nvram_get_int("gmac3_enable") == 0	// disable gmac3
	//|| (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none"))
#if defined(RTAC88U) || defined(RTAC3100)
	//|| is_router_mode()
#endif
	)
		gmac3_restore_nvram();
	else
		gmac3_override_nvram();
}
#endif
#endif

void record_prev_sys_uptime(void)
{
	nvram_set("sys_uptime_prev", nvram_get_int("sys_uptime_now") ? nvram_safe_get("sys_uptime_now") : "0");
	nvram_set("sys_uptime_now", "0");
}

#ifdef RTCONFIG_FRS_LIVE_UPDATE
#ifdef RTCONFIG_AHS
void init_ahs_bhc_params(void)
{
	nvram_set("ahs_bhc_sent", "0");
	if(nvram_match("ahs_bhc_log_ext", "A"))
	{
		//Change from A to B
		nvram_set("ahs_bhc_log_ext", "B");
	}
	else
	{
		//Change from B or NULL to A
		nvram_set("ahs_bhc_log_ext", "A");
	}
}
#endif /* RTCONFIG_AHS */
#endif /* RTCONFIG_FRS_LIVE_UPDATE */

void set_ethsw_ifnames()
{
	int model = get_model();

	switch (model) {
	case MODEL_EBG19:
		nvram_set("ethsw_ifnames", "eth4 eth3 eth2 eth1 ethsw_3 ethsw_2 ethsw_1 ethsw_0");
		nvram_set("sdn_ifnames", "eth4 eth3 eth2 eth1");
		break;
	default:
		nvram_set("ethsw_ifnames", nvram_safe_get("wired_ifnames"));
		nvram_set("sdn_ifnames", nvram_safe_get("wired_ifnames"));
		break;
	}
}

void init_subunit(void)
{

#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_FRONTHAUL_DWB) || defined(RTCONFIG_MSSID_PRELINK) || defined(RTCONFIG_FRONTHAUL_DBG) || defined(RTCONFIG_VIF_ONBOARDING)
	init_amas_subunit();
#endif
#endif

#ifdef RTCONFIG_OWE_TRANS
    append_owe_trans_vif(); // reserve vif for OWE-Transition mode
#endif

}

// intialized in this area
//  lan_ifnames
//  wan_ifnames
//  wl_ifnames
//  btn_xxxx_gpio
//  led_xxxx_gpio
static int reset_gpio = 1;
int init_nvram(void)
{
	const int sw_mode __attribute__((unused)) = sw_mode();
	int model = get_model();
	int x_Setting __attribute__((unused)) = nvram_get_int("x_Setting");
	char *wan_ifaces[MAX_WAN_IFACE_ID];
	char *wl_ifaces[WL_NR_BANDS];
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_DUALWAN) || defined(RTCONFIG_QCA) || (defined(RTCONFIG_BCMARM) && defined(RTCONFIG_AMAS) && defined(RTCONFIG_BCN_RPT))
#if !defined(RTCONFIG_LANTIQ)
	int unit = 0;
#endif
#endif
#if defined(RTCONFIG_DUALWAN)
#if defined(CONFIG_BCMWL5)
	char wan_if[IFNAMSIZ * 3];
#endif
	char wancaps[16];
#endif

#if defined(RTCONFIG_WANPORT2) || defined(PLAX56_XP4)
	char *wan0, *wan1, *lan_1, *lan_2, lan_ifs[IFNAMSIZ * 4];
#endif
#ifdef RTCONFIG_GMAC3
	char *hw_name = "et0";
#endif

#if defined(RTCONFIG_HIDDEN_BACKHAUL)
	char tmp_wl[20];
	sprintf(tmp_wl,"%s %s","ath1",WIF_5G_BH);
#endif

#if defined(RTCONFIG_AMAS)

#if defined(RTCONFIG_CFGSYNC) && defined(RTCONFIG_LACP)
	char wired_ifnames[64], tmp_ifnames[64];
#endif
#endif

#if defined (CONFIG_BCMWL5) && defined(RTCONFIG_TCODE)
	refresh_cfe_nvram();
	check_wl_country();
#endif

	TRACE_PT("init_nvram for model(%d)\n", model);

	int usb_usb3 __attribute__((unused)) = 0;
#ifdef RTCONFIG_USB
#ifdef RTCONFIG_USB_XHCI
	usb_usb3 = atoi(nvram_get("usb_usb3")? : nvram_default_get("usb_usb3")? : "0");
#endif
#endif

	/* set default value */
	memset(wan_ifaces, 0, sizeof(wan_ifaces));
	memset(wl_ifaces, 0, sizeof(wl_ifaces));
	nvram_set("rc_support", "");
	if (reset_gpio) {
		nvram_set_int("btn_rst_gpio", 0xff);
		nvram_set_int("btn_wps_gpio", 0xff);
		nvram_set_int("btn_radio_gpio", 0xff);
		nvram_set_int("led_pwr_gpio", 0xff);
		nvram_set_int("led_wps_gpio", 0xff);
		nvram_set_int("pwr_usb_gpio", 0xff);
		nvram_set_int("pwr_usb_gpio2", 0xff);
		nvram_set_int("led_usb_gpio", 0xff);
		nvram_set_int("led_lan_gpio", 0xff);
		nvram_set_int("led_lan1_gpio", 0xff);
		nvram_set_int("led_lan2_gpio", 0xff);
		nvram_set_int("led_lan3_gpio", 0xff);
		nvram_set_int("led_lan4_gpio", 0xff);
		nvram_set_int("led_wan_gpio", 0xff);
#if defined(RTCONFIG_WANLEDX2)
		nvram_unset("led_wan2_gpio");
#endif
#ifdef HND_ROUTER
		nvram_set_int("led_wan_normal_gpio", 0xff);
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
		nvram_set_int("led_extphy_gpio", 0xff);
#endif
		nvram_set_int("led_2g_gpio", 0xff);
		nvram_set_int("led_5g_gpio", 0xff);
		nvram_set_int("led_all_gpio", 0xff);
		nvram_set_int("led_logo_gpio", 0xff);
		nvram_set_int("led_usb3_gpio", 0xff);
		nvram_set_int("fan_gpio", 0xff);
		nvram_set_int("have_fan_gpio", 0xff);
#ifdef RTCONFIG_SWMODE_SWITCH
#if defined(PLAC66U)
		nvram_set_int("btn_swmode1_gpio", 0xff);
#else
		nvram_set_int("btn_swmode1_gpio", 0xff);
		nvram_set_int("btn_swmode2_gpio", 0xff);
		nvram_set_int("btn_swmode3_gpio", 0xff);
		nvram_set_int("swmode_switch", 0);
#endif	/* Model */
#endif	/* RTCONFIG_SWMODE_SWITCH */
#ifdef RTCONFIG_WIRELESS_SWITCH
		nvram_set_int("btn_wifi_gpio", 0xff);
#endif
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 0xff);
#else
		nvram_unset("btn_wltog_gpio");
#endif
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 0xff);
#else
		nvram_unset("btn_led_gpio");
#endif
#ifdef RTCONFIG_EJUSB_BTN
		nvram_set_int("btn_ejusb1_gpio", 0xff);
		nvram_set_int("btn_ejusb2_gpio", 0xff);
#endif
#ifdef RTCONFIG_TURBO_BTN
		nvram_set_int("btn_turbo_gpio", 0xff);
		nvram_set_int("led_turbo_gpio", 0xff);
#endif
#if defined(RTCONFIG_WANRED_LED)
		nvram_unset("led_wan_red_gpio");
#if defined(RTCONFIG_WANLEDX2)
		nvram_unset("led_wan2_red_gpio");
#endif
#endif
#if defined(RTCONFIG_PWRRED_LED)
		nvram_unset("led_pwr_red_gpio");
#endif
#if defined(RTCONFIG_FAILOVER_LED)
		nvram_unset("led_failover_gpio");
#endif
#if defined(RTCONFIG_M2_SSD)
		nvram_unset("led_sata_gpio");
#endif
#if defined(RTCONFIG_R10G_LED)
		nvram_unset("led_r10g_gpio");
#endif
#if defined(RTCONFIG_SFPP_LED)
		nvram_unset("led_sfpp_gpio");
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
		nvram_unset("btn_lte_gpio");
		nvram_unset("led_3g_gpio");
		nvram_unset("led_lte_gpio");
		nvram_unset("led_sig1_gpio");
		nvram_unset("led_sig2_gpio");
		nvram_unset("led_sig3_gpio");
#endif
#ifdef BLUECAVE
		nvram_unset("led_ctl_sig1_gpio");
		nvram_unset("led_ctl_sig2_gpio");
		nvram_unset("led_ctl_sig3_gpio");
		nvram_unset("led_idr_sig1_gpio");
		nvram_unset("led_idr_sig1_gpio");
#endif
		reset_gpio = 0;
	}
#ifdef RTCONFIG_NBR_RPT
	nvram_unset("channel_2g");
	nvram_unset("channelclass_2g");
	nvram_unset("channel_5g");
	nvram_unset("channelclass_5g");
	nvram_unset("channel_5g1");
	nvram_unset("channelclass_5g1");
	nvram_unset("r_selected5gband");
	nvram_unset("r_selected5gchannel");
	nvram_unset("r_selected5gbw");
	nvram_unset("r_selected5gnctrlsb");
#endif

	/* In the order of physical placement */
	nvram_set("ehci_ports", "");
	nvram_set("ohci_ports", "");
	nvram_unset("xhci_ports");

#if 0
	conf_swmode_support(model);
#endif
#ifdef RTCONFIG_SSH
	if(nvram_get_int("sshd_port")>0 && nvram_get_int("sshd_port_x")<=0)
		nvram_set("sshd_port_x", nvram_safe_get("sshd_port"));
#endif

#ifdef RTCONFIG_DSL_TCLINUX
	nvram_set("dsllog_opmode", "");
	nvram_set("dsllog_adsltype", "");
	nvram_set("dsllog_snrmargindown", "0");
	nvram_set("dsllog_snrmarginup", "0");
	nvram_set("dsllog_attendown", "0");
	nvram_set("dsllog_attenup", "0");
	nvram_set("dsllog_wanlistmode", "0");
	nvram_set("dsllog_dataratedown", "0");
	nvram_set("dsllog_datarateup", "0");
	nvram_set("dsllog_attaindown", "0");
	nvram_set("dsllog_attainup", "0");
	nvram_set("dsllog_powerdown", "0");
	nvram_set("dsllog_powerup", "0");
	nvram_set("dsllog_crcdown", "0");
	nvram_set("dsllog_crcup", "0");
	nvram_set("dsllog_farendvendorid", "");//far end vendor
	nvram_set("dsllog_tcm", "");//TCM, Trellis Coded Modulation
	nvram_set("dsllog_pathmodedown", "");//downstream path mode
	nvram_set("dsllog_interleavedepthdown", "");//downstream interleave depth
	nvram_set("dsllog_pathmodeup", "");//upstream path mode
	nvram_set("dsllog_interleavedepthup", "");//upstream interleave depth
	nvram_set("dsllog_vdslcurrentprofile", "");//VDSL current profile
#endif

#ifdef RTCONFIG_FRS_FEEDBACK
	if(2 == nvram_get_int("fb_state"))
	{
		nvram_set("fb_state", "0");
	}
	else if(0 != nvram_get_int("fb_state"))
	{
	nvram_set("fb_state", "");
	}
#endif
#ifdef RTCONFIG_PUSH_EMAIL
	nvram_set("PM_state", "");
#endif
	nvram_unset("usb_buildin");

#ifdef CONFIG_BCMWL5
#if defined(RTCONFIG_BCM4708) || defined(RTCONFIG_BCM_7114) || defined(RTCONFIG_HND_ROUTER)
	nvram_set_int("ure_disable", repeater_mode() || mediabridge_mode() || psr_mode() || dpsta_mode() ? 0 : 1);
#else
	nvram_set_int("ure_disable", repeater_mode() ? 0 : 1);
#endif
#elif defined(RTCONFIG_WIRELESSREPEATER)
	if (sw_mode() != SW_MODE_REPEATER)
		nvram_set("ure_disable", "1");
#endif

	/* initialize this value to check fw upgrade status */
	nvram_set_int("upgrade_fw_status", FW_INIT);

#ifdef RTCONFIG_DEFAULT_AP_MODE
	if (sw_mode() == SW_MODE_AP && !x_Setting) {
		nvram_set("lan_ipaddr", nvram_safe_get("lan_ipaddr_rt"));
		nvram_set("lan_netmask", nvram_safe_get("lan_netmask_rt"));
	}
#endif

	nvram_unset("wanduck_start_detect");

#if defined(RTCONFIG_WIFI_SON)
	if (x_Setting==1 && !nvram_match("wifison_ready", "1")) {
		if (nvram_get_int("hyd_cfg_time") && nvram_safe_get("wl0_wpa_psk")[0] != '\0') { /* old Lyra firmware */
			_dprintf("##### upgrade from pure Lyra firmware!!!\n");
			nvram_set("wifison_ready", "1");
			nvram_commit();
		}
	}
#ifdef RTCONFIG_AMAS
	else if (!x_Setting) {
		if (!nvram_get("bt_turn_off_service")) /* check if the trigger coming from BLE */
			nvram_set("wifison_ready", "0");
	}
#endif	/* RTCONFIG_AMAS */
#endif

#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_PRELINK)
	if (!x_Setting)
		nvram_set("amas_bdl_wanstate", "0");
#endif	/* RTCONFIG_PRELINK */
	if (nvram_get_int("re_mode") == 1)
		nvram_unset("led_ctrl_cap");
#endif	/* RTCONFIG_AMAS */

#if defined(RTCONFIG_AMAS_WGN)
    if (nvram_get_int("re_mode") == 0 && nvram_get("wgn_wloff_vifs"))
        nvram_unset("wgn_wloff_vifs");
#endif  // RTCONFIG_AMAS_WGN

	switch (model) {
#ifdef RTCONFIG_RALINK
	case MODEL_EAN66:
		nvram_set("lan_ifname", "br0");
		nvram_set("lan_ifnames", "eth2 ra0 rai0");
		nvram_set("wan_ifnames", "");
		nvram_set("wl_ifnames","ra0 rai0");
		nvram_set("wl0_vifnames", "");
		nvram_set("wl1_vifnames", "");
		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 26|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		//if (!nvram_get("ct_max"))
		//	nvram_set("ct_max", "30000");
		if (is_router_mode())
			nvram_set_int("sw_mode", SW_MODE_REPEATER);
		add_rc_support("mssid 2.4G 5G update");
		break;

#ifdef RTN65U
	case MODEL_RTN65U:
		nvram_set("boardflags", "0x100");	// although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");	// vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");	// vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "rai0";
		wl_ifaces[WL_5G_BAND] = "ra0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 26|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 24|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_ALL
		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("led_lan_gpio", 19|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 27|GPIO_ACTIVE_LOW);
		nvram_set("ehci_ports", "1-1 1-2");
		nvram_set("ohci_ports", "2-1 2-2");
		nvram_set("wl0_HT_TxStream", "2");	// for 300 Mbps RT3352 2T2R 2.4G
		nvram_set("wl0_HT_RxStream", "2");	// for 300 Mbps RT3352 2T2R 2.4G
		nvram_set("wl1_HT_TxStream", "3");	// for 450 Mbps RT3353 3T3R 2.4G/5G
		nvram_set("wl1_HT_RxStream", "3");	// for 450 Mbps RT3883 3T3R 2.4G/5G
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX2");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		break;
#endif	/* RTN65U */

#if defined(RTN67U)
	case MODEL_RTN67U:
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		set_basic_ifname_vars(wan_ifaces, "eth2", wl_ifaces, "usb", NULL, NULL, NULL, 0);

		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 26|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 27|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb2_gpio", 28|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_ALL
		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);
#endif
		nvram_set("ehci_ports", "1-1 1-2");
		nvram_set("ohci_ports", "2-1 2-2");
		nvram_set("wl0_HT_TxStream", "3");	// for 450 Mbps RT3353 3T3R 2.4G
		nvram_set("wl0_HT_RxStream", "3");	// for 450 Mbps RT3883 3T3R 2.4G
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G update usbX2");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		break;
#endif	/* RTN67U */

#if defined(RTN36U3)
	case MODEL_RTN36U3:
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		set_basic_ifname_vars(wan_ifaces, "eth2", wl_ifaces, "usb", NULL, NULL, NULL, 0);

		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 26|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 24|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 19|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 27|GPIO_ACTIVE_LOW);
		nvram_set("ehci_ports", "1-1 1-2");
		nvram_set("ohci_ports", "2-1 2-2");
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G update usbX2");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		break;
#endif	/* RTN36U3 */

#if defined(RTN56U)
	case MODEL_RTN56U:
		nvram_set("lan_ifname", "br0");

		if (sw_mode()==SW_MODE_AP) {
			nvram_set("lan_ifnames", "eth2 eth3 rai0 ra0");
			nvram_set("wan_ifnames", "");
		}
		else {  // router/default
			nvram_set("lan_ifnames", "eth2 rai0 ra0");
			nvram_set("wan_ifnames", "eth3");
		}
		nvram_set("wl_ifnames","rai0 ra0");
		// it is virtual id only for ui control
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_N56U_SR2
		nvram_set_int("btn_rst_gpio", 25|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_wps_gpio", 26|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 24|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_N56U_SR2
		nvram_set_int("led_lan_gpio", 31|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("led_lan_gpio", 19|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("led_wan_gpio", 27|GPIO_ACTIVE_LOW);
#ifndef RTCONFIG_N56U_SR2
		eval("rtkswitch", "11");		// for SR3 LAN LED
#endif
		nvram_set("ehci_ports", "1-1 1-2");
		nvram_set("ohci_ports", "2-1 2-2");
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
		add_rc_support("mssid");
		add_rc_support("2.4G 5G usbX2");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("no_phddns");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "3");
		break;
#endif	/* RTN56U */

#if defined(RTN14U)
	case MODEL_RTN14U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 42|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 41|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 40|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 43|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 43|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 72|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_usbbus_bled("led_usb_gpio", "1 2");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
//		if (!nvram_get("ct_max"))
		nvram_set("ct_max", "300000"); // force

		if (nvram_match("wl_mssid", "1"))
		add_rc_support("mssid");
		add_rc_support("2.4G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl"); //for hwnat only
		add_rc_support("manual_stb");
		add_rc_support("meoVoda");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		break;
#endif

#if defined(RTN11P) || defined(RTN300)
	case MODEL_RTN11P:
	case MODEL_RTN300:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 44|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 39|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 72|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);

		nvram_set("ct_max", "300000"); // force

		if (nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G update");
		add_rc_support("rawifi");
		add_rc_support("switchctrl"); //for hwnat only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		break;
#endif

#if defined(RTAC52U)
	case MODEL_RTAC52U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  2|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wltog_gpio",13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio",  8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  9|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_2g_gpio", 72|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_usbbus_bled("led_usb_gpio", "1 2");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "1");
		nvram_set("wl1_HT_RxStream", "1");
		break;
#endif

#if defined(RTAC51U)
	case MODEL_RTAC51U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  9|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_2g_gpio", 72|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_usbbus_bled("led_usb_gpio", "1 2");

		eval("rtkswitch", "11");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("pwrctrl");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "1");
		nvram_set("wl1_HT_RxStream", "1");
		break;
#endif	/* RTAC51U */

#if defined(RTN56UB1) || defined(RTN56UB2)
	case MODEL_RTN56UB1:
	case MODEL_RTN56UB2:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface

		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  6|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 14|GPIO_ACTIVE_LOW);
		//nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 7|GPIO_ACTIVE_LOW);
		eval("rtkswitch", "11");

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ra0");
		config_netdev_bled("led_5g_gpio", "rai0");
		config_usbbus_bled("led_usb_gpio", "1 2");

		nvram_set("ehci_ports", "1-1 1-2");
		nvram_set("ohci_ports", "2-1 2-2");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX2");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		//add_rc_support("11AC");
		//either txpower or singlesku supports rc.
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");

		break;

#endif	/* RTN56UB1 RTN56UB2 */

#if defined(RTN54U) || defined(RTAC54U)
	case MODEL_RTN54U:
	case MODEL_RTAC54U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  9|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_2g_gpio", 72|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		eval("rtkswitch", "11");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
#if defined(RTAC54U)
		add_rc_support("11AC");
		add_rc_support("pwrctrl");
#endif
		//either txpower or singlesku supports rc.
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTN54U or RTAC54U*/

#if defined(RTAC1200HP)
	case MODEL_RTAC1200HP:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  62|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  61|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 67|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  65|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  65|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 70|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 72|GPIO_ACTIVE_LOW);
		//nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 69|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 68|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 66|GPIO_ACTIVE_LOW);
#endif
		eval("rtkswitch", "11");

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ra0");
		config_netdev_bled("led_5g_gpio", "rai0");
		config_usbbus_bled("led_usb_gpio", "1 2");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		//either txpower or singlesku supports rc.
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTAC1200HP */

#if defined(RTAC1200)
	case MODEL_RTAC1200:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  5|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 6);
		nvram_set_int("led_pwr_gpio",  37|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  37|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 4|GPIO_ACTIVE_LOW);

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		//add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		//either txpower or singlesku supports rc.
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTAC1200 */

#if defined(RTAC1200V2)
	case MODEL_RTAC1200V2:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		// set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  5|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  11|GPIO_ACTIVE_LOW);
		//nvram_set_int("led_usb_gpio", 6);
		nvram_set_int("led_pwr_gpio",  37|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  37|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 4|GPIO_ACTIVE_LOW);

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "5120"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		//add_rc_support("2.4G 5G update usbX1");
		add_rc_support("2.4G 5G update");
		add_rc_support("rawifi");
		//add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		//either txpower or singlesku supports rc.
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTAC1200V2 */


#if defined(RTAC1200GU)
	case MODEL_RTAC1200GU:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  41|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  43|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 47|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 48|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  48|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 46|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 16|GPIO_ACTIVE_LOW);
		eval("rtkswitch", "11");

		nvram_set("ehci_ports", "1-2");
		nvram_set("ohci_ports", "2-2");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		//either txpower or singlesku supports rc.
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTAC1200GU */

#if defined(RTAC1200GA1)
	case MODEL_RTAC1200GA1:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  41|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  43|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 47|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 48|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  48|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 46|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 16|GPIO_ACTIVE_LOW);
		eval("rtkswitch", "11");

		nvram_set("ehci_ports", "1-2");
		nvram_set("ohci_ports", "2-2");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		//either txpower or singlesku supports rc.
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTAC1200GA1 */

#if defined(RTAC51UP)
	case MODEL_RTAC51UP:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_usbbus_bled("led_usb_gpio", "1 2");

		eval("rtkswitch", "11");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "15000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "1");
		nvram_set("wl1_HT_RxStream", "1");
		break;
#endif	/* RTAC51UP */

#if defined(RTAC53)
	case MODEL_RTAC53:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "NULL", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_usbbus_bled("led_usb_gpio", "1 2");

		eval("rtkswitch", "11");

		nvram_set("ct_max", "15000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "1");
		nvram_set("wl1_HT_RxStream", "1");
		break;
#endif	/* RTAC53 */

#if defined(RPAC87)
	case MODEL_RPAC87:
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "eth2", wl_ifaces, "NULL", "eth2", NULL, "NULL", 0);

		nvram_set_int("btn_rst_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 4|GPIO_ACTIVE_LOW);

		nvram_set_int("led_2g_green_gpio1", 42|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_green_gpio2", 43|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_green_gpio3", 41|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_green_gpio4", 44|GPIO_ACTIVE_LOW);

		nvram_set_int("led_5g_green_gpio1", 45|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_green_gpio2", 46|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_green_gpio3", 47|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_green_gpio4", 48|GPIO_ACTIVE_LOW);

		add_rc_support("2.4G 5G update");
		add_rc_support("rawifi");
		add_rc_support("11AC");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		nvram_set("wl_time", "2");
		break;
#endif	/* RP-AC87 */

#if defined(RTN800HP)
	case MODEL_RTN800HP:

		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wl_ifaces[WL_2G_BAND] = "ra0";

		wan_ifaces[WAN_IFACE_ID] = "eth3";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  16|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio",  13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio",   14|GPIO_ACTIVE_LOW);

		eval("rtkswitch", "11");

		config_swports_bled("led_wan_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ra0");

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G update");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");

		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");

		//nvram_set("wl_time", "15");
		break;
#endif	/* RT-N800HP */

#if defined(RTAC85U)
	case MODEL_RTAC85U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  16|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  18|GPIO_ACTIVE_LOW);
//		nvram_set_int("btn_wltog_gpio",13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio",  13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 4|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		eval("rtkswitch", "11");

		/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ra0");
		config_netdev_bled("led_5g_gpio", "rai0");
		config_usbbus_bled("led_usb_gpio", "1 2");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
#if defined(RTAC65U)
		nvram_set("wl0_HT_TxStream", "3");
		nvram_set("wl0_HT_RxStream", "3");
		nvram_set("wl1_HT_TxStream", "3");
		nvram_set("wl1_HT_RxStream", "3");
		add_rc_support("noaidisk nodm noftp");
#endif
		break;
#endif /*  RTAC85U  */

#if defined(RTAC85P)
	case MODEL_RTAC85P:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  3|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio",  4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 10|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		eval("rtkswitch", "11");

		/* enable bled */
		config_netdev_bled("led_2g_gpio", "ra0");
		config_netdev_bled("led_5g_gpio", "rai0");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("app");
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
#if defined(RTCONFIG_AMAS)
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			add_lan_phy((char *)APCLI_2G);
			add_lan_phy((char *)APCLI_5G);
			nvram_set("eth_ifnames", "eth3");
			nvram_set("sta_phy_ifnames", "apcli0 apclii0");
			nvram_set("sta_ifnames", "apcli0 apclii0");
		}
#endif
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "vlan1");
#endif
		break;
#endif /*  RTAC85P  */

#if defined(RTACRH26)
	case MODEL_RTACRH26:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  3|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 10|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		eval("rtkswitch", "11");

		/* enable bled */
		config_netdev_bled("led_2g_gpio", "ra0");
		config_netdev_bled("led_5g_gpio", "rai0");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("app");
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		break;
#endif /*  RTACRH26  */

#if defined(TUFAC1750)
	case MODEL_TUFAC1750:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth3";
		wl_ifaces[WL_2G_BAND] = "ra0";
		wl_ifaces[WL_5G_BAND] = "rai0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_rst_gpio",  3|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio",  6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio",  4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 10|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_all_gpio", 10|GPIO_ACTIVE_LOW);

		eval("rtkswitch", "11");

		/* enable bled */
		config_netdev_bled("led_2g_gpio", "ra0");
		config_netdev_bled("led_5g_gpio", "rai0");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("rawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("app");
		//add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		break;
#endif

#if defined(RTN11P_B1)
	case MODEL_RTN11P_B1:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ra0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1", NULL, "vlan3", 0);
		nvram_set_int("btn_rst_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 43|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 42|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 44|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 37|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 44|GPIO_ACTIVE_LOW);

	/* enable bled */
		config_swports_bled("led_wan_gpio", 0);
		config_swports_bled("led_lan_gpio", 0);
		//eval("rtkswitch", "11");

		nvram_set("ct_max", "1024"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G update");
		add_rc_support("rawifi");
		//add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		//add_rc_support("11AC");
		//either txpower or singlesku supports rc.
		add_rc_support("pwrctrl");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		//nvram_set("wl1_HT_TxStream", "2");
		//nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTN11P_B1 */

#ifdef RTCONFIG_DSL
	case MODEL_DSLN55U:
		nvram_set("lan_ifname", "br0");

		if (sw_mode()==SW_MODE_AP) {
			// switch need to well-set for this config
			nvram_set("lan_ifnames", "eth2.2 rai0 ra0");
			nvram_set("wan_ifnames", "");
		}
		else { // router/default
			nvram_set("lan_ifnames", "eth2.2 rai0 ra0");
			nvram_set("wan_ifnames", "eth2.1.1");
		}

#ifdef RTCONFIG_DUALWAN
		// Support dsl, lan, dsl/usb, lan/usb, dsl/lan, usb/lan only, so far
		if (get_dualwan_secondary()==WANS_DUALWAN_IF_NONE) {
			// dsl, lan
			if (get_dualwan_primary()==WANS_DUALWAN_IF_DSL) {
				nvram_set("wan_ifnames", "eth2.1.1");
			}
			else if (get_dualwan_primary()==WANS_DUALWAN_IF_LAN) {
				nvram_set("wan_ifnames", "eth2.4");
			}
		}
		else if (get_dualwan_secondary()==WANS_DUALWAN_IF_LAN) {
			//dsl/lan
			if (get_dualwan_primary()==WANS_DUALWAN_IF_DSL) {
				nvram_set("wan_ifnames", "eth2.1.1 eth2.4");
			}
			/* Paul add 2013/1/24 */
			//usb/lan
			else {
				nvram_set("wan_ifnames", "usb eth2.4");
			}
		}
		else {
			//dsl/usb, lan/usb
			if (get_dualwan_primary()==WANS_DUALWAN_IF_DSL) {
				nvram_set("wan_ifnames", "eth2.1.1 usb");
			}
			else if (get_dualwan_primary()==WANS_DUALWAN_IF_LAN) {
				nvram_set("wan_ifnames", "eth2.4 usb");
			}
		}
#endif

		nvram_set("wl_ifnames","rai0 ra0");
		// it is virtual id only for ui control
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set_int("btn_rst_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 26|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 25|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIRELESS_SWITCH
		nvram_set_int("btn_wifi_gpio", 1|GPIO_ACTIVE_LOW);
#endif

		nvram_set("ehci_ports", "1-1 1-2");
		nvram_set("ohci_ports", "2-1 2-2");

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
		add_rc_support("mssid");
		add_rc_support("2.4G 5G usbX2");
		add_rc_support("rawifi");
		add_rc_support("dsl");
#if defined(RTCONFIG_WIRELESS_SWITCH) || defined(RTCONFIG_WIFI_TOG_BTN)
		add_rc_support("wifi_hw_sw");
#endif
		add_rc_support("pwrctrl");
		add_rc_support("no_phddns");

		//Ren: set "spectrum_supported" in /router/dsl_drv_tool/tp_init/tp_init_main.c
		if (nvram_match("spectrum_supported", "1"))
		{
			add_rc_support("spectrum"); //Ren
		}

		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "3");
		break;
#endif // RTCONFIG_DSL

#if defined(RTN13U)
	case MODEL_RTN13U:
		nvram_set("boardflags", "0x310"); // although it is not used in ralink driver
		nvram_set("lan_ifname", "br0");
		nvram_set("lan_ifnames", "vlan0 ra0");
		nvram_set("wan_ifnames", "vlan1");
		nvram_set("wl_ifnames","ra0");
		nvram_set("wan_ports", "4");
		nvram_set("vlan0hwname", "et0"); // although it is not used in ralink driver
		nvram_set("vlan1hwname", "et0");

		nvram_set_int("btn_rst_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 7|GPIO_ACTIVE_LOW);
		break;
#endif	/* RTN13U */
#endif // RTCONFIG_RALINK

#if defined(MAPAC1300)
	case MODEL_MAPAC1300:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth1";
		wl_ifaces[WL_2G_BAND] = "ath0";
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		wl_ifaces[WL_5G_BAND] = tmp_wl;
#else
		wl_ifaces[WL_5G_BAND] = "ath1";
#endif
#ifdef RTCONFIG_DETWAN
		if(!nvram_match("wifison_ready", "1")) {
			set_defwan("eth1", "16", "32");
			set_basic_ifname_vars(wan_ifaces, "eth0", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
		} else {
			if((IPTV_ports_cnt() > 0) || (nvram_safe_get("wan0_ifname")[0] =='\0' && x_Setting==1)) {
				set_defwan("eth1", "16", "32");
			}

			if(nvram_safe_get("wan0_ifname")[0] == '\0' || sw_mode() != SW_MODE_ROUTER)
			{
				wan_ifaces[WAN_IFACE_ID] = "";
				if(sw_mode() == SW_MODE_ROUTER || nvram_match("cfg_master", "1"))
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
				else {
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
#else
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1", wl_ifaces, "usb", "eth0 eth1 sta1", "vlan2", "vlan3", 0);
#endif
				}
				nvram_set("detwan_proto", "-1");
				nvram_set("wanports_mask", "0");
				nvram_set("lanports_mask", "48");
			}
			else if(sw_mode() == SW_MODE_ROUTER)
			{
				char wantmp[10], lantmp[128];
				strcpy(wantmp, nvram_safe_get("wan0_ifname"));
				strcpy(lantmp, "eth0 eth1");
				if (string_remove(lantmp, wantmp)) {
					if(lantmp[strlen(lantmp)-1]==' ')
						lantmp[strlen(lantmp)-1] = '\0';
					wan_ifaces[WAN_IFACE_ID]=wantmp;
					set_basic_ifname_vars(wan_ifaces, lantmp, wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
				}
			}

			nvram_set("detwan_max", "2");
			nvram_set("detwan_name_0", "eth0");
			nvram_set("detwan_mask_0", "32");
			nvram_set("detwan_name_1", "eth1");
			nvram_set("detwan_mask_1", "16");
		}
#else
		wan_ifaces[WAN_IFACE_ID] = "eth0";
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", "vlan2", "vlan3", 0);
#else
		set_basic_ifname_vars(wan_ifaces, "eth1 sta1", wl_ifaces, "usb", "eth1 sta1", "vlan2", "vlan3", 0);
#endif
#endif	/* RTCONFIG_DETWAN */
#ifdef RTCONFIG_AMAS
		nvram_set_int("amas_path_stat", -1);
#endif
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 63|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
#endif

		nvram_set("ct_max", "300000"); // force

		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		add_rc_support("noaidisk");
		add_rc_support("noitunes");
		add_rc_support("nodm");
		add_rc_support("manual_stb");
		add_rc_support("app");
		add_rc_support("switchctrl");

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");

		nvram_set("wl0_vifnames", "wl0.1 wl0.2");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "eth0");
#endif

#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth1");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
		}
		add_led_ctrl_capability(LP55XX_LED);
#endif
		break;
#endif //MAPAC1300

#if defined(MAPAC2200)
	case MODEL_MAPAC2200:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		wl_ifaces[WL_5G_BAND] = tmp_wl;
#else
		wl_ifaces[WL_5G_BAND] = "ath1";
#endif
		wl_ifaces[WL_5G_2_BAND] = "ath2";
#ifdef RTCONFIG_DETWAN
		if(!nvram_match("wifison_ready", "1")) {
			set_defwan("eth0", "32", "16");
			set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
		} else {
			if((IPTV_ports_cnt() > 0) || (nvram_safe_get("wan0_ifname")[0] =='\0' && x_Setting==1)) {
				set_defwan("eth0", "32", "16");
			}

			if(nvram_safe_get("wan0_ifname")[0] == '\0' || sw_mode() != SW_MODE_ROUTER)
			{
				wan_ifaces[WAN_IFACE_ID] = "";
				if(sw_mode() == SW_MODE_ROUTER || nvram_match("cfg_master", "1"))
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
				else {
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
#else
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1", wl_ifaces, "usb", "eth0 eth1 sta1", "vlan2", "vlan3", 0);
#endif
				}
				nvram_set("detwan_proto", "-1");
				nvram_set("wanports_mask", "0");
				nvram_set("lanports_mask", "48");
			}
			else if(sw_mode() == SW_MODE_ROUTER)
			{
				char wantmp[10], lantmp[128];
				strcpy(wantmp, nvram_safe_get("wan0_ifname"));
				strcpy(lantmp, "eth0 eth1");
				if (string_remove(lantmp, wantmp)) {
					if(lantmp[strlen(lantmp)-1]==' ')
						lantmp[strlen(lantmp)-1] = '\0';
					wan_ifaces[WAN_IFACE_ID]=wantmp;
					set_basic_ifname_vars(wan_ifaces, lantmp, wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
				}
			}

			nvram_set("detwan_max", "2");
			nvram_set("detwan_name_0", "eth0");
			nvram_set("detwan_mask_0", "32");
			nvram_set("detwan_name_1", "eth1");
			nvram_set("detwan_mask_1", "16");
		}
#else
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", "vlan2", "vlan3", 0);
#else
		set_basic_ifname_vars(wan_ifaces, "eth1 sta1", wl_ifaces, "usb", "eth1 sta1", "vlan2", "vlan3", 0);
#endif
#endif	/* RTCONFIG_DETWAN */
		/* 2.4G tx power switch */
		gpio_dir(39, GPIO_DIR_OUT);
		if ((nvram_match("wl0_country_code", "GB") && nvram_invmatch("territory_code", "EU/02") && nvram_invmatch("territory_code", "UK/02"))
		  || nvram_match("wl0_country_code", "CN"))
			set_gpio(39, 1);	// 3.3v: echo 1 > /sys/class/gpio/gpio39/value
		else
			set_gpio(39, 0);	// 5v  : echo 0 > /sys/class/gpio/gpio39/value

		/* set DPDT */
		gpio_dir(44, GPIO_DIR_OUT);
		set_gpio(44, 1);
		gpio_dir(45, GPIO_DIR_OUT);
		set_gpio(45, 0);
		gpio_dir(46, GPIO_DIR_OUT);
		set_gpio(46, 1);
		gpio_dir(47, GPIO_DIR_OUT);
		set_gpio(47, 0);

		nvram_set_int("btn_rst_gpio", 34|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 18|GPIO_ACTIVE_LOW);

		nvram_set("ct_max", "300000"); // force

		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		add_rc_support("noaidisk");
		add_rc_support("noitunes");
		add_rc_support("nodm");
		add_rc_support("manual_stb");
		add_rc_support("app");
		add_rc_support("switchctrl");

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
#if defined(RTCONFIG_WIFI_SON)		
		nvram_set("wl0_vifnames", "wl0.1 wl0.2");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2");
#endif		
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "eth1");
#endif

#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth0");
			nvram_set("sta_phy_ifnames", "sta0 sta2 sta1");
			nvram_set("sta_ifnames", "sta0 sta2 sta1");		//set all sta interfaces
			if(strlen(nvram_safe_get("sta_priority")) == 15)
				nvram_set("sta_priority", "2 0 2 1 5 1 3 0 5 2 1 1");	//control used or not in priority[3]
			if(nvram_get("skip_ifnames") == NULL)
				nvram_set("skip_ifnames", "sta2");		//not to create this interface
			nvram_unset("dfschinfo");
		}
		add_led_ctrl_capability(LP55XX_LED);
#endif
		break;
#endif //MAPAC2200

#ifdef VZWAC1300
	case MODEL_VZWAC1300:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");
		wl_ifaces[WL_2G_BAND] = "ath0";
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		wl_ifaces[WL_5G_BAND] = tmp_wl;
#else
		wl_ifaces[WL_5G_BAND] = "ath1";
#endif
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		if(sw_mode() == SW_MODE_ROUTER || nvram_match("cfg_master", "1"))
			set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
		else {
				wan_ifaces[WAN_IFACE_ID] = "";
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
				set_basic_ifname_vars(wan_ifaces, "eth0 eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
#else
#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_REPEATER_STAALLBAND)
				if (sw_mode()==SW_MODE_REPEATER)
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1 sta2 sta0", wl_ifaces, "usb", "eth0 eth1 sta1 sta2 sta0", "vlan2", "vlan3", 0);
				else
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1", wl_ifaces, "usb", "eth0 eth1 sta1", "vlan2", "vlan3", 0);
#else
				set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1", wl_ifaces, "usb", "eth0 eth1 sta1", "vlan2", "vlan3", 0);
#endif // REPEATER
#endif
		}

		nvram_set_int("btn_rst_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 63|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
#endif

		nvram_set("ct_max", "300000"); // force

		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		add_rc_support("noaidisk");
		add_rc_support("noitunes");
		add_rc_support("nodm");
		add_rc_support("switchctrl");

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");

		nvram_set("wl0_vifnames", "wl0.1 wl0.2");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		set_default_wifi_rate();
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "eth1");
#endif
		break;
#endif

#ifdef SHAC1300
	case MODEL_SHAC1300:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		if(sw_mode() == SW_MODE_ROUTER || nvram_match("cfg_master", "1"))
			set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth0 eth1", "vlan2", "vlan3", 0);
		else {
				wan_ifaces[WAN_IFACE_ID] = "";
#if defined(RTCONFIG_WIRELESSREPEATER) && defined(RTCONFIG_REPEATER_STAALLBAND)
				if (sw_mode()==SW_MODE_REPEATER)
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1 sta2 sta0", wl_ifaces, "usb", "eth0 eth1 sta1 sta2 sta0", "vlan2", "vlan3", 0);
				else
					set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1", wl_ifaces, "usb", "eth0 eth1 sta1", "vlan2", "vlan3", 0);
#else
				set_basic_ifname_vars(wan_ifaces, "eth0 eth1 sta1", wl_ifaces, "usb", "eth0 eth1 sta1", "vlan2", "vlan3", 0);
#endif // REPEATER
		}
#ifdef RTCONFIG_AMAS
		nvram_set_int("amas_path_stat", -1);
#endif
		nvram_set_int("btn_rst_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 63|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
#endif

		nvram_set("ct_max", "300000"); // force

		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		add_rc_support("noaidisk");
		add_rc_support("noitunes");
		add_rc_support("nodm");
		add_rc_support("switchctrl");

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");

		nvram_set("wl0_vifnames", "wl0.1 wl0.2");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		set_default_wifi_rate();
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "eth0");
#endif

#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth0");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
		}
#endif
		break;
#endif //SHAC1300

#if defined(MAPAC1750)
	case MODEL_MAPAC1750:	/* fall through */
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ath0";
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		wl_ifaces[WL_5G_BAND] = tmp_wl;
#else
		wl_ifaces[WL_5G_BAND] = "ath1";
#endif
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1 vlan2", NULL, "vlan3", 0);
#else
		set_basic_ifname_vars(wan_ifaces, "vlan1 sta1", wl_ifaces, "usb", "vlan1 vlan2 sta1", NULL, "vlan3", 0);
#endif

		nvram_set_int("btn_rst_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_blue_gpio", 14);
		nvram_set_int("led_green_gpio", 15);
		nvram_set_int("led_red_gpio", 16);

		/* enable Bluetooth functionality */
		gpio_dir(7, GPIO_DIR_OUT);
		set_gpio(7, 1);

		/* enable bled */
		config_netdev_bled("led_blue_gpio", "ath0");
		add_gpio_to_bled("led_blue_gpio", "led_green_gpio");
		add_gpio_to_bled("led_blue_gpio", "led_red_gpio");
		if (nvram_match("success_start_service", "0"))
			set_rgbled(RGBLED_BOOTING);

		/* Etron xhci host:
		 *	USB2 bus: 1-1
		 *	USB3 bus: 2-1
		 * Internal ehci host:
		 * 	USB2 bus: 3-1
		 */
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		add_rc_support("manual_stb");
		add_rc_support("switchctrl");
#if defined(RTCONFIG_QCA_LBD)
		add_rc_support("bandstr");
#endif

		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "3");
		nvram_set("wl0_HT_RxStream", "3");
		nvram_set("wl1_HT_TxStream", "3");
		nvram_set("wl1_HT_RxStream", "3");
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "vlan1");
#endif

#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "vlan2");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
		}
#endif
		break;
#endif	/* MAPAC1750 */

#if defined(RTAC95U)
	case MODEL_RTAC95U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");
		wl_ifaces[WL_2G_BAND] = "ath0";
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		wl_ifaces[WL_5G_2_BAND] = "ath2";
#ifdef RTCONFIG_AMAS
		if(nvram_match("re_mode", "1")) //RE mode.
			set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth0 eth1", NULL, "eth2", 0);
		else
#endif
		{
			set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", NULL, "eth2", 0);
		}

		nvram_set_int("btn_rst_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 54|GPIO_ACTIVE_LOW);

		/* Etron xhci host:
		 *	USB2 bus: 1-1
		 *	USB3 bus: 2-1
		 * Internal ehci host:
		 * 	USB2 bus: 3-1
		 */
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
#endif

		nvram_set("ct_max", "300000"); // force

		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		add_rc_support("noitunes");
		add_rc_support("manual_stb");
		add_rc_support("switchctrl");
		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");

#if defined(RTCONFIG_QCA_LBD)
		add_rc_support("bandstr");
#endif
#if defined(RTCONFIG_AMAS_WGN)
                add_rc_support("amas_wgn");
#endif
		add_rc_support("app");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		nvram_set("wl2_HT_TxStream", "4");
		nvram_set("wl2_HT_RxStream", "4");
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "eth1");
#endif
#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth0");
			nvram_set("sta_phy_ifnames", "sta0 sta1 sta2");
			nvram_set("sta_ifnames", "sta0 sta1 sta2");
			nvram_set("sta_priority", "2 0 2 1 5 1 3 0 5 2 1 1");	//control used or not in priority[3]
			nvram_set("skip_ifnames", "sta1");		//not to create this interface
			nvram_unset("dfschinfo");
		}
		add_led_ctrl_capability(LP55XX_LED);
#endif
		break;
#endif //RTAC95U

#if defined(RTAC55U) || defined(RTAC55UHP)
	case MODEL_RTAC55U:	/* fall through */
	case MODEL_RTAC55UHP:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", "vlan2", "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio",  4);
		nvram_set_int("led_lan_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 15);
		nvram_set_int("led_2g_gpio", 13);
#if defined(RTCONFIG_ETRON_XHCI_USB3_LED)
		/* SR1 */
		nvram_set("led_usb3_gpio", "etron");
#else
		nvram_set_int("led_5g_gpio", 0);
		nvram_set_int("led_usb3_gpio", 1);
#endif
		nvram_set_int("led_pwr_gpio", 19|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_red_gpio", 14);
#ifdef RTCONFIG_USBRESET
		nvram_set_int("pwr_usb_gpio", 21);
#endif

		/* enable bled */
		config_netdev_bled("led_wan_gpio", "eth0");
		config_swports_bled_sleep("led_lan_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ath0");

#if defined(RTCONFIG_ETRON_XHCI_USB3_LED)
		/* SR1 */
#else
		config_netdev_bled("led_5g_gpio", "ath1");
#endif

		config_usbbus_bled("led_usb_gpio", "3 4");
		config_usbbus_bled("led_usb3_gpio", "1 2");

		/* Etron xhci host:
		 *	USB2 bus: 1-1
		 *	USB3 bus: 2-1
		 * Internal ehci host:
		 * 	USB2 bus: 3-1
		 */
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX2");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RTAC55U | RTAC55UHP */

#ifdef RT4GAC55U
	case MODEL_RT4GAC55U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", "vlan2", "vlan3", 0);

		nvram_set_int("btn_wps_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_lte_gpio", 21|GPIO_ACTIVE_LOW);
#ifdef RTAC55U_SR1
		nvram_set_int("led_usb_gpio",  4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio" , 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio" , 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 19|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 19|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lte_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig1_gpio",18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig2_gpio",23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig3_gpio",22|GPIO_ACTIVE_LOW);
#else	/* RTAC55U_SR2 */
		nvram_set_int("led_pwr_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lte_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio" ,  4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio" , 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig1_gpio",19|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig2_gpio",20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig3_gpio",15|GPIO_ACTIVE_LOW);
#endif	/* RTAC55U_SR */

#ifdef RTCONFIG_WIRELESS_SWITCH
		nvram_set_int("btn_wifi_gpio", 1|GPIO_ACTIVE_LOW);
		add_rc_support("wifi_hw_sw");
#endif	/* RTCONFIG_WIRELESS_SWITCH */

		/* enable bled */
		config_netdev_bled("led_wan_gpio", "eth0");
		config_swports_bled_sleep("led_lan_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ath0");
		config_netdev_bled("led_5g_gpio", "ath1");

		config_usbbus_bled("led_usb_gpio", "1 2");
		nvram_set("ehci_ports", "1-1 2-1");
		nvram_set("usb_buildin", "2");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		break;
#endif	/* RT4GAC55U */

#if defined(RTN19)
	case MODEL_RTN19:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ath0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1", "vlan2", "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 5|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_lan1_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 3|GPIO_ACTIVE_LOW);
#endif

		/* enable bled */
		config_netdev_bled("led_2g_gpio", "ath0");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G update");
		add_rc_support("qcawifi");
		add_rc_support("manual_stb");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		break;
#endif	/* RTN19 */

#if defined(RTAC59U)
	case MODEL_RTAC59U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1 vlan2", "vlan2", "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio",  5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 16|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_netdev_bled("led_2g_gpio", "ath0");
		config_netdev_bled("led_5g_gpio", "ath1");

		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("nodm");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");
		if (!strncmp(nvram_safe_get("territory_code"), "CX/05", 5)) {
			add_rc_support("pwrctrl");
			add_rc_support("nz_isp");
		}
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "vlan1");
#endif

#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "vlan2");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
		}
#endif
		break;
#endif	/* RTAC59U */

#if defined(RTAC59_CD6R) || defined(RTAC59_CD6N)
	case MODEL_RTAC59CD6R:
	case MODEL_RTAC59CD6N:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
#if defined(RTAC59_CD6N) /* AMAS_ETHDETECT=y */
		nvram_set("vlan4hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
#else
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
#endif
		nvram_set("lan_ifname", "br0");
#if defined(RTAC59_CD6N) /* AMAS_ETHDETECT=y */
		wan_ifaces[WAN_IFACE_ID] = "vlan1";
#else
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
#endif
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
#if defined(RTAC59_CD6N) /* AMAS_ETHDETECT=y */
		set_basic_ifname_vars(wan_ifaces, "vlan4", wl_ifaces, NULL, "vlan1 vlan4", NULL, NULL, 0);
#else
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1 vlan2", "vlan2", "vlan3", 0);
#endif

		nvram_set_int("btn_rst_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_blue_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_red_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_white_gpio", 16|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_netdev_bled("led_blue_gpio", "ath0");
		add_gpio_to_bled("led_blue_gpio", "led_green_gpio");
		add_gpio_to_bled("led_blue_gpio", "led_red_gpio");
		if (nvram_match("success_start_service", "0"))
			set_rgbled(RGBLED_BOOTING);

		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("nodm");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");
#if defined(RTCONFIG_QCA_LBD)
		add_rc_support("bandstr");
#endif
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
#if defined(RTCONFIG_AMAS_ETHDETECT)
		if (aimesh_re_node())
			nvram_set("wired_ifnames", "");
		else
#endif
			nvram_set("wired_ifnames", "vlan1");
#endif

#if defined(RTAC59_CD6N) /* AMAS_ETHDETECT=y */
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "0")) {
			/* for ATE test in AP mode */
			nvram_set("eth_ifnames", "");
		}
		else {
			nvram_set("wl1_channel", "36"); /* fixed channel speeds up sta1 connection */
			nvram_set("eth_ifnames", "vlan1 vlan4");
			nvram_set("amas_ethif_type", "4 4");
			nvram_set("eth_priority", "0 1 1 1 2 1");
			nvram_set("sta_priority", "2 0 4 1 5 1 3 1");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
		}
		nvram_set("disable_ui", "0");
		add_led_ctrl_capability(LED_ON_OFF);
#else
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "vlan2");
			nvram_set("amas_ethif_type", "4");
			nvram_set("eth_priority", "0 1 1");
			nvram_set("sta_priority", "2 0 4 1 5 1 3 1");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
			add_led_ctrl_capability(LED_ON_OFF);
		}
#endif
		break;
#endif	/* RTAC59_CD6R || RTAC59_CD6N */

#if defined(PLN12)
	case MODEL_PLN12:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ath0";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1", NULL, "vlan3", 0);

		/* Enable 1st 2G guest network only. */
		nvram_set("wl0_vifnames", "wl0.1");
		nvram_set("wl1_vifnames", "");

		nvram_set_int("btn_rst_gpio", 15);
		nvram_set_int("btn_wps_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_red_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_green_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_orange_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_red_gpio", 17|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled_sleep("led_lan_gpio", 0);
		//config_netdev_bled("led_2g_green_gpio", "ath0");
		//config_netdev_bled("led_2g_orange_gpio", "ath0");
		config_netdev_bled("led_2g_red_gpio", "ath0");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G update");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("swmode_switch");
#ifdef RTCONFIG_DHCP_OVERRIDE
		add_rc_support("dhcp_override");

		if (nvram_match("dhcp_enable_x", "1") || !x_Setting)
			nvram_set("dnsqmode", "2");
		else
			nvram_set("dnsqmode", "1");
#endif
		add_rc_support("plc");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		/* avoid inserting unnecessary kernel modules */
		nvram_set("nf_ftp", "0");
		nvram_set("nf_pptp", "0");
		break;
#endif	/* PLN12 */

#if defined(PLAC56)
	case MODEL_PLAC56:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, NULL, "vlan1", NULL, "vlan3", 0);
		nvram_set("wl0_vifnames", "wl0.1 wl0.2");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2");

		nvram_set_int("btn_rst_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("plc_wake_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_red_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_green_gpio", 19|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_red_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_green_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_red_gpio", 7|GPIO_ACTIVE_LOW);

		/* enable bled */
		config_swports_bled_sleep("led_lan_gpio", 0);
		config_netdev_bled("led_2g_green_gpio", "ath0");
		//config_netdev_bled("led_2g_red_gpio", "ath0");
		config_netdev_bled("led_5g_green_gpio", "ath1");
		//config_netdev_bled("led_5g_red_gpio", "ath1");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("swmode_switch");
		add_rc_support("11AC");
#ifdef RTCONFIG_DHCP_OVERRIDE
		add_rc_support("dhcp_override");

		if (nvram_match("dhcp_enable_x", "1") || !x_Setting)
			nvram_set("dnsqmode", "2");
		else
			nvram_set("dnsqmode", "1");
#endif
		add_rc_support("plc");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
		/* avoid inserting unnecessary kernel modules */
		nvram_set("nf_ftp", "0");
		nvram_set("nf_pptp", "0");
		break;
#endif	/* PLAC56 */

#if defined(PLAC66U)
	case MODEL_PLAC66U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("vlan1hwname", "et1");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("vlan2hwname", "et1");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "vlan2";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "vlan1", wl_ifaces, "usb", "vlan1", NULL, "vlan3", 0);

		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 7|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_SWMODE_SWITCH
		nvram_set_int("btn_swmode1_gpio", 19|GPIO_ACTIVE_LOW);
		add_rc_support("swmode_switch");
#endif

		/* enable bled */
		config_netdev_bled("led_lan_gpio", "eth0");
		config_netdev_bled("led_2g_gpio", "ath0");
		config_netdev_bled("led_5g_gpio", "ath1");

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "3");
		nvram_set("wl0_HT_RxStream", "3");
		nvram_set("wl1_HT_TxStream", "3");
		nvram_set("wl1_HT_RxStream", "3");
		break;
#endif	/* PLAC66U */

#if defined(PLAX56_XP4)
//#define XP4_OLD
	case MODEL_PLAX56XP4:
		{ // XP4
			/* NEED to sync with the default HwVer in flash image */
			int xp4_old = (nvram_get_int("HwVer") < 1);
		/* PLC on eth1 */
		if (!strlen(nvram_safe_get("HwId"))) { // for old SR sample
			_dprintf("!!!WARNING!!! No HwId found, setto A!!\n");
			nvram_set("HwId", "A");
		}
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");

		if (nvram_match("HwId", "A")) {
			wan_ifaces[WAN_IFACE_ID] = "eth4";
		} else {
			wan_ifaces[WAN_IFACE_ID] = "eth2"; // LAN1 port, follow CD6N rule
		}
		wl_ifaces[WL_5G_BAND] = "ath0";
		wl_ifaces[WL_2G_BAND] = "ath1";
		//doSystem("ls /proc/device-tree/soc | grep dp | wc -l > /tmp/dp_cnt");
		if (nvram_match("HwId", "A")) {
#ifdef RTCONFIG_DUALWAN
			lan_1=NULL;
			strcpy(lan_ifs,"eth3 eth2 eth1");
			if (sw_mode() == SW_MODE_ROUTER && get_wans_dualwan() & WANSCAP_LAN) {
				memset(lan_ifs,0, sizeof(lan_ifs));
				if (nvram_match("wans_lanport", "1")) //lan port1
				{	
					lan_1="eth2";
					strcpy(lan_ifs,"eth3 eth1");
				}
				else if (nvram_match("wans_lanport", "2")) //lan port2
				{			
					lan_1="eth3";
					strcpy(lan_ifs,"eth2 eth1");
				}
				else
					_dprintf("error setting\n");
			}
			set_basic_ifname_vars(wan_ifaces, lan_ifs, wl_ifaces, "usb", NULL, NULL , lan_1, 0);
#else				
			set_basic_ifname_vars(wan_ifaces, "eth3 eth2 eth1", wl_ifaces, "usb", NULL, NULL, NULL, 0);
#endif			
		} else {
			set_basic_ifname_vars(wan_ifaces, "eth3 eth1", wl_ifaces, NULL, NULL, NULL, NULL, 0);
		}
	    if (xp4_old)
		nvram_unset("btn_rst_gpio");
	    else if(0)
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
	    else
		nvram_set_int("btn_rst_gpio", 34|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_blue_gpio", 25);
	    if (xp4_old) {
		nvram_set_int("led_green_gpio", 33);
		nvram_set_int("led_red_gpio", 32);
	    } else {
		nvram_set_int("led_green_gpio", 32);
		nvram_set_int("led_red_gpio", 33);
	    }
		nvram_set_int("led_white_gpio", 22);

		/* enable bled */
		config_netdev_bled("led_blue_gpio", "ath1");
		add_gpio_to_bled("led_blue_gpio", "led_green_gpio");
		add_gpio_to_bled("led_blue_gpio", "led_red_gpio");
		if (RGBLED_WHITE & RGBLED_WLED)
			add_gpio_to_bled("led_blue_gpio", "led_white_gpio");
		if (nvram_match("success_start_service", "0"))
			set_rgbled(RGBLED_BOOTING);

		if (nvram_match("HwId", "A")) {
#ifdef RTCONFIG_XHCIMODE
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "");
#else
			if(usb_usb3 == 1){
				nvram_set("xhci_ports", "2-1");
				nvram_set("ehci_ports", "1-1");
				nvram_set("ohci_ports", "");
			}
			else{
				nvram_set("ehci_ports", "1-1");
				nvram_set("ohci_ports", "");
			}
#endif
			add_rc_support("usbX1");
		} else {
			nvram_set("xhci_ports", "");
			nvram_set("ehci_ports", "");
			nvram_set("ohci_ports", "");
			nvram_set_int("usb_enable", 0);
			nvram_set_int("usb_usb3", 0);
			nvram_set_int("usb_usb2", 0);
			nvram_set_int("usb_ohci", 0);
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("noitunes");
		add_rc_support("wpa3");
		add_rc_support("ofdma");
		add_rc_support("app");

		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
#if defined(RTCONFIG_AMAS) /* AMAS_ETHDETECT should be enabled */
		if (nvram_match("HwId", "B"))
			nvram_set("wired_ifnames", "");
		else { // HwId A, XP4R only PLC(eth1) is dynamic WAN/LAN
			if (aimesh_re_node())
				nvram_set("wired_ifnames", "eth2 eth3");
			else
				nvram_set("wired_ifnames", "eth2 eth3 eth1");
		}
#endif

#if defined(RTCONFIG_AMAS) /* AMAS_ETHDETECT should be enabled */
		if (nvram_match("HwId", "B")) { // Node, follow CD6N
			if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "0")) {
				/* for ATE test in AP mode */
				nvram_set("eth_ifnames", "");
			} else {
				nvram_set("wl1_channel", "36"); /* fixed channel speeds up sta1 connection */
				nvram_set("eth_ifnames", "eth1 eth2 eth3"); /* PLC, LAN1, LAN2 */
				nvram_set("amas_ethif_type", "65536 4 4"); /* PLC, 1G, 1G */
				nvram_set("eth_priority", "0 3 1" " 1 1 1" " 2 2 1"); /* PLC priority:3, LAN1:1, LAN2:2 */
				nvram_set("sta_priority", "2 0 5 1" " 5 1 4 1"); /* 2G priority:5, 5G priority:4 */
				nvram_set("sta_phy_ifnames", "sta1 sta0"); /* 2G name, 5G name */
				nvram_set("sta_ifnames", "sta1 sta0"); /* 2G name, 5G name */
				nvram_unset("dfschinfo");
			}
			nvram_set("disable_ui", "0");
			add_led_ctrl_capability(LED_ON_OFF);
		} else { // Router
			if (nvram_get_int("x_Setting") == 0 || (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1"))) {
				_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
							__func__, __LINE__,
							sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
				nvram_set("eth_ifnames", "eth1 eth4"); /* PLC(eth1), normal WAN(eth4)*/
				nvram_set("fixed_eth_ifnames", "eth4"); /* XP4R WAN(eth4) only as wan */
				nvram_set("amas_ethif_type", "65536 4"); /* PLC, 1G */
				nvram_set("eth_priority", "0 2 1" " 1 1 1"); /* PLC priority:2, WAN priority:1 */
				nvram_set("sta_priority", "2 0 4 1" " 5 1 3 1"); /* 2G priority:4, 5G priority:3 */
				nvram_set("sta_phy_ifnames", "sta1 sta0"); /* 2G name, 5G name */
				nvram_set("sta_ifnames", "sta1 sta0"); /* 2G name, 5G name */
				nvram_unset("dfschinfo");
				add_led_ctrl_capability(LED_ON_OFF);
			}
			else {
				nvram_unset("eth_ifnames");	//unset, or the eth1 would not be add to LAN bridge (br0)
			}
		}
		if (strcmp(get_2g_hwaddr(), "00:AA:BB:CC:DD:E0") == 0 && strcmp(get_5g_hwaddr(), "00:AA:BB:CC:DD:E4") == 0) {
			nvram_set("eth_ifnames", "eth4"); /* only normal WAN(eth4), so PLC could be keep in bridge for PLC test in factory */
		}

		/* interface name & type mapping for lldp */
		nvram_set("amas_lldp_ifnames", "eth1 eth2 eth3"); /* PLC, LAN1, LAN2 */
		nvram_set("amas_lldp_iftypes", "65536 4 4"); /* PLC, 1G, 1G */
#endif
#if defined(RTCONFIG_QCA_EZMESH)
		if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1"))
			nvram_set("role","1");		
#endif		
		} // XP4
		break;
#endif	/* PLAX56_XP4 */

#if defined(ETJ)
	case MODEL_ETJ:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath2";		/* 5G */
		wl_ifaces[WL_5G_2_BAND] = "ath1";	/* 6G */
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", NULL, "vlan2", "vlan3", 0);
		
		nvram_set_int("btn_rst_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 38|GPIO_ACTIVE_LOW);
		// PWM channel R:0, G:1, B:2
		pwm_export(0, 100, 100); // period: 100ns, duty cycle:100
		pwm_export(1, 100, 100); // period: 100ns, duty cycle:100
		pwm_export(2, 100, 100); // period: 100ns, duty cycle:100
		nvram_set_int("led_blue_gpio", 2);
		nvram_set_int("led_green_gpio", 1);
		nvram_set_int("led_red_gpio", 0);
		// bled
		config_netdev_bled("led_blue_gpio", "ath1");
		add_gpio_to_bled("led_blue_gpio", "led_green_gpio");
		add_gpio_to_bled("led_blue_gpio", "led_red_gpio");
		if (nvram_match("success_start_service", "0"))
			set_rgbled(RGBLED_BOOTING);

		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "");
		add_rc_support("usbX1");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("noitunes");
		add_rc_support("nodm");
		add_rc_support("wpa3");
		add_rc_support("ofdma");
#if defined(RTCONFIG_SPF11_4_QSDK) || defined(RTCONFIG_SPF11_5_QSDK)
		add_rc_support("non_frameburst");
#endif
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		nvram_set("wired_ifnames", "eth1");

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "");
		}
		else{
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "");
		}
#endif

#if defined(RTCONFIG_AMAS)
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth0");
			nvram_set("eth_priority", "0 1 1"); /* ETH priority:1 */
			nvram_set("sta_phy_ifnames", "sta0 sta2 sta1"); /* 2G name, 5G name, 6G name */
			nvram_set("sta_ifnames", "sta0 sta2 sta1"); /* 2G name, 5G name, 6G name */
			nvram_set("sta_priority", "2 0 4 1" " 5 1 3 1" " 6 2 2 1"); /* 2G priority:4, 5G priority:3, 6G priority:2 */
			nvram_unset("dfschinfo");
			//add_led_ctrl_capability(LED_ON_OFF);
		}
#endif
		break;
#endif // ETJ

#if defined(RTAX57Q)
	case MODEL_RTAX57Q:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, NULL, "eth0 eth1", "eth0", "vlan3", 0);
		
		nvram_set_int("btn_rst_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 38|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 25);
		nvram_set_int("led_wan_gpio", 24);
		nvram_set_int("led_wan_red_gpio", 23);
		nvram_set_int("led_2g_gpio", 26);
		nvram_set_int("led_5g_gpio", 28);
		nvram_set_int("led_wps_gpio", 31);
		nvram_set_int("led_pwr_gpio", 31);

		/* enable bled */
		config_netdev_bled("led_lan_gpio", "eth1");
		config_netdev_bled("led_wan_gpio", "eth0");
		config_netdev_bled("led_2g_gpio", "ath0");
		config_netdev_bled("led_5g_gpio", "ath1");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("noitunes");
		add_rc_support("nodm");
		add_rc_support("wpa3");
		add_rc_support("ofdma");
#if defined(RTCONFIG_SPF11_4_QSDK) || defined(RTCONFIG_SPF11_5_QSDK)
		add_rc_support("non_frameburst");
#endif
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		nvram_set("wired_ifnames", "eth1");

#ifdef RTCONFIG_USB
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "");
		}
		else{
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "");
		}
#endif
#endif

#if defined(RTCONFIG_AMAS)
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth0");
			nvram_set("eth_priority", "0 1 1"); /* ETH priority:1 */
			nvram_set("sta_phy_ifnames", "sta0 sta1"); /* 2G name, 5G name */
			nvram_set("sta_ifnames", "sta0 sta1"); /* 2G name, 5G name */
			nvram_set("sta_priority", "2 0 3 1" " 5 1 2 1"); /* 2G priority:3, 5G priority:2 */
			nvram_unset("dfschinfo");
			//add_led_ctrl_capability(LED_ON_OFF);
		}
#endif
		break;
#endif // RT-AX57Q

#if defined(RPAC51)
	case MODEL_RPAC51:
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, NULL, "eth1", NULL, NULL, 0);
		nvram_set("wl0_vifnames", "wl0.1");
		nvram_set("wl1_vifnames", "wl1.1");

		nvram_set_int("btn_rst_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_red_gpio", 15|GPIO_ACTIVE_LOW);
		//nvram_set_int("led_lan_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_single_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_far_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_near_gpio", 16|GPIO_ACTIVE_LOW);

		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "1");
		nvram_set("wl1_HT_RxStream", "1");
		break;
#endif	/* RP-AC51 */

#if defined(RPAC66)
	case MODEL_RPAC66:
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "eth0", wl_ifaces, NULL, "eth0", NULL, NULL, 0);
		nvram_set("wl0_vifnames", "wl0.1");
		nvram_set("wl1_vifnames", "wl1.1");

		nvram_set_int("btn_rst_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 6);
		nvram_set_int("led_pwr_orange_gpio", 1);
		//nvram_set_int("led_lan_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_blue_gpio", 7);
		nvram_set_int("led_2g_green_gpio", 8);
		nvram_set_int("led_2g_red_gpio", 9);
		nvram_set_int("led_5g_blue_gpio", 14);
		nvram_set_int("led_5g_green_gpio", 15);
		nvram_set_int("led_5g_red_gpio", 16);

		add_rc_support("2.4G 5G update");
		add_rc_support("qcawifi");
		add_rc_support("11AC");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "3");
		nvram_set("wl0_HT_RxStream", "3");
		nvram_set("wl1_HT_TxStream", "3");
		nvram_set("wl1_HT_RxStream", "3");
		break;
#endif	/* RP-AC66 */

#if defined(RTAC58U)
	case MODEL_RTAC58U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", NULL, "eth2", 0);

		if (check_mid("Hydra")) {
			nvram_set_int("btn_rst_gpio", 1|GPIO_ACTIVE_LOW);
			nvram_set_int("btn_wps_gpio", 63|GPIO_ACTIVE_LOW);
			/* Hydra's MAC may not be multible of 4 */
			nvram_set("wl0_vifnames", "wl0.1");
			nvram_set("wl1_vifnames", "wl1.1");
		} else {
			nvram_set_int("btn_rst_gpio", 4|GPIO_ACTIVE_LOW);
			nvram_set_int("btn_wps_gpio", 63|GPIO_ACTIVE_LOW);
			nvram_set_int("led_usb_gpio", 0);
			nvram_set_int("led_usb3_gpio", 0);
			nvram_set_int("led_lan_gpio", 2);
			nvram_set_int("led_wan_gpio", 1);
			nvram_set_int("led_2g_gpio", 58);

			nvram_set_int("led_5g_gpio", 5);

			nvram_set_int("led_wps_gpio", 3);
			nvram_set_int("led_pwr_gpio", 3);
		}

		/* Etron xhci host:
		 *	USB2 bus: 1-1
		 *	USB3 bus: 2-1
		 * Internal ehci host:
		 * 	USB2 bus: 3-1
		 */
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
#endif

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("pwrctrl");
		add_rc_support("nodm");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		if (!strncmp(nvram_safe_get("territory_code"), "CX/01", 5)
		 || !strncmp(nvram_safe_get("territory_code"), "CX/05", 5))
			add_rc_support("nz_isp");
		else if (!strncmp(nvram_safe_get("territory_code"), "SP", 2))
			add_rc_support("spirit");
		if (get_meminfo_item("MemTotal") > 131072) {
			add_rc_support("repeater");
			add_rc_support("psta");
		}
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");
#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth0");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
		}
#endif

		break;
#endif //RTAC58U

#if defined(RT4GAC53U)
	case MODEL_RT4GAC53U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		wan_ifaces[WAN_IFACE_ID] = "eth2";
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", NULL, NULL, 0);

		nvram_set_int("btn_rst_gpio", 63|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 5|GPIO_ACTIVE_LOW);

		nvram_set_int("led_wps_gpio", 0);
		nvram_set_int("led_pwr_gpio", 0);
		nvram_set_int("led_pwr_red_gpio", 1);
		nvram_set_int("led_2g_gpio", 8);
		nvram_set_int("led_5g_gpio", 9);
		nvram_set_int("led_lan_gpio", 10);
		nvram_set_int("led_usb_gpio", 11);
		nvram_set_int("led_usb3_gpio", 11);

		nvram_set_int("led_lteoff_gpio", 50|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig4_gpio", 15);
		nvram_set_int("led_sig3_gpio", 14);
		nvram_set_int("led_sig2_gpio", 13);
		nvram_set_int("led_sig1_gpio", 12);

		nvram_set("ehci_ports", "3-1 1-1");
		nvram_set("ohci_ports", "4-1 1-1");
		nvram_set("usb_buildin", "2");

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("pwrctrl");
		add_rc_support("nodm");
		add_rc_support("nowan");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "2");
		nvram_set("wl1_HT_RxStream", "2");

		break;
#endif //RT4GAC53U

#if defined(RTAC82U)
	case MODEL_RTAC82U:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		set_basic_ifname_vars(wan_ifaces, "eth1", wl_ifaces, "usb", "eth1", "vlan2", "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 40|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 40|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 52|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 54|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 61);
		nvram_set_int("led_wan_red_gpio", 68);

#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_lan1_gpio", 45|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 43|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan3_gpio", 42|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan4_gpio", 49|GPIO_ACTIVE_LOW);
#endif
#if 0
		//nvram_set_int("led_usb_gpio", 0);
		//nvram_set_int("led_usb3_gpio", 0);
		//nvram_set_int("led_lan_gpio", 49);
#endif

		/* Etron xhci host:
		 *	USB2 bus: 1-1
		 *	USB3 bus: 2-1
		 * Internal ehci host:
		 * 	USB2 bus: 3-1
		 */
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "1-1 3-1");
		nvram_set("ohci_ports", "1-1 4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "1-1 4-1");
		}
#endif

		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX1");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("pwrctrl");
		add_rc_support("noitunes");
		add_rc_support("nodm");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "2");
		nvram_set("wl0_HT_RxStream", "2");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "eth1");
#endif

#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth0");
			nvram_set("sta_phy_ifnames", "sta0 sta1");
			nvram_set("sta_ifnames", "sta0 sta1");
			nvram_unset("dfschinfo");
		}
#endif
		break;
#endif //RTAC82U

#if defined(RTAC88N)
	case MODEL_RTAC88N:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");
		wan_ifaces[WAN_IFACE_ID] = "eth0";
		wl_ifaces[WL_2G_BAND] = "ath1";
		wl_ifaces[WL_5G_BAND] = "ath0";
		set_basic_ifname_vars(wan_ifaces, "eth1 eth2", wl_ifaces, "usb", "eth0", "vlan2", "vlan3", 0);

		nvram_set_int("btn_rst_gpio", 54|GPIO_ACTIVE_LOW);	/* SW_SWITCH */
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990)
		nvram_set_int("btn_wps_gpio", 65|GPIO_ACTIVE_LOW);	/* AP148-030, WPS_SWITCH */
#elif defined(RTCONFIG_WIFI_QCA9994_QCA9994)
		nvram_set_int("btn_wps_gpio", 68|GPIO_ACTIVE_LOW);	/* AP161, WPS_SWITCH */
#endif
		nvram_set_int("led_usb_gpio", 7);			/* LED_USB1 */
		nvram_set_int("led_usb3_gpio", 8);			/* LED_USB3 */
		nvram_set_int("led_2g_gpio", 9);			/* STATUS_LED_FAIL/GREEN; MiniCard controls another LED */
		nvram_set_int("led_5g_gpio", 53);			/* STATUS_LED_PASS/RED;   MiniCard controls another LED */
		nvram_set_int("led_pwr_gpio", 26);			/* SATA LED */
		nvram_set_int("led_wps_gpio", 26);			/* SATA LED */

		/* enable bled */
		config_netdev_bled("led_2g_gpio", "ath1");
		config_netdev_bled("led_5g_gpio", "ath0");
		config_usbbus_bled("led_usb_gpio", "1 3");
		config_usbbus_bled("led_usb3_gpio", "2 4");

		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1 4-1");
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "");
		}
		else{
			nvram_set("ehci_ports", "1-1 3-1");
			nvram_set("ohci_ports", "");
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update usbX2");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		break;
#endif	/* RTAC88N */

#if defined(BRTAC828) || defined(RTAC88S)
	case MODEL_BRTAC828:
	case MODEL_RTAC88S:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
		/* BRT-AC828 SR1~SR3, REV 1.00 ~ 1.20 */
		wan0 = "eth2";
		wan1 = "eth3";
		lan_1 = "eth0";
		lan_2 = "eth1";
#else
		/* BRT-AC828 SR4 or above, REV 1.30+ */
		wan0 = "eth0";
		wan1 = "eth3";
		lan_1 = "eth1";
		lan_2 = "eth2";
#endif

		wan_ifaces[WAN_IFACE_ID] = wan0;
		wan_ifaces[WAN2_IFACE_ID] = wan1;
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		if (sw_mode() == SW_MODE_ROUTER && get_wans_dualwan() & WANSCAP_LAN) {
			int wans = get_wans_dualwan();

			if (nvram_match("lan_trunk_type", "2") && !(wans & (WANSCAP_WAN | WANSCAP_WAN2))) {
				int h = nvram_get_int("lan_hash_algorithm");
				const char *str = NULL, *hash_num_to_xmit_str[] = { NULL, "layer2", "layer2+3", NULL, "layer3+4" };
				/* Allow to use two WAN ports as LAN LACP port
				 * if LAN as WAN enabled and none of any WAN port acts as WAN.
				 */
				snprintf(lan_ifs, sizeof(lan_ifs), "%s %s", wan_ifaces[WAN_IFACE_ID], wan_ifaces[WAN2_IFACE_ID]);
				nvram_set("bond0_ifnames", lan_ifs);
				nvram_set("bond0_mode", "802.3ad");
				if (h >= 0 && h < ARRAY_SIZE(hash_num_to_xmit_str))
					str = hash_num_to_xmit_str[h];
				if (!str)
					str = "layer3+4";
				nvram_set("bond0_policy", str);

				snprintf(lan_ifs, sizeof(lan_ifs), "%s %s", "bond0", lan_2);
				set_basic_ifname_vars(wan_ifaces, lan_ifs, wl_ifaces, "usb", NULL, NULL, lan_1, 1);
			} else {
				strlcpy(lan_ifs, lan_2, sizeof(lan_ifs));
				if (wans & WANSCAP_WAN) {
					strlcat(lan_ifs, " ", sizeof(lan_ifs));
					strlcat(lan_ifs, wan1, sizeof(lan_ifs));
				} else if (wans & WANSCAP_WAN2) {
					strlcat(lan_ifs, " ", sizeof(lan_ifs));
					strlcat(lan_ifs, wan0, sizeof(lan_ifs));
				} else {
					strlcat(lan_ifs, " ", sizeof(lan_ifs));
					strlcat(lan_ifs, wan0, sizeof(lan_ifs));
					strlcat(lan_ifs, " ", sizeof(lan_ifs));
					strlcat(lan_ifs, wan1, sizeof(lan_ifs));
				}
				nvram_unset("bond0_ifnames");
				set_basic_ifname_vars(wan_ifaces, lan_ifs, wl_ifaces, "usb", NULL, NULL, lan_1, 1);
			}
		} else {
			snprintf(lan_ifs, sizeof(lan_ifs), "%s %s", lan_1, lan_2);
			nvram_set("bond0_ifnames", lan_ifs);
			set_basic_ifname_vars(wan_ifaces, "bond0", wl_ifaces, "usb", NULL, "vlan2", "vlan3", 0);
		}

		/* Override default number of guest networks.
		 * Number of guest network of each band is 6.
		 */
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3 wl0.4 wl0.5 wl0.6");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3 wl1.4 wl1.5 wl1.6");

#if defined(BRTAC828_SR1)
		nvram_set_int("btn_rst_gpio", 54|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio",  7);
		nvram_set_int("led_usb3_gpio", 8);
		nvram_set_int("led_2g_gpio", 68);
		nvram_set_int("led_5g_gpio", 67);
		nvram_set_int("led_pwr_gpio", 53);
		nvram_set_int("led_wps_gpio", 53);
#else
		nvram_set_int("btn_rst_gpio", 54|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_ejusb1_gpio", 17|GPIO_ACTIVE_LOW | GPIO_EJUSB_PORT(2));
		nvram_set_int("btn_ejusb2_gpio", 24|GPIO_ACTIVE_LOW | GPIO_EJUSB_PORT(1));
		nvram_set_int("led_usb_gpio",  7);
		nvram_set_int("led_usb3_gpio", 15);
		nvram_set_int("led_2g_gpio", 68);
		nvram_set_int("led_5g_gpio", 67);
		nvram_set_int("led_wps_gpio", 53);
		nvram_set_int("led_pwr_gpio", 53);
		nvram_set_int("led_pwr_red_gpio", 57);
		nvram_set_int("led_wan_gpio", 9);
		nvram_set_int("led_wan2_gpio", 6);
		nvram_set_int("led_wan_red_gpio", 56);
		nvram_set_int("led_wan2_red_gpio", 55);	/* RTCONFIG_WANRED_LED && RTCONFIG_WANLEDX2 */
		nvram_set_int("led_failover_gpio", 26);
#if !defined(BRTAC828_SR2)
		nvram_set_int("led_sata_gpio", 25);
#endif
#endif

		/* enable bled */
		config_swports_bled_sleep("led_wan_gpio", 0);
		config_swports_bled_sleep("led_wan2_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ath0");
		config_netdev_bled("led_5g_gpio", "ath1");
		config_usbbus_bled("led_usb_gpio", "1 2");
		config_usbbus_bled("led_usb3_gpio", "3 4");
#if LINUX_KERNEL_VERSION < KERNEL_VERSION(3,14,0)
		config_interrupt_bled("led_sata_gpio", "241");
#else
		config_interrupt_bled("led_sata_gpio", "30");
#endif

		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			int dualwan_if = get_dualwan_by_unit(unit);
			char nvram_ports[20] = "wanports_mask", led_wan_gpio[20] = "led_wan_gpio";

			if (access_point_mode()) {
				if (unit) {
					snprintf(led_wan_gpio, sizeof(led_wan_gpio), "led_wan%d_gpio", unit + 1);
				}
				append_netdev_bled_if(led_wan_gpio, (unit == WAN_UNIT_FIRST)? wan0 : wan1);
			} else {
				if (dualwan_if != WANS_DUALWAN_IF_WAN &&
				    dualwan_if != WANS_DUALWAN_IF_WAN2)
					continue;
				if (unit) {
					snprintf(nvram_ports, sizeof(nvram_ports), "wan%dports_mask", unit);
					snprintf(led_wan_gpio, sizeof(led_wan_gpio), "led_wan%d_gpio", unit + 1);
				}
				append_netdev_bled_if(led_wan_gpio, (dualwan_if == WANS_DUALWAN_IF_WAN)? wan0 : wan1);
			}
		}

		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "4-1 2-1");
#if defined(RTCONFIG_M2_SSD)
			nvram_set("ehci_ports", "3-1 1-1 ata1");
#else
			nvram_set("ehci_ports", "3-1 1-1");
#endif
			nvram_set("ohci_ports", "");
		}
		else{
#if defined(RTCONFIG_M2_SSD)
			nvram_set("ehci_ports", "3-1 1-1 ata1");
#else
			nvram_set("ehci_ports", "3-1 1-1");
#endif
			nvram_set("ohci_ports", "");
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
#if defined(RTCONFIG_M2_SSD)
		add_rc_support("usbX3");
#else
		add_rc_support("usbX2");
#endif
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("11AC");
		add_rc_support("nodm");
		add_rc_support("app");

		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		break;
#endif	/* BRTAC828 || RTAC88S */

#if defined(RTAD7200)
	case MODEL_RTAD7200:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2)
		/* BRT-AC828 SR1~SR3, REV 1.00 ~ 1.20 */
		wan0 = "eth2";
		wan1 = "eth3";
		lan_1 = "eth0";
		lan_2 = "eth1";
#else
		/* BRT-AC828 SR4 or above, REV 1.30+ */
		wan0 = "eth0";
		wan1 = "eth3";
		lan_1 = "eth1";
		lan_2 = "eth2";
#endif

		wan_ifaces[WAN_IFACE_ID] = wan0;
		wan_ifaces[WAN2_IFACE_ID] = wan1;
		wl_ifaces[WL_2G_BAND] = "ath0";
		wl_ifaces[WL_5G_BAND] = "ath1";
		wl_ifaces[WL_5G_2_BAND] = "FAKE5G2";	/* Padding, make sure we can get right wl_unit when enumerating wl_ifnames. */
		wl_ifaces[WL_60G_BAND] = "wlan0";
		if(get_wans_dualwan() & WANSCAP_LAN) {
			int wans = get_wans_dualwan();

			strcpy(lan_ifs, lan_2);
			if (wans & WANSCAP_WAN) {
				strcat(lan_ifs, " ");
				strcat(lan_ifs, wan1);
			} else if (wans & WANSCAP_WAN2) {
				strcat(lan_ifs, " ");
				strcat(lan_ifs, wan0);
			} else {
				strcat(lan_ifs, " ");
				strcat(lan_ifs, wan0);
				strcat(lan_ifs, " ");
				strcat(lan_ifs, wan1);
			}
			nvram_unset("bond0_ifnames");
			set_basic_ifname_vars(wan_ifaces, lan_ifs, wl_ifaces, "usb", NULL, NULL, lan_1, 1);
		} else {
			snprintf(lan_ifs, sizeof(lan_ifs), "%s %s", lan_1, lan_2);
			nvram_set("bond0_ifnames", lan_ifs);
			set_basic_ifname_vars(wan_ifaces, "bond0", wl_ifaces, "usb", NULL, "vlan2", "vlan3", 0);
		}

		/* Override default number of guest networks of 802.11ad Wigig.
		 * Because 802.11ad driver for kernel v3.4 doesn't support multiple SSID.
		 */
		nvram_set("wl3_vifnames", "");

		nvram_set_int("btn_rst_gpio", 54|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_ejusb1_gpio", 17|GPIO_ACTIVE_LOW | GPIO_EJUSB_PORT(2));
		nvram_set_int("btn_ejusb2_gpio", 24|GPIO_ACTIVE_LOW | GPIO_EJUSB_PORT(1));
		nvram_set_int("led_usb_gpio",  7);
		nvram_set_int("led_usb3_gpio", 15);
		nvram_set_int("led_2g_gpio", 68);
		nvram_set_int("led_5g_gpio", 67);
		nvram_set_int("led_wps_gpio", 53);
		nvram_set_int("led_pwr_gpio", 53);
		nvram_set_int("led_pwr_red_gpio", 57);
		nvram_set_int("led_wan_gpio", 9);
		nvram_set_int("led_wan2_gpio", 6);
		nvram_set_int("led_wan_red_gpio", 56);
		nvram_set_int("led_wan2_red_gpio", 55);	/* RTCONFIG_WANRED_LED && RTCONFIG_WANLEDX2 */
		nvram_set_int("led_failover_gpio", 26);
#if defined(RTCONFIG_SATA_LED)
		nvram_set_int("led_sata_gpio", 25);
#else
		nvram_set_int("led_60g_gpio", 25);
#endif

		/* enable bled */
		config_swports_bled_sleep("led_wan_gpio", 0);
		config_swports_bled_sleep("led_wan2_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ath0");
		config_netdev_bled("led_5g_gpio", "ath1");
		config_usbbus_bled("led_usb_gpio", "1 2");
		config_usbbus_bled("led_usb3_gpio", "3 4");
#if defined(RTCONFIG_SATA_LED)
#if LINUX_KERNEL_VERSION < KERNEL_VERSION(3,14,0)
		config_interrupt_bled("led_sata_gpio", "241");
#else
		config_interrupt_bled("led_sata_gpio", "30");
#endif
#else
		config_netdev_bled("led_60g_gpio", "wlan0");
#endif

		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			int dualwan_if = get_dualwan_by_unit(unit);
			char nvram_ports[20] = "wanports_mask", led_wan_gpio[20] = "led_wan_gpio";

			if (access_point_mode()) {
				if (unit) {
					snprintf(led_wan_gpio, sizeof(led_wan_gpio), "led_wan%d_gpio", unit + 1);
				}
				append_netdev_bled_if(led_wan_gpio, (unit == WAN_UNIT_FIRST)? wan0 : wan1);
			} else {
				if (dualwan_if != WANS_DUALWAN_IF_WAN &&
				    dualwan_if != WANS_DUALWAN_IF_WAN2)
					continue;
				if (unit) {
					snprintf(nvram_ports, sizeof(nvram_ports), "wan%dports_mask", unit);
					snprintf(led_wan_gpio, sizeof(led_wan_gpio), "led_wan%d_gpio", unit + 1);
				}
				append_netdev_bled_if(led_wan_gpio, (dualwan_if == WANS_DUALWAN_IF_WAN)? wan0 : wan1);
			}
		}

		if (nvram_match("usb_usb3", "0")) {
			nvram_set("ehci_ports", "3-1 1-1");
			nvram_set("ohci_ports", "");
		} else {
			nvram_set("xhci_ports", "4-1 2-1");
			nvram_set("ehci_ports", "3-1 1-1");
			nvram_set("ohci_ports", "");
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("usbX2");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("nodm");
		add_rc_support("meoVoda");
		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "4");
		nvram_set("wl1_HT_RxStream", "4");
		break;
#endif	/* RTAD7200 */

#if defined(GTAXY16000) || defined(RTAX89U)
	case MODEL_GTAXY16000:
	case MODEL_RTAX89U:
		nvram_set("boardflags", "0"); /* Create VLAN iface. by ourself instead of start_vlan() due to it's executed later than config_switch(). */
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");

		wan0 = "eth3";				/*  1G RJ-45 */
		add_rc_support("11AX ofdma wpa3 10GS_LWAN");
#if defined(RTCONFIG_SPF11_4_QSDK) || defined(RTCONFIG_SPF11_5_QSDK)
		add_rc_support("non_frameburst");
#endif
		add_rc_support("sfp+");
		add_rc_support("sfp+2.5g");			/* qca-ssdk >= SPF11.5 */
#if defined(RTCONFIG_FANCTRL)
		add_rc_support("fanctrl");
#endif
		if (get_soc_version_major() == 1)
			add_rc_support("DL_OFDMA");		/* DL OFDMA only; UL OFDMA is not supported. */
#if defined(RTCONFIG_QCA_LBD)
		add_rc_support("bandstr");
#endif
		if (is_aqr_phy_exist()) {
			/* GT-AXY16000/RT-AX89U */
			wan1 = "eth5";				/* 10G RJ-45 */
			nvram_set_int("led_r10g_gpio", 20);	/* 10G RJ-45 */
			config_netdev_bled("led_r10g_gpio", "eth5");
			add_rc_support("10G_LWAN");
			/* ssdk is initialized after init_nvram(), can't use it here. */
		} else {
			/* RT-AC89U */
			int i, c = 0;
			char word[10], *next;
			char wans_dualwan[7 * WAN_UNIT_MAX], new[7 * WAN_UNIT_MAX] = { 0 };

			wan1 = NULL;
			nvram_set_int("led_usb_gpio",  20);
			config_usbbus_bled("led_usb_gpio", "1 2 3 4");

			/* Remove wan2 from wans_dualwan if AQR PHY absent. */
			strlcpy(wans_dualwan, nvram_safe_get("wans_dualwan"), sizeof(wans_dualwan));
			if (*wans_dualwan == '\0')
				strlcpy(wans_dualwan, nvram_default_get("wans_dualwan"), sizeof(wans_dualwan));

			foreach(word, wans_dualwan, next) {
				if (!strcmp(word, "wan2")) {
					c++;
					continue;
				}
				if (*new != '\0')
					strlcat(new, " ", sizeof(new));
				strlcat(new, word, sizeof(new));
			}
			for (i = 0; i < c; ++i) {
				strlcat(new, " ", sizeof(new));
				strlcat(new, "none", sizeof(new));
			}
			nvram_set("wans_dualwan", new);
			setFanOnOff(0);
		}
		lan_1 = "eth1 eth2";			/* QCA8075 (LAN2,LAN1) */

#if defined(RTCONFIG_LACP)
#if defined(RTCONFIG_BONDING_WAN)
		if (sw_mode == SW_MODE_ROUTER && bond_wan_enabled() && find_word(nvram_safe_get("wans_dualwan"), "wan")) {
			static char lan_ifaces[IFNAMSIZ * 5];
			const char *p;
			int i, b, c = 0;
			uint32_t m, wmask;
			char wans_dualwan[7 * WAN_UNIT_MAX], new[7 * WAN_UNIT_MAX] = { 0 };
			char word[IFNAMSIZ], *next, wan_bifaces[IFNAMSIZ * 5] = { 0 };

			wmask = nums_str_to_u32_mask(nvram_safe_get("wanports_bond"));
			/* If LAN1 or LAN2 is aggregated with WAN, disable LAN aggregation. */
			if ((wmask & (BS_LAN1_PORT_MASK | BS_LAN2_PORT_MASK)) && nvram_match("lacp_enabled", "1"))
				nvram_set("lacp_enabled", "0");

			/* If 10G base-T is aggregated with WAN, remove it from wans_dualwan. */
			if ((wmask & BS_10GR_PORT_MASK) && find_word(nvram_safe_get("wans_dualwan"), "wan2")) {
				strlcpy(wans_dualwan, nvram_safe_get("wans_dualwan"), sizeof(wans_dualwan));
				if (*wans_dualwan == '\0')
					strlcpy(wans_dualwan, nvram_default_get("wans_dualwan"), sizeof(wans_dualwan));
				foreach (word, wans_dualwan, next) {
					if (!strcmp(word, "wan2")) {
						c++;
						continue;
					}
					if (*new != '\0')
						strlcat(new, " ", sizeof(new));
					strlcat(new, word, sizeof(new));
				}
				for (i = 0; i < c; ++i) {
					strlcat(new, " ", sizeof(new));
					strlcat(new, "none", sizeof(new));
				}
				nvram_set("wans_dualwan", new);
			}

			/* List slave interfaces of WAN aggregation. */
			m = wmask;
			while ((b = ffs(m)) > 0) {
				b--;
				if ((p = bs_port_id_to_iface(b)) != NULL) {
					if (*wan_bifaces != '\0')
						strlcat(wan_bifaces, " ", sizeof(wan_bifaces));
					strlcat(wan_bifaces, p, sizeof(wan_bifaces));
				}
				m &= ~(1U << b);
			}
			if (wan1 && (wmask & BS_10GR_PORT_MASK))
				wan1 = NULL;

			if (*wan_bifaces != '\0') {
				nvram_set("bond1_ifnames", wan_bifaces);
				wan0 = "bond1";
			}

			/* If LAN1 and/or LAN2 is not aggregated with WAN, added to LAN. */
			*lan_ifaces = '\0';
			m = (BS_LAN1_PORT_MASK | BS_LAN2_PORT_MASK) & ~wmask;
			while ((b = ffs(m)) > 0) {
				b--;
				if ((p = bs_port_id_to_iface(b)) != NULL) {
					if (*lan_ifaces != '\0')
						strlcat(lan_ifaces, " ", sizeof(lan_ifaces));
					strlcat(lan_ifaces, p, sizeof(lan_ifaces));
				}
				m &= ~(1U << b);
			}
			lan_1 = lan_ifaces;
		} else {
			nvram_unset("bond1_ifnames");
		}
#endif

		if (nvram_match("lacp_enabled", "1")) {
			nvram_set("bond0_ifnames", lan_1);
			lan_1 = "bond0";
		} else {
			nvram_unset("bond0_ifnames");
		}
#endif

		/* If IPTV is enabled and out port is IPQ807X port and another ports are QCA8337 ports,
		 * we have to use VLAN1 to seperate traffic from VLANX.
		 */
		if (sw_mode() != SW_MODE_ROUTER || !iptv_enabled() || !sw_bridge_iptv_different_switches())
			lan_2 = "eth0";			/* QCA8337 + AR8033, no VLAN */
		else
			lan_2 = "eth0.1";		/* QCA8337 + AR8033, VLAN1 */

		wan_ifaces[WAN_IFACE_ID] = wan0;
		wan_ifaces[WAN2_IFACE_ID] = wan1;
		wan_ifaces[SFPP_IFACE_ID] = "eth4";	/* 10G SFP+ */
		wl_ifaces[WL_2G_BAND] = "ath1";
		wl_ifaces[WL_5G_BAND] = "ath0";
#if defined(RTCONFIG_WIGIG)
		wl_ifaces[WL_5G_2_BAND] = "FAKE5G2";	/* Padding, make sure we can get right wl_unit when enumerating wl_ifnames. */
		wl_ifaces[WL_60G_BAND] = "wlan0";
#endif

		snprintf(lan_ifs, sizeof(lan_ifs), "%s %s", lan_1, lan_2);
#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", lan_ifs);
#endif
#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "0")) {
			nvram_set("eth_ifnames", "");	/* for ATE test in AP mode */
		} else {
			if (is_aqr_phy_exist()) {
				nvram_set("eth_ifnames", "eth3 eth5 eth4");		/* WAN, 10G base-T, SFP+ */
				nvram_set("amas_ethif_type", "4 32 64");		/* ETH_TYPE_XXX: WAN, 10G base-T, SFP+ */
				nvram_set("eth_priority", "0 3 1" " 1 2 1" " 2 1 1"); /* WAN priority:3, 10G base-T:2, SFP+:1 */
			} else {
				nvram_set("eth_ifnames", "eth3 eth4");			/* WAN, SFP+ */
				nvram_set("amas_ethif_type", "4 64");			/* ETH_TYPE_XXX: WAN, SFP+ */
				nvram_set("eth_priority", "0 2 1" " 1 1 1");		/* WAN priority:2, SFP+:1 */
			}
			nvram_set("sta_priority", "2 0 5 1" " 5 1 4 1");		/* 2G priority:5, 5G priority:4 */
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
			nvram_set("sta_phy_ifnames", "sta1 sta0");
			nvram_set("sta_ifnames", "sta1 sta0");
			nvram_unset("dfschinfo");
		}
		add_led_ctrl_capability(LED_ON_OFF);
#endif
		set_basic_ifname_vars(wan_ifaces, lan_ifs, wl_ifaces, "usb", NULL, "vlan2", "vlan3", 0);

		nvram_set_int("btn_wps_gpio", 34|GPIO_ACTIVE_LOW);
#if defined(RTCONFIG_TURBO_BTN)
		nvram_set_int("btn_turbo_gpio", 25|GPIO_ACTIVE_LOW);	/* ER2 or above */
		nvram_set_int("led_turbo_gpio", 22);
		config_usbbus_bled("led_turbo_gpio", "");		/* Fake USB BLED */
#elif defined(RTCONFIG_LED_BTN)
		nvram_set_int("btn_led_gpio", 25|GPIO_ACTIVE_LOW);	/* RT-AX89U SR1 ~ ER1 */
#endif
		nvram_set_int("btn_wltog_gpio", 26|GPIO_ACTIVE_LOW);

		if (!strlen(nvram_safe_get("HwId"))) {
#if defined(GTAXY16000)
			nvram_set("HwId", "C");
#elif defined(RTAX89U)
			nvram_set("HwId", "A");
#endif
		}

		if (nvram_match("HwId", "A")) {
			/* PR: A; ER2 or before: not defined. */
			nvram_set_int("led_wan_gpio", 33);
			nvram_set_int("btn_rst_gpio", 58|GPIO_ACTIVE_LOW);

			/* Turn off SFP+ TX disable PIN. */
			gpio_dir(59, GPIO_DIR_OUT);
			set_gpio(59, 0);
		} else {
			/* PCB R4.00: B; or above. */
			nvram_set_int("led_wan_gpio", 47);
			nvram_set_int("btn_rst_gpio", 61|GPIO_ACTIVE_LOW);
		}
		nvram_set_int("led_wan_red_gpio", 44);
		nvram_set_int("led_2g_gpio", 18);
		nvram_set_int("led_5g_gpio", 19);
		nvram_set_int("led_pwr_gpio", 21);
		nvram_set_int("led_wps_gpio", 21);
		nvram_set_int("led_lan_gpio", 35);
		nvram_set_int("led_sfpp_gpio", 36);	/* 10G SFP+ */
		nvram_set_int("pwr_usb_gpio", 31);	/* USB port1 5V */
		nvram_set_int("pwr_usb_gpio2", 30);	/* USB port2 5V */

		/* enable bled */
		config_swports_bled_sleep("led_wan_gpio", 0);
		config_netdev_bled("led_sfpp_gpio", "eth4");
		config_swports_bled_sleep("led_lan_gpio", 0);
		config_netdev_bled("led_2g_gpio", "ath1");
		config_netdev_bled("led_5g_gpio", "ath0");

		if (nvram_match("usb_usb3", "0")) {
			nvram_set("ehci_ports", "3-1 1-1");
			nvram_set("ohci_ports", "");
		} else {
			nvram_set("xhci_ports", "4-1 2-1");
			nvram_set("ehci_ports", "3-1 1-1");
			nvram_set("ohci_ports", "");
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("usbX2");
		add_rc_support("qcawifi");
		add_rc_support("pwrctrl");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("11AC");
		add_rc_support("app");

		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "8");
		nvram_set("wl1_HT_RxStream", "8");
		break;
#endif	/* GTAXY16000 || RTAX89U */

#if defined(GTAX6000N)
	case MODEL_GTAX6000N:
		nvram_set("boardflags", "0x100"); // although it is not used in ralink driver, set for vlan
		//nvram_set("vlan1hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		//nvram_set("vlan2hwname", "et0");  // vlan. used to get "%smacaddr" for compare and find parent interface.
		nvram_set("lan_ifname", "br0");

		wan_ifaces[WAN_IFACE_ID] = "eth0";	/*  1G RJ-45 */
		wan_ifaces[WAN2_IFACE_ID] = "eth5";	/* 10G RJ-45 */
		wan_ifaces[SFPP_IFACE_ID] = "eth4";	/* 10G SFP+ */
		wl_ifaces[WL_2G_BAND] = "ath1";
		wl_ifaces[WL_5G_BAND] = "ath0";
		set_basic_ifname_vars(wan_ifaces, "eth1 eth2 eth3", wl_ifaces, "usb", NULL, "vlan2", "vlan3", 0);

		nvram_set_int("btn_wps_gpio", 34|GPIO_ACTIVE_LOW);

		if (nvram_match("usb_usb3", "0")) {
			nvram_set("ehci_ports", "3-1 1-1");
			nvram_set("ohci_ports", "");
		} else {
			nvram_set("xhci_ports", "4-1 2-1");
			nvram_set("ehci_ports", "3-1 1-1");
			nvram_set("ohci_ports", "");
		}
		nvram_set("ct_max", "300000"); // force

		if (nvram_get("wl_mssid") && nvram_match("wl_mssid", "1"))
			add_rc_support("mssid");
		add_rc_support("2.4G 5G update");
		add_rc_support("usbX2");
		add_rc_support("qcawifi");
		add_rc_support("switchctrl");
		add_rc_support("manual_stb");
		add_rc_support("11AC");
		add_rc_support("11AX");

		// the following values is model dep. so move it from default.c to here
		nvram_set("wl0_HT_TxStream", "4");
		nvram_set("wl0_HT_RxStream", "4");
		nvram_set("wl1_HT_TxStream", "8");
		nvram_set("wl1_HT_RxStream", "8");
		break;
#endif	/* GTAX6000N */

#ifdef CONFIG_BCMWL5
#ifndef RTCONFIG_BCMWL6
#ifdef RTN10U
	case MODEL_RTN10U:
#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			set_lan_phy("vlan0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								sprintf(wan_if, "vlan%d", wan_vid);
							else
								sprintf(wan_if, "eth0");
							add_wan_phy(wan_if);
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan1");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("lan_ifnames", "vlan0 eth1");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan0 eth1");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "");

		nvram_set_int("btn_rst_gpio", 21|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 7);
		nvram_set_int("led_usb_gpio", 8);
		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "2048");
		if (nvram_match("wl0_country_code", "XU")){
			nvram_set("sb/1/eu_edthresh2g", "-69"); //for CE adaptivity certification
		}
		add_rc_support("2.4G mssid usbX1 nomedia small_fw");
		break;
#endif

#if defined(RTN10P) || defined(RTN10D1) || defined(RTN10PV2)
	case MODEL_RTN10P:
	case MODEL_RTN10D1:
	case MODEL_RTN10PV2:
#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			set_lan_phy("vlan0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								sprintf(wan_if, "vlan%d", wan_vid);
							else
								sprintf(wan_if, "eth0");
							add_wan_phy(wan_if);
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan1");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("lan_ifnames", "vlan0 eth1");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan0 eth1");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "");

		nvram_set_int("btn_rst_gpio", 21|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 7);
		nvram_set("sb/1/maxp2ga0", "0x52");
		nvram_set("sb/1/maxp2ga1", "0x52");
		if (!nvram_get("ct_max")) {
			if (model == MODEL_RTN10D1)
				nvram_set_int("ct_max", 1024);
		}
		if (nvram_match("wl0_country_code", "XU")){
			nvram_set("sb/1/eu_edthresh2g", "-69"); //for CE adaptivity certification
		}

		add_rc_support("2.4G mssid");
#ifdef RTCONFIG_KYIVSTAR
		add_rc_support("kyivstar");
#endif
		break;
#endif

	case MODEL_APN12HP:
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3 wl0.4 wl0.5 wl0.6 wl0.7");
		add_rc_support("2.4G mssid");
		nvram_set_int("btn_rst_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set("sb/1/ledbh0", "11");
		nvram_set("sb/1/ledbh1", "11");
		nvram_set("sb/1/ledbh2", "11");
		nvram_set("sb/1/ledbh3", "2");
		nvram_set("sb/1/ledbh5", "11");
		nvram_set("sb/1/ledbh6", "11");
		add_rc_support("pwrctrl");
		nvram_set_int("et_swleds", 0);
		nvram_set("productid", "AP-N12");
		/* go to common N12* init */
		goto case_MODEL_RTN12X;

	case MODEL_RTN12HP:
	case MODEL_RTN12HP_B1:
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		add_rc_support("2.4G mssid update");
		nvram_set_int("btn_rst_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 4|GPIO_ACTIVE_LOW);	/* does HP have it */
		nvram_set_int("sb/1/ledbh5", 7);			/* is active_high? set 7 then */
		if (!nvram_match("hardware_version", "RTN12HP_B1-2.0.1.5"))	//6691 PA
			add_rc_support("pwrctrl");
		if (nvram_match("wl0_country_code", "US"))
		{
			if (nvram_match("wl0_country_rev", "37"))
			{
				nvram_set("wl0_country_rev", "16");
			}

			if (nvram_match("sb/1/regrev", "37"))
			{
				nvram_set("sb/1/regrev", "16");
			}
		}
		if (nvram_match("wl0_country_code", "EU"))
		{
			nvram_set("sb/1/eu_edthresh2g", "-69"); //for CE adaptivity certification
		}
		/* go to common N12* init */
		goto case_MODEL_RTN12X;

	case MODEL_RTN12D1:
	case MODEL_RTN12VP:
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		add_rc_support("2.4G mssid");
		nvram_set_int("btn_rst_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("sb/1/ledbh5", 7);
		nvram_set("sb/1/maxp2ga0", "0x52");
		nvram_set("sb/1/maxp2ga1", "0x52");
		nvram_set("sb/1/cck2gpo", "0x0");
		nvram_set("sb/1/ofdm2gpo0", "0x2000");
		nvram_set("sb/1/ofdm2gpo1", "0x6442");
		nvram_set("sb/1/ofdm2gpo", "0x64422000");
		nvram_set("sb/1/mcs2gpo0", "0x2200");
		nvram_set("sb/1/mcs2gpo1", "0x6644");
		nvram_set("sb/1/mcs2gpo2", "0x2200");
		nvram_set("sb/1/mcs2gpo3", "0x6644");
		nvram_set("sb/1/mcs2gpo4", "0x4422");
		nvram_set("sb/1/mcs2gpo5", "0x8866");
		nvram_set("sb/1/mcs2gpo6", "0x4422");
		nvram_set("sb/1/mcs2gpo7", "0x8866");
		add_rc_support("update");
		if (model == MODEL_RTN12D1) {
			if (nvram_match("regulation_domain", "US") && nvram_match("sb/1/regrev", "0")) {
				nvram_set("sb/1/regrev", "2");
			}
		}
		if(nvram_match("wl0_country_code", "XU")){
			nvram_set("sb/1/eu_edthresh2g", "-69"); //for CE adaptivity certification
		}
		/* go to common N12* init */
		goto case_MODEL_RTN12X;

	case MODEL_RTN12C1:
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		add_rc_support("2.4G mssid");
		nvram_set_int("btn_rst_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("et_swleds", 0x0f);
		nvram_set_int("sb/1/ledbh4", 2);
		nvram_set_int("sb/1/ledbh5", 11);
		nvram_set_int("sb/1/ledbh6", 11);
#ifdef RTCONFIG_SWMODE_SWITCH
		nvram_set_int("btn_swmode1_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_swmode2_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_swmode3_gpio", 8|GPIO_ACTIVE_LOW);
		add_rc_support("swmode_switch");
		nvram_set_int("swmode_switch", 1);
#endif
		/* go to common N12* init */
		goto case_MODEL_RTN12X;

	case MODEL_RTN12B1:
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		add_rc_support("2.4G mssid");
		nvram_set_int("btn_rst_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("sb/1/ledbh5", 2);
#ifdef RTCONFIG_SWMODE_SWITCH
		nvram_set_int("btn_swmode1_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_swmode2_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_swmode3_gpio", 8|GPIO_ACTIVE_LOW);
		add_rc_support("swmode_switch");
		nvram_set_int("swmode_switch", 1);
#endif
		/* go to common N12* init */
		goto case_MODEL_RTN12X;

	case MODEL_RTN12:
		nvram_set_int("btn_rst_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set("boardflags", "0x310");
		/* fall through to common N12* init */
	case_MODEL_RTN12X:
		//nvram_set("lan_ifnames", "vlan0 eth1");
		//nvram_set("wan_ifnames", "eth0");
		//nvram_set("wandevs", "et0");
		nvram_set("wl_ifnames", "eth1");

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan1 vlan2");
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								sprintf(wan_if, "vlan%d", wan_vid);
							else
								sprintf(wan_if, "eth0");
							add_wan_phy(wan_if);
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan1");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan0 eth1");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan0 eth1");
		nvram_set("wan_ifnames", "eth0");
		nvram_set("wandevs", "et0");
#endif
		break;

#ifdef RTN14UHP
	case MODEL_RTN14UHP:
		nvram_unset("odmpid");
		nvram_unset("et_dispatch_mode");
		nvram_unset("wl_dispatch_mode");
		add_rc_support("pwrctrl");
#ifdef RTCONFIG_LAN4WAN_LED
		if (nvram_match("odmpid", "TW")) {
			add_rc_support("lanwan_led2");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		set_lan_phy("vlan0");

		if (!(get_wans_dualwan()&WANSCAP_2G))
			add_lan_phy("eth1");

		if (nvram_get("wans_dualwan")) {
			set_wan_phy("");
			for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
				if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
					if (get_wans_dualwan()&WANSCAP_WAN)
						add_wan_phy("vlan2");
					else
						add_wan_phy("eth0");
				}
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
					add_wan_phy("eth1");
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
					if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
						int wan_vid = nvram_get_int("switch_wan0tagid");
						if (wan_vid)
							sprintf(wan_if, "vlan%d", wan_vid);
						else
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_wans_dualwan()&WANSCAP_LAN)
						add_wan_phy("vlan1");
					else
						add_wan_phy("eth0");
				}
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
					add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
					add_wan_phy("usb2");
#endif
			}
		}
		else
			nvram_set("wan_ifnames", "eth0 usb");
#else
		nvram_set("lan_ifnames", "vlan0 eth1");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "");

		nvram_set_int("btn_rst_gpio", 24|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 25|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 6);
#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_wan_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan1_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan3_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan4_gpio", 3|GPIO_ACTIVE_LOW);
#endif
		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "2048");
		add_rc_support("2.4G mssid media usbX1 update");
		break;
#endif

#ifdef RTN15U
	case MODEL_RTN15U:
#ifdef RTCONFIG_DUALWAN
		set_lan_phy("vlan1");

		if (!(get_wans_dualwan()&WANSCAP_2G))
			add_lan_phy("eth1");

		if (nvram_get("wans_dualwan")) {
			set_wan_phy("");
			for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
				if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
					if (get_wans_dualwan()&WANSCAP_WAN)
						add_wan_phy("vlan3");
					else
						add_wan_phy("eth0");
				}
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
					add_wan_phy("eth1");
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
					if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
						int wan_vid = nvram_get_int("switch_wan0tagid");
						if (wan_vid)
							sprintf(wan_if, "vlan%d", wan_vid);
						else
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_wans_dualwan()&WANSCAP_LAN)
						add_wan_phy("vlan2");
					else
						add_wan_phy("eth0");
				}
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
					add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
					add_wan_phy("usb2");
#endif
			}
		}
		else
			nvram_set("wan_ifnames", "eth0 usb");
#else
		nvram_set("lan_ifnames", "vlan1 eth1");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "");

		nvram_set_int("btn_rst_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 9);
		nvram_set_int("led_lan_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("sb/1/ledbh0", 0x82);
		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "15000");
		add_rc_support("2.4G mssid usbX1 nomedia");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		break;
#endif

#ifdef RTN16
	case MODEL_RTN16:
#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan2 vlan3");
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan3");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								sprintf(wan_if, "vlan%d", wan_vid);
							else
								sprintf(wan_if, "eth0");
							add_wan_phy(wan_if);
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan1 eth1");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1");
		nvram_set("wl0_vifnames", "wl0.1");
		nvram_set("wl1_vifnames", "");

		nvram_set("wl0_vifnames", "wl0.1");	/* Only one gueset network? */

		nvram_set_int("btn_rst_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set("ehci_ports", "1-2 1-1");
		nvram_set("ohci_ports", "2-2 2-1");
		nvram_set("boardflags", "0x310");
		nvram_set("sb/1/boardflags", "0x310");
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("2.4G update usbX2 mssid");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		break;
#endif

#ifdef RTN53
	case MODEL_RTN53:
		nvram_set("lan_ifnames", "vlan0 eth1 eth2");
		nvram_set("wan_ifnames", "eth0");
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set_int("btn_rst_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 1|GPIO_ACTIVE_LOW);
		/* change lan interface to vlan0 */
		nvram_set("vlan0hwname", "et0");
		nvram_set("landevs", "vlan0 wl0 wl1");
		nvram_unset("vlan2ports");
		nvram_unset("vlan2hwname");
		/* end */
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "2048");

		add_rc_support("2.4G 5G mssid no5gmssid small_fw");
#ifdef RTCONFIG_WLAN_LED
		add_rc_support("led_2g");
		nvram_set("led_5g", "1");
#endif
		break;
#endif
#endif	// RTCONFIG_BCMWL6

#ifdef RTN18U
	case MODEL_RTN18U:
		nvram_set("vlan1hwname", "et0");
		nvram_set("landevs", "vlan1 wl0");

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan2 vlan3");
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan3");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								sprintf(wan_if, "vlan%d", wan_vid);
							else
								sprintf(wan_if, "eth0");
							add_wan_phy(wan_if);
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan1 eth1");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");

#ifdef RTCONFIG_USBRESET
		nvram_set_int("pwr_usb_gpio", 13|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("pwr_usb_gpio", 13);
#endif
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 7|GPIO_ACTIVE_LOW);

		if (nvram_match("bl_version", "3.0.0.7")) {
			nvram_set_int("led_usb_gpio", 14|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_gpio", 3|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan_gpio", 6|GPIO_ACTIVE_LOW);
		}
		else if (nvram_match("bl_version", "1.0.0.0")) {
			nvram_set_int("led_usb_gpio", 3|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_gpio", 6|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan_gpio", 9|GPIO_ACTIVE_LOW);
			nvram_set_int("led_2g_gpio", 10|GPIO_ACTIVE_LOW);
		}
		else{
			nvram_set_int("led_usb_gpio", 3|GPIO_ACTIVE_LOW);
			nvram_set_int("led_usb3_gpio", 14|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_gpio", 6|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan_gpio", 9|GPIO_ACTIVE_LOW);
		}

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1");
		nvram_set("ehci_ports", "2-1 2-2");
		nvram_set("ohci_ports", "3-1 3-2");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "1-1");
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G update usbX2");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");

		if (nvram_match("bl_version", "1.0.0.0"))
			add_rc_support("led_2g");

		break;
#endif

#ifdef DSL_AC68U
	case MODEL_DSLAC68U:
		nvram_set("vlan1hwname", "et0");
		nvram_set("vlan2hwname", "et0");
		nvram_set("landevs", "vlan1 wl0 wl1");
#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			//Andy Chiu, 2015/09/11
			int ewan_dot1q = nvram_get_int("ewan_dot1q");	//Andy Chiu, 2015/09/08
			int ewan_vid = nvram_get_int("ewan_vid");		//Andy Chiu, 2015/09/08
			char vlanif[16];
			if(ewan_dot1q && ewan_vid> 0 && !(ewan_vid >= DSL_WAN_VID && ewan_vid<= (DSL_WAN_VID + 7)))
				sprintf(vlanif, "vlan%d", ewan_vid);
			else
				strcpy(vlanif, "vlan4");

			if (get_wans_dualwan()&WANSCAP_DSL && get_wans_dualwan()&WANSCAP_LAN)
			{
				//Andy Chiu, 2015/09/11
				char buf[64];
				sprintf(buf, "%s %s", DSL_WAN_VIF, vlanif);
				nvram_set("wandevs", buf);
			}
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth2");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN)
						add_wan_phy(vlanif);	//Andy Chiu, 2015/09/11
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_DSL)
						add_wan_phy(DSL_WAN_VIF);
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
			{
				char buf[64];
				snprintf(buf, sizeof(buf), "%s usb", DSL_WAN_VIF);
				nvram_set("wan_ifnames", buf);
			}
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan1 eth1 eth2");
			nvram_set("wan_ifnames", DSL_WAN_VIF);
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifnames", DSL_WAN_VIF);
#endif
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
#ifdef RTCONFIG_USBRESET
		nvram_set_int("pwr_usb_gpio", 9|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("pwr_usb_gpio", 9);
#endif
		nvram_set_int("led_wan_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 6|GPIO_ACTIVE_LOW);	// 4360's fake led 5g
		nvram_set_int("led_usb3_gpio", 14|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 15|GPIO_ACTIVE_LOW);
#endif

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1");
		nvram_set("ehci_ports", "2-1");
		nvram_set("ohci_ports", "3-1");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "1-1");
			nvram_set("ehci_ports", "2-1");
			nvram_set("ohci_ports", "3-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("dsl");
		add_rc_support("vdsl");
		add_rc_support("spectrum");

		if (	nvram_match("wl1_country_code", "EU") &&
				nvram_match("wl1_country_rev", "13")) {
			nvram_set("0:ed_thresh2g", "-70");
			nvram_set("0:ed_thresh5g", "-70");
			nvram_set("1:ed_thresh2g", "-72");
			nvram_set("1:ed_thresh5g", "-72");
		}

#if defined(RTCONFIG_AMAS) || defined(RTCONFIG_CFGSYNC)
		nvram_set("wired_ifnames", "vlan1");
#endif
#if defined(RTCONFIG_AMAS)
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth1 eth2");
		}
		nvram_set("sta_ifnames", "eth1 eth2");
		nvram_set("eth_ifnames", "");
#endif
		break;
#endif

#ifdef RTAC3200
	case MODEL_RTAC3200:
		nvram_set("vlan1hwname", "et0");
		nvram_set("landevs", "vlan1 wl0 wl1 wl2");

#ifdef RTCONFIG_GMAC3
		if (nvram_match("gmac3_enable", "1"))
			hw_name = "et2";
		else {
			nvram_set("fwd_wlandevs", "");
			nvram_set("fwddevs", "");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
			nvram_set("wandevs", "vlan2 vlan3");
#ifdef RTCONFIG_GMAC3
		else
			nvram_set("wandevs", hw_name);
#endif
		set_lan_phy("vlan1");

		if (!(get_wans_dualwan()&WANSCAP_2G))
			add_lan_phy("eth2");
		if (!(get_wans_dualwan()&WANSCAP_5G)) {
			add_lan_phy("eth1");
			add_lan_phy("eth3");
		}

		if (nvram_get("wans_dualwan")) {
			set_wan_phy("");
			for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
				if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
					if (get_wans_dualwan()&WANSCAP_WAN)
						add_wan_phy("vlan3");
					else
						add_wan_phy(the_wan_phy());
				}
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
					add_wan_phy("eth2");
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G) {
					add_wan_phy("eth1");
					add_wan_phy("eth3");
				}
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
					if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
						int wan_vid = nvram_get_int("switch_wan0tagid");
						if (wan_vid) {
							sprintf(wan_if, "vlan%d", wan_vid);
							add_wan_phy(wan_if);
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_wans_dualwan()&WANSCAP_LAN)
						add_wan_phy("vlan2");
					else
						add_wan_phy(the_wan_phy());
				}
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
					add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
				else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
					add_wan_phy("usb2");
#endif
			}
		}
		else
			nvram_set("wan_ifnames", "eth0 usb");
#else
		nvram_set("lan_ifnames", "vlan1 eth2 eth1 eth3");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth2 eth1 eth3");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_USBRESET
		nvram_set_int("pwr_usb_gpio", 9|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("pwr_usb_gpio", 9);
#endif
		nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 5);
		nvram_set_int("btn_wps_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 11|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 4|GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 15|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("rst_hw_gpio", 17|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1");
		nvram_set("ehci_ports", "2-1 2-2");
		nvram_set("ohci_ports", "3-1 3-2");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "1-1");
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");

		break;
#endif

#ifdef RTAC68U
	case MODEL_RTAC68U:
		check_cfe_ac68u();
		nvram_set("vlan1hwname", "et0");
		nvram_set("landevs", "vlan1 wl0 wl1");

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan2 vlan3");
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth2");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan3");
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								sprintf(wan_if, "vlan%d", wan_vid);
								add_wan_phy(wan_if);
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan1 eth1 eth2");
			nvram_set("wan_ifnames", the_wan_phy());
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
#if defined(RTCONFIG_AMAS)
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		} else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth1 eth2");
		}
		nvram_set("eth_ifnames", "vlan2");
		nvram_set("sta_ifnames", "eth1 eth2");	
		if (nvram_get_int("re_mode") == 1) {
			nvram_set("lan_ifnames", "vlan1 eth1 eth2");
			if (enable_ETH_U(0) == 1)
				add_lan_phy(nvram_safe_get("eth_ifnames"));
		}
#endif
#ifdef RT4GAC68U
		nvram_set_int("led_lan_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_3g_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lte_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig1_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig2_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig3_gpio", 8|GPIO_ACTIVE_LOW);

		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
#else // RT4GAC68U
		if (!is_ac66u_v2_series()) {
			nvram_set_int("led_5g_gpio", 6|GPIO_ACTIVE_LOW);// 4360's fake led 5g
#ifdef RTCONFIG_LED_BTN
			nvram_set_int("btn_led_gpio", 5);		// active high
#endif
#ifdef RTCONFIG_LOGO_LED
			if (!is_ac68u_v3_series())
			nvram_set_int("led_logo_gpio", 4|GPIO_ACTIVE_LOW);
#endif
			nvram_set_int("led_usb_gpio", 0|GPIO_ACTIVE_LOW);

			nvram_set_int("led_wps_gpio", 3|GPIO_ACTIVE_LOW);
		}
		else {
			nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		}
#endif  // RT4GAC68U
		nvram_set_int("pwr_usb_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 11|GPIO_ACTIVE_LOW);
		if (!is_ac66u_v2_series()) {
			nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
			nvram_set_int("led_usb3_gpio", 14|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIFI_TOG_BTN
			nvram_set_int("btn_wltog_gpio", 15|GPIO_ACTIVE_LOW);
#endif
			if (is_ac68u_v3_series())
				nvram_set_int("led_wan_gpio", 4);
		} else {
			nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_gpio", 5);
		}

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1");
#ifdef RT4GAC68U
		if(nvram_get_int("usb_gobi") == 2){
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
		}
		else{
			nvram_set("ehci_ports", "2-1 2-2.2 2-2.1");
			nvram_set("ohci_ports", "3-1 3-2.2 3-2.1");
		}
#else
		nvram_set("ehci_ports", "2-1 2-2");
		nvram_set("ohci_ports", "3-1 3-2");
#endif
#else // RTCONFIG_XHCIMODE
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "1-1");
#ifdef RT4GAC68U
#if 0 // old layout. there is the USB 2.0 port.
			if(nvram_get_int("usb_gobi") == 2){
				nvram_set("ehci_ports", "2-1 2-2");
				nvram_set("ohci_ports", "3-1 3-2");
			}
			else{
				nvram_set("ehci_ports", "2-1 2-2.2 2-2.1");
				nvram_set("ohci_ports", "3-1 3-2.2 3-2.1");
			}
#else
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
#endif
#else
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
#endif
		}
		else{
			nvram_unset("xhci_ports");
#ifdef RT4GAC68U
#if 0 // old layout. there is the USB 2.0 port.
			nvram_set("ehci_ports", "1-1 1-2.2 1-2.1");
			nvram_set("ohci_ports", "2-1 2-2.2 2-2.1");
#else
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
#endif
#else
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
#endif
		}
#endif // RTCONFIG_XHCIMODE

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
#ifdef RT4GAC68U
		add_rc_support("usbX1");
#else
		if (hw_usb_cap())
		add_rc_support("usbX2");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
#ifndef RT4GAC68U
		if (hw_usb_cap())
		add_rc_support("app");
#endif
		if (!hw_vht_cap())
			add_rc_support("no_vht");

		break;
#endif

#if defined(RTAC1200G) || defined(RTAC1200GP)
	case MODEL_RTAC1200G:
	case MODEL_RTAC1200GP:
		nvram_unset("et1macaddr");
		nvram_unset("et1phyaddr");
		nvram_set("wandevs", "et0");
		nvram_set("vlan1hwname", "et0");
		nvram_set("vlan2hwname", "et0");
		nvram_set("landevs", "vlan1 wl0 wl1");
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifnames", "eth0");
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set_int("btn_rst_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 10|GPIO_ACTIVE_LOW);
//		nvram_set_int("led_5g_gpio", 11);	// active high
		nvram_set_int("led_usb_gpio", 15|GPIO_ACTIVE_LOW);

		nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "2-1");
//		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "250000");
		add_rc_support("mssid 2.4G 5G usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("WIFI_LOGO");
		add_rc_support("update");
		if (model == MODEL_RTAC1200GP){
			add_rc_support("meoVoda");
			add_rc_support("movistarTriple");
		}
		if (model == MODEL_RTAC1200G) {
			add_rc_support("noitunes");
			add_rc_support("nodm");
			add_rc_support("noaidisk");
		}

		break;
#endif

#if defined(GTAC5300)
	case MODEL_GTAC5300:
		model_patch();
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth6 eth7 eth8");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#if defined(RTCONFIG_AMAS)
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
#endif
		else
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7 eth8");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth6 eth7 eth8");		
		nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif

		nvram_set("1:ledbh9", "0x7");
		nvram_set("2:ledbh9", "0x7");
		nvram_set("3:ledbh9", "0x7");

		nvram_set_int("led_pwr_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);
		nvram_set_int("led_lan_gpio", 25);
		nvram_set_int("led_wps_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 26);

		nvram_set_int("btn_wltog_gpio", 28|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 30|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 31|GPIO_ACTIVE_LOW);
#endif

		if(usb_usb3 == 1)
			nvram_set("xhci_ports", "2-2 2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-2 3-1");
		nvram_set("ohci_ports", "4-2 4-1");

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth4 eth3 eth5");
				else if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth4 eth3 eth5");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth2 eth1 eth3 eth5");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth2 eth1 eth4 eth5");
			}
			else
				set_lan_phy("eth2 eth1 eth4 eth3 eth5");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth7");
				add_lan_phy("eth8");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						sprintf(wan_if, "eth%d", nvram_get_int("wans_lanport"));
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
//		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");

		break;
#endif

#if defined(RTAC86U) || defined(GTAC2900)
	case MODEL_RTAC86U:
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "eth0");
			nvram_set("wan_ifname", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#if defined(RTCONFIG_AMAS)
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
#endif
		else
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");	
#endif

#ifdef GTAC2900
		nvram_set("0:ledbh9", "0x1");
		nvram_set("1:ledbh9", "0x1");
#else
		nvram_set("0:ledbh9", "0x7");
		nvram_set("1:ledbh9", "0x7");
#endif

#ifndef GTAC2900
		nvram_set_int("led_pwr_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);
#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_lan1_gpio", 16);
		nvram_set_int("led_lan2_gpio", 17);
		nvram_set_int("led_lan3_gpio", 18);
		nvram_set_int("led_lan4_gpio", 19);
#endif
		nvram_set_int("led_usb_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb3_gpio", 12|GPIO_ACTIVE_LOW);
#endif

		nvram_set_int("btn_wltog_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 23|GPIO_ACTIVE_LOW);
#ifdef GTAC2900
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 14|GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_TURBO_BTN
		nvram_set_int("btn_turbo_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_turbo_gpio", 17|GPIO_ACTIVE_LOW);
#endif
#else
		nvram_set_int("btn_led_gpio", 14|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("pwr_usb_gpio", 3|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-2");
		nvram_set("ehci_ports", "3-2 3-1");
		nvram_set("ohci_ports", "4-2 4-1");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-2");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth1 eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth2 eth4");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth3 eth4");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth2 eth3 eth4");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								sprintf(wan_if, "eth0.v0");
								add_wan_phy(wan_if);
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("meoVoda");
		add_rc_support("app");
		if (!strncmp(nvram_safe_get("territory_code"), "CX", 2)) {
			add_rc_support("nz_isp");
			nvram_set("wifi_psk", cfe_nvram_safe_get_raw("secret_code"));
		}
		break;
#endif

#if defined(GTAX11000)
	case MODEL_GTAX11000:
		nvram_unset("dhd0_rnr_flowring_physize");
		nvram_unset("dhd1_rnr_flowring_physize");
		nvram_unset("dhd2_rnr_flowring_physize");
		update_43684_tempthresh();
		update_cfe_basemac();
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth6 eth7 eth8");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7 eth8");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0 eth5"); // 1G, 2.5G
		nvram_set("amas_ethif_type", "4 8"); // 1G, 2.5G
		nvram_set("eth_priority", "0 1 1 1 2 1"); // eth0: 1G(idx:0,prio:1,used:1) > eth5: 2.5G(idx:1,prio:2,used:1)
		nvram_set("sta_priority", "2 0 5 1 5 1 4 0 5 2 3 1"); // 2.4G:(prio:5), 5G-1:(prio:4), 5G-2:(prio:3)
		nvram_set("sta_ifnames", "eth6 eth7 eth8");
		if(nvram_get_int("re_mode") == 1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
		else
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif

		nvram_set("1:ledbh15", "0x7");
		nvram_set("2:ledbh15", "0x7");
		nvram_set("3:ledbh15", "0x7");

		nvram_set_int("led_pwr_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);
		nvram_set_int("led_lan_gpio", 25);
		nvram_set_int("led_wps_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 26);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 27|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_wps_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 4|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_TURBO_BTN
		nvram_set_int("btn_turbo_gpio", 31|GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_LOGO_LED
		nvram_set_int("led_logo_gpio", 15|GPIO_ACTIVE_LOW);
#endif

#if 0
		if(usb_usb3 == 1)
			nvram_set("xhci_ports", "2-2 2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-2 3-1");
		nvram_set("ohci_ports", "4-2 4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-2 2-1");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-2 1-1");
			nvram_set("ohci_ports", "2-2 2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth1 eth2 eth3 eth5");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth2 eth4 eth5");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth2 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "5"))
					set_lan_phy("eth1 eth2 eth3 eth4");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4 eth5");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth7");
				add_lan_phy("eth8");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				char prefix[8];
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "5"))
							sprintf(wan_if, "eth5");
						add_wan_phy(wan_if);

					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
#ifdef RTCONFIG_EXTPHY_BCM84880
		add_rc_support("2p5G_LWAN");
#endif
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif


		break;
#endif

#if defined(RTAX88U)
	case MODEL_RTAX88U:
		nvram_set("fast_restart", "1");
		nvram_unset("dhd0_rnr_flowring_physize");
		nvram_unset("dhd1_rnr_flowring_physize");
		nvram_unset("dhd2_rnr_flowring_physize");
		update_rf_para();
		update_43684_tempthresh();
		update_cfe_basemac();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth6 eth7");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth6 eth7");
		nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif

		nvram_set("1:ledbh15", "0x7");
		nvram_set("2:ledbh15", "0x7");

		nvram_set_int("led_usb_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb3_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);
		nvram_set_int("led_lan_gpio", 25);
		nvram_set_int("led_wps_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 26);

		nvram_set_int("btn_wltog_gpio", 27|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_led_gpio", 31|GPIO_ACTIVE_LOW);
#if 0
#ifdef RTCONFIG_XHCIMODE
                nvram_set("xhci_ports", "2-1 2-2");
                nvram_set("ehci_ports", "3-1 3-2");
                nvram_set("ohci_ports", "4-1 4-2");
#else
                if (usb_usb3 == 1)
                        nvram_set("xhci_ports", "2-1 2-2");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1 3-2");
		nvram_set("ohci_ports", "4-1 4-2");
#endif
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1 2-2");
			nvram_set("ehci_ports", "3-1 3-2");
			nvram_set("ohci_ports", "4-1 4-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth1 eth2 eth3 eth5");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth2 eth4 eth5");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth2 eth3 eth4 eth5");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4 eth5");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth7");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		if (!strncmp(nvram_safe_get("territory_code"), "CX", 2)) {
			add_rc_support("nz_isp");
			nvram_set("wifi_psk", cfe_nvram_safe_get_raw("secret_code"));
		}

		break;
#endif

#if defined(BC109) || defined(EBG19) || defined(EBG15) || defined(EBP15)
        case MODEL_BC109:
                nvram_set("fast_restart", "1");
                //nvram_unset("dhd0_rnr_flowring_physize");
                //nvram_unset("dhd1_rnr_flowring_physize");
                //nvram_unset("dhd2_rnr_flowring_physize");
                //update_rf_para();
                //update_43684_tempthresh();
                //update_cfe_basemac();
                nvram_set("lan_ifname", "br0");
                if (is_router_mode()) {
                        nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5");
                        nvram_set("wan_ifname", "eth0");
                        nvram_set("wan_ifnames", "eth0");
                } else {
                        nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5");
                        nvram_set("wan_ifnames", "");
                        nvram_set("wan_ifname", "");
                }
                nvram_set("wl_ifnames", "");
                //nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
                //nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
		nvram_set("sta_phy_ifnames", "");

                if (nvram_get_int("re_mode") == 1) {
                        nvram_set("wait_band", "0");
                        nvram_set("wait_wifi", "0");
                }

                nvram_set("eth_ifnames", "eth0");
                nvram_set("sta_ifnames", "");
                nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif
                //nvram_set("1:ledbh15", "0x7");
                //nvram_set("2:ledbh15", "0x7");

                nvram_set_int("led_53134_int_gpio", 12);
		/* hw2
                nvram_set_int("led_spd_g0_gpio", 0|GPIO_ACTIVE_LOW);
                nvram_set_int("led_spd_g1_gpio", 2|GPIO_ACTIVE_LOW);
                nvram_set_int("led_spd_g2_gpio", 4|GPIO_ACTIVE_LOW);
                nvram_set_int("led_spd_g3_gpio", 6);
                nvram_set_int("led_spd_g4_gpio", 10|GPIO_ACTIVE_LOW);
		*/
                nvram_set_int("led_spd_g0_gpio", 1|GPIO_ACTIVE_LOW);
                nvram_set_int("led_spd_g1_gpio", 3|GPIO_ACTIVE_LOW);
                nvram_set_int("led_spd_g2_gpio", 5|GPIO_ACTIVE_LOW);
                nvram_set_int("led_spd_g3_gpio", 7);
                nvram_set_int("led_spd_g4_gpio", 11|GPIO_ACTIVE_LOW);
                nvram_set_int("led_53134_rst_gpio", 14|GPIO_ACTIVE_LOW);
                nvram_set_int("led_ar3012_dis_gpio", 15|GPIO_ACTIVE_LOW);
                nvram_set_int("led_act_g0_gpio", 16|GPIO_ACTIVE_LOW);
                nvram_set_int("led_act_g1_gpio", 17|GPIO_ACTIVE_LOW);
                nvram_set_int("led_act_g2_gpio", 18|GPIO_ACTIVE_LOW);
                nvram_set_int("led_act_g3_gpio", 19|GPIO_ACTIVE_LOW);
                nvram_set_int("led_act_g4_gpio", 21|GPIO_ACTIVE_LOW);
                nvram_set_int("led_ar3012_rst_gpio", 25|GPIO_ACTIVE_LOW);
                //nvram_set_int("pwr_usb_gpio", 67);

                nvram_set_int("btn_rst_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_led_gpio", 0xff);	// for sw-hw-auth

                if(usb_usb3 == 1){
                        nvram_set("xhci_ports", "2-1 2-2");
                        nvram_set("ehci_ports", "3-1 3-2");
                        nvram_set("ohci_ports", "4-1 4-2");
                }
                else{
                        nvram_unset("xhci_ports");
                        nvram_set("ehci_ports", "1-1 1-2");
                        nvram_set("ohci_ports", "2-1 2-2");
                }

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth1 eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth2 eth4");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth3 eth4");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth2 eth3 eth4");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4");
/*
			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
			}
*/
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								sprintf(wan_if, "eth0.v0");
								add_wan_phy(wan_if);
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif  // RTCONFIG_DUALWAN

                if (!nvram_get("ct_max"))
                        nvram_set("ct_max", "300000");
                add_rc_support("update usbX1");
                add_rc_support("switchctrl"); // broadcom: for jumbo frame only
                add_rc_support("manual_stb");
                add_rc_support("pwrctrl");
                add_rc_support("nandflash");
                add_rc_support("movistarTriple");
                add_rc_support("app");

                break;
#endif

	case MODEL_EBG19:
                nvram_set("fast_restart", "1");
                nvram_set("lan_ifname", "br0");
                if (is_router_mode()) {
                        nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5");
                        nvram_set("wan_ifname", "eth0");
                        nvram_set("wan_ifnames", "eth0");
                } else {
                        nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5");
                        nvram_set("wan_ifnames", "");
                        nvram_set("wan_ifname", "");
                }
                nvram_set("wl_ifnames", "");

#ifdef RTCONFIG_AMAS
		nvram_set("sta_phy_ifnames", "");

                if (nvram_get_int("re_mode") == 1) {
                        nvram_set("wait_band", "0");
                        nvram_set("wait_wifi", "0");
                }

                nvram_set("eth_ifnames", "eth0");
                nvram_set("sta_ifnames", "");
                nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif
		if(model == MODEL_BC109) {
                  nvram_set_int("led_53134_int_gpio", 0|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_spd_g0_gpio", 1|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_spd_g1_gpio", 3|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_spd_g2_gpio", 5|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_spd_g3_gpio", 7);
                  nvram_set_int("led_spd_g4_gpio", 11|GPIO_ACTIVE_LOW);
		}
                nvram_set_int("led_usb_gpio", 12|GPIO_ACTIVE_LOW);
		if(model == MODEL_BC109) {
                  nvram_set_int("led_53134_rst_gpio", 14|GPIO_ACTIVE_LOW);
		}
                nvram_set_int("led_ar3012_dis_gpio", 15|GPIO_ACTIVE_LOW);
		if(model == MODEL_BC109) {
                  nvram_set_int("led_act_g0_gpio", 16|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_act_g1_gpio", 17|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_act_g2_gpio", 18|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_act_g3_gpio", 19|GPIO_ACTIVE_LOW);
                  nvram_set_int("led_act_g4_gpio", 21|GPIO_ACTIVE_LOW);
		}
                nvram_set_int("led_pwr_gpio", 22|GPIO_ACTIVE_LOW);
                nvram_set_int("led_ethall_gpio", 24|GPIO_ACTIVE_LOW);
                nvram_set_int("led_ar3012_rst_gpio", 25|GPIO_ACTIVE_LOW);

                nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);       // use ax88u's as pre-test, no wps btn.
                nvram_set_int("btn_rst_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_led_gpio", 0xff);	// for sw-hw-auth

                if(usb_usb3 == 1){
                        nvram_set("xhci_ports", "2-1 2-2");
                        nvram_set("ehci_ports", "3-1 3-2");
                        nvram_set("ohci_ports", "4-1 4-2");
                }
                else{
                        nvram_unset("xhci_ports");
                        nvram_set("ehci_ports", "1-1 1-2");
                        nvram_set("ohci_ports", "2-1 2-2");
                }

#ifdef RTCONFIG_DUALWAN
                if (is_router_mode()) {
                        if (get_wans_dualwan()&WANSCAP_LAN) {
                                if (nvram_match("wans_lanport", "1"))
                                        set_lan_phy("eth2 eth3");
                                else if (nvram_match("wans_lanport", "2"))
                                        set_lan_phy("eth1 eth3");
                                else if (nvram_match("wans_lanport", "3"))
                                        set_lan_phy("eth1 eth2");
                        }
                        else
                                set_lan_phy("eth1 eth2 eth3");

                        if (!(get_wans_dualwan()&WANSCAP_2G))
                                add_lan_phy("eth4");
                        if (!(get_wans_dualwan()&WANSCAP_5G)) {
                                add_lan_phy("eth5");
                                //add_lan_phy("eth6");
                        }
                        if (nvram_get("wans_dualwan")) {
                                set_wan_phy("");
                                for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
                                        if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
                                                if (nvram_match("wans_lanport", "1"))
                                                        sprintf(wan_if, "eth1");
                                                else if (nvram_match("wans_lanport", "2"))
                                                        sprintf(wan_if, "eth2");
                                                else if (nvram_match("wans_lanport", "3"))
                                                        sprintf(wan_if, "eth3");
                                                add_wan_phy(wan_if);
                                        }
                                        else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
                                                add_wan_phy("eth4");
                                        else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
                                                add_wan_phy("eth5");
                                        else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
                                                if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
                                                        int wan_vid = nvram_get_int("switch_wan0tagid");
                                                        if (wan_vid) {
                                                                add_wan_phy("eth0.v0");
                                                        }
                                                        else
                                                                add_wan_phy(the_wan_phy());
                                                }
                                                else
                                                        add_wan_phy(the_wan_phy());
                                        }
                                        else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
                                                add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
                                        else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
                                                add_wan_phy("usb2");
#endif
                                }
                        }
                        else
                                nvram_set("wan_ifnames", "eth0 usb");
                }
                else
                        nvram_unset("wan1_ifname");
#endif  // RTCONFIG_DUALWAN

                if (!nvram_get("ct_max"))
                        nvram_set("ct_max", "300000");
                add_rc_support("mssid 2.4G 5G update usbX1");
                add_rc_support("switchctrl"); // broadcom: for jumbo frame only
                add_rc_support("manual_stb");
                add_rc_support("pwrctrl");
                //add_rc_support("WIFI_LOGO");
                add_rc_support("nandflash");
                add_rc_support("movistarTriple");
                add_rc_support("app");

                break;

#if defined(RTAX92U)
	case MODEL_RTAX92U:
		nvram_unset("dhd0_rnr_flowring_physize");
		nvram_unset("dhd1_rnr_flowring_physize");
		nvram_unset("dhd2_rnr_flowring_physize");
		update_rf_para();
		update_43684_tempthresh();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth5 eth6 eth7");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6 eth7");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth5 eth6 eth7");
		nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
#endif

		nvram_set("1:ledbh10", "0x7");
		nvram_set("2:ledbh10", "0x7");
		nvram_set("3:ledbh15", "0x7");

		nvram_set_int("led_pwr_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);
		nvram_set_int("led_lan_gpio", 16);
		nvram_set_int("btn_wps_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 23|GPIO_ACTIVE_LOW);

#if 0
#ifdef RTCONFIG_XHCIMODE
                nvram_set("xhci_ports", "2-1 2-2");
                nvram_set("ehci_ports", "3-1 3-2");
                nvram_set("ohci_ports", "4-1 4-2");
#else
                if (usb_usb3 == 1)
                        nvram_set("xhci_ports", "2-2 2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-2 3-1");
		nvram_set("ohci_ports", "4-2 4-1");
#endif
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-2");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-2 1-1");
			nvram_set("ohci_ports", "2-2 2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth1 eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth2 eth4");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth3 eth4");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth2 eth3 eth4");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
				add_lan_phy("eth7");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX2");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif	

		break;
#endif

#if defined(RTAX95Q)
	case MODEL_RTAX95Q:
#ifdef RTCONFIG_COMFW
                nvram_set_int("comfw_type", CF_RTAX95Q);
#endif
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");
		nvram_unset("2:ledbh15");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif
#if defined(XT8PRO)
	case MODEL_XT8PRO:
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");
		nvram_unset("2:ledbh15");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "5-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "3-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#if defined(BM68)
	case MODEL_BM68:
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");
		nvram_unset("2:ledbh15");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "5-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "3-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#if defined(XT8_V2)
	case MODEL_XT8_V2:
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");
		nvram_unset("2:ledbh15");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif

#if defined(RTAXE95Q)
	case MODEL_RTAXE95Q:
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");
		nvram_unset("2:ledbh15");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif

#if defined(ET8PRO)
	case MODEL_ET8PRO:
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");
		nvram_unset("2:ledbh15");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif

#if defined(ET8_V2)
	case MODEL_ET8_V2:
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");
		nvram_unset("2:ledbh15");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif

#if defined(RTAX56_XD4)
	case MODEL_RTAX56_XD4:
		// update_rf_para();
#ifdef RTCONFIG_BCM_MFG
		nvram_set("sw_mode", "3");
#endif

		if (nvram_get_int("asus_stacksize") == 0) nvram_set_int("asus_stacksize", 0x100000);

		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
				/* Master */
				nvram_set("lan_ifnames", "eth1 wl0 wl1");
			} else {
				/* Slave */
				nvram_set("lan_ifnames", "wl0 wl1");
			}
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
				nvram_set("lan_ifnames", "eth0 eth1 wl0 wl1");
			} else {
				nvram_set("lan_ifnames", "eth0 wl0 wl1");
			}
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "wl0 wl1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "wl0 wl1");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "wl0 wl1");
		if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
			nvram_set("wired_ifnames", "eth1");
		} else {
			nvram_set("wired_ifnames", "");
		}
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_bt_indicator_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_pa_indicator_gpio", 21|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth3");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "30000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif

#if defined(XD4PRO)
	case MODEL_XD4PRO:
		// update_rf_para();
#ifdef RTCONFIG_BCM_MFG
		nvram_set("sw_mode", "3");
#endif

		if (nvram_get_int("asus_stacksize") == 0) nvram_set_int("asus_stacksize", 0x100000);

		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 wl0 wl1");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 wl0 wl1");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "wl0 wl1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "wl0 wl1");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "wl0 wl1");
		nvram_set("wired_ifnames", "eth1");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_bt_indicator_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_pa_indicator_gpio", 21|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth3");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "30000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif

#if defined(CTAX56_XD4)
	case MODEL_CTAX56_XD4:
		// update_rf_para();
#ifdef RTCONFIG_BCM_MFG
		nvram_set("sw_mode", "3");
#endif

		if (nvram_get_int("asus_stacksize") == 0) nvram_set_int("asus_stacksize", 0x100000);

		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 wl0 wl1");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 wl0 wl1");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "wl0 wl1");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "wl0 wl1");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "wl0 wl1");
		nvram_set("wired_ifnames", "eth1");
#endif

		nvram_unset("0:ledbh39");
		nvram_unset("1:ledbh26");

		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 16|GPIO_ACTIVE_LOW);

#if 0
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth3");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "30000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif		

		break;
#endif

#if defined(RTAX55) || defined(RTAX1800)
	case MODEL_RTAX55:
		// update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth2 eth3");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth2 eth3");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth2 eth3");
		nvram_set("wired_ifnames", "eth1");
#endif

#if 1
		if (nvram_match("HwId", "B")) {
			nvram_unset("sb/0/ed_thresh2g");
			nvram_unset("sb/0/eu_edthresh2g");
			nvram_unset("sb/1/ed_thresh5g");
			nvram_unset("sb/1/eu_edthresh5g");
		}
#else
		nvram_unset("sb/0/eu_edthresh2g");
		nvram_unset("sb/1/eu_edthresh5g");
		if (!nvram_match("x_Setting", "1") || nvram_get_int("re_mode") == 1) {
			if (!strncmp(nvram_safe_get("territory_code"), "EU", 2) ||
				!strncmp(nvram_safe_get("territory_code"), "IL", 2) ||
				!strncmp(nvram_safe_get("territory_code"), "UK", 2)) {
				nvram_unset("no_dy_ed_thresh_ctrl");
				nvram_unset("sb/0/ed_thresh2g");
				nvram_unset("sb/1/ed_thresh5g");
			} else {
				nvram_set("no_dy_ed_thresh_ctrl", "-1");
				nvram_set("sb/0/ed_thresh2g", "-16");
				nvram_set("sb/1/ed_thresh5g", "-16");
			}
		} else {
			nvram_unset("sb/0/ed_thresh2g");
			nvram_unset("sb/1/ed_thresh5g");
		}
#endif

		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 12|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			/* always set LAN as eth1, set dualwan of rtkswitch(LAN) config on config_switch() */
			set_lan_phy("eth1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth3");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						/* set dualwan of rtkswitch(LAN) config on config_switch() */
						sprintf(wan_if, "vlan2");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "30000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#ifdef BCM6750
	case MODEL_RTAX58U:
#ifdef RTAX82_XD6
#ifdef RTCONFIG_MSSID_REALMAC
		nvram_unset("wl0_mbss_ign_mac_valid");
		nvram_unset("wl1_mbss_ign_mac_valid");
#else
		nvram_set("wl0_mbss_ign_mac_valid", "1");
		nvram_set("wl1_mbss_ign_mac_valid", "1");
#endif
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
#ifdef RTAX82_XD6
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth5 eth6");
#else
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth5 eth6");
#endif
			nvram_set("wan_ifname", "eth4");
			nvram_set("wan_ifnames", "eth4");
		} else {
#ifdef RTAX82_XD6
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth4 eth5 eth6");
#else
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
#endif
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth4");
		nvram_set("sta_ifnames", "eth5 eth6");
#ifdef RTAX82_XD6
		nvram_set("wired_ifnames", "eth2 eth1 eth0");
#else
		nvram_set("wired_ifnames", "eth3 eth2 eth1 eth0");
#endif
#endif

#ifdef RTAX82_XD6
		nvram_set("0:ed_thresh2g", "-65");
		nvram_set("0:eu_edthresh2g", "-65");
#endif

#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
		if (!nvram_get_int("LED_order"))
		{
			nvram_set("0:ledbh0", "0x1");   // fake 2.4g LED
			nvram_set("1:ledbh15", "0x80"); // fake WAN LED
		}
		else
#endif
		{
			nvram_unset("0:ledbh0");
#ifdef TUFAX5400
			nvram_set("1:ledbh15", "0x87");
#else
			nvram_set("1:ledbh15", "0x7");
#endif
		}

#ifndef RTAX82_XD6
#ifdef TUFAX5400
		nvram_set_int("led_pwr_gpio", 57|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 57|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("led_pwr_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 23|GPIO_ACTIVE_LOW);
#endif
#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
		if (nvram_get_int("LED_order"))
#endif
		{
#ifdef TUFAX5400
			nvram_set_int("led_wan_gpio", 53|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_normal_gpio", 56);
#else
#if defined(GSAX3000) || defined(GSAX5400)
			nvram_set_int("led_wan_gpio", 17|GPIO_ACTIVE_LOW);
#else
			nvram_set_int("led_wan_gpio", 30|GPIO_ACTIVE_LOW);
#endif
			nvram_set_int("led_wan_normal_gpio", 21);
#endif
		}
#endif

#ifdef RTCONFIG_LAN4WAN_LED
#ifdef TUFAX5400
		nvram_set_int("led_lan1_gpio", 47|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 51|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan3_gpio", 50|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan4_gpio", 55|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("led_lan1_gpio", 24|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 25|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan3_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan4_gpio", 10|GPIO_ACTIVE_LOW);
#endif
#endif
#ifdef TUFAX5400
		nvram_set_int("btn_wps_gpio", 21|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 0xff);
#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
		if (!nvram_get_int("LED_order"))
		nvram_set_int("led_5g_gpio", 31|GPIO_ACTIVE_LOW);	// fake 5g LED
#endif
#if defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400)
		nvram_set_int("led_group1_red_gpio", 22);
		nvram_set_int("led_group1_green_gpio", 6);
		nvram_set_int("led_group1_blue_gpio", 5);
		nvram_set_int("led_group2_red_gpio", 2);
		nvram_set_int("led_group2_green_gpio", 3);
		nvram_set_int("led_group2_blue_gpio", 4);
		nvram_set_int("led_group3_red_gpio", 11);
		nvram_set_int("led_group3_green_gpio", 13);
		nvram_set_int("led_group3_blue_gpio", 24);
		nvram_set_int("led_group4_red_gpio", 8);
		nvram_set_int("led_group4_green_gpio", 29);
		nvram_set_int("led_group4_blue_gpio", 25);
#if defined(GSAX3000) || defined(GSAX5400)
		nvram_set_int("led_group5_red_gpio", 15);
		nvram_set_int("led_group5_green_gpio", 10);
		nvram_set_int("led_group5_blue_gpio", 28);
#else
		nvram_set_int("btn_led_gpio", 10|GPIO_ACTIVE_LOW);
#endif
#endif
#ifdef TUFAX5400
		nvram_set_int("led_group1_red_gpio", 13);
		nvram_set_int("led_group1_green_gpio", 14);
		nvram_set_int("led_group1_blue_gpio", 15);
#endif
#ifdef RTAX82_XD6
		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 4|GPIO_ACTIVE_LOW);
#endif
#ifndef RTAX82_XD6
#ifdef TUFAX5400
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "5-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "3-1");
		}
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
#ifdef RTAX82_XD6
					set_lan_phy("eth1 eth0");
#else
					set_lan_phy("eth2 eth1 eth0");
#endif
				else if (nvram_match("wans_lanport", "2"))
#ifdef RTAX82_XD6
					set_lan_phy("eth2 eth0");
#else
					set_lan_phy("eth3 eth1 eth0");
#endif
				else if (nvram_match("wans_lanport", "3"))
#ifdef RTAX82_XD6
					set_lan_phy("eth2 eth1");
#else
					set_lan_phy("eth3 eth2 eth0");
#endif
#ifndef RTAX82_XD6
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth3 eth2 eth1");
#endif
			}
			else
#ifdef RTAX82_XD6
				set_lan_phy("eth2 eth1 eth0");
#else
				set_lan_phy("eth3 eth2 eth1 eth0");
#endif

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth6");
			if (get_wans_dualwan() == WANSCAP_USB)
				add_lan_phy("eth4");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
#ifdef RTAX82_XD6
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth0");
#else
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth0");
#endif
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth4.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
#ifndef RTAX82_XD6
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth4 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
#ifdef RTAX82_XD6
		add_rc_support("mssid 2.4G 5G update");
#else
		add_rc_support("mssid 2.4G 5G usbX1");
#endif
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
#if defined(TUFAX3000) || defined(TUFAX5400)
		add_rc_support("tuf");
#endif
#ifdef RTAX58U
		if (!strncmp(nvram_safe_get("territory_code"), "CX", 2)) {
			add_rc_support("nz_isp");
			nvram_set("wifi_psk", cfe_nvram_safe_get_raw("secret_code"));
		}
#endif

		break;
#endif

#ifdef RTAX82_XD6S
	case MODEL_RTAX82_XD6S:
#ifdef RTCONFIG_MSSID_REALMAC
		nvram_unset("wl0_mbss_ign_mac_valid");
		nvram_unset("wl1_mbss_ign_mac_valid");
#else
		nvram_set("wl0_mbss_ign_mac_valid", "1");
		nvram_set("wl1_mbss_ign_mac_valid", "1");
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth0 eth2 eth3");
			nvram_set("wan_ifname", "eth1");
			nvram_set("wan_ifnames", "eth1");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth2 eth3");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth2 eth3");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth1");
		nvram_set("sta_ifnames", "eth2 eth3");
		nvram_set("wired_ifnames", "eth0");
#endif

		nvram_set("0:ed_thresh2g", "-65");
		nvram_set("0:eu_edthresh2g", "-65");

		nvram_unset("0:ledbh0");
		nvram_set("1:ledbh15", "0x7");

		nvram_set_int("btn_wps_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 0xff);
		nvram_set_int("bt_rst_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 17|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("");
			}
			else
				set_lan_phy("eth0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth3");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth1.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
				}
			}
			else
				nvram_set("wan_ifnames", "eth1 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");

		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		break;
#endif

#ifdef XD6_V2
	case MODEL_XD6_V2:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth5 eth6");
			nvram_set("wan_ifname", "eth4");
			nvram_set("wan_ifnames", "eth4");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
		nvram_set("sta_phy_ifnames", "eth5 eth6");

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth4");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth2 eth1 eth0");
#endif

		nvram_unset("0:ledbh0");
		nvram_set("1:ledbh15", "0x7");

		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 0xff);

		nvram_set_int("bt_rst_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_disable_gpio", 33|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_red_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 4|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth1 eth0");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth2 eth0");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth2 eth1");
			}
			else
				set_lan_phy("eth2 eth1 eth0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth4.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
				}
			}
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		break;
#endif

#ifdef RTAX58U_V2
	case MODEL_RTAX58U_V2:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth2 eth3");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth2 eth3");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth2 eth3");
		nvram_set("wired_ifnames", "eth1");
#endif

		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 38|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 38|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 80);

		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

		if(!strncmp(nvram_safe_get("territory_code"), "AA", 2) ||
		   !strncmp(nvram_safe_get("territory_code"), "BZ", 2) ||
		   !strncmp(nvram_safe_get("territory_code"), "S2", 2) ||
		   !strncmp(nvram_safe_get("territory_code"), "SG", 2) ||
		   !strncmp(nvram_safe_get("territory_code"), "TW", 2) ||
		   !strncmp(nvram_safe_get("territory_code"), "US", 2)) {
			nvram_set("0:txs_shaper_en_2g", "1");
			nvram_set("0:txs_shaper_bypass", "0");
			nvram_set("0:txs_shaper_dis_2g_nonbndg", "0");
			nvram_set("0:txs_chan_rate_en", "6");
			nvram_set("1:txs_shaper_en_5g", "1");
			nvram_set("1:txs_shaper_bypass", "0");
			nvram_set("1:txs_chan_rate_en", "6");
			nvram_set("1:txs_shaper_dis_5g_nonbndg", "1");
			nvram_set("1:txs_shaper_en_unii3", "1");
			nvram_set("1:txs_shaper_en_unii4", "1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			/* always set LAN as eth1, set dualwan of rtkswitch(LAN) config on config_switch() */
			set_lan_phy("eth1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth3");
			if (get_wans_dualwan() == WANSCAP_USB)
				add_lan_phy("eth0");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						/* set dualwan of rtkswitch(LAN) config on config_switch() */
						sprintf(wan_if, "vlan2");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "30000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#ifdef RTAX3000N
	case MODEL_RTAX3000N:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth2 eth3");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth2 eth3");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth2 eth3");
		nvram_set("wired_ifnames", "eth1");
#endif

		nvram_unset("sb/0/ed_thresh2g");
		nvram_unset("sb/0/eu_edthresh2g");
		nvram_unset("sb/1/ed_thresh5g");
		nvram_unset("sb/1/eu_edthresh5g");

		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 12|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			/* always set LAN as eth1, set dualwan of rtkswitch(LAN) config on config_switch() */
			set_lan_phy("eth1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth3");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						/* set dualwan of rtkswitch(LAN) config on config_switch() */
						sprintf(wan_if, "vlan2");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "30000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#ifdef BR63
	case MODEL_BR63:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth2 eth3");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1)
			nvram_set("sta_phy_ifnames", "dpsta");
		else
#endif
			nvram_set("sta_phy_ifnames", "eth2 eth3");

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth2 eth3");
		nvram_set("wired_ifnames", "eth1");
#endif

		nvram_unset("sb/0/ed_thresh2g");
		nvram_unset("sb/0/eu_edthresh2g");
		nvram_unset("sb/1/ed_thresh5g");
		nvram_unset("sb/1/eu_edthresh5g");

		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 80);
		nvram_set_int("pwr_usb_gpio2", 82);

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1 4-1");
			nvram_set("ohci_ports", "5-1 6-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "3-1 4-1");
			nvram_set("ohci_ports", "5-1 6-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			/* always set LAN as eth1, set dualwan of rtkswitch(LAN) config on config_switch() */
			set_lan_phy("eth1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth2");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth3");
			if (get_wans_dualwan() == WANSCAP_USB)
				add_lan_phy("eth0");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						/* set dualwan of rtkswitch(LAN) config on config_switch() */
						sprintf(wan_if, "vlan2");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth3");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "30000");
		add_rc_support("mssid 2.4G 5G update usbX2");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#ifdef RTAX82U_V2
	case MODEL_RTAX82U_V2:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth5 eth6");
			nvram_set("wan_ifname", "eth4");
			nvram_set("wan_ifnames", "eth4");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth4");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth3 eth2 eth1 eth0");
#endif

		nvram_set("1:ledbh15", "0x7");

		nvram_set_int("led_pwr_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 30|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21|GPIO_ACTIVE_LOW);

		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 0xff);
#if 0
		nvram_set_int("led_group1_red_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_green_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_blue_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group2_red_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group2_green_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group2_blue_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group3_red_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group3_green_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group3_blue_gpio", 24|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group4_red_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group4_green_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group4_blue_gpio", 25|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_led_gpio", 10|GPIO_ACTIVE_LOW);

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth1 eth0");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth3 eth1 eth0");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth3 eth2 eth0");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth3 eth2 eth1");
			}
			else
				set_lan_phy("eth3 eth2 eth1 eth0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth6");
			if (get_wans_dualwan() == WANSCAP_USB)
				add_lan_phy("eth4");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth4.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth4 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		break;
#endif

#ifdef TUFAX5400_V2
	case MODEL_TUFAX5400_V2:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth5 eth6");
			nvram_set("wan_ifname", "eth4");
			nvram_set("wan_ifnames", "eth4");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
		nvram_set("sta_phy_ifnames", "eth5 eth6");

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth4");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth3 eth2 eth1 eth0");
#endif

		nvram_set("1:ledbh15", "0x87");

		nvram_set_int("led_pwr_gpio", 57|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 57|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 53|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 56|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_lan1_gpio", 47|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 51|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan3_gpio", 50|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan4_gpio", 55|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_wps_gpio", 21|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 0xff);
#if 0
		nvram_set_int("led_group1_red_gpio", 13);
		nvram_set_int("led_group1_green_gpio", 14);
		nvram_set_int("led_group1_blue_gpio", 15);
#endif

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth1 eth0");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth3 eth1 eth0");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth3 eth2 eth0");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth3 eth2 eth1");
			}
			else
				set_lan_phy("eth3 eth2 eth1 eth0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth6");
			if (get_wans_dualwan() == WANSCAP_USB)
				add_lan_phy("eth4");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth4.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth4 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("tuf");
		break;
#endif

#ifdef RTAX5400
	case MODEL_RTAX5400:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth5 eth6");
			nvram_set("wan_ifname", "eth4");
			nvram_set("wan_ifnames", "eth4");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");


#ifdef RTCONFIG_AMAS
		nvram_set("sta_phy_ifnames", "eth5 eth6");

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth4");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth3 eth2 eth1 eth0");
#endif

		nvram_set("1:ledbh15", "0x7");

		nvram_set_int("led_pwr_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 23|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 30|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_lan1_gpio", 24|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 25|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan3_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan4_gpio", 10|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 0xff);

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth1 eth0");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth3 eth1 eth0");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth3 eth2 eth0");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth3 eth2 eth1");
			}
			else
				set_lan_phy("eth3 eth2 eth1 eth0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth6");
			if (get_wans_dualwan() == WANSCAP_USB)
				add_lan_phy("eth4");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth4.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth4 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		break;
#endif

#ifdef GT10
	case MODEL_GT10:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			if (nvram_get_int("wans_extwan"))
				nvram_set_int("lacp_enabled", 0);

			if (!nvram_get_int("wans_extwan")) {
				nvram_set("lan_ifnames", "eth1 eth2 eth3 eth6 eth4 eth5");
				nvram_set("wan_ifname", "eth0");
				nvram_set("wan_ifnames", "eth0");
			} else {
				nvram_set("lan_ifnames", "eth0 eth2 eth3 eth6 eth4 eth5");
				nvram_set("wan_ifname", "eth1");
				nvram_set("wan_ifnames", "eth1");
			}
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth6 eth4 eth5");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

//		nvram_set("wl_ifnames", "eth6 eth4 eth5");
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth4 eth5");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
			nvram_set("eth_ifnames", "eth0 eth1");
			nvram_set("amas_ethif_type", "8 4");
			nvram_set("eth_priority", "0 2 1 1 1 1");		// eth0: 2.5G(idx:0,prio:2,used:1) > eth1: 1G(idx:1,prio:1,used:1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3");
			nvram_set("sta_ifnames", "eth6 eth4 eth5");
			nvram_set("sta_priority", "5 1 4 0 5 2 3 1 2 0 5 1");	// 5G-1:(prio:4, used:1), 5G-2:(prio:3, used:1), 2.4G:(prio:5, used:1)
		} else {
#if 0
			if (nvram_match("x_Setting", "0")) {
				nvram_set("eth_ifnames", "eth0 eth1");
				nvram_set("wired_ifnames", "eth2 eth3");
			} else
#endif
			{
				if (!nvram_get_int("wans_extwan")) {
					nvram_set("eth_ifnames", "eth0");
					nvram_set("wired_ifnames", "eth1 eth2 eth3");
				} else {
					nvram_set("eth_ifnames", "eth1");
					nvram_set("wired_ifnames", "eth0 eth2 eth3");
				}
			}

			nvram_set("amas_adtbw_unit", "1");
		}
#endif

		nvram_set_int("btn_wps_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 80);

		nvram_set("1:ledbh14", "0x1");
		nvram_set("2:ledbh14", "0x1");

		nvram_set_int("led_rgb1_red_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_green_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_rgb1_blue_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_red_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_green_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_blue_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group2_red_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group2_green_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group2_blue_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group3_red_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group3_green_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group3_blue_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_white_gpio", 20|GPIO_ACTIVE_LOW);

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth3");
					else
						set_lan_phy("eth0 eth3");
				} else if (nvram_match("wans_lanport", "3")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth2");
					else
						set_lan_phy("eth0 eth2");
				}
			}
			else {
				if (!nvram_get_int("wans_extwan"))
					set_lan_phy("eth1 eth2 eth3");
				else
					set_lan_phy("eth0 eth2 eth3");
			}

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth4 eth5");
			if (get_wans_dualwan() == WANSCAP_USB) {
				if (!nvram_get_int("wans_extwan"))
					add_lan_phy("eth0");
				else
					add_lan_phy("eth1");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1")) {
							if (!nvram_get_int("wans_extwan"))
								sprintf(wan_if, "eth1");
							else
								sprintf(wan_if, "eth0");
						} else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								if (!nvram_get_int("wans_extwan"))
									add_wan_phy("eth0.v0");
								else
									add_wan_phy("eth1.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
				}
			}
			else {
				if (!nvram_get_int("wans_extwan"))
					nvram_set("wan_ifnames", "eth0 usb");
				else
					nvram_set("wan_ifnames", "eth1 usb");
			}
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");

		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#ifdef RTAX9000
	case MODEL_RTAX9000:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth8 eth6 eth7");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth8 eth6 eth7");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

//		nvram_set("wl_ifnames", "eth8 eth6 eth7");
		nvram_set("wl_ifnames", "eth6 eth7 eth8");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7 eth8");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
			nvram_set("eth_ifnames", "eth0");
			nvram_set("amas_ethif_type", "8");
			nvram_set("eth_priority", "0 2 1");	// eth0: 2.5G(idx:0,prio:2,used:1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
			nvram_set("sta_ifnames", "eth8 eth6 eth7");
			nvram_set("sta_priority", "5 1 4 0 5 2 3 1 2 0 5 1");	// 5G-1:(prio:4, used:1), 5G-2:(prio:3, used:1), 2.4G:(prio:5, used:1)
		} else {
#if 0
			if (nvram_match("x_Setting", "0")) {
				nvram_set("eth_ifnames", "eth0 eth1");
				nvram_set("wired_ifnames", "eth2 eth3");
			} else
#endif
			{
				nvram_set("eth_ifnames", "eth0");
				nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
			}

			nvram_set("amas_adtbw_unit", "1");
		}
#endif

		nvram_set_int("btn_wps_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 80);

		nvram_set("1:ledbh14", "0x1");
		nvram_set("2:ledbh14", "0x1");

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "2")) {
					set_lan_phy("eth1 eth3 eth4 eth5");
				} else if (nvram_match("wans_lanport", "3")) {
					set_lan_phy("eth1 eth2 eth4 eth5");
				} else if (nvram_match("wans_lanport", "4")) {
					set_lan_phy("eth1 eth2 eth3 eth5");
				}
			}
			else {
				set_lan_phy("eth1 eth2 eth3 eth4 eth5");
			}

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth8");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth6 eth7");
			if (get_wans_dualwan() == WANSCAP_USB) {
				add_lan_phy("eth0");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth4");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth8");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								add_wan_phy("eth0.v0");
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");

		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#ifdef RTAXE7800
	case MODEL_RTAXE7800:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			if (nvram_get_int("wans_extwan"))
				nvram_set_int("lacp_enabled", 0);

			if (nvram_get_int("bond_wan")) {
				nvram_set("lan_ifnames", "eth0 eth3 eth4 eth5 eth7 eth6");
				nvram_set("wan_ifname", "bondx");
				nvram_set("wan_ifnames", "bondx");
			} else if (!nvram_get_int("wans_extwan")) {
				nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth7 eth6");
				nvram_set("wan_ifname", "eth0");
				nvram_set("wan_ifnames", "eth0");
			} else {
				nvram_set("lan_ifnames", "eth0 eth2 eth3 eth4 eth5 eth7 eth6");
				nvram_set("wan_ifname", "eth1");
				nvram_set("wan_ifnames", "eth1");
			}
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth7 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth5 eth7 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth7 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
			nvram_set("eth_ifnames", "eth0 eth1");
			nvram_set("amas_ethif_type", "8 4");
			nvram_set("eth_priority", "0 2 1 1 1 1");		// eth0: 2.5G(idx:0,prio:2,used:1) > eth1: 1G(idx:1,prio:1,used:1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
			nvram_set("sta_ifnames", "eth5 eth7 eth6");
			nvram_set("sta_priority", "2 0 5 1 5 1 4 1 6 2 3 1");	// 2.4G:(prio:5, used:1), 5G:(prio:4, used:1), 6G:(prio:3, used:1)
		} else {
#if 0
			if (nvram_match("x_Setting", "0")) {
				nvram_set("eth_ifnames", "eth0 eth1");
				nvram_set("wired_ifnames", "eth2 eth3 eth4");
			} else
#endif
			{
				if (nvram_get_int("bond_wan")) {
					nvram_set("eth_ifnames", "bond1");
					nvram_set("wired_ifnames", "eth0 eth3 eth4");
				} else if (!nvram_get_int("wans_extwan")) {
					nvram_set("eth_ifnames", "eth0");
					nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
				} else {
					nvram_set("eth_ifnames", "eth1");
					nvram_set("wired_ifnames", "eth0 eth2 eth3 eth4");
				}
			}
		}
#endif

		nvram_set("1:ledbh15", "0x7");

		nvram_set_int("led_pwr_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 29|GPIO_ACTIVE_LOW);
		if (nvram_get_int("wans_extwan") || nvram_get_int("bond_wan")) {
			nvram_set_int("led_wan_gpio", 5|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan2_gpio", 31|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_normal_gpio", 3|GPIO_ACTIVE_LOW);
		} else {
			nvram_set_int("led_wan_gpio", 31|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan2_gpio", 5|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan_gpio", 3|GPIO_ACTIVE_LOW);
		}

		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 0xff);

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1")) {
					set_lan_phy("eth2 eth3 eth4");
				} else if (nvram_match("wans_lanport", "2")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth3 eth4");
					else
						set_lan_phy("eth0 eth3 eth4");
				} else if (nvram_match("wans_lanport", "3")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth2 eth4");
					else
						set_lan_phy("eth0 eth2 eth4");
				} else if (nvram_match("wans_lanport", "4")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth2 eth3");
					else
						set_lan_phy("eth0 eth2 eth3");
				}
			} else {
				if (nvram_get_int("bond_wan"))
					set_lan_phy("eth0 eth3 eth4");
				else if (!nvram_get_int("wans_extwan"))
					set_lan_phy("eth1 eth2 eth3 eth4");
				else
					set_lan_phy("eth0 eth2 eth3 eth4");
			}
			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth7");
				add_lan_phy("eth6");
			}
			if (get_wans_dualwan() == WANSCAP_USB) {
				if (!nvram_get_int("wans_extwan"))
					add_lan_phy("eth0");
				else
					add_lan_phy("eth1");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1")) {
							if (!nvram_get_int("wans_extwan"))
								sprintf(wan_if, "eth1");
							else
								sprintf(wan_if, "eth0");
						} else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth4");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								if (!nvram_get_int("wans_extwan"))
									add_wan_phy("eth0.v0");
								else
									add_wan_phy("eth1.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else {
				if (!nvram_get_int("wans_extwan"))
					nvram_set("wan_ifnames", "eth0 usb");
				else
					nvram_set("wan_ifnames", "eth1 usb");
			}
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");

		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#ifdef TUFAX3000_V2
	case MODEL_TUFAX3000_V2:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			if (nvram_get_int("wans_extwan"))
				nvram_set_int("lacp_enabled", 0);

			if (!nvram_get_int("wans_extwan")) {
				nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
				nvram_set("wan_ifname", "eth0");
				nvram_set("wan_ifnames", "eth0");
			} else {
				nvram_set("lan_ifnames", "eth0 eth2 eth3 eth4 eth5 eth6");
				nvram_set("wan_ifname", "eth1");
				nvram_set("wan_ifnames", "eth1");
			}
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}

		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
			nvram_set("eth_ifnames", "eth0 eth1");
			nvram_set("amas_ethif_type", "8 4");
			nvram_set("eth_priority", "0 2 1 1 1 1");	// eth0: 2.5G(idx:0,prio:2,used:1) > eth1: 1G(idx:1,prio:1,used:1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
			nvram_set("sta_ifnames", "eth5 eth6");
			nvram_set("sta_priority", "2 0 3 1 5 1 2 1");	// 2.4G:(prio:3, used:1), 5G:(prio:2, used:1)
		} else {
			if (!nvram_get_int("wans_extwan")) {
				nvram_set("eth_ifnames", "eth0");
				nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
			} else {
				nvram_set("eth_ifnames", "eth1");
				nvram_set("wired_ifnames", "eth0 eth2 eth3 eth4");
			}
		}
#endif

		nvram_set_int("led_pwr_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int(nvram_get_int("wans_extwan") ? "led_wan_normal_gpio"  : "led_lan_gpio", 18|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 31|GPIO_ACTIVE_LOW);

		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 80);

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3 eth4");
				else if (nvram_match("wans_lanport", "2")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth3 eth4");
					else
						set_lan_phy("eth0 eth3 eth4");
				} else if (nvram_match("wans_lanport", "3")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth2 eth4");
					else
						set_lan_phy("eth0 eth2 eth4");
				} else if (nvram_match("wans_lanport", "4")) {
					if (!nvram_get_int("wans_extwan"))
						set_lan_phy("eth1 eth2 eth3");
					else
						set_lan_phy("eth0 eth1 eth3");
				}
			}
			else {
				if (!nvram_get_int("wans_extwan"))
					set_lan_phy("eth1 eth2 eth3 eth4");
				else
					set_lan_phy("eth0 eth2 eth3 eth4");
			}

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth6");
			if (get_wans_dualwan() == WANSCAP_USB) {
				if (!nvram_get_int("wans_extwan"))
					add_lan_phy("eth0");
				else
					add_lan_phy("eth1");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1")) {
							if (!nvram_get_int("wans_extwan"))
								sprintf(wan_if, "eth1");
							else
								sprintf(wan_if, "eth0");
						} else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth4");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								if (!nvram_get_int("wans_extwan"))
									add_wan_phy("eth0.v0");
								else
									add_wan_phy("eth1.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else {
				if (!nvram_get_int("wans_extwan"))
					nvram_set("wan_ifnames", "eth0 usb");
				else
					nvram_set("wan_ifnames", "eth1 usb");
			}
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");

		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("tuf");

		break;
#endif

#if defined(DSL_AX82U)
	case MODEL_DSLAX82U:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth5 eth6");
			nvram_set("wan_ifname", "eth4");
			if (nvram_match("dslx_transmode", "ptm"))
				nvram_set("wan_ifnames", DSL_WAN_PTM_IF);
			else
				nvram_set("wan_ifnames", DSL_WAN_ATM_IF);
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth4");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth3 eth2 eth1 eth0");
#endif

		nvram_set_int("led_pwr_gpio", 24|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_red_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 24|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WANRED_LED
		nvram_set_int("led_wan_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_red_gpio", 9|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("led_wan_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 8|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("led_lan_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wifi_gpio", 27|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_red_gpio", 26);
		nvram_set_int("led_group1_green_gpio", 28);
		nvram_set_int("led_group1_blue_gpio", 11);
		nvram_set_int("led_group2_red_gpio", 14);
		nvram_set_int("led_group2_green_gpio", 15);
		nvram_set_int("led_group2_blue_gpio", 16);
		nvram_set_int("led_group3_red_gpio", 2);
		nvram_set_int("led_group3_green_gpio", 3);
		nvram_set_int("led_group3_blue_gpio", 4);
		nvram_set_int("led_group4_red_gpio", 5);
		nvram_set_int("led_group4_green_gpio", 10);
		nvram_set_int("led_group4_blue_gpio", 7);

#if 0
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-2");
#endif
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1");
			nvram_set("ohci_ports", "4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1");
			nvram_set("ohci_ports", "2-1");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth1 eth0");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth3 eth1 eth0");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth3 eth2 eth0");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth3 eth2 eth1");
			}
			else
				set_lan_phy("eth3 eth2 eth1 eth0");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth0");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_DSL)
						if (nvram_match("dslx_transmode", "ptm"))
							add_wan_phy(DSL_WAN_PTM_IF);
						else
							add_wan_phy(DSL_WAN_ATM_IF);
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth4.v0");
							}
							else
								add_wan_phy("eth4");
						}
						else
							add_wan_phy("eth4");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				if (nvram_match("dslx_transmode", "ptm"))
					nvram_set("wan_ifnames", DSL_WAN_PTM_IF);
				else
					nvram_set("wan_ifnames", DSL_WAN_ATM_IF);
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (is_ax5400_i1d())
			nvram_set("TR_debug", "1");
		else
			nvram_set("TR_debug", "0");
		if (is_ax5400_i1n())
			add_rc_support("noFwManual");

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("dsl");
		add_rc_support("vdsl");
		add_rc_support("spectrum");
		add_rc_support("mssid 2.4G 5G usbX1");
		if(is_ax5400_i1()){
			add_rc_support("noupdate");
		}
		else{
			add_rc_support("update");
		}
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("nandflash");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("defpsk");

		nvram_set("wl0_mbss_ign_mac_valid", "1");
		nvram_set("wl1_mbss_ign_mac_valid", "1");

		break;
#endif

#if defined(RTAX56U)
	case MODEL_RTAX56U:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth4 eth3 eth2 eth1 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth4 eth3 eth2 eth1");
#endif

		nvram_unset("sb/0/ledbh39");
		nvram_unset("sb/1/ledbh7");

		nvram_set_int("led_pwr_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 27|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 28|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);

#if 0
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "2-1");
		nvram_set("ehci_ports", "3-1");
		nvram_set("ohci_ports", "4-1");
#else
		if (usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "3-1 5-1");
		nvram_set("ohci_ports", "4-1 6-1");
#endif
#else
		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "3-1 4-1");
			nvram_set("ohci_ports", "4-1 4-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 2-1");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth3 eth2 eth1");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth4 eth2 eth1");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth4 eth3 eth1");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth4 eth3 eth2");
			}
			else
				set_lan_phy("eth4 eth3 eth2 eth1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#if defined(RPAX56) || defined(RPAX58)
	case MODEL_RPAX58:
		nvram_set("lan_ifname", "br0");
		nvram_set("lan_ifnames", "eth0 eth1 eth2");
		nvram_set("wan_ifnames", "");
		nvram_set("wan_ifname", "");
		if(nvram_match("x_Setting", "0"))
			nvram_set("wan0_ifname", "eth0");

		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		if(nvram_match("sw_mode", "3") && nvram_match("wlc_psta", "0") && !nvram_match("wlc_dpsta", "0")){
			_dprintf("adjust wlc_dpsta as 0 (from %d)\n", nvram_get_int("wlc_dpsta"));
			nvram_set("wlc_dpsta", "0");
		}

		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

		if(dpsr_mode()) {
			_dprintf("%s, set dpsr\n", __func__);
			set_dpsr_ifnames();
		}
//#if 0
		if(!nvram_match("ed_thresh_dft", "1")) {
			_dprintf("reset phy_ed_thresh.");
			if(!strncmp(nvram_safe_get("territory_code"), "EU", 2) ||
			!strncmp(nvram_safe_get("territory_code"), "UK", 2)
			) {
				nvram_unset("sb/0/eu_edthresh2g");
				nvram_unset("sb/1/eu_edthresh5g");
				nvram_unset("sb/0/ed_thresh2g");
				nvram_unset("sb/1/ed_thresh5g");
			} else {
				nvram_set("no_dy_ed_thresh_ctrl", "-1");
				nvram_set("sb/0/ed_thresh2g", "-16");
				nvram_set("sb/1/ed_thresh5g", "-16");
			}	
		}
//#endif
#ifdef RTCONFIG_AMAS

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "10");
			nvram_set("wait_wifi", "15");
		}


		if(is_cap()) {
			nvram_set("eth_ifnames", "");
			nvram_set("wired_ifnames", "eth0");

			nvram_set("amas_ethif_type", ""); 		// 1G
			nvram_set("eth_priority", ""); 		// eth0: 1G(idx:0,prio:1,used:1)
			nvram_set("sta_ifnames", "");
			nvram_set("sta_priority", "");	// 2.4G:(prio:4), 5G-1:(prio:3)
			nvram_set("sta_phy_ifnames", "");	// dpsr
		} else {
			nvram_set("eth_ifnames", "eth0");
			nvram_set("wired_ifnames", "");

			nvram_set("amas_ethif_type", "4"); 		// 1G
			nvram_set("eth_priority", "0 1 1"); 		// eth0: 1G(idx:0,prio:1,used:1)
			nvram_set("sta_ifnames", "eth1 eth2");
			nvram_set("sta_priority", "2 0 4 1 51 1 3 1");	// 2.4G:(prio:4), 5G-1:(prio:3)
			nvram_set("sta_phy_ifnames", "eth1 eth2");	// dpsr
		}
		// 504 added
		nvram_set("amas_lldp_ifnames", "eth0");
		nvram_set("amas_lldp_iftypes", "4");		// 1G
#endif

		if(dpsr_mode() && nvram_get_int("x_Setting")>0) {
			if(!*nvram_safe_get("wlc_band")) {
				if(*nvram_safe_get("wlc0_ssid") && !*nvram_safe_get("wlc1_ssid"))
					nvram_set("wlc_band", "0");
				else if(*nvram_safe_get("wlc1_ssid") && !*nvram_safe_get("wlc0_ssid"))
					nvram_set("wlc_band", "1");
			}
			/* transfer settings due ap interface changes */
			if(!*nvram_safe_get("wlc0_ssid") && *nvram_safe_get("wl0.1_ssid")) {
				_dprintf("\n%s, chkchk Reset wl0_ssid here: [%d][%s][%s]\n", __func__, rp_mode(),  nvram_safe_get("wlc0_ssid"), nvram_safe_get("wl0.1_ssid"));
				nvram_set("wl0_ssid", nvram_safe_get("wl0.1_ssid"));
				nvram_set("wl0_auth_mode_x", nvram_safe_get("wl0.1_auth_mode_x"));
				nvram_set("wl0_wpa_psk", nvram_safe_get("wl0.1_wpa_psk"));
			}
			if(!*nvram_safe_get("wlc1_ssid") && *nvram_safe_get("wl1.1_ssid")) {
				_dprintf("\n%s, chkchk Reset wl1_ssid here: [%d][%s][%s]\n", __func__, rp_mode(),  nvram_safe_get("wlc1_ssid"), nvram_safe_get("wl1.1_ssid"));
				nvram_set("wl1_ssid", nvram_safe_get("wl1.1_ssid"));
				nvram_set("wl1_auth_mode_x", nvram_safe_get("wl1.1_auth_mode_x"));
				nvram_set("wl1_wpa_psk", nvram_safe_get("wl1.1_wpa_psk"));
			}
			if(!*nvram_safe_get("wlc0_ssid")) {
				set_rannv("wlc0_ssid", 20);
			}
			if(!*nvram_safe_get("wlc1_ssid")) {
				set_rannv("wlc1_ssid", 20);
			}
		} else if(nvram_get_int("x_Setting")==0) {
			reset_rp_def_ssid();
		} else
			_dprintf("%s: chkchk: Not reset wlx_ssid due not rp_mode or in default\n", __func__);

                // force enable atf when not be RE
                if(client_mode() && !nvram_match("re_mode", "1")) {
			nvram_set("wl_atf", "1");
			nvram_set("wl0_atf", "1");
			nvram_set("wl1_atf", "1");
		}

		if(!nvram_get_int("dwds_ctrl")) {
			_dprintf("force disable dwds\n");
			nvram_set("wl_dwds", "0");
			nvram_set("wl0_dwds", "0");
			nvram_set("wl1_dwds", "0");
			nvram_set("wl0.1_dwds", "0");
			nvram_set("wl1.1_dwds", "0");
		}

		if(client_mode() == 1)
			nvram_set("wl0_frameburst", "off");

		nvram_set_int("led_red_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_green_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_blue_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_white_gpio", 12);
		nvram_set_int("led_yellow_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_purple_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "150000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("11AX");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("wifi2017");
		add_rc_support("ofdma");
		add_rc_support("app");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		break;

	case MODEL_RPAX56:
		//chk_cfe_ate();
		nvram_set("lan_ifname", "br0");
#if defined(RTCONFIG_BCM_MFG)
		nvram_set("lan_ifnames", "eth0 eth1 eth2");
#else
		if(nvram_match("x_Setting", "0"))
			nvram_set("lan_ifnames", "eth0 eth1 eth2 wl0.1 wl1.1");
		else
			nvram_set("lan_ifnames", "eth0 eth1 eth2");
#endif
		nvram_set("wan_ifnames", "");
		nvram_set("wan_ifname", "");
		if(nvram_match("x_Setting", "0")) {
			nvram_set("wan0_ifname", "eth0");
		}

		if(nvram_get_int("sw_mode") == 2){
			nvram_set("sw_mode", "3");
			nvram_set("wlc_psta", "2");
		}
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//#if 0
		if(!nvram_match("ed_thresh_dft", "1")) {
			_dprintf("reset phy_ed_thresh.");
			if(!strncmp(nvram_safe_get("territory_code"), "EU", 2) ||
			!strncmp(nvram_safe_get("territory_code"), "UK", 2)
			) {
				nvram_unset("sb/0/eu_edthresh2g");
				nvram_unset("sb/1/eu_edthresh5g");
				nvram_unset("sb/0/ed_thresh2g");
				nvram_unset("sb/1/ed_thresh5g");
			} else {
				nvram_set("no_dy_ed_thresh_ctrl", "-1");
				nvram_set("sb/0/ed_thresh2g", "-16");
				nvram_set("sb/1/ed_thresh5g", "-16");
			}	
		}
//#endif
#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth1 eth2");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "10");
			nvram_set("wait_wifi", "15");
		}

		nvram_set("sta_ifnames", "eth1 eth2");

		if(is_cap()) {
			nvram_set("eth_ifnames", "");
			nvram_set("wired_ifnames", "eth0");
		} else {
			nvram_set("eth_ifnames", "eth0");
			nvram_set("wired_ifnames", "");
		}
#endif

		if(rp_mode() && nvram_get_int("x_Setting")>0) {
			if(!*nvram_safe_get("wlc_band")) {
				if(*nvram_safe_get("wlc0_ssid") && !*nvram_safe_get("wlc1_ssid"))
					nvram_set("wlc_band", "0");
				else if(*nvram_safe_get("wlc1_ssid") && !*nvram_safe_get("wlc0_ssid"))
					nvram_set("wlc_band", "1");
			}
			/* transfer settings due ap interface changes */
			if(!*nvram_safe_get("wlc0_ssid") && *nvram_safe_get("wl0.1_ssid")) {
				_dprintf("\n%s, chkchk Reset wl0_ssid here: [%d][%s][%s]\n", __func__, rp_mode(),  nvram_safe_get("wlc0_ssid"), nvram_safe_get("wl0.1_ssid"));
				nvram_set("wl0_ssid", nvram_safe_get("wl0.1_ssid"));
				nvram_set("wl0_auth_mode_x", nvram_safe_get("wl0.1_auth_mode_x"));
				nvram_set("wl0_wpa_psk", nvram_safe_get("wl0.1_wpa_psk"));
			}
			if(!*nvram_safe_get("wlc1_ssid") && *nvram_safe_get("wl1.1_ssid")) {
				_dprintf("\n%s, chkchk Reset wl1_ssid here: [%d][%s][%s]\n", __func__, rp_mode(),  nvram_safe_get("wlc1_ssid"), nvram_safe_get("wl1.1_ssid"));
				nvram_set("wl1_ssid", nvram_safe_get("wl1.1_ssid"));
				nvram_set("wl1_auth_mode_x", nvram_safe_get("wl1.1_auth_mode_x"));
				nvram_set("wl1_wpa_psk", nvram_safe_get("wl1.1_wpa_psk"));
			}
			if(!*nvram_safe_get("wlc0_ssid")) {
				set_rannv("wlc0_ssid", 20);
			}
			if(!*nvram_safe_get("wlc1_ssid")) {
				set_rannv("wlc1_ssid", 20);
			}
		} else if(nvram_get_int("x_Setting")==0) {
			reset_rp_def_ssid();
		} else
			_dprintf("%s: chkchk: Not reset wlx_ssid due not rp_mode or in default\n", __func__);

                // force enable atf when not be RE
                if(client_mode() && !nvram_match("re_mode", "1")) {
			nvram_set("wl_atf", "1");
			nvram_set("wl0_atf", "1");
			nvram_set("wl1_atf", "1");
		}

		if(!nvram_get_int("dwds_ctrl")) {
			_dprintf("force disable dwds\n");
			nvram_set("wl_dwds", "0");
			nvram_set("wl0_dwds", "0");
			nvram_set("wl1_dwds", "0");
			nvram_set("wl0.1_dwds", "0");
			nvram_set("wl1.1_dwds", "0");
		}

// for sku test
		if(nvram_match("x_Setting", "0")) {
			_dprintf("%s, Reset 11ax defaults\n", __func__);
				nvram_set("wl0_11ax", "1");
				nvram_set("wl1_11ax", "1");
				nvram_set("wl0_ofdma", "2");
				nvram_set("wl1_ofdma", "2");
				nvram_set("wl0_he_features", "15");
				nvram_set("wl1_he_features", "15");
		}
// ~sku test
		//nvram_unset("sb/0/ledbh39");
		//nvram_unset("sb/1/ledbh7");

		nvram_set_int("led_red_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_green_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_blue_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_white_gpio", 12);
		//nvram_unset("led_white_gpio");
		nvram_set_int("led_yellow_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("led_purple_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "150000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		//add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("wifi2017");
		add_rc_support("ofdma");
		add_rc_support("app");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
#if defined(RTCONFIG_BCM_MFG) && (defined(RPAX56) || defined(RPAX58))
		if(!nvram_match("stop_telnetd", "1"))
			nvram_set("telnetd_enable", "1");
#endif
		break;
#endif

#if defined(RTAX86U)
	case MODEL_RTAX86U:
		{
		char productid[16];
		snprintf(productid, sizeof(productid), "%s", get_productid());

		nvram_unset("dhd0_rnr_flowring_physize");
		nvram_unset("dhd1_rnr_flowring_physize");
		nvram_unset("dhd2_rnr_flowring_physize");
		if(!strcmp(productid, "RT-AX86S")){
			if (is_router_mode()) {
				nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth5 eth6");
				nvram_set("wan_ifname", "eth0");
				nvram_set("wan_ifnames", "eth0");
			} else {
				nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth0 eth5 eth6");
				nvram_set("wan_ifnames", "");
				nvram_set("wan_ifname", "");
			}
			nvram_set("wl_ifnames", "eth5 eth6");
		} else {
			get_ext_phy_id();
			if (is_router_mode()) {
				nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth5 eth6 eth7");
				nvram_set("wan_ifname", "eth0");
				nvram_set("wan_ifnames", "eth0");
			} else {
				nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth0 eth5 eth6 eth7");
				nvram_set("wan_ifnames", "");
				nvram_set("wan_ifname", "");
			}
			nvram_set("wl_ifnames", "eth6 eth7");
		}
		nvram_set("lan_ifname", "br0");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
//		nvram_set("bcm43684_nic", "1");	// bring up bcm43684 in nic mode

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			if(!strcmp(productid, "RT-AX86S"))
				nvram_set("sta_phy_ifnames", "eth5 eth6");
			else
				nvram_set("sta_phy_ifnames", "eth6 eth7");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		if(!strcmp(productid, "RT-AX86S")){
			nvram_set("eth_ifnames", "eth0"); // 1G
			nvram_set("amas_ethif_type", "4"); // 1G
			nvram_set("eth_priority", "0 1 1"); // eth0: 1G(idx:0,prio:1,used:1)
			nvram_set("sta_ifnames", "eth5 eth6");
		} else {
			nvram_set("eth_ifnames", "eth0 eth5"); // 1G, 2.5G
			nvram_set("amas_ethif_type", "4 8"); // 1G, 2.5G
			nvram_set("eth_priority", "0 1 1 1 2 1"); // eth0: 1G(idx:0,prio:1,used:1) > eth5: 2.5G(idx:1,prio:2,used:1)
			nvram_set("sta_ifnames", "eth6 eth7");
		}
		nvram_set("sta_priority", "2 0 4 1 5 1 3 1"); // 2.4G:(prio:4), 5G-1:(prio:3)
		if(!strcmp(productid, "RT-AX86S") || nvram_get_int("re_mode") == 1)
			nvram_set("wired_ifnames", "eth4 eth3 eth2 eth1");
		else
			nvram_set("wired_ifnames", "eth4 eth3 eth2 eth1 eth5");
#endif

		nvram_set("0:ledbh7", "0x7");
		nvram_set("1:ledbh15", "0x7");

		nvram_set_int("led_pwr_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);

		if(!strcmp(productid, "RT-AX86S")){
			nvram_set_int("led_usb_gpio", 15|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LAN4WAN_LED
			nvram_set_int("led_lan1_gpio", 19|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan2_gpio", 18|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan3_gpio", 17|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan4_gpio", 16|GPIO_ACTIVE_LOW);
#endif
			nvram_set_int("led_wan_gpio", 20|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_normal_gpio", 21);
			nvram_set_int("btn_wps_gpio", 22|GPIO_ACTIVE_LOW);
			nvram_set_int("btn_rst_gpio", 23|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_BTN
			nvram_set_int("btn_led_gpio", 14|GPIO_ACTIVE_LOW);
#endif

			if(usb_usb3 == 1){
				nvram_set("xhci_ports", "2-2 2-1");
				nvram_set("ehci_ports", "3-2 3-1");
				nvram_set("ohci_ports", "4-2 4-1");
			}
			else{
				nvram_unset("xhci_ports");
				nvram_set("ehci_ports", "1-2 1-1");
				nvram_set("ohci_ports", "2-2 2-1");
			}
		}
		else{
#ifdef RTCONFIG_LAN4WAN_LED
			nvram_set_int("led_lan1_gpio", 19|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan2_gpio", 18|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan3_gpio", 17|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan4_gpio", 26|GPIO_ACTIVE_LOW);
#endif
			if(nvram_get_int("wans_extwan")){
				nvram_set_int("led_wan_gpio", 30|GPIO_ACTIVE_LOW);
				nvram_set_int("led_lan_gpio", 21|GPIO_ACTIVE_LOW); // WAN port is as LAN
				nvram_set_int("led_extphy_gpio", 27|GPIO_ACTIVE_LOW);
			}
			else
			{
				nvram_set_int("led_wan_gpio", 27|GPIO_ACTIVE_LOW);
				nvram_set_int("led_wan_normal_gpio", 21);
				nvram_set_int("led_extphy_gpio", 30|GPIO_ACTIVE_LOW);
			}
			nvram_set_int("btn_wps_gpio", 2|GPIO_ACTIVE_LOW);
			nvram_set_int("btn_rst_gpio", 31|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_BTN
			nvram_set_int("btn_led_gpio", 15|GPIO_ACTIVE_LOW);
#endif

			if(usb_usb3 == 1){
				nvram_set("xhci_ports", "2-1 2-2");
				nvram_set("ehci_ports", "3-1 3-2");
				nvram_set("ohci_ports", "4-1 4-2");
			}
			else{
				nvram_unset("xhci_ports");
				nvram_set("ehci_ports", "1-1 1-2");
				nvram_set("ohci_ports", "2-1 2-2");
			}
		}
		nvram_set_int("pwr_usb_gpio", 3|GPIO_ACTIVE_LOW);

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if(!strcmp(productid, "RT-AX86S")){
				if (get_wans_dualwan()&WANSCAP_LAN) {
					if (nvram_match("wans_lanport", "1"))
						set_lan_phy("eth3 eth2 eth1");
					else if (nvram_match("wans_lanport", "2"))
						set_lan_phy("eth4 eth2 eth1");
					else if (nvram_match("wans_lanport", "3"))
						set_lan_phy("eth4 eth3 eth1");
					else if (nvram_match("wans_lanport", "4"))
						set_lan_phy("eth4 eth3 eth2");
				}
				else
					set_lan_phy("eth4 eth3 eth2 eth1");

				if (!(get_wans_dualwan()&WANSCAP_2G))
					add_lan_phy("eth5");
				if (!(get_wans_dualwan()&WANSCAP_5G))
					add_lan_phy("eth6");

				get_wans_dualwan_str(wancaps, sizeof(wancaps));
				if (strlen(wancaps) > 0) {
					set_wan_phy("");
					for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
						if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
							if (nvram_match("wans_lanport", "1"))
								add_wan_phy("eth4");
							else if (nvram_match("wans_lanport", "2"))
								add_wan_phy("eth3");
							else if (nvram_match("wans_lanport", "3"))
								add_wan_phy("eth2");
							else if (nvram_match("wans_lanport", "4"))
								add_wan_phy("eth1");
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
							add_wan_phy("eth5");
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
							add_wan_phy("eth6");
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
							if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
								int wan_vid = nvram_get_int("switch_wan0tagid");
								if (wan_vid)
									add_wan_phy("eth0.v0");
								else
									add_wan_phy("eth0");
							}
							else
								add_wan_phy("eth0");
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
							add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
							add_wan_phy("usb2");
#endif
					}
				}
				else
					nvram_set("wan_ifnames", "eth0 usb");
			} else {
				if (get_wans_dualwan()&WANSCAP_LAN) {
					if (nvram_match("wans_lanport", "1"))
						set_lan_phy("eth3 eth2 eth1 eth5");
					else if (nvram_match("wans_lanport", "2"))
						set_lan_phy("eth4 eth2 eth1 eth5");
					else if (nvram_match("wans_lanport", "3"))
						set_lan_phy("eth4 eth3 eth1 eth5");
					else if (nvram_match("wans_lanport", "4"))
						set_lan_phy("eth4 eth3 eth2 eth5");
					else if (nvram_match("wans_lanport", "5"))
						set_lan_phy("eth4 eth3 eth2 eth1");
				}
				else
					set_lan_phy("eth4 eth3 eth2 eth1 eth5");

				if (!(get_wans_dualwan()&WANSCAP_2G))
					add_lan_phy("eth6");
				if (!(get_wans_dualwan()&WANSCAP_5G))
					add_lan_phy("eth7");

				get_wans_dualwan_str(wancaps, sizeof(wancaps));
				if (strlen(wancaps) > 0) {
					set_wan_phy("");
					for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
						if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
							if (nvram_match("wans_lanport", "1"))
								add_wan_phy("eth4");
							else if (nvram_match("wans_lanport", "2"))
								add_wan_phy("eth3");
							else if (nvram_match("wans_lanport", "3"))
								add_wan_phy("eth2");
							else if (nvram_match("wans_lanport", "4"))
								add_wan_phy("eth1");
							else if (nvram_match("wans_lanport", "5"))
								add_wan_phy("eth5");
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
							add_wan_phy("eth6");
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
							add_wan_phy("eth7");
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
							if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
								int wan_vid = nvram_get_int("switch_wan0tagid");
								if (wan_vid)
									add_wan_phy("eth0.v0");
								else
									add_wan_phy("eth0");
							}
							else
								add_wan_phy("eth0");
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
							add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
							add_wan_phy("usb2");
#endif
					}
				}
				else
					nvram_set("wan_ifnames", "eth0 usb");
			}
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		if(strcmp(productid, "RT-AX86S"))
			add_rc_support("2p5G_LWAN");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		}
		break;
#endif

#if defined(RTAX68U)
	case MODEL_RTAX68U:
#if 1
		if(nvram_get_int("wl1_txpower") == 0)
			nvram_unset("avsint");
		else
			nvram_set("avsint", "0");
#else
		nvram_unset("avsint");
#endif

		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth0 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth4 eth3 eth2 eth1");
#endif

		nvram_set("0:ledbh7", "0x7");
		nvram_set("1:ledbh7", "0x7");

		nvram_set_int("led_pwr_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 16|GPIO_ACTIVE_LOW);

		nvram_set_int("led_wan_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);

		nvram_set_int("btn_wps_gpio", 22|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 23|GPIO_ACTIVE_LOW);

		//nvram_set_int("pwr_usb_gpio", 3|GPIO_ACTIVE_LOW);

		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-2");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-2 1-1");
			nvram_set("ohci_ports", "2-2 2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth3 eth2 eth1");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth4 eth2 eth1");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth4 eth3 eth1");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth4 eth3 eth2");
			}
			else
				set_lan_phy("eth4 eth3 eth2 eth1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
			}

			get_wans_dualwan_str(wancaps, sizeof(wancaps));
			if (strlen(wancaps) > 0) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy("eth0");
						}
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif

		break;
#endif

#if defined(RTAC68U_V4)
	case MODEL_RTAC68U_V4:
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth4 eth3 eth2 eth1 eth0 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth5 eth6");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0");
		nvram_set("sta_ifnames", "eth5 eth6");
		nvram_set("wired_ifnames", "eth4 eth3 eth2 eth1");
#endif

		nvram_set("0:ledbh7", "0x7");
		nvram_set("1:ledbh7", "0x7");

		nvram_set_int("led_usb_gpio", 0 | GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb3_gpio", 2 | GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 4 | GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 4 | GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 14 | GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 15 | GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_lan1_gpio", 16);
		nvram_set_int("led_lan2_gpio", 17);
		nvram_set_int("led_lan3_gpio", 18);
		nvram_set_int("led_lan4_gpio", 19);
#endif
		nvram_set_int("led_wan_gpio", 20 | GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);
		nvram_set_int("btn_wps_gpio", 22 | GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 23 | GPIO_ACTIVE_LOW);

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-2");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-2 1-1");
			nvram_set("ohci_ports", "2-2 2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth3 eth2 eth1");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth4 eth2 eth1");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth4 eth3 eth1");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth4 eth3 eth2");
			}
			else
				set_lan_phy("eth4 eth3 eth2 eth1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth5");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth6");
			}

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth1");
						add_wan_phy(wan_if);
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy("eth0");
						}
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");

		break;
#endif

#if defined(GTAXE11000)
	case MODEL_GTAXE11000:
		nvram_unset("dhd0_rnr_flowring_physize");
		nvram_unset("dhd1_rnr_flowring_physize");
		nvram_unset("dhd2_rnr_flowring_physize");
		update_rf_para();
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth6 eth7 eth8");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7 eth8");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0 eth5");
		nvram_set("amas_ethif_type", "4 8"); // 1G, 2.5G
		nvram_set("eth_priority", "0 1 1 1 2 1"); // eth0: 1G(idx:0,prio:1,used:1) > eth5: 2.5G(idx:1,prio:2,used:1)
		nvram_set("sta_priority", "2 0 5 1 5 1 4 1 6 2 3 1");
		nvram_set("sta_ifnames", "eth6 eth7 eth8");

		if(nvram_get_int("re_mode") == 1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
		else
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif

		nvram_set("1:ledbh9", "0x7");
		nvram_set("2:ledbh9", "0x7");
		nvram_set("3:ledbh9", "0x7");

		nvram_set_int("led_pwr_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_normal_gpio", 21);
		nvram_set_int("led_lan_gpio", 25);
		nvram_set_int("led_wps_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio", 26);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 27|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_wps_gpio", 29|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 4|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_TURBO_BTN
		nvram_set_int("btn_turbo_gpio", 31|GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_LOGO_LED
		nvram_set_int("led_logo_gpio", 15|GPIO_ACTIVE_LOW);
#endif

		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-2 2-1");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-2 1-1");
			nvram_set("ohci_ports", "2-2 2-1");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth2 eth3 eth5");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth1 eth2 eth4 eth5");
				else if (nvram_match("wans_lanport", "5"))
					set_lan_phy("eth1 eth2 eth3 eth4");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4 eth5");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth7");
				add_lan_phy("eth8");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				char prefix[8];
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "5"))
							sprintf(wan_if, "eth5");
						add_wan_phy(wan_if);

					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
#ifdef RTCONFIG_EXTPHY_BCM84880
		add_rc_support("2p5G_LWAN");
#endif
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		if(!nvram_match("wl2_nband","4"))
			nvram_set("wl2_nband","4");

		break;
#endif

#if defined(GTAX6000)
	case MODEL_GTAX6000:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth6 eth7");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
			nvram_set("eth_ifnames", "eth0 eth5");
			nvram_set("amas_ethif_type", "8 8");
			nvram_set("eth_priority", "0 2 1 1 1 1");	// eth0: 2.5G(idx:0,prio:2,used:1) > eth5: 2.5G(idx:1,prio:1,used:1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
			nvram_set("sta_ifnames", "eth6 eth7");
			nvram_set("sta_priority", "2 0 4 1 5 1 3 1");	// 2.4G:(prio:4, used:1), 5G:(prio:3, used:1)
		} else {
			nvram_set("eth_ifnames", "eth0");
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
		}
#endif

		nvram_set_int("led_pwr_gpio", 50|GPIO_ACTIVE_LOW);	// PWR LED
		nvram_set_int("led_wan_gpio", 49|GPIO_ACTIVE_LOW);	// WAN RED LED
		nvram_set_int("led_wan_normal_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 1);			// LAN LED
		nvram_set_int("led_wps_gpio", 48|GPIO_ACTIVE_LOW);	// WPS LED
//		nvram_set_int("pwr_usb_gpio", 59);
		nvram_set_int("btn_wps_gpio", 12|GPIO_ACTIVE_LOW);	// WPS BTN
		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);	// RST BTN
		nvram_set_int("btn_led_gpio", 10|GPIO_ACTIVE_LOW);	// LED BTN

		nvram_set_int("led_group1_red_gpio", 2);	// sw_led_02-pin_2
		nvram_set_int("led_group1_green_gpio", 4);	// sw_led_04-pin_4
		nvram_set_int("led_group1_blue_gpio", 3);	// sw_led_04-pin_3
		nvram_set_int("led_group2_red_gpio", 5);	// sw_led_05-pin_5
		nvram_set_int("led_group2_green_gpio", 8);	// sw_led_08-pin_8
		nvram_set_int("led_group2_blue_gpio", 7);	// sw_led_07-pin_7
		nvram_set_int("led_group3_red_gpio", 14);	// sw_led_14-pin_40
		nvram_set_int("led_group3_green_gpio", 16);	// sw_led_16-pin_42
		nvram_set_int("led_group3_blue_gpio", 15);	// sw_led_15-pin_41
		nvram_set_int("led_group_ant1_gpio", 18);	// ANT1 LED
		nvram_set_int("led_group_ant2_gpio", 19);	// ANT2 LED
		nvram_set_int("led_group_ant3_gpio", 20);	// ANT3 LED
		nvram_set_int("led_group_ant4_gpio", 21);	// ANT4 LED

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2 eth4 eth5");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth1 eth2 eth3 eth5");
				else if (nvram_match("wans_lanport", "5"))
					set_lan_phy("eth1 eth2 eth3 eth4");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4 eth5");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth7");
			if (get_wans_dualwan() == WANSCAP_USB)
				add_lan_phy("eth0");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "5"))
							sprintf(wan_if, "eth5");
						add_wan_phy(wan_if);

					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
//#ifdef RTCONFIG_EXTPHY_BCM84880
//		add_rc_support("2p5G_LWAN");
//#endif
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("no_jumbo_frame");
		/* Antenna LED */
		if(nvram_get_int("CoBrand")){
			if(nvram_get_int("CoBrand") == 3){
#ifdef GTAX6000
				add_rc_support("antled");
#endif
				f_write_string("/proc/nvram/set", "CoBrand=3", 0, 0);
			}
		}

		break;
#endif

#if defined(GTAX11000_PRO)
	case MODEL_GTAX11000_PRO:
		update_rf_para();
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("lan_ifname", "br0");
		reconfig_manual_wan_ifnames();
		nvram_set("wl_ifnames", "eth6 eth7 eth8");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7 eth8");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0 eth5"); // 2.5G, 10G
		nvram_set("amas_ethif_type", "8 32"); // 2.5G, 10G
		nvram_set("eth_priority", "0 1 1 1 2 1"); // eth0: 2.5G(idx:0,prio:1,used:1) > eth5: 10G(idx:1,prio:2,used:1)
		nvram_set("sta_priority", "2 0 5 1 5 1 4 0 5 2 3 1"); // 2.4G:(prio:5), 5G-1:(prio:4), 5G-2:(prio:3)
		nvram_set("sta_ifnames", "eth6 eth7 eth8");
		if(nvram_get_int("re_mode") == 1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
		else
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif

		nvram_set_int("led_pwr_gpio", 50|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_10g_white_gpio", 43);                        // 10G WHITE
		nvram_set_int("led_wan_normal_gpio", 52);
		nvram_set_int("led_wan_gpio", 44|GPIO_ACTIVE_LOW);              // WAN RED
		nvram_set_int("led_wan_green_gpio", 45|GPIO_ACTIVE_LOW);        // WAN GREEN
		nvram_set_int("led_wan_blue_gpio", 46|GPIO_ACTIVE_LOW);         // WAN BLUE
		nvram_set_int("led_10g_red_gpio", 47|GPIO_ACTIVE_LOW);		// 10G RED
		nvram_set_int("led_10g_green_gpio", 48|GPIO_ACTIVE_LOW);	// 10G GREEN
		nvram_set_int("led_10g_blue_gpio", 49|GPIO_ACTIVE_LOW);		// 10G BLUE
		nvram_set_int("led_wps_gpio", 53);
#if defined(RTCONFIG_TURBO_BTN)
		nvram_set_int("btn_turbo_gpio", 12|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_wps_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_red_gpio", 2);        // sw_parallel_led_2
		nvram_set_int("led_group1_green_gpio", 4);      // sw_parallel_led_4
		nvram_set_int("led_group1_blue_gpio", 3);       // sw_parallel_led_3
		nvram_set_int("led_group2_red_gpio", 5);        // sw_parallel_led_5
		nvram_set_int("led_group2_green_gpio", 8);      // sw_parallel_led_8
		nvram_set_int("led_group2_blue_gpio", 7);       // sw_parallel_led_7
		nvram_set_int("led_group3_red_gpio", 14);       // sw_parallel_led_14
		nvram_set_int("led_group3_green_gpio", 16);     // sw_parallel_led_16
		nvram_set_int("led_group3_blue_gpio", 15);      // sw_parallel_led_15
//		nvram_set_int("led_10g_red_gpio", 21|GPIO_ACTIVE_LOW);          // sw_parallel_led_21
//		nvram_set_int("led_10g_green_gpio", 22|GPIO_ACTIVE_LOW);        // sw_parallel_led_22
//		nvram_set_int("led_10g_blue_gpio", 23|GPIO_ACTIVE_LOW);         // sw_parallel_led_23

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		}

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX2");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
//#ifdef RTCONFIG_EXTPHY_BCM84880
//		add_rc_support("2p5G_LWAN");
//#endif
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("no_jumbo_frame");

		/* In order To Change CoBrand Model's Default LEDG Color At BootLoader Stage */
		if(nvram_get_int("CoBrand")){
			if(nvram_get_int("CoBrand") == 3){
#ifdef GTAX6000
				add_rc_support("antled");
#endif
				f_write_string("/proc/nvram/set", "CoBrand=3", 0, 0);
			}
		}
		break;
#endif

#if defined(GTAXE16000)
	case MODEL_GTAXE16000:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		nvram_set("dhd_rnr_offload_override", "7");
		nvram_set("lan_ifname", "br0");
		reconfig_manual_wan_ifnames();
		nvram_set("wl_ifnames", "eth7 eth8 eth9 eth10");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
		nvram_set("wl3_vifnames", "wl3.1 wl3.2 wl3.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth10 eth7 eth8 eth9");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0 eth5 eth6");
		nvram_set("amas_ethif_type", "8 32 32"); // 2.5G, 10G, 10G
		nvram_set("sta_ifnames", "eth10 eth7 eth8 eth9");
		if(nvram_get_int("re_mode") == 1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
		else
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
		nvram_set("eth_priority", "0 1 1 1 2 1 2 3 1"); // eth0: 2.5G(idx:0,prio:1,used:1) > eth5: 10G(idx:1,prio:2,used:1) > eth6: 10G(idx:2,prio:3,used:1)
		nvram_set("sta_priority", "5 1 6 0 5 2 4 1 6 3 5 1 2 0 7 1"); // 5G-1:(prio:6, used:0), 5G-2:(prio:4, used:1) 6G:(prio:5, used:1) 2.4G:(prio:7, used:1)
#endif

		nvram_set_int("led_pwr_gpio", 50|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_10g_white_gpio", 43);			// 10G WHITE
		nvram_set_int("led_wan_normal_gpio", 52);			// WAN WHITE
		nvram_set_int("led_wan_gpio", 44|GPIO_ACTIVE_LOW);		// WAN RED
		nvram_set_int("led_wan_green_gpio", 45|GPIO_ACTIVE_LOW);        // WAN GREEN
		nvram_set_int("led_wan_blue_gpio", 46|GPIO_ACTIVE_LOW);         // WAN BLUE
		nvram_set_int("led_10g_red_gpio", 47|GPIO_ACTIVE_LOW);		// 10G RED
		nvram_set_int("led_10g_green_gpio", 48|GPIO_ACTIVE_LOW);	// 10G GREEN
		nvram_set_int("led_10g_blue_gpio", 49|GPIO_ACTIVE_LOW);		// 10G BLUE

#if defined(RTCONFIG_TURBO_BTN)
		nvram_set_int("btn_turbo_gpio", 12|GPIO_ACTIVE_LOW);
#endif

		nvram_set_int("btn_wps_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_group1_red_gpio", 2);	// sw_parallel_led_2
		nvram_set_int("led_group1_green_gpio", 4);	// sw_parallel_led_4
		nvram_set_int("led_group1_blue_gpio", 3);	// sw_parallel_led_3
		nvram_set_int("led_group2_red_gpio", 5);	// sw_parallel_led_5
		nvram_set_int("led_group2_green_gpio", 8);	// sw_parallel_led_8
		nvram_set_int("led_group2_blue_gpio", 7);	// sw_parallel_led_7
		nvram_set_int("led_group3_red_gpio", 14);	// sw_parallel_led_14
		nvram_set_int("led_group3_green_gpio", 16);	// sw_parallel_led_16
		nvram_set_int("led_group3_blue_gpio", 15);	// sw_parallel_led_15

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		}

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX2");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
//#ifdef RTCONFIG_EXTPHY_BCM84880
//		add_rc_support("2p5G_LWAN");
//#endif
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		break;
#endif


#if defined(ET12) || defined(XT12)
	case MODEL_ET12:
	case MODEL_XT12:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		update_rf_para();
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth4 eth5 eth6");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
		nvram_set("sta_phy_ifnames", "eth4 eth5 eth6");

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0 eth3");
		nvram_set("amas_ethif_type", "8 8"); // 2.5G, 2.5G
		nvram_set("sta_ifnames", "eth4 eth5 eth6");
		nvram_set("wired_ifnames", "eth1 eth2 eth3");
		nvram_set("eth_priority", "0 1 1 1 2 1"); // eth0: 2.5G(idx:0,prio:1,used:1) > eth3: 2.5G(idx:1,prio:2,used:1)
#if defined(ET12)
		nvram_set("sta_priority", "2 0 5 1 5 1 4 1 6 2 3 1"); // 2.4G:(prio:5, used:1), 5G-1:(prio:4, used:1), 6G:(prio:3, used:1)
#endif
#if defined(XT12)
		nvram_set("sta_priority", "2 0 5 1 5 1 4 0 5 2 3 1"); // 2.4G:(prio:5, used:1), 5G-1:(prio:4, used:0), 5G-2:(prio:3, used:1)
#endif
#endif

		nvram_set_int("btn_wps_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 19|GPIO_ACTIVE_LOW);
		nvram_set_int("bt_rst_gpio", 3);
//		nvram_set_int("bt_disable_gpio", 5);
		// TOP LED E GROUP
		nvram_set_int("led_rgb1_red_gpio", 21);	// sw_led_21-pin_47
		nvram_set_int("led_rgb1_green_gpio", 17);	// sw_led_17-pin_43
		nvram_set_int("led_rgb1_blue_gpio", 16);	// sw_led_16-pin_42
		// TOP LED F GROUP
		nvram_set_int("led_rgb2_red_gpio", 18);	// sw_led_18-pin_44
		nvram_set_int("led_rgb2_green_gpio", 1);	// sw_led_01-pin_1
		nvram_set_int("led_rgb2_blue_gpio", 13);	// sw_led_13-pin_13
		// TOP LED G GROUP
		nvram_set_int("led_rgb3_red_gpio", 2);	// sw_led_02-pin_2
		nvram_set_int("led_rgb3_green_gpio", 14);	// sw_led_14-pin_40
		nvram_set_int("led_rgb3_blue_gpio", 15);	// sw_led_15-pin_41
		// SIDE LED GROUP (White LED only)
		nvram_set_int("led_side1_white_gpio", 23|GPIO_ACTIVE_LOW);	// sw_led_23-pin_49
		nvram_set_int("led_side2_white_gpio", 24|GPIO_ACTIVE_LOW);	// sw_led_24-pin_50
		nvram_set_int("led_side3_white_gpio", 22|GPIO_ACTIVE_LOW);	// sw_led_22-pin_48

		if(usb_usb3 == 1){
			nvram_set("xhci_ports", "2-2 2-1");
			nvram_set("ehci_ports", "3-2 3-1");
			nvram_set("ohci_ports", "4-2 4-1");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-2 1-1");
			nvram_set("ohci_ports", "2-2 2-1");
		}
#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2");
			}
			else
				set_lan_phy("eth1 eth2 eth3");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth4");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth5");
				add_lan_phy("eth6");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				char prefix[8];
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						add_wan_phy(wan_if);

					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth4");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth5");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("no_jumbo_frame");
		break;
#endif

#if defined(RTAX88U_PRO)
	case MODEL_RTAX88U_PRO:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
		int ext_phy_model = nvram_get_int("ext_phy_model");
#endif
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan_ifnames", "eth0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth6 eth7");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
			nvram_set("eth_ifnames", "eth0 eth5");
			nvram_set("amas_ethif_type", "8 8");
			nvram_set("eth_priority", "0 2 1 1 1 1");	// eth0: 2.5G(idx:0,prio:2,used:1) > eth5: 2.5G(idx:1,prio:1,used:1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
			nvram_set("sta_ifnames", "eth6 eth7");
			nvram_set("sta_priority", "2 0 4 1 5 1 3 1");	// 2.4G:(prio:4, used:1), 5G:(prio:3, used:1)
		} else {
			nvram_set("eth_ifnames", "eth0");
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
		}
#endif

		nvram_set_int("led_pwr_gpio", 50|GPIO_ACTIVE_LOW);	// PWR LED
		nvram_set_int("led_wan_gpio", 49|GPIO_ACTIVE_LOW);	// WAN RED LED
		nvram_set_int("led_wan_normal_gpio", 17|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 1|GPIO_ACTIVE_LOW);	// LAN LED
		nvram_set_int("led_wps_gpio", 48|GPIO_ACTIVE_LOW);	// WPS LED
		nvram_set_int("led_usb_gpio", 2|GPIO_ACTIVE_LOW);
		if(ext_phy_model == EXT_PHY_GPY211)
			nvram_set_int("led_extphy_gpio", 47|GPIO_ACTIVE_LOW);   // EXT LAN LED
		else
			nvram_set_int("led_extphy_gpio", 19|GPIO_ACTIVE_LOW); 	// EXT LAN LED

		nvram_set_int("btn_wps_gpio", 13|GPIO_ACTIVE_LOW);	// WPS BTN
		nvram_set_int("btn_rst_gpio", 15|GPIO_ACTIVE_LOW);	// RST BTN

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2 eth4 eth5");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth1 eth2 eth3 eth5");
				else if (nvram_match("wans_lanport", "5"))
					set_lan_phy("eth1 eth2 eth3 eth4");
			}
			else
				set_lan_phy("eth1 eth2 eth3 eth4 eth5");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth7");
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "5"))
							sprintf(wan_if, "eth5");
						add_wan_phy(wan_if);

					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("no_jumbo_frame");
		break;
#endif


#if defined(RTAX86U_PRO)
	case MODEL_RTAX86U_PRO:
#ifdef RTCONFIG_EXTPHY_BCM84880
		get_ext_phy_id();
#endif
		int wans_extwan = nvram_get_int("wans_extwan");
		nvram_set("lan_ifname", "br0");
		if (is_router_mode()) {
			if(wans_extwan){
				nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth0 eth6 eth7");
				nvram_set("wan_ifname", "eth5");
				nvram_set("wan_ifnames", "eth5");
			}
			else{
				nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7");
				nvram_set("wan_ifname", "eth0");
				nvram_set("wan_ifnames", "eth0");
			}
		} else {
			nvram_set("lan_ifnames", "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7");
			nvram_set("wan_ifnames", "");
			nvram_set("wan_ifname", "");
		}
		nvram_set("wl_ifnames", "eth6 eth7");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
			nvram_set("sta_phy_ifnames", "dpsta");
		}
		else
#endif
		{
			nvram_set("sta_phy_ifnames", "eth6 eth7");
		}

		if (nvram_get_int("re_mode") == 1) {
			nvram_set("wait_band", "0");
			nvram_set("wait_wifi", "0");
		}

		nvram_set("eth_ifnames", "eth0 eth5"); // 1G, 2.5G
		nvram_set("amas_ethif_type", "4 8"); // 1G, 2.5G
		nvram_set("sta_ifnames", "eth6 eth7"); // 2.4G, 5G
		nvram_set("sta_priority", "2 0 4 1 5 1 3 1"); // 2.4G:(prio:4), 5G-1:(prio:3)
		if(nvram_get_int("re_mode") == 1)
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4");
		else
			nvram_set("wired_ifnames", "eth1 eth2 eth3 eth4 eth5");
#endif

		nvram_set("0:ledbh7", "0x7");

		nvram_set_int("led_pwr_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 14|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LAN4WAN_LED
		nvram_set_int("led_lan1_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan2_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan3_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan4_gpio", 4|GPIO_ACTIVE_LOW);
#endif
		if(nvram_get_int("wans_extwan")){
			nvram_set_int("led_wan_gpio", 16|GPIO_ACTIVE_LOW);
			nvram_set_int("led_lan_gpio", 19|GPIO_ACTIVE_LOW); // WAN port is as LAN
			nvram_set_int("led_extphy_gpio", 15|GPIO_ACTIVE_LOW);
		}
		else
		{
			nvram_set_int("led_wan_gpio", 15|GPIO_ACTIVE_LOW);
			nvram_set_int("led_wan_normal_gpio", 19|GPIO_ACTIVE_LOW);
			nvram_set_int("led_extphy_gpio", 16|GPIO_ACTIVE_LOW);
		}
		nvram_set_int("btn_wps_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 13|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 10|GPIO_ACTIVE_LOW);
#endif

		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		} else {
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "0-1 0-2");
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_LAN) {
				if (nvram_match("wans_lanport", "1"))
					set_lan_phy("eth2 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "2"))
					set_lan_phy("eth1 eth3 eth4 eth5");
				else if (nvram_match("wans_lanport", "3"))
					set_lan_phy("eth1 eth2 eth4 eth5");
				else if (nvram_match("wans_lanport", "4"))
					set_lan_phy("eth1 eth2 eth3 eth5");
				else if (nvram_match("wans_lanport", "5"))
					set_lan_phy("eth1 eth2 eth3 eth4");
			}
			else if(wans_extwan)
				set_lan_phy("eth1 eth2 eth3 eth4");
			else
				set_lan_phy("eth1 eth2 eth3 eth4 eth5");

			if(nvram_match("wans_dualwan", "wan none") && wans_extwan)
				add_lan_phy("eth0");
			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth6");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth7");
			}
			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				char prefix[8];
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (nvram_match("wans_lanport", "1"))
							sprintf(wan_if, "eth1");
						else if (nvram_match("wans_lanport", "2"))
							sprintf(wan_if, "eth2");
						else if (nvram_match("wans_lanport", "3"))
							sprintf(wan_if, "eth3");
						else if (nvram_match("wans_lanport", "4"))
							sprintf(wan_if, "eth4");
						else if (nvram_match("wans_lanport", "5"))
							sprintf(wan_if, "eth5");
						add_wan_phy(wan_if);

					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth6");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth7");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								if(nvram_match("wans_dualwan", "wan none") && wans_extwan)
									add_wan_phy("eth5.v0");
								else
									add_wan_phy("eth0.v0");
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else
			nvram_unset("wan1_ifname");
#endif	// RTCONFIG_DUALWAN

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX2");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("11AX");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		add_rc_support("wifi2017");
		add_rc_support("2p5G_LWAN");
		add_rc_support("app");
		add_rc_support("ofdma");
#ifdef RTCONFIG_BRCM_HOSTAPD
		add_rc_support("wpa3");
#endif
		add_rc_support("no_jumbo_frame");
		break;
#endif

#ifdef RTCONFIG_BCM_7114
	case MODEL_RTAC5300:
		nvram_set("2:ledbh9", "0x7");
		nvram_set("wl2_vifnames", "wl2.1 wl2.2 wl2.3");
#if defined(RTAC5300)
		update_boardlimit_ac5300();
#endif
	case MODEL_RTAC88U:
	case MODEL_RTAC3100:
		ldo_patch();

		set_tcode_misc();
#if defined(RTAC88U) || defined(RTAC3100)
		//if(nvram_match("lazy_et", "1"))
			nvram_set("et_rxlazy_timeout",  "1000");
		//else
		//	nvram_set("et_rxlazy_timeout",  "300");
#endif

		nvram_set("0:ledbh9", "0x7");
		nvram_set("1:ledbh9", "0x7");
#ifdef RTCONFIG_RGMII_BRCM5301X
		hw_name = "et1";

		nvram_set("rgmii_port", "5");
#else
		nvram_unset("rgmii_port");
#endif
#ifdef RTCONFIG_GMAC3
		if (nvram_match("gmac3_enable", "1"))
			hw_name = "et2";
#endif
		if(model == MODEL_RTAC5300)
			nvram_set("landevs", "vlan1 wl0 wl1 wl2");
		else
			nvram_set("landevs", "vlan1 wl0 wl1");

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan2 vlan3");
#ifdef RTCONFIG_GMAC3
			else
				nvram_set("wandevs", hw_name);
#endif
			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");
			if (!(get_wans_dualwan()&WANSCAP_5G)) {
				add_lan_phy("eth2");
				if(model == MODEL_RTAC5300)
					add_lan_phy("eth3");
			}

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan3");
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								sprintf(wan_if, "vlan%d", wan_vid);
								add_wan_phy(wan_if);
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
#ifdef RTCONFIG_GMAC3
			nvram_set("wandevs", hw_name);
#endif
			if(model == MODEL_RTAC5300)
				nvram_set("lan_ifnames", "vlan1 eth1 eth2 eth3");
			else
				nvram_set("lan_ifnames", "vlan1 eth1 eth2");

			nvram_set("wan_ifnames", the_wan_phy());
			nvram_unset("wan1_ifname");
		}
#else
		if(model == MODEL_RTAC5300)
			nvram_set("lan_ifnames", "vlan1 eth1 eth2 eth3");
		else
			nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifnames", "eth0");
#endif
		if(model == MODEL_RTAC5300)
			nvram_set("wl_ifnames", "eth1 eth2 eth3");
		else
			nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
#if 0
		if(!nvram_match("ed_thresh_dft", "1")) {
			_dprintf("reset phy_ed_thresh.");
			if(!strncmp(nvram_safe_get("territory_code"), "EU", 2)) {
				nvram_set("0:eu_edthresh2g", "-16");
				nvram_set("1:eu_edthresh5g", "-16");
				if(model == MODEL_RTAC5300)
					nvram_set("2:ed_thresh5g", "-16");
			} else {
				nvram_set("0:ed_thresh2g", "-16");
				nvram_set("1:ed_thresh5g", "-16");
				if(model == MODEL_RTAC5300)
					nvram_set("2:ed_thresh5g", "-16");
			}
		}
#endif
#if defined(RTCONFIG_AMAS)
		if(model == MODEL_RTAC5300) {
#ifdef RTCONFIG_DPSTA
			if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
				nvram_set("sta_phy_ifnames", "dpsta");
			} else
#endif
			{
				nvram_set("sta_phy_ifnames", "eth1 eth2 eth3");
			}
			nvram_set("eth_ifnames", "vlan2");
			nvram_set("sta_ifnames", "eth1 eth2 eth3");
			if (nvram_get_int("re_mode") == 1) {
				nvram_set("lan_ifnames", "vlan1 eth1 eth2 eth3");
				if (enable_ETH_U(0) == 1)
					add_lan_phy(nvram_safe_get("eth_ifnames"));
			}
		} else {
#ifdef RTCONFIG_DPSTA
			if (dpsta_mode() && nvram_get_int("re_mode") == 1) {
				nvram_set("sta_phy_ifnames", "dpsta");
			} else
#endif
			{
				nvram_set("sta_phy_ifnames", "eth1 eth2");
			}
			nvram_set("eth_ifnames", "vlan2");
			nvram_set("sta_ifnames", "eth1 eth2");
			if (nvram_get_int("re_mode") == 1) {
				nvram_set("lan_ifnames", "vlan1 eth1 eth2");
				if (enable_ETH_U(0) == 1)
					add_lan_phy(nvram_safe_get("eth_ifnames"));
			}
		}
#endif
		/* gpio */
		/* HW reset, 2 | LOW */
		nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 4|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("led_wan_gpio", 5);
		/* MDC_4709 RGMII, 6 */
		/* MDIO_4709 RGMII, 7 */
		nvram_set_int("pwr_usb_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("reset_switch_gpio", 10);
		nvram_set_int("btn_rst_gpio", 11|GPIO_ACTIVE_LOW);
		/* UART1_RX,  12 */
		/* UART1_TX,  13 */
		/* UART1_CTS, 14*/
		/* UART1_RTS, 15 */
		nvram_set_int("led_usb_gpio", 16|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb3_gpio", 17|GPIO_ACTIVE_LOW);
		//nvram_set_int("led_mmc_gpio",  17|GPIO_ACTIVE_LOW);	/* abort */
		nvram_set_int("led_wps_gpio", 19|GPIO_ACTIVE_LOW);
		if(model == MODEL_RTAC5300) {
			nvram_set_int("btn_wltog_gpio", 20|GPIO_ACTIVE_LOW);
			nvram_set_int("btn_wps_gpio", 18|GPIO_ACTIVE_LOW);
			nvram_set_int("fan_gpio", 15);
			nvram_set_int("rpm_fan_gpio", 14|GPIO_ACTIVE_LOW);
		} else {
			nvram_set_int("btn_wltog_gpio", 18|GPIO_ACTIVE_LOW);
			nvram_set_int("btn_wps_gpio", 20|GPIO_ACTIVE_LOW);
		}
		nvram_set_int("led_lan_gpio", 21|GPIO_ACTIVE_LOW);	/* FAN CTRL: reserved */
		/* PA 5V/3.3V switch, 22 */
		/* SDIO_EN_1P8, 23 | HIGH */

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1");
		nvram_set("ehci_ports", "2-1 2-2");
		nvram_set("ohci_ports", "3-1 3-2");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "1-1");
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");
	
		nvram_set("ehci_irq", "111");
		nvram_set("xhci_irq", "112");
#ifdef RTCONFIG_MMC_LED
		nvram_set("mmc_irq", "177");
#endif
		if (!strncmp(nvram_safe_get("territory_code"), "AU/05", 5)) {
			add_rc_support("nz_isp");
			nvram_set("wifi_psk", cfe_nvram_safe_get_raw("secret_code"));
		}
		break;

#endif

#ifdef RTAC87U
	case MODEL_RTAC87U:
		if(nvram_match("QTNTELNETSRV","")) nvram_set("QTNTELNETSRV", "0");

		nvram_set("vlan1hwname", "et1");
		nvram_set("landevs", "vlan1 wl0");
		nvram_set("rgmii_port", "5");
#ifdef RTCONFIG_QTN
		nvram_set("qtn_ntp_ready", "0");
#endif

#ifdef RTCONFIG_HW_DUALWAN
		if (is_router_mode()) {
			init_dualwan();
		}
		else{
			nvram_set("wandevs", "et1");
			nvram_set("lan_ifnames", "vlan1 eth1 wifi0");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1 wifi0");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1 wifi0");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		/* WAN2 MAC = 3rd 5G GuestNetwork */
		if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN){
			nvram_set("wl1_vifnames", "wl1.1 wl1.2");
			nvram_set("wl1.3_bss_enabled", "0");
		}
		nvram_set("wl1_country_code", nvram_safe_get("1:ccode"));
		nvram_set("wl1_country_rev", nvram_safe_get("1:regrev"));
		nvram_set("wl1_ifname", "wifi0");
		nvram_set("wl1_phytype", "v");

#ifdef RTCONFIG_USBRESET
		nvram_set_int("pwr_usb_gpio", 9|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("pwr_usb_gpio", 9);
#endif
		nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("reset_qtn_gpio", 8|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 15|GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 4);	// active high
#endif

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1");
		nvram_set("ehci_ports", "2-1 2-2");
		nvram_set("ohci_ports", "3-1 3-2");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "1-1");
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");
#ifdef RTCONFIG_WPS_DUALBAND
	nvram_set_int("wps_band_x", 0);
	nvram_set_int("wps_dualband", 1);
#else
	nvram_set_int("wps_dualband", 0);
#endif
#if 0
		if (!x_Setting && nvram_get_int("ac87uopmode") == 5) {
			nvram_set("wl1_txbf", "0");
			nvram_set("wl1_itxbf", "0");
		}
#endif

		break;
#endif

#if defined(RTAC56S) || defined(RTAC56U)
	case MODEL_RTAC56S:
	case MODEL_RTAC56U:
		nvram_set("vlan1hwname", "et0");
		nvram_set("0:ledbh3", "0x87");	  /* since 163.42 */
		nvram_set("1:ledbh10", "0x87");
		nvram_set("landevs", "vlan1 wl0 wl1");

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan2 vlan3");
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth2");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan3");
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid) {
								sprintf(wan_if, "vlan%d", wan_vid);
								add_wan_phy(wan_if);
							}
							else
								add_wan_phy(the_wan_phy());
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy(the_wan_phy());
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan1 eth1 eth2");
			nvram_set("wan_ifnames", the_wan_phy());
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

#ifdef RTCONFIG_USBRESET
		nvram_set_int("pwr_usb_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("pwr_usb_gpio2", 10|GPIO_ACTIVE_LOW);	// Use at the first shipment of RT-AC56S,U.
#else
		nvram_set_int("pwr_usb_gpio", 9);
		nvram_set_int("pwr_usb_gpio2", 10);	// Use at the first shipment of RT-AC56S,U.
#endif
		nvram_set_int("led_usb_gpio", 14|GPIO_ACTIVE_LOW);	// change led gpio(usb2/usb3) to sync the outer case
		nvram_set_int("led_wan_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_all_gpio", 4|GPIO_ACTIVE_LOW);	// actually, this is high active, and will power off all led when active; to fake LOW_ACTIVE to sync boardapi
		nvram_set_int("led_5g_gpio", 6|GPIO_ACTIVE_LOW);	// 4352's fake led 5g
		nvram_set_int("led_usb3_gpio", 0|GPIO_ACTIVE_LOW);	// change led gpio(usb2/usb3) to sync the outer case
		nvram_set_int("btn_wps_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 11|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 7|GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1");
		nvram_set("ehci_ports", "2-1 2-2");
		nvram_set("ohci_ports", "3-1 3-2");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "1-1");
			nvram_set("ehci_ports", "2-1 2-2");
			nvram_set("ohci_ports", "3-1 3-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("movistarTriple");
		if(model == MODEL_RTAC56U){
			add_rc_support("app");
			add_rc_support("meoVoda");
		}
		break;
#endif

#if defined(RTN66U) || defined(RTAC66U)
	case MODEL_RTN66U:
		nvram_set_int("btn_radio_gpio", 13|GPIO_ACTIVE_LOW);	// ? should not used by Louis
		if (nvram_match("pci/1/1/ccode", "JP") && nvram_match("pci/1/1/regrev", "42") && !nvram_match("watchdog", "20000")) {
			_dprintf("force set watchdog as 20s\n");
			nvram_set("watchdog", "20000");
			nvram_commit();
			reboot(0);
		}

	case MODEL_RTAC66U:
		if (nvram_match("regulation_domain_5G", "EU") && !nvram_match("watchdog", "0")) {
			nvram_set("watchdog", "0");
			nvram_commit();
			reboot(0);
		}

#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan2 vlan3");
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth2");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan3");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								sprintf(wan_if, "vlan%d", wan_vid);
							else
								sprintf(wan_if, "eth0");
							add_wan_phy(wan_if);
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan1 eth1 eth2");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");

		nvram_set_int("btn_rst_gpio", 9|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_gpio", 13|GPIO_ACTIVE_LOW);	// tmp use
		nvram_set_int("led_pwr_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 15|GPIO_ACTIVE_LOW);
		nvram_set_int("fan_gpio", 14|GPIO_ACTIVE_LOW);
		nvram_set_int("have_fan_gpio", 6|GPIO_ACTIVE_LOW);
		if (nvram_get("regulation_domain")) {
			nvram_set("ehci_ports", "1-1.2 1-1.1 1-1.4");
			nvram_set("ohci_ports", "2-1.2 2-1.1 2-1.4");
		}
		else {
			nvram_set("ehci_ports", "1-1.2 1-1.1");
			nvram_set("ohci_ports", "2-1.2 2-1.1");
		}
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("app");
		break;
#endif

#ifdef RTAC53U
	case MODEL_RTAC53U:
#ifdef RTCONFIG_DUALWAN
		if (is_router_mode()) {
			if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN)
				nvram_set("wandevs", "vlan2 vlan3");
			else
				nvram_set("wandevs", "et0");

			set_lan_phy("vlan1");

			if (!(get_wans_dualwan()&WANSCAP_2G))
				add_lan_phy("eth1");
			if (!(get_wans_dualwan()&WANSCAP_5G))
				add_lan_phy("eth2");

			if (nvram_get("wans_dualwan")) {
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
					if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN) {
						if (get_wans_dualwan()&WANSCAP_WAN)
							add_wan_phy("vlan3");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("eth1");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("eth2");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
						if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
							int wan_vid = nvram_get_int("switch_wan0tagid");
							if (wan_vid)
								sprintf(wan_if, "vlan%d", wan_vid);
							else
								sprintf(wan_if, "eth0");
							add_wan_phy(wan_if);
						}
						else if (get_wans_dualwan()&WANSCAP_LAN)
							add_wan_phy("vlan2");
						else
							add_wan_phy("eth0");
					}
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else
				nvram_set("wan_ifnames", "eth0 usb");
		}
		else{
			nvram_set("wandevs", "et0");
			nvram_set("lan_ifnames", "vlan1 eth1 eth2");
			nvram_set("wan_ifnames", "eth0");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifnames", "eth0");
#endif
		nvram_set("wl_ifnames", "eth1 eth2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		nvram_set("sb/1/ledbh3", "0x87");
		nvram_set("0:ledbh9", "0x87");
		nvram_set_int("led_pwr_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 0|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 2|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 7|GPIO_ACTIVE_LOW);
		nvram_set_int("led_usb_gpio", 8|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio",22|GPIO_ACTIVE_LOW);
		nvram_set("ehci_ports", "1-1.4");
		nvram_set("ohci_ports", "2-1.4");
		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("2.4G 5G mssid usbX1 update");
		break;
#endif

#endif // CONFIG_BCMWL5
#ifdef RTCONFIG_REALTEK
	//case MODEL_RTRTL8198C:
#ifdef RPAC68U
	case MODEL_RPAC68U:
		//_dprintf("REALTEK nvram_set........\n");

		//USB Ports Init of RT-RTL8198C
		nvram_set("xhci_ports", "2-1"); // 2-1(at the first place) for USB-3.0 device on USB-3.0 HW Port
		nvram_set("ehci_ports", "1-1 3-1"); // 1-1(at the first place) for USB-2.0 device on USB-3.0 HW Port, 3-1(at the second place) for USB-2.0 device on USB-2.0 HW Port

		nvram_set("lan_ifname", "br0");
		if (is_router_mode())
			nvram_set("lan_ifnames", "eth0 wl0 wl1 eth2 eth3 eth4 wl0-vxd wl1-vxd wl0-wds0 wl1-wds0");
		else
			nvram_set("lan_ifnames", "eth0 wl0 wl1 eth1 eth2 eth3 eth4 wl0-vxd wl1-vxd wl0-wds0 wl1-wds0");
		nvram_set("wl0_ifname", "wl0");
		nvram_set("wl1_ifname", "wl1");
		if (is_router_mode())
			nvram_set("wan_ifnames", "eth1");
		else
			nvram_set("wan_ifnames", "");
		nvram_set("wl_ifnames","wl0 wl1");
		nvram_set("wl0_vifnames", "wl0.1");
		nvram_set("wl1_vifnames", "wl1.1");
		nvram_set("wl0_vxdifnames", "wl0-vxd");
		nvram_set("wl1_vxdifnames", "wl1-vxd");
		if(check_rtk_hw_invaild())
		{
			//cprintf("%s:%d check_rtk_hw_invaild\n",__FUNCTION__,__LINE__);
			set_rtk_hw_default();
		}
		set_nvram_mac_addr_from_flash();
		set_nvram_pincode_from_flash();
		nvram_set_int("btn_rst_gpio", 21|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 2|GPIO_ACTIVE_LOW);
		//_dprintf("sw_mode=%d\n", sw_mode());
/*#ifdef RTCONFIG_WIRELESSREPEATER
		if (is_router_mode())
			nvram_set_int("sw_mode", SW_MODE_REPEATER);
#endif*/
		nvram_commit();
		add_rc_support("2.4G 5G usbX1 update");
		add_rc_support("rtkwifi");
		_dprintf("sw_mode=%d\n",sw_mode());
		break;
#endif
#ifdef RPAC53
	case MODEL_RPAC53:

		nvram_set("lan_ifname", "br0");
		if (is_router_mode())
			nvram_set("lan_ifnames", "wl0 wl0-vxd wl1 wl1-vxd wl0-wds0 wl1-wds0");
		else if (access_point_mode())
			nvram_set("lan_ifnames", "eth1 wl0 wl0-vxd wl1 wl1-vxd wl0-wds0 wl1-wds0");
		else
			nvram_set("lan_ifnames", "eth1 wl0 wl0-vxd wl1 wl1-vxd");
		nvram_set("wl0_ifname", "wl0");
		nvram_set("wl1_ifname", "wl1");
		if (is_router_mode())
			nvram_set("wan_ifnames", "eth1");
		else
			nvram_set("wan_ifnames", "");
		nvram_set("wl_ifnames","wl0 wl1");
		nvram_set("wl0_vxdifnames", "wl0-vxd");
		nvram_set("wl1_vxdifnames", "wl1-vxd");
		nvram_set("wl0_vifnames", "wl0.1");
		nvram_set("wl1_vifnames", "wl1.1");
		if(check_rtk_hw_invaild())
		{
			set_rtk_hw_default();
		}
		set_nvram_mac_addr_from_flash();
		set_nvram_pincode_from_flash();

		nvram_set_int("btn_rst_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 12|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_red_gpio", 13|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_orange_gpio", 11|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_green_gpio", 10|GPIO_ACTIVE_LOW);
		nvram_set_int("led_2g_red_gpio", 5|GPIO_ACTIVE_LOW); // RTL8192ER
		nvram_set_int("led_5g_orange_gpio", 50|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_green_gpio", 51|GPIO_ACTIVE_LOW);
		nvram_set_int("led_5g_red_gpio", 24|GPIO_ACTIVE_LOW);
		nvram_set_int("led_lan_gpio", 14|GPIO_ACTIVE_LOW);

		nvram_commit();
		add_rc_support("2.4G 5G update");
		add_rc_support("rtkwifi");
		_dprintf("sw_mode=%d\n",sw_mode());
		break;
#endif
#ifdef RPAC55
	case MODEL_RPAC55:

		nvram_set("lan_ifname", "br0");
#ifdef RTCONFIG_AMAS
		if(repeater_mode() && !x_Setting)
		{
		// default status use eth0 as wan
            nvram_set("lan_ifnames", "wl0 wl0-vxd wl1 wl1-vxd eth0");
			nvram_set("wan_ifnames", "eth0");
		}
		else if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1"))
		{
			nvram_set("lan_ifnames", "wl0 wl0-vxd wl1 wl1-vxd eth0");
			nvram_set("eth_ifnames", "eth0");
			nvram_set("wan_ifnames", "eth0");
			nvram_set("sta_phy_ifnames", "wl0-vxd wl1-vxd");
			nvram_set("sta_ifnames", "wl0-vxd wl1-vxd");
		} else
#endif
		if (access_point_mode())
		{
			nvram_set("lan_ifnames", "wl0 wl0-vxd wl1 wl1-vxd wl0-wds0 wl1-wds0 eth0");
			nvram_set("wan_ifnames", "");
		}
		else
		{
			nvram_set("lan_ifnames", "wl0 wl0-vxd wl1 wl1-vxd eth0");
			nvram_set("wan_ifnames", "");
		}


        nvram_set("wl0_ifname", "wl0");
        nvram_set("wl1_ifname", "wl1");

		nvram_set("wl_ifnames","wl0 wl1");
		nvram_set("wl0_vxdifnames", "wl0-vxd");
		nvram_set("wl1_vxdifnames", "wl1-vxd");
		nvram_set("wl0_vifnames", "wl0.1");
		nvram_set("wl1_vifnames", "wl1.1");
		if(check_rtk_hw_invaild())
		{
			set_rtk_hw_default();
		}
		set_nvram_mac_addr_from_flash();
		set_nvram_pincode_from_flash();

		nvram_set_int("btn_rst_gpio", 54);
		nvram_set_int("btn_wps_gpio", 56|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 57);
		nvram_set_int("led_pwr_red_gpio", 24);
		nvram_set_int("led_wifi_gpio", 29);
		nvram_set_int("led_sig1_gpio", 31);
		nvram_set_int("led_sig2_gpio", 32);

		nvram_commit();
		add_rc_support("2.4G 5G update");
		add_rc_support("rtkwifi");
		_dprintf("sw_mode=%d\n",nvram_get_int("sw_mode"));
		break;
#endif
#ifdef RPAC92
	case MODEL_RPAC92:

		nvram_set("lan_ifname", "br0");
#ifdef RTCONFIG_AMAS
		if(repeater_mode() && !nvram_get_int("x_Setting")) {
			nvram_set("lan_ifnames", "wlan2 wlan0 wlan1 wlan2-vxd wlan0-vxd wlan1-vxd eth0.3");
			nvram_set("wan_ifnames", "eth0.3");
		}
		else if(sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1"))
		{
			nvram_set("lan_ifnames", "wlan2 wlan0 wlan1 wlan2-vxd wlan1-vxd wlan0-vxd eth0.3");
			nvram_set("eth_ifnames", "eth0.3");
			nvram_set("wan_ifnames", "eth0.3");
			nvram_set("sta_phy_ifnames", "wlan2-vxd wlan1-vxd wlan0-vxd");
			nvram_set("sta_ifnames", "wlan2-vxd wlan1-vxd wlan0-vxd");
		} else
#endif		
		if (nvram_get_int("sw_mode") == SW_MODE_ROUTER)
			nvram_set("lan_ifnames", "wlan2 wlan0 wlan1 eth0.3");
		else if (access_point_mode()){
			nvram_set("lan_ifnames", "wlan2 wlan0 wlan1 eth0.3");
			nvram_set("wan_ifnames", "");
		}
		else{
			nvram_set("lan_ifnames", "wlan2 wlan0 wlan1 wlan2-vxd wlan1-vxd wlan0-vxd eth0.3");
			nvram_set("wan_ifnames", "");
		}
		nvram_set("wl0_ifname", "wlan2");
		nvram_set("wl1_ifname", "wlan1");
		nvram_set("wl2_ifname", "wlan0");
		if (nvram_get_int("sw_mode") == SW_MODE_ROUTER)
			nvram_set("wan_ifnames", "eth0.3");
		nvram_set("wl_ifnames","wlan2 wlan1 wlan0");
		nvram_set("wl0_vxdifnames", "wlan2-vxd");
		nvram_set("wl1_vxdifnames", "wlan1-vxd");
		nvram_set("wl2_vxdifnames", "wlan0-vxd");
		nvram_set("wl0_vifnames", "wlan2.1");
		nvram_set("wl1_vifnames", "wlan1.1");
		nvram_set("wl2_vifnames", "wlan0.1");
		set_mac_to_nvram();
		set_nvram_pincode_from_flash();

		nvram_set_int("btn_rst_gpio", 34|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_wps_gpio", 37|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_gpio", 21|GPIO_ACTIVE_LOW);
		nvram_set_int("led_pwr_red_gpio", 6|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wifi_gpio", 12);
		nvram_set_int("led_sig1_gpio", 20|GPIO_ACTIVE_LOW);
		nvram_set_int("led_sig2_gpio", 62|GPIO_ACTIVE_LOW);
		nvram_set_int("led_purple_gpio", 10|GPIO_ACTIVE_LOW);

		nvram_commit();
		add_rc_support("2.4G 5G update");
		add_rc_support("rtkwifi");
		_dprintf("sw_mode=%d\n",nvram_get_int("sw_mode"));
		nvram_set_int("wlc_triBand", 0);
		nvram_set_int("wlc_down_5g", 0);
		nvram_set_int("wlc_down_5g2", 0);
		break;
#endif
#endif /* RTCONFIG_REALTEK */

#ifdef RTCONFIG_ALPINE
	case MODEL_GTAC9600:
		if(nvram_match("QTNTELNETSRV","")) nvram_set("QTNTELNETSRV", "0");

		nvram_set("lan_ifname", "br0");
		nvram_set("landevs", "eth3 eth0 eth2 host0");
		nvram_set("qtn_ntp_ready", "0");

#if defined(RTCONFIG_DUALWAN)
		if(is_router_mode()){
			set_lan_phy("eth3");

			// this platform uses host0 to communicate the WiFi interfaces
			add_lan_phy("host0");

			int LAN10G = 3; // 0: no, 1: 10G ETH, 2: 10G SFP+, 3: all.
			if(nvram_get("wans_dualwan")){
				set_wan_phy("");
				for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
					if(get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G)
						add_wan_phy("host0");
					else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G)
						add_wan_phy("host0");
					else if(get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN){
						add_wan_phy("eth1");
					}
					else if(get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN){
						// TODO: can use eth0 or eth2 as the WAN2.
						if(nvram_get_int("wans_lanport") == 2){
							LAN10G = 1;
							add_wan_phy("eth2");
						}
						else{ // wans_lanport == 1
							LAN10G = 2;
							add_wan_phy("eth0");
						}
					}
					else if(get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
						add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
					else if(get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
						add_wan_phy("usb2");
#endif
				}
			}
			else{
				nvram_set("wan_ifnames", "eth1");
			}

			if(LAN10G == 3){
				add_lan_phy("eth0");
				add_lan_phy("eth2");
			}
			else if(LAN10G == 2){
				add_lan_phy("eth2");
			}else if(LAN10G == 1)
				add_lan_phy("eth0");
		}
		else{
			nvram_set("lan_ifnames", "eth3 eth0 eth1 eth2 host0");
			nvram_set("wan_ifnames", "");
			nvram_unset("wan1_ifname");
		}
#else
		nvram_set("lan_ifnames", "eth3 eth2 host0");
		nvram_set("wan_ifnames", "eth1");
#endif

		nvram_set("wl_ifnames", "wifi2_0 wifi0_0");
		nvram_set("wl0_vifnames", "wifi2_1 wifi2_2 wifi2_3");
		nvram_set("wl1_vifnames", "wifi0_1 wifi0_2 wifi0_3");
		/* WAN2 MAC = 3rd 5G GuestNetwork */
		if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN){
			nvram_set("wl1_vifnames", "wl1.1 wl1.2");
			nvram_set("wl1.3_bss_enabled", "0");
		}
		// nvram_set("wl1_country_code", nvram_safe_get("1:ccode"));
		// nvram_set("wl1_country_rev", nvram_safe_get("1:regrev"));
		nvram_set("wl0_ifname", "wifi2_0");
		nvram_set("wl1_ifname", "wifi0_0");
		nvram_set("wl1_phytype", "v");

#ifdef RTCONFIG_USBRESET
		nvram_set_int("pwr_usb_gpio", 9|GPIO_ACTIVE_LOW);
#else
		nvram_set_int("pwr_usb_gpio", 9);
#endif
#if 0
		nvram_set_int("led_pwr_gpio", 3|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 5|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 1|GPIO_ACTIVE_LOW);
#endif
		nvram_set_int("btn_wps_gpio", 38|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 39|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_radio_gpio", 3|GPIO_ACTIVE_LOW);
#ifdef RTCONFIG_WIFI_TOG_BTN
		nvram_set_int("btn_wltog_gpio", 3|GPIO_ACTIVE_LOW);
#endif
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("btn_led_gpio", 22|GPIO_ACTIVE_LOW);
#endif

#ifdef RTCONFIG_XHCIMODE
		nvram_set("xhci_ports", "1-1 1-2");
		nvram_set("ehci_ports", "2-1 2-2");
		nvram_set("ohci_ports", "3-1 3-2");
#else
#if 1 // temp don't support to disable the USB 3.0.
		nvram_set("xhci_ports", "2-1 2-2");
		nvram_set("ehci_ports", "1-1 1-2");
		nvram_set("ohci_ports", "3-1 3-2");
#else
		if (usb_usb3 == 1) {
			nvram_set("xhci_ports", "2-1 2-2");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "3-1 3-2");
		}
		else{
			nvram_unset("xhci_ports");
			nvram_set("ehci_ports", "1-1 1-2");
			nvram_set("ohci_ports", "2-1 2-2");
		}
#endif
#endif

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G usbX2");
#ifdef RTCONFIG_MERLINUPDATE
		add_rc_support("update");
#else
		add_rc_support("noupdate");
#endif
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("app");
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("AllLED", 1);
#endif
#ifdef RTCONFIG_WPS_DUALBAND
	nvram_set_int("wps_band_x", 0);
	nvram_set_int("wps_dualband", 1);
#else
	nvram_set_int("wps_dualband", 0);
#endif
#if 0
		if (!x_Setting && nvram_get_int("ac87uopmode") == 5) {
			nvram_set("wl1_txbf", "0");
			nvram_set("wl1_itxbf", "0");
		}
#endif

		break;
#endif

#ifdef BLUECAVE
	case MODEL_BLUECAVE:
		_dprintf("BLUECAVE: todo, init_nvram()\n");
		nvram_set("wave_action", "0");
		nvram_set("lan_ifname", "br0");
		nvram_set("landevs", "eth0_1 eth0_2 eth0_3 eth0_4");
		nvram_set("ct_max", "5000");
		nvram_unset("aimesh_macacl_2g_mac");
		nvram_unset("aimesh_macacl_5g_mac");

#ifdef RTCONFIG_AMAS
		if (sw_mode() == SW_MODE_AP && nvram_match("re_mode", "1")) {
			nvram_set("lan_ifnames", "eth0_1 eth0_2 eth0_3 eth0_4 wlan0 wlan2 eth1");
			nvram_set("wan_ifnames", "");
		}
		else
#endif
		if (sw_mode()==SW_MODE_AP) {
			nvram_set("lan_ifnames", "eth0_1 eth0_2 eth0_3 eth0_4 wlan0 wlan2 eth1");
			nvram_set("wan_ifnames", "");
		}
		else {
			nvram_set("lan_ifnames", "eth0_1 eth0_2 eth0_3 eth0_4 wlan0 wlan2");
			nvram_set("wan_ifnames", "eth1");
		}
		nvram_set("wl_ifnames", "wlan0 wlan2");
		nvram_set("wl0_vifnames", "wl0.1 wl0.2 wl0.3");
		nvram_set("wl1_vifnames", "wl1.1 wl1.2 wl1.3");
		/* WAN2 MAC = 3rd 5G GuestNetwork */
		if (get_wans_dualwan()&WANSCAP_WAN && get_wans_dualwan()&WANSCAP_LAN){
			nvram_set("wl1_vifnames", "wl1.1 wl1.2");
			nvram_set("wl1.3_bss_enabled", "0");
		}
		// nvram_set("wl1_country_code", nvram_safe_get("1:ccode"));
		// nvram_set("wl1_country_rev", nvram_safe_get("1:regrev"));
		nvram_set("wl1_phytype", "v");

		char prefix[] = "wlXXXXXXXXXXXXX_", tmp[100];
		char prefix2[] = "wlXXXXXXXXXXXXX_", tmp2[100];
		int i, j, wlc_band = nvram_get_int("wlc_band");

		// only for the all SKIP_ABSENT_BAND().
		for(i = 0; i < MAX_NR_WL_IF; ++i){
			snprintf(prefix, sizeof(prefix), "wl%d_", i);

#ifdef RTCONFIG_WIRELESSREPEATER
#ifdef RTCONFIG_PROXYSTA
			if(mediabridge_mode())
				nvram_set(strcat_r(prefix, "ifname", tmp), "");
			else
#endif
			if(repeater_mode() && i == wlc_band)
				nvram_set(strcat_r(prefix, "ifname", tmp), get_staifname(i));
			else
#endif
#ifdef RTCONFIG_AMAS
			if(aimesh_re_mode())
				nvram_set(strcat_r(prefix, "ifname", tmp), get_staifname(i));
			else
#endif
				nvram_set(strcat_r(prefix, "ifname", tmp), get_wififname(i));

			//for(j = 0; j < MAX_NO_MSSID; ++j){
			for(j = 0; j < 1; ++j){
				snprintf(prefix2, sizeof(prefix2), "wl%d.%d_", i, j+1);

#ifdef RTCONFIG_WIRELESSREPEATER
				if(client_mode() && i == wlc_band && j == 0)
					nvram_set(strcat_r(prefix2, "ifname", tmp2), get_wififname(i));
#endif
#ifdef RTCONFIG_AMAS
				if(aimesh_re_mode())
					nvram_set(strcat_r(prefix2, "ifname", tmp), get_wififname(i));
#endif
				if(!nvram_get(strcat_r(prefix2, "nband", tmp2)) || strlen(nvram_safe_get(tmp2)) <= 0)
					nvram_set(tmp2, nvram_safe_get(strcat_r(prefix, "nband", tmp)));
			}
		}

#if 0
		nvram_set_int("led_pwr_gpio", 1|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wan_gpio", 4|GPIO_ACTIVE_LOW);
		nvram_set_int("led_wps_gpio", 6|GPIO_ACTIVE_LOW);
#endif

		nvram_set_int("led_ctl_sig1_gpio", 42);	// Level 1
		nvram_set_int("led_ctl_sig2_gpio", 8);	// Level 2
		nvram_set_int("led_ctl_sig3_gpio", 1);	// Level 3

		nvram_set_int("led_idr_sig1_gpio", 4);	// RED
		nvram_set_int("led_idr_sig2_gpio", 6);	// BLUE

		nvram_set_int("btn_wps_gpio", 30|GPIO_ACTIVE_LOW);
		nvram_set_int("btn_rst_gpio", 0|GPIO_ACTIVE_LOW);

		if(usb_usb3 == 1)
			nvram_set("xhci_ports", "2-1");
		else
			nvram_unset("xhci_ports");
		nvram_set("ehci_ports", "1-1");
		nvram_set("ohci_ports", "1-1");

		if (!nvram_get("ct_max"))
			nvram_set("ct_max", "300000");
		add_rc_support("mssid 2.4G 5G update usbX1");
		add_rc_support("switchctrl"); // broadcom: for jumbo frame only
		add_rc_support("manual_stb");
		add_rc_support("pwrctrl");
		add_rc_support("WIFI_LOGO");
		add_rc_support("nandflash");
		add_rc_support("meoVoda");
		add_rc_support("movistarTriple");
		add_rc_support("lantiq");
		add_rc_support("app");
#if defined(LANTIQ_BSD)
		add_rc_support("bandstr");
#endif
#ifdef RTCONFIG_LED_BTN
		nvram_set_int("AllLED", 1);
#endif
#ifdef RTCONFIG_WPS_DUALBAND
	nvram_set_int("wps_band_x", 0);
	nvram_set_int("wps_dualband", 1);
#else
	nvram_set_int("wps_dualband", 0);
#endif
#if 0
		if (!x_Setting && nvram_get_int("ac87uopmode") == 5) {
			nvram_set("wl1_txbf", "0");
			nvram_set("wl1_itxbf", "0");
		}
#endif

//#if 0
#ifdef RTCONFIG_AMAS
		if ( nvram_match("re_mode", "1") ) {
			_dprintf("[%s][%d] AMAS nvram testing init",
						__func__, __LINE__);
			_dprintf("[%s][%d] sw mode = %d, repeater=%d, ap= %d ",
						__func__, __LINE__,
						sw_mode(),SW_MODE_REPEATER,SW_MODE_AP);
			nvram_set("eth_ifnames", "eth1");
			nvram_set("sta_phy_ifnames", "wlan1 wlan3");
			nvram_set("sta_ifnames", "wlan1 wlan3");
			nvram_unset("dfschinfo");
		}
		add_led_ctrl_capability(CENTRAL_LED);
		nvram_set("wired_ifnames", "eth0_1 eth0_2 eth0_3 eth0_4");
#endif
		break;
#endif

	default:
		_dprintf("############################ unknown model(init.c:8512) #################################\n");
		break;
	}

#ifdef RTCONFIG_HSPOT
	update_hspotap_config();
#endif

#ifdef HND_ROUTER
	if (!nvram_get_int("ct_expect_max"))
		nvram_set_int("ct_expect_max", 150);
#endif

	init_subunit();

#if defined(RTCONFIG_AMAS)
#if defined(RTCONFIG_FRONTHAUL_DWB) || defined(RTCONFIG_MSSID_PRELINK) || defined(RTCONFIG_FRONTHAUL_DBG) || defined(RTCONFIG_VIF_ONBOARDING)
#if defined(RTCONFIG_FRONTHAUL_DWB)
	if (nvram_get_int("fh_ap_enabled") >= 0)
		nvram_set("fh_ap_bss", "0");  // Fronthaul AP is be control by cfg daemon. So disable it when booted.
#endif
	unset_selected_channel_info();
#endif

#if defined(RTCONFIG_CFGSYNC) && defined(RTCONFIG_LACP)
	if (nvram_get("wired_ifnames")) {
		if (nvram_match("lacp_enabled", "1")) {
			strlcpy(tmp_ifnames, nvram_safe_get("wired_ifnames"), sizeof(tmp_ifnames));
			snprintf(wired_ifnames, sizeof(wired_ifnames), "%s %s", tmp_ifnames, "bond0");
			nvram_set("wired_ifnames", wired_ifnames);
		}
	}
#endif
#ifdef RTCONFIG_BHCOST_OPT
#ifdef RTCONFIG_BCMWL6
	if (nvram_get_int("re_mode") == 1)
		amas_stop_acsd_config_init(model);
#endif
#endif
#endif

	if(nvram_match("wifison_ready", "1"))
	{
#ifdef RTCONFIG_WIFI_SON
		if (x_Setting==1 && sw_mode() == SW_MODE_ROUTER)
		{
			nvram_set("cfg_master", "1");
		}
		/* set DFS check time */
		if (nvram_match("wl1_country_code", "GB"))
			nvram_set("dfs_check_period","1");
		else
			nvram_set("dfs_check_period","0");
#else
		_dprintf("no wifison feature\n");
#endif
	}

#if !defined(RTCONFIG_DUALWAN) && \
    !(defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) || defined(SHAC1300) || defined(RTAC95U)) && \
    (defined(CONFIG_BCMWL5) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
	/* Broadcom, QCA, MTK (use MT7620/MT7621/MT7628 ESW) platform */
	if (nvram_get("switch_wantag") && !nvram_match("switch_wantag", "") && !nvram_match("switch_wantag", "none")) {
		char wan_if[10];
		int wan_vid = nvram_get_int("switch_wan0tagid");
		if (wan_vid) {
			sprintf(wan_if, "vlan%d", wan_vid);
			nvram_set("wan_ifnames", wan_if);
		}
#if !defined(RTCONFIG_RALINK) && !defined(RTCONFIG_QCA)
		else
			nvram_set("wan_ifnames", "eth0");
#endif
	}
#endif

#ifdef RTCONFIG_MULTICAST_IPTV
	if (nvram_get_int("switch_stb_x") > 6) {
#if !defined(HND_ROUTER) && !defined(BLUECAVE)
		if(nvram_match("switch_wantag", "singtel")) {
			nvram_set("iptv_wan_ifnames", "vlan30 vlan40");
		} else if(nvram_match("switch_wantag", "maxis_fiber_sp_iptv")) {
			nvram_set("iptv_wan_ifnames", "vlan15");
			nvram_set("iptv_ifname", "vlan15");
		} else if(nvram_match("switch_wantag", "maxis_fiber_iptv")) {
			nvram_set("iptv_wan_ifnames", "vlan823");
			nvram_set("iptv_ifname", "vlan823");
		} else if(nvram_match("switch_wantag", "movistar")) {
			nvram_set("iptv_wan_ifnames", "vlan2 vlan3");
			nvram_set("iptv_ifname", "vlan2");
		}
		else if(nvram_match("switch_wantag", "starhub")) {
			nvram_set("iptv_wan_ifnames", "vlan1091");
			nvram_set("iptv_ifname", "vlan1091");
		}
#elif defined(HND_ROUTER)
		if(nvram_match("switch_wantag", "movistar")) {
			nvram_set("iptv_wan_ifnames", "eth0.v1 eth0.v2");
			nvram_set("iptv_ifname", "eth0.v1");
		}
#elif defined(BLUECAVE)
		if(nvram_match("switch_wantag", "movistar")) {
			nvram_set("iptv_wan_ifnames", "eth1.2 eth1.3");
			nvram_set("iptv_ifname", "eth1.2");
		}
#endif
	}
#endif /* MULTICAST_IPTV */

#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	{
		char word[256], *next;
		char tmp[128], prefix[] = "wlXXXXXXXXXX_";

		unit = 0;
		foreach (word, nvram_safe_get("wl_ifnames"), next) {
#if defined(RTCONFIG_HIDDEN_BACKHAUL)
			if(!strcmp(word,WIF_5G_BH))
				continue;
#endif
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			nvram_set_int(strcat_r(prefix, "unit", tmp), unit);
			nvram_set(strcat_r(prefix, "ifname", tmp), word);

			unit++;
		}
	}

#if defined(RTCONFIG_LP5523)
	nvram_set_int("prelink_pap_status", -1);
	nvram_set("lp55xx_lp5523_col", "");
	nvram_set("lp55xx_lp5523_beh", "");
	if (nvram_match("lp55xx_lp5523_sch_enable", "2"))
		nvram_set_int("lp55xx_lp5523_sch_enable", 1);
#endif
#if defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
	nvram_set_int("prelink_pap_status", -1);
#endif
#if defined(RTCONFIG_REPEATER_STAALLBAND)
	if (sw_mode()==SW_MODE_REPEATER) {
		nvram_set_int("wlc_triBand", -1);
		nvram_set_int("wlc_triState", 0);
	}
#endif
#endif
#if defined(RTCONFIG_BT_CONN) && defined(RTCONFIG_PRELINK)
	if (!x_Setting) {
		nvram_set_int("ble_dut_con", 0);
		nvram_set_int("ble_rename_ssid", 0);
	}
#endif

#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_AMAS) && defined(RTCONFIG_BCN_RPT)
	{
		unit = 0;
		char word[256], *next;
		char tmp[16];
		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			if(nvram_match("re_mode", "1"))
				snprintf(tmp, sizeof(tmp), "wl%d.1_rrm", unit);
			else
				snprintf(tmp, sizeof(tmp), "wl%d_rrm", unit);
#if defined(RTCONFIG_NBR_RPT)
			nvram_set(tmp, "0x32");//beacon passive scan and active scan neighbor report
#else
			nvram_set(tmp, "0x30");//beacon passive scan and active scan
#endif
			unit++;
		}
	}
#endif

#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_AMAS) && defined(RTCONFIG_SW_CTRL_ALLLED)
	add_led_ctrl_capability(LED_ON_OFF);
#endif

#ifdef RTCONFIG_REBOOT_SCHEDULE
	add_rc_support("reboot_schedule");
	// tmp to add default nvram
	if(nvram_match("reboot_schedule_enable", ""))
		nvram_set("reboot_schedule_enable", "0");
	if(nvram_match("reboot_schedule", ""))
		nvram_set("reboot_schedule", "00000000000");
#endif

#ifdef RTCONFIG_IPV6
	add_rc_support("ipv6");
#ifdef RTCONFIG_6RELAYD
	add_rc_support("ipv6pt");
#endif
#ifdef RTCONFIG_SOFTWIRE46
	add_rc_support("s46");
#ifdef RTCONFIG_OCNVC
	add_rc_support("ocnvc");
#endif
#ifdef RTCONFIG_DSLITE
	add_rc_support("dslite");
#endif
#endif
#endif

#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_FANCTRL)
	if ( button_pressed(BTN_HAVE_FAN) )
		add_rc_support("fanctrl");
#endif

#ifdef RTCONFIG_PARENTALCTRL
	add_rc_support("PARENTAL2");
#endif

#ifdef RTCONFIG_TCODE
	/* add tcode-based features */
	config_tcode(0);
#endif

#ifdef RTCONFIG_DPSTA
	set_dpsta_ifnames();
#endif
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
	set_dpsr_ifnames();
#endif

#ifdef RTCONFIG_YANDEXDNS
#ifdef RTCONFIG_TCODE
	if (!nvram_contains_word("rc_support", "yadns") &&
	    !nvram_contains_word("rc_support", "yadns_hideqis"))
#endif
	{
		/* 0: disable, 1: full, -1: partial without qis */
		int yadns_support;

		switch (model) {
		case MODEL_RTN11P:
		case MODEL_RTN300:
			/* Enabled for CE & RU area. (but IN could also be included) */
			yadns_support = nvram_match("reg_spec", "CE") && nvram_match("wl_reg_2g", "2G_CH13");
			break;
		case MODEL_RTN12VP:
		case MODEL_RTN10P:
		case MODEL_RTN10PV2:
			/* Enabled unconditionally */
			yadns_support = 1;
			break;
		default:
#ifdef RTCONFIG_TCODE
		case MODEL_RTAC82U:
		case MODEL_RTAC58U:
		case MODEL_RT4GAC53U:
		case MODEL_MAPAC1300:
		case MODEL_MAPAC2200:
		case MODEL_VZWAC1300:
		case MODEL_SHAC1300:
		case MODEL_RTAC95U:
			/* Only enabled by territory_code */
			yadns_support = 0;
#else
			/* Enabled by default since YANDEXDNS=y */
			yadns_support = 1;
#endif
			break;
		}
		if (yadns_support == 0)
			nvram_set_int("yadns_enable_x", 0);
		else if (yadns_support < 0)
			add_rc_support("yadns_hideqis");
		else
			add_rc_support("yadns");
	}
#endif
	set_ethsw_ifnames();

#ifdef RTCONFIG_DNSFILTER
	add_rc_support("dnsfilter");
#endif

	add_rc_support("am_addons");
#ifdef HND_ROUTER
	add_rc_support("cake");
#endif

#ifdef RTCONFIG_NTPD
	add_rc_support("ntpd");
#endif

#ifdef RTCONFIG_DNSPRIVACY
	add_rc_support("dnspriv");
#endif

#ifdef RTCONFIG_DSL
	set_wanscap_support("dsl");
#elif defined(RTCONFIG_NO_WANPORT) || defined(RTCONFIG_NoETH)
	set_wanscap_support("");
#else
	set_wanscap_support("wan");
#endif

#ifdef DSL_AX82U
	add_wanscap_support("wan");
#endif

#ifdef RTCONFIG_DUALWAN // RTCONFIG_DUALWAN
	add_rc_support("dualwan");

#ifdef RTCONFIG_WANPORT2
#if defined(GTAXY16000) || defined(RTAX89U)
	if (is_aqr_phy_exist())
		add_wanscap_support("wan2");
#else
	add_wanscap_support("wan2");
#endif
#endif

#ifdef RTCONFIG_USB_MODEM
#ifdef RTAC68U
	if (hw_usb_cap())
#endif
	add_wanscap_support("usb");
#endif
#ifdef RTCONFIG_SFPP
	add_wanscap_support("sfp+");
#endif
#if defined(RTCONFIG_SWITCH_QCA8075_PHY_AQR107) \
 || defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
	/* LAN as WAN is not supported yet. */
#else
	add_wanscap_support("lan");
#endif
#endif // RTCONFIG_DUALWAN

#ifdef RTCONFIG_MULTIWAN_CFG
	add_rc_support("mtwancfg");
#endif

#if defined(RTCONFIG_PPTPD) || defined(RTCONFIG_ACCEL_PPTPD)
	add_rc_support("pptpd");
#endif

#ifdef RTCONFIG_OPENVPN
	add_rc_support("openvpnd");
	//nvram_set("vpnc_proto", "disable");
#ifdef RTCONFIG_TPVPN
	add_rc_support("hma");
#endif
#endif

#ifdef RTCONFIG_UTF8_SSID
	add_rc_support("utf8_ssid");
#endif

#ifdef RTCONFIG_USB
#ifdef RTAC68U
	if (!hw_usb_cap())
		goto NO_USB_CAP;
#endif
#ifdef RTCONFIG_USB_PRINTER
	add_rc_support("printer");
#endif

#ifdef RTCONFIG_USB_MODEM
#if !defined(RTAC1200)
	// TODO: hide USB Modem UI in 3.0.0.1
	if (strcmp(nvram_safe_get("firmver"), "3.0.0.1")!=0)
		add_rc_support("modem");
#endif

#ifdef RTCONFIG_USB_BECEEM
	add_rc_support("wimax");
#endif

#ifdef RTCONFIG_USB_SMS_MODEM
	add_rc_support("usbsms");
#endif

#ifdef RTCONFIG_USB_MULTIMODEM
	add_rc_support("multimodem");
#endif

#ifdef RTCONFIG_MODEM_BRIDGE
	add_rc_support("modembridge");
#endif
#endif

#ifdef RTCONFIG_WEBDAV
	add_rc_support("webdav");
#endif

#ifdef RTCONFIG_CLOUDSYNC
	add_rc_support("rrsut");	//syn server
	add_rc_support("cloudsync");

	char ss_support_value[1024]="\0";

#ifdef RTCONFIG_CLOUDSYNC
	strcat(ss_support_value, "asuswebstorage ");
#endif

#ifdef RTCONFIG_SWEBDAVCLIENT
	strcat(ss_support_value, "webdav ");
#endif

#ifdef RTCONFIG_DROPBOXCLIENT
	strcat(ss_support_value, "dropbox ");
#endif

#ifdef RTCONFIG_FTPCLIENT
	strcat(ss_support_value, "ftp ");
#endif

#ifdef RTCONFIG_SAMBACLIENT
	strcat(ss_support_value, "samba ");
#endif

#ifdef RTCONFIG_USBCLIENT
	strcat(ss_support_value, "usb ");
#endif

#ifdef RTCONFIG_FLICKRCLIENT
	strcat(ss_support_value, "flickr ");
#endif

	nvram_set("ss_support", ss_support_value);
#endif

#ifdef RTCONFIG_MEDIA_SERVER
	add_rc_support("media");
#endif

#ifdef RTCONFIG_APP_PREINSTALLED
	add_rc_support("appbase");
#endif

#ifdef RTCONFIG_APP_NETINSTALLED
	add_rc_support("appnet");

// DSL-N55U will follow N56U to have DM
//#ifdef RTCONFIG_RALINK
//#ifdef RTCONFIG_DSL
//	if (model==MODEL_DSLN55U)
//		add_rc_support("nodm");
//#endif
//#endif
#endif

#ifdef RTCONFIG_APP_FILEFLEX
	add_rc_support("fileflex");
#endif

#ifdef RTCONFIG_TIMEMACHINE
	add_rc_support("timemachine");
#endif

#ifdef RTCONFIG_WIGIG
	add_rc_support("wigig");
#endif

#ifdef RTCONFIG_HD_SPINDOWN
	add_rc_support("hdspindown");
#endif

#ifdef RTCONFIG_DISK_MONITOR
	add_rc_support("diskutility");
#endif

#ifdef RTCONFIG_NFS
	add_rc_support("nfsd");
#endif

#ifdef RTCONFIG_IGD2
	add_rc_support("igd2");
#endif
#ifdef RTCONFIG_DNSSEC
        add_rc_support("dnssec");
#endif

#ifdef RTCONFIG_DNSSEC
        add_rc_support("dnssec");
#endif

#ifdef RTCONFIG_OPENPLUS_TFAT
#if defined(RTAC58U)
	if(!strncmp(nvram_safe_get("odmpid"), "RT-ACRH", 7))
		nvram_set("usb_fatfs_mod", "open");
	else
		nvram_set("usb_fatfs_mod", "tuxera");
#else
	if(fs_coexist() == 1)
		nvram_set("usb_fatfs_mod", "open");
	else
		nvram_set("usb_fatfs_mod", "tuxera");
#endif
#elif defined(RTCONFIG_TFAT)
	nvram_set("usb_fatfs_mod", "tuxera");
#else
	nvram_set("usb_fatfs_mod", "open");
#endif

#ifdef RTCONFIG_NTFS
	nvram_set("usb_ntfs_mod", "");
#if defined(RTCONFIG_OPENPLUSPARAGON_NTFS) || defined(RTCONFIG_OPENPLUSTUXERA_NTFS)
#if defined(RTAC58U)
	if(!strncmp(nvram_safe_get("odmpid"), "RT-ACRH", 7))
		nvram_set("usb_ntfs_mod", "open");
	else{
		add_rc_support("sparse");
		nvram_set("usb_ntfs_mod", "tuxera");
	}
#else
	if(fs_coexist() == 1)
		nvram_set("usb_ntfs_mod", "open");
	else
		nvram_set("usb_ntfs_mod", "tuxera");
#endif
#else
#ifdef RTCONFIG_OPEN_NTFS3G
	nvram_set("usb_ntfs_mod", "open");
#elif defined(RTCONFIG_PARAGON_NTFS)
	add_rc_support("sparse");
	nvram_set("usb_ntfs_mod", "paragon");
#elif defined(RTCONFIG_TUXERA_NTFS)
	nvram_set("usb_ntfs_mod", "tuxera");
#endif
#endif
#endif // RTCONFIG_NTFS

#ifdef RTCONFIG_HFS
	nvram_set("usb_hfs_mod", "");
#if defined(RTCONFIG_OPENPLUSPARAGON_HFS) || defined(RTCONFIG_OPENPLUSTUXERA_HFS)
#if defined(RTAC58U)
	if(!strncmp(nvram_safe_get("odmpid"), "RT-ACRH", 7))
		nvram_set("usb_hfs_mod", "open");
	else
		nvram_set("usb_hfs_mod", "tuxera");
#else
	if(fs_coexist() == 1)
		nvram_set("usb_hfs_mod", "open");
	else
		nvram_set("usb_hfs_mod", "tuxera");
#endif
#else
#ifdef RTCONFIG_KERNEL_HFSPLUS
	nvram_set("usb_hfs_mod", "open");
#elif defined(RTCONFIG_PARAGON_HFS)
	nvram_set("usb_hfs_mod", "paragon");
#elif defined(RTCONFIG_TUXERA_HFS)
	nvram_set("usb_hfs_mod", "tuxera");
#endif
#endif
#endif // RTCONFIG_HFS

#ifdef RTCONFIG_APFS
	nvram_set("usb_apfs_mod", "");
#if defined(RTCONFIG_TUXERA_APFS)
	nvram_set("usb_apfs_mod", "tuxera");
#endif
#endif // RTCONFIG_APFS

	if(!nvram_get_int("enable_samba_manual")){
#ifdef RTCONFIG_TUXERA_SMBD
		nvram_set("enable_samba_tuxera", "1");
#else
		nvram_set("enable_samba_tuxera", "0");
#endif
	}
#ifdef RTAC68U
NO_USB_CAP:
#endif
#endif // RTCONFIG_USB

#ifdef RTCONFIG_FRS_FEEDBACK
//	add_rc_support("frs_feedback");
#ifdef RTCONFIG_DBLOG
	add_rc_support("dblog");
#endif /* RTCONFIG_DBLOG */
#endif

#ifdef RTCONFIG_PUSH_EMAIL
	add_rc_support("email");
#endif

#ifdef RTCONFIG_ISP_METER
	add_rc_support("ispmeter");
#endif

#ifdef RTCONFIG_FINDASUS
	add_rc_support("findasus");
#endif

#ifdef RTCONFIG_AIR_TIME_FAIRNESS
	add_rc_support("atf");
#endif

#ifdef RTCONFIG_POWER_SAVE
	add_rc_support("pwrsave");
#endif

#ifdef RTCONFIG_PAGECACHE_RATIO
	add_rc_support("pcache_ratio");
#endif

#ifdef RTCONFIG_HAS_5G_2
	add_rc_support("5G-2");
#else
	if (nvram_get("wl2_nband") != NULL)
		add_rc_support("5G-2");
#endif

#if defined(RTCONFIG_BWDPI)
#ifdef RTAC68U
	if (!is_n66u_v2())
#elif defined(DSL_AX82U)
	if (!is_ax5400_i1())
#endif
	add_rc_support("bwdpi");

	/* modify logic for AiProtection switch */
	// DON'T USE the logic of nvram_match, it's the wrong logic in this case!!
	// 1. when wrs_protect_enable == "", set to 1
	// 2. when wrs_protect_enable == 0, not to change this value
	if (!strcmp(nvram_safe_get("wrs_protect_enable"), ""))
	{
		if (nvram_get_int("wrs_mals_enable") || nvram_get_int("wrs_cc_enable") ||  nvram_get_int("wrs_vp_enable"))
			nvram_set("wrs_protect_enable", "1");
		else
			nvram_set("wrs_protect_enable", "0");
	}

	// wrs - white and black list
	add_rc_support("wrs_wbl");
#endif

#ifdef RTCONFIG_TRAFFIC_LIMITER
	add_rc_support("traffic_limiter");
#endif

#if defined(RTCONFIG_OOKLA)
	add_rc_support("ookla");
#endif

#if defined(RTCONFIG_OOKLA_LITE)
	add_rc_support("ookla_lite");
#endif

#ifdef RTCONFIG_SNMPD
	add_rc_support("snmp");
#endif

#ifdef RTCONFIG_TOR
	add_rc_support("tor");
#endif

#ifdef RTCONFIG_HTTPS
	add_rc_support("HTTPS");

	/* workaround : openssl self-signed certificate from old firmware version */
	// force to enable https_crt_save to store certificate
	if (nvram_get_int("https_crt_save") == 0) {
		nvram_set_int("https_crt_save", 1);
	}

	/* remove nvram https_crt_file */
	if (nvram_safe_get("https_crt_file")) {
		nvram_unset("https_crt_file");
	}
#ifdef RTCONFIG_LETSENCRYPT
	add_rc_support("letsencrypt");
#endif
#endif

#ifdef RTCONFIG_SSH
	add_rc_support("ssh");
#endif

#ifdef RTCONFIG_VPNC
	add_rc_support("vpnc");
#endif

#ifdef RTCONFIG_VPN_FUSION
	add_rc_support("vpn_fusion");
#endif

#ifdef RTCONFIG_WIRELESSREPEATER
#ifndef RTCONFIG_DISABLE_REPEATER_UI
	add_rc_support("repeater");
#endif
#endif
#ifdef RTCONFIG_PROXYSTA
#ifndef RTCONFIG_DISABLE_PROXYSTA_UI
	add_rc_support("psta");
#endif
#endif
#if defined(RTCONFIG_VHT80_80)
	add_rc_support("vht80_80");
#endif
#if defined(RTCONFIG_VHT160)
	add_rc_support("vht160");
#endif
#ifdef RTCONFIG_BCMWL6
	add_rc_support("wl6");
#endif
#if defined(RTN66U) || defined(RTAC66U) || defined(RTAC68U) || defined(DSL_AC68U) || defined(RTAC68U_V4) || defined(RTCONFIG_QCA)
#ifdef RTCONFIG_OPTIMIZE_XBOX
	add_rc_support("optimize_xbox");
#endif
#endif
#ifdef RTCONFIG_SFP
	add_rc_support("sfp");
#endif
#ifdef RTCONFIG_4M_SFP
	add_rc_support("sfp4m");
#endif
#ifdef RTCONFIG_8M_SFP
	add_rc_support("sfp8m");
#endif
	//add_rc_support("ruisp");
#ifdef RTCONFIG_WIFI_TOG_BTN
	add_rc_support("wifi_tog_btn");
#endif

#ifdef RTCONFIG_WPSMULTIBAND
	add_rc_support("wps_multiband");
#endif

#if (defined(RTCONFIG_USER_LOW_RSSI) || defined(RTCONFIG_NEW_USER_LOW_RSSI))
	add_rc_support("user_low_rssi");
#endif

#ifdef RTCONFIG_BCMFA
	add_rc_support("bcmfa");
#endif

#ifdef RTCONFIG_ROG_UI
	add_rc_support("rog");
#endif

#ifdef RTCONFIG_VISUALIZATION
	add_rc_support("wifiradar");
#endif

#ifdef RTCONFIG_TCODE
	add_rc_support("tcode");
#endif

#ifdef RTCONFIG_JFFS2USERICON
	add_rc_support("usericon");
#endif

#ifdef RTCONFIG_TR069
#if defined(DSL_AX82U)
	if (0)
#endif
	add_rc_support("tr069");

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2)
	add_rc_support("jffs2");
#endif
#endif

#if defined(RTCONFIG_WPS_ALLLED_BTN) || (!defined(RTCONFIG_LED_BTN) && !defined(RTCONFIG_WIFI_TOG_BTN) && !defined(RTCONFIG_QCA))
	add_rc_support("cfg_wps_btn");
#endif

#ifdef RTCONFIG_STAINFO
	add_rc_support("stainfo");
#endif

#ifdef RTCONFIG_CLOUDCHECK
	add_rc_support("cloudcheck");
#endif

#ifdef RTCONFIG_GETREALIP
	add_rc_support("realip");
#endif
#ifdef RTCONFIG_LACP
	add_rc_support("lacp");
#endif
#ifdef RTCONFIG_BONDING_WAN
	add_rc_support("wanbonding");
#endif
#ifdef RTCONFIG_KEY_GUARD
	add_rc_support("keyGuard");
#endif
#if !defined(RTCONFIG_SSID_AMAPS) /* AMAPS default is open system */
	if(strcmp(nvram_safe_get("wifi_psk"), "") != 0)
		add_rc_support("defpsk");
#endif /* RTCONFIG_SSID_AMAPS */

#if defined(RTAC1200)
	add_rc_support("noaidisk nodm noftp");
#endif

#if defined(RTAC1200V2)
	add_rc_support("noaidisk nodm noftp");
#endif

#ifdef RTCONFIG_WTFAST
	add_rc_support("wtfast");
#endif
#ifdef RTCONFIG_IFTTT
	add_rc_support("ifttt");
#endif
#ifdef RTCONFIG_ALEXA
	add_rc_support("alexa");
#endif
#ifdef RTCONFIG_WIFILOGO
	add_rc_support("wifilogo");
#endif
#ifdef RTCONFIG_IPSEC
#ifdef RTCONFIG_IPSEC_SERVER
	add_rc_support("ipsec_srv");
#endif
#ifdef RTCONFIG_IPSEC_CLIENT
	add_rc_support("ipsec_cli");
#endif
#endif
#if defined(RTCONFIG_CAPTIVE_PORTAL) && !defined(RTCONFIG_CP_FREEWIFI) && !defined(RTCONFIG_CP_ADVANCED)
	add_rc_support("cp_freewifi cp_advanced captivePortal");
#elif defined(RTCONFIG_CP_FREEWIFI) && defined(RTCONFIG_CAPTIVE_PORTAL)
	add_rc_support("cp_freewifi captivePortal");
#elif defined(RTCONFIG_CP_ADVANCED) && defined(RTCONFIG_CAPTIVE_PORTAL)
	add_rc_support("cp_advanced captivePortal");
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
	add_rc_support("gobi");
#endif

#ifdef RTCONFIG_PORT_BASED_VLAN
	add_rc_support("vlan");
#endif
#ifdef RTCONFIG_TAGGED_BASED_VLAN
	add_rc_support("tagged_based_vlan");
#endif
#ifdef RTCONFIG_MUMIMO
	add_rc_support("mumimo");
#endif
#ifdef RTCONFIG_MUMIMO_5G
	add_rc_support("mumimo_5g");
#endif
#ifdef RTCONFIG_MUMIMO_2G
	add_rc_support("mumimo_2g");
#endif
#ifdef RTCONFIG_QAM256_2G
	add_rc_support("qam256_2g");
#endif
#ifdef RTCONFIG_NOIPTV
	add_rc_support("noiptv");
#endif
#ifdef RTCONFIG_IMPROXY
	add_rc_support("improxy");
#endif
#ifdef RTCONFIG_NOTIFICATION_CENTER
	//add_rc_support("nt_center");	//UI hide
#endif
#ifdef RTCONFIG_NETOOL
	add_rc_support("netool");
#endif
#ifdef RTCONFIG_DISABLE_NETWORKMAP
	add_rc_support("disable_nwmd");
#endif
#ifdef RTCONFIG_PERMISSION_MANAGEMENT
	add_rc_support("permission_management");
#endif

#ifdef RTCONFIG_FBWIFI
	add_rc_support("fbwifi");
#endif
#ifdef RTCONFIG_CFGSYNC
	add_rc_support("cfg_sync");
#endif

#if (defined(RTAC3200) || defined(HND_ROUTER))
	add_rc_support("no_finiwl");
#endif

#ifdef RTCONFIG_FORCE_AUTO_UPGRADE
	add_rc_support("fupgrade");
#endif

#ifdef RTCONFIG_AUTO_FW_UPGRADE
        add_rc_support("afwupg");
#endif

#ifdef RTCONFIG_AMAS
	add_rc_support("amas");
	if (nvram_get_int("amas_bdl"))
	add_rc_support("amas_bdl");
#endif

#ifdef RTCONFIG_WIFI_PROXY
	add_rc_support("wifiproxy");
#endif

#ifdef CONFIG_BCMWL5
	add_rc_support("bcmwifi");
#endif

#ifdef RTCONFIG_DFS_US
	add_rc_support("dfs");
#endif

#ifdef RTCONFIG_LYRA_HIDE
	add_rc_support("lyra_hide");
#endif

#ifdef RTCONFIG_PORT2_DEVICE
	add_rc_support("port2_device");
#endif

#if defined(RTCONFIG_RGBLED)
	add_rc_support("aura_rgb");
#endif

#if defined(RTCONFIG_AURASYNC)
	add_rc_support("aura_sync");
#endif

#if defined(RTCONFIG_TURBO_BTN)
	add_rc_support("boostkey");
#endif

#ifdef RTCONFIG_TCODE
	/* del tcode-based features after everything is set above
	 * this is the last action, do not insert anything after */
	config_tcode(2);
	config_tcode(3); /* del rc_support by odmpid */
#endif

#if defined(RTCONFIG_HND_ROUTER)
	add_rc_support("bcmhnd");
#endif

#if defined(RTCONFIG_HND_ROUTER_AX)
	add_rc_support("mbo");
#endif

#ifdef RTCONFIG_CONNDIAG
	add_rc_support("conndiag");
	if (nvram_invmatch("enable_diag", "2"))
		nvram_set("enable_diag", "2");
	if (nvram_invmatch("diag_local_data", "1"))
		nvram_set("diag_local_data", "1");
	if (nvram_invmatch("diag_interval", "60"))
		nvram_set("diag_interval", "60");
#endif

	add_rc_support("eula");

#ifdef RTCONFIG_PROXYSTA
	add_rc_support("proxysta");
#endif

#ifdef RTCONFIG_QCA
	add_rc_support("qca");
#endif

#ifdef RTCONFIG_GEFORCENOW
	add_rc_support("nvgfn");
#endif

#ifdef RTCONFIG_IPERF3
	add_rc_support("iperf3");
#endif

#ifdef RTCONFIG_GN_WBL
	add_rc_support("gn_wbl");
#ifdef RTCONFIG_AMAZON_WSS
	add_rc_support("amazon_wss"); // depends on gn_wbl
#endif
#endif

#ifdef RTCONFIG_MULTISERVICE_WAN
	add_rc_support("mswan");
#endif

#ifdef RTCONFIG_PIPEFW
	add_rc_support("pipefw");
#endif

#ifdef RTCONFIG_WIFI6E
	add_rc_support("wifi6e");
#endif

#ifdef RTCONFIG_ACCOUNT_BINDING
	add_rc_support("account_binding");
#endif

#ifdef RTCONFIG_URLFW
	add_rc_support("urlfw");
#endif

#ifdef RTCONFIG_GAME_MODE
	add_rc_support("gameMode");
#endif

#ifdef RTCONFIG_REDIRECT_DNAME
	if (sw_mode() == SW_MODE_REPEATER || sw_mode() == SW_MODE_AP)
		add_rc_support("redirect_dname");
#endif

#ifdef RTCONFIG_OWE_TRANS
	add_rc_support("owe_trans");
#endif

#ifdef RTCONFIG_WIREGUARD
//	add_rc_support("wireguard");
#ifdef RTCONFIG_TPVPN
	add_rc_support("nordvpn");
#endif
#endif

#ifdef RTCONFIG_FTP_SSL
	add_rc_support("ftp_ssl");
#endif

#ifdef RTCONFIG_ACL96
	add_rc_support("acl96");
#endif

#ifdef RTCONFIG_QCA_PLC2
	add_rc_support("qca_plc2");
#endif

#if defined(RTCONFIG_HND_ROUTER_AX_675X)
	add_rc_support("hnd_ax_675x");
#endif

#ifdef RTCONFIG_HND_ROUTER
#ifdef RTCONFIG_DBLOG
	add_rc_support("dhdlog");
#endif /* RTCONFIG_DBLOG */
#endif /* RTCONFIG_HND_ROUTER */

#if defined(RTCONFIG_ASUSCTRL)
	if (nvram_get_int("SG_mode")==1) {
		add_rc_support("noFwManual");
	}
#endif

#if defined(RTCONFIG_ASUSCTRL) && defined(RTCONFIG_QCA)
	if (nvram_get_int("EG_mode")==1 && nvram_match("wl_country_code", "GB")) {
		add_rc_support("loclist");
		add_rc_support("pwrctrl");
	}
#endif
#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_MT798X) \
 || (defined(RTCONFIG_QCA) && (defined(RTCONFIG_SOC_IPQ40XX) || defined(RTCONFIG_SOC_IPQ50XX) \
			    || defined(RTCONFIG_SOC_IPQ60XX) || defined(RTCONFIG_SOC_IPQ8074)))
	/* QCA: SPF5+ */
	add_rc_support("dis11b");
#endif

#ifdef RTCONFIG_COMFW
	nvram_set_int("comfw_type", get_cf_id(model, RT_BUILD_NAME));
#endif
#ifdef RTCONFIG_SECUREBOOT
	nvram_set("build_name", RT_BUILD_NAME_SEC);
#else
	nvram_set("build_name", RT_BUILD_NAME);
#endif

#if defined(RTCONFIG_BCMBSD)
	add_rc_support("smart_connect");
#endif
#if defined(RTCONFIG_BCMBSD_V2)
	del_rc_support("smart_connect");
	add_rc_support("smart_connect_v2"); 
#endif
	if(!nvram_match("forget_it", ""))
		add_rc_support("defpass");

	return 0;
}  // end of init_nvram

int init_nvram2(void)
{
	unsigned char ea[ETHER_ADDR_LEN];
	char hostname[32];
	char ver[64];
	int i;
	int dhcpc_mode = nvram_get_int("dhcpc_mode");
	char varname[20];

	if (nvram_match("x_Setting", "0")) {
		snprintf(ver, sizeof(ver), "%s.%s_%s", nvram_safe_get("firmver"), nvram_safe_get("buildno"), nvram_safe_get("extendno"));
		nvram_set("innerver", ver);
	}

	/* set default lan_hostname */
	if (NEED_TO_SET_DEFAULT_VALUE("lan_hostname") || !nvram_invmatch("lan_hostname", "")) {
		ether_atoe(get_2g_hwaddr(), ea);
#ifdef RTAC1200GP
		snprintf(hostname, sizeof(hostname), "%s-%02X%02X", "RT-AC1200G", ea[4], ea[5]);
#else
		snprintf(hostname, sizeof(hostname), "%s-%02X%02X", get_productid(), ea[4], ea[5]);
#endif
		if (!restore_defaults_g && !nvram_invmatch("computer_name", "")) {
			/* migrate from computer_name on fw upgrade */
			strlcpy(hostname, nvram_safe_get("computer_name"), sizeof(hostname));
		}
		nvram_set("lan_hostname", hostname);
		nvram_commit();
	}

#if defined(RTAC85U) || defined(RTAC85P) || defined(RTACRH26) || defined(TUFAC1750) || defined(RPAX56) || defined(RPAX58)
	int model = get_model();
	switch(model)
	{
	case MODEL_RTAC85U:
	case MODEL_RTAC85P:
	case MODEL_RTACRH26:
	case MODEL_TUFAC1750:
#ifdef RTCONFIG_USB_SWAP
		if(nvram_get_int("apps_swap_threshold") == 0) {
			nvram_set("apps_swap_enable", "1");
			nvram_set("apps_swap_threshold", "75000");
			nvram_set("apps_swap_size", "62000");
		}
#endif
#if defined(RPAX56) || defined(RPAX58)
	case MODEL_RPAX56:
	case MODEL_RPAX58:
		if(!nvram_match("x_Setting", "1")) {
			_dprintf("%s, rp(def) set wan_ifname here\n", __func__);
			nvram_set("wan_ifname", "eth0");
			nvram_set("wan0_ifname", "eth0");
		}
#endif
		break;
	default:
		_dprintf("############################ unknown model(init.c:9292) #################################\n");
		break;
	}
#endif

#ifdef RTCONFIG_OPENVPN
/* Initialize OpenVPN state flags */

	for (i = 1; i <= OVPN_CLIENT_MAX; i++) {
		sprintf(varname, "vpn_client%d_state", i);
		nvram_set(varname, "0");

		sprintf(varname, "vpn_client%d_errno", i);
		nvram_set(varname, "0");
	}

	nvram_set("vpn_server1_state", "0");
	nvram_set("vpn_server2_state", "0");
	nvram_set("vpn_server1_errno", "0");
	nvram_set("vpn_server2_errno", "0");
	nvram_set("vpn_upload_state", "");
#endif

/* Remove potentially outdated data */
	nvram_unset("webs_state_info");
	nvram_unset("webs_state_info_am");
	nvram_set("webs_state_flag","0");

#if defined(RTCONFIG_CONCURRENTREPEATER)
	if (sw_mode() == SW_MODE_REPEATER) {
		if (nvram_get_int("wlc_express") == 0) {
			nvram_set_int("wlc_band", -1);
#ifdef RTCONFIG_REALTEK
			nvram_set("wlc_ssid", "");
#endif
		}
		else {
			if (nvram_get_int("wlc_express") == 1) {
				nvram_set_int("wlc_band", 0);
				nvram_set_int("wlc1_state", 0);
#ifdef RTCONFIG_REALTEK
				nvram_set("wlc_ssid", nvram_safe_get("wlc1_ssid"));
#endif
			}
#if defined(RTCONFIG_HAS_5G)
			else if (nvram_get_int("wlc_express") == 2) {
				nvram_set_int("wlc_band", 1);
				nvram_set_int("wlc0_state", 0);
#ifdef RTCONFIG_REALTEK
				nvram_set("wlc_ssid", nvram_safe_get("wlc1_ssid"));
#endif
			}
#endif
			//nvram_unset("wlc_band");
		}
	}
#ifdef RTCONFIG_REALTEK
	/* For Realtek Media Bridge Mode condition */
	else if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1) {
		nvram_set_int("wlc_band", -1);
		nvram_set("wlc_ssid", "");
	}
#endif

	nvram_set_int("led_status", LED_BOOTING);
#endif // RTCONFIG_CONCURRENTREPEATER

#ifndef RTCONFIG_SW_CTRL_ALLLED
#ifdef RTCONFIG_WPS_ALLLED_BTN
	nvram_set_int("AllLED", 1);
#endif
#endif
#ifdef RTCONFIG_WIFI_SON
	nvram_set("wsplcd_uptime", "0");
	nvram_set("hyd_uptime", "0");
	nvram_set("cap_syncing","0");
	nvram_set("re_syncing","0");
	nvram_set("re_asuscmd","0");
#if defined(RTCONFIG_AUTHSUPP)
	nvram_set("now_security","0");
	nvram_set("cap_security_old","0");
	nvram_set("re_security_new","0");
	nvram_set("re_security_ext","0");
#endif
	nvram_unset("lyra_re_dist");
	nvram_set("spcmd","0");
#ifdef RTCONFIG_ETHBACKHAUL
	nvram_unset("chaos_eth_daemon");
#endif
	nvram_set("wps_band_x","1");
//debug
	nvram_set("hive_dbg","0");
#if defined(MAPAC1300) || defined(MAPAC2200) || defined(VZWAC1300) || defined(SHAC1300) || defined(RTAC95U) /* for NAND flash based Lyra */
	nvram_set("log_size","1024");
#elif defined(MAPAC1750)
	nvram_set("log_size","256"); // overwrite old value (if user do not restore to default)
#endif
//
#endif /* RTCONFIG_WIFI_SON */
#ifdef RTCONFIG_CFGSYNC
	nvram_set("cfg_cost", "0");
	nvram_unset("cfg_alive");
	nvram_unset("cfg_masterip");
	nvram_set("cfg_rejoin", "0");
	if (!is_router_mode())
		nvram_unset("cfg_cost");
#ifdef RTCONFIG_AMAS
	if (nvram_get_int("re_mode") == 1) {
		nvram_set_int("cfg_rejoin", 1);
		nvram_set_int("amas_path_stat", -1);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
		nvram_set("aurargb_val", "255,0,0,5,0,0");
#endif
#ifndef RTCONFIG_BHCOST_OPT
		if (enable_ETH_U(0) == 0)
			nvram_set("eth_ifnames", "");
#endif
		nvram_unset("cfg_first_sync");
		if (strlen(nvram_safe_get("cfg_group")) == 0)
			nvram_set("cfg_first_sync", "1");
#ifdef RTCONFIG_BHCOST_OPT
		nvram_unset("cfg_level");
		nvram_unset("cfg_maxlevel");
#endif
	}
	else
	{
		nvram_set_int("cfg_re_maxnum", MAX_RELIST_COUNT);
	}
	if (nvram_get("CoBrand") != NULL) {
		amas_set_misc_info(MISC_INFO_COBRAND, nvram_safe_get("CoBrand"));
	}
#endif /* AMAS */
#endif /* CFGSYNC */
#if defined(RTCONFIG_WIFI_DRV_DISABLE) /* for IPQ40XX */
	if (nvram_match("disableWifiDrv_fac", "1"))
		nvram_set("lyra_disable_wifi_drv", "1");
	nvram_unset("disableWifiDrv_fac");
#endif

#ifdef RTCONFIG_DWB
	void dwb_init_settings(void);
	dwb_init_settings();
#endif

#ifdef RTCONFIG_AMAS_WGN
	wgn_check_settings();
#endif	/* RTCONFIG_AMAS_WGN */	

#ifdef CONFIG_BCMWL5
	if (nvram_get_int("CoBrand") == 1)
		add_rc_support("GD");
#endif

	nvram_set("label_mac", get_label_mac());

#ifdef RTCONFIG_FRS_FEEDBACK
	/* reset the counter "fb_req_cnt" to "0" */
	nvram_set("fb_req_cnt", "0");
#endif /* RTCONFIG_FRS_FEEDBACK */

#if defined(RTCONFIG_BWDPI)
	if(!nvram_match("extendno", nvram_safe_get("extendno_org"))){
		adjust_62_nv_list("bwdpi_game_list");
		adjust_62_nv_list("bwdpi_stream_list");
		adjust_62_nv_list("bwdpi_wfh_list");
	}
#endif

	// upgrade/downgrade dont keep info
	if(!nvram_match("extendno", nvram_safe_get("extendno_org"))){
		nvram_set("mfp_ip_requeue", "");
		nvram_unset("webs_state_update");
		nvram_unset("webs_state_upgrade");
		nvram_unset("webs_state_info");
		nvram_unset("webs_state_REQinfo");
		nvram_unset("webs_state_url");
		nvram_unset("webs_state_flag");
		nvram_unset("webs_state_error");
#if defined(RTAC68U) || defined(RTCONFIG_FORCE_AUTO_UPGRADE)
		nvram_set_int("auto_upgrade", 0);
#endif
	}

	/// TODO: unset old dhcpc_mode once all UI drop it.
	if (dhcpc_mode != 1) { //non default
		nvram_set("dhcpc_mode", "1");
		nvram_set_int("wan0_dhcp_qry", dhcpc_mode);
		nvram_set_int("wan1_dhcp_qry", dhcpc_mode);
#ifdef RTCONFIG_DSL
		nvram_set_int("dsl0_dhcp_qry", dhcpc_mode);
		nvram_set_int("dsl8_dhcp_qry", dhcpc_mode);
#endif
	}

#if defined(GTAXE11000)
	if(nvram_match("x_Setting", "0") || is_passwd_default())
		reset_button_state();
#endif
	nvram_unset("slowloris_ip_list");
#if RTCONFIG_ASUSCTRL
	nvram_set("webs_chg_sku", "0");
	nvram_set("webs_SG_mode", "0");
#endif

#ifdef RTCONFIG_IPV6
	if (strlen(nvram_safe_get("ipv6_ula_random")) == 0) {
		uint8_t s[5] = {0};
		char prefix[32] = {0};
		f_read("/dev/urandom", s, 5);
		snprintf(prefix, sizeof(prefix), "fd%02x:%02x%02x:%02x%02x::", s[0], s[1], s[2], s[3], s[4]);
		nvram_set("ipv6_ula_random", prefix);
	}
#endif
#ifdef RTCONFIG_OPENVPN
	ovpn_defaults();
#endif

#if defined(RTCONFIG_IFTTT) || defined(RTCONFIG_ALEXA) || defined(RTCONFIG_GOOGLE_ASST)
	nvram_set("ifttt_stoken", "");
	nvram_set("ifttt_timestamp", "");
#endif

#ifdef RTCONFIG_FRS_LIVE_UPDATE
#ifdef RTCONFIG_AHS
	init_ahs_bhc_params();
#endif /* RTCONFIG_AHS */
#endif /* RTCONFIG_FRS_LIVE_UPDATE */
	return 0;
}

#if defined(RTCONFIG_SOC_IPQ40XX)
int init_nvram3(void)
{

	int model = get_model();
	switch(model)
	{
	case MODEL_RTAC82U:
		/* enable bled */
		config_netdev_bled("led_wan_gpio", "eth0");

#ifdef RTCONFIG_LAN4WAN_LED
		config_swports_bled_sleep("led_lan1_gpio", 1);
		config_swports_bled_sleep("led_lan2_gpio", 2);
		config_swports_bled_sleep("led_lan3_gpio", 4);
		config_swports_bled_sleep("led_lan4_gpio", 8);
#endif

		config_netdev_bled("led_2g_gpio", "ath0");

		config_netdev_bled("led_5g_gpio", "ath1");

		//config_usbbus_bled("led_usb_gpio", "3 4");
		//config_usbbus_bled("led_usb3_gpio", "1 2");
		break;

	case MODEL_RTAC58U:
		/* enable bled */
		config_netdev_bled("led_wan_gpio", "eth0");
#if 0
		config_swports_bled_sleep("led_lan_gpio", 0);
#else  //workaround for lan led
		config_netdev_bled("led_lan_gpio", "eth1");
#endif

		config_netdev_bled("led_2g_gpio", "ath0");

		config_netdev_bled("led_5g_gpio", "ath1");

		config_usbbus_bled("led_usb_gpio", "3 4");
		config_usbbus_bled("led_usb3_gpio", "1 2");
		break;

	case MODEL_RT4GAC53U:
		/* enable bled */
		config_netdev_bled("led_lan_gpio", "eth1");
		config_netdev_bled("led_2g_gpio", "ath0");
		config_netdev_bled("led_5g_gpio", "ath1");
		config_usbbus_bled("led_usb_gpio", "3 4");
		config_usbbus_bled("led_usb3_gpio", "1 2");
		break;

	case MODEL_MAPAC2200:
		/* modify PSGMII parameter that suggested from ODM */
		doSystem("devmem 0x98288 32 0x8280");
	case MODEL_MAPAC1300:
	case MODEL_VZWAC1300:
	case MODEL_SHAC1300:
	case MODEL_RTAC95U:
		/* TBD */
		break;

	default:
		_dprintf("############################ unknown model(init.c:9480) #################################\n");
		break;
	}

	return 0;
}
#endif

void force_free_caches()
{
#ifdef RTCONFIG_RALINK
	int model = get_model();

	if (model != MODEL_RTN65U)
		free_caches(FREE_MEM_PAGE, 2, 0);
#elif defined(RTCONFIG_QCA)
	/* do nothing */
#elif defined(RTCONFIG_REALTEK)
	int model = get_model();
	if (/*model != MODEL_RTRTL8198C &&*/ model != MODEL_RPAC68U)
		free_caches(FREE_MEM_PAGE, 2, 0);
#else
	int model = get_model();

	if (model==MODEL_RTN53||model==MODEL_RTN10D1) {
		free_caches(FREE_MEM_PAGE, 2, 0);
	}
#endif

#ifdef RTCONFIG_BCMARM
	f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
#endif
}

#ifdef RTN65U
/* Set PCIe ASPM policy */
int set_pcie_aspm(void)
{
	const char *aspm_policy = "powersave";

	if (nvram_get_int("aspm_disable"))
		aspm_policy = "performance";

	return f_write_string("/sys/module/pcie_aspm/parameters/policy", aspm_policy, 0, 0);
}
#endif //RTN65U

int limit_page_cache_ratio(int ratio)
{
	int min = 5;
	char p[] = "100XXX";

	if (ratio > 100 || ratio < min)
		return -1;

	if (ratio < 90) {
#if defined(RTCONFIG_OPEN_NTFS3G) && (LINUX_KERNEL_VERSION < KERNEL_VERSION(3,0,0))
		min = 90;
#endif

		if (get_meminfo_item("MemTotal") <= 64)
			min = 90;
	}

	if (ratio < min)
		ratio = min;

	sprintf(p, "%d", ratio);
	return f_write_string("/proc/sys/vm/pagecache_ratio", p, 0, 0);
}

#if defined(RTCONFIG_DUAL_TRX2)
static int fixdmgfw(void)
{
	int i, act_fw = 0, nact_fw;
	struct {
		int mtd_id, mtd_size;
		char fwpart[sizeof("linux2XXX")];
		char mtdpart[sizeof("/dev/mtdblockXYYYYYY")];
	} fw[2] = {{ 0 }}, *p;

	for (i = 0, p = &fw[0]; i < ARRAY_SIZE(fw); ++i, ++p) {
		if (i != 0)
			snprintf(p->fwpart, sizeof(p->fwpart), "linux%d", i + 1);
		else
			strlcpy(p->fwpart, "linux", sizeof(p->fwpart));
		if (mtd_getinfo(p->fwpart, &p->mtd_id, &p->mtd_size) != 1) {
			dbg("%s: can't get mtd info of %s\n", __func__, p->fwpart);
			return 1;
		}

		snprintf(p->mtdpart, sizeof(p->mtdpart), "/dev/mtdblock%d", p->mtd_id);
	}

	/* Find inactive firmware MTD partition. */
	act_fw = get_active_fw_num();
	if (act_fw >= ARRAY_SIZE(fw)) {
		dbg("%s: get_active_fw_num() return invalid firmware id (%d)!\n",
			__func__, act_fw);
		act_fw = 0;
	}
	nact_fw = act_fw ^ 1;

	/* Verify inactive firmware. */
	if (!checkcrc(fw[nact_fw].mtdpart)) {
		dbg("%s: Veirfy inactive firmware in %s: OK\n", __func__, fw[nact_fw].mtdpart);
		return 0;
	}

	/* Program it if damaged. */
	eval("mtd-write", "-i", fw[act_fw].mtdpart, "-d", fw[nact_fw].fwpart);

	return 0;
}

int fixdmgfw_main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp(argv[1], "-Y"))
		fixdmgfw();
	return 0;
}
#endif

#ifdef RTCONFIG_BCMFA
#if 0
static int
build_ifnames(char *type, char *names, int *size)
{
	char name[32], *next;
	int len = 0;
	int s;

	/* open a raw scoket for ioctl */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	foreach(name, type, next) {
		struct ifreq ifr;
		int i, unit;
		char var[32], *mac;
		unsigned char ea[ETHER_ADDR_LEN];

		/* vlan: add it to interface name list */
		if (!strncmp(name, "vlan", 4)) {
			/* append interface name to list */
			len += snprintf(&names[len], *size - len, "%s ", name);
			continue;
		}

		/* others: proceed only when rules are met */
		for (i = 1; i <= DEV_NUMIFS; i ++) {
			/* ignore i/f that is not ethernet */
			ifr.ifr_ifindex = i;
			if (ioctl(s, SIOCGIFNAME, &ifr))
				continue;
			if (ioctl(s, SIOCGIFHWADDR, &ifr))
				continue;
			if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
				continue;
			if (!strncmp(ifr.ifr_name, "vlan", 4))
				continue;
			/* wl: use unit # to identify wl */
			if (!strncmp(name, "wl", 2)) {
				if (wl_probe(ifr.ifr_name) ||
				    wl_ioctl(ifr.ifr_name, WLC_GET_INSTANCE, &unit, sizeof(unit)) ||
				    unit != atoi(&name[2]))
					continue;
			}
			/* et/il: use mac addr to identify et/il */
			else if (!strncmp(name, "et", 2) || !strncmp(name, "il", 2)) {
				snprintf(var, sizeof(var), "%smacaddr", name);
				if (!(mac = nvram_get(var)) || !ether_atoe(mac, ea) ||
				    bcmp(ea, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
					continue;
			}
			/* mac address: compare value */
			else if (ether_atoe(name, ea) &&
				!bcmp(ea, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
				;
			/* others: ignore */
			else
				continue;
			/* append interface name to list */
			len += snprintf(&names[len], *size - len, "%s ", ifr.ifr_name);
		}
	}

	close(s);

	*size = len;
	return 0;
}
#endif

/* Override the "0 5u" to "0 5" to backward compatible with old image */
static void
fa_override_vlan2ports()
{
	char port[] = "XXXX", *nvalue;
	char *next, *cur, *ports, *u;
	int len;

	ports = nvram_get("vlan2ports");
	nvalue = malloc(strlen(ports) + 2);
	if (!nvalue) {
		_dprintf("Memory allocate failed!\n");
		return;
	}
	memset(nvalue, 0, strlen(ports) + 2);

	/* search last port include 'u' */
	for (cur = ports; cur; cur = next) {
		/* tokenize the port list */
		while (*cur == ' ')
			cur ++;
		next = strstr(cur, " ");
		len = next ? next - cur : strlen(cur);
		if (!len)
			break;
		if (len > sizeof(port) - 1)
			len = sizeof(port) - 1;
		strncpy(port, cur, len);
		port[len] = 0;

		/* prepare new value */
		if ((u = strchr(port, 'u')))
			*u = '\0';
		strcat(nvalue, port);
		strcat(nvalue, " ");
	}

	/* Remove last " " */
	len = strlen(nvalue);
	if (len) {
		nvalue[len-1] = '\0';
		nvram_set("vlan2ports", nvalue);
	}
}

void
fa_mode_adjust()
{
	char *wan_proto;
	char buf[16];
	int sw_mode = sw_mode();
	int bhdr = 0;

	if (sw_mode == SW_MODE_ROUTER
#ifdef RTCONFIG_RGMII_BCM_FA
		|| sw_mode == SW_MODE_AP
#endif
	) {
		if (!nvram_match("ctf_disable_force", "1")
			&& nvram_get_int("ctf_fa_cap")
#if !defined(HND_ROUTER)
			&& !nvram_get_int("cstats_enable")
#endif
			&& !nvram_match("gmac3_enable", "1")
			&& !nvram_get_int("qos_enable")
			&& nvram_match("x_Setting", "1")
		) {
			bhdr = 1;
			nvram_set_int("ctf_fa_mode", CTF_FA_NORMAL);
		} else {
			nvram_set_int("ctf_fa_mode", CTF_FA_DISABLED);
		}
	} else { /* repeater mode */
		nvram_set_int("ctf_fa_mode", CTF_FA_DISABLED);
	}

	if (sw_mode == SW_MODE_ROUTER) {
		snprintf(buf, 16, "%s", nvram_safe_get("wans_dualwan"));
		if (!strcmp(buf, "wan none") || !strcmp(buf, "wan usb"))
			wan_proto = nvram_safe_get("wan0_proto");
		else // for (usb wan).
			wan_proto = nvram_safe_get("wan1_proto");

		if ((strcmp(wan_proto, "dhcp") && strcmp(wan_proto, "static"))
#if defined(RTCONFIG_BWDPI)
			|| check_bwdpi_nvram_setting()
#endif
		) {
			nvram_set_int("ctf_fa_mode", CTF_FA_DISABLED);
			_dprintf("wan_proto:%s not support FA mode...\n", wan_proto);
		}

		if (!nvram_match("switch_wantag", "none") && !nvram_match("switch_wantag", "")) {
			nvram_set_int("ctf_fa_mode", CTF_FA_DISABLED);
			dbG("IPTV environment, disable FA mode\n");
		}
	}

#ifdef RTCONFIG_DSL	//TODO
	nvram_set_int("ctf_fa_mode", CTF_FA_DISABLED);
#elif defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	if (vlan_enable())
		nvram_set_int("ctf_fa_mode", CTF_FA_DISABLED);
#endif
	nvram_set_int("bhdr_enable", bhdr ? 1 : 0);
}

void
fa_mode_init()
{
	fa_mode_adjust();

	fa_mode = atoi(nvram_safe_get("ctf_fa_mode"));
	switch (fa_mode) {
		case CTF_FA_BYPASS:
		case CTF_FA_NORMAL:
			break;
		default:
			fa_mode = CTF_FA_DISABLED;
			break;
	}
}

void
fa_nvram_adjust()	/* before insmod et */
{
#ifdef RTCONFIG_DUALWAN
	char buf[16];
	snprintf(buf, 16, "%s", nvram_safe_get("wans_dualwan"));
	if (strcmp(buf, "wan none") && strcmp(buf, "wan usb") && strcmp(buf, "usb wan"))
		return;
#endif

	fa_mode = nvram_get_int("ctf_fa_mode");
	if (FA_ON(fa_mode)) {

		_dprintf("\nFA on.\n");

		/* Set et2macaddr, et2phyaddr as same as et0macaddr, et0phyaddr */
		if (!nvram_get("vlan2ports") || !nvram_get("wandevs"))  {
			_dprintf("Insufficient envram, cannot do FA override\n");
			return;
		}

		/* adjusted */
		if (!strcmp(nvram_get("wandevs"), "vlan2") &&
		!strchr(nvram_get("vlan2ports"), 'u'))
			return;

		/* The vlan2ports will be change to "0 8u" dynamically by
		 * robo_fa_imp_port_upd. Keep nvram vlan2ports unchange.
		 */
		fa_override_vlan2ports();

		/* Override wandevs to "vlan2" */
		nvram_set("wandevs", "vlan2");
		nvram_set("wan_ifnames", "vlan2");
		if (nvram_match("wan0_ifname", "eth0")) nvram_set("wan0_ifname", "vlan2");

		if (!RESTORE_DEFAULTS()) {
			_dprintf("Override FA nvram...\n");
			nvram_commit();
		}
	}
	else {
		_dprintf("\nFA off.\n");
	}
}

void
chk_etfa()	/* after insmod et */
{
	if (FA_ON(fa_mode)) {
		if (
			!nvram_get_int("ctf_fa_cap") ||
			nvram_match("ctf_disable", "1") || nvram_match("ctf_disable_force", "1")) { // in case UI chaos
			_dprintf("\nChip Not Support FA Mode or ctf disabled!\n");

			nvram_unset("ctf_fa_mode");
			nvram_commit();
			reboot(RB_AUTOBOOT);
			return;
		}
	} else {
		if (!nvram_get_int("ctf_fa_cap"))
			nvram_unset("ctf_fa_mode");
	}
}
#endif /* RTCONFIG_BCMFA */

#if defined(RTCONFIG_BT_CONN)
void BT_chip_reset(int stage)
{
#if !defined(RTCONFIG_REALTEK) && !defined(RTCONFIG_ALPINE)
	uint32_t bt_reset, get_value;
#if defined(MAPAC1300) || defined(VZWAC1300) || defined(SHAC1300)
	bt_reset = 2;
#elif defined(MAPAC1750)
	bt_reset = 8;
#elif defined(MAPAC2200)
	bt_reset = 48;
#elif defined(BLUECAVE)
	bt_reset = 43;
#elif defined(RTAC95U)
	bt_reset = 34;
#elif defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX82_XD6) || defined(XD6_V2)
        bt_reset = 29;
#elif defined(RTAX82_XD6S)
	bt_reset = 16;
#elif defined(PLAX56_XP4)
        bt_reset = 79;
#elif defined(ET12) || defined(XT12)
	bt_reset = 3;
#elif defined(BC109) || defined(EBG19) || defined(EBG19_504)
	bt_reset = 25;
#else
#error NEED bt_reset defined
#endif

#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
	if(nvram_match("HwId", "B") || nvram_match("HwId", "D")){
		/* Slave, no bluetooth */
		return;
	}
#endif
	if (stage == 0) {
		gpio_dir(bt_reset, GPIO_DIR_OUT);
		set_gpio(bt_reset, 0);
		get_value=get_gpio(bt_reset);
		_dprintf("bt reset value: %d.\n", get_value);
		sleep(2);
		set_gpio(bt_reset, 1);
		get_value=get_gpio(bt_reset);
		_dprintf("bt reset value: %d.\n", get_value);
	} else {

#if defined(RTCONFIG_SOC_IPQ40XX) || defined(RTCONFIG_SOC_IPQ60XX)	/* only IPQ40XX install btchk.sh */
		nvram_unset("bt_reset_cnt");
		if (nvram_match("x_Setting", "0"))
		{
#if defined(RTCONFIG_SOC_IPQ60XX) && defined(RTCONFIG_BT_CONN_UART)
			nvram_set_int("bt_inf_state", 0);
			execute_bt_bscp();
			doSystem("/usr/bin/btchk_uart.sh wait_plug %d %s &", bt_reset, "/dev/ttyMSM1"); /* workaround script */
#elif defined(RTCONFIG_SOC_IPQ40XX) && defined(RTCONFIG_BT_CONN_USB)
			doSystem("/usr/bin/btchk.sh wait_plug %d &", bt_reset); /* workaround script */
#else
			;
#endif
		}
#endif
	}
#endif
}
#endif	/* RTCONFIG_BT_CONN */

#if defined(RPAC51)
void write_caldata_file()
{
TRACE_PT("write_caldata_file\n");
doSystem("dd if=/dev/mtdblock2 of=/tmp/wifi0.caldata bs=32 count=377 skip=128");  //2.4
doSystem("dd if=/dev/mtdblock2 of=/tmp/wifi1.caldata bs=32 count=377 skip=640");  //5G
}
#endif

#ifdef HND_ROUTER
#define WATCHDOG_MIN_LX4x 4000
static void start_hw_wdt(void)
{
#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_AX_6756)
	int wdt = nvram_get("watchdog_new") ? atoi(nvram_safe_get("watchdog_new")) : atoi(nvram_default_get("watchdog_new"));
#else
	int wdt = nvram_get("watchdog") ? atoi(nvram_safe_get("watchdog")) : atoi(nvram_default_get("watchdog"));
#endif

	/* arm the hw watchdog timer */
	if (wdt) {
		char tmp[100];

		wdt = (wdt > WATCHDOG_MIN_LX4x) ? wdt : WATCHDOG_MIN_LX4x;
		/* convert to secs */
		wdt /= 1000;
		printf("wdt:%d\n", wdt);

		snprintf(tmp, sizeof(tmp), "wdtctl -d -t %d start", wdt);
		system(tmp);
	}
}
#endif

void Ate_on_off_led_fail_loop(void)
{

#if defined(ET12) || defined(XT12)
	setATEModeLedFail();
	return;
#endif
	while(1) {
#ifdef RTCONFIG_LP5523
		lp55xx_leds_proc(LP55XX_RED_LEDS, LP55XX_ACT_NONE);
		pause();
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
		set_rgbled(RGBLED_RED);
		pause();
#elif defined(RTCONFIG_CONCURRENTREPEATER) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK))
		setAllLedOff();
		sleep(1);
		setAllLedOn();
		sleep(1);
#else
		led_control(LED_POWER, LED_OFF);
		sleep(1);
		led_control(LED_POWER, LED_ON);
		sleep(1);
#endif	/* RTCONFIG_LP5523 */
	}
}

#if defined(RTCONFIG_SOC_IPQ8074)
void Ate_run_in_interrupted_led_fail_loop(void)
{
	unsigned int i;
	int v[] = { LED_OFF, LED_ON };
	enum led_id right_leds_gpio[] = {
#if defined(RTAX89U)
		LED_POWER, LED_2G, LED_5G, LED_WAN,
#elif defined(GTAXY16000)
		LED_POWER, LED_5G, LED_WAN, LED_WAN_RED, LED_SFPP,
#else
#endif
		LED_ID_MAX
	}, *p;

	enum led_id left_leds_gpio[] = {
#if defined(RTAX89U)
		LED_WAN_RED, LED_LAN, LED_R10G, LED_SFPP,
#elif defined(GTAXY16000)
		LED_R10G, LED_2G, LED_LAN,
#else
#endif
		LED_ID_MAX
	};

	dbg("Enter infinit-loop!!!\n");
	for (i = 0; ; ++i) {
		if (pids("wanduck")) {
			killall("wanduck", SIGTERM);
			nvram_set("wanduck_down", "1");
		}

		for (p = right_leds_gpio; *p != LED_ID_MAX; ++p) {
			led_control(*p, v[i & 1]);
		}

		for (p = left_leds_gpio; *p != LED_ID_MAX; ++p) {
			led_control(*p, v[(i + 1) & 1]);
		}

		sleep(1);
	}
}
#endif

void Ate_on_off_led_success(void)
{
	extern void asus_ate_StartATEMode(void);
	int led_on_cut;
#if defined(RT4GAC68U) || defined(RTCONFIG_LP5523)
	#define LED_ON_CUT_TRY	1
#else
	#define LED_ON_CUT_TRY	6
#endif

	for(led_on_cut=0; led_on_cut<LED_ON_CUT_TRY; led_on_cut++)
	{
		dbG("delay 5 secs & Set_StartATEMode...\n");
		logmessage("ATE", "delay 5 secs & Set_StartATEMode...\n");
#if defined(RTCONFIG_SOC_IPQ40XX)
		doSystem("/sbin/delay_exec 5");
#else
		{
			int i;
			i = 1;
			while(i <= 5){
				sleep(1);

				++i;
			}
		}
#endif
#ifdef RTCONFIG_BCMARM
#ifdef RT4GAC68U
		asus_ate_StartATEMode();
#endif
		setAllLedOn();
#else
		asus_ate_StartATEMode();
#ifdef RTCONFIG_LP5523
		lp55xx_leds_proc(LP55XX_GREENERY_LEDS, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
		set_rgbled(RGBLED_GREEN);
#else
		if (get_model() == MODEL_RTAC53U) {
			led_control(LED_POWER, LED_ON);
			led_control(LED_USB, LED_ON);
		}
		else
			setAllLedOn();
#endif	/* RTCONFIG_LP5523 */
#endif
	}
	
}

void Ate_on_off_led_start(void)
{
#ifdef RTCONFIG_LP5523
	lp55xx_leds_proc(LP55XX_PURPLE_LEDS, LP55XX_ACT_3ON1OFF);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
	set_rgbled(RGBLED_PURPLE_3ON1OFF);
#endif	/* RTCONFIG_LP5523 */
}

#ifdef RPAX58
void rc_cled_init()
{
	int cled_rc = nvram_get_int("cled_rc");

        _dprintf("\n\n-----------rc reset cled: %d\n\n", cled_rc);

        eval("sw", "0xff803014", "0xd8a0");
        eval("sw", "0xff803010", "0xd8a0");

	switch(cled_rc) {
		case 1:	// static red
        	eval("sw", "0xff803070", "0x2000");
		eval("sw", "0xff8030e0", "0");
		eval("sw", "0xff803100", "0");
		break;

		case 2:	// static orange (cled-14)
        	eval("sw", "0xff803070", "0");
		eval("sw", "0xff8030e0", "0");
		eval("sw", "0xff803100", "0x2000");
		break;

		case 0: // white pulse
		_dprintf("cled: 0\n");
        	eval("sw", "0xff803070", "0");
        	eval("sw", "0xff8030e0", "0x3e002");
		eval("sw", "0xff803100", "0");
		break;
	}

        eval("sw", "0xff803090", "0");
        eval("sw", "0xff8030d0", "0");
	eval("sw", "0xff803110", "0");
        eval("sw", "0xff80301c", "0xd8a0");
}
#endif

static void sysinit(void)
{
#if defined(RTCONFIG_DEBUG)
	struct rlimit limit;
#endif
	static int noconsole = 0;
	static const time_t tm = 0;
	int i, r, retry = 0;
	DIR *d;
	struct dirent *de;
	char s[256];
	char t[256];
	int ratio = 30;
	int min_free_kbytes_check= 0;
#if defined(RTCONFIG_DUAL_TRX2)
	char *fixdmgfw_args[] = { "fixdmgfw", "-Y", NULL };
	pid_t pid;
#endif
#if !defined(RTCONFIG_QCA)
	int model;
#endif

#ifdef HND_ROUTER
	_dprintf("\nLaunch boot...\n");

	system("bcm_boot_launcher start");
	start_hw_wdt();
#endif

#ifdef RPAX58
	rc_cled_init();
#endif

#define MKNOD(name,mode,dev)		if (mknod(name,mode,dev))		perror("## mknod " name)
#define MOUNT(src,dest,type,flag,data)	if (mount(src,dest,type,flag,data))	perror("## mount " src)
#define MKDIR(pathname,mode)		if (mkdir(pathname,mode))		perror("## mkdir " pathname)
#ifdef RTCONFIG_RTK_NAND
/* [MUST]: Need to Clarify ... */
	system("mount -t yaffs2 -o tags-ecc-off /dev/mtdblock1 /hw_setting");
	system("mount -t yaffs2 -o tags-ecc-off /dev/mtdblock0 /hw_setting2");
	sleep(1);

	if(check_rtk_hw_invaild()) {
		printf("\n==> restore settings from /hw_setting2 !!\n");
		system("cp -f /hw_setting2/*.bin /hw_setting/");
		system("cp -f /hw_setting2/hw.bin /hw_setting/hw_backup.bin");
	}
#endif
#ifndef HND_ROUTER
	MOUNT("proc", "/proc", "proc", MS_MGC_VAL, NULL);
#endif

#ifdef RTCONFIG_REALTEK
#ifdef RPAC68U
	char c;
	f_write_string("/proc/asus_ate", "restore 1", 0, 0);
	f_read("/proc/asus_ate", &c, 1);
	int restore_flag = c - '0';
	if (restore_flag == 1) {
		dbg("Erase NVRAM...\n");
		system("rm /hw_setting/nvram.bin");
		system("rm /hw_setting2/nvram.bin");
		sleep(1);
		reboot(RB_AUTOBOOT);
	}
#endif
#endif

	MOUNT("tmpfs", "/tmp", "tmpfs", 0, NULL);
#if defined(HND_ROUTER) && !defined(RTCONFIG_USB)
	mkdir("/tmp/var", 0777);
	MOUNT("tmpfs", "/var", "tmpfs", 0, NULL);
#endif

#ifdef LINUX26
#ifndef DEVTMPFS
	MOUNT("devfs", "/dev", "tmpfs", MS_MGC_VAL | MS_NOATIME, NULL);
#endif
	if (!f_exists("/dev/null"))
		MKNOD("/dev/null", S_IFCHR | 0666, makedev(1, 3));
	if (!f_exists("/dev/console"))
		MKNOD("/dev/console", S_IFCHR | 0600, makedev(5, 1));
	MOUNT("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
	MKDIR("/dev/shm", 0777);
	MKDIR("/dev/pts", 0777);
#ifdef BLUECAVE
	MKDIR("/dev/switch_api", 0775);
	MKDIR("/tmp/wireless", 0777);
	MKDIR("/tmp/ramdisk", 0777);
#endif
	MOUNT("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
#if defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
    defined(RTCONFIG_QCA_AXCHIP) || \
    defined(RTCONFIG_SOC_IPQ40XX)
	MOUNT("debugfs", "/sys/kernel/debug", "debugfs", MS_MGC_VAL, NULL);
#endif
#endif

#ifdef __CONFIG_HSPOT__
	if (mkdir(RAMFS_CONFMTD_DIR, 0777) < 0 && errno != EEXIST) {
		perror("/tmp/confmtd not created.");
	}
	if (mkdir(RAMFS_CONFMTD_DIR"/hspot", 0777) < 0 && errno != EEXIST) {
		perror("/tmp/confmtd/hspot not created.");
	}
#endif /* __CONFIG_HSPOT__ */

#if defined(RTCONFIG_BCM_7114) || defined(HND_ROUTER)
	if (mkdir(RAMFS_CONFMTD_DIR, 0777) < 0 && errno != EEXIST) {
		perror("/tmp/confmtd not created.");
	}
	if (mkdir(RAMFS_CONFMTD_DIR"/crash_logs", 0777) < 0 && errno != EEXIST) {
		perror("/tmp/confmtd/crash_logs not created.");
	}
#endif

	init_devs(); // for system dependent part

#ifdef DSL_AX82U
	if (is_ax5400_i1()) noconsole = 1; else
#endif
	stime(&tm);

	if(nvram_match("noconsole", "1")) {
		noconsole = 1;
	}


	static const char *mkd[] = {
		"/tmp/etc", "/tmp/var", "/tmp/home",
#ifdef RTCONFIG_USB
		POOL_MOUNT_ROOT,
		NOTIFY_DIR,
		NOTIFY_DIR"/"NOTIFY_TYPE_USB,
#endif
		"/tmp/share", "/var/webmon", // !!TB
		"/var/log", "/var/run", "/var/tmp", "/var/lib", "/var/lib/misc",
		"/var/spool", "/var/spool/cron", "/var/spool/cron/crontabs",
		"/var/cache",
#ifdef RTCONFIG_INADYN
		"/var/cache/inadyn",
#endif
		"/tmp/var/wwwext", "/tmp/var/wwwext/cgi-bin",	// !!TB - CGI support
#ifdef BLUECAVE
		"/tmp/etc/rc.d",
#endif
		"/tmp/var/tmp",
		"/tmp/inadyn.cache",
#ifdef RTCONFIG_WLMODULE_MT7663E_AP
		"/tmp/lib",
		"/tmp/lib/firmware",
		"/tmp/etc/wireless",
#endif // RTCONFIG_WLMODULE_MT7663E_AP
		NULL
	};
	umask(0);
	for (i = 0; mkd[i]; ++i) {
		mkdir(mkd[i], 0755);
	}
	// LPRng support
	mkdir("/var/state", 0777);
	mkdir("/var/state/parport", 0777);
	mkdir("/var/state/parport/svr_statue", 0777);

	mkdir("/var/lock", 0777);
	mkdir("/var/tmp/dhcp", 0777);
	mkdir("/home/root", 0700);
	chmod("/tmp", 0777);
#ifdef RTCONFIG_USB
	chmod(POOL_MOUNT_ROOT, 0777);
#endif
	f_write("/etc/hosts", NULL, 0, 0, 0644);			// blank
	f_write("/etc/fstab", NULL, 0, 0, 0644);			// !!TB - blank
	f_write("/tmp/settings", NULL, 0, 0, 0644);

	//umask(022);

	if ((d = opendir("/rom/etc")) != NULL) {
		while ((de = readdir(d)) != NULL) {
			if (de->d_name[0] == '.') continue;
			snprintf(s, sizeof(s), "%s/%s", "/rom/etc", de->d_name);
			snprintf(t, sizeof(t), "%s/%s", "/etc", de->d_name);
			symlink(s, t);
		}
		closedir(d);
	}
	if(nvram_match("noconsole", "1")) {
		printf("!! no use mtab\n");
	} else {
		symlink("/proc/mounts", "/etc/mtab");
	}

	/* /etc/resolv.conf compatibility */
	unlink("/etc/resolv.conf");
	symlink("/tmp/resolv.conf", "/etc/resolv.conf");

#ifdef HND_ROUTER
	system("/sbin/ldconfig &>/dev/null");
#endif

#ifdef RTCONFIG_VAR_NVRAM
	var_nvram_init();
#endif

#ifdef RTCONFIG_WLMODULE_MT7663E_AP
	f_write_string("/tmp/etc/wireless/l1profile.dat", l1profile_default, 0, 0);
#endif	// RTCONFIG_WLMODULE_MT7663E_AP

#if defined(RTAC55U) || defined(RTAC55UHP) || defined(RTAC58U) || defined(RT4GAC53U) || defined(RTAC82U)
	ratio = 20;
#elif defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
	ratio = 90;
#endif
	limit_page_cache_ratio(ratio);

#ifdef RTN65U
	set_pcie_aspm();
#endif

#ifdef RTCONFIG_SAMBASRV
	if ((d = opendir("/usr/codepages")) != NULL) {
		while ((de = readdir(d)) != NULL) {
			if (de->d_name[0] == '.') continue;
			snprintf(s, sizeof(s), "/usr/codepages/%s", de->d_name);
			snprintf(t, sizeof(t), "/usr/share/%s", de->d_name);
			symlink(s, t);
		}
		closedir(d);
	}
#endif


#if !defined(RTCONFIG_SOC_IPQ40XX)
#if defined(RTCONFIG_BLINK_LED)
	modprobe("bled");
#endif
#endif
#ifdef LINUX26
	do {
		r = eval("mdev", "-s");
		if (r || retry)
			_dprintf("mdev coldplug terminated. (ret %d retry %d)\n", r, retry);
	} while (r && retry++ < 10);
#ifdef RTCONFIG_LANTIQ
	init_devs_defer(); // for system dependent part
#endif

	start_hotplug2();

	static const char *dn[] = {
		"null", "zero", "random", "urandom", "full", "ptmx",
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
		MTD_OF_NVRAM,
#else
		"nvram",
#endif
		NULL
	};
	for (i = 0; dn[i]; ++i) {
		snprintf(s, sizeof(s), "/dev/%s", dn[i]);
		chmod(s, 0666);
	}
#ifndef HND_ROUTER
	chmod("/dev/gpio", 0660);
#endif
#ifdef RTCONFIG_HND_ROUTER_AX_6756
        eval("/etc/make_static_devnodes.sh");
#endif
#endif

#if defined(RTCONFIG_SOC_IPQ40XX)
	unlink("/dev/random");
	system("ln -sf /dev/urandom /dev/random");
#endif
	set_action(ACT_IDLE);

	for (i = 0; defenv[i]; ++i) {
		putenv(defenv[i]);
	}

	if(nvram_match("noconsole", "1"))
		noconsole = 1;

	if (!noconsole) {
		printf("\n\nHit ENTER for console...\n\n");
		run_shell(1, 0);
	} else
		printf("\n\nNo console...\n\n");
		

#ifdef RPAX56
def_boot_reinit:
#endif
	def_boot = 0;

	dbg("firmware version: %s_%s, dev:(%s/%s)\n",
		rt_serialno, rt_extendno,
#ifdef CONFIG_BCMWL5
		strlen(nvram_safe_get("serial_no")) ? nvram_safe_get("serial_no") : cfe_nvram_safe_get("serial_no"),
		strlen(nvram_safe_get("et0macaddr")) ? nvram_safe_get("et0macaddr") : cfe_nvram_safe_get("et0macaddr")
#else
		nvram_safe_get("serial_no"), nvram_safe_get("et0macaddr")
#endif
	);

	nvram_set("firmver_org", nvram_safe_get("firmver"));
	nvram_set("buildno_org", nvram_safe_get("buildno"));
	if(rt_rcno)
		nvram_set("rcno_org", nvram_safe_get("rcno"));
	else
		nvram_unset("rcno_org");
	nvram_set("extendno_org", nvram_safe_get("extendno"));

#ifdef RPAX56
	reset_abmac();
#endif
#if defined(RTCONFIG_QCA)
	pre_syspara();
#endif
	init_syspara();// for system dependent part (befor first get_model())

#ifdef RTCONFIG_RALINK
	model = get_model();
	// avoid the process like fsck to devour the memory.
	// ex: when DUT ran fscking, restarting wireless would let DUT crash.

	if ((model == MODEL_RTAC1200) || (model == MODEL_RTAC1200V2) || (model == MODEL_RTAC1200GA1) ||(model == MODEL_RTAC1200GU))
	{
		f_write_string("/proc/sys/vm/min_free_kbytes", "8192", 0, 0);
		min_free_kbytes_check = 1;
	}
	else if ((model == MODEL_RTN56U) || (model == MODEL_DSLN55U) || (model == MODEL_RTAC52U) || (model == MODEL_RTAC51U) \
			 || (model == MODEL_RTAC51UP) ||(model == MODEL_RTAC53) ||(model == MODEL_RTN14U) ||(model == MODEL_RTN54U) \
			 || (model == MODEL_RTAC54U) ||(model == MODEL_RTAC1200HP) || (model == MODEL_RTN56UB1) || (model == MODEL_RTN56UB2) \
			 || (model == MODEL_RTN800HP))
	{
		f_write_string("/proc/sys/vm/min_free_kbytes", "4096", 0, 0);
		min_free_kbytes_check = 1;
	}
	else
	{
		f_write_string("/proc/sys/vm/min_free_kbytes", "2048", 0, 0);
		min_free_kbytes_check = 0;
	}
#elif defined(RTCONFIG_QCA)
#if defined(PLAX56_XP4)
	f_write_string("/proc/sys/net/ipv4/conf/default/arp_ignore", "1", 0, 0);
	f_write_string("/proc/sys/net/ipv4/conf/all/arp_ignore", "1", 0, 0);
	f_write_string("/proc/sys/net/ipv4/conf/default/arp_announce", "2", 0, 0);
	f_write_string("/proc/sys/net/ipv4/conf/all/arp_announce", "2", 0, 0);
#endif
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994)
	f_write_string("/proc/sys/vm/min_free_kbytes", "23916", 0, 0);
#elif defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
      defined(RTCONFIG_QCA_AXCHIP)
	/* system default, ~ 3.7MB */
	min_free_kbytes_check = 1;
#elif defined(RPAC51)
	f_write_string("/proc/sys/vm/min_free_kbytes", "8192", 0, 0);
#else
	f_write_string("/proc/sys/vm/min_free_kbytes", "4096", 0, 0);
#endif
	min_free_kbytes_check = 1;
#elif defined(RTCONFIG_REALTEK)
	model = get_model();
	if (/*model == MODEL_RTRTL8198C ||*/ model == MODEL_RPAC68U)
	{
		f_write_string("/proc/sys/vm/min_free_kbytes", "2048", 0, 0);
		min_free_kbytes_check = 1;
		rtl_configRps();
	}else if (model == MODEL_RPAC92)
	{
		f_write_string("/proc/sys/vm/min_free_kbytes", "20480", 0, 0);
		min_free_kbytes_check = 1;
	}

#else
	model = get_model();
	// At the Broadcom platform, restarting wireless won't use too much memory.
	if (model==MODEL_RTN53) {
		f_write_string("/proc/sys/vm/min_free_kbytes", "2048", 0, 0);
		min_free_kbytes_check = 0;
	}
	else if (model==MODEL_RTAC53U) {
		f_write_string("/proc/sys/vm/min_free_kbytes", "4096", 0, 0);
		min_free_kbytes_check = 1;
	}
#endif
#ifdef RTCONFIG_16M_RAM_SFP
	f_write_string("/proc/sys/vm/min_free_kbytes", "512", 0, 0);
	min_free_kbytes_check = 0;
#endif
#ifdef RTCONFIG_BCMARM
	if (nvram_get_int("min_free_kbytes_manual"))
		f_write_string("/proc/sys/vm/min_free_kbytes", nvram_safe_get("min_free_kbytes_manual"), 0, 0);
	else if (model==MODEL_RTAC1200G || model==MODEL_RTAC1200GP)
		f_write_string("/proc/sys/vm/min_free_kbytes", "4096", 0, 0);
	else if (model==MODEL_RTAX56_XD4 || model==MODEL_XD4PRO || model==MODEL_CTAX56_XD4)
		f_write_string("/proc/sys/vm/min_free_kbytes", "40960", 0, 0);
	else if (model==MODEL_RTAX95Q || model==MODEL_XT8PRO || model==MODEL_BM68 || model==MODEL_XT8_V2 || model==MODEL_RTAXE95Q || model==MODEL_ET8PRO || model==MODEL_ET8_V2 || model==MODEL_RTAX56U || model==MODEL_RPAX56 || model==MODEL_RPAX58)
		f_write_string("/proc/sys/vm/min_free_kbytes", "61440", 0, 0);
	else if (model==MODEL_RTAX55 || model==MODEL_RTAX3000N) {
		f_write_string("/proc/sys/vm/overcommit_ratio", "60", 0, 0);
	} else	// fix _dma_rxfill error under stress test
		f_write_string("/proc/sys/vm/min_free_kbytes", "20480", 0, 0);
	min_free_kbytes_check = 1;
#endif
#ifdef RTCONFIG_USB
	if(min_free_kbytes_check<1){
		f_write_string("/proc/sys/vm/min_free_kbytes", "4096", 0, 0);
	}
#endif

#ifdef RTCONFIG_LANTIQ
	if(nvram_match("blver", "0.0.3.12"))
		f_write_string("/proc/sys/vm/min_free_kbytes", "30720", 0, 0);
	else
		f_write_string("/proc/sys/vm/min_free_kbytes", "8192", 0, 0);
	f_write_string("/proc/sys/vm/overcommit_memory", "0", 0, 0);
	f_write_string("/proc/sys/vm/panic_on_oom", "1", 0, 0);
	f_write_string("/proc/sys/vm/overcommit_ratio", "10", 0, 0);

	f_write_string("/proc/sys/vm/lowmem_reserve_ratio", "250", 0, 0);
	f_write_string("/proc/sys/vm/dirty_background_ratio", "2", 0, 0);
	f_write_string("/proc/sys/vm/dirty_writeback_centisecs", "250", 0, 0);
	f_write_string("/proc/sys/vm/dirty_ratio", "10", 0, 0);
	f_write_string("/proc/sys/vm/max_map_count", "16384", 0, 0);
	f_write_string("/proc/sys/vm/scan_unevictable_pages", "1", 0, 0);
	f_write_string("/proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established", "2400", 0, 0);
#endif

#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
	// for the nvram file's commit, if don't set this & the commit will cause the abnormal nvram file.
	_dprintf("init: set drop_caches be 1 at ALPINE & LANTIQ...\n");
	f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
#else
	force_free_caches();
#endif

	// autoreboot after kernel panic
	f_write_string("/proc/sys/kernel/panic", "3", 0, 0);
	f_write_string("/proc/sys/kernel/panic_on_oops", "3", 0, 0);

	// be precise about vm commit
#ifdef CONFIG_BCMWL5
	f_write_string("/proc/sys/vm/overcommit_memory", "2", 0, 0);
	f_write_string("/proc/sys/vm/panic_on_oom", "1", 0, 0);
	char overcommit_ratio[4];
#if defined(BCM6750) || defined(BCM63178)
	int default_overcommit_ratio = 50;
#else
	int default_overcommit_ratio = 100;
#endif
	snprintf(overcommit_ratio, sizeof(overcommit_ratio), "%d", nvram_get_int("vm_overcommit_ratio") ? : default_overcommit_ratio);
	f_write_string("/proc/sys/vm/overcommit_ratio", overcommit_ratio, 0, 0);
#endif

#ifdef RTCONFIG_IPV6
	// disable IPv6 by default on all interfaces
	f_write_string("/proc/sys/net/ipv6/conf/default/disable_ipv6", "1", 0, 0);
	// disable IPv6 DAD by default on all interfaces
	f_write_string("/proc/sys/net/ipv6/conf/default/accept_dad", "2", 0, 0);
	// increase ARP cache
	f_write_string("/proc/sys/net/ipv6/neigh/default/gc_thresh1", "512", 0, 0);
	f_write_string("/proc/sys/net/ipv6/neigh/default/gc_thresh2", "1024", 0, 0);
	f_write_string("/proc/sys/net/ipv6/neigh/default/gc_thresh3", "2048", 0, 0);
#endif

#if defined(RTCONFIG_BWDPI) && defined(RTCONFIG_QCA)
	f_write_string("/proc/sys/net/netfilter/nf_conntrack_acct", "1", 0, 0);
#endif

#if defined(RTCONFIG_PSISTLOG) || defined(RTCONFIG_JFFS2LOG)
	f_write_string("/proc/sys/net/unix/max_dgram_qlen", "150", 0, 0);
#endif

#if defined(RTCONFIG_BCM_HND_CRASHLOG) && !defined(RTCONFIG_HND_ROUTER_AX_6756)
	f_write_string("/proc/sys/kernel/crashlog_mtd", "misc3", 0, 0);
#endif

#if defined(RTCONFIG_DEBUG)
	/* Unlock core-dump generation. */
	limit.rlim_cur = RLIM_INFINITY;
	limit.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &limit);
#endif
#if defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(RTCONFIG_BCM_502L07P2) || defined(RTCONFIG_DEBUG)
	f_write_string("/proc/sys/kernel/core_uses_pid", "1", 0, 0);
	f_write_string("/proc/sys/kernel/core_pattern", "/tmp/core-%e-%g-%p-%s-%t-%u", 0, 0);
	f_write_string("/proc/sys/fs/suid_dumpable", "2", 0, 0);
#endif
	f_write_string("/proc/sys/kernel/print-fatal-signals", "1", 0, 0);

	for (i = 0; i < sizeof(fatalsigs) / sizeof(fatalsigs[0]); i++) {
		signal(fatalsigs[i], handle_fatalsigs);
	}
	signal(SIGCHLD, handle_reap);

#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_RALINK
	check_upgrade_from_old_ui();
#endif
#endif

#if defined(RTCONFIG_DUAL_TRX2)
	/* Find inactive firmware and restore it if it is broken. */
	_eval(fixdmgfw_args, NULL, 0, &pid);
#endif

#ifdef RTCONFIG_ATEUSB3_FORCE
	post_syspara();	// make sure usb_usb3 is fix-up in accordance with the force USB3 parameter.
#endif

#ifdef RTCONFIG_GMAC3
	int gmac3 = 0;

	if (!nvram_match("disable_gmac3_force", "1")
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		&& !psr_mode() && !psta_exist()
#endif
#ifdef RTCONFIG_DPSTA
		&& !dpsta_mode()
#endif
	)
		gmac3 = 1;

	nvram_set_int("gmac3_enable", gmac3 ? 1 : 0);
	nvram_set_int("bhdr_enable", gmac3 ? 1 : 0);
#endif

#ifdef RTCONFIG_BCMFA
	fa_mode_init();
#endif

#ifdef RTCONFIG_DETWAN
	if (nvram_match("wifison_ready", "1") && sw_mode() == SW_MODE_ROUTER) {
		if (nvram_safe_get("detwan_ifname")[0] != '\0') {
			if (nvram_safe_get("wan0_ifname")[0] == '\0') {
				/* restore wan0_ifname */
				nvram_set("wan0_ifname", nvram_safe_get("detwan_ifname"));
				nvram_set("wanports_mask", nvram_safe_get("detwan_wan_mask"));
				nvram_set("lanports_mask", nvram_safe_get("detwan_lan_mask"));
				nvram_set("detwan_proto", "1");
				nvram_unset("lan_ipaddr");
				nvram_unset("lan_gateway");
				nvram_unset("lan_hwaddr");
			}
		} else if (nvram_match("x_Setting", "1")) {
			/* old firmware, update nvram */
			nvram_set("detwan_ifname", nvram_safe_get("wan0_ifname"));
			nvram_set("detwan_wan_mask", nvram_safe_get("wanports_mask"));
			nvram_set("detwan_lan_mask", nvram_safe_get("lanports_mask"));
		}
	}
#endif
#if defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
	nvram_set("success_start_service", "0");
#endif
#ifdef GT10
	gt10_nvram_convert();
#endif
#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_VIF_ONBOARDING
	nvram_unset("obvif_set");
#endif
#endif
	init_nvram();  // for system indepent part after getting model

#ifdef RTCONFIG_JFFS_NVRAM
	if(RESTORE_DEFAULTS()) {
		nvram_set("jffs2_on", "1");
		nvram_set("jffs2_clean_fs", "1");
#if defined(RTCONFIG_HND_ROUTER_AX_6756) && defined(RTCONFIG_UBIFS)
		nvram_set("ubifs_on", "1");
		nvram_set("ubifs_clean_fs", "1");
#endif
	}
#ifdef RPAX56
	if(!def_boot)
#endif
	start_jffs2();
#endif
	if(restore_defaults()) // restore default if necessary
	{
		def_boot = 1;
		init_nvram();	//refill nvram parameters after restore	
	}
	init_nvram2();
#ifdef RPAX56
	if(def_boot) {
		reset_psr_hwaddr();
		_dprintf("Go sysinit since restore defaults\n");
		goto def_boot_reinit;
	}

	update_rf_para();
#endif

	/* set hostname after nvram init */
	set_hostname();

#ifdef RTCONFIG_AVBLCHAN
	avblchan_defaults();
#endif

#if defined (RTCONFIG_WLMODULE_MT7615E_AP)
#if !defined(RTCONFIG_CONCURRENTREPEATER)
	nvram_set("lan_ifnames_guess", nvram_safe_get("lan_ifnames"));
#endif
#elif defined (RTCONFIG_WLMODULE_MT7663E_AP) || defined (RTCONFIG_WLMODULE_MT7628_AP)
#if defined(RTAC1200V2)
	nvram_set("lan_ifnames_guess", nvram_safe_get("lan_ifnames"));
#endif
#endif

#ifdef RTCONFIG_ATEUSB3_FORCE
	post_syspara(); // adjust nvram variable after restore_defaults
#endif
#if defined(RTCONFIG_ASUSCTRL)
#if defined(RTCONFIG_BCMARM) ||  defined(RTCONFIG_QCA)
	init_asusctrl();
#endif
	asus_ctrl_nv_restore();
#endif
#ifdef RTCONFIG_BCM_7114
#ifdef RTCONFIG_GMAC3
	chk_gmac3_excludes();
#else
	gmac3_restore_nvram();
#endif
#endif
	nvram_set("label_mac", get_label_mac());

#if defined(RPAC51)
	write_caldata_file();
#endif

#if !defined(CONFIG_BCMWL5)	//Broadcom set this in check_wl_territory_code()
	void handle_location_code_for_wl(void);
	handle_location_code_for_wl();
#endif	/* CONFIG_BCMWL5 */

	init_gpio();   // for system dependent part
#if defined(RTCONFIG_CONCURRENTREPEATER)
#ifdef RPAC68U
	set_led(LED_BLINK_SLOW, LED_BLINK_SLOW);
#elif defined(RPAC92)
	setAllLedOff();
	start_led_monitor();
	nvram_set_int("led_status", LED_BOOTING);
#else
	start_led_monitor();
	nvram_set_int("led_status", LED_BOOTING);
#endif
#endif
#ifdef RTCONFIG_SWMODE_SWITCH
	init_swmode(); // need to check after gpio initized
#endif	/* RTCONFIG_SWMODE_SWITCH */
#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
	init_others();
#endif

#if defined(RTCONFIG_EXTPHY_BCM84880) && defined(BCM4908)
	config_ext_wan_port();
#endif
	init_switch(); // for system dependent part
	upgrade_bootloader_v2();

#if defined(RTCONFIG_SOC_IPQ40XX)
#if defined(RTCONFIG_BLINK_LED)
	modprobe("bled");
#endif
	init_nvram3();
#endif

#if defined(RTCONFIG_QCA)
#ifdef RTCONFIG_AMAS
	nvram_unset("obd_prelinking");
#endif
	set_uuid();
#if defined(RTCONFIG_TEST_BOARDDATA_FILE)
	start_jffs2();
#endif
#endif
#if defined(RTCONFIG_BCM_7114)
	mkdir("/tmp/media", 0777);
	symlink("/jffs", "/tmp/media/nand");
#endif
	init_wl(); // for system dependent part

	klogctl(8, NULL, nvram_get_int("console_loglevel"));

	// Enable rate report for tc classes (QoS stats)
#ifdef HND_ROUTER
	f_write_string("/sys/module/sch_htb/parameters/htb_rate_est", "1", 0, 0);
#endif
	setup_conntrack();
	setup_timezone();
	if (!noconsole) xstart("console");

#ifdef RTCONFIG_REALTEK
{
	extern void usb3_enable(int en);
	if (nvram_get("usb_usb3")) {
		usb3_enable(atoi(nvram_get("usb_usb3")));
	}
}
#endif

#ifdef RTCONFIG_BCMFA
	chk_etfa();
#endif

#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
    defined(RTCONFIG_QCA_AXCHIP)
	start_thermald();
#endif
	set_power_save_mode();
#if defined(RTCONFIG_FANCTRL)
	restart_fanctrl();
#endif
#if defined(RTCONFIG_BT_CONN)
	BT_chip_reset(0);		 // initialize BT
#endif
#if defined(MAPAC2200)
	nvram_unset("dpdt_ant");
#endif
#ifdef HND_ROUTER
	reset_stacksize(ASUSRT_STACKSIZE);
#endif
#ifdef RTCONFIG_SOFTWIRE46
	if (nvram_match("x_Setting", "0") && !strncmp(nvram_safe_get("territory_code"), "JP", 2)) {
		init_wan46();
		nvram_set("ipv6_service", "ipv6pt");
	}
	nvram_set("s46_mapsvr_id", "");
#endif

#ifdef RTCONFIG_PC_SCHED_V2
	convert_pc_sched_v1_to_sched_v2();
#endif
#ifdef RTCONFIG_WL_SCHED_V2
	convert_wl_sched_v1_to_sched_v2();
#endif

#ifdef RTCONFIG_ASD
	nvram_set("3rd-party", "merlin");
#endif

#if defined(RTCONFIG_BCMBSD_V2)
	if(nvram_match("reset_bsd_def", "1"))
		gen_bcmbsd_def_policy(0);
#endif

	record_prev_sys_uptime();

#ifdef DSL_AX82U
	if (is_ax5400_i1())
		reset_corefilesize(0x800000);
#endif
}

#if defined(RTCONFIG_TEMPROOTFS)
static void sysinit2(void)
{
	static int noconsole = 0;
	static const time_t tm = 0;
	int i;

	MOUNT("proc", "/proc", "proc", 0, NULL);
	MOUNT("tmpfs", "/tmp", "tmpfs", 0, NULL);

#ifdef LINUX26
	MOUNT("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
	MOUNT("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
#endif

	if (console_init()) noconsole = 1;

	stime(&tm);

	f_write("/tmp/settings", NULL, 0, 0, 0644);

	if(nvram_match("noconsole", "1")) {
		printf("!! no use mtab\n");
	} else {
		symlink("/proc/mounts", "/etc/mtab");
	}
	set_action(ACT_IDLE);

	if (!noconsole) {
		printf("\n\nHit ENTER for new console...\n\n");
		run_shell(1, 0);
	}

	for (i = 0; i < sizeof(fatalsigs) / sizeof(fatalsigs[0]); i++) {
		signal(fatalsigs[i], handle_fatalsigs);
	}
	signal(SIGCHLD, handle_reap);

	if (!noconsole) xstart("console");
}
#else
static inline void sysinit2(void) { }
#endif

void
Alarm_Led(void) {
	while(1) {
#if defined(RTCONFIG_CONCURRENTREPEATER)
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK)
		setAllLedOff();
		sleep(1);
		setAllLedOn();
		sleep(1);
#endif
#else
		led_control(LED_POWER, LED_OFF);
		sleep(1);
		led_control(LED_POWER, LED_ON);
		sleep(1);
#endif
	}
}
void config_format_compatibility_handler(void)
{
	adjust_merlin_config();
	adjust_url_urlelist();
	adjust_ddns_config();
	adjust_access_restrict_config();
#if defined(RTCONFIG_VPN_FUSION)
	adjust_vpnc_config();
#endif
#if defined(RTCONFIG_NOTIFICATION_CENTER)
	sync_nc_conf();
	force_off_push_msg();
#endif
}

#ifdef RTCONFIG_WIFILOGO
static void
run_rc_local(void)
{
	char *cmd;
	struct stat tmp_stat;

	if ((cmd = nvram_get("rclocal")) != NULL &&
		stat(cmd, &tmp_stat) == 0) {
		system(cmd);
	}
}
#endif

#ifdef RTCONFIG_NVRAM_ENCRYPT
int init_nvram4(void)
{
	int model = get_model();
	switch(model)
	{
#if defined(DSL_AX82U) && !defined(RTCONFIG_BCM_MFG)
	case MODEL_DSLAX82U:
		if (!nvram_get_int("x_Setting") && is_ax5400_i1()) {
			char http_passwd[128] = {0};
			char output[2048] = {0};
			int enclen = 0;
			strlcpy(http_passwd, cfe_nvram_safe_get_raw("wifi_psk"), sizeof(http_passwd));
			enclen = pw_enc_blen(http_passwd) + 1;
			char *enc_passwd = malloc(enclen);
			if(enc_passwd){
				memset(enc_passwd, 0, enclen);
				pw_enc(http_passwd, enc_passwd, 1);
				//_dprintf("\n=====http_passwd: %s\nenc: %s\n=====\n\n", http_passwd, enc_passwd);
				nvram_set("http_passwd", enc_passwd);
				nvram_set("http_username", "optus");
				nvram_set("x_Setting", "1");
				snprintf(output, sizeof(output), "optus>%s", enc_passwd);
				nvram_set("acc_list", output);
				free(enc_passwd);
			}
		}
		break;
#endif
	default:
		break;
	}

	return 0;
}
#endif

#if defined(RTCONFIG_HND_ROUTER_AX_6756)
int getRebootPartition()
{
      int higher = 1;
      int reboot;
      int seq1 = -1;
      int seq2 = -1;

      seq1 = getSequenceNumber(1);
      seq2 = getSequenceNumber(2);

      if (seq1 > seq2)
          higher = 1;
      else
          higher = 2;

      if ((seq1 == 0) && (seq2 == 999))
         higher = 1;
      else if ((seq2 == 0) && (seq1 == 999))
         higher = 2;

      switch(getBootImageState())
      {
           case(BOOT_SET_NEW_IMAGE):
              reboot = higher;
              break;

           case(BOOT_SET_OLD_IMAGE):
              reboot = (higher == 1) ? 2 : 1;
              break;

           case(BOOT_SET_NEW_IMAGE_ONCE):
              reboot = higher;
              break;

           case(BOOT_SET_OLD_IMAGE_ONCE):
              reboot = (higher == 1) ? 2 : 1;
              break;

           case(BOOT_SET_PART1_IMAGE):
              reboot = 1;
              break;

           case(BOOT_SET_PART1_IMAGE_ONCE):
              reboot = 1;
              break;

           case(BOOT_SET_PART2_IMAGE):
              reboot = 2;
              break;

           case(BOOT_SET_PART2_IMAGE_ONCE):
              reboot = 2;
              break;

           default:
              if (seq1 > seq2)
                 reboot = 1;
              else
                 reboot = 2;
              break;
      }

        return reboot;
}

int chk_same_boot_policy()
{
	int boot = getBootPartition();
	int reboot = getRebootPartition();
#ifdef CUSTOM_NAND_SINGLE_IMAGE
	return 1;
#endif
	_dprintf("%s: boot(%d), reboot(%d)\n", __func__, boot, reboot);
	
	return boot == reboot;
}

void sync_boot_state()
{
	int matched = 0;

	matched = chk_same_boot_policy();

	/* Assume this is first time boot after upgrading fw */
	if(matched) {
		_dprintf("%s, matched, do nothing\n", __func__);
		return;
	} else {
		_dprintf("%s, reset bootstate as new\n", __func__);
		setBootImageState(BOOT_SET_NEW_IMAGE);
	}

	matched = chk_same_boot_policy();

	/* Assume it falls back to old fw due upgrading new fw fail */
	if(matched) {
		_dprintf("%s, matched. fin sync.\n", __func__);
		return;
	} else {
		_dprintf("%s, reset bootstate as old\n", __func__);
		setBootImageState(BOOT_SET_OLD_IMAGE);
	}

	matched = chk_same_boot_policy();

	if(matched) {
		_dprintf("%s, matched. fin sync..\n", __func__);
		return;
	} else {
		_dprintf("%s, boot state sync error.\n", __func__);
	}
}

#endif

int init_main(int argc, char *argv[])
{
	int state, i;
	sigset_t sigset;
	siginfo_t info;
	int rc_check = 0, boot_check; //Power on/off test
	int boot_fail, dev_fail, dev_fail_count, total_fail_check = 0;
	int check_delay, delay;
	char reboot_log[128], dev_log[128];
	int ret;
#if defined(RTCONFIG_SOC_IPQ40XX)
	char argva[128];
#endif
#ifdef RTCONFIG_SINGLEIMG_B
	int r;
	//char *const new_argv[] = { "/usr/bin/fwupg_flashing2.sh", NULL };
	char *const new_argv[] = { "/bin/fwupg_flashing", NULL };
	char *const new_argv2[] = { "/bin/sh", NULL };
	char *newdir, *newroot;
	int newexec = 0;
#endif

	_dprintf("init_main start.\n");

#ifdef RT4GAC68U
	_dprintf("Turn on the power of the built-in modem.\n\n");
	set_pwr_modem(1);
#endif

#if defined(RTCONFIG_TEMPROOTFS)
	if (argc >= 2 && !strcmp(argv[1], "reboot")) {
		state = SIGTERM;
		sysinit2();
	} else
#endif
	{

		sysinit();

#ifdef RT4GAC68U
#if defined(RTCONFIG_USB) && defined(RTCONFIG_USB_MODEM)
		// to avoid the modem cannot be found after hard boot sometimes.
		_dprintf("4G-AC68U: Insert USB modules early.\n");
		set_pwr_usb(1); // for Sandisk CZ43 Ultra Fit.
		add_usb_host_modules();
		add_usb_modem_modules();
#endif
#endif

#ifdef RTCONFIG_LANTIQ
		init_others_defer();
#endif

		sigemptyset(&sigset);
		for (i = 0; i < sizeof(initsigs) / sizeof(initsigs[0]); i++) {
			sigaddset(&sigset, initsigs[i]);
		}
		sigprocmask(SIG_BLOCK, &sigset, NULL);

#if !defined(RTCONFIG_TEST_BOARDDATA_FILE) && !defined(RTCONFIG_JFFS_NVRAM)
		start_jffs2();
#endif
#if defined(RTCONFIG_HTTPS)
		restore_cert();
#endif
#ifdef RTCONFIG_AMAS
		create_amas_sys_folder();
#endif
#ifdef RTCONFIG_NVRAM_ENCRYPT
		init_pass_nvram();
		init_enc_nvram();
		init_nvram4();
#endif

		config_format_compatibility_handler();

		// 1. passwd don't need to write too early.
		// 2. the PMS table is existed at /jffs.
		setup_passwd();

#if defined(RTCONFIG_PSISTLOG)
		//set soft link for some app such as 3ginfo.sh
		char *syslog[]  = { "/tmp/syslog.log", "/tmp/syslog.log-1" };
		char *filename;
#if defined(MAPAC1750)
		{ /* fix old firmware's bug */
			char *jffs_syslog[2]  = { "/jffs/syslog.log", "/jffs/syslog.log-1" };
			unsigned long ok_size;
			int is_over_size = 0;
			struct stat st;

			ok_size = nvram_get_int("log_size") * 1024;
			if (ok_size) {
				ok_size += 30 * 1024; /* buffer for excess size */
				for(i = 0; i < 2; i++) {
					filename = jffs_syslog[i];
					if (stat(filename, &st)==0 && st.st_size > ok_size) {
						is_over_size = 1;
						break;
					}
				}
				if (is_over_size) {
						for(i = 0; i < 2; i++) {
							_dprintf("[TRIO] remove oversize log file!\n");
							unlink(jffs_syslog[i]);
						}
				}
			}
		}
#endif
		for(i = 0; i < 2; i++)
		{
			filename = get_syslog_fname(i);
			if(strcmp(filename, syslog[i]) != 0)
				eval("ln", "-s", filename, syslog[i]);
		}
#endif

#if defined(RTCONFIG_BCM_HND_CRASHLOG)
#if defined(RTCONFIG_HND_ROUTER_AX_6756)
	mtd_export_crashlog();
#else
	struct stat crashlog_stat;
	char clogpath[32];
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2)
	snprintf(clogpath, sizeof(clogpath), "/jffs/crashlog.log");
#else
	snprintf(clogpath, sizeof(clogpath), "/tmp/crashlog.log");
#endif
	f_write_string("/proc/sys/kernel/crashlog_filename", clogpath, 0, 0);
	if(!stat(clogpath, &crashlog_stat)) {
		if(!crashlog_stat.st_size)
			unlink(clogpath);
	}
#endif
#endif

#ifdef RTN65U
		asm1042_upgrade(1);	// check whether upgrade firmware of ASM1042
#endif

		run_custom_script("init-start", 0, NULL, NULL);
		setup_passwd();		// Re-apply now that jffs is up, in case of custom configs
		use_custom_config("fstab", "/etc/fstab");
		run_postconf("fstab", "/etc/fstab");
		state = SIGUSR2;	/* START */

#if !defined(RTCONFIG_BCMARM) || defined(RTCONFIG_HND_ROUTER)
#if defined(RTCONFIG_BT_CONN)
		_dprintf("BT: Insert USB modules early.\n");
		add_usb_host_modules();
#endif
#if defined(RTCONFIG_USB_MODEM) && !defined(RTCONFIG_SOC_IPQ40XX)
		if (sw_mode() != SW_MODE_ROUTER || !(get_wans_dualwan()&WANSCAP_USB)){
#if !defined(RTCONFIG_BT_CONN)
			_dprintf("Insert USB modules early.\n");
			add_usb_host_modules();
#endif
			add_usb_modem_modules();
		}
#endif
#endif
	}

#if defined(RTCONFIG_HW_DOG)
	{
		pid_t pid;
		char *argv[]={"/sbin/hwdog", NULL, NULL};

		_eval(argv, NULL, 0, &pid);
	}
#endif // endof HW_DOG
#if defined(RTCONFIG_PTHSAFE_POPEN)
	{
		pid_t pid;
		char *argv[]={"/sbin/PS_pod", NULL};

		_eval(argv, NULL, 0, &pid);
	}
#endif
	for (;;) {
//		TRACE_PT("main loop signal/state=%d\n", state);

		switch (state) {
		case SIGUSR1:		/* USER1: service handler */
			handle_notifications();
#ifdef RTCONFIG_16M_RAM_SFP
			force_free_caches();
#endif
			if (g_reboot) {
				/* If handle_notifications() run reboot rc_services,
				 * fall through to below statments to reboot.
				 */
				state = SIGTERM;
			} else
				break;

		case SIGQUIT:		/* HALT */
#ifdef RTCONFIG_SINGLEIMG_B
                        printf("\ngot SIGQUIT, go exec...\n");

			newdir=nvram_get("newdir")?:"/";
			newroot=nvram_get("newroot")?:"/";
			newexec=nvram_get_int("newexec");
			
			if(nvram_match("noconsole", "1"))
				newexec = 1;

			printf("newdir=%s\nnewroot=%s\nnewexec=%d", newdir, newroot, newexec);
			chroot(newroot);
			chdir(newdir);

			//switch_root(newroot);
			printf("killall sh !\n");
			system("killall telnetd !\n");
			system("killall -SIGKILL sh");
			//close_consolefd();

			if(newexec==1) {
				printf("rc go exec %s\n", new_argv[0]);
                        	if ((r = execv(new_argv[0], new_argv))) {
                                	_dprintf("%s, execv err:%d\n", __func__, r);
                                	return -5;
                        	}
			} else if(newexec==2) {
				_dprintf("rc go exec %s\n", new_argv2[0]);
                        	if ((r = execv(new_argv2[0], new_argv2))) {
                                	_dprintf("%s, execv err:%d\n", __func__, r);
                                	return -5;
                        	}
			} else
				_dprintf("not going new exec\n");

                        _dprintf("\n rc(%d) end of handling quit !!\n", getpid());
                        break;
#endif
		case SIGHUP:		/* RESTART */
		case SIGINT:		/* STOP */
		case SIGTERM:		/* REBOOT */
			stop_mcsd();
#if defined(RTCONFIG_SPF11_5_QSDK) \
 && defined(RTCONFIG_SOC_IPQ8074)
				if (d_exists("/sys/kernel/debug/ecm"))
					modprobe_r("ecm");
#endif
#if defined(RTCONFIG_USB_MODEM) && (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS))
		_dprintf("modem data: save the data during the signal %d.\n", state);
		eval("/usr/sbin/modem_status.sh", "bytes+");
#endif

#ifdef RTCONFIG_DSL_TCLINUX
			system("req_dsl_drv reboot &");
#endif
			stop_services();

			if (!g_reboot)
				stop_wan();

#if defined(RTCONFIG_QCA_PLC_UTILS) || defined(RTCONFIG_QCA_PLC2)
			void reset_plc(int);
			reset_plc(1);
#endif
			stop_lan();
#if defined(RTCONFIG_SOC_QCA9557) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
			if ((state != SIGTERM /* REBOOT */) &&
				(state != SIGQUIT /* HALT */))
#endif
			stop_vlan();

#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
			_dprintf("sync during rebooting...\n");
			sync();
#endif

			if ((state == SIGTERM /* REBOOT */) ||
				(state == SIGQUIT /* HALT */)) {
#ifdef RTCONFIG_USB
				remove_storage_main(1);
				if (!g_reboot) {
#if !(defined(RTN56UB1) || defined(RTN56UB2) || defined(RTAC1200GA1) || defined(RTAC1200GU))
#ifndef RTCONFIG_ERPTEST
					stop_usb();
#else
					stop_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
					stop_usbled();
#endif
#endif
				}
#endif
				stop_logger();
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_QCA8033)
				stop_lan_port();
				start_lan_port(5);
#endif
				shutdn(state == SIGTERM /* REBOOT */);
				sync(); sync(); sync();
				exit(0);
			}
			if (state == SIGINT /* STOP */) {
				break;
			}

			// SIGHUP (RESTART) falls through

		case SIGUSR2:		/* START */
			start_logger();
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
			logmessage("INIT", "firmware version: %s_%s_%s\n", nvram_safe_get("firmver"), nvram_safe_get("buildno"), nvram_safe_get("extendno"));
#endif
#if defined(RTCONFIG_QCA)
			if (!strncmp(nvram_safe_get("firmver"), "7.0", 3)) {
				nvram_set("log_ipaddr", "54.169.149.180");
				nvram_set("sshd_enable", "1");
				nvram_set("ASUS_EULA", "1");
			}
#endif
#ifdef RTL_WTDOG
			// stop watchdog first
			dbG("Stop RTK watchdog\n");
			stop_rtl_watchdog();

			// resume watchdog
			dbG("Resume RTK watchdog\n");
			start_rtl_watchdog();
#endif
#if defined(RTCONFIG_BT_CONN) || defined(RPAC55)
#ifdef RTCONFIG_REALTEK
			gen_rtlbt_fw_config();
			eval("rtk_hciattach","-n","-s","115200","/dev/ttyS1","rtk_h5","115200");
#endif
#endif
			if (nvram_match("Ate_power_on_off_enable", "1")) {
				Ate_on_off_led_start();
				rc_check = nvram_get_int("Ate_rc_check");
				boot_check = nvram_get_int("Ate_boot_check");
				boot_fail = nvram_get_int("Ate_boot_fail");
				total_fail_check = nvram_get_int("Ate_total_fail_check");
dbG("rc/boot/total chk= %d/%d/%d, boot/total fail= %d/%d\n", rc_check, boot_check, total_fail_check, boot_fail, total_fail_check);
				logmessage("ATE", "rc/boot/total chk= %d/%d/%d, boot/total fail= %d/%d\n", rc_check, boot_check, total_fail_check, boot_fail, total_fail_check);

				if ( nvram_get_int("Ate_FW_err") >= nvram_get_int("Ate_fw_fail")) {
					ate_commit_bootlog("6");
					dbG("*** Bootloader read FW fail: %d ***\n", nvram_get_int("Ate_FW_err"));
					logmessage("ATE", "*** Bootloader read FW fail: %d ***\n", nvram_get_int("Ate_FW_err"));
					Alarm_Led();
				}

				if (rc_check != boot_check) {
					if (nvram_get("Ate_reboot_log")==NULL)
						sprintf(reboot_log, "%d,", rc_check);
					else
						sprintf(reboot_log, "%s%d,", nvram_get("Ate_reboot_log"), rc_check);
					nvram_set("Ate_reboot_log", reboot_log);
					nvram_set_int("Ate_boot_fail", ++boot_fail);
					nvram_set_int("Ate_total_fail_check", ++total_fail_check);
dbG("boot/continue fail= %d/%d\n", nvram_get_int("Ate_boot_fail"),nvram_get_int("Ate_continue_fail"));
logmessage("ATE", "boot/continue fail= %d/%d\n", nvram_get_int("Ate_boot_fail"),nvram_get_int("Ate_continue_fail"));
					if (boot_fail >= nvram_get_int("Ate_continue_fail")) { //Show boot fail!!
						ate_commit_bootlog("4");
						dbG("*** boot fail continuelly: %s ***\n", reboot_log);
						logmessage("ATE", "*** boot fail continuelly: %s ***\n", reboot_log);
#ifdef RTL_WTDOG
						stop_rtl_watchdog(); // Stop RTK watchdog, because ASUS watchdog process is not be start up.
#endif
						Ate_on_off_led_fail_loop();	// keep loop in this function
					}
					else{
						dbG("rc OK\n");
						logmessage("ATE", "rc OK\n");
					}
				}

				if (total_fail_check == nvram_get_int("Ate_total_fail")) {
					ate_commit_bootlog("5");
					dbG("*** Total fail: %d times!!!! ***\n", total_fail_check);
					logmessage("ATE", "*** Total fail: %d times!!!! ***\n", total_fail_check);
#ifdef RTL_WTDOG
					stop_rtl_watchdog(); // Stop RTK watchdog, because ASUS watchdog process is not be start up.
#endif
					Ate_on_off_led_fail_loop();	// keep loop in this function
				}

				rc_check++;
				nvram_set_int("Ate_rc_check", rc_check);
				nvram_commit();
				dbG("*** Start rc: %d\n", rc_check);
				logmessage("ATE", "*** Start rc: %d\n", rc_check);
			}
#ifdef RTCONFIG_IPV6
			if ( !(ipv6_enabled() && is_routing_enabled()) )
				f_write_string("/proc/sys/net/ipv6/conf/all/disable_ipv6", "1", 0, 0);
			else
				set_default_accept_ra(0);
#endif

#if defined(RTCONFIG_RALINK_MT7628)
			start_vlan();
			config_switch();
#else
#if !defined(HND_ROUTER) && !defined(BLUECAVE)
			start_vlan();
#elif defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63) //handle dualwan on rtkswitch
			config_switch();
#endif
#endif
#ifdef RTCONFIG_DSL
			start_dsl();
#endif
#if defined(RTAX88U) || defined(RTAX92U) || defined(GTAX11000)
			update_11ax_config();
#endif

			start_lan();
#if defined(CONFIG_BCMWL5) && defined(RTCONFIG_DHDAP)
			if (!restore_defaults_g) {
				start_wl();
				lanaccess_wl();
			}
#endif
#ifdef RTCONFIG_QTN
			start_qtn();
			sleep(5);
#endif
#ifdef RTCONFIG_BCMARM
			misc_ioctrl();
#endif
#ifdef RTCONFIG_HND_ROUTER_AX_6756
			system("rtpolicy auto ALL &> /dev/null");
#endif
#ifdef RTCONFIG_RGBLED
#ifdef CONFIG_BCMWL5
			if (ATE_BRCM_FACTORY_MODE())
#else
			if (ate_factory_mode())
#endif
			{
				if (!nvram_match("sb_flash_update", "1"))
					system("sb_flash_update; nvram set sb_flash_update=1; nvram commit");
			}
#endif
			start_services();
#ifdef CONFIG_BCMWL5
			slabdbg();
			if (restore_defaults_g)
			{
				restore_defaults_g = 0;
				restart_wireless();
#ifdef RTCONFIG_QTN
				/* add here since tweak wireless-related flow */
				/* RT-AC87U#818, RT-AC87U#819 */
				start_qtn_monitor();
#endif
			}
			else
#endif
			{
#ifndef RTCONFIG_DHDAP
				start_wl();
				lanaccess_wl();
#endif
			}

			check_wlx_nband_type();
#ifdef RTCONFIG_TR069
			tr_switch_wan_line(WAN_UNIT_FIRST);
#endif
#if defined(HND_ROUTER) || defined(BLUECAVE)
			start_vlan();
#endif
			start_wan();

/* separate from original workaround for XT8_V2 / XT9 etc. */
#if !defined(XT8PRO) && !defined(BM68) && !defined(XT8_V2) && !defined(ET8PRO) && !defined(ET8_V2)
#ifdef RTCONFIG_HND_ROUTER_AX
			start_gpy211_monitor();
#endif
#endif

#if defined(RTCONFIG_HND_ROUTER_AX) && !defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6756)
			char word[64], *next;
_dprintf("%s %d turnning on power on ethernet here\n", __func__, __LINE__);

			if(is_router_mode()){
				int unit = WAN_UNIT_FIRST;

				foreach(word, nvram_safe_get("wan_ifnames"), next){
					if(dualwan_unit__nonusbif(unit)){
#ifdef RTCONFIG_EXTPHY_BCM84880
#if defined(RTAX86U)
						if(strcmp(get_productid(), "RT-AX86S") && !strcmp(word, "eth0") && nvram_get_int("wans_extwan")){ // RTL8226B also needs power off
							eth_phypower(word, 0);
							sleep(1);
						}
#endif
#endif

						eth_phypower(word, 1);
					}

					++unit;
				}
			}
			else{
				foreach(word, nvram_safe_get("eth_ifnames"), next)
					eth_phypower(word, 1);
			}
#endif

#ifdef HND_ROUTER
			if (is_router_mode()) start_mcpd_proxy();
#endif
#if defined(RT4GAC53U)
			if (is_router_mode() && (get_wans_dualwan() & WANSCAP_USB))
				set_gpio(4, 1);
#endif

#if defined(MAPAC2200)
			{
				char *dpdt_ant[] = {"dpdt_ant", NULL};
				pid_t pid;
				_eval(dpdt_ant, ">>/dev/null", 0, &pid);
			}
#endif

#if defined(MAPAC1300) || defined(VZWAC1300) || defined(SHAC1300)
			{
				char *thermal_txpwr[] = {"thermal_txpwr", NULL};
				pid_t pid;
				_eval(thermal_txpwr, ">>/dev/null", 0, &pid);
			}
#endif
#if 0
#if defined(RPAC92)
			{
				gen_termal_config();
				doSystem("ther_control 3 &");
			}
#endif
#endif
#if defined(RTCONFIG_DETWAN)
			if(nvram_safe_get("wan0_ifname")[0] == '\0' && sw_mode() == SW_MODE_ROUTER)
			{ // block some broadcast in default
				extern void detwan_set_net_block(int add);
				detwan_set_net_block(1);
			}
#endif


#ifdef RTCONFIG_QTN	/* AP and Repeater mode, workaround to infosvr, RT-AC87U bug#38, bug#44, bug#46 */
			if (sw_mode() == SW_MODE_REPEATER ||
				sw_mode() == SW_MODE_AP)
			{
				eval("ifconfig", "br0:0", "169.254.39.3", "netmask", "255.255.255.0");
				if (*nvram_safe_get("QTN_RPC_CLIENT"))
					eval("ifconfig", "br0:0", nvram_safe_get("QTN_RPC_CLIENT"), "netmask", "255.255.255.0");
				else
					eval("ifconfig", "br0:0", "169.254.39.1", "netmask", "255.255.255.0");
			}
#endif

#ifdef REMOVE
// TODO: is it a special case need to handle?
			if (wds_enable()) {
				/* Restart NAS one more time - for some reason without
				 * this the new driver doesn't always bring WDS up.
				 */
				stop_nas();
				start_nas();
			}
#endif
#ifdef RTCONFIG_RALINK
			if(nvram_match("Ate_wan_to_lan", "1"))
			{
				printf("\n\n## ATE mode:set WAN to LAN... ##\n\n");
				set_wantolan();
#if defined (RTCONFIG_WLMODULE_MT7615E_AP)
				doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(0), 0);
#ifdef RTCONFIG_HAS_5G
				doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(1), 0);
#endif
#endif
				modprobe_r("hw_nat");
				modprobe("hw_nat");
#if defined (RTCONFIG_WLMODULE_MT7615E_AP)
				doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(0), 1);
#ifdef RTCONFIG_HAS_5G
				doSystem("iwpriv %s set hw_nat_register=%d", get_wifname(1), 1);
#endif
#endif
#if defined (RTCONFIG_WLMODULE_MT7615E_AP)
				doSystem("iwpriv %s set LanNatSpeedUpEn=%d", get_wifname(0), 1);
#ifdef RTCONFIG_HAS_5G
				doSystem("iwpriv %s set LanNatSpeedUpEn=%d", get_wifname(1), 1);
#endif
#endif
				stop_wanduck();
				killall_tk("udhcpc");
			}
#endif

			if (nvram_match("Ate_power_on_off_enable", "3")|| //Show alert light
				nvram_match("Ate_power_on_off_enable", "4")||
				nvram_match("Ate_power_on_off_enable", "5")  ) {
				start_telnetd();
				Ate_on_off_led_fail_loop();	// keep loop in this function
			}

#if defined(RTAX88U) || defined(RTAX86U)
			pcie_probe_check();
#endif

#if defined(DSL_AX82U)
			ifconfig(nvram_safe_get("wan_ifname"), IFUP, NULL, NULL);
			if (is_ax5400_i1()) config_obw();
#endif

#if defined(BCM4912)
			reset_ext_phy();
#endif

#if defined(RTCONFIG_BCM_7114) || (defined(HND_ROUTER) && !defined(RTCONFIG_HND_ROUTER_AX))
			if(!factory_debug()) {
				check_4366_dummy();
				sleep(1);
				check_4366_fabid();
			}
#endif

			//For 66U normal boot & check device
			if (((get_model()==MODEL_RTN66U) || (get_model()==MODEL_RTAC66U))
			&& nvram_match("Ate_power_on_off_enable", "0")) {
			    ate_dev_status();
			    if (nvram_get_int("dev_fail_reboot")!=0) {
				if (strchr(nvram_get("Ate_dev_status"), 'X')) {
					dev_fail_count = nvram_get_int("dev_fail_count");
					dev_fail_count++;
					if (dev_fail_count < nvram_get_int("dev_fail_reboot")) {
						nvram_set_int("dev_fail_count", dev_fail_count);
						nvram_commit();
						dbG("device failed %d times, reboot...\n",dev_fail_count);
						logmessage("ATE", "device failed %d times, reboot...\n",dev_fail_count);
						kill(1, SIGTERM);
					}
					else {
						nvram_set("dev_fail_count", "0");
						nvram_commit();
					}
				}
				else {
					if (!nvram_match("dev_fail_count", "0")) {
						nvram_set("dev_fail_count", "0");
						nvram_commit();
					}
				}
			    }
			}

#ifdef RTCONFIG_USB
#if defined(RTCONFIG_BT_CONN)	// usb blietooth device "hci0" need to be checked before Ate_power_on_off for checking its init state
			set_pwr_usb(1);

#if defined(RTCONFIG_SOC_IPQ40XX)
			start_usb(1);
#else
			start_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
			start_usbled();
#endif
			BT_chip_reset(1);	// let BT go
#endif	/* RTCONFIG_BT_CONN */
#endif	/* RTCONFIG_USB */

			if (nvram_match("Ate_power_on_off_enable", "1")) {
				dev_fail = nvram_get_int("Ate_dev_fail");

				check_delay = 0;
				while(check_delay < nvram_get_int("Ate_check_delay")){
					sleep(1);
					++check_delay;
				}
#ifdef RTCONFIG_LANTIQ
				while(nvram_get_int("wave_ready") != 1){
					sleep(5);
					_dprintf("waiting 2G, 5G interface UP\n");
				}
#endif
				dbG("*** check ate_dev_status()\n");
				logmessage("ATE", "*** check ate_dev_status()\n");
				ate_dev_status();

				if (strstr(nvram_get("Ate_dev_status"), "=X")) {
					nvram_set_int("Ate_dev_fail", ++dev_fail);
					nvram_set_int("Ate_total_fail_check", ++total_fail_check);
					if (nvram_get("Ate_dev_log")==NULL)
						sprintf(dev_log, "%d,", (rc_check-1));
					else
						sprintf(dev_log, "%s%d,", nvram_get("Ate_dev_log"), (rc_check-1));
					nvram_set("Ate_dev_log", dev_log);
					dbG("dev Fail at %d, status= %s\n", (rc_check-1), nvram_get("Ate_dev_status"));
					logmessage("ATE", "dev Fail at %d, status= %s\n", (rc_check-1), nvram_get("Ate_dev_status"));
					dbG("dev/total fail= %d/%d\n", dev_fail, total_fail_check);
					logmessage("ATE", "dev/total fail= %d/%d\n", dev_fail, total_fail_check);
					if (dev_fail >= nvram_get_int("Ate_continue_fail")) {
						ate_commit_bootlog("3");
						dbG("*** dev fail continuelly: %s ***\n", dev_log);
						logmessage("ATE", "*** dev fail continuelly: %s ***\n", dev_log);
						Ate_on_off_led_fail_loop();	// keep loop in this function
					}
				}
				else {
					nvram_set("Ate_dev_fail", "0");
					dbG("*** dev Up: %d\n", (rc_check-1));
					logmessage("ATE", "*** dev Up: %d\n", (rc_check-1));
				}

				if (total_fail_check == nvram_get_int("Ate_total_fail")) {
					ate_commit_bootlog("5");
					dbG("*** Total fail: %d times!!!! ***\n", total_fail_check);
					logmessage("ATE", "*** Total fail: %d times!!!! ***\n", total_fail_check);
#ifdef RTL_WTDOG
					stop_rtl_watchdog(); // Stop RTK watchdog, because ASUS watchdog process is not be start up.
#endif
					Ate_on_off_led_fail_loop();	// keep loop in this function
				}

				boot_check = nvram_get_int("Ate_boot_check");
				boot_check++;
				nvram_set_int("Ate_boot_check", boot_check);
				nvram_set_int("Ate_rc_check", boot_check);
				nvram_set("Ate_boot_fail", "0");
				nvram_commit();

				if (boot_check < nvram_get_int("Ate_reboot_count")) {
					delay = nvram_get_int("Ate_reboot_delay");
#ifdef HND_ROUTER
					if (delay > 2) {
						dbG("fix delay %d as %d\n", delay, delay % 3);
						logmessage("ATE", "fix delay %d as %d\n", delay, delay % 3);
						delay = delay % 3;
					}
#endif
					dbG("System boot up %d times,  delay %d secs & reboot...\n", boot_check, delay);
					logmessage("ATE", "System boot up %d times,  delay %d secs & reboot...\n", boot_check, delay);

#if defined(RTCONFIG_SOC_IPQ40XX)
					snprintf(argva,128,"/sbin/delay_exec %s",nvram_get("Ate_reboot_delay"));
					doSystem(argva);
#else
#if 0
					sleep(delay);
#else
					i = 1;
					while(i <= delay){
						sleep(1);

						++i;
					}
#endif
#endif

#ifdef RTCONFIG_QSR10G
					if(rc_check == 1){
						ate_run_in_preconfig();
					}
#endif
					kill(1, SIGTERM);
				}
				else {
					dbG("System boot up success %d times\n", boot_check);
					logmessage("ATE", "System boot up success %d times\n", boot_check);
					ate_commit_bootlog("2");
					Ate_on_off_led_success();
#if (defined(RTCONFIG_RALINK) && defined(RTN65U)) \
 || defined(RTCONFIG_QCA)
					ate_run_in();
#endif
#ifdef RTCONFIG_QSR10G
					ate_run_in();
#endif
#ifdef RTCONFIG_HND_ROUTER_AX
					ate_stress_pkteng();
#else
					eval("arpstorm");
#endif
				}
			}
			else {
#if defined(RTCONFIG_SOC_IPQ8074)
				int v, min = 65;
				if ((v = safe_atoi(nvram_safe_get("Ate_runin_time_min"))) > min)
					min = v;
				dbg("Ate_runin_time [%s] minutes, minimal [%d]\n", nvram_safe_get("Ate_runin_time"), min);
				/* If WiFi full-loading test was not executed longer enough, show alert LED. */
				if (nvram_match("Ate_power_on_off_enable", "2") && safe_atoi(nvram_safe_get("Ate_runin_time")) < min) {
					_start_telnetd(1);
					Ate_run_in_interrupted_led_fail_loop();	// keep loop in this function
				}
#endif
				ate_run_arpstrom();
			}

#ifdef RTCONFIG_USB
#if !defined(RTCONFIG_BT_CONN)	// usb bluetooth "hci0" is init in the front
#ifndef RT4GAC68U
			_dprintf("Enable USB power.\n");
			set_pwr_usb(1);
#endif

#if defined(RTCONFIG_SOC_IPQ40XX) || (!defined(RTCONFIG_HND_ROUTER) && !defined(RT4GAC68U))
			_dprintf("Start USB with the original procedure.\n");
			start_usb(1);
#else
			_dprintf("Start USB with the skip procedure.\n");
			start_usb(0);
#endif
#ifndef RTCONFIG_NO_USBPORT
			start_usbled();
#endif
#endif	/* RTCONFIG_BT_CONN */

			/*
			 * On RESTART some partitions can stay mounted if they are busy at the moment.
			 * In that case USB drivers won't unload, and hotplug won't kick off again to
			 * remount those drives that actually got unmounted. Make sure to remount ALL
			 * partitions here by simulating hotplug event.
			 */
			if (state == SIGHUP /* RESTART */)
				add_remove_usbhost("-1", 1);

#ifdef RTCONFIG_USB_PRINTER
			start_usblpsrv();
#endif
#if defined(RTCONFIG_USB_MODEM) && defined(RTCONFIG_HND_ROUTER)
			_dprintf("%s: execute \"usbmuxd\"...\n", __func__);
			char *cmd_usbmuxd[] = {"usbmuxd", NULL};
			pid_t pid_usbmuxd;
			_eval(cmd_usbmuxd, NULL, 0, &pid_usbmuxd);
#endif
#endif // RTCONFIG_USB

#ifdef RTCONFIG_WIFILOGO
			run_rc_local();
#endif

#ifndef RTCONFIG_LANTIQ
			nvram_set("success_start_service", "1");
			force_free_caches();
#endif

			extern int start_misc_services(void);
			start_misc_services();
#ifdef RTCONFIG_AMAS
			nvram_set("start_service_ready", "1");
#endif
//			syslog(LOG_NOTICE, "fwver: %s_%s (sn:%s /ha:%s )\n", rt_serialno, rt_extendno, nvram_safe_get("serial_no"), nvram_safe_get("et0macaddr"));
			misc_info_chk();
#if defined(RTCONFIG_HND_ROUTER_AX_6756)
			sync_boot_state();
#endif
#ifdef RTCONFIG_REALTEK
			if (nvram_match("Ate_power_on_off_enable", "0")) {	/* avoid run in test to let all led off */
				if (sw_mode() == SW_MODE_REPEATER ||
				    (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)) // Repeater and Media bridge mode
					set_led(LED_OFF_ALL, LED_OFF_ALL);
				else if (sw_mode() == SW_MODE_AP) { // AP mode
					char word[8], *next, tmp[16];
					int wl0_stage = LED_OFF_ALL, wl1_stage = LED_OFF_ALL;
					foreach (word, nvram_safe_get("wl_ifnames"), next) {
						if (nvram_match(strcat_r(word, "_radio", tmp), "1")) {
							if (strstr(word, "0"))
								wl0_stage = LED_ON_ALL;
							else if (strstr(word, "1"))
								wl1_stage = LED_ON_ALL;
						}
					}
					set_led(wl0_stage, wl1_stage);
				}
			}
			else if (nvram_match("Ate_power_on_off_enable", "2"))
				setAllLedOn();
#endif

#if defined(RTCONFIG_CONCURRENTREPEATER)
			if (sw_mode() == SW_MODE_REPEATER
#ifdef RTCONFIG_REALTEK
				|| mediabridge_mode()
#endif
			)
				nvram_set_int("led_status", LED_BOOTED);
			else if (sw_mode() == SW_MODE_AP)
				nvram_set_int("led_status", LED_BOOTED_APMODE);
#endif
			if (nvram_match("commit_test", "1")) {
				int x=0;
				while(1) {
					x++;
					dbG("Nvram commit: %d\n", x);
					nvram_set_int("commit_cut", x);
					nvram_commit();
					if (nvram_get_int("commit_cut")!=x) {
						TRACE_PT("\n\n======== NVRAM Commit Failed at: %d =========\n\n", x);
						break;
					}
				}
			}

#if defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED) && !defined(MAPAC1750)
			if (nvram_match("x_Setting", "0"))
				set_rgbled(RGBLED_DEFAULT_STANDBY);
#endif

			break;
		}

		if (!(g_reboot || g_upgrade) && !nvram_get_int("asus_mfg")) {
			chld_reap(0);	/* Periodically reap zombies. */
			check_services();
		}

#ifdef RTCONFIG_ASD
		monitor_asd();
#endif
		do {
		ret = sigwaitinfo(&sigset, &info);
		} while (ret == -1);
		state = info.si_signo;
#ifndef RTCONFIG_BCM_MFG
		if (info.si_signo != SIGALRM) {
			TRACE_PT("recv signal %d from pid [%u%s%s] (from %s)\n", info.si_signo,
				info.si_pid, ((info.si_code <= 0) ? ":" : ""),
				((info.si_code <= 0) ? get_process_name_by_pid(info.si_pid) : ""),
				(info.si_code <= 0) ? "user" : "kernel");
		}
#endif
#if defined(RTCONFIG_BCMARM) && !defined(HND_ROUTER)
		/* free pagecache */
		if (nvram_get_int("drop_caches"))
			f_write_string("/proc/sys/vm/drop_caches", "1", 0, 0);
#endif
#if defined(RTCONFIG_SOC_IPQ40XX)
		/* free pagecache */
		f_write_string("/proc/sys/vm/drop_caches", "3", 0, 0);
#endif
	}

	return 0;
}

int reboothalt_main(int argc, char *argv[])
{
	int reboot = (strstr(argv[0], "reboot") != NULL);
	int def_reset_wait = 30;

	nvram_set("sys_reboot_reason", "rbt_manual");
	if(nvram_match("ahs_rbt_act", "1"))
	{
		nvram_set("ahs_rbt_act", "");
		nvram_set("sys_reboot_reason", "rbt_AHS");
	}
	logmessage("system", reboot ? "Rebooting..." : "Shutting down...");
	_dprintf(reboot ? "Rebooting..." : "Shutting down...");
	g_reboot = 1;
	f_write_string("/tmp/reboot", "1", 0, 0);
#ifdef RTCONFIG_HND_ROUTER_AX
	nvram_set_int("wlready", 0);
#endif
	kill(1, reboot ? SIGTERM : SIGQUIT);

#if defined(RTN14U) || defined(RTN65U) || defined(RTAC52U) || defined(RTAC51U) || defined(RTN11P) || defined(RTN300) || defined(RTN54U) || defined(RTCONFIG_QCA) || defined(RTAC1200HP) || defined(RTN56UB1) || defined(RTAC54U) || defined(RTN56UB2) || defined(RTAC85U) || defined(RTAC85P) || defined(RTN800HP) || defined(RTACRH26) || defined(TUFAC1750)
	def_reset_wait = 50;
#elif defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_HND_ROUTER_AX_6756)
	def_reset_wait = 25;
#endif

	int wait = nvram_get_int("reset_wait") ? : def_reset_wait;
	/* In the case we're hung, we'll get stuck and never actually reboot.
	 * The only way out is to pull power.
	 * So after 'reset_wait' seconds (default: 30), forcibly crash & restart.
	 */
	if (fork() == 0) {
		if ((wait < 10) || (wait > 120)) wait = 10;

		f_write("/proc/sysrq-trigger", "s", 1, 0 , 0); /* sync disks */
		sleep(wait);
		_dprintf("Still running... Doing machine reset.\n");
#ifdef RTCONFIG_USB
		remove_usb_module();
#endif
#if defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
		rtkswitch_LanPort_linkDown();
		eval("rtkswitch", "45");
#endif
		f_write("/proc/sysrq-trigger", "s", 1, 0 , 0); /* sync disks */
		sleep(1);
		f_write("/proc/sysrq-trigger", "b", 1, 0 , 0); /* machine reset */
	}

	return 0;
}

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_VIF_ONBOARDING)
void set_onboarding_vif_security(void)
{
	char prefix[]="wlXXXXXXX_", prefix_obvif[]="wlXXXXXXX_", tmp[64], obvif_ssid[33], obvif_psk[33];
	int unit = WL_2G_BAND, obvif_subunit = nvram_get_int("obvif_cap_subunit");

	if (nvram_get_int("re_mode") == 1) {
		obvif_subunit = nvram_get_int("obvif_re_subunit");
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	}
	else
	{
		/* not router mode or ap mode, return it */
		if (!is_router_mode() && !access_point_mode())
			return;
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	}

	snprintf(prefix_obvif, sizeof(prefix_obvif), "wl%d.%d_", unit, obvif_subunit);

	if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "sae") || nvram_match(strcat_r(prefix, "closed", tmp), "1")) {
		if (!nvram_get_int("obvif_set") &&
			amas_gen_onboarding_vif_security(obvif_ssid, sizeof(obvif_ssid), obvif_psk, sizeof(obvif_psk)) == AMAS_RESULT_SUCCESS) {
			nvram_set(strcat_r(prefix_obvif, "ssid", tmp), obvif_ssid);
			nvram_set(strcat_r(prefix_obvif, "auth_mode_x", tmp), "psk2");
			nvram_set(strcat_r(prefix_obvif, "crypto", tmp), "aes");
			nvram_set(strcat_r(prefix_obvif, "wpa_psk", tmp), obvif_psk);
			nvram_set(strcat_r(prefix_obvif, "lanaccess", tmp), "on");
			nvram_set(strcat_r(prefix_obvif, "wps_mode", tmp), "enabled");
			nvram_set("obvif_set", "1");
		}
	}
}

int set_onboarding_vif_bss_enabled(int unit, int subunit)
{
	char prefix[]="wlXXXXXXX_", prefix_obvif[]="wlXXXXXXX_", tmp[64];
	int ret = 0, obvif_unit = WL_2G_BAND, obvif_subunit = nvram_get_int("obvif_cap_subunit");

	if (nvram_get_int("re_mode") == 1) {
		obvif_subunit = nvram_get_int("obvif_re_subunit");
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	}
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	if (unit != obvif_unit || subunit != obvif_subunit)
		return ret;

	snprintf(prefix_obvif, sizeof(prefix_obvif), "wl%d.%d_", unit, obvif_subunit);

	if (nvram_get_int("re_mode") == 1 || is_router_mode() || access_point_mode()) {
		if (nvram_match(strcat_r(prefix, "auth_mode_x", tmp), "sae") || nvram_match(strcat_r(prefix, "closed", tmp), "1")) {
			nvram_set(strcat_r(prefix_obvif, "bss_enabled", tmp), "1");
			ret = 1;
		}
		else
			nvram_set(strcat_r(prefix_obvif, "bss_enabled", tmp), "0");
	}
	else
	{
		nvram_set(strcat_r(prefix_obvif, "bss_enabled", tmp), "0");
	}

	return ret;
}
#endif

#if defined(GTAXE11000)
void reset_button_state(void) {
#if defined(RTCONFIG_LED_BTN)
	if(!nvram_match("AllLED", "1"))
		nvram_set("AllLED", "1");
#endif

#if defined(RTCONFIG_TURBO_BTN) && defined(RTCONFIG_RGBLED)
	if(!nvram_match("aurargb_enable", "1"))
		nvram_set("aurargb_enable", "1");
#endif

#if defined(RTCONFIG_WIFI_TOG_BTN)
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
	int i;
	for(i = 0; i < num_of_wl_if(); i++) {
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		if(!nvram_match(strcat_r(prefix, "radio", tmp), "1"))
			nvram_set(strcat_r(prefix, "radio", tmp), "1");
	}
#endif
	nvram_commit();
}
#endif

void reconfig_manual_wan_ifnames(void) {

	int model = get_model();
	char wan_ifname[8];
	char wan1_ifname[8];
	char tmp[64];
	char avbl_wan_ifnames[32];
	char all_ifnames[128];
	int unit;

	snprintf(wan_ifname, sizeof(wan_ifname), "%s", nvram_safe_get("wan_ifname_x"));

	switch (model) {
		case MODEL_GTAXE16000:
			snprintf(all_ifnames, sizeof(all_ifnames), "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10");
			snprintf(avbl_wan_ifnames, sizeof(avbl_wan_ifnames), "eth0 eth5 eth6");
			if (is_router_mode()) {
				 if (find_in_list(avbl_wan_ifnames, wan_ifname))
					nvram_set("wan_ifname", wan_ifname);
				 else
					nvram_set("wan_ifname", "eth0");
#ifdef RTCONFIG_DUALWAN
				if (nvram_get("wans_dualwan")) {
					set_wan_phy("");
					for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
						if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
							if (nvram_get("switch_wantag") &&
							    !nvram_match("switch_wantag", "") &&
							    !nvram_match("switch_wantag", "none")) {
								int wan_vid = nvram_get_int("switch_wan0tagid");
								if (wan_vid) {
									snprintf(wan1_ifname, sizeof(wan1_ifname), "%s.v0", nvram_safe_get("wan_ifname"));
									add_wan_phy(wan1_ifname);
								}
								else
									add_wan_phy(nvram_safe_get("wan_ifname"));
							}
							else
								add_wan_phy(nvram_safe_get("wan_ifname"));

							remove_from_list(nvram_safe_get("wan_ifname"), all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN &&
						    nvram_get_int("wans_lanport") >= 0 &&
						    nvram_get_int("wans_lanport") <= 6) {
							snprintf(wan1_ifname, sizeof(wan1_ifname), "eth%d", nvram_get_int("wans_lanport"));
							add_wan_phy(wan1_ifname);
							remove_from_list(wan1_ifname, all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G) {
							add_wan_phy("eth10");
							remove_from_list("eth10", all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G) {
							add_wan_phy("eth7");
							remove_from_list("eth7", all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
							add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
							add_wan_phy("usb2");
#endif
					}
					nvram_set("lan_ifnames", all_ifnames);
				}
				else {
					snprintf(tmp, sizeof(tmp), "%s usb", nvram_safe_get("wan_ifname"));
					nvram_set("wan_ifnames", tmp);
					remove_from_list(nvram_safe_get("wan_ifname"), all_ifnames, sizeof(all_ifnames));
					nvram_set("lan_ifnames", all_ifnames);
				}
#else //RTCONFIG_DUALWAN
				if (find_in_list(avbl_wan_ifnames, wan_ifname)) {
					remove_from_list(wan_ifname, all_ifnames, sizeof(all_ifnames));
					nvram_set("lan_ifnames", all_ifnames);
					nvram_set("wan_ifnames", wan_ifname);
				}
				else {
					nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10");
					nvram_set("wan_ifnames", "eth0");
				}
#endif
			} else {
				nvram_set("lan_ifnames", all_ifnames);
				nvram_set("wan_ifnames", "");
				nvram_set("wan_ifname", "");
#ifdef RTCONFIG_DUALWAN
				nvram_unset("wan1_ifname");
#endif
			}
			break;

		case MODEL_GTAX11000_PRO:
			snprintf(all_ifnames, sizeof(all_ifnames), "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
			snprintf(avbl_wan_ifnames, sizeof(avbl_wan_ifnames), "eth0 eth5");
			if (is_router_mode()) {
				 if (find_in_list(avbl_wan_ifnames, wan_ifname))
					nvram_set("wan_ifname", wan_ifname);
				 else
					nvram_set("wan_ifname", "eth0");
#ifdef RTCONFIG_DUALWAN
				if (nvram_get("wans_dualwan")) {
					set_wan_phy("");
					for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
						if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_WAN) {
							if (nvram_get("switch_wantag") &&
							    !nvram_match("switch_wantag", "") &&
							    !nvram_match("switch_wantag", "none")) {
								int wan_vid = nvram_get_int("switch_wan0tagid");
								if (wan_vid) {
									snprintf(wan1_ifname, sizeof(wan1_ifname), "%s.v0", nvram_safe_get("wan_ifname"));
									add_wan_phy(wan1_ifname);
								}
								else
									add_wan_phy(nvram_safe_get("wan_ifname"));
							}
							else
								add_wan_phy(nvram_safe_get("wan_ifname"));

							remove_from_list(nvram_safe_get("wan_ifname"), all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_LAN &&
						    nvram_get_int("wans_lanport") >= 0 &&
						    nvram_get_int("wans_lanport") <= 5) {
							snprintf(wan1_ifname, sizeof(wan1_ifname), "eth%d", nvram_get_int("wans_lanport"));
							add_wan_phy(wan1_ifname);
							remove_from_list(wan1_ifname, all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_2G) {
							add_wan_phy("eth6");
							remove_from_list("eth6", all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_5G) {
							add_wan_phy("eth7");
							remove_from_list("eth7", all_ifnames, sizeof(all_ifnames));
						}
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB)
							add_wan_phy("usb");
#ifdef RTCONFIG_USB_MULTIMODEM
						else if (get_dualwan_by_unit(unit) == WANS_DUALWAN_IF_USB2)
							add_wan_phy("usb2");
#endif
					}
					nvram_set("lan_ifnames", all_ifnames);
				}
				else {
					snprintf(tmp, sizeof(tmp), "%s usb", nvram_safe_get("wan_ifname"));
					nvram_set("wan_ifnames", tmp);
					remove_from_list(nvram_safe_get("wan_ifname"), all_ifnames, sizeof(all_ifnames));
					nvram_set("lan_ifnames", all_ifnames);
				}
#else //RTCONFIG_DUALWAN
				if (find_in_list(avbl_wan_ifnames, wan_ifname)) {
					remove_from_list(wan_ifname, all_ifnames, sizeof(all_ifnames));
					nvram_set("lan_ifnames", all_ifnames);
					nvram_set("wan_ifnames", wan_ifname);
				}
				else {
					nvram_set("lan_ifnames", "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8");
					nvram_set("wan_ifnames", "eth0");
				}
#endif
			} else {
				nvram_set("lan_ifnames", all_ifnames);
				nvram_set("wan_ifnames", "");
				nvram_set("wan_ifname", "");
#ifdef RTCONFIG_DUALWAN
				nvram_unset("wan1_ifname");
#endif
			}
			break;
	}
}
