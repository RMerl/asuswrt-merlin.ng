 /*
 * Copyright 2016, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#if !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK)) \
 || defined(RTCONFIG_SOC_IPQ8074)
#include <rc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <limits.h>

#if defined(RTCONFIG_QCA)
#include <qca.h>

#define PROC_NSS_CLOCK	"/proc/sys/dev/nss/clock"
#endif

// ErP debug
#define ERP_DEBUG  "/tmp/ERP_DEBUG"
#define ERP_DBG(fmt,args...) \
	if(f_exists(ERP_DEBUG)) { \
		printf("[ERP][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

// ErP path
#define ERP_FOLDER     "/tmp/ERP/"
#define ERP_GPHY       ERP_FOLDER"ERP_GPHY"       // erp gphy status
#define ERP_IPV6_ARP   ERP_FOLDER"ERP_IPV6"       // erp ipv6 client status

// ErP mode status
enum {
	ERP_WAKEUP = 0,
	ERP_STANDBY
};

// ErP parameters
/* the total period : 96 * 10  sec = 960 sec = 16 * 60 sec = 16 min */
#define ERP_DEFAULT_COUNT  96
static int erp_det_period = 0;             // detect mode
static int erp_led_period = 0;             // led mode for flashing
static int erp_count = ERP_DEFAULT_COUNT;  // ERP_DEFAULT_COUNT * 10 sec
static int erp_status = ERP_WAKEUP;        // init status is under wakeup mode
static int erp_led_switch = 0;             // control led on or off

// ErP Wifi station counter
#define MAX_SUBIF_NUM 4
#define MAX_STA_COUNT 128
#define ERP_ZERO_STA_COUNT 6	// Zero sta state over 6 * 10 sec = 60 sec, force to flush arp table if necessary
static unsigned int erp_zero_sta_cnt = 0;

static int erp_check_usb_stat()
{
	/*
		if ret = 0, no usb
		if ret = 1, plugin usb2 or usb3
	*/

	int ret = 0;

	if (strcmp(nvram_safe_get("usb_path1_node"), "") || strcmp(nvram_safe_get("usb_path2_node"), "")) ret = 1;

	return ret;
}

static int erp_check_wl_stat(int model)
{
	/*
		if enable wifi schedule, return -1

		if ret = 0, active interface = 0
		if ret = 1, active interface = 1
		if ret = 2, active interface = 2
		if ret = 3, active interface = 3
	*/

#if defined(RTCONFIG_QCA)
	int ret = 0, band;
	char prefix[sizeof("wlXYYY_")];

	for (band = WL_2G_BAND; band < MAX_NR_WL_IF ; ++band) {
		SKIP_ABSENT_BAND(band);

		snprintf(prefix, sizeof(prefix), "wl%d_", band);
		if (nvram_pf_match(prefix, "timesched", "1")) {
			ret = -1;
			break;
		}

		if (nvram_pf_get_int(prefix, "radio"))
			ret++;
	}

	return ret;
#else
	int ret = 0;

	/* wifi scheduler casse */
	if (nvram_match("wl0_timesched", "1") || nvram_match("wl1_timesched", "1") || nvram_match("wl2_timesched", "1")) {
		return -1;
	}

	if (nvram_get_int("wl0_radio")) ret++;
	if (nvram_get_int("wl1_radio")) ret++;

	/* special case */
	if (model == MODEL_RTAC5300 || model == MODEL_RTAC3200 || model == MODEL_GTAC5300 || model == MODEL_GTAX11000 || model == MODEL_RTAX92U || model == MODEL_RTAX95Q || model == MODEL_GTAXE11000)
	{
		if (nvram_get_int("wl2_radio")) ret++;
	}

	return ret;
#endif
}

#if defined(RTCONFIG_QCA)
/* Helper of erp_check_wl_auth_stat()
 * @src:	pointer to WLANCONFIG_LIST
 * @arg:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int handle_erp_check_wl_auth_stat(const WLANCONFIG_LIST *src, void *arg)
{
	int *sta_count = arg;

	if (!src || !arg)
		return -1;

	sta_count++;
	return 0;
}
#endif

static int erp_check_wl_auth_stat()
{
#if defined(RTCONFIG_QCA)
	int sta_count = 0, band, sunit, max_sunit;
	char wif[IFNAMSIZ], prefix[sizeof("wlX.YYYY_")];

	for (band = 0; band < MAX_NR_WL_IF; ++band) {
		SKIP_ABSENT_BAND(band);

		snprintf(prefix, sizeof(prefix), "wl%d_", band);
		if (!nvram_pf_match(prefix, "radio", "1"))
			continue;

		max_sunit = num_of_mssid_support(band);
		snprintf(prefix, sizeof(prefix), "wl%d_", band);
		for (sunit = 0; sunit <= max_sunit; ++sunit) {
			snprintf(prefix, sizeof(prefix), "wl%d.%d_", band, sunit);
			if (!nvram_pf_match(prefix, "bss_enabled", "1"))
				continue;

			__get_wlifname(band, sunit, wif);
			__get_qca_sta_info_by_ifname(wif, 0, handle_erp_check_wl_auth_stat, &sta_count);
		}
	}

	return sta_count;
#else
	char ifname[128], name[128], *next;
	char prefix[32], tmp[128];
	int unit, sub_unit;
	struct maclist *mac_list;
	int mac_list_size;
	int total_sta_cnt = 0;

	unit = 0;
	mac_list_size = sizeof(mac_list->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	mac_list = malloc(mac_list_size);
	if(!mac_list)
                goto exit;

	foreach(ifname, nvram_safe_get("wl_ifnames"), next) {
		for(sub_unit = 0; sub_unit < MAX_SUBIF_NUM; sub_unit++) {
			if(sub_unit > 0) {
				snprintf(prefix, sizeof(prefix), "wl%d.%d", unit, sub_unit);
				snprintf(name, sizeof(name), "wl%d.%d", unit, sub_unit);
			}
			else {
				snprintf(prefix, sizeof(prefix), "wl%d", unit);
				snprintf(name, sizeof(name), "%s", ifname);
			}
			if( !nvram_match(strcat_r(prefix, "_radio", tmp), "1") ) continue;
			if( !nvram_match(strcat_r(prefix, "_bss_enabled", tmp), "1") ) continue;

			memset(mac_list, 0, mac_list_size);
			/* query authentication sta list */
			strcpy((char*) mac_list, "authe_sta_list");
			if(wl_ioctl(name, WLC_GET_VAR, mac_list, mac_list_size))
				goto exit;

			ERP_DBG("%s: station count = %d\n", name, mac_list->count);
			total_sta_cnt += mac_list->count;
		}
		unit++;
	}
	if(mac_list) free(mac_list);

	return total_sta_cnt;

exit:
	if(mac_list) free(mac_list);
	return -1;
#endif
}

static int erp_check_arp_stat(int model)
{
	/*
		return ret
		0 : it doesn't match any rule as below
		1 : lan or wlan connection exist and more than 0 client
		2 : wan connection exist
	*/

	char buf[256], br_if[IFNAMSIZ];
	FILE *fp = NULL;
	int ret = 0;
	int br_n = 0;     // bridge client num
	char ifname[20];  // need to use array
	char ipaddr[128]; // ipv6 addr
	int flags = 0;    // arp flags
	int BR_NUM = 0;   // the bridge interface numbers
	int unit = 0;
	int wan_conn = 0; // wan connection stat

	/* check arp table for IPv4 */
	strlcpy(br_if, nvram_get("lan_ifname")? : "br0", sizeof(br_if));
	if ((fp = fopen("/proc/net/arp", "r")) != NULL)
	{
		// skip first row
		fgets(buf, sizeof(buf), fp);

		// while loop
		while (fgets(buf, sizeof(buf), fp))
		{
			memset(ifname, 0, sizeof(ifname));
			if (sscanf(buf, "%*s %*s 0x%x %*s %*s %s", &flags, ifname) != 2) continue;
			ERP_DBG("ifname=%s, flags=%d\n", ifname, flags);
			if (!strcmp(ifname, br_if) && (flags != 0)) br_n++;
		}

		fclose(fp);
	}

	/* check ipv6 client for IPv6 */
	eval("mkdir", "-p", ERP_FOLDER);
	doSystem("ip -f inet6 neigh show dev %s > %s", nvram_safe_get("lan_ifname"), ERP_IPV6_ARP);

	if ((fp = fopen(ERP_IPV6_ARP, "r")) != NULL)
	{
		// while loop
		while (fgets(buf, sizeof(buf), fp))
		{
			memset(ifname, 0, sizeof(ifname));
			memset(ipaddr, 0, sizeof(ipaddr));
			if (sscanf(buf, "%s %*s %s %*s", ipaddr, ifname) != 2) continue;
			ERP_DBG("ipaddr=%s, ifname=%s\n", ipaddr, ifname);
			if ( (ipaddr[0] == '2' || ipaddr[0] == '3')
				&& ipaddr[0] != ':' && ipaddr[1] != ':'
				&& ipaddr[2] != ':' && ipaddr[3] != ':')
			{
				br_n++; // ipv6 clients
			}
		}

		fclose(fp);
	}

	/* check wan connection stat */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if(is_wan_connect(unit)){
			wan_conn++;
		}
	}

	ERP_DBG("wan_conn = %d, br = %d\n", wan_conn, br_n);

	// TODO: RTAC87U BR_NUM is special case
	if (model == MODEL_RTAC87U) {
		BR_NUM = 1;
	}

	/* check bridge interface for WLAN or LAN */
	if (br_n > BR_NUM) {
		ret = 1;
		goto check_end;
	}

	/* check wan connection for WAN */
	if (wan_conn > 0) {
		ret = 2;
		goto check_end;
	}

	/* if no any match, return 0, it means "need to use ErP mode" */
	ret = 0;

check_end:
	ERP_DBG("ret = %d\n", ret);
	return ret;
}

static int erp_check_gphy_stat(int model)
{
	/*
		if ret = 0, there is "no"  gphy linked
		if ret = 1, there is       gphy linked
		if ret = 2, there is "wan" gphy linked
	*/

#if defined(RTCONFIG_QCA)
#if defined(RTCONFIG_DUALWAN)
	const int fb_fo = nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb");
#else
	const int fb_fo = 1;
#endif
	int ret = 1, wlink = 0, llink, unit;

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		if (wan_primary_ifunit() != unit && fb_fo)
			continue;
		wlink += rtkswitch_wanPort_phyStatus(unit);
	}
	llink = rtkswitch_lanPorts_phyStatus();

	if (!wlink && !llink)
		ret = 0;
	else if (wlink)
		ret = 2;

	ERP_DBG("wlink %d llink %d fb_fo %d ret %d\n", wlink, llink, fb_fo, ret);
	return ret;
#else
	/* create ERP_FOLDOER */
	if (!f_exists(ERP_FOLDER))
		mkdir(ERP_FOLDER, 0666);

	FILE *fp = NULL;
	int ret = 1;
	char v0[2], v1[2], v2[2], v3[2], v4[2], v5[2], v6[2], v7[2], v8[2];
	char buf[80];

	doSystem("ATE Get_WanLanStatus > %s", ERP_GPHY);

	if ((fp = fopen(ERP_GPHY, "r")) != NULL)
	{
		while (fgets(buf, sizeof(buf), fp))
		{
			memset(v0, 0, sizeof(v0));
			memset(v1, 0, sizeof(v1));
			memset(v2, 0, sizeof(v2));
			memset(v3, 0, sizeof(v3));
			memset(v4, 0, sizeof(v4));
			memset(v5, 0, sizeof(v5));
			memset(v6, 0, sizeof(v6));
			memset(v7, 0, sizeof(v7));
			memset(v8, 0, sizeof(v8));

			int len = strlen(buf);
			ERP_DBG("len = %d\n", len);
			if (len == 46) {
				sscanf(buf, "W0=%[^;];L1=%[^;];L2=%[^;];L3=%[^;];L4=%[^;];L5=%[^;];L6=%[^;];L7=%[^;];L8=%[^;];", v0, v1, v2, v3, v4, v5, v6, v7, v8);
				if ( (!strcmp(v0, "X") || (model==MODEL_DSLAC68U)) /* DSL-AC68U v0 always = "M" */
					&& !strcmp(v1, "X") && !strcmp(v2, "X") && !strcmp(v3, "X") && !strcmp(v4, "X")
					&& !strcmp(v5, "X") && !strcmp(v6, "X") && !strcmp(v7, "X") && !strcmp(v8, "X"))
				{
					ret = 0;
				}
				else if (strcmp(v0, "X") && (model!=MODEL_DSLAC68U)) /* no DSL-model */
				{
					ret = 2;
				}
			}
			else if(len == 31) {
				sscanf(buf, "W0=%[^;];L1=%[^;];L2=%[^;];L3=%[^;];L4=%[^;];L5=%[^;];", v0, v1, v2, v3, v4, v5);
				if ( (!strcmp(v0, "X") || (model==MODEL_DSLAC68U)) /* DSL-AC68U v0 always = "M" */
					&& !strcmp(v1, "X") && !strcmp(v2, "X") && !strcmp(v3, "X") && !strcmp(v4, "X") && !strcmp(v5, "X"))
				{
					ret = 0;
				}
				else if (strcmp(v0, "X") && (model!=MODEL_DSLAC68U)) /* no DSL-model */
				{
					ret = 2;
				}
			}
			else if(len == 26) {
				sscanf(buf, "W0=%[^;];L1=%[^;];L2=%[^;];L3=%[^;];L4=%[^;];", v0, v1, v2, v3, v4);
				if ( (!strcmp(v0, "X") || (model==MODEL_DSLAC68U)) /* DSL-AC68U v0 always = "M" */
					&& !strcmp(v1, "X") && !strcmp(v2, "X") && !strcmp(v3, "X") && !strcmp(v4, "X"))
				{
					ret = 0;
				}
				else if (strcmp(v0, "X") && (model!=MODEL_DSLAC68U)) /* no DSL-model */
				{
					ret = 2;
				}
			}
		}
		fclose(fp);
	}

	ERP_DBG("v0=%s, v1=%s, v2=%s, v3=%s, v4=%s, v5=%s, v6=%s, v7=%s, v8=%s, ret=%d\n", v0, v1, v2, v3, v4, v5, v6, v7, v8, ret);
	return ret;
#endif
}

static int erp_check_dsl_stat(int model)
{
	/*
		this function is only for DSL model,
		if you don't match model name of DSL, it always return 0
	*/

	int ret = 0;

	if (model != MODEL_DSLAC68U) return ret;

	if (nvram_match("dsltmp_adslsyncsts", "down"))
		ret = 0;
	else
		ret = 1;

	return ret;
}

#if defined(RTCONFIG_QCA)
#if defined(RTCONFIG_SOC_IPQ8074)
/* Reference to ipq8074_battery_power() of /etc/pm/power.d/ipq-power-save.sh of QSDK */
static void ipq8074_battery_power(void)
{
	char path[128], nss_freq[16] = "";

#if defined(RTCONFIG_FANCTRL)
	/* Turn off FAN */
	__setFanOnOff(0);
#endif

#if defined(RTCONFIG_WIGIG)
#warning FIXME: Implement PCIe Power-Down Sequence
	/* remove devices
	sleep 2
	for i in `ls /sys/bus/pci/devices/`; do
		d=/sys/bus/pci/devices/${i}
		v=`cat ${d}/vendor`
		[ "xx${v}" != "xx0x17cb" ] && echo 1 > ${d}/remove
	done
	 * remove Buses
	sleep 2
	for i in `ls /sys/bus/pci/devices/`; do
		d=/sys/bus/pci/devices/${i}
		echo 1 > ${d}/remove
	done
	 * remove RC.
	sleep 2

	[ -f /sys/bus/pci/rcremove ] && {
		echo 1 > /sys/bus/pci/rcremove
	}
	[ -f /sys/devices/pci0000:00/pci_bus/0000:00/rcremove ] && {
		echo 1 > /sys/devices/pci0000:00/pci_bus/0000:00/rcremove
	}
	sleep 1
	 */
#endif

	/* Wifi Power-down Sequence = fini_wl() in caller. */
	/* Find scsi devices and remove it and USB Power-down Sequence */
	system("(ejusb -1 -u 1 ; rmmod dwc3 ; rmmod dwc3-of-simple ; rmmod phy_msm_qusb ; rmmod phy_msm_ssusb_qmp) &");

	/* Power off Malibu PHY of LAN ports = lanport_ctrl(0) in caller */
	/* SD/MMC Power-down Sequence */
	/* LAN interface down */

	/* Disabling Auto scale on NSS cores */
	snprintf(path, sizeof(path), "%s/auto_scale", PROC_NSS_CLOCK);
	f_write_string(path, "0", 0, 0);

	/* Scaling Down UBI Cores */
	snprintf(nss_freq, sizeof(nss_freq), "%lu", 748800000UL);
	snprintf(path, sizeof(path), "%s/current_freq", PROC_NSS_CLOCK);
	f_write_string(path, nss_freq, 0, 0);

	/* Cortex Power-down Sequence */
	set_cpufreq_attr("scaling_governor", "powersave");
}
#endif	/* RTCONFIG_SOC_IPQ8074 */

static void pre_erp_standby_mode(int model)
{
	if (model <= MODEL_UNKNOWN)
		return;

	return;
}

static void post_erp_standby_mode(int model)
{
	if (model <= MODEL_UNKNOWN)
		return;

	switch (model) {
		case MODEL_RTAX89U:
		case MODEL_GTAXY16000:	/* fall-through */
			ipq8074_battery_power();
			break;
	}
	return;
}
#endif

static void erp_standby_mode(int model)
{
	ERP_DBG("enter standby mode, model = %d\n", model);

	// step1. wireless interface down
#if defined(RTCONFIG_QCA)
	pre_erp_standby_mode(model);
#else
	switch(model) {
		case MODEL_RTAC87U:
			eval("wl", "-i", "eth1", "down");
			break;
		case MODEL_GTAC5300:
		case MODEL_GTAX11000:
		case MODEL_RTAX92U:
		case MODEL_GTAXE11000:
			eval("wl", "-i", "eth6", "down");
			eval("wl", "-i", "eth7", "down"); // turn off 5g radio
			break;
		case MODEL_RTAX95Q:
			eval("wl", "-i", "eth4", "down");
			eval("wl", "-i", "eth5", "down"); // turn off 5g radio
			break;
		case MODEL_RTAX56_XD4:
		case MODEL_CTAX56_XD4:
			eval("wl", "-i", "wl0", "down");
			eval("wl", "-i", "wl1", "down"); // turn off 5g radio
			break;
		case MODEL_RTAX58U:
			eval("wl", "-i", "eth5", "down");
			eval("wl", "-i", "eth6", "down"); // turn off 5g radio
			break;
		case MODEL_RTAX55:
			eval("wl", "-i", "eth2", "down");
			eval("wl", "-i", "eth3", "down"); // turn off 5g radio
			break;
		case MODEL_RTAX56U:
			eval("wl", "-i", "eth5", "down");
			eval("wl", "-i", "eth6", "down"); // turn off 5g radio
			break;
		case MODEL_RPAX56:
			eval("wl", "-i", "eth1", "down");
			eval("wl", "-i", "eth2", "down"); // turn off 5g radio
			break;
		case MODEL_RTAX88U:
			eval("wl", "-i", "eth6", "down");
			eval("wl", "-i", "eth7", "down"); // turn off 5g radio
			break;
		default:
			eval("wl", "-i", "eth1", "down");
			eval("wl", "-i", "eth2", "down"); // turn off 5g radio
			break;
	}

	/* TODO : add different platform or model case here */
	if (model == MODEL_RTAC87U) {
		eval("qcsapi_sockrpc", "pm", "suspend");
		eval("qcsapi_sockrpc", "pm", "idle");
	}

	if (model == MODEL_RTAC5300 || model == MODEL_RTAC3200)
	{
		// triple band
		eval("wl", "-i", "eth3", "down"); // turn off 5g-2 radio
	}

	if (model == MODEL_GTAC5300 || model == MODEL_GTAX11000 || model == MODEL_GTAXE11000) {
		// triple band
		eval("wl", "-i", "eth8", "down"); // turn off 5g-2 radio
	}

	if (model == MODEL_RTAX92U) {
		// triple band
		eval("wl", "-i", "eth5", "down"); // turn off 2g radio
	}

	if (model == MODEL_RTAX95Q) {
		// triple band
		eval("wl", "-i", "eth4", "down"); // turn off 2g radio
	}

	if (model == MODEL_RTAX56_XD4 || model == MODEL_CTAX56_XD4) {
		// triple band
		eval("wl", "-i", "wl0", "down"); // turn off 2g radio
	}

	if (model == MODEL_RTAX58U) {
		// triple band
		eval("wl", "-i", "eth5", "down"); // turn off 2g radio
	}

	if (model == MODEL_RTAX56U) {
		// triple band
		eval("wl", "-i", "eth5", "down"); // turn off 2g radio
	}

	if (model == MODEL_RPAX56) {
		eval("wl", "-i", "eth1", "down"); // turn off 2g radio
	}

	if (model == MODEL_RTAX55) {
		// triple band
		eval("wl", "-i", "eth2", "down"); // turn off 2g radio
	}

	if (model == MODEL_DSLAC68U) {
		eval("req_dsl_drv", "dslenable", "off"); //turn off DSL
	}
#endif

	// step2. remove wireless driver module
	fini_wl();

	// step3. use set_phy_ctrl to control all lan / wan ports down
	wanport_ctrl(0);
	lanport_ctrl(0);

	// step4. special case
#if defined(RTCONFIG_QCA)
	post_erp_standby_mode(model);
#else
	if (model == MODEL_RTAC88U || model == MODEL_RTAC5300 || model == MODEL_RTAC3100) {
		eval("devmem", "0x18000068", "32", "0x401");
		eval("devmem", "0x18000064", "32", "0x0");
	}
#endif

	/* update status */
	erp_status = ERP_STANDBY;
	erp_count = -1;
}

#if defined(RTCONFIG_QCA)
#if defined(RTCONFIG_SOC_IPQ8074)
/* Reference to ipq8074_ac_power() of /etc/pm/power.d/ipq-power-save.sh of QSDK */
static void ipq8074_ac_power(void)
{
	char path[128];

	/* Cortex Power-UP Sequence  = ipq807x_power_auto() of /etc/init.d/powerctl */
	/* change scaling governor as ondemand to enable clock scaling based on system load */
	set_cpufreq_attr("scaling_governor", "ondemand");
	/* Change sampling rate for frequency scaling decisions to 1s, from 10 ms */
	/* FIXME: It's 200000 on ASUSWRT */
	f_write_string("/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate", "1000000", 0, 0);

	/* Change sampling rate for frequency down scaling decision to 10s */
	/* FIXME: It's 1 on ASUSWRT */
	f_write_string("/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor", "10", 0, 0);

	/* Change the CPU load threshold above which frequency is up-scaled to
	 * turbo frequency,to 50%
	 */
	/* FIXME: It's 95 on ASUSWRT */
	f_write_string("/sys/devices/system/cpu/cpufreq/ondemand/up_threshold", "50", 0, 0);

	/* Enabling Auto scale on NSS cores */
	snprintf(path, sizeof(path), "%s/auto_scale", PROC_NSS_CLOCK);
	f_write_string(path, "1", 0, 0);

	/* Power on Malibu PHY of LAN ports = lanport_ctrl(1) of caller. */

#if defined(RTCONFIG_WIGIG)
	/* PCIe Power-UP Sequence */
	/* FIXME: It's not good idea to sleep in signal handler. */
	sleep(1);
	f_write_string("/sys/bus/pci/rcrescan", "1", 0, 0);
	sleep(2);
	f_write_string("/sys/bus/pci/rescan", "1", 0, 0);
#endif

	/* Wifi Power-up Sequence = restart_wireless in caller. */

	/* USB Power-UP Sequence */
	system("(insmod phy-msm-ssusb-qmp.ko ; insmod phy-msm-qusb.ko ; insmod dwc3-of-simple.ko ; insmod dwc3.ko) &");

	/* LAN interface up, closed to restart_wireless in caller. */
	/* SD/MMC Power-UP sequence */

#if defined(RTCONFIG_FANCTRL)
	restart_fanctrl();
#endif
}
#endif	/* RTCONFIG_SOC_IPQ8074 */

static void pre_erp_wakeup_mode(int model)
{
	if (model <= MODEL_UNKNOWN)
		return;

	switch (model) {
		case MODEL_RTAX89U:
		case MODEL_GTAXY16000:	/* fall-through */
			ipq8074_ac_power();
			break;
	}
	return;
}

static void post_erp_wakeup_mode(int model)
{
	if (model <= MODEL_UNKNOWN)
		return;

	return;
}
#endif

static void erp_wakeup_mode(int model)
{
	ERP_DBG("enter wakeup mode, model = %d\n", model);

	/* usb power up*/
	eval("rc", "pwr_usb", "1");

#if defined(RTCONFIG_QCA)
	pre_erp_wakeup_mode(model);
#else
	/* TODO : add different platform or model case here */
	if (model == MODEL_RTAC88U) {
		// Realtek set reset mode
		eval("devmem", "0x18000068", "32", "0x200");
		eval("rtkswitch", "0");
		eval("rtkswitch", "1");
		eval("rtkswitch", "14", "1");
		eval("rtkswitch", "15", "1");
	}

	if (model == MODEL_RTAC5300 || model == MODEL_RTAC3100) {
		eval("devmem", "0x18000068", "32", "0x200");
	}

	if (model == MODEL_RTAC87U) {
		eval("qcsapi_sockrpc", "pm", "off");
	}

	if (model == MODEL_DSLAC68U) {
		eval("req_dsl_drv", "dslenable", "on"); //turn on DSL
	}
#endif

	/* use set_phy_ctrl to control all lan / wan ports up */
	wanport_ctrl(1);
	lanport_ctrl(1);

	/* restart_wireless */
	notify_rc("restart_wireless");

#if defined(RTCONFIG_QCA)
	post_erp_wakeup_mode(model);
#endif

	/* update status */
	erp_status = ERP_WAKEUP;
	erp_count = ERP_DEFAULT_COUNT;
}

static void ERP_BTN_WAKEUP()
{
	int active = 0;
	int model = get_model();
#if defined(RTCONFIG_LED_BTN) || !defined(RTCONFIG_WIFI_TOG_BTN)
	static int led_status_on = 1;
#endif

	if (erp_status != ERP_STANDBY || erp_count != -1)
		return;

#if defined(RTCONFIG_LED_BTN) || !defined(RTCONFIG_WIFI_TOG_BTN)
	if (model != MODEL_RTAC87U) {
#if defined(RTAX88U) || defined(RTAX92U) || defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_QCA)
#ifndef RTCONFIG_WIFI_TOG_BTN
		if (button_pressed(BTN_WPS) && nvram_match("btn_ez_radiotoggle", "0") && nvram_match("btn_ez_mode", "1"))
#else
		if (button_pressed(BTN_LED))
#endif
#else
#ifndef RTCONFIG_WIFI_TOG_BTN
		if (!button_pressed(BTN_WPS) && nvram_match("btn_ez_radiotoggle", "0") && nvram_match("btn_ez_mode", "1"))
#else
		if (!button_pressed(BTN_LED))
#endif
#endif
		{
			ERP_DBG("PRESSED LED BUTTON!\n");
			active = 1;
		}
	}
	else {
		// TODO: RTAC87U BTN_LED is special case
		if (led_status_on != nvram_get_int("AllLED"))
		{
			ERP_DBG("PRESSED LED BUTTON! (RT-AC87U)\n");
			active = 1;
			led_status_on = nvram_get_int("AllLED");
		}
	}
#endif
#ifndef RTCONFIG_N56U_SR2
	if (button_pressed(BTN_RESET))
	{
		ERP_DBG("PRESSED RESET BUTTON!\n");
		active = 1;
	}
#endif
	if (button_pressed(BTN_WPS))
	{
		ERP_DBG("PRESSED WPS BUTTON!\n");
		active = 1;
	}
#if defined(RTCONFIG_WIFI_TOG_BTN)
	if (button_pressed(BTN_WIFI_TOG))
	{
		ERP_DBG("PRESSED WIFI_TOG BUTTON!\n");
		active = 1;
	}
#endif
#if defined(RTCONFIG_TURBO_BTN)
	if (button_pressed(BTN_TURBO))
	{
		ERP_DBG("PRESSED WIFI_TOG BUTTON!\n");
		active = 1;
	}
#endif

	if (active == 1) erp_wakeup_mode(model);
}

static void ERP_STANDBY_LED()
{
	if (erp_status != ERP_STANDBY || erp_count != -1)
		return;

	int gpio = nvram_get_int("led_pwr_gpio");

	// get the real led_pwr_gpio
	gpio &= ~GPIO_ACTIVE_LOW;
	erp_led_switch = !erp_led_switch;
	ERP_DBG("gpio=%d, erp_led_switch=%d\n", gpio, erp_led_switch);

	// execute the led on or off
	doSystem("rc gpiow %d %d", gpio, erp_led_switch);
}

static void ERP_CHECK_MODE()
{
	// step1. check support model list
	/*
		support list for BRCM only
		RT-AC5300 / RT-AC3100 / RT-AC5300
		RT-AC3200
		RT-AC68U / RT-AC87U / DSL-AC68U
		RT-AC66U / RT-N66U
	*/
	int model = get_model();
	if (
#if defined(RTCONFIG_QCA)
		   model != MODEL_RTAX89U
		&& model != MODEL_GTAXY16000
#else
		   model != MODEL_RTAC88U
		&& model != MODEL_RTAC3100
		&& model != MODEL_RTAC5300
		&& model != MODEL_RTAC3200
		&& model != MODEL_RTAC68U
		&& model != MODEL_RTAC87U
		&& model != MODEL_DSLAC68U
		&& model != MODEL_RTAC66U
		&& model != MODEL_RTN66U
		&& model != MODEL_GTAC5300
		&& model != MODEL_RTAX88U
		&& model != MODEL_GTAX11000
		&& model != MODEL_RTAX92U
		&& model != MODEL_RTAX95Q
		&& model != MODEL_RTAX56_XD4
		&& model != MODEL_CTAX56_XD4
		&& model != MODEL_RTAX58U
		&& model != MODEL_RTAX55
		&& model != MODEL_RTAX56U
		&& model != MODEL_RPAX56
		&& model != MODEL_GTAXE11000
#endif
	   )
	{
//		ERP_DBG("The model isn't under support list!\n");
		return;
	}

	// step2. tcode in EE / WE / UK / EU
	char *tcode = nvram_safe_get("territory_code");
	if (strstr(tcode, "EE") == NULL && strstr(tcode, "WE") == NULL
		&& strstr(tcode, "UK") == NULL && strstr(tcode, "EU") == NULL)
	{
//		ERP_DBG("The model isn't under EU SKU!\n");
		return;
	}

	if (nvram_get_int("Ate_power_on_off_enable")) {
		ERP_DBG("ATE run-in mode\n");
		return;
	}

	// step3. check QIS finished
	if (nvram_get_int("x_Setting") != 1 || nvram_get_int("w_Setting") != 1)
	{
		ERP_DBG("QIS isn't finished\n");
		return;
	}

	// step4. check wl, gphy ,arp, and dsl status
	int erp_wl_sta_num = -1;
	int erp_usb  = erp_check_usb_stat();
	int erp_wl   = erp_check_wl_stat(model);
	int erp_arp  = erp_check_arp_stat(model);
	int erp_gphy = erp_check_gphy_stat(model);
	int erp_dsl  = erp_check_dsl_stat(model); // DSL model
#if defined(RTCONFIG_QCA)
	erp_wl_sta_num = erp_check_wl_auth_stat();
#else
	if (model == MODEL_GTAC5300 || model == MODEL_RTAX88U || model == MODEL_GTAX11000 || model == MODEL_RTAX92U || model == MODEL_RTAX95Q || model == MODEL_RTAX56_XD4 || model == MODEL_CTAX56_XD4 || model == MODEL_RTAX58U || model == MODEL_RTAX55 || model == MODEL_RTAX56U || model == MODEL_RPAX56 || model == MODEL_GTAXE11000)
		erp_wl_sta_num = erp_check_wl_auth_stat();
#endif

	ERP_DBG("erp_usb=%d, erp_wl=%d, erp_arp=%d, erp_gphy=%d, erp_dsl=%d, erp_status=%d, erp_wl_sta_num=%d, erp_count=%d\n",
		erp_usb, erp_wl, erp_arp, erp_gphy, erp_dsl, erp_status, erp_wl_sta_num, erp_count);

	/* force to flust arp table in case arp table does not free*/
	if(erp_status == ERP_WAKEUP) {
		if((erp_arp > 0 && erp_gphy == 0 && (erp_wl == 0 || erp_wl == 1)) && erp_wl_sta_num == 0) {
			ERP_DBG("erp_zero_sta_cnt = %d\n", erp_zero_sta_cnt);
			erp_zero_sta_cnt = (erp_zero_sta_cnt == UINT_MAX) ? 1 : erp_zero_sta_cnt + 1;
		}
		else
			erp_zero_sta_cnt = 0;

		if (erp_zero_sta_cnt >= ERP_ZERO_STA_COUNT) {
			ERP_DBG("force to flush arp table...\n");
			doSystem("ip -s neigh flush all > /dev/null");
			erp_zero_sta_cnt = 0;
			goto wakeup;
		}
	}

	// usb enabled
	if (erp_usb == 1) goto wakeup;

	// wifi scheduler enabled
	if (erp_wl == -1) goto wakeup;

	// wifi interface is over 1
	if (erp_wl == 2 || erp_wl == 3) goto wakeup;

	// lan / wlan / wan connection
	if (erp_arp != 0) goto wakeup;

	// gphy linked
	if (erp_gphy != 0) goto wakeup;

	// DSL model
	if (model == MODEL_DSLAC68U && erp_dsl == 1) goto wakeup;

	// step5. check timeout count
	/*
		(wl, arp, gphy) = (0, 0, 0) or (1, 0, 0)
	*/
	if (erp_status == ERP_WAKEUP)
	{
		if ((erp_wl == 0 && erp_arp == 0 && erp_gphy == 0) || (erp_wl == 1 && erp_arp == 0 && erp_gphy == 0)) {
			if (erp_count > 0)
				erp_count--;

			if (erp_count == 0)
				erp_standby_mode(model);
		}
		else { // not match erp standby condition
			goto wakeup;
		}
	}

	return;

wakeup:
	if (erp_status == ERP_STANDBY)
		erp_wakeup_mode(model);
	else
		erp_count = ERP_DEFAULT_COUNT;
}

static int sig_cur = -1;

static void catch_sig(int sig)
{
	sig_cur = sig;

	if (sig == SIGTERM)
	{
		remove("/var/run/erp_monitor.pid");
		exit(0);
	}
	else if(sig == SIGALRM)
	{
		ERP_BTN_WAKEUP(); // use hardware button to wake up router

		erp_led_period = (erp_led_period + 1) % 3;
		if (erp_led_period == 0) ERP_STANDBY_LED();

		erp_det_period = (erp_det_period + 1) % 10;
		if (erp_det_period == 0) ERP_CHECK_MODE();
	}
}

int erp_monitor_main(int argc, char **argv)
{
	FILE *fp;
	sigset_t sigs_to_catch;

	/*
		support list for BRCM only
		RT-AC5300 / RT-AC3100 / RT-AC5300
		RT-AC3200
		RT-AC68U / RT-AC87U / DSL-AC68U
		RT-AC66U / RT-N66U
	*/
	int model = get_model();
	if (
#if defined(RTCONFIG_QCA)
		   model != MODEL_RTAX89U
		&& model != MODEL_GTAXY16000
#else
		   model != MODEL_RTAC88U
		&& model != MODEL_RTAC3100
		&& model != MODEL_RTAC5300
		&& model != MODEL_RTAC3200
		&& model != MODEL_RTAC68U
		&& model != MODEL_RTAC87U
		&& model != MODEL_DSLAC68U
		&& model != MODEL_RTAC66U
		&& model != MODEL_RTN66U
		&& model != MODEL_GTAC5300
		&& model != MODEL_RTAX88U
		&& model != MODEL_GTAX11000
		&& model != MODEL_RTAX92U
		&& model != MODEL_RTAX95Q
		&& model != MODEL_RTAX56_XD4
		&& model != MODEL_CTAX56_XD4
		&& model != MODEL_RTAX58U
		&& model != MODEL_RTAX55
		&& model != MODEL_RTAX56U
		&& model != MODEL_RPAX56
		&& model != MODEL_GTAXE11000
#endif
	   )
	{
//		logmessage("ERP", "The model isn't under support list!\n");
		return -1;
	}

	/* tcode support in EE / WE / UK / EU */
	char *tcode = nvram_safe_get("territory_code");
	if (strstr(tcode, "EE") == NULL && strstr(tcode, "WE") == NULL
		&& strstr(tcode, "UK") == NULL && strstr(tcode, "EU") == NULL)
	{
//		logmessage("ERP", "The model isn't under EU SKU!\n");
		return -2;
	}

	/* write pid */
	if ((fp = fopen("/var/run/erp_monitor.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	else {
		logmessage("ERP", "Fail to create erp_monitor.pid\n");
		return -3;
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGTERM, catch_sig);
	signal(SIGALRM, catch_sig);

	while(1)
	{
		alarm(1);
		pause();
	}

	return 0;
}

void stop_erp_monitor()
{
	killall_tk("erp_monitor");
}

void start_erp_monitor()
{
	char *erp_monitor_argv[] = {"erp_monitor", NULL};
	pid_t pid;

	_eval(erp_monitor_argv, NULL, 0, &pid);
}
#endif // !(defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_REALTEK))
