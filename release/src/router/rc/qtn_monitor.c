/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2014, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlutils.h>
#include <shared.h>
#include <qtn/qtn_vlan.h>

#ifdef RTCONFIG_QTN
#include "web-qtn.h"

static void
cleanup(void)
{
	remove("/var/run/qtn_monitor.pid");
	exit(0);
}

/* #565: Access Intranet off */
void create_mbssid_vlan(int unit, int subunit)
{
	char prefix_bss_enabled[] = "wlXXX_bss_enabled";
	char prefix_lanaccess[] = "wlXXX_lanaccess";
	char vlan_id[] = "4000";
	char vlan_name[] = "vlan4000";
	char vlan_index[] = "0x0fa0";
	char mbsswifi_name[] = "wifi1";
	qcsapi_vlan_cmd cmd = 0;

	if(unit != 1) return;
	if(subunit == -1 || subunit == 0) return;

	snprintf(prefix_bss_enabled, sizeof(prefix_bss_enabled),
					"wl%d.%d_bss_enabled", unit, subunit);
	snprintf(prefix_lanaccess, sizeof(prefix_lanaccess),
					"wl%d.%d_lanaccess", unit, subunit);

	snprintf(vlan_id, sizeof(vlan_id), "%d", 4000+subunit-1);
	snprintf(vlan_name, sizeof(vlan_name), "vlan%d", 4000+subunit-1);
	snprintf(vlan_index, sizeof(vlan_index), "0x%04x", 4000+subunit-1);
	snprintf(mbsswifi_name, sizeof(mbsswifi_name), "wifi%d", subunit);

	logmessage("mbss", "prefix_bss_enabled:[%s][%s]",
		prefix_bss_enabled, nvram_safe_get(prefix_bss_enabled));
	logmessage("mbss", "prefix_lanaccess:[%s][%s]",
		prefix_lanaccess, nvram_safe_get(prefix_lanaccess));
	if(nvram_get_int(prefix_bss_enabled) == 1){
		if(nvram_match(prefix_lanaccess, "off")){
			/* VID 4000 */
			eval("vconfig", "add", "eth0", vlan_id);
			eval("ifconfig", vlan_name, "hw", "ether", nvram_safe_get("lan_hwaddr"), "up");
			eval("brctl", "addif", "br0", vlan_name);
			eval("et", "robowr", "0x05", "0x81", vlan_index);
			eval("et", "robowr", "0x05", "0x83", "0x00a0");
			eval("et", "robowr", "0x05", "0x80", "0x0000");
			eval("et", "robowr", "0x05", "0x80", "0x0080");

			logmessage("mbss", "dp-10 unit:[%d], subunit:[%d]", unit, subunit);
			qcsapi_wifi_vlan_config("wifi0" /* main if */,
				e_qcsapi_vlan_enable, QVLAN_VID_ALL);
			qcsapi_wifi_vlan_config("eth1_0" /* main if */,
				e_qcsapi_vlan_trunk, QVLAN_VID_ALL);
			qcsapi_wifi_vlan_config("eth1_1" /* main if */,
				e_qcsapi_vlan_trunk, 0x1);
			cmd = e_qcsapi_vlan_access | 
				e_qcsapi_vlan_untag | e_qcsapi_vlan_pvid;
			qcsapi_wifi_vlan_config(mbsswifi_name,
				cmd, atoi(vlan_id) /* vid */);
		}else{
			eval("brctl", "delif", "br0", vlan_name);
			eval("et", "robowr", "0x05", "0x81", vlan_index);
			eval("et", "robowr", "0x05", "0x83", "0x0000");
			eval("et", "robowr", "0x05", "0x80", "0x0000");
			eval("et", "robowr", "0x05", "0x80", "0x0080");

			logmessage("mbss", "dp-11 unit:[%d], subunit:[%d]", unit, subunit);
			cmd = e_qcsapi_vlan_access | e_qcsapi_vlan_del |
				e_qcsapi_vlan_untag | e_qcsapi_vlan_pvid;
			qcsapi_wifi_vlan_config(mbsswifi_name,
				cmd, atoi(vlan_id) /* vid */);
		}
	}
}

static void
qtn_monitor_exit(int sig)
{
	if (sig == SIGTERM)
		cleanup();
}

void rpc_parse_nvram_from_httpd(int unit, int subunit)
{
	int ret = 0;
	if (!rpc_qtn_ready())
		return;

	if (unit == 1 && subunit == -1){
		rpc_qcsapi_set_SSID(WIFINAME, nvram_safe_get("wl1_ssid"));
		rpc_qcsapi_set_SSID_broadcast(WIFINAME, nvram_safe_get("wl1_closed"));
		rpc_qcsapi_set_vht(nvram_safe_get("wl1_nmode_x"));
		rpc_qcsapi_set_bw(nvram_safe_get("wl1_bw"));
		rpc_qcsapi_set_channel(nvram_safe_get("wl1_chanspec"));
		rpc_qcsapi_set_beacon_type(WIFINAME, nvram_safe_get("wl1_auth_mode_x"));
		rpc_qcsapi_set_WPA_encryption_modes(WIFINAME, nvram_safe_get("wl1_crypto"));
		rpc_qcsapi_set_key_passphrase(WIFINAME, nvram_safe_get("wl1_wpa_psk"));
		rpc_qcsapi_set_dtim(nvram_safe_get("wl1_dtim"));
		rpc_qcsapi_set_beacon_interval(nvram_safe_get("wl1_bcn"));
		rpc_set_radio(1, 0, nvram_get_int("wl1_radio"));
		rpc_update_macmode(nvram_safe_get("wl1_macmode"));
		rpc_update_wlmaclist();
		rpc_update_wdslist();
		rpc_update_wdslist();
		rpc_update_wds_psk(nvram_safe_get("wl1_wds_psk"));
		rpc_update_ap_isolate(WIFINAME, atoi(nvram_safe_get("wl1_ap_isolate")));

		qcsapi_wifi_rfenable((qcsapi_unsigned_int) 0);
		if(nvram_get_int("wps_enable") == 1){
			ret = rpc_qcsapi_wifi_disable_wps(WIFINAME, 0);
			if (ret < 0)
				dbG("rpc_qcsapi_wifi_disable_wps %s error[%d]\n", WIFINAME, ret);

			ret = qcsapi_wps_set_ap_pin(WIFINAME, nvram_safe_get("wps_device_pin"));
			if (ret < 0)
				dbG("qcsapi_wps_set_ap_pin %s error[%d]\n", WIFINAME, ret);

			ret = qcsapi_wps_registrar_set_pp_devname(WIFINAME, 0, (const char *) get_productid());
			if (ret < 0)
				dbG("qcsapi_wps_registrar_set_pp_devname %s error[%d]\n", WIFINAME, ret);

		}else{
			ret = rpc_qcsapi_wifi_disable_wps(WIFINAME, 1);
			if (ret < 0)
				dbG("rpc_qcsapi_wifi_disable_wps %s error[%d]\n", WIFINAME, ret);
		}
		qcsapi_wifi_rfenable(nvram_get_int("wl1_radio"));

		ret = qcsapi_wps_upnp_enable(WIFINAME, 0);
		if (ret < 0)
			dbG("disable WPS UPnP %s error[%d]\n", WIFINAME, ret);

		if(sw_mode() == SW_MODE_ROUTER ||
			(sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)){
			if(nvram_get_int("wl1_mumimo") == 1){
				dbG("mu-mimo: enable MU-MIMO\n");
				ret = qcsapi_wifi_set_enable_mu(WIFINAME, 1);
			}else{
				dbG("mu-mimo: disable MU-MIMO\n");
				 qcsapi_wifi_set_enable_mu(WIFINAME, 0);
			}
			if (ret < 0)
				dbG("enable_mu %s error, return: %d\n", WIFINAME, ret);
		}
#ifdef RTCONFIG_IPV6
		if (get_ipv6_service() == IPV6_DISABLED)
			qcsapi_wifi_run_script("router_command.sh", "ipv6_off wifi0");
		else
			qcsapi_wifi_run_script("router_command.sh", "ipv6_on wifi0");
#endif
	}else if (unit == 1 && subunit == 1){
		if(nvram_get_int("wl1.1_bss_enabled") == 1){
			rpc_update_mbss("wl1.1_bss_enabled", nvram_safe_get("wl1.1_bss_enabled"));
#ifdef RTCONFIG_IPV6
			if (get_ipv6_service() == IPV6_DISABLED)
				qcsapi_wifi_run_script("router_command.sh", "ipv6_off wifi1");
			else
				qcsapi_wifi_run_script("router_command.sh", "ipv6_on wifi1");
#endif

			/* these should be done after rfenabled */
			if(nvram_get_int("sw_mode") == SW_MODE_ROUTER){
				create_mbssid_vlan(unit, subunit);
			}
		}
		else{
			qcsapi_wifi_remove_bss(wl_vifname_qtn(unit, subunit));
		}
	}else if (unit == 1 && subunit == 2){
		if(nvram_get_int("wl1.2_bss_enabled") == 1){
			rpc_update_mbss("wl1.2_bss_enabled", nvram_safe_get("wl1.2_bss_enabled"));
#ifdef RTCONFIG_IPV6
			if (get_ipv6_service() == IPV6_DISABLED)
				qcsapi_wifi_run_script("router_command.sh", "ipv6_off wifi2");
			else
				qcsapi_wifi_run_script("router_command.sh", "ipv6_on wifi2");
#endif

			/* these should be done after rfenabled */
			if(nvram_get_int("sw_mode") == SW_MODE_ROUTER){
				create_mbssid_vlan(unit, subunit);
			}
		}
		else{
			qcsapi_wifi_remove_bss(wl_vifname_qtn(unit, subunit));
		}
	}else if (unit == 1 && subunit == 3){
		if(nvram_get_int("wl1.3_bss_enabled") == 1){
			rpc_update_mbss("wl1.3_bss_enabled", nvram_safe_get("wl1.3_bss_enabled"));
#ifdef RTCONFIG_IPV6
			if (get_ipv6_service() == IPV6_DISABLED)
				qcsapi_wifi_run_script("router_command.sh", "ipv6_off wifi3");
			else
				qcsapi_wifi_run_script("router_command.sh", "ipv6_on wifi3");
#endif

			/* these should be done after rfenabled */
			if(nvram_get_int("sw_mode") == SW_MODE_ROUTER){
				create_mbssid_vlan(unit, subunit);
			}
		}
		else{
			qcsapi_wifi_remove_bss(wl_vifname_qtn(unit, subunit));
		}
	}

	/* disable UPNP */
	qcsapi_wps_upnp_enable(WIFINAME, 0);

//	rpc_show_config();
}

int 
qtn_monitor_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;
	int ret, retval = 0;
	time_t start_time = uptime();

	/* write pid */
	if ((fp = fopen("/var/run/qtn_monitor.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGTERM, qtn_monitor_exit);

QTN_RESET:
	ret = rpc_qcsapi_init(1);
	if (ret < 0) {
		dbG("rpc_qcsapi_init error, return: %d\n", ret);
		retval = -1;
		goto ERROR;
	}
#if 0	/* replaced by STATELESS, send configuration from brcm to qtn */
	else if (nvram_get_int("qtn_restore_defaults"))
	{
		nvram_unset("qtn_restore_defaults");
		rpc_qcsapi_restore_default_config(0);
		dbG("Restaring Qcsapi init...\n");
		sleep(15);
		goto QTN_RESET;
	}
#endif

#if 0
	if(sw_mode() == SW_MODE_AP &&
		nvram_get_int("wlc_psta") == 1 &&
		nvram_get_int("wlc_band") == 1){
		dbG("[sw_mode] start QTN PSTA mode\n");
		start_psta_qtn();
	}
#endif

	ret = qcsapi_init();
	if (ret < 0)
	{
		dbG("Qcsapi qcsapi_init error, return: %d\n", ret);
		retval = -1;
		goto ERROR;
	}
	else
		dbG("Qcsapi qcsapi init takes %ld seconds\n", uptime() - start_time);

	dbG("defer lanport_ctrl(1)\n");
	lanport_ctrl(1);
	eval("ifconfig", "br0:1", "down");
#if defined(RTCONFIG_JFFS2ND_BACKUP)
	check_2nd_jffs();
#endif
	nvram_set("qtn_ready", "1");

	if(nvram_get_int("AllLED") == 0) setAllLedOff();

	// dbG("[QTN] update router_command.sh from brcm to qtn\n");
	// qcsapi_wifi_run_script("set_test_mode", "update_router_command");

#if 1	/* STATELESS */
	if(sw_mode() == SW_MODE_AP &&
		nvram_get_int("wlc_psta") == 1 &&
		nvram_get_int("wlc_band") == 1){
		dbG("[sw_mode] skip start_psta_qtn, QTN will run scripts automatically\n");
		// start_psta_qtn();
	}else{
		dbG("[***] rpc_parse_nvram_from_httpd\n");
		rpc_parse_nvram_from_httpd(1,-1);	/* wifi0 */
		rpc_parse_nvram_from_httpd(1,1);	/* wifi1 */
		rpc_parse_nvram_from_httpd(1,2);	/* wifi2 */
		rpc_parse_nvram_from_httpd(1,3);	/* wifi3 */
		dbG("[sw_mode] skip start_ap_qtn, QTN will run scripts automatically\n");
		// start_ap_qtn();
		qcsapi_mac_addr wl_mac_addr;
		ret = rpc_qcsapi_interface_get_mac_addr(WIFINAME, &wl_mac_addr);
		if (ret < 0)
			dbG("rpc_qcsapi_interface_get_mac_addr, return: %d\n", ret);
		else
		{
			nvram_set("1:macaddr", wl_ether_etoa((struct ether_addr *) &wl_mac_addr));
			nvram_set("wl1_hwaddr", wl_ether_etoa((struct ether_addr *) &wl_mac_addr));
		}

		rpc_update_wdslist();


	}
#endif

	if(nvram_get_int("wl1_80211h") == 1){
		dbG("[80211h] set_80211h_on\n");
		qcsapi_wifi_run_script("router_command.sh", "80211h_on");
	}else{
		dbG("[80211h] set_80211h_off\n");
		qcsapi_wifi_run_script("router_command.sh", "80211h_off");
	}

	if(sw_mode() == SW_MODE_ROUTER ||
		(sw_mode() == SW_MODE_AP &&
			nvram_get_int("wlc_psta") == 0)){
		if(nvram_get_int("wl1_chanspec") == 0){
			if (nvram_match("1:ccode", "EU")){
				if(nvram_get_int("acs_dfs") != 1){
					dbG("[dfs] start channel scanning and selection[%d]\n",
							nvram_get_int("acs_dfs"));
					start_channel_scan_qtn(0);
				}else{
					start_channel_scan_qtn(1);
				}
			}else{
				/* all country except EU */
				dbG("[dfs] start channel scanning and selection\n");
				start_channel_scan_qtn(1);
			}
		}
	}
	if(sw_mode() == SW_MODE_AP &&
		nvram_get_int("wlc_psta") == 1 &&
		nvram_get_int("wlc_band") == 0){
		ret = qcsapi_wifi_reload_in_mode(WIFINAME, qcsapi_station);
		if (ret < 0)
			dbG("qtn reload_in_mode STA fail\n");
	}
	if(nvram_get_int("QTNTELNETSRV") == 1 && sw_mode() == SW_MODE_ROUTER){
		dbG("[QTNT] enable telnet server\n");
		qcsapi_wifi_run_script("router_command.sh", "enable_telnet_srv 1");
	}

	dbG("[dbg] qtn_monitor startup\n");
ERROR:
	remove("/var/run/qtn_monitor.pid");

	return retval;
}
#endif
